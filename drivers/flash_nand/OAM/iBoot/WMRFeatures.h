/*
 * Copyright (c) 2010 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */
 
/////
//
// Whimory Feature Definitions
//
/////


#include "WMRTypes.h"

#ifndef _WMR_FEATURES_H_
#define _WMR_FEATURES_H_

// Define the platform we're on
#define WMR_BUILDING_IBOOT (1)
// Allow yaFTL to open
#define AND_SUPPORT_YAFTL   (1)
// Allow FTL to open
#define AND_SUPPORT_LEGACY_FTL (0)
#define AND_SUPPORT_LEGACY_VFL (0)
#define PPN_ENABLE_RMA_MODE (1)

#define AND_MAX_SPECIAL_BLOCKS (120)

//#define WMR_FIL_TEST_ENABLED    (1)

#if (defined(WMR_DEBUG) && WMR_DEBUG) &&        \
    (defined(PRODUCT_IBOOT) && PRODUCT_IBOOT) &&    \
    (defined(WMR_FIL_TEST_ENABLED) && WMR_FIL_TEST_ENABLED)
// Allow writes on H2 and H3 DEBUG-ONLY iBoot builds to enable the FIL interface test
#define AND_COLLECT_STATISTICS (1)
#else
// Disallow write and erase operations to block device as a bootloader
#define AND_READONLY (1)
#endif
#ifndef ENABLE_L2V_TREE
#define ENABLE_L2V_TREE (0)
#endif

// Don't bother collect stats in read-only mode
//#define AND_COLLECT_STATISTICS			(1)
// Use polling (instead of interrupts)
#define BUILDING_SINGLETASKING (1)

//Should we use line-wise or full-cache operations
#if (defined(PLATFORM_S5L8720X) && PLATFORM_S5L8720X)
// M2 products shipped with full-cache flushing
#define WMR_USE_FULL_CACHE_FLUSH (1)
#endif

#if (defined(WITH_NAND_BOOT) && WITH_NAND_BOOT)
// Boot-from-nand products use boot block
#define AND_SUPPORT_FIL_BOOT_PAGES          (1)
#else
// Don't write a boot block, because this platform won't use it
#define AND_SUPPORT_FIL_BOOT_PAGES          (0)
#endif

// Create feature code for NVRAM support.
#if ((defined(WITH_NAND_NVRAM) && WITH_NAND_NVRAM) || (defined(WITH_EFFACEABLE_NAND) && WITH_EFFACEABLE_NAND))
#define AND_SUPPORT_NVRAM (1)
#else
#define AND_SUPPORT_NVRAM (0)
#endif

// Create feature code for firmware area support.
#if (defined(WITH_NAND_FIRMWARE) && WITH_NAND_FIRMWARE)
#define AND_SUPPORT_FW_AREA (1)
#else
#define AND_SUPPORT_FW_AREA (0)
#endif


// Allow FIL layer to be built without support for block device.
#if (defined(WITH_NAND_FILESYSTEM) && WITH_NAND_FILESYSTEM)
#define AND_SUPPORT_BLOCK_STORAGE (1)
#else
#define AND_SUPPORT_BLOCK_STORAGE (0)
#endif

// Build just enough to support boot-from-nand but not full block storage support.
#if (!AND_SUPPORT_BLOCK_STORAGE && WITH_NAND_BOOT)
#define AND_FPART_ONLY (1)
#else
#define AND_FPART_ONLY (0)
#endif

// Sanity check
#if (defined (AND_SUPPORT_2ND_BOOTLOADER) && AND_SUPPORT_2ND_BOOTLOADER && defined (AND_SIGNATURE_IN_BLOCK_ZERO) && AND_SIGNATURE_IN_BLOCK_ZERO)
#error "AND_SUPPORT_2ND_BOOTLOADER & AND_SIGNATURE_IN_BLOCK_ZERO can't both be defined"
#endif

// Defined by platform, but they're all OK with 30 for now
#define FIL_MAX_ECC_CORRECTION              (30)

#if (!defined(AND_READONLY) && WMR_BUILDING_IBOOT)
//#error iBoot nand driver does not support writes
#endif

// =============================================================================
// configurable preprocessor compilation control
// =============================================================================

// Set H2FMI_DEBUG below to 1 if you want to build for debugging (default
// to 0).
#define WMR_IBOOT_WRITE_ENABLE 0
#define H2FMI_DEBUG  0

// Set H2FMI_TEST_HOOK below to 1 if you want to insert tests at the end
// of FIL_Init in iBoot (default to 0).
#define H2FMI_TEST_HOOK  0

// Set H2FMI_WAIT_USING_ISR to true if you want operations to wait for
// dma and bus events by hooking an interrupt service routine to the
// FMI interrupt vector; set to false for waiting using register
// polling with yield (default to true).
#if !(defined(AND_READONLY)) || WITH_NAND_NVRAM || WITH_NAND_FIRMWARE || WITH_EFFACEABLE_NAND
// Disable ISR for more efficient deadstripping
#define H2FMI_WAIT_USING_ISR 1
#endif

// Change the constant below to 1 if you want to see all FMI
// subsystem register writes; note that doing so is likely to change
// timing and therefore behavior, so it should be a rare case that you
// would ever want to do this.  In cases where you want to see just
// one or two writes, consider calling the 'h2fmi_dbg_wr' or
// 'h2fmi_dbg_wr8' function directly (default to 0).
#define H2FMI_TRACE_REG_WRITES  0

// Change the constant below to 1 if you want to see all FMI
// subsystem register reads; note that doing so is likely to change
// timing and therefore behavior, so it should be a rare case that you
// would ever want to do this.  In cases where you want to see just
// one or two reads, consider calling the 'h2fmi_dbg_rd' or
// 'h2fmi_dbg_rd8' function directly (default to 0).
#define H2FMI_TRACE_REG_READS  0

// =============================================================================
// fixed preprocessor compilation control
// =============================================================================

#if (defined (WITH_HW_BOOTROM_NAND) && WITH_HW_BOOTROM_NAND)
#define H2FMI_BOOTROM 1
#else
#define H2FMI_BOOTROM 0
#endif

#define H2FMI_IOP 0
#define H2FMI_IBOOT 1
#define H2FMI_EFI 0

// Always build SecureROM read-only.
#if (H2FMI_BOOTROM)
#define H2FMI_READONLY 1
// Otherwise, ignore nand driver read-only configuration in iBoot
// debug builds so that erase/write are available for use in testing
// read operations.
#elif (H2FMI_IBOOT && defined(AND_READONLY) && H2FMI_DEBUG)
#define H2FMI_READONLY 0
// Otherwise, mirror nand driver configuration in iBoot.
#elif (defined(AND_READONLY))
#define H2FMI_READONLY 1
#else
#define H2FMI_READONLY 0
#endif

#define DEFAULT_OAM_BOOT_BUFFER_SIZE (0x800000) // 8MB

#define H2FMI_ALLOW_MULTIS (1)

#endif /* _WMR_FEATURES_H_ */

