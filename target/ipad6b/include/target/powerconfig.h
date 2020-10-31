#ifndef __TARGET_POWERCONFIG_H
#define __TARGET_POWERCONFIG_H

// Configuration per target

#define PMU_IIC_BUS		0
#define CHARGER_IIC_BUS		0


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

//  IO_CONFIG[7:6] 0=I 1=O_OD 2=O_PP 3=SS
//    IO_TYPE[5:3] O 0=L    1=32K  2=nR 3=TI 4=L
//       PUPD[2:1] O 0=NP 1=PU 2=NP     3=<0>
//   WAKE_LVL[0]   O 0=low 1=high
//    IO_TYPE[5:3] I  0=L_AH 1=L_AL 2=RE 3=FE 4=AE
//       PUPD[2:1] I  0=NP 1=PU 2=PD_GND 3=<0>
//   WAKE_LVL[0]   I  0=NW 1=W
//
// <rdar://problem/19493772> J127 GPIO Spreadsheet (master)
// J127_IO_spreadsheet_v26 (EVT).xlsx
static const struct pmu_setup_struct pmu_cold_init[] =
{
    // TODO :This is a Bringup Hack, remove
    { PMU_IIC_BUS, kD2257_BUCK0_VSEL, 0xc0 },       // CPU rail
    { PMU_IIC_BUS, kD2257_BUCK1_VSEL, 0xc0 },       // GPU rail
    { PMU_IIC_BUS, kD2257_PWRONOFF_BUCK1_EN, 0xf }, // GPU power rail (see <rdar://problem/19733370>)
    { PMU_IIC_BUS, kD2257_PWRONOFF_BUCK8_EN, 0xf }, // GPU RAM power rail (see <rdar://problem/19733370>)
    // TODO :This is a Bringup Hack, remove

    { PMU_IIC_BUS, kD2257_PWRONOFF_BUCK3_SW2_EN, 0xf},  // <rdar://20080883> J99A/J127: Keep DVDD power rail on to avoid speaker pops (work around HW issue) 
    { PMU_IIC_BUS, kD2257_PWRONOFF_LDO4_EN, 0x1 },  // Enable PP3V0_ALS
    { PMU_IIC_BUS, kD2257_PWRONOFF_LDO7_EN, 0x1 },  // Enable PP3V3_USB_SOC

    { PMU_IIC_BUS, kD2257_LDO2_BYPASS, 0x1}, // bypass when Active rdar://19773152

    // Drive GPIO_PMU_TO_1V1_EXT_SW_ON (PMU GPIO 20) to enable PP1V1_S1_EXT_SW
    // which inturn is SNSI_PP1V1_S1_DDR_VDDQ_x.
    //
    // Should be ON by default via OTP.
    
    // Drive GPIO_PMU_TO_1V8_EXT_SW2_ON (PMU GPIO 21) to enable PP1V8_S2_EXT_SW2
    // which inturn is PP1V8_S2_IO_EXPANDER.
    //
    // Should be ON by default via OTP.

    { PMU_IIC_BUS, kDIALOG_TEST_ACCESS, kDIALOG_TEST_ACCESS_ENA },      // Enable Test Mode
    { PMU_IIC_BUS, kD2257_EN_CMD3, 0},                                  // Enable SPI access for BUCKx (x=3)
    { PMU_IIC_BUS, kDIALOG_TEST_ACCESS, kDIALOG_TEST_ACCESS_DIS },      // Disable Test Mode

    // GPIO1: GPIO_AJ_HALL_TO_PMU_IRQ_L
    { PMU_IIC_BUS, kD2257_GPIO1_CONF1,  (0<<6)|(4<<3)|(1<<1)|(1<<0) },	// 0x23 Input, AnyE, PU, WakeUp
    { PMU_IIC_BUS, kD2257_GPIO1_CONF2,  (1<<3)|(1<<0) },		// 0x09 VBUCK3, 5ms

    // GPIO2: GPIO_ACC_TO_PMU_PWR_SW_OC_L
    { PMU_IIC_BUS, kD2257_GPIO2_CONF1,  (0<<6)|(3<<3)|(0<<1)|(0<<0) },	// 0x18 Input, AnyE, No PU/PD
    { PMU_IIC_BUS, kD2257_GPIO2_CONF2,  (1<<3)|(2<<0) },		// 0x0A VBUCK3, 10ms

    // GPIO3: GPIO_ORION_TO_SOC_TO_PMU_IRQ
    { PMU_IIC_BUS, kD2257_GPIO3_CONF1,  (0<<6)|(0<<3)|(2<<1)|(1<<0) },	// 0x25 Input, AH, PD, Wakeup
    { PMU_IIC_BUS, kD2257_GPIO3_CONF2,  (1<<3)|(1<<0) },		// 0x09 VBUCK3, 5ms

    // GPIO4: GPIO_PMU_TO_ACC_PWR_SW (was GPIO_PMU_TO_ACC_PWR_SEL)
    { PMU_IIC_BUS, kD2257_GPIO4_CONF1,  (2<<6)|(0<<3)|(0<<1)|(0<<0) },	// 0x80 Output(Push-Pull), Level, No PU, Low
    { PMU_IIC_BUS, kD2257_GPIO4_CONF2,  (0<<3)|(0<<0) },		// 0x00 VCC_MAIN, no DEB

    // GPIO6: UART_BATT_HDQ
    { PMU_IIC_BUS, kD2257_GPIO6_CONF1,  (0<<6)|(3<<3)|(0<<1)|(1<<0) },	// 0x19 Input, FE, No PU/PD
    { PMU_IIC_BUS, kD2257_GPIO6_CONF2,  (1<<3)|(2<<0) },		// 0x0A VBUCK3, 10ms

    // GPIO7: GPIO_BT_TO_PMU_HOST_WAKE
    { PMU_IIC_BUS, kD2257_GPIO7_CONF1,  (0<<6)|(2<<3)|(0<<1)|(1<<0) },	// 0x11 Input, RE, No PU/PD, Wakeup
    { PMU_IIC_BUS, kD2257_GPIO7_CONF2,  (1<<3)|(1<<0) },		// 0x09 VBUCK3, 5ms

    // GPIO8: GPIO_WLAN_TO_PMU_HOST_WAKE
    { PMU_IIC_BUS, kD2257_GPIO8_CONF1,  (0<<6)|(2<<3)|(2<<1)|(1<<0) },	// 0x15 Input, RE, PD, Wakeup
    { PMU_IIC_BUS, kD2257_GPIO8_CONF2,  (1<<3)|(1<<0) },		// 0x09 VBUCK3, 5ms

    // GPIO10: GPIO_CODEC_TO_PMU_WAKE_L
    { PMU_IIC_BUS, kD2257_GPIO10_CONF1, (0<<6)|(4<<3)|(1<<1)|(1<<0) },	// 0x23 Input, AnyE, PU, Wakeup
    { PMU_IIC_BUS, kD2257_GPIO10_CONF2, (1<<3)|(2<<0) },		// 0x0A VBUCK3, 10ms

    // GPIO11: GPIO_STB_HALL_TO_PMU_IRQ_L
    { PMU_IIC_BUS, kD2257_GPIO11_CONF1, (0<<6)|(4<<3)|(1<<1)|(1<<0) },	// 0x23 Input, AnyE, PU, WakeUp
    { PMU_IIC_BUS, kD2257_GPIO11_CONF2, (1<<3)|(1<<0) },		// 0x09 VBUCK3, 5ms

    // GPIO12: GPIO_TS_TO_SOC_TO_PMU_IRQ
    { PMU_IIC_BUS, kD2257_GPIO12_CONF1, (0<<6)|(0<<3)|(0<<1)|(1<<0) },	// 0x01 Input, AH, No PU/PD, Wakeup
    { PMU_IIC_BUS, kD2257_GPIO12_CONF2, (1<<3)|(1<<0) },		// 0x09 VBUCK3, 5ms

    // GPIO13: GPIO_BELFIELD_TO_PMU_WAKE_L (was GPIO_IO_EXP_TO_PMU_IRQ)
    { PMU_IIC_BUS, kD2257_GPIO13_CONF1, (0<<6)|(4<<3)|(1<<1)|(1<<0) },	// 0x23 Input, AnyE, PU, WakeUp
    { PMU_IIC_BUS, kD2257_GPIO13_CONF2, (1<<3)|(2<<0) },		// 0x0A VBUCK3, 10ms

    // GPIO14: GPIO_EUPHRATES_TO_PMU_WAKE (was PCIE_WLAN_TO_PMU_WAKE_L)
    { PMU_IIC_BUS, kD2257_GPIO14_CONF1, (0<<6)|(0<<3)|(0<<1)|(1<<0) },	// 0x01 Input, AH, No PU/PD, WakeUp
    { PMU_IIC_BUS, kD2257_GPIO14_CONF2, (3<<3)|(1<<0) },		// 0x19 PP1V8_ALWAYS, 5ms

    // GPIO15: GPIO_EUPHRATES_TO_SOC_TO_PMU_IRQ_L (was GPIO_PMU_TO_CODEC_RESET_L)
    { PMU_IIC_BUS, kD2257_GPIO15_CONF1, (0<<6)|(3<<3)|(1<<1)|(1<<0) },	// 0x1B Input, FE, PU, WakeUp
    { PMU_IIC_BUS, kD2257_GPIO15_CONF2, (3<<3)|(2<<0) },		// 0x1A PP1V8_ALWAYS, 10ms

    // GPIO16: GPIO_PMU_TO_NAND_LOW_BATT_L
    // kD2257_GPIO16_CONF1 gets set in target_late_init()
    { PMU_IIC_BUS, kD2257_GPIO16_CONF2, (1<<3)|(0<<0) },		// 0x08 VBUCK3, no DEB

    // GPIO18: Enable GPIO_PMU_TO_LCD_PWREN 
    { PMU_IIC_BUS, kD2257_GPIO18_CONF1, (2<<6)|(0<<3)|(0<<1)|(0<<0) },	// 0x80 Output(Push-Pull), Level, No PU/PD, Low
    { PMU_IIC_BUS, kD2257_GPIO18_CONF2, (1<<3)|(0<<0) },		// 0x08 VBUCK3, no DEB

    // GPIO9: GPIO_BB_TO_PMU_HOST_WAKE_L
    { PMU_IIC_BUS, kD2257_GPIO9_CONF1,  (0<<6)|(1<<3)|(2<<1)|(0<<0) },	// 0x0C Input, AL, PD
    { PMU_IIC_BUS, kD2257_GPIO9_CONF2,  (1<<3)|(1<<0) },		// 0x09 VBUCK3, 5ms

#if TARGET_HAS_BASEBAND
    // GPIO5: GPIO_PMU_TO_BBPMU_RESET_R_L (rdar://20064772, rdar://20110706, rdar://21211826)
    { PMU_IIC_BUS, kD2257_GPIO5_CONF1,  (2<<6)|(0<<3)|(0<<1)|(0<<0) },	// 0x80 Output(Push-Pull), Level, no PU/PD, Low
    { PMU_IIC_BUS, kD2257_GPIO5_CONF2,  (1<<3)|(0<<0) },		// 0x08 VBUCK3, no DEB

    // GPIO17: GPIO_PMU_TO_BB_VBUS_DET
    { PMU_IIC_BUS, kD2257_GPIO17_CONF1, (2<<6)|(0<<3)|(0<<1)|(1<<0) },	// 0x81 Output(Push-Pull), Level, No PU/PD, High
    { PMU_IIC_BUS, kD2257_GPIO17_CONF2, (0<<3)|(0<<0) },		// 0x00 VCC_MAIN, no DEB
#else
    // GPIO5: GPIO_PMU_TO_BBPMU_RESET_R_L 
    { PMU_IIC_BUS, kD2257_GPIO5_CONF1,  (0<<6)|(0<<3)|(2<<1)|(0<<0) },	// 0x40 Input, AH, PD
    { PMU_IIC_BUS, kD2257_GPIO5_CONF2,  (1<<3)|(0<<0) },		// 0x08 VBUCK3, no DEB

    // GPIO17: GPIO_PMU_TO_BB_VBUS_DET
    { PMU_IIC_BUS, kD2257_GPIO17_CONF1,  (0<<6)|(0<<3)|(2<<1)|(0<<0) },	// 0x40 Input, AH, PD
    { PMU_IIC_BUS, kD2257_GPIO17_CONF2, (1<<3)|(0<<0) },		// 0x08 VBUCK3, no DEB
#endif

    // GPIO19: GPIO_PMU_TO_EUPHRATES_LINCH_EN (was GPIO_GANGES_TO_PMU_WAKE)
    { PMU_IIC_BUS, kD2257_GPIO19_CONF1_SLP1,(1<<6)|(0<<3)|(0<<1)|(1<<0) },	// 0x41 Output(Open Drain), Level, No PU/PD, High
    { PMU_IIC_BUS, kD2257_GPIO19_CONF1_SLP2,(1<<6)|(0<<3)|(0<<1)|(1<<0) },	// 0x41 Output(Open Drain), Level, No PU/PD, High
    { PMU_IIC_BUS, kD2257_GPIO19_CONF1_SLP3,(1<<6)|(0<<3)|(0<<1)|(1<<0) },	// 0x41 Output(Open Drain), Level, No PU/PD, High
    { PMU_IIC_BUS, kD2257_GPIO19_CONF1_OFF, (1<<6)|(0<<3)|(0<<1)|(0<<0) },	// 0x40 Output(Open Drain), Level, No PU/PD, Low
    { PMU_IIC_BUS, kD2257_GPIO19_CONF1,     (1<<6)|(0<<3)|(0<<1)|(1<<0) },	// 0x41 Output(Open Drain), Level, No PU/PD, High
    { PMU_IIC_BUS, kD2257_GPIO19_CONF2,	    (1<<3)|(0<<0) },			// 0x08 VBUCK3, no DEB

    // GPIO20: GPIO_PMU_TO_1V1_EXT_SW_ON
    { PMU_IIC_BUS, kD2257_GPIO20_CONF1_SLP1,(2<<6)|(0<<3)|(0<<1)|(1<<0) },	// 0x81 Output(Push-Pull), Level, No PU/PD, High
    { PMU_IIC_BUS, kD2257_GPIO20_CONF1_SLP2,(2<<6)|(0<<3)|(0<<1)|(0<<0) },	// 0x80 Output(Push-Pull), Level, No PU/PD, Low
    { PMU_IIC_BUS, kD2257_GPIO20_CONF1_SLP3,(2<<6)|(0<<3)|(0<<1)|(0<<0) },	// 0x80 Output(Push-Pull), Level, No PU/PD, Low
    { PMU_IIC_BUS, kD2257_GPIO20_CONF1_OFF, (2<<6)|(0<<3)|(0<<1)|(0<<0) },	// 0x80 Output(Push-Pull), Level, No PU/PD, Low
    { PMU_IIC_BUS, kD2257_GPIO20_CONF1,     (2<<6)|(0<<3)|(0<<1)|(1<<0) },	// 0x81 Output(Push-Pull), Level, No PU/PD, High
    { PMU_IIC_BUS, kD2257_GPIO20_CONF2,	    (1<<3)|(0<<0) },			// 0x08 VBUCK3, no DEB

    // GPIO21: GPIO_PMU_TO_1V8_EXT_SW2_ON
    { PMU_IIC_BUS, kD2257_GPIO21_CONF1_SLP1,(2<<6)|(0<<3)|(0<<1)|(1<<0) },	// 0x81 Output(Push-Pull), Level, No PU/PD, Low
    { PMU_IIC_BUS, kD2257_GPIO21_CONF1_SLP2,(2<<6)|(0<<3)|(0<<1)|(1<<0) },	// 0x81 Output(Push-Pull), Level, No PU/PD, Low
    { PMU_IIC_BUS, kD2257_GPIO21_CONF1_SLP3,(2<<6)|(0<<3)|(0<<1)|(0<<0) },	// 0x80 Output(Push-Pull), Level, No PU/PD, Low
    { PMU_IIC_BUS, kD2257_GPIO21_CONF1_OFF, (2<<6)|(0<<3)|(0<<1)|(0<<0) },	// 0x80 Output(Push-Pull), Level, No PU/PD, Low
    { PMU_IIC_BUS, kD2257_GPIO21_CONF1,     (2<<6)|(0<<3)|(0<<1)|(1<<0) },	// 0x81 Output(Push-Pull), Level, No PU/PD, High
    { PMU_IIC_BUS, kD2257_GPIO21_CONF2,	    (1<<3)|(0<<0) },			// 0x08 VBUCK3, no DEB

    // GPIO22: GPIO_PMU_TO_3V3_EXT_SW_ON (was GPIO_GANGES_TO_SOC_TO_PMU_IRQ_L)
    { PMU_IIC_BUS, kD2257_GPIO22_CONF1_SLP1,(2<<6)|(0<<3)|(0<<1)|(0<<0) },	// 0x80 Output(Push-Pull), Level, No PU/PD, Low
    { PMU_IIC_BUS, kD2257_GPIO22_CONF1_SLP2,(2<<6)|(0<<3)|(0<<1)|(0<<0) },	// 0x80 Output(Push-Pull), Level, No PU/PD, Low
    { PMU_IIC_BUS, kD2257_GPIO22_CONF1_SLP3,(2<<6)|(0<<3)|(0<<1)|(0<<0) },	// 0x80 Output(Push-Pull), Level, No PU/PD, Low
    { PMU_IIC_BUS, kD2257_GPIO22_CONF1_OFF, (2<<6)|(0<<3)|(0<<1)|(0<<0) },	// 0x80 Output(Push-Pull), Level, No PU/PD, Low
    { PMU_IIC_BUS, kD2257_GPIO22_CONF1,     (2<<6)|(0<<3)|(0<<1)|(1<<0) },	// 0x81 Output(Push-Pull), Level, No PU/PD, High
    { PMU_IIC_BUS, kD2257_GPIO22_CONF2,     (1<<3)|(0<<0) },			// 0x08 VBUCK3, no DEB
};

static const struct pmu_setup_struct pmu_backlight_enable[] =
{
    { PMU_IIC_BUS, kDIALOG_WLED_ISET,  0x06 },  // 75% of max 0x7FF
    { PMU_IIC_BUS, kDIALOG_WLED_ISET2, 0x00 },
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
    { PMU_IIC_BUS, kD2257_PWRONOFF_WLEDA_EN, 0x1 },	// enable LED Power
    { PMU_IIC_BUS, kD2257_PWRONOFF_WLEDB_EN, 0x1 },	// enable LED Power
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
