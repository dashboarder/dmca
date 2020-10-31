/*
 *
 * Tanzanite: <rdar://problem/14756079> Tanzanite Specification
 *
 * Copyright (C) 2014 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */

#ifndef __DIALOG_D2238_H
#define __DIALOG_D2238_H

#define PMU_HAS_SWI		0
#define PMU_HAS_DWI		0
#define PMU_HAS_LCM_LDO		0
#define PMU_HAS_BIST_ADC	0
#define PMU_HAS_CHG_ABCC_FLAG	0
#define PMU_HAS_VIB		0
#define PMU_HAS_RAM		1
#define PMU_HAS_WLED		0
#define PMU_HAS_32K_RTC		1
// TODO: #define PMU_HAS_ACCUMULATORS	1
#define PMU_HAS_GPIO_CONF	1
#define PMU_HAS_SYS		1
#define PMU_HAS_WDT		0
#define PMU_HAS_CHARGER		1

enum {
	kDIALOG_ADDR_R		= 0x79,
	kDIALOG_ADDR_W		= 0x78,
	kDIALOG_REG_BYTES	= 2,
};

enum {
	kDIALOG_EVENT_COUNT		= 13,
	kDIALOG_STATUS_COUNT		= 13,
	kDIALOG_FAULTLOG_COUNT		= 2,
	kDIALOG_CHIPID_COUNT		= 10,
	kDIALOG_GPIO_COUNT		= 15,
};

#include "dCommon.h"

enum {
	kDIALOG_MASK_REV_CODE	= 0x0000,
	kDIALOG_TRIM_REL_CODE	= 0x0001,
	kDIALOG_PLATFORM_ID		= 0x0002,
	kDIALOG_DEVICE_ID1		= 0x0004,
	kDIALOG_DEVICE_ID2		= 0x0005,
	kDIALOG_DEVICE_ID3		= 0x0006,
	kDIALOG_DEVICE_ID4		= 0x0007,
	kDIALOG_DEVICE_ID5		= 0x0008,
	kDIALOG_CHIP_ID			= kDIALOG_MASK_REV_CODE,
	kDIALOG_DEVICE_ID6		= 0x0009,
	kDIALOG_DEVICE_ID7		= 0x000A,
	
	kD2238_SYS_CTRL1		= 0x0010,
	kD2238_SYS_CTRL2		= 0x0011,
	kDIALOG_SYS_CONTROL		= kD2238_SYS_CTRL1,
	kDIALOG_SYS_CONTROL2            = kD2238_SYS_CTRL2,

	kD2238_SYS_CONF_A		= 0x0020,
	kDIALOG_SYS_CONFIG		= kD2238_SYS_CONF_A,
	kD2238_SYS_CONF_B		= 0x0021,
	kD2238_SYS_CONF_C		= 0x0022,
	kD2238_SYS_CONF_D		= 0x0023,
	kD2238_BG_TRIM			= 0x0024,
	kD2238_BG_TC_ADJ		= 0x0025,
	kD2238_VREF_RTC_TRIM	= 0x0026,
	kD2238_VREF_AUX_TRIM	= 0x0027,
	kD2238_VDD_FAULT		= 0x0028,
	kD2238_PRE_UVLO			= 0x0029,
	kD2238_VDD_MIN_BOOT		= 0x002a,
	
	kD2238_RTC_CTRL0		= 0x002b,
	kD2238_RTC_CTRL1		= 0x002c,
	kD2238_OSC_CTRL0		= 0x002d,
	kD2238_INT_OSC_TRIM		= 0x002e,
	kD2238_CLK_REQ_FRC		= 0x002f,
	kD2238_UOV_CONTROL		 = 0x0030,
	kD2238_UOV_BLANK_CONTROL = 0x0031,

	kD2238_FAULT_LOG1		= 0x0040,
	kD2238_FAULT_LOG2		= 0x0041,
	kDIALOG_FAULT_LOG		= kD2238_FAULT_LOG1,
	kDIALOG_FAULT_LOG2		= kD2238_FAULT_LOG2,

	kD2238_BUCK_1_0_OUV     = 0x0042,
	kD2238_BUCK_3_2_OUV     = 0x0043,

	kD2238_LDO_1_0_OUV      = 0x0046,
	kD2238_LDO_3_2_OUV      = 0x0047,
	kD2238_LDO_5_4_OUV      = 0x0048,
	kD2238_LDO_7_6_OUV      = 0x0049,
	kD2238_LDO_9_8_OUV      = 0x004a,

	kDIALOG_EVENT_A			= 0x0050,
	kDIALOG_EVENT_B			= 0x0051,
	kDIALOG_EVENT_C			= 0x0052,
	kDIALOG_EVENT_D			= 0x0053,
	kDIALOG_EVENT_E			= 0x0054,
	kDIALOG_EVENT_F			= 0x0055,
	kDIALOG_EVENT_G			= 0x0056,
	kDIALOG_EVENT_H			= 0x0057,
	kDIALOG_EVENT_I			= 0x0058,
	kDIALOG_EVENT_J			= 0x0059,
	kDIALOG_EVENT_K			= 0x005a,
	kDIALOG_EVENT_L			= 0x005b,
	kDIALOG_EVENT_M			= 0x005c,

	kDIALOG_STATUS_A		= 0x0060,
	kDIALOG_STATUS_B		= 0x0061,
	kDIALOG_STATUS_C		= 0x0062,
	kDIALOG_STATUS_D		= 0x0063,
	kDIALOG_STATUS_E		= 0x0064,
	kDIALOG_STATUS_F		= 0x0065,
	kDIALOG_STATUS_G		= 0x0066,
	kDIALOG_STATUS_H		= 0x0067,
	kDIALOG_STATUS_I		= 0x0068,
	kDIALOG_STATUS_J		= 0x0069,
	kDIALOG_STATUS_K		= 0x006a,
	kDIALOG_STATUS_L		= 0x006b,
	kDIALOG_STATUS_M		= 0x006c,

	kDIALOG_IRQ_MASK_A		= 0x0070,
	kDIALOG_IRQ_MASK_B		= 0x0071,
	kDIALOG_IRQ_MASK_C		= 0x0072,
	kDIALOG_IRQ_MASK_D		= 0x0073,
	kDIALOG_IRQ_MASK_E		= 0x0074,
	kDIALOG_IRQ_MASK_F		= 0x0075,
	kDIALOG_IRQ_MASK_G		= 0x0076,
	kDIALOG_IRQ_MASK_H		= 0x0077,
	kDIALOG_IRQ_MASK_I		= 0x0078,
	kDIALOG_IRQ_MASK_J		= 0x0079,
	kDIALOG_IRQ_MASK_K		= 0x007A,
	kDIALOG_IRQ_MASK_L		= 0x007B,
	kDIALOG_IRQ_MASK_M		= 0x007C,
	
	kD2238_ACTIVE1			= 0x0080,
	kD2238_ACTIVE2			= 0x0081,
	kD2238_ACTIVE3			= 0x0082,
	kD2238_ACTIVE4			= 0x0083,
	kD2238_ACTIVE5			= 0x0084,
	kD2238_ACTIVE6			= 0x0085,
	kD2238_ACTIVE7			= 0x0086,
	kD2238_ACTIVE8			= 0x0087,
	
	kD2238_STANDBY1			= 0x0088,
	kD2238_STANDBY3			= 0x008a,
	kD2238_STANDBY4			= 0x008b,
	kD2238_STANDBY6			= 0x008d,
	kD2238_STANDBY7			= 0x008e,
	kD2238_STANDBY8			= 0x008f,
	
	kD2238_HIBERNATE1		= 0x0090,
	kD2238_HIBERNATE3		= 0x0092,
	kD2238_HIBERNATE4		= 0x0093,
	kD2238_HIBERNATE6		= 0x0095,
	kD2238_HIBERNATE7		= 0x0096,
	kD2238_HIBERNATE8		= 0x0097,

	kD2238_HALT1			= 0x0098,
	kD2238_HALT3			= 0x009a,
	kD2238_HALT4			= 0x009b,
	kD2238_HALT6			= 0x009d,
	kD2238_HALT7			= 0x009e,
	kD2238_HALT8			= 0x009f,

