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

#include "s_cxt.h"
#include "s_geom.h"
#include "s_sb.h"
#include "s_dbg.h"
#include "s_stats.h"
#include "s_write.h"
#include "s_trim.h"

#ifndef AND_READONLY

static void cxtContigAdd(void)
{
    s_cxt_t *const cxt = &sftl.cxt;
    L2V_SearchCtx_t *const c = &cxt->save.c;

    cxt->save.curContig->span = c->span;
    cxt->save.curContig->vba = c->vba;
    cxt->save.curContig++;
    cxt->save.curIdx++;
}

static void cxtNextPage(UInt32 baseLba, BOOL32 done)
{
    s_cxt_t *const cxt = &sftl.cxt;
    L2V_SearchCtx_t *const c = &cxt->save.c;

    if ((cxt->save.curIdx < cxt->save.idxPerVba) && !done)
        return;

    while (cxt->save.curIdx < cxt->save.idxPerVba) {
        c->vba = c->span = 0xffffffff;
        cxtContigAdd();
    }

    cxt->save.curVba++;
    if(cxt->save.curVba >= sftl.gc.zoneSize)
    {
        return;
    }
    cxt->save.curContig = (s_cxt_contig_t*)&sftl.gc.zone[s_g_mul_bytes_per_lba(cxt->save.curVba)];
    cxt->save.curIdx = 0;

    // Base lba:
    cxt->save.curContig->span = 0xfffffff0;
    cxt->save.curContig->vba = baseLba;
    WMR_ASSERT((0xffffffff != baseLba) || done);
    cxt->save.curContig++;
    cxt->save.curIdx++;
}

static void cxtNextSB(void)
{
    s_cxt_t *const cxt = &sftl.cxt;
    UInt32 thisVbas;

    cxt->save.vba = s_g_addr_to_vba(cxt->save.sb[cxt->save.num_sb_used], 0);
    thisVbas = sftl.vfl->GetVbasPerVb(cxt->save.sb[cxt->save.num_sb_used]);
    WMR_ASSERT(0 != thisVbas);
    cxt->save.maxVba = cxt->save.vba + thisVbas;
    cxt->save.num_sb_used++;
}

static void cxtBatchSetup(UInt32 baseLba)
{
    s_cxt_t *const cxt = &sftl.cxt;

    cxt->save.idxPerVba = s_g_bytes_per_lba / sizeof(s_cxt_contig_t);
    cxt->save.curVba = (UInt32)(-1);
    cxt->save.curIdx = (UInt32)(-1);
    cxtNextPage(baseLba, FALSE32);
}

static BOOL32 cxtNextBatch(UInt32 baseLba, BOOL32 done)
{
    s_cxt_t *const cxt = &sftl.cxt;
    BOOL32 status;
    s_cxt_contig_t *contig;

    if (((cxt->save.curVba < sftl.gc.zoneSize) && !done) || (0 == cxt->save.curVba))
        return TRUE32;

    // Fill out zone with pad
    while (cxt->save.curVba < sftl.gc.zoneSize) {
        contig = (s_cxt_contig_t*)&sftl.gc.zone[s_g_mul_bytes_per_lba(cxt->save.curVba)];
        contig->span = 0xffffffff;
        contig->vba = 0xffffffff;
        cxt->save.curVba++;
    }

    if ((cxt->save.vba+cxt->save.curVba) >= cxt->save.maxVba) {
        cxtNextSB();
    }

    s_SetupMeta_Cxt(sftl.gc.meta, cxt->save.curVba, S_CXTTAG_TREE, 0, 0, 0);
    status = sftl.vfl->ProgramMultipleVbas(cxt->save.vba, cxt->save.curVba, (UInt8*)sftl.gc.zone, (UInt8*)sftl.gc.meta, TRUE32, FALSE32);
    if (!status) {
        WMR_PRINT(QUAL, "sftl: program failure writing cxt batch vba:0x%x\n", cxt->save.vba);
    }
    cxt->save.vba += cxt->save.curVba;

    cxtBatchSetup(baseLba);

    return status;
}

