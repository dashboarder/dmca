// Copyright (C) 2014-2015 Apple Inc. All rights reserved.
//
// This document is the property of Apple Inc.
// It is considered confidential and proprietary.
//
// This document may not be reproduced or transmitted in any form,
// in whole or in part, without the express written permission of
// Apple Inc.
//

#ifndef _MIB_DEF_H
#define _MIB_DEF_H

#include <lib/mib.h>
#include <platform/memmap.h>

// Application/Product selection
#if PRODUCT_IBSS
#define MIB_APP_PROD			MIB_APP_PROD_IBSS
#elif PRODUCT_IBEC
#define MIB_APP_PROD			MIB_APP_PROD_IBEC
#elif PRODUCT_IBOOT
#define MIB_APP_PROD			MIB_APP_PROD_IBOOT
#elif PRODUCT_LLB
#define MIB_APP_PROD			MIB_APP_PROD_LLB
#elif APPLICATION_EMBEDDEDIOP
#define MIB_APP_PROD			MIB_APP_PROD_EMBEDDEDIOP
#elif APPLICATION_SECUREROM
#define MIB_APP_PROD			MIB_APP_PROD_SECUREROM
#else
#error "Unknown APPLICATION/PRODUCT"
#endif

// Standard MIB constants for all platforms and targets
MIB_CONSTANT(kMIBPlatformCacheLineSize,		kOIDTypeSize,	CPU_CACHELINE_SIZE);
MIB_CONSTANT(kMIBPlatformSDRAMBaseAddress,	kOIDTypeAddr,	SDRAM_BASE);
MIB_CONSTANT(kMIBPlatformSDRAMSize,		kOIDTypeSize,	(SDRAM_END - SDRAM_BASE));
MIB_CONSTANT(kMIBTargetApplicationProduct,	kOIDTypeUInt32, MIB_APP_PROD);
MIB_CONSTANT(kMIBPlatformPageSize,		kOIDTypeSize,	PAGE_SIZE);

#if WITH_IMAGE4
MIB_CONSTANT(kMIBPlatformImageFormat,		kOIDTypeUInt32, 4);
#elif WITH_IMAGE3
MIB_CONSTANT(kMIBPlatformImageFormat,		kOIDTypeUInt32, 3);
#endif

#ifdef PLATFORM_ENTROPY_RATIO
MIB_CONSTANT(kMIBPlatformEntropyRatio,		kOIDTypeUInt32,	PLATFORM_ENTROPY_RATIO);
#endif

// All of the following depend upon INSECURE_MEMORY_BASE being defined
#ifdef INSECURE_MEMORY_BASE

#ifdef DEFAULT_DEVICETREE_ADDRESS
MIB_CONSTANT(kMIBTargetDefaultDeviceTreeAddress,kOIDTypeAddr,	DEFAULT_DEVICETREE_ADDRESS);
#endif

#ifdef DEFAULT_DEVICETREE_SIZE
MIB_CONSTANT(kMIBTargetDefaultDeviceTreeSize,	kOIDTypeSize,	DEFAULT_DEVICETREE_SIZE);
#endif

#ifdef DEFAULT_FREE_ADDRESS
MIB_CONSTANT(kMIBTargetDefaultFreeAddress,	kOIDTypeAddr,	DEFAULT_FREE_ADDRESS);
#endif

#ifdef DEFAULT_LOAD_ADDRESS
MIB_CONSTANT(kMIBTargetDefaultLoadAddress,	kOIDTypeAddr,	DEFAULT_LOAD_ADDRESS);
#endif

#ifdef DEFAULT_LOAD_SIZE
MIB_CONSTANT(kMIBTargetDefaultLoadSize,		kOIDTypeSize,	DEFAULT_LOAD_SIZE);
#endif

#ifdef DEFAULT_KERNEL_ADDRESS
MIB_CONSTANT(kMIBTargetDefaultKernelAddress,	kOIDTypeAddr,	DEFAULT_KERNEL_ADDRESS);
#endif

#ifdef DEFAULT_KERNEL_SIZE
MIB_CONSTANT(kMIBTargetDefaultKernelSize,	kOIDTypeSize,	DEFAULT_KERNEL_SIZE);
#endif

#ifdef DEFAULT_RAMDISK_ADDRESS
MIB_CONSTANT(kMIBTargetDefaultRamdiskAddress,	kOIDTypeAddr,	DEFAULT_RAMDISK_ADDRESS);
#endif

#ifdef DEFAULT_RAMDISK_SIZE
MIB_CONSTANT(kMIBTargetDefaultRamdiskSize,	kOIDTypeSize,	DEFAULT_RAMDISK_SIZE);
#endif

#ifdef DISPLAY_BASE
MIB_CONSTANT(kMIBTargetDisplayBaseAddress,	kOIDTypeAddr,	DISPLAY_BASE);
#endif

#endif	// INSECURE_MEMORY_BASE

#ifdef PANIC_BASE
MIB_CONSTANT(kMIBTargetPanicBufferAddress,	kOIDTypeAddr,	PANIC_BASE);
#endif

#ifdef PANIC_SIZE
MIB_CONSTANT(kMIBTargetPanicBufferSize,		kOIDTypeSize,	PANIC_SIZE);
#endif

#if (PRODUCT_LLB || APPLICATION_SECUREROM) && WITH_NO_RANDOM_HEAP_COOKIE
MIB_CONSTANT(kMIBTargetRandomHeapCookie, kOIDTypeBoolean, false);
#else
MIB_CONSTANT(kMIBTargetRandomHeapCookie, kOIDTypeBoolean, true);
#endif

// Target-specified MIB weak overrides
#if WITH_HW_ASP
MIB_CONSTANT(kMIBTargetWithHwAsp, kOIDTypeBoolean, true);
#endif

#if WITH_EFFACEABLE
MIB_CONSTANT(kMIBTargetWithEffaceable, kOIDTypeBoolean, true);
#endif

#if WITH_FS
MIB_CONSTANT(kMIBTargetWithFileSystem, kOIDTypeBoolean, true);
#endif

#if WITH_HW_FLASH_NAND
MIB_CONSTANT(kMIBTargetWithHwFlashNand, kOIDTypeBoolean, true);
#endif

#if WITH_HW_FLASH_NOR
MIB_CONSTANT(kMIBTargetWithHwFlashNor, kOIDTypeBoolean, true);
#endif

#if WITH_NVME
MIB_CONSTANT(kMIBTargetWithHwNvme, kOIDTypeBoolean, true);
#endif

#if WITH_LEGACY_PANIC_LOGS
MIB_CONSTANT(kMIBTargetWithLegacyPanicLogs, kOIDTypeBoolean, true);
#endif

// Get rid of intermediate abstractions
#undef MIB_APP_PROD

#endif /* _MIB_DEF_H */

