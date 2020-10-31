// Copyright (C) 2010 Apple Inc. All rights reserved.
//
// This document is the property of Apple Inc.
// It is considered confidential and proprietary.
//
// This document may not be reproduced or transmitted in any form,
// in whole or in part, without the express written permission of
// Apple Inc.
//
//
//
//

#include "WMROAM.h"
#include "WMRBuf.h"
#include "ANDTypes.h"
#include "WMRTest.h"
#include "FTL.h"
#include "PPN_FIL.h"
#include "PPNMiscTypes.h"
#include "WMRConfig.h"

////////////////////////////////////////////////////////////////////////////////
// Local types
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Defines
////////////////////////////////////////////////////////////////////////////////
#define AND_MAX_BANKS (AND_MAX_BANKS_PER_CS * 4)
#define AND_MAX_CHANNELS         (2)
#define TEST_DATA_WATERMARK      (0xB6)
#define TEST_META_WATERMARK      (0xC7)
#define TEST_STATS_WATERMARK     (0xD8)

#define TEST_MAX_PAGES_PER_OP    (2)
#define TEST_DUMP_ON_MISCOMPARE  (0)

#define PPN_COMMAND_PROGRAM      (2UL)
#define PPN_COMMAND_ERASE        (3UL)


////////////////////////////////////////////////////////////////////////////////
// Static function declarations
////////////////////////////////////////////////////////////////////////////////

static BOOL32 FILInterfaceTest(UInt16 *blockList);
static BOOL32 InitTestInfo(void);
static void FillBufferWithCountingPattern(UInt8 *buffer, UInt32 startValue, UInt32 bytes);
#if TEST_DUMP_ON_MISCOMPARE
static void FILTestHexdump(void *data, UInt32 length);
#endif // TEST_DUMP_ON_MISCOMPARE

////////////////////////////////////////////////////////////////////////////////
// Static variables
typedef struct
{
    UInt32 pagesPerBlock;
    UInt32 numCE;
    UInt32 bootPageBytes;
    UInt32 correctableSectorsPerPage;
} FILTestInfo;

static FILTestInfo      testInfo;
static PPN_DeviceInfo   devInfo;
static LowFuncTbl       *pFIL = NULL;
static WMR_BufZone_t    bufZone;


////////////////////////////////////////////////////////////////////////////////
// Public functions
////////////////////////////////////////////////////////////////////////////////

// Entry point
BOOL32
WMR_FIL_Test
    (LowFuncTbl *fil)
{
    pFIL = fil;
    //return TRUE32;
    return FIL_Test();
}


BOOL32 FIL_Test()
{
    UInt16 blockList[AND_MAX_BANKS];
    WMR_MEMSET(blockList, 0, sizeof(UInt16) * AND_MAX_BANKS);
    WMR_BufZone_Init(&bufZone);
    InitTestInfo();
    ppnMiscFillDevStruct(&devInfo, pFIL);
    return FILInterfaceTest(blockList);
}

// Local functions

BOOL32 InitTestInfo(void)
{
    testInfo.pagesPerBlock = pFIL->GetDeviceInfo(AND_DEVINFO_PAGES_PER_BLOCK);
    WMR_ASSERT(testInfo.pagesPerBlock > 0);
    testInfo.numCE = pFIL->GetDeviceInfo(AND_DEVINFO_NUM_OF_CES_PER_CHANNEL) * pFIL->GetDeviceInfo(AND_DEVINFO_NUM_OF_CHANNELS);
    WMR_ASSERT(testInfo.numCE > 0);
    
    testInfo.bootPageBytes = pFIL->GetDeviceInfo(AND_DEVINFO_BYTES_PER_BL_PAGE);
    WMR_ASSERT(testInfo.bootPageBytes > 0);

    return TRUE32;
}




// Block list is assumed to be numBanks long and non-overlapping banks
BOOL32 isClean(UInt8 *buf, UInt32 size)
{
    UInt32 i;
    for (i=0; i<size; i++)
    {
        if (buf[i] != 0xFF)
        {
            return FALSE32;
        }
    }
    return TRUE32;
}

