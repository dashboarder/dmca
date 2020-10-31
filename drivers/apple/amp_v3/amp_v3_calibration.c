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

#ifdef ENV_IBOOT
#include "iboot/amp_v3_shim.h"
#elif defined ENV_DV
#include "dv/amp_v3_shim.h"
#elif defined ENV_CFE
#include "include/amp_v3_shim.h"
#else
#error "Unidentified compilation environment for shared memory calibration code"
#endif

// PRBS7 patterns used for ca calibration. cfg_params.cacalib_sw_loops * cfg_params.cacalib_hw_loops must equal 64
// We extend the size by 8 and repeat the first 8 values because when swloop=64, we don't go outside the array for programming CACALPAT1-7
static const uint32_t CA_PRBS7_PATTERNS[CA_NUM_PATTERNS + 8] = {
	0x2550B, 0xCF135, 0xC4342, 0x67BFF, 0x825A0, 0x1487E, 0x984EF, 0xEA43E,
	0x0B277, 0xA388D, 0xE5E5F, 0x96DDe, 0x8CC91, 0x720D1, 0xE1649, 0xA8ACA,
	0x466E2, 0x73381, 0x1A14F, 0xFEC40, 0x93698, 0x49C83, 0xEEC28, 0x35563,
	0x692CE, 0xE4D0F, 0x6DAD8, 0xDAA1B, 0xA70AB, 0xDB94B, 0x5C7AD, 0x8DFC1,
	0x897D7, 0xB70C3, 0x7DAB0, 0x7C9E0, 0x87EE6, 0xD186C, 0x04816, 0x3E714,
	0xCAA73, 0x01350, 0xFB706, 0x5668A, 0xD507A, 0x3AF02, 0xF4D67, 0xCB923,
	0xFA456, 0xAD18C, 0x836F0, 0xEEF78, 0xCE265, 0x3F444, 0x31D75, 0x575DA,
	0x2E77C, 0x6C988, 0x21D1D, 0xF1621, 0x0E931, 0x668AF, 0x792A6, 0x42EF4,
	0x2550B, 0xCF135, 0xC4342, 0x67BFF, 0x825A0, 0x1487E, 0x984EF, 0xEA43E,
};

