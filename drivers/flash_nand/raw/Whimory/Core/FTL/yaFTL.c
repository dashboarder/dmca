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

#define TIME_TREE 0
#define AND_TRACE_LAYER FTL

#include "yaFTL_whoami.h"
#include "WMRConfig.h"
#include "ANDTypes.h"
#include "WMROAM.h"
#include "WMRBuf.h"
#include "VFLBuffer.h"
#include "VFL.h"
#include "yaFTLTypes.h"
#include "FTL.h"
#include "yaFTL_Defines.h"
#include "yaFTL_gc.h"
#include "yaFTL_BTOC.h"
#include "L2V/L2V_Extern.h"
#if TIME_TREE
#include <kern/clock.h>
#endif
#include "WMRFeatures.h"
#ifndef ENABLE_L2V_TREE
#error ENABLE_L2V_TREE must be set to 0 or 1 in WMRFeatures.h
#endif

#define kYaftlMinorVersion (1)


#if !NAND_RAW && !NAND_PPN
#error PPN or raw?
#endif

#ifdef AND_COLLECT_STATISTICS
FTLStatistics stFTLStatistics;
#endif /* AND_COLLECT_STATISTICS */

#define CXT_VER "CX01"
FTLWMRDeviceInfo yaFTL_FTLDeviceInfo;
VFLFunctions yaFTL_VFLFunctions;
yaFTL_t yaFTL;
static BOOL32 yaftl_init_done = FALSE32;

/* static functions declaration section */
static void         YAFTL_FreeMemory(void);
static Int32        YAFTL_Init(VFLFunctions *pVFLFunctions);
static Int32        YAFTL_Open(UInt32 *pTotalScts,
                               UInt32 * pdwSectorSize, 
                               BOOL32 nandFullRestore,
                               BOOL32 justFormatted,
                               UInt32 dwMinorVer,
                               UInt32 dwOptions);
static Int32        YAFTL_Read(UInt32 nLpn, UInt32 nNumOfScts,
                               UInt8 *pBuf);
static void         YAFTL_Close(void);
static BOOL32       YAFTL_GetStruct(UInt32 dwStructType,
                                    void * pvoidStructBuffer,
                                    UInt32 * pdwStructSize);
Int32               _readPage(UInt32 vpn, UInt8 *pageData,
                              PageMeta_t *pMeta,
                              BOOL32 bInternalOp, BOOL32 boolCleanCheck, BOOL32 scrubOnUECC);
BOOL32              _readMultiPages(UInt32 * padwVpn,
                                    UInt16 wNumPagesToRead, UInt8 * pbaData,
                                    UInt8 * pbaSpare,
                                    BOOL32 bInternalOp, BOOL32 scrubOnUECC);
#if ENABLE_L2V_TREE
static void         PopulateFromIndexCache(UInt32 indexPageNo, UInt32 *indexPageData);
#endif //ENABLE_L2V_TREE

static void         PopulateTreesOnBoot_Fast(void);

#ifndef AND_READONLY
static Int32        YAFTL_Write(UInt32 nLpn, UInt32 nNumOfScts,
                                UInt8 *pBuf, BOOL32 isStatic);
static Int32        YAFTL_Format(UInt32 dwOptions);
static Int32        YAFTL_WearLevel(void);
static BOOL32       YAFTL_GarbageCollect(void);
static BOOL32       YAFTL_ShutdownNotify(BOOL32 boolMergeLogs);
Int32               _writePage(UInt32 vpn, UInt8 *pageData,
                               PageMeta_t *,
                               BOOL32 bInternalOp);
static ANDStatus    writeIndexPage(UInt8 *pageBuffer, PageMeta_t *mdPtr,
                                   UInt8 flag);
UInt32              _allocateBlock(UInt32 blockNo, UInt8 flag, UInt8 type);
BOOL32              _writeMultiPages(UInt16 vbn, UInt16 pageOffset,
                                     UInt16 wNumPagesToWrite, UInt8 *pageData,
                                     PageMeta_t *mdPtr,
                                     BOOL32 bInternalOp);
UInt32*             IndexLoadDirty(UInt32 lba, UInt32 *indexOfs);
UInt32*             IndexLoadClean(UInt32 lba, UInt32 *indexOfs, UInt32 *d_tocEntry);
void                IndexMarkDirty(UInt32 tocEntry);
ANDStatus invalidateCXT(void);
BOOL32 isBlockInEraseNowList(UInt16 blockNo);
BOOL32 removeBlockFromEraseNowList(UInt16 blockNo);
BOOL32 addBlockToEraseNowList(UInt16 blockNo);

#endif // ! AND_READONLY

/*
   static variables/structures declaration section
 */

typedef struct blockList
{
    WeaveSeq_t weaveSeq;
    UInt16 blockNo;
    struct blockList *next;
    struct blockList *prev;
} BlockListType;

typedef struct BlockRange
{
    UInt32 start;
    UInt32 end;
} BlockRangeType;

#if NAND_PPN
typedef struct YAFTLControlInfo
{
    UInt32 versionNo;
    UInt32 indexSize;       /* index size in blocks */
    UInt32 logicalPartitionSize; /* partititon size in pages exposed to file system */
    UInt32 wrState_data_block;    /* current physical block being used for updates */
    UInt32 wrState_data_weaveSeq_Lo;
    UInt32 wrState_data_weaveSeq_Hi;
    UInt32 wrState_index_block;    /* current physical block being used for updates */
    UInt32 wrState_index_weaveSeq_Lo;
    UInt32 wrState_index_weaveSeq_Hi;
    UInt32 seaState_data_freeBlocks;  /* number of free ( erased ) blocks */
    UInt32 seaState_index_freeBlocks;  /* number of free ( erased ) blocks */
    UInt32 seaState_data_allocdBlocks;
    UInt32 seaState_index_allocdBlocks;
    UInt32 indexCacheSize;
    UInt32 FTLRestoreCnt;
    UInt32 reserved32[10];

    UInt16 TOCtableEntriesNo;  /* number of entries in index TOC table */
    UInt16 controlPageNo;
    UInt16 indexPageRatio;   /* TOC of each block contains Lpa for each page written to block */
    UInt16 freeCachePages;
    UInt16 nextFreeCachePage;
    UInt16 wrState_data_nextPage;   /* offset in current block of next free page */
    UInt16 wrState_index_nextPage;   /* offset in current block of next free page */
    UInt32 reserved16[10];

    UInt8 exportedRatio;
    UInt8 cxtEraseCounter;
    UInt8 reserved8[10];
} YAFTLControlInfoType;
#elif NAND_RAW
typedef struct YAFTLControlInfo
{
    UInt32 versionNo;
    UInt32 indexSize;       /* index size in blocks */
    UInt32 logicalPartitionSize; /* partititon size in pages exposed to file system */
    UInt32 wrState_data_block;    /* current physical block being used for updates */
    UInt32 wrState_data_weaveSeq;
    UInt32 wrState_index_block;    /* current physical block being used for updates */
    UInt32 wrState_index_weaveSeq;
    UInt32 seaState_data_freeBlocks;  /* number of free ( erased ) blocks */
    UInt32 seaState_index_freeBlocks;  /* number of free ( erased ) blocks */
    UInt32 seaState_data_allocdBlocks;
    UInt32 seaState_index_allocdBlocks;
    UInt32 indexCacheSize;
    UInt32 FTLRestoreCnt;
    UInt32 reserved32[10];

    UInt16 TOCtableEntriesNo;  /* number of entries in index TOC table */
    UInt16 controlPageNo;
    UInt16 indexPageRatio;   /* TOC of each block contains Lpa for each page written to block */
    UInt16 freeCachePages;
    UInt16 nextFreeCachePage;
    UInt16 wrState_data_nextPage;   /* offset in current block of next free page */
    UInt16 wrState_index_nextPage;   /* offset in current block of next free page */
    UInt32 reserved16[10];

    UInt8 exportedRatio;
    UInt8 cxtEraseCounter;
    UInt8 reserved8[10];
} YAFTLControlInfoType;
#else
#error PPN or raw?
#endif

static BOOL32        _allocateRestoreIndexBuffers(BlockRangeType **blockRangeBuffer, UInt32 **indexTable, UInt32 *indexTableSize);

#ifdef AND_COLLECT_STATISTICS
#ifndef AND_READONLY
static BOOL32 _FTLGetStatisticsToCxt(UInt8 * pabData);
#endif //ifndef AND_READONLY
static BOOL32 _FTLSetStatisticsFromCxt(UInt8 * pabData);
#endif
#ifndef AND_READONLY
static BOOL32    YAFTL_WriteStats(void);
#define CXT_ERASE_GAP 50
#define MOVE_CXT_TRESHOLD 1000

#define ERASE_CXT_UPDATE_THRESHOLD (FTL_AREA_SIZE>>4)
#define WRITE_CXT_UPDATE_THRESHOLD (USER_PAGES_TOTAL>>4)
#endif 
static UInt32 tempIndexBufferSize = 0;
/*****************************************************************************/
/* Code Implementation                                                       */
/*****************************************************************************/

void freeBlockList(BlockListType * listHead)
{
    BlockListType * tmpPtr = listHead;
    while(tmpPtr != NULL)
    {
        listHead = tmpPtr;
        tmpPtr = tmpPtr->next;
        WMR_FREE(listHead, sizeof(BlockListType));
        
    }
}

#if TIME_TREE
mach_timebase_info_data_t sTimebaseInfo;

static UInt64 ANDPerf_getTime() {
    return mach_absolute_time();
}

static UInt32 ANDPerf_toUs(UInt64 elapsed) {
       if (sTimebaseInfo.denom != sTimebaseInfo.numer) {
           if (sTimebaseInfo.denom == 1) {
            elapsed *= sTimebaseInfo.numer;
           } else {
            elapsed = (elapsed * sTimebaseInfo.numer) / (uint64_t)sTimebaseInfo.denom;
           }
       }

    return elapsed / 1000 /* NS_PER_USEC */;
}
#endif // TIME_TREE

static void updateReadCounter(UInt16 vba)
{
    if (yaFTL.blockArray[vba].pagesRead > YAFTL_READ_DISTURB_LIMIT)
        return;

    yaFTL.blockArray[vba].pagesReadSubCounter++;
    if(yaFTL.blockArray[vba].pagesReadSubCounter >= PAGES_READ_SUB_COUNTER_LIMIT)
    {
        yaFTL.blockArray[vba].pagesReadSubCounter = 0;
        yaFTL.blockArray[vba].pagesRead++;
    }
}

static BOOL32 _InitDeviceInfo(void)
{
    yaFTL_FTLDeviceInfo.wPagesPerVb =
        (UInt16)VFL_GetDeviceInfo(AND_DEVINFO_PAGES_PER_SUBLK);
    yaFTL_FTLDeviceInfo.wUserVbTotal =
        (UInt16)VFL_GetDeviceInfo(AND_DEVINFO_NUM_OF_USER_SUBLK);
    yaFTL_FTLDeviceInfo.wBytesPerPage =
        (UInt16)VFL_GetDeviceInfo(AND_DEVINFO_BYTES_PER_PAGE);
    yaFTL_FTLDeviceInfo.wBytesPerPageMeta =
        (UInt16)VFL_GetDeviceInfo(AND_DEVINFO_FIL_META_BUFFER_BYTES) * (UInt16)VFL_GetDeviceInfo(AND_DEVINFO_FIL_LBAS_PER_PAGE);
    yaFTL_FTLDeviceInfo.wNumOfBanks =
        (UInt16)VFL_GetDeviceInfo(AND_DEVINFO_NUM_OF_BANKS);
    yaFTL_FTLDeviceInfo.dwUserPagesTotal = (UInt32)yaFTL_FTLDeviceInfo.wUserVbTotal *
                                           (UInt32)yaFTL_FTLDeviceInfo.wPagesPerVb;

    debug(INIT, "wPagesPerVb 0x%X", yaFTL_FTLDeviceInfo.wPagesPerVb);
    debug(INIT, "wUserVbTotal 0x%X", yaFTL_FTLDeviceInfo.wUserVbTotal);
    debug(INIT, "wBytesPerPage 0x%X", yaFTL_FTLDeviceInfo.wBytesPerPage);
    debug(INIT, "wNumOfBanks 0x%X", yaFTL_FTLDeviceInfo.wNumOfBanks);
    debug(INIT, "dwUserPagesTotal 0x%X", yaFTL_FTLDeviceInfo.dwUserPagesTotal);

    if (sizeof(PageMeta_t) != yaFTL_FTLDeviceInfo.wBytesPerPageMeta) {
        WMR_PANIC("yaFTL meta struct size (%d) not equal to bytes per metadata (%d)", sizeof(PageMeta_t), yaFTL_FTLDeviceInfo.wBytesPerPageMeta);
    }

    return TRUE32;
}

/*****************************************************************************/
/*                                                                           */
/* NAME                                                                      */
/*      YAFTL_Init															 */
/* DESCRIPTION                                                               */
/*      This function initializes yaftl global structures					 */
/* RETURN VALUES                                                             */
/*		FTL_SUCCESS															 */
/*				YAFTL_Init is completed										 */
/*		FTL_CRITICAL_ERROR													 */
/*				YAFTL_Init is failed										 */
/* NOTES                                                                     */
/*                                                                           */
/*****************************************************************************/
static Int32
YAFTL_Init(VFLFunctions *pVFLFunctions)
{
    UInt16 pagesInBlk;
    UInt16 pageSize;
    UInt32 temp1;
    
    // Do not allow multiple init() calls because number of user blocks can change
	WMR_ASSERT(!yaftl_init_done);
    
    // Guarantee that yaftl context was packed as expected
#if NAND_PPN
    WMR_ASSERT(sizeof(struct YAFTLControlInfo) == 168);
#elif NAND_RAW
    WMR_ASSERT(sizeof(struct YAFTLControlInfo) == 160);
#else
#error PPN or raw?
#endif

    // Initialize yaFTL structure
    WMR_MEMSET((void*)&yaFTL, 0, sizeof(yaFTL));
    yaFTL.readBufferIndex = 0xffffffff;
    yaFTL.currentCxtVpn = 0xffffffff;
    yaFTL.indexCacheSize = DEFAULT_INDEX_CACHE_SIZE;
    yaFTL.exportedRatio = DEFAULT_EXPO_RATIO;

    WMR_MEMCPY(&yaFTL_VFLFunctions, pVFLFunctions, sizeof(VFLFunctions));
    debug(INIT, "YAFTL_INIT[start]");
    _InitDeviceInfo();

    pagesInBlk = PAGES_PER_SUBLK;
    pageSize = BYTES_PER_PAGE;
    /* find out how many pages we need to keep pointer to each data/index page within unit */
    for (yaFTL.controlPageNo = 1; (Int32)(pagesInBlk - yaFTL.controlPageNo) * sizeof(Int32) > ((Int32)pageSize * yaFTL.controlPageNo) - (yaFTL.controlPageNo * sizeof(Int32)); yaFTL.controlPageNo++)

    {
        ;
    }
    debug(INIT, "yaFTL.controlPageNo %d", yaFTL.controlPageNo);
    debug(INIT, " PAGES_PER_SUBLK %d", PAGES_PER_SUBLK);
    debug(INIT, "BYTES_PER_PAGE %d", BYTES_PER_PAGE);

    yaFTL.indexPageRatio = pageSize / sizeof(UInt32);

    // Buffer allocations
    WMR_BufZone_Init(&yaFTL.BufZone);
    yaFTL.quickRestore_tBuff = (UInt8 *)WMR_Buf_Alloc_ForDMA(&yaFTL.BufZone, BYTES_PER_PAGE);
    yaFTL.tmpReadBuffer = (UInt32 *)WMR_Buf_Alloc_ForDMA(&yaFTL.BufZone, BYTES_PER_PAGE);
    yaFTL.restoreTOC = (UInt32 *)WMR_Buf_Alloc_ForDMA(&yaFTL.BufZone, yaFTL.controlPageNo * BYTES_PER_PAGE);
    yaFTL.meta_restoreIdx = (PageMeta_t *)WMR_Buf_Alloc_ForDMA(&yaFTL.BufZone, sizeof(PageMeta_t));
    yaFTL.meta_restoreData = (PageMeta_t *)WMR_Buf_Alloc_ForDMA(&yaFTL.BufZone, sizeof(PageMeta_t));
    yaFTL.meta_readCxt = (PageMeta_t *)WMR_Buf_Alloc_ForDMA(&yaFTL.BufZone, sizeof(PageMeta_t));
    yaFTL.meta_quickRestore = (PageMeta_t *)WMR_Buf_Alloc_ForDMA(&yaFTL.BufZone, sizeof(PageMeta_t));
    yaFTL.meta_invalidateCxt = (PageMeta_t *)WMR_Buf_Alloc_ForDMA(&yaFTL.BufZone, sizeof(PageMeta_t));
    yaFTL.meta_writeCxt = (PageMeta_t *)WMR_Buf_Alloc_ForDMA(&yaFTL.BufZone, sizeof(PageMeta_t));
    yaFTL.meta_restoreMountErase = (PageMeta_t *)WMR_Buf_Alloc_ForDMA(&yaFTL.BufZone, sizeof(PageMeta_t));
    yaFTL.meta_restoreMount = (PageMeta_t *)WMR_Buf_Alloc_ForDMA(&yaFTL.BufZone, sizeof(PageMeta_t));
    yaFTL.meta_clearEntryInCache = (PageMeta_t *)WMR_Buf_Alloc_ForDMA(&yaFTL.BufZone, sizeof(PageMeta_t));
    yaFTL.meta_writeBTOC = (PageMeta_t *)WMR_Buf_Alloc_ForDMA(&yaFTL.BufZone, sizeof(PageMeta_t));
    yaFTL.meta_indexLoadDirty = (PageMeta_t *)WMR_Buf_Alloc_ForDMA(&yaFTL.BufZone, sizeof(PageMeta_t));
    yaFTL.meta_getStats = (PageMeta_t *)WMR_Buf_Alloc_ForDMA(&yaFTL.BufZone, sizeof(PageMeta_t));
    yaFTL.meta_GetBlockTOC = (PageMeta_t *)WMR_Buf_Alloc_ForDMA(&yaFTL.BufZone, sizeof(PageMeta_t));
    yaFTL.meta_IsDataPageCurrent = (PageMeta_t *)WMR_Buf_Alloc_ForDMA(&yaFTL.BufZone, sizeof(PageMeta_t));
    yaFTL.meta_WriteZoneData = (PageMeta_t *)WMR_Buf_Alloc_ForDMA(&yaFTL.BufZone, sizeof(PageMeta_t));
    yaFTL.singleMdPtr = (PageMeta_t *)WMR_Buf_Alloc_ForDMA(&yaFTL.BufZone, sizeof(PageMeta_t));
    yaFTL.writeMdPtr = (PageMeta_t *)WMR_Buf_Alloc_ForDMA(&yaFTL.BufZone, NUM_BANKS * YAFTL_WRITE_MAX_STRIPES * sizeof(PageMeta_t));
    yaFTL.readMdPtr = (PageMeta_t *)WMR_Buf_Alloc_ForDMA(&yaFTL.BufZone, (PAGES_PER_SUBLK)*sizeof(PageMeta_t));

    if (!WMR_BufZone_FinishedAllocs(&yaFTL.BufZone)) {
        goto errorPath;
    }

    WMR_BufZone_Rebase(&yaFTL.BufZone, (void**)&yaFTL.quickRestore_tBuff);
    WMR_BufZone_Rebase(&yaFTL.BufZone, (void**)&yaFTL.tmpReadBuffer);
    WMR_BufZone_Rebase(&yaFTL.BufZone, (void**)&yaFTL.restoreTOC);
    WMR_BufZone_Rebase(&yaFTL.BufZone, (void**)&yaFTL.meta_restoreIdx);
    WMR_BufZone_Rebase(&yaFTL.BufZone, (void**)&yaFTL.meta_restoreData);
    WMR_BufZone_Rebase(&yaFTL.BufZone, (void**)&yaFTL.meta_readCxt);
    WMR_BufZone_Rebase(&yaFTL.BufZone, (void**)&yaFTL.meta_quickRestore);
    WMR_BufZone_Rebase(&yaFTL.BufZone, (void**)&yaFTL.meta_invalidateCxt);
    WMR_BufZone_Rebase(&yaFTL.BufZone, (void**)&yaFTL.meta_writeCxt);
    WMR_BufZone_Rebase(&yaFTL.BufZone, (void**)&yaFTL.meta_restoreMountErase);
    WMR_BufZone_Rebase(&yaFTL.BufZone, (void**)&yaFTL.meta_restoreMount);
    WMR_BufZone_Rebase(&yaFTL.BufZone, (void**)&yaFTL.meta_clearEntryInCache);
    WMR_BufZone_Rebase(&yaFTL.BufZone, (void**)&yaFTL.meta_writeBTOC);
    WMR_BufZone_Rebase(&yaFTL.BufZone, (void**)&yaFTL.meta_indexLoadDirty);
    WMR_BufZone_Rebase(&yaFTL.BufZone, (void**)&yaFTL.meta_getStats);
    WMR_BufZone_Rebase(&yaFTL.BufZone, (void**)&yaFTL.meta_GetBlockTOC);
    WMR_BufZone_Rebase(&yaFTL.BufZone, (void**)&yaFTL.meta_IsDataPageCurrent);
    WMR_BufZone_Rebase(&yaFTL.BufZone, (void**)&yaFTL.meta_WriteZoneData);
    WMR_BufZone_Rebase(&yaFTL.BufZone, (void**)&yaFTL.singleMdPtr);
    WMR_BufZone_Rebase(&yaFTL.BufZone, (void**)&yaFTL.writeMdPtr);
    WMR_BufZone_Rebase(&yaFTL.BufZone, (void**)&yaFTL.readMdPtr);
    WMR_BufZone_FinishedRebases(&yaFTL.BufZone);

    debug(INIT, "yaFTL.controlPageNo %d", yaFTL.controlPageNo);
    debug(INIT, "BYTES_PER_PAGE %d", BYTES_PER_PAGE);
    debug(INIT, "indexPageRatio %d", yaFTL.indexPageRatio);

    /* size of media in pages after we take out CTX ,free blocks and block TOC for each block */
    temp1 = (pagesInBlk - yaFTL.controlPageNo) * (FTL_AREA_SIZE - FTL_CXT_SECTION_SIZE - FREE_BLK_TRH);

    yaFTL.indexSize = (temp1 / (yaFTL.indexPageRatio * (pagesInBlk - yaFTL.controlPageNo))) * INDEX_BLOCKS_RATIO;
    if (temp1 % (yaFTL.indexPageRatio * (pagesInBlk - yaFTL.controlPageNo)))
    {
        yaFTL.indexSize += INDEX_BLOCKS_RATIO;
    }

    yaFTL.dataSize = FTL_AREA_SIZE - (yaFTL.indexSize + FTL_CXT_SECTION_SIZE);
    yaFTL.logicalPartitionSize = temp1 - yaFTL.indexSize * PAGES_PER_SUBLK;  /* logical parititon size in pages . this is the size reported to file system */

    yaFTL.TOCtableEntriesNo = (FTL_AREA_SIZE * PAGES_PER_SUBLK * 4) / BYTES_PER_PAGE;
    yaFTL.tocArray = (TOCEntry *)WMR_MALLOC(yaFTL.TOCtableEntriesNo * sizeof(TOCEntry));
    if (yaFTL.tocArray == NULL)
    {
        goto errorPath;
    }
    yaFTL.blockArray = (BlockEntry *)WMR_MALLOC(FTL_AREA_SIZE * sizeof(BlockEntry));

    if (yaFTL.blockArray == NULL)
    {
        goto errorPath;
    }

    yaFTL.multiReadvpN = (UInt32 *)WMR_MALLOC(sizeof(UInt32) * PAGES_PER_SUBLK);
    if (yaFTL.multiReadvpN == NULL)
    {
        goto errorPath;
    }
    
    yaFTL.cxtSizeTOC = (yaFTL.TOCtableEntriesNo * sizeof(UInt32)) / BYTES_PER_PAGE;
    if ((yaFTL.TOCtableEntriesNo * sizeof(UInt32)) % BYTES_PER_PAGE)
    {
        yaFTL.cxtSizeTOC++;
    }

    yaFTL.cxtSizeBlockStatus = FTL_AREA_SIZE / BYTES_PER_PAGE;
    if (FTL_AREA_SIZE % BYTES_PER_PAGE)
    {
        yaFTL.cxtSizeBlockStatus++;
    }

    yaFTL.cxtSizeBlockRead = (FTL_AREA_SIZE * sizeof(UInt16)) / BYTES_PER_PAGE;
    if ((FTL_AREA_SIZE * sizeof(UInt16)) % BYTES_PER_PAGE)
    {
        yaFTL.cxtSizeBlockRead++;
    }

    yaFTL.cxtSizeBlockErase = (FTL_AREA_SIZE * sizeof(UInt32)) / BYTES_PER_PAGE;
    if ((FTL_AREA_SIZE * sizeof(UInt32)) % BYTES_PER_PAGE)
    {
        yaFTL.cxtSizeBlockErase++;
    }

    yaFTL.cxtSizeValidPagesD = (FTL_AREA_SIZE * sizeof(UInt16)) / BYTES_PER_PAGE;
    if ((FTL_AREA_SIZE * sizeof(UInt16)) % BYTES_PER_PAGE)
    {
        yaFTL.cxtSizeValidPagesD++;
    }

    yaFTL.cxtSizeValidPagesI = (FTL_AREA_SIZE * sizeof(UInt16)) / BYTES_PER_PAGE;
    if ((FTL_AREA_SIZE * sizeof(UInt16)) % BYTES_PER_PAGE)
    {
        yaFTL.cxtSizeValidPagesI++;
    }

    yaFTL.cxtSize = (yaFTL.cxtSizeTOC + yaFTL.cxtSizeBlockStatus + yaFTL.cxtSizeBlockRead + yaFTL.cxtSizeBlockErase + yaFTL.cxtSizeValidPagesI + yaFTL.cxtSizeValidPagesD) + (2 + 2 * yaFTL.controlPageNo) /* yaftl_controlInfo + currentDTOC + currentITOC + yaftl/vfl/fil stats .each one page */;
#ifndef AND_READONLY
    yaFTL.cxtAllocation = 0;
    yaFTL.cxtEraseCounter = 0;
    yaFTL.cxtValid = 0; yaFTL.currentCxtVpn = 0xffffffff;
#endif // ! AND_READONLY
    WMR_MEMSET(&yaFTL.seaState, 0, sizeof(yaFTL.seaState));
#ifdef AND_COLLECT_STATISTICS
    WMR_MEMSET(&stFTLStatistics, 0, sizeof(FTLStatistics));
#endif /* AND_COLLECT_STATISTICS */

#if ENABLE_L2V_TREE
    L2V_Search_Init(&yaFTL.read_c);
#endif // ENABLE_L2V_TREE

#ifndef AND_READONLY
    if (FTL_SUCCESS != YAFTL_GC_Init())
    {
        goto errorPath;
    }
#endif

    if (!BTOC_Init())
    {
        goto errorPath;
    }

    // Get wrState TOCs
    yaFTL.wrState.data.TOC = BTOC_Alloc(0xfffffffd, 1);
    yaFTL.wrState.index.TOC = BTOC_Alloc(0xfffffffe, 0);

#if ENABLE_L2V_TREE
    if (!L2V_Init(yaFTL.logicalPartitionSize, FTL_AREA_SIZE, PAGES_PER_SUBLK)) {
        return FTL_CRITICAL_ERROR;
    }
#endif // ENABLE_L2V_TREE

    // Success path
    yaftl_init_done = TRUE32;
    
    if( yaFTL.indexSize == 3)
    {
        yaFTL.indexSize++;
        yaFTL.dataSize--;
    }
    
    yaFTL.erasedBlockCount = 0;
    return FTL_SUCCESS;

errorPath:
    YAFTL_FreeMemory();
    return FTL_CRITICAL_ERROR;
}

/*****************************************************************************/
/*                                                                           */
/* NAME                                                                      */
/*      addBlockToList														 */
/* DESCRIPTION                                                               */
/*      This function builds linked list of valid units ( used in restore )  */
/* RETURN VALUES                                                             */
/*		pointer to a new entry ina list                                      */
/*                                                                           */
/*****************************************************************************/
static BlockListType *addBlockToList(BlockListType *listHead, UInt16 Vbn, WeaveSeq_t weaveSeq)
{
    BlockListType *nextBlock;

    nextBlock = (BlockListType *)WMR_MALLOC(sizeof(BlockListType));
    if (nextBlock == NULL)
    {
        debug(ERROR, "cannot allocate %d ", sizeof(BlockListType));
        return NULL;
    }

    nextBlock->next = NULL;
    nextBlock->prev = NULL;
    nextBlock->weaveSeq = weaveSeq;
    nextBlock->blockNo = Vbn;

    if (listHead == NULL)
    {
        nextBlock->next = NULL;
        nextBlock->prev = NULL;
        listHead = nextBlock;
    }
    else
    {
        BlockListType *tmpPtr = listHead;

        while ((tmpPtr->next != NULL) && (tmpPtr->weaveSeq > weaveSeq))
        {
            tmpPtr = tmpPtr->next;
        }
        if (tmpPtr->weaveSeq == weaveSeq)
        {
            if(sizeof(WeaveSeq_t) <= 4)
                WMR_PRINT(QUAL_FATAL, "yaFTL found two blocks (%d, %d) with the same weaveSeq (%d)\n", tmpPtr->blockNo, Vbn, weaveSeq);
            else
                WMR_PRINT(QUAL_FATAL, "yaFTL found two blocks (%d, %d) with the same weaveSeq (%llu)\n", tmpPtr->blockNo, Vbn, weaveSeq);
        }
        if (tmpPtr->weaveSeq <= weaveSeq)
        {
            /* add new node in front of current one */

            nextBlock->next = tmpPtr;
            nextBlock->prev = tmpPtr->prev;
            if (tmpPtr->prev != NULL)
            {
                tmpPtr->prev->next = nextBlock;
            }
            tmpPtr->prev = nextBlock;
        }
        else
        {
            /* add new node as a last node in the list */
            nextBlock->prev = tmpPtr;
            nextBlock->next = NULL;
            tmpPtr->next = nextBlock;
        }
    }
    if (listHead->prev == NULL)
    {
        return listHead;
    }
    else
    {
        return nextBlock;
    }
}

