/*
 * Copyright (C) 2006 Apple Computer, Inc. All rights reserved.
 * Copyright (C) 2007-2012 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Computer, Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */
#include <sys.h>
#include <arch.h>
#include <sys/types.h>
#include <lib/cksum.h>

#if (ARCH_ARMv7 && WITH_VFP) || defined(__arm64__)
extern uint32_t adler32_vec(uint32_t, uint32_t, const uint8_t *, long);
#endif


#define BASE 65521L /* largest prime smaller than 65536 */
#define NMAX 5000  
// NMAX (was 5521) the largest n such that 255n(n+1)/2 + (n+1)(BASE-1) <= 2^32-1

#define DO1(buf,i)  {s1 += buf[i]; s2 += s1;}
#define DO2(buf,i)  DO1(buf,i); DO1(buf,i+1);
#define DO4(buf,i)  DO2(buf,i); DO2(buf,i+2);
#define DO8(buf,i)  DO4(buf,i); DO4(buf,i+4);
#define DO16(buf)   DO8(buf,0); DO8(buf,8);

uint32_t adler32(const uint8_t *buf, long len)
{
    uint32_t s1 = 1; // adler & 0xffff;
    uint32_t s2 = 0; // (adler >> 16) & 0xffff;
    int k;

#if (ARCH_ARMv7 && WITH_VFP) || defined(__arm64__)

	/* align buf to 16-byte boundary */
	while ((int)buf & 15) { /* not on a 16-byte boundary */
		len--;
		s1 += *buf++;
		s2 += s1;
		s1 %= BASE;
		s2 %= BASE;
	}
	if (len > 16) {
#if WITH_VFP_ALWAYS_ON
		return adler32_vec(s1, s2, buf, len);
#else
		bool fp_enabled;
		uint32_t result;

		fp_enabled = arch_task_fp_enable(true);
		result = adler32_vec(s1, s2, buf, len);
		arch_task_fp_enable(fp_enabled);

		return result;
#endif // WITH_VFP_ALWAYS_ON
	}
#endif

    while (len > 0) {
        k = len < NMAX ? len : NMAX;
        len -= k;
        while (k >= 16) {
            DO16(buf);
	    buf += 16;
            k -= 16;
        }
        if (k != 0) do {
            s1 += *buf++;
	    s2 += s1;
        } while (--k);
        s1 %= BASE;
        s2 %= BASE;
    }
    return (s2 << 16) | s1;
}

