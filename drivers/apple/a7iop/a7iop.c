/*
 * Copyright (C) 2011-2012 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */

#include <debug.h>
#include <drivers/a7iop/a7iop.h>
#include <platform/int.h>
#include <platform/soc/hwclocks.h>
#include <platform/soc/hwisr.h>
#include <platform/soc/hwregbase.h>
#include <sys.h>
#include <sys/task.h>
#include <platform/timer.h>

#define low_bits(x) ((x) & 0x00000000ffffffffULL)
#define high_bits(x) (((x) & 0xffffffff00000000ULL) >> 32)

#define MAX_CLOCKS	(2)

static int32_t _akf_init(const KFWRAPPER_TYPE_T kfw_type);
static void _akf_recv_mbox_nonempty_int_handler(void *arg);
static void _akf_send_mbox_empty_int_handler(void *arg);
static void _akf_set_timebase(addr_t regs_base);

// KFWrappers are indexed by types defined in enum table
static struct akf_wrapper {
	uint64_t		base;
	uint32_t		clock_idx[MAX_CLOCKS];
	uint32_t		inbox_int;
	uint32_t		outbox_int;
	struct task_event	recv_mbox_wait_event;
	struct task_event	send_mbox_wait_event;
	bool			set_iop_timebase;
} _akf_wrappers[] = {
#ifdef ANS_AKF_BASE_ADDR
#ifdef WITH_ANS_DLL
	[KFW_ANS] =	{ .base = ANS_AKF_BASE_ADDR, 
			  .clock_idx = { CLK_ANS, CLK_ANS_DLL },
			  .inbox_int = INT_ANS_KF_INBOX_EMPTY, 
			  .outbox_int = INT_ANS_KF_OUTBOX_NOTEMPTY,
			  .set_iop_timebase = true
		    	},
#else
	[KFW_ANS] =	{ .base = ANS_AKF_BASE_ADDR,
			  .clock_idx = { CLK_ANS, UINT32_MAX },
			  .inbox_int = INT_ANS_KF_INBOX_EMPTY,
			  .outbox_int = INT_ANS_KF_OUTBOX_NOTEMPTY,
			  .set_iop_timebase = true
		    	},
#endif
#else 	
	[KFW_ANS] =	{ .base = UINT64_MAX, 
			  .clock_idx = { UINT32_MAX }, 
			  .inbox_int = UINT32_MAX, 
			  .outbox_int = UINT32_MAX,
			  .set_iop_timebase = false
		    	},
#endif
#ifdef ASIO_AKF_BASE_ADDR	
	[KFW_SIO] =	{ .base = ASIO_AKF_BASE_ADDR, 
			  .clock_idx = { CLK_SIO, CLK_SIO_P },
			  .inbox_int = INT_SIO_KF_INBOX_EMPTY, 
			  .outbox_int = INT_SIO_KF_OUTBOX_NOTEMPTY,
			  .set_iop_timebase = false
		    	},
#else 	
	[KFW_SIO] =	{ .base = UINT64_MAX, 
			  .clock_idx = { UINT32_MAX }, 
			  .inbox_int = UINT32_MAX, 
			  .outbox_int = UINT32_MAX,
			  .set_iop_timebase = false
		    	},
#endif
#ifdef ASEP_AKF_BASE_ADDR	
	[KFW_SEP] =	{ .base = ASEP_AKF_BASE_ADDR, 
		  	  .clock_idx = { UINT32_MAX, UINT32_MAX }, 
			  .inbox_int = INT_SEP_KF_INBOX_EMPTY, 
			  .outbox_int = INT_SEP_KF_OUTBOX_NOTEMPTY 
		    	},
#else 	
	[KFW_SEP] =	{ .base = UINT64_MAX, 
			  .clock_idx = { UINT32_MAX }, 
			  .inbox_int = UINT32_MAX, 
			  .outbox_int = UINT32_MAX,
			  .set_iop_timebase = false
		    	},
#endif
};

/*
 * akf_start
 *
 * Initialize KFWrapper (clocks, interrupts), sets up remap window,
 * Enable FIFOs, and boot KF core.
 * It assumes firmware address is aligned, and memory allocated is contiguous.
 *
 */
