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

#ifdef ENV_IBOOT
#include <drivers/dcs/dcs_init_lib.h>
#elif defined ENV_DV
#include "dcs_shim.h"
#elif defined ENV_CFE
#include "cfe/amp_v3_shim.h"
#else
#error "Unidentified compilation environment for shared memory calibration code"
#endif

#define MAX(X,Y) ((X > Y) ? X : Y)
#define MIN(X,Y) ((X < Y) ? X : Y)
#define NEGATE(X) (-1 * X) 
#define POS(X) ((X < 0) ? 0 : X)

// In non-DV environments, ROMCODE_SEGMENT & MCODE_SEGMENT prefixes would be empty
#ifdef ENV_DV
   #ifndef AMS_SIM_ENV
      #define _ROMCODE_SEGMENT_PREFIX ROMCODE_SEGMENT
      #define _MCODE_SEGMENT_PREFIX MCODE_SEGMENT
   #else
      #define _ROMCODE_SEGMENT_PREFIX 
      #define _MCODE_SEGMENT_PREFIX 
   #endif
#else
   #define _ROMCODE_SEGMENT_PREFIX 
   #define _MCODE_SEGMENT_PREFIX 
#endif


/*
 * DQ_PRBS7_PATTERNS: Currently unused
 */
#if 0
// PRBS7 patterns used for ca calibration. cfg_params.cacalib_sw_loops * cfg_params.cacalib_hw_loops must equal 64
// We extend the size by 8 and repeat the first 8 values because when swloop=64, we don't go outside the array for programming CACALPAT1-7


// TODO TODO : What would be the final
// 1. Vref values & ranges for CA, RDDQ & WRDQ
// 2. Patter and invert masks for PRBS4I.

// PRBS patterns for Wrdq and Rddq (for the one after Wrdq) calibration
_MCODE_SEGMENT_PREFIX static const uint32_t DQ_PRBS7_PATTERNS[DQ_NUM_PATTERNS] = {
	0x85858585, 0x4a4a4a4a, 0x9a9a9a9a, 0x9e9e9e9e, 0xa1a1a1a1, 0x88888888, 0xffffffff, 0xcfcfcfcf,
	0xd0d0d0d0, 0x04040404, 0x3f3f3f3f, 0x29292929, 0x77777777, 0x30303030, 0x1f1f1f1f, 0xd4d4d4d4,
	0x3b3b3b3b, 0x16161616, 0x5e5e5e5e, 0x47474747, 0x2f2f2f2f, 0xcbcbcbcb, 0xefefefef, 0x2d2d2d2d,
	0x48484848, 0x19191919, 0x68686868, 0xe4e4e4e4, 0x24242424, 0xc2c2c2c2, 0x65656565, 0x51515151,
	0x71717171, 0x8c8c8c8c, 0xc0c0c0c0, 0xe6e6e6e6, 0xa7a7a7a7, 0x34343434, 0x20202020, 0xfdfdfdfd,
	0x4c4c4c4c, 0x26262626, 0x41414141, 0x93939393, 0x14141414, 0xdddddddd, 0xb1b1b1b1, 0x6a6a6a6a,
	0x67676767, 0xd2d2d2d2, 0x87878787, 0xc9c9c9c9, 0x6c6c6c6c, 0xdbdbdbdb, 0x0d0d0d0d, 0xb5b5b5b5,
	0x55555555, 0x4e4e4e4e, 0xa5a5a5a5, 0xb7b7b7b7, 0xd6d6d6d6, 0xb8b8b8b8, 0xe0e0e0e0, 0x1b1b1b1b,
	0xebebebeb, 0x12121212, 0x61616161, 0x6e6e6e6e, 0x58585858, 0xfbfbfbfb, 0xf0f0f0f0, 0xf9f9f9f9,
	0x73737373, 0x0f0f0f0f, 0x36363636, 0xa3a3a3a3, 0x0b0b0b0b, 0x09090909, 0x8a8a8a8a, 0x7c7c7c7c,
	0x39393939, 0x95959595, 0xa8a8a8a8, 0x02020202, 0x83838383, 0xf6f6f6f6, 0x45454545, 0xacacacac,
	0x3d3d3d3d, 0xaaaaaaaa, 0x81818181, 0x75757575, 0xb3b3b3b3, 0xe9e9e9e9, 0x91919191, 0x97979797,
	0x2b2b2b2b, 0xf4f4f4f4, 0xc6c6c6c6, 0x5a5a5a5a, 0x78787878, 0x06060606, 0xbcbcbcbc, 0xdfdfdfdf,
	0x32323232, 0x9c9c9c9c, 0x22222222, 0x7e7e7e7e, 0xbabababa, 0x63636363, 0xedededed, 0xaeaeaeae,
	0xbebebebe, 0x5c5c5c5c, 0xc4c4c4c4, 0xd9d9d9d9, 0x8e8e8e8e, 0x43434343, 0x10101010, 0xe2e2e2e2,
	0x98989898, 0x1d1d1d1d, 0x57575757, 0xcdcdcdcd, 0x53535353, 0xf2f2f2f2, 0x7a7a7a7a, 0x85858585
};
#endif

_MCODE_SEGMENT_PREFIX static uint32_t mdllcode[DCS_NUM_CHANNELS][3];		// 1 for each AMP_DQ, and 1 for AMP_CA
_MCODE_SEGMENT_PREFIX static uint32_t cacal_patterns_mask[CA_NUM_PATTERNS];

// This array will hold the calibration values to be saved for resume boot.
// Size is CALIB_PMU_BYTES, as this is the max space available in PMU for calibration, in iBoot environment
_MCODE_SEGMENT_PREFIX static uint8_t cal_bits[CALIB_PMU_BYTES] = { 0 };

/*
 * Following global arrays will record passing points for calibration operations
 */

// Used to save calibration values for each bit per channel and rank for every iteration
_MCODE_SEGMENT_PREFIX static int32_t cacal_per_loopchrnk_right[DCS_NUM_CHANNELS * DCS_NUM_RANKS * CACAL_MAX_SWLOOPS][CA_NUM_VREF][CA_NUM_BITS];
_MCODE_SEGMENT_PREFIX static int32_t cacal_per_loopchrnk_left[DCS_NUM_CHANNELS * DCS_NUM_RANKS * CACAL_MAX_SWLOOPS][CA_NUM_VREF][CA_NUM_BITS];
_MCODE_SEGMENT_PREFIX static int32_t cscal_per_loopchrnk_right[DCS_NUM_CHANNELS * DCS_NUM_RANKS * CACAL_MAX_SWLOOPS];
_MCODE_SEGMENT_PREFIX static int32_t cscal_per_loopchrnk_left[DCS_NUM_CHANNELS * DCS_NUM_RANKS * CACAL_MAX_SWLOOPS];

// Used to record values from CS training which will be used in CA training
_MCODE_SEGMENT_PREFIX static uint32_t cksdll_cscal[CA_NUM_VREF]; 
_MCODE_SEGMENT_PREFIX static uint32_t cssdll_cscal[CA_NUM_VREF]; 
_MCODE_SEGMENT_PREFIX static uint32_t casdll_cscal[CA_NUM_VREF]; 

// cs data aggregated over all iterations
_MCODE_SEGMENT_PREFIX static int32_t cscal_per_chrnk_right[DCS_NUM_CHANNELS * DCS_NUM_RANKS];
_MCODE_SEGMENT_PREFIX static int32_t cscal_per_chrnk_left[DCS_NUM_CHANNELS * DCS_NUM_RANKS];
	
// ca data aggregated over all iterations
_MCODE_SEGMENT_PREFIX static int32_t cacal_per_chrnk_right[DCS_NUM_CHANNELS * DCS_NUM_RANKS][CA_NUM_BITS];
_MCODE_SEGMENT_PREFIX static int32_t cacal_per_chrnk_left[DCS_NUM_CHANNELS * DCS_NUM_RANKS][CA_NUM_BITS];

// wrlvl data aggregated over all iterations, we save value for 4 byte lanes + the ca value
_MCODE_SEGMENT_PREFIX static uint32_t wrlvlcal_per_chrnk_rise[DCS_NUM_CHANNELS * DCS_NUM_RANKS][DQ_NUM_BYTES + 1];
_MCODE_SEGMENT_PREFIX static uint32_t wrlvlcal_per_chrnk_fall[DCS_NUM_CHANNELS * DCS_NUM_RANKS][DQ_NUM_BYTES + 1];

// rddq data aggregated over all iterations
_MCODE_SEGMENT_PREFIX static int32_t rddqcal_per_chrnk_right[DCS_NUM_CHANNELS * DCS_NUM_RANKS][RDDQ_NUM_VREF][DQ_TOTAL_BITS + DMI_TOTAL_BITS];
_MCODE_SEGMENT_PREFIX static int32_t rddqcal_per_chrnk_left[DCS_NUM_CHANNELS * DCS_NUM_RANKS][RDDQ_NUM_VREF][DQ_TOTAL_BITS + DMI_TOTAL_BITS];

// wrdq data aggregated over all iterations
_MCODE_SEGMENT_PREFIX static int32_t wrdqcal_per_chrnk_right[DCS_NUM_CHANNELS * DCS_NUM_RANKS][WRDQ_NUM_VREF][DQ_TOTAL_BITS + DMI_TOTAL_BITS];
_MCODE_SEGMENT_PREFIX static int32_t wrdqcal_per_chrnk_left[DCS_NUM_CHANNELS * DCS_NUM_RANKS][WRDQ_NUM_VREF][DQ_TOTAL_BITS + DMI_TOTAL_BITS];

// Indicates the optimal Vref value for CA calibration
// Along with Vref value, also save Vref range here
_MCODE_SEGMENT_PREFIX static uint32_t vref_opt_ca[DCS_NUM_CHANNELS * DCS_NUM_RANKS * CACAL_MAX_SWLOOPS];

// Indicates the optimal Vref value for RDDQ calibration
// Along with Vref value, also save Vref range here
_MCODE_SEGMENT_PREFIX static uint32_t vref_opt_rddq[DCS_NUM_CHANNELS * DCS_NUM_RANKS];

// Indicates the optimal Vref value for WRDQ calibration
// Along with Vref value, also save Vref range here
_MCODE_SEGMENT_PREFIX static uint32_t vref_opt_wrdq[DCS_NUM_CHANNELS * DCS_NUM_RANKS];

// Indicates the final center values for CA calibration
_MCODE_SEGMENT_PREFIX static int32_t cntr_opt_ca[DCS_NUM_CHANNELS * DCS_NUM_RANKS * CACAL_MAX_SWLOOPS];

// Indicates the final center values for RDDQ calibration
_MCODE_SEGMENT_PREFIX static int32_t cntr_opt_rddq[DCS_NUM_CHANNELS * DCS_NUM_RANKS];

// Indicates the final center values for WRDQ calibration
_MCODE_SEGMENT_PREFIX static int32_t cntr_opt_wrdq[DCS_NUM_CHANNELS * DCS_NUM_RANKS];

// Indicates the value of CAWRLVL SDLL after Write Leveling is done and before WRDQ calibration
_MCODE_SEGMENT_PREFIX static uint32_t cawrlvl_sdll_wrlvl[DCS_NUM_CHANNELS * DCS_NUM_RANKS]; 

// Indicates the value of WRDQWRLVL SDLL before Write Leveling is done.
_MCODE_SEGMENT_PREFIX static uint32_t dqwrlvl_sdll_before_wrlvl[DCS_NUM_CHANNELS * DCS_NUM_RANKS][DQ_NUM_BYTES] = { { 0 } }; 

// Indicates the value of WRDQWRLVL SDLL after Write Leveling is done and before WRDQ calibration
_MCODE_SEGMENT_PREFIX static uint32_t dqwrlvl_sdll_after_wrlvl[DCS_NUM_CHANNELS * DCS_NUM_RANKS][DQ_NUM_BYTES];

// Indicates the final CA/CS calibrated values in Bucket1
_MCODE_SEGMENT_PREFIX static uint32_t cksdll_bucket1[DCS_NUM_CHANNELS * DCS_NUM_RANKS] = { 0 };
_MCODE_SEGMENT_PREFIX static uint32_t cssdll_bucket1[DCS_NUM_CHANNELS * DCS_NUM_RANKS] = { 0 };
_MCODE_SEGMENT_PREFIX static uint32_t casdll_bucket1[DCS_NUM_CHANNELS * DCS_NUM_RANKS] = { 0 };
_MCODE_SEGMENT_PREFIX static uint32_t cadeskew_bucket1[DCS_NUM_CHANNELS * DCS_NUM_RANKS][CA_NUM_BITS] = {{ 0 }};

// Indicates if there was any panic at a given Vref setting
_MCODE_SEGMENT_PREFIX static int32_t cacal_vref_panic[DCS_NUM_CHANNELS * DCS_NUM_RANKS * CACAL_MAX_SWLOOPS][CA_NUM_VREF] = { { 0 } };
_MCODE_SEGMENT_PREFIX static int32_t rddqcal_vref_panic[DCS_NUM_CHANNELS * DCS_NUM_RANKS][RDDQ_NUM_VREF] = { { 0 } };
_MCODE_SEGMENT_PREFIX static int32_t wrdqcal_vref_panic[DCS_NUM_CHANNELS * DCS_NUM_RANKS][WRDQ_NUM_VREF] = { { 0 } };

#ifdef ENV_DV
// Force initialization of cfg_params (used by DV)
void calibration_init_cfg_params();
#endif

// <rdar://problem/13878186>
void calibrate_ca(void);
void calibrate_rddq(void);
void calibrate_wrlvl(void);
void calibrate_wrdq(void);

// Static local function declarations
static void amp_do_mdll_calib(uint32_t ch);
static void amp_do_imp_calib(uint32_t ch);
static void amp_setup_ca_cal(uint32_t ch, uint32_t rnk, uint32_t swloop);
static void run_soc_upd(uint32_t ch);
static void amp_init_ca_deskew(uint32_t ch);
static void amp_init_ca_cs_ck_sdll(uint32_t ch);
static void amp_init_wr_deskew(uint32_t ch);
static void amp_push_casdll_out(uint32_t ch, uint32_t casdll_ovr_val);
static void amp_push_cksdll_out(uint32_t ch, uint32_t cksdll_ovr_val);
static void amp_push_cssdll_out(uint32_t ch, uint32_t cssdll_ovr_val);
static void amp_program_ca_sdll(uint32_t ch, uint32_t casdll_val);
static void amp_program_ck_sdll(uint32_t ch, uint32_t cksdll_val);
static void amp_program_cs_sdll(uint32_t ch, uint32_t cssdll_val);
static void amp_set_dqs_weak_pd(uint32_t ch, uint32_t freq_bin);
static void amp_reset_dqs_weak_pd(uint32_t ch, uint32_t freq_bin);
static void amp_set_dqs_idle_active(uint32_t ch, uint32_t freq_bin);
static void amp_reset_dqs_idle_active(uint32_t ch, uint32_t freq_bin);
static void amp_enter_cacal_mode(uint32_t ch);
static void amp_exit_cacal_mode(uint32_t ch);
static void amp_run_cacal_vref(uint32_t ch);
static uint32_t amp_run_cacal(uint32_t ch, uint32_t training_mode);
static void amp_setup_rddq_cal(uint32_t ch, uint32_t rnk);
static void amp_rddq_cal_wrfifo(uint32_t ch, uint32_t rnk);
static void amp_set_rddq_sdll(uint32_t ch, uint32_t byte, uint32_t rddqs_sdll_ovr_val);
static void amp_program_rddq_sdll(uint32_t ch, uint32_t byte, uint32_t rddqs_sdll_wr_val);
static void amp_run_rddqcal(uint32_t ch);
static void amp_wrlvl_init(void);
static void amp_wrlvl_entry(uint32_t ch);
static void amp_wrlvl_exit(uint32_t ch, uint8_t freq_bin);
static void amp_set_cawrlvl_sdll(uint32_t ch, uint32_t offset);
static void amp_set_dqwrlvl_sdll(uint32_t ch, uint32_t byte, uint32_t offset);
static void amp_program_cawrlvl_sdll(uint32_t ch, uint32_t cawrlvl_offset, uint32_t cawrlvlmax_offset);
static void amp_program_dqwrlvl_sdll(uint32_t ch, uint32_t byte, uint32_t dqwrlvl_offset, uint32_t dqwrlvlmax_offset);
static void amp_run_wrlvlcal(uint32_t ch, uint32_t wrlvlrun);
static void cacal_init_registers();
static void cacal_generate_patterns_mask(void);
static void cacal_run_sequence(uint32_t ch, uint32_t rnk, uint32_t training_mode, uint32_t mask_bits, uint32_t swloop, uint32_t vref);
static void cacal_find_right_failing_point(uint32_t ch, uint32_t rnk, uint32_t training_mode, uint32_t combined_mask, uint32_t swloop, uint32_t vref);
static void cacal_find_right_passing_point(uint32_t ch, uint32_t rnk, uint32_t training_mode, uint32_t combined_mask, uint32_t swloop, uint32_t vref);
static void cacal_find_left_failing_point(uint32_t ch, uint32_t rnk, uint32_t training_mode, uint32_t combined_mask, uint32_t swloop, uint32_t vref);
static void cacal_find_left_passing_point(uint32_t ch, uint32_t rnk, uint32_t training_mode, uint32_t combined_mask, uint32_t swloop, uint32_t vref);
static void cacal_program_cs_training_values(void);
static void cacal_program_final_values(void);
static void rddqcal_find_right_failing_point(uint32_t ch, uint32_t rnk, uint32_t vref, bool after_wrddqcal);
static void rddqcal_find_right_passing_point(uint32_t ch, uint32_t rnk, uint32_t vref, uint32_t *start_b, bool after_wrddqcal);
static void rddqcal_find_left_failing_point(uint32_t ch, uint32_t rnk, uint32_t vref, bool after_wrddqcal);
static void rddqcal_find_left_passing_point(uint32_t ch, uint32_t rnk, uint32_t vref, uint32_t *start_b, bool after_wrddqcal);
static void rddqcal_program_final_values(void);
static void wrlvlcal_push_to_0s_region(uint32_t ch, uint32_t rnk);
static void wrlvlcal_find_0to1_transition(uint32_t ch, uint32_t rnk);
static void wrlvlcal_find_1to0_transition(uint32_t ch, uint32_t rnk);
static void wrlvlcal_program_final_values(void);
static void amp_compute_opt_vref_cacal(uint32_t ch, uint32_t rnk, uint32_t swloop, uint32_t bitMax);
static void amp_compute_opt_vref_rddqcal(uint32_t ch, uint32_t rnk, uint32_t bitMax);
static uint32_t rddqcal_encode_dlyval(uint32_t ch, uint32_t phy_type, uint32_t val);
static void amp_program_ca_vref(uint32_t ch, uint32_t vref,  uint32_t cacalib_dqs_pulse_cnt, uint32_t cacalib_dqs_pulse_width);
static void amp_program_rddq_vref(uint32_t ch, uint32_t vref, uint32_t vref_pulse_cnt, uint8_t freq_bin);
static void amp_program_wrdq_vref(uint8_t chan, uint8_t rank, uint32_t vref, uint32_t vref_pulse_cnt);
static void wrdqcal_init_regs(uint8_t chan, uint8_t rank);
static int wrdqcal_set_half_clk_delay(uint32_t byte, uint32_t val);
static int wrdqcal_set_delay_point(uint32_t chan, uint32_t byte, int32_t point, bool result);
static int wrdqcal_set_delay_point_ovr(uint32_t chan, uint32_t byte, int32_t point);
static int wrdqcal_set_delay_point_res(uint32_t chan, uint32_t byte, int32_t point);
static int wrdqcal_program_dq_deskew(uint32_t chan, uint32_t byte, uint32_t val);
static int wrdqcal_set_search_point(uint32_t byte, int point);
static uint32_t wrdqcal_assess_position();
static uint8_t wrdqcal_solid_pass(uint32_t byte, int32_t point, int step, uint8_t max);
static uint8_t wrdqcal_all_bits_fail(uint32_t byte, int32_t point, int step, uint8_t max);
static void wrdqcal_find_perbit_skew(uint32_t byte, int edge, int step, int limit, int16_t *bit_edge);
static void wrdqcal_sequence(uint8_t chan, uint8_t rank);
static void amp_compute_opt_vref_wrdqcal(uint32_t ch, uint32_t rnk, uint32_t bitMax);
static void wrdqcal_program_final_values(void);
static void amp_save_ca_bucket1(uint32_t ch);
static void amp_program_ca_bucket1(uint32_t ch);

// Helper functions
static int32_t find_center_of_eye(int32_t left_pos_val, int32_t right_pos_val);
static int32_t find_wrdq_center_of_eye(int32_t left_pos_val, int32_t right_pos_val);
static int32_t find_common_endpoint(int32_t val0, int32_t val1, uint32_t left_or_right);
static void cal_save_value(uint8_t data, uint32_t num_bits, uint32_t *bit_pos, uint32_t *byte_pos);
static uint8_t cal_retrieve_value(uint32_t num_bits, uint32_t *bit_pos, uint32_t *byte_pos);
void amp_save_masterdll_values();

///////////////////////////////////////////////////////////////////////////////
/* Global configuration parameters for calibration, setup is different for iBoot, DV, SiVal, PE */
///////////////////////////////////////////////////////////////////////////////
_MCODE_SEGMENT_PREFIX static struct amp_calibration_params cfg_params;

///////////////////////////////////////////////////////////////////////////////
////// Global functions
///////////////////////////////////////////////////////////////////////////////
_ROMCODE_SEGMENT_PREFIX
struct amp_calibration_params *calibration_cfg_params_init(
	int bootMode, int numChans, int numRanksPerChn)
{
	uint32_t i;

	// Set the calibration params.

	// chip_id and chip_rev are by default set to ELBA A0 for SEG-DV, since they don't perform run time check
	// iBoot will override with correct values at run time
	cfg_params.chip_id = 0x8001;
	cfg_params.chip_rev = CHIP_REVISION_B0;

	cfg_params.num_channels = numChans;
	cfg_params.num_ranks = numRanksPerChn;
#if ENV_DV
	// Run one channel calibration in DV environment. <rdar://problem/15729353>
	cfg_params.num_channels_runCalib = 1;			// Only Calibrate one Channel
#else
	cfg_params.num_channels_runCalib = numChans;		// Calibrate All Channels
#endif

	// Elba Defaults
	cfg_params.clk_period_bin0 = CLK_PERIOD_1600;
	cfg_params.clk_period_bin1 = CLK_PERIOD_800;
	
	// <rdar://problem/15729353>
	cfg_params.ca_num_vref = CA_NUM_VREF;
	cfg_params.rddq_num_vref = RDDQ_NUM_VREF;
	cfg_params.wrdq_num_vref = WRDQ_NUM_VREF;
	
	cfg_params.wrdq_apply_NC_algorithm = true;

	for(i=0;i< cfg_params.ca_num_vref;i++) {
		cfg_params.ca_vref_values[i] = 0xC + i;
	}
	
	for(i=0;i< cfg_params.rddq_num_vref;i++) {
		cfg_params.rddq_vref_values[i] = 0xBE + i;
	}
	
	for(i=0;i< cfg_params.wrdq_num_vref;i++) {
		cfg_params.wrdq_vref_values[i] = 0xC + i;
	}
	
	// This is to take care of bit/byte swizzling
#ifdef AMS_SIM_ENV
	for(i=0;i<8;i++) {
		cfg_params.cacal_bit_sel[i] = ((CSR_READ(rAMP_CACALBITSELECT(0)) >> (4 * i)) & 0x7);
	}
#else
	for(i=0;i<8;i++) {
		cfg_params.cacal_bit_sel[i] = i;
	}
#endif
	
#ifdef AMS_SIM_ENV
	cfg_params.cacal_rd_byte_sel = ((CSR_READ(rAMP_CACALCTRL(0)) >> 28) & 0x1);
#else
	cfg_params.cacal_rd_byte_sel = 1;
#endif
	
	cfg_params.resume = (bootMode)? true : false;
	cfg_params.disable_coarse_cs_training = true;
	cfg_params.cacalib_hw_loops = 15;
	cfg_params.cacalib_sw_loops = 1;
	cfg_params.cacalib_dqs_pulse_cnt = 0;
	cfg_params.cacalib_dqs_pulse_width = 2;
	cfg_params.rddq_cal_loopcnt = 1;
	cfg_params.wrdq_cal_loopcnt = 1;
	
	cfg_params.rddq_legacy_mode = false;
	cfg_params.wrdq_legacy_mode = false;
	
	cfg_params.freq_bin = 0;
	cfg_params.RL = 28; // This is Bin 0 value, with DBI_RD disabled.
	cfg_params.WL = 14; // This is Bin 0 value, with WL Set A.
	
	cfg_params.dv_params_valid = false;
	cfg_params.dv_randomize = false;
	cfg_params.dbg_calib_mode = 0;
	cfg_params.opt_vref_mode = DCS_CAL_OPT_VREF_MODE_COM;
	
	// Giving a margin of 2 clocks from JEDEC spec to account for propagation delays from AMPS to DDR pins.
	// cfg_params.tWLO = 15; // max = 20ns
	cfg_params.tWLO = 15; // max = 20ns
	cfg_params.tWLMRD = 21; // min = 40 tCK
	cfg_params.tWLDQSEN = 11; // max = 20 tCK	
	
	cfg_params.tDSTrain = 2;
	cfg_params.tCKCKEH = 2;
	cfg_params.tVREF_LONG = 160;
	cfg_params.tCAENT	= 9;

	cfg_params.tADR = 20; // max = 20ns + propagation delay from AMPS-DRAM + receiver delays.
	cfg_params.tMRD = 13; // min = 14ns
	cfg_params.tDQSCKE = 9; // min = 10ns
	cfg_params.tCKEHCMD = 7;
	
	cfg_params.WrLvlDqsPulseCnt = 0; // This sets the number of pulses to 2.
	
	cfg_params.prbs7_pattern_0 = 0x70F20C28;
	cfg_params.prbs7_pattern_1 = 0x634BB995;
	cfg_params.prbs7_pattern_2 = 0x00006D6F;
	cfg_params.prbs7_pattern_3 = 0x00000000;
	cfg_params.ca_invert_mask = 0x65;
	cfg_params.CsStopCycleCnt = 1;
	cfg_params.CsActiveCycleCnt = 2;

	for(int i = 0; i < CA_NUM_BITS; i++) {
		cfg_params.ca_prbs_seed_bit[i] = i;
	}

	for(int i = 0; i < DQ_NUM_BYTES; i++) {
		for(int j = 0; j < DQ_NUM_BITS_PER_BYTE + DMI_NUM_BITS_PER_BYTE; j++) {
			cfg_params.wrdq_prbs_seed[i][j] = (i * DQ_NUM_BITS_PER_BYTE + j);
		}
	}
	
	// Returning this allows Clients of this Calibration Library to
	// modify the configuration beyond this default setup
	return &cfg_params;
}

///////////////////////////////////////////////////////////////////////////////
////// Standard way to calculate the correct index for results arrays
////// Based on Chan, Rank, SW-Loop ... degenerates appropriately when
////// one of the the configured "number of" parameters is ONE, or when
////// one of the passed parameters is ZERO.
///////////////////////////////////////////////////////////////////////////////
static inline
uint32_t loopchrnk_index_calc(uint32_t ch, uint32_t rnk, uint32_t swloop)
{
	return (rnk + (cfg_params.num_ranks * (ch + (cfg_params.num_channels * swloop))));
}

///////////////////////////////////////////////////////////////////////////////
_ROMCODE_SEGMENT_PREFIX void cal_pmgr_change_mcu_clk(uint32_t bucket)
{
#ifdef ENV_DV
uint32_t pmgr_src_sel;

	#ifdef AMS_SIM_ENV
		// Call custom API provided in PMGR_Xactor.h for PMGR changing mcu_clk to indicated Freq Bucket
		pmgr_special_cmd(PMGR_CMD_FREQ_CHANGE, bucket, 0);
	#else
		// Call custom API provided in mcu_helper_fxns.c for PMGR changing mcu_clk to 1600Mhz, 800Mhz or 50Mhz.
		pmgr_src_sel = (bucket == 1) ? 5 : (bucket == 0) ? 5 : 8;  				
		pmgr_amc_clk_cfg(pmgr_src_sel, bucket, 0);
	#endif
#else
	// Call shim function to ask the PMGR to change the mcu_clk to indicated Freq Bucket
	shim_change_freq(bucket);
#endif
}

///////////////////////////////////////////////////////////////////////////////
_ROMCODE_SEGMENT_PREFIX void amp_save_masterdll_values()
{
	uint8_t ch;

	for(ch = 0; ch < cfg_params.num_channels ; ch++) {
		mdllcode[ch][AMP_DQ0] = (CSR_READ(rAMP_MDLLCODE(AMP_DQ0, ch)) & DLLVAL_BITS);
		mdllcode[ch][AMP_DQ1] = (CSR_READ(rAMP_MDLLCODE(AMP_DQ1, ch)) & DLLVAL_BITS);
		// Since there is no CA MDLL for Maui, pick the maximum of DQ0 MDLL & DQ1 MDLL
		mdllcode[ch][AMP_CA] = (mdllcode[ch][AMP_DQ0] > mdllcode[ch][AMP_DQ1]) ? mdllcode[ch][AMP_DQ0] : mdllcode[ch][AMP_DQ1];
	}
}

_ROMCODE_SEGMENT_PREFIX void amp_save_ca_bucket1(uint32_t ch)
{
	uint8_t bit;

	cksdll_bucket1[ch] = (CSR_READ(rAMP_CKSDLLCTRL(ch)) >> SDLLOFFSET) & 0xFF;
	cssdll_bucket1[ch] = (CSR_READ(rAMP_CSSDLLCTRL(ch)) >> SDLLOFFSET) & 0xFF;
	casdll_bucket1[ch] = (CSR_READ(rAMP_CASDLLCTRL(ch)) >> SDLLOFFSET) & 0xFF;
	shim_printf("DEBUG - amp_save_ca_bucket1:: Saving %d for CKSDLL, %d for CSSDLL, %d for CASDLL for Channel %d\n", cksdll_bucket1[ch], cssdll_bucket1[ch], casdll_bucket1[ch], ch);

	for(bit = 0; bit < CA_NUM_BITS; bit++) {
		cadeskew_bucket1[ch][bit] = CSR_READ(rAMP_CADESKEW_CTRL(ch,bit)) & 0x3F; 
		shim_printf("DEBUG - amp_save_ca_bucket1:: Saving %d for CA deskew bit %d for Channel %d \n", cadeskew_bucket1[ch][bit], bit, ch);
	}
}

_ROMCODE_SEGMENT_PREFIX void amp_program_ca_bucket1(uint32_t ch)
{
	uint8_t bit;

	amp_push_cksdll_out(ch, cksdll_bucket1[ch]);
	amp_push_cssdll_out(ch, cssdll_bucket1[ch]);
	amp_push_casdll_out(ch, casdll_bucket1[ch]);

	for(bit = 0; bit < CA_NUM_BITS; bit++) {
		CSR_WRITE(rAMP_CADESKEW_CTRL(ch, bit), cadeskew_bucket1[ch][bit] | RUNDESKEWUPD);

		SPIN_W_TMO_WHILE (CSR_READ(rAMP_CADESKEW_CTRL(ch, bit)) & RUNDESKEWUPD);
	}
}

