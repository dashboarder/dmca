/*
 * Copyright (C) 2011-2014 Apple Inc. All rights reserved.
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

/* S5L8960X specific memory map */

///////////////////////////////////////////////////////////////////////////////////
//           0x800000000 + dram_size     -------------------------------
//                                       |        ASP (8MiB)           |
//                                       |-----------------------------|
//                                       |         TZ1 (1MiB)          |
//                                       -------------------------------
//                                       |   Consistent Debug (4KiB)   |
//                                       |-----------------------------|
//                                       |  Sleep Token Buffer (4KiB)  |
//                                       |-----------------------------|
//                                       |        Panic (512KiB)       |
//                                       |-----------------------------|
//                                       |        Display              |
//                                       |   (Framebuffer + scratch)   |
//                                       |                             |
//                                       |                             |
//                                       |-----------------------------|
//                                       |                             |
//                                       |     iBoot (Heap)            |
//                                       |  (Variable size depending   |
//                                       |    on DISPLAY_SIZE)         |
//                       0x830180000 --> |-----------------------------|
//                                       |     iBoot (Stacks)          |
//                                       |     (STACKS_SIZE)           |
//                       0x83017C000 --> |-----------------------------|
//                                       |     iBoot (Page Tables)     |
//                                       |     (PAGE_TABLES_SIZE)      |
//                       0x830100000 --> |-----------------------------|
//                                       |     iBoot (Text + Data)     |
//                                       |     (TEXT_FOOTPRINT)        |
//                       0x830000000 --> |-----------------------------| <-- 0x800000000 + dram_size - 256MB
//                                       |                             |
//                                       |          Unused             |
//                                       |                             |
//                                       |-----------------------------|
//                                       |                             |
//                                       |         (Ramdisk)           |
//                                       |    (Kernel + DeviceTree)    |
//                                       |       Secure memory         |
//                                       |                             |
//                                       |-----------------------------|
//                                       |                             |
//                                       |        Load region /        |
//                                       |     Insecure Memory         |
//                                       |    (overlaps with TZ0)      |    |-----------------------------|
//                                       |                             |    |          TZ0 (varies)       |
//                       0x800000000 --> -------------------------------    -------------------------------

///////////////////////////////////////////////////////////////////////////////////

#define SDRAM_BASE		(0x800000000ULL)
#define SDRAM_BANK_LEN		(SDRAM_LEN)
#define SDRAM_BANK_COUNT	(1)
#define SDRAM_END		(SDRAM_BASE + SDRAM_LEN)

// We'll map the DRAM uncached at the end of the cached mapping
#define SDRAM_BASE_UNCACHED	(SDRAM_END)

#define VROM_BASE		(0x100000000ULL)
#define VROM_BANK_LEN		(0x00100000ULL)
#define VROM_LEN		(0x00080000ULL)
#define VROM_RSVD		(0x00100000ULL)		// 512 KiB reserved for SecureROM testing, remaining 512 KiB for Data and book-keeping
#define SRAM_BASE		(0x180000000ULL)
#define SRAM_LEN		(0x000400000ULL)	// 4 MiB
#define SRAM_BANK_LEN		(SRAM_LEN)
#define SRAM_BANK_COUNT		(1)
#define SRAM_END		(SRAM_BASE + SRAM_LEN)

#define IO_BASE			(0x200000000ULL)
#define IO_SIZE			(0x10000000ULL)

#define SECUREROM_LOAD_ADDRESS	(VROM_BASE)

/* reserved for ASP */
// NOTE ASP_SIZE is now defined by the platform or target makefile.
#define ASP_BASE		(SDRAM_END - (ASP_SIZE))

// Reserved for TZ1/AP Monitor
#define TZ1_SIZE		(0x00100000ULL)
#define TZ1_BASE		(ASP_BASE - TZ1_SIZE)

#define CONSISTENT_DEBUG_SIZE	(0x00004000ULL)
#define CONSISTENT_DEBUG_BASE	(TZ1_BASE - CONSISTENT_DEBUG_SIZE)

/* reserved area for sleep token info */
#define SLEEP_TOKEN_BUFFER_SIZE	(0x00001000ULL)
#define SLEEP_TOKEN_BUFFER_BASE	(CONSISTENT_DEBUG_BASE - SLEEP_TOKEN_BUFFER_SIZE)

/* reserved area for panic info */
#define PANIC_SIZE		(0x00080000ULL)
#define PANIC_BASE		(SLEEP_TOKEN_BUFFER_BASE - PANIC_SIZE)

/* reserved area for display */
// DISPLAY_SIZE comes from platform's rules.mk
#define DISPLAY_BASE		(PANIC_BASE - DISPLAY_SIZE)

// Reserved for TZ0/SEP
// TZ0_SIZE is defined by the platform or target makefile
#define TZ0_BASE		(SDRAM_BASE)

/* where to stick heap and insecure memory */
#if APPLICATION_SECUREROM

