/*
 * Copyright (c) 2010 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */
#include <debug.h>
#include <drivers/flash_nand.h>
#include <sys/menu.h>
#include <lib/env.h>
#include <platform/memmap.h>
#include <platform/soc/hwclocks.h>

#include "FIL.h"
#include "ANDTypes.h"
#include "WMRConfig.h"
#include "H2fmi.h"
#include "PPNMiscTypes.h"
#include "WMROAM.h"

#if !RELEASE_BUILD && WITH_MENU

#define WITH_NAND_DEBUG (0)
#define MAX_NAND_SIZE_KB (67108864) //(64GB)

#if WITH_PPN
extern BOOL32 WMR_PPN_CtrlIO(UInt32 dwCtlrIOType, void * pvoidDataBuffer, UInt32 * pdwDataSize);
#endif
#if WITH_RAW_NAND
extern BOOL32 WMR_CtrlIO(UInt32 dwCtlrIOType, void * pvoidDataBuffer, UInt32 * pdwDataSize);
#endif // WITH_RAW_NAND

static BOOL32 ctrl_io(UInt32 dwCtlrIOType, void * pvoidDataBuffer, UInt32 * pdwDataSize)
{
#if WITH_PPN
    LowFuncTbl *fil = FIL_GetFuncTbl();
    if (fil->GetDeviceInfo(AND_DEVINFO_PPN_DEVICE) > 0)
    {
        if (!WMR_PPN_CtrlIO(dwCtlrIOType, pvoidDataBuffer, pdwDataSize))
        {
           return FALSE32;
        }
    }
    else
#endif
    {
#if WITH_RAW_NAND
        if (!WMR_CtrlIO(dwCtlrIOType, pvoidDataBuffer, pdwDataSize))
#endif // WITH_RAW_NAND
        {
            return FALSE32;
        }
    }
    
    return TRUE32;
}

// raw_read
//
// Read the specifed number of full (main + spare) nand pages to the given
// buffer. They are concatenated with no padding, so a part with an unaligned 
// number of spare bytes, multiple pages requests will not have aligned
// main data
//
static int raw_read(UInt32 ce, UInt32 base_page, UInt32 num_pages, void *buffer)
{
    LowFuncTbl *fil = FIL_GetFuncTbl();
    const UInt32 main_size = fil->GetDeviceInfo(AND_DEVINFO_BYTES_PER_PAGE);
    const UInt32 spare_size = fil->GetDeviceInfo(AND_DEVINFO_BYTES_PER_SPARE);
    const UInt32 bytes_per_read = main_size + spare_size;
    UInt8 *cursor = buffer;
    int err = 0;
    UInt32 page_offset;
    UInt32 nand_status;
    UInt8 *bounce;

    if (!security_allow_memory(buffer, num_pages * bytes_per_read))
    {
        printf("Permission Denied\n");
        return -1;
    }
    

    printf("raw_read ce %d base page %d (0x%08x) #pages %d\n", ce, base_page, base_page, num_pages);
    
    if (NULL == fil->ReadNoECC)
    {
        printf("ReadNoECC not available\n");
        return -1;
    }
    
    bounce = malloc(main_size + spare_size);

    for (page_offset = 0; page_offset < num_pages; ++page_offset)
    {
        UInt32 current_page = base_page + page_offset;
        void *spare = bounce + main_size; // put the spare immediately after main
        memset(bounce, 0xB6, bytes_per_read);
        if (FIL_SUCCESS != (nand_status = fil->ReadNoECC(ce, current_page, bounce, spare)))
        {
            printf("error 0x%08x reading ce %d page %d\n", nand_status, ce, current_page);
            err = -1;
        }
        memcpy(cursor, bounce, bytes_per_read);
        cursor += bytes_per_read;
    }
    
    free(bounce);
    printf("%d bytes read to %p\n", cursor - (UInt8*)buffer, buffer);
    return err;
}

#if WITH_PPN
static int cbp2r(UInt16 cau, UInt32 block, UInt32 page_offset)
{
    UInt32 row_addr = 0;
    PPN_DeviceInfo* dev = malloc(sizeof(PPN_DeviceInfo));
    LowFuncTbl *fil = FIL_GetFuncTbl();

    ppnMiscFillDevStruct(dev, fil);
    row_addr = ppnMiscConvertToPPNPageAddress(dev, cau, block, page_offset, FALSE32);
    printf("cau: %d, block: %d, page_offset: %d corresponds to row addr: 0x%X\n", cau, block, page_offset, row_addr);
    free(dev);
    return 0;
}

static int r2cbp(UInt32 row_addr)
{
    UInt16 cau = 0;
    UInt16 block = 0;
    UInt16 page_offset = 0;
    PPN_DeviceInfo* dev = malloc(sizeof(PPN_DeviceInfo));
    LowFuncTbl *fil = FIL_GetFuncTbl();

    ppnMiscFillDevStruct(dev, fil);
    ppnMiscConvertPhysicalAddressToCauBlockPage(dev, row_addr, &cau, &block, &page_offset);
    printf("row_addr: 0x%X corresponds to cau: %d, block: %d, page_offset: %d\n",row_addr, cau, block, page_offset);
    free(dev);    
    return 0;
}

static int read4k(UInt32 ce, UInt32 num_lbas, UInt8 lba_offset, UInt16 cau, UInt16 block, UInt16 base_page, void *buffer)
{
    LowFuncTbl *fil = FIL_GetFuncTbl();
    PPNCommandStruct* ppn_command; 
    PPN_DeviceInfo* dev = malloc(sizeof(PPN_DeviceInfo));
    PPNReorderStruct* reorder = malloc(sizeof(PPNReorderStruct));
    const UInt32 main_size = (fil->GetDeviceInfo(AND_DEVINFO_BYTES_PER_PAGE)) / (fil->GetDeviceInfo(AND_DEVINFO_FIL_LBAS_PER_PAGE));
    const UInt32 meta_size = fil->GetDeviceInfo(AND_DEVINFO_FIL_META_BUFFER_BYTES);
    const UInt32 bytes_per_read = main_size + meta_size;
    PPNStatusType status;
    int err = 0;
    UInt32 lba_idx;
    UInt8 *bounce;
    UInt8 *cursor = buffer;
    UInt8 current_lba_offset = lba_offset;
    UInt16 current_page = base_page;
    UInt16 allowed_lbas;
        
    ppnMiscFillDevStruct(dev, fil);
    allowed_lbas = ((dev->pages_per_block_mlc - base_page) * (dev->lbas_per_page)) - lba_offset;
    if(num_lbas > allowed_lbas)
    {
        printf("Reading lbas from only one block is allowed (No more than %d lbas with a page_offset of %d and lba_offset of %d)\n", allowed_lbas, base_page, lba_offset);
        printf("pages per blk: %d\n", dev->pages_per_block_mlc);
        free(dev);
        free(reorder);
        return -1;
    }

    if (!security_allow_memory(buffer, num_lbas * bytes_per_read))
    {
        free(dev);
        free(reorder);
        printf("Permission Denied\n");
        return -1;
    }

    ppn_command = malloc((sizeof(PPNCommandStruct)));
    bounce = malloc(bytes_per_read);
   
    printf("reading %d 4k LBA's starting from CE: %d, cau: %d, Block: %d, Page offset: %d,  lba_offset: %d\n", num_lbas, ce, cau, block, base_page, lba_offset);
    for(lba_idx = 0; lba_idx < num_lbas; lba_idx++)
    {
        void *meta = bounce + main_size;
        memset(bounce, 0xC7, bytes_per_read);
        memset(ppn_command, 0xB6, (sizeof(PPNCommandStruct)));
        status = ppnMiscSingleLbaRead(dev, ppn_command, PPN_COMMAND_READ, PPN_NO_OPTIONS, ce, cau, block, current_page, current_lba_offset, FALSE32, bounce, meta);
        if(status != PPN_READ_STATUS_VALID)
        {
            printf("error status: 0x%X reading lba at page offset: %d, lba offset: %d\n", status, current_page, current_lba_offset);
            err = -1;
        }
        memcpy(cursor, bounce, bytes_per_read);
        cursor += bytes_per_read;
        if((dev->lbas_per_page - current_lba_offset) == 1)
        {
            current_page ++;
            current_lba_offset = 0;
        }
        else
        {
            current_lba_offset++;
        }
    }
       
    free(bounce);
    free(ppn_command);
    free(dev);
    free(reorder);
    return err;
}
#endif // WITH_PPN

