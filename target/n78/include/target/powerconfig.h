/*
 * Copyright (C) 2010-2011 Apple Inc. All rights reserved.
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
#include <stdbool.h>

/* configuration per target */

#define PMU_IIC_BUS		0

/* this configuration is very PMU specific and must be included exactly once in power.c */
#ifdef POWERCONFIG_PMU_SETUP

/* Matched to D1881B1-OTP-CD-v5 */

static const struct pmu_setup_struct pmu_ldo_warm_setup[] =
{
	{ PMU_IIC_BUS, kD1881_LDO1,		0x0D },	// PP3V0_USB		3.15V <rdar://problem/9200307>
	{ PMU_IIC_BUS, kD1881_LDO2,		0x0A },	// PP1V7_VA_VCP		1.70V
	{ PMU_IIC_BUS, kD1881_LDO3,		0x0A },	// PP3V0_IMU		3.00V
	{ PMU_IIC_BUS, kD1881_LDO4,		0x16 },	// PP3V0_OPTICAL	2.90V
	{ PMU_IIC_BUS, kD1881_LDO5,		0x0A },	// PP3V0_NAND		3.00V
	{ PMU_IIC_BUS, kD1881_LDO6,		0x10 },	// PP3V3_ACC		3.30V
	{ PMU_IIC_BUS, kD1881_LDO7,		0x0F },	// PP3V3_VIDEO		3.00V
	{ PMU_IIC_BUS, kD1881_LDO8,		0x14 },	// PP3V0_USBMUX		3.00V
	{ PMU_IIC_BUS, kD1881_LDO9,		0x01 },	// PP1V5_CAM_AVDD	1.30V
	{ PMU_IIC_BUS, kD1881_LDO10,		0x01 },	// PP2V8_CAM_AVDD	2.55V
	{ PMU_IIC_BUS, kD1881_LDO11,		0x02 },	// 			1.80V
	{ PMU_IIC_BUS, kD1881_LDO12,		0x10 },	// PP1V1		1.00V
	{ PMU_IIC_BUS, kDIALOG_LDO_CONTROL,	0x00 }, // no bypass
};

static const struct pmu_setup_struct pmu_ldo_cold_setup[] =
{
	{ PMU_IIC_BUS, kD1881_ACTIVE1, 		0xBD },	// Everything except VBUCK1, LDO2
	{ PMU_IIC_BUS, kD1881_ACTIVE2, 		0xBB },	// LDO4, LDO5, LDO7, LDO8, LDO9, LDO11 enabled
	{ PMU_IIC_BUS, kD1881_ACTIVE3, 		0x2F },	// LDO12
	{ PMU_IIC_BUS, kD1881_ACTIVE4, 		0xE0 }, // CPU1V8 & CPU1V2 on, WDIG off
	{ PMU_IIC_BUS, kD1881_LCM_CONTROL1,	0x02 }, // PP5V1_GRAPE_VDDH	5.10V
	{ PMU_IIC_BUS, kD1881_LCM_CONTROL2,	0x0E }, // PP5V7_LCD_AVDDH	5.70V
	{ PMU_IIC_BUS, kD1881_LCM_CONTROL3,	0x00 }, // BB_VBUS_DET		5.00V
	{ PMU_IIC_BUS, kD1881_LCM_BST_CONTROL,	0x1A }, // PP6V3_LCM_VBOOST	6.30V
};

static const struct pmu_setup_struct pmu_warm_init[] =
{
	{ PMU_IIC_BUS, kDIALOG_ADC_CONTROL, kDIALOG_ADC_CONTROL_DEFAULTS },
};

