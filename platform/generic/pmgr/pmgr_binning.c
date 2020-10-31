/*
 * Copyright (C) 2015 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */

#include <platform.h>
#include <platform/pmgr.h>
#include <debug.h>

static uint64_t pmgr_binning_read_register(uint64_t address, uint32_t low, uint32_t high, bool reg64)
{
	uint64_t result = 0;
	do {
		uint64_t current_low;
		uint64_t offset;
		uint64_t register_value;

		if (reg64) {
			offset = high/64;
			current_low = offset * 64;
			register_value = ((uint64_t *)(address))[offset];
		} else {
			offset = high/32;
			current_low = offset * 32;
			register_value = ((uint32_t *)(address))[offset];
		}

		// Mask useful bit : [current_low:high]
		register_value &= (1ULL << (high - current_low + 1)) - 1;

		if (current_low <= low) {
			result |= register_value >> (low - current_low);
			break;
		}

		result |= register_value << (current_low - low);
		high = current_low - 1;
	} while (1);

	return result;
}

static uint64_t pmgr_binning_read(const struct pmgr_binning_fuse_to_register *fuse_to_register, uint32_t binning_low, uint32_t binning_high)
{
	uint32_t result = 0;
	while (1) {
		uint32_t i = 0;
		while (fuse_to_register[i].register_address != 0) {
			if ((fuse_to_register[i].binning_high >= binning_high) && (fuse_to_register[i].binning_low <= binning_high)) {
				break;
			}
			i++;
		}
		if (fuse_to_register[i].register_address == 0) {
			return PMGR_BINNING_NOTFOUND;
		}

		if ((fuse_to_register[i].binning_high >= binning_high) && (fuse_to_register[i].binning_low <= binning_low)) {
			result |= pmgr_binning_read_register(fuse_to_register[i].register_address,
													binning_low - fuse_to_register[i].binning_low + fuse_to_register[i].register_low,
													binning_high - fuse_to_register[i].binning_low + fuse_to_register[i].register_low,
													fuse_to_register[i].register_64bit);
			break;
		}
		result |= pmgr_binning_read_register(fuse_to_register[i].register_address,
											 fuse_to_register[i].register_low,
											 binning_high - fuse_to_register[i].binning_low + fuse_to_register[i].register_low,
											 fuse_to_register[i].register_64bit) << (fuse_to_register[i].binning_low - binning_low);
		binning_high = fuse_to_register[i].binning_low - 1;
	}
	return result;
}

static uint32_t pmgr_binning_from_const_data(enum pmgr_binning_type_t type, uint32_t mode, bool use_binning)
{
	uint32_t i;
	for (i = 0; i < pmgr_binning_mode_to_const_data_size; i++) {
		if (pmgr_binning_mode_to_const_data[i].type != type) {
			continue;
		}
		if (pmgr_binning_mode_to_const_data[i].mode != mode) {
			continue;
		}
		if ((use_binning) && pmgr_binning_mode_to_const_data[i].bingroup != pmgr_binning_get_group() && pmgr_binning_mode_to_const_data[i].bingroup != PMGR_BINNING_GROUP_ALL) {
			continue;
		}
		if ((use_binning) && (pmgr_binning_get_revision() < pmgr_binning_mode_to_const_data[i].fuse_revision_minimum)) {
			continue;
		}
		if ((!use_binning) && (pmgr_binning_mode_to_const_data[i].fuse_revision_minimum != 0)) {
			continue;
		}
		return pmgr_binning_mode_to_const_data[i].mv;
	}
	return PMGR_BINNING_NOTFOUND;
}

uint32_t pmgr_binning_from_config_data(uint32_t voltage_index, const struct pmgr_binning_voltage_index_to_config **config_data)
{
	uint32_t i;
	for (i = 0; i < pmgr_binning_voltage_index_to_config_data_size; i++) {
		if (pmgr_binning_voltage_index_to_config_data[i].voltage_index != voltage_index) {
			continue;
		}
		if (pmgr_binning_voltage_index_to_config_data[i].bingroup != pmgr_binning_get_group() && pmgr_binning_voltage_index_to_config_data[i].bingroup != PMGR_BINNING_GROUP_ALL) {
			continue;
		}
		if (pmgr_binning_get_revision() < pmgr_binning_voltage_index_to_config_data[i].fuse_revision_minimum) {
			continue;
		}
		*config_data = &pmgr_binning_voltage_index_to_config_data[i];

		return pmgr_binning_voltage_index_to_config_data[i].safe_voltage;
	}
	return PMGR_BINNING_NOTFOUND;
}

int32_t pmgr_binning_get_voltage_offset(uint32_t voltage_index, enum pmgr_binning_type_t type)
{
	uint32_t i;
	uint32_t size = 0;
	const struct pmgr_binning_voltage_index_to_offset *voltage_index_to_offset;
	uint32_t board_id = platform_get_board_id();

	for (i = 0; i < pmgr_binning_board_id_to_offsets_data_size; i++) {
		if (pmgr_binning_board_id_to_offsets_data[i].board_id != board_id) {
			continue;
		}
		if (pmgr_binning_board_id_to_offsets_data[i].type != type) {
			continue;
		}
		if (platform_get_chip_revision() < pmgr_binning_board_id_to_offsets_data[i].chip_rev_min) {
			continue;
		}
		if (pmgr_binning_board_id_to_offsets_data[i].bingroup != pmgr_binning_get_group() && pmgr_binning_board_id_to_offsets_data[i].bingroup != PMGR_BINNING_GROUP_ALL) {
			continue;
		}

		voltage_index_to_offset = pmgr_binning_board_id_to_offsets_data[i].voltage_offsets;
		size = pmgr_binning_board_id_to_offsets_data[i].size;
		break;
	}

	for (i = 0; i < size; i++) {
		if (voltage_index_to_offset[i].voltage_index != voltage_index) {
			continue;
		}
		if (pmgr_binning_get_revision() < voltage_index_to_offset[i].fuse_revision_minimum) {
			continue;
		}

		return voltage_index_to_offset[i].offset;
	}

	return 0;
}

