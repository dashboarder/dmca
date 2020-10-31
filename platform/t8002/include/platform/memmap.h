/*
 * Copyright (C) 2013-2015 Apple Inc. All rights reserved.
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

/* T8002 specific memory map */

///////////////////////////////////////////////////////////////////////////////////
//      0x80000000  + dram_size -->      -------------------------------
//                                       |          TZ0 (varies)       |       
//                                       |-----------------------------|
//                                       |          ASP (5 MiB)        |
//                                       |-----------------------------|
//                                       |   Consistent Debug (16KiB)  |
//                                       |-----------------------------|
//                                       |  Sleep Token Buffer (4KiB)  |
//                                       |-----------------------------|
//                                       | DRAM Config Sequence (16KiB)|
//                                       |-----------------------------|
//                                       |        Panic (512KiB)       |
//                                       |-----------------------------|
//                                       |        Display (16MiB)      |
//                                       |     (Framebuffer + scratch) |
//                                       |                             |
//                                       |                             |
//                                       |                             |
//                                       |-----------------------------|
//                                       |        iBoot (32MiB)        |
//                                       |   (Text + Data + Stacks )   |
//                                       |   (Page Tables + Heap)      |
//                                       |                             |
//                                       |-----------------------------| <-- 0x80000000 + dram_size - 64MB
//                                       |          Unused             |
//                                       |                             |
//                                       |                             |
//                                       |                             |
//                                       |                             |
//                                       |-----------------------------|
//                                       |                             |
//                                       |         (Ramdisk)           |
//                                       |    (Kernel + DeviceTree)    |
//                                       |       Secure memory         |
//                                       |-----------------------------|
//                                       |                             |
//                                       |                             |
//                                       |        Load region /        |   
//                                       |     Insecure Memory         |    
//                                       |    (overlaps with TZ0)      |
//                                       |                             |    |-----------------------|
//                                       |                             |    |     TZ0 (varies       |
//      0x80000000             -->       -------------------------------    -------------------------
///////////////////////////////////////////////////////////////////////////////////

#define SDRAM_BASE		(0x80000000ULL)
#define SDRAM_BANK_LEN		(SDRAM_LEN)
#define SDRAM_BANK_COUNT	(1)
#define SDRAM_END		(SDRAM_BASE + SDRAM_LEN)

#define VROM_BASE		(0x40000000ULL)
#define VROM_BANK_LEN		(0x00100000ULL)
#define VROM_LEN		(0x00040000ULL)

#define SRAM_BASE		(0x48800000ULL)
#define SRAM_LEN		(0x00120000ULL)
#define SRAM_BANK_LEN		(SRAM_LEN)
#define SRAM_BANK_COUNT		(1)
#define SRAM_END		(SRAM_BASE + SRAM_LEN)

#define IO_BASE			(0x40000000ULL)
#define IO_SIZE			(0x20000000ULL)

/* custom ROM is stored at first physical page DRAM, and booted by enabling remap */
#define SECUREROM_LOAD_ADDRESS	(SDRAM_BASE)

// Reserved for TZ0/SEP
// TZ0_SIZE is defined by the platform or target makefile
#define TZ0_BASE		(SDRAM_END - TZ0_SIZE)

/* reserved area for ASP */
#define ASP_SIZE		(0x00500000ULL)
#define ASP_BASE		(TZ0_BASE - ASP_SIZE)

#define CONSISTENT_DEBUG_SIZE	(0x00004000ULL)
#define CONSISTENT_DEBUG_BASE	(ASP_BASE - CONSISTENT_DEBUG_SIZE)

/* reserved area for sleep token info */
#define SLEEP_TOKEN_BUFFER_SIZE	(0x00001000ULL)
#define SLEEP_TOKEN_BUFFER_BASE	(CONSISTENT_DEBUG_BASE - SLEEP_TOKEN_BUFFER_SIZE)

/* reserved area for config sequence region */
#define DRAM_CONFIG_SEQ_SIZE	(0x00004000ULL)
#define DRAM_CONFIG_SEQ_BASE	(SLEEP_TOKEN_BUFFER_BASE - DRAM_CONFIG_SEQ_SIZE)

/* reserved area for panic info */
#define PANIC_SIZE		(0x00080000ULL)
#define PANIC_BASE		(DRAM_CONFIG_SEQ_BASE - PANIC_SIZE)

/* reserved area for display 
 * DISPLAY_SIZE comes from platform's rules.mk
 */
#define DISPLAY_BASE		(PANIC_BASE - DISPLAY_SIZE)


/* where to stick heap and insecure memory */
#if APPLICATION_SECUREROM

// SecureROM SRAM memory map
//
// Data				0x000000 - 0x007000	(  28 KiB)
// Boot Trampoline		0x007000 - 0x008000	(   4 KiB)
// Page Tables			0x008000 - 0x00C000	(  16 KiB)
// Stacks			0x00C000 - 0x010000	(  16 KiB)
// Heaps			0x010000 - 0x018000	(  32 KiB)
// DFU Region			0x018000 - 0x120000	(1056 KiB)	

