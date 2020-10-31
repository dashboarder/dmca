/*
 * Copyright (C) 2012-2015 Apple Inc. All rights reserved.
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

// <rdar://problem/18127631>Remove memory map hack in s8000's memmap.h
//#include <platform/memmap_defaults.h>
#define DEFAULT_LOAD_ADDRESS		(INSECURE_MEMORY_BASE)
#define DEFAULT_LOAD_SIZE		(0x08000000ULL) // 128 MB instead of 64 MB in memmap_defaults.h
#define DEFAULT_KERNEL_ADDRESS		(DEFAULT_LOAD_ADDRESS + DEFAULT_LOAD_SIZE)
#define DEFAULT_KERNEL_SIZE		(0x03F00000ULL)
#define DEFAULT_DEVICETREE_ADDRESS	(DEFAULT_KERNEL_ADDRESS + DEFAULT_KERNEL_SIZE)
#define DEFAULT_DEVICETREE_SIZE		(0x00100000ULL)
#define DEFAULT_RAMDISK_ADDRESS		(DEFAULT_DEVICETREE_ADDRESS + DEFAULT_DEVICETREE_SIZE)
#define DEFAULT_RAMDISK_SIZE		(0x04000000ULL)
#define DEFAULT_FREE_ADDRESS		(DEFAULT_RAMDISK_ADDRESS + DEFAULT_RAMDISK_SIZE)

/* T8010 specific memory map */

///////////////////////////////////////////////////////////////////////////////////
//           0x800000000 + dram_size     -------------------------------
//                                       |        NVMe (12MiB)         |
//                                       |                             |
//                                       |-----------------------------|
//                                       |   Consistent Debug (16KiB)  |
//                                       |-----------------------------|
//                                       |  Sleep Token Buffer (16KiB) |
//                                       |-----------------------------|
//                                       | DRAM Config Sequence (16KiB)|
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
//                         (Cayman)      |    on DISPLAY_SIZE)         |
//                       0x870180000 --> |-----------------------------|
//                                       |     iBoot (Trampoline)      |
//                         (Cayman)      |     (BOOT_TRAMPOLINE_SIZE)  |
//                       0x87017C000 --> |-----------------------------|
//                                       |     iBoot (Stacks)          |
//                         (Cayman)      |     (STACKS_SIZE)           |
//                       0x870178000 --> |-----------------------------|
//                                       |     iBoot (Page Tables)     |
//                         (Cayman)      |     (PAGE_TABLES_SIZE)      |
//                       0x870100000 --> |-----------------------------|
//                                       |     iBoot (Text + Data)     |
//                         (Cayman)      |     (TEXT_FOOTPRINT)        |
//                       0x870000000 --> |-----------------------------| <-- 0x800000000 + dram_size - 256MB
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

#define VROM_BASE		(0x100000000ULL)
#define VROM_BANK_LEN		(0x002000000ULL)
#define VROM_RSVD		(0x00080000ULL)		// 512 KiB reserved for SecureROM testing
#define SRAM_BASE		(0x180000000ULL)
#define SRAM_LEN		(0x000200000ULL)		// 2 MiB
#define SRAM_BANK_LEN		(SRAM_LEN)		// SRAM should be in a 32 MB decode region
#define SRAM_BANK_COUNT		(1)
#define SRAM_END		(SRAM_BASE + SRAM_LEN)

#define IO_BASE			(0x200000000ULL)
#define IO_SIZE			(0x020000000ULL)

#define PCI_REG_BASE		(0x600000000ULL)
#define PCI_REG_LEN		(0x00C000000ULL)
#define PCI_CONFIG_BASE		(0x610000000ULL)
#define PCI_CONFIG_LEN		(0x10000000ULL)
#define PCI_32BIT_BASE		(0x7C0000000ULL)
#define PCI_32BIT_LEN		(0x002000000ULL)	// 32 MiB fits nicely into a 32 MiB L2 MMU block

#define SECUREROM_LOAD_ADDRESS	(VROM_BASE)

/* reserved for ASP/NVMe */
// NOTE ASP_SIZE is now defined by the platform or target makefile.
#define ASP_BASE		(SDRAM_END - ASP_SIZE)

#define CONSISTENT_DEBUG_SIZE	(0x00004000ULL)
#define CONSISTENT_DEBUG_BASE	(ASP_BASE - CONSISTENT_DEBUG_SIZE)

/* reserved area for sleep token info */
#define SLEEP_TOKEN_BUFFER_SIZE	(0x00004000ULL)
#define SLEEP_TOKEN_BUFFER_BASE	(CONSISTENT_DEBUG_BASE - SLEEP_TOKEN_BUFFER_SIZE)

/* reserved area for config sequence region */
#define DRAM_CONFIG_SEQ_SIZE	(0x00004000ULL)
#define DRAM_CONFIG_SEQ_BASE	(SLEEP_TOKEN_BUFFER_BASE - DRAM_CONFIG_SEQ_SIZE)

/* reserved area for panic info */
#define PANIC_SIZE		(0x00080000ULL)
#define PANIC_BASE		(DRAM_CONFIG_SEQ_BASE - PANIC_SIZE)

/* reserved area for display */
// DISPLAY_SIZE comes from platform's rules.mk
#define DISPLAY_BASE		(PANIC_BASE - DISPLAY_SIZE)

