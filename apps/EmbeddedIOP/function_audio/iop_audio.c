/*
 * Copyright (C) 2009-2010 Apple Inc. All rights reserved.
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

#include <sys/task.h>

#include <iop.h>
#include <qwi.h>
#include <EmbeddedIOPProtocol.h>
#include <arch.h>

#include "iop_audio_protocol.h"
#include "AudioCodecA5.h"
#include "clock_stepping.h"

#ifdef DO_PROFILING

// doing peak heap check requires heap.c to support it
//#define DO_PEAKHEAPCHECK
// doing peak heap check requires task.c to support it
//#define DO_PEAKSTACKCHECK

#include <platform/timer.h>
#include <arch/arm/arm.h>

#define kPerformanceSize   16
struct PerformanceDetails
{
	bool valid;
	utime_t time;
	uint32_t cycles;
};

struct PerformanceDetails performanceDetails[kPerformanceSize];

void PerformanceSnapShotReset()
{
	memset(performanceDetails, 0, sizeof(performanceDetails));
}

void PerformanceSnapShotStart(uint32_t which)
{
	if (which >= kPerformanceSize)
		return;
	performanceDetails[which].time = system_time();
	performanceDetails[which].cycles = arm_read_pmreg(ARM_PMCCNTR);
}

void PerformanceSnapShotEnd(uint32_t which)
{
	if (which >= kPerformanceSize)
		return;
	performanceDetails[which].valid = true;
	performanceDetails[which].time = system_time() - performanceDetails[which].time;
	performanceDetails[which].cycles = arm_read_pmreg(ARM_PMCCNTR) - performanceDetails[which].cycles;
}

void SetupProfiling(IOPAUDIO_METRICS* metrics)
{
	// mapping from enum to event monitors from the A5 TRM
	static const uint32_t eventtable[ACLKCycles-ICacheAccesses] = { 0x14, 0x1, 0x4, 0x3, 0x6, 0x7, 0xF, 0x8, 0x12, 0x10 };

	// reset our user defined performance counters.
	PerformanceSnapShotReset();

	// select wich events we are going to monitor
	uint32_t evt0 = 0x03; // by default we measure dcache fills
	uint32_t evt1 = 0x04; // by default we measure dcache accesses
	uint32_t i = ICacheAccesses;
	// choose the event monitor to follow.  We only choose the first two we find
	for (;i < ACLKCycles; ++i)
	{
		if (metrics->mMetricsItemsValid & (1 << i))
		{
			evt0 = eventtable[(i - ICacheAccesses)];
			break;
		}
	}
	++i;
	for (;i < ACLKCycles; ++i)
	{
		if (metrics->mMetricsItemsValid & (1 << i))
		{
			evt1 = eventtable[(i - ICacheAccesses)];
			break;
		}
	}
#ifdef DO_PEAKSTACKCHECK
	if (metrics->mMetricsItemsValid & (1<<PeakStackUsage))
		reset_stack_usage("audio", 1024);
#endif // DO_PEAKSTACKCHECK
#ifdef DO_PEAKHEAPCHECK
	if (metrics->mMetricsItemsValid & (1<<PeakHeapUsage))
		heap_reset_peak_mem();
#endif // DO_PEAKHEAPCHECK

	arm_write_pmreg(ARM_PMOVSR, ~0);	/* clear overflow status */
	arm_write_pmreg(ARM_PMSELR, 0);		/* set event 0... */
	arm_write_pmreg(ARM_PMXEVTYPER, evt0);
	arm_write_pmreg(ARM_PMSELR, 1);		/* set event 1... */
	arm_write_pmreg(ARM_PMXEVTYPER, evt1);
	arm_write_pmreg(ARM_PMCNTENSET, (1<<31) | (1<<1) | (1<<0));	/* enable cycles and events 0 and 1 */
	arm_write_pmreg(ARM_PMCR, 2);		/* clear counters */
	arm_write_pmreg(ARM_PMCR, 1);		/* enable all enabled counters */
	metrics->mMetricsFields[ProcessTimeUSec] = system_time();
	metrics->mMetricsFields[ProcessCycles] = arm_read_pmreg(ARM_PMCCNTR);
}

