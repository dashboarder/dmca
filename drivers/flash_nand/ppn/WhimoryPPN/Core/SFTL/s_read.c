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

#include "WMRConfig.h"
#include "s_read.h"
#include "s_readdist.h"
#include "s_geom.h"
#include "s_sb.h"
#include "s_stats.h"
#ifndef AND_READONLY
#include "s_write.h"
#include "s_gc.h"
#endif

BOOL32 s_verify_meta(UInt32 len, PageMeta_t *m)
{
    UInt32 i;
    PageMeta_t *meta = m;
    FTLExtent_t *extent = sftl.read.batch.extents;
    UInt32 offset = sftl.read.batch.offset;

    for (i = 0; i < len; i++, meta++, offset++) {
        if (offset >= extent->span)
        {
            offset = 0;
            extent++;
        }

        if (META_GET_LBA(meta) != (extent->lba+offset)) {
#ifndef AND_READONLY
            WMR_PANIC("sftl: lba mismatch; lba:0x%llx, meta:0x%x\n", (UInt64)extent->lba+offset, META_GET_LBA(meta));
#else
            WMR_PRINT(QUAL_FATAL, "sftl: lba mismatch; lba:0x%llx, meta:0x%x\n", (UInt64)extent->lba+offset, META_GET_LBA(meta));
#endif
        }
        if (META_IS_UECC(meta)) {
            WMR_PRINT(QUAL, "sftl: uECC flag in meta; lba:0x%llx, meta:0x%x\n", (UInt64)extent->lba+offset, META_GET_LBA(meta));
            return FALSE32;
        }
    }

    return TRUE32;
}

UInt32 s_read_xlate(UInt32 lba)
{
#ifndef AND_READONLY
    UInt32 i, j;
    UInt32 bufLba;
#endif // AND_READONLY

    if (lba >= sftl.max_lba) {
        return AND_GET_ADDRESS_OUT_OF_RANGE;
    }
    
#ifndef AND_READONLY
    // Return token value of AND_GET_ADDRESS_CACHED_WRITE if it's currently buffered
    for (i = 0; i < S_SBSTREAM_MAX; i++) {
        for (j = 0; j < sftl.write.stream[i].bufLbas; j++) {
            bufLba = META_GET_LBA(&sftl.write.stream[i].bufMeta[j]);
            if (bufLba == lba) {
                return AND_GET_ADDRESS_CACHED_WRITE;
            }
        }
    }
#endif // AND_READONLY

    sftl.read.c.lba = lba;
    L2V_Search(&sftl.read.c);

    if (L2V_VBA_DEALLOC == sftl.read.c.vba) {
        return AND_GET_ADDRESS_TRIMMED;
    }
    
    return sftl.read.c.vba;
}

void sftl_read_cb(UInt32 vba, VFLReadStatusType status, UInt8 *meta)
{
    PageMeta_t *pMeta = (PageMeta_t *)meta;
    FTLExtent_t *extent = sftl.read.batch.extents;
    UInt32 offset = sftl.read.batch.offset;
#ifndef AND_READONLY
    UInt32 i;
#endif // AND_READONLY

    offset += pMeta - sftl.read.meta;
    while (offset >= extent->span)
    {
        offset -= (UInt32)extent->span;
        extent++;
    }

    if (status & VFL_READ_STATUS_UNIDENTIFIED) {
        WMR_PRINT(QUAL_FATAL, "read got unidentified read status (0x%x) on vba:0x%x; did we get a timeout?  Pretending it was uECC.\n", status, vba);
        status = (status & ~VFL_READ_STATUS_UNIDENTIFIED) | VFL_READ_STATUS_UECC;
    }

    if (status & VFL_READ_STATUS_REFRESH) {
        WMR_PRINT(QUAL, "sftl: requested refresh lba:%lld vba:0x%x status:0x%x\n", (UInt64)extent->lba+offset, vba, status);
#ifndef AND_READONLY
        s_readRefresh_enq(vba);
#endif // AND_READONLY
    }
    if (status & VFL_READ_STATUS_RETIRE) {
        WMR_PRINT(QUAL, "sftl: requested retire lba:%lld vba:0x%x status:0x%x\n", (UInt64)extent->lba+offset, vba, status);
#ifndef AND_READONLY
        s_readRefresh_enq(vba);
#endif // AND_READONLY
    }
    if (status & VFL_READ_STATUS_UECC) {
        WMR_PRINT(QUAL, "sftl: error uECC lba:%lld vba:0x%x status:0x%x\n", (UInt64)extent->lba+offset, vba, status);
    }
    if (status & VFL_READ_STATUS_CLEAN) {
        WMR_PRINT(QUAL, "sftl: error clean lba:%lld vba:0x%x status:0x%x\n", (UInt64)extent->lba+offset, vba, status);
    }

#ifndef AND_READONLY
    if ((status & VFL_READ_STATUS_REFRESH) || (status & VFL_READ_STATUS_RETIRE) || (status & VFL_READ_STATUS_UECC)) {
        for (i = 0; i < S_SBSTREAM_MAX; i++) {
            if (sftl.write.stream[i].sb == s_g_vba_to_sb(vba)) {
                sftl.write.stream[i].abandon = TRUE32;
            }
        }
    }
#endif // AND_READONLY
}

