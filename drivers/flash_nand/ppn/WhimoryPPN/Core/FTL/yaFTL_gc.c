/*
 * Copyright (c) 2009 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */

#define AND_TRACE_LAYER FTL

#include "yaFTL_whoami.h"
#include "yaFTL_gc.h"
#include "WMRConfig.h"
#include "ANDTypes.h"
#include "WMROAM.h"
#include "VFLBuffer.h"
#include "VFL.h"
#include "FTL.h"
#include "yaFTLTypes.h"
#include "yaFTL_Defines.h"
#include "yaFTL_BTOC.h"
#include "L2V/L2V_Extern.h"
#include "WMRFeatures.h"
#ifndef ENABLE_L2V_TREE
#error ENABLE_L2V_TREE must be set to 0 or 1 in WMRFeatures.h
#endif

#ifndef AND_READONLY

// Prototypes
ANDStatus YAFTL_GC_Index(UInt32 block, BOOL32 scrubOnUECC);
ANDStatus YAFTL_GC_Data(UInt32 block, BOOL32 scrubOnUECC);

static ANDStatus GCMachine_Data(Int32 writeSize, BOOL32 scrubOnUECC);
static void      ChooseIndexSB(void);
static void      ChooseDataSB(void);
static ANDStatus EraseIndex(UInt32 block);
static ANDStatus EraseData(UInt32 block);
static ANDStatus GetBlockTOC(GC_Ctx_t *c, BOOL32 srcubOnUECC, UInt32 upperBound, UInt32 *ueccCounter);
static void      SanityCheckValid(GC_Ctx_t *c);
static ANDStatus EvictIndex(GC_Ctx_t *c, BOOL32 scrubOnUECC);
static ANDStatus EvictData(UInt32 minAmount, BOOL32 scrubOnUECC);
static ANDStatus ReadZone(GC_Ctx_t *c, BOOL32 index, BOOL32 scrubOnUECC);
static ANDStatus WriteZoneData(GC_Ctx_t *c, BOOL32 scrubOnUECC);
static ANDStatus WriteZoneIndex(GC_Ctx_t *c, BOOL32 scrubOnUECC);

// Externs, from yAFTL.c: to be cleaned up into a common header/source
#ifdef AND_COLLECT_STATISTICS
extern FTLStatistics stFTLStatistics;
#endif
extern yaFTL_t yaFTL;
extern VFLFunctions yaFTL_VFLFunctions;
extern FTLWMRDeviceInfo yaFTL_FTLDeviceInfo;
extern UInt16 findFreeCacheEntry(void);
extern UInt16 clearEntryInCache(UInt8 flag, UInt16 candidate, UInt8 flush);
extern UInt32 allocateBlock(UInt32 blockNo, UInt8 flag, UInt8 type);
extern Int32               _readPage(UInt32 vpn, UInt8 *pageData,
                                     PageMeta_t * pMeta,
                                     BOOL32 bInternalOp, BOOL32 boolCleanCheck, BOOL32 bMarkECCForScrub);
extern BOOL32              _readMultiPages(UInt32 * padwVpn,
                                           UInt16 wNumPagesToRead, UInt8 * pbaData,
                                           UInt8 * pbaSpare,
                                           BOOL32 bInternalOp, BOOL32 scrubOnUECC);
extern BOOL32              _writeMultiPages(UInt16 vbn, UInt16 pageOffset,
                                            UInt16 wNumPagesToWrite, UInt8 *pageData,
                                            PageMeta_t *mdPtr,
                                            BOOL32 bInternalOp);
extern ANDStatus writeCurrentBlockTOC(UInt8 type);
extern UInt16 createTOClookUP(UInt32 *TOCarray, UInt16 *lookUParray, UInt16 arrayLn);
extern ANDStatus invalidateCXT(void);
extern BOOL32 isBlockInEraseNowList(UInt16 blockNo);
extern BOOL32 removeBlockFromEraseNowList(UInt16 blockNo);
extern BOOL32 addBlockToEraseNowList(UInt16 blockNo);

// Defines
#define FREE_BLK_TRH_UPDATED 4 //FREE_BLK_TRH 
#define FREE_BLK_GAP         2
#define lowData(adjust)       (((yaFTL.seaState.data.freeBlocks*PAGES_PER_SUBLK)+yaFTL.gc.data.gcDataAdjust) < (Int32)(((FREE_BLK_TRH_UPDATED + FREE_BLK_GAP) * PAGES_PER_SUBLK) + (adjust)))
#define dangerousData(adjust) ((((yaFTL.seaState.data.freeBlocks*PAGES_PER_SUBLK)+yaFTL.gc.data.gcDataAdjust) < (Int32)((FREE_BLK_TRH_UPDATED * PAGES_PER_SUBLK) + (adjust))) || ((yaFTL.seaState.data.allocdBlocks + FREE_BLK_TRH) > yaFTL.dataSize))
#define dangerousIndex        ((yaFTL.seaState.index.freeBlocks < (Int32)FREE_I_BLOCK_TRS) || ((yaFTL.seaState.index.allocdBlocks + FREE_I_BLOCK_TRS) > yaFTL.indexSize))

// Debug self-consistency checks
#define SELFCONSISTENCY_CHECKS 0
#if SELFCONSISTENCY_CHECKS

//SELFCONSISTENCY_CHECKS==1
void CheckDataCounts(void)
{
    UInt32 i, sum = 0;

    for (i = 0; i < FTL_AREA_SIZE; i++)
    {
        sum += yaFTL.blockArray[i].validPagesDNo;
    }
    WMR_ASSERT(sum == yaFTL.seaState.data.validPages);
}

//SELFCONSISTENCY_CHECKS==1
static UInt32 ValidSnapshot[8192];

//SELFCONSISTENCY_CHECKS==1
void SaveValids()
{
    UInt32 i;

    for (i = 0; i < FTL_AREA_SIZE; i++)
    {
        ValidSnapshot[i] = yaFTL.blockArray[i].validPagesDNo;
    }
}

//SELFCONSISTENCY_CHECKS==1
void CheckValids()
{
    UInt32 i;

    for (i = 0; i < FTL_AREA_SIZE; i++)
    {
        WMR_ASSERT(ValidSnapshot[i] == yaFTL.blockArray[i].validPagesDNo);
    }
}

//SELFCONSISTENCY_CHECKS==1
void CheckBlockDist()
{
    UInt32 i, D, I, free;
    BOOL32 err = FALSE32;

    // Don't check during GC of power-on restore
    if (!yaFTL.seaState.distCheckable)
        return;

    D = I = free = 0;

    for (i = 0; i < FTL_AREA_SIZE; i++)
    {
        switch (yaFTL.blockArray[i].status)
        {
        case BLOCK_ALLOCATED:
        case BLOCK_CURRENT:
        case BLOCK_GC:
            D++;
            break;

        case BLOCK_I_ALLOCATED:
        case BLOCK_I_CURRENT:
            I++;
            break;

        case BLOCK_FREE:
            free++;
            break;

        case BLOCK_I_GC:
            // Should never get here, since I_GC is monolithic and we should only call this routine before or after
            // Invalidate context
            if (yaFTL.cxtValid == 1)
            {
                invalidateCXT();
                yaFTL.cxtValid = 0;
            }    
            WMR_PANIC("BLOCK_I_GC in CheckBlockDist()");
            break;
        }
    }

    if (!((D == yaFTL.seaState.data.allocdBlocks) || (D == (yaFTL.seaState.data.allocdBlocks + 1))))
    {
        WMR_PRINT(ERROR, "D err\n");
        err = TRUE32;
    }
    if (!((I == yaFTL.seaState.index.allocdBlocks) || (I == (yaFTL.seaState.index.allocdBlocks + 1))))
    {
        WMR_PRINT(ERROR, "I err\n");
        err = TRUE32;
    }
    if ((free != (UInt32)(yaFTL.seaState.data.freeBlocks + yaFTL.seaState.index.freeBlocks)) || (free != yaFTL.seaState.freeBlocks))
    {
        WMR_PRINT(ERROR, "Free err\n");
        err = TRUE32;
    }
    if (yaFTL.seaState.data.freeBlocks & 0x80000000)
    {
        WMR_PRINT(ERROR, "D underflow error\n");
        err = TRUE32;
    }
    if (yaFTL.seaState.index.freeBlocks & 0x80000000)
    {
        WMR_PRINT(ERROR, "I underflow error\n");
        err = TRUE32;
    }
    WMR_ASSERT(FALSE32 == err);
}

