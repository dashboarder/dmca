#ifndef __TARGET_POWERCONFIG_H
#define __TARGET_POWERCONFIG_H

// Configuration per target

#define PMU_IIC_BUS		0

// This configuration is very PMU specific and must be included exactly
// once in power.c
#ifdef POWERCONFIG_PMU_SETUP

static const struct pmu_setup_struct pmu_ldo_warm_setup[] =
{
};

static const struct pmu_setup_struct pmu_ldo_cold_setup[] =
{
    { PMU_IIC_BUS, kD2257_EN_CMD0, (1<<7)|(1<<3)|(1<<2)|(1<<1)|(1<<0) },
    { PMU_IIC_BUS, kD2257_EN_CMD1, (1<<0) }, // buck8
    { PMU_IIC_BUS, kD2257_EN_CMD2, 1 },      // enable WLED SPI access
    { PMU_IIC_BUS, kD2257_SPI_CTRL, 1 }      // enable SPI access
};

static const struct pmu_setup_struct pmu_warm_init[] =
{
};

static const struct pmu_setup_struct pmu_cold_init[] =
{
    // TODO :This is a Bringup Hack, remove
    { PMU_IIC_BUS, kD2257_BUCK0_VSEL, 0xc0},       // CPU rail
    { PMU_IIC_BUS, kD2257_BUCK1_VSEL, 0xc0},       // GPU rail
    { PMU_IIC_BUS, kD2257_PWRONOFF_BUCK1_EN, 0xf}, // GPU power rail (see <rdar://problem/19733370>)
    { PMU_IIC_BUS, kD2257_PWRONOFF_BUCK8_EN, 0xf}, // GPU RAM power rail (see <rdar://problem/19733370>)
    // TODO :This is a Bringup Hack, remove

    { PMU_IIC_BUS, kD2257_PWRONOFF_LDO4_EN, 0x1},  // Enable PP3V0_ALS
    { PMU_IIC_BUS, kD2257_PWRONOFF_LDO7_EN, 0x1},  // Enable PP3V3_USB3

    { PMU_IIC_BUS, kD2257_LDO2_BYPASS, 0x1}, // bypass when Active rdar://19773152

    // Drive GPIO_PMU_TO_1V1_EXT_SW_ON (PMU GPIO 20) to enable PP1V1_S1_EXT_SW
    // which inturn is SNSI_PP1V1_S1_DDR_VDDQ_x.
    //
    // Should be ON by default via OTP.
    
    // Drive GPIO_PMU_TO_1V8_EXT_SW2_ON (PMU GPIO 21) to enable PP1V8_S2_EXT_SW2
    // which inturn is PP1V8_S2_IO_EXPANDER.
    //
    // Should be ON by default via OTP.

    { PMU_IIC_BUS, 0x7000, 0x1D },     // Enable Test Mode
    { PMU_IIC_BUS, kD2257_EN_CMD3, 0}, // Enable SPI access for BUCKx (x=3)
    { PMU_IIC_BUS, 0x7000, 0x0 },      // Disable Test Mode

#if SUB_TARGET_J98a
    { PMU_IIC_BUS, kD2257_GPIO5_CONF1, 0x14},
    { PMU_IIC_BUS, kD2257_GPIO5_CONF2, 0x0F},
    { PMU_IIC_BUS, kD2257_GPIO9_CONF1, 0x14},
    { PMU_IIC_BUS, kD2257_GPIO9_CONF2, 0x0F}
#endif

    // ElbaRefUI uses PMU_GPIO19 as GPIO_BEACON_EN_R (J99a uses UART7_RXD as GPIO_SOC_TO_BEACON_EN)
    { PMU_IIC_BUS, kD2257_GPIO19_CONF1,  (2<<6)|(0<<3)|(0<<1)|(1<<0) },	// 0x80 Output(Push-Pull), Level, no PU/PD, High
    { PMU_IIC_BUS, kD2257_GPIO19_CONF2,  (1<<3)|(0<<0) },		// 0x08 VBUCK3, no DEB
};

