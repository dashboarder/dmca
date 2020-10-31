/*
 * Copyright (C) 2010-2011 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */

/**
 * The PIO object pretends to be a DMA for MCA.
 */

#include "pio_ae2.h"
#include "ae2_i2s.h"
#include "ae2_mca.h"
#include <drivers/audio/audio.h>
#include <debug.h>
#include <stdlib.h>


//Per AE2 Local DMA Controller/Requests
static void* sDMAAddressRX[kAudioDevice_Last] = { (void*)rI2S0_RXDB, (void*)rI2S1_RXDB, (void*)rI2S2_RXDB, (void*)rI2S3_RXDB, (void*)rMCA0_RXDATA, (void*)rMCA1_RXDATA };
static void* sDMAAddressTX[kAudioDevice_Last] = { (void*)rI2S0_TXDB, (void*)rI2S1_TXDB, (void*)rI2S2_TXDB, (void*)rI2S3_TXDB, (void*)rMCA0_TXDATA, (void*)rMCA1_TXDATA };

static internal_pio_object_t* sPIOObjects[kNumPIOs] = { NULL };
static uint32_t sPIORunning = 0;

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

dma_object_t create_dma_object(void *buffer, AudioDevice_Index device, DMALinkedListItem *chain, DMADirection direction, size_t bytesToTransfer)
{
	dprintf(DEBUG_CRITICAL, "Creating a PIO object\n");
	// hardcoding to support only MCA right now.
	if ((device >= kAudioDevice_Last) || ((direction != kDirectionIn) && (direction != kDirectionOut)) || (device != kMCA_0))
	{
		dprintf(DEBUG_CRITICAL, "oops, bad arg\n");
		return NULL;
	}
	uint32_t whichPIO = (direction == kDirectionIn) ? kPIOReceive : kPIOTransmit;
	// fail if device already exists
	if (sPIOObjects[whichPIO])
	{
		dprintf(DEBUG_CRITICAL, "PIO already exists\n");
		return NULL;
	}
	internal_pio_object_t *This = (internal_pio_object_t*)malloc(sizeof(internal_pio_object_t));
	if (This)
	{
		This->mWhichDevice = whichPIO;
		This->mInterruptHandler = NULL;
		This->mInterruptHandlerData = NULL;
		for (uint32_t i = kFrameError; i < kError_last; ++i)
		{
			This->mErrorCount[i] = 0;
		}
		if (direction == kDirectionIn)
		{
			setupPIO(This, sDMAAddressRX[device], buffer, chain, bytesToTransfer);
		}
		else
		{
			setupPIO(This, buffer, sDMAAddressTX[device], chain, bytesToTransfer);
		}
		sPIOObjects[This->mWhichDevice] = This;
		// we can do this here repeatedly
		install_int_handler(AE2_INT_MCA0, handleAudioDeviceInterrupt, NULL);
	}
	return This;
}

void destroy_dma_object(dma_object_t dma)
{
	// preliminary stop
	stopDMAObject(dma, true);
	internal_pio_object_t *This = (internal_pio_object_t*)dma;
	if (This)
	{
		sPIOObjects[This->mWhichDevice] = NULL;
		free(This);
	}
}

uint32_t getErrorCount(dma_object_t dma, Error_Index which)
{
	// preliminary stop
	internal_pio_object_t *This = (internal_pio_object_t*)dma;
	if (This)
	{
		return This->mErrorCount[which];
	}
	return 0;
}

void setupInterruptHandler(dma_object_t dma, int_handler handler, void *arg)
{
	internal_pio_object_t *This = (internal_pio_object_t*)dma;
	if (This)
	{
		This->mInterruptHandler = handler;
		This->mInterruptHandlerData = arg;
	}
}

void setupErrorHandler(dma_object_t dma, int_handler handler, void *arg)
{
}

void startDMAObject(dma_object_t dma)
{
	internal_pio_object_t *This = (internal_pio_object_t*)dma;
	if (This)
	{
		startPIO(This);

		// we wait until two of them are ready before we unmask the int
		sPIORunning |= (1 << This->mWhichDevice);
		if (sPIORunning == 3)
		{
			unmask_int(AE2_INT_MCA0);
		}
	}
}

void stopDMAObject(dma_object_t dma, bool immediate)
{
	internal_pio_object_t *This = (internal_pio_object_t*)dma;
	if (This)
	{
		sPIORunning &= ~(1 << This->mWhichDevice);
		if (sPIORunning != 3)
		{
			mask_int(AE2_INT_MCA0);
		}
		stopPIO(This);
	}
}

bool receivePIOData(DMALinkedListItem * item)
{
	// we do burst sizes of 16-bit data
	int16_t data[kBurstSize];
	volatile uint32_t * src = (volatile uint32_t *)item->source;
	for (size_t i = 0; i < sizeof(data)/sizeof(data[0]); ++i)
	{
		data[i] = *src;
	}
	uint8_t * dst = (uint8_t*)item->destination;
	memcpy(dst, data, sizeof(data));
	dst += sizeof(data);
	item->destination = (uint32_t) dst;
	item->control -= sizeof(data);
	return !item->control;
}

