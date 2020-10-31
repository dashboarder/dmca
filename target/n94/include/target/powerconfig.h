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

/* configuration per target */

#define PMU_IIC_BUS		0

/* this configuration is very PMU specific and must be included exactly once in power.c */
#ifdef POWERCONFIG_PMU_SETUP

/* Matched to D1881A3-OTP-AO-v4 */

static const struct pmu_setup_struct pmu_ldo_warm_setup[] =
{
	{ PMU_IIC_BUS, kD1881_LDO1,		0x0D },	// PP3V0_USB		3.15V <rdar://problem/9200307>
	{ PMU_IIC_BUS, kD1881_LDO2,		0x0A },	// PP1V7_VA_VCP		1.70V
	{ PMU_IIC_BUS, kD1881_LDO3,		0x0A },	// PP3V0_IMU		3.00V
	{ PMU_IIC_BUS, kD1881_LDO4,		0x18 },	// PP3V0_OPTICAL	3.00V
	{ PMU_IIC_BUS, kD1881_LDO5,		0x0A },	// PP3V0_NAND		3.00V
	{ PMU_IIC_BUS, kD1881_LDO6,		0x10 },	// PP3V3_ACC		3.30V
	{ PMU_IIC_BUS, kD1881_LDO7,		0x0F },	// PP3V3_VIDEO		3.00V
	{ PMU_IIC_BUS, kD1881_LDO8,		0x14 },	// PP3V0_USBMUX		3.00V
	{ PMU_IIC_BUS, kD1881_LDO9,		0x12 },	// PP3V0_IO		3.00V
	{ PMU_IIC_BUS, kD1881_LDO10,		0x06 },	// PP2V8_CAM_AVDD	2.80V
	{ PMU_IIC_BUS, kD1881_LDO11,		0x16 },	// PP2V8_CAM_AF		2.80V
	{ PMU_IIC_BUS, kD1881_LDO12,		0x14 },	// PP1V1		1.10V
	{ PMU_IIC_BUS, kDIALOG_LDO_CONTROL,	0x00 }, // no bypass
};

static const struct pmu_setup_struct pmu_ldo_cold_setup[] =
{
	{ PMU_IIC_BUS, kD1881_ACTIVE1, 		0xFD },	// Everything except VBUCK1
	{ PMU_IIC_BUS, kD1881_ACTIVE2, 		0xFB },	// Everything except LDO6
	{ PMU_IIC_BUS, kD1881_ACTIVE3, 		0x2B },	// WLED_BST, LCM_BST, LDO_LCM2, LDO12
	{ PMU_IIC_BUS, kD1881_ACTIVE4, 		0xA0 }, // CPU1V8 & CPU1V2 on, WDIG off
	{ PMU_IIC_BUS, kD1881_LCM_CONTROL1,	0x02 }, // PP5V1_GRAPE_VDDH	5.10V
	{ PMU_IIC_BUS, kD1881_LCM_CONTROL2,	0x0E }, // PP5V7_LCD_AVDDH	5.70V
	{ PMU_IIC_BUS, kD1881_LCM_CONTROL3,	0x00 }, // BB_VBUS_DET		5.00V
	{ PMU_IIC_BUS, kD1881_LCM_BST_CONTROL,	0x14 }, // PP6V0_LCM_VBOOST	6.00V
};

static const struct pmu_setup_struct pmu_warm_init[] =
{
	{ PMU_IIC_BUS, kDIALOG_ADC_CONTROL, kDIALOG_ADC_CONTROL_DEFAULTS },
};

