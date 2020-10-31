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

#include "s_internal.h"
#include "s_defines.h"
#include "s_sb.h"
#include "s_btoc.h"
#include "s_wearlev.h"
#include "s_geom.h"
#include "s_dbg.h"
#include "s_trim.h"
#ifndef AND_READONLY
#include "s_gc.h"
#include "s_bg.h"
#endif

BOOL32 s_sb_init(void)
{
    UInt32 i;

    sftl.minEraseLocations = WMR_MALLOC(sizeof(UInt16) * S_GC_FREE_THRESHOLD);
    if(NULL == sftl.minEraseLocations)
    {
        return FALSE32;
    }
    WMR_MEMSET(sftl.minEraseLocations, 0xff, sizeof(UInt16) * S_GC_FREE_THRESHOLD);
    
    sftl.sb = (s_sb_t*)WMR_MALLOC(sizeof(*sftl.sb) * s_g_max_sb);
    if (NULL == sftl.sb) {
        return FALSE32;
    }
    sftl.sb_userSeq = (s_sb_userSeq_t*)WMR_MALLOC(sizeof(*sftl.sb_userSeq) * s_g_max_sb);
    if (NULL == sftl.sb_userSeq) {
        return FALSE32;
    }
    WMR_MEMSET(sftl.sb_userSeq, 0, sizeof(*sftl.sb_userSeq) * s_g_max_sb);

    for (i = 0; i < s_g_max_sb; i++) {
        s_sb_set_type(i, S_SB_UNKNOWN);
        s_sb_set_num_btoc_vbas_0ok(i, 0);
        s_sb_set_validLbas(i, 0);
        s_sb_set_reads(i, 0);
    }

    return TRUE32;
}

void s_sb_close(void)
{
    if(NULL != sftl.minEraseLocations) {
        WMR_FREE(sftl.minEraseLocations, sizeof(UInt16) * S_GC_FREE_THRESHOLD);
    }
    if (NULL != sftl.sb) {
        WMR_FREE(sftl.sb, sizeof(*sftl.sb) * s_g_max_sb);
    }
    if (NULL != sftl.sb_userSeq) {
        WMR_FREE(sftl.sb_userSeq, sizeof(*sftl.sb_userSeq) * s_g_max_sb);
    }

    sftl.minEraseLocations = NULL;
    sftl.sb = NULL;
    sftl.sb_userSeq = NULL;
}

#ifndef AND_READONLY
void s_sb_next_cur(void)
{
    s_sb_next(sftl.write.curStream);
}

static BOOL32 checkForEOL(void)
{
    UInt32 vbas, sb;

    // Sum available VBAs
    vbas = 0;
    for (sb = 0; sb < s_g_max_sb; sb++) {
        switch (s_sb_get_type(sb)) {
            case S_SB_ERASED:
            case S_SB_PENDING_ERASE:
            case S_SB_ERASE_BEFORE_USE:
            case S_SB_DATA:
                vbas += sftl.vfl->GetVbasPerVb(sb);
                break;
        }
    }

    WMR_PRINT(ALWAYS, "Checking for NAND EOL; VBAs:%d LBA-max:%d LBAs-in-use:%d\n", vbas, sftl.reported_lba, sftl.seaState.validLbas);

    // Check if it's a legitimate EOL, or GC bug
    if ((sftl.seaState.validLbas + (2 * S_GC_FREE_THRESHOLD * s_g_vbas_per_sb)) >= vbas) {
#if (defined(AND_SIMULATOR) && AND_SIMULATOR)
        WMR_SIM_EXIT("NAND end-of-life!");
#else // defined(AND_SIMULATOR) && AND_SIMULATOR
        WMR_PANIC("NAND end-of-life!");
#endif // defined(AND_SIMULATOR) && AND_SIMULATOR
    }

    return FALSE32;
}
#define ENTRY_UNPOPULATED 0xffff
static UInt16 getMaxErase(UInt16 *eraseableArray, UInt8 count, UInt8 *location)
{
    UInt8 i;
    UInt16 maxValue = 0;
    
    if((eraseableArray == NULL) || (location == NULL))
    {
        return 0;
    }
    
    for(i = 0; i < count; i++)
    {
        if(eraseableArray[i] == ENTRY_UNPOPULATED)
        {
            maxValue = ENTRY_UNPOPULATED;
            *location = i;
            break;
        }
        if(sftl.sb[eraseableArray[i]].erases > maxValue)
        {
            maxValue = sftl.sb[eraseableArray[i]].erases;
            *location = i;
        }
    }
    return maxValue;
}

