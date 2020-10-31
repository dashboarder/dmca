/*
 * Copyright (C) 2011-2014 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */

#include <debug.h>
#include <sys/menu.h>
#include <drivers/amc/amc.h>
#include <drivers/amc/amc_phy.h>
#include <drivers/amc/amc_regs.h>
#include <drivers/amp/amp_v2.h>
#include <drivers/amp/amp_v2_calibration.h>
#include <drivers/dram.h>
#include <drivers/miu.h>
#include <drivers/power.h>
#include <platform.h>
#include <sys.h>

// PRBS7 patterns used for ca calibration. amc_phy_params.cacalib_sw_loops * amc_phy_params.cacalib_hw_loops must equal 64
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


static uint32_t ca_patterns_mask[CA_NUM_PATTERNS];
static int32_t mdllcode[AMC_NUM_CHANNELS][2];		// 1 for AMP_DQ and 1 for AMP_CA

// Used to save calibration values for each bit per channel and rank for every iteration
static int32_t ca_cal_per_loopchrnk_right[CA_MAX_LOOP_CHN_RNK][CA_NUM_BITS];
static int32_t ca_cal_per_loopchrnk_left[CA_MAX_LOOP_CHN_RNK][CA_NUM_BITS];

// ca data aggregated over all iterations
static int32_t ca_cal_per_chrnk_right[AMC_NUM_CHANNELS * AMC_NUM_RANKS][CA_NUM_BITS];
static int32_t ca_cal_per_chrnk_left[AMC_NUM_CHANNELS * AMC_NUM_RANKS][CA_NUM_BITS];

// rddq data aggregated over all iterations
static int32_t rddq_cal_per_chrnk_right[AMC_NUM_CHANNELS * AMC_NUM_RANKS][DQ_TOTAL_BITS];
static int32_t rddq_cal_per_chrnk_left[AMC_NUM_CHANNELS * AMC_NUM_RANKS][DQ_TOTAL_BITS];

// wrdq data aggregated over all iterations
static int32_t wrdq_cal_per_chrnk_right[AMC_NUM_CHANNELS * AMC_NUM_RANKS][DQ_TOTAL_BITS];
static int32_t wrdq_cal_per_chrnk_left[AMC_NUM_CHANNELS * AMC_NUM_RANKS][DQ_TOTAL_BITS];

// wrlvl data aggregated over all iterations, we save value for 4 byte lanes + the ca value
static int32_t wrlvl_cal_per_chrnk_rise[AMC_NUM_CHANNELS * AMC_NUM_RANKS][DQ_NUM_BYTES + 1];
static int32_t wrlvl_cal_per_chrnk_fall[AMC_NUM_CHANNELS * AMC_NUM_RANKS][DQ_NUM_BYTES + 1];

// This array will hold the contents of memory that will be used for dq calibration
static uint8_t dqcal_saved_data[AMC_NUM_RANKS][sizeof(DQ_PRBS7_PATTERNS) * AMC_NUM_CHANNELS]__aligned(32);

// This array will hold the calibration values to be saved to the PMU for resume boot
static uint8_t cal_pmu_bits[CALIB_PMU_BYTES] = { 0 };

// Static local function declarations
static void calibrate_ca(void);
static void calibrate_rddq(bool after_wrddqcal);
static void calibrate_wrlvl(void);
static void calibrate_wrdq(void);
static void save_masterdll_values(void);
static void generate_ca_patterns_mask(void);
static void amp_program_ca_patterns(uint32_t ch, uint32_t rnk, uint32_t swloop);
static void amp_init_ca_offset_and_deskew(uint32_t ch);
static uint32_t amp_mask_ca_bits(uint32_t ch, uint32_t mr_cmd);
static void amp_push_casdll_out(uint32_t ch, int32_t offset);
static void amp_enable_cacal_mode(bool enable, uint32_t ch);
static void amp_run_cacal(uint32_t ch);
static void amp_push_ctl_out(uint32_t ch, uint32_t dly_val);
static void amp_setup_rddq_cal(uint32_t ch, uint32_t rnk);
static void amp_set_rddq_sdll(uint32_t ch, uint32_t byte, uint32_t offset);
static void amp_run_rddqcal(uint32_t ch);
static void amp_wrlvl_init(void);
static void amp_phy_update(uint32_t ch, uint32_t update);
static void amp_set_cawrlvl_sdll(uint32_t ch, uint32_t offset, bool set_dly_sel);
static void amp_set_dqwrlvl_sdll(uint32_t ch, uint32_t byte, uint32_t offset, bool set_dly_sel);
static void amp_run_wrlvlcal(uint32_t ch, uint32_t wrlvlrun);
static void amp_set_wrdq_sdll(uint32_t ch, uint32_t byte, int32_t offset);
static void run_cacal_sequence(uint32_t ch, uint32_t rnk, uint32_t mr_cmd, uint32_t mask_bits, uint32_t swloop);
static void find_cacal_right_side_failing_point(uint32_t ch, uint32_t rnk, uint32_t mr_cmd, uint32_t combined_mask, uint32_t swloop);
static void find_cacal_right_side_passing_point(uint32_t ch, uint32_t rnk, uint32_t mr_cmd, uint32_t combined_mask, uint32_t swloop);
static void enter_cacal_mode(uint32_t ch, uint32_t rnk, uint32_t mr_cmd, bool enter);
static void find_cacal_left_side_failing_point(uint32_t ch, uint32_t rnk, uint32_t combined_mask, uint32_t swloop);
static void find_cacal_left_side_passing_point(uint32_t ch, uint32_t rnk, uint32_t combined_mask, uint32_t swloop);
static void ca_program_final_values(void);
static void find_rddqcal_right_side_failing_point(uint32_t ch, uint32_t rnk, bool after_wrddqcal);
static void find_rddqcal_right_side_passing_point(uint32_t ch, uint32_t rnk, int32_t *start_b, bool after_wrddqcal);
static void find_rddqcal_left_side_failing_point(uint32_t ch, uint32_t rnk, bool after_wrddqcal);
static void find_rddqcal_left_side_passing_point(uint32_t ch, uint32_t rnk, int32_t *start_b, bool after_wrddqcal);
static void rddq_program_final_values(bool after_wrddqcal);
static uint32_t wrlvl_encode_dlyval(uint32_t ch, uint32_t phy_type, uint32_t val);
static uint32_t wrlvl_encode_clk90dly(uint32_t ch, uint32_t val);
static void push_wrlvl_to_0s_region(uint32_t ch, uint32_t rnk);
static void find_wrlvl_0to1_transition(uint32_t ch, uint32_t rnk);
static void find_wrlvl_1to0_transition(uint32_t ch, uint32_t rnk);
static void wrlvl_program_final_values(void);
static void find_wrdqcal_right_side_failing_point(uint32_t ch, uint32_t rnk);
static void find_wrdqcal_right_side_passing_point(uint32_t ch, uint32_t rnk, int32_t *start_b);
static void find_wrdqcal_left_side_failing_point(uint32_t ch, uint32_t rnk);
static void find_wrdqcal_left_side_passing_point(uint32_t ch, uint32_t rnk, int32_t *start_b);
static void wrdq_program_final_values(void);
static uint32_t wr_rd_pattern_result(uint32_t ch, uint32_t rnk, uint32_t sdll_value);
static void save_restore_ca_wrlvl_regs(uint32_t save_or_restore);
static void save_restore_memory_region(bool dqcal_start);
static int32_t find_center_of_eye(int32_t left_pos_val, int32_t right_pos_val);
static int32_t find_common_endpoint(int32_t val0, int32_t val1, uint32_t min_or_max);

///////////////////////////////////////////////////////////////////////////////
////// Global functions
///////////////////////////////////////////////////////////////////////////////

#ifdef AMP_SWIZZLE
void amp_swizzle_init(void)
{
#if (AMP_SWIZZLE == AMP_SWIZZLE_PER_J34M)	// per <rdar://15498696>
  int ch;

  for (ch = 0; ch < AMC_NUM_CHANNELS; ch++) {
    // Set up the CA Byte Select Mapping for both DQ and CA
    rAMP_CACALBYTESEL(AMP_DQ,ch) = 0x00000001;
    rAMP_CACALBYTESEL(AMP_CA,ch) = 0x00000001;

    // Set up the CA Bit  Select Bit Mapping for both DQ and CA
    rAMP_CACALBITSELMAP(AMP_DQ,ch,0) = 0x01234567;
    rAMP_CACALBITSELMAP(AMP_DQ,ch,1) = 0xabcdef67;
    rAMP_CACALBITSELMAP(AMP_DQ,ch,2) = 0x0000ef89;
    rAMP_CACALBITSELMAP(AMP_CA,ch,0) = 0x01234567;
    rAMP_CACALBITSELMAP(AMP_CA,ch,1) = 0xabcdef67;
    rAMP_CACALBITSELMAP(AMP_CA,ch,2) = 0x0000ef89;
  }
#endif // end of (AMP_SWIZZLE == AMP_SWIZZLE_PER_J34M)
}
#endif

void amc_phy_init(bool resume)
{
	uint32_t ch, f;
#if !SUPPORT_FPGA
	uint32_t rd, dq;
#endif

	for (ch = 0; ch < AMC_NUM_CHANNELS; ch++) {
		rAMP_DLLUPDTCTRL(AMP_DQ, ch) = 0x00017307;
		rAMP_DLLUPDTCTRL(AMP_CA, ch) = 0x00017307;
	}
	amc_phy_enable_dqs_pulldown(false);

#ifdef AMP_SWIZZLE
	amp_swizzle_init();
#endif

	for (ch = 0; ch < AMC_NUM_CHANNELS; ch++) {
		rAMP_AMPEN(AMP_DQ, ch) = 1;
		rAMP_AMPEN(AMP_CA, ch) = 1;
		
#if !SUPPORT_FPGA

		rAMP_DQDQSDS(ch) = amc_phy_params.drive_strength;
		rAMP_NONDQDS(ch) = amc_phy_params.drive_strength;
#endif

		for (f = 0; f < AMP_FREQUENCY_SLOTS; f++) {
			rAMP_DIFFMODE_FREQ(ch, f) = 0x00000121;
		}

#if !SUPPORT_FPGA
		for (rd = 0; rd < AMP_MAX_RD; rd++) {
			for (dq = 0; dq < AMP_MAX_DQ; dq++) {
				rAMP_RDDQDESKEW_CTRL(ch, rd, dq) = 0x00000006;
			}
		}
		
		rAMP_DLLLOCKTIM(AMP_DQ, ch) = 0x000d0013;
		rAMP_DLLLOCKTIM(AMP_CA, ch) = 0x000d0013;
#endif

		for (f = 0; f < AMP_FREQUENCY_SLOTS; f++) {
			rAMP_DQSINDLLSCL_FREQ(ch, f) = amc_phy_params.freq[f].dqsindllscl;
			rAMP_CAOUTDLLSCL_FREQ(ch, f) = amc_phy_params.freq[f].caoutdllscl;
		}

		for (f = 0; f < AMP_FREQUENCY_SLOTS; f++) {
			rAMP_RDCAPCFG_FREQ(AMP_DQ, ch, f) = amc_phy_params.freq[f].rdcapcfg;
		}

#if !SUPPORT_FPGA
		rAMP_DLLUPDTCTRL(AMP_DQ, ch) = 0x00017507;
		rAMP_DLLUPDTCTRL(AMP_CA, ch) = 0x00017507;

		if (!amc_phy_params.imp_auto_cal) {
			rAMP_IMPAUTOCAL(AMP_DQ, ch) = 0x000103ac;
			rAMP_IMPAUTOCAL(AMP_CA, ch) = 0x000103ac;
		}
		else {
			rAMP_IMPAUTOCAL(AMP_DQ, ch) = amc_phy_params.imp_auto_cal;
			rAMP_IMPAUTOCAL(AMP_CA, ch) = amc_phy_params.imp_auto_cal;
		}
#endif

#if !SUPPORT_FPGA
		// Keep DLL update interval to 0, it will restored after write calibration.
		// <rdar://problem/15777393> CoreOS: M7 init sequence change
#if SUB_PLATFORM_S7002
		rAMP_DLLUPDTINTVL(AMP_DQ, ch) = 0x10200000;
		rAMP_DLLUPDTINTVL(AMP_CA, ch) = 0x10200000;
#else
		rAMP_DLLUPDTINTVL(AMP_DQ, ch) = 0x1020005a;
		rAMP_DLLUPDTINTVL(AMP_CA, ch) = 0x1020005a;
#endif
		
# ifdef rAMP_MDLLFREQBINDISABLE
		rAMP_MDLLFREQBINDISABLE(AMP_DQ, ch) = 0x00000008;
		rAMP_MDLLFREQBINDISABLE(AMP_CA, ch) = 0x00000008;
# endif		
#else
		rAMP_DLLUPDTINTVL(AMP_DQ, ch) = 0;
		rAMP_DLLUPDTINTVL(AMP_CA, ch) = 0;

# ifdef rAMP_MDLLFREQBINDISABLE
		rAMP_MDLLFREQBINDISABLE(AMP_DQ, ch) = 0x0000000F;
		rAMP_MDLLFREQBINDISABLE(AMP_CA, ch) = 0x0000000F;
# endif
#endif

#if !SUPPORT_FPGA
		rAMP_DLLEN(AMP_DQ, ch) = 0x00000100;
		rAMP_DLLEN(AMP_CA, ch) = 0x00000100;

		rAMP_DLLEN(AMP_DQ, ch) = 0x00000101;
		rAMP_DLLEN(AMP_CA, ch) = 0x00000101;

		rAMP_DLLEN(AMP_DQ, ch) = 0x00000100;
		rAMP_DLLEN(AMP_CA, ch) = 0x00000100;
		
		amc_phy_run_dll_update(ch);
#endif

		rAMP_AMPINIT(AMP_DQ, ch) = 0x00000001;
		rAMP_AMPINIT(AMP_CA, ch) = 0x00000001;

#if !SUPPORT_FPGA
		rAMP_IMPCALCMD(AMP_CA, ch) = 0x00000101;
		rAMP_IMPCALCMD(AMP_DQ, ch) = 0x00000101;
		
		while (rAMP_IMPCALCMD(AMP_CA, ch) & 0x1) {}
		while (rAMP_IMPCALCMD(AMP_DQ, ch) & 0x1) {}
        
		// Wait 5 us after Impedence Calibration to avoid McPhyPending
		// preventing the SRFSM from exiting SR.
		spin(5);
#endif
	}
	
#if !SUPPORT_FPGA
#ifndef AMP_CALIBRATION_SKIP
	// Restore CA and WrLvl offsets from PMU
	if (resume)
		save_restore_ca_wrlvl_regs(CALIB_RESTORE);
#endif
#endif
}

void amc_phy_enable_dqs_pulldown(bool enable)
{
// Stub because H6 init sequence does not recommend doing this
}

void amc_phy_scale_dll(int freqsel, int factor)
{
}

void amc_phy_run_dll_update(uint8_t ch)
{
	rAMP_DLLUPDTCMD(AMP_DQ, ch) = 0x00000001;
	rAMP_DLLUPDTCMD(AMP_CA, ch) = 0x00000001;

	while ((rAMP_DLLUPDTCMD(AMP_DQ, ch) & 0x1) != 0) ;	
	while ((rAMP_DLLUPDTCMD(AMP_CA, ch) & 0x1) != 0) ;	
}

void amc_phy_bypass_prep(int step)
{	
}

void amc_phy_finalize()
{
}

