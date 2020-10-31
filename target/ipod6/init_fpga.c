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

bool fpga_target_should_recover(void)
{
	return false;
}

bool fpga_target_should_poweron(bool *cold_button_boot)
{
	*cold_button_boot = true;
	return true;
}

bool fpga_target_should_poweroff(bool at_boot)
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

void fpga_target_setup_default_environment(void)
{
	env_set("boot-args", " debug=0x14e serial=3 amfi_unrestrict_task_for_pid=1 amfi_allow_any_signature=1 amfi_get_out_of_my_way=1 cs_enforcement_disable=1 no-dockfifo-uart=1", 0);
	env_set("boot-device", "asp_nand", 0);
	env_set("display-color-space","RGB888", 0);
	env_set("display-timing", "fpga-wsvga", 0);	/* defaults to no display support */
 	env_set("ramdisk-delay", "180000000", 0); 	/* This tells PurpleRestore to slow down; rdar://6345846 */
 	env_set("idle-off", "false", 0);
	env_set("bootdelay", "3", 0);
	env_set("debug-uarts", "3", 0);
}

#endif

#if WITH_DEVICETREE

int fpga_target_update_device_tree(void)
{
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

bool power_needs_thermal_trap(void)
{
	return false;
}

bool power_needs_precharge(void)
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