static BOOL32 cxtSaveBase(void)
{
    s_cxt_t *const cxt = &sftl.cxt;
    UInt32 *base = (UInt32*)sftl.gc.zone;
    PageMeta_t *baseMeta = sftl.gc.meta;
    UInt32 *buf;
    UInt32 i, bufMaxIdx;
    BOOL32 status;

    s_SetupMeta_Cxt(baseMeta, s_g_vbas_per_page, S_CXTTAG_BASE, 0, 0, 0);

    // Fill out base data
    buf = base;
    WMR_MEMSET(buf, 0, s_g_bytes_per_page);
    buf[0] = cxt->save.num_sb;
    WMR_ASSERT(0 != buf[0]);
    for (i = 0; i < cxt->save.num_sb; i++) {
        buf[i+1] = cxt->save.sb[i];
    }

    // Stuff FTL superblocks in
    bufMaxIdx = (s_g_bytes_per_page / sizeof(*buf)) - 1;
    buf[bufMaxIdx] = s_g_max_sb;

    status = sftl.vfl->ProgramMultipleVbas(cxt->save.vba, s_g_vbas_per_page, (UInt8*)base, (UInt8*)baseMeta, TRUE32, FALSE32);
    if (!status) {
        WMR_PRINT(QUAL, "sftl: program failure writing cxt base vba:0x%x\n", cxt->save.vba);
        return FALSE32;
    }

    cxt->save.vba += s_g_vbas_per_page;
    WMR_ASSERT(cxt->save.vba < cxt->save.maxVba);

    return TRUE32;
}

static BOOL32 cxtSaveStats(void)
{
    s_cxt_t *const cxt = &sftl.cxt;
    UInt32 *buf = (UInt32*)sftl.gc.zone;
    BOOL32 status;

    s_stats_to_buf(buf);
    s_SetupMeta_Cxt(sftl.gc.meta, s_g_vbas_per_page, S_CXTTAG_STATS, 0, 0, 0);

    status = sftl.vfl->ProgramMultipleVbas(cxt->save.vba, s_g_vbas_per_page, (UInt8*)sftl.gc.zone, (UInt8*)sftl.gc.meta, TRUE32, FALSE32);
    if (!status) {
        WMR_PRINT(QUAL, "sftl: program failure writing cxt stats vba:0x%x\n", cxt->save.vba);
        return FALSE32;
    }

    cxt->save.vba += s_g_vbas_per_page;
    WMR_ASSERT(cxt->save.vba < cxt->save.maxVba);

    return TRUE32;
}

static BOOL32 cxtSaveSB(void)
{
    s_cxt_t *const cxt = &sftl.cxt;
    UInt32 count, thisCount, curCount, thisCountLbas, idx = 0;
    BOOL32 status;
    s_sb_t *src;
    s_sb_fmt_t *dest;

    count = sizeof(s_sb_fmt_t) * s_g_max_sb;
    src = &sftl.sb[0];

    while (count) {
        thisCount = WMR_MIN(s_g_mul_bytes_per_lba(sftl.gc.zoneSize), count);
        thisCountLbas = ((thisCount + s_g_bytes_per_page - 1) / s_g_bytes_per_page) * s_g_vbas_per_page;

        dest = (s_sb_fmt_t*)&sftl.gc.zone[0];
        curCount = thisCount;
        while (curCount >= sizeof(s_sb_fmt_t)) {
            dest->type = src->type;
            dest->num_btoc_vbas_AND_staticFlag = src->num_btoc_vbas_AND_staticFlag;
            dest->validLbas = src->validLbas;
            dest->__was_reads = 0;
            dest->erases = src->erases;

            src++, dest++;
            curCount -= sizeof(s_sb_fmt_t);
        }

        s_SetupMeta_Cxt(sftl.gc.meta, thisCountLbas, S_CXTTAG_SB, idx, s_g_bytes_per_lba/sizeof(s_sb_fmt_t), thisCount / sizeof(s_sb_fmt_t));
        status = sftl.vfl->ProgramMultipleVbas(cxt->save.vba, thisCountLbas, (UInt8*)sftl.gc.zone, (UInt8*)sftl.gc.meta, TRUE32, FALSE32);
        if (!status) {
            WMR_PRINT(QUAL, "sftl: program failure writing cxt SB array vba:0x%x\n", cxt->save.vba);
            return FALSE32;
        }

        // Next
        idx += thisCountLbas * s_g_bytes_per_lba / sizeof(s_sb_fmt_t);
        count -= thisCount;
        cxt->save.vba += thisCountLbas;
        WMR_ASSERT(cxt->save.vba < cxt->save.maxVba);
    }

    return TRUE32;
}

