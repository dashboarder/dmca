/*
 * Copyright (C) 2010-2013 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */

#include <debug.h>
#include <AssertMacros.h>
#include <platform/memmap.h>
#include <platform.h>
#include <platform/timer.h>
#include <sys/task.h>

#include <iop.h>
#include <qwi.h>
#include <qwi_protocol.h>
#include <EmbeddedIOPProtocol.h>
#include <arch.h>
#include <arch/arm/arm.h>
#include <drivers/audio/audio.h>
#include "clock_stepping.h"

#define USE_DMA 0

#include "iop_audiodsp_protocol.h"
#include "iop_au_interface.h"
#include "loopback_process.h"
#include "timestamper.h"
#include "debug_tap.h"
#include "ae2_mca.h"
#if USE_DMA
#include "ae2_i2s.h"
#include "ae2_dma.h"
#else
#include "loopback_device.h"
#endif


static const uint32_t kDefaultSampleRate = 8000;
static const uint32_t kMaxSampleRate = 48000;
// We configure for 0.004 sec of latency.
static const uint32_t kDefaultLatencyInUS = 4000;
static const uint32_t kMaxLatencyInUS = 40000;
// Describing the MCA data format.
static const uint32_t kDefaultNumChannels = 1;
static const uint32_t kMaxNumChannels = 2;
#if USE_DMA
static const uint32_t kValidChannelBitmask = 1;
#endif
static const uint32_t kDefaultSampleSize = 2;

static const size_t kDefaultDMABufferSize = 0x300;

static const AudioDevice_Index kDataDevice = kMCA_0;
#if USE_DMA
static const uint32_t kDataReceivePort = rMCA0_RXDATA;
static const uint32_t kDataTransmitPort = rMCA0_TXDATA;
#endif

static bool audiodsp_message_process(void);

static int iop_audiodsp_task(void *cfg);
static void iop_audiodsp_sleep(int mode);

IOP_FUNCTION(audiodsp, iop_audiodsp_task, 8*1024, AUDIODSP_CONTROL_CHANNEL);
IOP_SLEEP_HOOK(audiodsp, iop_audiodsp_sleep);

static int audiodsp_channel;

#if USE_DMA
static dma_object_t s_input_dma = NULL;
static dma_object_t s_output_dma = NULL;
static void audiodevice_dma_int_err_handler(void *arg);
static void audiodevice_dma_int_handler(void *arg);

static const size_t kNumDMABuffers = 4;
static int16_t *sBuffer[kNumDMABuffers];
static size_t sCurrentProcessBufferIndex;
static DMALinkedListItem *sLLIDMAInput[kNumDMABuffers], *sLLIDMAOutput[kNumDMABuffers];
#endif
#if !USE_DMA
static loopback_device_t s_loopback_device = NULL;
#endif // USE_DMA

// Loopback processing "globals"
static audio_unit_t s_loopback_au = NULL;

// Audio Unit biquad property. 
static void* s_loopback_au_state_property = NULL;
static uint32_t s_loopback_au_state_property_size = 0;
// forward declare here.  Make sure to change this if id changes
static const uint32_t k_loopback_au_state_property_id = 10001;

static loopback_process_t s_loopback_process = NULL;
static debug_tap_t sInputTap = NULL;
static debug_tap_t sOutputTap = NULL;

// forward declaration:
audio_unit_t Create_Loopback_AU(uint32_t sampleRate, uint32_t channels, uint32_t framesToProcess);

/***
 * IMPORTANT!
 * We assume that all the SRAM allocations will total less than 0x1000.
 * Adjust the SRAM Heap in memmap.h if this assumption changes.
 */
static uint32_t sSRAMMallocOffset = 0;
#ifdef AUDIO_SRAM_RESERVE
static const uint32_t kSRAMMallocMax = AUDIO_SRAM_RESERVE;
#else /* !AUDIO_SRAM_RESERVE */
static const uint32_t kSRAMMallocMax = (uint32_t)-1;
#endif /* AUDIO_SRAM_RESERVE */

