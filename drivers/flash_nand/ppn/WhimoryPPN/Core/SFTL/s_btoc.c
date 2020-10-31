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
#include "s_btoc.h"
#include "s_sb.h"
#include "s_gc.h"
#include "s_geom.h"

void btoc_calc_userSeq(s_btoc_entry_t *bte, UInt32 len, UInt32 sb)
{
    UInt32 curUser, userMin = 0xffffffff, userMax = 0;
    UInt64 userSum = 0, lenAdjust = 0;
    UInt32 i;

    // Calculate userSeq min/max/avg given a BTOC
    for (i = 0; i < len; i++) {
        curUser = bte[i].userWeaveSeq;
        if (0xffffffff == curUser) {
            lenAdjust++;
            continue;
        }

        if (curUser < userMin) {
            userMin = curUser;
        }
        if (curUser > userMax) {
            userMax = curUser;
        }
        userSum += curUser;
    }

    len = (UInt32)(len - lenAdjust);
    if (0 == len) {
        len = 1;
    }
    if (0xffffffff == userMin) {
        userMin = userMax;
    }

    sftl.sb_userSeq[sb].min = userMin;
    sftl.sb_userSeq[sb].max = userMax;
    sftl.sb_userSeq[sb].avg = (UInt32)(userSum / len);
    sftl.sb_userSeq[sb].count = len;
}

BOOL32 s_btoc_init(void)
{
    UInt32 i, bpl, btb;

    // Calculate maximal BTOC size
    bpl = sizeof(s_btoc_entry_t);
    btb = ((s_g_vbas_per_sb * bpl) + s_g_bytes_per_page - 1);
    sftl.__geom.max_pages_per_btoc = (btb / s_g_bytes_per_page) + 1; // One for leftovers

    sftl.BTOCcache.size = S_BTOCCACHE_SIZE;
    WMR_ASSERT(sftl.BTOCcache.size <= 32);
    sftl.BTOCcache.srcSize = S_BTOCCACHE_SRCSIZE;
    sftl.BTOCcache.curSrc = sftl.BTOCcache.srcSize-1;

    sftl.BTOCcache.curAge = 0;
    sftl.BTOCcache.age = WMR_MALLOC(sizeof(*sftl.BTOCcache.age) * sftl.BTOCcache.size);
    sftl.BTOCcache.sb = WMR_MALLOC(sizeof(*sftl.BTOCcache.sb) * sftl.BTOCcache.size);
    sftl.BTOCcache.BTOC = WMR_MALLOC(sizeof(*sftl.BTOCcache.BTOC) * sftl.BTOCcache.size);
    sftl.BTOCcache.srcVpn = WMR_MALLOC(sizeof(*sftl.BTOCcache.srcVpn) * sftl.BTOCcache.srcSize);
    sftl.BTOCcache.srcSb = WMR_MALLOC(sizeof(*sftl.BTOCcache.srcSb) * sftl.BTOCcache.srcSize);

    // Check for allocation failure
    if ((NULL == sftl.BTOCcache.age) || (NULL == sftl.BTOCcache.sb) || (NULL == sftl.BTOCcache.BTOC)
        || (NULL == sftl.BTOCcache.srcVpn) || (NULL == sftl.BTOCcache.srcSb))
    {
        return FALSE32;
    }

    for (i = 0; i < sftl.BTOCcache.size; i++)
    {
        sftl.BTOCcache.sb[i] = 0xffffffff;
    }

    for (i = 0; i < sftl.BTOCcache.srcSize; i++)
    {
        sftl.BTOCcache.srcSb[i] = 0xffffffff;
        sftl.BTOCcache.srcVpn[i] = WMR_MALLOC(sizeof(**sftl.BTOCcache.srcVpn) * s_g_vbas_per_sb);
        WMR_MEMSET(sftl.BTOCcache.srcVpn[i], 0xff, sizeof(**sftl.BTOCcache.srcVpn) * s_g_vbas_per_sb);
    }

    sftl.BTOCcache.free = (1 << sftl.BTOCcache.size) - 1;

    // Allocate buffers
    WMR_BufZone_Init(&sftl.BTOCcache.BufZone);

    for (i = 0; i < sftl.BTOCcache.size; i++)
    {
        sftl.BTOCcache.BTOC[i] = (s_btoc_entry_t*)WMR_Buf_Alloc_ForDMA(&sftl.BTOCcache.BufZone, s_g_max_pages_per_btoc * s_g_bytes_per_page);
    }
    sftl.BTOCcache.meta = WMR_Buf_Alloc_ForDMA(&sftl.BTOCcache.BufZone, WMR_MAX(S_BTOC_META_SIZE, s_g_max_pages_per_btoc * s_g_vbas_per_page) * s_g_bytes_per_lba_meta);

    if (!WMR_BufZone_FinishedAllocs(&sftl.BTOCcache.BufZone)) {
        return FALSE32;
    }

    for (i = 0; i < sftl.BTOCcache.size; i++)
    {
        WMR_BufZone_Rebase(&sftl.BTOCcache.BufZone, (void**)&sftl.BTOCcache.BTOC[i]);
    }
    WMR_BufZone_Rebase(&sftl.BTOCcache.BufZone, (void**)&sftl.BTOCcache.meta);

    WMR_BufZone_FinishedRebases(&sftl.BTOCcache.BufZone);

    return TRUE32;
}

