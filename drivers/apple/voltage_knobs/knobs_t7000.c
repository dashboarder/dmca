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

// for some reasons these are not defined for t7000 platform
#ifndef CCC_DVFM_ST_SAFE_VOL_MASK
#define CCC_DVFM_ST_SAFE_VOL_MASK (0xFF00000000000000ull)
#endif

#ifndef CCC_DVFM_ST_SAFE_VOL_ADJ_MASK
#define CCC_DVFM_ST_SAFE_VOL_ADJ_MASK (0xFF3FFFFFC0000000ull)
#endif

#ifndef CCC_DVFM_ST_SAFE_VOL_SHIFT
#define CCC_DVFM_ST_SAFE_VOL_SHIFT (56)
#endif

#ifndef PMGR_GFX_PERF_STATE_VOLTAGE_SHIFT
#define PMGR_GFX_PERF_STATE_VOLTAGE_SHIFT  (24)
#endif

#if (GPU_KNOB_MAX > 0)
uint8_t knobs_platform_get_gfx_perf_voltage(uint32_t perfStateNo)
{
    uint32_t gpu_perf_state = rPMGR_GFX_PERF_STATE_ENTRY(perfStateNo);
    return (gpu_perf_state >> PMGR_GFX_PERF_STATE_VOLTAGE_SHIFT);
}

void knobs_platform_set_gfx_perf_voltage(uint32_t perfStateNo, uint8_t newVoltage)
{
    uint32_t gpu_perf_state = rPMGR_GFX_PERF_STATE_ENTRY(perfStateNo);
    gpu_perf_state &= 0x00FFFFFF;
    rPMGR_GFX_PERF_STATE_ENTRY(perfStateNo) = gpu_perf_state | (newVoltage << PMGR_GFX_PERF_STATE_VOLTAGE_SHIFT);
}
#endif

#if (CPU_KNOB_MAX > 0)
uint8_t knobs_platform_get_cpu_perf_voltage(uint32_t perfStateNo)
{
    uint64_t dvfm_state = rCCC_DVFM_ST(perfStateNo);
    return ((dvfm_state & CCC_DVFM_ST_SAFE_VOL_MASK) >> CCC_DVFM_ST_SAFE_VOL_SHIFT);
}

void knobs_platform_set_cpu_perf_voltage(uint32_t perfStateNo, uint8_t newVoltage)
{
    uint64_t dvfm_state = rCCC_DVFM_ST(perfStateNo);

    // Clean safe_vol and volAdj[0-3] as we want to disable temperature adjustment
    // when voltage is explicitly specified.
    dvfm_state &= ~CCC_DVFM_ST_SAFE_VOL_ADJ_MASK;

    // clean safe voltage out of state register
    dvfm_state &= ~CCC_DVFM_ST_SAFE_VOL_MASK;

    // Update selected CPU power state with new voltage.
    rCCC_DVFM_ST(perfStateNo) = dvfm_state | CCC_DVFM_ST_SAFEVOL(newVoltage);
}
#endif

#if (SOC_KNOB_MAX > 0)
uint8_t knobs_platform_get_soc_perf_voltage(uint32_t perfStateNo)
{
    return rPMGR_SOC_PERF_STATE_ENTRY_C(perfStateNo);
}

void knobs_platform_set_soc_perf_voltage(uint32_t perfStateNo, uint8_t newVoltage)
{
    rPMGR_SOC_PERF_STATE_ENTRY_C(perfStateNo) = newVoltage;
}
#endif

// **** PMU platform-specific code ****

// For FIJI SoC with D2186 PMU
//   * 1.1V + 3.125mV * BUCK3_VSEL, so BUCK3_MIN = 1.1V, BUCK3_MAX = 1.896V
//   * 0.6V + 3.125mV * BUCK4_VSEL, so BUCK4_MIN = 0.6V, BUCK4_MAX = 1.396V
//   * 0.6V + 3.125mV * BUCK5_VSEL, so BUCK5_MIN = 0.6V, BUCK5_MAX = 1.396V
#define KNOBS_BUCKn_MV_PER_UNIT   3125

#define KNOBS_BUCK3_VMIN (1100)
#define KNOBS_BUCK3_VMAX (1896)
#define KNOBS_BUCK3_BUCKOUT(mv) (((mv - KNOBS_BUCK3_VMIN) * 1000) + (KNOBS_BUCKn_MV_PER_UNIT - 1)) / KNOBS_BUCKn_MV_PER_UNIT
#define KNOBS_BUCK3_BUCKMV(vsel) (KNOBS_BUCK3_VMIN + ((((vsel) * KNOBS_BUCKn_MV_PER_UNIT)) / 1000))

#define kDIALOG_BUCK3_VSEL  0x0160
#define kDIALOG_BUCK4_VSEL  0x0180
#define kDIALOG_BUCK5_VSEL  0x01a0

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

// convert units to millivolts for specific knob
int knobs_convert_dwi_to_mv(knob_types_e type, uint8_t index, u_int32_t dwival)
{
#if BUCK_CPU && BUCK_GPU
    int buck_type = (type == kKnobTypeGPU) ? BUCK_GPU : BUCK_CPU;
#else
    int buck_type = 0;
#endif
    switch(type)
    {
        case kKnobTypePMU:
            switch(index)
            {
                case KNOB_PMU_DRAM_1V8:
                    return KNOBS_BUCK3_BUCKMV(dwival);
                default:
                    return power_convert_dwi_to_mv(buck_type, dwival);
            }
        default:
            return power_convert_dwi_to_mv(buck_type, dwival);
    }
}

// converts millivolts to units for specific knob
int knobs_convert_mv_to_dwi(knob_types_e type, uint8_t index, u_int32_t mv, u_int32_t *dwival)
{
#if BUCK_CPU && BUCK_GPU
    int buck_type = (type == kKnobTypeGPU) ? BUCK_GPU : BUCK_CPU;
#else
    int buck_type = 0;
#endif
    switch(type)
    {
        case kKnobTypePMU:
            switch(index)
            {
                case KNOB_PMU_DRAM_1V8:
                    *dwival = KNOBS_BUCK3_BUCKOUT(mv);
                    return 0;
                default:
                    return power_get_buck_value(buck_type, mv, dwival);
            }
        default:
            return power_get_buck_value(buck_type, mv, dwival);
    }
}


#endif