void EndProfiling(IOPAUDIO_METRICS* metrics)
{
	uint32_t evt0, evt1, cycles;
	arm_write_pmreg(ARM_PMCNTENCLR, ~0);	/* disable all counters */
	utime_t timeEnd = system_time();
	arm_write_pmreg(ARM_PMSELR, 0);
	evt0 = arm_read_pmreg(ARM_PMXEVCNTR); /* read event 0 */
	arm_write_pmreg(ARM_PMSELR, 1);
	evt1 = arm_read_pmreg(ARM_PMXEVCNTR); /* read event 1 */
	cycles = arm_read_pmreg(ARM_PMCCNTR); /* read cycles */

	if (arm_read_pmreg(ARM_PMOVSR))
		dprintf(DEBUG_CRITICAL, "*** counter overflow ***\n");

	uint32_t i = ICacheAccesses;
	// choose the event monitor to follow.  We only choose the first two we find
	for (;i < ACLKCycles; ++i)
	{
		if (metrics->mMetricsItemsValid & (1 << i))
		{
			metrics->mMetricsFields[i] = evt0;
			break;
		}
	}
	++i;
	for (;i < ACLKCycles; ++i)
	{
		if (metrics->mMetricsItemsValid & (1 << i))
		{
			metrics->mMetricsFields[i] = evt1;
			break;
		}
	}

	if (metrics->mMetricsItemsValid & (1<<ProcessTimeUSec))
		metrics->mMetricsFields[ProcessTimeUSec] = timeEnd - metrics->mMetricsFields[ProcessTimeUSec];
	if (metrics->mMetricsItemsValid & (1<<ProcessCycles))
		metrics->mMetricsFields[ProcessCycles] = cycles - metrics->mMetricsFields[ProcessCycles];
	if (metrics->mMetricsItemsValid & (1<<HeapInUsage))
		metrics->mMetricsFields[HeapInUsage] = heap_get_free_mem();
#ifdef DO_PEAKHEAPCHECK
	if (metrics->mMetricsItemsValid & (1<<PeakHeapUsage))
		metrics->mMetricsFields[PeakHeapUsage] = heap_get_peak_mem();
#endif // DO_PEAKHEAPCHECK
#ifdef DO_PEAKSTACKCHECK
	if (metrics->mMetricsItemsValid & (1<<PeakStackUsage))
	 	metrics->mMetricsFields[PeakStackUsage] = get_stack_usage("audio");
#endif // DO_PEAKSTACKCHECK

	for (uint32_t i = 0; i < kPerformanceSize; ++i)
	{
		if ((ReservedStart+i) >= MetricsFieldsSize)
			break;
		if (performanceDetails[i].valid)
		{
			metrics->mMetricsFields[ReservedStart+i] = performanceDetails[i].time;
			metrics->mMetricsItemsValid |= 1<<(ReservedStart+i);
		}
	}
}

#else

void PerformanceSnapShotStart(uint32_t which) {}
void PerformanceSnapShotEnd(uint32_t which) {}
void SetupProfiling(IOPAUDIO_METRICS* metrics) {}
void EndProfiling(IOPAUDIO_METRICS* metrics) { metrics->mMetricsItemsValid = 0; }

#endif

static bool audio_message_process(void);

static int iop_audio_task(void *cfg);
static void iop_audio_sleep(int mode);

IOP_FUNCTION(audio, iop_audio_task, 64*1024, AUDIO_CONTROL_CHANNEL);
IOP_SLEEP_HOOK(audio, iop_audio_sleep);

static int audio_channel;

#define kMaxCodecs 8
static AudioCodec sTokens[kMaxCodecs] = { NULL, };