// read_with_ecc
//
// Read the specifed number of main + meta nand pages to the given
// buffer. They are concatenated with no padding, so a part with an unaligned 
// number of meta bytes (default today is 12), multiple pages requests will not
//  have aligned main data
//
static int read_with_ecc(UInt32 ce, UInt32 base_page, UInt32 num_pages, BOOL32 stats, BOOL32 disable_whitening, void *buffer)
{
    LowFuncTbl *fil = FIL_GetFuncTbl();
    const UInt32 main_size = fil->GetDeviceInfo(AND_DEVINFO_BYTES_PER_PAGE);
    const UInt32 meta_size = fil->GetDeviceInfo(AND_DEVINFO_FIL_LBAS_PER_PAGE) * fil->GetDeviceInfo(AND_DEVINFO_FIL_META_BUFFER_BYTES);
    const UInt32 bytes_per_read = main_size + meta_size;
    const UInt32 sectors_per_page = main_size / fil->GetDeviceInfo(AND_DEVINFO_CORRECTABLE_SIZE);
    UInt8 *cursor = buffer;
    int err = 0;
    UInt32 page_offset;
    UInt32 nand_status;
    UInt8 *bounce;
    UInt8 *buf_stats = NULL;
    UInt8 *buf_stats_cursor = NULL;
    UInt32 idx;
    bool zero_flips = true;

    if (!security_allow_memory(buffer, num_pages * bytes_per_read))
    {
        printf("Permission Denied\n");
        return -1;
    }

    printf("read ce %d base page %d (0x%08x) #pages %d\n", ce, base_page, base_page, num_pages);
    
    if (NULL == fil->ReadWithECC)
    {
        printf("ReadWithECC not available\n");
        return -1;
    }

    bounce = malloc(main_size + meta_size);
    if(stats)
    {
        buf_stats = malloc(sectors_per_page);
        memset(buf_stats, 0x00, sectors_per_page);
        printf("sector stats: \n");
    }

    for (page_offset = 0; page_offset < num_pages; ++page_offset)
    {
        UInt32 current_page = base_page + page_offset;
        void *meta = bounce + main_size; // put the meta immediately after main
        memset(bounce, 0xC7, bytes_per_read);

        if (FIL_SUCCESS != (nand_status = fil->ReadWithECC(ce, current_page, bounce, meta, NULL, buf_stats, disable_whitening)))
        {
            printf("error 0x%08x reading ce %d page %d\n", nand_status, ce, current_page);
            err = -1;
        }
        memcpy(cursor, bounce, bytes_per_read);
        cursor += bytes_per_read;

        if(stats)
        {
            buf_stats_cursor = buf_stats;
            zero_flips = true;
            for(idx = 0; idx < sectors_per_page; idx++)
            {
                if(*buf_stats_cursor != 0)
                {
                    zero_flips = false;
                    break;
                }
                buf_stats_cursor++;
            }
            if(!zero_flips)
            {
                buf_stats_cursor = buf_stats;
                printf("ce %d page %d: ", ce, (base_page + page_offset));
                for(idx = 0; idx < sectors_per_page; idx++)
                {
                    printf("%02X ", *buf_stats_cursor);
                    buf_stats_cursor++;
                }
                printf("\n");
            }
        }
    }

    free(bounce);
    printf("%d bytes read to %p\n", cursor - (UInt8*)buffer, buffer);
    return err;
}

// read_bl_pages
//
// Read the specifed number of bootloader-style nand pages to the given
// buffer. Multiple pages are concatenated with no padding.
//
static int read_bl_pages(UInt32 ce, UInt32 base_page, UInt32 num_pages, void *buffer)
{
    LowFuncTbl *fil = FIL_GetFuncTbl();
    const UInt32 bl_size = fil->GetDeviceInfo(AND_DEVINFO_BYTES_PER_BL_PAGE);
    UInt8 *cursor = buffer;
    int err = 0;
    UInt32 page_offset;
    UInt32 nand_status;
    
    if (!security_allow_memory(buffer, num_pages * bl_size))
    {
        printf("Permission Denied\n");
        return -1;
    }

    printf("readboot ce %d base page %d (0x%08x) #pages %d\n", ce, base_page, base_page, num_pages);
    
    if (NULL == fil->ReadBLPage)
    {
        printf("ReadBL not available\n");
        return -1;
    }

    for (page_offset = 0; page_offset < num_pages; ++page_offset)
    {
        UInt32 current_page = base_page + page_offset;
        memset(cursor, 0xD8, bl_size);
        if (FIL_SUCCESS != (nand_status = fil->ReadBLPage(ce, current_page, cursor)))
        {
            printf("error 0x%08x reading ce %d page %d\n", nand_status, ce, current_page);
            err = -1;
        }
        cursor += bl_size;
    }
    
    printf("%d bytes read to %p\n", cursor - (UInt8*)buffer, buffer);
    return err;
}

