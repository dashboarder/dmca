/*
 * Copyright (C) 2012-2014 Apple Inc. All rights reserved.
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

/* S8000 specific memory map */

///////////////////////////////////////////////////////////////////////////////////
//           0x800000000 + dram_size     -------------------------------
//                                       |        NVMe (12MiB)         |
//                                       |                             |
//                                       |-----------------------------|
//                                       |         TZ0 (varies)        |
//                                       |                             |
//                                       |-----------------------------|
//                                       |         TZ1 (512KiB)        |
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
//            (Elba)       (Maui)        |    on DISPLAY_SIZE)         |
//         0x8F0180000 / 0x870180000 --> |-----------------------------|
//                                       |     iBoot (Stacks)          |
//            (Elba)       (Maui)        |     (STACKS_SIZE)           |
//         0x8F017C000 / 0x87017C000 --> |-----------------------------|
//                                       |     iBoot (Page Tables)     |
//            (Elba)       (Maui)        |     (PAGE_TABLES_SIZE)      |
//         0x8F0100000 / 0x870100000 --> |-----------------------------|
//                                       |     iBoot (Text + Data)     |
//            (Elba)       (Maui)        |     (TEXT_FOOTPRINT)        |
//         0x8F0000000 / 0x870000000 --> |-----------------------------| <-- 0x800000000 + dram_size - 256MB
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
//                                       |    (overlaps with TZ0)      |
//                                       |                             |
//                       0x800000000 --> -------------------------------
//
// Note that Elba A0/B0 instead moves TZ0/TZ1 to the bottom of DRAM. See
// TZ0_BASE_A0 / TZ1_BASE_A0 below for more info
///////////////////////////////////////////////////////////////////////////////////

#define SDRAM_BASE		(0x800000000ULL)
#define SDRAM_BANK_LEN		(SDRAM_LEN)
#define SDRAM_BANK_COUNT	(1)
#define SDRAM_END		(SDRAM_BASE + SDRAM_LEN)

#define VROM_BASE		(0x100000000ULL)
#define VROM_BANK_LEN		(0x02000000ULL)
#define VROM_LEN		(0x00080000ULL)
#define SRAM_BASE		(0x180000000ULL)
#if SUB_PLATFORM_S8000 || SUB_PLATFORM_S8003
  #define VROM_RSVD		(0x00100000ULL)		// 512 KiB reserved for SecureROM testing, remaining 512 KiB for Data and book-keeping
  #define SRAM_LEN		(0x00400000ULL)		// 4 MiB
  #define SRAM_BANK_LEN		(SRAM_LEN)		// Starting with Elba and Cayman, SRAM should be in a 32 MB decode region
#elif SUB_PLATFORM_S8001
  #define VROM_RSVD		(0ULL)
  #define SRAM_LEN		(0x00080000ULL)		// 512 KiB
  #define SRAM_BANK_LEN		(0x02000000ULL)
#endif
#define SRAM_BANK_COUNT		(1)
#define SRAM_END		(SRAM_BASE + SRAM_LEN)

#define IO_BASE			(0x200000000ULL)
#define IO_SIZE			(0x020000000ULL)

#define PCI_REG_BASE		(0x600000000ULL)
#define PCI_REG_LEN		(0x00C000000ULL)
#define PCI_CONFIG_BASE		(0x610000000ULL)
#define PCI_CONFIG_LEN		(0x010000000ULL)
#define PCI_32BIT_BASE		(0x7C0000000ULL)
#define PCI_32BIT_LEN		(0x002000000ULL)		// 32 MiB fits nicely into a 32 MiB L2 MMU block

#define SECUREROM_LOAD_ADDRESS	(VROM_BASE)

/* reserved for ASP/NVMe */
// NOTE ASP_SIZE is now defined by the platform or target makefile.
#define ASP_BASE		(SDRAM_END - ASP_SIZE)

#define CONSISTENT_DEBUG_SIZE	(0x00004000ULL)
#define TZ1_SIZE		(0x00080000ULL)

#if SUB_PLATFORM_S8001
// Elba A0 and B0 both have SEPROM bugs that prevent TZ0/TZ1
// from being located high in DRAM. (<rdar://problem/21748486> Streamline Elba B0 memory map)
//
// So for these chips, we are placing TZ0/TZ1 at the base of DRAM
// before the kernel base.
//
// Actual TZ0/TZ1 reservation happens later

#define CONSISTENT_DEBUG_BASE	(ASP_BASE - CONSISTENT_DEBUG_SIZE)

#else
// Reserved for TZ0/SEP
// TZ0_SIZE is defined by the platform or target makefile
#define TZ0_BASE		(ASP_BASE - TZ0_SIZE)

// Reserved for TZ1/AP Monitor
#define TZ1_BASE		(TZ0_BASE - TZ1_SIZE)

