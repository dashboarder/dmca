/*
 * Copyright (C) 2012-2014 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */

/*
 * See docs/mmu64.txt for an in-depth version of what is going on here.
 * Short version:
 * - H6, H7 use 4KB page granule.
 * - H8 uses 16KB page granule (4KB not supported by hardware).
 * - Start at L1 for 4KB page granule.
 * - Start at L2 for 16KB page granule.
 * - Set virtual memory limit so the first level (L0/L1) has just two entries.
 * - ...then never touch the top level so we don't suffer from NS/S coherency.
 */

#include <debug.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arch/arm/arm.h>
#include <arch/arm64/proc_reg.h>
#ifndef TEST
# include <sys.h>
# include <arch.h>
# include <platform.h>
# include <platform/memmap.h>
#endif

#define LAST_TABLE_LEVEL	3

extern uint64_t arm_read_sctlr();

// Start level page table entries
static uint64_t *tt_top_level;

// Allocation area and current allocation pointer.
static addr_t tt_alloc_start, tt_alloc_next, tt_alloc_end;

#ifdef TEST

// Mock test interfaces for mmu.c, implemented in mmu_test.c

void mmu_printf(int verbosity, const char *fmt, ...);
unsigned get_page_granule_shift(void);
void mmu_get_memmap(addr_t *page_tables_base,
		    size_t *page_tables_size);
void platform_mmu_setup(bool resume);
void get_table_entry_hook(unsigned level, addr_t table_base, size_t index);
void set_table_entry_hook(unsigned level, addr_t table_base, size_t index,
			  uint64_t value);

#else  // !defined(TEST)

// Real interfaces, designed to be inlined. These have no host-side coverage,
// so they should be no-ops or simple constants.

// No printf spew.
static void mmu_printf(int verbosity, const char *fmt, ...)
{
	(void) verbosity;
	(void) fmt;
}

// Constants.
static unsigned get_page_granule_shift(void) { return PAGE_GRANULE_SHIFT; }
static void mmu_get_memmap(addr_t *page_tables_base,
			   size_t *page_tables_size)
{
	*page_tables_base = PAGE_TABLES_BASE;
	*page_tables_size = PAGE_TABLES_SIZE;
}

// Remove expensive validity checking.
static void get_table_entry_hook(unsigned level, addr_t table_base,
				 size_t index)
{
	(void) level; (void) table_base; (void) index;
}
static void set_table_entry_hook(unsigned level, addr_t table_base,
				 size_t index, uint64_t value)
{
	(void) level; (void) table_base; (void) index; (void) value;
}

#endif  // !defined(TEST)

#define mmu_spew(x...) mmu_printf(1, x)
#define mmu_extra_spew(x...) mmu_printf(2, x)

static void assert_valid_level(unsigned level)
{
	ASSERT((get_page_granule_shift() == 12 || level >= 1) &&
	       level <= LAST_TABLE_LEVEL);
}

static bool level_permits_blocks(unsigned level)
{
	assert_valid_level(level);
	// No granule supports L0 blocks.
	// 4KB granule supports L1 blocks.
	// All granules support L2 and L3 blocks.
	return (level == 1 && get_page_granule_shift() == 12) || level >= 2;
}

static uint64_t get_table_entry(unsigned level, addr_t table_base,
				size_t index)
{
	get_table_entry_hook(level, table_base, index);
	mmu_extra_spew("get_table_entry(%u, 0x%llx, %u) ",
		       level, (unsigned long long) table_base,
		       (unsigned) index);
	// No possible aliases, known aligned.
	uint64_t result = *(uint64_t *) (table_base + index * 8);
	mmu_extra_spew("= 0x%016llx\n", (unsigned long long) result);
	return result;
}

static void set_table_entry(unsigned level, addr_t table_base, size_t index,
			    uint64_t value)
{
	set_table_entry_hook(level, table_base, index, value);
	mmu_extra_spew("set_table_entry(%u, 0x%llx, %u, 0x%llx)\n",
		       level, (unsigned long long) table_base, (unsigned) index,
		       (unsigned long long) value);
	*((volatile uint64_t *) (table_base + index * 8)) = value;
}

