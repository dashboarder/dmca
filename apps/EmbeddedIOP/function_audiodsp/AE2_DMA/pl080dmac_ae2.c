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

#include "pl080dmac_ae2.h"
#include "ae2_i2s.h"
#include "ae2_mca.h"
#include <drivers/audio/audio.h>
#include <debug.h>
#include <stdlib.h>

#ifdef MAP_SRAM_CACHED
#error DMA requires the SRAM to be uncached
#endif


typedef struct
{
	uint32_t mDMAChannel;
	int_handler mInterruptHandler;
	void * mInterruptHandlerData;
	int_handler mErrorHandler;
	void * mErrorHandlerData;
} internal_dma_object_t;

#define validDMAChannel(dmaChannel) (dmaChannel!=0xffffffff)

//Per AE2 Local DMA Controller/Requests
static const uint32_t sDMARequestInterfaceRx[kAudioDevice_Last] = {0, 2, 4, 6, 10, 6};
static const uint32_t sDMARequestInterfaceTx[kAudioDevice_Last] = {1, 3, 5, 7, 11, 7};
static void* sDMAAddressRX[kAudioDevice_Last] = { (void*)rI2S0_RXDB, (void*)rI2S1_RXDB, (void*)rI2S2_RXDB, (void*)rI2S3_RXDB, (void*)rMCA0_RXDATA, (void*)rMCA1_RXDATA };
static void* sDMAAddressTX[kAudioDevice_Last] = { (void*)rI2S0_TXDB, (void*)rI2S1_TXDB, (void*)rI2S2_TXDB, (void*)rI2S3_TXDB, (void*)rMCA0_TXDATA, (void*)rMCA1_TXDATA };

static internal_dma_object_t* sDMAChannelObjects[kDMAChannelsPerController] = { NULL };
static uint32_t sDMACC0InUse = 0;
static uint32_t sDMACC0Running = 0;

uint32_t readReg(uint32_t address)
{
	return *(volatile uint32_t *)address;
}

void writeReg(uint32_t address, uint32_t value)
{
	*(volatile uint32_t *)address = value;
}

uint32_t readDMAReg(uint32_t offset)
{
	return readReg(kDMACBase + offset);
}

void writeDMAReg(uint32_t offset, uint32_t value)
{
	writeReg(kDMACBase + offset, value);
}

uint32_t readDMAChannelReg(uint32_t channel, uint32_t offset)
{
	uint32_t dmaChannelOffset = kDMAC0Base + channel * kDMACRegisterSize;
	return readReg(dmaChannelOffset + offset);
}

void writeDMAChannelReg(uint32_t channel, uint32_t offset, uint32_t value)
{
	uint32_t dmaChannelOffset = kDMAC0Base + channel * kDMACRegisterSize;
	writeReg(dmaChannelOffset + offset, value);
}


dma_object_t create_dma_object(void *buffer, AudioDevice_Index device, DMALinkedListItem *chain, DMADirection direction, size_t bytesToTransfer)
{
	dprintf(DEBUG_CRITICAL, "Creating a DMA object\n");
	if ((device >= kAudioDevice_Last) || ((direction != kDirectionIn) && (direction != kDirectionOut)))
	{
		dprintf(DEBUG_CRITICAL, "oops, bad arg\n");
		return NULL;
	}
	internal_dma_object_t *This = (internal_dma_object_t*)malloc(sizeof(internal_dma_object_t));
	if (This)
	{
		uint32_t dmaChannel = acquireDMAChannel();
		dprintf(DEBUG_CRITICAL, "channel is %d\n", dmaChannel);
		if (dmaChannel == 0xFFFFFFFF)
		{
			free(This);
			return NULL;
		}
		This->mDMAChannel = dmaChannel;
		This->mInterruptHandler = NULL;
		This->mInterruptHandlerData = NULL;
		This->mErrorHandler = NULL;
		This->mErrorHandlerData = NULL;
		configureDMA(This->mDMAChannel);
		if (direction == kDirectionIn)
		{
			setupDMA(This->mDMAChannel, sDMAAddressRX[device], buffer, chain, kDirectionIn, sDMARequestInterfaceRx[device], bytesToTransfer);
		}
		else
		{
			setupDMA(This->mDMAChannel, buffer, sDMAAddressTX[device], chain, kDirectionOut, sDMARequestInterfaceTx[device], bytesToTransfer);
		}
		sDMAChannelObjects[This->mDMAChannel] = This;
		// we can do this here repeatedly
		install_int_handler(AE2_INT_DMACINTTC, handleAudioDeviceDMAInterrupt, NULL);
		install_int_handler(AE2_INT_DMACINTERR, handleAudioDeviceDMAInterruptError, NULL);
	}
	return This;
}