static int inspect_single_block(UInt32 ce, UInt32 block)
{
    Int32 nand_status;
    int num_clean = 0;
    int num_good_boot_pages = 0;
    int num_good_full_pages = 0;
    int num_uecc = 0;
    int num_out_of_order = 0;
    
    void *page_buffer;
    void *meta_buffer;
    UInt32 page_offset;
    LowFuncTbl *fil = FIL_GetFuncTbl();
    const UInt32 slcBits = fil->GetDeviceInfo(AND_DEVINFO_ADDR_BITS_BITS_PER_CELL);
    const UInt32 cauBits = fil->GetDeviceInfo(AND_DEVINFO_BITS_PER_CAU_ADDRESS);
    const UInt32 blockBits = fil->GetDeviceInfo(AND_DEVINFO_BITS_PER_BLOCK_ADDRESS);
    const UInt32 slc_mask = ~((~0) << slcBits) << (cauBits + blockBits);
    const UInt32 pages_per_block = (slc_mask & block)?
        fil->GetDeviceInfo(AND_DEVINFO_SLC_PAGES_PER_BLOCK):
        fil->GetDeviceInfo(AND_DEVINFO_PAGES_PER_BLOCK);

    if (NULL == fil->ReadWithECC)
    {
        printf("ReadWithECC is not available\n");
        return -1;
    }
    
    if (NULL == fil->ReadBLPage)
    {
        printf("ReadBLPage is not available\n");
        return -1;
    }
    
    page_buffer = malloc(fil->GetDeviceInfo(AND_DEVINFO_BYTES_PER_PAGE) + fil->GetDeviceInfo(AND_DEVINFO_FIL_META_BUFFER_BYTES));
    meta_buffer = (UInt8*)page_buffer + fil->GetDeviceInfo(AND_DEVINFO_BYTES_PER_PAGE);
    
    
    for (page_offset = 0; page_offset < pages_per_block; ++page_offset)
    {
        const UInt32 current_page = (block * pages_per_block) + page_offset;

        nand_status = fil->ReadBLPage(ce, current_page, page_buffer);
        if (FIL_SUCCESS == nand_status)
        {
            num_good_boot_pages++;;
            if (num_clean > 0)
            {
                if (0 == num_out_of_order)
                {
                    // only print once
                    printf("ERROR: Found OK boot-page after clean page on ce %d page %d(0x%08x)\n",
                            ce, current_page, current_page);
                }
                num_out_of_order++;
            }
        }
        else if (FIL_SUCCESS_CLEAN == nand_status)
        {
            num_clean++;
        }
        else
        {
            // see if it's a full-page before we call it bad
            if (FIL_SUCCESS == (nand_status = fil->ReadWithECC(ce, current_page, page_buffer, meta_buffer, NULL, NULL, FALSE32)))
            {   
                num_good_full_pages++;
                if (num_clean > 0)
                {
                    // only print once
                    if (0 == num_out_of_order)
                    {
                        printf("ERROR: Found OK full-page after clean page on ce %d page %d(0x%08x)\n",
                                ce, current_page, current_page);
                    }
                    num_out_of_order++;
                }
            }
            else
            {
                num_uecc++;
                printf("ERROR: Found UECC at ce %d page %d(0x%08x)\n",
                        ce, current_page, current_page);
            }
        }
    }

    if (num_out_of_order > 0)
    {
        printf("Error on ce %d block %d - %d pages programmed out of order\n",
                ce, block, num_out_of_order);
    }
    
    printf("ce %d block %4d - boot-pages: %4d full-pages: %4d clean: %4d UECC: %4d\n", 
            ce, block, num_good_boot_pages, num_good_full_pages, num_clean, num_uecc);


    free(page_buffer);
    
    return ((num_out_of_order > 0) || (num_uecc > 0)) ? -1 : 0;
}

// inspect_blocks
//
// Verify that this block has not been programmed incorrectly
// Checks for uecc and clean pages out of order
//
static int inspect_blocks(UInt32 ce, UInt32 start_block, UInt32 num_blocks)
{
    UInt32 block, last_ce;
    int overall = 0;
    
    LowFuncTbl *fil = FIL_GetFuncTbl();

    last_ce = fil->GetDeviceInfo(AND_DEVINFO_NUM_OF_CS) - 1;

    if (ce > last_ce)
    {
        printf("ERROR: invalid ce (max is %d)\n", last_ce);
        return -1;
    }

    for(block = 0; block < num_blocks; ++block)
    {
        overall |= inspect_single_block(ce, start_block + block);
    }
    
    return overall;
}

static int nand_info(void)
{
    NandTimingParams timing;
    UInt32 data_size;
    UInt32 ns_per_clock;
    UInt32 clk_mhz;
    UInt32 is_toggle;
    int err = 0;
    
    LowFuncTbl *fil = FIL_GetFuncTbl();
    
    
    printf("PPN: %s\n", fil->GetDeviceInfo(AND_DEVINFO_PPN_DEVICE) ? "Yes" : "No");
    printf("Toggle: %s\n", (is_toggle = fil->GetDeviceInfo(AND_DEVINFO_TOGGLE_DEVICE)) ? "Yes" : "No");

    printf("Page Size: %d\n", fil->GetDeviceInfo(AND_DEVINFO_BYTES_PER_PAGE));
    printf("Spare Size: %d\n", fil->GetDeviceInfo(AND_DEVINFO_BYTES_PER_SPARE));
    printf("Pages Per Block: %d\n", fil->GetDeviceInfo(AND_DEVINFO_PAGES_PER_BLOCK));
    printf("Blocks Per CE: %d\n", fil->GetDeviceInfo(AND_DEVINFO_BLOCKS_PER_CS));
    printf("Blocks Per CAU: %d\n", fil->GetDeviceInfo(AND_DEVINFO_BLOCKS_PER_CAU));
    printf("CAU's per CE: %d\n", fil->GetDeviceInfo(AND_DEVINFO_CAUS_PER_CE));
    
    printf("VS Type: 0x%08x\n", fil->GetDeviceInfo(AND_DEVINFO_VENDOR_SPECIFIC_TYPE));
    printf("Banks Per CE: %d\n", fil->GetDeviceInfo(AND_DEVINFO_BANKS_PER_CS));

    printf("Refresh: %d\n", fil->GetDeviceInfo(AND_DEVINFO_REFRESH_THRESHOLD));
    printf("Correctable Bits: %d\n", fil->GetDeviceInfo(AND_DEVINFO_CORRECTABLE_BITS));
    printf("Correctable Sector: %d\n", fil->GetDeviceInfo(AND_DEVINFO_CORRECTABLE_SIZE));
    
    printf("Active Channels: %d\n", fil->GetDeviceInfo(AND_DEVINFO_NUM_OF_CHANNELS));
    printf("CE Per Channel: %d\n", fil->GetDeviceInfo(AND_DEVINFO_NUM_OF_CES_PER_CHANNEL));
    
    printf("Bootloader Page: %d\n", fil->GetDeviceInfo(AND_DEVINFO_BYTES_PER_BL_PAGE));

    printf("Bits per page address: %d\n", fil->GetDeviceInfo(AND_DEVINFO_BITS_PER_PAGE_ADDRESS));
    printf("Bits per block address: %d\n", fil->GetDeviceInfo(AND_DEVINFO_BITS_PER_BLOCK_ADDRESS));
    printf("Bits per cau address: %d\n", fil->GetDeviceInfo(AND_DEVINFO_BITS_PER_CAU_ADDRESS));
    printf("Address bits for bits per cell: %d\n", fil->GetDeviceInfo(AND_DEVINFO_ADDR_BITS_BITS_PER_CELL));

    // Avoid 64-bit math, we're rounding anyway
#ifdef CLK_FMI
    clk_mhz = clock_get_frequency(CLK_FMI) / 1000000;
#else
    clk_mhz = clock_get_frequency(CLK_HCLK) / 1000000;
#endif
    ns_per_clock = 1000 / clk_mhz;
    
    
    memset(&timing, 0xff, sizeof(timing));
    data_size = sizeof(timing);
    if (FIL_GetStruct(AND_STRUCT_FIL_GET_TIMINGS, &timing, &data_size))
    {
        if (is_toggle)
        {
            printf("Read/Write Cycle Time: %dns\n", ((timing.ddr_tHALFCYCLE + 1) * 1000 * 2) / clk_mhz);
        }
        else
        {
            printf("Read Timing: %dns setup %dns hold %d ns strobe\n", 
                    (timing.sdrTimings.tRP + 1) * ns_per_clock,
                    (timing.sdrTimings.tREH + 1) * ns_per_clock,
                    timing.sdrTimings.DCCYCLE * ns_per_clock);
            printf("Write Timing: %dns setup %dns hold\n",
                    (timing.sdrTimings.tWP + 1) * ns_per_clock,
                    (timing.sdrTimings.tWH + 1) * ns_per_clock);
        }
    }
    else
    {
        err = -1;
    }
   
    return err;
}

