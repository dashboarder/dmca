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

#include "s_internal.h"
#include "s_write.h"
#include "s_btoc.h"
#include "s_meta.h"
#include "s_sb.h"
#include "s_gc.h"
#include "s_bg.h"
#include "s_dbg.h"
#include "s_cxt.h"
#include "s_token.h"
#include "s_flatten.h"
#include "s_geom.h"
#include "s_stats.h"
#include "s_trim.h"
#include "s_wearlev.h"

#ifndef AND_READONLY

// Prototypes:
BOOL32 s_write_multi_internal(s_write_multi_t *wm);
BOOL32 handleRagged(BOOL32 front, s_write_multi_t *wm, BOOL32 forceWrite, BOOL32 *wrotePage);

BOOL32 s_write_init(void)
{
    UInt32 i;

    // Initialize to a known state
    for (i = 0; i < S_SBSTREAM_MAX; i++) {
        sftl.write.stream[i].sb = 0xffffffff;
        sftl.write.stream[i].abandon = FALSE32;
        sftl.write.stream[i].pageFlags = 0;
    }

    // Set up the static stream to contain the static flag
    sftl.write.stream[S_SBSTREAM_STATIC].pageFlags = PFLAG_STATIC;

    // Init write-multis
    sftl.write.drain_wm.lba      = (UInt32*)WMR_MALLOC(sizeof(UInt32));
    sftl.write.drain_wm.subCount = (UInt32*)WMR_MALLOC(sizeof(UInt32));
    sftl.write.host_wm.lba       = (UInt32*)WMR_MALLOC(sizeof(UInt32));
    sftl.write.host_wm.subCount  = (UInt32*)WMR_MALLOC(sizeof(UInt32));
    sftl.write.pad_wm.lba      = (UInt32*)WMR_MALLOC(sizeof(UInt32));
    sftl.write.pad_wm.subCount = (UInt32*)WMR_MALLOC(sizeof(UInt32));
    
    if (   (NULL == sftl.write.drain_wm.lba) || (NULL == sftl.write.drain_wm.subCount)
        || (NULL == sftl.write.host_wm.lba)  || (NULL == sftl.write.host_wm.subCount)
        || (NULL == sftl.write.pad_wm.lba)  || (NULL == sftl.write.pad_wm.subCount)
    )
    {
        return FALSE32;
    }

    return TRUE32;
}

void s_write_close(void)
{
    // Free the memory
    if (NULL != sftl.write.drain_wm.lba) {
        WMR_FREE(sftl.write.drain_wm.lba, sizeof(UInt32));
    }
    if (NULL != sftl.write.drain_wm.subCount) {
        WMR_FREE(sftl.write.drain_wm.subCount, sizeof(UInt32));
    }
    if (NULL != sftl.write.host_wm.lba) {
        WMR_FREE(sftl.write.host_wm.lba, sizeof(UInt32));
    }
    if (NULL != sftl.write.host_wm.subCount) {
        WMR_FREE(sftl.write.host_wm.subCount, sizeof(UInt32));
    }
    if (NULL != sftl.write.pad_wm.lba) {
        WMR_FREE(sftl.write.pad_wm.lba, sizeof(UInt32));
    }
    if (NULL != sftl.write.pad_wm.subCount) {
        WMR_FREE(sftl.write.pad_wm.subCount, sizeof(UInt32));
    }
    // Null out the pointers
    sftl.write.drain_wm.lba = NULL;
    sftl.write.drain_wm.subCount = NULL;
    sftl.write.host_wm.lba = NULL;
    sftl.write.host_wm.subCount = NULL;
    sftl.write.pad_wm.lba = NULL;
    sftl.write.pad_wm.subCount = NULL;
}

void s_write_switch(UInt32 stream)
{
    WMR_ASSERT(stream < S_SBSTREAM_MAX);
    sftl.write.curStream = stream;
}

void s_write_host_switch(UInt32 stream)
{
    if (stream != sftl.write.curStream) {
        s_drain_stream_cur(TRUE32);
        s_write_switch(stream);
    }
}

void s_write_gc_switch(UInt32 stream)
{
    if (stream != sftl.write.curStream) {
        s_drain_stream(sftl.write.curStream, FALSE32);
        s_write_switch(stream);
    }
}

