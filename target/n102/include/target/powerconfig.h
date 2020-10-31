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
	{ PMU_IIC_BUS, kD2186_ACTIVE4,		0x04 },		// enable LDO10
	{ PMU_IIC_BUS, kD2186_ACTIVE6,		0x17 },		// enable BUCK3_SW1, BUCK3_SW2, BUCK3_SW3, BUCK4_SW1
	{ PMU_IIC_BUS, kD2186_ACTIVE3,		0x3E },		// enable LDO4 along with LDO{1,2,3,5}
	{ PMU_IIC_BUS, kD2186_ACTIVE6,		0x37 },		// enable BUCK4_SW2 <rdar://problem/15904020> N56 and N61: PP1V2_Oscar and PP1V8_Oscar brought up in wrong order during iBoot
	{ PMU_IIC_BUS, kD2186_HIBERNATE3,	0x18 },		// enable LDO3+LDO4 in hibernate
	{ PMU_IIC_BUS, kD2186_OUT_32K,		0x49 },		// enable OUT32 clock
};

static const struct pmu_setup_struct pmu_warm_init[] =
{
};

// Settings as of <rdar://problem/16942620> N102 | IOSpreadsheet, version 0v4
static const struct pmu_setup_struct pmu_cold_init[] =
{
	{ PMU_IIC_BUS, kD2186_GPIO1_CONF1,		0x91 },		//				NC
	{ PMU_IIC_BUS, kD2186_GPIO1_CONF2,		0x72 },		//				POR
	{ PMU_IIC_BUS, kD2186_GPIO2_CONF1,		0x91 },		//				NC
	{ PMU_IIC_BUS, kD2186_GPIO2_CONF2,		0x72 },		//				POR
	{ PMU_IIC_BUS, kD2186_GPIO3_CONF1,		0x91 },		//				NC
	{ PMU_IIC_BUS, kD2186_GPIO3_CONF2,		0x72 },		//				POR
	{ PMU_IIC_BUS, kD2186_GPIO4_CONF1,		0x83 },		// TRISTAR_TO_AP_INT		In, L High, Wake, PD
	{ PMU_IIC_BUS, kD2186_GPIO4_CONF2,		0x72 },		// TRISTAR_TO_AP_INT		POR
	{ PMU_IIC_BUS, kD2186_GPIO5_CONF1,		0x91 },		//				NC
	{ PMU_IIC_BUS, kD2186_GPIO5_CONF2,		0x72 },		//				POR
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
	{ PMU_IIC_BUS, kD2186_GPIO11_CONF1,		0x01 },		// PMU_TO_WLAN_REG_ON_R		Out 0, Push-Pull
	{ PMU_IIC_BUS, kD2186_GPIO11_CONF2,		0x12 },		// PMU_TO_WLAN_REG_ON_R		VBUCK3
	{ PMU_IIC_BUS, kD2186_GPIO12_CONF1,		0x08 },		// AP_TO_I2C0_SCL		Out, nReset, PU
	{ PMU_IIC_BUS, kD2186_GPIO12_CONF2,		0x72 },		// AP_TO_I2C0_SCL		POR
	{ PMU_IIC_BUS, kD2186_GPIO13_CONF1,		0x83 },		// OSCAR_TO_PMU_HOST_WAKE	In, In, L High, Wake, PD
	{ PMU_IIC_BUS, kD2186_GPIO13_CONF2,		0x72 },		// OSCAR_TO_PMU_HOST_WAKE	POR
	{ PMU_IIC_BUS, kD2186_GPIO14_CONF1,		0x91 },		//				NC
	{ PMU_IIC_BUS, kD2186_GPIO14_CONF2,		0x72 },		//				POR
	{ PMU_IIC_BUS, kD2186_GPIO15_CONF1,		0x91 },		//				NC
	{ PMU_IIC_BUS, kD2186_GPIO15_CONF2,		0x72 },		//				POR
	{ PMU_IIC_BUS, kD2186_GPIO16_CONF1,		0x91 },		//				NC
	{ PMU_IIC_BUS, kD2186_GPIO16_CONF2,		0x72 },		//				POR
	{ PMU_IIC_BUS, kD2186_GPIO17_CONF1,		0x88 },		// WLAN_TO_PMU_PCIE_WAKE_L	In, L Low, PU
	{ PMU_IIC_BUS, kD2186_GPIO17_CONF2,		0x72 },		// WLAN_TO_PMU_PCIE_WAKE_L	POR
	{ PMU_IIC_BUS, kD2186_GPIO18_CONF1,		0x91 },		//				NC
	{ PMU_IIC_BUS, kD2186_GPIO18_CONF2,		0x72 },		//				POR
	{ PMU_IIC_BUS, kD2186_GPIO19_CONF1,		0x91 },		//				NC
	{ PMU_IIC_BUS, kD2186_GPIO19_CONF2,		0x72 },		//				POR
	{ PMU_IIC_BUS, kD2186_GPIO20_CONF1,		0x91 },		//				NC
	{ PMU_IIC_BUS, kD2186_GPIO20_CONF2,		0x72 },		//				POR
	{ PMU_IIC_BUS, kD2186_GPIO21_CONF1,		0x91 },		//				NC
	{ PMU_IIC_BUS, kD2186_GPIO21_CONF2,		0x72 },		//				POR
	
// TODO: configure debounce for WAS kD2045_GPIO_DEB2,kD2045_GPIO_DEB5,kD2045_GPIO_DEB6?

	{ PMU_IIC_BUS, kD2186_BUCK_DWI_CTRL0,	0x07 },		// enable DWI for BUCK0, BUCK1, BUCK2
	{ PMU_IIC_BUS, kD2186_SYS_CTRL2,		0x11 },		// enable HIB_32kHz (for WIFI/BT) and DWI

// TODO: verify
	{ PMU_IIC_BUS, kD2186_ACT_TO_HIB_DLY,	0x03 },		// set ACT_TO_HIB_DLY to 3 (6ms)
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
