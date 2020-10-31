/*
 * Copyright (C) 2010-2011, 2014 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */
#include <debug.h>
#include <drivers/sha1.h>
#include <lib/mib.h>
#include <platform.h>

static int random_get_bytes_internal(uint8_t *buffer, size_t length, uint32_t entropy_ratio, void *entropy_buffer, size_t entropy_buffer_size);

int random_get_bytes(uint8_t *buffer, size_t length)
{
	uint32_t entropy_ratio = mib_get_u32(kMIBPlatformEntropyRatio);
	return random_get_bytes_internal(buffer, length, entropy_ratio, NULL, 0);
}

// Called when randomness is needed but the heap is not available to store
// the entropy pool which is instead stored in a caller-provided buffer.
int random_get_bytes_noheap(uint8_t *buffer, size_t length, void *entropy_buffer, size_t entropy_buffer_size)
{
	uint32_t entropy_ratio = mib_get_u32(kMIBPlatformEntropyRatio);
	return random_get_bytes_internal(buffer, length, entropy_ratio, entropy_buffer, entropy_buffer_size);
}

#if DEBUG_BUILD

int random_get_bytes_debug(uint8_t *buffer, size_t length, uint32_t entropy_ratio)
{
	if (entropy_ratio == 0) {
		entropy_ratio = mib_get_u32(kMIBPlatformEntropyRatio);
	}

	return random_get_bytes_internal(buffer, length, entropy_ratio, NULL, 0);
}

#endif

static uint32_t random_count;
static uint8_t  random_pool[CCSHA1_OUTPUT_SIZE];

static int random_get_bytes_internal(uint8_t *buffer, size_t length, uint32_t entropy_ratio, void *entropy_buffer, size_t entropy_buffer_size)
{
	uint32_t cnt, chunk, entropy_pool_size;
	uint32_t *entropy_pool = NULL;

	entropy_pool_size = entropy_ratio * CCSHA1_OUTPUT_SIZE;

	RELEASE_ASSERT(entropy_buffer == NULL || entropy_pool_size <= entropy_buffer_size);

	while (length > 0) {
		// Make random data by calculating a sha-1 hash of entropy data
		if (random_count == 0) {
			// lazy intialize: we might need to go to the entropy well
			// more than once for a big request, or not at all for
			// small requests we already have whitened randomness for
			if (entropy_pool == NULL) {
				if (entropy_buffer == NULL)
					entropy_pool = malloc(entropy_pool_size);
				else
					entropy_pool = entropy_buffer;
			}

			for (cnt = 0; cnt < (entropy_pool_size / sizeof(uint32_t)); cnt++) {
				entropy_pool[cnt] = platform_get_entropy();
			}

			sha1_calculate(entropy_pool, entropy_pool_size, random_pool);

			random_count = CCSHA1_OUTPUT_SIZE;
		}

		chunk = (length >= random_count) ? random_count : length;
		memcpy(buffer, random_pool - random_count + CCSHA1_OUTPUT_SIZE, chunk);

		random_count -= chunk;

		buffer += chunk;
		length -=chunk;
	}

	if (entropy_pool != NULL) {
		if (entropy_buffer == NULL) {
			free(entropy_pool);
		} else {
			memset(entropy_buffer, 0, entropy_pool_size);
		}
	}

	return 0;
}
