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

#ifndef _YAFTLTYPES_H_
#define _YAFTLTYPES_H_

#include "yaFTL_whoami.h"
#include "VFLBuffer.h"
#include "yaFTL_Defines.h"
#include "L2V/L2V_Extern.h"
#include "WMRBuf.h"
#include "yaFTL_meta.h"

typedef struct
{
    // Info generated by CalcGlobal
    UInt16 wPagesPerVb;                /* the count of pages per virtual block */
    UInt16 wUserVbTotal;               /* the total number of data virtual block */
    UInt32 dwUserPagesTotal;           /* the total number of data sector   */
    UInt16 wBytesPerPage;              /* bytes per page (main)			 */
    UInt16 wNumOfBanks;                /* the number of banks				 */
    UInt16 wBytesPerPageMeta;          /* bytes per page (meta)			 */
} FTLWMRDeviceInfo;

#define     PAGES_PER_SUBLK         (yaFTL_FTLDeviceInfo.wPagesPerVb)
#define     NUM_BANKS               (yaFTL_FTLDeviceInfo.wNumOfBanks)
#define     FTL_AREA_SIZE           (yaFTL_FTLDeviceInfo.wUserVbTotal)
#define     USER_PAGES_TOTAL        (yaFTL_FTLDeviceInfo.dwUserPagesTotal)
#define     BYTES_PER_PAGE          (yaFTL_FTLDeviceInfo.wBytesPerPage)
#define     BYTES_PER_PAGE_META     (yaFTL_FTLDeviceInfo.wBytesPerPageMeta)
#define     NUM_BANKS               (yaFTL_FTLDeviceInfo.wNumOfBanks)

typedef struct
{
//	UInt32 serialCounter;   // serial allocation counter that would be used for restore
//	UInt16 validPagesNo;    // Number of valid pages in block
    UInt32 erasableCount;
    UInt16 validPagesDNo;
    UInt16 validPagesINo;
    UInt16 pagesRead;
    UInt8 status;
    UInt8 eraseBeforeUse;
    UInt8 pagesReadSubCounter;
} BlockEntry;

// TOCEntry: pointer to cache index if the given index page is in the cache, or pointer to NAND if not.
// P.S.: not related to BlockTOCs.
typedef struct
{
    UInt32 indexPage;    // location on the flash
    UInt16 cacheIndex;   // 0xff means not cached
} TOCEntry;

typedef struct
{
    UInt32 *indexPage;  // pointer to the page address array
    UInt32 tocEntry;    // index of the TOCEntry
    UInt16 counter;     // number of hits on this cache
    UInt16 status;      // free/occupied/dirty
} CacheIndexEntry;

typedef struct
{
    UInt64 ddwPagesWrittenCnt;
    UInt64 ddwPagesReadCnt;
    UInt64 ddwFTLWriteCnt;
    UInt64 ddwFTLReadCnt;
    UInt64 ddwDataGCCnt;
    UInt64 ddwIndexGCCnt;
    UInt64 ddwEmptyGCDataCnt; /* case there is data to copy */
    UInt64 ddwEmptyGCIndexCnt; /* case there is data to copy */
    UInt64 ddwValidDataPages;
    UInt64 ddwValidIndexPages;
    UInt64 ddwFreeBlks;
    UInt64 ddwFTLRestoreCnt;
    UInt64 ddwReadDisturbHandleCall;
    UInt64 ddwWearLevelCnt;
    UInt64 ddwFlushCnt;
    UInt64 ddwRefreshCnt;
    UInt64 ddwRead[8];
    UInt64 ddwReadOver8;
    UInt64 ddwWrite[8];
    UInt64 ddwWriteOver8;
    UInt64 ddwNodesFree;
    UInt64 ddwNodesTotal;
} FTLStatistics;