#else
//SELFCONSISTENCY_CHECKS==0
void CheckDataCounts(void)
{
}
void SaveValids(void)
{
}
void CheckValids(void)
{
}
void CheckBlockDist(void)
{
}
#endif

BOOL32 WorkFifo_isEmpty(GC_Fifo_t *f)
{
    return f->tail == f->head;
}

void WorkFifo_Enq(GC_Fifo_t *f, UInt32 block)
{
    UInt32 p;
    p = f->head;
    while (p != f->tail)
    {
        if (f->block[p] == block)
        {
            return;
        }

        p++;
        if (p > GCFIFO_DEPTH)
        {
            p = 0;
        }
    }

    f->block[f->tail] = block;
    f->tail++;
    if (f->tail > GCFIFO_DEPTH)
    {
        f->tail = 0;
    }
    WMR_ASSERT(f->tail != f->head);
}

UInt32 WorkFifo_Deq(GC_Fifo_t *f)
{
    UInt32 ret;

    WMR_ASSERT(f->tail != f->head);

    ret = f->block[f->head];

    f->head++;
    if (f->head > GCFIFO_DEPTH)
    {
        f->head = 0;
    }

    return ret;
}

UInt32 YAFTL_GC_Data_Deq_sb(UInt32 block)
{
    UInt32 p;
    UInt32 nRet = -1; 
    GC_Fifo_t *f = &yaFTL.gc.data.workFifo;

    p = f->head;
    
    while (p != f->tail)
    {   
        if (f->block[p] == block)
        {   
            nRet = f->block[p];
            break;
        }   
    
        p++;
        if (p > GCFIFO_DEPTH)
        {   
            p = 0;
        }   
    }   
    
    if(p == f->tail)
        return nRet;
    
    while (p != f->tail)
    {                                                                                                             
        if(p == GCFIFO_DEPTH)
        {   
            f->block[p]=f->block[0];
            p=0;
        }   
        else
        {   
            f->block[p]=f->block[p+1];
            p++;
        }   
    }   
    
    if(f->tail == 0)
        f->tail = GCFIFO_DEPTH;
    else
        f->tail--;
    
    return nRet;
}

// Functions

ANDStatus YAFTL_GC_Init()
{
#if ENABLE_L2V_TREE
    L2V_Search_Init(&yaFTL.gc.data.read_c);
#endif // ENABLE_L2V_TREE

    // GC Zone size should be a multiple of banks, up to 16 (or whatever banks is naturally).
    // This logic guarantees low-bank configs will have higher performance, while making sure
    // high-bank configs don't occupy too much memory.
    // It is similar to setting a minimum of 16, but works with non-pow-2 configs.
    yaFTL.gc.zoneSize = NUM_BANKS;
    while (yaFTL.gc.zoneSize < GCZONE_DOUBLEUPTO) {
        yaFTL.gc.zoneSize <<= 1;
    }

    yaFTL.gc.data.state = GCD_IDLE;
    yaFTL.gc.data.in_block = 0xffffffff;
    yaFTL.gc.index.in_block = 0xffffffff;

    WMR_BufZone_Init(&yaFTL.gc.BufZone);
    yaFTL.gc.data.TOCbuff     = (UInt32*)WMR_Buf_Alloc_ForDMA(&yaFTL.gc.BufZone, yaFTL.controlPageNo * BYTES_PER_PAGE);
    yaFTL.gc.data.pageBuffer  = (UInt8 *)WMR_Buf_Alloc_ForDMA(&yaFTL.gc.BufZone, BYTES_PER_PAGE);
    yaFTL.gc.index.TOCbuff    = (UInt32*)WMR_Buf_Alloc_ForDMA(&yaFTL.gc.BufZone, yaFTL.controlPageNo * BYTES_PER_PAGE);
    yaFTL.gc.index.pageBuffer = (UInt8 *)WMR_Buf_Alloc_ForDMA(&yaFTL.gc.BufZone, BYTES_PER_PAGE);
    yaFTL.gc.meta_buf         = (PageMeta_t*)WMR_Buf_Alloc(&yaFTL.gc.BufZone, sizeof(PageMeta_t));
    yaFTL.gc.data.zone        = (UInt8*)WMR_Buf_Alloc_ForDMA(&yaFTL.gc.BufZone, BYTES_PER_PAGE * yaFTL.gc.zoneSize);
    yaFTL.gc.index.zone       = (UInt8*)WMR_Buf_Alloc_ForDMA(&yaFTL.gc.BufZone, BYTES_PER_PAGE * yaFTL.gc.zoneSize);
    yaFTL.gc.data.meta  =       (PageMeta_t*)WMR_Buf_Alloc_ForDMA(&yaFTL.gc.BufZone, yaFTL.gc.zoneSize * sizeof(PageMeta_t));
    yaFTL.gc.index.meta =       (PageMeta_t*)WMR_Buf_Alloc_ForDMA(&yaFTL.gc.BufZone, yaFTL.gc.zoneSize * sizeof(PageMeta_t));

    if (!WMR_BufZone_FinishedAllocs(&yaFTL.gc.BufZone)) {
        return FTL_CRITICAL_ERROR;
    }

    WMR_BufZone_Rebase(&yaFTL.gc.BufZone, (void**)&yaFTL.gc.data.TOCbuff);
    WMR_BufZone_Rebase(&yaFTL.gc.BufZone, (void**)&yaFTL.gc.data.pageBuffer);
    WMR_BufZone_Rebase(&yaFTL.gc.BufZone, (void**)&yaFTL.gc.index.TOCbuff);
    WMR_BufZone_Rebase(&yaFTL.gc.BufZone, (void**)&yaFTL.gc.index.pageBuffer);
    WMR_BufZone_Rebase(&yaFTL.gc.BufZone, (void**)&yaFTL.gc.meta_buf);
    WMR_BufZone_Rebase(&yaFTL.gc.BufZone, (void**)&yaFTL.gc.data.zone);
    WMR_BufZone_Rebase(&yaFTL.gc.BufZone, (void**)&yaFTL.gc.index.zone);
    WMR_BufZone_Rebase(&yaFTL.gc.BufZone, (void**)&yaFTL.gc.data.meta);
    WMR_BufZone_Rebase(&yaFTL.gc.BufZone, (void**)&yaFTL.gc.index.meta);
    WMR_BufZone_FinishedRebases(&yaFTL.gc.BufZone);

    yaFTL.gc.data.vpns  = (UInt32*)      WMR_MALLOC(yaFTL.gc.zoneSize * sizeof(UInt32));
    yaFTL.gc.index.vpns = (UInt32*)      WMR_MALLOC(yaFTL.gc.zoneSize * sizeof(UInt32));
    
    if ((NULL == yaFTL.gc.data.TOCbuff)     || (NULL == yaFTL.gc.data.pageBuffer)
        || (NULL == yaFTL.gc.index.TOCbuff) || (NULL == yaFTL.gc.index.pageBuffer)
        || (NULL == yaFTL.gc.meta_buf)
        || (NULL == yaFTL.gc.data.zone)     || (NULL == yaFTL.gc.index.zone)
        || (NULL == yaFTL.gc.data.vpns)     || (NULL == yaFTL.gc.data.meta)
        || (NULL == yaFTL.gc.index.vpns)    || (NULL == yaFTL.gc.index.meta)
    )
    {
        return FTL_CRITICAL_ERROR;
    }

    return FTL_SUCCESS;
}

void YAFTL_GC_Close()
{
    WMR_BufZone_Free(&yaFTL.gc.BufZone);

    yaFTL.gc.data.TOCbuff = NULL;
    yaFTL.gc.data.pageBuffer = NULL;
    yaFTL.gc.index.TOCbuff = NULL;
    yaFTL.gc.index.pageBuffer = NULL;
    yaFTL.gc.meta_buf = NULL;
    yaFTL.gc.data.zone = NULL;
    yaFTL.gc.index.zone = NULL;
    yaFTL.gc.data.meta = NULL;
    yaFTL.gc.index.meta = NULL;

    if (NULL != yaFTL.gc.data.vpns) {
        WMR_FREE(yaFTL.gc.data.vpns, yaFTL.gc.zoneSize * sizeof(UInt32));
    }
    if (NULL != yaFTL.gc.index.vpns) {
        WMR_FREE(yaFTL.gc.index.vpns, yaFTL.gc.zoneSize * sizeof(UInt32));
    }

    yaFTL.gc.data.vpns = NULL;
    yaFTL.gc.index.vpns = NULL;
}

