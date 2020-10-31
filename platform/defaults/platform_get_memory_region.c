/*
 * Copyright (C) 2007-2014 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */
#include <debug.h>
#include <platform.h>
#include <platform/memmap.h>

uintptr_t platform_get_memory_region_base_optional(memory_region_type_t region)
{
	uintptr_t base;

	switch (region) {
		case kMemoryRegion_Panic:
			base = SDRAM_BASE + platform_get_memory_size() - PANIC_SIZE;
			break;

#if defined(DISPLAY_BASE)
		case kMemoryRegion_Display:
			base = SDRAM_BASE + platform_get_memory_size() - platform_get_memory_region_size(kMemoryRegion_Display) - PANIC_SIZE;
			break;
#endif

#if defined(SLEEP_TOKEN_BUFFER_BASE)
		case kMemoryRegion_SleepToken:
			base = SLEEP_TOKEN_BUFFER_BASE;
			break;
#endif

		case kMemoryRegion_Kernel:
			base = SDRAM_BASE;
			break;

		default:
			base = (uintptr_t)-1;
	}

	return base;
}

size_t platform_get_memory_region_size_optional(memory_region_type_t region)
{
	size_t size;

	switch (region) {
		case kMemoryRegion_Panic:
			size = PANIC_SIZE;
			break;

#if defined(DISPLAY_SIZE)
		case kMemoryRegion_Display:
			size = platform_get_display_memory_size();
			ASSERT(size != 0);
			break;
#endif

#if defined(SLEEP_TOKEN_BUFFER_SIZE)
		case kMemoryRegion_SleepToken:
			size = SLEEP_TOKEN_BUFFER_SIZE;
			break;
#endif

		case kMemoryRegion_Kernel:
#if defined(DISPLAY_BASE)
			size = platform_get_memory_region_base(kMemoryRegion_Display)- platform_get_memory_region_base(kMemoryRegion_Kernel);
#else
			size = platform_get_memory_region_base(kMemoryRegion_Panic) - platform_get_memory_region_base(kMemoryRegion_Kernel);
#endif
			break;

		default:
			size = (size_t)-1;
	}

	return size;
}