	kD2238_SLOT_TIMINGS_PWR_UP1	= 0x00a0,
	kD2238_SLOT_TIMINGS_PWR_UP2	= 0x00a1,
	kD2238_SLOT_TIMINGS_PWR_UP3	= 0x00a2,
	kD2238_SLOT_TIMINGS_PWR_UP4	= 0x00a3,
	kD2238_SLOT_TIMINGS_PWR_UP5	= 0x00a4,
	kD2238_SLOT_TIMINGS_PWR_UP6	= 0x00a5,
	kD2238_SLOT_TIMINGS_PWR_UP7	= 0x00a6,
	kD2238_SLOT_TIMINGS_PWR_UP8	= 0x00a7,
	kD2238_SLOT_TIMINGS_PWR_DN1	= 0x00a8,
	kD2238_SLOT_TIMINGS_PWR_DN2	= 0x00a9,
	kD2238_SLOT_TIMINGS_PWR_DN3	= 0x00aa,
	kD2238_SLOT_TIMINGS_PWR_DN4	= 0x00ab,
	kD2238_SLOT_TIMINGS_PWR_DN5	= 0x00ac,
	kD2238_SLOT_TIMINGS_PWR_DN6	= 0x00ad,
	kD2238_SLOT_TIMINGS_PWR_DN7	= 0x00ae,
	kD2238_SLOT_TIMINGS_PWR_DN8	= 0x00af,
	kD2238_SLOT_TIMINGS_PWR_DN9	= 0x00b0,
	kD2238_SLOT_TIMINGS_PWR_HALT_UP1	= 0x00c0,
	kD2238_SLOT_TIMINGS_PWR_HALT_UP2	= 0x00c1,
	kD2238_SLOT_TIMINGS_PWR_HALT_UP3	= 0x00c2,
	kD2238_SLOT_TIMINGS_PWR_HALT_UP4	= 0x00c3,
	kD2238_SLOT_TIMINGS_PWR_HALT_UP5	= 0x00c4,
	kD2238_SLOT_TIMINGS_PWR_HALT_UP6	= 0x00c5,
	kD2238_SLOT_TIMINGS_PWR_HALT_UP7	= 0x00c6,
	kD2238_SLOT_TIMINGS_PWR_HALT_UP8	= 0x00c7,

	kD2238_ACT_TO_HIB_DLY		= 0x00c8,

	kD2238_BUCK0_SLOT		= 0x00d0,
	kD2238_BUCK1_SLOT		= 0x00d1,
	kD2238_BUCK2_SLOT		= 0x00d2,
	kD2238_BUCK3_SLOT		= 0x00d3,
	
	kD2238_LDO1_SLOT		= 0x00d9,
	kD2238_LDO2_SLOT		= 0x00da,
	kD2238_LDO3_SLOT		= 0x00db,
	kD2238_LDO4_SLOT		= 0x00dc,
	kD2238_LDO5_SLOT		= 0x00dd,
	kD2238_LDO6_SLOT		= 0x00de,
	kD2238_LDO7_SLOT		= 0x00df,
	kD2238_LDO8_SLOT		= 0x00e0,
		
	kD2238_CHARGE_PUMP_SLOT		= 0x00e8,
	
	kD2238_SW0_SLOT			= 0x00e9,
	kD2238_SW1_SLOT			= 0x00ea,
	kD2238_SW2_SLOT			= 0x00eb,
	kD2238_SW3A_SLOT		= 0x00ec,
	kD2238_SW3B_SLOT		= 0x00ed,
	kD2238_SW3C_SLOT		= 0x00ee,
	kD2238_SW4_SLOT			= 0x00ef,

	kD2238_EXT0_SLOT		= 0x00f8,
	kD2238_EXT1_SLOT		= 0x00f9,
	kD2238_EXT2_SLOT		= 0x00fa,
	kD2238_EXT3_SLOT		= 0x00fb,

	kD2238_BUCK0_VSEL		= 0x0100,
	kD2238_BUCK0_VSEL_ALT		= 0x0101,
	kD2238_BUCK0_VSEL_ACTUAL	= 0x0102,
	kD2238_BUCK0_MINV		= 0x0103,
	kD2238_BUCK0_MAXV		= 0x0104,
	kD2238_BUCK0_OFS_V		= 0x0105,
	kD2238_BUCK0_MODE		= 0x0106,
	kD2238_BUCK0_START_ILIMIT	= 0x0107,
	kD2238_BUCK0_SYNC_ILIMIT	= 0x0108,
	kD2238_BUCK0_SLEEP_ILIMIT	= 0x0109,
	
	kD2238_BUCK0_FSM_TRIM0		= 0x010a,
	kD2238_BUCK0_FSM_TRIM1		= 0x010b,
	kD2238_BUCK0_FSM_TRIM2		= 0x010c,
	kD2238_BUCK0_FSM_TRIM3		= 0x010d,	
	kD2238_BUCK0_CLK_TRIM4		= 0x010e,
	kD2238_BUCK0_CLK_TRIM		= 0x010f,
	
	kD2238_BUCK0_CALIB0		    = 0x0111,
	kD2238_BUCK0_CALIB1		    = 0x0112,
	
	kD2238_BUCK0_ANA_TRIM0		= 0x0113,
	kD2238_BUCK0_ANA_TRIM1		= 0x0114,
	kD2238_BUCK0_ANA_TRIM2		= 0x0115,
	kD2238_BUCK0_ANA_TRIM3		= 0x0116,
	kD2238_BUCK0_ANA_TRIM4		= 0x0117,
	kD2238_BUCK0_ANA_TRIM5		= 0x0118,
	kD2238_BUCK0_ANA_TRIM6		= 0x0119,
	kD2238_BUCK0_ANA_TRIM7		= 0x011a,
	kD2238_BUCK0_ANA_TRIM8		= 0x011b,
	kD2238_BUCK0_ANA_TRIM9		= 0x011c,
	
	kD2238_BUCK1_VSEL		    = 0x0120,
	kD2238_BUCK1_VSEL_ACTUAL	= 0x0122,
	kD2238_BUCK1_MINV		    = 0x0123,
	kD2238_BUCK1_MAXV		    = 0x0124,
	kD2238_BUCK1_OFS_V		    = 0x0125,
	kD2238_BUCK1_MODE		    = 0x0126,
	kD2238_BUCK1_START_ILIMIT	= 0x0127,
	kD2238_BUCK1_SYNC_ILIMIT	= 0x0128,
	kD2238_BUCK1_SLEEP_ILIMIT	= 0x0129,
	kD2238_BUCK1_FSM_TRIM0		= 0x012a,
	kD2238_BUCK1_FSM_TRIM1		= 0x012b,
	kD2238_BUCK1_FSM_TRIM2		= 0x012c,
	kD2238_BUCK1_FSM_TRIM3		= 0x012d,
	kD2238_BUCK1_CLK_TRIM4		= 0x012e,
	kD2238_BUCK1_CLK_TRIM		= 0x012f,
	
	kD2238_BUCK1_CALIB0		    = 0x0131,
	
	kD2238_BUCK1_ANA_TRIM0		= 0x0133,
	kD2238_BUCK1_ANA_TRIM1		= 0x0134,
	kD2238_BUCK1_ANA_TRIM2		= 0x0135,
	kD2238_BUCK1_ANA_TRIM3		= 0x0136,
	kD2238_BUCK1_ANA_TRIM4		= 0x0137,
	kD2238_BUCK1_ANA_TRIM5		= 0x0138,
	kD2238_BUCK1_ANA_TRIM6		= 0x0139,
	kD2238_BUCK1_ANA_TRIM7		= 0x013a,
	kD2238_BUCK1_ANA_TRIM8		= 0x013b,
	kD2238_BUCK1_ANA_TRIM9		= 0x013c,
	
	kD2238_BUCK2_VSEL		    = 0x0140,
	kD2238_BUCK2_VSEL_HIB		    = 0x0141,
	kD2238_BUCK2_VSEL_HALT		    = 0x0142,
	kD2238_BUCK2_VSEL_ACTUAL	= 0x0143,
	kD2238_BUCK2_MINV		    = 0x0144,
	kD2238_BUCK2_MAXV		    = 0x0145,
	kD2238_BUCK2_OFS_V		    = 0x0146,
	kD2238_BUCK2_MODE		    = 0x0147,
	kD2238_BUCK2_START_ILIMIT	= 0x0148,
	kD2238_BUCK2_SYNC_ILIMIT	= 0x0149,
	kD2238_BUCK2_SLEEP_ILIMIT	= 0x014a,
	kD2238_BUCK2_FSM_TRIM0		= 0x014b,
	kD2238_BUCK2_FSM_TRIM1		= 0x014c,
	kD2238_BUCK2_FSM_TRIM2		= 0x014d,
	kD2238_BUCK2_FSM_TRIM3		= 0x014e,
	kD2238_BUCK2_CLK_TRIM4		= 0x014f,
	kD2238_BUCK2_CLK_TRIM		= 0x0150,

	kD2238_BUCK2_CALIB_OFFSET_TRIM	    = 0x0151,
	kD2238_BUCK2_CALIB0		    = 0x0152,
	kD2238_BUCK2_ANA_TRIM0		= 0x0154,
	kD2238_BUCK2_ANA_TRIM1		= 0x0155,
	kD2238_BUCK2_ANA_TRIM2		= 0x0156,
	kD2238_BUCK2_ANA_TRIM3		= 0x0157,
	kD2238_BUCK2_ANA_TRIM4		= 0x0158,
	kD2238_BUCK2_ANA_TRIM5		= 0x0159,
	kD2238_BUCK2_ANA_TRIM6		= 0x015a,
	kD2238_BUCK2_ANA_TRIM7		= 0x015b,
	kD2238_BUCK2_ANA_TRIM8		= 0x015c,
	kD2238_BUCK2_ANA_TRIM9		= 0x015d,

