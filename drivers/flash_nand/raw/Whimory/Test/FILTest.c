// Copyright (C) 2009 Apple Inc. All rights reserved.
//
// This document is the property of Apple Inc.
// It is considered confidential and proprietary.
//
// This document may not be reproduced or transmitted in any form,
// in whole or in part, without the express written permission of
// Apple Inc.
//
//
#include "WMROAM.h"
#include "FIL.h"
#include "WMRTest.h"
#include "WMRConfig.h"
#include "WMRBuf.h"

#define AND_MAX_BANKS (AND_MAX_BANKS_PER_CS * 32)
#define TEST_DATA_WATERMARK (0xB6)
#define TEST_META_WATERMARK (0xC7)
#define TEST_STATS_WATERMARK (0xD8)

// Static function definitions
static BOOL32 FILInterfaceTest(UInt16 *blockList);
static BOOL32 InitTestInfo(void);
static const char* FILErrorToString(Int32 filStatus);
static void InitPageListFromBlocks(const UInt16 *blockList, UInt32 *pageList);
static void FillBufferWithCountingPattern(UInt32 *buffer, UInt32 startValue, UInt32 bytes);
static void FillMetaBufferWithPattern(UInt8 *buffer, UInt32 startValue, UInt32 numPages);
static void FILTestHexdump(void *data, UInt32 length);

// Static variables

typedef struct
{
    UInt32 pagesPerBlock;
    UInt32 numBanks;
    UInt32 numCE;
    UInt32 bytesPerPage;
    UInt32 bytesPerSpare;
    UInt32 bufferBytesPerMeta;
    UInt32 validBytesPerMeta;
    UInt32 bootPageBytes;
    UInt32 correctableSectorsPerPage;
} FILTestInfo;

static FILTestInfo      testInfo;
static LowFuncTbl       *pFIL = NULL;
static WMR_BufZone_t    bufZone;


// Public functions
BOOL32 FIL_Test(void)
{
    UInt16 blockList[AND_MAX_BANKS] = { 0 };

    WMR_BufZone_Init(&bufZone);
    
    InitTestInfo();

    return FILInterfaceTest(blockList);
}

// Local functions

BOOL32 InitTestInfo(void)
{

    pFIL = FIL_GetFuncTbl();

    testInfo.pagesPerBlock = pFIL->GetDeviceInfo(AND_DEVINFO_PAGES_PER_BLOCK);
    WMR_ASSERT(testInfo.pagesPerBlock > 0);
    testInfo.numBanks = pFIL->GetDeviceInfo(AND_DEVINFO_NUM_OF_BANKS);
    WMR_ASSERT(testInfo.numBanks > 0);
    testInfo.numCE = pFIL->GetDeviceInfo(AND_DEVINFO_NUM_OF_CS);
    WMR_ASSERT(testInfo.numCE > 0);
    testInfo.bytesPerPage = pFIL->GetDeviceInfo(AND_DEVINFO_BYTES_PER_PAGE);
    WMR_ASSERT(testInfo.bytesPerPage > 0);
    testInfo.bytesPerSpare = pFIL->GetDeviceInfo(AND_DEVINFO_BYTES_PER_SPARE);
    WMR_ASSERT(testInfo.bytesPerSpare > 0);
    testInfo.bufferBytesPerMeta = 12;
    testInfo.validBytesPerMeta = 10;
    testInfo.bootPageBytes = pFIL->GetDeviceInfo(AND_DEVINFO_BYTES_PER_BL_PAGE);
    WMR_ASSERT(testInfo.bootPageBytes > 0);
    testInfo.correctableSectorsPerPage = testInfo.bytesPerPage / pFIL->GetDeviceInfo(AND_DEVINFO_CORRECTABLE_SIZE);
    WMR_ASSERT(testInfo.correctableSectorsPerPage > 0);

    return TRUE32;
}

static BOOL32 TestBootloaderBlock(UInt32 ce, UInt32 block, UInt8 *writeDataBuffer, UInt8 *verifyDataBuffer)
{
    UInt32 base_page = block * testInfo.pagesPerBlock;
    Int32  andStatus;
    UInt32 testSeed = 1234567890;
    UInt32 offset;
    
    // Erase block
    WMR_PRINT(ALWAYS, "Erasing CE %d Block %d\n", ce, block);
    andStatus = pFIL->Erase(ce, block);

    
    if (FIL_SUCCESS != andStatus)
    {
        WMR_PRINT(ALWAYS, "Erase failed with %s\n", FILErrorToString(andStatus));
        return FALSE32;
    }
    
    for (offset = 0; offset < testInfo.pagesPerBlock; ++offset)
    {
        const UInt32 page = base_page + offset;
        // Read bootloader page and verify clean
        WMR_ASSERT(pFIL->ReadBLPage != NULL);
        WMR_MEMSET(verifyDataBuffer, TEST_DATA_WATERMARK, testInfo.bytesPerPage);
        WMR_PRINT(ALWAYS, "Using ReadBL for clean detect CE %d Page %d\n", (UInt32) ce, page);
        andStatus = pFIL->ReadBLPage(ce, page, verifyDataBuffer);

        if (FIL_SUCCESS_CLEAN != andStatus)
        {
            WMR_PRINT(ALWAYS, "ReadBL Clean Detect failed with %s\n", FILErrorToString(andStatus));
            return FALSE32;
        }

        // Write bootloader page
        WMR_ASSERT(pFIL->WriteBLPage != NULL);
        WMR_PRINT(ALWAYS, "Writing CE %d Page %d\n", (UInt32) ce, page);
        FillBufferWithCountingPattern((UInt32*)writeDataBuffer, testSeed++, testInfo.bootPageBytes);
        andStatus = pFIL->WriteBLPage(ce, page, writeDataBuffer);

        if (FIL_SUCCESS != andStatus)
        {
            WMR_PRINT(ALWAYS, "Write failed with %s\n", FILErrorToString(andStatus));
            return FALSE32;
        }

        // Read back and verify
        WMR_MEMSET(verifyDataBuffer, TEST_DATA_WATERMARK, testInfo.bootPageBytes);
        WMR_PRINT(ALWAYS, "ReadBL CE %d Page %d\n", (UInt32) ce, page);
       
        andStatus = pFIL->ReadBLPage(ce, page, verifyDataBuffer);

        if (FIL_SUCCESS != andStatus)
        {
            WMR_PRINT(ALWAYS, "ReadBL readback failed with %s\n", FILErrorToString(andStatus));
            return FALSE32;
        }

        if (WMR_MEMCMP(writeDataBuffer, verifyDataBuffer, testInfo.bootPageBytes))
        {
            WMR_PRINT(ALWAYS, "Read back contents of boot page failed comparison\n");
            return FALSE32;
        }
    }

    return TRUE32;
}

