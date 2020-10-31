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
	{ PMU_IIC_BUS, kD2186_ACTIVE3,		0xAE },		// Current OTP enables LDO{1,2,3,4,5,7}, disable LDO4.
	{ PMU_IIC_BUS, kD2186_ACTIVE6,		0x37 },		// enable BUCK3_SW1, BUCK3_SW2, BUCK3_SW3, BUCK4_SW1, BUCK4_SW2
	{ PMU_IIC_BUS, kD2186_ACTIVE3,		0xBE },		// enable LDO4 along with LDO{1,2,3,5,7}

	{ PMU_IIC_BUS, kD2186_BUCK1_VSEL,	0x80 },		// rdar://14689066 TODO: remove when not necessary

	{ PMU_IIC_BUS, kD2186_OUT_32K,		0x49 },		// enable OUT32 clock

	{ PMU_IIC_BUS, kD2186_BUCK6_VSEL,	0x26 },		// <rdar://problem/15038492> N56/T133 Adi PMU: Change default voltage on BUCK6 to 2.0375 V

	{ PMU_IIC_BUS, kD2186_LDO9_VSEL,	0x44 },		// <rdar://problem/15047684> N56/T133/N61 Front Camera AVDD Voltage setting from PMU
};

static const struct pmu_setup_struct pmu_warm_init[] =
{
};

