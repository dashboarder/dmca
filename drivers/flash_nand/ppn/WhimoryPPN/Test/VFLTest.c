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
// Just call WMR_VFL_Test() with an opened VFL. Test will run on burnin
// area, leaving FTL untouched
//

#include "WMROAM.h"
#include "WMRBuf.h"
#include "ANDTypes.h"
#include "VFL.h"

#define SVFL 0
////////////////////////////////////////////////////////////////////////////////
// Local types
////////////////////////////////////////////////////////////////////////////////
typedef struct
{
    UInt16 num_ftl_vb;
    UInt16 num_total_vb;
    UInt16 pages_per_vb;
    UInt16 bytes_per_page;
    UInt32 lba_per_page;
    UInt32 bytes_per_meta;
}VFLTestInfo;

typedef struct
{
    void * write_main;
    void * write_meta;
    void * verify_main;
    void * verify_meta;
    UInt32 * virtual_page_list;
}VFLTestBuffers;

////////////////////////////////////////////////////////////////////////////////
// Static variables
////////////////////////////////////////////////////////////////////////////////

static VFLFunctions   *_vfl = NULL;
static VFLTestInfo _test_info;
static VFLTestBuffers _buf;

VFLTestBuffers *my_vfl_test_buffers = &_buf; // convenience pointer

#if(SVFL)
static VFL_ReadSpans_t s;
#endif

////////////////////////////////////////////////////////////////////////////////
// Static function declarations
////////////////////////////////////////////////////////////////////////////////

static BOOL32
_GetVFLUInt16
    (UInt32 struct_name,
    UInt16 *value);

static BOOL32
_GetVFLUInt32
    (UInt32 struct_name,
    UInt32 *value);

static BOOL32
_GetTestInfo
    (void);

static void
_FillBuffer
    (UInt8 *buffer,
    UInt32 size,
    UInt8 seed);

static void
_FillSwissMetaBuffer
    (UInt8 *buffer,
    UInt32 size,
    UInt8 seed);

static BOOL32
_RunSequentialTest
    (void);

////////////////////////////////////////////////////////////////////////////////
// Public functions
////////////////////////////////////////////////////////////////////////////////

// Entry point
BOOL32
WMR_VFL_Test
    (VFLFunctions *vfl)
{
    WMR_BufZone_t zone;
    BOOL32 result = TRUE32;

    WMR_MEMSET(&_test_info, 0, sizeof(_test_info));
    WMR_MEMSET(&_buf, 0, sizeof(_buf));

    _vfl = vfl;
    if (!_GetTestInfo())
    {
        return FALSE32;
    }

    WMR_BufZone_Init(&zone);

    _buf.write_main = WMR_Buf_Alloc_ForDMA(&zone,
                                           _test_info.pages_per_vb * _test_info.bytes_per_page);
    _buf.write_meta = WMR_Buf_Alloc_ForDMA(&zone,
                                           _test_info.pages_per_vb * _test_info.lba_per_page * _test_info.bytes_per_meta );
    _buf.verify_main = WMR_Buf_Alloc_ForDMA(&zone,
                                            _test_info.pages_per_vb * _test_info.bytes_per_page);
    _buf.verify_meta = WMR_Buf_Alloc_ForDMA(&zone,
                                            _test_info.pages_per_vb * _test_info.lba_per_page * _test_info.bytes_per_meta);

    result = WMR_BufZone_FinishedAllocs(&zone);
    WMR_ASSERT(result);

    WMR_BufZone_Rebase(&zone, &_buf.write_main);
    WMR_BufZone_Rebase(&zone, &_buf.write_meta);
    WMR_BufZone_Rebase(&zone, &_buf.verify_main);
    WMR_BufZone_Rebase(&zone, &_buf.verify_meta);

    WMR_BufZone_FinishedRebases(&zone);

    _buf.virtual_page_list = WMR_MALLOC(sizeof(UInt32) * _test_info.pages_per_vb);
    WMR_ASSERT(NULL != _buf.virtual_page_list);

    result = _RunSequentialTest();

    WMR_BufZone_Free(&zone);
    WMR_FREE(_buf.virtual_page_list, sizeof(UInt32) * _test_info.pages_per_vb);
    return result;
}

