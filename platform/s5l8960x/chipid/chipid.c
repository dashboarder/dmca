/*
 * Copyright (C) 2011-2013 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */
#include <platform.h>
#include <platform/soc/chipid.h>
#include <platform/soc/hwclocks.h>
#include <platform/soc/pmgr.h>

#if SUB_PLATFORM_S5L8960X
#define MINIMUM_BINNING_VERSION		1

#if SUB_TARGET_N51 || SUB_TARGET_N53 || SUB_TARGET_J85 || SUB_TARGET_J86 || SUB_TARGET_J87 || SUB_TARGET_J85M || SUB_TARGET_J86M || SUB_TARGET_J87M

#define DEFAULT_CPU_VOLTAGE_BYPASS		1137
#define DEFAULT_CPU_VOLTAGE_SECUREROM		1137
#define DEFAULT_CPU_VOLTAGE_V0			800
#define DEFAULT_CPU_VOLTAGE_V1			862
#define DEFAULT_CPU_VOLTAGE_V2			987
#define DEFAULT_CPU_VOLTAGE_V3			1137
#define DEFAULT_CPU_VOLTAGE_V4			1200

#define BASE_CPU_VOLTAGE_BYPASS			1137
#define BASE_CPU_VOLTAGE_SECUREROM		1137
#define BASE_CPU_VOLTAGE_V0			800
#define BASE_CPU_VOLTAGE_V1			862
#define BASE_CPU_VOLTAGE_V2			987
#define BASE_CPU_VOLTAGE_V3			1137
#define BASE_CPU_VOLTAGE_V4			1200

#define DEFAULT_SOC_VOLTAGE			1000
#define DEFAULT_SRAM_VOLTAGE			1000

#if SUB_TARGET_N51 || SUB_TARGET_N53
// For fuse rev < 3 we don't use higher operating points.
#define SAFE_CPU_VOLTAGE_BYPASS			875
#define SAFE_CPU_VOLTAGE_SECUREROM		875
#define SAFE_CPU_VOLTAGE_V0			825
#define SAFE_CPU_VOLTAGE_V1			865
#define SAFE_CPU_VOLTAGE_V2			865
#define SAFE_CPU_VOLTAGE_V3			875
#define SAFE_CPU_VOLTAGE_V4			950
#endif

#elif SUB_TARGET_J34 || SUB_TARGET_J34M || SUB_TARGET_J71 || SUB_TARGET_J72 || SUB_TARGET_J73

#define DEFAULT_CPU_VOLTAGE_BYPASS		1137
#define DEFAULT_CPU_VOLTAGE_SECUREROM		1137
#define DEFAULT_CPU_VOLTAGE_V0			862
#define DEFAULT_CPU_VOLTAGE_V1			987
#define DEFAULT_CPU_VOLTAGE_V2			1137
#define DEFAULT_CPU_VOLTAGE_V3			1200
#define DEFAULT_CPU_VOLTAGE_V4			1237

#define BASE_CPU_VOLTAGE_BYPASS			1137
#define BASE_CPU_VOLTAGE_SECUREROM		1137
#define BASE_CPU_VOLTAGE_V0			862
#define BASE_CPU_VOLTAGE_V1			987
#define BASE_CPU_VOLTAGE_V2			1137
#define BASE_CPU_VOLTAGE_V3			1200
#define BASE_CPU_VOLTAGE_V4			1237

#define DEFAULT_SOC_VOLTAGE			1000
#define DEFAULT_SRAM_VOLTAGE			1050

#else

#define DEFAULT_CPU_VOLTAGE_BYPASS		875
#define DEFAULT_CPU_VOLTAGE_SECUREROM		875
#define DEFAULT_CPU_VOLTAGE_V0			825
#define DEFAULT_CPU_VOLTAGE_V1			865
#define DEFAULT_CPU_VOLTAGE_V2			875
#define DEFAULT_CPU_VOLTAGE_V3			950

#define BASE_CPU_VOLTAGE_BYPASS			875
#define BASE_CPU_VOLTAGE_SECUREROM		875
#define BASE_CPU_VOLTAGE_V0			825
#define BASE_CPU_VOLTAGE_V1			865
#define BASE_CPU_VOLTAGE_V2			875
#define BASE_CPU_VOLTAGE_V3			950

#define DEFAULT_SOC_VOLTAGE			1000
#define DEFAULT_SRAM_VOLTAGE			1000

#endif


