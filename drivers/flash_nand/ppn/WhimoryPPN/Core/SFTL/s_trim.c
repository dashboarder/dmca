/*
 * Copyright (c) 2008-2009 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */

#include "s_internal.h"
#include "s_trim.h"
#include "s_token.h"
#include "s_write.h"
#include "s_flatten.h"
#include "s_sb.h"
#include "s_gc.h"
#include "s_geom.h"
#include "s_stats.h"
#include "s_dbg.h"

#ifndef AND_READONLY

#define BLOOM_BITS (sizeof(*sftl.trim.bloomFilter)*8)
#define VULNERABLE_BITS (sizeof(*sftl.trim.vulnerable)*8)

BOOL32 s_trim_init(void)
{
    sftl.trim.count = 0;
    sftl.trim.sumSpan = 0;
    sftl.trim.max = ((s_g_bytes_per_lba - sizeof(UInt32)) / sizeof(s_trim_item_t));
    sftl.trim.vulnerableWords = (s_g_vfl_max_sb+VULNERABLE_BITS-1)/VULNERABLE_BITS;
    sftl.trim.bloomFilterWords = ((((s_g_vfl_max_sb * s_g_vbas_per_sb) + S_TRIM_BLOOMFILTER_SPAN - 1) / S_TRIM_BLOOMFILTER_SPAN) + BLOOM_BITS-1) / BLOOM_BITS;

    sftl.trim.item = WMR_MALLOC(s_g_bytes_per_lba);
    sftl.trim.vulnerable = WMR_MALLOC(sftl.trim.vulnerableWords * sizeof(*sftl.trim.vulnerable));
    sftl.trim.bloomFilter = WMR_MALLOC(sftl.trim.bloomFilterWords * sizeof(*sftl.trim.bloomFilter));
    return (NULL != sftl.trim.item) && (NULL != sftl.trim.vulnerable) && (NULL != sftl.trim.bloomFilter);
}

void s_trim_close(void)
{
    sftl.trim.count = 0;
    sftl.trim.sumSpan = 0;
    if (NULL != sftl.trim.item) {
        WMR_FREE(sftl.trim.item, s_g_bytes_per_lba);
        sftl.trim.item = NULL;
    }
    if (NULL != sftl.trim.vulnerable) {
        WMR_FREE(sftl.trim.vulnerable, sftl.trim.vulnerableWords * sizeof(*sftl.trim.vulnerable));
        sftl.trim.vulnerable = NULL;
    }
    if (NULL != sftl.trim.bloomFilter) {
        WMR_FREE(sftl.trim.bloomFilter, sftl.trim.bloomFilterWords * sizeof(*sftl.trim.bloomFilter));
        sftl.trim.bloomFilter = NULL;
    }
}

void s_trim_apply(BOOL32 token)
{
    s_trim_t *const trim = &sftl.trim;
    UInt32 *base;
    UInt32 *buf;
    Int32 i;

    base = (UInt32*)sftl.tmpBuf;
    buf = base;
    *buf++ = trim->count;
    for (i = 0; i < trim->count; i++) {
        if (L2V_LowMem && ((trim->item[i].lbaEnd-trim->item[i].lba) < S_TRIM_CUTOFF_LOWMEM)) {
            (*base)--;
            continue;
        }
        *buf++ = trim->item[i].lba;
        *buf++ = trim->item[i].lbaEnd - trim->item[i].lba;
        L2V_Update(trim->item[i].lba, trim->item[i].lbaEnd - trim->item[i].lba, L2V_VBA_DEALLOC);
    }
    trim->count = 0;
    trim->sumSpan = 0;

    if (token) {
        s_token_insert(S_TOK_DELETE, (UInt32*)sftl.tmpBuf);
        s_write_push_full_buf(TRUE32);
    }

    // Clear bloom filter
    WMR_MEMSET(sftl.trim.bloomFilter, 0, sftl.trim.bloomFilterWords * sizeof(*sftl.trim.bloomFilter));
}