// Reserved for TZ0/SEP
// TZ0_SIZE is defined by the platform or target makefile
#define TZ0_BASE		(SDRAM_BASE)

/* where to stick heap and insecure memory */
#if APPLICATION_SECUREROM

// SecureROM SRAM memory map
//
// ROM Testing		0x000000 - 0x07FFFF	( 512 KiB)
// Data			0x080000 - 0x09FFFF	( 128 KiB)
// Page Tables		0x0A0000 - 0x0A7FFF	(  32 KiB)
// Stacks		0x0A8000 - 0x0ABFFF	(  16 KiB)
// Boot Trampoline	0x0AC000 - 0x0AFFFF	(  16 KiB)
// DFU Region		0x0B0000 - 0x1AFFFF	(1024 KiB)
// Heap			0x1B0000 - 0x1FFFFF	( 320 KiB)

// DATA_BASE comes from platform/t8010/rules.mk
#define DATA_SIZE		(0x00020000ULL)

#define PAGE_TABLES_BASE	(DATA_BASE + DATA_SIZE)
#define PAGE_TABLES_SIZE	(0x00008000ULL)

#define STACKS_BASE		(PAGE_TABLES_BASE + PAGE_TABLES_SIZE)
#define STACKS_SIZE		(0x00004000ULL)

#define BOOT_TRAMPOLINE_BASE	(STACKS_BASE + STACKS_SIZE)
#define BOOT_TRAMPOLINE_SIZE	(0x00004000ULL)

#define INSECURE_MEMORY_BASE	(BOOT_TRAMPOLINE_BASE + BOOT_TRAMPOLINE_SIZE)
#define INSECURE_MEMORY_SIZE	(0x00100000ULL)

#define HEAP_BASE		(INSECURE_MEMORY_BASE + INSECURE_MEMORY_SIZE)
#define HEAP_END		(SRAM_END)
#define HEAP_SIZE		(HEAP_END - HEAP_BASE)
#define HEAP_GUARD		(HEAP_END)

#endif /* APPLICATION_SECUREROM */

#if APPLICATION_IBOOT

#if WITH_DFU_MODE

// iBSS/LLB SRAM memory map
//
// ROM Testing		0x000000 - 0x07FFFF	( 512 KiB)
// Page Tables		0x080000 - 0x097FFF	(  96 KiB)
// Stacks		0x098000 - 0x09BFFF	(  16 KiB)
// <unused>		0x09C000 - 0x0AFFFF	(  96 KiB)
// Code/Data		0x0B0000 - 0x1AFFFF	(1024 KiB)
// Boot Trampoline	0x1B0000 - 0x1B3FFF	(  16 KiB)
// Heap			0x1B4000 - 0x1FFFFF	( 304 KiB)

#define PAGE_TABLES_BASE	(SRAM_BASE + VROM_RSVD)
#define PAGE_TABLES_SIZE	(0x0018000ULL)

#define STACKS_BASE		(PAGE_TABLES_BASE + PAGE_TABLES_SIZE)
#define STACKS_SIZE		(EXCEPTION_STACK_SIZE + BOOTSTRAP_STACK_SIZE + INTERRUPT_STACK_SIZE)

#define CODE_DATA_END		(TEXT_BASE + TEXT_FOOTPRINT)

#define BOOT_TRAMPOLINE_BASE	(CODE_DATA_END)
#define BOOT_TRAMPOLINE_SIZE	(PAGE_SIZE)

#define HEAP_BASE		(BOOT_TRAMPOLINE_BASE + BOOT_TRAMPOLINE_SIZE)
#define HEAP_END		(SRAM_END)
#define HEAP_SIZE		(HEAP_END - HEAP_BASE)
#define HEAP_GUARD		(HEAP_END)

// Reserved area for iBoot
#define IBOOT_BASE		(SDRAM_END - SDRAM_LOAD_OFFSET)
#define IBOOT_SIZE		(0x01400000ULL)

#define INSECURE_MEMORY_BASE	(IBOOT_BASE)
#define INSECURE_MEMORY_SIZE	(IBOOT_SIZE)

#else /* !WITH_DFU_MODE */

// iBoot/iBEC SDRAM memory map

// SDRAM memory map will usually look like the following for T8010:
//
// iBoot Text		0x870000000 - 0x8700FFFFF	(  1 MiB)
// Page Tables		0x870100000 - 0x870177FFF	(480 KiB)
// Stacks		0x870178000 - 0x87017BFFF	( 16 KiB)
// Boot trampoline	0x87017C000 - 0x87017FFFF	( 16 KiB)
// Heap			0x870180000 - (DISPLAY_BASE - 1 page)
// Guard		(DISPLAY_BASE - 1 page) - DISPLAY_BASE

/* reserved area for iBoot */
#define IBOOT_BASE		(TEXT_BASE)
#define IBOOT_SIZE		(HEAP_END - IBOOT_BASE)

#define CODE_DATA_END		(IBOOT_BASE + IBOOT_SIZE)

#if (IBOOT_BASE != (SDRAM_END - SDRAM_LOAD_OFFSET))
#error "Memory map configuration error"
#endif

#define PAGE_TABLES_BASE	(IBOOT_BASE + TEXT_FOOTPRINT)
#define PAGE_TABLES_SIZE	(0x00078000ULL)
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