// GPU voltages in mV.
static uint32_t default_gpu_voltages[kPMGR_GFX_STATE_MAX] = { 0, 950, 1000, 1100, 950, 1000, 1100, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
#endif

// CCC_PWRCTL_EFUSE_DVFM0.Revision should be the only
// binning rev info for all the binned rails. (rev id starting from 0)
// If CCC_PWRCTL_EFUSE_DVFM0 is 0, we can safely assume that
// this is an unbinned part.
static uint32_t chipid_get_binning_revision(void)
{
	if (rCCC_EFUSE_DVFM(0) == 0) {
		return 0;
	} else {
		// Add 1 to distinguish against an unbinned part.
		return ((rCCC_EFUSE_DVFM(0) >> 29) & 0x7) + 1;
	}
}

// (Base Fuse + 1) * 25
static uint32_t chipid_get_basevoltage(void)
{
	return (((rCCC_EFUSE_DVFM(0) >> 24) & 0x1F) + 1) * 25;
}

// Here are the mappings for operating points to modes.
// Mode 9: 1392 MHz. (V4 for Discrete and only in Discrete).
// Mode 7: 1296 MHz. (V3 for Discrete, V4 for POP).
// Mode 4: 1128 MHz. (V2 for Discrete, V3 for POP).
// Mode 2:  840 MHz. (V1 for Discrete, V2 for POP).
// Mode 1:  600 MHz. (V0 for Discrete, V1 for POP).
// Mode 5:  396 MHz. (V0 for POP and only in POP).                                                                                                                                                                                                                                                        

static uint32_t get_ccc_mode_for_index(uint32_t volt_index)
{
	static uint32_t v2m[] = {
#if SUB_TARGET_N51 || SUB_TARGET_N53 || SUB_TARGET_J85 || SUB_TARGET_J86 || SUB_TARGET_J87 || SUB_TARGET_J85M || SUB_TARGET_J86M || SUB_TARGET_J87M
		[CHIPID_CPU_VOLTAGE_BYPASS] = 5,
		[CHIPID_CPU_VOLTAGE_V0] = 5,
		[CHIPID_CPU_VOLTAGE_V1] = 1,
		[CHIPID_CPU_VOLTAGE_V2] = 2,
		[CHIPID_CPU_VOLTAGE_V3] = 4,
		[CHIPID_CPU_VOLTAGE_V4] = 7,
#elif SUB_TARGET_J34 || SUB_TARGET_J34M || SUB_TARGET_J71 || SUB_TARGET_J72 || SUB_TARGET_J73
		[CHIPID_CPU_VOLTAGE_BYPASS] = 1,
		[CHIPID_CPU_VOLTAGE_V0] = 1,
		[CHIPID_CPU_VOLTAGE_V1] = 2,
		[CHIPID_CPU_VOLTAGE_V2] = 4,
		[CHIPID_CPU_VOLTAGE_V3] = 7,
		[CHIPID_CPU_VOLTAGE_V4] = 9,
#else	// By default we will fall back to Jx configuration.
		[CHIPID_CPU_VOLTAGE_BYPASS] = 1,
		[CHIPID_CPU_VOLTAGE_V0] = 1,
		[CHIPID_CPU_VOLTAGE_V1] = 2,
		[CHIPID_CPU_VOLTAGE_V2] = 4,
		[CHIPID_CPU_VOLTAGE_V3] = 7,
		[CHIPID_CPU_VOLTAGE_V4] = 9,
#endif
	};

	return v2m[volt_index];
}

static uint32_t get_ccc_binfuse_for_mode(uint32_t mode)
{
	uint32_t binfuse = 0;

	switch(mode) {
		case 9:
			binfuse = (rCCC_EFUSE_DVFM(1) >> 25);
			break;
		case 7:
			binfuse = (rCCC_EFUSE_DVFM(1) >> 18);
			break;
		case 4:
			binfuse = (rCCC_EFUSE_DVFM(1) >> 11);
			break;
		case 2:
			binfuse = ((rCCC_EFUSE_DVFM(0) >> 21) & 0x7) |
				(((rCCC_EFUSE_DVFM(1) >> 0) & 0xF) << 3);
			break;
		case 1:
			binfuse = (rCCC_EFUSE_DVFM(0) >> 14);
			break;
		case 5:
			binfuse = (rCCC_EFUSE_DVFM(0) >> 0);
			break;
	}

	return (binfuse & 0x7F);
}

static uint32_t get_ccc_bin_voltage(uint32_t volt_index)
{
	uint32_t mode = 0, binfuse = 0;
	// We will never transition to SECUREROM state
	// during normal operation. 
	// So, for completeness return the default
	// voltage value for SecureROM.
	if (volt_index == DEFAULT_CPU_VOLTAGE_SECUREROM)
		return DEFAULT_CPU_VOLTAGE_SECUREROM;

	uint32_t base_cpu_voltage = chipid_get_basevoltage();

	mode = get_ccc_mode_for_index(volt_index);
	binfuse = get_ccc_binfuse_for_mode(mode);

	return (base_cpu_voltage + binfuse * 5);

}

static uint32_t get_soc_binfuse(void)
{
	uint32_t binfuse = 0;
	binfuse = (rCFG_FUSE1 >> 21);

	return (binfuse & 0x7F);
}

static uint32_t get_soc_bin_voltage(void)
{
	uint32_t binfuse = 0;
	uint32_t base_soc_voltage = chipid_get_basevoltage();

	binfuse = get_soc_binfuse();

	return (base_soc_voltage + binfuse * 5);
}

static uint32_t get_gpu_binfuse(uint32_t volt_index)
{
	uint32_t binfuse = 0, index = 0;

	// voltages at indices 1,2,3 are repeated for 4,5,6
	index = (volt_index - 1) % 3;

	binfuse = rCFG_FUSE1 >> (index * 7);

	return (binfuse & 0x7F);
}

static uint32_t get_gpu_bin_voltage(uint32_t volt_index)
{
	uint32_t binfuse = 0;

	uint32_t base_gpu_voltage;

	// For the OFF mode and unused entries we should
	// return 0.
	if (default_gpu_voltages[volt_index] == 0)
		return 0;

	base_gpu_voltage = chipid_get_basevoltage();
	binfuse = get_gpu_binfuse(volt_index);

	return (base_gpu_voltage + binfuse * 5);
}

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
	return (rCFG_FUSE0 >> 4) & 3;
}