int32_t akf_start(const KFWRAPPER_TYPE_T kfw_type, const addr_t firmware_address, const uint64_t firmware_size)
{
	addr_t fw_phy_addr;
	addr_t akf_base;
	uint32_t i;

	akf_base = _akf_wrappers[kfw_type].base;
	ASSERT(akf_base != 0);

	// register for interrupts
	install_int_handler(_akf_wrappers[kfw_type].outbox_int, _akf_recv_mbox_nonempty_int_handler, (void *)kfw_type);
	install_int_handler(_akf_wrappers[kfw_type].inbox_int, _akf_send_mbox_empty_int_handler, (void *)kfw_type);

	// initialize wait events
	event_init(&_akf_wrappers[kfw_type].recv_mbox_wait_event, EVENT_FLAG_AUTO_UNSIGNAL, false);
	event_init(&_akf_wrappers[kfw_type].send_mbox_wait_event, EVENT_FLAG_AUTO_UNSIGNAL, false);

	// enable clock and power
	for (i = 0; i < MAX_CLOCKS; i++) {
		if (_akf_wrappers[kfw_type].clock_idx[i] == UINT32_MAX)
			break;
		clock_gate(_akf_wrappers[kfw_type].clock_idx[i], true);
	}

	// mask all the interrupts
	rAKF_AP_MAILBOX_SET(akf_base) = AKF_MAILBOX_O_NONEMPTY_MSK | AKF_MAILBOX_O_EMPTY_MSK | 
					AKF_MAILBOX_I_NONEMPTY_MSK | AKF_MAILBOX_I_EMPTY_MSK;

	// setup remap
	rAKF_AXI_START(akf_base) = 0;
	rAKF_AXI_START_EXT(akf_base) = 0;

	rAKF_AXI_END(akf_base) = (uint32_t)(low_bits(firmware_size));
	rAKF_AXI_END_EXT(akf_base) = (uint32_t)(high_bits(firmware_size));

	fw_phy_addr = mem_static_map_physical(firmware_address);

	rAKF_AXI_BASE(akf_base) = (uint32_t)(low_bits(fw_phy_addr));
	rAKF_AXI_BASE_EXT(akf_base) = (uint32_t)(high_bits(fw_phy_addr));

	// enable FIFOs
	rAKF_AP_INBOX_CTRL(akf_base) |= AKF_AP_INBOX_CTRL_ENABLE;
	rAKF_AP_OUTBOX_CTRL(akf_base) |= AKF_AP_OUTBOX_CTRL_ENABLE;

#if AKF_VERSION == 2
	rAKF_KIC_MAILBOX_EXT_CLR(akf_base) = 0x1111;		
#endif
	// if appropriate, set IOP timebase to be close to and in sync with local timebase (from aic),
	if (_akf_wrappers[kfw_type].set_iop_timebase) {
	  _akf_set_timebase(akf_base);
	}

	// set run bit
	rAKF_CPU_CTRL(akf_base) = AKF_CPU_CTRL_RUN;

	// let interrupt flow
	unmask_int(_akf_wrappers[kfw_type].outbox_int);
	unmask_int(_akf_wrappers[kfw_type].inbox_int);

	return 0;
}

/*
 * akf_start_sep
 *
 * SEP needs special handling.
 * Register and enables interrupts, enable fifos
 *
 */
int32_t akf_start_sep(void)
{
	// register for interrupts
	install_int_handler(_akf_wrappers[KFW_SEP].outbox_int, _akf_recv_mbox_nonempty_int_handler, (void *)KFW_SEP);
	install_int_handler(_akf_wrappers[KFW_SEP].inbox_int, _akf_send_mbox_empty_int_handler, (void *)KFW_SEP);

	// initialize wait events
	event_init(&_akf_wrappers[KFW_SEP].recv_mbox_wait_event, EVENT_FLAG_AUTO_UNSIGNAL, false);
	event_init(&_akf_wrappers[KFW_SEP].send_mbox_wait_event, EVENT_FLAG_AUTO_UNSIGNAL, false);

	// mask all the interrupts
	rAKF_AP_MAILBOX_SET(_akf_wrappers[KFW_SEP].base) = AKF_MAILBOX_O_NONEMPTY_MSK | AKF_MAILBOX_O_EMPTY_MSK |
							AKF_MAILBOX_I_NONEMPTY_MSK | AKF_MAILBOX_I_EMPTY_MSK;

	// enable FIFOs
	rAKF_AP_INBOX_CTRL(_akf_wrappers[KFW_SEP].base) |= AKF_AP_INBOX_CTRL_ENABLE;
	rAKF_AP_OUTBOX_CTRL(_akf_wrappers[KFW_SEP].base) |= AKF_AP_OUTBOX_CTRL_ENABLE;

	// let interrupt flow
	unmask_int(_akf_wrappers[KFW_SEP].outbox_int);
	unmask_int(_akf_wrappers[KFW_SEP].inbox_int);	

	return 0;
}