static UInt16 replaceMaxErase(UInt16 *eraseableArray, UInt8 location, UInt16 newEntry)
{
    if((eraseableArray == NULL) || ((eraseableArray[location] != ENTRY_UNPOPULATED) && (sftl.sb[eraseableArray[location]].erases <  sftl.sb[newEntry].erases)))
    {
        return ENTRY_UNPOPULATED;
    }
    
    eraseableArray[location] = newEntry;
    
    return location;
}

UInt32 s_sb_alloc(BOOL32 chooseMinErases)
{
    UInt32 i, chosen, vbas, type;
    UInt32 minErases, minIdx;
    UInt32 maxErases, maxIdx;
    UInt8 maxEraseEntryLocation;
    UInt32 minErased, minErasedIdx;

again:
    minErases = 0xffffffff;
    minIdx = 0xffffffff;
    minErased = 0xffffffff;
    minErasedIdx = 0xffffffff;
    maxErases = 0;
    maxIdx = 0xffffffff;

    WMR_MEMSET(sftl.minEraseLocations, 0xff, sizeof(UInt16) * S_GC_FREE_THRESHOLD);
    // Find min-erase block
    for (i = 0; i < s_g_max_sb; i++) {
        type = s_sb_get_type(i);
        if(((sftl.seaState.erased_sb ) < S_GC_FREE_THRESHOLD)
           && ((S_SB_ERASE_BEFORE_USE == type) || (S_SB_PENDING_ERASE == type)))
        {
            if(getMaxErase(sftl.minEraseLocations, S_GC_FREE_THRESHOLD, &maxEraseEntryLocation) > sftl.sb[i].erases)
            {
                replaceMaxErase(sftl.minEraseLocations, maxEraseEntryLocation, (UInt16)i);
            }
        }

        if ((S_SB_ERASED == type)
            || (S_SB_PENDING_ERASE == type)
            || (S_SB_ERASE_BEFORE_USE == type))
        {
            WMR_ASSERT(0 == s_sb_get_validLbas(i));
            if (s_sb_get_erases(i) < minErases) {
                minErases = s_sb_get_erases(i);
                minIdx = i;
            }
            if (s_sb_get_erases(i) >= maxErases) {
                maxErases = s_sb_get_erases(i);
                maxIdx = i;
            }
            if(S_SB_ERASED == type)
            {
                if (s_sb_get_erases(i) < minErased) {
                    minErased = s_sb_get_erases(i);
                    minErasedIdx = i;
                }
            }
        }
    }

    // Check for allocation failure
    if ((0xffffffff == minIdx) || (0xffffffff == maxIdx)) {
        if (!checkForEOL()) {
            WMR_PANIC("GC bug?  Ran out of blocks, but probably not EOL.");
        }
    }

    for(i = 0; (i < S_GC_FREE_THRESHOLD) && (sftl.seaState.erased_sb < S_GC_FREE_THRESHOLD); i++)
    {
        if((sftl.minEraseLocations[i] != ENTRY_UNPOPULATED) && ((S_SB_ERASE_BEFORE_USE == s_sb_get_type(sftl.minEraseLocations[i])) || (S_SB_PENDING_ERASE == s_sb_get_type(sftl.minEraseLocations[i]))))
        {
            sftl.seaState.vbas_in_free_sb -= sftl.vfl->GetVbasPerVb(sftl.minEraseLocations[i]);
            sftl.vfl->Erase(sftl.minEraseLocations[i], TRUE32);
            sftl.seaState.vbas_in_free_sb += sftl.vfl->GetVbasPerVb(sftl.minEraseLocations[i]);
            s_sb_set_type(sftl.minEraseLocations[i], S_SB_ERASED);
            s_sb_log_erase(sftl.minEraseLocations[i]);
            sftl.seaState.erased_sb++;
            s_dbg_check_sb_dist();
            s_trim_checkVulnerable(sftl.minEraseLocations[i]);
            
        }
        
    }
    
    if (chooseMinErases) {
        if((minErasedIdx != minIdx) && ((minErases + 1) >= minErased))
            chosen = minErasedIdx;
        else
            chosen = minIdx;
    } else {
        if((minErasedIdx != minIdx) && ((minErases + 1) >= minErased))
            chosen = minErasedIdx;
        else
            chosen = minIdx;
        //TODO:evaluate median instead of maxIdx ( too extreme ) for static stream.
    }

    // Decrement free space by pre-erase size
    sftl.seaState.vbas_in_free_sb -= sftl.vfl->GetVbasPerVb(chosen);

    if (S_SB_ERASED != s_sb_get_type(chosen)) {
        // Erase it
        sftl.vfl->Erase(chosen, TRUE32);
        s_sb_log_erase(chosen);
        s_trim_checkVulnerable(chosen);
        // Don't set the type, since it's set by caller
    } else {
        sftl.seaState.erased_sb--;
    }

    // Filter out dead superblocks
    vbas = sftl.vfl->GetVbasPerVb(chosen);
    if (0 == vbas) {
        s_sb_set_type(chosen, S_SB_DEAD);
        sftl.seaState.dead_sb++;
        sftl.seaState.free_sb--;
        goto again;
    }

    // Counters
    sftl.seaState.free_sb--;
    s_sb_set_reads(chosen, 0);

    return chosen;
}