static unsigned get_start_level(void)
{
	switch (get_page_granule_shift()) {
	case 12: return 1;
	case 14: return 2;
	case 16: return 2;
	default: panic("Bad granule");
	}
}

static unsigned get_virt_address_bits(void)
{
	switch (get_page_granule_shift()) {
	case 12: return 36;
	case 14: return 36;
	case 16: return 36;
	default: panic("Bad granule");
	}
}

static uint64_t get_mair(void)
{
	return (MAIR_WRITEBACK << MAIR_ATTR_SHIFT(CACHE_ATTRINDX_WRITEBACK)) |
		(MAIR_DISABLE << MAIR_ATTR_SHIFT(CACHE_ATTRINDX_DISABLE));
}

static uint64_t get_tcr(void)
{
	// Form the contents of the TCR register for the selected
	// virtual address limit and page granule.
	uint64_t tcr;
	uint32_t txsz = 64 - get_virt_address_bits();
#if WITH_EL3
	switch (get_page_granule_shift()) {
	case 12: tcr = TCR_ELx_TG0_GRANULE_4KB; break;
	case 14: tcr = TCR_ELx_TG0_GRANULE_16KB; break;
	case 16: tcr = TCR_ELx_TG0_GRANULE_64KB; break;
	default: panic("Bad granule");
	}
	tcr |=	TCR_EL3_PS_36BITS |
		TCR_ELx_SH0_OUTER | TCR_ELx_ORGN0_WRITEBACK | TCR_ELx_IRGN0_WRITEBACK |
		(txsz << TCR_ELx_T0SZ_SHIFT);
#else
	switch (get_page_granule_shift()) {
	case 12: tcr = TCR_ELx_TG0_GRANULE_4KB  | TCR_EL1_TG1_GRANULE_4KB;  break;
	case 14: tcr = TCR_ELx_TG0_GRANULE_16KB | TCR_EL1_TG1_GRANULE_16KB; break;
	case 16: tcr = TCR_ELx_TG0_GRANULE_64KB | TCR_EL1_TG1_GRANULE_64KB; break;
	default: panic("Bad granule");
	}
	tcr |=	TCR_EL1_IPS_36BITS |
		TCR_ELx_SH0_OUTER | TCR_ELx_ORGN0_WRITEBACK | TCR_ELx_IRGN0_WRITEBACK |
		(txsz << TCR_ELx_T0SZ_SHIFT) |
		TCR_EL1_EPD1_TTBR1_DISABLED |
		TCR_EL1_SH1_OUTER | TCR_EL1_ORGN1_WRITEBACK | TCR_EL1_IRGN1_WRITEBACK |
		(txsz << TCR_EL1_T1SZ_SHIFT);
#endif
	return tcr;
}

static uint64_t get_table_flags(void)
{
	// All table walks after the start level are non-secure.
	return ARM_LPAE_NSTABLE | ARM_LPAE_TYPE_TABLE | ARM_LPAE_VALID;
}

