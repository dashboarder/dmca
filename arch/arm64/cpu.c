/*
 * Copyright (C) 2011-2012, 2014 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */
#include <arch.h>
#include <arch/arm/arm.h>
#include <arch/arm64/proc_reg.h>
#include <debug.h>
#include <platform.h>
#include <platform/memmap.h>

extern void arm_write_sctlr(uint64_t);
extern uint64_t arm_read_sctlr();
extern void arm_enable_async_aborts();
#if	WITH_EL3
extern void arm_write_scr(uint64_t);
#endif

int arch_cpu_init(bool resume)
{
	uint64_t sctlr;

#if	WITH_EL3
	uint64_t scr;

	/* enable interrupts forwarding to EL3 */
	scr = (SCR_EA | SCR_FIQ | SCR_IRQ);
	arm_write_scr(scr);
#endif

	/* enable async aborts */
	arm_enable_async_aborts();

	/* invalidate caches */
	arm_invalidate_icache();
	arm_invalidate_dcache();

	sctlr = arm_read_sctlr();

	/* turn on Stack Alignment check */
	sctlr |= SCTLR_SA_ENABLED;

	/* turn on the mmu */
	arm_mmu_init(resume);
	sctlr |= SCTLR_M_ENABLED;

	/* disallow executing from writeable pages */
	sctlr |= SCTLR_WXN_ENABLED;

	/* turn on d-cache */
	sctlr |= SCTLR_D_ENABLED;

	/* turn on i-cache */
	sctlr |= SCTLR_I_ENABLED;

	/* enable the MMU, caches and etc. */
	arm_write_sctlr(sctlr);

#if WITH_VFP
	/* initialize VFP */
	arm_fp_init();
#endif

	return 0;
}

int arch_cpu_quiesce(void)
{
#if !CPU_APPLE_CYCLONE
	uint64_t sctlr;

	/* clean the d-cache */
	arm_clean_dcache();

	/* read the control regiser base value */
	sctlr = arm_read_sctlr();

	sctlr &= ~(SCTLR_D_ENABLED | SCTLR_SA_ENABLED | SCTLR_I_ENABLED | SCTLR_M_ENABLED | SCTLR_WXN_ENABLED);

	/* disable the MMU, caches and etc. */
	arm_write_sctlr(sctlr);

	/* invalidate i-cache */
	arm_invalidate_icache();
#endif /* !CPU_APPLE_CYCLONE */

	return 0;
}

int arch_cpu_init_posttasks(void)
{
	return 0;
}