/*****************************************************************************/
/*                                                                           */
/* NAME                                                                      */
/*      restoreIndexBlock													 */
/* DESCRIPTION                                                               */
/*      This function is used in restore                                     */
/*                                                                           */
/*****************************************************************************/
static ANDStatus restoreIndexBlock(UInt16 blockNo, UInt8 isCurrent, WeaveSeq_t listWeaveSequence )
{
    UInt32 Vpn = blockNo * PAGES_PER_SUBLK, *pageBuffer = yaFTL.tmpReadBuffer, i;
    ANDStatus status;
    PageMeta_t *mdPtr = yaFTL.meta_restoreIdx;
    BOOL32 stillClean = TRUE32;
    UInt32 *bTOC = yaFTL.restoreTOC;
    UInt32 upperBound;
    
    if(BTOC_GET_IPN(1) == 1)
    {
        upperBound = yaFTL.TOCtableEntriesNo;
    }
    else 
    {
        upperBound = yaFTL.logicalPartitionSize;
    }
    
    status = _readPage(Vpn, ( UInt8 *)pageBuffer, mdPtr, FALSE32, TRUE32, FALSE32);
    if (!META_IS_IDX(mdPtr) && (status == FTL_SUCCESS)) {
#ifndef AND_READONLY
        WMR_PANIC("yaFTL restoreIndexBlock on a non-index block %d...\n", blockNo);
#else
        WMR_PRINT(ERROR, "yaFTL restoreIndexBlock on a non-index block %d... trying anyway...\n", blockNo);
#endif
        // Give it a shot anyway in read-only versions...
        return FTL_SUCCESS;
    }
    
    status = BTOC_Read(Vpn + (PAGES_PER_SUBLK - yaFTL.controlPageNo), bTOC, mdPtr, FALSE32, FALSE32, upperBound);
    
    if ((status != FTL_SUCCESS))
    {
        yaFTL.blockArray[blockNo].eraseBeforeUse = BLOCK_TO_BE_MOVED;
        if (status != FIL_SUCCESS_CLEAN)
            stillClean = FALSE32;
        debug(INIT, "cannot read page properly");
    }
    if (isCurrent)
    {
        yaFTL.wrState.index.block = blockNo;
        yaFTL.blockArray[blockNo].status = BLOCK_I_CURRENT;
    }
    else
    {
        yaFTL.blockArray[blockNo].status = BLOCK_I_ALLOCATED;
    }
    if (META_IS_BTOC_IDX(mdPtr) && (status == FTL_SUCCESS))
    {
        /* block was fully used and it contains valid TOC page */
        for (i = 0; (Int32)i < (Int32)PAGES_PER_SUBLK - yaFTL.controlPageNo; i++)
        {
            if ((BTOC_Get(bTOC, (UInt16)(PAGES_PER_SUBLK - yaFTL.controlPageNo - i - 1), upperBound) != 0xffffffff) && (yaFTL.tocArray[BTOC_GET_IPN(BTOC_Get(bTOC, (UInt16)(PAGES_PER_SUBLK - yaFTL.controlPageNo - i - 1), upperBound))].indexPage == 0xffffffff))
            {
                yaFTL.tocArray[BTOC_GET_IPN(BTOC_Get(bTOC, (UInt16)(PAGES_PER_SUBLK - yaFTL.controlPageNo - i - 1), upperBound))].indexPage = Vpn + (PAGES_PER_SUBLK - yaFTL.controlPageNo - i - 1);
                yaFTL.blockArray[blockNo].validPagesINo++;
                yaFTL.seaState.index.validPages++;
            }
        }
        if (isCurrent)
        {
            yaFTL.wrState.index.nextPage = PAGES_PER_SUBLK;   /* offset in current block of next free page */
            BTOC_Copy(yaFTL.wrState.index.TOC, bTOC, upperBound);
            if( listWeaveSequence <= META_GET_WEAVESEQ(mdPtr))
            {
                yaFTL.wrState.weaveSeq = WMR_MAX(yaFTL.wrState.weaveSeq, META_GET_WEAVESEQ(mdPtr));
            }
            else
            {
                if(sizeof(WeaveSeq_t) <= 4)
                    debug(ERROR, "restoreIndexBlock weaveSequence is lower than expected 0x%x < 0x%x", META_GET_WEAVESEQ(mdPtr) ,listWeaveSequence);
                else
                    debug(ERROR, "restoreIndexBlock weaveSequence is lower than expected %llu < %llu", META_GET_WEAVESEQ(mdPtr) ,listWeaveSequence);
                yaFTL.wrState.weaveSeq = WMR_MAX(yaFTL.wrState.weaveSeq, listWeaveSequence);
            }
        }
    }
    else
    {
        /* probably current block . perform full body scan */
        if (isCurrent)
        {
            if ((status != FTL_SUCCESS) && (status != FIL_SUCCESS_CLEAN))
            {
                yaFTL.wrState.index.nextPage = PAGES_PER_SUBLK - yaFTL.controlPageNo;
            }
            else
            {
                yaFTL.wrState.index.nextPage = PAGES_PER_SUBLK;
            }
        }
        for (i = 0; (Int32)i < (Int32)PAGES_PER_SUBLK - yaFTL.controlPageNo; i++)
        {
            status = _readPage(Vpn + (PAGES_PER_SUBLK - yaFTL.controlPageNo - i - 1), ( UInt8 *)pageBuffer, mdPtr, FALSE32, TRUE32, FALSE32);
            if (((status != FTL_SUCCESS) && (status != FIL_SUCCESS_CLEAN)) || ((status == FTL_SUCCESS) && ((!META_IS_IDX(mdPtr)) || (META_GET_IPN(mdPtr) >= yaFTL.TOCtableEntriesNo))))
            {
                // vadim : just skip the page
                debug(INIT, "failure during index scan at 0x%x", Vpn + (PAGES_PER_SUBLK - yaFTL.controlPageNo - i - 1));
                stillClean = FALSE32;
                yaFTL.blockArray[blockNo].eraseBeforeUse = BLOCK_TO_BE_MOVED;
                continue;
            }
            
            if (status == FIL_SUCCESS_CLEAN)
            {
                if ((isCurrent) && (stillClean))
                {
                    yaFTL.wrState.index.nextPage = (UInt16)(PAGES_PER_SUBLK - yaFTL.controlPageNo - i - 1);
                }
                continue;
            }
            if(status != FTL_SUCCESS)
            {
#ifndef AND_READONLY                
                WMR_PANIC("no invalid pages beyong this point");
#else
                WMR_PRINT(ERROR,"no invalid pages beyong this point\n");
#endif                
                continue;
            }

            stillClean = FALSE32;
            if (isCurrent)
            {
                BTOC_SET_FROM_IPN(yaFTL.wrState.index.TOC[PAGES_PER_SUBLK - yaFTL.controlPageNo - i - 1], mdPtr);
                if( listWeaveSequence <= META_GET_WEAVESEQ(mdPtr))
                {
                    yaFTL.wrState.weaveSeq = WMR_MAX(yaFTL.wrState.weaveSeq, META_GET_WEAVESEQ(mdPtr));
                }
                else
                {
                    if(sizeof(WeaveSeq_t) <= 4)
                        debug(ERROR, "restoreIndexBlock weaveSequence is lower than expected 0x%x < 0x%x", META_GET_WEAVESEQ(mdPtr) ,listWeaveSequence);
                    else
                        debug(ERROR, "restoreIndexBlock weaveSequence is lower than expected %llu < %llu", META_GET_WEAVESEQ(mdPtr) ,listWeaveSequence);
                    yaFTL.wrState.weaveSeq = WMR_MAX(yaFTL.wrState.weaveSeq, listWeaveSequence);
                }
            }
            if (yaFTL.tocArray[META_GET_IPN(mdPtr)].indexPage == 0xffffffff)
            {
                yaFTL.tocArray[META_GET_IPN(mdPtr)].indexPage = Vpn + (PAGES_PER_SUBLK - yaFTL.controlPageNo - i - 1);
                yaFTL.blockArray[blockNo].validPagesINo++;
                yaFTL.seaState.index.validPages++;
            }
        }
    }
    if (isCurrent)
    {
        debug(INIT, "wrState.index.nextPage  0x%x -- %d  blockNo 0x%x is current", yaFTL.wrState.index.nextPage, yaFTL.wrState.index.nextPage, blockNo);
    }
    
    return FTL_SUCCESS;
}

/*****************************************************************************/
/*                                                                           */
/* NAME                                                                      */
/*      restoreBlock														 */
/* DESCRIPTION                                                               */
/*      This function is used in restore                                     */
/*                                                                           */
/*****************************************************************************/
static ANDStatus restoreBlock(UInt16 blockNo, UInt32 *indexTable, UInt8 isCurrent, UInt32 rangeOffset, UInt32 rangeSize, BlockRangeType *brPtr, WeaveSeq_t listWeaveSequence)
{
    UInt32 Vpn = blockNo * PAGES_PER_SUBLK, *pageBuffer = yaFTL.tmpReadBuffer, i, lastPageToCheck;
    ANDStatus status = FTL_SUCCESS;
    PageMeta_t *mdPtr = yaFTL.meta_restoreData;
    BOOL32 stillClean = TRUE32;
    UInt32 *bTOC = yaFTL.restoreTOC;
    
    if ((rangeOffset) && (brPtr != NULL))
    {
        if ((brPtr[blockNo].end < rangeOffset) || (brPtr[blockNo].start >= (rangeOffset + rangeSize)))
        {
            return FTL_SUCCESS;
        }
    }
    
    if (yaFTL.blockArray[blockNo].validPagesINo == (PAGES_PER_SUBLK - yaFTL.controlPageNo))
    {
        return FTL_SUCCESS;
    }
    
    if ((rangeOffset > 0) && (isCurrent))
    {
        WMR_MEMSET(&yaFTL.wrState.data.TOC[yaFTL.wrState.data.nextPage], 0xff, (PAGES_PER_SUBLK - yaFTL.wrState.data.nextPage) * sizeof(UInt32));
        BTOC_Copy(bTOC, yaFTL.wrState.data.TOC, yaFTL.logicalPartitionSize);
        META_SET_BTOC_DATA(mdPtr);
    }
    else
    {
        status = BTOC_Read(Vpn + (PAGES_PER_SUBLK - yaFTL.controlPageNo), bTOC, mdPtr, FALSE32, FALSE32, yaFTL.logicalPartitionSize);
        if ((status != FTL_SUCCESS))
        {
            yaFTL.blockArray[blockNo].eraseBeforeUse = BLOCK_TO_BE_MOVED;
            if (status != FIL_SUCCESS_CLEAN)
                stillClean = FALSE32;
            debug(INIT, "cannot read block TOC during scan");
            //return status;
        }
    }
    if (rangeOffset == 0)
    {
        if (isCurrent)
        {
            yaFTL.wrState.data.block = blockNo;
            yaFTL.blockArray[blockNo].status = BLOCK_CURRENT;
        }
        else
        {
            yaFTL.blockArray[blockNo].status = BLOCK_ALLOCATED;
        }
    }
    
    if ((META_IS_BTOC_DATA(mdPtr)) && (status == FTL_SUCCESS))
    {
        /* block was fully used and it contains valid TOC page */
        long length;
        if ((rangeOffset > 0) && (isCurrent))
        {
            length = yaFTL.wrState.data.nextPage;
        }
        else
        {
            length = (Int32)PAGES_PER_SUBLK - yaFTL.controlPageNo;
        }
        
        for (i = 0; (Int32)i < (Int32)length; i++)
        {
            // check the range before executing restore
            if ((!rangeOffset) && (brPtr != NULL))
            {
                if ((brPtr[blockNo].end == brPtr[blockNo].start) && (brPtr[blockNo].start == 0xffffffff))
                {
                    brPtr[blockNo].end = brPtr[blockNo].start = BTOC_Get(bTOC, (UInt16)(length - (i + 1)), yaFTL.logicalPartitionSize);
                }
                else
                {
                    if (brPtr[blockNo].end < BTOC_Get(bTOC, (UInt16)(length - (i + 1)), yaFTL.logicalPartitionSize))
                    {
                        brPtr[blockNo].end = BTOC_Get(bTOC, (UInt16)(length - (i + 1)), yaFTL.logicalPartitionSize);
                    }
                    
                    if (brPtr[blockNo].start > BTOC_Get(bTOC, (UInt16)(length - (i + 1)), yaFTL.logicalPartitionSize))
                    {
                        brPtr[blockNo].start = BTOC_Get(bTOC, (UInt16)(length - (i + 1)), yaFTL.logicalPartitionSize);
                    }
                }
            }
            
            if ((BTOC_Get(bTOC, (UInt16)(length - (i + 1)), yaFTL.logicalPartitionSize) < rangeOffset) ||
                (BTOC_Get(bTOC, (UInt16)(length - (i + 1)), yaFTL.logicalPartitionSize) >= (rangeOffset + rangeSize)))
            {
                continue;
            }
            yaFTL.blockArray[blockNo].validPagesINo++;
            if ((BTOC_Get(bTOC, (UInt16)(length - (i + 1)), yaFTL.logicalPartitionSize) != 0xffffffff) && (indexTable[BTOC_Get(bTOC, (UInt16)(length - (i + 1)), yaFTL.logicalPartitionSize) - rangeOffset] == 0xffffffff))
            {
                indexTable[BTOC_Get(bTOC, (UInt16)(length - (i + 1)), yaFTL.logicalPartitionSize) - rangeOffset] = Vpn + (length - (i + 1));
                yaFTL.blockArray[blockNo].validPagesDNo++;
                yaFTL.seaState.data.validPages++;
            }
        }
        if ((isCurrent) && (!rangeOffset))
        {
            yaFTL.wrState.data.nextPage = PAGES_PER_SUBLK;   /* offset in current block of next free page */
            BTOC_Copy(yaFTL.wrState.data.TOC, bTOC, yaFTL.logicalPartitionSize);
#ifndef AND_READONLY
            if( listWeaveSequence <= META_GET_WEAVESEQ(mdPtr))
            {
                yaFTL.wrState.weaveSeq = WMR_MAX(yaFTL.wrState.weaveSeq, META_GET_WEAVESEQ(mdPtr));
            }
            else
            {
                if(sizeof(WeaveSeq_t) <= 4)
                    debug(ERROR, "restoreBlock weaveSequence is lower than expected 0x%x < 0x%x", META_GET_WEAVESEQ(mdPtr) ,listWeaveSequence);
                else
                    debug(ERROR, "restoreBlock weaveSequence is lower than expected %llu < %llu", META_GET_WEAVESEQ(mdPtr) ,listWeaveSequence);
                yaFTL.wrState.weaveSeq = WMR_MAX(yaFTL.wrState.weaveSeq, listWeaveSequence);
            }
#endif
        }
    }
    else
    {
        /* probably current block . perform full body scan */
        lastPageToCheck = PAGES_PER_SUBLK - yaFTL.controlPageNo;
        if (isCurrent)
        {
            if (status == FTL_SUCCESS)
            {
                yaFTL.wrState.data.nextPage = PAGES_PER_SUBLK - yaFTL.controlPageNo;
            }
            else
            {
                yaFTL.wrState.data.nextPage = PAGES_PER_SUBLK;
            }
            yaFTL.blockArray[blockNo].validPagesINo = PAGES_PER_SUBLK - yaFTL.controlPageNo;
            for (i = 0; (Int32)i < (Int32)PAGES_PER_SUBLK - yaFTL.controlPageNo; i++)
            {
                status = _readPage(Vpn + i, ( UInt8 *)pageBuffer, mdPtr, FALSE32, TRUE32, FALSE32);
                if ((status != FTL_SUCCESS) || (!META_IS_DATA(mdPtr)) || (META_GET_LBA(mdPtr) >= yaFTL.logicalPartitionSize))
                {
                    lastPageToCheck = i;
                    yaFTL.wrState.data.nextPage = i;
                    break;
                }
                else
                {
                    yaFTL.blockArray[blockNo].validPagesINo--;
                }
                
            }
        }
        
        
        for (i = 0; (Int32)i < (Int32)lastPageToCheck; i++)
        {
            status = _readPage(Vpn + (lastPageToCheck - i - 1), ( UInt8 *)pageBuffer, mdPtr, FALSE32, TRUE32, FALSE32);
            if (((status != FTL_SUCCESS) && (status != FIL_SUCCESS_CLEAN)) || ((status == FTL_SUCCESS) && ((!META_IS_DATA(mdPtr)) || (META_GET_LBA(mdPtr) >= yaFTL.logicalPartitionSize))))
            {
                // vadim
                // just ignore page and continue ( power failure during update
                if (!rangeOffset)
                {
                    yaFTL.blockArray[blockNo].validPagesINo++;
                }
                yaFTL.blockArray[blockNo].eraseBeforeUse = BLOCK_TO_BE_MOVED;
                stillClean = FALSE32;
                continue;
                //return status;
            }
            
            if ((status == FIL_SUCCESS_CLEAN) && (!rangeOffset))
            {
                yaFTL.blockArray[blockNo].validPagesINo++;
            }
            
            if ((isCurrent) && (status == FIL_SUCCESS_CLEAN) && (stillClean))
            {
                yaFTL.wrState.data.nextPage = (UInt16)(lastPageToCheck - i - 1);
                continue;
            }
            if (status == FIL_SUCCESS_CLEAN)
                continue;
            if(status != FTL_SUCCESS)
            {
#ifndef AND_READONLY                
                WMR_PANIC("no invalid pages beyong this point");
#else
                WMR_PRINT(ERROR,"no invalid pages beyong this point\n");
#endif                
                continue;
            }

            stillClean = FALSE32;
            if ((brPtr != NULL) && (!rangeOffset) && (META_GET_LBA(mdPtr) < yaFTL.logicalPartitionSize))
            {
                if ((brPtr[blockNo].end == brPtr[blockNo].start) && (brPtr[blockNo].start == 0xffffffff))
                {
                    brPtr[blockNo].end = brPtr[blockNo].start = META_GET_LBA(mdPtr);
                }
                else
                {
                    if (brPtr[blockNo].end < META_GET_LBA(mdPtr))
                    {
                        brPtr[blockNo].end = META_GET_LBA(mdPtr);
                    }
                    
                    if (brPtr[blockNo].start > META_GET_LBA(mdPtr))
                    {
                        brPtr[blockNo].start = META_GET_LBA(mdPtr);
                    }
                }
            }
            
            if (isCurrent)
            {
                yaFTL.wrState.data.TOC[lastPageToCheck - i - 1] = META_GET_LBA(mdPtr);
#ifndef AND_READONLY
                if( listWeaveSequence <= META_GET_WEAVESEQ(mdPtr))
                {
                    yaFTL.wrState.weaveSeq = WMR_MAX(yaFTL.wrState.weaveSeq, META_GET_WEAVESEQ(mdPtr));
                }
                else
                {
                    if(sizeof(WeaveSeq_t) <= 4)
                        debug(ERROR, "restoreBlock weaveSequence is lower than expected 0x%x < 0x%x", META_GET_WEAVESEQ(mdPtr) ,listWeaveSequence);
                    else
                        debug(ERROR, "restoreBlock weaveSequence is lower than expected %llu < %llu", META_GET_WEAVESEQ(mdPtr) ,listWeaveSequence);
                    yaFTL.wrState.weaveSeq = WMR_MAX(yaFTL.wrState.weaveSeq, listWeaveSequence);
                }
#endif // ! AND_READONLY
            }
            if ((META_GET_LBA(mdPtr) < rangeOffset) ||
                (META_GET_LBA(mdPtr) >= (rangeOffset + rangeSize)))
            {
                continue;
            }
            // this to count all pages we already scanned . Once this number reaches blockszie we can ignore block in all subsequent iterations
            yaFTL.blockArray[blockNo].validPagesINo++;
            
            if (indexTable[META_GET_LBA(mdPtr) - rangeOffset] == 0xffffffff)
            {
                indexTable[META_GET_LBA(mdPtr) - rangeOffset] = Vpn + (lastPageToCheck - i - 1);
                yaFTL.blockArray[blockNo].validPagesDNo++;
                yaFTL.seaState.data.validPages++;
            }
        }
    }
    
    return FTL_SUCCESS;
}

static ANDStatus readCXTInfo(UInt32 vpn, UInt8 *tBuff, UInt8 qm_valid, BOOL32 *formatSupported)
{
    UInt32 i = 0, size, k, j;
    ANDStatus status;
    PageMeta_t *md = yaFTL.meta_readCxt;
    UInt8 *statPtr = NULL;
    UInt16 *validPtr = NULL;
    UInt32 *tocPtr = NULL;
    struct YAFTLControlInfo *yaftlCI;
    UInt32 tmpIndexSize = 0;

    debug(INIT, "readCXTInfo from offset 0x%x", vpn);
    
    status = _readPage(vpn, tBuff, md, FALSE32, TRUE32, FALSE32);
    if ((status != FTL_SUCCESS) || !META_IS_CXT(md))
    {
        return FTL_CRITICAL_ERROR;
    }
    yaftlCI = (struct YAFTLControlInfo *)tBuff;
    if (WMR_MEMCMP(&(yaftlCI->versionNo), CXT_VER, 4) != 0)

    {
        if (formatSupported != NULL)
        {
            *formatSupported = FALSE32;
        }
        debug(ERROR, "wrong version of CXT %s expected %s", (UInt8 *)(&yaftlCI->versionNo), CXT_VER);

        return FTL_CRITICAL_ERROR;
    }
#ifndef AND_READONLY    
    if(sizeof(WeaveSeq_t) <= 4)
    {
        if (META_GET_WEAVESEQ(md) != 0xffffffff)
        {
            yaFTL.cxtAllocation = META_GET_WEAVESEQ(md);
        }    
    }
    else
    {
        if (META_GET_WEAVESEQ(md) != (WeaveSeq_t)0xffffffffffffULL)
        {
            yaFTL.cxtAllocation = META_GET_WEAVESEQ(md);
        }    
    }    
#endif
    if( yaFTL.indexSize == 4)
        tmpIndexSize = 3;
    else
        tmpIndexSize = yaFTL.indexSize;
    
    if (!((yaFTL.logicalPartitionSize == yaftlCI->logicalPartitionSize) && 
          (tmpIndexSize == yaftlCI->indexSize) && (yaFTL.controlPageNo == yaftlCI->controlPageNo) && 
          (yaFTL.indexPageRatio == yaftlCI->indexPageRatio) && (yaFTL.TOCtableEntriesNo == yaftlCI->TOCtableEntriesNo) && 
          (yaFTL.exportedRatio == yaftlCI->exportedRatio)))
    {
        debug(INIT, "CXT control info mismatch . Getting out");
        return FTL_CRITICAL_ERROR;
    }
        
    if (qm_valid)
    {
        debug(INIT, "readCXTInfo from offset 0x%x", vpn);
        yaFTL.freeCachePages = yaftlCI->freeCachePages;
        yaFTL.wrState.data.block = yaftlCI->wrState_data_block;
        yaFTL.wrState.data.nextPage = yaftlCI->wrState_data_nextPage;
        yaFTL.seaState.data.freeBlocks = yaftlCI->seaState_data_freeBlocks;
        yaFTL.wrState.index.block = yaftlCI->wrState_index_block;
#if NAND_PPN
        yaFTL.wrState.weaveSeq = (WeaveSeq_t)yaftlCI->wrState_index_weaveSeq_Lo | ((WeaveSeq_t)yaftlCI->wrState_index_weaveSeq_Hi << 32UL);
#elif NAND_RAW
        yaFTL.wrState.weaveSeq = yaftlCI->wrState_index_weaveSeq;
#else
#error PPN or raw?
#endif
        yaFTL.wrState.index.nextPage = yaftlCI->wrState_index_nextPage;
        yaFTL.seaState.index.freeBlocks = yaftlCI->seaState_index_freeBlocks;
        yaFTL.seaState.data.allocdBlocks = yaftlCI->seaState_data_allocdBlocks;
        yaFTL.seaState.index.allocdBlocks = yaftlCI->seaState_index_allocdBlocks;
#ifndef AND_READONLY
        yaFTL.nextFreeCachePage = yaftlCI->nextFreeCachePage;
#if NAND_PPN
        yaFTL.wrState.weaveSeq = WMR_MAX(yaFTL.wrState.weaveSeq, (WeaveSeq_t)yaftlCI->wrState_data_weaveSeq_Lo | ((WeaveSeq_t)yaftlCI->wrState_data_weaveSeq_Hi << 32UL));
#elif NAND_RAW
        yaFTL.wrState.weaveSeq = WMR_MAX(yaFTL.wrState.weaveSeq, yaftlCI->wrState_data_weaveSeq);
#else
#error PPN or raw?
#endif
        yaFTL.cxtEraseCounter = yaftlCI->cxtEraseCounter;
        yaFTL.FTLRestoreCnt = yaftlCI->FTLRestoreCnt;
#endif
        yaFTL.indexCacheSize = yaftlCI->indexCacheSize;
        yaFTL.exportedRatio = yaftlCI->exportedRatio;
        
        debug(INIT, "readCXTInfo logicalPartitionSize %d  TOCtableEntriesNo %d \nwrState.data.block %d wrState.data.nextPage %d \n BYTES_PER_PAGE %d \n wrState.index.block %d wrState.index.nextPage %d wrState.weaveSeq %d"
              , yaFTL.logicalPartitionSize, yaFTL.TOCtableEntriesNo, yaFTL.wrState.data.block, yaFTL.wrState.data.nextPage, BYTES_PER_PAGE, yaFTL.wrState.index.block, yaFTL.wrState.index.nextPage, yaFTL.wrState.weaveSeq);
        debug(INIT, "readCXTInfo cxtSizeTOC %d  cxtSizeBlockStatus %d \ncxtSizeBlockRead %d cxtSizeBlockErase %d\n cxtSizeValidPagesI %d cxtSizeValidPagesD %d ",
              yaFTL.cxtSizeTOC, yaFTL.cxtSizeBlockStatus, yaFTL.cxtSizeBlockRead, yaFTL.cxtSizeBlockErase, yaFTL.cxtSizeValidPagesI, yaFTL.cxtSizeValidPagesD);
    }
    vpn++;

    if (qm_valid)
    {
        UInt8 i;
        for (i = 0; i < yaFTL.controlPageNo; i++)
        {
            status = _readPage(vpn + i, ( UInt8 *)((UInt8 *)yaFTL.wrState.data.TOC + (BYTES_PER_PAGE * i)), md, FALSE32, TRUE32, FALSE32);
            if ((status != FTL_SUCCESS) || (!META_IS_CXT(md)))
            {
#ifndef AND_READONLY
                yaFTL.cxtValid = 0;
#endif

                return FTL_CRITICAL_ERROR;
            }
        }
    }
    vpn += yaFTL.controlPageNo;
    if (qm_valid)
    {
        UInt8 i;
        for (i = 0; i < yaFTL.controlPageNo; i++)
        {
            status = _readPage(vpn + i, ( UInt8 *)((UInt8 *)yaFTL.wrState.index.TOC + (BYTES_PER_PAGE * i)), md, FALSE32, TRUE32, FALSE32);
            if ((status != FTL_SUCCESS) || !META_IS_CXT(md))
            {
#ifndef AND_READONLY
                yaFTL.cxtValid = 0;
#endif

                return FTL_CRITICAL_ERROR;
            }
        }
    }
    vpn += yaFTL.controlPageNo;

#ifdef AND_COLLECT_STATISTICS
    /* set statistics */
    status = _readPage(vpn + i, tBuff, md, FALSE32, TRUE32, FALSE32);
    if ((status != FTL_SUCCESS) || !META_IS_CXT(md))
    {
#ifndef AND_READONLY
        yaFTL.cxtValid = 0;
#endif

        return FTL_CRITICAL_ERROR;
    }
    _FTLSetStatisticsFromCxt(tBuff);
#endif
    vpn++;

    if (qm_valid)
    {
        size = yaFTL.TOCtableEntriesNo;
        for (i = 0, j = 0; i < yaFTL.cxtSizeTOC; i++, size -= BYTES_PER_PAGE / sizeof(UInt32), j += BYTES_PER_PAGE / sizeof(UInt32))
        {
            status = _readPage(vpn + i, tBuff, md, FALSE32, TRUE32, FALSE32);
            if ((status != FTL_SUCCESS) || !META_IS_CXT(md))
            {
#ifndef AND_READONLY
                yaFTL.cxtValid = 0;
#endif

                return FTL_CRITICAL_ERROR;
            }
            tocPtr = (UInt32 *)tBuff;
            for (k = 0; (k < BYTES_PER_PAGE / sizeof(UInt32)) && (k < size); k++)
            {
                yaFTL.tocArray[j + k].indexPage = tocPtr[k];
            }
        }
    }
    debug(INIT, " readCXTInfo after TOC table i = %d", i);
    vpn += yaFTL.cxtSizeTOC;
    if (qm_valid)
    {
        size = FTL_AREA_SIZE;
        for (i = 0, j = 0; i < yaFTL.cxtSizeBlockStatus; i++, size -= BYTES_PER_PAGE / sizeof(UInt8), j += BYTES_PER_PAGE / sizeof(UInt8))
        {
            status = _readPage(vpn + i, tBuff, md, FALSE32, TRUE32, FALSE32);
            if ((status != FTL_SUCCESS) || !META_IS_CXT(md))
            {
#ifndef AND_READONLY
                yaFTL.cxtValid = 0;
#endif

                return FTL_CRITICAL_ERROR;
            }
            statPtr = (UInt8 *)tBuff;
            for (k = 0; (k < BYTES_PER_PAGE / sizeof(UInt8)) && (k < size); k++)
            {
                yaFTL.blockArray[j + k].status = statPtr[k];
            }
        }
    }
    debug(INIT, "readCXTInfo after blockstatus i = %d", i);
    vpn += yaFTL.cxtSizeBlockStatus;

    size = FTL_AREA_SIZE;
    for (i = 0, j = 0; i < yaFTL.cxtSizeBlockRead; i++, size -= BYTES_PER_PAGE / sizeof(UInt16), j += BYTES_PER_PAGE / sizeof(UInt16))
    {
        status = _readPage(vpn + i, tBuff, md, FALSE32, TRUE32, FALSE32);
        if ((status != FTL_SUCCESS) || !META_IS_CXT(md))
        {
#ifndef AND_READONLY
            yaFTL.cxtValid = 0;
#endif

            return FTL_CRITICAL_ERROR;
        }
        validPtr = (UInt16 *)tBuff;
        for (k = 0; (k < BYTES_PER_PAGE / sizeof(UInt16)) && (k < size); k++)
        {
            yaFTL.blockArray[j + k].pagesRead = validPtr[k];
        }
    }
    debug(INIT, "readCXTInfo after blockread i = %d", i);
    vpn += yaFTL.cxtSizeBlockRead;

    size = FTL_AREA_SIZE;
    for (i = 0, j = 0; i < yaFTL.cxtSizeBlockErase; i++, size -= BYTES_PER_PAGE / sizeof(UInt32), j += BYTES_PER_PAGE / sizeof(UInt32))
    {
        status = _readPage(vpn + i, tBuff, md, FALSE32, TRUE32, FALSE32);
        if ((status != FTL_SUCCESS) || !META_IS_CXT(md))
        {
#ifndef AND_READONLY
            yaFTL.cxtValid = 0;
#endif

            return FTL_CRITICAL_ERROR;
        }
        tocPtr = (UInt32 *)tBuff;
        for (k = 0; (k < BYTES_PER_PAGE / sizeof(UInt32)) && (k < size); k++)
        {
            yaFTL.blockArray[j + k].erasableCount = tocPtr[k];
        }
    }
    debug(INIT, "readCXTInfo after blockerase i = %d", i);
    vpn += yaFTL.cxtSizeBlockErase;

    if (qm_valid)
    {
        size = FTL_AREA_SIZE;
        for (i = 0, j = 0; i < yaFTL.cxtSizeValidPagesI; i++, size -= BYTES_PER_PAGE / sizeof(UInt16), j += BYTES_PER_PAGE / sizeof(UInt16))
        {
            status = _readPage(vpn + i, tBuff, md, FALSE32, TRUE32, FALSE32);
            if ((status != FTL_SUCCESS) || !META_IS_CXT(md))
            {
#ifndef AND_READONLY
                yaFTL.cxtValid = 0;
#endif

                return FTL_CRITICAL_ERROR;
            }
            validPtr = (UInt16 *)tBuff;
            for (k = 0; (k < BYTES_PER_PAGE / sizeof(UInt16)) && (k < size); k++)
            {
                yaFTL.blockArray[j + k].validPagesINo = validPtr[k];
                yaFTL.seaState.index.validPages += yaFTL.blockArray[j + k].validPagesINo;
            }
        }
    }
    debug(INIT, "readCXTInfo after validpagesI i = %d", i);
    vpn += yaFTL.cxtSizeValidPagesI;

    if (qm_valid)
    {
        size = FTL_AREA_SIZE;
        for (i = 0, j = 0; i < yaFTL.cxtSizeValidPagesD; i++, size -= BYTES_PER_PAGE / sizeof(UInt16), j += BYTES_PER_PAGE / sizeof(UInt16))
        {
            status = _readPage(vpn + i, tBuff, md, FALSE32, TRUE32, FALSE32);
            if ((status != FTL_SUCCESS) || !META_IS_CXT(md))
            {
#ifndef AND_READONLY
                yaFTL.cxtValid = 0;
#endif

                return FTL_CRITICAL_ERROR;
            }
            validPtr = (UInt16 *)tBuff;
            for (k = 0; (k < BYTES_PER_PAGE / sizeof(UInt16)) && (k < size); k++)
            {
                yaFTL.blockArray[j + k].validPagesDNo = validPtr[k];
                yaFTL.seaState.data.validPages += yaFTL.blockArray[j + k].validPagesDNo;
            }
        }
    }
    debug(INIT, "readCXTInfo after validPagesD i = %d", i);
#ifdef AND_COLLECT_STATISTICS
    if (qm_valid)
    {
        stFTLStatistics.ddwValidIndexPages = yaFTL.seaState.index.validPages;
        stFTLStatistics.ddwValidDataPages = yaFTL.seaState.data.validPages;
    }
#endif

    return FTL_SUCCESS;
}