void s_btoc_close()
{
    UInt32 i;

    WMR_BufZone_Free(&sftl.BTOCcache.BufZone);

    if (NULL != sftl.BTOCcache.age)
    {
        WMR_FREE(sftl.BTOCcache.age, sizeof(*sftl.BTOCcache.age) * sftl.BTOCcache.size);
    }
    if (NULL != sftl.BTOCcache.sb)
    {
        WMR_FREE(sftl.BTOCcache.sb, sizeof(*sftl.BTOCcache.sb) * sftl.BTOCcache.size);
    }
    if (NULL != sftl.BTOCcache.BTOC)
    {
        WMR_FREE(sftl.BTOCcache.BTOC, sizeof(*sftl.BTOCcache.BTOC) * sftl.BTOCcache.size);
    }

    // Free srcVpn memory
    if (NULL != sftl.BTOCcache.srcSb)
    {
        WMR_FREE(sftl.BTOCcache.srcSb, sizeof(*sftl.BTOCcache.srcSb) * sftl.BTOCcache.srcSize);
    }
    for (i = 0; i < sftl.BTOCcache.srcSize; i++)
    {
        WMR_FREE(sftl.BTOCcache.srcVpn[i], sizeof(**sftl.BTOCcache.srcVpn) * s_g_vbas_per_sb);
    }
    WMR_FREE(sftl.BTOCcache.srcVpn, sizeof(*sftl.BTOCcache.srcVpn) * sftl.BTOCcache.srcSize);

    sftl.BTOCcache.age = NULL;
    sftl.BTOCcache.sb = NULL;
    sftl.BTOCcache.BTOC = NULL;
    sftl.BTOCcache.srcSb = NULL;
    sftl.BTOCcache.srcVpn = NULL;

    sftl.BTOCcache.size = 0;

    sftl.BTOCcache.srcSize = 0;
}

#ifndef AND_READONLY

BOOL32 s_btoc_isFull(void)
{
    s_wrstream_t *const wr = &sftl.write.stream[sftl.write.curStream];

    WMR_ASSERT((wr->nextVbaOfs + wr->BTOC_vba) <= wr->maxVbaOfs);
    return ((wr->nextVbaOfs + wr->BTOC_vba) == wr->maxVbaOfs);
}

