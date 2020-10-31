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

size_t
strxfrm(char *dest, const char *src, size_t n)
{
	size_t len = strlen(src);

	if(n) {
		size_t copy_len = len < n ? len : n - 1;
		memcpy(dest, src, copy_len);
		dest[copy_len] = 0;
	}
	return len;
}

