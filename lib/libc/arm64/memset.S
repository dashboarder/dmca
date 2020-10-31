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

/* void memset_16(uint8_t *ptr, uint8_t c, size_t count); */
	.text
	.align 2
	.globl _memset_16
_memset_16:
	mov	x3, x0
	cbz	x1, L__memset_16_loop
	orr	x1, x1, x1, lsl #8
	orr	x1, x1, x1, lsl #16
	orr	x1, x1, x1, lsl #32
L__memset_16_loop:
	stp	x1, x1, [x3], #16
	subs	x2, x2, #16
	b.gt	L__memset_16_loop
	ret

/* void bzero_64(uint8_t *ptr, size_t count); */
	.text
	.align 2
	.globl _bzero_64
_bzero_64:
	mov	x2, x0
L__bzero_64_loop:
	dc		zva, x2
	add		x2, x2, #64
	subs		x1, x1, #64
	b.gt		L__bzero_64_loop
	ret
