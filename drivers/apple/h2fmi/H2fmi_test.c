// *****************************************************************************
//
// File: H2fmi_test.c
//
// *****************************************************************************
//
// Notes:
//
// *****************************************************************************
//
// Copyright (C) 2008 Apple Computer, Inc. All rights reserved.
//
// This document is the property of Apple Computer, Inc.
// It is considered confidential and proprietary.
//
// This document may not be reproduced or transmitted in any form,
// in whole or in part, without the express written permission of
// Apple Computer, Inc.
//
// *****************************************************************************

#include "H2fmi_private.h"

#if (H2FMI_TEST_HOOK)

#include "H2fmi.h"

#include <debug.h>

#include <platform/memmap.h>
#include <stdlib.h>

// =============================================================================
// preprocessor compilation control
// =============================================================================

#define TEST_WRITE_WITH_CORRUPTED_BITS true
#define TEST_WRITE_WITH_CORRUPT_META true
#define TEST_WRITE_WITH_CORRUPT_MAIN true

#define TESTING_BASIC_OPS true

#define DUMP_BUFS false
#define DUMP_ERASE_BUFS true
#define DUMP_WRITE_BUFS true
#define DUMP_CORRUPTED_BUFS true

#define ERASE_DIRTY_THRESHOLD 10

// TODO: fix this some other way
#define FIL_SUCCESS        ((int32_t)0)
#define FIL_SUCCESS_CLEAN  ((int32_t)1)

// =============================================================================
// external function declarations
// =============================================================================

extern int nand_read_bootblock(u_int8_t interface, uint8_t cs, void* address, size_t* size);

// =============================================================================
// private implementation function declarations
// =============================================================================

static bool h2fmi_test_erase(h2fmi_t* fmi, uint32_t ce, uint32_t block, uint32_t page);
static bool h2fmi_test_write(h2fmi_t* fmi, uint32_t ce, uint32_t page);

static bool h2fmi_test_read_bootblock(void);

// =============================================================================
// externally visible test function definitions
// =============================================================================

bool h2fmi_test_hook(h2fmi_t* fmi)
{
    bool result = false;

    dprintf(DEBUG_CRITICAL, "running test hook\n");

    h2fmi_spew_config(fmi);

    if (TESTING_BASIC_OPS)
    {
        const uint32_t ce    = 0;
        const uint32_t block = 517;
        const uint32_t page  = (block * fmi->pages_per_block);

        result = h2fmi_test_erase(fmi, ce, block, page);
        result = h2fmi_test_write(fmi, ce, page);
    }
    else
    {
        result = h2fmi_test_read_bootblock();
    }

    return result;
}

// =============================================================================
// static test function definitions
// =============================================================================

static bool h2fmi_test_read_bootblock(void)
{
    const uint32_t bootpage_size = 3 * 512;
    const uint32_t bootblock_max_size = 128 * bootpage_size;

    uint8_t* boot_buf = (uint8_t*)malloc(bootblock_max_size);

    bool result = false;

    uint32_t interface;
    uint32_t ce;

    for (interface = 0; interface < 2; interface++)
    {
        for (ce = 0; ce < 8; ce++)
        {
            size_t bootblock_size = bootblock_max_size;

            dprintf(DEBUG_INFO,
                    "attempting boot from NAND %d CS %d\n",
                    interface, ce);

            // clean read buffer
            memset(boot_buf, 0x00, bootblock_max_size);

            // attempt to read bootblock
            if (nand_read_bootblock(interface, ce, boot_buf, &bootblock_size) == 0)
            {
                // TODO: do minimal verification of bootblock by checking
                // for known pattern
                if ((boot_buf[0] == '3') &&
                    (boot_buf[1] == 'g') &&
                    (boot_buf[2] == 'm') &&
                    (boot_buf[3] == 'I'))
                {
                    dprintf(DEBUG_INFO, "loaded boot image\n");
                    result = true;
                }
            }
            else
            {
                dprintf(DEBUG_SPEW, "load failed\n");
            }

            if (DUMP_BUFS)
            {
                dprintf(DEBUG_SPEW, "boot_buf\n");
                h2fmi_spew_buffer(boot_buf, bootpage_size);
            }
        }
    }

    // TODO: free allocated buffers

    if (!result)
    {
        dprintf(DEBUG_CRITICAL, "ERROR: failed to load any boot images!\n");
    }

    return result;
}