// Block list is assumed to be numBanks long and non-overlapping banks
BOOL32 FILInterfaceTest(UInt16 *blockList)
{
    UInt8  *writeDataBuffer = NULL;
    UInt8  *writeMetaBuffer = NULL;
    UInt8  *verifyDataBuffer = NULL;
    UInt8  *verifyMetaBuffer = NULL;
    Int32  andStatus;
    UInt32 pageList[2 * AND_MAX_BANKS];
    UInt16 ceList[AND_MAX_BANKS] = { 0 };
    UInt8 *sectorStats = NULL;
    UInt32 testSeed = 1234567890;
    UInt16 bankIdx;
    const UInt32 dataBufferSize = 2 * testInfo.numBanks * testInfo.bytesPerPage;
    const UInt32 metaBufferSize = 2 * testInfo.numBanks * testInfo.bytesPerSpare;
    const UInt32 sectorStatsSize = 2 * testInfo.numBanks * testInfo.correctableSectorsPerPage;


    InitPageListFromBlocks(blockList, pageList);

    WMR_PRINT(ALWAYS, "Starting FIL Interface Test\n");

    writeMetaBuffer  = WMR_MALLOC(metaBufferSize);
    verifyMetaBuffer = WMR_MALLOC(metaBufferSize);
    sectorStats      = WMR_MALLOC(sectorStatsSize);
    writeDataBuffer  = WMR_Buf_Alloc_ForDMA(&bufZone, dataBufferSize);
    verifyDataBuffer = WMR_Buf_Alloc_ForDMA(&bufZone, dataBufferSize);
    
    WMR_BufZone_FinishedAllocs(&bufZone);

    WMR_BufZone_Rebase(&bufZone, (void **)&writeDataBuffer);
    WMR_BufZone_Rebase(&bufZone, (void **)&verifyDataBuffer);    
    
    if (!writeDataBuffer || !writeMetaBuffer || !verifyDataBuffer || !verifyMetaBuffer || !sectorStats)
    {
        WMR_PANIC("Buffer allocations failed");
        goto test_failed;
    }

    WMR_BufZone_FinishedRebases(&bufZone);
    
    // Enable whitening if available
    if (pFIL->SetWhiteningState) {
        pFIL->SetWhiteningState(TRUE32);
    }
    if (pFIL->RegisterCurrentTransaction) {
        pFIL->RegisterCurrentTransaction(0,0, NULL);
    }

    WMR_ASSERT(TestBootloaderBlock(0, 16, writeDataBuffer, verifyDataBuffer));

    // Erase Block
    WMR_ASSERT(pFIL->Erase != NULL);
    WMR_PRINT(ALWAYS, "Erasing CE %d Block %d\n", (UInt32) ceList[0], (UInt32) blockList[0]);
    andStatus = pFIL->Erase(ceList[0], blockList[0]);

    if (FIL_SUCCESS != andStatus)
    {
        WMR_PRINT(ALWAYS, "Erase failed with %s\n", FILErrorToString(andStatus));
        goto test_failed;
    }

    // Clean check single page
    WMR_ASSERT(pFIL->ReadWithECC != NULL);
    WMR_PRINT(ALWAYS, "Clean checking CE %d Page %d\n", (UInt32) ceList[0], pageList[0]);
    WMR_MEMSET(verifyDataBuffer, TEST_DATA_WATERMARK, testInfo.bytesPerPage);
    WMR_MEMSET(verifyMetaBuffer, TEST_META_WATERMARK, testInfo.bytesPerSpare);
    andStatus = pFIL->ReadWithECC(ceList[0], pageList[0], verifyDataBuffer, verifyMetaBuffer, NULL, NULL, FALSE32);

    if (FIL_SUCCESS_CLEAN != andStatus)
    {
        WMR_PRINT(ALWAYS, "Clean Detect failed with %s\n", FILErrorToString(andStatus));
        goto test_failed;
    }

    // Verify ReadNoECC on that page
    // XXX put guard bytes at end of buffer to make sure the right number of bytes are read
    WMR_ASSERT(pFIL->ReadNoECC != NULL);
    WMR_MEMSET(verifyDataBuffer, TEST_DATA_WATERMARK, testInfo.bytesPerPage);
    WMR_MEMSET(verifyMetaBuffer, TEST_META_WATERMARK, testInfo.bytesPerSpare);
    WMR_PRINT(ALWAYS, "ReadNoECC on clean CE %d Page %d\n", (UInt32) ceList[0], pageList[0]);
    andStatus = pFIL->ReadNoECC(ceList[0], pageList[0], verifyDataBuffer, verifyMetaBuffer);

    if (FIL_UNSUPPORTED_ERROR == andStatus)
    {
        WMR_PRINT(ALWAYS, "WARNING: ReadNoECC is unsupported on this platform - formatting will not work\n");
    }
    else if (FIL_SUCCESS != andStatus)
    {
        WMR_PRINT(ALWAYS, "ReadNoECC failed with %s\n", FILErrorToString(andStatus));
        goto test_failed;
    }
    else
    {
        WMR_MEMSET(writeDataBuffer, 0xFF, testInfo.bytesPerPage);
        WMR_MEMSET(writeMetaBuffer, 0xFF, testInfo.bytesPerSpare);
    
        if (WMR_MEMCMP(writeDataBuffer, verifyDataBuffer, testInfo.bytesPerPage))
        {
            WMR_PRINT(ALWAYS, "ReadNoECC failed all 1's comparison in data section\n");
            goto test_failed;
        }
    
        if (WMR_MEMCMP(writeMetaBuffer, verifyMetaBuffer, testInfo.bytesPerSpare))
        {
            WMR_PRINT(ALWAYS, "ReadNoECC failed all 1's comparison in spare section\n");
            goto test_failed;
        }
    }


    // Clean check multiple pages
    WMR_ASSERT(pFIL->ReadScatteredPages != NULL);
    InitPageListFromBlocks(blockList, pageList);
    // Pick two consecutive pages from the same block
    pageList[1] = pageList[testInfo.numBanks];
    ceList[0] = 0;
    ceList[1] = 0;
    WMR_PRINT(ALWAYS, "Clean checking CE %d Page %d & %d\n", (UInt32) ceList[0], pageList[0], pageList[1]);
    andStatus = pFIL->ReadScatteredPages(ceList, pageList, verifyDataBuffer, verifyMetaBuffer, 2, NULL, NULL, FALSE32);

    if (FIL_SUCCESS_CLEAN != andStatus)
    {
        WMR_PRINT(ALWAYS, "Multi-page clean detect failed with %s\n", FILErrorToString(andStatus));
        goto test_failed;
    }

    // Program first page
    WMR_ASSERT(pFIL->Write != NULL);
    WMR_PRINT(ALWAYS, "Writing CE %d Page %d\n", (UInt32) ceList[0], pageList[0]);
    FillBufferWithCountingPattern((UInt32*)writeDataBuffer, testSeed++, testInfo.bytesPerPage);
    FillMetaBufferWithPattern(writeMetaBuffer, testSeed++, 1);
    andStatus = pFIL->Write(ceList[0], pageList[0], writeDataBuffer, writeMetaBuffer, FALSE32);

    if (FIL_SUCCESS != andStatus)
    {
        WMR_PRINT(ALWAYS, "Write failed with %s\n", FILErrorToString(andStatus));
        goto test_failed;
    }

    // Read back and verify
    WMR_PRINT(ALWAYS, "Reading back CE %d Page %d\n", (UInt32) ceList[0], pageList[0]);
    WMR_MEMSET(verifyDataBuffer, TEST_DATA_WATERMARK, testInfo.bytesPerPage);
    WMR_MEMSET(verifyMetaBuffer, TEST_META_WATERMARK, testInfo.bufferBytesPerMeta);
    andStatus = pFIL->ReadWithECC(ceList[0], pageList[0], verifyDataBuffer, verifyMetaBuffer, NULL, NULL, FALSE32);

    if (FIL_SUCCESS != andStatus)
    {
        WMR_PRINT(ALWAYS, "Read back failed with %s\n", FILErrorToString(andStatus));
        goto test_failed;
    }

    if (WMR_MEMCMP(writeMetaBuffer, verifyMetaBuffer, testInfo.bufferBytesPerMeta))
    {
        WMR_PRINT(ALWAYS, "Read back contents failed comparison in meta section\n");
        goto test_failed;
    }

    if (WMR_MEMCMP(writeDataBuffer, verifyDataBuffer, testInfo.bytesPerPage))
    {
        WMR_PRINT(ALWAYS, "Read back contents failed comparison in data section\n");
        goto test_failed;
    }


    // Verify ReadNoECC finds non-FF data
    WMR_MEMSET(verifyDataBuffer, TEST_DATA_WATERMARK, testInfo.bytesPerPage);
    WMR_MEMSET(verifyMetaBuffer, TEST_META_WATERMARK, testInfo.bytesPerSpare);
    WMR_PRINT(ALWAYS, "ReadNoECC on written CE %d Page %d\n", (UInt32) ceList[0], pageList[0]);
    andStatus = pFIL->ReadNoECC(ceList[0], pageList[0], verifyDataBuffer, verifyMetaBuffer);

    if (FIL_UNSUPPORTED_ERROR == andStatus)
    {
        WMR_PRINT(ALWAYS, "WARNING: ReadNoECC is unsupported on this platform - formatting will not work\n");
    }
    else if (FIL_SUCCESS != andStatus)
    {
        WMR_PRINT(ALWAYS, "ReadNoECC failed with %s\n", FILErrorToString(andStatus));
        goto test_failed;
    }
    else
    {
        WMR_MEMSET(writeDataBuffer, 0xFF, testInfo.bytesPerPage);
        WMR_MEMSET(writeMetaBuffer, 0xFF, testInfo.bytesPerSpare);
    
        if (!WMR_MEMCMP(writeMetaBuffer, verifyMetaBuffer, testInfo.bytesPerSpare))
        {
            WMR_PRINT(ALWAYS, "ReadNoECC read all 1's on a programmed page in meta section\n");
            goto test_failed;
        }

        if (!WMR_MEMCMP(writeDataBuffer, verifyDataBuffer, testInfo.bytesPerPage))
        {
            WMR_PRINT(ALWAYS, "ReadNoECC read all 1's on a programmed page in data section\n");
            goto test_failed;
        }
    }


    // Read programmed + clean multiple, verify UECC status
    InitPageListFromBlocks(blockList, pageList);
    // Pick two consecutive pages from the same block
    pageList[1] = pageList[testInfo.numBanks];
    ceList[0] = 0;
    ceList[1] = 0;
    WMR_PRINT(ALWAYS, "Multi-page clean checking mixed CE %d Page %d & %d\n", (UInt32) ceList[0], pageList[0], pageList[1]);
    andStatus = pFIL->ReadScatteredPages(ceList, pageList, verifyDataBuffer, verifyMetaBuffer, 2, NULL, NULL, FALSE32);

    if (FIL_U_ECC_ERROR != andStatus)
    {
        WMR_PRINT(ALWAYS, "Multi-page clean checking mixed detect failed with %s\n", FILErrorToString(andStatus));
        goto test_failed;
    }

    // Over-program the first page to induce real UECC
    WMR_PRINT(ALWAYS, "Over-programming CE %d Page %d\n", (UInt32) ceList[0], pageList[0]);
    WMR_MEMSET(writeDataBuffer, 0xA5, testInfo.bytesPerPage);
    WMR_MEMSET(writeDataBuffer, 0xA5, testInfo.validBytesPerMeta);
    andStatus = pFIL->Write(ceList[0], pageList[0], writeDataBuffer, writeMetaBuffer, FALSE32);

    if (FIL_SUCCESS != andStatus)
    {
        WMR_PRINT(ALWAYS, "Write failed with %s\n", FILErrorToString(andStatus));
        goto test_failed;
    }

    // Read back and verify UECC
    WMR_PRINT(ALWAYS, "Reading back expecting UECC on CE %d Page %d\n", (UInt32) ceList[0], pageList[0]);
    WMR_MEMSET(verifyDataBuffer, TEST_DATA_WATERMARK, testInfo.bytesPerPage);
    WMR_MEMSET(verifyMetaBuffer, TEST_META_WATERMARK, testInfo.bufferBytesPerMeta);
    andStatus = pFIL->ReadWithECC(ceList[0], pageList[0], writeDataBuffer, writeMetaBuffer, NULL, NULL, FALSE32);

    if (FIL_U_ECC_ERROR != andStatus)
    {
        WMR_PRINT(ALWAYS, "Read with expected UECC failed with %s\n", FILErrorToString(andStatus));
        goto test_failed;
    }

    // Program second page
    WMR_PRINT(ALWAYS, "Writing CE %d Page %d\n", (UInt32) ceList[1], pageList[1]);
    FillBufferWithCountingPattern((UInt32*)writeDataBuffer, testSeed++, testInfo.bytesPerPage);
    FillMetaBufferWithPattern(writeMetaBuffer, testSeed++, 1);
    andStatus = pFIL->Write(ceList[1], pageList[1], writeDataBuffer, writeMetaBuffer, FALSE32);

    if (FIL_SUCCESS != andStatus)
    {
        WMR_PRINT(ALWAYS, "Write failed with %s\n", FILErrorToString(andStatus));
        goto test_failed;
    }


    // Read back and verify
    WMR_PRINT(ALWAYS, "Reading back CE %d Page %d\n", (UInt32) ceList[1], pageList[1]);
    WMR_MEMSET(verifyDataBuffer, TEST_DATA_WATERMARK, testInfo.bytesPerPage);
    WMR_MEMSET(verifyMetaBuffer, TEST_META_WATERMARK, testInfo.bufferBytesPerMeta);
    andStatus = pFIL->ReadWithECC(ceList[1], pageList[1], verifyDataBuffer, verifyMetaBuffer, NULL, NULL, FALSE32);

    if (FIL_SUCCESS != andStatus)
    {
        WMR_PRINT(ALWAYS, "Read back failed with %s\n", FILErrorToString(andStatus));
        goto test_failed;
    }

    if (WMR_MEMCMP(writeMetaBuffer, verifyMetaBuffer, testInfo.bufferBytesPerMeta))
    {
        WMR_PRINT(ALWAYS, "Read back contents failed comparison in meta section\n");
        goto test_failed;
    }
        
    if (WMR_MEMCMP(writeDataBuffer, verifyDataBuffer, testInfo.bytesPerPage))
    {
        WMR_PRINT(ALWAYS, "Read back contents failed comparison in data section\n");
        goto test_failed;
    }


    // Read first + second pages and verify UECC status
    // Also make sure that reading did not stop on first UECC
    WMR_PRINT(ALWAYS, "Multi-page clean checking mixed CE %d Page %d & %d\n", (UInt32) ceList[0], pageList[0], pageList[1]);

    WMR_MEMSET(verifyDataBuffer, TEST_DATA_WATERMARK, 2 * testInfo.bytesPerPage);
    WMR_MEMSET(verifyMetaBuffer, TEST_META_WATERMARK, 2 * testInfo.bufferBytesPerMeta);

    andStatus = pFIL->ReadScatteredPages(ceList, pageList, verifyDataBuffer, verifyMetaBuffer, 2, NULL, NULL, FALSE32);

    if (FIL_U_ECC_ERROR != andStatus)
    {
        WMR_PRINT(ALWAYS, "Multi-page uecc checking mixed detect failed with %s\n", FILErrorToString(andStatus));
        goto test_failed;
    }

    // Verify that the read continued after UECC (compare last buffer written versus the second page)

    if (WMR_MEMCMP(writeMetaBuffer, verifyMetaBuffer + testInfo.bufferBytesPerMeta, testInfo.bufferBytesPerMeta))
    {
        WMR_PRINT(ALWAYS, "Read back contents of second page failed comparison in meta section\n");
        goto test_failed;
    }

    if (WMR_MEMCMP(writeDataBuffer, verifyDataBuffer + testInfo.bytesPerPage, testInfo.bytesPerPage))
    {
        WMR_PRINT(ALWAYS, "Read back contents of second page failed comparison in data section\n");
        goto test_failed;
    }

    // Erase block
    WMR_PRINT(ALWAYS, "Erasing CE %d Block %d\n", (UInt32) ceList[0], (UInt32) blockList[0]);
    andStatus = pFIL->Erase(ceList[0], blockList[0]);

    if (FIL_SUCCESS != andStatus)
    {
        WMR_PRINT(ALWAYS, "Erase failed with %s\n", FILErrorToString(andStatus));
        goto test_failed;
    }

    // Read bootloader page and verify clean
    WMR_ASSERT(pFIL->ReadBLPage != NULL);
    WMR_MEMSET(verifyDataBuffer, TEST_DATA_WATERMARK, testInfo.bytesPerPage);
    WMR_PRINT(ALWAYS, "Using ReadBL for clean detect CE %d Page %d\n", (UInt32) ceList[0], pageList[0]);
    andStatus = pFIL->ReadBLPage(ceList[0], pageList[0], verifyDataBuffer);

    if (FIL_SUCCESS_CLEAN != andStatus)
    {
        WMR_PRINT(ALWAYS, "ReadBL Clean Detect failed with %s\n", FILErrorToString(andStatus));
        goto test_failed;
    }

    // Write bootloader page
    WMR_ASSERT(pFIL->WriteBLPage != NULL);
    WMR_PRINT(ALWAYS, "Writing CE %d Page %d\n", (UInt32) ceList[0], pageList[0]);
    FillBufferWithCountingPattern((UInt32*)writeDataBuffer, testSeed++, testInfo.bootPageBytes);
    andStatus = pFIL->WriteBLPage(ceList[0], pageList[0], writeDataBuffer);

    if (FIL_SUCCESS != andStatus)
    {
        WMR_PRINT(ALWAYS, "Write failed with %s\n", FILErrorToString(andStatus));
        goto test_failed;
    }

    // Read back and verify
    WMR_MEMSET(verifyDataBuffer, TEST_DATA_WATERMARK, testInfo.bootPageBytes);
    WMR_PRINT(ALWAYS, "ReadBL CE %d Page %d\n", (UInt32) ceList[0], pageList[0]);
   
    andStatus = pFIL->ReadBLPage(ceList[0], pageList[0], verifyDataBuffer);

    if (FIL_SUCCESS != andStatus)
    {
        WMR_PRINT(ALWAYS, "ReadBL Clean Detect failed with %s\n", FILErrorToString(andStatus));
        goto test_failed;
    }

    if (WMR_MEMCMP(writeDataBuffer, verifyDataBuffer, testInfo.bootPageBytes))
    {
        WMR_PRINT(ALWAYS, "Read back contents of boot page failed comparison\n");
        goto test_failed;
    }

    // Over-program the same page
    WMR_PRINT(ALWAYS, "Over-program BL page CE %d Page %d\n", (UInt32) ceList[0], pageList[0]);
    WMR_MEMSET(writeDataBuffer, 0xA5, testInfo.bootPageBytes);
    andStatus = pFIL->WriteBLPage(ceList[0], pageList[0], writeDataBuffer);

    if (FIL_SUCCESS != andStatus)
    {
        WMR_PRINT(ALWAYS, "Write failed with %s\n", FILErrorToString(andStatus));
        goto test_failed;
    }

    // Read back and verify UECC
    WMR_MEMSET(verifyDataBuffer, TEST_DATA_WATERMARK, testInfo.bootPageBytes);
    WMR_PRINT(ALWAYS, "ReadBL CE %d Page %d\n", (UInt32) ceList[0], pageList[0]);
    andStatus = pFIL->ReadBLPage(ceList[0], pageList[0], verifyDataBuffer);

    if (FIL_U_ECC_ERROR != andStatus)
    {
        WMR_PRINT(ALWAYS, "ReadBL did not detect UECC with %s\n", FILErrorToString(andStatus));
        goto test_failed;
    }

    // XXX Write the sections for multiple using VS (need to find paired blocks)

    if (testInfo.numBanks == testInfo.numCE)
    {
        UInt16 wFailingCE = ~0;
        UInt32 wFailingBlock = ~0;

        // Setup up ceList for scattered ops
        for (bankIdx = 0; bankIdx < testInfo.numCE; ++bankIdx)
        {
            ceList[bankIdx] = bankIdx;
            pageList[bankIdx] = blockList[bankIdx] * testInfo.pagesPerBlock; //XXX this won't work for block gaps
        }

        // Erase multiple blocks

        if (pFIL->EraseMultiple)
        {
            WMR_PRINT(ALWAYS, "EraseMultiple on %d blocks\n", testInfo.numCE);
            andStatus = pFIL->EraseMultiple(blockList, TRUE32, TRUE32, &wFailingCE, &wFailingBlock);
        }
        else
        {
            // only works for 1 bank per ce
            WMR_PRINT(ALWAYS, "EraseMultiple fallback on %d blocks\n", testInfo.numCE);
            for (bankIdx = 0; bankIdx < testInfo.numCE; ++bankIdx)
            {
                andStatus = pFIL->Erase(bankIdx, blockList[bankIdx]);
                if (FIL_SUCCESS != andStatus)
                {
                    wFailingCE = bankIdx;
                    wFailingBlock = blockList[bankIdx];
                    break;
                }
            }
        }

        if (FIL_SUCCESS != andStatus)
        {
            WMR_PRINT(ALWAYS, "Erase failed CE %d Block %d with %s\n", wFailingCE, wFailingBlock, FILErrorToString(andStatus));
            goto test_failed;
        }

        // Clean check
        WMR_MEMSET(verifyDataBuffer, TEST_DATA_WATERMARK, testInfo.numCE * testInfo.bytesPerPage);
        WMR_MEMSET(verifyMetaBuffer, TEST_META_WATERMARK, testInfo.numCE * testInfo.bufferBytesPerMeta);
        WMR_MEMSET(sectorStats, TEST_STATS_WATERMARK, testInfo.numCE * testInfo.correctableSectorsPerPage);
        // Only check the first page of each block
        WMR_PRINT(ALWAYS, "ReadScatteredPages clean check on %d pages\n", testInfo.numCE);
        andStatus = pFIL->ReadScatteredPages(ceList, pageList, verifyDataBuffer, verifyMetaBuffer, testInfo.numCE, NULL, sectorStats, FALSE32);

        if (FIL_SUCCESS_CLEAN != andStatus)
        {
            WMR_PRINT(ALWAYS, "Readback clean check failed with %s\n", FILErrorToString(andStatus));
            WMR_PRINT(ALWAYS, "Sector Stats:\n");
            FILTestHexdump(sectorStats, testInfo.numCE * testInfo.correctableSectorsPerPage);
            goto test_failed;
        }

        // Program scattered pages


        WMR_ASSERT(pFIL->WriteScatteredPages != NULL);
        FillBufferWithCountingPattern((UInt32*)writeDataBuffer, testSeed++, testInfo.numCE * testInfo.bytesPerPage);
        FillMetaBufferWithPattern(writeMetaBuffer, testSeed++, testInfo.numCE);        
        WMR_PRINT(ALWAYS, "WriteScatteredPages on %d pages\n", testInfo.numCE);
        andStatus = pFIL->WriteScatteredPages(ceList, pageList, writeDataBuffer, writeMetaBuffer, testInfo.numCE, &wFailingCE, FALSE32, NULL);

        if (FIL_SUCCESS != andStatus)
        {
            WMR_PRINT(ALWAYS, "WriteScatteredPages failed CE %d with %s\n", wFailingCE, FILErrorToString(andStatus));
            goto test_failed;
        }
        
        // Read back multiple pages

        WMR_MEMSET(verifyDataBuffer, TEST_DATA_WATERMARK, testInfo.numCE * testInfo.bytesPerPage);
        WMR_MEMSET(verifyMetaBuffer, TEST_META_WATERMARK, testInfo.numCE * testInfo.bufferBytesPerMeta);
        WMR_MEMSET(sectorStats, TEST_STATS_WATERMARK, testInfo.numCE * testInfo.correctableSectorsPerPage);
        WMR_PRINT(ALWAYS, "ReadScatteredPages verify on %d pages\n", testInfo.numCE);
        andStatus = pFIL->ReadScatteredPages(ceList, pageList, verifyDataBuffer, verifyMetaBuffer, testInfo.numCE, NULL, sectorStats, FALSE32);

        if (FIL_SUCCESS != andStatus)
        {
            WMR_PRINT(ALWAYS, "ReadScatteredPages failed with %s\n", FILErrorToString(andStatus));
            WMR_PRINT(ALWAYS, "Sector Stats:\n");
            FILTestHexdump(sectorStats, testInfo.numCE * testInfo.correctableSectorsPerPage);
            goto test_failed;
        }

        if (WMR_MEMCMP(writeMetaBuffer, verifyMetaBuffer, testInfo.numCE * testInfo.bufferBytesPerMeta))
        {
            WMR_PRINT(ALWAYS, "Read back contents of pages failed comparison in meta section\n");
            goto test_failed;
        }
    
        if (WMR_MEMCMP(writeDataBuffer, verifyDataBuffer, testInfo.numCE * testInfo.bytesPerPage))
        {
            WMR_PRINT(ALWAYS, "Read back contents of second page failed comparison in data section\n");
            goto test_failed;
        }

        if (pFIL->EraseMultiple)
        {
            WMR_PRINT(ALWAYS, "EraseMultiple on %d blocks\n", testInfo.numCE);
            andStatus = pFIL->EraseMultiple(blockList, TRUE32, TRUE32, &wFailingCE, &wFailingBlock);
        }
        else
        {
            // only works for 1 bank per ce
            WMR_PRINT(ALWAYS, "EraseMultiple fallback on %d blocks\n", testInfo.numCE);
            for (bankIdx = 0; bankIdx < testInfo.numCE; ++bankIdx)
            {
                andStatus = pFIL->Erase(bankIdx, blockList[bankIdx]);
                if (FIL_SUCCESS != andStatus)
                {
                    wFailingCE = bankIdx;
                    wFailingBlock = blockList[bankIdx];
                    break;
                }
            }
        }

        if (FIL_SUCCESS != andStatus)
        {
            WMR_PRINT(ALWAYS, "Erase failed CE %d Block %d with %s\n", wFailingCE, wFailingBlock, FILErrorToString(andStatus));
            goto test_failed;
        }

        // Clean check
        WMR_MEMSET(verifyDataBuffer, TEST_DATA_WATERMARK, testInfo.numCE * testInfo.bytesPerPage);
        WMR_MEMSET(verifyMetaBuffer, TEST_META_WATERMARK, testInfo.numCE * testInfo.bufferBytesPerMeta);
        WMR_MEMSET(sectorStats, TEST_STATS_WATERMARK, testInfo.numCE * testInfo.correctableSectorsPerPage);

#if (!AND_DISABLE_READ_MULTIPLE)

        // Only check the first page of each block
        WMR_PRINT(ALWAYS, "ReadMultiplePages clean check on %d pages\n", testInfo.numCE);
        andStatus = pFIL->ReadMultiplePages(pageList, verifyDataBuffer, verifyMetaBuffer, testInfo.numCE, NULL, sectorStats, FALSE32);

        if (FIL_SUCCESS_CLEAN != andStatus)
        {
            WMR_PRINT(ALWAYS, "Readback clean check failed with %s\n", FILErrorToString(andStatus));
            WMR_PRINT(ALWAYS, "Sector Stats:\n");
            FILTestHexdump(sectorStats, testInfo.numCE * testInfo.correctableSectorsPerPage);
            goto test_failed;
        }

#endif /* if (!AND_DISABLE_READ_MULTIPLE) */

        // Program multiple pages

        FillBufferWithCountingPattern((UInt32*)writeDataBuffer, testSeed++, testInfo.numCE * testInfo.bytesPerPage);
        FillMetaBufferWithPattern(writeMetaBuffer, testSeed++, testInfo.numCE);
        WMR_PRINT(ALWAYS, "WriteMultiplePages on %d pages\n", testInfo.numCE);
        andStatus = pFIL->WriteMultiplePages(pageList, writeDataBuffer, writeMetaBuffer, testInfo.numCE, FALSE32, FALSE32, &wFailingCE, FALSE32, NULL);

        if (FIL_SUCCESS != andStatus)
        {
            WMR_PRINT(ALWAYS, "WriteMultiplePages failed CE %d with %s\n", wFailingCE, FILErrorToString(andStatus));
            goto test_failed;
        }
        
        // Read back multiple pages

        WMR_MEMSET(verifyDataBuffer, TEST_DATA_WATERMARK, testInfo.numCE * testInfo.bytesPerPage);
        WMR_MEMSET(verifyMetaBuffer, TEST_META_WATERMARK, testInfo.numCE * testInfo.bufferBytesPerMeta);
        WMR_MEMSET(sectorStats, TEST_STATS_WATERMARK, testInfo.numCE * testInfo.correctableSectorsPerPage);

#if (!AND_DISABLE_READ_MULTIPLE)

        WMR_PRINT(ALWAYS, "ReadMultiplePages verify on %d pages\n", testInfo.numCE);
        andStatus = pFIL->ReadMultiplePages(pageList, verifyDataBuffer, verifyMetaBuffer, testInfo.numCE, NULL, sectorStats, FALSE32);

        if (FIL_SUCCESS != andStatus)
        {
            WMR_PRINT(ALWAYS, "ReadMultiplePages failed with %s\n", FILErrorToString(andStatus));
            WMR_PRINT(ALWAYS, "Sector Stats:\n");
            FILTestHexdump(sectorStats, testInfo.numCE * testInfo.correctableSectorsPerPage);
            goto test_failed;
        }

#else /* if (!AND_DISABLE_READ_MULTIPLE) */

        WMR_PRINT(ALWAYS, "ReadScatteredPages verify on %d pages\n", testInfo.numCE);
        andStatus = pFIL->ReadScatteredPages(ceList, pageList, verifyDataBuffer, verifyMetaBuffer, testInfo.numCE, NULL, sectorStats, FALSE32);

        if (FIL_SUCCESS != andStatus)
        {
            WMR_PRINT(ALWAYS, "ReadScatteredPages failed with %s\n", FILErrorToString(andStatus));
            WMR_PRINT(ALWAYS, "Sector Stats:\n");
            FILTestHexdump(sectorStats, testInfo.numCE * testInfo.correctableSectorsPerPage);
            goto test_failed;
        }

#endif /* if (!AND_DISABLE_READ_MULTIPLE) */

        if (WMR_MEMCMP(writeMetaBuffer, verifyMetaBuffer, testInfo.numCE * testInfo.bufferBytesPerMeta))
        {
            WMR_PRINT(ALWAYS, "Read back contents of pages failed comparison in meta section\n");
            goto test_failed;
        }
    
        if (WMR_MEMCMP(writeDataBuffer, verifyDataBuffer, testInfo.numCE * testInfo.bytesPerPage))
        {
            WMR_PRINT(ALWAYS, "Read back contents of second page failed comparison in data section\n");
            goto test_failed;
        }


    }
    else
    {
        WMR_PRINT(ALWAYS, "Skipping Multi-Page test on VS formatted part\n");
    }

    (void) testSeed;
    
    WMR_PRINT(ALWAYS, "Interface Test Passed\n");
    
    WMR_BufZone_Free(&bufZone);
    
    return TRUE32;

test_failed:
    WMR_PRINT(ALWAYS, "Interface Test Failed\n");
    
    if (writeMetaBuffer)
    {
        WMR_FREE(writeMetaBuffer, metaBufferSize);
    }
    if (verifyMetaBuffer)
    {
        WMR_FREE(verifyMetaBuffer, metaBufferSize);
    }
    if (sectorStats)
    {
        WMR_FREE(sectorStats, sectorStatsSize);
    }
    
    WMR_BufZone_Free(&bufZone);

    return FALSE32;
}