void YAFTL_GC_PreWrite(UInt32 writeSize)
{
    CheckDataCounts();
    CheckBlockDist();
    yaFTL.gc.data.in_block = 0xffffffff;

    // Below low low low mark?
    while (dangerousIndex || dangerousData(writeSize))
    {
        if (dangerousIndex)
        {
            YAFTL_GC_Index(0xffffffff, TRUE32);
        }
        if (dangerousData(writeSize))
        {
            YAFTL_GC_Data(0xffffffff, TRUE32);
        }
    }

    CheckBlockDist();

    // If we're low on space, or the garbage collector is not idle, (-> this is important to not cause hangs)
    if (lowData(writeSize) || (GCD_IDLE != yaFTL.gc.data.state))
    {
        GCMachine_Data(writeSize, TRUE32);
    }

    CheckBlockDist();
}

static void MoveOnIfCurrentI(UInt32 inBlock)
{
    // Is this the current index block?
    if (BLOCK_I_CURRENT == yaFTL.blockArray[inBlock].status)
    {
        // Have to allocate a new one so we don't pull the rug out from underneath ourselves...
        allocateBlock(yaFTL.wrState.index.block, 0, 0);
    }
}

ANDStatus YAFTL_GC_Index(UInt32 block, BOOL32 scrubOnUECC)
{
    GC_Ctx_t *c = &yaFTL.gc.index;
    ANDStatus status = FTL_SUCCESS;

    WMR_TRACE_IST_2(GCIndex, START, block, scrubOnUECC);

    c->in_block = block;

    stFTLStatistics.ddwIndexGCCnt++;

    CheckBlockDist();

 again:
    ChooseIndexSB();

    if (0 == c->chosenValid)
    {
        // No valid pages--just erase it and be done with it all
        stFTLStatistics.ddwEmptyGCIndexCnt++;
        goto eraseExit;
    }

    // Get block TOC, either from memory or by reading it off the NAND
    if(BTOC_GET_IPN(1) == 1)
    {
        GetBlockTOC(c, scrubOnUECC, yaFTL.TOCtableEntriesNo, NULL);
    }
    else
    {
        GetBlockTOC(c, scrubOnUECC, yaFTL.logicalPartitionSize, NULL);
    }
    
    status = EvictIndex(c, scrubOnUECC);
    if (FTL_SUCCESS != status)
    {
        goto again;
    }

 eraseExit:
    SanityCheckValid(c);

    // Zap it!
    status = EraseIndex(c->chosenBlock);
    if (FTL_SUCCESS != status)
    {
        WMR_TRACE_IST_1(GCIndex, END, status);
        return status;
    }

    if (!WorkFifo_isEmpty(&c->workFifo))
    {
        goto again;
    }

    CheckBlockDist();

    WMR_TRACE_IST_1(GCIndex, END, FTL_SUCCESS);
    return FTL_SUCCESS;
}

static void MoveOnIfCurrentD(UInt32 inBlock)
{
    // Is it the current data block?
    if (BLOCK_CURRENT == yaFTL.blockArray[inBlock].status)
    {
        allocateBlock(yaFTL.wrState.data.block, 0, 1);
    }
}


// Data state machine:
// 1) choose block
// 2) read TOC or reconstruct
// 3) [loop] find valid spans, move preferentially in chunks of sequentiality
// 4) sanity check
// 5) erase block


static ANDStatus GCMachine_Data(Int32 writeSize, BOOL32 scrubOnUECC)
{
    ANDStatus status;
    UInt32 cost;
    Int32 ratio;

 again:
    WMR_TRACE_IST_3(GCMachine, START, writeSize, scrubOnUECC, yaFTL.gc.data.state);
    switch (yaFTL.gc.data.state)
    {
    case GCD_IDLE:
        while (dangerousIndex)
        {
            // Important for wearleveling calls, for instance
            // Safe because YAFTL_GC_Index only calls GCMachine_Data if state != GCD_IDLE
            YAFTL_GC_Index(0xffffffff, scrubOnUECC);
        }

        stFTLStatistics.ddwDataGCCnt++;
        ChooseDataSB();
        if (0 == yaFTL.gc.data.chosenValid)
        {
            stFTLStatistics.ddwEmptyGCDataCnt++;
            yaFTL.gc.data.state = GCD_ERASE;
            WMR_TRACE_IST_0(GCMachine, END);
            goto again;
        }

        yaFTL.gc.data.curPage = 0;
        yaFTL.gc.data.pagesCopied = 0;
        yaFTL.gc.data.ueccCounter = 0;    
        GetBlockTOC(&yaFTL.gc.data, scrubOnUECC, yaFTL.logicalPartitionSize, &yaFTL.gc.data.ueccCounter);
        yaFTL.blockArray[yaFTL.gc.data.chosenBlock].status = BLOCK_GC;
        yaFTL.gc.data.state = GCD_EVICT;
        break;

    case GCD_EVICT:
        // Check if it's been invalidated by host writes since we entered the state machine
        WMR_ASSERT(0 == yaFTL.blockArray[yaFTL.gc.data.chosenBlock].validPagesINo);
        if (0 == yaFTL.blockArray[yaFTL.gc.data.chosenBlock].validPagesDNo)
        {
            // And short-circuit to the ERASE state
            yaFTL.gc.data.state = GCD_ERASE;
            WMR_TRACE_IST_0(GCMachine, END);
            goto again;
        }

        cost = yaFTL.gc.zoneSize;
        if (writeSize > 0)
        {
            ratio = (FREE_BLK_TRH_UPDATED * 2 * PAGES_PER_SUBLK) - ((yaFTL.seaState.data.freeBlocks*PAGES_PER_SUBLK)+yaFTL.gc.data.gcDataAdjust);
            if (ratio <= 0) ratio = 1;
            ratio = (ratio * YAFTL_GC_RATIO_SCALAR) / FREE_BLK_GAP;
            // Writes should evict an amount relative to the current ratio of low-end validity
            cost = PAGES_PER_SUBLK - yaFTL.controlPageNo - yaFTL.gc.data.chosenValid; // Divisor
            if (!cost)
            {
                cost = 1; // Avoid div0 when we move a full block for wear-leveling purposes etc.
            }
            cost = (writeSize * PAGES_PER_SUBLK) / cost;
            cost = (cost * ratio) / PAGES_PER_SUBLK;
            // Round up to a zone boundary (makes no sense to do less!)
            cost += yaFTL.gc.zoneSize - 1;
            cost -= cost % yaFTL.gc.zoneSize;
        }

        status = EvictData(cost, scrubOnUECC);
        if (FTL_SUCCESS != status)
        {
            if (GCD_EVICT == yaFTL.gc.data.state)
            {
                yaFTL.gc.data.state = GCD_IDLE;
            }
            WMR_TRACE_IST_0(GCMachine, END);
            goto again;
        }
        if (yaFTL.gc.data.curPage >= yaFTL.gc.data.pagesRange)
        {
            SanityCheckValid(&yaFTL.gc.data);
        }
        break;

    case GCD_ERASE:
        SanityCheckValid(&yaFTL.gc.data);
        status = EraseData(yaFTL.gc.data.chosenBlock);
        yaFTL.gc.data.state = GCD_IDLE;
        if (FTL_SUCCESS != status)
        {
            WMR_TRACE_IST_1(GCMachine, END, status);
            return status;
        }
        if (!WorkFifo_isEmpty(&yaFTL.gc.data.workFifo))
        {
            // If we had an error causing us to do extra data GCs,
            // don't stop in the idle state.
            WMR_TRACE_IST_0(GCMachine, END);
            goto again;
        }
        break;

    case GCD_FIXINDEX_RETRY:
        while (!WorkFifo_isEmpty(&yaFTL.gc.index.workFifo))
        {
            YAFTL_GC_Index(0xffffffff, scrubOnUECC);
        }
        yaFTL.gc.data.state = GCD_IDLE;
        WMR_TRACE_IST_0(GCMachine, END);
        goto again;

    case GCD_FIXDATA_RETRY:
        yaFTL.gc.data.state = GCD_IDLE;
        WMR_TRACE_IST_0(GCMachine, END);
        goto again;
    }

    WMR_TRACE_IST_1(GCMachine, END, FTL_SUCCESS);
    return FTL_SUCCESS;
}

void YAFTL_GC_Data_Enq(UInt32 block)
{
    WMR_TRACE_IST_1(GCDataEnq, NONE, block);
    WorkFifo_Enq(&yaFTL.gc.data.workFifo, block);
}

void YAFTL_GC_Index_Enq(UInt32 block)
{
    WMR_TRACE_IST_1(GCIndexEnq, NONE, block);
    WorkFifo_Enq(&yaFTL.gc.index.workFifo, block);
}