static bool h2fmi_test_erase(h2fmi_t* fmi,
                             uint32_t ce,
                             uint32_t block,
                             uint32_t page)
{
    bool result = true;

    const uint32_t bytes_per_page  = fmi->bytes_per_page;
    const uint32_t bytes_per_spare = fmi->bytes_per_spare;
    const uint32_t bytes_per_meta  = fmi->bytes_per_meta;

    uint8_t* data_buf_0  = (uint8_t*)malloc(bytes_per_page);
    uint8_t* spare_buf_0 = (uint8_t*)malloc(bytes_per_spare);

    uint8_t* data_buf_1  = (uint8_t*)malloc(bytes_per_page);
    uint8_t* meta_buf_1  = (uint8_t*)malloc(bytes_per_meta);

    dprintf(DEBUG_INFO, "h2fmi_test_erase()...\n");

    uint32_t byte_idx;
    uint32_t dirty_count   = 0;
    uint8_t bCorrectedBits = 0;

    // clean read buffers
    memset(data_buf_0, 0x00, bytes_per_page);
    memset(spare_buf_0, 0x00, bytes_per_spare);
    memset(data_buf_1, 0x00, bytes_per_page);
    memset(meta_buf_1, 0x00, bytes_per_meta);

    if (FIL_SUCCESS != h2fmiEraseSingleBlock(ce, block))
    {
        dprintf(DEBUG_CRITICAL, "ERROR: failed erase\n");
        h2fmi_fail(result);
    }
    else
    {
        dprintf(DEBUG_SPEW, "h2fmiEraseSingleBlock succeeded\n");
    }

    // read back with ECC, verifying that page is reported as "clean"
    if (FIL_SUCCESS_CLEAN != h2fmiReadSinglePage(ce, page,
                                                 data_buf_1,
                                                 meta_buf_1,
                                                 &bCorrectedBits,
                                                 NULL))
    {
        dprintf(DEBUG_CRITICAL, "ERROR: failed read single of erased page without detecting clean\n");
        h2fmi_fail(result);
    }
    else
    {
        dprintf(DEBUG_SPEW, "h2fmiReadSinglePage correctly reported FIL_SUCCESS_CLEAN\n");
    }

    // read back raw contents of page (i.e. with no ECC)
    if (FIL_SUCCESS != h2fmiReadNoECC(ce, page,
                                      data_buf_0,
                                      spare_buf_0))
    {
        dprintf(DEBUG_CRITICAL, "ERROR: failed raw readback from erased page\n");
        h2fmi_fail(result);
    }
    else
    {
        dprintf(DEBUG_SPEW, "h2fmiReadNoECC succeeded\n");
    }

    // verify that raw readback buffer is mostly all 0xFF
    for (byte_idx = 0; byte_idx < bytes_per_page; byte_idx++)
    {
        if (0xFF != data_buf_0[byte_idx])
        {
            dirty_count++;
        }
    }
    for (byte_idx = 0; byte_idx < bytes_per_spare; byte_idx++)
    {
        if (0xFF != spare_buf_0[byte_idx])
        {
            dirty_count++;
        }
    }
    if (dirty_count > ERASE_DIRTY_THRESHOLD)
    {
        dprintf(DEBUG_CRITICAL,
                "ERROR: %d bytes reported not 0xFF by ReadNoECC after Erase\n",
                dirty_count);
        h2fmi_fail(result);
    }
    else
    {
        dprintf(DEBUG_INFO, "%d bytes reported dirty\n", dirty_count);
    }

    if (!result && DUMP_ERASE_BUFS)
    {
        dprintf(DEBUG_INFO, "data_buf_0\n");
        h2fmi_spew_buffer(data_buf_0, bytes_per_page);
        dprintf(DEBUG_INFO, "spare_buf_0\n");
        h2fmi_spew_buffer(spare_buf_0, bytes_per_spare);

        dprintf(DEBUG_INFO, "data_buf_1\n");
        h2fmi_spew_buffer(data_buf_1, bytes_per_page);
        dprintf(DEBUG_INFO, "meta_buf_1\n");
        h2fmi_spew_buffer(meta_buf_1, bytes_per_meta);
    }

    // using free causes panic with error...

    //free(data_buf_0);
    //free(spare_buf_0);
    //free(data_buf_1);
    //free(meta_buf_1);

    dprintf(DEBUG_INFO, "...%s\n", result ? "succeeded" : "failed");

    return result;
}