static const struct pmu_setup_struct pmu_backlight_enable[] =
{
    { PMU_IIC_BUS, kDIALOG_WLED_ISET, 0xff },	// 50% value from OS
    { PMU_IIC_BUS, kDIALOG_WLED_ISET2, 0x7f },
    { PMU_IIC_BUS, kD2257_WLED_CTRL1,		// enable all LED strings
      (kDIALOG_WLED_CONTROL_WLED_ENABLE1 |
       kDIALOG_WLED_CONTROL_WLED_ENABLE2 |
       kDIALOG_WLED_CONTROL_WLED_ENABLE3 |
       kDIALOG_WLED_CONTROL_WLED_ENABLE4 |
       kDIALOG_WLED_CONTROL_WLED_ENABLE5 |
       kDIALOG_WLED_CONTROL_WLED_ENABLE6) },
    { PMU_IIC_BUS, kD2257_WLED_CTRL2,		// enable all LED strings
      (kDIALOG_WLED_CONTROL_WLED_ENABLE1 |
       kDIALOG_WLED_CONTROL_WLED_ENABLE2 |
       kDIALOG_WLED_CONTROL_WLED_ENABLE3 |
       kDIALOG_WLED_CONTROL_WLED_ENABLE4 |
       kDIALOG_WLED_CONTROL_WLED_ENABLE5 |
       kDIALOG_WLED_CONTROL_WLED_ENABLE6) },
    { PMU_IIC_BUS, kD2257_PWRONOFF_WLEDA_EN, 0x1f },	// enable LED Power
    { PMU_IIC_BUS, kD2257_PWRONOFF_WLEDB_EN, 0x1f },	// enable LED Power
};

static const struct pmu_setup_struct pmu_backlight_disable[] =
{
    { PMU_IIC_BUS, kD2257_PWRONOFF_WLEDA_EN, 0x0 },	// disable LED Power
    { PMU_IIC_BUS, kD2257_PWRONOFF_WLEDB_EN, 0x0 },	// disable LED Power
};

static const struct core_rails_struct soc_rails[] =
{
    { PMU_IIC_BUS, 0, kD2257_BUCK2_VSEL }
};

static const struct core_rails_struct cpu_rails[] =
{
    { PMU_IIC_BUS, 0, kD2257_BUCK0_VSEL }
};

// It's VDD FIXED! used for RAM inside the SOC
static const struct core_rails_struct ram_rails[] =
{
    { PMU_IIC_BUS, 0, kD2257_BUCK5_VSEL } 
};

// This is indexed with POWER_RAIL_*, I really just need to use the
// index in LDO[]
target_rails_t target_rails = {
    { 0x10, 450, 1250, 3125 }, // POWER_RAIL_CPU,         BUCK0
    { 0x15, 600, 1400, 3125 }, // POWER_RAIL_VDD_FIXED    BUCK5
    { 0x12, 600, 1400, 3125 }, // POWER_RAIL_SOC,         BUCK2

    { 0x17, 600, 1400, 3125 }, // POWER_RAIL_CPU_RAM,     BUCK7
    { 0x11, 450, 1250, 3125 }, // POWER_RAIL_GPU,         BUCK1
    { 0x18, 600, 1400, 3125 }, // POWER_RAIL_GPU_RAM,     BUCK8
};

#endif // POWERCONFIG_PMU_SETUP

#define NO_BATTERY_VOLTAGE		2700
#define MIN_BOOT_BATTERY_VOLTAGE	3600
#define TARGET_BOOT_BATTERY_VOLTAGE	3700
#define TARGET_BOOT_BATTERY_CAPACITY	50
#define PRECHARGE_BACKLIGHT_LEVEL	824
#define ALWAYS_BOOT_BATTERY_VOLTAGE	(TARGET_BOOT_BATTERY_VOLTAGE + 100)
#define TARGET_MAX_USB_INPUT_CURRENT	2400

#define TARGET_PRECHARGE_ALWAYS_DISPLAY_IDLE    1

#define ACC_PWR_LDO			(6)

#define GASGAUGE_BATTERYID_BLOCK	1

#endif
