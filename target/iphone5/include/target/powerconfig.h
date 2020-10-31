/*
 * Copyright (C) 2010-2012 Apple Inc. All rights reserved.
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

/* Matched to D1972A2-OTP-AE-v2 */

static const struct pmu_setup_struct pmu_ldo_warm_setup[] =
{
	{ PMU_IIC_BUS, kD1972_LDO1,		0x0A },	// PP3V0_USB		3.00V
	{ PMU_IIC_BUS, kD1972_LDO2,		0x2A },	// PP1V7_VA_VCP		1.70V
	{ PMU_IIC_BUS, kD1972_LDO3,		0x0A },	// PP3V0_VIDEO		3.00V
	{ PMU_IIC_BUS, kD1972_LDO4,		0x0F },	// PP2V8_CAM0_AF	2.55V
	{ PMU_IIC_BUS, kD1972_LDO5,		0x0A },	// PP3V0_NAND		3.00V
	{ PMU_IIC_BUS, kD1972_LDO6,		0x10 },	// PP3V3_ACC		3.30V
	{ PMU_IIC_BUS, kD1972_LDO7,		0x0F },	// P3V0_IMU		3.00V
	{ PMU_IIC_BUS, kD1972_LDO8,		0x14 },	// PP3V0_USBMUX		3.00V

	// <rdar://problem/12061759>
	{ PMU_IIC_BUS, kD1972_TEST_MODE,	0x1D },
	{ PMU_IIC_BUS, 0xF5,			0x01 },
	{ PMU_IIC_BUS, kD1972_LDO9,		0x13 },	// PP3V0_IO		3.10V
	{ PMU_IIC_BUS, 0xF5,			0x00 },
	{ PMU_IIC_BUS, kD1972_TEST_MODE,	0x00 },
	
	{ PMU_IIC_BUS, kD1972_LDO10,		0x0A },	// PP3V0_OPTICAL	3.00V
	{ PMU_IIC_BUS, kD1972_LDO11,		0x17 },	// PP2V8_CAM_AVDD	2.85V
	{ PMU_IIC_BUS, kD1972_LDO12,		0x10 },	// PP1V0		1.00V
	{ PMU_IIC_BUS, kD1972_LDO16,		0xA0 },	// PP1V0_SRAM_PMU	1.00V
	{ PMU_IIC_BUS, kDIALOG_LDO_CONTROL,	0x00 }, // no bypass
	{ PMU_IIC_BUS, kD1972_BUCK_CTRL5,	0xD3 }, // Enable x_DWI_EN and CPUx_EN_CTRL
};

static const struct pmu_setup_struct pmu_ldo_cold_setup[] =
{
	{ PMU_IIC_BUS, kD1972_ACTIVE1, 		0xFF },	// Everything on
	{ PMU_IIC_BUS, kD1972_ACTIVE2, 		0xF4 },	// Everything but LDO3, LDO4, LDO6 on
	{ PMU_IIC_BUS, kD1972_ACTIVE3, 		0xEB },	// WLED_BST, LCM_BST, LDO16, LDO_LCM2, LDO12, LDO11 on
	{ PMU_IIC_BUS, kD1972_ACTIVE4, 		0xAD }, // CPU1V8, CPU1V2, CPUA_SW on
	{ PMU_IIC_BUS, kD1972_HIBERNATE1,	0x70 }, // BUCK3, BUCK4, LDO1 on
	{ PMU_IIC_BUS, kD1972_HIBERNATE2,	0x00 }, // Everything off
	{ PMU_IIC_BUS, kD1972_LCM_CONTROL1,	0x02 }, // PP5V1_GRAPE_VDDH_PMU	5.10V
	{ PMU_IIC_BUS, kD1972_LCM_CONTROL2,	0x0E }, // PP5V7_LCD_AVDDH_PMU	5.70V
	{ PMU_IIC_BUS, kD1972_LCM_CONTROL3,	0x00 }, // BB_VBUS_DET		5.00V
	{ PMU_IIC_BUS, kD1972_LCM_BST_CONTROL,	0x14 }, // LCM_VBOOST		6.00V
};

static const struct pmu_setup_struct pmu_warm_init[] =
{
	{ PMU_IIC_BUS, kDIALOG_ADC_CONTROL, kDIALOG_ADC_CONTROL_DEFAULTS },
};