// Save or restore ca and wrlvl registers for resume boot
_ROMCODE_SEGMENT_PREFIX void calibration_save_restore_ca_wrlvl_regs(uint32_t save_or_restore, uint32_t channels, bool freq_bin)
{

	_MCODE_SEGMENT_PREFIX static uint32_t bit_pos = 0;
	_MCODE_SEGMENT_PREFIX static uint32_t byte_pos = 0;
	uint32_t ch, bit_indx, byte;
	uint8_t casdll[DCS_NUM_CHANNELS], cksdll[DCS_NUM_CHANNELS], cssdll[DCS_NUM_CHANNELS], ca_deskew[DCS_NUM_CHANNELS][CA_NUM_BITS];
	//uint8_t ca_vref_f0, ca_vref_f1;
	//uint8_t rddq_vref_f0, rddq_vref_f1;
	//uint8_t wrdq_vref_f0, wrdq_vref_f1;
	//uint8_t wrlvlsdll[DCS_NUM_CHANNELS];
	uint32_t camdll_vtscl_refcntl, rdmdll_vtscl_refcntl, wrmdll_vtscl_refcntl;
	uint16_t camdll_vtscl_ref_ovrcode, rdmdll_vtscl_ref_ovrcode, wrmdll_vtscl_ref_ovrcode;
	uint8_t ca_vref_f1, wrdq_vref_f1, rddq_vref_f1;	
	uint8_t ca_vref_f0, wrdq_vref_f0, rddq_vref_f0;	
	uint32_t wrdqcalvrefcodecontrol;
	uint32_t freqchngctl1;
	
	if (save_or_restore == CALIB_SAVE) {
		if(freq_bin) {
	
			 // Save only write leveling registers when the save function is called first.
			 // Save function needs to be called after write leveling because the register fields
			 // being saved here would be over-written after WRDQ calibration is done.
			if((bit_pos == 0) && (byte_pos == 0)) {	
				for (ch = 0; ch < channels; ch++) {
					// save the WrLvl registers for this channel (2 DQ SDLLs and 1 CA SDLL)
					for (byte = 0; byte < (DQ_NUM_BYTES + 1); byte++) {
			
						// cawrlvlsdll is stored as the "3rd" byte
						if (byte == DQ_NUM_BYTES)
							cal_save_value(CSR_READ(rAMP_CAWRLVLSDLLCODE(ch)) & DLLVAL_BITS, SDLL_NUM_BITS, &bit_pos, &byte_pos);
						else
							cal_save_value(CSR_READ(rAMP_DQWRLVLSDLLCODE(byte, ch)) & DLLVAL_BITS, SDLL_NUM_BITS, &bit_pos, &byte_pos);
					}
			 	} 
			}
			else {
				for (ch = 0; ch < channels; ch++) {

					// save the CA registers for this channel
					// Save CA SDLL
					cal_save_value(CSR_READ(rAMP_CASDLLCODE(ch)) & DLLVAL_BITS, SDLL_NUM_BITS, &bit_pos, &byte_pos);
			
					// Save CK SDLL 
					cal_save_value(CSR_READ(rAMP_CKSDLLCODE(ch)) & DLLVAL_BITS, SDLL_NUM_BITS, &bit_pos, &byte_pos);

					// Save CS SDLL
					cal_save_value(CSR_READ(rAMP_CSSDLLCODE(ch)) & DLLVAL_BITS, SDLL_NUM_BITS, &bit_pos, &byte_pos);
			
					// Save CA deskews
					for (bit_indx = 0; bit_indx < CA_NUM_BITS; bit_indx++)
						cal_save_value(CSR_READ(rAMP_CADESKEW_CTRL(ch, bit_indx)) & DESKEW_CTRL_BITS, DESKEW_NUM_BITS, &bit_pos, &byte_pos);

				
					// Save CA, WR and RD VT reference codes
					for (byte = 0; byte < DQ_NUM_BYTES ; byte++) {
						cal_save_value((CSR_READ(rAMP_CAMDLL_VTSCL_REFCNTL(byte, ch)) & VT_SCL_REF_MASK) & 0xFF, 8, &bit_pos, &byte_pos); 					 
						cal_save_value((((CSR_READ(rAMP_CAMDLL_VTSCL_REFCNTL(byte, ch)) & VT_SCL_REF_MASK) >> 8) & 0x3), VT_SCL_REF_BITS - 8, &bit_pos, &byte_pos); 					 
						cal_save_value((CSR_READ(rAMP_RDMDLL_VTSCL_REFCNTL(byte, ch)) & VT_SCL_REF_MASK) & 0xFF, 8, &bit_pos, &byte_pos); 					 
						cal_save_value((((CSR_READ(rAMP_RDMDLL_VTSCL_REFCNTL(byte, ch)) & VT_SCL_REF_MASK) >> 8) & 0x3), VT_SCL_REF_BITS - 8, &bit_pos, &byte_pos); 					 
						cal_save_value((CSR_READ(rAMP_WRMDLL_VTSCL_REFCNTL(byte, ch)) & VT_SCL_REF_MASK) & 0xFF, 8, &bit_pos, &byte_pos); 					 
						cal_save_value((((CSR_READ(rAMP_WRMDLL_VTSCL_REFCNTL(byte, ch)) & VT_SCL_REF_MASK) >> 8) & 0x3), VT_SCL_REF_BITS - 8, &bit_pos, &byte_pos); 					 
					}

					// Save optimal vref for CA/CS
					cal_save_value(cfg_params.ca_vref_values[vref_opt_ca[ch]] & CA_WRDQ_VREF_CTRL_BITS, CA_WRDQ_VREF_NUM_BITS, &bit_pos, &byte_pos);	
				
					// Save optimal Vref for RDDQ
					cal_save_value(cfg_params.rddq_vref_values[vref_opt_rddq[ch]] & RDDQ_VREF_CTRL_BITS, RDDQ_VREF_NUM_BITS, &bit_pos, &byte_pos);
		
					// Save optimal Vref for WRDQ
					cal_save_value(cfg_params.wrdq_vref_values[vref_opt_wrdq[ch]] & CA_WRDQ_VREF_CTRL_BITS, CA_WRDQ_VREF_NUM_BITS, &bit_pos, &byte_pos);
				}

				for (ch = 0; ch < channels; ch++) {
					// Save WR DQS SDLL
					for (byte = 0; byte < DQ_NUM_BYTES ; byte++) {
						cal_save_value((CSR_READ(rAMP_WRDQSDQ_SDLLCODE(byte, ch)) >> WR_DQS_SDLL_CODE_OFFSET) & DLLVAL_BITS, SDLL_NUM_BITS, &bit_pos, &byte_pos);
					}
	
					// Save WR DQ SDLL
					for (byte = 0; byte < DQ_NUM_BYTES ; byte++) {
						cal_save_value(CSR_READ(rAMP_WRDQSDQ_SDLLCODE(byte, ch)) & DLLVAL_BITS, SDLL_NUM_BITS, &bit_pos, &byte_pos);
					}
	
					// Save WR DQS deskew
					for (byte = 0; byte < DQ_NUM_BYTES ; byte++) {
						cal_save_value(CSR_READ(rAMP_WRDQSDESKEW_CTRL(byte, ch)) & DESKEW_CTRL_BITS, DESKEW_NUM_BITS, &bit_pos, &byte_pos);
					}

					// Save WR DQ deskew
					for (byte = 0; byte < DQ_NUM_BYTES ; byte++) {
						for (bit_indx = 0; bit_indx < DQ_NUM_BITS_PER_BYTE; bit_indx++) {
							cal_save_value(CSR_READ(rAMP_WRDQDESKEW_CTRL(byte, ch, bit_indx)) & DESKEW_CTRL_BITS, DESKEW_NUM_BITS, &bit_pos, &byte_pos);	
						}
					}

					// Save WR DM deskew
					for (byte = 0; byte < DQ_NUM_BYTES ; byte++) {
						cal_save_value(CSR_READ(rAMP_WRDMDESKEW_CTRL(byte, ch)) & DESKEW_CTRL_BITS, DESKEW_NUM_BITS, &bit_pos, &byte_pos);
					}
				
					// Save RD SDLL
					for (byte = 0; byte < DQ_NUM_BYTES ; byte++) {
						cal_save_value(CSR_READ(rAMP_DQSDLLCODE_RD(byte, ch)) & DLLVAL_BITS, SDLL_NUM_BITS, &bit_pos, &byte_pos);
					}
	
					// Save RD DQ deskew codes
					for (byte = 0; byte < DQ_NUM_BYTES ; byte++) {
						for (bit_indx = 0; bit_indx < DQ_NUM_BITS_PER_BYTE; bit_indx++) {
							cal_save_value(CSR_READ(rAMP_RDDQDESKEW_CTRL(byte, ch, bit_indx)) & DESKEW_CTRL_BITS, DESKEW_NUM_BITS, &bit_pos, &byte_pos);
						}
					}
					
					// Save RD DMI deskew
					for (byte = 0; byte < DQ_NUM_BYTES ; byte++) {
						cal_save_value(CSR_READ(rAMP_RDDMDESKEW_CTRL(byte, ch)) & DESKEW_CTRL_BITS, DESKEW_NUM_BITS, &bit_pos, &byte_pos);
					}
				
					// Save CAWRLVL & DQ WRLVL SDLL values after WRDQ calibration is done
					for (byte = 0; byte < (DQ_NUM_BYTES + 1); byte++) {
			
						// cawrlvlsdll is stored as the "5th" byte
						if (byte == DQ_NUM_BYTES)
							cal_save_value(CSR_READ(rAMP_CAWRLVLSDLLCODE(ch)) & DLLVAL_BITS, SDLL_NUM_BITS, &bit_pos, &byte_pos);
						else
							cal_save_value(CSR_READ(rAMP_DQWRLVLSDLLCODE(byte, ch)) & DLLVAL_BITS, SDLL_NUM_BITS, &bit_pos, &byte_pos);
					}
			 	}
			 }
			}
			else {
					
				for (ch = 0; ch < channels; ch++) {
					// Save optimal vref for CA/CS
					cal_save_value(cfg_params.ca_vref_values[vref_opt_ca[ch]] & CA_WRDQ_VREF_CTRL_BITS, CA_WRDQ_VREF_NUM_BITS, &bit_pos, &byte_pos);	
				
					// Save optimal Vref for RDDQ
					cal_save_value(cfg_params.rddq_vref_values[vref_opt_rddq[ch]] & RDDQ_VREF_CTRL_BITS, RDDQ_VREF_NUM_BITS, &bit_pos, &byte_pos);
		
					// Save optimal Vref for WRDQ
					cal_save_value(cfg_params.wrdq_vref_values[vref_opt_wrdq[ch]] & CA_WRDQ_VREF_CTRL_BITS, CA_WRDQ_VREF_NUM_BITS, &bit_pos, &byte_pos);
				}

				// Save the cal_bits array
				shim_store_memory_calibration((void *) cal_bits, CALIB_NUM_BYTES_TO_SAVE);
			}
	} else {
		// Retrieve cal_bits array
		shim_load_memory_calibration((void *) cal_bits, CALIB_NUM_BYTES_TO_SAVE);

		// Reset bit & byte positions for the first call 
			bit_pos = 0;
			byte_pos = 0;
		
			for (ch = 0; ch < channels; ch++) {
				for (byte = 0; byte < (DQ_NUM_BYTES + 1); byte++) {
					// cawrlvlsdll is stored as the "3rd" byte
					if (byte == DQ_NUM_BYTES)
						amp_program_cawrlvl_sdll(ch, cal_retrieve_value(SDLL_NUM_BITS, &bit_pos, &byte_pos), 0);
					else
						amp_program_dqwrlvl_sdll(ch, byte, cal_retrieve_value(SDLL_NUM_BITS, &bit_pos, &byte_pos), 0);	
				}
			}

			for (ch = 0; ch < channels; ch++) {
				casdll[ch] = cal_retrieve_value(SDLL_NUM_BITS, &bit_pos, &byte_pos);
				amp_program_ca_sdll(ch, casdll[ch]);

				cksdll[ch] = cal_retrieve_value(SDLL_NUM_BITS, &bit_pos, &byte_pos);
				amp_program_ck_sdll(ch, cksdll[ch]);

				cssdll[ch] = cal_retrieve_value(SDLL_NUM_BITS, &bit_pos, &byte_pos);
				amp_program_cs_sdll(ch, cssdll[ch]);
			
				for (bit_indx = 0; bit_indx < CA_NUM_BITS; bit_indx++) {
					ca_deskew[ch][bit_indx] = cal_retrieve_value(DESKEW_NUM_BITS, &bit_pos, &byte_pos);
					CSR_WRITE(rAMP_CADESKEW_CTRL(ch, bit_indx), ca_deskew[ch][bit_indx] | RUNDESKEWUPD);
					SPIN_W_TMO_WHILE ( CSR_READ(rAMP_CADESKEW_CTRL(ch, bit_indx)) & RUNDESKEWUPD);
				}

				for (byte = 0; byte < DQ_NUM_BYTES ; byte++) {
					camdll_vtscl_refcntl = CSR_READ(rAMP_CAMDLL_VTSCL_REFCNTL(byte, ch));
					camdll_vtscl_ref_ovrcode = cal_retrieve_value(8, &bit_pos, &byte_pos);
					camdll_vtscl_ref_ovrcode |= (cal_retrieve_value(VT_SCL_REF_BITS - 8, &bit_pos, &byte_pos) << 8);
			   	CSR_WRITE(rAMP_CAMDLL_VTSCL_REFCNTL(byte, ch), (camdll_vtscl_refcntl & VT_SCL_REF_MASK) | (camdll_vtscl_ref_ovrcode << VT_SCL_OVR_OFFSET) | (1 << VT_SCL_REF_UPD_OFFSET));

					rdmdll_vtscl_refcntl = CSR_READ(rAMP_RDMDLL_VTSCL_REFCNTL(byte, ch));
					rdmdll_vtscl_ref_ovrcode = cal_retrieve_value(8, &bit_pos, &byte_pos);
					rdmdll_vtscl_ref_ovrcode |= (cal_retrieve_value(VT_SCL_REF_BITS - 8, &bit_pos, &byte_pos) << 8);
			   	CSR_WRITE(rAMP_RDMDLL_VTSCL_REFCNTL(byte, ch), (rdmdll_vtscl_refcntl & VT_SCL_REF_MASK) | (rdmdll_vtscl_ref_ovrcode << VT_SCL_OVR_OFFSET) | (1 << VT_SCL_REF_UPD_OFFSET));

					wrmdll_vtscl_refcntl = CSR_READ(rAMP_WRMDLL_VTSCL_REFCNTL(byte, ch));
					wrmdll_vtscl_ref_ovrcode = cal_retrieve_value(8, &bit_pos, &byte_pos);
					wrmdll_vtscl_ref_ovrcode |= (cal_retrieve_value(VT_SCL_REF_BITS - 8, &bit_pos, &byte_pos) << 8);
			   	CSR_WRITE(rAMP_WRMDLL_VTSCL_REFCNTL(byte, ch), (wrmdll_vtscl_refcntl & VT_SCL_REF_MASK) | (wrmdll_vtscl_ref_ovrcode << VT_SCL_OVR_OFFSET) | (1 << VT_SCL_REF_UPD_OFFSET));
				}

				ca_vref_f1 = cal_retrieve_value(CA_WRDQ_VREF_NUM_BITS, &bit_pos, &byte_pos);
				rddq_vref_f1 = cal_retrieve_value(RDDQ_VREF_NUM_BITS, &bit_pos, &byte_pos);
				wrdq_vref_f1 = cal_retrieve_value(CA_WRDQ_VREF_NUM_BITS, &bit_pos, &byte_pos);

				// Program CA and WRDQ optimal Vref's
				wrdqcalvrefcodecontrol = CSR_READ(rAMP_WRDQCALVREFCODESTATUS(ch));
				CSR_WRITE(rAMP_WRDQCALVREFCODECONTROL(ch), (wrdqcalvrefcodecontrol & WRDQ_VREF_F1_MASK) | (wrdq_vref_f1 << WRDQ_VREF_F1_OFFSET));

				freqchngctl1 = CSR_READ(rAMCX_DRAMCFG_FREQCHNGCTL1_FREQ1(ch));
				CSR_WRITE(rAMCX_DRAMCFG_FREQCHNGCTL1_FREQ1(ch), (freqchngctl1	& 0x00C0FFFF) | (0xC << 16) | (ca_vref_f1 << 24));
	
				// Program AMPS register with optimal RDDQ Vref
				amp_program_rddq_vref(ch, rddq_vref_f1, 0, 1);
			}

			// Simply retrieve WRDQ /RDDQ delays as they are not planned to be used. These delays would be recomputed by Full scan
			// hardware calibration.
				for (ch = 0; ch < channels; ch++) {
					// Save WR DQS SDLL
					for (byte = 0; byte < DQ_NUM_BYTES ; byte++) {
						cal_retrieve_value(SDLL_NUM_BITS, &bit_pos, &byte_pos);
					}
	
					// Save WR DQ SDLL
					for (byte = 0; byte < DQ_NUM_BYTES ; byte++) {
						cal_retrieve_value(SDLL_NUM_BITS, &bit_pos, &byte_pos);
					}
	
					// Save WR DQS deskew
					for (byte = 0; byte < DQ_NUM_BYTES ; byte++) {
						cal_retrieve_value(DESKEW_NUM_BITS, &bit_pos, &byte_pos);
					}

					// Save WR DQ deskew
					for (byte = 0; byte < DQ_NUM_BYTES ; byte++) {
						for (bit_indx = 0; bit_indx < DQ_NUM_BITS_PER_BYTE; bit_indx++) {
							cal_retrieve_value(DESKEW_NUM_BITS, &bit_pos, &byte_pos);	
						}
					}

					// Save WR DM deskew
					for (byte = 0; byte < DQ_NUM_BYTES ; byte++) {
						cal_retrieve_value(DESKEW_NUM_BITS, &bit_pos, &byte_pos);
					}
				
					// Save RD SDLL
					for (byte = 0; byte < DQ_NUM_BYTES ; byte++) {
						cal_retrieve_value(SDLL_NUM_BITS, &bit_pos, &byte_pos);
					}
	
					// Save RD DQ deskew codes
					for (byte = 0; byte < DQ_NUM_BYTES ; byte++) {
						for (bit_indx = 0; bit_indx < DQ_NUM_BITS_PER_BYTE; bit_indx++) {
							cal_retrieve_value(DESKEW_NUM_BITS, &bit_pos, &byte_pos);
						}
					}
					
					// Save RD DMI deskew
					for (byte = 0; byte < DQ_NUM_BYTES ; byte++) {
						cal_retrieve_value(DESKEW_NUM_BITS, &bit_pos, &byte_pos);
					}
				
					// Save CAWRLVL & DQ WRLVL SDLL values after WRDQ calibration is done
					for (byte = 0; byte < (DQ_NUM_BYTES + 1); byte++) {
						// cawrlvlsdll is stored as the "5th" byte
						if (byte == DQ_NUM_BYTES)
							cal_retrieve_value(SDLL_NUM_BITS, &bit_pos, &byte_pos);
						else
							cal_retrieve_value(SDLL_NUM_BITS, &bit_pos, &byte_pos);
					}
			 	}
				
				// Retrieve optimal Vref values for Bucket 0.
				for (ch = 0; ch < channels; ch++) {
					ca_vref_f0 = cal_retrieve_value(CA_WRDQ_VREF_NUM_BITS, &bit_pos, &byte_pos);
					rddq_vref_f0 = cal_retrieve_value(RDDQ_VREF_NUM_BITS, &bit_pos, &byte_pos);
					wrdq_vref_f0 = cal_retrieve_value(CA_WRDQ_VREF_NUM_BITS, &bit_pos, &byte_pos);
					
					// Program CA and WRDQ optimal Vref's
					wrdqcalvrefcodecontrol = CSR_READ(rAMP_WRDQCALVREFCODESTATUS(ch));
					CSR_WRITE(rAMP_WRDQCALVREFCODECONTROL(ch), (wrdqcalvrefcodecontrol & WRDQ_VREF_F0_MASK) | (wrdq_vref_f0 << WRDQ_VREF_F0_OFFSET));


					freqchngctl1 = CSR_READ(rAMCX_DRAMCFG_FREQCHNGCTL1_FREQ0(ch));
					CSR_WRITE(rAMCX_DRAMCFG_FREQCHNGCTL1_FREQ0(ch), (freqchngctl1	& 0x00C0FFFF) | (0xC << 16) | (ca_vref_f0 << 24));
	
					// Program AMPS register with optimal RDDQ Vref
					amp_program_rddq_vref(ch, rddq_vref_f0, 0, 0);
				}
		}
}

_ROMCODE_SEGMENT_PREFIX void calibration_dump_results(uint32_t operations, bool dump_passing_window)
{
  uint32_t ch, rnk, chrnk_indx, bit, byte, tmp;

	if ((operations & (SELECT_CAL_ALL)) == 0)
		// Nothing selected
		return;
	
	shim_printf("LPDDR4 DCS Calibration final results");
	
	if (dump_passing_window)
		shim_printf(" with passing points per rank in brackets");
	
	shim_printf(":\n");
	
	for (ch = 0; ch < cfg_params.num_channels; ch++) {
		shim_printf("DCS Channel %d\n", ch);

		if (operations & SELECT_CACAL) {
			shim_printf("\tCA SDLL: %d\n", CSR_READ(rAMP_CASDLLCODE(ch)));
			shim_printf("\tCS SDLL: %d\n", CSR_READ(rAMP_CSSDLLCODE(ch)));
			shim_printf("\tCK SDLL: %d\n", CSR_READ(rAMP_CKSDLLCODE(ch)));

			shim_printf("\t\tPer Bit Deskew: ");
			for (bit = 0; bit < CA_NUM_BITS; bit++) {
				shim_printf("%d ", CSR_READ(rAMP_CADESKEW_CTRL(ch, bit)) & DESKEW_CTRL_BITS);

				if (dump_passing_window && !cfg_params.resume) {

					shim_printf("[");

					for (rnk = 0; rnk < cfg_params.num_ranks; rnk++) {
						chrnk_indx = loopchrnk_index_calc(ch, rnk, 0);
						if (rnk > 0)
							shim_printf(", ");



						shim_printf("%d ", cacal_per_chrnk_right[chrnk_indx][bit]);
						shim_printf("%d", cacal_per_chrnk_left[chrnk_indx][bit]);
					}

					shim_printf("] ");
				}

			}
			shim_printf("\n\t\tCS Deskew: %d", CSR_READ(rAMP_CSDESKEW_CTRL(ch)) & DESKEW_CTRL_BITS);
			shim_printf("\n\t\tCK Deskew: %d", CSR_READ(rAMP_CKDESKEW_CTRL(ch)) & DESKEW_CTRL_BITS);
			shim_printf("\n");
		}
		
		if (operations & SELECT_WRLVLCAL) {

			shim_printf("\tCA WrLvlSDLL: %d", CSR_READ(rAMP_CAWRLVLSDLLCODE(ch)) & DLLVAL_BITS);
		
			if (dump_passing_window && !cfg_params.resume) {
				shim_printf(" [");

				for (rnk = 0; rnk < cfg_params.num_ranks; rnk++) {
					chrnk_indx = loopchrnk_index_calc(ch, rnk, 0);
					if (rnk > 0)
						shim_printf(", ");



					shim_printf("%d ", wrlvlcal_per_chrnk_rise[chrnk_indx][DQ_NUM_BYTES]);
					shim_printf("%d", wrlvlcal_per_chrnk_fall[chrnk_indx][DQ_NUM_BYTES]);
				}

				shim_printf("]");
			}

			shim_printf("\n");
		
			shim_printf("\tDQ WrLvlSDLL: ");
			for (byte = 0; byte < DQ_NUM_BYTES; byte++) {

				shim_printf("%d ", CSR_READ(rAMP_DQWRLVLSDLLCODE(byte, ch)) & DLLVAL_BITS);
			
				if (dump_passing_window && !cfg_params.resume) {
					shim_printf("[");

					for (rnk = 0; rnk < cfg_params.num_ranks; rnk++) {
						chrnk_indx = loopchrnk_index_calc(ch, rnk, 0);
						if (rnk > 0)
							shim_printf(", ");



						shim_printf("%d ", wrlvlcal_per_chrnk_rise[chrnk_indx][byte]);
						shim_printf("%d", wrlvlcal_per_chrnk_fall[chrnk_indx][byte]);
					}

					shim_printf("] ");
				}
			}

			shim_printf("\n");
		}
		
		if (operations & SELECT_RDDQCAL) {

			shim_printf("\tRead DQ:\n");
			for (byte = 0; byte < DQ_NUM_BYTES; byte++) {

				shim_printf("\t\tByte %d SDLL: %d\n", byte, CSR_READ(rAMP_DQSDLLCODE_RD(byte, ch)) & DLLVAL_BITS);
			
				shim_printf("\t\t\tPer Bit Deskew: ");
				for (bit = 0; bit < DQ_NUM_BITS_PER_BYTE; bit++) {
					shim_printf("%d ", CSR_READ(rAMP_RDDQDESKEW_CTRL(byte, ch, bit)) & DESKEW_CTRL_BITS);
				
					if (dump_passing_window) {
						shim_printf("[");
						for (rnk = 0; rnk < cfg_params.num_ranks; rnk++) {
							chrnk_indx = loopchrnk_index_calc(ch, rnk, 0);
							if (rnk > 0)
								shim_printf(", ");

							shim_printf("%d ", rddqcal_per_chrnk_right[chrnk_indx][vref_opt_rddq[chrnk_indx]][bit + DQ_NUM_BITS_PER_BYTE * byte]);
							shim_printf("%d", rddqcal_per_chrnk_left[chrnk_indx][vref_opt_rddq[chrnk_indx]][bit + DQ_NUM_BITS_PER_BYTE * byte]);
						}
						shim_printf("] ");
					}
				}

				shim_printf("\n");
			}
		}
		
		if (operations & SELECT_WRDQCAL) {

			shim_printf("\tWrite DQ:\n");
			for (byte = 0; byte < DQ_NUM_BYTES; byte++) {
				tmp = CSR_READ(rAMP_WRDQSDQ_SDLLCODE(byte, ch));
				shim_printf("\t\tByte %d DQ  SDLL: %d\n", byte, (tmp >> 0) & DLLVAL_BITS);
				shim_printf("\t\tByte %d DQS SDLL: %d\n", byte, (tmp >> 8) & DLLVAL_BITS);
			
				shim_printf("\t\t\tPer Bit Deskew: ");
				for (bit = 0; bit < DQ_NUM_BITS_PER_BYTE; bit++) {
					shim_printf("%d ", CSR_READ(rAMP_WRDQDESKEW_CTRL(byte, ch, bit)) & DESKEW_CTRL_BITS);
				
					if (dump_passing_window) {
						shim_printf(" [");
						for (rnk = 0; rnk < cfg_params.num_ranks; rnk++) {
							chrnk_indx = loopchrnk_index_calc(ch, rnk, 0);
							if (rnk > 0)
								shim_printf(", ");

							shim_printf("%d ", wrdqcal_per_chrnk_right[chrnk_indx][vref_opt_wrdq[chrnk_indx]][bit + DQ_NUM_BITS_PER_BYTE * byte]);
							shim_printf("%d", wrdqcal_per_chrnk_left[chrnk_indx][vref_opt_wrdq[chrnk_indx]][bit + DQ_NUM_BITS_PER_BYTE * byte]);
						}
						shim_printf("] ");
					}
				}

				shim_printf("\n");
			}
		}
	}
}

#ifdef ENV_DV
// Force initialization of cfg_params (used by DV)
_ROMCODE_SEGMENT_PREFIX void calibration_init_cfg_params()
{
	// Allow shim layer to change parameters as needed
	shim_init_calibration_params(&cfg_params);
}
#endif

////////////////////////////////////////////////////////////////////////////////////////////////
// CA Sequence
////////////////////////////////////////////////////////////////////////////////////////////////
// CP : Use the following settings for Maui
// cfg_params.sw_loops = 1
// cfg_params.num_ranks = 1
// Number of HW Loops >= 16 ****ALWAYS****
// NOTE: cfg_params.hw_loops is "number of loops less one", so (cfg_params.hw_loops >= 15)
_ROMCODE_SEGMENT_PREFIX void calibrate_ca(void)
{
	uint32_t ch, rnk, swloop, mask_bits, vref;
				
	uint32_t loopchrnk0_indx;
	uint32_t curr_freq_bin;
	uint32_t good_vref_not_found;

	// Initialize registers for CA calibration
	cacal_init_registers();

	// CP : Maui Change
	// CP : Masks will change for Maui	
	cacal_generate_patterns_mask();
	
	mask_bits = 0;
		
	for (ch = 0; ch < cfg_params.num_channels_runCalib; ch++) {

		for (rnk = 0; rnk < cfg_params.num_ranks; rnk++) {
			for (swloop = 0; swloop < cfg_params.cacalib_sw_loops; swloop++) {

				// CP : Maui Change
				amp_setup_ca_cal(ch, rnk, swloop);

				// Reset DQS weak pull-down before CA entry
				curr_freq_bin = 3; 
				amp_reset_dqs_weak_pd(ch, curr_freq_bin); 
				
				// Enter CA training mode
				amp_enter_cacal_mode(ch);

				// Switch to target Freq (800Mz or 1200MHz)
				if(cfg_params.freq_bin < 2)
					cal_pmgr_change_mcu_clk(cfg_params.freq_bin);

            // Update SOC Drive Strengths to match DRAM's FSP
            run_soc_upd(ch);

			// Apply Samsung work-around.
			if(shim_apply_samsung_workaround()) {
				// Perform Impedance calibration.
				amp_do_imp_calib(ch);
			}

				
				// Saving Bucket 1 calibration results at the end of Bin1 calibration
				// Don't save in Elba, since there is no frequency bin 1.
				//if(cfg_params.freq_bin == 0) {
				//	amp_save_ca_bucket1(ch);	
				//}	

				// Clear all CA, CS and CK registers before starting CS/CA calibration.
				amp_init_ca_deskew(ch);
				amp_init_ca_cs_ck_sdll(ch);
				
				// Clear WrLvl SDLL before starting CA calibration.
				// <rdar://problem/17426999>
				amp_program_cawrlvl_sdll(ch, 0, 0);

				// If we are going to frequency Bin 0 from Bin 1, Software initiated MDLL calibration must be done
				// to lock in the new MDLL code
				// FIXME_ELBA: Is this required for Elba??
				if(cfg_params.freq_bin == 0) {
					amp_do_mdll_calib(ch);
				}

				// Required since the dll values may change slightly during calibration
				amp_save_masterdll_values();

				// Following scaling is not applicable to Elba, since Bin 1 is not supported.
				////////*******************************************************************************
				/*
				// Scale Bin1 CK and CS SDLL based on MDLL values at Bin 1 and Bin 0 calibrations.
				if(cfg_params.freq_bin == 0 && (cfg_params.disable_coarse_cs_training == true)) {

					shim_printf("DEBUG - ClkPeriod for Bin0=%d, mdllcode_freq_bin_1[ch]=%d, ClkPeriod for Bin1=%d, mdllcode[ch][AMP_CA]=%d \n",
					cfg_params.clk_period_bin0, mdllcode_freq_bin_1[ch], cfg_params.clk_period_bin1, mdllcode[ch][AMP_CA]);

					if (cfg_params.clk_period_bin0 == 0) {
						shim_panic("Clock Period for Freq Bin 0 should not be ZERO");
					}

					vt_scl_factor = (cfg_params.clk_period_bin1 * mdllcode[ch][AMP_CA] * 100)/(cfg_params.clk_period_bin0 * mdllcode_freq_bin_1[ch]); 
					shim_printf("DEBUG - Computed vt_scl_factor for Bin 0 as %d \n", vt_scl_factor);
	
					cs_sdll_precal_bin0 = (cssdll_bucket1[ch] * vt_scl_factor + (100 >> 1))/100;
					shim_printf("DEBUG - Computed cs_sdll_precal_bin0 value as %d \n", cs_sdll_precal_bin0);

					ck_sdll_precal_bin0 = (cksdll_bucket1[ch] * vt_scl_factor + (100 >> 1))/100; 	
					shim_printf("DEBUG - Computed ck_sdll_precal_bin0 value as %d \n", ck_sdll_precal_bin0);

					// Program the scaled value into CK and CS SDLL registers.
					amp_program_ck_sdll(ch, ck_sdll_precal_bin0);
					amp_program_cs_sdll(ch, cs_sdll_precal_bin0);
				
					// Save MDLL code during Bin 1 calibration.
					if(cfg_params.freq_bin == 1) {
						mdllcode_freq_bin_1[ch] = mdllcode[ch][AMP_CA];			
						shim_printf("DEBUG - Saving a value of %d for frequency bin 1 mdll code for channel %d \n",mdllcode_freq_bin_1[ch], ch);
					}
				}
				*/
				////////*******************************************************************************
			
				// Calibration sequence is to be run for each Vref setting for each rank in each channel, cfg_params.cacalib_sw_loops number of times
				for (vref = 0; vref < cfg_params.ca_num_vref; vref++) {
		
					// Program Vref setting
					amp_program_ca_vref(ch, cfg_params.ca_vref_values[vref], cfg_params.cacalib_dqs_pulse_cnt, cfg_params.cacalib_dqs_pulse_width);

					// Set the Vref run bit
					amp_run_cacal_vref(ch);

					cksdll_cscal[vref] = 0; 
					cssdll_cscal[vref] = 0; 

					// Coarse CS calibration is done for Elba
					// CS Training
					cacal_run_sequence(ch, rnk, CS_TRAINING, mask_bits, swloop, vref);

					// Program delay values calculated from CS training
					cacal_program_cs_training_values();
					
					cksdll_cscal[vref] = ((CSR_READ(rAMP_CKSDLLCTRL(ch)) >> SDLLOFFSET) & DLLVAL_BITS); 
					cssdll_cscal[vref] = ((CSR_READ(rAMP_CSSDLLCTRL(ch)) >> SDLLOFFSET) & DLLVAL_BITS); 

					//shim_printf("DEBUG - After CS training for ch=%d, vref=%d, CKSDLL=0x%x, CSSDLL=0x%x \n", ch, vref, cksdll_cscal[vref], cssdll_cscal[vref]);

					// Training of CA Bits 0-5
					cacal_run_sequence(ch, rnk, CA_TRAINING, mask_bits, swloop, vref);

					// Clear all CA, CS and CK registers before proceeding to the next Vref value
					amp_init_ca_deskew(ch);
					amp_init_ca_cs_ck_sdll(ch);
				}
				
				if(cfg_params.freq_bin == 1) {
					cal_pmgr_change_mcu_clk(3);		// switch to 50MHz
				} else {
					// DON'T restore for Elba since there is no Bin 1 calibration.
					// Restore 800Mhz calibrated values
					// amp_program_ca_bucket1(ch);
		
					cal_pmgr_change_mcu_clk(3);		// switch to 50MHz
				}
                // Update SOC Drive Strengths to match DRAM's FSP
                run_soc_upd(ch);

				// Exit CA Training mode
				amp_exit_cacal_mode(ch);

				// Set DQS weak pull-down after CA entry
				curr_freq_bin = 3; 
				amp_set_dqs_weak_pd(ch, curr_freq_bin); 
				
				// Find if there is even a single Vref which did not panic.
				good_vref_not_found = 1;
				for (vref = 0; vref < cfg_params.ca_num_vref; vref++) {
					good_vref_not_found &= cacal_vref_panic[ch][vref]; 	
				}
				if(good_vref_not_found == 1) {
					shim_panic("Memory CA calibration: Unable to find any Vref which did not panic for channel %d\n", ch);
				}

				// Compute the optimal Vref index
				amp_compute_opt_vref_cacal(ch, rnk, swloop, CA_NUM_BITS); 
			}
		}
	}

	// By now, we have compiled right and left edges of passing window for all CA bits over a number of iterations
	// Aggregate the results, and find the center point of the window, and program it
	// Make sure CS trained values are not messed up because of CA training programing
	// Make sure final CK SDLL value is same for both CS & CA trainings.

	cacal_program_final_values();

	// Do a second stage CS training at the optimal Vref. 
	// Once we figure out the final window and optimal Vref values, do CS training again.
	// We need to move CK & CA together in this step. The final CSSDLL value from this step 
	// will be programmed into CS SDLL.
	//***** Fine tune CS calibration *******//

	for (ch = 0; ch < cfg_params.num_channels_runCalib; ch++) {
		for (rnk = 0; rnk < cfg_params.num_ranks; rnk++) {
			for (swloop = 0; swloop < cfg_params.cacalib_sw_loops; swloop++) {
				
				// Reset DQS weak pull-down before CA entry
				curr_freq_bin = 3; 
				amp_reset_dqs_weak_pd(ch, curr_freq_bin); 
				
				// Enter CA training mode
				amp_enter_cacal_mode(ch);

				if(cfg_params.freq_bin < 2)
					cal_pmgr_change_mcu_clk(cfg_params.freq_bin);		// switch to target Freq (800Mz or 1200MHz)

                // Update SOC Drive Strengths to match DRAM's FSP
                run_soc_upd(ch);
				
				// Apply Samsung work-around.
				if(shim_apply_samsung_workaround()) {
					// Perform Impedance calibration.
					amp_do_imp_calib(ch);
				}

				// Program optimal Vref setting
				loopchrnk0_indx = loopchrnk_index_calc(ch, rnk, swloop);
				uint32_t vref_ch = vref_opt_ca[loopchrnk0_indx];
				amp_program_ca_vref(ch, cfg_params.ca_vref_values[vref_ch], cfg_params.cacalib_dqs_pulse_cnt, cfg_params.cacalib_dqs_pulse_width); //vref_pulse_cnt=0

				// Set the Vref run bit
				amp_run_cacal_vref(ch);

				cksdll_cscal[vref_ch] = ((CSR_READ(rAMP_CKSDLLCTRL(ch)) >> SDLLOFFSET) & DLLVAL_BITS); 
				cssdll_cscal[vref_ch] = ((CSR_READ(rAMP_CSSDLLCTRL(ch)) >> SDLLOFFSET) & DLLVAL_BITS); 
				casdll_cscal[vref_ch] = ((CSR_READ(rAMP_CASDLLCTRL(ch)) >> SDLLOFFSET) & DLLVAL_BITS); 

				// Training of CA Bits 0-5
				cacal_run_sequence(ch, rnk, CS_FINE_TRAINING, mask_bits, swloop, vref_ch);
				
				// Program delay values calculated from fine CS training
				cacal_program_cs_training_values();
					
				cksdll_cscal[vref_ch] = ((CSR_READ(rAMP_CKSDLLCTRL(ch)) >> SDLLOFFSET) & DLLVAL_BITS); 
				cssdll_cscal[vref_ch] = ((CSR_READ(rAMP_CSSDLLCTRL(ch)) >> SDLLOFFSET) & DLLVAL_BITS); 
				casdll_cscal[vref_ch] = ((CSR_READ(rAMP_CASDLLCTRL(ch)) >> SDLLOFFSET) & DLLVAL_BITS); 

				if(cfg_params.freq_bin == 1) {
					cal_pmgr_change_mcu_clk(3);		// switch to 50MHz
				} else {
					// Don't restore for Elba since there is no Bin 1 calibration.
					// Restore 800Mhz calibrated values
					//amp_program_ca_bucket1(ch);
		
					cal_pmgr_change_mcu_clk(3);		// switch to 50MHz
				}
                // Update SOC Drive Strengths to match DRAM's FSP
                run_soc_upd(ch);
			
				// Exit CA Training mode
				amp_exit_cacal_mode(ch);

				// Set DQS weak pull-down after CA entry
				curr_freq_bin = 3; 
				amp_set_dqs_weak_pd(ch, curr_freq_bin); 
			}
		}
	}
	
	// Program CA calibration results again after fine tune CS training.
	cacal_program_final_values();

	if(cfg_params.freq_bin == 1) {
		amp_save_ca_bucket1(ch);	
	}	
}

////////////////////////////////////////////////////////////////////////////////////////////////
// RdDQ Sequence
////////////////////////////////////////////////////////////////////////////////////////////////
_ROMCODE_SEGMENT_PREFIX void calibrate_rddq(void)
{
	uint32_t ch, rnk, vref;
	uint32_t good_vref_not_found;
	
		for (ch = 0; ch < cfg_params.num_channels_runCalib; ch++) {

			for (rnk = 0; rnk < cfg_params.num_ranks; rnk++) {
				amp_setup_rddq_cal(ch, rnk);

				// Populate FIFO in Bin 3 by sending MPC Wr-FIFO command
				if(cfg_params.rddq_legacy_mode == false) {
					// Switch to Bin 3
					cal_pmgr_change_mcu_clk(3);		// switch to 50MHz
					
					// Send Wr-FIFO commands
					amp_rddq_cal_wrfifo(ch, rnk);
	
					// Switch back to Bin 0		
					cal_pmgr_change_mcu_clk(0);		// switch to 1600MHz
				}

				for(vref = 0; vref < cfg_params.rddq_num_vref; vref++) {	
				
					// Program RDDQ Vref setting 
					amp_program_rddq_vref(ch, cfg_params.rddq_vref_values[vref], 0, cfg_params.freq_bin);	
				
					// Find the left and right edges of the eye
					// CP : Maui Change 
					// Find Left failing point first. Then, move on to finding right failing point.
					rddqcal_find_left_failing_point(ch, rnk, vref, false);
					rddqcal_find_right_failing_point(ch, rnk, vref, false);
								
				}
				// Find if there is even a single Vref which did not panic.
				good_vref_not_found = 1;
				for(vref = 0; vref < cfg_params.rddq_num_vref; vref++) {	
					good_vref_not_found &= rddqcal_vref_panic[ch][vref];
				}
				if(good_vref_not_found == 1) {
					shim_panic("Memory RDDQ calibration: Unable to find any Vref which did not panic for channel %d\n", ch);
				}	

				// Compute the optimal Vref index
				amp_compute_opt_vref_rddqcal(ch, rnk, (DQ_TOTAL_BITS + DMI_TOTAL_BITS)); 
			}
		}
	
	// Now that we have per bit left and right endpoints for each channel and rank, aggregate and program final values
	rddqcal_program_final_values();
}