void s_write_push_full_buf(BOOL32 doGC)
{
    s_wrstream_t *const wr = &sftl.write.stream[sftl.write.curStream];

    if (wr->bufLbas >= wr->bufSize) {
        s_drain_stream_cur(doGC);
    }
}

void s_drain_stream_cur(BOOL32 doGC)
{
    s_drain_stream(sftl.write.curStream, doGC);
}

BOOL32 s_drain_stream(UInt32 stream, BOOL32 doGC)
{
    BOOL32 wrotePage = FALSE32;
    s_wrstream_t *wr;
    s_write_multi_t *const wm = &sftl.write.drain_wm;

    wr = &sftl.write.stream[stream];

    // MUST BE REENTRANT: called by sftl_shutdown_notify, get a pfail call gc, calls this again

    if (0 == wr->bufLbas) {
        return FALSE32;
    }

    // Switch streams
    s_write_switch(stream);

    WMR_ASSERT(wr->bufLbas <= wr->bufSize);

again:
    // Check if we're already drained (because GC did it for us)
    if (0 == wr->bufLbas) {
        return FALSE32;
    }

    // Allocate a superblock if we don't have one
    if (0xffffffff == wr->sb) {
        s_sb_next(sftl.write.curStream);
    }

    // Pad to page boundary
    while (wr->bufLbas & (s_g_vbas_per_page-1)) {
        s_stats_insert();
    }

    // Set up the wm
    wm->buf = wr->bufPage;
    wm->meta = wr->bufMeta;
    wm->gc = FALSE32;
    wm->flatten = FALSE32;
    wm->count = 0;

    // Push the page
    if (!handleRagged(TRUE32, wm, TRUE32, &wrotePage)) {
        s_write_reseq();
        WMR_ASSERT(wrotePage);
        if (doGC) {
            s_write_handle_pfail();
            goto again;
        } else {
            s_gc_data_enq(sftl.write.stream[sftl.write.curStream].sb, S_GC_CTX_FG);
            s_sb_next_cur();
            goto again;
        }
    }
    WMR_ASSERT(wrotePage);

    return TRUE32;
}

void s_drain_stream_all(BOOL32 doGC)
{
    UInt32 i;
    BOOL32 drained;

    // Drain all streams until they are all clean, so GC doesn't leave any hanging
    do {
        drained = FALSE32;
        for (i = 0; i < S_SBSTREAM_MAX; i++) {
            drained |= s_drain_stream(i, doGC);
        }
    } while (drained);
}

BOOL32 sftl_shutdown_notify(BOOL32 ignoredOption)
{
    UInt16 i;

    s_dbg_check_data_counts();
    s_drain_stream_all(TRUE32);
    s_dbg_check_data_counts();

    // BG shutdown
    s_gc_bg_shutdown();

    // Erase all pending-erase without BTOC
    for (i = 0; i < s_g_max_sb; i++) {
        if (S_SB_PENDING_ERASE == s_sb_get_type(i)) {
            sftl.seaState.vbas_in_free_sb -= sftl.vfl->GetVbasPerVb(i);
            sftl.vfl->Erase(i, TRUE32);
            sftl.seaState.vbas_in_free_sb += sftl.vfl->GetVbasPerVb(i);
            s_sb_set_type(i, S_SB_ERASED);
            s_sb_log_erase(i);
            sftl.seaState.erased_sb++;
        }
    }
    s_dbg_check_data_counts();

    sftl.stats.shutdowns++;

    // Save context
    s_cxt_save();
    s_dbg_check_data_counts();

    return TRUE32;
}

void btoc_terminate(void)
{
    s_wrstream_t *const wr = &sftl.write.stream[sftl.write.curStream];
    wr->BTOC[wr->BTOCidx].count = 0;
}

void s_write_handle_pfail(void)
{
    UInt32 old_sb;
    s_wrstream_t *const wr = &sftl.write.stream[sftl.write.curStream];

    old_sb = wr->sb;
    btoc_terminate();
    s_sb_next_cur();

    if (S_SB_DATA == s_sb_get_type(old_sb)) {
        // Only GC it if it wasn't taken care of by DeleteSectors
        s_gc_data(old_sb, TRUE32);
    }
}

