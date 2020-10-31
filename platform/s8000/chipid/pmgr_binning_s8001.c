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

//
// Please update test plan version to know what is the reference.
//
// Elba Test Plan Version 1.6
// Elba Tunables Specification Revision 0.46
//

#include <platform/pmgr.h>
#include <platform/soc/chipid.h>

//
// Fuses in MINIPMGR are linearly mapped
//
static const struct pmgr_binning_fuse_to_register minipmgr_fuse_register[] = {
	//   binning_low
	//   |    binning_high
	//   |    |  register_low
	//   |    |  |  register_64bit
	//   |    |  |  | register_address
	//   |    |  |  |  |
	{    0, 369, 0, 0, AOP_MINIPMGR_BASE_ADDR + MINIPMGR_FUSE_CFG_FUSE0_OFFSET},
	{    0,   0, 0, 0,                                                       0},
};

//
// Fuses in ACC (UDR) are sparsely mapped
//
static const struct pmgr_binning_fuse_to_register acc_fuse_register[] = {
	//   binning_low
	//   |    binning_high
	//   |    |  register_low
	//   |    |  |  register_64bit
	//   |    |  |  | register_address
	//   |    |  |  |  |
	{    0,   2, 0, 1, ACC_BASE_ADDR + ACC_PWRCTL_DVFM_ST0_EXT2_OFFSET},
	{    3,   6, 4, 1, ACC_BASE_ADDR + ACC_PWRCTL_DVFM_ST0_EXT2_OFFSET},
	{    7,   9, 0, 1, ACC_BASE_ADDR + ACC_PWRCTL_DVFM_ST1_EXT2_OFFSET},
	{   10,  13, 4, 1, ACC_BASE_ADDR + ACC_PWRCTL_DVFM_ST1_EXT2_OFFSET},
	{   14,  16, 0, 1, ACC_BASE_ADDR + ACC_PWRCTL_DVFM_ST2_EXT2_OFFSET},
	{   17,  20, 4, 1, ACC_BASE_ADDR + ACC_PWRCTL_DVFM_ST2_EXT2_OFFSET},
	{   21,  23, 0, 1, ACC_BASE_ADDR + ACC_PWRCTL_DVFM_ST3_EXT2_OFFSET},
	{   24,  27, 4, 1, ACC_BASE_ADDR + ACC_PWRCTL_DVFM_ST3_EXT2_OFFSET},
	{   28,  30, 0, 1, ACC_BASE_ADDR + ACC_PWRCTL_DVFM_ST4_EXT2_OFFSET},
	{   31,  34, 4, 1, ACC_BASE_ADDR + ACC_PWRCTL_DVFM_ST4_EXT2_OFFSET},
	{   35,  37, 0, 1, ACC_BASE_ADDR + ACC_PWRCTL_DVFM_ST5_EXT2_OFFSET},
	{   38,  41, 4, 1, ACC_BASE_ADDR + ACC_PWRCTL_DVFM_ST5_EXT2_OFFSET},
	{   42,  44, 0, 1, ACC_BASE_ADDR + ACC_PWRCTL_DVFM_ST6_EXT2_OFFSET},
	{   45,  48, 4, 1, ACC_BASE_ADDR + ACC_PWRCTL_DVFM_ST6_EXT2_OFFSET},
	{   40,  51, 0, 1, ACC_BASE_ADDR + ACC_PWRCTL_DVFM_ST7_EXT2_OFFSET},
	{   52,  55, 4, 1, ACC_BASE_ADDR + ACC_PWRCTL_DVFM_ST7_EXT2_OFFSET},
	{   56,  58, 0, 1, ACC_BASE_ADDR + ACC_PWRCTL_DVFM_ST8_EXT2_OFFSET},
	{   59,  62, 4, 1, ACC_BASE_ADDR + ACC_PWRCTL_DVFM_ST8_EXT2_OFFSET},
	{  112, 114,29, 0, ACC_BASE_ADDR + ACC_PWRCTL_EFUSE_REV_OFFSET},
	{  115, 119,24, 0, ACC_BASE_ADDR + ACC_PWRCTL_EFUSE_REV_OFFSET},
	{  120, 127,16, 0, ACC_BASE_ADDR + ACC_PWRCTL_EFUSE_REV_OFFSET},
	{   0,    0, 0, 0,                                               0},
};