// PRBS patterns for Wrdq and Rddq (for the one after Wrdq) calibration
static const uint32_t DQ_PRBS7_PATTERNS[DQ_NUM_PATTERNS] = {
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

/* Global configuration parameters for calibration, setup is different for iBoot, DV, SiVal, PE */
static struct amp_calibration_params cfg_params;

int32_t mdllcode[AMC_NUM_CHANNELS][3];		// 1 for each AMP_DQ, and 1 for AMP_CA
static uint32_t cacal_patterns_mask[CA_NUM_PATTERNS];

// This array will hold the calibration values to be saved for resume boot.
// Size is CALIB_PMU_BYTES, as this is the max space available in PMU for calibration, in iBoot environment
static uint8_t cal_bits[CALIB_PMU_BYTES] = { 0 };

/*
 * Following global arrays will record passing points for calibration operations
 */

// Used to save calibration values for each bit per channel and rank for every iteration
int32_t cacal_per_loopchrnk_right[AMC_NUM_CHANNELS * AMC_NUM_RANKS * CACAL_MAX_SWLOOPS][CA_NUM_BITS];
int32_t cacal_per_loopchrnk_left[AMC_NUM_CHANNELS * AMC_NUM_RANKS * CACAL_MAX_SWLOOPS][CA_NUM_BITS];

// ca data aggregated over all iterations
int32_t cacal_per_chrnk_right[AMC_NUM_CHANNELS * AMC_NUM_RANKS][CA_NUM_BITS];
int32_t cacal_per_chrnk_left[AMC_NUM_CHANNELS * AMC_NUM_RANKS][CA_NUM_BITS];
int32_t cacal_saved_val[AMC_NUM_CHANNELS][CA_NUM_BITS + 4];

// wrlvl data aggregated over all iterations, we save value for 4 byte lanes + the ca value
uint32_t wrlvlcal_per_chrnk_rise[AMC_NUM_CHANNELS * AMC_NUM_RANKS][DQ_NUM_BYTES + 1];
uint32_t wrlvlcal_per_chrnk_fall[AMC_NUM_CHANNELS * AMC_NUM_RANKS][DQ_NUM_BYTES + 1];
uint32_t wrlvlcal_saved_val[AMC_NUM_CHANNELS][DQ_NUM_BYTES + 1];

// rddq data aggregated over all iterations
int32_t rddqcal_per_chrnk_right[AMC_NUM_CHANNELS * AMC_NUM_RANKS][DQ_TOTAL_BITS];
int32_t rddqcal_per_chrnk_left[AMC_NUM_CHANNELS * AMC_NUM_RANKS][DQ_TOTAL_BITS];

// wrdq data aggregated over all iterations
int32_t wrdqcal_per_chrnk_right[AMC_NUM_CHANNELS * AMC_NUM_RANKS][DQ_TOTAL_BITS];
int32_t wrdqcal_per_chrnk_left[AMC_NUM_CHANNELS * AMC_NUM_RANKS][DQ_TOTAL_BITS];

// <rdar://problem/13878186>
void calibrate_ca(void);
void calibrate_rddq(bool after_wrddqcal);
void calibrate_wrlvl(void);
void calibrate_wrdq(void);
// Force initialization of cfg_params (used by DV)
void calibration_init_cfg_params(bool resume);
// save off mdll values, used by DV for individual calibration simulations
void amp_save_masterdll_values(void);

// Static local function declarations
static void amp_program_ca_patterns(uint32_t ch, uint32_t rnk, uint32_t swloop);
static void amp_init_ca_offset_and_deskew(uint32_t ch);
static uint32_t amp_mask_ca_bits(uint32_t ch, uint32_t mr_cmd);
static void amp_push_casdll_out(uint32_t ch, int32_t offset);
static void amp_enable_cacal_mode(bool enable, uint32_t ch);
static uint32_t amp_run_cacal(uint32_t ch);
static void amp_push_ctl_out(uint32_t ch, uint32_t dly_val);
static void amp_setup_rddq_cal(uint32_t ch, uint32_t rnk);
static void amp_set_rddq_sdll(uint32_t ch, uint32_t byte, uint32_t offset);
static void amp_run_rddqcal(uint32_t ch);
static void amp_wrlvl_init(void);
static void amp_set_cawrlvl_sdll(uint32_t ch, uint32_t offset, bool set_dly_sel);
static void amp_set_dqwrlvl_sdll(uint32_t ch, uint32_t byte, uint32_t offset, bool set_dly_sel);
static void amp_run_wrlvlcal(uint32_t ch, uint32_t wrlvlrun);
static void amp_set_wrdq_sdll(uint32_t ch, uint32_t byte, int32_t offset);
static void cacal_generate_patterns_mask(void);
static void cacal_run_sequence(uint32_t ch, uint32_t rnk, uint32_t mr_cmd, uint32_t mask_bits, uint32_t swloop);
static void cacal_find_right_failing_point(uint32_t ch, uint32_t rnk, uint32_t mr_cmd, uint32_t combined_mask, uint32_t swloop);
static void cacal_find_right_passing_point(uint32_t ch, uint32_t rnk, uint32_t mr_cmd, uint32_t combined_mask, uint32_t swloop);
static void cacal_enter_cacal_mode(uint32_t ch, uint32_t rnk, uint32_t mr_cmd, bool enter);
static void cacal_find_left_failing_point(uint32_t ch, uint32_t rnk, uint32_t combined_mask, uint32_t swloop);
static void cacal_find_left_passing_point(uint32_t ch, uint32_t rnk, uint32_t combined_mask, uint32_t swloop);
static void cacal_program_final_values(void);
static void rddqcal_find_right_failing_point(uint32_t ch, uint32_t rnk, bool after_wrddqcal);
static void rddqcal_find_right_passing_point(uint32_t ch, uint32_t rnk, int32_t *start_b, bool after_wrddqcal);
static void rddqcal_find_left_failing_point(uint32_t ch, uint32_t rnk, bool after_wrddqcal);
static void rddqcal_find_left_passing_point(uint32_t ch, uint32_t rnk, int32_t *start_b, bool after_wrddqcal);
static void rddqcal_program_final_values(void);
static uint32_t dqcal_handle_swizzle(uint32_t result, bool after_wrddqcal);
static uint32_t wrlvlcal_encode_dlyval(uint32_t ch, uint32_t phy_type, uint32_t val);
static uint32_t wrlvlcal_encode_clk90dly(uint32_t ch, uint32_t phy_type, uint32_t val);
static void wrlvlcal_push_to_0s_region(uint32_t ch, uint32_t rnk);
static void wrlvlcal_find_0to1_transition(uint32_t ch, uint32_t rnk);
static void wrlvlcal_find_1to0_transition(uint32_t ch, uint32_t rnk);
static void wrlvlcal_program_final_values(void);
static void wrdqcal_find_right_failing_point(uint32_t ch, uint32_t rnk);
static void wrdqcal_find_right_passing_point(uint32_t ch, uint32_t rnk, int32_t *start_b);
static void wrdqcal_find_left_failing_point(uint32_t ch, uint32_t rnk);
static void wrdqcal_find_left_passing_point(uint32_t ch, uint32_t rnk, int32_t *start_b);
static void wrdqcal_program_final_values(void);
static uint32_t dqcal_wr_rd_pattern_result(uint32_t ch, uint32_t rnk, uint32_t sdll_value);
// Helper functions
static uint32_t map_byte_to_dq(uint32_t *byte);
static uint8_t reverse_bits_in_byte(uint8_t result);
static int32_t find_center_of_eye(int32_t left_pos_val, int32_t right_pos_val);
static int32_t find_common_endpoint(int32_t val0, int32_t val1, uint32_t min_or_max);
static int32_t offset_convert(int32_t offset, uint32_t direction);

///////////////////////////////////////////////////////////////////////////////
////// Global functions
///////////////////////////////////////////////////////////////////////////////

// Perform CA, RDDQ, and WRLVL calibration
void calibration_ca_rddq_wrlvl(bool resume)
{
	// Initialize calibration parameters to their default values
	calibration_init_cfg_params(resume);
	
	if ((cfg_params.cacalib_hw_loops * cfg_params.cacalib_sw_loops) != CA_NUM_PATTERNS)
		shim_panic("Memory calibration: hwloops (%d) and swloops (%d) values are unexpected\n",
				   cfg_params.cacalib_hw_loops, cfg_params.cacalib_sw_loops);
	
	// Configure AMC registers and perform other pre-ca calibration steps
	shim_configure_pre_ca();

	if (cfg_params.dbg_calib_mode & SELECT_CACAL) {
		if (!resume)
			calibrate_ca();
	}
	
	/*
	 * The first RdDq Cal is using MRR32 and MRR40. It's needed for WrDq calibration. Hence run before WrDq.
	 * The second Rd Dq calibration is PRBS pattern based, which needs Wr Dq calibration done. Hence done after WrDq.
	 * PRBS patterns help in reducing aliasing, hence needed for better accuracy.
	 */
	if (cfg_params.dbg_calib_mode & SELECT_RDDQCAL) {
		calibrate_rddq(false);
	}
	
	if (cfg_params.dbg_calib_mode & SELECT_WRLVLCAL) {
		if (!resume)
			calibrate_wrlvl();
	}
	
	shim_configure_post_wrlvl(&cfg_params);
}

void calibration_wrdq_rddq(bool resume)
{	
	shim_configure_pre_wrdq(resume);
	
	if (cfg_params.dbg_calib_mode & SELECT_WRDQCAL) {
		calibrate_wrdq();
	}
	
	/*
	 * The first RdDq Cal is using MRR32 and MRR40. It's needed for WrDq calibration. Hence run before WrDq.
	 * The second Rd Dq calibration is PRBS pattern based, which needs Wr Dq calibration done. Hence done after WrDq.
	 * PRBS patterns help in reducing aliasing, hence needed for better accuracy.
	 */
	if (cfg_params.dbg_calib_mode & SELECT_RDDQCAL2) {
		calibrate_rddq(true);
	}

	shim_configure_post_prbs_rddq(&cfg_params);
}

// Save or restore ca, wrlvl, rddq, and wrdq registers for resume boot
void calibration_save_restore_regs(uint32_t save_or_restore, uint32_t channels)
{
	uint32_t ch, bit_indx, byte, dq, dq_byte;
	uint32_t byte_pos = 0;
	
	if (save_or_restore == CALIB_SAVE) {
		
		for (ch = 0; ch < channels; ch++) {
			
			// save the CA per bit offset for this channel into global array (offset = sdll + deskew)
			uint8_t casdll = (uint8_t) (CSR_READ(rAMP_CASDLLCTRL(ch)) & DLLVAL_BITS);
			
			for (bit_indx = 0; bit_indx < CA_NUM_BITS; bit_indx++) {
				uint8_t ca_deskew = (uint8_t) (CSR_READ(rAMP_CADESKEW_CTRL(ch, bit_indx)) & DESKEW_CTRL_BITS);
				// need to convert deskew into sdll scale, since offsets are saved in sdll scale
				ca_deskew = offset_convert(ca_deskew, DIRECTION_DESKEW_TO_SDLL);
				uint8_t ca_offset = INT_TO_OFFSET(OFFSET_TO_INT(casdll) + OFFSET_TO_INT(ca_deskew));
				cal_bits[byte_pos++] = ca_offset;
			}
			
			// CK, CS, and CKE share the same value
			uint8_t ck_deskew = (uint8_t) (CSR_READ(rAMP_CKDESKEW_CTRL(ch, 0)) & DESKEW_CTRL_BITS);
			// need to convert deskew into sdll scale, since offsets are saved in sdll scale
			cal_bits[byte_pos++] = offset_convert(ck_deskew, DIRECTION_DESKEW_TO_SDLL);
			
			// save the WrLvl registers for this channel (4 DQ SDLLs and 1 CA SDLL)
			for (byte = 0; byte < (DQ_NUM_BYTES + 1); byte++) {
				
				dq_byte = byte;
				dq = map_byte_to_dq(&dq_byte);
				
				// cawrlvlsdll is stored as the "5th" byte
				if (byte == DQ_NUM_BYTES)
					cal_bits[byte_pos++] = (uint8_t) (CSR_READ(rAMP_CAWRLVLSDLLCODE(ch)) & DLLVAL_BITS);
				else
					cal_bits[byte_pos++] = (uint8_t) (CSR_READ(rAMP_DQWRLVLSDLLCODE(dq, ch, dq_byte)) & DLLVAL_BITS);
			}
			
			// save the RDDQ registers for this channel (32 per bit offset = sdll - deskew)
			for (byte = 0; byte < DQ_NUM_BYTES; byte++) {
				
				dq_byte = byte;
				dq = map_byte_to_dq(&dq_byte);

				uint8_t rddqsdll = (uint8_t) (CSR_READ(rAMP_DQSDLLCTRL_RD(dq, ch, dq_byte)) & DLLVAL_BITS);
				
				for (bit_indx = 0; bit_indx < DQ_NUM_BITS_PER_BYTE; bit_indx++) {
					uint8_t rddq_deskew = (uint8_t) (CSR_READ(rAMP_RDDQDESKEW_CTRL(dq, ch, dq_byte, bit_indx)) & DESKEW_CTRL_BITS);
					// need to convert deskew into sdll scale, since offsets are saved in sdll scale
					rddq_deskew = offset_convert(rddq_deskew, DIRECTION_DESKEW_TO_SDLL);
					uint8_t rddq_offset = INT_TO_OFFSET(OFFSET_TO_INT(rddqsdll) - OFFSET_TO_INT(rddq_deskew));
					cal_bits[byte_pos++] = rddq_offset;
				}
			}
			
			// save the WRDQ registers for this channel (32 per bit offset = sdll + deskew)
			for (byte = 0; byte < DQ_NUM_BYTES; byte++) {
				
				dq_byte = byte;
				dq = map_byte_to_dq(&dq_byte);
				
				uint8_t wrdqsdll = (uint8_t) (CSR_READ(rAMP_DQSDLLCTRL_WR(dq, ch, dq_byte)) & DLLVAL_BITS);
				
				// if sdll is negative, dqs deskew adds to sdll. For positive sdll, dqs should be 0
				if (wrdqsdll & (1 << SIGN_BIT_POS)) {
					// need to convert deskew into sdll scale, since offsets are saved in sdll scale
					uint8_t dqs_deskew = (uint8_t) (CSR_READ(rAMP_WRDMDESKEW_CTRL(dq, ch, dq_byte)) & DESKEW_CTRL_BITS);
					dqs_deskew = offset_convert(dqs_deskew, DIRECTION_DESKEW_TO_SDLL);
					wrdqsdll += dqs_deskew;
				}
				
				for (bit_indx = 0; bit_indx < DQ_NUM_BITS_PER_BYTE; bit_indx++) {
					uint8_t wrdq_deskew = (uint8_t) (CSR_READ(rAMP_WRDQDESKEW_CTRL(dq, ch, dq_byte, bit_indx)) & DESKEW_CTRL_BITS);
					// need to convert deskew into sdll scale, since offsets are saved in sdll scale
					wrdq_deskew = offset_convert(wrdq_deskew, DIRECTION_DESKEW_TO_SDLL);
					uint8_t wrdq_offset = INT_TO_OFFSET(OFFSET_TO_INT(wrdqsdll) + OFFSET_TO_INT(wrdq_deskew));
					cal_bits[byte_pos++] = wrdq_offset;
				}
			}
		}
		
		// Save the cal_bits array
		shim_store_memory_calibration((void *) cal_bits, CALIB_NUM_BYTES_TO_SAVE);
	} else {
		// Retrieve cal_bits array
		shim_load_memory_calibration((void *) cal_bits, CALIB_NUM_BYTES_TO_SAVE);
		
		// Initialize calibration parameters to their default values
		calibration_init_cfg_params(true);
		
		amp_save_masterdll_values();
		
		for (ch = 0; ch < channels; ch++) {
			
			/*
			 * CA
			 */
			
			// CA bit offsets are first 10 values
			int32_t casdll = OFFSET_TO_INT(cal_bits[byte_pos]);
			int32_t ca_offset[CA_NUM_BITS];
			
			// Find the min, assign to sdll.
			for (bit_indx = 0; bit_indx < CA_NUM_BITS; bit_indx++) {
				ca_offset[bit_indx] = OFFSET_TO_INT(cal_bits[byte_pos]);
				byte_pos++;
				if (ca_offset[bit_indx] < casdll)
					casdll = ca_offset[bit_indx];
			}
			
			// get the value pushed on CK, CS, CKE signals, add it to casdll
			casdll -= OFFSET_TO_INT(cal_bits[byte_pos]);
			byte_pos++;
			uint8_t caclk = 0;
			
			// check if sdll < -camdllcode, then clamp to it. If sdll > camdllcode - DELIMIT_POS_ADJ_CASDLL, then also clamp it.
			if (casdll < (-1 * mdllcode[ch][AMP_CA])) {
				caclk = (uint8_t) ((-1 * casdll) - mdllcode[ch][AMP_CA]);
				casdll = (-1 * mdllcode[ch][AMP_CA]);
			} else if (casdll > (mdllcode[ch][AMP_CA] - DELIMIT_POS_ADJ_CASDLL)) {
				casdll = mdllcode[ch][AMP_CA] - DELIMIT_POS_ADJ_CASDLL;
			}
			
			// write the casdll register, and caclk into ctl signals
			amp_push_casdll_out(ch, casdll);
			if (caclk)
				amp_push_ctl_out(ch, offset_convert(caclk, DIRECTION_SDLL_TO_DESKEW));

			// compute deskew and write to the per bit deskew registers
			for (bit_indx = 0; bit_indx < CA_NUM_BITS; bit_indx++) {
				uint8_t ca_deskew = (uint8_t) (ca_offset[bit_indx] - (casdll - caclk));
				// need to convert to deskew scale
				ca_deskew = offset_convert(ca_deskew, DIRECTION_SDLL_TO_DESKEW);
				CSR_WRITE(rAMP_CADESKEW_CTRL(ch, bit_indx), ca_deskew);
			}
			
			/*
			 * WrLvl
			 */
			
			for (byte = 0; byte < (DQ_NUM_BYTES + 1); byte++) {
				uint8_t wrlvlsdll = cal_bits[byte_pos++];
				
				// At this point, DRAM is not in WRLVL mode so can use dlysel with forceckelow
				if (byte == DQ_NUM_BYTES)
					amp_set_cawrlvl_sdll(ch, wrlvlsdll, true);
				else
					amp_set_dqwrlvl_sdll(ch, byte, wrlvlsdll, true);
			}
			
			/*
			 * Rddq
			 */
			
			for (byte = 0; byte < DQ_NUM_BYTES; byte++) {
				
				dq_byte = byte;
				dq = map_byte_to_dq(&dq_byte);
				
				// find the max offset and assign it to sdll
				int32_t rddqsdll = OFFSET_TO_INT(cal_bits[byte_pos]);
				int32_t rddq_offset[DQ_NUM_BITS_PER_BYTE];
				for (bit_indx = 0; bit_indx < DQ_NUM_BITS_PER_BYTE; bit_indx++) {
					rddq_offset[bit_indx] = OFFSET_TO_INT(cal_bits[byte_pos]);
					byte_pos++;
					if (rddq_offset[bit_indx] > rddqsdll)
						rddqsdll = rddq_offset[bit_indx];
				}
				
				// clamp sdll to -mdllcode if it is < -mdllcode
				if (rddqsdll < (-1 * mdllcode[ch][dq]))
					rddqsdll = (-1 * mdllcode[ch][dq]);
				
				// write the rddqsdll register
				amp_set_rddq_sdll(ch, byte, INT_TO_OFFSET(rddqsdll));
				
				// compute deskew and write to the per bit deskew registers
				for (bit_indx = 0; bit_indx < DQ_NUM_BITS_PER_BYTE; bit_indx++) {
					uint8_t rddq_deskew = (uint8_t) (rddqsdll - rddq_offset[bit_indx]);
					// need to convert to deskew scale
					rddq_deskew = offset_convert(rddq_deskew, DIRECTION_SDLL_TO_DESKEW);
					CSR_WRITE(rAMP_RDDQDESKEW_CTRL(dq, ch, dq_byte, bit_indx), rddq_deskew);
				}
			}
			
			/*
			 * Wrdq
			 */
			
			for (byte = 0; byte < DQ_NUM_BYTES; byte++) {
				
				dq_byte = byte;
				dq = map_byte_to_dq(&dq_byte);
				
				// find the min offset and assign it to sdll
				int32_t wrdqsdll = OFFSET_TO_INT(cal_bits[byte_pos]);
				int32_t wrdq_offset[DQ_NUM_BITS_PER_BYTE];
				for (bit_indx = 0; bit_indx < DQ_NUM_BITS_PER_BYTE; bit_indx++) {
					wrdq_offset[bit_indx] = OFFSET_TO_INT(cal_bits[byte_pos]);
					byte_pos++;
					if (wrdq_offset[bit_indx] < wrdqsdll)
						wrdqsdll = wrdq_offset[bit_indx];
				}
				
				uint8_t wrdqs_deskew = 0;
				
				// clamp sdll to -mdllcode if it is < -mdllcode
				if (wrdqsdll < (-1 * mdllcode[ch][dq])) {
					wrdqs_deskew = (uint8_t) ((-1 * wrdqsdll) - mdllcode[ch][dq]);
					wrdqsdll = (-1 * mdllcode[ch][dq]);
				} else if (wrdqsdll > (mdllcode[ch][dq] - DELIMIT_POS_ADJ_WRDQSDLL)) {
					// for positive sdll, clamp to mdllcode - DELIMIT_POS_ADJ_WRDQSDLL
					wrdqsdll = mdllcode[ch][dq] - DELIMIT_POS_ADJ_WRDQSDLL;
				}
				
				// set the wrdqsdll register
				amp_set_wrdq_sdll(ch, byte, wrdqsdll);
				if (wrdqs_deskew)
					CSR_WRITE(rAMP_WRDMDESKEW_CTRL(dq, ch, dq_byte), offset_convert(wrdqs_deskew, DIRECTION_SDLL_TO_DESKEW));
				
				// compute deskew and write to the per bit deskew registers (keeping track of min and max deskews for data mask)
				uint8_t min_wrdq_deskew = (uint8_t) (wrdq_offset[0] - (wrdqsdll - wrdqs_deskew));
				uint8_t max_wrdq_deskew = (uint8_t) (wrdq_offset[0] - (wrdqsdll - wrdqs_deskew));
				// need to convert to deskew scale
				min_wrdq_deskew = offset_convert(min_wrdq_deskew, DIRECTION_SDLL_TO_DESKEW);
				max_wrdq_deskew = offset_convert(max_wrdq_deskew, DIRECTION_SDLL_TO_DESKEW);
														
				for (bit_indx = 0; bit_indx < DQ_NUM_BITS_PER_BYTE; bit_indx++) {
					uint8_t wrdq_deskew = (uint8_t) (wrdq_offset[bit_indx] - (wrdqsdll - wrdqs_deskew));
					// need to convert to deskew scale
					wrdq_deskew = offset_convert(wrdq_deskew, DIRECTION_SDLL_TO_DESKEW);
					CSR_WRITE(rAMP_WRDQDESKEW_CTRL(dq, ch, dq_byte, bit_indx), wrdq_deskew);
					
					if (wrdq_deskew < min_wrdq_deskew)
						min_wrdq_deskew = wrdq_deskew;
					if (wrdq_deskew > max_wrdq_deskew)
						max_wrdq_deskew = wrdq_deskew;
				}
														
				// put average of min and max into data mask
				CSR_WRITE(rAMP_WRDQSDESKEW_CTRL(dq, ch, dq_byte), (min_wrdq_deskew + max_wrdq_deskew) >> 1);
			}
		}
	}
}

void calibration_dump_results(uint32_t operations, bool dump_passing_points)
{
	uint32_t ch, rnk, chrnk_indx, dq, dq_byte, bit, byte;
	
	shim_printf("Memory calibration results with passing points per rank in brackets:\n");
	
	for (ch = 0; ch < cfg_params.num_channels; ch++) {
		shim_printf("Channel %d\n", ch);
		
		if (operations & SELECT_CACAL) {
		
			shim_printf("\tCA SDLL: %d\n", OFFSET_TO_INT(CSR_READ(rAMP_CASDLLCTRL(ch)) & DLLVAL_BITS));
		
			shim_printf("\t\tPer Bit Deskew: ");
			for (bit = 0; bit < CA_NUM_BITS; bit++) {
				shim_printf("%d ", CSR_READ(rAMP_CADESKEW_CTRL(ch, bit)) & DESKEW_CTRL_BITS);
			
				if (dump_passing_points) {
				
					shim_printf("[");
				
					for (rnk = 0; rnk < cfg_params.num_ranks; rnk++) {
					
						if (rnk > 0)
							shim_printf(", ");
					
						chrnk_indx = (ch * cfg_params.num_ranks) + rnk;
					
						shim_printf("%d ", cacal_per_chrnk_right[chrnk_indx][bit]);
						shim_printf("%d", cacal_per_chrnk_left[chrnk_indx][bit]);
					}
				
					shim_printf("] ");
				}
			
			}
			shim_printf("\n\t\tCS, CK, CKE Deskew: %d", CSR_READ(rAMP_CKDESKEW_CTRL(ch, 0)) & DESKEW_CTRL_BITS);
		
			shim_printf("\n");
		}
		
		if (operations & SELECT_WRLVLCAL) {
		
			shim_printf("\tCA WrLvlSDLL: %d", CSR_READ(rAMP_CAWRLVLSDLLCODE(ch)) & DLLVAL_BITS);
		
			if (dump_passing_points) {
				shim_printf(" [");
			
				for (rnk = 0; rnk < cfg_params.num_ranks; rnk++) {
				
					if (rnk > 0)
						shim_printf(", ");
				
					chrnk_indx = (ch * cfg_params.num_ranks) + rnk;
				
					shim_printf("%d ", wrlvlcal_per_chrnk_rise[chrnk_indx][DQ_NUM_BYTES]);
					shim_printf("%d", wrlvlcal_per_chrnk_fall[chrnk_indx][DQ_NUM_BYTES]);
				}
			
				shim_printf("]");
			}
		
			shim_printf("\n");
		
			shim_printf("\tDQ WrLvlSDLL: ");
			for (byte = 0; byte < DQ_NUM_BYTES; byte++) {
			
				dq_byte = byte;
				dq = map_byte_to_dq(&dq_byte);
			
				shim_printf("%d ", CSR_READ(rAMP_DQWRLVLSDLLCODE(dq, ch, dq_byte)) & DLLVAL_BITS);
			
				if (dump_passing_points) {
					shim_printf("[");
				
					for (rnk = 0; rnk < cfg_params.num_ranks; rnk++) {
					
						if (rnk > 0)
							shim_printf(", ");
					
						chrnk_indx = (ch * cfg_params.num_ranks) + rnk;
					
						shim_printf("%d ", wrlvlcal_per_chrnk_rise[chrnk_indx][byte]);
						shim_printf("%d", wrlvlcal_per_chrnk_fall[chrnk_indx][byte]);
					}
				
					shim_printf("] ");
				}
			}
		
			shim_printf("\n");
		}
		
		if ((operations & SELECT_RDDQCAL) || (operations & SELECT_RDDQCAL2)) {
		
			shim_printf("\tRead DQ:\n");
			for (byte = 0; byte < DQ_NUM_BYTES; byte++) {
			
				dq_byte = byte;
				dq = map_byte_to_dq(&dq_byte);
			
				shim_printf("\t\tByte %d SDLL: %d\n", byte, OFFSET_TO_INT(CSR_READ(rAMP_DQSDLLCTRL_RD(dq, ch, dq_byte)) & DLLVAL_BITS));
			
				shim_printf("\t\t\tPer Bit Deskew: ");
				for (bit = 0; bit < DQ_NUM_BITS_PER_BYTE; bit++) {
					shim_printf("%d ", CSR_READ(rAMP_RDDQDESKEW_CTRL(dq, ch, dq_byte, bit)) & DESKEW_CTRL_BITS);
				
					if (dump_passing_points) {
						shim_printf("[");
						for (rnk = 0; rnk < cfg_params.num_ranks; rnk++) {
					
							if (rnk > 0)
								shim_printf(", ");
					
							chrnk_indx = (ch * cfg_params.num_ranks) + rnk;
					
							shim_printf("%d ", rddqcal_per_chrnk_right[chrnk_indx][bit + DQ_NUM_BITS_PER_BYTE * byte]);
							shim_printf("%d", rddqcal_per_chrnk_left[chrnk_indx][bit + DQ_NUM_BITS_PER_BYTE * byte]);
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
			
				dq_byte = byte;
				dq = map_byte_to_dq(&dq_byte);
			
				shim_printf("\t\tByte %d SDLL: %d\n", byte, OFFSET_TO_INT(CSR_READ(rAMP_DQSDLLCTRL_WR(dq, ch, dq_byte)) & DLLVAL_BITS));
			
				shim_printf("\t\t\tPer Bit Deskew: ");
				for (bit = 0; bit < DQ_NUM_BITS_PER_BYTE; bit++) {
					shim_printf("%d ", CSR_READ(rAMP_WRDQDESKEW_CTRL(dq, ch, dq_byte, bit)) & DESKEW_CTRL_BITS);
				
					if (dump_passing_points) {
						shim_printf(" [");
						for (rnk = 0; rnk < cfg_params.num_ranks; rnk++) {
					
							if (rnk > 0)
								shim_printf(", ");
					
							chrnk_indx = (ch * cfg_params.num_ranks) + rnk;
					
							shim_printf("%d ", wrdqcal_per_chrnk_right[chrnk_indx][bit + DQ_NUM_BITS_PER_BYTE * byte]);
							shim_printf("%d", wrdqcal_per_chrnk_left[chrnk_indx][bit + DQ_NUM_BITS_PER_BYTE * byte]);
						}
						shim_printf("] ");
					}
				}
			
				shim_printf("\n");
			}
		}
	}
}

// <rdar://problem/13878186>
// Force initialization of cfg_params
void calibration_init_cfg_params(bool resume)
{
	// Initialize calibration parameters to their default values
	cfg_params.num_channels = 2;
	cfg_params.num_ranks = 1;
	cfg_params.resume = resume;
	cfg_params.cacalib_hw_loops = 8;
	cfg_params.cacalib_sw_loops = 8;
	// On Fiji B0 and Capri A0, deskew step is 22% bigger than sdll step
	cfg_params.sdll_scale = 122;
	cfg_params.deskew_scale = 100;
	cfg_params.dv_params_valid = false;
	cfg_params.dv_randomize = false;
	cfg_params.dbg_calib_mode = SELECT_CAL_ALL;
	
	// Allow shim layer to change parameters as needed
	shim_init_calibration_params(&cfg_params);
}

#ifdef ENV_IBOOT
// To dump calibration results from iBoot menu command
static int dump_mem_calibration_info(int argc, struct cmd_arg *args)
{
	calibration_init_cfg_params(false);
	calibration_dump_results(SELECT_CAL_ALL, false);
	return 0;
}
MENU_COMMAND_DEBUG(memcal_info, dump_mem_calibration_info, "Prints memory calibration results", NULL);
#endif

void calibrate_ca(void)
{
	uint32_t ch, rnk, swloop, mask_bits;
	
	cacal_generate_patterns_mask();
	
	// Required since the dll values may change slightly during calibration
	amp_save_masterdll_values();
	
	// Calibration sequence is to be run for each rank in each channel, cfg_params.cacalib_sw_loops number of times
	for (ch = 0; ch < cfg_params.num_channels; ch++) {
		for (rnk = 0; rnk < cfg_params.num_ranks; rnk++) {
			for (swloop = 0; swloop < cfg_params.cacalib_sw_loops; swloop++) {
				amp_program_ca_patterns(ch, rnk, swloop);
				
				amp_init_ca_offset_and_deskew(ch);
				
				// Training of CA Bits 0-3 and 5-8: MR41 cmd (training cmd must be sent before cacalibmode is enabled in AMP)
				mask_bits = amp_mask_ca_bits(ch, MR41);
				shim_mrcmd_to_ch_rnk(MR_WRITE, ch, rnk, MR41, MR41 << 2);
				amp_enable_cacal_mode(true, ch);
				cacal_run_sequence(ch, rnk, MR41, mask_bits, swloop);
				amp_enable_cacal_mode(false, ch);

				amp_init_ca_offset_and_deskew(ch);
				
				// Training of CA Bits 4 and 9: MR48 cmd (training cmd must be sent before cacalibmode is enabled in AMP)
				mask_bits = amp_mask_ca_bits(ch, MR48);
				shim_mrcmd_to_ch_rnk(MR_WRITE, ch, rnk, MR48, MR48 << 2);
				amp_enable_cacal_mode(true, ch);
				cacal_run_sequence(ch, rnk, MR48, mask_bits, swloop);
				amp_enable_cacal_mode(false, ch);
				
				amp_init_ca_offset_and_deskew(ch);
				
				// Exit CA Training mode: MR42
				shim_mrcmd_to_ch_rnk(MR_WRITE, ch, rnk, MR42, MR42 << 2);
			}
		}
	}
	
	// By now, we have compiled right and left edges of passing window for all CA bits over a number of iterations
	// Aggregate the results, and find the center point of the window, and program it
	cacal_program_final_values();
}

void calibrate_rddq(bool after_wrddqcal)
{
	uint32_t ch, rnk, data;
	
	// step7
	if (after_wrddqcal == false) {
		
		for (ch = 0; ch < cfg_params.num_channels; ch++) {
			for (rnk = 0; rnk < cfg_params.num_ranks; rnk++) {
				amp_setup_rddq_cal(ch, rnk);
				
				shim_mrcmd_to_ch_rnk(MR_READ, ch, rnk, MR5, (uintptr_t)&data);
				
				shim_enable_rddqcal(true);
				
				// Find the left and right edges of the eye
				rddqcal_find_right_failing_point(ch, rnk, false);
				rddqcal_find_left_failing_point(ch, rnk, false);
								
				shim_enable_rddqcal(false);
			}
		}
	}
	// step10
	else {
		for (ch = 0; ch < cfg_params.num_channels; ch++) {
			for (rnk = 0; rnk < cfg_params.num_ranks; rnk++) {
				// Find the left and right edges of the eye using PRBS patterns
				// These results will be more accurate
				rddqcal_find_right_failing_point(ch, rnk, true);
				rddqcal_find_left_failing_point(ch, rnk, true);
			}
		}
	}
	
	// Now that we have per bit left and right endpoints for each channel and rank, aggregate and program final values
	rddqcal_program_final_values();
}

// Align the clock signal with the DQ signals
void calibrate_wrlvl(void)
{
	uint32_t ch, rnk;
	uint32_t data, cawrlvlsdll;
	
	amp_wrlvl_init();
	
	for (ch = 0; ch < cfg_params.num_channels; ch++) {
		
		cawrlvlsdll = CSR_READ(rAMP_CAWRLVLSDLLCODE(ch));
		
		for (rnk = 0; rnk < cfg_params.num_ranks; rnk++) {
			data = 0x80 + RD_LATENCY_ENCODE; // 0x80 is added here to set the Write Level bit (bit 7) to 1
			
			shim_mrcmd_to_ch_rnk(MR_WRITE, ch, rnk, MR2, data);

			// find the region where all bits return a 0
			wrlvlcal_push_to_0s_region(ch, rnk);
			
			// push out the clock signal until all bits return a 1
			wrlvlcal_find_0to1_transition(ch, rnk);
			
			// now go back towards the transition edge found earlier, but from this side of the edge
			wrlvlcal_find_1to0_transition(ch, rnk);
			
			// reset cawrlvlsdllcode to original value (0), before sending cmd to exit wrlvl mode (MR2)
			amp_set_cawrlvl_sdll(ch, cawrlvlsdll, false);
			
			data = RD_LATENCY_ENCODE;
			shim_mrcmd_to_ch_rnk(MR_WRITE, ch, rnk, MR2, data);
		}
	}
	
	// Program the final wrlvl values
	wrlvlcal_program_final_values();
}

void calibrate_wrdq(void)
{
	uint32_t ch, rnk;
	
	for (ch = 0; ch < cfg_params.num_channels; ch++) {
		for (rnk = 0; rnk < cfg_params.num_ranks; rnk++) {
			wrdqcal_find_right_failing_point(ch, rnk);
			wrdqcal_find_left_failing_point(ch, rnk);
		}
	}
	
	wrdqcal_program_final_values();

}

void amp_save_masterdll_values(void)
{
	uint32_t ch;
	
	for (ch = 0; ch < cfg_params.num_channels; ch++) {
		mdllcode[ch][AMP_DQ0] = (CSR_READ(rAMP_MDLLCODE(AMP_DQ0, ch)) & DLLVAL_BITS);
		mdllcode[ch][AMP_DQ1] = (CSR_READ(rAMP_MDLLCODE(AMP_DQ1, ch)) & DLLVAL_BITS);
		mdllcode[ch][AMP_CA] = (CSR_READ(rAMP_MDLLCODE(AMP_CA, ch)) & DLLVAL_BITS);
	}
}

///////////////////////////////////////////////////////////////////////////////
////// Local functions
///////////////////////////////////////////////////////////////////////////////

static void amp_program_ca_patterns(uint32_t ch, uint32_t rnk, uint32_t swloop)
{
	uint32_t cacalctrl, p;
	
	// Map the CA bits to appropriate dq byte lanes, accounting for swizzling.
	// Bytes 0 and 2 from DDR are hooked to AMPDQ0, and are bit swizzled (i.e., bit 0 from DDR connected to bit 7 on AMPDQ, and so on).
	// Bytes 1 and 3 are hooked to AMPDQ1, and are NOT bit swizzled.
	// For the 10 CA bits, that means CA0-3 (MR41) and CA4 (MR48) come back from DDR on AMPDQ0,
	// and CA5-8 (MR41) and CA9 (MR48) on AMPDQ1.
	
	CSR_WRITE(rAMP_CACALCAMAP0(AMP_DQ0, ch), 0x01234567);
	CSR_WRITE(rAMP_CACALCAMAP1(AMP_DQ0, ch), 0xdcba9867);
	CSR_WRITE(rAMP_CACALCABYTESEL(AMP_DQ0, ch), 0);
	
	CSR_WRITE(rAMP_CACALCAMAP1(AMP_DQ1, ch), 0x54321010);
	CSR_WRITE(rAMP_CACALCAMAP2(AMP_DQ1, ch), 0x00001076);
	CSR_WRITE(rAMP_CACALCABYTESEL(AMP_DQ1, ch), 0);
	
	// Program rank, hardware loop count, and timing params
	// Timing params are taken from lpddr3 jedec spec
	cacalctrl = (rnk << 24) | ((cfg_params.cacalib_hw_loops - 1) << 16) | (16 << 8) | (10 << 0);
	
	CSR_WRITE(rAMP_CACALCTRL(AMP_DQ0, ch), cacalctrl);
	CSR_WRITE(rAMP_CACALCTRL(AMP_DQ1, ch), cacalctrl);
	CSR_WRITE(rAMP_CACALCTRL(AMP_CA, ch), cacalctrl);
	
	for (p = 0; p < AMP_MAX_PATTERNS; p++) {
		CSR_WRITE(rAMP_CACALPAT(AMP_DQ0, ch, p), CA_PRBS7_PATTERNS[(swloop * cfg_params.cacalib_hw_loops) + p]);
		CSR_WRITE(rAMP_CACALPAT(AMP_DQ1, ch, p), CA_PRBS7_PATTERNS[(swloop * cfg_params.cacalib_hw_loops) + p]);
		CSR_WRITE(rAMP_CACALPAT(AMP_CA, ch, p), CA_PRBS7_PATTERNS[(swloop * cfg_params.cacalib_hw_loops) + p]);
	}
}

// (Re-)Initialize ca offset and deskew registers
static void amp_init_ca_offset_and_deskew(uint32_t ch)
{
	uint32_t d;
	int32_t camdllcode = mdllcode[ch][AMP_CA];
	
	// ensure negative sign is set with mdllcode value for ca offset (mdllcode guaranteed by designers not to be negative)
	amp_push_casdll_out(ch, (-1 * camdllcode));
	
	// Clear cadeskewctrl registers
	for (d = 0; d < CA_NUM_BITS; d++)
		CSR_WRITE(rAMP_CADESKEW_CTRL(ch, d), 0);
}

static uint32_t amp_mask_ca_bits(uint32_t ch, uint32_t mr_cmd)
{
	uint32_t mask_bits, dq0_mask, dq1_mask;
	
	if (mr_cmd == MR41) {
		// MR41:  Mask out bits 9 and 4
		mask_bits = 0x210;
		
		// set all bits except CA0-3 in mask register for MR41 on DQ0
		dq0_mask = 0x3F0;
		
		// set all bits except CA5-8 in mask register for MR41 on DQ1
		dq1_mask = 0x21F;		
	} else if (mr_cmd == MR48) {
		// MR48:  Mask out bits 0-3 and bits 5-8
		mask_bits = 0x1EF;
		
		// set all bits except CA4 in mask register for MR48 on DQ0
		dq0_mask = 0x3EF;
		
		// set all bits except CA9 in mask register for MR48 on DQ1
		dq1_mask = 0x1FF;				
	} else {
		// No bits are masked out
		mask_bits = 0;
		dq0_mask = 0;
		dq1_mask = 0;
	}
	
	CSR_WRITE(rAMP_CACALMASK(AMP_DQ0, ch), dq0_mask);
	CSR_WRITE(rAMP_CACALMASK(AMP_DQ1, ch), dq1_mask);
	
	return mask_bits;
}

static void amp_push_casdll_out(uint32_t ch, int32_t offset)
{
	uint32_t ca_bit;
	uint32_t cadeskewcode;
	int32_t camdllcode = mdllcode[ch][AMP_CA];
	
	if (offset > 0) {
		// if offset is within DELIMIT_POS_ADJ_CASDLL steps of camdllcode, limit it to (master dll - DELIMIT_POS_ADJ_CASDLL)
		if (offset >= (camdllcode - DELIMIT_POS_ADJ_CASDLL)) {
			uint8_t difference = (uint8_t) (offset - (camdllcode - DELIMIT_POS_ADJ_CASDLL));
			offset = camdllcode - DELIMIT_POS_ADJ_CASDLL;
			// convert sdll taps into deskew taps
			difference = offset_convert(difference, DIRECTION_SDLL_TO_DESKEW);
			
			if (difference >= CA_MAX_DESKEW_OFFSET)
				cadeskewcode = CA_MAX_DESKEW_OFFSET;
			else
				cadeskewcode = difference;

			// Adjust deskew registers for each ca bit
			for (ca_bit = 0; ca_bit < CA_NUM_BITS; ca_bit++)
				CSR_WRITE(rAMP_CADESKEW_CTRL(ch, ca_bit), cadeskewcode);
		}
	}
	
	CSR_WRITE(rAMP_CASDLLCTRL(ch), (1 << 24) | INT_TO_OFFSET(offset));
	while (CSR_READ(rAMP_CASDLLCTRL(ch)) & (1 << 24));
}

static void amp_enable_cacal_mode(bool enable, uint32_t ch)
{
	uint32_t cacalrun = CSR_READ(rAMP_CACALRUN(AMP_CA, ch));
	
	// Set or clear CACalMode bit
	if (enable)
		CSR_WRITE(rAMP_CACALRUN(AMP_CA, ch), cacalrun | CACALRUN_CACALMODE);
	else
		CSR_WRITE(rAMP_CACALRUN(AMP_CA, ch), cacalrun & ~CACALRUN_CACALMODE);
}

static uint32_t amp_run_cacal(uint32_t ch)
{
	uint32_t cacalrun_dq = CSR_READ(rAMP_CACALRUN(AMP_DQ0, ch));
	uint32_t cacalrun_ca = CSR_READ(rAMP_CACALRUN(AMP_CA, ch));
	
	// DQ must be set before CA
	CSR_WRITE(rAMP_CACALRUN(AMP_DQ0, ch), cacalrun_dq | CACALRUN_RUNCACAL);
	CSR_WRITE(rAMP_CACALRUN(AMP_DQ1, ch), cacalrun_dq | CACALRUN_RUNCACAL);
	// CACalMode should already be set
	CSR_WRITE(rAMP_CACALRUN(AMP_CA, ch), cacalrun_ca | CACALRUN_RUNCACAL);
	// Poll on the DQ register
	while(CSR_READ(rAMP_CACALRUN(AMP_DQ0, ch)) & CACALRUN_RUNCACAL);
	while(CSR_READ(rAMP_CACALRUN(AMP_DQ1, ch)) & CACALRUN_RUNCACAL);
	
	return ((CSR_READ(rAMP_CACALRESULT(AMP_DQ0, ch)) & CA_ALL_BITS) &
			(CSR_READ(rAMP_CACALRESULT(AMP_DQ1, ch)) & CA_ALL_BITS));
}

static void amp_push_ctl_out(uint32_t ch, uint32_t dly_val)
{	
	uint32_t cadramsigdly;
	
	CSR_WRITE(rAMP_TESTMODE(AMP_CA, ch), TESTMODE_FORCECKELOW);
	
	// Fix for Radar 10790574 - Hold Violation on CKE
	if (dly_val >= 0xd)
		cadramsigdly = (3 << 4);
	else if (dly_val >= 0xa)
		cadramsigdly = (2 << 4);
	else if (dly_val >= 0x8)
		cadramsigdly = (1 << 4);
	else
		cadramsigdly = (0 << 4);

	CSR_WRITE(rAMP_DRAMSIGDLY(AMP_CA, ch, 0), cadramsigdly);
	CSR_WRITE(rAMP_CSDESKEW_CTRL(ch), dly_val);
	CSR_WRITE(rAMP_CKDESKEW_CTRL(ch, 0), dly_val);
	CSR_WRITE(rAMP_CKDESKEW_CTRL(ch, 1), dly_val);
	CSR_WRITE(rAMP_CKEDESKEW_CTRL(ch), dly_val);
		
	CSR_WRITE(rAMP_TESTMODE(AMP_CA, ch), 0);
}

static void amp_setup_rddq_cal(uint32_t ch, uint32_t rnk)
{
	// At this point the AMC's READLEVELING should already be setup as 0x00000300
	// Make DQCALCTRL.DQCalPatSel (bits 1:0) match READLEVELING.RdLvlPatOpt
	CSR_WRITE(rAMP_DQCALCTRL(AMP_DQ0, ch), (rnk << 16) | (RDDQ_LOOPCNT << 8) | (3 << 0));
	CSR_WRITE(rAMP_DQCALCTRL(AMP_DQ1, ch), (rnk << 16) | (RDDQ_LOOPCNT << 8) | (3 << 0));
}

// This functions set the slave dll for a particular byte lane of RDDQ as specified in the offset parameter
static void amp_set_rddq_sdll(uint32_t ch, uint32_t byte, uint32_t offset)
{
	uint32_t dq = map_byte_to_dq(&byte);
	
	CSR_WRITE(rAMP_DQSDLLCTRL_RD(dq, ch, byte), (1 << 24) | offset);

	// Wait for Run bit to clear
	while(CSR_READ(rAMP_DQSDLLCTRL_RD(dq, ch, byte)) & (1 << 24));
}

static void amp_run_rddqcal(uint32_t ch)
{
	CSR_WRITE(rAMP_DQCALRUN(AMP_DQ0, ch), 1);
	CSR_WRITE(rAMP_DQCALRUN(AMP_DQ1, ch), 1);
	while (CSR_READ(rAMP_DQCALRUN(AMP_DQ0, ch)) & 1);
	while (CSR_READ(rAMP_DQCALRUN(AMP_DQ1, ch)) & 1);
}

static void amp_wrlvl_init(void)
{
	uint32_t ch;
	
	for (ch = 0; ch < cfg_params.num_channels; ch++) {
		// Write Leveling Timing Control Registers to program tMRD and tWLO timing params
		// Taking these values from jedec_lpddr3s8_4gb_x32_1600.soma
		// tWLO has a max value of 20ns for 1600 freq
		// tWLMRD has a min value of 40tck for 1600 freq
		
		CSR_WRITE(rAMP_DQWRLVLTIM(AMP_DQ0, ch), (12 << 8) | (16 << 0));
		CSR_WRITE(rAMP_DQWRLVLTIM(AMP_DQ1, ch), (12 << 8) | (16 << 0));
	}
}

// Must ensure that WrLvL SDLLs are stepped by 1 to their final value to avoid glitch on clock signals
static void amp_set_cawrlvl_sdll(uint32_t ch, uint32_t offset, bool set_dly_sel)
{
	uint32_t cawrlvlsdll = CSR_READ(rAMP_CAWRLVLSDLLCODE(ch));
	int32_t step = (cawrlvlsdll < offset) ? 1 : -1;
	
	if (set_dly_sel) {
		/* Must send phyupdt to AMC to avoid traffic while CKE is low */
		CSR_WRITE(rAMP_CAPHYUPDTREQ(ch), 1);
		while((CSR_READ(rAMP_CAPHYUPDTSTATUS(ch)) & 1) == 0);

		/* CKE must be low when updating dlysel to avoid glitches */
		CSR_WRITE(rAMP_TESTMODE(AMP_CA, ch), TESTMODE_FORCECKELOW);
	}
		
	for ( ; cawrlvlsdll != offset; cawrlvlsdll += step) {
		CSR_WRITE(rAMP_CAWRLVLSDLLCODE(ch), cawrlvlsdll + step);
	}
	
	if (set_dly_sel) {
		// final value programmed into the sdllcode will be offset
		CSR_WRITE(rAMP_CAWRLVLCLKDLYSEL(ch), wrlvlcal_encode_dlyval(ch, AMP_CA, offset));

		/* Release CKE and disable phyupdt to allow normal operation */
		CSR_WRITE(rAMP_TESTMODE(AMP_CA, ch), 0);

		CSR_WRITE(rAMP_CAPHYUPDTREQ(ch), 0);
		while((CSR_READ(rAMP_CAPHYUPDTSTATUS(ch)) & 1) == 1);
	}
}

// Must ensure that WrLvL SDLLs are stepped by 1 to their final value to avoid glitch on clock signals
static void amp_set_dqwrlvl_sdll(uint32_t ch, uint32_t byte, uint32_t offset, bool set_dly_sel)
{
	uint32_t dq_byte = byte;
	uint32_t dq = map_byte_to_dq(&dq_byte);
	uint32_t dqwrlvlsdll = CSR_READ(rAMP_DQWRLVLSDLLCODE(dq, ch, dq_byte));
	uint32_t dqwrlvldlychainctrl = CSR_READ(rAMP_DQWRLVLDLYCHAINCTRL(dq, ch, dq_byte));

	int32_t step = (dqwrlvlsdll < offset) ? 1 : -1;
	
	if (set_dly_sel) {
		/* Must send phyupdt to AMC to avoid traffic while CKE is low */
		CSR_WRITE(rAMP_CAPHYUPDTREQ(ch), 1);
		while((CSR_READ(rAMP_CAPHYUPDTSTATUS(ch)) & 1) == 0);
		
		/* CKE must be low when updating dlysel to avoid glitches */
		CSR_WRITE(rAMP_TESTMODE(AMP_CA, ch), TESTMODE_FORCECKELOW);
	}
		
	for ( ; dqwrlvlsdll != offset; dqwrlvlsdll += step)
		CSR_WRITE(rAMP_DQWRLVLSDLLCODE(dq, ch, dq_byte), dqwrlvlsdll + step);
	
	if (set_dly_sel) {
		// final value programmed into the sdllcode will be offset (also, preserve bits 17:16)
		CSR_WRITE(rAMP_DQWRLVLDLYCHAINCTRL(dq, ch, dq_byte), (dqwrlvldlychainctrl & 0x00030000) |
			wrlvlcal_encode_dlyval(ch, dq, offset));

		/* Release CKE and disable phyupdt to allow normal operation */
		CSR_WRITE(rAMP_TESTMODE(AMP_CA, ch), 0);

		CSR_WRITE(rAMP_CAPHYUPDTREQ(ch), 0);
		while((CSR_READ(rAMP_CAPHYUPDTSTATUS(ch)) & 1) == 1);
	}
}

static void amp_run_wrlvlcal(uint32_t ch, uint32_t wrlvlrun)
{
	CSR_WRITE(rAMP_DQWRLVLRUN(AMP_DQ0, ch), wrlvlrun & 0x3);
	CSR_WRITE(rAMP_DQWRLVLRUN(AMP_DQ1, ch), wrlvlrun >> 2);
	while(CSR_READ(rAMP_DQWRLVLRUN(AMP_DQ0, ch)));
	while(CSR_READ(rAMP_DQWRLVLRUN(AMP_DQ1, ch)));
}

// This functions set the slave dll for a particular byte lane of WRDQ as specified in the offset parameter
static void amp_set_wrdq_sdll(uint32_t ch, uint32_t byte, int32_t offset)
{
	uint32_t dq_bit;
	uint32_t dqdeskewcode;
	//<rdar://problem/14345921>
	uint32_t dq = map_byte_to_dq(&byte);
	int32_t dqmdllcode = mdllcode[ch][dq];

	if (offset > 0) {
		// New equation given by Rakesh: if offset is within DELIMIT_POS_ADJ_WRDQSDLL steps of dqmdllcode, limit it to (master dll - DELIMIT_POS_ADJ_WRDQSDLL)
		if (offset >= (dqmdllcode - DELIMIT_POS_ADJ_WRDQSDLL)) {
			uint8_t difference = (uint8_t) (offset - (dqmdllcode - DELIMIT_POS_ADJ_WRDQSDLL));
			offset = dqmdllcode - DELIMIT_POS_ADJ_WRDQSDLL;
			// convert sdll taps to deskew taps
			difference = offset_convert(difference, DIRECTION_SDLL_TO_DESKEW);

			if (difference >= DQ_MAX_DESKEW_PER_BIT)
				dqdeskewcode = DQ_MAX_DESKEW_PER_BIT;
			else
				dqdeskewcode = difference;

			// Adjust deskew registers for each dq bit
			for (dq_bit = 0; dq_bit < DQ_NUM_BITS_PER_BYTE; dq_bit++)
				CSR_WRITE(rAMP_WRDQDESKEW_CTRL(dq, ch, byte, dq_bit), dqdeskewcode);

			// Also update the Data Mask (DM), controlled by the DQSDESKEW register
			CSR_WRITE(rAMP_WRDQSDESKEW_CTRL(dq, ch, byte), dqdeskewcode);
		}

		// set wrlvlclk90dly (bits 17:16 of wrlvldlychainctrl reg) if positive sdll value
		CSR_WRITE(rAMP_DQWRLVLDLYCHAINCTRL(dq, ch, byte), (wrlvlcal_encode_clk90dly(ch, dq, offset) << 16) |
				  (CSR_READ(rAMP_DQWRLVLDLYCHAINCTRL(dq, ch, byte)) & 0x3));
	}
	
	CSR_WRITE(rAMP_DQSDLLCTRL_WR(dq, ch, byte), (1 << 24) | INT_TO_OFFSET(offset));
	
	// Wait for Run bit to clear
	while(CSR_READ(rAMP_DQSDLLCTRL_WR(dq, ch, byte)) & (1 << 24));
}

static void cacal_generate_patterns_mask(void)
{
	uint32_t index, patr, patf = 0;
	uint32_t mask = 0;
	
	// generate the pattern to be used for each CA calibration iteration
	for (index = 0; index < (cfg_params.cacalib_sw_loops * cfg_params.cacalib_hw_loops); index++) {
		patr = (CA_PRBS7_PATTERNS[index]) & CA_ALL_BITS;
		patf = (CA_PRBS7_PATTERNS[index]) >> CA_NUM_BITS;
		mask = patr ^ patf;
		cacal_patterns_mask[index] = mask;
	}
}

static void cacal_run_sequence(uint32_t ch, uint32_t rnk, uint32_t mr_cmd, uint32_t mask_bits, uint32_t swloop)
{
	uint32_t combined_mask, hwloop;
	uint32_t pat_mask = 0;
	
	for (hwloop = 0; hwloop < cfg_params.cacalib_hw_loops; hwloop++)
		pat_mask |= cacal_patterns_mask[(swloop * cfg_params.cacalib_hw_loops) + hwloop];
	
	// This represents the bits that don't have a transition on any of the patterns used during the hwloop calibration
	combined_mask = mask_bits | (CA_ALL_BITS - pat_mask);
	
	// To find the FAIL <-> PASS <-> FAIL window
	cacal_find_right_failing_point(ch, rnk, mr_cmd, combined_mask, swloop);
	cacal_find_left_failing_point(ch, rnk, combined_mask, swloop);
}

// Establish the right edge of the window by finding the point where all CA bits fail
static void cacal_find_right_failing_point(uint32_t ch, uint32_t rnk, uint32_t mr_cmd, uint32_t combined_mask, uint32_t swloop)
{
	bool all_bits_fail = false;
	uint32_t cacalresult = CA_ALL_BITS;
	uint32_t push_ck_out = 0;
	
	// Increase delay to the right until all bits fail
	do {
		cacalresult = cacalresult & amp_run_cacal(ch);
		
		if ((cacalresult & (CA_ALL_BITS ^ combined_mask)) != 0) {
			all_bits_fail = false;
			push_ck_out = push_ck_out + FINER_STEP_SZ;
			
			amp_init_ca_offset_and_deskew(ch);
			
			// Make AMP and DRAM exit CaCal Mode in order to update the CK, CKE, and CS delays
			cacal_enter_cacal_mode(ch, rnk, mr_cmd, false);
			
			// Update CK, CKE, and CS signal delays
			amp_push_ctl_out(ch, push_ck_out);
			
			// Re-enter CaCal mode
			cacal_enter_cacal_mode(ch, rnk, mr_cmd, true);
		} else {
			all_bits_fail = true;
			
			// Do a per bit calculation of when they start passing again
			cacal_find_right_passing_point(ch, rnk, mr_cmd, combined_mask, swloop);
		}
	} while ((push_ck_out <= CA_MAX_DESKEW_OFFSET) && (all_bits_fail == false));
	
	if (all_bits_fail == false) {
		shim_printf("Memory CA calibration: Unable to find right side failing point for channel %d\n", ch);

		// Failing point cannot be found, continuing to passing point assuming failure at this setting
		cacal_find_right_passing_point(ch, rnk, mr_cmd, combined_mask, swloop);
	}
	
	// Reset CK delay back to 0
	if (CSR_READ(rAMP_CKDESKEW_CTRL(ch, 0))) {
		// Exit CaCal Mode for AMP and DRAM before modifying CK, CKE, and CS signals
		cacal_enter_cacal_mode(ch, rnk, mr_cmd, false);
		
		// Ok from Rakesh to set to 0 directly instead of decrementing by 1
		amp_push_ctl_out(ch, 0);
		
		// Re-enable CACal Mode
		cacal_enter_cacal_mode(ch, rnk, mr_cmd, true);
	}
}

// Finds the passing region on the right edge of window
static void cacal_find_right_passing_point(uint32_t ch, uint32_t rnk, uint32_t mr_cmd, uint32_t combined_mask, uint32_t swloop)
{
	bool switch_from_cktoca;
	int32_t tap_value, max_ca_tap_val;
	uint32_t cacalresult;
	int32_t camdllcode, sdll_limit;
	int32_t saved_val, deskew_taps;
	uint32_t all_bits_pass;
	uint32_t BitPass[CA_NUM_BITS] = { 0 };
	uint32_t SolidBitPass[CA_NUM_BITS] = { 0 };
	uint32_t bit_indx;
	uint32_t ckdeskew;
	uint32_t loopchrnk_indx;
		
	all_bits_pass = 0;
	camdllcode = mdllcode[ch][AMP_CA];
	ckdeskew = CSR_READ(rAMP_CKDESKEW_CTRL(ch, 0));
	sdll_limit = camdllcode - DELIMIT_POS_ADJ_CASDLL;
	max_ca_tap_val = sdll_limit + offset_convert(CA_MAX_DESKEW_OFFSET, DIRECTION_DESKEW_TO_SDLL);
	
	// For every swloop, we'll save passing values for each channel & rank
	loopchrnk_indx = (swloop * cfg_params.num_channels * cfg_params.num_ranks) + (ch * cfg_params.num_ranks) + rnk;
	
	if (ckdeskew) {
		tap_value = ckdeskew;
		switch_from_cktoca = false;
	} else {
		// Since clock delay is already down to 0, use the slave delay.
		// We only have 2 knobs to turn for delay: clock and sdll
		tap_value = OFFSET_TO_INT(CSR_READ(rAMP_CASDLLCTRL(ch)) & DLLVAL_BITS);
		switch_from_cktoca = true;
	}

	// combined_mask contains don't care bits (due to pattern) or masked bits (MR41 or MR48), so consider those done
	for (bit_indx = 0; bit_indx < CA_NUM_BITS; bit_indx++)
		if ((combined_mask & (1 << bit_indx)) != 0)
			BitPass[bit_indx] = 1;
	
	// Finding Right side passing point on per bit level. Moving Right to Left to find point where it turns from FAIL TO PASS
	do {
		if (switch_from_cktoca == false) {
			// Make AMP and DRAM exit CaCal Mode in order to update the CK, CKE, and CS delays
			cacal_enter_cacal_mode(ch, rnk, mr_cmd, false);
			
			// Update CK, CKE, and CS signal delays
			amp_push_ctl_out(ch, tap_value);
			
			// Re-enter CaCal mode
			cacal_enter_cacal_mode(ch, rnk, mr_cmd, true);
		} else {
			amp_push_casdll_out(ch, tap_value);
		}
		
		// Run the ca calibration in hw
		cacalresult = amp_run_cacal(ch);
				
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
						if (switch_from_cktoca == false) {
							// MdllCode is considered '0' in this case
							// convert tap_value into sdll steps, since saved value is units of sdll
							saved_val = -1 * (offset_convert(tap_value, DIRECTION_DESKEW_TO_SDLL) + camdllcode);
						} else {
							// saved value is in units of sdll
							if (tap_value >= sdll_limit)
								saved_val = sdll_limit + offset_convert(deskew_taps, DIRECTION_DESKEW_TO_SDLL);
							else
								saved_val = tap_value;
						}
						
						cacal_per_loopchrnk_right[loopchrnk_indx][bit_indx] = saved_val;
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

		// If ALL bits are not passing - keep moving ca signals from Right to Left
		if (all_bits_pass == 0) {
			if ((tap_value == 0) && (switch_from_cktoca == false)) {
				switch_from_cktoca = true;
				
				tap_value = OFFSET_TO_INT(CSR_READ(rAMP_CASDLLCTRL(ch)) & DLLVAL_BITS);
			}
			
			if (switch_from_cktoca == false) {
				tap_value -= FINER_STEP_SZ;
			} else {
				tap_value += FINER_STEP_SZ;
				
				if (tap_value >= sdll_limit)
					deskew_taps = tap_value - sdll_limit;
			}
		}
		
	} while ((tap_value <= max_ca_tap_val) && (all_bits_pass == 0));
	
	if (all_bits_pass == 0) {
		shim_panic("Memory CA calibration: Unable to find passing point for all bits on the right side");
	}
}

static void cacal_enter_cacal_mode(uint32_t ch, uint32_t rnk, uint32_t mr_cmd, bool enter)
{
	// For entry, send MR41 command to DRAM before AMP register is changed
	if (enter) {
		// Re-enter CaCal Mode with MR41 always since some DRAMs don't support entering this mode with MR48
		shim_mrcmd_to_ch_rnk(MR_WRITE, ch, rnk, MR41, MR41 << 2);
		if (mr_cmd != MR41)
			shim_mrcmd_to_ch_rnk(MR_WRITE, ch, rnk, mr_cmd, mr_cmd << 2);
		
		amp_enable_cacal_mode(true, ch);
	}
	
	// For exit, change AMP register before sending DRAM command (MR42)
	else {
		amp_enable_cacal_mode(false, ch);
		shim_mrcmd_to_ch_rnk(MR_WRITE, ch, rnk, MR42, MR42 << 2);
	}
}

static void cacal_find_left_failing_point(uint32_t ch, uint32_t rnk, uint32_t combined_mask, uint32_t swloop)
{
	// At this point, we've already played with all possible CK delays. At the end of cacal_find_right_failing_point routine,
	// we reset the CK delays to 0.
	// Loop through CaSDLLOvrVal from -MasterDLL to +Max until all failing points on the left side are found

	uint32_t all_bits_fail;
	int32_t push_ca_out;
	uint32_t cacalresult;
	int32_t camdllcode, ca0deskewctrl;
	uint32_t max_caleft_point_exceeded;
	int32_t max_caleft_point_val;
	int32_t casdllctrl, sdll_limit;
		
	all_bits_fail = 0;
	cacalresult = CA_ALL_BITS;
	max_caleft_point_exceeded = 0;
	
	camdllcode = mdllcode[ch][AMP_CA];
	sdll_limit = camdllcode - DELIMIT_POS_ADJ_CASDLL;
	max_caleft_point_val = sdll_limit + offset_convert(CA_MAX_DESKEW_OFFSET, DIRECTION_DESKEW_TO_SDLL);
	
	casdllctrl = OFFSET_TO_INT(CSR_READ(rAMP_CASDLLCTRL(ch)) & DLLVAL_BITS);
	ca0deskewctrl = CSR_READ(rAMP_CADESKEW_CTRL(ch, 0));
	// convert deskew taps to sdll taps
	ca0deskewctrl = offset_convert(ca0deskewctrl, DIRECTION_DESKEW_TO_SDLL);

	// ca0deskewctrl will be non-zero only if casdll reached (camdllcode - DELIMIT_POS_ADJ_CASDLL)
	push_ca_out = casdllctrl + ca0deskewctrl;
	
	// Increment CaSDLLOvrVal from -ve Master Code to max_caleft_point_val
	do {
		// Push out this new ca offset
		amp_push_casdll_out(ch, push_ca_out);
		
		// run the calibration in hw
		cacalresult = cacalresult & amp_run_cacal(ch);
		
		// combined mask has don't care bits (based on pattern) and masked bits (based on MR41 or MR48) that we should ignore
		if ((cacalresult & (CA_ALL_BITS ^ combined_mask)) != 0) {
			all_bits_fail = 0;
		} else {
			all_bits_fail = 1;
			
			// Now, we have found the left edge of window. Find the passing point for all bits
			cacal_find_left_passing_point(ch, rnk, combined_mask, swloop);
		}
		
		if (all_bits_fail == 0) {
			if ((push_ca_out + COARSE_STEP_SZ) <= max_caleft_point_val)
				push_ca_out += COARSE_STEP_SZ;
			else
				push_ca_out += FINER_STEP_SZ;
		}
		
		if (push_ca_out > max_caleft_point_val) {
			max_caleft_point_exceeded = 1;
		}
		
		// Forcefully ending this loop as there are no more sdll taps left to proceed ahead
		if (max_caleft_point_exceeded && (all_bits_fail == 0))
		{
			shim_printf("Memory CA calibration: SDLL ran out of taps when trying to find left side failing point\n");

			cacal_find_left_passing_point(ch, rnk, combined_mask, swloop);
			all_bits_fail = 1;
		}
	} while ((all_bits_fail == 0) && (max_caleft_point_exceeded == 0));
}

static void cacal_find_left_passing_point(uint32_t ch, uint32_t rnk, uint32_t combined_mask, uint32_t swloop)
{
	uint32_t loopchrnk_indx;
	uint32_t BitPass[CA_NUM_BITS] = { 0 };
	uint32_t SolidBitPass[CA_NUM_BITS] = { 0 };
	int32_t tap_value;
	uint32_t cacalresult;
	int32_t camdllcode;
	int32_t ca0deskewctrl;
	uint32_t all_bits_pass;
	uint32_t bit_indx;
	int32_t sdll_limit, deskew_taps;
	
	loopchrnk_indx = (swloop * cfg_params.num_channels * cfg_params.num_ranks) + (ch * cfg_params.num_ranks) + rnk;
	
	tap_value = OFFSET_TO_INT(CSR_READ(rAMP_CASDLLCTRL(ch)) & DLLVAL_BITS);
	ca0deskewctrl = CSR_READ(rAMP_CADESKEW_CTRL(ch, 0));
	deskew_taps = ca0deskewctrl;
	// convert deskew taps to sdll taps
	ca0deskewctrl = offset_convert(ca0deskewctrl, DIRECTION_DESKEW_TO_SDLL);
	camdllcode = mdllcode[ch][AMP_CA];
	sdll_limit = camdllcode - DELIMIT_POS_ADJ_CASDLL;
	
	all_bits_pass = 0;
	
	// ca0deskewctrl will be non-zero only if casdll reached (camdllcode - DELIMIT_POS_ADJ_CASDLL)
	tap_value += ca0deskewctrl;
	
	// combined_mask contains don't care bits (due to pattern) or masked bits (MR41 or MR48), so consider those passed
	for (bit_indx = 0; bit_indx < CA_NUM_BITS; bit_indx++)
		if ((combined_mask & (1 << bit_indx)) != 0)
			BitPass[bit_indx] = 1;

	
	// Finding Left side passing point on per bit level. Move Left to Right to find point where it turns from FAIL TO PASS
	do {
		// Push out this new ca offset
		amp_push_casdll_out(ch, tap_value);
		
		// Run the calibration in hw
		cacalresult = amp_run_cacal(ch);

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
						
						int32_t saved_val;
						
						// saved value is in units of sdll steps
						if (tap_value >= sdll_limit)
							saved_val = sdll_limit + offset_convert(deskew_taps, DIRECTION_DESKEW_TO_SDLL);
						else
							saved_val = tap_value;
						
						cacal_per_loopchrnk_left[loopchrnk_indx][bit_indx] = saved_val;
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
		
		// If ALL bits are not passing - keep moving from Left to Right Side of window
		if (all_bits_pass == 0) {
			tap_value -= FINER_STEP_SZ;
			
			if (tap_value >= sdll_limit)
				deskew_taps = offset_convert(tap_value - sdll_limit, DIRECTION_SDLL_TO_DESKEW);
		}
		
		if ((tap_value < (-1 * camdllcode)) && (all_bits_pass == 0)) {
			// print error message as Left Passing Point cannot be found
			all_bits_pass = 1;
			shim_panic("Memory CA calibration: Unable to find passing point for all bits on the left side");
		}
	} while ((tap_value > (-1 * camdllcode)) && (all_bits_pass == 0));
}

static void cacal_program_final_values(void)
{
	uint32_t loopchrnk0_indx, loopchrnk1_indx, chrnk0_indx, chrnk1_indx, ch;
	uint32_t bit_indx;
	int32_t ca_bit_center[CA_NUM_BITS];
	int32_t ca_bit_deskew[CA_NUM_BITS];
	int32_t tmp_left_pos_val, tmp_right_pos_val;
	int32_t left_pos_val;
	int32_t right_pos_val;
	int32_t camdllcode;
	int32_t min_ca_bit_center;
	int32_t adj_ca_bit_center;
	uint32_t cs_adj_val;
	
	int32_t rank_val[AMP_MAX_RANKS_PER_CHAN];
	uint32_t swloop, hwloop;
	uint32_t mask;
	uint32_t comb_mask, tmp_mask;
	uint32_t mask_txn_detect;
	
	for (ch = 0; ch < cfg_params.num_channels; ch++) {
		camdllcode = mdllcode[ch][AMP_CA];
		
		// Calculate the Center Points for each CA bit
		for (bit_indx=0; bit_indx < CA_NUM_BITS; bit_indx++) {
			comb_mask = 0x0;
			mask_txn_detect = 0x0;
			tmp_mask = 0x0;
			
			// Compute the aggr eye over multiple swloop and hwloop for all ranks
			for (swloop = 0; swloop < cfg_params.cacalib_sw_loops; swloop++) {
				mask = 0x0;
				for (hwloop=0; hwloop < cfg_params.cacalib_hw_loops; hwloop++)
					mask = mask | cacal_patterns_mask[(swloop * cfg_params.cacalib_hw_loops) + hwloop];
					
				// An explanation of the masks is below. Note that we only recorded result for a bit from a particular iteration if the bit had a transition.
				// mask: for pattern(s) sent in this swloop, indicates if the bit had a transition
				// tmp_mask: aggregates mask over all loops, including current swloop
				// comb_mask: aggregates mask over all loops, upto the last iteration of the swloop. After it is used to generate mask_txn_detect, it catches upto same value as tmp_mask
				// mask_txn_detect: indicates the first time a bit transitioned was in this swloop
				tmp_mask = tmp_mask | mask;
				mask_txn_detect = tmp_mask ^ comb_mask;
				comb_mask = comb_mask | mask;
				
				/* 
				 * Rank 0
				 */
				
				loopchrnk0_indx = (swloop * cfg_params.num_channels * cfg_params.num_ranks) + (ch * cfg_params.num_ranks) + 0;
				chrnk0_indx = (ch * cfg_params.num_ranks) + 0;
				
				/* Left side */
				
				// lookup the value in the left side for this bit given loop, ch, and rnk
				rank_val[0] = cacal_per_loopchrnk_left[loopchrnk0_indx][bit_indx];
				tmp_left_pos_val = rank_val[0];
					
				// If this is the first time this bit transitioned, just put it in the aggregate result array
				if (mask_txn_detect & (1 << bit_indx)) {
					left_pos_val = tmp_left_pos_val;
					cacal_per_chrnk_left[chrnk0_indx][bit_indx] = left_pos_val;
				} else if (mask & (1 << bit_indx)) {
					// This is not the 1st time this bit transitioned so there is a recorded result already, but since we have a new result
					// to compare with, find the value that would cover both points and put that in the array
					left_pos_val = cacal_per_chrnk_left[chrnk0_indx][bit_indx];
					left_pos_val = find_common_endpoint(tmp_left_pos_val, left_pos_val, MIN_ENDPT);
					cacal_per_chrnk_left[chrnk0_indx][bit_indx] = left_pos_val;
				} else {
					// <rdar://problem/13834199>
					left_pos_val = tmp_left_pos_val;
					cacal_per_chrnk_left[chrnk0_indx][bit_indx] = left_pos_val;
				}
				
				/* Right side */
				
				// lookup the value in the right side for this bit given loop, ch, and rnk
				rank_val[0] = cacal_per_loopchrnk_right[loopchrnk0_indx][bit_indx];
				tmp_right_pos_val = rank_val[0];
					
				// If this is the first time this bit transitioned, just put it in the aggregate result array
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
				
				if (cfg_params.num_ranks > 1) {
					/*
					 * Rank 1
					 */
					
					if (cfg_params.num_ranks > AMP_MAX_RANKS_PER_CHAN)
						shim_panic("amp_v3: cfg_params.num_ranks = %d is more than hw is capable of supporting (%d)\n", cfg_params.num_ranks, AMP_MAX_RANKS_PER_CHAN);
					
					loopchrnk1_indx = (swloop * cfg_params.num_channels * cfg_params.num_ranks) + (ch * cfg_params.num_ranks) + 1;
					chrnk1_indx = (ch * cfg_params.num_ranks) + 1;
					
					/* Left side */
					
					// lookup the value in the left side for this bit given loop, ch, and rnk
					rank_val[1] = cacal_per_loopchrnk_left[loopchrnk1_indx][bit_indx];
					tmp_left_pos_val = rank_val[1];
					
					// If this is the first time this bit transitioned, just put it in the aggregate result array
					if (mask_txn_detect & (1 << bit_indx)) {
						left_pos_val = tmp_left_pos_val;
						cacal_per_chrnk_left[chrnk1_indx][bit_indx] = left_pos_val;
					} else if (mask & (1 << bit_indx)) {
						// This is not the 1st time this bit transitioned so there is a recorded result already, but since we have a new result
						// to compare with, find the value that would cover both points and put that in the array
						left_pos_val = cacal_per_chrnk_left[chrnk1_indx][bit_indx];
						left_pos_val = find_common_endpoint(tmp_left_pos_val, left_pos_val, MIN_ENDPT);
						cacal_per_chrnk_left[chrnk1_indx][bit_indx] = left_pos_val;
					} else {
						// <rdar://problem/13834199>
						left_pos_val = tmp_left_pos_val;
						cacal_per_chrnk_left[chrnk1_indx][bit_indx] = left_pos_val;
					}
					
					/* Right side */
					
					// lookup the value in the right side for this bit given loop, ch, and rnk
					rank_val[1] = cacal_per_loopchrnk_right[loopchrnk1_indx][bit_indx];
					tmp_right_pos_val = rank_val[1];
					
					// If this is the first time this bit transitioned, just put it in the aggregate result array
					if (mask_txn_detect & (1 << bit_indx)) {
						right_pos_val = tmp_right_pos_val;
						cacal_per_chrnk_right[chrnk1_indx][bit_indx] = right_pos_val;
					} else if (mask & (1 << bit_indx)) {
						// This is not the 1st time this bit transitioned so there is a recorded result already, but since we have a new result
						// to compare with, find the value that would cover both points and put that in the array
						right_pos_val = cacal_per_chrnk_right[chrnk1_indx][bit_indx];
						right_pos_val = find_common_endpoint(tmp_right_pos_val, right_pos_val, MAX_ENDPT);
						cacal_per_chrnk_right[chrnk1_indx][bit_indx] = right_pos_val;
					} else {
						// <rdar://problem/13834199>
						right_pos_val = tmp_right_pos_val;
						cacal_per_chrnk_right[chrnk1_indx][bit_indx] = 0;
					}

					// Find the common endpoint for both ranks
					left_pos_val = find_common_endpoint(cacal_per_chrnk_left[chrnk0_indx][bit_indx], cacal_per_chrnk_left[chrnk1_indx][bit_indx], MIN_ENDPT);
					right_pos_val = find_common_endpoint(cacal_per_chrnk_right[chrnk0_indx][bit_indx], cacal_per_chrnk_right[chrnk1_indx][bit_indx], MAX_ENDPT);
				}
				
			}
			
			// At this point, the left edge and the right edge of the eye for this channel and bit are defined by left_pos_val and right_pos_val
			// Find the center of the eye
			ca_bit_center[bit_indx] = find_center_of_eye(left_pos_val, right_pos_val);
		}
		
		// Since center for each bit may be different, find the min val
		// Min val will get programmed to the sdll, while the other bits will require deskew
		min_ca_bit_center = ca_bit_center[0];

		for (bit_indx = 1; bit_indx < CA_NUM_BITS; bit_indx++) {
			if (ca_bit_center[bit_indx] < min_ca_bit_center)
				min_ca_bit_center = ca_bit_center[bit_indx];
		}
		
		// for positive sdll, clamp it to mdllcode - DELIMIT_POS_ADJ_CASDLL
		if (min_ca_bit_center > (camdllcode - DELIMIT_POS_ADJ_CASDLL)) {
			min_ca_bit_center = camdllcode - DELIMIT_POS_ADJ_CASDLL;
		}
		
		// Since the min value of all bits is chosen for sdll, if the rest of the bits need more delay, compute their deskew
		for (bit_indx=0; bit_indx < CA_NUM_BITS; bit_indx++) {
			
			if (ca_bit_center[bit_indx] < min_ca_bit_center)
				shim_panic("Memory CA Calibration: ca_bit_center[%d] = (%d) < min_ca_bit_center = %d\n", bit_indx, ca_bit_center[bit_indx], min_ca_bit_center);
			
			ca_bit_deskew[bit_indx] = ca_bit_center[bit_indx] - min_ca_bit_center;
		}
		
		// If min < -camdllcode, then we will clamp the sdll to -mdll
		// and put the remaining delay on the CK signals
		if (min_ca_bit_center < (-1 * camdllcode)) {
			cs_adj_val = (-1 * min_ca_bit_center) - camdllcode;
			adj_ca_bit_center = (-1 * camdllcode);
		} else {
			cs_adj_val = 0;
			adj_ca_bit_center = min_ca_bit_center;
		}
		
		/*
		 * Finally, program the values
		 */
		
		// Need to convert deskew values, which have been in the sdll scale so far, into the deskew scale
		cs_adj_val = offset_convert(cs_adj_val, DIRECTION_SDLL_TO_DESKEW);
		for (bit_indx = 0; bit_indx < CA_NUM_BITS; bit_indx++) {
			ca_bit_deskew[bit_indx] = offset_convert(ca_bit_deskew[bit_indx], DIRECTION_SDLL_TO_DESKEW);
			// Make sure deskew value programmed is not negative and is <= CA_MAX_DESKEW_OFFSET
			if ((ca_bit_deskew[bit_indx] < 0) || (ca_bit_deskew[bit_indx] > CA_MAX_DESKEW_OFFSET))
				shim_panic("Memory CA Calibration: ca_bit_deskew[%d] = %d invalid\n", bit_indx, ca_bit_deskew[bit_indx]);
		}
		
		// Push the remainder of the delay to CK signals (if adj_CaBitCenterPoint_val_data was clamped to camdll)
		amp_push_ctl_out(ch, cs_adj_val);
		
		// Program the SDLL with the adjusted min value
		amp_push_casdll_out(ch, adj_ca_bit_center);

		// saved for data collection
		cacal_saved_val[ch][CSBIT] = cs_adj_val;
		cacal_saved_val[ch][CKBIT] = cs_adj_val;
		cacal_saved_val[ch][CKEBIT] = cs_adj_val;
		cacal_saved_val[ch][CASDLLBIT] = adj_ca_bit_center;
				
		// Program the CA Deskew values for each bit
		for (bit_indx = 0; bit_indx < CA_NUM_BITS; bit_indx++) {
			CSR_WRITE(rAMP_CADESKEW_CTRL(ch, bit_indx), ca_bit_deskew[bit_indx]);
			cacal_saved_val[ch][bit_indx] = ca_bit_deskew[bit_indx];
		}
	}
}

// Loop through PerBitDeskewCode ranges for rddq until failing points for each byte (& bit) are found.
static void rddqcal_find_right_failing_point(uint32_t ch, uint32_t rnk, bool after_wrddqcal)
{
	uint32_t dq_deskew;
	uint32_t all_bits_fail;
	uint32_t bits_fail_b[DQ_NUM_BYTES] = { 0 };
	uint32_t rddqcalresult, result_swiz;
	uint32_t mask_b[DQ_NUM_BYTES];
	int32_t start_b[DQ_NUM_BYTES];
	uint32_t byte, bit;//, dqbyte;
	uint32_t dq, dq_byte;
	
	all_bits_fail = 0;
	dq_deskew = 0;
	
	// set the rddq sdll to negative dqmdllcode
	for (byte = 0; byte < DQ_NUM_BYTES; byte++) {
		dq_byte = byte;
		dq = map_byte_to_dq(&dq_byte);
		amp_set_rddq_sdll(ch, byte, (1 << SIGN_BIT_POS) + mdllcode[ch][dq]);
		
		// initialize the mask for each byte lane
		mask_b[byte] = 0xFF << (byte * 8);
	}
	
	rddqcalresult = 0xFFFFFFFF;
	
	// PerBit Deskew lines cannot be pushed beyond DQ_MAX_DESKEW_PER_BIT value
	do {
		for (byte = 0; byte < DQ_NUM_BYTES; byte++) {
			if (bits_fail_b[byte] == 0) {
				
				dq_byte = byte;
				dq = map_byte_to_dq(&dq_byte);
				
				for (bit = 0; bit < DQ_NUM_BITS_PER_BYTE; bit++)
					CSR_WRITE(rAMP_RDDQDESKEW_CTRL(dq, ch, dq_byte, bit), dq_deskew);
			}
		}
				
		// Call Basic run Dq Cal Commands
		if (after_wrddqcal == false) {
			amp_run_rddqcal(ch);
			rddqcalresult &= ((CSR_READ(rAMP_DQCALRESULT(AMP_DQ1, ch)) << 16) |
								CSR_READ(rAMP_DQCALRESULT(AMP_DQ0, ch)));
		} else {
			rddqcalresult &= dqcal_wr_rd_pattern_result(ch, rnk, dq_deskew);
		}
		
		// Account for swizzled bits and bytes of the result
		result_swiz = dqcal_handle_swizzle(rddqcalresult, after_wrddqcal);
		
		for (byte = 0; byte < DQ_NUM_BYTES; byte++) {
			// if all bits haven't failed yet and this run shows all bits failing, we have found the failing point for this byte
			if ((bits_fail_b[byte] == 0) && ((result_swiz & mask_b[byte]) == 0)) {
				bits_fail_b[byte] = 1;
				start_b[byte] = dq_deskew;
			}
		}
		
		all_bits_fail = bits_fail_b[0] & bits_fail_b[1] & bits_fail_b[2] & bits_fail_b[3];
		
		if (all_bits_fail == 1) {
			// If failing point has been found for all bits, find the passing point now
			rddqcal_find_right_passing_point(ch, rnk, start_b, after_wrddqcal);
		} else {
			// To find right failing point, make more negative adjustment to the sdll (same as incrementing deskew)
			if ((dq_deskew + COARSE_STEP_SZ) <= DQ_MAX_DESKEW_PER_BIT)
				dq_deskew += COARSE_STEP_SZ;
			else
				dq_deskew += FINER_STEP_SZ;
		}
		
	} while ((dq_deskew <= DQ_MAX_DESKEW_PER_BIT) && (all_bits_fail == 0));
	
	if ((dq_deskew > DQ_MAX_DESKEW_PER_BIT) && (all_bits_fail == 0)) {
		// print error message as Right Failing Point cannot be found
		shim_printf("Memory Rddq cal: Right side failing point not found, max deskew limit reach for channel %d\n", ch);

		// Assume failure at this setting, and continue to passing point
		for (byte = 0; byte < DQ_NUM_BYTES; byte++) {
			// if all bits haven't failed yet, assign start_b for this byte to current reg setting
			if (bits_fail_b[byte] == 0)
				start_b[byte] = DQ_MAX_DESKEW_PER_BIT;
		}

		rddqcal_find_right_passing_point(ch, rnk, start_b, after_wrddqcal);
	}
	
	// Reset deskew for all bits to 0
	for (byte = 0; byte < DQ_NUM_BYTES; byte++) {
		
		dq_byte = byte;
		dq = map_byte_to_dq(&dq_byte);
		
		for (bit = 0; bit < DQ_NUM_BITS_PER_BYTE; bit++)
			CSR_WRITE(rAMP_RDDQDESKEW_CTRL(dq, ch, dq_byte, bit), 0);
	}
}

// Purpose of this function is to start from right side failing point and find locations for every DQ bit
// until the start of passing window for that bit is found
// Save all this locations to compute the center of window
static void rddqcal_find_right_passing_point(uint32_t ch, uint32_t rnk, int32_t *start_b, bool after_wrddqcal)
{
	uint32_t chrnk_indx;
	bool switch_from_dqstodq, max_tap_value_reached;
	int32_t tap_value_b[DQ_NUM_BYTES];
	uint32_t BitPass[DQ_TOTAL_BITS] = { 0 };
	uint32_t SolidBitPass[DQ_TOTAL_BITS] = { 0 };
	uint32_t rddqcalresult;
	uint32_t all_bits_pass;
	uint32_t all_bits_pass_b[DQ_NUM_BYTES] = { 0 };
	uint32_t bit_indx, byte;
	int32_t dqmdllcode;
	int32_t saved_val;
	uint32_t dq, dq_byte;
	
	chrnk_indx = (ch * cfg_params.num_ranks) + rnk;
	all_bits_pass = 0;
	switch_from_dqstodq = false;
	max_tap_value_reached = false;

	for (byte = 0; byte < DQ_NUM_BYTES; byte++) {
		tap_value_b[byte] = start_b[byte];
	}
	
	// Moving Right to Left to find point where each bit turns from FAIL TO PASS
	do {
		if (switch_from_dqstodq == false) {
			// continue to update per bit deskew until all bits pass for each byte lane
			for (byte = 0; byte < DQ_NUM_BYTES; byte++) {
				if (all_bits_pass_b[byte] == 0) {
					
					dq_byte = byte;
					dq = map_byte_to_dq(&dq_byte);
					
					for (bit_indx = 0; bit_indx < DQ_NUM_BITS_PER_BYTE; bit_indx++)
						CSR_WRITE(rAMP_RDDQDESKEW_CTRL(dq, ch, dq_byte, bit_indx), tap_value_b[byte]);
				}
			}
		} else {
			// adjust rddq sdll until all bits pass for each byte lane
			for (byte = 0; byte < DQ_NUM_BYTES; byte++)
				if (all_bits_pass_b[byte] == 0)
					amp_set_rddq_sdll(ch, byte, INT_TO_OFFSET(tap_value_b[byte]));
		}
		
		// Run rddq calibration in hw
		if (after_wrddqcal == false) {
			amp_run_rddqcal(ch);
			rddqcalresult = ((CSR_READ(rAMP_DQCALRESULT(AMP_DQ1, ch)) << 16) |
							 CSR_READ(rAMP_DQCALRESULT(AMP_DQ0, ch)));
		} else {
			rddqcalresult = dqcal_wr_rd_pattern_result(ch, rnk, tap_value_b[0]);
		}
		
		// Account for swizzled bits and bytes of the result
		rddqcalresult = dqcal_handle_swizzle(rddqcalresult, after_wrddqcal);
		
		// Make sure that each Bit sees a transition from 0 to 1 on DqCalresult Register
		for (bit_indx = 0; bit_indx < DQ_TOTAL_BITS; bit_indx++) {
			
			byte = bit_indx >> 3; // bit_indx / DQ_NUM_BITS_PER_BYTE
			
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
					
					byte = bit_indx >> 3; // bit_indx / DQ_NUM_BITS_PER_BYTE 
					
					if (switch_from_dqstodq == false) {
						dq_byte = byte;
						dq = map_byte_to_dq(&dq_byte);
						dqmdllcode = mdllcode[ch][dq];
						
						// consider mdllcode as '0' since sdll is set to -mdllcode
						// need to convert tap_valyue_b into sdll scale, since saved value is in units of sdll steps
						saved_val = -1 * (offset_convert(tap_value_b[byte], DIRECTION_DESKEW_TO_SDLL) + dqmdllcode);
					} else
						saved_val = tap_value_b[byte];
						
					rddqcal_per_chrnk_right[chrnk_indx][bit_indx] = saved_val;
				}
			} else {
				// bit failed calibration, reset the pass count to 0
				SolidBitPass[bit_indx] = 0;
			}
		}
		
		all_bits_pass = 1;
		for (byte = 0; byte < DQ_NUM_BYTES; byte++)
			all_bits_pass_b[byte] = 1;
		
		for (bit_indx = 0; bit_indx < DQ_TOTAL_BITS; bit_indx++) {
			byte = bit_indx >> 3; // bit_indx / DQ_NUM_BITS_PER_BYTE
			
			// Did all the bits pass (SOLID_PASS_DETECT number of times) in this byte lane?
			// If anyone of the bits failed, then the byte flag is cleared
			all_bits_pass_b[byte] = all_bits_pass_b[byte] & BitPass[bit_indx];
			
			// Did all bits in all byte lanes pass?
			all_bits_pass = all_bits_pass & BitPass[bit_indx];
		}		
		
		// If ALL bits are not passing - keep moving from Right to Left Side of window (by adding less negative adjustment to mdll)
		if (all_bits_pass == 0) {
			// Even if one of the byte lanes arrives early to tap_value = 0. Remain here until all byte lane catch up before proceeding to pushing out dq
			
			// if the tap_value for all bytes has reached 0 on the deskew, make the transition to SDLL
			if (switch_from_dqstodq == false) {
				
				// check for all bytes reaching 0 on the tap value (could be deskew or sdll)
				int32_t all_bytes_tap = tap_value_b[0];
				for (byte = 1; (byte < DQ_NUM_BYTES) && (all_bytes_tap == 0); byte++) {
					all_bytes_tap += tap_value_b[byte];
				}
				
				if (all_bytes_tap == 0) {
					switch_from_dqstodq = true;
				
					for (byte = 0; byte < DQ_NUM_BYTES; byte++) {
						dq_byte = byte;
						dq = map_byte_to_dq(&dq_byte);
	
						tap_value_b[byte] = OFFSET_TO_INT(CSR_READ(rAMP_DQSDLLCTRL_RD(dq, ch, dq_byte)) & DLLVAL_BITS);
					}
				}
			}
			
			// To find right side passing point, add less negative adjustment to mdll (same as decrementing deskew)
			
			// For deskew taps, we just decrement by FINER_STEP_SZ if we haven't reached 0 yet
			if (switch_from_dqstodq == false) {
				for (byte = 0; byte < DQ_NUM_BYTES; byte++)
					if (tap_value_b[byte] > 0)
						tap_value_b[byte] -= FINER_STEP_SZ;
			} else {
				// For sdll taps, increment it
				for (byte = 0; byte < DQ_NUM_BYTES; byte++) {
					if (all_bits_pass_b[byte] == 0) {
						tap_value_b[byte] += FINER_STEP_SZ;
					}
				}
			}
		}
		
		// trigger for loop to end if any of the bytes reach max tap value
		for (byte = 0; byte < DQ_NUM_BYTES; byte++) {
			if (!max_tap_value_reached)
				max_tap_value_reached = (tap_value_b[byte] > MAX_SDLL_VAL);
			
			if (max_tap_value_reached) {
				if (all_bits_pass == 0)
					shim_panic("Memory rddq calibration: Unable to find right side passing point, max tap value reached");
				
				break;
			}
		}
		
	} while ((!max_tap_value_reached) && (all_bits_pass == 0));
}

// Purpose of this function is to start push DQS out till left side failing point of Data window is found
static void rddqcal_find_left_failing_point(uint32_t ch, uint32_t rnk, bool after_wrddqcal)
{
	int32_t rddqsdll[DQ_NUM_BYTES];
	uint32_t rddqcalresult, result_swiz;
	uint32_t all_bits_fail;
	uint32_t all_bits_fail_b[DQ_NUM_BYTES] = { 0 };
	uint32_t mask_b[DQ_NUM_BYTES];
	int32_t start_b[DQ_NUM_BYTES];
	uint32_t byte, dq, dq_byte;
	bool max_tap_value_reached = false;
		
	all_bits_fail = 0;
	rddqcalresult = 0xFFFFFFFF;
	
	for (byte = 0; byte < DQ_NUM_BYTES; byte++) {
		// initialize the mask for each byte lane
		mask_b[byte] = 0xFF << (byte * 8);
		
		dq_byte = byte;
		dq = map_byte_to_dq(&dq_byte);
		
		// Get the starting values for RD DQS SDLL
		rddqsdll[byte] = OFFSET_TO_INT(CSR_READ(rAMP_DQSDLLCTRL_RD(dq, ch, dq_byte)) & DLLVAL_BITS);
	}
		
	// To find left failing point, keep adding less negative adjustment to mdll
	do {
		for (byte = 0; byte < DQ_NUM_BYTES; byte++) {
			// set the new sdll for this byte lane if all bits are not yet failing
			if (all_bits_fail_b[byte] == 0)
				amp_set_rddq_sdll(ch, byte, INT_TO_OFFSET(rddqsdll[byte]));
		}
		
		// Run rddqcal in hw
		if (after_wrddqcal == false) {
			amp_run_rddqcal(ch);
			rddqcalresult &= ((CSR_READ(rAMP_DQCALRESULT(AMP_DQ1, ch)) << 16) |
							  CSR_READ(rAMP_DQCALRESULT(AMP_DQ0, ch)));
		} else {
			rddqcalresult &= dqcal_wr_rd_pattern_result(ch, rnk, rddqsdll[0]);
		}
		
		// Account for swizzled bits and bytes of the result
		result_swiz = dqcal_handle_swizzle(rddqcalresult, after_wrddqcal);
		
		// If the result of all bits in this byte show a fail, record this as the failing point
		all_bits_fail = 1;
		for (byte = 0; byte < DQ_NUM_BYTES; byte++) {
			if ((all_bits_fail_b[byte] == 0) && ((result_swiz & mask_b[byte]) == 0)) {
				all_bits_fail_b[byte] = 1;
				start_b[byte] = rddqsdll[byte];
			}
			
			all_bits_fail &= all_bits_fail_b[byte];
		}

		// all bytes fail, call the function to find left passing point
		if (all_bits_fail == 1) {
			rddqcal_find_left_passing_point(ch, rnk, start_b, after_wrddqcal);
		} else {
			// if the byte has not yet failed, find the next sdll value to be set
			for (byte = 0; byte < DQ_NUM_BYTES; byte++) {
				if (all_bits_fail_b[byte] == 0) {
					if ((rddqsdll[byte] + COARSE_STEP_SZ) <= MAX_SDLL_VAL)
						rddqsdll[byte] += COARSE_STEP_SZ;
					else
						rddqsdll[byte] += FINER_STEP_SZ;
				}
			}
		}
		
		for (byte = 0; byte < DQ_NUM_BYTES; byte++) {
			// none of the previous bytes reached max_tap_value, then update the boolean
			if (!max_tap_value_reached) {
				max_tap_value_reached = (rddqsdll[byte] > MAX_SDLL_VAL);
				
				if (max_tap_value_reached) {
					shim_printf("Memory rddq calibration: Unable to find left failing point, max tap value reached for ch %d byte %d\n", ch, byte);
					break;
				}
			}
		}
		
		if (max_tap_value_reached) {
			// Continue to passing point if any of the bytes reaches max value and not all bits are failing
			if (all_bits_fail == 0) {
				for (byte = 0; byte < DQ_NUM_BYTES; byte++) {
					if (all_bits_fail_b[byte] == 0)
						start_b[byte] = MAX_SDLL_VAL;
				}

				rddqcal_find_left_passing_point(ch, rnk, start_b, after_wrddqcal);
			}
		}
		
	} while ((!max_tap_value_reached) && (all_bits_fail == 0));
}

// Purpose of this function is to start from left side failing point and find passing locations for every DQ bit on left side of window
// Save all the locations to compute the center of window later
// To find left passing point, move to the right from the failing point, which means keep adding more negative adjustment to mdll
static void rddqcal_find_left_passing_point(uint32_t ch, uint32_t rnk, int32_t *start_b, bool after_wrddqcal)
{
	uint32_t chrnk_indx;
	bool max_tap_value_reached = false;
	int32_t tap_value_b[DQ_NUM_BYTES];
	uint32_t BitPass[DQ_TOTAL_BITS] = { 0 };
	uint32_t SolidBitPass[DQ_TOTAL_BITS] = { 0 };
	uint32_t rddqcalresult;
	uint32_t all_bits_pass;
	uint32_t all_bits_pass_b[DQ_NUM_BYTES] = { 0 };
	uint32_t bit_indx, byte;
	
	chrnk_indx = (ch * cfg_params.num_ranks) + rnk;
	all_bits_pass = 0;
	rddqcalresult = 0xFFFFFFFF;

	for (byte = 0; byte < DQ_NUM_BYTES; byte++) {
		tap_value_b[byte] = start_b[byte];
	}

	// Finding Left side passing point on per bit level. Moving Left to Right (keep adding more negative adj to mdll) to find point where it turns from FAIL TO PASS
	do {
		
		for (byte = 0; byte < DQ_NUM_BYTES; byte++) {
			// if we haven't found all bits passing for this byte, push out new sdll value
			if (all_bits_pass_b[byte] == 0)
				amp_set_rddq_sdll(ch, byte, INT_TO_OFFSET(tap_value_b[byte]));
		}
				
		// Run rddqcal in hw
		if (after_wrddqcal == false) {
			amp_run_rddqcal(ch);
			rddqcalresult = ((CSR_READ(rAMP_DQCALRESULT(AMP_DQ1, ch)) << 16) |
							 CSR_READ(rAMP_DQCALRESULT(AMP_DQ0, ch)));
		} else {
			rddqcalresult = dqcal_wr_rd_pattern_result(ch, rnk, tap_value_b[0]);
		}
		
		rddqcalresult = dqcal_handle_swizzle(rddqcalresult, after_wrddqcal);
		
		// Make sure that each Bit sees a transition from 0 to 1 on DqCalresult Register
		for (bit_indx = 0; bit_indx < DQ_TOTAL_BITS; bit_indx++) {

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
					
					byte = bit_indx >> 3; // bit_indx / DQ_NUM_BITS_PER_BYTE
					
					rddqcal_per_chrnk_left[chrnk_indx][bit_indx] = tap_value_b[byte];
				}
			} else {
				// bit failed calibration, reset the pass count to 0
				SolidBitPass[bit_indx] = 0;
			}

		}

		all_bits_pass = 1;
		for (byte = 0; byte < DQ_NUM_BYTES; byte++)
			all_bits_pass_b[byte] = 1;
		
		for (bit_indx = 0; bit_indx < DQ_TOTAL_BITS; bit_indx++) {
			byte = bit_indx >> 3; // bit_indx / DQ_NUM_BITS_PER_BYTE
			
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
				if (all_bits_pass_b[byte] == 0)
					tap_value_b[byte] -= FINER_STEP_SZ;
			}
		}
		
		// check for end of loop condition
		for (byte = 0; byte < DQ_NUM_BYTES; byte++) {
			uint32_t dq_byte = byte;
			uint32_t dq = map_byte_to_dq(&dq_byte);
			
			if (!max_tap_value_reached)
				max_tap_value_reached = (tap_value_b[byte] < (-1 * MAX_SDLL_VAL));

 			if (max_tap_value_reached) {
				if (all_bits_pass == 0)
					shim_panic("Memory rddq calibration: Unable to find left passing point, max tap value reached");
 				break;
 			}
 						
			// panic if we get beyond -dqmdllcode, since we really shouldn't have to go that far
			if ((all_bits_pass == 0) && (tap_value_b[byte] < (-1 * mdllcode[ch][dq])))
				shim_panic("Memory rddq calibration: Not yet found left passing point but SDLL < -dqmdllcode for ch %d byte %d", ch, byte);
		}
	} while ((!max_tap_value_reached) && (all_bits_pass == 0));
}