void* mallocFromSRAM(uint32_t byteSize)
{
	//Make sure its a 4-byte aligned allocation (for DMA xfers)
	byteSize = (byteSize + 3) / 4;
	byteSize *= 4;
	
	//AUDIO_BASE_ADDR is the start of SRAM in AE2
	void *ptr = (void*)(AUDIO_BASE_ADDR + sSRAMMallocOffset);
	sSRAMMallocOffset += byteSize;

	if (sSRAMMallocOffset > kSRAMMallocMax)
		panic("mallocFromSRAM beyond limit");

	memset(ptr, 0, byteSize);

	return ptr;
}

#if USE_DMA
static bool sDMABuffersInitialized = false;
void
initialize_dma_buffers(void)
{
	if (!sDMABuffersInitialized)
	{
		for(uint32_t i = 0; i < kNumDMABuffers; i++)
		{
			sLLIDMAInput[i]  = (DMALinkedListItem *)mallocFromSRAM(sizeof(DMALinkedListItem));
			sLLIDMAOutput[i] = (DMALinkedListItem *)mallocFromSRAM(sizeof(DMALinkedListItem));
			sBuffer[i] = (int16_t *)mallocFromSRAM(kDefaultDMABufferSize);
		}

		//Setup input chain for infinite quad-buffer looping, control setup handled in startDMA
		for(uint32_t i = 0; i < kNumDMABuffers; i++)
		{
			// RX goes into buffer 2 on start, so we start the chain on 3
			sLLIDMAInput[i]->destination = (uint32_t)sBuffer[(i+3)%kNumDMABuffers];
			sLLIDMAInput[i]->next = sLLIDMAInput[(i+1)%kNumDMABuffers];
			sLLIDMAInput[i]->source = kDataReceivePort;

			// TX goes from buffer 0 on start, so we start the chain on 1
			sLLIDMAOutput[i]->source = (uint32_t)sBuffer[(i+1)%kNumDMABuffers];
			sLLIDMAOutput[i]->next = sLLIDMAOutput[(i+1)%kNumDMABuffers];
			sLLIDMAOutput[i]->destination = kDataTransmitPort;
		}
		sDMABuffersInitialized = true;
	}
}

void
reset_dma_buffers(void)
{
	for(uint32_t i = 0; i < kNumDMABuffers; i++)
	{
		memset(sBuffer[i], 0, kDefaultDMABufferSize);
	}
	sCurrentProcessBufferIndex = 1;
}
#endif

static int
iop_audiodsp_task(void *cfg)
{
	struct iop_channel_config *channel = (struct iop_channel_config *)cfg;
	struct task_event* audiodsp_message_event = (struct task_event*) malloc(sizeof(struct task_event));

	dprintf(DEBUG_SPEW, "@@ AudioDSP task starting\n");

	check(kIOPAUDIODSP_COMMAND_SIZE == sizeof(IOPAUDIODSP_Command));

	/* establish the host communications channel */
	event_init(audiodsp_message_event, EVENT_FLAG_AUTO_UNSIGNAL, false);
	dprintf(DEBUG_SPEW, "** opening audiodsp channel\n");
	audiodsp_channel = qwi_instantiate_channel("audio command",
						QWI_ROLE_CONSUMER,
						channel->ring_size,
						(void *)mem_static_map_cached(channel->ring_base),
						(qwi_channel_hook)event_signal,
						audiodsp_message_event);

#if WITH_VFP && !WITH_VFP_ALWAYS_ON
	/* Little doubt we'll need VFP/Neon */
	arch_task_fp_enable(true);
#endif
	
	// create a audio unit if one does not exist
	if (!s_loopback_au)
	{
		dprintf(DEBUG_INFO, "creating loopback\n");
		s_loopback_au = Create_Loopback_AU(kDefaultSampleRate, kDefaultNumChannels, kDefaultSampleSize);
	}

	for (;;) {
		dprintf(DEBUG_SPEW, "@@ waiting for message on audiodsp channel\n");
		while (audiodsp_message_process()) {
			// eat all available messages
		}		
		event_wait(audiodsp_message_event);
	}

	return(0);
}