	kD2238_BUCK3_VSEL		    = 0x0160,
	kD2238_BUCK3_VSEL_HIB		= 0x0161,
	kD2238_BUCK3_VSEL_HALT		= 0x0162,
	kD2238_BUCK3_VSEL_ACTUAL	= 0x0163,
	kD2238_BUCK3_MINV		    = 0x0164,
	kD2238_BUCK3_MAXV		    = 0x0165,
	kD2238_BUCK3_OFS_V		    = 0x0166,
	kD2238_BUCK3_MODE		    = 0x0167,
	kD2238_BUCK3_START_ILIMIT	= 0x0168,
	kD2238_BUCK3_SYNC_ILIMIT	= 0x0169,
	kD2238_BUCK3_SLEEP_ILIMIT	= 0x016a,
	kD2238_BUCK3_FSM_TRIM0		= 0x016b,
	kD2238_BUCK3_FSM_TRIM1		= 0x016c,
	kD2238_BUCK3_FSM_TRIM2		= 0x016d,
	kD2238_BUCK3_FSM_TRIM3		= 0x016e,
	kD2238_BUCK3_CLK_TRIM		= 0x016f,
	
	kD2238_BUCK3_CALIB_OFFSET_TRIM = 0x0171,
	
	kD2238_BUCK3_CALIB0		    = 0x0172,
	kD2238_BUCK3_ANA_TRIM0		= 0x0174,
	kD2238_BUCK3_ANA_TRIM1		= 0x0175,
	kD2238_BUCK3_ANA_TRIM2		= 0x0176,
	kD2238_BUCK3_ANA_TRIM3		= 0x0177,
	kD2238_BUCK3_ANA_TRIM4		= 0x0178,
	kD2238_BUCK3_ANA_TRIM5		= 0x0179,
	kD2238_BUCK3_ANA_TRIM6		= 0x017a,
	kD2238_BUCK3_ANA_TRIM7		= 0x017b,
	kD2238_BUCK3_ANA_TRIM8		= 0x017c,
	kD2238_BUCK3_ANA_TRIM9		= 0x017d,
	
	kD2238_LDO_RTC_TRIM		    = 0x0304,
	kD2238_LDO_SOFT_START		    = 0x0307,

	kD2238_LDO1_VSEL		    = 0x0308,
	kD2238_LDO1_VSEL_ACTUAL		= 0x0309,
	kD2238_LDO1_MINV		    = 0x030a,
	kD2238_LDO1_MAXV		    = 0x030b,
	
	kD2238_LDO2_VSEL		    = 0x0310,
	kD2238_LDO2_VSEL_ACTUAL		= 0x0311,
	kD2238_LDO2_MINV		    = 0x0312,
	kD2238_LDO2_MAXV		    = 0x0313,

	kD2238_LDO3_VSEL		    = 0x0318,
	kD2238_LDO3_VSEL_ACTUAL		= 0x0319,
	kD2238_LDO3_MINV		    = 0x031a,
	kD2238_LDO3_MAXV		    = 0x031b,

	kD2238_LDO4_VSEL		    = 0x0320,
	kD2238_LDO4_VSEL_ACTUAL		= 0x0321,
	kD2238_LDO4_MINV		    = 0x0322,
	kD2238_LDO4_MAXV		    = 0x0323,
	kD2238_LDO4_UOV_LIM		    = 0x0324,
	kD2238_LDO4_SOFT_START		    = 0x0325,
	
	kD2238_LDO5_VSEL		    = 0x0328,
	kD2238_LDO5_VSEL_ACTUAL		= 0x0329,
	kD2238_LDO5_MINV		    = 0x032a,
	kD2238_LDO5_MAXV		    = 0x032b,
	
	kD2238_LDO6_VSEL		    = 0x0330,
	kD2238_LDO6_VSEL_ACTUAL		= 0x0331,
	kD2238_LDO6_MINV		    = 0x0332,
	kD2238_LDO6_MAXV		    = 0x0333,
	
	kD2238_LDO7_VSEL		    = 0x0338,
	kD2238_LDO7_VSEL_ACTUAL		= 0x0339,
	kD2238_LDO7_MINV		    = 0x033a,
	kD2238_LDO7_MAXV		    = 0x033b,
	
	kD2238_LDO8_VSEL		    = 0x0340,
	kD2238_LDO8_VSEL_ACTUAL		= 0x0341,
	kD2238_LDO8_MINV		    = 0x0342,
	kD2238_LDO8_MAXV		    = 0x0343,
	
	kD2238_LDO9_VSEL		    = 0x0348,
	kD2238_LDO9_VSEL_ACTUAL		= 0x0349,
	kD2238_LDO9_MINV		    = 0x034a,
	kD2238_LDO9_MAXV		    = 0x034b,
	kD2238_LDO9_TRIM		    = 0x034c,
	
	kD2238_HIB_SW_CTRL		    = 0x03c0,
	
	kD2238_GPIO1_CONF1          = 0x0400,
	kD2238_GPIO1_CONF2          = 0x0401,
	kD2238_GPIO2_CONF1			= 0x0402,
	kD2238_GPIO2_CONF2			= 0x0403,
	kD2238_GPIO3_CONF1			= 0x0404,
	kD2238_GPIO3_CONF2			= 0x0405,
	kD2238_GPIO4_CONF1			= 0x0406,
	kD2238_GPIO4_CONF2			= 0x0407,
	kD2238_GPIO5_CONF1			= 0x0408,
	kD2238_GPIO5_CONF2			= 0x0409,
	kD2238_GPIO6_CONF1			= 0x040a,
	kD2238_GPIO6_CONF2			= 0x040b,
	kD2238_GPIO7_CONF1			= 0x040c,
	kD2238_GPIO7_CONF2			= 0x040d,
	kD2238_GPIO8_CONF1			= 0x040e,
	kD2238_GPIO8_CONF2			= 0x040f,
	kD2238_GPIO9_CONF1			= 0x0410,
	kD2238_GPIO9_CONF2			= 0x0411,
	kD2238_GPIO10_CONF1			= 0x0412,
	kD2238_GPIO10_CONF2			= 0x0413,
	kD2238_GPIO11_CONF1			= 0x0414,
	kD2238_GPIO11_CONF2			= 0x0415,
	kD2238_GPIO12_CONF1			= 0x0416,
	kD2238_GPIO12_CONF2			= 0x0417,
	kD2238_GPIO13_CONF1			= 0x0418,
	kD2238_GPIO13_CONF2			= 0x0419,
	kD2238_GPIO14_CONF1			= 0x041a,
	kD2238_GPIO14_CONF2			= 0x041b,
	kD2238_GPIO15_CONF1			= 0x041c,
	kD2238_GPIO15_CONF2			= 0x041d,
	kDIALOG_SYS_GPIO_REG_START	 = kD2238_GPIO1_CONF1,

	kD2238_OUT_32K	        	= 0x0430,
	
	kD2238_BUTTON1_CONF		    = 0x0440,
	kD2238_BUTTON2_CONF		    = 0x0441,
	kD2238_BUTTON3_CONF		    = 0x0442,
	kDIALOG_BUTTON_DBL		    = 0x0443,
	kD2238_BUTTON_WAKE		    = 0x0444,
	
	kD2238_RESET_IN1_CONF		= 0x0448,
	kD2238_RESET_IN2_CONF		= 0x0449,

	kD2238_ALL_RAILS_CONF		    = 0x044a,
	kD2238_CORE_HI_CONF		    = 0x044b,
	kD2238_VBUS_DET_CONF		    = 0x044c,
	
	kD2238_NRST_CONF		    = 0x0460,
	kD2238_NRST_ADJ			    = 0x0461,
	kD2238_NIRQ_CONF		    = 0x0462,
	kD2238_SYS_ALIVE_CONF		    = 0x0463,
	kD2238_PWR_GOOD_CONF		    = 0x0464,
	kD2238_PWR_GOOD_ADJ		    = 0x0465,
	kD2238_VDD_OK_CONF		    = 0x0466,
    
	kD2238_TDEV1_RISE		    = 0x0480,
	kD2238_TDEV1_FALL		    = 0x0481,
	kD2238_TDEV2_RISE		    = 0x0482,
	kD2238_TDEV2_FALL		    = 0x0483,
	kD2238_TDEV3_RISE		    = 0x0484,
	kD2238_TDEV3_FALL		    = 0x0485,
	kD2238_TDEV4_RISE		    = 0x0486,
	kD2238_TDEV4_FALL		    = 0x0487,

	kD2238_TLDO4_RISE		    = 0x04a0,
	kD2238_TCHARGER_RISE		    = 0x04a1,
	kD2238_TBUCK0_RISE		    = 0x04a2,
	kD2238_TBUCK1_RISE		    = 0x04a3,
	kD2238_TBUCK2_RISE		    = 0x04a4,
	kD2238_TBUCK3_RISE		    = 0x04a5,
	kD2238_TISENSE_HYST		    = 0x04a6,

	kDIALOG_T_OFFSET_MSB		    = 0x04a7,
	kDIALOG_T_OFFSET_LSB		    = 0x04a8,
	
