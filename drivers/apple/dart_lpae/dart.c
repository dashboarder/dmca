/*
 * Copyright (C) 2012-2013 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */
#include <debug.h>
#include <arch/arm/arm.h>
#include <drivers/dart.h>
#include <platform/soc/hwregbase.h>
#include <sys.h>
#include "dart_regs.h"

#if DART_PAGE_SIZE != DART_TT_PGSIZE
#error "The generic header's idea of the page size doesn't agree with this driver"
#endif

typedef uint64_t	dart_ent;

typedef struct dart_l2_tt {
	dart_ent	*tt_base;
	uintptr_t	tt_phys;

	dart_ent	*l3_tt_bases[DART_TTE_PGENTRIES];
} dart_l2_tt_t;

typedef struct dart {
	uintptr_t	regs;
	dart_l2_tt_t	*l2_tts[DART_NUM_TTBR];
	dart_iovm_addr_t	addr_mask;
} dart_t;

dart_t darts[NUM_DARTS];

static void *alloc_tt_page(void)
{
	void *result;
	result = memalign(DART_TT_PGSIZE, DART_TT_PGSIZE);
	bzero(result, DART_TT_PGSIZE);

	return result;
}

static void flush_tlb(dart_t *dart)
{
	arm_memory_barrier();

	// Flush SID 0
	rDART_TLB_OP(dart->regs) = DART_TLB_OP_FLUSH | DART_TLB_OP_SID_MASK0;

	while(rDART_TLB_OP(dart->regs) & DART_TLB_OP_BUSY)
		;
}

// Clear out any L3 entries in the DART so that nothing is mapped into IO virtual
// space. Don't clear out the mappings at the L2 level, just at L3. The DART
// doesn't support block mappings, so L2 entries can never cause a mapping
// without a subsequent L3 entry.
static void clear_mappings(dart_t *dart)
{
	unsigned int i, j;
	dart_l2_tt_t *l2_tt;
	dart_ent *l3_tt_base;

	for (i = 0; i < DART_NUM_TTBR; i++) {

		l2_tt = dart->l2_tts[i];
		if (l2_tt == NULL)
			continue;

		for (j = 0; j < DART_TTE_PGENTRIES; j++) {
			l3_tt_base = l2_tt->l3_tt_bases[j];

			if (l3_tt_base != NULL)
				memset(l3_tt_base, 0, DART_TT_PGSIZE);
		}
	}
}

void dart_init(unsigned int dart_id)
{
	dart_t *dart;
	uint32_t reg;

	ASSERT(dart_id < NUM_DARTS);
	dart = &darts[dart_id];

	switch (dart_id)
	{
#ifdef DART0_BASE_ADDR
		case 0:
			dart->regs = DART0_BASE_ADDR;
			dart->addr_mask = DART0_ADDR_MASK;
			break;
#endif
#ifdef DART1_BASE_ADDR
		case 1:
			dart->regs = DART1_BASE_ADDR;
			dart->addr_mask = DART1_ADDR_MASK;
			break;
#endif
#ifdef DART2_BASE_ADDR
		case 2:
			dart->regs = DART2_BASE_ADDR;
			dart->addr_mask = DART2_ADDR_MASK;
			break;
#endif
#ifdef DART3_BASE_ADDR
		case 3:
			dart->regs = DART3_BASE_ADDR;
			dart->addr_mask = DART3_ADDR_MASK;
			break;
#endif
		default:
			panic("Unknown DART index %d", dart_id);
			break;
	}

	// If the DART has previously been initialized, we need to clear out all mappings
	// This will be a quick no-op if the DART hasn't been initialized
	clear_mappings(dart);

	// If for some reason translation gets disabled, bypass to an invalid
	// address instead of DRAM
	rDART_BYPASS_ADDR(dart->regs) = 0;

	rDART_DIAG_BOGUS_ACCESS(dart->regs) = ERR_REFLECTION_BASE_ADDR >> 12;

	// Tunables
	rDART_DIAG_CONFIG(dart->regs) |= DART_DIAG_CONFIG_PTE_FETCH_PROMOTE_QOS;
	reg = rDART_FETCH_REQ_CONFIG(dart->regs);
	reg &= ~DART_FETCH_REQ_CONFIG_STE(0xf);
	reg |= DART_FETCH_REQ_CONFIG_STE(0x3);
	reg &= ~DART_FETCH_REQ_CONFIG_PTE(0xf);
	reg |= DART_FETCH_REQ_CONFIG_PTE(0x3);
	reg &= ~DART_FETCH_REQ_CONFIG_PTE_PREFETCH(0xf);
	reg |= DART_FETCH_REQ_CONFIG_PTE_PREFETCH(0xe);
	rDART_FETCH_REQ_CONFIG(dart->regs) = reg;

	// Restore TTBRs if we were previously enabled, otherwise initialize
	// all the TTBRs to 0
	dart_enable_translation(dart_id);

	// Enable translation and exceptions for SID 0
	rDART_CONFIG(dart->regs) = 0x80;
}

