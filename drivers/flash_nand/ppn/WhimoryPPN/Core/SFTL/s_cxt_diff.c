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

#include "s_cxt.h"
#include "s_btoc.h"
#include "s_geom.h"
#include "s_sb.h"
#include "s_dbg.h"

// functions:
static void data_weaveseq_add(UInt32 sb, PageMeta_BTOC_t *meta);
static void wo_append(UInt32 sb, WeaveSeq_t minWeaveSeq, WeaveSeq_t maxWeaveSeq);
static void wo_sort(void);
static void clean_add(UInt32 sb, BOOL32 force);

void s_diff_sbFilter_add(UInt32 sb)
{
    s_cxt_diff_t *const diff = &sftl.cxt.diff;
    UInt32 idx, bit;

    WMR_ASSERT(NULL != diff->sbFilter);
    WMR_ASSERT(sb < s_g_max_sb);

    idx = sb / 32;
    bit = sb & 31;

    diff->sbFilter[idx] |= (1 << bit);
}

BOOL32 s_diff_sbFilter_has(UInt32 sb)
{
    s_cxt_diff_t *const diff = &sftl.cxt.diff;
    UInt32 idx, bit;

    WMR_ASSERT(NULL != diff->sbFilter);
    WMR_ASSERT(sb < s_g_max_sb);

    idx = sb / 32;
    bit = sb & 31;

    return (diff->sbFilter[idx] >> bit) & 1;
}

void s_diff_filterPost(WeaveSeq_t weave)
{
    s_cxt_diff_t *const diff = &sftl.cxt.diff;
    UInt32 idx;

    for (idx = 0; idx < diff->size; idx++) {
        if (diff->item[idx].minWeaveSeq > weave) {
            s_diff_sbFilter_add(diff->item[idx].sb);
        }
    }
}

static void wo_seed(void)
{
    s_cxt_t *const cxt = &sftl.cxt;
    s_cxt_diff_t *const diff = &sftl.cxt.diff;
    UInt32 seed_max, num;
    BOOL32 no_gc;

    diff->idx = 0;
    seed_max = WMR_MIN(S_SBSTREAM_PHY_MAX, WMR_MIN(sftl.BTOCcache.size, diff->size - diff->idx));

    for (num = 0; (num < seed_max) && (diff->idx < diff->size); diff->idx++) {
        // Skip sbs wholly older than cxt
        if ((diff->item[diff->idx].maxWeaveSeq < cxt->load.baseWeaveSeq) && (diff->idx < diff->size))  {
            continue;
        }

        diff->itemIdx[num] = diff->idx;
        diff->btoc[num] = s_btoc_alloc(0xffffffff);
        no_gc = s_btoc_read(diff->item[diff->idx].sb, diff->btoc[num], &diff->btoc_len[num], TRUE32, FALSE32);
        if (!no_gc) {
            s_sb_set_type(diff->item[diff->idx].sb, S_SB_DATA_PENDING_GC);
        }
        diff->vba[num] = s_g_addr_to_vba(diff->item[diff->idx].sb, 0);
        diff->bte[num] = diff->btoc[num];
        diff->weave[num] = diff->item[diff->idx].minWeaveSeq;
        num++;
    }
}

static void wo_unseed(void)
{
    s_cxt_diff_t *const diff = &sftl.cxt.diff;
    UInt32 i;

    for (i = 0; i < S_SBSTREAM_PHY_MAX; i++) {
        if (NULL != diff->btoc[i]) {
            s_btoc_dealloc(diff->btoc[i]);
        }
    }
}

static UInt32 find_min(void)
{
    s_cxt_diff_t *const diff = &sftl.cxt.diff;
    UInt32 i, which;
    WeaveSeq_t weave, minWeave;
    
    minWeave = (WeaveSeq_t)(~0L);
    which = ~0;

    for (i = 0; i < S_SBSTREAM_PHY_MAX; i++) {
        if (0 == diff->btoc_len[i])
            continue;

        weave = diff->weave[i] + diff->bte[i]->weaveSeqAdd;
        if (weave < minWeave) {
            minWeave = weave;
            which = i;
        }
    }

    return which;
}

