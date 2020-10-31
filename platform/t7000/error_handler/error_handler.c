/*
 * Copyright (C) 2011-2013 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */
#include <debug.h>
#include <drivers/amc/amc.h>
#include <drivers/amc/amc_phy.h>
#include <drivers/amc/amc_regs.h>
#include <platform/int.h>
#include <platform/error_handler.h>
#include <platform/soc/hwisr.h>
#include <platform/soc/miu.h>

// Enables all errors (the remaining bits are perf counters)
#define AMCC_IRERRDBG_INTEN_ALLERRORS		((1 << 20) | (1 << 21) | (1 << 22) | \
						 (1 << 23) | (1 << 24) | (1 << 25))

#define AMCC_INTSTATUS_ALLERRORS		AMCC_IRERRDBG_INTEN_ALLERRORS

#define AMCC_IERRDBG_INTSTS_DRAM_ONE_BIT_ERR	(1 << 21)
#define AMCC_IERRDBG_INTSTS_DRAM_MULTI_BIT_ERR	(1 << 22)

#define AMCC_IRERRDBG_MCCAFERRLOG2_AGENT_MASK	(0xFF)
#define AMCC_IRERRDBG_MCCAFERRLOG2_AGENT_IODAP	(0x86)

#define CP_COM_NORM_MSK_CLR_ALL 		(0xFFFFFFFF)
#define CP_COM_NORM_MSK_ILLEGAL_COH_INTS	((1<<0) | (1<<14))

static void amc_int_handler(void *arg __unused)
{
	uint32_t	intsts = rAMC_INTSTATUS;

	if ((intsts & (AMCC_IERRDBG_INTSTS_DRAM_ONE_BIT_ERR | AMCC_IERRDBG_INTSTS_DRAM_ONE_BIT_ERR)) == 0) {
		// Whitelist errors caused by the IODAP agent (Astris) since
		// these are most likely caused by user input errors.
		uint32_t agent_id = rAMC_MCCAFERRLOG2 & AMCC_IRERRDBG_MCCAFERRLOG2_AGENT_MASK;
		if (agent_id == AMCC_IRERRDBG_MCCAFERRLOG2_AGENT_IODAP) {
			dprintf(DEBUG_INFO, "AMC error from IODAP (ignored): 0x%x\n", rAMC_INTSTATUS);

			// Clear the interrupt and continue on.
			rAMC_INTSTATUS = intsts;
			return;
		}
	}

	panic("Received unexpected AMC error. AMC_IERRDBG_INTSTATUS = 0x%x\n", rAMC_INTSTATUS);
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
	install_int_handler(INT_MEM_CONTROLLER, amc_int_handler, NULL);
	rAMC_INTEN = AMCC_IRERRDBG_INTEN_ALLERRORS;
	if (rAMC_INTSTATUS & AMCC_INTSTATUS_ALLERRORS) {
		// AMC Interrupts, unlike the CP ones, don't seem to stick. If an error already exists
		// this early in bootup, we won't get an interrupt upon unmask.
		amc_int_handler(NULL);
	}
	rAMC_INTSTATUS = AMCC_INTSTATUS_ALLERRORS;
	unmask_int(INT_MEM_CONTROLLER);
}

void platform_enable_error_handler() {
	enable_amc_checks();
	enable_cp_checks();
}