	kD2238_CHG_CTRL_A		    = 0x04c0,
	kD2238_CHG_CTRL_B		    = 0x04c1,
	kD2238_CHG_CTRL_C		    = 0x04c2,
	kD2238_CHG_CTRL_D		    = 0x04c3,
	kD2238_CHG_CTRL_E		    = 0x04c4,
	kD2238_CHG_CTRL_F		    = 0x04c5,
	kD2238_CHG_TIME			    = 0x04c6,
	kD2238_CHG_TIME_PRE		    = 0x04c7,
	kDIALOG_CHARGE_CONTROL_ICHG_BAT	    = kD2238_CHG_CTRL_B,
	kDIALOG_CHARGE_CONTROL_TIME	    = kD2238_CHG_CTRL_D,
	kDIALOG_CHARGE_CONTROL_VSET	    = kD2238_CHG_CTRL_E,
	kDIALOG_OTP_ISET_BAT		    = kD2238_CHG_CTRL_C,

	kD2238_CHG_STAT			    = 0x04c8,
	kDIALOG_CHARGE_STATUS		    = kD2238_CHG_STAT,
	
	kD2238_CHG_VSET_TRIM		    = 0x04c9,
	kD2238_CHG_TRIM1		    = 0x04ca,
	kD2238_CHG_TRIM2		    = 0x04cb,
	kD2238_CHG_TRIM3		    = 0x04cc,
	kD2238_ICHG_END			    = 0x04cd,

	kDIALOG_TBAT_0			= 0x04ce,
	kDIALOG_TBAT_1		    	= 0x04cf,
	kDIALOG_TBAT_2		    	= 0x04d0,
	kDIALOG_TBAT_3		    	= 0x04d1,
	kDIALOG_TBAT_4		    	= 0x04d2,
	kDIALOG_TBAT_MAX	    	= 0x04d3,
	kDIALOG_ICHG_TBAT_0		= 0x04d4,
	kDIALOG_ICHG_TBAT_1	    	= 0x04d5,
	kDIALOG_ICHG_TBAT_2	    	= 0x04d6,
	kDIALOG_ICHG_TBAT_3	    	= 0x04d7,
	kDIALOG_ICHG_TBAT_4		= 0x04d8,
	kD2238_ICHG_TBAT		= 0x04d9,

	kDIALOG_ICHG_AVG		    = 0x04da,

	kD2238_HR_RES			= 0x04db,
	kD2238_HR_LOW_TH		= 0x04dc,
	kD2238_HR_LOOPUP_V1		= 0x04dd,
	kD2238_HR_LOOPUP_I1		= 0x04de,
	kD2238_HR_LOOPUP_V2		= 0x04df,
	kD2238_HR_LOOPUP_I2		= 0x04e0,
	kD2238_HR_LOOPUP_V3		= 0x04e1,
	kD2238_HR_LOOPUP_I3		= 0x04e2,
	kD2238_HR_LOOPUP_V4		= 0x04e3,
	kD2238_HR_LOOPUP_I4		= 0x04e4,
	kD2238_HR_LOOPUP_V5		= 0x04e5,
	kD2238_HR_LOOPUP_I5		= 0x04e6,
	kD2238_HR_LOOPUP_V6		= 0x04e7,
	kD2238_HR_LOOPUP_I6		= 0x04e8,
	kD2238_HR_LOOPUP_V7		= 0x04e9,
	kD2238_HR_LOOPUP_I7		= 0x04ea,
	kD2238_HR_LOOPUP_V8		= 0x04eb,
	kD2238_HR_LOOPUP_I8		= 0x04ec,
	kD2238_HR_LOOPUP_V9		= 0x04ed,
	kD2238_HR_LOOPUP_I9		= 0x04ee,
	kD2238_HR_LOOPUP_I10		= 0x04ef,
	kD2238_HR_LOOPUP_HR		= 0x04f0,

	kD2238_ADC_CTRL1		= 0x0500,
	kD2238_ADC_CTRL2		= 0x0501,
	kDIALOG_ADC_CONTROL		= kD2238_ADC_CTRL1,
	kDIALOG_ADC_CONTROL2		= kD2238_ADC_CTRL2,
	kDIALOG_ADC_LSB			    = 0x0502,
	kDIALOG_ADC_MSB			    = 0x0503,
	kD2238_ADC_TEMP_CTRL0		= 0x0504,
	kD2238_ADC_TEMP_CTRL1		= 0x0505,
	kD2238_ADC_HR_CTRL		= 0x0506,
	kD2238_ADC_FSM_TRIM0		= 0x0507,
	kD2238_ADC_FSM_TRIM1		= 0x0508,
	kD2238_ADC_FSM_TRIM2		= 0x0509,
	kD2238_ADC_ANA_TRIM0		= 0x050a,
	kD2238_BIST_ADC_ANA_TRIM0	= 0x050b,
	kD2238_VDD_MAIN_MON		= 0x050c,

	kD2238_IBUCK0_ACCUM1        = 0x0514,
	kD2238_IBUCK0_ACCUM2        = 0x0515,
	kD2238_IBUCK0_ACCUM3        = 0x0516,
	kD2238_IBUCK0_COUNT1        = 0x0517,
	kD2238_IBUCK0_COUNT2        = 0x0518,
	kD2238_IBUCK1_ACCUM1        = 0x0519,
	kD2238_IBUCK1_ACCUM2        = 0x051a,
	kD2238_IBUCK1_ACCUM3        = 0x051b,
	kD2238_IBUCK1_COUNT1        = 0x051c,
	kD2238_IBUCK1_COUNT2        = 0x051d,
	kD2238_IBUCK2_ACCUM1        = 0x051e,
	kD2238_IBUCK2_ACCUM2        = 0x052f,
	kD2238_IBUCK2_ACCUM3        = 0x0520,
	kD2238_IBUCK2_COUNT1        = 0x0521,
	kD2238_IBUCK2_COUNT2        = 0x0522,
	
	kDIALOG_APP_TMUX		    = 0x0540,
	
	kDIALOG_RTC_ALARM_A	    	= 0x05c0,
	kDIALOG_RTC_ALARM_B		    = 0x05c1,
	kDIALOG_RTC_ALARM_C		    = 0x05c2,
	kDIALOG_RTC_ALARM_D	    	= 0x05c3,
	kDIALOG_RTC_CONTROL	    	= 0x05c4,
	kDIALOG_RTC_TIMEZONE		= 0x05c5,
	kDIALOG_RTC_SUB_SECOND_A	= 0x05c6,
	kDIALOG_RTC_SUB_SECOND_B	= 0x05c7,
	kDIALOG_RTC_SECOND_A		= 0x05c8,
	kDIALOG_RTC_SECOND_B		= 0x05c9,
	kDIALOG_RTC_SECOND_C		= 0x05ca,
	kDIALOG_RTC_SECOND_D		= 0x05cb,

	kD2238_SPARE_RW0		    = 0x0680,
	kD2238_SPARE_RW_LAST		= 0x0687,
	kD2238_SPARE_RWTOTP0		= 0x06a0,
	kD2238_SPARE_RWTOTP_LAST	= 0x06a7,
	kD2238_SPARE_RWOTP0		    = 0x06c0,
	kD2238_SPARE_RWOTP1	    	= 0x06c1,

	kDIALOG_MEMBYTE0		    = 0x4000,
	kDIALOG_MEMBYTE_LAST		= 0x4027,

        kDIALOG_TEST_ACCESS		= 0x7000,
            kDIALOG_TEST_ACCESS_ENA	= 0x1D,
            kDIALOG_TEST_ACCESS_DIS	= 0x00,
    
	kDIALOG_RAM0			= 0x8000,
			kDIALOG_DIAG_SCRATCH	= kDIALOG_RAM0 + 64,	// keep offset the same as Adi
			kDIALOG_DIAG_SCRATCH_SIZE = 8,
		kDIALOG_VOLTAGE_KNOBS = kDIALOG_DIAG_SCRATCH+kDIALOG_DIAG_SCRATCH_SIZE,
			kDIALOG_VOLTAGE_KNOBS_SIZE=32,

	kDIALOG_RAM_LAST		= 0x80ff,
};

// Synthetic Accumulator Registers 
enum {
	kDIALOG_ACCUMULATOR_SEL_COUNT   = 3,
	kDIALOG_IBUCK0_ACCUMULATOR	= 0,
	kDIALOG_IBUCK1_ACCUMULATOR	= 1,
	kDIALOG_IBUCK2_ACCUMULATOR	= 2,
};

enum {
	kD2238_EVENT_A_TBAT		= (1 << 7),
	kD2238_EVENT_A_VBUS_DET_REM	= (1 << 6),
	kD2238_EVENT_A_VDD_LOW		= (1 << 5),
	kD2238_EVENT_A_VBUS_DET		= (1 << 3),
	kD2238_EVENT_A_ACC_DET		= (1 << 2),
};

enum {
	kD2238_EVENT_B_BTN3_DBL		= (1 << 6),
	kD2238_EVENT_B_BTN2_DBL		= (1 << 5),
	kD2238_EVENT_B_BTN1_DBL		= (1 << 4),
	kD2238_EVENT_B_BUTTON_3		= (1 << 2),
	kD2238_EVENT_B_BUTTON_2		= (1 << 1),
	kD2238_EVENT_B_BUTTON_1		= (1 << 0),
};