// Perform CA, RDDQ, and WRLVL calibration
void amc_phy_calibration_ca_rddq_cal(bool resume)
{
#ifndef AMP_CALIBRATION_SKIP
	
	if ((amc_phy_params.cacalib_hw_loops * amc_phy_params.cacalib_sw_loops) != CA_NUM_PATTERNS)
		panic("Memory calibration: hwloops (%d) and swloops (%d) values are unexpected\n",
		      amc_phy_params.cacalib_hw_loops, amc_phy_params.cacalib_sw_loops);
	
	amc_calibration_start(true);

	if (!resume)
		calibrate_ca();
	
	/*
	 * The first RdDq Cal is using MRR32 and MRR40. It's needed for WrDq calibration. Hence run before WrDq.
	 * The second Rd Dq calibration is PRBS pattern based, which needs Wr Dq calibration done. Hence done after WrDq.
	 * PRBS patterns help in reducing aliasing, hence needed for better accuracy.
	 */
	calibrate_rddq(false);
	
	if (!resume) {
		calibrate_wrlvl();
		// Save off the CA and WrLvl offsets to PMU
		save_restore_ca_wrlvl_regs(CALIB_SAVE);
	}
	
	amc_calibration_start(false);
#endif
}

void amc_phy_calibration_wrdq_cal(bool resume)
{
#ifndef AMP_CALIBRATION_SKIP
	if (resume)
		save_restore_memory_region(true);
	
	amc_calibration_start(true);
	
	// ok to keep PSQWQCTL0 and PSQWQCTL1 at their value setup for wrdqcal even for the rddqcal that follows
	amc_wrdqcal_start(true);
	
	calibrate_wrdq();
	
	/*
	 * The first RdDq Cal is using MRR32 and MRR40. It's needed for WrDq calibration. Hence run before WrDq.
	 * The second Rd Dq calibration is PRBS pattern based, which needs Wr Dq calibration done. Hence done after WrDq.
	 * PRBS patterns help in reducing aliasing, hence needed for better accuracy.
	 */
	calibrate_rddq(true);
 
	amc_wrdqcal_start(false);
	
	amc_calibration_start(false);
		
	if (resume)
		save_restore_memory_region(false);
#endif

	uint8_t ch;
	for (ch = 0; ch < AMC_NUM_CHANNELS; ch++) {
		// Restore AMP DLL update interval to POR value
		// <rdar://problem/15777393> CoreOS: M7 init sequence change
#if !SUPPORT_FPGA
#if SUB_PLATFORM_S7002
		rAMP_DLLUPDTINTVL(AMP_DQ, ch) = 0x1020005a;
		rAMP_DLLUPDTINTVL(AMP_CA, ch) = 0x1020005a;
#endif
#endif

		// Enable AMP clock gating only after Wrdqcal is done
		rAMP_AMPCLK(AMP_DQ, ch) = 0x00010000;
		rAMP_AMPCLK(AMP_CA, ch) = 0x00010000;
	}
}

///////////////////////////////////////////////////////////////////////////////
////// Local functions
///////////////////////////////////////////////////////////////////////////////

// To dump calibration results from iBoot menu command
static int dump_mem_calibration_info(int argc, struct cmd_arg *args)
{
	uint32_t ch, byte, bit;
	
	dprintf(DEBUG_INFO, "Memory calibration results\n");
	
	for (ch = 0; ch < AMC_NUM_CHANNELS; ch++) {
		dprintf(DEBUG_INFO, "Channel %d\n", ch);
		
		dprintf(DEBUG_INFO, "\tCA SDLL: 0x%02x\n", rAMP_CASDLLCTRL(ch) & DLLVAL_BITS);
		
		dprintf(DEBUG_INFO, "\t\tPer Bit Deskew: ");
		for (bit = 0; bit < CA_NUM_BITS; bit++)
			dprintf(DEBUG_INFO, "0x%02x ", rAMP_CADESKEW_CTRL(ch, bit) & DESKEW_CTRL_BITS);
		
		dprintf(DEBUG_INFO, "\n\t\tCS, CK, CKE Deskew: 0x%02x", rAMP_CKDESKEW_CTRL(ch) & DESKEW_CTRL_BITS);
		
		dprintf(DEBUG_INFO, "\n");
		
		dprintf(DEBUG_INFO, "\tCA WrLvlSDLL: 0x%02x\n", rAMP_CAWRLVLSDLLCODE(ch) & DLLVAL_BITS);
		dprintf(DEBUG_INFO, "\tDQ WrLvlSDLL: ");
		for (byte = 0; byte < DQ_NUM_BYTES; byte++)
			dprintf(DEBUG_INFO, "0x%02x ", rAMP_DQWRLVLSDLLCODE(ch, byte) & DLLVAL_BITS);
		
		dprintf(DEBUG_INFO, "\n");
		
		dprintf(DEBUG_INFO, "\tRead DQ:\n");
		for (byte = 0; byte < DQ_NUM_BYTES; byte++) {
			dprintf(DEBUG_INFO, "\t\tByte %d SDLL: 0x%02x\n", byte, rAMP_DQSDLLCTRL_RD(ch, byte) & DLLVAL_BITS);
			
			dprintf(DEBUG_INFO, "\t\t\tPer Bit Deskew: ");
			for (bit = 0; bit < DQ_NUM_BITS_PER_BYTE; bit++)
				dprintf(DEBUG_INFO, "0x%02x ", rAMP_RDDQDESKEW_CTRL(ch, byte, bit) & DESKEW_CTRL_BITS);
			
			dprintf(DEBUG_INFO, "\n");
			
		}
		
		dprintf(DEBUG_INFO, "\tWrite DQ:\n");
		for (byte = 0; byte < DQ_NUM_BYTES; byte++) {
			dprintf(DEBUG_INFO, "\t\tByte %d SDLL: 0x%02x\n", byte, rAMP_DQSDLLCTRL_WR(ch, byte) & DLLVAL_BITS);
			
			dprintf(DEBUG_INFO, "\t\t\tPer Bit Deskew: ");
			for (bit = 0; bit < DQ_NUM_BITS_PER_BYTE; bit++)
				dprintf(DEBUG_INFO, "0x%02x ", rAMP_WRDQDESKEW_CTRL(ch, byte, bit) & DESKEW_CTRL_BITS);
			
			dprintf(DEBUG_INFO, "\n");
		}
	}
	
	return 0;
}
MENU_COMMAND_DEBUG(memcal_info, dump_mem_calibration_info, "Prints memory calibration results", NULL);

static void calibrate_ca(void)
{
	uint32_t ch, rnk, swloop, mask_bits;

	generate_ca_patterns_mask();
	
	// Required since the dll values may change slightly during calibration
	save_masterdll_values();
	
	// Calibration sequence is to be run for each rank in each channel, amc_phy_params.cacalib_sw_loops number of times
	for (ch = 0; ch < AMC_NUM_CHANNELS; ch++) {
		for (rnk = 0; rnk < AMC_NUM_RANKS; rnk++) {
			for (swloop = 0; swloop < amc_phy_params.cacalib_sw_loops; swloop++) {
				amp_program_ca_patterns(ch, rnk, swloop);
				
				amp_init_ca_offset_and_deskew(ch);
				
				// Training of CA Bits 0-3 and 5-8: MR41 cmd (training cmd must be sent before cacalibmode is enabled in AMP)
				mask_bits = amp_mask_ca_bits(ch, MR41);
				amc_mrcmd_to_ch_rnk(MR_WRITE, ch, rnk, MR41, MR41 << 2);
				amp_enable_cacal_mode(true, ch);
				run_cacal_sequence(ch, rnk, MR41, mask_bits, swloop);
				amp_enable_cacal_mode(false, ch);

				amp_init_ca_offset_and_deskew(ch);
				
				// Training of CA Bits 4 and 9: MR48 cmd (training cmd must be sent before cacalibmode is enabled in AMP)
				mask_bits = amp_mask_ca_bits(ch, MR48);
				amc_mrcmd_to_ch_rnk(MR_WRITE, ch, rnk, MR48, MR48 << 2);
				amp_enable_cacal_mode(true, ch);
				run_cacal_sequence(ch, rnk, MR48, mask_bits, swloop);
				amp_enable_cacal_mode(false, ch);
				
				amp_init_ca_offset_and_deskew(ch);
				
				// Exit CA Training mode: MR42
				amc_mrcmd_to_ch_rnk(MR_WRITE, ch, rnk, MR42, MR42 << 2);
			}
		}
	}
	
	// By now, we have compiled right and left edges of passing window for all CA bits over a number of iterations
	// Aggregate the results, and find the center point of the window, and program it
	ca_program_final_values();
}

static void calibrate_rddq(bool after_wrddqcal)
{
	uint32_t ch, rnk, data;
	
	// step7
	if (after_wrddqcal == false) {
		
		for (ch = 0; ch < AMC_NUM_CHANNELS; ch++) {
			for (rnk = 0; rnk < AMC_NUM_RANKS; rnk++) {
				amp_setup_rddq_cal(ch, rnk);
				
				amc_mrcmd_to_ch_rnk(MR_READ, ch, rnk, MR5, (uintptr_t)&data);
				
				amc_enable_rddqcal(true);
				
				// Find the left and right edges of the eye
				find_rddqcal_right_side_failing_point(ch, rnk, false);
				find_rddqcal_left_side_failing_point(ch, rnk, false);
								
				amc_enable_rddqcal(false);
			}
		}
	}
	// step10
	else {
		for (ch = 0; ch < AMC_NUM_CHANNELS; ch++) {
			for (rnk = 0; rnk < AMC_NUM_RANKS; rnk++) {
				// Find the left and right edges of the eye using PRBS patterns
				// These results will be more accurate
				find_rddqcal_right_side_failing_point(ch, rnk, true);
				find_rddqcal_left_side_failing_point(ch, rnk, true);
			}
		}
	}
	
	// Now that we have per bit left and right endpoints for each channel and rank, aggregate and program final values
	rddq_program_final_values(after_wrddqcal);
}

// Align the clock signal with the DQ signals
static void calibrate_wrlvl(void)
{
	uint32_t ch, rnk;
	uint32_t data, cawrlvlsdll;
	
	amp_wrlvl_init();
	
	for (ch = 0; ch < AMC_NUM_CHANNELS; ch++) {
		
		cawrlvlsdll = rAMP_CAWRLVLSDLLCODE(ch);
		
		for (rnk = 0; rnk < AMC_NUM_RANKS; rnk++) {
			data = 0x80 + RD_LATENCY_ENCODE; // 0x80 is added here to set the Write Level bit (bit 7) to 1
			
			amc_mrcmd_to_ch_rnk(MR_WRITE, ch, rnk, MR2, data);

			// find the region where all bits return a 0
			push_wrlvl_to_0s_region(ch, rnk);
			
			// push out the clock signal until all bits return a 1
			find_wrlvl_0to1_transition(ch, rnk);
			
			// now go back towards the transition edge found earlier, but from this side of the edge
			find_wrlvl_1to0_transition(ch, rnk);
			
			// reset cawrlvlsdllcode to original value (0), before sending cmd to exit wrlvl mode (MR2)
			amp_set_cawrlvl_sdll(ch, cawrlvlsdll, false);
			
			data = RD_LATENCY_ENCODE;
			amc_mrcmd_to_ch_rnk(MR_WRITE, ch, rnk, MR2, data);
		}
	}
	
	// Program the final wrlvl values
	wrlvl_program_final_values();
}

static void calibrate_wrdq(void)
{
	uint32_t ch, rnk;
	
	for (ch = 0; ch < AMC_NUM_CHANNELS; ch++) {
		for (rnk = 0; rnk < AMC_NUM_RANKS; rnk++) {
			find_wrdqcal_right_side_failing_point(ch, rnk);
			find_wrdqcal_left_side_failing_point(ch, rnk);
		}
	}
	
	wrdq_program_final_values();
}

static void save_masterdll_values(void)
{
	uint32_t ch;
	
	for (ch = 0; ch < AMC_NUM_CHANNELS; ch++) {
		mdllcode[ch][AMP_DQ] = (rAMP_MDLLCODE(AMP_DQ, ch) & DLLVAL_BITS);
		mdllcode[ch][AMP_CA] = (rAMP_MDLLCODE(AMP_CA, ch) & DLLVAL_BITS);
	}
}

static void generate_ca_patterns_mask(void)
{
	uint32_t index, patr, patf = 0;
	uint32_t mask = 0;
	
	// generate the pattern to be used for each CA calibration iteration
	for (index = 0; index < (amc_phy_params.cacalib_sw_loops * amc_phy_params.cacalib_hw_loops); index++) {
		patr    = (CA_PRBS7_PATTERNS[index]) & CA_ALL_BITS;
		patf    = (CA_PRBS7_PATTERNS[index]) >> CA_NUM_BITS;
		mask    = patr ^ patf;
		ca_patterns_mask[index] = mask;
	}
}

static void amp_program_ca_patterns(uint32_t ch, uint32_t rnk, uint32_t swloop)
{
	uint32_t cacalctrl, p;
	
	// Program rank, hardware loop count, and timing params
	// Timing params are taken from lpddr3 jedec spec
	cacalctrl = (rnk << 24) | ((amc_phy_params.cacalib_hw_loops - 1) << 16) | (16 << 8) | (10 << 0);
	
	rAMP_CACALCTRL(AMP_DQ, ch) = cacalctrl;
	rAMP_CACALCTRL(AMP_CA, ch) = cacalctrl;
	
	for (p = 0; p < AMP_MAX_PATTERNS; p++) {
		rAMP_CACALPAT(AMP_DQ, ch, p) = CA_PRBS7_PATTERNS[(swloop * amc_phy_params.cacalib_hw_loops) + p];
		rAMP_CACALPAT(AMP_CA, ch, p) = CA_PRBS7_PATTERNS[(swloop * amc_phy_params.cacalib_hw_loops) + p];
	}
}

// (Re-)Initialize ca offset and deskew registers
static void amp_init_ca_offset_and_deskew(uint32_t ch)
{
	uint8_t d;
	int32_t camdllcode = mdllcode[ch][AMP_CA];
	
	// ensure negative sign is set with mdllcode value for ca offset (mdllcode guaranteed by designers not to be negative)
	amp_push_casdll_out(ch, (-1 * camdllcode));
	
	// Clear cadeskewctrl registers
	for (d = 0; d < CA_NUM_BITS; d++)
		rAMP_CADESKEW_CTRL(ch, d) = 0;
}

static uint32_t amp_mask_ca_bits(uint32_t ch, uint32_t mr_cmd)
{
	uint32_t mask_bits;
	
	// Assuming no byte swizzling
	if (mr_cmd == MR41) {
		// MR41:  Mask out bits 9 and 4
		mask_bits = 0x210;
	} else if (mr_cmd == MR48) {
		// MR48:  Mask out bits 0-3 and bits 5-8
		mask_bits = 0x1EF;
	} else {
		// No bits are masked out
		mask_bits = 0;
	}
	
	rAMP_CACALMASK(AMP_DQ, ch) = mask_bits;
	rAMP_CACALMASK(AMP_CA, ch) = mask_bits;
	
	return mask_bits;
}

