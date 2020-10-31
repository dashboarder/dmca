/*
 * Copyright (C) 2007-2009 Apple Inc. All rights reserved.
 * Copyright (C) 2006 Apple Computer, Inc. All rights reserved.
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
#include <sys.h>

#if defined(strlcat)
#undef strlcat
#endif

size_t
strlcat(char *dst, char const *src, size_t s)
{
	size_t i;
	size_t j= strnlen(dst, s);

	if(!s || (s== j)) {
		return j+strlen(src);
	}

	dst+= j;
	s-= j;

	for(i= 0; ((i< s-1) && src[i]); i++) {
		dst[i]= src[i];
	}

	dst[i]= 0;

	return j + i + strlen(src+i);
}