static ANDStatus quickRestore(UInt16 *cxtT, BOOL32 * formatSupported)
{
    UInt16 i;
    ANDStatus status;
    PageMeta_t *md = yaFTL.meta_quickRestore;
    WeaveSeq_t lastCX = 0;
    UInt32 currentCX = 0;

    /*
       Read first page out of each Cxt unit to establish which contains a latest valid ( hopefully )
       copy of a quick mount info . Then read last copy and establish its validity . If it's valid read out the whole
       thing and return success . If it's not valid use erase/read count info and return failure which would trigger full restore
     */

    if (yaFTL.quickRestore_tBuff == NULL)
    {
        return FTL_CRITICAL_ERROR;
    }
    
    for (i = 0; i < FTL_CXT_SECTION_SIZE; i++)
    {
        yaFTL.blockArray[cxtT[i]].status = BLOCK_CTX_CNTR;
    }

    for (i = 0; i < FTL_CXT_SECTION_SIZE; i++)
    {
        status = _readPage(cxtT[i] * PAGES_PER_SUBLK, yaFTL.quickRestore_tBuff, md, FALSE32, TRUE32, FALSE32);
        if (status != FTL_SUCCESS)
        {
#ifndef AND_READONLY
            if (status != FIL_SUCCESS_CLEAN)
            {
                VFL_Erase(cxtT[i], TRUE32);
                yaFTL.blockArray[cxtT[i]].erasableCount++;
                removeBlockFromEraseNowList(cxtT[i]);
            }
#endif
            continue;
        }
        
        if(sizeof(WeaveSeq_t) <= 4)
        {        
            if ((META_GET_WEAVESEQ(md) != 0xffffffff) && (lastCX < META_GET_WEAVESEQ(md)))
            {
                lastCX = META_GET_WEAVESEQ(md);
                currentCX = cxtT[i];
#ifndef AND_READONLY
                yaFTL.currentCXTIndex = i;
#endif // ! AND_READONLY
            }
        }
        else
        {        
            if ((META_GET_WEAVESEQ(md) != (WeaveSeq_t)0xffffffffffffULL) && (lastCX < META_GET_WEAVESEQ(md)))
            {
                lastCX = META_GET_WEAVESEQ(md);
                currentCX = cxtT[i];
#ifndef AND_READONLY
                yaFTL.currentCXTIndex = i;
#endif // ! AND_READONLY
            }
        }
    }
    if (lastCX != 0)
    {
        yaFTL.blockArray[currentCX].status = BLOCK_CTX_CURRENT;
        debug(INIT, "currentCX is %d", currentCX);
    }
    else
    {
        /* no quick mount info */
        yaFTL.blockArray[cxtT[0]].status = BLOCK_CTX_CURRENT;
#ifndef AND_READONLY
        yaFTL.currentCXTIndex = 0;
#endif // ! AND_READONLY
        return 5;
    }

    /* check validity */
    for (i = 0; (i + (UInt16)yaFTL.cxtSize) < PAGES_PER_SUBLK; i += (UInt16)(yaFTL.cxtSize + 1))
    {
#ifndef AND_READONLY
        yaFTL.cxtAllocation = META_GET_WEAVESEQ(md);
#endif
        status = _readPage((currentCX * PAGES_PER_SUBLK) + i, yaFTL.quickRestore_tBuff, md, FALSE32, TRUE32, FALSE32); // read first page of a potential cxt
        if (status != FIL_SUCCESS)
        {
            // previous copy was the last one . no valid cxt avaiable
            // we still read blockerase and blockread counts and then let full restore to kick in
            break;
        }

        status = _readPage((currentCX * PAGES_PER_SUBLK) + i + yaFTL.cxtSize, yaFTL.quickRestore_tBuff, md, FALSE32, TRUE32, FALSE32);
        if (status == FIL_SUCCESS_CLEAN)
        {
            yaFTL.currentCxtVpn = currentCX * PAGES_PER_SUBLK + i;

            if (readCXTInfo(currentCX * PAGES_PER_SUBLK + i, yaFTL.quickRestore_tBuff, 1, formatSupported) == FTL_SUCCESS)
            {
                debug(INIT, "quickRestore : seaState.index.validPages = %d seaState.data.validPages = %d", yaFTL.seaState.index.validPages, yaFTL.seaState.data.validPages);
#ifndef AND_READONLY
                yaFTL.cxtValid = 1;
#endif // ! AND_READONLY
                return FTL_SUCCESS;
            }
            else
            {
                debug(INIT, "quickRestore : something is not quite right with it . going for a full one");
#ifndef AND_READONLY
                yaFTL.cxtValid = 0;
#endif // ! AND_READONLY
                return 5;  // TODO: find a cleaner way to mark a failure
            }
        }
    }
    // previous copy was the last one . no valid cxt avaiable
    // we still read blockerase and blockread counts and then let full restore to kick in
    yaFTL.currentCxtVpn = currentCX * PAGES_PER_SUBLK + i - (yaFTL.cxtSize + 1); // back off to the last invalid copy of cxt
#ifndef AND_READONLY
    if(sizeof(WeaveSeq_t) <= 4)
    {
        if (META_GET_WEAVESEQ(md) != 0xffffffff)
        {
            yaFTL.cxtAllocation = META_GET_WEAVESEQ(md);
        }
    }
    else
    {
        if (META_GET_WEAVESEQ(md) != (WeaveSeq_t)0xffffffffffffULL)
        {
            yaFTL.cxtAllocation = META_GET_WEAVESEQ(md);
        }
    }
    yaFTL.cxtValid = 0;
#endif // ! AND_READONLY

    debug(INIT, "cannot find valid copy of cxt ... reading only relevant info from %d", yaFTL.currentCxtVpn);
    readCXTInfo(yaFTL.currentCxtVpn, yaFTL.quickRestore_tBuff, 0, formatSupported);    //only need to read out erase and read counters
    return 5;
}

#ifndef AND_READONLY
ANDStatus invalidateCXT(void)
{
    PageMeta_t *mdPtr = yaFTL.meta_invalidateCxt;
    ANDStatus status;
    Buffer buff;

    buff.pData = (UInt8 *)yaFTL.wrState.index.TOC;
    buff.pSpare = (UInt8 *)mdPtr;
  
    SetupMeta_Cxt(mdPtr, yaFTL.cxtAllocation);
    status = VFL_Write(yaFTL.currentCxtVpn + yaFTL.cxtSize, &buff, TRUE32, FALSE32);
    if (status != VFL_SUCCESS)
    // erase current cxt
    {
        VFL_Erase((UInt16)yaFTL.cxtTable[yaFTL.currentCXTIndex], TRUE32);
        removeBlockFromEraseNowList((UInt16)yaFTL.cxtTable[yaFTL.currentCXTIndex]);
        yaFTL.blockArray[(UInt16)yaFTL.cxtTable[yaFTL.currentCXTIndex]].erasableCount++;
        yaFTL.cxtEraseCounter++;
        yaFTL.blockArray[yaFTL.cxtTable[yaFTL.currentCXTIndex]].status = BLOCK_CTX_CNTR;
        yaFTL.currentCXTIndex++;
        if (yaFTL.currentCXTIndex >= FTL_CXT_SECTION_SIZE)
        {
            yaFTL.currentCXTIndex = 0;
        }
        yaFTL.blockArray[yaFTL.cxtTable[yaFTL.currentCXTIndex]].status = BLOCK_CTX_CURRENT;
        /* todo : add erase in case this is not the first time
           todo : check if we need to erase ( no need if this is the first time we use this unit ) */
        VFL_Erase((UInt16)yaFTL.cxtTable[yaFTL.currentCXTIndex], TRUE32);
        removeBlockFromEraseNowList((UInt16)yaFTL.cxtTable[yaFTL.currentCXTIndex]);
        yaFTL.blockArray[(UInt16)yaFTL.cxtTable[yaFTL.currentCXTIndex]].erasableCount++;
        yaFTL.cxtEraseCounter++;
        yaFTL.currentCxtVpn = 0xffffffff;
        //return status;
    }

    return FTL_SUCCESS;
}
#endif

#ifndef AND_READONLY
ANDStatus writeCXTInfo(UInt32 vpn, UInt8* tBuff, BOOL32 statsOnly)
{
    UInt32 i, size, k, j;
    PageMeta_t *md = yaFTL.meta_writeCxt;
    UInt8 *statPtr = NULL;
    UInt16 * validPtr = NULL;
    UInt32 *tocPtr = NULL;
    UInt32 tempVpn = vpn;
    ANDStatus status;
    struct YAFTLControlInfo *yaftlCI = (struct YAFTLControlInfo *)tBuff;

    if (yaFTL.cxtValid == 1)
    {
        return FTL_SUCCESS;
    }

    yaFTL.cxtAllocation++;
    SetupMeta_Cxt(md, yaFTL.cxtAllocation);

    /* Read yaftl control info first also currentDTOC and currentITOC = 3 pages*/
    WMR_MEMCPY(yaftlCI, CXT_VER, 4);    // update cxt version info
    yaftlCI->controlPageNo = yaFTL.controlPageNo;
    yaftlCI->indexPageRatio = yaFTL.indexPageRatio;
    if( yaFTL.indexSize == 4)
        yaftlCI->indexSize = 3;
    else
        yaftlCI->indexSize = yaFTL.indexSize;
    yaftlCI->logicalPartitionSize = yaFTL.logicalPartitionSize;
    yaftlCI->TOCtableEntriesNo = yaFTL.TOCtableEntriesNo;
    yaftlCI->freeCachePages = yaFTL.freeCachePages;
    yaftlCI->nextFreeCachePage = yaFTL.nextFreeCachePage;
    yaftlCI->wrState_data_block = yaFTL.wrState.data.block;
#if NAND_PPN
    yaftlCI->wrState_data_weaveSeq_Lo = (UInt32)yaFTL.wrState.weaveSeq & 0xFFFFFFFFUL;
    yaftlCI->wrState_data_weaveSeq_Hi = (UInt32)(yaFTL.wrState.weaveSeq >> 32ul) & 0xFFFFFFFFUL;
#elif NAND_RAW
    yaftlCI->wrState_data_weaveSeq = (WeaveSeq_t)yaFTL.wrState.weaveSeq;
#else
#error PPN or raw?
#endif

    yaftlCI->wrState_data_nextPage = yaFTL.wrState.data.nextPage;
    yaftlCI->seaState_data_freeBlocks = yaFTL.seaState.data.freeBlocks;
    yaftlCI->wrState_index_block = yaFTL.wrState.index.block;
#if NAND_PPN
    yaftlCI->wrState_index_weaveSeq_Lo = (UInt32)yaFTL.wrState.weaveSeq & 0xFFFFFFFFUL;
    yaftlCI->wrState_index_weaveSeq_Hi = (UInt32)(yaFTL.wrState.weaveSeq >> 32UL) & 0xFFFFFFFFUL;    
#elif NAND_RAW
    yaftlCI->wrState_index_weaveSeq = (WeaveSeq_t)yaFTL.wrState.weaveSeq;
#else
#error PPN or raw?
#endif
    yaftlCI->wrState_index_nextPage = yaFTL.wrState.index.nextPage;
    yaftlCI->seaState_index_freeBlocks = yaFTL.seaState.index.freeBlocks;
    yaftlCI->seaState_data_allocdBlocks = yaFTL.seaState.data.allocdBlocks;
    yaftlCI->seaState_index_allocdBlocks = yaFTL.seaState.index.allocdBlocks;
    yaftlCI->indexCacheSize = DEFAULT_INDEX_CACHE_SIZE; // use default size for backward compatibility
    yaftlCI->exportedRatio = yaFTL.exportedRatio;
    yaftlCI->cxtEraseCounter = yaFTL.cxtEraseCounter;
    yaftlCI->FTLRestoreCnt = yaFTL.FTLRestoreCnt;
    /* yaftl control info */

    status = _writePage(vpn, tBuff, md, FALSE32);
    if (status != VFL_SUCCESS)
    {
        return status;
    }
    vpn++;

    for (i = 0; i < yaFTL.controlPageNo; i++)
    {
        status = _writePage(vpn + i, (UInt8 *)((UInt8 *)yaFTL.wrState.data.TOC + BYTES_PER_PAGE * i), md, FALSE32);
        if (status != VFL_SUCCESS)
        {
            return status;
        }
    }
    vpn += yaFTL.controlPageNo;

    for (i = 0; i < yaFTL.controlPageNo; i++)
    {
        status = _writePage(vpn + i, (UInt8 *)((UInt8 *)yaFTL.wrState.index.TOC + BYTES_PER_PAGE * i), md, FALSE32);
        if (status != VFL_SUCCESS)
        {
            return status;
        }
    }
    vpn += yaFTL.controlPageNo;

    /* Write statistics */
#ifdef AND_COLLECT_STATISTICS
    _FTLGetStatisticsToCxt(tBuff);

#else
    WMR_MEMSET(tBuff, 0, BYTES_PER_PAGE);
#endif
    status = _writePage(vpn, (UInt8 *)tBuff, md, FALSE32);
    if (status != VFL_SUCCESS)
    {
        return status;
    }
    vpn++;

    size = yaFTL.TOCtableEntriesNo;

    if(statsOnly == TRUE32)
        WMR_MEMSET(md, 0, sizeof(PageMeta_t));

    for (i = 0, j = 0; i < yaFTL.cxtSizeTOC; i++, size -= BYTES_PER_PAGE / sizeof(UInt32), j += BYTES_PER_PAGE / sizeof(UInt32))
    {
        tocPtr = (UInt32 *)tBuff;
        for (k = 0; (k < BYTES_PER_PAGE / sizeof(UInt32)) && (k < size); k++)
        {
            tocPtr[k] = yaFTL.tocArray[j + k].indexPage;
        }
        status = _writePage(vpn + i, tBuff, md, FALSE32);
        if (status != VFL_SUCCESS)
        {
            return status;
        }
    }

    vpn += yaFTL.cxtSizeTOC;

    size = FTL_AREA_SIZE;
    for (i = 0, j = 0; i < yaFTL.cxtSizeBlockStatus; i++, size -= BYTES_PER_PAGE / sizeof(UInt8), j += BYTES_PER_PAGE / sizeof(UInt8))
    {
        statPtr = (UInt8 *)tBuff;
        for (k = 0; (k < BYTES_PER_PAGE / sizeof(UInt8)) && (k < size); k++)
        {
            statPtr[k] = yaFTL.blockArray[j + k].status;

            // Fixup state so running GC operations don't break on-media format:
            if (BLOCK_GC == statPtr[k])
            {
                statPtr[k] = BLOCK_ALLOCATED;
            }
            else if (BLOCK_I_GC == statPtr[k])
            {
                statPtr[k] = BLOCK_I_ALLOCATED;
            }
        }
        status = _writePage(vpn + i, tBuff, md, FALSE32);
        if (status != VFL_SUCCESS)
        {
            return status;
        }
    }

    vpn += yaFTL.cxtSizeBlockStatus;

    size = FTL_AREA_SIZE;
    for (i = 0, j = 0; i < yaFTL.cxtSizeBlockRead; i++, size -= BYTES_PER_PAGE / sizeof(UInt16), j += BYTES_PER_PAGE / sizeof(UInt16))
    {
        validPtr = (UInt16 *)tBuff;
        for (k = 0; (k < BYTES_PER_PAGE / sizeof(UInt16)) && (k < size); k++)
        {
            validPtr[k] = yaFTL.blockArray[j + k].pagesRead;
        }
        status = _writePage(vpn + i, tBuff, md, FALSE32);
        if (status != VFL_SUCCESS)
        {
            return status;
        }
    }

    vpn += yaFTL.cxtSizeBlockRead;

    size = FTL_AREA_SIZE;
    for (i = 0, j = 0; i < yaFTL.cxtSizeBlockErase; i++, size -= BYTES_PER_PAGE / sizeof(UInt32), j += BYTES_PER_PAGE / sizeof(UInt32))
    {
        tocPtr = (UInt32 *)tBuff;
        for (k = 0; (k < BYTES_PER_PAGE / sizeof(UInt32)) && (k < size); k++)
        {
            tocPtr[k] = yaFTL.blockArray[j + k].erasableCount;
        }
        status = _writePage(vpn + i, tBuff, md, FALSE32);
        if (status != VFL_SUCCESS)
        {
            return status;
        }
    }

    vpn += yaFTL.cxtSizeBlockErase;

    size = FTL_AREA_SIZE;
    for (i = 0, j = 0; i < yaFTL.cxtSizeValidPagesI; i++, size -= BYTES_PER_PAGE / sizeof(UInt16), j += BYTES_PER_PAGE / sizeof(UInt16))
    {
        validPtr = (UInt16 *)tBuff;
        for (k = 0; (k < BYTES_PER_PAGE / sizeof(UInt16)) && (k < size); k++)
        {
            validPtr[k] = yaFTL.blockArray[j + k].validPagesINo;
        }
        status = _writePage(vpn + i, tBuff, md, FALSE32);
        if (status != VFL_SUCCESS)
        {
            return status;
        }
    }

    vpn += yaFTL.cxtSizeValidPagesI;

    size = FTL_AREA_SIZE;
    for (i = 0, j = 0; i < yaFTL.cxtSizeValidPagesD; i++, size -= BYTES_PER_PAGE / sizeof(UInt16), j += BYTES_PER_PAGE / sizeof(UInt16))
    {
        validPtr = (UInt16 *)tBuff;
        for (k = 0; (k < BYTES_PER_PAGE / sizeof(UInt16)) && (k < size); k++)
        {
            validPtr[k] = yaFTL.blockArray[j + k].validPagesDNo;
        }
        status = _writePage(vpn + i, tBuff, md, FALSE32);
        if (status != VFL_SUCCESS)
        {
            return status;
        }
    }

    yaFTL.currentCxtVpn = tempVpn;
    yaFTL.cxtValid = 1;

    return status;
}

#endif

#ifndef AND_READONLY
static void moveCXTarea(void)
{
    UInt32 i = 0, j;
    UInt16 tempCXT[FTL_CXT_SECTION_SIZE];
    ANDStatus status;

    if (yaFTL.seaState.data.freeBlocks < FTL_CXT_SECTION_SIZE)
    {
        return;
    }
    
    if (yaFTL.cxtValid == 1)
    {
        invalidateCXT();
        yaFTL.cxtValid = 0;
    }
    

    for (j = 0; j < FTL_CXT_SECTION_SIZE; j++)
    {
        tempCXT[j] = yaFTL.cxtTable[j];
    }

    for (j = 0; (Int32)j < FTL_AREA_SIZE; j++)
    {
        if ((yaFTL.blockArray[j].status == BLOCK_FREE) && ( (yaFTL.blockArray[j].erasableCount + CXT_ERASE_GAP) < yaFTL.blockArray[tempCXT[i]].erasableCount))
        {
            if( yaFTL.blockArray[j].eraseBeforeUse != 0)
            {
                status = VFL_Erase(j, TRUE32);
                if (status != FTL_SUCCESS)
                {
                    WMR_PANIC("VFL_Erase(%d) failed with 0x%08x", (UInt32) j, status);
                    continue;
                }
                yaFTL.blockArray[j].eraseBeforeUse = 0;
                removeBlockFromEraseNowList((UInt16)j);
                yaFTL.erasedBlockCount++;
            }
            yaFTL.blockArray[j].status = BLOCK_CTX_CNTR;
            yaFTL.erasedBlockCount--;
            yaFTL.cxtTable[i] = (UInt16)j;
            i++;
            if (i >= FTL_CXT_SECTION_SIZE)
            {
                break;
            }
        }
    }
    
    if ( i == 0 )
        return;

    VFL_ChangeFTLCxtVbn(yaFTL.cxtTable);
    yaFTL.currentCXTIndex = 0;
    yaFTL.currentCxtVpn = 0xffffffff;
    yaFTL.blockArray[yaFTL.cxtTable[yaFTL.currentCXTIndex]].status = BLOCK_CTX_CURRENT;
    for (j = 0; j < FTL_CXT_SECTION_SIZE; j++)
    {
        VFL_Erase((UInt16)tempCXT[j], TRUE32);
        removeBlockFromEraseNowList((UInt16)tempCXT[j]);
        yaFTL.blockArray[tempCXT[j]].status = BLOCK_FREE;
        yaFTL.blockArray[tempCXT[j]].erasableCount++;
        yaFTL.blockArray[tempCXT[j]].pagesRead = 0;
        yaFTL.blockArray[tempCXT[j]].pagesReadSubCounter = 0;
        yaFTL.blockArray[tempCXT[j]].validPagesDNo = yaFTL.blockArray[tempCXT[j]].validPagesINo = 0;
        yaFTL.blockArray[tempCXT[j]].eraseBeforeUse = 0;
        if (j < i)
            yaFTL.erasedBlockCount++;
        // Update max erase count
        if (yaFTL.blockArray[tempCXT[j]].erasableCount > yaFTL.maxEraseCount)
        {
            yaFTL.maxEraseCount = yaFTL.blockArray[tempCXT[j]].erasableCount;
        }
    }
    for(j = i; j < FTL_CXT_SECTION_SIZE; j++)
        yaFTL.blockArray[yaFTL.cxtTable[j]].status = BLOCK_CTX_CNTR;
}

ANDStatus quickMountUpdate(BOOL32 statsOnly)
{
    UInt8 *tBuff = NULL;
    Buffer *tmpB = NULL;
    ANDStatus status = FTL_SUCCESS;
    UInt32 i;
    /*
     * Read first page out of each Cxt unit to establish which contains a latest valid ( hopefully )
     * copy of a quick mount info . Then read last copy and establish its validity . If it's valid read out the whole
     * thing and return success . If it's not valid use erase/read count info and return failure which would trigger full restore
     */
    if (yaFTL.cxtValid == 1)     // no need to update valid cxt
    {
        return FTL_SUCCESS;
    }

    WMR_TRACE_IST_1(CxtSave, START, statsOnly);
    
    for (i = 0; ((Int32)i < FTL_AREA_SIZE) && (statsOnly == FALSE32); i++)
    {
        if((yaFTL.blockArray[i].status == BLOCK_FREE) && (yaFTL.blockArray[i].eraseBeforeUse != 0))
        {
            ANDStatus status;
            
            // Invalidate context
            if (yaFTL.cxtValid == 1)
            {
                invalidateCXT();
                yaFTL.cxtValid = 0;
            }
            // Erase it
            if(yaFTL.formatWasCalled == 0)
            {
                status = VFL_Erase(i, TRUE32);
                if (status != FTL_SUCCESS)
                {
                    WMR_PANIC("VFL_Erase(%d) failed with 0x%08x", (UInt32) i, status);
                    WMR_TRACE_IST_1(CxtSave, END, status);
                    return status;
                }
                yaFTL.blockArray[i].erasableCount++;
                yaFTL.periodicCxt.erases++;
                yaFTL.erasedBlockCount++;
                if (yaFTL.blockArray[i].erasableCount > yaFTL.maxEraseCount)
                {
                    yaFTL.maxEraseCount = yaFTL.blockArray[i].erasableCount;
                }
                removeBlockFromEraseNowList(i);
            }
            yaFTL.blockArray[i].eraseBeforeUse = 0;
        }
    }    
    
    yaFTL.formatWasCalled = 0;
    
    tmpB = BUF_Get(BUF_MAIN_ONLY);
    if (tmpB == NULL)
    {
        WMR_TRACE_IST_1(CxtSave, END, FTL_CRITICAL_ERROR);
        return FTL_CRITICAL_ERROR;
    }

    tBuff = (UInt8 *)tmpB->pData;

    if (yaFTL.currentCxtVpn == 0xffffffff)
    {
        /* First time update */
        status = writeCXTInfo(yaFTL.cxtTable[yaFTL.currentCXTIndex] * PAGES_PER_SUBLK, tBuff, statsOnly);
        if (status != VFL_SUCCESS)
        {
            VFL_Erase((UInt16)yaFTL.cxtTable[yaFTL.currentCXTIndex], TRUE32);
            removeBlockFromEraseNowList((UInt16)yaFTL.cxtTable[yaFTL.currentCXTIndex]);
            yaFTL.blockArray[(UInt16)yaFTL.cxtTable[yaFTL.currentCXTIndex]].erasableCount++;
            yaFTL.cxtEraseCounter++;
            yaFTL.blockArray[yaFTL.cxtTable[yaFTL.currentCXTIndex]].status = BLOCK_CTX_CNTR;
            yaFTL.currentCXTIndex++;
            if (yaFTL.currentCXTIndex >= FTL_CXT_SECTION_SIZE)
            {
                yaFTL.currentCXTIndex = 0;
            }
            yaFTL.blockArray[yaFTL.cxtTable[yaFTL.currentCXTIndex]].status = BLOCK_CTX_CURRENT;
            /* todo : add erase in case this is not the first time
               todo : check if we need to erase ( no need if this is the first time we use this unit ) */
            VFL_Erase((UInt16)yaFTL.cxtTable[yaFTL.currentCXTIndex], TRUE32);
            removeBlockFromEraseNowList((UInt16)yaFTL.cxtTable[yaFTL.currentCXTIndex]);
            yaFTL.blockArray[(UInt16)yaFTL.cxtTable[yaFTL.currentCXTIndex]].erasableCount++;
            yaFTL.cxtEraseCounter++;
            yaFTL.currentCxtVpn = 0xffffffff;
            BUF_Release(tmpB);
            WMR_TRACE_IST_1(CxtSave, END, status);
            return status;
        }
    }
    else
    {
        /* check whether we can add cxt to a current cxt block */
        if (((yaFTL.currentCxtVpn / PAGES_PER_SUBLK) == ((yaFTL.currentCxtVpn + 1 + yaFTL.cxtSize) / PAGES_PER_SUBLK)) &&
            (((yaFTL.currentCxtVpn + 1 + yaFTL.cxtSize) / PAGES_PER_SUBLK) == ((yaFTL.currentCxtVpn + (1 + yaFTL.cxtSize) * 2) / PAGES_PER_SUBLK)))
        {
            status = writeCXTInfo(yaFTL.currentCxtVpn + 1 + yaFTL.cxtSize, tBuff, statsOnly);
            if (status != VFL_SUCCESS)
            {
                VFL_Erase((UInt16)yaFTL.cxtTable[yaFTL.currentCXTIndex], TRUE32);
                removeBlockFromEraseNowList((UInt16)yaFTL.cxtTable[yaFTL.currentCXTIndex]);
                yaFTL.blockArray[(UInt16)yaFTL.cxtTable[yaFTL.currentCXTIndex]].erasableCount++;
                yaFTL.cxtEraseCounter++;
                yaFTL.blockArray[yaFTL.cxtTable[yaFTL.currentCXTIndex]].status = BLOCK_CTX_CNTR;
                yaFTL.currentCXTIndex++;
                if (yaFTL.currentCXTIndex >= FTL_CXT_SECTION_SIZE)
                {
                    yaFTL.currentCXTIndex = 0;
                }
                yaFTL.blockArray[yaFTL.cxtTable[yaFTL.currentCXTIndex]].status = BLOCK_CTX_CURRENT;
                /* todo : add erase in case this is not the first time
                   todo : check if we need to erase ( no need if this is the first time we use this unit ) */
                VFL_Erase((UInt16)yaFTL.cxtTable[yaFTL.currentCXTIndex], TRUE32);
                removeBlockFromEraseNowList((UInt16)yaFTL.cxtTable[yaFTL.currentCXTIndex]);
                yaFTL.blockArray[(UInt16)yaFTL.cxtTable[yaFTL.currentCXTIndex]].erasableCount++;
                yaFTL.cxtEraseCounter++;
                yaFTL.currentCxtVpn = 0xffffffff;
                BUF_Release(tmpB);
                WMR_TRACE_IST_1(CxtSave, END, status);
                return status;
            }
        }
        else
        {
            /* not enough space in current cxt block . erase the oldest and update in it */

            yaFTL.blockArray[yaFTL.cxtTable[yaFTL.currentCXTIndex]].status = BLOCK_CTX_CNTR;
            yaFTL.currentCXTIndex++;
            if (yaFTL.currentCXTIndex >= FTL_CXT_SECTION_SIZE)
            {
                yaFTL.currentCXTIndex = 0;
            }
            yaFTL.blockArray[yaFTL.cxtTable[yaFTL.currentCXTIndex]].status = BLOCK_CTX_CURRENT;
            /* todo : add erase in case this is not the first time
               todo : check if we need to erase ( no need if this is the first time we use this unit ) */
            VFL_Erase((UInt16)yaFTL.cxtTable[yaFTL.currentCXTIndex], TRUE32);
            removeBlockFromEraseNowList((UInt16)yaFTL.cxtTable[yaFTL.currentCXTIndex]);
            yaFTL.blockArray[(UInt16)yaFTL.cxtTable[yaFTL.currentCXTIndex]].erasableCount++;
            yaFTL.cxtEraseCounter++;
            status = writeCXTInfo(yaFTL.cxtTable[yaFTL.currentCXTIndex] * PAGES_PER_SUBLK, tBuff, statsOnly);
            if (status != VFL_SUCCESS)
            {
                VFL_Erase((UInt16)yaFTL.cxtTable[yaFTL.currentCXTIndex], TRUE32);
                removeBlockFromEraseNowList((UInt16)yaFTL.cxtTable[yaFTL.currentCXTIndex]);
                yaFTL.blockArray[(UInt16)yaFTL.cxtTable[yaFTL.currentCXTIndex]].erasableCount++;
                yaFTL.cxtEraseCounter++;
                yaFTL.blockArray[yaFTL.cxtTable[yaFTL.currentCXTIndex]].status = BLOCK_CTX_CNTR;
                yaFTL.currentCXTIndex++;
                if (yaFTL.currentCXTIndex >= FTL_CXT_SECTION_SIZE)
                {
                    yaFTL.currentCXTIndex = 0;
                }
                yaFTL.blockArray[yaFTL.cxtTable[yaFTL.currentCXTIndex]].status = BLOCK_CTX_CURRENT;
                /* todo : add erase in case this is not the first time
                   todo : check if we need to erase ( no need if this is the first time we use this unit ) */
                VFL_Erase((UInt16)yaFTL.cxtTable[yaFTL.currentCXTIndex], TRUE32);
                removeBlockFromEraseNowList((UInt16)yaFTL.cxtTable[yaFTL.currentCXTIndex]);
                yaFTL.blockArray[(UInt16)yaFTL.cxtTable[yaFTL.currentCXTIndex]].erasableCount++;
                yaFTL.cxtEraseCounter++;
                yaFTL.currentCxtVpn = 0xffffffff;
                BUF_Release(tmpB);
                WMR_TRACE_IST_1(CxtSave, END, status);
                return status;
            }
        }
    }
    BUF_Release(tmpB);
    WMR_TRACE_IST_1(CxtSave, END, FTL_SUCCESS);
    return FTL_SUCCESS;
}

#endif // 0 : still working on the above code
/*****************************************************************************/
/*                                                                           */
/* NAME                                                                      */
/*      restoreMount														 */
/* DESCRIPTION                                                               */
/*      Perfroms restore mount in case control info is dirty                 */
/*                                                                           */
/*****************************************************************************/

UInt16 findFreeCacheEntry(void);

static void SetupFreeAndAllocd(void)
{
    UInt32 i;

    yaFTL.seaState.data.freeBlocks = 0;
    yaFTL.seaState.data.allocdBlocks = 0;
    yaFTL.seaState.index.freeBlocks = 0;
    yaFTL.seaState.index.allocdBlocks = 0;
    yaFTL.seaState.freeBlocks = 0;

    for (i = 0; i < FTL_AREA_SIZE; i++) {
        switch (yaFTL.blockArray[i].status) {
            case BLOCK_ALLOCATED:
            case BLOCK_CURRENT:
                yaFTL.seaState.data.allocdBlocks++;
                break;

            case BLOCK_I_ALLOCATED:
            case BLOCK_I_CURRENT:
                yaFTL.seaState.index.allocdBlocks++;
                break;

            case BLOCK_FREE:
                yaFTL.seaState.freeBlocks++;
                break;

            case BLOCK_CTX_CNTR:
            case BLOCK_CTX_CURRENT:
                break;

            default:
#ifndef AND_READONLY
                // Invalidate context
                if (yaFTL.cxtValid == 1)
                {
                    invalidateCXT();
                    yaFTL.cxtValid = 0;
                }
                WMR_PANIC("Unknown block type: 0x%x\n", yaFTL.blockArray[i].status);
#else
                WMR_PRINT(QUAL_FATAL, "Unknown block type: 0x%x\n", yaFTL.blockArray[i].status);
#endif
        }
    }

    if (yaFTL.seaState.freeBlocks < 2)
    {
#ifndef AND_READONLY        
        // Invalidate context
        if (yaFTL.cxtValid == 1)
        {
            invalidateCXT();
            yaFTL.cxtValid = 0;
        }
        WMR_PANIC("no free blocks %d ", yaFTL.seaState.freeBlocks);
#else
        WMR_PRINT(QUAL_FATAL, "no free blocks %d ", yaFTL.seaState.freeBlocks);
#endif 
    }

    yaFTL.seaState.index.freeBlocks = yaFTL.indexSize - yaFTL.seaState.index.allocdBlocks;
    if (yaFTL.seaState.freeBlocks > (UInt32)yaFTL.seaState.index.freeBlocks)
    {
        yaFTL.seaState.data.freeBlocks = yaFTL.seaState.freeBlocks - yaFTL.seaState.index.freeBlocks;
    }
    else
    {
        if (yaFTL.seaState.freeBlocks == 1)
        {
            yaFTL.seaState.index.freeBlocks = 0;
            yaFTL.seaState.data.freeBlocks = 1;
        }
        else
        {
            yaFTL.seaState.index.freeBlocks = 1;
            yaFTL.seaState.data.freeBlocks = yaFTL.seaState.freeBlocks - yaFTL.seaState.index.freeBlocks;
        }
    }
}