static void amp_push_casdll_out(uint32_t ch, int32_t offset)
{
	uint32_t ca_bit;
	uint32_t cadeskewcode;
	int32_t camdllcode = mdllcode[ch][AMP_CA];
	
	if (offset > 0) {
		// New equation given by Rakesh: if offset is within DELIMIT_POS_ADJ_CASDLL steps of camdllcode, limit it to (master dll - DELIMIT_POS_ADJ_CASDLL)
		if (offset >= (camdllcode - DELIMIT_POS_ADJ_CASDLL)) {
			uint8_t difference = (uint8_t) (offset - (camdllcode - DELIMIT_POS_ADJ_CASDLL));
			offset = camdllcode - DELIMIT_POS_ADJ_CASDLL;
			
			if (difference >= MAX_DESKEW_PROGRAMMED)
				cadeskewcode = MAX_DESKEW_PROGRAMMED;
			else
				cadeskewcode = difference;

			// Adjust deskew registers for each ca bit
			for (ca_bit = 0; ca_bit < CA_NUM_BITS; ca_bit++)
				rAMP_CADESKEW_CTRL(ch, ca_bit) = cadeskewcode;

		}
	}
	
	rAMP_CASDLLCTRL(ch) = (1 << 24) | INT_TO_OFFSET(offset);
	while (rAMP_CASDLLCTRL(ch) & (1 << 24));
}

static void amp_enable_cacal_mode(bool enable, uint32_t ch)
{
	// Set or clear CACalMode bit
	if (enable)
		rAMP_CACALRUN(AMP_CA, ch) |= CACALRUN_CACALMODE;
	else
		rAMP_CACALRUN(AMP_CA, ch) &= ~CACALRUN_CACALMODE;
}

static void amp_run_cacal(uint32_t ch)
{
	// DQ must be set before CA
	rAMP_CACALRUN(AMP_DQ, ch) |= CACALRUN_RUNCACAL;
	// CACalMode should already be set
	rAMP_CACALRUN(AMP_CA, ch) |= CACALRUN_RUNCACAL;
	// Poll on the DQ register
	while(rAMP_CACALRUN(AMP_DQ, ch) & CACALRUN_RUNCACAL);
}

static void amp_push_ctl_out(uint32_t ch, uint32_t dly_val)
{	
	uint32_t cadramsigdly;
	
	rAMP_TESTMODE(AMP_CA, ch) = TESTMODE_FORCECKELOW;
	
	// Fix for Radar 10790574 - Hold Violation on CKE
	if (dly_val >= 0xd)
		cadramsigdly = (3 << 4);
	else if (dly_val >= 0xa)
		cadramsigdly = (2 << 4);
	else if (dly_val >= 0x8)
		cadramsigdly = (1 << 4);
	else
		cadramsigdly = (0 << 4);

	rAMP_DRAMSIGDLY(AMP_CA, ch, 0) = cadramsigdly;
	rAMP_CSDESKEW_CTRL(ch) = dly_val;
	rAMP_CKDESKEW_CTRL(ch) = dly_val;
	rAMP_CKEDESKEW_CTRL(ch) = dly_val;
		
	rAMP_TESTMODE(AMP_CA, ch) = 0;
}

static void amp_setup_rddq_cal(uint32_t ch, uint32_t rnk)
{
	// At this point the AMC's READLEVELING should already be setup as 0x00000300
	// Make DQCALCTRL.DQCalPatSel (bits 1:0) match READLEVELING.RdLvlPatOpt
	rAMP_DQCALCTRL(ch) = (rnk << 16) | (RDDQ_LOOPCNT << 8) | (3 << 0);
}

// This functions set the slave dll for a particular byte lane of RDDQ as specified in the offset parameter
static void amp_set_rddq_sdll(uint32_t ch, uint32_t byte, uint32_t offset)
{
	rAMP_DQSDLLCTRL_RD(ch, byte) = (1 << 24) | offset;
	// Wait for Run bit to clear
	while(rAMP_DQSDLLCTRL_RD(ch, byte) & (1 << 24));
}

static void amp_run_rddqcal(uint32_t ch)
{
	rAMP_DQCALRUN(ch) = 1;
	while (rAMP_DQCALRUN(ch) & 1);
}

static void amp_wrlvl_init(void)
{
	uint32_t ch;
	
	for (ch = 0; ch < AMC_NUM_CHANNELS; ch++) {
		// Write Leveling Timing Control Registers to program tMRD and tWLO timing params
		// Taking these values from jedec_lpddr3s8_4gb_x32_1600.soma
		// tWLO has a max value of 20ns for 1600 freq
		// tWLMRD has a min value of 40tck for 1600 freq
		
		rAMP_DQWRLVLTIM(ch) = (12 << 8) | (16 << 0);
	}
}

// follow a certain sequence required when wrlvl dlysel is to be updated
static void amp_phy_update(uint32_t ch, uint32_t update) {
	// Release CKE and disable phyupdt to allow normal operation
	if (!update)
		rAMP_TESTMODE(AMP_CA, ch) = 0;
	
	// issue phyupdt to block AMC traffic
	rAMP_CAPHYUPDTCRTL(ch) = update;
	// wait for the phyupdt change to take effect. there is only 1 bit in the status reg: bit 0.
	while((rAMP_CAPHYUPDTSTATUS(ch) & 1) != (update & 1));
	
	// CKE must be low when updating dlysel to avoid glitches
	if (update)
		rAMP_TESTMODE(AMP_CA, ch) = TESTMODE_FORCECKELOW;
}

// Must ensure that WrLvL SDLLs are programmed with all precautions to avoid glitch on clock signals
static void amp_set_cawrlvl_sdll(uint32_t ch, uint32_t offset, bool set_dly_sel)
{
	if (set_dly_sel) {
		// Must send phyupdt to AMC to avoid traffic while CKE is low
		amp_phy_update(ch, 1);
		
		// Ok to set directly to final value (instead of incrementing) since CKE is low
		rAMP_CAWRLVLSDLLCODE(ch) = offset;
		
		/*
		 * Since M7 memory clock much slower than Alcatraz, need to toggle phyupdt to ensure
		 * refreshes that have piled up due to phyupdt being set are flushed out
		 */
		if (amc_phy_params.wrlvl_togglephyupdt) {
			amp_phy_update(ch, 0);
			amp_phy_update(ch, 1);
		}
		
		// program the dlysel value with CKE low to avoid glitches to DRAM
		rAMP_CAWRLVLCLKDLYSEL(ch) = wrlvl_encode_dlyval(ch, AMP_CA, offset);
		
		// disable phyupdt and release CKE back to high
		amp_phy_update(ch, 0);
	} else {
		uint32_t cawrlvlsdll = rAMP_CAWRLVLSDLLCODE(ch);
		int32_t step = (cawrlvlsdll < offset) ? 1 : -1;

		// when CKE is not low, need to step by 1 to avoid glitches to DRAM
		for ( ; cawrlvlsdll != offset; cawrlvlsdll += step)
			rAMP_CAWRLVLSDLLCODE(ch) = cawrlvlsdll + step;
	}
}

// Must ensure that WrLvL SDLLs are programmed with all precautions to avoid glitches
static void amp_set_dqwrlvl_sdll(uint32_t ch, uint32_t byte, uint32_t offset, bool set_dly_sel)
{
	if (set_dly_sel) {
		uint32_t dqwrlvldlychainctrl = rAMP_DQWRLVLDLYCHAINCTRL(ch, byte);
		
		// Must send phyupdt to AMC to avoid traffic while CKE is low
		amp_phy_update(ch, 1);
		
		// Ok to set directory final value (instead of incrementing) since CKE is low
		rAMP_DQWRLVLSDLLCODE(ch, byte) = offset;
		
		/*
		 * Since M7 memory clock much slower than Alcatraz, need to toggle phyupdt to ensure
		 * refreshes that have piled up due to phyupdt being set are flushed out
		 */
		if (amc_phy_params.wrlvl_togglephyupdt) {
			amp_phy_update(ch, 0);
			amp_phy_update(ch, 1);
		}
				
		// program dlysel (also, preserve bits 17:16) with CKE low to avoid glitches to DRAM
		rAMP_DQWRLVLDLYCHAINCTRL(ch, byte) = (dqwrlvldlychainctrl & 0x00030000) | wrlvl_encode_dlyval(ch, AMP_DQ, offset);
		
		// disable phyupdt and release CKE back to high
		amp_phy_update(ch, 0);
	} else {
		uint32_t dqwrlvlsdll = rAMP_DQWRLVLSDLLCODE(ch, byte);
		int32_t step = (dqwrlvlsdll < offset) ? 1 : -1;
		
		// when CKE is not low, need to step by 1 to avoid glitches to DRAM
		for ( ; dqwrlvlsdll != offset; dqwrlvlsdll += step)
			rAMP_DQWRLVLSDLLCODE(ch, byte) = dqwrlvlsdll + step;
	}
}

static void amp_run_wrlvlcal(uint32_t ch, uint32_t wrlvlrun)
{
	rAMP_DQWRLVLRUN(ch) = wrlvlrun;
	while(rAMP_DQWRLVLRUN(ch));
}

// This functions set the slave dll for a particular byte lane of WRDQ as specified in the offset parameter
static void amp_set_wrdq_sdll(uint32_t ch, uint32_t byte, int32_t offset)
{
	uint32_t dq_bit;
	uint32_t dqdeskewcode;
	int32_t dqmdllcode = mdllcode[ch][AMP_DQ];
	
	if (offset > 0) {
		// New equation given by Rakesh: if offset is within DELIMIT_POS_ADJ_WRDQSDLL steps of dqmdllcode, limit it to (master dll - DELIMIT_POS_ADJ_WRDQSDLL)
		if (offset >= (dqmdllcode - DELIMIT_POS_ADJ_WRDQSDLL)) {
			uint8_t difference = (uint8_t) (offset - (dqmdllcode - DELIMIT_POS_ADJ_WRDQSDLL));
			offset = dqmdllcode - DELIMIT_POS_ADJ_WRDQSDLL;
				
			if (difference >= DQ_MAX_DESKEW_PER_BIT)
				dqdeskewcode = DQ_MAX_DESKEW_PER_BIT;
			else
				dqdeskewcode = difference;
			
			// Adjust deskew registers for each dq bit
			for (dq_bit = 0; dq_bit < DQ_NUM_BITS_PER_BYTE; dq_bit++)
				rAMP_WRDQDESKEW_CTRL(ch, byte, dq_bit) = dqdeskewcode;

			// Also update the Data Mask (DM), controlled by the DQSDESKEW register
			rAMP_WRDQSDESKEW_CTRL(ch, byte) = dqdeskewcode;
		}
		
		// set wrlvlclk90dly (bits 17:16 of wrlvldlychainctrl reg) if positive sdll value
		rAMP_DQWRLVLDLYCHAINCTRL(ch, byte) = (wrlvl_encode_clk90dly(ch, offset) << 16) | (rAMP_DQWRLVLDLYCHAINCTRL(ch, byte) & 0x3);
	}
		
	rAMP_DQSDLLCTRL_WR(ch, byte) = (1 << 24) | INT_TO_OFFSET(offset);
	// Wait for Run bit to clear
	while(rAMP_DQSDLLCTRL_WR(ch, byte) & (1 << 24));
}

static void run_cacal_sequence(uint32_t ch, uint32_t rnk, uint32_t mr_cmd, uint32_t mask_bits, uint32_t swloop)
{
	uint32_t combined_mask, hwloop;
	uint32_t pat_mask = 0;
	
	for (hwloop = 0; hwloop < amc_phy_params.cacalib_hw_loops; hwloop++)
		pat_mask |= ca_patterns_mask[(swloop * amc_phy_params.cacalib_hw_loops) + hwloop];
	
	// This represents the bits that don't have a transition on any of the patterns used during the hwloop calibration
	combined_mask = mask_bits | (CA_ALL_BITS - pat_mask);
	
	// To find the FAIL <-> PASS <-> FAIL window
	find_cacal_right_side_failing_point(ch, rnk, mr_cmd, combined_mask, swloop);
	find_cacal_left_side_failing_point(ch, rnk, combined_mask, swloop);
}

// Establish the right edge of the window by finding the point where all CA bits fail
static void find_cacal_right_side_failing_point(uint32_t ch, uint32_t rnk, uint32_t mr_cmd, uint32_t combined_mask, uint32_t swloop)
{
	bool all_bits_fail = false;
	uint32_t cacalresult = 0;
	uint32_t push_ck_out = 0;
	
	cacalresult = CA_ALL_BITS;
	
	// Increase delay to the right until all bits fail
	do {
		amp_run_cacal(ch);
		cacalresult = cacalresult & (rAMP_CACALRESULT(ch) & CA_ALL_BITS);
		
		if ((cacalresult & (CA_ALL_BITS ^ combined_mask)) != 0) {
			all_bits_fail = false;
			push_ck_out = push_ck_out + FINER_STEP_SZ;
			
			amp_init_ca_offset_and_deskew(ch);
			
			// Make AMP and DRAM exit CaCal Mode in order to update the CK, CKE, and CS delays
			enter_cacal_mode(ch, rnk, mr_cmd, false);
			
			// Update CK, CKE, and CS signal delays
			amp_push_ctl_out(ch, push_ck_out);
			
			// Re-enter CaCal mode
			enter_cacal_mode(ch, rnk, mr_cmd, true);
		} else {
			all_bits_fail = true;
			
			// Do a per bit calculation of when they start passing again
			find_cacal_right_side_passing_point(ch, rnk, mr_cmd, combined_mask, swloop);
		}
	} while ((push_ck_out < MAX_DESKEW_OFFSET) && (all_bits_fail == false));
	
	if ((push_ck_out >= MAX_DESKEW_OFFSET) && (all_bits_fail == false)) {
		dprintf(DEBUG_INFO, "Memory CA calibration: Unable to find right side failing point for channel %d\n", ch);
		
		// Failing point cannot be found, continuing to passing point assuming failure at this setting
		find_cacal_right_side_passing_point(ch, rnk, mr_cmd, combined_mask, swloop);
	}
	
	// Reset CK delay back to 0
	if (rAMP_CKDESKEW_CTRL(ch)) {
		// Exit CaCal Mode for AMP and DRAM before modifying CK, CKE, and CS signals
		enter_cacal_mode(ch, rnk, mr_cmd, false);
		
		// Ok from Rakesh to set to 0 directly instead of decrementing by 1
		amp_push_ctl_out(ch, 0);
		
		// Re-enable CACal Mode
		enter_cacal_mode(ch, rnk, mr_cmd, true);
	}
}

// Finds the passing region on the right edge of window
static void find_cacal_right_side_passing_point(uint32_t ch, uint32_t rnk, uint32_t mr_cmd, uint32_t combined_mask, uint32_t swloop)
{
	bool switch_from_cktoca;
	int32_t tap_value;
	uint32_t cacalresult;
	int32_t camdllcode;
	int32_t saved_val;
	uint32_t all_bits_pass;
	uint32_t BitPass[CA_NUM_BITS] = { 0 };
	uint32_t SolidBitPass[CA_NUM_BITS] = { 0 };
	uint32_t step_incr;
	uint8_t bit_indx;
	uint32_t ckdeskew;
	uint32_t loopchrnk_indx;
		
	all_bits_pass = 0;
	step_incr  = FINER_STEP_SZ;
	camdllcode = mdllcode[ch][AMP_CA];
	ckdeskew = rAMP_CKDESKEW_CTRL(ch);
	
	// For every swloop, we'll save passing values for each channel & rank
	loopchrnk_indx = (swloop * AMC_NUM_CHANNELS * AMC_NUM_RANKS) + (ch * AMC_NUM_RANKS) + rnk;
	
	if (ckdeskew) {
		tap_value = ckdeskew;
		switch_from_cktoca = false;
	} else {
		// Since clock delay is already down to 0, use the slave delay.
		// We only have 2 knobs to turn for delay: clock and sdll
		tap_value = (rAMP_CASDLLCTRL(ch) & DLLVAL_BITS);
		tap_value = OFFSET_TO_INT(tap_value);
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
			enter_cacal_mode(ch, rnk, mr_cmd, false);
			
			// Update CK, CKE, and CS signal delays
			amp_push_ctl_out(ch, tap_value);
			
			// Re-enter CaCal mode
			enter_cacal_mode(ch, rnk, mr_cmd, true);
		} else {
			amp_push_casdll_out(ch, tap_value);
		}

		// Run the ca calibration in hw
		amp_run_cacal(ch);
		cacalresult = rAMP_CACALRESULT(ch) & CA_ALL_BITS;
				
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
							saved_val = -1 * (tap_value + camdllcode);
						} else {
							saved_val = tap_value;
						}
						
						ca_cal_per_loopchrnk_right[loopchrnk_indx][bit_indx] = saved_val;
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
				
				tap_value = (rAMP_CASDLLCTRL(ch) & DLLVAL_BITS);
				tap_value = OFFSET_TO_INT(tap_value);
			}
			
			if (switch_from_cktoca == false) {
				tap_value = tap_value - step_incr;
			} else {
				tap_value = tap_value + step_incr;
			}
		}
		
	} while ((tap_value <= MAX_SDLL_VAL) && (all_bits_pass == 0));
	
	if (all_bits_pass == 0) {
		panic("Memory CA calibration: Unable to find passing point for all bits on the right side");
	}
}

