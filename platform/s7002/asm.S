/*
 * Copyright (c) 2014 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */
#include <arch/arm/assembler.h>

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

	.text
	.arm
	.align	12
	.globl _platform_start
_platform_start:
	// Use VBAR to relocate exception vectors to binary linked address
	mcr     p15, 0, r0, c12, c0, 0
	isb
        bx	lr

#endif
