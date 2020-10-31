/*
 * Copyright (C) 2011-2015 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */
#include <debug.h>
#include <platform/int.h>
#include <platform/error_handler.h>
#include <platform/soc/hwisr.h>
#include <platform/soc/hwregbase.h>
#include <platform/soc/miu.h>

// All errors (the remaining bits are perf counters)
#define AMCC_IRERRDBG_INTEN_ALLERRORS		(								\
						  AMCC_IRERRDBG_INTSTS_MCC_TP_PAR_ERR_UMASK			\
						| AMCC_IRERRDBG_INTSTS_MCC_DAT_ONE_BIT_ERR_UMASK		\
						| AMCC_IRERRDBG_INTSTS_MCC_DAT_MULTI_BITS_ERR_UMASK		\
						| AMCC_IRERRDBG_INTSTS_MCC_AF_ERR_UMASK				\
						| AMCC_IRERRDBG_INTSTS_MCC_TP_MULTI_PAR_ERR_UMASK		\
						| AMCC_IRERRDBG_INTSTS_MCS_LS3_B_ALLOC_PAR_ERR_UMASK		\
						| AMCC_IRERRDBG_INTSTS_MCS_LS3_B_RESP_PAR_ERR_UMASK		\
						| AMCC_IRERRDBG_INTSTS_MCS_LS3_B_TOKEN_PAR_ERR_UMASK		\
						| AMCC_IRERRDBG_INTSTS_MCS_LS3_B_DATA_PULL_PAR_ERR_UMASK	\
						)

#define AMCC_IRERRDBG_INTSTS_ALLERRORS		AMCC_IRERRDBG_INTEN_ALLERRORS

#define AMCC_IRERRDBG_MCC_AF_ERR_LOG2_AGENT_IODAP	(0x88)

#define CP_COM_NORM_MSK_CLR_ALL 		(~0)
#define CP_COM_NORM_MSK_ILLEGAL_COH_INTS	(									\
						  CP_COM_INT_NORM_MASK_SET_CCU0_ILLEGAL_PIO_ACCESS_INT_MSK_SET_UMASK	\
						| CP_COM_INT_NORM_MASK_SET_CCU1_ILLEGAL_PIO_ACCESS_INT_MSK_SET_UMASK	\
						)

#if NUM_AMCCS > 1
#error "amc_int_handler() must be recoded for NUM_AMCCS > 1"
#endif

static void amc_int_handler(void *arg __unused)
{
	uint32_t intsts = rAMCC_IRERRDBG_INTSTS(0);

	if ((intsts & (AMCC_IRERRDBG_INTEN_ALLERRORS & ~AMCC_IRERRDBG_INTSTS_MCC_AF_ERR_UMASK)) == 0) {
		// Whitelist errors caused by the IODAP agent (Astris) since
		// these are most likely caused by user input errors.
		uint32_t agent_id = AMCC_IRERRDBG_MCC_AF_ERR_LOG2_MCC_AF_ERR_AID_LOG_XTRCT(rAMCC_IRERRDBG_MCC_AF_ERR_LOG2(0));
		if (agent_id == AMCC_IRERRDBG_MCC_AF_ERR_LOG2_AGENT_IODAP) {
			dprintf(DEBUG_INFO, "AMC error from IODAP (ignored): 0x%x\n", intsts);

			// Clear the interrupt and continue on.
			rAMCC_IRERRDBG_INTSTS(0) = intsts;
			return;
		}
	}

	panic("Received unexpected AMC error. AMCC_IRERRDBG_INTSTS = 0x%x\n", intsts);
}

static void cp_int_handler(void *arg __unused) {
	panic("Received unexpected Coherency Point error. CP_COM_INT_NORM_REQUEST = 0x%x\n", rCP_COM_INT_NORM_REQUEST);
}

static void enable_cp_checks()
{
	install_int_handler(INT_COHERENCE_PNT_ERR, cp_int_handler, NULL);
	rCP_COM_INT_NORM_MASK_CLR = CP_COM_NORM_MSK_CLR_ALL;
	unmask_int(INT_COHERENCE_PNT_ERR);
}

static void enable_amc_checks()
{
	install_int_handler(INT_MEM_CONTROLLER0, amc_int_handler, NULL);
	rAMCC_IRERRDBG_INTEN(0) = AMCC_IRERRDBG_INTEN_ALLERRORS;
	if (rAMCC_IRERRDBG_INTSTS(0) & AMCC_IRERRDBG_INTSTS_ALLERRORS) {
		// AMC Interrupts, unlike the CP ones, don't seem to stick. If an error already exists
		// this early in bootup, we won't get an interrupt upon unmask.
		amc_int_handler(NULL);
	}
	rAMCC_IRERRDBG_INTSTS(0) = AMCC_IRERRDBG_INTSTS_ALLERRORS;
	unmask_int(INT_MEM_CONTROLLER0);
}

void platform_enable_error_handler() {
	enable_amc_checks();
	enable_cp_checks();
}

