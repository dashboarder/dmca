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

#ifndef _APPLE_A7IOP_H
#define _APPLE_A7IOP_H

#include <sys/types.h>
#include <platform/soc/hwregbase.h>

/*******************************************
 * KFWrappers 
 *
********************************************/
typedef enum {
	KFW_ANS = 0,
	KFW_SIO,
	KFW_SEP,
	KFW_TYPE_MAX,
} KFWRAPPER_TYPE_T;

#define A7IOP_WAIT_FOREVER	(UINT32_MAX)
#define A7IOP_NO_WAIT		(0)

/*******************************************
 * A7IOP interface
 *
********************************************/

int32_t akf_start(const KFWRAPPER_TYPE_T kfw_type, const addr_t firmware_address, const uint64_t firmware_size);
int32_t akf_start_sep(void);
int32_t akf_send_mbox(const KFWRAPPER_TYPE_T kfw_type, const uint64_t msg, uint32_t wait_timeout) __attribute__((warn_unused_result));
int32_t akf_recv_mbox(const KFWRAPPER_TYPE_T kfw_type, uint64_t *msg, uint32_t wait_timeout) __attribute__((warn_unused_result));
int32_t akf_send_nmi(const KFWRAPPER_TYPE_T kfw_type);
int32_t akf_stop(const KFWRAPPER_TYPE_T kfw_type);


/*******************************************
 * KFWrapper registers
 *
 * Below are offsets, and bit definitions
 *
 ********************************************/

#if AKF_VERSION == 1

#define AKF_STRIDE				(0x1000)

#define rAKF_AXI_BASE(_b)			(*(volatile uint32_t *)((_b) + 0x0008))
#define rAKF_AXI_BASE_EXT(_b)			(*(volatile uint32_t *)((_b) + 0x000C))

#define rAKF_AXI_START(_b)			(*(volatile uint32_t *)((_b) + 0x0010))
#define rAKF_AXI_START_EXT(_b)			(*(volatile uint32_t *)((_b) + 0x0014))

#define rAKF_AXI_END(_b)			(*(volatile uint32_t *)((_b) + 0x0018))
#define rAKF_AXI_END_EXT(_b)			(*(volatile uint32_t *)((_b) + 0x001C))

#define rAKF_CPU_CTRL(_b)			(*(volatile uint32_t *)((_b) + 0x0028))

#define rAKF_KIC_GLB_TIME_BASE_LO(_b)		(*(volatile uint32_t *)((_b) + 0x0810))
#define rAKF_KIC_GLB_TIME_BASE_HI(_b)		(*(volatile uint32_t *)((_b) + 0x0818))

#define rAKF_KIC_TMR_CFG1(_b)			(*(volatile uint32_t *)((_b) + 0x0884))

#define rAKF_KIC_TMR_STATE_SET1(_b)		(*(volatile uint32_t *)((_b) + 0x08A4))

#elif AKF_VERSION == 2

#define AKF_STRIDE				(0x4000)

#define rAKF_AXI_BASE(_b)			(*(volatile uint32_t *)((_b) + 0x0008))
#define rAKF_AXI_BASE_EXT(_b)			(*(volatile uint32_t *)((_b) + 0x0010))

#define rAKF_AXI_START(_b)			(*(volatile uint32_t *)((_b) + 0x0018))
#define rAKF_AXI_START_EXT(_b)			(*(volatile uint32_t *)((_b) + 0x0020))

#define rAKF_AXI_END(_b)			(*(volatile uint32_t *)((_b) + 0x0028))
#define rAKF_AXI_END_EXT(_b)			(*(volatile uint32_t *)((_b) + 0x0030))

#define rAKF_CPU_CTRL(_b)			(*(volatile uint32_t *)((_b) + 0x0044))

#define rAKF_KIC_MAILBOX_EXT_SET(_b)		(*(volatile uint32_t *)((_b) + 0x0c00))
#define rAKF_KIC_MAILBOX_EXT_CLR(_b)		(*(volatile uint32_t *)((_b) + 0x0c04))

#define rAKF_KIC_GLB_TIME_BASE_LO(_b)		(*(volatile uint32_t *)((_b) + 0xc030))
#define rAKF_KIC_GLB_TIME_BASE_HI(_b)		(*(volatile uint32_t *)((_b) + 0xc038))

#define rAKF_KIC_TMR_CFG1(_b)			(*(volatile uint32_t *)((_b) + 0xc000))

#define rAKF_KIC_TMR_STATE_SET1(_b)		(*(volatile uint32_t *)((_b) + 0xc020))

#else
#error "Invalid or no AKF_VERSION"
// shut up additional errors
#define AKF_STRIDE				(0)
#endif

#define 	AKF_CPU_CTRL_RUN			(1<<4)

#define rAKF_KIC_GLB_CFG(_b)		(*(volatile uint32_t *)((_b) + 0x080c))
#define 	AKF_KIC_GLB_CFG_TEN			(1<<1)

#define 	AKF_KIC_TMR_CFG_FSL_TIMER	(0 << 4)
#define 	AKF_KIC_TMR_CFG_FSL_SW		(1 << 4)
#define 	AKF_KIC_TMR_CFG_FSL_EXTERNAL	(2 << 4)
#define 	AKF_KIC_TMR_CFG_SMD_FIQ		(0<<3)	
#define 	AKF_KIC_TMR_CFG_SMD_IRQ		(1<<3)
#define 	AKF_KIC_TMR_CFG_EMD_IRQ		(1<<2)
#define 	AKF_KIC_TMR_CFG_IMD_FIQ		(0<<1)	
#define 	AKF_KIC_TMR_CFG_IMD_IRQ		(1<<1)
#define 	AKF_KIC_TMR_CFG_EN		(1<<0)
#define 	AKF_KIC_TMR_CFG_NMI		(AKF_KIC_TMR_CFG_FSL_SW | AKF_KIC_TMR_CFG_SMD_FIQ  | AKF_KIC_TMR_CFG_IMD_FIQ | AKF_KIC_TMR_CFG_EN)