static uint64_t get_block_flags(unsigned level, arm_mmu_extended_attr_t attr)
{
	assert_valid_level(level);
	RELEASE_ASSERT(level_permits_blocks(level));
	bool normal, writable, executable;
	switch (attr) {
	case kARMMMUDeviceR:
		normal = false; writable = false; executable = false; break;
	case kARMMMUDeviceRX:
		normal = false; writable = false; executable = true; break;
	case kARMMMUDeviceRW:
		normal = false; writable = true;  executable = false; break;
	case kARMMMUNormalR:
		normal = true;  writable = false; executable = false; break;
	case kARMMMUNormalRX:
		normal = true;  writable = false; executable = true; break;
	case kARMMMUNormalRW:
		normal = true;  writable = true;  executable = false; break;
	default:
		panic("Bad attr %d", (int) attr);
	}
	// Always set Access Flag, Non-Secure and Valid.
	uint64_t value = ARM_LPAE_AF | ARM_LPAE_NS | ARM_LPAE_VALID;
	if (level < LAST_TABLE_LEVEL) {
		value |= ARM_LPAE_TYPE_L0L1L2BLOCK;
	} else {
		value |= ARM_LPAE_TYPE_L3BLOCK;
	}
	uint64_t sh, attr_indx;
	if (normal) {
		sh = SH_OUTER_MEMORY;
		attr_indx = CACHE_ATTRINDX_WRITEBACK;
	} else {
		sh = SH_NONE;
		attr_indx = CACHE_ATTRINDX_DISABLE;
	}
	value |= ARM_LPAE_SH(sh);
	value |= ARM_LPAE_ATTRINDX(attr_indx);
	uint64_t ap = writable ? 0 : 2;
	value |= ARM_LPAE_AP(ap);
	if (!executable) {
		value |= ARM_LPAE_PXN | ARM_LPAE_XN;
	}
	return value;
}

static uint64_t get_virt_size_bytes(void)
{
	return 1ULL << get_virt_address_bits();
}

static uint64_t get_phys_size_bytes(void)
{
	return 1ULL << 40;
}

static size_t get_level_shift(unsigned level)
{
	assert_valid_level(level);
	switch (get_page_granule_shift()) {
	case 12: return 39 - (level * 9);
	case 14: return 47 - (level * 11);
	case 16: return 55 - (level * 13);
	default: panic("Granule 2^%u not supported\n",
		       get_page_granule_shift());
	}
}

static size_t get_level_entries(unsigned level)
{
	assert_valid_level(level);
	// 1 granule per sub-table, 8 bytes per entry.
	return ((size_t) 1) << (get_page_granule_shift() - 3);
}

static addr_t tt_alloc(unsigned level)
{
	// Allocate a translation table. The start level was
	// allocated as tt_top_level in arm_mmu_init and should not be
	// requested to be allocated. All levels are the
	// same size: one granule.
	RELEASE_ASSERT(level > get_start_level() && level <= LAST_TABLE_LEVEL);
	size_t bytes = ((size_t) 1) << get_page_granule_shift();
	mmu_spew("Allocate L%u size 0x%llx\n", level,
		 (unsigned long long) bytes);
	if (bytes > tt_alloc_end || tt_alloc_next > tt_alloc_end - bytes) {
		panic("Out of table allocation space L%u\n", level);
	}
	addr_t ret = tt_alloc_next;
	// Alignment is guaranteed by arm_mmu_init() and all
	// allocations being the same size (1 granule), but it's cheap
	// to double-check allocation alignment.
	if ((ret & (bytes - 1)) != 0) {
		panic("Bad aligned pte alloc 0x%llx\n",
		      (unsigned long long) ret);
	}
	tt_alloc_next += bytes;
	mmu_spew("Allocated L%u size 0x%llx at 0x%llx\n",
		 level, (unsigned long long) bytes, (unsigned long long) ret);
	return ret;
}

