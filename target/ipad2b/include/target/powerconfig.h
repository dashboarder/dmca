/*
 * Copyright (C) 2010 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */
#ifndef __TARGET_POWERCONFIG_H
#define __TARGET_POWERCONFIG_H

#include <drivers/gasgauge.h>

/* configuration per target */

#define PMU_IIC_BUS		0

/* this configuration is very PMU specific and must be included exactly once in power.c */
#ifdef POWERCONFIG_PMU_SETUP

static const struct pmu_setup_struct pmu_ldo_warm_setup[] =
{
	{ PMU_IIC_BUS, kDIALOG_LDO_CONTROL,	0x00 }, // Force bypass off
	{ PMU_IIC_BUS, kD1946_LDO1,		0x0A },	// PP3V0_SPARE1		3.00V
	{ PMU_IIC_BUS, kD1946_LDO2,		0x0A },	// PP1V7_VA_VCP		1.70V
	{ PMU_IIC_BUS, kD1946_LDO3,		0x0A },	// PP3V0_S2R_TRISTAR	3.00V
	{ PMU_IIC_BUS, kD1946_LDO4,		0x18 },	// PP3V0_SENSOR		3.00V
	{ PMU_IIC_BUS, kD1946_LDO5,		0x0E },	// PP3V2_SPARE2		3.20V
	{ PMU_IIC_BUS, kD1946_LDO6,		0x10 },	// PP3V3_ACC		3.30V
	{ PMU_IIC_BUS, kD1946_LDO7,		0x03 },	// PP1V8_CAM		1.80V
	{ PMU_IIC_BUS, kD1946_LDO8,		0x14 },	// PP3V0_S2R_HALL	3.00V
	{ PMU_IIC_BUS, kD1946_LDO9,		0x01 },	// PP1V2_CAM		1.30V
	{ PMU_IIC_BUS, kD1946_LDO10,		0x02 },	// PP2V8_CAM_AF		2.60V
	{ PMU_IIC_BUS, kD1946_LDO11,		0x16 },	// PP2V8_CAM 		2.80V
	{ PMU_IIC_BUS, kD1946_LDO12,		0x10 },	// PP1V0		1.00V
	{ PMU_IIC_BUS, kDIALOG_LDO_CONTROL,	0x00 }, // Leave bypass off
};

static const struct pmu_setup_struct pmu_ldo_cold_setup[] =
{
	{ PMU_IIC_BUS, kD1946_ACTIVE1, 		0xDD }, // Everything but BUCK1 and LDO1
	{ PMU_IIC_BUS, kD1946_ACTIVE2, 		0xB9 }, // LDO5, 6, and 10 are off
	{ PMU_IIC_BUS, kD1946_ACTIVE3, 		0x61 }, // LDO12, Buck5, and WLED Boost
	{ PMU_IIC_BUS, kD1946_ACTIVE4, 		0xA0 }, // CPU1V8_SW and CPU1V2_SW
	{ PMU_IIC_BUS, kD1946_LCM_CONTROL1,	0x05 }, // unused		5.25V
	{ PMU_IIC_BUS, kD1946_LCM_CONTROL2,	0x0E }, // unused		5.70V
	{ PMU_IIC_BUS, kD1946_LCM_CONTROL3,	0x00 }, // BB_VBUS_DET		5.00V
	{ PMU_IIC_BUS, kD1946_LCM_BST_CONTROL,	0x14 }, // VBOOST_LCM		6.00V
};

static const struct pmu_setup_struct pmu_warm_init[] =
{
	{ PMU_IIC_BUS, kDIALOG_SYS_CONTROL2,	0x10 },  // HIB_32K on, DWI off

	{ PMU_IIC_BUS, kDIALOG_IRQ_MASK_A,	0x00 },
	{ PMU_IIC_BUS, kDIALOG_IRQ_MASK_B,	0x00 },
	{ PMU_IIC_BUS, kDIALOG_IRQ_MASK_C,	0x00 },
	{ PMU_IIC_BUS, kDIALOG_IRQ_MASK_D,	0x00 },
	{ PMU_IIC_BUS, kDIALOG_IRQ_MASK_E,	0x00 },
	{ PMU_IIC_BUS, kDIALOG_IRQ_MASK_F,	0x00 },
	{ PMU_IIC_BUS, kDIALOG_IRQ_MASK_G,	0x00 },
	{ PMU_IIC_BUS, kDIALOG_IRQ_MASK_H,	0x00 },
	{ PMU_IIC_BUS, kDIALOG_IRQ_MASK_K,	0x00 },
};