Int32 s_read_real(FTLExtent_t *origExtent, UInt32 origCount, UInt8 *origBuf)
{
    UInt32 curLba, curCount;
    FTLExtent_t *curExtent;
    UInt32 thisCount, batchCount;
    UInt8 *curBuf = origBuf;
    PageMeta_t *batchMeta;
    Int32 status;
    ANDStatus statusRet = ANDErrorCodeOk;
    UInt32 nandSpans;
    s_wrstream_t *const wr = &sftl.write.stream[sftl.write.curStream]; // for trim inject
#ifndef AND_READONLY
    UInt32 i;
#endif // AND_READONLY

    curExtent = origExtent;
    curLba = (UInt32)curExtent->lba;
    curCount = (UInt32)curExtent->span;
    batchCount = 0;
    nandSpans = 0;
    sftl.read.c.span = 0;
    batchMeta = sftl.read.meta;

    sftl.read.batch.extents = curExtent;
    sftl.read.batch.offset = 0;

    sftl.vfl->ReadSpansInit(&sftl.read.rs, sftl_read_cb, VFL_READ_STATUS_ALL, FALSE32, TRUE32);

    while (curCount)
    {
        // Translate
        if (0 == sftl.read.c.span) {
            sftl.read.c.lba = curLba;
            L2V_Search(&sftl.read.c);
        }
        WMR_ASSERT(0 != sftl.read.c.span);

        // How much?
        thisCount = WMR_MIN(WMR_MIN(sftl.read.c.span, curCount), s_g_vbas_per_sb - batchCount);

        // Real data, or trimmed token?
        if (L2V_VBA_SPECIAL > sftl.read.c.vba) {
            sftl.vfl->ReadSpansAdd(&sftl.read.rs, sftl.read.c.vba, thisCount, curBuf, (UInt8*)batchMeta);
#ifndef AND_READONLY
            s_readDist_add(sftl.read.c.vba, thisCount);
#endif // AND_READONLY
            sftl.read.c.vba += thisCount;
            nandSpans++;
        } else {
            s_SetupMeta_TrimRead(batchMeta, curLba, thisCount, wr->pageFlags);
        }
        sftl.read.c.span -= thisCount;

        // Next
        curBuf += s_g_mul_bytes_per_lba(thisCount);
        batchMeta += thisCount;

        curCount -= thisCount;
        curLba += thisCount;
        batchCount += thisCount;

        if (0 >= curCount)
        {
            curExtent++;
            if ((UInt32)(curExtent - origExtent) < origCount)
            {
                curLba = (UInt32)curExtent->lba;
                curCount = (UInt32)curExtent->span;
                sftl.read.c.span = 0;
            }
        }

        // End of line, or saturated batch?
        if (!curCount || (batchCount == s_g_vbas_per_sb)) {
            if (nandSpans) {
                status = sftl.vfl->ReadSpans(&sftl.read.rs);
                if (!(status & (VFL_READ_STATUS_UECC | VFL_READ_STATUS_CLEAN))) {
                    if(!s_verify_meta(batchCount, sftl.read.meta)) {
                        statusRet = ANDErrorCodeUserDataErr;
                    }
                } else {
                    statusRet = ANDErrorCodeUserDataErr;
                }
            }
            if (curCount) {
                // Next round's on me...
                sftl.vfl->ReadSpansInit(&sftl.read.rs, sftl_read_cb, (VFL_READ_STATUS_REFRESH | VFL_READ_STATUS_RETIRE), TRUE32, FALSE32);
                batchCount = 0;
                nandSpans = 0;
                batchMeta = sftl.read.meta;
                sftl.read.batch.extents = curExtent;
                sftl.read.batch.offset = curLba - (UInt32)curExtent->lba;
            }
        }
    }

#ifndef AND_READONLY
    for (i = 0; i < S_SBSTREAM_MAX; i++) {
        if (sftl.write.stream[i].abandon) {
            s_sb_next(i);
            sftl.write.stream[i].abandon = FALSE32;
        }
    }

    s_readDist_end_of_read_handle();
#endif // AND_READONLY

    return statusRet;
}

