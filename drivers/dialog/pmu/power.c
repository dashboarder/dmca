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

#if DIALOG_D1755
#include "d1755.h"
#elif DIALOG_D1815
#include "d1815.h"
#elif DIALOG_D1881
#include "d1881.h"
#elif DIALOG_D1946
#include "d1946.h"
#elif DIALOG_D1972
#include "d1972.h"
#elif DIALOG_D1974
#include "d1974.h"
#elif DIALOG_D2018
#include "d2018.h"
#elif DIALOG_D2045
#include "d2045.h"
#elif DIALOG_D2089
#include "d2089.h"
#elif DIALOG_D2186
#include "d2186.h"
#elif DIALOG_D2207
#include "d2207.h"
#elif DIALOG_D2238
#include "d2238.h"
#elif DIALOG_D2255
#include "d2255.h"
#elif DIALOG_D2257
#include "d2257.h"
#else
#error Unknown Dialog PMU selection
#endif

#include "NTCTables.h"

struct pmu_setup_struct
{
	int		pmu;
	uint16_t	reg;
	uint16_t	value;
};

struct core_rails_struct
{
	int		pmu;
	uint16_t	ctrl_reg;
	uint16_t	data_reg;
};

struct power_rail_s {
	uint8_t index;
    uint16_t min_mv;
    uint16_t max_mv;
    uint16_t step_uv; // 3125=3.125 mV
};
typedef struct power_rail_s target_rails_t[POWER_RAIL_COUNT];

// must include this after the above declarations
#define POWERCONFIG_PMU_SETUP 1
#include <target/powerconfig.h>

static uint8_t		boot_flags;
static eventRegisters	cold_boot_ints;
static bool		cold_boot_ints_valid;

// This goes after the powerconfig
#ifndef MAX_BACKLIGHT_LEVEL
#define MAX_BACKLIGHT_LEVEL ((1 << (WLED_ISET_BITS)) - 1)
#endif

#ifndef TARGET_BRICKID_FULL_SCALE
#define TARGET_BRICKID_FULL_SCALE (5000)
#endif

// define PMU_DATA_DEBUG_SPEW as DEBUG_CRITICAL to enable logging
#define PMU_DATA_DEBUG_SPEW	DEBUG_SPEW

int pmu_set_data(int dev, uint16_t reg, uint8_t byte, bool do_confirm)
{
	uint8_t confirm;
	UInt8 data[kDIALOG_REG_BYTES + 1];

	if (kDIALOG_REG_BYTES > 1) data[kDIALOG_REG_BYTES - 2] = (reg >> 8) & 0xFF;
	data[kDIALOG_REG_BYTES - 1] = (reg & 0xFF);
	data[kDIALOG_REG_BYTES] = byte;

	int rc=iic_write(dev, kDIALOG_ADDR_W, data, sizeof(data));
	if ( rc!=0 ) {
		dprintf(DEBUG_CRITICAL, "pmu%d: wrote %x to reg %x failed (%d)\n", dev, byte, reg, rc);
	} else if (do_confirm) {
		iic_read(dev, kDIALOG_ADDR_R, data, sizeof(data)-1, &confirm, 1, IIC_NORMAL);
		if (byte == confirm) {
			dprintf(PMU_DATA_DEBUG_SPEW, "pmu%d: wrote %x to reg %x\n", dev, byte, reg);
		} else {
			dprintf(PMU_DATA_DEBUG_SPEW, "pmu%d: try to write %x to reg %x, but got %x back\n", dev, byte, reg, confirm);
			return -1;
		}
	}

	return 0;
}

int pmu_get_data(int dev, uint16_t reg, uint8_t *byte)
{
	UInt8 addr[kDIALOG_REG_BYTES];
	if (kDIALOG_REG_BYTES > 1) addr[kDIALOG_REG_BYTES - 2] = (reg >> 8) & 0xFF;
	addr[kDIALOG_REG_BYTES - 1] = (reg & 0xFF);
	
	const int rc=iic_read(dev, kDIALOG_ADDR_R, addr, sizeof(addr), byte, 1, IIC_NORMAL);
	dprintf(PMU_DATA_DEBUG_SPEW, "pmu%d: read %x from reg %x\n", dev, *byte, reg);		
	return rc;
}

void pmu_read_events(eventRegisters data)
{
	bzero(data, sizeof(eventRegisters));
	UInt8 addr[kDIALOG_REG_BYTES];
	if (kDIALOG_REG_BYTES > 1) addr[kDIALOG_REG_BYTES - 2] = (kDIALOG_EVENT_A >> 8) & 0xFF;
	addr[kDIALOG_REG_BYTES - 1] = (kDIALOG_EVENT_A & 0xFF);
	iic_read(PMU_IIC_BUS, kDIALOG_ADDR_R, addr, sizeof(addr), data, sizeof(eventRegisters), IIC_NORMAL);
}

static void pmu_read_status(statusRegisters data)
{
	bzero(data, sizeof(statusRegisters));
	UInt8 addr[kDIALOG_REG_BYTES];
	if (kDIALOG_REG_BYTES > 1) addr[kDIALOG_REG_BYTES - 2] = (kDIALOG_STATUS_A >> 8) & 0xFF;
	addr[kDIALOG_REG_BYTES - 1] = (kDIALOG_STATUS_A & 0xFF);
	iic_read(PMU_IIC_BUS, kDIALOG_ADDR_R, addr, sizeof(addr), data, sizeof(statusRegisters), IIC_NORMAL);
}

static void pmu_write_mask(const eventRegisters data)
{
	uint8_t	bytes[sizeof(eventRegisters) + kDIALOG_REG_BYTES];

	if (kDIALOG_REG_BYTES > 1) bytes[kDIALOG_REG_BYTES - 2] = (kDIALOG_IRQ_MASK_A >> 8) & 0xFF;
	bytes[kDIALOG_REG_BYTES - 1] = (kDIALOG_IRQ_MASK_A & 0xFF);
	memcpy(bytes+kDIALOG_REG_BYTES, data, sizeof(eventRegisters));

	iic_write(PMU_IIC_BUS, kDIALOG_ADDR_W, bytes, sizeof(bytes));
}

static int pmu_read_nvram(uint16_t addr, void *buffer, uint8_t count) 
{
	uint8_t	bytes[kDIALOG_REG_BYTES];

	if (kDIALOG_REG_BYTES > 1) bytes[kDIALOG_REG_BYTES - 2] = (addr>>8) & 0xFF;
	bytes[kDIALOG_REG_BYTES - 1] = (addr & 0xFF);

	return iic_read(PMU_IIC_BUS, kDIALOG_ADDR_R, bytes, sizeof(bytes), buffer, count, IIC_NORMAL);
}

static int pmu_write_nvram(uint16_t addr, void *buffer, uint8_t count) 
{
	uint8_t	bytes[kDIALOG_REG_BYTES+count];

	if (kDIALOG_REG_BYTES > 1) bytes[kDIALOG_REG_BYTES - 2] = (addr>>8) & 0xFF;
	bytes[kDIALOG_REG_BYTES - 1] = (addr & 0xFF);

	memcpy(bytes+kDIALOG_REG_BYTES, buffer, count);

	return iic_write(PMU_IIC_BUS, kDIALOG_ADDR_W, bytes, kDIALOG_REG_BYTES + count);
}

static int
power_read_memory_calibration_data(uint16_t addr, void *buffer, uint32_t length)
{
	for (uint32_t size=0, count=0; length-count ; count+=size) {
		size=length-count;
		if ( size>64 ) size=64;

		const bool success=( pmu_read_nvram(addr+count, ((char*)buffer)+count, size) == 0 );
		if ( !success ) return 1;
	}

	return 0;
}

static int
power_write_memory_calibration_data(uint16_t addr, void *buffer, uint32_t length)
{
	for (uint32_t size=0, count=0; length-count ; count+=size) {
		size=length-count;
		if ( size>64 ) size=64;

		const bool success=( pmu_write_nvram(addr+count, ((char*)buffer)+count, size) == 0 );
		if ( !success ) return 1;
	}

	return 0;
}

// @pre settings is kDIALOG_EXT_MEM_CAL_SIZE byes
bool
power_load_memory_calibration(void *settings, uint32_t length) {

#if PMU_HAS_RAM && !DIALOG_D2238
	uint32_t count=0, size;
	if ((!settings) || (length > kDIALOG_EXT_MEM_CAL_SIZE)) return false;

	bzero(settings, length); 

	size=( length>kDIALOG_EXT_MEM_CAL0_SIZE ) ? kDIALOG_EXT_MEM_CAL0_SIZE : length ;
	if ( pmu_read_nvram(kDIALOG_EXT_MEM_CAL0, &settings[count], size)!=0 ) return false;	
	length-=size; count+=size;

	if ( length ) {
		size= ( length>kDIALOG_EXT_MEM_CAL1_SIZE ) ? kDIALOG_EXT_MEM_CAL1_SIZE : length ;
		if ( power_read_memory_calibration_data(kDIALOG_EXT_MEM_CAL1, &settings[count], size)!=0 ) return false;
		length-=size; count+=size;
	}

#if DIALOG_D2257
	if ( length ) {
		size= ( length>kDIALOG_EXT_MEM_CAL2_SIZE ) ? kDIALOG_EXT_MEM_CAL2_SIZE : length ;
		if ( power_read_memory_calibration_data(kDIALOG_EXT_MEM_CAL2, &settings[count], size)!=0 ) return false;
		length-=size; count+=size;
	}
#endif

	if ( length ) dprintf(DEBUG_SPEW, "WARN: power_load_memory_calibration remainder=%d\n", length);

	return true;
#endif

	return false;
}

