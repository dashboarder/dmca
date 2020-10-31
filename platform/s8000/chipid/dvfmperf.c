/*
 * Copyright (C) 2014 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */

/*
 * This file is only a database of the performance sate for CPU and GPU, regarding the kind of chip
 * or board, it has no access to the chip.
 */
#include <platform.h>
#include <platform/soc/dvfmperf.h>

// These array are the first iteration, the next part will include reference to board id, board rev, fuse rev.

static const enum chipid_voltage_index s8000_cpu[] = {
	CHIPID_CPU_VOLTAGE_BYPASS,
	CHIPID_CPU_VOLTAGE_SECUREROM,
	CHIPID_CPU_VOLTAGE_396,
	CHIPID_CPU_VOLTAGE_600,
	CHIPID_CPU_VOLTAGE_912,
	CHIPID_CPU_VOLTAGE_1200,
	CHIPID_CPU_VOLTAGE_1512,
	CHIPID_CPU_VOLTAGE_1800,
	CHIPID_CPU_VOLTAGE_1848,
	CHIPID_CPU_VOLTAGE_396,
	CHIPID_CPU_VOLTAGE_396,
	CHIPID_CPU_VOLTAGE_396,
	CHIPID_CPU_VOLTAGE_396,
	CHIPID_CPU_VOLTAGE_1512_WA,
	CHIPID_CPU_VOLTAGE_1200_WA,
	CHIPID_CPU_VOLTAGE_396_WA,
};

static const enum chipid_voltage_index s8003_cpu[] = {
	CHIPID_CPU_VOLTAGE_BYPASS,
	CHIPID_CPU_VOLTAGE_SECUREROM,
	CHIPID_CPU_VOLTAGE_396,
	CHIPID_CPU_VOLTAGE_600,
	CHIPID_CPU_VOLTAGE_912,
	CHIPID_CPU_VOLTAGE_1200,
	CHIPID_CPU_VOLTAGE_1512,
	CHIPID_CPU_VOLTAGE_1800,
	CHIPID_CPU_VOLTAGE_1848,
};

static const enum chipid_voltage_index s8000a1_cpu[] = {
	CHIPID_CPU_VOLTAGE_BYPASS,
	CHIPID_CPU_VOLTAGE_SECUREROM,
	CHIPID_CPU_VOLTAGE_396,
	CHIPID_CPU_VOLTAGE_600,
	CHIPID_CPU_VOLTAGE_912,
	CHIPID_CPU_VOLTAGE_1200,
	CHIPID_CPU_VOLTAGE_1512,
	CHIPID_CPU_VOLTAGE_1800,
	CHIPID_CPU_VOLTAGE_1896,
};

static const enum chipid_voltage_index s8000_gpu[] = {
	CHIPID_GPU_VOLTAGE_OFF,
	CHIPID_GPU_VOLTAGE_340,
	CHIPID_GPU_VOLTAGE_474,
	CHIPID_GPU_VOLTAGE_550,
	CHIPID_GPU_VOLTAGE_723,
};

static const enum chipid_voltage_index s8003_gpu[] = {
	CHIPID_GPU_VOLTAGE_OFF,
	CHIPID_GPU_VOLTAGE_340,
	CHIPID_GPU_VOLTAGE_474,
	CHIPID_GPU_VOLTAGE_550,
	CHIPID_GPU_VOLTAGE_723,
};

static const enum chipid_voltage_index s8000a1_gpu[] = {
	CHIPID_GPU_VOLTAGE_OFF,
	CHIPID_GPU_VOLTAGE_340,
	CHIPID_GPU_VOLTAGE_474,
	CHIPID_GPU_VOLTAGE_550,
	CHIPID_GPU_VOLTAGE_616,
};

static const enum chipid_voltage_index s8001_cpu[] = {
	CHIPID_CPU_VOLTAGE_BYPASS,
	CHIPID_CPU_VOLTAGE_SECUREROM,
	CHIPID_CPU_VOLTAGE_396,
	CHIPID_CPU_VOLTAGE_720,
	CHIPID_CPU_VOLTAGE_1080,
	CHIPID_CPU_VOLTAGE_1440,
	CHIPID_CPU_VOLTAGE_1800,
	CHIPID_CPU_VOLTAGE_2160,
	CHIPID_CPU_VOLTAGE_2256_1core,
};