UInt32 s_sb_cxt_alloc(void)
{
    UInt32 chosen;

    chosen = s_sb_alloc(TRUE32);
    s_sb_set_type(chosen, S_SB_CXT);
    sftl.seaState.cxt_sb++;

    return chosen;
}

void s_sb_cxt_free_clean(UInt32 sb)
{
    UInt32 type;

    type = s_sb_get_type(sb);
    if ((S_SB_ERASED != type)
        && (S_SB_PENDING_ERASE != type)
        && (S_SB_ERASE_BEFORE_USE != type))
    {
        sftl.seaState.vbas_in_free_sb += sftl.vfl->GetVbasPerVb(sb);
        WMR_ASSERT(S_SB_CXT == s_sb_get_type(sb));
        sftl.seaState.free_sb++;
        sftl.seaState.cxt_sb--;
    }

    if (S_SB_ERASED != type) {
        sftl.seaState.erased_sb++;
    }

    s_sb_set_type(sb, S_SB_ERASED);
    s_sb_log_erase(sb);
}

void s_sb_cxt_free_erase(UInt32 sb)
{
    WMR_ASSERT(S_SB_CXT == s_sb_get_type(sb));
    sftl.vfl->Erase(sb, TRUE32);
    s_sb_cxt_free_clean(sb);
}

void s_sb_boot_free_erase(UInt32 sb)
{
    sftl.seaState.vbas_in_free_sb -= sftl.vfl->GetVbasPerVb(sb);
    sftl.vfl->Erase(sb, TRUE32);
    sftl.seaState.vbas_in_free_sb += sftl.vfl->GetVbasPerVb(sb);
    s_sb_set_type(sb, S_SB_ERASED);
    sftl.seaState.erased_sb++;
    s_sb_log_erase(sb);
}

void s_sb_fmt_erase(UInt32 sb)
{
    sftl.vfl->Erase(sb, TRUE32);
    s_sb_set_type(sb, S_SB_ERASED);
    sftl.seaState.erased_sb++;
    sftl.seaState.free_sb++;
    sftl.seaState.vbas_in_free_sb += sftl.vfl->GetVbasPerVb(sb);
}

