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
	{ PMU_IIC_BUS, kD2207_HIBERNATE6,   0x22 },	// DVDD must be on <rdar://problem/20080758> 
};

static const struct pmu_setup_struct pmu_warm_init[] =
{

};

static const struct pmu_setup_struct pmu_cold_init[] =
{
	/* <rdar://problem/14957338> J96-J97 IO spreadsheet tracker */
#if TARGET_HAS_BASEBAND
	{ PMU_IIC_BUS, kD2207_GPIO5,		0x09}, // PMU_GPIO_PMU2BBPMU_RST_L		Out 0, Push-Pull, VBUCK3
#else 
	{ PMU_IIC_BUS, kD2207_GPIO5,		0xB9}, // PMU_GPIO_PMU2BBPMU_RST_L		In, R Edge, PD
#endif
	{ PMU_IIC_BUS, kD2207_GPIO6,		0xDA}, // UART5_BATT_TRXD			In, F Edge, Wake, PD
	{ PMU_IIC_BUS, kD2207_GPIO7,		0xBA}, // PMU_GPIO_BT_HOST_WAKE			In, R Edge, Wake, PD
	{ PMU_IIC_BUS, kD2207_GPIO8,		0x7B}, // PMU_GPIO_WLAN_HOST_WAKE		In, Hi level, Wake, PD
#if TARGET_HAS_BASEBAND
	{ PMU_IIC_BUS, kD2207_GPIO9,		0x8A}, // PMU_GPIO_BB2PMU_HOST_WAKE		In, Active low, Wake, PU, VBUCK3
#else
	{ PMU_IIC_BUS, kD2207_GPIO9,		0xB9}, // PMU_GPIO_BB2PMU_HOST_WAKE		In, R Edge, PD
#endif
	{ PMU_IIC_BUS, kD2207_GPIO10,		0xEA}, // PMU_GPIO_CODEC_HS_INT_L		In, Any Edge, Wake, PU, VBUCK3
	{ PMU_IIC_BUS, kD2207_GPIO11,		0xFA}, // PMU_GPIO_MB_HALL0_IRQ			In, Any Edge, Wake, No PU
	{ PMU_IIC_BUS, kD2207_GPIO12,		0x7B}, // GPIO_TS2SOC2PMU_INT			In, Hi level, Wake, PD
	{ PMU_IIC_BUS, kD2207_GPIO13,		0xF8}, // PMU_GPIO_MB_HALL1_IRQ				In, Any Edge, NoWake, No PU
	{ PMU_IIC_BUS, kD2207_GPIO14,		0xB8}, // PMU_GPIO_WLAN2PMU_PCIE_WAKE_L		In, R Edge, PU
	{ PMU_IIC_BUS, kD2207_GPIO16,		0x7B}, // PMU_GPIO_OSCAR2PMU_HOST_WAKE		In, Hi level, Wake, PD
	{ PMU_IIC_BUS, kD2207_GPIO21,		0xF8}, // PMU_GPIO_MB_HALL2_IRQ			In, Any Edge, NoWake, No PU
	{ PMU_IIC_BUS, kD2207_BUCK_DWI_CTRL0,	0x23}, // enable DWI for BUCK0, BUCK1, BUCK5
	{ PMU_IIC_BUS, kDIALOG_SYS_CONTROL2,	0x11}, // DWI_EN HIB_32K (for WiFi/BT) <rdar://problem/16257601>

	{ PMU_IIC_BUS, kDIALOG_TEST_ACCESS,	kDIALOG_TEST_ACCESS_ENA}, // enable TEST Access
	{ PMU_IIC_BUS, kD2207_PRE_UVLO_CONF,	0x06}, // SOCHOT0 save power in sleep, disable pull-up <rdar://21643083>
	{ PMU_IIC_BUS, kDIALOG_TEST_ACCESS,	kDIALOG_TEST_ACCESS_DIS}, // disable TEST Access
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
	{ PMU_IIC_BUS, 0, kD2207_BUCK5_VSEL }
};

static const struct core_rails_struct cpu_rails[] =
{
	{ PMU_IIC_BUS, 0, kD2207_BUCK0_VSEL }
};

static const struct core_rails_struct ram_rails[] =
{
	{ PMU_IIC_BUS, 0, kD2207_BUCK2_VSEL }
};

#define TARGET_USE_CHARGE_TABLE 1
static const struct power_charge_limits pmu_charge_table[16] = {
	{ 4100,			1000, 1500,	1004	},  	// 10 - 15C
	{ 4200,			1000, 1500,	1004	},
	{ USHRT_MAX,	1000, 1500,	1004	},
	{ 4100,			1500, 2000,	2509	},		// 15 - 20C
	{ 4200,			1500, 2000,	2007	},
	{ USHRT_MAX,	1500, 2000,	1004	},
	{ 4100,			2000, 4500,	2509	},		// 20 - 45C
	{ 4200,			2000, 4500,	2509	},
	{ USHRT_MAX,	2000, 4500,	2007	},
};

#endif /* POWERCONFIG_PMU_SETUP */

#define NO_BATTERY_VOLTAGE		2700
#define MIN_BOOT_BATTERY_VOLTAGE	3600
#define TARGET_BOOT_BATTERY_VOLTAGE	3700
#define TARGET_BOOT_BATTERY_CAPACITY	60
#define PRECHARGE_BACKLIGHT_LEVEL	824
#define ALWAYS_BOOT_BATTERY_VOLTAGE	(TARGET_BOOT_BATTERY_VOLTAGE + 100)
#define TARGET_PRECHARGE_ALWAYS_DISPLAY_IDLE    1
#define TARGET_MAX_USB_INPUT_CURRENT	2400

#define ACC_PWR_LDO			(6)

#define GASGAUGE_BATTERYID_BLOCK	1
// no charge table 

#endif