static const struct pmu_setup_struct pmu_cold_init[] =
{
	/* Note: the following differences from OTP are intentional:
		- GPIO 5: Enable CLK32k
		- GPIO 8: Enable Codec (Mikey) Wake
		- GPIO 7: Enable WiFi Wake
		- GPIO 10: Enable Wake amd Pull-Up
	*/
	{ PMU_IIC_BUS, kD1881_SYS_GPIO_1,	0xB9 }, // CLK32K_GRAPE		In, R Edge, No Wake, No PU
							// Clock is enabled by AppleARMFunction default
	{ PMU_IIC_BUS, kD1881_SYS_GPIO_2,	0x09 }, // BB_WAKE_AP -- NC
	{ PMU_IIC_BUS, kD1881_SYS_GPIO_3,	0x09 }, // BB_PMU_FET_ON -- NC
	{ PMU_IIC_BUS, kD1881_SYS_GPIO_4,	0xBA }, // TRISTAR_INT		In, R Edge, Wake, No PU
	{ PMU_IIC_BUS, kD1881_SYS_GPIO_5,	0x49 }, // CLK32K_WIFI		Out 0, Push-Pull, VBUCK3
	{ PMU_IIC_BUS, kD1881_SYS_GPIO_6,	0x09 }, // BATTERY_SWI	-- NC
	{ PMU_IIC_BUS, kD1881_SYS_GPIO_7,	0xBB }, // WLAN_HOST_WAKE	In, R Edge, Wake, PD, No PU
	{ PMU_IIC_BUS, kD1881_SYS_GPIO_8,	0xEA }, // CODEC_HOST_WAKE_L	In, Any Edge, Wake, PU, VBUCK3
	{ PMU_IIC_BUS, kD1881_SYS_GPIO_9,	0x09 }, // BT_REG_ON		Out 1, Push-Pull, VBUCK3
	{ PMU_IIC_BUS, kD1881_SYS_GPIO_10,	0xBA }, // BT_HOST_WAKE		In, R Edge, Wake, PU
	{ PMU_IIC_BUS, kD1881_SYS_GPIO_11,	0x09 }, // WIFI_REG_ON		Out 0, Push-Pull, VBUCK3

	{ PMU_IIC_BUS, kD1881_SYS_GPIO_DEB1,	0x12 },
	{ PMU_IIC_BUS, kD1881_SYS_GPIO_DEB2,	0x02 },
	{ PMU_IIC_BUS, kD1881_SYS_GPIO_DEB3,	0x12 },
	{ PMU_IIC_BUS, kD1881_SYS_GPIO_DEB4,	0x00 },
	{ PMU_IIC_BUS, kD1881_SYS_GPIO_DEB5,	0x12 },
	{ PMU_IIC_BUS, kD1881_SYS_GPIO_DEB6,	0x02 },

	{ PMU_IIC_BUS, kDIALOG_SYS_CONTROL2,	0x10 },	// enable HIB_32kHz (for WIFI/BT); DWI disabled

	// <rdar://problem/12192156> turn off buck4 negative current limit
	{ PMU_IIC_BUS, kDIALOG_BANKSEL,		0x01 },	// set bank1
	{ PMU_IIC_BUS, kD1881_TEST_MODE,	0x01 },	// enter test mode
	{ PMU_IIC_BUS, 0xCB,			0x23 },	// turn off negative current limit
	{ PMU_IIC_BUS, kD1881_TEST_MODE,	0x00 },	// exit test mode
	{ PMU_IIC_BUS, kDIALOG_BANKSEL,		0x00 },	// set bank0
};

static const struct pmu_setup_struct pmu_backlight_enable[] =
{
	{ PMU_IIC_BUS, kDIALOG_WLED_ISET,	0xC8 },	// 50% value from OS
	{ PMU_IIC_BUS, kDIALOG_WLED_ISET2,	0x01 },
	{ PMU_IIC_BUS, kDIALOG_WLED_CONTROL,		// enable LED Power
		(kDIALOG_WLED_CONTROL_WLED_ENABLE2 | kDIALOG_WLED_CONTROL_WLED_ENABLE1 | kDIALOG_WLED_CONTROL_WLED_RAMP_EN) }
};

static const struct pmu_setup_struct pmu_backlight_disable[] =
{
	{ PMU_IIC_BUS, kDIALOG_WLED_CONTROL, 0x0 }    // disable LED power
};

static const struct core_rails_struct soc_rails[] =
{
	{ PMU_IIC_BUS, 0, kDIALOG_BUCK0 }
};

static const struct core_rails_struct cpu_rails[] =
{
	{ PMU_IIC_BUS, 0, kDIALOG_BUCK2 }
};

#endif /* POWERCONFIG_PMU_SETUP */

#define NO_BATTERY_VOLTAGE		2700
#define MIN_BOOT_BATTERY_VOLTAGE	3600
#define TARGET_BOOT_BATTERY_VOLTAGE	3700
#define TARGET_BOOT_BATTERY_CAPACITY	50
#define PRECHARGE_BACKLIGHT_LEVEL	824
#define ALWAYS_BOOT_BATTERY_VOLTAGE	(TARGET_BOOT_BATTERY_VOLTAGE + 100)

#if WITH_HW_TRISTAR
extern bool n78_get_e75(void);
#define TARGET_IGNORE_TRISTAR		(!n78_get_e75())
#define TARGET_BRICKID_FULL_SCALE	(5000 * (n78_get_e75() ? 1 : 3))
#else
#define TARGET_BRICKID_FULL_SCALE	(5000 * 3)	// H5P has resistor divider for USB monitor
#endif
#define ACC_PWR_LDO			(6)

#endif
