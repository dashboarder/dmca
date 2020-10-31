/*
 * Copyright (C) 2009-2011 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */

#ifndef __PLATFORM_MEMMAP_DEFAULTS_H
#define __PLATFORM_MEMMAP_DEFAULTS_H

// Defines for the default load areas
#define DEFAULT_LOAD_ADDRESS		(INSECURE_MEMORY_BASE)
#if SDRAM_LEN >= (1*1024*1024*1024)
# define DEFAULT_LOAD_SIZE		(0x10000000ULL)
#else
# define DEFAULT_LOAD_SIZE		(0x04000000ULL)
#endif
#define DEFAULT_KERNEL_ADDRESS		(DEFAULT_LOAD_ADDRESS + DEFAULT_LOAD_SIZE)
#define DEFAULT_KERNEL_SIZE		(0x03F00000ULL)
#define DEFAULT_DEVICETREE_ADDRESS	(DEFAULT_KERNEL_ADDRESS + DEFAULT_KERNEL_SIZE)
#define DEFAULT_DEVICETREE_SIZE		(0x00100000ULL)
#define DEFAULT_RAMDISK_ADDRESS		(DEFAULT_DEVICETREE_ADDRESS + DEFAULT_DEVICETREE_SIZE)
#define DEFAULT_RAMDISK_SIZE		(DEFAULT_LOAD_SIZE)
#define DEFAULT_FREE_ADDRESS		(DEFAULT_RAMDISK_ADDRESS + DEFAULT_RAMDISK_SIZE)

#endif /* __PLATFORM_MEMMAP_DEFAULTS_H */