uint32_t pmgr_binning_mode_get_value(enum pmgr_binning_type_t type, uint32_t mode)
{
	uint32_t i = 0;
	do {
		if (pmgr_binning_mode_to_fuse_data[i].type == PMGR_BINNING_END_OF_LIST) {
			return PMGR_BINNING_NOTFOUND;
		}
		if (pmgr_binning_mode_to_fuse_data[i].type != type) {
			i++;
			continue;
		}
		if (pmgr_binning_mode_to_fuse_data[i].mode != mode) {
			i++;
			continue;
		}
		if (type != PMGR_BINNING_GROUP && pmgr_binning_mode_to_fuse_data[i].bingroup != pmgr_binning_get_group() && pmgr_binning_mode_to_fuse_data[i].bingroup != PMGR_BINNING_GROUP_ALL) {
			i++;
			continue;
		}
		break;
	} while (1);

	if (pmgr_binning_mode_to_fuse_data[i].low > pmgr_binning_mode_to_fuse_data[i].high) {
		panic("Invalid low (%u) and high (%u) mode_to_fuse_data for type %u mode %u\n", pmgr_binning_mode_to_fuse_data[i].low, pmgr_binning_mode_to_fuse_data[i].high, type, mode);
	}

	uint32_t result = pmgr_binning_read(pmgr_binning_mode_to_fuse_data[i].fuse_to_register, pmgr_binning_mode_to_fuse_data[i].low, pmgr_binning_mode_to_fuse_data[i].high);
	return result;
}

uint32_t pmgr_binning_get_base(void)
{
	static uint32_t base = 0;
	if (base == 0) {
		base = pmgr_binning_mode_get_value(PMGR_BINNING_BASE, 0);
	}
	return base;
}

uint32_t pmgr_binning_get_group(void)
{
	static uint32_t group = 0;
	if (group == 0) {
		group =pmgr_binning_mode_get_value(PMGR_BINNING_GROUP, 0);
	}
	return group;
}

uint32_t pmgr_binning_get_revision(void)
{
	static uint32_t revision = 0;
	if (revision == 0) {
		revision = pmgr_binning_mode_get_value(PMGR_BINNING_REV, 0);
	}
	return revision;
}

uint32_t pmgr_binning_mode_get_mv(enum pmgr_binning_type_t type, uint32_t mode)
{
	uint32_t value = pmgr_binning_mode_get_value(type, mode);
	if ((value == PMGR_BINNING_NOTFOUND) || (value == 0)) {
		return PMGR_BINNING_NOTFOUND;
	}
	return 25 * (pmgr_binning_get_base() + 1) + value * 5;
}

uint32_t pmgr_binning_get_mv(uint32_t voltage_index, bool sram, bool use_binning)
{
	uint32_t voltage;
	int32_t voltage_offset;
	uint32_t mode;
	enum pmgr_binning_type_t type;

	if (sram) {
		if (voltage_index >= pmgr_binning_voltage_config_sram_data_size) {
			return PMGR_BINNING_NOTFOUND;
		}
		const struct pmgr_binning_voltage_config *binning_voltage_config;
		binning_voltage_config = &pmgr_binning_voltage_config_sram_data[voltage_index];

		voltage = binning_voltage_config->safe_voltage;
		type = binning_voltage_config->type;
		mode = binning_voltage_config->mode;

		if ((voltage == 0) && (type != PMGR_BINNING_NONE)){
			return pmgr_binning_from_const_data(type, mode, use_binning);
		}
	} else {
		const struct pmgr_binning_voltage_index_to_config *config_data;
		if (pmgr_binning_from_config_data(voltage_index, &config_data) == PMGR_BINNING_NOTFOUND) {
			return PMGR_BINNING_NOTFOUND;
		}

		voltage = config_data->safe_voltage;
		type = config_data->type;
		mode = config_data->mode;

		if (use_binning) {
			uint32_t voltage_binning = pmgr_binning_mode_get_mv(type, mode);
			if (voltage_binning != PMGR_BINNING_NOTFOUND) {
				voltage = voltage_binning;
			}
		}
	}

	voltage_offset = pmgr_binning_get_voltage_offset(voltage_index, type);
	if ((int32_t)voltage + voltage_offset < 0) {
		panic("Invalid voltage offset");
	}

	return voltage + voltage_offset;
}

const struct pmgr_binning_vol_adj *pmgr_binning_get_vol_adj(uint32_t chipdid, uint32_t chip_rev, uint32_t voltage_index)
{
	uint32_t fuse_rev = pmgr_binning_get_revision();

	for (size_t i = 0; i < pmgr_binning_voltadj_entry_data_size; i++) {
		if (voltage_index != pmgr_binning_voltadj_entry_data[i].voltage_index) {
			continue;
		}
		if ((chip_rev < pmgr_binning_voltadj_entry_data[i].chip_rev_min)) {
			continue;
		}
		if ((fuse_rev < pmgr_binning_voltadj_entry_data[i].fuse_rev_min)) {
			continue;
		}
		return &pmgr_binning_voltadj_entry_data[i].voltages;
	}
	return NULL;
}