static void enter_cacal_mode(uint32_t ch, uint32_t rnk, uint32_t mr_cmd, bool enter)
{
	// For entry, send MR41 command to DRAM before AMP register is changed
	if (enter) {
		// Re-enter CaCal Mode with MR41 always since some DRAMs don't support entering this mode with MR48
		amc_mrcmd_to_ch_rnk(MR_WRITE, ch, rnk, MR41, MR41 << 2);
		if (mr_cmd != MR41)
			amc_mrcmd_to_ch_rnk(MR_WRITE, ch, rnk, mr_cmd, mr_cmd << 2);
		
		amp_enable_cacal_mode(true, ch);
	}
	
	// For exit, change AMP register before sending DRAM command (MR42)
	else {
		amp_enable_cacal_mode(false, ch);
		amc_mrcmd_to_ch_rnk(MR_WRITE, ch, rnk, MR42, MR42 << 2);
	}
}

static void find_cacal_left_side_failing_point(uint32_t ch, uint32_t rnk, uint32_t combined_mask, uint32_t swloop)
{
	// At this point, we've already played with all possible CK delays. At the end of find_cacal_right_side_failing_point routine,
	// we reset the CK delays to 0.
	// Loop through CaSDLLOvrVal from -MasterDLL to +Max until all failing points on the left side are found

	uint32_t all_bits_fail;
	uint32_t step_incr;
	int32_t push_ca_out;
	uint32_t cacalresult;
	int32_t camdllcode;
	uint32_t max_caleft_point_reached;
	int32_t max_caleft_point_val;
	int32_t casdllctrl, ca0deskewctrl;
		
	all_bits_fail = 0;
	cacalresult = CA_ALL_BITS;
	step_incr  = COARSE_STEP_SZ;
	max_caleft_point_reached = 0;
	
	camdllcode = mdllcode[ch][AMP_CA];
	max_caleft_point_val = camdllcode + MAX_DESKEW_OFFSET - DELIMIT_POS_ADJ_CASDLL;
	
	casdllctrl = rAMP_CASDLLCTRL(ch) & DLLVAL_BITS;
	casdllctrl= OFFSET_TO_INT(casdllctrl);
	ca0deskewctrl = rAMP_CADESKEW_CTRL(ch, 0);
	
	// ca0deskewctrl will be non-zero only if casdll reached (camdllcode - DELIMIT_POS_ADJ_CASDLL)
	push_ca_out = casdllctrl + ca0deskewctrl;
	
	// Increment CaSDLLOvrVal from -ve Master Code to +MAX_SDLL_VAL
	do {
		if (push_ca_out >= max_caleft_point_val) {
			max_caleft_point_reached = 1;
		}
		// Push out this new ca offset
		amp_push_casdll_out(ch, push_ca_out);
		
		// run the calibration in hw
		amp_run_cacal(ch);
		cacalresult = cacalresult & (rAMP_CACALRESULT(ch) & CA_ALL_BITS);
		
		// combined mask has don't care bits (based on pattern) and masked bits (based on MR41 or MR48) that we should ignore
		if ((cacalresult & (CA_ALL_BITS ^ combined_mask)) != 0) {
			all_bits_fail = 0;
		} else {
			all_bits_fail = 1;
			
			// Now, we have found the left edge of window. Find the passing point for all bits
			find_cacal_left_side_passing_point(ch, rnk, combined_mask, swloop);
		}
		
		// increase the offset
		if (all_bits_fail == 0)
			push_ca_out = push_ca_out + step_incr;
		
		if ((push_ca_out > MAX_SDLL_VAL) && (all_bits_fail == 0)) {
			panic("Memory CA calibration: Unable to find failing point for all bits on the left side");
		}
		
		// Forcefully ending this loop as there are no more sdll taps left to proceed ahead
		if (max_caleft_point_reached && (all_bits_fail == 0))
		{
			dprintf(DEBUG_INFO, "Memory CA calibration: SDLL ran out of taps when trying to find left side failing point\n");

			find_cacal_left_side_passing_point(ch, rnk, combined_mask, swloop);
			all_bits_fail = 1;
		}
	} while ((push_ca_out <= MAX_SDLL_VAL) && (all_bits_fail == 0) && (max_caleft_point_reached == 0));
}

static void find_cacal_left_side_passing_point(uint32_t ch, uint32_t rnk, uint32_t combined_mask, uint32_t swloop)
{
	uint32_t loopchrnk_indx;
	uint32_t BitPass[CA_NUM_BITS] = { 0 };
	uint32_t SolidBitPass[CA_NUM_BITS] = { 0 };
	int32_t tap_value;
	uint32_t cacalresult;
	int32_t camdllcode;
	int32_t ca0deskewctrl;
	uint32_t all_bits_pass;
	uint32_t step_incr;
	uint32_t bit_indx;
	
	loopchrnk_indx = (swloop * AMC_NUM_CHANNELS * AMC_NUM_RANKS) + (ch * AMC_NUM_RANKS) + rnk;
	
	tap_value = rAMP_CASDLLCTRL(ch) & DLLVAL_BITS;
	tap_value = OFFSET_TO_INT(tap_value);
	ca0deskewctrl = rAMP_CADESKEW_CTRL(ch, 0);
	camdllcode = mdllcode[ch][AMP_CA];

	step_incr  = FINER_STEP_SZ;
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
		amp_run_cacal(ch);
		cacalresult = rAMP_CACALRESULT(ch) & CA_ALL_BITS;

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
						
						ca_cal_per_loopchrnk_left[loopchrnk_indx][bit_indx] = tap_value;					}
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
			tap_value = tap_value - step_incr;
		}
		
		if ((tap_value < (-1 * camdllcode)) && (all_bits_pass == 0)) {
			// print error message as Left Failing Point cannot be found
			all_bits_pass = 1;
			panic("Memory CA calibration: Unable to find passing point for all bits on the left side");
		}
	} while ((tap_value > (-1 * camdllcode)) && (all_bits_pass == 0));
}

