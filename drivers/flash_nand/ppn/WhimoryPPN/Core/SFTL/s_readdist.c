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
#include "s_sb.h"
#include "s_bg.h"
#include "s_geom.h"

void s_readDist_init(void)
{
    sftl.readDist.readsSince = 0;
    sftl.readDist.refresh_sb = 0xffffffff;
    sftl.readDist.readCount_sb = 0xffffffff;
}

void s_readDist_close(void)
{
    // Nothing now
}

void s_readCount_enq(UInt32 vba)
{
    UInt32 sb;

    WMR_TRACE_IST_1(ReadCountEnq, NONE, vba);

    sb = s_g_vba_to_sb(vba);

    if (0xffffffff == sftl.readDist.readCount_sb) {
        sftl.readDist.readCount_sb = sb;
    }
}

void s_readRefresh_enq(UInt32 vba)
{
    UInt32 sb;

    WMR_TRACE_IST_1(ReadRefreshEnq, NONE, vba);

    sb = s_g_vba_to_sb(vba);

    if (0xffffffff == sftl.readDist.refresh_sb) {
        sftl.readDist.refresh_sb = sb;
    }
    else
    {
        sftl.sb[sb].reads = S_READDIST_LIMIT_PER_SB;
    }
}

void s_readDist_end_of_read_handle(void)
{
#ifndef AND_READONLY
    UInt32 sb, type;

    if (!sftl.booted) {
        return;
    }

    sftl.readDist.readsSince++;

    // Find offending sb
    sb = sftl.readDist.refresh_sb;
    if (0xffffffff != sb) {
        sftl.readDist.refresh_sb = 0xffffffff;
        sftl.stats.refresh_gcs++;
    } else {
        sb = sftl.readDist.readCount_sb;

        // Not been long enough?  Get out
        if (sftl.readDist.readsSince < S_READDIST_PERIOD) {
            return;
        }

        // Service BG slice for read
        s_bg_read(1);

        // Don't go further if we have nothing to do
        if (0xffffffff == sb) {
            return;
        }
        sftl.readDist.readCount_sb = 0xffffffff;
        sftl.readDist.readsSince = 0;
        sftl.stats.readCount_gcs++;
    }

    WMR_TRACE_IST_0(ReadDist, START);

    // Give it to bg
    type = s_sb_get_type(sb);
    if ((S_SB_DATA == type) || (S_SB_DATA_CUR == type) || (S_SB_DATA_GC == type) || (S_SB_DATA_PENDING_GC == type)) {
        s_bg_enq(sb, TRUE32);
    }

    WMR_TRACE_IST_0(ReadDist, END);
#endif
}