static const struct pmu_setup_struct pmu_cold_init[] =
{
	{ PMU_IIC_BUS, kDIALOG_BUCK_CONTROL1,   0x0a },	// buck0 sync mode, 1.25A limit per phase
	{ PMU_IIC_BUS, kDIALOG_BUCK_CONTROL3,   0x0e },	// buck2 sync mode, 1.55A limit per phase

	/* Note: the following differences from OTP are intentional:
	   - GPIO2,6,7,8,10 enable wake
	   - GPIO5 enables CLK32
	   - GPIO8 interrupt type
	   - GPIO9 asserted
	   - GPIO6,8 no debounce
	*/
	{ PMU_IIC_BUS, kD1881_SYS_GPIO_1,	0x11 }, // CLK32K_GRAPE		Out 0, Push-Pull, CPU_1V8
							// Clock is enabled by AppleARMFunction default
	{ PMU_IIC_BUS, kD1881_SYS_GPIO_2,	0xBA }, // BB_WAKE_AP		In, R Edge, Wake, No PU or PD
	{ PMU_IIC_BUS, kD1881_SYS_GPIO_3,	0x09 }, // BB_PMU_FET_ON	Out 0, Push-Pull, VBUCK3
	{ PMU_IIC_BUS, kD1881_SYS_GPIO_4,	0x09 }, // DOCK_BB_USB_SEL	Out 0, Push-Pull, VBUCK3
	{ PMU_IIC_BUS, kD1881_SYS_GPIO_5,	0x49 }, // CLK32K_WIFI		Out 0, Push-Pull, VBUCK3
	{ PMU_IIC_BUS, kD1881_SYS_GPIO_6,	0xDA }, // BATTERY_SWI		In, F Edge, Out Hib, Wake, PD, No PU
	{ PMU_IIC_BUS, kD1881_SYS_GPIO_7,	0xBB }, // WLAN_WAKE_AP		In, R Edge, Wake, PD, No PU
	{ PMU_IIC_BUS, kD1881_SYS_GPIO_8,	0xEA }, // MIKEY_INT_L		In, Any Edge, Wake, PU, VBUCK3
	{ PMU_IIC_BUS, kD1881_SYS_GPIO_9,	0x0B }, // BT_REG_ON		Out 1, Push-Pull, VBUCK3
	{ PMU_IIC_BUS, kD1881_SYS_GPIO_10,	0xBB }, // BT_WAKE_AP		In, R Edge, Wake, PD, No PU
	{ PMU_IIC_BUS, kD1881_SYS_GPIO_11,	0x09 }, // WIFI_REG_ON		Out 0, Push-Pull, VBUCK3

	{ PMU_IIC_BUS, kD1881_SYS_GPIO_DEB1,	0x12 },
	{ PMU_IIC_BUS, kD1881_SYS_GPIO_DEB2,	0x12 },
	{ PMU_IIC_BUS, kD1881_SYS_GPIO_DEB3,	0x02 },
	{ PMU_IIC_BUS, kD1881_SYS_GPIO_DEB4,	0x02 },
	{ PMU_IIC_BUS, kD1881_SYS_GPIO_DEB5,	0x12 },
	{ PMU_IIC_BUS, kD1881_SYS_GPIO_DEB6,	0x02 },

	{ PMU_IIC_BUS, kDIALOG_SYS_CONTROL2,	0x10 },	// enable HIB_32kHz (for WIFI/BT); DWI disabled
};

static const struct pmu_setup_struct pmu_backlight_enable[] =
{
	{ PMU_IIC_BUS, kDIALOG_WLED_ISET,	0xC8 },	// 50% value from OS
	{ PMU_IIC_BUS, kDIALOG_WLED_ISET2,	0x01 },
	{ PMU_IIC_BUS, kDIALOG_WLED_CONTROL,		// enable LED Power
		(kDIALOG_WLED_CONTROL_WLED_ENABLE1 | kDIALOG_WLED_CONTROL_WLED_RAMP_EN |
		 kDIALOG_WLED_CONTROL_WLED_DITH_EN) }
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

#define NO_BATTERY_VOLTAGE		2700
#define MIN_BOOT_BATTERY_VOLTAGE	3600
#define TARGET_BOOT_BATTERY_VOLTAGE	3700
#define TARGET_BOOT_BATTERY_CAPACITY	50
#define PRECHARGE_BACKLIGHT_LEVEL	824

#define TARGET_DISPLAY_VOLTAGE_BASE	6000
#define TARGET_DISPLAY_VOLTAGE_SCALE	50
#define TARGET_DISPLAY_BOOST_LDO	kD1881_LCM_CONTROL2
#define TARGET_DISPLAY_BOOST_OFFSET	300

#endif