ANDStatus YAFTL_GC_Data(UInt32 block, BOOL32 scrubOnUECC)
{
    ANDStatus status;

    WMR_TRACE_IST_2(GCData, START, block, scrubOnUECC);

    CheckBlockDist();

    // Kill any previous GC operations
    // This makes GC recovery from write fail more safe
    if (GCD_IDLE != yaFTL.gc.data.state) {
        // Un-mark pending GC operations
        if ((0xffffffff != yaFTL.gc.data.chosenBlock) && (BLOCK_GC == yaFTL.blockArray[yaFTL.gc.data.chosenBlock].status)) {
            yaFTL.blockArray[yaFTL.gc.data.chosenBlock].status = BLOCK_ALLOCATED;
        }
    }
    yaFTL.gc.data.state = GCD_IDLE;

again:
    // Set up the new one
    yaFTL.gc.data.in_block = block;

    // And start it
    do
    {
        status = GCMachine_Data(0, scrubOnUECC);
        if (FTL_SUCCESS != status)
        {
            WMR_TRACE_IST_1(GCData, END, status);
            return status;
        }
    }
    while (GCD_IDLE != yaFTL.gc.data.state);

    if (!WorkFifo_isEmpty(&yaFTL.gc.data.workFifo))
    {
        block = WorkFifo_Deq(&yaFTL.gc.data.workFifo);
        goto again;
    }

    CheckBlockDist();

    WMR_TRACE_IST_1(GCData, END, FTL_SUCCESS);
    return FTL_SUCCESS;
}

static void ChooseSB(GC_Ctx_t *c, UInt32 filter)
{
    UInt32 i, status;
    UInt32 thisValid, thisErases;
    UInt32 best, bestValid, bestErases;

    best = bestValid = bestErases = 0xFFFFFFFF;

    if (0xffffffff != c->in_block) {
        MoveOnIfCurrentI(c->in_block);
        MoveOnIfCurrentD(c->in_block);
    }

    if (!WorkFifo_isEmpty(&c->workFifo))
    {
        if (0xffffffff != c->in_block)
        {
            WorkFifo_Enq(&c->workFifo, c->in_block);
        }
        c->in_block = WorkFifo_Deq(&c->workFifo);
    }

    if (0xffffffff != c->in_block)
    {
        c->chosenBlock = c->in_block;
        c->chosenValid = yaFTL.blockArray[c->in_block].validPagesDNo + yaFTL.blockArray[c->in_block].validPagesINo;
        c->chosenErases = yaFTL.blockArray[c->in_block].erasableCount;
        status = yaFTL.blockArray[c->in_block].status;
        if (status != filter)
        {
            // Note: these rules use implication, that works in conjunction with the wrapper if statement so
            // BLOCK_I_ALLOCATED filter means BLOCK_I_CURRENT/GC will work, and the same for BLOCK_ALLOCATED and BLOCK_CURRENT/GC.
            WMR_ASSERT((filter != BLOCK_ALLOCATED) || ((status == BLOCK_CURRENT) || (status == BLOCK_GC)));
            WMR_ASSERT((filter != BLOCK_I_ALLOCATED) || ((status == BLOCK_I_CURRENT) || (status == BLOCK_I_GC)));
        }
        c->in_block = 0xffffffff;
        return;
    }

    for (i = 0; i < FTL_AREA_SIZE; i++)
    {
        // Not allocated to the filter type?  Don't consider...
        if (yaFTL.blockArray[i].status != filter)
        {
            continue;
        }

        // Grab local copies
        thisValid = yaFTL.blockArray[i].validPagesDNo + yaFTL.blockArray[i].validPagesINo;
        thisErases = yaFTL.blockArray[i].erasableCount;

        // Minimize valid
        if ((thisValid < bestValid) || ((thisValid == bestValid) && (thisErases < bestErases)))
        {
            best = i;
            bestValid = thisValid;
            bestErases = thisErases;
        }
    }

    WMR_ASSERT(best != 0xFFFFFFFF);
    WMR_ASSERT(bestValid != 0xFFFFFFFF);
    WMR_ASSERT(bestErases != 0xFFFFFFFF);

    // Output
    c->chosenBlock = best;
    c->chosenValid = bestValid;
    c->chosenErases = bestErases;
}

static void ChooseIndexSB()
{
    ChooseSB(&yaFTL.gc.index, BLOCK_I_ALLOCATED);
}

static void ChooseDataSB()
{
    ChooseSB(&yaFTL.gc.data, BLOCK_ALLOCATED);
}

static ANDStatus EraseIndex(UInt32 block)
{
    yaFTL.blockArray[block].eraseBeforeUse = BLOCK_TO_BE_ERASED_ALIGNED_INDEX;
    yaFTL.blockArray[block].pagesRead = 0;
    yaFTL.blockArray[block].pagesReadSubCounter = 0;
    yaFTL.blockArray[block].validPagesDNo = yaFTL.blockArray[block].validPagesINo = 0;
    yaFTL.seaState.freeBlocks++;
    yaFTL.blockArray[block].status = BLOCK_I_FREE;
    yaFTL.seaState.index.freeBlocks++;
    yaFTL.seaState.index.allocdBlocks--;
#ifdef AND_COLLECT_STATISTICS
    stFTLStatistics.ddwFreeBlks++;
#endif    
    return FTL_SUCCESS;
}

static ANDStatus EraseData(UInt32 block)
{
    yaFTL.blockArray[block].eraseBeforeUse = BLOCK_TO_BE_ERASED_ALIGNED_DATA;
    yaFTL.blockArray[block].pagesRead = 0;
    yaFTL.blockArray[block].pagesReadSubCounter = 0;
    yaFTL.blockArray[block].validPagesDNo = yaFTL.blockArray[block].validPagesINo = 0;
    yaFTL.seaState.freeBlocks++;
    yaFTL.blockArray[block].status = BLOCK_FREE;
    yaFTL.seaState.data.freeBlocks++;
    yaFTL.seaState.data.allocdBlocks--;
#ifdef AND_COLLECT_STATISTICS
    stFTLStatistics.ddwFreeBlks++;
#endif      
    return FTL_SUCCESS;
}

static ANDStatus GetBlockTOC(GC_Ctx_t *c, BOOL32 scrubOnUECC, UInt32 upperBound, UInt32 *ueccCounter)
{
    UInt32 i;
    UInt32 *ramBTOC;
    PageMeta_t *meta = yaFTL.gc.meta_buf;

    ANDStatus status;

    c->pagesRange = PAGES_PER_SUBLK - yaFTL.controlPageNo;

    // Is it in memory?
    ramBTOC = BTOC_Search(c->chosenBlock, c == &yaFTL.gc.data);
    if (NULL != ramBTOC) {
        BTOC_Copy(c->TOCbuff, ramBTOC, upperBound);
        return FTL_SUCCESS;
    }

    // Have to read it off the NAND...
    status = BTOC_Read((c->chosenBlock * PAGES_PER_SUBLK) + (PAGES_PER_SUBLK - yaFTL.controlPageNo), c->TOCbuff, meta, FALSE32, scrubOnUECC, upperBound);
    if ((status != FTL_SUCCESS) || ((status == FTL_SUCCESS) && !META_IS_BTOC(meta)))
    {
        BOOL32 uncleanFound = FALSE32;

        debug(MISC, "cannot read blockTOC - restoring blockTOC manually");
        BTOC_SetAll(c->TOCbuff, 0xff);
        i = (UInt32)(PAGES_PER_SUBLK - yaFTL.controlPageNo);
        while (i-- > 0)
        {
            status = _readPage((c->chosenBlock * PAGES_PER_SUBLK) + i, c->pageBuffer, meta, FALSE32, TRUE32, scrubOnUECC);
            if (VFL_SUCCESS_CLEAN != status)
            {
                uncleanFound = TRUE32;
            }
            if ((status == FTL_SUCCESS) && (META_GET_IPNRAW(meta) < upperBound))
            {
                c->TOCbuff[i] = META_GET_IPNRAW(meta);
            }
            else
            {
                if ((FTL_CRITICAL_ERROR == status) || ((VFL_SUCCESS_CLEAN == status) && uncleanFound))
                {
                    if (NULL != ueccCounter)
                    {
                        debug(MISC, "got error at block %d page %d, assuming uecc\n", c->chosenBlock, i);
                        (*ueccCounter)++;
                    }
                }
            }
        }
    }

    return FTL_SUCCESS;
}