#ifndef AND_READONLY
static ANDStatus restoreMountEraseIndex(UInt16 * ctxT)
{
    UInt32 Vpn;
    ANDStatus status = FTL_SUCCESS;
    UInt32 *pageBuffer = NULL;
    UInt32 i, n;
    Buffer * pBuff;

    UInt32 j;
    UInt32 *indexTable;
    UInt8 ctxNo = 0;
    PageMeta_t *mdPtr = yaFTL.meta_restoreMountErase;
    BlockListType *listHead = NULL, *tempPtr, *tempHdr;
    BlockRangeType *brPtr;
    // u_int64_t t1,t2,t3,t4,t22;
    BlockListType *listIndexHead = NULL, *tempIndexPtr;

    yaFTL.wrState.index.block = 0xffffffff;
    WMR_MEMSET(yaFTL.tocArray, 0xff, yaFTL.TOCtableEntriesNo * sizeof(TOCEntry));
    WMR_MEMSET(&yaFTL.seaState, 0, sizeof(yaFTL.seaState));
    // t1=mach_absolute_time();
    // Nir - what do you mean we have no cxt? those are kept by the VFL for you...
    if (ctxT[0] == 0xffff)
    {
        /* No ctx units . need to allocate at the end of restore */
        ctxNo = 0;
    }
    else
    {
        ctxNo = FTL_CXT_SECTION_SIZE;
    }

    Vpn = 0;
    pBuff = BUF_Get(BUF_MAIN_AND_SPARE);
    if (pBuff == NULL)
    {
        return FTL_CRITICAL_ERROR;
    }
    
    for (i = 0; (Int32)i < FTL_AREA_SIZE; i++, Vpn += PAGES_PER_SUBLK)
    {
        /*
           go over all blocks in partititon
           todo : setup currentDndex block . Also need to calculate free dataBlocks and perhaps execute GC before we finish restore .
         */

        if (ctxNo)
        {
            if ((yaFTL.blockArray[i].status == BLOCK_CTX_CNTR) || (yaFTL.blockArray[i].status == BLOCK_CTX_CURRENT))
            {
                continue;
            }
        }
        for (n = 0; n < (UInt32)(PAGES_PER_SUBLK - yaFTL.controlPageNo); n++)
        {
            status = _readPage(Vpn + n, pBuff->pData, mdPtr, FALSE32, TRUE32, FALSE32);
            if ((status != FTL_SUCCESS) && (status != FIL_SUCCESS_CLEAN))
            {
                debug(INIT, "something is not right with the x%x x%x", Vpn / PAGES_PER_SUBLK, status);
				continue;
            }

            if (status == FIL_SUCCESS_CLEAN)
            {
                break;
            }

            if (status == FTL_SUCCESS)
            {
                if((META_IS_IDX(mdPtr)) && (META_GET_IPN(mdPtr) >= yaFTL.TOCtableEntriesNo))
                {
                    status = FTL_CRITICAL_ERROR;
                    continue;
                }
                if((META_IS_DATA(mdPtr)) && (META_GET_LBA(mdPtr) >= yaFTL.logicalPartitionSize))
                {
                    status = FTL_CRITICAL_ERROR;
                    continue;
                }
                break;
            }
        }

        if ((status != FTL_SUCCESS) && (status != FIL_SUCCESS_CLEAN))
        {
            debug(INIT, "something is not right block x%x ... probably need to erase", i);
#ifndef AND_READONLY
            status = VFL_Erase((UInt16)i, TRUE32);
            if (status == FTL_SUCCESS)
            {
                yaFTL.blockArray[i].status = BLOCK_FREE;
            }
            yaFTL.blockArray[i].erasableCount++;
            removeBlockFromEraseNowList((UInt16)i);
#endif
            continue;
        }

        if ((META_ARE_FLAGS_FF(mdPtr) && (META_GET_LBA(mdPtr) == 0xffffffff)) || (status == FIL_SUCCESS_CLEAN))       /* free block */
        {
            yaFTL.blockArray[i].status = BLOCK_FREE;
            yaFTL.blockArray[i].eraseBeforeUse = BLOCK_TO_BE_ERASED;

            continue;
        }
        if (META_IS_IDX(mdPtr))    /* index block : erase */
        {
#ifndef AND_READONLY
            status = VFL_Erase((UInt16)i, TRUE32);
            if (status == FTL_SUCCESS)
            {
                yaFTL.blockArray[i].status = BLOCK_FREE;
            }
            yaFTL.blockArray[i].erasableCount++;
            removeBlockFromEraseNowList((UInt16)i);
#else
            /* create a parallel list for Index blocks */
            listIndexHead = addBlockToList(listIndexHead, (UInt16)i, META_GET_WEAVESEQ(mdPtr));
            WMR_ASSERT(NULL != listIndexHead);
#endif
            continue;
        }
        /* Valid data block . Add it to a sorted linked list of data blocks according to weaveSeq */
        if (META_IS_DATA(mdPtr))
        {
            yaFTL.blockArray[i].status = BLOCK_ALLOCATED;
            yaFTL.wrState.weaveSeq = WMR_MAX(yaFTL.wrState.weaveSeq, META_GET_WEAVESEQ(mdPtr));
            listHead = addBlockToList(listHead, (UInt16)i, META_GET_WEAVESEQ(mdPtr));
            WMR_ASSERT(NULL != listHead);
            continue;
        }
        debug(ERROR, "block that does not belong to anything 0x%x  .. probably need to erase", META_GET_ALL_FLAGS(mdPtr));
#ifndef AND_READONLY
        status = VFL_Erase((UInt16)i, TRUE32);
        if (status == FTL_SUCCESS)
        {
            yaFTL.blockArray[i].status = BLOCK_FREE;
        }
        yaFTL.blockArray[i].erasableCount++;
        removeBlockFromEraseNowList((UInt16)i);
#endif
    }
    // t2=mach_absolute_time();

    BUF_Release(pBuff);

    yaFTL.freeCachePages = (UInt16)yaFTL.indexCacheSize;
#ifndef AND_READONLY
    yaFTL.nextFreeCachePage = 0;
#endif
    debug(INIT, " restoreMount[after blocklist has been created]");
    if (listIndexHead != NULL)
    {
        tempIndexPtr = listIndexHead;
        while (tempIndexPtr != NULL)
        {
            if (tempIndexPtr == listIndexHead)
            {
                status = restoreIndexBlock(tempIndexPtr->blockNo, 1, tempIndexPtr->weaveSeq);
            }
            else
            {
                status = restoreIndexBlock(tempIndexPtr->blockNo, 0, tempIndexPtr->weaveSeq);
            }
            if (status != FTL_SUCCESS)
            {
                freeBlockList(tempIndexPtr);
                freeBlockList(listHead);
                return status;
            }
            listIndexHead = tempIndexPtr;
            tempIndexPtr = tempIndexPtr->next;
            WMR_FREE(listIndexHead, sizeof(BlockListType));
        }
    }
    if (yaFTL.wrState.index.block == 0xffffffff)
    {
        debug(INIT, "index list is empty ");
        for (i = 0; i < (UInt32)FTL_AREA_SIZE; i++)
        {
            if (yaFTL.blockArray[i].status == BLOCK_FREE)
            {
                yaFTL.wrState.index.block = i;      /* current physical block being used for updates */
                yaFTL.wrState.index.nextPage = 0;     /* offset in current block of next free page */
                WMR_MEMSET(yaFTL.wrState.index.TOC, 0xff, yaFTL.controlPageNo * BYTES_PER_PAGE);
                yaFTL.blockArray[i].status = BLOCK_I_CURRENT;
                debug(INIT, "current index block would be %d", i);
                break;
            }
        }
    }

    // t22=mach_absolute_time();
    if (listHead == NULL)   /* empty partition */
    {
        for (i = 0; i < (UInt32)FTL_AREA_SIZE; i++)
        {
            if (yaFTL.blockArray[i].status == BLOCK_FREE)
            {
                yaFTL.wrState.data.block = i;
                yaFTL.blockArray[yaFTL.wrState.data.block].status = BLOCK_CURRENT;
                break;
            }
        }
        yaFTL.wrState.data.nextPage = 0;
        WMR_MEMSET(yaFTL.wrState.data.TOC, 0xff, yaFTL.controlPageNo * BYTES_PER_PAGE);
#ifndef AND_READONLY
        if (ctxNo == 0)
        {
            i = 0;
            for (j = 0; (Int32)j < FTL_AREA_SIZE; j++)
            {
                if (yaFTL.blockArray[j].status == BLOCK_FREE)
                {
                    yaFTL.blockArray[j].status = BLOCK_CTX_CNTR;
                    ctxT[i] = (UInt16)j;
                    i++;
                    if (i >= FTL_CXT_SECTION_SIZE)
                    {
                        break;
                    }
                }
            }
            VFL_ChangeFTLCxtVbn(ctxT);
            for (i = 0; i < FTL_CXT_SECTION_SIZE; i++)
            {
                yaFTL.cxtTable[i] = ctxT[i];
            }
        }
#endif

        // Set up block counters
        for (i = 0, yaFTL.erasedBlockCount = 0; i < FTL_AREA_SIZE; i++)
        {
            if((yaFTL.blockArray[i].status == BLOCK_FREE) && (yaFTL.blockArray[i].eraseBeforeUse == 0))
                yaFTL.erasedBlockCount++;
        }
        SetupFreeAndAllocd();

#ifdef AND_COLLECT_STATISTICS
        stFTLStatistics.ddwFTLRestoreCnt++;
        stFTLStatistics.ddwFreeBlks = yaFTL.seaState.data.freeBlocks + yaFTL.seaState.index.freeBlocks;
        stFTLStatistics.ddwValidDataPages = 0;
        stFTLStatistics.ddwValidIndexPages = 0;
#endif
        yaFTL.seaState.distCheckable = 1;
        CheckBlockDist();
        return FTL_SUCCESS;
    }

    tempHdr = listHead;

    pBuff = BUF_Get(BUF_MAIN_AND_SPARE);
    if (NULL == pBuff)
    {
        freeBlockList(listHead);
        return FTL_CRITICAL_ERROR;
    }
    
    if (!_allocateRestoreIndexBuffers(&brPtr, &indexTable, &tempIndexBufferSize))
    {
        freeBlockList(listHead);
        return FTL_CRITICAL_ERROR;
    }
    
    WMR_MEMSET(brPtr, 0xff, (FTL_AREA_SIZE * sizeof(BlockRangeType)));

    // Set up block counters
    SetupFreeAndAllocd();

    for (j = 0; j < yaFTL.logicalPartitionSize; j += tempIndexBufferSize)
    {
        WMR_MEMSET(indexTable, 0xff, tempIndexBufferSize * sizeof(UInt32));
        tempPtr = listHead = tempHdr;
        while (tempPtr != NULL)
        {
            if (tempPtr == listHead)
            {
                status = restoreBlock(tempPtr->blockNo, indexTable, 1, j, tempIndexBufferSize, brPtr, tempPtr->weaveSeq);
            }
            else
            {
                status = restoreBlock(tempPtr->blockNo, indexTable, 0, j, tempIndexBufferSize, brPtr, tempPtr->weaveSeq);
            }
            if (status != FTL_SUCCESS)
            {
                WMR_BOOT_FREE(indexTable);
                WMR_FREE(brPtr, FTL_AREA_SIZE * sizeof(BlockRangeType));
                freeBlockList(tempHdr);
                BUF_Release(pBuff);
                return status;
            }
            listHead = tempPtr;
            tempPtr = tempPtr->next;
            if ((j + tempIndexBufferSize) >= yaFTL.logicalPartitionSize)
            {
                tempHdr = tempPtr;
                WMR_FREE(listHead, sizeof(BlockListType));
            }
        }

        WMR_MEMSET(pBuff->pData, 0xff, BYTES_PER_PAGE);

        for (i = 0, pageBuffer = indexTable; (i < (tempIndexBufferSize * 4) / BYTES_PER_PAGE) && ((i * yaFTL.indexPageRatio + j) < yaFTL.logicalPartitionSize); i++, pageBuffer += (BYTES_PER_PAGE / 4))
        {
            WMR_MEMSET(pBuff->pData, 0xff, BYTES_PER_PAGE);
            if (WMR_MEMCMP(pageBuffer, pBuff->pData, BYTES_PER_PAGE) != 0)
            {
                UInt16 cE;

                /* Need to copy valid page in index cache */
                if (yaFTL.tocArray[i + ((j * 4) / BYTES_PER_PAGE)].indexPage != 0xffffffff)
                {
                    yaFTL.blockArray[(yaFTL.tocArray[i + ((j * 4) / BYTES_PER_PAGE)].indexPage) / PAGES_PER_SUBLK].validPagesINo--;
                    yaFTL.seaState.index.validPages--;
                }

                cE = findFreeCacheEntry();
                if (cE == 0xffff)
                {
#ifdef AND_READONLY
                    debug(ERROR, "can still be enough to mount OS partition");
                    BUF_Release(pBuff);
                    return FTL_SUCCESS;
#else
                    WMR_MEMCPY(yaFTL.tmpReadBuffer, pageBuffer, BYTES_PER_PAGE);
                    SetupMeta_Index(mdPtr, i + ((j * 4) / BYTES_PER_PAGE));
                    status = writeIndexPage((UInt8*)yaFTL.tmpReadBuffer, mdPtr, 0);
                    if (status != FTL_SUCCESS)
                    {
                        BUF_Release(pBuff);
                        return FTL_CRITICAL_ERROR;
                    }

                    continue;
#endif
                }
                WMR_MEMCPY(yaFTL.indexCache[cE].indexPage, pageBuffer, BYTES_PER_PAGE);
                yaFTL.indexCache[cE].status = IC_DIRTY_PAGE;
                yaFTL.indexCache[cE].tocEntry = (UInt16)(i + ((j * 4) / BYTES_PER_PAGE));
                yaFTL.tocArray[i + ((j * 4) / BYTES_PER_PAGE)].cacheIndex = cE;
                yaFTL.tocArray[i + ((j * 4) / BYTES_PER_PAGE)].indexPage = 0xffffffff;
            }
            else
            {
                yaFTL.tocArray[i + ((j * 4) / BYTES_PER_PAGE)].indexPage = 0xffffffff;
                yaFTL.tocArray[i + ((j * 4) / BYTES_PER_PAGE)].cacheIndex = 0xffff;
            }
        }
    }
    // t3=mach_absolute_time();

#ifndef AND_READONLY
    /* Vadim - check presence of CXT blocks . If there are none allocate them */

    if (ctxNo == 0)
    {
        i = 0;
        for (j = 0; j < (UInt32)FTL_AREA_SIZE; j++)
        {
            if (yaFTL.blockArray[j].status == BLOCK_FREE)
            {
                yaFTL.blockArray[j].status = BLOCK_CTX_CNTR;
                ctxT[i] = (UInt16)j;
                i++;
                if (i >= FTL_CXT_SECTION_SIZE)
                {
                    break;
                }
            }
        }
        VFL_ChangeFTLCxtVbn(ctxT);
    }
#endif

    BUF_Release(pBuff);
    WMR_BOOT_FREE(indexTable);
    WMR_FREE(brPtr, FTL_AREA_SIZE * sizeof(BlockRangeType));

    // t4=mach_absolute_time();
    // debug(MISC, "time spent in restore is  %llu %llu %llu %llu %llu ",t2-t1,t22-t2,t3-t22,t4-t3,t4-t1);
    debug(INIT, "current i block %d current i page %d", yaFTL.wrState.index.block, yaFTL.wrState.index.nextPage);

    debug(INIT, "restoreMount : logicalPartitionSize %d  TOCtableEntriesNo %d \nwrState.data.block %d wrState.data.nextPage %d \n BYTES_PER_PAGE %d \n wrState.index.block %d wrState.index.nextPage %d wrState.weaveSeq %d"
          , yaFTL.logicalPartitionSize, yaFTL.TOCtableEntriesNo, yaFTL.wrState.data.block, yaFTL.wrState.data.nextPage, BYTES_PER_PAGE, yaFTL.wrState.index.block, yaFTL.wrState.index.nextPage, yaFTL.wrState.weaveSeq);
    debug(INIT, "restoreMount seaState.data.allocdBlocks %d dataSize %d FTL_SIZE %d pagesinblock %d", yaFTL.seaState.data.allocdBlocks, yaFTL.dataSize, FTL_AREA_SIZE, PAGES_PER_SUBLK);

    for (i = 0, yaFTL.erasedBlockCount = 0; i < FTL_AREA_SIZE; i++)
    {
        if((yaFTL.blockArray[i].status == BLOCK_FREE) && (yaFTL.blockArray[i].eraseBeforeUse == 0))
            yaFTL.erasedBlockCount++;
    }
#ifndef AND_READONLY
    for (i = 0; ((Int32)i < FTL_AREA_SIZE); i++)
    {
        if ((yaFTL.blockArray[i].status == BLOCK_ALLOCATED) && (yaFTL.blockArray[i].validPagesDNo == 0 ))
        {
            yaFTL.blockArray[i].validPagesINo = 0;
            YAFTL_GC_Data(i, FALSE32);
            continue;
        }
        if ((yaFTL.blockArray[i].status == BLOCK_I_ALLOCATED) && (yaFTL.blockArray[i].validPagesINo == 0 ))
        {
            YAFTL_GC_Index(i, FALSE32);
        }
    }
    // Move the current blocks out :
    i = yaFTL.wrState.index.block;
    YAFTL_GC_Index(i, FALSE32); 
    if ((yaFTL.blockArray[i].status == BLOCK_FREE)&& (yaFTL.blockArray[i].eraseBeforeUse != 0))
    {
        status = VFL_Erase((UInt16)i, TRUE32);
        if (status == FTL_SUCCESS)
        {
            yaFTL.blockArray[i].erasableCount++;
            yaFTL.blockArray[i].eraseBeforeUse = 0;
            yaFTL.erasedBlockCount++;
        }
    }
    
    i = yaFTL.wrState.data.block;
    yaFTL.blockArray[i].validPagesINo = 0;
    YAFTL_GC_Data(i, FALSE32);
    if ((yaFTL.blockArray[i].status == BLOCK_FREE) && (yaFTL.blockArray[i].eraseBeforeUse != 0))
    {
        status = VFL_Erase((UInt16)i, TRUE32);
        if (status == FTL_SUCCESS)
        {
            yaFTL.blockArray[i].erasableCount++;
            yaFTL.blockArray[i].eraseBeforeUse = 0;
            yaFTL.erasedBlockCount++;
        }
    }
    
    while ((yaFTL.indexSize - FREE_I_BLOCK_TRS) < yaFTL.seaState.index.allocdBlocks)
    {
        debug(MISC, "free index %d %d", yaFTL.indexSize, yaFTL.seaState.index.allocdBlocks);
        YAFTL_GC_Index(0xffffffff, FALSE32);
    }
#endif
    for (i = 0; (Int32)i < FTL_AREA_SIZE; i++)
    {
        if ((yaFTL.blockArray[i].status == BLOCK_ALLOCATED) || (yaFTL.blockArray[i].status == BLOCK_CURRENT))
        {
            yaFTL.blockArray[i].validPagesINo = 0;
        }
#ifndef AND_READONLY
        if ((yaFTL.blockArray[i].status != BLOCK_FREE) && (yaFTL.blockArray[i].eraseBeforeUse == BLOCK_TO_BE_MOVED))
        {
            if ((yaFTL.blockArray[i].status == BLOCK_ALLOCATED) || (yaFTL.blockArray[i].status == BLOCK_CURRENT))
            {
                YAFTL_GC_Data(i, FALSE32);
                if ((yaFTL.blockArray[i].status == BLOCK_FREE) && (yaFTL.blockArray[i].eraseBeforeUse != 0))
                {
                    status = VFL_Erase((UInt16)i, TRUE32);
                    if (status == FTL_SUCCESS)
                    {
                        yaFTL.blockArray[i].erasableCount++;
                        yaFTL.blockArray[i].eraseBeforeUse = 0;
                        yaFTL.erasedBlockCount++;
                    }
                }
            }
            else
            {
                YAFTL_GC_Index(i, FALSE32);
                if ((yaFTL.blockArray[i].status == BLOCK_FREE) && (yaFTL.blockArray[i].eraseBeforeUse != 0))
                {
                    status = VFL_Erase((UInt16)i, TRUE32);
                    if (status == FTL_SUCCESS)
                    {
                        yaFTL.blockArray[i].erasableCount++;
                        yaFTL.blockArray[i].eraseBeforeUse = 0;
                        yaFTL.erasedBlockCount++;
                    }
                }
            }
        }
#endif
    }
    debug(INIT, "restoreMount : seaState.index.validPages = %d seaState.data.validPages = %d", yaFTL.seaState.index.validPages, yaFTL.seaState.data.validPages);

#ifdef AND_COLLECT_STATISTICS
    stFTLStatistics.ddwFTLRestoreCnt++;
    stFTLStatistics.ddwFreeBlks = yaFTL.seaState.data.freeBlocks + yaFTL.seaState.index.freeBlocks;
    stFTLStatistics.ddwValidDataPages = yaFTL.seaState.data.validPages;
    stFTLStatistics.ddwValidIndexPages = yaFTL.seaState.index.validPages;
#endif

    yaFTL.seaState.distCheckable = 1;
    CheckBlockDist();
    return FTL_SUCCESS;
}

#endif


static ANDStatus restoreMount(UInt16 * ctxT, const UInt32 readOnly)
{
    UInt32 Vpn;
    ANDStatus status = FTL_SUCCESS;
    UInt32 *pageBuffer = NULL;
    UInt32 i, n;
    Buffer * pBuff;

    UInt32 j;
    UInt32 *indexTable;
    UInt8 ctxNo = 0;
    PageMeta_t *mdPtr = yaFTL.meta_restoreMount;
    BlockListType *listHead = NULL, *tempPtr, *tempHdr;
    BlockRangeType *brPtr;
    // u_int64_t t1,t2,t3,t4,t22;
    BlockListType *listIndexHead = NULL, *tempIndexPtr;

    yaFTL.wrState.index.block = 0xffffffff;
    WMR_MEMSET(yaFTL.tocArray, 0xff, yaFTL.TOCtableEntriesNo * sizeof(TOCEntry));
    WMR_MEMSET(&yaFTL.seaState, 0, sizeof(yaFTL.seaState));

    // t1=mach_absolute_time();
    // Nir - what do you mean we have no cxt? those are kept by the VFL for you...
    if (ctxT[0] == 0xffff)
    {
        /* No ctx units . need to allocate at the end of restore */
        ctxNo = 0;
    }
    else
    {
        ctxNo = FTL_CXT_SECTION_SIZE;
    }

    Vpn = 0;
    pBuff = BUF_Get(BUF_MAIN_AND_SPARE);
    if (pBuff == NULL)
    {
        return FTL_CRITICAL_ERROR;
    }

    for (i = 0; (Int32)i < FTL_AREA_SIZE; i++, Vpn += PAGES_PER_SUBLK)
    {
        /*
           go over all blocks in partititon
           todo : setup currentDndex block . Also need to calculate free dataBlocks and perhaps execute GC before we finish restore .
         */

        if (ctxNo)
        {
            if ((yaFTL.blockArray[i].status == BLOCK_CTX_CNTR) || (yaFTL.blockArray[i].status == BLOCK_CTX_CURRENT))
            {
                continue;
            }
        }
        for (n = 0; n < (UInt32)(PAGES_PER_SUBLK - yaFTL.controlPageNo); n++)
        {
            status = _readPage(Vpn + n, pBuff->pData, mdPtr, FALSE32, TRUE32, FALSE32);
            if ((status != FTL_SUCCESS) && (status != FIL_SUCCESS_CLEAN))
            {
                debug(INIT, "something is not right with the x%x x%x", Vpn / PAGES_PER_SUBLK, status);
                continue;
            }
            
            if (status == FIL_SUCCESS_CLEAN)
            {
                break;
            }
            
            if (status == FTL_SUCCESS)
            {
                if((META_IS_IDX(mdPtr)) && (META_GET_IPN(mdPtr) >= yaFTL.TOCtableEntriesNo))
                {
                    status = FTL_CRITICAL_ERROR;
                    continue;
                }
                if((META_IS_DATA(mdPtr)) && (META_GET_LBA(mdPtr) >= yaFTL.logicalPartitionSize))
                {
                    status = FTL_CRITICAL_ERROR;
                    continue;
                }
                break;
            }
        }
        
        if ((status != FTL_SUCCESS) && (status != FIL_SUCCESS_CLEAN))
        {
            debug(INIT, "something is not right block x%x ... probably need to erase", i);
#ifndef AND_READONLY
            if ( !readOnly )
            {
                status = VFL_Erase((UInt16)i, TRUE32);
                if (status == FTL_SUCCESS)
                {
                    yaFTL.blockArray[i].status = BLOCK_FREE;
                }
                yaFTL.blockArray[i].erasableCount++;
                removeBlockFromEraseNowList((UInt16)i);
            }
#endif
            continue;
        }

        if ((META_ARE_FLAGS_FF(mdPtr) && (META_GET_LBA(mdPtr) == 0xffffffff)) || (status == FIL_SUCCESS_CLEAN))       /* free block */
        {
            yaFTL.blockArray[i].status = BLOCK_FREE;
            yaFTL.blockArray[i].eraseBeforeUse = BLOCK_TO_BE_ERASED;

            continue;
        }
        if (META_IS_IDX(mdPtr))    /* index block : erase */
        {
            /* create a parallel list for Index blocks */
            listIndexHead = addBlockToList(listIndexHead, (UInt16)i, META_GET_WEAVESEQ(mdPtr));
            yaFTL.wrState.weaveSeq = WMR_MAX(yaFTL.wrState.weaveSeq, META_GET_WEAVESEQ(mdPtr));
            WMR_ASSERT(NULL != listIndexHead);
            continue;
        }
        /* Valid data block . Add it to a sorted linked list of data blocks according to weaveSeq */
        if (META_IS_DATA(mdPtr))
        {
            listHead = addBlockToList(listHead, (UInt16)i, META_GET_WEAVESEQ(mdPtr));
            yaFTL.wrState.weaveSeq = WMR_MAX(yaFTL.wrState.weaveSeq, META_GET_WEAVESEQ(mdPtr));
            WMR_ASSERT(NULL != listHead);
            continue;
        }
        debug(ERROR, "block that does not belong to anything 0x%x  .. probably need to erase", META_GET_ALL_FLAGS(mdPtr));
#ifndef AND_READONLY
        if ( !readOnly )
        {
            status = VFL_Erase((UInt16)i, TRUE32);
            if (status == FTL_SUCCESS)
            {
                yaFTL.blockArray[i].status = BLOCK_FREE;
            }
            yaFTL.blockArray[i].erasableCount++;
            removeBlockFromEraseNowList((UInt16)i);
        }
#endif
    }
    // t2=mach_absolute_time();

    BUF_Release(pBuff);

    yaFTL.freeCachePages = (UInt16)yaFTL.indexCacheSize;
#ifndef AND_READONLY
    if ( !readOnly )
    {
        yaFTL.nextFreeCachePage = 0;
    }
#endif
    debug(INIT, " restoreMount[after blocklist has been created]");
    if (listIndexHead != NULL)
    {
        tempIndexPtr = listIndexHead;
        while (tempIndexPtr != NULL)
        {
            if (tempIndexPtr == listIndexHead)
            {
                status = restoreIndexBlock(tempIndexPtr->blockNo, 1, tempIndexPtr->weaveSeq);
            }
            else
            {
                status = restoreIndexBlock(tempIndexPtr->blockNo, 0, tempIndexPtr->weaveSeq);
            }
            if (status != FTL_SUCCESS)
            {
                freeBlockList(listIndexHead);
                freeBlockList(listHead);
                return status;
            }
            listIndexHead = tempIndexPtr;
            tempIndexPtr = tempIndexPtr->next;
            WMR_FREE(listIndexHead, sizeof(BlockListType));
        }
    }
    if (yaFTL.wrState.index.block == 0xffffffff)
    {
        for (i = 0; i < (UInt32)FTL_AREA_SIZE; i++)
        {
            if (yaFTL.blockArray[i].status == BLOCK_FREE)
            {
                yaFTL.wrState.index.block = i;      /* current physical block being used for updates */
                yaFTL.wrState.index.nextPage = 0;     /* offset in current block of next free page */
                WMR_MEMSET(yaFTL.wrState.index.TOC, 0xff, yaFTL.controlPageNo * BYTES_PER_PAGE);
                yaFTL.blockArray[i].status = BLOCK_I_CURRENT;
                debug(INIT, "current index block would be %d", i);
                break;
            }
        }
    }

    // t22=mach_absolute_time();
    if (listHead == NULL)   /* empty partition */
    {
        for (i = 0; i < (UInt32)FTL_AREA_SIZE; i++)
        {
            if (yaFTL.blockArray[i].status == BLOCK_FREE)
            {
                yaFTL.wrState.data.block = i;
                yaFTL.blockArray[yaFTL.wrState.data.block].status = BLOCK_CURRENT;
                break;
            }
        }
        yaFTL.wrState.data.nextPage = 0;
        WMR_MEMSET(yaFTL.wrState.data.TOC, 0xff, yaFTL.controlPageNo * BYTES_PER_PAGE);
#ifndef AND_READONLY
        if ( !readOnly )
        {
            if (ctxNo == 0)
            {
                i = 0;
                for (j = 0; (Int32)j < FTL_AREA_SIZE; j++)
                {
                    if (yaFTL.blockArray[j].status == BLOCK_FREE)
                    {
                        yaFTL.blockArray[j].status = BLOCK_CTX_CNTR;
                        ctxT[i] = (UInt16)j;
                        i++;
                        if (i >= FTL_CXT_SECTION_SIZE)
                        {
                            break;
                        }
                    }
                }
                VFL_ChangeFTLCxtVbn(ctxT);
                for (i = 0; i < FTL_CXT_SECTION_SIZE; i++)
                {
                    yaFTL.cxtTable[i] = ctxT[i];
                }
            }
        }
#endif

        // Set up block counters
        SetupFreeAndAllocd();

#ifdef AND_COLLECT_STATISTICS
        stFTLStatistics.ddwFTLRestoreCnt++;
        stFTLStatistics.ddwFreeBlks = yaFTL.seaState.data.freeBlocks + yaFTL.seaState.index.freeBlocks;
        stFTLStatistics.ddwValidDataPages = 0;
        stFTLStatistics.ddwValidIndexPages = 0;
#endif

        yaFTL.seaState.distCheckable = 1;
        for (i = 0, yaFTL.erasedBlockCount = 0; i < FTL_AREA_SIZE; i++)
        {
            if((yaFTL.blockArray[i].status == BLOCK_FREE) && (yaFTL.blockArray[i].eraseBeforeUse == 0))
                yaFTL.erasedBlockCount++;
        }
        CheckBlockDist();
        return FTL_SUCCESS;
    }

    tempHdr = listHead;

    pBuff = BUF_Get(BUF_MAIN_AND_SPARE);
    if (NULL == pBuff)
    {
        freeBlockList(listHead);
        return FTL_CRITICAL_ERROR;
    }
    
    if (!_allocateRestoreIndexBuffers(&brPtr, &indexTable, &tempIndexBufferSize))
    {
        freeBlockList(listHead);
        return FTL_CRITICAL_ERROR;
    }
    
    WMR_MEMSET(brPtr, 0xff, (FTL_AREA_SIZE * sizeof(BlockRangeType)));

    for (j = 0; j < yaFTL.logicalPartitionSize; j += tempIndexBufferSize)
    {
        WMR_MEMSET(indexTable, 0xff, tempIndexBufferSize * sizeof(UInt32));
        tempPtr = listHead = tempHdr;
        while (tempPtr != NULL)
        {
            if (tempPtr == listHead)
            {
                status = restoreBlock(tempPtr->blockNo, indexTable, 1, j, tempIndexBufferSize, brPtr, tempPtr->weaveSeq);
            }
            else
            {
                status = restoreBlock(tempPtr->blockNo, indexTable, 0, j, tempIndexBufferSize, brPtr, tempPtr->weaveSeq);
            }
            if (status != FTL_SUCCESS)
            {
                WMR_BOOT_FREE(indexTable);
                WMR_FREE(brPtr, FTL_AREA_SIZE * sizeof(BlockRangeType));
                freeBlockList(tempHdr);
                BUF_Release(pBuff);
                return status;
            }
            listHead = tempPtr;
            tempPtr = tempPtr->next;
            if ((j + tempIndexBufferSize) >= yaFTL.logicalPartitionSize)
            {
                tempHdr = tempPtr;
                WMR_FREE(listHead, sizeof(BlockListType));
            }
        }

        WMR_MEMSET(pBuff->pData, 0xff, BYTES_PER_PAGE);

        for (i = 0, pageBuffer = indexTable; (i < (tempIndexBufferSize * 4) / BYTES_PER_PAGE) && ((i * yaFTL.indexPageRatio + j) < yaFTL.logicalPartitionSize); i++, pageBuffer += (BYTES_PER_PAGE / 4))
        {
            WMR_MEMSET(pBuff->pData, 0xff, BYTES_PER_PAGE);

            if (WMR_MEMCMP(pageBuffer, pBuff->pData, BYTES_PER_PAGE) == 0)
            {
                if ((yaFTL.tocArray[i + ((j * 4) / BYTES_PER_PAGE)].indexPage != 0xffffffff) && (yaFTL.blockArray[(yaFTL.tocArray[i + ((j * 4) / BYTES_PER_PAGE)].indexPage) / PAGES_PER_SUBLK].validPagesINo > 0))
                {
                    yaFTL.blockArray[(yaFTL.tocArray[i + ((j * 4) / BYTES_PER_PAGE)].indexPage) / PAGES_PER_SUBLK].validPagesINo--;
                }

                if (yaFTL.tocArray[i + ((j * 4) / BYTES_PER_PAGE)].cacheIndex != 0xffff)
                {
                    yaFTL.indexCache[yaFTL.tocArray[i + ((j * 4) / BYTES_PER_PAGE)].cacheIndex].status = FREE_CACHE_PAGE;
                    yaFTL.indexCache[yaFTL.tocArray[i + ((j * 4) / BYTES_PER_PAGE)].cacheIndex].counter = 0;
                }
                yaFTL.tocArray[i + ((j * 4) / BYTES_PER_PAGE)].indexPage = 0xffffffff;
                yaFTL.tocArray[i + ((j * 4) / BYTES_PER_PAGE)].cacheIndex = 0xffff;
                continue;   /* index page does not contain any valid pages */
            }
            if (yaFTL.tocArray[i + ((j * 4) / BYTES_PER_PAGE)].indexPage != 0xffffffff)
            {
                status = _readPage(yaFTL.tocArray[i + ((j * 4) / BYTES_PER_PAGE)].indexPage, (UInt8 *)pBuff->pData, NULL, FALSE32, TRUE32, FALSE32);
                if (status != FTL_SUCCESS)
                {
                    debug(ERROR, "index page is unreadable at 0x%x", yaFTL.tocArray[i + ((j * 4) / BYTES_PER_PAGE)].indexPage);
                    WMR_MEMSET(pBuff->pData, 0xff, BYTES_PER_PAGE);
                }
            }

            if (WMR_MEMCMP(pageBuffer, pBuff->pData, BYTES_PER_PAGE) != 0)
            {
                UInt16 cE;

                /* Need to copy valid page in index cache */
                if (yaFTL.tocArray[i + ((j * 4) / BYTES_PER_PAGE)].indexPage != 0xffffffff)
                {
                    yaFTL.blockArray[(yaFTL.tocArray[i + ((j * 4) / BYTES_PER_PAGE)].indexPage) / PAGES_PER_SUBLK].validPagesINo--;
                    yaFTL.seaState.index.validPages--;
                }

                cE = findFreeCacheEntry();
                if (cE == 0xffff)
                {
                    WMR_BOOT_FREE(indexTable);
                    WMR_FREE(brPtr, FTL_AREA_SIZE * sizeof(BlockRangeType));
                    BUF_Release(pBuff);
#ifdef AND_READONLY
                    if ( !readOnly )
                    {
                        debug(ERROR, "ran out of available cache pages on toc entry %d", (i + ((j * 4) / BYTES_PER_PAGE)));
                        debug(ERROR, "can still be enough to mount OS partition");
                        freeBlockList(tempHdr);
                        return FTL_SUCCESS;
                    }
                    else
#endif
                    {
                        debug(ERROR,"ran out of available cache pages on toc entry %d", (i + ((j * 4) / BYTES_PER_PAGE)));
                        freeBlockList(tempHdr);
                        return FTL_CRITICAL_ERROR;
                    }
                }
                WMR_MEMCPY(yaFTL.indexCache[cE].indexPage, pageBuffer, BYTES_PER_PAGE);
                yaFTL.indexCache[cE].status = IC_DIRTY_PAGE;
                yaFTL.indexCache[cE].tocEntry = (UInt16)(i + ((j * 4) / BYTES_PER_PAGE));
                yaFTL.tocArray[i + ((j * 4) / BYTES_PER_PAGE)].cacheIndex = cE;
                yaFTL.tocArray[i + ((j * 4) / BYTES_PER_PAGE)].indexPage = 0xffffffff;
            }
        }
    }
    // t3=mach_absolute_time();

#ifndef AND_READONLY
    if ( !readOnly )
    {
        /* Vadim - check presence of CXT blocks . If there are none allocate them */

        if (ctxNo == 0)
        {
            i = 0;
            for (j = 0; j < (UInt32)FTL_AREA_SIZE; j++)
            {
                if (yaFTL.blockArray[j].status == BLOCK_FREE)
                {
                    yaFTL.blockArray[j].status = BLOCK_CTX_CNTR;
                    ctxT[i] = (UInt16)j;
                    i++;
                    if (i >= FTL_CXT_SECTION_SIZE)
                    {
                        break;
                    }
                }
            }
            VFL_ChangeFTLCxtVbn(ctxT);
        }
    }
#endif

    BUF_Release(pBuff);
    WMR_BOOT_FREE(indexTable);
    WMR_FREE(brPtr, FTL_AREA_SIZE * sizeof(BlockRangeType));

    // Set up block counters
    SetupFreeAndAllocd();

    // t4=mach_absolute_time();
    // debug(MISC, "time spent in restore is  %llu %llu %llu %llu %llu ",t2-t1,t22-t2,t3-t22,t4-t3,t4-t1);
    debug(INIT, "current i block %d current i page %d", yaFTL.wrState.index.block, yaFTL.wrState.index.nextPage);

    debug(INIT, "restoreMount : logicalPartitionSize %d  TOCtableEntriesNo %d \nwrState.data.block %d wrState.data.nextPage %d \n BYTES_PER_PAGE %d \n wrState.index.block %d wrState.index.nextPage %d wrState.weaveSeq %d"
          , yaFTL.logicalPartitionSize, yaFTL.TOCtableEntriesNo, yaFTL.wrState.data.block, yaFTL.wrState.data.nextPage, BYTES_PER_PAGE, yaFTL.wrState.index.block, yaFTL.wrState.index.nextPage, yaFTL.wrState.weaveSeq);
    debug(INIT, "restoreMount seaState.data.allocdBlocks %d dataSize %d FTL_SIZE %d pagesinblock %d", yaFTL.seaState.data.allocdBlocks, yaFTL.dataSize, FTL_AREA_SIZE, PAGES_PER_SUBLK);

#ifdef AND_COLLECT_STATISTICS
    stFTLStatistics.ddwFTLRestoreCnt++;
    stFTLStatistics.ddwFreeBlks = yaFTL.seaState.data.freeBlocks + yaFTL.seaState.index.freeBlocks;
    stFTLStatistics.ddwValidDataPages = yaFTL.seaState.data.validPages;
    stFTLStatistics.ddwValidIndexPages = yaFTL.seaState.index.validPages;
#endif

    yaFTL.seaState.distCheckable = 1;
    CheckBlockDist();

    for (i = 0, yaFTL.erasedBlockCount = 0; i < FTL_AREA_SIZE; i++)
    {
        if((yaFTL.blockArray[i].status == BLOCK_FREE) && (yaFTL.blockArray[i].eraseBeforeUse == 0))
            yaFTL.erasedBlockCount++;
    }
#ifndef AND_READONLY
    if ( !readOnly )
    {
        for (i = 0; ((Int32)i < FTL_AREA_SIZE); i++)
        {
            if ((yaFTL.blockArray[i].status == BLOCK_ALLOCATED) && (yaFTL.blockArray[i].validPagesDNo == 0 ))
            {
                yaFTL.blockArray[i].validPagesINo = 0;
                YAFTL_GC_Data(i, FALSE32);
                continue;
            }
            if ((yaFTL.blockArray[i].status == BLOCK_I_ALLOCATED) && (yaFTL.blockArray[i].validPagesINo == 0 ))
            {
                YAFTL_GC_Index(i, FALSE32);
            }
        }
        // Move the current blocks out :
        i = yaFTL.wrState.index.block;
        YAFTL_GC_Index(i, FALSE32); 
        if ((yaFTL.blockArray[i].status == BLOCK_FREE) && (yaFTL.blockArray[i].eraseBeforeUse != 0))
        {
            status = VFL_Erase((UInt16)i, TRUE32);
            if (status == FTL_SUCCESS)
            {
                yaFTL.blockArray[i].erasableCount++;
                yaFTL.blockArray[i].eraseBeforeUse = 0;
                yaFTL.erasedBlockCount++;
            }
        }
        
        i = yaFTL.wrState.data.block;
        yaFTL.blockArray[i].validPagesINo = 0;
        YAFTL_GC_Data(i, FALSE32);
        if ((yaFTL.blockArray[i].status == BLOCK_FREE) && (yaFTL.blockArray[i].eraseBeforeUse != 0))
        {
            status = VFL_Erase((UInt16)i, TRUE32);
            if (status == FTL_SUCCESS)
            {
                yaFTL.blockArray[i].erasableCount++;
                yaFTL.blockArray[i].eraseBeforeUse = 0;
                yaFTL.erasedBlockCount++;
            }
        }

        while ((yaFTL.indexSize - FREE_I_BLOCK_TRS) < yaFTL.seaState.index.allocdBlocks)
        {
            debug(MISC, "free index %d %d", yaFTL.indexSize, yaFTL.seaState.index.allocdBlocks);
            YAFTL_GC_Index(0xffffffff, FALSE32);
        }
    }
#endif
    for (i = 0; (Int32)i < FTL_AREA_SIZE; i++)
    {
        if ((yaFTL.blockArray[i].status == BLOCK_ALLOCATED) || (yaFTL.blockArray[i].status == BLOCK_CURRENT))
        {
            yaFTL.blockArray[i].validPagesINo = 0;
        }
#ifndef AND_READONLY
        if ( !readOnly )
        {
            if ((yaFTL.blockArray[i].status != BLOCK_FREE) && (yaFTL.blockArray[i].eraseBeforeUse == BLOCK_TO_BE_MOVED))
            {
                if ((yaFTL.blockArray[i].status == BLOCK_ALLOCATED) || (yaFTL.blockArray[i].status == BLOCK_CURRENT))
                {
                    YAFTL_GC_Data(i, FALSE32);
                    if ((yaFTL.blockArray[i].status == BLOCK_FREE) && (yaFTL.blockArray[i].eraseBeforeUse != 0))
                    {
                        status = VFL_Erase((UInt16)i, TRUE32);
                        if (status == FTL_SUCCESS)
                        {
                            yaFTL.blockArray[i].erasableCount++;
                            yaFTL.blockArray[i].eraseBeforeUse = 0;
                            yaFTL.erasedBlockCount++;
                        }
                    }
                }
                else
                {
                    YAFTL_GC_Index(i, FALSE32);
                    if ((yaFTL.blockArray[i].status == BLOCK_FREE) && (yaFTL.blockArray[i].eraseBeforeUse != 0))
                    {
                        status = VFL_Erase((UInt16)i, TRUE32);
                        if (status == FTL_SUCCESS)
                        {
                            yaFTL.blockArray[i].erasableCount++;
                            yaFTL.blockArray[i].eraseBeforeUse = 0;
                            yaFTL.erasedBlockCount++;
                        }
                    }
                }
            }
        }
#endif
    }
    debug(INIT, "restoreMount : seaState.index.validPages = %d seaState.data.validPages = %d", yaFTL.seaState.index.validPages, yaFTL.seaState.data.validPages);

    CheckBlockDist();

    return FTL_SUCCESS;
}

