// *****************************************************************************
//
// File: H2fmi_ppn.c
//
// Copyright (C) 2009 Apple Inc. All rights reserved.
//
// This document is the property of Apple Computer, Inc.
// It is considered confidential and proprietary.
//
// This document may not be reproduced or transmitted in any form,
// in whole or in part, without the express written permission of
// Apple Inc.
//
// *****************************************************************************

#include "H2fmi_private.h"
#include "H2fmi_dma.h"
#include "H2fmi_ppn.h"
#include <FIL.h>
#include <FILTypes.h>
#include "WMRFeatures.h"

#if WMR_BUILDING_IBOOT
#include <sys.h>
#if !APPLICATION_EMBEDDEDIOP
#include "nandid.h"
#endif /* !APPLICATION_EMBEDDEDIOP */
#else
#include "NandSpecTables.h"
#endif /* WMR_BUILDING_IBOOT */

#if FMI_VERSION > 0

#if H2FMI_PPN_VERIFY_SET_FEATURES

typedef struct
{
    UInt16 feature;
    UInt32 states;
} h2fmi_ppn_feature_states_t;

typedef struct
{
    UInt32 count;
    UInt32 state;
    struct
    {
        UInt16 feature;
        UInt32 length;;
        UInt32 value;
    } list[PPN_MAX_FEATURE_COUNT];
} h2fmi_ppn_feature_shadow_t;

static h2fmi_ppn_feature_states_t _ppn_feature_states[] =
{
    { PPN_FEATURE__VREF_ENABLE , (1UL << PPN_FEATURE__POWER_STATE__LOW_POWER) } ,
};

static h2fmi_ppn_feature_shadow_t _ppn_feature_shadow[PPN_MAX_CES_PER_BUS];

#endif // H2FMI_PPN_VERIFY_SET_FEATURES

static void   h2fmi_send_cmd(h2fmi_t *fmi, UInt8 cmd);
static void   h2fmi_send_cmd_addr(h2fmi_t *fmi, UInt8 cmd, UInt8 *address, UInt32 address_bytes);
static void   h2fmi_ppn_set_format(h2fmi_t *fmi, UInt32 sector_len, UInt32 sector_count, BOOL32 bootpage);
static void   h2fmi_ppn_get_page_data(h2fmi_t *fmi, UInt32 length);
static void   h2fmi_send_cmd_addr_cmd(h2fmi_t *fmi, UInt8 cmd1, UInt8 cmd2, const UInt8 *address, UInt32 address_bytes);
static BOOL32 h2fmi_ppn_prep_read(h2fmi_t *fmi, const RowColLenAddressType *page, BOOL32 last);
static void   h2fmi_ppn_prep_write_multi(h2fmi_t *fmi, const PPNCommandStruct *ppnCommand);
static void   h2fmi_ppn_program_page(h2fmi_t *fmi, UInt32 page, BOOL32 lastPage, UInt32 lbas);
static BOOL32 h2fmi_get_nand_status(h2fmi_t *fmi, UInt8 *status);
static void   h2fmi_ppn_read_serial_output(h2fmi_t *fmi, UInt8 *buf, UInt32 len);
static BOOL32 h2fmi_ppn_switch_ce(h2fmi_t *fmi, h2fmi_ce_t ce);
static void   h2fmi_ppn_disable_all_ces(h2fmi_t *fmi);
static void   h2fmi_ppn_start_fmi_write_and_wait(h2fmi_t *fmi);
static void   h2fmi_ppn_start_fmi_read_and_wait(h2fmi_t *fmi);
static BOOL32 h2fmi_ppn_get_next_operation_status_with_addr(h2fmi_t *fmi, UInt8 *addr, UInt8 *operation_status);
static void   h2fmi_prepare_for_fmi_interrupt(h2fmi_t *fmi, UInt32 condition);
static BOOL32 h2fmi_wait_for_fmi_interrupt(h2fmi_t *fmi, UInt32 condition);
static BOOL32 h2fmi_ppn_set_generic_debug_data(h2fmi_t *fmi, h2fmi_ce_t ce, UInt32 flags);
static BOOL32 _validate_device_parameters(h2fmi_ppn_t *ppn);
#if H2FMI_PPN_VERIFY_SET_FEATURES
static void   _update_feature_shadow(h2fmi_t *fmi, UInt16 feature, UInt8 *value, UInt32 len);
#endif // H2FMI_PPN_VERIFY_SET_FEATURES

static h2fmi_ce_t h2fmi_ppn_ce_index_to_physical(const PPNCommandStruct *command, UInt32 index)
{
    return command->ceInfo[index].ce;
}

BOOL32 h2fmi_ppn_get_uid(h2fmi_t *fmi, UInt8 ce, UInt8 die, UInt8 *buf)
{
    UInt8 status;

    h2fmi_reset(fmi);

    return h2fmi_ppn_get_feature(fmi,
                                 ce,
                                 (PPN_FEATURE__NAND_DIE_UNIQUE_ID  | (die << 8)),
                                 buf,
                                 NAND_UID_PPN_BYTES_TO_READ,
                                 &status);
}

Int32 h2fmi_ppn_read_multi(h2fmi_t            *fmi,
                           PPNCommandStruct   *ppnCommand,
                           struct dma_segment *data_segment_array,
                           struct dma_segment *meta_segment_array)
{
    const UInt32                page_count = ppnCommand->num_pages;
    UInt32            prepPage;
    UInt32            readPage;
    BOOL32            ret;
    UInt32            queueDepth;
    const UInt32      readQueueSize = fmi->ppn->device_params.read_queue_size;
    UInt32            overallStatus;
    UInt32            i;
    UInt32            cePageCount[PPN_MAX_CES_PER_BUS] = {0};
    UInt8             setFeaturesBuffer[PPN_FEATURE_LENGTH_ENABLE_BITFLIPS_DATA_COLLECTION];
    UInt32            iopfmiStatus;
    BOOL32            timeout;
    UInt8             opStatus;

    timeout       = FALSE32;
    prepPage      = 0;
    readPage      = 0;
    queueDepth    = 0;
    overallStatus = 0;

    WMR_ASSERT(fmi->activeCe == (h2fmi_ce_t)~0);
    WMR_ASSERT(page_count > 0);

    h2fmi_reset(fmi);

    if(ppnCommand->options & PPN_OPTIONS_GET_PAGE_RMA_INFO)
    {
//        h2fmi_ce_t ce = h2fmi_ppn_ce_index_to_physical(ppnCommand, 0);

        WMR_PANIC("Get RMA data not yet implemented with new PPN_FIL");
//        WMR_PRINT(ALWAYS, "Attempting to pull RMA data for CE %d\n", ce);
//        h2fmi_ppn_force_geb_address(fmi, ce, ppnCommand->pages[0]);
        return  _kIOPFMI_STATUS_PPN_GENERAL_ERROR;
    }

    if(ppnCommand->options & PPN_OPTIONS_REPORT_HEALTH)
    {
        setFeaturesBuffer[0] = 1;    // To enable Bitflips/1KB data collection feature
        setFeaturesBuffer[1] = 0;
        setFeaturesBuffer[2] = 0;
        setFeaturesBuffer[3] = 0;
    }

    for (i = 0; i < PPN_MAX_CES_PER_BUS; i++)
    {
        if (ppnCommand->ceInfo[i].pages > 0)
        {
            h2fmi_ce_t ce = h2fmi_ppn_ce_index_to_physical(ppnCommand, i);
            if(ppnCommand->options & PPN_OPTIONS_REPORT_HEALTH)
            {
                if (FIL_SUCCESS != h2fmi_ppn_set_features(fmi,
                                                          ce,
                                                          PPN_FEATURE__ENABLE_BITFLIPS_DATA_COLLECTION,
                                                          setFeaturesBuffer,
                                                          PPN_FEATURE_LENGTH_ENABLE_BITFLIPS_DATA_COLLECTION,
                                                          FALSE32,
                                                          NULL))
                {
                    WMR_PANIC("Set Features failed on CE index %d (physical %d)", i, ce);
                }
            }
        }
    }

    for( readPage = 0; (readPage < page_count) && !timeout; readPage++)
    {
        const UInt8      readPageCeIndex = ppnCommand->entry[readPage].ceIdx;
        const h2fmi_ce_t readPagePhysCe  = h2fmi_ppn_ce_index_to_physical(ppnCommand,
                                                                          readPageCeIndex);
        UInt32 lba;

        while((queueDepth < readQueueSize) && (prepPage < page_count))
        {
            const UInt8      ceIndex = ppnCommand->entry[prepPage].ceIdx;
            const h2fmi_ce_t physCe  = h2fmi_ppn_ce_index_to_physical(ppnCommand,
                                                                      ceIndex);

            WMR_ASSERT(ceIndex < PPN_MAX_CES_PER_BUS);
            h2fmi_ppn_switch_ce(fmi, physCe);
            ret = h2fmi_ppn_prep_read(fmi,
                                      &ppnCommand->entry[prepPage].addr,
                                      cePageCount[ceIndex] + 1 == ppnCommand->ceInfo[ceIndex].pages);

            WMR_ASSERT( ret == TRUE32);
            cePageCount[ceIndex]++;
            prepPage++;
            queueDepth++;
        }

        if (0 == readPage) {
            h2fmi_dma_execute_async(DMA_CMD_DIR_RX,
                                    h2fmi_dma_data_chan(fmi),
                                    data_segment_array,
                                    h2fmi_dma_data_fifo(fmi),
                                    ppnCommand->lbas * fmi->logical_page_size,
                                    sizeof(UInt32),
                                    H2FMI_DMA_BURST_CYCLES,
                                    fmi->current_aes_cxt);

            h2fmi_dma_execute_async(DMA_CMD_DIR_RX,
                                    h2fmi_dma_meta_chan(fmi),
                                    meta_segment_array,
                                    h2fmi_dma_meta_fifo(fmi),
                                    ppnCommand->lbas * fmi->valid_bytes_per_meta,
#if FMI_VERSION >= 4
                                    sizeof(UInt32),
                                    4,
#else // FMI_VERSION < 4
                                    sizeof(UInt8),
                                    1,
#endif // FMI_VERSION < 4
                                    NULL);
        }

        h2fmi_ppn_switch_ce(fmi, readPagePhysCe);
        ret = h2fmi_ppn_get_next_operation_status(fmi, &opStatus);
        if (!ret)
        {
            ppnCommand->entry[readPage].status = 0;
            timeout = TRUE32;
            break;
        }
        else if (fmi->retire_on_invalid_refresh &&
                 ((PPN_OPERATION_STATUS__REFRESH | PPN_OPERATION_STATUS__ERROR) ==
                  (opStatus & (PPN_OPERATION_STATUS__REFRESH | PPN_OPERATION_STATUS__ERROR))))
        {
            // Remap 0x43 (refresh + invalid) to 0x45 (retire + invalid)
            opStatus = (opStatus & ~(PPN_OPERATION_STATUS__REFRESH)) | PPN_OPERATION_STATUS__RETIRE;
        }

        ppnCommand->entry[readPage].status = opStatus;
        overallStatus |= opStatus;

        h2fmi_send_cmd(fmi, NAND_CMD__READ_SERIAL_OUTPUT);
        for (lba = 0; lba < ppnCommand->entry[readPage].lbas; lba++)
        {
            h2fmi_ppn_get_page_data(fmi, fmi->logical_page_size);
        }
        queueDepth--;
    }

    if (timeout ||
        !h2fmi_dma_wait(h2fmi_dma_data_chan(fmi), H2FMI_PAGE_TIMEOUT_MICROS) ||
        !h2fmi_dma_wait(h2fmi_dma_meta_chan(fmi), H2FMI_PAGE_TIMEOUT_MICROS))
    {
        h2fmi_dma_cancel(h2fmi_dma_meta_chan(fmi));
        h2fmi_dma_cancel(h2fmi_dma_data_chan(fmi));
    }

    for (i = 0; i < PPN_MAX_CES_PER_BUS; i++)
    {
        if (ppnCommand->ceInfo[i].pages > 0)
        {
            h2fmi_ce_t physCe = ppnCommand->ceInfo[i].ce;

            h2fmi_ppn_switch_ce(fmi, physCe);
            h2fmi_send_cmd(fmi, NAND_CMD__GET_NEXT_OPERATION_STATUS);
        }
    }

    h2fmi_ppn_disable_all_ces(fmi);

    ppnCommand->page_status_summary = overallStatus;
    if (timeout)
    {
        iopfmiStatus = _kIOPFMI_STATUS_READY_BUSY_TIMEOUT;
    }
    else if (overallStatus & PPN_OPERATION_STATUS__GENERAL_ERROR)
    {
        iopfmiStatus = _kIOPFMI_STATUS_PPN_GENERAL_ERROR;
        WMR_PRINT(ERROR, "IOP returning kIOPFMI_STATUS_PPN_GENERAL_ERROR\n");
    }
    else
    {
        iopfmiStatus = _kIOPFMI_STATUS_SUCCESS;
    }

    return iopfmiStatus;
}

