/*
 * Copyright (C) 2012-2015 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */

#ifndef __DCS_CALIBRATION_H
#define __DCS_CALIBRATION_H

#ifdef ENV_IBOOT

// Calibration registers
#if DCS_REG_VERSION < 3
#include <drivers/dcs/dcs_cal.h>
#else
#include <drivers/dcs/dcs_cal_v2.h>
#endif

#endif

// Chip rev definitions for non-iBoot environments
#ifndef ENV_IBOOT
#define	CHIP_REVISION_A0		0x00
#define	CHIP_REVISION_A1		0x01
#define	CHIP_REVISION_B0		0x10
#define	CHIP_REVISION_B1		0x11
#define	CHIP_REVISION_C0		0x20
#define	CHIP_REVISION_C1		0x21
#endif

#define AMP_DQ_PER_CHAN		2

#define	AMP_MAX_PATTERNS	8

#define CALIB_SAVE		0
#define CALIB_RESTORE		1
#define CALIB_PMU_BYTES		400
// 1 CASDLL (8 bits), 1 CKSDLL (8 bits), 1 CSSDLL (8 bits), 6 CA bit deskews (6 bits * 6), CK/CS deskew (6 bits * 2), 3 WrLvl SDLLs (8 bits * 3)
// WRDQ SDLLs (2 * 8 bits), WRDQS SDLLs (2 * 8 bits), WR DQ deskew (2 * 8 * 6 bits), WR DQS deskew ( 2 * 6 bits), WR DM deskew (2 * 6 bits)
// 3 WrLvl SDLLs (8 bits * 3) after WRDQ calibration
// RD SDLLs (2 * 8 bits), RD deskew (2 * 8 * 6 bits), RD DMI deskew (2 * 6 bits)
// Optimal Vref for CA, RD & WR for bins 0 & 1 (3 * 2 * 6 bits) 
// VT scaling refence codes for CA, RD and WR for both DQ splits ( 3 * 2 * 10 bits)
// Total bits per channel = 24 +36 + 12 + 24 + 16 + 16 + 96 + 12 + 12 + 24 + 16 + 96 + 12 + 36 + 60 = 492 bits 

// TODO: Recalculate this (somehow!  --> better: sizeof(struct)  )
#define CALIB_NUM_BYTES_TO_SAVE		(DCS_NUM_CHANNELS * (62))

// CP Changed 
#define CA_NUM_BITS		6
#define CA_ALL_BITS		((1 << CA_NUM_BITS) - 1)

#if DCS_REG_VERSION < 3
#define CA_NUM_VREF_F0 10
#define RDDQ_NUM_VREF_F0 10
#define CA_NUM_VREF_F1 21
#define RDDQ_NUM_VREF_F1 26
#define CA_NUM_VREF_MAX ((CA_NUM_VREF_F0 > CA_NUM_VREF_F1) ? CA_NUM_VREF_F0 : CA_NUM_VREF_F1)
#define RDDQ_NUM_VREF_MAX ((RDDQ_NUM_VREF_F0 > RDDQ_NUM_VREF_F1) ? RDDQ_NUM_VREF_F0 : RDDQ_NUM_VREF_F1)

#else
#define CA_NUM_VREF 10
#define RDDQ_NUM_VREF 10
#endif // #if DCS_REG_VERSION < 3

#define WRDQ_NUM_VREF 10

#define CA_NUM_PATTERNS		64
// <rdar://problem/13821643>
#define DELIMIT_POS_ADJ_CASDLL		6
#define DELIMIT_POS_ADJ_WRDQSDLL	6

#define MR_READ	0
#define MR_WRITE	1

#define MR0			0x0
#define MR2			0x02
#define MR5			0x05
#define MR12			0x0C
#define MR13			0x0D
#define MR14			0x0E
#define MR15			0x0F
#define MR20			0x14
#define MR32			0x20
#define MR40			0x28
#define MR41			0x29
#define MR42			0x2A
#define MR48			0x30

