// *****************************************************************************
//
// File: H2fmi_read.c
//
// *****************************************************************************
//
// Notes:
//
//   - DMA_FMI0CHECK & DMA_FMI1CHECK names in hwdmachannels.h is a bit funny;
//     consider getting changed to DMA_FMI0META & DMA_FMI1META
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
#include "H2fmi.h"
#include <FIL.h>
#include "H2fmi_dma.h"

/**
 * Define the following to 'simulate' NAND read errors. 
 * ( and select an error type below ) 
 */
//??#define SIMULATE_READ_FAILURES

#if defined(SIMULATE_READ_FAILURES)
#define STICKY_READ_FAILURES
#define READ_FAILURE_MAGIC_NUMBER (159871/10)
static unsigned long ulFailureCount = 0;
static UInt32 ulSimulatedFailingCE = ~0;;
static UInt32 ulSimulatedFailingPage = ~0;
#define READ_FAILURE_STATUS_TYPE _kIOPFMI_STATUS_AT_LEAST_ONE_UECC
//??#define READ_FAILURE_STATUS_TYPE _kIOPFMI_STATUS_BLANK
//??#define READ_FAILURE_STATUS_TYPE _kIOPFMI_STATUS_FUSED
//??#define READ_FAILURE_STATUS_TYPE _kIOPFMI_STATUS_ECC_DONE_TIMEOUT
#endif

#if (defined (AND_COLLECT_FIL_STATISTICS) && AND_COLLECT_FIL_STATISTICS)
extern FILStatistics stFILStatistics;
#endif

// =============================================================================
// private implementation function declarations
// =============================================================================
typedef struct
{
    enum
    {
        kh2fmi_sector_status_normal,
        kh2fmi_sector_status_clean,
        kh2fmi_sector_status_clean_with_stuck,
        kh2fmi_sector_status_fused,
        kh2fmi_sector_status_uecc
    } status;

    UInt32 error_cnt;
} h2fmi_sector_status;

static void h2fmi_read_page_start(h2fmi_t* fmi, UInt32 page);
static void h2fmi_prepare_read(h2fmi_t* fmi, UInt32 page_idx, h2fmi_ce_t*  chip_enable_array, UInt32*  page_number_array );
static BOOL32 h2fmi_complete_read_page_start(h2fmi_t* fmi);
static BOOL32 h2fmi_rx_raw(h2fmi_t* fmi, UInt8*   data_buf, UInt8*   spare_buf);
#if FMI_VERSION == 0
static void read_raw_sgl_prep(struct dma_segment* data_sgl, struct dma_segment* meta_sgl, UInt8* buf1, UInt8* buf2, UInt32 bytes_per_page, UInt32 bytes_per_spare);
#endif //FMI_VERSION == 0

#if FMI_VERSION > 0
static h2fmi_sector_status h2fmi_rx_check_sector_ecc(h2fmi_t* fmi);
#endif

static UInt32 h2fmi_dma_rx_complete(h2fmi_t* fmi, UInt32 ecc_pend, UInt8* max_corrected, UInt8* sector_stats);
static void h2fmi_reset_ecc(h2fmi_t* fmi);

// =============================================================================
// extern implementation function definitions
// =============================================================================
UInt32 h2fmi_read_bootpage_pio(h2fmi_t* fmi,
                               UInt32   ce,
                               UInt32   page,
                               UInt8*   buf,
                               UInt8*   max_corrected)
{
    UInt32  result;
    UInt32  i;
    UInt32  ecc_pend;
    UInt32 fmi_control = (FMI_CONTROL__MODE__READ |
                          FMI_CONTROL__START_BIT);

#if FMI_VERSION == 4
    if (fmi->read_stream_disable)
    {
        fmi_control |= FMI_CONTROL__DISABLE_STREAMING;
    }
#endif // FMI_VERSION == 4

    h2fmi_reset_ecc(fmi);

    // enable specified NAND device
    h2fmi_fmc_enable_ce(fmi, ce);

    // start a nand page read operation
    h2fmi_read_page_start(fmi, page);
    if (!h2fmi_complete_read_page_start(fmi))
    {
        h2fmi_fail(result);
    }
    else
    {
        // Set page format to enable seed scrambling before pushing seeds into FIFO
        h2fmi_set_bootpage_data_format(fmi);

#if FMI_VERSION > 3
        UInt32 sector;
        h2fmi_wr(fmi, FMI_CONTROL, FMI_CONTROL__RESET_SEED);
        // Fill randomizer seeds based on page address
        for (sector = 0; sector < H2FMI_BOOT_SECTORS_PER_PAGE; sector++)
        {
            h2fmi_wr(fmi, FMI_SCRAMBLER_SEED_FIFO, page + sector);
        }
#endif

        // Since we're transferring a strange page size, we'll still make FMI deal with this
        // operation by individual sectors by just looping over all sectors, starting an FMI operation,
        // and waiting for a LAST_FMC_DONE interrupt.

        for (i = 0; i < H2FMI_BOOT_SECTORS_PER_PAGE; i++)
        {
            h2fmi_wr(fmi, FMI_CONTROL, fmi_control);
            if (!h2fmi_wait_done(fmi, FMI_INT_PEND, FMI_INT_PEND__LAST_FMC_DONE, FMI_INT_PEND__LAST_FMC_DONE))
            {
                h2fmi_fail(result);
            }
            else
            {
                h2fmi_pio_read_sector(fmi, buf, H2FMI_BYTES_PER_BOOT_SECTOR);
                buf += H2FMI_BYTES_PER_BOOT_SECTOR;
            }
        }
    }

    // disable NAND device
    h2fmi_fmc_disable_ce(fmi, ce);

    // Get ECC stats
    ecc_pend = h2fmi_rd(fmi, ECC_PND);
    h2fmi_wr(fmi, ECC_PND, ecc_pend);
    result = h2fmi_rx_check_page_ecc(fmi, ecc_pend, max_corrected, NULL, H2FMI_BOOT_SECTORS_PER_PAGE);

    h2fmi_clear_interrupts_and_reset_masks(fmi);

    h2fmi_reset(fmi);

    return result;
}


