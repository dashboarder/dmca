/*
 * Copyright (C) 2006-2011 Apple Computer, Inc. All rights reserved.
 *
 * This document is the property of Apple Computer, Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Computer, Inc.
 */
#include <debug.h>
#include <compiler.h>
#include <sys.h>
#include <arch.h>
#include <arch/arm/arm.h>
#include <platform.h>
#include <platform/memmap.h>

#define SECTION_SIZE MB
#define LARGE_PAGE_SIZE 65536
#define PAGE_SIZE 4096

#define TTBCR_PD1 (1 << 5)

typedef u_int32_t tt_ent;

/* needs to be in non-static, initialized data for successful patching
 * during IOP firmware loading */
u_int32_t tt_size __attribute__((used)) = MMU_TT_SIZE;
tt_ent *tt = (void *)0xffffffff;

/* ARM MMU first-level section descriptor definitions */
#define ARM_DESC_SECT		((0 << 18) | (1 << 1))
#define ARM_SECT_B		(1 << 2)
#define ARM_SECT_C		(1 << 3)
#define ARM_SECT_XN		(1 << 4)
#define ARM_SECT_DOMAIN(_d)	((_d) << 5)
#define ARM_SECT_TEX(_t)	((_t) << 12)
#define ARM_SECT_SHARED		(1 << 16)
#define ARM_SECT_FULLACC	((0 << 15) | (3 << 10))
#define ARM_SECT_EXEC		(ARM_SECT_DOMAIN(0))
#define ARM_SECT_NOEXEC		(ARM_SECT_DOMAIN(1) | ARM_SECT_XN)
#define ARM_SECT_READ_ONLY	(1 << 15)

/* Non-normal memory needs XN set (and consequently a client domain assignment) to avoid
 * instruction prefetch to those regions */
static u_int32_t sect_flags[] = {
	[kARMMMUStronglyOrdered]           = ARM_SECT_NOEXEC | ARM_SECT_FULLACC,
	[kARMMMUDevice]                    = ARM_SECT_NOEXEC | ARM_SECT_B | ARM_SECT_FULLACC,
	[kARMMMUNormal]                    = ARM_SECT_EXEC | ARM_SECT_C | ARM_SECT_B | ARM_SECT_FULLACC,
	[kARMMMUInnerNormalOuterNoncached] = ARM_SECT_EXEC | ARM_SECT_TEX(4) | ARM_SECT_C | ARM_SECT_B | ARM_SECT_FULLACC,
	[kARMMMUWriteCombined]             = ARM_SECT_NOEXEC | ARM_SECT_TEX(1) | ARM_SECT_FULLACC,	
	[kARMMMUNormalRX]                  = ARM_SECT_DOMAIN(1) | ARM_SECT_C | ARM_SECT_B | ARM_SECT_READ_ONLY | ARM_SECT_FULLACC,
};

static tt_ent attr_to_sect_flags(arm_mmu_attr_t attr)
{
	if (attr >= kARMMMUNumAttr)
		attr = kARMMMUStronglyOrdered;
	return sect_flags[attr];
}

void arm_mmu_map_section(addr_t vaddr, addr_t paddr, arm_mmu_attr_t attr, bool shared)
{
	if ((vaddr & (SECTION_SIZE - 1)) != 0)
		panic("section virtual address base is nonzero");
	if ((paddr & (SECTION_SIZE - 1)) != 0)
		panic("section physical address base is nonzero");

	tt[vaddr / MB] = paddr | ARM_DESC_SECT | attr_to_sect_flags(attr) | (shared ?  ARM_SECT_SHARED : 0);

	arm_clean_dcache_line((unsigned long)&tt[vaddr / MB]);
	arm_flush_tlbs();
}

