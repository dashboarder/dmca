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
	return MINIPMGR_FUSE_CFG_FUSE0_PRODUCTION_MODE_XTRCT(rCFG_FUSE0) != 0;
}

bool chipid_get_raw_production_mode(void)
{
	return MINIPMGR_FUSE_CFG_FUSE0_PRODUCTION_MODE_XTRCT(rCFG_FUSE0_RAW) != 0;
}

void chipid_clear_production_mode(void)
{
	rCFG_FUSE0 &= ~MINIPMGR_FUSE_CFG_FUSE0_PRODUCTION_MODE_UMASK;
}

bool chipid_get_secure_mode(void)
{
	// demotion only applies to the SEP, so iBoot always reads
	// the raw value for secure mode (<rdar://problem/15182573>)
	return MINIPMGR_FUSE_CFG_FUSE0_SECURE_MODE_XTRCT(rCFG_FUSE0_RAW);
}

uint32_t chipid_get_security_domain(void)
{
	return MINIPMGR_FUSE_CFG_FUSE0_SECURITY_DOMAIN_XTRCT(rCFG_FUSE0);
}

uint32_t chipid_get_board_id(void)
{
	return MINIPMGR_FUSE_CFG_FUSE0_BID_XTRCT(rCFG_FUSE0);
}

uint32_t chipid_get_minimum_epoch(void)
{
	return MINIPMGR_FUSE_CFG_FUSE0_MINIMUM_EPOCH_XTRCT(rCFG_FUSE0);
}

uint32_t chipid_get_chip_id(void)
{
	return 0x8002;
}

uint32_t chipid_get_chip_revision(void)
{
	uint32_t fuse_val = MINIPMGR_FUSE_CFG_FUSE4_DEV_VERSION_XTRCT(rCFG_FUSE4);

	// we use 4 bits for base layer and 4 bits for metal,
	// the fuses use 3 for each
	return (fuse_val & 0x7) | (((fuse_val >> 3) & 0x7) << 4);
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
	return MINIPMGR_FUSE_CFG_FUSE1_AP_LOCK_XTRCT(rCFG_FUSE1) != 0;
}

void chipid_set_fuse_lock(bool locked)
{
	if (locked) {
		rCFG_FUSE1 |= MINIPMGR_FUSE_CFG_FUSE1_AP_LOCK_INSRT(1);
		asm("dsb sy");
		if (!chipid_get_fuse_lock()) {
			panic("Failed to lock fuses\n");
		}
	}
}

bool chipid_get_fuse_seal(void)
{
	return MINIPMGR_FUSE_CFG_FUSE1_SEAL_FUSES_XTRCT(rCFG_FUSE1) != 0;
}

uint32_t chipid_get_fuse_revision(void)
{
	return MINIPMGR_FUSE_CFG_FUSE4_REV_XTRCT(rCFG_FUSE4);
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
