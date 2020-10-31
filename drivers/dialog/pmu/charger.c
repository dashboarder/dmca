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

#if DIALOG_D2255 && !WITH_HW_CHARGER

bool charger_has_usb(int dock) { return false; }
bool charger_has_external(int dock) { return false; }
bool charger_has_firewire(int dock) { return false; }
bool charger_has_batterypack(int dock) { return false; }
bool charger_charge_done(int dock) { return true; }

uint32_t charger_get_max_charge_current(int dock) { return 1500; } 		// bogus, only to prevent infinite loops
int charger_read_battery_level(uint32_t *milliVolts) { return 5000; };  // bogus, only to prevent infinite loops

void charger_clear_usb_state(void) { }
void charger_clear_alternate_usb_current_limit(void) { }
void charger_set_charging(int dock, uint32_t input_current_limit, uint32_t *charge_current_ma) { }

#elif !WITH_HW_CHARGER || WITH_HW_CHARGER_DIALOG || DIALOG_D2238

#if WITH_HW_CHARGER_DIALOG

#if DIALOG_D2231
#include "d2231.h"
#elif DIALOG_D2355
#include "d2355.h"
#else
#error Unknown Dialog charger selection
#endif

#else // !WITH_HW_CHARGER_DIALOG

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
#define charger_print_status pmu_charger_print_status
#define charger_set_charging pmu_charger_set_charging
#define charger_has_external pmu_charger_has_external
#elif DIALOG_D2257
#include "d2257.h"
#else
#error Unknown Dialog PMU selection
#endif

#endif // !WITH_HW_CHARGER_DIALOG

#include "NTCTables.h"
#include <target/powerconfig.h>

#if !WITH_HW_CHARGER
#define CHARGER_IIC_BUS PMU_IIC_BUS
#endif

#ifdef DIALOG_PMU_USE_CHARGE_TABLE
#error powerconfig.h is out of date
#endif

#if PMU_HAS_ISET_BAT_2BYTES
typedef uint16_t iset_bat_t;
#else
typedef uint8_t iset_bat_t;
#endif

static iset_bat_t	ichg_bat_max;
static uint8_t		iset_buck_limit;
#if DIALOG_D1755
static uint8_t		iset_buck_calibration_500;
static uint8_t		iset_buck_calibration_1000;
#endif
#if TARGET_USE_CHARGE_TABLE
#if DIALOG_D1815 || DIALOG_D1881 || DIALOG_D1946
static uint8_t		ichg_tbat_max[kDIALOG_ICHG_TBAT_NUM] = TARGET_ICHG_TBAT_MAX;
#endif
#endif

// This goes after the powerconfig
#ifndef TARGET_MAX_USB_INPUT_CURRENT
#define TARGET_MAX_USB_INPUT_CURRENT (1000)
#endif

// this is aria With internal charger
#if DIALOG_D2257 && !DIALOG_D2231 && !DIALOG_D2355
#define DIALOG_CHG_CONTROL kD2257_CHG_CONTROL
#else
#define DIALOG_CHG_CONTROL kDIALOG_SYS_CONTROL
#endif

static int dialog_set_data(int dev, uint16_t reg, uint8_t byte, bool do_confirm)
{
	uint8_t confirm;
	UInt8 data[kDIALOG_REG_BYTES + 1];

	if (kDIALOG_REG_BYTES > 1) data[kDIALOG_REG_BYTES - 2] = (reg >> 8) & 0xFF;
	data[kDIALOG_REG_BYTES - 1] = (reg & 0xFF);
	data[kDIALOG_REG_BYTES] = byte;
	
	iic_write(dev, kDIALOG_ADDR_W, data, sizeof(data));
	if (do_confirm) {
		iic_read(dev, kDIALOG_ADDR_R, data, sizeof(data)-1, &confirm, 1, IIC_NORMAL);
		if (byte == confirm) {
			dprintf(DEBUG_SPEW, "pmu%d: wrote %x to reg %x\n", dev, byte, reg);
		} else {
			dprintf(DEBUG_SPEW, "pmu%d: try to write %x to reg %x, but got %x back\n", dev, byte, reg, confirm);
			return -1;
		}
	}
	
	return 0;
}

static int dialog_get_data(int dev, uint16_t reg, uint8_t *byte)
{
	UInt8 addr[kDIALOG_REG_BYTES];
	if (kDIALOG_REG_BYTES > 1) addr[kDIALOG_REG_BYTES - 2] = (reg >> 8) & 0xFF;
	addr[kDIALOG_REG_BYTES - 1] = (reg & 0xFF);
	return iic_read(dev, kDIALOG_ADDR_R, addr, sizeof(addr), byte, 1, IIC_NORMAL);
}

static void dialog_read_events(eventRegisters data)
{
	bzero(data, sizeof(eventRegisters));
	UInt8 addr[kDIALOG_REG_BYTES];
	if (kDIALOG_REG_BYTES > 1) addr[kDIALOG_REG_BYTES - 2] = (kDIALOG_EVENT_A >> 8) & 0xFF;
	addr[kDIALOG_REG_BYTES - 1] = (kDIALOG_EVENT_A & 0xFF);
	iic_read(CHARGER_IIC_BUS, kDIALOG_ADDR_R, addr, sizeof(addr), data, sizeof(eventRegisters), IIC_NORMAL);
}

static void dialog_read_status(statusRegisters data)
{
	bzero(data, sizeof(statusRegisters));
	UInt8 addr[kDIALOG_REG_BYTES];
	if (kDIALOG_REG_BYTES > 1) addr[kDIALOG_REG_BYTES - 2] = (kDIALOG_STATUS_A >> 8) & 0xFF;
	addr[kDIALOG_REG_BYTES - 1] = (kDIALOG_STATUS_A & 0xFF);
	iic_read(CHARGER_IIC_BUS, kDIALOG_ADDR_R, addr, sizeof(addr), data, sizeof(statusRegisters), IIC_NORMAL);
}

#if DIALOG_D2257 && !DIALOG_D2231 && !DIALOG_D2355
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

#if !DIALOG_D2231 && !DIALOG_D2238 && !DIALOG_D2255 && !DIALOG_D2355
	if (kDIALOG_ADC_CONTROL_MUX_SEL_ACC_ID == input_select) {
#if DIALOG_D2045 || DIALOG_D2089 || DIALOG_D2186 || DIALOG_D2207 || DIALOG_D2238
		dialog_set_data(dev, kDIALOG_ADC_CONTROL2, kDIALOG_ADC_CONTROL2_ADC_REF_EN, 0);
#else
		dialog_set_data(dev, kDIALOG_ADC_CONTROL, kDIALOG_ADC_CONTROL_ADC_REF_EN | reg, 0);
		reg |= kDIALOG_ADC_CONTROL_ADC_REF_EN;
#endif
		spin(80 * 1000);
	}