/*
 * akf_recv_mbox
 *
 * Fetch a message from outbound mailbox.
 * If there are no messages in outbox, outbox-non-empty interrupt will be enabled.
 * If wait_timeout is UINT32_MAX, driver will wait for ever for a message
 * 0: success, -1: outbox empty, -2: timed out
 *
 */
int32_t akf_recv_mbox(const KFWRAPPER_TYPE_T kfw_type, uint64_t *msg, uint32_t wait_timeout)
{
	int32_t ret;
	addr_t akf_base;
	
	akf_base = _akf_wrappers[kfw_type].base;
	ASSERT(akf_base != 0);

	ret = 0;
	enter_critical_section();
	
	while (rAKF_AP_OUTBOX_CTRL(akf_base) & AKF_AP_OUTBOX_CTRL_EMPTY) {
		if (!wait_timeout) {
			ret = -1;
			goto exit;
		}
		
		rAKF_AP_MAILBOX_CLR(akf_base) = AKF_MAILBOX_O_NONEMPTY_MSK;
		if (wait_timeout == UINT32_MAX) {
			event_wait(&_akf_wrappers[kfw_type].recv_mbox_wait_event);
		}
		else {
			// XXX recompute wait timeout here?
			if (event_wait_timeout(&_akf_wrappers[kfw_type].recv_mbox_wait_event, wait_timeout) == false) {
				ret = -2;
				goto exit;
			}
		}
	}

	// assert for over-flow and under-flow errors
	RELEASE_ASSERT((rAKF_AP_OUTBOX_CTRL(akf_base) & (AKF_AP_OUTBOX_CTRL_UDFL | AKF_AP_OUTBOX_CTRL_OVFL)) == 0);

	*msg = akf_read_ap_outbox(akf_base);

exit:
	exit_critical_section();
	return ret;
}

/*
 * akf_send_mbox
 *
 * Send a message to inbound mailbox.
 * If there is no space in the inbox, inbox-empty interrupt will be enabled.
 * If wait_timeout is UINT32_MAX, driver will wait for ever for space in the mailbox to stuff a new message.
 * 0: success, -1: inbox full, -2: timed out
 *
 */
int32_t akf_send_mbox(const KFWRAPPER_TYPE_T kfw_type, const uint64_t msg, uint32_t wait_timeout)
{
	int32_t ret;
	addr_t akf_base;
	
	akf_base = _akf_wrappers[kfw_type].base;
	ASSERT(akf_base != 0);
	
	ret = 0;
	enter_critical_section();
	
	while (rAKF_AP_INBOX_CTRL(akf_base) & AKF_AP_INBOX_CTRL_FULL) {
		if (!wait_timeout) {
			ret = -1;
			goto exit;
		}
		
		rAKF_AP_MAILBOX_CLR(akf_base) = AKF_MAILBOX_I_EMPTY_MSK;
		if (wait_timeout == UINT32_MAX) {
			event_wait(&_akf_wrappers[kfw_type].send_mbox_wait_event);
		}
		else {
			// XXX recompute wait timeout here?
			if (event_wait_timeout(&_akf_wrappers[kfw_type].send_mbox_wait_event, wait_timeout) == false) {
				ret = -2;
				goto exit;
			}
		}
	}
	
	// assert for over-flow and under-flow errors
	RELEASE_ASSERT((rAKF_AP_INBOX_CTRL(akf_base) & (AKF_AP_INBOX_CTRL_UDFL | AKF_AP_INBOX_CTRL_OVFL)) == 0);

	akf_write_ap_inbox(akf_base, msg);

exit:	
	exit_critical_section();	
	return ret;
}

/*
 * akf_send_nmi
 *
 * Sends NMI to the KF core.
 * KF Timer1 is used for generating FIQ NMI. It setups up FIQ Software source.
 *
 */
int32_t akf_send_nmi(const KFWRAPPER_TYPE_T kfw_type)
{
	addr_t akf_base;
	
	akf_base = _akf_wrappers[kfw_type].base;
	ASSERT(akf_base != 0);

	if ((rAKF_KIC_TMR_CFG1(akf_base) & AKF_KIC_TMR_CFG_NMI) == 0) {
		dprintf(DEBUG_INFO, "akf_send_nmi: KIC_TMR_CFG1 not configured for NMI\n");
		return -1;
	}
	
	rAKF_KIC_TMR_STATE_SET1(akf_base) = AKF_KIC_TMR_STATE_SET_SGT;
	
	return 0;
}

