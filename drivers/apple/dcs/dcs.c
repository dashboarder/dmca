/*
 * Copyright (C) 2013-2015 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */

/*
 * Driver for the LPDDR4 compatible DRAM Control Subsystem (DCS) block
 */

#include <debug.h>
#include <sys/menu.h>
#include <drivers/dcs/dcs_init_lib.h>
#include <drivers/dram.h>
#include <drivers/power.h>
#include <drivers/miu.h>
#include <lib/env.h>
#include <platform.h>
#include <platform/soc/hwclocks.h>
#include <platform/chipid.h>
#include <platform/memmap.h>
#include <platform/timer.h>
#include <sys.h>
#include <string.h>

void mcu_late_init(void)
{
}

uint32_t density_to_Gb(int density)
{
	switch (density) {
		case JEDEC_LPDDR4_DENSITY_4Gb:
			return 4;
		case JEDEC_LPDDR4_DENSITY_6Gb:
			return 6;
		case JEDEC_LPDDR4_DENSITY_8Gb:
			return 8;
		case JEDEC_LPDDR4_DENSITY_12Gb:
			return 12;
		case JEDEC_LPDDR4_DENSITY_16Gb:
			return 16;
		case JEDEC_LPDDR4_DENSITY_24Gb:
			return 24;
		case JEDEC_LPDDR4_DENSITY_32Gb:
			return 32;
		default:
			panic("unknown LPDDR4 density %d", density);
	}
}

static struct dcs_memory_device_info _dcs_device_info;
static bool _dcs_device_info_inited;

const struct dcs_memory_device_info *dcs_get_memory_device_info(void)
{
	// if memory is not inited, device info is unknown, spin here ...
	if (false == _dcs_device_info_inited)
		panic("DCS Memory Device Info grab has jumped the gun");

	return ((const struct dcs_memory_device_info *)&_dcs_device_info);
}

// Returns memory size per amcc block
uint64_t dcs_get_memory_size(void)
{
	// if memory is not inited, density is unknown, spin here ...
	if (false == _dcs_device_info_inited)
		panic("DCS Memory Size grab has jumped the gun");

	// device density (in MBytes) * ( / 2 due to x16 lpddr4 devices) * num of channels * num of ranks
	return (density_to_Gb(_dcs_device_info.density) * (1024 / 8 / 2) * DCS_NUM_CHANNELS * DCS_NUM_RANKS);
}

uint32_t dcs_get_memory_info(void)
{
	uint8_t config_id = 0;

	// Read device info: vendor, revision, and configuration info
	// Configuration info: device width, device type, device density
	// We are assuming all of our devices are identical
	dcs_mrcmd(MR_READ, 1, 1, 0x5, (uintptr_t)&_dcs_device_info.vendor_id);
	dcs_mrcmd(MR_READ, 1, 1, 0x6, (uintptr_t)&_dcs_device_info.rev_id);
	dcs_mrcmd(MR_READ, 1, 1, 0x7, (uintptr_t)&_dcs_device_info.rev_id2);
	dcs_mrcmd(MR_READ, 1, 1, 0x8, (uintptr_t)&config_id);

	if ((_dcs_device_info.vendor_id == 0) || (config_id == 0))
		panic("failed to read vendor-id/config-id, vid:%08x, config:%08x, rev:%08x\n",
		      _dcs_device_info.vendor_id, config_id, _dcs_device_info.rev_id);

	_dcs_device_info.width = (32 >> ((config_id >> JEDEC_MR8_WIDTH_SHIFT) & JEDEC_MR8_WIDTH_MASK)) >> 3;
	_dcs_device_info.density = ((config_id >> JEDEC_MR8_DENSITY_SHIFT) & JEDEC_MR8_DENSITY_MASK);
	_dcs_device_info.type =  ((config_id >> JEDEC_MR8_TYPE_SHIFT) & JEDEC_MR8_TYPE_MASK);

	_dcs_device_info_inited = true;

	uint32_t device_size_Gbits = density_to_Gb(_dcs_device_info.density);
	uint32_t total_size_Mbytes = dcs_get_memory_size();

	dprintf(DEBUG_INFO, "sdram vendor id:0x%02x rev id:0x%02x rev id2:%02x\n",
		  _dcs_device_info.vendor_id, _dcs_device_info.rev_id, _dcs_device_info.rev_id2);
	dprintf(DEBUG_INFO, "sdram config: width %d/%d Gbit/type %d\n", _dcs_device_info.width << 3,
		  device_size_Gbits,  _dcs_device_info.type);

	// Check against Hardware Register Min/Max
	if (total_size_Mbytes < 128 || total_size_Mbytes > (16 * 1024) )
		panic("unsupported DRAM density: %d Gbit  (with Chan==%d x Rank==%d yields %d MB)",
		      device_size_Gbits, DCS_NUM_CHANNELS, DCS_NUM_RANKS, total_size_Mbytes);

	return 0;
}