static const enum chipid_voltage_index s8001_gpu[] = {
	CHIPID_GPU_VOLTAGE_OFF,
	CHIPID_GPU_VOLTAGE_360,
	CHIPID_GPU_VOLTAGE_520,
	CHIPID_GPU_VOLTAGE_650,
	CHIPID_GPU_VOLTAGE_723,
};

static const enum chipid_voltage_index s8000_soc[] = {
	CHIPID_SOC_VOLTAGE_BYPASS,
	CHIPID_SOC_VOLTAGE_SECUREROM,
	CHIPID_SOC_VOLTAGE_VMIN,
	CHIPID_SOC_VOLTAGE_VNOM,
};

static const enum chipid_voltage_index s8003_soc[] = {
	CHIPID_SOC_VOLTAGE_BYPASS,
	CHIPID_SOC_VOLTAGE_SECUREROM,
	CHIPID_SOC_VOLTAGE_VMIN,
	CHIPID_SOC_VOLTAGE_VNOM,
};

static const enum chipid_voltage_index s8001_soc[] = {
	CHIPID_SOC_VOLTAGE_BYPASS,
	CHIPID_SOC_VOLTAGE_SECUREROM,
	CHIPID_SOC_VOLTAGE_VMIN,
	CHIPID_SOC_VOLTAGE_VMIN,
};

static const enum chipid_voltage_index s8001_j127_gpu[] = {
	CHIPID_GPU_VOLTAGE_OFF,
	CHIPID_GPU_VOLTAGE_360,
	CHIPID_GPU_VOLTAGE_520,
	CHIPID_GPU_VOLTAGE_650,
	CHIPID_GPU_VOLTAGE_723,
	CHIPID_GPU_VOLTAGE_804,
};

static const enum chipid_voltage_index s8001_j105_cpu[] = {
	CHIPID_CPU_VOLTAGE_BYPASS,
	CHIPID_CPU_VOLTAGE_SECUREROM,
	CHIPID_CPU_VOLTAGE_2256,
};

static const enum chipid_voltage_index s8001_j105_soc[] = {
	CHIPID_SOC_VOLTAGE_BYPASS,
	CHIPID_SOC_VOLTAGE_SECUREROM,
	CHIPID_SOC_VOLTAGE_VMIN,
	CHIPID_SOC_VOLTAGE_VMIN,
};

static const enum chipid_voltage_index s8001_j105_gpu[] = {
	CHIPID_GPU_VOLTAGE_OFF,
	CHIPID_GPU_VOLTAGE_723,
};

static const enum chipid_voltage_index s8001_j127_soc[] = {
	CHIPID_SOC_VOLTAGE_BYPASS,
	CHIPID_SOC_VOLTAGE_SECUREROM,
	CHIPID_SOC_VOLTAGE_VNOM,
	CHIPID_SOC_VOLTAGE_VNOM,
};

static const enum chipid_voltage_index s8001_j99a_soc[] = {
	CHIPID_SOC_VOLTAGE_BYPASS,
	CHIPID_SOC_VOLTAGE_SECUREROM,
	CHIPID_SOC_VOLTAGE_VMIN,
	CHIPID_SOC_VOLTAGE_VMIN,
};

struct dvfmperf {
	uint32_t chip_id;
	uint32_t chip_rev;
	uint32_t board_id;
	const enum chipid_voltage_index *voltage_indexes;
	uint32_t size;
};