#define IS_BLOCK_GOOD(bbt, block) ((bbt)[(block) >> 3] & (1 << ((block) & 0x07)))

// Caller frees the buffer
static UInt8 * allocate_bbt()
{
    LowFuncTbl *fil = FIL_GetFuncTbl();
    const UInt32 total_ce = fil->GetDeviceInfo(AND_DEVINFO_NUM_OF_CS);
    const UInt32 blocks_per_ce = fil->GetDeviceInfo(AND_DEVINFO_BLOCKS_PER_CS);
    const UInt32 bbt_size_per_ce = (blocks_per_ce / 8) + (blocks_per_ce % 8 ? 1 : 0); // bit per block
    const UInt32 expected_bbt_size = total_ce * bbt_size_per_ce;
    UInt32 data_size = expected_bbt_size;
    void * buffer = malloc(data_size);

#if WITH_PPN
    const UInt32 main_size = fil->GetDeviceInfo(AND_DEVINFO_BYTES_PER_PAGE);
    const UInt32 spare_size = fil->GetDeviceInfo(AND_DEVINFO_BYTES_PER_SPARE);
    const UInt32 bytes_per_read = main_size + spare_size;

    UInt8 *bounce = NULL;
    UInt8 *pData = NULL;
    PPNCommandStruct *ppn_command = NULL;
    PPN_DeviceInfo *dev = NULL;
    PPNStatusType ppn_status;
    UInt32 ce;
    UInt32 cau;
    UInt32 length;

    if (fil->GetDeviceInfo(AND_DEVINFO_PPN_DEVICE) > 0)
    {
        const UInt32 caus_per_ce = fil->GetDeviceInfo(AND_DEVINFO_CAUS_PER_CE);
        const UInt32 blocks_per_cau = fil->GetDeviceInfo(AND_DEVINFO_BLOCKS_PER_CAU);
        const UInt32 cau_bbt_size = (blocks_per_cau / 8) + (blocks_per_cau % 8 ? 1 : 0);
        const UInt32 total_caus = caus_per_ce * total_ce;
        const UInt32 required_buffer_size = cau_bbt_size * total_caus;

        data_size = required_buffer_size;
        bounce = malloc(bytes_per_read);
        ppn_command = malloc((sizeof(PPNCommandStruct)));
        dev = malloc(sizeof(PPN_DeviceInfo));
        pData = (UInt8 *) buffer;
        ppnMiscFillDevStruct(dev, fil);

        for (ce = 0; ce < total_ce; ce++)
        {
            for (cau = 0; cau < caus_per_ce; cau++)
            {
                UInt16 bank = (cau * total_ce) + ce;
                memset(bounce, 0xC7, bytes_per_read);
                memset(ppn_command, 0xB6, (sizeof(PPNCommandStruct)));
                ppn_status = ppnMiscSingleOperation(dev, ppn_command, PPN_COMMAND_CAUBBT,
                                                    PPN_NO_OPTIONS, bank,
                                                    0, 0, TRUE32, bounce, NULL);
                if(ppn_status != PPN_READ_STATUS_VALID){
                    printf("error pulling BBT, status: 0x%x\n", ppn_status);
                    goto error;
                }
                WMR_MEMCPY(pData, bounce, cau_bbt_size);

                // Consumers expect opposite bit-order
                for (length = cau_bbt_size; length > 0; length--)
                {
                    UInt8 byte = *pData;
                    *pData = ((byte & 0x01) << 7) |
                             ((byte & 0x02) << 5) |
                             ((byte & 0x04) << 3) |
                             ((byte & 0x08) << 1) |
                             ((byte & 0x10) >> 1) |
                             ((byte & 0x20) >> 3) |
                             ((byte & 0x40) >> 5) |
                             ((byte & 0x80) >> 7);
                    pData++;
                }
            }
        }
    }
    else
#endif
    {
#if WITH_RAW_NAND
        if (!WMR_CtrlIO(AND_STRUCT_VFL_FACTORY_BBT, buffer, &data_size))
#endif // WITH_RAW_NAND
        {
            goto error;
        }
    }
    
    if (data_size != expected_bbt_size)
    {
        printf("error in BBT size. got %d expected %d\n", 
                expected_bbt_size, data_size);
        goto error;
    }

    return buffer;
error:
    if (NULL != buffer)
    {
        free(buffer);
        buffer = NULL;
    }
#if WITH_PPN
    if (NULL != bounce)
    {
        free(bounce);
        bounce = NULL;
    }
    if (NULL != ppn_command)
    {
        free(ppn_command);
        ppn_command = NULL;
    }
    if (NULL != dev)
    {
        free(dev);
        dev = NULL;
    }
#endif

    return buffer;
}

static int print_raw_bbt()
{
    int err = 0;
    LowFuncTbl *fil = FIL_GetFuncTbl();
    const UInt32 num_ce = fil->GetDeviceInfo(AND_DEVINFO_NUM_OF_CS);
    const UInt32 blocks_per_ce = fil->GetDeviceInfo(AND_DEVINFO_BLOCKS_PER_CS);
    const UInt32 bbt_size_per_ce = (blocks_per_ce / 8) + (blocks_per_ce % 8 ? 1 : 0); // bit per block
    UInt32 ce, block;
    void * buffer = allocate_bbt();
    
    if (NULL == buffer)
    {
        err = -1;
    }
    else
    {
        UInt8 *cursor = buffer;
        for (ce = 0; ce < num_ce; ++ce)
        {
            for(block = 0; block < blocks_per_ce; ++block)
            {
                if (!IS_BLOCK_GOOD(cursor, block))
                {
                    printf("CE %d Block %d\n", ce, block);
                }
            }
            cursor += bbt_size_per_ce;
        }
        
    }

    free(buffer);
    
    return err;
}

