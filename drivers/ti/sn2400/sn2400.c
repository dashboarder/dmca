/*
 * Copyright (C) 2007-2008 Apple Inc. All rights reserved.
 * Copyright (C) 2006 Apple Computer, Inc. All rights reserved.
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
#include <drivers/charger.h>
#include <drivers/power.h>
#include <platform.h>
#include <platform/gpiodef.h>
#include <sys.h>
#include <sys/boot.h>
#include <sys/menu.h>
#include <sys/task.h>
#include <target/powerconfig.h>

#define SN2400_ADDR_R				0xEA
#define SN2400_ADDR_W				0xEB

static int charger_reset_battery_protection(void);

enum {
	FAULT_LOG				= 0x00,
	    FAULT_LOG_BAT_SHORT			= (1 << 0),
	
	IRQ_1_MASK				= 0x01,
	IRQ_2_MASK				= 0x02,
	IRQ_3_MASK				= 0x03,

	EVENT_count				= 3,
	
	EVENT_1					= 0x04,
	    EVENT_1_VBUS_OV			= (1 << 7),
	    EVENT_1_VBUS_UV			= (1 << 6),
	    EVENT_1_VBUS_OC			= (1 << 5),
	    EVENT_1_OVER_TEMP			= (1 << 4),
	    EVENT_1_TEMP_WARNING		= (1 << 3),
	    EVENT_1_BAT_SHORT			= (1 << 2),
	    EVENT_1_MAIN_SHORT			= (1 << 1),
	    EVENT_1_RECOVERY_TIMEOUT		= (1 << 0),
	
	EVENT_2					= 0x05,
	    EVENT_2_LDO_OCP			= (1 << 7),
	    EVENT_2_BATT_ALERT			= (1 << 6),
	    EVENT_2_MISSING_BATTERY		= (1 << 5),
	    EVENT_2_HDQ_HOST_ALERT		= (1 << 4),
	    EVENT_2_VBUS_VLIM_ACTIVE		= (1 << 3),
	    EVENT_2_ACCUM_OVERFLOW		= (1 << 2),
	    EVENT_2_ADC_DONE			= (1 << 1),
	    EVENT_2_HDQ_INT			= (1 << 0),
	
	EVENT_3					= 0x06,
	    EVENT_3_MAX_DUTY			= (1 << 2),
	    EVENT_3_ZERO_IBAT			= (1 << 1),
	    EVENT_3_VBUS_DET			= (1 << 0),
		
	SYSTEM_STATE				= 0x07,
	    SYSTEM_STATE_BUCK_EN_STAT		= (1 << 7),
	    SYSTEM_STATE_EXT_OVP_STAT		= (1 << 6),
	    SYSTEM_STATE_VBUS_DET_STAT		= (1 << 5),
	    SYSTEM_STATE_SYS_ALIVE_STAT		= (1 << 4),
	    SYSTEM_STATE_DISCHARGE_BAT_STAT	= (1 << 3),
	    SYSTEM_STATE_IBAT_ZERO		= (1 << 2),
	    SYSTEM_STATE_RECOVERY_STAT		= (1 << 1),
	    SYSTEM_STATE_CHARGE_EN		= (1 << 0),
	
	DC_LIMITER				= 0x08,
	    DC_LIMITER_I_REDUCED		= (1 << 6),
	    DC_LIMITER_BUCK_EN			= (1 << 5),
	    DC_LIMITER_VBUS_LIM			= (1 << 4),
	    DC_LIMITER_ICHG_LIM			= (1 << 3),
	    DC_LIMITER_VBAT_LIM			= (1 << 2),
	    DC_LIMITER_VMAIN_LIM		= (1 << 1),
	    DC_LIMITER_VBUS_VLIM		= (1 << 0),
	
	INPUT_CURRENT_LIMIT			= 0x09,
	    INPUT_CURRENT_LIMIT_base		= 90,
	    INPUT_CURRENT_LIMIT_step		= 10,
	    INPUT_CURRENT_LIMIT_100mA		= 0x01,
	    INPUT_CURRENT_LIMIT_500mA		= 0x29,
	    INPUT_CURRENT_LIMIT_1000mA		= 0x5B,
	    INPUT_CURRENT_LIMIT_1500mA		= 0x8D,
	    INPUT_CURRENT_LIMIT_2000mA		= 0xBF,
	    INPUT_CURRENT_LIMIT_max		= INPUT_CURRENT_LIMIT_2000mA,
	
	POR_INPUT_CURRENT_LIMIT			= 0x0A,
	ACTUAL_INPUT_CURRENT_LIMIT		= 0x0B,

	TARGET_CHARGE_CURRENT			= 0x0C,
	    CHARGE_CURRENT_base			= 0,
	    CHARGE_CURRENT_step			= 10,
	    CHARGE_CURRENT_disabled		= 0x00,
	    CHARGE_CURRENT_10mA			= 0x01,
	    CHARGE_CURRENT_max			= 0xFF,
	
	HOST_CONTROLLED_CHARGE_CURRENT		= 0x0D,
	ACTUAL_CHARGE_CURRENT			= 0x0E,

	TARGET_VOLTAGE_REGULATION		= 0x0F,
	    VOLTAGE_REGULATION_base		= 3000,
	    VOLTAGE_REGULATION_step		= 10,
	    VOLTAGE_REGULATION_3000mV		= 0x00,
	    VOLTAGE_REGULATION_4200mV		= 0x78,
	    VOLTAGE_REGULATION_4300mV		= 0x82,
	    VOLTAGE_REGULATION_4500mV		= 0x96,
	    VOLTAGE_REGULATION_max		= VOLTAGE_REGULATION_4500mV,

	CONTROL					= 0x10,
	    CONTROL_EN_ADAPTER_HS		= (1 << 4),
	    CONTROL_USB_SUSP			= (1 << 3),
	    CONTROL_VBUS_IMIL_SEL		= (1 << 2),
	    CONTROL_BUCK_FREQ_SHIFT_mask	= (3 << 0),
	    CONTROL_BUCK_FREQ_SHIFT_normal	= (0 << 0),
	    CONTROL_BUCK_FREQ_SHIFT_plus_150	= (1 << 0),
	    CONTROL_BUCK_FREQ_SHIFT_minus_150	= (2 << 0),
	    CONTROL_BUCK_FREQ_SHIFT_spread	= (3 << 0),
	
	ADC_CONTROL				= 0x11,
	   ADC_CONTROL_MANUAL_START		= (1 << 4),
	   ADC_CONTROL_CHANNEL_mask		= (7 << 1),
	   ADC_CONTROL_CHANNEL_vbus		= (0 << 1),
	   ADC_CONTROL_CHANNEL_ibus		= (1 << 1),
	   ADC_CONTROL_CHANNEL_ibat		= (2 << 1),
	   ADC_CONTROL_CHANNEL_vdd_main		= (3 << 1),
	   ADC_CONTROL_CHANNEL_die_temp		= (4 << 1),
	   ADC_CONTROL_CHANNEL_vbat		= (5 << 1),
	   ADC_CONTROL_ACCUM_RESET		= (1 << 0),
	
	ADC_OUTPUT				= 0x12,
	IBUS_AUTO				= 0x13,
	VBUS_AUTO				= 0x14,
	
	ADC_IBUS_ACCUMULATION_3			= 0x15,
	ADC_IBUS_ACCUMULATION_2			= 0x16,
	ADC_IBUS_ACCUMULATION_1			= 0x17,
	ADC_VBUS_ACCUMULATION_3			= 0x18,
	ADC_VBUS_ACCUMULATION_2			= 0x19,
	ADC_VBUS_ACCUMULATION_1			= 0x1A,
	SAMPLE_COUNT_2				= 0x1B,
	SAMPLE_COUNT_1				= 0x1C,

	HDQ_INTERFACE				= 0x1D,
	    HDQ_INTERFACE_HDQ_MSTR_mask		= (3 << 6),
	    HDQ_INTERFACE_HDQ_MSTR_none		= (3 << 6),
	    HDQ_INTERFACE_HDQ_MSTR_charger	= (3 << 6),
	    HDQ_INTERFACE_HDQ_MSTR_host		= (3 << 6),
	    HDQ_INTERFACE_HDQ_MSTR_bmu		= (3 << 6),
	    HDQ_INTERFACE_HDQ_HOST_EN		= (1 << 5),
	    HDQ_INTERFACE_MISSING_BATTERY	= (1 << 4),
	    HDQ_INTERFACE_MSTR_REQ_CHGR		= (1 << 3),
	    HDQ_INTERFACE_MSTR_REQ_HOST		= (1 << 2),
	    HDQ_INTERFACE_MSTR_REQ_BMU		= (1 << 1),
	    HDQ_INTERFACE_MSTR_FORCE_HOST	= (1 << 0),

	TEST_MODE_ENABLE			= 0x20,
	    TEST_MODE_ENABLE_off		= 0x00,
	    TEST_MODE_ENABLE_on			= 0xA5,

	TEST_MODE_CONTROL			= 0x21,
	    TEST_MODE_CONTROL_ENB_AUTOCONV	= (1 << 5),
	    TEST_MODE_CONTROL_EN_FORCEMODE	= (1 << 4),
	    TEST_MODE_CONTROL_FORCE_MODE_mask	= (3 << 2),
	    TEST_MODE_CONTROL_FORCE_MODE_stdby	= (0 << 2),
	    TEST_MODE_CONTROL_FORCE_MODE_chgdis	= (1 << 2),
	    TEST_MODE_CONTROL_FORCE_MODE_recov	= (2 << 2),
	    TEST_MODE_CONTROL_FORCE_MODE_normal	= (3 << 2),
	    TEST_MODE_CONTROL_GG_DIS		= (1 << 1),
	    TEST_MODE_CONTROL_SOFT_RESET	= (1 << 0),
};

static int charger_write(uint8_t reg, uint8_t value)
{
    uint8_t data[2] = { reg, value };
    return iic_write(CHARGER_IIC_BUS, SN2400_ADDR_W, data, sizeof(data));
}

static int charger_read(uint8_t reg, uint8_t *data)
{
    return iic_read(CHARGER_IIC_BUS, SN2400_ADDR_R, &reg, sizeof(reg), data, sizeof(uint8_t), IIC_NORMAL);
}

static int charger_readn(uint8_t reg, uint8_t *data, unsigned int count)
{
    return iic_read(CHARGER_IIC_BUS, SN2400_ADDR_R, &reg, sizeof(reg), data, count, IIC_NORMAL);
}

static int charger_read_events(uint8_t data[EVENT_count])
{
    uint8_t reg = EVENT_1;
    return iic_read(CHARGER_IIC_BUS, SN2400_ADDR_R, &reg, sizeof(reg), data, EVENT_count, IIC_NORMAL);
}

static int charger_read_adc(uint8_t channel, uint8_t *result)
{
    uint8_t data = ADC_CONTROL_MANUAL_START | (channel & ADC_CONTROL_CHANNEL_mask);
    if (charger_write(ADC_CONTROL, data) != 0) return -1;
    
    do {
	task_sleep(5 * 1000);	// data sheet lists 3.7 ms as typ. average time
	if (charger_read(ADC_CONTROL, &data) != 0) return -1;
    } while (data & ADC_CONTROL_MANUAL_START);
    
    return charger_read(ADC_OUTPUT, result);
}

void charger_early_init(void)
{
    // clear any events
    uint8_t events[EVENT_count];
    charger_read_events(events);

#if TARGET_POWER_NEEDS_BATTERY_PROTECTION_RESET && PRODUCT_LLB
    // check status for recovery mode
    uint8_t data;
    if (charger_read(SYSTEM_STATE, &data) == 0) {
	dprintf(DEBUG_INFO, "sn2400    : 0x07=%02x\n", data);
	if ((data & SYSTEM_STATE_RECOVERY_STAT) != 0) {
	    charger_reset_battery_protection();
	}
    }
#endif
}

void charger_clear_alternate_usb_current_limit(void)
{
    uint8_t data;
    charger_read(CONTROL, &data);
    data |= CONTROL_VBUS_IMIL_SEL;
    charger_write(CONTROL, data);
}

bool charger_has_usb(int dock)
{
    uint8_t data;
    charger_read(SYSTEM_STATE, &data);
    return data & SYSTEM_STATE_VBUS_DET_STAT;
}

bool charger_has_firewire(int dock)
{
    return false;   // FW not supported
}

bool charger_has_external(int dock)
{
    return false;   // Ext. not supported
}

bool charger_check_usb_change(int dock, bool expected)
{
    uint8_t events[EVENT_count];
    if (charger_read_events(events) != 0) return false;

    if (events[0] & (EVENT_1_VBUS_UV | EVENT_1_VBUS_OV)) return true;
    if (events[2] & (EVENT_3_VBUS_DET)) return true;

    return false;
}

// CBAT, maybe next...
static uint8_t sn2400_adjust_iset(uint32_t iset)
{
	if ( iset==0 ) {
		// leave it
	} else if ( iset<3 ) {
		iset=1;
	} else if ( iset<INPUT_CURRENT_LIMIT_500mA ) {
		iset-=2; 	// 20mA
	} else if ( iset<INPUT_CURRENT_LIMIT_1000mA ) {
		iset-=5;	// 50mA
	} else if ( iset<INPUT_CURRENT_LIMIT_1500mA ) {
		iset-=8;	// 80mA
	} else {
		iset-=10;	// 100ma
	}

	return iset;
}

void charger_set_charging(int dock, uint32_t input_current_ma, uint32_t *charge_current_ma)
{
    uint8_t charge_current;
    if (*charge_current_ma > (CHARGE_CURRENT_base + (CHARGE_CURRENT_max * CHARGE_CURRENT_step))) {
	charge_current = CHARGE_CURRENT_max;
    } else {
	charge_current = (*charge_current_ma - CHARGE_CURRENT_base) / CHARGE_CURRENT_step;
    }

    *charge_current_ma -= CHARGE_CURRENT_base + (charge_current * CHARGE_CURRENT_step);
    charger_write(HOST_CONTROLLED_CHARGE_CURRENT, charge_current ) ;
    
    if (input_current_ma < 100) {
	charger_write(INPUT_CURRENT_LIMIT, INPUT_CURRENT_LIMIT_100mA);

	uint8_t data;
	if (charger_read(CONTROL, &data) != 0) return;
	data |= CONTROL_USB_SUSP;
	charger_write(CONTROL, data);
    } else {
	uint8_t data;
	if (charger_read(CONTROL, &data) != 0) return;
	if (data & CONTROL_USB_SUSP) {
	    data &= ~CONTROL_USB_SUSP;
	    charger_write(CONTROL, data);

	    task_sleep(100 * 1000);
	}

	if (charger_read(ACTUAL_INPUT_CURRENT_LIMIT, &data) != 0) return;

	uint8_t input_current_limit;
	if (input_current_ma > (INPUT_CURRENT_LIMIT_base + (INPUT_CURRENT_LIMIT_max * INPUT_CURRENT_LIMIT_step))) {
	    input_current_limit = INPUT_CURRENT_LIMIT_max;
	} else {
	    input_current_limit = (input_current_ma - INPUT_CURRENT_LIMIT_base) / INPUT_CURRENT_LIMIT_step;
	}

	while (data < input_current_limit) {
	    data += (100 / INPUT_CURRENT_LIMIT_step);
	    if (data > input_current_limit) data = input_current_limit;
	    charger_write(INPUT_CURRENT_LIMIT, sn2400_adjust_iset(data)  );
	    task_sleep(10 * 1000);
	}
    }
}

void charger_clear_usb_state(void)
{
    // empty
}

bool charger_charge_done(int dock)
{
    uint8_t data;
    if (charger_read(SYSTEM_STATE, &data) != 0) return false;
    return !(data & SYSTEM_STATE_CHARGE_EN);
}

bool charger_has_batterypack(int dock)
{
    return true;
}

uint32_t charger_get_max_charge_current(int dock)
{
    return CHARGE_CURRENT_base + (CHARGE_CURRENT_max * CHARGE_CURRENT_step);
}

int charger_read_battery_level(uint32_t *milliVolts)
{
    uint8_t adc;
    if (charger_read_adc(ADC_CONTROL_CHANNEL_vbat, &adc) != 0) return -1;
    *milliVolts = 2500 + (10 * adc);
    return 0;
}

int charger_set_gasgauge_interface(bool enabled)
{
    if (enabled) {
	if (charger_write(HDQ_INTERFACE, HDQ_INTERFACE_MSTR_REQ_HOST) != 0) return -1;

	utime_t timeout = system_time() + 1*1000*1000;
	for (;;) {
	    uint8_t data;
	    if (charger_read(HDQ_INTERFACE, &data) != 0) {
		charger_write(HDQ_INTERFACE, 0);
		return -1;
	    }
	    if (data & HDQ_INTERFACE_HDQ_HOST_EN) break;
	    if (system_time() > timeout) {
		printf("charger HDQ interface request read timed out\n");
		return (-1);
	    }
	    task_sleep(10 * 1000);
	}
    } else {
	charger_write(HDQ_INTERFACE, 0);
    }
    return 0;
}

int charger_reset_battery_protection(void)
{
    // enter test mode and force charging at 4.3V VBATREG and 10mA
    // charging current for 20mS
    int ret, ret2;

    dprintf(DEBUG_CRITICAL, "starting battery protection reset...\n");
    // check all PMU temp sensors for "ambient temperature" check
    for (int idx = 1; idx <= 4; idx++) {
	int temp_cC;
	ret = pmu_read_system_temperature(idx, &temp_cC);
	if (ret != 0) {
	    dprintf(DEBUG_CRITICAL, "temp failure\n");
	    return -1;
	}
	if (temp_cC <= 500 || temp_cC >= 4000) {
	    dprintf(DEBUG_CRITICAL, "system temp %d out of range: %d.%02dC\n", idx, temp_cC / 100, temp_cC % 100);
	    return -1;
	}
    }
    
    ret = charger_write(HOST_CONTROLLED_CHARGE_CURRENT, CHARGE_CURRENT_10mA);
    if (ret == 0) ret = charger_write(TEST_MODE_ENABLE, TEST_MODE_ENABLE_on);
    if (ret == 0) ret = charger_write(TEST_MODE_CONTROL, TEST_MODE_CONTROL_GG_DIS);
    if (ret == 0) ret = charger_write(TARGET_CHARGE_CURRENT, CHARGE_CURRENT_disabled);
    if (ret == 0) ret = charger_write(TARGET_VOLTAGE_REGULATION, VOLTAGE_REGULATION_4300mV);
    if (ret == 0) ret = charger_write(TARGET_CHARGE_CURRENT, CHARGE_CURRENT_10mA);

    task_sleep(20 * 1000);
    
    if (ret == 0) ret = charger_write(TARGET_CHARGE_CURRENT, CHARGE_CURRENT_disabled);
    if (ret == 0) ret = charger_write(TARGET_VOLTAGE_REGULATION, VOLTAGE_REGULATION_4200mV);
    if (ret == 0) ret = charger_write(TEST_MODE_CONTROL, 0x00);
    // always attempt to take out of test mode, even on failure
    ret2 = charger_write(TEST_MODE_ENABLE, TEST_MODE_ENABLE_off);
    if (ret == 0) ret = ret2;
    if (ret == 0) ret = charger_write(HOST_CONTROLLED_CHARGE_CURRENT, CHARGE_CURRENT_max);

    dprintf(DEBUG_CRITICAL, "battery protection reset completed: %s\n", (ret == 0) ? "success" : "error");
    
    return ret;
}

void charger_print_status(void)
{
    int rc;
    uint8_t data[7];

    rc=charger_readn(SYSTEM_STATE, data, 2);
    if ( rc!=0 ) {
        dprintf(DEBUG_CRITICAL, "cannot read %x (%d)\n", SYSTEM_STATE, rc);
    } else {
        rc=charger_readn(ACTUAL_INPUT_CURRENT_LIMIT, &data[2], 5);
        if ( rc!=0 ) {
            dprintf(DEBUG_CRITICAL, "cannot read %x (%d)\n", ACTUAL_INPUT_CURRENT_LIMIT, rc);
        } else {
            dprintf(DEBUG_CRITICAL, "sn2400    : 0x07=%02x 0x08=%02x 0x0b=%02x 0x0c=%02x 0x0d=%02x 0x0e=%02x 0x0f=%02x\n", 
                data[0], data[1], data[2], data[3], data[4], data[5], data[6]);
        }
    }
}