UInt32 s_btoc_update_size(Int32 remaining)
{
    s_wrstream_t *const wr = &sftl.write.stream[sftl.write.curStream];
    UInt32 maxSize, vba;

    maxSize = wr->maxVbaOfs - wr->nextVbaOfs;

    // Recalculate number of BTOC VBAs
    do {
        vba = (((wr->BTOCidx+1+remaining) * sizeof(s_btoc_entry_t)) + s_g_bytes_per_page - 1);
        vba = s_g_div_bytes_per_page(vba);
        vba = s_g_mul_vbas_per_page(vba);
        remaining--;
        WMR_ASSERT(remaining >= -1);
    } while (vba > maxSize);

    wr->BTOC_vba = vba;

    return remaining+1;
}

void s_btoc_add_data(UInt32 vba, UInt32 lba, UInt32 count, WeaveSeq_t weaveSeq, UInt32 userWeaveSeq)
{
    s_wrstream_t *const wr = &sftl.write.stream[sftl.write.curStream];
    s_btoc_entry_t *last_bte, *bte;
    UInt32 lastCount;

    if (0 == wr->BTOCidx) {
        // First entry?  Set up minWeaveSeq
        wr->minWeaveSeq = weaveSeq;
    }

    // RLE-compress
    if (wr->BTOCidx > 0) {
        last_bte = &wr->BTOC[wr->BTOCidx-1];
        lastCount = last_bte->count;
        if ((lba < S_TOKEN_LBA_BASE)
            && (lba == (last_bte->lba + lastCount))
            && (wr->minWeaveSeq + last_bte->weaveSeqAdd + lastCount) == weaveSeq)
        {
            // Perfectly sequential; not token lba; no gaps in lba, vba (implied), or weaveSeq?
            last_bte->count += count;
            goto commonExit;
        }
    }

    WMR_ASSERT(wr->BTOCidx <= s_g_vbas_per_sb);
    bte = &wr->BTOC[wr->BTOCidx];
    bte->lba = lba;
    bte->count = count;
    bte->weaveSeqAdd = (UInt32)(weaveSeq - wr->minWeaveSeq);
    WMR_ASSERT(bte->weaveSeqAdd < S_WEAVE_SEQ_ADD_TRS);
    bte->userWeaveSeq = userWeaveSeq;
    wr->BTOCidx++;

commonExit:
    WMR_ASSERT(vba == s_g_addr_to_vba(wr->sb, wr->nextVbaOfs));
    wr->nextVbaOfs += count;
}

BOOL32 s_btoc_cross_data(void)
{
    ANDStatus status;
    UInt32 num_vba, vba;
    s_wrstream_t *const wr = &sftl.write.stream[sftl.write.curStream];

    // Terminate btoc
    WMR_ASSERT(0 != wr->BTOCidx);
    wr->BTOC[wr->BTOCidx].count = 0;
    num_vba = wr->maxVbaOfs - wr->nextVbaOfs; // write all of the pages

    // Calculate userSeq values
    btoc_calc_userSeq(wr->BTOC, wr->BTOCidx, wr->sb);

    // Write BTOC
    vba = s_g_addr_to_vba(wr->sb, sftl.vfl->GetVbasPerVb(wr->sb) - num_vba);
    s_SetupMeta_Data_BTOC(sftl.BTOCcache.meta, wr->minWeaveSeq, num_vba, wr->pageFlags);
    status = sftl.vfl->ProgramMultipleVbas(vba, num_vba, (UInt8*)wr->BTOC, (UInt8*)sftl.BTOCcache.meta, TRUE32, FALSE32);
    if (!status) {
        s_sb_clr_num_btoc_vbas(wr->sb);
        WMR_PRINT(QUAL, "sftl: program failure writing BTOC sb:%d vba:0x%x\n", wr->sb, vba);
        // Handled by caller
        return FALSE32;
    }
    s_sb_set_num_btoc_vbas(wr->sb, num_vba);

    // Get next sb--find best, reallocate btoc
    s_sb_next_cur();

    return TRUE32;
}