Int32 s_read_int(UInt32 origLba, UInt32 origCount, UInt8 *origBuf)
{
    Int32 status;
    FTLExtent_t extent;

    if (((origLba + origCount) > sftl.max_lba) || (origLba < sftl.reported_lba))
    {
        return ANDErrorCodeHwErr;
    }

    extent.lba = origLba;
    extent.span = origCount;
    status = s_read_real(&extent, 1, origBuf);

    return status;
}

#ifndef AND_READONLY
void s_read_coherency(UInt32 lba, UInt32 count) {
    UInt32 i, j;
    UInt32 bufLba;

    for (i = 0; i < S_SBSTREAM_MAX; i++) {
        for (j = 0; j < sftl.write.stream[i].bufLbas; j++) {
            bufLba = META_GET_LBA(&sftl.write.stream[i].bufMeta[j]);
            if ((bufLba >= lba) && (bufLba < (lba + count))) {
                // Hit -- drain this stream
                s_drain_stream(i, TRUE32);
            }
        }
    }
}

static void pad_current_block(void)
{
    UInt8 i;
    UInt32 currentSb;
    for (i = 0; i < S_SBSTREAM_MAX; i++) {
        currentSb = sftl.write.stream[i].sb;
        if ((currentSb < s_g_max_sb) && (sftl.sb[currentSb].reads > S_READDIST_LIMIT_PER_OPEN_SB)) {
            if(s_pad_block(currentSb) == FALSE32) {
                s_gc_data_enq(currentSb, S_GC_CTX_FG);
            }
            break;
        }
    }
}
#endif // AND_READONLY

Int32 sftl_read(UInt32 origLba, UInt32 origCount, UInt8 *origBuf)
{
    Int32 status;
    FTLExtent_t extent;

    sftl.stats.xacts_read++;
    switch (origCount)
       {
        case 0 : break;     
        case 1 : sftl.stats.lbas_read_1++; 
                 break;
        case 2 : sftl.stats.lbas_read_2++; 
                 break;
        case 3 : sftl.stats.lbas_read_3++; 
                 break;
        case 4 : sftl.stats.lbas_read_4++; 
                 break;
        case 5 : sftl.stats.lbas_read_5++; 
                 break;
        case 6 : sftl.stats.lbas_read_6++; 
                 break;
        case 7 : sftl.stats.lbas_read_7++; 
                 break;
        case 8 : sftl.stats.lbas_read_8++; 
                 break;
        default : sftl.stats.lbas_read_over_8++;
    }

    if ((origLba + origCount) > sftl.reported_lba)
    {
        return ANDErrorCodeHwErr;
    }

#ifndef AND_READONLY
    s_read_coherency(origLba, origCount);
#endif // AND_READONLY

    extent.lba = origLba;
    extent.span = origCount;
    status = s_read_real(&extent, 1, origBuf);

    sftl.stats.lbas_read += origCount;

    s_stats_update();

#ifndef AND_READONLY
    pad_current_block();
#endif

    return status;
}

Int32 sftl_read_spans(FTLExtent_t *extents, UInt32 count, UInt8 *buf)
{
    Int32 status;
    UInt32 i;

    sftl.stats.xacts_read += count;
    sftl.stats.span_xacts_read++;
    sftl.stats.span_freebies_read += count - 1;

    for (i = 0 ; i < count ; i++)
    {
        UInt32 lba = (UInt32)extents[i].lba;
        UInt32 span = (UInt32)extents[i].span;

        switch (span)
        {
            case 0 : break;
            case 1 : sftl.stats.lbas_read_1++;
                    break;
            case 2 : sftl.stats.lbas_read_2++;
                    break;
            case 3 : sftl.stats.lbas_read_3++;
                    break;
            case 4 : sftl.stats.lbas_read_4++;
                    break;
            case 5 : sftl.stats.lbas_read_5++;
                    break;
            case 6 : sftl.stats.lbas_read_6++;
                    break;
            case 7 : sftl.stats.lbas_read_7++;
                    break;
            case 8 : sftl.stats.lbas_read_8++;
                    break;
            default : sftl.stats.lbas_read_over_8++;
        }

        sftl.stats.lbas_read += span;

        if ((lba + span) > sftl.reported_lba)
        {
            return ANDErrorCodeHwErr;
        }

#ifndef AND_READONLY
        s_read_coherency(lba, span);
#endif // AND_READONLY
    }

    status = s_read_real(extents, count, buf);

    s_stats_update();
    return status;
}