// return true if registered, false if could not.
static bool
Register(AudioCodec audioCodec)
{
	for (size_t i = 0; i < kMaxCodecs; ++i)
		if (!sTokens[i])
		{
			sTokens[i] = audioCodec;
			return true;
		}
	return false;
}

static void
Unregister(AudioCodec audioCodec)
{
	for (size_t i = 0; i < kMaxCodecs; ++i)
		if (sTokens[i] == audioCodec)
			sTokens[i] = NULL;
}

static bool
isAudioCodecValid(AudioCodec audioCodec)
{
	for (size_t i = 0; i < kMaxCodecs; ++i)
		if (sTokens[i] == audioCodec)
			return true;
	return false;
}

static int
iop_audio_task(void *cfg)
{
	struct iop_channel_config *channel = (struct iop_channel_config *)cfg;
	struct task_event* audio_message_event = (struct task_event*) malloc(sizeof(struct task_event));

	dprintf(DEBUG_SPEW, "@@ Audio task starting\n");

	check(kIOPAUDIO_COMMAND_SIZE == sizeof(IOPAUDIO_Command));

	/* establish the host communications channel */
	event_init(audio_message_event, EVENT_FLAG_AUTO_UNSIGNAL, false);
	dprintf(DEBUG_SPEW, "** opening audio channel\n");
	audio_channel = qwi_instantiate_channel("audio command",
						QWI_ROLE_CONSUMER,
						channel->ring_size,
						(void *)mem_static_map_cached(channel->ring_base),
						(qwi_channel_hook)event_signal,
						audio_message_event);

#if WITH_VFP && !WITH_VFP_ALWAYS_ON
	/* Little doubt we'll need VFP/Neon */
	arch_task_fp_enable(true);
#endif
	
	for (;;) {
		dprintf(DEBUG_SPEW, "@@ waiting for message on audio channel\n");
		while (audio_message_process()) {
			// eat all available messages
		}		
		event_wait(audio_message_event);
	}

	return(0);
}