void s_write_reseq(void)
{
    s_wrstream_t *const wr = &sftl.write.stream[sftl.write.curStream];
    UInt32 i;

    // Resequence metadata for buffered content
    for (i = 0; i < wr->bufLbas; i++) {
        META_SET_WEAVESEQ(&wr->bufMeta[i]);
    }
}

Int32 sftl_write(UInt32 lba, UInt32 span, UInt8 *buf, BOOL32 isStatic)
{
    s_write_multi_t *const wm = &sftl.write.host_wm;
    UInt32 origSpan = span;

    WMR_TRACE_4(Write, START, lba, span, buf, isStatic);

    s_dbg_check_data_counts();

    s_debug(WRITE, " sftl_write[start] 0x%x 0x%x isStatic: %s", lba, span, (isStatic)?"YES":"NO");

    sftl.stats.xacts_write++;
    switch (span)
    {
        case 0 : break;     
        case 1 : sftl.stats.lbas_write_1++; 
                 break;
        case 2 : sftl.stats.lbas_write_2++; 
                 break;
        case 3 : sftl.stats.lbas_write_3++; 
                 break;
        case 4 : sftl.stats.lbas_write_4++; 
                 break;
        case 5 : sftl.stats.lbas_write_5++; 
                 break;
        case 6 : sftl.stats.lbas_write_6++; 
                 break;
        case 7 : sftl.stats.lbas_write_7++; 
                 break;
        case 8 : sftl.stats.lbas_write_8++; 
                 break;
        default : sftl.stats.lbas_write_over_8++;
    }

    if ((lba + span) >= sftl.max_lba)
    {
        WMR_TRACE_1(Write, END, ANDErrorCodeHwErr);
        return ANDErrorCodeHwErr;
    }

    s_trim_writeCollide(lba, span);

    wm->blockStartLba = lba;
    wm->blockStartCnt = span;
    wm->blockStartBuf = buf;
recoverPfail:
    // Restore counters
    lba = wm->blockStartLba;
    span = wm->blockStartCnt;

    // Flatten and GC until we're happy about both
    do {
        while (!s_flatten()) {
            // Program failure?
            s_write_reseq();
            s_write_handle_pfail();
        }

        // Do garbage collection prior to the write
        s_gc_prewrite(span);
    } while (L2V_LowMem);

    // Switch to the appropriate stream
    if (isStatic) {
        s_write_host_switch(S_SBSTREAM_STATIC);
        sftl.stats.lbas_written_static += span;
    } else {
        s_write_host_switch(S_SBSTREAM_DYN);
        sftl.stats.lbas_written_dynamic += span;
    }

    // Set up write_multi struct for a single write
    wm->gc = FALSE32;
    wm->flatten = FALSE32;
    wm->lba[0] = wm->blockStartLba;
    wm->subCount[0] = wm->blockStartCnt;
    wm->buf = wm->blockStartBuf;
    wm->meta = sftl.write.meta;
    wm->count = wm->blockStartCnt;
    wm->len = 1;
    wm->cur = 0;

    if (!s_write_multi_internal(wm)) {
        s_write_reseq();
        L2V_Update(wm->blockStartLba, wm->blockStartCnt, L2V_VBA_DEALLOC);
        s_write_handle_pfail();
        goto recoverPfail;
    }

    sftl.stats.lbas_written += origSpan;

    s_wearlev_search(); // find a target if appropriate
    s_bg_write(origSpan);

#if AND_SWISS_DRAIN_AFTER_WRITE
    // Only for EFI/diags...
    s_drain_stream_all(TRUE32);
#endif

    // Periodic cxt save for ANDStats preservation
    s_cxt_periodic();
    
    s_stats_update();

    s_dbg_check_data_counts();

    WMR_TRACE_1(Write, END, ANDErrorCodeOk);

    return ANDErrorCodeOk;
}

