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

#define AND_TRACE_LAYER FTL

#include "WMRConfig.h"
#include "s_readdist.h"
#include "s_wearlev.h"
#include "s_geom.h"
#include "s_gc.h"
#include "s_sb.h"
#include "s_bg.h"

#ifndef AND_READONLY

void s_wearlev_search(void)
{
    UInt32 i, erases, type;
    UInt32 curErases, sb, refreshSB;
    UInt32 maxErases;
    UInt32 maxDynErases;

    // Early out: not our time yet
    if (sftl.wearLev.blocksSince < S_WEARLEV_PERIOD) {
        return;
    }
    sftl.wearLev.blocksSince -= S_WEARLEV_PERIOD;

    WMR_TRACE_IST_0(WearLevel, START);

    // Find lowest-cycled static data superblock
    maxErases = 0;
    maxDynErases = 0;
    curErases = ~0;
    sb = ~0; 
    refreshSB = ~0;
    for (i = 0; i < s_g_max_sb; i++) {
        // Filter for data SB
        type = s_sb_get_type(i);
        if (S_SB_DATA != type) {
            continue;
        }

        erases = s_sb_get_erases(i);

        // Is it lowest-cycled and static (old data)?
        if ((erases < curErases) && S_USERSEQ_IS_OLD(sftl.sb_userSeq[i].avg)) {
            // Minify
            curErases = erases;
            sb = i;
        }

        if (erases > maxErases) {
            // Maxify
            maxErases = erases;
        }

        if (!S_USERSEQ_IS_OLD(sftl.sb_userSeq[i].avg) && (erases > maxDynErases)) {
            // Maxify dynamic-sb erases
            maxDynErases = erases;
        }
        
        if ((~((UInt32)0) == refreshSB) && (sftl.sb[i].reads >= S_READDIST_LIMIT_PER_SB) && (sb != i))
        {
            refreshSB = i;
        }
    }

    // Don't wear-level unless the lowest-cycled static block makes the gap big
    if ((curErases + S_WEARLEV_MIN_GAP) >= maxErases) {
        sb = ~0;
    }

    // Don't wear-level unless the highest-cycled dynamic superblock is approaching the gap
    if ((maxDynErases + S_WEARLEV_DYN_GAP) < maxErases) {
        sb = ~0;
    }

    // Put into high-cycled superblock; GC will use the static flags to partition
    if (~((UInt32)0) != refreshSB)
    {
        sftl.stats.readCount_gcs++;
        s_bg_enq(refreshSB, TRUE32);
    }
    else
    {
        if (~((UInt32)0) != sb)
        {
            sftl.stats.wearLev_gcs++;
            s_bg_enq(sb, TRUE32);
        }
    }

    WMR_TRACE_IST_0(WearLevel, END);
}

#endif // AND_READONLY
