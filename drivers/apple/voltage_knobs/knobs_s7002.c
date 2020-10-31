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
#include <platform/soc/pmgr.h>
#include <drivers/power.h>
#include <drivers/iic.h>
#include <lib/env.h>
#include <target/powerconfig.h>
#include "knobs_s7002.h"

// this code should be included and debug and development builds only
#if !RELEASE_BUILD

#include <drivers/voltage_knobs/knobs.h>

// **** PMU platform-specific code ****

// For M7 SoC with D2238(Tanzanite) PMU <rdar://problem/14756079>
//   * 0.6V + 3.125mV * BUCK0_VSEL, so BUCK0_MIN = 0.6V, BUCK0_MAX = 1.396V
//   * 0.6V + 3.125mV * BUCK2_VSEL, so BUCK2_MIN = 0.6V, BUCK2_MAX = 1.396V
//   * 1.1V + 3.125mV * BUCK3_VSEL, so BUCK3_MIN = 1.1V, BUCK3_MAX = 1.896V
#define KNOBS_BUCKn_MV_PER_UNIT   3125

#define KNOBS_BUCK3_VMIN (1100)
#define KNOBS_BUCK3_VMAX (1896)
#define KNOBS_BUCK3_BUCKOUT(mv) (((mv - KNOBS_BUCK3_VMIN) * 1000) + (KNOBS_BUCKn_MV_PER_UNIT - 1)) / KNOBS_BUCKn_MV_PER_UNIT
#define KNOBS_BUCK3_BUCKMV(vsel) (KNOBS_BUCK3_VMIN + ((((vsel) * KNOBS_BUCKn_MV_PER_UNIT)) / 1000))

#define kDIALOG_BUCK0_VSEL  0x0100
#define kDIALOG_BUCK2_VSEL  0x0140
#define kDIALOG_BUCK3_VSEL  0x0160

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
            pmu_set_data(PMU_IIC_BUS, kDIALOG_BUCK2_VSEL, value, false);
            break;
        case KNOB_PMU_SOC_FIXED:
            pmu_set_data(PMU_IIC_BUS, kDIALOG_BUCK0_VSEL, value, false);
            break;
    }
}

// convert units to millivolts for specific knob
int knobs_convert_dwi_to_mv(knob_types_e type, uint8_t index, u_int32_t dwival)
{
    int buck_type = 0;
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
    int buck_type = 0;
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

// Request default value for a knob in units.
bool knobs_get_default_val(knob_types_e type, uint8_t index, uint8_t *defaultValueUnits)
{
    if (defaultValueUnits == NULL || type >= kKnobTypeInvalid || index >= all_knobs[type].max_index)
    {
        // Invalid parameter.
        return false;
    }

    switch (type)
    {
        case kKnobTypePMU:
            switch(index)
            {
                case KNOB_PMU_DRAM_1V8:
                    *defaultValueUnits = KNOB_PMU_BUCK3_POR;
                    break;
                case KNOB_PMU_DRAM_1V2:
                    *defaultValueUnits = KNOB_PMU_BUCK2_POR;
                    break;
                case KNOB_PMU_SOC_FIXED:
                {
                    uint32_t dwiVal = 0;
                    knobs_convert_mv_to_dwi(type, index, platform_get_base_ram_voltage(), &dwiVal);
                    *defaultValueUnits = dwiVal;
                    break;
                }
                default:
                    return false;
            }
            break;

        default:
            return false;
    }

    return true;
}

#endif
