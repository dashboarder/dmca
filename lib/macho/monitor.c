/*
 * Copyright (C) 2007-2013 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */

#include <arch.h>
#include <debug.h>
#include <lib/image.h>
#include <lib/macho.h>
#include <lib/env.h>
#include <platform.h>
#include <platform/memmap.h>
#if WITH_HW_MIU
#include <platform/soc/miu.h>
#endif
#include <sys.h>
#include <sys/menu.h>

#include "boot.h"

addr_t	gMonitorEntry;
addr_t	gMonitorArgs;

/*
 * Only applicable if the platform memory map has allocations for 
 * a monitor.
 */
#if WITH_MONITOR

static int loaded_monitor(addr_t secure_addr, size_t secure_size);

/*
 * This expects a known macho image of size 'size' is placed at address 'addr'.
 * This is helper function to single monitor + kernel load image, where monitor macho 
 * is placed at the end of compressed kernelcache, and entire object is wrapped into
 * img3/4 format.
 */
int 
load_monitor_image(addr_t addr, size_t size)
{
	if (size > platform_get_memory_region_size(kMemoryRegion_Monitor)) {
		printf("monitor too large\n");
		return -2;
	}

	return (loaded_monitor(addr, size));
}

static int 
loaded_monitor(addr_t secure_addr, size_t secure_size)
{
	monitor_boot_args	*mba;
	addr_t			virtualBase;
	addr_t			virtualEnd;
	addr_t			monitor_base;
	addr_t			monitor_size;
  
	if (!macho_valid(secure_addr))
		return -6;
  
	monitor_base = platform_get_memory_region_base(kMemoryRegion_Monitor);
	monitor_size = platform_get_memory_region_size(kMemoryRegion_Monitor);

	if (!macho_load(secure_addr, secure_size, monitor_base, &virtualBase, &virtualEnd, &gMonitorEntry, 0))
		return -7;

	dprintf(DEBUG_INFO, "monitor loaded at virt: %p, phys: %p, entry: %p\n",
		(void *)virtualBase, (void *)monitor_base, (void *)gMonitorEntry);

	gMonitorEntry -= virtualBase;
	gMonitorEntry += monitor_base;

	mba = (monitor_boot_args *)(monitor_base + monitor_size - sizeof(*mba));
	mba->version  = 2;
	mba->virtBase = virtualBase;
	mba->physBase = monitor_base;
	mba->memSize  = monitor_size;
	gMonitorArgs = (addr_t)mba;

	return 0;
}

#endif	 /* WITH_MONITOR */
