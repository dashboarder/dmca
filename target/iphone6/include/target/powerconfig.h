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
	{ PMU_IIC_BUS, kD2045_ACTIVE3, 		0xAE },		// Current OTP enables LDO{1,2,3,4,5,7}, disable LDO4.
	{ PMU_IIC_BUS, kD2045_ACTIVE6, 		0x37 },		// enable BUCK3_SW1, BUCK3_SW2, BUCK3_SW3, BUCK4_SW1, BUCK4_SW2
	{ PMU_IIC_BUS, kD2045_ACTIVE3, 		0xBE },		// enable LDO4 along with LDO{1,2,3,5,7}

	{ PMU_IIC_BUS, kD2045_HIBERNATE1, 0x18 },
	{ PMU_IIC_BUS, kD2045_HIBERNATE3, 0x18 },
	{ PMU_IIC_BUS, kD2045_HIBERNATE6, 0x24 },
};

static const struct pmu_setup_struct pmu_warm_init[] =
{
};

// NOTE:  <rdar://problem/13296265> N51 PMU continuously reboots when VBatt is near UVLO
//	  ** GPIO4,GPIO8, and GPIO12 are enabled using the gpio-activate-defaults property in the device tree.

static const struct pmu_setup_struct pmu_cold_init[] =
{
	{ PMU_IIC_BUS, kD2045_GPIO2,		0x7A }, 	// BB_TO_PMU_HOST_WAKE		In, L High, Wake, No PU
	{ PMU_IIC_BUS, kD2045_GPIO3,		0x09 }, 	// PMU_TO_BB_RST_R_L		Out 0, Push-Pull, VBUCK3
	{ PMU_IIC_BUS, kD2045_GPIO4,		0x78 }, 	// TRISTAR_TO_AP_INT		In, L High, Wake**, No PU
	{ PMU_IIC_BUS, kD2045_GPIO6,		0xDA }, 	// AP_BI_BATTERY_SWI		In, F Edge, Out Hib, Wake, PD, No PU
	{ PMU_IIC_BUS, kD2045_GPIO7,		0xBB }, 	// WLAN_TO_PMU_HOST_WAKE	In, R Edge, Wake, PD, No PU
	{ PMU_IIC_BUS, kD2045_GPIO8,		0xE8 }, 	// CODEC_TO_PMU_MIKEY_INT_L	In, Any Edge, Wake**, PU, VBUCK3
	{ PMU_IIC_BUS, kD2045_GPIO9,		0x09 }, 	// PMU_TO_BT_REG_ON_R		Out 0, Push-Pull, VBUCK3
	{ PMU_IIC_BUS, kD2045_GPIO10,		0xAA }, 	// BT_TO_PMU_HOST_WAKE		In, R Edge, Wake, PU, VBUCK3
	{ PMU_IIC_BUS, kD2045_GPIO11,		0x09 }, 	// PMU_TO_WLAN_REG_ON_R		Out 0, Push-Pull, VBUCK3
	{ PMU_IIC_BUS, kD2045_GPIO12,		0xB9 }, 	// NAVAJO_TO_PMU_INT_L		In, R Edge, Wake**, No PU
	{ PMU_IIC_BUS, kD2045_GPIO13,		0x7B }, 	// OSCAR_TO_PMU_HOST_WAKE	In, L High, Wake, PD, No PU
	{ PMU_IIC_BUS, kD2045_GPIO14,		0x49 }, 	// PMU_TO_OSCAR_CLK_32K		Out 32K, Push-Pull, VBUCK3
	{ PMU_IIC_BUS, kD2045_GPIO15,		0x01 }, 	// PMU_TO_BB_VBUS_DET		Out 0, Push-Pull, VCC_MAIN
	{ PMU_IIC_BUS, kD2045_GPIO16,		0x38 }, 	// <rdar://problem/14062728> N5x PMU | Set GPIO16 to 0x38 to better support OTP-AQ on DVT boards
	{ PMU_IIC_BUS, kD2045_OUT_32K, 		0x49 },		// OUT_32K (WLAN_CLK32K)	Out 32K, Push-Pull, VBUCK3

	{ PMU_IIC_BUS, kD2045_GPIO_DEB2,	0x12 },
	{ PMU_IIC_BUS, kD2045_GPIO_DEB5,	0x12 },
	{ PMU_IIC_BUS, kD2045_GPIO_DEB6,	0x12 },

	{ PMU_IIC_BUS, kD2045_BUCK_DWI_CTRL0, 	0x03 },		// enable DWI for BUCK0, BUCK1
	{ PMU_IIC_BUS, kDIALOG_SYS_CONTROL2,	0x11 },		// enable HIB_32kHz (for WIFI/BT) and DWI

	{ PMU_IIC_BUS, kD2045_ACT_TO_HIB_DLY,	0x03 },		// set ACT_TO_HIB_DLY to 3 (6ms)

	{ PMU_IIC_BUS, 0x7000,			0x1D },	// Enable Test Mode
	{ PMU_IIC_BUS, kD2045_ADC_FSM_TRIM2,	0x00 },	// <rdar://problem/14276806> Enable offset correction on Amber IBUS ADC
	{ PMU_IIC_BUS, 0x7000,			0x00 },	// Disable Test Mode
};

static const struct core_rails_struct soc_rails[] =
{
	{ PMU_IIC_BUS, 0, kD2045_BUCK2_VSEL }
};

static const struct core_rails_struct cpu_rails[] =
{
	{ PMU_IIC_BUS, 0, kD2045_BUCK0_VSEL }
};

static const struct core_rails_struct ram_rails[] =
{
	{ PMU_IIC_BUS, 0, kD2045_BUCK5_VSEL }
};

#endif /* POWERCONFIG_PMU_SETUP */

#define NO_BATTERY_VOLTAGE		2700
#define MIN_BOOT_BATTERY_VOLTAGE	3600
#define TARGET_BOOT_BATTERY_VOLTAGE	3700
#define TARGET_BOOT_BATTERY_CAPACITY	50
#define PRECHARGE_BACKLIGHT_LEVEL	824
#define ALWAYS_BOOT_BATTERY_VOLTAGE	(TARGET_BOOT_BATTERY_VOLTAGE + 100)

#define ACC_PWR_LDO			(6)

#endif
