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

#ifndef  __S_GC_H__
#define  __S_GC_H__

#include "s_internal.h"

// Defines
#define lowData(adjust)       ((sftl.seaState.vbas_in_free_sb+sftl.write.sum_gcDataAdjust) < ((s_g_vbas_per_sb * (S_GC_FREE_THRESHOLD + S_GC_LOW_GAP + sftl.cxt.cxt_sbs)) + (adjust)))
#define idleLowData(adjust)   ((sftl.seaState.vbas_in_free_sb+sftl.write.sum_gcDataAdjust) < ((s_g_vbas_per_sb * (S_GC_FREE_THRESHOLD + S_GC_IDLELOW_GAP + sftl.cxt.cxt_sbs)) + (adjust)))
#define dangerousData(adjust) ((sftl.seaState.vbas_in_free_sb+sftl.write.sum_gcDataAdjust) < ((s_g_vbas_per_sb * (S_GC_FREE_THRESHOLD + sftl.cxt.cxt_sbs)) + (adjust)))

// General setup/close
extern BOOL32 s_gc_init(void);
extern void   s_gc_close(void);

// Called by Write
extern void s_gc_prewrite(UInt32 writeSize);
extern void s_gc_bg(UInt32 writeSize);
extern void s_gc_for_flatten(UInt32 writeSize);
extern void s_gc_bg_shutdown(void);

// For error recovery
extern void      s_gc_data_enq(UInt32 sb, UInt32 ctx);

// Direct access to the machines
extern ANDStatus s_gc_data(UInt32 sb, BOOL32 scrubOnUECC);
extern BOOL32 s_gc_move_advisable(void);

extern BOOL32    s_gc_inject(UInt32 lba, UInt32 count);
extern void      s_gc_zeroValidCross(void);
extern BOOL32    s_gc_slice(UInt32 ctx);
extern BOOL32    s_WorkFifo_isEmpty(GC_Fifo_t *f);
extern BOOL32    s_WorkFifo_isFullish(GC_Fifo_t *f);

#endif   // #ifndef __S_GC_H__