static size_t map_at_level(unsigned level, addr_t table_base,
			   addr_t vaddr, addr_t paddr, size_t size,
			   arm_mmu_extended_attr_t attr,
			   bool blank_table)
{
	mmu_spew("map_at_level(level=%u,\n"
		 "             table_base=0x%llx,\n"
		 "             vaddr=0x%llx,\n"
		 "             paddr=0x%llx,\n"
		 "             size=0x%llx,\n"
		 "             attr=%u\n",
		 level, (unsigned long long) table_base,
		 (unsigned long long) vaddr, (unsigned long long) paddr,
		 (unsigned long long) size, (unsigned) attr);

	// Check this mapping is bounded by virtual and physical address limits.
	assert_valid_level(level);
	RELEASE_ASSERT(size <= get_virt_size_bytes() &&
	               vaddr <= get_virt_size_bytes() - size);
	RELEASE_ASSERT(size <= get_phys_size_bytes() &&
	               paddr <= get_phys_size_bytes() - size);

	// Calculate constants about this translation level.
	size_t level_shift = get_level_shift(level);
	size_t level_entries = get_level_entries(level);
	size_t level_size = 1ULL << level_shift;
	size_t level_mask = level_size - 1;
	bool permit_block = level_permits_blocks(level);
	size_t index = (vaddr >> level_shift) & (level_entries - 1);
	mmu_spew("shift:%d entries:%d size:0x%llx mask:0x%llx "
		 "block:%d index:%d\n",
		 (int) level_shift,
		 (int) level_entries,
		 (unsigned long long) level_size,
		 (unsigned long long) level_mask,
		 (int) permit_block,
		 (int) index);

	// Can we make block entries here? Must be permitted at this
	// level, have enough bytes remaining, and both virtual and
	// physical addresses aligned to a block.
	if (permit_block &&
	    size >= level_size &&
	    ((vaddr | paddr) & level_mask) == 0) {
		// Map contiguous blocks.
		mmu_spew("Array of blocks\n");
		size_t mapped_size = 0;
		size_t remaining_size = size;
		// Common block flags for all entries at this level.
		uint64_t block_flags = get_block_flags(level, attr);
		while (remaining_size >= level_size && index < level_entries) {
			// Check the existing entry is unused.
			uint64_t entry = get_table_entry(level, table_base,
							 index);
			if (entry != 0) {
				// Not difficult but no use case.
				panic("Remapping an existing L%u at 0x%016llx",
				      level, (unsigned long long) vaddr);
			}
			// Form an entry.
			uint64_t ent = block_flags | paddr;
			// Write a block entry to the table.
			set_table_entry(level, table_base, index, ent);
			// Next index.
			mapped_size += level_size;
			remaining_size -= level_size;
			paddr += level_size;
			vaddr += level_size;
			++index;
		}
		// Must make forward progress... and not too much.
		RELEASE_ASSERT(mapped_size > 0 && mapped_size <= size);
		return mapped_size;
	} else {
		// Sub-divide into a next level table.
		mmu_spew("Sub-divide\n");

		// Only so much granularity available.
		RELEASE_ASSERT(level < LAST_TABLE_LEVEL);
	
		// Get the existing entry.
		uint64_t entry = blank_table ? 0 : get_table_entry(level, table_base, index);
		if ((entry & ARM_LPAE_VALID) &&
		    (entry & ARM_LPAE_TYPE_MASK) == ARM_LPAE_TYPE_L0L1L2BLOCK) {
			// Not hard but no use case.
			panic("Breaking down blocks not implemented");
		}

		unsigned sub_level = level + 1;
		bool blank_sub_table;
		addr_t sub_base;
		if (entry & ARM_LPAE_VALID) {
			mmu_spew("Existing entry\n");
			sub_base = entry & ARM_LPAE_TABLE_MASK;
			blank_sub_table = false;
		} else {
			mmu_spew("Allocating sub-level table\n");
			sub_base = tt_alloc(sub_level);
			// Form an entry.
			uint64_t ent = get_table_flags() | sub_base;
			// Store an entry pointing to the sub-level table.
			set_table_entry(level, table_base, index, ent);
			blank_sub_table = true;
		}
		// Recurse at next level.
		return map_at_level(sub_level, sub_base,
				    vaddr, paddr, size, attr, blank_sub_table);
	}
}

void arm_mmu_map_range(addr_t vaddr, addr_t paddr, size_t size,
		       arm_mmu_extended_attr_t attr)
{
	// Ensure all addresses and sizes are at least L3 aligned.
	size_t l3_align = ((size_t) 1) << get_level_shift(LAST_TABLE_LEVEL);
	RELEASE_ASSERT(((vaddr | paddr | size) & (l3_align - 1)) == 0);
	while (size > 0) {
		// Map at the top level first, allowing the mapper function
		// to break it down into smaller chunks as necessary.
		size_t mapped = map_at_level(get_start_level(),
					     (addr_t) tt_top_level,
					     vaddr, paddr, size, attr, false);
		RELEASE_ASSERT(mapped <= size);
		size -= mapped;
		vaddr += mapped;
		paddr += mapped;
	}

	if ((arm_read_sctlr() & SCTLR_M_ENABLED) != 0) {
		arm_flush_tlbs();
	}
}