////////////////////////////////////////////////////////////////////////////////////////////////
// WrLvl Sequence
////////////////////////////////////////////////////////////////////////////////////////////////
// Align the clock signal with the DQ signals
_ROMCODE_SEGMENT_PREFIX void calibrate_wrlvl(void)
{
	uint32_t ch, rnk;
	uint8_t byte;	

	amp_wrlvl_init();
	
	for (ch = 0; ch < cfg_params.num_channels_runCalib; ch++) {
		
		// Set up MRW-1 & MRW-2 for Write leveling entry
		// Set RunWrlvlEntry bit
		amp_wrlvl_entry(ch);

		for (rnk = 0; rnk < cfg_params.num_ranks; rnk++) {
		
			// Program CAWRLVL to 0.
			// Program WR WRLVL fields to the value before Write Leveling.
			// <rdar://problem/15714090>
			amp_program_cawrlvl_sdll(ch, 0, 0);
			for(byte = 0; byte < DQ_NUM_BYTES; byte++) {
				amp_program_dqwrlvl_sdll(ch, byte, dqwrlvl_sdll_before_wrlvl[ch][byte], 0);
			}

			// find the region where all bits return a 0
			wrlvlcal_push_to_0s_region(ch, rnk);
			
			// push out the clock signal until all bits return a 1
			wrlvlcal_find_0to1_transition(ch, rnk);
			
			// now go back towards the transition edge found earlier, but from this side of the edge
			wrlvlcal_find_1to0_transition(ch, rnk);
		}

		// Set up MRW-1 & MRW-2 for Write leveling exit
		// Set RunWrlvlExit bit
		amp_wrlvl_exit(ch, cfg_params.freq_bin);
	}
	
	// Program the final wrlvl values
	wrlvlcal_program_final_values();
}

////////////////////////////////////////////////////////////////////////////////////////////////
// WrDQ Sequence
////////////////////////////////////////////////////////////////////////////////////////////////
_ROMCODE_SEGMENT_PREFIX void calibrate_wrdq(void)
{
  uint32_t ch, rnk;
  uint32_t chrnk_indx;
  uint32_t byte;
	
	for (ch = 0; ch < cfg_params.num_channels_runCalib; ch++) {
    for (rnk = 0; rnk < cfg_params.num_ranks; rnk++) {
		
		wrdqcal_init_regs(ch, rnk);

		// Clear deskew registers
		amp_init_wr_deskew(ch);

		// Read the value of CAWRLVL SDLL code obtained from Write Leveling
		chrnk_indx = loopchrnk_index_calc(ch, rnk, 0);
		cawrlvl_sdll_wrlvl[chrnk_indx] = CSR_READ(rAMP_CAWRLVLSDLLCODE(ch)) & 0xFF;
		shim_printf("DEBUG - calibrate_wrdq:: chrnk_indx=%d, cawrlvl_sdll_wrlvl[chrnk_indx]=0x%x \n",chrnk_indx, cawrlvl_sdll_wrlvl[chrnk_indx]);
		for(byte = 0 ; byte < DQ_NUM_BYTES; byte++) {
			dqwrlvl_sdll_after_wrlvl[chrnk_indx][byte] = CSR_READ(rAMP_DQWRLVLSDLLCODE(byte, ch)) & 0xFF;
			shim_printf("DEBUG - calibrate_wrdq:: chrnk_indx=%d, byte=%d, dqwrlvl_sdll_after_wrlvl[chrnk_indx][byte]=0x%x \n", chrnk_indx, byte, dqwrlvl_sdll_after_wrlvl[chrnk_indx][byte]);
		}
	
		wrdqcal_sequence(ch, rnk);
    }
  }

  wrdqcal_program_final_values();
}

///////////////////////////////////////////////////////////////////////////////
////// Local functions
///////////////////////////////////////////////////////////////////////////////
_ROMCODE_SEGMENT_PREFIX void amp_do_mdll_calib(uint32_t ch)
{
	uint8_t byte;
	// POR is to program DLL update for both bytes and then poll for update to clear
	// on both bytes.
	for(byte = 0 ; byte < DQ_NUM_BYTES; byte++) {
		CSR_WRITE(rAMP_DLLUPDTCMD(byte, ch), 1 << RUN_DLL_UPDT);
	}

	for(byte = 0 ; byte < DQ_NUM_BYTES; byte++) {
		SPIN_W_TMO_WHILE (CSR_READ(rAMP_DLLUPDTCMD(byte, ch)) & (1 << RUN_DLL_UPDT));
	}
}

_ROMCODE_SEGMENT_PREFIX void amp_do_imp_calib(uint32_t ch)
{
	uint32_t ImpCalCmd;

	ImpCalCmd = CSR_READ(rAMP_CA_IMPCALCMD(ch));

	ImpCalCmd = (ImpCalCmd & RUNIMPCAL_MASK) | (1 << RUNIMPCAL_OFFSET); 
	
	CSR_WRITE(rAMP_CA_IMPCALCMD(ch), ImpCalCmd); 

	SPIN_W_TMO_WHILE (CSR_READ(rAMP_CA_IMPCALCMD(ch)) & (1 << RUNIMPCAL_OFFSET));
}

_ROMCODE_SEGMENT_PREFIX void amp_setup_ca_cal(uint32_t ch, uint32_t rnk, uint32_t swloop)
{
	uint32_t cacalctrl;

	// Map the CA bits to appropriate dq byte lanes, accounting for swizzling.
	// Bytes 0 and 2 from DDR are hooked to AMPDQ0, and are bit swizzled (i.e., bit 0 from DDR connected to bit 7 on AMPDQ, and so on).
	// Bytes 1 and 3 are hooked to AMPDQ1, and are NOT bit swizzled.
	// For the 10 CA bits, that means CA0-3 (MR41) and CA4 (MR48) come back from DDR on AMPDQ0,
	// and CA5-8 (MR41) and CA9 (MR48) on AMPDQ1.
	
	CSR_WRITE(rAMP_CACALBITSELECT(ch), ((cfg_params.cacal_bit_sel[7] << 28)|(cfg_params.cacal_bit_sel[6] << 24)|(cfg_params.cacal_bit_sel[5] << 20)|(cfg_params.cacal_bit_sel[4] << 16)|(cfg_params.cacal_bit_sel[3] << 12)|(cfg_params.cacal_bit_sel[2] << 8)|(cfg_params.cacal_bit_sel[1]<< 4)|(cfg_params.cacal_bit_sel[0] << 0)));

	// Program rank, hardware loop count, and timing params
	// Timing params are taken from lpddr4 jedec spec
	// Program DQ Byte on which to expect CA data during calibration
	cacalctrl = (cfg_params.cacal_rd_byte_sel << 28) | (rnk << 24) | (cfg_params.cacalib_hw_loops << 16) | (cfg_params.tADR << 8) | (cfg_params.tMRD << 0);
	
	CSR_WRITE(rAMP_CACALCTRL(ch), cacalctrl);
	
	// CP: Maui Change
	// Also Program rAMP_CACALCTRL2 register
	CSR_WRITE(rAMP_CACALCTRL2(ch),(cfg_params.tDSTrain << 24) | (cfg_params.tCKCKEH << 16) | (cfg_params.tVREF_LONG << 8) | (cfg_params.tCAENT));
	
	// Program rAMP_CACALCTRL3 register
	CSR_WRITE(rAMP_CACALCTRL3(ch),(cfg_params.CsActiveCycleCnt << 24) | (cfg_params.CsStopCycleCnt << 16) | (cfg_params.tDQSCKE << 8) | (cfg_params.tCKEHCMD));
	
	// Program CA invert mask
	CSR_WRITE(rAMP_CACALPAT(ch), (cfg_params.ca_invert_mask<<16)|(0));

	// Program PRBS7I patterns
	CSR_WRITE(rAMP_CALPATPRBS7_0(ch), cfg_params.prbs7_pattern_0);
	CSR_WRITE(rAMP_CALPATPRBS7_1(ch), cfg_params.prbs7_pattern_1);
	CSR_WRITE(rAMP_CALPATPRBS7_2(ch), cfg_params.prbs7_pattern_2);
	CSR_WRITE(rAMP_CALPATPRBS7_3(ch), cfg_params.prbs7_pattern_3);
	
	// Program start points in PRBS7I patterns for all CA bits
	CSR_WRITE(rAMP_CACALPATSEED_0(ch), (cfg_params.ca_prbs_seed_bit[3] << 24) | (cfg_params.ca_prbs_seed_bit[2] << 16) | (cfg_params.ca_prbs_seed_bit[1] << 8) | (cfg_params.ca_prbs_seed_bit[0] << 0));
	CSR_WRITE(rAMP_CACALPATSEED_1(ch), (cfg_params.ca_prbs_seed_bit[5] << 8) | (cfg_params.ca_prbs_seed_bit[4] << 0));
}

_ROMCODE_SEGMENT_PREFIX void run_soc_upd(uint32_t ch)
{
	uint32_t freqchngctl;

	// Save the value of freqchngctl register
		freqchngctl = CSR_READ(rAMCX_DRAMCFG_FREQCHNGMRWCNT(ch));
		CSR_WRITE(rAMCX_DRAMCFG_FREQCHNGMRWCNT(ch), (freqchngctl & 0xFFFEFFFF) | 0x10000);

    // Add some delay here. Do dummy reads	
		CSR_READ(rAMCX_DRAMCFG_FREQCHNGMRWCNT(ch));
		CSR_READ(rAMCX_DRAMCFG_FREQCHNGMRWCNT(ch));
		CSR_READ(rAMCX_DRAMCFG_FREQCHNGMRWCNT(ch));
		CSR_READ(rAMCX_DRAMCFG_FREQCHNGMRWCNT(ch));
		CSR_READ(rAMCX_DRAMCFG_FREQCHNGMRWCNT(ch));
}

// CP : Maui Change
_ROMCODE_SEGMENT_PREFIX void amp_program_ca_vref(uint32_t ch, uint32_t vref, uint32_t cacalib_dqs_pulse_cnt, uint32_t cacalib_dqs_pulse_width)
{
	uint32_t cacacalvref; 
	uint32_t vref_swizzled;	

	// This is required if there is any bit swizzling.
	vref_swizzled = (((vref & 0x40) >> 6) << cfg_params.cacal_bit_sel[6]) | ( ((vref & 0x20) >> 5) << cfg_params.cacal_bit_sel[5]) | (((vref & 0x10) >> 4) << cfg_params.cacal_bit_sel[4]) | (((vref & 0x8) >> 3) << cfg_params.cacal_bit_sel[3]) 
						 | (((vref & 0x4) >> 2) << cfg_params.cacal_bit_sel[2]) | (((vref & 0x2) >> 1) << cfg_params.cacal_bit_sel[1]) | (((vref & 0x1) >> 0) << cfg_params.cacal_bit_sel[0]); 

	// Program 
	// 1.Vref setting at which CA calibration is to be run
	// 2.Vref pulse count
		cacacalvref = CSR_READ(rAMP_CACACALVREF(ch)); 
		
		CSR_WRITE(rAMP_CACACALVREF(ch),(cacacalvref & 0xFF000000) | (cacalib_dqs_pulse_width << CACAL_DQS_PULSE_WIDTH_OFFSET) | (cacalib_dqs_pulse_cnt << CACAL_DQS_PULSE_CNT_OFFSET) | (vref_swizzled));
}

_ROMCODE_SEGMENT_PREFIX void amp_program_rddq_vref(uint32_t ch, uint32_t vref, uint32_t vref_pulse_cnt, uint8_t freq_bin)
{
	uint32_t rddqcalvrefcodecontrol; 

	// Program 
	// 1.Vref setting at which RDDQ calibration is to be run
	// 2.Vref pulse count

		if(freq_bin == 0) {
			// Program AMPS register with optimal RDDQ Vref
			rddqcalvrefcodecontrol = CSR_READ(rAMP_RDDQCALVREFCODESTATUS(ch));
			CSR_WRITE(rAMP_RDDQCALVREFCODECONTROL(ch), (rddqcalvrefcodecontrol & RDDQ_VREF_F0_MASK) | (1 << 7) | (vref << RDDQ_VREF_F0_OFFSET));				
		}
		else if(freq_bin == 1) {
			// Program AMPS register with optimal RDDQ Vref
			rddqcalvrefcodecontrol = CSR_READ(rAMP_RDDQCALVREFCODESTATUS(ch));
			CSR_WRITE(rAMP_RDDQCALVREFCODECONTROL(ch), (rddqcalvrefcodecontrol & RDDQ_VREF_F1_MASK) | (1 << 7) | (vref << RDDQ_VREF_F1_OFFSET));				
		}
}

// CP : Maui Change
_ROMCODE_SEGMENT_PREFIX void amp_program_wrdq_vref(uint8_t chan, uint8_t rank, uint32_t vref, uint32_t vref_pulse_cnt)
{
	uint32_t mr14_data; 

	shim_mrcmd_to_ch_rnk(MR_READ, chan, rank, MR14, (uintptr_t)&mr14_data);
	
	mr14_data = (mr14_data &0x80) | (vref);

	shim_mrcmd_to_ch_rnk(MR_WRITE, chan, rank, MR14, mr14_data);
}

// (Re-)Initialize CA & CS deskew registers
_ROMCODE_SEGMENT_PREFIX void amp_init_ca_deskew(uint32_t ch)
{
	uint32_t d;
	
	// Clear cadeskewctrl registers
	for (d = 0; d < CA_NUM_BITS; d++) {
		CSR_WRITE(rAMP_CADESKEW_CTRL(ch, d), 0 | RUNDESKEWUPD);
		SPIN_W_TMO_WHILE (CSR_READ(rAMP_CADESKEW_CTRL(ch, d)) & RUNDESKEWUPD);
	}
}

// (Re-)Initialize CA, CK and CS SDLL registers
_ROMCODE_SEGMENT_PREFIX void amp_init_ca_cs_ck_sdll(uint32_t ch)
{
	// Clear casdll register
	CSR_WRITE(rAMP_CASDLLCTRL(ch), (0 << SDLLOFFSET) | (1 << RUN_SDLLUPDOVR));
	SPIN_W_TMO_WHILE (CSR_READ(rAMP_CASDLLCTRL(ch)) & (1 << RUN_SDLLUPDOVR));

	// Clear cssdll register
	CSR_WRITE(rAMP_CSSDLLCTRL(ch), (0 << SDLLOFFSET) | (1 << RUN_SDLLUPDOVR));
	SPIN_W_TMO_WHILE (CSR_READ(rAMP_CSSDLLCTRL(ch)) & (1 << RUN_SDLLUPDOVR));

	// Clear cksdll register
	CSR_WRITE(rAMP_CKSDLLCTRL(ch), (0 << SDLLOFFSET) | (1 << RUN_SDLLUPDOVR));
	SPIN_W_TMO_WHILE (CSR_READ(rAMP_CKSDLLCTRL(ch)) & (1 << RUN_SDLLUPDOVR));

}

_ROMCODE_SEGMENT_PREFIX  void amp_init_wr_deskew(uint32_t ch)
{
	uint8_t byte, bit_indx;

	for(byte = 0; byte < DQ_NUM_BYTES; byte++) {
		// Clear DQS deskew
		CSR_WRITE(rAMP_WRDQSDESKEW_CTRL(byte, ch), 0 | RUNDESKEWUPD);

		// Poll for deskew update to be done.
		SPIN_W_TMO_WHILE (CSR_READ(rAMP_WRDQSDESKEW_CTRL(byte, ch)) & RUNDESKEWUPD);
		
		// Clear DM deskew 
		CSR_WRITE(rAMP_WRDMDESKEW_CTRL(byte, ch), 0 | RUNDESKEWUPD);
			
		// Poll for deskew update to be done.
		SPIN_W_TMO_WHILE (CSR_READ(rAMP_WRDMDESKEW_CTRL(byte, ch)) & RUNDESKEWUPD);

		// Clear DQ deskew 
		for (bit_indx = 0; bit_indx < DQ_NUM_BITS_PER_BYTE; bit_indx++) {
			CSR_WRITE(rAMP_WRDQDESKEW_CTRL(byte, ch, bit_indx), 0 | RUNDESKEWUPD);

			// Poll for deskew update to be done.
			SPIN_W_TMO_WHILE (CSR_READ(rAMP_WRDQDESKEW_CTRL(byte, ch, bit_indx)) & RUNDESKEWUPD);
		}
	}
}

_ROMCODE_SEGMENT_PREFIX void amp_push_casdll_out(uint32_t ch, uint32_t casdll_ovr_val)
{
	uint32_t ca_bit;
	uint32_t casdll_tap;
	uint32_t cadeskewcode;
	
	// For Max skew slow corner, SDLL may have to go upto 3*MDLL+1.  
	if (casdll_ovr_val > (3 * mdllcode[ch][AMP_CA] + 1 )) {
		casdll_tap = (3 * mdllcode[ch][AMP_CA]);
			
		if ((casdll_ovr_val - casdll_tap) >= MAX_DESKEW_PROGRAMMED)
			cadeskewcode = MAX_DESKEW_PROGRAMMED;
		else
			cadeskewcode = casdll_ovr_val - casdll_tap;

		// Adjust deskew registers for each ca bit
		for (ca_bit = 0; ca_bit < CA_NUM_BITS; ca_bit++) {
			CSR_WRITE(rAMP_CADESKEW_CTRL(ch, ca_bit), cadeskewcode | RUNDESKEWUPD);
			SPIN_W_TMO_WHILE (CSR_READ(rAMP_CADESKEW_CTRL(ch, ca_bit)) & RUNDESKEWUPD);
		}

		casdll_ovr_val = (3 * mdllcode[ch][AMP_CA]);

	}
	
	CSR_WRITE(rAMP_CASDLLCTRL(ch), (casdll_ovr_val << SDLLOFFSET) | (1 << RUN_SDLLUPDOVR));
	// Poll for SDLL RunOverride bit to go low
	SPIN_W_TMO_WHILE (CSR_READ(rAMP_CASDLLCTRL(ch)) & (1 << RUN_SDLLUPDOVR));
}

_ROMCODE_SEGMENT_PREFIX void amp_program_ca_sdll(uint32_t ch, uint32_t casdll_val)
{
	CSR_WRITE(rAMP_CASDLLCTRL(ch), (casdll_val << SDLLOFFSET) | (1 << RUN_SDLLUPDWRRES));

	// Poll for SDLL RunOverride bit to go low
	SPIN_W_TMO_WHILE (CSR_READ(rAMP_CASDLLCTRL(ch)) & (1 << RUN_SDLLUPDWRRES));
}

_ROMCODE_SEGMENT_PREFIX void amp_push_cksdll_out(uint32_t ch, uint32_t cksdll_ovr_val)
{
	uint32_t cksdll_tap;
	uint32_t ckdeskewcode;
	
	// For Max skew slow corner, SDLL may have to go upto 3*MDLL+1.  
	if (cksdll_ovr_val > (3 * mdllcode[ch][AMP_CA]) + 1) {
		cksdll_tap = (3 * mdllcode[ch][AMP_CA]);
			
		if ((cksdll_ovr_val - cksdll_tap) >= MAX_DESKEW_PROGRAMMED)
			ckdeskewcode = MAX_DESKEW_PROGRAMMED;
		else
			ckdeskewcode = cksdll_ovr_val - cksdll_tap;

		// Adjust deskew registers for each ck bit
		CSR_WRITE(rAMP_CKDESKEW_CTRL(ch), ckdeskewcode | RUNDESKEWUPD);

		cksdll_ovr_val = (3 * mdllcode[ch][AMP_CA]);
	}
	
	CSR_WRITE(rAMP_CKSDLLCTRL(ch), (cksdll_ovr_val << SDLLOFFSET) | (1 << RUN_SDLLUPDOVR));
	
	SPIN_W_TMO_WHILE (CSR_READ(rAMP_CKSDLLCTRL(ch)) & (1 << RUN_SDLLUPDOVR));
}

_ROMCODE_SEGMENT_PREFIX void amp_program_ck_sdll(uint32_t ch, uint32_t cksdll_val)
{
	CSR_WRITE(rAMP_CKSDLLCTRL(ch), (cksdll_val << SDLLOFFSET) | (1 << RUN_SDLLUPDWRRES));
	
	SPIN_W_TMO_WHILE (CSR_READ(rAMP_CKSDLLCTRL(ch)) & (1 << RUN_SDLLUPDWRRES));
}

_ROMCODE_SEGMENT_PREFIX void amp_push_cssdll_out(uint32_t ch, uint32_t cssdll_ovr_val)
{
	uint32_t cssdll_tap;
	uint32_t csdeskewcode;
	
	// For Max skew slow corner, SDLL may have to go upto 3*MDLL+1.  
	if (cssdll_ovr_val > (3 * mdllcode[ch][AMP_CA]) + 1) {
		cssdll_tap = (3 * mdllcode[ch][AMP_CA]);
			
		if ((cssdll_ovr_val - cssdll_tap) >= MAX_DESKEW_PROGRAMMED)
			csdeskewcode = MAX_DESKEW_PROGRAMMED;
		else
			csdeskewcode = cssdll_ovr_val - cssdll_tap;

		// Adjust deskew register for CS
		CSR_WRITE(rAMP_CSDESKEW_CTRL(ch), csdeskewcode | RUNDESKEWUPD);

		cssdll_ovr_val = (3 * mdllcode[ch][AMP_CA]);
	}
	
	CSR_WRITE(rAMP_CSSDLLCTRL(ch), (cssdll_ovr_val << SDLLOFFSET) | (1 << RUN_SDLLUPDOVR));
	// Poll for SDLL RunOverride bit to go low
	SPIN_W_TMO_WHILE (CSR_READ(rAMP_CSSDLLCTRL(ch)) & (1 << RUN_SDLLUPDOVR));
}

_ROMCODE_SEGMENT_PREFIX void amp_program_cs_sdll(uint32_t ch, uint32_t cssdll_val)
{
	CSR_WRITE(rAMP_CSSDLLCTRL(ch), (cssdll_val << SDLLOFFSET) | (1 << RUN_SDLLUPDWRRES));

	// Poll for SDLL RunOverride bit to go low
	SPIN_W_TMO_WHILE (CSR_READ(rAMP_CSSDLLCTRL(ch)) & (1 << RUN_SDLLUPDWRRES));
}

_ROMCODE_SEGMENT_PREFIX void amp_set_dqs_weak_pd(uint32_t ch, uint32_t freq_bin)
{
	uint32_t amph_cfgh_dqs0_wkpupd, amph_cfgh_dqs1_wkpupd;
	
	amph_cfgh_dqs0_wkpupd = CSR_READ(rAMPH_CFGH_DQS0_WKPUPD(ch));
	amph_cfgh_dqs1_wkpupd = CSR_READ(rAMPH_CFGH_DQS1_WKPUPD(ch));


	if(freq_bin == 0) {
		CSR_WRITE(rAMPH_CFGH_DQS0_WKPUPD(ch), (amph_cfgh_dqs0_wkpupd & F0_WEAK_PD_MASK) | (1 << F0_WEAK_PD_OFFSET));
		CSR_WRITE(rAMPH_CFGH_DQS1_WKPUPD(ch), (amph_cfgh_dqs1_wkpupd & F0_WEAK_PD_MASK) | (1 << F0_WEAK_PD_OFFSET));
	}
	else if(freq_bin == 1) {
		CSR_WRITE(rAMPH_CFGH_DQS0_WKPUPD(ch), (amph_cfgh_dqs0_wkpupd & F1_WEAK_PD_MASK) | (1 << F1_WEAK_PD_OFFSET));
		CSR_WRITE(rAMPH_CFGH_DQS1_WKPUPD(ch), (amph_cfgh_dqs1_wkpupd & F1_WEAK_PD_MASK) | (1 << F1_WEAK_PD_OFFSET));
	}
	else if(freq_bin == 3) {
		CSR_WRITE(rAMPH_CFGH_DQS0_WKPUPD(ch), (amph_cfgh_dqs0_wkpupd & F3_WEAK_PD_MASK) | (1 << F3_WEAK_PD_OFFSET));
		CSR_WRITE(rAMPH_CFGH_DQS1_WKPUPD(ch), (amph_cfgh_dqs1_wkpupd & F3_WEAK_PD_MASK) | (1 << F3_WEAK_PD_OFFSET));
	}


}

_ROMCODE_SEGMENT_PREFIX void amp_reset_dqs_weak_pd(uint32_t ch, uint32_t freq_bin)
{
	uint32_t amph_cfgh_dqs0_wkpupd, amph_cfgh_dqs1_wkpupd;
	
	amph_cfgh_dqs0_wkpupd = CSR_READ(rAMPH_CFGH_DQS0_WKPUPD(ch));
	amph_cfgh_dqs1_wkpupd = CSR_READ(rAMPH_CFGH_DQS1_WKPUPD(ch));


	if(freq_bin == 0) {
		CSR_WRITE(rAMPH_CFGH_DQS0_WKPUPD(ch), (amph_cfgh_dqs0_wkpupd & F0_WEAK_PD_MASK) | (0 << F0_WEAK_PD_OFFSET));
		CSR_WRITE(rAMPH_CFGH_DQS1_WKPUPD(ch), (amph_cfgh_dqs1_wkpupd & F0_WEAK_PD_MASK) | (0 << F0_WEAK_PD_OFFSET));
	}
	else if(freq_bin == 1) {
		CSR_WRITE(rAMPH_CFGH_DQS0_WKPUPD(ch), (amph_cfgh_dqs0_wkpupd & F1_WEAK_PD_MASK) | (0 << F1_WEAK_PD_OFFSET));
		CSR_WRITE(rAMPH_CFGH_DQS1_WKPUPD(ch), (amph_cfgh_dqs1_wkpupd & F1_WEAK_PD_MASK) | (0 << F1_WEAK_PD_OFFSET));
	}
	else if(freq_bin == 3) {
		CSR_WRITE(rAMPH_CFGH_DQS0_WKPUPD(ch), (amph_cfgh_dqs0_wkpupd & F3_WEAK_PD_MASK) | (0 << F3_WEAK_PD_OFFSET));
		CSR_WRITE(rAMPH_CFGH_DQS1_WKPUPD(ch), (amph_cfgh_dqs1_wkpupd & F3_WEAK_PD_MASK) | (0 << F3_WEAK_PD_OFFSET));
	}
}

_ROMCODE_SEGMENT_PREFIX void amp_set_dqs_idle_active(uint32_t ch, uint32_t freq_bin)
{
	uint32_t amph_cfgh_dqs0_wkpupd, amph_cfgh_dqs1_wkpupd;
	
	amph_cfgh_dqs0_wkpupd = CSR_READ(rAMPH_CFGH_DQS0_WKPUPD(ch));
	amph_cfgh_dqs1_wkpupd = CSR_READ(rAMPH_CFGH_DQS1_WKPUPD(ch));


	if(freq_bin == 0) {
		CSR_WRITE(rAMPH_CFGH_DQS0_WKPUPD(ch), (amph_cfgh_dqs0_wkpupd & F0_IDLE_ACTIVE_MASK) | (1 << F0_IDLE_ACTIVE_OFFSET));
		CSR_WRITE(rAMPH_CFGH_DQS1_WKPUPD(ch), (amph_cfgh_dqs1_wkpupd & F0_IDLE_ACTIVE_MASK) | (1 << F0_IDLE_ACTIVE_OFFSET));
	}
	else if(freq_bin == 1) {
		CSR_WRITE(rAMPH_CFGH_DQS0_WKPUPD(ch), (amph_cfgh_dqs0_wkpupd & F1_IDLE_ACTIVE_MASK) | (1 << F1_IDLE_ACTIVE_OFFSET));
		CSR_WRITE(rAMPH_CFGH_DQS1_WKPUPD(ch), (amph_cfgh_dqs1_wkpupd & F1_IDLE_ACTIVE_MASK) | (1 << F1_IDLE_ACTIVE_OFFSET));
	}
	else if(freq_bin == 3) {
		CSR_WRITE(rAMPH_CFGH_DQS0_WKPUPD(ch), (amph_cfgh_dqs0_wkpupd & F3_IDLE_ACTIVE_MASK) | (1 << F3_IDLE_ACTIVE_OFFSET));
		CSR_WRITE(rAMPH_CFGH_DQS1_WKPUPD(ch), (amph_cfgh_dqs1_wkpupd & F3_IDLE_ACTIVE_MASK) | (1 << F3_IDLE_ACTIVE_OFFSET));
	}
}

_ROMCODE_SEGMENT_PREFIX void amp_reset_dqs_idle_active(uint32_t ch, uint32_t freq_bin)
{
	uint32_t amph_cfgh_dqs0_wkpupd, amph_cfgh_dqs1_wkpupd;
	
	amph_cfgh_dqs0_wkpupd = CSR_READ(rAMPH_CFGH_DQS0_WKPUPD(ch));
	amph_cfgh_dqs1_wkpupd = CSR_READ(rAMPH_CFGH_DQS1_WKPUPD(ch));


	if(freq_bin == 0) {
		CSR_WRITE(rAMPH_CFGH_DQS0_WKPUPD(ch), (amph_cfgh_dqs0_wkpupd & F0_IDLE_ACTIVE_MASK) | (0 << F0_IDLE_ACTIVE_OFFSET));
		CSR_WRITE(rAMPH_CFGH_DQS1_WKPUPD(ch), (amph_cfgh_dqs1_wkpupd & F0_IDLE_ACTIVE_MASK) | (0 << F0_IDLE_ACTIVE_OFFSET));
	}
	else if(freq_bin == 1) {
		CSR_WRITE(rAMPH_CFGH_DQS0_WKPUPD(ch), (amph_cfgh_dqs0_wkpupd & F1_IDLE_ACTIVE_MASK) | (0 << F1_IDLE_ACTIVE_OFFSET));
		CSR_WRITE(rAMPH_CFGH_DQS1_WKPUPD(ch), (amph_cfgh_dqs1_wkpupd & F1_IDLE_ACTIVE_MASK) | (0 << F1_IDLE_ACTIVE_OFFSET));
	}
	else if(freq_bin == 3) {
		CSR_WRITE(rAMPH_CFGH_DQS0_WKPUPD(ch), (amph_cfgh_dqs0_wkpupd & F3_IDLE_ACTIVE_MASK) | (0 << F3_IDLE_ACTIVE_OFFSET));
		CSR_WRITE(rAMPH_CFGH_DQS1_WKPUPD(ch), (amph_cfgh_dqs1_wkpupd & F3_IDLE_ACTIVE_MASK) | (0 << F3_IDLE_ACTIVE_OFFSET));
	}
}

_ROMCODE_SEGMENT_PREFIX void amp_enter_cacal_mode(uint32_t ch)
{
	uint32_t cacalrun = CSR_READ(rAMP_CACALRUN(ch));
	
	CSR_WRITE(rAMP_CACALRUN(ch), cacalrun | CACALRUN_RUNCACALENTRY);
	
	SPIN_W_TMO_WHILE (CSR_READ(rAMP_CACALRUN(ch)) & CACALRUN_RUNCACALENTRY);
}

_ROMCODE_SEGMENT_PREFIX void amp_exit_cacal_mode(uint32_t ch)
{
	uint32_t cacalrun = CSR_READ(rAMP_CACALRUN(ch));
	uint32_t cabist_bistrdfifoctrl = CSR_READ(rAMPH_CFGH_CABIST_BISTRDFIFOCTRL(ch));
	
	CSR_WRITE(rAMP_CACALRUN(ch), cacalrun | CACALRUN_RUNCACALEXIT);

	SPIN_W_TMO_WHILE (CSR_READ(rAMP_CACALRUN(ch)) & CACALRUN_RUNCACALEXIT);

	// Toggle Read FIFO pointer reset.
	CSR_WRITE(rAMPH_CFGH_CABIST_BISTRDFIFOCTRL(ch), (cabist_bistrdfifoctrl & BIST_RDFIFO_RESET_MASK) | (1 << BIST_RDFIFO_RESET_OFFSET));
	
	SPIN_W_TMO_WHILE ((CSR_READ(rAMPH_CFGH_CABIST_BISTRDFIFOCTRL(ch)) & (1 << BIST_RDFIFO_RESET_OFFSET)) == 0);  
	
	CSR_WRITE(rAMPH_CFGH_CABIST_BISTRDFIFOCTRL(ch), (cabist_bistrdfifoctrl & BIST_RDFIFO_RESET_MASK) | (0 << BIST_RDFIFO_RESET_OFFSET));
	
	SPIN_W_TMO_WHILE ((CSR_READ(rAMPH_CFGH_CABIST_BISTRDFIFOCTRL(ch)) & (1 << BIST_RDFIFO_RESET_OFFSET)) == 1);  
}

_ROMCODE_SEGMENT_PREFIX void amp_run_cacal_vref(uint32_t ch)
{
	uint32_t cacalrun = CSR_READ(rAMP_CACALRUN(ch));
	
	// Set RunCaCalVref bit
	CSR_WRITE(rAMP_CACALRUN(ch), cacalrun | CACALRUN_RUNCACALVREF);
	// Poll for RunCaCalVref to go low.	
	SPIN_W_TMO_WHILE (CSR_READ(rAMP_CACALRUN(ch)) & CACALRUN_RUNCACALVREF);
}

_ROMCODE_SEGMENT_PREFIX static uint32_t amp_run_cacal(uint32_t ch, uint32_t training_mode)
{
	uint32_t cacalrun = CSR_READ(rAMP_CACALRUN(ch));
	
	if(training_mode == CS_TRAINING) {
		// Set RunCaCalCS bit
		CSR_WRITE(rAMP_CACALRUN(ch), cacalrun | CACALRUN_RUNCACALCS);

		// Poll on the RunCaCalCS bit
		SPIN_W_TMO_WHILE (CSR_READ(rAMP_CACALRUN(ch)) & CACALRUN_RUNCACALCS);
	}
	
	if((training_mode == CA_TRAINING) || (training_mode == CS_FINE_TRAINING)) {
		// Set RunCaCalCA bit
		CSR_WRITE(rAMP_CACALRUN(ch), cacalrun | CACALRUN_RUNCACALCA);

		// Poll on the RunCaCalCA bit
		SPIN_W_TMO_WHILE (CSR_READ(rAMP_CACALRUN(ch)) & CACALRUN_RUNCACALCA);
	}

	return ((CSR_READ(rAMP_CACALRESULT(ch)) & CA_ALL_BITS));
}

