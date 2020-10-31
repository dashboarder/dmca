/*
 * Copyright (C) 2012-2014 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */

#include <debug.h>
#include <platform.h>
#include <platform/soc/chipid.h>
#include <platform/soc/hwclocks.h>

#define CHIPID_VOLTAGE_FIXED 0
#define CHIPID_MODE_NONE 0

struct chipid_voltage_config {
	uint32_t	safe_voltage;
	uint32_t	mode;
};

#if SUB_PLATFORM_T7000 || SUB_PLATFORM_T7001

#if SUB_PLATFORM_T7000
static struct chipid_voltage_config chipid_cpu_voltages[] = {
	[CHIPID_CPU_VOLTAGE_BYPASS]		= {1000, CHIPID_MODE_NONE},
	[CHIPID_CPU_VOLTAGE_SECUREROM]		= {1000, CHIPID_MODE_NONE},
	[CHIPID_CPU_VOLTAGE_V0]			= { 775, 1},
	[CHIPID_CPU_VOLTAGE_V1]			= { 800, 3},
	[CHIPID_CPU_VOLTAGE_V2]			= { 880, 4},
	[CHIPID_CPU_VOLTAGE_V3]			= { 995, 5},
#if !SUB_TARGET_N102
	[CHIPID_CPU_VOLTAGE_V4]			= {1125, 7},
#endif
#if SUB_TARGET_J96 || SUB_TARGET_J97
	[CHIPID_CPU_VOLTAGE_V5]                 = {1195, 8},
#endif
};

static struct chipid_voltage_config chipid_soc_voltages[] = {
	[CHIPID_SOC_VOLTAGE_BYPASS]		= {950, CHIPID_MODE_NONE},
	[CHIPID_SOC_VOLTAGE_SECUREROM]		= {950, CHIPID_MODE_NONE},
	[CHIPID_SOC_VOLTAGE_IBOOT_MEM_LOW_PERF]	= {900, CHIPID_MODE_NONE},
	[CHIPID_SOC_VOLTAGE_VMIN]		= {900, 1},
	[CHIPID_SOC_VOLTAGE_VNOM]		= {950, 2},
};

static struct chipid_voltage_config chipid_gpu_voltages[] = {
	[CHIPID_GPU_VOLTAGE_OFF]		= {   0, CHIPID_MODE_NONE},
	[CHIPID_GPU_VOLTAGE_V0]			= { 900, 1},
	[CHIPID_GPU_VOLTAGE_V0_DIDT]		= { 900, 1},
	[CHIPID_GPU_VOLTAGE_V1]			= { 950, 2},
	[CHIPID_GPU_VOLTAGE_V1_DIDT]		= { 950, 2},
	[CHIPID_GPU_VOLTAGE_V2]			= {1050, 3},
	[CHIPID_GPU_VOLTAGE_V2_DIDT]		= {1050, 3},
#if SUB_TARGET_N56 || SUB_TARGET_J42 || SUB_TARGET_J42D || SUB_TARGET_J96 || SUB_TARGET_J97
	[CHIPID_GPU_VOLTAGE_V3]			= {1050, 4},
	[CHIPID_GPU_VOLTAGE_V3_DIDT]		= {1050, 4},
#endif
#if SUB_TARGET_J96 || SUB_TARGET_J97
	[CHIPID_GPU_VOLTAGE_V4]			= {1010, 5},
	[CHIPID_GPU_VOLTAGE_V4_DIDT]		= {1010, 5},
	[CHIPID_GPU_VOLTAGE_V5]			= {}, //CHIPID_GPU_VOLTAGE_V5 not used on J96/J97
	[CHIPID_GPU_VOLTAGE_V5_DIDT]		= {}, 
#endif
};
	
static struct chipid_voltage_config chipid_sram_voltages[] = {
#if SUB_TARGET_N56 || SUB_TARGET_J42 || SUB_TARGET_J42D
	[CHIPID_VOLTAGE_FIXED]	= {970, 1},
#else
	[CHIPID_VOLTAGE_FIXED]	= {950, 1},
#endif
};