static const struct pmu_setup_struct pmu_cold_init[] =
{
	{ PMU_IIC_BUS, kD1946_BUCK4,		0x9B }, // BUCK4		1.21V
	{ PMU_IIC_BUS, kD1946_BUCK4_HIB,	0x9B }, // BUCK4_HIB		1.21V

	{ PMU_IIC_BUS, kD1946_SYS_GPIO_1,	0x11 }, // PMU_CLK_32K_CUMULUS		Out 0, Push-Pull, CPU_1V8
	{ PMU_IIC_BUS, kD1946_SYS_GPIO_2,	0x49 }, // PMU_CLK_32K_WLAN		CLK32K, Pull Down, VBUCK3
	{ PMU_IIC_BUS, kD1946_SYS_GPIO_3,	0x09 }, // PMU_GPIO_BT_REG_ON		Out 0, Push-Pull, VBUCK3
	{ PMU_IIC_BUS, kD1946_SYS_GPIO_4,	0x09 }, // PMU_GPIO_WLAN_REG_ON		Out 0, Push-Pull, VBUCK3
	{ PMU_IIC_BUS, kD1946_SYS_GPIO_5,	0x18 }, // PMU_GPIO_BB_RST_L		Out 0, OD/PU, Disable Pull-Up
	{ PMU_IIC_BUS, kD1946_SYS_GPIO_6,	0xDA }, // UART5_BATT_RTXD		In, F Edge, Wake, Pull Up, Disable Pull-Up
	{ PMU_IIC_BUS, kD1946_SYS_GPIO_7,	0xBA }, // PMU_GPIO_BT_HOST_WAKE	In, R Edge, Wake, Pull Down, Disable Pull-Up
	{ PMU_IIC_BUS, kD1946_SYS_GPIO_8,	0xBA }, // PMU_GPIO_WLAN_HOST_WAKE	In, R Edge, Wake, Pull Down, Disable Pull-Up
#if TARGET_HAS_BASEBAND
	{ PMU_IIC_BUS, kD1946_SYS_GPIO_9,	0xBA }, // PMU_GPIO_BB_WAKE		In, R Edge, Wake, No Pull Down, Disable Pull-Up
#else
	{ PMU_IIC_BUS, kD1946_SYS_GPIO_9,	0xB9 }, // PMU_GPIO_BB_WAKE		In, R Edge, No Wake, Pull Down, Disable Pull-Up
#endif
	{ PMU_IIC_BUS, kD1946_SYS_GPIO_10,	0xEA }, // PMU_GPIO_CODEC_HS_IRQ_L	In, Any Edge, Wake, Pull Up, VBUCK3
	{ PMU_IIC_BUS, kD1946_SYS_GPIO_11,	0xC8 }, // PMU_GPIO_CODEC_RST_L		In, F Edge, No Wake, VBUCK3 (for internal PU)
	{ PMU_IIC_BUS, kD1946_SYS_GPIO_12,	0xB9 }, // PMU_GPIO_TRISTAR_IRQ 	In, R Edge, No Wake, Pull Down
	{ PMU_IIC_BUS, kD1946_SYS_GPIO_13,	0xFA }, // PMU_GPIO_HALL_IRQ_1		In, Change, Wake, Pull Up
	{ PMU_IIC_BUS, kD1946_SYS_GPIO_14,	0xFA }, // PMU_GPIO_HALL_IRQ_2		In, Change, Wake, Pull Up
	{ PMU_IIC_BUS, kD1946_SYS_GPIO_15,	0xB9 }, // PMU_GPIO_HALL_IRQ_3		In, R Edge, No Wake, Pull Down, Disable Pull-Up
	{ PMU_IIC_BUS, kD1946_SYS_GPIO_16,	0xF8 }, // PMU_GPIO_HALL_IRQ_4		In, Change, No Wake, Disable Pull-Up
	{ PMU_IIC_BUS, kD1946_SYS_GPIO_17,	0xB9 }, // unused			In, R Edge, No Wake, Pull Down, Disable Pull-Up
	{ PMU_IIC_BUS, kD1946_SYS_GPIO_18,	0xB9 }, // unused			In, R Edge, No Wake, Pull Down, Disable Pull-Up

	{ PMU_IIC_BUS, kD1946_SYS_GPIO_DEB1,	0x12 }, // 10ms debounce
	{ PMU_IIC_BUS, kD1946_SYS_GPIO_DEB2,	0x12 }, // 10ms debounce
	{ PMU_IIC_BUS, kD1946_SYS_GPIO_DEB3,	0x12 }, // 10ms debounce
	{ PMU_IIC_BUS, kD1946_SYS_GPIO_DEB4,	0x12 }, // 10ms debounce
	{ PMU_IIC_BUS, kD1946_SYS_GPIO_DEB5,	0x12 }, // 10ms debounce
	{ PMU_IIC_BUS, kD1946_SYS_GPIO_DEB6,	0x12 }, // 10ms debounce
	{ PMU_IIC_BUS, kD1946_SYS_GPIO_DEB7,	0x12 }, // 10ms debounce
	{ PMU_IIC_BUS, kD1946_SYS_GPIO_DEB8,	0x12 }, // 10ms debounce
	{ PMU_IIC_BUS, kD1946_SYS_GPIO_DEB9,	0x12 }, // 10ms debounce

	// <rdar://problem/8932682>
	{ PMU_IIC_BUS, kDIALOG_BANKSEL,		0x02 },
	{ PMU_IIC_BUS, kD1946_TEST_MODE,	0x01 },	// Enable test mode
	{ PMU_IIC_BUS, kDIALOG_BANKSEL,		0x01 },
	{ PMU_IIC_BUS, kD1946_WLED_CONTROL5,	0x00 },	// set WLED to IDAC always
	{ PMU_IIC_BUS, kDIALOG_BANKSEL,		0x02 },
	{ PMU_IIC_BUS, kD1946_TEST_MODE,	0x00 },	// Disable test mode
	{ PMU_IIC_BUS, kDIALOG_BANKSEL,		0x00 },
};

