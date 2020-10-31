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
	{ PMU_IIC_BUS, kD1946_LDO1,		0x0A },	// PP3V0_GRAPE		3.00V
	{ PMU_IIC_BUS, kD1946_LDO2,		0x0A },	// PP1V7_VA_VCP		1.70V
	{ PMU_IIC_BUS, kD1946_LDO3,		0x0A },	// PP3V0_VIDEO		3.00V
	{ PMU_IIC_BUS, kD1946_LDO4,		0x18 },	// PP3V0_OPTICAL	3.00V
	{ PMU_IIC_BUS, kD1946_LDO5,		0x0E },	// PP3V2_SD		3.20V
	{ PMU_IIC_BUS, kD1946_LDO6,		0x10 },	// PP3V3_ACC		3.30V
	{ PMU_IIC_BUS, kD1946_LDO7,		0x0F },	// PP3V0_VIDEO_BUF	3.00V
	{ PMU_IIC_BUS, kD1946_LDO8,		0x18 },	// PP3V2_S2R_USBMUX	3.20V
	{ PMU_IIC_BUS, kD1946_LDO9,		0x12 },	// PP3V0_IO		3.00V
	{ PMU_IIC_BUS, kD1946_LDO10,		0x0A },	// PP3V2_SD_CTRL	3.00V
	{ PMU_IIC_BUS, kD1946_LDO11,		0x17 },	// PP2V85_CAM 		2.85V
	{ PMU_IIC_BUS, kD1946_LDO12,		0x14 },	// PP1V1		1.10V
	{ PMU_IIC_BUS, kDIALOG_LDO_CONTROL,	0x00 }, // Leave bypass off
};

