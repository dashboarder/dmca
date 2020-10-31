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

#include "WMRConfig.h"
#include "s_internal.h"
#include "s_init.h"
#include "s_boot.h"
#include "s_read.h"
#include "s_write.h"
#include "s_trim.h"
#include "s_fmt.h"
#include "s_gc.h"
#include "s_wearlev.h"
#include "s_geom.h"
#include "s_sb.h"
#include "s_stats.h"
#include "FTL.h"
#include "VFL.h"

#define kSftlMinorVersion 1

static UInt32 _getMinorVersion(void)
{
    return kSftlMinorVersion;
}

#ifndef AND_READONLY
static Int32 sftl_wearlevel(void)
{
    return ANDErrorCodeOutOfRangeErr;
}


static void L2V_IdleRepack(void)
{
    if(L2V_IdleRepackThr)
    {
        L2V_ForceRepack();
    }
}

static Int32 sftl_IdleGCStripe(void)
{
    static UInt32 ctx = S_GC_CTX_BG;
    Int32 ret = FTL_SUCCESS;
    
    if(s_gc_slice(ctx) == FALSE32)
    {
        /* Switch a stream */
        if(ctx == S_GC_CTX_BG)
            ctx = S_GC_CTX_FG;
        else
            ctx = S_GC_CTX_BG;
        if(s_gc_slice(ctx) == FALSE32)
        {
            /* Nothing to wearlevel? check the LV2 tree state */
            L2V_IdleRepack();
            /* Nothing more to do . Switch a stream and get out */
            if(ctx == S_GC_CTX_BG)
                ctx = S_GC_CTX_FG;
            else
                ctx = S_GC_CTX_BG;
            /* nothing more to do . get out */
            ret = FTL_OUT_OF_RANGE_ERROR;
        }
    }

    return ret;
}


static BOOL32 sftl_garbagecollect(void)
{
    return TRUE32;
}

static BOOL32 sftl_drain(void)
{
    s_drain_stream_all(TRUE32);
    return TRUE32;
}

// This function is stateless. Don't use stFTLDeviceInfo
static UInt32 sftl_ConvertUserMBtoFTLSuperblocks(VFLFunctions *vfl, UInt32 user_mb)
{
    UInt32 ftl_virtual_blocks;
    const UInt32 page_size = vfl->GetDeviceInfo(AND_DEVINFO_BYTES_PER_PAGE);
    const UInt32 pages_per_vb = vfl->GetDeviceInfo(AND_DEVINFO_PAGES_PER_SUBLK);
    const UInt32 vfl_vbs = vfl->GetDeviceInfo(AND_DEVINFO_BLOCKS_PER_CAU);
    const UInt32 requested_pages = (user_mb * 1024) / (page_size / 1024);
    
    ftl_virtual_blocks = (requested_pages / pages_per_vb) + 1;
    ftl_virtual_blocks += ((vfl_vbs * S_RESERVE_PERMIL_APPROX) + 999) / 1000;
    
    return ftl_virtual_blocks;
}
#endif //!AND_READONLY

static BOOL32 sftl_ec_bins(void *structBuffer, UInt32 *structSize)
{
    BOOL32 boolRes = FALSE32;
    UInt32 blockIdx, erases;
    FTLBinsStruct hdr;
    const UInt32 neededStructSize = sizeof(hdr) + sizeof(*hdr.usage) * FTL_NUM_EC_BINS;

    if (!sftl.sb) {
        return FALSE32;
    }

    if (structBuffer && structSize && (*structSize >= neededStructSize)) {
        const UInt32 max_bin_val = FTL_MAX_EC_BIN_VAL;
        const UInt32 bin_size = FTL_EC_BIN_SIZE;
        UInt32 size = *structSize; // only used for WMR_FILL_STRUCT

        hdr.maxValue = max_bin_val;
        hdr.binCount = FTL_NUM_EC_BINS;
        WMR_MEMSET(structBuffer, 0, neededStructSize);
        WMR_FILL_STRUCT(structBuffer, &size, &hdr, sizeof(hdr));

        for (blockIdx = 0; blockIdx < s_g_max_sb; ++blockIdx) {
            UInt32 index;
            UInt16 usage;
            void *cursor;

            erases = s_sb_get_erases(blockIdx);

            if (erases >= max_bin_val) {
                index = FTL_NUM_EC_BINS-1;
            } else {
                index = erases/bin_size;
            }

            cursor = ((char *)structBuffer) + WMR_OFFSETOF(FTLBinsStruct, usage[index]);
            WMR_MEMCPY(&usage, cursor, sizeof(usage));
            usage++;
            WMR_MEMCPY(cursor, &usage, sizeof(usage));
        }
        boolRes = TRUE32;
    }

    if (structSize) {
        *structSize = neededStructSize;
        boolRes = TRUE32;
    }

    return boolRes;
}

