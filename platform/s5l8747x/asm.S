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
#include <platform/memmap.h>

	.text

ARM_FUNCTION _platform_start
	
#if WITH_PROFILE && PROFILE_BUF_SIZE > 0
	// Turn on PCLK to TIM64
	mov		r2, #0x0200
	movt		r2, #0x3c50
	ldr		r3, [r2, #4]
	bic		r3, #(1 << 30)
	str		r3, [r2, #4]
	
	// Turn on NCLK to TIM64
	ldr		r3, [r2, #0x10]
	bic		r3, #(1 << 8)
	str		r3, [r2, #0x10]

	// Enable TIM64
	mov		r2, #0x0088
	movt		r2, #0x3c70
	// r3 = (r<<15)|(1<<4)
	mov		r3, #((1 << 15) | (1 << 4))
	movt		r3, #1
	str		r3, [r2]
#endif

	bx		lr

	
