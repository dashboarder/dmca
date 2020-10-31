/*
 * Copyright (C) 2010, 2014 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */

#include <stdint.h>
#include <stdarg.h>

extern void*	heap_malloc(size_t size, const char *caller_name);
extern void*	heap_calloc(size_t count, size_t size, const char *caller_name);
extern void*	heap_realloc(void *ptr, size_t size, const char *caller_name);
extern void	heap_free(void *ptr);
extern void	bzero(void *s, size_t n);
extern void*	memcpy(void* dest, const void* src, size_t len);
extern void*	memset(void* dest, int val, size_t len);

/* Global stderrp definition */
void* __stderrp = 0;


void* malloc(size_t size)
{
	return heap_malloc(size, NULL);
}

void* realloc(void *ptr, size_t size)
{
	return heap_realloc(ptr, size, NULL);
}

void* calloc(size_t count, size_t size)
{
	return heap_calloc(count, size, NULL);
}

void free(void *ptr)
{
	heap_free(ptr);
}

void* __memcpy_chk(void* dest, const void* src, size_t len, size_t destSize)
{
	memcpy(dest, src, (len > destSize) ? destSize : len);
	return dest;
}

void* __memset_chk(void* dest, int val, size_t len, size_t destSize)
{
	memset(dest, val, (len > destSize) ? destSize : len);
	return dest;
}