void arm_mmu_map_ro(addr_t addr, size_t size)
{
	arm_mmu_map_range(addr, addr, size, kARMMMUNormalR);
}

void arm_mmu_map_rw(addr_t addr, size_t size)
{
	arm_mmu_map_range(addr, addr, size, kARMMMUNormalRW);
}

void arm_mmu_map_rx(addr_t addr, size_t size)
{
	arm_mmu_map_range(addr, addr, size, kARMMMUNormalRX);
}

void arm_mmu_map_device_ro(addr_t addr, size_t size)
{
	arm_mmu_map_range(addr, addr, size, kARMMMUDeviceR);
}

void arm_mmu_map_device_rw(addr_t addr, size_t size)
{
	arm_mmu_map_range(addr, addr, size, kARMMMUDeviceRW);
}

void arm_mmu_map_device_rx(addr_t addr, size_t size)
{
	arm_mmu_map_range(addr, addr, size, kARMMMUDeviceRX);
}

void arm_mmu_init(bool resume)
{
	// Setup memory attributes.
	arm_write_mair(get_mair());

	// Setup translation control register.
	arm_write_tcr(get_tcr());

	/*
	 * Sub-level page table allocations starts in the book-keeping
	 * region after all stacks, rounded up. Allocation rounded down.
	 */
	size_t granule_mask = (((size_t) 1) << get_page_granule_shift()) - 1;
	addr_t page_tables_base;
	size_t page_tables_size;
	mmu_get_memmap(&page_tables_base,
		       &page_tables_size);

	RELEASE_ASSERT((page_tables_base & granule_mask) == 0);
	RELEASE_ASSERT((page_tables_size & granule_mask) == 0);

	tt_alloc_start = page_tables_base;
	tt_alloc_next = tt_alloc_start;
	tt_alloc_end = (page_tables_base + page_tables_size) & ~granule_mask;

	// allocate top-level page table
	tt_top_level = (uint64_t *)tt_alloc_start;
	tt_alloc_next += 1 << get_page_granule_shift();

	// Defer to platform code for the memory map setup.
	platform_mmu_setup(resume);

	// Write the translation table base.
	arm_write_ttbr0(tt_top_level);

	// Flush TLBs.
	arm_flush_tlbs();
}


#ifdef TEST
// static->global stubs for mmu_test.c
uint64_t *mmu_get_tt_top_level(void) { return tt_top_level; }
unsigned mmu_get_start_level(void) { return get_start_level(); }
uint64_t mmu_get_tcr(void) { return get_tcr(); }
uint64_t mmu_tt_alloc(unsigned level) { return tt_alloc(level); }
size_t mmu_get_level_entries(unsigned level) {
	return get_level_entries(level);
}
uint64_t mmu_get_table_flags(void) {
	return get_table_flags();
}
uint64_t mmu_get_block_flags(unsigned level, arm_mmu_extended_attr_t attr) {
	return get_block_flags(level, attr);
}

size_t mmu_map_at_level(unsigned level, addr_t table_base,
			addr_t vaddr, addr_t paddr, size_t size,
			arm_mmu_extended_attr_t attr)
{
	return map_at_level(level, table_base, vaddr, paddr, size, attr, false);
}

void mmu_get_tt_alloc_range(addr_t *start, addr_t *next, addr_t *end)
{
	*start = tt_alloc_start;
	*next = tt_alloc_next;
	*end = tt_alloc_end;
}

void mmu_reset_state(void)
{
	tt_top_level = 0;
	tt_alloc_start = 0;
	tt_alloc_next = 0;
	tt_alloc_end = 0;
}
#endif  // defined(TEST)
