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
 * This file is only a database of the frequencies used by CPU and GPU, it has no access
 * To the chip.
 */
#include <platform.h>
#include <platform/soc/operating_point.h>

#define CHIP_ID_ALL 0
#define DELIMITER CHIPID_ALL_VOLTAGE_LAST

static const struct operating_point_params operating_point_params[] = {
	// T8010
	{.voltage_index = DELIMITER, .chip_id = 0x8010, .voltage_type = CHIPID_GPU_VOLTAGE},
	//  voltage_index                   P   M  S
	{.gpu = {CHIPID_GPU_VOLTAGE_OFF,    0,  0, 0}},

	// T8010
	{.voltage_index = DELIMITER, .chip_id = 0x8010, .voltage_type = CHIPID_CPU_VOLTAGE},
	//                                    bypass
	//                                    |  clkSrc
	//                                    |  |    fcwInt
	//                                    |  |    |   fcwFrac
	//                                    |  |    |   |  pstDivS
	//                                    |  |    |   |  | migDivS
	//                                    |  |    |   |  |  |                   coreTyp
	//                                    |  |    |   |  |  |                   |  biuDiv4HiVol
	//                                    |  |    |   |  |  |                   |  |  biuDiv4LoVol
	//                                    |  |    |   |  |  |                   |  |  |    dvmrMaxWgt
	//                                    |  |    |   |  |  |                   |  |  |    |  iexrfCfgWrData
	//                                    |  |    |   |  |  |                   |  |  |    |  |  iexrfCfgWrIdxa
	//                                    |  |    |   |  |  |                   |  |  |    |  |  |  iexrfCfgWrIdxb
	//                                    |  |    |   |  |  |                   |  |  |    |  |  |  |  exrfCfgWrIdxmuxsel
	//                                    |  |    |   |  |  |                   |  |  |    |  |  |  |  |
	{.cpu = {CHIPID_CPU_VOLTAGE_BYPASS,   1, 0, 125,  0, 7, 7, kACC_CORETYP_FUSED, 2, 2, 0xf, 0, 0, 0, 0}}, // 24 MHz
	{.cpu = {CHIPID_CPU_VOLTAGE_SECUREROM,1, 1, 125,  0, 4, 7, kACC_CORETYP_FUSED, 2, 2, 0xf, 0, 0, 0, 0}}, // 300 MHz
	{.cpu = {CHIPID_CPU_VOLTAGE_396,      0, 0,   0,  0, 0, 0, kACC_CORETYP_ECORE, 2, 2, 0xf, 0, 0, 0, 0}}, // 396 MHz !!!FIXME!!! Get correct settings
};

const struct operating_point_params *operating_point_get_params(enum chipid_voltage_index voltage_index, enum chipid_voltage_type voltage_type)
{
	uint64_t chip_id = chipid_get_chip_id();
	uint64_t chip_id_in_array = 0;
	size_t size = sizeof(operating_point_params)/sizeof(operating_point_params[0]);
	enum chipid_voltage_type voltage_type_in_array = CHIPID_CPU_VOLTAGE;

	size_t found = size;
	for (size_t i = 0; i < size; i++) {
		if (operating_point_params[i].voltage_index == DELIMITER) {
			chip_id_in_array = operating_point_params[i].chip_id;
			voltage_type_in_array = operating_point_params[i].voltage_type;
			continue;
		}
		if ((voltage_type_in_array != voltage_type)) {
			continue;
		}
		if ((operating_point_params[i].voltage_index == voltage_index) && ((chip_id_in_array == CHIP_ID_ALL) || (chip_id_in_array == chip_id))) {
			found = i;
		}
	}
	if (found == size) {
		panic("%s Index %d not found for chipId 0x%llx\n",
			__func__, voltage_index, chip_id);
	}
	return &operating_point_params[found];
}
