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
	{ PMU_IIC_BUS, kD2045_LDO9_VSEL, 	0x5A},		// LDO9 (PP2V8_CAM) to 2.85V <rdar://problem/12660594>

	{ PMU_IIC_BUS, kD2045_ACTIVE3, 		0xAE },		// Current OTP enables LDO{1,2,3,4,5,7}, disable LDO4.
	{ PMU_IIC_BUS, kD2045_ACTIVE6, 		0x37 },		// enable BUCK3_SW1, BUCK3_SW2, BUCK3_SW3, BUCK4_SW1, BUCK4_SW2
	{ PMU_IIC_BUS, kD2045_ACTIVE3, 		0xBE },		// enable LDO4 along with LDO{1,2,3,5,7}

	{ PMU_IIC_BUS, kD2045_HIBERNATE1, 0x18 },
	{ PMU_IIC_BUS, kD2045_HIBERNATE3, 0x18 },
	{ PMU_IIC_BUS, kD2045_HIBERNATE6, 0x24 },
};

static const struct pmu_setup_struct pmu_ldo_cold_setup_a0[] =
{
	{ PMU_IIC_BUS, kD2045_LDO9_VSEL, 	0x42},		// LDO9 (PP2V8_CAM) to 2.85V <rdar://problem/12660594>

	{ PMU_IIC_BUS, kD2045_ACTIVE3, 		0xAE },		// Current OTP enables LDO{1,2,3,4,5,7}, disable LDO4.
	{ PMU_IIC_BUS, kD2045_ACTIVE6, 		0x37 },		// enable BUCK3_SW1, BUCK3_SW2, BUCK3_SW3, BUCK4_SW1, BUCK4_SW2
	{ PMU_IIC_BUS, kD2045_ACTIVE3, 		0xBE },		// enable LDO4 along with LDO{1,2,3,5,7}

	// <rdar://problem/12264399> N51: Workaround needed for hibernate on PROTO2 Amber OTP
	{ PMU_IIC_BUS, 0x7000,			0x1d },		// Enable test mode
	{ PMU_IIC_BUS, kD2045_BUTTON4_CONF,	0x94 },		// Set BUTTON4 config to be pulled up to VBUCK3 instead of VBUCK3_SW
	{ PMU_IIC_BUS, 0x7000,			0x00 },		// Disable test mode

	{ PMU_IIC_BUS, kD2045_HIBERNATE1, 0x18 },
	{ PMU_IIC_BUS, kD2045_HIBERNATE3, 0x18 },
	{ PMU_IIC_BUS, kD2045_HIBERNATE6, 0x24 },
};

static const struct pmu_setup_struct pmu_warm_init[] =
{
};

static const struct pmu_setup_struct pmu_cold_init[] =
{
	{ PMU_IIC_BUS, kD2045_GPIO_DEB2,	0x12 },
	{ PMU_IIC_BUS, kD2045_GPIO_DEB5,	0x12 },
	{ PMU_IIC_BUS, kD2045_GPIO_DEB6,	0x12 },

	{ PMU_IIC_BUS, kD2045_BUCK_DWI_CTRL0, 	0x03 },		// enable DWI for BUCK0, BUCK1
	{ PMU_IIC_BUS, kDIALOG_SYS_CONTROL2,	0x11 },		// enable HIB_32kHz (for WIFI/BT) and DWI

	{ PMU_IIC_BUS, kD2045_ACT_TO_HIB_DLY,	0x03 },		// set ACT_TO_HIB_DLY to 3 (6ms)
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