Int32 h2fmi_ppn_read_bootpage(h2fmi_t  *fmi,
                              UInt16    ce,
                              UInt32    page,
                              UInt8    *buf,
                              UInt8    *max_corrected)
{
    Int32                ret = _kIOPFMI_STATUS_SUCCESS;
    UInt8                operationStatus;
    UInt8                i;
    UInt32               fmi_control = (FMI_CONTROL__MODE__READ |
                                        FMI_CONTROL__START_BIT);

#if SUPPORT_TOGGLE_NAND && !APPLICATION_EMBEDDEDIOP
    if (fmi->is_toggle_system)
    {
        transitionWorldFromDDR(PPN_FEATURE__POWER_STATE__ASYNC);
    }
#endif

#if FMI_VERSION == 4
    if (fmi->read_stream_disable)
    {
        fmi_control |= FMI_CONTROL__DISABLE_STREAMING;
    }
#endif // FMI_VERSION == 4

    WMR_ASSERT(fmi->activeCe == (h2fmi_ce_t)~0);

    h2fmi_ppn_switch_ce(fmi, ce);

    h2fmi_send_cmd_addr_cmd(fmi,
                            NAND_CMD__MULTIPAGE_READ_LAST,
                            NAND_CMD__MULTIPAGE_READ_CONFIRM,
                            (UInt8 *)&page,
                            fmi->ppn->bytes_per_row_address);

    ret = h2fmi_ppn_get_next_operation_status(fmi, &operationStatus);
    if (!ret)
    {
        WMR_PRINT(ERROR, "Timeout reading operation status in Read Bootpage\n");
        ret = _kIOPFMI_STATUS_READY_BUSY_TIMEOUT;
    }
    else if (operationStatus != PPN_OPERATION_STATUS__READY)
    {
        if (operationStatus & PPN_OPERATION_STATUS__GENERAL_ERROR)
        {
            WMR_PRINT(ERROR, "General Error bit asserted during Read Boot Page\n");
            ret = _kIOPFMI_STATUS_PPN_GENERAL_ERROR;
        }
        else if (operationStatus & PPN_OPERATION_STATUS__CLEAN)
        {
            ret = _kIOPFMI_STATUS_BLANK;
        }
        else
        {
            WMR_PRINT(ERROR, "Error 0x%02x bus: %d ce: %d p:0x%08x\n", (UInt32)operationStatus, (UInt32) fmi->bus_id, (UInt32) ce, page);
            ret = _kIOPFMI_STATUS_AT_LEAST_ONE_UECC;
        }
    }

    if (buf != NULL)
    {
        h2fmi_clean_ecc(fmi);
        h2fmi_send_cmd(fmi, NAND_CMD__READ_SERIAL_OUTPUT);

        h2fmi_ppn_set_format(fmi, H2FMI_BYTES_PER_BOOT_SECTOR, 1, TRUE32);
        for (i = 0; i < H2FMI_BOOT_SECTORS_PER_PAGE; i++)
        {
#if FMI_ECC_DEBUG
            h2fmi_wr(fmi, ECC_PND, ~0UL);
#endif // FMI_ECC_DEBUG

#if FMI_VERSION > 3
            h2fmi_wr(fmi, FMI_SCRAMBLER_SEED_FIFO, page + i);
#endif // FMI_VERSION > 3
            h2fmi_wr(fmi, FMI_CONTROL, fmi_control);
            if (!h2fmi_pio_read_sector(fmi, buf, H2FMI_BYTES_PER_BOOT_SECTOR))
            {
#if FMI_ECC_DEBUG
                WMR_PANIC("PIO on bus %lu of sector %lu failed - %lu previous successes\n",
                          fmi->bus_id, i, fmi->ppn->ecc_sectors_read);
#endif // FMI_ECC_DEBUG
                ret = _kIOPFMI_STATUS_DMA_DONE_TIMEOUT;
                break;
            }
            buf += H2FMI_BYTES_PER_BOOT_SECTOR;

#if FMI_ECC_DEBUG
            fmi->ppn->ecc_sectors_read++;
#endif // FMI_ECC_DEBUG
        }

        if (_kIOPFMI_STATUS_SUCCESS == ret)
        {
            ret = h2fmi_rx_check_page_ecc(fmi, 0, NULL, NULL, H2FMI_BOOT_SECTORS_PER_PAGE);
            if (_kIOPFMI_STATUS_SUCCESS != ret)
            {
                WMR_PRINT(ERROR, "PPN status check reported success, but ECC status check reported 0x%08x!\n", ret);
            }
        }
    }
    h2fmi_send_cmd(fmi, NAND_CMD__GET_NEXT_OPERATION_STATUS);

    h2fmi_ppn_disable_all_ces(fmi);
    h2fmi_reset(fmi);

#if SUPPORT_TOGGLE_NAND && !APPLICATION_EMBEDDEDIOP
    if (fmi->is_toggle_system)
    {
        transitionWorldToDDR(PPN_FEATURE__POWER_STATE__ASYNC);
    }
#endif

    return ret;
}

Int32 h2fmi_ppn_read_cau_bbt(h2fmi_t          *fmi,
                            PPNCommandStruct *ppnCommand,
                            UInt8            *buf)
{
    UInt8 *status         = &ppnCommand->entry[0].status;
    const UInt16 ceIndex  = ppnCommand->entry[0].ceIdx;
    const UInt32 pageAddr = ppnCommand->entry[0].addr.row;
    const h2fmi_ppn_device_params_t *dev = &fmi->ppn->device_params;
    const UInt32 cau = (pageAddr >> (dev->page_address_bits + dev->block_bits)) & ((1 << dev->cau_bits) - 1);
    const h2fmi_ce_t physCe = h2fmi_ppn_ce_index_to_physical(ppnCommand, ceIndex);

    WMR_ASSERT(fmi->activeCe == (h2fmi_ce_t)~0);
    WMR_ASSERT(cau < fmi->ppn->device_params.caus_per_channel);

    //WMR_PRINT(ALWAYS, "Read CAU BBT: bus %d, ce %d, CAU 0x%08x\n", fmi->bus_id, physCe, cau);

    h2fmi_ppn_get_feature(fmi,
                          physCe,
                          PPN_FEATURE__BAD_BLOCK_BITMAP_ARRAY | (cau << 8),
                          buf,
                          fmi->ppn->device_params.blocks_per_cau / 8,
                          status);

    ppnCommand->page_status_summary = status[0];

    if (ppnCommand->page_status_summary  & PPN_OPERATION_STATUS__GENERAL_ERROR)
    {
        return _kIOPFMI_STATUS_PPN_GENERAL_ERROR;
    }
    else
    {
        return _kIOPFMI_STATUS_SUCCESS;
    }
}

UInt8 h2fmi_ppn_get_idle_counter(h2fmi_t *fmi,
                                 h2fmi_ce_t ce)
{
    UInt32   counter;
    UInt8    status;

    if (!h2fmi_ppn_get_feature(fmi,
                               ce,
                               PPN_FEATURE__IDLE_COUNTER,
                               (UInt8 *)&counter,
                               4,
                               &status))
    {
        WMR_PANIC("Unexpected status or timeout reading PPN idle counter");
    }

    h2fmi_reset(fmi);

    return counter & 0xFF;
}


#if !(defined(AND_READONLY)) || AND_SUPPORT_NVRAM
Int32 h2fmi_ppn_write_multi(h2fmi_t            *fmi,
                            PPNCommandStruct   *ppnCommand,
                            struct dma_segment *data_segment_array,
                            struct dma_segment *meta_segment_array)
{
    const UInt32  page_count        = ppnCommand->num_pages;
    BOOL32        ret = TRUE32;
    UInt32        pageIndex = 0;
    BOOL32        error = FALSE32;
    h2fmi_ce_t    errorCe;
    UInt8         overallStatus = 0;
    UInt32        cePageCount[PPN_MAX_CES_PER_BUS] = {0};
    BOOL32        timeout = FALSE32;

    WMR_ASSERT(fmi->activeCe == (h2fmi_ce_t)~0);
    WMR_ASSERT(page_count > 0);
    WMR_ASSERT(page_count <= fmi->ppn->device_params.prep_function_buffer_size);

    h2fmi_reset(fmi);

    if (page_count > 1)
    {
        h2fmi_ppn_prep_write_multi(fmi, ppnCommand);
    }

    h2fmi_dma_execute_async(DMA_CMD_DIR_TX,
                            h2fmi_dma_data_chan(fmi),
                            data_segment_array,
                            h2fmi_dma_data_fifo(fmi),
                            ppnCommand->lbas * fmi->logical_page_size,
                            sizeof(UInt32),
                            H2FMI_DMA_BURST_CYCLES,
                            fmi->current_aes_cxt);

    h2fmi_dma_execute_async(DMA_CMD_DIR_TX,
                            h2fmi_dma_meta_chan(fmi),
                            meta_segment_array,
                            h2fmi_dma_meta_fifo(fmi),
                            ppnCommand->lbas * fmi->valid_bytes_per_meta,
#if FMI_VERSION >= 4
                            sizeof(UInt32),
                            4,
#else // FMI_VERSION < 4
                            sizeof(UInt8),
                            1,
#endif // FMI_VERSION < 4
                            NULL);

    for (pageIndex = 0; (ret &&
                         (pageIndex < page_count) &&
                         !timeout &&
                         !error);
         pageIndex++)
    {
        const UInt8      ceIndex = ppnCommand->entry[pageIndex].ceIdx;
        const h2fmi_ce_t physCe  = h2fmi_ppn_ce_index_to_physical(ppnCommand, ceIndex);
        UInt8            status;

        h2fmi_ppn_switch_ce(fmi, physCe);
        h2fmi_ppn_program_page(fmi,
                               ppnCommand->entry[pageIndex].addr.row,
                               cePageCount[ceIndex] + 1 == ppnCommand->ceInfo[ceIndex].pages,
                               ppnCommand->entry[pageIndex].lbas);

        ret = h2fmi_ppn_get_controller_status(fmi, &status);
        if (!ret)
        {
            // Don't look at op status at all if we timed out - it could have bogus bits set.
            status = 0;
        }

        // Always mark the program as good here - if there was a program error or if the page was
        // kicked off but not completed, we'll fix up the status when we pull the program error
        // lists.
        ppnCommand->entry[pageIndex].status = PPN_CONTROLLER_STATUS__READY;
        overallStatus |= status;

        if (!ret)
        {
            WMR_PRINT(ERROR, "Timeout waiting for controller status after multi write\n");
            timeout = TRUE32;
        }
        else if (status & PPN_CONTROLLER_STATUS__PENDING_ERRORS)
        {
            error   = TRUE32;
            errorCe = ceIndex;
            WMR_PRINT(ERROR, "Bailing out due to program error\n");
        }
        else if (status & PPN_CONTROLLER_STATUS__GENERAL_ERROR)
        {
            WMR_PRINT(ERROR, "Terminating program due to General Error\n");
            break;
        }

        cePageCount[ceIndex]++;
    }
    h2fmi_ppn_disable_all_ces(fmi);

    if (error || timeout || fmi->ppn->general_error ||
        !h2fmi_dma_wait(h2fmi_dma_data_chan(fmi), H2FMI_PAGE_TIMEOUT_MICROS) ||
        !h2fmi_dma_wait(h2fmi_dma_meta_chan(fmi), H2FMI_PAGE_TIMEOUT_MICROS))
    {
        WMR_PRINT(ERROR, "Cancelling DMA\n");
        h2fmi_dma_cancel(h2fmi_dma_meta_chan(fmi));
        h2fmi_dma_cancel(h2fmi_dma_data_chan(fmi));
        h2fmi_reset(fmi);
        ret = FALSE32;
    }

    if (!timeout && error && !fmi->ppn->general_error)
    {
        UInt32    temp = 1;
        UInt8     status = 0;
        h2fmi_ce_t physCe = h2fmi_ppn_ce_index_to_physical(ppnCommand, errorCe);

        // Find failed pages...
        h2fmi_ppn_get_feature(fmi,
                              physCe,
                              PPN_FEATURE__PROGRAM_FAILED_PAGES,
                              (UInt8 *)fmi->error_list,
                              PPN_ERROR_LIST_SIZE,
                              &status);

        WMR_PRINT(ERROR, "PPN device reports %d pages failed program\n", fmi->error_list[0]);
        h2fmi_ppn_process_error_list(fmi, ppnCommand, errorCe, fmi->error_list, PPN_PROGRAM_STATUS_FAIL);

        // Find pending pages...
        h2fmi_ppn_get_feature(fmi,
                              physCe,
                              PPN_FEATURE__PROGRAM_IGNORED_PAGES,
                              (UInt8 *)fmi->error_list,
                              PPN_ERROR_LIST_SIZE,
                              &status);

        WMR_PRINT(ERROR, "PPN device reports %d pages pending after program failure\n", fmi->error_list[0]);
        h2fmi_ppn_process_error_list(fmi, ppnCommand, errorCe, fmi->error_list, PPN_PROGRAM_STATUS_NOT_PROGRAMMED);


        // Find pages to retire...
        h2fmi_ppn_get_feature(fmi,
                              physCe,
                              PPN_FEATURE__PROGRAM_RETIRED_PAGES,
                              (UInt8 *)fmi->error_list,
                              PPN_ERROR_LIST_SIZE,
                              &status);

        WMR_PRINT(ERROR, "PPN device reports %d pages should be retired after program failure\n", fmi->error_list[0]);
        h2fmi_ppn_process_error_list(fmi, ppnCommand, errorCe, fmi->error_list, PPN_PROGRAM_STATUS_FAIL);

        // Clear Program error lists
        h2fmi_ppn_set_features(fmi,
                               physCe,
                               PPN_FEATURE__CLEAR_PROGRAM_ERROR_LISTS,
                               (UInt8 *)&temp,
                               4,
                               FALSE32,
                               NULL);

    }

    ppnCommand->page_status_summary = overallStatus;
    if (timeout)
    {
        return _kIOPFMI_STATUS_READY_BUSY_TIMEOUT;
    }
    else if (overallStatus & PPN_OPERATION_STATUS__GENERAL_ERROR)
    {
        return _kIOPFMI_STATUS_PPN_GENERAL_ERROR;
    }
    else
    {
        return _kIOPFMI_STATUS_SUCCESS;
    }
}
#endif // !(defined(AND_READONLY)) || AND_SUPPORT_NVRAM

void h2fmi_ppn_process_error_list(h2fmi_t          *fmi,
                                  PPNCommandStruct *ppnCommand,
                                  UInt8             ceIdx,
                                  UInt32           *list,
                                  UInt8             status)
{
    const UInt32 count = list[0];
    UInt32 listIdx;

    if (!count)
    {
        return;
    }

    for (listIdx = 1; listIdx <= count; listIdx++)
    {
        const UInt32 page = list[listIdx];
        UInt16       cmdIdx;

        for (cmdIdx = 0; cmdIdx < ppnCommand->num_pages; cmdIdx++)
        {
            if ((ppnCommand->entry[cmdIdx].addr.row == page) &&
                (ppnCommand->entry[cmdIdx].ceIdx == ceIdx))
            {
                WMR_PRINT(ALWAYS, "Bus %d ceIdx %d setting index %d (page 0x%08x) status to 0x%02x\n",
                          fmi->bus_id, ceIdx, cmdIdx, page, status);

                ppnCommand->entry[cmdIdx].status |= status;
                break;
            }
        }

        if (cmdIdx >= ppnCommand->num_pages)
        {
            WMR_PRINT(ERROR, "CE 0x%x Page 0x%08x is not in the command structure!", ppnCommand->ceInfo[ceIdx].ce, page);
        }
    }
}