static BOOL32
_allocateRestoreIndexBuffers(BlockRangeType **blockRangeBuffer, UInt32 **indexTable, UInt32 *indexTableSize)
{    
    const UInt32 kBlockRangeSize = FTL_AREA_SIZE * sizeof(BlockRangeType);
    void *blockRange = NULL;
    void *table = NULL;
    UInt32 bootBufferSize;
    
    if ((NULL == blockRangeBuffer) || (NULL == indexTable))
    {
        return FALSE32;
    }
    
    blockRange = WMR_MALLOC(kBlockRangeSize);
    
    bootBufferSize = MIN_RESTORE_BUFFER;
    table = (UInt8 *)WMR_BOOT_MALLOC(&bootBufferSize);
    
    if ((NULL == table) || (NULL == blockRange))
    {
        if (NULL != blockRange)
        {
            WMR_FREE(blockRange, kBlockRangeSize);
        }
        debug(ERROR, "could not allocate enough memory for the restore");
        return FALSE32;
    }
    
    if(bootBufferSize > (MAX_RESTORE_BUFFER * sizeof(UInt32)))
    {
        bootBufferSize = (MAX_RESTORE_BUFFER) * sizeof(UInt32);
    }
    else
    {
        bootBufferSize = ROUNDDOWNTO(bootBufferSize, MIN_RESTORE_BUFFER);
    }
    
    *indexTable = (UInt32*) table;
    *blockRangeBuffer = (BlockRangeType*) blockRange;
    *indexTableSize = bootBufferSize / sizeof(UInt32);
 
    return TRUE32;
}

/*****************************************************************************/
/*                                                                           */
/* NAME                                                                      */
/*      YAFTL_Open															 */
/* DESCRIPTION                                                               */
/*      This function mounts the device using control info or restore        */
/*         if control info is dirty 					                     */
/* RETURN VALUES                                                             */
/*		FTL_SUCCESS															 */
/*				YAFTL_Init is completed										 */
/*		FTL_CRITICAL_ERROR													 */
/*				YAFTL_Init is failed										 */
/* NOTES                                                                     */
/*                                                                           */
/*****************************************************************************/
static Int32
YAFTL_Open(UInt32 *pTotalPages, UInt32 * pdwPageSize, BOOL32 nandFullRestore, BOOL32 justFormatted, UInt32 dwMinorVer, UInt32 dwOptions)
{
    UInt16 i;
#ifndef AND_READONLY
    UInt32 tempIndexCacheSize = yaFTL.indexCacheSize;
#endif    
    Int32 status = FTL_CRITICAL_ERROR;
    UInt16 cxtT[FTL_CXT_SECTION_SIZE];
    BOOL32 formatSupported = TRUE32;

    const UInt32 readOnly = ( WMR_INIT_OPEN_READONLY == (WMR_INIT_OPEN_READONLY & dwOptions) ? 1 : 0 );

    /* read out VFL_meta data . If it is not present execute full format . If it is present go and fetch "quick mount" info from CX area if it's valid .If not execute restore
       For now we need to start from fresh media which means there is no quick mount info and all blocks are free . */
#ifndef AND_READONLY    
    if(justFormatted == TRUE32)
        yaFTL.formatWasCalled = justFormatted;
    WMR_MEMSET(yaFTL.eraseNowList, 0xff, sizeof(UInt16) * ERASE_NOW_LIST_SIZE);
#endif    
    WMR_MEMSET(yaFTL.tocArray, 0xff, sizeof(TOCEntry) * yaFTL.TOCtableEntriesNo);
    WMR_MEMSET(yaFTL.blockArray, 0xff, sizeof(BlockEntry) * FTL_AREA_SIZE);
    for (i = 0; i < FTL_AREA_SIZE; i++)
    {
        yaFTL.blockArray[i].validPagesDNo = 0;
        yaFTL.blockArray[i].validPagesINo = 0;
        yaFTL.blockArray[i].erasableCount = 0;
        yaFTL.blockArray[i].pagesRead = 0;
        yaFTL.blockArray[i].pagesReadSubCounter = 0;
        yaFTL.blockArray[i].eraseBeforeUse = 0;
        yaFTL.blockArray[i].status = 0;
    }
    
    yaFTL.wrState.lastBlock = 0xffffffff;
    /*  Get CXT blocks from the VFL  */
    WMR_MEMCPY(cxtT, VFL_GetFTLCxtVbn(), FTL_CXT_SECTION_SIZE * sizeof(UInt16));

    /*
       Vadim : here we make a decision about quick mount .
       If quick mount units do not exist - go to restore
       If quick mount units exist - go with quick mount if there is a valid copy .
       Otherwise, go with restore .
     */
    if (cxtT[0] != 0xffff)
    {
        for (i = 0; i < FTL_CXT_SECTION_SIZE; i++)
        {
            yaFTL.cxtTable[i] = cxtT[i];
            yaFTL.blockArray[yaFTL.cxtTable[i]].status = BLOCK_CTX_CNTR;
            debug(OPEN, "YAFTL_Open cxt [ %d ] = 0x%x %d 0x%x %d", i, cxtT[i], cxtT[i], FTL_AREA_SIZE, FTL_AREA_SIZE);
        }

        status = (UInt16)quickRestore(cxtT, &formatSupported);
        if (formatSupported != TRUE32)
        {
            debug(ERROR, "YAFTL_OPEN unsupported low-level format version cannot continue .");
            return FTL_CRITICAL_ERROR;
        }
#ifdef AND_COLLECT_STATISTICS
        if(stFTLStatistics.ddwRefreshCnt == 0xffffffff)
        {
            debug(ERROR, "YAFTL_OPEN clearing refresh stats");
            stFTLStatistics.ddwRefreshCnt = 0;
        }
#endif        
#ifndef AND_READONLY
        if ((dwOptions & WMR_INIT_SET_INDEX_CACHE_SIZE) == WMR_INIT_SET_INDEX_CACHE_SIZE)
        {
            yaFTL.indexCacheSize = (dwOptions >> 20);
            tempIndexCacheSize = yaFTL.indexCacheSize;
            debug(INIT, "YAFTL_OPEN index cache will be set to 0x%x -- 0x%x", yaFTL.indexCacheSize, tempIndexCacheSize);
        }
        if ((yaFTL.cxtAllocation != 0) && (yaFTL.cxtAllocation != 0xffffffff)) 
            for(i = 0; i < FTL_CXT_SECTION_SIZE; i++)
                if( yaFTL.blockArray[yaFTL.cxtTable[i]].erasableCount == 0)
                    yaFTL.blockArray[yaFTL.cxtTable[i]].erasableCount = (UInt32
                                                                         )(yaFTL.cxtAllocation / (PAGES_PER_SUBLK / yaFTL.cxtSize));
#endif        
        if ((status != FTL_SUCCESS) || (nandFullRestore == TRUE32))
        {
            debug(INIT, "YAFTL_OPEN forcing full mount 0x%x %d ", status, nandFullRestore);
            status = FTL_CRITICAL_ERROR;
        }
        /* Allocate index cache based on a size saved in cxt area */
        if ((status != FTL_SUCCESS) && (WMR_MIN( NAND_MOUNT_INDEX_SIZE, ((yaFTL.logicalPartitionSize / yaFTL.indexPageRatio) + 1)) > yaFTL.indexCacheSize))
        {
#ifndef AND_READONLY            
            tempIndexCacheSize = yaFTL.indexCacheSize;
#endif            
            yaFTL.indexCacheSize = WMR_MIN( NAND_MOUNT_INDEX_SIZE, ((yaFTL.logicalPartitionSize / yaFTL.indexPageRatio) + 1));//NAND_MOUNT_INDEX_SIZE;
        }
        yaFTL.indexCache = (CacheIndexEntry *)WMR_MALLOC(yaFTL.indexCacheSize * sizeof(CacheIndexEntry));
        if (yaFTL.indexCache == NULL)
        {
            return FTL_CRITICAL_ERROR;
        }

        // Allocate index cache buffers
        WMR_BufZone_Init(&yaFTL.BufZone_IndexCache);
        for (i = 0; i < yaFTL.indexCacheSize; i++)
        {
            // Pre-reserve
            yaFTL.indexCache[i].indexPage = (UInt32 *)WMR_Buf_Alloc_ForDMA(&yaFTL.BufZone_IndexCache, BYTES_PER_PAGE);
        }

        // Finish allocations
        if (!WMR_BufZone_FinishedAllocs(&yaFTL.BufZone_IndexCache))
        {
            return FTL_CRITICAL_ERROR;
        }

        // Rebase and initialize
        for (i = 0; i < yaFTL.indexCacheSize; i++)
        {
            WMR_BufZone_Rebase(&yaFTL.BufZone_IndexCache, (void**)&yaFTL.indexCache[i].indexPage);
            yaFTL.indexCache[i].status = FREE_CACHE_PAGE;
            yaFTL.indexCache[i].tocEntry = 0xffff;
            yaFTL.indexCache[i].counter = 0;
            WMR_MEMSET(yaFTL.indexCache[i].indexPage, 0xff, BYTES_PER_PAGE);
        }

        // Finish rebase
        WMR_BufZone_FinishedRebases(&yaFTL.BufZone_IndexCache);

        if (status == FTL_SUCCESS)
        {
            debug(OPEN, "YAFTL_OPEN quick mount is present and valid . open is done ");
            *pTotalPages = ((yaFTL.logicalPartitionSize - 1) / 100) * yaFTL.exportedRatio /*EXPO_RATIO*/;
            *pdwPageSize = BYTES_PER_PAGE;
            yaFTL.maxEraseCount = 0; yaFTL.minEraseCount = 0xffffffff; yaFTL.erasedBlockCount = 0;
            for (i = 0; i < FTL_AREA_SIZE; i++)
            {
                if((yaFTL.blockArray[i].status == BLOCK_FREE) && (yaFTL.blockArray[i].eraseBeforeUse == 0))
                    yaFTL.erasedBlockCount++;
                if ((yaFTL.blockArray[i].status != BLOCK_CTX_CNTR) && (yaFTL.blockArray[i].status != BLOCK_CTX_CURRENT))
                {
                    if (yaFTL.blockArray[i].erasableCount > yaFTL.maxEraseCount)
                    {
                        yaFTL.maxEraseCount = yaFTL.blockArray[i].erasableCount;
                    }
                    if (yaFTL.blockArray[i].erasableCount < yaFTL.minEraseCount)
                    {
                        yaFTL.minEraseCount = yaFTL.blockArray[i].erasableCount;
                    }
                }
            }

            // Set up block counters
            SetupFreeAndAllocd();
            BTOC_BootFixup();

            PopulateTreesOnBoot_Fast();
            for (i = 0, yaFTL.erasedBlockCount = 0; i < FTL_AREA_SIZE; i++)
            {
                if((yaFTL.blockArray[i].status == BLOCK_FREE) && (yaFTL.blockArray[i].eraseBeforeUse == 0))
                    yaFTL.erasedBlockCount++;
            }
            return status;
        }
    }
    for (i = 0; i < FTL_AREA_SIZE; i++)
    {
        yaFTL.blockArray[i].validPagesDNo = 0;
        yaFTL.blockArray[i].validPagesINo = 0;
        if ((yaFTL.blockArray[i].status == BLOCK_CTX_CNTR) || (yaFTL.blockArray[i].status == BLOCK_CTX_CURRENT))
        {
            continue;
        }
        else
            yaFTL.blockArray[i].status = 0;
    }
    debug(ERROR, "CXT is not valid . Performing full NAND R/O restore ...  ");
#ifdef AND_USE_NOTIFY
    {
        LowFuncTbl  *pLowFuncTbl;

        pLowFuncTbl = FIL_GetFuncTbl();
        pLowFuncTbl->Notify(AND_NOTIFY_RECOVER);
    }
#endif /* AND_USE_NOTIFY */

    status = (UInt16)restoreMount(cxtT, readOnly);
    if (status != FTL_SUCCESS)
    {
#ifndef AND_READONLY
        debug(ERROR, "Read only full restore failed. Performing full NAND R/W restore ...  ");
        for (i = 0; i < yaFTL.indexCacheSize; i++)
        {
            yaFTL.indexCache[i].status = FREE_CACHE_PAGE;
            yaFTL.indexCache[i].tocEntry = 0xffff;
            yaFTL.indexCache[i].counter = 0;
            WMR_MEMSET(yaFTL.indexCache[i].indexPage, 0xff, BYTES_PER_PAGE);
        }

        for (i = 0; i < FTL_AREA_SIZE; i++)
        {
            yaFTL.blockArray[i].validPagesDNo = 0;
            yaFTL.blockArray[i].validPagesINo = 0;
            if ((yaFTL.blockArray[i].status == BLOCK_CTX_CNTR) || (yaFTL.blockArray[i].status == BLOCK_CTX_CURRENT))
            {
                continue;
            }
            else
                yaFTL.blockArray[i].status = 0;
        }

        status = restoreMountEraseIndex(cxtT);
        if (status != FTL_SUCCESS)
#endif
        return status;
    }

    BTOC_BootFixup();
    
    // force next cxt update out of the current cxt block ( unclean shutdown )
    if(yaFTL.currentCxtVpn != 0xffffffff)
        yaFTL.currentCxtVpn = ((yaFTL.currentCxtVpn / PAGES_PER_SUBLK) * PAGES_PER_SUBLK) + (PAGES_PER_SUBLK - 1);

    // Detect min/max erase counts
    for (i = 0; i < FTL_AREA_SIZE; i++)
    {
        yaFTL.maxEraseCount = 0; yaFTL.minEraseCount = 0xffffffff;
        if ((yaFTL.blockArray[i].status != BLOCK_CTX_CNTR) && (yaFTL.blockArray[i].status != BLOCK_CTX_CURRENT))
        {
            if (yaFTL.blockArray[i].erasableCount > yaFTL.maxEraseCount)
            {
                yaFTL.maxEraseCount = yaFTL.blockArray[i].erasableCount;
            }
            if (yaFTL.blockArray[i].erasableCount < yaFTL.minEraseCount)
            {
                yaFTL.minEraseCount = yaFTL.blockArray[i].erasableCount;
            }
        }
    }

    *pTotalPages = ((yaFTL.logicalPartitionSize - 1) / 100) * yaFTL.exportedRatio /*EXPO_RATIO*/;
    *pdwPageSize = BYTES_PER_PAGE;
#ifndef AND_READONLY
    if ( !readOnly )
    {
        if (yaFTL.indexCacheSize != tempIndexCacheSize)
        {
            UInt32 tmpVar = (UInt32)tempIndexCacheSize;
            YAFTL_GetStruct(AND_FUNCTION_INDEX_CACHE_UPDATE, NULL, (UInt32 *)&tmpVar);
        }
    }
#endif

    PopulateTreesOnBoot_Fast();
    for (i = 0, yaFTL.erasedBlockCount = 0; i < FTL_AREA_SIZE; i++)
    {
        if((yaFTL.blockArray[i].status == BLOCK_FREE) && (yaFTL.blockArray[i].eraseBeforeUse == 0))
            yaFTL.erasedBlockCount++;
    }
    return FTL_SUCCESS;
}


#if ENABLE_L2V_TREE
static void PopulateFromIndexCache(UInt32 indexPageNo, UInt32 *indexPageData)
{
    UInt32 lpn;
    UInt32 i, lastLpn, lastPhy, lastLen;
    UInt32 *phyPtr;

    lpn = indexPageNo * yaFTL.indexPageRatio;
    lastLpn = 0xffffffff;
    lastPhy = 0xffffffff;
    lastLen = 0;
    phyPtr = indexPageData;

    for (i = 0; (i < yaFTL.indexPageRatio) && !L2V_LowMem; i++, phyPtr++)
    {
        if (0 == lastLen)
        {
            lastLpn = lpn + i;
            lastPhy = *phyPtr;
            lastLen = 1;
            continue;
        }

        if (((0xffffffff != lastPhy) && ((lastPhy + lastLen) == *phyPtr)) || ((0xffffffff == lastPhy) && (lastPhy == *phyPtr)))
        {
            lastLen++;
        }
        else
        {
            if (0xffffffff == lastPhy) {
                L2V_Update(lastLpn, lastLen, L2V_VPN_DEALLOC);
            } else {
                L2V_Update(lastLpn, lastLen, lastPhy);
            }

            lastLpn = lpn + i;
            lastPhy = *phyPtr;
            lastLen = 1;
        }
    }

    if (lastLen && !L2V_LowMem)
    {
        if (0xffffffff == lastPhy) {
            L2V_Update(lastLpn, lastLen, L2V_VPN_DEALLOC);
        } else {
            L2V_Update(lastLpn, lastLen, lastPhy);
        }
    }
}
#endif // ENABLE_L2V_TREE


#if ENABLE_L2V_TREE
#ifdef AND_READONLY
#error Tree not supported in read-only AND driver. This would require GC to allocate index buffer in AND_READONLY mode, since we share that to populate trees.
#endif
static void PopulateTreesOnBoot_Fast()
{
    UInt32 zoneSize, indexPageNo, successCount, failCount, i;
#if TIME_TREE
    UInt64 startTime;
#endif
    BOOL32 BoolStatus;
    Int32  status;

    indexPageNo = 0;
    successCount = 0;
    failCount = 0;

#if TIME_TREE
    (void) clock_timebase_info(&sTimebaseInfo);
    startTime = ANDPerf_getTime();
#endif

    while ((indexPageNo < yaFTL.TOCtableEntriesNo) && !L2V_LowMem) {
        // Fill out VPNs
        zoneSize = 0;
        while (((indexPageNo + zoneSize) < yaFTL.TOCtableEntriesNo) && (zoneSize < yaFTL.gc.zoneSize))
        {
            if (0xffffffff != yaFTL.tocArray[indexPageNo+zoneSize].indexPage) {
                yaFTL.gc.index.vpns[zoneSize] = yaFTL.tocArray[indexPageNo+zoneSize].indexPage;
                zoneSize++;
            } else {
                // Creates gaps.  Ok so long as we're careful about inferring starts/ends, and use meta read from page, not calculated
                indexPageNo++;
            }
        }

        // Read index pages
        BoolStatus = _readMultiPages(yaFTL.gc.index.vpns, zoneSize, &yaFTL.gc.index.zone[0], (UInt8*)&yaFTL.gc.index.meta[0], 0, 0);
        if (BoolStatus != TRUE32)
        {
            for (i = 0; i < zoneSize; i++)
            {
                status = _readPage(yaFTL.gc.index.vpns[i], &yaFTL.gc.index.zone[BYTES_PER_PAGE * i], &yaFTL.gc.index.meta[i], 0, TRUE32, 0);
                if (status != FTL_SUCCESS)
                {
                    META_SET_LBA(&yaFTL.gc.index.meta[i], 0xffffffff);
                }
            }
        }

        // Now scan the results--if it's the current one, update using it
        for (i = 0; (i < zoneSize) && !L2V_LowMem; i++)
        {
            if ((META_IS_IDX(&yaFTL.gc.index.meta[i])) && (yaFTL.TOCtableEntriesNo > META_GET_IPN(&yaFTL.gc.index.meta[i])))
            {
                PopulateFromIndexCache(META_GET_IPN(&yaFTL.gc.index.meta[i]), (UInt32*)&yaFTL.gc.index.zone[BYTES_PER_PAGE * i]);
                successCount++;
            }
            else
            {
                failCount++;
            }
        }

        indexPageNo += zoneSize;
    }

#if TIME_TREE
    WMR_PRINT("NAND cache population time: %dus\n", ANDPerf_toUs(ANDPerf_getTime() - startTime));
#endif
}
#else  // if not ENABLE_L2V_TREE:
static void PopulateTreesOnBoot_Fast()
{
    // Do nothing
}
#endif // !ENABLE_L2V_TREE


#ifndef AND_READONLY
/*****************************************************************************/
/*                                                                           */
/* NAME                                                                      */
/*      allocateBlock														 */
/* DESCRIPTION                                                               */
/*     Allocates free block for either index or data update operartions      */
/*                                                                           */
/*****************************************************************************/
UInt32 allocateBlock(UInt32 blockNo, UInt8 flag, UInt8 type)
{
    UInt32 i;
    UInt32 minCount = 0xffffffff, minCountIndex = 0xffffffff;
    UInt16 blockToEraseForReal = 0;

    // Wear leveling periodicity counter
    yaFTL.wearLevel.blocksSince++;
    
    if(yaFTL.erasedBlockCount < (FREE_BLK_TRH + FREE_I_BLOCK_TRS))
    {
        blockToEraseForReal = (FREE_BLK_TRH + FREE_I_BLOCK_TRS) - yaFTL.erasedBlockCount; 
    }
    else
        blockToEraseForReal = 0;

    // Set up minWeaveSeq, clear out data gc adjust
    if (1 == type)
    {
        yaFTL.wrState.data.minWeaveSeq = yaFTL.wrState.weaveSeq;
        yaFTL.gc.data.gcDataAdjust = 0;
    }
    else
    {
        yaFTL.wrState.index.minWeaveSeq = yaFTL.wrState.weaveSeq;
    }

    // look for a block with smallest erasable unit count
    for (i = 0; (Int32)i < FTL_AREA_SIZE; i++)
    {
        if((type == 1) && (yaFTL.blockArray[i].status == BLOCK_FREE) && ((yaFTL.blockArray[i].eraseBeforeUse == BLOCK_TO_BE_ERASED_ALIGNED_DATA) || (yaFTL.blockArray[i].eraseBeforeUse == BLOCK_TO_BE_ERASED)) )
        {
            if((isBlockInEraseNowList(i)) || (blockToEraseForReal > 0))
            {
                ANDStatus status;
                
                // Invalidate context
                if (yaFTL.cxtValid == 1)
                { 
                    invalidateCXT();
                    yaFTL.cxtValid = 0;
                }
                // Erase it
                status = VFL_Erase(i, TRUE32);
                if (status != FTL_SUCCESS)
                {
                    WMR_PANIC("VFL_Erase(%d) failed with 0x%08x", (UInt32) i, status);
                    return 0xffffffff;
                }
                yaFTL.erasedBlockCount++;
                if(blockToEraseForReal > 0)
                {
                    blockToEraseForReal--;
                }
                yaFTL.blockArray[i].eraseBeforeUse = 0;
                yaFTL.blockArray[i].erasableCount++;
                yaFTL.periodicCxt.erases++;
                if (yaFTL.blockArray[i].erasableCount > yaFTL.maxEraseCount)
                {
                    yaFTL.maxEraseCount = yaFTL.blockArray[i].erasableCount;
                }
                removeBlockFromEraseNowList((UInt16)i);
            }
            else
            {
                yaFTL.blockArray[i].eraseBeforeUse = BLOCK_TO_BE_ERASED; 
            }
        }
        else 
        {
            if((type != 1) && (yaFTL.blockArray[i].status == BLOCK_FREE) && ((yaFTL.blockArray[i].eraseBeforeUse == BLOCK_TO_BE_ERASED_ALIGNED_INDEX) || (yaFTL.blockArray[i].eraseBeforeUse == BLOCK_TO_BE_ERASED)))
            {
                if((isBlockInEraseNowList(i)) || (blockToEraseForReal > 0))
                {
                    ANDStatus status;
                    
                    // Invalidate context
                    if (yaFTL.cxtValid == 1)
                    {
                        invalidateCXT();
                        yaFTL.cxtValid = 0;
                    }
                    // Erase it
                    status = VFL_Erase(i, TRUE32);
                    if (status != FTL_SUCCESS)
                    {
                        WMR_PANIC("VFL_Erase(%d) failed with 0x%08x", (UInt32) i, status);
                        return 0xffffffff;
                    }
                    yaFTL.erasedBlockCount++;
                    if(blockToEraseForReal > 0)
                    {
                        blockToEraseForReal--;
                    }
                    removeBlockFromEraseNowList((UInt16)i);
                    yaFTL.blockArray[i].eraseBeforeUse = 0;
                    yaFTL.blockArray[i].erasableCount++;
                    yaFTL.periodicCxt.erases++;
                    if (yaFTL.blockArray[i].erasableCount > yaFTL.maxEraseCount)
                    {
                        yaFTL.maxEraseCount = yaFTL.blockArray[i].erasableCount;
                    }
                }
                else
                {
                    yaFTL.blockArray[i].eraseBeforeUse = BLOCK_TO_BE_ERASED;
                }
            }
        }
         
        if ((yaFTL.blockArray[i].status == BLOCK_FREE) && (yaFTL.blockArray[i].eraseBeforeUse != BLOCK_TO_BE_ERASED_ALIGNED_DATA) && (yaFTL.blockArray[i].eraseBeforeUse != BLOCK_TO_BE_ERASED_ALIGNED_INDEX) && (minCount > yaFTL.blockArray[i].erasableCount))
        {
            minCount = yaFTL.blockArray[i].erasableCount;
            minCountIndex = i;
            WMR_ASSERT(0 == yaFTL.blockArray[i].validPagesINo);
            WMR_ASSERT(0 == yaFTL.blockArray[i].validPagesDNo);
        }
    }
    if (minCountIndex == 0xffffffff)
    {
        // Invalidate context
        if (yaFTL.cxtValid == 1)
        {
            invalidateCXT();
            yaFTL.cxtValid = 0;
        }
        WMR_PANIC("no free yaFTL blocks: type %d\n", type);
        return minCountIndex;
    }
    if (type == 1)    /* dataBlock */
    {
        BTOC_Dealloc(yaFTL.wrState.data.TOC);
#ifndef AND_READONLY
        if (yaFTL.blockArray[minCountIndex].eraseBeforeUse == BLOCK_TO_BE_ERASED)
        {
            UInt8 status;
            WMR_ASSERT(BLOCK_FREE == yaFTL.blockArray[minCountIndex].status);
            if (yaFTL.cxtValid == 1)
            {
                invalidateCXT();
                yaFTL.cxtValid = 0;
            }
            status = VFL_Erase((UInt16)minCountIndex, TRUE32);
            if (status != FTL_SUCCESS)
            {
                return 0xffffffff;
            }
            yaFTL.erasedBlockCount++;
            removeBlockFromEraseNowList((UInt16)minCountIndex);
            yaFTL.blockArray[minCountIndex].erasableCount++;
            yaFTL.periodicCxt.erases++;
            yaFTL.blockArray[minCountIndex].eraseBeforeUse = 0;
        }
#endif
        yaFTL.blockArray[yaFTL.wrState.data.block].status = BLOCK_ALLOCATED;
        yaFTL.wrState.data.block = minCountIndex;
        yaFTL.blockArray[minCountIndex].status = BLOCK_CURRENT;
        yaFTL.wrState.data.nextPage = 0;
        yaFTL.wrState.data.TOC = BTOC_Alloc(yaFTL.wrState.data.block, 1);
        WMR_MEMSET(yaFTL.wrState.data.TOC, 0xff, yaFTL.controlPageNo * BYTES_PER_PAGE);
        yaFTL.seaState.data.freeBlocks--;
        yaFTL.seaState.freeBlocks--;
        if(yaFTL.erasedBlockCount)
            yaFTL.erasedBlockCount--;
#ifdef AND_COLLECT_STATISTICS
        stFTLStatistics.ddwFreeBlks--;
#endif
        yaFTL.seaState.data.allocdBlocks++;
        //wrState.data.weaveSeq++;
        return yaFTL.wrState.data.block;
    }
    else
    {
        BTOC_Dealloc(yaFTL.wrState.index.TOC);
#ifndef AND_READONLY
        if (yaFTL.blockArray[minCountIndex].eraseBeforeUse == BLOCK_TO_BE_ERASED)
        {
            UInt8 status;
            if (yaFTL.cxtValid == 1)
            {
                invalidateCXT();
                yaFTL.cxtValid = 0;
            }
            status = VFL_Erase((UInt16)minCountIndex, TRUE32);
            if (status != FTL_SUCCESS)
            {
                return 0xffffffff;
            }
            yaFTL.erasedBlockCount++;
            removeBlockFromEraseNowList((UInt16)minCountIndex);
            yaFTL.blockArray[minCountIndex].erasableCount++;
            yaFTL.periodicCxt.erases++;
            yaFTL.blockArray[minCountIndex].eraseBeforeUse = 0;
        }
#endif
        yaFTL.blockArray[yaFTL.wrState.index.block].status = BLOCK_I_ALLOCATED;
        yaFTL.wrState.index.block = minCountIndex;
        yaFTL.blockArray[minCountIndex].status = BLOCK_I_CURRENT;
        yaFTL.wrState.index.nextPage = 0;
        yaFTL.wrState.index.TOC = BTOC_Alloc(yaFTL.wrState.index.block, 0);
        WMR_MEMSET(yaFTL.wrState.index.TOC, 0xff, yaFTL.controlPageNo * BYTES_PER_PAGE);
        yaFTL.seaState.index.freeBlocks--;
        yaFTL.seaState.freeBlocks--;
        if(yaFTL.erasedBlockCount)
            yaFTL.erasedBlockCount--;
#ifdef AND_COLLECT_STATISTICS
        stFTLStatistics.ddwFreeBlks--;
#endif
        yaFTL.seaState.index.allocdBlocks++;

        return yaFTL.wrState.index.block;
    }
}
#endif //ifndef AND_READONLY