bool
power_store_memory_calibration(void *settings, uint32_t length) {

#if PMU_HAS_RAM && !DIALOG_D2238
	uint32_t count=0, size;

	if ((!settings) || (length > kDIALOG_EXT_MEM_CAL_SIZE)) return false;

	size=(length>kDIALOG_EXT_MEM_CAL0_SIZE)?kDIALOG_EXT_MEM_CAL0_SIZE:length;
	if ( power_write_memory_calibration_data(kDIALOG_EXT_MEM_CAL0, &settings[count], size)!=0 ) return false;
	length-=size; count+=size;
	
	if ( length ) {
		size=(length>kDIALOG_EXT_MEM_CAL1_SIZE)?kDIALOG_EXT_MEM_CAL1_SIZE:length;
		if ( power_write_memory_calibration_data(kDIALOG_EXT_MEM_CAL1, &settings[count], size)!=0 ) return false;
		length-=size; count+=size;
	}

#if DIALOG_D2257
	if ( length ) {
		size=(length>kDIALOG_EXT_MEM_CAL2_SIZE)?kDIALOG_EXT_MEM_CAL2_SIZE:length;
		if ( power_write_memory_calibration_data(kDIALOG_EXT_MEM_CAL2, &settings[count], size)!=0 ) return false;
		length-=size; count+=size;
	}
#endif

	if ( length ) dprintf(DEBUG_SPEW, "WARN: power_store_memory_calibration remainder=%d\n", length);

	return true;
#endif

	return false;
}


// @pre settings is kDIALOG_VOLTAGE_KNOBS_SIZE bytes
bool
power_load_voltage_knobs(void *settings, uint32_t length) {
	bool success=false;
    
#if WITH_VOLTAGE_KNOBS
	if ((!settings) || (length != kDIALOG_VOLTAGE_KNOBS_SIZE)) return false;
    
	bzero(settings, length);
    
	success=( 0 == pmu_read_nvram(kDIALOG_VOLTAGE_KNOBS, settings, length) );
#endif
    
	return success;
}

bool
power_store_voltage_knobs(void *settings, uint32_t length) {
	bool success=false;
    
#if WITH_VOLTAGE_KNOBS
	if ((!settings) || (length != kDIALOG_VOLTAGE_KNOBS_SIZE)) return false;
    
	success=(0 == pmu_write_nvram(kDIALOG_VOLTAGE_KNOBS, settings, length));
#endif
    
	return success;
}

#if DIALOG_D2255 || DIALOG_D2257
static int
dialog_read_adc(int dev, unsigned input_select, unsigned *level)
{
	int	result;
	const uint16_t man_ctrl_addr=kDIALOG_ADC_MAN_CTL;
	const uint8_t channel_index=input_select & kDIALOG_ADC_CONTROL_MUX_SEL_MASK;

	result=pmu_set_data(dev, man_ctrl_addr, channel_index, 0);
	if ( result!=0 ) return -1;

	int done=0;
	uint8_t	data[2]; // MAN*_RES_LSB, MAN*_RES_MSB
	uint8_t addr[2] = { (kDIALOG_ADC_LSB >> 8) & 0xFF, (kDIALOG_ADC_LSB & 0xFF)};
	const utime_t end_time = system_time() + 50*1000 ;

	do {
	    spin(1000);

		result=iic_read(PMU_IIC_BUS, kDIALOG_ADDR_R, addr, sizeof(addr), &data[0], sizeof(data), IIC_NORMAL);
		if ( result!=0 ) break;

		done=( (data[0]&kDIALOG_ADC_LSB_MANADC_ERROR)==0 );
		if ( done ) {
			*level = ((unsigned)data[1] << 4) | (data[0] & 0xf);
		} else if ( system_time() > end_time ) {
			result=-1;
		    dprintf(DEBUG_CRITICAL,  "dialog_read_adc timeout, MUX_SEL=%x\n", input_select);
		    break;
	    }

	} while ( !done );

#if DIALOG_D2255
	// Antigua130 inverts the ADC bits for these channels
	if (input_select == kD2255_ADC_CONTROL_MUX_SEL_VDD_MAIN ||
	    input_select == kD2255_ADC_CONTROL_MUX_SEL_BRICK_ID ||
	    input_select == kD2255_ADC_CONTROL_MUX_SEL_APP_MUX_A ||
	    input_select == kD2255_ADC_CONTROL_MUX_SEL_APP_MUX_B) {

	    // check for 2265 part (Antigua130)
	    uint8_t antigua130;
	    uint8_t addr[2] = { (kD2265_IMPLEMENTATION >> 8) & 0xFF, (kD2265_IMPLEMENTATION & 0xFF)};
	    result=iic_read(PMU_IIC_BUS, kDIALOG_ADDR_R, addr, sizeof(addr), &antigua130, 1, IIC_NORMAL);

	    if ( (result == 0) && (antigua130 == kD2265_130NM) ) {
		*level = ~(*level) & ((1 << kDIALOG_ADC_RESOLUTION_BITS) - 1);
	    }
	}
	dprintf (DEBUG_SPEW, "kD2255_ADC_CONTROL_(%d): 2265 bit inverting %#x -> %#x\n", input_select, ((unsigned)data[1] << 4) | (data[0] & 0xf), *level);
#endif

	// clear EOC
	eventRegisters ints_pending;
	pmu_read_events(ints_pending);

	return result;
}
#else 
static int
dialog_read_adc(int dev, unsigned input_select, unsigned *level)
{
	int	result = 0;
	uint8_t	data[2];
	uint8_t reg;
	utime_t start_time;
	int 	timeoutOccurred;
#if ADC_TIMEOUT_STATISTICS
	int 	chg_dis=0;
#endif
#if ADC_TIMEOUT_WORKAROUND
	int 	numTimeouts=0;
#endif

	reg = (input_select & kDIALOG_ADC_CONTROL_MUX_SEL_MASK) | kDIALOG_ADC_CONTROL_DEFAULTS;

#if !DIALOG_D2238 
	if (kDIALOG_ADC_CONTROL_MUX_SEL_ACC_ID == input_select) {
#if DIALOG_D2045 || DIALOG_D2089 || DIALOG_D2186 || DIALOG_D2207 || DIALOG_D2238
		pmu_set_data(dev, kDIALOG_ADC_CONTROL2, kDIALOG_ADC_CONTROL2_ADC_REF_EN, 0);
#else
		pmu_set_data(dev, kDIALOG_ADC_CONTROL, kDIALOG_ADC_CONTROL_ADC_REF_EN | reg, 0);
		reg |= kDIALOG_ADC_CONTROL_ADC_REF_EN;
#endif
		spin(80 * 1000);
	}
#endif

	pmu_set_data(dev, kDIALOG_ADC_CONTROL, kDIALOG_ADC_CONTROL_MAN_CONV | reg, 0);

#if ADC_TIMEOUT_WORKAROUND
continue_conv:
#endif
	start_time = system_time();
	timeoutOccurred = 0;
	do {
	    spin(1000);

	    if (system_time() > (start_time + 50*1000))
	    {
		    dprintf(DEBUG_CRITICAL, 
			    "dialog_read_adc timeout, MUX_SEL=%x\n", 
			    input_select);
#if ADC_TIMEOUT_WORKAROUND
		    timeoutOccurred = 1;
		    break;
#else
		    return -1;
#endif
	    }

	    pmu_get_data(dev, kDIALOG_ADC_CONTROL, &reg);
	} while ((reg & kDIALOG_ADC_CONTROL_MAN_CONV) != 0);

#if ADC_TIMEOUT_WORKAROUND
	enum adc_issues {
		kRetriesDidntHelp	= 0x01,
		kAutoConversionVDD	= 0x02,
		kAutoConversionTemp	= 0x04,
		kBatPwrSuspNotAsserted	= 0x08,
		kChrgDisabled		= 0x10
	};
		
	if(timeoutOccurred) {
		uint8_t issues = 0;
#if ADC_TIMEOUT_STATISTICS
		// Write timeout count to scratchpad
		if(numTimeouts==0) {
			pmu_get_data(dev, kDIALOG_MEMBYTE0+0x09, &data[0]);
			if(data[0]<255) data[0]++;
			pmu_set_data(dev, kDIALOG_MEMBYTE0+0x09, data[0], 0);
		}
		// Check that auto conversions are disabled
		uint8_t adc_regs[5];
		reg = kDIALOG_ADC_CONTROL;
		iic_read(PMU_IIC_BUS, kDIALOG_ADDR_R, &reg, 1, &adc_regs[0], 
			 sizeof(adc_regs), IIC_NORMAL);
		if(adc_regs[0] & kDIALOG_ADC_CONTROL_AUTO_VDD_OUT_EN) 
			issues |= kAutoConversionVDD;
		if(adc_regs[4] & 0x80) issues |= kAutoConversionTemp;
		// Check that we're charging. Not treating this as critical yet,
		// but if we're not charging this might be a reason the workaround
		// could fail
		pmu_get_data(dev, kDIALOG_STATUS_A, &data[0]);
		if((data[0]&kD1815_STATUS_A_VBUS_DET)==0) chg_dis |= 1;
		// Check that bat_pwr_susp is enabled
		pmu_get_data(dev, kDIALOG_SYS_CONTROL, &data[0]);
		if((data[0]&kDIALOG_SYS_CONTROL_BAT_PWR_SUSPEND)==0) 
			issues |= kBatPwrSuspNotAsserted;
#endif
		numTimeouts++;
		if(numTimeouts > 10) issues |= kRetriesDidntHelp;
		// Trouble? Give up
		if(issues) {
#if ADC_TIMEOUT_STATISTICS
			// Charging was disabled anytime we tried the workaround
			if(chg_dis) issues |= kChrgDisabled;
			// Write issues to scratchpad
			pmu_set_data(dev, kDIALOG_MEMBYTE0+0x8, issues, 0);
			// Write reset count to scratchpad
			pmu_get_data(dev, kDIALOG_MEMBYTE0+0x0a, &data[0]);
			if(data[0]<255) data[0]++;
			pmu_set_data(dev, kDIALOG_MEMBYTE0+0x0a, data[0], 0);
#endif
			// PMU register reset
			pmu_get_data(dev, 0xE0, &data[0]);
			data[0]|=1;	// test_enable
			pmu_set_data(dev, 0xE0, data[0], 0);
			data[0]|=3;	// test_enable|SW_RESET
			pmu_set_data(dev, 0xE0, data[0], 0);
			// NOTREACHED
			while(1);
		} else {
			pmu_get_data(dev, kDIALOG_SYS_CONTROL, &data[0]);
			// Clear BAT_PWR_SUSP
			data[0] &= ~kDIALOG_SYS_CONTROL_BAT_PWR_SUSPEND;
			pmu_set_data(dev, kDIALOG_SYS_CONTROL, data[0], 0);
			// Wait
			spin(1300);
			// Set BAT_PWR_SUSP
			data[0] |= kDIALOG_SYS_CONTROL_BAT_PWR_SUSPEND;
			pmu_set_data(dev, kDIALOG_SYS_CONTROL, data[0], 0);
			// Try again!
			goto continue_conv;
		}
	}
#endif /* ADC_TIMEOUT_WORKAROUND */
	
	UInt8 addr[kDIALOG_REG_BYTES];
	if (kDIALOG_REG_BYTES > 1) addr[kDIALOG_REG_BYTES - 2] = (kDIALOG_ADC_LSB >> 8) & 0xFF;
	addr[kDIALOG_REG_BYTES - 1] = (kDIALOG_ADC_LSB & 0xFF);
	iic_read(PMU_IIC_BUS, kDIALOG_ADDR_R, addr, sizeof(addr), &data[0], sizeof(data), IIC_NORMAL);

	pmu_set_data(dev, kDIALOG_ADC_CONTROL, kDIALOG_ADC_CONTROL_DEFAULTS, 0);
#if DIALOG_D2045 || DIALOG_D2089 || DIALOG_D2186 || DIALOG_D2207 || DIALOG_D2238
	pmu_set_data(dev, kDIALOG_ADC_CONTROL2, kDIALOG_ADC_CONTROL2_DEFAULTS, 0);
#endif

#if DIALOG_D1755
	*level = ((unsigned)data[1] << 2) | (data[0] & 0x3);
#else
	*level = ((unsigned)data[1] << 4) | (data[0] & 0xf);
#endif
    
	return result;
}
#endif

