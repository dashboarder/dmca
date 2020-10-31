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

#include <debug.h>
#include <drivers/consistent_debug.h>
#include <platform/memmap.h>
#include <arch/arm/arm.h>
#include <platform.h>
#include <platform/timer.h>
#include <platform/soc/pmgr.h>
#include <stdio.h>

static dbg_registry_t * consistent_debug_registry = NULL; 
static dbg_cpr_t*  ap_cpr;


static dbg_registry_t* init_consistent_debug_registry(void *consistent_debug_base);
static dbg_record_header_t* consistent_debug_allocate_entry();
static void consistent_debug_register_root_pointer(void);

static void consistent_debug_ensure_visibility()
{
	platform_memory_barrier();
#if WITH_NON_COHERENT_DMA	
	// On platforms without coherent DMA, JTAG access is likely incoherent as well.
	platform_cache_operation(CACHE_CLEAN, consistent_debug_registry, sizeof(dbg_registry_t));
#endif	
}

int consistent_debug_resume(void)
{
	// When resuming from hibernate, assume we have a valid Consistent Debug Registry, and
	// put the root address back in the PMGR scratch (which gets cleared on hibernate)
	dprintf(DEBUG_SPEW, "Restoring consistent debug state, preserving existing entries...\n");
	consistent_debug_registry = (dbg_registry_t*)((void*)CONSISTENT_DEBUG_BASE);
	ap_cpr = &consistent_debug_registry->ap_cpr_region;
	// Ensure the header is visible before publishing it in the scratch register.
	consistent_debug_ensure_visibility();
	consistent_debug_register_root_pointer();
	return 0;
}

int consistent_debug_init(void)
{
	dprintf(DEBUG_SPEW, "Enabling consistent debug...\n");
	ASSERT(CONSISTENT_DEBUG_SIZE >= sizeof(dbg_registry_t)); 
	consistent_debug_registry = init_consistent_debug_registry((void*)CONSISTENT_DEBUG_BASE);


	// Set up the CPR
	dbg_record_header_t ap_progress_report;
	ap_progress_report.record_id = kDbgIdCPRHeaderAP;
	ap_progress_report.physaddr = (uint64_t)(mem_static_map_physical((uintptr_t)(&consistent_debug_registry->ap_cpr_region)));
	ap_progress_report.length = sizeof(dbg_cpr_t);
	ap_cpr = &consistent_debug_registry->ap_cpr_region;
	consistent_debug_update_ap_cpr(DBG_CPR_STATE_BOOTING, 0);

	// Set up the XNU PANIC_BASE pointer, so once booted Astris can still unpack the panic string.
	dbg_record_header_t panic_region;
	panic_region.record_id = kDbgIdXNUPanicRegion;
	panic_region.physaddr = (uint64_t)(PANIC_BASE);
	panic_region.length = (uint64_t)(PANIC_SIZE);
	if(!consistent_debug_register_header(panic_region))
	{
		dprintf(DEBUG_INFO, "Unable to register XNU panic region in consistent debug.\n");
	}

	dbg_record_header_t* hdr = (dbg_record_header_t*)consistent_debug_register_header(ap_progress_report);
	RELEASE_ASSERT(hdr);
	// Ensure the header is visible before publishing it in the scratch register.
	consistent_debug_ensure_visibility();
	consistent_debug_register_root_pointer();
	return 0;
}

static void consistent_debug_register_root_pointer(void)
{
	uint32_t root;
	uintptr_t consistent_debug_addr = mem_static_map_physical((uintptr_t)consistent_debug_registry);
	root = consistent_debug_encode_root_pointer(consistent_debug_addr, 0);
	platform_set_consistent_debug_root_pointer(root);
	dprintf(DEBUG_SPEW, "Registered consistent debug root pointer (encoded as 0x%x)\n", (unsigned int)root);
}


dbg_registry_t* consistent_debug_get_registry(void)
{
	return consistent_debug_registry;
}