void destroy_dma_object(dma_object_t dma)
{
	// preliminary stop
	stopDMAObject(dma, true);
	internal_dma_object_t *This = (internal_dma_object_t*)dma;
	if (This)
	{
		sDMAChannelObjects[This->mDMAChannel] = NULL;
		freeDMAChannel(This->mDMAChannel);
		free(This);
	}
}

uint32_t getErrorCount(dma_object_t, Error_Index)
{
	return 0;
}

void setupInterruptHandler(dma_object_t dma, int_handler handler, void *arg)
{
	internal_dma_object_t *This = (internal_dma_object_t*)dma;
	if (This)
	{
		This->mInterruptHandler = handler;
		This->mInterruptHandlerData = arg;
	}
}

void setupErrorHandler(dma_object_t dma, int_handler handler, void *arg)
{
	internal_dma_object_t *This = (internal_dma_object_t*)dma;
	if (This)
	{
		This->mErrorHandler = handler;
		This->mErrorHandlerData = arg;
	}
}

void startDMAObject(dma_object_t dma)
{
	internal_dma_object_t *This = (internal_dma_object_t*)dma;
	if (This)
	{
		startDMA(This->mDMAChannel);

		sDMACC0Running |= (1 << This->mDMAChannel);
		if (sDMACC0Running)
		{
			unmask_int(AE2_INT_DMACINTTC);
			unmask_int(AE2_INT_DMACINTERR);
		}
	}
}

void stopDMAObject(dma_object_t dma, bool immediate)
{
	internal_dma_object_t *This = (internal_dma_object_t*)dma;
	if (This)
	{
		if (immediate)
		{
			disableDMAImmediate(This->mDMAChannel);
		}
		else
		{
			disableDMA(This->mDMAChannel);
		}
		sDMACC0Running &= ~(1 << This->mDMAChannel);
		if (!sDMACC0Running)
		{
			mask_int(AE2_INT_DMACINTTC);
			mask_int(AE2_INT_DMACINTERR);
		}
	}
}

void handleAudioDeviceDMAInterrupt()
{
	uint32_t status = readDMAReg(kDMACIntTCStatus);
	for (uint32_t channel = 0; channel < kDMAChannelsPerController; channel++)
	{
		uint32_t mask = 1 << channel;
		if (status & mask)
		{
			writeDMAReg(kDMACIntTCClear, mask); //clear int

			if(sDMAChannelObjects[channel] != NULL && sDMAChannelObjects[channel]->mInterruptHandler)
			{
				sDMAChannelObjects[channel]->mInterruptHandler(sDMAChannelObjects[channel]->mInterruptHandlerData);
			}
		}
	}
}

void handleAudioDeviceDMAInterruptError()
{
	uint32_t status = readDMAReg(kDMACIntErrStatus);
	for (uint32_t channel = 0; channel < kDMAChannelsPerController; channel++)
	{
		uint32_t mask = 1 << channel;
		if (status & mask)
		{
			writeDMAReg(kDMACIntErrClear, mask); //clear int
			if(sDMAChannelObjects[channel] != NULL && sDMAChannelObjects[channel]->mInterruptHandler)
			{
				sDMAChannelObjects[channel]->mInterruptHandler(sDMAChannelObjects[channel]->mInterruptHandlerData);
			}
		}
	}
}

uint32_t acquireDMAChannel(void)
{
	uint32_t result = 0xFFFFFFFF;
	for(uint32_t channel = 0; channel < kDMAChannelsPerController; channel++)
	{
		if(!(sDMACC0InUse & (1 << channel)))
		{
			sDMACC0InUse |= (1 << channel);
			result = channel;
			break;
		}
	}
	if (sDMACC0InUse)
	{
		writeReg(kDMACClockGating, 0);
		writeDMAReg(kDMACConfiguration, DMACConfiguration_M2_Little | DMACConfiguration_M1_Little | DMACConfiguration_E_Enable);
	}
	return result;
}

void freeDMAChannel(uint32_t dmaChannel)
{
	sDMACC0InUse &= ~(1 << dmaChannel);
	// if there are no channels in use, shut down DMA
	if (!sDMACC0InUse)
	{
		uint32_t config = readDMAReg(kDMACConfiguration);
		writeDMAReg(kDMACConfiguration, config & ~DMACConfiguration_E_Enable);
	}
}

void configureDMA(uint32_t dmaChannel)
{
	if(!validDMAChannel(dmaChannel)) return;
	
	uint32_t control = DMACCxControl_I_Enable |
					   DMACCxControl_D_Master1 |
					   DMACCxControl_S_Master1 |
					   DMACCxControl_SWidth_Halfword |
					   DMACCxControl_DWidth_Halfword |
					   DMACCxControl_SBSize_4 |
					   DMACCxControl_DBSize_4;
	
	writeDMAChannelReg(dmaChannel, kDMACCtrlOffset, control);
}