int
pmu_uvwarn_config(int dev, uint32_t thresholdMV)
{
	int rc=-1; // not supported

#if DIALOG_D2257
	rc=pmu_set_data(dev, kDIALOG_TEST_ACCESS, kDIALOG_TEST_ACCESS_ENA, false);
	if ( rc<0 ) {

	} else {
		uint8_t uv_dig_conf;

		rc=pmu_get_data(dev, kD2257_IPK_UV_BUCK1_UV_DIG_CONF_1, &uv_dig_conf);
		if ( rc<0 ) {
			dprintf(DEBUG_CRITICAL, "pmu: cannot read UV_CONF (%d)\n", rc);
		} else {
			if ( thresholdMV!=0 ) {
				u_int32_t uv_en_thr;

				rc=power_get_rail_value(POWER_RAIL_GPU, thresholdMV,  &uv_en_thr);
				if ( rc==0 ) rc=pmu_set_data(dev, kD2257_IPK_UV_BUCK1_UV_EN_THR, uv_en_thr, true);

				uv_dig_conf|=1<<0; // UV_EN
			} else {
				uv_dig_conf&=0xfe; // clear UV_EN
			}

			if ( rc==0 ) rc=pmu_set_data(dev, kD2257_IPK_UV_BUCK1_UV_DIG_CONF_1, uv_dig_conf, true);
		}

		int temp=pmu_set_data(dev, kDIALOG_TEST_ACCESS, kDIALOG_TEST_ACCESS_DIS, false);
		if ( temp<0 ) panic("pmu: cannot exit test mode\n");
	}
#endif

	return rc;
}

static int
dialog_read_battery_level(int dev, unsigned *level)
{
	int		 result;

#if DIALOG_D2255
// PFIXME: D2255 need to use charger_read_battery_level()
	result = 0;
#else
	unsigned adc = 0;

	result = dialog_read_adc(dev, kDIALOG_ADC_CONTROL_MUX_SEL_VBAT, &adc);

#if DIALOG_D2045 || DIALOG_D2089 || DIALOG_D2186 || DIALOG_D2207 || DIALOG_D2238 || DIALOG_D2231
	*level = ((adc * 2500) >> kDIALOG_ADC_RESOLUTION_BITS) + 2500;
#else
	*level = ((adc * 2000) >> kDIALOG_ADC_RESOLUTION_BITS) + 2500;
#endif

#endif
	return result;
}

static int lut_interpolate(const struct NTCLUT * const LUT, const uint32_t R)
{
	int idx=0;
	uint32_t upper, lower, R1, R2;
	int32_t T1, T2;
	do {
		upper = LUT[idx].ohm;
		lower = LUT[idx+1].ohm;
		// Is our lookup value within the range of the two table entries?
		// This is also true if the lookup value is outside the table
		// range; we just interpolate from the last table entries.
		if((R<=lower) && (LUT[idx+2].ohm != 0)) {
			idx++;
			continue;
		}
		
		// Subtract the lower limit, so lower is the zero offset for interpolation
		R1 = upper-lower;
		R2 = R - lower;
		T1 = LUT[idx].centiCelsius - LUT[idx+1].centiCelsius;
		// The interpolation.
		// (T1/T2) = (R1/R2)  <=> T2 = ( (T1*R2) / R1 )
		T2 = ( (T1*(int32_t)R2) / (int32_t)R1 );
		// Add back the lower limit to our result.
		T2 += LUT[idx+1].centiCelsius;
		
		dprintf(DEBUG_SPEW, "LUTInterpolate: R=%d, R1=%d, R2=%d, T1=%d, T2=%d\n", 
		       R, R1, R2, T1, T2);
		break;
	} while(1);
	
	return T2;
}

static int
dialog_read_battery_temperature(int *level)
{
	int		 result;
#if DIALOG_D2255
// PFIXME: D2255 need to use charger_read_battery_temperature()
	result=-1;
#else
	unsigned adc = 0;

	if (level == NULL) return -1;

	result = dialog_read_adc(PMU_IIC_BUS, kDIALOG_ADC_CONTROL_MUX_SEL_TBAT, &adc);

	if (result == 0) {
		// The NTC is connected to a 50uA current source
		// R = U / I; 1/I = 1/50uA = 20000 * 1/A
		// U = <adc_val> * <adc_full_scale_range> / <adc_resolution>
		// adc_fsr comes in mV, take the factor of 1000 out of current constant
		const uint32_t Rntc = (adc * (20000/1000) * kDIALOG_ADC_FULL_SCALE_MV) >> kDIALOG_ADC_RESOLUTION_BITS;
		dprintf(DEBUG_SPEW, "ADC chan %d: value=%d, Rntc=%d\n", kDIALOG_ADC_CONTROL_MUX_SEL_TBAT, adc, Rntc);
		*level=lut_interpolate(NTCG103JF103F, Rntc);
	}
#endif	
	return result;
}

static int
dialog_use_setup(const struct pmu_setup_struct *setup, uint32_t count)
{
	uint32_t i;

	/* set pmu registers from the list */
	for (i = 0; i < count; i++) {
		pmu_set_data(setup[i].pmu, setup[i].reg, setup[i].value, true);
	}

	return 0;
}

// assume that the offset of the lock register from the VSEL register is fixed
static int
dialog_lock_buck_vsel(int dev, uint16_t buck_vsel)
{
#if PMU_HAS_VSEL_LOCK
	const uint16_t buck_vsel_lock=buck_vsel+kDIALOG_BUCK_VSEL_LOCK_OFFSET; 
	//dprintf(DEBUG_CRITICAL, " pmu: locking vsel_lock_addr=%x\n", buck_vsel_lock);
	return pmu_set_data(dev, buck_vsel_lock, kDIALOG_BUCK_VSEL_LOCK_EN, true);
#else
	return 0;
#endif
}

u_int32_t
pmu_read_brick_id_level(void)
{
#if !DIALOG_D2238
	uint32_t adc = 0;

	if (dialog_read_adc(PMU_IIC_BUS, kDIALOG_ADC_CONTROL_MUX_SEL_BRICK_ID, &adc))
	    return 0;

	// Scale to a 1mV resolution - 5000mV is full scale
	return (adc * TARGET_BRICKID_FULL_SCALE) >> kDIALOG_ADC_RESOLUTION_BITS;
#else
    return 0;
#endif
}

int pmu_read_system_temperature(int idx, int *centiCelsiusTemperature)
{
	int		 result;
	unsigned adc = 0;

	result = dialog_read_adc(PMU_IIC_BUS, kDIALOG_ADC_CONTROL_MUX_SEL_NTC0 + idx, &adc);

	if (result == 0) {
		// The NTC is connected to a 50uA current source
		// R = U / I; 1/I = 1/50uA = 20000 * 1/A
		// U = <adc_val> * <adc_full_scale_range> / <adc_resolution>
		// adc_fsr comes in mV, take the factor of 1000 out of current constant
		const uint32_t Rntc = (adc * (20000/1000) * kDIALOG_ADC_FULL_SCALE_MV) >> kDIALOG_ADC_RESOLUTION_BITS;
		dprintf(DEBUG_SPEW, "ADC chan %d: value=%d, Rntc=%d\n", kDIALOG_ADC_CONTROL_MUX_SEL_NTC0, adc, Rntc);
		*centiCelsiusTemperature=lut_interpolate(NTCG103JF103F, Rntc);
	}

	return result;

}