Int32 h2fmi_ppn_write_bootpage(h2fmi_t     *fmi,
                               UInt16       ce,
                               UInt32       dwPpn,
                               const UInt8 *pabData)
{
    Int32              ret = _kIOPFMI_STATUS_SUCCESS;
    UInt8              controller_status = 0;
    struct dma_segment sgl;
    BOOL32             timeout = FALSE32;
    UInt8                i;

#if SUPPORT_TOGGLE_NAND && !APPLICATION_EMBEDDEDIOP
    if (fmi->is_toggle_system)
    {
        transitionWorldFromDDR(PPN_FEATURE__POWER_STATE__ASYNC);
    }
#endif

    WMR_ASSERT(fmi->activeCe == (h2fmi_ce_t)~0);
    h2fmi_reset(fmi);
    h2fmi_ppn_switch_ce(fmi, ce);

    sgl.paddr = (UInt32)pabData;
    sgl.length = H2FMI_BOOT_BYTES_PER_PAGE;

    h2fmi_dma_execute_async(DMA_CMD_DIR_TX,
                            h2fmi_dma_data_chan(fmi),
                            &sgl,
                            h2fmi_dma_data_fifo(fmi),
                            H2FMI_BOOT_BYTES_PER_PAGE,
                            sizeof(UInt32),
                            32,
                            NULL);

    h2fmi_send_cmd_addr(fmi,
                        NAND_CMD__MULTIPAGE_PROGRAM_LAST,
                        (UInt8 *)&dwPpn,
                        fmi->ppn->bytes_per_row_address);

    h2fmi_ppn_set_format(fmi, H2FMI_BYTES_PER_BOOT_SECTOR, 1, TRUE32);
    for (i = 0; i < H2FMI_BOOT_SECTORS_PER_PAGE; i++)
    {
#if FMI_VERSION > 3
        h2fmi_wr(fmi, FMI_SCRAMBLER_SEED_FIFO, dwPpn + i);
#endif // FMI_VERSION > 3
        h2fmi_ppn_start_fmi_write_and_wait(fmi);
    }

    h2fmi_send_cmd(fmi, NAND_CMD__MULTIPAGE_PROGRAM_CONFIRM);

    if (!h2fmi_dma_wait(h2fmi_dma_data_chan(fmi), H2FMI_PAGE_TIMEOUT_MICROS))
    {
        WMR_PANIC("Timeout waiting for CDMA channel %d to complete\n",
                  h2fmi_dma_data_chan(fmi));
        h2fmi_dma_cancel(h2fmi_dma_data_chan(fmi));
    }

    ret = h2fmi_ppn_get_controller_status(fmi, &controller_status);
    if (!ret)
    {
        WMR_PRINT(ERROR, "Timeout waiting for controller status in Write bootpage\n");
        timeout = TRUE32;
    }

    h2fmi_ppn_disable_all_ces(fmi);

    if (timeout)
    {
        ret = _kIOPFMI_STATUS_READY_BUSY_TIMEOUT;
    }
    else if (controller_status & PPN_CONTROLLER_STATUS__GENERAL_ERROR)
    {
        ret = _kIOPFMI_STATUS_PPN_GENERAL_ERROR;
    }
    else if (controller_status & PPN_CONTROLLER_STATUS__PENDING_ERRORS)
    {
        UInt32  temp = 1;

        WMR_PRINT(ERROR, "PPN device reports program error (0x%02x) on write boot page 0x%08x\n",
                  controller_status, dwPpn);

        h2fmi_ppn_set_features(fmi,
                               ce,
                               PPN_FEATURE__CLEAR_PROGRAM_ERROR_LISTS,
                               (UInt8 *)&temp,
                               4,
                               FALSE32,
                               NULL);

        ret = _kIOPFMI_STATUS_PGM_ERROR;
    }

#if SUPPORT_TOGGLE_NAND && !APPLICATION_EMBEDDEDIOP
    if (fmi->is_toggle_system)
    {
        transitionWorldToDDR(PPN_FEATURE__POWER_STATE__ASYNC);
    }
#endif

    return ret;
}

Int32 h2fmi_ppn_erase_blocks(h2fmi_t *fmi, PPNCommandStruct *ppnCommand)
{
    const UInt32   queueSize    = fmi->ppn->device_params.erase_queue_size;
    BOOL32         ret;
    UInt16         ceIndex;
    UInt8          overallStatus = 0;
    BOOL32         timeout = FALSE32;

    WMR_ASSERT(fmi->activeCe == (h2fmi_ce_t)~0);

    for (ceIndex = 0; ceIndex < PPN_MAX_CES_PER_BUS; ceIndex++)
    {
        const h2fmi_ce_t physCe          = h2fmi_ppn_ce_index_to_physical(ppnCommand, ceIndex);
        const UInt32    numBlocks        = ppnCommand->ceInfo[ceIndex].pages;
        const UInt32    ceOffset         = ppnCommand->ceInfo[ceIndex].offset;
        UInt32          prepBlock        = 0;
        UInt32          eraseBlock       = 0;
        UInt8           controllerStatus = 0;
        UInt32          currentQueueLevel = 0;

        if (numBlocks == 0)
        {
            continue;
        }

        h2fmi_ppn_switch_ce(fmi, physCe);

        for (eraseBlock = 0; eraseBlock < numBlocks; eraseBlock++)
        {
            while((currentQueueLevel < queueSize) && (prepBlock < numBlocks))
            {
                UInt8 cmd;

                cmd = (prepBlock == (numBlocks - 1)) ?
                    NAND_CMD__MULTIBLOCK_ERASE_LAST :
                    NAND_CMD__MULTIBLOCK_ERASE;

                h2fmi_send_cmd_addr_cmd(fmi,
                                        cmd,
                                        NAND_CMD__MULTIBLOCK_ERASE_CONFIRM,
                                        (UInt8 *)&ppnCommand->entry[ceOffset + prepBlock].addr.row,
                                        fmi->ppn->bytes_per_row_address);

                if (!h2fmi_ppn_get_controller_status(fmi, &controllerStatus))
                {
                    timeout = TRUE32;
                    goto done;
                }

                currentQueueLevel++;
                prepBlock++;
            }

            // How do I find the right place for this op status if I'm striping across CEs...?
            ret = h2fmi_ppn_get_next_operation_status(fmi,
                                                      &ppnCommand->entry[ceOffset + eraseBlock].status);
            if (!ret)
            {
                timeout = TRUE32;
                goto done;
            }
            currentQueueLevel--;
            overallStatus |= ppnCommand->entry[ceOffset + eraseBlock].status;
        }

        h2fmi_send_cmd(fmi, NAND_CMD__GET_NEXT_OPERATION_STATUS);
    }

  done:
    h2fmi_ppn_disable_all_ces(fmi);
    ppnCommand->page_status_summary = overallStatus;

    if (timeout)
    {
        return _kIOPFMI_STATUS_READY_BUSY_TIMEOUT;
    }
    else if (overallStatus & PPN_CONTROLLER_STATUS__GENERAL_ERROR)
    {
        return _kIOPFMI_STATUS_PPN_GENERAL_ERROR;
    }
    else
    {
        return _kIOPFMI_STATUS_SUCCESS;
    }
}


/**
 * Send a multipage read PPN command for a particular page
 *
 * Send a:
 *        MULTIPAGE_READ / page / MULTIPAGE_READ_CONFIRM
 *        CONTROLLER_STATUS / wait for 0x42
 *
 * sequence to the NAND device
 *
 * @param fmi            Pointer to FMI bus sturucture
 * @param page           Value to send as NAND page address
 * @param last           TRUE32 if last page in operation
 *
 * @return
 *     - TRUE32 if operation completed with expected status
 *     - FALSE32 on timeout waiting for expected status
 */
static BOOL32 h2fmi_ppn_prep_read(h2fmi_t *fmi, const RowColLenAddressType *page, BOOL32 last)
{
    UInt8 cmd = (last == TRUE32) ? NAND_CMD__MULTIPAGE_READ_LAST : NAND_CMD__MULTIPAGE_READ;

    if (fmi->logical_page_size == fmi->bytes_per_page)
    {
        h2fmi_send_cmd_addr_cmd(fmi,
                                cmd,
                                NAND_CMD__MULTIPAGE_READ_CONFIRM,
                                (UInt8 *)&page->row,
                                fmi->ppn->bytes_per_row_address);
    }
    else
    {
        h2fmi_send_cmd_addr_cmd(fmi,
                                cmd,
                                NAND_CMD__MULTIPAGE_READ_CONFIRM,
                                (UInt8 *)page,
                                fmi->ppn->bytes_per_full_address);
    }

    return TRUE32;
}

static void h2fmi_ppn_prep_pages(h2fmi_t  *fmi,
                                 UInt16    physCe,
                                 UInt32    cePageCount)
{
    UInt32 writePrepCount = (cePageCount << 8) | 0x87;
    UInt32 fullSectors = 0;
    UInt32 address_bytes = cePageCount * sizeof(UInt32);

    h2fmi_ppn_switch_ce(fmi, physCe);

    h2fmi_send_cmd_addr(fmi,
                        NAND_CMD__MULTIPAGE_PREP,
                        (UInt8 *)&writePrepCount,
                        3);

    // First fill as many 1KB sectors as possible - note that it's the client's responsibility
    // to have started a DMA for the page address buffer.
    fullSectors = address_bytes / H2FMI_BYTES_PER_SECTOR;
    if (fullSectors > 0)
    {
        h2fmi_ppn_set_format(fmi, H2FMI_BYTES_PER_SECTOR, fullSectors, FALSE32);
        h2fmi_ppn_start_fmi_write_and_wait(fmi);
        address_bytes -= (H2FMI_BYTES_PER_SECTOR * fullSectors);
    }

    if (address_bytes > 0)
    {
        h2fmi_ppn_set_format(fmi, address_bytes, 1, FALSE32);
        h2fmi_ppn_start_fmi_write_and_wait(fmi);
    }

    h2fmi_send_cmd(fmi, NAND_CMD__MULTIPAGE_PREP__CONFIRM);
}

static void h2fmi_ppn_prep_write_multi(h2fmi_t                *fmi,
                                       const PPNCommandStruct *ppnCommand)
{
    const UInt32 totalPages = ppnCommand->num_pages;
    UInt16 ceIndex;
    UInt32 pageIndex;
    struct dma_segment dma_segment;

    for (pageIndex = 0 ; pageIndex < totalPages ; pageIndex++)
    {
        const PPNCommandEntry *entry = &ppnCommand->entry[pageIndex];

        *fmi->ppn->prep_buffer[entry->ceIdx]++ = entry->addr.row;
    }


    // If we ever get DMA chaining in EFI we could make a SGL out of this and use
    // a single DMA for all CEs..
    for (ceIndex = 0; ceIndex < PPN_MAX_CES_PER_BUS; ceIndex++)
    {
        const UInt32       numPages = ppnCommand->ceInfo[ceIndex].pages;
        const h2fmi_ce_t   physCe   = h2fmi_ppn_ce_index_to_physical(ppnCommand, ceIndex);

        fmi->ppn->prep_buffer[ceIndex] -= numPages;

        if (numPages > 0)
        {
            const UInt32 prepBuffer = (UInt32)fmi->ppn->prep_buffer[ceIndex];

            WMR_PREPARE_WRITE_BUFFER(fmi->ppn->prep_buffer[ceIndex], numPages * sizeof(**fmi->ppn->prep_buffer));

#if WMR_BUILDING_IBOOT
            dma_segment.paddr  = mem_static_map_physical(prepBuffer);
#else
            dma_segment.paddr  = prepBuffer;
#endif /* WMR_BUILDING_IBOOT */
            dma_segment.length = numPages * sizeof(UInt32);

            h2fmi_dma_execute_async(DMA_CMD_DIR_TX,
                                    h2fmi_dma_data_chan(fmi),
                                    &dma_segment,
                                    h2fmi_dma_data_fifo(fmi),
                                    numPages * sizeof(UInt32),
                                    sizeof(UInt32),
                                    1,
                                    NULL);

            h2fmi_ppn_prep_pages(fmi, physCe, numPages);

            if (!h2fmi_dma_wait(h2fmi_dma_data_chan(fmi), H2FMI_PAGE_TIMEOUT_MICROS))
            {
                WMR_PANIC("Timeout waiting for CDMA channel %d to complete multi prep: ce %d, %d pages", h2fmi_dma_data_chan(fmi), physCe, numPages);
                h2fmi_dma_cancel(h2fmi_dma_data_chan(fmi));
                WMR_PRINT(ERROR, "Timeout waiting for CDMA to complete multi prep\n");
            }
        }
    }
}

Int32 h2fmi_ppn_set_features(h2fmi_t *fmi, UInt32 ce, UInt16 feature, UInt8 *data, UInt32 len, BOOL32 optFeature, UInt8 *stat)
{
     UInt8  operation_status = 0;
     Int32 ret = FIL_SUCCESS;
     UInt32 fmi_control = (FMI_CONTROL__MODE__WRITE |
                           FMI_CONTROL__START_BIT);
     UInt32 pos;
     UInt32 sector_len;

     WMR_ASSERT(fmi->activeCe == (h2fmi_ce_t)~0);
     h2fmi_ppn_switch_ce(fmi, ce);
     h2fmi_wr(fmi, FMI_CONTROL, FMI_CONTROL__MODE__SOFTWARE_RESET);

     h2fmi_send_cmd_addr(fmi, NAND_CMD__SET_FEATURES, (UInt8 *)&feature, 2);
     h2fmi_clear_interrupts_and_reset_masks(fmi);

     for(pos = 0; pos < len; pos += sector_len)
     {
         UInt32 bounce = 0;
         void *cursor = &data[pos];

         sector_len = WMR_MIN(H2FMI_BYTES_PER_SECTOR, len - pos);
         if (sizeof(bounce) > sector_len)
         {
             WMR_MEMCPY(&bounce, cursor, sector_len);
             cursor = &bounce;
             sector_len = sizeof(bounce);
         }
         else
         {
             sector_len = ROUNDDOWNTO(sector_len, sizeof(bounce));
         }

         h2fmi_ppn_set_format(fmi, sector_len, 1, FALSE32);
         h2fmi_wr(fmi, FMI_CONTROL, fmi_control);
         h2fmi_pio_write_sector(fmi, cursor, sector_len);
         h2fmi_busy_wait(fmi, FMI_INT_PEND, FMI_INT_PEND__LAST_FMC_DONE, FMI_INT_PEND__LAST_FMC_DONE);
         h2fmi_wr(fmi, FMI_INT_PEND, FMI_INT_PEND__LAST_FMC_DONE);
     }

     h2fmi_send_cmd(fmi, NAND_CMD__SET_GET_FEATURES_CONFIRM);
     if (!h2fmi_ppn_get_next_operation_status(fmi, &operation_status) || (operation_status != PPN_FEAT_OPERATION_STATUS__SUCCESS))
     {
         if ((operation_status == PPN_FEAT_OPERATION_STATUS__UNSUPPORTED_BY_CH) || (operation_status == PPN_FEAT_OPERATION_STATUS__UNSUPPORTED))
         {
             if (!optFeature)
             {
                 WMR_PRINT(ERROR, "PPN feature 0x%04x is unsupported on CH %d: status = 0x%02x\n", feature, fmi->bus_id, operation_status);
             }
             ret = FIL_UNSUPPORTED_ERROR;
         }
         else
         {
             WMR_PRINT(ERROR, "Unable to set PPN feature 0x%04x: status = 0x%02x\n", feature, operation_status);
             ret = FIL_CRITICAL_ERROR;
         }
     }
     h2fmi_send_cmd(fmi, NAND_CMD__GET_NEXT_OPERATION_STATUS);
     h2fmi_wr(fmi, FMI_CONTROL, FMI_CONTROL__MODE__SOFTWARE_RESET);
     h2fmi_ppn_disable_all_ces(fmi);
     h2fmi_reset(fmi);

#if H2FMI_PPN_VERIFY_SET_FEATURES
     if (FIL_SUCCESS == ret)
     {
         _update_feature_shadow(fmi, feature, data, len);
     }
#endif // H2FMI_PPN_VERIFY_SET_FEATURES

     if (stat)
     {
         *stat = operation_status;
     }

     return ret;
}