/*****************************************************************************/
/*                                                                           */
/* NAME                                                                      */
/*      findFreeCacheEntry													 */
/* DESCRIPTION                                                               */
/*     Finds free entry in index cache table                                 */
/*                                                                           */
/*****************************************************************************/
UInt16 findFreeCacheEntry(void)
{
    UInt16 i;

    for (i = 0; i < yaFTL.indexCacheSize; i++)
    {
        if (yaFTL.indexCache[i].status == FREE_CACHE_PAGE)
        {
            return i;
        }
    }
    return 0xffff;
}

#ifndef AND_READONLY
UInt16 clearEntryInCache(UInt8 flag, UInt16 candidate, UInt8 keepInCache)
{
    UInt16 i;
    UInt16 bestCandidateCounterBusy = 0xffff, bestCandidateBusy = 0xffff, bestCandidateCounterDirty = 0xffff, bestCandidateDirty = 0xffff;
    UInt8 status;

    if (candidate == 0xffff)
    {
        for (i = 0; i < yaFTL.indexCacheSize; i++)
        {
            if (yaFTL.indexCache[i].status == FREE_CACHE_PAGE)
            {
                return i;
            }
            if ((yaFTL.indexCache[i].status == IC_BUSY_PAGE) && (yaFTL.indexCache[i].counter <= bestCandidateCounterBusy))
            {
                bestCandidateBusy = i;
                bestCandidateCounterBusy = yaFTL.indexCache[i].counter;
            }
            if ((yaFTL.indexCache[i].status == IC_DIRTY_PAGE) && (yaFTL.indexCache[i].counter <= bestCandidateCounterDirty))
            {
                bestCandidateDirty = i;
                bestCandidateCounterDirty = yaFTL.indexCache[i].counter;
            }
        }
    }
    else
    {
        // Note: bestCandidateCounterBusy and bestCandidateCounterDirty are not updated by this path, since they are not needed.
        if (yaFTL.indexCache[candidate].status == FREE_CACHE_PAGE)
        {
            return candidate;
        }
        if (yaFTL.indexCache[candidate].status == IC_BUSY_PAGE)
        {
            bestCandidateBusy = candidate;
        }
        if (yaFTL.indexCache[candidate].status == IC_DIRTY_PAGE)
        {
            bestCandidateDirty = candidate;
        }
    }

    if (bestCandidateBusy != 0xffff)
    {
        if(keepInCache == 0)
        {
            yaFTL.tocArray[yaFTL.indexCache[bestCandidateBusy].tocEntry].cacheIndex = 0xffff;
            WMR_MEMSET(yaFTL.indexCache[bestCandidateBusy].indexPage, 0xff, BYTES_PER_PAGE);
            yaFTL.indexCache[bestCandidateBusy].status = FREE_CACHE_PAGE;
            yaFTL.indexCache[bestCandidateBusy].counter = 0;
            yaFTL.freeCachePages++;
        }
        return bestCandidateBusy;
    }

    if (bestCandidateDirty != 0xffff)
    {
        PageMeta_t *mdPtr = yaFTL.meta_clearEntryInCache;
        UInt32 tocEntry = yaFTL.indexCache[bestCandidateDirty].tocEntry;

        SetupMeta_Index(mdPtr, tocEntry);

        if (yaFTL.tocArray[tocEntry].indexPage != 0xffffffff)
        {
            WMR_ASSERT(0 != yaFTL.blockArray[yaFTL.tocArray[tocEntry].indexPage / PAGES_PER_SUBLK].validPagesINo);
            yaFTL.blockArray[(yaFTL.tocArray[tocEntry].indexPage) / PAGES_PER_SUBLK].validPagesINo--;
            yaFTL.seaState.index.validPages--;
#ifdef AND_COLLECT_STATISTICS
            stFTLStatistics.ddwValidIndexPages--;
#endif
        }
        status = (UInt8)writeIndexPage((UInt8 *)yaFTL.indexCache[bestCandidateDirty].indexPage, mdPtr, flag);
        if (FTL_SUCCESS != status)
        {
            // Invalidate context
            if (yaFTL.cxtValid == 1)
            {
                invalidateCXT();
                yaFTL.cxtValid = 0;
            }
            WMR_PANIC("Couldn't write index page %d in clearEntryInCache", yaFTL.indexCache[bestCandidateDirty].indexPage);
        }

        //  collectBlockStats();
        if(keepInCache == 0)
        {
            yaFTL.tocArray[tocEntry].cacheIndex = 0xffff;
            yaFTL.indexCache[bestCandidateDirty].status = FREE_CACHE_PAGE;
            yaFTL.indexCache[bestCandidateDirty].counter = 0;
            WMR_MEMSET(yaFTL.indexCache[bestCandidateDirty].indexPage, 0xff, BYTES_PER_PAGE);
            yaFTL.freeCachePages++;
        }
        else
        {
            yaFTL.indexCache[bestCandidateDirty].status = IC_BUSY_PAGE;
        }
        return bestCandidateDirty;
    }

    return 0xffff;
}
#endif

#ifndef AND_READONLY
/*****************************************************************************/
/*                                                                           */
/* NAME                                                                      */
/*      writeCurrentBlockTOC												 */
/* DESCRIPTION                                                               */
/*     writes block TOC info once block is full                              */
/*                                                                           */
/*****************************************************************************/
ANDStatus writeCurrentBlockTOC(UInt8 type)
{
    UInt8 i;
    Int32 status;
    UInt32 Vpn;
    PageMeta_t *md = yaFTL.meta_writeBTOC;

    if (type == 1)
    {
        SetupMeta_Data_BTOC(md);

        Vpn = yaFTL.wrState.data.nextPage + (yaFTL.wrState.data.block * PAGES_PER_SUBLK);
        if (yaFTL.wrState.data.nextPage == (PAGES_PER_SUBLK - yaFTL.controlPageNo))
        {
            for (i = 0; i < yaFTL.controlPageNo; i++, Vpn++)
            {
                status = _writePage(Vpn, ((UInt8*)yaFTL.wrState.data.TOC) + (i * BYTES_PER_PAGE), md, FALSE32);
                if (status != FTL_SUCCESS)
                {
                    return FTL_CRITICAL_ERROR;
                }
                yaFTL.wrState.data.nextPage++;
            }
        }
    }
    else
    {
        SetupMeta_Index_BTOC(md);

        Vpn = yaFTL.wrState.index.nextPage + (yaFTL.wrState.index.block * PAGES_PER_SUBLK);
        if (yaFTL.wrState.index.nextPage == (PAGES_PER_SUBLK - yaFTL.controlPageNo))
        {
            for (i = 0; i < yaFTL.controlPageNo; i++, Vpn++)
            {
                status = _writePage(Vpn, ((UInt8*)yaFTL.wrState.index.TOC) + (i * BYTES_PER_PAGE), md, FALSE32);
                if (status != FTL_SUCCESS)
                {
                    return FTL_CRITICAL_ERROR;
                }
                yaFTL.wrState.index.nextPage++;
            }
        }
    }
    return FTL_SUCCESS;
}

#endif

#ifndef AND_READONLY
/*****************************************************************************/
/*                                                                           */
/* NAME                                                                      */
/*      flushIndexCache												         */
/* DESCRIPTION                                                               */
/*     commits index cached pages to flash                                   */
/*                                                                           */
/*****************************************************************************/
static ANDStatus flushIndexCache(UInt8 keepInCache)
{
    UInt16 i;

    while ((yaFTL.indexSize - FREE_I_BLOCK_TRS) < yaFTL.seaState.index.allocdBlocks)
    {
        YAFTL_GC_Index(0xffffffff, TRUE32);
    }

    for (i = 0; i < yaFTL.indexCacheSize; i++)
    {
        clearEntryInCache(0, i, keepInCache);
    }

    return FTL_SUCCESS;
}

#endif

#ifndef AND_READONLY
/*****************************************************************************/
/*                                                                           */
/* NAME                                                                      */
/*      writeIndexPage												         */
/* DESCRIPTION                                                               */
/*     writes index page to flash                                            */
/*                                                                           */
/*****************************************************************************/
static ANDStatus writeIndexPage(UInt8 *pageBuffer, PageMeta_t *mdPtr, UInt8 flag)
{
    UInt32 Vpn;
    ANDStatus status;

 ONEMORETIME:
    if (yaFTL.wrState.index.nextPage < (PAGES_PER_SUBLK - yaFTL.controlPageNo))
    {
        /* we have space in current index block  */
        Vpn = yaFTL.wrState.index.nextPage + (yaFTL.wrState.index.block * PAGES_PER_SUBLK); /* should use << if possible */
        BTOC_SET_FROM_IPN(yaFTL.wrState.index.TOC[yaFTL.wrState.index.nextPage], mdPtr);
        META_SET_WEAVESEQ(mdPtr);
        status = _writePage(Vpn, pageBuffer, mdPtr, FALSE32);
        if (status != FTL_SUCCESS)
        {
            debug(WRITE, "cannot write index page ");
            YAFTL_GC_Index_Enq(yaFTL.wrState.index.block);
            allocateBlock(yaFTL.wrState.index.block, 0, 0);

            goto ONEMORETIME;
        }
        yaFTL.wrState.index.nextPage++;
        yaFTL.blockArray[yaFTL.wrState.index.block].validPagesINo++;
        yaFTL.seaState.index.validPages++;

#ifdef AND_COLLECT_STATISTICS
        stFTLStatistics.ddwValidIndexPages++;
#endif
    }
    else
    {
        /* current block is full . need to allocate new block */
        writeCurrentBlockTOC(0);
        allocateBlock(yaFTL.wrState.index.block, flag, 0);

        Vpn = yaFTL.wrState.index.nextPage + (yaFTL.wrState.index.block * PAGES_PER_SUBLK); // should use << if possible
        WMR_ASSERT(0 == yaFTL.wrState.index.nextPage);
        BTOC_SET_FROM_IPN(yaFTL.wrState.index.TOC[yaFTL.wrState.index.nextPage], mdPtr);
        META_SET_WEAVESEQ(mdPtr);
        status = _writePage(Vpn, pageBuffer, mdPtr, FALSE32);
        if (status != FTL_SUCCESS)
        {
            debug(WRITE, "cannot write index page ");
            YAFTL_GC_Index_Enq(yaFTL.wrState.index.block);
            allocateBlock(yaFTL.wrState.index.block, 0, 0);

            goto ONEMORETIME;
        }
        yaFTL.blockArray[yaFTL.wrState.index.block].validPagesINo++;
        yaFTL.seaState.index.validPages++;

#ifdef AND_COLLECT_STATISTICS
        stFTLStatistics.ddwValidIndexPages++;
#endif
        yaFTL.wrState.index.nextPage++;
    }

    yaFTL.tocArray[META_GET_IPN(mdPtr)].indexPage = Vpn;
    return FTL_SUCCESS;
}
#endif

#ifndef AND_READONLY


static ANDStatus makeErasable(UInt32 block)
{
    if((block < FTL_AREA_SIZE) && (yaFTL.blockArray[block].status == BLOCK_ALLOCATED) && (yaFTL.blockArray[block].validPagesDNo == 0))
    {
    YAFTL_GC_Data_Deq_sb(block);
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
    }
    return FTL_SUCCESS;
}

/*****************************************************************************/
/*                                                                           */
/* NAME                                                                      */
/*      YAFTL_DeleteSectors												     */
/* DESCRIPTION                                                               */
/*     marks range of logical pages that are deleted or about to be deleted  */
/*     as not valid                                                          */
/*                                                                           */
/*****************************************************************************/
static void YAFTL_DeleteSectors(UInt32 lba, UInt32 count)
{
    UInt32 i, indexOfs;
    UInt32 *indexPage = NULL;
    UInt32 oldVpn, oldBlock, oldBlockValid;
    UInt32 newZeroValid = 0, entry;
    
    if (yaFTL.cxtValid) {   
        invalidateCXT();
        yaFTL.cxtValid = 0;
    }

    // Index: decrement valid in sb, and global validPages, update vpn, erase?
    for (i = 0; i < count; i++) {
        // Load index page if it's not valid
#if ENABLE_L2V_TREE
        if(indexPage == NULL)
        {
            yaFTL.read_c.span = 0;
            yaFTL.read_c.lba = lba + i;
            L2V_Search(&yaFTL.read_c);
            WMR_ASSERT(0 != yaFTL.read_c.span);
            if (L2V_VPN_DEALLOC == yaFTL.read_c.vpn)
            {
                i += (yaFTL.read_c.span -1);
                continue;
            }
        }
#endif // ENABLE_L2V_TREE        
        if (indexPage == NULL)
        {
            indexPage = IndexLoadClean(lba + i, &indexOfs, &entry);
        }
        
        if(indexPage == NULL)
        {
            i += (yaFTL.indexPageRatio - (((lba + i) % yaFTL.indexPageRatio) + 1));
            continue;
        }
        
        // Take care of the old
        oldVpn = indexPage[indexOfs];
        oldBlockValid = 0xffffffff;
        oldBlock = oldVpn / PAGES_PER_SUBLK;
        if (0xffffffff != oldVpn) 
        {
            yaFTL.seaState.data.validPages--;
            WMR_ASSERT(0 != yaFTL.blockArray[oldBlock].validPagesDNo);
            oldBlockValid = --yaFTL.blockArray[oldBlock].validPagesDNo;
            indexPage[indexOfs] = 0xffffffff;
            IndexMarkDirty(entry);
#ifdef AND_COLLECT_STATISTICS
            stFTLStatistics.ddwValidDataPages--;
#endif
        }
        indexOfs++;

        // Ready to erase?
        if ((0 == oldBlockValid)
            && (BLOCK_ALLOCATED == yaFTL.blockArray[oldBlock].status))
        {
            makeErasable(oldBlock);
            newZeroValid++;
        }

        // Invalidate current index page if we've passed the end
        if (indexOfs >= yaFTL.indexPageRatio) 
        {
            indexPage = NULL;
        }
    }
    
#if ENABLE_L2V_TREE
    L2V_Update(lba, count, L2V_VPN_DEALLOC);
#endif // ENABLE_L2V_TREE    
}

/*****************************************************************************/
/*                                                                           */
/* NAME                                                                      */
/*      YAFTL_Unmap                                                          */
/* DESCRIPTION                                                               */
/*     spans trim                                                            */
/*                                                                           */
/*****************************************************************************/
static ANDStatus YAFTL_Unmap(FTLExtent_t *extents, UInt32 numExtents)
{
    while (numExtents--)
    {
        YAFTL_DeleteSectors((UInt32)extents->lba, (UInt32)extents->span);
        extents++;
    }

    return FTL_SUCCESS;
}

#endif

BOOL32 _verifyMetaData(UInt32 lpn, PageMeta_t * pmdPtr, UInt32 numOfPages)
{
    UInt32 i;

    for (i = 0; i < numOfPages; i++)
    {
        if (META_GET_LBA(&pmdPtr[i]) != (lpn + i))
        {
#ifndef AND_READONLY
            // Invalidate context
            if (yaFTL.cxtValid == 1)
            {
                invalidateCXT();
                yaFTL.cxtValid = 0;
            }
            WMR_PANIC("Yaftl_read error .mismatch between lpn and metadata at lpn 0x%08x meta 0x%08x", lpn + i, META_GET_LBA(&pmdPtr[i]));
#else
            WMR_PRINT(QUAL_FATAL, "Yaftl_read error .mismatch between lpn and metadata at lpn 0x%08x meta 0x%08x", lpn + i, META_GET_LBA(&pmdPtr[i]));
#endif
            return FALSE32;
        }
        // Check for uECC mark
        if (META_IS_UECC(&pmdPtr[i])) {
            return FALSE32;
        }
    }
    return TRUE32;
}



/*****************************************************************************/
/*                                                                           */
/* NAME                                                                      */
/*      YAFTL_Read												             */
/* DESCRIPTION                                                               */
/*     reads out a range of consecutive logical pages                        */
/*                                                                           */
/*****************************************************************************/
//#include <debug.h>