UInt32 h2fmi_read_bootpage(h2fmi_t* fmi,
                           UInt32   ce,
                           UInt32   page,
                           UInt8*   buf,
                           UInt8*   max_corrected)
{
    UInt32  result;
    UInt32  i;
    struct dma_segment sgl;
    UInt32 ecc_pend;
    UInt32 fmi_control = (FMI_CONTROL__MODE__READ |
                          FMI_CONTROL__START_BIT);

#if FMI_VERSION == 4
    if (fmi->read_stream_disable)
    {
        fmi_control |= FMI_CONTROL__DISABLE_STREAMING;
    }
#endif // FMI_VERSION == 4

    // enable specified NAND device
    h2fmi_fmc_enable_ce(fmi, ce);

    sgl.paddr  = (UInt32)buf;
    sgl.length = H2FMI_BOOT_BYTES_PER_PAGE;

    // start a nand page read operation
    h2fmi_read_page_start(fmi, page);
    if (!h2fmi_complete_read_page_start(fmi))
    {
        h2fmi_fail(result);
    }
    else
    {
        // Start DMA for entire operation (regardless of FMI sector/page size capabilities)
        h2fmi_dma_execute_async(DMA_CMD_DIR_RX,
                            h2fmi_dma_data_chan(fmi),
                            &sgl,
                            h2fmi_dma_data_fifo(fmi),
                            H2FMI_BOOT_BYTES_PER_PAGE,
                            sizeof(UInt32),
                            32,
                            NULL);

        // Since we're transferring a strange page size, we'll still make FMI deal with this
        // operation by individual sectors by just looping over all sectors, starting an FMI operation,
        // and waiting for a LAST_FMC_DONE interrupt.
        h2fmi_set_bootpage_data_format(fmi);        
        
#if FMI_VERSION > 3
        UInt32 sector;
        h2fmi_wr(fmi, FMI_CONTROL, FMI_CONTROL__RESET_SEED);
        // Fill randomizer seeds based on page address
        for (sector = 0; sector < H2FMI_BOOT_SECTORS_PER_PAGE; sector++)
        {
            h2fmi_wr(fmi, FMI_SCRAMBLER_SEED_FIFO, page + sector);
        }
#endif
        
        for (i = 0; i < H2FMI_BOOT_SECTORS_PER_PAGE; i++)
        {
            h2fmi_wr(fmi, FMI_CONTROL, fmi_control);
            if (!h2fmi_wait_done(fmi, FMI_INT_PEND, FMI_INT_PEND__LAST_FMC_DONE, FMI_INT_PEND__LAST_FMC_DONE))
            {
                h2fmi_fail(result);
            }
        }
        
        if (!h2fmi_dma_wait(h2fmi_dma_data_chan(fmi), H2FMI_PAGE_TIMEOUT_MICROS))
        {
            h2fmi_dma_cancel(h2fmi_dma_data_chan(fmi));
            h2fmi_fail(result);
        }
    }

    // disable NAND device
    h2fmi_fmc_disable_ce(fmi, ce);

    // Get ECC stats
    ecc_pend = h2fmi_rd(fmi, ECC_PND);
    h2fmi_wr(fmi, ECC_PND, ecc_pend);
    result = h2fmi_rx_check_page_ecc(fmi, ecc_pend, max_corrected, NULL, H2FMI_BOOT_SECTORS_PER_PAGE);

    h2fmi_clear_interrupts_and_reset_masks(fmi);

    h2fmi_reset(fmi);

    return result;    
}

BOOL32 h2fmi_read_raw_page(h2fmi_t* fmi,
                           UInt32   ce,
                           UInt32   page,
                           UInt8*   data_buf,
                           UInt8*   spare_buf)
{
    BOOL32 result = TRUE32;

    // enable specified nand device
    h2fmi_fmc_enable_ce(fmi, ce);

    // start a nand page read operation
    h2fmi_read_page_start(fmi, page);
    if (!h2fmi_complete_read_page_start(fmi))
    {
        h2fmi_fail(result);
    }
    else
    {
        result = h2fmi_rx_raw(fmi,data_buf,spare_buf);
    }

    // disable all nand devices
    h2fmi_fmc_disable_ce(fmi, ce);

    h2fmi_clear_interrupts_and_reset_masks(fmi);

    return result;
}


BOOL32 h2fmi_rx_raw(h2fmi_t* fmi,
                    UInt8*   data_buf,
                    UInt8*   spare_buf)
{
#if FMI_VERSION == 0
    const UInt32 kReadChunks = fmi->bytes_per_page / H2FMI_BYTES_PER_SECTOR; // varies by FMI version
    UInt32 i = 0;
    UInt32 spareBytesLeft;
    struct dma_segment data_sgl[H2FMI_MAX_SGL_SEGMENTS_PER_RAW];
    struct dma_segment meta_sgl[H2FMI_MAX_SGL_SEGMENTS_PER_RAW];
    BOOL32 result = TRUE32;

    read_raw_sgl_prep(data_sgl,
                      meta_sgl,
                      data_buf,
                      spare_buf,
                      fmi->bytes_per_page,
                      fmi->bytes_per_spare);

        // use CDMA to feed meta buffer to FMI
        h2fmi_dma_execute_async(DMA_CMD_DIR_RX,
                            h2fmi_dma_meta_chan(fmi),
                            meta_sgl,
                            h2fmi_dma_meta_fifo(fmi),
                            fmi->bytes_per_spare,
                            sizeof(UInt8),
                            1,
                            NULL);

        // use CDMA to suck data buffer from FMI
        h2fmi_dma_execute_async(DMA_CMD_DIR_RX,
                            h2fmi_dma_data_chan(fmi),
                            data_sgl,
                            h2fmi_dma_data_fifo(fmi),
                            fmi->bytes_per_page,
                            sizeof(UInt32),
                            32,
                            NULL);

    spareBytesLeft = fmi->bytes_per_spare;
    
    for (i = 0; i < kReadChunks; i++)
    {

        UInt32 metaBytesThisEnvelope = WMR_MIN(H2FMI_MAX_META_PER_ENVELOPE, spareBytesLeft);

        h2fmi_wr(fmi, FMI_CONFIG, FMI_CONFIG__DMA_BURST__32_CYCLES |
                 FMI_CONFIG__LBA_MODE__BYPASS_ECC |
                 FMI_CONFIG__META_PER_ENVELOPE(metaBytesThisEnvelope) |
                 FMI_CONFIG__META_PER_PAGE(metaBytesThisEnvelope) |
                 FMI_CONFIG__PAGE_SIZE__512);

        h2fmi_wr(fmi, FMI_CONTROL, (FMI_CONTROL__MODE__READ | FMI_CONTROL__START_BIT));
        h2fmi_wait_done(fmi, FMI_INT_PEND, FMI_INT_PEND__LAST_FMC_DONE, FMI_INT_PEND__LAST_FMC_DONE);

        spareBytesLeft = spareBytesLeft - metaBytesThisEnvelope;
    }

    if (!h2fmi_dma_wait(h2fmi_dma_data_chan(fmi), H2FMI_PAGE_TIMEOUT_MICROS) ||
        !h2fmi_dma_wait(h2fmi_dma_meta_chan(fmi), H2FMI_PAGE_TIMEOUT_MICROS))
    {
        h2fmi_dma_cancel(h2fmi_dma_meta_chan(fmi));
        h2fmi_dma_cancel(h2fmi_dma_data_chan(fmi));
        h2fmi_fail(result);
    }
    return result;

#else // FMI_VERSION == 0
    const UInt32 kFMIMainSectorsToRead = fmi->bytes_per_page / H2FMI_BYTES_PER_SECTOR;
    struct dma_segment data_sgl;
    BOOL32 result = TRUE32;
    UInt32 fmi_control = (FMI_CONTROL__MODE__READ |
                          FMI_CONTROL__START_BIT);

#if FMI_VERSION == 4
    if (fmi->read_stream_disable)
    {
        fmi_control |= FMI_CONTROL__DISABLE_STREAMING;
    }
#endif // FMI_VERSION == 4

    data_sgl.paddr = (UInt32) data_buf;
    data_sgl.length = fmi->bytes_per_page;

    // use CDMA to suck data buffer from FMI
    h2fmi_dma_execute_async(DMA_CMD_DIR_RX,
                        h2fmi_dma_data_chan(fmi),
                        &data_sgl,
                        h2fmi_dma_data_fifo(fmi),
                        fmi->bytes_per_page,
                        sizeof(UInt32),
                        32,
                        NULL);

    // First read the main data
    h2fmi_wr(fmi, FMI_CONFIG, FMI_CONFIG__ECC_CORRECTABLE_BITS(0) | FMI_CONFIG__DMA_BURST__32_CYCLES);
    h2fmi_wr(fmi, FMI_DATA_SIZE, (FMI_DATA_SIZE__META_BYTES_PER_SECTOR(0) |
                                  FMI_DATA_SIZE__META_BYTES_PER_PAGE(0) |
                                  FMI_DATA_SIZE__BYTES_PER_SECTOR(H2FMI_BYTES_PER_SECTOR) |
                                  FMI_DATA_SIZE__SECTORS_PER_PAGE(kFMIMainSectorsToRead)));

    h2fmi_wr(fmi, FMI_CONTROL, fmi_control);
    h2fmi_wait_done(fmi, FMI_INT_PEND, FMI_INT_PEND__LAST_FMC_DONE, FMI_INT_PEND__LAST_FMC_DONE);


    if (!h2fmi_dma_wait(h2fmi_dma_data_chan(fmi), H2FMI_PAGE_TIMEOUT_MICROS))
    {
        h2fmi_dma_cancel(h2fmi_dma_data_chan(fmi));
        h2fmi_fail(result);
    }
    if (result)
    {
        // Now pick up the spare - we'll round this up to the nearest DWORD because the DMA will hang otherwise
        UInt32 rounded_len = ROUNDUPTO(fmi->bytes_per_spare, sizeof(UInt32));

        data_sgl.paddr = (UInt32) spare_buf;
        data_sgl.length = rounded_len;

        // use CDMA to suck data buffer from FMI
        h2fmi_dma_execute_async(DMA_CMD_DIR_RX,
                            h2fmi_dma_data_chan(fmi),
                            &data_sgl,
                            h2fmi_dma_data_fifo(fmi),
                            rounded_len,
                            sizeof(UInt32),
                            1,
                            NULL);



        h2fmi_wr(fmi, FMI_CONFIG, FMI_CONFIG__ECC_CORRECTABLE_BITS(0) | FMI_CONFIG__DMA_BURST__1_CYCLES);
        h2fmi_wr(fmi, FMI_DATA_SIZE, (FMI_DATA_SIZE__META_BYTES_PER_SECTOR(0) |
                                      FMI_DATA_SIZE__META_BYTES_PER_PAGE(0) |
                                      FMI_DATA_SIZE__BYTES_PER_SECTOR(rounded_len) |
                                      FMI_DATA_SIZE__SECTORS_PER_PAGE(1)));

        h2fmi_wr(fmi, FMI_CONTROL, fmi_control);
        h2fmi_wait_done(fmi, FMI_INT_PEND, FMI_INT_PEND__LAST_FMC_DONE, FMI_INT_PEND__LAST_FMC_DONE);


        if (!h2fmi_dma_wait(h2fmi_dma_data_chan(fmi), H2FMI_PAGE_TIMEOUT_MICROS))
        {
            h2fmi_dma_cancel(h2fmi_dma_data_chan(fmi));
            h2fmi_fail(result);
        }
    }
    return result;

#endif //FMI_VERSION != 0
}