const struct pmgr_binning_mode_to_fuse pmgr_binning_mode_to_fuse_data[] = {
	//
	// CPU
	//

	//type
	//|                 mode
	//|                 |  bingroup
	//|                 |  |                         low
	//|                 |  |                         |    high
	//|                 |  |                         |    |  fuse_to_register
	//|                 |  |                         |    |  |
	{PMGR_BINNING_CPU,  1, PMGR_BINNING_GROUP_ALL,   0,   6, acc_fuse_register},
	{PMGR_BINNING_CPU,  2, PMGR_BINNING_GROUP_ALL,   7,  13, acc_fuse_register},
	{PMGR_BINNING_CPU,  3, PMGR_BINNING_GROUP_ALL,  14,  20, acc_fuse_register},
	{PMGR_BINNING_CPU,  4, PMGR_BINNING_GROUP_ALL,  21,  27, acc_fuse_register},
	{PMGR_BINNING_CPU,  5, PMGR_BINNING_GROUP_ALL,  28,  34, acc_fuse_register},
	{PMGR_BINNING_CPU,  6, PMGR_BINNING_GROUP_ALL,  35,  41, acc_fuse_register},
	{PMGR_BINNING_CPU,  7, PMGR_BINNING_GROUP_ALL,  42,  48, acc_fuse_register},
	{PMGR_BINNING_CPU,  8, PMGR_BINNING_GROUP_ALL,  49,  55, acc_fuse_register},

	//
	// GPU/SOC
	//

	//type
	//|                 mode
	//|                 |  bingroup
	//|                 |  |                         low
	//|                 |  |                         |    high
	//|                 |  |                         |    |  fuse_to_register
	//|                 |  |                         |    |  |
	{PMGR_BINNING_GPU,  1, PMGR_BINNING_GROUP_ALL, 238, 244, minipmgr_fuse_register},
	{PMGR_BINNING_GPU,  2, PMGR_BINNING_GROUP_ALL, 245, 251, minipmgr_fuse_register},
	{PMGR_BINNING_GPU,  3, PMGR_BINNING_GROUP_ALL, 252, 258, minipmgr_fuse_register},
	{PMGR_BINNING_GPU,  4, PMGR_BINNING_GROUP_ALL, 259, 265, minipmgr_fuse_register},
	{PMGR_BINNING_GPU,  5, PMGR_BINNING_GROUP_ALL, 266, 272, minipmgr_fuse_register},

	{PMGR_BINNING_SOC,  1, PMGR_BINNING_GROUP_ALL, 224, 230, minipmgr_fuse_register},
	{PMGR_BINNING_SOC,  2, PMGR_BINNING_GROUP_ALL, 231, 237, minipmgr_fuse_register},

	{PMGR_BINNING_BASE, 0, PMGR_BINNING_GROUP_ALL, 115, 119, acc_fuse_register},

	{PMGR_BINNING_REV,  0, PMGR_BINNING_GROUP_ALL, 142, 147, minipmgr_fuse_register},


	{PMGR_BINNING_END_OF_LIST, 0, PMGR_BINNING_GROUP_ALL, 0, 0, NULL},
};