BOOL32 h2fmi_ppn_get_feature(h2fmi_t    *fmi,
                             h2fmi_ce_t ce,
                             UInt32     feature,
                             UInt8      *buffer,
                             UInt32     len,
                             UInt8      *status)
{
    BOOL32 ret = TRUE32;
    UInt32 fmi_control = (FMI_CONTROL__MODE__READ |
                          FMI_CONTROL__START_BIT);
    UInt32 pos;
    UInt32 sector_len;
    UInt8  operation_status = 0;

#if FMI_VERSION == 4
    if (fmi->read_stream_disable)
    {
        fmi_control |= FMI_CONTROL__DISABLE_STREAMING;
    }
#endif // FMI_VERSION == 4

    h2fmi_ppn_switch_ce(fmi, ce);

    h2fmi_clear_interrupts_and_reset_masks(fmi);

    h2fmi_send_cmd_addr_cmd(fmi,
                            NAND_CMD__GET_FEATURES,
                            NAND_CMD__SET_GET_FEATURES_CONFIRM,
                            (UInt8 *)&feature,
                            2);
    if (!h2fmi_ppn_get_next_operation_status(fmi, &operation_status) || (operation_status != 0x40))
    {
        WMR_PRINT(ERROR, "Unable to get PPN feature 0x%04x: status = 0x%02x\n", feature, (UInt32) operation_status);
        ret = FALSE32;
    }
    h2fmi_send_cmd(fmi, NAND_CMD__READ_SERIAL_OUTPUT);

    for(pos = 0; pos < len; pos += sector_len)
    {
        UInt32 bounce = 0;
        void *cursor = &buffer[pos];

        sector_len = WMR_MIN(H2FMI_BYTES_PER_SECTOR, len - pos);
        if (sizeof(bounce) > sector_len)
        {
            cursor = &bounce;
            sector_len = sizeof(bounce);
        }
        else
        {
            sector_len = ROUNDDOWNTO(sector_len, sizeof(bounce));
        }

        h2fmi_ppn_set_format(fmi, sector_len, 1, FALSE32);
        h2fmi_wr(fmi, FMI_CONTROL, fmi_control);
        h2fmi_pio_read_sector(fmi, cursor, sector_len);

        if (&bounce == cursor)
        {
            WMR_MEMCPY(&buffer[pos], cursor, WMR_MIN(H2FMI_BYTES_PER_SECTOR, len - pos));
        }
    }

    h2fmi_wr(fmi, FMI_CONTROL, FMI_CONTROL__MODE__SOFTWARE_RESET);
    h2fmi_send_cmd(fmi, NAND_CMD__GET_NEXT_OPERATION_STATUS);
    h2fmi_ppn_disable_all_ces(fmi);

    if (status)
    {
        *status = operation_status;
    }

    return ret;
}


static void h2fmi_ppn_set_format(h2fmi_t *fmi, UInt32 sector_len, UInt32 sector_count, BOOL32 bootpage)
{
    UInt32 fmi_config;

    if (bootpage)
    {
        fmi_config  = FMI_CONFIG__ECC_CORRECTABLE_BITS(30) |
                      FMI_CONFIG__DMA_BURST__32_CYCLES;
#if FMI_VERSION > 3
        fmi_config |= FMI_CONFIG__SCRAMBLE_SEED |
                      FMI_CONFIG__ENABLE_WHITENING;
#endif
    }
    else
    {
        fmi_config  = FMI_CONFIG__ECC_CORRECTABLE_BITS(0) |
                      FMI_CONFIG__DMA_BURST__32_CYCLES;
    }

    h2fmi_wr(fmi, FMI_CONFIG, fmi_config);
    h2fmi_wr(fmi, FMI_DATA_SIZE, (FMI_DATA_SIZE__META_BYTES_PER_SECTOR(0) |
                                  FMI_DATA_SIZE__META_BYTES_PER_PAGE(0) |
                                  FMI_DATA_SIZE__BYTES_PER_SECTOR(sector_len) |
                                  FMI_DATA_SIZE__SECTORS_PER_PAGE(sector_count)));
#if FMI_VERSION > 3
    if (bootpage)
    {
        h2fmi_wr(fmi, FMI_CONTROL, FMI_CONTROL__RESET_SEED);
    }
#endif // FMI_VERSION > 3
}

static void h2fmi_ppn_start_fmi_read_and_wait(h2fmi_t *fmi)
{
    UInt32 fmi_control = (FMI_CONTROL__MODE__READ |
                          FMI_CONTROL__START_BIT);

#if FMI_VERSION == 4
    if (fmi->read_stream_disable)
    {
        fmi_control |= FMI_CONTROL__DISABLE_STREAMING;
    }
#endif // FMI_VERSION == 4

    h2fmi_clear_interrupts_and_reset_masks(fmi);
    h2fmi_wr(fmi, FMI_CONTROL, fmi_control);
    h2fmi_busy_wait(fmi, FMI_INT_PEND, FMI_INT_PEND__LAST_FMC_DONE, FMI_INT_PEND__LAST_FMC_DONE);
    h2fmi_wr(fmi, FMI_INT_PEND, FMI_INT_PEND__LAST_FMC_DONE);
}

static void h2fmi_ppn_start_fmi_write_and_wait(h2fmi_t *fmi)
{
    UInt32 fmi_control = (FMI_CONTROL__MODE__WRITE |
                          FMI_CONTROL__START_BIT);

    h2fmi_clear_interrupts_and_reset_masks(fmi);
    h2fmi_wr(fmi, FMI_CONTROL, fmi_control);
    h2fmi_busy_wait(fmi, FMI_INT_PEND, FMI_INT_PEND__LAST_FMC_DONE, FMI_INT_PEND__LAST_FMC_DONE);
    h2fmi_wr(fmi, FMI_INT_PEND, FMI_INT_PEND__LAST_FMC_DONE);
}

static void h2fmi_ppn_get_page_data(h2fmi_t *fmi, UInt32 length)
{
    UInt32 fmi_control = (FMI_CONTROL__MODE__READ |
                          FMI_CONTROL__START_BIT);

#if FMI_VERSION == 4
    if (fmi->read_stream_disable)
    {
        fmi_control |= FMI_CONTROL__DISABLE_STREAMING;
    }
#endif // FMI_VERSION == 4

	h2fmi_prepare_for_fmi_interrupt(fmi, FMI_INT_EN__LAST_FMC_DONE);
    h2fmi_wr(fmi, FMI_CONFIG, fmi->fmi_config_reg);
    h2fmi_wr(fmi, FMI_DATA_SIZE, (FMI_DATA_SIZE__BYTES_PER_SECTOR(H2FMI_BYTES_PER_SECTOR) |
                                  FMI_DATA_SIZE__SECTORS_PER_PAGE(length / H2FMI_BYTES_PER_SECTOR) |
                                  FMI_DATA_SIZE__META_BYTES_PER_SECTOR(fmi->valid_bytes_per_meta) |
                                  FMI_DATA_SIZE__META_BYTES_PER_PAGE(fmi->valid_bytes_per_meta)));
    h2fmi_wr(fmi, FMI_CONTROL, fmi_control);
	if (!h2fmi_wait_for_fmi_interrupt(fmi, FMI_INT_PEND__LAST_FMC_DONE))
	{
        WMR_PANIC("Timeout waiting for LAST_FMC_DONE interrupt");
    }
}

static void   h2fmi_ppn_program_page(h2fmi_t *fmi, UInt32 page, BOOL32 lastPage, UInt32 lbas)
{
    UInt32 lba;
    UInt32 fmi_control = (FMI_CONTROL__MODE__WRITE |
                          FMI_CONTROL__START_BIT);

    h2fmi_send_cmd_addr(fmi,
                        lastPage ? NAND_CMD__MULTIPAGE_PROGRAM_LAST : NAND_CMD__MULTIPAGE_PROGRAM,
                        (UInt8 *)&page,
                        fmi->ppn->bytes_per_row_address);

    for (lba = 0; lba < lbas; lba++)
    {
        h2fmi_prepare_for_fmi_interrupt(fmi, FMI_INT_EN__LAST_FMC_DONE);
        h2fmi_wr(fmi, FMI_CONFIG, fmi->fmi_config_reg);
        h2fmi_wr(fmi, FMI_DATA_SIZE, (FMI_DATA_SIZE__BYTES_PER_SECTOR(H2FMI_BYTES_PER_SECTOR) |
                                      FMI_DATA_SIZE__SECTORS_PER_PAGE(fmi->logical_page_size / H2FMI_BYTES_PER_SECTOR) |
                                      FMI_DATA_SIZE__META_BYTES_PER_SECTOR(fmi->valid_bytes_per_meta) |
                                      FMI_DATA_SIZE__META_BYTES_PER_PAGE(fmi->valid_bytes_per_meta)));

        h2fmi_wr(fmi, FMI_CONTROL, fmi_control);
        if (!h2fmi_wait_for_fmi_interrupt(fmi, FMI_INT_PEND__LAST_FMC_DONE))
        {
            WMR_PANIC("Timeout waiting for LAST_FMC_DONE interrupt");
        }
    }
    
    h2fmi_send_cmd(fmi, NAND_CMD__MULTIPAGE_PROGRAM_CONFIRM);
}

BOOL32 h2fmi_ppn_get_debug_data(h2fmi_t *fmi, h2fmi_ce_t ce, UInt32 type, UInt8 *buffer, UInt32 len)
{
    UInt8 operation_status = 0;
    h2fmi_ppn_switch_ce(fmi, ce);

    h2fmi_send_cmd_addr_cmd(fmi,
                            NAND_CMD__GET_DEBUG_DATA,
                            NAND_CMD__SET_GET_FEATURES_CONFIRM,
                            (UInt8 *)&type,
                            3);

    WMR_SLEEP_US(20000);

    h2fmi_send_cmd(fmi, NAND_CMD__OPERATION_STATUS);
    h2fmi_get_nand_status(fmi, &operation_status);
    //WMR_ASSERT(operation_status == 0x50);
    h2fmi_ppn_read_serial_output(fmi, buffer, len);
    h2fmi_ppn_disable_all_ces(fmi);
    return TRUE32;
}

BOOL32 h2fmi_ppn_set_debug_data(h2fmi_t *fmi,
                                h2fmi_ce_t ce,
                                UInt32     type,
                                UInt8     *buffer,
                                UInt32     len)
{
    UInt8 operationStatus = 0;
    UInt32 fmi_control = (FMI_CONTROL__MODE__WRITE |
                          FMI_CONTROL__START_BIT);

    h2fmi_ppn_switch_ce(fmi, ce);

    h2fmi_send_cmd_addr(fmi,
                        NAND_CMD__SET_DEBUG_DATA,
                        (UInt8 *)&type,
                        3);

    h2fmi_prepare_for_fmi_interrupt(fmi, FMI_INT_EN__LAST_FMC_DONE);
    h2fmi_ppn_set_format(fmi, len, 1, FALSE32);
    h2fmi_wr(fmi, FMI_CONTROL, fmi_control);
    h2fmi_pio_write_sector(fmi, buffer, len);
    if (!h2fmi_wait_for_fmi_interrupt(fmi, FMI_INT_PEND__LAST_FMC_DONE))
    {
        h2fmi_ppn_disable_all_ces(fmi);
        WMR_PANIC("Timeout waiting for LAST_FMC_DONE interrupt");
    }

    h2fmi_wr(fmi, FMI_CONTROL, FMI_CONTROL__MODE__SOFTWARE_RESET);

    h2fmi_send_cmd(fmi, NAND_CMD__SET_GET_FEATURES_CONFIRM);

    WMR_SLEEP_US(10000);

    h2fmi_send_cmd(fmi, NAND_CMD__OPERATION_STATUS);
    if (!h2fmi_get_nand_status(fmi, &operationStatus))
    {
        WMR_PRINT(ERROR, "Timeout reading operation status in SetDebugData - don't trust any debug data you pull from here.\n");
    }

    h2fmi_ppn_disable_all_ces(fmi);
    return TRUE32;
}

BOOL32 h2fmi_ppn_get_fw_version(h2fmi_t    *fmi,
                                h2fmi_ce_t  ce,
                                UInt8      *buffer)
{
    BOOL32 ret;
    UInt8  operationStatus = 0;

    h2fmi_reset(fmi);

    ret = h2fmi_ppn_get_feature(fmi,
                                ce,
                                PPN_FEATURE__FW_VERSION,
                                buffer,
                                NAND_DEV_PARAM_LEN_PPN,
                                &operationStatus);

    if (!ret)
    {
        WMR_PRINT(ERROR, "failed CE %d\n", ce);
    }
    return ret;
}

BOOL32 h2fmi_ppn_get_package_assembly_code(h2fmi_t    *fmi,
                                           h2fmi_ce_t  ce,
                                           UInt8      *buffer)
{
    BOOL32 ret;
    UInt8  operationStatus = 0;

    h2fmi_reset(fmi);
    ret = h2fmi_ppn_get_feature(fmi,
                                ce,
                                PPN_FEATURE__PACKAGE_ASSEMBLY_CODE,
                                buffer,
                                NAND_DEV_PARAM_LEN_PPN,
                                &operationStatus);
    return ret;
}

