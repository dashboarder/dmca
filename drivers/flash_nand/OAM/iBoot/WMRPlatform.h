/*
 * Copyright (c) 2010 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */

#include "ANDTypes.h"
#include "WMRFeatures.h"

#include <string.h>        // memcmp
#include <sys.h>           // panic
#include <platform.h>      // cache
#include <platform/timer.h> // ticks

#ifndef _WMR_PLATFORM_H_
#define _WMR_PLATFORM_H_

////////////////////////////////////////////////////////////////////////////////
//
// WMR_LOG2
//
// Returns the floor(log2) of the given number
//
#define WMR_LOG2(x) (31UL - __builtin_clz((UInt32)(x)))

////////////////////////////////////////////////////////////////////////////////
//
// WMR_LSB1
//
// Returns the least significant set bit's position for the given number
//
#define WMR_LSB1(x) (0 != x ? __builtin_ctz((UInt32)x) : ~0UL)

////////////////////////////////////////////////////////////////////////////////
//
// WMR_MEMSET
// WMR_MEMCPY
// WMR_MEMCMP
//
// Standard memory operations (libraries where available)
//

#define WMR_MEMSET(dest, pattern, length) memset((dest), (pattern), (length))
#define WMR_MEMCPY(dest, src, length) memcpy((dest), (src), (length))
#define WMR_MEMCMP(buf1, buf2, length) memcmp((buf1), (buf2), (length))
#define WMR_STRLEN(str) strlen((str))

////////////////////////////////////////////////////////////////////////////////
//
// WMR_ASSERT
//
// Print an error and panic if the given condition is not true
//

# define WMR_ASSERT(_expr)                           \
    do {                                             \
        if (__builtin_expect(!(_expr), 0))           \
            panic("WMR_ASSERT failed:(%s) %u", \
            __FILE__, __LINE__);             \
    } while(0);


