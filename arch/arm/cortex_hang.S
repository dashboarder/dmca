/*
 * Copyright (C) 2009 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */

	.text
	.global _cortex_hang_start, _cortex_hang_ffe, _cortex_hang_end
	.thumb
	.syntax unified

// Believe it or not, without this, it does not fail
#define NOP	mov r8,r8

	// R0 holds the number of iterations we should run
	// start this routine at a page boundary
_cortex_hang_start:
	// save some registers and set aside a word
	push	{r4,r5,r6,r7,lr}
	sub	sp, #4
1:
	cmp	r0, #0
	beq	1f
	NOP			// 0xff0
	NOP			// 0xff2
	NOP			// 0xff4
	NOP			// 0xff6
	subs	r0, #1		// 0xff8
	str.w	ip, [sp]	// 0xffa
_cortex_hang_ffe:
	bl	1b		// 0xffe
1:
	add	sp, #4
	pop	{r4,r5,r6,r7,pc}
	bx	lr
_cortex_hang_end:
