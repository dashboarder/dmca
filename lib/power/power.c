/*
 * Copyright (C) 2008-2013 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */
#include <debug.h>
#include <drivers/iic.h>
#include <drivers/power.h>
#include <drivers/usb/usb_public.h>
#include <drivers/gasgauge.h>
#include <drivers/charger.h>
#include <drivers/tristar.h>
#include <lib/paint.h>
#include <lib/syscfg.h>
#include <platform.h>
#include <platform/gpiodef.h>
#include <platform/miu.h>
#include <target.h>
#include <sys.h>
#include <sys/boot.h>
#include <sys/menu.h>
#include <sys/task.h>
#include <lib/nvram.h>
 
#define POWERCONFIG_CHARGE_TABLE 1
#include <target/powerconfig.h>

#ifndef NUM_DOCKS
#define NUM_DOCKS (1)
#endif

enum {
	kPowerSupplyBattery = 1,
	kPowerSupplyFirewire = 2,
	kPowerSupplyUSBHost = 3,
	kPowerSupplyUSBBrick = 4,
	kPowerSupplyUSBCharger = 5,
	kPowerSupplyExternal = 6,
	kPowerSupplyNum
};

bool force_usb_power = false;

static unsigned int	boot_battery_level = -1U;
static bool		need_precharge;
static bool		need_buttonwait;
static int		power_supply_type[NUM_DOCKS];
static int		usb_brick_id[NUM_DOCKS];
static uint32_t available_charge_current;

static int		battery_delta_target;

static const char * kPowerSupplyNames[kPowerSupplyNum] = { 
	"error", "batt", "firewire", "usb host", "usb brick", "usb charger", "external" };

static bool
power_set_usb_brick_detect(int dock, int select)
{
#if WITH_HW_TRISTAR
    return tristar_set_usb_brick_detect(select);
#elif WITH_HW_CHARGER
    return charger_set_usb_brick_detect(dock, select);
#else
    return platform_set_usb_brick_detect(select);
#endif
}

static unsigned
power_detect_usb_brick(int dock)
{
	static const int dM = 0;
	static const int dP = 1;
	static const int dNum = 2;

	uint32_t dataVoltage[dNum], dataID[dNum], ratio;
	uint32_t brickID;

	usb_brick_id[dock] = 0;

	for (int idx = dM; idx < dNum; idx++)
	{
		uint32_t dMon = (idx == dP) ? kUSB_DP : kUSB_DM;

		power_set_usb_brick_detect(dock, dMon);
		spin(2 * 1000);
		dataVoltage[idx] = pmu_read_brick_id_level();

		// Thresholds based on N20 code, adjusted for 5V scale
		if (dataVoltage[idx] <= 2220) {
		    dataID[idx] = 0;
		} else if (dataVoltage[idx] <= 2890) {
		    dataID[idx] = 1;
		} else {
		    dataID[idx] = 2;
		}

		power_set_usb_brick_detect(dock, kUSB_NONE);
	}

	ratio = (dataVoltage[dP] == 0) ? 0 : (dataVoltage[dM] * 1000 / dataVoltage[dP]);

	// First apply legacy ratio-based detection for 1A and 2A bricks to ensure
	// detection of all bricks correctly identified by previous products.
	if ((ratio > 1290) && (ratio <= 1505))
	{
		// Ratio 1.34
		brickID = 2;
	}
	else if ((ratio > 664) && (ratio <= 775))
	{
		// Ratio 0.75
		brickID = 4;
	}
	else
	{
		// New (N20-based) brick detection method
		brickID = 1 + (3 * dataID[dP]) + dataID[dM];
	}
	
	if (NUM_DOCKS > 1) dprintf(DEBUG_INFO, "dock %d ", dock);
	dprintf(DEBUG_INFO, "usb voltage dP %d, dM %d, ratio %d id %d\n", dataVoltage[dP] , dataVoltage[dM], ratio, brickID);

	if ((dataVoltage[dM] < 1000) || (dataVoltage[dP] < 1000))
	{
	    	if (power_set_usb_brick_detect(dock, kUSB_CP1)) {
			uint32_t dv;

			// must wait 40ms for a CDP to assert DM voltage source
			task_sleep(40 * 1000);

			dv = pmu_read_brick_id_level();

			power_set_usb_brick_detect(dock, kUSB_NONE);
			
			if (NUM_DOCKS > 1) dprintf(DEBUG_INFO, "dock %d ", dock);
			dprintf(DEBUG_INFO, "usb charger detect voltage dM %d\n", dv);

			// if we see 0.25 to 0.8V, it is a USB charger (at this point, we
			// don't try to distinguish DCP from CDP)
			if (dv > 250 && dv < 800) {
			    return kPowerSupplyUSBCharger;
			}
		}
		
		return (kPowerSupplyUSBHost);
	}
		
	if ((ratio < 500) || (ratio > 2000))
	    return (kPowerSupplyUSBHost);

	usb_brick_id[dock] = brickID;
	return kPowerSupplyUSBBrick;
}

static void
power_determine_power_supply(void)
{
	for (int dock = 0; dock < NUM_DOCKS; dock++) {
		int powerSupplyType;

		if (charger_has_external(dock))
		{
		    powerSupplyType = kPowerSupplyExternal;
		}
		else if (charger_has_usb(dock))
		{
		    if (usb_brick_id[dock] != 0) {
			powerSupplyType = kPowerSupplyUSBBrick;
		    } else {
			powerSupplyType = power_detect_usb_brick(dock);
		    }
		}
		else if (charger_has_firewire(dock)) {
			powerSupplyType = kPowerSupplyFirewire;
		}
		else {
		    powerSupplyType = kPowerSupplyBattery;
		    usb_brick_id[dock] = 0;
		}

		if (NUM_DOCKS > 1) printf("dock %d ", dock);
		printf("power supply type %s", kPowerSupplyNames[powerSupplyType] );
		if (powerSupplyType == kPowerSupplyUSBBrick) {
		    printf(" id %d", usb_brick_id[dock] );
		}
		printf("\n");

		if (powerSupplyType != power_supply_type[dock])
		{
			power_supply_type[dock] = powerSupplyType;
			charger_clear_usb_state();
		}
	}
	
#ifdef GPIO_USB_MUX_SEL
	if (NUM_DOCKS > 1)
	{
		bool mux = (power_supply_type[0] != kPowerSupplyUSBHost) && (power_supply_type[1] == kPowerSupplyUSBHost);
		gpio_write(GPIO_USB_MUX_SEL, mux);
	}
#endif
}

// will return charger_get_max_charge_current() unless we have an ATV table.
// not much of a limiting... I guess...
static UInt16
limit_precharge_current(bool charging)
{
	UInt16 charge_current_limit;

	charge_current_limit = 0;
	for (int dock = 0; dock < NUM_DOCKS; dock++) {
	    charge_current_limit += charger_get_max_charge_current(dock);
	}

	if (!charging) {
	    // If the charge rate is too low to need limiting, don't bother.  This avoids needing to
	    // talk to talk to the gas gauge or measure temperature when waking from sleep (7062950)
	    return charge_current_limit;
	}

	if (!power_has_batterypack())
	    return 0;

#if TARGET_USE_CHARGE_TABLE
	// limit charge current in low temperature
	unsigned int milliVolts = power_get_battery_level();
	
	int centiCelsius;
#if WITH_HW_GASGAUGE
	int error = gasgauge_read_temperature(&centiCelsius);
#else
	int error = charger_read_battery_temperature(&centiCelsius);
#endif
	if (!error) {
		// Set charge current to max allowable charge current.
		UInt16 limit = 0;	// Default - suspend. If temp is higher than table can catch
		const struct power_charge_limits *charge_table = pmu_charge_table;
		static const size_t num_charge_limits = sizeof(pmu_charge_table)/sizeof(struct power_charge_limits);

#if WITH_HW_GASGAUGE && TARGET_USE_CHARGE_TABLE
		static struct power_charge_limits gg_charge_table[num_charge_limits];
		static bool chargetable_read = false;
		if (!chargetable_read) {
		    chargetable_read = gasgauge_read_charge_table(gg_charge_table, num_charge_limits);
		}
		if (chargetable_read) {
		    charge_table = gg_charge_table;
		}
#endif

		// Retain last-used charge limit; only switch if no longer applicable
		static const struct power_charge_limits *active_limit = NULL;
		const struct power_charge_limits *tl = charge_table;

		const struct power_charge_limits *new_limit = NULL;
		while (tl < &charge_table[num_charge_limits]) {
			if((milliVolts > tl->upperVoltageLimit) || 
			   (centiCelsius < tl->lowerTempLimit) ||
			   (centiCelsius > tl->upperTempLimit)) {
				tl++;
				continue;
			}
			new_limit = tl;
			break;
		}

		// switch to the new limit unless the voltage range is the same and
		// we are still within the temperature range of the old limit
		if ((new_limit == NULL) || (active_limit == NULL)
		    || (new_limit->upperVoltageLimit != active_limit->upperVoltageLimit)
		    || (centiCelsius < active_limit->lowerTempLimit)
		    || (centiCelsius > active_limit->upperTempLimit))
		{
		    active_limit = new_limit;
		}

		if (active_limit != NULL) {
		    limit = active_limit->currentSetting;
		}

		if (charge_current_limit > limit) {
		    charge_current_limit = limit;
		    dprintf(DEBUG_INFO, "Precharge: Battery temp %dcC voltage %dmV, limiting charge current: %dmA\n", centiCelsius, milliVolts, charge_current_limit);
		}
	}		
#endif

	return charge_current_limit;
}

uint32_t power_get_available_charge_current()
{
	return available_charge_current;
}

#if TARGET_HAS_SMARTPORT
// orion centric BUT it might work with bellatrix too
int smartport_get_data(int dev, uint16_t reg, uint8_t *byte)
{
	const UInt8 addr[1]={ reg&0xff };
	return iic_read(dev, 0xe0, addr, sizeof(addr), byte, 1, IIC_NORMAL);
}

//
int smartport_get_pwr_in_sel(int dev, uint8_t *sel)
{
	int rc=smartport_get_data(dev, 0x16, sel);
	if ( rc<0 ) return rc;

	switch ( *sel&(0x3<<3) ) {
		case 0x10: *sel=SMARTPORT_EXT_PWR_IN_SEL; break; 	// ext
		case 0x08: *sel=SMARTPORT_ACC_PWR_IN_SEL; break; 	// acc
		default: rc=-2; break;	 	// not known
	}

	return rc;
}

#else
int smartport_get_data(int dev, uint16_t reg, uint8_t *byte) { return -1; };
int smartport_get_pwr_in_sel(int dev, uint8_t *sel) { return -1; };
#endif

static bool
power_set_charging(bool critical, bool configured, bool suspended,
		    bool pause)
{
	dprintf(DEBUG_SPEW, "power_set_charging(critical=%d, configured=%d, suspended=%d, pause=%d)\n",
		critical, configured, suspended, pause);

	bool ok = false;

	int max_input_current_limit_dock = 0;
	uint32_t dock_input_current_limit[NUM_DOCKS]={ 0 };

	for (int dock = 0; dock < NUM_DOCKS; dock++)
	{		    

		if ((power_supply_type[dock] == kPowerSupplyUSBHost) && !critical && suspended) {
			dock_input_current_limit[dock] = 0;
		}
		else if (power_supply_type[dock] == kPowerSupplyUSBBrick) {
		    switch (usb_brick_id[dock]) {
		        case 1:
			    dock_input_current_limit[dock] = 500;
			    break;
                
		        case 2:
			    dock_input_current_limit[dock] = 1000;
			    break;
            
		        case 4:
			    dock_input_current_limit[dock] = 2100;
			    break;

		        case 5:
			    dock_input_current_limit[dock] = 2400;
			    break;

		    	default:
			    // reserved brick IDs default to 1A
			    dock_input_current_limit[dock] = 1000;
			    break;
		    }
		}
		else if (power_supply_type[dock] == kPowerSupplyUSBCharger) {
			// limit USB charger to 1A even though spec allows 1500mA (13332167)
			dock_input_current_limit[dock] = 1000;
		}
		else if ((power_supply_type[dock] == kPowerSupplyUSBHost) 
			 && (force_usb_power || configured || critical)) {
		    	dock_input_current_limit[dock] = 500;
		}
		else {
			dock_input_current_limit[dock] = 100;
		}

		if (power_supply_type[dock] >= kPowerSupplyUSBHost) {
		    ok = true;
		}

		if (dock_input_current_limit[dock] > dock_input_current_limit[max_input_current_limit_dock]) {
		    max_input_current_limit_dock = dock;
		}

	}

	available_charge_current=0;

	uint32_t charge_current_limit = 0;
	if ( !pause ) {
		available_charge_current = dock_input_current_limit[max_input_current_limit_dock];
		charge_current_limit = limit_precharge_current( available_charge_current > 100 ); // might not limit at all

#if TARGET_HAS_SMARTPORT
		uint8_t sel;
		int rc=smartport_get_pwr_in_sel(1, &sel);
		if ( rc<0 ) {
		} else if ( sel==SMARTPORT_ACC_PWR_IN_SEL ) { // accessory
			available_charge_current=500; // <rdar://problem/21289146> J99a Orion power-in support for iBoot
		}
#endif
	}
	// charger_set_charging will subtract from charge_current_limit, and we may need to
	// spread the limit across multiple docks.  So start with the one with the highest
	// current limit, since it is most efficient to use the local power.
	for (int i = 0; i < NUM_DOCKS; i++)
	{
		int dock = (i + max_input_current_limit_dock) % NUM_DOCKS;	// start with highest current limit

		// available_charge_current is the MAX unless I have SMARTPORT 
		uint32_t docklimit=dock_input_current_limit[dock];
		if ( (docklimit>available_charge_current) && available_charge_current)  docklimit=available_charge_current;

		charger_set_charging(dock, docklimit, &charge_current_limit);
	}

	return ok;
}