////////////////////////////////////////////////////////////////////////////////
//
// WMR_PANIC
//
// Reset the system
//
# define WMR_PANIC(fmt, ...)                   \
            panic("WMR_PANIC: " #fmt " %s:%u", \
            ## __VA_ARGS__, __FILE__, __LINE__)

////////////////////////////////////////////////////////////////////////////////
//
// WMR_PREPARE_WRITE_BUFFER
//
// Call before an outgoing I/O operation to ensure cache coherency
//

#define WRITE_PREP_CACHE_CMD (CACHE_CLEAN)

#if (defined(WMR_USE_FULL_CACHE_FLUSH) && WMR_USE_FULL_CACHE_FLUSH)
#define WMR_PREPARE_WRITE_BUFFER(buf, length) \
            platform_cache_operation(WRITE_PREP_CACHE_CMD, 0, 0)
#else //WMR_USE_FULL_CACHE_FLUSH
#if WMR_DEBUG
#define WMR_PREPARE_WRITE_BUFFER(buf, length)  \
    do{ \
        WMR_ASSERT(0 == ((UInt32)(buf) % CPU_CACHELINE_SIZE)); \
        platform_cache_operation(WRITE_PREP_CACHE_CMD, (buf), ROUNDUPTO((length), CPU_CACHELINE_SIZE));\
    }while(0)
#else
#define WMR_PREPARE_WRITE_BUFFER(buf, length)  \
    platform_cache_operation(WRITE_PREP_CACHE_CMD, (buf), ROUNDUPTO((length), CPU_CACHELINE_SIZE))
#endif// !WMR_DEBUG
#endif // !WMR_USE_FULL_CACHE_FLUSH

////////////////////////////////////////////////////////////////////////////////
//
// WMR_PREPARE_READ_BUFFER
//
// Call before an incoming I/O operation to ensure cache coherency
//
#define READ_PREP_CACHE_CMD (CACHE_CLEAN | CACHE_INVALIDATE)

#if (defined(WMR_USE_FULL_CACHE_FLUSH) && WMR_USE_FULL_CACHE_FLUSH)
#define WMR_PREPARE_READ_BUFFER(buf, length) \
            platform_cache_operation(READ_PREP_CACHE_CMD, 0, 0)
#else //WMR_USE_FULL_CACHE_FLUSH
#if WMR_DEBUG
#define WMR_PREPARE_READ_BUFFER(buf, length)  \
    do{ \
        WMR_ASSERT(0 == ((UInt32)(buf) % CPU_CACHELINE_SIZE)); \
        platform_cache_operation(READ_PREP_CACHE_CMD, (buf), ROUNDUPTO((length), CPU_CACHELINE_SIZE));\
    }while(0)
#else
#define WMR_PREPARE_READ_BUFFER(buf, length)  \
    platform_cache_operation(READ_PREP_CACHE_CMD, (buf), ROUNDUPTO((length), CPU_CACHELINE_SIZE))
#endif// !WMR_DEBUG
#endif // !WMR_USE_FULL_CACHE_FLUSH

////////////////////////////////////////////////////////////////////////////////
//
// WMR_COMPLETE_READ_BUFFER
//
// Call after an incoming I/O operation to ensure cache coherency
//
// Note: On the IOP, we only prepare aligned read buffers from the host, so we
// shouldn't have to actually clean any dirty lines
#define WMR_COMPLETE_READ_BUFFER(buf, length) \
    WMR_PREPARE_READ_BUFFER(buf, length)

////////////////////////////////////////////////////////////////////////////////
//
// WMR_GET_TICKS_PER_US
//
// Return the number of native ticks (as returned from WMR_CLOCK_NATIVE) per
// microsecond.
//

// UInt32
#define WMR_GET_TICKS_PER_US() (timer_get_tick_rate() / 1000000UL)

////////////////////////////////////////////////////////////////////////////////
//
// WMR_CLOCK_NATIVE
//
// Get the current system time in microseconds to be passed to
// WMR_HAS_TIME_ELAPSED_US and WMR_HAS_TIME_ELAPSED_MS, but NOT
// WMR_HAS_TIME_ELAPSED_TICKS
//

// UInt64
#define WMR_CLOCK_NATIVE() system_time()

////////////////////////////////////////////////////////////////////////////////
//
// WMR_CLOCK_TICKS
//
// Get the current system time in ticks (suitable for WMR_HAS_TIME_ELAPSED_TICKS)
//

//UInt64
#define WMR_CLOCK_TICKS() timer_get_ticks()

////////////////////////////////////////////////////////////////////////////////
//
// WMR_HAS_TIME_ELAPSED_US
//
// Supply a start timestamp from WMR_CLOCK_NATIVE to determine if the given
// time has elapsed
//

//UInt64 startTicks, UInt64 elapsedUS
#define WMR_HAS_TIME_ELAPSED_US(startTicks, elapsedUS) \
    (time_has_elapsed((startTicks), (elapsedUS)) ? TRUE32 : FALSE32)

////////////////////////////////////////////////////////////////////////////////
//
// WMR_HAS_TIME_ELAPSED_TICKS
//
// Supply a start timestamp from WMR_CLOCK_NATIVE to determine if the given
// time has elapsed.

//UInt64 startTicks, UInt64 elapsedTicks
#define WMR_HAS_TIME_ELAPSED_TICKS(startTicks, elapsedTicks) \
    ((timer_get_ticks() - (startTicks)) > (elapsedTicks) ? TRUE32 : FALSE32)


////////////////////////////////////////////////////////////////////////////////
//
// WMR_ENTER_CRITICAL_SECTION
//
// Disable pre-emption, reference counted (safe to call in interrupt)

#define WMR_ENTER_CRITICAL_SECTION() enter_critical_section()

////////////////////////////////////////////////////////////////////////////////
//
// WMR_EXIT_CRITICAL_SECTION
//
// Enable pre-emption, reference counted (safe to call in interrupt)

#define WMR_EXIT_CRITICAL_SECTION() exit_critical_section()

////////////////////////////////////////////////////////////////////////////////
//
// WMR_TRACE_*
//
// Compile-time tracepoints (not supported)
//
#define WMR_TRACE_0(...)
#define WMR_TRACE_1(...)
#define WMR_TRACE_2(...)
#define WMR_TRACE_3(...)
#define WMR_TRACE_4(...)
#define WMR_TRACE_IST_0(...)
#define WMR_TRACE_IST_1(...)
#define WMR_TRACE_IST_2(...)
#define WMR_TRACE_IST_3(...)
#define WMR_TRACE_IST_4(...)

////////////////////////////////////////////////////////////////////////////////
//
// WMR_RAND
//
// Get the next random number in the stream
//
#define WMR_RAND()  rand()

#endif /* _WMR_PLATFORM_H_ */