#if FMI_VERSION == 0
static void read_raw_sgl_prep(struct dma_segment* data_sgl,
                              struct dma_segment* meta_sgl,
                              UInt8*   data_buf,
                              UInt8*   spare_buf,
                              UInt32   bytes_per_page,
                              UInt32   bytes_per_spare)
{
    UInt32 metaChannelBytesLeft = bytes_per_spare;
    UInt32 dataChannelBytesLeft = bytes_per_page;
    UInt32 bytesHandled;
    UInt32 data_index, meta_index;
    BOOL32 inBuf1, skipData, skipMeta;

    data_index = 0;
    meta_index = 0;
    inBuf1   = TRUE32; // tells if we are populating buf1 or buf2

    // The following code has 2 divisions: one for data channel other for meta channel.
    // When we reach the boundary of buf1 we will have to skip one of the division.
    skipData = FALSE32;
    skipMeta = FALSE32;

    bytesHandled = 0;
    while(dataChannelBytesLeft > 0 || metaChannelBytesLeft > 0)
    {
        if (!skipData)
        {
            if (bytesHandled != bytes_per_page)
            {
                data_sgl[data_index].paddr  = ((data_index == 0 && meta_index == 0) ? (UInt32)data_buf : meta_sgl[meta_index - 1].paddr + meta_sgl[meta_index - 1].length);
                data_sgl[data_index].length = WMR_MIN(H2FMI_BYTES_PER_SECTOR, dataChannelBytesLeft);
            }
            else // transitioning from buf1 to buf2
            {
                data_sgl[data_index].paddr = (UInt32)spare_buf;
                if (skipMeta)
                {
                    // data from one envelope got split into the end of buf1 and beginning of buf2
                    // here we making sure that begining of buf2 gets the remaining data of that envelope
                    // setting length to the remaining data in that envelope
                    data_sgl[data_index].length = WMR_MIN(H2FMI_BYTES_PER_SECTOR, dataChannelBytesLeft + data_sgl[data_index - 1].length) - data_sgl[data_index - 1].length;
                }
                else
                {
                    // this happens when buf1 boundary coincides with the end of meta data from the envelope i.e. no splitting happens
                    data_sgl[data_index].length = WMR_MIN(H2FMI_BYTES_PER_SECTOR, dataChannelBytesLeft);
                }
                inBuf1 = FALSE32;
            }
            if (skipMeta)
            {
                skipMeta = FALSE32;
            }

            bytesHandled += data_sgl[data_index].length;
            if (inBuf1 && bytesHandled > bytes_per_page)  // if we have overshot buf1 boundary
            {
                bytesHandled -= data_sgl[data_index].length; // undo the last increment in bytesHandled
                data_sgl[data_index].length = bytes_per_page - bytesHandled; // set up the length correctly
                bytesHandled += data_sgl[data_index].length; // now increment bytesHandled with the correct value
                skipMeta = TRUE32;
            }
            dataChannelBytesLeft -= data_sgl[data_index].length;
            data_index++;

        }

        if (!skipMeta)
        {
            if (bytesHandled != bytes_per_page)
            {
                meta_sgl[meta_index].paddr  = data_sgl[data_index - 1].paddr + data_sgl[data_index - 1].length;
                meta_sgl[meta_index].length = WMR_MIN(H2FMI_MAX_META_PER_ENVELOPE, metaChannelBytesLeft);
            }
            else // transitioning from buf1 to buf2
            {
                meta_sgl[meta_index].paddr = (UInt32)spare_buf;
                if (skipData)
                {
                    // meta data from one envelope got split into the end of buf1 and beginning of buf2
                    // here we making sure that begining of buf2 gets the remaining meta data of that envelope
                    // setting length to the remaining meta data in that envelope
                    meta_sgl[meta_index].length = WMR_MIN(H2FMI_MAX_META_PER_ENVELOPE, metaChannelBytesLeft + meta_sgl[meta_index - 1].length) - meta_sgl[meta_index - 1].length;
                }
                else
                {
                    // this happens when buf1 boundary coincides with the end of data from the envelope i.e. no splitting happens
                    meta_sgl[meta_index].length = WMR_MIN(H2FMI_MAX_META_PER_ENVELOPE, metaChannelBytesLeft);
                }
                inBuf1 = FALSE32;
            }
            if (skipData)
            {
                skipData = FALSE32;
            }

            bytesHandled += meta_sgl[meta_index].length;
            if (inBuf1 && bytesHandled > bytes_per_page)  // if we have overshot buf1 boundary
            {
                bytesHandled -= meta_sgl[meta_index].length; // undo the last increment in bytesHandled
                meta_sgl[meta_index].length = bytes_per_page - bytesHandled; // set up the length correctly
                bytesHandled += meta_sgl[meta_index].length; // now increment bytesHandled with the correct value
                skipData = TRUE32;
            }
            metaChannelBytesLeft -= meta_sgl[meta_index].length;
            meta_index++;
        }
    }
}
#endif //FMI_VERSION == 0

void h2fmi_prepare_read(    h2fmi_t* fmi, UInt32 page_idx,
                            h2fmi_ce_t*  chip_enable_array,
                            UInt32*  page_number_array )
{
    const h2fmi_ce_t chip_enable = chip_enable_array[page_idx];
    const UInt32 page_number = page_number_array[page_idx];

    h2fmi_fmc_enable_ce(fmi, chip_enable);

    // start a nand page read operation, but don't wait for ready
    h2fmi_read_page_start(fmi, page_number);

    fmi->failureDetails.wNumCE_Executed++;
    fmi->stateMachine.currentCE = chip_enable;
}