static bool power_charge_done()
{
	if (!power_has_batterypack())
		return false;

#if WITH_HW_GASGAUGE
	return gasgauge_full();
#else
	for (int dock = 0; dock < NUM_DOCKS; dock++) {
	    if ((power_supply_type[dock] >= kPowerSupplyUSBHost) && !charger_charge_done(dock))
		return false;
	}
	
	return true;
#endif
	
}


bool
power_enable_charging(bool inflow, bool charging)
{
	return power_set_charging(need_precharge, false, !inflow, !charging);
}

void
power_set_usb_state(bool configured, bool suspended)
{
	charger_clear_usb_state();
	power_set_charging(false, configured, suspended, false);
}

void power_set_usb_enabled(bool enabled)
{
#if WITH_HW_TRISTAR
	if (enabled) {
	    // if there is a B139/B164 attached, provide it power (11991279)
	    uint8_t digital_id[6];
	    if (tristar_read_id(digital_id)) {
		// if accessory could be a USB host (USB=2 or USB=3) and needs power for USB (PS bit), enable power
		if ((digital_id[0] & (1 << 4)) && (digital_id[2] & (1 << 7))) {
		    tristar_enable_acc_pwr(true);
		}
	    }
	} else {
	    tristar_enable_acc_pwr(false);
	}
#endif
}

// same as gasgauge_needs_precharge(), reset the charger when needed
// @return 1 when needs precharge, 0 when it doesn't
static int charger_needs_precharge(int debug_print_level, bool debug_show_target)
{
	int state;

	state=gasgauge_needs_precharge(debug_print_level, debug_show_target);
	if ( state&GG_COMMS_WD ) { 
		dprintf(debug_print_level, "comms wd\n");

		// TODO: recovery procedure?

	}  else if ( state&GG_NEEDS_RESET ) {
		const int rc=gasgauge_reset_timer(debug_print_level, 8 ); // 8 seconds timeout
		if ( rc!=0 ) dprintf(debug_print_level, "reset charge timer failed (%d)", rc);
	}

	charger_print_status();

	return (state&GG_NEEDS_PRECHARGE)!=0;
}

/* power_init() is only called in iBSS and LLB - not in iBoot or iBEC. */
int 
power_init(void)
{
	u_int8_t data;

	pmu_early_init();

	if (!power_is_suspended()) {
#if WITH_HW_CHARGER
		charger_early_init();
#endif

#if TARGET_POWER_NO_BATTERY
		need_precharge = power_needs_precharge();
#else
		/*
		 * Use two different heurestics to determine whether the unit has a working battery installed:
		 * - no_battery_power: if the voltage is very low, there may be no battery present (or it may be very drained)
		 * - no_battery_pack: if the pack temperature reads very low, there may be no NTC and hence no battery (or it may be very cold)
		 *
		 * In the case where no battery pack is present, we want to avoid either trying to talk
		 * to the gas gauge (which may not be present) or sitting in the battery trap (which would
		 * not be charging a real battery).  There are two different types of no-battery situations, though:
		 * If nothing is connected to the battery terminals, both no_battery_power and no_battery_pack will be
		 * true.  More likely (on a dev board or in the factory) we will have a 4V supply attached to the
		 * battery terminals, but NTC and gas gauge will still be disconnected, so no_battery_power will be
		 * false but no_battery_pack will be true.
		 *
		 * In the latter case, the battery voltage is always well above 3.6V, so only bypass the voltage check
		 * if both indications (no_battery_power and no_battery_pack) are set.  Otherwise, we check
		 * for a low battery voltage.  On a gas gauge-based device, we can therefore assume it's safe to
		 * check with the gas gauge even when no_battery_power is set, if the voltage is in this range (see
		 * below).  On a device with no gas gauge, the voltage check is the only determination of whether
		 * is necessary, so we want to be conservative and only skip a battery voltage that would otherwise
		 * lead to precharge only if both no_battery_power and no_battery_pack are set.
		 *
		 * However, since both of these values can be true even when there is a battery, we try to avoid
		 * making any permanent decisions based on them that will affect the user's ability to use
		 * the unit later.  We also want to avoid a poor user experience for a user that happens to have
		 * a very dead or very cold battery, although if they have both, there's little we can do
		 * to distinguish this from a dev board or factory situation.
		 *
		 * Note that a "very cold" battery that triggers no_battery_pack is always well below the point
		 * at which the battery is too cold to charge safely.  So battery trap would not necessarily
		 * help the user recover anyway (although sitting in the trap can warm up the unit simply by
		 * having the CPU, screen, backlight on).
		 */
		bool		no_battery_power, no_battery_pack;

		no_battery_pack = !power_has_batterypack();
		
		// read boot battery voltage with minimal current being drawn, charger disabled
		power_set_charging(false, false, false, true);
		dprintf(DEBUG_INFO, "battery voltage ");

		if (!charger_read_battery_level(&boot_battery_level))
			dprintf(DEBUG_INFO, "%d mV\n", boot_battery_level);
		else
			dprintf(DEBUG_INFO, "error\n");
		power_set_charging(false, false, false, false);

		no_battery_power = (boot_battery_level < NO_BATTERY_VOLTAGE);
		need_precharge = (!no_battery_power || !no_battery_pack) && (boot_battery_level < MIN_BOOT_BATTERY_VOLTAGE);

#if WITH_HW_GASGAUGE
		/*
		 * If no_battery_pack is set, need_precharge will still be set if
		 * NO_BATTERY_VOLTAGE < boot_battery_level < MIN_BOOT_BATTERY_VOLTAGE.  This means
		 * while a very cold, very dead, battery can still bypass battery trap, a
		 * very cold, mostly dead, battery will not.
		 *
		 * If the voltage falls in this range, assume we have a real battery and talk to
		 * the gas gauge anyway; a fake battery from an external supply should always
		 * be well above MIN_BOOT_BATTERY_VOLTAGE (3.4-3.6V).
		 *
		 * We do not talk to the gas gauge if we're waking from sleep and it's unlikely
		 * the battery is low: it will delay wakeup by a measurable amount, and wiggling
		 * the SWI line will confuse the PMU when it looks for wakeup events.
		 */
		if (!no_battery_pack || need_precharge) {
			gasgauge_init();

			// iOS or charge trap will reset the charge timer if needed
			need_precharge = ( gasgauge_needs_precharge(DEBUG_INFO, true)&GG_NEEDS_PRECHARGE) !=0 ;
		}
#endif

		if (power_get_nvram(kPowerNVRAMiBootDebugKey, &data) == 0 && (data & kPowerNVRAMiBootDebugBatteryTrap) != 0) {
#if TARGET_FORCE_DEBUG_PRECHARGE
		    need_precharge = true;
#elif !RELEASE_BUILD && defined(GPIO_RINGER_AB)
		    if (gpio_read(GPIO_RINGER_AB)) {
			need_precharge = true;
		    }
#endif
		}
            
		power_determine_power_supply();
		power_set_charging(need_precharge || no_battery_power, false, false, no_battery_power && no_battery_pack);
#endif

		// store boot voltage and brick ID for iBoot.  Keep backwards compatible with old
		// LLB that stored boot voltage in mV: mV truncated to cV with brick ID encoded in lowest
		// order decimal digit, encoded in bits [12:0].  Bits [15:14] indicate which dock is
		// in use; 1-indexed with zero meaning none.  Reserved: bit 13 reserved, "brick ID" zero
		UInt16 boot_battery_data = ((boot_battery_level / 10) * 10) & 0x1FFF;
	#if NUM_DOCKS > 3
	#error too many docks
	#endif
		for (int dock = 0; dock < NUM_DOCKS; dock++) {
		    if (power_supply_type[dock] == kPowerSupplyUSBBrick) {
			boot_battery_data |= (dock+1) << 14;
			boot_battery_data += usb_brick_id[dock] % 10;
			break;
		    }
		}

		// pass boot voltage between LLB and iBoot
		power_set_nvram(kPowerNVRAMiBootBootFlags0Key, boot_battery_data & 0xFF);
		power_set_nvram(kPowerNVRAMiBootBootFlags1Key, (boot_battery_data >> 8) & 0xFF);

		// this allow charging via ISET
		charger_clear_alternate_usb_current_limit();
	}

	power_get_nvram(kPowerNVRAMiBootStateKey, &data);
	need_buttonwait = ((data & kPowerNVRAMiBootStateModeMask) == kPowerNVRAMiBootStateModeButtonwaitWithGraphics)
			  || ((data & kPowerNVRAMiBootStateModeMask) == kPowerNVRAMiBootStateModeButtonwaitNoGraphics);
	if (need_precharge)
		data |= kPowerNVRAMiBootStatePrecharge;
	else
		data &= ~kPowerNVRAMiBootStatePrecharge;
	power_set_nvram(kPowerNVRAMiBootStateKey, data);

	dprintf(DEBUG_INFO, "need precharge %d set nvram %02x\n", need_precharge, data);

	pmu_setup();
	
	return 0;
}