static void ca_program_final_values(void)
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
	
	for (ch = 0; ch < AMC_NUM_CHANNELS; ch++) {
		camdllcode = mdllcode[ch][AMP_CA];
		
		// Calculate the Center Points for each CA bit
		for (bit_indx=0; bit_indx < CA_NUM_BITS; bit_indx++) {
			comb_mask   = 0x0;
			mask_txn_detect = 0x0;
			tmp_mask    = 0x0;
			
			// Compute the aggr eye over multiple swloop and hwloop for all ranks
			for (swloop = 0; swloop < amc_phy_params.cacalib_sw_loops; swloop++) {
				mask = 0x0;
				for (hwloop=0; hwloop < amc_phy_params.cacalib_hw_loops; hwloop++)
					mask = mask | ca_patterns_mask[(swloop * amc_phy_params.cacalib_hw_loops) + hwloop];
					
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
				
				loopchrnk0_indx = (swloop * AMC_NUM_CHANNELS * AMC_NUM_RANKS) + (ch * AMC_NUM_RANKS) + 0;
				chrnk0_indx = (ch * AMC_NUM_RANKS) + 0;
				
				/* Left side */
				
				// lookup the value in the left side for this bit given loop, ch, and rnk
				rank_val[0] = ca_cal_per_loopchrnk_left[loopchrnk0_indx][bit_indx];
				tmp_left_pos_val = rank_val[0];
					
				// If this is the first time this bit transitioned, just put it in the aggregate result array
				if (mask_txn_detect & (1 << bit_indx)) {
					left_pos_val = tmp_left_pos_val;
					ca_cal_per_chrnk_left[chrnk0_indx][bit_indx] = left_pos_val;
				} else if (mask & (1 << bit_indx)) {
					// This is not the 1st time this bit transitioned so there is a recorded result already, but since we have a new result
					// to compare with, find the value that would cover both points and put that in the array
					left_pos_val = ca_cal_per_chrnk_left[chrnk0_indx][bit_indx];
					left_pos_val = find_common_endpoint(tmp_left_pos_val, left_pos_val, MIN_ENDPT);
					ca_cal_per_chrnk_left[chrnk0_indx][bit_indx] = left_pos_val;
				}

				/* Right side */
				
				// lookup the value in the right side for this bit given loop, ch, and rnk
				rank_val[0] = ca_cal_per_loopchrnk_right[loopchrnk0_indx][bit_indx];
				tmp_right_pos_val = rank_val[0];
					
				// If this is the first time this bit transitioned, just put it in the aggregate result array
				if (mask_txn_detect & (1 << bit_indx)) {
					right_pos_val = tmp_right_pos_val;
					ca_cal_per_chrnk_right[chrnk0_indx][bit_indx] = right_pos_val;
				} else if (mask & (1 << bit_indx)) {
					// This is not the 1st time this bit transitioned so there is a recorded result already, but since we have a new result
					// to compare with, find the value that would cover both points and put that in the array
					right_pos_val = ca_cal_per_chrnk_right[chrnk0_indx][bit_indx];
					right_pos_val = find_common_endpoint(tmp_right_pos_val, right_pos_val, MAX_ENDPT);
					ca_cal_per_chrnk_right[chrnk0_indx][bit_indx] = right_pos_val;
				}
				
				if (AMC_NUM_RANKS > 1) {
					/*
					 * Rank 1
					 */
					
					if (AMC_NUM_RANKS > AMP_MAX_RANKS_PER_CHAN)
						panic("amp_v2: AMC_NUM_RANKS = %d is more than hw is capable of supporting (%d)\n", AMC_NUM_RANKS, AMP_MAX_RANKS_PER_CHAN);
					
					loopchrnk1_indx = (swloop * AMC_NUM_CHANNELS * AMC_NUM_RANKS) + (ch * AMC_NUM_RANKS) + 1;
					chrnk1_indx = (ch * AMC_NUM_RANKS) + 1;
					
					/* Left side */
					
					// lookup the value in the left side for this bit given loop, ch, and rnk
					rank_val[1] = ca_cal_per_loopchrnk_left[loopchrnk1_indx][bit_indx];
					tmp_left_pos_val = rank_val[1];
					
					// If this is the first time this bit transitioned, just put it in the aggregate result array
					if (mask_txn_detect & (1 << bit_indx)) {
						left_pos_val = tmp_left_pos_val;
						ca_cal_per_chrnk_left[chrnk1_indx][bit_indx] = left_pos_val;
					} else if (mask & (1 << bit_indx)) {
						// This is not the 1st time this bit transitioned so there is a recorded result already, but since we have a new result
						// to compare with, find the value that would cover both points and put that in the array
						left_pos_val = ca_cal_per_chrnk_left[chrnk1_indx][bit_indx];
						left_pos_val = find_common_endpoint(tmp_left_pos_val, left_pos_val, MIN_ENDPT);
						ca_cal_per_chrnk_left[chrnk1_indx][bit_indx] = left_pos_val;
					}
					
					/* Right side */
					
					// lookup the value in the right side for this bit given loop, ch, and rnk
					rank_val[1] = ca_cal_per_loopchrnk_right[loopchrnk1_indx][bit_indx];
					tmp_right_pos_val = rank_val[1];
					
					// If this is the first time this bit transitioned, just put it in the aggregate result array
					if (mask_txn_detect & (1 << bit_indx)) {
						right_pos_val = tmp_right_pos_val;
						ca_cal_per_chrnk_right[chrnk1_indx][bit_indx] = right_pos_val;
					} else if (mask & (1 << bit_indx)) {
						// This is not the 1st time this bit transitioned so there is a recorded result already, but since we have a new result
						// to compare with, find the value that would cover both points and put that in the array
						right_pos_val = ca_cal_per_chrnk_right[chrnk1_indx][bit_indx];
						right_pos_val = find_common_endpoint(tmp_right_pos_val, right_pos_val, MAX_ENDPT);
						ca_cal_per_chrnk_right[chrnk1_indx][bit_indx] = right_pos_val;
					}

					// Find the common endpoint for both ranks
					left_pos_val = find_common_endpoint(ca_cal_per_chrnk_left[chrnk0_indx][bit_indx], ca_cal_per_chrnk_left[chrnk1_indx][bit_indx], MIN_ENDPT);
					right_pos_val = find_common_endpoint(ca_cal_per_chrnk_right[chrnk0_indx][bit_indx], ca_cal_per_chrnk_right[chrnk1_indx][bit_indx], MAX_ENDPT);
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
				panic("Memory CA Calibration: ca_bit_center[%d] = (%d) < min_ca_bit_center = %d\n", bit_indx, ca_bit_center[bit_indx], min_ca_bit_center);
			
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

		for (bit_indx = 0; bit_indx < CA_NUM_BITS; bit_indx++) {
			// Make sure deskew value programmed is not negative and is <= MAX_DESKEW_PROGRAMMED
			if ((ca_bit_deskew[bit_indx] < 0) || (ca_bit_deskew[bit_indx] > MAX_DESKEW_PROGRAMMED))
				panic("Memory CA Calibration: ca_bit_deskew[%d] = %d invalid\n", bit_indx, ca_bit_deskew[bit_indx]);
		}
		
		// Push the remainder of the delay to CK signals (if adj_CaBitCenterPoint_val_data was clamped to camdll)
		amp_push_ctl_out(ch, cs_adj_val);
		
		// Program the SDLL with the adjusted min value
		amp_push_casdll_out(ch, adj_ca_bit_center);
		
		// Program the CA Deskew values for each bit
		for (bit_indx = 0; bit_indx < CA_NUM_BITS; bit_indx++) {
			rAMP_CADESKEW_CTRL(ch, bit_indx) = ca_bit_deskew[bit_indx];
		}
	}
}

// Loop through PerBitDeskewCode ranges for rddq until failing points for each byte (& bit) are found.
static void find_rddqcal_right_side_failing_point(uint32_t ch, uint32_t rnk, bool after_wrddqcal)
{
	uint32_t dq_deskew;
	uint32_t all_bits_fail;
	uint32_t bits_fail_b[DQ_NUM_BYTES] = { 0 };
	uint32_t rddqcalresult;
	uint32_t mask_b[DQ_NUM_BYTES];
	int32_t start_b[DQ_NUM_BYTES];
	uint32_t byte, bit;
	
	all_bits_fail = 0;
	dq_deskew = 0;
	
	// set the rddq sdll to negative dqmdllcode
	mdllcode[ch][AMP_DQ] = (rAMP_MDLLCODE(AMP_DQ, ch) & DLLVAL_BITS);
	for (byte = 0; byte < DQ_NUM_BYTES; byte++) {
		amp_set_rddq_sdll(ch, byte, (1 << SIGN_BIT_POS) +  mdllcode[ch][AMP_DQ]);
		
		// initialize the mask for each byte lane
		mask_b[byte] = 0xFF << (byte * 8);
	}
	
	rddqcalresult = 0xFFFFFFFF;
	
	// PerBit Deskew lines cannot be pushed beyond DQ_MAX_DESKEW_PER_BIT value
	do {
		for (byte = 0; byte < DQ_NUM_BYTES; byte++) {
			if (bits_fail_b[byte] == 0) {
				for (bit = 0; bit < DQ_NUM_BITS_PER_BYTE; bit++)
					rAMP_RDDQDESKEW_CTRL(ch, byte, bit) = dq_deskew;
			}
		}
				
		// Call Basic run Dq Cal Commands
		if (after_wrddqcal == false) {
			amp_run_rddqcal(ch);
			rddqcalresult &= rAMP_DQCALRESULT(ch);
		} else {
			rddqcalresult &= wr_rd_pattern_result(ch, rnk, dq_deskew);
		}
		
		for (byte = 0; byte < DQ_NUM_BYTES; byte++) {
			// if all bits haven't failed yet and this run shows all bits failing, we have found the failing point for this byte
			if ((bits_fail_b[byte] == 0) && ((rddqcalresult & mask_b[byte]) == 0)) {
				bits_fail_b[byte] = 1;
				start_b[byte] = dq_deskew;
			}
		}
		
		all_bits_fail = bits_fail_b[0] & bits_fail_b[1] & bits_fail_b[2] & bits_fail_b[3];
		
		if (all_bits_fail == 1) {
			// If failing point has been found for all bits, find the passing point now
			find_rddqcal_right_side_passing_point(ch, rnk, start_b, after_wrddqcal);
		} else {
			// To find right failing point, make more negative adjustment to the sdll (same as incrementing deskew)
			dq_deskew = dq_deskew + COARSE_STEP_SZ;
		}
		
	} while ((dq_deskew <= DQ_MAX_DESKEW_PER_BIT) && (all_bits_fail == 0));
	
	if ((dq_deskew > DQ_MAX_DESKEW_PER_BIT) && (all_bits_fail == 0)) {
		// print error message as Right Failing Point cannot be found
		dprintf(DEBUG_INFO, "Memory Rddq cal: Right side failing point not found, max deskew limit reach for channel %d", ch);

		// Assume failure at this setting, and continue to passing point
		for (byte = 0; byte < DQ_NUM_BYTES; byte++) {
			// if all bits haven't failed yet, assign start_b for this byte to current reg setting
			if (bits_fail_b[byte] == 0)
				start_b[byte] = dq_deskew - COARSE_STEP_SZ;
		}
		
		find_rddqcal_right_side_passing_point(ch, rnk, start_b, after_wrddqcal);
	}
	
	// Reset deskew for all bits to 0
	for (byte = 0; byte < DQ_NUM_BYTES; byte++)
		for (bit = 0; bit < DQ_NUM_BITS_PER_BYTE; bit++)
			rAMP_RDDQDESKEW_CTRL(ch, byte, bit) = 0;
}

// Purpose of this function is to start from right side failing point and find locations for every DQ bit
// until the start of passing window for that bit is found
// Save all this locations to compute the center of window
static void find_rddqcal_right_side_passing_point(uint32_t ch, uint32_t rnk, int32_t *start_b, bool after_wrddqcal)
{
	uint32_t chrnk_indx;
	bool switch_from_dqstodq, max_tap_value_reached;
	int32_t tap_value_b[DQ_NUM_BYTES];
	uint32_t BitPass[DQ_TOTAL_BITS] = { 0 };
	uint32_t SolidBitPass[DQ_TOTAL_BITS] = { 0 };
	uint32_t rddqcalresult;
	uint32_t all_bits_pass;
	uint32_t all_bits_pass_b[DQ_NUM_BYTES] = { 0 };
	uint32_t step_incr;
	uint32_t bit_indx, byte;
	int32_t dqmdllcode;
	int32_t saved_val;
	
	chrnk_indx = (ch * AMC_NUM_RANKS) + rnk;
	all_bits_pass = 0;
	switch_from_dqstodq = false;
	max_tap_value_reached = false;
	rddqcalresult = 0xFFFFFFFF;
	step_incr = FINER_STEP_SZ;
	dqmdllcode = mdllcode[ch][AMP_DQ];

	for (byte = 0; byte < DQ_NUM_BYTES; byte++) {
		tap_value_b[byte] = start_b[byte];
	}
	
	// Moving Right to Left to find point where each bit turns from FAIL TO PASS
	do {
		if (switch_from_dqstodq == false) {
			// continue to update per bit deskew until all bits pass for each byte lane
			for (byte = 0; byte < DQ_NUM_BYTES; byte++) {
				if (all_bits_pass_b[byte] == 0) {
					for (bit_indx = 0; bit_indx < DQ_NUM_BITS_PER_BYTE; bit_indx++)
						rAMP_RDDQDESKEW_CTRL(ch, byte, bit_indx) = tap_value_b[byte];
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
			rddqcalresult = rAMP_DQCALRESULT(ch);
		} else {
			rddqcalresult = wr_rd_pattern_result(ch, rnk, tap_value_b[0]);
		}
		
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
					
					if (switch_from_dqstodq == false)
						// consider mdllcode as '0' since sdll is set to -mdllcode
						saved_val = -1 * (tap_value_b[byte] + dqmdllcode);
					else
						saved_val = tap_value_b[byte];
						
					rddq_cal_per_chrnk_right[chrnk_indx][bit_indx] = saved_val;
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
			
			// check for all bytes reaching 0 on the tap value (could be deskew or sdll)
			int32_t all_bytes_tap = tap_value_b[0];
			for (byte = 1; (byte < DQ_NUM_BYTES) && (all_bytes_tap == 0); byte++) {
				all_bytes_tap += tap_value_b[byte];
			}
			
			// if the tap_value for all bytes has reached 0 on the deskew, make the transition to SDLL
			if ((all_bytes_tap == 0) && (switch_from_dqstodq == false)) {
				switch_from_dqstodq = true;
				
				for (byte = 0; byte < DQ_NUM_BYTES; byte++) {
					tap_value_b[byte] = (rAMP_DQSDLLCTRL_RD(ch, byte) & DLLVAL_BITS);
					tap_value_b[byte] = OFFSET_TO_INT(tap_value_b[byte]);
				}
				
			}
			
			// To find right side passing point, add less negative adjustment to mdll (same as decrementing deskew)
			
			// For deskew taps, we just decrement by step_incr if we haven't reached 0 yet
			if (switch_from_dqstodq == false) {
				for (byte = 0; byte < DQ_NUM_BYTES; byte++)
					if (tap_value_b[byte] > 0)
						tap_value_b[byte] -= step_incr;
			} else {
				// For sdll taps, increment it
				for (byte = 0; byte < DQ_NUM_BYTES; byte++) {
					if (all_bits_pass_b[byte] == 0) {
						tap_value_b[byte] += step_incr;
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
					panic("Memory rddq calibration: Unable to find right side passing point, max tap value reached");
				
				break;
			}
		}
		
	} while ((!max_tap_value_reached) && (all_bits_pass == 0));
}

// Purpose of this function is to start push DQS out till left side failing point of Data window is found
static void find_rddqcal_left_side_failing_point(uint32_t ch, uint32_t rnk, bool after_wrddqcal)
{
	int32_t rddqsdll[DQ_NUM_BYTES];
	uint32_t rddqcalresult;
	uint32_t all_bits_fail;
	uint32_t all_bits_fail_b[DQ_NUM_BYTES] = { 0 };
	uint32_t step_incr;
	uint32_t mask_b[DQ_NUM_BYTES];
	int32_t start_b[DQ_NUM_BYTES];
	uint32_t byte;
	bool max_tap_value_reached = false;
		
	all_bits_fail = 0;
	rddqcalresult = 0xFFFFFFFF;
	step_incr = COARSE_STEP_SZ;
	
	for (byte = 0; byte < DQ_NUM_BYTES; byte++) {
		// initialize the mask for each byte lane
		mask_b[byte] = 0xFF << (byte * 8);
		
		// Get the starting values for RD DQS SDLL
		rddqsdll[byte] = rAMP_DQSDLLCTRL_RD(ch, byte);
		rddqsdll[byte] = OFFSET_TO_INT(rddqsdll[byte]);
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
			rddqcalresult &= rAMP_DQCALRESULT(ch);
		} else {
			rddqcalresult &= wr_rd_pattern_result(ch, rnk, rddqsdll[0]);
		}
		
		// If the result of all bits in this byte show a fail, record this as the failing point
		all_bits_fail = 1;
		for (byte = 0; byte < DQ_NUM_BYTES; byte++) {
			if ((all_bits_fail_b[byte] == 0) && ((rddqcalresult & mask_b[byte]) == 0)) {
				all_bits_fail_b[byte] = 1;
				start_b[byte] = rddqsdll[byte];
			}
			
			all_bits_fail &= all_bits_fail_b[byte];
		}

		// all bytes fail, call the function to find left passing point
		if (all_bits_fail == 1) {
			find_rddqcal_left_side_passing_point(ch, rnk, start_b, after_wrddqcal);
		} else {
			// if the byte has not yet failed, find the next sdll value to be set
			for (byte = 0; byte < DQ_NUM_BYTES; byte++) {
				if (all_bits_fail_b[byte] == 0) {
					rddqsdll[byte] += step_incr;
				}
			}
		}
		
		for (byte = 0; byte < DQ_NUM_BYTES; byte++) {
			// none of the previous bytes reached max_tap_value, then update the boolean
			if (!max_tap_value_reached) {
				max_tap_value_reached = (rddqsdll[byte] > MAX_SDLL_VAL);
				
				if (max_tap_value_reached) {
					dprintf(DEBUG_INFO, "Memory rddq calibration: Unable to find left failing point, max tap value reached for ch %d byte %d", ch, byte);
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
				
				find_rddqcal_left_side_passing_point(ch, rnk, start_b, after_wrddqcal);
			}
		}
	} while ((!max_tap_value_reached) && (all_bits_fail == 0));
}

// Purpose of this function is to start from left side failing point and find passing locations for every DQ bit on left side of window
// Save all the locations to compute the center of window later
// To find left passing point, move to the right from the failing point, which means keep adding more negative adjustment to mdll
static void find_rddqcal_left_side_passing_point(uint32_t ch, uint32_t rnk, int32_t *start_b, bool after_wrddqcal)
{
	uint32_t chrnk_indx;
	bool max_tap_value_reached = false;
	int32_t tap_value_b[DQ_NUM_BYTES];
	uint32_t BitPass[DQ_TOTAL_BITS] = { 0 };
	uint32_t SolidBitPass[DQ_TOTAL_BITS] = { 0 };
	uint32_t rddqcalresult;
	uint32_t all_bits_pass;
	uint32_t all_bits_pass_b[DQ_NUM_BYTES] = { 0 };
	uint32_t step_incr;
	uint32_t bit_indx, byte;
	
	chrnk_indx = (ch * AMC_NUM_RANKS) + rnk;
	all_bits_pass = 0;
	rddqcalresult = 0xFFFFFFFF;
	step_incr = FINER_STEP_SZ;
	
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
			rddqcalresult = rAMP_DQCALRESULT(ch);
		} else {
			rddqcalresult = wr_rd_pattern_result(ch, rnk, tap_value_b[0]);
		}
		
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
					
					rddq_cal_per_chrnk_left[chrnk_indx][bit_indx] = tap_value_b[byte];
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
				if (all_bits_pass_b[byte] == 0) {
					tap_value_b[byte] -= step_incr;
				}
			}
		}
		
		// check for end of loop condition
		for (byte = 0; byte < DQ_NUM_BYTES; byte++) {
			if (!max_tap_value_reached)
				max_tap_value_reached = (tap_value_b[byte] < (-1 * MAX_SDLL_VAL));
			
			if (max_tap_value_reached) {
				if (all_bits_pass == 0)
				    panic("Memory rddq calibration: Unable to find left passing point, max tap value reached");
				break;
			}
			
			// panic if we get beyond -dqmdllcode, since we really shouldn't have to go that far
			if ((all_bits_pass == 0) && (tap_value_b[byte] < (-1 * mdllcode[ch][AMP_DQ])))
				panic("Memory rddq calibration: Not yet found left passing point but SDLL < -dqmdllcode for ch %d byte %d", ch, byte);
		}
	} while ((!max_tap_value_reached) && (all_bits_pass == 0));
}

static void rddq_program_final_values(bool after_wrddqcal)
{
	uint32_t ch, bit_indx, byte;
	uint32_t chrnk0_indx, chrnk1_indx;
	int32_t rddq_bit_center[DQ_TOTAL_BITS];
	int32_t rddq_bit_deskew[DQ_TOTAL_BITS];
	int32_t left_pos_val;
	int32_t right_pos_val;
	int32_t max_rddq_center[DQ_NUM_BYTES];
	int32_t dqmdllcode;
	
	for (ch = 0; ch < AMC_NUM_CHANNELS; ch++) {
		dqmdllcode = mdllcode[ch][AMP_DQ];
		
		// find the center point of passing window for each bit over all ranks
		for (bit_indx = 0; bit_indx < DQ_TOTAL_BITS; bit_indx++) {
		
			chrnk0_indx = (ch * AMC_NUM_RANKS) + 0;
			left_pos_val = rddq_cal_per_chrnk_left[chrnk0_indx][bit_indx];
			right_pos_val = rddq_cal_per_chrnk_right[chrnk0_indx][bit_indx];
			
			if (AMC_NUM_RANKS > 1) {
				
				chrnk1_indx = (ch * AMC_NUM_RANKS) + 1;
				
				// find the endpoint that covers both ranks
				left_pos_val = find_common_endpoint(rddq_cal_per_chrnk_left[chrnk0_indx][bit_indx],
								    rddq_cal_per_chrnk_left[chrnk1_indx][bit_indx],
								    MIN_ENDPT);
				right_pos_val = find_common_endpoint(rddq_cal_per_chrnk_right[chrnk0_indx][bit_indx],
								     rddq_cal_per_chrnk_right[chrnk1_indx][bit_indx],
								     MAX_ENDPT);
			}
			
			// find center of the eye for this bit
			rddq_bit_center[bit_indx] = find_center_of_eye(left_pos_val, right_pos_val);
		}
		
		// <rdar://problem/13439594>, <rdar://problem/13888162> Need additional shift to DQ offset
		if (after_wrddqcal) {
			
			int8_t signed_byte_center_point[DQ_TOTAL_BITS];
			
			// convert to signed bytes first as required by shift function
			for (bit_indx = 0; bit_indx < DQ_TOTAL_BITS; bit_indx++)
				signed_byte_center_point[bit_indx] = (int8_t) rddq_bit_center[bit_indx];
			
			// call platform specific amc routine to apply apropriate shifts depending on DRAM vendor
			amc_dram_shift_dq_offset(signed_byte_center_point, DQ_TOTAL_BITS);
			
			// convert shifted signed bytes back to signed ints
			for (bit_indx = 0; bit_indx < DQ_TOTAL_BITS; bit_indx++)
				rddq_bit_center[bit_indx] = (int32_t) signed_byte_center_point[bit_indx];
		}
		
		// initialize the max centerpoint to the 1st bit's center point in each byte lane
		for (byte = 0; byte < DQ_NUM_BYTES; byte++)
			max_rddq_center[byte] = rddq_bit_center[byte * DQ_NUM_BITS_PER_BYTE];

		// Find the maximum CenterPoint per byte lane given each bit's center point
		for (bit_indx=0; bit_indx < DQ_TOTAL_BITS; bit_indx++) {
			
			byte = bit_indx >> 3; // bit_indx / DQ_NUM_BITS_PER_BYTE
			
			// if this bit's center point is greater than current max, make it the new max (we'll program this to sdll, and other values will require deskew)
			if (rddq_bit_center[bit_indx] > max_rddq_center[byte])
				max_rddq_center[byte] = rddq_bit_center[bit_indx];
		}
		
		// if the max for each byte lane is < -dqmdllcode, clamp it to -dqmdllcode (the remainder will go on per bit deskew)
		for (byte = 0; byte < DQ_NUM_BYTES; byte++) {
			
			if (max_rddq_center[byte] < (-1 * dqmdllcode))
				max_rddq_center[byte] = (-1 * dqmdllcode);
		}
		
		// Compute the individual deskew values: any bits with center point < max for its byte lane will require deskew
		// Each bit's center is guaranteed to be <= max for its byte lane
		// Deskewing means adding more negative adjustment for this bit in addition to the sdll, which is clamped on the negative side to -dqmdllcode
		for (bit_indx = 0; bit_indx < DQ_TOTAL_BITS; bit_indx++) {
			
			byte = bit_indx >> 3; // bit_indx / DQ_NUM_BITS_PER_BYTE
			
			if (rddq_bit_center[bit_indx] > max_rddq_center[byte])
				panic("Memory Rddq calibration: rddq_bit_center[%d] = %d > max_rddq_center[%d] = %d\n", bit_indx, rddq_bit_center[bit_indx], byte, max_rddq_center[byte]);
			
			rddq_bit_deskew[bit_indx] = max_rddq_center[byte] - rddq_bit_center[bit_indx];

			if ((rddq_bit_deskew[bit_indx] < 0) || (rddq_bit_deskew[bit_indx] > DQ_MAX_DESKEW_PER_BIT))
				panic("Memory Rddq calibration: rddq_bit_deskew[%d] = %d invalid\n", bit_indx, rddq_bit_deskew[bit_indx]);
		}
		
		// Program the SDLL and deskew per bit for each byte lane
		for (byte = 0; byte < DQ_NUM_BYTES; byte++) {
			amp_set_rddq_sdll(ch, byte, INT_TO_OFFSET(max_rddq_center[byte]));
			
			// per bit deskew for this byte lane
			for (bit_indx = 0; bit_indx < DQ_NUM_BITS_PER_BYTE; bit_indx++) {
				rAMP_RDDQDESKEW_CTRL(ch, byte, bit_indx) = rddq_bit_deskew[(byte * DQ_NUM_BITS_PER_BYTE) + bit_indx];
			}
		}
	} // for (ch = 0; ch < AMC_NUM_CHANNELS; ch++)
}

static uint32_t wrlvl_encode_dlyval(uint32_t ch, uint32_t phy_type, uint32_t val) {
	
	uint32_t ret_val, mdll;
	
	mdll = mdllcode[ch][phy_type];
	
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

static uint32_t wrlvl_encode_clk90dly(uint32_t ch, uint32_t val) {
	
	uint32_t ret_val, mdll;
	
	mdll = mdllcode[ch][AMP_DQ];
	
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

static void push_wrlvl_to_0s_region(uint32_t ch, uint32_t rnk)
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
		wrlvldata = rAMP_DQWRLVLDATA(ch);
		
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
				panic("Memory Wrlvl calibration: CA sdll reached max tap value, yet all bytes not all 0s");
		} else {
		
			for (byte = 0; byte < DQ_NUM_BYTES; byte++) {
				if (dqwrlvlcode[byte] == MAX_DQWRLVL_CODE) {
					if (wrlvlrun)
						panic("Memory Wrlvl calibration: DQ%d sdll reached max tap value, yet all bytes not all 0s", byte);
					max_tap_value_reached = true;
					break;
				}
			}
		}
	} while (wrlvlrun && !max_tap_value_reached);
}

