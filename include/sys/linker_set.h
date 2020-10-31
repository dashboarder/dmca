/*
 * Copyright (C) 2008 Apple Computer, Inc. All rights reserved.
 *
 * This document is the property of Apple Computer, Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Computer, Inc.
 */

/*
 * Linker sets.
 */

#ifndef __LINKER_SET_H__
#define __LINKER_SET_H__

#include <compiler.h>

/*
 * Macro evil to generate the awkward Mach-O section specifier format.
 *
 * These macros should never be used outside this file.
 *
 * The end result is a line of the form
 *
 * void const *__set_<_set>_sym_<_sym> __attribute ((section("__Data,<_set>"))) = (void *)&<_sym>
 */
#ifndef __linux__
# define __LS_VA_STRINGIFY(_x...)	#_x
# define __LS_VA_STRCONCAT(_x,_y)	__LS_VA_STRINGIFY(_x,_y)
# define __PUT_IN_SECTION(_seg, _sec)	__attribute__ ((section(__LS_VA_STRCONCAT(_seg,_sec))))
#else
# define __PUT_IN_SECTION(_seg, _sec) __attribute__ ((section (#_sec)))
#endif
#define __LINKER_MAKE_SET(_set,_sym)					\
	void const *__set_##_set##_sym_##_sym				\
	__PUT_IN_SECTION(__TEXT,_set)					\
	__attribute__ ((used))						\
		= (void *) &_sym

// The address sanitizer's metadata results in the size of each global being
// bloated by a factor of 8
#if !__has_feature(address_sanitizer)
#define __LINKER_SET_STRIDE	(1)
#else
#define __LINKER_SET_STRIDE	(8)
#endif

/*
 * Public macros.
 */

#define LINKER_SET_ENTRY(_set,_sym)		__LINKER_MAKE_SET(_set,_sym)

#define LINKER_SET_BEGIN(_set)			__section_start(__TEXT, _set)
#define LINKER_SET_LIMIT(_set)			__section_end(__TEXT, _set)


#define LINKER_SET_FOREACH(_pvar, _set)					\
	for (_pvar = LINKER_SET_BEGIN(_set);				\
	     _pvar < LINKER_SET_LIMIT(_set);				\
	     _pvar += __LINKER_SET_STRIDE)

#define LINKER_SET_ITEM(_set, _i)		LINKER_SET_BEGIN(_set)[_i]

#endif /* __LINKER_SET_H__ */