const char * FILErrorToString(Int32 filStatus)
{

    switch (filStatus)
    {
        case FIL_SUCCESS:
        {
            return "FIL_SUCCESS";
        }
        case FIL_SUCCESS_CLEAN:
        {
            return "FIL_SUCCESS_CLEAN";
        }
        case FIL_CRITICAL_ERROR:
        {
            return "FIL_CRITICAL_ERROR";
        }
        case FIL_U_ECC_ERROR:
        {
            return "FIL_U_ECC_ERROR";
        }
        case FIL_WRITE_FAIL_ERROR:
        {
            return "FIL_WRITE_FAIL_ERROR";
        }
        case FIL_UNSUPPORTED_ERROR:
        {
            return "FIL_UNSUPPORTED_ERROR";
        }
        default:
        {
            return "UNKNOWN";
        }
    }
}

static void InitPageListFromBlocks(const UInt16 *blockList, UInt32 *pageList)
{
    UInt32 pageIdx;

    // Fill in page list based on provided blocks
    // XXX Mind the block gap
    for (pageIdx = 0; pageIdx < (2 * testInfo.numBanks); ++pageIdx)
    {
        pageList[pageIdx] = (blockList[pageIdx % testInfo.numBanks] * testInfo.pagesPerBlock) + (pageIdx / testInfo.numBanks);
    }
}