static bool
audio_message_process(void)
{
	uint32_t           message;
	IOPAUDIO_Command*  command;

	dprintf(DEBUG_SPEW, "@@ handling host message\n");

	/* look to see if there's an item waiting for us */
	if (qwi_receive_item(audio_channel, &message) == -1)
		return(false);
	
	// set the clocks high
	SetClockState(kClockRequestMessageProcess, kClockValueHigh);

	dprintf(DEBUG_SPEW, "@@ received audio message\n");

	/* find the command structure based on the message */
	command = (IOPAUDIO_Command*)mem_static_map_cached(message);

	/*
	 * Flush any cached item contents we might have lying around - we are guaranteed
	 * that the command size is a multiple of our cacheline size.
	 */
	platform_cache_operation(CACHE_INVALIDATE,
				 (void *)command, 
				 sizeof(*command));

	/* 
	 * TODO: make this part of the API and push this
	 * architecture-specific command handling down into the s5l8920x
	 * platform directory.
	 */
	switch (command->iopaudio.mOpcode) {

	case kIOPAUDIO_OPCODE_CREATE:
	{
		AudioComponent comp = NULL;
		AudioComponentDescription desc;
		desc.componentType = command->create.mComponentDesc.mComponentType;
		desc.componentSubType = command->create.mComponentDesc.mComponentSubType;
		desc.componentManufacturer = command->create.mComponentDesc.mComponentManufacturer;
		desc.componentFlags = command->create.mComponentDesc.mComponentFlags;
		desc.componentFlagsMask = command->create.mComponentDesc.mComponentFlagsMask;
		comp = AudioComponentFindNext(comp, &desc);
		if(comp == NULL)
		{
			dprintf(DEBUG_CRITICAL, "Could not find AudioComponent\n");
			command->iopaudio.mStatus = kIOPAUDIO_STATUS_CODECERROR;
			break;
		}
		dprintf(DEBUG_CRITICAL, "found component\n");
		AudioCodec audioCodec = NULL;
		size_t err = AudioComponentInstanceNew(comp, &audioCodec);
		command->iopaudio.Codec.mCodecStatus = err;
		if (err != 0 || audioCodec == NULL)
		{
			dprintf(DEBUG_CRITICAL, "Could not create audioCodec\n");
			command->iopaudio.mStatus = kIOPAUDIO_STATUS_CODECERROR;
			break;
		}

		const void *magicCookie = (command->create.mAdditionalParametersSizeBytes) ? command->create.mAdditionalParameters : 0;
		
		err = AudioCodecInitialize(audioCodec, &(command->create.mInputFormat), &(command->create.mOutputFormat), magicCookie, command->create.mAdditionalParametersSizeBytes);
		command->iopaudio.Codec.mCodecStatus = err;
		if (err != 0)
		{
			dprintf(DEBUG_CRITICAL, "Could not initialize audioCodec\n");
			AudioComponentInstanceDispose(audioCodec);
			command->iopaudio.mStatus = kIOPAUDIO_STATUS_CODECERROR;
			break;
		}
		if (!Register(audioCodec))
		{
			dprintf(DEBUG_CRITICAL, "Could not register audioCodec\n");
			AudioComponentInstanceDispose(audioCodec);
			command->iopaudio.mStatus = kIOPAUDIO_STATUS_CODECERROR;
			break;
		}
		command->iopaudio.mStatus = kIOPAUDIO_STATUS_SUCCESS;
		command->iopaudio.Codec.mIOPToken = (uint32_t)audioCodec;
		dprintf(DEBUG_CRITICAL, "Created codec with id %x\n", command->iopaudio.Codec.mIOPToken);
	}
		break;
	case kIOPAUDIO_OPCODE_DESTROY:
	{
		AudioCodec audioCodec = (AudioCodec)command->iopaudio.Codec.mIOPToken;
		if (!isAudioCodecValid(audioCodec))
		{
			dprintf(DEBUG_CRITICAL, "Codec not valid\n");
			command->iopaudio.mStatus = kIOPAUDIO_STATUS_PARAM_INVALID;
			break;
		}
		AudioComponentInstanceDispose(audioCodec);
		Unregister(audioCodec);
		command->iopaudio.mStatus = kIOPAUDIO_STATUS_SUCCESS;
	}
		break;
	case kIOPAUDIO_OPCODE_RESET:
	{
		AudioCodec audioCodec = (AudioCodec)command->iopaudio.Codec.mIOPToken;
		if (!isAudioCodecValid(audioCodec))
		{
			dprintf(DEBUG_CRITICAL, "Codec not valid\n");
			command->iopaudio.mStatus = kIOPAUDIO_STATUS_PARAM_INVALID;
			break;
		}
		size_t err = AudioCodecReset(audioCodec);
		command->iopaudio.Codec.mCodecStatus = err;
		command->iopaudio.mStatus = kIOPAUDIO_STATUS_SUCCESS;
	}
		break;
	case kIOPAUDIO_OPCODE_GETPROPINFO:
	{
		AudioCodec audioCodec = (AudioCodec)command->iopaudio.Codec.mIOPToken;
		if (isAudioCodecValid(audioCodec))
		{
			dprintf(DEBUG_CRITICAL, "Codec not valid\n");
			command->iopaudio.mStatus = kIOPAUDIO_STATUS_PARAM_INVALID;
			break;
		}
		size_t err = AudioCodecGetPropertyInfo(audioCodec, command->get_propinfo.mPropertyID, (size_t*)&command->get_propinfo.mPropertySize, (bool*)&command->get_propinfo.mPropertyWritable);
		command->iopaudio.Codec.mCodecStatus = err;
		command->iopaudio.mStatus = kIOPAUDIO_STATUS_SUCCESS;
	}
		break;
	case kIOPAUDIO_OPCODE_GETPROPERTY:
	{
		AudioCodec audioCodec = (AudioCodec)command->iopaudio.Codec.mIOPToken;
		if (isAudioCodecValid(audioCodec))
		{
			dprintf(DEBUG_CRITICAL, "Codec not valid\n");
			command->iopaudio.mStatus = kIOPAUDIO_STATUS_PARAM_INVALID;
			break;
		}
		// <rdar://problem/8466788> update getProperty and setProperty to limit property address
		size_t max_size_for_property = sizeof(*command) - ((uint32_t)&command->get_property.mPropertyData - (uint32_t)command);
		if (max_size_for_property < command->get_property.mPropertySizeBytes)
		{
			dprintf(DEBUG_CRITICAL, "Parameters not valid\n");
			command->iopaudio.mStatus = kIOPAUDIO_STATUS_PARAM_INVALID;
			break;
		}

		// the message should be cleaned before and after we use it, so do not need to clean ourselves
		void* property = (void*)command->get_property.mPropertyData;

		size_t err = AudioCodecGetProperty(audioCodec, command->get_property.mPropertyID, (size_t*)&command->get_property.mPropertySizeBytes, property);
		command->iopaudio.Codec.mCodecStatus = err;
		command->iopaudio.mStatus = kIOPAUDIO_STATUS_SUCCESS;
	}
		break;
	case kIOPAUDIO_OPCODE_SETPROPERTY:
	{
		AudioCodec audioCodec = (AudioCodec)command->iopaudio.Codec.mIOPToken;
		if (isAudioCodecValid(audioCodec))
		{
			dprintf(DEBUG_CRITICAL, "Codec not valid\n");
			command->iopaudio.mStatus = kIOPAUDIO_STATUS_PARAM_INVALID;
			break;
		}
		// <rdar://problem/8466788> update getProperty and setProperty to limit property address
		size_t max_size_for_property = sizeof(*command) - ((uint32_t)&command->get_property.mPropertyData - (uint32_t)command);
		if (max_size_for_property < command->get_property.mPropertySizeBytes)
		{
			dprintf(DEBUG_CRITICAL, "Parameters not valid\n");
			command->iopaudio.mStatus = kIOPAUDIO_STATUS_PARAM_INVALID;
			break;
		}

		// the message should be cleaned before and after we use it, so do not need to clean ourselves
		void* property = (void*)command->get_property.mPropertyData;

		size_t err = AudioCodecSetProperty(audioCodec, command->set_property.mPropertyID, command->set_property.mPropertySizeBytes, property);
		command->iopaudio.Codec.mCodecStatus = err;
		command->iopaudio.mStatus = kIOPAUDIO_STATUS_SUCCESS;
	}
		break;
	case kIOPAUDIO_OPCODE_PROCESSFRAME:
	{
		AudioCodec audioCodec = (AudioCodec)command->iopaudio.Codec.mIOPToken;
		if (!isAudioCodecValid(audioCodec))
		{
			dprintf(DEBUG_CRITICAL, "Codec not valid\n");
			command->iopaudio.mStatus = kIOPAUDIO_STATUS_PARAM_INVALID;
			break;
		}
		// <rdar://problem/8486723> function_audio should make sure parameters to processSingleFrame (input, output buffers) are valid
		// make sure the parameters are not-null.
		if (!command->process_frame.mSrcAddr
			|| !command->process_frame.mSrcSizeBytes
			|| !command->process_frame.mDstAddr
			|| !command->process_frame.mDstSizeBytes)
		{
			dprintf(DEBUG_CRITICAL, "Parameters not valid\n");
			command->iopaudio.mStatus = kIOPAUDIO_STATUS_PARAM_INVALID;
			break;
		}
		const void* src = (const void*)mem_static_map_cached(command->process_frame.mSrcAddr);
		void* dst = (void*)mem_static_map_cached(command->process_frame.mDstAddr);
		
		
		// the onus is on the caller to make sure the cache is aligned
		void* lineBase = (void*)((uint32_t)src & ~(CPU_CACHELINE_SIZE-1));
		size_t sizeToClean = ((((uint32_t)src & (CPU_CACHELINE_SIZE-1)) + command->process_frame.mSrcSizeBytes)/CPU_CACHELINE_SIZE + 1) * CPU_CACHELINE_SIZE;
		platform_cache_operation(CACHE_CLEAN | CACHE_INVALIDATE, lineBase, sizeToClean);
		
		SetupProfiling(&command->process_frame.mMetrics);
		
		void* packet_dst = dst;
		uint16_t numPackets = command->process_frame.mNumPackets;
		size_t remaining_dst_size = command->process_frame.mDstSizeBytes;
		size_t total_dst_size = 0;
		size_t total_src_size = 0;
		
		//sanity check, if the multi frame fields are not used
		if(0 == numPackets)
		{
			numPackets = 1;
			command->process_frame.mPacketSize[0] = command->process_frame.mSrcSizeBytes;			
		}	
		
		for(uint16_t i = 0; i < numPackets; ++i)
		{
			size_t src_size = command->process_frame.mPacketSize[i];
			//make sure we dont go pass all of the valid input data
			if(total_src_size + src_size > command->process_frame.mSrcSizeBytes)
				break;			
			size_t packet_dst_size = remaining_dst_size; //the current mp3 decoder doesn't take dst size that's bigger than the packet output size
			dprintf(DEBUG_SPEW, "IOP_Audio start %d of %d, input %p size %zu, output %p, size %zu\n", i+1, numPackets, src, src_size, packet_dst, packet_dst_size);
			size_t err = AudioCodecProcessSinglePacket(audioCodec, src, &src_size, packet_dst, &packet_dst_size);			
			command->iopaudio.Codec.mCodecStatus = err;
			if(err) 
				break;
			packet_dst += packet_dst_size;
			remaining_dst_size -= packet_dst_size;
			total_dst_size += packet_dst_size;
			src += src_size;			
			total_src_size += src_size;
			
			//for encoding, write the size back to the msg
			command->process_frame.mPacketSize[i] = packet_dst_size;
		}
		command->process_frame.mSrcSizeBytes = total_src_size;
		command->process_frame.mDstSizeBytes = total_dst_size;
		

		//dprintf(DEBUG_CRITICAL, "Decoded/Encoded Total SrcSize %d, dstSize %d\n", command->process_frame.mSrcSizeBytes, command->process_frame.mDstSizeBytes);
		EndProfiling(&command->process_frame.mMetrics);
		
		// the onus is on the caller to make sure the cache is aligned
		lineBase = (void*)((uint32_t)dst & ~(CPU_CACHELINE_SIZE-1));
		sizeToClean = ((((uint32_t)dst & (CPU_CACHELINE_SIZE-1)) + command->process_frame.mDstSizeBytes)/CPU_CACHELINE_SIZE + 1) * CPU_CACHELINE_SIZE;
		platform_cache_operation(CACHE_CLEAN, lineBase, sizeToClean);
		
		command->iopaudio.mStatus = kIOPAUDIO_STATUS_SUCCESS;
	}		
		break;
	default:
		dprintf(DEBUG_CRITICAL, "@@ ERROR: unrecognised audio opcode 0x%x\n", 
			command->iopaudio.mOpcode);
		command->iopaudio.mStatus = kIOPAUDIO_STATUS_PARAM_INVALID;
		break;
	}

	dprintf(DEBUG_SPEW, "@@ done processing audio message with status 0x%08x\n", command->iopaudio.mStatus);

	platform_cache_operation(CACHE_CLEAN, 
				 (void *)command, 
				 sizeof(IOPAUDIO_Command));

	qwi_send_item(audio_channel, message);

	dprintf(DEBUG_SPEW, "@@ signaled completion of audio message to host\n");

	// set the clocks low
	SetClockState(kClockRequestMessageProcess, kClockValueLow);

	return(true);
}

static void
iop_audio_sleep(int mode)
{
}
