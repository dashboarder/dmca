/*
 * Copyright (C) 2013 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Computer, Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */

#ifndef __TARGET_POWERCONFIG_H
#define __TARGET_POWERCONFIG_H

/* configuration per target */

#define PMU_IIC_BUS     0
//#define CHARGER_IIC_BUS 0

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

static const struct pmu_setup_struct pmu_cold_init[] =
{
	{ PMU_IIC_BUS, kD2238_ALL_RAILS_CONF,		0x00 },		// enable SOC_VDD_ALL_RAILS_ON processing
	{ PMU_IIC_BUS, kD2238_GPIO14_CONF1,		0x93 },		// <rdar://problem/15821471> PMU register 0x041A needs to be changed to 0x93 during boot-up
	{ PMU_IIC_BUS, kD2238_GPIO12_CONF1,		0x00 },		// Display needs to be in reset
	{ PMU_IIC_BUS, kDIALOG_CHARGE_CONTROL_VSET,	0x3f },		// <rdar://problem/16344890> n27: PMU CV voltage needs to be changed from POR
};

static const struct core_rails_struct soc_rails[] =
{
};

static const struct core_rails_struct cpu_rails[] =
{
};

static const struct core_rails_struct ram_rails[] =
{
};

#endif /* POWERCONFIG_PMU_SETUP */

#define NO_BATTERY_VOLTAGE		2700
#define MIN_BOOT_BATTERY_VOLTAGE	3600
#define TARGET_BOOT_BATTERY_VOLTAGE	3700
#define TARGET_BOOT_BATTERY_CAPACITY	0	// rely on SOCF flag only
#define TARGET_PRECHARGE_GG_FLAG_MASK	kHDQRegFlagsMaskSOCF
#define PRECHARGE_BACKLIGHT_LEVEL	824
#define ALWAYS_BOOT_BATTERY_VOLTAGE	(TARGET_BOOT_BATTERY_VOLTAGE + 100)
#define TARGET_FORCE_DEBUG_PRECHARGE	1	// always precharge if "debug" precharge bit set

#define TARGET_CHARGER_HEADROOM		150
#define TARGET_CHARGER_MAX_VRECT	7500
#define TARGET_CHARGER_NEEDS_MAX_VRECT(rem_cap, max_cap) (((rem_cap) * 1000) <= ((max_cap) * 15))	/* 1.5% or lower */

#define ACC_PWR_LDO			(0)	// ACC_PWR not connected

#endif
