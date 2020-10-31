/*
 * Copyright (C) 2014-2015 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */

#include <arch.h>
#include <debug.h>
#include <drivers/dram.h>
#include <lib/libc.h>
#include <platform.h>
#include <platform/tunables.h>
#include <stdint.h>

// platfom_indep holds generic functions that are just defined once for all
// platforms, and are not expected to be rebuilt for every target/product.
//
// These functions can vary per BUILD, and per architecture, but not per
// PLATFORM, TARGET, PRODUCT, or CONFIG

void platform_not_supported(void)
{
	dprintf(DEBUG_CRITICAL, "\n");
	dprintf(DEBUG_CRITICAL, "\n");
	dprintf(DEBUG_CRITICAL, "\n");
	dprintf(DEBUG_CRITICAL, "**************************************\n");
	dprintf(DEBUG_CRITICAL, "*                                    *\n");
	dprintf(DEBUG_CRITICAL, "*     This unit is not supported     *\n");
	dprintf(DEBUG_CRITICAL, "*                                    *\n");
	dprintf(DEBUG_CRITICAL, "**************************************\n");
	while (1);
}

//	Fuse Modes
//		Bit 0 - Chip is Secure
//		Bit 1 - Chip is Production

uint32_t platform_get_fuse_modes(void)
{
	u_int32_t fuse_summary = 0;

	if (platform_get_secure_mode()) fuse_summary |= (1 << 0);

	if (platform_get_raw_production_mode()) fuse_summary |= (1 << 1);

	return fuse_summary;
}

const char *platform_get_memory_manufacturer_string(void)
{
	uint8_t index;

	// This list is based of JEDEC Vendor list in dram.h 
	// index 0 and 15 are reserverd, we are using them to provide additional information related to the system.
	static const char *memory_manufacturer_strings[JEDEC_NUM_MANUF_IDS] = {
		"Unknown",
		"Samsung",
		"Qimonda",
		"Elpida",
		"Etron",
		"Nanya",
		"Hynix",
		"Mosel",
		"Winbond",
		"ESMT",
		"Reserved",
		"Spansion",
		"SST",
		"ZMOS",
		"Intel",
		"Numonyx",
		"Micron",
		"FPGA",
	};
	
	index = platform_get_memory_manufacturer_id();
	
	if(platform_is_pre_lpddr4()) {
	
		if (index >= JEDEC_NUM_MANUF_IDS)
			return NULL;

		return memory_manufacturer_strings[index];
	} else {
		switch(index) {
			case JEDEC_LPDDR4_MANUF_ID_SAMSUNG:
				return "Samsung";
				break;
		
			case JEDEC_LPDDR4_MANUF_ID_HYNIX:
				return "Hynix";
				break;
				
			case JEDEC_LPDDR4_MANUF_ID_MICRON:
				return "Micron";
				break;
			
			default:
				return NULL;
				break;
		}
	}
}


uintptr_t platform_get_memory_region_base(memory_region_type_t region)
{
	uintptr_t base = platform_get_memory_region_base_optional(region);
	if (base == (uintptr_t)-1)
		panic("unsupported region %d base requested\n", (int)region);

	return base;
}

size_t platform_get_memory_region_size(memory_region_type_t region)
{
	size_t size = platform_get_memory_region_size_optional(region);

	if (size == (size_t)-1)
		panic("unsupported region %d size requested\n", region);

	return size;
}

