/*
 * Copyright (c) 2011 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */

#ifndef _EFFACEABLE_DEBUG_H_
#define _EFFACEABLE_DEBUG_H_

// =============================================================================

#define EFFACEABLE_DBGMSK_INIT  (1<<0)
#define EFFACEABLE_DBGMSK_ERR   (1<<1)
#define EFFACEABLE_DBGMSK_WARN  (1<<2)
#define EFFACEABLE_DBGMSK_INFO  (1<<3)
#define EFFACEABLE_DBGMSK_SPEW  (1<<4)

// -----------------------------------------------------------------------------

#define EFFACEABLE_DBGTAG_INIT  "INIT"
#define EFFACEABLE_DBGTAG_ERR   "ERR "
#define EFFACEABLE_DBGTAG_WARN  "WARN"
#define EFFACEABLE_DBGTAG_INFO  "INFO"
#define EFFACEABLE_DBGTAG_SPEW  "SPEW"

// -----------------------------------------------------------------------------

#define EFFACEABLE_DEBUG_ALL (EFFACEABLE_DBGMSK_INIT       |    \
                              EFFACEABLE_DBGMSK_ERR        |    \
                              EFFACEABLE_DBGMSK_WARN       |    \
                              EFFACEABLE_DBGMSK_INFO       |    \
                              EFFACEABLE_DBGMSK_SPEW)

#define EFFACEABLE_DEBUG_MOST (EFFACEABLE_DBGMSK_INIT      |    \
                               EFFACEABLE_DBGMSK_ERR       |    \
                               EFFACEABLE_DBGMSK_WARN      |    \
                               EFFACEABLE_DBGMSK_INFO)

#define EFFACEABLE_DEBUG_DEFAULT (EFFACEABLE_DBGMSK_INIT   |    \
                                  EFFACEABLE_DBGMSK_ERR)

// -----------------------------------------------------------------------------

#define EFFACEABLE_DEBUG EFFACEABLE_DEBUG_DEFAULT

// -----------------------------------------------------------------------------

#if defined(EFFACEABLE_DEBUG) && EFFACEABLE_DEBUG

# define dlogf(_api, _fac, _fmt, ...)                   \
    do {                                                \
        if (EFFACEABLE_DBGMSK_##_fac &                  \
            EFFACEABLE_DEBUG) {                         \
            logf(_api, "[effaceable:%s] " _fmt "\n",    \
                 EFFACEABLE_DBGTAG_##_fac ,             \
                 ## __VA_ARGS__ );                      \
        }                                               \
    } while(0)

# define dhexdump(_api, _fac, _buf, _count)                             \
    do {                                                                \
        if (EFFACEABLE_DBGMSK_##_fac &                                  \
            EFFACEABLE_DEBUG) {                                         \
            unsigned _i, _j;                                            \
            for (_i = 0; _i < _count; _i += 16) {                       \
                logf(_api, "[effaceable:%s] 0x%08x:",                   \
                     EFFACEABLE_DBGTAG_##_fac ,                         \
                     ((unsigned) _buf) + _i);                           \
                for (_j = 0; (_j < 16) && ((_i + _j) < _count); _j++)   \
                    logf(_api, " %2.2x", ((uint8_t*)_buf)[_i + _j]);    \
                logf(_api, "\n");                                       \
            }                                                           \
        }                                                               \
    } while(0)

#else
# define dlogf(...)  do { /* nothing */ } while(0)
# define dhexdump(...)  do { /* nothing */ } while(0)
#endif

// =============================================================================

#endif /* _EFFACEABLE_DEBUG_H_ */