_ROMCODE_SEGMENT_PREFIX void amp_setup_rddq_cal(uint32_t ch, uint32_t rnk)
{
	// Program tWL, tRL for frequency bins 0-3 --> Should be part of the init sequence

	uint32_t rdwr_dqcal_loopcnt = CSR_READ(rAMP_RDWRDQCALLOOPCNT(ch));
	uint32_t rdwrdqcaltiming_f0 = CSR_READ(rAMP_RdWrDqCalTiming_f0(ch));	
	uint32_t rdwrdqcaltiming_f1 = CSR_READ(rAMP_RdWrDqCalTiming_f1(ch));	
	uint32_t dqcal_burst_len_mode = CSR_READ(rAMP_DQCAL_BURSTLEN_MODE(ch));
	uint8_t  tRL, tWL;

	// Program SWRDDQCAL Loop count
	CSR_WRITE(rAMP_RDWRDQCALLOOPCNT(ch), (rdwr_dqcal_loopcnt & 0xFFFFFF00) | (cfg_params.rddq_cal_loopcnt & 0xFF));

	// Init sequence sets up pattern & Invert mask registers (MR15, MR20, MR32 & MR40)

	// Set up tRL values for each frequency bucket
	tRL = ((cfg_params.RL)/2) - 6;
	tWL = ((cfg_params.WL)/2) - 2; 

	if(cfg_params.freq_bin == 0) {
		CSR_WRITE(rAMP_RdWrDqCalTiming_f0(ch), (rdwrdqcaltiming_f0 & 0xFFFF0000) | (tWL << 8) | tRL);
	}
	else if(cfg_params.freq_bin == 1) {
		shim_printf("amp_setup_rddq_cal:: Writing a value of 0x%x into rAMP_RdWrDqCalTiming_f1 \n", (rdwrdqcaltiming_f1 & 0xFFFF0000) | (tWL << 8) | tRL);
		CSR_WRITE(rAMP_RdWrDqCalTiming_f1(ch), (rdwrdqcaltiming_f1 & 0xFFFF0000) | (tWL << 8) | tRL);
	}
	//CSR_WRITE(rAMP_RdWrDqCalTiming_f3(ch), (rdwrdqcaltiming_f3 & 0xFFFFFF00) | (0x0 << 4) | (0x0));
	
	// Program PRBS7I patterns
	CSR_WRITE(rAMP_CALPATPRBS7_0(ch), cfg_params.prbs7_pattern_0);
	CSR_WRITE(rAMP_CALPATPRBS7_1(ch), cfg_params.prbs7_pattern_1);
	CSR_WRITE(rAMP_CALPATPRBS7_2(ch), cfg_params.prbs7_pattern_2);
	CSR_WRITE(rAMP_CALPATPRBS7_3(ch), cfg_params.prbs7_pattern_3);
	
	// Program start points in PRBS7I patterns for all CA bits
	CSR_WRITE(rAMP_WRDQCALPATSEED_B0_0(ch), (cfg_params.wrdq_prbs_seed[0][3] << 24) | (cfg_params.wrdq_prbs_seed[0][2] << 16) | (cfg_params.wrdq_prbs_seed[0][1] << 8) | (cfg_params.wrdq_prbs_seed[0][0] << 0));
	CSR_WRITE(rAMP_WRDQCALPATSEED_B0_1(ch), (cfg_params.wrdq_prbs_seed[0][7] << 24) | (cfg_params.wrdq_prbs_seed[0][6] << 16) | (cfg_params.wrdq_prbs_seed[0][5] << 8) | (cfg_params.wrdq_prbs_seed[0][4] << 0));
	CSR_WRITE(rAMP_WRDQCALPATSEED_B0_2(ch), (cfg_params.wrdq_prbs_seed[0][8] << 0));
	CSR_WRITE(rAMP_WRDQCALPATSEED_B1_0(ch), (cfg_params.wrdq_prbs_seed[1][3] << 24) | (cfg_params.wrdq_prbs_seed[1][2] << 16) | (cfg_params.wrdq_prbs_seed[1][1] << 8) | (cfg_params.wrdq_prbs_seed[1][0] << 0));
	CSR_WRITE(rAMP_WRDQCALPATSEED_B1_1(ch), (cfg_params.wrdq_prbs_seed[1][7] << 24) | (cfg_params.wrdq_prbs_seed[1][6] << 16) | (cfg_params.wrdq_prbs_seed[1][5] << 8) | (cfg_params.wrdq_prbs_seed[1][4] << 0));
	CSR_WRITE(rAMP_WRDQCALPATSEED_B1_2(ch), (cfg_params.wrdq_prbs_seed[1][8] << 0));

	if(cfg_params.rddq_legacy_mode == true) {
		CSR_WRITE(rAMP_DQCAL_BURSTLEN_MODE(ch), (dqcal_burst_len_mode & RDDQCAL_BURSTLEN_MODE_MASK) | (1 << RDDQCAL_BURSTLEN_MODE_OFFSET));
	}
	else {
		CSR_WRITE(rAMP_DQCAL_BURSTLEN_MODE(ch), (dqcal_burst_len_mode & RDDQCAL_BURSTLEN_MODE_MASK) | (0 << RDDQCAL_BURSTLEN_MODE_OFFSET));
	}
}

// This functions sends WR-FIFO commands to populate FIFO for RDDQ calibration 
_ROMCODE_SEGMENT_PREFIX void amp_rddq_cal_wrfifo(uint32_t ch, uint32_t rnk)
{
	uint32_t dq_cal_run = CSR_READ(rAMP_DQCALRUN(ch)); 

	CSR_WRITE(rAMP_DQCALRUN(ch), (dq_cal_run & RUNRDDQCAL_WRFIFO_MASK) | (1 << RUNRDDQCAL_WRFIFO_OFFSET));
	while (CSR_READ(rAMP_DQCALRUN(ch)) & ~RUNRDDQCAL_WRFIFO_MASK);
}

// This functions set the slave dll for a particular byte lane of RDDQ as specified in the offset parameter
_ROMCODE_SEGMENT_PREFIX void amp_set_rddq_sdll(uint32_t ch, uint32_t byte, uint32_t rddqs_sdll_ovr_val)
{
	uint32_t rd_clk_dlysel;

	rd_clk_dlysel = rddqcal_encode_dlyval(ch, ((byte == 0) ? AMP_DQ0 : AMP_DQ1), rddqs_sdll_ovr_val);

	// Program RDDLYSEL	
	CSR_WRITE(rAMP_RDCLKDLYSEL(byte, ch), rd_clk_dlysel); 

	// CP : Maui change
	CSR_WRITE(rAMP_DQSDLLCTRL_RD(byte, ch), (rddqs_sdll_ovr_val << 16) | (1 << RUN_SDLLUPDOVR));
	
	// Wait for Run bit to clear
	SPIN_W_TMO_WHILE (CSR_READ(rAMP_DQSDLLCTRL_RD(byte, ch)) & (1 << RUN_SDLLUPDOVR));

}

// This functions set the slave dll for a particular byte lane of RDDQ as specified in the offset parameter
_ROMCODE_SEGMENT_PREFIX void amp_program_rddq_sdll(uint32_t ch, uint32_t byte, uint32_t rddqs_sdll_wr_val)
{
	uint32_t rd_clk_dlysel;

	rd_clk_dlysel = rddqcal_encode_dlyval(ch, ((byte == 0) ? AMP_DQ0 : AMP_DQ1), rddqs_sdll_wr_val);
	//shim_printf("DEBUG - rddqs_sdll_wr_val = %d, Phy = %d, rd_clk_dlysel = %d \n", rddqs_sdll_wr_val, ((byte == 0) ? AMP_DQ0 : AMP_DQ1), rd_clk_dlysel);

	// Program RDDLYSEL	
	CSR_WRITE(rAMP_RDCLKDLYSEL(byte, ch), rd_clk_dlysel); 

	// CP : Maui change
	CSR_WRITE(rAMP_DQSDLLCTRL_RD(byte, ch), (rddqs_sdll_wr_val << 16) | (1 << RUN_SDLLUPDWRRES));
	
	// Wait for Run bit to clear
	SPIN_W_TMO_WHILE (CSR_READ(rAMP_DQSDLLCTRL_RD(byte, ch)) & (1 << RUN_SDLLUPDWRRES));

}

_ROMCODE_SEGMENT_PREFIX void amp_run_rddqcal(uint32_t ch)
{
	// CP : Maui Change
	uint32_t dq_cal_run = CSR_READ(rAMP_DQCALRUN(ch)); 

	CSR_WRITE(rAMP_DQCALRUN(ch), (dq_cal_run & 0xFFFFFFFE) | (1 << 0));
	SPIN_W_TMO_WHILE (CSR_READ(rAMP_DQCALRUN(ch)) & 1);
}

_ROMCODE_SEGMENT_PREFIX void amp_wrlvl_init(void)
{
	uint32_t ch;
	
	for (ch = 0; ch < cfg_params.num_channels; ch++) {
		// Write Leveling Timing Control Registers to program tMRD and tWLO timing params
		CSR_WRITE(rAMP_CAWRLVLTIM(ch), (cfg_params.tWLO << 16) | (cfg_params.tWLMRD << 8) | (cfg_params.tWLDQSEN << 0));
	}
}

_ROMCODE_SEGMENT_PREFIX void amp_wrlvl_entry(uint32_t ch)
{
	uint32_t mr2_data, ca_run_wrlvl;
	uint8_t wrlvl, wls, wl, rl;

	// Setup MRW-1 & MRW-2 for write leveling entry
	// rAMP_CAWRLVLENTRYCMD : (MRW_2[11:0] << 16) | (MRW_1[11:0] <<0)
	// MRW_1[5:0] = {WR_LVL, 5'b00110} = 6'b100110
	// MRW_1[11:6] = MA[5:0] = 6'b000010   
	// MRW_1[11:0] = 12'b000010100110=0x0A6
	// MRW_2[5:0] = {OP[6], 5'b10110} = {mr2_data[6], 5'b10110}
	// MRW_2[11:6] = {OP[5:0]} = {mr2_data[5:0]
	// Hence, rAMP_CAWRLVLENTRYCMD = {(mr2_data[5:0] << 18) | (mr2_data[6] << 17) | (0x16 << 16) | (0x0A6 << 0)  
	wrlvl = 1;
	wls = 0;

	if(cfg_params.freq_bin == 0) {
		if(cfg_params.WL == 14) {
			wls = 0;
		}
		else if(cfg_params.WL == 26) {
			wls = 1;
		}
		wl = 5; // 'b101
		rl = 5; // 'b101
	}
	else {
		if(cfg_params.WL == 8) {
			wls = 0;
		}
		else if(cfg_params.WL == 12) {
			wls = 1;
		}
		wl = 2; // 'b010
		rl = 2; // 'b010
	}

	mr2_data = (wrlvl << 7) | (wls << 6) | (wl << 3) | (rl << 0);  

	CSR_WRITE(rAMP_CAWRLVLENTRYCMD(ch), ((mr2_data & 0x3F) << 22) | (((mr2_data & 0x40)>> 6) << 21) | (0x16 << 16) | (0x0A6 << 0));	


	// <rdar://problem/16027977>
	// DQS IdleActive should be set while write leveling is going on
	amp_set_dqs_idle_active(ch, cfg_params.freq_bin);

	// Set write leveling entry bit
	ca_run_wrlvl = CSR_READ(rAMP_CARUNWRLVL(ch));
	CSR_WRITE(rAMP_CARUNWRLVL(ch), (ca_run_wrlvl & 0xFFFFFFFE) | (1 << RUNWRLVLENTRY_OFFSET)); 

	// Poll for the RUNWRLVLENTRY bit to clear
	SPIN_W_TMO_WHILE (CSR_READ(rAMP_CARUNWRLVL(ch)) & (1 << RUNWRLVLENTRY_OFFSET));
}

_ROMCODE_SEGMENT_PREFIX void amp_wrlvl_exit(uint32_t ch, uint8_t freq_bin)
{
	uint32_t mr2_data, ca_run_wrlvl;
	uint8_t wrlvl, wls, wl, rl;

	// Setup MRW-1 & MRW-2 for write leveling exit
	// rAMP_CAWRLVLEXITCMD : (MRW_2[11:0] << 16) | (MRW_1[11:0] <<0)
	// MRW_1[5:0] = {WR_LVL=0, 5'b00110} = 6'b000110
	// MRW_1[11:6] = MA[5:0] = 6'b000010   
	// MRW_1[11:0] = 12'b000010000110=0x086
	// MRW_2[5:0] = {OP[6], 5'b10110} = {mr2_data[6], 5'b10110}
	// MRW_2[11:6] = {OP[5:0]} = {mr2_data[5:0]
	// Hence, rAMP_CAWRLVLEXITCMD = {(mr2_data[5:0] << 18) | (mr2_data[6] << 17) | (0x16 << 16) | (0x086 << 0)  
	wrlvl = 0;
	wls = 0;

	if(cfg_params.freq_bin == 0) {
		if(cfg_params.WL == 14) {
			wls = 0;
		}
		else if(cfg_params.WL == 26) {
			wls = 1;
		}
		wl = 5; // 'b101
		rl = 5; // 'b101
	}
	else {
		if(cfg_params.WL == 8) {
			wls = 0;
		}
		else if(cfg_params.WL == 12) {
			wls = 1;
		}
		wl = 2; // 'b010
		rl = 2; // 'b010
	}

	mr2_data = (wrlvl << 7) | (wls << 6) | (wl << 3) | (rl << 0);  

	CSR_WRITE(rAMP_CAWRLVLEXITCMD(ch), ((mr2_data & 0x3F) << 22) | (((mr2_data & 0x40)>> 6) << 21) | (0x16 << 16) | (0x086 << 0));	

	// Set write leveling exit bit
	ca_run_wrlvl = CSR_READ(rAMP_CARUNWRLVL(ch));
	CSR_WRITE(rAMP_CARUNWRLVL(ch), (ca_run_wrlvl & 0xFFFFFFF7) | (1 << RUNWRLVLEXIT_OFFSET)); 

	// Poll for the RUNWRLVLEXIT bit to clear
	SPIN_W_TMO_WHILE (CSR_READ(rAMP_CARUNWRLVL(ch)) & (1 << RUNWRLVLEXIT_OFFSET));
	
	// <rdar://problem/16027977>
	// DQS IdleActive should be reset after write leveling is done.
	amp_reset_dqs_idle_active(ch, cfg_params.freq_bin);
}

_ROMCODE_SEGMENT_PREFIX void amp_set_cawrlvl_sdll(uint32_t ch, uint32_t offset)
{
	// Maui changes
	// 1. No need to increment CA Wrlvl SDLL once step at a step.
	// 2. No delay select programming is required
	// 3. No need to force CKE to low. 
	uint32_t cawrlvl_code = CSR_READ(rAMP_CAWRLVLSDLLCODE(ch));
	cawrlvl_code = (cawrlvl_code & WRLVL_SDLLCODE_MASK) | (offset << WRLVL_SDLLCODE_OFFSET);
	
	CSR_WRITE(rAMP_CAWRLVLSDLLCODE(ch), cawrlvl_code | (1 << RUNWRLVL_SDLLUPDOVR));

	// Poll for SDLL to get updated
	SPIN_W_TMO_WHILE (CSR_READ(rAMP_CAWRLVLSDLLCODE(ch)) & (1 << RUNWRLVL_SDLLUPDOVR));
}

_ROMCODE_SEGMENT_PREFIX void amp_set_wrlvlmax_cawrlvl_sdll(uint32_t ch, uint32_t offset)
{
	// Maui changes
	// 1. No need to increment CA Wrlvl SDLL once step at a step.
	// 2. No delay select programming is required
	// 3. No need to force CKE to low. 
	uint32_t cawrlvl_code = CSR_READ(rAMP_CAWRLVLSDLLCODE(ch));
	cawrlvl_code = (cawrlvl_code & WRLVLMAX_SDLLCODE_MASK) | (offset << WRLVLMAX_SDLLCODE_OFFSET);

	CSR_WRITE(rAMP_CAWRLVLSDLLCODE(ch), cawrlvl_code | (1 << RUNWRLVL_SDLLUPDOVR));

	// Poll for SDLL to get updated
	SPIN_W_TMO_WHILE (CSR_READ(rAMP_CAWRLVLSDLLCODE(ch)) & (1 << RUNWRLVL_SDLLUPDOVR));
}

_ROMCODE_SEGMENT_PREFIX void amp_set_dqwrlvl_sdll(uint32_t ch, uint32_t byte, uint32_t offset)
{
	// Maui changes
	// 1. No need to increment CA Wrlvl SDLL once step at a step.
	// 2. No delay select programming is required
	// 3. No need to force CKE to low. 
	uint32_t dqwrlvl_code = CSR_READ(rAMP_DQWRLVLSDLLCODE(byte, ch));
	dqwrlvl_code = (dqwrlvl_code & WRLVL_SDLLCODE_MASK) | (offset << WRLVL_SDLLCODE_OFFSET);
	
	// Byte 0 corresponds to DQ0, Byte 1 corresponds to DQ1
	CSR_WRITE(rAMP_DQWRLVLSDLLCODE(byte, ch), dqwrlvl_code | (1 << RUNWRLVL_SDLLUPDOVR));

	// Poll for SDLL to get updated
	SPIN_W_TMO_WHILE (CSR_READ(rAMP_DQWRLVLSDLLCODE(byte, ch)) & (1 << RUNWRLVL_SDLLUPDOVR));
}

_ROMCODE_SEGMENT_PREFIX void amp_set_wrlvlmax_dqwrlvl_sdll(uint32_t ch, uint32_t byte, uint32_t offset)
{
	// Maui changes
	// 1. No need to increment CA Wrlvl SDLL once step at a step.
	// 2. No delay select programming is required
	// 3. No need to force CKE to low. 
	uint32_t dqwrlvl_code = CSR_READ(rAMP_DQWRLVLSDLLCODE(byte, ch));
	dqwrlvl_code = (dqwrlvl_code & WRLVLMAX_SDLLCODE_MASK) | (offset << WRLVLMAX_SDLLCODE_OFFSET);
	
	// Byte 0 corresponds to DQ0, Byte 1 corresponds to DQ1
	CSR_WRITE(rAMP_DQWRLVLSDLLCODE(byte, ch), dqwrlvl_code | (1 << RUNWRLVL_SDLLUPDOVR));

	// Poll for SDLL to get updated
	SPIN_W_TMO_WHILE (CSR_READ(rAMP_DQWRLVLSDLLCODE(byte, ch)) & (1 << RUNWRLVL_SDLLUPDOVR));
}

// Must ensure that WrLvL SDLLs are stepped by 1 to their final value to avoid glitch on clock signals
_ROMCODE_SEGMENT_PREFIX void amp_program_cawrlvl_sdll(uint32_t ch, uint32_t cawrlvl_offset, uint32_t cawrlvlmax_offset)
{
	// Maui changes
	// 1. No need to increment CA Wrlvl SDLL once step at a step.
	// 2. No delay select programming is required
	// 3. No need to force CKE to low. 
	uint32_t cawrlvl_code = CSR_READ(rAMP_CAWRLVLSDLLCODE(ch));
	//shim_printf("DEBUG - amp_program_cawrlvl_sdll:: cawrlvl_code=0x%x, cawrlvl_code & WRLVL_SDLLCODE_MASK & WRLVLMAX_SDLLCODE_MASK=0x%x , cawrlvl_offset = 0x%x, cawrlvlmax_offset=0x%x \n", cawrlvl_code, cawrlvl_code & WRLVL_SDLLCODE_MASK & WRLVLMAX_SDLLCODE_MASK, cawrlvl_offset, cawrlvlmax_offset);
	cawrlvl_code = (cawrlvl_code & WRLVL_SDLLCODE_MASK & WRLVLMAX_SDLLCODE_MASK) | (cawrlvl_offset << WRLVL_SDLLCODE_OFFSET) | (cawrlvlmax_offset << WRLVLMAX_SDLLCODE_OFFSET);

	//shim_printf("DEBUG - amp_program_cawrlvl_sdll:: cawrlvl_code = 0x%x \n", cawrlvl_code); 

	CSR_WRITE(rAMP_CAWRLVLSDLLCODE(ch), cawrlvl_code | (1 << RUNWRLVL_SDLLUPDWRRES));

	// Poll for SDLL to get updated
	SPIN_W_TMO_WHILE (CSR_READ(rAMP_CAWRLVLSDLLCODE(ch)) & (1 << RUNWRLVL_SDLLUPDWRRES));
}

_ROMCODE_SEGMENT_PREFIX void amp_program_dqwrlvl_sdll(uint32_t ch, uint32_t byte, uint32_t dqwrlvl_offset, uint32_t dqwrlvlmax_offset)
{
	// Maui changes
	// 1. No need to increment CA Wrlvl SDLL once step at a step.
	// 2. No delay select programming is required
	// 3. No need to force CKE to low. 
	uint32_t dqwrlvl_code = CSR_READ(rAMP_DQWRLVLSDLLCODE(byte, ch));
	dqwrlvl_code = (dqwrlvl_code & WRLVL_SDLLCODE_MASK & WRLVLMAX_SDLLCODE_MASK) | (dqwrlvl_offset << WRLVL_SDLLCODE_OFFSET) | (dqwrlvlmax_offset << WRLVLMAX_SDLLCODE_OFFSET);

	//shim_printf("DEBUG - amp_program_dqwrlvl_sdll:: dqwrlvl_code = 0x%x \n", dqwrlvl_code);

	// Byte 0 corresponds to DQ0, Byte 1 corresponds to DQ1
	CSR_WRITE(rAMP_DQWRLVLSDLLCODE(byte, ch), dqwrlvl_code | (1 << RUNWRLVL_SDLLUPDWRRES));

	// Poll for SDLL to get updated
	SPIN_W_TMO_WHILE (CSR_READ(rAMP_DQWRLVLSDLLCODE(byte, ch)) & (1 << RUNWRLVL_SDLLUPDWRRES));
}

_ROMCODE_SEGMENT_PREFIX void amp_run_wrlvlcal(uint32_t ch, uint32_t wrlvlrun)
{
	CSR_WRITE(rAMP_CARUNWRLVL(ch), ((wrlvlrun & 0x3) << RUNWRLVLDQ_OFFSET));
	SPIN_W_TMO_WHILE ((CSR_READ(rAMP_CARUNWRLVL(ch)) >> RUNWRLVLDQ_OFFSET) & 0x3);
}

_ROMCODE_SEGMENT_PREFIX void cacal_init_registers()
{
	uint32_t ch;
	for(ch=0;ch<cfg_params.num_channels;ch++)
	{
		// Bin 1 does not exist in Elba.
		if(cfg_params.freq_bin == 1) {
			// FSP_OP = 0, FSP_WR =1, VRCG = 1
			CSR_WRITE(rAMP_AMPCACALENTRYCMD(ch), (0x276 << 16) | (0x346));
			// FSP_OP = 0, FSP_WR =1, VRCG = 1
			CSR_WRITE(rAMP_AMPCACALEXITCMD(ch), (0x236 << 16) | (0x346));

			// FSP_OP = 1, FSP_WR =0, VRCG = 1
			CSR_WRITE(rAMP_AMPCACALENTRYCMD(ch), ((0x19 << 22) | (0x16 << 16) | (0xd << 6) | (0x26 << 0)));
			// FSP_OP = 1, FSP_WR =0, VRCG = 1
			CSR_WRITE(rAMP_AMPCACALEXITCMD(ch), ((0x18 << 22) | (0x16 << 16) | (0xd << 6) | (0x26 << 0)));
		}
		else {
			// FSP_OP = 0, FSP_WR =1, VRCG = 1
			CSR_WRITE(rAMP_AMPCACALENTRYCMD(ch), ((0x19 << 22) | (0x36 << 16) | (0xd << 6) | (0x6 << 0)));
			// FSP_OP = 0, FSP_WR =1, VRCG = 1
			CSR_WRITE(rAMP_AMPCACALEXITCMD(ch),  ((0x18 << 22) | (0x36 << 16) | (0xd << 6) | (0x6 << 0)));
		}
		CSR_WRITE(rAMP_CACALPATSEED_0(ch), (3 << 24) | (2 << 16) | (1 << 8) | (0 << 0));
		CSR_WRITE(rAMP_CACALPATSEED_1(ch), (5 << 8) | (4 << 0));

	}
}

_ROMCODE_SEGMENT_PREFIX void cacal_generate_patterns_mask(void)
{
	uint32_t index;

	// CP : Maui Change
	// During CA training, if a CA pattern is preceeded and succeeded by pattern_bar,
	// we do not need not mask any result bits

	// generate the pattern to be used for each CA calibration iteration
	for (index = 0; index < (cfg_params.cacalib_sw_loops * cfg_params.cacalib_hw_loops); index++) {
		//patr = (CA_PRBS7_PATTERNS[index]) & CA_ALL_BITS;
		//patf = (CA_PRBS7_PATTERNS[index]) >> CA_NUM_BITS;
		//mask = patr ^ patf;
		//cacal_patterns_mask[index] = mask;
		cacal_patterns_mask[index] = 0x3F;
	}
}

// CP : Maui Change -> Pass an argument which tells if we are in CA or CS training.
//		  Use this argument to update CA or CS delay lines.
// 	  Hardware loop for CS should always be > 1.
_ROMCODE_SEGMENT_PREFIX void cacal_run_sequence(uint32_t ch, uint32_t rnk, uint32_t training_mode, uint32_t mask_bits, uint32_t swloop, uint32_t vref)
{

	uint32_t combined_mask, hwloop;
	uint32_t pat_mask = 0;
	
	for (hwloop = 0; hwloop < cfg_params.cacalib_hw_loops; hwloop++)
		pat_mask |= cacal_patterns_mask[(swloop * cfg_params.cacalib_hw_loops) + hwloop];
	
	// This represents the bits that don't have a transition on any of the patterns used during the hwloop calibration
	// PRBS4I pattern guarantees that there would be a 1->0 and 0->1 transition on every CA bit. Hence the combined
	// mask can be set to 0x0
	combined_mask = 0;

	
	// To find the FAIL <-> PASS <-> FAIL window
	cacal_find_right_failing_point(ch, rnk, training_mode, combined_mask, swloop, vref);
	cacal_find_left_failing_point(ch, rnk, training_mode, combined_mask, swloop, vref);
}

// Establish the right edge of the window by finding the point where all CA bits fail
_ROMCODE_SEGMENT_PREFIX void cacal_find_right_failing_point(uint32_t ch, uint32_t rnk, uint32_t training_mode, uint32_t combined_mask, uint32_t swloop, uint32_t vref)
{
	bool all_bits_fail = false;
	uint32_t cacalresult = CA_ALL_BITS;

	// With an SDLL code of '0', CA is centered wrt CK. So to get to the right failing point, push CK by 180 degrees
	// Increment CKSDLL & CSSDLL by 2*MDLL. In CA training, initial values of CKSDLL & CSSDLL would be the final CS center point values. 
	uint32_t push_ck_out = 2 * mdllcode[ch][AMP_CA] + cksdll_cscal[vref];
	uint32_t push_cs_out = 2 * mdllcode[ch][AMP_CA] + cssdll_cscal[vref];
	uint32_t push_ca_out = 2 * mdllcode[ch][AMP_CA] + casdll_cscal[vref];

	uint32_t max_caright_point_val;
	uint32_t max_caright_point_reached;
	uint32_t step_size = COARSE_STEP_SZ;

	// For Max skew slow corner, SDLL may have to go upto 3*MDLL+1.  
	max_caright_point_val = (3 * mdllcode[ch][AMP_CA]) + 1;
	max_caright_point_reached = 0;

	// If we skip CS training, clear CS & CA deskew registers during CA training
	if(training_mode == CS_TRAINING) {
		amp_init_ca_deskew(ch);
	}

	// Increase delay to the right until all bits fail
	do {
		// Update CK signal delay
		//shim_printf("DEBUG - cacal_find_right_failing_point:: Programming a value of 0x%x into CKSDLL \n",push_ck_out);
		amp_push_cksdll_out(ch, push_ck_out);

		// CS training would align CK and CS correctly. To maintain that alignment, CK and CS should be delayed by the same amount
		// during CA training
		if(training_mode == CA_TRAINING) {
			//shim_printf("DEBUG - cacal_find_right_failing_point:: Programming a value of 0x%x into CSSDLL \n",push_cs_out);
			amp_push_cssdll_out(ch, push_cs_out);
		}			
		// Maintain CK and CA alignment during fine tuned CS training.
		else if(training_mode == CS_FINE_TRAINING) {
			//shim_printf("DEBUG - cacal_find_right_failing_point:: Programming a value of 0x%x into CASDLL \n",push_ca_out);
			amp_push_casdll_out(ch, push_ca_out);
		}			

		cacalresult = cacalresult & amp_run_cacal(ch, training_mode);
		//shim_printf("DEBUG - cacal_find_right_failing_point:: CA cal result is 0x%x \n",cacalresult);
		
		if ((cacalresult & (CA_ALL_BITS ^ combined_mask)) != 0) {
			all_bits_fail = false;

			// If the next increment of tap value is over the maximum allowed tap value, switch
			// from coarse stepping to fine stepping.
			if(((push_ck_out + step_size) > max_caright_point_val) || ((push_cs_out + step_size) > max_caright_point_val) || ((push_ca_out + step_size) > max_caright_point_val)) {
				step_size = FINER_STEP_SZ;
			}

			push_ck_out = push_ck_out + step_size;
			if(training_mode == CA_TRAINING) {
				push_cs_out = push_cs_out + step_size;
			}
			else if(training_mode == CS_FINE_TRAINING) {
				push_ca_out = push_ca_out + step_size;
			}

		} else {
			all_bits_fail = true;
			//shim_printf("DEBUG - cacal_find_right_failing_point:: All bits failed. Moved on to cacal_find_right_passing_point \n");
			
			// Do a per bit calculation of when they start passing again
			cacal_find_right_passing_point(ch, rnk, training_mode, combined_mask, swloop, vref);
			break;
		}
		
		max_caright_point_reached = (training_mode == CA_TRAINING) ? ((push_ck_out > max_caright_point_val) || (push_cs_out > max_caright_point_val))
																					  : (training_mode ==  CS_FINE_TRAINING) ? ((push_ck_out > max_caright_point_val) || (push_ca_out > max_caright_point_val)) : (push_ck_out > max_caright_point_val); 	
	} while (max_caright_point_reached == 0);
	
	if (all_bits_fail == false) {
		//shim_panic("Memory CA calibration: Unable to find right side failing point for channel %d\n", ch);
		shim_printf("PANIC during Vref scan. Record and move forward:Memory CA calibration: Unable to find right side failing point for channel %d\n", ch);
		cacal_vref_panic[ch][vref] = 1;
	}
	
		// Ok from Rakesh to set to cksdll_cscal directly instead of decrementing by 1
		amp_push_cksdll_out(ch, cksdll_cscal[vref]);

		if(training_mode == CA_TRAINING) {
			amp_push_cssdll_out(ch, cssdll_cscal[vref]);
		}			
		else if(training_mode == CS_FINE_TRAINING) {
			amp_push_casdll_out(ch, casdll_cscal[vref]);
		}
}

// Finds the passing region on the right edge of window
_ROMCODE_SEGMENT_PREFIX void cacal_find_right_passing_point(uint32_t ch, uint32_t rnk, uint32_t training_mode, uint32_t combined_mask, uint32_t swloop, uint32_t vref)
{
	bool switch_from_cktoca;
	uint32_t tap_value, tap_value_cs, tap_value_ca;
	uint32_t cacalresult;
	uint32_t saved_val;
	uint32_t all_bits_pass;
	uint32_t BitPass[CA_NUM_BITS] = { 0 };
	uint32_t SolidBitPass[CA_NUM_BITS] = { 0 };
	uint32_t bit_indx;
	uint32_t ckdeskew, cksdll, cssdll, csdeskew, casdll;
	uint32_t loopchrnk_indx;
	//uint32_t casdllcode_casdllovrval;
		
	all_bits_pass = 0;
	ckdeskew = CSR_READ(rAMP_CKDESKEW_CTRL(ch));
	csdeskew = CSR_READ(rAMP_CSDESKEW_CTRL(ch));
	cksdll = ((CSR_READ(rAMP_CKSDLLCTRL(ch)) >> SDLLOFFSET) & DLLVAL_BITS);	
	cssdll = ((CSR_READ(rAMP_CSSDLLCTRL(ch)) >> SDLLOFFSET) & DLLVAL_BITS);	
	casdll = ((CSR_READ(rAMP_CASDLLCTRL(ch)) >> SDLLOFFSET) & DLLVAL_BITS);	

	// For every swloop, we'll save passing values for each channel & rank
	loopchrnk_indx = loopchrnk_index_calc(ch, rnk, swloop);
	
	// CP : Maui Change
	// CK SDLL will always be > 0. Also, given the range of CK SDLL, i assume that we never had to
	//	increment CK Deskew to find the right side failing point.
	tap_value = cksdll;	
	tap_value_cs = cssdll;
	tap_value_ca = casdll;
	switch_from_cktoca = false;

	// combined_mask contains don't care bits (due to pattern) or masked bits (MR41 or MR48), so consider those done
	for (bit_indx = 0; bit_indx < CA_NUM_BITS; bit_indx++)
		if ((combined_mask & (1 << bit_indx)) != 0)
			BitPass[bit_indx] = 1;
	
	// Finding Right side passing point on per bit level. Moving Right to Left to find point where it turns from FAIL TO PASS
	do {
		if (switch_from_cktoca == false) {
				
			// Update CK signal delays
			//shim_printf("DEBUG - cacal_find_right_passing_point:: Programming a value of 0x%x into CKSDLL \n",tap_value);
			amp_push_cksdll_out(ch, tap_value);
	
			// CS training would align CK and CS correctly. To maintain that alignment, CK and CS should be delayed by the same amount
			// during CA training
			if(training_mode == CA_TRAINING) {
				amp_push_cssdll_out(ch, tap_value_cs);
			}
			else if(training_mode == CS_FINE_TRAINING) {
				amp_push_casdll_out(ch, tap_value_ca);
			}
		} else {
			if(training_mode == CA_TRAINING) {
				amp_push_casdll_out(ch, tap_value);
			}
			if((training_mode == CS_TRAINING) || (training_mode == CS_FINE_TRAINING)) {
				amp_push_cssdll_out(ch, tap_value);
			}
		}
			
		// Run the ca/cs calibration in hw
		cacalresult = amp_run_cacal(ch, training_mode);
		
		//shim_printf("DEBUG - cacal_find_right_passing_point:: CA cal result is 0x%x \n",cacalresult);
				
		// Make sure that each Bit sees a transition from 0 to 1 on CaCalresult Register
		for (bit_indx = 0; bit_indx < CA_NUM_BITS; bit_indx++) {
			// For bits that are not masked, need to check pass/fail
			if ((combined_mask & (1 << bit_indx)) == 0) {
				if ((BitPass[bit_indx] == 0) && ((cacalresult & (1 << bit_indx)) != 0)) {
					if (SolidBitPass[bit_indx] == SOLID_PASS_DETECT) {
						// Bit has passed for SOLID_PASS_DETECT number of times, consider it done.
						BitPass[bit_indx] = 1;
					} else if (SolidBitPass[bit_indx] > 0) {
						SolidBitPass[bit_indx] = SolidBitPass[bit_indx] + 1;
					} else {
						// This is the first time this bit has passed, save this point in the array
						SolidBitPass[bit_indx] = SolidBitPass[bit_indx] + 1;
						saved_val = tap_value;
						if (switch_from_cktoca == false) {
							if(training_mode == CA_TRAINING) {
								cacal_per_loopchrnk_right[loopchrnk_indx][vref][bit_indx] = saved_val;
								//shim_printf("DEBUG - cacal_find_right_passing_point:: Found CA right passing point for vref=%d, loopchrnk_indx=%d, bit_indx=%d, saved_val = %d \n",vref,loopchrnk_indx,bit_indx,cacal_per_loopchrnk_right[loopchrnk_indx][vref][bit_indx]);
							}
							if((training_mode == CS_TRAINING) || (training_mode == CS_FINE_TRAINING)) {
								// Subtract cssdll to take into account the case where CS sdll is non-zero, before starting CS calibration.
								cscal_per_loopchrnk_right[loopchrnk_indx] = saved_val - cssdll;
								//shim_printf("DEBUG - cacal_find_right_passing_point:: Found CS right passing point for loopchrnk_indx=%d, saved_val = %d \n",loopchrnk_indx,cscal_per_loopchrnk_right[loopchrnk_indx]); 
							}
						} else {
							if(training_mode == CA_TRAINING) {
								cacal_per_loopchrnk_right[loopchrnk_indx][vref][bit_indx] = -1 * saved_val;
								//shim_printf("DEBUG - cacal_find_right_passing_point:: Found CA right passing point for vref=%d, loopchrnk_indx=%d, bit_indx=%d, saved_val = %d \n",vref,loopchrnk_indx,bit_indx,cacal_per_loopchrnk_right[loopchrnk_indx][vref][bit_indx]);
							}
							if((training_mode == CS_TRAINING) || (training_mode == CS_FINE_TRAINING)) {
								// Subtract cksdll to take into account the case where CK sdll is non-zero, before starting CS calibration.
								cscal_per_loopchrnk_right[loopchrnk_indx] = -1 * (saved_val - cksdll_cscal[vref]);
								//shim_printf("DEBUG - cacal_find_right_passing_point:: Found CS right passing point for loopchrnk_indx=%d, saved_val = %d \n",loopchrnk_indx,cscal_per_loopchrnk_right[loopchrnk_indx]); 
							}
						}
					}
				} else {
					// Bit failed to pass calibration, reset the SolidBitPass value to 0
					SolidBitPass[bit_indx] = 0;
				}
			}
		}
		
		all_bits_pass = 1;
		for (bit_indx = 0; bit_indx < CA_NUM_BITS; bit_indx++) {
			all_bits_pass = all_bits_pass & BitPass[bit_indx];
			if (all_bits_pass == 0)
				break;
		}		

		// If ALL bits are not passing - keep moving ca/cs signals from Right to Left
		if (all_bits_pass == 0) {
			// CP : Maui Change
			if ((tap_value == cksdll_cscal[vref]) && (switch_from_cktoca == false)) {
				switch_from_cktoca = true;
			
				// Make tap_value '0' just in case cksdll_cscal > 0
				tap_value = 0;
			}
			
			if (switch_from_cktoca == false) {
				tap_value = tap_value - FINER_STEP_SZ;
				if(training_mode == CA_TRAINING) {
					tap_value_cs = tap_value_cs - FINER_STEP_SZ;
				}
				else if(training_mode == CS_FINE_TRAINING) {
					tap_value_ca = tap_value_ca - FINER_STEP_SZ;
				}

			} else {
				tap_value = tap_value + FINER_STEP_SZ;
			}
		}
		
	// For Max skew slow corner, SDLL may have to go upto 3*MDLL+1.  
	} while ((tap_value <= (3 * mdllcode[ch][AMP_CA] + 1)) && (all_bits_pass == 0));
	
	if (all_bits_pass == 0) {
		//shim_panic("Memory CA calibration: Unable to find passing point for all bits on the right side \n");
		shim_printf("PANIC during Vref scan.Record and move forward:Memory CA calibration: Unable to find passing point for all bits on the right side \n");
		cacal_vref_panic[ch][vref] = 1;
	}
}

