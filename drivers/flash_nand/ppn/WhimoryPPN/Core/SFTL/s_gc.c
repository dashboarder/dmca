/*
 * Copyright (c) 2009-2010 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */

#define AND_TRACE_LAYER FTL

#include "s_gc.h"
#include "s_sb.h"
#include "s_dbg.h"
#include "s_btoc.h"
#include "s_write.h"
#include "s_geom.h"
#include "s_stats.h"

#ifndef AND_READONLY

// Statics
static void      GCMachine_Data(Int32 writeSize, BOOL32 scrubOnUECC, UInt32 ctx);
static void      EraseData(UInt32 sb);
static void      ChooseSB(UInt32 ctx);
static void      SanityCheckValid(UInt32 ctx);
static ANDStatus EvictData(UInt32 minAmount, BOOL32 scrubOnUECC, UInt32 ctx);
static void      ZoneSetup(BOOL32 scrubOnUECC);
static void      ZoneAdd(UInt32 lba, UInt32 vba, UInt32 count);
static BOOL32    MoveZone(BOOL32 dynStream, BOOL32 forFlatten, BOOL32 scrubOnUECC, UInt32 ctx);
static void      ReadZone_cb(UInt32 vba, VFLReadStatusType status, UInt8 *meta);
static void      ReadZone(BOOL32 scrubOnUECC, UInt32 ctx);
static UInt32    AnalyzeZone(void);


BOOL32 s_WorkFifo_isEmpty(GC_Fifo_t *f)
{
    return f->tail == f->head;
}

BOOL32 s_WorkFifo_isFullish(GC_Fifo_t *f)
{
    Int32 space;

    if (s_WorkFifo_isEmpty(f)) {
        return FALSE32;
    }

    if (f->tail > f->head) {
        space = f->tail - f->head;
    } else {
        space = f->tail + (S_GCFIFO_DEPTH+1 - f->head);
    }

    WMR_ASSERT(space > 0);

    return (space < S_GCFIFO_FULL);
}

void s_WorkFifo_Enq(GC_Fifo_t *f, UInt32 sb)
{
    UInt32 p;
    UInt32 newTail;

    newTail = f->tail + 1;
    if (newTail > S_GCFIFO_DEPTH)
    {
        newTail = 0;
    }
    if (newTail == f->head) {
        // FIFO is full, get out
        return;
    }

    p = f->head;
    while (p != f->tail)
    {
        if (f->sb[p] == sb)
        {
            return;
        }

        p++;
        if (p > S_GCFIFO_DEPTH)
        {
            p = 0;
        }
    }

    f->sb[f->tail] = sb;
    f->tail = newTail;
    WMR_ASSERT(f->tail != f->head);
}

UInt32 s_WorkFifo_Deq(GC_Fifo_t *f)
{
    UInt32 ret;

    WMR_ASSERT(f->tail != f->head);

    ret = f->sb[f->head];

    f->head++;
    if (f->head > S_GCFIFO_DEPTH)
    {
        f->head = 0;
    }

    return ret;
}

UInt32 s_WorkFifo_Deq_sb(GC_Fifo_t *f, UInt32 sb)
{
    UInt32 p;
    UInt32 nRet = (UInt32)-1;
    p = f->head;
    
    while (p != f->tail)
    {
        if (f->sb[p] == sb)
        {
            nRet = f->sb[p];
            break;
        }
        
        p++;
        if (p > S_GCFIFO_DEPTH)
        {
            p = 0;
        }
    }
    
    if(p == f->tail)
        return nRet;
    
    while (p != f->tail)
    {
        if(p == S_GCFIFO_DEPTH)
        {
            f->sb[p]=f->sb[0];
            p=0;
        }
        else
        {
            f->sb[p]=f->sb[p+1];
            p++;
        }
    }
    
    if(f->tail == 0)
        f->tail = S_GCFIFO_DEPTH;
    else
        f->tail--;
    
    return nRet;
}

// Functions

#endif // AND_READONLY

