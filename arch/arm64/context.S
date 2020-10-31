/*
 * Copyright (c) 2011-2012 Apple Inc.  All rights reserved.	
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */

/*
 * Task context switch support.
 * 
 */
	
/*
 * Code here must remain synchronized with the structure and offsets defined in arch_task.h:
 *
 *	struct arch_task {
 *		uint64_t		regs[29];	// x0-x28
 *		uint64_t		fp;
 *		uint64_t		lr;
 *		uint64_t		sp;	
 *
 *		union {
 *			uint128_t	q;
 *			uint64_t	d;
 *			uint32_t	s;
 *		} vregs[32];			// v0-v31
 *		uint32_t		fpsr;
 *		uint32_t		fpcr;
 *	};
 */

/* AAPCS-64 5.1.1 Page 13:
 * Preserve r16-17, r19-r28, FP, and SP.
 *
 * AAPCS-64 5.1.2 Page 14:
 * Registers d8-d15 (s8-s15) must be preserved by a callee across subroutine
 * calls; the remaining registers (v0-v7, v16-v31) do not need to be preserved
 * (or should be preserved by the caller).
 */

/*
 * void arch_task_context_switch(struct arch_task *old, struct arch_task *new_task);
 */

	.text
	.align 2
	.globl _arch_task_context_switch
_arch_task_context_switch:
	stp	x16, x17, [x0, #120]
	stp	x19, x20, [x0, #152]
	stp	x21, x22, [x0, #168]
	stp	x23, x24, [x0, #184]
	stp	x25, x26, [x0, #200]
	stp	x27, x28, [x0, #216]
	stp	fp, lr, [x0, #232]
	mov	x2, sp
	str	x2, [x0, #248]

#if WITH_VFP
        str	d8,  [x0, #384]
        str	d9,  [x0, #400]
        str	d10, [x0, #416]
        str	d11, [x0, #432]
        str	d12, [x0, #448]
        str	d13, [x0, #464]
        str	d14, [x0, #480]
        str	d15, [x0, #496]
#endif
	
	mov	x0, x1
	b	_arch_task_context_restore
	
/*
 * void arch_task_context_restore(struct arch_task *restore_context)
 */

	.text
	.align 2
	.globl _arch_task_context_restore
_arch_task_context_restore:
	ldp	x16, x17, [x0, #120]
	ldp	x19, x20, [x0, #152]
	ldp	x21, x22, [x0, #168]
	ldp	x23, x24, [x0, #184]
	ldp	x25, x26, [x0, #200]
	ldp	x27, x28, [x0, #216]
	ldp	fp, lr, [x0, #232]
	ldr	x1, [x0, #248]
	mov	sp, x1

#if WITH_VFP
        ldr	d8,  [x0, #384]
        ldr	d9,  [x0, #400]
        ldr	d10, [x0, #416]
        ldr	d11, [x0, #432]
        ldr	d12, [x0, #448]
        ldr	d13, [x0, #464]
        ldr	d14, [x0, #480]
        ldr	d15, [x0, #496]
#endif

	ret
	