#define CONSISTENT_DEBUG_BASE	(TZ1_BASE - CONSISTENT_DEBUG_SIZE)
#endif

/* reserved area for sleep token info */
#define SLEEP_TOKEN_BUFFER_SIZE	(0x00004000ULL)
#define SLEEP_TOKEN_BUFFER_BASE	(CONSISTENT_DEBUG_BASE - SLEEP_TOKEN_BUFFER_SIZE)

/* reserved area for config sequence region */
#if SUB_PLATFORM_S8000 || SUB_PLATFORM_S8003
#define DRAM_CONFIG_SEQ_SIZE	(0x00004000ULL)
#elif SUB_PLATFORM_S8001
#define DRAM_CONFIG_SEQ_SIZE	(0x00008000ULL)
#endif
#define DRAM_CONFIG_SEQ_BASE	(SLEEP_TOKEN_BUFFER_BASE - DRAM_CONFIG_SEQ_SIZE)

/* reserved area for panic info */
#define PANIC_SIZE		(0x00080000ULL)
#define PANIC_BASE		(DRAM_CONFIG_SEQ_BASE - PANIC_SIZE)

/* reserved area for display */
// DISPLAY_SIZE comes from platform's rules.mk
#define DISPLAY_BASE		(PANIC_BASE - DISPLAY_SIZE)

#if SUB_PLATFORM_S8001
#define TZ0_BASE		(SDRAM_BASE)
#define TZ1_BASE		(TZ0_BASE + TZ0_SIZE)


// For Elba, the kernel starts at 32MB alignment above TZ0/TZ1
#define KERNEL_BASE		(((TZ1_BASE + TZ1_SIZE) + 32*1024*1024 - 1) & ~(32*1024*1024-1))

#else

// For non-Elba systems, KERNEL_BASE simply starts at SDRAM_BASE
#define KERNEL_BASE		(SDRAM_BASE)
#endif

/* where to stick heap and insecure memory */
#if APPLICATION_SECUREROM

#if SUB_PLATFORM_S8000 || SUB_PLATFORM_S8003

#define HEAP_BASE		(SRAM_BASE + VROM_RSVD)
#define HEAP_SIZE               (0x00100000ULL)
#define HEAP_END		(HEAP_BASE + HEAP_SIZE)
// XXX for future platforms, include a guard page after the heap

// DATA_BASE comes from rules.mk
#define DATA_SIZE		(0x00040000ULL)

#define PAGE_TABLES_BASE	(DATA_BASE + DATA_SIZE)
#define PAGE_TABLES_SIZE	(0x00008000ULL)

#define BOOT_TRAMPOLINE_BASE	(DATA_BASE + DATA_SIZE)
#define BOOT_TRAMPOLINE_SIZE	(0x00004000ULL)

#define STACKS_BASE		(PAGE_TABLES_BASE + PAGE_TABLES_SIZE)
#define STACKS_SIZE		(0x00004000ULL)

// XXX for future platforms, this should be increased to allow larger images
//     to be loaded, with a guard between it and the stacks
#define INSECURE_MEMORY_SIZE	(0x00080000ULL)
#define INSECURE_MEMORY_BASE	(SRAM_BASE + SRAM_LEN - INSECURE_MEMORY_SIZE)

#elif SUB_PLATFORM_S8001

// DFU SPACE       0x00000 - 0x40000
// BOOT TRAMPOLINE 0x40000 - 0x44000 (read-execute, acts as write guard between load area and data area)
// DATA            0x44000 - 0x50000
// PAGE TABLES     0x50000 - 0x58000 (not mapped, acts as guard between data area and stacks)
// STACKS          0x58000 - 0x5c000
// HEAP            0x5c000 - 0x80000

#define INSECURE_MEMORY_SIZE	(0x00040000ULL)
#define INSECURE_MEMORY_BASE	(SRAM_BASE)

#define BOOT_TRAMPOLINE_BASE	(INSECURE_MEMORY_BASE + INSECURE_MEMORY_SIZE)
#define BOOT_TRAMPOLINE_SIZE	(0x00004000ULL)

// DATA_BASE comes from rules.mk
#define DATA_SIZE		(0x0000C000ULL)
  
#define PAGE_TABLES_BASE	(DATA_BASE + DATA_SIZE)
#define PAGE_TABLES_SIZE	(0x00008000ULL)

#define STACKS_BASE		(PAGE_TABLES_BASE + PAGE_TABLES_SIZE)
#define STACKS_SIZE		(0x00004000ULL)

#define HEAP_BASE		(STACKS_BASE + STACKS_SIZE)
#define HEAP_SIZE		(SRAM_BASE + SRAM_LEN - HEAP_BASE)
#define HEAP_END		(HEAP_BASE + HEAP_SIZE)

#endif

#endif /* APPLICATION_SECUREROM */

#if APPLICATION_IBOOT