audio_unit_t Create_Loopback_AU(uint32_t sampleRate, uint32_t channels, uint32_t framesToProcess)
{
	audio_unit_t loopback_au = NULL;
	dprintf(DEBUG_INFO, "creating loopback\n");
#if USE_SIDETONE
	loopback_au = Create_AUSidetone(sampleRate, channels, kDefaultSampleSize);
#else
	loopback_au = Create_AUNull(sampleRate, channels, kDefaultSampleSize);
#endif
	if (loopback_au && s_loopback_au_state_property && s_loopback_au_state_property_size)
	{
		// if we had a previous property
		uint32_t propSize = s_loopback_au_state_property_size;
		bool propWritable = true;
		if (/*noErr*/0 == AudioUnit_GetPropertyInfo(loopback_au, k_loopback_au_state_property_id, &propSize, &propWritable))
		{
			if (propWritable && propSize == s_loopback_au_state_property_size)
			{
				dprintf(DEBUG_INFO, "Restoring previous SidetoneEQ values\n");
				if (/*noErr*/0 != AudioUnit_SetProperty(loopback_au, k_loopback_au_state_property_id, s_loopback_au_state_property, s_loopback_au_state_property_size))
				{
					dprintf(DEBUG_INFO, "Warning, could not restore previous SidetoneEQ values\n");
					// now what?  Destroy the AU and start over again
					Destroy_AudioUnit(loopback_au);
					loopback_au = NULL;
#if USE_SIDETONE
					loopback_au = Create_AUSidetone(sampleRate, channels, kDefaultSampleSize);
#else
					loopback_au = Create_AUNull(sampleRate, channels, kDefaultSampleSize);
#endif
				}
			}
		}
	}
	return loopback_au;
}


void Destroy_Loopback_AU(audio_unit_t loopback_au)
{
	if (loopback_au)
	{
		uint32_t propSize = s_loopback_au_state_property_size;
		bool propWritable = true;
		if (/*noErr*/0 == AudioUnit_GetPropertyInfo(loopback_au, k_loopback_au_state_property_id, &propSize, &propWritable))
		{
			// clear the old property block if it won't fit
			if (s_loopback_au_state_property && s_loopback_au_state_property_size != propSize)
			{
				free(s_loopback_au_state_property);
				s_loopback_au_state_property = NULL;
				s_loopback_au_state_property_size = 0;
			}
			// if we don't have somewhere to write the properties, create it:
			if (!s_loopback_au_state_property && (0 != propSize))
			{
				s_loopback_au_state_property = malloc(propSize);
				s_loopback_au_state_property_size = propSize;
			}
			// if we do have somewhere to write it, save off what we have
			if (s_loopback_au_state_property)
			{
				dprintf(DEBUG_INFO, "Saving SidetoneEQ values\n");
				if (/*noErr*/0 != AudioUnit_GetProperty(loopback_au, k_loopback_au_state_property_id, s_loopback_au_state_property, &s_loopback_au_state_property_size))
				{
					dprintf(DEBUG_INFO, "Warning, could not save SidetoneEQ values\n");
					// for whatever reason, we didn't save the property.  free it so we don't try to restore it
					free(s_loopback_au_state_property);
					s_loopback_au_state_property = NULL;
					s_loopback_au_state_property_size = 0;
				}
			}
		}
		Destroy_AudioUnit(loopback_au);
		loopback_au = NULL;
	}
}


#if USE_DMA
// Destroy the loopback processing and AU.  Assumes that loopback is not running
bool Destroy_Loopback_Processing()
{
	if (s_loopback_process)
	{
		destroy_loopback_process(s_loopback_process);
		s_loopback_process = NULL;
	}
	if (s_loopback_au)
	{
		Destroy_Loopback_AU(s_loopback_au);
		s_loopback_au = NULL;
	}
	return true;
}

// Creates the loopback processing and AU.  Assumes that loopback is not running
// will always destroy any existing loopbacks
bool Create_Loopback_Processing(uint32_t sampleRate, uint32_t channels, uint32_t framesToProcess)
{
	Destroy_Loopback_Processing();
	// initialize the loopback processing.  It will be enabled later
	// create a audio unit if one does not exist
	if (!s_loopback_au)
	{
		dprintf(DEBUG_INFO, "creating loopback\n");
		s_loopback_au = Create_Loopback_AU(sampleRate, channels, kDefaultSampleSize);
	}
	dprintf(DEBUG_INFO, "loopback is %p\n", s_loopback_au);
	// if we have an audio unit, create a loopback proces
	if (s_loopback_au && !s_loopback_process)
	{
		dprintf(DEBUG_INFO, "creating loopback process\n");
		s_loopback_process = create_loopback_process(s_loopback_au, framesToProcess, channels, kDefaultSampleSize, kValidChannelBitmask);
	}
	dprintf(DEBUG_INFO, "loopback process is %p\n", s_loopback_process);
	dprintf(DEBUG_INFO, "free mem after %d\n", heap_get_free_mem());

	if (s_loopback_process)
	{
		return true;
	}
	return false;
}
#endif