/* Disables all TTBRs and flushes the TLB, but leaves translation enabled. This will
 * make sure that all translation attempts result in an error. If we disabled translation,
 * all translation attempts would go to the bypass address, which is not what we want */
void dart_disable_translation(unsigned int dart_id)
{
	dart_t *dart;
	int i;

	ASSERT(dart_id < NUM_DARTS);
	dart = &darts[dart_id];

	for (i = 0; i < DART_NUM_TTBR; i++)
		rDART_TTBR(dart->regs, i) = 0;

	flush_tlb(dart);
}

/* Re-enables mappings that were previously created with dart_map_page_range by reprogramming
 * the DART's TTBRs with the previously created translation tables */
void dart_enable_translation(unsigned int dart_id)
{
	uint32_t i;
	uint32_t ttbr;
	dart_t *dart;
	dart_l2_tt_t *l2_tt;

	ASSERT(dart_id < NUM_DARTS);
	dart = &darts[dart_id];

	for (i = 0; i < DART_NUM_TTBR; i++) {
		l2_tt = dart->l2_tts[i];

		if (l2_tt == NULL) {
			ttbr = 0;
		} else {
			ttbr = (l2_tt->tt_phys >> DART_TTBR_SHIFT) | DART_TTBR_VALID;
		}

		rDART_TTBR(dart->regs, i) = ttbr;
	}

	flush_tlb(dart);
}

void dart_map_page_range(unsigned int dart_id, uintptr_t paddr, dart_iovm_addr_t vaddr, uint32_t pages, bool write_protect)
{
	uint32_t ttbr;
	uint32_t i;
	dart_iovm_addr_t page_vaddr;
	uintptr_t page_paddr;
	dart_ent ent;
	dart_t *dart;

	ASSERT(dart_id < NUM_DARTS);
	dart = &darts[dart_id];

	ASSERT((vaddr & DART_TT_L3_OFFMASK) == 0);
	ASSERT((paddr & DART_TT_L3_OFFMASK) == 0);

	page_vaddr = vaddr & dart->addr_mask;
	page_paddr = paddr;

	dprintf(DEBUG_SPEW, "dart%d: mapped 0x%04x pages at iovm 0x%08x to phys %p\n", dart_id, pages, vaddr, (void *)paddr);

	for (i = 0; i < pages; i++) {
		uint32_t l1_index = (page_vaddr & DART_TT_L1_INDEX_MASK) >> DART_TT_L1_SHIFT;
		uint32_t l2_index = (page_vaddr & DART_TT_L2_INDEX_MASK) >> DART_TT_L2_SHIFT;
		uint32_t l3_index = (page_vaddr & DART_TT_L3_INDEX_MASK) >> DART_TT_L3_SHIFT;
		dart_l2_tt_t *l2_tt;
		dart_ent *l3_tt;

		if (dart->l2_tts[l1_index] == NULL) {
			dart_ent *new_tt;
			uintptr_t new_tt_phys;

			new_tt = alloc_tt_page();
			new_tt_phys = mem_static_map_physical((uintptr_t)new_tt);
			dart->l2_tts[l1_index] = calloc(1, sizeof(dart_l2_tt_t));
			dart->l2_tts[l1_index]->tt_base = new_tt;
			dart->l2_tts[l1_index]->tt_phys = new_tt_phys;

			ttbr = (new_tt_phys >> DART_TTBR_SHIFT) | DART_TTBR_VALID;
			rDART_TTBR(dart->regs, l1_index) = ttbr;
		}
		l2_tt = dart->l2_tts[l1_index];

		if (l2_tt->l3_tt_bases[l2_index] == NULL) {
			dart_ent *new_tt = alloc_tt_page();
			l2_tt->l3_tt_bases[l2_index] = new_tt;

			ent = ((uintptr_t)mem_static_map_physical((uintptr_t)new_tt)) | DART_TTE_TABLE | DART_TTE_VALID;
			l2_tt->tt_base[l2_index] = ent;
		}
		l3_tt = l2_tt->l3_tt_bases[l2_index];

		RELEASE_ASSERT(l3_tt[l3_index] == 0);

		ent = page_paddr | DART_PTE_WRPROT(write_protect) | DART_PTE_TYPE_VALID;
		l3_tt[l3_index] = ent;

		page_vaddr += DART_TT_L3_SIZE;
		page_paddr += DART_TT_L3_SIZE;
        }
	
	flush_tlb(dart);
}

