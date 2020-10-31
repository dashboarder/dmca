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

/*
   Short feature description:
  
      ** Voltage knobs are set as NVRAM variables using menu provided or any other way
      ** Knobs are stored in PMU units (~3.1 mv) even if they been provided in mV in menu
      ** LLB has no access to NVRAM, so we need to use some PMU scratch registers to support sleep/wake cycle
      ** On a cold boot PMU registers used for temp. storage are cleared as their initial state is undefined
      ** On a cold boot knobs are loaded from NVRAM, copied to PMU Scratch registers, and applied to hardware
      ** On a worm boot knobs are loaded from PMU Scratch registers
      ** DISABLE temperature adjustments if knob has absolute voltage (DVFM only)
      ** Knobs are either absolute or offset; offsets may be negative
      ** PMU scratch registers have master switch bit
*/

#include <sys/menu.h>
#include <stdint.h>
#include <assert.h>
#include <debug.h>
#include <platform.h>
#include <platform/chipid.h>
#include <stdio.h>
#include <sys.h>
#include <lib/env.h>

#include <platform/soc/pmgr.h>

#if WITH_VOLTAGE_KNOBS
#include <drivers/voltage_knobs/knobs.h>
#endif

#define MENUCMD     knobs
#define MENUCMDSTR  "knobs"

#define LISTKNOBSCMD    "list"
#define ADDKNOBCMD      "add"
#define REMOVEKNOBCMD   "remove"
#define DEFAULTSCMD     "defaults"

// this code should be included and debug and development builds only
#if !RELEASE_BUILD && WITH_MENU

static int usage(void)
{
    printf("usage:\n");
    printf(MENUCMDSTR " " LISTKNOBSCMD "\n");
    printf(MENUCMDSTR " " ADDKNOBCMD " <knob cpu/gpu/soc/pmu> <knobindex> <knobtype abs/offset> <value> <mv/units>\n");
    printf(MENUCMDSTR " " REMOVEKNOBCMD " <knob cpu/gpu/soc/pmu> <knobindex>\n");
    printf(MENUCMDSTR " " DEFAULTSCMD " list default voltages when knobs are not defined\n");
    printf("\nNew voltages will be used on a next boot.\n");
    printf("\nAbsolute voltage below 600mv may be handled not correctly.\n");

    return 0;
}

knob_types_e knob_type_by_name(char* knobname)
{
    const knob_type_description_t *currentDesc = &all_knobs[1];
    
    while(currentDesc->visible_name)
    {
        if (!strcmp(knobname, currentDesc->visible_name))
        {
            return currentDesc->defined_type;
        }
        currentDesc ++;
    }
    
    return kKnobTypeInvalid;
}