#define 	AKF_KIC_TMR_STATE_SET_SGT	(1<<0)

#define rAKF_AP_MAILBOX_SET(_b)			(*(volatile uint32_t *)((_b) + AKF_STRIDE + 0x000))
#define rAKF_AP_MAILBOX_CLR(_b)			(*(volatile uint32_t *)((_b) + AKF_STRIDE + 0x004))
#define 	AKF_MAILBOX_O_NONEMPTY_MSK	(1<<12)
#define 	AKF_MAILBOX_O_EMPTY_MSK		(1<<8)
#define 	AKF_MAILBOX_I_NONEMPTY_MSK	(1<<4)
#define 	AKF_MAILBOX_I_EMPTY_MSK		(1<<0)

#define rAKF_AP_INBOX_CTRL(_b)			(*(volatile uint32_t *)((_b) + AKF_STRIDE + 0x008))
#define 	AKF_AP_INBOX_CTRL_UDFL		(1<<19)
#define 	AKF_AP_INBOX_CTRL_OVFL		(1<<18)
#define 	AKF_AP_INBOX_CTRL_EMPTY		(1<<17)
#define 	AKF_AP_INBOX_CTRL_FULL		(1<<16)
#define 	AKF_AP_INBOX_CTRL_ENABLE	(1<<0)

// AKF "64-bit" register helpers.
__attribute__((always_inline)) inline void _akf_write_reg_64(addr_t low_addr, uint64_t value)
{
#if defined(__arm64__)
	(*(volatile uint64_t *)(low_addr)) = value;
#else
	uint32_t low = (uint32_t)(value & 0xFFFFFFFF);
	uint32_t high = (uint32_t)((value >> 32) & 0xFFFFFFFF);
	__asm__ __volatile__("strd  %0, %1, [%2]" : :  "r"(low), "r"(high) ,  "r"(low_addr));
#endif
}

__attribute__((always_inline)) inline uint64_t _akf_read_reg_64(addr_t low_addr)
{
#if defined(__arm64__)
	return (*(volatile uint64_t *)(low_addr));
#else
	uint32_t low, high;
	__asm__ __volatile__ ("ldrd  %0, %1, [%2]" : "=r"(low), "=r"(high) : "r"(low_addr));
	return ((uint64_t)high << 32) | low;
#endif
}

__attribute__((always_inline)) inline void akf_write_ap_inbox(addr_t akf_base, uint64_t value)
{
	_akf_write_reg_64(akf_base + AKF_STRIDE + 0x010, value);
}
#define rAKF_AP_INBOX_WR_L(_b)			(*(volatile uint32_t *)((_b) + AKF_STRIDE + 0x010))
#define rAKF_AP_INBOX_WR_H(_b)			(*(volatile uint32_t *)((_b) + AKF_STRIDE + 0x014))

__attribute__((always_inline)) inline uint64_t akf_read_ap_inbox(addr_t akf_base)
{
	return _akf_read_reg_64(akf_base + AKF_STRIDE + 0x018);
}
#define rAKF_AP_INBOX_RD(_b)			(*(volatile uint64_t *)((_b) + AKF_STRIDE + 0x018))
#define rAKF_AP_INBOX_RD_L(_b)			(*(volatile uint32_t *)((_b) + AKF_STRIDE + 0x018))
#define rAKF_AP_INBOX_RD_H(_b)			(*(volatile uint32_t *)((_b) + AKF_STRIDE + 0x01C))

#define rAKF_AP_OUTBOX_CTRL(_b)			(*(volatile uint32_t *)((_b) + AKF_STRIDE + 0x020))
#define 	AKF_AP_OUTBOX_CTRL_UDFL		(1<<19)
#define 	AKF_AP_OUTBOX_CTRL_OVFL		(1<<18)
#define 	AKF_AP_OUTBOX_CTRL_EMPTY	(1<<17)
#define 	AKF_AP_OUTBOX_CTRL_FULL		(1<<16)
#define 	AKF_AP_OUTBOX_CTRL_ENABLE	(1<<0)

__attribute__((always_inline)) inline void akf_write_ap_outbox(addr_t akf_base, uint64_t value)
{
	_akf_write_reg_64(akf_base + AKF_STRIDE + 0x030, value);
}
#define rAKF_AP_OUTBOX_WR_L(_b)			(*(volatile uint32_t *)((_b) + AKF_STRIDE + 0x030))
#define rAKF_AP_OUTBOX_WR_H(_b)			(*(volatile uint32_t *)((_b) + AKF_STRIDE + 0x034))

__attribute__((always_inline)) inline uint64_t akf_read_ap_outbox(addr_t akf_base)
{
	return _akf_read_reg_64(akf_base + AKF_STRIDE + 0x038);
}
#define rAKF_AP_OUTBOX_RD_L(_b)			(*(volatile uint32_t *)((_b) + AKF_STRIDE + 0x038))
#define rAKF_AP_OUTBOX_RD_H(_b)			(*(volatile uint32_t *)((_b) + AKF_STRIDE + 0x03C))

#endif /* _APPLE_A7IOP_H */
