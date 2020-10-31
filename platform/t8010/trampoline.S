/*
 * Copyright (C) 2012-2015 Apple, Inc. All rights reserved.
 *
 * This document is the property of Apple, Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple, Inc.
 */

#include <platform/memmap.h>
#include <platform/trampoline.h>
#include <platform/soc/hwregbase.h>
#include <arch/arm64/proc_reg.h>

// Clears from X0 to X1 (not including X1).
// Assumes that X0 and X1 are both 64-byte aligned as this
// will normally be used to clear a whole page.
.macro CLEAR_X0_TO_X1
1:
	stp	xzr, xzr, [x0], #16
	stp	xzr, xzr, [x0], #16
	stp	xzr, xzr, [x0], #16
	stp	xzr, xzr, [x0], #16
	cmp	x0, x1
	b.lo	1b
.endmacro

// Clears from X0 to X1 (not including X1) using cacheline clear
// operations.
// Assumes that X0 and X1 are both cacheline aligned as this
// will normally be used to clear a whole page.
.macro CLEAR_X0_TO_X1_CACHE_ON
1:
	dc	zva, x0			// zero cacheline
	add	x0, x0, #L1_CACHELINE_SIZE
	cmp	x0, x1
	b.lo	1b
.endmacro

	.text
	.balign	16
	.globl _boot_handoff_trampoline, _boot_handoff_trampoline_end
_boot_handoff_trampoline:

	// Disable external aborts, interrupts
	msr	DAIFSet, #(0xf)

	// Save jump address in x29, and next stage boot info into x28.
	mov	x29, x0
	mov	x28, x1

	// Start by clearing the heap.
	ldr	x0, L_heap_base
	ldr	x1, L_heap_end
	CLEAR_X0_TO_X1_CACHE_ON

	// Clear the stacks area.
	ldr	x0, L_stacks_base
	ldr	x1, L_stacks_end
	CLEAR_X0_TO_X1_CACHE_ON

	// Retire all data accesses prior to this
	// <rdar://problem/19862668> H9: L2 as RAM request RO and DO alias result in prb hang
	dsb	sy
	// Retire all instructions executed prior to this (may include ROM).
	isb	sy

	// Disable caches, mmu, stack alignment fault
	mov	x3, #0
	msr	SCTLR_EL1, x3
	dsb	sy
	isb	sy

	// Flush the TLBs
	tlbi	vmalle1
	dsb	sy
	isb	sy

	// Now running EL1 with MMU off. All accesses are device.

	// Clear the page tables.
	ldr	x0, L_page_tables_base
	ldr	x1, L_page_tables_end
	CLEAR_X0_TO_X1

	// Clear the text area (or data area in ROM)
	// This can only be done if the trampoline was copied to a safe location
	ldr	x0, L_text_base
	ldr	x1, L_text_end
	CLEAR_X0_TO_X1

	// Clear all the pointers to interesting memory areas
	// that were included in the trampoline
	adr	x0, L_pointers_base
	adr	x1, L_pointers_end
	CLEAR_X0_TO_X1

#if WITH_ROM_TRAMPOLINE
	// Disable R/W access to ROM region
	ldr	x1, L_rom_trampoline_reg
	ldr	w2, L_rom_trampoline_val
	ldr	w3, [x1]
	orr	w3, w3, w2
	str	w3, [x1]
	dsb	sy
	ldr	w3, [x1]
	tst	w3, w2			// Verify ROM access is disabled
	b.eq	_trampoline_spin	// Oops, not disabled

	// ROM doesn't pass any info to next stage (LLB)
	mov	x28, #0

	// Clear remap if enabled to boot this ROM
	ldr	x1, L_rom_remap_reg
	str	wzr, [x1]
#endif // WITH_ROM_TRAMPOLINE

	// Reset CPU state
	mov	x1, #0
	mov	x2, #0
	mov	x3, #0
	mov	x4, #0
	mov	x5, #0
	mov	x6, #0
	mov	x7, #0
	mov	x8, #0
	mov	x9, #0
	mov	x10, #0
	mov	x11, #0
	mov	x12, #0
	mov	x13, #0
	mov	x14, #0
	mov	x15, #0
	mov	x16, #0
	mov	x17, #0
	mov	x18, #0
	mov	x19, #0
	mov	x20, #0
	mov	x21, #0
	mov	x22, #0
	mov	x23, #0
	mov	x24, #0
	mov	x25, #0
	mov	x26, #0
	mov	x27, #0
	mov	x30, x29	// lr = next boot stage PC
	mov	x0, x28		// x0 = next boot stage info
	mov	x28, #0
	mov	x29, #0
	msr	TTBR0_EL1, x1
	msr	VBAR_EL1, x1
	msr	ELR_EL1, x1
	msr	SPSR_EL1, x1
	msr	SPSel, #0
	mov	sp, x1		// SP_EL0
	msr	SPSel, #1
	mov	sp, x1		// SP_EL1

	// Invalidate I-cache
	ic      iallu
	dsb	sy
	isb	sy

	ret

#if WITH_ROM_TRAMPOLINE
	// Not really able to panic at this point, so let's just spin.
_trampoline_spin:
	wfe
	b	_trampoline_spin

	.balign	8
L_rom_remap_reg:
	.8byte	REMAP_REG
L_rom_trampoline_reg:
	.8byte	SECURITY_REG
L_rom_trampoline_val:
	.4byte	ROM_READ_DISABLE
#endif // WITH_ROM_TRAMPOLINE

	.balign 64
L_pointers_base:
#if WITH_ROM_TRAMPOLINE
	// in ROM we don't erase TEXT, but we can erase DATA
L_text_base:
	.8byte	DATA_BASE
L_text_end:
	.8byte	DATA_BASE + DATA_SIZE
#else
	// In non-ROM we erase TEXT_FOOTPRINT, which includes both TEXT and DATA
L_text_base:
	.8byte	TEXT_BASE
L_text_end:
	.8byte	TEXT_BASE + TEXT_FOOTPRINT
#endif // WITH_ROM_TRAMPOLINE
L_heap_base:
	.8byte	HEAP_BASE
L_heap_end:
	.8byte	HEAP_BASE + HEAP_SIZE
L_stacks_base:
	.8byte  STACKS_BASE
L_stacks_end:
	.8byte  STACKS_BASE + STACKS_SIZE
L_page_tables_base:
	.8byte  PAGE_TABLES_BASE
L_page_tables_end:
	.8byte  PAGE_TABLES_BASE + PAGE_TABLES_SIZE
	.balign	64
L_pointers_end:

_boot_handoff_trampoline_end:

	.balign	16