static void FillBufferWithCountingPattern(UInt32 *buffer, UInt32 startValue, UInt32 bytes)
{
    UInt32 wordIdx;
    const UInt32 kWordsToFill = bytes / sizeof(UInt32);

    for (wordIdx = 0; wordIdx < kWordsToFill; wordIdx += 1)
    {
        if ((wordIdx + 1) == kWordsToFill)
        {
            // Last word may not be full
            UInt8 *endOfBuffer = (UInt8*) &buffer[wordIdx];
            switch (bytes % sizeof(UInt32))
            {
                case 0:
                    endOfBuffer[1] = (startValue >> 24) & 0xFF;
                    // fall through
                case 3:
                    endOfBuffer[1] = (startValue >> 16) & 0xFF;
                    // fall through
                case 2:
                    endOfBuffer[1] = (startValue >> 8) & 0xFF;
                    // fall through
                case 1:
                    endOfBuffer[1] = startValue & 0xFF;
            }
        }
        else
        {
            buffer[wordIdx] = startValue++;
        }
    }
}


static void FillMetaBufferWithPattern(UInt8 *buffer, UInt32 startValue, UInt32 numPages)
{
    typedef struct
    {
        UInt32 valid1;
        UInt32 valid2;
        UInt16 valid3;
        UInt16 pad;
    } _TestMeta;

    _TestMeta *pTestMeta = (_TestMeta*) buffer;
    
    while (numPages--)
    {
        pTestMeta->valid1 = startValue++;
        pTestMeta->valid2 = startValue++;
        pTestMeta->valid3 = startValue++;
        pTestMeta->pad = ~0;

        pTestMeta++;
    }
}

#define HEXDUMP_WIDTH (16)
static void FILTestHexdump(void *data, UInt32 length)
{
	UInt32 i, j;
    UInt8 *bytes = (UInt8*) data;
	for (i = 0; i < length; i += HEXDUMP_WIDTH)
    {
		for (j = 0; (j < HEXDUMP_WIDTH) && ((i + j) < length); j++)
        {
			_WMR_PRINT(" %02x", (UInt32) *bytes++);
        }
		_WMR_PRINT("\n");
	}
}