uint32_t chipid_get_minimum_epoch(void)
{
	return (rCFG_FUSE0 >> 9) & 0x7F;
}

uint32_t chipid_get_chip_id(void)
{
#if SUB_PLATFORM_S5L8960X
        return 0x8960;
#endif
}

uint32_t chipid_get_chip_revision(void)
{
	return (((rCFG_FUSE0 >> 25) & 0x7) << 4) | (((rCFG_FUSE0 >> 22) & 0x7) << 0);
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
	uint32_t soc_voltage = 0;

	if (chipid_get_binning_revision() < MINIMUM_BINNING_VERSION) {
		return DEFAULT_SOC_VOLTAGE;
	} else {
		soc_voltage = get_soc_bin_voltage();
	}

	return soc_voltage;
}

uint32_t chipid_get_cpu_voltage(uint32_t index)
{
	uint32_t cpu_voltage = 0;

	if (chipid_get_binning_revision() < MINIMUM_BINNING_VERSION) {
		switch(index) {
			case CHIPID_CPU_VOLTAGE_BYPASS:
				cpu_voltage = DEFAULT_CPU_VOLTAGE_BYPASS;
				break;
			case CHIPID_CPU_VOLTAGE_SECUREROM:
				cpu_voltage = DEFAULT_CPU_VOLTAGE_SECUREROM;
				break;
			case CHIPID_CPU_VOLTAGE_V0:
				cpu_voltage = DEFAULT_CPU_VOLTAGE_V0;
				break;
			case CHIPID_CPU_VOLTAGE_V1:
				cpu_voltage = DEFAULT_CPU_VOLTAGE_V1;
				break;
			case CHIPID_CPU_VOLTAGE_V2:
				cpu_voltage = DEFAULT_CPU_VOLTAGE_V2;
				break;
			case CHIPID_CPU_VOLTAGE_V3:
				cpu_voltage = DEFAULT_CPU_VOLTAGE_V3;
				break;
#if SUB_TARGET_N51 || SUB_TARGET_N53 || SUB_TARGET_J34 || SUB_TARGET_J34M || SUB_TARGET_J71 || SUB_TARGET_J72 || SUB_TARGET_J73 || SUB_TARGET_J85 || SUB_TARGET_J86 || SUB_TARGET_J87 || SUB_TARGET_J85M || SUB_TARGET_J86M || SUB_TARGET_J87M
			case CHIPID_CPU_VOLTAGE_V4:
				cpu_voltage = DEFAULT_CPU_VOLTAGE_V4;
				break;
#endif
		}
	} else {
		cpu_voltage = get_ccc_bin_voltage(index);
	}

	return cpu_voltage;
}


