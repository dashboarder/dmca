/*
 * Copyright (C) 2011-2012, 2015 Apple Inc. All rights reserved.
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
#include <drivers/power.h>
#include <lib/env.h>
#include <lib/mib.h>
#include <platform.h>
#include <platform/gpio.h>
#include <platform/gpiodef.h>
#include <platform/memmap.h>
#include <sys.h>
#include <sys/boot.h>
#include <sys/menu.h>
#include <sys/task.h>
#include <target.h>

MIB_CONSTANT(kMIBTargetAcceptGlobalTicket, kOIDTypeBoolean, true);

void target_init(void)
{
#if WITH_HW_FLASH_NOR
	flash_nor_init(SPI_NOR0);
#endif

#if !RELEASE_BUILD
	// <rdar://problem/10967375> API for B137 test harness to control G1 CLK0_OUT state
	gpio_configure(GPIO_CLK0_OUT, GPIO_CFG_FUNC0);
#endif
}

void target_poweroff(void)
{
}

int target_bootprep(enum boot_target target)
{
	switch (target) {
		case BOOT_DARWIN: {
			break;
		}

		default:
			; // do nothing
	}

	return 0;
}

bool target_should_recover(void)
{
	return 0;
}

bool target_should_poweron(bool *cold_button_boot)
{
	return 1;
}

bool target_should_poweroff(bool at_boot)
{
	return 0;
}

#if !PRODUCT_LLB
bool target_get_property(enum target_property prop, void *data, int maxdata, int *retlen)
{
	return false;
}
#endif

#if WITH_ENV

void target_setup_default_environment(void)
{
#if DEVELOPMENT_BUILD || DEBUG_BUILD
	env_set("boot-args", "debug=0x14e serial=3 rd=md0", 0);
	env_set("debug-uarts", "3", 0);
#else
	env_set("boot-args", "rd=md0", 0);
#endif
	env_set("auto-boot", "false", 0);
	env_set("idle-off", "false", 0);
}

#endif // WITH_ENV

// Dummy function to make LIBENV happy.
int syscfgCopyDataForTag(u_int32_t tag, u_int8_t *buffer, size_t size) {
#pragma unused ( tag, buffer, size )
    return -1;
}

#if !WITH_HW_POWER

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