#elif SUB_PLATFORM_T7001
static struct chipid_voltage_config chipid_cpu_voltages[] = {
	[CHIPID_CPU_VOLTAGE_BYPASS]		= {1000, CHIPID_MODE_NONE},
	[CHIPID_CPU_VOLTAGE_SECUREROM]		= {1000, CHIPID_MODE_NONE},
	[CHIPID_CPU_VOLTAGE_V0]			= { 775, 1},
	[CHIPID_CPU_VOLTAGE_V1]			= { 800, 3},
	[CHIPID_CPU_VOLTAGE_V2]			= { 880, 4},
	[CHIPID_CPU_VOLTAGE_V3]			= { 980, 5},
	[CHIPID_CPU_VOLTAGE_V4]			= {1125, 7},
	[CHIPID_CPU_VOLTAGE_V5]			= {1250, 8},
	[CHIPID_CPU_VOLTAGE_V6]			= {1200, 9},
	[CHIPID_CPU_VOLTAGE_V6_UNBINNED]	= {1200, CHIPID_MODE_NONE},
};

static struct chipid_voltage_config chipid_sram_voltages[] = {
	[CHIPID_CPU_VOLTAGE_BYPASS]		= { 950, CHIPID_MODE_NONE},
	[CHIPID_CPU_VOLTAGE_SECUREROM]		= { 950, CHIPID_MODE_NONE},
	[CHIPID_CPU_VOLTAGE_V0]			= { 950, 1},
	[CHIPID_CPU_VOLTAGE_V1]			= { 950, 1},
	[CHIPID_CPU_VOLTAGE_V2]			= { 950, 1},
	[CHIPID_CPU_VOLTAGE_V3]			= { 950, 1},
	[CHIPID_CPU_VOLTAGE_V4]			= {1000, 1},
	[CHIPID_CPU_VOLTAGE_V5]			= {1000, 2},
	[CHIPID_CPU_VOLTAGE_V6]			= {1000, 2},
	[CHIPID_CPU_VOLTAGE_V6_UNBINNED]	= {1000, CHIPID_MODE_NONE},
};

// CHIPID_GPU_VOLTAGE_V4 / 600MHz is only used on T7000
static struct chipid_voltage_config chipid_gpu_voltages[] = {
	[CHIPID_GPU_VOLTAGE_OFF]		= {   0, CHIPID_MODE_NONE},
	[CHIPID_GPU_VOLTAGE_V0]			= { 900, 1},
	[CHIPID_GPU_VOLTAGE_V0_DIDT]		= { 900, 1},
	[CHIPID_GPU_VOLTAGE_V1]			= { 950, 2},
	[CHIPID_GPU_VOLTAGE_V1_DIDT]		= { 950, 2},
	[CHIPID_GPU_VOLTAGE_V2]			= {1050, 3},
	[CHIPID_GPU_VOLTAGE_V2_DIDT]		= {1050, 3},
	[CHIPID_GPU_VOLTAGE_V3]			= {1050, 4},
	[CHIPID_GPU_VOLTAGE_V3_DIDT]		= {1050, 4},
	[CHIPID_GPU_VOLTAGE_V5]			= {1100, 5},
	[CHIPID_GPU_VOLTAGE_V5_DIDT]		= {1100, 5},
};

static struct chipid_voltage_config chipid_gpu_sram_voltages[] = {
	[CHIPID_GPU_VOLTAGE_OFF]		= { 950,  1},
	[CHIPID_GPU_VOLTAGE_V0]			= { 950, 1},
	[CHIPID_GPU_VOLTAGE_V0_DIDT]		= { 950, 1},
	[CHIPID_GPU_VOLTAGE_V1]			= { 950, 1},
	[CHIPID_GPU_VOLTAGE_V1_DIDT]		= { 950, 1},
	[CHIPID_GPU_VOLTAGE_V2]			= {1050, 1},
	[CHIPID_GPU_VOLTAGE_V2_DIDT]		= {1050, 1},
	[CHIPID_GPU_VOLTAGE_V3]			= {1050, 2},
	[CHIPID_GPU_VOLTAGE_V3_DIDT]		= {1050, 2},
	[CHIPID_GPU_VOLTAGE_V5]			= {1050, 2},
	[CHIPID_GPU_VOLTAGE_V5_DIDT]		= {1050, 2},
};

static struct chipid_voltage_config chipid_soc_voltages[] = {
	[CHIPID_VOLTAGE_FIXED]	= {950, CHIPID_MODE_NONE},
};
#endif

#endif

#define CHIP_REVISION_ALL	0xff
#define CHIP_ID_ALL		0x00
struct vol_adj_struct {
	uint32_t	index;
	uint32_t	chipid;
	uint32_t	chiprev;
	uint32_t	minimum_fuserev;
	struct chipid_vol_adj vol_adj;
};