#if WITH_DFU_MODE
/* LLB, iBSS DFU */

/* reserved area for iBoot */
#define IBOOT_SIZE		(0x01400000ULL)
#define IBOOT_BASE		(DISPLAY_BASE - IBOOT_SIZE)

#define INSECURE_MEMORY_SIZE	(IBOOT_SIZE)
#define INSECURE_MEMORY_BASE	(IBOOT_BASE)

#if SUB_PLATFORM_S8000 || SUB_PLATFORM_S8003

// SRAM memory map
//
// ROM Testing		0x000000 - 0x0fffff	(1024 KiB)
// Heap			0x100000 - 0x1fbfff	(1008 KiB)
// Guard		0x1fc000 - 0x1fffff	(  16 KiB)
// Unused		0x200000 - 0x37ffff	(1536 KiB)
// Code/Data		0x380000 - 0x3e7fff	( 416 KiB)
// Trampoline		0x3e8000 - 0x3ebfff	(  16 KiB)
// Page Tables		0x3ec000 - 0x3fbfff	(  64 KiB = 4 pages @ 16 KiB each)
// Stacks		0x3fc000 - 0x3fffff	(  16 KiB)

#define HEAP_BASE		(SRAM_BASE + VROM_RSVD)
#define HEAP_SIZE               (0x00100000ULL - PAGE_SIZE)
#define HEAP_END		(HEAP_BASE + HEAP_SIZE)
#define HEAP_GUARD		(HEAP_END)

#define BOOT_TRAMPOLINE_BASE	(TEXT_BASE + TEXT_FOOTPRINT)
#define BOOT_TRAMPOLINE_SIZE	(PAGE_SIZE)
#define PAGE_TABLES_BASE	(BOOT_TRAMPOLINE_BASE + BOOT_TRAMPOLINE_SIZE)
#define PAGE_TABLES_SIZE	(STACKS_BASE - PAGE_TABLES_BASE)
#define STACKS_BASE		(SRAM_BASE + SRAM_LEN - STACKS_SIZE)
#define STACKS_SIZE		(EXCEPTION_STACK_SIZE + BOOTSTRAP_STACK_SIZE + INTERRUPT_STACK_SIZE)

#elif SUB_PLATFORM_S8001

// Elba (S8001) offsets from SRAM_BASE
// Code/data		0x00000 - 0x4bfff	( 304 KiB)
// Boot trampoline	0x4c000 - 0x4ffff	(  16 KiB)
// Heap			0x50000 - 0x73fff	( 144 KiB)
// Page tables		0x74000 - 0x7Bfff	(  32 KiB)
// Stacks		0x7C000 - 0x7ffff	(  16 KiB)

#define BOOT_TRAMPOLINE_BASE	(SRAM_BASE + TEXT_FOOTPRINT)
#define BOOT_TRAMPOLINE_SIZE	(PAGE_SIZE)

#define HEAP_BASE		(BOOT_TRAMPOLINE_BASE + BOOT_TRAMPOLINE_SIZE)
#define HEAP_SIZE		(0x00024000ULL)
#define HEAP_END		(HEAP_BASE + HEAP_SIZE)

#define PAGE_TABLES_BASE	(HEAP_BASE + HEAP_SIZE)
#define PAGE_TABLES_SIZE	(0x00008000ULL)

#define STACKS_BASE		(PAGE_TABLES_BASE + PAGE_TABLES_SIZE)
#define STACKS_SIZE		(0x00004000ULL)

#endif

#else /* !WITH_DFU_MODE */

/* iBoot, iBEC */

// SDRAM memory map will usually look like the following for S8000/S8003:
//
// iBoot Text		0x870000000 - 0x8700FFFFF	(  1 MiB)
// Page Tables		0x870100000 - 0x870177FFF	(480 KiB)
// Stacks		0x870178000 - 0x87017BFFF	( 16 KiB)
// Boot trampoline	0x87017C000 - 0x87017FFFF	( 16 KiB)
// Heap			0x870180000 - (DISPLAY_BASE - 1 page)
// Guard		(DISPLAY_BASE - 1 page) - DISPLAY_BASE

// And for S8001:
//
// iBoot Text		0x8f0000000 - 0x8f00FFFFF	(  1 MiB)
// Page Tables		0x8f0100000 - 0x8f0177FFF	(480 KiB)
// Stacks		0x8f0178000 - 0x8f017BFFF	( 16 KiB)
// Boot trampoline	0x8f017C000 - 0x8f017FFFF	( 16 KiB)
// Heap			0x8f0180000 - (DISPLAY_BASE - 1 page)
// Guard		(DISPLAY_BASE - 1 page) - DISPLAY_BASE

/* reserved area for iBoot */
#define IBOOT_BASE		(TEXT_BASE)
#define IBOOT_SIZE		(HEAP_END - IBOOT_BASE)

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
