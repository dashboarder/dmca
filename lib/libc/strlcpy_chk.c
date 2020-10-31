/*
 * Copyright (C) 2014 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Computer, Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Computer, Inc.
 */

#include <compiler.h>
#include <string.h>
#include <sys/types.h>
#include <sys.h>

#if defined(strlcpy)
#undef strlcpy
#endif

size_t
__strlcpy_chk(char *dst, char const *src, size_t s, size_t chk_size)
{
	if (unlikely(chk_size < s))
		panic("object size check failed (%zu < %zu)", s, chk_size);

	return strlcpy(dst, src, s);
}