void platform_apply_tunables(const struct tunable_chip_struct *tunable_chips, uint32_t num_tunable_chips, const char* type)
{
	const struct tunable_struct *tunable;
	uintptr_t base_address;
	uint32_t chip_rev = platform_get_chip_revision();
	uint32_t i;
	uint32_t t;
	bool match = false;

	if (num_tunable_chips == 0)
		return;

	ASSERT(tunable_chips != NULL);

	// For each bucket of tunable values...
	for (i = 0; i < num_tunable_chips; i++) {

		// Tunable for a newer chip revision?
		if (tunable_chips[i].chip_rev > chip_rev) {
			continue;
		}

		// Tunable for an older chip revision?
		if (tunable_chips[i].chip_rev < chip_rev) {

			if (match) {
				// We've already found all the tunables matching
				// this chip revision -- we're done.
				break;
			} else {
				// We haven't found a match yet. Because the tunable
				// chip table is supposed to be sorted from highest
				// to lowest, finding an older chip revision means
				// that there is no entry in the table for this
				// chip revision and we'll use the first older one
				// we find.
				dprintf(DEBUG_SPEW, "%s tunables for chip rev %d not found -- using tunables for chip rev %d\n",
					type, chip_rev, tunable_chips[i].chip_rev);
				chip_rev = tunable_chips[i].chip_rev;
			}
		}

		// We've found a matching tunable chip revision (either an exact
		// match or the next older chip revision).
		match = true;

		// Get the base address and pointer to the tunables data.
		tunable = tunable_chips[i].tunable;
		ASSERT(tunable != NULL);
		base_address = tunable_chips[i].base_address;

		// For each tunable in registers at this base address...
		for (t = 0; tunable[t].offset != -1; t++) {

			// Elided tunables have a size of zero, just skip them.
			if (tunable[t].size == 0) {
				continue;
			}

			switch (tunable[t].size) {
				case sizeof(uint32_t):
				{
					volatile uint32_t *addr;
					uint32_t new_reg, old_reg, mask, value;

					// Get the address of the register.
					addr = (volatile uint32_t *)(base_address + tunable[t].offset);

					// Get the current value of the register.
					old_reg = *addr;
					mask  = (uint32_t)tunable[t].mask;
					value = (uint32_t)tunable[t].value;

					// If the tunable needs to be applied, do it.
					if ((old_reg & mask) != (value & mask)) {
						new_reg = old_reg;
						new_reg &= ~mask;
						new_reg |= value & mask;
						*addr = new_reg;
						dprintf(DEBUG_SPEW, "%s tunable addr/mask/value=%p/0x%08x/0x%08x: 0x%08x -> 0x%08x\n",
							type, addr, mask, value, old_reg, new_reg);
					} else {
						dprintf(DEBUG_SPEW, "%s tunable addr/mask/value=%p/0x%08x/0x%08x: already correct 0x%08x\n",
							type, addr, mask, value, old_reg);
					}
					break;
				}

				case sizeof(uint64_t):
				{
					volatile uint64_t *addr;
					uint64_t new_reg, old_reg, mask, value;

					// Get the address of the register.
					addr = (volatile uint64_t *)(base_address + tunable[t].offset);

					// Get the current value of the register.
					old_reg = *addr;
					mask  = tunable[t].mask;
					value = tunable[t].value;

					// If the tunable needs to be applied, do it.
					if ((old_reg & mask) != (value & mask)) {
						new_reg = old_reg;
						new_reg &= ~mask;
						new_reg |= value & mask;
						*addr = new_reg;
						dprintf(DEBUG_SPEW, "%s tunable addr/mask/value=%p/0x%016llx/0x%016llx: 0x%016llx -> 0x%016llx\n",
							type, addr, mask, value, old_reg, new_reg);
					} else {
						dprintf(DEBUG_SPEW, "%s tunable addr/mask/value=%p/0x%016llx/0x%016llx: already correct 0x%016llx\n",
							type, addr, mask, value, old_reg);
					}
					break;
				}

				default:
					panic("Unsupported tunable register size");
			}
		}
	}
}

uint8_t *platform_apply_dt_tunables(const struct tunable_chip_struct *tunable_chips,
					uint32_t num_tunable_chips, uint8_t *buffer,
					uintptr_t dt_base, const char *type)
{
	const struct tunable_struct *tunable;
	struct tunable_struct_unpacked tunable_data;
	uintptr_t addr;
	uintptr_t base_address;
	uint32_t chip_rev = platform_get_chip_revision();
	uint32_t i;
	uint32_t t;
	bool match = false;

	if (num_tunable_chips == 0)
		return buffer;

	ASSERT((tunable_chips != NULL) && (buffer != NULL));

	// For each bucket of tunable values...
	for (i = 0; i < num_tunable_chips; i++) {

		// Tunable for a newer chip revision?
		if (tunable_chips[i].chip_rev > chip_rev) {
			continue;
		}

		// Tunable for an older chip revision?
		if (tunable_chips[i].chip_rev < chip_rev) {

			if (match) {
				// We've already found all the tunables matching
				// this chip revision -- we're done.
				break;
			} else {
				// We haven't found a match yet. Because the tunable
				// chip table is supposed to be sorted from highest
				// to lowest, finding an older chip revision means
				// that there is no entry in the table for this
				// chip revision and we'll use the first older one
				// we find.
				dprintf(DEBUG_SPEW, "%s tunables for chip rev %d not found -- using tunables for chip rev %d\n",
					type, chip_rev, tunable_chips[i].chip_rev);
				chip_rev = tunable_chips[i].chip_rev;
			}
		}

		// We've found a matching tunable chip revision (either an exact
		// match or the next older chip revision).
		match = true;

		// Copy the tunable value and get the base address of the block it pertains to.
		tunable = tunable_chips[i].tunable;
		ASSERT(tunable != NULL);
		base_address = tunable_chips[i].base_address;

		// For each tunable register at this base address...
		for (t = 0; tunable[t].offset != -1; t++) {

			// Elided tunables have a size of zero, but pass them
			// through in case the driver wants to un-elide them.

			// Copy the tunable data so we can update it.
			tunable_data.offset = tunable[t].offset;
			tunable_data.size = tunable[t].size;
			tunable_data.mask = tunable[t].mask;
			tunable_data.value = tunable[t].value;

			addr = base_address + tunable_data.offset;

			// Get the address of the register.
			if (dt_base != 0) {
				// Adjust the offset to match the reg property in the
				// device tree.
				tunable_data.offset = (uint32_t)addr - dt_base;
				ASSERT(tunable_data.offset != -1);
			}

			if (tunable_data.size == -1) {
				dprintf(DEBUG_SPEW, "%s DT tunable @ 0x%lx: offset=0x%08x, size=%d, mask=0x%16llx, value=0x%16llx\n",
					type, addr,
					tunable_data.offset, tunable_data.size,
					tunable_data.mask, tunable_data.value);
			} else {
				dprintf(DEBUG_SPEW, "%s DT tunable @ 0x%lx: offset=0x%08x, size=%d, mask=0x%08llx, value=0x%08llx\n",
					type, addr,
					tunable_data.offset, tunable_data.size,
					tunable_data.mask, tunable_data.value);
			}
			memcpy(buffer, &tunable_data, sizeof(tunable_data));
			buffer += sizeof(tunable_data);
		}
	}
	return buffer;
}