_ROMCODE_SEGMENT_PREFIX void cacal_find_left_failing_point(uint32_t ch, uint32_t rnk, uint32_t training_mode, uint32_t combined_mask, uint32_t swloop, uint32_t vref)
{
	// At this point, we've already played with all possible CK delays. At the end of cacal_find_right_failing_point routine,
	// we reset the CK delays to 0.
	// Loop through CaSDLLOvrVal from -MasterDLL to +Max until all failing points on the left side are found

	uint32_t all_bits_fail;
	uint32_t push_ca_out;
	uint32_t push_cs_out;
	uint32_t cacalresult;
	uint32_t max_caleft_point_reached;
	uint32_t max_casdll_reached;
	uint32_t max_caleft_point_val;
	uint32_t casdllcode, ca0deskewctrl;
	uint32_t cssdllcode, csdeskewctrl;
	uint32_t step_size = COARSE_STEP_SZ;
		
	all_bits_fail = 0;
	cacalresult = CA_ALL_BITS;
	max_caleft_point_reached = 0;
	max_casdll_reached = 0;
	
	// For Max skew slow corner, SDLL may have to go upto 3*MDLL+1.  
	max_caleft_point_val = (3 * mdllcode[ch][AMP_CA] + 1);
	
	casdllcode = ((CSR_READ(rAMP_CASDLLCTRL(ch)) >> SDLLOFFSET) & DLLVAL_BITS);
	ca0deskewctrl = CSR_READ(rAMP_CADESKEW_CTRL(ch, 0));
	
	cssdllcode = ((CSR_READ(rAMP_CSSDLLCTRL(ch)) >> SDLLOFFSET) & DLLVAL_BITS);
	csdeskewctrl = CSR_READ(rAMP_CSDESKEW_CTRL(ch));

	push_ca_out = casdllcode + ca0deskewctrl;
	push_cs_out = cssdllcode + csdeskewctrl;

	// Increment CaSDLL/CsSDLL from 0 to max_caleft_point_reached
	do {

		if(training_mode == CA_TRAINING) {
			// Push out this new ca offset
			//shim_printf("DEBUG - cacal_find_left_failing_point:: Pushing out CASDLL by 0x%x taps \n",push_ca_out);
			// Push out this new ca offset
			amp_push_casdll_out(ch, push_ca_out);
		} 
		if((training_mode == CS_TRAINING) || (training_mode == CS_FINE_TRAINING)) {
			// Push out this new cs offset
			//shim_printf("DEBUG - cacal_find_left_failing_point:: Pushing out CSSDLL by 0x%x taps \n",push_cs_out);
			amp_push_cssdll_out(ch, push_cs_out);
		}

		// Run the CA/CS calibration
		cacalresult = amp_run_cacal(ch, training_mode);
		
		//shim_printf("DEBUG - cacal_find_left_failing_point:: CA cal result is 0x%x \n",cacalresult);

		// combined mask has don't care bits (based on pattern) and masked bits (based on MR41 or MR48) that we should ignore
		if ((cacalresult & (CA_ALL_BITS ^ combined_mask)) != 0) {
			all_bits_fail = 0;
		} else {
			all_bits_fail = 1;
			
			// Now, we have found the left edge of window. Find the passing point for all bits
			cacal_find_left_passing_point(ch, rnk, training_mode, combined_mask, swloop, vref);
			break;
		}
		
		if (all_bits_fail == 0) {
			if(training_mode == CA_TRAINING) {

				// If the next increment of tap value is over the maximum allowed tap value, switch
				// from coarse stepping to fine stepping.
				if(push_ca_out + step_size > max_caleft_point_val) {
					step_size = FINER_STEP_SZ;
				}

				push_ca_out = push_ca_out + step_size;
				
				if ((push_ca_out > max_caleft_point_val) && (all_bits_fail == 0)) {
					//shim_panic("Memory CA calibration: Unable to find failing point for all bits on the left side");
					shim_printf("PANIC during Vref scan. Record and move forward:Memory CA calibration: Unable to find failing point for all bits on the left side");
					cacal_vref_panic[ch][vref] = 1;
				}
			} 
			if ((training_mode == CS_TRAINING) || (training_mode == CS_FINE_TRAINING)) {
				if(push_cs_out + step_size > max_caleft_point_val) {
					step_size = FINER_STEP_SZ;
				}

				push_cs_out = push_cs_out + step_size;

				if ((push_cs_out > max_caleft_point_val) && (all_bits_fail == 0)) {
					//shim_panic("Memory CS calibration: Unable to find failing point for all bits on the left side");
					shim_printf("PANIC during Vref scan. Record and move forward:Memory CS calibration: Unable to find failing point for all bits on the left side");
					cacal_vref_panic[ch][vref] = 1;
				}
			}
		}
		if(training_mode == CA_TRAINING) {
			if (push_ca_out > max_caleft_point_val) {
				max_caleft_point_reached = 1;
			}
		} 
		if ((training_mode == CS_TRAINING) || (training_mode == CS_FINE_TRAINING)) {
			if (push_cs_out > max_caleft_point_val) {
				max_caleft_point_reached = 1;
			}
		}

		// Panic if there are no more sdll taps left to proceed ahead
		if (max_caleft_point_reached && (all_bits_fail == 0))
		{
			//shim_panic("Memory CA calibration: SDLL ran out of taps when trying to find left side failing point\n");
			shim_printf("PANIC during Vref scan. Record and move forward:Memory CA calibration: SDLL ran out of taps when trying to find left side failing point\n");
			cacal_vref_panic[ch][vref] = 1;

		}
	} while ((all_bits_fail == 0) && (max_caleft_point_reached == 0));
}

_ROMCODE_SEGMENT_PREFIX void cacal_find_left_passing_point(uint32_t ch, uint32_t rnk, uint32_t training_mode, uint32_t combined_mask, uint32_t swloop, uint32_t vref)
{
	uint32_t loopchrnk_indx;
	uint32_t BitPass[CA_NUM_BITS] = { 0 };
	uint32_t SolidBitPass[CA_NUM_BITS] = { 0 };
	uint32_t tap_value;
	uint32_t cacalresult;
	uint32_t camdllcode;
	uint32_t casdllcode;
	uint32_t ca0deskewctrl;
	uint32_t cssdllcode;
	uint32_t csdeskew;
	uint32_t saved_val;
	uint32_t all_bits_pass;
	uint32_t bit_indx;
	
	loopchrnk_indx = loopchrnk_index_calc(ch, rnk, swloop);
	
	casdllcode = ((CSR_READ(rAMP_CASDLLCTRL(ch)) >> SDLLOFFSET) & DLLVAL_BITS);
	ca0deskewctrl = CSR_READ(rAMP_CADESKEW_CTRL(ch, 0));
	camdllcode = mdllcode[ch][AMP_CA];
	csdeskew = CSR_READ(rAMP_CSDESKEW_CTRL(ch));
	cssdllcode = ((CSR_READ(rAMP_CSSDLLCTRL(ch)) >> SDLLOFFSET) & DLLVAL_BITS);	
	
	all_bits_pass = 0;
	
	// CP : Maui Change
	if(training_mode == CA_TRAINING) {
		tap_value = casdllcode;
	} 
	if ((training_mode == CS_TRAINING) || (training_mode == CS_FINE_TRAINING)) {
		tap_value = cssdllcode;
	}

	// combined_mask contains don't care bits (due to pattern) or masked bits (MR41 or MR48), so consider those passed
	for (bit_indx = 0; bit_indx < CA_NUM_BITS; bit_indx++)
		if ((combined_mask & (1 << bit_indx)) != 0)
			BitPass[bit_indx] = 1;

	
	// Finding Left side passing point on per bit level. Move Left to Right to find point where it turns from FAIL TO PASS
	do {
		// Push out this new CA/CS offset
		if(training_mode == CA_TRAINING) {
			// Push out this new ca offset
			amp_push_casdll_out(ch, tap_value);
		}
		if ((training_mode == CS_TRAINING) || (training_mode == CS_FINE_TRAINING)) {
			// Push out this new cs offset
			//shim_printf("DEBUG - cacal_find_left_passing_point:: Pushing out CSSDLL by 0x%x taps \n",tap_value);
			amp_push_cssdll_out(ch, tap_value);
		}
		
		// Run the CA/CS calibration
		cacalresult = amp_run_cacal(ch, training_mode);

		//shim_printf("DEBUG - cacal_find_left_passing_point:: CA cal result is 0x%x \n",cacalresult);

		// Make sure that each Bit sees a transition from 0 to 1 on CaCalresult Register
		for (bit_indx=0; bit_indx < CA_NUM_BITS; bit_indx++) {
			// check pass/fail for bits not masked
			if ((combined_mask & (1 << bit_indx)) == 0) {
				if ((BitPass[bit_indx] == 0) && ((cacalresult & (1 << bit_indx)) != 0)) {
					if (SolidBitPass[bit_indx] == SOLID_PASS_DETECT) {
						// bit has passed SOLID_PASS_DETECT straight times, consider it done
						BitPass[bit_indx] = 1;
					} else if (SolidBitPass[bit_indx] > 0) {
						SolidBitPass[bit_indx] = SolidBitPass[bit_indx] + 1;
					} else {
						// first time bit has passed, record this value
						SolidBitPass[bit_indx] = SolidBitPass[bit_indx] + 1;
						saved_val = tap_value;
						if(training_mode == CA_TRAINING) {
							// Effective CA tap value would be saved_val - (CK SDLL value after CS calibration).
							cacal_per_loopchrnk_left[loopchrnk_indx][vref][bit_indx] = -1 * (saved_val - cksdll_cscal[vref]) ;
							//shim_printf("DEBUG - cacal_find_left_passing_point:: Found CA left passing point for vref=%d, loopchrnk_indx=%d, bit_indx=%d, saved_val = %d \n",vref,loopchrnk_indx,bit_indx,cacal_per_loopchrnk_left[loopchrnk_indx][vref][bit_indx]);
						}
						if ((training_mode == CS_TRAINING) || (training_mode == CS_FINE_TRAINING)) {
							// Effective CS tap value would be saved_val - (CK SDLL value after CS calibration).
							cscal_per_loopchrnk_left[loopchrnk_indx] = -1 * (saved_val - cksdll_cscal[vref]);
							//shim_printf("DEBUG - cacal_find_left_passing_point:: Found CS left passing point for loopchrnk_indx=%d, saved_val = %d \n",loopchrnk_indx,cscal_per_loopchrnk_left[loopchrnk_indx]); 
						}
					}
				} else {
					// bit failed calibration, reset the SolidBitPass value back to 0
					SolidBitPass[bit_indx] = 0;
				}
			}
		}
		
		all_bits_pass = 1;
		for (bit_indx=0; bit_indx < CA_NUM_BITS; bit_indx++) {
			all_bits_pass = all_bits_pass & BitPass[bit_indx];
			if (all_bits_pass == 0)
				break;
		}
		
		// TODO TODO: Make (tap_value < FINER_STEP_SZ) change at other places too
		if ((tap_value < FINER_STEP_SZ) && (all_bits_pass == 0)) {
			// print error message as Left Passing Point cannot be found
			//shim_panic("Memory CA calibration: Unable to find passing point for all bits on the left side \n");
			shim_printf("PANIC during Vref scan. Record and move forward:Memory CA calibration: Unable to find passing point for all bits on the left side \n");
			cacal_vref_panic[ch][vref] = 1;
			break;
		}
		
		// If ALL bits are not passing - keep moving from Left to Right Side of window
		if (all_bits_pass == 0) {
			tap_value -= FINER_STEP_SZ;
		}
		
	// For Max skew slow corner, SDLL may have to go upto 3*MDLL+1.  
	} while ((tap_value <= (3 * mdllcode[ch][AMP_CA] + 1)) && (all_bits_pass == 0));
}

_ROMCODE_SEGMENT_PREFIX void cacal_program_final_values(void)
{
	uint32_t ch;
	uint32_t bit_indx;
	int32_t arr_CaBitCenterPoint[CA_NUM_BITS];
	//uint32_t arr_CaBitDeSkew[CA_NUM_BITS];
	int32_t tmp_left_pos_val, tmp_right_pos_val;
	int32_t left_pos_val;
	int32_t right_pos_val;
	uint32_t camdllcode;
	int32_t max_CaBitCenterPoint_val;
	int32_t ckcs_diff,cabit_deskew;
	uint32_t casdllcode;
	uint8_t byte;
	uint32_t freqchngctl1;
	
	int32_t rank_val[AMP_MAX_RANKS_PER_CHAN];
	uint32_t swloop;
	uint32_t loopchrnk0_indx, chrnk0_indx;
	uint32_t vref_ch;
#if ENV_DV
	uint32_t loopchrnk000 = loopchrnk_index_calc(0, 0, 0);
	uint32_t vref0 = vref_opt_ca[loopchrnk000];

	shim_printf("DEBUG - cacal_program_final_values:: Preloading non-calibrated channels with channel 0's calibration results \n");
	for(ch = cfg_params.num_channels_runCalib; ch < cfg_params.num_channels; ch++) {
		if (ch == cfg_params.num_channels_runCalib)
			shim_printf("DEBUG - %s:: Preloading non-calibrated channels "
				    "with channel 0's calibration results \n", __PRETTY_FUNCTION__);
		// For channels on which calibration was not done, load calibration results from channel 0.
		loopchrnk0_indx = loopchrnk_index_calc(ch, 0, 0);
		vref_ch = vref_opt_ca[loopchrnk0_indx] = vref0; 
	
		//shim_printf("DEBUG - cacal_program_final_values:: loopchrnk0_indx = %d, vref_ch=%d \n", loopchrnk0_indx, vref_ch); 
	
		for (bit_indx=0; bit_indx < CA_NUM_BITS; bit_indx++) {
			cacal_per_loopchrnk_left[loopchrnk0_indx][vref_ch][bit_indx] = cacal_per_loopchrnk_left[loopchrnk000][vref0][bit_indx];
			cacal_per_loopchrnk_right[loopchrnk0_indx][vref_ch][bit_indx] = cacal_per_loopchrnk_right[loopchrnk000][vref0][bit_indx];

			//shim_printf("DEBUG - cacal_program_final_values:: cacal_per_loopchrnk_left[loopchrnk0_indx][vref_ch][bit_indx] = %d \n", cacal_per_loopchrnk_left[loopchrnk0_indx][vref_ch][bit_indx]);
			//shim_printf("DEBUG - cacal_program_final_values:: cacal_per_loopchrnk_right[loopchrnk0_indx][vref_ch][bit_indx] = %d \n", cacal_per_loopchrnk_right[loopchrnk0_indx][vref_ch][bit_indx]);
		}

		cksdll_cscal[vref_ch] = cksdll_cscal[vref0];
		cssdll_cscal[vref_ch] = cssdll_cscal[vref0];


		//shim_printf("DEBUG - cacal_program_final_values:: cksdll_cscal[vref_opt_ca[loopchrnk0_indx]] = %d, cssdll_cscal[vref_opt_ca[loopchrnk0_indx]] = %d \n", cksdll_cscal[vref_opt_ca[loopchrnk0_indx]], cssdll_cscal[vref_opt_ca[loopchrnk0_indx]] );

	}
#endif

	
	for (ch = 0; ch < cfg_params.num_channels; ch++) {
		casdllcode = ((CSR_READ(rAMP_CASDLLCTRL(ch)) >> SDLLOFFSET) & DLLVAL_BITS);
		camdllcode = mdllcode[ch][AMP_CA];
		
		chrnk0_indx = loopchrnk_index_calc(ch, 0, 0);
		vref_ch = vref_opt_ca[chrnk0_indx];
		// Calculate the Center Points for each CA bit
		for (bit_indx=0; bit_indx < CA_NUM_BITS; bit_indx++) {
			//comb_mask = 0x0;
			//mask_txn_detect = 0x0;
			//tmp_mask = 0x0;
			
			// Compute the aggr eye over multiple swloop and hwloop for all ranks
			for (swloop = 0; swloop < cfg_params.cacalib_sw_loops; swloop++) {
				loopchrnk0_indx = loopchrnk_index_calc(ch, 0, swloop);
				//mask = 0x0;
				//for (hwloop=0; hwloop < cfg_params.cacalib_hw_loops; hwloop++)
				//	mask = mask | cacal_patterns_mask[(swloop * cfg_params.cacalib_hw_loops) + hwloop];
					
				// An explanation of the masks is below. Note that we only recorded result for a bit from a particular iteration if the bit had a transition.
				// mask: for pattern(s) sent in this swloop, indicates if the bit had a transition
				// tmp_mask: aggregates mask over all loops, including current swloop
				// comb_mask: aggregates mask over all loops, upto the last iteration of the swloop. After it is used to generate mask_txn_detect, it catches upto same value as tmp_mask
				// mask_txn_detect: indicates the first time a bit transitioned was in this swloop
				//tmp_mask = tmp_mask | mask;
				//mask_txn_detect = tmp_mask ^ comb_mask;
				//comb_mask = comb_mask | mask;
				
				/* 
				 * Rank 0
				 */
				
				/* Left side */
				
				// lookup the value in the left side for this bit given loop, ch, and rnk
				rank_val[0] = cacal_per_loopchrnk_left[loopchrnk0_indx][vref_ch][bit_indx];
				tmp_left_pos_val = rank_val[0];
					
				// If this is the first time this bit transitioned, just put it in the aggregate result array
				//if (mask_txn_detect & (1 << bit_indx)) {
				//	left_pos_val = tmp_left_pos_val;
				//	cacal_per_chrnk_left[chrnk0_indx][bit_indx] = left_pos_val;
				//} else if (mask & (1 << bit_indx)) {
				//	// This is not the 1st time this bit transitioned so there is a recorded result already, but since we have a new result
				//	// to compare with, find the value that would cover both points and put that in the array
				//	left_pos_val = cacal_per_chrnk_left[chrnk0_indx][bit_indx];
				//	left_pos_val = find_common_endpoint(tmp_left_pos_val, left_pos_val, MIN_ENDPT);
				//	cacal_per_chrnk_left[chrnk0_indx][bit_indx] = left_pos_val;
				//} else {
				//	// <rdar://problem/13834199>
				//	left_pos_val = tmp_left_pos_val;
				//	cacal_per_chrnk_left[chrnk0_indx][bit_indx] = left_pos_val;
				//}
				
				if(swloop == 0) {
					left_pos_val = tmp_left_pos_val;
					cacal_per_chrnk_left[chrnk0_indx][bit_indx] = left_pos_val;
				} else {
					// This is not the 1st time this bit transitioned so there is a recorded result already, but since we have a new result
					// to compare with, find the value that would cover both points and put that in the array
		 			left_pos_val = cacal_per_chrnk_left[chrnk0_indx][bit_indx];
					left_pos_val = find_common_endpoint(tmp_left_pos_val, left_pos_val, MAX_ENDPT);
					cacal_per_chrnk_left[chrnk0_indx][bit_indx] = left_pos_val;
				}

				
				/* Right side */
				
				// lookup the value in the right side for this bit given loop, ch, and rnk
				rank_val[0] = cacal_per_loopchrnk_right[loopchrnk0_indx][vref_ch][bit_indx];
				tmp_right_pos_val = rank_val[0];
					
				// If this is the first time this bit transitioned, just put it in the aggregate result array
				//if (mask_txn_detect & (1 << bit_indx)) {
				//	right_pos_val = tmp_right_pos_val;
				//	cacal_per_chrnk_right[chrnk0_indx][bit_indx] = right_pos_val;
				//} else if (mask & (1 << bit_indx)) {
				//	// This is not the 1st time this bit transitioned so there is a recorded result already, but since we have a new result
				//	// to compare with, find the value that would cover both points and put that in the array
				//	right_pos_val = cacal_per_chrnk_right[chrnk0_indx][bit_indx];
				//	right_pos_val = find_common_endpoint(tmp_right_pos_val, right_pos_val, MIN_ENDPT);
				//	cacal_per_chrnk_right[chrnk0_indx][bit_indx] = right_pos_val;
				//} else {
				//	// <rdar://problem/13834199>
				//	right_pos_val = tmp_right_pos_val;
				//	cacal_per_chrnk_right[chrnk0_indx][bit_indx] = 0;
				//}
			
				if(swloop == 0) {
					right_pos_val = tmp_right_pos_val;
					cacal_per_chrnk_right[chrnk0_indx][bit_indx] = right_pos_val;
				} else {
					// This is not the 1st time this bit transitioned so there is a recorded result already, but since we have a new result
					// to compare with, find the value that would cover both points and put that in the array
		 			right_pos_val = cacal_per_chrnk_right[chrnk0_indx][bit_indx];
					right_pos_val = find_common_endpoint(tmp_right_pos_val, right_pos_val, MIN_ENDPT);
					cacal_per_chrnk_right[chrnk0_indx][bit_indx] = right_pos_val;
				}
				
			}
			
			// At this point, the left edge and the right edge of the eye for this channel and bit are defined by left_pos_val and right_pos_val
			// Find the center of the eye
			arr_CaBitCenterPoint[bit_indx] = find_center_of_eye(cacal_per_chrnk_left[chrnk0_indx][bit_indx],cacal_per_chrnk_right[chrnk0_indx][bit_indx]);
		}
		
		// Since center for each bit may be different, find the max val
		// Max val will get programmed to the ckdll (if Max >=0) or to casdll (if Max <0), while the other bits will require deskew
		max_CaBitCenterPoint_val = arr_CaBitCenterPoint[0];

		for (bit_indx = 1; bit_indx < CA_NUM_BITS; bit_indx++) {
			
			if(arr_CaBitCenterPoint[bit_indx] > max_CaBitCenterPoint_val) {
				max_CaBitCenterPoint_val = arr_CaBitCenterPoint[bit_indx];
			}
		}
		
		/*
		 * Finally, program the values
		 * Adjust CKSDLL, CSSDLL and CASDLL such that 
		 	1. Their relative difference is maintained
			2. None of the SDLL values are negative
		*/
		// Calculate our delay point for CK SDLL and CS SDLL
		ckcs_diff = cksdll_cscal[vref_ch] - cssdll_cscal[vref_ch];

		if(max_CaBitCenterPoint_val >= 0) {
			if (ckcs_diff > 0) {
				if(max_CaBitCenterPoint_val > ckcs_diff) {
					amp_program_ck_sdll(ch, max_CaBitCenterPoint_val);
					
					// Since CK SDLL is moved, also program DQS WRLVL SDLL here 
					for(byte = 0 ; byte < DQ_NUM_BYTES; byte++ ) {
						dqwrlvl_sdll_before_wrlvl[ch][byte] = max_CaBitCenterPoint_val;
						amp_program_dqwrlvl_sdll(ch, byte, dqwrlvl_sdll_before_wrlvl[ch][byte], 0);
					}
	
					amp_program_cs_sdll(ch, max_CaBitCenterPoint_val - ckcs_diff);
					amp_program_ca_sdll(ch, 0);

				}
				else {
					amp_program_ck_sdll(ch, ckcs_diff); 
					
					// Since CK SDLL is moved, also program DQS WRLVL SDLL here 
					for(byte = 0 ; byte < DQ_NUM_BYTES; byte++ ) {
						dqwrlvl_sdll_before_wrlvl[ch][byte] = ckcs_diff;
						amp_program_dqwrlvl_sdll(ch, byte, dqwrlvl_sdll_before_wrlvl[ch][byte], 0);
					}
	
					amp_program_cs_sdll(ch, 0); 
					amp_program_ca_sdll(ch, (ckcs_diff - max_CaBitCenterPoint_val));
				}
				shim_printf("DEBUG - cacal_program_final_values:: ch = %d, max_CaBitCenterPoint_val = %d \n ", ch, max_CaBitCenterPoint_val);
			}
			else {
				amp_program_ck_sdll(ch, max_CaBitCenterPoint_val);
					
				// Since CK SDLL is moved, also program DQS WRLVL SDLL here 
				for(byte = 0 ; byte < DQ_NUM_BYTES; byte++ ) {
					dqwrlvl_sdll_before_wrlvl[ch][byte] = max_CaBitCenterPoint_val;
					amp_program_dqwrlvl_sdll(ch, byte, dqwrlvl_sdll_before_wrlvl[ch][byte], 0);
				}
	
				amp_program_cs_sdll(ch, max_CaBitCenterPoint_val - ckcs_diff);
				amp_program_ca_sdll(ch, 0);
				shim_printf("DEBUG - cacal_program_final_values:: ch = %d, max_CaBitCenterPoint_val = %d \n ", ch, max_CaBitCenterPoint_val);
			}
		}
		else {
			if (ckcs_diff > 0) {
				amp_program_ck_sdll(ch, ckcs_diff);
					
				// Since CK SDLL is moved, also program DQS WRLVL SDLL here 
				for(byte = 0 ; byte < DQ_NUM_BYTES; byte++ ) {
					dqwrlvl_sdll_before_wrlvl[ch][byte] = ckcs_diff;
					amp_program_dqwrlvl_sdll(ch, byte, dqwrlvl_sdll_before_wrlvl[ch][byte] , 0);
				}
	
				amp_program_cs_sdll(ch, 0);
				amp_program_ca_sdll(ch, __abs(max_CaBitCenterPoint_val) + ckcs_diff); 
			}
			else {
				amp_program_ck_sdll(ch, 0);
					
				// Since CK SDLL is moved, also program DQS WRLVL SDLL here 
				for(byte = 0 ; byte < DQ_NUM_BYTES; byte++ ) {
					dqwrlvl_sdll_before_wrlvl[ch][byte] = 0;
					amp_program_dqwrlvl_sdll(ch, byte, dqwrlvl_sdll_before_wrlvl[ch][byte], 0);
				}
	
				amp_program_cs_sdll(ch, -ckcs_diff);
				amp_program_ca_sdll(ch, __abs(max_CaBitCenterPoint_val));
			}
		}

		// Push the remainder of the delay on CA deskew signals
		// Since the max value of all bits was chosen for sdll, if the rest of the bits need more delay, compute their deskew
		for (bit_indx=0; bit_indx < CA_NUM_BITS; bit_indx++) {
			// We need not use __abs(max_CaBitCenterPoint_val) here
			cabit_deskew = max_CaBitCenterPoint_val - arr_CaBitCenterPoint[bit_indx];

			if(cabit_deskew < 0) {
				//shim_panic("Memory CA Calibration: cabit_deskew for bit_indx:%d is negative \n", bit_indx);
				shim_printf("PANIC during Vref scan. Record and move forward:Memory CA Calibration: cabit_deskew for bit_indx:%d is negative \n", bit_indx);
				cacal_vref_panic[ch][vref_ch] = 1;
			}
			
			// Program the CA Deskew values for each bit
			CSR_WRITE(rAMP_CADESKEW_CTRL(ch, bit_indx), cabit_deskew | RUNDESKEWUPD);
			
			// Poll for Run deskew to go low.
			SPIN_W_TMO_WHILE (CSR_READ(rAMP_CADESKEW_CTRL(ch, bit_indx)) & RUNDESKEWUPD);
		}

		// Set up DRAM mode register with the optimal CA Vref. 
		shim_mrcmd_to_ch_rnk(MR_WRITE, ch, 0, MR12, cfg_params.ca_vref_values[vref_ch]);

		// Program MCU registers for optimal CA Vref
		if(cfg_params.freq_bin == 1) {
			freqchngctl1 = CSR_READ(rAMCX_DRAMCFG_FREQCHNGCTL1_FREQ1(ch));		
			CSR_WRITE(rAMCX_DRAMCFG_FREQCHNGCTL1_FREQ1(ch), (freqchngctl1	& 0x00C0FFFF) | (0xC << 16) | (cfg_params.ca_vref_values[vref_opt_ca[ch]] << 24));
		}
		else if(cfg_params.freq_bin == 0) {
			freqchngctl1 = CSR_READ(rAMCX_DRAMCFG_FREQCHNGCTL1_FREQ0(ch));
			CSR_WRITE(rAMCX_DRAMCFG_FREQCHNGCTL1_FREQ0(ch), (freqchngctl1	& 0x00C0FFFF) | (0xC << 16) | (cfg_params.ca_vref_values[vref_opt_ca[ch]] << 24));
		}
	}
}

_ROMCODE_SEGMENT_PREFIX void cacal_program_cs_training_values(void)
{
	uint32_t loopchrnk0_indx, chrnk0_indx, ch;
	// Center point of CS training
	int32_t arr_CsBitCenterPoint;
	int32_t tmp_left_pos_val, tmp_right_pos_val;
	int32_t left_pos_val;
	int32_t right_pos_val;
	uint8_t byte;	

	int32_t rank_val[AMP_MAX_RANKS_PER_CHAN];
	uint32_t swloop;
	
	#ifdef ENV_DV
	uint32_t loopchrnk000 = loopchrnk_index_calc(0, 0, 0);

	for(ch = cfg_params.num_channels_runCalib; ch < cfg_params.num_channels; ch++) {
		if (ch == cfg_params.num_channels_runCalib)
			shim_printf("DEBUG - %s:: Preloading non-calibrated channels "
				    "with channel 0's calibration results \n", __PRETTY_FUNCTION__);
		// For channels on which calibration was not done, load calibration results from channel 0.
		chrnk0_indx = loopchrnk_index_calc(ch, 0, 0);

		cscal_per_loopchrnk_left[chrnk0_indx] = cscal_per_loopchrnk_left[loopchrnk000];	
		cscal_per_loopchrnk_right[chrnk0_indx] = cscal_per_loopchrnk_right[loopchrnk000];	
	
		//shim_printf("DEBUG - cscal_program_final_values:: cscal_per_loopchrnk_left[%d] = %d , cscal_per_loopchrnk_right[%d] = %d \n", loopchrnk0_indx, cscal_per_loopchrnk_left[loopchrnk0_indx], loopchrnk0_indx, cscal_per_loopchrnk_right[loopchrnk0_indx]);
 
	}
	#endif

	for (ch = 0; ch < cfg_params.num_channels; ch++) {
		// Calculate the Center Points for CK bit
		//comb_mask = 0x0;
		//mask_txn_detect = 0x0;
		//tmp_mask = 0x0;
			
		chrnk0_indx = loopchrnk_index_calc(ch, 0, 0);
		// Compute the aggr eye over multiple swloop and hwloop for all ranks
		for (swloop = 0; swloop < cfg_params.cacalib_sw_loops; swloop++) {
			loopchrnk0_indx = loopchrnk_index_calc(ch, 0, swloop);				
			//mask = 0x0;
			//for (hwloop=0; hwloop < cfg_params.cacalib_hw_loops; hwloop++)
			//		mask = mask | cacal_patterns_mask[(swloop * cfg_params.cacalib_hw_loops) + hwloop];
					
			// An explanation of the masks is below. Note that we only recorded result for a bit from a particular iteration if the bit had a transition.
			// mask: for pattern(s) sent in this swloop, indicates if the bit had a transition
			// tmp_mask: aggregates mask over all loops, including current swloop
			// comb_mask: aggregates mask over all loops, upto the last iteration of the swloop. After it is used to generate mask_txn_detect, it catches upto same value as tmp_mask
			// mask_txn_detect: indicates the first time a bit transitioned was in this swloop
			//tmp_mask = tmp_mask | mask;
			//mask_txn_detect = tmp_mask ^ comb_mask;
			//comb_mask = comb_mask | mask;
				
			/* 
			 * Rank 0
			 */
				
			/* Left side */
			
			// lookup the value in the left side for this bit given loop, ch, and rnk
			rank_val[0] = cscal_per_loopchrnk_left[loopchrnk0_indx];
			tmp_left_pos_val = rank_val[0];
					
			// If this is the first time this bit transitioned, just put it in the aggregate result array
			//	if (mask_txn_detect & (1 << bit_indx)) {
			//		left_pos_val = tmp_left_pos_val;
			//		cacal_per_chrnk_left[chrnk0_indx][bit_indx] = left_pos_val;
			//	} else if (mask & (1 << bit_indx)) {
					// This is not the 1st time this bit transitioned so there is a recorded result already, but since we have a new result
					// to compare with, find the value that would cover both points and put that in the array
		 	//		left_pos_val = cacal_per_chrnk_left[chrnk0_indx][bit_indx];
			//		left_pos_val = find_common_endpoint(tmp_left_pos_val, left_pos_val, MIN_ENDPT);
			//		cacal_per_chrnk_left[chrnk0_indx][bit_indx] = left_pos_val;
			//	} else {
			//		// <rdar://problem/13834199>
			//		left_pos_val = tmp_left_pos_val;
			//		cacal_per_chrnk_left[chrnk0_indx][bit_indx] = left_pos_val;
			//	}

			if(swloop == 0) {
				left_pos_val = tmp_left_pos_val;
				cscal_per_chrnk_left[chrnk0_indx] = left_pos_val;
			} else {
				// This is not the 1st time this bit transitioned so there is a recorded result already, but since we have a new result
				// to compare with, find the value that would cover both points and put that in the array
		 		left_pos_val = cscal_per_chrnk_left[chrnk0_indx];
				left_pos_val = find_common_endpoint(tmp_left_pos_val, left_pos_val, MAX_ENDPT);
				cscal_per_chrnk_left[chrnk0_indx] = left_pos_val;
			}

				
			/* Right side */
				
			// lookup the value in the right side for this bit given loop, ch, and rnk
			rank_val[0] = cscal_per_loopchrnk_right[loopchrnk0_indx];
			tmp_right_pos_val = rank_val[0];
					
			/*	// If this is the first time this bit transitioned, just put it in the aggregate result array
				if (mask_txn_detect & (1 << bit_indx)) {
					right_pos_val = tmp_right_pos_val;
					cacal_per_chrnk_right[chrnk0_indx][bit_indx] = right_pos_val;
				} else if (mask & (1 << bit_indx)) {
					// This is not the 1st time this bit transitioned so there is a recorded result already, but since we have a new result
					// to compare with, find the value that would cover both points and put that in the array
					right_pos_val = cacal_per_chrnk_right[chrnk0_indx][bit_indx];
					right_pos_val = find_common_endpoint(tmp_right_pos_val, right_pos_val, MAX_ENDPT);
					cacal_per_chrnk_right[chrnk0_indx][bit_indx] = right_pos_val;
				} else {
					// <rdar://problem/13834199>
					right_pos_val = tmp_right_pos_val;
					cacal_per_chrnk_right[chrnk0_indx][bit_indx] = 0;
				}
			*/

			if(swloop == 0) {
				right_pos_val = tmp_right_pos_val;
				cscal_per_chrnk_right[chrnk0_indx] = right_pos_val;
			} else {
				// This is not the 1st time this bit transitioned so there is a recorded result already, but since we have a new result
				// to compare with, find the value that would cover both points and put that in the array
		 		right_pos_val = cscal_per_chrnk_right[chrnk0_indx];
				right_pos_val = find_common_endpoint(tmp_right_pos_val, right_pos_val, MIN_ENDPT);
				cscal_per_chrnk_right[chrnk0_indx] = right_pos_val;
			}

		}
		// At this point, the left edge and the right edge of the eye for this channel and bit are defined by left_pos_val and right_pos_val
		// Find the center of the eye
		arr_CsBitCenterPoint = find_center_of_eye(cscal_per_chrnk_left[chrnk0_indx], cscal_per_chrnk_right[chrnk0_indx]);
		
		/*
		 * Finally, program the values
		 * If the center point is positive, put the magnitude on CKSDLL.
		 * If the center point is negative, put the magnitude on CSSDLL.
		*/
		if(arr_CsBitCenterPoint >= 0) {
			// Program the CK SDLL with the center point value
			amp_program_ck_sdll(ch, arr_CsBitCenterPoint);
					
			// Since CK SDLL is moved, also program DQS WRLVL SDLL here 
			for(byte = 0 ; byte < DQ_NUM_BYTES; byte++ ) {
				dqwrlvl_sdll_before_wrlvl[ch][byte] = arr_CsBitCenterPoint;
				amp_program_dqwrlvl_sdll(ch, byte, dqwrlvl_sdll_before_wrlvl[ch][byte], 0);
			}
	
			amp_program_cs_sdll(ch, 0);
		} else {
			// Program the CS SDLL with the center point value
			amp_program_cs_sdll(ch, __abs(arr_CsBitCenterPoint));
			amp_program_ck_sdll(ch, 0);
					
			// Since CK SDLL is moved, also program DQS WRLVL SDLL here 
			for(byte = 0 ; byte < DQ_NUM_BYTES; byte++ ) {
				dqwrlvl_sdll_before_wrlvl[ch][byte] = 0;
				amp_program_dqwrlvl_sdll(ch, byte, dqwrlvl_sdll_before_wrlvl[ch][byte], 0);
			}
	
		}
	}
}

