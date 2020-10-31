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
#ifndef __HEAP_H
#define __HEAP_H

#include <sys/types.h>

__BEGIN_DECLS

/*
 * The public heap allocation functions will never return NULL. The internal
 * implementation will always call panic() if an allocation cannot be
 * satisfied. You should not check for NULL being returned as this is a waste
 * of code and gives other readers the incorrect impression that this is
 * required.
 */

#define HEAP_COOKIE_SIZE	16

void heap_add_chunk(void *chunk, size_t size, bool clear_memory);
void heap_set_cookie(uint8_t *cookie);
u_int32_t heap_get_free_mem(void);
bool heap_is_initialized(void);

void *heap_malloc(size_t size, const char *caller_name);
void *heap_realloc(void *ptr, size_t size, const char *caller_name);
void *heap_memalign(size_t size, size_t alignment, const char *caller_name);
int heap_posix_memalign(void **memptr, size_t size, size_t alignment, const char *caller_name);
void *heap_calloc(size_t count, size_t size, const char *caller_name);
void heap_free(void *ptr);
void heap_verify(void);

#ifndef __clang_analyzer__
#ifndef HEAP_UNITTEST
# if DEBUG_BUILD
#  define malloc(_s)		heap_malloc(_s, __FUNCTION__)
#  define realloc(_p, _s)	heap_realloc(_p, _s, __FUNCTION__)
#  define memalign(_s, _a)	heap_memalign(_s, _a, __FUNCTION__)
#  define posix_memalign(_m, _s, _a)	heap_posix_memalign(_m, _s, _a, __FUNCTION__)
#  define calloc(_c, _s)	heap_calloc(_c, _s, __FUNCTION__)
# else
#  define malloc(_s)		heap_malloc(_s, NULL)
#  define realloc(_p, _s)	heap_realloc(_p, _s, NULL)
#  define memalign(_s, _a)	heap_memalign(_s, _a, NULL)
#  define posix_memalign(_m, _s, _a)	heap_posix_memalign(_m, _s, _a, NULL)
#  define calloc(_c, _s)		heap_calloc(_c, _s, NULL)
# endif
# define free(_p)		heap_free(_p)
#endif // HEAP_UNITTEST
#else // __clang_analyzer__
// the clang static analyzer needs malloc and friends to have their normal
// names with underscores prepended in order to find things like use after free
void *malloc(size_t size);
void *realloc(void *ptr, size_t size);
void *memalign(size_t size, size_t alignment);
void *calloc(size_t count, size_t size);
void free(void *ptr);
int posix_memalign(void **memptr, size_t alignment, size_t size);
#endif

/* heap debugging features */
enum
{
	HEAP_BLOCK__FREE,
	HEAP_BLOCK__MALLOC,
	HEAP_BLOCK__MEMALIGN,
	HEAP_BLOCK__SENTINEL,
};

struct walker_info_t
{
	unsigned  block_type;
	void     *raw_block;
	size_t    block_size;
	unsigned  extended_info;
	uint32_t  checksum;

	/* these are only meaningful for HEAP_BLOCK__MALLOC & HEAP_BLOCK__MEMALIGN */
	void     *user_block;
	size_t    user_size;  /* only when extended_info != 0 */
	utime_t   timestamp;  /* only when extended_info != 0 */
	void     *requestor;  /* only when extended_info != 0 */

	/* this is only meaningful for HEAP_BLOCK__MEMALIGN, set to zero for HEAP_BLOCK__MALLOC */
	size_t    alignment;  /* only when extended_info != 0 */
};

typedef int (heap_walker_f)(void *cookie, struct walker_info_t const *);

__END_DECLS

#endif

