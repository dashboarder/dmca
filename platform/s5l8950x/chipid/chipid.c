/*
 * Copyright (C) 2010-2012 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */
#include <lib/env.h>
#include <platform.h>
#include <platform/soc/chipid.h>
#include <platform/soc/hwclocks.h>

#if SUPPORT_FPGA
#define _rCFG_FUSE0				(*(volatile u_int32_t *)(CHIPID_BASE_ADDR + 0x00))
#undef rCFG_FUSE0
// XXX ECID (1 << 7)?
// XXX double-check memory values
#define rCFG_FUSE0		((0 << 31) | (3 << 28) | (0xE << 24) | (2 << 22) | \
			 (0 << 9) | (1 << 8) | (3 << 4) | (kPlatformSecurityDomainDarwin << 2) | (0 << 1) | (0 << 0) | \
			 _rCFG_FUSE0)
#endif

#if SUB_PLATFORM_S5L8950X
// H5 Tunables rev 0.59 & H5P Test Plan rev 1.13
#define MINIMUM_BINNING_VERSION	(1)
static u_int32_t default_soc_voltages[CHIPID_SOC_VOLTAGE_COUNT] = { 950, 1000, 1100 };
static u_int32_t default_cpu_voltages[CHIPID_CPU_VOLTAGE_COUNT] = { 810, 935, 1020, 1065, 1100, 1145, 0, 0 };
static u_int32_t default_ram_voltages[CHIPID_RAM_VOLTAGE_COUNT] = { 950, 1000 };
#endif

static u_int32_t chipid_get_binning_revision(void);
static u_int32_t chipid_get_base_voltage(void);

bool chipid_get_production_mode(void)
{
	return ((rCFG_FUSE0 >> 0) & 1) != 0;
}

void chipid_clear_production_mode(void)
{
#if SUPPORT_FPGA
	_rCFG_FUSE0 &= ~1;
#else
	rCFG_FUSE0 &= ~1;
#endif
}

bool chipid_get_secure_mode(void)
{
	return ((rCFG_FUSE0 >> 1) & 1) != 0;
}

u_int32_t chipid_get_security_domain(void)
{
	return (rCFG_FUSE0 >> 2) & 3;
}

u_int32_t chipid_get_board_id(void)
{
	return (rCFG_FUSE0 >> 4) & 3;
}

bool chipid_get_ecid_image_personalization_required(void)
{
	return ((rCFG_FUSE0 >> 7) & 1) != 0;
}

u_int32_t chipid_get_minimum_epoch(void)
{
	return (rCFG_FUSE0 >> 9) & 0x7F;
}

u_int32_t chipid_get_chip_id(void)
{
#if SUB_PLATFORM_S5L8950X
        return 0x8950;
#endif
}

u_int32_t chipid_get_chip_revision(void)
{
	return (((rECIDHI >> 13) & 0x7) << 4) | (((rECIDHI >> 10) & 0x7) << 0);
}

u_int32_t chipid_get_osc_frequency(void)
{
	return OSC_FREQ;
}

u_int64_t chipid_get_ecid_id(void)
{
	u_int64_t ecid = 0;

#if SUPPORT_FPGA
	ecid = 0x000012345678ABCDULL;
#else
	ecid |= ((rECIDLO >>  0)) & ((1ULL << (21 -  0)) - 1);	// LOT_ID

	ecid <<= (26 - 21);
	ecid |= ((rECIDLO >> 21)) & ((1ULL << (26 - 21)) - 1);	// WAFER_NUM

	ecid <<= (10 -  2);
	ecid |= ((rECIDHI >>  2)) & ((1ULL << (10 -  2)) - 1);	// Y_POS

	ecid <<= (32 - 26);
	ecid |= ((rECIDLO >> 26)) & ((1ULL << (32 - 26)) - 1);	// X_POS_H

	ecid <<= ( 2 -  0);
	ecid |= ((rECIDHI >>  0)) & ((1ULL << ( 2 -  0)) - 1);	// X_POS_L
#endif

	return ecid;
}

u_int64_t chipid_get_die_id(void)
{
	return ((u_int64_t)rECIDHI << 32) | rECIDLO;
}