BOOL32 FILInterfaceTest(UInt16 *blockList)
{
    BOOL32 ret = FALSE32;

    UInt8  *writeDataBuffer = NULL;
    UInt8  *writeMetaBuffer = NULL;
    UInt8  *verifyDataBuffer = NULL;
    UInt8  *verifyMetaBuffer = NULL;
    UInt32 testSeed = 1234567890;
    const UInt32 dataPerPage = devInfo.main_bytes_per_lba * devInfo.lbas_per_page;
    const UInt32 metaPerPage = devInfo.lba_meta_bytes_buffer * devInfo.lbas_per_page;
    const UInt32 dataBufferSize = devInfo.num_of_banks * dataPerPage * TEST_MAX_PAGES_PER_OP;
    const UInt32 metaBufferSize = devInfo.num_of_banks * metaPerPage * TEST_MAX_PAGES_PER_OP;
    PPNCommandStruct *ppnCmd[AND_MAX_CHANNELS];
    PPNReorderStruct *ppnReorder;
    UInt32 bank = 0;
    UInt32 page = 0;
    UInt32 channel, lba;
    
    WMR_PRINT(ALWAYS, "Starting FIL Interface Test\n");
    writeMetaBuffer  = WMR_Buf_Alloc_ForDMA(&bufZone, metaBufferSize);
    verifyMetaBuffer = WMR_Buf_Alloc_ForDMA(&bufZone, metaBufferSize);
    writeDataBuffer  = WMR_Buf_Alloc_ForDMA(&bufZone, dataBufferSize);
    verifyDataBuffer = WMR_Buf_Alloc_ForDMA(&bufZone, dataBufferSize);
    for (channel = 0 ; channel < devInfo.num_channels ; channel++)
    {
        ppnCmd[channel] = WMR_Buf_Alloc_ForDMA(&bufZone, sizeof(PPNCommandStruct));
    }
    WMR_BufZone_FinishedAllocs(&bufZone);
    WMR_BufZone_Rebase(&bufZone, (void **)&writeMetaBuffer);
    WMR_BufZone_Rebase(&bufZone, (void **)&verifyMetaBuffer);    
    WMR_BufZone_Rebase(&bufZone, (void **)&writeDataBuffer);
    WMR_BufZone_Rebase(&bufZone, (void **)&verifyDataBuffer);    
    for (channel = 0 ; channel < devInfo.num_channels ; channel++)
    {
        WMR_BufZone_Rebase(&bufZone, (void **)&ppnCmd[channel]);
    }
    
    if (!writeDataBuffer || !writeMetaBuffer || !verifyDataBuffer || !verifyMetaBuffer)
    {
        WMR_PANIC("Buffer allocations failed");
        goto exit;
    }

    WMR_BufZone_FinishedRebases(&bufZone);

    ppnReorder = WMR_MALLOC(sizeof(PPNReorderStruct));

    ppnMiscInitCommandStructure(&devInfo, ppnCmd, devInfo.num_channels, PPN_COMMAND_ERASE, PPN_NO_OPTIONS);
    // Erase Block
    WMR_PRINT(ALWAYS, "Erasing CAU %d Block %d\n", bank, (UInt32) blockList[0]);
    ppnMiscSingleOperation(&devInfo, ppnCmd[0], PPN_COMMAND_ERASE, PPN_NO_OPTIONS, bank, blockList[0], page, FALSE32, NULL, NULL);
    if (ppnCmd[0]->page_status_summary != 0x40)
    {
        WMR_PRINT(ALWAYS, "Erase failed with summary status = 0x%x\n", ppnCmd[0]->page_status_summary);
        goto exit;
    }

    // Clean check single page
    WMR_PRINT(ALWAYS, "Clean checking CAU %d Block %d Page %d\n", bank, blockList[0], page);
    WMR_MEMSET(verifyDataBuffer, TEST_DATA_WATERMARK, dataPerPage);
    WMR_MEMSET(verifyMetaBuffer, TEST_META_WATERMARK, metaPerPage);
    ppnMiscInitCommandStructure(&devInfo, ppnCmd, devInfo.num_channels, PPN_COMMAND_READ, PPN_NO_OPTIONS);
    for (lba = 0 ; lba < devInfo.lbas_per_page ; lba++)
    {
        ppnMiscAddPhysicalAddressToCommandStructure(&devInfo, ppnCmd,
            ppnMiscGetChannelFromBank(&devInfo, bank),
            ppnMiscGetCEIdxFromBank(&devInfo, bank),
            ppnMiscGetCAUFromBank(&devInfo, bank),
            blockList[0], 0, FALSE32, devInfo.lbas_per_page - 1 - lba, 1, lba, 0);
        ppnMiscAddMemoryToCommandStructure(&devInfo, ppnCmd, devInfo.num_channels,
            &verifyDataBuffer[lba * devInfo.main_bytes_per_lba],
            &verifyMetaBuffer[lba * devInfo.lba_meta_bytes_buffer],
            1);
    }
    WMR_MEMSET(verifyDataBuffer, TEST_DATA_WATERMARK, dataBufferSize);
    WMR_MEMSET(verifyMetaBuffer, TEST_META_WATERMARK, metaBufferSize);
    ppnMiscReorderCommandStruct(&devInfo, ppnCmd, devInfo.num_channels, ppnReorder);
    pFIL->PerformCommandList(ppnCmd, 1);
    for (lba = 0 ; lba < devInfo.lbas_per_page ; lba++)
    {
        WMR_PRINT(ALWAYS, "ppnCmd[0]->entry[%d].status = 0x%08x\n", lba, ppnCmd[0]->entry[lba].status);
    }
    if (ppnCmd[0]->page_status_summary != 0x49)
    {
        WMR_PRINT(ALWAYS, "Clean check single page failed with summary status = 0x%x\n", ppnCmd[0]->page_status_summary);
        goto exit;
    }
    if (isClean(verifyDataBuffer, dataPerPage) == FALSE32)
    {
        WMR_PRINT(ALWAYS, "Clean Detect failed: Main!\n");
        //goto exit;
    }
    if (isClean(verifyMetaBuffer, metaPerPage) == FALSE32)
    {
        WMR_PRINT(ALWAYS, "Clean Detect failed: Meta!\n");
        //goto exit;
    }
   
    // Program single page
    WMR_PRINT(ALWAYS, "Writing single page CAU %d Block %d Page %d\n", bank, blockList[0], page);
    FillBufferWithCountingPattern(writeDataBuffer, testSeed++, dataPerPage);
    FillBufferWithCountingPattern(writeMetaBuffer, testSeed++, metaPerPage);
    ppnMiscSingleOperation(&devInfo, ppnCmd[0], PPN_COMMAND_PROGRAM, PPN_NO_OPTIONS, bank, blockList[0], page, FALSE32, writeDataBuffer, writeMetaBuffer);
    if (ppnCmd[0]->page_status_summary != 0x62)
    {
        WMR_PRINT(ALWAYS, "Program single page failed with summary status = 0x%x\n", ppnCmd[0]->page_status_summary);
        goto exit;
    }

    // Read back and verify
    WMR_PRINT(ALWAYS, "Reading back single page CAU %d Block %d Page %d\n", bank, blockList[0], page);
    WMR_MEMSET(verifyDataBuffer, TEST_DATA_WATERMARK, dataPerPage);
    WMR_MEMSET(verifyMetaBuffer, TEST_META_WATERMARK, metaPerPage);
    ppnMiscSingleOperation(&devInfo, ppnCmd[0], PPN_COMMAND_READ, PPN_NO_OPTIONS, bank, blockList[0], page, FALSE32, verifyDataBuffer, verifyMetaBuffer);
    if (ppnCmd[0]->page_status_summary != 0x40)
    {
        WMR_PRINT(ALWAYS, "Read back single page failed with summary status = 0x%x\n", ppnCmd[0]->page_status_summary);
        goto exit;
    }
    if (WMR_MEMCMP(writeMetaBuffer, verifyMetaBuffer, metaPerPage))
    {
        WMR_PRINT(ALWAYS, "Read back contents failed comparison in meta section\n");
#if TEST_DUMP_ON_MISCOMPARE
        WMR_PRINT(ALWAYS, "Expected:\n");
        FILTestHexdump(writeMetaBuffer, metaPerPage);
        WMR_PRINT(ALWAYS, "Actual:\n");
        FILTestHexdump(verifyMetaBuffer, metaPerPage);
#endif // TEST_DUMP_ON_MISCOMPARE
        goto exit;
    }
    if (WMR_MEMCMP(writeDataBuffer, verifyDataBuffer, dataPerPage))
    {
        WMR_PRINT(ALWAYS, "Read back contents failed comparison in data section\n");
#if TEST_DUMP_ON_MISCOMPARE
        WMR_PRINT(ALWAYS, "Expected:\n");
        FILTestHexdump(writeDataBuffer, dataPerPage);
        WMR_PRINT(ALWAYS, "Actual:\n");
        FILTestHexdump(verifyDataBuffer, dataPerPage);
#endif // TEST_DUMP_ON_MISCOMPARE
        goto exit;
    }

    // MultiBlockErase (Block from all bank)
    WMR_PRINT(ALWAYS, "Erasing Block %d from all banks\n", blockList[0]);
    ppnMiscInitCommandStructure(&devInfo, ppnCmd, devInfo.num_channels, PPN_COMMAND_ERASE, PPN_NO_OPTIONS);
    for (bank=0; bank<devInfo.num_of_banks; bank++)
    {
        ppnMiscAddPhysicalAddressToCommandStructure(&devInfo, ppnCmd,
            ppnMiscGetChannelFromBank(&devInfo, bank),
            ppnMiscGetCEIdxFromBank(&devInfo, bank),
            ppnMiscGetCAUFromBank(&devInfo, bank),
            blockList[0], 0, FALSE32, 0, 0, bank, 0);
        ppnMiscAddMemoryToCommandStructure(&devInfo, ppnCmd, devInfo.num_channels,
            NULL,
            NULL,
            devInfo.lbas_per_page);
    }
    ppnMiscReorderCommandStruct(&devInfo, ppnCmd, devInfo.num_channels, ppnReorder);
    pFIL->PerformCommandList(ppnCmd, devInfo.num_channels);
    for (channel = 0 ; channel < devInfo.num_channels ; channel++)
    {
        if (ppnCmd[channel]->page_status_summary != 0x40)
        {
            WMR_PRINT(ALWAYS, "MultiBlockErase failed with summary status = 0x%x\n", ppnCmd[channel]->page_status_summary);
            goto exit;
        }
    }

    // MultiPageProgram (Program 2 pages on each bank)
    WMR_PRINT(ALWAYS, "Multipage Programming %d pages of Block %d of each bank\n", TEST_MAX_PAGES_PER_OP, blockList[0]);
    ppnMiscInitCommandStructure(&devInfo, ppnCmd, devInfo.num_channels, PPN_COMMAND_PROGRAM, PPN_NO_OPTIONS);
    for (bank=0; bank<devInfo.num_of_banks; bank++)
    {
        for (page = 0 ; TEST_MAX_PAGES_PER_OP > page ; page++)
        {
            ppnMiscAddPhysicalAddressToCommandStructure(&devInfo, ppnCmd,
                ppnMiscGetChannelFromBank(&devInfo, bank),
                ppnMiscGetCEIdxFromBank(&devInfo, bank),
                ppnMiscGetCAUFromBank(&devInfo, bank),
                blockList[0], page, FALSE32, 0, devInfo.lbas_per_page, bank*2+page, 0);
            ppnMiscAddMemoryToCommandStructure(&devInfo, ppnCmd, devInfo.num_channels,
                &writeDataBuffer[(bank*2+page) * dataPerPage],
                &writeMetaBuffer[(bank*2+page) * metaPerPage],
                devInfo.lbas_per_page);
        }
    }
    FillBufferWithCountingPattern(writeDataBuffer, testSeed++, dataBufferSize);
    FillBufferWithCountingPattern(writeMetaBuffer, testSeed++, metaBufferSize);
    ppnMiscReorderCommandStruct(&devInfo, ppnCmd, devInfo.num_channels, ppnReorder);
    pFIL->PerformCommandList(ppnCmd, devInfo.num_channels);
    for (channel = 0 ; channel < devInfo.num_channels ; channel++)
    {
        if (ppnCmd[channel]->page_status_summary != 0x62)
        {
            WMR_PRINT(ALWAYS, "Multipage Program failed with summary status = 0x%x\n", ppnCmd[channel]->page_status_summary);
            goto exit;
        }
    }

    // MultiPageRead (Read 2 pages on each bank)
    WMR_PRINT(ALWAYS, "Multipage Read of %d pages of Block %d of each bank\n", TEST_MAX_PAGES_PER_OP, blockList[0]);
    ppnMiscInitCommandStructure(&devInfo, ppnCmd, devInfo.num_channels, PPN_COMMAND_READ, PPN_NO_OPTIONS);
    for (bank=0; bank<devInfo.num_of_banks; bank++)
    {
        for (page = 0 ; TEST_MAX_PAGES_PER_OP > page ; page++)
        {
            ppnMiscAddPhysicalAddressToCommandStructure(&devInfo, ppnCmd,
                ppnMiscGetChannelFromBank(&devInfo, bank),
                ppnMiscGetCEIdxFromBank(&devInfo, bank),
                ppnMiscGetCAUFromBank(&devInfo, bank),
                blockList[0], page, FALSE32, 0, devInfo.lbas_per_page, bank*2+page, 0);
            ppnMiscAddMemoryToCommandStructure(&devInfo, ppnCmd, devInfo.num_channels,
                &verifyDataBuffer[(bank*2+page) * dataPerPage],
                &verifyMetaBuffer[(bank*2+page) * metaPerPage],
                devInfo.lbas_per_page);
        }
    }
    WMR_MEMSET(verifyDataBuffer, TEST_DATA_WATERMARK, dataBufferSize);
    WMR_MEMSET(verifyMetaBuffer, TEST_META_WATERMARK, metaBufferSize);
    ppnMiscReorderCommandStruct(&devInfo, ppnCmd, devInfo.num_channels, ppnReorder);
    pFIL->PerformCommandList(ppnCmd, devInfo.num_channels);
    for (channel = 0 ; channel < devInfo.num_channels ; channel++)
    {
        if (ppnCmd[channel]->page_status_summary != 0x40)
        {
            WMR_PRINT(ALWAYS, "Multipage Read failed with summary status = 0x%x\n", ppnCmd[channel]->page_status_summary);
            goto exit;
        }
    }
    if (WMR_MEMCMP(writeMetaBuffer, verifyMetaBuffer, metaBufferSize))
    {
        WMR_PRINT(ALWAYS, "Read back contents failed comparison in meta section\n");
#if TEST_DUMP_ON_MISCOMPARE
        WMR_PRINT(ALWAYS, "Expected:\n");
        FILTestHexdump(writeMetaBuffer, metaBufferSize);
        WMR_PRINT(ALWAYS, "Actual:\n");
        FILTestHexdump(verifyMetaBuffer, metaBufferSize);
#endif // TEST_DUMP_ON_MISCOMPARE
        goto exit;
    }
    if (WMR_MEMCMP(writeDataBuffer, verifyDataBuffer, dataBufferSize))
    {
        WMR_PRINT(ALWAYS, "Read back contents failed comparison in data section\n");
#if TEST_DUMP_ON_MISCOMPARE
        WMR_PRINT(ALWAYS, "Expected:\n");
        FILTestHexdump(writeDataBuffer, dataBufferSize);
        WMR_PRINT(ALWAYS, "Actual:\n");
        FILTestHexdump(verifyDataBuffer, dataBufferSize);
#endif // TEST_DUMP_ON_MISCOMPARE
        goto exit;
    }

    ret = TRUE32;

exit:
    WMR_PRINT(ALWAYS, "Interface Test %s\n", ret ? "Passed" : "Failed");
    
    if (ppnReorder)
    {
        WMR_FREE(ppnReorder, sizeof(PPNReorderStruct));
    }
    
    WMR_BufZone_Free(&bufZone);

    return ret;
}

static void FillBufferWithCountingPattern(UInt8 *buffer, UInt32 value, UInt32 bytes)
{
    UInt32 *word;

    if (0 != bytes % sizeof(*word))
    {
        const UInt32 extra = bytes % sizeof(*word);

        bytes -= extra;
        WMR_MEMCPY(&buffer[bytes], &value, extra);
        value++;
    }

    while (0 < bytes)
    {
        bytes -= sizeof(*word);
        word = (UInt32 *)&buffer[bytes];
        *word = value++;
    }
}


#if TEST_DUMP_ON_MISCOMPARE
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
#endif // TEST_DUMP_ON_MISCOMPARE

