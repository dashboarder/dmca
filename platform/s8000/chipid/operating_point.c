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
#include <platform/soc/chipid.h>

#define CHIP_ID_ALL 0
#define DELIMITER CHIPID_ALL_VOLTAGE_LAST

static const struct operating_point_params operating_point_params[] = {
	// GPU
	{.chipid_max = DELIMITER, .chip_id = 0x8000, .voltage_type = CHIPID_GPU_VOLTAGE},
	//                                    P   M  S
	//                                    |   |  |
	{{.gpu = {CHIPID_GPU_VOLTAGE_OFF,0,0, 0,  0, 0}}},
	{{.gpu = {CHIPID_GPU_VOLTAGE_340,0,0, 2,170, 5}}},
	{{.gpu = {CHIPID_GPU_VOLTAGE_474,0,0, 2,158, 3}}},
	{{.gpu = {CHIPID_GPU_VOLTAGE_550,0,0, 3,275, 3}}},
	{{.gpu = {CHIPID_GPU_VOLTAGE_616,0,0, 3,154, 1}}},
	{{.gpu = {CHIPID_GPU_VOLTAGE_723,0,0, 1, 90, 2}}}, // It's 720 now for <rdar://problem/22294854> Change GPU & SoC PLL settings to recover yield
	{{.gpu = {CHIPID_GPU_VOLTAGE_804,0,0, 1, 67, 1}}},
	{{.gpu = {CHIPID_GPU_VOLTAGE_850,0,0, 6,425, 1}}},
	{{.gpu = {CHIPID_GPU_VOLTAGE_900,0,0, 1, 75, 1}}},

	{.chipid_max = DELIMITER, .chip_id = 0x8003, .voltage_type = CHIPID_GPU_VOLTAGE},
	//                                    P   M  S
	//                                    |   |  |
	{{.gpu = {CHIPID_GPU_VOLTAGE_OFF,0,0, 0,  0, 0}}},
	{{.gpu = {CHIPID_GPU_VOLTAGE_340,0,0, 2, 85, 2}}},
	{{.gpu = {CHIPID_GPU_VOLTAGE_474,0,0, 2, 79, 1}}},
	{{.gpu = {CHIPID_GPU_VOLTAGE_550,0,0, 4,275, 2}}},
	{{.gpu = {CHIPID_GPU_VOLTAGE_723,0,0, 4,241, 1}}},
	{{.gpu = {CHIPID_GPU_VOLTAGE_804,0,0, 1, 67, 1}}},
	{{.gpu = {CHIPID_GPU_VOLTAGE_850,0,0, 6,425, 1}}},
	{{.gpu = {CHIPID_GPU_VOLTAGE_900,0,0, 1, 75, 1}}},

	{.chipid_max = DELIMITER, .chip_id = 0x8001, .voltage_type = CHIPID_GPU_VOLTAGE},
	//                             P   M  S
	//                             |   |  |
	{{.gpu = {CHIPID_GPU_VOLTAGE_OFF,0,0, 0,  0, 0}}},
	{{.gpu = {CHIPID_GPU_VOLTAGE_360,0,0, 1, 60, 3}}},
	{{.gpu = {CHIPID_GPU_VOLTAGE_520,0,0, 1, 65, 2}}},
	{{.gpu = {CHIPID_GPU_VOLTAGE_650,0,0, 6,325, 1}}},
	{{.gpu = {CHIPID_GPU_VOLTAGE_723,0,0, 4,241, 1}}},
	{{.gpu = {CHIPID_GPU_VOLTAGE_800,0,0, 3,100, 0}}},
	{{.gpu = {CHIPID_GPU_VOLTAGE_804,0,0, 1, 67, 1}}},


	// S8000 B0
	{.chipid_max = DELIMITER, .chip_id = 0x8000, .voltage_type = CHIPID_CPU_VOLTAGE, .chip_rev = CHIP_REVISION_B0},
	//                              bypass
	//                              |  clkSrc
	//                              |  |  P   M  S  biuDiv4HiVol
	//                              |  |  |   |  |  |  biuDiv4LoVol
	//                              |  |  |   |  |  |  |    dvmrMaxWgt
	//                              |  |  |   |  |  |  |    |  iexrfCfgWrData
	//                              |  |  |   |  |  |  |    |  |  iexrfCfgWrIdxa
	//                              |  |  |   |  |  |  |    |  |  |  iexrfCfgWrIdxb
	//                              |  |  |   |  |  |  |    |  |  |  |  exrfCfgWrIdxmuxsel
	//                              |  |  |   |  |  |  |    |  |  |  |  |    nrgAccScaleTab(Ext)
	//                              |  |  |   |  |  |  |    |  |  |  |  |    |     lkgEstTab(Ext)
	//                              |  |  |   |  |  |  |    |  |  |  |  |    |     |     dpe0DvfmTab(Ext)
	//                              |  |  |   |  |  |  |    |  |  |  |  |    |     |     |
	{{{CHIPID_CPU_VOLTAGE_BYPASS,   0, 0, 0, 0,  0, 2, 2, 0xf, 1, 0, 0, 0, 0x1,  0x1,  0xff}}},
	{{{CHIPID_CPU_VOLTAGE_SECUREROM,1, 1, 1, 50, 3, 2, 2, 0xf, 1, 0, 0, 0, 0x1,  0x1,  0xff}}}, // 300MHz
	{{{CHIPID_CPU_VOLTAGE_396,      0, 1, 1, 66, 3, 2, 2, 0xf, 1, 0, 0, 0, 0x37, 0x30, 0xff}}},
	{{{CHIPID_CPU_VOLTAGE_600,      0, 1, 1, 75, 2, 2, 3, 0xf, 1, 0, 0, 0, 0x3e, 0x1f, 0xff}}},
	{{{CHIPID_CPU_VOLTAGE_912,      0, 1, 1, 76, 1, 4, 4, 0xf, 1, 0, 0, 0, 0x53, 0x14, 0xff}}},
	{{{CHIPID_CPU_VOLTAGE_1200,     0, 1, 1, 50, 0, 4, 5, 0xf, 1, 0, 0, 0, 0x66, 0x0f, 0xff}}},
	{{{CHIPID_CPU_VOLTAGE_1512,     0, 1, 1, 63, 0, 5, 6, 0xf, 1, 0, 0, 0, 0x88, 0x0c, 0xd7}}},
	{{{CHIPID_CPU_VOLTAGE_1800,     0, 1, 1, 75, 0, 6, 7, 0xf, 1, 0, 0, 0, 0xbf, 0x0b, 0x97}}},
	{{{CHIPID_CPU_VOLTAGE_1848,     0, 1, 1, 77, 0, 6, 7, 0x2, 1, 0, 0, 0, 0xbf, 0x0a, 0x93}}},
	{{{CHIPID_CPU_VOLTAGE_1896,     0, 1, 1, 79, 0, 6, 7, 0x2, 1, 0, 0, 0, 0xc5, 0x16, 0xff}}},
	{{{CHIPID_CPU_VOLTAGE_1992,     0, 1, 1, 83, 0, 6, 7, 0xf, 1, 0, 0, 0, 0xc8, 0x15, 0xff}}},
	{{{CHIPID_CPU_VOLTAGE_2112,     0, 1, 1, 88, 0, 7, 0, 0x2, 1, 0, 0, 0, 0xc8, 0x14, 0xff}}},
	{{{CHIPID_CPU_VOLTAGE_396_WA,   0, 1, 1, 82, 4, 2, 2, 0xf, 1, 0, 0, 0, 0x37, 0x30, 0xff}}},
	{{{CHIPID_CPU_VOLTAGE_1200_WA,  0, 1, 1, 76, 1, 4, 5, 0xf, 1, 0, 0, 0, 0x66, 0x0f, 0xff}}},
	{{{CHIPID_CPU_VOLTAGE_1512_WA,  0, 1, 1, 76, 1, 5, 6, 0xf, 1, 0, 0, 0, 0x88, 0x0c, 0xd7}}},

	// S8000 A1
	{.chipid_max = DELIMITER, .chip_id = 0x8000, .voltage_type = CHIPID_CPU_VOLTAGE},
	//                              bypass
	//                              |  clkSrc
	//                              |  |  P   M  S  biuDiv4HiVol
	//                              |  |  |   |  |  |  biuDiv4LoVol
	//                              |  |  |   |  |  |  |    dvmrMaxWgt
	//                              |  |  |   |  |  |  |    |  iexrfCfgWrData
	//                              |  |  |   |  |  |  |    |  |  iexrfCfgWrIdxa
	//                              |  |  |   |  |  |  |    |  |  |  iexrfCfgWrIdxb
	//                              |  |  |   |  |  |  |    |  |  |  |  exrfCfgWrIdxmuxsel
	//                              |  |  |   |  |  |  |    |  |  |  |  |    nrgAccScaleTab(Ext)
	//                              |  |  |   |  |  |  |    |  |  |  |  |    |     lkgEstTab(Ext)
	//                              |  |  |   |  |  |  |    |  |  |  |  |    |     |     dpe0DvfmTab(Ext)
	//                              |  |  |   |  |  |  |    |  |  |  |  |    |     |     |
	{{{CHIPID_CPU_VOLTAGE_BYPASS,   0, 0, 0, 0,  0, 2, 2, 0xf, 0, 0, 0, 0, 0x1,  0x1,  0xff}}},
	{{{CHIPID_CPU_VOLTAGE_SECUREROM,1, 1, 1, 50, 3, 2, 2, 0xf, 0, 0, 0, 0, 0x1,  0x1,  0xff}}}, // 300MHz
	{{{CHIPID_CPU_VOLTAGE_396,      0, 1, 1, 66, 3, 2, 2, 0xf, 0, 0, 0, 0, 0x3e, 0x46, 0xff}}},
	{{{CHIPID_CPU_VOLTAGE_600,      0, 1, 1, 50, 1, 2, 3, 0xf, 0, 0, 0, 0, 0x44, 0x2f, 0xff}}},
	{{{CHIPID_CPU_VOLTAGE_912,      0, 1, 1, 76, 1, 4, 4, 0xf, 0, 0, 0, 0, 0x5b, 0x1d, 0xff}}},
	{{{CHIPID_CPU_VOLTAGE_1200,     0, 1, 1, 50, 0, 4, 5, 0xf, 0, 0, 0, 0, 0x74, 0x16, 0xff}}},
	{{{CHIPID_CPU_VOLTAGE_1512,     0, 1, 1, 63, 0, 5, 6, 0xf, 0, 0, 0, 0, 0x91, 0x11, 0xd7}}},
	{{{CHIPID_CPU_VOLTAGE_1800,     0, 1, 1, 75, 0, 6, 7, 0xf, 0, 0, 0, 0, 0xaf, 0x0f, 0x97}}},
	{{{CHIPID_CPU_VOLTAGE_1896,     0, 1, 1, 79, 0, 6, 7, 0x2, 0, 0, 0, 0, 0xbc, 0x0e, 0xff}}},
	{{{CHIPID_CPU_VOLTAGE_1992,     0, 1, 1, 83, 0, 6, 7, 0xf, 0, 0, 0, 0, 0xc8, 0x0d, 0xff}}},
	{{{CHIPID_CPU_VOLTAGE_2112,     0, 1, 1, 88, 0, 7, 0, 0x2, 0, 0, 0, 0, 0xc8, 0x0c, 0xff}}},

	// S8001
	{.chipid_max = DELIMITER, .chip_id = 0x8001, .voltage_type = CHIPID_CPU_VOLTAGE},
	//                              bypass
	//                              |  clkSrc
	//                              |  |  P   M  S  biuDiv4HiVol
	//                              |  |  |   |  |  |  biuDiv4LoVol
	//                              |  |  |   |  |  |  |    dvmrMaxWgt
	//                              |  |  |   |  |  |  |    |  iexrfCfgWrData
	//                              |  |  |   |  |  |  |    |  |  iexrfCfgWrIdxa
	//                              |  |  |   |  |  |  |    |  |  |  iexrfCfgWrIdxb
	//                              |  |  |   |  |  |  |    |  |  |  |  exrfCfgWrIdxmuxsel
	//                              |  |  |   |  |  |  |    |  |  |  |  |    nrgAccScaleTab(Ext)
	//                              |  |  |   |  |  |  |    |  |  |  |  |    |     lkgEstTab(Ext)
	//                              |  |  |   |  |  |  |    |  |  |  |  |    |     |     dpe0DvfmTab(Ext)
	//                              |  |  |   |  |  |  |    |  |  |  |  |    |     |     |
	{{{CHIPID_CPU_VOLTAGE_BYPASS,   0, 0, 0, 0,  0, 2, 2, 0xf, 0, 0, 0, 0, 0x1,  0x1,  0xff}}},
	{{{CHIPID_CPU_VOLTAGE_SECUREROM,1, 1, 1, 75, 6, 2, 2, 0xf, 0, 0, 0, 0, 0x1,  0x1,  0xff}}}, // 300MHz
	{{{CHIPID_CPU_VOLTAGE_396,      0, 1, 1, 66, 3, 2, 2, 0xf, 0, 0, 0, 0, 0x3d, 0x2b, 0xff}}},
	{{{CHIPID_CPU_VOLTAGE_720,      0, 1, 1, 90, 2, 2, 2, 0xf, 0, 0, 0, 0, 0x4a, 0x17, 0xff}}},
	{{{CHIPID_CPU_VOLTAGE_1080,     0, 1, 1, 90, 1, 3, 3, 0xf, 0, 0, 0, 0, 0x58, 0x0e, 0xff}}},
	{{{CHIPID_CPU_VOLTAGE_1440,     0, 1, 1, 60, 0, 4, 4, 0xf, 0, 0, 0, 0, 0x6b, 0x0a, 0xfb}}},
	{{{CHIPID_CPU_VOLTAGE_1800,     0, 1, 1, 75, 0, 5, 5, 0xf, 0, 0, 0, 0, 0x99, 0x08, 0xa8}}},
	{{{CHIPID_CPU_VOLTAGE_2160,     0, 1, 1, 90, 0, 5, 5, 0xf, 0, 0, 0, 0, 0xc1, 0x06, 0x7c}}},
	{{{CHIPID_CPU_VOLTAGE_2256_1core,0,1, 1, 94, 0, 5, 5, 0x2, 0, 0, 0, 0, 0xc1, 0x06, 0x77}}},
	{{{CHIPID_CPU_VOLTAGE_2256,     0, 1, 1, 94, 0, 5, 5, 0xf, 0, 0, 0, 0, 0xc1, 0x06, 0x77}}},
	{{{CHIPID_CPU_VOLTAGE_2352,     0, 1, 1, 98, 0, 5, 5, 0xf, 0, 0, 0, 0, 0xe0, 0x0d, 0xff}}},
	{{{CHIPID_CPU_VOLTAGE_2448,     0, 1, 1, 102,0, 6, 6, 0x2, 0, 0, 0, 0, 0xe0, 0x0d, 0xff}}},

	// S8003
	{.chipid_max = DELIMITER, .chip_id = 0x8003, .voltage_type = CHIPID_CPU_VOLTAGE},
	//                              bypass
	//                              |  clkSrc
	//                              |  |  P   M  S  biuDiv4HiVol
	//                              |  |  |   |  |  |  biuDiv4LoVol
	//                              |  |  |   |  |  |  |    dvmrMaxWgt
	//                              |  |  |   |  |  |  |    |  iexrfCfgWrData
	//                              |  |  |   |  |  |  |    |  |  iexrfCfgWrIdxa
	//                              |  |  |   |  |  |  |    |  |  |  iexrfCfgWrIdxb
	//                              |  |  |   |  |  |  |    |  |  |  |  exrfCfgWrIdxmuxsel
	//                              |  |  |   |  |  |  |    |  |  |  |  |    nrgAccScaleTab(Ext)
	//                              |  |  |   |  |  |  |    |  |  |  |  |    |     lkgEstTab(Ext)
	//                              |  |  |   |  |  |  |    |  |  |  |  |    |     |     dpe0DvfmTab(Ext)
	//                              |  |  |   |  |  |  |    |  |  |  |  |    |     |     |
	{{{CHIPID_CPU_VOLTAGE_BYPASS,   0, 0, 0, 0,  0, 2, 2, 0xf, 0, 0, 0, 0, 0x1,  0x1,  0xff}}},
	{{{CHIPID_CPU_VOLTAGE_SECUREROM,1, 1, 1, 75, 6, 2, 2, 0xf, 0, 0, 0, 0, 0x1,  0x1,  0xff}}}, // 300MHz
	{{{CHIPID_CPU_VOLTAGE_396,      0, 1, 1, 66, 3, 2, 2, 0xf, 0, 0, 0, 0, 0x3a, 0x2c, 0xff}}},
	{{{CHIPID_CPU_VOLTAGE_600,      0, 1, 1, 75, 2, 2, 3, 0xf, 0, 0, 0, 0, 0x40, 0x1c, 0xff}}},
	{{{CHIPID_CPU_VOLTAGE_912,      0, 1, 1, 76, 1, 4, 4, 0xf, 0, 0, 0, 0, 0x4f, 0x11, 0xff}}},
	{{{CHIPID_CPU_VOLTAGE_1200,     0, 1, 1,100, 1, 4, 5, 0xf, 0, 0, 0, 0, 0x5e, 0x0d, 0xff}}},
	{{{CHIPID_CPU_VOLTAGE_1512,     0, 1, 1, 63, 0, 5, 6, 0xf, 0, 0, 0, 0, 0x73, 0x0a, 0xe7}}},
	{{{CHIPID_CPU_VOLTAGE_1800,     0, 1, 1, 75, 0, 6, 7, 0xf, 0, 0, 0, 0, 0x82, 0x08, 0xb6}}},
	{{{CHIPID_CPU_VOLTAGE_1848,     0, 1, 1, 77, 0, 6, 7, 0x2, 0, 0, 0, 0, 0x82, 0x08, 0xb1}}},
	{{{CHIPID_CPU_VOLTAGE_1896,     0, 1, 1, 79, 0, 6, 7, 0xf, 0, 0, 0, 0, 0xa7, 0x10, 0xff}}},
	{{{CHIPID_CPU_VOLTAGE_1992,     0, 1, 1, 83, 0, 6, 7, 0xf, 0, 0, 0, 0, 0xb2, 0x10, 0xff}}},
	{{{CHIPID_CPU_VOLTAGE_2112,     0, 1, 1, 88, 0, 7, 0, 0x2, 0, 0, 0, 0, 0xbf, 0x0f, 0xff}}},
};

