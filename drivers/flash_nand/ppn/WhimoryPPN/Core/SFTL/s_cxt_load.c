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

#include "s_cxt_load.h"
#include "s_cxt_diff.h"
#include "s_geom.h"
#include "s_stats.h"
#include "s_sb.h"
#include "s_dbg.h"
#include "s_read.h"

static BOOL32 cxtLoadBase(BOOL32 *enumerate, BOOL32 statsOnly)
{
    s_cxt_t *const cxt = &sftl.cxt;
    UInt32 *base = (UInt32*)sftl.gc.zone;
    PageMeta_t *baseMeta = sftl.gc.meta;
    UInt32 *buf;
    UInt32 sb, vba, num, bufMaxIdx;
    VFLReadStatusType vfl_status;
    const VFLReadStatusType vfl_status_mask = (VFL_READ_STATUS_UECC | VFL_READ_STATUS_CLEAN |
                                               VFL_READ_STATUS_VALID_DATA | VFL_READ_STATUS_UNIDENTIFIED);
    const VFLReadStatusType vfl_status_expected = VFL_READ_STATUS_VALID_DATA;

    *enumerate = TRUE32;

    sb = cxt->load.baseSb[cxt->load.baseCur];
    vba = s_g_addr_to_vba(sb, 0);

    vfl_status = sftl.vfl->ReadMultipleVbas(vba, s_g_vbas_per_page, (UInt8*)base, (UInt8*)baseMeta, FALSE32, FALSE32);
    if ((vfl_status & vfl_status_mask) != vfl_status_expected) {
        return FALSE32;
    }

    // Suck out sb's
    buf = base;
    num = *buf++;
    cxt->save.num_sb = 0;
    while (num--) {
        WMR_ASSERT(cxt->save.num_sb < S_CXT_MAX_SB);
        cxt->save.sb[cxt->save.num_sb] = *buf++;
        cxt->save.num_sb++;
    }
    WMR_ASSERT(0 != cxt->save.num_sb);
    WMR_ASSERT(cxt->save.sb[0] == sb);

    // Check FTL superblocks
    buf = base;
    bufMaxIdx = (s_g_bytes_per_page / sizeof(*buf)) - 1;
    if (!statsOnly && (0 != buf[bufMaxIdx]) && (buf[bufMaxIdx] != s_g_max_sb)) {
        WMR_PRINT(ALWAYS, "context-stored FTL superblocks doesn't match current--enmuerating as blank...\n");
        *enumerate = FALSE32;
        return FALSE32;
    }

    cxt->save.num_sb_used = 0;
    cxt->load.baseWeaveSeq = META_GET_WEAVESEQ(&sftl.gc.meta[0]);

    return TRUE32;
}

static BOOL32 toTree(UInt8* data8)
{
    s_cxt_t *const cxt = &sftl.cxt;
    s_cxt_contig_t *contig = (s_cxt_contig_t*)data8;
    UInt32 count = s_g_bytes_per_lba / sizeof(s_cxt_contig_t);
    UInt32 lba, baseLba, sb;

    if (0xffffffff == contig->span) {
        // Unused
        return TRUE32;
    }
    WMR_ASSERT(0xfffffff0 == contig->span);
    baseLba = contig->vba;
    contig++;
    count--;

    if (baseLba != cxt->load.curLba) {
        WMR_PRINT(ALWAYS, "sftl: error, cxt lba not consecutive: %d, %d\n", baseLba, cxt->load.curLba);
        return FALSE32;
    }

    lba = baseLba;
    while (count--) {
        if (0xffffffff == contig->vba) {
            break;
        }
        WMR_ASSERT(0 != contig->span);
        sb = s_g_vba_to_sb(contig->vba);
        if ((L2V_VBA_SPECIAL <= contig->vba) || !s_diff_sbFilter_has(sb)) {
            L2V_Update(lba, contig->span, contig->vba);
        }
        lba += contig->span;
        contig++;
    }

    cxt->load.curLba = lba;
    return TRUE32;
}

