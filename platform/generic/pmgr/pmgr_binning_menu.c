/*

 * Copyright (c) 2015 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */

#if !RELEASE_BUILD && WITH_MENU

#include <sys/menu.h>
#include <stdint.h>
#include <assert.h>
#include <debug.h>
#include <platform.h>
#include <stdio.h>
#include <sys.h>
#include <lib/env.h>
#include <platform/pmgr.h>

static bool pmgr_binning_menu_override_get_voltage_indexes(bool gpu, uint32_t *voltage_indexes, uint32_t voltage_index_size, uint32_t *voltage_index_found)
{
	uint32_t freq = 0;
	uint32_t i = 0;
	const char *name = env_get(gpu?"pmgr_gpu_override":"pmgr_cpu_override");
	const struct pmgr_binning_voltage_index_to_config *config_data;
	*voltage_index_found = 0;
	if (name == NULL) {
		return false;
	}

	while (i < voltage_index_size) {
		if (i > 0) {
			name = strchr(name, ',');
			if (name == NULL) {
				break;
			}
			name++;
		}
		
		voltage_indexes[i] = atoi(name);
		if (pmgr_binning_from_config_data(voltage_indexes[i], &config_data) == PMGR_BINNING_NOTFOUND) {
			return false;
		}
		if ((gpu) && (config_data->type != PMGR_BINNING_GPU)) {
			return false;
		}
		if ((!gpu) && (config_data->type != PMGR_BINNING_CPU)) {
			return false;
		}
		freq = pmgr_platform_get_freq(gpu, voltage_indexes[i]);
		if (freq == 0) {
			return false;
		}
		i++;
	}
	*voltage_index_found = i;
	return true;
}

static bool pmgr_binning_menu_find_safe_voltage(enum pmgr_binning_type_t type, uint32_t mode,
				uint32_t *safe_voltage, uint32_t *freq)
{
	uint32_t i;
	*freq = 0;
	for (i = 0; i < pmgr_binning_voltage_config_sram_data_size; i++) {
		if (pmgr_binning_voltage_config_sram_data[i].mode != mode) {
			continue;
		}
		if (pmgr_binning_voltage_config_sram_data[i].type != type) {
			continue;
		}
		*safe_voltage = pmgr_binning_voltage_config_sram_data[i].safe_voltage;
		return true;
	}
	for (i = 0; i < pmgr_binning_voltage_index_to_config_data_size; i++) {
		if (pmgr_binning_voltage_index_to_config_data[i].mode != mode) {
			continue;
		}
		if (pmgr_binning_voltage_index_to_config_data[i].type != type) {
			continue;
		}
		if (type == PMGR_BINNING_CPU) {
			*freq = pmgr_platform_get_freq(false, pmgr_binning_voltage_index_to_config_data[i].voltage_index);
		} else if (type == PMGR_BINNING_GPU) {
			*freq = pmgr_platform_get_freq(true, pmgr_binning_voltage_index_to_config_data[i].voltage_index);
		}
		*safe_voltage = pmgr_binning_voltage_index_to_config_data[i].safe_voltage;
		return true;
	}
	return false;
}

static void pmgr_binning_menu_print_type(enum pmgr_binning_type_t type)
{
	uint32_t mode;
	for (mode = 1; mode < 63; mode++) {
		uint32_t result = pmgr_binning_mode_get_value(type, mode);
		uint32_t safe_voltage;
		uint32_t freq;
		if ((result == PMGR_BINNING_NOTFOUND) || (result == 0)) {
			continue;
		}
		printf("    mode %u %u mV (0x%x)", mode, pmgr_binning_mode_get_mv(type, mode), pmgr_binning_mode_get_value(type, mode));

		if (!pmgr_binning_menu_find_safe_voltage(type, mode, &safe_voltage, &freq)) {
			printf(" no safe voltage");
		} else {
			printf(" safe voltage %u mV", safe_voltage);
		}

		if (freq > 0) {
			printf(" %u MHz", freq);
		}
		printf("\n");
	}
}

