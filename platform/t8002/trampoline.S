
/*
 * Copyright (c) 2014 - 2015 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */
#include <arch/arm/assembler.h>
#include <platform/trampoline.h>

// iBSS & LLB trampoline
#if APPLICATION_IBOOT && (PRODUCT_IBSS || PRODUCT_LLB)
	.text
	.arm
	.align	12
	.globl _boot_handoff_trampoline, _boot_handoff_trampoline_end
_boot_handoff_trampoline:
	// Reset VBAR to 0
	mov	r1, #0				// iBSS and LLB doesn't pass forward anything to next stage.
	mcr     p15, 0, r1, c12, c0, 0
	isb
	bx	r0
_boot_handoff_trampoline_end:
#endif

// SecureROM trampoline
#if WITH_ROM_TRAMPOLINE
	.text
	.arm
	.align	12
	.globl _boot_handoff_trampoline, _boot_handoff_trampoline_end
_boot_handoff_trampoline:
	ldr	r1, _trampoline_reg
	ldr	r2, _trampoline_val
	ldr	r3, [r1]
	orr	r3, r2
	str	r3, [r1]
	dsb	sy
	ldr	r3, [r1]
	tst	r3, r2		// Verify ROM access is disabled
	beq	1f
	
	mov	r4, r0
	mov	r0, #0
	mov	r1, #0
	mov	r2, #0
	mov	r3, #0
	bx	r4

1:				// trampoline spin function
	wfi
	b	.

_trampoline_reg:
	.word	SECURITY_REG
_trampoline_val:
	.word	ROM_READ_DISABLE
	.align	CPU_CACHELINE_SHIFT
_boot_handoff_trampoline_end:
#endif