//================================================================================
void dcs_change_freq(uint32_t freq_bin)
{
	// Change the MCU Clk Frequency to the new Frequency
	dbgprintf(DCS_DBG_FREQ, "Changing Frequency to Bin %d\n", freq_bin);

	// Make the actual Change by sending the appropriate message to the PMGR
	uint32_t perf_level = (freq_bin == 0)? kPerformanceMemoryFull:
			      (freq_bin == 1)? kPerformanceMemoryMid:
			      (freq_bin == 3)? kPerformanceMemoryLow:
			      DCS_NONVAL;
	if (perf_level == DCS_NONVAL) {
		shim_panic("Unsupported Frequency Bin Request: %d\n", freq_bin);
	}

	clocks_set_performance(perf_level);
}

//================================================================================
void dcs_store_memory_calibration(void *cal_values, uint32_t cal_size)
{
	// Save the calibration array to PMU nvram
#if 0
	if (power_store_memory_calibration(cal_values, cal_size) == 0)
		shim_panic("Unable to save memory calibration values to PMU nvram\n");
#endif
}

void dcs_load_memory_calibration(void *cal_values, uint32_t cal_size)
{
	// Retrieve calibration array from PMU nvram
#if 0
	if (power_load_memory_calibration(cal_values, cal_size) == 0)
		shim_panic("Unable to load memory calibration values from PMU nvram\n");
#endif
}

// To dump calibration results from iBoot menu command
static int dump_mem_calibration_info(int argc, struct cmd_arg *args)
{
	dcs_init_debug_enable(DCS_DBG_CAL);
	debug_level_dcs = DEBUG_CRITICAL;
	calibration_cfg_params_init(false, DCS_NUM_CHANNELS, DCS_NUM_RANKS);
	calibration_dump_results(SELECT_CAL_ALL, false);
	return 0;
}
MENU_COMMAND_DEBUG(memcal_info, dump_mem_calibration_info, "Prints memory calibration results", NULL);

//================================================================================
////////////////////////////////////////////////////////////
//
// DCS Init Sequence
// Incorporates these specs from SEG

// Maui: Init_A1.html dated Mar 2, 2015, Init_B0.html dated May 20, 2015, Init_B0_1200.html dated June 2, 2015, Init_1200.html dated Oct 29, 2015

// Elba: Init.html dated March 04, 2016, Init_A0.html dated July 14 15, 2015

// s8003: Init_A0.html dated May 20, 2015, Init_A0_1200.html dated June 17, 2015, Init_1200.html dated Oct 29, 2015

// M8: Init.html dated Apr 2, 2015

// Cayman: Init.html dated Feb 18, 2015

////////////////////////////////////////////////////////////