#if WITH_PPN
static int print_ppn_bbt()
{
    int err = 0;
    LowFuncTbl *fil = FIL_GetFuncTbl();
    const UInt32 num_ce = fil->GetDeviceInfo(AND_DEVINFO_NUM_OF_CS);
    const UInt32 caus_per_ce = fil->GetDeviceInfo(AND_DEVINFO_CAUS_PER_CE);
    const UInt32 blocks_per_cau = fil->GetDeviceInfo(AND_DEVINFO_BLOCKS_PER_CAU);
    UInt32 ce;
    UInt32 block;
    UInt32 cau;
    UInt32 cau_block;
    UInt8  *buffer = allocate_bbt();

    if (!buffer)
    {
        return -1;
    }

    block = 0;
    for (cau = 0; cau < caus_per_ce; cau++)
    {
        for (ce = 0; ce < num_ce; ce++)
        {
            for (cau_block = 0; cau_block < blocks_per_cau; cau_block++, block++)
            {
                if (!IS_BLOCK_GOOD(buffer, block))
                {
                    printf("CE %d CAU %d Block 0x%08x bad\n", ce, cau, cau_block);
                }
            }
        }
    }
    free(buffer);
    return err;
}
#endif

//get_physical_ce
//gets the physical ce given a channel and a ce index within the channel

#if WITH_PPN
static int get_physical_ce()
{
    LowFuncTbl *fil = FIL_GetFuncTbl();
    PPN_DeviceInfo* dev = malloc(sizeof(PPN_DeviceInfo));;
    UInt16 channel_idx, cau_idx, ce_idx, bank, phy_ce;
    UInt32 idx = 0;

    ppnMiscFillDevStruct(dev, fil);

    for(channel_idx = 0; channel_idx < dev->num_channels; channel_idx++)
    {
        for(ce_idx = 0; ce_idx < dev->ces_per_channel; ce_idx++)
        {
            for(cau_idx = 0; cau_idx < dev->caus_per_ce; cau_idx++)
            {
                bank = ((ce_idx * dev->num_channels * dev->caus_per_ce) +
                                 (cau_idx * dev->num_channels) + channel_idx);

                phy_ce = ppnMiscGetCEFromBank(dev, bank);

                printf("%d: channel: %d, ceIdx: %d, cauIdx: %d corresponds to physical CE: %d\n", idx, channel_idx, ce_idx, cau_idx, phy_ce);
                idx++;
            }
        }
    }
    return 0;
}
#endif

// get_stats
// gets the sector stats for the entire device to $loadaddr. can be obtained with usb put

#if WITH_PPN
static int get_stats(void *buffer)
{
    UInt64 idx;
    UInt8 *cursor = (UInt8 *)buffer;
    void *meta;
    UInt8 *bounce;
    LowFuncTbl *fil = FIL_GetFuncTbl();
    PPN_DeviceInfo* dev;
    UInt16 channel_idx, ce_idx, bank, cau_idx, page_offset_idx, block_idx, sector_idx;
    UInt32 block;
    PPNStatusType block_status;
    PPNStatusType current_status;
    PPNCommandStruct* ppn_command;
    BOOL32 is_SLC;
    const UInt32 main_size = fil->GetDeviceInfo(AND_DEVINFO_BYTES_PER_PAGE);
    const UInt32 spare_size = fil->GetDeviceInfo(AND_DEVINFO_BYTES_PER_SPARE);
    const UInt32 bytes_per_read = main_size + spare_size;
    const UInt32 blocks_per_ce = fil->GetDeviceInfo(AND_DEVINFO_BLOCKS_PER_CS);
    const UInt32 sectors_per_page = main_size / fil->GetDeviceInfo(AND_DEVINFO_CORRECTABLE_SIZE);
    const UInt32 bbt_size_per_ce = (blocks_per_ce / 8) + (blocks_per_ce % sectors_per_page ? 1 : 0); // bit per block
    const UInt32 stats_per_page = sectors_per_page + 1; //1 extra byte for SLC/MLC mode
    const UInt32 max_stats_size = MAX_NAND_SIZE_KB + (MAX_NAND_SIZE_KB/sectors_per_page);
    void * bbt_buffer = allocate_bbt();

    if (!security_allow_memory(buffer, max_stats_size))
    {
        free(bbt_buffer);
        printf("Permission Denied\n");
        return -1;
    }
    memset(buffer, 0x00, max_stats_size);

    dev = malloc(sizeof(PPN_DeviceInfo));
    bounce = malloc(bytes_per_read);
    ppn_command = malloc((sizeof(PPNCommandStruct)));

    ppnMiscFillDevStruct(dev, fil);

    idx = 0;
    block_status = 0;
    is_SLC = FALSE32; //MLC by default
    for(channel_idx = 0; channel_idx < dev->num_channels; channel_idx++)
    {
        for(ce_idx = 0; ce_idx < dev->ces_per_channel; ce_idx++)
        {
            for(cau_idx = 0; cau_idx < dev->caus_per_ce; cau_idx++)
            {
                for(block_idx = 0; block_idx < dev->blocks_per_cau; block_idx++)
                {
                    //check for bad blocks
                    block = block_idx + (cau_idx * dev->blocks_per_cau);
                    if(!IS_BLOCK_GOOD(((UInt8 *) bbt_buffer + (bbt_size_per_ce * ((channel_idx * dev->ces_per_channel) + ce_idx))), block))
                    {
                        for(page_offset_idx = 0; page_offset_idx < dev->pages_per_block_mlc; page_offset_idx++)
                        {
                            idx = stats_per_page * 
                                  ((page_offset_idx) +
                                   (block_idx * dev->pages_per_block_mlc) +
                                   (cau_idx * dev->blocks_per_cau * dev->pages_per_block_mlc) +
                                   (ce_idx * dev->caus_per_ce * dev->blocks_per_cau * dev->pages_per_block_mlc) +
                                   (channel_idx * dev->ces_per_channel * dev->caus_per_ce * dev->blocks_per_cau * dev->pages_per_block_mlc));
                            cursor[idx] = (UInt8) FALSE32; //MLC default
                            idx++;
                            for(sector_idx = 0; sector_idx < sectors_per_page; sector_idx++)
                            {
                                cursor[idx] = 0xfc;        //bad block marking
                                idx++;
                            }
                        }
                        continue;
                    }

                    //if previous MLC read was for an SLC programmed page, read it in SLC mode
                    if((((block_status & PPN_READ_STATUS_WRONG_BITS_PER_CELL))) && (!is_SLC))
                    {
                        is_SLC = TRUE32;
                        block_idx--;
                    }
                    else
                    {
                        is_SLC = FALSE32;
                    }
                    block_status = 0;
                    for(page_offset_idx = 0; page_offset_idx < dev->pages_per_block_mlc; page_offset_idx++)
                    {
                        bank = ((ce_idx * dev->num_channels * dev->caus_per_ce) +
                                 (cau_idx * dev->num_channels) + channel_idx);

                        meta = bounce + main_size;
                        memset(bounce, 0xC7, bytes_per_read);
                        memset(ppn_command, 0xB6, (sizeof(PPNCommandStruct)));
                        current_status = ppnMiscSingleOperation(dev, ppn_command, PPN_COMMAND_READ, 
                                                        PPN_OPTIONS_REPORT_HEALTH, bank, block_idx, 
                                                        page_offset_idx, is_SLC, 
                                                        bounce, meta);
                        block_status |= current_status;

                        idx = stats_per_page * 
                                  ((page_offset_idx) +
                                   (block_idx * dev->pages_per_block_mlc) +
                                   (cau_idx * dev->blocks_per_cau * dev->pages_per_block_mlc) +
                                   (ce_idx * dev->caus_per_ce * dev->blocks_per_cau * dev->pages_per_block_mlc) +
                                   (channel_idx * dev->ces_per_channel * dev->caus_per_ce * dev->blocks_per_cau * dev->pages_per_block_mlc));
                        cursor[idx] = (UInt8) is_SLC;
                        idx++;

                        for(sector_idx = 0; sector_idx < sectors_per_page; sector_idx++)
                        {
                            if((((current_status & PPN_READ_STATUS_INVALID_DATA) &&(!(current_status & PPN_READ_STATUS_CLEAN)))) && (!is_SLC))
                            {
                                cursor[idx] = 0xff;
                            }
                            else if(current_status == (PPN_READ_STATUS_INVALID_DATA | PPN_READ_STATUS_CLEAN | PPN_READ_STATUS_VALID))
                            {
                                cursor[idx] = 0xfe;
                            }
                            else if(current_status == PPN_READ_STATUS_VALID)
                            {
                                cursor[idx] = bounce[sector_idx];
                            }
                            else
                            {
                                cursor[idx] = 0xfd;
                                printf("Bad read status: 0x%X - channel: %d, ce: %d, cau: %d, blk: %d, pg_offset: %d, is_SLC: %d\n", current_status, channel_idx, ce_idx, cau_idx, block_idx, page_offset_idx, is_SLC);
                            }
                            idx++;
                        }
                    }
                }
            }
        }
    }
    printf("usb put <filename> %llu\n", idx);

    printf("Device parameters:\n");
    printf("Number of channels: %d\n", dev->num_channels);
    printf("CE's per channel: %d\n", dev->ces_per_channel);
    printf("CAU's per CE: %d\n", dev->caus_per_ce);
    printf("Blocks per CAU: %d\n", dev->blocks_per_cau);
    printf("Pages per Block: %d\n", dev->pages_per_block_mlc);

    free(bbt_buffer);
    free(dev);
    free(ppn_command);
    return 0;
}
#endif //#if WITH_PPN