static void SanityCheckValid(GC_Ctx_t *c)
{
    // Sanity check: should now be empty...
    if ((yaFTL.blockArray[c->chosenBlock].validPagesINo + yaFTL.blockArray[c->chosenBlock].validPagesDNo) != 0)
    {
        CheckDataCounts();
        debug(ERROR, "something could be wrong with GC; index:%d data:%d ", yaFTL.blockArray[c->chosenBlock].validPagesINo, yaFTL.blockArray[c->chosenBlock].validPagesDNo);
        if (c->ueccCounter >= (yaFTL.blockArray[c->chosenBlock].validPagesINo + yaFTL.blockArray[c->chosenBlock].validPagesDNo))
        {
            // If we had uncorrectables, we may be missing data... best way to mark it is to do a P->L translation, but that
            // would be a very slow, exhaustive searchof the index tables--though we should consider implementing it at some point.
            debug(ERROR, "fixing up validity counters since we had %d uECC(s) ", c->ueccCounter);
            yaFTL.blockArray[c->chosenBlock].validPagesINo = 0;
            yaFTL.blockArray[c->chosenBlock].validPagesDNo = 0;
        }
        else
        {
            // It must be a real bug since we had no uncorrectables to blame
            // Invalidate context
            if (yaFTL.cxtValid == 1)
            {
                invalidateCXT();
                yaFTL.cxtValid = 0;
            }
            WMR_PANIC("Non-zero validity counter Block %d Index: %d Data: %d UECC: %d", 
                      c->chosenBlock, 
                      (UInt32) yaFTL.blockArray[c->chosenBlock].validPagesINo, 
                      (UInt32) yaFTL.blockArray[c->chosenBlock].validPagesDNo, 
                      c->ueccCounter);
        }
    }
}

typedef enum
{
    IPS_DEAD,
    IPS_DIRTYCACHE,
    IPS_BUSYCACHE,
    IPS_ALIVE,
    IPS_ERROR,
} IPageState_t;

static IPageState_t GetIndexPageState(UInt32 page, UInt32 *r_presentIdxPage, UInt32 *r_indexPagePhy, UInt32 *r_cacheIndexPtr)
{
    GC_Ctx_t *c = &yaFTL.gc.index;
    UInt32 presentIdxPage, indexPagePhy, cacheIndexPtr;

    // Extract which IdxPage lives at this location
    presentIdxPage = BTOC_GET_IPN(c->TOCbuff[page]);
    *r_presentIdxPage = presentIdxPage;

    if (0xffffffff == c->TOCbuff[page])
    {
        // Nothing to move
        return IPS_DEAD;
    }

    // Calculate which index page, its location (if any) in the cache
    indexPagePhy = yaFTL.tocArray[presentIdxPage].indexPage;
    cacheIndexPtr = yaFTL.tocArray[presentIdxPage].cacheIndex;
    *r_indexPagePhy = indexPagePhy;
    *r_cacheIndexPtr = cacheIndexPtr;

    if ((0xffffffff == indexPagePhy) && (0xffff == cacheIndexPtr))
    {
        // Not on medium or in cache... (span deallocated)
        return IPS_DEAD;
    }

    if ((0xffffffff != indexPagePhy) && (indexPagePhy != ((c->chosenBlock * PAGES_PER_SUBLK) + page)))
    {
        // Early out for wrong location...
        return IPS_DEAD;
    }

    if ((cacheIndexPtr != 0xffff) && (yaFTL.indexCache[cacheIndexPtr].status == IC_DIRTY_PAGE))
    {
        // In cache, and dirty, so it will make it down eventually...
        return IPS_DIRTYCACHE;
    }

    if ((cacheIndexPtr != 0xffff) && (yaFTL.indexCache[cacheIndexPtr].status == IC_BUSY_PAGE))
    {
        // It's in the cache as busy; special logic needed by caller...
        return IPS_BUSYCACHE;
    }

    if (indexPagePhy != 0xffffffff)
    {
        if (((c->chosenBlock * PAGES_PER_SUBLK) + page) == indexPagePhy)
        {
            // We've found a live one!
            return IPS_ALIVE;
        }
        else
        {
            // On medium, but not in that location, so don't move it
            // Should be unreachable
            WMR_ASSERT(0);
            return IPS_DEAD;
        }
    }

    // Should never get here, as the above logic should cover all cases.
    WMR_ASSERT(0);
    return IPS_ERROR;
}

static ANDStatus EvictIndex(GC_Ctx_t *c, BOOL32 scrubOnUECC)
{
    UInt32 page, alignZoneSize;
    UInt32 presentIdxPage, indexPagePhy, cacheIndexPtr;
    UInt16 newCacheEntry;
    ANDStatus status;

    status = FTL_SUCCESS;
    c->pagesCopied = 0;
    c->curZoneSize = 0;
    c->ueccCounter = 0;

    yaFTL.blockArray[c->chosenBlock].status = BLOCK_I_GC;

    // Iterate over the range of potentially valid virtual pages
    for (page = 0; (page < c->pagesRange) && (c->pagesCopied < c->chosenValid); page++)
    {
        switch (GetIndexPageState(page, &presentIdxPage, &indexPagePhy, &cacheIndexPtr))
        {
        case IPS_DEAD:
        case IPS_DIRTYCACHE:
            // Nothing to move.
            break;

        case IPS_BUSYCACHE:
            yaFTL.indexCache[cacheIndexPtr].status = IC_DIRTY_PAGE;     // So it gets written to the flash later.
            if (((c->chosenBlock * PAGES_PER_SUBLK) + page) == indexPagePhy)
            {
                // Pre-check validity
                WMR_ASSERT(0 != yaFTL.blockArray[c->chosenBlock].validPagesINo);

                // Manage validity and page copy count
                yaFTL.blockArray[c->chosenBlock].validPagesINo--;
                yaFTL.seaState.index.validPages--;
#ifdef AND_COLLECT_STATISTICS
                stFTLStatistics.ddwValidIndexPages--;
#endif
                yaFTL.tocArray[presentIdxPage].indexPage = 0xffffffff;

                c->pagesCopied++;
            }
            break;

        case IPS_ALIVE:
            newCacheEntry = findFreeCacheEntry();
            if (newCacheEntry != 0xffff)
            {
                // Read it in to the cache if we can
                status = _readPage((c->chosenBlock * PAGES_PER_SUBLK) + page, (UInt8 *)(yaFTL.indexCache[newCacheEntry].indexPage), yaFTL.singleMdPtr, FALSE32, TRUE32, scrubOnUECC);
                if (status != FTL_SUCCESS)
                {
                    return status;
                }
                yaFTL.indexCache[newCacheEntry].status = IC_DIRTY_PAGE;
                yaFTL.indexCache[newCacheEntry].tocEntry = presentIdxPage;
                yaFTL.tocArray[presentIdxPage].cacheIndex = newCacheEntry;
                yaFTL.tocArray[presentIdxPage].indexPage = 0xffffffff;

                // Pre-check validity
                WMR_ASSERT(0 != yaFTL.blockArray[c->chosenBlock].validPagesINo);

                // Manage validity and page copy count
                yaFTL.blockArray[c->chosenBlock].validPagesINo--;
                yaFTL.seaState.index.validPages--;
#ifdef AND_COLLECT_STATISTICS
                stFTLStatistics.ddwValidIndexPages--;
#endif
                c->pagesCopied++;
            }
            else
            {
                // No free cache pages... copy it over

                // Pre-check validity
                WMR_ASSERT(yaFTL.blockArray[c->chosenBlock].validPagesINo >= c->curZoneSize);

                // Zone overflow?
                alignZoneSize = yaFTL.gc.zoneSize - (yaFTL.wrState.index.nextPage % NUM_BANKS);
                if (c->curZoneSize >= alignZoneSize)
                {
                    status = ReadZone(c, TRUE32, scrubOnUECC);
                    if (status != FTL_SUCCESS)
                    {
                        return status;
                    }
                    status = WriteZoneIndex(c, scrubOnUECC);
                    if (status != FTL_SUCCESS)
                    {
                        return status;
                    }

                    c->curZoneSize = 0;
                }

                // Place in zone
                yaFTL.gc.index.vpns[c->curZoneSize] = (c->chosenBlock * PAGES_PER_SUBLK) + page;
                c->curZoneSize++;
            }
            break;

        default:
            WMR_ASSERT(0);
            break;
        }
    }

    // Do remainder
    if (c->curZoneSize > 0)
    {
        status = ReadZone(c, TRUE32, scrubOnUECC);
        if (status != FTL_SUCCESS)
        {
            return status;
        }
        status = WriteZoneIndex(c, scrubOnUECC);
        if (status != FTL_SUCCESS)
        {
            return status;
        }
    }

    return status;
}

