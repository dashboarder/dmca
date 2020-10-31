/*
 * Copyright (c) 2014 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */

#include <drivers/power.h>
#include <AssertMacros.h>
#include <platform.h>
#include <platform/chipid.h>
#include <platform/soc/pmgr.h>
#include <drivers/power.h>
#include <drivers/iic.h>
#include <lib/env.h>
#include <lib/nvram.h>
#include <target/powerconfig.h>

// this code should be included and debug and development builds only
#if !RELEASE_BUILD

#include <drivers/voltage_knobs/knobs.h>

uint8_t knobs_platform_get_gfx_perf_voltage(uint32_t perfStateNo)
{
    uint32_t gpu_perf_state_a = rPMGR_GFX_PERF_STATE_ENTRY_A(perfStateNo);
    return PMGR_GFX_PERF_STATE_ENTRY0A_VOLTAGE_XTRCT(gpu_perf_state_a);
}

void knobs_platform_set_gfx_perf_voltage(uint32_t perfStateNo, uint8_t newVoltage)
{
    uint32_t gpu_perf_state_a = rPMGR_GFX_PERF_STATE_ENTRY_A(perfStateNo);
    gpu_perf_state_a &= ~PMGR_GFX_PERF_STATE_ENTRY0A_VOLTAGE_UMASK;
    rPMGR_GFX_PERF_STATE_ENTRY_A(perfStateNo) = gpu_perf_state_a | PMGR_GFX_PERF_STATE_ENTRY0A_VOLTAGE_INSRT(newVoltage);
}


uint8_t knobs_platform_get_cpu_perf_voltage(uint32_t perfStateNo)
{
    uint64_t dvfm_state = rACC_DVFM_ST(perfStateNo);
    return ACC_PWRCTL_DVFM_ST0_SAFE_VOL_XTRCT(dvfm_state);
}

void knobs_platform_set_cpu_perf_voltage(uint32_t perfStateNo, uint8_t newVoltage)
{
    uint64_t dvfm_state = rACC_DVFM_ST(perfStateNo);

    // clean volAdjX as we want to disable temperature adjustments when voltage is explicitly specified
    dvfm_state &= ~ACC_PWRCTL_DVFM_ST0_VOL_ADJ0_UMASK;
    dvfm_state &= ~ACC_PWRCTL_DVFM_ST0_VOL_ADJ1_UMASK;
    dvfm_state &= ~ACC_PWRCTL_DVFM_ST0_VOL_ADJ2_UMASK;
    dvfm_state &= ~ACC_PWRCTL_DVFM_ST0_VOL_ADJ3_UMASK;

    // Clean safe voltage as well.
    dvfm_state &= ~ACC_PWRCTL_DVFM_ST0_SAFE_VOL_UMASK;

    // Update selected CPU power state with new voltage.
    rACC_DVFM_ST(perfStateNo) = dvfm_state | ACC_DVFM_ST_SAFE_VOL(newVoltage);
}

uint8_t knobs_platform_get_soc_perf_voltage(uint32_t perfStateNo)
{
    return PMGR_SOC_PERF_STATE_ENTRY_0C_VOLTAGE_XTRCT(rPMGR_SOC_PERF_STATE_ENTRY_C(perfStateNo));
}

void knobs_platform_set_soc_perf_voltage(uint32_t perfStateNo, uint8_t newVoltage)
{
    uint32_t perf_state_entry_c = rPMGR_SOC_PERF_STATE_ENTRY_C(perfStateNo);
    perf_state_entry_c &= ~PMGR_SOC_PERF_STATE_ENTRY_0C_VOLTAGE_UMASK;
    rPMGR_SOC_PERF_STATE_ENTRY_C(perfStateNo) = perf_state_entry_c | PMGR_SOC_PERF_STATE_ENTRY_0C_VOLTAGE_INSRT(newVoltage);
}


// For Maui SoC with D2255 (Antigua) PMU
//   * 1.1V + 3.125mV * BUCK3_VSEL, so BUCK3_MIN = 1.1V, BUCK3_MAX = 1.896V
//   * 0.6V + 3.125mV * BUCK4_VSEL, so BUCK4_MIN = 0.6V, BUCK4_MAX = 1.396V
//   * 0.6V + 3.125mV * BUCK5_VSEL, so BUCK5_MIN = 0.6V, BUCK5_MAX = 1.396V


// KNOB_PMU_DRAM_1V8    >> BUCK3 (1.8V)
// KNOB_PMU_DRAM_1V2    >> BUCK4 (1.1V)
// KNOB_PMU_SOC_FIXED   >> BUCK5 (0.85V)

