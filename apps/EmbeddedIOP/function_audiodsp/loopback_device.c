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

#include "loopback_device.h"

#include "ae2_mca.h"

#include <arch/arm/arm.h>
#include <drivers/audio/audio.h>
#include <debug.h>
#include <stdlib.h>
#include <platform/timer.h>

enum { kBurstSize = 4 };

typedef struct
{
	audio_unit_t mAudioUnit;
	size_t mBytesPerFrame;
	uint32_t mErrorCount[kAudioDevice_Last];
} internal_loopback_device_t;


uint32_t readReg(uint32_t address)
{
	return *(volatile uint32_t *)address;
}

void writeReg(uint32_t address, uint32_t value)
{
	*(volatile uint32_t *)address = value;
}

uint32_t readMCA0Reg(uint32_t offset)
{
	return readReg(rMCA0_BASE + offset);
}

void writeMCA0Reg(uint32_t offset, uint32_t value)
{
	writeReg(rMCA0_BASE + offset, value);
}


// if this function takes too long, bail.  We cannot hang the system here.
static const utime_t kMaxTime = 1000000;
bool startPIO()
{
	bool result = true;
	const utime_t time_out = system_time() + kMaxTime;
        
	// disable TX and RX
	uint32_t MCATXCFG = readMCA0Reg(rMCATXCFG);
	writeMCA0Reg(rMCATXCFG, MCATXCFG & ~(1));
	uint32_t MCARXCFG = readMCA0Reg(rMCARXCFG);
	writeMCA0Reg(rMCARXCFG, MCARXCFG & ~(1));

	// Flush the RX and TX fifos
	writeMCA0Reg(rMCACTL, 0x300);
	uint32_t MCACTL = readMCA0Reg(rMCACTL);
	while (result && (MCACTL & 0x300))
	{
		MCACTL = readMCA0Reg(rMCACTL);
		result = system_time() < time_out;
	}

	// "Prime" the tx with zeros.
	uint32_t tx_fifo_level = (readMCA0Reg(rMCASTATUS)) & 0x3FF;
	while (result && (tx_fifo_level < 0x8))
	{
		writeMCA0Reg(rMCATXDATA, 0);
		tx_fifo_level = (readMCA0Reg(rMCASTATUS)) & 0x3FF;
		result = system_time() < time_out;
	}

	// enable RX IRQ (we don't need TX IRQ, we only respond when we have RX samples)
	uint32_t MCAUNSRXCFG = readMCA0Reg(rMCAUNSRXCFG);
	writeMCA0Reg(rMCAUNSRXCFG, MCAUNSRXCFG | (1 << rMCAUNSRXCFG_IRQ_EN));

	// enable TX and RX
	writeMCA0Reg(rMCATXCFG, MCATXCFG);
	writeMCA0Reg(rMCARXCFG, MCARXCFG);
	return result;
}

bool stopPIO()
{
	uint32_t MCAUNSRXCFG = readMCA0Reg(rMCAUNSRXCFG);
	writeMCA0Reg(rMCAUNSRXCFG, MCAUNSRXCFG & ~((1 << rMCAUNSRXCFG_IRQ_EN)));
	uint32_t MCAUNSTXCFG = readMCA0Reg(rMCAUNSTXCFG);
	writeMCA0Reg(rMCAUNSTXCFG, MCAUNSTXCFG & ~((1 << rMCAUNSTXCFG_IRQ_EN)));
	return true;
}

static void
handleAudioDeviceErrors(internal_loopback_device_t * This, const uint32_t status)
{
	// <rdar://problem/13467070> Panic when setting AppleSongbirdDSP sidetone EQ
	// clear any sort of sticky-bit errors.
	const uint32_t errors = status & ( rMCASTATUS_FRAMEEEROR_MASK | rMCASTATUS_RXOVERRUN_MASK | rMCASTATUS_RXUNDERRUN_MASK | rMCASTATUS_TXOVERRUN_MASK | rMCASTATUS_TXUNDERRUN_MASK );
	writeMCA0Reg(rMCASTATUS, errors);
	if (errors && This)
	{
		if (status & rMCASTATUS_FRAMEEEROR_MASK) ++This->mErrorCount[kFrameError];
		if (status & rMCASTATUS_RXOVERRUN_MASK) ++This->mErrorCount[kRXOverrun];
		if (status & rMCASTATUS_RXUNDERRUN_MASK) ++This->mErrorCount[kRXUnderrun];
		if (status & rMCASTATUS_TXOVERRUN_MASK) ++This->mErrorCount[kTXOverrun];
		if (status & rMCASTATUS_TXUNDERRUN_MASK) ++This->mErrorCount[kTXUnderrun];
	}
}

