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
	{ PMU_IIC_BUS, kD2186_ACTIVE1,		0x3f },		// <rdar://problem/18255102> J42: Enable GPU in iBoot
	{ PMU_IIC_BUS, kD2186_ACTIVE6,		0x11 },		// <rdar://problem/17940873> J42: DEV1 PMU OTP Change - Disable BUCK4_SW2 in HIB and ACT
	{ PMU_IIC_BUS, kD2186_HIBERNATE6,       0x00 },         // <rdar://problem/17940873> J42: DEV1 PMU OTP Change - Disable BUCK4_SW2 in HIB and ACT
};


#define PMU_LDO_COLD_SETUP_AP_DEV	1

static const struct pmu_setup_struct pmu_ldo_cold_setup_dev[] =
{
};

static const struct pmu_setup_struct pmu_ldo_cold_setup_ap[] =
{
	// <rdar://problem/18181374> J42: PMU PreProto OTP Override [URGENT!]
	{ PMU_IIC_BUS, kD2186_ACTIVE4,		0x0f },		// (LDO9 enabled in Active)
	{ PMU_IIC_BUS, kD2186_HIBERNATE3,	0x44 },		// (LDO5 disabled in Hibernate)
	{ PMU_IIC_BUS, kD2186_HIBERNATE4,	0x03 },		// (LDO9 enabled in Hibernate)
	{ PMU_IIC_BUS, kD2186_LDO9_VSEL,	0x54 },		// (LDO9 set to 3.3V)
	{ PMU_IIC_BUS, kD2186_BUCK5_SLOT,	0x33 },		// (BUCK5 slot 3)
	{ PMU_IIC_BUS, kD2186_LDO9_SLOT,	0x33 },		// (LDO9 slot 3)
	{ PMU_IIC_BUS, kD2186_LDO10_SLOT,	0x44 },		// (LDO10 slot 4)
};

static const struct pmu_setup_struct pmu_warm_init[] =
{
};

static const struct pmu_setup_struct pmu_cold_init[] =
{

	{ PMU_IIC_BUS, kD2186_GPIO3_CONF1,	0x00 },		// <rdar://problem/18585578> J42 IOKit / wlan / Match Timeout [iBoot BT_REG_ON]
	{ PMU_IIC_BUS, kD2186_GPIO4_CONF1,	0x00 },		// <rdar://problem/17960476> J42: DEV1 OTP Change - WLAN_REG_ON should be low
	{ PMU_IIC_BUS, kD2186_GPIO6_CONF1,	0x00 },		// <rdar://problem/17922907> J42: DEV1 PMU OTP Change - Assert HVR_RESET_L
	{ PMU_IIC_BUS, kD2186_GPIO7_CONF1,	0x92 },		// <rdar://problem/18036378> GPIO_BT2PMU_HOST_WAKE	In, R Edge, Wake, PU
	{ PMU_IIC_BUS, kD2186_GPIO7_CONF2,	0x72 },		// <rdar://problem/18036378> GPIO_BT2PMU_HOST_WAKE	PU Disabled
	{ PMU_IIC_BUS, kD2186_GPIO12_CONF1,	0x81 },		// <rdar://problem/18414083> j42: disable wake on tristar pmu gpio 12
	{ PMU_IIC_BUS, kD2186_GPIO17_CONF1,	0x93 },		// <rdar://problem/17923135> J42: DEV1 PMU OTP Change - GPIO17 Wake on Rising Edge
	{ PMU_IIC_BUS, kD2186_GPIO20_CONF1,	0xa1 },         // <rdar://problem/18388179> J42: PMU kD2186_GPIO20_CONF1 needs to be updated to 0xa1
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

#define TARGET_POWER_NO_BATTERY		1
#define ALWAYS_BOOT_BATTERY_VOLTAGE	(3000)
#define PRECHARGE_BACKLIGHT_LEVEL	103
#define TARGET_MAX_USB_INPUT_CURRENT	2400

#define ACC_PWR_LDO			(0)	// <rdar://problem/17977549> J42: PMU LDO config wrong in iBoot

#endif