static void pmgr_binning_menu_voltage_config_print(const struct pmgr_binning_voltage_config *voltage_config, uint32_t voltage_index)
{
	const char *name = "";
	uint32_t freq = 0, safe_voltage = 0, mode = 0, type = 0;
	bool sram = false;

	if (voltage_config != NULL) {
		safe_voltage = voltage_config->safe_voltage;
		mode = voltage_config->mode;
		type = voltage_config->type;
	}
	else {
		const struct pmgr_binning_voltage_index_to_config *config_data;
		if (pmgr_binning_from_config_data(voltage_index, &config_data) == PMGR_BINNING_NOTFOUND) {
			return;
		}
		safe_voltage = config_data->safe_voltage;
		mode = config_data->mode;
		type = config_data->type;
	}
	if (type == PMGR_BINNING_NONE) {
		return;
	}
	switch (type) {
		case PMGR_BINNING_CPU:
			name = "CPU";
			freq = pmgr_platform_get_freq(false, voltage_index);
			break;
		case PMGR_BINNING_GPU:
			name = "GPU";
			freq = pmgr_platform_get_freq(true, voltage_index);
			break;
		case PMGR_BINNING_SOC:
			name = "SOC";
			break;
		case PMGR_BINNING_CPU_SRAM:
			name = "CPU SRAM";
			sram = true;
			break;
		case PMGR_BINNING_GPU_SRAM:
			name = "GPU SRAM";
			sram = true;
			break;
		case PMGR_BINNING_SRAM:
			name = "SRAM";
			sram = true;
			break;
		default:
			name = "Unknown";
			break;
	}

	printf("    %s index %u safe voltage %u mV",
			name,
			voltage_index,
			pmgr_binning_get_mv(voltage_index, sram, false));

	if (pmgr_binning_mode_get_mv(type,  mode) != PMGR_BINNING_NOTFOUND) {
		printf(" binning %u mV", pmgr_binning_get_mv(voltage_index, sram, true));
	} else {
		printf(" no binning %u mV", pmgr_binning_get_mv(voltage_index, sram, true));
	}

	printf(" offset %d mV", pmgr_binning_get_voltage_offset(voltage_index, type));

	if (mode != PMGR_BINNING_MODE_NONE && mode > 0) {
		printf(" mode %u", mode);
	}

	if (freq > 0) {
		printf(" %u MHz", freq);
	}

	if (sram && safe_voltage == 0) {
		uint32_t i;
		for (i = 0; i < pmgr_binning_mode_to_const_data_size; i++) {
			if (pmgr_binning_mode_to_const_data[i].type != type) {
				continue;
			}
			if (pmgr_binning_mode_to_const_data[i].mode != mode) {
				continue;
			}
			if (pmgr_binning_mode_to_const_data[i].bingroup != pmgr_binning_get_group() && pmgr_binning_mode_to_const_data[i].bingroup != PMGR_BINNING_GROUP_ALL) {
				continue;
			}
			printf(" fuse >= %d %d mV", pmgr_binning_mode_to_const_data[i].fuse_revision_minimum,  pmgr_binning_mode_to_const_data[i].mv + pmgr_binning_get_voltage_offset(voltage_index, type));
		}
	}
	else if (!sram && mode != PMGR_BINNING_MODE_NONE) {
		uint32_t i;
		for (i = 0; i < pmgr_binning_voltage_index_to_config_data_size; i++) {
			if (pmgr_binning_voltage_index_to_config_data[i].voltage_index != voltage_index) {
				continue;
			}
			if (pmgr_binning_voltage_index_to_config_data[i].type != type) {
				continue;
			}
			if (pmgr_binning_voltage_index_to_config_data[i].bingroup != pmgr_binning_get_group() && pmgr_binning_voltage_index_to_config_data[i].bingroup != PMGR_BINNING_GROUP_ALL) {
				continue;
			}
			printf(" fuse >= %d %d mV", pmgr_binning_voltage_index_to_config_data[i].fuse_revision_minimum,  pmgr_binning_voltage_index_to_config_data[i].safe_voltage + pmgr_binning_get_voltage_offset(voltage_index, type));
		}
	}
	printf("\n");
}

static void pmgr_binning_menu_current_values_type(enum pmgr_binning_type_t type)
{
	uint32_t i = 0;
	uint32_t freq;
	uint32_t voltage;
	while (pmgr_platform_get_perf_state(type, i, &voltage, &freq)) {
		if (freq == 0) {
			printf("    state %u: %u mV", i, voltage);
		} else {
			printf("    state %u: %u Mhz %u mV", i, freq / 1000000, voltage);
		}
		if ((type == PMGR_BINNING_CPU) && (pmgr_platform_get_perf_state(PMGR_BINNING_CPU_SRAM, i, &voltage, &freq))) {
			printf(" SRAM %u mV", voltage);
		}
		if ((type == PMGR_BINNING_GPU) && (pmgr_platform_get_perf_state(PMGR_BINNING_GPU_SRAM, i, &voltage, &freq))) {
			printf(" SRAM %u mV", voltage);
		}
		printf("\n");
		i++;
	}
}

