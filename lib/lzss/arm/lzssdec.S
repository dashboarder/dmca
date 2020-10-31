/*
 * Copyright (C) 2011-2012, 2014 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */

#ifdef __arm__

	.text
	.syntax unified

	.text
	.align 2

#define N         4096  /* size of ring buffer - must be power of 2 */
#define F         18    /* upper limit for match_length */
#define THRESHOLD 2     /* encode string into position and length if match_length is greater than this */

	.globl _decompress_lzss_vec
_decompress_lzss_vec:

// int decompress_lzss(
//    u_int8_t       * dst,		- r0
//    u_int32_t        dstlen,	- r1 never used
//    u_int8_t       * src,		- r2 
//    u_int32_t        srclen)	- r3 (used once to define srcend)
// {
//    u_int8_t text_buf[N + F - 1];
//    u_int8_t * dststart = dst;
//    const u_int8_t * srcend = src + srclen;
//    int  i, j, k, r, c;
//    unsigned int flags;

//    dst = dststart;

	mov		r12, r0

//    for (i = 0; i < N - F; i++) text_buf[i] = ' ';

	push	{r4-r8,r10-r11,lr}

	mov		lr, sp				// store sp in lr, need to aligned
	mov		r4, #0xfffffff0
	and		sp, r4				// 16-byte alignment

	mov		r5, #32				// r5 = ' ';
	sub		sp, #N				// it appears that text_buf[N] is sufficient, the extra F-1 bytes is never used

	add		r5, r5, r5, lsl #8		// r5 = '  ';
	mov		r4, sp				// r1 -> text_buf[0]
	add		r5, r5, r5, lsl #16		// r5 = '    ';

	mov		r1, #(N-F+2)		// do extra 2, so that 4080 is a multiple of 16 (255)

#if WITH_VFP
	vdup.32	q0, r5
#else
	mov		r6, r5
	mov		r7, r5
	mov		r8, r5
#endif

loop0:
#if WITH_VFP
	vst1.f32	{q0}, [r4,:128]!
#else
	stmia		r4!, {r5-r8}
#endif
	subs	r1, #16
	bgt		loop0

//    r = N - F;
//    flags = 0;


	mov		r4, #N
	sub		r11, r4, #1					// r11 = N-1, will be used for circular buffering in text_buf[r]
	sub		r4, #F						// r = N-r
	mov		r1, #0						// flag = 0

	#define	dststart	r12
	#define	dst			r0
	#define	flags		r1
	#define	src			r2
	#define	srcend		r3
	#define	r			r4
	#define	c			r5
	#define	i			r6
	#define	j			r7
	#define	t			r8
	#define	k			r10
	#define	text_buf	sp
	#define	Nm1			r11

	add		srcend, src, r3	

for_loop:
	
//    for ( ; ; ) {

//        if (((flags >>= 1) & 0x100) == 0) {
//            if (src < srcend) c = *src++; else break;
//            flags = c | 0xFF00;
//        }

	lsr		flags, #1					// flags >>= 1
	tst		flags, #0x100				// (flags >>= 1) & 0x100)
	bne		skip1						// if != 0, skip the following 
	cmp		src, srcend					// src vs srcend
	bge		break_for_loop				// if (src>=srcend) break_for_loop	
	ldrb	c,[src],#1					// c = *src++
	orr		flags,c,#0x00ff00			// flags = c | 0xFF00;

skip1:

//        if (flags & 1) {
	tst		flags, #1
	beq		skip2
//            if (src < srcend) c = *src++; else break;
//            *dst++ = c;
//            text_buf[r++] = c;
//            r &= (N - 1);

	cmp		src, srcend					// src vs srcend 
	bge		break_for_loop				// if (src>=srcend) break_for_loop	
	ldrb	c,[src],#1					// c = *src++
	strb	c,[text_buf,r]				// text_buf[r]
	add		r, #1						// r++
	strb	c,[dst],#1					// *dst++ = c;
	and		r, Nm1						// r &= (N - 1);
	b		for_loop

//        } else {
skip2:
//            if (src < srcend) i = *src++; else break;
//            if (src < srcend) j = *src++; else break;

#if (0)
	cmp		src, srcend					// src vs srcend 
	bge		break_for_loop				// if (src>=srcend) break_for_loop	
	ldrb	i,[src],#1					// i = *src++
	cmp		src, srcend					// src vs srcend 
	bge		break_for_loop				// if (src>=srcend) break_for_loop	
	ldrb	j,[src],#1					// j = *src++
#else
	ldrh	i,[src],#2					//
	cmp		src, srcend					//
	lsr		j, i, #8
	bgt		break_for_loop				
	sub		i, i, j, lsl #8	
	
#endif

//            i |= ((j & 0xF0) << 4);
//            j  =  (j & 0x0F) + THRESHOLD;

	and		t, j, #0xf0					// j&0xf0
	and		j, #0x0f					// j&0x0f	
	orr		i, i, t, lsl #4				// i |= ((j & 0xF0) << 4);
	add		j, #THRESHOLD				// j  =  (j & 0x0F) + THRESHOLD;

//            for (k = 0; k <= j; k++) {
//                c = text_buf[(i + k) & (N - 1)];
//                *dst++ = c;
//                text_buf[r++] = c;
//                r &= (N - 1);
//            }
	mov		k, #0

k_loop:
	add		t, i, k						// (i+k)
	and		t, Nm1						// (i + k) & (N - 1)
	ldrb	c, [text_buf, t]			// c = text_buf[(i + k) & (N - 1)];
	strb	c, [dst], #1				// *dst++ = c;
	strb	c, [text_buf, r]			// text_buf[r] = c;
	add		r, #1						// r++
	and		r, Nm1						// r &= (N - 1);
	add		k, #1						// k++
	cmp		k, j						// k vs j
	ble		k_loop
	b		for_loop

//        }
//    }

break_for_loop:
	sub		dst, dststart

	mov		sp, lr				// lr stores sp before 16-byte alignment
	pop		{r4-r8,r10-r11,pc}



//    return dst - dststart;
// }

#endif	// __arm__