#endif

	dialog_set_data(dev, kDIALOG_ADC_CONTROL, kDIALOG_ADC_CONTROL_MAN_CONV | reg, 0);

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

	    dialog_get_data(dev, kDIALOG_ADC_CONTROL, &reg);
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
			dialog_get_data(dev, kDIALOG_MEMBYTE0+0x09, &data[0]);
			if(data[0]<255) data[0]++;
			dialog_set_data(dev, kDIALOG_MEMBYTE0+0x09, data[0], 0);
		}
		// Check that auto conversions are disabled
		uint8_t adc_regs[5];
		reg = kDIALOG_ADC_CONTROL;
		iic_read(CHARGER_IIC_BUS, kDIALOG_ADDR_R, &reg, 1, &adc_regs[0],
			 sizeof(adc_regs), IIC_NORMAL);
		if(adc_regs[0] & kDIALOG_ADC_CONTROL_AUTO_VDD_OUT_EN) 
			issues |= kAutoConversionVDD;
		if(adc_regs[4] & 0x80) issues |= kAutoConversionTemp;
		// Check that we're charging. Not treating this as critical yet,
		// but if we're not charging this might be a reason the workaround
		// could fail
		dialog_get_data(dev, kDIALOG_STATUS_A, &data[0]);
		if((data[0]&kD1815_STATUS_A_VBUS_DET)==0) chg_dis |= 1;
		// Check that bat_pwr_susp is enabled
		dialog_get_data(dev, DIALOG_CHG_CONTROL, &data[0]);
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
			dialog_set_data(dev, kDIALOG_MEMBYTE0+0x8, issues, 0);
			// Write reset count to scratchpad
			dialog_get_data(dev, kDIALOG_MEMBYTE0+0x0a, &data[0]);
			if(data[0]<255) data[0]++;
			dialog_set_data(dev, kDIALOG_MEMBYTE0+0x0a, data[0], 0);
#endif
			// PMU register reset
			dialog_get_data(dev, 0xE0, &data[0]);
			data[0]|=1;	// test_enable
			dialog_set_data(dev, 0xE0, data[0], 0);
			data[0]|=3;	// test_enable|SW_RESET
			dialog_set_data(dev, 0xE0, data[0], 0);
			// NOTREACHED
			while(1);
		} else {
			dialog_get_data(dev, DIALOG_CHG_CONTROL, &data[0]);
			// Clear BAT_PWR_SUSP
			data[0] &= ~kDIALOG_SYS_CONTROL_BAT_PWR_SUSPEND;
			dialog_set_data(dev, DIALOG_CHG_CONTROL, data[0], 0);
			// Wait
			spin(1300);
			// Set BAT_PWR_SUSP
			data[0] |= kDIALOG_SYS_CONTROL_BAT_PWR_SUSPEND;
			dialog_set_data(dev, DIALOG_CHG_CONTROL, data[0], 0);
			// Try again!
			goto continue_conv;
		}
	}
#endif /* ADC_TIMEOUT_WORKAROUND */
	
	UInt8 addr[kDIALOG_REG_BYTES];
	if (kDIALOG_REG_BYTES > 1) addr[kDIALOG_REG_BYTES - 2] = (kDIALOG_ADC_LSB >> 8) & 0xFF;
	addr[kDIALOG_REG_BYTES - 1] = (kDIALOG_ADC_LSB & 0xFF);
	iic_read(CHARGER_IIC_BUS, kDIALOG_ADDR_R, addr, sizeof(addr), &data[0], sizeof(data), IIC_NORMAL);

	dialog_set_data(dev, kDIALOG_ADC_CONTROL, kDIALOG_ADC_CONTROL_DEFAULTS, 0);
#if DIALOG_D2045 || DIALOG_D2089 || DIALOG_D2186 || DIALOG_D2207 || DIALOG_D2231 || DIALOG_D2238
	dialog_set_data(dev, kDIALOG_ADC_CONTROL2, kDIALOG_ADC_CONTROL2_DEFAULTS, 0);
#endif

#if DIALOG_D1755 || DIALOG_D2231
	*level = ((unsigned)data[1] << 2) | (data[0] & 0x3);
#else
	*level = ((unsigned)data[1] << 4) | (data[0] & 0xf);
#endif

	return result;
}
#endif

static int
dialog_read_battery_level(int dev, unsigned *level)
{
	int		 result;
	unsigned adc;

	result = dialog_read_adc(dev, kDIALOG_ADC_CONTROL_MUX_SEL_VBAT, &adc);

#if DIALOG_D2045 || DIALOG_D2089 || DIALOG_D2186 || DIALOG_D2207 || DIALOG_D2231 || DIALOG_D2238 || DIALOG_D2257 || DIALOG_D2355
	*level = ((adc * 2500) >> kDIALOG_ADC_RESOLUTION_BITS) + 2500;
#else
	*level = ((adc * 2000) >> kDIALOG_ADC_RESOLUTION_BITS) + 2500;
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
	unsigned adc;

	if (level == NULL) return -1;

	result = dialog_read_adc(CHARGER_IIC_BUS, kDIALOG_ADC_CONTROL_MUX_SEL_TBAT, &adc);

	if (result == 0) {
		// The NTC is connected to a 50uA current source
		// R = U / I; 1/I = 1/50uA = 20000 * 1/A
		// U = <adc_val> * <adc_full_scale_range> / <adc_resolution>
		// adc_fsr comes in mV, take the factor of 1000 out of current constant
		const uint32_t Rntc = (adc * (20000/1000) * kDIALOG_ADC_FULL_SCALE_MV) >> kDIALOG_ADC_RESOLUTION_BITS;
		dprintf(DEBUG_SPEW, "ADC chan %d: value=%d, Rntc=%d\n", kDIALOG_ADC_CONTROL_MUX_SEL_TBAT, adc, Rntc);
		*level=lut_interpolate(NTCG103JF103F, Rntc);
	}

	return result;
}