int 
power_late_init(void)
{
    pmu_late_init();
#if WITH_HW_GASGAUGE
	gasgauge_late_init();
#endif 

	return 0;
}

bool
power_needs_precharge(void)
{
#if TARGET_POWER_NO_BATTERY
	// since power_init() doesn't get called from regular iBoot, this is "the place" to set up
	// iBoot properly (we call it from power_init if TARGET_POWER_NO_BATTERY). 
	// Make sure we have the right power supply type, and force USB power enabled, as if we were
	// in precharge on a battery-powered device.
	if (-1U == boot_battery_level) {
	    boot_battery_level = 0;
	    force_usb_power = true;
	    power_determine_power_supply();
	    power_set_charging(true, false, false, false);
	}

	return (false);
#elif PRODUCT_IBSS || PRODUCT_IBEC
	return (false);
#elif PRODUCT_LLB
	return (need_precharge);
#else
	if (need_precharge) return (true);

	u_int8_t data = 0;
	power_get_nvram(kPowerNVRAMiBootStateKey, &data);
	need_precharge = (0 != (kPowerNVRAMiBootStatePrecharge & data));
	need_buttonwait = ((data & kPowerNVRAMiBootStateModeMask) == kPowerNVRAMiBootStateModeButtonwaitWithGraphics)
			  || ((data & kPowerNVRAMiBootStateModeMask) == kPowerNVRAMiBootStateModeButtonwaitNoGraphics);
	if (need_precharge || (-1U == boot_battery_level)) {
		bool no_battery_power;

		// see encoding table in power_init
		power_get_nvram(kPowerNVRAMiBootBootFlags1Key, &data);
		int dock = (data & 0xC0) >> 6;
		boot_battery_level = ((data & 0x1F) << 8);
		power_get_nvram(kPowerNVRAMiBootBootFlags0Key, &data);
		if (dock > 0) usb_brick_id[dock-1] = ((boot_battery_level | data) % 10);
		boot_battery_level = ((boot_battery_level | data) / 10) * 10;

		printf("battery voltage %d mV\n", boot_battery_level);

		if ((boot_battery_level + 50) > TARGET_BOOT_BATTERY_VOLTAGE)
		    battery_delta_target = 50;	// charge at least 50 mV
		else
		    battery_delta_target = TARGET_BOOT_BATTERY_VOLTAGE - boot_battery_level;
		no_battery_power = (boot_battery_level < NO_BATTERY_VOLTAGE);

		power_determine_power_supply();
		power_set_charging(need_precharge || no_battery_power, false, false, no_battery_power && !power_has_batterypack());
	}

	return (need_precharge);
#endif
}

bool
power_needs_thermal_trap(void)
{
	u_int8_t data = 0;
	power_get_nvram(kPowerNVRAMiBootStateKey, &data);
	return data & kPowerNVRAMiBootStateThermalTrap;
}

void
power_cancel_buttonwait(void)
{
	if (need_buttonwait) {
		u_int8_t data = 0;
		power_get_nvram(kPowerNVRAMiBootStateKey, &data);
		data &= ~kPowerNVRAMiBootStateModeMask;
		data |= kPowerNVRAMiBootStateModeNormalBoot;
		power_set_nvram(kPowerNVRAMiBootStateKey, data);

		need_buttonwait = false;
	}
}

u_int32_t 
power_get_boot_battery_level(void)
{
	return (boot_battery_level);
}

void
power_get_usb_brick_id(uint32_t *id, size_t count)
{
    for (unsigned dock = 0; dock < NUM_DOCKS && dock < count; dock++) {
	if (power_supply_type[dock] == kPowerSupplyUSBBrick) {
	    id[dock] = usb_brick_id[dock];
	} else {
	    id[dock] = 0;
	}
    }
}

u_int32_t 
power_get_battery_level(void)
{
    uint32_t voltage;

#if WITH_HW_GASGAUGE
    if (gasgauge_read_voltage(&voltage) != 0)
	return 0;
#else
    if (charger_read_battery_level(&voltage) != 0)
	return 0;
#endif

    return voltage;
}

bool
power_has_batterypack(void)
{
	return charger_has_batterypack(0);
}

enum 
{
	kCheckBatteryInterval     = 5 * 1000 * 1000,	// 5s
	kCheckPowerSupplyInterval   = 100 * 1000,			// 100ms
	kChargingAnimationFrameInterval   = 1 * 1000 * 1000,	// 1s
	kHomeButtonPoweronTimeout   = 400 * 1000,	// 400ms
	kPrechargeDisplayDimTimeout = 8 * 1000 * 1000,	// 8s
};

enum chargetrap_type_t
{
	kChargetrapPrecharge = 1,
	kChargetrapButtonwaitWithGraphics = 2,
	kChargetrapButtonwaitNoGraphics = 3,
};

enum chargetrap_return_t
{
	kChargetrapReturnSuccess = 0,		// conditions of precharge were met
	kChargetrapReturnNoCharger = 1,	// no charger attached
	kChargetrapReturnChargerRemoved = 2,	// charger was attached, and then removed
};