static void load_trims(UInt32 vba)
{
    s_cxt_t *const cxt = &sftl.cxt;
    UInt8 *buf = sftl.tmpBuf;
    PageMeta_Data_t *meta = (PageMeta_Data_t*)sftl.boot.meta;
    UInt32 *trims = (UInt32*)sftl.tmpBuf;
    UInt32 num, lba, count;

    sftl.vfl->ReadSpansInit(&cxt->load.rs, NULL, 0, TRUE32, FALSE32);

    sftl.vfl->ReadSpansAdd(&cxt->load.rs, vba, 1, buf, (UInt8*)meta);
    if ((sftl.vfl->ReadSpans(&cxt->load.rs)) & (VFL_READ_STATUS_UECC | VFL_READ_STATUS_CLEAN)) {
        return;
    }

    num = *trims++;
    while (num--) {
        lba = *trims++;
        count = *trims++;
        L2V_Update(lba, count, L2V_VBA_DEALLOC);
    }
}

void s_cxt_scan_diff(void)
{
    s_cxt_diff_t *const diff = &sftl.cxt.diff;
    s_cxt_t *const cxt = &sftl.cxt;
    BOOL32 no_gc, skipping;
    UInt32 i, which, lba, vba, count;
#ifndef AND_READONLY
    UInt32 lastCount = 0;
#endif // AND_READONLY
    WeaveSeq_t weaveSeq, lastWeave = 0;

    // Seed it
    wo_seed();
    diff->updates = 0;
    count = 0;

    skipping = FALSE32;
    while (1) {
        // Find next piece to weave
        which = find_min();
        if ((UInt32)~0 == which) {
            for (i = 0; i < S_SBSTREAM_PHY_MAX; i++) {
                WMR_ASSERT(0 == diff->btoc_len[i]);
            }
            break;
        }

        // Iterate along mappings
        lba = diff->bte[which]->lba;
        count = diff->bte[which]->count;
        weaveSeq = diff->weave[which] + diff->bte[which]->weaveSeqAdd;
        vba = diff->vba[which];
#ifndef AND_READONLY
        if ((lba < S_TOKEN_LBA_BASE) && (weaveSeq < lastWeave)) {
            if (WMR_I_CAN_HAZ_DEBUGGER()) {
                // Panic on debuggable units
                WMR_PANIC("weaveSeq non-monotonic! lba:%d count:%d vba:0x%x weaveSeq:0x%llx lastWeave:0x%llx lastCount:%d which:%d", lba, count, vba, weaveSeq, lastWeave, lastCount, which);
            } else {
                if (0 == s_sb_get_num_btoc_vbas(s_g_vba_to_sb(vba))) {
                    // Misordering in sb's without BTOCs are more likely to be the result of NAND errors
                    WMR_PRINT(ERROR, "weaveSeq non-monotonic, treating as media error! lba:%d count:%d vba:0x%x weaveSeq:0x%llx lastWeave:0x%llx lastCount:%d which:%d\n", lba, count, vba, weaveSeq, lastWeave, lastCount, which);
                    lba = S_TOK_UECC;
                } else {
                    // Whereas in sb's with BTOCs, a software bug is more likely
                    WMR_PRINT(ERROR, "weaveSeq non-monotonic, ignoring the fact... lba:%d count:%d vba:0x%x weaveSeq:0x%llx lastWeave:0x%llx lastCount:%d which:%d\n", lba, count, vba, weaveSeq, lastWeave, lastCount, which);
                }
            }
        }
        if (weaveSeq >= diff->item[diff->itemIdx[which]].maxWeaveSeq) {
            WMR_PANIC("weaveSeq exceeds block maximum! lba:%d count:%d vba:0x%x weaveSeq:0x%llx maxWeaveSeq:0x%llx which:%d\n", lba, count, vba, weaveSeq, diff->item[diff->itemIdx[which]].maxWeaveSeq, which);
        }
#endif // AND_READONLY

        if (weaveSeq > cxt->load.baseWeaveSeq) {
            if (lba < S_TOKEN_LBA_BASE) {
                if (!skipping) {
                    L2V_Update(lba, count, vba);
                    diff->updates++;
                    if (S_LBA_STATS == lba) {
                        cxt->load.sawStats_lba = 1;
                    }
                }
            } else if (S_TOK_PAD == lba) {
                // Ignore
            } else if (S_TOK_DELETE == lba) {
                if (!skipping) {
                    load_trims(vba);
                }
            } else if (S_TOK_ERASED == lba) {
                // Ignore erased spans [comes from BTOC read-scan]
                s_sb_set_type(s_g_vba_to_sb(vba), S_SB_DATA_PENDING_GC);
            } else if (S_TOK_UECC == lba) {
                s_sb_set_type(s_g_vba_to_sb(vba), S_SB_DATA_PENDING_GC);
                if (!skipping && (diff->idx >= diff->size)) {
                    // Skip anything newer
                    WMR_PRINT(ALWAYS, "uECC in stream at vba:0x%x; exiting early to be coherent!\n", vba);
                    skipping = TRUE32;
                }
            } else {
                WMR_PRINT(ALWAYS, "unknown token: 0x%x\n", lba);
            }
        } else {
            // Check ordering
            if ((S_TOK_ERASED != lba) && (S_TOK_UECC != lba))  {
                WMR_ASSERT(0 == diff->updates);
            }
        }

        // Next
        diff->vba[which] += count;
        diff->bte[which]++;
        WMR_ASSERT(0 != diff->btoc_len[which]);
        diff->btoc_len[which]--;
        if (lba < S_TOKEN_LBA_BASE) {
            lastWeave = WMR_MAX(lastWeave, weaveSeq + count);
#ifndef AND_READONLY
            lastCount = count;
#endif // AND_READONLY
        }

        if ((!diff->bte[which]->count) && (diff->idx < diff->size)) {
            WMR_ASSERT(0 == diff->btoc_len[which]);

            // Load next
            // Skip sbs wholly older than cxt
            while ((diff->item[diff->idx].maxWeaveSeq < cxt->load.baseWeaveSeq) && (diff->idx < diff->size)) {
                diff->idx++;
            }
            if (diff->idx != diff->size) {
                diff->itemIdx[which] = diff->idx;
                no_gc = s_btoc_read(diff->item[diff->idx].sb, diff->btoc[which], &diff->btoc_len[which], TRUE32, FALSE32);
                if (!no_gc) {
                    s_sb_set_type(diff->item[diff->idx].sb, S_SB_DATA_PENDING_GC);
                }
                diff->vba[which] = s_g_addr_to_vba(diff->item[diff->idx].sb, 0);
                diff->bte[which] = diff->btoc[which];
                diff->weave[which] = diff->item[diff->idx].minWeaveSeq;

                diff->idx++;
            }
        }
    }

    // Set up weave sequence for incoming writes
    sftl.write.weaveSeq = WMR_MAX(sftl.write.weaveSeq, lastWeave);

    // Free BTOCs
    wo_unseed();

    s_dbg_check_validSums();
}