static const struct pmu_setup_struct pmu_cold_init[] =
{
	/* Note: the following differences from OTP are intentional:
	   - GPIO2,6,7,10 enable wake
	   - GPIO5 enables CLK32
	   - GPIO8 enables wake up and state change
	   - GPIO10 pullup
	*/
	{ PMU_IIC_BUS, kD1972_SYS_GPIO_1,	0xB9 }, // CLK32K_GRAPE_RESET_L	In, R Edge, No Wake, No PU
	{ PMU_IIC_BUS, kD1972_SYS_GPIO_2,	0xBA }, // BB_WAKE_AP		In, R Edge, Wake, No PU
	{ PMU_IIC_BUS, kD1972_SYS_GPIO_3,	0x09 }, // BB_PMU_ON_R_L	Out 0, Push-Pull, VBUCK3
	{ PMU_IIC_BUS, kD1972_SYS_GPIO_4,	0x7A }, // TRISTAR_INT		In, L High, Wake, No PU
	{ PMU_IIC_BUS, kD1972_SYS_GPIO_5,	0x49 }, // CLK32K_WIFI		Out 0, Push-Pull, VBUCK3
	{ PMU_IIC_BUS, kD1972_SYS_GPIO_6,	0xDA }, // BATTERY_SWI		In, F Edge, Out Hib, Wake, PD, No PU
	{ PMU_IIC_BUS, kD1972_SYS_GPIO_7,	0xBB }, // HOST_WAKE_WLAN	In, R Edge, Wake, PD, No PU
	{ PMU_IIC_BUS, kD1972_SYS_GPIO_8,	0xEA }, // MIKEY_INT_L		In, Any Edge, Wake, PU, VBUCK3
	{ PMU_IIC_BUS, kD1972_SYS_GPIO_9,	0x09 }, // BT_REG_ON_R		Out 0, Push-Pull, VBUCK3
	{ PMU_IIC_BUS, kD1972_SYS_GPIO_10,	0xAA }, // HOST_WAKE_BT		In, R Edge, Wake, PU, VBUCK3
	{ PMU_IIC_BUS, kD1972_SYS_GPIO_11,	0x09 }, // WIFI_REG_ON_R	Out 0, Push-Pull, VBUCK3
	{ PMU_IIC_BUS, kD1972_SYS_GPIO_12,	0xB9 }, // PMU_DBG_GPIO12	In, R Edge, No Wake, No PU

	{ PMU_IIC_BUS, kD1972_SYS_GPIO_DEB1,	0x12 },
	{ PMU_IIC_BUS, kD1972_SYS_GPIO_DEB2,	0x12 },
	{ PMU_IIC_BUS, kD1972_SYS_GPIO_DEB3,	0x02 }, // no debounce on GPIO6 [OTP: 0x12]
	{ PMU_IIC_BUS, kD1972_SYS_GPIO_DEB4,	0x02 },	// no debounce on GPIO8 [OTP: 0x12]
	{ PMU_IIC_BUS, kD1972_SYS_GPIO_DEB5,	0x12 },
	{ PMU_IIC_BUS, kD1972_SYS_GPIO_DEB6,	0x12 },

	{ PMU_IIC_BUS, kDIALOG_SYS_CONTROL2,	0x11 },	// enable HIB_32kHz (for WIFI/BT) & DWI
};

static const struct pmu_setup_struct pmu_backlight_enable[] =
{
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

static const struct core_rails_struct ram_rails[] =
{
	{ PMU_IIC_BUS, 0, kD1972_LDO16 }
};

#endif /* POWERCONFIG_PMU_SETUP */

#define NO_BATTERY_VOLTAGE		2700
#define MIN_BOOT_BATTERY_VOLTAGE	3600
#define TARGET_BOOT_BATTERY_VOLTAGE	3700
#define TARGET_BOOT_BATTERY_CAPACITY	50
#define PRECHARGE_BACKLIGHT_LEVEL	824

#define TARGET_DISPLAY_VOLTAGE_BASE	6000
#define TARGET_DISPLAY_VOLTAGE_SCALE	50
#define TARGET_DISPLAY_BOOST_LDO	kD1972_LCM_CONTROL2
#define TARGET_DISPLAY_BOOST_OFFSET	300

#define ACC_PWR_LDO			(6)

#endif
