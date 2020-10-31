/*
 * Copyright (C) 2010 Apple Inc. All rights reserved.
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

//	u_int32_t arch_get_entropy(volatile u_int32_t *counter_address)
//
//	arch_get_entropy exploits the uncertainty in the asynchronous boundary crossing
//	between the CPU and a counter.  To work correctly, the CPU and counter must be
//	derived from clocks with relative jitter (PLL vs. OSC).  Ideally the inner loop,
//	up to 32 iterations, should consume a large part of the effective read period of
//	the counter.
//

ARM_FUNCTION _arch_get_entropy

	mov	r1, r0
	mov	r0, #0
	mov	r2, #32
1:
	mov	r3, r2
2:
	subs	r3, r3, #1
	bne	2b
	ldr	r3, [r1]
	and	r3, r3, #1
	orr	r0, r3, r0, lsl #1
	subs	r2, r2, #1
	bne	1b
	bx	lr