#if DIALOG_D1815
bool dialog_workaround_7886796(const statusRegisters status)
{
	// Ashley has a bug where if the voltage drops below VBUS_THR (~4.3V), the charger buck digital control
	// resets to 100mA, but the analog side keeps drawing more current.  The only way around it
	// is to disable the charger.
	if (STATUS_FLAG_TEST(status, kDIALOG_STATUS_CHG_ATT_MASK) && !EVENT_FLAG_TEST(status, kDIALOG_STATUS_USB_MASK)) {
		dialog_set_data(CHARGER_IIC_BUS, kDIALOG_CHARGE_BUCK_CONTROL, kDIALOG_CHARGE_BUCK_CONTROL_ISET_BUCK_100, true);
		dialog_set_data(CHARGER_IIC_BUS, kDIALOG_CHARGE_BUCK_CONTROL, kDIALOG_CHARGE_BUCK_CONTROL_ISET_BUCK_100 | kDIALOG_CHARGE_BUCK_CONTROL_ISET_VBUS_CHG_BUCK_EN, true);
		task_sleep(25 * 1000);  // wait out 16ms VBUS_PROT_DET debounce plus some margin
		return true;
	}
	
	return false;
}
#endif

bool charger_has_usb(int dock)
{
	statusRegisters status;

	dialog_read_status(status);

#if DIALOG_D1815
	if (dialog_workaround_7886796(status)) {
		dialog_read_status(status);
	}
#endif

#ifdef TARGET_POWER_USB_MASK
	return (STATUS_FLAG_TEST(status, TARGET_POWER_USB_MASK));
#else
	return (STATUS_FLAG_TEST(status, kDIALOG_STATUS_USB_MASK));
#endif
}

bool charger_has_firewire(int dock)
{
	statusRegisters status;
	dialog_read_status(status);
	return STATUS_REGISTER_TEST_MASK(status, kDialogStatusFWMask);
}

bool charger_has_external(int dock)
{
	// external charge not supported
	return false;
}

static void
read_defaults_if_needed(void)
{
	static bool defaults_read = 0;
	if (defaults_read)
		return;
	defaults_read = 1;

#if PMU_HAS_ISET_BAT_2BYTES
	uint8_t otp[2];

	/* Read and remember max bat charge current */
	dialog_get_data(CHARGER_IIC_BUS, kDIALOG_OTP_ISET_BAT_MSB, &otp[0]);
	dialog_get_data(CHARGER_IIC_BUS, kDIALOG_OTP_ISET_BAT_MSB+1, &otp[1]);
	ichg_bat_max = (otp[0] << 8) | otp[1];
#else
	uint8_t otp;

	/* Read and remember max bat charge current */
	dialog_get_data(CHARGER_IIC_BUS, kDIALOG_OTP_ISET_BAT, &otp);
#if DIALOG_D2238
	ichg_bat_max = kDIALOG_CHARGE_CONTROL_A_ISET_BAT_MASK & (otp << 1);
#else
	ichg_bat_max = kDIALOG_CHARGE_CONTROL_A_ISET_BAT_MASK &
			((otp >> kDIALOG_OTP_ISET_BAT_SHIFT) << kDIALOG_CHARGE_CONTROL_A_ISET_BAT_SHIFT);
#endif // DIALOG_D2238
    
#endif

#if DIALOG_D1755
	// <rdar://problem/6019689>
	if (5 == ichg_bat_max)
		ichg_bat_max = 0x27;
	// <rdar://problem/6861148>
	if (1 == ichg_bat_max)
		ichg_bat_max = 0x18;

	// read out calibrated USB current limits from OTP.  If unset or out of range, use defaults.
	// use 450mA rather than 500mA setting by default, because that's what was tested on N88.
	dialog_get_data(CHARGER_IIC_BUS, kD1755_OTP_USB_500_LIMIT, &iset_buck_calibration_500);
	if ((iset_buck_calibration_500 < kDIALOG_CHARGE_BUCK_CONTROL_ISET_BUCK_350) || (iset_buck_calibration_500 > kDIALOG_CHARGE_BUCK_CONTROL_ISET_BUCK_600))
	    iset_buck_calibration_500 = kDIALOG_CHARGE_BUCK_CONTROL_ISET_BUCK_450;
	dialog_get_data(CHARGER_IIC_BUS, kD1755_OTP_USB_900_LIMIT, &iset_buck_calibration_1000);
	if ((iset_buck_calibration_1000 < kDIALOG_CHARGE_BUCK_CONTROL_ISET_BUCK_600) || (iset_buck_calibration_1000 > kDIALOG_CHARGE_BUCK_CONTROL_ISET_BUCK_1300))
	    iset_buck_calibration_1000 = kDIALOG_CHARGE_BUCK_CONTROL_ISET_BUCK_900;
#endif
}

static iset_bat_t
dialog_get_charge_current_setting(unsigned int charge_target_ma)
{
    if (charge_target_ma > kDIALOG_CHARGE_CONTROL_MAX) charge_target_ma = kDIALOG_CHARGE_CONTROL_MAX;
    return (charge_target_ma / kDIALOG_CHARGE_CONTROL_STEP);
}

static uint32_t
dialog_get_charge_current_limit(iset_bat_t setting)
{
    setting = (setting & kDIALOG_CHARGE_CONTROL_A_ISET_BAT_MASK) >> kDIALOG_CHARGE_CONTROL_A_ISET_BAT_SHIFT;
    return setting * kDIALOG_CHARGE_CONTROL_STEP;
}

uint32_t
charger_get_max_charge_current(int dock)
{
    read_defaults_if_needed();
    return dialog_get_charge_current_limit(ichg_bat_max);
}

int charger_read_battery_temperature(int *centiCelsiusTemperature)
{
	return dialog_read_battery_temperature(centiCelsiusTemperature);
}

int charger_read_battery_level(uint32_t *milliVolts)
{
	return dialog_read_battery_level(CHARGER_IIC_BUS, milliVolts);
}

#if !DIALOG_D2238

static int
dialog_get_charger_limit(UInt8 setting)
{
#if DIALOG_D1755
    if (setting <= kDIALOG_CHARGE_BUCK_CONTROL_ISET_BUCK_120)
	return 70 + setting*10;
    else if (setting <= kDIALOG_CHARGE_BUCK_CONTROL_ISET_BUCK_600)
	return 350 + (setting-6)*50;
    else if (setting <= kDIALOG_CHARGE_BUCK_CONTROL_ISET_BUCK_1300)
	return 700 + (setting-12)*200;
#elif DIALOG_D1815
    if (setting <= kDIALOG_CHARGE_BUCK_CONTROL_ISET_BUCK_200)
	return 50 + setting*10;
    else if (setting <= kDIALOG_CHARGE_BUCK_CONTROL_ISET_BUCK_600)
	return 250 + (setting-16)*50;
    else if (setting <= kDIALOG_CHARGE_BUCK_CONTROL_ISET_BUCK_900)
	return 700 + (setting-24)*200;
    else if (setting <= kDIALOG_CHARGE_BUCK_CONTROL_ISET_BUCK_2000)
	return 1000 + (setting-26)*200;
#else
    if (setting <= kDIALOG_CHARGE_BUCK_CONTROL_ISET_BUCK_MIN)
	return kDIALOG_CHARGE_BUCK_CONTROL_MIN;
    else
	return kDIALOG_CHARGE_BUCK_CONTROL_MIN + (100*(setting - kDIALOG_CHARGE_BUCK_CONTROL_ISET_BUCK_MIN) + (kDIALOG_CHARGE_BUCK_CONTROL_STEP_PER_100MA-1))/kDIALOG_CHARGE_BUCK_CONTROL_STEP_PER_100MA;	    // round up
#endif

    return -1;
}