static BOOL32 interpret(PageMeta_t *meta, UInt8* data, UInt32 vba, BOOL32 statsOnly, s_sb_fmt_t *shadowSB)
{
    s_cxt_t *const cxt = &sftl.cxt;
    UInt32 tag, ofs, thisCount, i;
    UInt32 *readPtr;
    s_sb_fmt_t *sbarr;
    s_sb_userSeq_t *userSeqPtr;

    tag = META_GET_CXT_TAG(meta);

    if (!META_IS_CXT(meta))
        return (S_CXTTAG_CLEAN == tag);

    if (statsOnly) {
        // For the format path, we may want to load only stats
        if (S_CXTTAG_STATS == tag) {
            if (!cxt->load.sawStats_cxt) {
                s_stats_from_buf(data, s_g_bytes_per_lba);
                cxt->load.sawStats_cxt = 1;
            }
        }
        return TRUE32;
    }

    if (cxt->load.total_clean && (S_CXTTAG_CLEAN != tag)) {
        WMR_PRINT(ALWAYS, "sftl: error, unexpected clean content in the middle of context\n");
        return FALSE32;
    }

    switch (tag) {
        case S_CXTTAG_CLEAN:
            // Clean: ignore
            cxt->load.total_clean++;
            break;

        case S_CXTTAG_BASE:
            // Ignore
            break;

        case S_CXTTAG_STATS:
            // Remember this vba for later:
            if (!cxt->load.sawStats_cxt) {
                cxt->load.sawStats_cxt = 1;
                cxt->load.stats_cxt_vba = vba;
            }
            break;

        case S_CXTTAG_SB:
            ofs = META_GET_CXT_OFS(meta);
            thisCount = META_GET_CXT_LEN(meta);
            if (0 == thisCount) {
                // Ignore 0-count vbas past the end of the array; it may occur becase of padding
                break;
            }
            WMR_ASSERT((ofs + thisCount) >= ofs);
            WMR_ASSERT((ofs+thisCount) <= s_g_max_sb);
            sbarr = (s_sb_fmt_t*)data;
            if(shadowSB != NULL)
            {
                WMR_MEMCPY(&(shadowSB[ofs]), sbarr, (thisCount * sizeof(s_sb_fmt_t)));
            }
            for (i = 0; i < thisCount; i++) {
                sftl.sb[ofs+i].erases = sbarr[i].erases;
            }
            break;

        case S_CXTTAG_TREE:
            if (!toTree(data)) {
                return FALSE32;
            }
            break;

        case S_CXTTAG_USERSEQ:
            ofs = META_GET_CXT_OFS(meta);
            thisCount = META_GET_CXT_LEN(meta);
            if (0 == thisCount) {
                // Ignore 0-count vbas past the end of the array; it may occur because of padding
                break;
            }
            WMR_ASSERT((ofs + thisCount) >= ofs);
            WMR_ASSERT((ofs+thisCount) <= s_g_max_sb);
            userSeqPtr = (s_sb_userSeq_t*)data;
            for (i = 0; i < thisCount; i++) {
                sftl.sb_userSeq[ofs+i] = userSeqPtr[i];
            }
            break;

        case S_CXTTAG_READS:
            ofs = META_GET_CXT_OFS(meta);
            thisCount = META_GET_CXT_LEN(meta);
            if (0 == thisCount) {
                // Ignore 0-count vbas past the end of the array; it may occur becase of padding
                break;
            }
            WMR_ASSERT((ofs + thisCount) >= ofs);
            WMR_ASSERT((ofs+thisCount) <= s_g_max_sb);
            readPtr = (UInt32*)data;
            for (i = 0; i < thisCount; i++) {
                sftl.sb[ofs+i].reads = readPtr[i];
            }
            break;

        default:
            WMR_PRINT(ALWAYS, "cxt load: unknown tag type %d\n", tag);
            break;
    }

    return TRUE32;
}