/* pmu_early_init() is only called in iBSS and LLB - not in iBoot or iBEC. */
void
pmu_early_init(void)
{
	int rc;
	u_int8_t data;

	/* Get and clear the boot flags, ie. kDIALOG_EVENT_B_HIB. */
	/* Events in this register are lost over sleep/wake, so they're stored for the OS later */
	rc=pmu_get_data(PMU_IIC_BUS, kDIALOG_EVENT_A + EVENT_FLAG_GET_BYTE(kDIALOG_EVENT_HIB_MASK), &boot_flags);
	if ( rc!=0 ) dprintf(DEBUG_CRITICAL, "pmu%d: cannot read HIB_MASK (%d)\n", PMU_IIC_BUS, rc);

	/* Set all the warm boot defaults */
	dialog_use_setup(pmu_warm_init, sizeof(pmu_warm_init)/sizeof(pmu_warm_init[0]));

	if (!power_is_suspended()) {
		dialog_use_setup(pmu_cold_init, sizeof(pmu_cold_init)/sizeof(pmu_cold_init[0]));
#if PMU_COLD_INIT_AP_DEV
		if (target_config_dev()) {
			dialog_use_setup(pmu_cold_init_dev, sizeof(pmu_cold_init_dev)/sizeof(pmu_cold_init_dev[0]));
		} else {
			dialog_use_setup(pmu_cold_init_ap, sizeof(pmu_cold_init_ap)/sizeof(pmu_cold_init_ap[0]));
		}
#endif
		
#if DIALOG_D1946
		/* <rdar://problem/11006711> Force sync mode on P105 proto 1 only */
		pmu_get_data(PMU_IIC_BUS, kDIALOG_CHIP_ID, &data);
		if (data == 0x45) {
			pmu_set_data(PMU_IIC_BUS, kDIALOG_BANKSEL, 1, true);	// Bank 1
			pmu_set_data(PMU_IIC_BUS, kD1946_BUCK_CONTROL1, 0x0e, true);	// Buck0 sync mode, 1250mA limit per phase
			pmu_set_data(PMU_IIC_BUS, kD1946_BUCK_CONTROL3, 0x0e, true);	// Buck2 sync mode, 1250mA limit per phase
			pmu_set_data(PMU_IIC_BUS, kDIALOG_BANKSEL, 0, true);
		}
#endif
#if DIALOG_D1881
		// <rdar://problem/10420919> Update PMU settings to support PRQs
		pmu_get_data(PMU_IIC_BUS, kDIALOG_CHIP_ID, &data);
		data &= kD1881_CHIP_ID_MRC_MASK;

		// Angelina prior to Revision B1 requires adjustment to OTP buck parameters
		// <rdar://problem/9892023> N94 Force PWM for buck 0/2 in iboot
		if (data < kD1881_CHIP_ID_MRC_B1) {
			pmu_set_data(PMU_IIC_BUS, kDIALOG_BUCK_CONTROL1, 0x0a, true);	// buck0 sync mode, 1.25A limit per phase
			pmu_set_data(PMU_IIC_BUS, kDIALOG_BUCK_CONTROL3, 0x0e, true);	// buck2 sync mode, 1.55A limit per phase
		}
#endif
		/* Make sure we don't hit the interrupt-squashing
		 * behavior.  See <rdar://problem/8167729> */
		pmu_get_data(PMU_IIC_BUS, kDIALOG_EVENT_A + kDIALOG_EVENT_COUNT - 1, &data);
#if DIALOG_D1881
		// <rdar://problem/9005339>
		pmu_set_data(PMU_IIC_BUS, kDIALOG_BANKSEL, 1, true);
		pmu_get_data(PMU_IIC_BUS, 0xAD, &data);
		if (data & 0x60) {
			pmu_set_data(PMU_IIC_BUS, kD1881_TEST_MODE, 1, true);	// Enable test mode
			pmu_set_data(PMU_IIC_BUS, 0xE4, 8, true);
			pmu_set_data(PMU_IIC_BUS, 0xAD, data & ~0x60, true);	// Clear softstart
			pmu_set_data(PMU_IIC_BUS, 0xE4, 0, true);
			pmu_set_data(PMU_IIC_BUS, kD1881_TEST_MODE, 0, true);	// Disable test mode
		}
		pmu_set_data(PMU_IIC_BUS, kDIALOG_BANKSEL, 0, true);
#endif
#if DIALOG_D2018
		// <rdar://problem/10608368> P102: WLED boost frequency incorrectly set to 2MHz in the initial A0 samples
		pmu_get_data(PMU_IIC_BUS, kDIALOG_CHIP_ID, &data);
		if (data == 0x20) {
			dialog_use_setup(pmu_cold_init_a0_samples, sizeof(pmu_cold_init_a0_samples)/sizeof(pmu_cold_init_a0_samples[0]));
		}
#endif
#if DIALOG_D1972
		// <rdar://problem/11661786> N41 Test iBoot - VDD_SRAM DVC down-ramp
		pmu_get_data(PMU_IIC_BUS, kDIALOG_CHIP_ID, &data);
		if (data == 0x34) {
			pmu_set_data(PMU_IIC_BUS, kD1972_TEST_MODE, 0x1d, true);	// Enable test mode
			pmu_set_data(PMU_IIC_BUS, kDIALOG_BANKSEL, 0x01, true);		// Select bank1
			pmu_set_data(PMU_IIC_BUS, 0xb2, 0xf1, true);			// Change LDO16 soak field
			pmu_set_data(PMU_IIC_BUS, 0xb3, 0x2f, true);			// Change LDO16 pull down strength to 150mA
			pmu_set_data(PMU_IIC_BUS, kDIALOG_BANKSEL, 0x00, true);		// Select bank0
			pmu_set_data(PMU_IIC_BUS, kD1972_TEST_MODE, 0x00, true);	// Disable test mode
		}
#endif /* DIALOG_D1972 */
#if DIALOG_D2257 

		// <rdar://problem/19688297> Re: Aria A0 - Change OTP settings of PMU for better buck response
		data=0xff;
		pmu_get_data(PMU_IIC_BUS, kDIALOG_CHIP_ID, &data);
		if ( data==0x00 ) {
			uint8_t plat_id=0xff;

			pmu_get_data(PMU_IIC_BUS, kDIALOG_PLATFORM_ID, &plat_id);
			if ( plat_id==0x01 || plat_id==0x02 ) {
				data=0xff;

				pmu_get_data(PMU_IIC_BUS, kDIALOG_TRIM_REL_CODE, &data);
				if ( data==0x02 ) {
					int rc;

					rc=pmu_set_data(PMU_IIC_BUS, kDIALOG_TEST_ACCESS, kDIALOG_TEST_ACCESS_ENA, false);
					if ( rc!=0 ) {
						dprintf(DEBUG_CRITICAL, "D2257: cannot enter PMU test mode\n");
					} else {
						rc=pmu_set_data(PMU_IIC_BUS, 0x1161, 0x0a, true);
						if ( rc==0 ) rc=pmu_set_data(PMU_IIC_BUS, 0x1861, 0x0a, true);
						if ( rc==0 && plat_id==0x2 ) rc=pmu_set_data(PMU_IIC_BUS, 0x3af, 0x44, true);
						if ( rc==0 ) rc=pmu_set_data(PMU_IIC_BUS, 0x1792, 0x7B, true);
						if ( rc!=0 ) {
							dprintf(DEBUG_CRITICAL, "D2257: cannot apply fix for PMU A0 silicon (%d)\n", rc);
						} else {
							dprintf(DEBUG_CRITICAL, "D2257: PMU A0 fixups ok\n");
						}

						rc=pmu_set_data(PMU_IIC_BUS, kDIALOG_TEST_ACCESS, kDIALOG_TEST_ACCESS_DIS, false);
						if ( rc!=0 ) panic("D2257: cannot exit PMU test mode");
					}
				}
			}
		}

#endif
	}
}

/* pmu_setup() is only called in iBSS and LLB - not in iBoot or iBEC. */
void pmu_setup(void)
{
	/* Set core voltage to the platform's base soc voltage */
	power_set_soc_voltage(platform_get_base_soc_voltage(), false);

#if !WITH_CPU_APSC
	/* Set cpu voltage to the platform's base cpu voltage */
	power_set_cpu_voltage(platform_get_base_cpu_voltage(), false);
#endif

	/* Set ram voltage to the platform's base ram voltage */
	power_set_ram_voltage(platform_get_base_ram_voltage(), false);

#if DIALOG_D1972 || DIALOG_D2018 || DIALOG_D2045 || DIALOG_D2089 || DIALOG_D2186 || DIALOG_D2207
	uint8_t data;
	/* Enable DWI from the start — we need to ensure this happens on wakeup */
	pmu_get_data(PMU_IIC_BUS, kDIALOG_SYS_CONTROL2, &data);
	data |= kDIALOG_SYS_CONTROL2_DWI_EN;
	pmu_set_data(PMU_IIC_BUS, kDIALOG_SYS_CONTROL2, data, true);
#endif

#if DIALOG_D2018
	/* Enable DWI for BUCK[0,2] & LDO16 */
	pmu_set_data(PMU_IIC_BUS, kD2018_BUCK_DWI, kDIALOG_DWI_BUCK0_DWI_EN | 
							kDIALOG_DWI_BUCK2_DWI_EN | kDIALOG_DWI_LDO16_DWI_EN |
							kDIALOG_DWI_CPUA_EN_CTRL | kDIALOG_DWI_CPUB_EN_CTRL, true);
#endif

	/* Initialize the LDOs to default values */
	dialog_use_setup(pmu_ldo_warm_setup, sizeof(pmu_ldo_warm_setup)/sizeof(pmu_ldo_warm_setup[0]));
	if (!power_is_suspended()) {
	    dialog_use_setup(pmu_ldo_cold_setup, sizeof(pmu_ldo_cold_setup)/sizeof(pmu_ldo_cold_setup[0]));
#if PMU_LDO_COLD_SETUP_AP_DEV
	    if (target_config_dev()) {
	        dialog_use_setup(pmu_ldo_cold_setup_dev, sizeof(pmu_ldo_cold_setup_dev)/sizeof(pmu_ldo_cold_setup_dev[0]));
	    } else {
	        dialog_use_setup(pmu_ldo_cold_setup_ap, sizeof(pmu_ldo_cold_setup_ap)/sizeof(pmu_ldo_cold_setup_ap[0]));
	    }
#endif
	}

#if PMU_HAS_VSEL_LOCK && !DEBUG_BUILD
	unsigned i;

	for (i= 0;  (i != sizeof(ram_rails)/sizeof(ram_rails[0])); i++) {
		dialog_lock_buck_vsel(ram_rails[i].pmu, ram_rails[i].data_reg);
	}
#endif

}

void
pmu_late_init(void)
{
#if DIALOG_D1755
	u_int8_t otp_iset_usb;
	u_int8_t test_mode;
	int error = 0;

	pmu_get_data(PMU_IIC_BUS, kD1755_OTP_ISET_USB, &otp_iset_usb);
	pmu_get_data(PMU_IIC_BUS, kD1755_TEST_MODE, &test_mode);

	if ((otp_iset_usb & kDIALOG_CHARGE_BUCK_CONTROL_ISET_BUCK_MASK) != kDIALOG_CHARGE_BUCK_CONTROL_ISET_BUCK_100) {
		// enter test mode
		test_mode = kD1755_TEST_MODE_TEST_EN;
		pmu_set_data(PMU_IIC_BUS, kD1755_TEST_MODE, test_mode, true);

		// override OTP shadow register: 350mA -> 100mA
		otp_iset_usb = (otp_iset_usb & ~kDIALOG_CHARGE_BUCK_CONTROL_ISET_BUCK_MASK) | kDIALOG_CHARGE_BUCK_CONTROL_ISET_BUCK_100;
		error = pmu_set_data(PMU_IIC_BUS, kD1755_OTP_ISET_USB, otp_iset_usb, true);
	}

	// exit test mode if enabled
	if ((error == 0) && (test_mode & kD1755_TEST_MODE_TEST_EN)) {
		test_mode = 0;
		error = pmu_set_data(PMU_IIC_BUS, kD1755_TEST_MODE, test_mode, true);
	}

	// Dialog recommends that if this sequence fails, the PMU should be reset
	if (error != 0) {
	    pmu_set_data(PMU_IIC_BUS, kD1755_TEST_MODE, kD1755_TEST_MODE_TEST_EN | kD1755_TEST_MODE_RESET_PMU, true);
	    pmu_set_data(PMU_IIC_BUS, kD1755_TEST_MODE, kD1755_TEST_MODE_TEST_EN | kD1755_TEST_MODE_RESET_PMU, true);
	}
#endif
}