// Keep incrementing dqsdll until the byte shows 1s again. This counters the casdll that was incremented previously in order to show 0s
static void find_wrlvl_0to1_transition(uint32_t ch, uint32_t rnk)
{
	uint32_t chrnk_indx, byte;
	uint32_t wrlvlrun, wrlvldata;
	bool max_tap_value_reached = false;
	uint32_t dqwrlvlcode[DQ_NUM_BYTES];
	uint32_t cawrlvlcode = rAMP_CAWRLVLSDLLCODE(ch);
	
	wrlvlrun = 0xF;
	wrlvldata = 0;
	
	for (byte = 0; byte < DQ_NUM_BYTES; byte++)
		dqwrlvlcode[byte] = rAMP_DQWRLVLSDLLCODE(ch, byte);
	
	chrnk_indx = (ch * AMC_NUM_RANKS) + rnk;
	
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
		
		wrlvldata = rAMP_DQWRLVLDATA(ch);
		
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
					panic("Memory Wrlvl calibration: DQ%d sdll reached max tap value, yet all bytes not all 1s", byte);
				max_tap_value_reached = true;
				break;
			}
		}
	} while (wrlvlrun && !max_tap_value_reached);
	
	// save the per byte codes for this channel and rank
	for (byte = 0; byte < DQ_NUM_BYTES; byte++)
		wrlvl_cal_per_chrnk_rise[chrnk_indx][byte] = dqwrlvlcode[byte];
	// in the "5th byte" entry, save the cawrlvl code
	wrlvl_cal_per_chrnk_rise[chrnk_indx][byte] = cawrlvlcode;
}

// Go back towards the 0s region (that was found earlier). Note: not trying to find the next edge, just the previous edge that was found already
static void find_wrlvl_1to0_transition(uint32_t ch, uint32_t rnk)
{
	uint32_t chrnk_indx, byte;
	uint32_t wrlvlrun, wrlvldata;
	bool max_tap_value_reached = false;
	uint32_t dqwrlvlcode[DQ_NUM_BYTES];
	uint32_t cawrlvlcode = rAMP_CAWRLVLSDLLCODE(ch);
	bool incr_cawrlvl = false;
	
	chrnk_indx = (ch * AMC_NUM_RANKS) + rnk;
	wrlvlrun = 0xF;

	// jump ahead by SOLID_PASS_DETECT into the 1s region
	for (byte = 0; byte < DQ_NUM_BYTES; byte++) {
		dqwrlvlcode[byte] = rAMP_DQWRLVLSDLLCODE(ch, byte);
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
		
		wrlvldata = rAMP_DQWRLVLDATA(ch);
		
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
				panic("Memory Wrlvl calibration: max tap value reached, yet all bytes not back to 0s");
		}
		
	} while (wrlvlrun && !max_tap_value_reached);
	
	// save the per byte codes for this channel and rank
	for (byte = 0; byte < DQ_NUM_BYTES; byte++)
		wrlvl_cal_per_chrnk_fall[chrnk_indx][byte] = dqwrlvlcode[byte];

	// in the "5th byte" entry, save the cawrlvl code
	wrlvl_cal_per_chrnk_fall[chrnk_indx][byte] = cawrlvlcode;
}