static struct vol_adj_struct vol_adj_cpu[] = {
// T7000 b0 and T7001 a0
	{CHIPID_CPU_VOLTAGE_V0,	0x7000,	CHIP_REVISION_B0,	0,	{{		0,	12500,	25000,	31250}}},
	{CHIPID_CPU_VOLTAGE_V0,	0x7001,	CHIP_REVISION_A0,	0,	{{		0,	12500,	25000,	31250}}},
	{CHIPID_CPU_VOLTAGE_V0, 0x7001, CHIP_REVISION_A1,	0,	{{		0,	12500,	25000,	31250}}},
	{CHIPID_CPU_VOLTAGE_V1,	0x7000,	CHIP_REVISION_B0,	0,	{{		0,	12500,	25000,	25000}}},
	{CHIPID_CPU_VOLTAGE_V1,	0x7001,	CHIP_REVISION_A0,	0,	{{		0,	12500,	25000,	25000}}},
	{CHIPID_CPU_VOLTAGE_V1, 0x7001, CHIP_REVISION_A1,	0,	{{		0,	9380,	25000,	31250}}},
	{CHIPID_CPU_VOLTAGE_V2,	0x7000,	CHIP_REVISION_B0,	0,	{{		0,	12500,	25000,	25000}}},
	{CHIPID_CPU_VOLTAGE_V2,	0x7001,	CHIP_REVISION_A0,	0,	{{		0,	12500,	25000,	25000}}},
	{CHIPID_CPU_VOLTAGE_V3,	0x7000, CHIP_REVISION_B0,	0,	{{		0,	12500,	25000,	25000}}},
	{CHIPID_CPU_VOLTAGE_V3,	0x7001,	CHIP_REVISION_A0,	0,	{{		0,	12500,	25000,	25000}}},
	{CHIPID_CPU_VOLTAGE_V4,	0x7000,	CHIP_REVISION_B0,	0,	{{	12500,	12500,	12500,		0}}},
	{CHIPID_CPU_VOLTAGE_V4,	0x7001,	CHIP_REVISION_A0,	0,	{{	12500,	12500,	12500,		0}}},
	{CHIPID_CPU_VOLTAGE_V5,	0x7000,	CHIP_REVISION_B0,	0,	{{		0,		0,		0,		0}}},
	{CHIPID_CPU_VOLTAGE_V5,	0x7001,	CHIP_REVISION_A0,	0,	{{		0,		0,		0,		0}}},
	{CHIPID_CPU_VOLTAGE_V6,	0x7000,	CHIP_REVISION_B0,	0,	{{		0,		0,		0,		0}}},
	{CHIPID_CPU_VOLTAGE_V6,	0x7001,	CHIP_REVISION_A0,	0,	{{		0,		0,		0,		0}}},
	{CHIPID_CPU_VOLTAGE_V6_UNBINNED,	0x7000,	CHIP_REVISION_B0,	0,	{{		0,		0,		0,		0}}},
	{CHIPID_CPU_VOLTAGE_V6_UNBINNED,	0x7001,	CHIP_REVISION_A0,	0,	{{		0,		0,		0,		0}}},

// If not found in previous line, the following will be used, i.e. 7001 a1 and 7000 b1 will use it.
	{CHIPID_CPU_VOLTAGE_V0,	CHIP_ID_ALL, CHIP_REVISION_ALL,	0,	{{		0,	15630,	34380,	40630}}},
	{CHIPID_CPU_VOLTAGE_V1,	CHIP_ID_ALL, CHIP_REVISION_ALL,	0,	{{		0,	12500,	25000,	31250}}},
	{CHIPID_CPU_VOLTAGE_V2,	CHIP_ID_ALL, CHIP_REVISION_ALL,	0,	{{		0,	9380,	18750,	18750}}},
	{CHIPID_CPU_VOLTAGE_V3,	CHIP_ID_ALL, CHIP_REVISION_ALL,	0,	{{		0,	6250,	12500,	12500}}},
	{CHIPID_CPU_VOLTAGE_V4,	CHIP_ID_ALL, CHIP_REVISION_ALL,	0,	{{		0,		0,		0,		0}}},
	{CHIPID_CPU_VOLTAGE_V5,	CHIP_ID_ALL, CHIP_REVISION_ALL,	0,	{{		0,		0,		0,		0}}},
	{CHIPID_CPU_VOLTAGE_V6,	CHIP_ID_ALL, CHIP_REVISION_ALL,	0,	{{		0,		0,		0,		0}}},
	{CHIPID_CPU_VOLTAGE_V6_UNBINNED,	CHIP_ID_ALL, CHIP_REVISION_ALL,	0,	{{		0,		0,		0,		0}}},
};

