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

#include "s_boot.h"
#include "s_cxt.h"
#include "s_sb.h"
#include "s_write.h"
#include "s_geom.h"
#include "s_gc.h"
#include "s_flatten.h"
#include "s_dbg.h"
#include "WhimoryBoot.h"

Int32 sftl_boot(UInt32 *reported_lba, UInt32 *lba_size, BOOL32 fullRestore, BOOL32 justFmt, UInt32 minorVer, UInt32 opts)
{
#ifndef AND_READONLY
    UInt32 i, sb, t;
#endif // AND_READONLY

    const UInt32 readOnly = ( kWhimoryBootOptionReadOnly == ( kWhimoryBootOptionReadOnly & opts ) ? 1 : 0 );

#ifdef AND_SIMULATOR
    if(opts & WMR_INIT_IGNORE_ERASE_GAP)
    {
        sftl.ignoreEraseGap = TRUE32;
    }
#endif

    // Populate return fields
    *reported_lba = sftl.reported_lba;
    *lba_size = s_g_mul_bytes_per_lba(1);

    // Load cxt, scan, etc
    if (!s_cxt_boot(readOnly)) {
        return ANDErrorCodeHwErr;
    }

    s_dbg_check_sb_dist();
    s_dbg_check_data_counts();

    sftl.stats.boot_count++;

#ifndef AND_READONLY
    if (readOnly) {
        // -- skip remaining block if r/w driver opened in RO mode.
        return ANDErrorCodeOk;
    }

    sftl.readDist.readsSince = S_READDIST_PERIOD;
    sftl.wearLev.blocksSince = S_WEARLEV_PERIOD;
    // Clear out stream state
    for (i = 0; i < S_SBSTREAM_MAX; i++) {
        sftl.write.stream[i].sb = 0xffffffff;
    }
    // Switch to dynamic stream
    s_write_switch(S_SBSTREAM_DYN);

    s_dbg_check_sb_dist();
    s_dbg_check_data_counts();

    // GC some PENDING_GC sb's, and erase all PENDING_ERASEs
    for (sb = 0; sb < s_g_max_sb; sb++) {
        t = s_sb_get_type(sb);
        if (S_SB_DATA_PENDING_GC == t) {
            // Flatten out some memory if necessary
            while (!s_flatten()) {
                // Program failure?
                s_write_reseq();
                s_write_handle_pfail();
            }
            t = s_sb_get_type(sb);
            if (S_SB_DATA_PENDING_GC == t) {
                // Double-check the sb type; could have changed via flatten
                s_gc_data(sb, FALSE32);
                s_dbg_check_data_counts();
            }
            t = s_sb_get_type(sb);
        }
        // Erase PENDING_ERASES, whether from GC or scan (uECC/invalid PageType)
        if (S_SB_PENDING_ERASE == t) {
            s_sb_boot_free_erase(sb);
            s_dbg_check_data_counts();
        }

        s_dbg_check_sb_dist();
        s_dbg_check_data_counts();
    }
    sftl.readDist.readsSince = S_READDIST_PERIOD;
    sftl.wearLev.blocksSince = S_WEARLEV_PERIOD;
#endif // AND_READONLY

    return ANDErrorCodeOk;
}