//
// Group-specific entries must come before the corresponding entry with PMGR_BINNING_GROUP_ALL.
//
const struct pmgr_binning_voltage_index_to_config pmgr_binning_voltage_index_to_config_data[] = {
	// voltage index
	// |                             safe voltage
	// |                             |    	mode
	// |                             |    	|   					type
	// |                             |    	|   					|                 binning group
	// |                             |    	|   					|                 |                       fuse revision minimun
	// |                             |    	|   					|                 |                       |
	{CHIPID_CPU_VOLTAGE_BYPASS,		606, 	PMGR_BINNING_MODE_NONE, PMGR_BINNING_CPU, PMGR_BINNING_GROUP_ALL, 0},
	{CHIPID_CPU_VOLTAGE_SECUREROM,	606, 	PMGR_BINNING_MODE_NONE, PMGR_BINNING_CPU, PMGR_BINNING_GROUP_ALL, 0},
	{CHIPID_CPU_VOLTAGE_396,		606, 	1, 						PMGR_BINNING_CPU, PMGR_BINNING_GROUP_ALL, 0},
	{CHIPID_CPU_VOLTAGE_720,		606, 	2, 						PMGR_BINNING_CPU, PMGR_BINNING_GROUP_ALL, 0},
	{CHIPID_CPU_VOLTAGE_1080,		720, 	3, 						PMGR_BINNING_CPU, PMGR_BINNING_GROUP_ALL, 0},
	{CHIPID_CPU_VOLTAGE_1440,		833, 	4, 						PMGR_BINNING_CPU, PMGR_BINNING_GROUP_ALL, 0},
	{CHIPID_CPU_VOLTAGE_1800,		947,	5, 						PMGR_BINNING_CPU, PMGR_BINNING_GROUP_ALL, 0},
	{CHIPID_CPU_VOLTAGE_2160,		1060, 	6, 						PMGR_BINNING_CPU, PMGR_BINNING_GROUP_ALL, 0},
	{CHIPID_CPU_VOLTAGE_2256_1core,	1060, 	6, 						PMGR_BINNING_CPU, PMGR_BINNING_GROUP_ALL, 0},
	{CHIPID_CPU_VOLTAGE_2256,		1090, 	7, 						PMGR_BINNING_CPU, PMGR_BINNING_GROUP_ALL, 0},
	{CHIPID_CPU_VOLTAGE_2352,		1130, 	PMGR_BINNING_MODE_NONE, PMGR_BINNING_CPU, PMGR_BINNING_GROUP_ALL, 0},
	{CHIPID_CPU_VOLTAGE_2448,		1130, 	PMGR_BINNING_MODE_NONE, PMGR_BINNING_CPU, PMGR_BINNING_GROUP_ALL, 0},

	// There is only one SOC voltage state on S8001
	// But Test Plan named MS001 for J99a and MS002 for J127
	// and this is like S8000 which call MS1 for VMIN and MS2 for VNOM
	{CHIPID_SOC_VOLTAGE_VMIN,		800,	PMGR_BINNING_MODE_NONE, PMGR_BINNING_SOC, PMGR_BINNING_GROUP_ALL, 0},
	{CHIPID_SOC_VOLTAGE_VNOM,		900,	PMGR_BINNING_MODE_NONE, PMGR_BINNING_SOC, PMGR_BINNING_GROUP_ALL, 0},

	{CHIPID_GPU_VOLTAGE_OFF,		0,      PMGR_BINNING_MODE_NONE, PMGR_BINNING_GPU, PMGR_BINNING_GROUP_ALL, 0},
	{CHIPID_GPU_VOLTAGE_360,		670, 	1, 						PMGR_BINNING_GPU, PMGR_BINNING_GROUP_ALL, 0},
	{CHIPID_GPU_VOLTAGE_520,		800, 	2, 						PMGR_BINNING_GPU, PMGR_BINNING_GROUP_ALL, 0},
	{CHIPID_GPU_VOLTAGE_650,		920, 	3, 						PMGR_BINNING_GPU, PMGR_BINNING_GROUP_ALL, 0},
	{CHIPID_GPU_VOLTAGE_723,		1030, 	4, 						PMGR_BINNING_GPU, PMGR_BINNING_GROUP_ALL, 0},
	{CHIPID_GPU_VOLTAGE_800,		1030,	PMGR_BINNING_MODE_NONE, PMGR_BINNING_GPU, PMGR_BINNING_GROUP_ALL, 0},
	{CHIPID_GPU_VOLTAGE_804,		1100, 	5, 						PMGR_BINNING_GPU, PMGR_BINNING_GROUP_ALL, 0},
};