extern bool dialog_charger_check_usb_change(const eventRegisters ints_pending, const statusRegisters status);

void
pmu_check_events(bool *powersupply_change_event, bool *button_event, bool *other_wake_event)
{
	eventRegisters ints_pending;
	statusRegisters status;

	/* Read & clear interrupts */
	pmu_read_events(ints_pending);
	pmu_read_status(status);

#if !WITH_HW_CHARGER && !DIALOG_D2238 && !DIALOG_D2255
	*powersupply_change_event = dialog_charger_check_usb_change(ints_pending, status);
#else
	*powersupply_change_event = false;
#endif

	// remove power supply events from ints_pending so that they are not picked up by
	// the secondary checks; powersupply_changed will always be true if it's an
	// event we still care about
	for (int i = 0; i < kDIALOG_EVENT_COUNT; i++) {
	    ints_pending[i] &= ~kDialogEventPwrsupplyMask[i];
	}

	// PMU only wakes on ACC_DET assert, but event fires on either edge.  Remove
	// accessory detach events.
	if (EVENT_FLAG_TEST(ints_pending, kDIALOG_EVENT_ACC_DET_MASK) && !STATUS_FLAG_TEST(status, kDIALOG_STATUS_ACC_DET_MASK)) {
	    EVENT_REGISTERS_GET_BYTE(ints_pending, kDIALOG_EVENT_ACC_DET_MASK) &= ~EVENT_FLAG_GET_BIT(kDIALOG_EVENT_ACC_DET_MASK);
	}

	*button_event = EVENT_FLAG_TEST(ints_pending, kDIALOG_EVENT_BUTTONS_MASK);
	*other_wake_event = EVENT_FLAG_TEST(ints_pending, kDIALOG_EVENT_ACC_DET_MASK) || EVENT_FLAG_TEST(ints_pending, kDIALOG_EVENT_ALARM_MASK);
}

static int
do_pmem_cmd(int argc, struct cmd_arg *args)
{
#if PMU_HAS_RAM && !DIALOG_D2238
	bool success;
	char buffer[kDIALOG_EXT_MEM_CAL_SIZE];
	
	if ( argc<2 || strcmp("read", args[1].str)==0 ) {
		size_t count=kDIALOG_EXT_MEM_CAL0_SIZE;
		size_t offset=0;
		
		if ( argc>2 ) count=atoi( args[2].str );
		if ( argc>3 ) offset=atoi( args[3].str );

		success=power_load_memory_calibration(buffer, count);
		if ( !success ) {
			printf("failed to read %zu bytes\n", count);
		} else {
			for (size_t i=0; i<count; i++) {
				printf("%02x", buffer[i+offset]);
				if ( (i+1)%4==0 ) printf(" ");
				if ( (i+1)%64==0 ) printf("\n");
			}
			printf("\n");
		}		
	} else if ( argc>3 && strcmp("write", args[1].str)==0 ) {
					
		success=power_load_memory_calibration(buffer, kDIALOG_EXT_MEM_CAL1_SIZE);
		if ( !success ) {
			printf("failed to read %d bytes\n", kDIALOG_EXT_MEM_CAL1_SIZE);
		} else {
			size_t offset=atoi( args[2].str );
			int i, overflow=0;
	
			for (i=3; !overflow && i<argc; i++)  {
				const int index=offset+i-3;			
				const uint8_t val=atoi(args[i].str)&0xff;
				
				overflow=index>kDIALOG_EXT_MEM_CAL_SIZE ;
				if ( overflow ) {
					printf("%#x@%d is out of bounds\n", val, index);
				} else {				
					buffer[index]=val;
				}
			}
			
			if ( !overflow ) {
				size_t count=kDIALOG_EXT_MEM_CAL0_SIZE;
	
				if ( offset+i-3>kDIALOG_EXT_MEM_CAL0_SIZE ) count+=kDIALOG_EXT_MEM_CAL1_SIZE;

				success=power_store_memory_calibration(buffer, count);
				if ( !success )  printf("failed to write\n");					
			}
		}
		
	} else {
		printf("usage:\n\t%s <command>\n", args[0].str);
		printf("\t\tread  [<count>] [<offset>]    show memory calibration values\n");
		printf("\t\twrite <offset> byte byte ...   change byte(s) at offset\n");			
		return -1;	
	} 

	return 0;	
#else
	printf("not available on this target\n");
	return -1;	
#endif
}

static inline uint16_t pmu_index2gpio_addr(uint32_t index)
{
#if DIALOG_D2207
    if (index >= 17) return kD2207_GPIO18 + (index-17)*3;
#endif
#if PMU_HAS_GPIO_CONF
    return kDIALOG_SYS_GPIO_REG_START + index*2;
#endif
    return kDIALOG_SYS_GPIO_REG_START + index;
}

static
void dump_gpiocfg_(int gpion) // gpio[1-21]
{
	int result;
	uint8_t val[2];
	const uint16_t addr=pmu_index2gpio_addr(gpion-1);

	result=pmu_get_data(PMU_IIC_BUS, addr, val);
#if PMU_HAS_GPIO_CONF
	result=pmu_get_data(PMU_IIC_BUS, addr+1, &val[1]);
#endif
	if ( result!=0 ) {
		printf("gpio%02d: result=%d\n", gpion, result);
	} else {
		printf("gpio%02d: 0x%04x : cnfga=%02x", gpion, addr, val[0]);
#if PMU_HAS_GPIO_CONF
		printf(" cnfgb=%02x", val[1]);
#endif
		printf("\n");
	}
}

static void dump_railcfg_(uint32_t index)
{
	int result;
	uint8_t vsel, ena;
	char label[32];
	const struct ldo_params *p=&LDOP[index];

#if DIALOG_D2257
	const uint8_t last_ldo=16;
	const uint8_t last_buck=25;
#else
	const uint8_t last_ldo=15;
	const uint8_t last_buck=24;
#endif 

	if (index<last_ldo) {
		snprintf(label, sizeof(label), "LDO%02d", index+1);
	} else if ( index<last_buck ) {
		snprintf(label, sizeof(label), "BUCK%d", index-last_ldo);
	} else {
		snprintf(label, sizeof(label), "      ");		
	}


	if ( p->ldoreg && p->actreg ) {
		result=pmu_get_data(PMU_IIC_BUS, p->ldoreg, &vsel);
		if ( result==0 ) result=pmu_get_data(PMU_IIC_BUS, p->actreg, &ena);
		if ( result!=0 ) {
			printf("rail%02d: result=%d\n", index, result);
		} else {
			printf("rail%02d: %s : ena[%04x]=%02x vsel[%04x]=%02x",
				index, label, p->actreg, ena, p->ldoreg, vsel);

#if DIALOG_D2255
			uint8_t actual=0xff, fvsel=0xff;
			uint16_t areg=0xffff, fvreg=0xffff;

			if ( index<15 ) {
				areg=p->ldoreg+1;				
			} else if ( index>=15 ) {
				areg=p->ldoreg+2;
				fvreg=p->ldoreg-1;
			}

			if ( areg!=0xffff ) {
				result=pmu_get_data(PMU_IIC_BUS, areg, &actual);
				if ( result!=0 ) actual=0xff;
			}

			if ( fvreg!=0xffff ) {
				result=pmu_get_data(PMU_IIC_BUS, fvreg, &fvsel);
				if ( result!=0 ) actual=0xff;
				printf(" fvsel[%04x]=%02x", fvreg, fvsel);
			}
			
			// TODO: convert value to voltage
			if ( ena ) printf(" actual[%04x]=%02x", areg, actual);
#endif
			printf("\n");
		}
	} else {
		printf("rail%02d: %s N/A\n", index, label);
	}

}

static void do_dctl_cmd_help()
{
		printf("\tdctl gpio[<n>] [cfg <config>][<value> <direction>]\n");
		printf("\tdctl adc <channel>\n");
		printf("\tdctl rail[<n>]\n");
}