// Loop through PerBitDeskewCode ranges for rddq until failing points for each byte (& bit) are found.
// Maui Change: Right failing to left failing.
_ROMCODE_SEGMENT_PREFIX void rddqcal_find_left_failing_point(uint32_t ch, uint32_t rnk, uint32_t vref, bool after_wrddqcal)
{
	uint32_t dq_deskew;
	uint32_t all_bits_fail;
	uint32_t bits_fail_b[DQ_NUM_BYTES] = { 0 };
	// CP : Maui Change
	uint32_t rddqcalresult, result_swiz;
	uint32_t mask_b[DQ_NUM_BYTES];
	uint32_t start_b[DQ_NUM_BYTES];
	uint32_t byte, bit;//, dqbyte;
	uint32_t step_size = COARSE_STEP_SZ;
	
	all_bits_fail = 0;
	dq_deskew = 0;
	
	// set the rddq sdll to 0.
	for (byte = 0; byte < DQ_NUM_BYTES; byte++) {
		// CP : Maui Change
		amp_set_rddq_sdll(ch, byte, 0);
		
		// initialize the mask for each byte lane (Including DMI)
		mask_b[byte] = (0xFF << (byte * 8)) | (0x1 << (byte + 16));
	}
	
	rddqcalresult = 0x3FFFF;
	
	// PerBit Deskew lines cannot be pushed beyond DQ_MAX_DESKEW_PER_BIT value
	do {
		for (byte = 0; byte < DQ_NUM_BYTES; byte++) {
			if (bits_fail_b[byte] == 0) {
				
				for (bit = 0; bit < DQ_NUM_BITS_PER_BYTE; bit++) {
					CSR_WRITE(rAMP_RDDQDESKEW_CTRL(byte, ch, bit), dq_deskew | RUNDESKEWUPD);
					SPIN_W_TMO_WHILE (CSR_READ(rAMP_RDDQDESKEW_CTRL(byte, ch, bit)) & RUNDESKEWUPD);
				}

				// Also move DMI for the byte here
				CSR_WRITE(rAMP_RDDMDESKEW_CTRL(byte, ch), dq_deskew | RUNDESKEWUPD);	
				SPIN_W_TMO_WHILE (CSR_READ(rAMP_RDDMDESKEW_CTRL(byte, ch)) & RUNDESKEWUPD);
			}
		}
				
			amp_run_rddqcal(ch);
			//shim_printf("DEBUG - rddqcal_find_left_failing_point:: Running RDDQ cal with DQ deskew=0x%x \n",dq_deskew);

			rddqcalresult = (CSR_READ(rAMP_DQCALRESULT(ch)) & 0x3FFFF);
		
			//shim_printf("DEBUG - rddqcal_find_left_failing_point:: RDDQ cal result=0x%x \n",rddqcalresult);
		result_swiz	= rddqcalresult;

		for (byte = 0; byte < DQ_NUM_BYTES; byte++) {
			// if all bits haven't failed yet and this run shows all bits failing, we have found the failing point for this byte
			if ((bits_fail_b[byte] == 0) && ((result_swiz & mask_b[byte]) == 0)) {
				bits_fail_b[byte] = 1;
				start_b[byte] = dq_deskew;
			}
		}
		
		// FIMXE
		all_bits_fail = bits_fail_b[0];
		for (byte = 1; byte < DQ_NUM_BYTES; byte++) {
			all_bits_fail &= bits_fail_b[byte];
		}

		if (all_bits_fail == 1) {
			// If failing point has been found for all bits, find the passing point now
			// CP : Maui Change
			rddqcal_find_left_passing_point(ch, rnk, vref, start_b, after_wrddqcal);
			break;
		} else {
			// To find right failing point, make more negative adjustment to the sdll (same as incrementing deskew)

			// If the next increment of tap value is over the maximum allowed tap value, switch
			// from coarse stepping to fine stepping.
			if((dq_deskew + step_size) > DQ_MAX_DESKEW_PER_BIT) {
				step_size = FINER_STEP_SZ;
			} 

			dq_deskew = dq_deskew + step_size;
		}
		
	} while ((dq_deskew <= DQ_MAX_DESKEW_PER_BIT) && (all_bits_fail == 0));
	
	if (all_bits_fail == 0) {
		// print error message as Left Failing Point cannot be found
		//shim_panic("Memory Rddq cal: Left side failing point not found, max deskew limit reach for channel %d", ch);
		shim_printf("PANIC during Vref scan. Record and move forward:Memory Rddq cal: Left side failing point not found, max deskew limit reach for channel %d", ch);
		rddqcal_vref_panic[ch][vref] = 1;
	}
	
	// Reset deskew for all bits to 0
	for (byte = 0; byte < DQ_NUM_BYTES; byte++) {
		for (bit = 0; bit < DQ_NUM_BITS_PER_BYTE; bit++) {
			CSR_WRITE(rAMP_RDDQDESKEW_CTRL(byte, ch, bit), 0 | RUNDESKEWUPD);
			SPIN_W_TMO_WHILE (CSR_READ(rAMP_RDDQDESKEW_CTRL(byte, ch, bit)) & RUNDESKEWUPD);	
		}

		// Also reset DMI deskew
		CSR_WRITE(rAMP_RDDMDESKEW_CTRL(byte, ch), 0 | RUNDESKEWUPD);	
		SPIN_W_TMO_WHILE (CSR_READ(rAMP_RDDMDESKEW_CTRL(byte, ch)) & RUNDESKEWUPD);
	}
}

// Purpose of this function is to start from left side failing point and find locations for every DQ bit
// until the start of passing window for that bit is found
// Save all this locations to compute the center of window
_ROMCODE_SEGMENT_PREFIX void rddqcal_find_left_passing_point(uint32_t ch, uint32_t rnk, uint32_t vref, uint32_t *start_b, bool after_wrddqcal)
{
	uint32_t chrnk_indx;
	uint32_t tap_value_b[DQ_NUM_BYTES];
	bool switch_from_dqtodqs, max_tap_value_reached;
	uint32_t BitPass[DQ_TOTAL_BITS+DMI_TOTAL_BITS] = { 0 };
	uint32_t SolidBitPass[DQ_TOTAL_BITS+DMI_TOTAL_BITS] = { 0 };
	uint32_t rddqcalresult;
	uint32_t all_bits_pass;
	uint32_t all_bits_pass_b[DQ_NUM_BYTES] = { 0 };
	uint32_t bit_indx, byte;
	uint32_t dqmdllcode;
	uint32_t saved_val;
	
	chrnk_indx = loopchrnk_index_calc(ch, rnk, 0);
	all_bits_pass = 0;
	switch_from_dqtodqs = false;
	max_tap_value_reached = false;

	for (byte = 0; byte < DQ_NUM_BYTES; byte++) {
		tap_value_b[byte] = start_b[byte];
	}
	
	// Moving Right to Left to find point where each bit turns from FAIL TO PASS
	do {
		if (switch_from_dqtodqs == false) {
			// continue to update per bit deskew until all bits pass for each byte lane
			for (byte = 0; byte < DQ_NUM_BYTES; byte++) {
				if (all_bits_pass_b[byte] == 0) {
					
					for (bit_indx = 0; bit_indx < DQ_NUM_BITS_PER_BYTE; bit_indx++) {
						CSR_WRITE(rAMP_RDDQDESKEW_CTRL(byte, ch, bit_indx), tap_value_b[byte] | RUNDESKEWUPD);
						SPIN_W_TMO_WHILE (CSR_READ(rAMP_RDDQDESKEW_CTRL(byte, ch, bit_indx)) & RUNDESKEWUPD);
					}

					// Also program DMI here
					CSR_WRITE(rAMP_RDDMDESKEW_CTRL(byte, ch), tap_value_b[byte] | RUNDESKEWUPD);
					SPIN_W_TMO_WHILE (CSR_READ(rAMP_RDDMDESKEW_CTRL(byte, ch)) & RUNDESKEWUPD);
				
					//shim_printf("DEBUG - rddqcal_find_left_passing_point:: Pushing byte %d by a DQ deskew of 0x%x \n",byte,tap_value_b[byte]);
				}
			}
		} else {
			// adjust rddqs sdll until all bits pass for each byte lane
			for (byte = 0; byte < DQ_NUM_BYTES; byte++) {
				if (all_bits_pass_b[byte] == 0) {
					amp_set_rddq_sdll(ch, byte, tap_value_b[byte]);
					//shim_printf("DEBUG - rddqcal_find_left_passing_point:: Pushing byte %d by DQS sdll of 0x%x \n",byte,tap_value_b[byte]);
				}
			}
		}
		
		// Run rddq calibration in hw
			amp_run_rddqcal(ch);

			// CP : Maui Change
			rddqcalresult = (CSR_READ(rAMP_DQCALRESULT(ch)) & 0x3FFFF);
			
			//shim_printf("DEBUG - rddqcal_find_left_passing_point:: RDDQ cal result=0x%x \n",rddqcalresult);
		
		// Make sure that each Bit sees a transition from 0 to 1 on DqCalresult Register
		for (bit_indx = 0; bit_indx < (DQ_TOTAL_BITS + DMI_TOTAL_BITS); bit_indx++) {
			
			byte = (bit_indx < DQ_TOTAL_BITS) ? (bit_indx >> 3) : (bit_indx - DQ_TOTAL_BITS); // bit_indx / DQ_NUM_BITS_PER_BYTE
			
			// Check if this bit passed during the calibration (not necessarily for first time)
			if ((BitPass[bit_indx] == 0) && ((rddqcalresult & (1 << bit_indx)) != 0)) {
				// Has this bit passed SOLID_PASS_DETECT number of times? Then consider it done
				if (SolidBitPass[bit_indx] == SOLID_PASS_DETECT) {
					BitPass[bit_indx] = 1;
				} else if (SolidBitPass[bit_indx] > 0) {
					SolidBitPass[bit_indx] = SolidBitPass[bit_indx] + 1;
				} else {
					// bit passed for the first time, record this value in the global array as the right edge
					SolidBitPass[bit_indx] = SolidBitPass[bit_indx] + 1;
					
					byte = (bit_indx < DQ_TOTAL_BITS) ? (bit_indx >> 3) : (bit_indx - DQ_TOTAL_BITS); // bit_indx / DQ_NUM_BITS_PER_BYTE
					
					if (switch_from_dqtodqs == false) {
						dqmdllcode = mdllcode[ch][byte+1];
						
						saved_val = tap_value_b[byte];

						// If saved_val is in terms of DQ deskew, save it as a negative number.
						// If saved_val is in terms of DQS SDLL, save it as a positive number.
						rddqcal_per_chrnk_left[chrnk_indx][vref][bit_indx] = -1 * saved_val;
					} else {
						saved_val = tap_value_b[byte];
						rddqcal_per_chrnk_left[chrnk_indx][vref][bit_indx] = saved_val;
					}
					shim_printf("DEBUG - rddqcal_find_left_passing_point:: Found Left edge for channel %d bit %d at tap value %d \n",chrnk_indx,bit_indx,rddqcal_per_chrnk_left[chrnk_indx][vref][bit_indx]);
				}
			} else {
				// bit failed calibration, reset the pass count to 0
				SolidBitPass[bit_indx] = 0;
			}
		}
		
		all_bits_pass = 1;
		for (byte = 0; byte < DQ_NUM_BYTES; byte++)
			all_bits_pass_b[byte] = 1;
		
		for (bit_indx = 0; bit_indx < (DQ_TOTAL_BITS + DMI_TOTAL_BITS); bit_indx++) {
			byte = (bit_indx < DQ_TOTAL_BITS) ? (bit_indx >> 3) : (bit_indx - DQ_TOTAL_BITS); // bit_indx / DQ_NUM_BITS_PER_BYTE
			
			// Did all the bits pass (SOLID_PASS_DETECT number of times) in this byte lane?
			// If anyone of the bits failed, then the byte flag is cleared
			all_bits_pass_b[byte] = all_bits_pass_b[byte] & BitPass[bit_indx];
			
			// Did all bits in all byte lanes pass?
			all_bits_pass = all_bits_pass & BitPass[bit_indx];
		}		
		
		// If ALL bits are not passing - keep moving from Right to Left Side of window (by adding less negative adjustment to mdll)
		if (all_bits_pass == 0) {
			// Even if one of the byte lanes arrives early to tap_value = 0. Remain here until all byte lane catch up before proceeding to pushing out dq
			
			// check for all bytes reaching 0 on the tap value (could be deskew or sdll)
			uint32_t all_bytes_tap = tap_value_b[0];
			for (byte = 1; (byte < DQ_NUM_BYTES) && (all_bytes_tap == 0); byte++) {
				all_bytes_tap += tap_value_b[byte];
			}
			
			// if the tap_value for all bytes has reached 0 on the deskew, make the transition to SDLL
			if ((all_bytes_tap == 0) && (switch_from_dqtodqs == false)) {
				switch_from_dqtodqs = true;
				
				for (byte = 0; byte < DQ_NUM_BYTES; byte++) {
					
					tap_value_b[byte] = ((CSR_READ(rAMP_DQSDLLCTRL_RD(byte, ch)) >> SDLLOFFSET) & DLLVAL_BITS);
				}
			} 
			// CP : Maui Change
			//else if (switch_from_dqtodqs == true) {
				// If any of the failing bytes has reached tap_value of 0, clear sign bit for that byte lane
			//	for (byte = 0; byte < DQ_NUM_BYTES; byte++)
					//if ((all_bits_pass_b[byte]) == 0 && (tap_value_b[byte] == 0))
					//	sgn_bit_b[byte] = 0;
			//}
			
			// To find left side passing point, add positive adjustment to mdll (same as decrementing deskew)
			
			// For deskew taps, we just decrement by FINER_STEP_SZ if we haven't reached 0 yet
			if (switch_from_dqtodqs == false) {
				for (byte = 0; byte < DQ_NUM_BYTES; byte++)
					if (tap_value_b[byte] > 0)
						tap_value_b[byte] -= FINER_STEP_SZ;
			} else {
				// For sdll taps, increment by FINER_STEP_SZ
				for (byte = 0; byte < DQ_NUM_BYTES; byte++) {
					if (all_bits_pass_b[byte] == 0) {
						//if (sgn_bit_b[byte]) {
						//	if (tap_value_b[byte] <= FINER_STEP_SZ) {
						//		tap_value_b[byte] = 0;
						//		sgn_bit_b[byte] = 0;
						//	} else
						//		tap_value_b[byte] -= FINER_STEP_SZ;
						//}
						//else
						// CP : Maui Change
						tap_value_b[byte] += FINER_STEP_SZ;
					}
				}
			}
		}
		
		// trigger for loop to end if any of the bytes reach max tap value
		for (byte = 0; byte < DQ_NUM_BYTES; byte++) {
			if (!max_tap_value_reached) {
				max_tap_value_reached = (tap_value_b[byte] > (3 * mdllcode[ch][1 + byte]));
				shim_printf("DEBUG - rddqcal_find_left_passing_point:: tap_value_b[%d] = %d, mdllcode[%d][%d] = %d , max_tap_value_reached = %d \n",byte,tap_value_b[byte],ch,1 + byte,mdllcode[ch][1 + byte],max_tap_value_reached);
			}			

			if (max_tap_value_reached) {
				if (all_bits_pass == 0)
					shim_printf("PANIC during Vref scan. Record and move forward:Memory rddq calibration: Unable to find left side passing point, max tap value reached");
					rddqcal_vref_panic[ch][vref] = 1;
					break;
			}
		}
		
	} while ((!max_tap_value_reached) && (all_bits_pass == 0));
}

// Purpose of this function is to start push DQS out till right side failing point of Data window is found
_ROMCODE_SEGMENT_PREFIX void rddqcal_find_right_failing_point(uint32_t ch, uint32_t rnk, uint32_t vref, bool after_wrddqcal)
{
	uint32_t rddqsdll[DQ_NUM_BYTES];
	// CP : Maui Change
	uint32_t rddqcalresult, result_swiz;
	uint32_t all_bits_fail;
	uint32_t all_bits_fail_b[DQ_NUM_BYTES] = { 0 };
	uint32_t mask_b[DQ_NUM_BYTES];
	uint32_t start_b[DQ_NUM_BYTES];
	uint32_t byte;
	bool max_tap_value_reached = false;
	uint32_t step_size = COARSE_STEP_SZ;
		
	all_bits_fail = 0;
	rddqcalresult = 0x3FFFF;
	
	for (byte = 0; byte < DQ_NUM_BYTES; byte++) {
		// initialize the mask for each byte lane (Including DMI)
		mask_b[byte] = (0xFF << (byte * 8)) | (0x1 << (byte + 16));
		
		// Get the starting values for RD DQS SDLL
		rddqsdll[byte] = ((CSR_READ(rAMP_DQSDLLCTRL_RD(byte, ch)) >> SDLLOFFSET) & DLLVAL_BITS);
	}
		
	// To find left failing point, keep adding less negative adjustment to mdll
	do {
		for (byte = 0; byte < DQ_NUM_BYTES; byte++) {
			// set the new sdll for this byte lane if all bits are not yet failing
			if (all_bits_fail_b[byte] == 0) {
				amp_set_rddq_sdll(ch, byte, rddqsdll[byte]);
				//shim_printf("DEBUG - rddqcal_find_right_failing_point:: Pushing byte %d by DQS sdll of 0x%x \n",byte,rddqsdll[byte]);
			}
		}
		
		// Run rddqcal in hw
			amp_run_rddqcal(ch);
			rddqcalresult &= (CSR_READ(rAMP_DQCALRESULT(ch)) & 0x3FFFF);
			//shim_printf("DEBUG - rddqcal_find_right_failing_point:: RDDQ cal result=0x%x \n",rddqcalresult);
		// Account for swizzled bits and bytes of the result
		// <rdar://problem/15544055> 
		// result_swiz = dqcal_handle_swizzle(rddqcalresult, after_wrddqcal);
		result_swiz = rddqcalresult;		


		// If the result of all bits in this byte show a fail, record this as the failing point
		all_bits_fail = 1;
		for (byte = 0; byte < DQ_NUM_BYTES; byte++) {
			if ((all_bits_fail_b[byte] == 0) && ((result_swiz & mask_b[byte]) == 0)) {
				all_bits_fail_b[byte] = 1;
				start_b[byte] = rddqsdll[byte];
			}
			
			all_bits_fail &= all_bits_fail_b[byte];
		}

		// all bytes fail, call the function to find right passing point
		if (all_bits_fail == 1) {
			rddqcal_find_right_passing_point(ch, rnk, vref, start_b, after_wrddqcal);
		} else {
			// if the byte has not yet failed, find the next sdll value to be set
			for (byte = 0; byte < DQ_NUM_BYTES; byte++) {
				if (all_bits_fail_b[byte] == 0) {
						if((rddqsdll[byte] + step_size) > (3 * mdllcode[ch][1 + byte])) {
							step_size = FINER_STEP_SZ;
						}

						rddqsdll[byte] += step_size;
				}
			}
		}
		
		for (byte = 0; byte < DQ_NUM_BYTES; byte++) {
			// none of the previous bytes reached max_tap_value, then update the boolean
			if (!max_tap_value_reached)
				max_tap_value_reached = (rddqsdll[byte] > (3 * mdllcode[ch][1 + byte]));
			
			if (max_tap_value_reached) {
				if (all_bits_fail_b[byte] == 0) {
					//shim_panic("Memory rddq calibration: Unable to find right failing point, max tap value reached for ch %d byte %d", ch, byte);
					shim_printf("PANIC during Vref scan. Record and move forward:Memory rddq calibration: Unable to find right failing point, max tap value reached for ch %d byte %d", ch, byte);
					rddqcal_vref_panic[ch][vref] = 1;
				}
			}
		}
	} while ((!max_tap_value_reached) && (all_bits_fail == 0));
}

// Purpose of this function is to start from left side failing point and find passing locations for every DQ bit on left side of window
// Save all the locations to compute the center of window later
// To find left passing point, move to the right from the failing point, which means keep adding more negative adjustment to mdll
_ROMCODE_SEGMENT_PREFIX void rddqcal_find_right_passing_point(uint32_t ch, uint32_t rnk, uint32_t vref, uint32_t *start_b, bool after_wrddqcal)
{
	uint32_t chrnk_indx;
	bool max_tap_value_reached = false;
	uint32_t tap_value_b[DQ_NUM_BYTES];
	uint32_t BitPass[DQ_TOTAL_BITS + DMI_TOTAL_BITS] = { 0 };
	uint32_t SolidBitPass[DQ_TOTAL_BITS + DMI_TOTAL_BITS] = { 0 };
	uint32_t rddqcalresult;
	uint32_t all_bits_pass;
	uint32_t all_bits_pass_b[DQ_NUM_BYTES] = { 0 };
	uint32_t bit_indx, byte;
	uint32_t saved_val;
	
	chrnk_indx = loopchrnk_index_calc(ch, rnk, 0);
	all_bits_pass = 0;
	rddqcalresult = 0x3FFFF;

	for (byte = 0; byte < DQ_NUM_BYTES; byte++) {
		tap_value_b[byte] = start_b[byte];
	}

	// Finding Left side passing point on per bit level. Moving Left to Right (keep adding more negative adj to mdll) to find point where it turns from FAIL TO PASS
	do {
		
		for (byte = 0; byte < DQ_NUM_BYTES; byte++) {
			// if we haven't found all bits passing for this byte, push out new sdll value
			if (all_bits_pass_b[byte] == 0) {
				amp_set_rddq_sdll(ch, byte, tap_value_b[byte]);
				//shim_printf("DEBUG - rddqcal_find_right_passing_point:: Pushing byte %d by DQS sdll of 0x%x \n",byte,tap_value_b[byte]);
			}
		}
				
		// Run rddqcal in hw
			amp_run_rddqcal(ch);
			rddqcalresult = (CSR_READ(rAMP_DQCALRESULT(ch)) & 0x3FFFF);
			//shim_printf("DEBUG - rddqcal_find_right_passing_point:: RDDQ cal result=0x%x \n",rddqcalresult);
		
		// Make sure that each Bit sees a transition from 0 to 1 on DqCalresult Register
		for (bit_indx = 0; bit_indx < (DQ_TOTAL_BITS + DMI_TOTAL_BITS); bit_indx++) {

			// Check if this bit passed during the calibration (not necessarily for first time)
			if ((BitPass[bit_indx] == 0) && ((rddqcalresult & (1 << bit_indx)) != 0)) {
				// Has this bit passed SOLID_PASS_DETECT number of times? Then consider it done
				if (SolidBitPass[bit_indx] == SOLID_PASS_DETECT) {
					BitPass[bit_indx] = 1;
				} else if (SolidBitPass[bit_indx] > 0) {
					SolidBitPass[bit_indx] = SolidBitPass[bit_indx] + 1;
				} else {
					// bit passed for the first time, record this value in the global array as the left edge
					SolidBitPass[bit_indx] = SolidBitPass[bit_indx] + 1;
					
					byte = (bit_indx < DQ_TOTAL_BITS) ? (bit_indx >> 3) : (bit_indx - DQ_TOTAL_BITS); // bit_indx / DQ_NUM_BITS_PER_BYTE
					
					saved_val = tap_value_b[byte];
					
					rddqcal_per_chrnk_right[chrnk_indx][vref][bit_indx] = saved_val;
					shim_printf("DEBUG - rddqcal_find_right_passing_point:: Found right edge for channel %d bit %d at tap value %d \n",chrnk_indx,bit_indx,rddqcal_per_chrnk_right[chrnk_indx][vref][bit_indx]);
				}
			} else {
				// bit failed calibration, reset the pass count to 0
				SolidBitPass[bit_indx] = 0;
			}
		}

		all_bits_pass = 1;
		for (byte = 0; byte < DQ_NUM_BYTES; byte++)
			all_bits_pass_b[byte] = 1;
		
		for (bit_indx = 0; bit_indx < (DQ_TOTAL_BITS + DMI_TOTAL_BITS); bit_indx++) {
			byte = (bit_indx < DQ_TOTAL_BITS) ? (bit_indx >> 3) : (bit_indx - DQ_TOTAL_BITS); // bit_indx / DQ_NUM_BITS_PER_BYTE
			
			// Did all the bits pass (SOLID_PASS_DETECT number of times) in this byte lane?
			// If anyone of the bits failed, then the byte flag is cleared
			all_bits_pass_b[byte] = all_bits_pass_b[byte] & BitPass[bit_indx];
			
			// Did all bits in all byte lanes pass?
			all_bits_pass = all_bits_pass & BitPass[bit_indx];
		}
		
		// If ALL bits are not passing - keep moving from Left to Right Side of window (by adding more negative adjustment to mdll)
		if (all_bits_pass == 0) {
			
			for (byte = 0; byte < DQ_NUM_BYTES; byte++) {
				
				// if this byte lane does not have all passing bits, adjust this byte's sdll
				if (all_bits_pass_b[byte] == 0) {
					// for negative value, increase the magnitude
					//if (sgn_bit_b[byte] == 1)
				   //		tap_value_b[byte] += FINER_STEP_SZ;
					//else {
						// if we're at 0, increase the magnitude and change sign
					//	if (tap_value_b[byte] == 0) {
					//		tap_value_b[byte] += FINER_STEP_SZ;
					//		sgn_bit_b[byte] = 1;
					//	}
					//	// for positive value, decrease the magnitude
					//	else
							tap_value_b[byte] -= FINER_STEP_SZ;
					}
				}
			}
		
		// check for end of loop condition
		for (byte = 0; byte < DQ_NUM_BYTES; byte++) {
			if (!max_tap_value_reached)
				max_tap_value_reached = (tap_value_b[byte] > (3 * mdllcode[ch][1 + byte]));

 			if (max_tap_value_reached) {
				if (all_bits_pass == 0) {
					//shim_panic("Memory rddq calibration: Unable to find right passing point, max tap value reached");
					shim_printf("PANIC during Vref scan. Record and move forward:Memory rddq calibration: Unable to find right passing point, max tap value reached");
					rddqcal_vref_panic[ch][vref] = 1;
				}
 				break;
 			}
 			
			// panic if we get beyond -dqmdllcode, since we really shouldn't have to go that far
			if ((all_bits_pass == 0) && (tap_value_b[byte] == 0)) {
				//shim_panic("Memory rddq calibration: Not yet found right passing point but SDLL = 0 for ch %d byte %d", ch, byte);
				shim_printf("PANIC during Vref scan. Record and move forward:Memory rddq calibration: Not yet found right passing point but SDLL = 0 for ch %d byte %d", ch, byte);
				rddqcal_vref_panic[ch][vref] = 1;
			}
		}
	} while ((!max_tap_value_reached) && (all_bits_pass == 0));
}

_ROMCODE_SEGMENT_PREFIX void rddqcal_program_final_values(void)
{
	uint32_t ch, bit_indx, byte;
	uint32_t chrnk0_indx;
	int32_t arr_RdDqBitCenterPoint[DQ_TOTAL_BITS + DMI_TOTAL_BITS];
	uint32_t arr_RdDqBitDeSkew[DQ_TOTAL_BITS + DMI_TOTAL_BITS];
	int32_t left_pos_val;
	int32_t right_pos_val;
	int32_t max_RdDqCenterPoint_val_b[DQ_NUM_BYTES];
	uint32_t vref_ch;
#if ENV_DV
	uint32_t loopchrnk000 = loopchrnk_index_calc(0, 0, 0);
	uint32_t vref0 = vref_opt_rddq[loopchrnk000];
	
	for(ch = cfg_params.num_channels_runCalib; ch < cfg_params.num_channels; ch++) {
		if (ch == cfg_params.num_channels_runCalib)
			shim_printf("DEBUG - %s:: Preloading non-calibrated channels "
				    "with channel 0's calibration results \n", __PRETTY_FUNCTION__);
		// For channels on which calibration was not done, load calibration results from channel 0.
		chrnk0_indx = loopchrnk_index_calc(ch, 0, 0);
		vref_ch = vref_opt_rddq[chrnk0_indx] = vref0; 
	
		for (bit_indx=0; bit_indx < (DQ_TOTAL_BITS + DMI_TOTAL_BITS); bit_indx++) {
			rddqcal_per_chrnk_left[chrnk0_indx][vref_ch][bit_indx] = rddqcal_per_chrnk_left[loopchrnk000][vref0][bit_indx];	
			rddqcal_per_chrnk_right[chrnk0_indx][vref_ch][bit_indx] = rddqcal_per_chrnk_right[loopchrnk000][vref0][bit_indx];	
		}
	}

#endif

	for (ch = 0; ch < cfg_params.num_channels; ch++) {
		chrnk0_indx = loopchrnk_index_calc(ch, 0, 0);
		vref_ch = vref_opt_rddq[chrnk0_indx];
		
		// find the center point of passing window for each bit over all ranks
		for (bit_indx = 0; bit_indx < (DQ_TOTAL_BITS + DMI_TOTAL_BITS); bit_indx++) {
		
			left_pos_val = rddqcal_per_chrnk_left[chrnk0_indx][vref_ch][bit_indx];
			right_pos_val = rddqcal_per_chrnk_right[chrnk0_indx][vref_ch][bit_indx];

			// find center of the eye for this bit
			arr_RdDqBitCenterPoint[bit_indx] = find_center_of_eye(left_pos_val, right_pos_val);
			shim_printf("DEBUG - rddqcal_program_final_values:: Center value for bit %d in Channel %d = %d \n", bit_indx, ch, arr_RdDqBitCenterPoint[bit_indx]);
	
		}
		
		// initialize the max centerpoint to the 1st bit's center point in each byte lane
		for (byte = 0; byte < DQ_NUM_BYTES; byte++)
			max_RdDqCenterPoint_val_b[byte] = arr_RdDqBitCenterPoint[byte * DQ_NUM_BITS_PER_BYTE];

		// Find the maximum CenterPoint per byte lane given each bit's center point
		for (bit_indx=0; bit_indx < (DQ_TOTAL_BITS + DMI_TOTAL_BITS); bit_indx++) {
			
			byte = (bit_indx < DQ_TOTAL_BITS) ? (bit_indx >> 3) : (bit_indx - DQ_TOTAL_BITS); // bit_indx / DQ_NUM_BITS_PER_BYTE
			
			// REVIEW
			// if this bit's center point is greater than current max, make it the new max (we'll program this to sdll, and other values will require deskew)
			max_RdDqCenterPoint_val_b[byte] = find_common_endpoint(max_RdDqCenterPoint_val_b[byte], arr_RdDqBitCenterPoint[bit_indx], MAX_ENDPT);
		}
		
		for (byte = 0; byte < DQ_NUM_BYTES; byte++) {
			shim_printf("DEBUG - rddqcal_program_final_values:: Maximum center value for byte %d = %d \n", byte, max_RdDqCenterPoint_val_b[byte]);
		}

		// Compute the individual deskew values: any bits with center point < max for its byte lane will require deskew
		// Each bit's center is guaranteed to be <= max for its byte lane
		// Deskewing means adding more negative adjustment for this bit in addition to the sdll, which is clamped on the negative side to -dqmdllcode
		for (bit_indx = 0; bit_indx < (DQ_TOTAL_BITS + DMI_TOTAL_BITS); bit_indx++) {
			
			byte = (bit_indx < DQ_TOTAL_BITS) ? (bit_indx >> 3) : (bit_indx - DQ_TOTAL_BITS); // bit_indx / DQ_NUM_BITS_PER_BYTE
			
			arr_RdDqBitDeSkew[bit_indx] = max_RdDqCenterPoint_val_b[byte] - arr_RdDqBitCenterPoint[bit_indx];
		}
		
		// Program the SDLL and deskew per bit for each byte lane
		for (byte = 0; byte < DQ_NUM_BYTES; byte++) {

			// If max_RdDqCenterPoint_val_b[byte] < 0, program a value of 0 into RD DQS SDLL. 
			if(max_RdDqCenterPoint_val_b[byte] >= 0) {
				amp_program_rddq_sdll(ch, byte, max_RdDqCenterPoint_val_b[byte]);
			}
			else {
				amp_program_rddq_sdll(ch, byte, 0); 
			}
			
			// per bit deskew for this byte lane
			for (bit_indx = 0; bit_indx < DQ_NUM_BITS_PER_BYTE; bit_indx++) {
			
				// If max_RdDqCenterPoint_val_b[byte] < 0, add the absolute value of max_RdDqCenterPoint_val_b[byte] to deskew on each bit.
				if(max_RdDqCenterPoint_val_b[byte] < 0) {
					arr_RdDqBitDeSkew[(byte * DQ_NUM_BITS_PER_BYTE) + bit_indx] += __abs(max_RdDqCenterPoint_val_b[byte]);
				}				

				CSR_WRITE(rAMP_RDDQDESKEW_CTRL(byte, ch, bit_indx), arr_RdDqBitDeSkew[(byte * DQ_NUM_BITS_PER_BYTE) + bit_indx] | RUNDESKEWUPD);

				// Poll for Deskew code to be updated
				SPIN_W_TMO_WHILE (CSR_READ(rAMP_RDDQDESKEW_CTRL(byte, ch, bit_indx)) & RUNDESKEWUPD);
			
			}
			
			// If max_RdDqCenterPoint_val_b[byte] < 0, add the absolute value of max_RdDqCenterPoint_val_b[byte] to deskew on each bit.
			if(max_RdDqCenterPoint_val_b[byte] < 0) {
					arr_RdDqBitDeSkew[DQ_TOTAL_BITS + byte] += __abs(max_RdDqCenterPoint_val_b[byte]);
			}				
			
			// Also program DMI deskew here
			CSR_WRITE(rAMP_RDDMDESKEW_CTRL(byte, ch), arr_RdDqBitDeSkew[DQ_TOTAL_BITS + byte] | RUNDESKEWUPD);
		
			// Poll for Deskew code to be updated
			SPIN_W_TMO_WHILE (CSR_READ(rAMP_RDDMDESKEW_CTRL(byte, ch)) & RUNDESKEWUPD);

			// Also, program AMPS register for optimal RDDQ Vref
			if(cfg_params.freq_bin == 1) {
				amp_program_rddq_vref(ch, cfg_params.rddq_vref_values[vref_opt_rddq[ch]], 0, 1);
			} else if(cfg_params.freq_bin == 0) {
				amp_program_rddq_vref(ch, cfg_params.rddq_vref_values[vref_opt_rddq[ch]], 0, 0);
			}
		}
	} // for (ch = 0; ch < cfg_params.num_channels; ch++)
}

_ROMCODE_SEGMENT_PREFIX static uint32_t rddqcal_encode_dlyval(uint32_t ch, uint32_t phy_type, uint32_t val)
{
	uint32_t ret_val, mdll;

	mdll = mdllcode[ch][phy_type];

	//shim_printf("DEBUG - rddqcal_encode_dlyval:: mdll=%d, phy_type = %d, val = %d \n", mdll, phy_type, val);

	if (val > ( 1.75 * mdll )) {
		ret_val = 3;
	} else if (val > ( 1.5 * mdll )) {
		ret_val = 2;
	} else if (val > ( 1.0 * mdll )) {
		ret_val = 1;
	} else {
		ret_val = 0;
	}
	
	return ret_val;
}