static struct vol_adj_struct vol_adj_soc[] = {
	{CHIPID_SOC_VOLTAGE_IBOOT_MEM_LOW_PERF,	0x7000,	CHIP_REVISION_B0,	0,	{{		0,		0,		0,		0}}},
	{CHIPID_SOC_VOLTAGE_VMIN,				0x7000,	CHIP_REVISION_B0,	0,	{{		0,	9400,	18800,	18800}}},
	{CHIPID_SOC_VOLTAGE_VNOM,				0x7000,	CHIP_REVISION_B0,	0,	{{		0,	9400,	15600,	9400}}},

//If not found in previous line, the following will be used, i.e. 7000 b1 will use it.
	{CHIPID_SOC_VOLTAGE_IBOOT_MEM_LOW_PERF,	0x7000,	CHIP_REVISION_ALL,	0,	{{		0,		0,		0,		0}}},
	{CHIPID_SOC_VOLTAGE_VMIN,				0x7000,	CHIP_REVISION_ALL,	0,	{{		0,	12500,	28130,	28130}}},
	{CHIPID_SOC_VOLTAGE_VNOM,				0x7000,	CHIP_REVISION_ALL,	0,	{{		0,	9380,	21880,	21880}}},

};

static uint32_t chipid_get_bin_type(void)
{
	// For Capri FUSE_REV is a combination of CFG_FUSE4[5:0] and bit 287/286 for J82/J99 applications
	// Chip   fuse286 fuse287
	// 0x7000 0       0	N56/N61
	// 0X7000 1       0	J42
	// 0X7000 0       1	J96/J97/N102
	// 0X7000 0       0	J81/J82
	// 0x7001 1       0	J98/J99
	return (rCFG_FUSE8 >> 30) & 0x3;
}

// Base Voltage = (Base Fuse + 1) * 25
static uint32_t chipid_get_base_voltage(void)
{
	uint32_t voltage;

	voltage = (((rCCC_EFUSE_REV >> 24) & 0x1F) + 1) * 25;

#if SUB_PLATFORM_T7000
	// <rdar://problem/16190886> Fiji B0: Binning information for parts with fuse revision 0x9 Fiji B0:(CoreOs)
	if (chipid_get_fuse_revision() == 0x9)
		voltage -= 25;
#endif

	return voltage;
}

static uint32_t chipid_get_cpu_binfuse_for_mode(uint32_t mode)
{
	uint32_t binfuse;

	switch (mode) {
		case 9:
			binfuse = (rCCC_EFUSE_DVFM0 >> 0) & 0x7F;
			break;
		case 1:
			binfuse = (rCCC_EFUSE_DVFM0 >> 8) & 0x7F;
			break;
		case 8:
#if SUB_TARGET_J96 || SUB_TARGET_J97 || SUB_TARGET_N102
			if (chipid_get_bin_type() == 0x0) {
				return 0;
			}
#endif
			binfuse = (rCCC_EFUSE_DVFM0 >> 16) & 0x7F;
			break;
		case 3:
			binfuse = (rCCC_EFUSE_DVFM0 >> 24) & 0x7F;
			break;
		case 4:
			binfuse = (rCCC_EFUSE_DVFM1 >> 0) & 0x7F;
			break;
		case 5:
			binfuse = (rCCC_EFUSE_DVFM1 >> 8) & 0x7F;
			break;
		case 6:
			binfuse = (rCCC_EFUSE_DVFM1 >> 16) & 0x7F;
			break;
		case 7:
			binfuse = (rCCC_EFUSE_DVFM1 >> 24) & 0x7F;
			break;
		default:
			panic("Unsupported CPU mode %d\n", mode);
			break;
	}

	return binfuse;
}

#if SUB_PLATFORM_T7000
static uint32_t chipid_get_soc_binfuse_for_mode(uint32_t mode)
{
	uint32_t binfuse;

	switch (mode) {
		case 1:
			binfuse = (rCFG_FUSE7 >> 0) & 0x7F;
			break;
		case 2:
			binfuse = (rCFG_FUSE7 >> 7) & 0x7F;
			break;
		default:
			panic("Unsupported SOC mode %d\n", mode);
			break;
	}

	return binfuse;
}
#elif SUB_PLATFORM_T7001
static uint32_t chipid_get_sram_binfuse_for_mode(uint32_t mode)
{
	uint32_t binfuse;

	switch (mode) {
		case 1:
			binfuse = (rCFG_FUSE7 >> 0) & 0x7F;
			break;
		case 2:
			binfuse = (rCFG_FUSE7 >> 7) & 0x7F;
			break;
		default:
			panic("Unsupported SRAM mode %d\n", mode);
			break;
	}

	return binfuse;
}
#endif