static const struct pmu_setup_struct pmu_ldo_cold_setup[] =
{
	{ PMU_IIC_BUS, kD1946_ACTIVE1, 		0xDD }, // Everything but BUCK1 and LDO1
	{ PMU_IIC_BUS, kD1946_ACTIVE2, 		0xF9 }, // Everything but SD and ACC
	{ PMU_IIC_BUS, kD1946_ACTIVE3, 		0x61 }, // BUCK5, VBOOST_LCM, PP1V1
	{ PMU_IIC_BUS, kD1946_ACTIVE4, 		0xA0 }, // CPU1V8_SW and CPU1V2_SW
	{ PMU_IIC_BUS, kD1946_LCM_CONTROL1,	0x02 }, // unused		5.10V
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

	{ PMU_IIC_BUS, kD1946_SYS_GPIO_1,	0x11 }, // CLK_32K_PMU	  	Out 0, Push-Pull, CPU_1V8
	{ PMU_IIC_BUS, kD1946_SYS_GPIO_2,	0x49 }, // CLK_32K_WLAN		CLK32K, Pull Down, VBUCK3
	{ PMU_IIC_BUS, kD1946_SYS_GPIO_3,	0x09 }, // RST_BT_L		Out 0, Push-Pull, VBUCK3
	{ PMU_IIC_BUS, kD1946_SYS_GPIO_4,	0x09 }, // RST_WLAN_L		Out 0, Push-Pull, VBUCK3
	{ PMU_IIC_BUS, kD1946_SYS_GPIO_5,	0x09 }, // RST_BB_PMU_L		Out 0, Push-pull, VBUCK3
	{ PMU_IIC_BUS, kD1946_SYS_GPIO_6,	0xDA }, // BATTERY_SWI		In, F Edge, Wake, Pull Up, Disable Pull-Up
	{ PMU_IIC_BUS, kD1946_SYS_GPIO_7,	0xBB }, // PM_BT_HOST_WAKE	In, R Edge, Wake, Pull Down, Disable Pull-Up
	{ PMU_IIC_BUS, kD1946_SYS_GPIO_8,	0xBB }, // PM_WLAN_HOST_WAKE	In, R Edge, Wake, Pull Down, Disable Pull-Up
	{ PMU_IIC_BUS, kD1946_SYS_GPIO_9,	0xBB }, // PM_BB_HOST_WAKE	In, R Edge, Wake, Pull Down, Disable Pull-Up
	{ PMU_IIC_BUS, kD1946_SYS_GPIO_10,	0xEA }, // AUD_MIK_HS1_INT_L	In, Change, Wake, Pull Up, VBUCK3
	{ PMU_IIC_BUS, kD1946_SYS_GPIO_11,	0x09 }, // DOCK_BB_EN		Out 0, Push-Pull, VBUCK3
	{ PMU_IIC_BUS, kD1946_SYS_GPIO_12,	0x49 }, // CLK_32K_GPS		CLK32K, Push-Pull, VBUCK3
	{ PMU_IIC_BUS, kD1946_SYS_GPIO_13,	0xB9 }, // SD_CON_CD		In, R Edge, No Wake, Pull Down, Disable Pull-Up
	{ PMU_IIC_BUS, kD1946_SYS_GPIO_14,	0xC8 }, // RST_L63_L		In, F Edge, No Wake, Pull Up, VBUCK3
	{ PMU_IIC_BUS, kD1946_SYS_GPIO_15,	0xFA }, // HALL_IRQ		In, E Edge, Wake, Pull Up, Disable Pull-Up
	{ PMU_IIC_BUS, kD1946_SYS_GPIO_16,	0xB9 }, // unused		In, R Edge, No Wake, Pull Down, Disable Pull-Up
	{ PMU_IIC_BUS, kD1946_SYS_GPIO_17,	0xB9 }, // unused		In, R Edge, No Wake, Pull Down, Disable Pull-Up
	{ PMU_IIC_BUS, kD1946_SYS_GPIO_18,	0xB9 }, // unused		In, R Edge, No Wake, Pull Down, Disable Pull-Up

	{ PMU_IIC_BUS, kD1946_SYS_GPIO_DEB1,	0x12 }, // 10ms debounce
	{ PMU_IIC_BUS, kD1946_SYS_GPIO_DEB2,	0x12 }, // 10ms debounce
	{ PMU_IIC_BUS, kD1946_SYS_GPIO_DEB3,	0x02 }, // no debounce on GPIO6
	{ PMU_IIC_BUS, kD1946_SYS_GPIO_DEB4,	0x10 }, // no debounce on GPIO7
	{ PMU_IIC_BUS, kD1946_SYS_GPIO_DEB5,	0x02 }, // no debounce on GPIO10
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

	// <rdar://problem/8784560>
	{ PMU_IIC_BUS, kDIALOG_BANKSEL,		0x01 }, // Select Bank 1
	{ PMU_IIC_BUS, kD1946_BUCK_CONTROL1,	0x0e }, // Force Buck0 into PWM Mode
	{ PMU_IIC_BUS, kD1946_BUCK_CONTROL3,	0x0e }, // Force Buck2 into PWM Mode
	{ PMU_IIC_BUS, kDIALOG_BANKSEL,		0x00 }, // Select Bank 0
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
	{ 4000,		1000, 1600,	3000	},  // 10 - 16C	    0.44C
	{ 4100,		1000, 1600,	1375	},  //		    0.2C
	{ USHRT_MAX,	1000, 1600,	750	},  //		    0.11C
	{ 4000,		1500, 2100,	3000	},  // 15 - 21C	    0.44C
	{ 4100,		1500, 2100,	2000	},  //		    0.29C
	{ USHRT_MAX,	1500, 2100,	1375	},  //		    0.2C
	{ 4000,		2000, 3500,	3000	},  // 20 - 35C	    0.44C
	{ 4100,		2000, 3500,	3000	},  //		    0.44C
	{ USHRT_MAX,	2000, 3500,	2000	},  //		    0.29C
	{ 4000,		3400, 4500,	3000	},  // 34 - 45C	    0.44C
	{ 4100,		3400, 4500,	3000	},  //		    0.44C
	{ USHRT_MAX,	3400, 4500,	3000	},  //		    0.44C
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
static const unsigned int atv_current_limit[16] = { 0, 0, 625, 750, 875, 1000, 1125, 1250, 1375, 1500, 1750, 2000, 2250, 2500, 2750, 3000 };

#endif
