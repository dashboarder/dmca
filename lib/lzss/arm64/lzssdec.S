/*
 * Copyright (C) 2013-2014 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */

#ifdef __arm64__

	.text
	.align 2

#define N         4096  /* size of ring buffer - must be power of 2 */
#define F         18    /* upper limit for match_length */
#define THRESHOLD 2     /* encode string into position and length if match_length is greater than this */

	.globl _decompress_lzss_vec
_decompress_lzss_vec:

// int decompress_lzss(
//    u_int8_t       * dst,		- x0
//    u_int32_t        dstlen,	- x1 never used
//    u_int8_t       * src,		- x2 
//    u_int32_t        srclen)	- x3 (used once to define srcend)
// {
//    u_int8_t text_buf[N + F - 1];
//    u_int8_t * dststart = dst;
//    const u_int8_t * srcend = src + srclen;
//    int  i, j, k, r, c;
//    unsigned int flags;

	#define	dststart	x12
	#define	dst			x0
	#define	flags		w1
	#define	src			x2
	#define	srcend		x3
	#define	r			w4
	#define	c			w5
	#define	i			w6
	#define	j			w7
	#define	t			w8
	#define	k			w10
	#define	text_buf	sp
	#define	Nm1			w11
	#define	spsave		x9

//    dst = dststart;

	mov		spsave, sp				// store sp as we need to align sp
	mov		dststart, dst

//    for (i = 0; i < N - F; i++) text_buf[i] = ' ';

	lsr		x1, spsave, #4			// 16-byte alignment

	mov		w5, #((32<<8)+32)		// r5 = ' ';
	lsl		x1, x1, #4				// 16-byte alignment
	sub		sp, x1, #N				// it appears that text_buf[N] is sufficient, the extra F-1 bytes is never used
	add		w5, w5, w5, lsl #16		// r5 = '    ';
	mov		x4, sp					// r1 -> text_buf[0]

	mov		x1, #(N-F+2)			// do extra 2, so that 4080 is a multiple of 16 (255)

	dup.4s	v0, w5

0:
	st1.4s	{v0}, [x4]
	add		x4, x4, #16
	subs	x1, x1, #16
	b.gt	0b

//    r = N - F;
//    flags = 0;


	mov		w4, #N
	sub		w11, w4, #1					// r11 = N-1, will be used for circular buffering in text_buf[r]
	sub		w4, w4, #F						// r = N-r
	mov		w1, #0						// flag = 0

	add		srcend, src, x3	

L_for_loop:
	
//    for ( ; ; ) {

//        if (((flags >>= 1) & 0x100) == 0) {
//            if (src < srcend) c = *src++; else break;
//            flags = c | 0xFF00;
//        }

	lsr		flags, flags, #1			// flags >>= 1
	tst		flags, #0x100				// (flags >>= 1) & 0x100)
	b.ne	L_skip1						// if != 0, skip the following 
	cmp		src, srcend					// src vs srcend
	b.ge	L_break_for_loop				// if (src>=srcend) break_for_loop	
	ldrb	c,[src],#1					// c = *src++
	orr		flags,c,#0x00ff00			// flags = c | 0xFF00;

L_skip1:

//        if (flags & 1) {
	tst		flags, #1
	b.eq		L_skip2
//            if (src < srcend) c = *src++; else break;
//            *dst++ = c;
//            text_buf[r++] = c;
//            r &= (N - 1);

	cmp		src, srcend					// src vs srcend 
	b.ge	L_break_for_loop			// if (src>=srcend) break_for_loop	
	ldrb	c,[src],#1					// c = *src++
	strb	c,[text_buf,r, uxtw]				// text_buf[r]
	add		r, r, #1					// r++
	strb	c,[dst],#1					// *dst++ = c;
	and		r, r, Nm1					// r &= (N - 1);
	b		L_for_loop

//        } else {
L_skip2:
//            if (src < srcend) i = *src++; else break;
//            if (src < srcend) j = *src++; else break;

	ldrh	i,[src],#2					//
	cmp		src, srcend					//
	lsr		j, i, #8
	b.gt	L_break_for_loop				
	sub		i, i, j, lsl #8	

//            i |= ((j & 0xF0) << 4);
//            j  =  (j & 0x0F) + THRESHOLD;

	and		t, j, #0xf0					// j&0xf0
	and		j, j, #0x0f					// j&0x0f	
	orr		i, i, t, lsl #4				// i |= ((j & 0xF0) << 4);
	add		j, j, #THRESHOLD			// j  =  (j & 0x0F) + THRESHOLD;

//            for (k = 0; k <= j; k++) {
//                c = text_buf[(i + k) & (N - 1)];
//                *dst++ = c;
//                text_buf[r++] = c;
//                r &= (N - 1);
//            }
	mov		k, #0

0:
	add		t, i, k						// (i+k)
	and		t, t, Nm1					// (i + k) & (N - 1)
	ldrb	c, [text_buf, t, uxtw]			// c = text_buf[(i + k) & (N - 1)];
	strb	c, [dst], #1				// *dst++ = c;
	strb	c, [text_buf, r, uxtw]			// text_buf[r] = c;
	add		r, r, #1					// r++
	and		r, r, Nm1					// r &= (N - 1);
	add		k, k, #1					// k++
	cmp		k, j						// k vs j
	b.le	0b
	b		L_for_loop

//        }
//    }

L_break_for_loop:
	sub		x0, dst, dststart

	mov		sp, spsave				// stores sp before 16-byte alignment

	ret		lr



//    return dst - dststart;
// }

#endif	// __arm64__