// dctl gpio
// dctl gpio<n>
// dctl gpio<n> cfg configa
// dctl gpio<n> value direction
// dctl adc <chan>
// dctl rail
// dctl rail<n>
// dctl uvwarn thresholdMV
static int
do_dctl_cmd(int argc, struct cmd_arg *args)
{

	if ( argc==1 ) {
		do_dctl_cmd_help();
	} else if ( strncmp("gpio", args[1].str, 4)==0 ) {

		if ( args[1].str[4]==0 ) {
			for (int gpion=1; gpion<=kDIALOG_GPIO_COUNT; gpion++) {
				dump_gpiocfg_(gpion);
			}
		} else if ( argc==2 ) {
			const int gpion=atoi( &args[1].str[4] );
			if ( gpion>0 && gpion<=kDIALOG_GPIO_COUNT ) dump_gpiocfg_(gpion);
		} else if ( argc==4 && args[2].str[0]=='c' ) {
			const int gpion=atoi( &args[1].str[4] );
			if ( gpion>0 && gpion<=kDIALOG_GPIO_COUNT ) {
				const uint32_t cfga=strtoul( args[3].str,NULL,0 );
				
				power_gpio_configure(gpion-1, cfga);
				dump_gpiocfg_(gpion);
			}

		} else if ( argc==4 ) {
			const int gpion=atoi( &args[1].str[4] );

			if ( gpion>0 && gpion<=kDIALOG_GPIO_COUNT ) {
				const uint32_t direction=atoi( args[2].str );
				const uint32_t value=atoi( args[3].str );

				const int rc=power_set_gpio(gpion-1, direction, value);
				if ( rc!=0 )  printf("gpio%02d: direction=%x value=%x (%d)\n", gpion, direction, value, rc);
				dump_gpiocfg_(gpion);
			}
		}
	} else if ( strncmp("rail", args[1].str, 4)==0 ) {

		if ( args[1].str[4]==0 ) {
			for (int index=0; index<NUM_LDOS; index++) {
				dump_railcfg_(index);
			}
		} else if ( argc==2 ) {
			const int index=atoi( &args[1].str[4] );
			if ( index<NUM_LDOS ) dump_railcfg_(index);
		} else if ( argc>=3 ) {
			// dctl rail<n>0 ena [vsel]
			const int index=atoi( &args[1].str[4] );

			if ( index<NUM_LDOS ) {
				int result;
				const struct ldo_params *p=&LDOP[index];	

				if ( !p->actreg ) {
					printf("rail%d: enable register not available", index);
				} else {
					const uint32_t ena=atoi( args[2].str )&0x1;
				
					result=pmu_set_data(PMU_IIC_BUS, p->actreg, ena, true);
					if ( result!=0 ) {
						printf("rail%d: enable error (%d)", index, result);
					} else if ( argc==4 ) {
						if ( !p->ldoreg ) {
							printf("rail%d: vsel register not available", index);
						} else {
							const uint32_t vsel=atoi( args[3].str )&0xff;

							result=pmu_set_data(PMU_IIC_BUS, p->ldoreg, vsel, true);
							if ( result!=0 ) {
								printf("rail%d: vsel=%x error (%d)", index, vsel, result);
							} 
						}
					}
				} 

				dump_railcfg_(index);
			}

		}
	} else if ( strncmp("uvwarn", args[1].str, 5)==0 && argc>1) {
		const uint32_t thresholdMV=(uint32_t)atoi( args[2].str );

		int result=pmu_uvwarn_config(0, thresholdMV);
		printf("uvwarn: threshold=%dmV (%d)\n", thresholdMV, result);

	} else if ( strncmp("adc", args[1].str, 4)==0 && argc>1) {
		unsigned value = 0;
		const int chan=atoi( args[2].str );

		int result=dialog_read_adc(0, chan, &value);
		if ( result<0 ) {
			printf("error: chan=%d (%d)\n", chan, result);
		} else {			
			printf("adc: chan=%d value=%u\n", chan, value);
		}

	} else {
		printf("usage:\n\t%s <command>\n", args[0].str);
		do_dctl_cmd_help();
		return -1;
	}

	return 0;
}

#if WITH_RECOVERY_MODE
MENU_COMMAND_DEBUG(pmem, do_pmem_cmd, "Manage memory calibration nvram", NULL);
MENU_COMMAND_DEBUG(dctl, do_dctl_cmd, "Dialog control menu", NULL);
#endif

static uint8_t nvram_key_map[kPowerNVRAMPropertyCount] = {
	0x0F,		// kPowerNVRAMiBootStateKey
	0x00,		// kPowerNVRAMiBootDebugKey
	0x01,		// kPowerNVRAMiBootStageKey
	0x02,		// kPowerNVRAMiBootErrorCountKey
	0x03,		// kPowerNVRAMiBootErrorStageKey
	0x10,		// kPowerNVRAMiBootMemCalCAOffset0Key
	0x11,		// kPowerNVRAMiBootMemCalCAOffset1Key
	0x12,		// kPowerNVRAMiBootMemCalCAOffset2Key
	0x13,		// kPowerNVRAMiBootMemCalCAOffset3Key
	0x0D,		// kPowerNVRAMiBootBootFlags0Key
	0x0E,		// kPowerNVRAMiBootBootFlags1Key
	0x14,		// kPowerNVRAMiBootEnterDFUKey
};

static int dialog_get_nvram(int dev, uint8_t offset, uint8_t *data);
static int dialog_set_nvram(int dev, uint8_t offset, uint8_t data);

int
power_get_nvram(uint8_t key, uint8_t *data)
{
	uint8_t offset;
	
	if (key >= kPowerNVRAMPropertyCount) return -1;
	offset = nvram_key_map[key];
	
	return dialog_get_nvram(PMU_IIC_BUS, offset, data);
}

int
power_set_nvram(uint8_t key, uint8_t data)
{
	uint8_t offset;
	
	if (key >= kPowerNVRAMPropertyCount) return -1;
	offset = nvram_key_map[key];
	
	return dialog_set_nvram(PMU_IIC_BUS, offset, data);
}

void
power_clr_events(int wake)
{
	/* Read & clear interrupts */
	eventRegisters events;
	pmu_read_events(events);

	if (wake)
	{
		/* Unmask pmu wake interrupts */
		pmu_write_mask(kDialogEventIntMasks);
	}
}

static void
dialog_shutdown(u_int8_t type)
{
#if !DIALOG_D1755
	if (type == kDIALOG_SYS_CONTROL_STANDBY)
	{
		int rc;
		// turn off all GPIO wake sources so that spurious events will not prevent
		// the system from turning off; Ashley will reload OTP once entering standby,
		// so OTP wake sources will still turn the system on (this was not true on
		// previous PMUs).
		for (int i = 0; i < kDIALOG_GPIO_COUNT; i++) {
		    UInt8 gpioData;
		    UInt16 addr=pmu_index2gpio_addr(i);

		    rc=pmu_get_data(PMU_IIC_BUS, addr, &gpioData);
		    if ( rc!=0 ) {
				dprintf(DEBUG_SPEW, "gpio%d: cannot read (%d)\n", i+1, rc);
		    } else if (!IS_GPIO_OUTPUT(gpioData) && (gpioData & kDIALOG_SYS_GPIO_INPUT_WAKE) ) {
				gpioData &= ~kDIALOG_SYS_GPIO_INPUT_WAKE;
				rc=pmu_set_data(PMU_IIC_BUS, addr, gpioData, false);
				if ( rc!=0 ) dprintf(DEBUG_SPEW, "gpio%d: cannot read (%d)\n", i+1, rc);
		    }
		}
	}
#endif

	pmu_set_data(PMU_IIC_BUS, kDIALOG_SYS_CONTROL, type, false);
	enter_critical_section();
	while (true);
}

void
pmu_shutdown(void)
{
	dialog_shutdown(kDIALOG_SYS_CONTROL_STANDBY);
}

void
pmu_suspend(void)
{
	dialog_shutdown(kDIALOG_SYS_CONTROL_HIBERNATE);
}

void
pmu_set_backlight_enable(uint32_t backlight_level)
{
#if WITH_SWI_BACKLIGHT
	extern int swi_backlight_enable(u_int32_t iset_code, u_int32_t backlight_level);

	swi_backlight_enable(kDIALOG_SWI_WLED_ISET, backlight_level);
#elif PMU_HAS_WLED
	uint32_t cnt, count = sizeof(pmu_backlight_enable)/sizeof(pmu_backlight_enable[0]);
	struct pmu_setup_struct		tmp[count];
	const struct pmu_setup_struct	*setup;

	if (backlight_level) {
		memcpy(tmp, pmu_backlight_enable, count * sizeof(struct pmu_setup_struct));
		setup = tmp;
		if ((backlight_level >= 1) && (backlight_level <= MAX_BACKLIGHT_LEVEL)) {
			UInt8 backlight_level_1;
#if DIALOG_D1755
			backlight_level_1 = backlight_level;
#elif DIALOG_D2257
			UInt8 backlight_level_2;
			backlight_level_1 = (backlight_level >> 8) & 0x7;       // msb bits
			backlight_level_2 = backlight_level & 0xff;
#else
			UInt8 backlight_level_2;
			backlight_level_1 = backlight_level >> 3;
			backlight_level_2 = backlight_level & 0x7;
#endif
			for (cnt = 0; cnt < count; cnt++) {
				if (tmp[cnt].reg == kDIALOG_WLED_ISET) {
					tmp[cnt].value = backlight_level_1;
				}
#if !DIALOG_D1755
				if (tmp[cnt].reg == kDIALOG_WLED_ISET2) {
					tmp[cnt].value = backlight_level_2;
				}
#endif
			}
		}
	} else {
		count = sizeof(pmu_backlight_disable)/sizeof(pmu_backlight_disable[0]);
		setup = pmu_backlight_disable;
	}

	dialog_use_setup(setup, count);
#endif
}


#if DIALOG_D1881 || DIALOG_D1946 || DIALOG_D1972 || DIALOG_D1974 || DIALOG_D2045 || DIALOG_D2186 || DIALOG_D2238
#define POWER_SOC_VMIN (600)
#define POWER_SOC_VMAX (1400)
#define DIALOG_CORE_BUCKOUT(mv)		(((mv-600)*1000)+3124)/3125
#define DIALOG_CORE_BUCKMV(vsel)	(600 + ((((vsel) * 3125)) / 1000))
#elif DIALOG_D2018
#define	POWER_SOC_VMIN	(725)
#define POWER_SOC_VMAX	(1521)
#define DIALOG_CORE_BUCKOUT(mv)		(((mv-725)*1000)+3124)/3125
#define DIALOG_CORE_BUCKMV(vsel)	(725 + ((((vsel) * 3125)) / 1000))
#elif DIALOG_D2089 || DIALOG_D2207 || DIALOG_D2255 || DIALOG_D2257
#define	POWER_SOC_VMIN	(600)
#define POWER_SOC_VMAX	(1396)
#define DIALOG_CORE_BUCKOUT(mv)		(((mv-600)*1000)+3124)/3125
#define DIALOG_CORE_BUCKMV(vsel)	(600 + ((((vsel) * 3125)) / 1000))
#else
#define POWER_SOC_VMIN (725)
#define POWER_SOC_VMAX (1500)
#define DIALOG_CORE_BUCKOUT(mv)		((mv-725)+24)/25
#define DIALOG_CORE_BUCKMV(vsel)	(725 + ((vsel) * 25))
#endif