static void rddqcal_program_final_values(void)
{
	uint32_t ch, bit_indx, byte;
	uint32_t chrnk0_indx, chrnk1_indx;
	uint32_t dq, dq_byte;
	int32_t rddq_bit_center[DQ_TOTAL_BITS];
	int32_t rddq_bit_deskew[DQ_TOTAL_BITS];
	int32_t left_pos_val;
	int32_t right_pos_val;
	int32_t max_rddq_center[DQ_NUM_BYTES];
	int32_t dqmdllcode;
	
	for (ch = 0; ch < cfg_params.num_channels; ch++) {
		
		// find the center point of passing window for each bit over all ranks
		for (bit_indx = 0; bit_indx < DQ_TOTAL_BITS; bit_indx++) {
		
			chrnk0_indx = (ch * cfg_params.num_ranks) + 0;
			left_pos_val = rddqcal_per_chrnk_left[chrnk0_indx][bit_indx];
			right_pos_val = rddqcal_per_chrnk_right[chrnk0_indx][bit_indx];
			
			if (cfg_params.num_ranks > 1) {
				
				chrnk1_indx = (ch * cfg_params.num_ranks) + 1;
				
				// find the endpoint that covers both ranks
				left_pos_val = find_common_endpoint(rddqcal_per_chrnk_left[chrnk0_indx][bit_indx],
								    rddqcal_per_chrnk_left[chrnk1_indx][bit_indx],
								    MIN_ENDPT);
				right_pos_val = find_common_endpoint(rddqcal_per_chrnk_right[chrnk0_indx][bit_indx],
								     rddqcal_per_chrnk_right[chrnk1_indx][bit_indx],
								     MAX_ENDPT);
			}
			
			// find center of the eye for this bit
			rddq_bit_center[bit_indx] = find_center_of_eye(left_pos_val, right_pos_val);
		}
		
		// initialize the max centerpoint to the 1st bit's center point in each byte lane
		for (byte = 0; byte < DQ_NUM_BYTES; byte++)
			max_rddq_center[byte] = rddq_bit_center[byte * DQ_NUM_BITS_PER_BYTE];

		// Find the maximum CenterPoint per byte lane given each bit's center point
		for (bit_indx = 0; bit_indx < DQ_TOTAL_BITS; bit_indx++) {
			
			byte = bit_indx >> 3; // bit_indx / DQ_NUM_BITS_PER_BYTE
			
			// if this bit's center point is greater than current max, make it the new max (we'll program this to sdll, and other values will require deskew)
			if (rddq_bit_center[bit_indx] > max_rddq_center[byte])
				max_rddq_center[byte] = rddq_bit_center[bit_indx];
		}
		
		// if the max for each byte lane is < -dqmdllcode, clamp it to -dqmdllcode (the remainder will go on per bit deskew)
		for (byte = 0; byte < DQ_NUM_BYTES; byte++) {
			
			dq_byte = byte;
			dq = map_byte_to_dq(&dq_byte);
			dqmdllcode = mdllcode[ch][dq];
			
			if (max_rddq_center[byte] < (-1 * dqmdllcode))
				max_rddq_center[byte] = (-1 * dqmdllcode);
		}
		
		// Compute the individual deskew values: any bits with center point < max for its byte lane will require deskew
		// Each bit's center is guaranteed to be <= max for its byte lane
		// Deskewing means adding more negative adjustment for this bit in addition to the sdll, which is clamped on the negative side to -dqmdllcode
		for (bit_indx = 0; bit_indx < DQ_TOTAL_BITS; bit_indx++) {
			
			byte = bit_indx >> 3; // bit_indx / DQ_NUM_BITS_PER_BYTE
			
			dq_byte = byte;
			dq = map_byte_to_dq(&dq_byte);
			dqmdllcode = mdllcode[ch][dq];

			if (rddq_bit_center[bit_indx] > max_rddq_center[byte])
				shim_panic("Memory Rddq calibration: rddq_bit_center[%d] = %d > max_rddq_center[%d] = %d\n", bit_indx, rddq_bit_center[bit_indx], byte, max_rddq_center[byte]);
				
			rddq_bit_deskew[bit_indx] = max_rddq_center[byte] - rddq_bit_center[bit_indx];
		
			// need to convert the deskew value, which is in sdll scale so far, into deskew scale
			rddq_bit_deskew[bit_indx] = offset_convert(rddq_bit_deskew[bit_indx], DIRECTION_SDLL_TO_DESKEW);
			
			if ((rddq_bit_deskew[bit_indx] < 0) || (rddq_bit_deskew[bit_indx] > DQ_MAX_DESKEW_PER_BIT))
				shim_panic("Memory Rddq calibration: rddq_bit_deskew[%d] = %d invalid\n", bit_indx, rddq_bit_deskew[bit_indx]);
		}
		
		// Program the SDLL and deskew per bit for each byte lane
		for (byte = 0; byte < DQ_NUM_BYTES; byte++) {
			amp_set_rddq_sdll(ch, byte, INT_TO_OFFSET(max_rddq_center[byte]));
			
			dq_byte = byte;
			dq = map_byte_to_dq(&dq_byte);
			
			// per bit deskew for this byte lane
			for (bit_indx = 0; bit_indx < DQ_NUM_BITS_PER_BYTE; bit_indx++) {
				CSR_WRITE(rAMP_RDDQDESKEW_CTRL(dq, ch, dq_byte, bit_indx), rddq_bit_deskew[(byte * DQ_NUM_BITS_PER_BYTE) + bit_indx]);
			}
		}
	} // for (ch = 0; ch < cfg_params.num_channels; ch++)
}