static const struct pmu_setup_struct pmu_backlight_enable[] =
{
	{ PMU_IIC_BUS, kDIALOG_WLED_ISET,	0xD4 },	// 50% value from OS
	{ PMU_IIC_BUS, kDIALOG_WLED_ISET2,	0x00 },
	{ PMU_IIC_BUS, kDIALOG_WLED_CONTROL,		// enable all LED strings
		(kDIALOG_WLED_CONTROL_WLED_ENABLE1 |
		 kDIALOG_WLED_CONTROL_WLED_ENABLE2 |
		 kDIALOG_WLED_CONTROL_WLED_ENABLE3 |
		 kDIALOG_WLED_CONTROL_WLED_ENABLE4 |
		 kDIALOG_WLED_CONTROL_WLED_ENABLE5 |
		 kDIALOG_WLED_CONTROL_WLED_ENABLE6) },
	{ PMU_IIC_BUS, kDIALOG_WLED_CONTROL2,		// enable LED Power
		(kDIALOG_WLED_CONTROL2_WLED_RAMP_EN |
		 kDIALOG_WLED_CONTROL2_WLED_DITH_EN) }
};

static const struct pmu_setup_struct pmu_backlight_disable[] =
{
	{ PMU_IIC_BUS, kDIALOG_WLED_CONTROL, 0x0 }    // disable LED power
};

static const struct core_rails_struct soc_rails[] =
{
	{ PMU_IIC_BUS, 0, kDIALOG_BUCK2 }
};

static const struct core_rails_struct cpu_rails[] =
{
	{ PMU_IIC_BUS, 0, kDIALOG_BUCK0 }
};

#endif /* POWERCONFIG_PMU_SETUP */

#define TARGET_USE_CHARGE_TABLE 1
static const struct power_charge_limits pmu_charge_table[16] = { // gas gauge can have up to 16 entries
	{ 4000,			1000, 1600,	2520	},  // 10 - 16C
	{ 4100,			1000, 1600,	2240	},
	{ USHRT_MAX,	1000, 1600,	1320	},
	{ 4000,			1500, 2100,	2520	},  // 15 - 21C
	{ 4100,			1500, 2100,	2520	},
	{ USHRT_MAX,	1500, 2100,	1320	},
	{ 4000,			2000, 3500,	2520	},  // 20 - 35C
	{ 4100,			2000, 3500,	2520	},
	{ USHRT_MAX,	2000, 3500,	2240	},
	{ 4000,			3400, 4500,	2520	},  // 34 - 45C
	{ 4100,			3400, 4500,	2520	},
	{ USHRT_MAX,	3400, 4500,	2240	},
};

// matches OTP for ICHG_TBAT_x
#define TARGET_ICHG_TBAT_MAX { 0x00, 0x00, 0x3f, 0x3f, 0x3f }

#define NO_BATTERY_VOLTAGE		2700
#define MIN_BOOT_BATTERY_VOLTAGE	3600
#define TARGET_BOOT_BATTERY_VOLTAGE	3700
#define TARGET_BOOT_BATTERY_CAPACITY	50
#define PRECHARGE_BACKLIGHT_LEVEL	1287	/* 10% brightness */
#define TARGET_PRECHARGE_ALWAYS_DISPLAY_IDLE	1
#define TARGET_MAX_USB_INPUT_CURRENT	2100

#define GASGAUGE_BATTERYID_BLOCK	1
#define GASGAUGE_CHARGETABLE_BLOCK	    3
static const unsigned int atv_voltage_limit[16] = { 3600, 3650, 3700, 3750, 3800, 3850, 3900, 3950, 4000, 4050, 4100, 4150, 4200, 4250, 4300, 4350 };
static const unsigned int atv_current_limit[16] = { 0, 200, 440, 640, 880, 1120, 1320, 1560, 1760, 2000, 2240, 2440, 2520, 2520, 2520, 2520 };

#define ACC_PWR_LDO			(6)

#endif