static BOOL32 sftl_rc_bins(void *structBuffer, UInt32 *structSize)
{
    BOOL32 boolRes = FALSE32;
    UInt32 blockIdx, reads;
    FTLBinsStruct hdr;
    const UInt32 neededStructSize = sizeof(hdr) + sizeof(*hdr.usage) * FTL_NUM_RC_BINS;

    if (!sftl.sb) {
        return FALSE32;
    }

    if (structBuffer && structSize && (*structSize >= neededStructSize)) {
        const UInt32 max_bin_val = S_READDIST_LIMIT_PER_SB;
        const UInt32 bin_size = FTL_RC_BIN_SIZE(max_bin_val);
        UInt32 size = *structSize; // only used for WMR_FILL_STRUCT

        hdr.maxValue = max_bin_val;
        hdr.binCount = FTL_NUM_RC_BINS;
        WMR_MEMSET(structBuffer, 0, neededStructSize);
        WMR_FILL_STRUCT(structBuffer, &size, &hdr, sizeof(hdr));

        for (blockIdx = 0; blockIdx < s_g_max_sb; ++blockIdx) {
            UInt32 index;
            UInt16 usage;
            void *cursor;

            reads = s_sb_get_reads(blockIdx);

            if (reads >= max_bin_val) {
                index = FTL_NUM_RC_BINS-1;
            } else {
                index = reads/bin_size;
            }

            cursor = ((char *)structBuffer) + WMR_OFFSETOF(FTLBinsStruct, usage[index]);
            WMR_MEMCPY(&usage, cursor, sizeof(usage));
            usage++;
            WMR_MEMCPY(cursor, &usage, sizeof(usage));
        }
        boolRes = TRUE32;
    }

    if (structSize) {
        *structSize = neededStructSize;
        boolRes = TRUE32;
    }

    return boolRes;
}


static void fillUpTLBlockStructure(UInt32 blockNo, ANDFTLBlockStruct * tlBlockStruct)
{
    tlBlockStruct->blockSizeInLbas = sftl.vfl->GetVbasPerVb(blockNo);
    tlBlockStruct->validLbas = 0;
    tlBlockStruct->blockType = 0;
    switch (s_sb_get_type(blockNo))
    {
        case S_SB_DATA:
        case S_SB_DATA_CUR:
        case S_SB_DATA_GC:
        case S_SB_DATA_PENDING_GC:
        {
            tlBlockStruct->blockType |= TL_BLOCK_DATA;
            if(s_sb_get_static(blockNo))
                tlBlockStruct->blockType |= TL_BLOCK_DATA_STATIC;    
            tlBlockStruct->validLbas = s_sb_get_validLbas(blockNo);
            break;
        }    
            
        case S_SB_DEAD:
        {
            tlBlockStruct->blockType |= TL_BLOCK_DEAD;
            break;
        }
            
        case S_SB_ERASED:
        case S_SB_PENDING_ERASE:
        case S_SB_ERASE_BEFORE_USE:
        {
            tlBlockStruct->blockType |= TL_BLOCK_FREE;
            break;
        }
            
        case S_SB_CXT:
            tlBlockStruct->blockType |= TL_BLOCK_CXT;
            break;
            
        default:
            tlBlockStruct->blockType |= TL_BLOCK_UNKNOWN;
    }
}

