#ifndef __TARGET_POWERCONFIG_H
#define __TARGET_POWERCONFIG_H

/* configuration per target */

#define PMU_IIC_BUS		0
#define CHARGER_IIC_BUS		1

/* this configuration is very PMU specific and must be included exactly once in power.c */
#ifdef POWERCONFIG_PMU_SETUP

static const struct pmu_setup_struct pmu_ldo_warm_setup[] =
{
};

static const struct pmu_setup_struct pmu_ldo_cold_setup[] =
{
	{ PMU_IIC_BUS, kD2255_EN_CMD0, (1<<7)|(1<<2)|(1<<1)|(1<<0) },
	{ PMU_IIC_BUS, kD2255_EN_CMD1, (1<<0) }, // buck8
	{ PMU_IIC_BUS, kD2255_SPI_CTRL, 1 }, // enable SPI access
	
	{ PMU_IIC_BUS, kD2255_LDO9_VSEL, 0x5A},	// <rdar://problem/19034608> iBoot, N71 Proto 2: PMU_VLDO9 = 2.85V, not 2.95V
};

static const struct pmu_setup_struct pmu_warm_init[] =
{
};

//  IO_CONFIG[7:6] 0=I 1=O_OD 2=O_PP 3=SS
//    IO_TYPE[5:3] O 0=L    1=32K  2=nR 3=TI 4=L
//       PUPD[2:1] O 0=NP 1=PU 2=NP     3=<0>
//   WAKE_LVL[0]   O 0=low 1=high
//    IO_TYPE[5:3] I  0=L_AH 1=L_AL 2=RE 3=FE 4=AE
//       PUPD[2:1] I  0=NP 1=PU 2=PD_GND 3=<0>
//   WAKE_LVL[0]   I  0=NW 1=W
//
// <rdar://problem/21564835> N71/N66 IO Spreadsheet v25 (PMU)
static const struct pmu_setup_struct pmu_cold_init[] =
{
	{ PMU_IIC_BUS, kD2255_GPIO1_CONF1,  (0<<6)|(1<<3)|(0<<1)|(1<<0) },	// I.AL.NP.WS   TIGRIS_TO_PMU_INT_L
	{ PMU_IIC_BUS, kD2255_GPIO1_CONF2,  (1<<3)|(2<<0) },			// VBUCK3, 10ms
	{ PMU_IIC_BUS, kD2255_GPIO2_CONF1,  (0<<6)|(1<<3)|(2<<1)|(0<<0) },	// I.AL.PD.NW,  BB_TO_PMU_PCIE_HOST_WAKE_L
	{ PMU_IIC_BUS, kD2255_GPIO2_CONF2,  (1<<3)|(2<<0) },			// VBUCK3, 10ms
	{ PMU_IIC_BUS, kD2255_GPIO3_CONF1,  (2<<6)|(0<<3)|(0<<1)|(0<<0) },	// O_PP.L.NP.0  PMU_TO_BB_PMIC_RESET_R_L
	{ PMU_IIC_BUS, kD2255_GPIO3_CONF2,  (1<<3)|(2<<0) },			// VBUCK3, 10ms
	{ PMU_IIC_BUS, kD2255_GPIO4_CONF1,  (0<<6)|(0<<3)|(2<<1)|(1<<0) },	// I.AH.PD.WS   TRISTAR_TO_AP_INT
	{ PMU_IIC_BUS, kD2255_GPIO4_CONF2,  (1<<3)|(2<<0) },			// VBUCK3, 10ms
	{ PMU_IIC_BUS, kD2255_GPIO5_CONF1,  (0<<6)|(0<<3)|(2<<1)|(1<<0) },	// I.AH.PD.WS   STOCKHOLM_TO_PMU_HOST_WAKE
	{ PMU_IIC_BUS, kD2255_GPIO5_CONF2,  (1<<3)|(2<<0) },			// VBUCK3, 10ms
	{ PMU_IIC_BUS, kD2255_GPIO6_CONF1,  (2<<6)|(0<<3)|(0<<1)|(0<<0) },	// O_PP.L.NP.0  PMU_TO_NAND_LOW_BATT_BOOT_L
	{ PMU_IIC_BUS, kD2255_GPIO6_CONF2,  (1<<3)|(2<<0) },			// VBUCK3, 10ms
	{ PMU_IIC_BUS, kD2255_GPIO7_CONF1,  (0<<6)|(0<<3)|(2<<1)|(1<<0) },	// I.AH.PD.WS   WLAN_TO_PMU_HOST_WAKE
	{ PMU_IIC_BUS, kD2255_GPIO7_CONF2,  (1<<3)|(2<<0) },			// VBUCK3, 10ms
	{ PMU_IIC_BUS, kD2255_GPIO8_CONF1,  (0<<6)|(4<<3)|(1<<1)|(1<<0) },	// I.AE.PU.WS   CODEC_TO_PMU_MIKEY_INT_L
	{ PMU_IIC_BUS, kD2255_GPIO8_CONF2,  (1<<3)|(2<<0) },			// VBUCK3, 10ms
	{ PMU_IIC_BUS, kD2255_GPIO9_CONF1,  (2<<6)|(0<<3)|(0<<1)|(0<<0) },	// O_PP.L.NP.0  PMU_TO_BT_REG_ON
	{ PMU_IIC_BUS, kD2255_GPIO9_CONF2,  (1<<3)|(2<<0) },			// VBUCK3, 10ms
	{ PMU_IIC_BUS, kD2255_GPIO10_CONF1, (0<<6)|(2<<3)|(0<<1)|(1<<0) },	// I.RE.NP.WS   BT_TO_PMU_HOST_WAKE
	{ PMU_IIC_BUS, kD2255_GPIO10_CONF2, (1<<3)|(2<<0) },			// VBUCK3, 10ms
	{ PMU_IIC_BUS, kD2255_GPIO11_CONF1, (2<<6)|(0<<3)|(0<<1)|(0<<0) },	// O_PP.L.NP.0  PMU_TO_WLAN_REG_ON
	{ PMU_IIC_BUS, kD2255_GPIO11_CONF2, (1<<3)|(2<<0) },			// VBUCK3, 10ms
	{ PMU_IIC_BUS, kD2255_GPIO12_CONF1, (1<<6)|(2<<3)|(0<<1)|(0<<0) },	// NC - O_OD.n.NP.0
	{ PMU_IIC_BUS, kD2255_GPIO12_CONF2, (1<<3)|(2<<0) },			// VBUCK3, 10ms
	{ PMU_IIC_BUS, kD2255_GPIO13_CONF1, (1<<6)|(0<<3)|(1<<1)|(0<<0) },	// O_OD.L.PU.0  PMU_TO_CODEC_DIGLDO_PULLDN
	{ PMU_IIC_BUS, kD2255_GPIO13_CONF2, (1<<3)|(2<<0) },			// VBUCK3, 10ms
	{ PMU_IIC_BUS, kD2255_GPIO14_CONF1, (0<<6)|(3<<3)|(1<<1)|(0<<0) },	// I.FE.PU.NW   CODEC_TO_AP_PMU_INT_L
	{ PMU_IIC_BUS, kD2255_GPIO14_CONF2, (1<<3)|(2<<0) },			// VBUCK3, 10ms
	{ PMU_IIC_BUS, kD2255_GPIO15_CONF1, (2<<6)|(0<<3)|(0<<1)|(0<<0) },	// O_PP.L.NP.0 	PMU_TO_BB_USB_VBUS_DETECT
	{ PMU_IIC_BUS, kD2255_GPIO15_CONF2, (1<<3)|(2<<0) },			// VBUCK3, 10ms
	{ PMU_IIC_BUS, kD2255_GPIO16_CONF1, (2<<6)|(0<<3)|(0<<1)|(0<<0) },	// O_PP.L.NP.0	PMU_TO_STOCKHOLM_EN
	{ PMU_IIC_BUS, kD2255_GPIO16_CONF2, (1<<3)|(2<<0) },			// VBUCK3, 10ms
	{ PMU_IIC_BUS, kD2255_GPIO17_CONF1, (3<<6)|(2<<3)|(2<<1)|(0<<0) },	// NC
	{ PMU_IIC_BUS, kD2255_GPIO17_CONF2, (1<<3)|(2<<0) },			// VBUCK3, 10ms
	{ PMU_IIC_BUS, kD2255_GPIO18_CONF1, (2<<6)|(0<<3)|(0<<1)|(0<<0) },	// NC
	{ PMU_IIC_BUS, kD2255_GPIO18_CONF2, (1<<3)|(2<<0) },			// VBUCK3, 10ms
	{ PMU_IIC_BUS, kD2255_GPIO19_CONF1, (2<<6)|(0<<3)|(0<<1)|(1<<0) },	// O_PP.L.NP.1  PMU_TO_LCM_PANICB use OTP-AJ..AG setting rdar://19987012
	{ PMU_IIC_BUS, kD2255_GPIO19_CONF2, (2<<3)|(2<<0) },			// VBUCK3_SW1, 10ms
	{ PMU_IIC_BUS, kD2255_GPIO20_CONF1, (3<<6)|(2<<3)|(2<<1)|(0<<0) },	// NC
	{ PMU_IIC_BUS, kD2255_GPIO20_CONF2, (1<<3)|(2<<0) },			// VBUCK3, 10ms
	{ PMU_IIC_BUS, kD2255_GPIO21_CONF1, (1<<6)|(0<<3)|(0<<1)|(1<<0) },	// O_OD.L.NP.1  I2C0_AP_SCL
	{ PMU_IIC_BUS, kD2255_GPIO21_CONF2, (2<<3)|(2<<0) },			// VBUCK3_SW1, 10ms

	{ PMU_IIC_BUS, kDIALOG_TEST_ACCESS,		kDIALOG_TEST_ACCESS_ENA },	// enable test register access
	// Fix Slew Rate on Buck2
	{ PMU_IIC_BUS, kD2255_BUCK2_FSM_TRIM6, 0 },  
	{ PMU_IIC_BUS, kD2255_BUCK2_FSM_TRIM7, 0 },  
	{ PMU_IIC_BUS, kD2255_PWRONOFF_OUT_32K_EN, 0x7 },   // OUT_32K active in ACTIVE, SLEEP1,2
	{ PMU_IIC_BUS, kD2255_BUCK2_MINV, 0x8 },            // <rdar://problem/21472020> Lower BUCK2_MINV
	{ PMU_IIC_BUS, kD2255_SYSCTL_PRE_UVLO_CTRL, 0x46 }, // <rdar://problem/21496782> + <rdar://problem/21807731>
        // Set Buck5 Current
#if SUB_PLATFORM_S8003
	{ PMU_IIC_BUS, kD2255_BUCK5_ANA_TRIM14, 0x36 },		// ILIM at 1.80A Max Load at 1.43 rdar://19846032
#else
	{ PMU_IIC_BUS, kD2255_BUCK5_ANA_TRIM14, 0x39 },		// ILIM at 1.89A Max Load at 1.52 rdar://19724394
#endif
	{ PMU_IIC_BUS, kDIALOG_TEST_ACCESS,		kDIALOG_TEST_ACCESS_DIS },	// disabled test register access

    { PMU_IIC_BUS, kD2255_OUT_32K, 0x49 },  			// OUT_32K active, VBUCK3, PP
        
};