#define MIN_ENDPT		0
#define MAX_ENDPT		1

#if DCS_REG_VERSION == 1
#define MAX_DESKEW_PROGRAMMED	31
#define MAX_SDLL_VAL		203
#else
#define MAX_DESKEW_PROGRAMMED	55
#if DCS_REG_VERSION == 2
#define MAX_SDLL_VAL		248
#else
#define MAX_SDLL_VAL		204
#endif // #if DCS_REG_VERSION == 2
#endif // #if DCS_REG_VERSION == 1

#define MIN_SDLL_VAL		0
#define FINER_STEP_SZ		1
#define COARSE_STEP_SZ		4
#define SOLID_PASS_DETECT	8
#define SOLID_PASS_ZONE_DETECT	4
#define SOLID_FAIL	2
#define DLLVAL_BITS		0xFF
#define DESKEW_CTRL_BITS	0x3F
#define SDLL_NUM_BITS		8
#define DESKEW_NUM_BITS		6
#define CA_WRDQ_VREF_CTRL_BITS	0x7F
#define CA_WRDQ_VREF_NUM_BITS		7
#define RDDQ_VREF_CTRL_BITS	0xFF
#define RDDQ_VREF_NUM_BITS		8
#define RD_LATENCY_ENCODE	26

#define MAX_DQWRLVL_CODE	127
#define DQ_NUM_PATTERNS	128
// The number of bytes below is consecutive for a given ch, rnk, bank, and row (interleaving starts after these bytes)
//#define DQ_CONSECUTIVE_BYTES_PER_CHRNK		256
#define DQ_BYTES_PER_COL			4	// That makes for 64 consecutive columns (or words)
// the bank and row numbers where patterns will be written
#define DQ_ROW			0
#define DQ_BANK		0
#define DQ_ADDR_RANK_BIT	16 // RIBI2 addressing for dual rank devices

// Calibration operation selection
#define SELECT_CACAL		0x1
#define SELECT_WRLVLCAL	0x2
#define SELECT_RDDQCAL		0x4
#define SELECT_WRDQCAL		0x8
#define SELECT_CAL_ALL		0xF

// This defintion is put here for non-iBoot compilation environments
#ifndef __abs
#define __abs(x) (((x) < 0) ? -(x) : (x))
#endif

#if DCS_REG_VERSION < 2
#define DQ_MAX_DESKEW_PER_BIT	47
#else
#define DQ_MAX_DESKEW_PER_BIT	55
#endif

// maximum possible loops in the hw
#define RDDQ_LOOPCNT		16
#define DQ_NUM_BYTES		2
#define DQ_NUM_BYTES_PER_DQ		1
#define DQ_NUM_BITS_PER_BYTE	8
#define DQ_TOTAL_BITS		(DQ_NUM_BYTES * DQ_NUM_BITS_PER_BYTE)
#define DMI_TOTAL_BITS		2
#define DMI_NUM_BITS_PER_BYTE		1

#if DCS_REG_VERSION == 2
#define MAX_CAWRLVL_CODE	52
#else
#define MAX_CAWRLVL_CODE	47
#endif // #if DCS_REG_VERSION == 2

#define CLK_PERIOD_NUMER		(1000000)
#define CLK_PERIOD_OF(_freq_in_mhz)	(CLK_PERIOD_NUMER / (_freq_in_mhz))

#define CLK_PERIOD_1600			CLK_PERIOD_OF(1600)	//  625
#define CLK_PERIOD_1200			CLK_PERIOD_OF(1200)	//  833
#define CLK_PERIOD_800			CLK_PERIOD_OF(800)	// 1250

void calibration_ca_rddq_wrlvl(bool resume);
void calibration_wrdq_rddq(bool resume);
void calibration_save_restore_ca_wrlvl_regs(uint32_t save_or_restore, uint32_t channels, bool freq_bin);
void calibration_dump_results(uint32_t operations, bool dump_passing_window);