static void cxtReadInterpret_cb(UInt32 vba, VFLReadStatusType status, UInt8 *__meta)
{
    s_cxt_t *const cxt = &sftl.cxt;
    
    if (status & VFL_READ_STATUS_UNIDENTIFIED) {
        WMR_PRINT(QUAL_FATAL, "cxt load got unidentified read status (0x%x) on vba:0x%x; did we get a timeout?  Pretending it was uECC.\n", status, vba);
        status = (status & ~VFL_READ_STATUS_UNIDENTIFIED) | VFL_READ_STATUS_UECC;
    }

    if (status & VFL_READ_STATUS_UECC) {
        cxt->load.scan_uecc++;
    } else if (status & VFL_READ_STATUS_CLEAN) {
        META_SET_CXT_TAG(__meta, S_CXTTAG_CLEAN);
        cxt->load.scan_clean++;
    } else if (status & VFL_READ_STATUS_VALID_DATA) {
        cxt->load.scan_ok++;
    }
}

static BOOL32 cxtReadInterpret(BOOL32 statsOnly, s_sb_fmt_t *shadowSB)
{
    // Read all sb's, interpreting and loading vbas
    // Non-cxt superblocks -> remove from sb list (are clean or data)

    s_cxt_t *const cxt = &sftl.cxt;
    UInt32 vba, vbaOfs, thisCount;
    UInt32 sb, curSb, vbaMax, i;
    UInt32 wasCxt;

    cxt->load.total_clean = 0;

    for (curSb = 0; curSb < cxt->save.num_sb; curSb++) {
        sb = cxt->save.sb[curSb];
        vbaOfs = 0;
        vbaMax = sftl.vfl->GetVbasPerVb(sb);
        wasCxt = 0;

        while (vbaOfs < vbaMax) {
            thisCount = WMR_MIN(vbaMax - vbaOfs, sftl.gc.zoneSize);

            cxt->load.scan_clean = 0;
            cxt->load.scan_uecc = 0;
            cxt->load.scan_ok = 0;

            sftl.vfl->ReadSpansInit(&cxt->load.rs, cxtReadInterpret_cb, VFL_READ_STATUS_ALL, TRUE32, FALSE32);

            vba = s_g_addr_to_vba(sb, vbaOfs);
            sftl.vfl->ReadSpansAdd(&cxt->load.rs, vba, thisCount, (UInt8*)sftl.gc.zone, (UInt8*)sftl.gc.meta);
            sftl.vfl->ReadSpans(&cxt->load.rs);

            if (cxt->load.scan_uecc)
                return FALSE32;
            if ((cxt->load.scan_ok) && META_IS_CXT(&sftl.gc.meta[0]) && (0 == vbaOfs))
                wasCxt = 1;

            // Iterate over vbas we just read, and interpret
            for (i = 0; i < thisCount; i++) {
                if (!interpret(&sftl.gc.meta[i], &sftl.gc.zone[s_g_mul_bytes_per_lba(i)], vba+i, statsOnly, shadowSB))
                {
                    return FALSE32;
                }
            }

            if ((cxt->load.scan_clean == thisCount) && (thisCount == sftl.gc.zoneSize)) {
                // Early exit when we see a lot of clean; this is normal
                cxt->save.num_sb_used += wasCxt;
                goto exit;
            }

            vbaOfs += thisCount;
        }

        cxt->save.num_sb_used += wasCxt;
    }

exit:
    cxt->save.num_sb = cxt->save.num_sb_used;
    return TRUE32;
}

void cxtRamNuke(void)
{
    s_cxt_t *const cxt = &sftl.cxt;
    UInt32 i;

    cxt->load.baseCur++;
    L2V_Nuke();
    WMR_MEMSET(&sftl.stats, 0, sizeof(sftl.stats));

    cxt->load.sawStats_cxt = 0;
    cxt->load.sawStats_lba = 0;
    cxt->save.num_sb = 0;

    for (i = 0; i < s_g_max_sb; i++) {
        sftl.sb[i].erases = 0;
        sftl.sb[i].reads = 0;
        sftl.sb[i].validLbas = 0;
    }
    sftl.seaState.validLbas = 0;
}

void s_cxt_load_finish(void)
{
#ifndef AND_READONLY
    // Erase all old cxts
    s_cxt_t *const cxt = &sftl.cxt;
    UInt32 sb, i;
    BOOL32 skip;

    for (sb = 0; sb < s_g_max_sb; sb++) {
        if (S_SB_CXT == s_sb_get_type(sb)) {
            skip = FALSE32;
            for (i = 0; i < cxt->save.num_sb; i++) {
                if (sb == cxt->save.sb[i])
                    skip = TRUE32;
            }
            if (!skip) {
                s_sb_cxt_free_erase(sb);
            }
        }
    }
#endif
}

