/*
 * Copyright (C) 2007-2013 Apple Inc. All rights reserved.
 * Copyright (C) 2006 Apple Computer, Inc. All rights reserved.
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
#include <debug.h>
#include <platform.h>
#include <platform/memmap.h>

#if PERIPH_PORT_REMAP
static void config_periph_port_remap(void *base, size_t len);
#endif

#if WITH_EARLY_ICACHE && WITH_L1_PARITY
#error Early i-cache init and L1 parity enable are mutually exclusive
#endif

int arch_cpu_init(bool resume)
{
	u_int32_t cr, aux_cr;

	aux_cr = 0;

#if WITH_CACHE
	/* make sure the cpu caches are invalidated */
#if !WITH_EARLY_ICACHE
    /* d-cache init is deferred if we're turning i-cache on early */
	arm_invalidate_dcache();
#endif
	arm_invalidate_icache();
#endif

#if ARCH_ARMv7
	aux_cr = arm_read_aux_cr();

#if CPU_ARM_CORTEX_A8
	// Ask platform if WFI should be enabled or not
	if (platform_get_core_idle_enable()) {
		aux_cr &= ~(1 << 8);
	} else {
		aux_cr |= (1 << 8);
	}
#endif

#if WITH_ARCHITECTED_L2
	/* Manage the v7 L2 cache */
# if WITH_AUX_L2_ENABLE_BIT && L2_CACHE_SIZE
	aux_cr |= (1 << 1);
# else
	aux_cr &= ~(1 << 1);
# endif
#endif

#if SET_AUX_SMP_BIT
	/* Set SMP bit; enable snooping within cluster and sharing of L2 */
	aux_cr |= (1 << 6);
#endif

	arm_write_aux_cr(aux_cr);
#endif

	/* read control register default value */
	cr = arm_read_cr();

#if WITH_BRANCH_PREDICTION
	/* turn on branch predictor */
	cr |= (1 << 11);
#endif

#if WITH_CACHE && WITH_EARLY_ICACHE
	/* early i-cache enable */
	cr |= (1 << 12);
	
	arm_write_cr(cr);
#endif

#if PERIPH_PORT_REMAP
	/* do any peripheral port remapping */
	config_periph_port_remap((void *)PERIPH_BASE, PERIPH_LEN);
#endif

#if WITH_CACHE

#if WITH_EARLY_ICACHE
    /* d-cache invalidation delayed until this point */
	arm_invalidate_dcache();
#else
	/* turn on i-cache */
	cr |= (1 << 12);
#endif

	/* turn on d-cache */
	cr |= (1 << 2);
	
#if WITH_L1_PARITY
	arm_enable_l1parity();
#endif
#endif

#if WITH_UNALIGNED_MEM
	/* enable unaligned access handler */
	cr &= ~(1 << 1);
#if !ARCH_ARMv7
	cr |= (1 << 22);
#endif
#endif

	/* enable the caches and etc. before setting up the translation table */
	arm_write_cr(cr);

#if WITH_MMU
	/* set up the translation table and then turn on the MMU */
	arm_mmu_init(resume);
	cr |= (1 << 0);

	arm_write_cr(cr);
#endif

#if WITH_VFP
	/* initialize VFP */
	arm_fp_init();
#if WITH_VFP_ALWAYS_ON	
	arm_fp_enable(true);
#endif	
#endif

	return 0;
}

int arch_cpu_quiesce(void)
{
	u_int32_t cr, aux_cr;

	aux_cr = 0;

#if ARCH_ARMv7
	aux_cr = arm_read_aux_cr();

#if CPU_ARM_CORTEX_A8
	/* turn WFI on */
	aux_cr &= ~(1 << 8);
#endif

	arm_write_aux_cr(aux_cr);
#endif

#if WITH_CACHE && !CLEAN_INV_CACHE_ON_QUIESCE
	/* clean the d-cache */
	arm_clean_dcache();
#endif
	
	/* read the control regiser base value */
	cr = arm_read_cr();

#if WITH_MMU
	/* turn off the mmu */
	cr &= ~(1 << 0);
#endif
#if WITH_CACHE	
	/* turn off d-cache */
	cr &= ~(1 << 2);

	/* turn off i-cache */
	cr &= ~(1 << 12);
#endif

#if WITH_BRANCH_PREDICTION
        /* turn off branch predictor */
        cr &= ~(1 << 11);
#endif

#if WITH_UNALIGNED_MEM
        /* disable unaligned access handler */
        cr |= (1 << 1);
        cr &= ~(1 << 22);
#endif

	/* disable the MMU, caches and etc. */
	arm_write_cr(cr);
	
#if WITH_CACHE && CLEAN_INV_CACHE_ON_QUIESCE
	/* clean and/or invalidate the caches */
	arm_clean_invalidate_dcache();
	arm_invalidate_icache();
#endif		

#if CPU_APPLE_SWIFT

#if !CLEAN_INV_CACHE_ON_QUIESCE
	/* Invalidate all lines in the L1-D cache */
	arm_invalidate_dcache();
#endif

	/* Clear any pending L2C errors */
	swift_write_l2cerrsts(0);
	swift_write_l2cerradr(0);
#endif /* CPU_APPLE_SWIFT */

	return 0;
}

int arch_cpu_init_posttasks(void)
{
#if WITH_ARM_DCC
	/* initialize the DCC mechanism */
	arm_dcc_init();
#endif

	return 0;
}

u_int32_t arch_get_noncached_address(u_int32_t address)
{
#if WITH_MMU
	return arm_get_noncached_address(address);
#else
	return address; /* not really any other choice... */
#endif
}

#if PERIPH_PORT_REMAP
static void config_periph_port_remap(void *base, size_t len)
{
	uint32_t val = (uint32_t)base;

	switch (len) {
		case 0: val |= 0x0; break;
		case 0x1000: val |= 0x3; break;
		case 0x2000: val |= 0x4; break;
		case 0x4000: val |= 0x5; break;
		case 0x8000: val |= 0x6; break;
		case 0x10000: val |= 0x7; break;
		case 0x20000: val |= 0x8; break;
		case 0x40000: val |= 0x9; break;
		case 0x80000: val |= 0xa; break;
		case 0x100000: val |= 0xb; break;
		case 0x200000: val |= 0xc; break;
		case 0x400000: val |= 0xd; break;
		case 0x800000: val |= 0xe; break;
		case 0x1000000: val |= 0xf; break;
		case 0x2000000: val |= 0x10; break;
		case 0x4000000: val |= 0x11; break;
		case 0x8000000: val |= 0x12; break;
		case 0x10000000: val |= 0x13; break;
		case 0x20000000: val |= 0x14; break;
		case 0x40000000: val |= 0x15; break;
		case 0x80000000: val |= 0x16; break;
	}
	
	arm_write_perip_port_remap(val);
}
#endif

static bool cpu_setup_mmu(bool resume)
{
#if WITH_MMU
	arm_mmu_init(resume);
	return true;
#endif

#if WITH_MPU
	arm_mpu_init();
	return true;
#endif

	return false;
}