BOOL32 h2fmi_ppn_get_controller_unique_id(h2fmi_t    *fmi,
                                          h2fmi_ce_t  ce,
                                          UInt8      *buffer)
{
    BOOL32 ret;
    UInt8  operationStatus = 0;

    h2fmi_reset(fmi);

    ret = h2fmi_ppn_get_feature(fmi,
                                ce,
                                PPN_FEATURE__CONTROLLER_UNIQUE_ID,
                                buffer,
                                NAND_DEV_PARAM_LEN_PPN,
                                &operationStatus);
    return ret;
}

BOOL32 h2fmi_ppn_get_controller_hw_id(h2fmi_t    *fmi,
                                      h2fmi_ce_t  ce,
                                      UInt8      *buffer)
{
    BOOL32 ret;
    UInt8  operationStatus = 0;

    h2fmi_reset(fmi);

    ret = h2fmi_ppn_get_feature(fmi,
                                ce,
                                PPN_FEATURE__CONTROLLER_HW_ID,
                                buffer,
                                NAND_DEV_PARAM_LEN_PPN,
                                &operationStatus);
    return ret;
}

BOOL32 h2fmi_ppn_get_marketing_name(h2fmi_t    *fmi,
                                    h2fmi_ce_t  ce,
                                    UInt8      *buffer)
{
    BOOL32 ret;
    UInt8  operationStatus = 0;

    h2fmi_reset(fmi);

    ret = h2fmi_ppn_get_feature(fmi,
                                ce,
                                PPN_FEATURE__NAND_MARKETING_NAME,
                                buffer,
                                PPN_NAND_MARKETING_NAME_LENGTH,
                                &operationStatus);
    return ret;
}

BOOL32 h2fmi_ppn_get_manufacturer_id(h2fmi_t *fmi, h2fmi_ce_t ce, UInt8 *buffer)
{
    UInt8        addr;
    const UInt32 len = 8;
    UInt32 fmi_control = (FMI_CONTROL__MODE__READ |
                          FMI_CONTROL__START_BIT);

#if SUPPORT_TOGGLE_NAND
    // assert that SoC is in non-toggle (async) mode
    WMR_ASSERT(!(fmi->is_toggle));
#endif

#if FMI_VERSION == 4
    if (fmi->read_stream_disable)
    {
        fmi_control |= FMI_CONTROL__DISABLE_STREAMING;
    }
#endif // FMI_VERSION == 4

    h2fmi_reset(fmi);

    h2fmi_ppn_switch_ce(fmi, ce);

    addr = MfgID_ADDR;

    h2fmi_send_cmd_addr(fmi,
                        NAND_CMD__READ_ID,
                        (UInt8 *)&addr,
                        1);

    h2fmi_ppn_set_format(fmi, len, 1, FALSE32);
    h2fmi_wr(fmi, FMI_CONTROL, fmi_control);
    h2fmi_pio_read_sector(fmi, buffer, len);

    h2fmi_ppn_disable_all_ces(fmi);
    h2fmi_reset(fmi);
    return TRUE32;
}

BOOL32 h2fmi_ppn_get_die_chip_id(h2fmi_t *fmi, h2fmi_ce_t ce, UInt32 die, UInt8 *buffer)
{
    BOOL32 ret;
    UInt8  operationStatus = 0;

    h2fmi_reset(fmi);

    ret = h2fmi_ppn_get_feature(fmi,
                                ce,
                                PPN_FEATURE__NAND_DIE_CHIP_ID | (die << 8),
                                buffer,
                                PPN_FEATURE_LENGTH_DIE_CHIP_ID,
                                &operationStatus);
    return ret;
}


BOOL32 h2fmi_ppn_get_general_error_info(h2fmi_t                  *fmi,
                                        h2fmi_ce_t                ce,
                                        h2fmi_ppn_failure_info_t *buffer,
                                        UInt32                   *dataLength,
                                        struct dma_segment       *sgl)
{
    BOOL32 ret;
    UInt32 structLength;

    WMR_ASSERT(buffer != NULL);
    WMR_ASSERT(dataLength != NULL);

    h2fmi_reset(fmi);

    if(fmi->ppn->bytes_per_row_address == PPN_1_5_ROW_ADDR_BYTES)
    {
        structLength = sizeof(h2fmi_ppn_failure_info_ppn1_5_t);
    }
    else
    {
        structLength = sizeof(h2fmi_ppn_failure_info_ppn1_0_t);
    }
    ret = h2fmi_ppn_get_debug_data(fmi,
                                   ce,
                                   PPN_GET_DEBUG_DATA__GET_FAILURE_TYPE,
                                   (UInt8 *)buffer,
                                   structLength);
    
    if (ret)
    {
        UInt32 failureInfoPage;
        UInt32 failureInfoPageCount;

        if(fmi->ppn->bytes_per_row_address == PPN_1_5_ROW_ADDR_BYTES)
        {
            h2fmi_ppn_failure_info_ppn1_5_t *tempBuffer = &(buffer->failure_info_1_5);
            WMR_PRINT(ERROR, "General Failure Info - failure: 0x%04x  page 0x%02x%02x%02x%02x count 0x%02x%02x%02x checksum %02x\n",
                  tempBuffer->type,
                  tempBuffer->startPage[3], tempBuffer->startPage[2], tempBuffer->startPage[1], tempBuffer->startPage[0],
                  tempBuffer->pageCount[2], tempBuffer->pageCount[1], tempBuffer->pageCount[0],
                  tempBuffer->checksum);

            failureInfoPage = (tempBuffer->startPage[0] |
                               (tempBuffer->startPage[1] << 8) |
                               (tempBuffer->startPage[2] << 16) |
                               (tempBuffer->startPage[3] << 24));
    
            failureInfoPageCount = (((tempBuffer->pageCount[0]) |
                                     (tempBuffer->pageCount[1] << 8) |
                                     (tempBuffer->pageCount[2] << 16)));
        }
        else
        {
            h2fmi_ppn_failure_info_ppn1_0_t *tempBuffer = &(buffer->failure_info_1_0);
            WMR_PRINT(ERROR, "General Failure Info - failure: 0x%04x  page 0x%02x%02x%02x count 0x%02x%02x%02x checksum %02x\n",
                  tempBuffer->type,
                  tempBuffer->startPage[2], tempBuffer->startPage[1], tempBuffer->startPage[0],
                  tempBuffer->pageCount[2], tempBuffer->pageCount[1], tempBuffer->pageCount[0],
                  tempBuffer->checksum);

            failureInfoPage = (tempBuffer->startPage[0] |
                               (tempBuffer->startPage[1] << 8) |
                               (tempBuffer->startPage[2] << 16));
    
            failureInfoPageCount = (((tempBuffer->pageCount[0]) |
                                     (tempBuffer->pageCount[1] << 8) |
                                     (tempBuffer->pageCount[2] << 16)));
        }

        *dataLength = failureInfoPageCount * PPN_PERFECT_PAGE_SIZE;

        if ((failureInfoPageCount > 0) &&
            (NULL != sgl))
        {
            // Pull the full Debug Data
            h2fmi_ppn_dma_debug_data_payload(fmi,
                                             ce,
                                             failureInfoPage,
                                             failureInfoPageCount,
                                             sgl,
                                             TRUE32);
        }
    }
    else
    {
        *dataLength = 0;
        WMR_PRINT(ERROR, "Timeout reading Failure Info Debug Data\n");
    }

    h2fmi_ppn_disable_all_ces(fmi);
    return ret;
}

static Int32 h2fmi_ppn_update_fw_chunk(h2fmi_t            *fmi,
                                       UInt32              chunk,
                                       UInt32              length)
{
    Int32  ret;
    UInt8  operation_status = 0;

    h2fmi_send_cmd_addr(fmi, NAND_CMD__UPDATE_FW, (UInt8 *)&chunk, 2);

    h2fmi_ppn_set_format(fmi, PPN_PERFECT_FMI_SECTOR_SIZE, PPN_PERFECT_FMI_SECTORS_PER_PERFECT_PAGE, FALSE32);
    h2fmi_ppn_start_fmi_write_and_wait(fmi);

    h2fmi_send_cmd(fmi, NAND_CMD__SET_GET_FEATURES_CONFIRM);
    ret = h2fmi_ppn_get_next_operation_status_with_addr(fmi, (UInt8 *)&chunk, &operation_status);
    if (!ret)
    {
        WMR_PRINT(ERROR, "Timeout reading operation status on firmware chunk %d\n", chunk);
        ret = FIL_CRITICAL_ERROR;
    }
    else if (operation_status == PPN_OPERATION_STATUS__READY)
    {
        ret = FIL_SUCCESS;
    }
    else if (operation_status == (PPN_OPERATION_STATUS__READY |
                                  PPN_OPERATION_STATUS__CLEAN |
                                  PPN_OPERATION_STATUS__ERROR))
    {
        // 0x49 indicates channel not updateable
        WMR_PRINT(ERROR, "Channel doesn't support firmware updates - skipping\n");
        ret = FIL_UNSUPPORTED_ERROR;
    }
    else
    {
        WMR_PRINT(ERROR, "Unexpected status 0x%02x on firmware chunk %d\n",
                  operation_status, chunk);
        ret = FIL_CRITICAL_ERROR;
    }

    h2fmi_send_cmd(fmi, NAND_CMD__GET_NEXT_OPERATION_STATUS);

    return ret;
}


Int32 h2fmi_ppn_fw_update(h2fmi_t              *fmi,
                          h2fmi_ce_t            ce,
                          struct dma_segment   *sgl,
                          UInt32                fw_size,
                          h2fmi_ppn_fw_type     fw_type)
{
    UInt32 start_signature = PPN_FW_START_SIGNATURE;
    UInt32 end_signature   = PPN_FW_END_SIGNATURE;
    UInt32 chunk = 0;
    UInt32 max_chunk_size = PPN_PERFECT_PAGE_SIZE;
    Int32  ret = FIL_SUCCESS;
    UInt16 feature;
    UInt8  operStat = 0;

    if (fw_type == ppnFwTypeFw)
    {
        feature = PPN_FEATURE__FW_UPDATE;
    }
    else if (fw_type == ppnFwTypeFwa)
    {
        feature = PPN_FEATURE__FWA_UPDATE;
    }
    else
    {
        WMR_PANIC("Invalid firmware type");
    }

    h2fmi_reset(fmi);

    h2fmi_set_if_ctrl(fmi, (FMC_IF_CTRL__REB_SETUP(FMC_IF_CTRL__TIMING_MAX_CLOCKS) |
                            FMC_IF_CTRL__REB_HOLD(FMC_IF_CTRL__TIMING_MAX_CLOCKS) |
                            FMC_IF_CTRL__WEB_SETUP(FMC_IF_CTRL__TIMING_MAX_CLOCKS) |
                            FMC_IF_CTRL__WEB_HOLD(FMC_IF_CTRL__TIMING_MAX_CLOCKS) |
                            FMC_IF_CTRL__DCCYCLE(0)));

    //rdar://problem/11286520 Allow timeout on FW update for channel 1
#if !APPLICATION_EMBEDDEDIOP
    if (fmi->bus_id != 0)
    {
        fmi->h2fmi_ppn_panic_on_status_timeout = FALSE32;
    }
#endif

    if (FIL_SUCCESS != h2fmi_ppn_set_features(fmi, ce, feature, (UInt8 *)&start_signature, 4, FALSE32, &operStat))
    {
        if (operStat == (PPN_OPERATION_STATUS__READY | PPN_OPERATION_STATUS__CLEAN | PPN_OPERATION_STATUS__ERROR))
        {
            WMR_PRINT(ERROR, "CH %d, CE %d doesn't support entering firmware updates!\n", fmi->bus_id, ce);

#if !APPLICATION_EMBEDDEDIOP
            fmi->h2fmi_ppn_panic_on_status_timeout = TRUE32;
#endif
            return FIL_UNSUPPORTED_ERROR;
        }
        else
        {
            WMR_PRINT(ERROR, "Error entering PPN firmware update mode\n");
#if !APPLICATION_EMBEDDEDIOP
            if (fmi->h2fmi_ppn_panic_on_status_timeout == FALSE32){
                fmi->h2fmi_ppn_panic_on_status_timeout = TRUE32;
                return FIL_UNSUPPORTED_ERROR;
            }
#endif
            return FIL_CRITICAL_ERROR;
        }
    }
#if !APPLICATION_EMBEDDEDIOP
    else
    {
        fmi->h2fmi_ppn_panic_on_status_timeout = TRUE32;
    }
#endif

    h2fmi_ppn_switch_ce(fmi, ce);

    // We may be coming in here from a corrupted image, etc.  Go as slow
    // as possible through the update.

    h2fmi_dma_execute_async(DMA_CMD_DIR_TX,
                            h2fmi_dma_data_chan(fmi),
                            sgl,
                            h2fmi_dma_data_fifo(fmi),
                            fw_size,
                            sizeof(UInt32),
                            1,
                            NULL);

    while( fw_size )
    {
        const UInt32 chunk_len = WMR_MIN(max_chunk_size, fw_size);
        Int32 status;

        status = h2fmi_ppn_update_fw_chunk(fmi, chunk, chunk_len);
        if (status != FIL_SUCCESS)
        {
            ret = status;
            goto done;
        }
        chunk++;
        fw_size -= chunk_len;
    }

    h2fmi_ppn_disable_all_ces(fmi);

    operStat = 0;
    if (FIL_SUCCESS != h2fmi_ppn_set_features(fmi, ce, feature, (UInt8 *)&end_signature, 4, FALSE32, &operStat))
    {
        if (operStat == (PPN_OPERATION_STATUS__READY | PPN_OPERATION_STATUS__CLEAN | PPN_OPERATION_STATUS__ERROR))
        {
            WMR_PRINT(ERROR, "CH %d, CE %d doesn't support exiting firmware updates!\n", fmi->bus_id, ce);
            ret = FIL_UNSUPPORTED_ERROR;
        }
        else
        {
            WMR_PRINT(ERROR, "Error exiting PPN firmware update mode\n");
            ret = FIL_CRITICAL_ERROR;
        }
    }

  done:
    h2fmi_ppn_disable_all_ces(fmi);

    if (ret == FIL_SUCCESS)
    {
        if (!h2fmi_dma_wait(h2fmi_dma_data_chan(fmi), H2FMI_PAGE_TIMEOUT_MICROS))
        {
            WMR_PRINT(ERROR, "Timeout waiting for CDMA channel %d to complete\n",
                      h2fmi_dma_data_chan(fmi));
            h2fmi_dma_cancel(h2fmi_dma_data_chan(fmi));
            ret = FIL_CRITICAL_ERROR;
        }
    }
    else
    {
        h2fmi_dma_cancel(h2fmi_dma_data_chan(fmi));
    }

    return ret;
}


