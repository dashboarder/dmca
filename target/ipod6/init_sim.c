/*
 * Copyright (C) 2013-2014 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */
#include <debug.h>
#include <drivers/flash_nor.h>
#include <lib/devicetree.h>
#include <lib/env.h>
#include <lib/mib.h>
#include <platform.h>
#include <platform/gpiodef.h>
#include <platform/memmap.h>
#include <sys.h>
#include <sys/menu.h>
#include <sys/task.h>
#include <target.h>

bool sim_target_should_recover(void)
{
	return false;
}

bool sim_target_should_poweron(bool *cold_button_boot)
{
	*cold_button_boot = true;
	return true;
}

bool sim_target_should_poweroff(bool at_boot)
{
	return false;
}

#if !PRODUCT_LLB
bool target_get_property(enum target_property prop, void *data, int maxdata, int *retlen)
{
	return false;
}
#endif

#if WITH_ENV

void sim_target_setup_default_environment(void)
{
	env_set("boot-args", "debug=0x14e serial=3 amfi=0x83 cs_enforcement_disable=1 -disable_aslr", ENV_PERSISTENT);
	env_set("boot-device", "asp_nand", 0);
	env_set("display-color-space","RGB888", 0);
	env_set("display-timing", "POR_DISP_B_VID", 0);
 	env_set("idle-off", "false", 0);
//	env_set("kaslr-slide", "0x10000000", 0);
	env_set("kaslr-off", "true", 0);
	env_set("wifiaddr", "12:22:33:44:55:66", 0);
	env_set("ethaddr", "12:22:33:44:55:77", 0);
	env_set("btaddr", "00:00:00:00:00:00", 0);
}

#endif

bool product_target_is_display_in_video_mode(void)
{
	return true;
}

#if WITH_DEVICETREE

#define ROOT_DTPATH		""
int sim_target_update_device_tree(void)
{
	DTNode *node;
	uint32_t propSize;
	char *propName;
	void *propData;

	// Overwrite the serial-number with data from the simplertc fastsim model
	if (FindNode(0, ROOT_DTPATH, &node)) {
		propName = "serial-number";
		if (FindProperty(node, &propName, &propData, &propSize)) {
			if (propSize != 0x20) {
				dprintf(DEBUG_CRITICAL, "serial-number property size 0x%x (expected 0x20)\n", propSize);
				return -1;
			}
			uint64_t *serial = (uint64_t *)propData;
			serial[0] = *(volatile uint64_t *)0x4a000040;
			serial[1] = *(volatile uint64_t *)0x4a000048;
			serial[2] = *(volatile uint64_t *)0x4a000050;
			serial[3] = *(volatile uint64_t *)0x4a000058;
		}
	}

	// Overwrite the region-info with data from the simplertc fastsim model
	if (FindNode(0, ROOT_DTPATH, &node)) {
		propName = "region-info";
		if (FindProperty(node, &propName, &propData, &propSize)) {
			if (propSize != 0x20) {
				dprintf(DEBUG_CRITICAL, "region-info property size 0x%x (expected 0x20)\n", propSize);
				return -1;
			}
			uint64_t *region = (uint64_t *)propData;
			region[0] = *(volatile uint64_t *)0x4a000060;
			region[1] = *(volatile uint64_t *)0x4a000068;
			region[2] = *(volatile uint64_t *)0x4a000070;
			region[3] = *(volatile uint64_t *)0x4a000078;
		}
	}

	if (FindNode(0, "arm-io/spu/iop-spu-nub", &node)) {
		propName = "AAPL,ignore";
		if (FindProperty(node, &propName, &propData, &propSize)) {
			propName[0] = '~';
		}
	}

	return 0;
}

#endif

bool target_do_chargetrap(void)
{
	return false;
}
bool target_needs_chargetrap(void)
{
	return false;
}

#if !WITH_HW_POWER

bool power_needs_precharge(void)
{
	return false;
}
bool power_needs_thermal_trap(void)
{
	return false;
}

void power_cancel_buttonwait(void)
{
}

bool power_do_chargetrap(void)
{
	return false;
}

bool power_is_suspended(void)
{
	return false;
}

void power_will_resume(void)
{
}

bool power_has_usb(void)
{
	return false;
}

int power_read_dock_id(unsigned *id)
{
	return -1;
}

bool power_get_diags_dock(void)
{
	return false;
}

u_int32_t power_get_boot_battery_level(void)
{
	return 0;
}

int power_get_nvram(u_int8_t key, u_int8_t *data)
{
	return -1;
}

int power_set_nvram(u_int8_t key, u_int8_t data)
{
	return -1;
}

int power_set_soc_voltage(unsigned mv, int override)
{
	return -1;
}

bool force_usb_power = false;

void power_set_usb_state(bool configured, bool suspended)
{
}

int power_backlight_enable(u_int32_t backlight_level)
{
	return 0;
}

#endif /* ! WITH_HW_POWER */
