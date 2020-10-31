#ifndef __TARGET_POWERCONFIG_H
#define __TARGET_POWERCONFIG_H

#include <drivers/gasgauge.h>

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
	{ PMU_IIC_BUS, kD2089_HIBERNATE3,       0x88 }, // pp3v0_s2r_sensor (LDO3)
	{ PMU_IIC_BUS, kD2089_HIBERNATE6,       0x24 }, // pp1v8_s2r_sw3 (buck3_sw3) and pp1v2_s2r_sw2 (buck4_sw2)
};

static const struct pmu_setup_struct pmu_warm_init[] =
{
};

static const struct pmu_setup_struct pmu_cold_init[] =
{
	/* GPIO configuration from I/O spreadsheet.
	   <rdar://problem/12012942> J72 IO Spreadsheet
	   Request spreadsheet update before modifying

           The following differences from OTP are intentional:
	   	- GPIO6: Enable wake
		- GPIO7: Enable wake
		- GPIO8: Enable wake
		- GPIO9: Enable wake (baseband targets)
		- GPIO10: Enable wake
		- GPIO11: Enable wake
		- GPIO12: Hi level interrupt, enable wake
		- GPIO16: Enable wake
		*/
	{ PMU_IIC_BUS, kD2089_GPIO1,		0x49 },	// PMU_GPIO_CLK_32K_OSCAR		32k, Push-Pull, VBUCK3
	{ PMU_IIC_BUS, kD2089_GPIO2,		0x49 },	// PMU_GPIO_CLK_32K_WLAN		32k, Push-Pull, VBUCK3
	{ PMU_IIC_BUS, kD2089_GPIO3,		0x09 },	// PMU_GPIO_BT_REG_ON			Out 0, Push-Pull, VBUCK3
	{ PMU_IIC_BUS, kD2089_GPIO4,		0x09 },	// PMU_GPIO_WLAN_REG_ON			Out 0, Push-Pull, VBUCK3
	{ PMU_IIC_BUS, kD2089_GPIO5,		0x09 },	// PMU_GPIO_PMU2BB_RST_L		Out 0, Push-Pull, VBUCK3
	{ PMU_IIC_BUS, kD2089_GPIO6,		0xDA },	// UART5_BATT_TRXD			In, F Edge, Wake, PD
	{ PMU_IIC_BUS, kD2089_GPIO7,		0xBA },	// PMU_GPIO_BT2PMU_HOST_WAKE		In, R Edge, Wake, No PU
	{ PMU_IIC_BUS, kD2089_GPIO8,		0xBA },	// PMU_GPIO_WLAN2PMU_HOST_WAKE		In, R Edge, Wake, No PU
#if TARGET_HAS_BASEBAND
	{ PMU_IIC_BUS, kD2089_GPIO9,		0x7A },	// PMU_GPIO_BB2PMU_HOST_WAKE		In, L High, Wake, No PU
#else
	{ PMU_IIC_BUS, kD2089_GPIO9,		0xB9 },	// PMU_GPIO_BB2PMU_HOST_WAKE		In, R Edge, No Wake, PD, No PU
#endif
	{ PMU_IIC_BUS, kD2089_GPIO10,		0xEA },	// PMU_GPIO_CODEC_HS_INT_L		In, Any Edge, Wake, PU, VBUCK3
	{ PMU_IIC_BUS, kD2089_GPIO11,		0xFA },	// PMU_GPIO_MB_HALL1_IRQ		In, Any Edge, Wake, No PU
	{ PMU_IIC_BUS, kD2089_GPIO12,		0x7B },	// GPIO_TS2SOC2PMU_INT			In, L High, Wake, PD, No PU
	{ PMU_IIC_BUS, kD2089_GPIO13,		0xF8 },	// PMU_GPIO_MB_HALL2_IRQ		In, Any Edge, No PU
	{ PMU_IIC_BUS, kD2089_GPIO14,		0xC8 },	// PMU_GPIO_CODEC_RST_L			In, F Edge, PU, VBUCK3
	{ PMU_IIC_BUS, kD2089_GPIO15,		0xB9 },	// GPIO_NAVAJO2SOC_INT			In, R Edge, No Wake, PD, No PU
	{ PMU_IIC_BUS, kD2089_GPIO16,		0x7B },	// PMU_GPIO_OSCAR2PMU_HOST_WAKE		In, L High, Wake, PD, No PU
	{ PMU_IIC_BUS, kD2089_GPIO17,		0x01 },	// PMU_GPIO_BB_VBUS_DET			Out 0, Push-Pull, VCC_MAIN

	{ PMU_IIC_BUS, kD2089_BUCK_DWI_CTRL0, 	0x03 },		// enable DWI for BUCK0, BUCK1
	{ PMU_IIC_BUS, kD2089_ACT_TO_HIB_DLY,	0x03 },		// set ACT_TO_HIB_DLY to 3 (6ms)
	{ PMU_IIC_BUS, kDIALOG_SYS_CONTROL2,	0x11 },		// enable HIB_32kHz (for WIFI/BT) and DWI
};