#define INSECURE_MEMORY_SIZE	(SRAM_END - HEAP_END)
#define INSECURE_MEMORY_BASE	(HEAP_END)

// DATA_BASE comes from rules.mk
#define DATA_SIZE		(0x00007000ULL)

#define BOOK_KEEPING_BASE	(DATA_BASE + DATA_SIZE)
#define   BOOT_TRAMPOLINE_BASE	(BOOK_KEEPING_BASE)
#define   BOOT_TRAMPOLINE_SIZE	(0x00001000ULL)
#define   MMU_TT_BASE		(BOOK_KEEPING_BASE + BOOT_TRAMPOLINE_SIZE)
#define   MMU_TT_SIZE		(0x00004000ULL)
#define   STACK_BASE		(BOOK_KEEPING_BASE + BOOT_TRAMPOLINE_SIZE + MMU_TT_SIZE)
#define   STACK_SIZE		(0x00004000ULL)
#define BOOK_KEEPING_SIZE	(BOOT_TRAMPOLINE_SIZE + STACK_SIZE + MMU_TT_SIZE) 

#define HEAP_BASE		(BOOK_KEEPING_BASE + BOOK_KEEPING_SIZE)
#define HEAP_SIZE		(0x00008000ULL)
#define HEAP_END		(HEAP_BASE + HEAP_SIZE)

#endif /* APPLICATION_SECUREROM */

#if APPLICATION_IBOOT

#if WITH_DFU_MODE
/* LLB, iBSS DFU */

/* reserved area for iBoot (aligned to 1MB boundary) */
#define IBOOT_SIZE		(0x02000000ULL + 0x100000ULL - (PANIC_SIZE + DISPLAY_SIZE))
#define IBOOT_BASE		(DISPLAY_BASE - IBOOT_SIZE)

#define INSECURE_MEMORY_SIZE	(IBOOT_SIZE)
#define INSECURE_MEMORY_BASE	(IBOOT_BASE)

#define   STACK_SIZE		(0x00004000ULL)
#define   MMU_TT_SIZE		(0x00004000ULL)
#define BOOK_KEEPING_SIZE	(STACK_SIZE + MMU_TT_SIZE)
#define BOOK_KEEPING_BASE	(SRAM_BASE)
#define   MMU_TT_BASE		(BOOK_KEEPING_BASE)
#define   STACK_BASE		(BOOK_KEEPING_BASE + MMU_TT_SIZE)

#define HEAP_SIZE		(TEXT_BASE - HEAP_BASE)
#define HEAP_BASE		(BOOK_KEEPING_BASE + BOOK_KEEPING_SIZE)
#define HEAP_END		(HEAP_BASE + HEAP_SIZE)

#else /* !WITH_DFU_MODE */
/* iBoot, iBEC */

#define IBOOT_BASE		(TEXT_BASE)
#define IBOOT_SIZE		(HEAP_END - IBOOT_BASE)

#define BOOK_KEEPING_BASE	(IBOOT_BASE + TEXT_FOOTPRINT)
#define   MMU_TT_BASE		(BOOK_KEEPING_BASE)
#define   MMU_TT_SIZE		(0x00004000ULL)
#define   STACK_BASE		(BOOK_KEEPING_BASE + MMU_TT_SIZE)
#define   STACK_SIZE		(0x00004000ULL)
#define BOOK_KEEPING_SIZE	(STACK_SIZE + MMU_TT_SIZE)

#define HEAP_BASE		(BOOK_KEEPING_BASE + BOOK_KEEPING_SIZE)
#define HEAP_SIZE		(DISPLAY_BASE - HEAP_BASE - PAGE_SIZE)
#define HEAP_END		(HEAP_BASE + HEAP_SIZE)
#define HEAP_GUARD		(HEAP_END)

#define INSECURE_MEMORY_BASE	(SDRAM_BASE)
#define INSECURE_MEMORY_SIZE	(DEFAULT_LOAD_SIZE)

#define SECURE_MEMORY_BASE	(INSECURE_MEMORY_BASE + INSECURE_MEMORY_SIZE)
#define SECURE_MEMORY_SIZE	(DEFAULT_FREE_ADDRESS - SECURE_MEMORY_BASE)

/* unused area */
#define UNUSED_MEMORY_BASE	(SECURE_MEMORY_BASE + SECURE_MEMORY_SIZE)
#define UNUSED_MEMORY_SIZE	(IBOOT_BASE - UNUSED_MEMORY_BASE)

#endif /* !WITH_DFU_MODE */

#endif /* APPLICATION_IBOOT */

#endif /* ! __PLATFORM_MEMMAP_H */
