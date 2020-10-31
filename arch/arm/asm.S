/*
 * Copyright (C) 2007-2011 Apple Inc. All rights reserved.
 * Copyright (C) 2006 Apple Computer, Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */

#include <arch/arm/assembler.h>
	
	.text

#define ALL_ROUTINES 1

// Define isb as a nop for older architectures
#if !ARCH_ARMv7
#define isb	nop
#define dsb	nop
#endif /* ! ARCH_ARMv7 */

ARM_FUNCTION _arm_read_cpsr

	mrs		r0, cpsr
	bx		lr

#if ALL_ROUTINES
ARM_FUNCTION _arm_read_main_id

	mrc		p15, 0, r0, c0, c0, 0
	bx		lr
#endif

#if ALL_ROUTINES
ARM_FUNCTION _arm_read_cache_id

	mrc		p15, 0, r0, c0, c0, 1
	bx		lr

ARM_FUNCTION _arm_read_cache_level_id

	mrc		p15, 1, r0, c0, c0, 1
	bx		lr

ARM_FUNCTION _arm_read_cache_size_selection

	mrc		p15, 2, r0, c0, c0, 0
	bx		lr

ARM_FUNCTION _arm_write_cache_size_selection

	mcr		p15, 2, r0, c0, c0, 0
	isb
	bx		lr

ARM_FUNCTION _arm_read_cache_size_id

	mrc		p15, 1, r0, c0, c0, 0
	bx		lr
#endif

#if ALL_ROUTINES
ARM_FUNCTION _arm_read_extended_feature_regs

	mov		r12, r0
	mrc		p15, 0, r0, c0, c1, 0
	mrc		p15, 0, r1, c0, c1, 1
	mrc		p15, 0, r2, c0, c1, 2
	mrc		p15, 0, r3, c0, c1, 3
	stmia		r12!, { r0-r3 }
	mrc		p15, 0, r0, c0, c1, 4
	mrc		p15, 0, r1, c0, c1, 5
	mrc		p15, 0, r2, c0, c1, 6
	mrc		p15, 0, r3, c0, c1, 7
	stmia		r12!, { r0-r3 }
	mrc		p15, 0, r0, c0, c2, 0
	mrc		p15, 0, r1, c0, c2, 1
	mrc		p15, 0, r2, c0, c2, 2
	mrc		p15, 0, r3, c0, c2, 3
	stmia		r12!, { r0-r3 }
	mrc		p15, 0, r0, c0, c2, 4
	mrc		p15, 0, r1, c0, c2, 5
	mrc		p15, 0, r2, c0, c2, 6
	mrc		p15, 0, r3, c0, c2, 7
	stmia		r12, { r0-r3 }
	bx		lr
#endif

#if ALL_ROUTINES
ARM_FUNCTION _arm_read_memory_model_feature_regs

	mov		r12, r0
	mrc		p15, 0, r0, c0, c1, 4
	mrc		p15, 0, r1, c0, c1, 5
	mrc		p15, 0, r2, c0, c1, 6
	mrc		p15, 0, r3, c0, c1, 7
	stmia		r12, { r0-r3 }
	bx		lr
#endif

#if ALL_ROUTINES
ARM_FUNCTION _arm_read_instruction_set_attribute_regs

	mov		r12, r0
	mrc		p15, 0, r0, c0, c2, 0
	mrc		p15, 0, r1, c0, c2, 1
	mrc		p15, 0, r2, c0, c2, 2
	mrc		p15, 0, r3, c0, c2, 3
	stmia		r12!, { r0-r3 }
	mrc		p15, 0, r0, c0, c2, 4
	mrc		p15, 0, r1, c0, c2, 5
	stmia		r12!, { r0-r1 }
	bx		lr
#endif

ARM_FUNCTION _arm_read_cr

	mrc		p15, 0, r0, c1, c0, 0
	bx		lr

ARM_FUNCTION _arm_write_cr

	mcr		p15, 0, r0, c1, c0, 0
	isb
	bx		lr

ARM_FUNCTION _arm_read_aux_cr

	mrc		p15, 0, r0, c1, c0, 1
	bx		lr
	
ARM_FUNCTION _arm_write_aux_cr

	mcr		p15, 0, r0, c1, c0, 1
	isb
	bx		lr

ARM_FUNCTION _arm_write_dar

	mcr		p15, 0, r0, c3, c0, 0
	isb
	bx		lr

ARM_FUNCTION _arm_write_ttb

	mcr		p15, 0, r0, c2, c0, 0
	isb
	bx		lr

ARM_FUNCTION _arm_write_ttbcr

	mcr		p15, 0, r0, c2, c0, 2
	isb
	bx		lr