void h2fmi_ppn_poweron(h2fmi_t *fmi)
{
    // Enable Vcc
    // Enable VccQ
}

void h2fmi_ppn_poweroff(h2fmi_t *fmi)
{
    // Will we ever even do this?
    WMR_PANIC("Called h2fmi_ppn_poweroff");
}

void h2fmi_ppn_set_power_low_power(h2fmi_t *fmi)
{
    // Only valid from NORMAL state - maybe STANDBY?
    // From ReadyForReset, we should come in through h2fmi_ppn_reset()

    // SetFeatures(PowerState, Low Power)
}

void h2fmi_ppn_set_power_standby(h2fmi_t *fmi)
{
    // Disable Vcc, leave VccQ enabled
}

static void h2fmi_send_cmd(h2fmi_t *fmi, UInt8 cmd)
{
    h2fmi_wr(fmi, FMC_CMD, FMC_CMD__CMD1(cmd));
    h2fmi_wr(fmi, FMC_RW_CTRL, FMC_RW_CTRL__CMD1_MODE);
    h2fmi_busy_wait(fmi, FMC_STATUS, FMC_STATUS__CMD1DONE, FMC_STATUS__CMD1DONE);
    h2fmi_wr(fmi, FMC_STATUS, FMC_STATUS__CMD1DONE);
}

static void h2fmi_send_cmd_addr(h2fmi_t *fmi, UInt8 cmd, UInt8 *address, UInt32 address_bytes)
{
    const UInt32 cmd1_addr_done = (FMC_STATUS__ADDRESSDONE |
                                   FMC_STATUS__CMD1DONE);
    h2fmi_wr(fmi, FMC_CMD, FMC_CMD__CMD1(cmd));
    h2fmi_wr(fmi, FMC_ADDR0, (FMC_ADDR0__SEQ0(address[0]) |
                              FMC_ADDR0__SEQ1(address[1]) |
                              FMC_ADDR0__SEQ2(address[2]) |
                              FMC_ADDR0__SEQ3(address[3])));
    h2fmi_wr(fmi, FMC_ADDRNUM, FMC_ADDRNUM__NUM(address_bytes - 1));
    h2fmi_wr(fmi, FMC_RW_CTRL, (FMC_RW_CTRL__ADDR_MODE |
                                FMC_RW_CTRL__CMD1_MODE));
    h2fmi_busy_wait(fmi, FMC_STATUS, cmd1_addr_done, cmd1_addr_done);
    h2fmi_wr(fmi, FMC_STATUS, cmd1_addr_done);
}

static void h2fmi_send_cmd_addr_cmd(h2fmi_t *fmi, UInt8 cmd1, UInt8 cmd2, const UInt8 *address, UInt32 address_bytes)
{
    const UInt32 cmd1_cmd2_addr_done = (FMC_STATUS__ADDRESSDONE |
                                        FMC_STATUS__CMD1DONE |
                                        FMC_STATUS__CMD2DONE);

    h2fmi_wr(fmi, FMC_CMD, (FMC_CMD__CMD1(cmd1) |
                            FMC_CMD__CMD2(cmd2)));
    h2fmi_wr(fmi, FMC_ADDR0, (FMC_ADDR0__SEQ0(address[0]) |
                              FMC_ADDR0__SEQ1(address[1]) |
                              FMC_ADDR0__SEQ2(address[2]) |
                              FMC_ADDR0__SEQ3(address[3])));

    if (address_bytes > 4)
    {
        h2fmi_wr(fmi, FMC_ADDR1, (FMC_ADDR1__SEQ4(address[4]) |
                                  FMC_ADDR1__SEQ5(address[5]) |
                                  FMC_ADDR1__SEQ6(address[6]) |
                                  FMC_ADDR1__SEQ7(address[7])));
    }

    h2fmi_wr(fmi, FMC_ADDRNUM, FMC_ADDRNUM__NUM(address_bytes - 1));
    h2fmi_wr(fmi, FMC_RW_CTRL, (FMC_RW_CTRL__ADDR_MODE |
                                FMC_RW_CTRL__CMD1_MODE |
                                FMC_RW_CTRL__CMD2_MODE));

    h2fmi_busy_wait(fmi, FMC_STATUS, cmd1_cmd2_addr_done, cmd1_cmd2_addr_done);
    h2fmi_wr(fmi, FMC_STATUS, cmd1_cmd2_addr_done);
}

static void h2fmi_ppn_read_serial_output(h2fmi_t *fmi, UInt8 *buf, UInt32 len)
{
    UInt32 fullSectors;
    UInt32 fmi_control = (FMI_CONTROL__MODE__READ |
                          FMI_CONTROL__START_BIT);

#if FMI_VERSION == 4
    if (fmi->read_stream_disable)
    {
        fmi_control |= FMI_CONTROL__DISABLE_STREAMING;
    }
#endif // FMI_VERSION == 4

    h2fmi_send_cmd(fmi, NAND_CMD__READ_SERIAL_OUTPUT);

    fullSectors = len / H2FMI_BYTES_PER_SECTOR;

    if (fullSectors > 0)
    {
        h2fmi_ppn_set_format(fmi, H2FMI_BYTES_PER_SECTOR, fullSectors, FALSE32);
        h2fmi_wr(fmi, FMI_CONTROL, fmi_control);

        while(fullSectors)
        {
            h2fmi_pio_read_sector(fmi, buf, H2FMI_BYTES_PER_SECTOR);
            buf += H2FMI_BYTES_PER_SECTOR;
            fullSectors--;
            len -= H2FMI_BYTES_PER_SECTOR;
        }
    }

    if (len > 0)
    {
        h2fmi_ppn_set_format(fmi, len, 1, FALSE32);
        h2fmi_wr(fmi, FMI_CONTROL, fmi_control);
        h2fmi_pio_read_sector(fmi, buf, len);
    }

}

static BOOL32 h2fmi_get_nand_status(h2fmi_t *fmi, UInt8 *status)
{
    BOOL32 ret;

    h2fmi_prepare_for_fmi_interrupt(fmi, FMI_INT_EN__FMC_NSRBBDONE);

    h2fmi_set_if_ctrl(fmi, (~FMC_IF_CTRL__RBBEN & h2fmi_rd(fmi, FMC_IF_CTRL)));
    h2fmi_wr(fmi, FMC_RBB_CONFIG, (FMC_RBB_CONFIG__POL(0x40) |
                                   FMC_RBB_CONFIG__POS(0x40)));
    h2fmi_wr(fmi, FMC_DATANUM, FMC_DATANUM__NUM(0));
    h2fmi_wr(fmi, FMC_INTMASK, FMC_INTMASK__NSRBBDONE);
    h2fmi_wr(fmi, FMC_RW_CTRL, (FMC_RW_CTRL__REBHOLD | FMC_RW_CTRL__RD_MODE));

    ret = h2fmi_wait_for_fmi_interrupt(fmi, FMI_INT_PEND__FMC_NSRBBDONE);
    *status = (UInt8)h2fmi_rd(fmi, FMC_NAND_STATUS);

    if (!ret)
    {

//rdar://problem/11286520 Allow timeout on FW update for channel 1
#if !APPLICATION_EMBEDDEDIOP
        if (!fmi->h2fmi_ppn_panic_on_status_timeout)
        {
            return FALSE32;
        }
#endif
        // Don't panic in iBSS - we want to at least get to a prompt on errors.
        WMR_PANIC("Timeout waiting for NAND status: 0x%02x\n", *status);
    }
    else if (*status & PPN_OPERATION_STATUS__GENERAL_ERROR)
    {
        fmi->ppn->general_error = TRUE32;
    }

    h2fmi_wr(fmi, FMC_RW_CTRL, 0);
    return TRUE32;
}

BOOL32 h2fmi_ppn_get_next_operation_status(h2fmi_t *fmi, UInt8 *operation_status)
{
    BOOL32 ret;
    const UInt32 cmd1_cmd2_done = (FMC_STATUS__CMD1DONE |
                                   FMC_STATUS__CMD2DONE);

    h2fmi_wr(fmi, FMC_CMD, (FMC_CMD__CMD1(NAND_CMD__GET_NEXT_OPERATION_STATUS) |
                            FMC_CMD__CMD2(NAND_CMD__OPERATION_STATUS)));
    h2fmi_wr(fmi, FMC_RW_CTRL, (FMC_RW_CTRL__CMD1_MODE |
                                FMC_RW_CTRL__CMD2_MODE));
    h2fmi_busy_wait(fmi, FMC_STATUS, cmd1_cmd2_done, cmd1_cmd2_done);
    h2fmi_wr(fmi, FMC_STATUS, cmd1_cmd2_done);

    ret =  h2fmi_get_nand_status(fmi, operation_status);
    if (ret && (*operation_status & PPN_OPERATION_STATUS__GENERAL_ERROR))
    {
        fmi->ppn->general_error = TRUE32;
        fmi->ppn->general_error_ce = fmi->activeCe;
    }
    return ret;
}

BOOL32 h2fmi_ppn_get_next_operation_status_with_addr(h2fmi_t *fmi, UInt8 *addr, UInt8 *operation_status)
{
    h2fmi_send_cmd_addr_cmd(fmi,
                            NAND_CMD__GET_NEXT_OPERATION_STATUS,
                            NAND_CMD__OPERATION_STATUS,
                            addr,
                            3);
    return h2fmi_get_nand_status(fmi, operation_status);
}


BOOL32 h2fmi_ppn_get_controller_status(h2fmi_t *fmi, UInt8 *controller_status)
{
    BOOL32 ret;

    h2fmi_wr(fmi, FMC_CMD, FMC_CMD__CMD1(NAND_CMD__CONTROLLER_STATUS));
    h2fmi_wr(fmi, FMC_RW_CTRL, FMC_RW_CTRL__CMD1_MODE);
    h2fmi_busy_wait(fmi, FMC_STATUS, FMC_STATUS__CMD1DONE, FMC_STATUS__CMD1DONE);
    h2fmi_wr(fmi, FMC_STATUS, FMC_STATUS__CMD1DONE);

    ret = h2fmi_get_nand_status(fmi, controller_status);

    return ret;
}

#if 0
#define DUMP_VAR(variable) _WMR_PRINT("%s : %u\n", #variable, (UInt32)(variable))
#else
#define DUMP_VAR(variable) 
#endif
BOOL32 h2fmi_ppn_get_device_params(h2fmi_t *fmi, h2fmi_ce_t ce, h2fmi_ppn_device_params_t *params)
{
    UInt8 address[3] = {0, 0, 0};
    UInt8 operation_status = 0;
    BOOL32 result = TRUE32;

    WMR_ASSERT(fmi->activeCe == (h2fmi_ce_t)~0);

    h2fmi_set_if_ctrl(fmi, (FMC_IF_CTRL__REB_SETUP(FMC_IF_CTRL__TIMING_MAX_CLOCKS) |
                            FMC_IF_CTRL__REB_HOLD(FMC_IF_CTRL__TIMING_MAX_CLOCKS) |
                            FMC_IF_CTRL__WEB_SETUP(FMC_IF_CTRL__TIMING_MAX_CLOCKS) |
                            FMC_IF_CTRL__WEB_HOLD(FMC_IF_CTRL__TIMING_MAX_CLOCKS) |
                            FMC_IF_CTRL__DCCYCLE(0)));

    h2fmi_ppn_switch_ce(fmi, ce);

    h2fmi_send_cmd_addr_cmd(fmi,
                            NAND_CMD__READ_DEVICE_PARAMETERS,
                            NAND_CMD__READ_DEVICE_PARAMETERS_CONFIRM,
                            address,
                            1);

    if (!h2fmi_ppn_get_next_operation_status(fmi, &operation_status))
    {
        WMR_PRINT(ERROR, "Timeout waiting for operation status\n");
        result = FALSE32;
    }

    if (operation_status != 0x40)
    {
        WMR_PRINT(ERROR, "Invalid operation status reading PPN device parameters: 0x%02x\n",
                  operation_status);
        result = FALSE32;
    }

    if (result)
    {
        h2fmi_ppn_read_serial_output(fmi, (UInt8 *)params, 512);
        h2fmi_send_cmd(fmi, NAND_CMD__GET_NEXT_OPERATION_STATUS);
    }

    h2fmi_ppn_disable_all_ces(fmi);
    
    if (result)
    {
        DUMP_VAR(params->caus_per_channel);
        DUMP_VAR(params->cau_bits);
        DUMP_VAR(params->blocks_per_cau);
        DUMP_VAR(params->block_bits);
        DUMP_VAR(params->pages_per_block);
        DUMP_VAR(params->pages_per_block_slc);
        DUMP_VAR(params->page_address_bits);
        DUMP_VAR(params->address_bits_bits_per_cell);
        DUMP_VAR(params->default_bits_per_cell);
        DUMP_VAR(params->page_size);
        DUMP_VAR(params->dies_per_channel);
    
        DUMP_VAR(params->tRC);
        DUMP_VAR(params->tREA);
        DUMP_VAR(params->tREH);
        DUMP_VAR(params->tRHOH);
        DUMP_VAR(params->tRHZ);
        DUMP_VAR(params->tRLOH);
        DUMP_VAR(params->tRP);
        DUMP_VAR(params->tWC);
        DUMP_VAR(params->tWH);
        DUMP_VAR(params->tWP);
        
        DUMP_VAR(params->read_queue_size);
        DUMP_VAR(params->program_queue_size);
        DUMP_VAR(params->erase_queue_size);
        DUMP_VAR(params->prep_function_buffer_size);
        
        DUMP_VAR(params->tRST_ms);
        DUMP_VAR(params->tPURST_ms);
        DUMP_VAR(params->tSCE_ms);
        DUMP_VAR(params->tCERDY_us);

        DUMP_VAR(params->cau_per_channel2);
        DUMP_VAR(params->dies_per_channel2);
        DUMP_VAR(params->ddr_tRC);
        DUMP_VAR(params->ddr_tREH);
        DUMP_VAR(params->ddr_tRP);
        DUMP_VAR(params->tDQSL_ps);
        DUMP_VAR(params->tDQSH_ps);
        DUMP_VAR(params->tDSC_ps);
        DUMP_VAR(params->tDQSRE_ps);
        DUMP_VAR(params->tDQSQ_ps);
        DUMP_VAR(params->tDVW_ps);
        DUMP_VAR(params->max_interface_speed);

        if (fmi->ppn->spec_version < PPN_VERSION_1_5_0)
        {
            // PPN 1.0.x device
            fmi->ppn->bytes_per_row_address = (params->cau_bits + params->block_bits + params->page_address_bits +
                                        params->address_bits_bits_per_cell + 8 - 1) / 8;
            fmi->ppn->bytes_per_full_address = 7;
            WMR_ASSERT(sizeof(h2fmi_ppn_failure_info_ppn1_0_t) == PPN_1_0_FAILURE_INFO_EXPCT_LEN);
        }
        else
        {
            fmi->ppn->bytes_per_row_address  = 4;
            fmi->ppn->bytes_per_full_address = 8;
            WMR_ASSERT(sizeof(h2fmi_ppn_failure_info_ppn1_5_t) == PPN_1_5_FAILURE_INFO_EXPCT_LEN);
        }

        result = _validate_device_parameters(fmi->ppn);
    }

    h2fmi_set_if_ctrl(fmi, fmi->if_ctrl);
    return result;
}

