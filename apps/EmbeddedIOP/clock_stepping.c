/*
 * Copyright (C) 2009-2012 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */

#include "clock_stepping.h"
#include "clock_management.h"

// clock management only supported on AE2 platforms (H4P, H4G, and H5P)
// On other systems, this will be a no-op
#if !(TARGET_S5L8940XAE2 || TARGET_S5L8945XAE2 || TARGET_S5L8950XAE2)

void SetClockState(clockrequest_reason_t reason, clockvalue_t value)
{
}

#else

#include <debug.h>
#include <AssertMacros.h>

#include <platform/soc/pmgr.h> 
#include <drivers/audio/audio.h>

// helpful macros
#define  AE2_CLK_CFG_PENDING           (1 << 16)
#define  AE2_PCLK_CFG_DIVIDER(_d)      (((_d) & 0xFF) << 8)
#define  AE2_PCLK_CFG_DIV_MASK         AE2_PCLK_CFG_DIVIDER(0xFF)
#define  AE2_ACLK_CFG_DIVIDER(_d)      (((_d) & 0xFF) << 0)
#define  AE2_ACLK_CFG_DIV_MASK         AE2_ACLK_CFG_DIVIDER(0xFF)

// because PMGR.h on sl58950x does not use the rXXX syntax, define it here
#if TARGET_S5L8950XAE2
#define rPMGR_CLK_CFG_DIV_MASK PMGR_CLK_CFG_DIV_MASK
#define rPMGR_CLK_CFG_DIVIDER PMGR_CLK_CFG_DIVIDER
#define rPMGR_CLK_CFG_PENDING PMGR_CLK_CFG_PENDING
#endif

#if TARGET_S5L8940XAE2 || TARGET_S5L8945XAE2
static const size_t ClockStatesStepLowToHigh[][2] = {
	{ 16, 1 },
	{ 4, 1 },
	{ 4, 2 },
	{ 2, 2 },
};
#else
static const size_t ClockStatesStepLowToHigh[][2] = {
	{ 8, 1 },
	{ 2, 1 },
	{ 2, 2 },
	{ 1, 2 },
};
#endif

typedef enum
{
        kInternalClockStateLow = 0,
        kInternalClockStateHigh = 3,
} internal_clockstate_t;

uint32_t GetAudioClockDivider()
{
	return rPMGR_AUDIO_CLK_CFG & rPMGR_CLK_CFG_DIV_MASK;
}

void SetAudioClockDivider(uint32_t value)
{
	rPMGR_AUDIO_CLK_CFG = (rPMGR_AUDIO_CLK_CFG & ~rPMGR_CLK_CFG_DIV_MASK) | rPMGR_CLK_CFG_DIVIDER(value);
	while (rPMGR_AUDIO_CLK_CFG & rPMGR_CLK_CFG_PENDING)
		;
}

uint32_t GetACLKDivider()
{
	return (rAE2_ACSCSR & AE2_ACLK_CFG_DIV_MASK) + 1;
}

void SetACLKDivider(uint32_t value)
{
	rAE2_ACSCSR = (rAE2_ACSCSR & ~AE2_ACLK_CFG_DIV_MASK) | AE2_ACLK_CFG_DIVIDER(value - 1);
	while (rAE2_ACSCSR & AE2_CLK_CFG_PENDING)
		;
}

bool ConfirmClocksValid()
{
	uint32_t AudioDivider = GetAudioClockDivider();
	uint32_t ACLKDivider = GetACLKDivider();
	dprintf(DEBUG_SPEW, "*** clocks at %d %d \n", AudioDivider, ACLKDivider);
	return ((AudioDivider == ClockStatesStepLowToHigh[kInternalClockStateHigh][0]) &&
		(ACLKDivider == ClockStatesStepLowToHigh[kInternalClockStateHigh][1]))
		||
		((AudioDivider == ClockStatesStepLowToHigh[kInternalClockStateLow][0]) &&
		(ACLKDivider == ClockStatesStepLowToHigh[kInternalClockStateLow][1]));
}

void SetClocksLow()
{
	if (GetAudioClockDivider() == ClockStatesStepLowToHigh[kInternalClockStateLow][0])
		return;
	uint32_t whichState = kInternalClockStateHigh;
	while (whichState != kInternalClockStateLow)
	{
		// increment first, then apply the settings
		--whichState;
		SetAudioClockDivider(ClockStatesStepLowToHigh[whichState][0]);
		SetACLKDivider(ClockStatesStepLowToHigh[whichState][1]);
	}
}

void SetClocksHigh()
{
	if (GetAudioClockDivider() == ClockStatesStepLowToHigh[kInternalClockStateHigh][0])
		return;
	uint32_t whichState = kInternalClockStateLow;
	while (whichState != kInternalClockStateHigh)
	{
		// increment first, then apply the settings
		++whichState;
		SetAudioClockDivider(ClockStatesStepLowToHigh[whichState][0]);
		SetACLKDivider(ClockStatesStepLowToHigh[whichState][1]);
	}
}


void SetClockState(clockrequest_reason_t reason, clockvalue_t value)
{
	static clockvalue_t currentRequestValue[kClockRequestMax];
	clockvalue_t newValue = kClockValueHigh;
	
	if (!ParticipateInClockStateManagement())
	{
		return ;
	}

	bool result = true;
	dprintf(DEBUG_SPEW, "*** requesting clock to %d (for %d)***\n", value, reason);
	// turn interrupts off
	enter_critical_section();

	// make sure our arguments are valid
	require((result = value < kClockValueMax), Exit);
	require((result = reason < kClockRequestMax), Exit);
	// assert clocks in a valid state
	require((result = ConfirmClocksValid()), Exit);

	// We modify the actual value of the clock state depending on the value of the current requests
	currentRequestValue[reason] = value;
	
	// If all requests are for kClockValueLow, we switch to the slowest rate
	uint32_t n;
	for(n=0; n<kClockRequestMax; n++)
	{
		dprintf(DEBUG_SPEW, "*** clockReason[%d] = %d ***\n", n, currentRequestValue[n]);
		if (kClockValueHigh==currentRequestValue[n])
		{
			break;
		}	
	}
	if (kClockRequestMax==n)
	{
		newValue = kClockValueLow;
	}

	if (newValue == kClockValueLow)
	{
		SetClocksLow();
	}
	if (newValue == kClockValueHigh)
	{
		SetClocksHigh();
	}

	// assert clocks in a valid state
	require((result = ConfirmClocksValid()), Exit);
Exit:
	dprintf(DEBUG_INFO, "*** setting clock to %s (result %d) ***\n", (newValue==kClockValueHigh)?"HIGH":"LOW", result);
	// turn interrupts on
	exit_critical_section();
}

#endif

