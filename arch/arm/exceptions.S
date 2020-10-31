/*
 * Copyright (C) 2006 Apple Computer, Inc. All rights reserved.
 * Copyright (C) 2007-2013 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Computer, Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Computer, Inc.
 */

#include <arch/arm/arm.h>
#include <arch/arm/assembler.h>
	
/*
 * struct arm_exception_frame {
 *	uint32_t sp;
 *	uint32_t lr;
 * 	uint32_t spsr;
 * 	uint32_t r[13];
 * 	uint32_t pc;
 *	uint64_t fpr[32];
 *	uint32_t fpscr;
 *	uint32_t fpexc;
 * };
 *
 * Note that when returning to the mode in which the exception was
 * taken that set the I and F bits to mask interrupts and clear the
 * T bit so that we stay in ARM mode.
 */

#if VFP_REV >= 3

#define VFP_SAVE			\
	vstmdb.64   sp!, {d0-d3}    ;	\
	vstmdb.64   sp!, {d4-d7}    ;	\
	vstmdb.64   sp!, {d8-d11}   ;	\
	vstmdb.64   sp!, {d12-d15}  ;	\
	vstmdb.64   sp!, {d16-d19}  ;	\
	vstmdb.64   sp!, {d20-d23}  ;	\
	vstmdb.64   sp!, {d24-d27}  ;	\
	vstmdb.64   sp!, {d28-d31}  ;	\
	fmrx        r1, fpscr	    ;	\
	stmfd	    sp!, { r1 }     ;	\
	fmrx        r1, fpexc       ;	\
	stmfd	    sp!, { r1 }
	

#define VFP_LOAD			\
	ldmfd       sp!, { r1 }     ;	\
	fmxr        fpexc, r1       ;	\
	ldmfd       sp!, { r1 }     ;	\
	vldmia.64   sp!, {d28-d31}  ;	\
	vldmia.64   sp!, {d24-d27}  ;	\
	vldmia.64   sp!, {d20-d23}  ;	\
	vldmia.64   sp!, {d16-d19}  ;	\
	vldmia.64   sp!, {d12-d15}  ;	\
	vldmia.64   sp!, {d8-d11}   ;	\
	vldmia.64   sp!, {d4-d7}    ;	\
	vldmia.64   sp!, {d0-d3}    ;	\
	fmxr        fpscr, r1

#else

#define VFP_SAVE
#define VFP_LOAD

#endif	

#define PUSH_EXCEPTION_FRAME		\
	stmfd	sp!, { r0-r12, lr } ;	\
	mrs	r0, spsr	    ;	\
	stmfd	sp!, { r0 }	    ;	\
	orr	r0, r0, #0xc0	    ;	\
	bic	r0, r0, #(1 << 5)   ;	\
	mrs	r1, cpsr	    ;	\
	msr	cpsr_c, r0	    ;	\
	mov	r2, sp		    ;	\
	mov	r3, lr		    ;	\
	msr	cpsr_c, r1	    ;	\
	stmfd	sp!, { r2-r3 }

#define POP_EXCEPTION_FRAME		\
	add	sp, sp, #12         ;	\
	ldmfd	sp!, { r0-r12, pc }^


	.text
/*
 * Interrupt handling.
 *
 * Note that FIQ is not intended to be "fast", but rather an NMI.
 */

ARM_FUNCTION ___arm_irq

	sub	lr, lr, #4		/* fix up return address */

	PUSH_EXCEPTION_FRAME
	VFP_SAVE

#if ARCH_ARMv7
	mrs	r1, cpsr
	bic	r1, #0x100		/* re-enable imprecise data aborts */
	msr	cpsr_x, r1
#endif

	bl	_arm_irq		/* call handler */

	VFP_LOAD
	POP_EXCEPTION_FRAME


ARM_FUNCTION	___arm_fiq

	sub	lr, lr, #4		/* fix up return address */

	/*
	 * We can't use PUSH_EXCEPTION_FRAME here as we need to save banked registers,
	 * but the only difference should be the re-saving of r8-r12 in the saved mode.
	 */
	stmfd	sp!, { r0-r12, lr }	/* save unbanked registers */
	mrs	r0, spsr		/* save spsr */
	stmfd	sp!, { r0 }
	add	r4, sp, #36		/* address of r8 in save frame */
	orr	r0, r0, #0xc0		/* mask interrupts and swap to previous mode */
	mrs	r1, cpsr
	msr	cpsr_c, r0
	stmia	r4, { r8-r12 }		/* save banked registers */
	mov	r2, sp			/* grab sp & lr,  swap back and save them */
	mov	r3, lr
	msr	cpsr_c, r1
#if ARCH_ARMv7
	bic	r1, #0x100		/* re-enable imprecise data aborts */
	msr	cpsr_x, r1
#endif
	stmfd	sp!, { r2-r3 }

	VFP_SAVE
	
	bl	_arm_fiq		/* call handler */

	VFP_LOAD
	POP_EXCEPTION_FRAME


	
/*
 * Exception handling.
 *
 * In each case, the handler is expected to populate the
 * exception frame structure and call the high-level dispatch
 * routine.  Note that we don't support resuming from any
 * exceptions other than IRQ/FIQ.
 *
 */

ARM_FUNCTION ___arm_undefined

	PUSH_EXCEPTION_FRAME

	mov	r0, sp
	bl	_arm_undefined
	b	.

ARM_FUNCTION ___arm_syscall

	PUSH_EXCEPTION_FRAME

	mov	r0, sp
	bl	_arm_syscall
	b	.

ARM_FUNCTION ___arm_prefetch_abort

	sub	lr, lr, #4

	PUSH_EXCEPTION_FRAME

	mov	r0, sp
	bl	_arm_prefetch_abort
	b	.

ARM_FUNCTION ___arm_data_abort

	PUSH_EXCEPTION_FRAME

	mov	r0, sp
	bl	_arm_data_abort
	b	.

ARM_FUNCTION ___arm_reserved

	b	.
