/*
 * Copyright (C) 2006,2011 Apple Computer, Inc. All rights reserved.
 *
 * This document is the property of Apple Computer, Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Computer, Inc.
 */
#ifndef __COMPILER_H
#define __COMPILER_H

#ifndef __ASSEMBLY__

#define likely(x)		__builtin_expect(!!(x), 1)
#define unlikely(x)		__builtin_expect(!!(x), 0)
#ifndef offsetof
# define offsetof(TYPE, MEMBER)	__builtin_offsetof (TYPE, MEMBER)
#endif
#define clz(_x)			__builtin_clz(_x)
#define ctz(_x)			__builtin_ctz(_x)

#define __max(_x, _y)		({ typeof (_x) _a = (_x); typeof (_y) _b = (_y); _a > _b ? _a : _b; })
#define __min(_x, _y)		({ typeof (_x) _a = (_x); typeof (_y) _b = (_y); _a < _b ? _a : _b; })

#ifndef __unused
# define __unused		__attribute__((unused))
#endif
#ifndef __pure
# define __pure			__attribute__((pure))
#endif
#ifndef __printflike
# define __printflike(__fmt,__varargs) __attribute__((__format__ (__printf__, __fmt, __varargs)))
#endif
#ifndef __scanflike
# define __scanflike(__fmt,__varargs) __attribute__((__format__ (__scanf__, __fmt, __varargs)))
#endif

#ifndef __used
# define __used			__attribute__((used))
#endif
#define __packed		__attribute__((packed))
#define __aligned(x)		__attribute__((aligned(x)))
#define __noreturn		__attribute__((noreturn))
#ifndef __always_inline
# define __always_inline	__attribute__((always_inline))
#endif
#define __deprecated		__attribute__((deprecated))
#ifndef __nonnull
#define __nonnull(_x)		__attribute__((nonnull _x))
#endif
#define __malloc		__attribute__((malloc))
#define __warn_unused_result	__attribute__((warn_unused_result))
#define __return_address(_x)	__builtin_return_address(_x)
#ifndef __has_feature
# define has_feature(x)		0
#endif

#ifndef __has_builtin
# define __has_builtin(x) 0
#endif

/* 
 * Statement expressions for determining the start/end of segments and sections.
 *
 * These are uglier than they should be courtesy of
 *  <rdar://problem/9245770> No way to generate new segment/section boundary symbols from C code
 */
/* these have to be used at function scope due to their use of statement expressions */
#ifndef __linux__
 #define __segment_start(_seg)		\
	({extern void *__seg_start_##_seg __asm("segment$start$" #_seg); &__seg_start_##_seg;})

 #define __segment_end(_seg)		\
	({extern void *__seg_end_##_seg __asm("segment$end$" #_seg); &__seg_end_##_seg;})

 #define __section_start(_seg, _sec)	\
	({extern void *__sec_start_##_seg##_sec __asm("section$start$" #_seg "$" #_sec); &__sec_start_##_seg##_sec;})

 #define __section_end(_seg, _sec)	\
	({extern void *__sec_end_##_seg##_sec __asm("section$end$" #_seg "$" #_sec); &__sec_end_##_seg##_sec;})
#else
 #define __section_start(_seg, _sec)	({extern void *__start_##_sec; &__start_##_sec;})
 #define __section_end(_seg, _sec)	({extern void *__stop_##_sec; &__stop_##_sec;})
#endif


/* these work at global scope, when tagged onto the definition of a variable */
#define __segment_start_sym(_seg)	__asm("segment$start$" #_seg)
#define __segment_end_sym(_seg)		__asm("segment$end$"   #_seg)
#define	__section_start_sym(_seg, _sec)	__asm("section$start$" #_seg "$" #_sec)
#define	__section_end_sym(_seg, _sec)	__asm("section$end$"   #_seg "$" #_sec)

#endif /* __ASSEMBLY__ */
#endif /* __COMPILER_H */