void consistent_debug_update_ap_cpr(cp_state_t state, int arg)
{
	if(ap_cpr) {
		enter_critical_section();
		if(ap_cpr->rdptr + 1  == ap_cpr->wrptr)
		{
			dprintf(DEBUG_SPEW, "AP CPR is full, advancing rdptr\n");
			// Eat the oldest entry if buffer gets full.
			ap_cpr->rdptr++;
			consistent_debug_ensure_visibility();
		}
		dbg_cpr_state_entry_t* entry = &ap_cpr->cp_state_entries[ap_cpr->wrptr];
		entry->cp_state = state;
		entry->cp_state_arg = arg;
		entry->timestamp = timer_get_ticks();
		consistent_debug_ensure_visibility();
		ap_cpr->wrptr++;
		exit_critical_section(); 
		dprintf(DEBUG_SPEW, "AP CPR updated at time %llu with state %d (argument %d)\n", entry->timestamp, entry->cp_state, entry->cp_state_arg);
	}

}

static dbg_registry_t* init_consistent_debug_registry(void* consistent_debug_base)
{
	dbg_registry_t* reg = (dbg_registry_t*)(consistent_debug_base);
	memset(reg, 0, sizeof(dbg_registry_t));
	reg->top_level_header.record_id = kDbgIdTopLevelHeader;
	reg->top_level_header.num_records = DEBUG_REGISTRY_MAX_RECORDS;
	reg->top_level_header.record_size_bytes = sizeof(dbg_record_header_t);


	return reg;
}

/**
 * Finds a free entry in the Consistent Debug Registry and 
 * reserves it. 
 * 
 * @author jdong (12/7/2012)
 * 
 * @return NULL if unsuccessful, otherwise a pointer to a 
 *         reserved entry (where the record_id is set to
 *         kDbgIdReservedEntry)
 */
static dbg_record_header_t* consistent_debug_allocate_entry(void) {
	if (!consistent_debug_registry) {
		enter_critical_section();
		if(!consistent_debug_registry) {
			// Check again to make sure someone else didn't enable it already
			consistent_debug_init();
		}
		exit_critical_section();
		return consistent_debug_allocate_entry();
	}

	for (unsigned int i = 0; i < consistent_debug_registry->top_level_header.num_records; i++) {
		dbg_record_header_t *record = &consistent_debug_registry->records[i];
		if (__sync_bool_compare_and_swap(&record->record_id, kDbgIdUnusedEntry, kDbgIdReservedEntry)) {
			// Reserved an entry at position i.
			return (dbg_record_header_t*)record;
		}
	}
	return NULL;
}


dbg_record_header_t* consistent_debug_register_header(dbg_record_header_t hdr) {
	dbg_record_header_t *allocated_header = consistent_debug_allocate_entry();
	if (allocated_header) {
		allocated_header->length = hdr.length;
		allocated_header->physaddr = hdr.physaddr;
		consistent_debug_ensure_visibility();
		// Make sure the hdr/length are visible before the record_id.
		allocated_header->record_id = hdr.record_id;
		consistent_debug_ensure_visibility();
	}
	return allocated_header;
}


int consistent_debug_unregister_header(dbg_record_header_t *hdr)
{
	if (!consistent_debug_registry) {
		return -1;
	}
	if ((hdr < &consistent_debug_registry->records[0]) || 
		(hdr >= &consistent_debug_registry->records[consistent_debug_registry->top_level_header.num_records])) {
		// Out of range
		return -1;
	}
	// The record ID may be modified by the JTAG observer, and the caller
	// loops on the value changing to kDbgIdUnusedEntry. Make sure we read it from memory.
	switch (*(volatile uint64_t*)(&hdr->record_id)) {
		case kDbgIdUnusedEntry:
			return -1;
		case kDbgIdFreeReqEntry:
			return 0;
		case kDbgIdFreeAckEntry:
			hdr->record_id = kDbgIdUnusedEntry;
			return 1;
		default:
			hdr->record_id = kDbgIdFreeReqEntry;
			return 0;
	}
}