// TODO! move to separate function, so those defaults may be used in 'add' function for validating user input
static void knobs_defaults(void)
{
    uint8_t base_vol = 0xff;

#if (CPU_KNOB_MAX > 0)
    for (uint32_t knob = 0 ; knob < CPU_KNOB_MAX; knob ++)
    {
        const char *prefix = "";
        
        switch (knob + kDVFM_STATE_IBOOT)
        {
            case kDVFM_STATE_VNOM:
                prefix = "-- Vnom";
                break;
#ifdef kDVFM_STATE_VMAX
            case kDVFM_STATE_VMAX:
                prefix = "-- Vmax";
                break;
#endif
            default:
                break;
        }
        
#ifdef DVFM_STATE_VMAX
    u_int32_t chip_rev = chipid_get_chip_revision();
    if ((knob + kDVFM_STATE_IBOOT) == DVFM_STATE_VMAX(chip_rev))
    {
        prefix = "-- Vmax";
    }
#endif

        if (knobs_get_default_val(kKnobTypeCPU, knob, &base_vol))
        {
            printf("cpu %d abs %d units (%d mV) %s\n", knob, base_vol, knobs_convert_dwi_to_mv(kKnobTypeCPU, knob, base_vol), prefix);
        }
    }
#endif

    for (uint32_t knob = 0 ; knob < GPU_KNOB_MAX; knob ++)
    {
        if (knobs_get_default_val(kKnobTypeGPU, knob, &base_vol))
        {
            printf("gpu %d abs %d units (%d mV)\n", knob, base_vol, knobs_convert_dwi_to_mv(kKnobTypeGPU, knob, base_vol));
        }
    }

    for (uint32_t knob = 0 ; knob < SOC_KNOB_MAX; knob ++)
    {
        if (knobs_get_default_val(kKnobTypeSOC, knob, &base_vol))
        {
            printf("soc %d abs %d units (%d mV)\n", knob, base_vol, knobs_convert_dwi_to_mv(kKnobTypeSOC, knob, base_vol));
        }
    }

    for (uint32_t knob = 0 ; knob < PMU_KNOB_MAX; knob ++)
    {
        const char *prefix = "";
        
        switch (knob)
        {
            case KNOB_PMU_DRAM_1V8:
                prefix = "-- DRAM 1.8V";
                break;
            case KNOB_PMU_DRAM_1V2:
                prefix = "-- DRAM 1.2V";
            break;
            case KNOB_PMU_SOC_FIXED:
                prefix = "-- SoC Fixed Rail";
                break;
            default:
                break;
        }

        if (knobs_get_default_val(kKnobTypePMU, knob, &base_vol))
        {
            printf("pmu %d abs %d units (%d mV) %s\n", knob, base_vol, knobs_convert_dwi_to_mv(kKnobTypePMU, knob, base_vol), prefix);
        }
    }
}

static void knobs_list(void)
{
    const knob_type_description_t *currentDesc = &all_knobs[1];
    char knobName[KNOB_NAME_MAXLEN] = {0};

    // try all possible names for building the list
    while(currentDesc->visible_name)
    {
        for (uint32_t idx = 0; idx < currentDesc->max_index; idx ++)
        {
            knob_as_binary_t actualValue;
            
            snprintf(knobName, sizeof(knobName), KNOBNAMEFORMAT, idx, currentDesc->nameprefix);

            // found one???
            if (knobs_load_knob_from_NVRAM(knobName, &actualValue))
            {
                int mvValue = knobs_convert_dwi_to_mv(currentDesc->defined_type, idx, actualValue.u.bit.knobVal);
                
                if (actualValue.u.bit.isAbsolute == 0)
                {
                    // apply correction for offset to get correct value in mV
                    int minPossibleValue = knobs_convert_dwi_to_mv(currentDesc->defined_type, idx, 0);
                    mvValue -= minPossibleValue;
                }
                
                
                printf("knob %s %d: %s %c%d units (%c%d mV)\n", currentDesc->visible_name, idx,
                       actualValue.u.bit.isAbsolute ? "abs" : "offset",
                       actualValue.u.bit.isNegative ? '-' : ' ',
                       actualValue.u.bit.knobVal,
                       actualValue.u.bit.isNegative ? '-' : ' ',
                       mvValue);
            }
        }
        
        currentDesc ++;
    }
}

