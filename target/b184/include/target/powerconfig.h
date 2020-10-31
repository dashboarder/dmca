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

static const struct pmu_setup_struct pmu_cold_init[] =
{
	{ PMU_IIC_BUS, kD2045_GPIO1,		0xBB }, 	// AUD_ARRAY_HOST_WAKE		In, R Edge, No PU, Wake, PD
	{ PMU_IIC_BUS, kD2045_GPIO2,		0xBB }, 	// WLAN2_HOST_WAKE		IN, R Edge, No PU, Wake, PD
	{ PMU_IIC_BUS, kD2045_GPIO3,		0x0B }, 	// WLAN2_REG_ON			Out 1, Buck 3, push-pull
	{ PMU_IIC_BUS, kD2045_GPIO4,		0xB9 }, 	// TRI_INT			In, R Edge, No PU, no wake, PD
	{ PMU_IIC_BUS, kD2045_GPIO5,		0x09 }, 	// BT1_REG_ON
	{ PMU_IIC_BUS, kD2045_GPIO6,		0xBB }, 	// BT1_HOST_WAKE		In, R Edge, No PU, wake, PD
	{ PMU_IIC_BUS, kD2045_GPIO7,		0xBB }, 	// WLAN1_HOST_WAKE		In, R Edge, No PU, wake, PD
	{ PMU_IIC_BUS, kD2045_GPIO8,		0xBB }, 	// ENET_PHY_HOST_WAKE		In, R Edge, No PU, wake, PD
	{ PMU_IIC_BUS, kD2045_GPIO9,		0x09 }, 	// BT2_REG_ON			Out 0, Push-Pull, VBUCK3
	{ PMU_IIC_BUS, kD2045_GPIO10,		0xBB }, 	// BT2_HOST_WAKE		In, R Edge, No PU, wake, PD
	{ PMU_IIC_BUS, kD2045_GPIO11,		0x0B }, 	// WLAN1_REG_ON			Out 1, Push-Pull, VBUCK3
	{ PMU_IIC_BUS, kD2045_GPIO12,		0xBB }, 	// ENET_PME_1V8			In, R Edge, No PU, wake, PD
	{ PMU_IIC_BUS, kD2045_GPIO13,		0xBB }, 	// MCU_HOST_WAKE		In, R Edge, No PU, wake, PD
	{ PMU_IIC_BUS, kD2045_GPIO14,		0x0B }, 	// PS_ON (16V)			Out 1, VBUCK3, open drain
	{ PMU_IIC_BUS, kD2045_GPIO15,		0x7A }, 	// PMU_WAKE_ON_VCC
	{ PMU_IIC_BUS, kD2045_GPIO16,		0xBB }, 	// ACCEL_HOST_WAKE1		In, R Edge, No PU, wake, PD
	{ PMU_IIC_BUS, kD2045_OUT_32K, 		0x49 },		// OUT_32K (WLAN_CLK32K)	Out 32K, Push-Pull, VBUCK3

	{ PMU_IIC_BUS, kD2045_BUCK_DWI_CTRL0, 	0x03 },		// enable DWI for BUCK0, BUCK1

	{ PMU_IIC_BUS, kD2045_BUTTON1_CONF,	0x84 },		// PMU_BUTTTON1
	{ PMU_IIC_BUS, kD2045_BUTTON2_CONF,	0x84 },		// PMU_BUTTTON2
	{ PMU_IIC_BUS, kD2045_BUTTON3_CONF,	0xD4 },		// PMU_BUTTTON3
	{ PMU_IIC_BUS, kD2045_BUTTON4_CONF,	0x94 },		// PMU_BUTTTON4

	// Special function
	{ PMU_IIC_BUS, kDIALOG_BUTTON_DBL,	0x00 },		// 
	{ PMU_IIC_BUS, kD2045_BUTTON_WAKE,  0x00 },		// AUD_ARRAY_HOST_WAKE
	{ PMU_IIC_BUS, kD2045_RESET_IN1_CONF,0x02}, 	// PMU_RESET_IN1_WDOG
	{ PMU_IIC_BUS, kD2045_RESET_IN2_CONF,0x00}, 	// PMU_RESET_IN2_TRISTAR
	{ PMU_IIC_BUS, kD2045_RESET_IN3_CONF,0x14}, 	// SOCHOT1_L

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

#define TARGET_POWER_NO_BATTERY		1
#define ALWAYS_BOOT_BATTERY_VOLTAGE	(3000)
#define PRECHARGE_BACKLIGHT_LEVEL	103
#define TARGET_MAX_USB_INPUT_CURRENT	2400

#define ACC_PWR_LDO			(6)

#endif