static void scan_all_sb_cb(UInt32 vba, VFLReadStatusType status, UInt8 *__meta)
{
    if (status & VFL_READ_STATUS_UNIDENTIFIED) {
         WMR_PRINT(QUAL_FATAL, "scan_all got unidentified read status (0x%x) on vba:0x%x; did we get a timeout?  Pretending it was uECC.\n", status, vba);
        status = (status & ~VFL_READ_STATUS_UNIDENTIFIED) | VFL_READ_STATUS_UECC;
    }

    if (status & VFL_READ_STATUS_UECC) {
        META_SET_LBA_UECC(__meta);
    } else if (status & VFL_READ_STATUS_CLEAN) {
        META_SET_LBA_CLEAN(__meta);
    }
}

void s_scan_all_sb(void)
{
    s_cxt_t *const cxt = &sftl.cxt;
    UInt32 vba, sb, size, i, lookSb;
    PageMeta_t *meta, *metaBTOC;
    UInt8 *buf;
    UInt32 vbas_in_sb;

    // Find pagetypes, weaveSeqMin, weaveSeqAdd
    for (sb = 0; sb < s_g_max_sb; ) {
        size = 0;

        // Clear read-spattered
        sftl.vfl->ReadSpansInit(&cxt->load.rs, scan_all_sb_cb, VFL_READ_STATUS_ALL, TRUE32, FALSE32);
        buf = sftl.gc.zone;
        meta = sftl.scan_meta;
        
        // Quick first pass; scan the first and last vbas
        lookSb = sb;
        while ((size < S_SCAN_META_SIZE) && (sb < s_g_max_sb)) {
            vbas_in_sb = sftl.vfl->GetVbasPerVb(sb);
            if (vbas_in_sb) {
                // Schedule read of first page
                vba = s_g_addr_to_vba(sb, 0);
                sftl.vfl->ReadSpansAdd(&cxt->load.rs, vba, 1, buf, (UInt8*)meta);
                meta++;

                // And last, which should be a subset of the BTOC
                vba = s_g_addr_to_vba(sb, (vbas_in_sb - 1));
                sftl.vfl->ReadSpansAdd(&cxt->load.rs, vba, 1, buf, (UInt8*)meta);
                meta++;

                // Since we took space
                size += 2;
            } else {
                s_sb_set_type(sb, S_SB_DEAD);
                sftl.seaState.dead_sb++;
            }

            // Next
            sb++;
        }

        WMR_ASSERT(size <= S_SCAN_META_SIZE);
        // Perform read
        sftl.vfl->ReadSpans(&cxt->load.rs);

        // Examine metadata
        meta = sftl.scan_meta;
        for (i = 0; i < size; i += 2) {
            // Skip over zero-sized superblocks, again
            while (0 == (vbas_in_sb = sftl.vfl->GetVbasPerVb(lookSb))) {
                lookSb++;
            }

            metaBTOC = meta+1;
            if (!META_IS_LBA_UECC(metaBTOC)
                && !META_IS_LBA_CLEAN(metaBTOC)
                && (PAGETYPE_FTL_BTOC_DATA == META_GET_PAGETYPE(metaBTOC)))
            {
                data_weaveseq_add(lookSb, (PageMeta_BTOC_t*)metaBTOC);
            } else {
                if (META_IS_LBA_UECC(meta)) {
                    clean_add(lookSb, TRUE32);
                } else if (META_IS_LBA_CLEAN(meta)) {
                    clean_add(lookSb, FALSE32);
                } else {
                    if ((PAGETYPE_FTL_DATA == meta->c.PageType)
                        || (PAGETYPE_FTL_INTDATA == meta->c.PageType)) {
                        META_BTOC_SET_NUM_VBAS(meta, 0); // Override btoc vba counter
                        data_weaveseq_add(lookSb, (PageMeta_BTOC_t*)meta);
                    } else if (PAGETYPE_FTL_CXT == meta->c.PageType) {
                        s_sb_set_type(lookSb, S_SB_CXT);
                        sftl.seaState.cxt_sb++;
                        if (s_cxt_is_firstSb(meta, lookSb, 0)) {
                            s_cxt_addSb(lookSb, META_GET_WEAVESEQ(meta));
                        }
                    } else {
                        WMR_PRINT(ALWAYS, "Unknown PageType:0x%02x in sb:%d (r/w: erase, r/o: ignore)\n", META_GET_PAGETYPE(meta), lookSb);
                        clean_add(lookSb, TRUE32);
                    }
                }
            }

            // Next
            meta += 2;
            lookSb++;
        }
    }

    wo_sort();
}

