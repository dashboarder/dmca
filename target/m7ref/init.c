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
#include <drivers/display.h>
#if WITH_HW_DISPLAY_PMU
#include <drivers/display_pmu.h>
#endif
#include <drivers/flash_nor.h>
#include <drivers/iic.h>
#include <drivers/power.h>
#include <lib/devicetree.h>
#include <lib/env.h>
#include <lib/mib.h>
#include <lib/paint.h>
#include <lib/syscfg.h>
#include <platform.h>
#include <platform/chipid.h>
#include <platform/gpio.h>
#include <platform/gpiodef.h>
#include <platform/memmap.h>
#include <platform/soc/hwregbase.h>
#include <platform/soc/pmgr.h>
#include <sys.h>
#include <sys/boot.h>
#include <sys/menu.h>
#include <sys/task.h>
#include <target.h>

MIB_CONSTANT(kMIBTargetOsPictureScale,	kOIDTypeUInt32, 2);
MIB_CONSTANT(kMIBTargetPictureRotate,	kOIDTypeInt32,  0);

void target_early_init(void)
{
}

void target_late_init(void)
{
}

void target_init(void)
{
#if WITH_HW_FLASH_NOR
	flash_nor_init(SPI_NOR0);
#endif
}

void target_quiesce_hardware(void)
{
}

void target_poweroff(void)
{
}


int target_bootprep(enum boot_target target)
{
	return 0;
}

bool target_should_recover(void)
{
	return platform_get_request_dfu2() && power_has_usb();
}

bool target_should_poweron(bool *cold_button_boot)
{
#if WITH_HW_POWER
	if (power_get_boot_flag() == kPowerBootFlagColdButton) *cold_button_boot = true;
#else
	*cold_button_boot = false;
#endif // WITH_HW_POWER

	return !*cold_button_boot || platform_get_request_dfu1();
}

bool target_should_poweroff(bool at_boot)
{
	return platform_get_request_dfu1() && (!at_boot || !power_has_usb());
}

#if WITH_ENV

void target_setup_default_environment(void)
{
	env_set("boot-args", " debug=0x14e serial=3 amfi_unrestrict_task_for_pid=1 amfi_allow_any_signature=1 amfi_get_out_of_my_way=1 cs_enforcement_disable=1 no-dockfifo-uart=1", 0);
	env_set("boot-device", "asp_nand", 0);
	env_set("diags-path", "/AppleInternal/Diags/bin/diag.img4", 0);
	env_set("diags-vendor-path", "/AppleInternal/Diags/bin/diag-vendor.img4", 0);
}

#endif // WITH_ENV

#if WITH_DEVICETREE

int target_update_device_tree(void)
{
	return 0;
}

#endif // WITH_DEVICETREE

#if !WITH_HW_POWER

bool power_load_memory_calibration(void *settings, uint32_t settingsSize)
{
	return true;
}
bool power_store_memory_calibration(void *settings, uint32_t settingsSize)
{
	return true;
}

bool power_is_suspended(void)
{
	return false;
}

void power_will_resume(void)
{
}

void power_cancel_buttonwait(void)
{
}

int power_get_nvram(u_int8_t key, u_int8_t *data)
{
	return -1;
}

int power_set_nvram(u_int8_t key, u_int8_t data)
{
	return -1;
}

bool force_usb_power = false;

bool power_has_usb(void)
{
	return false;
}

bool power_get_diags_dock(void)
{
	return false;
}

bool power_needs_precharge(void)
{
	return false;
}

bool power_do_chargetrap(void)
{
	return false;
}

#endif /* ! WITH_HW_POWER */

bool charger_has_external(int dock)
{
	// external charge not supported
	return false;
}

void charger_set_charging(int dock, uint32_t input_current_limit, uint32_t *charge_current_ma)
{
}