static int do_pmgr(int argc, struct cmd_arg *args)
{
	uint32_t voltage_indexes[32];
	uint32_t max_voltage_index = 0;
	uint32_t voltage_index_found;
	if ((argc < 2) || ((strcmp(args[1].str, "binning") != 0) && (strcmp(args[1].str, "values") != 0))) {
		printf("Usage\n");
		printf("    pmgr [values | [binning [internal|values]]]\n");
		printf("         pmgr binning: fused value\n");
		printf("         pmgr binning internal: fused value and all known operating points\n");
		printf("         pmgr values: values currently set in PMGR/ACC registers\n");
		return 0;
	}

	if ((argc > 1) && (strcmp(args[1].str, "values") == 0)) {
                printf("CPU:\n");
                pmgr_binning_menu_current_values_type(PMGR_BINNING_CPU);
                printf("GPU:\n");
                pmgr_binning_menu_current_values_type(PMGR_BINNING_GPU);
                printf("SOC:\n");
                pmgr_binning_menu_current_values_type(PMGR_BINNING_SOC);
		return 0;
        }

	printf("Fuse revision %u base %u bin group %u\n", pmgr_binning_get_revision(), pmgr_binning_get_base(), pmgr_binning_get_group());
	printf("  CPU\n");
	pmgr_binning_menu_print_type(PMGR_BINNING_CPU);
	printf("  CPU SRAM\n");
	pmgr_binning_menu_print_type(PMGR_BINNING_CPU_SRAM);
	printf("  GPU\n");
	pmgr_binning_menu_print_type(PMGR_BINNING_GPU);
	printf("  GPU SRAM\n");
	pmgr_binning_menu_print_type(PMGR_BINNING_GPU_SRAM);
	printf("  SRAM\n");
	pmgr_binning_menu_print_type(PMGR_BINNING_SRAM);
	printf("  SOC\n");
	pmgr_binning_menu_print_type(PMGR_BINNING_SOC);

	if ((argc > 2) && (strcmp(args[2].str, "internal") == 0)) {
		uint32_t i;
		printf("\n");
		for (i = 0; i < pmgr_binning_voltage_index_to_config_data_size; i++) {
			if (pmgr_binning_voltage_index_to_config_data[i].voltage_index > max_voltage_index) {
				max_voltage_index = pmgr_binning_voltage_index_to_config_data[i].voltage_index;
			}
		}
		for (i = 0; i <= max_voltage_index; i++) {
			pmgr_binning_menu_voltage_config_print(NULL, i);
		}
		for (i = 0; i < pmgr_binning_voltage_config_sram_data_size; i++) {
			pmgr_binning_menu_voltage_config_print(&pmgr_binning_voltage_config_sram_data[i], i);
		}
	}

	if (pmgr_binning_menu_override_get_voltage_indexes(false, voltage_indexes, sizeof(voltage_indexes)/sizeof(voltage_indexes[0]), &voltage_index_found)) {
		uint32_t i;
		for (i = 0; i < voltage_index_found; i++) {
			if (i == 0) {
				printf("\n\nOverridden CPU states:\n");
			}
			printf("    state %u ", i + 3); // state 0 (bypass), 1 (securerom) and 2 (current state) are not overridden
			pmgr_binning_menu_voltage_config_print(NULL, voltage_indexes[i]);
		}
	}
	if (pmgr_binning_menu_override_get_voltage_indexes(true, voltage_indexes, sizeof(voltage_indexes)/sizeof(voltage_indexes[0]), &voltage_index_found)) {
		uint32_t i;
		for (i = 0; i < voltage_index_found; i++) {
			if (i == 0) {
				printf("\n\nOverridden GPU states:\n");
			}
			printf("    state %u ", i + 1); // state 0 (off) are not overridden
			pmgr_binning_menu_voltage_config_print(NULL, voltage_indexes[i]);
		}
	}
	return 0;
}

void pmgr_binning_menu_update_states(void)
{
	uint32_t voltage_indexes[32];
	uint32_t voltage_index_found;

	if (pmgr_binning_menu_override_get_voltage_indexes(false, voltage_indexes, sizeof(voltage_indexes)/sizeof(voltage_indexes[0]), &voltage_index_found)) {
		pmgr_platform_set_perf_state(false, 3, voltage_indexes, voltage_index_found);
	}
	if (pmgr_binning_menu_override_get_voltage_indexes(true, voltage_indexes, sizeof(voltage_indexes)/sizeof(voltage_indexes[0]), &voltage_index_found)) {
		pmgr_platform_set_perf_state(true, 1, voltage_indexes, voltage_index_found);
	}
}

// _command, _function, _help, _meta
MENU_COMMAND_DEVELOPMENT(pmgr, do_pmgr, "Print pmgr data", NULL);

#endif // !RELEASE_BUILD && WITH_MENU