void dart_unmap_page_range(unsigned int dart_id, dart_iovm_addr_t vaddr, uint32_t pages)
{
	uint32_t i;
	dart_iovm_addr_t page_vaddr;
	dart_t *dart;

	ASSERT(dart_id < NUM_DARTS);
	dart = &darts[dart_id];

	ASSERT((vaddr & DART_TT_L3_OFFMASK) == 0);

	dprintf(DEBUG_SPEW, "dart%d: unmapped 0x%04x pages at iovm 0x%08x\n", dart_id, pages, vaddr);

	page_vaddr = vaddr & dart->addr_mask;

	for (i = 0; i < pages; i++) {
		uint32_t l1_index = (page_vaddr & DART_TT_L1_INDEX_MASK) >> DART_TT_L1_SHIFT;
		uint32_t l2_index = (page_vaddr & DART_TT_L2_INDEX_MASK) >> DART_TT_L2_SHIFT;
		uint32_t l3_index = (page_vaddr & DART_TT_L3_INDEX_MASK) >> DART_TT_L3_SHIFT;
		dart_l2_tt_t *l2_tt;
		dart_ent *l3_tt;

		l2_tt = dart->l2_tts[l1_index];
		RELEASE_ASSERT(l2_tt != NULL);

		l3_tt = l2_tt->l3_tt_bases[l2_index];
		RELEASE_ASSERT(l3_tt != NULL);

		l3_tt[l3_index] = 0;

		page_vaddr += DART_TT_L3_SIZE;
        }

	flush_tlb(dart);
}

void dart_write_protect_page_range(unsigned int dart_id, dart_iovm_addr_t vaddr, uint32_t pages, bool write_protect)
{
	uint32_t i;
	dart_iovm_addr_t page_vaddr;
	dart_ent ent;
	dart_t *dart;

	ASSERT(dart_id < NUM_DARTS);
	dart = &darts[dart_id];

	ASSERT((vaddr & DART_TT_L3_OFFMASK) == 0);

	page_vaddr = vaddr;

	for (i = 0; i < pages; i++) {
		uint32_t l1_index = (page_vaddr & DART_TT_L1_INDEX_MASK) >> DART_TT_L1_SHIFT;
		uint32_t l2_index = (page_vaddr & DART_TT_L2_INDEX_MASK) >> DART_TT_L2_SHIFT;
		uint32_t l3_index = (page_vaddr & DART_TT_L3_INDEX_MASK) >> DART_TT_L3_SHIFT;
		dart_l2_tt_t *l2_tt;
		dart_ent *l3_tt;

		l2_tt = dart->l2_tts[l1_index];
		RELEASE_ASSERT(l2_tt != NULL);

		l3_tt = l2_tt->l3_tt_bases[l2_index];
		RELEASE_ASSERT(l3_tt != NULL);

		ent = l3_tt[l3_index];
		ent &= DART_PTE_PAGE_MASK;
		ent |= DART_PTE_WRPROT(write_protect);
		l3_tt[l3_index] = ent;

		page_vaddr += DART_TT_L3_SIZE;
        }

	flush_tlb(dart);
}

// Verifies that all mappings have been removed from the DART. This is used
// to verify that drivers aren't leaking mappings, as leaked mappings could
// lead to a security hole
void dart_assert_unmapped(unsigned int dart_id)
{
	dart_t *dart;
	unsigned int i, j, k;
	dart_l2_tt_t *l2_tt;
	dart_ent *l3_tt_base;

	ASSERT(dart_id < NUM_DARTS);
	dart = &darts[dart_id];

	for (i = 0; i < DART_NUM_TTBR; i++) {
		// If the driver isn't tracking an L2 table under the given TTBR,
		// then, the TTBR needs to be 0
		l2_tt = dart->l2_tts[i];
		if (l2_tt == NULL) {
			RELEASE_ASSERT(rDART_TTBR(dart->regs, i) == 0);
			continue;
		}

		// For each L2 table, verify each entry is either 0 or points
		// to an L3 table filled with 0s
		for (j = 0; j < DART_TTE_PGENTRIES; j++) {
			l3_tt_base = l2_tt->l3_tt_bases[j];

			if (l3_tt_base != NULL) {
				for (k = 0; k < DART_TTE_PGENTRIES; k++) {
					RELEASE_ASSERT(l3_tt_base[k] == 0);
				}
			} else {
				RELEASE_ASSERT(l2_tt->tt_base[j] == 0);
			}
		}
	}
}