void setupDMA(uint32_t dmaChannel, void *src, void *dst, DMALinkedListItem *chain, DMADirection direction, uint32_t peripheral, size_t bytesToTransfer)
{
	if(!validDMAChannel(dmaChannel)) return;

	uint32_t transferSize = bytesToTransfer >> 1; //assuming burst size of DMACCxControl_DWidth_Halfword	
	uint32_t control = readDMAChannelReg(dmaChannel, kDMACCtrlOffset);

	control &= ~(DMACCxControl_TransferSizeMask           |  // Clear transfer size
				 DMACCxControl_SI_Mask                    |  // Clear source increment
				 DMACCxControl_DI_Mask);                     // Clear destination increment
	
	control |= (transferSize & 0x0FFF);
	
	uint32_t flowControl, sourcePeripheral, destinationPeripheral;
	if(direction == kDirectionIn) // -> peripheral-to-memory
	{
		control |= DMACCxControl_SI_No_Increment;
		control |= DMACCxControl_DI_Increment;
        
		flowControl			  = DMACCxConfiguration_FlowCntrl_Peripheral_to_Memory_DMA;
        sourcePeripheral      = peripheral;
        destinationPeripheral = 0;  //mem
	}
	else if(direction == kDirectionOut) //kDirectionOut -> memory-to-peripheral
	{
		control |= DMACCxControl_SI_Increment;
		control |= DMACCxControl_DI_No_Increment;
        
		flowControl			  = DMACCxConfiguration_FlowCntrl_Memory_to_Peripheral_DMA;
        sourcePeripheral      = 0;  //mem
        destinationPeripheral = peripheral;
	}
	else //kDirectionNone -> mem-to-mem here
	{
		control |= DMACCxControl_SI_Increment;
		control |= DMACCxControl_DI_Increment;

		flowControl			  = DMACCxConfiguration_FlowCntrl_Memory_to_Memory_DMA;
        sourcePeripheral      = 0; //mem
        destinationPeripheral = 0; //mem
	}
	
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
	
	writeDMAChannelReg(dmaChannel, kDMACSourceOffset, (uint32_t)src);
	writeDMAChannelReg(dmaChannel, kDMACDestOffset, (uint32_t)dst);
	writeDMAChannelReg(dmaChannel, kDMACLLIOffset, (uint32_t)chain);
	writeDMAChannelReg(dmaChannel, kDMACCtrlOffset, control);

	flowControl |= DMACCxConfiguration_ITC_Enable;
	flowControl |= DMACCxConfiguration_IError_Enable;
	flowControl |= ((destinationPeripheral & 0xF) << 6);
	flowControl |= ((sourcePeripheral & 0xF) << 1);

	// do this in the start
	//flowControl |= DMACCxConfiguration_E_Enable;
	
	writeDMAChannelReg(dmaChannel, kDMACConfigOffset, flowControl);
}

void startDMA(uint32_t dmaChannel)
{
	if(!validDMAChannel(dmaChannel)) return;

	uint32_t flowControl = readDMAChannelReg(dmaChannel, kDMACConfigOffset);
	writeDMAChannelReg(dmaChannel, kDMACConfigOffset, flowControl |= DMACCxConfiguration_E_Enable);
}

void disableDMA(uint32_t dmaChannel)
{
	if(!validDMAChannel(dmaChannel)) return;

	uint32_t config = readDMAChannelReg(dmaChannel, kDMACConfigOffset);
	writeDMAChannelReg(dmaChannel, kDMACConfigOffset, config | DMACCxConfiguration_Halt);

	config = readDMAChannelReg(dmaChannel, kDMACConfigOffset);
	// give a chance for the DMA to timeout.  But don't wait forever...
	int timeToWait = 0x100;
	while(config & DMACCxConfiguration_Active && (timeToWait-- > 0))
	{
		config = readDMAChannelReg(dmaChannel, kDMACConfigOffset);
	}
	disableDMAImmediate(dmaChannel);
}

void disableDMAImmediate(uint32_t dmaChannel)
{
	if(!validDMAChannel(dmaChannel)) return;

	uint32_t config = readDMAChannelReg(dmaChannel, kDMACConfigOffset);
	writeDMAChannelReg(dmaChannel, kDMACConfigOffset, config & ~DMACCxConfiguration_E_Enable);
    
	writeDMAReg(kDMACIntTCClear, 1 << dmaChannel); // clear any outstanding ints
	writeDMAReg(kDMACIntErrClear, 1 << dmaChannel); // clear any outstanding ints
}

