#ifndef __TARGET_POWERCONFIG_H
#define __TARGET_POWERCONFIG_H

/* configuration per target */

#define PMU_IIC_BUS		1

/* this configuration is very PMU specific and must be included exactly once in power.c */
#ifdef POWERCONFIG_PMU_SETUP

static const struct pmu_setup_struct pmu_ldo_warm_setup[] =
{
};

static const struct pmu_setup_struct pmu_ldo_cold_setup[] =
{
};

static const struct pmu_setup_struct pmu_warm_init[] =
{
};

static const struct pmu_setup_struct pmu_cold_init[] =
{
  // <rdar://problem/20176777> j105: make sure spi control of buck1 is enabled
  { PMU_IIC_BUS, kDIALOG_TEST_ACCESS, kDIALOG_TEST_ACCESS_ENA },     // Enable Test Mode
  { PMU_IIC_BUS, kD2255_EN_CMD3, 0xfd}, // Enable SPI access for BUCK1
  { PMU_IIC_BUS, kDIALOG_TEST_ACCESS, kDIALOG_TEST_ACCESS_DIS },      // Disable Test Mode
  { PMU_IIC_BUS, kD2255_EN_CMD0, 0x02},  // enable control of Buck1 via spi (dwi)
  { PMU_IIC_BUS, kD2255_SPI_CTRL, 0x01}, // global dwi control enable

  // <rdar://problem/20161932> j105: update iBoot PMU overrides in IO spreadsheet
  { PMU_IIC_BUS, kD2255_OUT_32K, 0x49 },        // OUT_32K active, VBUCK3, PP
};

// PMU Power overview docs can be found at:
// <rdar://problem/18449381> J105: ERS Power Chapter Tracker

// For now we rely on the OTP settings being right and don't override these

static const struct core_rails_struct soc_rails[] =
{
};

static const struct core_rails_struct cpu_rails[] =
{
};

// it's VDD FIXED! used for RAM inside the SOC
static const struct core_rails_struct ram_rails[] =
{
};

// CLPC does power management using this table via the
// power_get_rail_value api
//
// 1st field is index into the ldo_params array in the pmu specific header file.
// 2nd field is low value in millivolts
// 3rd field is high value in millivoltes
// 4th is step size in microvolts
//
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

#define PRECHARGE_BACKLIGHT_LEVEL	824
#define ALWAYS_BOOT_BATTERY_VOLTAGE	(TARGET_BOOT_BATTERY_VOLTAGE + 100)


#define ACC_PWR_LDO			(6)

#endif