enum {
	kD2238_EVENT_C_CHG_HR_OVER	= (1 << 7),
	kD2238_EVENT_C_CHG_HR_UNDER	= (1 << 6),
	kD2238_EVENT_C_CHG_TIMEOUT	= (1 << 4),
	kD2238_EVENT_C_CHG_PRECHG_TIMEOUT	= (1 << 3),
	kD2238_EVENT_C_CHG_END		= (1 << 2),
	kD2238_EVENT_C_CHG_FAST		= (1 << 1),
	kD2238_EVENT_C_CHG_PRE		= (1 << 0),
};

enum {
	kD2238_EVENT_D_GPIO8		= (1 << 7),
	kD2238_EVENT_D_GPIO7		= (1 << 6),
	kD2238_EVENT_D_GPIO6		= (1 << 5),
	kD2238_EVENT_D_GPIO5		= (1 << 4),
	kD2238_EVENT_D_GPIO4		= (1 << 3),
	kD2238_EVENT_D_GPIO3		= (1 << 2),
	kD2238_EVENT_D_GPIO2		= (1 << 1),
	kD2238_EVENT_D_GPIO1		= (1 << 0),
};

enum {
	kD2238_EVENT_E_GPIO15		= (1 << 6),
	kD2238_EVENT_E_GPIO14		= (1 << 5),
	kD2238_EVENT_E_GPIO13		= (1 << 4),
	kD2238_EVENT_E_GPIO12		= (1 << 3),
	kD2238_EVENT_E_GPIO11		= (1 << 2),
	kD2238_EVENT_E_GPIO10		= (1 << 1),
	kD2238_EVENT_E_GPIO9		= (1 << 0),
};

	// EVENT_F has no events

enum {
	kD2238_EVENT_G_T4_RISE		= (1 << 7),
	kD2238_EVENT_G_T4_FALL		= (1 << 6),
	kD2238_EVENT_G_T3_RISE		= (1 << 5),
	kD2238_EVENT_G_T3_FALL		= (1 << 4),
	kD2238_EVENT_G_T2_RISE		= (1 << 3),
	kD2238_EVENT_G_T2_FALL		= (1 << 2),
	kD2238_EVENT_G_T1_RISE		= (1 << 1),
	kD2238_EVENT_G_T1_FALL		= (1 << 0),
};

	// EVENT_H has no events

enum {
	kD2238_EVENT_I_TBUCK3			= (1 << 7),
	kD2238_EVENT_I_TBUCK2			= (1 << 6),
	kD2238_EVENT_I_TBUCK1			= (1 << 5),
	kD2238_EVENT_I_TBUCK0			= (1 << 4),
	kD2238_EVENT_I_TCHARGER			= (1 << 3),
	kD2238_EVENT_I_TLDO4			= (1 << 0),
};

enum {
	kD2238_EVENT_J_HIGH_TEMP_WARNING	= (1 << 7),
	kD2238_EVENT_J_BUCK_LDO_UOV		= (1 << 6),
	kD2238_EVENT_J_IBUCK2_OVERFLOW		= (1 << 2),
	kD2238_EVENT_J_IBUCK1_OVERFLOW		= (1 << 1),
	kD2238_EVENT_J_IBUCK0_OVERFLOW		= (1 << 0),
};

    	// EVENT_K has no events

// EVENT_L events are stored by LLB across sleep/wake
enum {
	kD2238_EVENT_L_CLK32K_ERROR		= (1 << 5),
	kD2238_EVENT_L_HIB			= (1 << 4),
	kD2238_EVENT_L_EOMC			= (1 << 1),
	kD2238_EVENT_L_ALARM			= (1 << 0),
};

	// EVENT_M has no events

enum {
	kD2238_STATUS_A_TBAT			= (1 << 7),
	kD2238_STATUS_A_VDD_LOW			= (1 << 5),
	kD2238_STATUS_A_VBUS_DET		= (1 << 3),
};

enum {
	kD2238_STATUS_B_BUTTON_3	= (1 << 2),
	kD2238_STATUS_B_BUTTON_2	= (1 << 1),
	kD2238_STATUS_B_BUTTON_1	= (1 << 0),
};

enum {
	kD2238_STATUS_C_CHG_HR_OVER		= (1 << 7),
	kD2238_STATUS_C_CHG_HR_UNDER		= (1 << 6),
	kD2238_STATUS_C_CHG_TIMEOUT		= (1 << 4),
	kD2238_STATUS_C_CHG_PRECHG_TIMEOUT	= (1 << 3),
	kD2238_STATUS_C_CHG_END				= (1 << 2),
	kD2238_STATUS_C_CHG_FAST			= (1 << 1),
	kD2238_STATUS_C_CHG_PRE				= (1 << 0),
};

enum {
	kD2238_STATUS_D_GPIO8		= (1 << 7),
	kD2238_STATUS_D_GPIO7		= (1 << 6),
	kD2238_STATUS_D_GPIO6		= (1 << 5),
	kD2238_STATUS_D_GPIO5		= (1 << 4),
	kD2238_STATUS_D_GPIO4		= (1 << 3),
	kD2238_STATUS_D_GPIO3		= (1 << 2),
	kD2238_STATUS_D_GPIO2		= (1 << 1),
	kD2238_STATUS_D_GPIO1		= (1 << 0),
};

enum {
	kD2238_STATUS_E_GPIO15		= (1 << 6),
	kD2238_STATUS_E_GPIO14		= (1 << 5),
	kD2238_STATUS_E_GPIO13		= (1 << 4),
	kD2238_STATUS_E_GPIO12		= (1 << 3),
	kD2238_STATUS_E_GPIO11		= (1 << 2),
	kD2238_STATUS_E_GPIO10		= (1 << 1),
	kD2238_STATUS_E_GPIO9		= (1 << 0),
};

	// STATUS_F has no bits

enum {
	kD2238_STATUS_G_T4_RISE		= (1 << 7),
	kD2238_STATUS_G_T4_FALL		= (1 << 6),
	kD2238_STATUS_G_T3_RISE		= (1 << 5),
	kD2238_STATUS_G_T3_FALL		= (1 << 4),
	kD2238_STATUS_G_T2_RISE		= (1 << 3),
	kD2238_STATUS_G_T2_FALL		= (1 << 2),
	kD2238_STATUS_G_T1_RISE		= (1 << 1),
	kD2238_STATUS_G_T1_FALL		= (1 << 0),
};

	// STATUS_H has no bits

enum {
	kD2238_STATUS_I_TBUCK3			= (1 << 7),
	kD2238_STATUS_I_TBUCK2			= (1 << 6),
	kD2238_STATUS_I_TBUCK1			= (1 << 5),
	kD2238_STATUS_I_TBUCK0			= (1 << 4),
	kD2238_STATUS_I_TCHARGER		= (1 << 3),
	kD2238_STATUS_I_TLDO4			= (1 << 0),
};

enum {
	kD2238_STATUS_J_HIGH_TEMP_WARNING	= (1 << 7),
	kD2238_STATUS_J_BUCK_LDO_UOV		= (1 << 6),
	kD2238_STATUS_J_IBUCK2_OVERFLOW		= (1 << 2),
	kD2238_STATUS_J_IBUCK1_OVERFLOW		= (1 << 1),
	kD2238_STATUS_J_IBUCK0_OVERFLOW		= (1 << 0),
};

	// STATUS_K has no bits

enum {
	kD2238_STATUS_L_CLK32K_ERROR		= (1 << 5),
};

	// STATUS_M has no bits

enum {
	kDIALOG_EVENT_HOLD_BUTTON_MASK	= EVENT_FLAG_MAKE(1, kD2238_EVENT_B_BUTTON_1),
	kDIALOG_EVENT_MENU_BUTTON_MASK	= EVENT_FLAG_MAKE(1, kD2238_EVENT_B_BUTTON_2),
	kDIALOG_EVENT_RINGER_BUTTON_MASK= EVENT_FLAG_MAKE(1, kD2238_EVENT_B_BUTTON_3),

	kDIALOG_EVENT_BUTTON4_MASK  = kDIALOG_NOTEXIST_MASK,
	kDIALOG_EVENT_BUTTONS		= (kD2238_EVENT_B_BUTTON_3 |
					   kD2238_EVENT_B_BUTTON_2 |
					   kD2238_EVENT_B_BUTTON_1),
	kDIALOG_EVENT_BUTTONS_MASK	= EVENT_FLAG_MAKE(1, kDIALOG_EVENT_BUTTONS),
	kDIALOG_EVENT_PWR_BUTTON_MASK	= kDIALOG_EVENT_HOLD_BUTTON_MASK,
	