BOOL32 s_gc_init(void)
{
    UInt32 ctx;

    // GC Zone size should be a multiple of banks, up to 16 (or whatever banks is naturally).
    // This logic guarantees low-bank configs will have higher performance, while making sure
    // high-bank configs don't occupy too much memory.
    // It is similar to setting a minimum of 16, but works with non-pow-2 configs.
    sftl.gc.zoneSize = s_g_num_banks * s_g_vbas_per_page;
    while (sftl.gc.zoneSize < S_GCZONE_DOUBLEUPTO) {
        sftl.gc.zoneSize <<= 1;
    }
    WMR_ASSERT(0 == (sftl.gc.zoneSize % s_g_vbas_per_page));

    L2V_Search_Init(&sftl.gc.read_c);
    for (ctx = 0; ctx < S_GC_NUM_CTX; ctx++) {
        sftl.gc.ctx[ctx].state = GCD_IDLE;
        sftl.gc.ctx[ctx].in_sb = 0xffffffff;
    }
    

    WMR_BufZone_Init(&sftl.gc.BufZone);
    WMR_ASSERT(0 != s_g_max_pages_per_btoc);
    for (ctx = 0; ctx < S_GC_NUM_CTX; ctx++) {
        sftl.gc.ctx[ctx].BTOC = (s_btoc_entry_t*)WMR_Buf_Alloc_ForDMA(&sftl.gc.BufZone, s_g_max_pages_per_btoc * s_g_bytes_per_page);
    }
    sftl.gc.meta_buf = (PageMeta_t*)WMR_Buf_Alloc_ForDMA(&sftl.gc.BufZone, sizeof(PageMeta_t));
    sftl.gc.zone     = (UInt8*)WMR_Buf_Alloc_ForDMA(&sftl.gc.BufZone, s_g_bytes_per_lba * sftl.gc.zoneSize);
    sftl.gc.meta     = (PageMeta_t*)WMR_Buf_Alloc_ForDMA(&sftl.gc.BufZone, sftl.gc.zoneSize * sizeof(PageMeta_t));

    if (!WMR_BufZone_FinishedAllocs(&sftl.gc.BufZone)) {
        return FALSE32;
    }

    for (ctx = 0; ctx < S_GC_NUM_CTX; ctx++) {
        WMR_BufZone_Rebase(&sftl.gc.BufZone, (void**)&sftl.gc.ctx[ctx].BTOC);
    }
    WMR_BufZone_Rebase(&sftl.gc.BufZone, (void**)&sftl.gc.meta_buf);
    WMR_BufZone_Rebase(&sftl.gc.BufZone, (void**)&sftl.gc.zone);
    WMR_BufZone_Rebase(&sftl.gc.BufZone, (void**)&sftl.gc.meta);
    WMR_BufZone_FinishedRebases(&sftl.gc.BufZone);

    sftl.gc.vbas  = (UInt32*)WMR_MALLOC(sftl.gc.zoneSize * sizeof(UInt32));
    sftl.gc.lbas  = (UInt32*)WMR_MALLOC(sftl.gc.zoneSize * sizeof(UInt32));
    sftl.gc.uecc  = (UInt32*)WMR_MALLOC(((sftl.gc.zoneSize+31)/32) * sizeof(UInt32));

    // Init write-multi
    sftl.gc.wm.lba = (UInt32*)     WMR_MALLOC(sftl.gc.zoneSize * sizeof(UInt32));
    sftl.gc.wm.subCount = (UInt32*)WMR_MALLOC(sftl.gc.zoneSize * sizeof(UInt32));

    for (ctx = 0; ctx < S_GC_NUM_CTX; ctx++) {
        if (NULL == sftl.gc.ctx[ctx].BTOC) {
            return FALSE32;
        }
    }
    if ((NULL == sftl.gc.meta_buf)
        || (NULL == sftl.gc.zone)     || (NULL == sftl.gc.meta)
        || (NULL == sftl.gc.lbas)     || (NULL == sftl.gc.vbas)
        || (NULL == sftl.gc.uecc)
        || (NULL == sftl.gc.wm.lba)  || (NULL == sftl.gc.wm.subCount)
    )
    {
        return FALSE32;
    }

    return TRUE32;
}

void s_gc_close()
{
    WMR_BufZone_Free(&sftl.gc.BufZone);

    if (NULL != sftl.gc.lbas) {
        WMR_FREE(sftl.gc.lbas, sftl.gc.zoneSize * sizeof(UInt32));
    }
    if (NULL != sftl.gc.vbas) {
        WMR_FREE(sftl.gc.vbas, sftl.gc.zoneSize * sizeof(UInt32));
    }
    if (NULL != sftl.gc.uecc) {
        WMR_FREE(sftl.gc.uecc, ((sftl.gc.zoneSize+31)/32) * sizeof(UInt32));
    }
    if (NULL != sftl.gc.wm.lba) {
        WMR_FREE(sftl.gc.wm.lba, sftl.gc.zoneSize * sizeof(UInt32));
    }
    if (NULL != sftl.gc.wm.subCount) {
        WMR_FREE(sftl.gc.wm.subCount, sftl.gc.zoneSize * sizeof(UInt32));
    }

    WMR_MEMSET(&sftl.gc, 0, sizeof(sftl.gc));
}

#ifndef AND_READONLY

void s_gc_for_flatten(UInt32 writeSize)
{
    while (dangerousData(writeSize))
    {
        GCMachine_Data(writeSize, TRUE32, S_GC_CTX_FG);
    }
}


BOOL32 s_gc_slice(UInt32 ctx)
{
    UInt32 writeSize = sftl.gc.zoneSize;
    BOOL32 status = FALSE32;
    
    s_dbg_check_data_counts();
    s_dbg_check_sb_dist();
    sftl.gc.ctx[ctx].in_sb = 0xffffffff;
    
    // Below low low low mark?
    if ((ctx == S_GC_CTX_FG) && dangerousData(writeSize))
    {
        GCMachine_Data(writeSize, TRUE32, ctx);
        status = TRUE32;
    }
    
    s_dbg_check_sb_dist();
    
    // If we're low on space, or the garbage collector is not idle, (-> this is important to not cause hangs)
    if ((status == FALSE32) && (((ctx == S_GC_CTX_FG) && idleLowData(writeSize)) || (GCD_IDLE != sftl.gc.ctx[ctx].state) || !s_WorkFifo_isEmpty(&sftl.gc.ctx[ctx].workFifo)))
    {
        GCMachine_Data(writeSize, TRUE32, ctx);
        status = TRUE32;
    }
    
    s_dbg_check_sb_dist();
    return status;
}