static BOOL32 IsDataPageCurrent(UInt32 page, UInt32 *vPN, ANDStatus *status, BOOL32 scrubOnUECC)
{
    GC_Ctx_t *c = &yaFTL.gc.data;
    UInt32 presentVpn, indexPageNo, indexPageSub, indexPagePhy, cacheIndexPtr;
    PageMeta_t *tMD = yaFTL.gc.meta_buf;

    // Extract which VPN lives at this location
    presentVpn = c->TOCbuff[page];

    if (0xffffffff == presentVpn)
    {
        // Nothing to move...
        return FALSE32;
    }

#if ENABLE_L2V_TREE
    // Out of search span, or switched to a different region?
    if ((0 == yaFTL.gc.data.read_c.span) || (yaFTL.gc.data.read_c.lba != presentVpn)) {
        yaFTL.gc.data.read_c.lba = presentVpn;
        L2V_Search(&yaFTL.gc.data.read_c);
    }
    WMR_ASSERT(0 != yaFTL.gc.data.read_c.span);

    // Move search iterator along--NOTE: can't use these values below; if we need to, move these to exit paths
    yaFTL.gc.data.read_c.lba++;
    yaFTL.gc.data.read_c.span--;

    if (L2V_VPN_DEALLOC == yaFTL.gc.data.read_c.vpn) {
        // Deallocated
        return FALSE32;
    }

    if (yaFTL.gc.data.read_c.vpn < L2V_VPN_SPECIAL) {
        *vPN = yaFTL.gc.data.read_c.vpn;
        yaFTL.gc.data.read_c.vpn++; // Only because not in special range
        return *vPN == ((c->chosenBlock * PAGES_PER_SUBLK) + page);
    }

    // Missed first search... :(
    WMR_ASSERT(L2V_VPN_MISS == yaFTL.gc.data.read_c.vpn);

    if (yaFTL.gc.data.read_c.vpn < L2V_VPN_SPECIAL) {
        yaFTL.gc.data.read_c.vpn++;
    }
#endif // ENABLE_L2V_TREE

    // Calculate which index page, how far in, where the index page lives, and its location in the cache
    indexPageNo = presentVpn / yaFTL.indexPageRatio;
    WMR_ASSERT(indexPageNo < yaFTL.TOCtableEntriesNo);
    indexPageSub = presentVpn % yaFTL.indexPageRatio;
    indexPagePhy = yaFTL.tocArray[indexPageNo].indexPage;
    cacheIndexPtr = yaFTL.tocArray[indexPageNo].cacheIndex;

    if ((0xffffffff == indexPagePhy) && (0xffff == cacheIndexPtr))
    {
        // Not on medium or in cache...
        return FALSE32;
    }
    
    if (0xffff == cacheIndexPtr)
    {
        // Need to read index page in to cache first
        cacheIndexPtr = findFreeCacheEntry();
        if (cacheIndexPtr == 0xffff)
        {
            /* Cache is full and we need to clear space for new index page */
            cacheIndexPtr = clearEntryInCache(0, 0xffff, 0);
            if (cacheIndexPtr == 0xffff)
            {
                *status = FTL_CRITICAL_ERROR;
                yaFTL.gc.data.state = GCD_FIXINDEX_RETRY;

                return FALSE32;
            }
        }

        *status = _readPage(indexPagePhy, (UInt8 *)(yaFTL.indexCache[cacheIndexPtr].indexPage), tMD, FALSE32, TRUE32, scrubOnUECC);
        if (*status != FTL_SUCCESS)
        {
            if (yaFTL.cxtValid == 1)
            {
                invalidateCXT();
                yaFTL.cxtValid = 0;
            }  

            WMR_PANIC("Index UECC Page 0x%08x Status 0x%08x", indexPagePhy, *status);
            return FALSE32;
        }

        if (!META_IS_IDX(tMD))
        {
            debug(ERROR, "meta data expected to be from an index page");
            if (yaFTL.cxtValid == 1)
            {
                invalidateCXT();
                yaFTL.cxtValid = 0;
            }  

            WMR_PANIC("Invalid Index metadata 0x%02x", (UInt32) META_GET_ALL_FLAGS(tMD));
            *status = FTL_CRITICAL_ERROR;
            return FALSE32;
        }
        yaFTL.freeCachePages--;
        yaFTL.indexCache[cacheIndexPtr].status = IC_BUSY_PAGE;
        yaFTL.indexCache[cacheIndexPtr].counter = 1;
        yaFTL.tocArray[indexPageNo].cacheIndex = cacheIndexPtr;
        yaFTL.indexCache[cacheIndexPtr].tocEntry = indexPageNo;
    }

    *vPN = yaFTL.indexCache[cacheIndexPtr].indexPage[indexPageSub];
    return *vPN == ((c->chosenBlock * PAGES_PER_SUBLK) + page);
}

static ANDStatus EvictData(UInt32 minAmount, BOOL32 scrubOnUECC)
{
    UInt32 vPN, alignZoneSize;
    ANDStatus status;
    UInt32 moved = 0;

    status = FTL_SUCCESS;

    yaFTL.gc.data.curZoneSize = 0;

    // Clear search state every time we enter EvictData, for coherency
#if ENABLE_L2V_TREE
    L2V_Search_Init(&yaFTL.gc.data.read_c);
#endif // ENABLE_L2V_TREE

    for (; (moved < minAmount) && (yaFTL.gc.data.curPage < yaFTL.gc.data.pagesRange) && (yaFTL.gc.data.pagesCopied < yaFTL.gc.data.chosenValid); yaFTL.gc.data.curPage++)
    {
        if (IsDataPageCurrent(yaFTL.gc.data.curPage, &vPN, &status, scrubOnUECC))
        {
            yaFTL.gc.data.vpns[yaFTL.gc.data.curZoneSize] = vPN;
            yaFTL.gc.data.curZoneSize++;
            moved++;
            yaFTL.gc.data.pagesCopied++;
        }
        if (FTL_SUCCESS != status)
        {
            return status;
        }

        alignZoneSize = yaFTL.gc.zoneSize - (yaFTL.wrState.data.nextPage % NUM_BANKS);
        if (yaFTL.gc.data.curZoneSize >= alignZoneSize)
        {
            WMR_ASSERT(yaFTL.blockArray[yaFTL.gc.data.chosenBlock].validPagesDNo >= yaFTL.gc.data.curZoneSize);

            status = ReadZone(&yaFTL.gc.data, FALSE32, scrubOnUECC);
            if (status != FTL_SUCCESS)
            {
                return status;
            }
            status = WriteZoneData(&yaFTL.gc.data, scrubOnUECC);
            if (status != FTL_SUCCESS)
            {
                yaFTL.gc.data.state = GCD_FIXDATA_RETRY;
                return status;
            }
            yaFTL.gc.data.curZoneSize = 0;
        }
    }

    if (yaFTL.gc.data.curZoneSize > 0)
    {
        WMR_ASSERT(yaFTL.blockArray[yaFTL.gc.data.chosenBlock].validPagesDNo >= yaFTL.gc.data.curZoneSize);

        status = ReadZone(&yaFTL.gc.data, FALSE32, scrubOnUECC);
        if (status != FTL_SUCCESS)
        {
            return status;
        }
        status = WriteZoneData(&yaFTL.gc.data, scrubOnUECC);
        if (status != FTL_SUCCESS)
        {
            yaFTL.gc.data.state = GCD_FIXDATA_RETRY;
            return status;
        }
    }

    return status;
}

static BOOL32 ReconstructIndex(UInt32 indexEntry, UInt32 *indexPage)
{
#if ENABLE_L2V_TREE
    UInt32 lpnOfs, lpn, vpn, add, amount;

    lpn = indexEntry * yaFTL.indexPageRatio;
    yaFTL.gc.index.read_c.span = 0;

    lpnOfs = 0;
    while (lpnOfs < yaFTL.indexPageRatio) {
        if (0 == yaFTL.gc.index.read_c.span) {
            yaFTL.gc.index.read_c.lba = lpn + lpnOfs;
            L2V_Search(&yaFTL.gc.index.read_c);
        }
        WMR_ASSERT(0 != yaFTL.gc.index.read_c.span);

        // Missing?  Bail out
        if (L2V_VPN_MISS == yaFTL.gc.index.read_c.vpn) {
            return FALSE32;
        }

        // Not missing from IC tree
        if (L2V_VPN_DEALLOC != yaFTL.gc.index.read_c.vpn) {
            // Not deallocated
            vpn = yaFTL.gc.index.read_c.vpn;
            WMR_ASSERT(yaFTL.gc.index.read_c.vpn < L2V_VPN_SPECIAL);

            // Complete the span
            amount = WMR_MIN(yaFTL.indexPageRatio-lpnOfs, yaFTL.gc.index.read_c.span);
            add = amount;
            while (amount--) {
                indexPage[lpnOfs++] = vpn++;
                
            }

            yaFTL.gc.index.read_c.vpn += add;
            yaFTL.gc.index.read_c.span -= add;
        } else {
            // Deallocated
            vpn = 0xffffffff;

            // Complete the span
            amount = WMR_MIN(yaFTL.indexPageRatio-lpnOfs, yaFTL.gc.index.read_c.span);
            add = amount;
            while (amount--) {
                indexPage[lpnOfs++] = vpn;
            }

            yaFTL.gc.index.read_c.span -= add;
        }

    }

    return TRUE32;
#else
    return FALSE32;
#endif
}