#define LOG2(x) (31UL - __builtin_clz((UInt32)(x)))
static int nuke(bool force)
{
    LowFuncTbl *fil = FIL_GetFuncTbl();
    const UInt32 num_ce = fil->GetDeviceInfo(AND_DEVINFO_NUM_OF_CS);
    const UInt32 blocks_per_ce = fil->GetDeviceInfo(AND_DEVINFO_BLOCKS_PER_CS);
    const UInt32 bbt_size_per_ce = (blocks_per_ce / 8) + (blocks_per_ce % 8 ? 1 : 0); // bit per block
#if WITH_PPN
    const UInt32 die_per_ce = fil->GetDeviceInfo(AND_DEVINFO_CAUS_PER_CE);
    const UInt32 blocks_per_die = fil->GetDeviceInfo(AND_DEVINFO_BLOCKS_PER_CAU);
    const UInt32 die_addr_bits = fil->GetDeviceInfo(AND_DEVINFO_BITS_PER_BLOCK_ADDRESS);
#else // WITH_PPN
    const UInt32 die_per_ce = fil->GetDeviceInfo(AND_DEVINFO_DIES_PER_CS);
    const UInt32 blocks_per_die = fil->GetDeviceInfo(AND_DEVINFO_BLOCKS_PER_DIE);
    const UInt32 die_addr_bits = LOG2(fil->GetDeviceInfo(AND_DEVINFO_DIE_STRIDE));
#endif // !WITH_PPN
    Int32 nand_status;
    UInt8 *bbt;
        
    UInt32 ce, die, block;
    
    if (NULL == fil->Erase)
    {
        printf("erase is not supported\n");
        return -1;
    }
    
    bbt = allocate_bbt();
    if ((NULL == bbt) && !force)
    {
        printf("bbt not found. force?\n");
        return -1;
    }
    
    for (ce = 0; ce < num_ce; ++ce)
    {
        const UInt8 *current_bbt = bbt + (bbt_size_per_ce * ce);
        for (die = 0; die < die_per_ce; ++die)
        {
            const UInt32 bbt_die_offset = die * blocks_per_die;
            for (block = 0; block < blocks_per_die; ++block)
            {
                UInt32 physical_block = (die << die_addr_bits) | block;

                if ((NULL == bbt) || IS_BLOCK_GOOD(current_bbt, bbt_die_offset + block))
                {
                    printf("ce %d block 0x%08x\r", ce, physical_block);
                    if (FIL_SUCCESS != (nand_status = fil->Erase(ce, physical_block)))
                    {
                        printf("erase failed 0x%08x ce %d block 0x%08x\n", 
                            nand_status, ce, physical_block);
                    }
                }
                else
                {
                    printf("skipping ce %d block 0x%08x\n", ce, physical_block);
                }
            }
        }
    }

    free(bbt);
    printf("done with %d blocks\n", (num_ce * die_per_ce * blocks_per_die));
    
    return 0;
}

static int nand_erase(UInt32 ce, UInt32 block, UInt32 num_blocks)
{
    LowFuncTbl *fil = FIL_GetFuncTbl();
    Int32 nand_status;
    int err = 0;
    
    if (NULL == fil->Erase)
    {
        printf("erase is not supported\n");
        return -1;
    }
    
    printf("Erase ce %d block %d to block %d\n", ce, block, block + num_blocks - 1);
    while(num_blocks > 0)
    {
        nand_status = fil->Erase(ce, block);
        if (FIL_SUCCESS != nand_status)
        {
            printf("erase failed 0x%08x ce %d block %d\n", nand_status, ce, block);
        }
        block++;
        num_blocks--;
    }

    return err;
}