static BOOL32 advance_sb(s_write_multi_t *wm)
{
    s_wrstream_t *const wr = &sftl.write.stream[sftl.write.curStream];

    if (s_btoc_isFull()) {
        WMR_ASSERT((wr->nextVbaOfs + wr->BTOC_vba) == wr->maxVbaOfs);
        if (!s_btoc_cross_data()) {
            return FALSE32;
        }
        if(wm->padding) {
            wm->blockStartLba = 0xffffffff;
        } else {
            if (wm->cur < wm->len) {
                wm->blockStartLba = wm->lba[wm->cur];
                wm->blockStartCnt = wm->count;
                wm->blockStartBuf = wm->buf;
            }
        }
    }

    return TRUE32;
}

BOOL32 handleRagged(BOOL32 front, s_write_multi_t *wm, BOOL32 forceWrite, BOOL32 *wrotePage)
{
    s_wrstream_t *wr = &sftl.write.stream[sftl.write.curStream];
    UInt32 i, lba, vba, bufIdx, bufMax, thisCount = 0;

    *wrotePage = FALSE32;

    if (front && (0 == wr->bufLbas)) {
        // Don't buffer anything up
        return TRUE32;
    }
    
    if (((sftl.write.weaveSeq + wm->count) - wr->minWeaveSeq) >= S_WEAVE_SEQ_ADD_TRS) {
        s_sb_next(sftl.write.curStream);
        wr = &sftl.write.stream[sftl.write.curStream];
    }

    bufMax = wr->bufSize;
    if (bufMax >= s_g_vbas_per_stripe) {
        if (wr->bufLbas <= (bufMax - s_g_mod_vbas_per_stripe(wr->nextVbaOfs))) {
            bufMax -= s_g_mod_vbas_per_stripe(wr->nextVbaOfs);
        }
    }

    // Don't buffer too much if we're going to program later anyway
    if (front && (wm->count >= (bufMax - wr->bufLbas) + wr->bufThreshold)) {
        bufMax = S_ROUNDUP_POW2(wr->bufLbas, s_g_vbas_per_page);
    }
    WMR_ASSERT(bufMax <= wr->bufSize);
 
    while ((wr->bufLbas < bufMax) && wm->count) {
        thisCount = WMR_MIN(wm->subCount[wm->cur], bufMax - wr->bufLbas);

        // Put in buffer
        WMR_PERFORM_AES(&wr->bufPage[s_g_mul_bytes_per_lba(wr->bufLbas)], wm->buf, thisCount, s_g_bytes_per_lba);
        if (!wm->gc) {
            s_SetupMeta_Data(&wr->bufMeta[wr->bufLbas], wm->lba[wm->cur], thisCount, wr->pageFlags);
        } else {
            // Copy over flags, userWeaveSeq, lba
            WMR_MEMCPY(&wr->bufMeta[wr->bufLbas], wm->meta, s_g_mul_bytes_per_lba_meta(thisCount));
            s_SetupMeta_DataGC(&wr->bufMeta[wr->bufLbas], thisCount);
        }

        // Next
        wr->bufLbas += thisCount;
        wm->count -= thisCount;
        wm->lba[wm->cur] += thisCount;
        wm->subCount[wm->cur] -= thisCount;
        if (0 == wm->subCount[wm->cur]) {
            wm->cur++;
        }
        wm->buf += s_g_mul_bytes_per_lba(thisCount);
        if (wm->gc) {
            wm->meta += thisCount;
        }
    }

    // Write a buffer?
    WMR_ASSERT(wr->bufLbas <= bufMax);
    if ((wr->bufLbas == bufMax) || forceWrite) {
        bufIdx = 0;
        WMR_ASSERT(0 == (wr->bufLbas & (s_g_vbas_per_page-1)));
        while (bufIdx < wr->bufLbas) {
            // May be necessary to move forward now
            if (!advance_sb(wm)) {
                return FALSE32;
            }

            thisCount = WMR_MIN(wr->bufLbas - bufIdx, (wr->maxVbaOfs - wr->nextVbaOfs - wr->BTOC_vba));
            thisCount = s_btoc_update_size(thisCount);
            thisCount = WMR_MIN(thisCount, (wr->maxVbaOfs - wr->nextVbaOfs - wr->BTOC_vba));

            if (0 == thisCount) {
                // Necessary if BTOC and user write just met
                continue;
            }

            vba = s_g_addr_to_vba(wr->sb, wr->nextVbaOfs);
            *wrotePage = TRUE32;
            if (!sftl.vfl->ProgramMultipleVbas(vba, thisCount, &wr->bufPage[s_g_mul_bytes_per_lba(bufIdx)], (UInt8*)&wr->bufMeta[bufIdx], TRUE32, TRUE32)) {
                WMR_PRINT(QUAL, "sftl: program failure sb:%d vba:0x%x\n", wr->sb, vba);
                return FALSE32;
            }
            for (i = 0; i < thisCount; i++) {
                // Update TOC:
                lba = META_GET_LBA(&wr->bufMeta[bufIdx+i]);
                s_btoc_add_data(vba+i, lba, 1, META_GET_WEAVESEQ(&wr->bufMeta[bufIdx+i]), META_GET_USERSEQ(&wr->bufMeta[bufIdx+i]));
                // Tree: update
                if (L2V_VBA_SPECIAL > lba) {
                    L2V_Update(lba, 1, vba + i);
                } else if (S_TOK_DELETE == lba) {
                    s_trim_markVulnerable(vba);
                }
            }

            // Next
            WMR_ASSERT(wr->nextVbaOfs <= wr->maxVbaOfs);
            bufIdx += thisCount;
        }

        // Buffer is now empty
        wr->bufLbas = 0;
    }

    // Advance superblock if we went far enough
    if (!advance_sb(wm)) {
        return FALSE32;
    }

    return TRUE32;
}