static void ScrubZone(GC_Ctx_t *c, BOOL32 index)
{
    UInt32 i, ipn, lba;

    if (index) {
        for (i = 0; i < c->curZoneSize; i++)
        {
            ipn = META_GET_IPN(&c->meta[i]);
            if (ipn >= yaFTL.TOCtableEntriesNo)
            {
                META_SET_IPN_FROM_BTOC(&c->meta[i], c->TOCbuff[c->vpns[i] % PAGES_PER_SUBLK]);
                SetupMeta_Data_UECC(&c->meta[i]);
            }
        }
    } else {
        for (i = 0; i < c->curZoneSize; i++)
        {
            lba = META_GET_LBA(&c->meta[i]);
            if (lba >= yaFTL.logicalPartitionSize)
            {
                META_SET_LBA(&c->meta[i], c->TOCbuff[c->vpns[i] % PAGES_PER_SUBLK]);
                SetupMeta_Data_UECC(&c->meta[i]);
            }
        }
    }
}

static ANDStatus ReadZone(GC_Ctx_t *c, BOOL32 index, BOOL32 scrubOnUECC)
{
    BOOL32 BoolStatus, dontRead;
    UInt8 i;
    // Handling uECCs, trying older data:
    BOOL32 failed, rereadOrig;
    UInt32 srcVpn;

    dontRead = FALSE32;
    if (index) {
        dontRead = TRUE32;
        for (i = 0; dontRead && (i < c->curZoneSize); i++)
        {
            META_SET_IPN_FROM_BTOC(&c->meta[i], c->TOCbuff[c->vpns[i] % PAGES_PER_SUBLK]);
            WMR_ASSERT(META_GET_LBA(&c->meta[i]) != 0xffffffff);
            dontRead = dontRead && ReconstructIndex(META_GET_IPN(&c->meta[i]), (UInt32*)&c->zone[BYTES_PER_PAGE * i]);
        }
        BoolStatus = TRUE32;
    }
    if (!dontRead) {
        BoolStatus = _readMultiPages(c->vpns, c->curZoneSize, c->zone, (UInt8*)c->meta, !index, scrubOnUECC);
    }

    if (BoolStatus == TRUE32)
    {
        ScrubZone(c, index);
        return FTL_SUCCESS;
    }
    else
    {
        for (i = 0; i < c->curZoneSize; i++)
        {
            if (_readPage(c->vpns[i], &c->zone[BYTES_PER_PAGE * i], &c->meta[i], !index, TRUE32, scrubOnUECC) != FTL_SUCCESS)
            {
                failed = TRUE32;
                if (!index)
                {
                    srcVpn = BTOC_GetSrc(c->vpns[i]);
                    if (0xffffffff != srcVpn)
                    {
                        rereadOrig = FALSE32;
                        if (_readPage(srcVpn, &c->zone[BYTES_PER_PAGE * i], &c->meta[i], !index, TRUE32, scrubOnUECC) == FTL_SUCCESS)
                        {
                            failed = FALSE32;
                        }
                        else
                        {
                            rereadOrig = TRUE32;
                        }

                        if (rereadOrig || (!rereadOrig && (c->TOCbuff[c->vpns[i] % PAGES_PER_SUBLK] != META_GET_LBA(&c->meta[i]))))
                        {
                            _readPage(c->vpns[i], &c->zone[BYTES_PER_PAGE * i], &c->meta[i], !index, TRUE32, scrubOnUECC);
                        }
                    }
                }

                if (failed) {
                    if (index) {
                        META_SET_IPN_FROM_BTOC(&c->meta[i], c->TOCbuff[c->vpns[i] % PAGES_PER_SUBLK]);
                    } else {
                        META_SET_LBA(&c->meta[i], c->TOCbuff[c->vpns[i] % PAGES_PER_SUBLK]);
                    }
                    WMR_ASSERT(META_GET_LBA(&c->meta[i]) != 0xffffffff);
                    if (index) {
                        if (!ReconstructIndex(META_GET_IPN(&c->meta[i]), (UInt32*)&c->zone[BYTES_PER_PAGE * i])) {
                            // Invalidate context
                            if (yaFTL.cxtValid == 1)
                            {
                                invalidateCXT();
                                yaFTL.cxtValid = 0;
                            }
                            WMR_PANIC("Couldn't reconstruct yaFTL index after a uECC: vpn 0x%x", c->vpns[i]);
                        }
                    } else {
                        SetupMeta_Data_UECC(&c->meta[i]);
                    }
                    c->ueccCounter++;
                }
            }
        }
    }

    ScrubZone(c, index);
    return FTL_SUCCESS;
}

