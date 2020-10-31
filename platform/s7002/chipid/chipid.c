/*
 * Copyright (C) 2013 Apple Inc. All rights reserved.
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
	// we never demote secure mode on platforms that don't have an SEP
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
        return 0x7002;
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
	return ((uint64_t)rECIDHI << 32) | rECIDLO;
}

uint64_t chipid_get_die_id(void)
{
	return ((uint64_t)rECIDHI << 32) | rECIDLO;
}

uint32_t chipid_get_soc_voltage(uint32_t index)
{
	uint32_t soc_voltage = 0;

	return soc_voltage;
}

uint32_t chipid_get_cpu_voltage(uint32_t index)
{
	uint32_t cpu_voltage = 0;

	return cpu_voltage;
}


uint32_t chipid_get_ram_voltage(uint32_t index)
{
	uint32_t sram_voltage = 0;

	return sram_voltage;
}

bool chipid_get_fuse_lock(void)
{
	return (rCFG_FUSE1 & (1 << 31)) != 0;
}

void chipid_set_fuse_lock(bool locked)
{
	if (locked) rCFG_FUSE1 |= (1 << 31);
}

uint32_t chipid_get_fuse_revision(void)
{
        return (rCFG_FUSE0 >> 18) & 0xf;
}

uint32_t chipid_get_total_rails_leakage()
{
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
