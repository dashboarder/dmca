/*
 * Copyright (C) 2006 Apple Computer, Inc. All rights reserved.
 *
 * This document is the property of Apple Computer, Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Computer, Inc.
 */

#include <string.h>
#include <sys/types.h>
#include <sys.h>

#if defined(strlcpy)
#undef strlcpy
#endif

size_t
strlcpy(char *dst, char const *src, size_t s)
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
