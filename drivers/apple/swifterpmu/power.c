/*
 * Copyright (C) 2013 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */

#include <debug.h>
#include <sys.h>
#include <drivers/power.h>
#include <target/powerconfig.h>

#include "power.h"

static uint32_t boot_flags;

static void write_pmu(uint32_t offset, uint32_t data)
{
	volatile uint32_t *addr = (uint32_t *)(SWIFTER_PMU_BASE + offset);
	*addr = data;
}

static uint32_t read_pmu(uint32_t offset)
{
	volatile uint32_t *addr = (uint32_t *)(SWIFTER_PMU_BASE + offset);
	return *addr;
}

bool charger_has_usb(int dock)
{
	return false;
}

bool charger_has_firewire(int dock)
{
	return false;
}

bool charger_has_external(int dock)
{
    	return false;
}

bool charger_has_batterypack(int dock)
{
	return false;
}

void charger_clear_usb_state(void)
{
}

uint32_t charger_get_max_charge_current(int dock)
{
	return 1500;
}

void charger_set_charging(int dock, uint32_t input_current_limit, uint32_t *charge_current_ma)
{
}

void charger_clear_alternate_usb_current_limit(void)
{
}

int charger_read_battery_level(uint32_t *milliVolts)
{
	return 5000;
}

bool charger_charge_done(int dock)
{
	return true;
}

void power_clr_events(int wake)
{
}

u_int32_t pmu_read_brick_id_level(void)
{
	return 5000;
}

int power_get_rail_value(int rail, unsigned mv, u_int32_t *buffer)
{
	return -1;
}

int power_get_buck_value(int buck, uint32_t mv, uint32_t *val)
{
	return -1;
}

int power_convert_dwi_to_mv(int buck, u_int32_t dwival)
{
	return -1;
}	

void pmu_early_init(void)
{
	// Latch the power-on event(s)
	boot_flags = read_pmu(SWIFTER_PMU_EVENT_REG);
	// [or could we just let the OS driver recover them?]
	dprintf(DEBUG_SPEW, "swifterpmu: boot_flags = %x\n", boot_flags);
}

void pmu_setup(void)
{
	// Probably nothing to do for simulation
}

void pmu_late_init(void)
{
	// Probably nothing to do for simulation
}

void pmu_check_events(bool *powersupply_change_event, bool *button_event, bool *other_wake_event)
{
	*powersupply_change_event = false;
	*button_event = false;
	*other_wake_event = false;
}

int power_get_boot_flag(void)
{
	return (boot_flags & SWIFTER_PMU_EVENT_HIB) ? kPowerBootFlagWarm : kPowerBootFlagCold;
}

void pmu_will_resume(void)
{
	// store boot flag for OS use (re-use same NVRAM byte used for voltage between LLB and iBoot)
	power_set_nvram(kPowerNVRAMiBootBootFlags0Key, boot_flags);
}

int power_read_dock_id(unsigned *id)
{
	return -1;
}

int power_set_soc_voltage(unsigned mv, int override)
{
	return -1;
}

void pmu_set_backlight_enable(uint32_t backlight_level)
{
}

int power_get_nvram(u_int8_t key, u_int8_t *data)
{
	if (key >= kPowerNVRAMPropertyCount) return -1;
	
	*data = read_pmu(SWIFTER_PMU_NVRAM_START + (key*4)) & 0xff;
	return 0;
}

int power_set_nvram(u_int8_t key, u_int8_t data)
{
	if (key >= kPowerNVRAMPropertyCount) return -1;
	
	write_pmu(SWIFTER_PMU_NVRAM_START + (key*4), data);
	return 0;
}

void pmu_shutdown(void)
{
	// Write the "shutdown" register
	write_pmu(SWIFTER_PMU_SHUTDOWN_REG, 1);
}

void pmu_suspend(void)
{
	// Write the "suspend" register
	write_pmu(SWIFTER_PMU_SUSPEND_REG, 1);
}

utime_t power_get_calendar_time(void)
{
	return 0;
}

void charger_print_status(void)
{

}