int32_t dcs_init(dcs_boottype_t boot_type)
{
	uint32_t num_dcs = DCS_NUM_CHANNELS;
	uint32_t num_rnk = DCS_NUM_RANKS;
	dcs_config_params_t *param_table;
#ifndef DCS_RUN_AT_50MHZ
	volatile uint32_t *cal_results_save_addr = (volatile uint32_t *) MEMORY_CALIB_SAVE_BASE_ADDR;
#if DCS_DO_CALIBRATION
	struct amp_calibration_params *cal_cfg = NULL;
#endif // #if DCS_DO_CALIBRATION
#endif // #ifndef DCS_RUN_AT_50MHZ

	dprintf(DEBUG_INFO, "dcs_init() ... !\n");

	// Config our boot type and note the ChipID/Rev
	if (!dcs_init_config(boot_type, platform_get_chip_id(), platform_get_chip_revision(),
			     num_dcs, num_rnk)) {
		panic("DCS Init Config is NOT RIGHT");
	}

	dcs_init_debug_enable(DCS_DBG_OVERVIEW);
	dcs_init_debug_enable(DCS_DBG_MILESTONE);
	// dcs_init_debug_enable(DCS_DBG_REG_ACCESS);
	// dcs_init_debug_enable(DCS_DBG_CHAN_DETAIL);
	// dcs_init_debug_enable(DCS_DBG_SPIN);
	// dcs_init_debug_enable(DCS_DBG_MR_READ);
	// dcs_init_debug_enable(DCS_DBG_CAL);
	dcs_init_debug_enable(DCS_DBG_CAL_MILESTONE);

	dcs_init_debug_enable(DCS_DBG_FREQ);

	// Initialize the DCS Parameter Table (uses platform-specific callout)
	param_table = dcs_init_param_table();

// <rdar://problem/22429659> Maui/Malta/Elba: Reset DRAM during panic reboot in LLB to avoid problems with init sequence
#if SUB_PLATFORM_S8000 || SUB_PLATFORM_S8003 || SUB_PLATFORM_S8001
#if WITH_HW_POWER
		int result;
		uint8_t boot_stage;

		// If we rebooted to save panic log, try resetting the DRAM to reset the FSP on the DRAM
		result = power_get_nvram(kPowerNVRAMiBootErrorStageKey, &boot_stage);

		// if we properly read the PMU NVRAM and it says we are booting to save panic log ...
		if(result == 0 && boot_stage == kPowerNVRAMiBootStagePanicSave) {
			dcs_reg_write_all_chan(rDCS_AMP_DRAM_RESETN(AMP_CA), 0);
			// Leave reset asserted for 200 us
			delay_for_us(200);
			dcs_reg_write_all_chan(rDCS_AMP_DRAM_RESETN(AMP_CA), 1);
			// Wait 2 ms before starting mem init sequence
			delay_for_us(2000);
		}
#endif
#endif

	//================================================================================
	// Step  0: AMC Prolog
	//================================================================================
	dbgprintf(DCS_DBG_MILESTONE, "DCS Init Step %2d: %s\n", 0, "Prolog");
	dcs_init_Prolog();

	//================================================================================
	// Step  1: AMC Initial Configuration
	//================================================================================
	dbgprintf(DCS_DBG_MILESTONE, "DCS Init Step %2d: %s\n", 1, "Initial AMC Config");
	dcs_init_AMC_Initial_Config();

	//================================================================================
	// Step  2: AMP Initial Configuration
	//================================================================================
	dbgprintf(DCS_DBG_MILESTONE, "DCS Init Step %2d: %s\n", 2, "AMP Initial Config");
	dcs_init_AMP_Initial_Config();

	//================================================================================
	// Step  3: Self-Refresh Exit
	//================================================================================
	dbgprintf(DCS_DBG_MILESTONE, "DCS Init Step %2d: %s\n", 3, "Self-Refresh Exit");
	dcs_init_Self_Refresh_Exit_a();

	if (izColdBoot && !izFPGA) {
		// Change MCU Clk --> 50Mz
		dcs_change_freq(3);

		// Wait 5 us after Switch to 50MHz to avoid race condition
		dcs_spin(5, "after 50MHz\n");
	}

	dcs_init_Self_Refresh_Exit_b();

	//================================================================================
	// Step  4: DRAM Reset, ZQ Calibration & Configuration (cold boot only)
	//================================================================================
	dbgprintf(DCS_DBG_MILESTONE, "DCS Init Step %2d: %s\n", 4, "ZQ Calibration & Configuration");
	dcs_init_ZQ_Cal_Cfg();

	//================================================================================
	// Step  5: Topology-specific configuration
	//================================================================================
	dbgprintf(DCS_DBG_MILESTONE, "DCS Init Step %2d: %s\n", 5, "Memory Topology Config");
	dcs_init_AddrCfg_Cfg();

	dcs_get_memory_info();
	uint32_t total_size_Mbytes = dcs_get_memory_size();

	dcs_init_TotalSize_Cfg(total_size_Mbytes);

    // DRAM vendor-specific workarounds
	dcs_dram_workarounds(boot_type);

	//================================================================================
	// Step  6: Prepare for switch from boot-clock speed to normal operation speed
	//================================================================================
	dbgprintf(DCS_DBG_MILESTONE, "DCS Init Step %2d: %s\n", 6, "Reenable_AMC_Scheduler");
	dcs_init_Reenable_AMC_Scheduler();

#if DCS_RUN_AT_50MHZ
	dprintf(DEBUG_CRITICAL, "DCS Init: memory frequency fixed to 50 MHz\n");
#else

#if DCS_DO_CALIBRATION
	cal_cfg = calibration_cfg_params_init(izResume, num_dcs, num_rnk);

	// Override the chip_id and chip_rev default values with actual values
	cal_cfg->chip_id = platform_get_chip_id();
	cal_cfg->chip_rev = platform_get_chip_revision();

	// Coarse CS training disabled on Maui A0/A1
	if(cal_cfg->chip_id == 0x8000 && cal_cfg->chip_rev < CHIP_REVISION_B0)
		cal_cfg->disable_coarse_cs_training = true;

	cal_cfg->clk_period_bin0 = param_table->freq[0].clk_period;
	cal_cfg->clk_period_bin1 = param_table->freq[1].clk_period;

	// Note if we will use Widest-Eye for calculating Optimal Vref
	if (dcs_init_variant_enable(0) & DCS_VARIANT_VREF_USES_WIDEST) {
		cal_cfg->opt_vref_mode = DCS_CAL_OPT_VREF_MODE_WIDEST;
	}

	dbgprintf(DCS_DBG_CAL_MILESTONE, "DCS Cal: Config Init'd\n");
#endif

	// Only perform steps 7-10 if frequequency bucket 1 is supported
	if(param_table->supported_freqs & DCS_FREQ(1)) {

		//================================================================================
		// Step  7: Setup registers for CA calibration for bucket 1
		//================================================================================
		dbgprintf(DCS_DBG_MILESTONE, "DCS Init Step %2d: %s\n", 7, "CA Calibration Setup--800MHz");
		dcs_init_CA_Cal_Setup_Freq1();

		//================================================================================
		// Step  8: AMP Dynamic CA Vref Timing Calibration at Vmin 800MHz
		//================================================================================
		dbgprintf(DCS_DBG_MILESTONE, "DCS Init Step %2d: %s\n", 8, "CA Vref Calibration--800MHz, Vmin");

#if DCS_DO_CALIBRATION
		// Use these values for 800MHz
		cal_cfg->RL       = 16;
		cal_cfg->WL       = 12;
		// We will be calibrating for @800MHz (FreqBin 1)
		cal_cfg->freq_bin = 1;

		if (izColdBoot) {
			dbgprintf(DCS_DBG_CAL_MILESTONE, "DCS Cal: FreqBin %d CA Cal\n", cal_cfg->freq_bin);
			calibrate_ca();
		} else {
			dbgprintf(DCS_DBG_CAL_MILESTONE, "DCS Cal: FreqBin %d Cal Regs Restore\n", cal_cfg->freq_bin);
			// Restore Calibration Parameters from PMU
			calibration_save_restore_ca_wrlvl_regs(CALIB_RESTORE, num_dcs, cal_cfg->freq_bin);
		}
#else
		dbgprintf(DCS_DBG_MILESTONE, "DCS is Skipping Calibration, per Build Config\n");
#endif
		dbgprintf(DCS_DBG_MILESTONE, "DCS Init Step %2d: %s\n", 8, "Post CA Vref Calibration");
		dcs_init_CA_Cal_Freq1();

		//================================================================================
		// Step  9: Setup registers for DQ calibration for bucket 1
		//================================================================================
		dbgprintf(DCS_DBG_MILESTONE, "DCS Init Step %2d: %s\n", 9, "DQ Calibration Setup--800MHz");
		dcs_init_DQ_Cal_Setup_Freq1_a();

		// Change MCU Clk --> bucket 1
		if(!izFPGA)
			dcs_change_freq(1);

		dcs_init_DQ_Cal_Setup_Freq1_b();

		//================================================================================
		// Step 10: PHY write DQ calibration
		//================================================================================
		dbgprintf(DCS_DBG_MILESTONE, "DCS Init Step %2d: %s\n", 10, "Wr DQ Cal--800MHz");

#if DCS_DO_CALIBRATION
		if (izColdBoot) {
			dbgprintf(DCS_DBG_CAL_MILESTONE, "DCS Cal: FreqBin %d WrLvl\n", cal_cfg->freq_bin);
			calibrate_wrlvl();

			dcs_init_post_wrlvl();

			dbgprintf(DCS_DBG_CAL_MILESTONE, "DCS Cal: FreqBin %d Cal Regs Save\n", cal_cfg->freq_bin);
			// Save Calibration Parameters (so far) to Memory Stash
			calibration_save_restore_ca_wrlvl_regs(CALIB_SAVE, num_dcs, cal_cfg->freq_bin);

			dbgprintf(DCS_DBG_CAL_MILESTONE, "DCS Cal: FreqBin %d RdDQ Cal\n", cal_cfg->freq_bin);
			calibrate_rddq();
			dbgprintf(DCS_DBG_CAL_MILESTONE, "DCS Cal: FreqBin %d WrDQ Cal\n", cal_cfg->freq_bin);
			calibrate_wrdq();
		}
#else
		dbgprintf(DCS_DBG_MILESTONE, "DCS is Skipping Calibration, per Build Config\n");
#endif

		dcs_init_wrdq_skew();

		cal_results_save_addr = dcs_save_calibration_results(cal_results_save_addr, DCS_FREQ(1));
	} // if(param_table->supported_freqs & DCS_FREQ1) {

	//================================================================================
	// Step 11: Setup registers for CA calibration for bucket 0
	//================================================================================
	dbgprintf(DCS_DBG_MILESTONE, "DCS Init Step %2d: %s\n", 11, "CA Vref Calibration Setup--1200MHz");
	dcs_init_CA_Cal_Setup_Freq0();

	//================================================================================
	// Step 12: AMP Dynamic CA Timing Calibration
	//================================================================================
	dbgprintf(DCS_DBG_MILESTONE, "DCS Init Step %2d: %s\n", 12, "CA Vref Calibration--1200MHz, Vnom");

#if DCS_DO_CALIBRATION

	if(param_table->freq[0].clk_period == CLK_PERIOD_1600) {
		// Use these values for 1600MHz
		cal_cfg->RL       = 28;
		cal_cfg->WL       = 14;
	} else if(param_table->freq[0].clk_period == CLK_PERIOD_800) {
		// Use these values for 800MHz
		cal_cfg->RL       = 14;
		cal_cfg->WL       = 12;
	} else {
		// Use these values for 1200MHz
		cal_cfg->RL       = 24;
		cal_cfg->WL       = 12;
	}

	// We will be calibrating for FreqBin 0
	cal_cfg->freq_bin = 0;

	if (izColdBoot) {
		dbgprintf(DCS_DBG_CAL_MILESTONE, "DCS Cal: FreqBin %d CA Cal\n", cal_cfg->freq_bin);
		calibrate_ca();
#if SUB_PLATFORM_S8000 || SUB_PLATFORM_S8003
 		dcs_dram_workarounds_step12();
#endif
	}
#else
	dbgprintf(DCS_DBG_MILESTONE, "DCS is Skipping Calibration, per Build Config\n");
#endif

	//================================================================================
	// Step 13: Setup registers for DQ calibration for bucket 0
	//================================================================================
	dbgprintf(DCS_DBG_MILESTONE, "DCS Init Step %2d: %s\n", 13, "DQ Calibration Setup--1200MHz");
	dcs_init_DQ_Cal_Setup_Freq0_a();

	// Change MCU Clk --> bucket 0
	if(!izFPGA)
		dcs_change_freq(0);

	dcs_init_DQ_Cal_Setup_Freq0_b();

	//================================================================================
	// Step 14: AMP Dynamic DQ Calibration
	//================================================================================
	dbgprintf(DCS_DBG_MILESTONE, "DCS Init Step %2d: %s\n", 14, "Wr DQ Cal--1200MHz, Vmin");

#if DCS_DO_CALIBRATION
	if (izColdBoot) {
		dbgprintf(DCS_DBG_CAL_MILESTONE, "DCS Cal: FreqBin %d WrLvl\n", cal_cfg->freq_bin);
		calibrate_wrlvl();

		dcs_init_post_wrlvl();

		dbgprintf(DCS_DBG_CAL_MILESTONE, "DCS Cal: FreqBin %d Cal Regs Save\n", cal_cfg->freq_bin);
		// Save Calibration Parameters (all the rest) to Memory Stash & then copy Stash to PMU
		calibration_save_restore_ca_wrlvl_regs(CALIB_SAVE, num_dcs, cal_cfg->freq_bin);

		dbgprintf(DCS_DBG_CAL_MILESTONE, "DCS Cal: FreqBin %d RdDQ Cal\n", cal_cfg->freq_bin);
		calibrate_rddq();
		dbgprintf(DCS_DBG_CAL_MILESTONE, "DCS Cal: FreqBin %d WrDQ Cal\n", cal_cfg->freq_bin);
		calibrate_wrdq();

		// Save off any applicable freq 0 registers
		cal_results_save_addr = dcs_save_calibration_results(cal_results_save_addr, DCS_FREQ(0));
	}
#else
	dbgprintf(DCS_DBG_MILESTONE, "DCS is Skipping Calibration, per Build Config\n");
#endif

#endif // #if DCS_RUN_AT_50MHZ ... #else

	//================================================================================
	// Step 15: Setup Registers for Boot
	//================================================================================
	dbgprintf(DCS_DBG_MILESTONE, "DCS Init Step %2d: %s\n", 15, "Setup Registers for Boot");
	dcs_init_Reg_for_Boot();

	//================================================================================
	// Step 16: Enable Other Features
	//================================================================================
	dbgprintf(DCS_DBG_MILESTONE, "DCS Init Step %2d: %s\n", 16, "Enable Other Features");
	dcs_init_MoreFeatures();

	//================================================================================
	// Step 17: Enable Fast Critical Word Forwarding Feature
	//================================================================================
	dbgprintf(DCS_DBG_MILESTONE, "DCS Init Step %2d: %s\n", 17, "Fast Critical Word Forwarding");
	dcs_init_Fast_Critical_Word_Forwarding();

	//================================================================================
	// Step 18: Enable Power- and Clock-Gating Features; Config MCC and Global Timers
	//================================================================================
	dbgprintf(DCS_DBG_MILESTONE, "DCS Init Step %2d: %s\n", 18, "Gating and Global Timers");
	dcs_init_Gating_Global_Timers();

	//================================================================================
	// Step 19: ODTS read & set interval for periodic MR4 on-die Temp sensor reading
	//================================================================================
	dbgprintf(DCS_DBG_MILESTONE, "DCS Init Step %2d: %s\n", 19, "ODTS & Die Temp");
	dcs_init_ODTS();

// rdar://problem/23244578, <rdar://problem/23355941>: Allow zqctimer value to be saved at this point to reconfig sequence
#if SUB_PLATFORM_S8000 || SUB_PLATFORM_S8003 || SUB_PLATFORM_S8001
#ifndef DCS_RUN_AT_50MHZ
	cal_results_save_addr = dcs_save_calibration_results(cal_results_save_addr, 0);
#endif
#endif

	//================================================================================
	// Program tunables at the end
	//================================================================================
	dbgprintf(DCS_DBG_MILESTONE, "DCS Init Tunables\n");
	dcs_init_apply_tunables();

	//================================================================================
	// Restore clock frequency to bucket 1 if it is supported
	//================================================================================
	if(!izFPGA)
		if(param_table->supported_freqs & DCS_FREQ(1))
			dcs_change_freq(1);

	// cache memory info for later
	platform_set_memory_info_with_revids(_dcs_device_info.vendor_id, dcs_get_memory_size(), _dcs_device_info.rev_id, _dcs_device_info.rev_id2);

	dbgprintf(DCS_DBG_MILESTONE, "DCS Init DONE!!\n");
	return 0;
}

//================================================================================