static Int32
YAFTL_Read(UInt32 dwLpn, UInt32 dwNumOfPages, UInt8 *pBuf)
{
    UInt32 i, vpnOffset = 0;
    UInt8 *currBatch = NULL;
    UInt32 vPN /*,*tPtr=NULL*/;
    UInt16 cacheIndexPtr;
    UInt8 *currPage = pBuf;
    Int32 status;
    ANDStatus statusRet = FTL_SUCCESS;
    UInt32 * vpnT = NULL;
    UInt32 indexPageNo, indexPageSub, indexPage;

    if((pBuf == NULL) && (dwNumOfPages != 1))
        return FTL_CRITICAL_ERROR;
#ifdef AND_COLLECT_STATISTICS
    if(pBuf != NULL)
        stFTLStatistics.ddwFTLReadCnt++;
    if((dwNumOfPages > 0) && (dwNumOfPages <= 8))
    {
        stFTLStatistics.ddwRead[dwNumOfPages-1]++;
    }
    else
    {
        if(dwNumOfPages > 8)
            stFTLStatistics.ddwReadOver8++;
    }
#endif
    /* take care of a simple case of reading single sector and then take it from there */
    if ((dwLpn + dwNumOfPages) >= yaFTL.logicalPartitionSize)
    {
        return FTL_CRITICAL_ERROR;
    }

    if (FIL_GetFuncTbl()->RegisterCurrentTransaction)
    {
        FIL_GetFuncTbl()->RegisterCurrentTransaction(dwLpn, dwNumOfPages, pBuf);
    }

    yaFTL.readBufferIndex = 0xffffffff;

    yaFTL.read_c.span = 0;
    vpnT = yaFTL.multiReadvpN;
    if((pBuf == NULL) && (dwNumOfPages == 1))
    {
        vpnT[0] = 0xffffffff;
    }
    for (i = 0; i < dwNumOfPages; i++, currPage += BYTES_PER_PAGE)
    {
#if ENABLE_L2V_TREE
        if (0 == yaFTL.read_c.span) {
            yaFTL.read_c.lba = dwLpn + i;
            L2V_Search(&yaFTL.read_c);

        }
        WMR_ASSERT(0 != yaFTL.read_c.span);

        if (L2V_VPN_MISS != yaFTL.read_c.vpn) {
            // Not missing from IC tree
            if (L2V_VPN_DEALLOC != yaFTL.read_c.vpn) {
                // Not deallocated
                vPN = yaFTL.read_c.vpn;
                if (yaFTL.read_c.vpn < L2V_VPN_SPECIAL) {
                    yaFTL.read_c.vpn++;
                }
            } else {
                // Deallocated
                vPN = 0xffffffff;
            }
            yaFTL.read_c.span--;
        } else {
            yaFTL.read_c.span--;
            if (yaFTL.read_c.vpn < L2V_VPN_SPECIAL) {
                yaFTL.read_c.vpn++;
            }
#else  // if not ENABLE_L2V_TREE:
        if (1) {
#endif // !ENABLE_L2V_TREE

            // Missed first search... :(
            indexPageNo = (dwLpn + i) / yaFTL.indexPageRatio;
            indexPageSub = (dwLpn + i) % yaFTL.indexPageRatio;
            indexPage = yaFTL.tocArray[indexPageNo].indexPage;
            cacheIndexPtr = yaFTL.tocArray[indexPageNo].cacheIndex;

            if ((indexPage != 0xffffffff) || (cacheIndexPtr != 0xffff))
            {
                /* Index page exists */
                if (cacheIndexPtr != 0xffff)
                {
                    /* Index page is in index cache */
                    vPN = yaFTL.indexCache[cacheIndexPtr].indexPage[indexPageSub];
                    if (yaFTL.indexCache[cacheIndexPtr].counter < YAFTL_INDEX_CACHE_COUNTER_LIMIT)
                        yaFTL.indexCache[cacheIndexPtr].counter++;
                }
                else
                {
                    /* Index page exists , but it is not in index cache . Read it from flash to cache first */
                    cacheIndexPtr = findFreeCacheEntry();

                    if (cacheIndexPtr != 0xffff)
                    {
                        status = (UInt8)_readPage(indexPage, (UInt8 *)(yaFTL.indexCache[cacheIndexPtr].indexPage), NULL, FALSE32, TRUE32, TRUE32);
                        if (status != FTL_SUCCESS)
                        {
                            if (FIL_GetFuncTbl()->RegisterCurrentTransaction)
                            {
                                FIL_GetFuncTbl()->RegisterCurrentTransaction(0, 0, NULL);
                            }
#ifndef AND_READONLY	
                            if (yaFTL.cxtValid == 1)
                            {
                                invalidateCXT();
                                yaFTL.cxtValid = 0;
                            }  
                            WMR_PANIC("Failed Index read Page 0x%08x Status 0x%08x", indexPage, status);
#endif							
                            return status;
                        }
#if ENABLE_L2V_TREE
                        PopulateFromIndexCache(indexPageNo, (UInt32*)yaFTL.indexCache[cacheIndexPtr].indexPage);
#endif // ENABLE_L2V_TREE
                        yaFTL.freeCachePages--;
                        yaFTL.indexCache[cacheIndexPtr].status = IC_BUSY_PAGE;
                        yaFTL.indexCache[cacheIndexPtr].counter = 1;
                        yaFTL.tocArray[indexPageNo].cacheIndex = cacheIndexPtr;
                        yaFTL.indexCache[cacheIndexPtr].tocEntry = indexPageNo;
                        vPN = yaFTL.indexCache[yaFTL.tocArray[indexPageNo].cacheIndex].indexPage[indexPageSub];
                    }
                    else
                    {
                        if (yaFTL.readBufferIndex != indexPageNo)
                        {
                            status = (UInt8)_readPage(indexPage, (UInt8 *)yaFTL.tmpReadBuffer, NULL, FALSE32, TRUE32, TRUE32);
                            if (status != FTL_SUCCESS)
                            {
                                if (FIL_GetFuncTbl()->RegisterCurrentTransaction)
                                {
                                    FIL_GetFuncTbl()->RegisterCurrentTransaction(0, 0, NULL);
                                }
#ifndef AND_READONLY
                                if (yaFTL.cxtValid == 1)
                                {
                                    invalidateCXT();
                                    yaFTL.cxtValid = 0;
                                }  
                                WMR_PANIC("Failed Index read Page 0x%08x Status 0x%08x", indexPage, status);
#endif
                                return status;
                            }
                            yaFTL.readBufferIndex = indexPageNo;
                        }
                        vPN = yaFTL.tmpReadBuffer[indexPageSub];
                     }
                 }
            }
            else
            {
                /* Index page does not exist => page was never updated */
                if((pBuf == NULL) && (dwNumOfPages == 1))
                {
                    return FTL_SUCCESS;
                }
                if (_readMultiPages((UInt32 *)vpnT, (UInt16)vpnOffset, currBatch, NULL, FALSE32, TRUE32))
                {
                    if(_verifyMetaData((dwLpn + ((currBatch - pBuf) / BYTES_PER_PAGE)), yaFTL.readMdPtr, vpnOffset) == FALSE32)
                        statusRet = FTL_USERDATA_ERROR;
                }
                else
                    statusRet = FTL_USERDATA_ERROR;
                vpnOffset = 0;
                continue;
            }
        }

        // Translation valid--perform the read:
        if (vPN != 0xffffffff)
        {
            vpnT[vpnOffset] = vPN;
            vpnOffset++;
            if (currBatch == NULL)
            {
                currBatch = currPage;
            }
            if (PAGES_PER_SUBLK == vpnOffset)
            {
                if((pBuf == NULL) && (dwNumOfPages == 1))
                {
                    return FTL_SUCCESS;
                }
                debug(READ, "Yaftl_read before readmulti1 how many  0x%x where  0x%x", vpnOffset, currBatch);
                if (_readMultiPages((UInt32 *)vpnT, (UInt16)vpnOffset, currBatch, NULL, FALSE32, TRUE32))
                 {
                    if(_verifyMetaData((dwLpn + ((currBatch - pBuf) / BYTES_PER_PAGE)), yaFTL.readMdPtr, vpnOffset) == FALSE32)
                        statusRet = FTL_USERDATA_ERROR;
                 }
                else
                    statusRet = FTL_USERDATA_ERROR;
                vpnOffset = 0;
                currBatch = NULL;
            }
        }
        else
        {
            if((pBuf == NULL) && (dwNumOfPages == 1))
            {
                return FTL_SUCCESS;
            }
            debug(READ, "Yaftl_read before readmulti2 how many  0x%x where  0x%x", vpnOffset, currBatch);
            if (_readMultiPages((UInt32 *)vpnT, (UInt16)vpnOffset, currBatch, NULL, FALSE32, TRUE32))
            {
                if(_verifyMetaData((dwLpn + ((currBatch - pBuf) / BYTES_PER_PAGE)), yaFTL.readMdPtr, vpnOffset) == FALSE32)
                    statusRet = FTL_USERDATA_ERROR;
            }
            else
                statusRet = FTL_USERDATA_ERROR;
            vpnOffset = 0;
            currBatch = NULL;
            continue;
        }
    }

    if (vpnOffset != 0)
    {
        if((pBuf == NULL) && (dwNumOfPages == 1))
        {
            return FTL_SUCCESS;
        }
        debug(READ, "Yaftl_read before readmulti how many  0x%x where  0x%x", vpnOffset, currBatch);
        if (_readMultiPages((UInt32 *)vpnT, (UInt16)vpnOffset, currBatch, NULL, FALSE32, TRUE32))
        {
            if(_verifyMetaData((dwLpn + ((currBatch - pBuf) / BYTES_PER_PAGE)), yaFTL.readMdPtr, vpnOffset) == FALSE32)
                statusRet = FTL_USERDATA_ERROR;
        }
        else
            statusRet = FTL_USERDATA_ERROR;
    }

    if((pBuf == NULL) && (dwNumOfPages == 1))
    {
        return FTL_USERDATA_ERROR;
    }
#ifdef AND_COLLECT_STATISTICS
    stFTLStatistics.ddwPagesReadCnt += dwNumOfPages;
#endif
#ifndef AND_READONLY
    if (yaFTL.readLimit == TRUE32)
    {
        if (yaFTL.blockArray[yaFTL.wrState.index.block].pagesRead > YAFTL_READ_DISTURB_LIMIT)
        {
            if ((yaFTL.blockArray[yaFTL.wrState.index.block].status == BLOCK_I_ALLOCATED)
                || (yaFTL.blockArray[yaFTL.wrState.index.block].status == BLOCK_I_CURRENT))
            {
                WMR_TRACE_IST_0(ReadDist, START);
#ifdef AND_COLLECT_STATISTICS
                if(yaFTL.blockArray[yaFTL.wrState.index.block].pagesRead == 0xffff)
                {
                    stFTLStatistics.ddwReadDisturbHandleCall++;
                }
                else
                {
                    stFTLStatistics.ddwRefreshCnt++;
                }
#endif            
                {
                    UInt16 sb = yaFTL.wrState.index.block;
                    
                    BTOC_Lock(sb);
                    YAFTL_GC_Index(yaFTL.wrState.index.block, TRUE32);
                    BTOC_Unlock(sb);
                }
                WMR_TRACE_IST_0(ReadDist, END);
                
            }
        }
        if (yaFTL.blockArray[yaFTL.wrState.data.block].pagesRead > YAFTL_READ_DISTURB_LIMIT)
        {
            if ((yaFTL.blockArray[yaFTL.wrState.data.block].status == BLOCK_ALLOCATED)
                || (yaFTL.blockArray[yaFTL.wrState.data.block].status == BLOCK_CURRENT))
            {
                WMR_TRACE_IST_0(ReadDist, START);
#ifdef AND_COLLECT_STATISTICS
                if(yaFTL.blockArray[yaFTL.wrState.data.block].pagesRead == 0xffff)
                {
                    stFTLStatistics.ddwReadDisturbHandleCall++;
                }
                else
                {
                    stFTLStatistics.ddwRefreshCnt++;
                }
#endif            
                {
                    UInt16 sb = yaFTL.wrState.data.block;  
                    
                    BTOC_Lock(sb);
                    YAFTL_GC_Data(yaFTL.wrState.data.block, TRUE32);
                    BTOC_Unlock(sb);
                }
                WMR_TRACE_IST_0(ReadDist, END);
            }
        }
        for (i = 0; i < (UInt32)FTL_AREA_SIZE; i++)
        {
            if (yaFTL.blockArray[i].pagesRead > YAFTL_READ_DISTURB_LIMIT)
            {
                if ((yaFTL.blockArray[i].status == BLOCK_I_ALLOCATED)
                    || (yaFTL.blockArray[i].status == BLOCK_I_CURRENT))
                {
                    WMR_TRACE_IST_0(ReadDist, START);
#ifdef AND_COLLECT_STATISTICS
                    if(yaFTL.blockArray[i].pagesRead == 0xffff)
                    {
                        stFTLStatistics.ddwReadDisturbHandleCall++;
                    }
                    else
                    {
                        stFTLStatistics.ddwRefreshCnt++;
                    }
#endif            
                    YAFTL_GC_Index(i, TRUE32);
                    WMR_TRACE_IST_0(ReadDist, END);
                    continue;
                }
                if ((yaFTL.blockArray[i].status == BLOCK_ALLOCATED)
                    || (yaFTL.blockArray[i].status == BLOCK_CURRENT))
                {
                    WMR_TRACE_IST_0(ReadDist, START);
#ifdef AND_COLLECT_STATISTICS
                    if(yaFTL.blockArray[i].pagesRead == 0xffff)
                    {
                        stFTLStatistics.ddwReadDisturbHandleCall++;
                    }
                    else
                    {
                        stFTLStatistics.ddwRefreshCnt++;
                    }
#endif            
                    YAFTL_GC_Data(i, TRUE32);
                    WMR_TRACE_IST_0(ReadDist, END);
                    continue;
                }
            }
        }
        yaFTL.readLimit = FALSE32;
    }

    // Periodic cxt save for ANDStats preservation
    if ((yaFTL.periodicCxt.pagesWritten >= WRITE_CXT_UPDATE_THRESHOLD) || (yaFTL.periodicCxt.erases >= ERASE_CXT_UPDATE_THRESHOLD))  
    {
        YAFTL_WriteStats();
    }
#endif

    if (FIL_GetFuncTbl()->RegisterCurrentTransaction)
    {
        FIL_GetFuncTbl()->RegisterCurrentTransaction(0, 0, NULL);
    }

    return statusRet;
}

#ifndef AND_READONLY
void IndexMarkDirty(UInt32 tocEntry)
{
    UInt32 cacheIndexOfs;
    UInt32 indexPagePhy;

    if(tocEntry >= yaFTL.TOCtableEntriesNo)
        return;

    cacheIndexOfs = yaFTL.tocArray[tocEntry].cacheIndex;
    indexPagePhy = yaFTL.tocArray[tocEntry].indexPage;

    if(cacheIndexOfs >= yaFTL.indexCacheSize)
        return;

    if (IC_DIRTY_PAGE == yaFTL.indexCache[cacheIndexOfs].status)
        return;

    // Mark as dirty
    yaFTL.indexCache[cacheIndexOfs].status = IC_DIRTY_PAGE;
    
    // Early out; nothing to account for
    if (0xffffffff == indexPagePhy) 
    {
        return;
    }

    // Mark previous index page as invalid
    yaFTL.blockArray[indexPagePhy / PAGES_PER_SUBLK].validPagesINo--;
    yaFTL.seaState.index.validPages--;
#ifdef AND_COLLECT_STATISTICS
    stFTLStatistics.ddwValidIndexPages--;
#endif
    yaFTL.tocArray[tocEntry].indexPage = 0xffffffff;
}
    
UInt32* IndexLoadClean(UInt32 lba, UInt32 *indexOfs, UInt32 *d_tocEntry)
{
    UInt32 indexPageNo, indexPageSub;
    UInt32 indexPagePhy;
    UInt32 cacheIndexOfs, oldCacheIndexOfs;
    UInt32 status;
    PageMeta_t *md = yaFTL.meta_indexLoadDirty;
    
    *d_tocEntry = 0xffffffff;
    
    // Calculate which index page and offset within for this update
    indexPageNo = lba / yaFTL.indexPageRatio;
    indexPageSub = lba % yaFTL.indexPageRatio;
    indexPagePhy = yaFTL.tocArray[indexPageNo].indexPage;
    cacheIndexOfs = yaFTL.tocArray[indexPageNo].cacheIndex;
    oldCacheIndexOfs = cacheIndexOfs;
    // Set up return parameter
    *indexOfs = indexPageSub;
    
    // Don't load unmapped regions
    if ((0xffffffff == indexPagePhy) && (0xffff == cacheIndexOfs))
    {
        // All sectors addressed are in deleted state
        return NULL;
    }
    
    // Find or create cache entry
    if (0xffff == cacheIndexOfs) {
        cacheIndexOfs = findFreeCacheEntry();
        if (cacheIndexOfs == 0xffff) {
            cacheIndexOfs = clearEntryInCache(0, 0xffff, 0);
            if (cacheIndexOfs == 0xffff) {
                // Invalidate context
                if (yaFTL.cxtValid == 1)
                {
                    invalidateCXT();
                    yaFTL.cxtValid = 0;
                }
                WMR_PANIC("failed to evict index page from cache");
            }
        }
        WMR_MEMSET(yaFTL.indexCache[cacheIndexOfs].indexPage, 0xff, BYTES_PER_PAGE);
        yaFTL.freeCachePages--;
        status = _readPage(indexPagePhy, (UInt8 *)yaFTL.indexCache[cacheIndexOfs].indexPage, md, FALSE32, TRUE32, TRUE32);
        //TODO: reconstruct index page by default, but be careful of coherency issues with prior tree update
        if (status != FTL_SUCCESS)
        {
            if (yaFTL.cxtValid == 1)
            {
                invalidateCXT();
                yaFTL.cxtValid = 0;
            }

            WMR_PANIC("uECC index page in ILD: vpn 0x%x, indexPageNo %d", indexPagePhy, indexPageNo); // TODO: reconstruct
        }
        // Reset counter since we just pulled this in to the cache
        yaFTL.indexCache[cacheIndexOfs].counter = 0;
        yaFTL.indexCache[cacheIndexOfs].status = IC_BUSY_PAGE;
        yaFTL.indexCache[cacheIndexOfs].tocEntry = indexPageNo;
        yaFTL.tocArray[indexPageNo].cacheIndex = cacheIndexOfs;
    }
    
    // Give back the pointer to the cached index page
    *d_tocEntry = indexPageNo;
    
    return yaFTL.indexCache[cacheIndexOfs].indexPage;
}
    
UInt32* IndexLoadDirty(UInt32 lba, UInt32 *indexOfs)
{
    UInt32 indexPageNo, indexPageSub;
    UInt32 indexPagePhy;
    UInt32 cacheIndexOfs, oldCacheIndexOfs;
    UInt32 *indexPage;
    UInt32 status;
    PageMeta_t *md = yaFTL.meta_indexLoadDirty;

    // Calculate which index page and offset within for this update
    indexPageNo = lba / yaFTL.indexPageRatio;
    indexPageSub = lba % yaFTL.indexPageRatio;
    indexPagePhy = yaFTL.tocArray[indexPageNo].indexPage;
    cacheIndexOfs = yaFTL.tocArray[indexPageNo].cacheIndex;
    oldCacheIndexOfs = cacheIndexOfs;
    // Set up return parameter
    *indexOfs = indexPageSub;

    // Find or create cache entry
    if (0xffff == cacheIndexOfs) {
        cacheIndexOfs = findFreeCacheEntry();
        if (cacheIndexOfs == 0xffff) {
            cacheIndexOfs = clearEntryInCache(0, 0xffff, 0);
            if (cacheIndexOfs == 0xffff) {
                // Invalidate context
                if (yaFTL.cxtValid == 1)
                {
                    invalidateCXT();
                    yaFTL.cxtValid = 0;
                }
                WMR_PANIC("failed to evict index page from cache");
            }
        }
        WMR_MEMSET(yaFTL.indexCache[cacheIndexOfs].indexPage, 0xff, BYTES_PER_PAGE);
        yaFTL.freeCachePages--;
        // Reset counter since we just pulled this in to the cache
        yaFTL.indexCache[cacheIndexOfs].counter = 0;
    }
    yaFTL.indexCache[cacheIndexOfs].tocEntry = indexPageNo;
    yaFTL.tocArray[indexPageNo].cacheIndex = cacheIndexOfs;

    yaFTL.indexCache[cacheIndexOfs].status = IC_DIRTY_PAGE;
    if (yaFTL.indexCache[cacheIndexOfs].counter < YAFTL_INDEX_CACHE_COUNTER_LIMIT)
        yaFTL.indexCache[cacheIndexOfs].counter++;
    indexPage = yaFTL.indexCache[cacheIndexOfs].indexPage;

    // Don't load unmapped regions
    if (0xffffffff == indexPagePhy) {
        // All sectors addressed are in deleted state
        return indexPage;
    }

    // Load index page
    if (0xffff == oldCacheIndexOfs) {
        status = _readPage(indexPagePhy, (UInt8 *)indexPage, md, FALSE32, TRUE32, TRUE32);
        //TODO: reconstruct index page by default, but be careful of coherency issues with prior tree update
        if (status != FTL_SUCCESS)
        {
            if (yaFTL.cxtValid == 1)
            {
                invalidateCXT();
                yaFTL.cxtValid = 0;
            }  

            WMR_PANIC("uECC index page in ILD: vpn 0x%x, indexPageNo %d", indexPagePhy, indexPageNo); // TODO: reconstruct
        }
    }

    // Mark previous index page as invalid
    yaFTL.blockArray[indexPagePhy / PAGES_PER_SUBLK].validPagesINo--;
    yaFTL.seaState.index.validPages--;
#ifdef AND_COLLECT_STATISTICS
    stFTLStatistics.ddwValidIndexPages--;
#endif
    yaFTL.tocArray[indexPageNo].indexPage = 0xffffffff;

    // Give back the pointer to the cached index page
    return indexPage;
}
#endif

#ifndef AND_READONLY
/*****************************************************************************/
/*                                                                           */
/* NAME                                                                      */
/*      YAFTL_Write                                                          */
/* DESCRIPTION                                                               */
/*     writes a range of consecutive logical pages to flash                  */
/*                                                                           */
/*****************************************************************************/
static Int32
YAFTL_Write(UInt32 dwLpn, UInt32 dwNumOfPages, UInt8 *pabDataBuf, BOOL32 isStatic)
{
    UInt8 *curPage;
    PageMeta_t *mdPtr;
    UInt32 lba, count;
    UInt32 toMegaStripeEnd, toBlockEnd, thisCount, ofs, i;
    UInt32 vpn, indexOfs, oldBlock;
    UInt32 *indexPage;
    // For recovering from program failure--restart the write at the beginning of
    // the write, or the most recently crossed superblock boundary.
    UInt32 blockStartLba, blockStartCnt;
    UInt8 *blockStartBuf;

    // Invalidate context
    if (yaFTL.cxtValid) {
        invalidateCXT();
        yaFTL.cxtValid = 0;
    }

    debug(WRITE, " YAFTL_Write[start] 0x%x 0x%x isStatic: %s", dwLpn, dwNumOfPages, (isStatic)?"YES":"NO");
#ifdef AND_COLLECT_STATISTICS
    stFTLStatistics.ddwFTLWriteCnt++;
    if((dwNumOfPages > 0) && (dwNumOfPages <= 8))
    {
        stFTLStatistics.ddwWrite[dwNumOfPages-1]++;
    }
    else
    {
        if(dwNumOfPages > 8)
            stFTLStatistics.ddwWriteOver8++;
    }
#endif    
    if ((dwLpn + dwNumOfPages) >= yaFTL.logicalPartitionSize)
    {
        return FTL_CRITICAL_ERROR;
    }

    if (FIL_GetFuncTbl()->RegisterCurrentTransaction)
    {
        FIL_GetFuncTbl()->RegisterCurrentTransaction(dwLpn, dwNumOfPages, pabDataBuf);
    }

    // Delete the sectors prior to write
    YAFTL_DeleteSectors(dwLpn, dwNumOfPages);

    blockStartLba = dwLpn;
    blockStartCnt = dwNumOfPages;
    blockStartBuf = pabDataBuf;
recoverPfail:
    // Restore counters
    curPage = blockStartBuf;
    lba = blockStartLba;
    count = blockStartCnt;

    // Do garbage collection prior to the write
    YAFTL_GC_PreWrite(count);

    mdPtr = yaFTL.writeMdPtr;
    // Writes and tree/index update:
    ofs = 0;
    while (count) {
        // Advance block?
        if (yaFTL.wrState.data.nextPage >= (PAGES_PER_SUBLK - yaFTL.controlPageNo)) {
            writeCurrentBlockTOC(1);
            allocateBlock(yaFTL.wrState.data.block, 0, 1);
            blockStartLba = lba;
            blockStartCnt = count;
            blockStartBuf = curPage;
        }
  
        // How many this round?  Do up to a megastripe, subtracting to align to stripe end.
        toMegaStripeEnd = ((NUM_BANKS * YAFTL_WRITE_MAX_STRIPES) - (yaFTL.wrState.data.nextPage % NUM_BANKS));
        toBlockEnd = (PAGES_PER_SUBLK - yaFTL.controlPageNo - yaFTL.wrState.data.nextPage);
        thisCount = WMR_MIN(count, WMR_MIN(toMegaStripeEnd, toBlockEnd));
  
        // Generate metadata
        SetupMeta_Data(mdPtr, lba, thisCount);
  
        // Write the data
        vpn = (yaFTL.wrState.data.block * (UInt32)PAGES_PER_SUBLK) + yaFTL.wrState.data.nextPage;
        if (!VFL_WriteMultiplePagesInVb(vpn, thisCount, curPage, (UInt8 *)mdPtr, TRUE32, FALSE32)) {
            oldBlock = yaFTL.wrState.data.block;
            if(addBlockToEraseNowList(oldBlock) == FALSE32)
            {
                // Invalidate context
                if (yaFTL.cxtValid == 1)
                {
                    invalidateCXT();
                    yaFTL.cxtValid = 0;
                }
                WMR_PANIC("cannot add block %d to a scrub list", oldBlock);   
            }
            BTOC_Lock(oldBlock);
            allocateBlock(oldBlock, 0, 1);
            YAFTL_DeleteSectors(blockStartLba, blockStartCnt);
            if (BLOCK_FREE != yaFTL.blockArray[oldBlock].status) {
                // Only GC it if it wasn't taken care of by DeleteSectors
                YAFTL_GC_Data(oldBlock, TRUE32);
            }
            BTOC_Unlock(oldBlock);
            goto recoverPfail;
        }
  
        // Update TOC:
        for (i = 0; i < thisCount; i++) {
            yaFTL.wrState.data.TOC[yaFTL.wrState.data.nextPage + i] = lba + i;
        }
  
        // Tree: update
#if ENABLE_L2V_TREE
        L2V_Update(lba, thisCount, vpn);
#endif // ENABLE_L2V_TREE

        // Index: decrement valid in sb, and global validPages, update vpn
        indexPage = IndexLoadDirty(lba, &indexOfs);
        for (i = 0; i < thisCount; i++) {
            // Update vpn
            indexPage[indexOfs] = vpn + i;
  
            // Next
            indexOfs++;
            if (indexOfs >= yaFTL.indexPageRatio) {
                indexPage = IndexLoadDirty(lba + i + 1, &indexOfs);
            }
        }
  
        // Update counters
        yaFTL.wrState.data.nextPage += thisCount;
        yaFTL.seaState.data.validPages += thisCount;
        yaFTL.blockArray[yaFTL.wrState.data.block].validPagesDNo += thisCount;
#ifdef AND_COLLECT_STATISTICS
        stFTLStatistics.ddwValidDataPages += thisCount;
#endif
  
        // Next
        lba += thisCount;
        ofs += thisCount;
        count -= thisCount;
        curPage += (thisCount * BYTES_PER_PAGE);
    }

    // Periodic cxt save for ANDStats preservation
    stFTLStatistics.ddwPagesWrittenCnt += dwNumOfPages;
    yaFTL.periodicCxt.pagesWritten += dwNumOfPages;
    if ((yaFTL.periodicCxt.pagesWritten >= WRITE_CXT_UPDATE_THRESHOLD) || (yaFTL.periodicCxt.erases >= ERASE_CXT_UPDATE_THRESHOLD))  
    {
        YAFTL_WriteStats();
    }

    if (FIL_GetFuncTbl()->RegisterCurrentTransaction)
    {
        FIL_GetFuncTbl()->RegisterCurrentTransaction(0, 0, NULL);
    }

    return FTL_SUCCESS;
}

#endif


#ifndef AND_READONLY
/*****************************************************************************/
/*                                                                           */
/* NAME                                                                      */
/*      YAFTL_Format												         */
/* DESCRIPTION                                                               */
/*     performs low-level clean up of a flash                                */
/*                                                                           */
/*****************************************************************************/
#ifdef AND_COLLECT_STATISTICS
static ANDStatus getStats(UInt16 *cxtT)
{
    UInt16 currentCX = 0;
    UInt16 i;
    ANDStatus status = FTL_CRITICAL_ERROR;
    WeaveSeq_t lastCX = 0;
    Buffer *buf = BUF_Get(BUF_MAIN_AND_SPARE);
    UInt8 *tBuff;
    PageMeta_t *md = yaFTL.meta_getStats;

    debug(FORMAT, "[FTL:MSG] getStats[start] 0x%x", PAGES_PER_SUBLK);

    /* read out VFL_meta data . If it is not present execute full format . If it is present go and fetch "quick mount" info from CX area if it's valid .If not execute restore
       For now we need to start from fresh media which means there is no quick mount info and all blocks are free . */
    /*  Get CXT blocks from the VFL  */
    if (buf == NULL)
    {
        return FTL_CRITICAL_ERROR;
    }

    tBuff = buf->pData;

    if (cxtT[0] == 0xffff)
    {
        BUF_Release(buf);
        return FTL_CRITICAL_ERROR;
    }
    
    for (i = 0; i < FTL_CXT_SECTION_SIZE; i++)
    {
        status = VFL_Read(cxtT[i] * PAGES_PER_SUBLK, buf, TRUE32, FALSE32, NULL, NULL, FALSE32);
        if (status != VFL_SUCCESS)
        {
            continue;
        }
        WMR_MEMCPY(md, buf->pSpare, sizeof(PageMeta_t));
        if(sizeof(WeaveSeq_t) <= 4)
        {
            if ((META_GET_WEAVESEQ(md) != 0xffffffff) && (lastCX < META_GET_WEAVESEQ(md)))
            {
                lastCX = (UInt32)META_GET_WEAVESEQ(md);
                currentCX = cxtT[i];
            }
        }
        else
        {
            if ((META_GET_WEAVESEQ(md) != (WeaveSeq_t)0xffffffffffffULL) && (lastCX < META_GET_WEAVESEQ(md)))
            {
                lastCX = (UInt32)META_GET_WEAVESEQ(md);
                currentCX = cxtT[i];
            }
        }
    }

    if (lastCX != 0)
    {
        debug(FORMAT, "getStats : currentCX is %d", currentCX);
    }
    else
    {
        debug(FORMAT, "[FTL:MSG] getStats could not find CXT at all ");
        BUF_Release(buf);
        return FTL_CRITICAL_ERROR;
    }

    for (i = PAGES_PER_SUBLK; i > 0; i--)
    {
        status = VFL_Read((currentCX * PAGES_PER_SUBLK) + (i - 1), buf, TRUE32, FALSE32, NULL, NULL, FALSE32);

        if (status == VFL_SUCCESS)
        {
            if (WMR_MEMCMP(tBuff, CXT_VER, 4) == 0)
            {
                debug(FORMAT, "latest cxt is at offset 0x%x",(currentCX * PAGES_PER_SUBLK) + (i - 1));
                status = VFL_Read((currentCX * PAGES_PER_SUBLK) + (i - 1) + 1 + 2*yaFTL.controlPageNo, buf, TRUE32, FALSE32, NULL, NULL, FALSE32);
                if (status == VFL_SUCCESS)
                {
                    debug(FORMAT, "getting stats at offset 0x%x", (currentCX * PAGES_PER_SUBLK) + (i - 1) + 1 + 2*yaFTL.controlPageNo);
                    _FTLSetStatisticsFromCxt(tBuff);
                    BUF_Release(buf);
                    return FTL_SUCCESS;
                }
            }
        }
    }
    debug(FORMAT, "[FTL:MSG] getStats could not find CXT at all ");
    BUF_Release(buf);

    return FTL_CRITICAL_ERROR;
}
#endif

static ANDStatus     YAFTL_Format(UInt32 dwOptions)
{
    UInt32 i, ftlType;
    ANDStatus status, statStatus;
    UInt16 cxtT[FTL_CXT_SECTION_SIZE];
    BOOL32 moveCxt = FALSE32;

    status = FTL_CRITICAL_ERROR;
    debug(FORMAT, "[FTL:MSG] YAFTL_Format[start]");
#ifdef AND_COLLECT_STATISTICS
    WMR_MEMCPY(cxtT, VFL_GetFTLCxtVbn(), FTL_CXT_SECTION_SIZE * sizeof(UInt16));

    statStatus = getStats(cxtT);
#endif
    for (i = 0; i < (UInt32)FTL_AREA_SIZE; i++)
    {
        status = VFL_Erase((UInt16)i, TRUE32);
    }

    if(cxtT[0] != 0xffff)
    {
        for (i = 0; i < (UInt32)FTL_CXT_SECTION_SIZE; i++)
        {
            if(cxtT[i] >= FTL_AREA_SIZE) 
            {
                debug(FORMAT, "[FTL:MSG] YAFTL_Format cxt 0x%x is out of range 0x%x",cxtT[i], FTL_AREA_SIZE);
                VFL_Erase((UInt16)cxtT[i], TRUE32);
                moveCxt = TRUE32;
            }
        }
        if(moveCxt == TRUE32)
        {
            for (i = 0; i < (UInt32)FTL_CXT_SECTION_SIZE; i++)
            {
                cxtT[i] = i; 
            }
            VFL_ChangeFTLCxtVbn(cxtT);
        }
    }
    
#ifdef AND_COLLECT_STATISTICS
    if (statStatus == FTL_SUCCESS)
    {
        yaFTL.currentCxtVpn = 0xffffffff;
        yaFTL.currentCXTIndex = 0;
        yaFTL.cxtValid = 0;
        yaFTL.formatWasCalled = 1;
        quickMountUpdate(FALSE32);
        if (yaFTL.cxtValid == 1)
        {
            invalidateCXT();
            yaFTL.cxtValid = 0;
        }
    }
#endif
    if (status != FTL_SUCCESS)
    {
        return status;
    }
    debug(FORMAT, "[FTL:MSG] YAFTL_Format[end]");
    ftlType = FTL_TYPE_YAFTL;
    VFL_SetStruct(AND_STRUCT_VFL_FTLTYPE, &ftlType, sizeof(UInt32));
    yaFTL.formatWasCalled = 1;
    return FTL_SUCCESS;
}
#endif
#ifndef AND_READONLY
/*****************************************************************************/
/*                                                                           */
/* NAME                                                                      */
/*      YAFTL_WearLevel												         */
/* DESCRIPTION                                                               */
/*     moves read-only blocks around                                         */
/*                                                                           */
/*****************************************************************************/
static Int32     YAFTL_WearLevel(void)
{
    UInt16 i,bestCandidate = 0xffff;
    UInt16 bestCount = 0xffff;
    UInt8  erasedCount = 0;
	
#ifdef AND_COLLECT_STATISTICS
    stFTLStatistics.ddwWearLevelCnt++;
#endif

    // Gap too big, or not our time yet? Exit.
    if (yaFTL.wearLevel.pointOfNoReturn || (yaFTL.wearLevel.blocksSince < YAFTL_WEARLEVEL_PERIOD))
    {
        for (i = 0; (i < FTL_AREA_SIZE) && (erasedCount < ERASE_IDLE_LIMIT); i++)
        {
            if((yaFTL.blockArray[i].status == BLOCK_FREE) && (yaFTL.blockArray[i].eraseBeforeUse == BLOCK_TO_BE_ERASED))
            {
                ANDStatus status;
                
                // Invalidate context
                if (yaFTL.cxtValid == 1)
                {
                    invalidateCXT();
                    yaFTL.cxtValid = 0;
                }
                // Erase it
                status = VFL_Erase(i, TRUE32);
                if (status != FTL_SUCCESS)
                {
                    WMR_PANIC("VFL_Erase(%d) failed with 0x%08x", (UInt32) i, status);
                    return status;
                }
                
                yaFTL.erasedBlockCount++;
                erasedCount++;
                yaFTL.blockArray[i].erasableCount++;
                if (yaFTL.blockArray[i].erasableCount > yaFTL.maxEraseCount)
                {
                    yaFTL.maxEraseCount = yaFTL.blockArray[i].erasableCount;
                }
                yaFTL.blockArray[i].eraseBeforeUse = 0;
            }
        }
        return FTL_OUT_OF_RANGE_ERROR;
    }

    WMR_TRACE_IST_0(WearLevel, START);

    // Find best candidate (least cycled block)
    for (i = 0; i < FTL_AREA_SIZE; i++)
    {
        if (BLOCK_ALLOCATED != yaFTL.blockArray[i].status)
        {
            // Don't consider non-data blocks; erased blocks are ok, and index will take care of itself
            continue;
        }

        // Detect gap-too-big condition (aka pointOfNoReturn);
        if ((yaFTL.blockArray[i].erasableCount + ERASE_COUNT_GAP_UPPER_LIMIT) < yaFTL.maxEraseCount)
        {
            debug(MISC, "\nYAFTL_WearLevel gap is too big . 0x%x maxerase 0x%x\n", yaFTL.blockArray[i].erasableCount, yaFTL.maxEraseCount);
            yaFTL.wearLevel.pointOfNoReturn = TRUE32;
            break;
        }

        // Block at least GAP_LIMIT less than the maxErase?
        if ((yaFTL.blockArray[i].erasableCount + ERASE_COUNT_GAP_LIMIT) < yaFTL.maxEraseCount)
        {
            if (yaFTL.blockArray[i].erasableCount < bestCount)
            {
                bestCandidate = i;
                bestCount = yaFTL.blockArray[i].erasableCount;
            }
        }
    }

    // Didn't find a candidate
    if (0xffff == bestCandidate)
    {
        WMR_TRACE_IST_1(WearLevel, END, FTL_OUT_OF_RANGE_ERROR);
        return FTL_OUT_OF_RANGE_ERROR;
    }

    // Perform wear leveling
    debug(MISC, "\nYAFTL_WearLevel @@@ block 0x%x  blockerase 0x%x maxerase 0x%x\n", bestCandidate, yaFTL.blockArray[bestCandidate].erasableCount, yaFTL.maxEraseCount);
    YAFTL_GC_Data(bestCandidate, TRUE32);
    yaFTL.wearLevel.blocksSince -= YAFTL_WEARLEVEL_PERIOD;

    WMR_TRACE_IST_1(WearLevel, END, FTL_SUCCESS);
    return FTL_SUCCESS;
}
#endif
#ifndef AND_READONLY
/*****************************************************************************/
/*                                                                           */
/* NAME                                                                      */
/*      YAFTL_GarbageCollect										         */
/* DESCRIPTION                                                               */
/*     perfroms defragementation of a disk while system in idle time         */
/*                                                                           */
/*****************************************************************************/
static BOOL32    YAFTL_GarbageCollect(void)
{
    return 1;
}
#endif
#ifndef AND_READONLY
/*****************************************************************************/
/*                                                                           */
/* NAME                                                                      */
/*      YAFTL_ShutdownNotify                                                 */
/* DESCRIPTION                                                               */
/*     commits cached index pages as well as control information to flash    */
/*                                                                           */
/*****************************************************************************/

UInt16 how_many_updates = 0;
    
static BOOL32    YAFTL_ShutdownNotify(BOOL32 boolMergeLogs)
{

    flushIndexCache(1);
    if(how_many_updates++ > MOVE_CXT_TRESHOLD )
    {
        moveCXTarea();
        how_many_updates = 0;
    }

#ifdef AND_COLLECT_STATISTICS
    stFTLStatistics.ddwFlushCnt++;
#endif

    while (quickMountUpdate(FALSE32) != FTL_SUCCESS)
    {
        debug(ERROR, "YAFTL_flushquickmountupdate failed");
    }

    // Clear periodic context drop counters
    yaFTL.periodicCxt.pagesWritten = 0;
    yaFTL.periodicCxt.erases = 0;

    return 1;
}

static BOOL32    YAFTL_WriteStats(void)
{
    BOOL32 result = TRUE32;

    if(how_many_updates++ > MOVE_CXT_TRESHOLD )
    {
        moveCXTarea();
        how_many_updates = 0;
    }

    if (yaFTL.cxtValid != 1)
    {
        if(quickMountUpdate(TRUE32) != FTL_SUCCESS)
        {
            debug(ERROR, "YAFTL_flushquickmountupdate failed");
            result = FALSE32;
        }
        else
        {    
            invalidateCXT();
            yaFTL.cxtValid = 0;
        }
    }

    // Clear periodic context drop counters
    yaFTL.periodicCxt.pagesWritten = 0;
    yaFTL.periodicCxt.erases = 0;

    return result;
}
#endif

/*****************************************************************************/
/*                                                                           */
/* NAME                                                                      */
/*      YAFTL_FreeMemory                                                     */
/* DESCRIPTION                                                               */
/*     frees all allocated memory; called on error path in _Open, and _Close */
/*                                                                           */
/*****************************************************************************/
static void YAFTL_FreeMemory(void)
{
    // Free the zone
    WMR_BufZone_Free(&yaFTL.BufZone);
    WMR_BufZone_Free(&yaFTL.BufZone_IndexCache);

    // [TOC_SIZE]  Table of content that contains location of index pages
    // in flash and index cache
    if (NULL != yaFTL.tocArray) {
        WMR_FREE(yaFTL.tocArray, yaFTL.TOCtableEntriesNo * sizeof(TOCEntry));
    }
    // [INDEX_CACHE_SIZE] cached index pages
    if (NULL != yaFTL.indexCache) {
        WMR_FREE(yaFTL.indexCache, yaFTL.indexCacheSize * sizeof(CacheIndexEntry));
    }
    // [YAFTL_NO_OF_BLOCKS] block information
    if (NULL != yaFTL.blockArray) {
        WMR_FREE(yaFTL.blockArray, FTL_AREA_SIZE * sizeof(BlockEntry));
    }
    if (NULL != yaFTL.multiReadvpN) {
        WMR_FREE(yaFTL.multiReadvpN, sizeof(UInt32) * PAGES_PER_SUBLK);
    }

#ifdef AND_COLLECT_STATISTICS
    WMR_MEMSET(&stFTLStatistics, 0, sizeof(FTLStatistics));
#endif // AND_COLLECT_STATISTICS

#ifndef AND_READONLY
    YAFTL_GC_Close();
#endif

#if ENABLE_L2V_TREE
    L2V_Free();
#endif // ENABLE_L2V_TREE

    BTOC_Close();

    // Null out pointers
    yaFTL.quickRestore_tBuff = NULL;
    yaFTL.wrState.data.TOC = NULL;
    yaFTL.wrState.index.TOC = NULL;
    yaFTL.tmpReadBuffer = NULL;
    yaFTL.restoreTOC = NULL;
    yaFTL.meta_restoreIdx = NULL;
    yaFTL.meta_restoreData = NULL;
    yaFTL.meta_readCxt = NULL;
    yaFTL.meta_quickRestore = NULL;
    yaFTL.meta_invalidateCxt = NULL;
    yaFTL.meta_writeCxt = NULL;
    yaFTL.meta_restoreMountErase = NULL;
    yaFTL.meta_restoreMount = NULL;
    yaFTL.meta_clearEntryInCache = NULL;
    yaFTL.meta_writeBTOC = NULL;
    yaFTL.meta_indexLoadDirty = NULL;
    yaFTL.meta_getStats = NULL;
    yaFTL.meta_GetBlockTOC = NULL;
    yaFTL.meta_IsDataPageCurrent = NULL;
    yaFTL.meta_WriteZoneData = NULL;
    yaFTL.tocArray = NULL;
    yaFTL.indexCache = NULL;
    yaFTL.blockArray = NULL;
    yaFTL.writeMdPtr = NULL;
    yaFTL.multiReadvpN = NULL;
    yaFTL.readMdPtr = NULL;
    yaFTL.singleMdPtr = NULL;
}
/*****************************************************************************/
/*                                                                           */
/* NAME                                                                      */
/*      YAFTL_Close                                                          */
/* DESCRIPTION                                                               */
/*     frees up all the memory allocated by YAFTL tables                     */
/*                                                                           */
/*****************************************************************************/
static void  YAFTL_Close(void)
{
    YAFTL_FreeMemory();
    WMR_MEMSET(&yaFTL_FTLDeviceInfo, 0, sizeof(FTLWMRDeviceInfo));
    WMR_MEMSET(&yaFTL_VFLFunctions, 0, sizeof(VFLFunctions));
    WMR_MEMSET(&yaFTL, 0, sizeof(yaFTL_t));
    yaftl_init_done = FALSE32;

}

#ifndef AND_READONLY

/*****************************************************************************/
/*                                                                           */
/* NAME                                                                      */
/*      _writeMultiPages	            									 */
/* DESCRIPTION                                                               */
/*     writes a set of virtual pages                                         */
/*                                                                           */
/*****************************************************************************/
BOOL32 _writeMultiPages(UInt16 vbn, UInt16 pageOffset, UInt16 wNumPagesToWrite, UInt8 *pageData, PageMeta_t *mdPtr, BOOL32 bInternalOp)
{
    BOOL32 status;
    UInt32 vpn;
    const BOOL32 bMovingUserData = bInternalOp && META_IS_DATA(mdPtr);

    debug(WRITE, "[FTL:MSG] writeMultiPages[start] 0x%x ", wNumPagesToWrite);

    if (yaFTL.cxtValid == 1)
    {
        invalidateCXT();
        yaFTL.cxtValid = 0;
    }
    vpn = (((UInt32)vbn * (UInt32)PAGES_PER_SUBLK) + (UInt32)pageOffset);
    status = VFL_WriteMultiplePagesInVb(vpn, wNumPagesToWrite, pageData, (UInt8 *)mdPtr, TRUE32, bMovingUserData);
    if (status)
    {
        return TRUE32;
    }
    else
    {
        WMR_PRINT(QUAL, "we got multiwrite failure at vbn %d\n", (UInt32) vbn);
        return FALSE32;
    }
}

#endif

/*****************************************************************************/
/*                                                                           */
/* NAME                                                                      */
/*      _readMultiPages	            									     */
/* DESCRIPTION                                                               */
/*     reads a set of virtual pages from flash                               */
/*                                                                           */
/*****************************************************************************/
BOOL32 _readMultiPages(UInt32 * padwVpn, UInt16 wNumPagesToRead, UInt8 * pbaData, UInt8 * pbaSpare, BOOL32 bInternalOp, BOOL32 scrubOnUECC)
{
    BOOL32 status;
    Int32 stat;
    PageMeta_t *tempP = NULL;
    UInt32 currentOffset, currentLen, i;
    BOOL32 needsRefresh = FALSE32;

    debug(READ, " _readMultiPages[start] 0x%x  ", wNumPagesToRead);
    if (pbaSpare == NULL)
    {
        tempP = yaFTL.readMdPtr;
    }
    else
    {
        tempP = (PageMeta_t *)pbaSpare;
    }
    currentOffset = 0;
    currentLen = wNumPagesToRead;

    while (currentLen > PAGES_PER_SUBLK)
    {
        for(i = 0; i < PAGES_PER_SUBLK; i++)
        {
            updateReadCounter((UInt16) padwVpn[currentOffset+i] / PAGES_PER_SUBLK);
        }
        
        status = VFL_ReadScatteredPagesInVb(&(padwVpn[currentOffset]), PAGES_PER_SUBLK, &(pbaData[currentOffset * BYTES_PER_PAGE]), (UInt8 *)tempP, &needsRefresh, NULL, bInternalOp, &stat);
        if (!status)
        {
            debug(ERROR, "we got read failure ");
            status = TRUE32;
            WMR_TRACE_IST_1(SingleReadFallback, START, stat);
            for (i = 0; i < PAGES_PER_SUBLK; i++)
            {
                UInt32 current_vpn = padwVpn[currentOffset + i];
                stat = _readPage(current_vpn, &(pbaData[(currentOffset + i) * BYTES_PER_PAGE]), &tempP[currentOffset + i], bInternalOp, TRUE32, scrubOnUECC);
                if (stat != FTL_SUCCESS)
                {
                    WMR_PRINT(QUAL, "read failure on vpn %d\n", current_vpn);
                    status = FALSE32;
                }
            }
            WMR_TRACE_IST_1(SingleReadFallback, END, stat);
            if (!status)
            {
                return FALSE32;
            }
            else
            {
                WMR_PRINT(QUAL_FATAL, "were able to overcome read failure\n");
            }
        }
        else
        {
            if(stat != FTL_SUCCESS)
                return FALSE32;
#ifndef AND_READONLY            
            if( needsRefresh )
            {
                WMR_PRINT(QUAL, "refresh is triggered\n");
                status = TRUE32;
                for (i = 0; i < PAGES_PER_SUBLK; i++)
                {
                    stat = _readPage(padwVpn[currentOffset + i], &(pbaData[(currentOffset + i) * BYTES_PER_PAGE]), &tempP[currentOffset + i], bInternalOp, TRUE32, scrubOnUECC);
                    if (stat != FTL_SUCCESS)
                    {
                        status = FALSE32;
                    }
                }
                if (!status)
                {
                    return FALSE32;
                }
            }
#endif            
        }
        currentOffset += PAGES_PER_SUBLK;
        currentLen -= PAGES_PER_SUBLK;
        #ifndef AND_READONLY
        /*  Vadim - should not invalidate cxt while reading */
        /*  cxtValid = 0; */
        if (yaFTL.blockArray[padwVpn[currentOffset] / PAGES_PER_SUBLK].pagesRead > YAFTL_READ_DISTURB_LIMIT)
        {
            yaFTL.readLimit = TRUE32;
            WMR_TRACE_IST_1(ReadCountEnq, NONE, padwVpn[currentOffset] / PAGES_PER_SUBLK);
        }
        #endif // !  AND_READONLY
    }

    if (currentLen > 0)
    {
        for(i = 0; i < currentLen; i++)
        {
            updateReadCounter((UInt16) padwVpn[currentOffset+i] / PAGES_PER_SUBLK);
        }

#ifndef AND_READONLY
        if (yaFTL.blockArray[padwVpn[currentOffset] / PAGES_PER_SUBLK].pagesRead > YAFTL_READ_DISTURB_LIMIT)
        {
            yaFTL.readLimit = TRUE32;
            WMR_TRACE_IST_1(ReadCountEnq, NONE, padwVpn[currentOffset] / PAGES_PER_SUBLK);
        }
#endif // !  AND_READONLY
        needsRefresh = FALSE32;
        status = VFL_ReadScatteredPagesInVb(&(padwVpn[currentOffset]), (UInt16)currentLen, &(pbaData[currentOffset * BYTES_PER_PAGE]), (UInt8 *)tempP, &needsRefresh, NULL, bInternalOp, &stat);
        if (status == FALSE32)
        {
            debug(ERROR, "we got read failure ");
            status = TRUE32;
            for (i = 0; i < currentLen; i++)
            {
                UInt32 current_vpn = padwVpn[currentOffset + i];
                stat = _readPage(current_vpn, &(pbaData[(currentOffset + i) * BYTES_PER_PAGE]), &tempP[currentOffset + i], bInternalOp, TRUE32, scrubOnUECC);
                if (stat != FTL_SUCCESS)
                {
                    WMR_PRINT(QUAL, "read failure on vpn %d\n", current_vpn);
                    status = FALSE32;
                }
            }

            if (!status)
            {
                return FALSE32;
            }
            
            WMR_PRINT(QUAL_FATAL, "were able to overcome read failure ");
        }
        else
        {
            if(stat != FTL_SUCCESS)
                return FALSE32;
#ifndef AND_READONLY            
            if( needsRefresh )
            {
                debug(ERROR, "refresh is triggered");
                status = TRUE32;
                for (i = 0; i < currentLen; i++)
                {
                    stat = _readPage(padwVpn[currentOffset + i], &(pbaData[(currentOffset + i) * BYTES_PER_PAGE]), &tempP[currentOffset + i], bInternalOp, TRUE32, scrubOnUECC);
                    if (stat != FTL_SUCCESS)
                    {
                        status = FALSE32;
                    }
                }
                if (!status)
                {
                    return FALSE32;
                }
            }
#endif            
        }
    }

    return TRUE32;
}

#ifndef AND_READONLY
    
BOOL32 isBlockInEraseNowList(UInt16 blockNo)
{    
    UInt8 i;  
    BOOL32 res = FALSE32;
    
    for(i = 0; i < ERASE_NOW_LIST_SIZE; i++)
    {
        if(yaFTL.eraseNowList[i] == blockNo)
        {
            res = TRUE32;
            break;
        }
    }
    
    return res;
}    

BOOL32 removeBlockFromEraseNowList(UInt16 blockNo)
{    
    UInt8 i;  
    BOOL32 res = FALSE32;
    
    for(i = 0; i < ERASE_NOW_LIST_SIZE; i++)
    {
        if(yaFTL.eraseNowList[i] == blockNo)
        {
            res = TRUE32;
            yaFTL.eraseNowList[i] = 0xffff;
            break;
        }
    }
    
    return res;
}

BOOL32 addBlockToEraseNowList(UInt16 blockNo)
{    
    UInt8 i;  
    BOOL32 res = FALSE32;
    
    for(i = 0; i < ERASE_NOW_LIST_SIZE; i++)
    {
        if(yaFTL.eraseNowList[i] == blockNo)
        {
            res = TRUE32;
            break;
        }
        if(yaFTL.eraseNowList[i] == 0xffff)
        {
            yaFTL.eraseNowList[i] = blockNo;
            res = TRUE32;
            break;
        }
    }
    if(res == FALSE32)
    {
        for(i = 0; i < ERASE_NOW_LIST_SIZE; i++)
        {
            UInt16 blockToCheck = yaFTL.eraseNowList[i];
            ANDStatus status;

            if(yaFTL.blockArray[blockToCheck].status == BLOCK_FREE)
            {
                if(yaFTL.blockArray[blockToCheck].eraseBeforeUse != 0)
                {
                    if (yaFTL.cxtValid == 1)
                    { 
                        invalidateCXT();
                        yaFTL.cxtValid = 0;
                    }
                    // Erase it
                    status = VFL_Erase(blockToCheck, TRUE32);
                    if (status != FTL_SUCCESS)
                    {
                        WMR_PANIC("VFL_Erase(%d) failed with 0x%08x", (UInt32) blockToCheck, status);
                        return 0xffffffff;
                    }
                    yaFTL.erasedBlockCount++;
                    yaFTL.blockArray[blockToCheck].eraseBeforeUse = 0;
                    yaFTL.blockArray[blockToCheck].erasableCount++;
                    yaFTL.periodicCxt.erases++;
                    if (yaFTL.blockArray[blockToCheck].erasableCount > yaFTL.maxEraseCount)
                    {
                        yaFTL.maxEraseCount = yaFTL.blockArray[blockToCheck].erasableCount;
                    }
                }
                yaFTL.eraseNowList[i] = blockNo;
                res = TRUE32;
                break;
            }
        }
    }
    return res;
}

/*****************************************************************************/
/*                                                                           */
/* NAME                                                                      */
/*      _writePage	            				     					     */
/* DESCRIPTION                                                               */
/*     writes sigle virtual page to flash                                    */
/*                                                                           */
/*****************************************************************************/
Int32 _writePage(UInt32 vpn, UInt8 *pageData, PageMeta_t *mdPtr, BOOL32 bInternalOp)
{
    Buffer *buff;
    Int32 status;
    BOOL32 bMovingUserData;

    buff = BUF_Get(BUF_MAIN_AND_SPARE);
    if (buff == NULL)
    {
        return FTL_CRITICAL_ERROR;
    }

    if (yaFTL.cxtValid == 1)
    {
        invalidateCXT();
        yaFTL.cxtValid = 0;
    }

    buff->pData = pageData;
    if (mdPtr != NULL)
    {
        WMR_MEMCPY(buff->pSpare, mdPtr, sizeof(PageMeta_t));
        bMovingUserData = bInternalOp && META_IS_DATA(mdPtr);
    }
    else
    {
        bMovingUserData = FALSE32;
    }

    status = VFL_Write(vpn, buff, TRUE32, bMovingUserData);
    if (status != VFL_SUCCESS)
    {
        WMR_PRINT(QUAL, "we got write failure at vpn %d ", vpn);
        if(addBlockToEraseNowList(vpn / PAGES_PER_SUBLK) == FALSE32)
        {
            // Invalidate context
            if (yaFTL.cxtValid == 1)
            {
                invalidateCXT();
                yaFTL.cxtValid = 0;
            }
            WMR_PANIC("cannot add block %d to a scrub list", vpn / PAGES_PER_SUBLK);   
        }
    }
    BUF_Release(buff);
    return status;
}
#endif

/*****************************************************************************/
/*                                                                           */
/* NAME                                                                      */
/*      _readPage	            				     					     */
/* DESCRIPTION                                                               */
/*     reads sigle virtual page from flash                                   */
/*                                                                           */
/*****************************************************************************/
Int32 _readPage(UInt32 vpn, UInt8 *pageData, PageMeta_t *mdPtr, BOOL32 bInternalOp, BOOL32 boolCleanCheck, BOOL32 bMarkECCForScrub)
{
    Buffer buff;
    long status;
    BOOL32 needsRefresh = FALSE32; 
    
    buff.pData = pageData;
    if (mdPtr)
    {
        buff.pSpare = (UInt8  *)mdPtr;
    }
    else
    {
        buff.pSpare = (UInt8 *)yaFTL.singleMdPtr;
    }

    status = VFL_Read(vpn, &buff, boolCleanCheck, bMarkECCForScrub, &needsRefresh, NULL, bInternalOp);
    if ((status == VFL_CRITICAL_ERROR) || (status == VFL_U_ECC_ERROR))
    {
        if (bMarkECCForScrub)
        {
            // This is more serious if we were expecting it to be good
            WMR_PRINT(QUAL, "we got read failure at x%x block 0x%x block status x%x scrub %d\n", 
              vpn, vpn / PAGES_PER_SUBLK, yaFTL.blockArray[vpn / PAGES_PER_SUBLK].status, bMarkECCForScrub);
#ifndef AND_READONLY            
            if(addBlockToEraseNowList(vpn / PAGES_PER_SUBLK) == FALSE32)
            {
                // Invalidate context
                if (yaFTL.cxtValid == 1)
                {
                    invalidateCXT();
                    yaFTL.cxtValid = 0;
                }
                WMR_PANIC("cannot add block %d to a scrub list", vpn / PAGES_PER_SUBLK);   
            }
#endif            
        }
        else
        {
            debug(ERROR, "we got read failure at x%x block 0x%x block status x%x scrub %d", 
              vpn, vpn / PAGES_PER_SUBLK, yaFTL.blockArray[vpn / PAGES_PER_SUBLK].status, bMarkECCForScrub);
        }
        status = FTL_CRITICAL_ERROR;
    }

    updateReadCounter((UInt16) vpn / PAGES_PER_SUBLK);
    
    if (needsRefresh && bMarkECCForScrub)
    {
        WMR_PRINT(QUAL, "vfl suggests read refresh on vpn %d\n", vpn);
    }
#ifndef AND_READONLY
    if ((yaFTL.blockArray[vpn / PAGES_PER_SUBLK].pagesRead > YAFTL_READ_DISTURB_LIMIT) || (needsRefresh == TRUE32))
    {
        yaFTL.readLimit = TRUE32;
        if( needsRefresh )
        {
            debug(ERROR, "refresh is triggered at vpn 0x%x\n",vpn);
            yaFTL.blockArray[vpn / PAGES_PER_SUBLK].pagesRead = 0xffff; //   !!! this is a prep for readDistrub handling
            WMR_TRACE_IST_1(ReadRefreshEnq, NONE, vpn / PAGES_PER_SUBLK);
        }
        else
        {
            WMR_TRACE_IST_1(ReadCountEnq, NONE, vpn / PAGES_PER_SUBLK);
        }
    }
#endif // !  AND_READONLY

    return status;
}

static BOOL32 YAFTL_ECBins(void * pvoidStructBuffer, UInt32 * pdwStructSize)
{
    BOOL32 boolRes = FALSE32;
    UInt16 blockIdx;
    FTLBinsStruct hdr;
    const UInt32 kdwStructSize = sizeof(hdr) + sizeof(*hdr.usage) * FTL_NUM_EC_BINS;

    if (!yaFTL.blockArray)
    {
        return FALSE32;
    }

    if (pvoidStructBuffer && pdwStructSize && (*pdwStructSize >= kdwStructSize))
    {
        const UInt32 max_bin_val = FTL_MAX_EC_BIN_VAL;
        const UInt32 bin_size = FTL_EC_BIN_SIZE;
        UInt32 size = *pdwStructSize; // only used for WMR_FILL_STRUCT

        hdr.maxValue = max_bin_val;
        hdr.binCount = FTL_NUM_EC_BINS;
        WMR_MEMSET(pvoidStructBuffer, 0, kdwStructSize);
        WMR_FILL_STRUCT(pvoidStructBuffer, &size, &hdr, sizeof(hdr));

        for (blockIdx = 0; blockIdx < FTL_AREA_SIZE; ++blockIdx)
        {
            UInt32 index;
            UInt16 usage;
            void *cursor;

            if (yaFTL.blockArray[blockIdx].erasableCount >= max_bin_val)
            {
                index = FTL_NUM_EC_BINS - 1;
            }
            else
            {
                index = yaFTL.blockArray[blockIdx].erasableCount / bin_size;
            }

            cursor = ((char *)pvoidStructBuffer) + WMR_OFFSETOF(FTLBinsStruct, usage[index]);
            WMR_MEMCPY(&usage, cursor, sizeof(usage));
            usage++;
            WMR_MEMCPY(cursor, &usage, sizeof(usage));
        }
        boolRes = TRUE32;
    }

    if (pdwStructSize)
    {
        *pdwStructSize = kdwStructSize;
        boolRes = TRUE32;
    }

    return boolRes;
}

static BOOL32 YAFTL_RCBins(void * pvoidStructBuffer, UInt32 * pdwStructSize)
{
    BOOL32 boolRes = FALSE32;
    UInt16 blockIdx;
    FTLBinsStruct hdr;
    const UInt32 kdwStructSize = sizeof(hdr) + sizeof(*hdr.usage) * FTL_NUM_RC_BINS;

    if (!yaFTL.blockArray)
    {
        return FALSE32;
    }

    if (pvoidStructBuffer && pdwStructSize && (*pdwStructSize >= kdwStructSize))
    {
        const UInt32 max_bin_val = YAFTL_READ_DISTURB_LIMIT;
        const UInt32 bin_size = FTL_RC_BIN_SIZE(max_bin_val);
        UInt32 size = *pdwStructSize; // only used for WMR_FILL_STRUCT

        hdr.maxValue = max_bin_val;
        hdr.binCount = FTL_NUM_RC_BINS;
        WMR_MEMSET(pvoidStructBuffer, 0, kdwStructSize);
        WMR_FILL_STRUCT(pvoidStructBuffer, &size, &hdr, sizeof(hdr));

        for (blockIdx = 0; blockIdx < FTL_AREA_SIZE; ++blockIdx)
        {
            UInt32 index;
            UInt16 usage;
            void *cursor;

            if (yaFTL.blockArray[blockIdx].pagesRead >= max_bin_val)
            {
                index = FTL_NUM_RC_BINS - 1;
            }
            else
            {
                index = yaFTL.blockArray[blockIdx].pagesRead / bin_size;
            }

            cursor = ((char *)pvoidStructBuffer) + WMR_OFFSETOF(FTLBinsStruct, usage[index]);
            WMR_MEMCPY(&usage, cursor, sizeof(usage));
            usage++;
            WMR_MEMCPY(cursor, &usage, sizeof(usage));
        }
        boolRes = TRUE32;
    }

    if (pdwStructSize)
    {
        *pdwStructSize = kdwStructSize;
        boolRes = TRUE32;
    }

    return boolRes;
}

static void fillUpTLBlockStructure(UInt32 blockNo, ANDFTLBlockStruct * tlBlockStruct)
{
    tlBlockStruct->blockSizeInLbas = PAGES_PER_SUBLK;
    tlBlockStruct->validLbas = 0;
    tlBlockStruct->blockType = 0;
    switch (yaFTL.blockArray[blockNo].status)
    {
        case BLOCK_ALLOCATED:
        case BLOCK_CURRENT:
        case BLOCK_GC:
        {
            tlBlockStruct->blockType |= TL_BLOCK_DATA;
            tlBlockStruct->validLbas = yaFTL.blockArray[blockNo].validPagesDNo;
            break;
        }

        case BLOCK_I_ALLOCATED:
        case BLOCK_I_CURRENT:
        case BLOCK_I_GC:  
        {
            tlBlockStruct->blockType |= TL_BLOCK_INDEX;
            tlBlockStruct->validLbas = yaFTL.blockArray[blockNo].validPagesINo;
            break;
        }

        case BLOCK_FREE:
        {
            tlBlockStruct->blockType |= TL_BLOCK_FREE;
            break;
        }
        case BLOCK_CTX_CNTR:
        case BLOCK_CTX_CURRENT:
            tlBlockStruct->blockType |= TL_BLOCK_CXT;
            break;
    
        default:
            tlBlockStruct->blockType |= TL_BLOCK_UNKNOWN;
    }
}
    
static BOOL32 YAFTL_GetStruct(UInt32 dwStructType, void * pvoidStructBuffer, UInt32 * pdwStructSize)
{
    BOOL32 boolRes = FALSE32;

    switch (dwStructType)
    {
    case AND_STRUCT_FTL_GET_TYPE:
    {
        UInt8 bFTLType = FTL_TYPE_YAFTL;
        boolRes = WMR_FILL_STRUCT(pvoidStructBuffer, pdwStructSize, &bFTLType, sizeof(bFTLType));
        break;
    }

#ifdef AND_COLLECT_STATISTICS
    case AND_STRUCT_FTL_GET_FTL_STATS_SIZE:
    {
        UInt32 dwStatsSize = sizeof(FTLStatistics);
        boolRes = WMR_FILL_STRUCT(pvoidStructBuffer, pdwStructSize, &dwStatsSize, sizeof(dwStatsSize));
        break;
    }

    case AND_STRUCT_FTL_STATISTICS:
    {
#if ENABLE_L2V_TREE
        stFTLStatistics.ddwNodesFree = L2V.Pool.FreeCount;
        stFTLStatistics.ddwNodesTotal = L2V_NODEPOOL_COUNT;
#endif //ENABLE_L2V_TREE
        boolRes = WMR_FILL_STRUCT(pvoidStructBuffer, pdwStructSize, &stFTLStatistics, sizeof(stFTLStatistics));
        break;
    }
#endif

    case AND_STRUCT_FTL_GET_FTL_DEVICEINFO_SIZE:
    {
        UInt32 dwFTLDevInfoSize = sizeof(FTLWMRDeviceInfo);
        boolRes = WMR_FILL_STRUCT(pvoidStructBuffer, pdwStructSize, &dwFTLDevInfoSize, sizeof(dwFTLDevInfoSize));
        break;
    }

    case AND_STRUCT_FTL_GET_FTL_DEVICEINFO:
    {
        boolRes = WMR_FILL_STRUCT(pvoidStructBuffer, pdwStructSize, &yaFTL_FTLDeviceInfo, sizeof(yaFTL_FTLDeviceInfo));

        break;
    }

    case AND_STRUCT_FTL_GETADDRESS:
    {
        ANDAddressStruct stAddr;
        UInt32 dwAddrSize = sizeof(stAddr);
        BOOL32 boolFillRes;
        
        if (pvoidStructBuffer && pdwStructSize && (*pdwStructSize >= sizeof(stAddr)))
        {    
            WMR_MEMCPY(&stAddr, pvoidStructBuffer, sizeof(stAddr));

            if((YAFTL_Read(stAddr.dwLpn, 1, NULL) == FTL_SUCCESS) && (yaFTL.multiReadvpN[0] != 0xffffffff))
            {   
                stAddr.dwVpn = yaFTL.multiReadvpN[0];
                boolRes = VFL_GetStruct(AND_STRUCT_VFL_GETADDRESS, &stAddr, &dwAddrSize);
            }
            else
            {
                boolRes = FALSE32;
            }
        }
        
        boolFillRes = WMR_FILL_STRUCT(pvoidStructBuffer, pdwStructSize, &stAddr, sizeof(stAddr));
        
        if (boolRes)
        {
            boolRes = boolFillRes;
        }
        
        break;
    }        
            
#ifndef AND_READONLY
    case AND_FUNCTION_INDEX_CACHE_UPDATE:
    {
        UInt32 i;

        if (!pdwStructSize)
        {
            boolRes = FALSE32;
            break;
        }

        boolRes = TRUE32; // So we can fail if the backup reallocate fails
        flushIndexCache(0);
        //todo : make sure we update TOC to reflect that current cache is gone
        WMR_BufZone_Free(&yaFTL.BufZone_IndexCache);
        WMR_FREE(yaFTL.indexCache, yaFTL.indexCacheSize * sizeof(CacheIndexEntry));              /* [INDEX_CACHE_SIZE] cached index pages */
        yaFTL.indexCacheSize = *pdwStructSize;

 ALLOCATE_DEFAULT_CACHE:
        yaFTL.indexCache = (CacheIndexEntry *)WMR_MALLOC(yaFTL.indexCacheSize * sizeof(CacheIndexEntry));
        if (yaFTL.indexCache == NULL)
        {
            boolRes = FALSE32;
            yaFTL.indexCacheSize = DEFAULT_INDEX_CACHE_SIZE;
            goto ALLOCATE_DEFAULT_CACHE;
        }

        // Allocate index cache buffers
        WMR_BufZone_Init(&yaFTL.BufZone_IndexCache);
        for (i = 0; i < yaFTL.indexCacheSize; i++)
        {
            // Pre-reserve
            yaFTL.indexCache[i].indexPage = (UInt32 *)WMR_Buf_Alloc_ForDMA(&yaFTL.BufZone_IndexCache, BYTES_PER_PAGE);
        }

        // Finish allocations
        if (!WMR_BufZone_FinishedAllocs(&yaFTL.BufZone_IndexCache))
        {
            WMR_FREE(yaFTL.indexCache, yaFTL.indexCacheSize * sizeof(CacheIndexEntry));
            WMR_ASSERT(FALSE32 != boolRes); // Panics if the re-allocate fails
            boolRes = FALSE32;
            yaFTL.indexCacheSize = DEFAULT_INDEX_CACHE_SIZE;
            goto ALLOCATE_DEFAULT_CACHE;
        }

        // Rebase and initialize
        for (i = 0; i < yaFTL.indexCacheSize; i++)
        {
            WMR_BufZone_Rebase(&yaFTL.BufZone_IndexCache, (void**)&yaFTL.indexCache[i].indexPage);
            yaFTL.indexCache[i].status = FREE_CACHE_PAGE;
            yaFTL.indexCache[i].tocEntry = 0xffff;
            yaFTL.indexCache[i].counter = 0;
            WMR_MEMSET(yaFTL.indexCache[i].indexPage, 0xff, BYTES_PER_PAGE);
        }

        // Finish rebases
        WMR_BufZone_FinishedRebases(&yaFTL.BufZone_IndexCache);

        while (quickMountUpdate(FALSE32) != FTL_SUCCESS)
        {
            debug(ERROR, "YAFTL_flushquickmountupdate failed");
        }
//            debug(ERROR, "YAFTL_GetStruct : update cache size to %d done ",indexCacheSize);
        yaFTL.freeCachePages = yaFTL.indexCacheSize;

        boolRes = TRUE32;
        break;
    }
#endif // ! AND_READONLY

    case AND_STRUCT_FTL_GET_FTL_EC_BINS:
    {
        boolRes = YAFTL_ECBins(pvoidStructBuffer, pdwStructSize);
        break;
    }

    case AND_STRUCT_FTL_GET_FTL_RC_BINS:
    {
        boolRes = YAFTL_RCBins(pvoidStructBuffer, pdwStructSize);
        break;
    }

    case AND_STRUCT_FTL_RECOMMEND_CONTENT_DELETION:
    {
        UInt32 data = 0;
        boolRes = WMR_FILL_STRUCT(pvoidStructBuffer, pdwStructSize, &data, sizeof(data));
        break;
    }
            
    case AND_STRUCT_FTL_GET_FTL_BLOCK_COUNT:
    {
        UInt32 tlBlockCount = FTL_AREA_SIZE;
        boolRes = WMR_FILL_STRUCT(pvoidStructBuffer, pdwStructSize, &tlBlockCount, sizeof(tlBlockCount));
        break;
    }
        
    case AND_STRUCT_FTL_GET_FTL_BLOCK_STAT:
    {
        UInt32 i, tmpBuffSize = *pdwStructSize, tmpChunkSize;
        ANDFTLBlockStruct tempStruct;
        UInt8 * tmpPtr = (UInt8 *)pvoidStructBuffer;
        
        if(tmpBuffSize >= (FTL_AREA_SIZE * sizeof(tempStruct)))
        {
            for(i = 0; (i < FTL_AREA_SIZE) && (tmpBuffSize >= sizeof(tempStruct)); i++, tmpBuffSize -= sizeof(tempStruct))
            {
                fillUpTLBlockStructure(i, &tempStruct);
                tmpChunkSize = tmpBuffSize;
                boolRes = WMR_FILL_STRUCT(tmpPtr + (i * sizeof(ANDFTLBlockStruct)), &tmpChunkSize, &tempStruct, sizeof(tempStruct));
            }
        }
        break;
    }
            
    default:
        boolRes = FALSE32;
    }
  
    return boolRes;
}
#ifdef AND_COLLECT_STATISTICS

void _UpdateStatisticsCounters(UInt8 *pabSrc, void *pvoidStat, UInt32 dwSize)
{
    UInt32 dwIdx;
    UInt64 * paddwStat = (UInt64 *)pvoidStat;

    for (dwIdx = 0; dwIdx < (dwSize / sizeof(UInt64)); dwIdx++)
    {
        UInt64 ddwTemp;
        WMR_MEMCPY(&ddwTemp, &pabSrc[dwIdx * sizeof(UInt64)], sizeof(UInt64));
        paddwStat[dwIdx] = ddwTemp; //intentionally overwrite, not increment
    }
}

#ifndef AND_READONLY
static BOOL32 _FTLGetStatisticsToCxt(UInt8 * pabData)
{
    UInt32 dwStatBuffSize;
    UInt32 dwStatVersion = AND_EXPORT_STRUCTURE_VERSION;

    UInt8  *pabCursor = pabData;
    UInt8  *pabVersion = &pabData[BYTES_PER_PAGE - sizeof(UInt32)];

    WMR_MEMSET(pabCursor, 0, BYTES_PER_PAGE);

    WMR_MEMCPY(pabCursor, &stFTLStatistics, sizeof(stFTLStatistics));
    pabCursor += AND_STATISTICS_SIZE_PER_LAYER;

    dwStatBuffSize = AND_STATISTICS_SIZE_PER_LAYER;
    VFL_GetStruct(AND_STRUCT_VFL_STATISTICS, pabCursor, &dwStatBuffSize);
    pabCursor += AND_STATISTICS_SIZE_PER_LAYER;

    dwStatBuffSize = AND_STATISTICS_SIZE_PER_LAYER;
    VFL_GetStruct(AND_STRUCT_VFL_FILSTATISTICS, pabCursor, &dwStatBuffSize);

    // mark the statistics version
    WMR_MEMCPY(pabVersion, &dwStatVersion, sizeof(UInt32));
    return TRUE32;
}
#endif //ifndef AND_READONLY

static BOOL32 _FTLSetStatisticsFromCxt(UInt8 * pabData)
{
    UInt32 dwStatBuffSize;
    Buffer *pBuff = BUF_Get(BUF_MAIN_AND_SPARE);
    UInt8  *pabCursor;

    if (!pBuff)
    {
        debug(ERROR, "BUF_Get failed in _FTLSetStatisticsFromCxt!\n");
        return FALSE32;
    }

    pabCursor = pabData;

    _UpdateStatisticsCounters(pabCursor, &stFTLStatistics, sizeof(FTLStatistics));
    pabCursor += AND_STATISTICS_SIZE_PER_LAYER;

    dwStatBuffSize = AND_STATISTICS_SIZE_PER_LAYER;
    VFL_GetStruct(AND_STRUCT_VFL_STATISTICS, pBuff->pData, &dwStatBuffSize);
    _UpdateStatisticsCounters(pabCursor, pBuff->pData, dwStatBuffSize);
    pabCursor += AND_STATISTICS_SIZE_PER_LAYER;
    VFL_SetStruct(AND_STRUCT_VFL_STATISTICS, pBuff->pData, dwStatBuffSize);

    dwStatBuffSize = AND_STATISTICS_SIZE_PER_LAYER;
    VFL_GetStruct(AND_STRUCT_VFL_FILSTATISTICS, pBuff->pData, &dwStatBuffSize);
    _UpdateStatisticsCounters(pabCursor, pBuff->pData, dwStatBuffSize);
    VFL_SetStruct(AND_STRUCT_VFL_FILSTATISTICS, pBuff->pData, dwStatBuffSize);

    BUF_Release(pBuff);
    return TRUE32;
}

#endif

static UInt32 _getMinorVersion(void)
{
    return kYaftlMinorVersion;   
}

#ifndef AND_READONLY
// This function is stateless. Don't use stFTLDeviceInfo
static UInt32 yaftl_ConvertUserMBtoFTLSuperblocks(VFLFunctions *vfl, UInt32 user_mb)
{
    UInt32 ftl_virtual_blocks;
    const UInt32 page_size = vfl->GetDeviceInfo(AND_DEVINFO_BYTES_PER_PAGE);
    const UInt32 pages_per_vb = vfl->GetDeviceInfo(AND_DEVINFO_PAGES_PER_SUBLK);
    const UInt32 requested_pages = (user_mb * 1024) / (page_size / 1024);
    
    ftl_virtual_blocks = ((((requested_pages * 100) / DEFAULT_EXPO_RATIO)) / pages_per_vb) + FTL_CXT_SECTION_SIZE + DEFAULT_INDEX_CACHE_SIZE + 1;
    
    return ftl_virtual_blocks;
}
#endif //!AND_READONLY

#if NAND_PPN
/*****************************************************************************/
/*                                                                           */
/* NAME                                                                      */
/*      YAFTL_PPN_Register                                                   */
/* DESCRIPTION                                                               */
/*     register YAFTL access functions                                       */
/*                                                                           */
/*****************************************************************************/
static FTLFunctions _yaftl_functions = 
{
    .Init = YAFTL_Init,
    .Open = YAFTL_Open,

    .Read = YAFTL_Read,
    .Close = YAFTL_Close,
    .GetStruct = YAFTL_GetStruct,
    .GetMinorVersion = _getMinorVersion,

#ifndef AND_READONLY
    .ConvertUserMBtoFTLSuperblocks = yaftl_ConvertUserMBtoFTLSuperblocks,
    .Write = YAFTL_Write,
    .Unmap = YAFTL_Unmap,
    .Format = YAFTL_Format,
    .WearLevel = YAFTL_WearLevel,
    .GarbageCollect = YAFTL_GarbageCollect,
    .ShutdownNotify = YAFTL_ShutdownNotify,
    .WriteStats = YAFTL_WriteStats,
#endif
};

FTLFunctions *YAFTL_PPN_Register(void)
{
    return &_yaftl_functions;
}

#elif NAND_RAW

/*****************************************************************************/
/*                                                                           */
/* NAME                                                                      */
/*      registerYAFTL                                                        */
/* DESCRIPTION                                                               */
/*     register YAFTL access functions                                       */
/*                                                                           */
/*****************************************************************************/

void YAFTL_Register(FTLFunctions * pFTLFunctions)
{
    pFTLFunctions->Init = YAFTL_Init;
    pFTLFunctions->Open = YAFTL_Open;

    pFTLFunctions->Read = YAFTL_Read;
    pFTLFunctions->Close = YAFTL_Close;
    pFTLFunctions->GetStruct = YAFTL_GetStruct;
    pFTLFunctions->GetMinorVersion = _getMinorVersion;

#ifndef AND_READONLY
    pFTLFunctions->ConvertUserMBtoFTLSuperblocks = yaftl_ConvertUserMBtoFTLSuperblocks;
    pFTLFunctions->Write = YAFTL_Write;
    pFTLFunctions->Unmap = YAFTL_Unmap;
    pFTLFunctions->Format = YAFTL_Format;
    pFTLFunctions->WearLevel = YAFTL_WearLevel;
    pFTLFunctions->GarbageCollect = YAFTL_GarbageCollect;
    pFTLFunctions->ShutdownNotify = YAFTL_ShutdownNotify;
    pFTLFunctions->WriteStats = YAFTL_WriteStats;
#endif
}
#else
#error PPN or raw?
#endif