////////////////////////////////////////////////////////////////////////////////
// Static function implementations
////////////////////////////////////////////////////////////////////////////////
#if SVFL
static void vfl_read_cb(UInt32 vba, VFLReadStatusType status, UInt8 *meta)
{
    if (status & VFL_READ_STATUS_REFRESH) {
        WMR_PANIC( "sftl: requested refresh vba:0x%x status:0x%x\n", vba, status);
    }
    if (status & VFL_READ_STATUS_RETIRE) {
        WMR_PANIC("sftl: requested retire vba:0x%x status:0x%x\n", vba, status);
    }
    if (status & VFL_READ_STATUS_UECC) {
        WMR_PANIC("sftl: error uECC vba:0x%x status:0x%x\n", vba, status);
    }
    if (status & VFL_READ_STATUS_CLEAN) {
        WMR_PANIC("sftl: error clean vba:0x%x status:0x%x\n", vba, status);
    }
}
#endif // SVFL

BOOL32
_RunSequentialTest
    (void)
{
    BOOL32 nand_result;
    UInt16 block;
    
    const UInt32 TEST_CHUNK_PAGES = _test_info.pages_per_vb;

    const UInt32 bytes_per_stripe_main =
        TEST_CHUNK_PAGES * _test_info.bytes_per_page;
    const UInt32 bytes_per_stripe_meta =
        TEST_CHUNK_PAGES * _test_info.lba_per_page * _test_info.bytes_per_meta;

    _FillBuffer(_buf.write_main, bytes_per_stripe_main, 0x34);
    _FillSwissMetaBuffer(_buf.write_meta, bytes_per_stripe_meta, 0);

    WMR_MEMCPY(_buf.verify_main, _buf.write_main, bytes_per_stripe_main);
    WMR_MEMCPY(_buf.verify_meta, _buf.write_meta, bytes_per_stripe_meta);

#if(!SVFL)    //Vanilla PPNVFL
    {
#ifndef AND_READONLY
        UInt32 page;
        BOOL32 needs_refresh = FALSE32;
    
        WMR_PRINT(ALWAYS,"***************************************Vanilla PPNVFL test starting******************************\n");
        for (block = _test_info.num_ftl_vb; block < _test_info.num_total_vb; ++block)
        {
            Int32 status = FIL_SUCCESS;
            const UInt32 first_page_in_vb = block * _test_info.pages_per_vb;
            WMR_PRINT(ALWAYS, "Erase block %d(0x%08x)\n", block, block);
            status = _vfl->Erase(block, FALSE32);
            WMR_ASSERT(VFL_SUCCESS == status);
            WMR_PRINT(ALWAYS, "Write pages %d(0x%08x):%d\n", 
                        first_page_in_vb, first_page_in_vb, _test_info.pages_per_vb);
            nand_result = _vfl->WriteMultiplePagesInVb(first_page_in_vb,
                                                       TEST_CHUNK_PAGES,
                                                       _buf.write_main,
                                                       _buf.write_meta,
                                                       FALSE32,
                                                       FALSE32);
    
            WMR_ASSERT(nand_result);
    
            for (page = 0; page < TEST_CHUNK_PAGES; ++page)
            {
                _buf.virtual_page_list[page] = first_page_in_vb + page;
            }
            WMR_PRINT(ALWAYS, "Readback pages %d(0x%08x):%d\n", 
                        first_page_in_vb, first_page_in_vb, _test_info.pages_per_vb);
            status = _vfl->ReadScatteredPagesInVb(_buf.virtual_page_list,
                                                  TEST_CHUNK_PAGES,
                                                  _buf.verify_main,
                                                  _buf.verify_meta,
                                                  &needs_refresh,
                                                  NULL,
                                                  FALSE32,
                                                  NULL);
    
            //WMR_ASSERT(VFL_SUCCESS == status);
            WMR_ASSERT(!WMR_MEMCMP(_buf.verify_main, _buf.write_main, bytes_per_stripe_main));
            WMR_ASSERT(!WMR_MEMCMP(_buf.verify_meta, _buf.write_meta, bytes_per_stripe_meta));
            WMR_ASSERT(!needs_refresh);
        }
#endif // !AND_READONLY
        // Make another read pass after all programming is complete
        for (block = _test_info.num_ftl_vb; block < _test_info.num_total_vb; ++block)
        {
            const UInt32 first_page_in_vb = block * _test_info.pages_per_vb;
            for (page = 0; page < TEST_CHUNK_PAGES; ++page)
            {
                _buf.virtual_page_list[page] = first_page_in_vb + page;
            }
            WMR_PRINT(ALWAYS, "Read pages %d(0x%08x):%d\n", 
                        first_page_in_vb, first_page_in_vb, _test_info.pages_per_vb);
            nand_result = _vfl->ReadScatteredPagesInVb(_buf.virtual_page_list,
                                                       TEST_CHUNK_PAGES,
                                                       _buf.verify_main,
                                                       _buf.verify_meta,
                                                       &needs_refresh,
                                                       NULL,
                                                       FALSE32,
                                                       NULL);
    
            WMR_ASSERT(nand_result);
            WMR_ASSERT(!WMR_MEMCMP(_buf.verify_main, _buf.write_main, bytes_per_stripe_main));
            WMR_ASSERT(!WMR_MEMCMP(_buf.verify_meta, _buf.write_meta, bytes_per_stripe_meta));
            WMR_ASSERT(!needs_refresh);
        }
    
    }
#else  //SwissVFL
    {
        UInt32 vba;
        WMR_PRINT(ALWAYS,"***************************************SVFL test starting******************************\n");
        WMR_PRINT(ALWAYS,"pages_per_vb = %d, lbas_per_page = %d\n",_test_info.pages_per_vb, _test_info.lba_per_page);
        WMR_PRINT(ALWAYS,"start blk = %d, end_blk = %d\n",_test_info.num_ftl_vb, _test_info.num_total_vb);
        WMR_PRINT(ALWAYS,"bytes_per_meta = %d, buf_write_data: 0x%X, buf_write_meta: 0x%X, buf_read_data: 0x%X, buf_read_meta: 0x%X\n",
                  _test_info.bytes_per_meta, _buf.write_main, _buf.write_meta, _buf.verify_main, _buf.verify_meta);
    
        //Erase + Program
        for (block = _test_info.num_ftl_vb; block < _test_info.num_total_vb; ++block)
        {
            if(!(_vfl->GetVbasPerVb(block)))
            {
                continue;
            }
            Int32 status = FIL_SUCCESS;
            const UInt32 first_page_in_vb = block * _test_info.pages_per_vb;
            const UInt32 VbasInBlock = _vfl->GetVbasPerVb(block);
    
            WMR_PRINT(ALWAYS, "Erase block %d(0x%08x)\n", block, block);
            status = _vfl->Erase(block, FALSE32);
            WMR_ASSERT(VFL_SUCCESS == status); 
            nand_result = _vfl->ProgramMultipleVbas((first_page_in_vb * _test_info.lba_per_page),
                                                    VbasInBlock,
                                                    _buf.write_main,
                                                    _buf.write_meta,
                                                    FALSE32,
                                                    TRUE32);
            WMR_ASSERT(nand_result);
        }
    
        //Readback
        for (block = _test_info.num_ftl_vb; block < _test_info.num_total_vb; ++block)
        {
            if(!(_vfl->GetVbasPerVb(block)))
            {
                continue;
            }
            const UInt32 start_vba = block * _test_info.pages_per_vb * _test_info.lba_per_page;
            const UInt32 end_vba = start_vba + _vfl->GetVbasPerVb(block);
            for(vba = start_vba; vba < end_vba; vba++)
            {
                _vfl->ReadSpansInit(&s, vfl_read_cb, VFL_READ_STATUS_ALL, TRUE32, FALSE32);
                _vfl->ReadSpansAdd(&s, vba, 1, _buf.verify_main, _buf.verify_meta);
                if(_vfl->ReadSpans(&s) & (VFL_READ_STATUS_UECC | VFL_READ_STATUS_CLEAN))
                {
                    ANDAddressStruct stAddr;
                    UInt32 dwAddrSize = sizeof(stAddr);
                    stAddr.dwVpn = vba;
                    _vfl->GetStruct(AND_STRUCT_VFL_GETADDRESS, &stAddr, &dwAddrSize);
                    WMR_PANIC("error reading: vba: = 0x%x cs = 0x%x ppn = 0x%x\n", vba, stAddr.dwCS, stAddr.dwPpn);
                }
                WMR_ASSERT(!WMR_MEMCMP(_buf.verify_main, _buf.write_main, (_test_info.bytes_per_page/_test_info.lba_per_page)));
                WMR_ASSERT(!WMR_MEMCMP(_buf.verify_meta, _buf.write_meta,  _test_info.bytes_per_meta));
            }
        }
    }
#endif

    WMR_PRINT(ALWAYS, "*************************************Test Passed***********************************************\n");
    return TRUE32;
}