BOOL32 h2fmi_ppn_init_channel(h2fmi_t *fmi,
                              BOOL32   debug_flags_valid,
                              UInt32   debug_flags,
                              BOOL32   vs_debug_flags_valid,
                              UInt32   vs_debug_flags,
                              BOOL32   allow_saving_debug_data,
                              UInt32   spec_version)
{
    h2fmi_ppn_t               *ppn;
    h2fmi_ce_t                 ce;

#if !APPLICATION_EMBEDDEDIOP
    fmi->h2fmi_ppn_panic_on_status_timeout = TRUE32;
#endif

    if (!fmi->ppn)
    {
        fmi->ppn = (h2fmi_ppn_t *)WMR_MALLOC(sizeof(h2fmi_ppn_t));
        for (ce = 0 ; ce < PPN_MAX_CES_PER_BUS ; ce++)
        {
            fmi->ppn->prep_buffer[ce] = (UInt32*)WMR_MALLOC(PPN_MAX_PAGES_PER_CE * sizeof(*fmi->ppn->prep_buffer));
        }
        //initialize error list for ppn
        fmi->error_list = (UInt32 *)WMR_MALLOC(PPN_ERROR_LIST_SIZE);
        WMR_ASSERT(fmi->error_list != NULL);
    }
    ppn    = fmi->ppn;

    ppn->debug_flags_valid       = debug_flags_valid;
    ppn->debug_flags             = debug_flags;
    ppn->vs_debug_flags_valid    = vs_debug_flags_valid;
    ppn->vs_debug_flags          = vs_debug_flags;
    ppn->allow_saving_debug_data = allow_saving_debug_data;
    ppn->spec_version            = spec_version;

#if FMISS_ENABLED
    fmiss_init_sequences(fmi);
#endif /* FMISS_ENABLED */

    return TRUE32;
}