ARM_FUNCTION _arm_read_ifsr

	mrc		p15, 0, r0, c5, c0, 1
	bx		lr
	
ARM_FUNCTION _arm_read_dfsr

	mrc		p15, 0, r0, c5, c0, 0
	bx		lr

ARM_FUNCTION _arm_read_ifar

#if ARCH_ARMv7
	mrc		p15, 0, r0, c6, c0, 2
	bx		lr
#endif
ARM_FUNCTION _arm_read_dfar

	mrc		p15, 0, r0, c6, c0, 0
	bx		lr
	
ARM_FUNCTION _arm_read_cp_access_cr

	mrc		p15, 0, r0, c1, c0, 2
	bx		lr

ARM_FUNCTION _arm_write_cp_access_cr

	mcr		p15, 0, r0, c1, c0, 2
	isb
	bx		lr

#if WITH_VFP
ARM_FUNCTION _arm_read_fpexc

	vmrs		r0, fpexc
	bx		lr

ARM_FUNCTION _arm_write_fpexc

	vmsr		fpexc, r0
	bx		lr

ARM_FUNCTION _arm_read_fpscr

	vmrs		r0, fpscr
	bx		lr

ARM_FUNCTION _arm_write_fpscr

	vmsr		fpscr, r0
	bx		lr

// Reset core registers to a known value. Kingfisher doesnt
// do this on reset, and simulation traces will show 'x' 
// uninitialized values otherwise. Prefer deterministic
ARM_FUNCTION _arm_init_fp_regs

	vmov.i64	q0, #0
	vmov		q1, q0
	vmov		q2, q0
	vmov		q3, q0
	vmov		q4, q0
	vmov		q5, q0
	vmov		q6, q0
	vmov		q7, q0
#if FP_REGISTER_COUNT > 16
	vmov		q8, q0
	vmov		q9, q0
	vmov		q10, q0
	vmov		q11, q0
	vmov		q12, q0
	vmov		q13, q0
	vmov		q14, q0
	vmov		q15, q0
#endif
	bx		lr

#endif
	
#if ARCH_ARMv7
ARM_FUNCTION _arm_read_l2_aux_cr

	mrc		p15, 1, r0, c9, c0, 2
	bx		lr
	
ARM_FUNCTION _arm_write_l2_aux_cr

	mcr		p15, 1, r0, c9, c0, 2
	isb
	bx		lr

ARM_FUNCTION _arm_read_pmreg

	cmp		r0, #0
	bne		1f
	mrc		p15, 0, r0, c9, c12, 0
	b		2f
1:	cmp		r0, #1
	bne		1f
	mrc		p15, 0, r0, c9, c12, 1
	b		2f
1:	cmp		r0, #2
	bne		1f
	mrc		p15, 0, r0, c9, c12, 2
	b		2f
1:	cmp		r0, #3
	bne		1f
	mrc		p15, 0, r0, c9, c12, 3
	b		2f
1:	cmp		r0, #4
	bne		1f
	mrc		p15, 0, r0, c9, c12, 4
	b		2f
1:	cmp		r0, #5
	bne		1f
	mrc		p15, 0, r0, c9, c12, 5
	b		2f
1:	cmp		r0, #6
	bne		1f
	mrc		p15, 0, r0, c9, c13, 0
	b		2f
1:	cmp		r0, #7
	bne		1f
	mrc		p15, 0, r0, c9, c13, 1
	b		2f
1:	cmp		r0, #8
	bne		2f
	mrc		p15, 0, r0, c9, c13, 2
	b		2f
1:	cmp		r0, #9
	bne		2f
	mcr		p15, 0, r0, c9, c14, 0
	b		2f
1:	cmp		r0, #10
	bne		2f
	mcr		p15, 0, r0, c9, c14, 1
	b		2f
1:	cmp		r0, #11
	bne		2f
	mcr		p15, 0, r0, c9, c14, 2
2:	isb
	bx		lr

ARM_FUNCTION _arm_write_pmreg

	cmp		r0, #0
	bne		1f
	mcr		p15, 0, r1, c9, c12, 0
	b		2f
1:	cmp		r0, #1
	bne		1f
	mcr		p15, 0, r1, c9, c12, 1
	b		2f
1:	cmp		r0, #2
	bne		1f
	mcr		p15, 0, r1, c9, c12, 2
	b		2f
1:	cmp		r0, #3
	bne		1f
	mcr		p15, 0, r1, c9, c12, 3
	b		2f
1:	cmp		r0, #4
	bne		1f
	mcr		p15, 0, r1, c9, c12, 4
	b		2f
1:	cmp		r0, #5
	bne		1f
	mcr		p15, 0, r1, c9, c12, 5
	b		2f