#define FTL_STATISTICS_DESCREPTION { \
        "ddwPagesWrittenCnt", \
        "ddwPagesReadCnt", \
        "ddwFTLWriteCnt", \
        "ddwFTLReadCnt", \
        "ddwDataGCCnt", \
        "ddwIndexGCCnt", \
        "ddwEmptyGCDataCnt", \
        "ddwEmptyGCIndexCnt", \
        "ddwValidDataPages", \
        "ddwValidIndexPages", \
        "ddwFreeBlks", \
        "ddwFTLRestoreCnt", \
        "ddwReadDisturbHandleCall", \
        "ddwWearLevelCnt", \
        "ddwFlushCnt", \
        "ddwRefreshCnt", \
        "ddwRead1", \
        "ddwRead2", \
        "ddwRead3", \
        "ddwRead4", \
        "ddwRead5", \
        "ddwRead6", \
        "ddwRead7", \
        "ddwRead8", \
        "ddwReadOver8", \
        "ddwWrite1", \
        "ddwWrite2", \
        "ddwWrite3", \
        "ddwWrite4", \
        "ddwWrite5", \
        "ddwWrite6", \
        "ddwWrite7", \
        "ddwWrite8", \
        "ddwWriteOver8", \
        "ddwNodesFree", \
        "ddwNodesTotal", \
}

#define GCFIFO_DEPTH 16
typedef struct
{
    UInt32 block[GCFIFO_DEPTH + 1];
    UInt32 head, tail;  // head==tail means it is empty
} GC_Fifo_t;

typedef enum
    {
        GCD_IDLE,
        GCD_EVICT,
        GCD_ERASE,
        GCD_FIXINDEX_RETRY,
        GCD_FIXDATA_RETRY,
    } GCDState;


typedef struct
{
    // In:
    UInt32 in_block;

    // Work FIFO for error handling
    GC_Fifo_t workFifo;

    // Choose out:
    UInt32 chosenBlock;
    UInt32 chosenValid;
    UInt32 chosenErases;

    // Buffers:
    UInt32 outstandingAllocs;
    Buffer *tmpB1, *tmpB2, *tmpB3;
    UInt32 *TOCbuff;
    UInt8  *pageBuffer;

    // GC zone:
    UInt32 *vpns;
    UInt8  *zone;
    PageMeta_t *meta;
    UInt32 curZoneSize;
    UInt32 ueccCounter;

    // State:
    GCDState state;
    UInt32 curPage;
    UInt32 pagesRange;
    UInt32 pagesCopied;
    Int32 gcDataAdjust; // only meaningful for data GC

    L2V_SearchCtx_t read_c; // Search context for GC translation
} GC_Ctx_t;

#define ERASE_NOW_LIST_SIZE 5