// validate the value argument is between (inclusive) min and max
#define _validate_item(value_param, min_param, max_param) \
    { \
        if (((value_param) < (min_param)) || ((value_param) > (max_param))) \
        { \
            WMR_PRINT(ERROR, "failure with %s = %d\n", #value_param , value_param); \
            return FALSE32; \
        } \
    }


BOOL32 _validate_device_parameters(h2fmi_ppn_t *ppn)
{
    // check the device params
    _validate_item(ppn->device_params.caus_per_channel, 1, 64);
    _validate_item(ppn->device_params.cau_bits, 1, 7);
    _validate_item(ppn->device_params.blocks_per_cau, 1024, 8500);
    _validate_item(ppn->device_params.block_bits, 10, 15);
    _validate_item(ppn->device_params.pages_per_block, 64, 512);
    _validate_item(ppn->device_params.pages_per_block_slc, 64, 256);
    _validate_item(ppn->device_params.page_address_bits, 5, 9);
    _validate_item(ppn->device_params.address_bits_bits_per_cell, 1, 2);
    _validate_item(ppn->device_params.default_bits_per_cell, 1, 4);
    _validate_item(ppn->device_params.page_size, (8 * 1028), (16 * 1028)); // page size includes meta area (4 bytes per KB)
    _validate_item(ppn->device_params.tRC, 1, 254);
    _validate_item(ppn->device_params.tREA, 1, 254);
    _validate_item(ppn->device_params.tREH, 1, 254);
    _validate_item(ppn->device_params.tRHOH, 1, 254);
    _validate_item(ppn->device_params.tRHZ, 1, 254);
    _validate_item(ppn->device_params.tRLOH, 1, 254);
    _validate_item(ppn->device_params.tRP, 1, 254);
    _validate_item(ppn->device_params.tWC, 1, 254);
    _validate_item(ppn->device_params.tWH, 1, 254);
    _validate_item(ppn->device_params.tWP, 1, 254);
    _validate_item(ppn->device_params.read_queue_size, 4, (8 * 1024));
    _validate_item(ppn->device_params.program_queue_size, 2, (8 * 1024));
    _validate_item(ppn->device_params.erase_queue_size, 4, (8 * 1024));
    _validate_item(ppn->device_params.prep_function_buffer_size, 4, (8 * 1024));
    _validate_item(ppn->device_params.tRST_ms, 1, 50);
    _validate_item(ppn->device_params.tPURST_ms, 1, 50);
    _validate_item(ppn->device_params.tSCE_ms, 1, 200);
    _validate_item(ppn->device_params.tCERDY_us, 1, 200);
    _validate_item(ppn->bytes_per_row_address, 3, 4);
    _validate_item(ppn->bytes_per_full_address, 7, 8);

    if ((ppn->device_params.blocks_per_cau % 8) != 0)
    {
        WMR_PRINT(ERROR, "%d blocks per cau - rdar://8426007\n", ppn->device_params.blocks_per_cau);
        return FALSE32;
    }

    if ((UInt32)(1 << ppn->device_params.page_address_bits) != ppn->device_params.pages_per_block)
    {
        WMR_PRINT(ERROR, "pages_per_block:%d != 1 << page_address_bits:%d\n", 
                  ppn->device_params.pages_per_block, ppn->device_params.page_address_bits);
        return FALSE32;
    }

    return TRUE32;
}

BOOL32 h2fmi_ppn_post_rst_pre_pwrstate_operations(h2fmi_t* fmi)
{
    BOOL32 result = TRUE32;
    UInt32 ce_idx;

    for(ce_idx = 0; ce_idx < H2FMI_MAX_CE_TOTAL; ce_idx++)
    {
        if (fmi->valid_ces & (1 << ce_idx))
        {
            h2fmi_ppn_configure_debug_data(fmi, ce_idx);
    
#if SUPPORT_TOGGLE_NAND
            if (fmi->is_toggle_system)
            {
                if (!h2fmi_ppn15_enable_optional_signals(fmi, ce_idx))
                {
                    result = FALSE32;
                }
            }
#endif // SUPPORT_TOGGLE_NAND

        }
    }

    return result;
}

BOOL32 h2fmi_ppn_set_channel_power_state(h2fmi_t *fmi, UInt32 ps_tran)
{
    h2fmi_ce_t ce;
    Int32 result;
    UInt32 ps;

#if SUPPORT_TOGGLE_NAND
    if (!fmi->is_toggle_system)
    {
#endif
        WMR_ASSERT(ps_tran == PPN_PS_TRANS_LOW_POWER_TO_ASYNC);
#if SUPPORT_TOGGLE_NAND
    }
#endif

    ps = PS_TRANS_GET_TO(ps_tran);

    for( ce = 0; ce < H2FMI_MAX_CE_TOTAL; ce++)
    {
        if (fmi->valid_ces & (1 << ce))
        {
            WMR_PRINT(MISC, "Setting PPN Power State: 0x%x\n", ps);
            result = h2fmi_ppn_set_features(fmi, ce, PPN_FEATURE__POWER_STATE, (UInt8 *)&ps, 1, FALSE32, NULL);

#if APPLICATION_EMBEDDEDIOP
            WMR_ASSERT(result == FIL_SUCCESS);
#endif
            if (result != FIL_SUCCESS)
            {
                WMR_PRINT(ERROR, "failed setting power state on ce %d\n", ce);
                return FALSE32;
            }
        }
    }
    
#if !H2FMI_IOP
    if (PPN_FEATURE__POWER_STATE__LOW_POWER == ps)
    {
        fmi->if_ctrl = H2FMI_IF_CTRL_LOW_POWER;
    }
    else
    {
        fmi->if_ctrl = fmi->if_ctrl_normal;
    }
    restoreTimingRegs(fmi);
#endif // !H2FMI_IOP

    return TRUE32;
}

#if !APPLICATION_EMBEDDEDIOP
void h2fmi_ppn_fill_nandinfo(h2fmi_t *fmi, NandInfo *nandInfo, NandRequiredTimings *requiredTiming)
{
    h2fmi_ppn_t               *ppn          = fmi->ppn;
    h2fmi_ppn_device_params_t *params       = &ppn->device_params;
    NandPpn                   *nandPpn      = &ppn->nandPpn;
    
    nandPpn->blocksPerCau        = params->blocks_per_cau;    
    nandPpn->causPerCe           = params->caus_per_channel;
    nandPpn->slcPagesPerBlock    = params->pages_per_block_slc;
    nandPpn->mlcPagesPerBlock    = params->pages_per_block;
    nandPpn->matchOddEvenCaus    = params->block_pairing_scheme;
    nandPpn->bitsPerCau          = params->cau_bits;
    nandPpn->bitsPerPage         = params->page_address_bits;
    nandPpn->bitsPerBlock        = params->block_bits;
    nandPpn->maxTransactionSize  = params->prep_function_buffer_size;
    nandPpn->specVersion         = ppn->spec_version;

    fmi->blocks_per_ce        = params->blocks_per_cau * params->caus_per_channel;
    fmi->banks_per_ce         = params->caus_per_channel; // Banks are CAUs in PPN
    fmi->bytes_per_page       = 1 << WMR_LOG2(params->page_size);
    fmi->sectors_per_page     = fmi->bytes_per_page / H2FMI_BYTES_PER_SECTOR;
    fmi->pages_per_block      = params->pages_per_block;
    fmi->bytes_per_spare      = params->page_size - fmi->bytes_per_page;
    fmi->dies_per_cs          = params->dies_per_channel;    
    
    // Cache page config registers
    fmi->correctable_bits = 30; // only used for Bonfire reporting
    fmi->refresh_threshold_bits = 0;

    WMR_MEMSET(requiredTiming, 0, sizeof(*requiredTiming));

    requiredTiming->clock_hz = WMR_BUS_FREQ_HZ();
#if FMI_USE_WRITE_CYCLE_BALANCING
    requiredTiming->balance_write_cycle = TRUE32;
#endif
    
    requiredTiming->soc_tx_prop_ns = FMI_TX_PROP_DELAY_NS;
    requiredTiming->soc_rx_prop_ns = FMI_RX_PROP_DELAY_NS;
    
    requiredTiming->tRC_ns = params->tRC;
    requiredTiming->tRP_ns = params->tRP;
    requiredTiming->tREH_ns = params->tREH;
    requiredTiming->tREA_ns = params->tREA;
    requiredTiming->tRHOH_ns = params->tRHOH;
    requiredTiming->tRLOH_ns = params->tRLOH;

    requiredTiming->tWC_ns = params->tWC;
    requiredTiming->tWP_ns = params->tWP;
    requiredTiming->tWH_ns = params->tWH;
    
#if SUPPORT_TOGGLE_NAND
    if (fmi->is_toggle_system)
    {
        requiredTiming->ddr_tRC_ps = params->ddr_tRC * PS_PER_NS;
        requiredTiming->ddr_tREH_ps = params->ddr_tREH * PS_PER_NS;
        requiredTiming->ddr_tRP_ps = params->ddr_tRP * PS_PER_NS;

        requiredTiming->tDSC_ps = params->tDSC_ps;
        requiredTiming->tDQSH_ps = params->tDQSH_ps;
        requiredTiming->tDQSL_ps = params->tDQSL_ps;
    }
#endif
    
    fmi->ppn->boardSupport.vsType = FIL_VS_UNKNOWN;
    nandInfo->boardSupport = &fmi->ppn->boardSupport;
    
    nandInfo->format = getPPNFormat();
    
    nandInfo->geometry = &fmi->ppn->geometry;
    
    fmi->ppn->geometry.blocksPerCS = fmi->blocks_per_ce;
    fmi->ppn->geometry.pagesPerBlock = fmi->pages_per_block;
    fmi->ppn->geometry.dataBytesPerPage = fmi->bytes_per_page;
    fmi->ppn->geometry.spareBytesPerPage = fmi->bytes_per_spare;
    fmi->ppn->geometry.eccPer512Bytes = 0;
    fmi->ppn->geometry.initialBBType = INIT_BBT_PPN;
    fmi->ppn->geometry.diesPerCS = fmi->dies_per_cs;
    
}
#endif /* !APPLICATION_EMBEDDEDIOP */

UInt32 h2fmi_ppn_calculate_fmi_config(h2fmi_t *fmi)
{
    UInt32 fmi_config = FMI_CONFIG__ECC_CORRECTABLE_BITS(0) |
                        FMI_CONFIG__DMA_BURST(WMR_LOG2(H2FMI_DMA_BURST_CYCLES));

#if FMI_VERSION >= 4
    fmi_config |= FMI_CONFIG__META_DMA_BURST__4_CYCLES |
                  FMI_CONFIG__META_DMA_BURST__4_BYTES;
#endif // FMI_VERSION >= 4

    return fmi_config;
}

UInt32 h2fmi_ppn_calculate_fmi_data_size(h2fmi_t *fmi)
{
    return (FMI_DATA_SIZE__META_BYTES_PER_SECTOR( fmi->valid_bytes_per_meta) |
            FMI_DATA_SIZE__META_BYTES_PER_PAGE( fmi->valid_bytes_per_meta) |
            FMI_DATA_SIZE__BYTES_PER_SECTOR( H2FMI_BYTES_PER_SECTOR) |
            FMI_DATA_SIZE__SECTORS_PER_PAGE( fmi->sectors_per_page));
}

static BOOL32 h2fmi_ppn_switch_ce(h2fmi_t *fmi, h2fmi_ce_t ce)
{
    if (fmi->activeCe != ce)
    {
        h2fmi_fmc_disable_all_ces(fmi);
        h2fmi_fmc_enable_ce(fmi, ce);
        fmi->activeCe = ce;
        return TRUE32;
    }

    return FALSE32;
}

static void h2fmi_ppn_disable_all_ces(h2fmi_t *fmi)
{
    h2fmi_fmc_disable_all_ces(fmi);
    fmi->activeCe = (h2fmi_ce_t)~0;
}

static void h2fmi_prepare_for_fmi_interrupt(h2fmi_t *fmi, UInt32 condition)
{
	h2fmi_clear_interrupts_and_reset_masks(fmi);
#if H2FMI_WAIT_USING_ISR
	h2fmi_wr(fmi, FMI_INT_EN, condition);
#endif /* H2FMI_WAIT_USING_ISR */
}

static BOOL32 h2fmi_wait_for_fmi_interrupt(h2fmi_t *fmi, UInt32 condition)
{
#if H2FMI_WAIT_USING_ISR
    if (!event_wait_timeout(&fmi->isr_event, H2FMI_PAGE_TIMEOUT_MICROS) ||
        !(fmi->isr_condition & condition))
	{
		return FALSE32;
	}
	else
	{
		return TRUE32;
	}
#else
	BOOL32 ret;

	ret = h2fmi_wait_done(fmi, FMI_INT_PEND, condition, condition);
	h2fmi_wr(fmi, FMI_INT_PEND, condition);
	return ret;
#endif /* H2FMI_WAIT_USING_ISR */
}

#if SUPPORT_TOGGLE_NAND
BOOL32 h2fmi_ppn15_get_temperature(h2fmi_t *fmi, h2fmi_ce_t ce, Int16 *temperature)
{
    UInt8 status;

    if (!h2fmi_ppn_get_feature(fmi, ce, PPN_FEATURE__GET_DEVICE_TEMPERATURE, (UInt8*)temperature, PPN_FEATURE_LENGTH_TEMPERATURE, &status))
    {
        WMR_PRINT(ERROR, "Get Temperature failed FMI %d CE %d, Status: 0x%x\n", fmi->bus_id, ce, status);
        return FALSE32;
    }

    *temperature -= 273; // convert to Celsius

    return TRUE32;
}

BOOL32 h2fmi_ppn15_enable_optional_signals(h2fmi_t *fmi, h2fmi_ce_t ce)
{
    const UInt32 enable = 1;
    BOOL32 ret = TRUE32;

    if (fmi->useDiffDQS)
    {
        WMR_PRINT(ALWAYS, "Enabling Diff DQS on CE %d\n", ce);
        if (FIL_SUCCESS != h2fmi_ppn_set_features(fmi, ce, PPN_FEATURE__DQS_COMPLEMENT_ENABLE, (UInt8 *)&enable, PPN_FEATURE_LENGTH_DEFAULT, FALSE32, NULL))
        {
            WMR_PRINT(ERROR, "Enabling Diff DQS failed on CE %d\n", ce);
            ret = FALSE32;
        }
    }

    if (fmi->useDiffRE)
    {
        WMR_PRINT(ALWAYS, "Enabling Diff RE on CE %d\n", ce);
        if (FIL_SUCCESS != h2fmi_ppn_set_features(fmi, ce, PPN_FEATURE__RE_COMPLEMENT_ENABLE, (UInt8 *)&enable, PPN_FEATURE_LENGTH_DEFAULT, FALSE32, NULL))
        {
            WMR_PRINT(ERROR, "Enabling Diff RE failed on CE %d\n", ce);
            ret = FALSE32;
        }
    }

    if (fmi->useVREF)
    {
        Int32 ret;
        WMR_PRINT(ALWAYS, "Enabling VREF on CE %d\n", ce);
        if(FIL_SUCCESS != (ret = h2fmi_ppn_set_features(fmi, ce, PPN_FEATURE__VREF_ENABLE, (UInt8 *)&enable, PPN_FEATURE_LENGTH_DEFAULT, (fmi->bus_id == 1) ? TRUE32 : FALSE32, NULL)))
        {
            if(!((fmi->bus_id == 1) && (ret == FIL_UNSUPPORTED_ERROR)))
            {
                WMR_PRINT(ERROR, "Enabling VREF failed on CE %d\n", ce);
                ret = FALSE32;
            }
        }
    }

    return ret;
}
#endif

Int32 h2fmi_ppn_configure_debug_data(h2fmi_t   *fmi,
                                     h2fmi_ce_t ce)
{
    if (fmi->ppn->allow_saving_debug_data)
    {
        const UInt32 debugInfo = PPN_FEATURE__ALLOW_SAVING_DEBUG_DATA_ENABLE;

        h2fmi_ppn_set_features(fmi,
                               ce,
                               PPN_FEATURE__ALLOW_SAVING_DEBUG_DATA,
                               (UInt8 *)&debugInfo,
                               PPN_FEATURE_LENGTH_DEFAULT,
                               FALSE32,
                               NULL);
    }

    if (fmi->ppn->debug_flags_valid)
    {
        WMR_PRINT(ALWAYS, "Enabling PPN generic debug flags 0x%08x on CE %d\n",
                  fmi->ppn->debug_flags, ce);

        h2fmi_ppn_set_features(fmi,
                               ce,
                               PPN_FEATURE__SET_DEBUG_DATA_GENERIC_CONFIG,
                               (UInt8*) &fmi->ppn->debug_flags,
                               PPN_FEATURE_LENGTH_DEFAULT,
                               FALSE32,
                               NULL);
    }

    if (fmi->ppn->vs_debug_flags_valid)
    {
        WMR_PRINT(ALWAYS, "Enabling PPN vendor-specific debug flags 0x%08x on CE %d\n",
                  fmi->ppn->vs_debug_flags, ce);

        h2fmi_ppn_set_features(fmi,
                               ce,
                               PPN_FEATURE__SET_DEBUG_DATA_VENDOR_SPECIFIC,
                               (UInt8 *)&fmi->ppn->vs_debug_flags,
                               PPN_FEATURE_LENGTH_DEFAULT,
                               FALSE32,
                               NULL);
    }

    return FIL_SUCCESS;
}

void h2fmi_ppn_dma_debug_data_payload(h2fmi_t            *fmi,
                                      h2fmi_ce_t          ce,
                                      UInt32              page,
                                      UInt32              pageCount,
                                      struct dma_segment *sgl,
                                      BOOL32              waitFortGDD)
{
    UInt32 i;
    UInt8  status = 0;

    h2fmi_reset(fmi);

    h2fmi_ppn_switch_ce(fmi, ce);

/*  Sometimes Hynix PPN parts need a reset (particularly when dealing with timeouts), but
    generally we shouldn't need to.

    h2fmi_ppn_reset(fmi);

    h2fmi_send_cmd(fmi, NAND_CMD__LOW_POWER_READ_STATUS);
    h2fmi_get_nand_status(fmi, &status);
    WMR_ASSERT(status == (PPN_LOW_POWER_STATUS__WRITE_PROTECT_BAR | PPN_LOW_POWER_STATUS__READY));
*/
    h2fmi_dma_execute_async(DMA_CMD_DIR_RX,
                            h2fmi_dma_data_chan(fmi),
                            sgl,
                            h2fmi_dma_data_fifo(fmi),
                            pageCount * PPN_PERFECT_PAGE_SIZE,
                            sizeof(UInt32),
                            32,
                            NULL);

    for (i = page; i < page + pageCount; i++)
    {
        h2fmi_send_cmd_addr_cmd(fmi,
                                NAND_CMD__GET_DEBUG_DATA,
                                NAND_CMD__SET_GET_FEATURES_CONFIRM,
                                (UInt8 *)&i,
                                fmi->ppn->bytes_per_row_address);

        if (waitFortGDD)
        {
            WMR_SLEEP_US(fmi->ppn->spec_version < PPN_VERSION_1_5_0 ? PPN_tGDD_1_0_US : PPN_tGDD_1_5_US);
        }

        h2fmi_send_cmd(fmi, NAND_CMD__OPERATION_STATUS);
        if (!h2fmi_get_nand_status(fmi, &status))
        {
            WMR_PANIC("Timeout getting nand status");
        }

        h2fmi_send_cmd(fmi, NAND_CMD__READ_SERIAL_OUTPUT);

        h2fmi_ppn_set_format(fmi, PPN_PERFECT_FMI_SECTOR_SIZE, PPN_PERFECT_FMI_SECTORS_PER_PERFECT_PAGE, FALSE32);
        h2fmi_ppn_start_fmi_read_and_wait(fmi);
    }

    if (!h2fmi_dma_wait(h2fmi_dma_data_chan(fmi), H2FMI_PAGE_TIMEOUT_MICROS))
    {
        WMR_PRINT(ERROR, "Timeout waiting for CDMA channel %d to complete\n",
                  h2fmi_dma_data_chan(fmi));
        h2fmi_dma_cancel(h2fmi_dma_data_chan(fmi));
    }

    h2fmi_ppn_disable_all_ces(fmi);
}

void h2fmi_ppn_force_geb_address(h2fmi_t *fmi, h2fmi_ce_t ce, PageAddressType addr)
{
    h2fmi_reset(fmi);

    WMR_PRINT(ERROR, "Sending SET_DEBUG_DATA to collect page on bus %d ce %d page 0x%08x\n",
              fmi->bus_id, ce, addr);
    h2fmi_ppn_set_debug_data(fmi, ce, PPN_SET_DEBUG_DATA__FORCE_ADDRESS, (UInt8 *)&addr, 4);
    WMR_PRINT(ERROR, "Sent command to force GEB\n");

    fmi->ppn->general_error = TRUE32;
    fmi->ppn->general_error_ce = ce;
}

void h2fmi_ppn_set_feature_list(h2fmi_t *fmi, ppn_feature_entry_t *list, UInt32 size)
{
    UInt32 i;

    for (i = 0 ; i < size ; i++)
    {
        ppn_feature_entry_t entry;
        h2fmi_ce_t ce;

        WMR_MEMCPY(&entry, &list[i], sizeof(entry));

        if (entry.version > fmi->ppn->spec_version)
        {
            continue;
        }

        for (ce = 0 ; (ce < H2FMI_MAX_CE_TOTAL) && (ce <= fmi->valid_ces) ; ce++)
        {
            if (0 != (fmi->valid_ces & (1 << ce)))
            {
                Int32 result;
                result = h2fmi_ppn_set_features(fmi, ce, entry.feature, (UInt8*)&entry.value, entry.bytes, TRUE32, NULL);
                WMR_ASSERT((FIL_SUCCESS == result) || (FIL_UNSUPPORTED_ERROR == result));
            }
        }
    }
}

#if H2FMI_PPN_VERIFY_SET_FEATURES

static void _update_feature_shadow(h2fmi_t *fmi, UInt16 feature, UInt8 *value, UInt32 len)
{
    UInt32 i;

    for (i = 0 ; i < _ppn_feature_shadow[fmi->bus_id].count ; i++)
    {
        if (feature == _ppn_feature_shadow[fmi->bus_id].list[i].feature)
        {
            break;
        }
    }

    if (i >= _ppn_feature_shadow[fmi->bus_id].count)
    {
        WMR_ASSERT(i < NUMELEMENTS(_ppn_feature_shadow[fmi->bus_id].list));
        _ppn_feature_shadow[fmi->bus_id].list[i].feature = feature;
        _ppn_feature_shadow[fmi->bus_id].count++;
    }

    _ppn_feature_shadow[fmi->bus_id].list[i].length = WMR_MIN(sizeof(_ppn_feature_shadow[fmi->bus_id].list[i].value), len);
    WMR_MEMCPY(&_ppn_feature_shadow[fmi->bus_id].list[i].value, value, _ppn_feature_shadow[fmi->bus_id].list[i].length);

    if (PPN_FEATURE__POWER_STATE == feature)
    {
        _ppn_feature_shadow[fmi->bus_id].state = 1UL << *value;
    }
}

void h2fmi_ppn_verify_feature_shadow(h2fmi_t *fmi)
{
    UInt32 ce_mask = fmi->valid_ces;
    BOOL32 failed = FALSE32;

    while (0 != ce_mask)
    {
        const h2fmi_ce_t ce = WMR_LOG2(ce_mask);
        UInt32 i;

        for (i = 0 ; i < _ppn_feature_shadow[fmi->bus_id].count ; i++)
        {
            UInt8 status;
            UInt8 value[sizeof(_ppn_feature_shadow[fmi->bus_id].list[i].value)];
            UInt32 statesIdx;

            for (statesIdx = 0 ; statesIdx < NUMELEMENTS(_ppn_feature_states) ; statesIdx++)
            {
                if (_ppn_feature_states[statesIdx].feature == _ppn_feature_shadow[fmi->bus_id].list[i].feature)
                {
                    break;
                }
            }
            if ((statesIdx < NUMELEMENTS(_ppn_feature_states)) &&
                (0 == (_ppn_feature_states[statesIdx].states & _ppn_feature_shadow[fmi->bus_id].state)))
            {
                continue;
            }

            if (!h2fmi_ppn_get_feature(fmi, ce, _ppn_feature_shadow[fmi->bus_id].list[i].feature, value, _ppn_feature_shadow[fmi->bus_id].list[i].length, &status))
            {
                if ((PPN_FEAT_OPERATION_STATUS__UNSUPPORTED != status) && (PPN_FEAT_OPERATION_STATUS__UNSUPPORTED_BY_CH != status))
                {
                    WMR_PRINT(ERROR, "Verify set feature of 0x%04x failed for FMI %d CE %d, Status: 0x%02x\n", _ppn_feature_shadow[fmi->bus_id].list[i].feature, fmi->bus_id, ce, status);
                    failed = TRUE32;
                }
            }
            else if (0 != WMR_MEMCMP(&_ppn_feature_shadow[fmi->bus_id].list[i].value, value, _ppn_feature_shadow[fmi->bus_id].list[i].length))
            {
                UInt32 j;
                WMR_PRINT(ERROR, "Verify set feature of 0x%04x failed for FMI %d CE %d,\n", _ppn_feature_shadow[fmi->bus_id].list[i].feature, fmi->bus_id, ce);
                WMR_PRINT(ERROR, "Expected: Actual:\n");
                for (j = 0 ; j < _ppn_feature_shadow[fmi->bus_id].list[i].length ; j++)
                {
                    UInt8 expected = ((UInt8 *)&_ppn_feature_shadow[fmi->bus_id].list[i].value)[j];
                    WMR_PRINT(ERROR, "0x%02x      0x%02x\n", expected, value[j]);
                }
                failed = TRUE32;
            }
        }

        ce_mask &= ~(1UL << ce);
    }

    WMR_ASSERT(!failed);
}

void h2fmi_ppn_reset_feature_shadow(h2fmi_t *fmi)
{
    _ppn_feature_shadow[fmi->bus_id].count = 0;
}

#endif // H2FMI_PPN_VERIFY_SET_FEATURES

#endif //FMI_VERSION > 0
