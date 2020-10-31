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

#include "loopback_process.h"
#include "timestamper.h"
#include "debug_tap.h"

#include <debug.h>
#include <AssertMacros.h>
#include <platform.h>
#include <platform/timer.h>
#include <sys/task.h>

typedef struct
{
	uint32_t mNumFrames;
	uint32_t mNumChannels;
	uint32_t mSampleSize;
	uint32_t mChannelBitmask;
	// the scratch buffer is used by the AudioUnit.  We assume it needs
	// 8.24 Mono (channel 0) data
	audio_unit_t mAudioUnit;

	// for debug: which index to send to user
	uint32_t mIndex;
	debug_tap_t mInputTap;
	debug_tap_t mOutputTap;

} internal_loopback_process_t;


// XXX channel bitmask currently not implemented.  Assuming audio data is packed.

loopback_process_t create_loopback_process(audio_unit_t au, uint32_t frames_to_process, uint32_t channels_to_process, uint32_t sample_size, uint32_t channel_bitmask)
{
	internal_loopback_process_t *This = (internal_loopback_process_t*)malloc(sizeof(internal_loopback_process_t));
	if (This)
	{
		This->mNumFrames = frames_to_process;
		This->mNumChannels = channels_to_process;
		This->mSampleSize = sample_size;
		This->mChannelBitmask = channel_bitmask;
		This->mAudioUnit = au;
		This->mIndex = 0;
		This->mInputTap = NULL;
		This->mOutputTap = NULL;
	}
	return This;
}

void destroy_loopback_process(loopback_process_t loopback)
{
	internal_loopback_process_t *This = (internal_loopback_process_t*)loopback;
	free(This);
}

void set_debug_tap(loopback_process_t loopback, debug_tap_t inputTap, debug_tap_t outputTap)
{
	internal_loopback_process_t *This = (internal_loopback_process_t *)loopback;
	// should we turn ints off for this?
	This->mInputTap = inputTap;
	This->mOutputTap = outputTap;
}

void set_tap_point(loopback_process_t loopback, uint32_t index)
{
	internal_loopback_process_t *This = (internal_loopback_process_t *)loopback;
	// should we turn ints off for this?
	This->mIndex = index;
}

void process_data(loopback_process_t loopback, void *processBuffer)
{
	internal_loopback_process_t *This = (internal_loopback_process_t *)loopback;

	uint32_t sizeOfInput = This->mNumFrames * This->mNumChannels * This->mSampleSize;
	uint32_t amountToSend = 0;
	
	// DebugTap1
	if (This->mInputTap && (This->mIndex == 1))
	{
		size_t this_amountToSend = send_to_tap(This->mInputTap, (uint8_t*)processBuffer, sizeOfInput);
		if (this_amountToSend) amountToSend = this_amountToSend;
	}
	
	// DebugInput1
	if (This->mOutputTap && (This->mIndex == 1))
	{
		size_t this_amountToSend = get_from_tap(This->mOutputTap, (uint8_t*)processBuffer, sizeOfInput);
		if (this_amountToSend) amountToSend = this_amountToSend;
	}
	
	AudioUnit_Process(This->mAudioUnit, processBuffer, processBuffer, This->mNumFrames);
	
	// DebugTap3
	if (This->mInputTap && (This->mIndex == 3))
	{
		size_t this_amountToSend = send_to_tap(This->mInputTap, (uint8_t*)processBuffer, sizeOfInput);
		if (this_amountToSend) amountToSend = this_amountToSend;
	}
	
	// DebugInput1
	if (This->mOutputTap && (This->mIndex == 3))
	{
		size_t this_amountToSend = get_from_tap(This->mOutputTap, (uint8_t*)processBuffer, sizeOfInput);
		if (this_amountToSend) amountToSend = this_amountToSend;
	}
	
	if (amountToSend)
	{
		// Should this be done at the beginning?
		uint64_t timestamp = timer_get_ticks();
		send_timestamp(This->mNumFrames, timestamp);
	}
}