1:	cmp		r0, #6
	bne		1f
	mcr		p15, 0, r1, c9, c13, 0
	b		2f
1:	cmp		r0, #7
	bne		1f
	mcr		p15, 0, r1, c9, c13, 1
	b		2f
1:	cmp		r0, #8
	bne		2f
	mcr		p15, 0, r1, c9, c13, 2
	b		2f
1:	cmp		r0, #9
	bne		2f
	mcr		p15, 0, r1, c9, c14, 0
	b		2f
1:	cmp		r0, #10
	bne		2f
	mcr		p15, 0, r1, c9, c14, 1
	b		2f
1:	cmp		r0, #11
	bne		2f
	mcr		p15, 0, r1, c9, c14, 2
2:	isb
	bx		lr
#endif /* ARCH_ARMv7 */

#if ALL_ROUTINES
ARM_FUNCTION _arm_read_user_rw_tid

	mrc		p15, 0, r0, c13, c0, 2
	bx		lr

ARM_FUNCTION _arm_write_user_rw_tid

	mcr		p15, 0, r0, c13, c0, 2
	isb
	bx		lr

ARM_FUNCTION _arm_read_user_ro_tid

	mrc		p15, 0, r0, c13, c0, 3
	bx		lr

ARM_FUNCTION _arm_write_user_ro_tid

	mcr		p15, 0, r0, c13, c0, 3
	isb
	bx		lr

ARM_FUNCTION _arm_read_sup_tid

	mrc		p15, 0, r0, c13, c0, 4
	bx		lr

ARM_FUNCTION _arm_write_sup_tid

	mcr		p15, 0, r0, c13, c0, 4
	isb
	bx		lr
#endif

ARM_FUNCTION _arm_read_perip_port_remap

	mrc		p15, 0, r0, c15, c2, 4
	bx		lr

ARM_FUNCTION _arm_write_perip_port_remap

	mcr		p15, 0, r0, c15, c2, 4
	isb
	bx		lr

ARM_FUNCTION _arm_flush_tlbs

	mov		r0, #0
	mcr		p15, 0, r0, c8, c7, 0
	dsb
	isb
	bx		lr

#define EXT(x) _ ## x

#if ALL_ROUTINES
/* mpu routines */
#define WRITE_DATA_PROT_REGION(func, n) \
	.global EXT(func); \
EXT(func): \
	mcr		p15, 0, r0, c6, c##n, 0; \
	isb
	bx		lr

WRITE_DATA_PROT_REGION(arm_write_dprot_region_0, 0)
WRITE_DATA_PROT_REGION(arm_write_dprot_region_1, 1)
WRITE_DATA_PROT_REGION(arm_write_dprot_region_2, 2)
WRITE_DATA_PROT_REGION(arm_write_dprot_region_3, 3)
WRITE_DATA_PROT_REGION(arm_write_dprot_region_4, 4)
WRITE_DATA_PROT_REGION(arm_write_dprot_region_5, 5)
WRITE_DATA_PROT_REGION(arm_write_dprot_region_6, 6)
WRITE_DATA_PROT_REGION(arm_write_dprot_region_7, 7)
WRITE_DATA_PROT_REGION(arm_write_dprot_region_8, 8)

#define WRITE_INSTRUCTION_PROT_REGION(func, n) \
	.global EXT(func); \
EXT(func): \
	mcr		p15, 0, r0, c6, c##n, 1; \
	isb
	bx		lr

WRITE_INSTRUCTION_PROT_REGION(arm_write_iprot_region_0, 0)
WRITE_INSTRUCTION_PROT_REGION(arm_write_iprot_region_1, 1)
WRITE_INSTRUCTION_PROT_REGION(arm_write_iprot_region_2, 2)
WRITE_INSTRUCTION_PROT_REGION(arm_write_iprot_region_3, 3)
WRITE_INSTRUCTION_PROT_REGION(arm_write_iprot_region_4, 4)
WRITE_INSTRUCTION_PROT_REGION(arm_write_iprot_region_5, 5)
WRITE_INSTRUCTION_PROT_REGION(arm_write_iprot_region_6, 6)
WRITE_INSTRUCTION_PROT_REGION(arm_write_iprot_region_7, 7)
WRITE_INSTRUCTION_PROT_REGION(arm_write_iprot_region_8, 8)

	.global _arm_write_data_prot_register
_arm_write_data_prot_register:
	mcr		p15, 0, r0, c5, c0, 0
	isb
	bx		lr

	.global _arm_write_ins_prot_register
_arm_write_ins_prot_register:
	mcr		p15, 0, r0, c5, c0, 1
	isb
	bx		lr

	.global _arm_write_cacheable_registers