static void
handleAudioDeviceInterrupt_wFloat(void *arg)
{
	internal_loopback_device_t * This = (internal_loopback_device_t *)arg;
	const uint32_t status = readMCA0Reg(rMCASTATUS);
	handleAudioDeviceErrors(This, status);

	// where the magic occurs
	if ((status & (1 << rMCASTATUS_RXHIGHWATER)))
	{
		// we do burst sizes of 16-bit data
		int16_t data[kBurstSize];
		for (size_t i = 0; i < kBurstSize; ++i)
		{
			data[i] = readMCA0Reg(rMCARXDATA);
		}
		if (This && This->mAudioUnit)
		{
			AudioUnit_Process(This->mAudioUnit, data, data, sizeof(data)/This->mBytesPerFrame);
		}
		// <rdar://problem/14296993>, only write if there is room in the TX fifo
		const uint32_t tx_fifo_level = status & 0x3FF;
		const uint32_t k_tx_fifo_size = readMCA0Reg(rMCAFIFOSIZE) & 0x3FF;
		if ((tx_fifo_level + kBurstSize) < k_tx_fifo_size) 
		{
			for (size_t i = 0; i < kBurstSize; ++i)
			{
				writeMCA0Reg(rMCATXDATA, data[i]);
			}
		}
	}
}

// enable floating point code
void handleAudioDeviceInterrupt(void * object)
{
	enter_critical_section();
	arm_call_fpsaved(object, &handleAudioDeviceInterrupt_wFloat);
	exit_critical_section();
}




// create a loopback_device that will read from device rx, optional process, and send to device tx
loopback_device_t create_loopback_device(AudioDevice_Index device, audio_unit_t optional_au, size_t bytes_per_frame)
{
	dprintf(DEBUG_CRITICAL, "Creating a loopback_device object\n");
	// hardcoding to support only MCA right now.
	if ((device != kMCA_0))
	{
		dprintf(DEBUG_CRITICAL, "oops, bad arg\n");
		return NULL;
	}
	internal_loopback_device_t *This = (internal_loopback_device_t*)malloc(sizeof(internal_loopback_device_t));
	if (This)
	{
		This->mAudioUnit = optional_au;
		This->mBytesPerFrame = bytes_per_frame;
		for (uint32_t i = kFrameError; i < kError_last; ++i)
		{
			This->mErrorCount[i] = 0;
		}
		// we can do this here repeatedly
		install_int_handler(AE2_INT_MCA0, handleAudioDeviceInterrupt, This);
	}
	return This;
}


void destroy_loopback_device(loopback_device_t device)
{
	// preliminary stop
	stop_loopback_device(device);
	internal_loopback_device_t *This = (internal_loopback_device_t*)device;
	if (This)
	{
		free(This);
	}
}


uint32_t getErrorCount(loopback_device_t dma, Error_Index which)
{
	// preliminary stop
	internal_loopback_device_t *This = (internal_loopback_device_t*)dma;
	if (This)
	{
		return This->mErrorCount[which];
	}
	return 0;
}



bool start_loopback_device(loopback_device_t device)
{
	internal_loopback_device_t *This = (internal_loopback_device_t*)device;
	if (This && startPIO())
	{
		unmask_int(AE2_INT_MCA0);
		return true;
	}
	return false;
}

bool stop_loopback_device(loopback_device_t device)
{
	internal_loopback_device_t *This = (internal_loopback_device_t*)device;
	if (This)
	{
		mask_int(AE2_INT_MCA0);
		return stopPIO();
	}
	return false;
}


