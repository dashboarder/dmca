/*
 * Copyright (c) 2009 Apple Inc.  All rights reserved.	
 * Copyright (C) 2006 Apple Computer, Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */

#include <arch/arm/arm.h>
#include <arch/arch_task.h>
#include <arch/arm/assembler.h>

/*
 * Task context switch support.
 * 
 * Basic VFP/Neon floating point context switch only.
 */
	
/*
 * Code here must remain synchronised with the structure and offsets defined in arch_task.h:
 *
 * struct arch_task {
 * 	uint32_t regs[10]; // r4-r11, r13, r14
 * #if WITH_VFP
 * 	uint32_t	fpscr;
 * 	uint32_t	fpexc;
 * 	uint32_t	fpinst;
 * 	uint32_t	fpinst2;
 * 	uint64_t	fpregs[32];	// assume VFP-D32
 * #endif
 * };
*/

/*
 * void arch_task_context_switch(struct arch_task *old, struct arch_task *new_task);
 */

ARM_FUNCTION _arch_task_context_switch
	
	/* save non-volatile core registers */
	stmia	r0, { r4-r11, r13, r14 }

#if WITH_VFP
#if !WITH_VFP_ALWAYS_ON
	/* save fpexc */
	vmrs	r2, fpexc
	str	r2, [r0, #ARM_ARCH_TASK_FPEXC] 

	/* check FP enable state */
	tst	r2, #FPEXC_EN
	beq	L_fp_save_done

	/* save fpscr */
	vmrs	r4, fpscr
	str	r4, [r0, #ARM_ARCH_TASK_FPSCR]
#endif
	
	/* save the FP registers */
	add	r3, r0, #ARM_ARCH_TASK_FPREGS
	vstm.64	r3!, { d0-d15 }
#if FP_REGISTER_COUNT > 16
	vstm.64	r3, { d16-d31 }
#endif
L_fp_save_done:	
#endif	/* WITH_VFP */
	
	/* select the context we're going to switch to and invoke the context restore */
	mov	r0, r1
	b	_arch_task_context_restore

	
/*
 * void arch_task_context_restore(struct arch_task *restore_context)
 */

ARM_FUNCTION _arch_task_context_restore

#if WITH_VFP
#if !WITH_VFP_ALWAYS_ON
	/* restore fpexc */
	ldr	r2, [r0, #ARM_ARCH_TASK_FPEXC]
	vmsr	fpexc, r2

	/* check FP enable state */
	tst	r2, #FPEXC_EN
	beq	L_fp_restore_done

	/* restore fpscr */
	ldr	r4, [r0, #ARM_ARCH_TASK_FPSCR]
	vmsr	fpscr, r4
#endif
	
	/* restore the FP registers */
	add	r3, r0, #ARM_ARCH_TASK_FPREGS
	vldm.64	r3!, { d0-d15 }
#if FP_REGISTER_COUNT > 16
	vldm.64	r3, { d16-d31 }
#endif
L_fp_restore_done:
#endif	/* WITH_VFP */

	/* restore non-volatile core registers */
	ldmia	r0, { r4-r11, r13, r14 }
	bx	lr


#if WITH_VFP && !WITH_VFP_ALWAYS_ON
/*
 * void arm_call_fpsaved(void *arg, void (*func)(void *arg))
 */
	.globl _arm_call_fpsaved
_arm_call_fpsaved:
	stmfd           sp!, {lr}

	/* fetch fpexc and check FP enable state */
	vmrs	r2, fpexc
	tst	r2, #FPEXC_EN
	beq	1f		/* bypass save if VFP is off */

	/* save fpscr */
	vmrs	r3, fpscr
	push	{r3}
	
	/* save the FP registers */
#if FP_REGISTER_COUNT > 16
	vpush	{ d16-d31 }
#endif
	vpush	{ d0-d15 }
1:
	/* stack fpexc last, always guranteed to be on the stack */
	push	{r2}
	
	/* unconditionally enable VFP for this call */
	orr	r2, r2, #FPEXC_EN
	vmsr	fpexc, r2
	
	/* branch to the routine */
	blx	r1

	/* pop fpexc and test whether FP was saved */
	pop	{r2}
	tst	r2, #FPEXC_EN
	beq	2f

	/* restore the FP registers */
	vpop	{ d0-d15 }
#if FP_REGISTER_COUNT > 16
	vpop	{ d16-d31 }
#endif

	/* restore fpcsr */
	pop	{r3}
	vmsr	fpscr, r2
2:
	/* restore fpexc last, may disable VFP */
	vmsr	fpexc, r2

	ldmfd		sp!, {lr}
	bx		lr
#endif /* WITH_VFP */
