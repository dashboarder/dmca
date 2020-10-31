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

#ifndef __AMP_V2_CALIBRATION_H
#define __AMP_V2_CALIBRATION_H

#define	AMP_MAX_PATTERNS	8

#define CALIB_SAVE		0
#define CALIB_RESTORE		1
#define CALIB_PMU_BYTES		64
// 10 CA bit offsets (8 bits * 10), CK/CKE/CS deskew (8 bits), 5 WrLvl SDLLs (8 bits * 5)
#define CALIB_NUM_BYTES_TO_SAVE		(AMC_NUM_CHANNELS * (16))

#define CA_NUM_BITS		10
#define CA_ALL_BITS		((1 << CA_NUM_BITS) - 1)
#define CA_NUM_PATTERNS		64
#define DELIMIT_POS_ADJ_CASDLL	5
#define DELIMIT_POS_ADJ_WRDQSDLL	5

#define CA_MAX_SW_LOOPS		CA_NUM_PATTERNS
#define CA_MAX_LOOP_CHN_RNK	(CA_MAX_SW_LOOPS * AMC_NUM_CHANNELS * AMC_NUM_RANKS)

#define MR2			0x02
#define MR5			0x05
#define MR41			0x29
#define MR42			0x2A
#define MR48			0x30

#define MIN_ENDPT		0
#define MAX_ENDPT		1

#define MAX_DESKEW_OFFSET	24
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

#if SUB_PLATFORM_S7002
#define RD_LATENCY_ENCODE	6
#else
#define RD_LATENCY_ENCODE	26
#endif

// maximum possible loops in the hw
#define RDDQ_LOOPCNT		63
#define DQ_NUM_BYTES		4
#define DQ_NUM_BITS_PER_BYTE	8
#define DQ_TOTAL_BITS		(DQ_NUM_BYTES * DQ_NUM_BITS_PER_BYTE)
#define DQ_MAX_DESKEW_PER_BIT	24
#define MAX_CAWRLVL_CODE	47
#define MAX_DQWRLVL_CODE	127
#define DQ_NUM_PATTERNS	128
// The number of bytes below is consecutive for a given ch, rnk, bank, and row (interleaving starts after these bytes)
#define DQ_CONSECUTIVE_BYTES_PER_CHRNK		64
#define DQ_BYTES_PER_COL			4	// That makes for 16 consecutive columns (or words)
// the bank and row numbers where patterns will be written
#define DQ_ROW			0
#define DQ_BANK		0
#define DQ_ADDR_RANK_BIT	16 // RIBI2 addressing for dual rank devices

// Macros to change format between signed integer value and offset with sign and magnitude
#define INT_TO_OFFSET(val) 	((((val < 0) ? 1 : 0) << SIGN_BIT_POS) | __abs(val))
#define OFFSET_TO_INT(val) 	((val & ~(1 << SIGN_BIT_POS)) * ((val & (1 << SIGN_BIT_POS)) ? -1 : 1))

#endif /* ! __AMP_V2_CALIBRATION_H */