static UInt8
dialog_input_current_limit_step_down(UInt8 iset_buck)
{
#if DIALOG_D1755
    // steps are big; go down by one
    if (iset_buck > kDIALOG_CHARGE_BUCK_CONTROL_ISET_BUCK_100) {
	return (iset_buck - 1);
    } else {
	return kDIALOG_CHARGE_BUCK_CONTROL_ISET_BUCK_100;
    }
#elif DIALOG_D1815
    if (iset_buck <= kDIALOG_CHARGE_BUCK_CONTROL_ISET_BUCK_200) {
	return kDIALOG_CHARGE_BUCK_CONTROL_ISET_BUCK_100;
    } else if (iset_buck <= kDIALOG_CHARGE_BUCK_CONTROL_ISET_BUCK_600) {
	// 50mA steps from 200-600mA
	return iset_buck - 2;
    } else {
	// steps above 600mA are all at least 100mA
	return iset_buck - 1;
    }
#else
    if (iset_buck <= kDIALOG_CHARGE_BUCK_CONTROL_ISET_BUCK_200) {
	return kDIALOG_CHARGE_BUCK_CONTROL_ISET_BUCK_100;
    } else {
	return iset_buck - kDIALOG_CHARGE_BUCK_CONTROL_STEP_PER_100MA;
    }
#endif
}

static void
dialog_log_charger_limit(UInt8 iset_buck)
{
	dprintf(DEBUG_CRITICAL, "limiting USB input current to %d mA\n", dialog_get_charger_limit(iset_buck));
}

#endif // !DIALOG_D2238