BOOL32 s_cxt_load(BOOL32 statsOnly, s_sb_fmt_t *shadowSB)
{
    s_cxt_t *const cxt = &sftl.cxt;
    BOOL32 status, enumerate;
    BOOL32 retStatus = TRUE32;

    // Try to load contexts, from newest to oldest
    cxt->load.baseCur = 0;
    s_dbg_check_validSums();

again:
    cxt->load.baseWeaveSeq = 0;
    // No good cxts
    if (cxt->load.baseCur >= cxt->load.bases) {
        goto exit;
    }

    // Load base
    if (!cxtLoadBase(&enumerate, statsOnly)) {
        WMR_PRINT(ALWAYS, "couldn't load cxt %d; issue with base...\n", cxt->load.baseCur);
        cxtRamNuke();

        if (!enumerate) {
            retStatus = FALSE32;
            goto exit;
        }
        // Force a context save post-boot
        cxt->periodic.sbsSince = S_CXT_PERIOD_SBS;
        goto again;
    }

    if (!statsOnly) {
        s_diff_filterPost(cxt->load.baseWeaveSeq);
    }

    // Read and interpret
    cxt->load.curLba = 0;
    status = cxtReadInterpret(statsOnly, shadowSB);
    if (!status) {
        WMR_PRINT(ALWAYS, "couldn't load cxt %d; uECC or missing pages...\n", cxt->load.baseCur);
        cxtRamNuke();
        // Force a context save post-boot
        cxt->periodic.sbsSince = S_CXT_PERIOD_SBS;
        goto again;
    }

    if (!statsOnly) {
        // Tree not fully populated?
        // Try older one...
        if (cxt->load.curLba < sftl.max_lba) {
            WMR_PRINT(ALWAYS, "couldn't load cxt %d; tree not fully loaded...\n", cxt->load.baseCur);
            cxtRamNuke();
            // Force a context save post-boot
            cxt->periodic.sbsSince = S_CXT_PERIOD_SBS;
            goto again;
        }
    }

    // Set up weaveSeq to be newer than cxt
    sftl.write.weaveSeq = sftl.cxt.load.baseWeaveSeq + (sftl.cxt.save.num_sb * s_g_vbas_per_sb) + 1;

    s_dbg_check_validSums();

exit:
    if (!statsOnly) {
        // Erase any cxts not currently used
        s_cxt_load_finish();
    }

    return retStatus;
}

#ifndef AND_READONLY
void s_cxt_stats_load(void)
{
    VFLReadStatusType status;

    s_cxt_t *const cxt = &sftl.cxt;

    WMR_ASSERT(!cxt->load.loadedStats);
    cxt->load.loadedStats = TRUE32;

    if (cxt->load.sawStats_lba) {
        if (ANDErrorCodeOk == s_read_int(S_LBA_STATS, 1, sftl.tmpBuf)) {
            s_stats_from_buf(sftl.tmpBuf, s_g_bytes_per_lba);
        } else {
            WMR_PRINT(ERROR, "could not load sftl stats");
        }
    } else if (cxt->load.sawStats_cxt) {
        sftl.vfl->ReadSpansInit(&cxt->load.rs, NULL, VFL_READ_STATUS_ALL, TRUE32, FALSE32);
        sftl.vfl->ReadSpansAdd(&cxt->load.rs, cxt->load.stats_cxt_vba, 1, sftl.tmpBuf, (UInt8*)sftl.boot.meta);
        status = sftl.vfl->ReadSpans(&cxt->load.rs);
        if (!(status & (VFL_READ_STATUS_UECC | VFL_READ_STATUS_CLEAN))) {
            s_stats_from_buf(sftl.tmpBuf, s_g_bytes_per_lba);
        } else {
            WMR_PRINT(ERROR, "could not load sftl stats. status = 0x%x", status);
        }
    }
}
#endif // !AND_READONLY