	kDIALOG_EVENT_ALARM_MASK	= EVENT_FLAG_MAKE(11, kD2238_EVENT_L_ALARM),
	kDIALOG_EVENT_ACC_DET_MASK	= kDIALOG_NOTEXIST_MASK,
	kDIALOG_EVENT_VBUS_DET_MASK	= EVENT_FLAG_MAKE(0, kD2238_EVENT_A_VBUS_DET),
	kDIALOG_EVENT_VBUS_EXT_REM_MASK	= EVENT_FLAG_MAKE(0, kD2238_EVENT_A_VBUS_DET_REM),
	kDIALOG_EVENT_EOMC_MASK		= EVENT_FLAG_MAKE(11, kD2238_EVENT_L_EOMC),
	kDIALOG_EVENT_HIB_MASK		= EVENT_FLAG_MAKE(11, kD2238_EVENT_L_HIB),
	kDIALOG_EVENT_ABCC_MASK		= kDIALOG_NOTEXIST_MASK,
	
	kDIALOG_EVENT_CHG_END_MASK	= EVENT_FLAG_MAKE(2, kD2238_EVENT_C_CHG_END),
	kDIALOG_EVENT_TBAT_MASK		= EVENT_FLAG_MAKE(0, kD2238_EVENT_A_TBAT),
	
	kDIALOG_EVENT_GPIO1_MASK	= EVENT_FLAG_MAKE(3, kD2238_EVENT_D_GPIO1),
	kDIALOG_EVENT_GPIO2_MASK	= EVENT_FLAG_MAKE(3, kD2238_EVENT_D_GPIO2),
	kDIALOG_EVENT_GPIO3_MASK	= EVENT_FLAG_MAKE(3, kD2238_EVENT_D_GPIO3),
	kDIALOG_EVENT_GPIO4_MASK	= EVENT_FLAG_MAKE(3, kD2238_EVENT_D_GPIO4),
	kDIALOG_EVENT_GPIO5_MASK	= EVENT_FLAG_MAKE(3, kD2238_EVENT_D_GPIO5),
	kDIALOG_EVENT_GPIO6_MASK	= EVENT_FLAG_MAKE(3, kD2238_EVENT_D_GPIO6),
	kDIALOG_EVENT_GPIO7_MASK	= EVENT_FLAG_MAKE(3, kD2238_EVENT_D_GPIO7),
	kDIALOG_EVENT_GPIO8_MASK	= EVENT_FLAG_MAKE(3, kD2238_EVENT_D_GPIO8),
	kDIALOG_EVENT_GPIO9_MASK	= EVENT_FLAG_MAKE(4, kD2238_EVENT_E_GPIO9),
	kDIALOG_EVENT_GPIO10_MASK	= EVENT_FLAG_MAKE(4, kD2238_EVENT_E_GPIO10),
	kDIALOG_EVENT_GPIO11_MASK	= EVENT_FLAG_MAKE(4, kD2238_EVENT_E_GPIO11),
	kDIALOG_EVENT_GPIO12_MASK	= EVENT_FLAG_MAKE(4, kD2238_EVENT_E_GPIO12),
	kDIALOG_EVENT_GPIO13_MASK	= EVENT_FLAG_MAKE(4, kD2238_EVENT_E_GPIO13),
	kDIALOG_EVENT_GPIO14_MASK	= EVENT_FLAG_MAKE(4, kD2238_EVENT_E_GPIO14),
	kDIALOG_EVENT_GPIO15_MASK	= EVENT_FLAG_MAKE(4, kD2238_EVENT_E_GPIO15),
	kDIALOG_EVENT_GPIO16_MASK	= kDIALOG_NOTEXIST_MASK,
	kDIALOG_EVENT_GPIO17_MASK	= kDIALOG_NOTEXIST_MASK,
	kDIALOG_EVENT_GPIO18_MASK	= kDIALOG_NOTEXIST_MASK,
	kDIALOG_EVENT_GPIO19_MASK	= kDIALOG_NOTEXIST_MASK,
	kDIALOG_EVENT_GPIO20_MASK	= kDIALOG_NOTEXIST_MASK,
	kDIALOG_EVENT_GPIO21_MASK	= kDIALOG_NOTEXIST_MASK,

	kDIALOG_EVENT_HOLD_DBL_MASK	= EVENT_FLAG_MAKE(1, kD2238_EVENT_B_BTN2_DBL),
	kDIALOG_EVENT_MENU_DBL_MASK	= EVENT_FLAG_MAKE(1, kD2238_EVENT_B_BTN1_DBL),
	kDIALOG_EVENT_RINGER_DBL_MASK	= EVENT_FLAG_MAKE(1, kD2238_EVENT_B_BTN3_DBL),

	kDIALOG_EVENT_VHP_DET_MASK	= kDIALOG_NOTEXIST_MASK,
	kDIALOG_EVENT_ON_MASK		= kDIALOG_NOTEXIST_MASK,
	kDIALOG_EVENT_LDO2_EN_MASK	= kDIALOG_NOTEXIST_MASK,
};

enum {
	kDIALOG_STATUS_VBUS_MASK	= STATUS_FLAG_MAKE(0, kD2238_STATUS_A_VBUS_DET),
	kDIALOG_STATUS_USB_MASK		= kDIALOG_STATUS_VBUS_MASK,
	kDIALOG_STATUS_FW_MASK		= kDIALOG_NOTEXIST_MASK,
	kDIALOG_STATUS_ACC_DET_MASK	= kDIALOG_NOTEXIST_MASK,
	kDIALOG_STATUS_CHARGING_MASK	= STATUS_FLAG_MAKE(3, kD2238_STATUS_C_CHG_PRE | kD2238_STATUS_C_CHG_FAST),
	kDIALOG_STATUS_CHG_TO_MASK	= STATUS_FLAG_MAKE(2, kD2238_STATUS_C_CHG_TIMEOUT),
	kDIALOG_STATUS_CHG_END_MASK	= STATUS_FLAG_MAKE(2, kD2238_STATUS_C_CHG_END),
	kDIALOG_STATUS_TBAT_MASK	= STATUS_FLAG_MAKE(0, kD2238_STATUS_A_TBAT),
	kDIALOG_STATUS_CHG_ATT_MASK     = kDIALOG_NOTEXIST_MASK,
	kDIALOG_STATUS_ABCC_MASK	= kDIALOG_NOTEXIST_MASK,
#define	kDIALOG_STATUS_GPIO_MASK(gpio)	  STATUS_FLAG_MAKE(3 + ((gpio)/8), (1 << ((gpio) % 8)))
};

static const statusRegisters kDialogStatusFWMask = {0, 0, 0, 0, 0, 0};
static const statusRegisters kDialogStatusChargingMask = {0, 0, kD2238_STATUS_C_CHG_PRE | kD2238_STATUS_C_CHG_FAST };

enum {
	kD2238_EVENT_A_WAKEMASK = kD2238_EVENT_A_VBUS_DET,
	kD2238_EVENT_B_WAKEMASK = (kD2238_EVENT_B_BUTTON_1 |
				   kD2238_EVENT_B_BUTTON_2 |
				   kD2238_EVENT_B_BUTTON_3),
	kD2238_EVENT_C_WAKEMASK = 0,
	kD2238_EVENT_D_WAKEMASK = 0,
	kD2238_EVENT_E_WAKEMASK = 0,
	kD2238_EVENT_F_WAKEMASK = 0,
	kD2238_EVENT_G_WAKEMASK = 0,
	kD2238_EVENT_H_WAKEMASK = 0,
	kD2238_EVENT_I_WAKEMASK = 0,
	kD2238_EVENT_J_WAKEMASK = 0,
	kD2238_EVENT_K_WAKEMASK = 0,
	kD2238_EVENT_L_WAKEMASK = kD2238_EVENT_L_ALARM,
};

// All events that are masked during shutdown - inverse of the wake mask,
// events that wake up the system
static const eventRegisters kDialogEventIntMasks = {
	~kD2238_EVENT_A_WAKEMASK,
	~kD2238_EVENT_B_WAKEMASK,
	~kD2238_EVENT_C_WAKEMASK,
	~kD2238_EVENT_D_WAKEMASK,
	~kD2238_EVENT_E_WAKEMASK,
	~kD2238_EVENT_F_WAKEMASK,
	~kD2238_EVENT_G_WAKEMASK,
	~kD2238_EVENT_H_WAKEMASK,
	~kD2238_EVENT_I_WAKEMASK,
	~kD2238_EVENT_J_WAKEMASK,
	~kD2238_EVENT_K_WAKEMASK,
	~kD2238_EVENT_L_WAKEMASK,
};


// All wake events without the buttons
static const eventRegisters kDialogEventNotButtonMasks = {
	kD2238_EVENT_A_WAKEMASK,
	kD2238_EVENT_B_WAKEMASK & ~kDIALOG_EVENT_BUTTONS,
	kD2238_EVENT_C_WAKEMASK,
	kD2238_EVENT_D_WAKEMASK,
	kD2238_EVENT_E_WAKEMASK,
	kD2238_EVENT_F_WAKEMASK,
	kD2238_EVENT_G_WAKEMASK,
	kD2238_EVENT_H_WAKEMASK,
	kD2238_EVENT_I_WAKEMASK,
	kD2238_EVENT_J_WAKEMASK,
	kD2238_EVENT_K_WAKEMASK,
	kD2238_EVENT_L_WAKEMASK,
};

