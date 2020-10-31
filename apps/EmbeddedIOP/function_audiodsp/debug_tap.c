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

#include "debug_tap.h"
#include "timestamper.h"

#include <debug.h>
#include <AssertMacros.h>
#include <platform.h>
#include <platform/timer.h>
#include <sys/task.h>

#include "iop_au_interface.h"


typedef struct
{
	uint8_t *IOBufferBegin;
	uint8_t *IOBufferEnd;
	uint8_t *IOBuffer;
} internal_debug_tap_t;

debug_tap_t create_debug_tap(uint8_t *inIOBegin, uint8_t *inIOEnd, uint8_t *inIO)
{
	internal_debug_tap_t *This = (internal_debug_tap_t*)malloc(sizeof(debug_tap_t));
	if (This)
	{
		This->IOBufferBegin = inIOBegin;
		This->IOBufferEnd = inIOEnd;
		This->IOBuffer = inIO;
	}
	return This;
}

void destroy_debug_tap(debug_tap_t tap)
{
	free(tap);
}

static void clean_cache(void *addr, size_t size)
{
	// the onus is on the caller to make sure the cache is aligned
	void* lineBase = (void*)((uint32_t)addr & ~(CPU_CACHELINE_SIZE-1));
	size_t sizeToDo = ((((uint32_t)addr & (CPU_CACHELINE_SIZE-1)) + size)/CPU_CACHELINE_SIZE + 1) * CPU_CACHELINE_SIZE;
	platform_cache_operation(CACHE_CLEAN, lineBase, sizeToDo);
}

static void invalidate_cache(void *addr, size_t size)
{
	void* lineBase = (void*)((uint32_t)addr & ~(CPU_CACHELINE_SIZE-1));
	size_t sizeToDo = ((((uint32_t)addr & (CPU_CACHELINE_SIZE-1)) + size)/CPU_CACHELINE_SIZE + 1) * CPU_CACHELINE_SIZE;
	platform_cache_operation(CACHE_INVALIDATE, lineBase, sizeToDo);
}

static size_t min(size_t a, size_t b) { return (a < b) ? a : b; }

size_t send_to_tap(debug_tap_t tap, const uint8_t* src, uint32_t sizeInBytes)
{
	internal_debug_tap_t *This = (internal_debug_tap_t*)tap;
	size_t result = 0;

	// don't transfer anything to user if sizeInBytes is larger than IOBuffer:
	if (sizeInBytes > (uint32_t)(This->IOBufferEnd - This->IOBufferBegin))
		return result;

	// make sure we don't overflow the IOBuffer
	size_t dataToSend = min(This->IOBufferEnd - This->IOBuffer, sizeInBytes);
	memcpy(This->IOBuffer, src, dataToSend);
	clean_cache(This->IOBuffer, dataToSend);
	This->IOBuffer += dataToSend;
	src += dataToSend;
	if (This->IOBuffer >= This->IOBufferEnd)
	{
		// if we've wrapped around, send the size of the IOBuffer to the caller
		This->IOBuffer = This->IOBufferBegin;
		result = This->IOBufferEnd - This->IOBufferBegin;
	}

	// now that we've sent the data, how much is left
	dataToSend = sizeInBytes - dataToSend;
	// if there is anything left, send it
	if (dataToSend)
	{
		memcpy(This->IOBuffer, src, dataToSend);
		clean_cache(This->IOBuffer, sizeInBytes);
		This->IOBuffer += dataToSend;
	}
	return result;
}


size_t get_from_tap(debug_tap_t tap, uint8_t* dst, uint32_t sizeInBytes)
{
	internal_debug_tap_t *This = (internal_debug_tap_t*)tap;
	size_t result = 0;

	// don't transfer anything to user if sizeInBytes is larger than IOBuffer:
	if (sizeInBytes > (uint32_t)(This->IOBufferEnd - This->IOBufferBegin))
		return result;

	// make sure we don't overflow the IOBuffer
	size_t dataToGet = min(This->IOBufferEnd - This->IOBuffer, sizeInBytes);
	invalidate_cache(This->IOBuffer, dataToGet);
	memcpy(dst, This->IOBuffer, dataToGet);
	This->IOBuffer += dataToGet;
	dst += dataToGet;
	if (This->IOBuffer >= This->IOBufferEnd)
	{
		// if we've wrapped around, send the size of the IOBuffer to the caller
		This->IOBuffer = This->IOBufferBegin;
		result = This->IOBufferEnd - This->IOBufferBegin;
	}

	// now that we've got the data, how much is left
	dataToGet = sizeInBytes - dataToGet;
	// if there is anything left, get it
	if (dataToGet)
	{
		invalidate_cache(This->IOBuffer, dataToGet);
		memcpy(dst, This->IOBuffer, dataToGet);
		This->IOBuffer += dataToGet;
	}
	return result;
}