static BOOL32 cxtSaveUserSeq(void)
{
    s_cxt_t *const cxt = &sftl.cxt;
    UInt32 count, thisCount, curCount, thisCountLbas, idx = 0;
    BOOL32 status;
    s_sb_userSeq_t *src;
    s_sb_userSeq_t *dest;

    count = sizeof(*src) * s_g_max_sb;
    src = &sftl.sb_userSeq[0];
    WMR_ASSERT((s_g_bytes_per_lba % sizeof(*src)) == 0);

    while (count) {
        thisCount = WMR_MIN(s_g_mul_bytes_per_lba(sftl.gc.zoneSize), count);
        thisCountLbas = ((thisCount + s_g_bytes_per_page - 1) / s_g_bytes_per_page) * s_g_vbas_per_page;

        dest = (s_sb_userSeq_t*)&sftl.gc.zone[0];
        curCount = thisCount;
        while (curCount >= sizeof(*src)) {
            *dest = *src;

            src++, dest++;
            curCount -= sizeof(*src);
        }

        s_SetupMeta_Cxt(sftl.gc.meta, thisCountLbas, S_CXTTAG_USERSEQ, idx, s_g_bytes_per_lba/sizeof(*dest), thisCount / sizeof(*dest));
        status = sftl.vfl->ProgramMultipleVbas(cxt->save.vba, thisCountLbas, (UInt8*)sftl.gc.zone, (UInt8*)sftl.gc.meta, TRUE32, FALSE32);
        if (!status) {
            WMR_PRINT(QUAL, "sftl: program failure writing cxt userSeq vba:0x%x\n", cxt->save.vba);
            return FALSE32;
        }

        // Next
        idx += thisCountLbas * (s_g_bytes_per_lba / sizeof(*src));
        count -= thisCount;
        cxt->save.vba += thisCountLbas;
        WMR_ASSERT(cxt->save.vba < cxt->save.maxVba);
    }   

    return TRUE32;
}

static BOOL32 cxtSaveReads(void)
{
    s_cxt_t *const cxt = &sftl.cxt;
    UInt32 count, thisCount, curCount, thisCountLbas, idx = 0;
    BOOL32 status;
    s_sb_t *src;
    UInt32 *dest;

    count = sizeof(UInt32) * s_g_max_sb;
    src = &sftl.sb[0];

    while (count) {
        thisCount = WMR_MIN(s_g_mul_bytes_per_lba(sftl.gc.zoneSize), count);
        thisCountLbas = ((thisCount + s_g_bytes_per_page - 1) / s_g_bytes_per_page) * s_g_vbas_per_page;

        dest = (UInt32*)&sftl.gc.zone[0];
        curCount = thisCount;
        while (curCount >= sizeof(UInt32)) {
            *dest = src->reads;

            src++, dest++;
            curCount -= sizeof(UInt32);
        }

        s_SetupMeta_Cxt(sftl.gc.meta, thisCountLbas, S_CXTTAG_READS, idx, s_g_bytes_per_lba/sizeof(UInt32), thisCount / sizeof(UInt32));
        status = sftl.vfl->ProgramMultipleVbas(cxt->save.vba, thisCountLbas, (UInt8*)sftl.gc.zone, (UInt8*)sftl.gc.meta, TRUE32, FALSE32);
        if (!status) {
            WMR_PRINT(QUAL, "sftl: program failure writing cxt reads vba:0x%x\n", cxt->save.vba);
            return FALSE32;
        }

        // Next
        idx += thisCountLbas * s_g_bytes_per_lba / sizeof(UInt32);
        count -= thisCount;
        cxt->save.vba += thisCountLbas;
        WMR_ASSERT(cxt->save.vba < cxt->save.maxVba);
    }

    return TRUE32;
}