static bool s_loopback_enabled = false;
// return true if loop processing stopped
bool Stop_Loopback_Processing()
{
	// if loopback processing not running, done
	if(!s_loopback_enabled)
	{
		return !s_loopback_enabled;
	}
#if USE_DMA
	// log any sort of errors encountered
	if (s_input_dma || s_output_dma)
	{
		for (uint32_t i = kFrameError; i < kError_last; ++i)
		{
			// from Alexei:
			// Ignore the frame errors.  They're not causing
			// interrupts and they are expected (because L81
			// masters BCLK at 12MHz always, which is more than the
			// exact number of bits specified in the MCA config;
			// unfortunately, there isn't a way to configure MCA to
			// say "give me a frame error if there are fewer bits,
			// but not if there are more", which is what you'd
			// want.)
			if (kFrameError == i)
			{
				continue;
			}

			uint32_t error_count = ((s_input_dma) ? getErrorCount(s_input_dma, i) : 0) + ((s_output_dma) ? getErrorCount(s_output_dma, i) : 0);
			if (error_count)
			{
				dprintf(DEBUG_CRITICAL, "DMA encountered %d %s error%s!\n", error_count, kErrorTypeStr[i], error_count > 1 ? "s" : "");
			}
		}
	}
	dprintf(DEBUG_INFO, "Going to stop the DMA\n");
	if (s_input_dma)
	{
		destroy_dma_object(s_input_dma);
		s_input_dma = NULL;
	}
	if (s_output_dma)
	{
		destroy_dma_object(s_output_dma);
		s_output_dma = NULL;
	}
	dprintf(DEBUG_CRITICAL, "Stopped the DMA\n");
#else
	// log any sort of errors encountered
	if (s_loopback_device)
	{
		for (uint32_t i = kFrameError; i < kError_last; ++i)
		{
			// from Alexei:
			// Ignore the frame errors.  They're not causing
			// interrupts and they are expected (because L81
			// masters BCLK at 12MHz always, which is more than the
			// exact number of bits specified in the MCA config;
			// unfortunately, there isn't a way to configure MCA to
			// say "give me a frame error if there are fewer bits,
			// but not if there are more", which is what you'd
			// want.)
			if (kFrameError == i)
			{
				continue;
			}

			uint32_t error_count = getErrorCount(s_loopback_device, i);
			if (error_count)
			{
				dprintf(DEBUG_CRITICAL, "loopback encountered %d %s error%s!\n", error_count, kErrorTypeStr[i], error_count > 1 ? "s" : "");
			}
		}
	}
	dprintf(DEBUG_INFO, "Going to stop the loopback\n");
	if (s_loopback_device)
	{
		destroy_loopback_device(s_loopback_device);
		s_loopback_device = NULL;
	}
	dprintf(DEBUG_CRITICAL, "Stopped the loopback\n");
#endif

	SetClockState(kClockRequestLoopback, kClockValueLow);

	s_loopback_enabled = false;
	return !s_loopback_enabled;
}