void s_gc_prewrite(UInt32 writeSize)
{
    s_dbg_check_data_counts();
    s_dbg_check_sb_dist();
    sftl.gc.ctx[S_GC_CTX_FG].in_sb = 0xffffffff;

    // Below low low low mark?
    while (dangerousData(writeSize))
    {
        GCMachine_Data(writeSize, TRUE32, S_GC_CTX_FG);
    }

    s_dbg_check_sb_dist();

    // If we're low on space, or the garbage collector is not idle, (-> this is important to not cause hangs)
    if (lowData(writeSize) || (GCD_IDLE != sftl.gc.ctx[S_GC_CTX_FG].state) || !s_WorkFifo_isEmpty(&sftl.gc.ctx[S_GC_CTX_FG].workFifo))
    {
        GCMachine_Data(writeSize, TRUE32, S_GC_CTX_FG);
    }

    s_dbg_check_sb_dist();
}

void s_gc_bg(UInt32 writeSize)
{
    s_dbg_check_data_counts();
    s_dbg_check_sb_dist();
    sftl.gc.ctx[S_GC_CTX_BG].in_sb = 0xffffffff;
    
    // Below low low low mark?
    if (dangerousData(writeSize))
    {
        // Get out
        return;
    }
    
    s_dbg_check_sb_dist();
    
    // If the garbage collector is not idle, (-> this is important to not cause hangs)
    if ((GCD_IDLE != sftl.gc.ctx[S_GC_CTX_BG].state) || !s_WorkFifo_isEmpty(&sftl.gc.ctx[S_GC_CTX_BG].workFifo))
    {
        GCMachine_Data(writeSize, TRUE32, S_GC_CTX_BG);
    }

    s_dbg_check_sb_dist();
}

void s_gc_bg_shutdown(void)
{
    s_dbg_check_data_counts();
    s_dbg_check_sb_dist();
    sftl.gc.ctx[S_GC_CTX_BG].in_sb = 0xffffffff;

    // If we're low on space, or the garbage collector is not idle, (-> this is important to not cause hangs)
    while ((GCD_IDLE != sftl.gc.ctx[S_GC_CTX_BG].state) || !s_WorkFifo_isEmpty(&sftl.gc.ctx[S_GC_CTX_BG].workFifo))
    {
        // Below low low low mark?
        if (dangerousData(0))
        {
            // Get out
            return;
        }
        
        s_dbg_check_sb_dist();

        GCMachine_Data(0, TRUE32, S_GC_CTX_BG);
    }
    
    s_dbg_check_sb_dist();
}

static void MoveOnIfCurrentD(UInt32 inBlock)
{
    // Is it the current data block?
    if (S_SB_DATA_CUR == sftl.sb[inBlock].type)
    {
        s_sb_next_from_sb(inBlock);
    }
}


// Data state machine:
// 1) choose block
// 2) read TOC or reconstruct
// 3) [loop] find valid spans, move preferentially in chunks of sequentiality
// 4) sanity check
// 5) erase block


static void GCMachine_Data(Int32 writeSize, BOOL32 scrubOnUECC, UInt32 ctx)
{
    GC_Ctx_t *const c = &sftl.gc.ctx[ctx];
    ANDStatus status;
    UInt32 cost;

    sftl.gc.inProgress = TRUE32;
 again:
    WMR_TRACE_IST_3(GCMachine, START, writeSize, scrubOnUECC, c->state);
    switch (c->state)
    {
    case GCD_IDLE:
        if (!idleLowData(writeSize) && s_WorkFifo_isEmpty(&c->workFifo) && (0xffffffff == c->in_sb)) {
            // Nothing to do... get out
            break;
        }
        if ((S_GC_CTX_BG == ctx) && s_WorkFifo_isEmpty(&c->workFifo)) {
            // BG writes shouldn't search for invalid
            break;
        }

        sftl.stats.data_gc++;
        ChooseSB(ctx);
        if (0 == c->chosenValid)
        {
            sftl.stats.zero_valid_cross++;
            c->state = GCD_ERASE;
            WMR_TRACE_IST_0(GCMachine, END);
            goto again;
        }

        c->vba = s_g_addr_to_vba(c->sb, 0);
        c->done = FALSE32;
        s_btoc_read(c->sb, c->BTOC, &c->BTOC_len, FALSE32, scrubOnUECC);
        c->curBteIdx = 0;
        s_sb_set_type(c->sb, S_SB_DATA_GC);
        c->state = GCD_EVICT;
        c->ueccCounter = 0;
        // Drain so GC batches prefer to be aligned
        s_drain_stream_all(FALSE32);

        // Fall through to GCD_EVICT

    case GCD_EVICT:
        // Check if it's been invalidated by host writes since we entered the state machine
        if (0 == s_sb_get_validLbas(c->sb))
        {
            // And short-circuit to the ERASE state
            c->state = GCD_ERASE;
            WMR_TRACE_IST_0(GCMachine, END);
            goto again;
        }

        cost = sftl.gc.zoneSize;
        if (S_GC_CTX_BG == ctx) {
            // BG writes should be 10% of the bandwidth, rounded up
            cost = (writeSize / 10) + sftl.gc.zoneSize - 1;
            cost -= cost % sftl.gc.zoneSize;
        } else if (writeSize > 0)  {
            if (c->invalidHistSize) {
                cost = (c->invalidHistSum / c->invalidHistSize); // Divisor
            } else {
                cost = 1;
            }
            if (!cost)
            {    
                cost = (writeSize * c->chosenSize); // Avoid div0 when we move a full block
            }
            cost = (writeSize * c->chosenSize) / cost;
            // Round up to a zone boundary (makes no sense to do less!)
            cost += sftl.gc.zoneSize - 1; 
            cost -= cost % sftl.gc.zoneSize;
        }

        s_drain_stream_all(FALSE32); // for coherency
        status = EvictData(cost, scrubOnUECC, ctx);
        if (ANDErrorCodeOk != status)
        {
            if (GCD_EVICT == c->state) {
                c->state = GCD_IDLE;
            }
            WMR_TRACE_IST_0(GCMachine, END);
            goto again;
        }
        if ((0 == s_sb_get_validLbas(c->sb)) || c->done)
        {
            if (0 != s_sb_get_validLbas(c->sb)) {
                // Drain, to make sure all LBAs get written and thus L2V updated
                s_drain_stream_all(FALSE32);
            }
            SanityCheckValid(ctx);

            // Short-circuit to the ERASE state
            c->state = GCD_ERASE;
            WMR_TRACE_IST_0(GCMachine, END);
            goto again;
        }
        break;

    case GCD_ERASE:
        SanityCheckValid(ctx);
        EraseData(c->sb);
        c->state = GCD_IDLE;
        break;

    default:
        WMR_PANIC("unknown gc state %d", c->state);
    }
    WMR_TRACE_IST_0(GCMachine, END);
}