u_int32_t chipid_get_soc_voltage(u_int32_t index)
{
	u_int32_t soc_voltage;
	u_int64_t soc_bin_data;

	if (index > CHIPID_SOC_VOLTAGE_COUNT) return 0;

	if (chipid_get_binning_revision() < MINIMUM_BINNING_VERSION) {
		// This part is unbinned to the binning version is not supported
		// Use the default voltage
		soc_voltage = default_soc_voltages[index];
	} else {
		// Read the SoC bin data from the fuses
		soc_bin_data = ((((u_int64_t)rCFG_FUSE1) << 32) | rCFG_FUSE0) >> 25;

		// Start with the base voltage
		soc_voltage = chipid_get_base_voltage();

		// Add in the correct bin from the "array"
		soc_voltage += 5 * ((soc_bin_data >> (index * 7)) & 0x7F);
	}

	return soc_voltage;
}

u_int32_t chipid_get_cpu_voltage(u_int32_t index)
{
	u_int32_t cpu_voltage;
	u_int64_t cpu_bin_data;

	if (index > CHIPID_CPU_VOLTAGE_COUNT) return 0;

	if (chipid_get_binning_revision() < MINIMUM_BINNING_VERSION) {
		// This part is unbinned to the binning version is not supported
		// Use the default voltage
		cpu_voltage = default_cpu_voltages[index];
	} else {
		// Read the CPU bin data from the fuses
		cpu_bin_data = ((((u_int64_t)rDVFM_FUSE(1)) << 32) | rDVFM_FUSE(0)) >> 5;

		// Start with the base voltage
		cpu_voltage = chipid_get_base_voltage();

		// Add in the correct bin from the "array"
		cpu_voltage += 5 * ((cpu_bin_data >> (index * 7)) & 0x7F);
	}

	return cpu_voltage;
}

#ifndef TARGET_RAM_VOLTAGE_OFFSET
#define TARGET_RAM_VOLTAGE_OFFSET 0
#endif

u_int32_t chipid_get_ram_voltage(u_int32_t index)
{
	u_int32_t ram_voltage;

	if (index > CHIPID_RAM_VOLTAGE_COUNT) return 0;
	
	// RAM voltage is not binned, use the default voltage
	ram_voltage = default_ram_voltages[index] + TARGET_RAM_VOLTAGE_OFFSET;

	return ram_voltage;
}

bool chipid_get_fuse_lock(void)
{
	return (rCFG_FUSE1 & (1 << 31)) != 0;
}

void chipid_set_fuse_lock(bool locked)
{
	if (locked) rCFG_FUSE1 |= 1 << 31;
}

int32_t chipid_get_cpu_temp_offset(u_int32_t cpu_number)
{
	int32_t temp_cal;

	switch (cpu_number) {
		case 0 : temp_cal = (rDVFM_FUSE(9) >>  0) & 0x7F; break;
		case 1 : temp_cal = (rDVFM_FUSE(9) >> 16) & 0x7F; break;
		default : return 0;
	}

	return 0x3B - temp_cal;
}

u_int32_t chipid_get_fused_thermal_sensor_70C(u_int32_t sensorID)
{
	u_int32_t temp_cal;

	switch (sensorID) {
		case 0: temp_cal = (rCFG_FUSE4 >>  8) & 0x7F; break;
		case 1: temp_cal = (rCFG_FUSE4 >> 24) & 0x7F; break;
		default : return 0;
	}
	
	return temp_cal;
}

u_int32_t chipid_get_fused_thermal_sensor_25C(u_int32_t sensorID)
{
	u_int32_t temp_cal;

	switch (sensorID) {
		case 0: temp_cal = (rCFG_FUSE4 >>  0) & 0x7F; break;
		case 1: temp_cal = (rCFG_FUSE4 >> 16) & 0x7F; break;
		default : return 0;
	}
	
	return temp_cal;	
}

u_int32_t chipid_get_fuse_revision(void)
{
        return (rCFG_FUSE0 >> 18) & 0xf;
}

static u_int32_t chipid_get_binning_revision(void)
{
	return (rDVFM_FUSE(1) >> (61 - 32)) & 7;
}

static u_int32_t chipid_get_base_voltage(void)
{
	return 25 * (1 + ((rDVFM_FUSE(0) >> 0) & 0x1F));
}