BOOL32 s_wo_init(void)
{
    s_cxt_diff_t *const diff = &sftl.cxt.diff;
    WMR_MEMSET(diff, 0, sizeof(*diff));

    diff->item = WMR_MALLOC(sizeof(s_wo_item_t) * s_g_max_sb);
    WMR_ASSERT(0 != s_g_max_sb);
    diff->sbFilter = WMR_MALLOC(sizeof(*diff->sbFilter) * ((s_g_max_sb + 31) / 32));
    diff->size = 0;

    return (NULL != diff->item) && (NULL != diff->sbFilter);
}

void s_wo_close(void)
{
    s_cxt_diff_t *const diff = &sftl.cxt.diff;

    if (NULL != diff->item) {
        WMR_FREE(diff->item, sizeof(s_wo_item_t) * s_g_max_sb);
    }
    diff->item = NULL;

    if (NULL != diff->sbFilter) {
        WMR_FREE(diff->sbFilter, sizeof(*diff->sbFilter) * ((s_g_max_sb + 31) / 32));
    }
    diff->sbFilter = NULL;
}

static void data_weaveseq_add(UInt32 sb, PageMeta_BTOC_t *meta)
{
    WeaveSeq_t maxWeave;

    if (META_IS_BTOC_DATA(meta)) {
        s_sb_set_num_btoc_vbas_0ok(sb, META_BTOC_GET_NUM_VBAS(meta));
        maxWeave = META_GET_WEAVESEQ_MAX(meta);
    } else {
        s_sb_set_num_btoc_vbas_0ok(sb, 0);
        maxWeave = ~0LL;
        WMR_ASSERT(0xffffffffffffffffLL == maxWeave);
    }

    s_sb_set_static(sb, META_IS_STATIC(meta));
    s_sb_set_type(sb, S_SB_DATA);
    sftl.seaState.data_sb++;
    s_sb_log_next();

    wo_append(sb, META_GET_WEAVESEQ_MIN(meta), maxWeave);
}