static uint32_t chipid_get_gpu_binfuse_for_mode(uint32_t mode)
{
	uint32_t binfuse;

	switch (mode) {
		case 1:
			binfuse = (rCFG_FUSE7 >> 14) & 0x7F;
			break;
		case 2:
			binfuse = (rCFG_FUSE7 >> 21) & 0x7F;
			break;
#if SUB_PLATFORM_T7000
		case 3: // CFG[265:259]
			binfuse = (rCFG_FUSE8 >> 3) & 0x7F;
			break;
#elif SUB_PLATFORM_T7001
		case 3: // CFG[258:252]
			binfuse = (rCFG_FUSE7 >> 28) & 0xF;
			binfuse |= (rCFG_FUSE8 << 4) & 0x70;
			break;
#endif
		case 4: // CFG[272:266]
			binfuse = (rCFG_FUSE8 >> 10) & 0x7F;
			break;
#if SUB_TARGET_J96 || SUB_TARGET_J97 || SUB_TARGET_N102
		case 5: // CFG[279:273]
			if (chipid_get_bin_type() == 0x0) {
				return 0;
                        }
			binfuse = (rCFG_FUSE8 >> (273 - 256)) & 0x7f;
			break;
#else
		case 5: // CFG[223:217]
			binfuse = (rCFG_FUSE6 >> 25) & 0x7F;
			break;
#endif
		default:
			panic("Unsupported GPU mode %d\n", mode);
			break;
	}

	return binfuse;
}

#if SUB_PLATFORM_T7000
static uint32_t chipid_get_sram_binfuse_for_mode(uint32_t mode)
{
	uint32_t binfuse;

	switch (mode) {
		case 1:
			binfuse = (rCFG_FUSE7 >> 28) & 0xF;
			binfuse |= (rCFG_FUSE8 << 4) & 0x70;
			break;
		default:
			panic("Unsupported SRAM mode %d\n", mode);
			break;
	}

	return binfuse;
}
#elif SUB_PLATFORM_T7001
static uint32_t chipid_get_soc_binfuse_for_mode(uint32_t mode)
{
	uint32_t binfuse;

	switch (mode) {
		case 1:
			binfuse = (rCFG_FUSE8 >> 3) & 0x7F;
			break;
		default:
			panic("Unsupported SOC mode %d\n", mode);
			break;
	}

	return binfuse;
}
#endif

static uint32_t chipid_get_cpu_bin_voltage(uint32_t volt_index)
{
	uint32_t mode, binfuse;

	mode = chipid_cpu_voltages[volt_index].mode;

	// Return safe voltage if no binned voltage
	if (mode == CHIPID_MODE_NONE)
		return chipid_cpu_voltages[volt_index].safe_voltage;

	binfuse = chipid_get_cpu_binfuse_for_mode(mode);
	if (binfuse == 0)
		return chipid_cpu_voltages[volt_index].safe_voltage;
#if SUB_TARGET_N102
	if (volt_index == CHIPID_CPU_VOLTAGE_V0) {
		// <rdar://problem/20056603> N102: Remove CPU P7 (1392 MHz) and lower CPU P1 voltage by 15mV
		return (chipid_get_base_voltage() + binfuse * 5 - 15);
	}
#endif
	return (chipid_get_base_voltage() + binfuse * 5);
}

static uint32_t chipid_get_soc_bin_voltage(uint32_t volt_index)
{
	uint32_t mode, binfuse;

	mode = chipid_soc_voltages[volt_index].mode;

	// Return safe voltage if no binned voltage
	if (mode == CHIPID_MODE_NONE)
		return chipid_soc_voltages[volt_index].safe_voltage;

	binfuse = chipid_get_soc_binfuse_for_mode(mode);

	return (chipid_get_base_voltage() + binfuse * 5);
}

static uint32_t chipid_get_gpu_bin_voltage(uint32_t volt_index)
{
	uint32_t mode, binfuse;

	mode = chipid_gpu_voltages[volt_index].mode;

	// Return safe voltage if no binned voltage
	if (mode == CHIPID_MODE_NONE)
		return chipid_gpu_voltages[volt_index].safe_voltage;

	binfuse = chipid_get_gpu_binfuse_for_mode(mode);
	if (binfuse == 0)
		return chipid_gpu_voltages[volt_index].safe_voltage;

	return (chipid_get_base_voltage() + binfuse * 5);
}