_ROMCODE_SEGMENT_PREFIX void wrlvlcal_push_to_0s_region(uint32_t ch, uint32_t rnk)
{
	uint32_t byte;
	uint32_t cawrlvlcode = 0;
	bool max_tap_value_reached = false;
	uint32_t wrlvlrun = 0x3;
	uint32_t wrlvlresult = 0x3;
	uint32_t dqwrlvlcode[DQ_NUM_BYTES];
	
	// Note that incrementing cawrlvl sdll has opposite effect of incrementing dqwrlvl

	// Also program DQS SDLL to zero here. Because DQS SDLL is MDLL code at this point.
	// This will make CK-DQS worst case skew as MDLL code + 200ps which cannot be calibrated.
	for(byte = 0 ; byte < DQ_NUM_BYTES; byte++ ) {
		// Set the DQ/DQS SDLL (RESULT) both to ZERO for this channel/byte
		wrdqcal_set_delay_point_res(ch, 1 << byte, 0);
	}
	
	for(byte = 0 ; byte < DQ_NUM_BYTES; byte++ ) {
		dqwrlvlcode[byte] = dqwrlvl_sdll_before_wrlvl[ch][byte]; 	
	}
	
	do {
		// If any byte lane shows that it returned a value of 1 - push ca wrlvl sdll out by 1 tap
		amp_set_cawrlvl_sdll(ch, cawrlvlcode);
		
		for (byte = 0; byte < DQ_NUM_BYTES; byte++) {
			// if this byte already showed a 0 during last run, push dqwrlvl sdll by 1 tap
			// this is done to ensure this byte remains at 0 despite cawrlvl sdll being incremented above
			if ((wrlvlrun & (1 << byte)) == 0) {
				dqwrlvlcode[byte]++;
				amp_set_dqwrlvl_sdll(ch, byte, dqwrlvlcode[byte]);
			}
		}
		
		// Run Wrlvl calibration in hw
		amp_run_wrlvlcal(ch, wrlvlrun);
		
		// result in reported in AMPWRLVLRESULT register
		wrlvlresult = ((CSR_READ(rAMP_CAWRLVLRESULT(ch)) >> WRLVL_RESULT) & 0x3);
		
		for (byte = 0; byte < DQ_NUM_BYTES; byte++) {
			if((wrlvlresult & (1 << byte)) == 0) {
				wrlvlrun &= ~(1 << byte);					
			}
		}

		//shim_printf("DEBUG - wrlvlcal_push_to_0s_region:: Write leveling result for ch=%d, with cawrlvlcode=%d is 0x%x \n",ch,cawrlvlcode, wrlvlresult);
		
		// Exit if ca or dq wrlvl sdlls reach max tap value
		if (cawrlvlcode == MAX_CAWRLVL_CODE) {
			max_tap_value_reached = true;
			if (wrlvlrun)
				shim_panic("Memory Wrlvl calibration: CAWRLVL sdll reached max tap value, yet all bytes not all 0s");
		} else {
		
			for (byte = 0; byte < DQ_NUM_BYTES; byte++) {
				if (dqwrlvlcode[byte] == MAX_DQWRLVL_CODE) {
					if (wrlvlrun)
						shim_panic("Memory Wrlvl calibration: DQ%d sdll reached max tap value, yet all bytes not all 0s", byte);
					max_tap_value_reached = true;
					break;
				}
			}
		}
		// Start write leveling run with CAWRLVL code of 0.
		cawrlvlcode++;
	} while (wrlvlrun && !max_tap_value_reached);
}

// Keep incrementing dqsdll until the byte shows 1s again. This counters the casdll that was incremented previously in order to show 0s
_ROMCODE_SEGMENT_PREFIX void wrlvlcal_find_0to1_transition(uint32_t ch, uint32_t rnk)
{
	uint32_t chrnk_indx, byte;
	uint32_t wrlvlrun;
	uint32_t wrlvlresult;
	bool max_tap_value_reached = false;
	uint32_t dqwrlvlcode[DQ_NUM_BYTES];
	uint32_t cawrlvlcode = CSR_READ(rAMP_CAWRLVLSDLLCODE(ch));
	
	wrlvlrun = 0x3;
	
	for (byte = 0; byte < DQ_NUM_BYTES; byte++) {
		dqwrlvlcode[byte] = CSR_READ(rAMP_DQWRLVLSDLLCODE(byte, ch));
	}
	
	chrnk_indx = loopchrnk_index_calc(ch, rnk, 0);
	
	do {
		for (byte = 0; byte < DQ_NUM_BYTES; byte++) {
			// if this byte is still showing a 0, increment the sdll
			if (wrlvlrun & (1 << byte)) {
				dqwrlvlcode[byte]++;
				amp_set_dqwrlvl_sdll(ch, byte, dqwrlvlcode[byte]);
			}
		}
		
		// run the wrlvl calibration in hw
		amp_run_wrlvlcal(ch, wrlvlrun);
		
		// result in reported in AMPWRLVLRESULT register
		wrlvlresult = ((CSR_READ(rAMP_CAWRLVLRESULT(ch)) >> WRLVL_RESULT) & 0x3);
		
		for (byte = 0; byte < DQ_NUM_BYTES; byte++) {
			if(wrlvlresult & (1 << byte)) {
				wrlvlrun &= ~(1 << byte);					
			}
		}

		//shim_printf("DEBUG - wrlvlcal_find_0to1_transition:: Write leveling result for ch=%d, with cawrlvlcode=%d, dqwrlvlcode[%d] = %d, dqwrlvlcode[%d] = %d is %d \n",ch,cawrlvlcode,0,dqwrlvlcode[0],1,dqwrlvlcode[1],wrlvlresult); 
		
		// Exit if any of the byte lane's sdll reaches max
		for (byte = 0; byte < DQ_NUM_BYTES; byte++) {
			if (dqwrlvlcode[byte] == MAX_DQWRLVL_CODE) {
				if (wrlvlrun)
					shim_panic("Memory Wrlvl calibration: DQ%d sdll reached max tap value, yet all bytes not all 1s", byte);
				max_tap_value_reached = true;
				break;
			}
		}
	} while (wrlvlrun && !max_tap_value_reached);
	
	// save the per byte codes for this channel and rank
	for (byte = 0; byte < DQ_NUM_BYTES; byte++)
		wrlvlcal_per_chrnk_rise[chrnk_indx][byte] = dqwrlvlcode[byte];
	// in the "3rd byte" entry, save the cawrlvl code
	wrlvlcal_per_chrnk_rise[chrnk_indx][byte] = cawrlvlcode;
}

// Go back towards the 0s region (that was found earlier). Note: not trying to find the next edge, just the previous edge that was found already
_ROMCODE_SEGMENT_PREFIX void wrlvlcal_find_1to0_transition(uint32_t ch, uint32_t rnk)
{
	uint32_t chrnk_indx, byte;
	uint32_t wrlvlrun;
	uint32_t wrlvlresult;
	bool max_tap_value_reached = false;
	uint32_t dqwrlvlcode[DQ_NUM_BYTES];
	uint32_t cawrlvlcode = CSR_READ(rAMP_CAWRLVLSDLLCODE(ch));
	bool incr_cawrlvl = false;
	
	chrnk_indx = loopchrnk_index_calc(ch, rnk, 0);
	wrlvlrun = 0x3;

	// jump ahead by SOLID_PASS_DETECT into the 1s region
	for (byte = 0; byte < DQ_NUM_BYTES; byte++) {
		dqwrlvlcode[byte] = CSR_READ(rAMP_DQWRLVLSDLLCODE(byte, ch));
		dqwrlvlcode[byte] += (SOLID_PASS_DETECT + 1); // + 1 because code is decremented before programming the sdll
	}
			
	do {
		// Make sure dqwrlvlsdll > 0, otherwise switch to cawrlvlsdll
		for (byte = 0; (byte < DQ_NUM_BYTES) && !incr_cawrlvl; byte++) {
			// Move to incrementing CAWRLVL SDLL only if the DQWRLVL code for the
			// byte which hasn't moved to 0's reached a zero code.
			if ((dqwrlvlcode[byte] == 0) && (wrlvlrun & (1 << byte)) )
				incr_cawrlvl = true;
		}
		
		// if we've reached 0 on any dqwrlvlsdll that were being decremented, switch to incrementing the cawrlvlsdll (same effect)
		if (incr_cawrlvl) {
			cawrlvlcode++;
			amp_set_cawrlvl_sdll(ch, cawrlvlcode);
			
			// In order to keep bytes that have transitioned to 0 to stay there, increment dqwrlvlsdll (counters effect of incrementing cawrlvlsdll)
			for (byte = 0; byte < DQ_NUM_BYTES; byte++) {
				if ((wrlvlrun & (1 << byte)) == 0) {
					dqwrlvlcode[byte]++;
					amp_set_dqwrlvl_sdll(ch, byte, dqwrlvlcode[byte]);
				}
			}
		} else {
			// if run bit is set for this byte, push out the new sdll value after decrementing by 1
			for (byte = 0; byte < DQ_NUM_BYTES; byte++) {
				if (wrlvlrun & (1 << byte)) {
					dqwrlvlcode[byte]--;
					amp_set_dqwrlvl_sdll(ch, byte, dqwrlvlcode[byte]);
				}
			}
		}
		
		// run the wrlvl calibration in hw
		amp_run_wrlvlcal(ch, wrlvlrun);
		
		// result in reported in AMPWRLVLRESULT register
		wrlvlresult = ((CSR_READ(rAMP_CAWRLVLRESULT(ch)) >> WRLVL_RESULT) & 0x3);
		
		for (byte = 0; byte < DQ_NUM_BYTES; byte++) {
			if((wrlvlresult & (1 << byte)) == 0) {
				wrlvlrun &= ~(1 << byte);					
			}
		}

		//shim_printf("DEBUG - wrlvlcal_find_1to0_transition:: Write leveling result for ch=%d, with cawrlvlcode=%d, dqwrlvlcode[%d] = %d, dqwrlvlcode[%d] = %d is %d \n",ch,cawrlvlcode,0,dqwrlvlcode[0],1,dqwrlvlcode[1],wrlvlresult); 
		// check if we've reached max tap value
		if (incr_cawrlvl && (cawrlvlcode == MAX_CAWRLVL_CODE)) {
			max_tap_value_reached = true;
			if (wrlvlrun)
				shim_panic("Memory Wrlvl calibration: max tap value reached, yet all bytes not back to 0s");
		}
		
	} while (wrlvlrun && !max_tap_value_reached);
	
	// save the per byte codes for this channel and rank
	for (byte = 0; byte < DQ_NUM_BYTES; byte++)
		wrlvlcal_per_chrnk_fall[chrnk_indx][byte] = dqwrlvlcode[byte];

	// in the "3rd byte" entry, save the cawrlvl code
	wrlvlcal_per_chrnk_fall[chrnk_indx][byte] = cawrlvlcode;
}

_ROMCODE_SEGMENT_PREFIX void wrlvlcal_program_final_values(void)
{
	uint32_t ch, chrnk0_indx, chrnk1_indx;
	uint32_t rank_rise_val[AMP_MAX_RANKS_PER_CHAN], rank_fall_val[AMP_MAX_RANKS_PER_CHAN];
	uint32_t edge_pos[AMP_MAX_RANKS_PER_CHAN];
	uint32_t common_edge_pos, min_edge_pos;
	uint32_t byte;
	uint32_t saved_val[DQ_NUM_BYTES + 1];
	
#if ENV_DV
	uint32_t loopchrnk000 = loopchrnk_index_calc(0, 0, 0);
	
	for(ch = cfg_params.num_channels_runCalib; ch < cfg_params.num_channels; ch++) {
		if (ch == cfg_params.num_channels_runCalib)
			shim_printf("DEBUG - %s:: Preloading non-calibrated channels "
				    "with channel 0's calibration results \n", __PRETTY_FUNCTION__);
		// For channels on which calibration was not done, load calibration results from channel 0.
		chrnk0_indx = loopchrnk_index_calc(ch, 0, 0);

		for (byte = 0; byte < (DQ_NUM_BYTES + 1); byte++) {
			wrlvlcal_per_chrnk_rise[chrnk0_indx][byte] = wrlvlcal_per_chrnk_rise[loopchrnk000][byte];
			wrlvlcal_per_chrnk_fall[chrnk0_indx][byte] = wrlvlcal_per_chrnk_fall[loopchrnk000][byte];
		}
	}
#endif

	for (ch = 0; ch < cfg_params.num_channels; ch++) {
		chrnk0_indx = loopchrnk_index_calc(ch, 0, 0);
		// we go upto DQ_NUM_BYTES + 1 to also take into account the cawrlvlcode that is stored in the 5th element
		for (byte = 0; byte < (DQ_NUM_BYTES + 1); byte++) {
			
			// Rank 0
			rank_rise_val[0] = wrlvlcal_per_chrnk_rise[chrnk0_indx][byte];
			rank_fall_val[0] = wrlvlcal_per_chrnk_fall[chrnk0_indx][byte];
			// average of 2 values is the edge for this rank
			edge_pos[0] = (rank_rise_val[0] + rank_fall_val[0]) >> 1;
			common_edge_pos = edge_pos[0];

			// Adjust for Dual ranks
			if (cfg_params.num_ranks > 1) {
				chrnk1_indx = loopchrnk_index_calc(ch, 1, 0);
				rank_rise_val[1] = wrlvlcal_per_chrnk_rise[chrnk1_indx][byte];
				rank_fall_val[1] = wrlvlcal_per_chrnk_fall[chrnk1_indx][byte];
				edge_pos[1] = (rank_rise_val[1] + rank_fall_val[1]) >> 1;
				
				// common_edge_pos between both ranks is simply their average
				common_edge_pos = (edge_pos[0] + edge_pos[1]) >> 1;
			}

			// save the wrlvlsdll for each byte (and the ca)
			saved_val[byte] = common_edge_pos;
		}
		
		// Find the min among all bytes (and the ca)
		min_edge_pos = saved_val[DQ_NUM_BYTES];		// initialize min as the cawrlvlsdll
		for (byte = 0; byte < DQ_NUM_BYTES; byte++)
			if (saved_val[byte] < min_edge_pos)
				min_edge_pos = saved_val[byte];
		
		// We'll subtract the min from all 3 sdlls, including ca
		// so the byte sdlls which are in opposite direction also need to be adjusted
		for (byte = 0; byte < (DQ_NUM_BYTES + 1); byte++) {
			saved_val[byte] -= min_edge_pos;
			
			// Program the values into the registers
			if (byte == DQ_NUM_BYTES) {
				// cawrlvl (use dlysel, which will require phyupdt and forceckelow)
				// CP : <rdar://problem/15702542>
				// Make sure WrLvl max field is set to zero before updating CAWRLVL SDLL code.
				amp_program_cawrlvl_sdll(ch, saved_val[byte], 0);
				shim_printf("wrlvlcal_program_final_values:: Saving 0x%x into CA WRLVL SDLL \n", saved_val[byte]);
			} else {
				// dqwrlvl (use dlysel, which will require phyupdt and forceckelow)
				// CP : <rdar://problem/15702542>
				// Make sure WrLvl max field is set to zero before updating CAWRLVL SDLL code.
				amp_program_dqwrlvl_sdll(ch, byte, saved_val[byte], 0);
				shim_printf("wrlvlcal_program_final_values:: Saving 0x%x into DQWRLVL%d SDLL\n", saved_val[byte], byte);
			}
		}
	}
}

/*
================================================================================
During the search for R/L pass/fail points, I notice this about each loop:
  1) Set the HW Parameters to reflect the current search point
       (This is often a combination of WrLvL, per-byte DQ/DQS SDLL)
  2) Assess the per-bit pass/fail status
  3) determine from the results of (2) whether or not we have found our desired skew
       (e.g. when looking for 'pass' do all bits pass?  when looking for 'fail' do all bits fail?)
  4) if not all bits have been satisfied, nudge the search point and repeat from (1)
  5) if all bits have been satisfied,
       Mark (per-bit) the skew at which that bit has notably found its left- or right- window_edge

We search in a +skew direction for the failing point (in Coarse Steps),
then search in a -skew direction from *there* for the passing point (in Fine Steps)

…and, when the (skew < 0) we program DQS SDLL instead of DQ SDLL
   (and in that case, WrLvL does not tract DQS…it only ever tracks DQ)

…the net result is, of course, that we end up with an array of left- and right- window-edges for each bit

The Maui twist is: we execute this process for each vref
The WRDQ speciality is:  within each vref loop iteration, we will first set "n" == 0 or 1, as appropriate

After finding all the L/R window_edges (for each vref), we can pass this info through the CoM
(Center of Mass) algorithm, as we did for CA and RDDQ, to get the optimal vref and the ideal SDLL
along with the individual bit skews.

So…how's the summary?

Some thoughts:
(A)  I'm planning to create an encapsulated function that will program WrLvl, DQ, DQS as appropriate for a given
       skew.  Using this within the Loops will clean up a lot of the code in the loop.
       So…..for a given skew (and an initial value of WrLvL), the function can do this:
         if (skew >= 0)
                WrLvL + skew --> WrLvL Reg
                skew -->  each Byte DQ Reg
          else // (skew < 0)
                abs(skew) -->  each Byte DQS Reg
       …making it an easy matter to call this from the various search loops
       Feedback?  does that feel like the right logic?

(B)   I'm still examining the "How do I know I'm done searching" logic in each loop.
        I suspect I can get this into an easy-to-use encapsulated function also.
================================================================================
*/

_MCODE_SEGMENT_PREFIX static uint8_t wrdqcal_chan = 0;
_MCODE_SEGMENT_PREFIX static uint8_t wrdqcal_rank = 0;
_MCODE_SEGMENT_PREFIX static uint8_t wrdqcal_vref = 0;

// Initialize Registers for the start of WR DQ Calibration
_ROMCODE_SEGMENT_PREFIX void wrdqcal_init_regs(uint8_t chan, uint8_t rank)
{
	// Program tWL, tRL for frequency bins 0-3 --> Should be part of the init sequence
	uint32_t rdwr_dqcal_loopcnt = CSR_READ(rAMP_RDWRDQCALLOOPCNT(chan));

  	wrdqcal_chan = chan;
  	wrdqcal_rank = rank;

	// Program SWRDDQCAL Loop count
	CSR_WRITE(rAMP_RDWRDQCALLOOPCNT(chan), (rdwr_dqcal_loopcnt & 0xFFFF00FF) | ((cfg_params.wrdq_cal_loopcnt & 0xFF) << 8));

	// Pattern and invert mask are programmed from init sequence. This includes setting Mode registers
	// MR15, MR20, MR32 and MR40
	
	// Program PRBS7I patterns
	CSR_WRITE(rAMP_CALPATPRBS7_0(chan), cfg_params.prbs7_pattern_0);
	CSR_WRITE(rAMP_CALPATPRBS7_1(chan), cfg_params.prbs7_pattern_1);
	CSR_WRITE(rAMP_CALPATPRBS7_2(chan), cfg_params.prbs7_pattern_2);
	CSR_WRITE(rAMP_CALPATPRBS7_3(chan), cfg_params.prbs7_pattern_3);
	
	// Program start points in PRBS7I patterns for all CA bits
	CSR_WRITE(rAMP_WRDQCALPATSEED_B0_0(chan), (cfg_params.wrdq_prbs_seed[0][3] << 24) | (cfg_params.wrdq_prbs_seed[0][2] << 16) | (cfg_params.wrdq_prbs_seed[0][1] << 8) | (cfg_params.wrdq_prbs_seed[0][0] << 0));
	CSR_WRITE(rAMP_WRDQCALPATSEED_B0_1(chan), (cfg_params.wrdq_prbs_seed[0][7] << 24) | (cfg_params.wrdq_prbs_seed[0][6] << 16) | (cfg_params.wrdq_prbs_seed[0][5] << 8) | (cfg_params.wrdq_prbs_seed[0][4] << 0));
	CSR_WRITE(rAMP_WRDQCALPATSEED_B0_2(chan), (cfg_params.wrdq_prbs_seed[0][8] << 0));
	CSR_WRITE(rAMP_WRDQCALPATSEED_B1_0(chan), (cfg_params.wrdq_prbs_seed[1][3] << 24) | (cfg_params.wrdq_prbs_seed[1][2] << 16) | (cfg_params.wrdq_prbs_seed[1][1] << 8) | (cfg_params.wrdq_prbs_seed[1][0] << 0));
	CSR_WRITE(rAMP_WRDQCALPATSEED_B1_1(chan), (cfg_params.wrdq_prbs_seed[1][7] << 24) | (cfg_params.wrdq_prbs_seed[1][6] << 16) | (cfg_params.wrdq_prbs_seed[1][5] << 8) | (cfg_params.wrdq_prbs_seed[1][4] << 0));
	CSR_WRITE(rAMP_WRDQCALPATSEED_B1_2(chan), (cfg_params.wrdq_prbs_seed[1][8] << 0));
}

// 'byte' is used in a number of these functions to indicate *which* byte lane's  registers are being affected.
// (byte & 0x1) indicates usage for DQ0
// (byte & 0x2) indicates usage for DQ1
#define byteN(_N)		(0x1 << (_N))
#define isforbyte(_N,byte)	((byte) & byteN(_N))

static inline _ROMCODE_SEGMENT_PREFIX uint32_t wrdqcal_bitmask(uint32_t byte)
{
  uint32_t wr_bits_pass = 0x0;

  if (isforbyte(0,byte)) wr_bits_pass |= 0x100FF;
  if (isforbyte(1,byte)) wr_bits_pass |= 0x2FF00;

  return wr_bits_pass;
}

// Write Value to 'n' To Add (or not Add) a 1/2 Clk Delay
_ROMCODE_SEGMENT_PREFIX int wrdqcal_set_half_clk_delay(uint32_t byte, uint32_t val)
{
	uint32_t wrdqsdq_sdllcode;
  if (isforbyte(0, byte)) {
		wrdqsdq_sdllcode = CSR_READ(rAMP_WRDQSDQ_SDLLCTRL(0, wrdqcal_chan));
		wrdqsdq_sdllcode = (wrdqsdq_sdllcode & HALF_CLK_OVRVAL_MASK) | (val << HALF_CLK_OVRVAL_OFFSET) | (1 << RUN_SDLLUPDWRRES);
		CSR_WRITE(rAMP_WRDQSDQ_SDLLCTRL(0, wrdqcal_chan), wrdqsdq_sdllcode);
		while(CSR_READ(rAMP_WRDQSDQ_SDLLCTRL(0, wrdqcal_chan)) & (1 << RUN_SDLLUPDWRRES));
	}
  if (isforbyte(1, byte)) {
		wrdqsdq_sdllcode = CSR_READ(rAMP_WRDQSDQ_SDLLCTRL(1, wrdqcal_chan));
		wrdqsdq_sdllcode = (wrdqsdq_sdllcode & HALF_CLK_OVRVAL_MASK) | (val << HALF_CLK_OVRVAL_OFFSET) | (1 << RUN_SDLLUPDWRRES);
		CSR_WRITE(rAMP_WRDQSDQ_SDLLCTRL(1, wrdqcal_chan), wrdqsdq_sdllcode);
		while(CSR_READ(rAMP_WRDQSDQ_SDLLCTRL(1, wrdqcal_chan)) & (1 << RUN_SDLLUPDWRRES));
	}

  	return 0;
}

// Set DQS SDLL and DQ SDLL
_ROMCODE_SEGMENT_PREFIX int wrdqcal_set_dqs_dq_sdll_type(uint32_t chan, uint32_t byte, uint32_t dq_val, uint32_t dqs_val, bool result)
{
	uint32_t wrdqsdq_sdllcode;
	uint32_t shyfted_bit  = 1 << (result? WR_SDLLUPDWRRES:WR_SDLLUPDOVR);
	uint32_t update = (dqs_val << WR_DQS_SDLL_OFFSET) | (dq_val << WR_DQ_SDLL_OFFSET) | shyfted_bit;
	
	// Range Check DQ, DQS ... if outside of max range, issue a panic
	if ((dqs_val > MAX_SDLL_VAL) || (dq_val > MAX_SDLL_VAL)) {
		//shim_panic("Trying to set WR %s SDLL (code == %d) which is more than (max == %d)",
		//	   (dqs_val > MAX_SDLL_VAL)? "DQS":"DQ", dq_val + dqs_val, MAX_SDLL_VAL);
		shim_printf("PANIC during Vref scan. Record and move forward:Trying to set WR %s SDLL (code == %d) which is more than (max == %d)",
			   (dqs_val > MAX_SDLL_VAL)? "DQS":"DQ", dq_val + dqs_val, MAX_SDLL_VAL);

		wrdqcal_vref_panic[chan][wrdqcal_vref] = 1;
		return -1;
	}
	
	// First, update each byte
	if (isforbyte(0, byte)) {
		wrdqsdq_sdllcode  = CSR_READ(rAMP_WRDQSDQ_SDLLCTRL(0, chan));
		wrdqsdq_sdllcode &= 0x0000FFFF;
		wrdqsdq_sdllcode |= update;
		CSR_WRITE(rAMP_WRDQSDQ_SDLLCTRL(0, chan), wrdqsdq_sdllcode);
	}
	if (isforbyte(1, byte)) {
		wrdqsdq_sdllcode = CSR_READ(rAMP_WRDQSDQ_SDLLCTRL(1, chan));
		wrdqsdq_sdllcode &= 0x0000FFFF;
		wrdqsdq_sdllcode |= update;
		CSR_WRITE(rAMP_WRDQSDQ_SDLLCTRL(1, chan), wrdqsdq_sdllcode);
	}
	
	// Then, poll each byte for update to finish
	if (isforbyte(0, byte))
		SPIN_W_TMO_WHILE (CSR_READ(rAMP_WRDQSDQ_SDLLCTRL(0, chan)) & shyfted_bit);
	if (isforbyte(1, byte))
		SPIN_W_TMO_WHILE (CSR_READ(rAMP_WRDQSDQ_SDLLCTRL(1, chan)) & shyfted_bit);
	
	return 0;
}

//  Set DQ/DQS SDLL (per-byte)  as a RESULT (true) or OVERRIDE (false)
_ROMCODE_SEGMENT_PREFIX int wrdqcal_set_delay_point(uint32_t chan, uint32_t byte, int32_t point, bool result)
{
	uint32_t dq  = 0;
	uint32_t dqs = 0;

	// if point is outside of range (min,max), return error code (+/- 1)
	if ((point > MAX_SDLL_VAL) || (point < -MAX_SDLL_VAL)) {
		//shim_panic("Trying to set a WR DQ/DQS code which is more than maximum allowed (%d vs. [%d,%d])",
		//	   point, -MAX_SDLL_VAL, MAX_SDLL_VAL);
		shim_printf("PANIC during Vref scan. Record and move forward:Trying to set a WR DQ/DQS code which is more than maximum allowed (%d vs. [%d,%d])",
			   point, -MAX_SDLL_VAL, MAX_SDLL_VAL);
		wrdqcal_vref_panic[chan][wrdqcal_vref] = 1;
		return -1;
	}
	
	if (point >= 0) {	// (+point) --> DQ  SDLL; ZERO --> DQS SDLL
		dq  =  point;
	} else {		// (-point) --> DQS SDLL; ZERO --> DQ  SDLL
		dqs = -point;
	}
	
	return wrdqcal_set_dqs_dq_sdll_type(chan, byte, dq, dqs, result);
}

// Program DQ deskew code.
_ROMCODE_SEGMENT_PREFIX int wrdqcal_program_dq_deskew(uint32_t chan, uint32_t byte, uint32_t val)
{
	uint32_t bit_indx;

	if (isforbyte(0, byte)) {
		for (bit_indx = 0; bit_indx < DQ_NUM_BITS_PER_BYTE; bit_indx++) {
			CSR_WRITE(rAMP_WRDQDESKEW_CTRL(0, chan, bit_indx), (val) | RUNDESKEWUPD);
			
			// Poll for deskew update to be done.
			while(CSR_READ(rAMP_WRDQDESKEW_CTRL(0, chan, bit_indx)) & RUNDESKEWUPD);
		}
		
		// Also program DMI deskew here.
		CSR_WRITE(rAMP_WRDMDESKEW_CTRL(0, chan), (val) | RUNDESKEWUPD);

		// Poll for deskew update to be done.
		while(CSR_READ(rAMP_WRDMDESKEW_CTRL(0, chan)) & RUNDESKEWUPD);	
	}
	if (isforbyte(1, byte)) {
		for (bit_indx = 0; bit_indx < DQ_NUM_BITS_PER_BYTE; bit_indx++) {
			CSR_WRITE(rAMP_WRDQDESKEW_CTRL(1, chan, bit_indx), (val) | RUNDESKEWUPD);
			
			// Poll for deskew update to be done.
			while(CSR_READ(rAMP_WRDQDESKEW_CTRL(1, chan, bit_indx)) & RUNDESKEWUPD);
		}
		
		// Also program DMI deskew here.
		CSR_WRITE(rAMP_WRDMDESKEW_CTRL(1, chan), (val) | RUNDESKEWUPD);

		// Poll for deskew update to be done.
		while(CSR_READ(rAMP_WRDMDESKEW_CTRL(1, chan)) & RUNDESKEWUPD);	
	}

	return 0;
}

// Set DQS SDLL and DQ SDLL as OVERRIDE
_ROMCODE_SEGMENT_PREFIX int wrdqcal_set_delay_point_ovr(uint32_t chan, uint32_t byte, int32_t point)
{
	return wrdqcal_set_delay_point(chan, byte, point, false);
}
		
// Set DQS SDLL and DQ SDLL as RESULT
_ROMCODE_SEGMENT_PREFIX int wrdqcal_set_delay_point_res(uint32_t chan, uint32_t byte, int32_t point)
{
	return wrdqcal_set_delay_point(chan, byte, point, true);
}
		
// WrLvl value and dq_sdll (per-byte) array
_ROMCODE_SEGMENT_PREFIX int wrdqcal_set_search_point(uint32_t byte, int32_t point)
{
  uint32_t chrnk_indx;
  uint32_t mdllcode_byte;
  uint32_t min_of_4mdll_maxsdll;
  uint32_t deskew_val;

  // if point is outside of 4*MDLL range, return error code (+/- 1)
  mdllcode_byte = (isforbyte(1, byte)) ? mdllcode[wrdqcal_chan][AMP_DQ1] : mdllcode[wrdqcal_chan][AMP_DQ0];
  if ((point < 0) && (((uint32_t)__abs(point)) > (4 * mdllcode_byte) || __abs(point) > MAX_SDLL_VAL)) {
	 shim_printf("PANIC during Vref scan. Trying to set a WR DQS code which is more than 4*MDLL, Code=%d, Limit=%d \n", __abs(point), MAX(4 * mdllcode_byte, MAX_SDLL_VAL));
	 wrdqcal_vref_panic[wrdqcal_chan][wrdqcal_vref] = 1;
    return -1;
  } else {
  		if (((uint32_t)__abs(point)) > (5 * mdllcode_byte)) {
			shim_printf("PANIC during Vref scan. Trying to set a WR DQ SDLL + WR DQ deskew code more than 5*MDLL (DQ deskew can hold a code of MDLL), Code=%d, Limit=%d \n", __abs(point), (5 * mdllcode_byte));
	 		wrdqcal_vref_panic[wrdqcal_chan][wrdqcal_vref] = 1;
    		return -1;
  		}
  }

  if (point >= 0) {
    // (+point) --> DQ  SDLL; ZERO --> DQS SDLL

    if ( (point > (MAX_SDLL_VAL + MAX_DESKEW_PROGRAMMED) ) || (((uint32_t)point) > (5*mdllcode_byte)) ) {
      shim_printf("PANIC during Vref scan. Trying to set a WR DQ SDLL + WR DQ deskew code which is more than maximum allowed \n");
	 	wrdqcal_vref_panic[wrdqcal_chan][wrdqcal_vref] = 1;
      return -1;
     }
    else {
       // Compute Min(4*MDLL, 248)
		 min_of_4mdll_maxsdll = MIN(4*mdllcode_byte, MAX_SDLL_VAL);

       if(((uint32_t)point) > min_of_4mdll_maxsdll) {
         deskew_val = point - min_of_4mdll_maxsdll;
			wrdqcal_set_delay_point_ovr(wrdqcal_chan, byte, min_of_4mdll_maxsdll);
         wrdqcal_program_dq_deskew(wrdqcal_chan, byte, deskew_val);
       }
       else if (((uint32_t)point) <= min_of_4mdll_maxsdll) {
			wrdqcal_set_delay_point_ovr(wrdqcal_chan, byte, point);
       }
    } 
			
	chrnk_indx = (wrdqcal_chan * cfg_params.num_ranks) + wrdqcal_rank;
	
  	amp_set_cawrlvl_sdll(wrdqcal_chan, cawrlvl_sdll_wrlvl[wrdqcal_chan]);
  } else {
    // if (point <  0), (-point) --> DQS SDLL; ZERO --> DQ  SDLL
	 wrdqcal_set_delay_point_ovr(wrdqcal_chan, byte, point);
  
	chrnk_indx = (wrdqcal_chan * cfg_params.num_ranks) + wrdqcal_rank;
	
  	amp_set_cawrlvl_sdll(wrdqcal_chan, cawrlvl_sdll_wrlvl[wrdqcal_chan] + (uint16_t)(-point));
  }
  return 0;
}

// Assess completion (pass/fail) for the hardware with current settings
_ROMCODE_SEGMENT_PREFIX uint32_t wrdqcal_assess_position()
{
  uint32_t wrdqcalresult;
  uint32_t wrdqcalrun;

  wrdqcalrun = CSR_READ(rAMP_WRDQ_CAL_RUN(wrdqcal_chan));	
  
  // Run SW WRDQ calibration
  CSR_WRITE(rAMP_WRDQ_CAL_RUN(wrdqcal_chan), (wrdqcalrun & 0xFFFFFFFE) | (1 << WR_CALIB_RUN)); 

  // Poll for WRDQ cal run to finish
  SPIN_W_TMO_WHILE (CSR_READ(rAMP_WRDQ_CAL_RUN(wrdqcal_chan)) & (1 << WR_CALIB_RUN));

  // Determine Pass/Fail for each bit of each Byte
  wrdqcalresult = (CSR_READ(rAMP_WRDQ_CAL_RESULT(wrdqcal_chan)) & 0x3FFFF);

  return wrdqcalresult;
}

// Return number of consequetive PASS positions from the indicated position that can be found (up to max)
_ROMCODE_SEGMENT_PREFIX uint8_t wrdqcal_solid_pass(uint32_t byte, int32_t point, int step, uint8_t max)
{
  uint8_t swatch_size = 0;
  uint32_t result;
  uint32_t wr_bits_pass = wrdqcal_bitmask(byte);
  uint8_t pass;

  // Determine Pass/Fail for each bit of each Byte
  while (wrdqcal_set_search_point(byte, point) == 0) {
    result = wrdqcal_assess_position();
	 shim_printf("DEBUG - wrdqcal_solid_pass:: result = 0x%x \n", result);
    if (((result & wr_bits_pass) != wr_bits_pass) || (++swatch_size >= max))
      break;
    point += step;
  }
  pass = (swatch_size == max) ? 1 : 0;

  return pass;
}

// Return number of consecutive all bit fail positions from the indicated position that can be found (up to max)
_ROMCODE_SEGMENT_PREFIX uint8_t wrdqcal_all_bits_fail(uint32_t byte, int32_t point, int step, uint8_t max)
{
  uint8_t swatch_size = 0;
  uint32_t result;
  uint32_t wr_bits_mask = wrdqcal_bitmask(byte);
  uint32_t wr_bits_fail = 0x00000;
  uint8_t pass;

  // Determine Pass/Fail for each bit of each Byte
  while (wrdqcal_set_search_point(byte, point) == 0) {
    result = wrdqcal_assess_position();
	 shim_printf("DEBUG - wrdqcal_all_bits_fail:: result = 0x%x \n", result);
    if (((result & wr_bits_mask) != wr_bits_fail) || (++swatch_size >= max))
      break;
    point += step;
  }

  pass = (swatch_size == max) ? 1 : 0;

  return pass;
}

