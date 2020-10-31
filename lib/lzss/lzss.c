/*
 * Copyright (c) 2007-2013 Apple Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 * 
 * The contents of this file constitute Original Code as defined in and
 * are subject to the Apple Public Source License Version 1.1 (the
 * "License").  You may not use this file except in compliance with the
 * License.  Please obtain a copy of the License at
 * http://www.apple.com/publicsource and read it before using this file.
 * 
 * This Original Code and all software distributed under the License are
 * distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE OR NON-INFRINGEMENT.  Please see the
 * License for the specific language governing rights and limitations
 * under the License.
 * 
 * @APPLE_LICENSE_HEADER_END@
 */

#include <sys.h>
#include <arch.h>
#include <lib/lzss.h>

#include <string.h>
#include <stdlib.h>

uint32_t decompress_lzss_vec(uint8_t *dst, uint32_t dstlen, const uint8_t *src, uint32_t srclen);

#define N         4096          /* size of ring buffer - must be power of 2 */
#define F         18            /* upper limit for match_length */
#define THRESHOLD 2             /* encode string into position and length
                                   if match_length is greater than this */
#define NIL       N             /* index for root of binary search trees */
#define TEXT_BUF  (N + F - 1)   /* size of ring buffer with F-1 extra bytes
                                   to aid string comparison */

uint32_t decompress_lzss(uint8_t * __restrict dst, uint32_t dstlen, const uint8_t * __restrict src, uint32_t srclen)
{
#if (ARCH_ARMv7 && WITH_VFP) || defined(__arm64__)
#if WITH_VFP_ALWAYS_ON
    return decompress_lzss_vec(dst, dstlen, src, srclen);
#else
    bool fp_enabled;
    uint32_t result;

    fp_enabled = arch_task_fp_enable(true);
    result = decompress_lzss_vec(dst, dstlen, src, srclen);
    arch_task_fp_enable(fp_enabled);
    return result;
#endif // WITH_VFP_ALWAYS_ON
#else
    uint8_t *text_buf = NULL;
    uint8_t *dststart = dst;
    uint8_t *dstend = dst + dstlen;
    const uint8_t *srcend = src + srclen;
    uint32_t i, j, k, r, c;
    uint32_t flags;
    uint32_t result = 0;

    text_buf = (uint8_t *)malloc(TEXT_BUF);
    if (text_buf == NULL)
        goto error;
    memset(text_buf, ' ', N - F);

    r = N - F;
    flags = 0;
    for ( ; ; ) {
        if (((flags >>= 1) & 0x100) == 0) {
            if (src < srcend) {
                c = *src++;
            } else {
                    break;
            }
            flags = c | 0xFF00;  /* uses higher byte cleverly */
        }                        /* to count eight */
        if (flags & 1) {
            if (src < srcend) {
                    c = *src++;
            } else {
                    break;
            }
            if (dst == dstend) {
                    goto error;
            }
            *dst++ = c;
            text_buf[r++] = c;
            r &= (N - 1);
        } else {
            if (src < srcend) {
                    i = *src++;
            } else {
                    break;
            }
            if (src < srcend) {
                    j = *src++;
            } else {
                    break;
            }
            i |= ((j & 0xF0) << 4);
            j  =  (j & 0x0F) + THRESHOLD;
            for (k = 0; k <= j; k++) {
                c = text_buf[(i + k) & (N - 1)];
                if (dst == dstend) {
                        goto error;
                }
                *dst++ = c;
                text_buf[r++] = c;
                r &= (N - 1);
            }
        }
    }

    result = dst - dststart;

error:
    if (text_buf != NULL)
        free(text_buf);

    return result;
#endif
}