#if DIALOG_D2238
int power_set_soc_voltage(unsigned mv, int override)
{
	return 0;
}
#else
int 
power_set_soc_voltage(unsigned mv, int override)
{
	u_int8_t tmp;

	tmp = 0;

	if ((mv < POWER_SOC_VMIN) || (mv > POWER_SOC_VMAX)) return -1;

	unsigned i;
	for (i= 0;  (i != sizeof(soc_rails)/sizeof(soc_rails[0])); i++) {
		pmu_set_data(soc_rails[i].pmu, soc_rails[i].data_reg, DIALOG_CORE_BUCKOUT(mv), true);

#if DIALOG_D1755 || DIALOG_D1815
		if (soc_rails[i].data_reg == kDIALOG_BUCK1) {
			// Be sure to set the BUCK1 selector to 0; this requires/assumes VBUCK1_SWI_EN=0
			pmu_set_data(soc_rails[i].pmu, kDIALOG_BUCK1_SEL, 0, true);
			pmu_get_data(soc_rails[i].pmu, soc_rails[i].ctrl_reg, &tmp);
			tmp |= kDIALOG_BUCK_CONTROL_BUCK1_GO;
			pmu_set_data(soc_rails[i].pmu, soc_rails[i].ctrl_reg, tmp, true);
		}
#endif
	}

	return 0;
}

#endif

#if DIALOG_D1881 || DIALOG_D1946 || DIALOG_D1972 || DIALOG_D1974 || DIALOG_D2045 || DIALOG_D2186 || DIALOG_D2238
#define POWER_CPU_VMIN (600)
#define POWER_CPU_VMAX (1400)
#define DIALOG_CPU_BUCKOUT(mv)		(((mv-600)*1000)+3124)/3125
#define DIALOG_CPU_BUCKMV(vsel)		(600 + ((((vsel) * 3125)) / 1000))
#elif DIALOG_D2018
#define	POWER_CPU_VMIN	(725)
#define POWER_CPU_VMAX	(1521)
#define DIALOG_CPU_BUCKOUT(mv)		(((mv-725)*1000)+3124)/3125
#define DIALOG_CPU_BUCKMV(vsel)		(725 + ((((vsel) * 3125)) / 1000))
#elif DIALOG_D2089 || DIALOG_D2207 || DIALOG_D2255
#define	POWER_CPU_VMIN	(600)
#define POWER_CPU_VMAX	(1396)
#define DIALOG_CPU_BUCKOUT(mv)		(((mv-600)*1000)+3124)/3125
#define DIALOG_CPU_BUCKMV(vsel)		(600 + ((((vsel) * 3125)) / 1000))
#elif DIALOG_D2255
#define	POWER_CPU_VMIN	(500)
#define POWER_CPU_VMAX	(1296)
#define DIALOG_CPU_BUCKOUT(mv)		(((mv-500)*1000)+3124)/3125
#define DIALOG_CPU_BUCKMV(vsel)		(500 + ((((vsel) * 3125)) / 1000))
#elif DIALOG_D2257
#define	POWER_CPU_VMIN	(450)
#define	POWER_CPU_VMAX	(1250)
#define DIALOG_CPU_BUCKOUT(mv)		(((mv-450)*1000)+3124)/3125
#define DIALOG_CPU_BUCKMV(vsel)		(450 + ((((vsel) * 3125)) / 1000))
#endif

#if DIALOG_D1881 || DIALOG_D1946 || DIALOG_D1972 || DIALOG_D1974 || DIALOG_D2018 || DIALOG_D2045 || DIALOG_D2089 || DIALOG_D2186 || DIALOG_D2207 || DIALOG_D2255 || DIALOG_D2257
int 
power_set_cpu_voltage(unsigned mv, int override)
{
	u_int8_t tmp;

	tmp = 0;

	if ((mv < POWER_CPU_VMIN) || (mv > POWER_CPU_VMAX)) return -1;

	unsigned i;
	for (i= 0;  (i != sizeof(cpu_rails)/sizeof(cpu_rails[0])); i++) {
		pmu_set_data(cpu_rails[i].pmu, cpu_rails[i].data_reg, DIALOG_CPU_BUCKOUT(mv), true);
	}

	return 0;
}

#else
int 
power_set_cpu_voltage(unsigned mv, int override)
{
	return -1;
}
#endif

#if DIALOG_D1972 || DIALOG_D2018 || DIALOG_D2045 || DIALOG_D2089 || DIALOG_D2186 || DIALOG_D2207 || DIALOG_D2255 || DIALOG_D2257
#define POWER_RAM_VMIN (600)
#define POWER_RAM_VMAX (1400)
#define DIALOG_RAM_BUCKOUT(mv)		(((mv-600)*1000)+3124)/3125

int
power_set_ram_voltage(unsigned mv, int override)
{
	u_int8_t tmp;

	tmp = 0;

	if ((mv < POWER_RAM_VMIN) || (mv > POWER_RAM_VMAX)) return -1;

	unsigned i;
	for (i= 0;  (i != sizeof(ram_rails)/sizeof(ram_rails[0])); i++) {
		pmu_set_data(ram_rails[i].pmu, ram_rails[i].data_reg, DIALOG_RAM_BUCKOUT(mv), true);
	}

	return 0;
}

#else

int
power_set_ram_voltage(unsigned mv, int override)
{
	return -1;
}

#endif

// TODO: deprecate, use power_get_rail_value() instead
int power_get_buck_value(int buck, unsigned mv, u_int32_t *buffer)
{
	int ret = 0;

	switch (buck) {
#if DIALOG_D1881 || DIALOG_D1946 || DIALOG_D1972 || DIALOG_D1974 || DIALOG_D2018 || DIALOG_D2045 || DIALOG_D2089 || DIALOG_D2186 || DIALOG_D2207 || DIALOG_D2238
	case 0: 
	case 1: 
		*buffer = DIALOG_CPU_BUCKOUT(mv);
		break;
	case 2: 
		*buffer = DIALOG_CORE_BUCKOUT(mv);
		break;
#endif
// compatibility only
#if DIALOG_D1972 || DIALOG_D2018
	case 16:
		*buffer = DIALOG_RAM_BUCKOUT(mv);
		break;
#endif
	default: ret = -1; break;
	}

	return ret;
}

int
power_get_rail_value(int rail, unsigned mv, u_int32_t *buffer)
{
    int ret = 0;
    
#if DIALOG_D2255 || DIALOG_D2257
    switch (rail) {
        case POWER_RAIL_CPU:
        case POWER_RAIL_SOC:
        case POWER_RAIL_VDD_FIXED:
        case POWER_RAIL_CPU_RAM:
        case POWER_RAIL_GPU:
        case POWER_RAIL_GPU_RAM:
            if ( mv<target_rails[rail].min_mv || mv>target_rails[rail].max_mv ) {
                *buffer=0; // safe?
                ret=-1;
            } else {
                *buffer=(( (mv-target_rails[rail].min_mv)*1000 ) + target_rails[rail].step_uv-1 ) / target_rails[rail].step_uv;
            }
            break;
        default:
            ret = -1;
            break;

    }
#elif DIALOG_D1881 || DIALOG_D1946 || DIALOG_D1972 || DIALOG_D1974 || DIALOG_D2018 || DIALOG_D2045 || DIALOG_D2089 || DIALOG_D2186 || DIALOG_D2207 || DIALOG_D2238 || DIALOG_D1972 || DIALOG_D2018
    //  compat. rail is a buck 
    ret=power_get_buck_value(rail, mv, buffer);
#endif
    
    return ret;
}

int
power_convert_dwi_to_mv(int rail, u_int32_t dwival)
{
    int ret;
    
    switch (rail) {
#if DIALOG_D2255 || DIALOG_D2257
        case POWER_RAIL_CPU:
        case POWER_RAIL_SOC:
        case POWER_RAIL_VDD_FIXED:
        case POWER_RAIL_CPU_RAM:
        case POWER_RAIL_GPU:
        case POWER_RAIL_GPU_RAM:
            ret=target_rails[rail].min_mv+(dwival*target_rails[rail].step_uv)/1000 ;
            if ( ret>target_rails[rail].max_mv ) ret=-1;
            break;		
#elif DIALOG_D1881 || DIALOG_D1946 || DIALOG_D1972 || DIALOG_D1974 || DIALOG_D2018 || DIALOG_D2045 || DIALOG_D2089 || DIALOG_D2186 || DIALOG_D2207 || DIALOG_D2238
        case 0:
        case 1:
            ret = DIALOG_CPU_BUCKMV(dwival);
            break;
        case 2:
            ret = DIALOG_CORE_BUCKMV(dwival);
            break;
#endif
        default: ret = -1; break;
    }
    
    return ret;
}

int
power_set_display_voltage_offset(u_int32_t voltage_offset, bool fixed_boost)
{
#if DIALOG_D1755 || !PMU_HAS_WLED
	return -1;
#elif !defined(TARGET_DISPLAY_VOLTAGE_BASE) || !defined(TARGET_DISPLAY_VOLTAGE_SCALE) || !defined(TARGET_DISPLAY_BOOST_LDO) || !defined(TARGET_DISPLAY_BOOST_OFFSET)
	return -1;
#else

	u_int32_t display_voltage = TARGET_DISPLAY_VOLTAGE_BASE - (voltage_offset * TARGET_DISPLAY_VOLTAGE_SCALE);
	u_int8_t lcm_controlx_data, lcm_boost_data;

	// Convert display_voltage to LCM_CONTROLx value
	lcm_controlx_data = (display_voltage - 5000 + 49) / 50;

	// Set LCM_CONTROLx register
	pmu_set_data(PMU_IIC_BUS, TARGET_DISPLAY_BOOST_LDO, lcm_controlx_data, true);

	if (false == fixed_boost) {
		// Wait for LCMx to settle... 1ms is probably enough, but 10ms for now
		task_sleep(10 * 1000);

		// LCM_BOOST should be TARGET_DISPLAY_BOOST_OFFSET higher then LCM_CONTROLx
		lcm_boost_data = lcm_controlx_data + TARGET_DISPLAY_BOOST_OFFSET / 50;

		// Set LCM_BOOST register
		pmu_set_data(PMU_IIC_BUS, kDIALOG_LCM_BOOST, lcm_boost_data, true);
	}

	return 0;
#endif
}