static int write(UInt32 ce, UInt32 base_page, UInt32 num_pages, void *buffer, UInt32 delay_ms, UInt32 disable_whitening, BOOL32 pattern_present, UInt32 pattern)
{
    LowFuncTbl *fil = FIL_GetFuncTbl();
    const UInt32 main_size = fil->GetDeviceInfo(AND_DEVINFO_BYTES_PER_PAGE);
    const UInt32 meta_size = fil->GetDeviceInfo(AND_DEVINFO_FIL_META_BUFFER_BYTES);
    const UInt32 bytes_per_write = main_size + meta_size;
    UInt8 *cursor = buffer;
    UInt8 *meta_cursor = cursor + (main_size * num_pages);
    int err = 0;
    UInt32 page_offset;
    UInt32 nand_status;
    UInt32 *data_ptr = NULL;

    if (!security_allow_memory(buffer, num_pages * bytes_per_write))
    {
        printf("Permission Denied\n");
        return -1;
    }
   
    if (NULL == fil->Write)
    {
        printf("Write not available. Set H2FMI_DEBUG and WMR_IBOOT_WRITE_ENABLE to 1.\n");
        return -1;
    }

    if ((num_pages > fil->GetDeviceInfo(AND_DEVINFO_PAGES_PER_BLOCK)) || ((base_page % fil->GetDeviceInfo(AND_DEVINFO_PAGES_PER_BLOCK)) != 0))
    {
        printf("Pages only in one block can be programmed at a time starting from the first page\n");
        return -1;
    }

    if(NULL == buffer)
    {
        cursor = malloc(main_size + meta_size);
        data_ptr = (UInt32 *) cursor;
        //set data pattern
        UInt32 idx;
        meta_cursor = cursor + main_size;
        *((UInt32 *) meta_cursor) = base_page;

        for(idx = 1; idx < (meta_size / sizeof(UInt32)); idx++)
        {
            *((UInt32 *) meta_cursor + idx) = WMR_RAND();
        }

        if(pattern_present)
        {
            printf("writing data pattern : 0x%X\n", pattern);
            for(idx = 0; idx < (main_size / sizeof(UInt32)); idx++)
            {
                data_ptr[idx] = pattern;
            }
        }
        else
        {
            printf("writing random data pattern\n");
            for(idx = 0; idx < (main_size / sizeof(UInt32)); idx++)
            {
                data_ptr[idx] = WMR_RAND() % 0xffffffff;
            }
        }
    }

    if(delay_ms)
    {
        nand_erase(ce, base_page/fil->GetDeviceInfo(AND_DEVINFO_PAGES_PER_BLOCK), 1);
        WMR_SLEEP_US(delay_ms * 1000);
    }

    printf("write ce %d base page %d (0x%08x) #pages %d\n", ce, base_page, base_page, num_pages);

    for (page_offset = 0; page_offset < num_pages; ++page_offset)
    {
        UInt32 current_page = base_page + page_offset;

        if (FIL_SUCCESS != (nand_status = fil->Write(ce, current_page, cursor, meta_cursor, (disable_whitening ? TRUE32 : FALSE32))))
        {
            printf("error 0x%08x writing ce %d page %d\n", nand_status, ce, current_page);
            err = -1;
        }
        if (cursor != (UInt8 *)data_ptr)
        {
            cursor += main_size;
            meta_cursor += meta_size;
        }
        else
        {
            *((UInt32 *) meta_cursor) = current_page + 1;
        }
    }
    if(NULL != data_ptr)
    {
        free(data_ptr);
    }
    return err;
}

static int write_bl_pages(UInt32 ce, UInt32 base_page, void *buffer, size_t len)
{
    LowFuncTbl *fil = FIL_GetFuncTbl();
    Int32 nand_status;
    int err = 0;
    UInt32 page_idx;
    const size_t bl_page_size = fil->GetDeviceInfo(AND_DEVINFO_BYTES_PER_BL_PAGE);
    const UInt32 num_pages = len / bl_page_size;
    UInt8 *cursor = buffer;

    if (NULL == fil->WriteBLPage)
    {
        printf("write boot page is not supported\n");
        return -1;
    }

    if (((len % bl_page_size) != 0) && (num_pages > 0))
    {
        printf("payload must be a multiple of %zu\n", bl_page_size);
        return -1;
    }
    
    if (!security_allow_memory(buffer, num_pages * bl_page_size))
    {
        printf("Permission Denied\n");
        return -1;
    }
    
    printf("Writing Boot Pages to CE %d from %d to %d\n", ce, base_page, base_page + num_pages - 1);
    
    for (page_idx = 0; page_idx < num_pages; ++page_idx)
    {
        const UInt32 current_page = base_page + page_idx;
        nand_status = fil->WriteBLPage(ce, current_page, cursor);
        if (FIL_SUCCESS != nand_status)
        {
            printf("Error 0x%x programming ce %d page %d\n", nand_status, ce, current_page);
            err = -1;
            break;
        }
        cursor += bl_page_size;
    }

    return err;
}

static void usage(void)
{
    LowFuncTbl *fil = FIL_GetFuncTbl(); 
    UInt32 lbasPerPage = fil->GetDeviceInfo(AND_DEVINFO_FIL_LBAS_PER_PAGE); 

    printf("nand read <ce> <base_page> <num_pages> <stats (0/1)> <disable whitening (0/1)>[<load_addr>]\n");
    printf("nand readraw <ce> <base_page> <num_pages> [<load_addr>]\n");
    printf("nand readboot <ce> <base_page> <num_pages> [<load_addr>]\n");
#if WITH_NAND_DEBUG
    printf("nand writeboot <ce> <base_page> [<load_addr> <file_size>]\n");
    printf("nand erase <ce> <block> <num_blocks>\n");
    printf("nand write <ce> <base_page> <num_pages> <delay_ms> <disable whitening (0/1)>[<load_addr>] [32 bit data pattern]\n");
    printf("           (if delay=0, then no erase before program), random pattern used if no pattern specified. load_addr ignored if pattern specified\n");
    printf("           (Pages only in one block can be programmed at a time starting from the first page)\n");
#endif // WITH_NAND_DEBUG
    printf("nand inspect <ce> <base_block> <num_blocks>\n");
    printf("nand bbt\n");
    printf("nand l2p <lba>\n");
    if ( 1 == lbasPerPage) //different usage for swiss vs yaFTL
    {
        printf("nand v2p <vpn>\n");
        printf("nand sbCycles [<vpn>]\n");
    }
    else
    {
        printf("nand sbCycles [<vba>]\n");
        printf("nand v2p <vba>\n");
    }
    printf("nand reset\n");
    printf("nand info\n");
    printf("nand params\n");
    printf("nand getstats\n");
    printf("nand getphyce\n");
    printf("nand r2cbp <row addr> (converts row address to cau, blk, pg offset)\n");
    printf("nand cbp2r <cau> <block> <page offset> (converts to cau, blk, pg offset to row address)\n"); 
    printf("nand read4k <ce> <num_lbas> <lba_offset within page> <cau> <block> <base_page> [<load_addr>]\n");
}