static const struct pmu_setup_struct pmu_backlight_enable[] =
{
	{ PMU_IIC_BUS, kDIALOG_WLED_ISET,	0xD4 },	// 50% value from OS
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
	{ PMU_IIC_BUS, kDIALOG_WLED_CONTROL, 0x0 }    // disable LED power
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

#define TARGET_USE_CHARGE_TABLE 1
static const struct power_charge_limits pmu_charge_table[16] = {
	{ 4000,		1000, 1600,	3150	},  // 10 - 16C
	{ 4100,		1000, 1600,	3150	},
	{ 4200,		1000, 1600,	3150	},
	{ USHRT_MAX,	1000, 1600,	2550	},
	{ 4000,		1500, 2100,	3150	},  // 15 - 21C
	{ 4100,		1500, 2100,	3150	},
	{ 4200,		1500, 2100,	3150	},
	{ USHRT_MAX,	1500, 2100,	2550	},
	{ 4000,		2000, 3500,	3150	},  // 20 - 35C
	{ 4100,		2000, 3500,	3150	},
	{ 4200,		2000, 3500,	3150	},
	{ USHRT_MAX,	2000, 3500,	3150	},
	{ 4000,		3400, 4500,	3150	},  // 34 - 45C
	{ 4100,		3400, 4500,	3150	},
	{ 4200,		3400, 4500,	3150	},
	{ USHRT_MAX,	3400, 4500,	3150	}
};

#define NO_BATTERY_VOLTAGE		2700
#define MIN_BOOT_BATTERY_VOLTAGE	3600
#define TARGET_BOOT_BATTERY_VOLTAGE	3700
#define TARGET_BOOT_BATTERY_CAPACITY	50
#define PRECHARGE_BACKLIGHT_LEVEL	1287	/* 10% brightness */
#define ALWAYS_BOOT_BATTERY_VOLTAGE	(TARGET_BOOT_BATTERY_VOLTAGE + 100)
#define TARGET_PRECHARGE_ALWAYS_DISPLAY_IDLE	1
#define TARGET_MAX_USB_INPUT_CURRENT	2400

#define GASGAUGE_BATTERYID_BLOCK	1
#define GASGAUGE_CHARGETABLE_BLOCK	3

static const unsigned int atv_voltage_limit[16] = { 3600, 3650, 3700, 3750, 3800, 3850, 3900, 3950, 4000, 4050, 4100, 4150, 4200, 4250, 4300, 4350 };
static const unsigned int atv_current_limit[16] = { 150, 350, 550, 750, 950, 1150, 1350, 1550, 1750, 1950, 2150, 2350, 2550, 2750, 2950, 3150 };

#define ACC_PWR_LDO			(6)

#endif