/* We want to end up returning the result in the following order
 * B3.B1.B2.B0, where B0 and B2 are mapped to ampdq0[0:7] and ampdq[8:15], respectively
 * and B1 and B3 are mapped to ampdq1[0:7] and ampdq1[8:15], respectively.
 * In addition, ampdq0 bytes (B0 and B2) requires their bits unswizzled.
 */
static uint32_t dqcal_handle_swizzle(uint32_t result, bool after_wrddqcal)
{
	uint32_t result_unswz, result_tmp;
	uint8_t result_byte;
	
	result_unswz = 0;
	
	if (after_wrddqcal) {
		// In the case of PRBS patterns, the result was constructed
		// by the CPU, which has normal byte order: B3.B2.B1.B0, so swap them around
		// to produce B3.B1.B2.B0
		
		// B3 and B0 are in the right place already, just swap around B2 and B1
		result_tmp = result & (0xFF0000FF);
		result_tmp |= (result & 0x00FF0000) >> 8;
		result_tmp |= (result & 0x0000FF00) << 8;
	} else {
		// In the case of rddq before wrdq, the result was constructed
		// with the byte order we want: B3.B1.B2.B0, so no byte swapping required
		result_tmp = result;
	}

	// Undo bit swizzling in DQ0 only (lower 16 bits of the result)
	result_byte = (result_tmp & 0x000000FF);
	result_byte = reverse_bits_in_byte(result_byte);
	result_unswz = result_byte;
	result_byte = (result_tmp & 0x0000FF00) >> 8;
	result_byte = reverse_bits_in_byte(result_byte);
	result_unswz |= result_byte << 8;
	
	// Now simply or in the top 16 bits (DQ1 does not have bits swizzled)
	result_unswz |= (result_tmp & 0xFFFF0000);
	
	return result_unswz;
}