void
charger_set_charging(int dock, uint32_t input_current_limit, uint32_t *charge_current_ma)
{
	dprintf(DEBUG_SPEW, "dialog_set_charging(input_current_limit=%d, charge_current_ma=%d)\n",
		input_current_limit, *charge_current_ma);

	iset_bat_t charge_control_bat;
	bool pause = (*charge_current_ma == 0);

	read_defaults_if_needed();

#if !DIALOG_D2238
	UInt8 syscontrol;
	UInt8 charge_buck_control, iset_buck;

	dialog_get_data(CHARGER_IIC_BUS, DIALOG_CHG_CONTROL, &syscontrol);
	dialog_get_data(CHARGER_IIC_BUS, kDIALOG_CHARGE_BUCK_CONTROL, &charge_buck_control);

	syscontrol &= ~kDIALOG_SYS_CONTROL_CHRG_CONTROLS;
#endif

	dprintf(DEBUG_SPEW, "dialog_set_charging(paus=%d, ichg_bat_max=0x%02x)\n", pause, ichg_bat_max);

	if (!pause) {
		charge_control_bat = dialog_get_charge_current_setting(*charge_current_ma);
		*charge_current_ma -= dialog_get_charge_current_limit(charge_control_bat);
	} else {
#if DIALOG_D1815 || DIALOG_D1881 || DIALOG_D1946
		charge_control_bat = kDIALOG_CHARGE_CONTROL_A_CHG_SUSP;
#else
		charge_control_bat = 0;
#endif
	}

#if TARGET_USE_CHARGE_TABLE
#if DIALOG_D1815 || DIALOG_D1881 || DIALOG_D1946
	// Ashley takes care of setting the charge limit based on temperature, but the temperature-relative limits
	// themselves can change based on voltage (ATV) level.  So program the ICHG_BAT registers.
	if (!pause) {
		for (unsigned int i = 0; i < kDIALOG_ICHG_TBAT_NUM; i++) {
		    UInt8 tbat_reg = kDIALOG_ICHG_TBAT_0 + 2*i;
		    UInt8 tbat_data = ichg_tbat_max[i];
		    if (charge_control_bat < tbat_data) tbat_data = charge_control_bat;
		    dialog_set_data(CHARGER_IIC_BUS, tbat_reg, tbat_data, true);
		}
	}
#endif
#endif

#if PMU_HAS_ISET_BAT_2BYTES
	dialog_set_data(CHARGER_IIC_BUS, kDIALOG_CHARGE_CONTROL_ICHG_BAT_MSB, (charge_control_bat >> 8) & 0xFF, true);
	dialog_set_data(CHARGER_IIC_BUS, kDIALOG_CHARGE_CONTROL_ICHG_BAT_MSB+1, charge_control_bat & 0xFF, true);
#else
	dialog_set_data(CHARGER_IIC_BUS, kDIALOG_CHARGE_CONTROL_ICHG_BAT, charge_control_bat, true);
#endif

#if !DIALOG_D2238
	if (input_current_limit < 100) {
		syscontrol |= kDIALOG_SYS_CONTROL_BAT_PWR_SUSPEND;
	}

	dialog_set_data(CHARGER_IIC_BUS, DIALOG_CHG_CONTROL, syscontrol, true);

#if DIALOG_D1946
	dialog_get_data(CHARGER_IIC_BUS, kDIALOG_SYS_CONTROL2, &syscontrol);
	if (input_current_limit < 100) {
	    syscontrol |= kDIALOG_SYS_CONTROL2_BAT_PWR_SUSPEND;
	} else {
	    syscontrol &= ~kDIALOG_SYS_CONTROL2_BAT_PWR_SUSPEND;
	}
	dialog_set_data(CHARGER_IIC_BUS, kDIALOG_SYS_CONTROL2, syscontrol, true);
#endif

	if (input_current_limit > TARGET_MAX_USB_INPUT_CURRENT) {
	    input_current_limit = TARGET_MAX_USB_INPUT_CURRENT;
	}

#if DIALOG_D1881 || DIALOG_D1972
	// use syscfg calibration to limit Angelina (and Agatha) values
#if WITH_SYSCFG
	uint32_t iset_buck_cal[3];
	if (syscfgCopyDataForTag('CBAT', (uint8_t *)iset_buck_cal, sizeof(iset_buck_cal)) == sizeof(iset_buck_cal)) {
	    uint32_t cal100 = iset_buck_cal[0];  // 16.16 fixed-point
	    uint32_t cal500 = iset_buck_cal[1];  // 16.16 fixed-point
	    uint32_t cal1000 = iset_buck_cal[2]; // 16.16 fixed-point

	    uint32_t calibrated_target = 0;

	    if (input_current_limit < 330 && cal100 != 0) {
		// round up (keep current below target)
		uint32_t calMA = (cal100 + 65535) >> 16;

		if (calMA < 100) {
		    calibrated_target = input_current_limit + (100 - calMA);
		} else if (calMA > 100) {
		    calibrated_target = input_current_limit - (calMA - 100);
		}
	    } else if (input_current_limit >= 330 && cal500 != 0 && cal1000 != 0) {
		SInt32 delta = cal1000 - cal500;	// 16.16 fixed-point
		SInt32 base = cal500 - delta;		// 16.16 fixed-point
		SInt32 slope = delta / 500;		// 16.16 fixed-point

		SInt32 target = (input_current_limit << 16);	// 16.16 fixed-point
		// fixed-point divide
		calibrated_target = (((SInt64)(target - base) << 16)/ slope) >> 16;
	    }
	    
	    if (calibrated_target != 0) {
		// limit to 15% corection
		if (calibrated_target > (input_current_limit * 115)/100) {
		    input_current_limit = (input_current_limit * 115)/100;
		} else if (calibrated_target < (input_current_limit * 85)/100) {
		    input_current_limit = (input_current_limit * 85)/100;
		} else {
		    input_current_limit = calibrated_target;
		}
	    }
	}
	else
#endif /* WITH_SYSCFG */
	{
	    // if not available, limit high-charge rate (above 500mA) by 15% to ensure
	    // charger is below its limit.
	    if (input_current_limit > 500) {
		input_current_limit = (85 * input_current_limit) / 100;
	    }
	}
#endif

#if DIALOG_D1881 || DIALOG_D1946 || DIALOG_D1972 || DIALOG_D1974 || DIALOG_D2018 || DIALOG_2045 || DIALOG_D2089 || DIALOG_D2207 || DIALOG_D2231 || DIALOG_D2355
	if (input_current_limit >= kDIALOG_CHARGE_BUCK_CONTROL_MAX) {
		iset_buck = kDIALOG_CHARGE_BUCK_CONTROL_ISET_BUCK_MAX;
	} else if (input_current_limit >= 100) {
		iset_buck = kDIALOG_CHARGE_BUCK_CONTROL_ISET_BUCK_MIN + (kDIALOG_CHARGE_BUCK_CONTROL_STEP_PER_100MA * (input_current_limit - kDIALOG_CHARGE_BUCK_CONTROL_MIN)) / 100;
	} else {
		iset_buck = kDIALOG_CHARGE_BUCK_CONTROL_ISET_BUCK_100;
	}

#if DIALOG_D1946
	// Alison trim values are too high at high current settings (8922649)
	// 1A up to but not including 1.5A should be 25mA lower 1.5A on up should be 50mA lower.
	if (iset_buck > kDIALOG_CHARGE_BUCK_CONTROL_ISET_BUCK_1500) {
	    iset_buck -= (kDIALOG_CHARGE_BUCK_CONTROL_STEP_PER_100MA / 2);
	} else if (iset_buck > kDIALOG_CHARGE_BUCK_CONTROL_ISET_BUCK_1450) {
	    iset_buck = kDIALOG_CHARGE_BUCK_CONTROL_ISET_BUCK_1450;
	} else if (iset_buck > kDIALOG_CHARGE_BUCK_CONTROL_ISET_BUCK_1000) {
	    iset_buck -= (kDIALOG_CHARGE_BUCK_CONTROL_STEP_PER_100MA / 4);
	} else if (iset_buck > kDIALOG_CHARGE_BUCK_CONTROL_ISET_BUCK_975) {
	    iset_buck = kDIALOG_CHARGE_BUCK_CONTROL_ISET_BUCK_975;
	}
#elif DIALOG_D1974
	// <rdar://problem/11910668> J1/J2/J2A : USB 500mA, 1A, 2.1A, 2.4A Adjust input current limit
	if (iset_buck >= kDIALOG_CHARGE_BUCK_CONTROL_ISET_BUCK_2200) {
		// For input current limits of 2.2A on up, should be 62.5mA lower.
		iset_buck -= 5; // 5*12.5mA

	} else if (iset_buck >= kDIALOG_CHARGE_BUCK_CONTROL_ISET_BUCK_1500) {
		// For input current limits of 1.5A<=x<2.2A. should be 50mA lower.
		iset_buck -= 4;	// 4*12.5mA
		
	} else if (iset_buck >= kDIALOG_CHARGE_BUCK_CONTROL_ISET_BUCK_750) {
		// For input current limits of 0.75A<=x<1.5A, should be 37.5mA lower.
		iset_buck -= 3;	// 3*12.5mA

	} else if (	(iset_buck >= kDIALOG_CHARGE_BUCK_CONTROL_ISET_BUCK_250) &&
				(iset_buck < kDIALOG_CHARGE_BUCK_CONTROL_ISET_BUCK_750) ) {
		// For input current limits of 0.25A<=x<0.75A, should be set 12.5mA lower.
		iset_buck -= 1;	// 12.5mA
	}

	// limit Amelia A0 to 2300mA (9066812)
	if (iset_buck > kDIALOG_CHARGE_BUCK_CONTROL_ISET_BUCK_2300) {
	    UInt8 chip_id;
	    dialog_get_data(CHARGER_IIC_BUS, kDIALOG_CHIP_ID, &chip_id);
	    if ((chip_id & kD1974_CHIP_ID_MRC_MASK) == kD1974_CHIP_ID_MRC_A0) {
		iset_buck = kDIALOG_CHARGE_BUCK_CONTROL_ISET_BUCK_2300;
	    }
	}
#elif DIALOG_D2018
	{
		// <rdar://problem/11621561> P101/P102/P103: USB 1A, 2.1A, and 2.4A Input Current Limit Measures Too High

		UInt8 chip_id;
		dialog_get_data(CHARGER_IIC_BUS, kDIALOG_CHIP_ID, &chip_id);
	
		if ((((chip_id & kD2018_CHIP_ID_MRC_MASK) == 0x09) && (chip_id & kD2018_CHIP_ID_TRC_MASK) <=0x30) ||
			((chip_id & kD2018_CHIP_ID_MRC_MASK) == 0x08)) {
	
			if (iset_buck >= kDIALOG_CHARGE_BUCK_CONTROL_ISET_BUCK_2200) {
				// For input current limits of 2.2A on up, should be 100mA lower.
				iset_buck -= kDIALOG_CHARGE_BUCK_CONTROL_STEP_PER_100MA;
			} else if (iset_buck >= kDIALOG_CHARGE_BUCK_CONTROL_ISET_BUCK_1500) {
				// For input current limits of 1.5A<=x<2.2A. should be 75mA lower.
				iset_buck -= (3*kDIALOG_CHARGE_BUCK_CONTROL_STEP_PER_100MA)/4;
			} else if (iset_buck >= kDIALOG_CHARGE_BUCK_CONTROL_ISET_BUCK_1000) {
				// For input current limits of 1.0A<=x<1.5A, should be set 25mA lower
				iset_buck -= kDIALOG_CHARGE_BUCK_CONTROL_STEP_PER_100MA/4;
			}
		}
		
		// <rdar://problem/11744688> P101/P102/P103:USB 500mA, 2.1A, 2.4A Adjust input current limit for DVT build
		// <rdar://problem/11749442> P101/P102/P103: Adjust USB input current limits for DVT build and beyond
		if (((chip_id & kD2018_CHIP_ID_MRC_MASK) >= 0x09) && (chip_id & kD2018_CHIP_ID_TRC_MASK) >=0x40) {
			if (iset_buck >= kDIALOG_CHARGE_BUCK_CONTROL_ISET_BUCK_1500) {
				// For input current limits of 1.5A on up, should be 37.5mA higher.
				iset_buck += 3;	// 3*12.5mA
				
			} else if (iset_buck >= kDIALOG_CHARGE_BUCK_CONTROL_ISET_BUCK_750) {
				// For input current limits of 0.75A<=x<1.5A. should be 25mA higher.
				iset_buck += 2;	// 2*12.5mA

			} else if (	(iset_buck >= kDIALOG_CHARGE_BUCK_CONTROL_ISET_BUCK_350) &&
						(iset_buck < kDIALOG_CHARGE_BUCK_CONTROL_ISET_BUCK_750) ) {
				// For input current limits of 0.35A<=x<0.75A, should be 12.5mA higher.
				iset_buck += 1;

			} else if (iset_buck < kDIALOG_CHARGE_BUCK_CONTROL_ISET_BUCK_350) {
				// For input current limits of x<0.35A, should be set 12.5mA lower.
				iset_buck -= 1;
			}
		}
	}
#elif DIALOG_D2089
	if (iset_buck >= kDIALOG_CHARGE_BUCK_CONTROL_ISET_BUCK_2100) {
		// For input current of 2.1A and higher, should be 75mA lower.
		iset_buck -= (3*kDIALOG_CHARGE_BUCK_CONTROL_STEP_PER_100MA)/4;
	} else if (iset_buck >= kDIALOG_CHARGE_BUCK_CONTROL_ISET_BUCK_1000) {
		// For input current nominally @ 1.0A should be set 50mA lower
		iset_buck -= kDIALOG_CHARGE_BUCK_CONTROL_STEP_PER_100MA/2;
	} else if (iset_buck >= kDIALOG_CHARGE_BUCK_CONTROL_ISET_BUCK_500) {
		// For input current nominally @ 500mA should be set 25mA lower
		iset_buck -= kDIALOG_CHARGE_BUCK_CONTROL_STEP_PER_100MA/4;
	}
#elif DIALOG_D2355
	// <rdar://problem/24336192> J127 : Implement input current limit calibration in iBoot
	if (iset_buck >= kDIALOG_CHARGE_BUCK_CONTROL_ISET_BUCK_2400) {
		iset_buck -= 5;
	} else if (iset_buck >= kDIALOG_CHARGE_BUCK_CONTROL_ISET_BUCK_1900) {
		iset_buck -= 4;
	} else if (iset_buck >= kDIALOG_CHARGE_BUCK_CONTROL_ISET_BUCK_1000) {
		iset_buck -= 3;
	} else if ( iset_buck >= kDIALOG_CHARGE_BUCK_CONTROL_ISET_BUCK_100 ) {
		iset_buck -= 1;
	}
#endif

#else
	if (input_current_limit >= 1000) {
#if DIALOG_D1755
		iset_buck = iset_buck_calibration_1000;
#else
		iset_buck = kDIALOG_CHARGE_BUCK_CONTROL_ISET_BUCK_1000;
#endif
	} 
	else if (input_current_limit >= 500) {
#if DIALOG_D1755
		iset_buck = iset_buck_calibration_500;
#elif DIALOG_D1815
		// should use 450mA instead of 500mA when CNBRICK software behavior bit is set,
		// but we can't access it here (syscfg may not yet be configured).
		iset_buck = kDIALOG_CHARGE_BUCK_CONTROL_ISET_BUCK_450;
#else
		iset_buck = kDIALOG_CHARGE_BUCK_CONTROL_ISET_BUCK_500;
#endif
	}
	else {
		iset_buck = kDIALOG_CHARGE_BUCK_CONTROL_ISET_BUCK_100;
	}
#endif /* DIALOG_D1881 || DIALOG_D1946 || DIALOG_D1972 || DIALOG_D1974 || DIALOG_D2018*/

	if (iset_buck_limit > 0) iset_buck = iset_buck_limit;
	
#ifdef FORCE_ISET_BUCK
	dprintf(DEBUG_INFO, "forcing 'iset_buck' from 0x%02X to 0x%02x\n", iset_buck, FORCE_ISET_BUCK);
	iset_buck = FORCE_ISET_BUCK;
#endif

	// this is used as a reference for ISET_BUCK, so remove everything else
	charge_buck_control &= kDIALOG_CHARGE_BUCK_CONTROL_ISET_BUCK_MASK;

	if (iset_buck < charge_buck_control) {
		// Ashley charger needs to be disabled to reduce usb current limit (6692148)
		charge_buck_control = iset_buck;
		dialog_set_data(CHARGER_IIC_BUS, kDIALOG_CHARGE_BUCK_CONTROL, charge_buck_control, true);
#if DIALOG_D1755 || DIALOG_D1815
		charge_buck_control |= kDIALOG_CHARGE_BUCK_CONTROL_ISET_VBUS_CHG_BUCK_EN;
		dialog_set_data(CHARGER_IIC_BUS, kDIALOG_CHARGE_BUCK_CONTROL, charge_buck_control, true);
#endif
	}

#if DIALOG_D1815
	if (input_current_limit >= 100) {
		statusRegisters status;
		dialog_read_status(status);
		if (dialog_workaround_7886796(status)) {
		    charge_buck_control = kDIALOG_CHARGE_BUCK_CONTROL_ISET_BUCK_100;
		}
	}
#endif

#if DIALOG_D1755
	if (iset_buck > charge_buck_control) {
		// clear any pending interrupts from previous attach/detach
		eventRegisters events;
		charger_read_events(events);
	}
#endif

	/* Limit USB current slew rate and back off chargers that can't supply the amount they claim (6662542) */
	while (iset_buck > charge_buck_control) {
#if DIALOG_D1755
		/* Raise the current limit one step at a time, and back off if VBUS drops but the charger remains present
		 * at 100mA (which the PMU will default to when VBUS snaps off the bus briefly).
		 */

		UInt8 data = (++charge_buck_control) | kDIALOG_CHARGE_BUCK_CONTROL_ISET_VBUS_CHG_BUCK_EN;
		dialog_set_data(CHARGER_IIC_BUS, kDIALOG_CHARGE_BUCK_CONTROL, data, true);

		// check to see if the charger looks removed, which may mean we have overloaded it and VBUS has dropped.
		UInt8 event;
		bool vbus_rem;
		dialog_get_data(CHARGER_IIC_BUS, kDIALOG_EVENT_D, &event);
		vbus_rem = event & kD1755_EVENT_D_VBUS_REM;
		
		if (vbus_rem) {
			statusRegisters status;
			
			// if CHG_ATT is set, this means that charger was not really extracted; otherwise, it might be real extraction and we should trigger a recheck
			dialog_read_status(status);
			if (STATUS_FLAG_TEST(status, kDIALOG_STATUS_CHG_ATT_MASK)) {
				// back off to the last value that worked and try again.
				iset_buck = dialog_input_current_limit_step_down(charge_buck_control);

				dialog_log_charger_limit(iset_buck);

				// reduce the current limit to 100mA and ramp up again (to keep the slew rate intact).
				charge_buck_control = kDIALOG_CHARGE_BUCK_CONTROL_ISET_BUCK_100;

				// make sure charger is disabled first, so we can reduce the current limit
				data = charge_buck_control;
				dialog_set_data(CHARGER_IIC_BUS, kDIALOG_CHARGE_BUCK_CONTROL, data, true);
				data = charge_buck_control | kDIALOG_CHARGE_BUCK_CONTROL_ISET_VBUS_CHG_BUCK_EN;
				dialog_set_data(CHARGER_IIC_BUS, kDIALOG_CHARGE_BUCK_CONTROL, data, true);
				
				continue;
			}

			break;
		}
#else

#if DIALOG_D1815
		/* Raise the current limit 100mA.  Keep a dwell time of 10ms between steps,
		 * except for 700-900mA, which requires 20ms.
		 */
		if (charge_buck_control == kDIALOG_CHARGE_BUCK_CONTROL_ISET_BUCK_700) {
			task_sleep(20 * 1000);
		} else	{
			task_sleep(10 * 1000);
		}
#else
		// 10ms per step (100mA)
		task_sleep(10 * 1000);
#endif
		
#if DIALOG_D1815		
		/* For Ashley only, need to keep VBUS above VBUS_THR (4.3V).  So if VBUS falls below 4.4V,
		 * stop increasing current limit.  This is not needed on Angelina/Alison, which are fine
		 * well below 4.3V (down to VCENTER_DET).
		 */
		unsigned int vbus;
		if (dialog_read_adc(CHARGER_IIC_BUS, kDIALOG_ADC_CONTROL_MUX_SEL_VBUS, &vbus) == 0) {
			// VBUS has range of 0-5.5V (2.5V ADC scaled by 0.4545)
			vbus = ((vbus * 5500) >> kDIALOG_ADC_RESOLUTION_BITS);

			if (vbus < 4400) {
				iset_buck = charge_buck_control;
				dialog_log_charger_limit(iset_buck);
				continue;
			}
		}

		if (charge_buck_control < kDIALOG_CHARGE_BUCK_CONTROL_ISET_BUCK_200) {
			charge_buck_control = kDIALOG_CHARGE_BUCK_CONTROL_ISET_BUCK_200;	// from 100mA to 200mA
		} else if (charge_buck_control < kDIALOG_CHARGE_BUCK_CONTROL_ISET_BUCK_300) {
			charge_buck_control = kDIALOG_CHARGE_BUCK_CONTROL_ISET_BUCK_300;	// from 200mA to 300mA
		} else if (charge_buck_control < kDIALOG_CHARGE_BUCK_CONTROL_ISET_BUCK_400) {
			charge_buck_control = kDIALOG_CHARGE_BUCK_CONTROL_ISET_BUCK_400;	// from 300mA to 400mA
		} else if (charge_buck_control < kDIALOG_CHARGE_BUCK_CONTROL_ISET_BUCK_500) {
			charge_buck_control = kDIALOG_CHARGE_BUCK_CONTROL_ISET_BUCK_500;	// from 400mA to 500mA
		} else if (charge_buck_control < kDIALOG_CHARGE_BUCK_CONTROL_ISET_BUCK_600) {
			charge_buck_control = kDIALOG_CHARGE_BUCK_CONTROL_ISET_BUCK_600;	// from 500mA to 600mA
		} else if (charge_buck_control < kDIALOG_CHARGE_BUCK_CONTROL_ISET_BUCK_700) {
			charge_buck_control = kDIALOG_CHARGE_BUCK_CONTROL_ISET_BUCK_700;	// from 600mA to 700mA
		} else if (charge_buck_control < kDIALOG_CHARGE_BUCK_CONTROL_ISET_BUCK_900) {
			charge_buck_control = kDIALOG_CHARGE_BUCK_CONTROL_ISET_BUCK_900;	// from 700mA to 900mA
		} else {
			charge_buck_control = kDIALOG_CHARGE_BUCK_CONTROL_ISET_BUCK_1000;	// from 900mA to 1000mA
		}
#else
		if (charge_buck_control + kDIALOG_CHARGE_BUCK_CONTROL_STEP_PER_100MA < iset_buck) {
		    charge_buck_control += kDIALOG_CHARGE_BUCK_CONTROL_STEP_PER_100MA;
		} else {
		    charge_buck_control = iset_buck;
		}
#endif

#if DIALOG_D1755 || DIALOG_D1815
		dialog_set_data(CHARGER_IIC_BUS, kDIALOG_CHARGE_BUCK_CONTROL,
			     charge_buck_control | kDIALOG_CHARGE_BUCK_CONTROL_ISET_VBUS_CHG_BUCK_EN, true);
#else
		dialog_set_data(CHARGER_IIC_BUS, kDIALOG_CHARGE_BUCK_CONTROL, charge_buck_control, true);
#endif

#endif	/* !DIALOG_D1755 */
	}

	iset_buck_limit = iset_buck;
#endif // !DIALOG_D2238
}