// Find SOLID pass point (and skew) for each bit from the current "All-Fail" (both bytes) Point
#define WITHIN_BOUNDS(_p,_s,_lim)	((_s < 0)? (_p > _lim):(_p < _lim))
_ROMCODE_SEGMENT_PREFIX void wrdqcal_find_perbit_skew(uint32_t byte, int edge, int step, int limit, int16_t *bit_edge)
{
  	uint32_t per_bit_pass;
  	uint32_t pass_result;
  	uint32_t bit,bitval;
  	int skew;
	uint32_t SolidBitPass[DQ_TOTAL_BITS + DMI_TOTAL_BITS] = { 0 };
   uint32_t BitPass[DQ_TOTAL_BITS + DMI_TOTAL_BITS] = { 0 };	
	uint8_t all_bits_pass = 0;

  	per_bit_pass = wrdqcal_bitmask(byte);
	shim_printf("DEBUG - wrdqcal_find_perbit_skew:: per_bit_pass=0x%x \n",per_bit_pass);

  // the 'edge' is the extreme position for All-Bits-Pass
  // how far can we push it to find failures on each bit?
  for(skew = edge + step; WITHIN_BOUNDS(skew, step, limit); skew += step) {
    	// Determine Pass/Fail for each bit of each Byte
    	wrdqcal_set_search_point(byte, skew);
    	pass_result = wrdqcal_assess_position();

    	per_bit_pass = wrdqcal_bitmask(byte) & pass_result;

		shim_printf("DEBUG - wrdqcal_find_perbit_skew:: pass_result=0x%x, per_bit_pass=0x%x, skew = %d \n", pass_result, per_bit_pass, skew);


    	for(bit=0, bitval=0x01; bit < (DQ_TOTAL_BITS + DMI_TOTAL_BITS); bit++,bitval<<=1) {
      	if((per_bit_pass & bitval) && (BitPass[bit] == 0)) {
				// Has this bit passed SOLID_PASS_DETECT number of times? Then consider it done
				if (SolidBitPass[bit] == SOLID_PASS_DETECT) {
					BitPass[bit] = 1;
					shim_printf("DEBUG - wrdqcal_find_perbit_skew:: Found solid pass for bit %d at sdll=%d \n",bit, bit_edge[bit]);
				} else if (SolidBitPass[bit] > 0) {
					SolidBitPass[bit] = SolidBitPass[bit] + 1;
				} else {
					// bit passed for the first time, record this value in the global array as the left edge
					SolidBitPass[bit] = SolidBitPass[bit] + 1;
					
					bit_edge[bit] = skew;	
				}
			} else {
				// bit failed calibration, reset the pass count to 0
				SolidBitPass[bit] = 0;
			}
	  	}
		
		all_bits_pass = 1;
		for(bit = 0 ; bit < DQ_TOTAL_BITS + DMI_TOTAL_BITS ; bit++) {
			all_bits_pass &= BitPass[bit];
		}

		if(all_bits_pass) {
			shim_printf("DEBUG - wrdqcal_find_perbit_skew:: Found passing point for all bits \n");
			break;
		} 
	}

  // Always return ZERO UNLESS we hit the Max/Min Bounds
  if (all_bits_pass == 0) {
    //shim_panic("Unable to find passing point in WRDQ calibration \n");
    shim_printf("PANIC during Vref scan. Record and move forward:Unable to find passing point in WRDQ calibration \n");
	wrdqcal_vref_panic[wrdqcal_chan][wrdqcal_vref] = 1;
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////
// Do the WrDQ Calibration
////////////////////////////////////////////////////////////////////////////////////////////////
_ROMCODE_SEGMENT_PREFIX void wrdqcal_sequence(uint8_t chan, uint8_t rank)
{
  int32_t point = 0;
  int32_t zone, left_allfail, right_allfail;
  int16_t left_edge[DQ_TOTAL_BITS + DMI_TOTAL_BITS] = {0};
  int16_t right_edge[DQ_TOTAL_BITS + DMI_TOTAL_BITS] = {0};

  uint32_t both_bytes = 0x3;
  uint32_t vref, bit;
  uint32_t SDLL_threshold_n_1;
  uint32_t SDLL_threshold_n_2;
  int chrnk_indx;
  uint32_t good_vref_not_found;

  // Initialize HW Parameters for the WR DQ Calibration
  wrdqcal_init_regs(chan, rank);
  chrnk_indx = loopchrnk_index_calc(chan, rank, 0);

  // Initialize DQ and DQS SDLL's to zero before starting 
  wrdqcal_set_search_point(both_bytes, 0);  
	
  if(cfg_params.wrdq_apply_NC_algorithm) {

  		// Program initial Vref setting
  		amp_program_wrdq_vref(chan, rank, cfg_params.wrdq_vref_values[0], 0); //vref_pulse_cnt=0

  		// ============================================================
  		// Determine the correct value for 'n': 1/2 Clk offset or not?
  		// Set n = 0 for both bytes
  		wrdqcal_set_half_clk_delay(both_bytes, 0);

  		shim_printf("DEBUG - wrdqcal_sequence:: Starting to determine the value of N \n");

		// N=0 threshold is set to 3*MDLL. N=1 threshold is set to 5*MDLL.
		// Also, make sure that DQ SDLL never exceeds 4*MDLL.
    	SDLL_threshold_n_1 = (3.0 * mdllcode[chan][AMP_CA]);
    	SDLL_threshold_n_2 = (5.0 * mdllcode[chan][AMP_CA]);

		shim_printf("DEBUG - wrdqcal_sequence:: SDLL_threshold_n_1=0x%x, SDLL_threshold_n_2=0x%x \n",SDLL_threshold_n_1, SDLL_threshold_n_2);

		// Keep incrementing in coarse steps until a P->F transition is found.
    	for(point = 0; ((uint32_t)point) < SDLL_threshold_n_2; point += COARSE_STEP_SZ) {
			

			// Look for one pass on all bits.
			if (wrdqcal_solid_pass(both_bytes, point, 1, 1)) {
				// In Phase 2, look for a fail on any of the DQ/DM bits.
	  			for(; ((uint32_t)point) < SDLL_threshold_n_2; point += COARSE_STEP_SZ) {
	    			if (!wrdqcal_solid_pass(both_bytes, point, 1, 1))
	      			break;
	  			}
	  			break;
     		}
		}

      // If we exceeded the Threshold, then set 'n' to ONE
      if (((uint32_t)point) >= SDLL_threshold_n_2) {
			// Set n = 2
			wrdqcal_set_half_clk_delay(both_bytes, 2);
  			shim_printf("DEBUG - wrdqcal_sequence:: Determined the value of N to be 2 \n");
      }
		else if (((uint32_t)point) >= SDLL_threshold_n_1) {
			// Set n = 1
			wrdqcal_set_half_clk_delay(both_bytes, 1);
  			shim_printf("DEBUG - wrdqcal_sequence:: Determined the value of N to be 1 \n");
      }
		else {
  			shim_printf("DEBUG - wrdqcal_sequence:: Determined the value of N to be 0 \n");
		}

  }
	// Reset deskews before calibrating.
	wrdqcal_program_dq_deskew(chan, both_bytes, 0);

  // per Vref:
  for(vref=0; vref < cfg_params.wrdq_num_vref; vref++) {
   
	 // Set global vref.
	 wrdqcal_vref = vref;
   
    // Program Vref setting
    amp_program_wrdq_vref(chan, rank, cfg_params.wrdq_vref_values[vref], 0); //vref_pulse_cnt=0
		
	 shim_printf("DEBUG - wrdqcal_sequence:: Done programming Vref value \n");

    // ============================================================
    // ZERO the arrays to hold Skew (Left and Right)
    for(bit=0; bit < (DQ_TOTAL_BITS + DMI_TOTAL_BITS); bit++)
      left_edge[bit] = right_edge[bit] = 0;			// ZERO all the skews (Left & Right)

    // ============================================================
    // Find Passing Zone for both bytes: Start @ZERO & Coarse-Step Left (+) ... Find "enough" Consecutive Pass-Points
    point = 0;
  	 // Once a passing point is found, go back a coarse step and switch to fine stepping. 
    for(; point <= MAX_SDLL_VAL; point += COARSE_STEP_SZ) {
      if (wrdqcal_solid_pass(both_bytes, point, 1, 1)) {
			// Found a passing point. Now go back a coarse step and switch to fine stepping.
			point -= COARSE_STEP_SZ;

			for(; point <= MAX_SDLL_VAL; point += FINER_STEP_SZ) {
      		if (wrdqcal_solid_pass(both_bytes, point, 1, SOLID_PASS_ZONE_DETECT)) {
					// Good Solid Pass on both bytes
					break;
				}
			}
			break;
      }
    }

    if (point > MAX_SDLL_VAL) {
      // We have failed to find any PASSing Zone at all!
      //shim_panic("Unable to find any passing zone in WRDQ calibration \n");
      shim_printf("PANIC during Vref scan. Record and move forward:Unable to find any passing zone in WRDQ calibration \n");
		wrdqcal_vref_panic[chan][vref] = 1;
    }
    // No Error...We found the Zone!
    zone = point;

    // ============================================================
    // From Passing Zone: Coarse-Step Left (+) until find Fail Zone
    // Record, per Bit, the location of the Left-Edge of the Passing Zone
    point = zone;
    for(; point <= (MAX_SDLL_VAL + MAX_DESKEW_PROGRAMMED); point += COARSE_STEP_SZ) {
      if (wrdqcal_all_bits_fail(both_bytes, point, 1, SOLID_FAIL)) {
			// Found all bits Fail Point
			break;
      }
    }

    if (point <= zone) {
      // Inexplicably, despite having already found one PASSing zone, we have failed to find any PASSing Zone at all!
      //shim_panic("Cannot find any PASSing Zone in WRDQ calibration \n");
      shim_printf("PANIC during Vref scan. Record and move forward:Cannot find any PASSing Zone in WRDQ calibration \n");
		wrdqcal_vref_panic[chan][vref] = 1;
    }

   // No Error...We found the all fail point on the Left side!
	 left_allfail = point;
    
	 // Next: Assess the skews for the various bits
	 // REVIEW 
    wrdqcal_find_perbit_skew(both_bytes, left_allfail, -1, -MAX_SDLL_VAL, left_edge);

    // ============================================================
    // From Passing Zone: Coarse-Step Right (-) until find Fail Zone
    // Record, per Bit, the location of the Right-Edge of the Passing Zone
    point = zone;
    for(; point >= -MAX_SDLL_VAL; point -= COARSE_STEP_SZ) {
      if (wrdqcal_all_bits_fail(both_bytes, point, -1, SOLID_FAIL)) {
			// Found all Fail Point
			break;
      }
    }

    if (point > zone) {
      // Inexplicably, despite having already found one PASSing zone, we have failed to find any PASSing Zone at all!
      //shim_panic("Cannot find any PASSing Zone \n");
      shim_printf("PANIC during Vref scan. Record and move forward:Cannot find any PASSing Zone \n");
		wrdqcal_vref_panic[chan][vref] = 1;
    }
    // No Error...We found the Right Edge!
    right_allfail = point;
	 
	 shim_printf("DEBUG - Found the right edge at %d \n", point);

    // Next: Assess the skews for the various bits
    wrdqcal_find_perbit_skew(both_bytes, right_allfail, 1, MAX_SDLL_VAL, right_edge);

    // ============================================================
    // Note these values in the result array
    for(bit=0; bit < DQ_TOTAL_BITS + DMI_TOTAL_BITS; bit++) {
      wrdqcal_per_chrnk_left[chrnk_indx][vref][bit]  = left_edge[bit];
      wrdqcal_per_chrnk_right[chrnk_indx][vref][bit] = right_edge[bit];
    }

    // ============================================================
  } // Done with all Vref

  // Find if there is even a single Vref which did not panic.
  good_vref_not_found = 1;
  for(vref=0; vref < cfg_params.wrdq_num_vref; vref++) {
		good_vref_not_found &= wrdqcal_vref_panic[chrnk_indx][vref];
  }
  if(good_vref_not_found == 1) {
  		shim_panic("Memory WRDQ calibration: Unable to find any Vref which did not panic for channel %d\n", chrnk_indx);
  }	

  // ============================================================
  // Find Optimal Vref, CoM
  // Can we use the known skews to allow a more optimal 'optimal vref'?
  amp_compute_opt_vref_wrdqcal(chan, rank, DQ_TOTAL_BITS + DMI_TOTAL_BITS);

  // ============================================================
  // Considering the Left and Right Skew:
  // per DQ Byte, normalize the SDLL vs. perBit deskew so that at least *one* of the perBit deskews is ZERO

  // ============================================================
  // Program Final Values
  // cntr_opt_wrdq[chrnk_indx] = optimal.center;
  // optimal.center --> SDLL
  wrdqcal_set_search_point(1 << 0, cntr_opt_wrdq[chrnk_indx]);
  wrdqcal_set_search_point(1 << 1, cntr_opt_wrdq[chrnk_indx]);
}


_ROMCODE_SEGMENT_PREFIX void wrdqcal_program_final_values(void)
{
	uint32_t ch, bit_indx, byte;
	uint32_t chrnk0_indx;
	int32_t arr_WrDqBitCenterPoint[DQ_TOTAL_BITS + DMI_TOTAL_BITS];
	int32_t arr_WrDqBitDeSkew[DQ_TOTAL_BITS + DMI_TOTAL_BITS];
	int32_t left_pos_val;
	int32_t right_pos_val;
	int32_t min_WrDqCenterPoint_val_b[DQ_NUM_BYTES];
	int32_t max_of_min_WrDqCenterPoint_val_b;
	int32_t arr_WrDmiDeSkew[DQ_NUM_BYTES] = { 0 };
	uint32_t wrdqcalvrefcodecontrol;
	uint32_t vref_ch;
#if ENV_DV
	uint32_t loopchrnk000 = loopchrnk_index_calc(0, 0, 0);
	uint32_t vref0 = vref_opt_wrdq[loopchrnk000];
	
	uint32_t N;
	uint32_t wrdqsdq_sdllcode;

	for(ch = cfg_params.num_channels_runCalib; ch < cfg_params.num_channels; ch++) {
		if (ch == cfg_params.num_channels_runCalib)
			shim_printf("DEBUG - %s:: Preloading non-calibrated channels "
				    "with channel 0's calibration results \n", __PRETTY_FUNCTION__);
		// For channels on which calibration was not done, load calibration results from channel 0.
		chrnk0_indx = loopchrnk_index_calc(ch, 0, 0);
		vref_ch = vref_opt_wrdq[chrnk0_indx] = vref0; 
	
		for (bit_indx=0; bit_indx < DQ_TOTAL_BITS + DMI_TOTAL_BITS; bit_indx++) {
			wrdqcal_per_chrnk_left[chrnk0_indx][vref_opt_wrdq[chrnk0_indx]][bit_indx] = wrdqcal_per_chrnk_left[loopchrnk000][vref0][bit_indx];
			wrdqcal_per_chrnk_right[chrnk0_indx][vref_opt_wrdq[chrnk0_indx]][bit_indx] = wrdqcal_per_chrnk_right[loopchrnk000][vref0][bit_indx];
		}

		cawrlvl_sdll_wrlvl[ch] = cawrlvl_sdll_wrlvl[0];

		for (byte = 0; byte < DQ_NUM_BYTES; byte++) {
			dqwrlvl_sdll_after_wrlvl[ch][byte] = dqwrlvl_sdll_after_wrlvl[0][byte];
		}
	}

	// Also program value of N for all channels correctly. 
	shim_printf("DEBUG - wrdqcal_program_final_values:: Preloading non-calibrated channels with channel 0's N value \n");
	for(ch = cfg_params.num_channels_runCalib; ch < cfg_params.num_channels; ch++) {
		chrnk0_indx = loopchrnk_index_calc(ch, 0, 0); 
		
		for (byte = 0; byte < DQ_NUM_BYTES; byte++) {
			N = (CSR_READ(rAMP_WRDQSDQ_SDLLCTRL(byte, loopchrnk000)) >> HALF_CLK_OVRVAL_OFFSET) & 0x3;
			wrdqsdq_sdllcode = CSR_READ(rAMP_WRDQSDQ_SDLLCTRL(byte, chrnk0_indx));
			wrdqsdq_sdllcode = (wrdqsdq_sdllcode & HALF_CLK_OVRVAL_MASK) | (N << HALF_CLK_OVRVAL_OFFSET) | (1 << RUN_SDLLUPDWRRES);		
			CSR_WRITE(rAMP_WRDQSDQ_SDLLCTRL(byte, chrnk0_indx), wrdqsdq_sdllcode);
			while(CSR_READ(rAMP_WRDQSDQ_SDLLCTRL(byte, chrnk0_indx)) & (1 << RUN_SDLLUPDWRRES));
		}
	}
	#endif

	for (ch = 0; ch < cfg_params.num_channels; ch++) {
		chrnk0_indx = loopchrnk_index_calc(ch, 0, 0);
		vref_ch = vref_opt_wrdq[chrnk0_indx];
				
		// find the center point of passing window for each bit over all ranks
		for (bit_indx = 0; bit_indx < DQ_TOTAL_BITS + DMI_TOTAL_BITS; bit_indx++) {
			
			left_pos_val = wrdqcal_per_chrnk_left[chrnk0_indx][vref_ch][bit_indx];
			right_pos_val = wrdqcal_per_chrnk_right[chrnk0_indx][vref_ch][bit_indx];
			
			// find center of the eye for this bit
			arr_WrDqBitCenterPoint[bit_indx] = find_wrdq_center_of_eye(left_pos_val, right_pos_val);
			shim_printf("DEBUG - wrdqcal_program_final_values:: Center value for bit %d in Channel %d = %d \n", bit_indx, ch, arr_WrDqBitCenterPoint[bit_indx]);
		}
		
		// initialize the min centerpoint to the 1st bit's center point in each byte lane
		for (byte = 0; byte < DQ_NUM_BYTES; byte++)
			min_WrDqCenterPoint_val_b[byte] = arr_WrDqBitCenterPoint[byte * DQ_NUM_BITS_PER_BYTE];
		
		// Find the min CenterPoint per byte lane given each bit's center point
		for (bit_indx=0; bit_indx < DQ_TOTAL_BITS + DMI_TOTAL_BITS; bit_indx++) {
			
			byte = (bit_indx < DQ_TOTAL_BITS) ? (bit_indx >> 3) : (bit_indx - DQ_TOTAL_BITS); // bit_indx / DQ_NUM_BITS_PER_BYTE
			
			// if this bit's center point is less than current min, make it the new min
			min_WrDqCenterPoint_val_b[byte] = find_common_endpoint(min_WrDqCenterPoint_val_b[byte], arr_WrDqBitCenterPoint[bit_indx], MIN_ENDPT);
		}

		for (byte = 0; byte < DQ_NUM_BYTES; byte++) {
			shim_printf("DEBUG - wrdqcal_program_final_values:: Minimum center value for byte %d = %d \n", byte, min_WrDqCenterPoint_val_b[byte]);
		}

		// Compute the individual deskew values: any bits with center point > min for its byte lane will require deskew
		// Each bit's center is guaranteed to be >= min for its byte lane
		// Deskewing means adding more positive adjustment for this bit in addition to the sdll, which is clamped on the negative side to -dqmdllcode
		// and clamped on the positive side to (mdllcode - DELIMIT_POS_ADJ_WRDQSDLL)
		
		// Also push CA WRLVL SDLL here
		// CA WRLVLMax value should be with the max DQS SDLL of the two bytes
		// 1. DQS SDLL will be the negation of min_WrDqCenterPoint_val_b. Negate using macro NEGATE
		// 2. If negation turns out to be -ve, make it zero, since DQS SDLL can only be >=0.
		max_of_min_WrDqCenterPoint_val_b = MAX(POS(NEGATE(min_WrDqCenterPoint_val_b[0])), POS(NEGATE(min_WrDqCenterPoint_val_b[1])));
		amp_program_cawrlvl_sdll(ch, cawrlvl_sdll_wrlvl[ch], max_of_min_WrDqCenterPoint_val_b); 
		shim_printf("DEBUG - wrdqcal_program_final_values:: Programming a value of %d into CA WRLVLMAX \n", max_of_min_WrDqCenterPoint_val_b);
		shim_printf("DEBUG - wrdqcal_program_final_values:: Programming a value of %d into CA WRLVL \n", cawrlvl_sdll_wrlvl[ch]); 
	
		// Program SDLL for each byte lane
		for (byte = 0; byte < DQ_NUM_BYTES; byte++) {
			// Set the DQ/DQS SDLL (RESULT) for this channel/byte
			wrdqcal_set_delay_point_res(ch, 1 << byte, min_WrDqCenterPoint_val_b[byte]);
			shim_printf("DEBUG - %s:: Programming (for byte %d) final DQ/DQS point to: %5d\n", __PRETTY_FUNCTION__,
					min_WrDqCenterPoint_val_b[byte], byte);
			
			// DQS WRLVL SDLL should be incremented by difference between MAX DQS SDLL & DQS SDLL of the bytes in question.		
			shim_printf("DEBUG - wrdqcal_program_final_values:: Programming a value of %d into DQ byte %d WRLVLMAX \n",max_of_min_WrDqCenterPoint_val_b - POS(NEGATE(min_WrDqCenterPoint_val_b[byte])), byte);
			amp_program_dqwrlvl_sdll(ch, byte, dqwrlvl_sdll_after_wrlvl[ch][byte], max_of_min_WrDqCenterPoint_val_b - POS(NEGATE(min_WrDqCenterPoint_val_b[byte])));	

			for (bit_indx = 0; bit_indx < DQ_NUM_BITS_PER_BYTE; bit_indx++) {
				arr_WrDqBitDeSkew[(byte * DQ_NUM_BITS_PER_BYTE) + bit_indx] = arr_WrDqBitCenterPoint[(byte * DQ_NUM_BITS_PER_BYTE) + bit_indx] - min_WrDqCenterPoint_val_b[byte];
	
				CSR_WRITE(rAMP_WRDQDESKEW_CTRL(byte, ch, bit_indx), arr_WrDqBitDeSkew[(byte * DQ_NUM_BITS_PER_BYTE) + bit_indx] | RUNDESKEWUPD);
				shim_printf("DEBUG - wrdqcal_program_final_values:: Programming a value of %d into DQ byte %d bit %d \n", arr_WrDqBitDeSkew[(byte * DQ_NUM_BITS_PER_BYTE) + bit_indx], byte, bit_indx);

				// Poll for deskew update to be done.
				SPIN_W_TMO_WHILE (CSR_READ(rAMP_WRDQDESKEW_CTRL(byte, ch, bit_indx)) & RUNDESKEWUPD);
			}
			
			// Program DMI deskew 
			arr_WrDmiDeSkew[byte] = arr_WrDqBitCenterPoint[DQ_TOTAL_BITS + byte] - min_WrDqCenterPoint_val_b[byte];  
			CSR_WRITE(rAMP_WRDMDESKEW_CTRL(byte, ch), arr_WrDmiDeSkew[byte] | RUNDESKEWUPD);
			shim_printf("DEBUG - wrdqcal_program_final_values:: Programming a value of %d into DMI deskew for byte %d \n", arr_WrDmiDeSkew[byte], byte);
			
			// Poll for deskew update to be done.
			SPIN_W_TMO_WHILE (CSR_READ(rAMP_WRDMDESKEW_CTRL(byte, ch)) & RUNDESKEWUPD);
		}

		// Program DRAM with optimal WRDQ Vref
  		amp_program_wrdq_vref(ch, 0, cfg_params.wrdq_vref_values[vref_ch], 0); //vref_pulse_cnt=0

		// Also program MCU registers with optimal WRDQ Vref value.
		if(cfg_params.freq_bin == 1) {
			wrdqcalvrefcodecontrol = CSR_READ(rAMP_WRDQCALVREFCODESTATUS(ch));
			CSR_WRITE(rAMP_WRDQCALVREFCODECONTROL(ch), (wrdqcalvrefcodecontrol & WRDQ_VREF_F1_MASK) | (cfg_params.wrdq_vref_values[vref_ch] << WRDQ_VREF_F1_OFFSET));
		}
		else if(cfg_params.freq_bin == 0) {
			wrdqcalvrefcodecontrol = CSR_READ(rAMP_WRDQCALVREFCODESTATUS(ch));
			CSR_WRITE(rAMP_WRDQCALVREFCODECONTROL(ch), (wrdqcalvrefcodecontrol & WRDQ_VREF_F0_MASK) | (cfg_params.wrdq_vref_values[vref_ch] << WRDQ_VREF_F0_OFFSET));
		}


	} // for (ch = 0; ch < cfg_params.num_channels; ch++)
}

_ROMCODE_SEGMENT_PREFIX static int32_t find_center_of_eye(int32_t left_pos_val, int32_t right_pos_val)
{
	// CP : Maui Change
   if (left_pos_val > right_pos_val)
      shim_panic("Memory calibration: find_center_of_eye: Left value (0x%x) is > right value (0x%x) \n", left_pos_val, right_pos_val);
   
   // center of 2 signed integers is simply their average
   return ((left_pos_val + right_pos_val) / 2);
}

_ROMCODE_SEGMENT_PREFIX static int32_t find_wrdq_center_of_eye(int32_t left_pos_val, int32_t right_pos_val)
{
	// CP : Maui Change
   if (left_pos_val < right_pos_val)
      shim_panic("Memory calibration: find_wrdq_center_of_eye: Left value (0x%x) is < right value (0x%x) \n", left_pos_val, right_pos_val);
   
   // center of 2 signed integers is simply their average
   return ((left_pos_val + right_pos_val) / 2);
}

// Select the value that would include the other value in the eye
_ROMCODE_SEGMENT_PREFIX static int32_t find_common_endpoint(int32_t val0, int32_t val1, uint32_t left_or_right)
{
	int32_t retVal = val0;
	
	// For the left endpoint, select the rightmost value on the number line (max value)
	if (left_or_right == MAX_ENDPT) {
		if (val0 > val1)
			retVal = val0;
		else
			retVal = val1;
	}
	// For the right endpoint, select the leftmost value (min value)
	else {
		if (val0 < val1)
			retVal = val0;
		else
			retVal = val1;
	}
	
	return retVal;
}

// Inserts the data at given byte and bit position in the cal_bits array
// Assumes that num_bits is always <= 8
_ROMCODE_SEGMENT_PREFIX void cal_save_value(uint8_t data, uint32_t num_bits, uint32_t *bit_pos, uint32_t *byte_pos)
{
	uint32_t space_in_this_byte;
	uint8_t mask;
	
	if (((*bit_pos) > 7) || ((*byte_pos) >= CALIB_PMU_BYTES))
		shim_panic("Error! bit position %d > 7 or byte position %d > capacity (%d)\n", *bit_pos, *byte_pos, CALIB_PMU_BYTES);
	
	// how many bits left in this byte?
	space_in_this_byte = 8 - (*bit_pos);
	
	// we'll grab as many bits from the data as there is space in this byte
	if (space_in_this_byte >= num_bits)
		mask = (1 << num_bits) - 1;
	else
		mask = (1 << space_in_this_byte) - 1;
	
	// Set the data value at given byte (only as many bits as space and making sure to preserve the other bits in this byte)
	cal_bits[*byte_pos] |= ((data & mask) << *bit_pos);
	
	if (space_in_this_byte < num_bits) {
		// any remainder bits get saved to the next byte
		cal_bits[(*byte_pos) + 1] = (data >> space_in_this_byte);
		(*byte_pos)++;
		*bit_pos = num_bits - space_in_this_byte;
	} else if (space_in_this_byte == num_bits) {
		(*byte_pos)++;
		*bit_pos = 0;
	} else {
		(*bit_pos) += num_bits;
	}
}

// Retrieve the data at given byte and bit position in the cal_bits array
// Assumes that num_bits is always <= 8
_ROMCODE_SEGMENT_PREFIX static uint8_t cal_retrieve_value(uint32_t num_bits, uint32_t *bit_pos, uint32_t *byte_pos)
{
	uint32_t space_in_this_byte;
	uint8_t mask, remainder_mask, ret_val;
	
	if (((*bit_pos) > 7) || ((*byte_pos) >= CALIB_PMU_BYTES))
		shim_panic("Error! bit position %d > 7 or byte position %d > capacity (%d)\n", *bit_pos, *byte_pos, CALIB_PMU_BYTES);
	
	// how many bits left in this byte?
	space_in_this_byte = 8 - (*bit_pos);
	
	// we'll grab as many bits from the array as there is space in this byte (max of num_bits)
	if (space_in_this_byte >= num_bits)
		mask = (1 << num_bits) - 1;
	else {
		mask = (1 << space_in_this_byte) - 1;
		remainder_mask = (1 << (num_bits - space_in_this_byte)) - 1;
	}
	
	// Get the data value at given byte (only as many bits as space)
	ret_val = (cal_bits[*byte_pos] >> *bit_pos) & mask;
	
	if (space_in_this_byte < num_bits) {
		// any remainder bits get loaded from the next byte
		ret_val |= (cal_bits[(*byte_pos) + 1] & remainder_mask) << space_in_this_byte;
		(*byte_pos)++;
		*bit_pos = num_bits - space_in_this_byte;
	} else if (space_in_this_byte == num_bits) {
		(*byte_pos)++;
		*bit_pos = 0;
	} else {
		(*bit_pos) += num_bits;
	}
	
	return ret_val;
}

// ================================================================================
typedef struct
{
  uint32_t sum;			// Number of "Mass" objects recorded
  int32_t  sum_mass_ave;	// Sum of the Average "Mass" position
  uint32_t sum_mass;		// Sum of the "Mass"
  uint32_t sum_mass_h;		// Sum of the "Mass" * "height"
  int32_t  center;		// Center-of-Mass
  uint32_t center_h;		// Center-of-Mass WRT height
} opt_center_accum_t;

// ================================================================================
_ROMCODE_SEGMENT_PREFIX void amp_opt_center_init(opt_center_accum_t *opt)
{
 uint32_t *ptr = (uint32_t *)opt;
 uint32_t *end = (uint32_t *)(opt + 1);

 // ZERO everything
 while (ptr < end)  *ptr++ = 0;
}

// ================================================================================
_ROMCODE_SEGMENT_PREFIX void amp_opt_center_add(opt_center_accum_t *opt, uint32_t height, int shift, int32_t hi, int32_t lo)
{
  if (hi < lo)
		shim_panic("Error! Non-sensical (hi < lo) in calculating center [hi==%d, lo==%d]\n", hi, lo);

  // Apply the Shift (could be ZERO)
  hi += shift;
  lo += shift;

  opt->sum          += 1;
  opt->sum_mass_ave += (hi + lo);		// NOTE: "divide by two" handled at finalization
  opt->sum_mass     += (hi - lo);
  opt->sum_mass_h   += (hi - lo) * height;
}

// ================================================================================
_ROMCODE_SEGMENT_PREFIX void amp_opt_center_final(opt_center_accum_t *opt)
{
  if (opt->sum > 0)
    // Find the Average Center (Rounding off to nearest)
    opt->center   = (opt->sum_mass_ave + ((int32_t)opt->sum)) / (2 * ((int32_t)opt->sum));

  if (opt->sum_mass > 0)
    // Find the Weighted-Average Height Center (Rounding off to nearest)
    opt->center_h = (opt->sum_mass_h + (opt->sum_mass / 2))  / (opt->sum_mass);
}

// ================================================================================
// Compute the optimal Vref index across multiple vref and multiple bits
// Used by CA, RdDQ, WrDQ
_ROMCODE_SEGMENT_PREFIX void amp_compute_opt_vref(char *descr, uint32_t vrefMax, uint32_t bitMax,
						  int32_t *right, int32_t *left, int32_t *vref_panic, int32_t *skewR, int32_t *skewL,
						  int32_t *opt_cent_solution, uint32_t *opt_vref_solution)
{
	uint32_t vref,bit;
	int32_t L,R;
	int32_t panic;
	int32_t deskew = 0;
	int32_t maxL,minR;
	uint32_t span,max_span = 0;
	uint32_t opt_vref = 0;
	int32_t opt_center = 0;
	opt_center_accum_t optimal;
	bool use_com = (cfg_params.opt_vref_mode == DCS_CAL_OPT_VREF_MODE_COM);
	
	if (use_com) {
		// Init the "optimal center" calculation
		amp_opt_center_init(&optimal);
	}

	for(vref = 0; vref < vrefMax; vref++) {

		panic = *vref_panic++;
	
		shim_printf("DEBUG - OptVref (%s) [vref=%d] Panic flag is %d \n", descr, vref, panic);

		// Only consider this Vref if it did not panic.
		if(!panic) { 

			maxL = -MAX_SDLL_VAL;
			minR =  MAX_SDLL_VAL;
			for(bit = 0; bit < bitMax; bit++) {
				// Grab the Values
				L = *left++;
				R = *right++;
				if (skewL && skewR)
					deskew = *skewL++ - *skewR++;

				shim_printf("DEBUG - OptVref (%s) [vref=%d][bit=%d]: L=%d, R=%d, deskew=%d\n",
						descr, vref, bit, L, R, deskew);

				if (use_com) {
					// Add this info to the "optimal center" calculation
					amp_opt_center_add(&optimal, vref, deskew, R, L);
				} else {
					// Find the minimum Right and find the maximum Left
					if (minR > R)  minR = R;
					if (maxL < L)  maxL = L;
				}
			}

			if (!use_com) {
				// Calculate the Span for this vref
				span = minR - maxL;
				// Remember the Maximum span (and record its vref and its center point)
				if (max_span < span) {
					max_span = span;
					opt_vref = vref;
					opt_center = (maxL + minR) / 2;
				}
			}
		}
	}

	if (use_com) {
		// Finalize the "optimal center" calculation
		amp_opt_center_final(&optimal);
		opt_vref   = optimal.center_h;
		opt_center = optimal.center;
	}

	shim_printf("DEBUG - OptVref (%s) OptimalVref=%d, OptimalCenter=%d\n", descr, opt_vref, opt_center);
	
	// Copy our results to caller
	if (opt_vref_solution)  *opt_vref_solution = opt_vref;
	if (opt_cent_solution)  *opt_cent_solution = opt_center;
}

// ================================================================================
// Compute the optimal Vref index for CA calibration
_ROMCODE_SEGMENT_PREFIX void amp_compute_opt_vref_cacal(uint32_t ch, uint32_t rnk, uint32_t swloop, uint32_t bitMax)
{
	// Calc the index for this Loop/Ch/Rnk
	uint32_t loopchrnk_indx = loopchrnk_index_calc(ch, rnk, swloop);

	// Run the Calculation for the Optimal VRef and Optimal Center
	amp_compute_opt_vref("CA", cfg_params.ca_num_vref, bitMax,
			     (int32_t *)(cacal_per_loopchrnk_right[loopchrnk_indx]),
			     (int32_t *)(cacal_per_loopchrnk_left[loopchrnk_indx]),
			     (int32_t *)(cacal_vref_panic[loopchrnk_indx]),
			     0, 0,
			     &(cntr_opt_ca[loopchrnk_indx]),
			     &(vref_opt_ca[loopchrnk_indx]));
}
		
// ================================================================================
// Compute the optimal Vref index for RdDQ calibration
_ROMCODE_SEGMENT_PREFIX void amp_compute_opt_vref_rddqcal(uint32_t ch, uint32_t rnk, uint32_t bitMax)
{
	// Calc the index for this Loop/Ch/Rnk
	uint32_t loopchrnk_indx = loopchrnk_index_calc(ch, rnk, 0);

	// Run the Calculation for the Optimal VRef and Optimal Center
	amp_compute_opt_vref("RdDQ", cfg_params.rddq_num_vref, bitMax,
			     (int32_t *)(rddqcal_per_chrnk_right[loopchrnk_indx]),
			     (int32_t *)(rddqcal_per_chrnk_left[loopchrnk_indx]),
			     (int32_t *)(rddqcal_vref_panic[loopchrnk_indx]),
			     0, 0,
			     &(cntr_opt_rddq[loopchrnk_indx]),
			     &(vref_opt_rddq[loopchrnk_indx]));
}
		
// ================================================================================
// Compute the optimal Vref index for WrDQ calibration
_ROMCODE_SEGMENT_PREFIX void amp_compute_opt_vref_wrdqcal(uint32_t ch, uint32_t rnk, uint32_t bitMax)
{
	// Calc the index for this Loop/Ch/Rnk
	uint32_t loopchrnk_indx = loopchrnk_index_calc(ch, rnk, 0);

	// Run the Calculation for the Optimal VRef and Optimal Center
	amp_compute_opt_vref("WrDQ", cfg_params.wrdq_num_vref, bitMax,
			     // NOTE: for WRDQ, Left and Right have been Swapped!
			     (int32_t *)(wrdqcal_per_chrnk_left[loopchrnk_indx]),
			     (int32_t *)(wrdqcal_per_chrnk_right[loopchrnk_indx]),
			     (int32_t *)(wrdqcal_vref_panic[loopchrnk_indx]),
			     // =======================================================
			     0, 0,
			     &(cntr_opt_wrdq[loopchrnk_indx]),
			     &(vref_opt_wrdq[loopchrnk_indx]));
}
// ================================================================================
