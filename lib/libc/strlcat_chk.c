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
#include <sys.h>

#if defined(strlcat)
#undef strlcat
#endif

size_t
__strlcat_chk(char *dst, char const *src, size_t s, size_t chk_size)
{
	if (unlikely(chk_size < s))
		panic("object size check failed (%zu < %zu)", s, chk_size);

	return strlcat(dst, src, s);
}