#define KNOB_MAX_UNITS            255
#define WARNING_TRESHOLD_MV       100
#define WARNING_DELTA_TRESHOLD_MV 5
static void knobs_add(knob_types_e knob_type, uint32_t knob_index, bool absolute, int32_t knob_value, bool in_mv)
{
    char knobName[KNOB_NAME_MAXLEN] = {0};
    knob_as_binary_t actualKnobValue;
    const knob_type_description_t *currentDesc = &all_knobs[knob_type];

	if (knob_index >= currentDesc->max_index)
	{
		printf("Knob index out of range.\n");
		return;
	}
    
    snprintf(knobName, sizeof(knobName), KNOBNAMEFORMAT, knob_index, all_knobs[knob_type].nameprefix);
    
    // query minimal possible value in mV by converting 0 units to millivolts
    // for validation new value requested
    int minPossibleValue = knobs_convert_dwi_to_mv(knob_type, knob_index, 0);
    int maxPossibleValue = knobs_convert_dwi_to_mv(knob_type, knob_index, KNOB_MAX_UNITS);
    
    if (in_mv)
    {
        // * Setting knob in millivolts *
        u_int32_t knobValueUnits = 0;
        
		if (absolute)
		{
            // requested absolute value in millivolts should not be less than minimal possible
            if (knob_value < minPossibleValue)
            {
                printf("Unable to set absolute value less than %d mV for this hardware.\n", minPossibleValue);
                return;
            }
            if (knob_value > maxPossibleValue)
            {
                printf("Unable to set absolute value more than %d mV for this hardware.\n", maxPossibleValue);
                return;
            }
            
        	// convert value to PMU units
        	knobs_convert_mv_to_dwi(knob_type, knob_index, knob_value, &knobValueUnits);
            // read it back to verify
            int actualVoltage = knobs_convert_dwi_to_mv(knob_type, knob_index, knobValueUnits);
            printf("Actual absolute voltage is %d mV\n", actualVoltage);
            int delta = __abs(actualVoltage - knob_value);
            if (delta > WARNING_DELTA_TRESHOLD_MV)
            {
                printf("WARNING!!! Actual voltage is different from requested by %d mV.\n", delta);
            }
            
            // check if new absolute value within 100mV ballpark from default value
            uint8_t defaultValUnits = 0;
            if (knobs_get_default_val(knob_type, knob_index, &defaultValUnits))
            {
                int defaultValMV = knobs_convert_dwi_to_mv(knob_type, knob_index, defaultValUnits);
                int delta = __abs(defaultValMV - knob_value);
                if (delta > WARNING_TRESHOLD_MV)
                {
                    printf("WARNING!!! New knob value is differ from its default value for more than 100mV.\n");
                }
            }
		}
		else
		{
			// as voltage scale starts not from 0, we must get initial point first for calculating offsets
        	knobs_convert_mv_to_dwi(knob_type, knob_index, __abs(knob_value) + minPossibleValue, &knobValueUnits);
            if (__abs(knob_value) > WARNING_TRESHOLD_MV)
            {
                printf("WARNING!!! Your offsett is above %d mV.\n", WARNING_TRESHOLD_MV);
            }
		}
		printf("Knob value is %d mV (%d units)\n", knob_value, knobValueUnits);
        
        actualKnobValue.u.bit.knobVal = knobValueUnits;
    }
    else
    {
        // * Setting knob in Units *
        int knobValueinMV = 0;
        uint8_t defaultValUnits = 0;
        knobs_get_default_val(knob_type, knob_index, &defaultValUnits);
        int defaultValMV = knobs_convert_dwi_to_mv(knob_type, knob_index, defaultValUnits);
        
		if (absolute)
		{
            knobValueinMV = knobs_convert_dwi_to_mv(knob_type, knob_index, knob_value);
            
            if (knob_value > KNOB_MAX_UNITS)
            {
                printf("Unable to set absolute value more than %d units for this hardware.\n", KNOB_MAX_UNITS);
                return;
            }
            
            int delta = __abs(defaultValMV - knobValueinMV);
            if (delta > WARNING_TRESHOLD_MV)
            {
                printf("WARNING!!! New knob value is differ from its default value for more than %d mV.\n", WARNING_TRESHOLD_MV);
            }
        }
        else
        {
            if (knob_value < 0 && __abs(knob_value) > defaultValUnits)
            {
                // negative offset should not be greater than default value
                printf("ERROR: negative offset is exceeeding default valie, so it cannot be set.\n");
                return;
            }
            
            if (knob_value > 0 && (knob_value + defaultValUnits) > KNOB_MAX_UNITS)
            {
                // positive offset will overflow 8 bit HW register
                printf("ERROR: positive offset will make voltage to exceed %d units, and it cannot be done.\n", KNOB_MAX_UNITS);
                return;
            }
            
            int newValInMV = knobs_convert_dwi_to_mv(knob_type, knob_index, defaultValUnits + knob_value);
            knobValueinMV = newValInMV - defaultValMV;
            if (__abs(knobValueinMV) > WARNING_TRESHOLD_MV)
            {
                printf("WARNING!!! Your offset is exceeding %d mV.\n", WARNING_TRESHOLD_MV);
            }
        }
        
		printf("Knob value is %d mV (%d units)\n", knobValueinMV, knob_value);
        
        // already in PMU DIALOG_CORE_BUCKOUTunits
        actualKnobValue.u.bit.knobVal = __abs(knob_value);
    }
    
    actualKnobValue.u.bit.isNegative = (knob_value < 0) ? 1 : 0;
    actualKnobValue.u.bit.isAbsolute = absolute ? 1 : 0;
    
    if (knobs_store_knob_in_NVRAM(knobName, &actualKnobValue) == false)
    {
		printf("Unable to store knob into NV storage\n");
    }
    
    // and update volatile PMU registers
    knobs_update_PMU_registers(false);
}

