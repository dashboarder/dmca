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

#ifndef __PLATFORM_PMGR_H
#define __PLATFORM_PMGR_H

#include <sys/types.h>

enum pmgr_binning_type_t {
	PMGR_BINNING_NONE = 0,
	PMGR_BINNING_CPU,
	PMGR_BINNING_CPU_SRAM,
	PMGR_BINNING_GPU,
	PMGR_BINNING_GPU_SRAM,
	PMGR_BINNING_SRAM,
	PMGR_BINNING_SOC,
	PMGR_BINNING_BASE,
	PMGR_BINNING_REV,
	PMGR_BINNING_GROUP,
	PMGR_BINNING_END_OF_LIST,
};

#define PMGR_BINNING_GROUP_ALL 0xf
#define PMGR_BINNING_NOTFOUND UINT32_MAX
#define PMGR_BINNING_MODE_NONE 0x3f

struct pmgr_binning_fuse_to_register {
	uint32_t binning_low:12;
	uint32_t binning_high:12;
	uint32_t register_low:6;
	uint32_t register_64bit:1;
	uint64_t register_address;
};

struct pmgr_binning_mode_to_fuse {
	enum pmgr_binning_type_t type;
	uint32_t mode:16;
	uint32_t bingroup:16;
	uint32_t low:11;
	uint32_t high:11;
	const struct pmgr_binning_fuse_to_register *fuse_to_register;
};

struct pmgr_binning_mode_to_const {
	uint32_t mv:12;
	uint32_t mode:4;
	uint32_t type:4;
	uint32_t bingroup:4;
	uint32_t fuse_revision_minimum:8;
};

struct pmgr_binning_voltage_config {
	uint32_t safe_voltage:11;
	uint32_t mode:6;
	uint32_t type:4;
};

struct pmgr_binning_voltage_index_to_config {
	uint32_t voltage_index:8;
	uint32_t safe_voltage:11;
	uint32_t mode:6;
	uint32_t type:4;
	uint32_t bingroup:4;
	uint32_t fuse_revision_minimum:8;
};

struct pmgr_binning_voltage_index_to_offset {
	uint32_t voltage_index:8;
	uint32_t fuse_revision_minimum:8;
	int32_t offset:15;
};

struct pmgr_binning_board_id_to_offsets {
	enum pmgr_binning_type_t type;
	const struct pmgr_binning_voltage_index_to_offset *voltage_offsets;
	uint32_t size:5;
	uint32_t chip_rev_min:8;
	uint32_t board_id:5;
	uint32_t bingroup;
};

struct pmgr_binning_vol_adj {
	uint64_t volAdj0:20;
	uint64_t volAdj1:20;
	uint64_t volAdj2:20;
	uint64_t volAdj3:20;
	uint64_t dvfmMaxAdj:20;
	uint64_t dvmrAdj0:20;
	uint64_t dvmrAdj1:20;
	uint64_t dvmrAdj2:20;
};

struct __attribute__((packed)) pmgr_binning_voltadj_entry {
	uint32_t voltage_index:8;
	uint32_t chip_rev_min:8;
	uint32_t fuse_rev_min:8;
	struct pmgr_binning_vol_adj voltages;
};

//
// Must be defined in platform/SOC/chipid/pmgr_binning_SOC.c
//
extern const struct pmgr_binning_mode_to_fuse pmgr_binning_mode_to_fuse_data[];
extern const struct pmgr_binning_voltage_config pmgr_binning_voltage_config_sram_data[];
extern const struct pmgr_binning_mode_to_const pmgr_binning_mode_to_const_data[];
extern const struct pmgr_binning_voltadj_entry pmgr_binning_voltadj_entry_data[];
extern const struct pmgr_binning_voltage_index_to_config pmgr_binning_voltage_index_to_config_data[];
extern const struct pmgr_binning_voltage_index_to_offset pmgr_binning_voltage_index_to_offset_data[];
extern const struct pmgr_binning_board_id_to_offsets pmgr_binning_board_id_to_offsets_data[];
extern const uint32_t pmgr_binning_voltage_config_sram_data_size;
extern const uint32_t pmgr_binning_mode_to_const_data_size;
extern const uint32_t pmgr_binning_voltadj_entry_data_size;
extern const uint32_t pmgr_binning_voltage_index_to_config_data_size;
extern const uint32_t pmgr_binning_board_id_to_offsets_data_size;

//
// Must be defined in platform/SOC/pmgr/pmgr.c
//
extern bool pmgr_platform_set_perf_state(bool gpu, uint32_t state_num, uint32_t *voltage_indexes, uint32_t voltage_index_size);
extern uint32_t pmgr_platform_get_freq(bool gpu, uint32_t voltage_index);
extern bool pmgr_platform_get_perf_state(enum pmgr_binning_type_t type, uint32_t state, uint32_t *voltage, uint32_t *freq);

//
// Defined in platform/generic/pmgr/pmgr_binning.c
//
// Return values are PMGR_BINNING_NOTFOUND if not found/unknown
//
extern uint32_t pmgr_binning_get_mv(uint32_t voltage_index, bool sram, bool use_binning);
extern uint32_t pmgr_binning_mode_get_mv(enum pmgr_binning_type_t type, uint32_t mode);
extern uint32_t pmgr_binning_mode_get_value(enum pmgr_binning_type_t binning_type, uint32_t mode);
extern uint32_t pmgr_binning_get_base(void);
extern uint32_t pmgr_binning_get_group(void);
extern uint32_t pmgr_binning_get_revision(void);
extern uint32_t pmgr_binning_from_config_data(uint32_t voltage_index, const struct pmgr_binning_voltage_index_to_config **config_data);
extern int32_t pmgr_binning_get_voltage_offset(uint32_t voltage_index, enum pmgr_binning_type_t type);
extern const struct pmgr_binning_vol_adj *pmgr_binning_get_vol_adj(uint32_t chipdid, uint32_t chip_rev, uint32_t voltage_index);

//
// Defined in platform/generic/pmgr/pmgr_binning_menu.c
//
extern void pmgr_binning_menu_update_states(void);

#endif
