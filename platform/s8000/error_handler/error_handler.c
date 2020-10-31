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
#include <platform.h>
#include <platform/int.h>
#include <platform/error_handler.h>
#include <platform/soc/hwisr.h>
#include <platform/soc/miu.h>
#include SUB_PLATFORM_SPDS_HEADER(amcc)

#define rAMCC_INTEN(n)		(*(volatile uint32_t *)(AMCC_BASE_ADDR(n) + AMCC_IRERRDBG_INTEN_OFFSET))
#define rAMCC_INTSTATUS(n)	(*(volatile uint32_t *)(AMCC_BASE_ADDR(n) + AMCC_IRERRDBG_INTSTS_OFFSET))
#define rAMCC_MCCAFERRLOG2(n)	(*(volatile uint32_t *)(AMCC_BASE_ADDR(n) + AMCC_IRERRDBG_MCC_AF_ERR_LOG2_OFFSET))

// Enables all errors (the remaining bits are perf counters)
#define AMCC_IRERRDBG_INTEN_ALLERRORS		( AMCC_IRERRDBG_INTSTS_MCC_TP_PAR_ERR_UMASK \
						| AMCC_IRERRDBG_INTSTS_MCC_DAT_ONE_BIT_ERR_UMASK \
						| AMCC_IRERRDBG_INTSTS_MCC_DAT_MULTI_BITS_ERR_UMASK \
						| AMCC_IRERRDBG_INTSTS_MCC_SCE_CLR_DONE_UMASK \
						| AMCC_IRERRDBG_INTSTS_MCC_AF_ERR_UMASK \
						| AMCC_IRERRDBG_INTSTS_MCC_TP_MULTI_PAR_ERR_UMASK )

#define AMCC_INTSTATUS_ALLERRORS		AMCC_IRERRDBG_INTEN_ALLERRORS

#define AMCC_IERRDBG_INTSTS_DRAM_ONE_BIT_ERR	AMCC_IRERRDBG_INTSTS_MCC_DAT_ONE_BIT_ERR_UMASK
#define AMCC_IERRDBG_INTSTS_DRAM_MULTI_BIT_ERR	AMCC_IRERRDBG_INTSTS_MCC_DAT_MULTI_BITS_ERR_UMASK

#define AMCC_IRERRDBG_MCCAFERRLOG2_AGENT_IODAP	(0x87)

#define CP_COM_NORM_MSK_CLR_ALL 		(0xFFFFFFFF)
#define CP_COM_NORM_MSK_ILLEGAL_COH_INTS	((1<<0) | (1<<14))

static void amc_int_handler(void *arg)
{
	uint32_t	amcc = (uint32_t)(uintptr_t)arg;
	uint32_t	intsts = rAMCC_INTSTATUS(amcc);

	if ((intsts & (AMCC_IERRDBG_INTSTS_DRAM_ONE_BIT_ERR | AMCC_IERRDBG_INTSTS_DRAM_ONE_BIT_ERR)) == 0) {
		// Whitelist errors caused by the IODAP agent (Astris) since
		// these are most likely caused by user input errors.
		uint32_t agent_id = AMCC_IRERRDBG_MCC_AF_ERR_LOG2_MCC_AF_ERR_AID_LOG_XTRCT(rAMCC_MCCAFERRLOG2(amcc));
		if (agent_id == AMCC_IRERRDBG_MCCAFERRLOG2_AGENT_IODAP) {
			dprintf(DEBUG_INFO, "AMC error from IODAP (ignored): 0x%x\n", rAMCC_INTSTATUS(amcc));

			// Clear the interrupt and continue on.
			rAMCC_INTSTATUS(amcc) = intsts;
			return;
		}
	}

	panic("Received unexpected AMC error. AMC_IERRDBG_INTSTATUS = 0x%lx\n", rAMCC_INTSTATUS(amcc));
}

#if SUB_PLATFORM_S8000 || SUB_PLATFORM_S8003
static void cp_int_handler(void *arg __unused) {
	panic("Received unexpected Coherency Point error. CP_COM_INT_NORM_REQUEST = 0x%x\n", rCP_COM_INT_NORM_REQUEST);
}
#else
static void cp_int_handler(void *arg) {
	panic("Received unexpected Coherency Point %d error. CP_COM_INT_NORM_REQUEST = 0x%x\n", 
		(uint32_t)arg, 
		(((uint32_t)arg) == 0) ? rCP0_COM_INT_NORM_REQUEST : rCP1_COM_INT_NORM_REQUEST);
}
#endif

static void enable_cp_checks()
{
#if SUB_PLATFORM_S8000 || SUB_PLATFORM_S8003
	install_int_handler(INT_COHERENCE_PNT_ERR, cp_int_handler, NULL);
	rCP_COM_INT_NORM_MASK_CLR = CP_COM_NORM_MSK_CLR_ALL;
	unmask_int(INT_COHERENCE_PNT_ERR);
#elif SUB_PLATFORM_S8001
	install_int_handler(INT_COHERENCE_PNT0_ERR, cp_int_handler, (void*)0);
	rCP0_COM_INT_NORM_MASK_CLR = CP_COM_NORM_MSK_CLR_ALL;
	unmask_int(INT_COHERENCE_PNT0_ERR);
	
	install_int_handler(INT_COHERENCE_PNT1_ERR, cp_int_handler, (void*)1);
	rCP1_COM_INT_NORM_MASK_CLR = CP_COM_NORM_MSK_CLR_ALL;
	unmask_int(INT_COHERENCE_PNT1_ERR);
#endif
}

static void enable_amc_checks()
{
	for (unsigned i = 0; i < NUM_AMCCS; i++) {
		uint32_t vector;
#if NUM_AMCCS == 1
		vector = INT_MEM_CONTROLLER0;
#elif NUM_AMCCS == 2
		// Amusingly enough, INT_MEM_CONTROLLER1 is the 1st interrupt of the
		// 2nd AMCC on Elba, but the 2nd interrupt of the first/only AMCC on Maui
		if (i == 0) vector = INT_MEM_CONTROLLER0;
		else vector = INT_MEM_CONTROLLER1;
#else
#error "Unsupported number of AMCCs"
#endif
		install_int_handler(vector, amc_int_handler, (void *)(uintptr_t)i);
		rAMCC_INTEN(i) = AMCC_IRERRDBG_INTEN_ALLERRORS;
		if (rAMCC_INTSTATUS(i) & AMCC_INTSTATUS_ALLERRORS) {
			// AMC Interrupts, unlike the CP ones, don't seem to stick. If an error already exists
			// this early in bootup, we won't get an interrupt upon unmask.
			amc_int_handler((void *)(uintptr_t)i);
		}
		rAMCC_INTSTATUS(i) = AMCC_INTSTATUS_ALLERRORS;
		unmask_int(vector);
	}
}

void platform_enable_error_handler() {
	enable_amc_checks();
	enable_cp_checks();
}