static bool h2fmi_test_write(h2fmi_t* fmi, uint32_t ce, uint32_t page)
{
    bool result = true;

    const uint32_t bytes_per_page = fmi->bytes_per_page;
    const uint32_t bytes_per_meta = fmi->bytes_per_meta;

    uint8_t* data_buf_2 = (uint8_t*)malloc(bytes_per_page);
    uint8_t* meta_buf_2 = (uint8_t*)malloc(bytes_per_meta);

    uint8_t* data_buf_3 = (uint8_t*)malloc(bytes_per_page);
    uint8_t* meta_buf_3 = (uint8_t*)malloc(bytes_per_meta);

    uint8_t* data_buf_4 = (uint8_t*)malloc(bytes_per_page);
    uint8_t* meta_buf_4 = (uint8_t*)malloc(bytes_per_meta);

    uint8_t* data_buf_5 = (uint8_t*)malloc(bytes_per_page);
    uint8_t* meta_buf_5 = (uint8_t*)malloc(bytes_per_meta);

    dprintf(DEBUG_INFO, "h2fmi_test_write()...\n");

    const uint32_t row_addr = (page << 16);

    uint32_t word_idx;
    uint32_t byte_idx;

    uint8_t max_corrected = 0;

    // clean read buffers
    memset(data_buf_4, 0xA5, bytes_per_page);
    memset(meta_buf_4, 0x81, bytes_per_meta);
    memset(data_buf_5, 0x5A, bytes_per_page);
    memset(meta_buf_5, 0x18, bytes_per_meta);

    // watermark metadata buffers
    for (byte_idx = 0; (int32_t)byte_idx < bytes_per_meta; byte_idx++)
    {
        uint8_t marker = (uint8_t)(0xC0 | byte_idx);
        meta_buf_2[byte_idx] = marker;

#if (TEST_WRITE_WITH_CORRUPT_META)
        meta_buf_3[byte_idx] = ~marker;
#else
        meta_buf_3[byte_idx] = marker;
#endif
    }

    // watermark both data buffers identically
    for (word_idx = 0; word_idx < (bytes_per_page >> 2); word_idx++)
    {
        const uint32_t col_addr = (word_idx << 2);
        const uint32_t marker   = (row_addr | col_addr) + 1;
        uint32_t* page_words_2  = (uint32_t*)data_buf_2;
        uint32_t* page_words_3  = (uint32_t*)data_buf_3;
        page_words_2[word_idx] = marker;
        page_words_3[word_idx] = marker;
    }

#if (TEST_WRITE_WITH_CORRUPT_MAIN)
    // corrupt second data buffer
    for (word_idx = 0; word_idx < 24; word_idx++)
    {
        uint32_t* page_words_3 = (uint32_t*)data_buf_3;
        page_words_3[word_idx] ^= 0xFFFFFFFF;
    }
#endif

    // write to erased page
    if (FIL_SUCCESS != h2fmiWriteSinglePage(ce,
                                            page,
                                            data_buf_2,
                                            meta_buf_2))
    {
        dprintf(DEBUG_CRITICAL, "ERROR: failed write of page\n");
        h2fmi_fail(result);
    }
    else
    {
        dprintf(DEBUG_SPEW, "h2fmiWriteSinglePage succeeded\n");
    }

    // read back page, expecting it to not have uncorrectable ECC errors
    if (FIL_SUCCESS != h2fmiReadSinglePage(ce,
                                           page,
                                           data_buf_4,
                                           meta_buf_4,
                                           &max_corrected,
                                           NULL))
    {
        dprintf(DEBUG_CRITICAL, "ERROR: failed read of written page\n");
        h2fmi_fail(result);
    }
    else
    {
        dprintf(DEBUG_SPEW, "h2fmiReadSinglePage succeeded\n");
    }

#if (TEST_WRITE_WITH_CORRUPTED_BITS)
    // write again to same page, with significant bit toggles in main data (and possibly meta data)
    if (FIL_SUCCESS != h2fmiWriteSinglePage(ce,
                                            page,
                                            data_buf_3,
                                            meta_buf_3))
    {
        dprintf(DEBUG_CRITICAL, "ERROR: failed second (corrupting) write\n");
        h2fmi_fail(result);
    }
    else
    {
        dprintf(DEBUG_SPEW, "h2fmiWriteSinglePage (corrupting) succeeded\n");
    }

    // read back page, expecting it to fail due to uncorrectable ECC errors
    if (FIL_SUCCESS == h2fmiReadSinglePage(ce,
                                           page,
                                           data_buf_5,
                                           meta_buf_5,
                                           &max_corrected,
                                           NULL))
    {
        // yell if we did NOT get an error
        dprintf(DEBUG_CRITICAL, "ERROR: succeeded read of corrupted page\n");
        h2fmi_fail(result);
    }
    else
    {
        dprintf(DEBUG_SPEW, "h2fmiReadSinglePage (corrupting) failed as expected\n");
    }
#endif // TEST_WRITE_WITH_CORRUPTED_BITS

    // TODO: check fail with blank check as well

    // verify clean readback buffer
    //
    // TODO: add quick memcmp for initial failure detection and
    // then scan byte by byte only if failed
    for (byte_idx = 0; byte_idx < bytes_per_page; byte_idx++)
    {
        const uint8_t wrote = data_buf_2[byte_idx];
        const uint8_t read  = data_buf_4[byte_idx];
        if (wrote != read)
        {
            dprintf(DEBUG_CRITICAL,
                    "ERROR: failed main data verify at byte %d, 0x%02X != 0x%02X\n",
                    byte_idx, wrote, read);
            h2fmi_fail(result);
        }
    }
    for (byte_idx = 0; (int32_t)byte_idx < fmi->bytes_per_meta; byte_idx++)
    {
        const uint8_t wrote = meta_buf_2[byte_idx];
        const uint8_t read  = meta_buf_4[byte_idx];
        if (wrote != read)
        {
            dprintf(DEBUG_CRITICAL,
                    "ERROR: failed meta data verify at byte %d, 0x%02X != 0x%02X\n",
                    byte_idx, wrote, read);
            h2fmi_fail(result);
        }
    }

    // TODO: verify corrupted readback buffer (using raw read)

    if (!result && DUMP_WRITE_BUFS)
    {
        dprintf(DEBUG_INFO, "data_buf_2\n");
        h2fmi_spew_buffer(data_buf_2, bytes_per_page);
        dprintf(DEBUG_INFO, "meta_buf_2\n");
        h2fmi_spew_buffer(meta_buf_2, bytes_per_meta);

        dprintf(DEBUG_INFO, "data_buf_4\n");
        h2fmi_spew_buffer(data_buf_4, bytes_per_page);
        dprintf(DEBUG_INFO, "meta_buf_4\n");
        h2fmi_spew_buffer(meta_buf_4, bytes_per_meta);
    }

    if (!result && DUMP_CORRUPTED_BUFS)
    {
        dprintf(DEBUG_INFO, "data_buf_3\n");
        h2fmi_spew_buffer(data_buf_3, bytes_per_page);
        dprintf(DEBUG_INFO, "meta_buf_3\n");
        h2fmi_spew_buffer(meta_buf_3, bytes_per_meta);

        dprintf(DEBUG_INFO, "data_buf_5\n");
        h2fmi_spew_buffer(data_buf_5, bytes_per_page);
        dprintf(DEBUG_INFO, "meta_buf_5\n");
        h2fmi_spew_buffer(meta_buf_5, bytes_per_meta);
    }

    // using free causes panic with error...

    //free(data_buf_2);
    //free(meta_buf_2);
    //free(data_buf_3);
    //free(meta_buf_3);
    //free(data_buf_4);
    //free(meta_buf_4);
    //free(data_buf_5);
    //free(meta_buf_5);

    dprintf(DEBUG_INFO, "...%s\n", result ? "succeeded" : "failed");

    return result;
}

#endif // H2FMI_TEST_HOOK

// ********************************** EOF **************************************
