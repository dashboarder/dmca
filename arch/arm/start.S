/*
 * Copyright (C) 2007-2014 Apple Inc. All rights reserved.
 * Copyright (C) 2006 Apple Computer, Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */

#include <platform/memmap.h>

	.text
	.balign	4
	.no_dead_strip _start
	.global _start

_start:
	b	__reset

	ldr	pc, vectoraddr + 4
	ldr	pc, vectoraddr + 8
	ldr	pc, vectoraddr + 12
	ldr	pc, vectoraddr + 16
	ldr	pc, vectoraddr + 20
	ldr	pc, vectoraddr + 24
	ldr	pc, vectoraddr + 28

vectoraddr:
	.word	__reset
	.word	___arm_undefined
	.word	___arm_syscall
	.word	___arm_prefetch_abort
	.word	___arm_data_abort
	.word	___arm_reserved
	.word	___arm_irq
	.word	___arm_fiq

__reset:
	/*
	 * Test the running versus desired the start of the text
	 * to determine if relocaiton is required
	 */
	adr	r0, _start
	ldr	r1, L_TEXT_START
	cmp	r0, r1

#ifdef PLATFORM_START_FUNCTION
	/*
	 * r0 and r1 must be preserved
	 * 'eq' will be true if no text relocation is required
	 */
	bl	PLATFORM_START_FUNCTION
#endif

	/* re-test the running start versus desired start of the text */
	cmp	r0, r1
	beq	__do_relocate_data

	/* relocate to the desired text location and branch to it */
	ldr	r2, L_TEXT_END
	sub	r2, r2, r1
__relocate_loop:
	subs	r2, r2, #4
	ldr	r3, [r0, r2]
	str	r3, [r1, r2]
	bne	__relocate_loop

#if ARCH_ARMv7
	/* prevent speculation to the desired text location */
	dsb
	isb
#endif

	/* branch into the desired text location */
	bx	r1

	/* test if the data if needs to be relocated */
__do_relocate_data:
	ldr	r0, L_DATA_RO_START
	mov	r1, #0x00001000		// Round up to a 4K page 
	sub	r1, r1, #1		//
	add	r0, r0, r1		//
	bic	r0, r0, r1		//
	ldr	r1, L_DATA_START
	cmp	r0, r1
	beq	__stacksetup

	/* relocate the data */
	ldr	r2, L_DATA_END
	sub	r2, r2, r1
__relocate_data_loop:
	subs	r2, r2, #4
	ldr	r3, [r0, r2]
	str	r3, [r1, r2]
	bne	__relocate_data_loop

__stacksetup:
	/* set up the stack for irq, fiq, abort, undefined, and lastly supervisor mode */
	mrs	r0, cpsr
	bic	r0, r0, #0x1f

	orr	r1, r0, #0x12 // irq
	msr	cpsr_c, r1
	mov	lr, #0
	ldr	sp, _irq_stack_top

	orr	r1, r0, #0x11 // fiq
	msr	cpsr_c, r1
	mov	lr, #0
	ldr	sp, _fiq_stack_top

	orr	r1, r0, #0x17 // abort
	msr	cpsr_c, r1
	mov	lr, #0
	ldr	sp, _exc_stack_top
	    
	orr	r1, r0, #0x1b // undefined
	msr	cpsr_c, r1
	mov	lr, #0
	ldr	sp, _exc_stack_top

	orr	r1, r0, #0x13 // supervisor
	msr	cpsr_c, r1
	ldr	sp, _svc_stack_top

#if ARCH_ARMv7
	mrs	r1, cpsr
	bic	r1, #0x100 // enable imprecise data aborts
	msr	cpsr_x, r1
#endif

#if SUPPORT_SLEEP
	/* look at the global wakeup flag, avoid clearing the BSS and go straight to wake up */
.global	_platform_wakeup
	ldr	r1, _arch_sleep_magic	/* will be zero on the wake path, nonzero otherwise */
	eor	r0, r0, r0
	cmp	r0, r1
	bne	__do_bss
	ldr	r0, L__platform_wakeup
	mov	lr, pc
	bx	r0
#endif

__do_bss:
	/* clear the bss section */
	ldr	r0, L_BSS_START
	ldr	r1, L_BSS_END
	mov	r2, #0
	mov	r3, #0
	mov	r4, #0
	mov	r5, #0
	mov	r6, #0
	mov	r7, #0
	mov	r8, #0
	mov	r10, #0
	sub	r12, r1, r0
	/* If there are less than 256 bytes, skip this loop */
	subs	r12, r12, #256
	blo	__bss_lessthan256

__bss_loop256:
	/* Clear 256 bytes at a time */
	subs	r12, #256
	stmia	r0!, {r2-r8, r10}
	stmia	r0!, {r2-r8, r10}
	stmia	r0!, {r2-r8, r10}
	stmia	r0!, {r2-r8, r10}
	stmia	r0!, {r2-r8, r10}
	stmia	r0!, {r2-r8, r10}
	stmia	r0!, {r2-r8, r10}
	stmia	r0!, {r2-r8, r10}
	bge	__bss_loop256