static uint32_t chipid_get_sram_bin_voltage(uint32_t volt_index)
{
	uint32_t mode, binfuse;

	mode = chipid_sram_voltages[volt_index].mode;

	// Return safe voltage if no binned voltage
	if (mode == CHIPID_MODE_NONE)
		return chipid_sram_voltages[volt_index].safe_voltage;

	binfuse = chipid_get_sram_binfuse_for_mode(mode);

	return (chipid_get_base_voltage() + binfuse * 5);
}

#if SUB_PLATFORM_T7001
static uint32_t chipid_get_gpu_sram_bin_voltage(uint32_t volt_index)
{
	uint32_t mode, binfuse;

	mode = chipid_gpu_sram_voltages[volt_index].mode;

	// Return safe voltage if no binned voltage
	if (mode == CHIPID_MODE_NONE)
		return chipid_gpu_sram_voltages[volt_index].safe_voltage;

	binfuse = chipid_get_sram_binfuse_for_mode(mode);

	return (chipid_get_base_voltage() + binfuse * 5);
}

#endif

bool chipid_get_current_production_mode(void)
{
	return ((rCFG_FUSE0 >> 0) & 1) != 0;
}

bool chipid_get_raw_production_mode(void)
{
	return ((rCFG_FUSE0_RAW >> 0) & 1) != 0;
}

void chipid_clear_production_mode(void)
{
	rCFG_FUSE0 &= ~1;
}

bool chipid_get_secure_mode(void)
{
	// demotion only applies to the SEP, so iBoot always reads
	// the raw value for secure mode (<rdar://problem/15182573>)
	return ((rCFG_FUSE0_RAW >> 1) & 1) != 0;
}

uint32_t chipid_get_security_domain(void)
{
	return (rCFG_FUSE0 >> 2) & 3;
}

uint32_t chipid_get_board_id(void)
{
	return (rCFG_FUSE0 >> 4) & 7;
}

uint32_t chipid_get_minimum_epoch(void)
{
	return (rCFG_FUSE0 >> 9) & 0x7F;
}

uint32_t chipid_get_pid(void)
{
	return (rCFG_FUSE0 >> 16) & 0x3;
}

uint32_t chipid_get_chip_id(void)
{
#if SUB_PLATFORM_T7000
        return 0x7000;
#elif SUB_PLATFORM_T7001
        return 0x7001;
#endif
}

uint32_t chipid_get_chip_revision(void)
{
	uint32_t	chip_rev = (((rCFG_FUSE4 >> 19) & 0x7) << 4)
				 | (((rCFG_FUSE4 >> 16) & 0x7) << 0);
	return chip_rev;
}

uint32_t chipid_get_osc_frequency(void)
{
	return OSC_FREQ;
}

uint64_t chipid_get_ecid_id(void)
{
#if SUPPORT_FPGA
	return (0x000012345678ABCDULL);
#else
	return ((uint64_t)rECIDHI << 32) | rECIDLO;
#endif
}

uint64_t chipid_get_die_id(void)
{
	return ((uint64_t)rECIDHI << 32) | rECIDLO;
}

uint32_t chipid_get_soc_voltage(uint32_t index)
{
	uint32_t soc_voltage;

#if SUB_PLATFORM_T7001
	index = CHIPID_VOLTAGE_FIXED;
#endif

	if (index >= sizeof(chipid_soc_voltages)/sizeof(chipid_soc_voltages[0]))
		panic("Invalid SOC voltage index %d\n", index);

	if (chipid_get_fuse_revision() < MINIMUM_FUSE_REVISION)
		soc_voltage = chipid_soc_voltages[index].safe_voltage;
	else
		soc_voltage = chipid_get_soc_bin_voltage(index);

	return soc_voltage;
}

uint32_t chipid_get_cpu_voltage(uint32_t index)
{
	uint32_t cpu_voltage;

	if (index >= sizeof(chipid_cpu_voltages)/sizeof(chipid_cpu_voltages[0]))
		panic("Invalid CPU voltage index %d\n", index);

#if SUB_PLATFORM_T7000
	if (chipid_get_fuse_revision() < MINIMUM_FUSE_REVISION)
		cpu_voltage = chipid_cpu_voltages[index].safe_voltage;
	else
		cpu_voltage = chipid_get_cpu_bin_voltage(index);
#elif SUB_PLATFORM_T7001
	if (chipid_get_fuse_revision() >= MINIMUM_FUSE_REVISION)
		cpu_voltage = chipid_get_cpu_bin_voltage(index);
	else
		cpu_voltage = chipid_cpu_voltages[index].safe_voltage;
#endif

	return cpu_voltage;
}