static void knobs_remove(knob_types_e knob_type, uint32_t knob_index)
{
    char knobName[KNOB_NAME_MAXLEN] = {0};
    const knob_type_description_t *currentDesc = &all_knobs[knob_type];

	if (knob_index >= currentDesc->max_index)
	{
		printf("Knob index out of range.\n");
		return;
	}
    
    snprintf(knobName, sizeof(knobName), KNOBNAMEFORMAT, knob_index, all_knobs[knob_type].nameprefix);
    
#if DEBUG_BUILD
	printf("removing '%s' from env.\n", knobName);
#endif

    if (knobs_remove_knob_from_NVRAM(knobName) == false)
    {
		printf("Unable to remove knob from NV storage\n");
    }

    // and update volatile PMU registers
    knobs_update_PMU_registers(false);
}

static int do_voltage_knobs(int argc, struct cmd_arg *args)
{
    knob_types_e knob_type = kKnobTypeUnknown;
    
    if (argc < 2)
    {
        printf("Not enough arguments\n");
        return usage();
    }
    
    
    if (!strcmp(args[1].str, LISTKNOBSCMD))
    {
        knobs_list();
    }
    else if (!strcmp(args[1].str, ADDKNOBCMD))
    {
        bool absolute = false;
        bool in_mv = false;

        if (argc==7)
        {
            knob_type = knob_type_by_name(args[2].str);
            if (knob_type == kKnobTypeUnknown || knob_type == kKnobTypeInvalid)
            {
                return usage();
            }
            
            if (!strcmp(args[4].str, "abs"))
            {
                absolute = true;
            }
            else if (!strcmp(args[4].str, "offset"))
            {
                absolute = false;
            }
            else
            {
                printf("parameter 4 shoud be abs or offset\n");
                return usage();
            }
            
            if (!strcmp(args[6].str, "mv"))
            {
                in_mv = true;
            }
            else if (!strcmp(args[6].str, "units"))
            {
                in_mv = false;
            }
            else
            {
                printf("parameter 6 shoud be mv or units\n");
                return usage();
            }
            
            knobs_add(knob_type, args[3].u, absolute, args[5].u, in_mv);
        }
        else
        {
            return usage();
        }
    }
    else if (!strcmp(args[1].str, REMOVEKNOBCMD))
    {
        if (argc==4)
        {
            knob_type = knob_type_by_name(args[2].str);
            if (knob_type == kKnobTypeUnknown || knob_type == kKnobTypeInvalid)
            {
                return usage();
            }
            
            knobs_remove(knob_type, args[3].u);
        }
        else
        {
            return usage();
        }
    }
    else if (!strcmp(args[1].str, DEFAULTSCMD))
    {
        knobs_defaults();
    }
	else
	{
        printf("unlknown argument 1\n");
        return usage();
	}

    return 0;
}

// _command, _function, _help, _meta
MENU_COMMAND_DEVELOPMENT(MENUCMD, do_voltage_knobs, "Platform voltage knobs adjustment commands", NULL);

#endif // !RELEASE_BUILD && WITH_MENU
