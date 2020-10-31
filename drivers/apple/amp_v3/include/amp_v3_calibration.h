/*
 * Copyright (C) 2012-2013 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */

#ifndef __AMP_V3_CALIBRATION_H
#define __AMP_V3_CALIBRATION_H

#define AMP_DQ_PER_CHAN		2

#define	AMP_MAX_PATTERNS	8

#define CALIB_SAVE		0
#define CALIB_RESTORE		1
#define CALIB_PMU_BYTES		400
// Each register is saved as 1 byte in the array. We will save 1 offset per bit, where offset = SDLL + deskew.
// Number of bytes to save per channel = 10 CA bit offsets + 1 CK/CKE/CS deskew + 5 WrLvl SDLLs
// + 32 RDDQ deskews + 32 WRDQ deskews = 80
#define CALIB_NUM_BYTES_TO_SAVE		(AMC_NUM_CHANNELS * (80))

#define CA_NUM_BITS		10
#define CA_ALL_BITS		((1 << CA_NUM_BITS) - 1)
#define CA_NUM_PATTERNS		64
// <rdar://problem/13821643>
#define DELIMIT_POS_ADJ_CASDLL		6
#define DELIMIT_POS_ADJ_WRDQSDLL	6

#define CSBIT			(CA_NUM_BITS)
#define CKBIT			(CA_NUM_BITS + 1)
#define CKEBIT			(CA_NUM_BITS + 2)
#define CASDLLBIT		(CA_NUM_BITS + 3)

#define CACAL_MAX_SWLOOPS		(8)

#define MR_READ	0
#define MR_WRITE	1

#define MR2			0x02
#define MR5			0x05
#define MR41			0x29
#define MR42			0x2A
#define MR48			0x30

#define MIN_ENDPT		0
#define MAX_ENDPT		1

/* defines for conversion between sdll and deskew steps */
#define DIRECTION_DESKEW_TO_SDLL 0
#define DIRECTION_SDLL_TO_DESKEW 1

#define CA_MAX_DESKEW_OFFSET	29
#define MAX_DESKEW_PROGRAMMED	31
#define MAX_SDLL_VAL		63
#define MIN_SDLL_VAL		24
#define FINER_STEP_SZ		1
#define COARSE_STEP_SZ		4
#define SOLID_PASS_DETECT	8
#define DLLVAL_BITS		0x7F
#define DESKEW_CTRL_BITS	0x1F
#define SDLL_NUM_BITS		7
#define DESKEW_NUM_BITS		5
#define RD_LATENCY_ENCODE	26

// maximum possible loops in the hw
#define RDDQ_LOOPCNT		63
#define DQ_NUM_BYTES		4
#define DQ_NUM_BYTES_PER_DQ		2
#define DQ_NUM_BITS_PER_BYTE	8
#define DQ_TOTAL_BITS		(DQ_NUM_BYTES * DQ_NUM_BITS_PER_BYTE)
#define DQ_MAX_DESKEW_PER_BIT	29
#define MAX_CAWRLVL_CODE	127
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
#define SELECT_RDDQCAL2	0x10
#define SELECT_CAL_ALL		(SELECT_RDDQCAL2 | SELECT_WRDQCAL | SELECT_RDDQCAL | SELECT_WRLVLCAL | SELECT_CACAL)

// This defintion is put here for non-iBoot compilation environments
#ifndef __abs
#define __abs(x) (((x) < 0) ? -(x) : (x))
#endif

// Macros to change format between signed integer value and offset with sign and magnitude
#define INT_TO_OFFSET(val) 	((((val < 0) ? 1 : 0) << SIGN_BIT_POS) | __abs(val))
#define OFFSET_TO_INT(val) 	((val & ~(1 << SIGN_BIT_POS)) * ((val & (1 << SIGN_BIT_POS)) ? -1 : 1))

void calibration_ca_rddq_wrlvl(bool resume);
void calibration_wrdq_rddq(bool resume);
void calibration_save_restore_regs(uint32_t save_or_restore, uint32_t channels);
void calibration_dump_results(uint32_t operations, bool dump_passing_points);

struct amp_calibration_params {
	uint32_t num_channels;
	uint32_t num_ranks;
	bool resume;
	uint32_t cacalib_hw_loops;
	uint32_t cacalib_sw_loops;

	/* 
	 * Starting with Fiji B0, 1 deskew step is ~22% bigger than 1 sdll step.
	 * Fiji B0 / Capri B0 should define deskew_scale to 100 and sdll_scale to 122 (or some combo of numbers that indicates 22% increase),
	 * while Fiji A0 should define 1 and 1.
	 */
	int32_t sdll_scale;
	int32_t deskew_scale;
		
	// SEG specific flags
	bool dv_params_valid; // DV flag
	bool dv_randomize; // flag to be used by DV to randomize some register values for DV purposes
	uint32_t dbg_calib_mode; // flag to be used by SEG to skip calibration operations for debug purposes
};


#endif /* ! __AMP_V3_CALIBRATION_H */
