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
	{ PMU_IIC_BUS, kD2089_ACTIVE2,          0x03 }, // WLEDA/B BST
};

static const struct pmu_setup_struct pmu_warm_init[] =
{
};

static const struct pmu_setup_struct pmu_cold_init[] =
{
	/* GPIO configuration from I/O spreadsheet.
	   <rdar://problem/13029600> J34: IOSpreadsheet
	   Request spreadsheet update before modifying
	*/
	{ PMU_IIC_BUS, kD2089_GPIO2,		0xA3 }, // PMU_MOCA2PMU_HOST_WAKE		In, R Edge, Wake, PD, VCC_MAIN
	{ PMU_IIC_BUS, kD2089_GPIO3,		0x09 },	// PMU_GPIO_BT_REG_ON			Out 0, Push-Pull, VBUCK3
	{ PMU_IIC_BUS, kD2089_GPIO4,		0x09 },	// PMU_GPIO_WLAN_REG_ON			Out 0, Push-Pull, VBUCK3
	{ PMU_IIC_BUS, kD2089_GPIO6,		0x18 }, // PMU_HVR_RESET_L			Out 0, Open-Drain, No PU
	{ PMU_IIC_BUS, kD2089_GPIO7,		0xBA },	// PMU_GPIO_BT2PMU_HOST_WAKE		In, R Edge, Wake, PD
	{ PMU_IIC_BUS, kD2089_GPIO8,		0xBA },	// PMU_GPIO_WLAN2PMU_HOST_WAKE		In, R Edge, Wake, PD
	{ PMU_IIC_BUS, kD2089_GPIO9,		0xA3 }, // LAN_PME_PMU				In, R Edge, Wake, PD, VCC_MAIN
	{ PMU_IIC_BUS, kD2089_GPIO10,		0x18 }, // MOCA_PWRON				Out 0, Open-Drain, No PU
	{ PMU_IIC_BUS, kD2089_GPIO11,		0x18 }, // PMU_LAN_RESET_L			Out 0, Open-Drain, No PU
	{ PMU_IIC_BUS, kD2089_GPIO12,		0x7B },	// GPIO_TS2SOC2PMU_INT			In, Hi level, Wake, PD
	{ PMU_IIC_BUS, kD2089_GPIO13,		0x18 }, // PMU_MOCA_RESET_L			Out 0, Open-Drain, No PU
	{ PMU_IIC_BUS, kD2089_GPIO15,		0xA3 }, // LAN2_PME_PMU				In, R Edge, Wake, PD, VCC_MAIN
	{ PMU_IIC_BUS, kD2089_GPIO16,		0x18 }, // PMU_LAN2_RESET_L			Out 0, Open-Drain, No PU
	{ PMU_IIC_BUS, kD2089_GPIO17,		0xA1 }, // VBAT_DETECT				In, R Edge, PD, VCC_MAIN

	{ PMU_IIC_BUS, kD2089_BUCK_DWI_CTRL0, 	0x03 },		// enable DWI for BUCK0, BUCK1
	{ PMU_IIC_BUS, kD2089_ACT_TO_HIB_DLY,	0x03 },		// set ACT_TO_HIB_DLY to 3 (6ms)
	{ PMU_IIC_BUS, kDIALOG_SYS_CONTROL2,	0x11 },		// enable HIB_32kHz (for WIFI/BT) and DWI

	{ PMU_IIC_BUS, kD2089_RESET_IN3_CONF,	0x11 },		// Disable RESET_IN3 for SOCHOT1 <rdar://problem/12407942>
};

static const struct pmu_setup_struct pmu_backlight_enable[] =
{
};

static const struct pmu_setup_struct pmu_backlight_disable[] =
{
};

static const struct core_rails_struct soc_rails[] =
{
	{ PMU_IIC_BUS, 0, kD2089_BUCK2_VSEL }
};

static const struct core_rails_struct cpu_rails[] =
{
	{ PMU_IIC_BUS, 0, kD2089_BUCK0_VSEL }
};

static const struct core_rails_struct ram_rails[] =
{
	{ PMU_IIC_BUS, 0, kD2089_BUCK5_VSEL }
};

#endif /* POWERCONFIG_PMU_SETUP */

#define TARGET_POWER_NO_BATTERY		1
#define ALWAYS_BOOT_BATTERY_VOLTAGE	(3000)
#define PRECHARGE_BACKLIGHT_LEVEL	103
#define TARGET_MAX_USB_INPUT_CURRENT	2400

#define ACC_PWR_LDO			(6)

#endif