bool charger_charge_done(int dock)
{
	statusRegisters status;
	
	dialog_read_status(status);

	return !STATUS_FLAG_TEST(status, kDIALOG_STATUS_CHARGING_MASK);
}

void charger_clear_usb_state(void)
{
	iset_buck_limit = 0;
}

void charger_clear_alternate_usb_current_limit(void)
{

#if DIALOG_D1755 || DIALOG_D2238
	/* no alternate current USB limit */
#elif DIALOG_D1815
	/* Ashley has an alternate USB current limit on boot; switch to the programmed
	 * limit, but only after we've programmed it in the case of precharge.
	 */
	dialog_set_data(CHARGER_IIC_BUS, kDIALOG_ADC_BIST_CONTROL, kD1815_BIST_CONTROL_STD_USB_LIMIT, true);
#else
	uint8_t data;
	/* Angelina's alternate USB current limit is elsewhere */
	dialog_get_data(CHARGER_IIC_BUS, kDIALOG_CHARGE_CONTROL_EN, &data);
	data |= kDIALOG_CHARGE_CONTROL_ALT_USB_DIS;
	dialog_set_data(CHARGER_IIC_BUS, kDIALOG_CHARGE_CONTROL_EN, data, true);

#endif

}

bool
charger_has_batterypack(int dock)
{
	// Use NTC to try to detect a disconnected battery, which will
	// appear very cold.  Since an extremely cold battery pack
	// will also fail this check, do not do anything that would
	// prevent the system from working correctly if it eventually
	// warmed up as a result of this check.
	int		 result;
	unsigned adc;

	result = dialog_read_adc(CHARGER_IIC_BUS, kDIALOG_ADC_CONTROL_MUX_SEL_TBAT, &adc);

	// "Cold" is when the upper 9 bits of the ADC reading are all 1.
	uint32_t max = (((1 << kDIALOG_ADC_RESOLUTION_BITS)-1) -
		       ((1 << (kDIALOG_ADC_RESOLUTION_BITS-9))-1));

	return (result == 0) && (adc < max);
}