uint32_t chipid_get_ram_voltage(uint32_t index)
{
	uint32_t sram_voltage;

#if SUB_PLATFORM_T7000
	index = CHIPID_VOLTAGE_FIXED;
#endif

	if (index >= sizeof(chipid_sram_voltages)/sizeof(chipid_sram_voltages[0]))
		panic("Invalid SRAM voltage index %d\n", index);

	if (chipid_get_fuse_revision() < MINIMUM_FUSE_REVISION)
		sram_voltage = chipid_sram_voltages[index].safe_voltage;
	else
		sram_voltage = chipid_get_sram_bin_voltage(index);

	return sram_voltage;
}

uint32_t chipid_get_gpu_ram_voltage(uint32_t index)
{
#if SUB_PLATFORM_T7000
	panic("GPU SRAM voltage not supported\n");
#elif SUB_PLATFORM_T7001
	uint32_t gpu_sram_voltage;

	if (index >= sizeof(chipid_gpu_sram_voltages)/sizeof(chipid_gpu_sram_voltages[0]))
		panic("Invalid GPU SRAM voltage index %d\n", index);

	if (chipid_get_fuse_revision() < MINIMUM_FUSE_REVISION)
		gpu_sram_voltage = chipid_gpu_sram_voltages[index].safe_voltage;
	else
		gpu_sram_voltage = chipid_get_gpu_sram_bin_voltage(index);

	return gpu_sram_voltage;
#endif
}

uint32_t chipid_get_gpu_voltage(uint32_t index)
{
	uint32_t gpu_voltage;

	if (index >= sizeof(chipid_gpu_voltages)/sizeof(chipid_gpu_voltages[0]))
		panic("Invalid GPU voltage index %d\n", index);

	if (chipid_get_fuse_revision() < MINIMUM_FUSE_REVISION)
		gpu_voltage = chipid_gpu_voltages[index].safe_voltage;
#if SUB_PLATFORM_T7001
	else if ((index == CHIPID_GPU_VOLTAGE_V4 || index == CHIPID_GPU_VOLTAGE_V4_DIDT) && 
			chipid_get_fuse_revision() < MINIMUM_BINNED_GPU_P6_REVISION)
		gpu_voltage = chipid_get_gpu_bin_voltage(index - 1) + P6_VOL_OFFSET_FROM_P4_BINNING;
#endif
	else
		gpu_voltage = chipid_get_gpu_bin_voltage(index);

	return gpu_voltage;
}

bool chipid_get_fuse_lock(void)
{
	return (rCFG_FUSE1 & (1 << 31)) != 0;
}

void chipid_set_fuse_lock(bool locked)
{
	if (locked) {
		rCFG_FUSE1 |= (1 << 31);
		asm("dsb sy");
		if ((rCFG_FUSE1 & (1 << 31)) == 0) {
			panic("Failed to lock fuses\n");
		}
	}
}

uint32_t chipid_get_soc_temp_sensor_trim(uint32_t sensor_index)
{
	uint32_t trim_value = CFG_FUSE2_THERMAL_SEN(sensor_index);
	return trim_value;
}

uint32_t chipid_get_fuse_revision(void)
{
#if SUB_PLATFORM_T7000	
	// For Fiji FUSE_REV is CFG_FUSE4[5:0]
#if SUB_TARGET_J96 || SUB_TARGET_J97 || SUB_TARGET_N102
	if (chipid_get_bin_type() == 0x2) {// J96/J97/N102 binning
		//  Start at MINIMUM_FUSE_REVISION + 1 = 0x10, to avoid 0x9 fuse revision which has a specific workaround.
		return MINIMUM_FUSE_REVISION + 1 + ((rCFG_FUSE4 >> 0) & 0x3f);
	}
#endif
	return (rCFG_FUSE4 >> 0) & 0x3f;
#elif SUB_PLATFORM_T7001	
	// For Capri FUSE_REV is a combination of CFG_FUSE4[5:0] and bit 287 for J82/J99 applications
	return (((rCFG_FUSE4 >> 0) & 0x3f) | ((rCFG_FUSE8 >> 31) & 0x1) << 6);
#endif
}

uint32_t chipid_get_total_rails_leakage()
{
	// FIXME
	return 0;
}

