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
	{ PMU_IIC_BUS, kD2207_HIBERNATE3,   0x90 },		// enable LDO7+LDO4 in hibernate
};

static const struct pmu_setup_struct pmu_warm_init[] =
{

};

static const struct pmu_setup_struct pmu_cold_init[] =
{
	/* <rdar://problem/14847504> J82 IO Spreadsheet */
#if TARGET_HAS_BASEBAND
	{ PMU_IIC_BUS, kD2207_GPIO5,		0x09}, // PMU_GPIO_PMU2BBPMU_RST_L		Out 0, Push-Pull, VBUCK3
#else 
	{ PMU_IIC_BUS, kD2207_GPIO5,		0xB9}, // PMU_GPIO_PMU2BBPMU_RST_L		In, R Edge, PD
#endif
	{ PMU_IIC_BUS, kD2207_GPIO6,		0xDA}, // UART5_BATT_TRXD			In, F Edge, Wake, PD
	{ PMU_IIC_BUS, kD2207_GPIO7,		0xBA}, // PMU_GPIO_BT_HOST_WAKE			In, R Edge, Wake, PD
	{ PMU_IIC_BUS, kD2207_GPIO8,		0xBA}, // PMU_GPIO_WLAN_HOST_WAKE		In, R Edge, Wake, PD
#if TARGET_HAS_BASEBAND
	{ PMU_IIC_BUS, kD2207_GPIO9,		0xBA}, // PMU_GPIO_BB2PMU_HOST_WAKE		In, R Edge, Wake, PU, VBUCK3
#else
	{ PMU_IIC_BUS, kD2207_GPIO9,		0xB9}, // PMU_GPIO_BB2PMU_HOST_WAKE		In, R Edge, PD
#endif
	{ PMU_IIC_BUS, kD2207_GPIO10,		0xCA}, // PMU_GPIO_CODEC_HS_INT_L		In, F Edge, Wake, PU, VBUCK3
	{ PMU_IIC_BUS, kD2207_GPIO11,		0xFA}, // PMU_GPIO_MB_HALL1_IRQ			In, Any Edge, Wake, No PU
	{ PMU_IIC_BUS, kD2207_GPIO12,		0x7B}, // GPIO_TS2SOC2PMU_INT			In, Hi level, Wake, PD
	{ PMU_IIC_BUS, kD2207_GPIO14,		0xB8}, // PMU_GPIO_WLAN2PMU_PCIE_WAKE_L		In, R Edge, PU
	{ PMU_IIC_BUS, kD2207_GPIO16,		0x7B}, // PMU_GPIO_OSCAR2PMU_HOST_WAKE		In, Hi level, Wake, PD

	{ PMU_IIC_BUS, kD2207_BUCK_DWI_CTRL0,	0x07 },			// enable DWI for BUCK0, BUCK1, BUCK2
};

static const struct pmu_setup_struct pmu_backlight_enable[] =
{
	{ PMU_IIC_BUS, kDIALOG_WLED_ISET,	0xD4 }, // 50% value from OS
	{ PMU_IIC_BUS, kDIALOG_WLED_ISET2,	0x00 },
	{ PMU_IIC_BUS, kDIALOG_WLED_CONTROL,		// enable all LED strings
		(kDIALOG_WLED_CONTROL_WLED_ENABLE1 |
		 kDIALOG_WLED_CONTROL_WLED_ENABLE2 |
		 kDIALOG_WLED_CONTROL_WLED_ENABLE3 |
		 kDIALOG_WLED_CONTROL_WLED_ENABLE4 |
		 kDIALOG_WLED_CONTROL_WLED_ENABLE5 |
		 kDIALOG_WLED_CONTROL_WLED_ENABLE6) },
	{ PMU_IIC_BUS, kDIALOG_WLED_OPTIONS,		// enable LED Power
		(kDIALOG_WLED_CONTROL2_WLED_RAMP_EN |
		 kDIALOG_WLED_CONTROL2_WLED_DITH_EN) }
};

static const struct pmu_setup_struct pmu_backlight_disable[] =
{
	{ PMU_IIC_BUS, kDIALOG_WLED_CONTROL, 0x0 }	// disable LED power
};

static const struct core_rails_struct soc_rails[] =
{
	{ PMU_IIC_BUS, 0, kD2207_BUCK2_VSEL }
};

static const struct core_rails_struct cpu_rails[] =
{
	{ PMU_IIC_BUS, 0, kD2207_BUCK0_VSEL }
};

static const struct core_rails_struct ram_rails[] =
{
	{ PMU_IIC_BUS, 0, kD2207_BUCK5_VSEL }
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
