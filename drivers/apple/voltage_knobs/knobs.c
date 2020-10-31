/*
 * Copyright (c) 2013 Apple Inc. All rights reserved.
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

#if SUB_PLATFORM_T7000 || SUB_PLATFORM_T7001
#include    "knobs_t7000.h"
#elif  SUB_PLATFORM_S7002
#include    "knobs_s7002.h"
#elif SUB_PLATFORM_S8000 || SUB_PLATFORM_S8001 || SUB_PLATFORM_S8003
#include    "knobs_s8000.h"
#else
#error      This platform is not supported
#endif

// a number of bytes used in PMU scratch RAM
#define VOLTAGE_KNOBS_SIZE      32

// A "master switch" bit position in the PMU bitfield. 0 in this bit means feature not activated
#define MASTER_SWITCH_BIT       (VOLTAGE_KNOBS_SIZE * 8 - 1)

// do not reorder the knobs in list as this array is used for iteration through all knobs,
// and the code below assumes the knobs order
const knob_type_description_t all_knobs[] =
{
    {NULL , ' ', kKnobTypeUnknown, -1},
    {"cpu", 'C', kKnobTypeCPU, CPU_KNOB_MAX},
    {"gpu", 'G', kKnobTypeGPU, GPU_KNOB_MAX},
    {"soc", 'S', kKnobTypeSOC, SOC_KNOB_MAX},
    {"pmu", 'P', kKnobTypePMU, PMU_KNOB_MAX},
    {NULL , ' ', kKnobTypeInvalid, -1},
};

// -========================================================================================-
// - NV Storage operations -
//   It could be done using standalone persistent environment variables or being a part of
//   boot-args.
//     Value formatting:
//        * Offset (-)[intValue] - just plain decimal number in units, may be negative
//        * Absolute @[intValue] - symbol '@'+ plain decimal number in units. Always positive

#define BOOT_ARGS_NAME	"boot-args"

// Utility function to store a knob value by its name into a non-volatile storage
bool knobs_store_knob_in_NVRAM(const char* knobName, knob_as_binary_t *knobValue)
{
    bool result = false;
    const char* existingBootArgs = NULL;
    char knobValStr[KNOB_NAME_MAXLEN + 20] = {0};

    // remove any existing one, ignore errors
    knobs_remove_knob_from_NVRAM(knobName);

    // fetch up existing set of boot-args
    existingBootArgs = env_get(BOOT_ARGS_NAME);
    if (existingBootArgs)
    {
        size_t newBootArgsLen = 0;
        char *newBootArgs = NULL;

        // format name=value pair to append to boot-args, it should contain a leading space
        snprintf(knobValStr, sizeof(knobValStr), " %s=%s%d",
                 knobName,
                 knobValue->u.bit.isAbsolute ? KNOB_ABSOLUTE_CHAR : "",
                 knobValue->u.bit.isNegative ? -knobValue->u.bit.knobVal : knobValue->u.bit.knobVal);
        // allocate buffer for both strings and a trailing zero
        newBootArgsLen = strlen(existingBootArgs) + strlen(knobValStr) + 1;
        newBootArgs = malloc(newBootArgsLen);
        if (newBootArgs)
        {
            strlcpy(newBootArgs, existingBootArgs, newBootArgsLen);
            strlcat(newBootArgs, knobValStr, newBootArgsLen);

            // store updated boot-args
            env_set(BOOT_ARGS_NAME, newBootArgs, ENV_PERSISTENT);
            if (nvram_save() == 0)
            {
                result = true;
            }
            free(newBootArgs);
        }
    }

    return result;
}

// Remove knob from NV storage
bool knobs_remove_knob_from_NVRAM(const char* knobName)
{
    bool result = false;
    const char* existingBootArgs = NULL;

    // fetch up existing set of boot-args
    existingBootArgs = env_get(BOOT_ARGS_NAME);
    if (existingBootArgs)
    {
        const char *findNamePtr = strstr(existingBootArgs, knobName);
        if (findNamePtr)
        {
            // found one, removing
            char *newBootArgs = strdup(existingBootArgs);
            if (newBootArgs)
            {
                // break-up the string by injecting a zero at the beginning of the value to remove
                newBootArgs[findNamePtr - existingBootArgs] = 0;
                // find next space and append the rest of boot-args string to the end
                char *nextSpcPtr = strstr(findNamePtr, " ");
                if (nextSpcPtr)
                {
                    // skip the whitespace just found as it belongs to the key/value pair we removing
                    nextSpcPtr ++;
                    if (strlen(newBootArgs) == 0)
                    {
                        // removed one was the first in string, so remove any spaces may be
                        while (*nextSpcPtr == ' ') nextSpcPtr ++;
                    }
                    strlcat(newBootArgs, nextSpcPtr, strlen(existingBootArgs) + 1);
                }

                // clean up any trailing spaces may left
                nextSpcPtr = &newBootArgs[strlen(newBootArgs)];
                while ((nextSpcPtr >= newBootArgs) && (*nextSpcPtr == ' ' || *nextSpcPtr == 0))
                {
                    *nextSpcPtr = 0;
                    nextSpcPtr --;
                }

                // store updated boot-args
                env_set(BOOT_ARGS_NAME, newBootArgs, ENV_PERSISTENT);
                if (nvram_save() == 0)
                {
                    result = true;
                }
                free(newBootArgs);
            }
        }
    }

    return result;
}

// Utility function to extract a knob value by its name from non-volatile storage
bool knobs_load_knob_from_NVRAM(const char* knobName, knob_as_binary_t *knobValue)
{
    bool result = false;
    const char* existingBootArgs = NULL;

    // fetch up existing set of boot-args
    existingBootArgs = env_get(BOOT_ARGS_NAME);
    if (existingBootArgs)
    {
        const char *findNamePtr = strstr(existingBootArgs, knobName);
        if (findNamePtr)
        {
            // found one, processing
            const char *findValPtr= strstr(findNamePtr, "=");
            if (findValPtr)
            {
                // skip the equal sign and move ahead by making a copy
                findValPtr ++;
                char *valueCopy = strdup(findValPtr);
                if (valueCopy)
                {
                    // find the end by searching space, may not find if it was the last in boot-args
                    char *findValueCopyEnd = strstr(valueCopy, " ");
                    if (findValueCopyEnd)
                    {
                        // this will be the end of string, safe as we made a copy
                        *findValueCopyEnd = 0;
                    }

                    // we have a knob value isolated for processing
                    result = true;
                    if (valueCopy[0] == KNOB_ABSOLUTE_CHAR[0])
                    {
                        // absolute value
                        knobValue->u.bit.isAbsolute = 1;
                        knobValue->u.bit.isNegative = 0;
                        knobValue->u.bit.knobVal = atoi(&valueCopy[1]);
                    }
                    else
                    {
                        // offset
                        knobValue->u.bit.isAbsolute = 0;
                        int numValue = atoi(valueCopy);
                        if (numValue < 0)
                        {
                            knobValue->u.bit.isNegative = 1;
                            numValue = -numValue;
                        }
                        else
                        {
                            knobValue->u.bit.isNegative = 0;
                        }
                        knobValue->u.bit.knobVal = numValue;
                    }

                    free(valueCopy);
                }
            }
        }
    }

    return result;
}
// -========================================================================================-

void knobs_prepare_standby_storage(void)
{
    uint8_t pmu_bytes[VOLTAGE_KNOBS_SIZE] = {0};

#if DEBUG_BUILD
    printf("Voltage knobs: initializing PMU scratch registers\n");
#endif

    // save zeroes into PMU Scratch registers
    power_store_voltage_knobs(pmu_bytes, VOLTAGE_KNOBS_SIZE);
}

#if PRODUCT_IBOOT || PRODUCT_IBEC
// get knobs from NVRAM variables, convert them, and store in PMU scratch registers
// this will not be available in 2nd stage bootloaders as no access to NVRAM env vars there
void knobs_update_PMU_registers(bool apply_to_hw)
{
    const knob_type_description_t *currentDesc = &all_knobs[1];
    char knobName[KNOB_NAME_MAXLEN] = {0};
    uint8_t pmu_bytes[VOLTAGE_KNOBS_SIZE] = {0};
    bool featureActivated = false;

#if DEBUG_BUILD
    printf("Voltage knobs: storing knobs into PMU scratch registers\n");
#endif

    // try all possible names for building the list
    while(currentDesc->visible_name)
    {
        for (uint32_t idx = 0; idx < currentDesc->max_index; idx ++)
        {
            knob_as_binary_t actual_value;

            snprintf(knobName, sizeof(knobName), KNOBNAMEFORMAT, idx, currentDesc->nameprefix);

            // found one???
            actual_value.u.intval = 0;
            if (knobs_load_knob_from_NVRAM(knobName, &actual_value))
            {
                // at least one knob is set, so we are using this feature
                featureActivated = true;

                // now, find the knob bit position in pmu_bytes and place them...
                uint32_t bit_no = (idx + 1) * KNOB_SIZE_BITS;
                switch (currentDesc->defined_type)
                {
                    case kKnobTypeCPU:
                        break;
                    case kKnobTypeGPU:
                        bit_no += KNOB_SIZE_BITS * CPU_KNOB_MAX;
                        break;
                    case kKnobTypeSOC:
                        bit_no += KNOB_SIZE_BITS * GPU_KNOB_MAX + KNOB_SIZE_BITS * CPU_KNOB_MAX;
                        break;
                    case kKnobTypePMU:
                        bit_no += KNOB_SIZE_BITS * GPU_KNOB_MAX + KNOB_SIZE_BITS * CPU_KNOB_MAX + KNOB_SIZE_BITS * SOC_KNOB_MAX;
                        break;
                    default:
                        break;
                }

                // set the bits. we may need up to use up to three bytes
                uint32_t byte_idx = (bit_no - 1) >> 3;
                uint32_t bits_byte2 = bit_no & 0x07;
                uint32_t bits_byte1 = 0;
                uint32_t bits_byte0 = 0;

                if (bits_byte2 == 0)
                {
                    bits_byte2 = 8;
                }
                bits_byte1 = KNOB_SIZE_BITS - bits_byte2;
                if (bits_byte1 > 8)
                {
                    bits_byte1 = 8;
                    bits_byte0 = KNOB_SIZE_BITS - bits_byte2 - 8;
                }

                pmu_bytes[byte_idx] |= (actual_value.u.intval >> (KNOB_SIZE_BITS - bits_byte2));
                pmu_bytes[byte_idx - 1] |= actual_value.u.intval << (8 - bits_byte1);
                if (bits_byte0)
                {
                    pmu_bytes[byte_idx - 2] |= actual_value.u.intval << (8 - bits_byte0);
                }
            }
        }

        currentDesc ++;
    }

    if (featureActivated)
    {
        // Turn master switch on
        pmu_bytes[MASTER_SWITCH_BIT / 8] |= (1 << (MASTER_SWITCH_BIT % 8));

        // save the bits into PMU
        power_store_voltage_knobs(pmu_bytes, sizeof(pmu_bytes) / sizeof(pmu_bytes[0]));

        if (apply_to_hw)
        {
            knobs_load_from_standby_storage();
        }
    }
}
#else
// empty stub for the 2nd stage bootloaders
void knobs_update_PMU_registers(bool apply_to_hw)
{
}
#endif

// find a bit in a byte array by its number. returns 0 or 1 only
uint8_t get_bit(uint8_t *pmu_bytes, uint32_t bit_no)
{
    uint32_t byte_idx = bit_no >> 3;
    uint32_t bit_idx = bit_no & 0x07;

    return ((pmu_bytes[byte_idx] & (1 << bit_idx)) == 0) ? 0 : 1;
}

// Get knob bits (KNOB_SIZE_BITS) from a byte array starting from bit_no
// @pre: KNOB_SIZE_BITS < 16
uint32_t get_knob_bits(uint8_t *pmu_bytes, uint32_t bit_no)
{
    uint32_t byte_idx = (bit_no - 1) >> 3;
    uint32_t bits_byte2 = 0;
    int32_t bits_byte1 = 0;
    int32_t bits_byte0 = 0;
    uint32_t result = 0;

    bits_byte2 = bit_no & 0x07;
    if (bits_byte2 == 0)
    {
        bits_byte2 = 8;
    }
    bits_byte1 = KNOB_SIZE_BITS - bits_byte2;
    if (bits_byte1 > 8)
    {
        bits_byte1 = 8;
        bits_byte0 = KNOB_SIZE_BITS - bits_byte2 - 8;
    }

    check(KNOB_SIZE_BITS < 16);
    check(byte_idx > (bits_byte0 ? 1 : 0));
    check(byte_idx < VOLTAGE_KNOBS_SIZE);

    // load first chunk
    result |= (pmu_bytes[byte_idx] & (0xFF >> (8 - bits_byte2)));
    // now, make some room for next chunk on right
    result <<= bits_byte1;
    // move next chunk right and merge with previous chunk
    result |= (pmu_bytes[byte_idx - 1] >> (8 - bits_byte1));
    if (bits_byte0)
    {
        // clean up some bit space for the last chunk
        result <<= bits_byte0;
        // append last chunk
        result |= (pmu_bytes[byte_idx - 2] >> (8 - bits_byte0));
    }

    return result;
}

// This function loads voltage knobs from PMU scratch registers and applies them
// into related SoC registers
void knobs_load_from_standby_storage(void)
{
    bool success = false;
    uint8_t pmu_bytes[VOLTAGE_KNOBS_SIZE] = {0};
    uint32_t consumed_bits = 0;
    knob_as_binary_t knobPMUVal;

    // load PMU bit blob from PMU scratch registers
    success = power_load_voltage_knobs(pmu_bytes, VOLTAGE_KNOBS_SIZE);
    if (!success)
    {
        // do not printf any error to save limited LLB codespace
        return;
    }

    // check if master switch is on
    if (get_bit(pmu_bytes, MASTER_SWITCH_BIT) == 0)
    {
        // nope, feature is not activated
        return;
    }

#if (CPU_KNOB_MAX > 0)
    // extract and apply CPU voltage throught DVFM state registers
    for (uint32_t knob = 0 ; knob < CPU_KNOB_MAX; knob ++)
    {
        uint8_t voltage = 0;

        // advance bit pointer and extract knob from PMU bit blob
        consumed_bits += KNOB_SIZE_BITS;
        knobPMUVal.u.intval = get_knob_bits(pmu_bytes, consumed_bits);

        if (knobPMUVal.u.intval == 0)
        {
            // Knob is not defined.
            continue;
        }

        if (knobPMUVal.u.bit.isAbsolute)
        {
            voltage = knobPMUVal.u.bit.knobVal;
        }
        else
        {
            // Extract pre-set safe voltage.
            voltage = knobs_platform_get_cpu_perf_voltage(knob + kDVFM_STATE_IBOOT);

            // Modify safe voltage with offset provided.
            if (knobPMUVal.u.bit.isNegative)
            {
                voltage -= knobPMUVal.u.bit.knobVal;
            }
            else
            {
                voltage += knobPMUVal.u.bit.knobVal;
            }
        }

        // Update selected CPU power state with adjusted safe voltage.
        knobs_platform_set_cpu_perf_voltage(knob + kDVFM_STATE_IBOOT, voltage);
    }
#endif // (CPU_KNOB_MAX > 0)

#if (GPU_KNOB_MAX > 0)
    // extract and apply GPU voltage adjustments
    for (uint32_t knob = 0 ; knob < GPU_KNOB_MAX; knob ++)
    {
        consumed_bits += KNOB_SIZE_BITS;
        knobPMUVal.u.intval = get_knob_bits(pmu_bytes, consumed_bits);

        if (knobPMUVal.u.intval == 0)
        {
            continue;
        }

        // GFX perf states are not defined in PMGR, so I am assuming that state0 is reserved for SecureROM
        uint8_t voltage = knobs_platform_get_gfx_perf_voltage(knob + 1);
        if (knobPMUVal.u.bit.isAbsolute)
        {
            // For absolute voltage override it completely.
            voltage = knobPMUVal.u.bit.knobVal;
        }
        else
        {
            // Otherwise, offset the voltage with knob value.
            if (knobPMUVal.u.bit.isNegative)
            {
                voltage -= knobPMUVal.u.bit.knobVal;
            }
            else
            {
                voltage += knobPMUVal.u.bit.knobVal;
            }
        }

        // Apply new voltage.
        knobs_platform_set_gfx_perf_voltage(knob + 1, voltage);
    }
#endif

#if (SOC_KNOB_MAX > 0)
    // extract and apply SoC voltage through SoC perf state PMGR registers
    for (uint32_t knob = 0 ; knob < SOC_KNOB_MAX; knob ++)
    {
        consumed_bits += KNOB_SIZE_BITS;
        knobPMUVal.u.intval = get_knob_bits(pmu_bytes, consumed_bits);

        if (knobPMUVal.u.intval == 0)
        {
            continue;
        }

        uint8_t soc_voltage = knobs_platform_get_soc_perf_voltage(kSOC_PERF_STATE_IBOOT + knob);

        if (knobPMUVal.u.bit.isAbsolute)
        {
            soc_voltage = knobPMUVal.u.bit.knobVal;
        }
        else
        {
            if (knobPMUVal.u.bit.isNegative)
            {
                soc_voltage -= knobPMUVal.u.bit.knobVal;
            }
            else
            {
                soc_voltage += knobPMUVal.u.bit.knobVal;
            }
        }

        knobs_platform_set_soc_perf_voltage(kSOC_PERF_STATE_IBOOT + knob, soc_voltage);
    }
#endif

#if (PMU_KNOB_MAX > 0)
    // Extract and apply PMU voltages thorough it's IIC registers.
    for (uint32_t knob = 0 ; knob < PMU_KNOB_MAX; knob ++)
    {
        consumed_bits += KNOB_SIZE_BITS;
        knobPMUVal.u.intval = get_knob_bits(pmu_bytes, consumed_bits);

        if (knobPMUVal.u.intval == 0)
        {
            continue;
        }

        if (knobPMUVal.u.bit.isAbsolute)
        {
            knobs_pmu_set_data(knob, knobPMUVal.u.bit.knobVal);
        }
        else
        {
            // Some of PMU knobs are not handled by iBoot somewhere else, and they are
            // preserved through sleep cycles as PMU never sleeps
            // so read - offset - write scheme is not acceptable here.
            // Using default value instead.
            uint8_t pmu_knob_val = 0x00;
            knobs_get_default_val(kKnobTypePMU, knob, &pmu_knob_val);

            if (knobPMUVal.u.bit.isNegative)
            {
                pmu_knob_val -= knobPMUVal.u.bit.knobVal;
            }
            else
            {
                pmu_knob_val += knobPMUVal.u.bit.knobVal;
            }

            knobs_pmu_set_data(knob, pmu_knob_val);
        }
    }
#endif

    check(consumed_bits <= MASTER_SWITCH_BIT);
}

#ifndef SUB_PLATFORM_S7002
// request default value of a knob in units
bool knobs_get_default_val(knob_types_e type, uint8_t index, uint8_t *defaultValueUnits)
{
#if (CPU_KNOB_MAX > 0)
    u_int32_t num_dvfm_states = kDVFM_STATE_IBOOT_CNT;
    u_int32_t cpu_vid[num_dvfm_states];
#endif
#if (GPU_KNOB_MAX > 0)
    u_int32_t gpu_vid[kPMGR_GFX_STATE_MAX];
#endif

    if (defaultValueUnits == NULL || type >= kKnobTypeInvalid || index >= all_knobs[type].max_index)
    {
        // Invalid parameter.
        return false;
    }

    switch (type)
    {
        case kKnobTypeCPU:
#if (CPU_KNOB_MAX > 0)
            platform_get_cpu_voltages(num_dvfm_states, cpu_vid);
            if (platform_convert_voltages(BUCK_CPU, num_dvfm_states, cpu_vid) == -1)
            {
                return false;
            }

            *defaultValueUnits = cpu_vid[index + kDVFM_STATE_IBOOT];
            break;
#else
            return false;
#endif
        case kKnobTypeGPU:
#if (GPU_KNOB_MAX > 0)
            {
                const u_int32_t buckCount = __min(kPMGR_GFX_STATE_MAX, GPU_KNOB_MAX);

                platform_get_gpu_voltages(buckCount, gpu_vid);
                if (platform_convert_voltages(BUCK_GPU, buckCount, gpu_vid) == -1)
                {
                    return false;
                }
            }

            // skip securerom state. this is not defined in platform support code
            *defaultValueUnits = gpu_vid[index + 1];
            break;
#else
            return false;
#endif
        case kKnobTypeSOC:
#if (SOC_KNOB_MAX > 0)
            // no API for that, so values are hardcoded until platform code will be extended to full support of SoC voltages
            if (index == 0) *defaultValueUnits = 0x60;
            if (index == 1) *defaultValueUnits = 0x70;
            break;
#else
            return false;
#endif
        case kKnobTypePMU:
            if (index == 0) *defaultValueUnits = KNOB_PMU_BUCK3_POR;
            if (index == 1) *defaultValueUnits = KNOB_PMU_BUCK4_POR;
            if (index == 2)
            {
                uint32_t dwiVal = 0;
                knobs_convert_mv_to_dwi(type, index, platform_get_base_ram_voltage(), &dwiVal);
                *defaultValueUnits = dwiVal;
            }
            break;

        default:
            return false;
    }

    return true;
}
#endif // SUB_PLATFORM_S7002

#endif