void arm_mmu_map_section_range(addr_t vaddr, addr_t paddr, size_t sections, arm_mmu_attr_t attr, bool shared, bool flush)
{
	tt_ent		entry, *tt_ptr;
	addr_t		flush_start, flush_stop;
	u_int32_t	cnt;

	if ((vaddr & (SECTION_SIZE - 1)) != 0)
		panic("section range virtual address base is nonzero");
	if ((paddr & (SECTION_SIZE - 1)) != 0)
		panic("section range physical address base is nonzero");

	entry  = paddr | ARM_DESC_SECT | attr_to_sect_flags(attr) | (shared ?  ARM_SECT_SHARED : 0);
	tt_ptr = &tt[vaddr / MB];

	while (sections > 0) {
		cnt = (sections < 16) ? sections : 16;
		switch (cnt) {
			case 16 : tt_ptr[15] = entry + MB * 15;
			case 15 : tt_ptr[14] = entry + MB * 14;
			case 14 : tt_ptr[13] = entry + MB * 13;
			case 13 : tt_ptr[12] = entry + MB * 12;
			case 12 : tt_ptr[11] = entry + MB * 11;
			case 11 : tt_ptr[10] = entry + MB * 10;
			case 10 : tt_ptr[ 9] = entry + MB *  9;
			case  9 : tt_ptr[ 8] = entry + MB *  8;
			case  8 : tt_ptr[ 7] = entry + MB *  7;
			case  7 : tt_ptr[ 6] = entry + MB *  6;
			case  6 : tt_ptr[ 5] = entry + MB *  5;
			case  5 : tt_ptr[ 4] = entry + MB *  4;
			case  4 : tt_ptr[ 3] = entry + MB *  3;
			case  3 : tt_ptr[ 2] = entry + MB *  2;
			case  2 : tt_ptr[ 1] = entry + MB *  1;
			case  1 : tt_ptr[ 0] = entry + MB *  0;
		}

		entry	 += MB * cnt;
		tt_ptr	 += cnt;
		sections -= cnt;
	}

	if (flush) {
		flush_start = ((addr_t)&tt[vaddr / MB]) & ~CPU_CACHELINE_SIZE;
		flush_stop  = ((addr_t)tt_ptr + (CPU_CACHELINE_SIZE - 1)) & ~CPU_CACHELINE_SIZE;

		while (flush_start < flush_stop) {
			arm_clean_dcache_line(flush_start);

			flush_start += CPU_CACHELINE_SIZE;
		}

		arm_flush_tlbs();
	}
}

void arm_mmu_init(bool resume)
{
#ifdef MMU_TT_BASE
	/* TT usually at the end of memory */
	tt = (tt_ent *)MMU_TT_BASE;
#endif

	if (false == resume) {
		/* identity map everything using the translation table as noncached */
		arm_mmu_map_section_range(0, 0, MMU_TT_SIZE / sizeof(tt_ent), kARMMMUStronglyOrdered, false, false);
	}

	/* let the platform code get a shot at mapping some ranges */
	platform_mmu_setup(resume);

	/* set up the domain register */
	arm_write_dar(0x7); /* domain 0 manager, domain 1 client */

	/* write the new translation table */
#if MMU_TT_SIZE == 0x4000
        arm_write_ttbcr(0);
#elif (MMU_TT_SIZE == 0x1000) && (WITH_MMU_SECURITY_EXTENSIONS)
        arm_write_ttbcr(2 | TTBCR_PD1);
#else
#error "Must set up TTBCR appropriately for translation table smaller than 16 kB"
#endif
	arm_write_ttb(tt);

	arm_flush_tlbs();
}

u_int32_t arm_get_noncached_address(u_int32_t address)
{
#ifdef MMU_NONCACHE0_SIZE
	if (((int32_t)(address - MMU_NONCACHE0_PBASE) >= 0) && ((address - MMU_NONCACHE0_PBASE) < MMU_NONCACHE0_SIZE)) {
	   	return address - MMU_NONCACHE0_PBASE + MMU_NONCACHE0_VBASE;
       }
#endif

#ifdef MMU_NONCACHE1_SIZE
	if (((int32_t)(address - MMU_NONCACHE1_PBASE) >= 0) && ((address - MMU_NONCACHE1_PBASE) < MMU_NONCACHE1_SIZE)) {
	   	return address - MMU_NONCACHE1_PBASE + MMU_NONCACHE1_VBASE;
       }
#endif

	return 0xFFFFFFFF;
}