BOOL32 s_write_multi_internal(s_write_multi_t *wm)
{
    UInt32 vba, updCount, updatedNoOfBanks, updatedVbasInSb, updatedMegaStripeSize;
    UInt32 toMegaStripeEnd, toBlockEnd, thisCount, chunk, wmThis, i;
    BOOL32 wrotePage;
    WeaveSeq_t weaveSeq;
    s_wrstream_t *const wr = &sftl.write.stream[sftl.write.curStream];

    // Allocate a superblock if we don't have one
    if ((0xffffffff == wr->sb) || ((wr->sb < s_g_max_sb) && ((sftl.write.weaveSeq - wr->minWeaveSeq) >= S_WEAVE_SEQ_ADD_TRS))) {
        s_sb_next(sftl.write.curStream);
    }

    // Finish prior ragged edge:
    if (!handleRagged(TRUE32, wm, FALSE32, &wrotePage)) {
        return FALSE32;
    } else {
        if((wm->padding) && (wm->blockStartLba != S_TOK_PAD)) {
            return TRUE32;
        }
    }

     // Program enough so the buffer won't be too full in post-write handleRagged
     while (wr->bufThreshold <= wm->count) {
        // Advance superblock?
        if (!advance_sb(wm)) {
            return FALSE32;
        } else {
            if((wm->padding) && (wm->blockStartLba != S_TOK_PAD)) {
                return TRUE32;
            }
        }

        // How much room do we have?
        wmThis = s_btoc_update_size(wm->len - wm->cur);
        thisCount = 0;
        for (i = 0; i < wmThis; i++) {
            thisCount += wm->subCount[wm->cur+i];
        }

        if((updatedVbasInSb = sftl.vfl->GetVbasPerVb(wr->sb)) != (s_g_vbas_per_sb))
        {
            updatedNoOfBanks = updatedVbasInSb / ((s_g_vbas_per_sb) / (s_g_num_banks));
            updatedMegaStripeSize = ROUNDDOWNTO(sftl.write.meta_size, (updatedNoOfBanks * s_g_vbas_per_page));
            WMR_ASSERT(0 != updatedMegaStripeSize);
        }
        else
        {
            updatedNoOfBanks = (s_g_num_banks);
            updatedMegaStripeSize = sftl.write.meta_size;
        }
        // How many this round?  Do up to a megastripe (aligned)
        toMegaStripeEnd = updatedMegaStripeSize - (wr->nextVbaOfs % (updatedNoOfBanks * (s_g_vbas_per_page)));
        toBlockEnd = (wr->maxVbaOfs - wr->nextVbaOfs - wr->BTOC_vba);
        thisCount = WMR_MIN(WMR_MIN(thisCount, wm->count & ~(s_g_vbas_per_page-1)), WMR_MIN(toMegaStripeEnd, toBlockEnd));
  
        if (0 == thisCount) {
            // Necessary if BTOC and user write just met
            continue;
        }
  
        // Generate metadata
        weaveSeq = sftl.write.weaveSeq;
        if (!wm->gc) {
            if(wm->padding) {
                s_SetupMeta_Data_Padding(wm->meta, thisCount, wr->pageFlags);
            }
            else {
                s_SetupMeta_Data(wm->meta, wm->lba[wm->cur], thisCount, wr->pageFlags);
            }
        } else {
            s_SetupMeta_DataGC(wm->meta, thisCount);
            if (!wm->flatten) {
                wr->gcDataAdjust += thisCount;
                sftl.write.sum_gcDataAdjust += thisCount;
            }
        }
  
        // Write the data
        vba = s_g_addr_to_vba(wr->sb, wr->nextVbaOfs);
        if (!sftl.vfl->ProgramMultipleVbas(vba, thisCount, wm->buf, (UInt8*)wm->meta, TRUE32, wm->gc)) {
            WMR_PRINT(QUAL, "sftl: program failure sb:%d vba:0x%x\n", wr->sb, vba);
            return FALSE32;
        }

        updCount = thisCount;
        while (updCount) {
            chunk = WMR_MIN(updCount, wm->subCount[wm->cur]);

            // Update TOC:
            WMR_ASSERT(0 != chunk);
            s_btoc_add_data(vba, wm->lba[wm->cur], chunk, weaveSeq, META_GET_USERSEQ(wm->meta));
      
            // Tree: update
            if(!wm->padding) {
                L2V_Update(wm->lba[wm->cur], chunk, vba);
            }

            // Next
            weaveSeq += chunk;
            vba += chunk;
            updCount -= chunk;
            wm->lba[wm->cur] += chunk;
            wm->subCount[wm->cur] -= chunk;
            if (0 == wm->subCount[wm->cur]) {
                wm->cur++;
            }
        }

        // Next
        wm->count -= thisCount;
        wm->buf += s_g_mul_bytes_per_lba(thisCount);
        if (wm->gc) {
            wm->meta += thisCount;
        }
    }

    // Advance sb, if the last write took us to the end
    if (!advance_sb(wm)) {
        return FALSE32;
    } else {
        if((wm->padding) && (wm->blockStartLba != S_TOK_PAD)) {
            return TRUE32;
        }
    }

    // Buffer up ending ragged edge
    if (wm->count) {
        WMR_ASSERT(0 == wr->bufLbas);
    }
    wrotePage = FALSE32;
    while (wm->count) {
        if (!handleRagged(FALSE32, wm, FALSE32, &wrotePage)) {
            return FALSE32;
        }
    }

    return TRUE32;
}

