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

#ifndef __S_SB_H__
#define __S_SB_H__

#include "s_internal.h"

// Getters
#define s_sb_get_type(sbno) (sftl.sb[sbno].type+0)
#define s_sb_get_validLbas(sbno) (sftl.sb[sbno].validLbas+0)
#define s_sb_get_reads(sbno) (sftl.sb[sbno].reads+0)
#define s_sb_get_num_btoc_vbas(sbno) (sftl.sb[sbno].num_btoc_vbas_AND_staticFlag>>1)
#define s_sb_get_erases(sbno) (sftl.sb[sbno].erases+0)
#define s_sb_get_static(sbno) (sftl.sb[sbno].num_btoc_vbas_AND_staticFlag & 1)

// Setters
#ifndef AND_READONLY
extern UInt32 s_WorkFifo_Deq_sb(GC_Fifo_t *f, UInt32 sb);

WMR_CASSERT(S_GC_NUM_CTX == 2, num_ctx_matches_sb_set_type);
#define s_sb_set_type(sbno, x) \
do {\
    if(((x) != S_SB_DATA_PENDING_GC) && ((x) != S_SB_DATA)) { \
        s_WorkFifo_Deq_sb(&sftl.gc.ctx[0].workFifo, (sbno)); \
        s_WorkFifo_Deq_sb(&sftl.gc.ctx[1].workFifo, (sbno)); \
    } \
    sftl.sb[sbno].type = (x); \
} while (0)
#else
#define s_sb_set_type(sbno, x) \
do  {\
        sftl.sb[sbno].type = (x); \
    } while (0)
#endif
#define s_sb_set_validLbas(sbno, x) do { sftl.sb[sbno].validLbas = (x); } while (0)
#define s_sb_set_reads(sbno, x) do { sftl.sb[sbno].reads = (x); } while (0)
#define s_sb_set_num_btoc_vbas(sbno, x) do { WMR_ASSERT(0 != (x)); sftl.sb[sbno].num_btoc_vbas_AND_staticFlag = (sftl.sb[sbno].num_btoc_vbas_AND_staticFlag & 1) | ((x) << 1); } while(0)
#define s_sb_set_num_btoc_vbas_0ok(sbno, x) do { sftl.sb[sbno].num_btoc_vbas_AND_staticFlag = (x) << 1; } while(0)
#define s_sb_clr_num_btoc_vbas(sbno) do { sftl.sb[sbno].num_btoc_vbas_AND_staticFlag &= 1; } while(0)
#define s_sb_set_static(sbno, sFlag) do { sftl.sb[sbno].num_btoc_vbas_AND_staticFlag = (sftl.sb[sbno].num_btoc_vbas_AND_staticFlag & ~1) | ((sFlag) & 1); } while (0)
#define s_sb_log_erase(sbN) \
do { \
    sftl.sb[sbN].erases++; \
} while (0)
#define s_sb_log_next() \
do { \
    sftl.cxt.periodic.sbsSince++; \
    sftl.cxt.periodic.sbsSinceStats++; \
} while (0)


extern BOOL32 s_sb_init(void);
extern void   s_sb_close(void);
extern void   s_sb_next_cur(void);
extern void   s_sb_next(UInt32 stream);
extern void   s_sb_next_from_sb(UInt32 sb);
extern UInt32 s_sb_cxt_alloc(void);
extern void   s_sb_boot_free_erase(UInt32 sb);
extern void   s_sb_cxt_free_erase(UInt32 sb);
extern void   s_sb_cxt_free_clean(UInt32 sb);
extern void   s_sb_fmt_erase(UInt32 sb);
extern void   s_sb_sweep0(void);

#endif // __S_SB_H__

