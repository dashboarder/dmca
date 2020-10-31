/*
 * Copyright (C) 2012-2014 Apple Computer, Inc. All rights reserved.
 *
 * This document is the property of Apple Computer, Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Computer, Inc.
 */

#include <platform/memmap.h>
#if WITH_ROM_TRAMPOLINE
#include <platform/trampoline.h>
#endif // WITH_ROM_TRAMPOLINE
#include <platform/soc/hwregbase.h>
#include <arch/arm64/proc_reg.h>

// Set 1 to use clean-invalidate by dup tags instead of architected clean-invalidate by set/way
#define WITH_DUP_TAGS_CLEAN_INVALIDATE	0

// Set to 1 to scan the CP dup-tags to see if any non-invalid entries stayed.
#define VERIFY_INVALIDATION		0

#define SCR_RW_SHIFT			10
#define SCR_RW				(1 << SCR_RW_SHIFT)
#define SCR_NS				1
#define SCR_EL1_64_NS			(SCR_RW | SCR_NS)
#define CPSR_EL1			(PSR64_MODE_EL1 | PSR64_MODE_SPX | DAIF_ALL)


// TODO: Remove dup-tags style set/way invalidate when A0 is deprecated.

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
	// Save jump address in x27, and next stage boot info into x28.
	mov	x27, x0
	mov	x28, x1

	// Disable external aborts, interrupts
	msr	DAIFSet, #(0xf)

#if !WITH_CLASSIC_SUSPEND_TO_RAM || !PRODUCT_LLB
	// Start by clearing the heap.
	ldr	x0, L_heap_base
	ldr	x1, L_heap_end
	CLEAR_X0_TO_X1_CACHE_ON

	// Clear the stacks area.
	ldr	x0, L_stacks_base
	ldr	x1, L_stacks_end
	CLEAR_X0_TO_X1_CACHE_ON
#endif // !WITH_SUSPEND_TO_RAM || !PRODUCT_LLB

	// Retire all instructions executed prior to this (may include ROM).
	isb	sy

	// Disable caches, mmu, stack alignment fault
	mov	x1, #0
	msr	SCTLR_EL3, x1
	dsb	sy
	isb	sy

	// Flush the TLBs - radar 18269688
	tlbi	alle3
	dsb	sy
	isb	sy

	// Now running EL3 with MMU off. All accesses are device, secure.

#if !WITH_CLASSIC_SUSPEND_TO_RAM || !PRODUCT_LLB
	// Clear the page tables.
	ldr	x0, L_page_tables_base
	ldr	x1, L_page_tables_end
	CLEAR_X0_TO_X1

#ifdef BOOT_TRAMPOLINE_BASE
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
#endif
#endif

#if WITH_ROM_TRAMPOLINE
	// Disable R/W access to ROM region
	ldr	x1, L_rom_trampoline_reg
	ldr	w2, L_rom_trampoline_val
	ldr	w3, [x1]
	orr	w3, w3, w2
	str	w3, [x1]
	ldr	w3, [x1]
	
	// ROM doesn't pass any info to next stage (LLB)
	mov	x28, #0
#endif // WITH_ROM_TRAMPOLINE

#if !WITH_DUP_TAGS_CLEAN_INVALIDATE

_architected_set_way_clean_invalidate:
	// B0 architected set/way clean-invalidate.
	// Register format for "dc cisw" is:
	// * 32 bit register
	// * Ways in the top bits (31 downwards)
	// * Sets starting at bit log2(line size)
	// * Level specified near the lowest bits.
	// E.g for Cyclone L2 with 64 byte lines, 2K lines, 8 ways:
	// [31:29] = way 0..7
	// [28:17] = SBZ
	//  [16:6] = set 0..2047 (aka "index")
	//   [5:4] = SBZ
	//   [3:1] = level = 3'h1 (L2)
	//     [0] = SBZ
	mov	x0, #(1 << 1)		// x0 = set 0, way 0, level L2
	mov	x1, #1 << L2_CACHELINE_SHIFT
	mov	x2, #1 << (L2_CACHELINE_SHIFT + L2_CACHEINDEX_SHIFT)
	mov	x3, #1 << (32 - L2_CACHEWAY_SHIFT)
	mov	x4, #1 << 32
1:	dc	cisw, x0		// Clean-invalidate with x0
	// Increment set field.
	add	x0, x0, x1		// Increment [16:6]
	tst	x0, x2			// Test for overflow in [17]
	b.eq	1b
	// Carry set field into way field.
	bic	x0, x0, x2		// Clear overflow in [17]
	add	x0, x0, x3		// Increment [31:29]
	tst	x0, x4			// Test for overflow in [32]
	b.eq	1b
	// Carry out of way field ends the loop.
	// Another dummy TLBI to act as sDsb.
	mov	x0, #0
	tlbi	ASIDE1IS, x0
	dsb	sy
	isb	sy

#else

_dup_tags_clean_invalidate:
	// Setup EL1 as AArch64, non-secure
	mov	x1, #SCR_EL1_64_NS
	msr	SCR_EL3, x1
	// Disable EL1 MMU.
	mov	x1, #SCTLR_RESERVED & 0xffff
	mov	x2, #SCTLR_RESERVED & 0xffff0000
	orr	x1, x1, x2
	msr	SCTLR_EL1, x1
	// Redirect EL1 vectors to 0, which will infinitely abort. We
	// should never get an abort here, and can't do anything
	// useful if we do anyway.
	mov	x1, #0
	msr	VBAR_EL1, x1
	// Go to EL1.
	mov	x1, #CPSR_EL1
	msr	SPSR_EL3, x1
	adr	x1, 1f
	msr	ELR_EL3, x1
	eret