static void
_FillBuffer
    (UInt8 *buffer,
    UInt32 size,
    UInt8 seed)
{
    // Use a counting pattern for now
    while (size--)
    {
        *buffer++ = seed++;
    }
}

static void
_FillSwissMetaBuffer
    (UInt8 *buffer,
    UInt32 size,
    UInt8 seed)
{
    UInt8 original_seed = seed;
    // Use a counting pattern for now
    while (size--)
    {
        *buffer++ = seed++;
        if((seed % (_test_info.bytes_per_meta)) == 0 )
        {
            seed = original_seed;
        }
    }
}

BOOL32
_GetVFLUInt16
    (UInt32 struct_name,
    UInt16 *value)
{
    UInt32 size = sizeof(*value);

    WMR_ASSERT(NULL != value);
    *value = 0;
    if (!_vfl->GetStruct(struct_name, value, &size) ||
        (size != sizeof(*value)))
    {
        WMR_PRINT(ERROR, "Failed 0x%08x\n", struct_name);
        return FALSE32;
    }
    return TRUE32;
}

BOOL32
_GetVFLUInt32
    (UInt32 struct_name,
    UInt32 *value)
{
    UInt32 size = sizeof(*value);

    WMR_ASSERT(NULL != value);
    *value = 0;
    if (!_vfl->GetStruct(struct_name, value, &size) ||
        (size > sizeof(*value)))
    {
        WMR_PRINT(ERROR, "Failed 0x%08x\n", struct_name);
        return FALSE32;
    }
    return TRUE32;
}

BOOL32
_GetTestInfo
    (void)
{
    if (!_GetVFLUInt16(AND_STRUCT_VFL_NUM_OF_FTL_SUBLKS,
                       &_test_info.num_ftl_vb))
    {
        return FALSE32;
    }
    
    if (!_GetVFLUInt16(AND_STRUCT_VFL_NUM_OF_SUBLKS,
                       &_test_info.num_total_vb))
    {
        return FALSE32;
    }

    if (!_GetVFLUInt16(AND_STRUCT_VFL_PAGES_PER_SUBLK,
                       &_test_info.pages_per_vb))
    {
        return FALSE32;
    }

    if (!_GetVFLUInt16(AND_STRUCT_VFL_BYTES_PER_PAGE,
                       &_test_info.bytes_per_page))
    {
        return FALSE32;
    }

    _test_info.lba_per_page = _vfl->GetDeviceInfo(AND_DEVINFO_FIL_LBAS_PER_PAGE);

    if (!_GetVFLUInt32(AND_STRUCT_VFL_BYTES_PER_META,
                       &_test_info.bytes_per_meta))
    {
        return FALSE32;
    }

    return TRUE32;
}

