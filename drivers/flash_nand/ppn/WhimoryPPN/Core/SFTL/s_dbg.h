/*
 * Copyright (c) 2008-2010 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */

// Author:  Daniel J. Post (djp), dpost@apple.com

#ifndef __S_DBG_H__
#define __S_DBG_H__

#include "s_internal.h"

#define S_DBG_INIT       (1 << 0)
#define S_DBG_FORMAT     (1 << 1)
#define S_DBG_READ       (1 << 2)
#define S_DBG_WRITE      (1 << 3)
#define S_DBG_ERASE      (1 << 4)
#define S_DBG_MISC       (1 << 5)
#define S_DBG_ERROR      (1 << 6)
#define S_DBG_OPEN       (1 << 7)
#define S_DBG_INFO       (1 << 8)

#define SFTL_DEBUG         (S_DBG_ERROR)

#ifdef SFTL_DEBUG
#define SFTL_ENABLE_DEBUG_PRINT (0xff)
# define s_debug(fac, fmt, args ...)                   \
    do { \
        if (S_DBG_ ## fac & SFTL_DEBUG & SFTL_ENABLE_DEBUG_PRINT) { \
            _WMR_PRINT("sftl::%s(l:%d): " fmt "\n", __FUNCTION__, __LINE__, ## args); \
        }                               \
    } while (0)
#else
# define s_debug(fac, fmt, args ...)   do { } while (0)
#endif


// Self-consistency checks
#if S_DBG_CONSISTENCY && !defined(AND_READONLY)
extern BOOL32 s_dbg_init(void);
extern void s_dbg_close(void);
extern void s_dbg_check_validSums(void);
extern void s_dbg_check_data_counts(void);
extern void s_dbg_check_sb_dist(void);
#else
#define s_dbg_init() TRUE32
#define s_dbg_close()
#define s_dbg_check_validSums()
#define s_dbg_check_data_counts()
#define s_dbg_check_sb_dist()
#endif

#endif // __S_DBG_H__