// The dvfmperfs arays must be sorted to have :
// - BOARD_ID_NONE always coming last for chip/chip revision
// - Highest chip revision coming first
static const struct dvfmperf dvfmperfs_cpu[] = { // Entry with highest chip revision must come first
	{0x8000, CHIP_REVISION_B0, BOARD_ID_NONE, s8000_cpu, sizeof(s8000_cpu)/sizeof(s8000_cpu[0])},
	{0x8000, CHIP_REVISION_A1, BOARD_ID_NONE, s8000a1_cpu, sizeof(s8000a1_cpu)/sizeof(s8000a1_cpu[0])},
	{0x8003, CHIP_REVISION_A0, BOARD_ID_NONE, s8003_cpu, sizeof(s8003_cpu)/sizeof(s8003_cpu[0])},
	{0x8001, CHIP_REVISION_A0, J105_AP_BOARD_ID, s8001_j105_cpu, sizeof(s8001_j105_cpu)/sizeof(s8001_j105_cpu[0])},
	{0x8001, CHIP_REVISION_A0, J105_DEV_BOARD_ID, s8001_j105_cpu, sizeof(s8001_j105_cpu)/sizeof(s8001_j105_cpu[0])},
	{0x8001, CHIP_REVISION_A0, BOARD_ID_NONE, s8001_cpu, sizeof(s8001_cpu)/sizeof(s8000_cpu[0])},
};

static const struct dvfmperf dvfmperfs_gpu[] = { // Entry with highest chip revision must come first
	{0x8000, CHIP_REVISION_B0, BOARD_ID_NONE, s8000_gpu, sizeof(s8000_gpu)/sizeof(s8000_gpu[0])},
	{0x8000, CHIP_REVISION_A1, BOARD_ID_NONE, s8000a1_gpu, sizeof(s8000a1_gpu)/sizeof(s8000a1_gpu[0])},
	{0x8003, CHIP_REVISION_A0, BOARD_ID_NONE, s8003_gpu, sizeof(s8003_gpu)/sizeof(s8003_gpu[0])},
		{0x8001, CHIP_REVISION_A0, J128_AP_BOARD_ID, s8001_j127_gpu, sizeof(s8001_j127_gpu)/sizeof(s8001_j127_gpu[0])},
		{0x8001, CHIP_REVISION_A0, J128_DEV_BOARD_ID, s8001_j127_gpu, sizeof(s8001_j127_gpu)/sizeof(s8001_j127_gpu[0])},
		{0x8001, CHIP_REVISION_A0, J127_AP_BOARD_ID, s8001_j127_gpu, sizeof(s8001_j127_gpu)/sizeof(s8001_j127_gpu[0])},
		{0x8001, CHIP_REVISION_A0, J127_DEV_BOARD_ID, s8001_j127_gpu, sizeof(s8001_j127_gpu)/sizeof(s8001_j127_gpu[0])},
		{0x8001, CHIP_REVISION_A0, J105_AP_BOARD_ID, s8001_j105_gpu, sizeof(s8001_j105_gpu)/sizeof(s8001_j105_gpu[0])},
		{0x8001, CHIP_REVISION_A0, J105_DEV_BOARD_ID, s8001_j105_gpu, sizeof(s8001_j105_gpu)/sizeof(s8001_j105_gpu[0])},
	{0x8001, CHIP_REVISION_A0, BOARD_ID_NONE, s8001_gpu, sizeof(s8001_gpu)/sizeof(s8001_gpu[0])},
};

