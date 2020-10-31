/*
 * Copyright (C) 2009-2012 Apple Inc. All rights reserved.
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

#include <platform/memmap_defaults.h>

/*
 * 0xa0000000	SDRAM_END
 * 0x9fffc000	PANIC_BASE
 * 0x9ff00000	IBOOT_BASE
 * 0x9f000000	DISPLAY_BASE
 * 0x9e800000	MONITOR_BASE
 */
/* S5L8940X specific memory map */
#define SDRAM_BASE			(0x80000000ULL)
#define SDRAM_BANK_LEN			(0x10000000ULL)
#define SDRAM_BANK_COUNT		(2)
#define SDRAM_END			(SDRAM_BASE + (SDRAM_BANK_LEN * SDRAM_BANK_COUNT))
#define VROM_BASE			(0x3F000000ULL)
#define VROM_BANK_LEN			(0x00100000ULL)
#define VROM_LEN			(0x00010000ULL)
#define SRAM_BASE			(0x34000000ULL)
#define SRAM_BANK_LEN			(0x00100000ULL)
#define SRAM_LEN			(0x00040000ULL)

/* Sleep token is stored at first physical page in kernel memory */
#define SLEEP_TOKEN_BUFFER_BASE		(SDRAM_BASE)

/* reserved area for panic info */
#define PANIC_SIZE		(0x00004000ULL)
#define PANIC_BASE		(SDRAM_END - PANIC_SIZE)

/* reserved area for iBoot */
#define IBOOT_SIZE		(0x00100000 - PANIC_SIZE)
#define IBOOT_BASE		(PANIC_BASE - IBOOT_SIZE)
#define PROTECTED_REGION_SIZE	IBOOT_SIZE

/* reserved area for display */
#ifndef DISPLAY_SIZE
#define DISPLAY_SIZE		(0x00F00000ULL)
#endif
#define DISPLAY_BASE		(IBOOT_BASE - DISPLAY_SIZE)

/* where to load the monitor/hypervisor */
#ifdef HACK_MONITOR
#define MONITOR_SIZE		(0x00800000ULL)
#else  /* !defined(HACK_MONITOR) */
#define MONITOR_SIZE		(0)
#endif /* defined(HACK_MONITOR) */
#define MONITOR_BASE		(DISPLAY_BASE - MONITOR_SIZE)

/* where to stick the mmu tt, default heap and insecure memory*/
#if APPLICATION_SECUREROM
#define MMU_TT_SIZE		(0x4000ULL)
#define MMU_TT_BASE		(SRAM_BASE + SRAM_LEN - MMU_TT_SIZE)
#define MMU_NONCACHE0_SIZE	(SRAM_BANK_LEN)
#define MMU_NONCACHE0_PBASE	(SRAM_BASE)
#define MMU_NONCACHE0_VBASE	(SRAM_BASE + SRAM_BANK_LEN * 3)

#define STACK_SIZE		(0x3000ULL)
#define STACK_BASE		(MMU_TT_BASE - STACK_SIZE)

#define HEAP_END		(STACK_BASE)

#ifndef __ASSEMBLY__

#define INSECURE_MEMORY_BASE	(SRAM_BASE)
#define INSECURE_MEMORY_SIZE	((uintptr_t)__segment_start(__DATA) - INSECURE_MEMORY_BASE)

#endif /* ! __ASSEMBLY__ */

#endif /* APPLICATION_SECUREROM */

#if APPLICATION_IBOOT
#define MMU_TT_SIZE		(0x4000ULL)
#define MMU_TT_BASE		(TEXT_BASE + TEXT_FOOTPRINT - MMU_TT_SIZE)
#define MMU_NONCACHE0_SIZE	(SRAM_BANK_LEN)
#define MMU_NONCACHE0_PBASE	(SRAM_BASE)
#define MMU_NONCACHE0_VBASE	(SRAM_BASE + SRAM_BANK_LEN * 3)
#define MMU_NONCACHE1_SIZE	(0x40000000ULL)
#define MMU_NONCACHE1_PBASE	(0x80000000ULL)
#define MMU_NONCACHE1_VBASE	(0xC0000000ULL)

#if WITH_DFU_MODE
#define STACK_SIZE		(0x3000ULL)

#define INSECURE_MEMORY_SIZE	(0x00200000ULL)
#define INSECURE_MEMORY_BASE	(SDRAM_BASE + SDRAM_BANK_LEN - INSECURE_MEMORY_SIZE)

#define HEAP_EXT_SIZE		(0x00100000ULL)
#define HEAP_EXT_BASE		(INSECURE_MEMORY_BASE - HEAP_EXT_SIZE)

#else /* !WITH_DFU_MODE */
#define STACK_SIZE		(0x1000ULL)

#define INSECURE_MEMORY_BASE	(SDRAM_BASE)
#define INSECURE_MEMORY_SIZE	(DEFAULT_KERNEL_ADDRESS - SDRAM_BASE)

#define SECURE_MEMORY_BASE	(INSECURE_MEMORY_BASE + INSECURE_MEMORY_SIZE)
/* If only 1 bank is populated, this only clears to the alias of
 * DISPLAY_BASE in the first bank. If 2 banks are populated, this
 * clears all the was to DISPLAY_BASE in the second bank. */
#define SECURE_MEMORY_SIZE	(DISPLAY_BASE - SECURE_MEMORY_BASE - SDRAM_BANK_LEN * 2 + platform_get_memory_size())

#define HEAP_EXT_BASE		(DEFAULT_FREE_ADDRESS)
#define HEAP_EXT_SIZE		(MONITOR_BASE - SDRAM_BANK_LEN - HEAP_EXT_BASE)
#endif /* !WITH_DFU_MODE */

#define STACK_BASE		(MMU_TT_BASE - STACK_SIZE)

#define HEAP_END		(STACK_BASE)

#endif /* APPLICATION_IBOOT */

#if APPLICATION_EMBEDDEDIOP
# if ARCH_ARMv7
#  define MMU_TT_SIZE		(0x4000ULL)
# endif

#define AUDIO_SRAM_RESERVE	(0x1000ULL)
#define USB_SRAM_RESERVE	(0x10000ULL)

#if TARGET_S5L8940XAE2
#define HEAP_EXT_BASE		(SRAM_BASE + AUDIO_SRAM_RESERVE)
#define HEAP_EXT_SIZE		(SRAM_LEN - AUDIO_SRAM_RESERVE - USB_SRAM_RESERVE)
#endif /* TARGET_S5L8940XAE2 */


#endif /* !APPLICATION_EMBEDDEDIOP */

#define SCRATCH_BASE     (0x33600000ULL)
#define SCRATCH_LEN      (2048)

#endif /* ! __PLATFORM_MEMMAP_H */
