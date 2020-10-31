/*
 * Copyright (C) 2012 Apple Computer, Inc. All rights reserved.
 *
 * This document is the property of Apple Computer, Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Computer, Inc.
 */

/* void memcpy_16(uint8_t *dest, const void * src, size_t count); */
	.text
	.align 2
	.globl _memcpy_16
_memcpy_16:
	mov	x3, x0
	mov	x4, x1
L__memcpy_16_loop:
	ldp	x5, x6, [x4], #16
	stp	x5, x6, [x3], #16
	subs	x2, x2, #16
	b.ne	L__memcpy_16_loop
	ret