const struct operating_point_params *operating_point_get_params(enum chipid_voltage_index voltage_index, enum chipid_voltage_type voltage_type)
{
	uint64_t chip_id = chipid_get_chip_id();
	uint64_t chip_rev = chipid_get_chip_revision();
	uint64_t chip_id_in_array = 0;
	uint64_t chip_rev_in_array = 0;
	size_t size = sizeof(operating_point_params)/sizeof(operating_point_params[0]);
	enum chipid_voltage_type voltage_type_in_array = CHIPID_CPU_VOLTAGE;

	size_t found = size;
	for (size_t i = 0; i < size; i++) {
		if (operating_point_params[i].voltage_index == DELIMITER) {
			chip_id_in_array = operating_point_params[i].chip_id;
			chip_rev_in_array = operating_point_params[i].chip_rev;
			voltage_type_in_array = operating_point_params[i].voltage_type;
			continue;
		}
		if ((voltage_type_in_array != voltage_type)) {
			continue;
		}
		if (chip_rev_in_array > chip_rev) {
			continue;
		}
		if ((operating_point_params[i].voltage_index == voltage_index) && ((chip_id_in_array == CHIP_ID_ALL) || (chip_id_in_array == chip_id))) {
			found = i;
			break;
		}
	}
	if (found == size) {
		panic("%s Index %d not found for chipId 0x%llx\n",
			__func__, voltage_index, chip_id);
	}
	return &operating_point_params[found];
}
