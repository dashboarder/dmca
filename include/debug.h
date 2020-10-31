/*
 * Copyright (C) 2007-2008 Apple Inc. All rights reserved.
 * Copyright (C) 2006 Apple Computer, Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */
#ifndef __DEBUG_H
#define __DEBUG_H

#include <compiler.h>
#include <stdio.h>
#include <sys.h>

#ifndef DEBUG_UART_ENABLE_DEFAULT
# define DEBUG_UART_ENABLE_DEFAULT	0
#endif

#define DEBUG_SILENT	0
#define DEBUG_RELEASE	5
#define DEBUG_CRITICAL	10
#define DEBUG_INFO	20
#define DEBUG_SPEW	30
#define DEBUG_NEVER	40

#ifndef DEBUG_LEVEL
# if DEBUG_BUILD
#  define DEBUG_LEVEL DEBUG_INFO
# else
#  define DEBUG_LEVEL DEBUG_CRITICAL
# endif
#endif

#ifndef OBFUSCATED_LOGGING
# if RELEASE_BUILD
#  define OBFUSCATED_LOGGING	1
# else
#  define OBFUSCATED_LOGGING	0
# endif
#endif

#define dprintf(level, x...) \
	do { \
		if ((level) <= DEBUG_LEVEL) { \
			if (OBFUSCATED_LOGGING && (level) > DEBUG_RELEASE) { printf("%llx:%d\n", DEBUG_FILE_HASH, __LINE__); } \
			else { printf(x); } \
		} \
	} while(0)
#define dhexdump(level, ptr, len) \
	do { \
		if ((level) <= DEBUG_LEVEL) { \
			if (OBFUSCATED_LOGGING && (level) > DEBUG_RELEASE) { printf("%llx:%d\n", DEBUG_FILE_HASH, __LINE__); } \
			else { hexdump(ptr, len); } \
		} \
	} while(0)

#define __ASSERT(_x, _str)						\
do {									\
	if (unlikely(!(_x))) {						\
 		panic("ASSERT FAILED at (%s:%d)\n", __FILE__, __LINE__);\
	}								\
} while(0)

#if DEBUG_BUILD || ENABLE_RELEASE_ASSERTS || defined(__clang_analyzer__)
/* for debug builds, turn all the asserts */
# define RELEASE_ASSERT(_x)	__ASSERT(_x, #_x)
# define ASSERT(_x)		__ASSERT(_x, #_x)
#else
/* for release, development builds */
# define RELEASE_ASSERT(_x)	__ASSERT(_x, #_x)
# define ASSERT(_x)		do {if (_x) {}} while(0)
#endif

#define static_assert(assertion, error) _Static_assert(assertion, error)

#if WITH_SIMULATION_TRACE
/*
 * To enable postmortem analysis of non-debug code being run in simulation, we allow a platform to
 * define two locations to which we will write trace data; one for location, one for parameters.
 * This also permits watchpoint debugging using a hardware debugger.
 *
 * In both cases, the write to the IP trace register stores the address of the current function.
 * Following this are either one or two writes to the parameter trace register; the first gives the
 * source file line containing the trace statement, the second (if present) the parameter.
 */
#if !defined(SIMULATION_TRACE_IP_REGISTER) || !defined(SIMULATION_TRACE_PARAMETER_REGISTER)
# error Must define SIMULATION_TRACE_IP_REGISTER and SIMULATION_TRACE_PARAMETER_REGISTER if WITH_SIMULATION_TRACE is defined
#endif

/*
 * Trace our passing a location.
 */
#define SIMULATION_TRACE()						\
	do {								\
		*(uint32_t *)SIMULATION_TRACE_IP_REGISTER = (uint32_t)__func__;	\
		*(uint32_t *)SIMULATION_TRACE_PARAMETER_REGISTER = (uint32_t)__LINE__; \
	} while (0)

/*
 * Trace a location and a parameter at that location.
 */
#define SIMULATION_TRACE_VALUE(_x)					\
	do {								\
		SIMULATION_TRACE();					\
		*(uint32_t *)SIMULATION_TRACE_PARAMETER_REGISTER = (uint32_t)(_x); \
	} while (0)

#define SIMULATION_TRACE_VALUE2(_x,_y)					\
	do {								\
		SIMULATION_TRACE();					\
		*(uint32_t *)SIMULATION_TRACE_PARAMETER_REGISTER = (uint32_t)(_x); \
		*(uint32_t *)SIMULATION_TRACE_PARAMETER_REGISTER = (uint32_t)(_y); \
	} while (0)

#endif /* WITH_SIMULATION_TRACE */

#endif /* __DEBUG_H */