void s_gc_data_enq(UInt32 sb, UInt32 ctx)
{
    WMR_TRACE_IST_1(GCDataEnq, NONE, sb);
    s_WorkFifo_Enq(&sftl.gc.ctx[ctx].workFifo, sb);
}

ANDStatus s_gc_data(UInt32 sb, BOOL32 scrubOnUECC)
{
    UInt32 ctx = S_GC_CTX_FG;

    WMR_TRACE_IST_2(GCData, START, sb, scrubOnUECC);

    s_dbg_check_sb_dist();

    // Kill any previous GC operations
    // This makes GC recovery from write fail more safe
    if (GCD_IDLE != sftl.gc.ctx[S_GC_CTX_FG].state) {
        // Un-mark pending GC operations
        if ((0xffffffff != sftl.gc.ctx[S_GC_CTX_FG].sb) && (S_SB_DATA_GC == sftl.sb[sftl.gc.ctx[S_GC_CTX_FG].sb].type)) {
            sftl.sb[sftl.gc.ctx[S_GC_CTX_FG].sb].type = S_SB_DATA;
        }
    }
    sftl.gc.ctx[ctx].state = GCD_IDLE;

again:
    // Set up the new one
    sftl.gc.ctx[ctx].in_sb = sb;

    // And start it
    do
    {
        GCMachine_Data(0, scrubOnUECC, ctx);
    }
    while (GCD_IDLE != sftl.gc.ctx[ctx].state);

    if (!s_WorkFifo_isEmpty(&sftl.gc.ctx[ctx].workFifo))
    {
        sb = s_WorkFifo_Deq(&sftl.gc.ctx[ctx].workFifo);
        goto again;
    }

    s_dbg_check_sb_dist();

    WMR_TRACE_IST_1(GCData, END, ANDErrorCodeOk);
    sftl.gc.inProgress = FALSE32;
    return ANDErrorCodeOk;
}

BOOL32 s_gc_move_advisable(void)
{
    return !dangerousData(s_g_vbas_per_sb);
}