//
// Safe voltage of 0 and binning mode != PMGR_BINNING_MODE_NONE
// means data are store in pmgr_binning_mode_to_const_data
//
const struct pmgr_binning_voltage_config pmgr_binning_voltage_config_sram_data[] = {
	//                                         safe voltage (0 means lookup data in pmgr_binning_mode_to_const_data)
	//                                         |  mode
	//                                         |  | 						type
	//                                         |  | 						|
	[CHIPID_CPU_VOLTAGE_396]			= {    0, 1,						PMGR_BINNING_CPU_SRAM},
	[CHIPID_CPU_VOLTAGE_720]			= {    0, 2,						PMGR_BINNING_CPU_SRAM},
	[CHIPID_CPU_VOLTAGE_1080]			= {    0, 3,						PMGR_BINNING_CPU_SRAM},
	[CHIPID_CPU_VOLTAGE_1440]			= {    0, 4,						PMGR_BINNING_CPU_SRAM},
	[CHIPID_CPU_VOLTAGE_1800]			= {    0, 5,						PMGR_BINNING_CPU_SRAM},
	[CHIPID_CPU_VOLTAGE_2160]			= {    0, 6,						PMGR_BINNING_CPU_SRAM},
	[CHIPID_CPU_VOLTAGE_2256_1core]		= {    0, 7,						PMGR_BINNING_CPU_SRAM},
	[CHIPID_CPU_VOLTAGE_2256]			= { 1090, PMGR_BINNING_MODE_NONE,	PMGR_BINNING_CPU_SRAM},
	[CHIPID_CPU_VOLTAGE_2352]			= { 1130, PMGR_BINNING_MODE_NONE,	PMGR_BINNING_CPU_SRAM},
	[CHIPID_CPU_VOLTAGE_2448]			= { 1130, PMGR_BINNING_MODE_NONE,	PMGR_BINNING_CPU_SRAM},

	[CHIPID_GPU_VOLTAGE_OFF]			= {    0, PMGR_BINNING_MODE_NONE,	PMGR_BINNING_GPU_SRAM},
	[CHIPID_GPU_VOLTAGE_360]			= {    0, 1,						PMGR_BINNING_GPU_SRAM},
	[CHIPID_GPU_VOLTAGE_520]			= {    0, 2,						PMGR_BINNING_GPU_SRAM},
	[CHIPID_GPU_VOLTAGE_650]			= {    0, 3,						PMGR_BINNING_GPU_SRAM},
	[CHIPID_GPU_VOLTAGE_723]			= {    0, 4,						PMGR_BINNING_GPU_SRAM},
	[CHIPID_GPU_VOLTAGE_800]			= { 1030, PMGR_BINNING_MODE_NONE,	PMGR_BINNING_GPU_SRAM},
	[CHIPID_GPU_VOLTAGE_804]			= {    0, 5,						PMGR_BINNING_GPU_SRAM},

	[CHIPID_VOLTAGE_FIXED]				= { 900,  PMGR_BINNING_MODE_NONE,	PMGR_BINNING_SRAM},
};

//
// Group-specific entries must come before the corresponding entry with PMGR_BINNING_GROUP_ALL.
//
const struct pmgr_binning_mode_to_const pmgr_binning_mode_to_const_data[] = {
	// Voltage
	// |   Mode
	// |   |  Type
	// |   |  |                      Binning group
	// |   |  |                      |                       fuse revision minimun
	// |   |  |                      |                       |
	{ 820, 4, PMGR_BINNING_CPU_SRAM, PMGR_BINNING_GROUP_ALL, 8},
	{ 930, 5, PMGR_BINNING_CPU_SRAM, PMGR_BINNING_GROUP_ALL, 8},
	{ 990, 6, PMGR_BINNING_CPU_SRAM, PMGR_BINNING_GROUP_ALL, 8},
	{1000, 7, PMGR_BINNING_CPU_SRAM, PMGR_BINNING_GROUP_ALL, 8},

	{ 835, 4, PMGR_BINNING_CPU_SRAM, PMGR_BINNING_GROUP_ALL, 6},
	{ 885, 5, PMGR_BINNING_CPU_SRAM, PMGR_BINNING_GROUP_ALL, 6},
	{1000, 6, PMGR_BINNING_CPU_SRAM, PMGR_BINNING_GROUP_ALL, 6},
	{1000, 7, PMGR_BINNING_CPU_SRAM, PMGR_BINNING_GROUP_ALL, 6},

	{ 800, 1, PMGR_BINNING_CPU_SRAM, PMGR_BINNING_GROUP_ALL, 0},
	{ 800, 2, PMGR_BINNING_CPU_SRAM, PMGR_BINNING_GROUP_ALL, 0},
	{ 800, 3, PMGR_BINNING_CPU_SRAM, PMGR_BINNING_GROUP_ALL, 0},
	{ 833, 4, PMGR_BINNING_CPU_SRAM, PMGR_BINNING_GROUP_ALL, 0},
	{ 947, 5, PMGR_BINNING_CPU_SRAM, PMGR_BINNING_GROUP_ALL, 0},
	{1060, 6, PMGR_BINNING_CPU_SRAM, PMGR_BINNING_GROUP_ALL, 0},
	{1060, 7, PMGR_BINNING_CPU_SRAM, PMGR_BINNING_GROUP_ALL, 0},

	{1100, 5, PMGR_BINNING_GPU_SRAM, PMGR_BINNING_GROUP_ALL, 10},

	{ 850, 2, PMGR_BINNING_GPU_SRAM, PMGR_BINNING_GROUP_ALL, 6},
	{ 930, 3, PMGR_BINNING_GPU_SRAM, PMGR_BINNING_GROUP_ALL, 6},
	{1000, 4, PMGR_BINNING_GPU_SRAM, PMGR_BINNING_GROUP_ALL, 6},

	{ 800, 1, PMGR_BINNING_GPU_SRAM, PMGR_BINNING_GROUP_ALL, 0},
	{ 800, 2, PMGR_BINNING_GPU_SRAM, PMGR_BINNING_GROUP_ALL, 0},
	{ 920, 3, PMGR_BINNING_GPU_SRAM, PMGR_BINNING_GROUP_ALL, 0},
	{1030, 4, PMGR_BINNING_GPU_SRAM, PMGR_BINNING_GROUP_ALL, 0},
	{1100, 5, PMGR_BINNING_GPU_SRAM, PMGR_BINNING_GROUP_ALL, 0},
};

