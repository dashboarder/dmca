/*
 * Copyright (C) 2012 Apple Inc. All rights reserved.
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
	.balign 4
	.globl _arch_get_entropy

//	u_int32_t arch_get_entropy(volatile u_int32_t *counter_address)
//
//	arch_get_entropy exploits the uncertainty in the asynchronous boundary crossing
//	between the CPU and a counter.  To work correctly, the CPU and counter must be
//	derived from clocks with relative jitter (PLL vs. OSC).  Ideally the inner loop,
//	up to 32 iterations, should consume a large part of the effective read period of
//	the counter.
//

_arch_get_entropy:

	mov	x1, x0
	mov	w0, #0
	mov	w2, #32
1:
	mov	w3, w2
2:
	subs	w3, w3, #1
	b.ne	2b
	ldr	w3, [x1]
	and	w3, w3, #1
	orr	w0, w3, w0, lsl #1
	subs	w2, w2, #1
	b.ne	1b
	ret
