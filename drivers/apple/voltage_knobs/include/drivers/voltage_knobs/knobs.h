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

#ifndef __APPLE_KNOBS_H
#define __APPLE_KNOBS_H

#if defined(SUB_PLATFORM_S7002)
// PMU knobs only for M7
#define CPU_KNOB_MAX    0
#define GPU_KNOB_MAX    0
#define SOC_KNOB_MAX    0
#define PMU_KNOB_MAX    3
#elif defined(SUB_PLATFORM_T7001)
// Last voltage, V5 could be unused,
// but all arrays in pmgr and cpuid
// have a value for it.
#define CPU_KNOB_MAX    6
#define GPU_KNOB_MAX    4
#define SOC_KNOB_MAX    2
#define PMU_KNOB_MAX    3
#else
#define CPU_KNOB_MAX	5
#define GPU_KNOB_MAX    4
#define SOC_KNOB_MAX    2
#define PMU_KNOB_MAX    3
#endif


#define KNOB_SIZE_BITS  10

// we are not expecting more than 8 knobs per unit
#define KNOBINDEX_ABSOLUTE_MAX  7

// buffer size to prepare NVRAM knob name
#define KNOB_NAME_MAXLEN        7

// the way knob is stored in PMU register (and in NVRAM too)
typedef struct
{
    union
    {
        struct
        {
            unsigned int knobVal    : 8;
            int isNegative : 1;
            int isAbsolute : 1;
        } bit;
        uint16_t intval;
    } u;
} knob_as_binary_t;

#ifndef __abs
#define __abs(x) (((x) < 0) ? -(x) : (x))
#endif

// Note: a knob has 10 bits ASVVVVVVVV value
//       A [0 - offset, 1 - absolute value]
//       S [if A = 0, S = 1 means negative offset]
//       VVVVVVVV - 8 bit knob value to apply

#define KNOBNAMEPREFIX  "knob"
#define KNOBNAMEFORMAT  "knob%X%c"
#define KNOB_ABSOLUTE_CHAR "@"
// KNOB NVRAM variable format is name=value
//   where name is knobIP
//                 I - knob index in hexadecimal [0...F]
//                 P - nameprefix from knob type definition

// Knobs defined so far including their T7000 aliases:
//     CPU 0 - kDVFM_STATE_IBOOT (2)        * for CPU skippint kDVFM_STATE_BYPASS and kDVFM_STATE_SECUREROM as we have no control there
//     CPU 1 - V0
//     CPU 2 - V1 kDVFM_STATE_VNOM
//     CPU 3 - V2 
//     CPU 4 - V3 kDVFM_STATE_VMAX  / kDVFM_STATE_VBOOST
//
//     CPU 0
//     CPU 1
//
//     GPU 0
//     GPU 1
//     GPU 2
//     GPU 3
//
//     PMU 0 - BUCK3 (1.8V)               * DRAM 1.8V
//     PMU 1 - BUCK4 (1.2V)               * DRAM 1.2V
//     PMU 2 - BUCK5 (0.95V)              * SoC fixed rail


typedef enum
{
    kKnobTypeUnknown = 0,
    kKnobTypeCPU,
    kKnobTypeGPU,
    kKnobTypeSOC,
    kKnobTypePMU,
    kKnobTypeInvalid,
} knob_types_e;

typedef struct
{
    const char *visible_name;
    char nameprefix;
    knob_types_e defined_type;
    uint32_t   max_index;
} knob_type_description_t;

// PMU has specific knobs.. 
#define KNOB_PMU_DRAM_1V8	0
#define KNOB_PMU_DRAM_1V2	1
#define KNOB_PMU_SOC_FIXED	2

// array index should match enum value for indexed access
extern const knob_type_description_t all_knobs[];

// clean up PMU scratch registers on a cold boot as their state should be undefined
// it may be called from LLB or iBSS
void knobs_prepare_standby_storage(void);

// called from LLB on a warm boot only
// for loading and appying voltage knobs from a standby storage
void knobs_load_from_standby_storage(void);

// get knobs from NVRAM variables, convert them, and store in PMU scratch registers
// or do nothing if we are in LLB or iBSS
void knobs_update_PMU_registers(bool apply_to_hw);

// request default value of a knob in units
bool knobs_get_default_val(knob_types_e type, uint8_t index, uint8_t *defaultValueUnits);

// convert units to millivolts for specific knob
int knobs_convert_dwi_to_mv(knob_types_e type, uint8_t index, u_int32_t dwival);
// converts millivolts to units for specific knob
int knobs_convert_mv_to_dwi(knob_types_e type, uint8_t index, u_int32_t mv, u_int32_t *dwival);

// search and load single knob from NV storage
bool knobs_load_knob_from_NVRAM(const char* knobName, knob_as_binary_t *knobValue);
// store single knob into NV storage
bool knobs_store_knob_in_NVRAM(const char* knobName, knob_as_binary_t *knobValue);
// remove single knob from NV storage
bool knobs_remove_knob_from_NVRAM(const char* knobName);

// write PMU buck voltage (use KNOB_PMU_BUCKn as index)
void knobs_pmu_set_data(uint8_t index, uint8_t value);

// * Platform specific stuff *

// Get/Set GPU/CPU/SoC voltage for specific performance state in units
uint8_t knobs_platform_get_gfx_perf_voltage(uint32_t perfStateNo);
void knobs_platform_set_gfx_perf_voltage(uint32_t perfStateNo, uint8_t newVoltage);

uint8_t knobs_platform_get_cpu_perf_voltage(uint32_t perfStateNo);
void knobs_platform_set_cpu_perf_voltage(uint32_t perfStateNo, uint8_t newVoltage);

uint8_t knobs_platform_get_soc_perf_voltage(uint32_t perfStateNo);
void knobs_platform_set_soc_perf_voltage(uint32_t perfStateNo, uint8_t newVoltage);

#endif
