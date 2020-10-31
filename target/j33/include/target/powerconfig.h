/*
 * Copyright (C) 2009-2010 Apple Inc. All rights reserved.
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

// XXX derived from values read from regs on K66 Dev 0 board; need to
// reconcile these with OTP doc (which appears to differ) and intended
// settings

// XXX also, need to reconcile OTP settings between K66 Dev 0 and K66 Proto 0

static const struct pmu_setup_struct pmu_ldo_warm_setup[] =
{
	{ PMU_IIC_BUS, kD1815_LDO1,		0x00 },	// PP2V5_BS		2.50V
	{ PMU_IIC_BUS, kD1815_LDO2,		0x1E },	// PP1V8_AP_PVDDP	1.80V
	{ PMU_IIC_BUS, kD1815_LDO3,		0x0A },	// PP3V0_USBMUX		3.00V
	{ PMU_IIC_BUS, kD1815_LDO4,		0x00 },	// PP1V8_AP_DAC		3.00V
	{ PMU_IIC_BUS, kD1815_LDO5,		0x0A },	// PPNNAD		3.00V
	{ PMU_IIC_BUS, kD1815_LDO6,		0x0A },	// PP3V0_MCU		3.00V
	{ PMU_IIC_BUS, kD1815_LDO7,		0x03 },	// PP1V8_AP_VDD		1.80V
	{ PMU_IIC_BUS, kD1815_LDO8,		0x00 },	// PP3V0_CPU		2.00V
	{ PMU_IIC_BUS, kD1815_LDO9,		0x00 },	// PP1V2_HSIC		1.20V
	{ PMU_IIC_BUS, kD1815_LDO10,		0x0A },	// PP3V0_IO		3.00V
	{ PMU_IIC_BUS, kD1815_LDO11,		0x02 },	// PP1V8_AP_VIDEO	1.80V
	{ PMU_IIC_BUS, kD1815_LDO12,		0x10 },	// PP1V0_AP		1.00V

	{ PMU_IIC_BUS, kD1815_LCM_CONTROL1,	0x05 }, // unused
	{ PMU_IIC_BUS, kD1815_LCM_CONTROL2,	0x0E }, // unused
	{ PMU_IIC_BUS, kD1815_LCM_CONTROL3,	0x00 }, // VBOOST_LCM		5.10V
	{ PMU_IIC_BUS, kDIALOG_LDO_CONTROL,	0x00 }, // Leave bypass off
};

static const struct pmu_setup_struct pmu_ldo_cold_setup[] =
{
	{ PMU_IIC_BUS, kD1815_ACTIVE1,		0xFF },	//
	{ PMU_IIC_BUS, kD1815_ACTIVE2,		0x3F },	//
	{ PMU_IIC_BUS, kD1815_ACTIVE3,		0x0C },	// VBOOST_LCM, CPU_SW, WDig
};

static const struct pmu_setup_struct pmu_warm_init[] =
{
	{ PMU_IIC_BUS, kDIALOG_SYS_CONTROL, 	0 },  // Disable SWI by default
	{ PMU_IIC_BUS, kDIALOG_ADC_CONTROL, 	kDIALOG_ADC_CONTROL_DEFAULTS },

	{ PMU_IIC_BUS, kDIALOG_BUCK_CONTROL2,	0xAA },
	
	{ PMU_IIC_BUS, kDIALOG_IRQ_MASK_A,	0xC0 },
	{ PMU_IIC_BUS, kDIALOG_IRQ_MASK_B,	0xFF },
	{ PMU_IIC_BUS, kDIALOG_IRQ_MASK_C,	0xBF },
	{ PMU_IIC_BUS, kDIALOG_IRQ_MASK_D,	0xFF },
	{ PMU_IIC_BUS, kDIALOG_IRQ_MASK_E,	0xAE },
	{ PMU_IIC_BUS, kDIALOG_IRQ_MASK_F,	0xFF },
};

// XXX: review setup for each GPIO and add back decoded description of each config
static const struct pmu_setup_struct pmu_cold_init[] =
{
	{ PMU_IIC_BUS, kD1815_SYS_GPIO_1,	0xB9 }, // ENBL_USB_BUFF_L
	{ PMU_IIC_BUS, kD1815_SYS_GPIO_2,	0xBA }, // LAN_PME_PMU
	{ PMU_IIC_BUS, kD1815_SYS_GPIO_3,	0xBA }, // WLAN_WAKE_HOST
	{ PMU_IIC_BUS, kD1815_SYS_GPIO_4,	0x03 }, // ENET_PWR_EN_L
	{ PMU_IIC_BUS, kD1815_SYS_GPIO_5,	0x01 }, // WLAN_PWR_EN_L
	{ PMU_IIC_BUS, kD1815_SYS_GPIO_6,	0xFA }, // VBAT_DETECT
	{ PMU_IIC_BUS, kD1815_SYS_GPIO_7,	0xBA }, // BT_HOST_WAKE
	{ PMU_IIC_BUS, kD1815_SYS_GPIO_8,	0x18 }, // WLAN_RESET_L
	{ PMU_IIC_BUS, kD1815_SYS_GPIO_TEMP,	0x18 }, // LAN_RESET_L
	{ PMU_IIC_BUS, kD1815_SYS_GPIO_SPARE,	0x4B }, // WLAN_CLK32K_R
	{ PMU_IIC_BUS, kD1815_SYS_GPIO_DEB6,	0x80 }, // enable 32kHz clocks in hibernate
};

static const struct core_rails_struct soc_rails[] =
{
};

#endif /* POWERCONFIG_PMU_SETUP */

#define TARGET_POWER_NO_BATTERY		1
#define TARGET_POWER_USB_MASK		STATUS_FLAG_MAKE(0, kD1815_STATUS_A_VBUS_EXT)
#define ALWAYS_BOOT_BATTERY_VOLTAGE	(3000)
#define PRECHARGE_BACKLIGHT_LEVEL	103

#endif