// round up when charging (same as SpringBoard)...
static int
power_battery_level(unsigned int capacity, unsigned int total_capacity)
{
	int level = (((IMAGE_TYPE_BATTERYFULL_N_TOTAL-1) * capacity) + (total_capacity-1)) / total_capacity;

	// ... only show full battery if completely full...
	if ((level == IMAGE_TYPE_BATTERYFULL_N_FULL) && (capacity < total_capacity)) level--;

	// ... and never show the "empty" level...
	if (level == 0)  level++;

#if !TARGET_PRECHARGE_ALWAYS_DISPLAY_IDLE
	// if iPhone, always show red battery until 20%
	if ((level > IMAGE_TYPE_BATTERYFULL_N_RED) && ((5 * capacity) <= total_capacity)) level = IMAGE_TYPE_BATTERYFULL_N_RED;
#endif

	return level;
}

static bool
power_paint_battery_with_capacity(bool precharge, int frameCount, unsigned int capacity, unsigned int total_capacity)
{
	bool ok = true;

	static int current_level = -1;

	int level = power_battery_level(capacity, total_capacity);

	dprintf(DEBUG_SPEW, "power_paint_battery: %d capacity %d/%d\n", level, capacity, total_capacity);

	if ((level == current_level) && (frameCount > 0)) {
	    return true;
	}

	paint_set_picture(0);

	bool batteryImage = true;
#if !WITH_HW_GASGAUGE
	batteryImage = precharge || level == IMAGE_TYPE_BATTERYFULL_N_FULL;
#endif
	if (!batteryImage) {
	    if (((frameCount / (kChargingAnimationFrameInterval / kCheckPowerSupplyInterval)) & 1) == 0) {
		ok = !paint_set_picture_for_tag(IMAGE_TYPE_BATTERYCHARGING0);
	    } else {
		ok = !paint_set_picture_for_tag(IMAGE_TYPE_BATTERYCHARGING1);
	    }
	} else {
	    ok = !paint_set_picture_for_tag(IMAGE_TYPE_BATTERYLOW0);
	    if (ok) {
		ok = !paint_set_picture_for_tag(IMAGE_TYPE_BATTERYLOW1);
		if (ok && level >= IMAGE_TYPE_BATTERYFULL_N_START) {
		    ok = !paint_set_picture_for_tag_list(IMAGE_TYPE_BATTERYFULL, 0, 1 + level - IMAGE_TYPE_BATTERYFULL_N_START);
		}
	    }
	}

	if (!ok) {
		// Default to yellow if there are no pictures.
		paint_set_bgcolor(255, 255, 0);
		paint_set_picture(0);
	}

	paint_update_image();

	current_level = level;

	return ok;
}

static bool
power_paint_battery(bool precharge, int frameCount)
{
	unsigned int capacity, total_capacity;
	
	if (precharge) {
	    // no need to ask gas gauge
	    capacity = 0;
	    total_capacity = 1;
	} else {
#if WITH_HW_GASGAUGE
	    total_capacity=100;

	    if (gasgauge_read_soc(&capacity) != 0) {
		// gas gauge error; leave screen as is
		return true;
	    }
#else
	    total_capacity = 1;
	    capacity = power_charge_done() ? 0 : 1;
#endif
	}
	
	return power_paint_battery_with_capacity(precharge, frameCount, capacity, total_capacity);
}


