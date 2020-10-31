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

#include "s_cxt.h"
#include "s_cxt_diff.h"
#include "s_cxt_load.h"
#include "s_cxt_save.h"
#include "s_dbg.h"
#include "s_stats.h"
#include "s_geom.h"
#include "s_sb.h"
#include "s_write.h"

// external function:

void s_cxt_init(void)
{
    UInt32 sbarrSize, treeSize, bytesPerSb;

    // Calculate cxt size
    sbarrSize = sizeof(sftl.sb[0]) * s_g_max_sb;
    treeSize = L2V_NODEPOOL_MEM * 2;
    bytesPerSb = s_g_mul_bytes_per_lba(s_g_vbas_per_sb);
    sftl.cxt.cxt_vbas = (((sbarrSize + treeSize + bytesPerSb - 1) / s_g_bytes_per_page) * s_g_vbas_per_page);
    sftl.cxt.cxt_sbs = (sftl.cxt.cxt_vbas + s_g_vbas_per_sb - 1) / s_g_vbas_per_sb;
}

BOOL32 s_cxt_boot(const UInt32 readOnly)
{
    s_cxt_t *const cxt = &sftl.cxt;
    UInt64 Tstart, Tfin;
    BOOL32 enumerate;
    s_sb_fmt_t *shadowSB = NULL;
    BOOL32 uncleanShutDownRecovery;

    // Set up cxt variables
    cxt->load.baseWeaveSeq = 0;
    cxt->load.bases = 0;

    // Init weave order, allocate memory
    if (!s_wo_init()) {
        return FALSE32;
    }
    
    // Scan all superblocks, make in-order age range list
    WMR_PRINT(FTL, "scanning for weaves and types...\n");
    Tstart = WMR_CLOCK_TICKS();
    s_scan_all_sb();
    Tfin = WMR_CLOCK_TICKS();
    WMR_PRINT(FTL, "[%dms]\n", (Tfin-Tstart) / WMR_GET_TICKS_PER_US() / 1000);

    // Load cxt
    WMR_PRINT(FTL, "loading cxt...\n");
    Tstart = WMR_CLOCK_TICKS();
    shadowSB = WMR_MALLOC(sizeof(s_sb_fmt_t) * s_g_max_sb);
    enumerate = s_cxt_load(FALSE32, shadowSB);
    Tfin = WMR_CLOCK_TICKS();
    WMR_PRINT(FTL, "[%dms]\n", (Tfin-Tstart) / WMR_GET_TICKS_PER_US() / 1000);
    if (!enumerate) {
        goto skipDiffStats;
    }

    // And scan data written since the cxt
    WMR_PRINT(FTL, "adopting diffs...\n");
    Tstart = WMR_CLOCK_TICKS();
    s_cxt_scan_diff();
    Tfin = WMR_CLOCK_TICKS();
    WMR_PRINT(FTL, "[%dms]\n", (Tfin-Tstart) / WMR_GET_TICKS_PER_US() / 1000);

    // New data must be newer than cxt
    WMR_ASSERT(sftl.write.weaveSeq >= cxt->load.baseWeaveSeq);

    // Load AND statistics
    s_cxt_stats_load();
    if (cxt->diff.updates) {
        sftl.stats.unclean_boot_count++;
        uncleanShutDownRecovery = TRUE32;
        WMR_PRINT(ALWAYS, "sftl: error, unclean shutdown; adopted %d spans\n", cxt->diff.updates);
    }
    else
    {
        uncleanShutDownRecovery = FALSE32;
    }
    
    if((uncleanShutDownRecovery == FALSE32) && (shadowSB != NULL))
    {
        UInt16 sb;
        for (sb = 0; sb < s_g_max_sb; sb++)
        {
            if((shadowSB[sb].type == S_SB_ERASED) && (s_sb_get_type(sb) == S_SB_ERASE_BEFORE_USE))
            {
                s_sb_set_type(sb, S_SB_ERASED);
                sftl.seaState.erased_sb++;
            }
        }
    }

skipDiffStats:
    // Free temporary memory
    if(shadowSB != NULL)
    {
        WMR_FREE(shadowSB, sizeof(s_sb_fmt_t) * s_g_max_sb);
    }
    s_wo_close();

#ifndef AND_READONLY
    // Mark all 0-valid data SBs as pending erase
    s_sb_sweep0();
#endif

    sftl.booted = 1;
    s_dbg_check_sb_dist();
    s_dbg_check_validSums();
    s_dbg_check_data_counts();

    return TRUE32;
}

#ifndef AND_READONLY
void s_cxt_periodic(void)
{
    s_cxt_t *const cxt = &sftl.cxt;

    if (cxt->periodic.sbsSince >= S_CXT_PERIOD_SBS) {
        s_cxt_save();
        cxt->periodic.sbsSince = 0;
        cxt->periodic.sbsSinceStats = 0;
    } else if (cxt->periodic.sbsSinceStats >= S_STATS_PERIOD_SBS) {
        s_stats_insert();
        s_write_push_full_buf(TRUE32);
        cxt->periodic.sbsSinceStats = 0;
    }
}
#endif // AND_READONLY

void s_cxt_addSb(UInt32 sb, WeaveSeq_t weaveSeq)
{
    s_cxt_t *const cxt = &sftl.cxt;
    Int32 pos;

    WMR_ASSERT(cxt->load.bases < S_CXT_MAX_SB);

    // Insert in decerasing order to sftl.load.baseSb, baseWeaves, bases:

    // Find position and slip
    pos = cxt->load.bases;
    while ((pos > 0) && (weaveSeq > cxt->load.baseWeaves[pos-1])) {
        cxt->load.baseSb[pos] = cxt->load.baseSb[pos-1];
        cxt->load.baseWeaves[pos] = cxt->load.baseWeaves[pos-1];
        pos--;
    }

    // Insert
    cxt->load.baseSb[pos] = sb;
    cxt->load.baseWeaves[pos] = weaveSeq;
    cxt->load.bases++;
}