static void wrlvl_program_final_values(void)
{
	uint32_t ch, chrnk0_indx, chrnk1_indx;
	uint32_t rank_rise_val[AMP_MAX_RANKS_PER_CHAN], rank_fall_val[AMP_MAX_RANKS_PER_CHAN];
	uint32_t edge_pos[AMP_MAX_RANKS_PER_CHAN];
	uint32_t common_edge_pos, min_edge_pos;
	uint32_t byte;
	uint32_t saved_val[DQ_NUM_BYTES + 1];
	
	for (ch = 0; ch < AMC_NUM_CHANNELS; ch++) {
		// we go upto DQ_NUM_BYTES + 1 to also take into account the cawrlvlcode that is stored in the 5th element
		for (byte = 0; byte < (DQ_NUM_BYTES + 1); byte++) {
			
			// Rank 0
			chrnk0_indx = (ch * AMC_NUM_RANKS) + 0;
			rank_rise_val[0] = wrlvl_cal_per_chrnk_rise[chrnk0_indx][byte];
			rank_fall_val[0] = wrlvl_cal_per_chrnk_fall[chrnk0_indx][byte];
			// average of 2 values is the edge for this rank
			edge_pos[0] = (rank_rise_val[0] + rank_fall_val[0]) >> 1;
			common_edge_pos = edge_pos[0];

			// Adjust for Dual ranks
			if (AMC_NUM_RANKS > 1) {
				chrnk1_indx = (ch * AMC_NUM_RANKS) + 1;
				rank_rise_val[1] = wrlvl_cal_per_chrnk_rise[chrnk1_indx][byte];
				rank_fall_val[1] = wrlvl_cal_per_chrnk_fall[chrnk1_indx][byte];
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
		
		// We'll subtract the min from all 5 sdlls, including ca
		// so the byte sdlls which are in opposite direction also need to be asjusted
		for (byte = 0; byte < (DQ_NUM_BYTES + 1); byte++) {
			saved_val[byte] -= min_edge_pos;
			
			// Program the values into the registers
			if (byte == DQ_NUM_BYTES) {
				// cawrlvl (use dlysel, which will require phyupdt and forceckelow)
				amp_set_cawrlvl_sdll(ch, saved_val[byte], true);
			} else {
				// dqwrlvl (use dlysel, which will require phyupdt and forceckelow)
				amp_set_dqwrlvl_sdll(ch, byte, saved_val[byte], true);
			}
		}
	}
}


// Keep pushing out WRDQS lines (controlled by WRDQM registers, oddly) until right side failing point is found
static void find_wrdqcal_right_side_failing_point(uint32_t ch, uint32_t rnk)
{
	uint32_t push_dqs_out;
	uint32_t all_bits_fail;
	uint32_t bits_fail_b[DQ_NUM_BYTES] = { 0 };
	uint32_t wrdqcalresult_cumulative;
	uint32_t mask_b[DQ_NUM_BYTES];
	int32_t start_b[DQ_NUM_BYTES];
	uint32_t byte;
	uint32_t cawrlvlcode = rAMP_CAWRLVLSDLLCODE(ch);
	
	all_bits_fail = 0;
	push_dqs_out = 0;
	wrdqcalresult_cumulative = 0xFFFFFFFF;
	
	// set the wrdq sdll to negative dqmdllcode
	mdllcode[ch][AMP_DQ] = (rAMP_MDLLCODE(AMP_DQ, ch) & DLLVAL_BITS);
	for (byte = 0; byte < DQ_NUM_BYTES; byte++) {
		amp_set_wrdq_sdll(ch, byte, (-1 * mdllcode[ch][AMP_DQ]));
		
		// initialize the mask for each byte lane
		mask_b[byte] = 0xFF << (byte * 8);
	}
	
	do {
		// NOTE: When DQS are pushed out then - cawrlvl sdll needs to be pushed out as well with equal taps
		// can use dlysel (with phyupdt and forceckelow)
		amp_set_cawrlvl_sdll(ch, cawrlvlcode + push_dqs_out, true);
	
		// Keep pushing per bit DQS (controlled by DM regs, oddly) out until all bytes start to fail
		for (byte = 0; byte < DQ_NUM_BYTES; byte++)
			rAMP_WRDMDESKEW_CTRL(ch, byte) = push_dqs_out;
		
		// Perform the WrDq calibration with PRBS patterns
		wrdqcalresult_cumulative &= wr_rd_pattern_result(ch, rnk, push_dqs_out);
		
		all_bits_fail = 1;
		for (byte = 0; byte < DQ_NUM_BYTES; byte++) {
			if (bits_fail_b[byte] == 0) {
				// if all bits fail for this byte for the 1st time, we've found the right failing point
				if ((wrdqcalresult_cumulative & mask_b[byte]) == 0) {
					bits_fail_b[byte] = 1;
					start_b[byte] = push_dqs_out;
				}
			}
			
			all_bits_fail &= bits_fail_b[byte];
		}
		
		// if all bits in all bytes fail, find the right passing point
		if (all_bits_fail == 1) {
			find_wrdqcal_right_side_passing_point(ch, rnk, start_b);
		} else {
			// increase the deskew since all bits are not yet failing
			push_dqs_out += COARSE_STEP_SZ;
		}
		
	} while ((push_dqs_out <= DQ_MAX_DESKEW_PER_BIT) && (all_bits_fail == 0));
	
	if ((push_dqs_out > DQ_MAX_DESKEW_PER_BIT) && (all_bits_fail == 0)) {
		// Right Failing Point cannot be found
		dprintf(DEBUG_INFO, "Memory Wrdq calibration: Max deskew reached, but right failing point not found for ch %d", ch);
		
		// Assume failure point is current reg setting
		for (byte = 0; byte < DQ_NUM_BYTES; byte++) {
			if (bits_fail_b[byte] == 0)
				start_b[byte] = push_dqs_out - COARSE_STEP_SZ;
		}
		
		// conitnue to passing point
		find_wrdqcal_right_side_passing_point(ch, rnk, start_b);
	}
	
	// Before quitting restore the cawrlvlsdll and per byte deskew back to original values.
	// can use dlysel (with phyupdt and forceckelow)
	amp_set_cawrlvl_sdll(ch, cawrlvlcode, true);
	for (byte = 0; byte < DQ_NUM_BYTES; byte++)
		rAMP_WRDMDESKEW_CTRL(ch, byte) = 0;
}

// Keep decreasing per byte deskew until right passing point is found
static void find_wrdqcal_right_side_passing_point(uint32_t ch, uint32_t rnk, int32_t *start_b)
{
	uint32_t chrnk_indx;
	bool switch_from_dqstodq, max_tap_value_reached;
	int32_t tap_value_b[DQ_NUM_BYTES];
	uint32_t BitPass[DQ_TOTAL_BITS] = { 0 };
	uint32_t SolidBitPass[DQ_TOTAL_BITS] = { 0 };
	uint32_t wrdqcalresult;
	uint32_t all_bits_pass;
	uint32_t all_bits_pass_b[DQ_NUM_BYTES] = { 0 };
	uint32_t step_incr;
	uint32_t bit_indx, byte;
	int32_t dqmdllcode, max_tap_value;
	int32_t saved_val, max_value;
	uint32_t cawrlvlcode = rAMP_CAWRLVLSDLLCODE(ch);
	
	chrnk_indx = (ch * AMC_NUM_RANKS) + rnk;
	all_bits_pass = 0;
	switch_from_dqstodq = false;
	max_tap_value_reached = false;
	step_incr = FINER_STEP_SZ;
	dqmdllcode = mdllcode[ch][AMP_DQ];
	max_tap_value = dqmdllcode - DELIMIT_POS_ADJ_WRDQSDLL + DQ_MAX_DESKEW_PER_BIT;
	
	// initialize tap_values to max of all bytes' start values
	max_value = start_b[0];
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
			for (byte = 0; byte < DQ_NUM_BYTES; byte++)
				rAMP_WRDMDESKEW_CTRL(ch, byte) = tap_value_b[byte];
		} else {
			// adjust wrdq sdll until all bits pass for each byte lane
			for (byte = 0; byte < DQ_NUM_BYTES; byte++) {
				if (all_bits_pass_b[byte] == 0)
					amp_set_wrdq_sdll(ch, byte, tap_value_b[byte]);
			}
		}
		
		// Send the PRBS patterns and read them back to see which bits are passing or failing
		wrdqcalresult = wr_rd_pattern_result(ch, rnk, tap_value_b[0] + tap_value_b[1] + tap_value_b[2] + tap_value_b[3]);
				
		// Make sure that each Bit sees a transition from 0 (fail) to 1 (pass) on wrdqcalresult
		for (bit_indx = 0; bit_indx < DQ_TOTAL_BITS; bit_indx++) {
			
			byte = bit_indx >> 3; // bit_indx / DQ_NUM_BITS_PER_BYTE
			
			// Check if this bit passed during the calibration (not necessarily for first time)
			if ((BitPass[bit_indx] == 0) && ((wrdqcalresult & (1 << bit_indx)) != 0)) {
				// Has this bit passed SOLID_PASS_DETECT number of times? Then consider it done
				if (SolidBitPass[bit_indx] == SOLID_PASS_DETECT) {
					BitPass[bit_indx] = 1;
				} else if (SolidBitPass[bit_indx] > 0) {
					SolidBitPass[bit_indx] = SolidBitPass[bit_indx] + 1;
				} else {
					// bit passed for the first time, record this value in the global array as the right edge
					SolidBitPass[bit_indx] = SolidBitPass[bit_indx] + 1;
					
					byte = bit_indx >> 3; // bit_indx / DQ_NUM_BITS_PER_BYTE
					
					if (switch_from_dqstodq == false)
						// consider mdllcode as '0' since sdll is set to -mdllcode
						saved_val = -1 * (tap_value_b[byte] + dqmdllcode);
					else
						saved_val = tap_value_b[byte];
					
					wrdq_cal_per_chrnk_right[chrnk_indx][bit_indx] = saved_val;
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
			
			// check for all bytes reaching 0 on the tap value (could be deskew or sdll)
			uint32_t all_bytes_tap = tap_value_b[0];
			for (byte = 1; (byte < DQ_NUM_BYTES) && (all_bytes_tap == 0); byte++) {
				all_bytes_tap += tap_value_b[byte];
			}
			
			// if the tap_value for all bytes has reached 0 on the deskew, make the transition to SDLL
			if ((all_bytes_tap == 0) && (switch_from_dqstodq == false)) {
				switch_from_dqstodq = true;
				
				for (byte = 0; byte < DQ_NUM_BYTES; byte++) {
					tap_value_b[byte] = (rAMP_DQSDLLCTRL_WR(ch, byte) & DLLVAL_BITS);
					tap_value_b[byte] = OFFSET_TO_INT(tap_value_b[byte]);
				}
				
			}
			
			// To find right side passing point, add less negative adjustment to mdll (same as decrementing deskew)
			
			// For deskew taps, we just decrement by step_incr if we haven't reached 0 yet
			// Note: All deskew taps will reach 0 at the same time since their start values are equal, and they are decremented regardless of pass or fail
			if (switch_from_dqstodq == false) {
				
				// Also decrement cawrlvlsdllcode along with tap_value_b
				if (tap_value_b[0] > 0)
					cawrlvlcode -= step_incr;
				
				for (byte = 0; byte < DQ_NUM_BYTES; byte++)
					if (tap_value_b[byte] > 0)
						tap_value_b[byte] -= step_incr;
			} else {
				// For sdll taps, increment
				for (byte = 0; byte < DQ_NUM_BYTES; byte++) {
					if (all_bits_pass_b[byte] == 0) {
						tap_value_b[byte] += step_incr;
					}
				}
			}
		}

		// trigger for loop to end if any of the bytes reach max tap value
		for (byte = 0; byte < DQ_NUM_BYTES; byte++) {
			if (!max_tap_value_reached)
				max_tap_value_reached = (tap_value_b[byte] > max_tap_value);
			
			if (max_tap_value_reached) {
				if (all_bits_pass == 0)
					panic("Memory wrdq calibration: Unable to find right side passing point for channel %d, max tap value reached. start_b[] = {0x%x 0x%x 0x%x 0x%x}", ch, start_b[0], start_b[1], start_b[2], start_b[3]);
			}
		}
		
	} while ((!max_tap_value_reached) && (all_bits_pass == 0));
}

// To find left failing point, keep adding less negative adjustment to mdll
static void find_wrdqcal_left_side_failing_point(uint32_t ch, uint32_t rnk)
{
	int32_t wrdqsdll[DQ_NUM_BYTES];
	uint32_t wrdqcalresult;
	uint32_t all_bits_fail;
	uint32_t all_bits_fail_b[DQ_NUM_BYTES] = { 0 };
	uint32_t step_incr;
	uint32_t mask_b[DQ_NUM_BYTES];
	int32_t start_b[DQ_NUM_BYTES];
	uint32_t byte;
	bool max_tap_value_reached = false;
	int32_t dqmdllcode, max_tap_value;
	
	dqmdllcode = mdllcode[ch][AMP_DQ];
	max_tap_value = dqmdllcode - DELIMIT_POS_ADJ_WRDQSDLL + DQ_MAX_DESKEW_PER_BIT;
	
	all_bits_fail = 0;
	wrdqcalresult = 0xFFFFFFFF;
	step_incr = COARSE_STEP_SZ;
	
	for (byte = 0; byte < DQ_NUM_BYTES; byte++) {
		// initialize the mask for each byte lane
		mask_b[byte] = 0xFF << (byte * 8);
		
		// Get the starting values for WR DQS SDLL
		wrdqsdll[byte] = rAMP_DQSDLLCTRL_WR(ch, byte) & DLLVAL_BITS;
		wrdqsdll[byte] = OFFSET_TO_INT(wrdqsdll[byte]);
		
		
		// Add per-bit deskew to wrdqsdll[byte] if sdll reached mdll - DELIMIT_POS_ADJ_WRDQSDLL (otherwise, deskew should be 0)
		// At this point per-bit deskew should be the same for each bit in this byte. Use bit 0's deskew value
		wrdqsdll[byte] += rAMP_WRDQDESKEW_CTRL(ch, byte, 0);
	}
	
	// Start with sdll value for which right passing point was found, then increase (less negative) until all bits fail
	do {
		
		for (byte = 0; byte < DQ_NUM_BYTES; byte++) {
			// set the new sdll for this byte lane if all bits are not yet failing
			if (all_bits_fail_b[byte] == 0)
				amp_set_wrdq_sdll(ch, byte, wrdqsdll[byte]);
		}
		
		// Send the PRBS patterns and read them back to see which bits are passing or failing
		wrdqcalresult &= wr_rd_pattern_result(ch, rnk, wrdqsdll[0] + wrdqsdll[1] + wrdqsdll[2] + wrdqsdll[3]);
		
		// If the result of all bits in this byte show a fail, record this as the failing point
		all_bits_fail = 1;
		for (byte = 0; byte < DQ_NUM_BYTES; byte++) {
			if ((all_bits_fail_b[byte] == 0) && ((wrdqcalresult & mask_b[byte]) == 0)) {
				all_bits_fail_b[byte] = 1;
				start_b[byte] = wrdqsdll[byte];
			}
			
			all_bits_fail &= all_bits_fail_b[byte];
		}
		
		if (all_bits_fail == 1) {
			find_wrdqcal_left_side_passing_point (ch, rnk, start_b);
		} else {
			// if the byte has not yet failed, find the next sdll value to be set
			for (byte = 0; byte < DQ_NUM_BYTES; byte++) {
				if (all_bits_fail_b[byte] == 0) {
					wrdqsdll[byte] += step_incr;
				}
			}
		}
	
		for (byte = 0; byte < DQ_NUM_BYTES; byte++) {
			// none of the previous bytes reached max_tap_value, then update the boolean
			if (!max_tap_value_reached) {
				max_tap_value_reached = (wrdqsdll[byte] > max_tap_value);
				
				if (max_tap_value_reached) {
					dprintf(DEBUG_INFO, "Memory wrdq calibration: Unable to find left failing point, max tap value reached for ch %d byte %d", ch, byte);
					break;
				}
			}
		}
		
		if (max_tap_value_reached) {
			// Continue to passing point if any of the bytes reaches max value and not all bits are failing
			if (all_bits_fail == 0) {
				for (byte = 0; byte < DQ_NUM_BYTES; byte++) {
					if (all_bits_fail_b[byte] == 0)
						start_b[byte] = max_tap_value;
				}
				
				find_wrdqcal_left_side_passing_point(ch, rnk, start_b);
			}
		}
	} while ((!max_tap_value_reached) && (all_bits_fail == 0));
}

static void find_wrdqcal_left_side_passing_point(uint32_t ch, uint32_t rnk, int32_t *start_b)
{
	uint32_t chrnk_indx;
	bool max_tap_value_reached = false;
	int32_t tap_value_b[DQ_NUM_BYTES];
	uint32_t BitPass[DQ_TOTAL_BITS] = { 0 };
	uint32_t SolidBitPass[DQ_TOTAL_BITS] = { 0 };
	uint32_t wrdqcalresult;
	uint32_t all_bits_pass;
	uint32_t all_bits_pass_b[DQ_NUM_BYTES] = { 0 };
	uint32_t step_incr;
	uint32_t bit_indx, byte;
	int32_t dqmdllcode, max_tap_value;
	
	dqmdllcode = mdllcode[ch][AMP_DQ];
	max_tap_value = -1 * dqmdllcode;
	
	chrnk_indx = (ch * AMC_NUM_RANKS) + rnk;
	all_bits_pass = 0;
	wrdqcalresult = 0xFFFFFFFF;
	step_incr = FINER_STEP_SZ;
	
	for (byte = 0; byte < DQ_NUM_BYTES; byte++) {
		tap_value_b[byte] = start_b[byte];
	}
			
	// Finding Left side failing point on per bit level. Moving Left to Right (keep adding more negative adj to mdll) to find point where it turns from FAIL TO PASS
	do {
		// adjust wrdq sdll until all bits pass for each byte lane
		for (byte = 0; byte < DQ_NUM_BYTES; byte++) {
			if (all_bits_pass_b[byte] == 0)
				amp_set_wrdq_sdll(ch, byte, tap_value_b[byte]);
		}
		
		// Send the PRBS patterns and read them back to see which bits are passing or failing
		wrdqcalresult = wr_rd_pattern_result(ch, rnk, tap_value_b[0] + tap_value_b[1] + tap_value_b[2] + tap_value_b[3]);
		
		// Make sure that each Bit sees a transition from 0 (fail) to 1 (pass) on wrdqcalresult
		for (bit_indx = 0; bit_indx < DQ_TOTAL_BITS; bit_indx++) {
			// Check if this bit passed during the calibration (not necessarily for first time)
			if ((BitPass[bit_indx] == 0) && ((wrdqcalresult & (1 << bit_indx)) != 0)) {
				// Has this bit passed SOLID_PASS_DETECT number of times? Then consider it done
				if (SolidBitPass[bit_indx] == SOLID_PASS_DETECT) {
					BitPass[bit_indx] = 1;
				} else if (SolidBitPass[bit_indx] > 0) {
					SolidBitPass[bit_indx] = SolidBitPass[bit_indx] + 1;
				} else {
					// bit passed for the first time, record this value in the global array as the right edge
					SolidBitPass[bit_indx] = SolidBitPass[bit_indx] + 1;
					
					byte = bit_indx >> 3; // bit_indx / DQ_NUM_BITS_PER_BYTE
					
					wrdq_cal_per_chrnk_left[chrnk_indx][bit_indx] = tap_value_b[byte];
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
			// For sdll taps, increment for neg tap_val, decrement for positive
			for (byte = 0; byte < DQ_NUM_BYTES; byte++) {
				if (all_bits_pass_b[byte] == 0) {
					tap_value_b[byte] -= step_incr;
				}
			}
		}
		
		// check for end of loop condition
		for (byte = 0; byte < DQ_NUM_BYTES; byte++) {
			if (!max_tap_value_reached)
				max_tap_value_reached = (tap_value_b[byte] < max_tap_value);
			
			if (max_tap_value_reached) {
				if (all_bits_pass_b[byte] == 0)
					panic("Memory wrdq calibration: Unable to find left passing point, max tap value reached. start_b[] = {0x%x, 0x%x, 0x%x, 0x%x}", start_b[0], start_b[1], start_b[2], start_b[3]);
				break;
			}
		}
	} while ((!max_tap_value_reached) && (all_bits_pass == 0));
}

static void wrdq_program_final_values(void)
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
	
	for (ch = 0; ch < AMC_NUM_CHANNELS; ch++) {
		dqmdllcode = mdllcode[ch][AMP_DQ];
				
		// find the center point of passing window for each bit over all ranks
		for (bit_indx = 0; bit_indx < DQ_TOTAL_BITS; bit_indx++) {
			
			chrnk0_indx = (ch * AMC_NUM_RANKS) + 0;
			left_pos_val = wrdq_cal_per_chrnk_left[chrnk0_indx][bit_indx];
			right_pos_val = wrdq_cal_per_chrnk_right[chrnk0_indx][bit_indx];
			
			if (AMC_NUM_RANKS > 1) {
				
				chrnk1_indx = (ch * AMC_NUM_RANKS) + 1;
				
				// find the endpoint that covers both ranks
				left_pos_val = find_common_endpoint(wrdq_cal_per_chrnk_left[chrnk0_indx][bit_indx],
								    wrdq_cal_per_chrnk_left[chrnk1_indx][bit_indx],
								    MIN_ENDPT);
				right_pos_val = find_common_endpoint(wrdq_cal_per_chrnk_right[chrnk0_indx][bit_indx],
								     wrdq_cal_per_chrnk_right[chrnk1_indx][bit_indx],
								     MAX_ENDPT);
			}
			
			// find center of the eye for this bit
			wrdq_bit_center[bit_indx] = find_center_of_eye(left_pos_val, right_pos_val);
		}

		
		// <rdar://problem/13439594>, <rdar://problem/13888162> Need additional shift to DQ offset
		int8_t signed_byte_center_point[DQ_TOTAL_BITS];
			
		// convert to signed bytes first as required by the shift function
		for (bit_indx = 0; bit_indx < DQ_TOTAL_BITS; bit_indx++)
			signed_byte_center_point[bit_indx] = (int8_t) wrdq_bit_center[bit_indx];
			
		// call platform specific amc routine to apply apropriate shifts depending on DRAM vendor
		amc_dram_shift_dq_offset(signed_byte_center_point, DQ_TOTAL_BITS);
			
		// convert shifted signed bytes back to offset format
		for (bit_indx = 0; bit_indx < DQ_TOTAL_BITS; bit_indx++)
			wrdq_bit_center[bit_indx] = (int32_t) signed_byte_center_point[bit_indx];


		// initialize the min centerpoint to the 1st bit's center point in each byte lane
		for (byte = 0; byte < DQ_NUM_BYTES; byte++)
			min_wrdq_center[byte] = wrdq_bit_center[byte * DQ_NUM_BITS_PER_BYTE];
		
		// Find the min CenterPoint per byte lane given each bit's center point
		for (bit_indx=0; bit_indx < DQ_TOTAL_BITS; bit_indx++) {
			
			byte = bit_indx >> 3; // bit_indx / DQ_NUM_BITS_PER_BYTE
			
			// if this bit's center point is less than current min, make it the new min
			// if this bit's center point is less than current min, make it the new min
			if (wrdq_bit_center[bit_indx] < min_wrdq_center[byte])
				min_wrdq_center[byte] = wrdq_bit_center[bit_indx];
		}

		// for positive value, clamp it to mdllcode - DELIMIT_POS_ADJ_WRDQSDLL
		for (byte = 0; byte < DQ_NUM_BYTES; byte++) {
			if (min_wrdq_center[byte] > (dqmdllcode - DELIMIT_POS_ADJ_WRDQSDLL))
				min_wrdq_center[byte] = (dqmdllcode - DELIMIT_POS_ADJ_WRDQSDLL);
		}
		
		// Compute the individual deskew values: any bits with center point > min for its byte lane will require deskew
		// Each bit's center is guaranteed to be >= min for its byte lane
		// Deskewing means adding more positive adjustment for this bit in addition to the sdll, which is clamped on the negative side to -dqmdllcode
		// and clamped on the positive side to (mdllcode - DELIMIT_POS_ADJ_WRDQSDLL)
		for (bit_indx = 0; bit_indx < DQ_TOTAL_BITS; bit_indx++) {
			
			byte = bit_indx >> 3; // bit_indx / DQ_NUM_BITS_PER_BYTE
			
			if (wrdq_bit_center[bit_indx] < min_wrdq_center[byte])
				panic("Memory Wrdq Calibration: wrdq_bit_center[%d] = (%d) < min_wrdq_center[%d] = %d\n", bit_indx, wrdq_bit_center[bit_indx], byte, min_wrdq_center[byte]);
			
			wrdq_bit_deskew[bit_indx] = wrdq_bit_center[bit_indx] - min_wrdq_center[byte];
			
			// Make sure deskew value programmed is not negative and is <= DQ_MAX_DESKEW_PER_BIT
			if ((wrdq_bit_deskew[bit_indx] < 0) || (wrdq_bit_deskew[bit_indx] > DQ_MAX_DESKEW_PER_BIT))
				panic("Memory Wrdq Calibration: wrdq_bit_deskew[%d] = %d invalid\n", bit_indx, wrdq_bit_deskew[bit_indx]);
		}

		// if the min for each byte lane is < -dqmdllcode, we'll need to adjust/clamp it to -dqmdllcode
		for (byte = 0; byte < DQ_NUM_BYTES; byte++) {
			if (min_wrdq_center[byte] < (-1 * dqmdllcode)) {
				int32_t dqs_deskew = (-1 * dqmdllcode) - min_wrdq_center[byte];
				// <rdar://problem/14116888> put the remainder on DQS
				rAMP_WRDMDESKEW_CTRL(ch, byte) = dqs_deskew;
				min_wrdq_center[byte] = (-1 * dqmdllcode);
			}
		}
		
		// Program the SDLL and deskew per bit for each byte lane
		for (byte = 0; byte < DQ_NUM_BYTES; byte++) {
			amp_set_wrdq_sdll(ch, byte, min_wrdq_center[byte]);
				
			// init the min and max deskew values for each byte to the 1st bit in the byte
			min_dq_deskew_code = wrdq_bit_deskew[(byte * DQ_NUM_BITS_PER_BYTE)];
			max_dq_deskew_code = wrdq_bit_deskew[(byte * DQ_NUM_BITS_PER_BYTE)];
			// per bit deskew for this byte lane
			for (bit_indx = 0; bit_indx < DQ_NUM_BITS_PER_BYTE; bit_indx++) {
				rAMP_WRDQDESKEW_CTRL(ch, byte, bit_indx) = wrdq_bit_deskew[(byte * DQ_NUM_BITS_PER_BYTE) + bit_indx];
				
				// is this bit the new min or max?
				if (wrdq_bit_deskew[(byte * DQ_NUM_BITS_PER_BYTE) + bit_indx] < min_dq_deskew_code)
					min_dq_deskew_code = wrdq_bit_deskew[(byte * DQ_NUM_BITS_PER_BYTE) + bit_indx];
				else if (wrdq_bit_deskew[(byte * DQ_NUM_BITS_PER_BYTE) + bit_indx] > max_dq_deskew_code)
					max_dq_deskew_code = wrdq_bit_deskew[(byte * DQ_NUM_BITS_PER_BYTE) + bit_indx];
			}
			
			// find midpoint of deskew registers for this byte, and program it to DM (controlled by DQS regs, oddly)
			rAMP_WRDQSDESKEW_CTRL(ch, byte) = (min_dq_deskew_code + max_dq_deskew_code) >> 1;
		}
	} // for (ch = 0; ch < AMC_NUM_CHANNELS; ch++)
}

// This function writes PRBS7 patterns to dram for given channel and rank,
// and reads them back. Read back values are compared with data that was written
static uint32_t wr_rd_pattern_result(uint32_t ch, uint32_t rnk, uint32_t sdll_value)
{
	uint32_t chrnk_indx, result, result_per_wr_and_rdbk;
	uint32_t pattern_indx, pattern, readback_data;
	uint32_t col, word;
	uint64_t mem_region, mem_addr;
	uint32_t all_bits = 0xFFFFFFFF;
	uint32_t consecutive_cols_per_chrnk = (DQ_CONSECUTIVE_BYTES_PER_CHRNK / DQ_BYTES_PER_COL);

	result = all_bits;
	result_per_wr_and_rdbk = all_bits;
	
	chrnk_indx = (ch * AMC_NUM_RANKS) + rnk;
	pattern_indx = sdll_value & DLLVAL_BITS; // sdll tap indexes into pattern array

	// <rdar://problem/14017861> need APB read inserted in function wr_rd_pattern_result
	rAMP_RDFIFOPTRSTS(ch);
	
	// write the patterns to memory 4 bytes at a time
	// interleaving applies every DQ_CONSECUTIVE_BYTES_PER_CHRNK bytes, so recompute the address at that point
	// Note that bank and row are fixed
	for (col = 0; col < DQ_NUM_PATTERNS; col += consecutive_cols_per_chrnk) {
		
		mem_region = amc_get_uncached_dram_virt_addr(ch, rnk, DQ_BANK, DQ_ROW, col);
		
		// next 16 words (or columns) are consecutively stored in a [channel,rank] combo
		for (word = 0; word < consecutive_cols_per_chrnk; word++) {
			
			mem_addr = mem_region + (uint64_t)((word << 2));

			// last pattern in array is dummy value, so skip it
			pattern = DQ_PRBS7_PATTERNS[pattern_indx % (DQ_NUM_PATTERNS - 1)];
			pattern_indx++;
			
			// write the pattern
			*(uint32_t *)mem_addr = pattern;
		}
	}
	
	pattern_indx = sdll_value & DLLVAL_BITS;
	
	// Now, read back the patterns (have to do it in a separate loop than the writes to get more robust calib values)
	for (col = 0; col < DQ_NUM_PATTERNS; col += consecutive_cols_per_chrnk) {
		
		mem_region = amc_get_uncached_dram_virt_addr(ch, rnk, DQ_BANK, DQ_ROW, col);
		
		// next 16 words (or columns) are consecutively stored in a [channel,rank] combo
		for (word = 0; word < consecutive_cols_per_chrnk; word++) {
			
			mem_addr = mem_region + (uint64_t)((word << 2));
			
			// last pattern in array is dummy value, so skip it
			pattern = DQ_PRBS7_PATTERNS[pattern_indx % (DQ_NUM_PATTERNS - 1)];
			pattern_indx++;
			
			// read the pattern
			readback_data = *(uint32_t *)mem_addr;
			
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

// Save or restore ca and wrlvl registers for resume boot
// Registers must be stored/retrieved in exactly the order below
static void save_restore_ca_wrlvl_regs(uint32_t save_or_restore)
{
	uint32_t ch, bit_indx, byte;
	uint32_t byte_pos = 0;
	
	if (save_or_restore == CALIB_SAVE) {
		
		for (ch = 0; ch < AMC_NUM_CHANNELS; ch++) {
			
			// save the CA registers for this channel
			uint8_t casdll = (uint8_t) (rAMP_CASDLLCTRL(ch) & DLLVAL_BITS);
			
			for (bit_indx = 0; bit_indx < CA_NUM_BITS; bit_indx++) {
				uint8_t ca_deskew = (uint8_t) (rAMP_CADESKEW_CTRL(ch, bit_indx) & DESKEW_CTRL_BITS);
				uint8_t ca_offset = INT_TO_OFFSET(OFFSET_TO_INT(casdll) + OFFSET_TO_INT(ca_deskew));
				cal_pmu_bits[byte_pos++] = ca_offset;
			}
		
			// CK, CS, and CKE share the same value
			uint8_t ck_deskew = (uint8_t) (rAMP_CKDESKEW_CTRL(ch) & DESKEW_CTRL_BITS);
			cal_pmu_bits[byte_pos++] = ck_deskew;
			
			// save the WrLvl registers for this channel (4 DQ SDLLs and 1 CA SDLL)
			for (byte = 0; byte < (DQ_NUM_BYTES + 1); byte++) {
				
				// cawrlvlsdll is stored as the "5th" byte
				if (byte == DQ_NUM_BYTES)
					cal_pmu_bits[byte_pos++] = (uint8_t) (rAMP_CAWRLVLSDLLCODE(ch) & DLLVAL_BITS);
				else
					cal_pmu_bits[byte_pos++] = (uint8_t) (rAMP_DQWRLVLSDLLCODE(ch, byte) & DLLVAL_BITS);
			}
		}
		
#if !SUB_PLATFORM_S7002
		// Save the cal_pmu_bits array to PMU nvram
		if (power_store_memory_calibration((void *) cal_pmu_bits, CALIB_NUM_BYTES_TO_SAVE) == 0)
		{
#if AMP_NO_PMU_PANIC
			printf("Unable to save memory calibration values to PMU nvram\n");
#else
			panic("Unable to save memory calibration values to PMU nvram\n");
#endif
		}
#endif // #if !SUB_PLATFORM_S7002
		
	} else {
		
#if !SUB_PLATFORM_S7002
		// Retrieve cal_pmu_bits array from PMU nvram
		if (power_load_memory_calibration((void *) cal_pmu_bits, CALIB_NUM_BYTES_TO_SAVE) == 0)
		{
#if AMP_NO_PMU_PANIC
			printf("Unable to load memory calibration values from PMU nvram\n");
#else
			panic("Unable to load memory calibration values from PMU nvram\n");
#endif
		}
#endif // #if !SUB_PLATFORM_S7002

		save_masterdll_values();
		
		for (ch = 0; ch < AMC_NUM_CHANNELS; ch++) {
			
			int8_t casdll = OFFSET_TO_INT(cal_pmu_bits[byte_pos]);
			int32_t ca_offset[CA_NUM_BITS];
			
			for (bit_indx = 0; bit_indx < CA_NUM_BITS; bit_indx++) {
				ca_offset[bit_indx] = OFFSET_TO_INT(cal_pmu_bits[byte_pos]);
				byte_pos++;
				if (ca_offset[bit_indx] < casdll)
					casdll = ca_offset[bit_indx];
			}
			
			// get the value pushed on CK, CS, CKE signals, add it to casdll
			casdll -= OFFSET_TO_INT(cal_pmu_bits[byte_pos]);
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
				amp_push_ctl_out(ch, caclk);

			// compute deskew and write to the per bit deskew registers
			for (bit_indx = 0; bit_indx < CA_NUM_BITS; bit_indx++) {
				uint8_t ca_deskew = (uint8_t) (ca_offset[bit_indx] - (casdll - caclk));
				rAMP_CADESKEW_CTRL(ch, bit_indx) = ca_deskew;;
			}
			
			for (byte = 0; byte < (DQ_NUM_BYTES + 1); byte++) {
				uint8_t wrlvlsdll = cal_pmu_bits[byte_pos++];
				
				// At this point, DRAM is not in WRLVL mode so can use dlysel with forceckelow
				if (byte == DQ_NUM_BYTES)
					amp_set_cawrlvl_sdll(ch, wrlvlsdll, true);
				else
					amp_set_dqwrlvl_sdll(ch, byte, wrlvlsdll, true);
			}
		}
	}
}

	
// Bit packing functions are not needed (enough space in PMU to be rid of this complexity)
#if 0
// Inserts the data at given byte and bit position in the cal_pmu_bits array
// Assumes that num_bits is always <= 8
static void cal_save_value(uint8_t data, uint32_t num_bits, uint32_t *bit_pos, uint32_t *byte_pos)
{
	uint32_t space_in_this_byte;
	uint8_t mask;
	
	if (((*bit_pos) > 7) || ((*byte_pos) >= CALIB_PMU_BYTES))
		panic("Error! bit position %d > 7 or byte position %d > capacity (%d)\n", *bit_pos, *byte_pos, CALIB_PMU_BYTES);
	
	// how many bits left in this byte?
	space_in_this_byte = 8 - (*bit_pos);
	
	// we'll grab as many bits from the data as there is space in this byte
	if (space_in_this_byte >= num_bits)
		mask = (1 << num_bits) - 1;
	else
		mask = (1 << space_in_this_byte) - 1;
	
	// Set the data value at given byte (only as many bits as space and making sure to preserve the other bits in this byte)
	cal_pmu_bits[*byte_pos] |= ((data & mask) << *bit_pos);
	
	if (space_in_this_byte < num_bits) {
		// any remainder bits get saved to the next byte
		cal_pmu_bits[(*byte_pos) + 1] = (data >> space_in_this_byte);
		(*byte_pos)++;
		*bit_pos = num_bits - space_in_this_byte;
	} else if (space_in_this_byte == num_bits) {
		(*byte_pos)++;
		*bit_pos = 0;
	} else {
		(*bit_pos) += num_bits;
	}
}

// Retrieve the data at given byte and bit position in the cal_pmu_bits array
// Assumes that num_bits is always <= 8
static uint8_t cal_retrieve_value(uint32_t num_bits, uint32_t *bit_pos, uint32_t *byte_pos)
{
	uint32_t space_in_this_byte;
	uint8_t mask, remainder_mask, ret_val;
	
	if (((*bit_pos) > 7) || ((*byte_pos) >= CALIB_PMU_BYTES))
		panic("Error! bit position %d > 7 or byte position %d > capacity (%d)\n", *bit_pos, *byte_pos, CALIB_PMU_BYTES);
	
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
	ret_val = (cal_pmu_bits[*byte_pos] >> *bit_pos) & mask;
	
	if (space_in_this_byte < num_bits) {
		// any remainder bits get loaded from the next byte
		ret_val |= (cal_pmu_bits[(*byte_pos) + 1] & remainder_mask) << space_in_this_byte;
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
#endif
						
						
// Before starting dq calibration, saves the contents of dram region that will be written to with calibration patterns.
// After calibration is complete, restores the contents back to DRAM.
static void save_restore_memory_region(bool dqcal_start)
{
	uint32_t rnk;
	volatile uintptr_t mem_addr, src, dest;
	
	mem_addr = SDRAM_BASE_UNCACHED;
	
	for (rnk = 0; rnk < AMC_NUM_RANKS; rnk++) {
		mem_addr |= (rnk << DQ_ADDR_RANK_BIT);
		
		if (dqcal_start) {
			dest = (uintptr_t) &(dqcal_saved_data[rnk][0]);
			src = mem_addr;
		} else {
			dest = mem_addr;
			src = (uintptr_t) &(dqcal_saved_data[rnk][0]);
		}
		
		// we'll be writing (or have written) the patterns for each channel
		memcpy((void *) dest, (void *) src, sizeof(DQ_PRBS7_PATTERNS) * AMC_NUM_CHANNELS);
	}
}

#if 0
// Given an input where bit SIGN_BIT_POS represents the sign, and the rest is magnitude
// separate out the sign and magnitude and return those values to the caller
static void get_offset_sign_magnitude(uint32_t offset, uint32_t *neg_bit_set, uint32_t *tap_val)
{
	*neg_bit_set = (offset & (1 << SIGN_BIT_POS)) >> SIGN_BIT_POS;
	*tap_val = offset - (*neg_bit_set << SIGN_BIT_POS);
}
#endif
						
static int32_t find_center_of_eye(int32_t left_pos_val, int32_t right_pos_val)
{
	if (left_pos_val < right_pos_val)
		panic("Memory calibration: find_center_of_eye: Left value (0x%x) is < right value (0x%x)", left_pos_val, right_pos_val);
	
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