/*
 * akf_stop
 *
 * Stops the KF core, mask all FIFO interrupts
 * Book-keeping related to shutdown (quiesce clocks, uninstall interrupt handler, clears remap window)
 *
 */
int32_t akf_stop(const KFWRAPPER_TYPE_T kfw_type)
{
	addr_t akf_base;
	uint32_t i;

	akf_base = _akf_wrappers[kfw_type].base;
	ASSERT(akf_base != 0);

	enter_critical_section();

	// mask interrupts
	mask_int(_akf_wrappers[kfw_type].outbox_int);
	mask_int(_akf_wrappers[kfw_type].inbox_int);

	// XXX clear any pending interrupts?

	// disable FIFOs
	rAKF_AP_INBOX_CTRL(akf_base) = (rAKF_AP_INBOX_CTRL(akf_base) & ~AKF_AP_INBOX_CTRL_ENABLE);
	rAKF_AP_OUTBOX_CTRL(akf_base) = (rAKF_AP_OUTBOX_CTRL(akf_base) & ~AKF_AP_OUTBOX_CTRL_ENABLE);

	// SEP: special handling, clean local states
	if (kfw_type == KFW_SEP)
		goto done;

	// at this time, KF should be idle (not generating any transactions to fabric)

	// stop KF and keep it in reset
	rAKF_CPU_CTRL(akf_base) = (rAKF_CPU_CTRL(akf_base) & ~AKF_CPU_CTRL_RUN);

	// clear remap registers
	rAKF_AXI_START(akf_base) = 0;
	rAKF_AXI_START_EXT(akf_base) = 0;

	rAKF_AXI_END(akf_base) = 0;
	rAKF_AXI_END_EXT(akf_base) = 0;

	rAKF_AXI_BASE(akf_base) = 0;
	rAKF_AXI_BASE_EXT(akf_base) = 0;

	// disable clock and power, clock_gate calls takes care of both
	for (i = 0; i < MAX_CLOCKS; i++) {
		if (_akf_wrappers[kfw_type].clock_idx[i] == UINT32_MAX)
			break;
		clock_gate(_akf_wrappers[kfw_type].clock_idx[i], false);
	}

done:
	exit_critical_section();

	return 0;
}

static void _akf_recv_mbox_nonempty_int_handler(void *arg)
{
	KFWRAPPER_TYPE_T kfw_type = (KFWRAPPER_TYPE_T)arg;
	
	RELEASE_ASSERT(kfw_type < KFW_TYPE_MAX);
	ASSERT(_akf_wrappers[kfw_type].base != 0);
	
	rAKF_AP_MAILBOX_SET(_akf_wrappers[kfw_type].base) = AKF_MAILBOX_O_NONEMPTY_MSK;
	// NOTE: this akf reg access will ensure that interrupt mask operation above
	// will complete before exiting interrupt handling - the read result is unimportant
	(void)rAKF_AP_INBOX_CTRL(_akf_wrappers[kfw_type].base);

	event_signal(&_akf_wrappers[kfw_type].recv_mbox_wait_event);
}

static void _akf_send_mbox_empty_int_handler(void *arg)
{
	KFWRAPPER_TYPE_T kfw_type = (KFWRAPPER_TYPE_T)arg;
	
	RELEASE_ASSERT(kfw_type < KFW_TYPE_MAX);
	ASSERT(_akf_wrappers[kfw_type].base != 0);

	rAKF_AP_MAILBOX_SET(_akf_wrappers[kfw_type].base) = AKF_MAILBOX_I_EMPTY_MSK;
	// NOTE: this akf reg access will ensure that interrupt mask operation above
	// will complete before exiting interrupt handling - the read result is unimportant
	(void)rAKF_AP_INBOX_CTRL(_akf_wrappers[kfw_type].base);

	event_signal(&_akf_wrappers[kfw_type].send_mbox_wait_event);
}


static void _akf_set_timebase(addr_t regs_base)
{
	uint32_t reg_value;
	uint64_t now;

	reg_value = rAKF_KIC_GLB_CFG(regs_base);

	enter_critical_section();
	rAKF_KIC_GLB_CFG(regs_base) = (reg_value & ~AKF_KIC_GLB_CFG_TEN);
	now = timer_get_ticks();
	rAKF_KIC_GLB_TIME_BASE_LO(regs_base) = (uint32_t)now;
	rAKF_KIC_GLB_TIME_BASE_HI(regs_base) = (uint32_t)(now>>32);
	rAKF_KIC_GLB_CFG(regs_base) = (reg_value | AKF_KIC_GLB_CFG_TEN);
	exit_critical_section();
}
