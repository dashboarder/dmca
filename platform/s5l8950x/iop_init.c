/*
 * Copyright (C) 2010 Apple Inc. All rights reserved.
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
#include <drivers/a5iop/a5iop.h>
#include <platform.h>
#include <platform/clocks.h>
#include <platform/int.h>
#include <platform/memmap.h>
#include <platform/timer.h>
#include <platform/soc/hwisr.h>
#include <platform/soc/pmgr.h>
#include <sys.h>
#include <sys/boot.h>
#include <sys/callout.h>

/*
 * Platform memory layout.
 *
 * The IOP sees SDRAM at its physical location uncached, and has an cached aperture
 * visible elsewhere.  SDRAM size of up to 1GB is currently supported.
 * SRAM is not visible/legally usable.
 *
 * For some platforms/targets, SDRAM_LEN is not constant and so we 
 * have to compute a maximum via other means.
 *
 * Note that the AE2 Sparrow shares this code.
 */
#define MEM_CACHED_BASE		(SDRAM_BASE | 0xc0000000)
#define SDRAM_MAX_LEN		(SDRAM_BANK_LEN * SDRAM_BANK_COUNT)
struct mem_static_map_entry mem_static_map_entries[] = {
        {
                MEM_CACHED_BASE,                        /* cached */
                SDRAM_BASE,                             /* uncached */
                SDRAM_BASE,                             /* physical */
                SDRAM_MAX_LEN                           /* length */
        },
        {MAP_NO_ENTRY, MAP_NO_ENTRY, MAP_NO_ENTRY, 0}
};

void platform_mmu_setup(bool resume)
{
	if (false == resume) {
		/* Note: sharing settings aren't critical, as SCC doesn't see the AxUSER bits
		 * and the AxCACHE widget does all the heavy lifting. */

		/* Remap text base to zero, outer uncacheable, shared */
		arm_mmu_map_section(0, TEXT_BASE, kARMMMUInnerNormalOuterNoncached, true);
#if MAP_SRAM_CACHED
		/* Map the SRAM as cacheable, shared */
		arm_mmu_map_section(SRAM_BASE, SRAM_BASE, kARMMMUNormal, true);
#endif // MAP_SRAM_CACHED
		/* Remap all of SDRAM through the cacheable aperture, outer uncacheable, shared */
		arm_mmu_map_section_range(MEM_CACHED_BASE, SDRAM_BASE, ROUNDUP(SDRAM_BANK_LEN * SDRAM_BANK_COUNT, MB)/MB,
					  kARMMMUInnerNormalOuterNoncached, true, false);
		/* Remap all of SDRAM through the uncacheable aperture (existing mappings should be there, but be sure) */
		arm_mmu_map_section_range(SDRAM_BASE, SDRAM_BASE, ROUNDUP(SDRAM_BANK_LEN * SDRAM_BANK_COUNT, MB)/MB,
					  kARMMMUStronglyOrdered, false, false);
	}
}

bool platform_get_production_mode(void)
{
	return(true);
}

bool platform_get_secure_mode(void)
{
	return(true);
}

void platform_reset(bool panic)
{
	for (;;)
		;
}

/*
 * Early init is done every time the IOP starts.
 */
int
platform_early_init()
{
#if PLATFORM_VARIANT_IOP
	/* In general we want power-gating to be disabled. */
	rPMGR_PWR_GATE_CTL_CLR = PMGR_PWR_GATE_IOP;
	while (rPMGR_PWR_GATE_CTL_CLR & PMGR_PWR_GATE_IOP) ;
#endif

	interrupt_init();

	timer_init(0);
	
	return(0);
}

/*
 * Halt waiting for an interrupt.
 */
void
platform_halt(void)
{
	arch_halt();
}

#if SUPPORT_SLEEP

#if WITH_IOP_POWER_GATING

/*
 * Deep idle sleep with power gating
 */
void
platform_deep_idle(void)
{
	rPMGR_PWR_GATE_CTL_SET = PMGR_PWR_GATE_IOP;
	while (!(rPMGR_PWR_GATE_CTL_SET & PMGR_PWR_GATE_IOP)) ;
	a5iop_sleep(1);
	rPMGR_PWR_GATE_CTL_CLR = PMGR_PWR_GATE_IOP;
	while (rPMGR_PWR_GATE_CTL_CLR & PMGR_PWR_GATE_IOP) ;
}

#endif

void
platform_sleep(void)
{
	/* We assume power-gating is disabled, which it need to be so
	 * the IOP wrapper registers don't disappear. */
	a5iop_sleep(0);
}

#endif

/*
 * Cache control
 */
void
platform_cache_operation(int operation, void *address, u_int32_t length)
{
	a5iop_cache_operation(operation, address, length);
}

/*
 * Host doorbell uses a SWI in the AIC.
 */
static void	(* _doorbell_handler)(void *arg);
static void
platform_doorbell_handler(void *arg)
{
	/* clear the doorbell before passing up */
	interrupt_clear_ipc(INT_IOP);
	_doorbell_handler(arg);
}

void
platform_init_iop_doorbell(void (* handler)(void *arg), void *arg)
{
	_doorbell_handler = handler;
	install_int_handler(INT_IOP, platform_doorbell_handler, arg);
	set_int_type(INT_IOP, INT_TYPE_IRQ | INT_TYPE_LEVEL);
	unmask_int(INT_IOP);
}

void
platform_mask_doorbell(void)
{
	mask_int(INT_IOP);
}

void
platform_unmask_doorbell(void)
{
	unmask_int(INT_IOP);
}

void
platform_ring_host_doorbell(void)
{
	interrupt_generate_ipc(INT_HOST);
}

int
platform_init_nmi(void (*handler)(void *arg), void *arg)
{
#ifdef INT_IOP_NMI
	install_int_handler(INT_IOP_NMI, handler, arg);
	set_int_type(INT_IOP_NMI, INT_TYPE_FIQ | INT_TYPE_LEVEL);
	unmask_int(INT_IOP_NMI);
#endif

	return(0);
}

/*
 * Clock management.
 *
 * Currently the IOP does none for this platform.
 */
void clock_gate(int device, bool enable)
{
}