static void ChooseSB(UInt32 ctx)
{
    UInt32 i, type;
    UInt32 thisValid, thisMerit, thisErases;
    UInt32 best, bestMerit, bestValid, bestErases;
    GC_Ctx_t *c = &sftl.gc.ctx[ctx];

    best = bestMerit = bestValid = bestErases = 0xFFFFFFFF;

    if (0xffffffff != c->in_sb) {
        MoveOnIfCurrentD(c->in_sb);
    }

    if (!s_WorkFifo_isEmpty(&c->workFifo))
    {
        if (0xffffffff != c->in_sb)
        {
            s_WorkFifo_Enq(&c->workFifo, c->in_sb);
        }
        c->in_sb = s_WorkFifo_Deq(&c->workFifo);
    } else {
        // BG shouldn't have any choice
        WMR_ASSERT(S_GC_CTX_BG != ctx);
    }

    if (0xffffffff != c->in_sb)
    {
        c->sb = c->in_sb;
        c->chosenValid = s_sb_get_validLbas(c->in_sb);
        c->chosenErases = s_sb_get_erases(c->in_sb);
        type = s_sb_get_type(c->in_sb);
        WMR_ASSERT((type == S_SB_DATA) || (type == S_SB_DATA_CUR) || (type == S_SB_DATA_GC) || (type == S_SB_DATA_PENDING_GC));
        c->in_sb = 0xffffffff;
        MoveOnIfCurrentD(c->sb);
        return;
    }

    for (i = 0; i < s_g_max_sb; i++)
    {
        // Not a data sb?  Don't consider...
        if ((S_SB_DATA != s_sb_get_type(i))
            && (S_SB_DATA_PENDING_GC != s_sb_get_type(i)))
        {
            continue;
        }

        // Grab local copies
        thisValid = s_sb_get_validLbas(i);
        thisMerit = (thisValid * s_g_vbas_per_sb) / sftl.vfl->GetVbasPerVb(i);
        thisErases = s_sb_get_erases(i);

        // Minimize merit (sounds backwards, I know)
        if ((thisMerit < bestMerit) || ((thisMerit == bestMerit) && (thisErases < bestErases)))
        {
            best = i;
            bestMerit = thisMerit;
            bestValid = thisValid;
            bestErases = thisErases;
        }
    }

    WMR_ASSERT(best != 0xFFFFFFFF);
    WMR_ASSERT(bestMerit != 0xFFFFFFFF);
    WMR_ASSERT(bestErases != 0xFFFFFFFF);

    // Output
    c->sb = best;
    c->chosenMerit = bestMerit;
    c->chosenValid = bestValid;
    c->chosenErases = bestErases;
    c->chosenSize = sftl.vfl->GetVbasPerVb(best);

    // Invalid history
    c->invalidHistSum -= c->invalidHist[c->invalidHistIdx];
    c->invalidHist[c->invalidHistIdx] = c->chosenSize - c->chosenValid;
    c->invalidHistSum += c->chosenSize - c->chosenValid;
    c->invalidHistIdx++;
    if (c->invalidHistIdx >= S_GC_VALIDHIST_SIZE) {
        c->invalidHistIdx = 0;
    }
    if (c->invalidHistSize < S_GC_VALIDHIST_SIZE) {
        c->invalidHistSize++;
    }
}

static void EraseData(UInt32 sb)
{
    UInt32 num_btoc_vbas;

    WMR_ASSERT(0 == s_sb_get_validLbas(sb));
    
    if((S_SB_ERASED == s_sb_get_type(sb)) || (S_SB_PENDING_ERASE == s_sb_get_type(sb)) || (S_SB_ERASE_BEFORE_USE == s_sb_get_type(sb)))
        return;

    // Set erase as pending
    s_sb_set_reads(sb, 0);
    s_sb_set_validLbas(sb, 0);
    num_btoc_vbas = s_sb_get_num_btoc_vbas(sb);
    s_sb_set_type(sb, num_btoc_vbas ? S_SB_ERASE_BEFORE_USE : S_SB_PENDING_ERASE);

    // Manage free/allocated counters
    sftl.seaState.free_sb++;
    sftl.seaState.data_sb--;
    sftl.seaState.vbas_in_free_sb += sftl.vfl->GetVbasPerVb(sb);
}

UInt32 findLba(UInt32 vba, UInt32 ctx)
{
    GC_Ctx_t *const c = &sftl.gc.ctx[ctx];
    L2V_SearchCtx_t *const read_c = &c->uecc_c;
    UInt32 read_vbaEnd, lba = 0;

    while (lba < sftl.max_lba) {
        read_c->lba = lba;
        L2V_Search(read_c);
        read_vbaEnd = read_c->vba + read_c->span;

        // Hit?
        if ((read_c->vba <= vba) && (vba < read_vbaEnd)) {
            return lba + (vba - read_c->vba);
        }

        // Next
        lba += read_c->span;
    }

    return S_TOK_UECC;
}

void findVbas(UInt32 vbaBase, UInt32 count, BOOL32 trimIfFound)
{
    L2V_SearchCtx_t *const read_c = &sftl.gc.read_c;
    UInt32 read_vbaEnd, lba = 0;
    UInt32 vbaEnd = vbaBase + count;

    while (lba < sftl.max_lba) {
        read_c->lba = lba;
        L2V_Search(read_c);
        read_vbaEnd = read_c->vba + read_c->span;

        if ((vbaBase < read_vbaEnd) && (read_c->vba < vbaEnd))
        {
            WMR_PRINT(ALWAYS, "Range hit: lba %d, search 0x%x-0x%x, found 0x%x-0x%x\n", lba, vbaBase, vbaEnd, read_c->vba, read_vbaEnd);

            if(trimIfFound)
            {
                UInt32 adjustedLba = read_c->lba;
                UInt32 adjustedCount = read_c->span;

                if(read_c->vba < vbaBase)
                {
                    adjustedLba += (vbaBase - read_c->vba);
                    adjustedCount -= (vbaBase - read_c->vba);
                }
                if(read_vbaEnd > vbaEnd)
                {
                    adjustedCount -= (read_vbaEnd - vbaEnd);
                }
                L2V_Update(adjustedLba, adjustedCount, L2V_VBA_DEALLOC);
            }
        }        
        // Next
        lba += read_c->span;
    }
}