static const struct dvfmperf dvfmperfs_soc[] = { // Entry with highest chip revision must come first
	{0x8000, CHIP_REVISION_B0, BOARD_ID_NONE, s8000_soc, sizeof(s8000_soc)/sizeof(s8000_soc[0])},
	{0x8003, CHIP_REVISION_A0, BOARD_ID_NONE, s8003_soc, sizeof(s8003_soc)/sizeof(s8003_soc[0])},
		{0x8001, CHIP_REVISION_A0, J105_AP_BOARD_ID, s8001_j105_soc, sizeof(s8001_j105_soc)/sizeof(s8001_j105_soc[0])},
		{0x8001, CHIP_REVISION_A0, J105_DEV_BOARD_ID, s8001_j105_soc, sizeof(s8001_j105_soc)/sizeof(s8001_j105_soc[0])},
		{0x8001, CHIP_REVISION_A0, J128_AP_BOARD_ID, s8001_j127_soc, sizeof(s8001_j127_soc)/sizeof(s8001_j127_soc[0])},
		{0x8001, CHIP_REVISION_A0, J128_DEV_BOARD_ID, s8001_j127_soc, sizeof(s8001_j127_soc)/sizeof(s8001_j127_soc[0])},
		{0x8001, CHIP_REVISION_A0, J127_AP_BOARD_ID, s8001_j127_soc, sizeof(s8001_j127_soc)/sizeof(s8001_j127_soc[0])},
		{0x8001, CHIP_REVISION_A0, J127_DEV_BOARD_ID, s8001_j127_soc, sizeof(s8001_j127_soc)/sizeof(s8001_j127_soc[0])},
		{0x8001, CHIP_REVISION_A0, J99A_AP_BOARD_ID, s8001_j99a_soc, sizeof(s8001_j99a_soc)/sizeof(s8001_j99a_soc[0])},
		{0x8001, CHIP_REVISION_A0, J99A_DEV_BOARD_ID, s8001_j99a_soc, sizeof(s8001_j99a_soc)/sizeof(s8001_j99a_soc[0])},
		{0x8001, CHIP_REVISION_A0, J98A_AP_BOARD_ID, s8001_j99a_soc, sizeof(s8001_j99a_soc)/sizeof(s8001_j99a_soc[0])},
		{0x8001, CHIP_REVISION_A0, J98A_DEV_BOARD_ID, s8001_j99a_soc, sizeof(s8001_j99a_soc)/sizeof(s8001_j99a_soc[0])},
	{0x8001, CHIP_REVISION_A0, BOARD_ID_NONE, s8001_soc, sizeof(s8001_soc)/sizeof(s8001_soc[0])},
};

enum chipid_voltage_index dvfmperf_get_voltage_index(uint32_t index, enum chipid_voltage_type voltage_type)
{
	const struct dvfmperf *dvfmperfs;
	enum chipid_voltage_index voltage_index_for_filling;
	uint32_t chip_id = chipid_get_chip_id();
	uint32_t chip_rev = chipid_get_chip_revision();
	uint32_t board_id = platform_get_board_id();
	uint32_t i;
	uint32_t size = 0;

	switch (voltage_type) {
		case CHIPID_CPU_VOLTAGE:
			dvfmperfs = dvfmperfs_cpu;
			voltage_index_for_filling = CHIPID_CPU_VOLTAGE_396;
			size = sizeof(dvfmperfs_cpu)/sizeof(dvfmperfs_cpu[0]);
			break;
		case CHIPID_GPU_VOLTAGE:
			dvfmperfs = dvfmperfs_gpu;
			size = sizeof(dvfmperfs_gpu)/sizeof(dvfmperfs_gpu[0]);
			voltage_index_for_filling = CHIPID_GPU_VOLTAGE_OFF;
			break;
		case CHIPID_SOC_VOLTAGE:
			dvfmperfs = dvfmperfs_soc;
			size = sizeof(dvfmperfs_soc)/sizeof(dvfmperfs_soc[0]);
			break;
		default:
			panic("Invalid voltage type\n");
			voltage_index_for_filling = CHIPID_GPU_VOLTAGE_OFF; // prevent compiler warning
			dvfmperfs = NULL; // Prevent compiler warning
			break;
	}


	for (i = 0; i < size; i++) {
		if (chip_id != dvfmperfs[i].chip_id) {
			continue;
		}
		if (chip_rev < dvfmperfs[i].chip_rev) {
			continue;
		}
		if ((board_id != dvfmperfs[i].board_id) && (dvfmperfs[i].board_id != BOARD_ID_NONE)) {
			continue;
		}
		if (index >= dvfmperfs[i].size) {
			if (voltage_type == CHIPID_SOC_VOLTAGE) {
				if (dvfmperfs[i].size < 4) {
					panic("Invalid number of SOC perf states");
				}
				voltage_index_for_filling = dvfmperfs[i].voltage_indexes[2];
			}
			return voltage_index_for_filling;
		}
		return dvfmperfs[i].voltage_indexes[index];
	}
	panic("Unknown voltage for current chip\n");
	return CHIPID_GPU_VOLTAGE_OFF; // Never reach this code
}
