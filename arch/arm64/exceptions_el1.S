/*
 * Copyright (C) 2011-2012, 2014 Apple Computer, Inc. All rights reserved.
 *
 * This document is the property of Apple Computer, Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Computer, Inc.
 */

#if	WITH_EL3
#error	"This file should not be built for platforms WITH_EL3"
#endif

////////////////////////////////////////////////////////////
// This is what exception frame looks like:
//
//	struct arm_exception_frame64 {
//		uint64_t	regs[29];	// x0-x28
//		uint64_t	fp;
//		uint64_t	lr;
//		uint64_t	sp;
//		uint32_t	spsr;
//		uint32_t	reserved0;
//		uint64_t	pc;
//		uint64_t	far;
//		uint32_t	esr;
//		uint32_t	reserved1;
//		uint64_t	reserved2;	// stp requires multiple of 16 in imm field
//		uint64_t	reserved3;	// stp requires multiple of 16 in imm field
//		union {
//			uint128_t	q;
//			uint64_t	d;
//			uint32_t	s;
//		} vregs[32];			// v0-v31
//		uint32_t	fpsr;
//		uint32_t	reserved4;
//		uint32_t	fpcr;
//		uint32_t	reserved5;
//	};
////////////////////////////////////////////////////////////

.macro EL1_SP0_VECTOR
	msr	SPSel, #0			// Switch to SP0
	sub	sp, sp, #832			// Create exception frame
	stp	x0, x1, [sp, #0]		// Save x0, x1 to exception frame
	add	x0, sp, #832			// Calculate the original stack pointer
	str	x0, [sp, #248]			// Save stack pointer to exception frame
	stp	fp, lr, [sp, #232]		// Save fp and lr to exception frame
	mrs     lr, ELR_EL1			// Get exception link reg so _dispatch64 can make a frame
	mov	x0, sp				// Copy saved state pointer to x0
.endmacro

.macro FASTSIM_DEBUG_HINT
	hint	0x45	// make fastsim drop to debugger (nop on hardware)
.endmacro

	.text
	.align 12
	.globl _exception_vector_base
_exception_vector_base:
L__el1_sp0_synchronous_vector:
	EL1_SP0_VECTOR
	mrs	x1, ESR_EL1			// Load exception syndrome
	str	x1, [x0, #280]
	mrs	x1, FAR_EL1			// Load fault address
	str	x1, [x0, #272]
	adrp	x1, _arm_synchronous_exception@page
	add	x1, x1, _arm_synchronous_exception@pageoff
	b	__dispatch64

	.text
	.align 7
L__el1_sp0_irq_vector:
	EL1_SP0_VECTOR
	adrp	x1, _interrupt_stack_top@page
	add	x1, x1, _interrupt_stack_top@pageoff
	ldr	x1, [x1]
	mov	sp, x1
	adrp	x1, _arm_irq@page
	add	x1, x1, _arm_irq@pageoff
	b	__dispatch64

	.text
	.align 7
L__el1_sp0_fiq_vector:
	EL1_SP0_VECTOR
	adrp	x1, _interrupt_stack_top@page
	add	x1, x1, _interrupt_stack_top@pageoff
	ldr	x1, [x1]
	mov	sp, x1
	adrp	x1, _arm_fiq@page
	add	x1, x1, _arm_fiq@pageoff
	b	__dispatch64

	.text
	.align 7
L__el1_sp0_serror_vector:
	EL1_SP0_VECTOR
	mrs	x1, ESR_EL1			// Load exception syndrome
	str	x1, [x0, #280]
	mrs	x1, FAR_EL1			// Load fault address
	str	x1, [x0, #272]
	adrp	x1, _arm_serror_exception@page
	add	x1, x1, _arm_serror_exception@pageoff
	b	__dispatch64

	.text
	.align 7
L__el1_sp1_synchronous_vector:
	FASTSIM_DEBUG_HINT
	b	.

	.text
	.align 7
L__el1_sp1_irq_vector:
	FASTSIM_DEBUG_HINT
	b	.

	.text
	.align 7
L__el1_sp1_fiq_vector:
	FASTSIM_DEBUG_HINT
	b	.

	.text
	.align 7
L__el1_sp1_serror_vector:
	FASTSIM_DEBUG_HINT
	b	.

	.text
	.align 7
L__elx_64_synchronous_vector:
	FASTSIM_DEBUG_HINT
	b	.

	.text
	.align 7
L__elx_64_irq_vector:
	FASTSIM_DEBUG_HINT
	b	.

	.text
	.align 7
L__elx_64_fiq_vector:
	FASTSIM_DEBUG_HINT
	b	.

	.text
	.align 7
L__elx_64_serror_vector:
	FASTSIM_DEBUG_HINT
	b	.

	.text
	.align 7
L__elx_32_synchronous_vector:
	FASTSIM_DEBUG_HINT
	b	.

	.text
	.align 7
L__elx_32_irq_vector:
	FASTSIM_DEBUG_HINT
	b	.

	.text
	.align 7
L__elx_32_fiq_vector:
	FASTSIM_DEBUG_HINT
	b	.

	.text
	.align 7
L__elx_32_serror_vector:
	FASTSIM_DEBUG_HINT
	b	.

/* Placeholder for macho_post_process.py. Placing the UUID here
   allows debuggers to locate the UUID based on the value of VBAR */
	.text
	.align 7
_UUID:
	.space 32

/* 64-bit first level exception handler dispatcher.
* Completes register context saving and branches to FLEH.
* Expects:
*  {x0, x1, fp, lr, sp} - saved
*  x0 - arm_context_t
*  x1 - address of FLEH
*  fp - previous stack frame if EL1
*  lr - unused
*  sp - kernel stack
*/
	.text
	.align 2
__dispatch64:
	stp	fp, lr, [sp, #-16]!		// Create a frame to help debuggers unwind
	mov	fp, sp				// past the exception handler
	stp	x2, x3, [x0, #16]		// Save remaining GPRs
	stp	x4, x5, [x0, #32]
	stp	x6, x7, [x0, #48]
	stp	x8, x9, [x0, #64]
	stp	x10, x11, [x0, #80]
	stp	x12, x13, [x0, #96]
	stp	x14, x15, [x0, #112]
	stp	x16, x17, [x0, #128]
	stp	x18, x19, [x0, #144]
	stp	x20, x21, [x0, #160]
	stp	x22, x23, [x0, #176]
	stp	x24, x25, [x0, #192]
	stp	x26, x27, [x0, #208]
	str	x28, [x0, #224]
	mrs	lr, ELR_EL1			// Get exception link register
	str	lr, [x0, #264]			// Save ELR to PCB
	mrs	x2, SPSR_EL1			// Get CPSR
	str	w2, [x0, #256]			// Save CPSR to PCB

#if WITH_VFP
	stp	q0, q1, [x0, #304]
	stp	q2, q3, [x0, #336]
	stp	q4, q5, [x0, #368]
	stp	q6, q7, [x0, #400]
	stp	q8, q9, [x0, #432]
	stp	q10, q11, [x0, #464]
	stp	q12, q13, [x0, #496]
	stp	q14, q15, [x0, #528]
	stp	q16, q17, [x0, #560]
	stp	q18, q19, [x0, #592]
	stp	q20, q21, [x0, #624]
	stp	q22, q23, [x0, #656]
	stp	q24, q25, [x0, #688]
	stp	q26, q27, [x0, #720]
	stp	q28, q29, [x0, #752]
	stp	q30, q31, [x0, #784]
	mrs	x2, FPSR
	str	w2, [x0, #816]
	mrs	x2, FPCR
	str	w2, [x0, #820]
#endif

	mov	x21, x0				// Copy arm_context_t pointer to x21
	blr	x1
	b	__exception_return
	
	.text
	.align 2
__exception_return:
	mov     sp, x21				// Reload the arm_context_t pointer

#if WITH_VFP
	ldp	q0, q1, [sp, #304]
	ldp	q2, q3, [sp, #336]
	ldp	q4, q5, [sp, #368]
	ldp	q6, q7, [sp, #400]
	ldp	q8, q9, [sp, #432]
	ldp	q10, q11, [sp, #464]
	ldp	q12, q13, [sp, #496]
	ldp	q14, q15, [sp, #528]
	ldp	q16, q17, [sp, #560]
	ldp	q18, q19, [sp, #592]
	ldp	q20, q21, [sp, #624]
	ldp	q22, q23, [sp, #656]
	ldp	q24, q25, [sp, #688]
	ldp	q26, q27, [sp, #720]
	ldp	q28, q29, [sp, #752]
	ldp	q30, q31, [sp, #784]
	ldr	w3, [sp, #816]
	msr	FPSR, x3
	ldr	w4, [sp, #820]
	msr	FPCR, x4
#endif

	ldr	x0, [sp, #264]			// Get the return address
	msr	ELR_EL1, x0			// Load the return address into ELR
	ldr	w1, [sp, #256]			// Get the return CPSR
	msr	SPSR_EL1, x1			// Load the return CPSR into SPSR
	ldp	x0, x1, [sp, #0]		// Restore the GPRs
	ldp	x2, x3, [sp, #16]
	ldp	x4, x5, [sp, #32]
	ldp	x6, x7, [sp, #48]
	ldp	x8, x9, [sp, #64]
	ldp	x10, x11, [sp, #80]
	ldp	x12, x13, [sp, #96]
	ldp	x14, x15, [sp, #112]
	ldp	x16, x17, [sp, #128]
	ldp	x18, x19, [sp, #144]
	ldp	x20, x21, [sp, #160]
	ldp	x22, x23, [sp, #176]
	ldp	x24, x25, [sp, #192]
	ldp	x26, x27, [sp, #208]
	ldr	x28, [sp, #224]
	ldp	fp, lr, [sp, #232]

	/* Use exception stack to restore SP0 */

	msr	SPSel, #1			// Switch to SPx
	stp	x0, x1, [sp, #-16]!		// Save x0 and x1
	mrs	x0, SP_EL0			// Reload the pcb pointer from SP0
	ldr	x1, [x0, #248]			// Get the saved SP from the pcb
	msr	SP_EL0, x1			// Restore SP0
	ldp	x0, x1, [sp], #16		// Restore x0 and x1
	eret