BOOL32 s_pad_block(UInt32 block)
{
    s_write_multi_t *const wm = &sftl.write.pad_wm;
    UInt8 i;
    
    if ((s_sb_get_num_btoc_vbas(block) > 0) || (S_SB_DATA_CUR != s_sb_get_type(block)))  {
        // Simplify caller logic--no work->success
        return TRUE32;
    }
    
    for (i = 0; i < S_SBSTREAM_MAX; i++) {
        if(block == sftl.write.stream[i].sb) {
            break;
        }
    }
    
    
    if(i < S_SBSTREAM_MAX) {
        s_write_switch(i);
    }
    else {
        return FALSE32;
    }
    
    
    s_drain_stream(i, TRUE32);
    
    if(block != sftl.write.stream[i].sb) {
        return TRUE32;
    }
    
    // Set up the write multi
    wm->blockStartLba = S_TOK_PAD;
    wm->blockStartCnt = WMR_MIN(sftl.gc.zoneSize, s_g_num_banks * s_g_vbas_per_page);
    wm->gc = FALSE32;
    wm->flatten = FALSE32;
    wm->padding = TRUE32;
    wm->buf = sftl.gc.zone;
    wm->meta = sftl.write.meta;
    wm->count = WMR_MIN(sftl.gc.zoneSize, s_g_num_banks * s_g_vbas_per_page);
    wm->subCount[0] = wm->count;
    wm->len = 1;
    wm->cur = 0;
    wm->lba[0] = S_TOK_PAD;
    
    return s_write_multi_internal(wm);
}

#endif // AND_READONLY