static int do_nand(int argc, struct cmd_arg *args)
{
    void *buffer;
    size_t buffer_len;
    int err = 0;
    const char *cmd;
    
    if (argc < 2)
    {
       usage();
       return 0;
    }
    
    buffer = (void *) env_get_uint("loadaddr", DEFAULT_LOAD_ADDRESS);
    buffer_len = (size_t) env_get_uint("filesize", 0);
    cmd = args[1].str;
    
    if (!strcmp(cmd, "info"))
    {
        (void) buffer;
        err = nand_info();
    }
    else if (!strcmp(cmd, "params"))
    {
        err = h2fmiPrintParameters();
    }
    else if (!strcmp(cmd, "bbt"))
    {
        err = print_raw_bbt();
    }
#if WITH_NAND_DEBUG
    else if(!strcmp(cmd, "nuke"))
    {
        bool force = argc > 2 ? true : false;
        err = nuke(force);
    }
    else if(!strcmp(cmd, "erase"))
    {
        if (argc < 5)
        {
            return -1;
        }
        err = nand_erase(args[2].u, args[3].u, args[4].u);
    }
    else if (!strcmp(cmd, "write"))
    {
        UInt32 delay_ms          = 6 <= argc ? args[5].u        : 0;
        UInt32 disable_whitening = 7 <= argc ? args[6].u        : 0;
        BOOL32 pattern_present   = 9 <= argc ? TRUE32           : FALSE32;
        UInt32 pattern           = 9 <= argc ? args[8].u        : 0;
        buffer                   = 8 == argc ? (void*)args[7].u : NULL;

        if (argc < 5)
        {
            usage();
            return -1;
        }

        err = write(args[2].u, args[3].u, args[4].u, buffer, delay_ms, disable_whitening, pattern_present, pattern);
    }
#endif
    else if (!strcmp(cmd, "inspect"))
    {
        if (argc != 5)
        {
            usage();
            return -1;
        }
        err = inspect_blocks(args[2].u, args[3].u, args[4].u);
    }
    else if (!strcmp(cmd, "readraw"))
    {
        if (argc < 5)
        {
            usage();
            return -1;
        }
        else if (argc > 5)
        {
            buffer = (void*) args[5].u;
        }
        err = raw_read(args[2].u, args[3].u, args[4].u, buffer);
    }
    else if (!strcmp(cmd, "read"))
    {
        if (argc < 7)
        {
            usage();
            return -1;
        }
        else if (argc > 7)
        {
            buffer = (void*) args[7].u;
        }
        err = read_with_ecc(args[2].u, args[3].u, args[4].u, args[5].b, args[6].b, buffer);
    }
#if WITH_PPN
    else if (!strcmp(cmd, "read4k"))
    {
        if (argc < 8)
        {
            usage();
            return -1;
        }
        else if (argc > 8)
        {
            buffer = (void*) args[8].u;
        }
        err = read4k(args[2].u, args[3].u, args[4].u, args[5].u, args[6].u, args[7].u, buffer);
    }
    else if (!strcmp(cmd, "cbp2r"))
    {
        if (argc < 5)
        {
            usage();
            return -1;
        }
        err = cbp2r(args[2].u, args[3].u, args[4].u);
    }
    else if (!strcmp(cmd, "r2cbp"))
    {
        if (argc < 3)
        {
            usage();
            return -1;
        }
        err = r2cbp(args[2].u);
    }
#endif // WITH_PPN
    else if (!strcmp(cmd, "readboot"))
    {
        if (argc < 5)
        {
            usage();
            return -1;
        }
        else if (argc > 5)
        {
            buffer = (void*) args[5].u;
        }
        err = read_bl_pages(args[2].u, args[3].u, args[4].u, buffer);
    }
    else if (!strcmp(cmd, "sbCycles"))
    {
        UInt16 pagesPerSB;
        UInt32 vba,vpn;
        UInt32 sbIndex;
        UInt16 *eraseCntBuf;
        UInt16 listLen;
        UInt32 size;
        bool printSingle = false;
        LowFuncTbl *fil = FIL_GetFuncTbl();
        UInt32 lbasPerPage;
  
        lbasPerPage = fil->GetDeviceInfo(AND_DEVINFO_FIL_LBAS_PER_PAGE); 

        size = sizeof(listLen);
        if( !ctrl_io(AND_STRUCT_VFL_NUM_OF_FTL_SUBLKS, &listLen, &size) ) 
        {
            return -1;
        }

        if (argc > 2)
        {
            size = sizeof(pagesPerSB);
            if (!ctrl_io(AND_STRUCT_VFL_PAGES_PER_SUBLK, &pagesPerSB, &size))
            {
                return -1;
            }

            vba = args[2].u;
            vpn = (lbasPerPage !=0)? vba / lbasPerPage: vba;
            sbIndex = vpn / (UInt32)pagesPerSB;
            if (sbIndex < listLen)
            {
                printSingle = true;
            }
            else
            {
                printf("sbIndex=%d out of bounds in array length %d (%d pages per SB)\n",
                    sbIndex,listLen,pagesPerSB);
            }
        }

        size = sizeof(UInt16)*listLen;
        eraseCntBuf = malloc(size);
        memset(eraseCntBuf, 0xFF,size);
        if ( ctrl_io(AND_STRUCT_FTL_SB_CYCLES, eraseCntBuf, &size) ) 
        {
            if (printSingle)
            {
                printf("vba: 0x%08x vpn: 0x%08x sb: %d cycles: %d\n", vba, vpn, sbIndex, eraseCntBuf[sbIndex]);
            } 
            else
            {
                printf("numSBs: %d\n",listLen);
                for (sbIndex=0; sbIndex < listLen; sbIndex++)
                {
                    printf("sb: %d cycles: %d\n", sbIndex, (UInt32)(eraseCntBuf[sbIndex]));
                }
            }
        }  
        else
        {
			printf("Error: unable to get NAND Cycle Count\n");
            err = -1;
        }

        free(eraseCntBuf);
    }
#if WITH_PPN
    else if (!strcmp(cmd, "getstats"))
    {
        err = get_stats(buffer);
    }
    else if(!strcmp(cmd, "getphyce"))
    {
        err = get_physical_ce();
    }
#endif //WITH_PPN
#if WITH_NAND_DEBUG
    else if (!strcmp(cmd, "writeboot"))
    {
        if ((argc < 3) || (argc == 5))
        {
            usage();
            return -1;
        }
        if (argc > 4)
        {
            buffer = (void*) args[4].u;
            buffer_len = (size_t) args[5].u;
        }
        err = write_bl_pages(args[2].u, args[3].u, buffer, buffer_len);
    }
#endif // WITH_NAND_DEBUG
    else if(!strcmp(cmd, "reset"))
    {
        LowFuncTbl *fil = FIL_GetFuncTbl();
        fil->Reset();
        printf("NAND reset complete\n");
    }
    else if (!strcmp(cmd, "l2p"))
    {
        ANDAddressStruct addr;
        UInt32 size = sizeof(addr);
        LowFuncTbl *fil = FIL_GetFuncTbl();
        UInt32 lbasPerPage;        

        if (argc < 3)
        {
            usage();
            return -1;
        }

        fil = FIL_GetFuncTbl();
        lbasPerPage = fil->GetDeviceInfo(AND_DEVINFO_FIL_LBAS_PER_PAGE); 

        memset(&addr, 0, size);
        addr.dwLpn = args[2].u;
        if (ctrl_io(AND_STRUCT_FTL_GETADDRESS, &addr, &size))
        {
            if (1 == lbasPerPage)
                printf("vpn: 0x%08x ce: %d ppn: 0x%08x\n", 
                        addr.dwVpn, addr.dwCS, addr.dwPpn);
            else
                printf("vba: 0x%08x ce: %d ppn: 0x%08x\n", 
                        addr.dwVpn, addr.dwCS, addr.dwPpn);

        }
        else
        {
            err = -1;
        }
    }
    else if (!strcmp(cmd, "v2p"))
    {
        ANDAddressStruct addr;
        UInt32 size = sizeof(addr);
        
        if (argc < 3)
        {
            usage();
            return -1;
        }
        memset(&addr, 0, size);
        addr.dwVpn = args[2].u;
        if (ctrl_io(AND_STRUCT_VFL_GETADDRESS, &addr, &size))
        {
            printf("ce: %d ppn: 0x%08x\n", 
                   addr.dwCS, addr.dwPpn);
        }
        else
        {
            err = -1;
        }
    }
    else
    {
        printf("unsupported operation: %s\n", args[1].str);
        usage();
        err = -1;
    }

    return err;
}

MENU_COMMAND_DEVELOPMENT(nand, do_nand, "physical nand commands", NULL);


#endif // !RELEASE_BUILD && WITH_MENU