_arm_write_cacheable_registers:
	mcr		p15, 0, r0, c2, c0, 0
	mcr		p15, 0, r1, c2, c0, 1
	isb
	bx		lr

	.global _arm_write_bufferable_register
_arm_write_bufferable_register:
	mcr		p15, 0, r0, c3, c0, 0
	isb
	bx		lr
#endif

/* uint arch_enable_ints(void); */
ARM_FUNCTION _arch_enable_ints

	mrs		r0, cpsr
	bic		r1, r0, #(1<<7)
#if WITH_FIQ_TIMER
	bic		r1, r1, #(1<<6)
#endif		
	msr		cpsr_c, r1
	and		r0, r0, #(1<<7)
	bx		lr

/* uint arch_disable_ints(void); */
ARM_FUNCTION _arch_disable_ints

	mrs		r0, cpsr
	orr		r1, r0, #(1<<7)
#if WITH_FIQ_TIMER
	orr		r1, r1, #(1<<6)
#endif	
	msr		cpsr_c, r1
	and		r0, r0, #(1<<7)
	bx		lr

/* uint arch_restore_ints(uint state); */
ARM_FUNCTION _arch_restore_ints

	mrs		r1, cpsr
	and		r0, r0, #(1<<7)
	bic		r1, r1, #(1<<7)
	orr		r2, r1, r0
	msr		cpsr_c, r2
	and		r0, r1, #(1<<7)
	bx		lr

/* void arm_enable_fiqs(void); */
ARM_FUNCTION _arm_enable_fiqs

	mrs		r0, cpsr
	bic		r0, r0, #(1<<6)
	msr		cpsr_c, r0
	bx		lr

/* void arm_disable_fiqs(void); */
ARM_FUNCTION _arm_disable_fiqs

	mrs		r0, cpsr
	orr		r0, r0, #(1<<6)
	msr		cpsr_c, r0
	bx		lr

#if ARCH_ARMv7
ARM_FUNCTION _arm_read_vbar

	mrc		p15, 0, r0, c12, c0, 0
	bx		lr
	
ARM_FUNCTION _arm_write_vbar

	mcr		p15,0, r0, c12, c0, 0
	bx		lr

#endif
	
#if ARCH_ARMv7
ARM_FUNCTION _arm_flush_branch_predictor

	mov		r0, #0
	mcr		p15, 0, r0, c7, c5, 6				/* flush the branch predictor cache */
	isb
	mov		pc, lr
#endif

/* void arm_memory_barrier(void); */
ARM_FUNCTION _arm_memory_barrier

#if ARCH_ARMv7 
	dmb
#endif
	bx		lr

#ifdef PRODUCT_EMBEDDEDIOP
/*  <rdar://problem/16290859> WFI errata workaround for Cortex-A5 */
	dummy_store:
	.long 		0xaaaabbbb

	.balign		32
#endif //PRODUCT_EMBEDDEDIOP

ARM_FUNCTION _arch_halt

#if !NO_ARM_HALT
# if ARCH_ARMv7
	dsb
#ifdef PRODUCT_EMBEDDEDIOP
	str		r0, dummy_store
#endif //PRODUCT_EMBEDDEDIOP
	wfi
# else
	mov		r0, #0
	mcr		p15, 0, r0, c7, c0, 4				/* wait for interrupt */
# endif
#endif
	bx		lr

ARM_FUNCTION _arch_spin

#if ARCH_ARMv7 || ARCH_ARMv8
	wfe
#endif
	b		_arch_spin

#if WITH_ARM_DCC
ARM_FUNCTION _read_dtr

	mrc		p14, 0, r0, c0, c5, 0
	bx		lr

ARM_FUNCTION _write_dtr

	mcr		p14, 0, r0, c0, c5, 0
	isb
	bx		lr

ARM_FUNCTION _read_dscr

	mrc		p14, 0, r0, c0, c1, 0
	bx		lr
#endif /* WITH_ARM_DCC */


#if CPU_APPLE_SWIFT
ARM_FUNCTION _swift_read_l2cerrsts

	mrc		p15, 1, r0, c15, c2, 5
	bx		lr

ARM_FUNCTION _swift_write_l2cerrsts

	mcr		p15, 1, r0, c15, c2, 5
	isb
	bx		lr

ARM_FUNCTION _swift_read_l2cerradr

	mrc		p15, 1, r0, c15, c2, 7
	bx		lr

ARM_FUNCTION _swift_write_l2cerradr

	mcr		p15, 1, r0, c15, c2, 7
	isb
	bx		lr
#endif /* CPU_APPLE_SWIFT */