const struct pmgr_binning_board_id_to_offsets pmgr_binning_board_id_to_offsets_data[] = {
//	type
//	|					voltage offset array
//	|					|						voltage offset array size
//	|					|						|																chip rev
//	|					|						|																|					board id
//	|					|						|																|					|					binning group
//	|					|						|																|					|					|
};

// .voltages = {volAdj0, volAdj1, volAdj2, volAdj3, dvfmMaxAdj, dvmrAdj0, dvmrAdj1, dvmrAdj2}

const struct pmgr_binning_voltadj_entry pmgr_binning_voltadj_entry_data[] = {
	//
	//                            CPU
	//
	//                                                          volAdj0
	//                                                          |     volAdj1
	//                                                          |     |      volAdj2
	//                                                          |     |      |      volAdj3
	//                                                          |     |      |      |      dvfmMaxAdj
	//                                                          |     |      |      |      |      dvmrAdj0
	//                                                          |     |      |      |      |      |      dvmrAdj1
	//                                                          |     |      |      |      |      |      |      dvmrAdj2
	//                                                          |     |      |      |      |      |      |      |
	{CHIPID_CPU_VOLTAGE_396 , CHIP_REVISION_B0, 0, .voltages = {0, 9375, 18750, 18750, 40625,     0,     0,     0}},
	{CHIPID_CPU_VOLTAGE_720 , CHIP_REVISION_B0, 0, .voltages = {0,    0, 18750, 21875, 40625,     0,     0,     0}},
	{CHIPID_CPU_VOLTAGE_1080, CHIP_REVISION_B0, 0, .voltages = {0,    0, 15625, 15625, 40625,     0,     0,     0}},
	{CHIPID_CPU_VOLTAGE_1440, CHIP_REVISION_B0, 0, .voltages = {0,    0, 12500, 12500, 40625,     0,     0,     0}},
	{CHIPID_CPU_VOLTAGE_1800, CHIP_REVISION_B0, 0, .voltages = {0,    0,     0,     0, 40625,     0,     0,     0}},
	{CHIPID_CPU_VOLTAGE_2160, CHIP_REVISION_B0, 0, .voltages = {21875,0,     0,     0, 40625,     0,     0,     0}},
	{CHIPID_CPU_VOLTAGE_2256_1core,CHIP_REVISION_B0,0,.voltages={21875,0,    0,     0, 40625,     0,     0,     0}},
	{CHIPID_CPU_VOLTAGE_2256, CHIP_REVISION_B0, 0, .voltages = {0,    0,     0,     0, 40625,     0,     0,     0}},
	{CHIPID_CPU_VOLTAGE_2352, CHIP_REVISION_B0, 0, .voltages = {0,    0,     0,     0, 40625,     0,     0,     0}},
	{CHIPID_CPU_VOLTAGE_2448, CHIP_REVISION_B0, 0, .voltages = {0,    0,     0,     0, 40625,     0,     0,     0}},


	{CHIPID_CPU_VOLTAGE_BYPASS,CHIP_REVISION_A0,0, .voltages = {0,    0,     0,     0, 40625,     0,     0,     0}},
	{CHIPID_SOC_VOLTAGE_SECUREROM,CHIP_REVISION_A0,0,.voltages={0,    0,     0,     0,     0,     0,     0,     0}},
	{CHIPID_CPU_VOLTAGE_396 , CHIP_REVISION_A0, 0, .voltages = {0, 6250, 12500, 18750, 40625,     0,     0,     0}},
	{CHIPID_CPU_VOLTAGE_720 , CHIP_REVISION_A0, 0, .voltages = {0, 6250, 12500, 18750, 40625,     0,     0,     0}},
	{CHIPID_CPU_VOLTAGE_1080, CHIP_REVISION_A0, 0, .voltages = {0, 3125,  6250, 12500, 40625,  6250,  6250,    0}},
	{CHIPID_CPU_VOLTAGE_1440, CHIP_REVISION_A0, 0, .voltages = {0, 3125,  6250, 12500, 40625,  9375,  9375,    0}},
	{CHIPID_CPU_VOLTAGE_1800, CHIP_REVISION_A0, 0, .voltages = {0,    0,     0,     0, 40625, 18750, 15625, 3125}},
	{CHIPID_CPU_VOLTAGE_2160, CHIP_REVISION_A0, 0, .voltages = {0,    0,     0,     0, 40625, 25000, 21875, 3125}},
	{CHIPID_CPU_VOLTAGE_2256_1core,CHIP_REVISION_A0,0,.voltages={0,   0,     0,     0, 40625,  3125,     0,    0}},
	{CHIPID_CPU_VOLTAGE_2256, CHIP_REVISION_A0, 0, .voltages = {0,    0,     0,     0, 40625, 25000, 21875, 3125}},
	{CHIPID_CPU_VOLTAGE_2352, CHIP_REVISION_A0, 0, .voltages = {0,    0,     0,     0, 40625, 28125, 25000, 3125}},
	{CHIPID_CPU_VOLTAGE_2448, CHIP_REVISION_A0, 0, .voltages = {0,    0,     0,     0, 40625,  3125,     0,    0}},

	//                           GFX
	//
	//                                                         volAdj0
	//                                                         |       volAdj1
	//                                                         |       |      volAdj2
	//                                                         |       |      |      volAdj3
	//                                                         |       |      |      | N/A
	//                                                         |       |      |      | | N/A
	//                                                         |       |      |      | | |  N/A
	//                                                         |       |      |      | | | | N/A
	//                                                         |       |      |      | | | | |
	{CHIPID_GPU_VOLTAGE_360,  CHIP_REVISION_B0, 0, .voltages = {0,  3125, 18750, 21875,0,0,0,0}},
	{CHIPID_GPU_VOLTAGE_520,  CHIP_REVISION_B0, 0, .voltages = {0,     0, 12500, 12500,0,0,0,0}},
	{CHIPID_GPU_VOLTAGE_650,  CHIP_REVISION_B0, 0, .voltages = {0,     0,  6125,  6125,0,0,0,0}},
	{CHIPID_GPU_VOLTAGE_723,  CHIP_REVISION_B0, 0, .voltages = {0,     0,     0,     0,0,0,0,0}},
	{CHIPID_GPU_VOLTAGE_804,  CHIP_REVISION_B0, 0, .voltages = {0,     0,     0,     0,0,0,0,0}},

	{CHIPID_GPU_VOLTAGE_360,  CHIP_REVISION_A0, 0, .voltages = {0, 12500, 25000, 25000,0,0,0,0}},
	{CHIPID_GPU_VOLTAGE_520,  CHIP_REVISION_A0, 0, .voltages = {0,  6250, 12500, 12500,0,0,0,0}},
	{CHIPID_GPU_VOLTAGE_650,  CHIP_REVISION_A0, 0, .voltages = {0,     0,     0,     0,0,0,0,0}},
	{CHIPID_GPU_VOLTAGE_723,  CHIP_REVISION_A0, 0, .voltages = {0,     0,     0,     0,0,0,0,0}},
	{CHIPID_GPU_VOLTAGE_804,  CHIP_REVISION_A0, 0, .voltages = {0,     0,     0,     0,0,0,0,0}},
};

const uint32_t pmgr_binning_voltage_config_sram_data_size =
sizeof(pmgr_binning_voltage_config_sram_data)/sizeof(pmgr_binning_voltage_config_sram_data[0]);
const uint32_t pmgr_binning_mode_to_const_data_size = sizeof(pmgr_binning_mode_to_const_data)/sizeof(pmgr_binning_mode_to_const_data[0]);
const uint32_t pmgr_binning_voltadj_entry_data_size = sizeof(pmgr_binning_voltadj_entry_data) / sizeof(pmgr_binning_voltadj_entry_data[0]);
const uint32_t pmgr_binning_voltage_index_to_config_data_size = sizeof(pmgr_binning_voltage_index_to_config_data)/sizeof(pmgr_binning_voltage_index_to_config_data[0]);
const uint32_t pmgr_binning_board_id_to_offsets_data_size = sizeof(pmgr_binning_board_id_to_offsets_data)/sizeof(pmgr_binning_board_id_to_offsets_data[0]);