// return true if loopback processing started
bool Start_Loopback_Processing(uint32_t sampleRate, uint32_t channels, uint32_t framesToProcess)
{
	dprintf(DEBUG_INFO, "Start_Loopback_Processing(sampleRate %d, channels %d, framesToProcess %d\n", sampleRate, channels, framesToProcess);
	if (!Stop_Loopback_Processing())
	{
		dprintf(DEBUG_CRITICAL, "Could not stop loopback\n");
		return false;
	}

	// if loopback processing already running, done
	if (s_loopback_enabled)
	{
		return s_loopback_enabled;
	}

	if (sampleRate > 32000)
	{
		dprintf(DEBUG_INFO, "Increasing clock rate for loopback\n");
		SetClockState(kClockRequestLoopback, kClockValueHigh);
	}
	
#if USE_DMA
	// create the loopback proessing with this sample rate
	if (Create_Loopback_Processing(sampleRate, channels, framesToProcess) && s_loopback_process)
	{
		dprintf(DEBUG_INFO, "starting up DMA\n");
		initialize_dma_buffers();

		reset_dma_buffers();
		if (s_input_dma)
		{
			destroy_dma_object(s_input_dma);
			s_input_dma = NULL;
		}
		s_input_dma = create_dma_object(sBuffer[2], kDataDevice, sLLIDMAInput[0], kDirectionIn, framesToProcess*channels*kDefaultSampleSize);
		if (s_output_dma)
		{
			destroy_dma_object(s_output_dma);
			s_output_dma = NULL;
		}
		s_output_dma = create_dma_object(sBuffer[0], kDataDevice, sLLIDMAOutput[0], kDirectionOut, framesToProcess*channels*kDefaultSampleSize);
		if (s_input_dma && s_output_dma)
		{
			setupInterruptHandler(s_input_dma, audiodevice_dma_int_handler, s_loopback_process);
			setupErrorHandler(s_input_dma, audiodevice_dma_int_err_handler, NULL);
			setupErrorHandler(s_output_dma, audiodevice_dma_int_err_handler, NULL);
			startDMAObject(s_input_dma);
			startDMAObject(s_output_dma);
			s_loopback_enabled = true;
		}
		else
		{
			dprintf(DEBUG_CRITICAL, "could not create DMA objects!\n");
		}
	}
#else
	if (s_loopback_au)
	{
		Destroy_Loopback_AU(s_loopback_au);
		s_loopback_au = NULL;
	}
	dprintf(DEBUG_INFO, "creating loopback\n");
	s_loopback_au = Create_Loopback_AU(sampleRate, channels, kDefaultSampleSize);
	// create the loopback proessing with this sample rate
	if (s_loopback_au)
	{
		dprintf(DEBUG_INFO, "starting up loopback_device\n");
		if (s_loopback_device)
		{
			destroy_loopback_device(s_loopback_device);
			s_loopback_device = NULL;
		}
		s_loopback_device = create_loopback_device(kDataDevice, s_loopback_au, channels * kDefaultSampleSize);
		if (s_loopback_device)
		{
			s_loopback_enabled = start_loopback_device(s_loopback_device);
		}
		else
		{
			dprintf(DEBUG_CRITICAL, "could not create loopback device!\n");
		}
	}
#endif
	return s_loopback_enabled;
}


bool Check_Parameter_Message_Valid(IOPAUDIODSP_MODULE_COMMAND *command)
{
	if(command->mModule != kIOPAUDIODSP_MODULE_LOOPBACK_PROCESSING)
	{
		dprintf(DEBUG_CRITICAL, "Invalid kIOPAUDIODSP_OPCODE_SET_PARAMETER received!\n");
		return false;
	}
	if(!s_loopback_au)
	{
		dprintf(DEBUG_CRITICAL, "No existing loopback processing audio unit!\n");
		return false;
	}
	return true;
}

bool Check_Property_Message_Valid(IOPAUDIODSP_MODULE_COMMAND *command)
{
	if (!Check_Parameter_Message_Valid(command))
	{
		return false;
	}
	size_t max_size_for_property = sizeof(IOPAUDIODSP_Command) - ((uint32_t)&command->mProperty.mPropertyData - (uint32_t)command); 
	//make sure its formatted correctly ...
	if(command->mProperty.mPropertySizeBytes > max_size_for_property)
	{
		dprintf(DEBUG_CRITICAL, "Invalid message size received!\n");
		return false;
	}
	return true;
}

bool Handle_OPCODE_GET_PARAMETER(IOPAUDIODSP_MODULE_COMMAND *command)
{
	if (!Check_Parameter_Message_Valid(command))
	{
		return false;
	}
	//XXX add support for array of parameters (perhaps with mNumberOfParameterValues)
	if (AudioUnit_GetParameter(s_loopback_au, command->mParameter.mParameterID, &command->mParameter.mParameterValue))
	{
		dprintf(DEBUG_CRITICAL, "FAILED getting parameter(%d) on loopback process!\n", command->mParameter.mParameterID);
		return false;
	}
	return true;
}

bool Handle_OPCODE_SET_PARAMETER(IOPAUDIODSP_MODULE_COMMAND *command)
{
	if (!Check_Parameter_Message_Valid(command))
	{
		return false;
	}
	//XXX add support for array of parameters (perhaps with mNumberOfParameterValues)
	if (AudioUnit_SetParameter(s_loopback_au, command->mParameter.mParameterID, command->mParameter.mParameterValue))
	{
		dprintf(DEBUG_CRITICAL, "FAILED setting parameter(%d) on loopback process!\n", command->mParameter.mParameterID);
		return false;
	}
	return true;
}