static BOOL32 cxtSaveTree(void)
{
    s_cxt_t *const cxt = &sftl.cxt;
    L2V_SearchCtx_t *const c = &cxt->save.c;
    UInt32 lba;

    L2V_Search_Init(c);

    // Initialize structures
    lba = 0;

    cxtBatchSetup(lba);

    while (lba < sftl.max_lba) {
        // Obtain next span
        c->lba = lba;
        L2V_Search(c);
        WMR_ASSERT(0 != c->span);

        // Save
        cxtContigAdd();

        // Next
        lba += c->span;

        // Page full?
        cxtNextPage(lba, FALSE32);
        // Batch full?
        if (!cxtNextBatch(lba, FALSE32))
            return FALSE32;
    }

    cxtNextPage(0xffffffff, TRUE32);
    return cxtNextBatch(lba, TRUE32);
}

static void cxtRememberOld(void)
{
    s_cxt_t *const cxt = &sftl.cxt;
    UInt32 i;

    // Remember old cxt sb's
    for (i = 0; i < cxt->save.num_sb; i++) {
        cxt->save.old_sb[i] = cxt->save.sb[i];
    }
    cxt->save.old_num_sb = cxt->save.num_sb;
}

static void cxtMakeSpace(void)
{
    s_cxt_t *const cxt = &sftl.cxt;
    UInt32 sumVbas, thisVbas;

    cxt->save.num_sb = 0;
    sumVbas = 0;
    while (sumVbas < sftl.cxt.cxt_vbas) {
        WMR_ASSERT(cxt->save.num_sb < S_CXT_MAX_SB);
        cxt->save.sb[cxt->save.num_sb] = s_sb_cxt_alloc();
        thisVbas = sftl.vfl->GetVbasPerVb(cxt->save.sb[cxt->save.num_sb]);
        WMR_ASSERT(0 != thisVbas);
        sumVbas += thisVbas;
        cxt->save.num_sb++;
    }
    s_dbg_check_sb_dist();

    cxt->save.num_sb_used = 0;
    cxtNextSB();
}

static void cxtEraseCur(void)
{
    s_cxt_t *const cxt = &sftl.cxt;
    UInt32 i;

    // Erase the ones we've used
    for (i = 0; i < cxt->save.num_sb_used; i++) {
       sftl.vfl->Erase(cxt->save.sb[i], TRUE32);
    }

    // Free them
    for (i = 0; i < cxt->save.num_sb; i++) {
        s_sb_cxt_free_clean(cxt->save.sb[i]);
    }

    // Reallocate
    cxtMakeSpace();
}

static void cxtEraseOld(void)
{
    s_cxt_t *const cxt = &sftl.cxt;
    UInt32 i;

    for (i = 0; i < cxt->save.old_num_sb; i++) {
        s_sb_cxt_free_erase(cxt->save.old_sb[i]);
    }

    s_dbg_check_sb_dist();
}

static void cxtFreeUnused(void)
{
    s_cxt_t *const cxt = &sftl.cxt;
    UInt32 i;

    for (i = cxt->save.num_sb_used; i < cxt->save.num_sb; i++) {
        s_sb_cxt_free_clean(cxt->save.sb[i]);
    }

    cxt->save.num_sb = cxt->save.num_sb_used;

    s_dbg_check_sb_dist();
}

void s_cxt_save(void)
{
    WMR_TRACE_IST_0(CxtSave, START);

    // Must drain pending buffers, since they already have weaveSeq's attached
    s_drain_stream_all(TRUE32);

    // Apply pending trims, without token
    s_trim_apply(FALSE32);

    // Allocate space
    cxtRememberOld();
    cxtMakeSpace();

retry:

    // Base page; contains pointers to other sb's, etc
    if (!cxtSaveBase()) {
        cxtEraseCur();
        goto retry;
    }

    // Save AND stats
    if (!cxtSaveStats()) {
        cxtEraseCur();
        goto retry;
    }

    // Save superblock array
    if (!cxtSaveSB()) {
        cxtEraseCur();
        goto retry;
    }

    // Save userSeq
    if (!cxtSaveUserSeq()) {
        cxtEraseCur();
        goto retry;
    }

    // Save reads
    if (!cxtSaveReads()) {
        cxtEraseCur();
        goto retry;
    }

    // Save tree
    s_dbg_check_validSums();
    if (!cxtSaveTree()) {
        cxtEraseCur();
        goto retry;
    }

    // Erase old cxt
    cxtEraseOld();

    // Free unused superblocks, since we over-allocated
    cxtFreeUnused();

    // Clear vulnerability flags
    s_trim_clearVulnerables();

    WMR_TRACE_IST_0(CxtSave, END);
}

#endif // AND_READONLY