static const struct core_rails_struct soc_rails[] =
{
	{ PMU_IIC_BUS, 0, kD2255_BUCK2_VSEL }
};

static const struct core_rails_struct cpu_rails[] =
{
	{ PMU_IIC_BUS, 0, kD2255_BUCK0_VSEL }
};

// it's VDD FIXED! used for RAM inside the SOC
static const struct core_rails_struct ram_rails[] =
{
	{ PMU_IIC_BUS, 0, kD2255_BUCK5_VSEL } 
};

// this is indexed with POWER_RAIL_*, I really just need to use the index in LDO[]
target_rails_t target_rails = {
    { 0x0F, 500, 1296, 3125 }, // POWER_RAIL_CPU,         BUCK0
    { 0x14, 600, 1396, 3125 }, // POWER_RAIL_VDD_FIXED    BUCK5
    { 0x11, 600, 1396, 3125 }, // POWER_RAIL_SOC,         BUCK2

    { 0x16, 600, 1396, 3125 }, // POWER_RAIL_CPU_RAM,     BUCK7
    { 0x10, 500, 1296, 3125 }, // POWER_RAIL_GPU,         BUCK1
    { 0x17, 600, 1396, 3125 }, // POWER_RAIL_GPU_RAM,     BUCK8
};

#endif /* POWERCONFIG_PMU_SETUP */

#define NO_BATTERY_VOLTAGE		    2700
#define MIN_BOOT_BATTERY_VOLTAGE	3600
#define TARGET_BOOT_BATTERY_VOLTAGE	3700
#define TARGET_POWER_NEEDS_BATTERY_PROTECTION_RESET 1

#define PRECHARGE_BACKLIGHT_LEVEL	824
#define ALWAYS_BOOT_BATTERY_VOLTAGE	(TARGET_BOOT_BATTERY_VOLTAGE + 100)


#define ACC_PWR_LDO			(6)

#endif
