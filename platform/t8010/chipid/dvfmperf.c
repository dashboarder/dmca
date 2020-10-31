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

static const enum chipid_voltage_index t8010_cpu[] = {
	CHIPID_CPU_VOLTAGE_BYPASS,
	CHIPID_CPU_VOLTAGE_SECUREROM,
	CHIPID_CPU_VOLTAGE_396,
};

static const enum chipid_voltage_index t8010_gpu[] = {
	CHIPID_GPU_VOLTAGE_OFF,
};

static const struct {
	uint32_t chip_id;
	uint32_t chip_rev;
	enum chipid_voltage_type voltage_type;
	const enum chipid_voltage_index *voltage_indexes;
	uint32_t size;
} dvfmperfs[] = { // Entry with highest chip revision must come first
	{0x8010, CHIP_REVISION_A0, CHIPID_CPU_VOLTAGE, t8010_cpu, sizeof(t8010_cpu)/sizeof(t8010_cpu[0])},
	{0x8010, CHIP_REVISION_A0, CHIPID_GPU_VOLTAGE, t8010_gpu, sizeof(t8010_gpu)/sizeof(t8010_gpu[0])},
};

enum chipid_voltage_index dvfmperf_get_voltage_index(uint32_t index, enum chipid_voltage_type voltage_type)
{
	uint32_t chip_id = chipid_get_chip_id();
	uint32_t chip_rev = chipid_get_chip_revision();
	uint32_t i;

	for (i = 0; i < sizeof(dvfmperfs)/sizeof(dvfmperfs[0]); i++) {
		if (chip_id != dvfmperfs[i].chip_id) {
			continue;
		}
		if (chip_rev < dvfmperfs[i].chip_rev) {
			continue;
		}
		if (voltage_type != dvfmperfs[i].voltage_type) {
			continue;
		}
		if (index >= dvfmperfs[i].size) {
			switch (voltage_type) {
				case CHIPID_CPU_VOLTAGE:
					return dvfmperfs[i].voltage_indexes[2]; // Default to lowest frequency
				case CHIPID_GPU_VOLTAGE:
					return dvfmperfs[i].voltage_indexes[0];
				default:
					panic("Unknwon voltage type\n");
			}
		}
		return dvfmperfs[i].voltage_indexes[index];
	}
	panic("Unknown voltage for current chip\n");
	return CHIPID_GPU_VOLTAGE_OFF; // Never reach this code
}
