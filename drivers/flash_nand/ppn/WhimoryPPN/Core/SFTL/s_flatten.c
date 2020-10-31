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

#include "s_flatten.h"
#include "s_gc.h"
#include "s_write.h"
#include "L2V/L2V_Extern.h"

#ifndef AND_READONLY

BOOL32 s_flatten(void)
{
    UInt32 lba, span;

    if (!L2V_LowishMem) {
        return TRUE32;
    }

    WMR_TRACE_IST_0(Flatten, START);

    do {
        
        do {
            // Repack while critically low
            L2V_ForceRepack();
        } while (L2V_CriticalMem);
        // No need to continue if we're above threshold
        if(!L2V_LowMem)
        {
            continue;
        }
        // Steam-roller!
        // find lba region
        do {
            L2V_FindFrag(&lba, &span);
            if((lba + span) > sftl.max_lba) {
                if(lba >= sftl.max_lba) {
                    span = 0;
                } else {
                    span = sftl.max_lba - lba;
                }
            }
        } while(span == 0);

        // Do any GC upfront
        s_gc_for_flatten(span);

        // Write it
        s_drain_stream_all(TRUE32);
        sftl.stats.lbas_flatten += span;
        if (!s_gc_inject(lba, span)) {
            WMR_TRACE_IST_1(Flatten, END, FALSE32);
            return FALSE32;
        }
        s_drain_stream_all(TRUE32);
        L2V_Repack(lba >> L2V_TREE_BITS);
    } while (L2V_LowMem);

    WMR_TRACE_IST_1(Flatten, END, TRUE32);
    return TRUE32;
}

#endif // AND_READONLY