bool
dialog_charger_check_usb_change(const eventRegisters ints_pending, const statusRegisters status)
{
	bool powersupply_change_event = EVENT_REGISTER_TEST_MASK(kDialogEventPwrsupplyMask, ints_pending);

#if !DIALOG_D2238
	// if VBUS was removed but charger is still attached, try reducing the current limit
	bool usb_limited = false;
	if (EVENT_REGISTER_TEST_MASK(kDialogEventUSBMask, ints_pending)
	    && STATUS_FLAG_TEST(status, kDIALOG_STATUS_CHG_ATT_MASK)
	    && (iset_buck_limit > kDIALOG_CHARGE_BUCK_CONTROL_ISET_BUCK_100))
	{
		// if input limit has been set back to 100mA when we expected it to be higher,
		// but is still attached, there was an extraction (we cannot use VBUS_EXT_REM
		// because at low charging voltage, VBUS may not be asserted).
		UInt8 iset_buck;
		dialog_get_data(CHARGER_IIC_BUS, kDIALOG_CHARGE_BUCK_CONTROL, &iset_buck);
		if ((iset_buck & kDIALOG_CHARGE_BUCK_CONTROL_ISET_BUCK_MASK) == kDIALOG_CHARGE_BUCK_CONTROL_ISET_BUCK_100) {
		    usb_limited = true;
		}
	}
	if (usb_limited)
	{
		iset_buck_limit = dialog_input_current_limit_step_down(iset_buck_limit);
		dialog_log_charger_limit(iset_buck_limit);

		if (power_enable_charging(true, true)) {
			powersupply_change_event = false;
		}
	}
#endif

	return powersupply_change_event;
}

