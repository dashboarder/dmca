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

#include "amp_v3_shim.h"

// This array will hold the contents of memory that will be used for dq calibration
// static uint8_t dqcal_saved_data[AMC_NUM_RANKS][sizeof(DQ_PRBS7_PATTERNS) * AMC_NUM_CHANNELS]__aligned(32);

static void shim_save_restore_memory_region(uint32_t save_or_restore);

///////////////////////////////////////////////////////////////////////////////
////// Local functions
///////////////////////////////////////////////////////////////////////////////

// Before starting dq calibration, saves the contents of dram region that will be written to with calibration patterns.
// After calibration is complete, restores the contents back to DRAM.
static void shim_save_restore_memory_region(uint32_t save_or_restore)
{
}

///////////////////////////////////////////////////////////////////////////////
////// Global functions
///////////////////////////////////////////////////////////////////////////////

void shim_init_calibration_params(struct amp_calibration_params *cfg_params)
{
	// Set the number of channels and ranks on this target
	cfg_params->num_channels = AMC_NUM_CHANNELS;
	cfg_params->num_ranks = AMC_NUM_RANKS;
	
	// Fiji A0 has a 1 to 1 deskew to sdll step ratio
	if((platform_get_chip_id() == 0x7000) && (platform_get_chip_revision() == CHIP_REVISION_A0)) {
		cfg_params->sdll_scale = 1;
		cfg_params->deskew_scale = 1;
	}
}

void shim_configure_pre_ca(void)
{
	amc_calibration_start(true);
}

void shim_enable_rddqcal(bool enable)
{
	amc_enable_rddqcal(enable);
}

void shim_configure_post_wrlvl(struct amp_calibration_params *cfg_params)
{
	amc_calibration_start(false);
}

void shim_configure_pre_wrdq(bool resume)
{
	amc_calibration_start(true);

	// ok to keep PSQWQCTL0 and PSQWQCTL1 at their value setup for wrdqcal even for the rddqcal that follows
	amc_wrdqcal_start(true);
}

void shim_configure_post_prbs_rddq(struct amp_calibration_params *cfg_params)
{
	amc_wrdqcal_start(false);

	amc_calibration_start(false);
	
	// Save off the CA, WrLvl, Rddq, and Wrdq offsets to PMU
	if (!(cfg_params->resume))
		calibration_save_restore_regs(CALIB_SAVE, cfg_params->num_channels);
}

void shim_mrcmd_to_ch_rnk(uint8_t rw, uint8_t channel, uint8_t rank, int32_t reg, uintptr_t val)
{
	amc_mrcmd_to_ch_rnk((amc_mrcmd_op_t) rw, channel, rank, reg, val);
}

uint64_t shim_compute_dram_addr(uint32_t ch, uint32_t rnk, uint32_t bank, uint32_t row, uint32_t col)
{
	return amc_get_uncached_dram_virt_addr(ch, rnk, bank, row, col);
}

uint32_t shim_get_consecutive_bytes_perchnrnk(void)
{
	// query AMC for how many consecutive bytes before channel interleaving
	return amc_get_consecutive_bytes_perchnrnk();
}

void shim_store_memory_calibration(void *cal_values, uint32_t cal_size)
{
	if (power_store_memory_calibration(cal_values, cal_size) == 0)
		panic("Unable to save memory calibration values to PMU nvram\n");
}
void shim_load_memory_calibration(void *cal_values, uint32_t cal_size)
{
	if (power_load_memory_calibration(cal_values, cal_size) == 0)
		panic("Unable to load memory calibration values from PMU nvram\n");
}