static uint8_t reverse_bits_in_byte(uint8_t byte)
{
	uint8_t i, tmp;
	uint8_t reversed_byte = 0;
	
	for (i = 0; i < 8; i++) {
		tmp = byte & (1 << i);
		tmp >>= i;
		reversed_byte |= (tmp << (7 - i));
	}
	
	return reversed_byte;
}

static uint32_t wrlvlcal_encode_dlyval(uint32_t ch, uint32_t phy_type, uint32_t val)
{
	uint32_t ret_val, mdll;

	mdll = (uint32_t) mdllcode[ch][phy_type];

	if (val < ( (2 * mdll) - 2 )) {
		ret_val = 0;
	} else if (val < ( 2 * mdll )) {
		ret_val = 1;
	} else if (val < ( (2 * mdll) + 3 )) {
		ret_val = 2;
	} else {
		ret_val = 3;
	}
	
	return ret_val;
}

static uint32_t wrlvlcal_encode_clk90dly(uint32_t ch, uint32_t phy_type, uint32_t val)
{
	uint32_t ret_val, mdll;

	mdll = (uint32_t) mdllcode[ch][phy_type];

	if (val < (mdll - 2)) {
		ret_val = 0;
	} else if (val < mdll) {
		ret_val = 1;
	} else if (val < (mdll + 3)) {
		ret_val = 2;
	} else {
		ret_val = 3;
	}
	
	return ret_val;
}

