/*
 * Copyright (c) 2008 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */

#include "sdiodrv_config.h"

#include <sys/task.h>

#include <debug.h>
#include <AssertMacros.h>


/** Wait this long after a Command / Data reset is triggered before continuing.
 * Allows time for the reset to propogate to the SDIO side of the hardware block.
 * Value chosen to ensure that at least 4 SD Clock periods pass at the lowest
 * expected SD Clock rate. 4 clock periods @ 10 KHz = 400 us
 * See: <rdar://problem/6676702> SDIO: Soft Reset not propagated until SD clock running
 */
#define RESET_WAIT_US (400)


/** The maximum SD Clock rate clients are allowed to set.
 * The maximum SD Clock rate allowed by the standard is 50 MHz. However,
 * The Arasan block is rated to support up to a 52 MHz clock. Allow clients
 * to set rates up to the higher max:
 * <rdar://problem/7091999> Arasan SDIO Driver should allow up to 52 MHz SD Clock
 */
static const IOSDIOClockRate kMaxAllowableSDClockRate = 52000000;


static bool
sdiodrv_waitForInternalClk(const SDHCRegisters_t *sdhc)
{
	for (int i = 0; i < 10 && !sdhc_isInternalClockStable(sdhc); i++) {
		task_sleep(5);
	}

	return sdhc_isInternalClockStable(sdhc);
}


static SDIOReturn_t
sdiodrv_enableClocks(SDHCRegisters_t *sdhc)
{
	sdhc_enableInternalClock(sdhc, true);

	if(!sdiodrv_waitForInternalClk(sdhc)) {
		return kSDIOReturnInternalClockUnstable;
	}

	sdhc_enableSDClock(sdhc, true);

	return kSDIOReturnSuccess;
}


static void
sdiodrv_disableClocks(SDHCRegisters_t *sdhc)
{
	sdhc_enableSDClock(sdhc, false);
	//sdhc_enableInternalClock(sdhc, false);
}

IOSDIOClockRate
sdiodrv_setClockRate(SDHCRegisters_t *sdhc, IOSDIOClockRate *targetSDClkRateHz, UInt32 inputClkRateHz)
{
	check(sdhc);
	check(targetSDClkRateHz);
	check(*targetSDClkRateHz > 0 && *targetSDClkRateHz <= kMaxAllowableSDClockRate);
	check(inputClkRateHz);
	
	if(*targetSDClkRateHz <= 0 || *targetSDClkRateHz > kMaxAllowableSDClockRate || !inputClkRateHz) {
		return kSDIOReturnBadArgument;
	}
	
	bool sdClkEnabled = sdhc_isSDClockEnabled(sdhc);
	
	sdhc_enableSDClock(sdhc, false);
	
	IOSDIOClockRate newRate = sdhc_setClockDividerRate(sdhc, *targetSDClkRateHz, inputClkRateHz);

	if(sdClkEnabled && !sdiodrv_waitForInternalClk(sdhc)) {
		*targetSDClkRateHz = 0;
		return kSDIOReturnInternalClockUnstable;
	}
	
	sdhc_setDataTimeoutCounter(sdhc);
	
	sdhc_enableSDClock(sdhc, sdClkEnabled);
	
	*targetSDClkRateHz = newRate;
	return kSDIOReturnSuccess;
}

SDIOReturn_t
sdiodrv_resetSDHC(SDHCRegisters_t *sdhc, SDHCResetFlags_t resetOptions)
{
	check(sdhc);
	check(resetOptions & (kSDHCResetDataLine | kSDHCResetCmdLine | kSDHCResetAll));
	
	if(!(resetOptions & (kSDHCResetDataLine | kSDHCResetCmdLine | kSDHCResetAll))) {
		return kSDIOReturnBadArgument;
	}
	
	dprintf(DEBUG_INFO, "%s: Resetting SDHC @ %p, 0x%X\n", __func__, sdhc, resetOptions);
	sdhc_reset(sdhc, resetOptions);
	
	for (int i = 0; i < 100 && sdhc_isResetting(sdhc); i++) {
		task_sleep(5);
	}

	if(sdhc_isResetting(sdhc)) {
		dprintf(DEBUG_CRITICAL, "%s: Failed to complete reset, 0x%X\n", __func__, sdhc->softwareReset);
		return kSDIOReturnInReset;
	}
	
	// <rdar://problem/6676702> SDIO: Soft Reset not propagated until SD clock running
	if(resetOptions & kSDHCResetAll) {
		// If we're doing a full reset, enable the internal clock to ensure
		// that the reset is propogated into the SDIO domain
		sdhc_enableInternalClock(sdhc, true);
		if(!sdiodrv_waitForInternalClk(sdhc)) {
			return kSDIOReturnInternalClockUnstable;
		}
		
	} else {
		// If we're not doing a full reset, wait for >= 4 SD Clock periods
		task_sleep(RESET_WAIT_US);
	}

	return kSDIOReturnSuccess;
}

SDIOReturn_t
sdiodrv_setClockMode(SDHCRegisters_t *sdhc, enum IOSDIOClockMode clockMode)
{
	check(sdhc);
	check(kIOSDIO1BitMode == busWidth || kIOSDIO4BitMode == busWidth);
	
	SDIOReturn_t retval = kSDIOReturnSuccess;
	
	switch(clockMode) 
	{
		case kIOSDIOClockOff:
			sdiodrv_disableClocks(sdhc);
			break;
		case kIOSDIOClockOn:
			retval = sdiodrv_enableClocks(sdhc);
			break;
		default:
			retval = kSDIOReturnBadArgument;
			break;
	};
	
	return retval;
}

SDIOReturn_t
sdiodrv_setBusWidth(SDHCRegisters_t *sdhc, enum IOSDIOBusWidth busWidth)
{
	check(sdhc);
	check(kIOSDIO1BitMode == busWidth || kIOSDIO4BitMode == busWidth);
	
	SDIOReturn_t retval = kSDIOReturnSuccess;
	
	switch(busWidth) 
	{
		case kIOSDIO1BitMode:
		case kIOSDIO4BitMode:
			sdhc_setDataTransferWidth(sdhc, busWidth);
			break;
		default:
			retval = kSDIOReturnBadArgument;
			break;
	};
	
	return retval;
}


SDIOReturn_t
sdiodrv_setBusSpeedMode(SDHCRegisters_t *sdhc, enum IOSDIOBusSpeedMode speedMode)
{
	check(sdhc);
	check(kIOSDIONormalSpeed == speedMode || kIOSDIOHighSpeed == speedMode);
	
	SDIOReturn_t retval = kSDIOReturnSuccess;
	
	switch(speedMode) 
	{
		case kIOSDIONormalSpeed:
			sdhc_setHighSpeedMode(sdhc, false);
			break;
		case kIOSDIOHighSpeed:
			check(sdhc_isHighSpeedSupported(sdhc));
			sdhc_setHighSpeedMode(sdhc, true);
			break;
		default:
			retval = kSDIOReturnBadArgument;
			break;
	};
	
	return retval;	
}




