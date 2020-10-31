/*
 * Copyright (C) 2009-2012 Apple Inc. All rights reserved.
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
#define rCFG_FUSE0	((0 << 31) | (3 << 28) | (0xE << 24) | (2 << 22) | \
			 (0 << 9) | (1 << 8) | (3 << 4) | (kPlatformSecurityDomainDarwin << 2) | (0 << 1) | (0 << 0) | \
			 _rCFG_FUSE0)
#endif

#if SUB_PLATFORM_S5L8945X
#define DEFAULT_SOC_VOLTAGE_LOW		1000
#define DEFAULT_SOC_VOLTAGE_MED		1100
#define DEFAULT_SOC_VOLTAGE_HIGH	1200
#define DEFAULT_CPU_VOLTAGE_LOW		1000
#define DEFAULT_CPU_VOLTAGE_MED		1150
#define DEFAULT_CPU_VOLTAGE_HIGH	1250
#define BASE_SOC_VOLTAGE_LOW		875
#define BASE_SOC_VOLTAGE_MED		975
#define BASE_SOC_VOLTAGE_HIGH		1075
#define BASE_CPU_VOLTAGE_LOW		875
#define BASE_CPU_VOLTAGE_MED		975
#define BASE_CPU_VOLTAGE_HIGH		1075
#endif

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
#if SUB_PLATFORM_S5L8945X
	return 0x8945;
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

u_int32_t chipid_get_fused_thermal_70C(void)
{
	return (rCFG_FUSE6 >> 8) & 0xff;
}

u_int32_t chipid_get_fused_thermal_25C(void)
{
	return rCFG_FUSE6 & 0xff;
}

u_int32_t chipid_get_fuse_revision(void)
{
	return (rCFG_FUSE1 >> 25) & 0xf;
} 

u_int32_t chipid_get_soc_voltage(u_int32_t index)
{
	u_int32_t soc_voltage = 0;
	u_int32_t soc_bin_offset_data = 0, soc_bin_data = (rCFG_FUSE1 >> 5) & 0x1FF;
	int32_t   soc_bin_offset;

#if DEBUG_BUILD && WITH_ENV
	soc_bin_offset_data = env_get_uint("soc-bin-offset", 0);

	switch (index) {
		case CHIPID_SOC_VOLTAGE_LOW	: soc_bin_offset_data = (soc_bin_offset_data >>  0) & 0xff;		break;
		case CHIPID_SOC_VOLTAGE_MED	: soc_bin_offset_data = (soc_bin_offset_data >>  8) & 0xff;		break;
		case CHIPID_SOC_VOLTAGE_HIGH	: soc_bin_offset_data = (soc_bin_offset_data >> 16) & 0xff;		break;
		default				:									break;
	}
#endif

	// if bin data is all zeros, bin data is not valid
	if (soc_bin_data == 0) {
		switch (index) {
			case CHIPID_SOC_VOLTAGE_LOW	: soc_voltage = DEFAULT_SOC_VOLTAGE_LOW;	break;
			case CHIPID_SOC_VOLTAGE_MED	: soc_voltage = DEFAULT_SOC_VOLTAGE_MED;	break;
			case CHIPID_SOC_VOLTAGE_HIGH	: soc_voltage = DEFAULT_SOC_VOLTAGE_HIGH;	break;
			default				:						break;
		}
	} else {
		switch (index) {
			case CHIPID_SOC_VOLTAGE_LOW	: soc_voltage = BASE_SOC_VOLTAGE_LOW  + 25 * ((soc_bin_data>>6)&7);	break;
			case CHIPID_SOC_VOLTAGE_MED	: soc_voltage = BASE_SOC_VOLTAGE_MED  + 25 * ((soc_bin_data>>3)&7);	break;
			case CHIPID_SOC_VOLTAGE_HIGH	: soc_voltage = BASE_SOC_VOLTAGE_HIGH + 25 * ((soc_bin_data>>0)&7);	break;
			default				:									break;
		}
	}

	soc_bin_offset = soc_bin_offset_data | (((soc_bin_offset_data & 0x80) == 0) ? 0 : 0xffffff00);
	soc_bin_offset *= 25;

	return soc_voltage + soc_bin_offset;
}

u_int32_t chipid_get_cpu_voltage(u_int32_t index)
{
	u_int32_t cpu_voltage = 0;
	u_int32_t cpu_bin_offset_data = 0, cpu_bin_data = ((((u_int64_t)rCFG_FUSE1 << 32) | rCFG_FUSE0) >> 28) & 0x1FF;
	int32_t   cpu_bin_offset;

#if DEBUG_BUILD && WITH_ENV
	cpu_bin_offset_data = env_get_uint("core-bin-offset", 0);

	switch (index) {
		case CHIPID_CPU_VOLTAGE_LOW	: cpu_bin_offset_data = (cpu_bin_offset_data >>  0) & 0xff;		break;
		case CHIPID_CPU_VOLTAGE_MED	: cpu_bin_offset_data = (cpu_bin_offset_data >>  8) & 0xff;	 	break;
		case CHIPID_CPU_VOLTAGE_HIGH	: cpu_bin_offset_data = (cpu_bin_offset_data >> 16) & 0xff;		break;
		default				:									break;
	}

#endif

	// if bin data is all zeros, bin data is not valid
	if (cpu_bin_data == 0) {
		switch (index) {
			case CHIPID_CPU_VOLTAGE_LOW	: cpu_voltage = DEFAULT_CPU_VOLTAGE_LOW;	break;
			case CHIPID_CPU_VOLTAGE_MED	: cpu_voltage = DEFAULT_CPU_VOLTAGE_MED;	break;
			case CHIPID_CPU_VOLTAGE_HIGH	: cpu_voltage = DEFAULT_CPU_VOLTAGE_HIGH;	break;
			default				:						break;
		}
	} else {
		switch (index) {
			case CHIPID_CPU_VOLTAGE_LOW	: cpu_voltage = BASE_CPU_VOLTAGE_LOW  + 25 * ((cpu_bin_data>>6)&7);	break;
			case CHIPID_CPU_VOLTAGE_MED	: cpu_voltage = BASE_CPU_VOLTAGE_MED  + 25 * ((cpu_bin_data>>3)&7);	break;
			case CHIPID_CPU_VOLTAGE_HIGH	: cpu_voltage = BASE_CPU_VOLTAGE_HIGH + 25 * ((cpu_bin_data>>0)&7);	break;
			default				:									break;
		}
	}

	cpu_bin_offset = cpu_bin_offset_data | (((cpu_bin_offset_data & 0x80) == 0) ? 0 : 0xffffff00);
	cpu_bin_offset *= 25;

	return cpu_voltage + cpu_bin_offset;
}

bool chipid_get_fuse_lock(void)
{
	return (rCFG_FUSE1 & (1 << 31)) != 0;
}

void chipid_set_fuse_lock(bool locked)
{
	if (locked) rCFG_FUSE1 |= 1 << 31;
}