void _nandCmd(h2fmi_t* fmi, UInt8 cmd)
{
    h2fmi_wr(fmi, FMC_CMD, FMC_CMD__CMD1(cmd));

    h2fmi_wr(fmi, FMC_RW_CTRL, FMC_RW_CTRL__CMD1_MODE);

    h2fmi_busy_wait(fmi, FMC_STATUS, FMC_STATUS__CMD1DONE, FMC_STATUS__CMD1DONE);
    h2fmi_wr(fmi, FMC_STATUS, FMC_STATUS__CMD1DONE);
}

void _nandAddrSingleCycle(h2fmi_t* fmi, UInt8 addr)
{
    h2fmi_wr(fmi, FMC_ADDR0, FMC_ADDR0__SEQ0(addr));

    h2fmi_wr(fmi, FMC_ADDRNUM, FMC_ADDRNUM__NUM(0));

    h2fmi_wr(fmi, FMC_RW_CTRL, FMC_RW_CTRL__ADDR_MODE);
    
    h2fmi_busy_wait(fmi, FMC_STATUS, FMC_STATUS__ADDRESSDONE, FMC_STATUS__ADDRESSDONE);
    h2fmi_wr(fmi, FMC_STATUS, FMC_STATUS__ADDRESSDONE);
}

#if (defined (ENABLE_VENDOR_UNIQUE_QUERIES) && ENABLE_VENDOR_UNIQUE_QUERIES)
BOOL32 h2fmiGenericNandReadSequence(h2fmi_t* fmi, UInt8 ce,  GenericReadRequest *genericRead)
{
    UInt8 i;
    BOOL32 result = TRUE32;
    h2fmi_reset(fmi);
    UInt32 fmi_control = (FMI_CONTROL__MODE__READ |
                          FMI_CONTROL__START_BIT);

#if FMI_VERSION == 4
    if (fmi->read_stream_disable)
    {
        fmi_control |= FMI_CONTROL__DISABLE_STREAMING;
    }
#endif // FMI_VERSION == 4

    h2fmi_nand_reset(fmi, ce);
    h2fmi_fmc_enable_ce(fmi, ce);
    if (!h2fmi_wait_status(fmi,
                           NAND_STATUS__DATA_CACHE_RB,
                           NAND_STATUS__DATA_CACHE_RB_READY,
                           NULL))
    {
        h2fmi_fail(result);
        h2fmi_fmc_disable_ce(fmi, ce);
        return result;
    }

    if (genericRead->arrCmd != NULL)
    {
        for (i=0; i<genericRead->cmdSize; i++)
        {
            _nandCmd(fmi, genericRead->arrCmd[i]);
        }
    }
    if (genericRead->arrAddr != NULL)
    {
        for (i=0; i<genericRead->addrSize; i++)
        {
            _nandAddrSingleCycle(fmi, genericRead->arrAddr[i]);
        }
    }
    if (genericRead->arrConfirmCmd != NULL)
    {
        for (i=0; i<genericRead->confirmCmdSize; i++)
        {
            _nandCmd(fmi, genericRead->arrConfirmCmd[i]);
        }
    }

    if (!h2fmi_complete_read_page_start(fmi))
    {
        h2fmi_fail(result);
    }
    else
    {
#if FMI_VERSION == 0
        h2fmi_wr(fmi, FMI_CONFIG, (FMI_CONFIG__LBA_MODE__BYPASS_ECC |
                               FMI_CONFIG__META_PER_ENVELOPE(0) |
                               FMI_CONFIG__META_PER_PAGE(0) |
                               FMI_CONFIG__PAGE_SIZE__512));
#else
        h2fmi_wr(fmi, FMI_CONFIG, FMI_CONFIG__ECC_CORRECTABLE_BITS( 0 ));
        h2fmi_wr(fmi, FMI_DATA_SIZE, (FMI_DATA_SIZE__META_BYTES_PER_SECTOR(0) |
                                  FMI_DATA_SIZE__META_BYTES_PER_PAGE(0) |
                                  FMI_DATA_SIZE__BYTES_PER_SECTOR(genericRead->bytesToRead) |
                                  FMI_DATA_SIZE__SECTORS_PER_PAGE(1)));    
#endif
        h2fmi_wr(fmi, FMI_CONTROL, fmi_control);

        if (!h2fmi_wait_done(fmi, FMI_INT_PEND, FMI_INT_PEND__LAST_FMC_DONE, FMI_INT_PEND__LAST_FMC_DONE))
        {
            h2fmi_fail(result);
        }
        else
        {
            result = h2fmi_pio_read_sector(fmi, genericRead->buf, genericRead->bytesToRead);
        }
    }
    h2fmi_fmc_disable_ce(fmi, ce);
    h2fmi_clear_interrupts_and_reset_masks(fmi);
    h2fmi_nand_reset(fmi, ce);
    h2fmi_fmc_enable_ce(fmi, ce);
    if (!h2fmi_wait_status(fmi,
                           NAND_STATUS__DATA_CACHE_RB,
                           NAND_STATUS__DATA_CACHE_RB_READY,
                           NULL))
    {
        h2fmi_fail(result);
    }

    h2fmi_fmc_disable_ce(fmi, ce);
    return result;
}
#endif //(defined (ENABLE_VENDOR_UNIQUE_QUERIES) && ENABLE_VENDOR_UNIQUE_QUERIES)


#if !FMISS_ENABLED

UInt32 h2fmi_read_multi(h2fmi_t* fmi,
                        UInt32   page_count,
                        h2fmi_ce_t*  chip_enable_array,
                        UInt32*  page_number_array,
                        struct dma_segment*    data_segment_array,
                        struct dma_segment*    meta_segment_array,
                        UInt8*   max_corrected_array,
                        UInt8*   sector_stats)
{
    BOOL32 result = TRUE32;
    UInt32 clean_pages = 0;
    UInt32 fused_pages = 0;

#if ( H2FMI_INSTRUMENT_BUS_1 )
    h2fmi_instrument_bit_set(0);
    // TODO: figure out why this PIO-specific workaround seems to
    // also be needed for DMA-based operation
    h2fmi_instrument_bit_clear(0);
    h2fmi_instrument_bit_set(0);
#endif

    WMR_ENTER_CRITICAL_SECTION();
    {
        fmi->stateMachine.chip_enable_array   = chip_enable_array;
        fmi->stateMachine.page_number_array   = page_number_array;
        fmi->stateMachine.data_segment_array  = data_segment_array;
        fmi->stateMachine.meta_segment_array  = meta_segment_array;
        fmi->stateMachine.max_corrected_array = max_corrected_array;
        fmi->stateMachine.sector_stats = sector_stats;
        fmi->stateMachine.clean_pages  = 0;
        fmi->stateMachine.uecc_pages   = 0;
        fmi->stateMachine.fused_pages  = 0;
        fmi->stateMachine.state.rd = readIdle;
        fmi->stateMachine.page_count   = page_count;

        h2fmi_reset(fmi);

        fmi->stateMachine.currentMode  = fmiRead;
    }
    WMR_EXIT_CRITICAL_SECTION();

    h2fmi_reset_ecc(fmi);

#if ( H2FMI_INSTRUMENT_BUS_1 )
    h2fmi_instrument_bit_set(1);
#endif
#if (H2FMI_WAIT_USING_ISR)
    static const utime_t clockDelay_uSec = 50000; // every 50 mSec
#endif
    volatile READ_STATE* pState = &fmi->stateMachine.state.rd;
#if (H2FMI_WAIT_USING_ISR)
    event_unsignal(&fmi->stateMachine.stateMachineDoneEvent);
#endif
    do
    {
        if ( readDone == (*pState) )
        {
            break;
        }
        WMR_ENTER_CRITICAL_SECTION();
        h2fmi_handle_read_ISR_state_machine(fmi);
        WMR_EXIT_CRITICAL_SECTION();

#if !(H2FMI_WAIT_USING_ISR)
        WMR_YIELD();
    }
    while ( readDone != (*pState) );
#else
    }
    while ( !event_wait_timeout(&fmi->stateMachine.stateMachineDoneEvent, clockDelay_uSec) );