#define kDIALOG_BUCK3_VSEL  0x1301
#define kDIALOG_BUCK4_VSEL  0x1401
#define kDIALOG_BUCK5_VSEL  0x1501

#define KNOBS_BUCKn_MV_PER_UNIT   3125

#define KNOBS_BUCK3_VMIN (1100)
#define KNOBS_BUCK3_VMAX (1896)
#define KNOBS_BUCK3_BUCKOUT(mv) (((mv - KNOBS_BUCK3_VMIN) * 1000) + (KNOBS_BUCKn_MV_PER_UNIT - 1)) / KNOBS_BUCKn_MV_PER_UNIT
#define KNOBS_BUCK3_BUCKMV(vsel) (KNOBS_BUCK3_VMIN + ((((vsel) * KNOBS_BUCKn_MV_PER_UNIT)) / 1000))

// defined in drivers/dialog/pmu/power.c
int pmu_set_data(int dev, uint16_t reg, uint8_t byte, bool do_confirm);

void knobs_pmu_set_data(uint8_t index, uint8_t value)
{
    switch(index)
    {
        case KNOB_PMU_DRAM_1V8:
            pmu_set_data(PMU_IIC_BUS, kDIALOG_BUCK3_VSEL, value, false);
            break;
        case KNOB_PMU_DRAM_1V2:
            pmu_set_data(PMU_IIC_BUS, kDIALOG_BUCK4_VSEL, value, false);
            break;
        case KNOB_PMU_SOC_FIXED:
            pmu_set_data(PMU_IIC_BUS, kDIALOG_BUCK5_VSEL, value, false);
            break;
    }
}

// Power rail value conversion is based on target_rails[] array in
// target/iphone8/include/target/powerconfig.h
// However, it does not have entries for BUCK3 and BUCK4 yet.
// Therefore, I need to have my own macro for BUCK3.
// BUCK4 is similar to BUCK2, so reuse its value for now.

// Convert units to millivolts for specific knob.
int knobs_convert_dwi_to_mv(knob_types_e type, uint8_t index, u_int32_t dwival)
{
    switch(type)
    {
        case kKnobTypeCPU:
            return power_convert_dwi_to_mv(POWER_RAIL_CPU, dwival);
        case kKnobTypeGPU:
            return power_convert_dwi_to_mv(POWER_RAIL_GPU, dwival);
        case kKnobTypeSOC:
            return power_convert_dwi_to_mv(POWER_RAIL_SOC, dwival);
        case kKnobTypePMU:
            switch(index)
            {
                case KNOB_PMU_DRAM_1V8:
                    return KNOBS_BUCK3_BUCKMV(dwival);
                case KNOB_PMU_SOC_FIXED:
                    return power_convert_dwi_to_mv(POWER_RAIL_VDD_FIXED, dwival);
                case KNOB_PMU_DRAM_1V2:
                    return power_convert_dwi_to_mv(POWER_RAIL_SOC, dwival);
                default:
                    printf("%s: unknown PMU knob index %d\n", __FUNCTION__, index);
                    return -1;
            }
        default:
            printf("%s: unknown knob type %d\n", __FUNCTION__, type);
            return -1;
    }
}

// Converts millivolts to units for specific knob.
int knobs_convert_mv_to_dwi(knob_types_e type, uint8_t index, u_int32_t mv, u_int32_t *dwival)
{
    switch(type)
    {
        case kKnobTypeCPU:
            return power_get_rail_value(POWER_RAIL_CPU, mv, dwival);
        case kKnobTypeGPU:
            return power_get_rail_value(POWER_RAIL_GPU, mv, dwival);
        case kKnobTypeSOC:
            return power_get_rail_value(POWER_RAIL_SOC, mv, dwival);
        case kKnobTypePMU:
            switch(index)
            {
                case KNOB_PMU_DRAM_1V8:
                    *dwival = KNOBS_BUCK3_BUCKOUT(mv);
                return 0;
                case KNOB_PMU_SOC_FIXED:
                    return power_get_rail_value(POWER_RAIL_VDD_FIXED, mv, dwival);
                case KNOB_PMU_DRAM_1V2:
                    return power_get_rail_value(POWER_RAIL_SOC, mv, dwival);
                default:
                    printf("%s: unknown PMU knob index %d\n", __FUNCTION__, index);
                    return -1;
            }
        default:
            printf("%s: unknown knob type %d\n", __FUNCTION__, type);
            return -1;
    }
}

#endif