int
power_get_boot_flag(void)
{
	if ((EVENT_FLAG_GET_BIT(kDIALOG_EVENT_HIB_MASK) & boot_flags) != 0) return kPowerBootFlagWarm;

	if (!cold_boot_ints_valid) {
		/* Read & clear interrupts */
		pmu_read_events(cold_boot_ints);
		cold_boot_ints_valid = true;
	}

	// if a cold boot, distinguish between two types of boot:
	// - boot immediately
	// - boot only if the power button is pressed and held
	// For example, charger attach, alarm wake, or 30-pin accessory insert always powers up.
	// A short tap of the power button or E75 accessory insert does not.

	bool button_event = false;
	eventRegisters wakeEvents;
	memcpy(wakeEvents, cold_boot_ints, sizeof(eventRegisters));

	const UInt16 cold_button_events[] = {
	    kDIALOG_EVENT_PWR_BUTTON_MASK,
#if WITH_HW_TRISTAR
	    kDIALOG_EVENT_ACC_DET_MASK,
#endif
	};
	const size_t num_cold_button_events = sizeof(cold_button_events)/sizeof(UInt16);
	
	for (unsigned i = 0; i < num_cold_button_events; i++) {
	    UInt16 event = cold_button_events[i];
	    if (EVENT_FLAG_TEST(wakeEvents, event)) {
		button_event = true;
		wakeEvents[EVENT_FLAG_GET_BYTE(event)] &= ~EVENT_FLAG_GET_BIT(event);
	    }
	}

	// if we saw one of the events that would result in a cold-button boot,
	// but there are other events pending, or USB is attached (but somehow failed
	// to flag an event), continue to boot.
	//
	// Ignore all buttons from wake mask, not just power, since
	// although they (e.g., home button) are "wake" events, they are not "power-on" events.
	if (button_event
	    && !(EVENT_REGISTER_TEST_MASK(kDialogEventNotButtonMasks, wakeEvents))
	    && !(power_has_usb()))
		return kPowerBootFlagColdButton;

	return kPowerBootFlagCold;
}

void
pmu_will_resume(void)
{
	// store boot flag for OS use (re-use same NVRAM byte used for voltage between LLB and iBoot)
	power_set_nvram(kPowerNVRAMiBootBootFlags0Key, boot_flags);
}

int 
power_read_dock_id(unsigned *id)
{
#if !DIALOG_D2238 && !DIALOG_D2255
	int		 result;
	unsigned value = 0;

	result = dialog_read_adc(PMU_IIC_BUS, kDIALOG_ADC_CONTROL_MUX_SEL_ACC_ID, &value);

	// Only want the high 4 bits
	*id = value >> (kDIALOG_ADC_RESOLUTION_BITS - 4);

	return result;
#else
	return -1;
#endif
}

int 
power_set_gpio(uint32_t index, uint32_t direction, uint32_t value)
{
	int      result;
	uint8_t  data;
	
	if (index >= kDIALOG_GPIO_COUNT) return -1;

	/* Read kDIALOG_SYS_GPIO_X register */
	result = pmu_get_data(PMU_IIC_BUS, pmu_index2gpio_addr(index), &data);
	if (result != 0) {
		dprintf(DEBUG_SPEW, "gpio%d: cannot read (%d)\n", index+1, result);
		return result;
	}

	data &= ~(kDIALOG_SYS_GPIO_DIRECTION_MASK | kDIALOG_SYS_GPIO_OUTPUT_LEVEL_HIGH);
	switch (direction) {
	case 2:
		data |= kDIALOG_SYS_GPIO_DIRECTION_OUT_32KHZ;
		break;
	case 1:
		data |= kDIALOG_SYS_GPIO_DIRECTION_OUT;
		data |= value ? kDIALOG_SYS_GPIO_OUTPUT_LEVEL_HIGH : kDIALOG_SYS_GPIO_OUTPUT_LEVEL_LOW;
		break;
	case 0:
	default:
		data |= kDIALOG_SYS_GPIO_DIRECTION_IN_LEVEL_HIGH;
		break;
	}

	/* Set kDIALOG_SYS_GPIO_X register */
	result=pmu_set_data(PMU_IIC_BUS, pmu_index2gpio_addr(index), data, true);
	if ( result!=0 ) dprintf(DEBUG_SPEW, "gpio%d: cannot set (%d)\n", index+1, result);

	return result;
}

bool
power_get_gpio(uint32_t index)
{
	u_int8_t reg, mask, data;

	if (index >= kDIALOG_GPIO_COUNT) return false;

	reg = EVENT_FLAG_GET_BYTE(kDIALOG_STATUS_GPIO_MASK(index));
	mask = EVENT_FLAG_GET_BIT(kDIALOG_STATUS_GPIO_MASK(index));

	int result=pmu_get_data(PMU_IIC_BUS, kDIALOG_STATUS_A + reg, &data);
	if ( result!=0 ) dprintf(DEBUG_SPEW, "gpio%d: cannot read (%d)\n", index+1, result);

	return (data & mask) != 0;
}

void
power_gpio_configure(uint32_t index, u_int32_t config)
{
	if (index >= kDIALOG_GPIO_COUNT) return;
	
	/* Set kDIALOG_SYS_GPIO_X register */
	int result=pmu_set_data(PMU_IIC_BUS, pmu_index2gpio_addr(index), config, true);
	if ( result!=0 ) dprintf(DEBUG_SPEW, "gpio%d: cannot configure (%d)\n", index+1, result);
}


int 
power_set_ldo(uint32_t ldoSelect, uint32_t voltage)
{
	u_int8_t data;

	// The argument is 1-based, so decrement to be an array index
	ldoSelect--;

	// Bounds check and make sure the "LDO" isn't a switch: must have a valid min voltage
	if ((ldoSelect >= NUM_LDOS) || (LDOP[ldoSelect].minv == 0)) return -1;

	// Range check the requested voltage
	if ((voltage < LDOP[ldoSelect].minv) || (voltage > (uint32_t)(LDOP[ldoSelect].step * 
								      LDOP[ldoSelect].maxs + 
								      LDOP[ldoSelect].minv)))
		return -1;

	// Construct the encoding and write the appropriate register
	data = ((voltage - LDOP[ldoSelect].minv)/LDOP[ldoSelect].step);
	pmu_set_data(PMU_IIC_BUS, LDOP[ldoSelect].ldoreg, data, true);

	return 0;
}

int 
power_enable_ldo(uint32_t ldo, bool enable)
{
	u_int8_t data;

	ldo--;
	if (ldo >= NUM_LDOS) return -1;

	pmu_get_data(PMU_IIC_BUS, LDOP[ldo].actreg, &data);

	if (enable) data |= LDOP[ldo].actmask;
	else data &= ~LDOP[ldo].actmask;

	pmu_set_data(PMU_IIC_BUS, LDOP[ldo].actreg, data, true);

	return 0;
}

static uint32_t nvram_valid_mask;
static uint8_t  nvram_shadow[kDIALOG_MEMBYTE_LAST - kDIALOG_MEMBYTE0 + 1];

static int 
dialog_get_nvram(int dev, uint8_t offset, uint8_t *data)
{
	int result;
	
	if (offset > (kDIALOG_MEMBYTE_LAST - kDIALOG_MEMBYTE0)) return -1;
	if (!((1 << offset) & nvram_valid_mask)) {
		result = pmu_get_data(dev, kDIALOG_MEMBYTE0 + offset, &nvram_shadow[offset]);
		if (result != 0) return result;
		nvram_valid_mask |= (1 << offset);
	}
	
	*data = nvram_shadow[offset];
	
	return 0;
}

static int 
dialog_set_nvram(int dev, uint8_t offset, uint8_t data)
{
	if (offset > (kDIALOG_MEMBYTE_LAST - kDIALOG_MEMBYTE0)) return -1;
	if (((1 << offset) & nvram_valid_mask) && (data == nvram_shadow[offset]))
		return 0;
	
	nvram_shadow[offset] = data;
	nvram_valid_mask |= (1 << offset);
	
	return pmu_set_data(dev, kDIALOG_MEMBYTE0 + offset, data, 0);
}

utime_t
power_get_calendar_time(void)
{
    uint8_t offset_secs[4], offset_ticks[2];
    static const uint64_t tick_frequency = 32768;

    if (pmu_read_nvram(kDIALOG_MEMBYTE0 + 4, offset_secs, sizeof(offset_secs)) != 0) return 0;
    if (pmu_read_nvram(kDIALOG_MEMBYTE0 + 21, offset_ticks, sizeof(offset_ticks)) != 0) return 0;

    uint64_t offset = ((uint64_t)(offset_secs[3]) << (24 + 15))
	+ ((uint64_t)(offset_secs[2]) << (16 + 15))
	+ ((uint64_t)(offset_secs[1]) << (8 + 15))
	+ ((uint64_t)(offset_secs[0]) << (15))
	+ ((uint64_t)(offset_ticks[1] & 0x7F) << (8))
	+ (uint64_t)(offset_ticks[0]);

    uint64_t ticks;
    
#if PMU_HAS_32K_RTC
    // registers are latched when reading the first byte; read all six at once
    UInt8 data[(kDIALOG_RTC_SECOND_D - kDIALOG_RTC_SUB_SECOND_A) + 1];

    if (pmu_read_nvram(kDIALOG_RTC_SUB_SECOND_A, data, sizeof(data)) != 0) return 0;

    ticks = ((uint64_t)(data[kDIALOG_RTC_SUB_SECOND_A - kDIALOG_RTC_SUB_SECOND_A])>>1)
	| ((uint64_t)(data[kDIALOG_RTC_SUB_SECOND_B - kDIALOG_RTC_SUB_SECOND_A]) << 7)
	| ((uint64_t)(data[kDIALOG_RTC_SECOND_A - kDIALOG_RTC_SUB_SECOND_A]) << 15)
	| ((uint64_t)(data[kDIALOG_RTC_SECOND_B - kDIALOG_RTC_SUB_SECOND_A]) << 23)
	| ((uint64_t)(data[kDIALOG_RTC_SECOND_C - kDIALOG_RTC_SUB_SECOND_A]) << 31)
	| ((uint64_t)(data[kDIALOG_RTC_SECOND_D - kDIALOG_RTC_SUB_SECOND_A]) << 39);
#else
    uint32_t upcount, upcount2;

    // read twice, to guard against race with the incrementer
    do {
	    if (pmu_read_nvram(kDIALOG_UPCOUNT_A, &upcount, sizeof(upcount)) != 0) return 0;
	    if (pmu_read_nvram(kDIALOG_UPCOUNT_A, &upcount2, sizeof(upcount2)) != 0) return 0;
    } while (upcount != upcount2);
    
    ticks = upcount * tick_frequency;
#endif
    ticks += offset;

    utime_t secs = ticks / tick_frequency;
    utime_t usecs = ((ticks % tick_frequency) * 1000000) / tick_frequency;

    return (secs * 1000000) + usecs;
}

#if !WITH_HW_CHARGER
void charger_print_status(void)
{

}
#endif