uint32_t chipid_get_ram_voltage(uint32_t index)
{
	return DEFAULT_SRAM_VOLTAGE;
}

uint32_t chipid_get_gpu_voltage(uint32_t index)
{
	uint32_t gpu_voltage = 0;

	if (index >= kPMGR_GFX_STATE_MAX)	return 0;

	if (chipid_get_binning_revision() < MINIMUM_BINNING_VERSION) {
		// Unbinned part. Use default voltage.
		gpu_voltage = default_gpu_voltages[index];
	} else {
		gpu_voltage = get_gpu_bin_voltage(index);
	}

	return gpu_voltage;
}

bool chipid_get_fuse_lock(void)
{
	return (rCFG_FUSE1 & (1 << 31)) != 0;
}

void chipid_set_fuse_lock(bool locked)
{
	if (locked) rCFG_FUSE1 |= (1 << 31);
}

bool chipid_valid_thermal_sensor_cal_data_expected(void)
{
#if SUPPORT_FPGA
	return false;
#else
	return true;
#endif
}

uint32_t chipid_get_fused_pmgr_thermal_sensor_cal(uint32_t sensorID, uint32_t bit_offset)
{
	uint32_t temp_cal = 0;
	switch (sensorID) {
		case 0:		temp_cal = (rCFG_FUSE2 >> bit_offset) & 0x1FF;	break;
		case 1:		temp_cal = (rCFG_FUSE3 >> bit_offset) & 0x1FF;	break;
		default:	break;
	}
	return temp_cal;
}

uint32_t chipid_get_fused_ccc_thermal_sensor_cal(uint32_t sensorID, uint32_t bit_offset)
{
	uint32_t temp_cal = 0;
	switch (sensorID) {
		case 0:		temp_cal = (rCCC_THEM_EFUSE_TADC0 >> bit_offset) & 0x1FF;	break;
		case 1:		temp_cal = (rCCC_THEM_EFUSE_TADC1 >> bit_offset) & 0x1FF;	break;
		default:	break;
	}
	return temp_cal;
}

uint32_t chipid_get_fused_pmgr_thermal_sensor_cal_70C(uint32_t sensorID)
{
	return chipid_get_fused_pmgr_thermal_sensor_cal(sensorID, 9);
}

uint32_t chipid_get_fused_pmgr_thermal_sensor_cal_25C(uint32_t sensorID)
{
	return chipid_get_fused_pmgr_thermal_sensor_cal(sensorID, 0);
}

uint32_t chipid_get_fused_ccc_thermal_sensor_cal_70C(uint32_t sensorID)
{
	return chipid_get_fused_ccc_thermal_sensor_cal(sensorID, 9);
}

uint32_t chipid_get_fused_ccc_thermal_sensor_cal_25C(uint32_t sensorID)
{
	return chipid_get_fused_ccc_thermal_sensor_cal(sensorID, 0);
}

uint32_t chipid_get_fuse_revision(void)
{
        return (rCFG_FUSE0 >> 18) & 0xf;
}

uint32_t chipid_get_total_rails_leakage()
{
	// 
	// Refer to following for details: 
	//	- <rdar://problem/12387731> N51 MLB leakage sort
	//	- Alcatraz Test Plan document
	//
	uint32_t total_leakage;
	uint32_t leakage_data0; 
	uint8_t leakage_data1;

	leakage_data0 = rCFG_FUSE5;
	leakage_data1 = (rCFG_FUSE4 >> 31) & 1;

	total_leakage = ((leakage_data0 >> 28) & 0xf) + 1;				// soc_sram: cfg_fuse4[31:28]
	total_leakage += ((leakage_data0 >> 24) & 0xf) + 1;				// cpu_sram: cfg_fuse4[27:24]
	total_leakage += ((leakage_data0 >> 16) & 0xff) + 1;				// gpu: cfg_fuse4[23:16]
	total_leakage += ((leakage_data0 >> 8) & 0xff) + 1;				// soc: cfg_fuse4[15:8]
	total_leakage += ((((leakage_data0 >> 0) & 0xff) << 1) | leakage_data1) + 1;	// cpu: cfg_fuse4[7:0], cfg_fuse4[31]

	return total_leakage;
}
