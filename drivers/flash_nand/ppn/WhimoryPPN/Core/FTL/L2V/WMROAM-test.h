/*
 * Copyright (c) 2009 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */

#ifndef __WMROAM_H__
#define __WMROAM_H__

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#define UInt32 uint32_t
#define Int32 int32_t
#define UInt16 uint16_t
#define Int16 int16_t
#define UInt8 uint8_t
#define Int8 int8_t
#define BOOL32 bool
#define TRUE32 true
#define FALSE32 false

#define _WMR_ASSERT(e, file, line) { printf("%s:%u: failed assertion '%s'\nWaiting for debugger...\n", file, line, e); abort(); }
#define WMR_ASSERT(e) if (!(e)) _WMR_ASSERT(#e, __FILE__, __LINE__)
#define WMR_PANIC(format, args...) { printf(format , ##args); abort(); }
#define WMR_MEMSET(a, b, c) memset(a, b, c)
#define WMR_FREE(x, y) free(x)

extern void *WMR_MALLOC(UInt32 size);

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

#endif // __WMROAM_H__