struct amp_calibration_params {
	uint32_t chip_id;
	uint32_t chip_rev;
	uint32_t num_channels;
	uint32_t num_channels_runCalib;
	uint32_t num_ranks;
	uint32_t clk_period_bin0;
	uint32_t clk_period_bin1;

#if DCS_REG_VERSION < 3
	uint32_t ca_num_vref_f0;
	uint32_t rddq_num_vref_f0;
	uint32_t ca_num_vref_f1;
	uint32_t rddq_num_vref_f1;
#else
	uint32_t ca_num_vref;
	uint32_t rddq_num_vref;
#endif // #if DCS_REG_VERSION < 3

	uint32_t wrdq_num_vref;

	bool wrdq_apply_NC_algorithm;

#if DCS_REG_VERSION < 3
	uint32_t ca_vref_values_f0[CA_NUM_VREF_F0];
	uint32_t rddq_vref_values_f0[RDDQ_NUM_VREF_F0];
	uint32_t ca_vref_values_f1[CA_NUM_VREF_F1];
	uint32_t rddq_vref_values_f1[RDDQ_NUM_VREF_F1];
#else
	uint32_t ca_vref_values[CA_NUM_VREF];
	uint32_t rddq_vref_values[RDDQ_NUM_VREF];
#endif

	uint32_t wrdq_vref_values[WRDQ_NUM_VREF];

	bool resume;
	bool disable_coarse_cs_training;
	uint32_t opt_vref_mode;			// Use CoM or "Widest Span" method?
	uint32_t cacalib_hw_loops;
	uint32_t cacalib_sw_loops;
	uint32_t cacalib_dqs_pulse_cnt;
	uint32_t cacalib_dqs_pulse_width;
	uint32_t rddq_cal_loopcnt;
	uint32_t wrdq_cal_loopcnt;
	uint32_t rddq_legacy_mode;
	uint32_t wrdq_legacy_mode;
	uint32_t cacal_bit_sel[8];
	uint32_t cacal_rd_byte_sel;
	uint32_t ca_prbs4_pattern;
	uint32_t prbs7_pattern_0;
	uint32_t prbs7_pattern_1;
	uint32_t prbs7_pattern_2;
	uint32_t prbs7_pattern_3;
	uint32_t ca_prbs_seed_bit[CA_NUM_BITS];
	uint32_t wrdq_prbs_seed[DQ_NUM_BYTES][DQ_NUM_BITS_PER_BYTE + DMI_NUM_BITS_PER_BYTE];
	uint32_t ca_invert_mask;
	uint32_t CsStopCycleCnt;
	uint32_t CsActiveCycleCnt;
	uint32_t tADR, tMRD, tCAENT, tVREF_LONG, tCKELCK, tCKCKEH, tDSTrain;
	uint32_t tWLO;
	uint32_t tWLMRD;
	uint32_t tWLDQSEN;
	uint32_t tDQSCKE;
	uint32_t tCKEHCMD;
	uint32_t WrLvlDqsPulseCnt;
	int freq_bin;
	int RL;
	int WL;
	
	// SEG specific flags
	bool dv_params_valid; // DV flag
	bool dv_randomize; // flag to be used by DV to randomize some register values for DV purposes
	uint32_t dbg_calib_mode; // flag to be used by SEG to skip calibration operations for debug purposes
};

#define DCS_CAL_OPT_VREF_MODE_WIDEST		0
#define DCS_CAL_OPT_VREF_MODE_COM		1

////////////////////////////////////////////////////////////////////////////////////////////////////////
// API Functions
////////////////////////////////////////////////////////////////////////////////////////////////////////
struct amp_calibration_params *calibration_cfg_params_init(int bootMode, int numChans, int numRanksPerChn);

void calibrate_ca(void);
void calibrate_wrlvl(void);
void calibrate_wrdq(void);
void calibrate_rddq(void);

////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif /* ! __DCS_CALIBRATION_H */
////////////////////////////////////////////////////////////////////////////////////////////////////////