/**
 * Allow for early wakeup
 */
    {
        UInt32 iLoopCount = 0;
        while ( readDone != (*pState) )
        {
            WMR_YIELD();
            if ( ++iLoopCount>H2FMI_EARLY_EXIT_ARBITRARY_LOOP_CONST )
            {
                iLoopCount = 0;
                /**
                 * Allow the state machine a timeslice in case the HW gets 
                 * stuck. 
                 */
                WMR_ENTER_CRITICAL_SECTION();
                h2fmi_handle_read_ISR_state_machine(fmi);
                WMR_EXIT_CRITICAL_SECTION();
            }
        }
    }
#endif
#if ( H2FMI_INSTRUMENT_BUS_1 )
    h2fmi_instrument_bit_clear(1);
#endif

    /**
     * Finally check the outcome from our state machine
     */
    result = fmi->stateMachine.fSuccessful;

    if ( result )
    {
        /**
         * ensure that system DMA is complete -- note that if we already 
         * are in an error state we skip this test as it adds an 
         * addition H2FMI_PAGE_TIMEOUT_MICROS to the overall operation.
         */
        // both dma streams should be complete now
        if (!h2fmi_dma_wait(h2fmi_dma_data_chan(fmi), H2FMI_PAGE_TIMEOUT_MICROS) ||
            !h2fmi_dma_wait(h2fmi_dma_meta_chan(fmi), H2FMI_PAGE_TIMEOUT_MICROS))
        {
            h2fmi_fail(result);

            WMR_PANIC("Timeout waiting for CDMA during successful NAND read operation");

            h2fmi_dma_cancel(h2fmi_dma_data_chan(fmi));
            h2fmi_dma_cancel(h2fmi_dma_meta_chan(fmi));

            fmi->failureDetails.wOverallOperationFailure = _kIOPFMI_STATUS_DMA_DONE_TIMEOUT;
        }
        else
        {
            fmi->failureDetails.wOverallOperationFailure = _kIOPFMI_STATUS_SUCCESS;
        }
    }
    else
    {
        h2fmi_dma_cancel(h2fmi_dma_data_chan(fmi));
        h2fmi_dma_cancel(h2fmi_dma_meta_chan(fmi));

        // Whoever set the FSM fSuccessful flag to FALSE is responsible
        // for telling Darwin why.
        WMR_ASSERT(fmi->failureDetails.wOverallOperationFailure != _kIOPFMI_STATUS_UNKNOWN);
    }

    /**
     * Reset our current mode so that interrupts are not re-directed
     * to our state machine.
     */
    fmi->stateMachine.currentMode = fmiNone;

    clean_pages = fmi->stateMachine.clean_pages;
    fused_pages = fmi->stateMachine.fused_pages;

    if (fmi->failureDetails.wOverallOperationFailure == _kIOPFMI_STATUS_SUCCESS)
    {
        // Only bother to check for ECC/clean/etc pages if we didn't get any sort of hardware error
        if( clean_pages > 0 )
        {
            // We have at least one clean page.  If ALL are clean, return a clean result.
            // Otherwise (if there was a mix of clean and non-clean pages), treat this like
            // an uncorrectable ECC error.
            fmi->failureDetails.wOverallOperationFailure = (clean_pages < page_count) ? _kIOPFMI_STATUS_NOT_ALL_CLEAN : _kIOPFMI_STATUS_BLANK;
        }
        else if (fused_pages)
        {
            fmi->failureDetails.wOverallOperationFailure = (fused_pages < page_count) ? _kIOPFMI_STATUS_AT_LEAST_ONE_UECC : _kIOPFMI_STATUS_FUSED;
        }
        else if (fmi->stateMachine.uecc_pages > 0)
        {
            result = FALSE32;
            fmi->failureDetails.wOverallOperationFailure = _kIOPFMI_STATUS_AT_LEAST_ONE_UECC;
        }
    }

#if ( H2FMI_INSTRUMENT_BUS_1 )
    h2fmi_instrument_bit_clear(0);
#endif

    h2fmi_clear_interrupts_and_reset_masks(fmi);

    return fmi->failureDetails.wOverallOperationFailure;
}

#endif /* !FMISS_ENABLED */

// =============================================================================
// static implementation function definitions
// =============================================================================

static void h2fmi_read_page_start(h2fmi_t* fmi,
                                  UInt32   page)
{
    const UInt32 cmd1_addr_cmd2_done = (FMC_STATUS__ADDRESSDONE |
                                        FMC_STATUS__CMD2DONE |
                                        FMC_STATUS__CMD1DONE);

    // configure FMC for cmd1/addr/cmd2 sequence
    h2fmi_config_page_addr(fmi, page);
    h2fmi_wr(fmi, FMC_CMD, (FMC_CMD__CMD2(NAND_CMD__READ_CONFIRM) |
                            FMC_CMD__CMD1(NAND_CMD__READ)));

    // start cmd1/addr/cmd2 sequence
    h2fmi_wr(fmi, FMC_RW_CTRL, (FMC_RW_CTRL__ADDR_MODE |
                                FMC_RW_CTRL__CMD2_MODE |
                                FMC_RW_CTRL__CMD1_MODE));

    // busy wait until cmd1/addr/cmd2 sequence completed; this should
    // be very short and should always complete, so no timeout or
    // yielding is being done to improve performance
    h2fmi_busy_wait(fmi, FMC_STATUS, cmd1_addr_cmd2_done, cmd1_addr_cmd2_done);
    h2fmi_wr(fmi, FMC_STATUS, cmd1_addr_cmd2_done);    
}

static BOOL32 h2fmi_complete_read_page_start(h2fmi_t* fmi)
{

    BOOL32 result = TRUE32;

    // wait for ready using Read Status command
    if (!h2fmi_wait_status(fmi,
                           NAND_STATUS__DATA_CACHE_RB,
                           NAND_STATUS__DATA_CACHE_RB_READY,
                           NULL))
    {
        h2fmi_fail(result);
    }
    else
    {
        // reissue cmd1
        h2fmi_wr(fmi, FMC_CMD, FMC_CMD__CMD1(NAND_CMD__READ));
        h2fmi_wr(fmi, FMC_RW_CTRL, 0); // An intermediate state is needed to meet tRHW
        h2fmi_wr(fmi, FMC_RW_CTRL, FMC_RW_CTRL__CMD1_MODE);

        // busy wait until completed
        h2fmi_busy_wait(fmi, FMC_STATUS, FMC_STATUS__CMD1DONE, FMC_STATUS__CMD1DONE);
        h2fmi_wr(fmi, FMC_STATUS, FMC_STATUS__CMD1DONE);        
    }

    return result;
}
// =============================================================================
// pio-only static implementation function definitions
// =============================================================================

static UInt32 h2fmi_dma_rx_complete(h2fmi_t* fmi,
                                    UInt32   ecc_pnd,
                                    UInt8*   max_corrected,
                                    UInt8*   sector_stats)
{
    return h2fmi_rx_check_page_ecc(fmi, ecc_pnd, max_corrected, sector_stats, fmi->sectors_per_page);
}