static const struct pmu_setup_struct pmu_cold_init[] =
{
	{ PMU_IIC_BUS, kD2186_GPIO1_CONF1,		0x88 }, 	// CHG_TO_PMU_INT_L		In, L Low, No Wake, PU
	{ PMU_IIC_BUS, kD2186_GPIO1_CONF2,		0x72 },		// CHG_TO_PMU_INT_L		POR
	{ PMU_IIC_BUS, kD2186_GPIO2_CONF1,		0x8a },		// BB_TO_PMU_HOST_WAKE_L	In, L Low, Wake, PU
	{ PMU_IIC_BUS, kD2186_GPIO2_CONF2,		0x72 },		// BB_TO_PMU_HOST_WAKE_L	POR
	{ PMU_IIC_BUS, kD2186_GPIO3_CONF1,		0x01 },		// PMU_TO_BB_RST_R_L		Out 0, Low , Push-Pull
	{ PMU_IIC_BUS, kD2186_GPIO3_CONF2,		0x12 },		// PMU_TO_BB_RST_R_L		VBUCK3
	{ PMU_IIC_BUS, kD2186_GPIO4_CONF1,		0x82 },		// TRISTAR_TO_AP_INT		In, L High, Wake, PU
	{ PMU_IIC_BUS, kD2186_GPIO4_CONF2,		0x72 },		// TRISTAR_TO_AP_INT		POR
	{ PMU_IIC_BUS, kD2186_GPIO5_CONF1,		0x83 },		// STOCKHOLM_TO_PMU_HOST_WAKE_L In, L High, Wake, PD
	{ PMU_IIC_BUS, kD2186_GPIO5_CONF2,		0x72 },		// STOCKHOLM_TO_PMU_HOST_WAKE_L POR
	{ PMU_IIC_BUS, kD2186_GPIO6_CONF1,		0x01 },		// PMU_TO_OSCAR_RESET_CLK_32K_L Out 0, Push-Pull
	{ PMU_IIC_BUS, kD2186_GPIO6_CONF2,		0x12 },		// PMU_TO_OSCAR_RESET_CLK_32K_L VBUCK3
	{ PMU_IIC_BUS, kD2186_GPIO7_CONF1,		0x83 },		// WLAN_TO_PMU_HOST_WAKE	In, L High, Wake, PD
	{ PMU_IIC_BUS, kD2186_GPIO7_CONF2,		0x72 },		// WLAN_TO_PMU_HOST_WAKE	POR
	{ PMU_IIC_BUS, kD2186_GPIO8_CONF1,		0xA2 },		// CODEC_TO_PMU_MIKEY_INT_L	In, Any Edge, Wake, PU
	{ PMU_IIC_BUS, kD2186_GPIO8_CONF2,		0x12 },		// CODEC_TO_PMU_MIKEY_INT_L	VBUCK3
	{ PMU_IIC_BUS, kD2186_GPIO9_CONF1,		0x01 },		// PMU_TO_BT_REG_ON_R		Out 0, Push-Pull
	{ PMU_IIC_BUS, kD2186_GPIO9_CONF2,		0x12 },		// PMU_TO_BT_REG_ON_R		VBUCK3
	{ PMU_IIC_BUS, kD2186_GPIO10_CONF1,		0x92 },		// BT_TO_PMU_HOST_WAKE		In, R Edge, Wake, Push-Pull
	{ PMU_IIC_BUS, kD2186_GPIO10_CONF2,		0x72 },		// BT_TO_PMU_HOST_WAKE		Disabled
//	{ PMU_IIC_BUS, kD2186_GPIO11_CONF1,		0x01 },		// PMU_TO_WLAN_REG_ON_R		Out 0, Push-Pull
	{ PMU_IIC_BUS, kD2186_GPIO11_CONF1,		0x03 },		// PMU_TO_WLAN_REG_ON_R		Out 1, Push-Pull (FIXME! rdar://14957372)
	{ PMU_IIC_BUS, kD2186_GPIO11_CONF2,		0x12 },		// PMU_TO_WLAN_REG_ON_R		VBUCK3
	{ PMU_IIC_BUS, kD2186_GPIO12_CONF1,		0x08},		// AP_TO_I2C0_SCL		Out, nReset, PU
	{ PMU_IIC_BUS, kD2186_GPIO12_CONF2,		0x72 },		// AP_TO_I2C0_SCL		POR
	{ PMU_IIC_BUS, kD2186_GPIO13_CONF1,		0x83 },		// OSCAR_TO_PMU_HOST_WAKE	In, In, L High, Wake, PD
	{ PMU_IIC_BUS, kD2186_GPIO13_CONF2,		0x72 },		// OSCAR_TO_PMU_HOST_WAKE	POR
	{ PMU_IIC_BUS, kD2186_GPIO14_CONF1,		0x91 },		//				NC
	{ PMU_IIC_BUS, kD2186_GPIO14_CONF2,		0x72 },		//				POR
	{ PMU_IIC_BUS, kD2186_GPIO15_CONF1,		0x01 },		// PMU_TO_BB_VBUS_DET		Out 0, Low, Push-Pull
	{ PMU_IIC_BUS, kD2186_GPIO15_CONF2,		0x02 },		// PMU_TO_BB_VBUS_DET		VCC_MAIN
	{ PMU_IIC_BUS, kD2186_GPIO16_CONF1,		0x91 },		//				NC
	{ PMU_IIC_BUS, kD2186_GPIO16_CONF2,		0x72 },		//				POR
	{ PMU_IIC_BUS, kD2186_GPIO17_CONF1,		0x9A },		// WLAN_TO_PMU_PCIE_WAKE_L	In, F Edge, Wake, PU
	{ PMU_IIC_BUS, kD2186_GPIO17_CONF2,		0x12 },		// WLAN_TO_PMU_PCIE_WAKE_L	VBUCK3
	{ PMU_IIC_BUS, kD2186_GPIO18_CONF1,		0x01 },		// PMU_TO_ACC_SW_ON		Out 0, Low, Push-Pull
	{ PMU_IIC_BUS, kD2186_GPIO18_CONF2,		0x12 },		// PMU_TO_ACC_SW_ON		VBUCK3
	{ PMU_IIC_BUS, kD2186_GPIO19_CONF1,		0x93 },		// NC_HALL1_TO_PMU_WAKE		In, R Edge, Wake, PD
	{ PMU_IIC_BUS, kD2186_GPIO19_CONF2,		0x12 },		// NC_HALL1_TO_PMU_WAKE		VBUCK3
	{ PMU_IIC_BUS, kD2186_GPIO20_CONF1,		0x93 },		// NC_HALL2_TO_PMU_WAKE		In, R Edge, Wake, PD
	{ PMU_IIC_BUS, kD2186_GPIO20_CONF2,		0x12 },		// NC_HALL2_TO_PMU_WAKE		VBUCK3
	{ PMU_IIC_BUS, kD2186_GPIO21_CONF1,		0x91 },		//				NC
	{ PMU_IIC_BUS, kD2186_GPIO21_CONF2,		0x72 },		//				POR
	
// TODO: configure debounce for WAS kD2045_GPIO_DEB2,kD2045_GPIO_DEB5,kD2045_GPIO_DEB6?

	{ PMU_IIC_BUS, kD2186_BUCK_DWI_CTRL0,	0x07 },			// enable DWI for BUCK0, BUCK1, BUCK2
	{ PMU_IIC_BUS, kD2186_SYS_CTRL2,		0x11 },		// enable HIB_32kHz (for WIFI/BT) and DWI

// TODO: verify
	{ PMU_IIC_BUS, kD2186_ACT_TO_HIB_DLY,	0x03 },			// set ACT_TO_HIB_DLY to 3 (6ms)
};

static const struct core_rails_struct soc_rails[] =
{
	{ PMU_IIC_BUS, 0, kD2186_BUCK2_VSEL }
};

static const struct core_rails_struct cpu_rails[] =
{
	{ PMU_IIC_BUS, 0, kD2186_BUCK0_VSEL }
};

static const struct core_rails_struct ram_rails[] =
{
	{ PMU_IIC_BUS, 0, kD2186_BUCK5_VSEL }
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
