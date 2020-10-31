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

#include "s_fmt.h"
#include "s_geom.h"
#include "s_cxt.h"
#include "s_cxt_load.h"
#include "s_sb.h"

#ifndef AND_READONLY

static void stats_scan_all_cb(UInt32 vba, VFLReadStatusType status, UInt8 *__meta)
{
    PageMeta_BTOC_t *meta = (PageMeta_BTOC_t*)__meta;
    UInt32 sb, vbaOfs;

    sb = s_g_vba_to_sb(vba);
    vbaOfs = s_g_vba_to_vbaOfs(vba);

    if (status & VFL_READ_STATUS_UNIDENTIFIED) {
        WMR_PRINT(QUAL_FATAL, "stats scan got unidentified read status (0x%x) on vba:0x%x; did we get a timeout?  Pretending it was uECC.\n", status, vba);
        status = (status & ~VFL_READ_STATUS_UNIDENTIFIED) | VFL_READ_STATUS_UECC;
    }

    // Need a valid first-cxt page
    if (status & VFL_READ_STATUS_VALID_DATA) {
        if (PAGETYPE_FTL_CXT == meta->c.PageType) {
            if (s_cxt_is_firstSb(meta, sb, vbaOfs)) {
                s_cxt_addSb(sb, META_GET_WEAVESEQ(meta));
            }
        }
    }
}

void stats_scan_all(void)
{
    s_cxt_t *const cxt = &sftl.cxt;
    UInt32 vba, sb, size;
    PageMeta_t *meta;
    UInt8 *buf;
    UInt32 vbas_in_sb;

    // Set up cxt variables
    cxt->load.baseWeaveSeq = 0;
    cxt->load.bases = 0;

    // Find pagetypes, weaveSeqMin, weaveSeqAdd
    for (sb = 0; sb < s_g_vfl_max_sb; ) {
        size = 0;

        // Clear read-spattered
        sftl.vfl->ReadSpansInit(&cxt->load.rs, stats_scan_all_cb, VFL_READ_STATUS_ALL, TRUE32, FALSE32);
        buf = sftl.gc.zone;
        meta = sftl.scan_meta;
        
        // Read the first vba per sb to find cxt
        while ((size < S_SCAN_META_SIZE) && (sb < s_g_vfl_max_sb)) {
            vbas_in_sb = sftl.vfl->GetVbasPerVb(sb);
            if (vbas_in_sb) {
                // Schedule read of first page
                vba = s_g_addr_to_vba(sb, 0);
                sftl.vfl->ReadSpansAdd(&cxt->load.rs, vba, 1, buf, (UInt8*)meta);
                meta++;
            }

            // Next
            size++;
            sb++;
        }

        WMR_ASSERT(size <= S_SCAN_META_SIZE);
        // Perform read
        sftl.vfl->ReadSpans(&cxt->load.rs);
    }
}

void loadStats(void)
{
    stats_scan_all();
    s_cxt_load(TRUE32, NULL);
}

Int32 sftl_format(UInt32 opts)
{
    UInt32 i;
    
    if(!(WMR_INIT_RUN_PRODUCTION_FORMAT & opts))
    {
        loadStats();
    }

    for (i = 0; i < s_g_max_sb; i++) {
        s_sb_fmt_erase(i);
    }

    sftl.cxt.save.num_sb = 0;
    s_cxt_save();

    return ANDErrorCodeOk;
}

#endif // AND_READONLY