static enum chargetrap_return_t
charger_precharge(enum chargetrap_type_t chargetrap_type)
{
	bool ok;
	int pollCount;
	unsigned startBatteryLevel, newBatteryLevel, lastChargingBatteryLevel;
	utime_t buttonpress_time = 0;
	utime_t display_idle_time;
	int frameCount = 0;
	bool animate, color_map_state;
	const char *chargetrap_label;
	bool test_buttonwait_with_ringer = false;
	uint8_t data;

	if (chargetrap_type != kChargetrapButtonwaitNoGraphics) {
	    power_set_usb_enabled(true);
	}

	// check connection type before starting
	power_determine_power_supply();

	charger_clear_usb_state();

	ok = power_set_charging(true, false, false, false);
	if (!ok) {

#if WITH_HW_GASGAUGE
		// if precharge and no charger, check gas gauge health once anyway
		// if charger is attached, we do this later in a more methodical manner.
		static bool gg_health_checked = false;
		if (!gg_health_checked && (chargetrap_type == kChargetrapPrecharge)) {
			gasgauge_check_health(boot_battery_level);
			gg_health_checked = true;
		}
#endif

		return kChargetrapReturnNoCharger;
	}


#if  !RELEASE_BUILD || TARGET_FORCE_DEBUG_PRECHARGE
	if (power_get_nvram(kPowerNVRAMiBootDebugKey, &data) == 0) {
	    test_buttonwait_with_ringer = (data & kPowerNVRAMiBootDebugBatteryTrap) != 0;
	} else {
		printf("  Error: cannot read kPowerNVRAMiBootDebugKey\n");
	}
#endif


	if ( test_buttonwait_with_ringer ) {
	    chargetrap_label = "debug precharge";
	} else if (chargetrap_type == kChargetrapPrecharge) {
	    chargetrap_label = "low-battery precharge";
	} else {
	    chargetrap_label = "power-off simulation";
	}


	charger_read_battery_level(&startBatteryLevel);

	printf("\nStarting %s: ", chargetrap_label);
	if (chargetrap_type == kChargetrapPrecharge) {
#if WITH_HW_GASGAUGE
		charger_needs_precharge(DEBUG_CRITICAL, true);	// print target and current level
#else
		printf("target %d boot %d, now %d mV\n", startBatteryLevel + battery_delta_target, boot_battery_level, startBatteryLevel);
#endif
	} else {
		printf("waiting for power button or unplug (screen %s)\n",
		       (chargetrap_type == kChargetrapButtonwaitWithGraphics) ? "on" : "off");
	}

	lastChargingBatteryLevel = newBatteryLevel = startBatteryLevel;
	pollCount = 0;
	power_set_charging(true, false, false, false);

	animate = (chargetrap_type != kChargetrapButtonwaitNoGraphics);
	display_idle_time = system_time();

	do {
		bool powersupply_changed = false;
		bool button_event = false;
		bool other_wake_event = false;

		if (need_buttonwait) {
		    bool button = platform_get_request_dfu1();

		    if (button && (buttonpress_time == 0))
			buttonpress_time = system_time();
		    else if (!button)
			buttonpress_time = 0;
		    
		    if ((buttonpress_time != 0) && time_has_elapsed(buttonpress_time, kHomeButtonPoweronTimeout - kCheckPowerSupplyInterval)) {
			printf("power button press detected: button wait cancelled\n");
			need_buttonwait = false;
			if ((chargetrap_type == kChargetrapButtonwaitWithGraphics) || (chargetrap_type == kChargetrapButtonwaitNoGraphics))
			    break;
		    }
		}

		// turn on the display if needed
		if (animate && (frameCount == 0)) {
			platform_init_display();
			color_map_state = paint_color_map_enable(false);
			ok = power_paint_battery(chargetrap_type == kChargetrapPrecharge, frameCount);
			if (!ok) {
				animate = false;
			}
		}

		task_sleep(kCheckPowerSupplyInterval);

		if (animate && ((++frameCount % (kChargingAnimationFrameInterval / kCheckPowerSupplyInterval)) == 0)) {
			if (chargetrap_type == kChargetrapButtonwaitWithGraphics) {
				power_paint_battery(false, frameCount);
			}

			// Turn off the screen after a timeout with no user activity.
			// If the display is too large to sustain being on at 500mA, we always idle off, but otherwise
			// if graphical button-wait is enabled we do not, even in precharge, because the graphical
			// button-wait mode is defined as leaving the screen on at all times.
#if TARGET_PRECHARGE_ALWAYS_DISPLAY_IDLE
			if (true) {
#else
			if ((chargetrap_type == kChargetrapPrecharge) && !need_buttonwait) {
#endif
				if (system_time() >= (display_idle_time + kPrechargeDisplayDimTimeout)) {
					platform_quiesce_display();
					animate = false;
					frameCount = 0;
				}
			}
		}

		/* Read & clear interrupts */
		pmu_check_events(&powersupply_changed, &button_event, &other_wake_event);
#if WITH_HW_CHARGER
		powersupply_changed = false;
		for (int dock = 0; dock < NUM_DOCKS; dock++) {
			if (charger_check_usb_change(dock, power_supply_type[dock] >= kPowerSupplyUSBHost))
				powersupply_changed = true;
		}
#endif

		if (++pollCount > (kCheckBatteryInterval / kCheckPowerSupplyInterval)) {
			pollCount = 0;
			dprintf(DEBUG_INFO, "%s: ", chargetrap_label);

			if (charger_read_battery_level(&newBatteryLevel) != 0)
			    continue;

#if WITH_HW_GASGAUGE
			need_precharge = charger_needs_precharge(DEBUG_INFO, (chargetrap_type == kChargetrapPrecharge));
#else
			if (chargetrap_type == kChargetrapPrecharge) {
			    dprintf(DEBUG_INFO, "battery level %d mV (target %d mV)\n", newBatteryLevel, startBatteryLevel + battery_delta_target);
			} else {
			    dprintf(DEBUG_INFO, "waiting for power button or unplug (battery level %d mV)\n", newBatteryLevel);
			}

			if (need_precharge 
			    && ((newBatteryLevel >= (startBatteryLevel + battery_delta_target))
				|| (newBatteryLevel >= (ALWAYS_BOOT_BATTERY_VOLTAGE)))) {
				need_precharge = false;
				boot_battery_level += (newBatteryLevel - startBatteryLevel);
			}
#endif

			if (test_buttonwait_with_ringer) {
#if TARGET_FORCE_DEBUG_PRECHARGE
			    need_precharge = true;
#elif !RELEASE_BUILD && defined(GPIO_RINGER_AB)
			    if (gpio_read(GPIO_RINGER_AB)) {
				need_precharge = true;
			    }
#endif
			}

			if ((chargetrap_type == kChargetrapPrecharge) && !need_precharge) {
			    break;
			}

#if WITH_HW_GASGAUGE
			if (need_precharge) {
			    gasgauge_check_health(newBatteryLevel);
			}
#endif

			// always reset the charge control in case the current limit has changed due to temperature or voltage
			if (!power_set_charging(true, false, false, false)) {
			    powersupply_changed = true;
			}
		}

		if (powersupply_changed)
		{
			power_determine_power_supply();
			ok = power_set_charging(true, false, false, false);
			if (!ok) {
				boot_battery_level   += (lastChargingBatteryLevel - startBatteryLevel);
				battery_delta_target -= (lastChargingBatteryLevel - startBatteryLevel);
				power_set_charging(true, false, false, false);
				if (animate) paint_color_map_enable(color_map_state);
				return kChargetrapReturnChargerRemoved;
			}
		}
		else if ((chargetrap_type == kChargetrapButtonwaitNoGraphics) && other_wake_event)
		{
			// if we are simulating off (kChargetrapButtonwaitNoGraphics) and got an event
			// that would have woken a real unit from standby (excluding buttons, which are special),
			// boot.  This ensures that, e.g., an accessory (7020690) or alarm will turn on the unit.
			need_buttonwait = false;
			break;
		}
		else if ((chargetrap_type != kChargetrapButtonwaitNoGraphics) && button_event)
		{
			// any button event will reset the display-off timer in precharge,
			// and turn the screen back off if already dim.
			display_idle_time = system_time();
			animate = true; // next loop will re-activate the screen
		}

		lastChargingBatteryLevel = newBatteryLevel;
	}
	while (true);

	power_set_charging(true, false, false, false);

	data = 0;
	power_get_nvram(kPowerNVRAMiBootStateKey, &data);
	if (!need_precharge)
	    data &= ~kPowerNVRAMiBootStatePrecharge;
	if (!need_buttonwait) {
	    data &= ~kPowerNVRAMiBootStateModeMask;
	    data |= kPowerNVRAMiBootStateModeNormalBoot;
	}
	power_set_nvram(kPowerNVRAMiBootStateKey, data);

	if (animate) paint_color_map_enable(color_map_state);

	return kChargetrapReturnSuccess;
}

bool
power_do_chargetrap(void)
{
	enum chargetrap_return_t ret;
	uint8_t buttonwait_type = 0;
	unsigned int stage;

	// There are three different types of charge traps.  If more than one applies, show them in this order:
	//
	// 1. button wait without graphics, used to simulate being completely powered off (screen off)
	// 2. precharge (aka battery trap), used when we don't have enough charge to boot (screen displaying low-battery icon)
	// 3. button wait with graphics, used when "off" on certain SKUs (screen displaying charging icon)
	//
	// These are gated by need_buttonwait and power_needs_precharge(), which can be relied on to only ever go
	// from true to false.

	if (need_buttonwait) {
		u_int8_t data = 0;
		power_get_nvram(kPowerNVRAMiBootStateKey, &data);
		
		buttonwait_type = (data & kPowerNVRAMiBootStateModeMask);
	}

	if (need_buttonwait && (buttonwait_type == kPowerNVRAMiBootStateModeButtonwaitNoGraphics)) {
		ret = charger_precharge(kChargetrapButtonwaitNoGraphics);
		if (ret != kChargetrapReturnSuccess) {
			// this mode is one-time; the next boot should always be normal
			power_cancel_buttonwait();
			platform_poweroff();
		}
	}

	stage = 0;
	while (power_needs_precharge() && (stage < 5)) {
		bool ok;

		ret = charger_precharge(kChargetrapPrecharge);
		if (ret != kChargetrapReturnNoCharger) {
		    stage = 0;	// if any charging occurred, restart battery trap sequence
		    continue;
		}

		platform_init_display();
		bool color_map_state = paint_color_map_enable(false);
		paint_set_picture(0);

		ok = !paint_set_picture_for_tag(IMAGE_TYPE_BATTERYLOW0);
		if (ok && !(stage++ & 1)) ok = !paint_set_picture_for_tag(IMAGE_TYPE_BATTERYLOW1);
		if (ok &&
		    ((power_supply_type[0] == kPowerSupplyFirewire) ||
		     (paint_set_picture_for_tag(IMAGE_TYPE_GLYPHPLUGIN) == 0))) {
				paint_set_bgcolor(0, 0, 0);
		} else {
			paint_set_picture(0);
		}
		paint_update_image();

		task_sleep(1 * 1000 * 1000);

		paint_color_map_enable(color_map_state);
	}

	if (power_needs_precharge()) {
		platform_quiesce_display();
		printf("need battery charge, no power source; shutting down\n");
		power_shutdown();
	}

	if (need_buttonwait && (buttonwait_type == kPowerNVRAMiBootStateModeButtonwaitWithGraphics)) {
		ret = charger_precharge(kChargetrapButtonwaitWithGraphics);
		if (ret != kChargetrapReturnSuccess) {
			platform_poweroff();
		}
	}

	power_set_usb_enabled(false);

	return (true);
}

static int
do_charge_cmd(int argc, struct cmd_arg *args)
{
	unsigned battery_level;
	bool enable = true;

	if (!security_allow_modes(kSecurityModeDebugCmd | kSecurityModeHWAccess)) {
		printf("Permission Denied\n");
		return -1;
	}

#if WITH_HW_USB
	usb_quiesce();
#endif

	if (argc >= 2) {
	    if (!strcmp("dis", args[1].str)) {
			enable = false;
	    }
	    else if (!strcmp("ena", args[1].str)) {
	    }
	    else if (!strcmp("trap", args[1].str)) {
	    }
	    else if (!strcmp("restart", args[1].str)) {
#if WITH_HW_GASGAUGE
			int result;

			result=gasgauge_reset_timer( DEBUG_CRITICAL, 8 ); // 8 seconds timeout
			if ( result==0 ) {
				result=gasgauge_needs_precharge(DEBUG_INFO, false);
				if ( result&GG_NEEDS_RESET ) printf("restart failed %#x\n", result);
			} else {
				printf("failed (%d)\n", result);
			}
#endif
			return 0;
	    }
	    else if (!strcmp("detect", args[1].str)) {
			for (int dock = 0; dock < NUM_DOCKS; dock++) usb_brick_id[dock]=0;
			power_determine_power_supply();
			return 0;
	    }
	    else if (!strcmp("draw", args[1].str)) {
	    }
	    else if (!strcmp("status", args[1].str)) {
#if WITH_HW_GASGAUGE
			gasgauge_print_status();
			charger_print_status();
#endif
			return 0;
	    }
	   else {
			printf("usage:\n\t%s <command>\n", args[0].str);
			printf("\t\tena                 Enable charger.\n");
			printf("\t\tdis                 Disable charger.\n");
			printf("\t\trestart             Reset/Restart Charge timer.\n");
			printf("\t\ttrap <delta>        Enter trap with target voltage delta.\n");
			printf("\t\tdraw <soc>          Draw battery logo with specified SoC (percent)\n");
			printf("\t\tdetect          	Force detect power brick\n");
			printf("\t\tstatus              Dump gas gauge and charger status\n");
		}
	}

	power_determine_power_supply();
#if TARGET_HAS_SMARTPORT
	uint8_t pwr_in_sel;
	int rc=smartport_get_pwr_in_sel(1, &pwr_in_sel);
	printf("smartport: pwr_in_sel=%x ", pwr_in_sel);
	if ( rc<0 ) {
		printf("error %d\n", rc);
	} else {
		printf("(%s)\n", (pwr_in_sel==SMARTPORT_ACC_PWR_IN_SEL)?"acc":"ext");
	}
#endif

	power_set_charging(false, enable, false, !enable);

	/* read battery voltage */
	printf("battery voltage ");
	if (!charger_read_battery_level(&battery_level))
	    printf("%d mV\n", battery_level);
	else
	    printf("error\n");

#if WITH_HW_GASGAUGE
	gasgauge_print_status();
	charger_print_status();
#endif

	if ((argc >= 2) && !strcmp("trap", args[1].str)) {
		boot_battery_level = battery_level;
		battery_delta_target = args[2].u;

		enum chargetrap_return_t ret = charger_precharge(kChargetrapPrecharge);
		if (ret == kChargetrapReturnSuccess) printf("charger_precharge success\n");
		else if (ret == kChargetrapReturnNoCharger) printf("charger_precharge no charger\n");
		else if (ret == kChargetrapReturnChargerRemoved) printf("charger_precharge charger removed\n");
	}

	if ( strcmp("draw", args[1].str)==0 ) {
		if ( argc==3 ) {
			// charger draw <soc>
			power_paint_battery_with_capacity(false, 0, args[2].u, 100);
		} else if ( argc==4 ) {
			// charger draw <precharge> <framecount>
			power_paint_battery(args[2].u, args[3].u);
		} else if ( argc>4 ) {
			// charger draw <precharge> <framecount> <soc>
			power_paint_battery_with_capacity(args[2].u, args[3].u, args[4].u, 100);
		}
	}

#if WITH_HW_USB
	/* re-initializes usb as a result of the usb_quiesce() call */
	usb_early_init();
	#if WITH_PLATFORM_INIT_USB
		if(platform_init_usb() != 0)
			usb_quiesce();
	#else
		if(usb_init() != 0)
			usb_quiesce();
	#endif
#endif

	return 0;
}

#if WITH_RECOVERY_MODE
MENU_COMMAND_DEBUG(charge, do_charge_cmd, "Manage the charger chip", NULL);
#endif

static const char *nvram_property_names[kPowerNVRAMPropertyCount] = {
	kPowerNVRAMiBootStateName,					// kPowerNVRAMiBootStateKey
	kPowerNVRAMiBootDebugName,					// kPowerNVRAMiBootDebugKey
	kPowerNVRAMiBootStageName,					// kPowerNVRAMiBootStageKey
	kPowerNVRAMiBootErrorCountName,					// kPowerNVRAMiBootErrorCountKey
	kPowerNVRAMiBootErrorStageName,					// kPowerNVRAMiBootErrorStageKey
	kPowerNVRAMiBootMemCalCAOffset0Name,
	kPowerNVRAMiBootMemCalCAOffset1Name,
	kPowerNVRAMiBootMemCalCAOffset2Name,
	kPowerNVRAMiBootMemCalCAOffset3Name,
	kPowerNVRAMiBootBootFlags0Name,
	kPowerNVRAMiBootBootFlags1Name,
	kPowerNVRAMiBootEnterDFUName,					// kPowerNVRAMiBootEnterDFUKey
};

static int
do_power_nvram(int argc, struct cmd_arg *args)
{
	uint32_t	cnt;
	uint8_t		data;
	
	if (!security_allow_modes(kSecurityModeDebugCmd)) {
		printf("Permission Denied\n");
		return -1;
	}
	
	if (argc < 2) {
		goto usage;
	}
	
	if	  (!strcmp("list", args[1].str)) {
		for (cnt = 0; cnt < kPowerNVRAMPropertyCount; cnt++) {
			if (power_get_nvram(cnt, &data) == 0) {
				printf("%x: [%s] 0x%02x\n", cnt, nvram_property_names[cnt], data);
			}
		}
		
	} else if (!strcmp("set", args[1].str)) {
		if (argc != 4) {
			goto usage;
		}
		
		switch (args[2].u) {
			case kPowerNVRAMiBootStateKey :
				if (!security_allow_modes(kSecurityModeHWAccess)) {
					printf("Permission Denied\n");
			                return -1;
				}
				break;
			
			default:
				break;
		}
		
		power_set_nvram(args[2].u, args[3].u);
	
	} else {
		goto usage;
	}
	
	return 0;
	
usage:
	printf("usage:\n\t%s <command>\n", args[0].str);
	printf("\t\tlist                  List NVRAM Properties.\n");
	printf("\t\tset <prop#> <value>   Set NVRAM Property.\n");
	return -1;
}

#if WITH_RECOVERY_MODE
MENU_COMMAND_DEBUG(powernvram, do_power_nvram, "Access Power NVRAM.", NULL);
#endif

void
power_suspend(void)
{
	platform_quiesce_display();

#if WITH_HW_MIU
	miu_suspend();
#endif
	pmu_set_backlight_enable(0);

	/* enter hibernate mode */
	dprintf(DEBUG_INFO, "pmu suspend\n");

	pmu_suspend();
}

void 
power_shutdown(void)
{
	platform_quiesce_display();

	/* enter standby mode */
	dprintf(DEBUG_INFO, "pmu shutdown\n");

#if WITH_HW_GASGAUGE
	gasgauge_will_shutdown();
#endif

	power_clr_events(1);

	pmu_shutdown();
}

int 
_power_backlight_enable_internal(uint32_t backlight_level)
{
	if (backlight_level) {
		if (power_needs_precharge() && (backlight_level > PRECHARGE_BACKLIGHT_LEVEL))
			backlight_level = PRECHARGE_BACKLIGHT_LEVEL;

	}

#if WITH_HW_LM3534
	extern int lm3534_backlight_enable(uint32_t backlight_level);

	if (lm3534_backlight_enable(backlight_level) != 0)
#endif
#if WITH_HW_LP8559
	extern int lp8559_backlight_enable(uint32_t backlight_level);

	if (lp8559_backlight_enable(backlight_level) != 0)
#endif
	{
		pmu_set_backlight_enable(backlight_level);
	}

	return 0;
}

void
power_clear_dark_boot_flag(void)
{
#if PRODUCT_IBOOT
	const char* boot_cmd = env_get("boot-command");

	if (env_get_bool("auto-boot", false) && boot_cmd && strncmp(boot_cmd, "upgrade", 7) == 0) {
		// If this is iBoot and we are going to upgrade,
		// we don't want to clear dark boot (because iBEC needs to know)
		return;
	}
#endif
	env_unset("darkboot");
	nvram_save();	
}


static bool force_disable_dark_boot;

bool
power_is_dark_boot (void)
{
	static bool dark_boot_init = false;
	static bool is_dark_boot = false;

	if (dark_boot_init)
		return is_dark_boot && !force_disable_dark_boot;
	else {
		is_dark_boot = env_get_bool("darkboot", false);
		dark_boot_init = true;
		return is_dark_boot && !force_disable_dark_boot;
	}
}

static uint32_t requested_backlight_level = 0;

void
power_dark_boot_checkpoint (void)
{
	if (!power_is_dark_boot())
		return;
	// Check buttons and light up screen if needed
	bool powersupply_changed = false;
	bool button_event = false;
	bool other_wake_event = false;

	pmu_check_events(&powersupply_changed, &button_event, &other_wake_event);

	if (button_event) {
		dprintf(DEBUG_INFO, "dark boot: found button event\n");
		power_disable_dark_boot();
		return;
	}
	dprintf(DEBUG_INFO, "dark boot: checkpoint found no pending button presses -- staying dark.\n");
}

void
power_disable_dark_boot (void)
{
	force_disable_dark_boot = true;
	dprintf(DEBUG_INFO, "dark boot: Lighting up display... (remembered backlight_level: %u)\n", (unsigned int)requested_backlight_level);
	_power_backlight_enable_internal(requested_backlight_level);
}

int
power_backlight_enable(uint32_t backlight_level)
{
#if !NO_DARK_BOOT
	dprintf(DEBUG_INFO, "dark boot: Received backlight level request %u\n", (unsigned int)backlight_level);
	requested_backlight_level = backlight_level;

	if (power_is_dark_boot()) {
		dprintf(DEBUG_INFO, "dark boot: This is a dark boot, remembering request but leaving backlight_level = 0\n");
		_power_backlight_enable_internal(0);
	} else {
		return _power_backlight_enable_internal(backlight_level);
	}
	return 0;
#else
	return _power_backlight_enable_internal(backlight_level);
#endif
}

static int
do_backlight_cmd(int argc, struct cmd_arg *args)
{
	if (argc != 2) {
		printf("usage: backlight <level>\n");
		return -1;
	}

	if (power_is_dark_boot()) {
		printf("[disabling dark boot due to explicit backlight command]\n");
		// This will momentarily set the backlight to a higher level then back to this level
		power_disable_dark_boot();
	}

	return power_backlight_enable(args[1].n);
}

#if WITH_MENU
MENU_COMMAND_DEBUG(backlight, do_backlight_cmd, "set the backlight", NULL);
#endif

bool 
power_has_usb(void)
{
	for (int dock = 0; dock < NUM_DOCKS; dock++) {
	    if (charger_has_usb(dock)) {
		return true;
	    }
	}
	
	return false;
}

bool 
power_is_suspended(void)
{
	bool suspended = false;
#if WITH_CLASSIC_SUSPEND_TO_RAM
#if PRODUCT_LLB
	bool resume_lost = power_get_boot_flag() != kPowerBootFlagWarm;
#else
	bool resume_lost = false;
#endif
	u_int8_t data = 0;
	power_get_nvram(kPowerNVRAMiBootStateKey, &data);
	suspended = (data & kPowerNVRAMiBootStateModeMask) == kPowerNVRAMiBootStateModeSuspended;
	if (suspended) {
	    if (resume_lost) {
			data &= ~kPowerNVRAMiBootStateModeMask;
			data |= kPowerNVRAMiBootStateModeNormalBoot;
			power_set_nvram(kPowerNVRAMiBootStateKey, data);
			suspended = false;
		}
	}
#endif	// WITH_CLASSIC_SUSPEND_TO_RAM
	return suspended;
}

void 
power_will_resume(void)
{
#if WITH_CLASSIC_SUSPEND_TO_RAM
	u_int8_t data = 0;
	power_get_nvram(kPowerNVRAMiBootStateKey, &data);
	data &= ~kPowerNVRAMiBootStateModeMask;
	data |=  kPowerNVRAMiBootStateModeResumed;
	power_set_nvram(kPowerNVRAMiBootStateKey, data);

	pmu_will_resume();
#endif	// WITH_CLASSIC_SUSPEND_TO_RAM
}

bool
power_get_diags_dock(void)
{
#if WITH_HW_TRISTAR
    // check the third byte of ID response for DI (diagnostics mode) bit
    uint8_t digital_id[6];
    if (!tristar_read_id(digital_id)) return false;
    return digital_id[2] & (1 << 4);
#else
    unsigned id;
    return !power_read_dock_id(&id) && (id == 0x07);
#endif
}
