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
#include <debug.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdarg.h>

#if defined(snprintf)
#undef snprintf
#endif
#if defined(vsnprintf)
#undef vsnprintf
#endif

int
__snprintf_chk(char *dst, size_t size, int flags __unused, size_t chk_size, const char *fmt, ...)
{
	int result;
	va_list ap;

	if (unlikely(chk_size < size))
		panic("object size check failed (%zu < %zu)", size, chk_size);

	va_start(ap, fmt);
	result = vsnprintf(dst, size, fmt, ap);
	va_end(ap);

	return result;
}

int
__vsnprintf_chk(char *dst, size_t size, int flags __unused, size_t chk_size, const char *fmt, va_list ap)
{
	if (unlikely(chk_size < size))
		panic("object size check failed (%zu < %zu)", size, chk_size);

	return vsnprintf(dst, size, fmt, ap);
}