static ANDStatus WriteZoneData(GC_Ctx_t *c, BOOL32 scrubOnUECC)
{
    BOOL32 status;
    UInt8 i;
    UInt32 prevVpn;
    UInt16 cacheIndexPtr;
    UInt32 count, thisOfs, thisCount;
    PageMeta_t *mdPtr = &yaFTL.gc.data.meta[0];
    PageMeta_t *md_index = yaFTL.gc.meta_buf;
    UInt32 indexPageNo, indexPageSub;
    UInt32 newVpn;

    thisOfs = 0;
    count = c->curZoneSize;
    c->gcDataAdjust += c->curZoneSize;
    
    // While we still have data we want to move,
    while (count)
    {
        // If we have space left in this block,
        if (yaFTL.wrState.data.nextPage < (PAGES_PER_SUBLK - yaFTL.controlPageNo))
        {
            // Move as much as we can
            thisCount = WMR_MIN((UInt32)((PAGES_PER_SUBLK - yaFTL.controlPageNo) - yaFTL.wrState.data.nextPage), (UInt32)count);

            // Fill in metadata
            SetupMeta_DataGC(&mdPtr[thisOfs], thisCount);

            // Write the data
            status = _writeMultiPages(yaFTL.wrState.data.block, yaFTL.wrState.data.nextPage, thisCount, &yaFTL.gc.data.zone[thisOfs * BYTES_PER_PAGE], &mdPtr[thisOfs], TRUE32);
            if (status == TRUE32)
            {
                newVpn = (yaFTL.wrState.data.block * PAGES_PER_SUBLK) + yaFTL.wrState.data.nextPage;
                for (i = 0; i < thisCount; i++, newVpn++)
                {
                    // Update block TOC
                    yaFTL.wrState.data.TOC[yaFTL.wrState.data.nextPage + i] = META_GET_LBA(&mdPtr[thisOfs + i]);
                    // and src vpn
                    BTOC_SetSrc(newVpn, yaFTL.gc.data.vpns[thisOfs + i]);
                    // And temporary housing for updates to tree
                    yaFTL.gc.data.vpns[thisOfs + i] = newVpn;
                }
                yaFTL.wrState.data.nextPage += thisCount;
                thisOfs += thisCount;
            }
            else
            {
                if(addBlockToEraseNowList(yaFTL.wrState.data.block) == FALSE32)
                {
                    // Invalidate context
                    if (yaFTL.cxtValid == 1)
                    {
                        invalidateCXT();
                        yaFTL.cxtValid = 0;
                    }
                    WMR_PANIC("cannot add block %d to a scrub list", yaFTL.wrState.data.block);   
                }
                WorkFifo_Enq(&c->workFifo, c->chosenBlock);
                WorkFifo_Enq(&c->workFifo, yaFTL.wrState.data.block);
                allocateBlock(yaFTL.wrState.data.block, 0, 1);

                return FTL_CRITICAL_ERROR;
            }

            // Move along
            count -= thisCount;
        }

        // Want to write more data but out of space?
        if (count && (yaFTL.wrState.data.nextPage >= (PAGES_PER_SUBLK - yaFTL.controlPageNo)))
        {
            // Write the block TOC
            writeCurrentBlockTOC(1);

            // Move to the next block
            allocateBlock(yaFTL.wrState.data.block, 0, 1);
        }
    }

    // Now we need to update index to reflect change in Lpn location
    for (i = 0; i < c->curZoneSize; i++)
    {
#if ENABLE_L2V_TREE
        L2V_Update(META_GET_LBA(&mdPtr[i]), 1, yaFTL.gc.data.vpns[i]);
#endif // ENABLE_L2V_TREE

        // Calculate which index page and offset within for this update
        indexPageNo = META_GET_LBA(&mdPtr[i]) / yaFTL.indexPageRatio;
        indexPageSub = META_GET_LBA(&mdPtr[i]) % yaFTL.indexPageRatio;

        WMR_ASSERT((yaFTL.tocArray[indexPageNo].indexPage != 0xffffffff) || (yaFTL.tocArray[indexPageNo].cacheIndex != 0xffff));

        // This is a page that belongs to valid index batch
        // Need to get index page to cache and update its content
        cacheIndexPtr = yaFTL.tocArray[indexPageNo].cacheIndex;
        if (0xffff == cacheIndexPtr)
        {
            cacheIndexPtr = findFreeCacheEntry();
            if (cacheIndexPtr == 0xffff)
            {
                /* Cache is full and we need to clear space for new index page */
                cacheIndexPtr = clearEntryInCache(0, 0xffff, 0);
                if (cacheIndexPtr == 0xffff)
                {
                    // Invalidate context
                    if (yaFTL.cxtValid == 1)
                    {
                        invalidateCXT();
                        yaFTL.cxtValid = 0;
                    }
                    WMR_PANIC("failed to evict index page from cache");
                }
            }
            status = _readPage(yaFTL.tocArray[indexPageNo].indexPage, (UInt8 *)(yaFTL.indexCache[cacheIndexPtr].indexPage), md_index, FALSE32, TRUE32, scrubOnUECC);
            if (status != FTL_SUCCESS)
            {
                if (yaFTL.cxtValid == 1)
                {
                    invalidateCXT();
                    yaFTL.cxtValid = 0;
                }  

                WMR_PANIC("Index UECC Page 0x%08x Status 0x%08x", yaFTL.tocArray[indexPageNo].indexPage, status);
                return status;
            }
            yaFTL.freeCachePages--;
            // Reset counter since we just pulled this in to the cache
            yaFTL.indexCache[cacheIndexPtr].counter = 0;
        }

        // Find previous physical location
        prevVpn = yaFTL.indexCache[cacheIndexPtr].indexPage[indexPageSub];
        if (prevVpn != 0xffffffff)
        {
            // Deprecate validity
            if (yaFTL.blockArray[prevVpn / PAGES_PER_SUBLK].validPagesDNo == 0)
            {
                debug(ERROR, "miscalculated valid pages ");
                if (yaFTL.cxtValid == 1)
                {
                    invalidateCXT();
                    yaFTL.cxtValid = 0;
                }  

                WMR_PANIC("Incorrect valid pages VPN 0x%08x Valid %d", 
                          prevVpn, yaFTL.blockArray[prevVpn / PAGES_PER_SUBLK].validPagesDNo);
                return FTL_CRITICAL_ERROR;
            }
            yaFTL.blockArray[prevVpn / PAGES_PER_SUBLK].validPagesDNo--;
            yaFTL.seaState.data.validPages--;
#ifdef AND_COLLECT_STATISTICS
            stFTLStatistics.ddwValidDataPages--;
#endif
        }

        // Update with new page location
        yaFTL.indexCache[cacheIndexPtr].indexPage[indexPageSub] = yaFTL.gc.data.vpns[i];
        // Update validity
        yaFTL.blockArray[yaFTL.gc.data.vpns[i] / PAGES_PER_SUBLK].validPagesDNo++;
        yaFTL.seaState.data.validPages++;
#ifdef AND_COLLECT_STATISTICS
        stFTLStatistics.ddwValidDataPages++;
#endif

        // Cross-link cache entry with index page info (redunant if it was already touched but it's ok)
        yaFTL.indexCache[cacheIndexPtr].tocEntry = indexPageNo;
        yaFTL.tocArray[indexPageNo].cacheIndex = cacheIndexPtr;

        // Mark previous index page as invalid if we just sucked it into cache
        if (yaFTL.tocArray[indexPageNo].indexPage != 0xffffffff)
        {
            yaFTL.blockArray[(yaFTL.tocArray[indexPageNo].indexPage) / PAGES_PER_SUBLK].validPagesINo--;
            yaFTL.seaState.index.validPages--;
#ifdef AND_COLLECT_STATISTICS
            stFTLStatistics.ddwValidIndexPages--;
#endif
            yaFTL.tocArray[indexPageNo].indexPage = 0xffffffff;
        }

        // Mark cache entry as being dirty and touch the counter
        yaFTL.indexCache[cacheIndexPtr].status = IC_DIRTY_PAGE;
        if (yaFTL.indexCache[cacheIndexPtr].counter < YAFTL_INDEX_CACHE_COUNTER_LIMIT)
            yaFTL.indexCache[cacheIndexPtr].counter++;
    }

    return FTL_SUCCESS;
}

static ANDStatus WriteZoneIndex(GC_Ctx_t *c, BOOL32 scrubOnUECC)
{
    BOOL32 BoolStatus;
    UInt8 i;
    UInt32 count, thisCount, thisOfs;
    PageMeta_t *mdPtr = &yaFTL.gc.index.meta[0];

    thisOfs = 0;
    count = c->curZoneSize;

    // While we still have data we want to move,
    while (count)
    {
        // And if we have space left in this block,
        if (yaFTL.wrState.index.nextPage < (PAGES_PER_SUBLK - yaFTL.controlPageNo))
        {
            thisCount = WMR_MIN((UInt32)((PAGES_PER_SUBLK - yaFTL.controlPageNo) - yaFTL.wrState.index.nextPage), (UInt32)count);

            // Fill in metadata
            SetupMeta_IndexGC(&mdPtr[thisOfs], thisCount);

            // Write the data
            BoolStatus = _writeMultiPages(yaFTL.wrState.index.block, yaFTL.wrState.index.nextPage, thisCount, &yaFTL.gc.index.zone[thisOfs * BYTES_PER_PAGE], &mdPtr[thisOfs], FALSE32);
            if (BoolStatus == TRUE32)
            {
                for (i = 0; i < thisCount; i++)
                {
                    // Update block TOC
                    BTOC_SET_FROM_IPN(yaFTL.wrState.index.TOC[yaFTL.wrState.index.nextPage + i], &mdPtr[thisOfs + i]);
                    yaFTL.tocArray[META_GET_IPN(&mdPtr[thisOfs + i])].indexPage = yaFTL.wrState.index.block * PAGES_PER_SUBLK + yaFTL.wrState.index.nextPage + i;
                }

                // Update valid counts
                yaFTL.blockArray[yaFTL.wrState.index.block].validPagesINo += thisCount;
                yaFTL.blockArray[c->chosenBlock].validPagesINo -= thisCount;
                c->pagesCopied += thisCount;

                yaFTL.wrState.index.nextPage += thisCount;
                thisOfs += thisCount;
            }
            else
            {
                if(addBlockToEraseNowList(yaFTL.wrState.index.block) == FALSE32)
                {
                    // Invalidate context
                    if (yaFTL.cxtValid == 1)
                    {
                        invalidateCXT();
                        yaFTL.cxtValid = 0;
                    }
                    WMR_PANIC("cannot add block %d to a scrub list", yaFTL.wrState.index.block);   
                }
                WorkFifo_Enq(&c->workFifo, c->chosenBlock);
                WorkFifo_Enq(&c->workFifo, yaFTL.wrState.index.block);
                allocateBlock(yaFTL.wrState.index.block, 0, 0);
                return FTL_CRITICAL_ERROR;
            }

            // Move along
            count -= thisCount;
        }

        // Want to write more data but out of space?
        if (count && (yaFTL.wrState.index.nextPage >= (PAGES_PER_SUBLK - yaFTL.controlPageNo)))
        {
            // Write the block TOC
            writeCurrentBlockTOC(0);

            // Move to the next block
            allocateBlock(yaFTL.wrState.index.block, 0, 0);
            WMR_ASSERT(0 == yaFTL.wrState.index.nextPage);
        }
    }

    // And get outta here
    return FTL_SUCCESS;
}

#endif // AND_READONLY