// All events indicating external power supply
static const eventRegisters kDialogEventPwrsupplyMask = {
	kD2238_EVENT_A_VBUS_DET | kD2238_EVENT_A_VBUS_DET_REM,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
};

static const eventRegisters kDialogEventUSBMask = {
        kD2238_EVENT_A_VBUS_DET | kD2238_EVENT_A_VBUS_DET_REM,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
};

static const eventRegisters kDialogEventFWMask = {
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
};

enum {
	kDIALOG_SYS_CONTROL_STANDBY		= (1 << 0),
	kDIALOG_SYS_CONTROL_HIBERNATE		= (1 << 1),
//	kDIALOG_SYS_CONTROL_BAT_PWR_SUSPEND	= 0,
	kDIALOG_SYS_CONTROL_HIB_CLK		= (1 << 4),
	kDIALOG_SYS_CONTROL_PROT_FET_DIS	= 0,
//	kDIALOG_SYS_CONTROL_BUS_PWR_SUSPEND	= 0,
//	kDIALOG_SYS_CONTROL_CHRG_CONTROLS	= 0,
	kDIALOG_SYS_CONTROL_HIBERNATE_ALWAYS    = kDIALOG_SYS_CONTROL_HIBERNATE,
};

enum {
	kDIALOG_SYS_CONTROL2_HIB_32K		= 0,
};

enum {
	// FAULT_LOG1
	kDIALOG_FAULT_LOG_WDOG			= kDIALOG_NOTEXIST_MASK,
	kDIALOG_FAULT_LOG_RESET_IN_3	= kDIALOG_NOTEXIST_MASK,
	kDIALOG_FAULT_LOG_RESET_IN_2	= FAULTLOG_FLAG_MAKE(0, (1 << 5)),
	kDIALOG_FAULT_LOG_RESET_IN_1	= FAULTLOG_FLAG_MAKE(0, (1 << 4)),
	kDIALOG_FAULT_LOG_RST			= FAULTLOG_FLAG_MAKE(0, (1 << 3)),
	kDIALOG_FAULT_LOG_POR			= FAULTLOG_FLAG_MAKE(0, (1 << 2)),
	kDIALOG_FAULT_LOG_OVER_TEMP		= FAULTLOG_FLAG_MAKE(0, (1 << 1)),
	kDIALOG_FAULT_LOG_VDD_UNDER	=	 FAULTLOG_FLAG_MAKE(0, (1 << 0)),
	// FAULT_LOG2
	kDIALOG_FAULT_LOG_BTN_SHUTDOWN      = FAULTLOG_FLAG_MAKE(1, (1 << 2)),
	kDIALOG_FAULT_LOG_TWO_FINGER_RESET  = FAULTLOG_FLAG_MAKE(1, (1 << 1)),
	kDIALOG_FAULT_LOG_NTC_SHDN          = FAULTLOG_FLAG_MAKE(1, (1 << 0)),
};

enum {
	kDIALOG_CHARGE_CONTROL_A_ISET_BAT_MASK	 = 0x7f,
	kDIALOG_CHARGE_CONTROL_A_ISET_BAT_SHIFT  = 0,
//	kDIALOG_OTP_ISET_BAT_SHIFT		= -1,	    // OTP is missing the LSB
	kDIALOG_CHARGE_CONTROL_B_CHG_SUSP	= (1 << 7),
	kDIALOG_CHARGE_CONTROL_CHG_HIB		= 0,
};

enum {
	kDIALOG_CHARGE_CONTROL_STEP		= 3,
	kDIALOG_CHARGE_CONTROL_MAX		= kDIALOG_CHARGE_CONTROL_A_ISET_BAT_MASK * kDIALOG_CHARGE_CONTROL_STEP,
};

enum {
	kDIALOG_CHARGE_CONTROL_ALT_USB_DIS	= 0,
	kDIALOG_CHARGE_CONTROL_CHG_BUCK_EN	= 0,
};

enum {
	kDIALOG_CHARGE_CONTROL_TIME_TCTR_MASK	    = 0xFF,
	kDIALOG_CHARGE_CONTROL_TIME_TCTR_DISABLED   = 0x00,
	kDIALOG_CHARGE_CONTROL_TIME_PCTR_MASK	    = 0x00,	// TODO
	kDIALOG_CHARGE_CONTROL_TIME_PCTR_DISABLED   = 0x00,
};

enum {
	kDIALOG_ADC_LSB_ADC_OVL			= (1 << 7),
	kDIALOG_ADC_LSB_MANADC_ERROR		= (1 << 6),
};

enum {
	kDIALOG_ADC_CONTROL_MUX_SEL_MASK	= (0x7f << 0),
	kDIALOG_ADC_CONTROL_MUX_SEL_VDD_OUT	= (0 << 0),
	kDIALOG_ADC_CONTROL_MUX_SEL_TBAT	= (1 << 0),
	kDIALOG_ADC_CONTROL_MUX_SEL_VBAT	= (2 << 0),
	kDIALOG_ADC_CONTROL_MUX_SEL_TJUNC	= (3 << 0),
	kDIALOG_ADC_CONTROL_MUX_SEL_NTC0	= (4 << 0),
	kDIALOG_ADC_CONTROL_MUX_NUM_NTC		= 5,
	kDIALOG_ADC_CONTROL_MUX_SEL_LDO4_TEMP	= (9 << 0),
	kDIALOG_ADC_CONTROL_MUX_SEL_CHG_TEMP,
	kDIALOG_ADC_CONTROL_MUX_SEL_BUCK0_TEMP,
	kDIALOG_ADC_CONTROL_MUX_SEL_BUCK1_TEMP,
	kDIALOG_ADC_CONTROL_MUX_SEL_BUCK2_TEMP,
	kDIALOG_ADC_CONTROL_MUX_SEL_BUCK3_TEMP,
	kDIALOG_ADC_CONTROL_MUX_SEL_TINT_START	= kDIALOG_ADC_CONTROL_MUX_SEL_LDO4_TEMP,
	kDIALOG_ADC_CONTROL_MUX_SEL_TINT_END	= kDIALOG_ADC_CONTROL_MUX_SEL_BUCK3_TEMP,
	kDIALOG_ADC_CONTROL_MUX_SEL_VLDO1	= (15 << 0),
	kDIALOG_ADC_CONTROL_MUX_SEL_ILDO1,
	kDIALOG_ADC_CONTROL_MUX_SEL_VLDO2,
	kDIALOG_ADC_CONTROL_MUX_SEL_ILDO2,
	kDIALOG_ADC_CONTROL_MUX_SEL_VLDO3,
	kDIALOG_ADC_CONTROL_MUX_SEL_ILDO3,
	kDIALOG_ADC_CONTROL_MUX_SEL_VLDO4,
	kDIALOG_ADC_CONTROL_MUX_SEL_ILDO4,
	kDIALOG_ADC_CONTROL_MUX_SEL_VLDO5,
	kDIALOG_ADC_CONTROL_MUX_SEL_ILDO5,
	kDIALOG_ADC_CONTROL_MUX_SEL_VLDO6,
	kDIALOG_ADC_CONTROL_MUX_SEL_ILDO6,
	kDIALOG_ADC_CONTROL_MUX_SEL_VLDO7,
	kDIALOG_ADC_CONTROL_MUX_SEL_ILDO7,
	kDIALOG_ADC_CONTROL_MUX_SEL_VLDO8,
	kDIALOG_ADC_CONTROL_MUX_SEL_ILDO8,
	kDIALOG_ADC_CONTROL_MUX_SEL_VLDO9,
	kDIALOG_ADC_CONTROL_MUX_SEL_ILDO9,
	kDIALOG_ADC_CONTROL_MUX_SEL_VRTC,
	kDIALOG_ADC_CONTROL_MUX_SEL_IRTC,
	kDIALOG_ADC_CONTROL_MUX_SEL_CHG_HR,
	kDIALOG_ADC_CONTROL_MUX_SEL_VBUCK0,
	kDIALOG_ADC_CONTROL_MUX_SEL_VBUCK1,
	kDIALOG_ADC_CONTROL_MUX_SEL_VBUCK2,
	kDIALOG_ADC_CONTROL_MUX_SEL_VBUCK3,
	kDIALOG_ADC_CONTROL_MUX_SEL_IBUCK0,
	kDIALOG_ADC_CONTROL_MUX_SEL_IBUCK1,
	kDIALOG_ADC_CONTROL_MUX_SEL_IBUCK2,
	kDIALOG_ADC_CONTROL_MUX_SEL_IBUCK3,
	kDIALOG_ADC_CONTROL_MUX_SEL_VSW0,
	kDIALOG_ADC_CONTROL_MUX_SEL_VSW1,
	kDIALOG_ADC_CONTROL_MUX_SEL_VSW2,
	kDIALOG_ADC_CONTROL_MUX_SEL_VSW3A,
	kDIALOG_ADC_CONTROL_MUX_SEL_VSW3B,
	kDIALOG_ADC_CONTROL_MUX_SEL_VSW3C,
	kDIALOG_ADC_CONTROL_MUX_SEL_VSW4,
	kDIALOG_ADC_CONTROL_MUX_SEL_APP_MUX,
	kDIALOG_ADC_CONTROL_MUX_SEL_IBUCK0_OFF,
	kDIALOG_ADC_CONTROL_MUX_SEL_IBUCK1_OFF,
	kDIALOG_ADC_CONTROL_MUX_SEL_IBUCK2_OFF,
	kDIALOG_ADC_CONTROL_MUX_SEL_IBUCK3_OFF,
	kDIALOG_ADC_CONTROL_MUX_SEL_ICH,
	kDIALOG_ADC_CONTROL_MUX_SEL_CHG_HR_OFF,
	kDIALOG_ADC_CONTROL_MUX_SEL_ISW0,
	kDIALOG_ADC_CONTROL_MUX_SEL_ISW1,
	kDIALOG_ADC_CONTROL_MUX_SEL_ISW2,
	kDIALOG_ADC_CONTROL_MUX_SEL_ISW3A,
	kDIALOG_ADC_CONTROL_MUX_SEL_ISW3B,
	kDIALOG_ADC_CONTROL_MUX_SEL_ISW3C,
	kDIALOG_ADC_CONTROL_MUX_SEL_ISW4,
	kDIALOG_ADC_CONTROL_MUX_SEL_ISW_OFF_0,
	kDIALOG_ADC_CONTROL_MUX_SEL_ISW_OFF_1,
	kDIALOG_ADC_CONTROL_MUX_SEL_ISW_OFF_2,
	kDIALOG_ADC_CONTROL_MUX_SEL_ISW_OFF_3A,
	kDIALOG_ADC_CONTROL_MUX_SEL_ISW_OFF_3B,
	kDIALOG_ADC_CONTROL_MUX_SEL_ISW_OFF_3C,
	kDIALOG_ADC_CONTROL_MUX_SEL_ISW_OFF_4,
	kDIALOG_ADC_CONTROL_MUX_SEL_APP_MUX_BC,
	kDIALOG_ADC_CONTROL_MAN_CONV		= (1 << 7),
	kDIALOG_ADC_CONTROL2_ADC_REF_EN		= (1 << 6),
	kDIALOG_ADC_CONTROL2_AUTO_VDD_OUT_EN	= (1 << 7),
};