#endif // AND_READONLY

void s_btoc_copy(s_btoc_entry_t *dest, s_btoc_entry_t *src, UInt32 *len)
{
    *len = (UInt32)(-1);
    do {
        *dest = *src;
        if ((dest->lba > sftl.max_lba) && (dest->lba < S_TOKEN_LBA_BASE)) {
            if (WMR_I_CAN_HAZ_DEBUGGER()) {
                WMR_PANIC("BTOC stomping lba:%d to S_TOK_UECC\n", dest->lba);
            } else {
                WMR_PRINT(ALWAYS, "BTOC stomping lba:%d to S_TOK_UECC\n", dest->lba);
            }
            dest->lba = S_TOK_UECC;
        }
        (*len)++;
        dest++;
        src++;
        WMR_ASSERT((s_g_max_pages_per_btoc * s_g_bytes_per_page / sizeof(s_btoc_entry_t)) > *len);
    } while (0 != (src-1)->count);
}

void s_btoc_read_cb(UInt32 vba, VFLReadStatusType status, UInt8 *__meta)
{
    PageMeta_Data_t *meta = (PageMeta_Data_t*)__meta;

    if (status & VFL_READ_STATUS_UNIDENTIFIED) {
        WMR_PRINT(QUAL_FATAL, "BTOC read got unidentified read status (0x%x) on vba:0x%x; did we get a timeout?  Pretending it was uECC.\n", status, vba);
        status = (status & ~VFL_READ_STATUS_UNIDENTIFIED) | VFL_READ_STATUS_UECC;
    }

    if (status & VFL_READ_STATUS_CLEAN) {
        META_SET_LBA(meta, S_TOK_ERASED);
    } else if (status & VFL_READ_STATUS_UECC) {
        WMR_PRINT(ALWAYS, "S_TOK_UECC vba:0x%x status:0x%x\n", vba, status);
        META_SET_LBA(meta, S_TOK_UECC);
    }
}

