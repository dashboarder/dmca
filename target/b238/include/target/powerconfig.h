/*
 * Copyright (C) 2015 Apple Inc. All rights reserved.
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

static const struct pmu_setup_struct pmu_ldo_warm_setup[] =
{
};

static const struct pmu_setup_struct pmu_ldo_cold_setup[] =
{
};

static const struct pmu_setup_struct pmu_warm_init[] =
{
};

// XXX: review setup for each GPIO and add back decoded description of each config
// rdar://problem/19654165 B238: IO Spreadsheet Tracker

static const struct pmu_setup_struct pmu_cold_init[] =
{
	{ PMU_IIC_BUS, kD2186_BUCK_DWI_CTRL0,	0x07 },			// <rdar://problem/20401454> B238: Multiple panics when entering single user mode - need to enable DWI comm between PMU and bucks
	{ PMU_IIC_BUS, kD2186_GPIO9_CONF1,	    0x93 },		    // <rdar://problem/21039044> B238: GPIO17 wake on rising edge	
};


static const struct core_rails_struct cpu_rails[] =
{
	{ PMU_IIC_BUS, 0, kD2186_BUCK0_VSEL }
};

static const struct core_rails_struct ram_rails[] =
{
	{ PMU_IIC_BUS, 0, kD2186_BUCK5_VSEL }
};

static const struct core_rails_struct soc_rails[] =
{
    { PMU_IIC_BUS, 0, kD2186_BUCK2_VSEL }
};

#endif /* POWERCONFIG_PMU_SETUP */

#define TARGET_POWER_NO_BATTERY		1
//#define TARGET_POWER_USB_MASK		STATUS_FLAG_MAKE(0, kD1815_STATUS_A_VBUS_EXT)
#define ALWAYS_BOOT_BATTERY_VOLTAGE	(3000)
#define PRECHARGE_BACKLIGHT_LEVEL	103

#define ACC_PWR_LDO			(6)

#endif
