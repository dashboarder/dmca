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
	{ PMU_IIC_BUS, kD2207_HIBERNATE3,   0xAA },		// enable LDO7 (Tristar), LDO5(Mesa) and LDO3(Sensors), LDO1(Hall) <rdar://16167964>
	{ PMU_IIC_BUS, kD2207_HIBERNATE6,   0x26 },		// enable BUCK4_SW2+BUCK3_SW3+BUCK3_SW2 (Oscar Rails, DVDD) in hibernate 
	{ PMU_IIC_BUS, kD2207_ACTIVE2,      0x03 },		// WLEDA/WLEDB enable
};

static const struct pmu_setup_struct pmu_warm_init[] =
{

};

static const struct pmu_setup_struct pmu_cold_init[] =
{
	/* <rdar://problem/14847504> J82 IO Spreadsheet */
#ifndef TARGET_HAS_BASEBAND
	{ PMU_IIC_BUS, kD2207_GPIO5,		0xB9}, // GPIO_PMU2BBPMU_RESET_R_L		In, R Edge, PD
#endif
	{ PMU_IIC_BUS, kD2207_GPIO6,		0xDA}, // UART_BATT_HDQ				In, F Edge, Wake, No PU
	{ PMU_IIC_BUS, kD2207_GPIO7,		0xBA}, // GPIO_BT2PMU_HOST_WAKE			In, R Edge, Wake, No PU
	{ PMU_IIC_BUS, kD2207_GPIO8,		0x7B}, // GPIO_WLAN2PMU_HOST_WAKE		In, Hi level, Wake, PD
#ifdef TARGET_HAS_BASEBAND
	{ PMU_IIC_BUS, kD2207_GPIO9,		0x8A}, // GPIO_BB2PMU_HOST_WAKE_L		In, Active low, Wake, PU, VBUCK3
#else
	{ PMU_IIC_BUS, kD2207_GPIO9,		0xB9}, // GPIO_BB2PMU_HOST_WAKE_L		In, R Edge, PD
#endif
	{ PMU_IIC_BUS, kD2207_GPIO10,		0xEA}, // GPIO_CODEC2PMU_HS_IRQ_L		In, Any Edge, Wake, PU, VBUCK3
	{ PMU_IIC_BUS, kD2207_GPIO11,		0xFA}, // GPIO_HALL2PMU_IRQ0			In, Any Edge, Wake, No PU
	{ PMU_IIC_BUS, kD2207_GPIO12,		0x7B}, // GPIO_TS2SOC2PMU_IRQ			In, Hi level, Wake, PD
	{ PMU_IIC_BUS, kD2207_GPIO16,		0x7B}, // GPIO_OSCAR2PMU_HOST_WAKE		In, Hi level, Wake, PD

	{ PMU_IIC_BUS, kD2207_BUCK_DWI_CTRL0,	0x23 },		// enable DWI for BUCK0, BUCK1, BUCK5  <rdar://16369016>
	{ PMU_IIC_BUS, kDIALOG_SYS_CONTROL2,	0x11 },		// DWI_EN HIB_32K (for WiFi/BT) <rdar://15647984>
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

#define TARGET_USE_CHARGE_TABLE 1

// Need this only once in lib/power/power.c
#if POWERCONFIG_CHARGE_TABLE
static const struct power_charge_limits pmu_charge_table[16] = {
	{ 4200,	1000, 1500, 1511 }, { USHRT_MAX, 1000, 1500, 756  }, // 10°C < T ≤ 15°C
	{ 4200,	1500, 2000, 3021 }, { USHRT_MAX, 1500, 2000, 2266 }, // 15°C < T ≤ 20°C
	{ 4200,	2000, 4500, 3021 }, { USHRT_MAX, 2000, 4500, 3021 }, // 20°C < T ≤ 45°C
};
#endif

#define NO_BATTERY_VOLTAGE		2700
#define MIN_BOOT_BATTERY_VOLTAGE	3600
#define TARGET_BOOT_BATTERY_VOLTAGE	3700
#define TARGET_BOOT_BATTERY_CAPACITY	50
#define PRECHARGE_BACKLIGHT_LEVEL	824
#define ALWAYS_BOOT_BATTERY_VOLTAGE	(TARGET_BOOT_BATTERY_VOLTAGE + 100)
#define TARGET_PRECHARGE_ALWAYS_DISPLAY_IDLE    1
#define TARGET_MAX_USB_INPUT_CURRENT	2400

#define ACC_PWR_LDO			(6)

#define GASGAUGE_BATTERYID_BLOCK	1
// NOTE: this product doesn't have a charge table

#endif