static void SanityCheckValid(UInt32 ctx)
{
    GC_Ctx_t *const c = &sftl.gc.ctx[ctx];

    // Sanity check: should now be empty...
    if (c->ueccCounter < sftl.sb[c->sb].validLbas)
    {
        s_debug(ERROR, "GC didn't fully evict (valid:%d); finding missing data and cross-checking counters...\n", sftl.sb[c->sb].validLbas);

        findVbas(s_g_addr_to_vba(c->sb, 0), s_g_vbas_per_sb, FALSE32);

        s_dbg_check_validSums();
        s_dbg_check_data_counts();

        WMR_PANIC("Non-zero validity counter Block %d: %d UECC: %d", 
                  c->sb, 
                  (UInt32) sftl.sb[c->sb].validLbas, 
                  c->ueccCounter);
    }
    else
    {
        if(sftl.sb[c->sb].validLbas)
        {
            // Some data gone missing due to UECCs. Adjust validLba counter
            findVbas(s_g_addr_to_vba(c->sb, 0), s_g_vbas_per_sb, TRUE32);
            sftl.seaState.validLbas -= sftl.sb[c->sb].validLbas;
            sftl.sb[c->sb].validLbas = 0;
        }
    }
}

BOOL32 s_gc_inject(UInt32 lba, UInt32 count)
{
    L2V_SearchCtx_t *const read_c = &sftl.gc.read_c;
    UInt32 origCount = count, thisCount, curCount;

    while (count) {
        thisCount = WMR_MIN(count, sftl.gc.zoneSize - sftl.gc.curZoneSize);

        // Set up zone
        ZoneSetup(TRUE32);
        while (sftl.gc.curZoneSize < thisCount) {
            read_c->lba = lba;
            L2V_Search(read_c);
            curCount = WMR_MIN(thisCount - sftl.gc.curZoneSize, read_c->span);
            ZoneAdd(lba, read_c->vba, curCount);

            // Next
            lba += curCount;
            count -= curCount;
        }

        // Move it
        if (!MoveZone(TRUE32, TRUE32, TRUE32, S_GC_CTX_FG)) {
            return FALSE32;
        }
    }

    // Compensate
    sftl.stats.lbas_gc -= origCount;

    return TRUE32;
}

static ANDStatus EvictData(UInt32 minAmount, BOOL32 scrubOnUECC, UInt32 ctx)
{
    GC_Ctx_t *const c = &sftl.gc.ctx[ctx];
    L2V_SearchCtx_t *const read_c = &sftl.gc.read_c;
    UInt32 evictedLbas = 0;
    UInt32 thisCount;
    s_btoc_entry_t *bte;
    UInt32 status;

    // Clear out search cxt since things have happened since the last call
    ZoneSetup(scrubOnUECC);

    do {
        // Consider the current bte
        bte = &c->BTOC[c->curBteIdx];
        if ((c->curBteIdx >= c->BTOC_len) || (0 == s_sb_get_validLbas(c->sb))) {
            // Reached the end of the BTOC, or moved all of the valid data?
            c->done = TRUE32;
            goto evictExit;
        }

        // uECC?
        if (S_TOK_UECC == bte->lba) {
            // Find which LBA lives here:
            bte->lba = findLba(c->vba, ctx);
        }

        // Search tree
        if (S_TOKEN_LBA_BASE <= bte->lba) {
            read_c->vba = 0xffffffff;
            read_c->span = bte->count;
        } else if ((0 == read_c->span) || (read_c->lba != bte->lba)) {
            read_c->lba = bte->lba;
            L2V_Search(read_c);
        }
        WMR_ASSERT(0 != read_c->span);

        thisCount = WMR_MIN((UInt32)bte->count, (UInt32)read_c->span);
        if (read_c->vba == c->vba) {
            // Hit!
            // How much can we read into the zone?
            thisCount = WMR_MIN(thisCount, sftl.gc.zoneSize - sftl.gc.curZoneSize);

            // Throw spans into zone
            WMR_ASSERT(0 != thisCount);
            ZoneAdd(bte->lba, c->vba, thisCount);
            evictedLbas += thisCount;
        }

        // Next
        bte->count -= thisCount;
        if (S_TOKEN_LBA_BASE > bte->lba) {
            bte->lba += thisCount;
        }
        c->vba += thisCount;
        read_c->span -= thisCount;

        // Move bte along?
        if (0 == bte->count) {
            c->curBteIdx++;
        }

evictExit:
        // Zone full, or done?
        if (c->done || (sftl.gc.curZoneSize == sftl.gc.zoneSize) || (evictedLbas >= minAmount)) {
            status = MoveZone(FALSE32, FALSE32, scrubOnUECC, ctx);
            if (!status) {
                s_write_reseq();
                s_WorkFifo_Enq(&c->workFifo, c->sb);
                s_WorkFifo_Enq(&c->workFifo, sftl.write.stream[sftl.write.curStream].sb);
                s_sb_next_cur();
                return ANDErrorCodeHwErr;
            }
            ZoneSetup(scrubOnUECC);
        }
    } while ((evictedLbas < minAmount) && !c->done);

    return ANDErrorCodeOk;
}

static void ZoneSetup(BOOL32 scrubOnUECC)
{
    s_write_multi_t *const wm = &sftl.gc.wm;
    L2V_SearchCtx_t *const read_c = &sftl.gc.read_c;
    UInt32 i;

    wm->len = 0;
    sftl.gc.curZoneSize = 0;
    for (i = 0; i < sftl.gc.zoneSize; i += 32) {
        sftl.gc.uecc[i>>5] = 0;
    }
    read_c->span = 0;
    sftl.vfl->ReadSpansInit(&sftl.gc.rs, ReadZone_cb, (VFL_READ_STATUS_UECC|VFL_READ_STATUS_CLEAN), TRUE32, scrubOnUECC);
}