BOOL32 s_btoc_read(UInt32 sb, s_btoc_entry_t *btoc, UInt32 *len, BOOL32 stopEarly, BOOL32 scrubOnUECC)
{
    // Return value: FALSE32 == had to reconstruct
    UInt32 vba, num_vba;
    VFLReadStatusType status;
    UInt32 count, thisCount, curLba, curCnt, lba, i;
    UInt32 innerCnt, innerOfs, innerThis, thisErased;
    WeaveSeq_t baseWeave = 0, curWeave = 0;
    BOOL32 baseWeaveSet, isData;
    s_btoc_entry_t *bte;
#ifndef AND_READONLY
    s_btoc_entry_t *ramBTOC;
#endif // AND_READONLY
    VFL_ReadSpans_t *const rs = &sftl.BTOCcache.rs;
    BOOL32 ret = TRUE32;

    *len = 0;

#ifndef AND_READONLY
    // Is it in memory?
    ramBTOC = s_btoc_search(sb);
    if (NULL != ramBTOC) {
        s_btoc_copy(btoc, ramBTOC, len);
        goto readExit;
    }
#endif // AND_READONLY

    num_vba = s_sb_get_num_btoc_vbas(sb);
    if (0 != num_vba) {
        vba = s_g_addr_to_vba(sb, sftl.vfl->GetVbasPerVb(sb) - num_vba);
        sftl.vfl->ReadSpansInit(rs, NULL, 0, FALSE32, scrubOnUECC);
        sftl.vfl->ReadSpansAdd(rs, vba, num_vba, (UInt8*)btoc, (UInt8*)sftl.BTOCcache.meta);
        status = sftl.vfl->ReadSpans(rs);
        if (!(status & (VFL_READ_STATUS_UECC | VFL_READ_STATUS_CLEAN))) {
            if (!META_IS_BTOC_DATA(sftl.BTOCcache.meta)) {
                WMR_PRINT(ALWAYS, "Error: BTOC not valid");
                goto scanIt;
            }

            WMR_ASSERT(0 == btoc->weaveSeqAdd);
            bte = btoc;
            while (0 != bte->count) {
                (*len)++;
                bte++;
            }

            goto readExit;
        }
    }

scanIt:
    // If we had to scan, either because we had none or it was uECC,
    // clear out the number of VBAs so it gets erased at shutdown.
    s_sb_set_num_btoc_vbas_0ok(sb, 0);

    // Scan
    count = sftl.vfl->GetVbasPerVb(sb);
    vba = s_g_addr_to_vba(sb, 0);
    curLba = (UInt32)(-1);
    curCnt = 0;
    baseWeaveSet = FALSE32;
    bte = btoc;

    while (count) {
        thisCount = WMR_MIN(count, S_SCAN_META_SIZE);
        sftl.vfl->ReadSpansInit(rs, s_btoc_read_cb, (VFL_READ_STATUS_UECC|VFL_READ_STATUS_CLEAN), FALSE32, scrubOnUECC);

        // Set up reads
        innerCnt = thisCount;
        innerOfs = 0;
        while (innerCnt) {
            innerThis = WMR_MIN(innerCnt, sftl.gc.zoneSize);
            sftl.vfl->ReadSpansAdd(rs, vba+innerOfs, innerThis, sftl.gc.zone, (UInt8*)&sftl.scan_meta[innerOfs]);
            innerOfs += innerThis;
            innerCnt -= innerThis;
        };
        sftl.vfl->ReadSpans(rs);
        if (!baseWeaveSet) {
            baseWeave = META_GET_WEAVESEQ(&sftl.scan_meta[0]);
            baseWeaveSet = TRUE32;
        }

        thisErased = 0;
        for (i = 0; i < thisCount; i++) {
            lba = META_GET_LBA(&sftl.scan_meta[i]);
            isData = META_IS_DATA(&sftl.scan_meta[i]);
            // Ignore LBAs past drive capacity that aren't token values
            if (((lba >= sftl.max_lba) && (lba < S_TOKEN_LBA_BASE)) || (!isData && (S_TOK_ERASED != lba))) {
                lba = S_TOK_UECC;
            }
            // Count erased content
            if (S_TOK_ERASED == lba) {
                thisErased++;
            }

            // Throw into BTOC if end, beginning, or not sequential, or weaveSeq gap
            if (((count - i) == 1)
                 || !curCnt
                 || ((curLba < S_TOKEN_LBA_BASE) && ((curLba + curCnt) != lba))
                 || ((curLba >= S_TOKEN_LBA_BASE) && (curLba != lba))
                 || (((curWeave + curCnt) != META_GET_WEAVESEQ(&sftl.scan_meta[i])) &&
                        (curLba != S_TOK_ERASED))
                 || (S_TOK_UECC == curLba))
            {
                if (curCnt) {
                    bte->lba = curLba;
                    bte->count = curCnt;
                    if ((S_TOK_ERASED == curLba) || (S_TOK_UECC == curLba)) {
                        bte->weaveSeqAdd = 0;
                    } else {
                        bte->weaveSeqAdd = (UInt32)(curWeave - baseWeave);
                    }
                    bte->userWeaveSeq = META_GET_USERSEQ(&sftl.scan_meta[i]);
                    bte++;
                    (*len)++;
                }
                curLba = lba;
                curCnt = 1;
                curWeave = META_GET_WEAVESEQ(&sftl.scan_meta[i]);
            } else {
                // RLE-compress
                curCnt++;
            }
        }

        if (stopEarly && (S_BTOC_META_SIZE == thisErased)) {
            // Stop early, we've seen enough erased content
            break;
        }

        // Next
        vba += thisCount;
        count -= thisCount;
    }

    // Null-terminate
    bte->count = 0;
    ret = FALSE32;

readExit:
    // Calculate user seq stats
    btoc_calc_userSeq(btoc, *len, sb);

    return ret;
}