#if SUPPORT_FPGA
uint32_t chipid_get_fpga_block_instantiation(void)
{
	// Hardware blocks instantiated.
	uint32_t	blocks = (rECID_FUSE7 >> 16) & 0xF;
	uint32_t	mask = FPGA_HAS_ALWAYS;

#if SUB_PLATFORM_T7000
	switch (blocks) {
		case 0x1:		// INT2
		case 0x2:		// old "Fiji" endcoding in pre 06_xx_yy_zz bitstream releases
			break;
		case 0x3:		// INT2 + pcie
			mask |= FPGA_HAS_PCIE;
			break;
		case 0x8:		// INT2 + VXD + AVE + JPEG + MEDIA
			mask |= FPGA_HAS_VXD | FPGA_HAS_AVE | FPGA_HAS_JPEG | FPGA_HAS_MEDIA;
			break;
		case 0x9:		// INT2 + DISP0 + DISP1 + MSR + JPEG + MEDIA
			mask |= FPGA_HAS_DISP0 | FPGA_HAS_DISP1 | FPGA_HAS_MSR | FPGA_HAS_JPEG | FPGA_HAS_MEDIA;
			break;
		case 0xA:		// INT2 + GFX
			mask |= FPGA_HAS_GFX;
			break;
		default:
			panic("Unknown hardware block instantiation: 0x%x", blocks);
	}
#elif SUB_PLATFORM_T7001 // No AVE on Capri FPGA
	switch (blocks) {
		case 0x1:		// INT2
			break;
		case 0x8:		// INT3 = INT2 + VXD + DISP0 + DISP1 + MEDIA + MSR + JPEG + PCIE
			mask |= FPGA_HAS_VXD | FPGA_HAS_DISP0 | FPGA_HAS_DISP1 | FPGA_HAS_MEDIA | FPGA_HAS_MSR | FPGA_HAS_JPEG; // <rdar://problem/15014312> enable PCIE later
			break;
		case 0xA:		// INT3 + GFX
			mask |= FPGA_HAS_VXD | FPGA_HAS_DISP0 | FPGA_HAS_DISP1 | FPGA_HAS_MEDIA | FPGA_HAS_MSR | FPGA_HAS_JPEG | FPGA_HAS_GFX; // <rdar://problem/15014312> enable PCIE later
			break;
		default:
			panic("Unknown hardware block instantiation: 0x%x", blocks);
	}
#endif
	
	return mask;
}
#endif

void chipid_get_vol_adj(enum chipid_vol_adj_type vol_adj_type, uint32_t index, struct chipid_vol_adj * vol_adj)
{
	uint32_t i;
	uint32_t chipid = chipid_get_chip_id();
	uint32_t chiprev = chipid_get_chip_revision();
	uint32_t fuserev = chipid_get_fuse_revision();
	switch (vol_adj_type) {
		case CHIPID_VOL_ADJ_CPU:
			for (i= 0; i < sizeof(vol_adj_cpu)/sizeof(vol_adj_cpu[0]); i++) {
				if (vol_adj_cpu[i].index != index)
					continue;
				if ((vol_adj_cpu[i].chipid != CHIP_ID_ALL) && (vol_adj_cpu[i].chipid != chipid))
					continue;
				if ((vol_adj_cpu[i].chiprev != CHIP_REVISION_ALL) && (vol_adj_cpu[i].chiprev != chiprev))
					continue;
				if ((vol_adj_cpu[i]).minimum_fuserev > fuserev)
					continue;
				*vol_adj = vol_adj_cpu[i].vol_adj;
				return;
			}
			panic("%s haven't found CPU index %d for chip %x rev 0x%x\n", __func__, index, chipid, chiprev);
			break;
		case CHIPID_VOL_ADJ_SOC:
			for (i= 0; i < sizeof(vol_adj_soc)/sizeof(vol_adj_soc[0]); i++) {
				if (vol_adj_soc[i].index != index)
					continue;
				if ((vol_adj_soc[i].chipid != CHIP_ID_ALL) && (vol_adj_soc[i].chipid != chipid))
					continue;
				if ((vol_adj_soc[i].chiprev != CHIP_REVISION_ALL) && (vol_adj_soc[i].chiprev != chiprev))
					continue;
				if ((vol_adj_soc[i]).minimum_fuserev > fuserev)
					continue;
				*vol_adj = vol_adj_soc[i].vol_adj;
				return;
			}
			panic("%s haven't found SOC Index %d for chip %x rev 0x%x\n", __func__, index, chipid, chiprev);
			break;
		default:
			panic("%s haven't found %d\n", __func__, index);
			break;
	}	
}