static BOOL32 sftl_getStruct(UInt32 dwStructType, void *structBuffer, UInt32 *structSize)
{
    ANDAddressStruct addr;
    UInt32 addrSize = sizeof(addr);
    BOOL32 boolRes = FALSE32, boolFillRes;
    UInt8 FTLType;
    UInt32 FTLDevInfoSize;
    UInt32 value;
#ifdef AND_COLLECT_STATISTICS
    UInt32 statSize;
#endif
    sftl_devinfo_t *devI = (sftl_devinfo_t*)sftl.tmpBuf;

    switch (dwStructType) {
        case AND_STRUCT_FTL_GET_TYPE:
            FTLType = FTL_TYPE_SFTL;
            boolRes = WMR_FILL_STRUCT(structBuffer, structSize, &FTLType, sizeof(FTLType));
            break;

    #ifdef AND_COLLECT_STATISTICS
        case AND_STRUCT_FTL_GET_FTL_STATS_SIZE:
            statSize = s_stats_to_buf((UInt32*)sftl.tmpBuf);
            boolRes = WMR_FILL_STRUCT(structBuffer, structSize, &statSize, sizeof(statSize));
            break;

        case AND_STRUCT_FTL_STATISTICS:
            statSize = s_stats_to_buf((UInt32*)sftl.tmpBuf);
            boolRes = WMR_FILL_STRUCT(structBuffer, structSize, sftl.tmpBuf, statSize);
            break;
    #endif

        case AND_STRUCT_FTL_GET_FTL_DEVICEINFO_SIZE:
            FTLDevInfoSize = sizeof(sftl_devinfo_t);
            boolRes = WMR_FILL_STRUCT(structBuffer, structSize, &FTLDevInfoSize, sizeof(FTLDevInfoSize));
            break;

        case AND_STRUCT_FTL_GET_FTL_DEVICEINFO:
            devI->wPagesPerVb = s_g_vbas_per_sb;
            devI->wUserVbTotal = s_g_max_sb;
            devI->dwUserPagesTotal = s_g_vbas_per_sb * s_g_max_sb;
            devI->wBytesPerPage = s_g_bytes_per_lba;
            devI->wNumOfBanks = s_g_num_banks;
            devI->wBytesPerPageMeta = s_g_bytes_per_lba_meta;
            boolRes = WMR_FILL_STRUCT(structBuffer, structSize, devI, sizeof(sftl_devinfo_t));
            break;

        case AND_STRUCT_FTL_GETADDRESS:
            
            if (structBuffer && structSize && (*structSize >= sizeof(addr)))
            {    
                WMR_MEMCPY(&addr, structBuffer, sizeof(addr));

                addr.dwVpn = s_read_xlate(addr.dwLpn);
                if (AND_GET_ADDRESS_SPECIAL > addr.dwVpn) {
                    boolRes = sftl.vfl->GetStruct(AND_STRUCT_VFL_GETADDRESS, &addr, &addrSize);
                } else {
                    boolRes = FALSE32;
                }
            }
            
            boolFillRes = WMR_FILL_STRUCT(structBuffer, structSize, &addr, sizeof(addr));
            
            if (boolRes) {
                boolRes = boolFillRes;
            }
            break;

        case AND_STRUCT_FTL_SB_CYCLES:
        {
            UInt32 sbIndex;
            UInt32 numBufEntries = (*structSize) / sizeof(UInt16);
            UInt16 *sbEraseCnts = (UInt16 *) structBuffer;   
            UInt32 numSBlks = s_g_max_sb;

            if((numSBlks != numBufEntries) || (sbEraseCnts == NULL)){
                boolRes = FALSE32;
                break;
            }

            for (sbIndex=0; sbIndex < numSBlks; sbIndex++){
                sbEraseCnts[sbIndex] = sftl.sb[sbIndex].erases;
            }

            boolRes = TRUE32; 
            break;
        }

        case AND_STRUCT_FTL_SB_READ_CYCLES:
        {
            UInt32 sbIndex;
            UInt32 numBufEntries = (*structSize) / sizeof(UInt32);
            UInt32 *sbReadCnts = (UInt32 *) structBuffer;
            UInt32 numSBlks = sftl.vfl->GetDeviceInfo(AND_DEVINFO_NUM_OF_USER_SUBLK);

            if((numSBlks != numBufEntries) || (sbReadCnts == NULL)){
                boolRes = FALSE32;
                break;
            }

            for (sbIndex=0; sbIndex < numSBlks; sbIndex++){
                sbReadCnts[sbIndex] = sftl.sb[sbIndex].reads;
            }

            boolRes = TRUE32; 
            break;
        }

        case AND_STRUCT_FTL_GET_FTL_EC_BINS:
            boolRes = sftl_ec_bins(structBuffer, structSize);
            break;

        case AND_STRUCT_FTL_GET_FTL_RC_BINS:
            boolRes = sftl_rc_bins(structBuffer, structSize);
            break;
            
        case AND_STRUCT_FTL_RECOMMEND_CONTENT_DELETION:
            value = 0;
            boolRes = WMR_FILL_STRUCT(structBuffer, structSize, &value, sizeof(value));
            break;

        case AND_STRUCT_FTL_IDEAL_SIZE_1K:
            value = s_calc_lbas(TRUE32) * (s_g_bytes_per_lba / 1024);
            boolRes = WMR_FILL_STRUCT(structBuffer, structSize, &value, sizeof(value));
            break;
            
        case AND_STRUCT_FTL_GET_FTL_BLOCK_COUNT:
        {
            UInt32 tlBlockCount = s_g_max_sb;
            boolRes = WMR_FILL_STRUCT(structBuffer, structSize, &tlBlockCount, sizeof(tlBlockCount));
            break;
        }
            
        case AND_STRUCT_FTL_GET_FTL_BLOCK_STAT:
        {
            UInt32 i, tmpBuffSize = *structSize, tmpChunkSize;
            ANDFTLBlockStruct tempStruct;
            UInt8 * tmpPtr = (UInt8 *)structBuffer;
            
            if(tmpBuffSize >= (s_g_max_sb * sizeof(tempStruct)))
            {
                for(i = 0; (i < s_g_max_sb)  && (tmpBuffSize >= sizeof(tempStruct)); i++, tmpBuffSize -= sizeof(tempStruct))
                {
                    fillUpTLBlockStructure(i, &tempStruct);
                    tmpChunkSize = tmpBuffSize;
                    boolRes = WMR_FILL_STRUCT(tmpPtr + (i * sizeof(ANDFTLBlockStruct)), &tmpChunkSize, &tempStruct, sizeof(tempStruct));
                }
            }
            break;
        }

        case AND_STRUCT_FTL_L2V_STRUCT:
            boolRes = WMR_FILL_STRUCT(structBuffer, structSize, &L2V, sizeof(L2V));
            break;

        case AND_STRUCT_FTL_L2V_ROOTS:
            boolRes = WMR_FILL_STRUCT(structBuffer, structSize, L2V.Root, L2V.numRoots*sizeof(*L2V.Root));
            break;

        case AND_STRUCT_FTL_L2V_POOL:
            boolRes = WMR_FILL_STRUCT(structBuffer, structSize, L2V.Pool.Node, L2V_nodepool_mem);
            break;

        default:
            boolRes = FALSE32;
            break;
    }

    return boolRes;
}

static FTLFunctions _sftl_functions = 
{
    .Init = sftl_init,
    .Open = sftl_boot,

    .Read = sftl_read,
    .ReadSpans = sftl_read_spans,
    .Close = sftl_close,
    .GetStruct = sftl_getStruct,
    .GetMinorVersion = _getMinorVersion,

#ifndef AND_READONLY
    .ConvertUserMBtoFTLSuperblocks = sftl_ConvertUserMBtoFTLSuperblocks,
    .Write = sftl_write,
    .Unmap = sftl_unmap,
    .Format = sftl_format,
    .WearLevel = sftl_wearlevel,
    .IdleGC = sftl_IdleGCStripe,
    .GarbageCollect = sftl_garbagecollect,
    .ShutdownNotify = sftl_shutdown_notify,
    .Drain = sftl_drain,
#endif
};

FTLFunctions *SFTL_PPN_Register(void)
{
    return &_sftl_functions;
}