enum {
	kDIALOG_ADC_CONTROL_DEFAULTS	= 0,
	kDIALOG_ADC_CONTROL2_DEFAULTS	= 0,
};

enum {
	kDIALOG_ADC_RESOLUTION_BITS	= 12,
	kDIALOG_ADC_FULL_SCALE_MV	= 2500,
};


enum {
	kDIALOG_APP_TMUX_SEL_MASK		= (7 << 0),
	kDIALOG_APP_TMUX_SEL_AMUX_0		= (0 << 0),
	kDIALOG_APP_TMUX_SEL_AMUX_1		= (1 << 0),
	kDIALOG_APP_TMUX_SEL_AMUX_2		= (2 << 0),
	kDIALOG_APP_TMUX_SEL_AMUX_3		= (3 << 0),
	kDIALOG_APP_TMUX_SEL_AMUX_4		= (4 << 0),
	kDIALOG_APP_TMUX_SEL_AMUX_5		= (5 << 0),
	kDIALOG_APP_TMUX_SEL_AMUX_6		= (6 << 0),
	kDIALOG_APP_TMUX_SEL_AMUX_7		= (7 << 0),
	KDIALOG_APP_TMUX_EN			= (1 << 7),
};

enum {
	kDIALOG_RTC_CONTROL_MONITOR		= (1 << 0),
	kDIALOG_RTC_CONTROL_ALARM_EN		= (1 << 6),
};

enum {
	kDIALOG_SYS_GPIO_OUTPUT_LEVEL_LOW		= (0 << 1),
	kDIALOG_SYS_GPIO_OUTPUT_LEVEL_HIGH		= (1 << 1),

	kDIALOG_SYS_GPIO_INPUT_WAKE			= (1 << 1),

	kDIALOG_SYS_GPIO_DIRECTION_MASK			= (0x1f << 3),
	kDIALOG_SYS_GPIO_DIRECTION_OUT			= (0 << 3),
	kDIALOG_SYS_GPIO_DIRECTION_OUT_32KHZ		= (2 << 3),
	kDIALOG_SYS_GPIO_DIRECTION_IN_LEVEL_HIGH	= (0x10 << 3),
	kDIALOG_SYS_GPIO_DIRECTION_IN_LEVEL_LOW		= (0x11 << 3),
};

#define IS_GPIO_OUTPUT(gpio) (((gpio)&kDIALOG_SYS_GPIO_DIRECTION_MASK) \
				< kDIALOG_SYS_GPIO_DIRECTION_IN_LEVEL_HIGH)

#define NUM_LDOS 16
static const struct ldo_params ldo_d2238[NUM_LDOS] = {
	{ 1200, 25, 0x54, 0x7f, 0x00, kD2238_LDO1_VSEL,  kD2238_ACTIVE3, 0x02 },	// 00 - LDO1
	{ 1200, 25, 0x54, 0x7f, 0x00, kD2238_LDO2_VSEL,  kD2238_ACTIVE3, 0x04 },	// 01 - LDO2
	{ 1200, 25, 0x54, 0x7f, 0x00, kD2238_LDO3_VSEL,  kD2238_ACTIVE3, 0x08 },	// 02 - LDO3
	{ 2400, 25, 0x5c, 0x7f, 0x00, kD2238_LDO4_VSEL,  kD2238_ACTIVE3, 0x10 },	// 03 - LDO4
	{ 1200, 25, 0x54, 0x7f, 0x00, kD2238_LDO5_VSEL,  kD2238_ACTIVE3, 0x20 },	// 04 - LDO5
	{ 1200, 25, 0x54, 0x7f, 0x00, kD2238_LDO6_VSEL,  kD2238_ACTIVE3, 0x40 },	// 05 - LDO6
	{ 1200, 25, 0x54, 0x7f, 0x00, kD2238_LDO7_VSEL,  kD2238_ACTIVE3, 0x80 },	// 06 - LDO7
	{ 1200, 25, 0x54, 0x7f, 0x00, kD2238_LDO8_VSEL,  kD2238_ACTIVE4, 0x01 },	// 07 - LDO8
	{ 1200, 25, 0x54, 0x7f, 0x00, kD2238_LDO9_VSEL,  0, 0 },			// 08 - LDO9 (no ACTIVE control)

	{ 0,    0,  0,    0,    0x00, 0,		kD2238_ACTIVE6, 0x01 },		// 09 - SW0
	{ 0,    0,  0,    0,    0x00, 0,		kD2238_ACTIVE6, 0x02 },		// 0a - SW1
	{ 0,    0,  0,    0,    0x00, 0,		kD2238_ACTIVE6, 0x04 },		// 0b - SW2
	{ 0,    0,  0,    0,    0x00, 0,		kD2238_ACTIVE6, 0x08 },		// 0c - SW3A
	{ 0,    0,  0,    0,    0x00, 0,		kD2238_ACTIVE6, 0x10 },		// 0d - SW3B
	{ 0,    0,  0,    0,    0x00, 0,		kD2238_ACTIVE6, 0x20 },		// 0e - SW3C
	{ 0,    0,  0,    0,    0x00, 0,		kD2238_ACTIVE6, 0x40 },		// 0f - SW4

};
#define LDOP ldo_d2238

enum {
	kDIALOG_BUTTON_DBL_CLICK_RATE_MASK	= (7 << 0),
	kDIALOG_BUTTON_DBL_CLICK_RATE_50MS	= (0 << 0),
	kDIALOG_BUTTON_DBL_CLICK_RATE_100MS	= (1 << 0),
	kDIALOG_BUTTON_DBL_CLICK_RATE_150MS	= (2 << 0),
	kDIALOG_BUTTON_DBL_CLICK_RATE_200MS	= (3 << 0),
	kDIALOG_BUTTON_DBL_CLICK_RATE_250MS	= (4 << 0),
	kDIALOG_BUTTON_DBL_CLICK_RATE_300MS	= (5 << 0),
	kDIALOG_BUTTON_DBL_CLICK_RATE_350MS	= (6 << 0),
	kDIALOG_BUTTON_DBL_CLICK_RATE_400MS	= (7 << 0),
	kD2238_BUTTON_DBL_BTN1_DBL_EN		= (1 << 3),
	kD2238_BUTTON_DBL_BTN2_DBL_EN		= (1 << 4),
	kD2238_BUTTON_DBL_BTN3_DBL_EN		= (1 << 5),
	kD2238_BUTTON_DBL_BTN4_DBL_EN		= 0,
	kDIALOG_BUTTON_DBL_HOLD_DBL_EN		= kD2238_BUTTON_DBL_BTN2_DBL_EN,
	kDIALOG_BUTTON_DBL_MENU_DBL_EN		= kD2238_BUTTON_DBL_BTN1_DBL_EN,
	kDIALOG_BUTTON_DBL_RINGER_DBL_EN	= kD2238_BUTTON_DBL_BTN3_DBL_EN,
};

#endif /* __DIALOG_D2238_H */
