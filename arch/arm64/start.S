/*
 * Copyright (c) 2011-2014 Apple Inc.  All rights reserved.	
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */

#include <arch/arm64/proc_reg.h>
#include <platform/memmap.h>

	.text
	.align		12
	.no_dead_strip 	_start
	.globl 		_start	
_start:
	// Relocate, if needed
	adrp	x0, _start@page
	add	x0, x0, _start@pageoff
	ldr	x1, L_TEXT_BASE

#ifdef	PLATFORM_START_FUNCTION
	/*
	 * x0 and x1 must be preserved
	 * 'eq' will be true if no text relocation is required
	 */
	bl	PLATFORM_START_FUNCTION
#endif

	cmp	x1, x0
	b.eq	L__continue_start


#if !CPU_APPLE_CYCLONE
	mov	x2, #(SCTLR_I_ENABLED)
	msr	SCTLR_ELx, x2
	ic	iallu
#endif

	mov	lr, x1
	ldr	x2, L_TEXT_END
	ldr	x3, L_TEXT_BASE
	sub	x2, x2, x3
L__relocate_loop:
	ldp	x3, x4, [x0], #16
	stp	x3, x4, [x1], #16
	subs	x2, x2, #16
	b.ne	L__relocate_loop
	ret

L__continue_start:
	// Disable all interrupts
	msr	DAIFSet, #(DAIFSC_ALL)

	// Setup return address (virtual)
	adrp	lr, __main@page
	add	lr, lr, __main@pageoff

	// Set up exception vectors
	adrp	x10, _exception_vector_base@page
	add	x10, x10, _exception_vector_base@pageoff
	msr	VBAR_ELx, x10

	// bzero stacks region
	ldr	x10, L_STACKS_BASE
	ldr	x11, L_STACKS_SIZE
	add	x11, x11, x10
	mov	x12, #0
L__stacks_zero_loop:
	stp	x12, x12, [x10], #16
	cmp	x10, x11
	b.ne	L__stacks_zero_loop

	// bzero page tables region
	ldr	x10, L_PAGE_TABLES_BASE
	ldr	x11, L_PAGE_TABLES_SIZE
	add	x11, x11, x10
	mov	x12, #0
L__tables_zero_loop:
	stp	x12, x12, [x10], #16
	cmp	x10, x11
	b.ne	L__tables_zero_loop

	// Set up stack pointers
	ldr	x10, L_STACKS_BASE

	// exception stack goes at very bottom
	add	x10, x10, #(EXCEPTION_STACK_SIZE)
	mov	sp, x10					// set SPx to exception stack

	// bootstrap stack above exception stack
	msr	SPSel, #0				// switch to SP0
	add	x10, x10, #(BOOTSTRAP_STACK_SIZE)
	mov	sp, x10					// set SP0 to bootstrap stack

	// Save interrupt stack top, to be stored after bss is cleared
	// interrupt stack goes at top so that it can share with bootstrap
	// stack after bootstrap task exits
	add	x20, x10, #(INTERRUPT_STACK_SIZE)

	// SecureROM: relocate data to SRAM 
#if APPLICATION_SECUREROM
	ldr	x10, L_DATA_RO_START
	mov	x11, #(PAGE_SIZE - 1)
	add	x10, x10, x11
	bic	x10, x10, x11
	ldr	x11, L_DATA_BASE
	cmp	x10, x11
	b.eq	L__do_bss

	ldr	x12, L_DATA_END
L__relocate_data_loop:
	ldp	x13, x14, [x10], #16
	stp	x13, x14, [x11], #16
	cmp	x11, x12
	b.ne	L__relocate_data_loop
#endif // APPLICATION_SECUREROM

	// Clear the bss section
	// We requested all the sections to be aligned by 64.
	// Section length should be 4 byte aligned.
L__do_bss:
	ldr	x10, L_BSS_START
	ldr	x11, L_BSS_END
	mov	x12, #15
	bic	x12, x11, x12
	mov	x13, #0