static void wo_append(UInt32 sb, WeaveSeq_t minWeaveSeq, WeaveSeq_t maxWeaveSeq)
{
    s_cxt_diff_t *const diff = &sftl.cxt.diff;
    UInt32 idx;

#if AND_SIMULATOR
    Int32 i;

    // Debug code to make sure it's not already on the list
    for (i = (Int32)diff->size-1; i >= 0; i--) {
        if (diff->item[i].sb == sb) {
            WMR_PANIC("duplicate item on weave list");
        }
    }
#endif

    // Put it on the list
    idx = diff->size;
    diff->size++;
    WMR_ASSERT(diff->size < s_g_max_sb);

    diff->item[idx].sb = sb;
    diff->item[idx].minWeaveSeq = minWeaveSeq;
    diff->item[idx].maxWeaveSeq = maxWeaveSeq;
}

static void wo_sort(void)
{
    s_cxt_diff_t *const diff = &sftl.cxt.diff;
    WeaveSeq_t min;
    UInt32 minIdx, i, j;
    s_wo_item_t tmp;

    if (0 == diff->size) {
        return;
    }

    for (i = 0; i < diff->size-1; i++) {
        min = diff->item[i].minWeaveSeq;
        minIdx = i;

        // Find min
        for (j = i+1; j < diff->size; j++) {
            if (diff->item[j].minWeaveSeq < min) {
                min = diff->item[j].minWeaveSeq;
                minIdx = j;
            }
        }

        // Found the smallest element?
        if (minIdx != i) {
            // Swap
            tmp = diff->item[i];
            diff->item[i] = diff->item[minIdx];
            diff->item[minIdx] = tmp;
        }
    }
}

static void clean_add(UInt32 sb, BOOL32 force)
{
    s_sb_set_type(sb, force ? S_SB_PENDING_ERASE : S_SB_ERASE_BEFORE_USE);
    sftl.seaState.free_sb++;
    sftl.seaState.vbas_in_free_sb += sftl.vfl->GetVbasPerVb(sb);
    s_diff_sbFilter_add(sb);
}