bool Handle_OPCODE_GET_PROPERTY(IOPAUDIODSP_MODULE_COMMAND *command)
{
	if (!Check_Property_Message_Valid(command))
	{
		return false;
	}
	uint32_t max_size_for_property = sizeof(IOPAUDIODSP_Command) - ((uint32_t)&command->mProperty.mPropertyData - (uint32_t)command); 
	if (AudioUnit_GetProperty(s_loopback_au, command->mProperty.mPropertyID, command->mProperty.mPropertyData, &max_size_for_property) )
	{
		dprintf(DEBUG_CRITICAL, "FAILED getting property(%d) on loopback process!\n", command->mProperty.mPropertyID);
		return false;
	}
	command->mProperty.mPropertySizeBytes = max_size_for_property;
	return true;
}

bool Handle_OPCODE_SET_PROPERTY(IOPAUDIODSP_MODULE_COMMAND *command)
{
	if (!Check_Property_Message_Valid(command))
	{
		return false;
	}
	//XXX add support for array of parameters (perhaps with mNumberOfParameterValues)
	if (AudioUnit_SetProperty(s_loopback_au, command->mProperty.mPropertyID, command->mProperty.mPropertyData, command->mProperty.mPropertySizeBytes))
	{
		dprintf(DEBUG_CRITICAL, "FAILED setting parameter(%d) on loopback process!\n", command->mProperty.mPropertyID);
		return false;
	}
	return true;
}