L__bss_loop_stp:
	stp	x13, x13, [x10], #16
	cmp	x10, x12
	b.ne	L__bss_loop_stp
	cmp	x11, x12
	b.eq	L__bss_done
L__bss_loop_word:
	str	w13, [x10], #4
	cmp	x10, x11
	b.lt	L__bss_loop_word
L__bss_done:

	// Store interrupt stack top
	adrp	x11, _interrupt_stack_top@page
	add	x11, x11, _interrupt_stack_top@pageoff
	str	x20, [x11]

#if defined(HEAP_GUARD) && (HEAP_GUARD != SRAM_END)
	// Clear the heap guard page, which will never be mapped
	mov	x13, #0
	ldr	x10, L_HEAP_GUARD
	mov	x11, #(PAGE_SIZE)
	add	x11, x10, x11
L__loop_heap_guard:
	stp	x13, x13, [x10], #16
	stp	x13, x13, [x10], #16
	stp	x13, x13, [x10], #16
	stp	x13, x13, [x10], #16
	cmp	x10, x11
	b.ne	L__loop_heap_guard
#endif

#ifdef BOOT_TRAMPOLINE_BASE
	// copy the trampoline into a non-writeable page
	ldr	x3, L_boot_trampoline_start
	ldr	x4, L_boot_trampoline_end
	ldr	x5, L_boot_trampoline_dst
1:
	ldp	x6, x7, [x3], #16
	stp	x6, x7, [x5], #16
	cmp	x3, x4
	b.lo	1b
#endif


	// Branch to main
	ret


	.org	0x200
L__build_banner_string:
	.ascii CONFIG_PROGNAME_STRING
	.ascii " for "
	.ascii CONFIG_BOARD_STRING
	.ascii ", Copyright 2007-2016, Apple Inc."

	.org	0x240
L__build_style_string:
	.ascii BUILD_STYLE

/* This will get replaced by macho_post_process.py so that the build tag
   is accurate even for iterative desktop builds that don't rebuild start.o */
	.org	0x280
_build_tag_string_contents:

	.org	0x300
	.globl	_build_banner_string
_build_banner_string:
	.8byte	L__build_banner_string

	.balign 8
	.globl	_build_style_string
_build_style_string:
	.8byte	L__build_style_string

	.balign	8
	.globl	_build_tag_string
_build_tag_string:
	.8byte	_build_tag_string_contents

	.balign	8
	// Linked start and end addresses of the text
L_TEXT_BASE:
	.8byte	(TEXT_BASE)
L_TEXT_END:
	.8byte	section$start$__DATA$__zerofill
L_TEXT_SIZE:
	.8byte	segment$end$__TEXT - segment$start$__TEXT

	// start address of the data in the read only text */
L_DATA_RO_START:
	.8byte	segment$end$__TEXT

	// Linked start and end addresses of the data
L_DATA_BASE:
	.8byte	segment$start$__DATA
L_DATA_END:
	.8byte	section$start$__DATA$__zerofill

	// Linked start and end addresses of the zero-filled range
L_BSS_START:
	.8byte	section$start$__DATA$__zerofill
L_BSS_END:
	.8byte	segment$end$__DATA

L_STACKS_BASE:
	.8byte	(STACKS_BASE)
L_STACKS_SIZE:
	.8byte	(STACKS_SIZE)
L_PAGE_TABLES_BASE:
	.8byte	(PAGE_TABLES_BASE)
L_PAGE_TABLES_SIZE:
	.8byte	(PAGE_TABLES_SIZE)

#if defined(HEAP_GUARD) && (HEAP_GUARD != SRAM_END)
L_HEAP_GUARD:
	.8byte	(HEAP_GUARD)
#endif
#ifdef BOOT_TRAMPOLINE_BASE
L_boot_trampoline_start:
	.8byte	_boot_handoff_trampoline
L_boot_trampoline_end:
	.8byte	_boot_handoff_trampoline_end
L_boot_trampoline_dst:
	.8byte	BOOT_TRAMPOLINE_BASE
#endif

	.data
 
	.globl	_interrupt_stack_top
_interrupt_stack_top:
	.space	8