typedef struct
{
    WMR_BufZone_t BufZone; // Buffer zone allocators
    WMR_BufZone_t BufZone_IndexCache; // ...
    UInt16 controlPageNo, indexPageRatio;    /* TOC of each block contains Lpa for each page written to block */
    UInt32 indexSize;                        /* index size in blocks */
    UInt32 dataSize;
    UInt32 logicalPartitionSize;             /* partititon size in pages exposed to file system */
    UInt16 TOCtableEntriesNo;                /* number of entries in index TOC table */

    UInt16 freeCachePages;
    UInt16 erasedBlockCount;
    #ifndef AND_READONLY
    UInt16 nextFreeCachePage;
    UInt64 cxtAllocation;
    UInt32 currentCXTIndex, FTLRestoreCnt;
    UInt8 cxtEraseCounter;
    UInt32 currentDBlockAllocationNo;
    UInt8 cxtValid;
    UInt8 formatWasCalled;
    BOOL32 readLimit;
    UInt16 eraseNowList[ERASE_NOW_LIST_SIZE];
    #endif

    struct
    {
        struct
        {
            UInt32 block;
            UInt32 *TOC;
            UInt16 nextPage;
            WeaveSeq_t minWeaveSeq;
        } data;
        struct
        {
            UInt32 block;
            UInt32 *TOC;
            UInt16 nextPage;
            WeaveSeq_t minWeaveSeq;
        } index;

        WeaveSeq_t weaveSeq;
        UInt32 lastBlock;
    } wrState;

    struct
    {
        UInt32 size;
        UInt32 curAge;
        UInt32 free;
        UInt32 isData;
        UInt32 **BTOC;
        UInt32 *sb;
        Int32 *age;
        WMR_BufZone_t BufZone; // Buffer zone allocator

        // For srcVpn:
        UInt32 srcSize;
        UInt32 curSrc;
        UInt32 *srcSb;
        UInt32 **srcVpn;
        UInt32 isLocked;
    } BTOCcache;

    struct
    {
        struct
        {
            UInt32 allocdBlocks;
            Int32 freeBlocks;
            UInt32 validPages;
        } data;
        struct
        {
            UInt32 allocdBlocks;
            Int32 freeBlocks;
            UInt32 validPages;
        } index;
        UInt32 freeBlocks;
        BOOL32 distCheckable;
    } seaState;

    struct
    {
        UInt32 blocksSince;
        BOOL32 pointOfNoReturn;
    } wearLevel;

    struct
    {
        UInt32 pagesWritten;
        UInt32 erases;
    } periodicCxt;

    TOCEntry *tocArray;                      /* [TOC_SIZE]  Table of content that contains location of index pages in flash and index cache */
    CacheIndexEntry  *indexCache;            /* [INDEX_CACHE_SIZE] cached index pages */
    BlockEntry *blockArray;                  /* [YAFTL_NO_OF_BLOCKS] block information */
    UInt32 *restoreTOC; // blockTOC for restore

    PageMeta_t *meta_restoreIdx;
    PageMeta_t *meta_restoreData;
    PageMeta_t *meta_readCxt;
    PageMeta_t *meta_quickRestore;
    PageMeta_t *meta_invalidateCxt;
    PageMeta_t *meta_writeCxt;
    PageMeta_t *meta_restoreMountErase;
    PageMeta_t *meta_restoreMount;
    PageMeta_t *meta_clearEntryInCache;
    PageMeta_t *meta_writeBTOC;
    PageMeta_t *meta_indexLoadDirty;
    PageMeta_t *meta_getStats;
    PageMeta_t *meta_GetBlockTOC;
    PageMeta_t *meta_IsDataPageCurrent;
    PageMeta_t *meta_WriteZoneData;

    UInt8  *quickRestore_tBuff;
    UInt32 *tmpReadBuffer; // = NULL;
    UInt32 readBufferIndex; // = 0xffffffff;
    PageMeta_t *writeMdPtr; // = NULL;
    UInt32 *multiReadvpN; // = NULL;
    PageMeta_t *readMdPtr; // = NULL;
    PageMeta_t *singleMdPtr;
    UInt32 cxtSize, cxtSizeTOC, cxtSizeBlockStatus, cxtSizeBlockRead,
           cxtSizeBlockErase, cxtSizeValidPagesD, cxtSizeValidPagesI;
    UInt32 currentCxtVpn; // = 0xffffffff;
    UInt16 cxtTable[FTL_CXT_SECTION_SIZE];
    UInt32 maxEraseCount, minEraseCount;        // = 0

    UInt32 indexCacheSize; // = DEFAULT_INDEX_CACHE_SIZE;
    UInt8 exportedRatio; // = DEFAULT_EXPO_RATIO;

    // Garbage collection structures
    struct
    {
        UInt32 zoneSize;
        GC_Ctx_t data;
        GC_Ctx_t index;
        WMR_BufZone_t BufZone; // Buffer zone allocator
        PageMeta_t *meta_buf;
    } gc;

    // Read search context
    L2V_SearchCtx_t read_c;
} yaFTL_t;

#endif /* _YAFTLTYPES_H_ */