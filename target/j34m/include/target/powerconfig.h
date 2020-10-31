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
	
	{ PMU_IIC_BUS, kD2089_BUCK4_VSEL,       0xD0 }, // rdar:// 15918274 PP1V2_S2R
	{ PMU_IIC_BUS, kD2089_BUCK4_VSEL_ALT,   0xD0 }, // 
};

static const struct pmu_setup_struct pmu_warm_init[] =
{
};

static const struct pmu_setup_struct pmu_cold_init[] =
{
	/* GPIO configuration from I/O spreadsheet.
	   <rdar://problem/15995289> J34m: IO spreadsheet
	   Request spreadsheet update before modifying
	   update GPIO1,15 and remove dup of OTP per <rdar://16415219> J34m: set iBoot PMU GPIOs to IOSpreadsheet
	   update GPIO1 per <rdar://problem/16502988> J34m: PMU register for GPIO1 not set properly
	*/
	{ PMU_IIC_BUS, kD2089_GPIO1,		0x01 }, // PHY_SEL				Out 0, Push-Pull, no PU
	{ PMU_IIC_BUS, kD2089_GPIO3,		0x09 },	// PMU_GPIO_BT_REG_ON			Out 0, Push-Pull, VBUCK3
	{ PMU_IIC_BUS, kD2089_GPIO4,		0x09 },	// PMU_GPIO_WLAN_REG_ON			Out 0, Push-Pull, VBUCK3
	{ PMU_IIC_BUS, kD2089_GPIO7,		0xBA },	// PMU_GPIO_BT2PMU_HOST_WAKE		In, R Edge, Wake, PD
	{ PMU_IIC_BUS, kD2089_GPIO8,		0xBA },	// PMU_GPIO_WLAN2PMU_HOST_WAKE		In, R Edge, Wake, PD
	{ PMU_IIC_BUS, kD2089_GPIO12,		0x7B },	// GPIO_TS2SOC2PMU_INT			In, Hi level, Wake, PD
	{ PMU_IIC_BUS, kD2089_GPIO15,		0x88 }, // MOCA_LINK_L				In, Lo level, PU, VBUCK3
	{ PMU_IIC_BUS, kD2089_GPIO16,		0x18 }, // PMU_LAN2_RESET_L			Out 0, Open-Drain, No PU
	{ PMU_IIC_BUS, kD2089_GPIO17,		0xA1 }, // VBAT_DETECT				In, R Edge, PD, VCC_MAIN

	{ PMU_IIC_BUS, kD2089_GPIO_DEB3,	0x20 },		// GPIO5_DEBOUNCE=0, rdar://16735592 Debounce of ENET_DET needs to be disabled
	{ PMU_IIC_BUS, kD2089_BUCK_DWI_CTRL0, 	0x03 },		// enable DWI for BUCK0, BUCK1
	{ PMU_IIC_BUS, kD2089_ACT_TO_HIB_DLY,	0x03 },		// set ACT_TO_HIB_DLY to 3 (6ms)
	{ PMU_IIC_BUS, kDIALOG_SYS_CONTROL2,	0x11 },		// enable HIB_32kHz (for WIFI/BT) and DWI
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