void s_sb_next(UInt32 stream)
{
    UInt32 chosen;
    s_wrstream_t *wr = &sftl.write.stream[stream];

    // Wear leveling periodicity counter
    s_wearlev_cross_block();

    s_sb_log_next();

    chosen = s_sb_alloc(S_SBSTREAM_DYN == stream);
    sftl.seaState.data_sb++;
    sftl.write.sum_gcDataAdjust -= wr->gcDataAdjust;
    wr->gcDataAdjust = 0;

    if (NULL != wr->BTOC) {
        s_btoc_dealloc(wr->BTOC);
    }
    
    if (0xffffffff != wr->sb) {
        s_sb_set_type(wr->sb, S_SB_DATA);
        if(s_sb_get_num_btoc_vbas(wr->sb) == 0)
        {
            s_bg_enq(wr->sb, TRUE32);
        }
    }

    wr->sb = chosen;
    s_sb_set_type(chosen, S_SB_DATA_CUR);
    if (S_SBSTREAM_STATIC == stream) {
        s_sb_set_static(chosen, 1);
    } else {
        s_sb_set_static(chosen, 0);
    }
    wr->nextVbaOfs = 0;
    wr->maxVbaOfs = sftl.vfl->GetVbasPerVb(chosen);
    wr->BTOC = s_btoc_alloc(chosen);
    wr->BTOCidx = 0;
    wr->BTOC_vba = s_g_vbas_per_page;

    // Set it to have 0 BTOC vbas for existence detection on program fail gc
    s_sb_clr_num_btoc_vbas(chosen);
}

void s_sb_next_from_sb(UInt32 sb)
{
    UInt32 i;

    for (i = 0; i < S_SBSTREAM_MAX; i++) {
        if (sb == sftl.write.stream[i].sb) {
            s_sb_next(i);
            return;
        }
    }

    WMR_PANIC("sb %d not a current write block", sb);
}


void s_sb_sweep0(void)
{
    UInt32 sb, t;

    for (sb = 0; sb < s_g_max_sb; sb++) {
        t = s_sb_get_type(sb);
        if ((0 == s_sb_get_validLbas(sb)) && ((S_SB_DATA == t) || (S_SB_DATA_PENDING_GC == t))) {
            s_sb_set_type(sb, S_SB_ERASE_BEFORE_USE);
            sftl.seaState.free_sb++;
            sftl.seaState.data_sb--;
            sftl.seaState.vbas_in_free_sb += sftl.vfl->GetVbasPerVb(sb);
        }
    }
}

#endif // !AND_READONLY

// Implement L2V functions for reducing validity:
void Outside_L2V_ValidUp(UInt32 vba, UInt32 count)
{
    UInt32 sbN;

    if (L2V_VBA_SPECIAL <= vba)
        return;
    WMR_ASSERT(count < s_g_vbas_per_sb);

    sbN = s_g_vba_to_sb(vba);
    sftl.sb[sbN].validLbas += count;
    sftl.seaState.validLbas += count;
}

void Outside_L2V_ValidDown(UInt32 vba, UInt32 count)
{
    UInt32 sbN;
    UInt32 num_btoc_vbas;

    if (L2V_VBA_SPECIAL <= vba)
        return;
    WMR_ASSERT(count < s_g_vbas_per_sb);

    sbN = s_g_vba_to_sb(vba);
    WMR_ASSERT(sftl.sb[sbN].validLbas >= count);
    WMR_ASSERT(sftl.seaState.validLbas >= count);
    sftl.sb[sbN].validLbas -= count;
    sftl.seaState.validLbas -= count;

    // Zero-valid and not the current block? ... and we're booted?
    if ((0 == sftl.sb[sbN].validLbas) && (S_SB_DATA == sftl.sb[sbN].type) && sftl.booted) {
        // Immediately transition to erasable
        num_btoc_vbas = s_sb_get_num_btoc_vbas(sbN);
        s_sb_set_type(sbN, num_btoc_vbas ? S_SB_ERASE_BEFORE_USE : S_SB_PENDING_ERASE);
        s_sb_set_reads(sbN, 0);
        sftl.seaState.free_sb++;
        sftl.seaState.data_sb--;
        sftl.seaState.vbas_in_free_sb += sftl.vfl->GetVbasPerVb(sbN);
        // Update stats
        sftl.stats.zero_valid_cross++;
#ifndef AND_READONLY        
        s_gc_zeroValidCross();
#endif
    }
}