int charger_set_data(int dev, uint16_t reg, uint8_t byte, bool do_confirm)
{
        return dialog_set_data(dev, reg, byte, do_confirm);
}

int charger_get_data(int dev, uint16_t reg, uint8_t *byte)
{
        return dialog_get_data(dev, reg, byte);
}

#if WITH_HW_CHARGER
bool
charger_check_usb_change(int dock, bool expected)
{
	eventRegisters ints_pending;
	statusRegisters status;

	/* Read & clear interrupts */
	dialog_read_events(ints_pending);
	dialog_read_status(status);

	return dialog_charger_check_usb_change(ints_pending, status);
}

void
charger_early_init(void)
{
    int rc;
	uint8_t data;

	if (!power_is_suspended()) {
		// make sure to clear any events
		rc=dialog_get_data(CHARGER_IIC_BUS, kDIALOG_EVENT_A + kDIALOG_EVENT_COUNT - 1, &data);
	}

#if DIALOG_D2231 || DIALOG_D2355
// <rdar://problem/21879203> J99 EVT not recognizing lightning connection

    rc=charger_get_data(CHARGER_IIC_BUS, kDIALOG_OTP_RELOAD_CONF, &data);
    if ( rc!=0 ) {
		dprintf(DEBUG_CRITICAL, "charger: cannot read reg=%x\n", kDIALOG_OTP_RELOAD_CONF);
    } else if ( (data&kDIALOG_OTP_SKIP_RELOAD_TO_ACT)==0 ) {
        data|=kDIALOG_OTP_SKIP_RELOAD_TO_ACT; // OVERRIDE OTP_SKIP_RELOAD_TO_ACT

        rc=charger_set_data(CHARGER_IIC_BUS, kDIALOG_TEST_ACCESS, kDIALOG_TEST_ACCESS_ENA, false);       // enter testmode
        if ( rc!=0 ) {

        } else {
            rc=charger_set_data(CHARGER_IIC_BUS, kDIALOG_OTP_RELOAD_CONF, data, false);
            if ( rc!=0 ) {

            } else {
                task_sleep(10 * 1000); // 10ms delay, race on charger insertion

                rc=charger_set_data(CHARGER_IIC_BUS, kDIALOG_OTP_RELOAD_CONF, data, false);
                if ( rc!=0 ) dprintf(DEBUG_CRITICAL, "charger: cannot set SKIP_RELOAD_TO_ACT\n");
            }

            rc=charger_set_data(CHARGER_IIC_BUS, kDIALOG_TEST_ACCESS, kDIALOG_TEST_ACCESS_DIS, false);    // exit testmode
            if ( rc!=0 ) panic("charger: cannot exit charger test mode");
        }
    } else {
		dprintf(DEBUG_INFO, "charger: kDIALOG_OTP_RELOAD_CONF ok\n");
    }
#endif
}

void
charger_print_status(void)
{
	// no status
}

#endif // WITH_HW_CHARGER

#endif /* !WITH_HW_CHARGER || WITH_HW_CHARGER_DIALOG */