static void ZoneAdd(UInt32 lba, UInt32 vba, UInt32 count)
{
    s_write_multi_t *const wm = &sftl.gc.wm;
    s_wrstream_t *const wr = &sftl.write.stream[sftl.write.curStream];
    VFL_ReadSpans_t *const rs = &sftl.gc.rs;
    UInt32 i;

    WMR_ASSERT(0 != count);

    if (L2V_VBA_SPECIAL > vba) {
        // Real data; read spattered
        sftl.vfl->ReadSpansAdd(rs, vba, count, &sftl.gc.zone[s_g_mul_bytes_per_lba(sftl.gc.curZoneSize)], (UInt8*)&sftl.gc.meta[sftl.gc.curZoneSize]);
        for (i = 0; i < count; i++) {
            sftl.gc.lbas[sftl.gc.curZoneSize+i] = lba + i;
            sftl.gc.vbas[sftl.gc.curZoneSize+i] = vba + i;
        }
    } else {
        // Trimmed content?  Setup metadata
        WMR_ASSERT(L2V_VBA_DEALLOC == vba);
        s_SetupMeta_Trim(&sftl.gc.meta[sftl.gc.curZoneSize], lba, count, wr->pageFlags);
        WMR_WRITE_TRIM(&sftl.gc.zone[s_g_mul_bytes_per_lba(sftl.gc.curZoneSize)], s_g_bytes_per_lba, count);
        for (i = 0; i < count; i++) {
            sftl.gc.lbas[sftl.gc.curZoneSize+i] = lba + i;
            sftl.gc.vbas[sftl.gc.curZoneSize+i] = 0xffffffff;
        }
    }

    // Add to write multi
    wm->lba[wm->len] = lba;
    wm->subCount[wm->len] = count;
    wm->len++;

    // Accumulate to zone size
    sftl.gc.curZoneSize += count;
}

static BOOL32 MoveZone(BOOL32 dynStream, BOOL32 forFlatten, BOOL32 scrubOnUECC, UInt32 ctx)
{
    s_write_multi_t *const wm = &sftl.gc.wm;
    UInt32 staticness;

    if (0 == sftl.gc.curZoneSize) {
        // Simplify caller logic--no work->success
        return TRUE32;
    }

    // Read the GC zone
    ReadZone(scrubOnUECC, ctx);

    // Analyze zone for staticness, and lba mismatches; switch stream at 50% point
    staticness = AnalyzeZone();
    if (dynStream) {
        staticness = 0;
    }
    if (S_GC_CTX_FG == ctx) {
        s_write_gc_switch(((staticness << 1) >= sftl.gc.curZoneSize) ? S_SBSTREAM_STATIC : S_SBSTREAM_DYN);
    } else {
        s_write_gc_switch(S_SBSTREAM_WEARLEV);
    }

    // Set up the write multi
    wm->gc = TRUE32;
    wm->flatten = forFlatten;
    wm->buf = sftl.gc.zone;
    wm->meta = sftl.gc.meta;
    wm->count = sftl.gc.curZoneSize;
    wm->cur = 0;
    wm->padding = FALSE32;

    sftl.stats.lbas_gc += sftl.gc.curZoneSize;
    sftl.gc.curZoneSize = 0;

    return s_write_multi_internal(wm);
}

static void ScrubZone(void)
{
    UInt32 i, lba;

    for (i = 0; i < sftl.gc.curZoneSize; i++) {
        lba = META_GET_LBA(&sftl.gc.meta[i]);
        // LBA in page out of range?
        if (lba >= sftl.max_lba) {
            // Stomp it
            if (WMR_I_CAN_HAZ_DEBUGGER()) {
                if (!(((sftl.gc.uecc[i>>5]) >> (i&31)) & 1)) {
                    WMR_PANIC("lba:%d out of range in GC; stomping to lba:%d", lba, sftl.gc.lbas[i]);
                }
            } else {
                WMR_PRINT(ALWAYS, "lba:%d out of range in GC; stomping to lba:%d", lba, sftl.gc.lbas[i]);
            }
            s_SetupMeta_Data_UECC(&sftl.gc.meta[i], sftl.gc.lbas[i]);
        }
    }
}

static void ReadZone_cb(UInt32 vba, VFLReadStatusType status, UInt8 *meta)
{
    UInt32 idx;

    if (status & VFL_READ_STATUS_UNIDENTIFIED) {
        WMR_PRINT(QUAL_FATAL, "gc read got unidentified read status (0x%x) on vba:0x%x; did we get a timeout?  Pretending it was uECC.\n", status, vba);
        status = (status & ~VFL_READ_STATUS_UNIDENTIFIED) | VFL_READ_STATUS_UECC;
    }

    if ((status & VFL_READ_STATUS_UECC) || (status & VFL_READ_STATUS_CLEAN)) {
        idx = ((PageMeta_t*)meta) - sftl.gc.meta;
        sftl.gc.uecc[idx>>5] |= (1 << (idx&31));
    }
}

