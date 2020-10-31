/*
* Copyright (C) 2014 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */
#include <sys/types.h>
#include <string.h>
#include <linux-support.h>

size_t strlcpy(char * restrict dst, const char * restrict src, size_t s)
{
	size_t i= 0;

	if(!s) {
		return strlen(src);
	}

	for(i= 0; ((i< s-1) && src[i]); i++) {
		dst[i]= src[i];
	}

	dst[i]= 0;

	return i + strlen(src+i);
}

size_t strlcat(char * restrict dst, const char * restrict src, size_t maxlen)
{
	const size_t srclen = strlen(src);
	const size_t dstlen = strnlen(dst, maxlen);
	if (dstlen == maxlen) return maxlen+srclen;
	if (srclen < maxlen-dstlen) {
		memcpy(dst+dstlen, src, srclen+1);
	} else {
		memcpy(dst+dstlen, src, maxlen-dstlen-1);
		dst[maxlen-1] = '\0';
	}
	return dstlen + srclen;
}