1:
	// Now running EL1 with MMU off. All accesses are device, non-secure.
	// Run through dup-tags and clean-invalidate anything marked TZ1=0.
	ldr	x0, L_ccu0_addr
	mov	x1, #0
	bl	_clean_invalidate_cp_channel
	ldr	x0, L_ccu1_addr
	mov	x1, #0
	bl	_clean_invalidate_cp_channel

	// Get back to EL3.
	adr	x0, 1f
	smc	#0x5ec3			// Secure EL3 (SEC3)
1:
	// Now running EL3 with MMU off. All accesses are device, secure.
	// Run through dup-tags and clean-invalidate anything marked TZ1=1.
	ldr	x0, L_ccu0_addr
	mov	x1, #1
	bl	_clean_invalidate_cp_channel
	ldr	x0, L_ccu1_addr
	mov	x1, #1
	bl	_clean_invalidate_cp_channel
	// All L1 and L2 Dcache contents are gone!

#endif // WITH_DUP_TAGS_CLEAN_INVALIDATE

_trampoline_finish:
#if VERIFY_INVALIDATION
	// Let's "quickly" double-check: all dup-tags should be invalid.
	// WARNING: This takes 70 ms when running with quiesced clocks
	ldr	x0, L_ccu0_addr
	bl	_verify_invalidated_cp_channel
	ldr	x0, L_ccu1_addr
	bl	_verify_invalidated_cp_channel
#endif // VERIFY_INVALIDATION

#if WITH_ROM_TRAMPOLINE
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
	mov	x30, x27	// lr = next boot stage PC
	mov	x0, x28		// x0 = next boot stage info
	mov	x27, #0
	mov	x28, #0
	mov	x29, #0
	msr	TTBR0_EL3, x1
	msr	VBAR_EL3, x1
	msr	ELR_EL1, x1
	msr	ELR_EL2, x1
	msr	ELR_EL3, x1
	msr	SPSR_EL1, x1
	msr	SPSR_EL2, x1
	msr	SPSR_EL3, x1
	msr	SP_EL1, x1
	msr	SP_EL2, x1
	mov	sp, x1		// SP_EL0
	msr	SPSel, #1
	mov	sp, x1		// SP_EL3

	// Invalidate I-cache
	ic      iallu
	dsb	sy
	isb	sy

	// Jump to the next stage
	ret

#if WITH_DUP_TAGS_CLEAN_INVALIDATE
_clean_invalidate_cp_channel:
	// x0 = pointer to base of channel
	// x1 = TZ1 match state, 0=Non-Secure, 1=Secure.
	mov	x2, #0			// x2 = way = 0..7
1:	mov	x3, #0			// x3 = set (index) = 0..1023	
2:	ldr	w4, [x0], #4		// x4 = duptag for set:x2, way:x3
	ubfx	x5, x4, #21, #2		// x5 = duptag[21:20] = state
	cmp	x5, #0			// Skip this duptag if state=0=invalid
	b.eq	9f
	ubfx	x5, x4, #19, #1		// x5 = TZ1
	cmp	x5, x1			// Skip this duptag if TZ1 mismatch
	b.ne	9f
	ubfx	x5, x4, #0, #19		// x5 = address bits[35:17]
	lsl	x5, x5, #17
	orr	x5, x5, x3, lsl #7	// x5 |= index << 7
	// TODO - calculate hash so we know better than accuracy of 2 lines.
	dc	civac, x5		// Clean-invalidate line.
	eor	x5, x5, #64
	dc	civac, x5		// Clean-invalidate other line of the pair
9:	add	x3, x3, #1		// Loop over sets (index)
	cmp	x3, #1024
	b.ne	2b
	add	x2, x2, #1		// Loop over ways
	cmp	x2, #8
	b.ne	1b
	dsb	sy			// Memory barrier.
	ret
#endif // WITH_DUP_TAGS_CLEAN_INVALIDATE

#if VERIFY_INVALIDATION
_verify_invalidated_cp_channel:
	// x0 = pointer to base of channel
	mov	x1, #1024 * 8		// Loop over all sets, ways.
1:	ldr	w2, [x0], #4		// x4 = duptag
	ubfx	x3, x2, #21, #2		// x5 = duptag[21:20] = state
	cmp	x3, #0			// Check state=0=invalid
	b.ne	9f
	sub	x1, x1, #1
	cmp	x1, #0
	b.ne	1b
	ret
	// Not really able to panic at this point, so let's just spin.
9:	wfe
	b	9b
#endif // VERIFY_INVALIDATION

	.balign	8
L_ccu0_addr:
	.8byte	CP_0_DT_DBG_CA0_ADDR
L_ccu1_addr:
	.8byte	CP_1_DT_DBG_CA0_ADDR
L_fuse0_addr:
	.8byte	CHIPID_BASE_ADDR + 0x00

#if WITH_ROM_TRAMPOLINE
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