static bool
audiodsp_message_process(void)
{
	uint32_t				message;
	IOPAUDIODSP_Command*	command;

	dprintf(DEBUG_SPEW, "@@ handling host message\n");

	/* look to see if there's an item waiting for us */
	if (qwi_receive_item(audiodsp_channel, &message) == -1)
		return(false);
	
	dprintf(DEBUG_SPEW, "@@ received audio message\n");

	/* find the command structure based on the message */
	command = (IOPAUDIODSP_Command*)mem_static_map_cached(message);

	/*
	 * Flush any cached item contents we might have lying around - we are guaranteed
	 * that the command size is a multiple of our cacheline size.
	 */
	platform_cache_operation(CACHE_INVALIDATE,
				 (void *)command, 
				 sizeof(*command));

	// assume every command is a failure unless it gets set to success
	command->iopaudiodsp.mStatus = kIOPAUDIODSP_STATUS_FAILURE;
	switch (command->iopaudiodsp.mOpcode) {

		case kIOPAUDIODSP_OPCODE_START_LOOPBACK_PROCESSING:
		{
			uint32_t sampleRate = kDefaultSampleRate;
			uint32_t channels = kDefaultNumChannels;
			uint32_t latency = kDefaultLatencyInUS;

			// read values from the paramter block
			uint32_t* parameterBlock = (uint32_t *)command->start.mAdditionalParameters;
			int32_t parameterBlockSize = command->start.mAdditionalParametersSizeBytes;
			if (parameterBlockSize > 0)
			{
				sampleRate = *parameterBlock;
				sampleRate = (sampleRate < kMaxSampleRate) ? sampleRate : kMaxSampleRate;
				++parameterBlock;
				parameterBlockSize -= sizeof(parameterBlock[0]);
			}
			if (parameterBlockSize > 0)
			{
				latency = *parameterBlock;
				latency = (latency < kMaxLatencyInUS) ? latency : kMaxLatencyInUS;
				++parameterBlock;
				parameterBlockSize -= sizeof(parameterBlock[0]);
			}
			if (parameterBlockSize > 0)
			{
				channels = *parameterBlock;
				channels = (channels < kMaxNumChannels) ? channels : kMaxNumChannels;
				++parameterBlock;
				parameterBlockSize -= sizeof(parameterBlock[0]);
			}
			if (!sampleRate || !latency || !channels)
			{
				dprintf(DEBUG_CRITICAL, "Some parameter not correct (sampleRate = %d, latency = %d, channels = %d)\n", sampleRate, latency, channels);
				break;
			}
			dprintf(DEBUG_CRITICAL, "using values sample rate %d, latency %d, channels %d\n", sampleRate, latency, channels);
			uint32_t bytesToProcess = ((sampleRate * latency * channels) / 1000000) * kDefaultSampleSize;
			// because we have an input buffer and process buffer, need to cut the value in half
			bytesToProcess /= 2;
			bytesToProcess = (bytesToProcess < kDefaultDMABufferSize) ? bytesToProcess : kDefaultDMABufferSize;
			// because the DMA will do burst-4, make sure we have a valid bytes to process size:
			uint32_t burstSize = kDefaultSampleSize * 4;
			bytesToProcess = ((bytesToProcess + burstSize - 1)/burstSize) * burstSize;
			bytesToProcess = (bytesToProcess < kDefaultDMABufferSize) ? bytesToProcess : kDefaultDMABufferSize;
			uint32_t framesToProcess = bytesToProcess / (channels * kDefaultSampleSize);
			dprintf(DEBUG_CRITICAL, "Start command with sample rate %d, channels %d, frames %d\n", sampleRate, channels, framesToProcess);

			// now what would the latency be?
			latency = (framesToProcess * 1000000) / sampleRate;
			// because we had to half the latency, we need to double it.
			latency *= 2;

			if (Start_Loopback_Processing(sampleRate, channels, framesToProcess))
			{
				command->iopaudiodsp.mStatus = kIOPAUDIODSP_STATUS_SUCCESS;
				dprintf(DEBUG_CRITICAL, "Started loopback processing!\n");
			}
			else
			{
				dprintf(DEBUG_CRITICAL, "Could not start loopback processing!\n");
			}

			// write values back to the paramter block
			parameterBlock = (uint32_t *)command->start.mAdditionalParameters;
			parameterBlockSize = command->start.mAdditionalParametersSizeBytes;
			if (parameterBlockSize > 0)
			{
				*parameterBlock = sampleRate;
				++parameterBlock;
				parameterBlockSize -= sizeof(parameterBlock[0]);
			}
			if (parameterBlockSize > 0)
			{
				*parameterBlock = latency;
				++parameterBlock;
				parameterBlockSize -= sizeof(parameterBlock[0]);
			}
			if (parameterBlockSize > 0)
			{
				*parameterBlock = channels;
				++parameterBlock;
				parameterBlockSize -= sizeof(parameterBlock[0]);
			}
		}
		break;
			
		case kIOPAUDIODSP_OPCODE_STOP_LOOPBACK_PROCESSING:
		{
			if (Stop_Loopback_Processing())
			{
				command->iopaudiodsp.mStatus = kIOPAUDIODSP_STATUS_SUCCESS;
				dprintf(DEBUG_CRITICAL, "Stopped loopback processing!\n");
			}
		}
		break;
			
		case kIOPAUDIODSP_OPCODE_GET_PROPERTY:
		{
			if (Handle_OPCODE_GET_PROPERTY(&command->module_command))
			{
				command->iopaudiodsp.mStatus = kIOPAUDIODSP_STATUS_SUCCESS;
			}
		}
		break;
			
		case kIOPAUDIODSP_OPCODE_SET_PROPERTY:
		{
			if (Handle_OPCODE_SET_PROPERTY(&command->module_command))
			{
				command->iopaudiodsp.mStatus = kIOPAUDIODSP_STATUS_SUCCESS;
			}
		}
		break;
			
		case kIOPAUDIODSP_OPCODE_GET_PARAMETER:
		{
			if (Handle_OPCODE_GET_PARAMETER(&command->module_command))
			{
				command->iopaudiodsp.mStatus = kIOPAUDIODSP_STATUS_SUCCESS;
			}
		}
		break;
			
		case kIOPAUDIODSP_OPCODE_SET_PARAMETER:
		{
			if (Handle_OPCODE_SET_PARAMETER(&command->module_command))
			{
				command->iopaudiodsp.mStatus = kIOPAUDIODSP_STATUS_SUCCESS;
			}
		}
		break;
			
		case kIOPAUDIODSP_OPCODE_INITTIMESTAMP:
		{
			dprintf(DEBUG_CRITICAL, "Timestamp buffer is 0x%x\n", command->init_timestamp.mTimeStamperBufferAddr);
			set_timestamper_message_buffer((IOPAUDIODSP_Command *)mem_static_map_cached(command->init_timestamp.mTimeStamperBufferAddr));
			command->iopaudiodsp.mStatus = kIOPAUDIODSP_STATUS_SUCCESS;
		}
		break;
			
		case kIOPAUDIODSP_OPCODE_DO_TRANSFER:
		{
			dprintf(DEBUG_CRITICAL, "Starting Transfer\n");
			dprintf(DEBUG_CRITICAL, "Index %d\n", command->do_transfer.mIndex);
			dprintf(DEBUG_CRITICAL, "Direction %d\n", command->do_transfer.mDirection);
			dprintf(DEBUG_CRITICAL, "Do transfer %d\n", command->do_transfer.mDoTransfer);
			dprintf(DEBUG_CRITICAL, "Buffer boundary [0x%x,0x%x), starting at offset 0x%x\n", command->do_transfer.mBufferBegin, command->do_transfer.mBufferEnd, command->do_transfer.mBufferStart);
			
			dprintf(DEBUG_CRITICAL, "Sample Rate %d, bytes per frame %d\n", command->do_transfer.mSampleRate, command->do_transfer.mBytesPerFrame);
			
			bool doTransfer = command->do_transfer.mDoTransfer;
			uint32_t index = command->do_transfer.mIndex;
			if (s_loopback_process)
			{
				set_tap_point(s_loopback_process, index);
			}
			
			if (doTransfer)
			{
				uint8_t *IOBufferBegin = (uint8_t*)mem_static_map_cached(command->do_transfer.mBufferBegin);
				uint8_t *IOBufferEnd = (uint8_t*)mem_static_map_cached(command->do_transfer.mBufferEnd);
				uint8_t *IOBuffer = (uint8_t*)mem_static_map_cached(command->do_transfer.mBufferStart);
				// unload and delete any tap points
				set_debug_tap(s_loopback_process, NULL, NULL);
				if (sInputTap)
				{
					destroy_debug_tap(sInputTap);
					sInputTap = NULL;
				}
				if (sOutputTap)
				{
					destroy_debug_tap(sOutputTap);
					sOutputTap = NULL;
				}
				// create the tap points
			 	if (command->do_transfer.mDirection & 0x1)
				{
					sInputTap = create_debug_tap(IOBufferBegin, IOBufferEnd, IOBuffer);
				}
			 	if (command->do_transfer.mDirection & 0x2)
				{
					sOutputTap = create_debug_tap(IOBufferBegin, IOBufferEnd, IOBuffer);
				}
				// set the tap points
				if (s_loopback_process)
				{
					set_debug_tap(s_loopback_process, sInputTap, sOutputTap);
				}
			}

			command->iopaudiodsp.mStatus = kIOPAUDIODSP_STATUS_SUCCESS;
		}
		break;	
			
		default:
			dprintf(DEBUG_CRITICAL, "@@ ERROR: unrecognised audiodsp opcode 0x%x\n", 
					command->iopaudiodsp.mOpcode);
			command->iopaudiodsp.mStatus = kIOPAUDIODSP_STATUS_PARAM_INVALID;
		break;
	}

	dprintf(DEBUG_SPEW, "@@ done processing audiodsp message with status 0x%08x\n", command->iopaudiodsp.mStatus);

	platform_cache_operation(CACHE_CLEAN, 
				 (void *)command, 
				 sizeof(IOPAUDIODSP_Command));

	qwi_send_item(audiodsp_channel, message);

	dprintf(DEBUG_SPEW, "@@ signaled completion of audiodsp message to host\n");

	return(true);
}

static void
iop_audiodsp_sleep(int mode)
{
}

#if USE_DMA
static void
audiodevice_dma_int_handler_wFloat(void *arg)
{
	sCurrentProcessBufferIndex = (sCurrentProcessBufferIndex+1) % kNumDMABuffers;
	process_data((loopback_process_t)arg, sBuffer[sCurrentProcessBufferIndex]);
}

static void
audiodevice_dma_int_handler(void *arg)
{
#if WITH_VFP && !WITH_VFP_ALWAYS_ON	
	//Enables floating point ops from the ISR
	enter_critical_section();
	arm_call_fpsaved(arg, &audiodevice_dma_int_handler_wFloat);
	exit_critical_section();
#endif	
}

static void
audiodevice_dma_int_err_handler(void *arg)
{
	dprintf(DEBUG_CRITICAL, "audiodevice_dma_int_err_handler!\n");
}
#endif

