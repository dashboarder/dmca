/*
 * Copyright (C) 2011-2012 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */
#ifndef __PLATFORM_MEMMAP_H
#define __PLATFORM_MEMMAP_H

/* S5L8747X hardware memory map */
#define SDRAM_BASE			(0x08000000ULL)
#define SDRAM_BANK_LEN			(0x10000000ULL)
#define SDRAM_BANK_COUNT		1
#define SDRAM_END			(SDRAM_BASE + (SDRAM_BANK_LEN * SDRAM_BANK_COUNT))
#define VROM_BASE			(0x20000000ULL)
#define VROM_BANK_LEN			(0x00100000ULL)
#define VROM_LEN			(0x00020000ULL)
#define SRAM_BASE			(0x22000000ULL)
#define SRAM_BANK_LEN			(0x00400000ULL)
#define SRAM_LEN			(0x00020000ULL)

/* Sleep token is stored at first physical page in kernel memory */
#define SLEEP_TOKEN_BUFFER_BASE		(SDRAM_BASE)

/* reserved area for panic info */
#define PANIC_SIZE		(0x00004000ULL)
#define PANIC_BASE		(SDRAM_END - PANIC_SIZE)

/* Purple graphics memory, must be 1 MB aligned */
#define PURPLE_GFX_MEMORY_LEN	(0x09600000 - PANIC_SIZE)

/* reserved area for iBoot */
#define IBOOT_SIZE		(0x00100000 - PANIC_SIZE)
#define IBOOT_BASE		(PANIC_BASE - IBOOT_SIZE)
#define PROTECTED_REGION_SIZE	IBOOT_SIZE

/* where to stick the mmu tt, default heap and insecure memory*/
#if APPLICATION_SECUREROM
#define MMU_TT_SIZE		(0x1000ULL)
#define MMU_TT_BASE		(SRAM_BASE + SRAM_LEN - MMU_TT_SIZE)
#define MMU_NONCACHE0_SIZE	(SRAM_BANK_LEN)
#define MMU_NONCACHE0_PBASE	(SRAM_BASE)
#define MMU_NONCACHE0_VBASE	(SRAM_BASE + SRAM_BANK_LEN)

#define STACK_SIZE		(0x3000ULL)
#define STACK_BASE		(MMU_TT_BASE - STACK_SIZE)

#define HEAP_END		(STACK_BASE)

#ifndef __ASSEMBLY__

#define INSECURE_MEMORY_BASE	(SRAM_BASE)
#define INSECURE_MEMORY_SIZE	((uintptr_t)__segment_start(__DATA) - INSECURE_MEMORY_BASE)

#endif /* ! __ASSEMBLY__ */

#endif /* APPLICATION_SECUREROM */

/* iBoot SDRAM map
0x08000000-0x0B000000 - Load area (48 MB)
0x0B000000-0x0DF00000 - Kernel (47 MB)
0x0DF00000-0x0E000000 - Device tree (1 MB)
0x0E000000-0x11000000 - Ramdisk (48 MB)
0x11000000-0x17F00000 - Heap (111 MB)
0x17F00000-0x17FFC000 - iBoot (unused) (1008 kB)
0x17FFC000-0x18000000 - Panic (16kB)
*/

#if APPLICATION_IBOOT
#define MMU_TT_SIZE		(0x1000ULL)
#define MMU_TT_BASE		(TEXT_BASE + TEXT_FOOTPRINT - MMU_TT_SIZE)
#define MMU_NONCACHE0_SIZE	(SRAM_BANK_LEN)
#define MMU_NONCACHE0_PBASE	(SRAM_BASE)
#define MMU_NONCACHE0_VBASE	(SRAM_BASE + SRAM_BANK_LEN)
#define MMU_NONCACHE1_SIZE	(0x10000000ULL)
#define MMU_NONCACHE1_PBASE	(0x08000000ULL)
#define MMU_NONCACHE1_VBASE	(0x28000000ULL)

#if WITH_DFU_MODE
#define STACK_SIZE		(0x3000ULL)

#define INSECURE_MEMORY_SIZE	(0x00800000ULL)
#define INSECURE_MEMORY_BASE	(SDRAM_BASE + ((u_int32_t)platform_get_memory_size()) - INSECURE_MEMORY_SIZE)

#else /* !WITH_DFU_MODE */
#define STACK_SIZE		(0x0800ULL)

#define INSECURE_MEMORY_BASE		(SDRAM_BASE)
#define INSECURE_MEMORY_SIZE		(DEFAULT_KERNEL_ADDRESS - SDRAM_BASE)
#define DEFAULT_LOAD_ADDRESS		(INSECURE_MEMORY_BASE)
#define DEFAULT_LOAD_SIZE		(0x03000000ULL)
#define DEFAULT_KERNEL_ADDRESS		(DEFAULT_LOAD_ADDRESS + DEFAULT_LOAD_SIZE)
#define DEFAULT_KERNEL_SIZE		(0x02F00000ULL)
#define DEFAULT_DEVICETREE_ADDRESS	(DEFAULT_KERNEL_ADDRESS + DEFAULT_KERNEL_SIZE)
#define DEFAULT_DEVICETREE_SIZE		(0x00100000ULL)
#define DEFAULT_RAMDISK_ADDRESS		(DEFAULT_DEVICETREE_ADDRESS + DEFAULT_DEVICETREE_SIZE)
#define DEFAULT_RAMDISK_SIZE		(0x03000000ULL)
#define DEFAULT_FREE_ADDRESS		(DEFAULT_RAMDISK_ADDRESS + DEFAULT_RAMDISK_SIZE)

#define SECURE_MEMORY_BASE	(INSECURE_MEMORY_BASE + INSECURE_MEMORY_SIZE)
#define SECURE_MEMORY_SIZE	(IBOOT_BASE - SECURE_MEMORY_BASE)

#define HEAP_EXT_BASE		(DEFAULT_FREE_ADDRESS)
#define HEAP_EXT_SIZE		(IBOOT_BASE - HEAP_EXT_BASE)
#endif /* !WITH_DFU_MODE */

#define STACK_BASE		(MMU_TT_BASE - STACK_SIZE)

/* Reserved area for boot trace */
/* WARNING: We're really tight on SRAM on B137. So, the boot trace area overlaps
   the initial stack area. We typically only use about 500 bytes of the initial stack,
   and once we jump into main_task, we never use the initial stack again. So, as long
   as we don't do too much tracing before jumping into main_task, we'll be OK */
#define PROFILE_BUF_SIZE	(0x0800ULL)
#define PROFILE_BUF_BASE	(STACK_BASE)

#define HEAP_END		(STACK_BASE)

#endif /* APPLICATION_IBOOT */

#endif /* ! __PLATFORM_MEMMAP_H */