__bss_lessthan256:
	/* If are less than 32 bytes left to handle, skip this loop */
	adds	r12, r12, #(256 - 32)
	bcc	__bss_lessthan32

__bss_loop32:
	/* Clear 32 bytes at a time */
	subs	r12, r12, #32
	stmia	r0!, {r2-r8, r10}
	bge	__bss_loop32

__bss_lessthan32:
	/* If there's nothing left to go, skip this loop */
	adds	r12, r12, #32
	beq	__bss_done

__bss_loop4:
	/* Clear 4 bytes at a time */
	subs	r12, r12, #4
	str	r2, [r0],#4
	bgt	__bss_loop4

__bss_done:
	/* branch to main */
	ldr	r0, L__main
	mov	lr, pc
	bx	r0
	b	.


	.org 0x200
L__build_banner_string:
	.ascii CONFIG_PROGNAME_STRING
	.ascii " for "
	.ascii CONFIG_BOARD_STRING
	.ascii ", Copyright 2007-2016, Apple Inc."

	.org 0x240
L__build_style_string:
	.ascii BUILD_STYLE

/* This will get replaced by macho_post_process.py so that the build tag
   is accurate even for iterative desktop builds that don't rebuild start.o */
	.org 0x280
_build_tag_string_contents:

/* Placeholder for macho_post_process.py. Placing the UUID here allows
   debuggers to locate the UUID based on the address of the vectors */
	.org 0x300
_UUID:
	.space 0x20

	.global _build_banner_string
_build_banner_string:
	.long L__build_banner_string

	.global _build_style_string
_build_style_string:
	.long L__build_style_string

	.global _build_tag_string
_build_tag_string:
	.long _build_tag_string_contents

L__main:
	.long __main

	/* linked start and end addresses of the text */
L_TEXT_START:
	.long (TEXT_BASE)
L_TEXT_END:
	.long section$start$__DATA$__zerofill

	/* start address of the data in the read only text */
L_DATA_RO_START:
	.long segment$end$__TEXT

	/* linked start and end addresses of the data */
L_DATA_START:
	.long segment$start$__DATA
L_DATA_END:
	.long section$start$__DATA$__zerofill

	/* linked start and end addresses of the zero-filled range */
L_BSS_START:
	.long section$start$__DATA$__zerofill
L_BSS_END:
	.long segment$end$__DATA

	/*
	 * Stacks.
	 *
	 * The following stacks must be defined:
	 * 
	 * bootstrap stack
	 *	Used as the stack for the initial thread.
	 *
	 * exception stack
	 *	Used when handling exceptions other than interrupts.
	 *
	 * interrupt stack
	 *	Used when handling regular interrupts.
	 *
	 * fiq stack
	 *	Used when handling FIQ interrupts.
	 */
	
	.global _irq_stack_top
	.global _fiq_stack_top
	.global _exc_stack_top
	.global _svc_stack_top

#ifndef STACK_SIZE
	/*
	 * This is a bit hokey, as the platform doesn't really know
	 * what the bootstrap stack size should be.
	 */
# define STACK_SIZE	BOOTSTRAP_STACK_SIZE
#endif
#if APPLICATION_EMBEDDEDIOP
	/*
	 * Locally-allocated stacks.
	*/
_irq_stack_top:
	.long irq_stack + IRQ_STACK_SIZE
_fiq_stack_top:
	.long fiq_stack + FIQ_STACK_SIZE
_exc_stack_top:
	.long exc_stack + EXCEPTION_STACK_SIZE
_svc_stack_top:
	.long svc_stack + STACK_SIZE

	/* allocate stacks here */
	.lcomm irq_stack, IRQ_STACK_SIZE, 4
	.lcomm fiq_stack, FIQ_STACK_SIZE, 4
	.lcomm exc_stack, EXCEPTION_STACK_SIZE, 4
	.lcomm svc_stack, STACK_SIZE, 4

#elif defined(STACK_BASE)
	/*
	 * Simple stack layout with just one allocation in STACK_BASE/STACK_SIZE
	 * supplied by the platform definition.
	 *
	 * Bootstrap stack starts at the top of the allocation,
	 * exception stack and interrupt stack share the same
	 * chunk at the bottom.
	 */
_fiq_stack_top:	
	.long STACK_BASE + FIQ_STACK_SIZE
_exc_stack_top:
_irq_stack_top:
	.long STACK_BASE + IRQ_STACK_SIZE
_svc_stack_top:
	.long STACK_BASE + STACK_SIZE
#else
# error No stack defined
#endif

#if SUPPORT_SLEEP
.global	_arch_sleep_magic
_arch_sleep_magic:
	.long	0xffffffff

L__platform_wakeup:
	.long _platform_wakeup
	
#endif