s_btoc_entry_t *s_btoc_alloc(UInt32 sb)
{
    UInt32 i, minIdx;
    Int32 minAge;

    // Get a srcVpn
    sftl.BTOCcache.curSrc = (sftl.BTOCcache.curSrc + 1) % sftl.BTOCcache.srcSize;
    sftl.BTOCcache.srcSb[sftl.BTOCcache.curSrc] = sb;
    WMR_MEMSET(sftl.BTOCcache.srcVpn[sftl.BTOCcache.curSrc], 0xff, sizeof(**sftl.BTOCcache.srcVpn) * s_g_vbas_per_sb);

    // Get a BTOC
    sftl.BTOCcache.curAge++;

    minAge = 0x7fffffff;
    minIdx = 0xffffffff;

    for (i = 0; i < sftl.BTOCcache.size; i++)
    {
        if ((sftl.BTOCcache.free & (1 << i))
            && (sftl.BTOCcache.age[i] < minAge))
        {
            minAge = sftl.BTOCcache.age[i];
            minIdx = i;
        }
    }

    if (0xffffffff != minIdx) {
        sftl.BTOCcache.free &= ~(1 << minIdx);
        sftl.BTOCcache.sb[minIdx] = sb;
        sftl.BTOCcache.age[minIdx] = sftl.BTOCcache.curAge;
        WMR_MEMSET(sftl.BTOCcache.BTOC[minIdx], 0, s_g_max_pages_per_btoc * s_g_bytes_per_page);
        return sftl.BTOCcache.BTOC[minIdx];
    }

    WMR_PANIC("Couldn't allocate a BTOC");
    return NULL;
}

#ifndef AND_READONLY
UInt32 s_btoc_getSrc(UInt32 dest_vba)
{
    UInt32 i, sb, vbaOfs;

    sb = s_g_vba_to_sb(dest_vba);
    vbaOfs = s_g_vba_to_vbaOfs(dest_vba);

    for (i = 0; i < sftl.BTOCcache.srcSize; i++)
    {
        if (sb == sftl.BTOCcache.srcSb[i]) {
            return sftl.BTOCcache.srcVpn[i][vbaOfs];
        }
    }

    return 0xffffffff;
}
#endif // AND_READONLY

#ifndef AND_READONLY
void s_btoc_setSrc(UInt32 dest_vba, UInt32 src_vba)
{
    UInt32 i, sb, vbaOfs;

    sb = s_g_vba_to_sb(dest_vba);
    vbaOfs = s_g_vba_to_vbaOfs(dest_vba);

    for (i = 0; i < sftl.BTOCcache.srcSize; i++)
    {
        if (sb == sftl.BTOCcache.srcSb[i]) {
            sftl.BTOCcache.srcVpn[i][vbaOfs] = src_vba;
        }
    }
}
#endif // AND_READONLY

void s_btoc_dealloc(s_btoc_entry_t *BTOC)
{
    UInt32 i;

    for (i = 0; i < sftl.BTOCcache.size; i++)
    {
        if (BTOC == sftl.BTOCcache.BTOC[i])
        {
            sftl.BTOCcache.free |= (1 << i);
            return;
        }
    }

    WMR_PANIC("Couldn't deallocate BTOC: %p", BTOC);
}

#ifndef AND_READONLY
s_btoc_entry_t *s_btoc_search(UInt32 sb)
{
    UInt32 i;
    Int32 maxAge = -1;
    s_btoc_entry_t *best = NULL;

    for (i = 0; i < sftl.BTOCcache.size; i++)
    {
        if ((sftl.BTOCcache.sb[i] == sb) && (sftl.BTOCcache.age[i] > maxAge))
        {
            maxAge = sftl.BTOCcache.age[i];
            best = sftl.BTOCcache.BTOC[i];
        }
    }

    return best;
}
#endif // AND_READONLY