UInt32 h2fmi_rx_check_page_ecc(h2fmi_t* fmi,
                                      UInt32   ecc_pend,
                                      UInt8*   max_corrected,
                                      UInt8*   sector_stats,
                                      UInt32   sectors_per_page)
{
#if FMI_VERSION == 0
    // S5L8920X
    UInt32 status = _kIOPFMI_STATUS_SUCCESS;
    BOOL32 clean  = FALSE32;
    UInt32 sector_idx;

    if (ecc_pend & ECC_PND__BCH_ALL_FF)
    {
#if (defined (AND_COLLECT_FIL_STATISTICS) && AND_COLLECT_FIL_STATISTICS)
        stFILStatistics.ddwReadCleanCnt++;
#endif
        // All pages are clean.  Set all sector stats to 0xFE to tell the client.
        if( sector_stats )
        {           
            WMR_MEMSET(sector_stats, 0xFE, sectors_per_page);
        }       
        
        status = _kIOPFMI_STATUS_BLANK;
        clean = TRUE32;
    }
    else if (ecc_pend & ECC_PND__UNCORRECTABLE)
    {
#if (defined (AND_COLLECT_FIL_STATISTICS) && AND_COLLECT_FIL_STATISTICS)
        stFILStatistics.ddwReadECCErrorCnt++;
#endif
        status = _kIOPFMI_STATUS_AT_LEAST_ONE_UECC;
    }

    if (!clean && max_corrected)
    {
        // Page is not entirely clean and the client is interested in the
        // max number of corrected bits/sector.  Pull per-sector ECC stats
        // from the ECC FIFO.
        for (sector_idx = 0; sector_idx < sectors_per_page; sector_idx++)
        {
            // collect ECC corrected bits results for this sector
            const UInt32 ecc_result = h2fmi_rd(fmi, ECC_RESULT);
            const UInt8 error_cnt   = (ecc_result >> 16) & 0x1F;

#if (defined (AND_COLLECT_FIL_STATISTICS) && AND_COLLECT_FIL_STATISTICS)
            stFILStatistics.addwReadBitFlipsCnt[error_cnt]++;
#endif
            if( error_cnt > *max_corrected )
            {
                *max_corrected = error_cnt;
            }

            if( sector_stats )
            {
                if( ecc_result & ECC_RESULT__ERROR_FLAG )
                {
                    // UECC Error
                    sector_stats[ sector_idx ] = 0xFF;
                }
                else
                {
                    sector_stats[ sector_idx ] = error_cnt;
                }
            }
        }
    }

    return status;
#else
    // S5L8922X and later
    UInt32 sector_idx;
    BOOL32 first_sector_good = FALSE32;
    UInt8 is_clean_count = 0;
    UInt8 is_clean_with_stuck_count = 0;
    UInt8 is_fused_count = 0;
    UInt8 is_uecc_count = 0;
    UInt32 result = _kIOPFMI_STATUS_SUCCESS;
    h2fmi_sector_status status;

    if (max_corrected)
    {
        *max_corrected = (ecc_pend & ECC_PND__MAX_ERROR_CNT_MASK) >> ECC_PND__MAX_ERROR_CNT_SHIFT;
    }

    for (sector_idx = 0; sector_idx < sectors_per_page; sector_idx++)
    {
        status = h2fmi_rx_check_sector_ecc(fmi);
        if (sector_stats)
        {
            sector_stats[sector_idx] = status.error_cnt;
        }

        if (status.status == kh2fmi_sector_status_fused)
        {
            // This sector was all zeroes, but we'll only consider it truely
            // fused if all sectors are zero.
            is_fused_count++;
        }
        else if (status.status == kh2fmi_sector_status_clean)
        {
            // This sector was clean, but we'll only set is_clean for real
            // a little later if all sectors in this page were clean.
            is_clean_count++;
        }
        else if (status.status == kh2fmi_sector_status_clean_with_stuck)
        {
            is_clean_with_stuck_count++;
        }
        else if (status.status == kh2fmi_sector_status_uecc)
        {
            is_uecc_count++;
        }
        else if (0 == sector_idx)
        {
            first_sector_good = TRUE32;
        }
    }

#if FMI_VERSION < 3
    // With H4P the PAUSE_ECC bit does not need to be cleared
    // as it only take affect when the ECC_RESULT FIFO is full.
    if ((fmiRead == fmi->stateMachine.currentMode) && (fmi->stateMachine.page_idx < fmi->stateMachine.page_count))
    {
        // Clear the PAUSE_ECC bit for all but the last page.  It should have already been cleared
        // when gathering ECC results for the previous page. Also, unnecessarily clearing it here causes
        // a race with FMI trying to go to idle mode during the read-modify-write.
        h2fmi_wr(fmi, FMI_CONTROL, h2fmi_rd(fmi, FMI_CONTROL) & ~FMI_CONTROL__ECC_PAUSE);
    }

    // Since 0xFF sectors generate 0xFF ECC, sector 0 indicates if the page is expected to be clean.
    if (first_sector_good)
    {
        is_clean_count += is_clean_with_stuck_count;
    }
    else
#endif // FMI_VERSION < 3
    {
        is_uecc_count += is_clean_with_stuck_count;
    }

    if (is_clean_count == sectors_per_page)
    {
        // All sectors in this page were clean; mark the whole page as clean
        result = _kIOPFMI_STATUS_BLANK;
    }
    else if (is_fused_count == sectors_per_page)
    {
        // All sectors in this page were zeroes; mark the whole page as fused and treat
        // it like an uncorrectable ECC error
        result = _kIOPFMI_STATUS_FUSED;
    }
    else if (0 != is_uecc_count)
    {
        result = _kIOPFMI_STATUS_AT_LEAST_ONE_UECC;
    }    

    return result;
#endif // SUB_PLATFORM_S5L8922X
}

#if FMI_VERSION > 0
static h2fmi_sector_status  h2fmi_rx_check_sector_ecc(h2fmi_t* fmi)
{
    const UInt32        ecc_result = h2fmi_rd(fmi, ECC_RESULT);
    const BOOL32        all_ff     = (ecc_result & ECC_RESULT__FREE_PAGE) ? TRUE32 : FALSE32;
    const BOOL32        all_zeros  = (ecc_result & ECC_RESULT__ALL_ZERO) ? TRUE32 : FALSE32;
    const BOOL32        uecc       = (ecc_result & ECC_RESULT__UNCORRECTABLE) ? TRUE32 : FALSE32;
    const UInt32        stuck_bits_shifted = (ecc_result & ECC_RESULT__STUCK_BIT_CNT(~0));
    h2fmi_sector_status ret;

    if (all_ff)
    {
        if (ECC_RESULT__STUCK_BIT_CNT(H2FMI_ALLOWED_STUCK_BITS_IN_FP) < stuck_bits_shifted)
        {
            ret.status = kh2fmi_sector_status_clean_with_stuck;
        }
        else
        {
            ret.status = kh2fmi_sector_status_clean;
        }
        ret.error_cnt = 0xFE;
    }
    else if (all_zeros)
    {
        ret.status    = kh2fmi_sector_status_fused;
        ret.error_cnt = 0xFF;
    }
    else if (uecc)
    {
        ret.status    = kh2fmi_sector_status_uecc;
        ret.error_cnt = 0xFF;
    }
    else
    {
        ret.status = kh2fmi_sector_status_normal;
        ret.error_cnt =  (ecc_result & ECC_RESULT__ERROR_CNT_MASK) >> ECC_RESULT__ERROR_CNT_SHIFT;
    }

    // We'll check for per-page UECC in ECC_PND, not per-sector from the FIFO to save some time
    return ret;
}
#endif