#define DATA_SIZE		(0x00040000ULL)

#define BOOK_KEEPING_BASE	(DATA_BASE + DATA_SIZE)
#define BOOK_KEEPING_SIZE	(0x00020000ULL)

#define BOOT_TRAMPOLINE_BASE	(BOOK_KEEPING_BASE + BOOK_KEEPING_SIZE)
#define BOOT_TRAMPOLINE_SIZE	(0x1000ULL)

#define HEAP_BASE		(SRAM_BASE + VROM_RSVD)
#if !SUPPORT_FPGA
#define HEAP_SIZE               (0x00100000ULL)
#else
#define HEAP_SIZE		(0x00080000ULL)
#endif
#define HEAP_END		(HEAP_BASE + HEAP_SIZE)

#define INSECURE_MEMORY_SIZE	(0x00080000ULL)
#define INSECURE_MEMORY_BASE	(SRAM_BASE + SRAM_LEN - INSECURE_MEMORY_SIZE)

#endif /* APPLICATION_SECUREROM */

#if APPLICATION_IBOOT

#if WITH_DFU_MODE
/* LLB, iBSS DFU */

/* reserved area for iBoot */
#define IBOOT_SIZE		(0x01400000ULL)
#define IBOOT_BASE		(DISPLAY_BASE - IBOOT_SIZE)

#define INSECURE_MEMORY_SIZE	(IBOOT_SIZE)
#define INSECURE_MEMORY_BASE	(IBOOT_BASE)

// SRAM memory map will usually look like the following:
//
// ROM Testing		0x000000 - 0x100000	(1024 KiB)
// Heap			0x100000 - 0x1FEFFF	(1020 KiB)
// Guard		0x1FF000 - 0x1FFFFF	(   4 KiB)
// Unused		0x200000 - 0x37FFFF	(1536 KiB)
// Code/Data		0x380000 - 0x3DFFFF	( 384 KiB)
// Page Tables		0x3E0000 - 0x3FBFFF	( 112 KiB)
// Stacks		0x3FC000 - 0x3FFFFF	(  16 KiB)

#define HEAP_BASE		(SRAM_BASE + VROM_RSVD)
#if !SUPPORT_FPGA
#define HEAP_SIZE               (0x00100000ULL - PAGE_SIZE)
#else
#define HEAP_SIZE		(0x00080000ULL - PAGE_SIZE)
#endif
#define HEAP_END		(HEAP_BASE + HEAP_SIZE)
#define HEAP_GUARD		(HEAP_END)

#define PAGE_TABLES_BASE	(TEXT_BASE + TEXT_FOOTPRINT)
#define PAGE_TABLES_SIZE	(STACKS_BASE - PAGE_TABLES_BASE)
#define STACKS_BASE		(SRAM_BASE + SRAM_LEN - STACKS_SIZE)
#define STACKS_SIZE		(EXCEPTION_STACK_SIZE + BOOTSTRAP_STACK_SIZE + INTERRUPT_STACK_SIZE)

#else /* !WITH_DFU_MODE */

/* iBoot, iBEC */

// SDRAM memory map will usually look like the following:
//
// iBoot Text		0x830000000 - 0x8300FFFFF	(  1 MiB)
// Page Tables		0x830100000 - 0x83017AFFF	(492 KiB)
// Stacks		0x83017B000 - 0x83017EFFF	( 16 KiB)
// Boot trampoline	0x83017F000 - 0x83017FFFF	(  4 KiB)
// Heap			0x830180000 - (DISPLAY_BASE - 1 page)
// Guard		(DISPLAY_BASE - 1 page) - DISPLAY_BASE

/* reserved area for iBoot */
#define IBOOT_BASE		(TEXT_BASE)
#define IBOOT_SIZE		(HEAP_END - IBOOT_BASE)

#define PAGE_TABLES_BASE	(IBOOT_BASE + TEXT_FOOTPRINT)
#define PAGE_TABLES_SIZE	(0x0007B000ULL)
#define STACKS_BASE		(PAGE_TABLES_BASE + PAGE_TABLES_SIZE)
#define STACKS_SIZE		(EXCEPTION_STACK_SIZE + BOOTSTRAP_STACK_SIZE + INTERRUPT_STACK_SIZE)
#define BOOT_TRAMPOLINE_BASE	(STACKS_BASE + STACKS_SIZE)
#define BOOT_TRAMPOLINE_SIZE	(PAGE_SIZE)

#define HEAP_BASE		(BOOT_TRAMPOLINE_BASE + BOOT_TRAMPOLINE_SIZE)
#define HEAP_SIZE		(DISPLAY_BASE - HEAP_BASE - PAGE_SIZE)
#define HEAP_END		(HEAP_BASE + HEAP_SIZE)
#define HEAP_GUARD		(HEAP_END)
#if (HEAP_GUARD != (DISPLAY_BASE - PAGE_SIZE))
#error "Memory map configuration error"
#endif

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