static void wrlvlcal_push_to_0s_region(uint32_t ch, uint32_t rnk)
{
	uint32_t wrlvldata, byte;
	uint32_t cawrlvlcode = 0;
	bool max_tap_value_reached = false;
	uint32_t wrlvlrun = 0xF;
	uint32_t dqwrlvlcode[DQ_NUM_BYTES] = { 0 };
	
	// Note that incrementing cawrlvl sdll has opposite effect of incrementing dqwrlvl
	
	do {
		// If any byte lane shows that it returned a value of 1 - push ca wrlvl sdll out by 1 tap
		cawrlvlcode++;
		amp_set_cawrlvl_sdll(ch, cawrlvlcode, false);
		
		for (byte = 0; byte < DQ_NUM_BYTES; byte++) {
			// if this byte already showed a 0 during last run, push dqwrlvl sdll by 1 tap
			// this is done to ensure this byte remains at 0 despite cawrlvl sdll being incremented above
			if ((wrlvlrun & (1 << byte)) == 0) {
				dqwrlvlcode[byte]++;
				amp_set_dqwrlvl_sdll(ch, byte, dqwrlvlcode[byte], false);
			}
		}
		
		// Run Wrlvl calibration in hw
		amp_run_wrlvlcal(ch, wrlvlrun);
		
		// result in reported in AMPWRLVLDATA register
		wrlvldata = (CSR_READ(rAMP_DQWRLVLDATA(AMP_DQ1, ch)) << 16) |
					(CSR_READ(rAMP_DQWRLVLDATA(AMP_DQ0, ch)));
		
		// check if all bits for this byte returned a 0, then this byte is done
		for (byte = 0; byte < DQ_NUM_BYTES; byte++) {
			if (wrlvlrun & (1 << byte)) {
				if ((wrlvldata & (0xFF << (byte * DQ_NUM_BITS_PER_BYTE))) == 0)
					wrlvlrun &= ~(1 << byte);
			}
		}
		
		// Exit if ca or dq wrlvl sdlls reach max tap value
		if (cawrlvlcode == MAX_CAWRLVL_CODE) {
			max_tap_value_reached = true;
			if (wrlvlrun)
				shim_panic("Memory Wrlvl calibration: CA sdll reached max tap value, yet all bytes not all 0s");
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
	} while (wrlvlrun && !max_tap_value_reached);
}

// Keep incrementing dqsdll until the byte shows 1s again. This counters the casdll that was incremented previously in order to show 0s
static void wrlvlcal_find_0to1_transition(uint32_t ch, uint32_t rnk)
{
	uint32_t chrnk_indx, byte, dq, dq_byte;
	uint32_t wrlvlrun, wrlvldata;
	bool max_tap_value_reached = false;
	uint32_t dqwrlvlcode[DQ_NUM_BYTES];
	uint32_t cawrlvlcode = CSR_READ(rAMP_CAWRLVLSDLLCODE(ch));
	
	wrlvlrun = 0xF;
	wrlvldata = 0;
	
	for (byte = 0; byte < DQ_NUM_BYTES; byte++) {
		dq_byte = byte;
		dq = map_byte_to_dq(&dq_byte);
		dqwrlvlcode[byte] = CSR_READ(rAMP_DQWRLVLSDLLCODE(dq, ch, dq_byte));
	}
	
	chrnk_indx = (ch * cfg_params.num_ranks) + rnk;
	
	do {
		for (byte = 0; byte < DQ_NUM_BYTES; byte++) {
			// if this byte is still showing a 0, increment the sdll
			if (wrlvlrun & (1 << byte)) {
				dqwrlvlcode[byte]++;
				amp_set_dqwrlvl_sdll(ch, byte, dqwrlvlcode[byte], false);
			}
		}
		
		// run the wrlvl calibration in hw
		amp_run_wrlvlcal(ch, wrlvlrun);
		
		wrlvldata = (CSR_READ(rAMP_DQWRLVLDATA(AMP_DQ1, ch)) << 16) |
				(CSR_READ(rAMP_DQWRLVLDATA(AMP_DQ0, ch)));
		
		// check if all bits return a 1 for this byte, then this byte is done
		for (byte = 0; byte < DQ_NUM_BYTES; byte++) {
			if (wrlvlrun & (1 << byte)) {
				if (((int) (wrlvldata & (0xFF << (byte * DQ_NUM_BITS_PER_BYTE)))) == (0xFF << (byte * DQ_NUM_BITS_PER_BYTE)))
					wrlvlrun &= ~(1 << byte);
			}
		}
		
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
	// in the "5th byte" entry, save the cawrlvl code
	wrlvlcal_per_chrnk_rise[chrnk_indx][byte] = cawrlvlcode;
}

// Go back towards the 0s region (that was found earlier). Note: not trying to find the next edge, just the previous edge that was found already
static void wrlvlcal_find_1to0_transition(uint32_t ch, uint32_t rnk)
{
	uint32_t chrnk_indx, byte, dq, dq_byte;
	uint32_t wrlvlrun, wrlvldata;
	bool max_tap_value_reached = false;
	uint32_t dqwrlvlcode[DQ_NUM_BYTES];
	uint32_t cawrlvlcode = CSR_READ(rAMP_CAWRLVLSDLLCODE(ch));
	bool incr_cawrlvl = false;
	
	chrnk_indx = (ch * cfg_params.num_ranks) + rnk;
	wrlvlrun = 0xF;

	// jump ahead by SOLID_PASS_DETECT into the 1s region
	for (byte = 0; byte < DQ_NUM_BYTES; byte++) {
		dq_byte = byte;
		dq = map_byte_to_dq(&dq_byte);
		dqwrlvlcode[byte] = CSR_READ(rAMP_DQWRLVLSDLLCODE(dq, ch, dq_byte));
		dqwrlvlcode[byte] += (SOLID_PASS_DETECT + 1); // + 1 because code is decremented before programming the sdll
	}
			
	do {
		// Make sure dqwrlvlsdll > 0, otherwise switch to cawrlvlsdll
		for (byte = 0; (byte < DQ_NUM_BYTES) && !incr_cawrlvl; byte++) {
			if (dqwrlvlcode[byte] == 0)
				incr_cawrlvl = true;
		}
		
		// if we've reached 0 on any dqwrlvlsdll that were being decremented, switch to incrementing the cawrlvlsdll (same effect)
		if (incr_cawrlvl) {
			cawrlvlcode++;
			amp_set_cawrlvl_sdll(ch, cawrlvlcode, false);
			
			// In order to keep bytes that have transitioned to 0 to stay there, increment dqwrlvlsdll (counters effect of incrementing cawrlvlsdll)
			for (byte = 0; byte < DQ_NUM_BYTES; byte++) {
				if ((wrlvlrun & (1 << byte)) == 0) {
					dqwrlvlcode[byte]++;
					amp_set_dqwrlvl_sdll(ch, byte, dqwrlvlcode[byte], false);
				}
			}
		} else {
			// if run bit is set for this byte, push out the new sdll value after decrementing by 1
			for (byte = 0; byte < DQ_NUM_BYTES; byte++) {
				if (wrlvlrun & (1 << byte)) {
					dqwrlvlcode[byte]--;
					amp_set_dqwrlvl_sdll(ch, byte, dqwrlvlcode[byte], false);
				}
			}
		}
		
		// run the wrlvl calibration in hw
		amp_run_wrlvlcal(ch, wrlvlrun);
		
		wrlvldata = (CSR_READ(rAMP_DQWRLVLDATA(AMP_DQ1, ch)) << 16) |
				(CSR_READ(rAMP_DQWRLVLDATA(AMP_DQ0, ch)));
		
		// check if all bits for this byte returned a 0, then this byte is done
		for (byte = 0; byte < DQ_NUM_BYTES; byte++) {
			if (wrlvlrun & (1 << byte)) {
				if ((wrlvldata & (0xFF << (byte * DQ_NUM_BITS_PER_BYTE))) == 0)
					wrlvlrun &= ~(1 << byte);
			}
		}
		
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

	// in the "5th byte" entry, save the cawrlvl code
	wrlvlcal_per_chrnk_fall[chrnk_indx][byte] = cawrlvlcode;
}

static void wrlvlcal_program_final_values(void)
{
	uint32_t ch, chrnk0_indx, chrnk1_indx;
	uint32_t rank_rise_val[AMP_MAX_RANKS_PER_CHAN], rank_fall_val[AMP_MAX_RANKS_PER_CHAN];
	uint32_t edge_pos[AMP_MAX_RANKS_PER_CHAN];
	uint32_t common_edge_pos, min_edge_pos;
	uint32_t byte;
	
	for (ch = 0; ch < cfg_params.num_channels; ch++) {
		// we go upto DQ_NUM_BYTES + 1 to also take into account the cawrlvlcode that is stored in the 5th element
		for (byte = 0; byte < (DQ_NUM_BYTES + 1); byte++) {
			
			// Rank 0
			chrnk0_indx = (ch * cfg_params.num_ranks) + 0;
			rank_rise_val[0] = wrlvlcal_per_chrnk_rise[chrnk0_indx][byte];
			rank_fall_val[0] = wrlvlcal_per_chrnk_fall[chrnk0_indx][byte];
			// average of 2 values is the edge for this rank
			edge_pos[0] = (rank_rise_val[0] + rank_fall_val[0]) >> 1;
			common_edge_pos = edge_pos[0];

			// Adjust for Dual ranks
			if (cfg_params.num_ranks > 1) {
				chrnk1_indx = (ch * cfg_params.num_ranks) + 1;
				rank_rise_val[1] = wrlvlcal_per_chrnk_rise[chrnk1_indx][byte];
				rank_fall_val[1] = wrlvlcal_per_chrnk_fall[chrnk1_indx][byte];
				edge_pos[1] = (rank_rise_val[1] + rank_fall_val[1]) >> 1;
				
				// common_edge_pos between both ranks is simply their average
				common_edge_pos = (edge_pos[0] + edge_pos[1]) >> 1;
			}

			// save the wrlvlsdll for each byte (and the ca)
			wrlvlcal_saved_val[ch][byte] = common_edge_pos;
		}
		
		// Find the min among all bytes (and the ca)
		min_edge_pos = wrlvlcal_saved_val[ch][DQ_NUM_BYTES];		// initialize min as the cawrlvlsdll
		for (byte = 0; byte < DQ_NUM_BYTES; byte++)
			if (wrlvlcal_saved_val[ch][byte] < min_edge_pos)
				min_edge_pos = wrlvlcal_saved_val[ch][byte];
		
		// We'll subtract the min from all 5 sdlls, including ca
		// so the byte sdlls which are in opposite direction also need to be adjusted
		for (byte = 0; byte < (DQ_NUM_BYTES + 1); byte++) {
			wrlvlcal_saved_val[ch][byte] -= min_edge_pos;
			
			// Program the values into the registers
			if (byte == DQ_NUM_BYTES) {
				// cawrlvl (use dlysel, which will require phyupdt and forceckelow)
				amp_set_cawrlvl_sdll(ch, wrlvlcal_saved_val[ch][byte], true);
			} else {
				// dqwrlvl (use dlysel, which will require phyupdt and forceckelow)
				amp_set_dqwrlvl_sdll(ch, byte, wrlvlcal_saved_val[ch][byte], true);
			}
		}
	}
}


// Keep pushing out WRDQS lines (controlled by WRDQM registers, oddly) until right side failing point is found
static void wrdqcal_find_right_failing_point(uint32_t ch, uint32_t rnk)
{
	uint32_t push_dqs_out;
	uint32_t all_bits_fail;
	uint32_t bits_fail_b[DQ_NUM_BYTES] = { 0 };
	uint32_t wrdqcalresult_swiz, wrdqcalresult;
	uint32_t mask_b[DQ_NUM_BYTES];
	int32_t start_b[DQ_NUM_BYTES];
	uint32_t byte, dq_byte, dq;
	uint32_t cawrlvlcode = CSR_READ(rAMP_CAWRLVLSDLLCODE(ch));
	
	all_bits_fail = 0;
	push_dqs_out = 0;
	wrdqcalresult = 0xFFFFFFFF;
	
	// set the wrdq sdll to negative dqmdllcode
	for (byte = 0; byte < DQ_NUM_BYTES; byte++) {
		dq_byte = byte;
		dq = map_byte_to_dq(&dq_byte);
		
		amp_set_wrdq_sdll(ch, byte, (-1 * mdllcode[ch][dq]));
		
		// initialize the mask for each byte lane
		mask_b[byte] = 0xFF << (byte * 8);
	}
	
	do {
		// NOTE: When DQS are pushed out then - cawrlvl sdll needs to be pushed out as well with equal taps
		// can use dlysel (with phyupdt and forceckelow)
		amp_set_cawrlvl_sdll(ch, cawrlvlcode + push_dqs_out, true);
	
		// Keep pushing per bit DQS (controlled by DM regs, oddly) out until all bytes start to fail
		for (byte = 0; byte < DQ_NUM_BYTES; byte++) {
			dq_byte = byte;
			dq = map_byte_to_dq(&dq_byte);
			CSR_WRITE(rAMP_WRDMDESKEW_CTRL(dq, ch, dq_byte), push_dqs_out);
		}
		
		// Perform the WrDq calibration with PRBS patterns
		wrdqcalresult &= dqcal_wr_rd_pattern_result(ch, rnk, push_dqs_out);
		wrdqcalresult_swiz = dqcal_handle_swizzle(wrdqcalresult, true);
		
		all_bits_fail = 1;
		for (byte = 0; byte < DQ_NUM_BYTES; byte++) {
			if (bits_fail_b[byte] == 0) {
				// if all bits fail for this byte for the 1st time, we've found the right failing point
				if ((wrdqcalresult_swiz & mask_b[byte]) == 0) {
					bits_fail_b[byte] = 1;
					start_b[byte] = push_dqs_out;
				}
			}
			
			all_bits_fail &= bits_fail_b[byte];
		}
		
		// if all bits in all bytes fail, find the right passing point
		if (all_bits_fail == 1) {
			wrdqcal_find_right_passing_point(ch, rnk, start_b);
		} else {
			// increase the deskew since all bits are not yet failing
			if ((push_dqs_out + COARSE_STEP_SZ) <= DQ_MAX_DESKEW_PER_BIT)
				push_dqs_out += COARSE_STEP_SZ;
			else
				push_dqs_out += FINER_STEP_SZ;
		}
		
	} while ((push_dqs_out <= DQ_MAX_DESKEW_PER_BIT) && (all_bits_fail == 0));
	
	if ((push_dqs_out > DQ_MAX_DESKEW_PER_BIT) && (all_bits_fail == 0)) {
		// Right Failing Point cannot be found
		shim_printf("Memory Wrdq calibration: Max deskew reached, but right failing point not found for ch %d\n", ch);

		// Assume failure point is max dqs deskew setting
		for (byte = 0; byte < DQ_NUM_BYTES; byte++) {
			if (bits_fail_b[byte] == 0)
				start_b[byte] = DQ_MAX_DESKEW_PER_BIT;
		}

		// continue to passing point
		wrdqcal_find_right_passing_point(ch, rnk, start_b);
	}
	
	// Before quitting restore the cawrlvlsdll and per byte deskew back to original values.
	// can use dlysel (with phyupdt and forceckelow)
	amp_set_cawrlvl_sdll(ch, cawrlvlcode, true);
	for (byte = 0; byte < DQ_NUM_BYTES; byte++) {
		dq_byte = byte;
		dq = map_byte_to_dq(&dq_byte);
		CSR_WRITE(rAMP_WRDMDESKEW_CTRL(dq, ch, dq_byte), 0);
	}
}

// Keep decreasing per byte deskew until right passing point is found
static void wrdqcal_find_right_passing_point(uint32_t ch, uint32_t rnk, int32_t *start_b)
{
	uint32_t chrnk_indx;
	bool switch_from_dqstodq, max_tap_value_reached;
	int32_t tap_value_b[DQ_NUM_BYTES];
	uint32_t BitPass[DQ_TOTAL_BITS] = { 0 };
	uint32_t SolidBitPass[DQ_TOTAL_BITS] = { 0 };
	uint32_t wrdqcalresult, wrdqcalresult_swiz;
	uint32_t all_bits_pass;
	uint32_t all_bits_pass_b[DQ_NUM_BYTES] = { 0 };
	uint32_t bit_indx, byte;
	int32_t saved_val, max_value, dqmdllcode;
	uint32_t cawrlvlcode = CSR_READ(rAMP_CAWRLVLSDLLCODE(ch));
	uint32_t dq, dq_byte;
	int32_t max_tap_value[AMP_DQ_PER_CHAN];
	int32_t sdll_limit_b[DQ_NUM_BYTES], deskew_taps_b[DQ_NUM_BYTES];
	
	for (byte = 0; byte < DQ_NUM_BYTES; byte++) {
		dq_byte = byte;
		dq = map_byte_to_dq(&dq_byte);
		sdll_limit_b[byte] = mdllcode[ch][dq] - DELIMIT_POS_ADJ_WRDQSDLL;
	}
	
	max_tap_value[AMP_DQ0] = mdllcode[ch][AMP_DQ0] - DELIMIT_POS_ADJ_WRDQSDLL + offset_convert(DQ_MAX_DESKEW_PER_BIT, DIRECTION_DESKEW_TO_SDLL);
	max_tap_value[AMP_DQ1] = mdllcode[ch][AMP_DQ1] - DELIMIT_POS_ADJ_WRDQSDLL + offset_convert(DQ_MAX_DESKEW_PER_BIT, DIRECTION_DESKEW_TO_SDLL);
	
	chrnk_indx = (ch * cfg_params.num_ranks) + rnk;
	all_bits_pass = 0;
	switch_from_dqstodq = false;
	max_tap_value_reached = false;
	
	max_value = start_b[0];
	// initialize tap_values to max of all bytes' start values
	for (byte = 1; byte < DQ_NUM_BYTES; byte++)
		max_value = (start_b[byte] > max_value) ? start_b[byte] : max_value;
	
	for (byte = 0; byte < DQ_NUM_BYTES; byte++) {
		tap_value_b[byte] = max_value;
	}

	// Any change in DM_DESKEW registers will require an equal change to cawrlvl sdll
	do {
		if (switch_from_dqstodq == false) {
			// cawrlvlcode is decremented with tap_value_b each time
			// can use dlysel (with phyupdt and forceckelow)
			amp_set_cawrlvl_sdll(ch, cawrlvlcode, true);
			
			// Keep pushing per bit DQS out until all bytes start to fail
			for (byte = 0; byte < DQ_NUM_BYTES; byte++) {
				dq_byte = byte;
				dq = map_byte_to_dq(&dq_byte);
				CSR_WRITE(rAMP_WRDMDESKEW_CTRL(dq, ch, dq_byte), tap_value_b[byte]);
			}
		} else {
			// adjust wrdq sdll until all bits pass for each byte lane
			for (byte = 0; byte < DQ_NUM_BYTES; byte++) {
				if (all_bits_pass_b[byte] == 0)
					amp_set_wrdq_sdll(ch, byte, tap_value_b[byte]);
			}
		}
		
		// Send the PRBS patterns and read them back to see which bits are passing or failing
		wrdqcalresult = dqcal_wr_rd_pattern_result(ch, rnk, tap_value_b[0] + tap_value_b[1] + tap_value_b[2] + tap_value_b[3]);
		wrdqcalresult_swiz = dqcal_handle_swizzle(wrdqcalresult, true);
				
		// Make sure that each Bit sees a transition from 0 (fail) to 1 (pass) on wrdqcalresult_swiz
		for (bit_indx = 0; bit_indx < DQ_TOTAL_BITS; bit_indx++) {
			
			byte = bit_indx >> 3; // bit_indx / DQ_NUM_BITS_PER_BYTE
			
			// Check if this bit passed during the calibration (not necessarily for first time)
			if ((BitPass[bit_indx] == 0) && ((wrdqcalresult_swiz & (1 << bit_indx)) != 0)) {
				// Has this bit passed SOLID_PASS_DETECT number of times? Then consider it done
				if (SolidBitPass[bit_indx] == SOLID_PASS_DETECT) {
					BitPass[bit_indx] = 1;
				} else if (SolidBitPass[bit_indx] > 0) {
					SolidBitPass[bit_indx] = SolidBitPass[bit_indx] + 1;
				} else {
					// bit passed for the first time, record this value in the global array as the right edge
					SolidBitPass[bit_indx] = SolidBitPass[bit_indx] + 1;
					
					byte = bit_indx >> 3; // bit_indx / DQ_NUM_BITS_PER_BYTE
					
					if (switch_from_dqstodq == false) {
						dq_byte = byte;
						dq = map_byte_to_dq(&dq_byte);
						dqmdllcode = mdllcode[ch][dq];
						
						// consider mdllcode as '0' since sdll is set to -mdllcode
						// saved value is in units of sdll steps
						saved_val = -1 * (offset_convert(tap_value_b[byte], DIRECTION_DESKEW_TO_SDLL) + dqmdllcode);
					} else {
						// saved value is in units of sdll steps
						if (tap_value_b[byte] >= sdll_limit_b[byte])
							saved_val = sdll_limit_b[byte] + offset_convert(deskew_taps_b[byte], DIRECTION_DESKEW_TO_SDLL);
						else
							saved_val = tap_value_b[byte];
					}
					
					wrdqcal_per_chrnk_right[chrnk_indx][bit_indx] = saved_val;
				}
			} else {
				// bit failed calibration, reset the pass count to 0
				SolidBitPass[bit_indx] = 0;
			}
		}
		
		all_bits_pass = 1;
		for (byte = 0; byte < DQ_NUM_BYTES; byte++)
			all_bits_pass_b[byte] = 1;
		
		for (bit_indx = 0; bit_indx < DQ_TOTAL_BITS; bit_indx++) {
			byte = bit_indx >> 3; // bit_indx / DQ_NUM_BITS_PER_BYTE
			
			// Did all the bits pass (SOLID_PASS_DETECT number of times) in this byte lane?
			// If anyone of the bits failed, then the byte flag is cleared
			all_bits_pass_b[byte] = all_bits_pass_b[byte] & BitPass[bit_indx];
			
			// Did all bits in all byte lanes pass?
			all_bits_pass = all_bits_pass & BitPass[bit_indx];
		}

		// If ALL bits are not passing - keep moving from Right to Left Side of window (by adding less negative adjustment to mdll)
		if (all_bits_pass == 0) {
			// Even if one of the byte lanes arrives early to tap_value = 0. Remain here until all byte lane catch up before proceeding to pushing out dq
						
			// if the tap_value for all bytes has reached 0 on the deskew, make the transition to SDLL
			if (switch_from_dqstodq == false) {
				
				// check for all bytes reaching 0 on the tap value (could be deskew or sdll)
				uint32_t all_bytes_tap = tap_value_b[0];
				for (byte = 1; (byte < DQ_NUM_BYTES) && (all_bytes_tap == 0); byte++) {
					all_bytes_tap += tap_value_b[byte];
				}

				if (all_bytes_tap == 0) {
					switch_from_dqstodq = true;
				
					for (byte = 0; byte < DQ_NUM_BYTES; byte++) {
						dq_byte = byte;
						dq = map_byte_to_dq(&dq_byte);
						tap_value_b[byte] = OFFSET_TO_INT(CSR_READ(rAMP_DQSDLLCTRL_WR(dq, ch, dq_byte)) & DLLVAL_BITS);
					}
				}
				
			}
			
			// To find right side passing point, add less negative adjustment to mdll (same as decrementing deskew)
			
			// For deskew taps, we just decrement by FINER_STEP_SZ if we haven't reached 0 yet
			// Note: All deskew taps will reach 0 at the same time since their start values are equal, and they are decremented regardless of pass or fail
			if (switch_from_dqstodq == false) {
				
				// Also decrement cawrlvlsdllcode along with tap_value_b
				if (tap_value_b[0] > 0)
					cawrlvlcode -= FINER_STEP_SZ;
				
				for (byte = 0; byte < DQ_NUM_BYTES; byte++)
					if (tap_value_b[byte] > 0)
						tap_value_b[byte] -= FINER_STEP_SZ;
			} else {
				// For sdll taps, increment
				for (byte = 0; byte < DQ_NUM_BYTES; byte++) {
					if (all_bits_pass_b[byte] == 0) {
						tap_value_b[byte] += FINER_STEP_SZ;
						
						if (tap_value_b[byte] >= sdll_limit_b[byte])
							deskew_taps_b[byte] = offset_convert(tap_value_b[byte] - sdll_limit_b[byte], DIRECTION_SDLL_TO_DESKEW);
					}
				}
			}
		}

		// trigger for loop to end if any of the bytes reach max tap value
		for (byte = 0; byte < DQ_NUM_BYTES; byte++) {
			dq_byte = byte;
			dq = map_byte_to_dq(&dq_byte);
			
			if (!max_tap_value_reached)
				max_tap_value_reached = (tap_value_b[byte] > max_tap_value[dq]);
			
			if (max_tap_value_reached) {
				if (all_bits_pass == 0)
					shim_panic("Memory wrdq calibration: Unable to find right side passing point, max tap value reached. ch %d, byte %d, start_b[] = {0x%x 0x%x 0x%x 0x%x}", ch, byte, start_b[0], start_b[1], start_b[2], start_b[3]);
			}
		}
		
	} while ((!max_tap_value_reached) && (all_bits_pass == 0));
}

// To find left failing point, keep adding less negative adjustment to mdll
static void wrdqcal_find_left_failing_point(uint32_t ch, uint32_t rnk)
{
	int32_t wrdqsdll[DQ_NUM_BYTES];
	uint32_t wrdqcalresult, wrdqcalresult_swiz;
	uint32_t all_bits_fail;
	uint32_t all_bits_fail_b[DQ_NUM_BYTES] = { 0 };
	uint32_t mask_b[DQ_NUM_BYTES];
	int32_t start_b[DQ_NUM_BYTES];
	uint32_t byte, dq, dq_byte;
	bool max_tap_value_reached = false;
	int32_t max_tap_value[AMP_DQ_PER_CHAN];
	int32_t dq_deskew;
	
	max_tap_value[AMP_DQ0] = mdllcode[ch][AMP_DQ0] - DELIMIT_POS_ADJ_WRDQSDLL + offset_convert(DQ_MAX_DESKEW_PER_BIT, DIRECTION_DESKEW_TO_SDLL);
	max_tap_value[AMP_DQ1] = mdllcode[ch][AMP_DQ1] - DELIMIT_POS_ADJ_WRDQSDLL + offset_convert(DQ_MAX_DESKEW_PER_BIT, DIRECTION_DESKEW_TO_SDLL);

	all_bits_fail = 0;
	wrdqcalresult = 0xFFFFFFFF;
	
	for (byte = 0; byte < DQ_NUM_BYTES; byte++) {
		// initialize the mask for each byte lane
		mask_b[byte] = 0xFF << (byte * 8);
		
		dq_byte = byte;
		// <rdar://problem/13907252>
		dq = map_byte_to_dq(&dq_byte);
		
		// Get the starting values for WR DQS SDLL
		wrdqsdll[byte] = OFFSET_TO_INT(CSR_READ(rAMP_DQSDLLCTRL_WR(dq, ch, dq_byte)) & DLLVAL_BITS);
		
		// Add per-bit deskew to wrdqsdll[byte] if sdll reached mdll - DELIMIT_POS_ADJ_WRDQSDLL (otherwise, deskew should be 0)
		// At this point per-bit deskew should be the same for each bit in this byte. Use bit 0's deskew value
		dq_deskew = CSR_READ(rAMP_WRDQDESKEW_CTRL(dq, ch, dq_byte, 0));
		// convert deskew taps to sdll taps
		dq_deskew = offset_convert(dq_deskew, DIRECTION_DESKEW_TO_SDLL);
		wrdqsdll[byte] += dq_deskew;
	}
	
	// Start with sdll value for which right passing point was found, then increase (less negative) until all bits fail
	do {
		
		for (byte = 0; byte < DQ_NUM_BYTES; byte++) {
			// set the new sdll for this byte lane if all bits are not yet failing
			if (all_bits_fail_b[byte] == 0)
				amp_set_wrdq_sdll(ch, byte, wrdqsdll[byte]);
		}
		
		// Send the PRBS patterns and read them back to see which bits are passing or failing
		wrdqcalresult &= dqcal_wr_rd_pattern_result(ch, rnk, wrdqsdll[0] + wrdqsdll[1] + wrdqsdll[2] + wrdqsdll[3]);
		wrdqcalresult_swiz = dqcal_handle_swizzle(wrdqcalresult, true);
		
		// If the result of all bits in this byte show a fail, record this as the failing point
		all_bits_fail = 1;
		for (byte = 0; byte < DQ_NUM_BYTES; byte++) {
			if ((all_bits_fail_b[byte] == 0) && ((wrdqcalresult_swiz & mask_b[byte]) == 0)) {
				all_bits_fail_b[byte] = 1;
				start_b[byte] = wrdqsdll[byte];
			}
			
			all_bits_fail &= all_bits_fail_b[byte];
		}
		
		if (all_bits_fail == 1) {
			wrdqcal_find_left_passing_point (ch, rnk, start_b);
		} else {
			// if the byte has not yet failed, find the next sdll value to be set
			for (byte = 0; byte < DQ_NUM_BYTES; byte++) {
				if (all_bits_fail_b[byte] == 0) {
					dq_byte = byte;
					dq = map_byte_to_dq(&dq_byte);
					if ((wrdqsdll[byte] + COARSE_STEP_SZ) <=  max_tap_value[dq])
						wrdqsdll[byte] += COARSE_STEP_SZ;
					else
						wrdqsdll[byte] += FINER_STEP_SZ;
				}
			}
		}
		
		for (byte = 0; byte < DQ_NUM_BYTES; byte++) {
			dq_byte = byte;
			dq = map_byte_to_dq(&dq_byte);
			
			// none of the previous bytes reached max_tap_value, then update the boolean
			if (!max_tap_value_reached) {
				max_tap_value_reached = (wrdqsdll[byte] > max_tap_value[dq]);
				
				if (max_tap_value_reached) {
					shim_printf("Memory wrdq calibration: Unable to find left failing point, max tap value reached for ch %d byte %d\n", ch, byte);
					break;
				}
			}
		}
		
		if (max_tap_value_reached) {
			// Continue to passing point if any of the bytes reaches max value and not all bits are failing
			if (all_bits_fail == 0) {
				for (byte = 0; byte < DQ_NUM_BYTES; byte++) {
					dq_byte = byte;
					dq = map_byte_to_dq(&dq_byte);
					
					if (all_bits_fail_b[byte] == 0)
						start_b[byte] = max_tap_value[dq];
				}
				
				wrdqcal_find_left_passing_point(ch, rnk, start_b);
			}
		}
	} while ((!max_tap_value_reached) && (all_bits_fail == 0));
}

static void wrdqcal_find_left_passing_point(uint32_t ch, uint32_t rnk, int32_t *start_b)
{
	uint32_t chrnk_indx;
	bool max_tap_value_reached = false;
	int32_t tap_value_b[DQ_NUM_BYTES];
	uint32_t BitPass[DQ_TOTAL_BITS] = { 0 };
	uint32_t SolidBitPass[DQ_TOTAL_BITS] = { 0 };
	uint32_t wrdqcalresult, wrdqcalresult_swiz;
	uint32_t all_bits_pass;
	uint32_t all_bits_pass_b[DQ_NUM_BYTES] = { 0 };
	uint32_t bit_indx, byte;
	uint32_t dq, dq_byte;
	int32_t max_tap_value[AMP_DQ_PER_CHAN];
	int32_t sdll_limit_b[DQ_NUM_BYTES], deskew_taps_b[DQ_NUM_BYTES];
	
	max_tap_value[AMP_DQ0] = -1 * mdllcode[ch][AMP_DQ0];
	max_tap_value[AMP_DQ1] = -1 * mdllcode[ch][AMP_DQ1];
	
	chrnk_indx = (ch * cfg_params.num_ranks) + rnk;
	all_bits_pass = 0;
	wrdqcalresult = 0xFFFFFFFF;
	
	for (byte = 0; byte < DQ_NUM_BYTES; byte++) {
		tap_value_b[byte] = start_b[byte];
		
		dq_byte = byte;
		dq = map_byte_to_dq(&dq_byte);
		sdll_limit_b[byte] = mdllcode[ch][dq] - DELIMIT_POS_ADJ_WRDQSDLL;
		
		if (tap_value_b[byte] >= sdll_limit_b[byte])
			deskew_taps_b[byte] = offset_convert(tap_value_b[byte] - sdll_limit_b[byte], DIRECTION_SDLL_TO_DESKEW);
		else
			deskew_taps_b[byte] = 0;
	}
			
	// Finding Left side failing point on per bit level. Moving Left to Right (keep adding more negative adj to mdll) to find point where it turns from FAIL TO PASS
	do {
		// adjust wrdq sdll until all bits pass for each byte lane
		for (byte = 0; byte < DQ_NUM_BYTES; byte++) {
			if (all_bits_pass_b[byte] == 0)
				amp_set_wrdq_sdll(ch, byte, tap_value_b[byte]);
		}
		
		// Send the PRBS patterns and read them back to see which bits are passing or failing
		wrdqcalresult = dqcal_wr_rd_pattern_result(ch, rnk, tap_value_b[0] + tap_value_b[1] + tap_value_b[2] + tap_value_b[3]);
		wrdqcalresult_swiz = dqcal_handle_swizzle(wrdqcalresult, true);
		
		// Make sure that each Bit sees a transition from 0 (fail) to 1 (pass) on wrdqcalresult_swiz
		for (bit_indx = 0; bit_indx < DQ_TOTAL_BITS; bit_indx++) {
			// Check if this bit passed during the calibration (not necessarily for first time)
			if ((BitPass[bit_indx] == 0) && ((wrdqcalresult_swiz & (1 << bit_indx)) != 0)) {
				// Has this bit passed SOLID_PASS_DETECT number of times? Then consider it done
				if (SolidBitPass[bit_indx] == SOLID_PASS_DETECT) {
					BitPass[bit_indx] = 1;
				} else if (SolidBitPass[bit_indx] > 0) {
					SolidBitPass[bit_indx] = SolidBitPass[bit_indx] + 1;
				} else {
					// bit passed for the first time, record this value in the global array as the right edge
					SolidBitPass[bit_indx] = SolidBitPass[bit_indx] + 1;
					
					byte = bit_indx >> 3; // bit_indx / DQ_NUM_BITS_PER_BYTE
					
					int32_t saved_val;
					// saved value is in units of sdll steps
					if (tap_value_b[byte] >= sdll_limit_b[byte])
						saved_val = sdll_limit_b[byte] + offset_convert(deskew_taps_b[byte], DIRECTION_DESKEW_TO_SDLL);
					else
						saved_val = tap_value_b[byte];
					
					wrdqcal_per_chrnk_left[chrnk_indx][bit_indx] = saved_val;
				}
			} else {
				// bit failed calibration, reset the pass count to 0
				SolidBitPass[bit_indx] = 0;
			}
		}

		all_bits_pass = 1;
		for (byte = 0; byte < DQ_NUM_BYTES; byte++)
			all_bits_pass_b[byte] = 1;
		
		for (bit_indx = 0; bit_indx < DQ_TOTAL_BITS; bit_indx++) {
			byte = bit_indx >> 3; // bit_indx / DQ_NUM_BITS_PER_BYTE
			
			// Did all the bits pass (SOLID_PASS_DETECT number of times) in this byte lane?
			// If anyone of the bits failed, then the byte flag is cleared
			all_bits_pass_b[byte] = all_bits_pass_b[byte] & BitPass[bit_indx];
			
			// Did all bits in all byte lanes pass?
			all_bits_pass = all_bits_pass & BitPass[bit_indx];
		}
		
		// If ALL bits are not passing - keep moving from Left to Right Side of window (by adding more negative adjustment to mdll)
		if (all_bits_pass == 0) {
			// For sdll taps, decrement it
			for (byte = 0; byte < DQ_NUM_BYTES; byte++) {
				if (all_bits_pass_b[byte] == 0) {
					tap_value_b[byte] -= FINER_STEP_SZ;
					
					if (tap_value_b[byte] >= sdll_limit_b[byte])
						deskew_taps_b[byte] = offset_convert(tap_value_b[byte] - sdll_limit_b[byte], DIRECTION_SDLL_TO_DESKEW);
				}
			}
		}
		
		// check for end of loop condition
		for (byte = 0; byte < DQ_NUM_BYTES; byte++) {
			
			dq_byte = byte;
			dq = map_byte_to_dq(&dq_byte);
			
			if (!max_tap_value_reached)
				max_tap_value_reached = (tap_value_b[byte] < max_tap_value[dq]);
			
			if (max_tap_value_reached) {
				if (all_bits_pass_b[byte] == 0)
					shim_panic("Memory wrdq calibration: Unable to find left passing point, max tap value reached. ch %d, byte %d, start_b[] = {0x%x, 0x%x, 0x%x, 0x%x}", ch, byte, start_b[0], start_b[1], start_b[2], start_b[3]);
				break;
			}
		}
	} while ((!max_tap_value_reached) && (all_bits_pass == 0));
}

static void wrdqcal_program_final_values(void)
{
	uint32_t ch, bit_indx, byte;
	uint32_t chrnk0_indx, chrnk1_indx;
	int32_t dqmdllcode;
	int32_t wrdq_bit_center[DQ_TOTAL_BITS];
	int32_t wrdq_bit_deskew[DQ_TOTAL_BITS];
	int32_t left_pos_val;
	int32_t right_pos_val;
	int32_t min_wrdq_center[DQ_NUM_BYTES];
	int32_t min_dq_deskew_code, max_dq_deskew_code;
	uint32_t dq, dq_byte;
	
	for (ch = 0; ch < cfg_params.num_channels; ch++) {
				
		// find the center point of passing window for each bit over all ranks
		for (bit_indx = 0; bit_indx < DQ_TOTAL_BITS; bit_indx++) {
			
			chrnk0_indx = (ch * cfg_params.num_ranks) + 0;
			left_pos_val = wrdqcal_per_chrnk_left[chrnk0_indx][bit_indx];
			right_pos_val = wrdqcal_per_chrnk_right[chrnk0_indx][bit_indx];
			
			if (cfg_params.num_ranks > 1) {
				
				chrnk1_indx = (ch * cfg_params.num_ranks) + 1;
				
				// find the endpoint that covers both ranks
				left_pos_val = find_common_endpoint(wrdqcal_per_chrnk_left[chrnk0_indx][bit_indx],
								    wrdqcal_per_chrnk_left[chrnk1_indx][bit_indx],
								    MIN_ENDPT);
				right_pos_val = find_common_endpoint(wrdqcal_per_chrnk_right[chrnk0_indx][bit_indx],
								     wrdqcal_per_chrnk_right[chrnk1_indx][bit_indx],
								     MAX_ENDPT);
			}
			
			// find center of the eye for this bit
			wrdq_bit_center[bit_indx] = find_center_of_eye(left_pos_val, right_pos_val);
		}
		
		// initialize the min centerpoint to the 1st bit's center point in each byte lane
		for (byte = 0; byte < DQ_NUM_BYTES; byte++)
			min_wrdq_center[byte] = wrdq_bit_center[byte * DQ_NUM_BITS_PER_BYTE];
		
		// Find the min CenterPoint per byte lane given each bit's center point
		for (bit_indx=0; bit_indx < DQ_TOTAL_BITS; bit_indx++) {
			
			byte = bit_indx >> 3; // bit_indx / DQ_NUM_BITS_PER_BYTE
			
			// if this bit's center point is less than current min, make it the new min
			if (wrdq_bit_center[bit_indx] < min_wrdq_center[byte])
				min_wrdq_center[byte] = wrdq_bit_center[bit_indx];
		}
				
		// for positive value, clamp it to mdllcode - DELIMIT_POS_ADJ_WRDQSDLL
		for (byte = 0; byte < DQ_NUM_BYTES; byte++) {
			dq_byte = byte;
			dq = map_byte_to_dq(&dq_byte);
			dqmdllcode = mdllcode[ch][dq];
			
			if (min_wrdq_center[byte] > (dqmdllcode - DELIMIT_POS_ADJ_WRDQSDLL))
				min_wrdq_center[byte] = (dqmdllcode - DELIMIT_POS_ADJ_WRDQSDLL);
		}

		// Compute the individual deskew values: any bits with center point > min for its byte lane will require deskew
		// Each bit's center is guaranteed to be >= min for its byte lane
		// Deskewing means adding more positive adjustment for this bit in addition to the sdll
		for (bit_indx = 0; bit_indx < DQ_TOTAL_BITS; bit_indx++) {
			
			byte = bit_indx >> 3; // bit_indx / DQ_NUM_BITS_PER_BYTE
			
			dq_byte = byte;
			dq = map_byte_to_dq(&dq_byte);
			dqmdllcode = mdllcode[ch][dq];

			if (wrdq_bit_center[bit_indx] < min_wrdq_center[byte])
				shim_panic("Memory Wrdq Calibration: wrdq_bit_center[%d] = (%d) < min_wrdq_center[%d] = %d\n", bit_indx, wrdq_bit_center[bit_indx], byte, min_wrdq_center[byte]);
			
			wrdq_bit_deskew[bit_indx] = wrdq_bit_center[bit_indx] - min_wrdq_center[byte];

			// Need to convert deskew values, which have been in the sdll scale so far, into the deskew scale
			wrdq_bit_deskew[bit_indx] = offset_convert(wrdq_bit_deskew[bit_indx], DIRECTION_SDLL_TO_DESKEW);
			
			// Make sure deskew value programmed is not negative and is <= DQ_MAX_DESKEW_PER_BIT
			if ((wrdq_bit_deskew[bit_indx] < 0) || (wrdq_bit_deskew[bit_indx] > DQ_MAX_DESKEW_PER_BIT))
				shim_panic("Memory Wrdq Calibration: wrdq_bit_deskew[%d] = %d invalid\n", bit_indx, wrdq_bit_deskew[bit_indx]);
		}

		// if the min for each byte lane is < -dqmdllcode, we'll need to adjust/clamp it to -dqmdllcode
		for (byte = 0; byte < DQ_NUM_BYTES; byte++) {
			
			dq_byte = byte;
			dq = map_byte_to_dq(&dq_byte);
			dqmdllcode = mdllcode[ch][dq];

			if (min_wrdq_center[byte] < (-1 * dqmdllcode)) {
				int32_t dqs_deskew = (-1 * dqmdllcode) - min_wrdq_center[byte];
				// need to convert from sdll scale to deskew scale
				dqs_deskew = offset_convert(dqs_deskew, DIRECTION_SDLL_TO_DESKEW);
				// <rdar://problem/14116888> put the remainder on DQS
				CSR_WRITE(rAMP_WRDMDESKEW_CTRL(dq, ch, dq_byte), dqs_deskew);
				min_wrdq_center[byte] = (-1 * dqmdllcode);
			}
		}
		
		// Program the SDLL and deskew per bit for each byte lane
		for (byte = 0; byte < DQ_NUM_BYTES; byte++) {
			amp_set_wrdq_sdll(ch, byte, min_wrdq_center[byte]);
			
			dq_byte = byte;
			dq = map_byte_to_dq(&dq_byte);
			
			// init the min and max deskew values for each byte to the 1st bit in the byte
			min_dq_deskew_code = wrdq_bit_deskew[(byte * DQ_NUM_BITS_PER_BYTE)];
			max_dq_deskew_code = wrdq_bit_deskew[(byte * DQ_NUM_BITS_PER_BYTE)];
			// per bit deskew for this byte lane
			for (bit_indx = 0; bit_indx < DQ_NUM_BITS_PER_BYTE; bit_indx++) {
				CSR_WRITE(rAMP_WRDQDESKEW_CTRL(dq, ch, dq_byte, bit_indx), wrdq_bit_deskew[(byte * DQ_NUM_BITS_PER_BYTE) + bit_indx]);
				
				// is this bit the new min or max?
				if (wrdq_bit_deskew[(byte * DQ_NUM_BITS_PER_BYTE) + bit_indx] < min_dq_deskew_code)
					min_dq_deskew_code = wrdq_bit_deskew[(byte * DQ_NUM_BITS_PER_BYTE) + bit_indx];
				else if (wrdq_bit_deskew[(byte * DQ_NUM_BITS_PER_BYTE) + bit_indx] > max_dq_deskew_code)
					max_dq_deskew_code = wrdq_bit_deskew[(byte * DQ_NUM_BITS_PER_BYTE) + bit_indx];
			}
			
			// find midpoint of deskew registers for this byte, and program it to DM (controlled by DQS regs, oddly)
			CSR_WRITE(rAMP_WRDQSDESKEW_CTRL(dq, ch, dq_byte), (min_dq_deskew_code + max_dq_deskew_code) >> 1);
		}
	} // for (ch = 0; ch < cfg_params.num_channels; ch++)
}

// This function writes PRBS7 patterns to dram for given channel and rank,
// and reads them back. Read back values are compared with data that was written
static uint32_t dqcal_wr_rd_pattern_result(uint32_t ch, uint32_t rnk, uint32_t sdll_value)
{
	uint32_t chrnk_indx, result, result_per_wr_and_rdbk;
	uint32_t pattern_indx, pattern, readback_data;
	uint32_t col, word;
	uint64_t mem_region;
	uint32_t all_bits = 0xFFFFFFFF;
	uint32_t consecutive_cols_per_chrnk = (shim_get_consecutive_bytes_perchnrnk() / DQ_BYTES_PER_COL);

	result = all_bits;
	result_per_wr_and_rdbk = all_bits;
	
	chrnk_indx = (ch * cfg_params.num_ranks) + rnk;
	pattern_indx = sdll_value & DLLVAL_BITS; // sdll tap indexes into pattern array
	
	// <rdar://problem/14017861> need APB read inserted in function wr_rd_pattern_result
	CSR_READ(rAMP_RDFIFOPTRSTS(AMP_DQ0, ch));
	
	// write the patterns to memory 4 bytes at a time
	// interleaving applies x bytes (obtained by shim_get_consecutive_bytes_perchnrnk), so recompute the address at that point
	// Note that bank and row are fixed
	for (col = 0; col < DQ_NUM_PATTERNS; col += consecutive_cols_per_chrnk) {
		
		mem_region = shim_compute_dram_addr(ch, rnk, DQ_BANK, DQ_ROW, col);
		
		// next 64 words (or columns, for H7 this is 256 bytes) are consecutively stored in a [channel,rank] combo
		for (word = 0; word < consecutive_cols_per_chrnk; word++) {
			
			// last pattern in array is dummy value, so skip it
			pattern = DQ_PRBS7_PATTERNS[pattern_indx % (DQ_NUM_PATTERNS - 1)];
			pattern_indx++;
			
			// <rdar://problem/13899661>
			// write the pattern: 3rd argument for DV requires +1, others should subtract 1 before adding to mem_region
			MEM_WRITE(mem_region, pattern, (word+1));
		}
	}
	
	pattern_indx = sdll_value & DLLVAL_BITS;
	
	// Now, read back the patterns (have to do it in a separate loop than the writes to get more robust calib values)
	for (col = 0; col < DQ_NUM_PATTERNS; col += consecutive_cols_per_chrnk) {
		
		mem_region = shim_compute_dram_addr(ch, rnk, DQ_BANK, DQ_ROW, col);
		
		// next 16 words (or columns) are consecutively stored in a [channel,rank] combo
		for (word = 0; word < consecutive_cols_per_chrnk; word++) {
			
			// last pattern in array is dummy value, so skip it
			pattern = DQ_PRBS7_PATTERNS[pattern_indx % (DQ_NUM_PATTERNS - 1)];
			pattern_indx++;
			
			// <rdar://problem/13899661>
			// read the pattern: 2nd argument for DV requires +1, others should subtract 1 before adding to mem_region
			readback_data = MEM_READ(mem_region, (word+1));
			
			// records if read back value was different than written value by
			// clearing bits that are different in the final result
			result_per_wr_and_rdbk &= ~(readback_data ^ pattern);
		}
		
		// result variable accumulates the results of all pattern matching results for a given sdll_value
		result &= result_per_wr_and_rdbk;
	}
	
	// failing bits are clear, passing bits are set
	return result;
}

// The change that switched to signed integer format from sign/magnitude (<rdar://problem/14649655>)
// has rendered this function unused. But leaving the code in for posterity's sake
#if 0
// Given an input where bit SIGN_BIT_POS represents the sign, and the rest is magnitude
// separate out the sign and magnitude and return those values to the caller
static void get_offset_sign_magnitude(uint32_t offset, uint32_t *neg_bit_set, uint32_t *tap_val)
{
	*neg_bit_set = (offset & (1 << SIGN_BIT_POS)) >> SIGN_BIT_POS;
	*tap_val = offset - (*neg_bit_set << SIGN_BIT_POS);
}
#endif

static uint32_t map_byte_to_dq(uint32_t *byte)
{
	if (*byte < DQ_NUM_BYTES_PER_DQ)
		return AMP_DQ0;
	else {
		*byte -= DQ_NUM_BYTES_PER_DQ;
		return AMP_DQ1;
	}
}

static int32_t find_center_of_eye(int32_t left_pos_val, int32_t right_pos_val)
{
	if (left_pos_val < right_pos_val)
		shim_panic("Memory calibration: find_center_of_eye: Left value (0x%x) is < right value (0x%x)", left_pos_val, right_pos_val);
	
	// center of 2 signed integers is simply their average
	return ((left_pos_val + right_pos_val) / 2);
}

// Select the value that would include the other value in the eye
static int32_t find_common_endpoint(int32_t val0, int32_t val1, uint32_t min_or_max)
{
	int32_t retVal = val0;
	
	// For the right endpoint, select the rightmost value on the number line (max value)
	if (min_or_max == MAX_ENDPT) {
		retVal = (val0 > val1) ? val0 : val1;
	}
	// For the left endpoint, select the leftmost value (min value)
	else {
		retVal = (val0 < val1) ? val0 : val1;
	}
	
	return retVal;
}

// Starting with Fiji B0, deskew steps are 22% bigger than sdll steps. This function helps convert between the 2 units
static int32_t offset_convert(int32_t offset, uint32_t direction) {
	
	if ((cfg_params.sdll_scale == 0) || (cfg_params.deskew_scale == 0))
		shim_panic("Memory calibration: Zero value for sdll (%d) or deskew (%d) scaling factor", cfg_params.sdll_scale, cfg_params.deskew_scale);
	
	if (direction == DIRECTION_DESKEW_TO_SDLL)
		return ((offset * cfg_params.sdll_scale + (cfg_params.deskew_scale >> 1)) / cfg_params.deskew_scale);
	else
		return ((offset * cfg_params.deskew_scale + (cfg_params.sdll_scale >> 1)) / cfg_params.sdll_scale);
}