UInt32 h2fmi_read_page(h2fmi_t* fmi,
                       h2fmi_ce_t ce,
                       UInt32   page,
                       UInt8*   data_buf,
                       UInt8*   meta_buf,
                       UInt8*   max_corrected,
                       UInt8*   sector_stats)
{
    struct dma_segment data_sgl;
    struct dma_segment meta_sgl;

    data_sgl.paddr  = (UInt32)data_buf;
    data_sgl.length = fmi->bytes_per_page;
    meta_sgl.paddr  = (UInt32)meta_buf;
    meta_sgl.length = fmi->valid_bytes_per_meta;

    UInt32 result = h2fmi_read_multi(fmi, 1, &ce, &page, &data_sgl, &meta_sgl, max_corrected, sector_stats);

    return result;
}

static void h2fmi_ISR_cleanup_ECC( h2fmi_t* fmi, UInt32 ecc_pnd )
{
    FMI_ISR_STATE_MACHINE* fsm = &fmi->stateMachine;
    UInt8 max_corrected = 0;
    const UInt32 currentIndex = fsm->page_idx - 1;
    UInt32 status;

    // Collect ECC stats from the previous page while we're DMAing the
    // current one.
    status = h2fmi_dma_rx_complete(fmi,
                               ecc_pnd,
                               &max_corrected,
                                   fsm->sector_stats);

#if defined(SIMULATE_READ_FAILURES)
    if ( ++ulFailureCount>=READ_FAILURE_MAGIC_NUMBER )
    {
        /**
         * Don't 'mask' real read errors ... only muck with 'successful'
         * reads. 
         */
        if ( _kIOPFMI_STATUS_SUCCESS == status )
        {
            ulFailureCount = 0;
            status = READ_FAILURE_STATUS_TYPE;

            {
                ulSimulatedFailingCE = fsm->chip_enable_array[ currentIndex ];
                ulSimulatedFailingPage = fsm->page_number_array[ currentIndex ];
            }
        }
    }
    else
    {
#ifdef STICKY_READ_FAILURES
        if ( (ulSimulatedFailingCE == fsm->chip_enable_array[ currentIndex ])
             &&(ulSimulatedFailingPage == fsm->page_number_array[ currentIndex ]) )
        {
            status = READ_FAILURE_STATUS_TYPE;
        }
#endif
    }
#endif
    switch (status)
    {
        case _kIOPFMI_STATUS_AT_LEAST_ONE_UECC:
        fsm->uecc_pages++;
            break;
        case _kIOPFMI_STATUS_BLANK:
            fsm->clean_pages++;
            break;
        case _kIOPFMI_STATUS_FUSED:
            fsm->fused_pages++;
            break;
    }

    if ( (_kIOPFMI_STATUS_AT_LEAST_ONE_UECC==status)
         ||(_kIOPFMI_STATUS_BLANK==status)
         ||(_kIOPFMI_STATUS_FUSED==status) )
    {
        if ( ((UInt32) ~0) == fmi->failureDetails.wFirstFailingCE )
        {
            fmi->failureDetails.wFirstFailingCE = fsm->chip_enable_array[ currentIndex ];
        }
    }
    if (fsm->max_corrected_array)
    {
        fsm->max_corrected_array[fsm->page_idx - 1] = max_corrected;
    }

    if (fsm->sector_stats)
    {
        fsm->sector_stats += fmi->sectors_per_page;
    }
}

static void h2fmi_reset_ecc(h2fmi_t* fmi)
{
    // Reset the ECC_RESULT FIFO
    // clear any results fifo by setting the error flag
#if FMI_VERSION == 0
    h2fmi_wr(fmi, ECC_RESULT, ECC_RESULT__ERROR_FLAG);
#else
    h2fmi_wr(fmi, ECC_RESULT, ECC_RESULT__FIFO_RESET);
#endif

    // clean status bits by setting them
    h2fmi_clean_ecc(fmi);
}


static void h2fmi_read_done_handler( h2fmi_t* fmi )
{
    h2fmi_clear_interrupts_and_reset_masks(fmi);
    h2fmi_fmc_disable_all_ces( fmi );
#if (H2FMI_WAIT_USING_ISR)
    event_signal( &fmi->stateMachine.stateMachineDoneEvent );
#endif
}

static void h2fmi_read_setup_handler( h2fmi_t* fmi );
static void h2fmi_read_start_wait_handler( h2fmi_t* fmi );

static void h2fmi_read_wait_ECC_done_handler( h2fmi_t* fmi )
{
    // Wait for fmc tasks pending to be two, indicating that FMI
    // buffers are available -- prevents us from re-programming FMI
    // HW before it has completely finished.
    if ((0 != ( h2fmi_rd(fmi, FMI_STATUS) & (FMI_STATUS__ECC_ACTIVE))))
    {
        BOOL32 fTimeout = WMR_HAS_TIME_ELAPSED_TICKS(fmi->stateMachine.waitStartTicks, fmi->stateMachine.wQuarterPageTimeoutTicks);

        if ( fTimeout )
        {
            fmi->stateMachine.fSuccessful = FALSE32;
            fmi->failureDetails.wOverallOperationFailure = _kIOPFMI_STATUS_ECC_DONE_TIMEOUT;
            // We failed to get what we wanted -- goto error state and complete.
            fmi->stateMachine.state.rd = readDone;
            h2fmi_read_done_handler(fmi);
        }

    }
    else
    {
        if (fmi->stateMachine.page_idx >= fmi->stateMachine.page_count)
        {
            const UInt32 ecc_pnd = h2fmi_rd(fmi, ECC_PND);
            h2fmi_wr(fmi, ECC_PND, ecc_pnd);

            h2fmi_ISR_cleanup_ECC(fmi, ecc_pnd);
            fmi->stateMachine.state.rd = readDone;
            h2fmi_read_done_handler(fmi);
        }
        else
        {
            h2fmi_prepare_for_ready_busy_interrupt(fmi);

            fmi->stateMachine.waitStartTicks = WMR_CLOCK_TICKS();
            fmi->stateMachine.state.rd = readStartWait;

            h2fmi_read_start_wait_handler(fmi);
        }
    }
}

static void h2fmi_read_Xfer_data_handler( h2fmi_t* fmi )
{
    fmi->isr_condition = h2fmi_rd(fmi, FMI_INT_PEND);
    if (FMI_INT_PEND__LAST_FMC_DONE != (fmi->isr_condition & FMI_INT_PEND__LAST_FMC_DONE))
    {
        BOOL32 fTimeout = WMR_HAS_TIME_ELAPSED_TICKS( fmi->stateMachine.waitStartTicks, fmi->stateMachine.wPageTimeoutTicks);

        if ( fTimeout )
        {
            fmi->stateMachine.fSuccessful = FALSE32;
            fmi->failureDetails.wOverallOperationFailure = _kIOPFMI_STATUS_FMC_DONE_TIMEOUT;
            // We failed to get what we wanted -- goto error state and complete.
            fmi->stateMachine.state.rd = readDone;
            h2fmi_read_done_handler(fmi);
        }
    }
    else
    {
        h2fmi_wr(fmi, FMI_INT_EN, 0);
        fmi->stateMachine.page_idx++;
        fmi->stateMachine.state.rd = readSetup;
        h2fmi_read_setup_handler( fmi );
    }
}