bool transmitPIOData(DMALinkedListItem * item)
{
	// we do burst sizes of 16-bit data
	int16_t data[kBurstSize];
	uint8_t * src = (uint8_t*)item->source;
	memcpy(data, src, sizeof(data));
	volatile uint32_t * dst = (volatile uint32_t *)item->destination;
	for (size_t i = 0; i < sizeof(data)/sizeof(data[0]); ++i)
	{
		*dst = data[i];
	}
	src += sizeof(data);
	item->source = (uint32_t) src;
	item->control -= sizeof(data);
	return !item->control;
}

void setupNextLLI(DMALinkedListItem * item)
{
	*item = *(item->next);
}

void handleAudioDeviceInterrupt()
{
	uint32_t status = readMCA0Reg(rMCASTATUS);

	// <rdar://problem/13467070> Panic when setting AppleSongbirdDSP sidetone EQ
	// clear any sort of sticky-bit errors.
	uint32_t errors = status & ( rMCASTATUS_FRAMEEEROR_MASK | rMCASTATUS_RXOVERRUN_MASK | rMCASTATUS_RXUNDERRUN_MASK | rMCASTATUS_TXOVERRUN_MASK | rMCASTATUS_TXUNDERRUN_MASK );
	writeMCA0Reg(rMCASTATUS, errors);
	if (errors)
	{
		if (sPIOObjects[kPIOReceive])
		{
			if (status & rMCASTATUS_FRAMEEEROR_MASK) ++sPIOObjects[kPIOReceive]->mErrorCount[kFrameError];
			if (status & rMCASTATUS_RXOVERRUN_MASK) ++sPIOObjects[kPIOReceive]->mErrorCount[kRXOverrun];
			if (status & rMCASTATUS_RXUNDERRUN_MASK) ++sPIOObjects[kPIOReceive]->mErrorCount[kRXUnderrun];
			if (status & rMCASTATUS_TXOVERRUN_MASK) ++sPIOObjects[kPIOReceive]->mErrorCount[kTXOverrun];
			if (status & rMCASTATUS_TXUNDERRUN_MASK) ++sPIOObjects[kPIOReceive]->mErrorCount[kTXUnderrun];
		}
	}

	if (status & (1 << rMCASTATUS_RXHIGHWATER))
	{
		if (sPIOObjects[kPIOReceive])
		{
			if (receivePIOData(&sPIOObjects[kPIOReceive]->mCurrentDMAItem))
			{
				setupNextLLI(&sPIOObjects[kPIOReceive]->mCurrentDMAItem);
				if (sPIOObjects[kPIOReceive]->mInterruptHandler)
				{
					sPIOObjects[kPIOReceive]->mInterruptHandler(sPIOObjects[kPIOReceive]->mInterruptHandlerData);
				}
			}
		}
	}
	if (status & (1 << rMCASTATUS_TXLOWWATER))
	{
		if (sPIOObjects[kPIOTransmit])
		{
			if (transmitPIOData(&sPIOObjects[kPIOTransmit]->mCurrentDMAItem))
			{
				setupNextLLI(&sPIOObjects[kPIOTransmit]->mCurrentDMAItem);
				if (sPIOObjects[kPIOTransmit]->mInterruptHandler)
				{
					sPIOObjects[kPIOTransmit]->mInterruptHandler(sPIOObjects[kPIOTransmit]->mInterruptHandlerData);
				}
			}
		}
	}
}

void setupPIO(internal_pio_object_t * This, void *src, void *dst, DMALinkedListItem *chain, size_t bytesToTransfer)
{
	uint32_t transferSize = bytesToTransfer;
	uint32_t control = transferSize;
	
	DMALinkedListItem *chainElement = chain;
	while(chainElement != NULL)
	{
		chainElement->control = (uint32_t)control;
		chainElement = chainElement->next;
		if(chainElement == chain)
		{
			//We've populated all of the control values in this circular linked list, we are done here
			break;
		}
	}
	
	This->mCurrentDMAItem.source = (uint32_t)src;
	This->mCurrentDMAItem.destination = (uint32_t)dst;
	This->mCurrentDMAItem.next = chain;
	This->mCurrentDMAItem.control = control;
}

void startPIO(internal_pio_object_t * pio_object)
{
	if (pio_object)
	{
		if (pio_object->mWhichDevice == kPIOReceive)
		{
			uint32_t MCAUNSRXCFG = readMCA0Reg(rMCAUNSRXCFG);
			writeMCA0Reg(rMCAUNSRXCFG, MCAUNSRXCFG | (1 << rMCAUNSRXCFG_IRQ_EN));
		}
		else
		{
			uint32_t MCAUNSTXCFG = readMCA0Reg(rMCAUNSTXCFG);
			writeMCA0Reg(rMCAUNSTXCFG, MCAUNSTXCFG | (1 << rMCAUNSTXCFG_IRQ_EN));
		}
	}
}

void stopPIO(internal_pio_object_t * pio_object)
{
	if (pio_object)
	{
		if (pio_object->mWhichDevice == kPIOReceive)
		{
			uint32_t MCAUNSRXCFG = readMCA0Reg(rMCAUNSRXCFG);
			writeMCA0Reg(rMCAUNSRXCFG, MCAUNSRXCFG & ~((1 << rMCAUNSRXCFG_IRQ_EN)));
		}
		else
		{
			uint32_t MCAUNSTXCFG = readMCA0Reg(rMCAUNSTXCFG);
			writeMCA0Reg(rMCAUNSTXCFG, MCAUNSTXCFG & ~((1 << rMCAUNSTXCFG_IRQ_EN)));
		}
	}
}