Int32 sftl_unmap(FTLExtent_t *extents, UInt32 numExtents)
{
    s_trim_t *const trim = &sftl.trim;
    UInt32 sb, lba, span, t, lbaEnd;
    UInt32 bloomBucket, bloomBucketMax;

    while (numExtents--) {
        lba = (UInt32)extents->lba;
        span = (UInt32)extents->span;
        lbaEnd = lba + span;
        WMR_ASSERT((lba + span) <= sftl.reported_lba);

        bloomBucketMax = lbaEnd / S_TRIM_BLOOMFILTER_SPAN;
        for (bloomBucket = lba / S_TRIM_BLOOMFILTER_SPAN; bloomBucket <= bloomBucketMax; bloomBucket++) {
            trim->bloomFilter[bloomBucket / BLOOM_BITS] |= 1 << (bloomBucket & (BLOOM_BITS-1));
        }

        // Insert into pending list, or compress
        if (trim->count && (trim->item[trim->count-1].lbaEnd == lba)) {
            trim->item[trim->count-1].lbaEnd = lbaEnd;
        } else {
            trim->item[trim->count].lba = lba;
            trim->item[trim->count].lbaEnd = lbaEnd;
            trim->count++;
        }
        trim->sumSpan += span;

        // Push trim vba?
        if (trim->count >= trim->max) {
            s_trim_apply(TRUE32);
        }

        s_dbg_check_data_counts();
        if ((0 == lba) && (sftl.reported_lba == span)) {
            s_trim_apply(TRUE32);
            // Erase all pending
            for (sb = 0; sb < s_g_max_sb; sb++) {
                t = s_sb_get_type(sb);
                if ((S_SB_PENDING_ERASE == t) || (S_SB_ERASE_BEFORE_USE == t)) {
                    s_sb_boot_free_erase(sb);
                }
            }
        }
        s_dbg_check_data_counts();

        // Move along
        extents++;
    }

    // Enough to justify an early push of the token?
    if ((trim->sumSpan >= S_TRIM_SUMSPAN_TRIGGER) && (L2V_LowMem || lowData(s_g_vbas_per_sb))) {
        s_trim_apply(TRUE32);
        s_drain_stream_cur(TRUE32);
    }

    s_stats_update();
    return ANDErrorCodeOk;
}

void s_trim_writeCollide(UInt32 lba, UInt32 span)
{
    s_trim_t *const trim = &sftl.trim;
    Int32 i, j;
    UInt32 tLbaMin, tLbaMax;
    UInt32 lbaEnd;
    BOOL32 bloomHit;
    UInt32 bloomBucket, bloomBucketMax;

    lbaEnd = lba + span;

    // Check bloom filter
    bloomHit = FALSE32;
    bloomBucketMax = lbaEnd / S_TRIM_BLOOMFILTER_SPAN;
    for (bloomBucket = lba / S_TRIM_BLOOMFILTER_SPAN; bloomBucket <= bloomBucketMax; bloomBucket++) {
        if (trim->bloomFilter[bloomBucket / BLOOM_BITS] & (1 << (bloomBucket & (BLOOM_BITS-1)))) {
            bloomHit = TRUE32;
            break;
        }
    }
    if (!bloomHit) {
        return;
    }

    // Find and manage collisions
    for (i = 0; i < trim->count; i++) {
        tLbaMin = trim->item[i].lba;
        tLbaMax = trim->item[i].lbaEnd;

        if (!((tLbaMax <= lba) || (lbaEnd <= tLbaMin))) {
            // Identity hit or eclipse?
            if ((lba <= tLbaMin) && (lbaEnd >= tLbaMax)) {
                // Slip back
                trim->sumSpan -= (tLbaMax - tLbaMin);
                for (j = i; j < (trim->count-1); j++) {
                    trim->item[j] = trim->item[j+1];
                }
                trim->count--;
                i--;
            }
            // Left-chop?
            else if ((lbaEnd < tLbaMax) && (lba <= tLbaMin)) {
                trim->sumSpan -= (lbaEnd - tLbaMin);
                trim->item[i].lba = lbaEnd;
            }
            // Right-chop?
            else if ((lba > tLbaMin) && (lbaEnd >= tLbaMax)) {
                trim->sumSpan -= (tLbaMax - lba);
                trim->item[i].lbaEnd = lba;
            }
            // Middle chop
            else {
                WMR_ASSERT(lba > tLbaMin);
                WMR_ASSERT(lbaEnd < tLbaMax);

                // No room to slip forward?  Drop it and move on
                if ((trim->count+1) >= trim->max) {
                    s_trim_apply(TRUE32);
                    return;
                }
                // Slip forward
                for (j = trim->count; j > i; j--) {
                    trim->item[j] = trim->item[j-1];
                }
                trim->count++;

                // Make right chop
                trim->item[i+1].lba = lbaEnd;

                // Chop to left
                trim->item[i].lbaEnd = lba;

                trim->sumSpan -= span;
            }
        }
    }
}

void s_trim_markVulnerable(UInt32 vba)
{
    s_trim_t *const trim = &sftl.trim;
    UInt32 sb;

    sb = s_g_vba_to_sb(vba);

    WMR_ASSERT((sb / VULNERABLE_BITS) < trim->vulnerableWords);
    trim->vulnerable[sb / VULNERABLE_BITS] |= (1 << (sb & (VULNERABLE_BITS-1)));
}

void s_trim_clearVulnerables(void)
{
    s_trim_t *const trim = &sftl.trim;

    WMR_MEMSET(trim->vulnerable, 0, trim->vulnerableWords * sizeof(*trim->vulnerable));
}

void s_trim_checkVulnerable(UInt32 sb)
{
    s_trim_t *const trim = &sftl.trim;

    if (trim->vulnerable[sb / VULNERABLE_BITS] & (1 << (sb & (VULNERABLE_BITS-1)))) {
        // Force a cxt save on the next write
        sftl.cxt.periodic.sbsSince = S_CXT_PERIOD_SBS;
    }
}

#endif