static void h2fmi_read_start_wait_handler( h2fmi_t* fmi )
{
    const UInt32   page_idx = fmi->stateMachine.page_idx;
    CE_STATE ceState = h2fmi_get_current_CE_state(fmi);
    UInt32 fmi_control = (FMI_CONTROL__MODE__READ |
                          FMI_CONTROL__START_BIT);

#if FMI_VERSION == 4
    if (fmi->read_stream_disable)
    {
        fmi_control |= FMI_CONTROL__DISABLE_STREAMING;
    }
#endif // FMI_VERSION == 4

    if ( CE_TIMEOUT == ceState )
    {
        h2fmi_wr(fmi, FMC_RW_CTRL, 0); // Turn off RE if we timeout.
        h2fmi_fmc_disable_all_ces( fmi );
        fmi->stateMachine.fSuccessful = FALSE32;
        fmi->failureDetails.wOverallOperationFailure = _kIOPFMI_STATUS_READY_BUSY_TIMEOUT;
        fmi->stateMachine.state.rd = readDone;
        h2fmi_read_done_handler(fmi);
    }
    else if ( CE_BUSY != ceState )
    {
#if FMI_VERSION == 0
        // Store and clear ECC_PND state before we kick off the next page to eliminate any
        // possible races between software clearing the ECC status and hardware changing it.
        // But to keep performance up, we'll only process this information after we've started
        // the next page.
        const UInt32 ecc_pend = h2fmi_rd(fmi, ECC_PND);
        h2fmi_wr(fmi, ECC_PND, ecc_pend);
#endif

        h2fmi_clear_interrupts_and_reset_masks(fmi);

        // reissue cmd1
        h2fmi_wr(fmi, FMC_CMD, FMC_CMD__CMD1(NAND_CMD__READ));
        h2fmi_wr(fmi, FMC_RW_CTRL, 0); // An intermediate state is needed to meet tRHW
        h2fmi_wr(fmi, FMC_RW_CTRL, FMC_RW_CTRL__CMD1_MODE);

        // busy wait until completed
        h2fmi_busy_wait(fmi, FMC_STATUS, FMC_STATUS__CMD1DONE, FMC_STATUS__CMD1DONE);
        h2fmi_wr(fmi, FMC_STATUS, FMC_STATUS__CMD1DONE);

        fmi->stateMachine.state.rd = readXferData;
        // enable appropriate interrupt source
        h2fmi_wr(fmi, FMI_INT_EN, FMI_INT_EN__LAST_FMC_DONE);

#if FMI_VERSION > 0
        if( page_idx > 0 )
        {
            // Don't let the ECC block do anything for the next page until we've had a chance to pull
            // out the previous page's ECC status
            fmi_control |= FMI_CONTROL__ECC_PAUSE;
        }
#endif

        // start FMI read of next page.  This starts the big delay while the data is actually
        // transferred over the bus.
        h2fmi_wr(fmi, FMI_CONTROL, fmi_control);

        if ( page_idx == 0)
        {
            h2fmi_start_dma(fmi);
        }

        fmi->stateMachine.waitStartTicks = WMR_CLOCK_TICKS();

        if( page_idx > 0 )
        {
#if FMI_VERSION > 0
            UInt32 ecc_pend;

            ecc_pend = h2fmi_rd(fmi, ECC_PND);
            h2fmi_wr(fmi, ECC_PND, ecc_pend);
#endif
            h2fmi_ISR_cleanup_ECC(fmi, ecc_pend);
        }
    }
}

static void h2fmi_read_setup_handler( h2fmi_t* fmi )
{
    BOOL32 fNeedsChipSelect = ( (fmi->stateMachine.page_idx < fmi->stateMachine.page_count) ? TRUE32 : FALSE32 );

    if (fmi->stateMachine.needs_prepare)
    {
        // We'll generally try to overlap reads (by taking care of this while the previous
        // page was being transferred).  But we'll have to setup the read here on the first
        // page and if we get two pages on the same CE in a row.
        fmi->stateMachine.needs_prepare = FALSE32;
        h2fmi_prepare_read(fmi,
                           fmi->stateMachine.page_idx,
                           fmi->stateMachine.chip_enable_array,
                           fmi->stateMachine.page_number_array);
        fNeedsChipSelect = FALSE32;
    }
    // Setup the next page while we're waiting for this one to complete...
    if ( (fmi->stateMachine.page_idx + 1) < fmi->stateMachine.page_count )
    {
        if( fmi->stateMachine.chip_enable_array[ fmi->stateMachine.page_idx + 1 ] == fmi->stateMachine.lastCE )
        {
            // Two consecutive operations on the same CE: it's not safe to
            // prepare the second one yet...
            fmi->stateMachine.needs_prepare = TRUE32;
        }
        else
        {
            fmi->stateMachine.needs_prepare = FALSE32;
            h2fmi_prepare_read(fmi,
                               fmi->stateMachine.page_idx + 1,
                               fmi->stateMachine.chip_enable_array,
                               fmi->stateMachine.page_number_array);
            fNeedsChipSelect = TRUE32;
        }
        fmi->stateMachine.lastCE = fmi->stateMachine.chip_enable_array[ fmi->stateMachine.page_idx + 1 ];
    }
    // We've setup page_idx+1 (if we could).  Time to deal with page_idx again.
    if ( fNeedsChipSelect )
    {
        h2fmi_fmc_enable_ce( fmi, fmi->stateMachine.chip_enable_array[ fmi->stateMachine.page_idx ] );
        fmi->stateMachine.currentCE = fmi->stateMachine.chip_enable_array[ fmi->stateMachine.page_idx ];
    }
    h2fmi_wr(fmi, FMI_INT_EN, FMI_INT_EN__ECC_READY_BUSY);

    fmi->stateMachine.state.rd = readWaitECCDone;
    fmi->stateMachine.waitStartTicks = WMR_CLOCK_TICKS();

    h2fmi_read_wait_ECC_done_handler(fmi);
}

static void h2fmi_read_idle_handler( h2fmi_t* fmi )
{
    /**
     * Called initially from client in non-interrupt context to let
     * us set everything up and start the process.
     */

    fmi->stateMachine.page_idx = 0;
    fmi->stateMachine.fSuccessful = TRUE32;
    fmi->stateMachine.needs_prepare = TRUE32;
    fmi->stateMachine.lastCE   = fmi->stateMachine.chip_enable_array[0];

    BOOL32 bSuccess = h2fmi_common_idle_handler(fmi);

    WMR_ASSERT(bSuccess == TRUE32);

    fmi->stateMachine.state.rd = readSetup;
    h2fmi_read_setup_handler(fmi);
}

typedef void (*h2fmi_isrReadStateHandler)( h2fmi_t* fmi );

static const h2fmi_isrReadStateHandler readStateHandlerTable[] = {
    h2fmi_read_idle_handler,                  // readIdle = 0,
    h2fmi_read_setup_handler,                 // readSetup = 1,
    h2fmi_read_start_wait_handler,             // readStartWait = 2,
    h2fmi_read_Xfer_data_handler,              // readXferData = 3,
    h2fmi_read_wait_ECC_done_handler,           // readWaitECCDone = 4,
    h2fmi_read_done_handler,                  // readDone = 5,
};

void h2fmi_handle_read_ISR_state_machine( h2fmi_t* fmi )
{
    WMR_ASSERT(fmi->stateMachine.currentMode == fmiRead);
    WMR_ASSERT(fmi->stateMachine.state.rd <= readDone);

#if ( H2FMI_INSTRUMENT_BUS_1 )
    {
        int k = (int)fmi->stateMachine.state.rd;
        h2fmi_instrument_bit_set(2);
        while (k-- > 0)
        {
            h2fmi_instrument_bit_clear(2);
            h2fmi_instrument_bit_set(2);
        }
    }
#endif

    readStateHandlerTable[ fmi->stateMachine.state.rd ]( fmi );

#if ( H2FMI_INSTRUMENT_BUS_1 )
    h2fmi_instrument_bit_clear(2);
#endif
}

// ********************************** EOF **************************************