static void ReadZone(BOOL32 scrubOnUECC, UInt32 ctx)
{
    BOOL32 status;
    UInt32 i;
    UInt32 j;
    UInt32 sb;
    // Handling uECCs, trying older data:
    BOOL32 failed, rereadOrig;
    UInt32 srcVba;

    if (0 == sftl.gc.rs.len) {
        // Empty rs state--writing all trimmed data
        return;
    }

    status = sftl.vfl->ReadSpans(&sftl.gc.rs);

    if (status & (VFL_READ_STATUS_UECC | VFL_READ_STATUS_CLEAN)) {
        
        // iterate along failures find uECCs;
        for (i = 0; i < sftl.gc.curZoneSize; i++) {
            if (sftl.gc.uecc[i>>5] & (1 << (i&31))) {
                // any of them map to the current block?  If so, get out
                sb = s_g_vba_to_sb(sftl.gc.vbas[i]);
                for (j = 0; j < S_SBSTREAM_MAX; j++) {
                    if (sftl.write.stream[j].sb == sb) {
                        sftl.write.stream[j].abandon = TRUE32;
                    }
                }

                failed = TRUE32;
                srcVba = s_btoc_getSrc(sftl.gc.vbas[i]);
                // try to reread from prior location;
                if (0xffffffff != srcVba) {
                    const VFLReadStatusType vfl_status_mask = (VFL_READ_STATUS_UECC | VFL_READ_STATUS_CLEAN |
                                                               VFL_READ_STATUS_VALID_DATA | VFL_READ_STATUS_UNIDENTIFIED);
                    const VFLReadStatusType vfl_status_expected = VFL_READ_STATUS_VALID_DATA;
                    VFLReadStatusType vfl_status;
                    
                    vfl_status = sftl.vfl->ReadMultipleVbas(srcVba, 1, &sftl.gc.zone[s_g_bytes_per_lba * i], (UInt8*)&sftl.gc.meta[i], TRUE32, scrubOnUECC);
                    rereadOrig = FALSE32;
                    
                    if ((vfl_status & vfl_status_mask) != vfl_status_expected) {
                        rereadOrig = TRUE32;
                    }
                    // if fail or LBA doesn't match, reread from original uECC spot, and mark meta as UECC
                    if (rereadOrig || (!rereadOrig && (sftl.gc.lbas[i] != META_GET_LBA(&sftl.gc.meta[i])))) {
                        vfl_status = sftl.vfl->ReadMultipleVbas(sftl.gc.vbas[i], 1, &sftl.gc.zone[s_g_bytes_per_lba * i], (UInt8*)&sftl.gc.meta[i], TRUE32, scrubOnUECC);
                        
                        if((vfl_status & vfl_status_mask) != vfl_status_expected) {
                            failed = FALSE32;
                        }
                    } else {
                        failed = FALSE32;
                    }
                }

                if (failed) {
                    s_SetupMeta_Data_UECC(&sftl.gc.meta[i], sftl.gc.lbas[i]);
                    sftl.gc.ctx[ctx].ueccCounter++;
                }
            }
        }
    }

    ScrubZone();
    for (j = 0; j < S_SBSTREAM_MAX; j++)
    {
        if(sftl.write.stream[j].abandon == TRUE32)
        {
            s_sb_next(j);
            sftl.write.stream[j].abandon = FALSE32;
        }
    }
}

static UInt32 AnalyzeZone(void)
{
    s_write_multi_t *const wm = &sftl.gc.wm;
    UInt32 i, j, cur, lba, staticness = 0;

    cur = j = 0;
    for (i = 0; i < sftl.gc.curZoneSize; i++) {
        if (META_IS_STATIC(&sftl.gc.meta[i]) || S_USERSEQ_IS_OLD(META_GET_USERSEQ(&sftl.gc.meta[i]))) {
            staticness++;
        }
        lba = META_GET_LBA(&sftl.gc.meta[i]);
        if (lba != (wm->lba[cur]+j)) {
            WMR_PANIC("LBA mismatch in GC; lba: %d, meta %d", wm->lba[cur]+j, lba);
        }
        if (S_LBA_STATS == lba) {
            // Override stats LBA data
            s_stats_to_buf((UInt32*)&sftl.gc.zone[s_g_mul_bytes_per_lba(i)]);
        }

        // Next lba
        j++;
        if (j >= wm->subCount[cur]) {
            j = 0;
            cur++;
        }
    }

    return staticness;
}

extern void s_gc_zeroValidCross(void)
{
    GC_Ctx_t *c = &sftl.gc.ctx[S_GC_CTX_FG];

    if (sftl.gc.inProgress) {
        return;
    }

    // Invalid history
    c->invalidHistSum -= c->invalidHist[c->invalidHistIdx];
    c->invalidHist[c->invalidHistIdx] = s_g_vbas_per_sb;
    c->invalidHistSum += s_g_vbas_per_sb;
    c->invalidHistIdx++;
    if (c->invalidHistIdx >= S_GC_VALIDHIST_SIZE) {
        c->invalidHistIdx = 0;
    }
    if (c->invalidHistSize < S_GC_VALIDHIST_SIZE) {
        c->invalidHistSize++;
    }
}

#endif // AND_READONLY
