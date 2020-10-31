// *****************************************************************************
//
// File: fmiss_ppn.c
//
// Copyright (C) 2010 Apple Inc. All rights reserved.
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

#if FMISS_ENABLED

#define FMISS_TIMEOUT_VALUE(f) (1 * 1000 * f->clock_speed_khz)
#define FMISS_NS_TO_CLKS(f, t) (t * f->clock_speed_khz / (1000 * 1000))

// sequencer "wait_for_ready" command interrupt codes
#define SEQ_WFR_CODE_UNUSED       (0x00)
#define SEQ_WFR_CODE_WRITE_STATUS (0x01)
#define SEQ_WFR_CODE_INVALID      (0xFF)

// sequencer "send_interrupt" command interrupt codes
#define SEQ_INT_CODE_UNUSED       (0xFF0000)
#define SEQ_INT_CODE_DRAIN_STATUS (0xFF0001)

#define MACRO_LENGTH(s) \
    (sizeof((UInt32[]){s})/sizeof(UInt32))

#if FMI_VERSION <= 3

#define GET_OPERATION_STATUS_SEQUENCE \
    CMD_COMMAND(NAND_CMD__GET_NEXT_OPERATION_STATUS),   \
    CMD_COMMAND(NAND_CMD__OPERATION_STATUS),        \
    CMD_TIMED_WAIT(0),                              \
    CMD_WAIT_FOR_READY(SEQ_WFR_CODE_UNUSED, 0x40, 0x40), \
    CMD_SEND_INTERRUPT(SEQ_INT_CODE_UNUSED), \
    CMD_PAUSE

#else // FMI_VERSION <= 3

#define GET_OPERATION_STATUS_SEQUENCE \
    CMD_COMMAND(NAND_CMD__GET_NEXT_OPERATION_STATUS),   \
    CMD_COMMAND(NAND_CMD__OPERATION_STATUS),        \
    CMD_TIMED_WAIT(0),                              \
    CMD_WAIT_FOR_READY(SEQ_WFR_CODE_UNUSED, 0x40, 0x40), \
    CMD_STORE_TO_FIFO(FMC_NAND_STATUS)

#define STORE_CONTROLLER_STATUS_SEQUENCE \
    CMD_COMMAND(NAND_CMD__CONTROLLER_STATUS),   \
    CMD_TIMED_WAIT(0),                          \
    CMD_WAIT_FOR_READY(SEQ_WFR_CODE_WRITE_STATUS, 0x51, 0x40), \
    CMD_STORE_TO_FIFO(FMC_NAND_STATUS)

#endif // FMI_VERSION <= 3

#define GET_CONTROLLER_STATUS_SEQUENCE \
    CMD_COMMAND(NAND_CMD__CONTROLLER_STATUS),   \
    CMD_TIMED_WAIT(0),                          \
    CMD_WAIT_FOR_READY(SEQ_WFR_CODE_UNUSED, 0x40, 0x40), \
    CMD_SEND_INTERRUPT(SEQ_INT_CODE_UNUSED), \
    CMD_PAUSE

#define PREP_READ_SEQUENCE \
    CMD_LOAD_FROM_FIFO(FMC_ADDRNUM), \
    CMD_LOAD_FROM_FIFO(FMC_ADDR0), \
    CMD_LOAD_FROM_FIFO(FMC_ADDR1), \
    CMD_LOAD_FROM_FIFO(FMC_CMD), \
    CMD_LOAD_NEXT_WORD(FMC_RW_CTRL), \
    FMC_RW_CTRL__CMD1_MODE | FMC_RW_CTRL__ADDR_MODE | FMC_RW_CTRL__CMD2_MODE, \
    CMD_POLL(FMC_STATUS), \
    FMC_STATUS__ADDRESSDONE | FMC_STATUS__CMD1DONE | FMC_STATUS__CMD2DONE, \
    FMC_STATUS__ADDRESSDONE | FMC_STATUS__CMD1DONE | FMC_STATUS__CMD2DONE, \
    CMD_LOAD_NEXT_WORD(FMC_STATUS), \
    FMC_STATUS__ADDRESSDONE | FMC_STATUS__CMD1DONE | FMC_STATUS__CMD2DONE, \
    CMD_TIMED_WAIT(1)

#define START_WRITE_PAGE_SEQUENCE \
    CMD_LOAD_FROM_FIFO(FMC_ADDR0), \
    CMD_LOAD_FROM_FIFO(FMC_CMD), \
    CMD_LOAD_NEXT_WORD(FMC_RW_CTRL), \
    FMC_RW_CTRL__CMD1_MODE | FMC_RW_CTRL__ADDR_MODE, \
    CMD_POLL(FMC_STATUS), \
    FMC_STATUS__CMD1DONE | FMC_STATUS__ADDRESSDONE, \
    FMC_STATUS__CMD1DONE | FMC_STATUS__ADDRESSDONE, \
    CMD_LOAD_NEXT_WORD(FMC_STATUS), \
    FMC_STATUS__CMD1DONE | FMC_STATUS__ADDRESSDONE

#define PREP_WRITE_SEQUENCE \
    CMD_LOAD_NEXT_WORD(FMC_CMD),                        \
    NAND_CMD__MULTIPAGE_PREP,                           \
    CMD_LOAD_FROM_FIFO(FMC_ADDR0),                      \
    CMD_LOAD_NEXT_WORD(FMC_ADDRNUM),                    \
    2,                                                  \
    CMD_LOAD_NEXT_WORD(FMC_RW_CTRL),                    \
    FMC_RW_CTRL__CMD1_MODE | FMC_RW_CTRL__ADDR_MODE,    \
    CMD_POLL(FMC_STATUS),                               \
    FMC_STATUS__ADDRESSDONE | FMC_STATUS__CMD1DONE,     \
    FMC_STATUS__ADDRESSDONE | FMC_STATUS__CMD1DONE,     \
    CMD_LOAD_NEXT_WORD(FMC_STATUS),                     \
    FMC_STATUS__ADDRESSDONE | FMC_STATUS__CMD1DONE,

// fmiss_put_* APIs implemented as macros so "value" can be a conditional statement evaluated at push-time

#define fmiss_put_command(fmi, value, level, timeout) \
    do \
    { \
        BOOL32 first = TRUE32; \
    \
        while ((COMMAND_FIFO_SIZE <= *(level)) && (!*(timeout)) && \
               ((NULL == (fmi)->fmiss_cxt) || _kIOPFMI_STATUS_SUCCESS == *((fmiss_cxt_t *)(fmi)->fmiss_cxt)->fmi_status)) \
        { \
    \
            if (!first) \
            { \
                if (!fmiss_yield(fmi)) \
                { \
                    *(timeout) = TRUE32; \
                    break; \
                } \
            } \
            else \
            { \
                first = FALSE32; \
            } \
            *(level) = COMMAND_FIFO_PTR__LEVEL(h2fmi_rd(fmi, COMMAND_FIFO_PTR)); \
        } \
    \
        if (COMMAND_FIFO_SIZE > *(level)) \
        { \
            h2fmi_wr(fmi, COMMAND_FIFO, value); \
            (*(level))++; \
        } \
    \
    } \
    while(0)

#define fmiss_put_operand(fmi, value, level, timeout) \
    do \
    { \
        BOOL32 first = TRUE32; \
    \
        while ((OPERAND_FIFO_SIZE <= *(level)) && (!*(timeout)) && \
               ((NULL == (fmi)->fmiss_cxt) || _kIOPFMI_STATUS_SUCCESS == *((fmiss_cxt_t *)(fmi)->fmiss_cxt)->fmi_status)) \
        { \
    \
            if (!first) \
            { \
                if (!fmiss_yield(fmi)) \
                { \
                    *(timeout) = TRUE32; \
                    break; \
                } \
            } \
            else \
            { \
                first = FALSE32; \
            } \
            *(level) = OPERAND_FIFO_PTR__LEVEL(h2fmi_rd(fmi, OPERAND_FIFO_PTR)); \
        } \
    \
        if (OPERAND_FIFO_SIZE > *(level)) \
        { \
            h2fmi_wr(fmi, OPERAND_FIFO, value); \
            (*(level))++; \
        } \
    \
    } \
    while(0)


static UInt32 sequencer_macros[] =
{
    GET_OPERATION_STATUS_SEQUENCE,
    GET_CONTROLLER_STATUS_SEQUENCE,
#if FMI_VERSION >= 4
    STORE_CONTROLLER_STATUS_SEQUENCE,
#endif // FMI_VERSION >= 4
    PREP_READ_SEQUENCE,
    START_WRITE_PAGE_SEQUENCE,
    PREP_WRITE_SEQUENCE
};

static const UInt32 sequencer_macro_count = sizeof(sequencer_macros) / sizeof(sequencer_macros[0]);

static const UInt32 timed_wait_ns[] =
{
    PPN_TIMING_MIN_TRHW_NS,
    PPN_TIMING_MIN_TWHR_NS,
};

static const UInt32 timed_wait_count = sizeof(timed_wait_ns) / sizeof(timed_wait_ns[0]);

typedef enum
{
    kGetOperationStatusHead,
    kGetOperationStatusLength = MACRO_LENGTH(GET_OPERATION_STATUS_SEQUENCE),
    kGetOperationStatusTail = kGetOperationStatusHead + kGetOperationStatusLength - 1,

    kGetControllerStatusHead,
    kGetControllerStatusLength = MACRO_LENGTH(GET_CONTROLLER_STATUS_SEQUENCE),
    kGetControllerStatusTail = kGetControllerStatusHead + kGetControllerStatusLength - 1,

#if FMI_VERSION >= 4
    kStoreControllerStatusHead,
    kStoreControllerStatusLength = MACRO_LENGTH(STORE_CONTROLLER_STATUS_SEQUENCE),
    kStoreControllerStatusTail = kStoreControllerStatusHead + kStoreControllerStatusLength - 1,
#endif // FMI_VERSION >= 4

    kPrepReadHead,
    kPrepReadLength = MACRO_LENGTH(PREP_READ_SEQUENCE),
    kPrepReadTail = kPrepReadHead + kPrepReadLength - 1,

    kStartWritePageHead,
    kStartWritePageLength = MACRO_LENGTH(START_WRITE_PAGE_SEQUENCE),
    kStartWritePageTail = kStartWritePageHead + kStartWritePageLength - 1,

    kPrepWriteHead,
    kPrepWriteLength = MACRO_LENGTH(PREP_WRITE_SEQUENCE),
    kPrepWriteTail = kPrepWriteHead + kPrepWriteLength - 1
} sequence_entry_t;

typedef struct
{
    PPNCommandStruct   *command;
    UInt32             *fmi_status;
    UInt8              *ppn_status;
    UInt32             *status_page;
    UInt32             *store_level;
    BOOL32             *error;
} fmiss_cxt_t;


static UInt8 fmiss_ce_index_to_physical(const PPNCommandStruct *ppnCommand, h2fmi_ce_t ceIndex);
static UInt32 fmiss_ce_reg_to_logical(h2fmi_t *fmi, UInt32 ceCtrl);
static UInt8 fmiss_get_controller_status(h2fmi_t *fmi, UInt32 *command_level, BOOL32 *timeout);
#if FMI_VERSION <= 3
static UInt8 fmiss_get_next_operation_status(h2fmi_t *fmi, UInt32 *command_level, BOOL32 *timeout);
#endif // FMI_VERSION <= 3
static BOOL32 fmiss_ppn_prep_write_multi(h2fmi_t *fmi, const PPNCommandStruct *ppnCommand, UInt32 *command_level, UInt32 *operand_level);

void fmiss_init_sequences(h2fmi_t *fmi)
{
    static BOOL32 initialized = FALSE32;
    UInt32 i, j;

    if (!initialized)
    {
        for (i = 0; i < sequencer_macro_count; i++)
        {
            if (CMD_TIMED_WAIT(0) != (COMMAND_FIFO__COMMAND(~0UL) & sequencer_macros[i]))
            {
                continue;
            }
            for (j = 0; j < timed_wait_count; j++)
            {
                if (CMD_TIMED_WAIT(j) == sequencer_macros[i])
                {
                    const UInt32 clks = FMISS_NS_TO_CLKS(fmi, timed_wait_ns[j]);
                    sequencer_macros[i] = CMD_TIMED_WAIT(clks);
                    break;
                }
            }
        }
        initialized = TRUE32;
    }

    for (i = 0, j = 0 ; i < sequencer_macro_count ; i++, j += sizeof(*sequencer_macros))
    {
        h2fmi_wr(fmi, SEQUENCER_MACROS(j), sequencer_macros[i]);
    }
}

#if FMI_VERSION >= 4

static void fmiss_ppn_handle_drain_status(h2fmi_t *fmi, UInt32 count)
{
    fmiss_cxt_t *cxt = fmi->fmiss_cxt;
    PPNCommandEntry *entry = &cxt->command->entry[*cxt->status_page];

    *cxt->store_level = STORE_FIFO_PTR__LEVEL(h2fmi_rd(fmi, STORE_FIFO_PTR));

    if ((0 == count) || (*cxt->store_level < count))
    {
        count = *cxt->store_level;
    }

    while (((0 < count) || cxt->error[entry->ceIdx]) &&
           (cxt->command->num_pages > *cxt->status_page))
    {
        if (!cxt->error[entry->ceIdx])
        {
            UInt32 status = h2fmi_rd(fmi, STORE_FIFO);
            (*cxt->store_level)--;
            count--;
            *cxt->ppn_status |= status;
            entry->status = status;
        }
        (*cxt->status_page)++;
        entry++;
    }
}

static void fmiss_ppn_handle_command_fixup(h2fmi_t *fmi)
{
    const UInt32 fmc_ce_ctrl = h2fmi_rd(fmi, FMC_CE_CTRL);
    const UInt32 operand_fifo_ptr = h2fmi_rd(fmi, OPERAND_FIFO_PTR);
    const UInt32 command_fifo_ptr = h2fmi_rd(fmi, COMMAND_FIFO_PTR);
    const UInt32 operand_count = OPERAND_FIFO_PTR__LEVEL(operand_fifo_ptr);
    const UInt32 command_count = COMMAND_FIFO_PTR__LEVEL(command_fifo_ptr);
    UInt32 operand_list[OPERAND_FIFO_SIZE];
    UInt32 command_list[COMMAND_FIFO_SIZE];
    BOOL32 failingCe = TRUE32;
    UInt32 i;

    WMR_ASSERT(OPERAND_FIFO_SIZE >= operand_count);
    WMR_ASSERT(COMMAND_FIFO_SIZE >= command_count);

    for (i = 0 ; i < operand_count ; i++)
    {
        const UInt32 fifoIdx = (OPERAND_FIFO_PTR__READ(operand_fifo_ptr) + i) % OPERAND_FIFO_SIZE * sizeof(UInt32);
        operand_list[i] = h2fmi_rd(fmi, SEQUENCER_OPERANDS(fifoIdx));
    }

    for (i = 0 ; i < command_count ; i++)
    {
        const UInt32 fifoIdx = (COMMAND_FIFO_PTR__READ(command_fifo_ptr) + i) % COMMAND_FIFO_SIZE * sizeof(UInt32);
        command_list[i] = h2fmi_rd(fmi, SEQUENCER_COMMANDS(fifoIdx));
    }

    h2fmi_wr(fmi, FMI_CONTROL, FMI_CONTROL__RESET_SEQUENCER | FMI_CONTROL__ENABLE_SEQUENCER | h2fmi_rd(fmi, FMI_CONTROL));

    // unfortunately, the sequencer reset does not propagate to the macro execution block: rdar://problem/10188242
    // wait for and discard the pending status store operation

    while (0 == STORE_FIFO_PTR__LEVEL(h2fmi_rd(fmi, STORE_FIFO_PTR)))
    {
        WMR_YIELD();
    }
    h2fmi_rd(fmi, STORE_FIFO);

    for (i = 0 ; i < operand_count ; i++)
    {
        h2fmi_wr(fmi, OPERAND_FIFO, operand_list[i]);
    }

    // fix-up outstanding "enable_chip" and "get_controller_status" commands for failing CE

    for (i = 0 ; i < command_count ; i++)
    {
        if (CMD_ENABLE_CHIP(fmc_ce_ctrl) == command_list[i])
        {
            failingCe = TRUE32;
            command_list[i] = CMD_ENABLE_CHIP(0);
        }
        else if (CMD_ENABLE_CHIP(0) == (COMMAND_FIFO__COMMAND(~0UL) & command_list[i]))
        {
            failingCe = FALSE32;
        }
        else if (failingCe &&
                 (CMD_MACRO(kStoreControllerStatusHead, kStoreControllerStatusLength) == command_list[i]))
        {
            continue;
        }

        h2fmi_wr(fmi, COMMAND_FIFO, command_list[i]);
    }
}

static void fmiss_ppn_handle_write_error(h2fmi_t *fmi, UInt8 status)
{
    fmiss_cxt_t *cxt = fmi->fmiss_cxt;
    PPNCommandEntry *entry;

    fmiss_ppn_handle_drain_status(fmi, 0);

    entry = &cxt->command->entry[*cxt->status_page];

    *cxt->ppn_status |= status;
    entry->status = status;
    (*cxt->status_page)++;

    if (0 == status)
    {
        *cxt->fmi_status = _kIOPFMI_STATUS_READY_BUSY_TIMEOUT;
    }
    else if (0 != (status & PPN_CONTROLLER_STATUS__GENERAL_ERROR))
    {
        fmi->ppn->general_error = TRUE32;
        fmi->ppn->general_error_ce = fmiss_ce_reg_to_logical(fmi, h2fmi_rd(fmi, FMC_CE_CTRL));

        *cxt->fmi_status = _kIOPFMI_STATUS_PPN_GENERAL_ERROR;
    }
    else if (0 != (status & PPN_CONTROLLER_STATUS__PENDING_ERRORS))
    {
        // we don't actually know which page on this CE failed yet
        entry->status = PPN_CONTROLLER_STATUS__READY;
        cxt->error[entry->ceIdx] = TRUE32;
    }
    else
    {
        WMR_PANIC("Unhandled NAND status: 0x%02x!\n", status);
    }

    if (_kIOPFMI_STATUS_SUCCESS == *cxt->fmi_status)
    {
        fmiss_ppn_handle_command_fixup(fmi);
    }

    return;
}

#endif // FMI_VERSION >= 4

static BOOL32 fmiss_yield(h2fmi_t *fmi)
{
    BOOL32 ret = TRUE32;
    const UInt32 seq_int_pend = h2fmi_rd(fmi, SEQ_INT_PEND);

    if (0 != (SEQ_INT_PEND__TIMEOUT & seq_int_pend))
    {
#if H2FMI_IOP
        WMR_PANIC("Sequencer Timeout!");
#endif // H2FMI_IOP

#if FMI_VERSION >= 4
        fmiss_ppn_handle_write_error(fmi, 0);
#endif // FMI_VERSION >= 4

        ret = FALSE32;
        goto exit;
    }

#if FMI_VERSION >= 4

    if ((NULL != fmi->fmiss_cxt) && (0 != (SEQ_INT_PEND__SEQUENCER_SIGNAL & seq_int_pend)))
    {
        const UInt32 code = h2fmi_rd(fmi, COMMAND_INT_CODE);

        h2fmi_wr(fmi, SEQ_INT_PEND, SEQ_INT_PEND__SEQUENCER_SIGNAL);
        if (SEQ_WFR_CODE_INVALID != COMMAND_INT_CODE__WAIT_FOR_READY_CODE(code))
        {
            switch (COMMAND_INT_CODE__WAIT_FOR_READY_CODE(code))
            {
                case SEQ_WFR_CODE_WRITE_STATUS:
                    fmiss_ppn_handle_write_error(fmi, COMMAND_INT_CODE__WAIT_FOR_READY_STATUS(code));
                    break;

                default:
                    WMR_PANIC("Unexpected sequencer interrupt: 0x%08x!\n", code);
            }
        }
        else
        {
            switch (COMMAND_INT_CODE__READ(code))
            {
                case SEQ_INT_CODE_DRAIN_STATUS:
                    fmiss_ppn_handle_drain_status(fmi, 0);
                    break;

                default:
                    WMR_PANIC("Unexpected sequencer interrupt: 0x%08x!\n", code);
            }
        }
    }
    else if (NULL != fmi->fmiss_cxt)
    {
            fmiss_ppn_handle_drain_status(fmi, 1);
    }

#endif // FMI_VERSION >= 4

    WMR_YIELD();

exit:

    return ret;
}

#if FMI_VERSION >= 4
static UInt32 fmiss_get_store(h2fmi_t *fmi, UInt32 *level, BOOL32 *timeout)
{
    UInt32 ret = 0;
    BOOL32 first = TRUE32;

    while ((0 >= *level) && (!*timeout) &&
           ((NULL == fmi->fmiss_cxt) || _kIOPFMI_STATUS_SUCCESS == *((fmiss_cxt_t *)fmi->fmiss_cxt)->fmi_status))
    {
        if (!first)
        {
            if (!fmiss_yield(fmi))
            {
                *timeout = TRUE32;
                break;
            }
        }
        else
        {
            first = FALSE32;
        }
        *level = STORE_FIFO_PTR__LEVEL(h2fmi_rd(fmi, STORE_FIFO_PTR));
    }

    if (0 < *level)
    {
        ret = h2fmi_rd(fmi, STORE_FIFO);
        (*level)--;
    }

    return ret;
}
#endif // FMI_VERSION >= 4


Int32 fmiss_ppn_read_multi(h2fmi_t            *fmi,
                           PPNCommandStruct   *ppnCommand,
                           struct dma_segment *data_segment_array,
                           struct dma_segment *meta_segment_array)
{
    const UInt32      page_count             = ppnCommand->num_pages;
    const UInt32      read_queue_size        = fmi->ppn->device_params.read_queue_size;
    UInt32            overall_status;
    UInt32            prep_page;
    UInt32            read_page;
    UInt32            i;
    UInt32            queue_depth[PPN_MAX_CES_PER_BUS] = {0};
    UInt32            ce_page_count[PPN_MAX_CES_PER_BUS] = {0};
    UInt32            iopfmiStatus;
    BOOL32            timeout;
    UInt32            command_level = 0;
    UInt32            operand_level = 0;
#if FMI_VERSION >= 4
    UInt32            status_page   = 0;
    UInt32            store_level   = 0;
#endif // FMI_VERSION >= 4
    UInt32            fmi_control = (FMI_CONTROL__RESET_SEQUENCER |
                                     FMI_CONTROL__ENABLE_SEQUENCER |
                                     FMI_CONTROL__SEQUENCER_TIMEOUT_ENABLE |
                                     FMI_CONTROL__MODE__READ);
    UInt8             op_status;

#if FMI_VERSION >= 4
    fmi_control |= FMI_CONTROL__PAUSE_WHEN_STORE_FIFO_FULL;
#if FMI_VERSION == 4
    if (fmi->read_stream_disable)
    {
        fmi_control |= FMI_CONTROL__DISABLE_STREAMING;
    }
#endif // FMI_VERSION == 4
#endif // FMI_VERSION >= 4

    timeout = FALSE32;
    overall_status = 0;
    prep_page = 0;
    read_page = 0;

    WMR_ASSERT(page_count > 0);

    h2fmi_wr(fmi, FMI_CONTROL, FMI_CONTROL__MODE__SOFTWARE_RESET);

    if(ppnCommand->options & PPN_OPTIONS_GET_PAGE_RMA_INFO)
    {
        h2fmi_ce_t ce = fmiss_ce_index_to_physical(ppnCommand, 0);

        WMR_PRINT(ALWAYS, "Attempting to pull RMA data for CE %d page 0x%08x\n",
                  ce, ppnCommand->entry[0].addr.row);

        h2fmi_ppn_force_geb_address(fmi, ce, ppnCommand->entry[0].addr.row);
        return  _kIOPFMI_STATUS_PPN_GENERAL_ERROR;
    }

    if (ppnCommand->options & PPN_OPTIONS_REPORT_HEALTH)
    {
        UInt32 feature = 1;

        for (i = 0; i < PPN_MAX_CES_PER_BUS; i++)
        {
            if (ppnCommand->ceInfo[i].pages > 0)
            {
                h2fmi_ce_t ce = ppnCommand->ceInfo[i].ce;

                if (FIL_SUCCESS != h2fmi_ppn_set_features(fmi,
                                                          ce,
                                                          PPN_FEATURE__ENABLE_BITFLIPS_DATA_COLLECTION,
                                                          (UInt8 *)&feature,
                                                          PPN_FEATURE_LENGTH_ENABLE_BITFLIPS_DATA_COLLECTION,
                                                          FALSE32,
                                                          NULL))
                {
                    WMR_PANIC("Failed to enable bitflip collection on CE index %d (physical %d)", i, ce);
                }
            }
        }
    }

    h2fmi_wr(fmi, FMI_CONFIG, fmi->fmi_config_reg);
    h2fmi_wr(fmi, FMI_DATA_SIZE, (FMI_DATA_SIZE__BYTES_PER_SECTOR(H2FMI_BYTES_PER_SECTOR) |
                                  FMI_DATA_SIZE__SECTORS_PER_PAGE(fmi->logical_page_size / H2FMI_BYTES_PER_SECTOR) |
                                  FMI_DATA_SIZE__META_BYTES_PER_SECTOR(fmi->valid_bytes_per_meta) |
                                  FMI_DATA_SIZE__META_BYTES_PER_PAGE(fmi->valid_bytes_per_meta)));
    h2fmi_wr(fmi, TIMEOUT_VALUE, FMISS_TIMEOUT_VALUE(fmi));
    h2fmi_wr(fmi, FMI_CONTROL, fmi_control);
    h2fmi_wr(fmi, SEQ_INT_PEND, 0xFFFFFFFF);

    for (read_page = 0; read_page < page_count; read_page++)
    {
        const UInt8      read_page_ce_index = ppnCommand->entry[read_page].ceIdx;
        const h2fmi_ce_t read_page_phys_ce  = fmiss_ce_index_to_physical(ppnCommand,
                                                                         read_page_ce_index);
        UInt32 lba;

        while ((queue_depth[ppnCommand->entry[prep_page].ceIdx] < read_queue_size) && (prep_page < page_count))
        {
            const UInt8      ce_index = ppnCommand->entry[prep_page].ceIdx;
            const h2fmi_ce_t phys_ce  = fmiss_ce_index_to_physical(ppnCommand, ce_index);
            UInt32 *page_addr = (UInt32 *)&ppnCommand->entry[prep_page].addr;

            if (ce_index > PPN_MAX_CES_PER_BUS)
            {
                WMR_PRINT(ERROR, "ce_index (%d) < PPN_MAX_CES_PER_BUS (%d)\n", ce_index, PPN_MAX_CES_PER_BUS);
                return _kIOPFMI_STATUS_PARAM_INVALID;
            }

            fmiss_put_command(fmi, CMD_ENABLE_CHIP(phys_ce), &command_level, &timeout);
            // in Async mode, we are currently meeting tCS requirements because the command FIFO
            // is empty on initial CE assertion and register writes from the IOP are relatively slow
            // if the location of the CE assertion command changes, this will have to be reevaluated
            if (fmi->logical_page_size == fmi->bytes_per_page)
            {
                fmiss_put_operand(fmi, fmi->ppn->bytes_per_row_address - 1, &operand_level, &timeout);
                fmiss_put_operand(fmi, ppnCommand->entry[prep_page].addr.row, &operand_level, &timeout);
                fmiss_put_operand(fmi, 0, &operand_level, &timeout);
            }
            else
            {
                fmiss_put_operand(fmi, fmi->ppn->bytes_per_full_address - 1, &operand_level, &timeout);
                fmiss_put_operand(fmi, page_addr[0], &operand_level, &timeout);
                fmiss_put_operand(fmi, page_addr[1], &operand_level, &timeout);
            }

            if (ce_page_count[ce_index] + 1 == ppnCommand->ceInfo[ce_index].pages)
            {
                fmiss_put_operand(fmi, (FMC_CMD__CMD1(NAND_CMD__MULTIPAGE_READ_LAST) |
                                        FMC_CMD__CMD2(NAND_CMD__MULTIPAGE_READ_CONFIRM)), &operand_level, &timeout);
            }
            else
            {
                fmiss_put_operand(fmi, (FMC_CMD__CMD1(NAND_CMD__MULTIPAGE_READ) |
                                        FMC_CMD__CMD2(NAND_CMD__MULTIPAGE_READ_CONFIRM)), &operand_level, &timeout);
            }
            fmiss_put_command(fmi, CMD_MACRO(kPrepReadHead, kPrepReadLength), &command_level, &timeout);
            if (timeout)
            {
                break;
            }

            ce_page_count[ce_index]++;
            prep_page++;
            queue_depth[ce_index]++;
        }

        if (0 == read_page)
        {
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

        fmiss_put_command(fmi, CMD_ENABLE_CHIP(read_page_phys_ce), &command_level, &timeout);
#if FMI_VERSION <= 3
        op_status = fmiss_get_next_operation_status(fmi, &command_level, &timeout);

        if (fmi->retire_on_invalid_refresh &&
                 ((PPN_OPERATION_STATUS__REFRESH | PPN_OPERATION_STATUS__ERROR) ==
                  (op_status & (PPN_OPERATION_STATUS__REFRESH | PPN_OPERATION_STATUS__ERROR))))
        {
            // Remap 0x43 (refresh + invalid) to 0x45 (retire + invalid)
            op_status = (op_status & ~(PPN_OPERATION_STATUS__REFRESH)) | PPN_OPERATION_STATUS__RETIRE;
        }
        ppnCommand->entry[read_page].status = op_status;
        overall_status |= op_status;
#else // FMI_VERSION <= 3
        if (STORE_FIFO_SIZE <= read_page)
        {
            op_status = fmiss_get_store(fmi, &store_level, &timeout);

            if (fmi->retire_on_invalid_refresh &&
                     ((PPN_OPERATION_STATUS__REFRESH | PPN_OPERATION_STATUS__ERROR) ==
                      (op_status & (PPN_OPERATION_STATUS__REFRESH | PPN_OPERATION_STATUS__ERROR))))
            {
                // Remap 0x43 (refresh + invalid) to 0x45 (retire + invalid)
                op_status = (op_status & ~(PPN_OPERATION_STATUS__REFRESH)) | PPN_OPERATION_STATUS__RETIRE;
            }
            ppnCommand->entry[status_page].status = op_status;
            overall_status |= op_status;

            if (0 != (overall_status & PPN_OPERATION_STATUS__GENERAL_ERROR))
            {
                const PPNCommandEntry *entry = &ppnCommand->entry[status_page];
                const PPNCommandCeInfo *ceInfo = &ppnCommand->ceInfo[entry->ceIdx];

                fmi->ppn->general_error = TRUE32;
                fmi->ppn->general_error_ce = ceInfo->ce;
            }
            status_page++;
        }
        fmiss_put_command(fmi, CMD_MACRO(kGetOperationStatusHead, kGetOperationStatusLength), &command_level, &timeout);
#endif // FMI_VERSION <= 3

        if (overall_status & PPN_OPERATION_STATUS__GENERAL_ERROR)
        {
            WMR_PRINT(ERROR, "Aborting read due to general error\n");
            break;
        }

        fmiss_put_command(fmi, CMD_TIMED_WAIT(FMISS_NS_TO_CLKS(fmi, PPN_TIMING_MIN_TRHW_NS)), &command_level, &timeout);
        fmiss_put_command(fmi, CMD_COMMAND(NAND_CMD__READ_SERIAL_OUTPUT), &command_level, &timeout);
        fmiss_put_command(fmi, CMD_TIMED_WAIT(FMISS_NS_TO_CLKS(fmi, PPN_TIMING_MIN_TWHR_NS)), &command_level, &timeout);
        for (lba = 0; lba < ppnCommand->entry[read_page].lbas; lba++)
        {
#if FMI_VERSION >= 5
            if ((0 < fmi->read_tx_page_delay) && (0 < lba))
            {
                fmiss_put_command(fmi, CMD_TIMED_WAIT(fmi->read_tx_page_delay - 1), &command_level, &timeout);
            }
#endif // FMI_VERSION >= 5
            fmiss_put_command(fmi, CMD_TX_PAGE, &command_level, &timeout);
        }
        fmiss_put_command(fmi, CMD_TIMED_WAIT(FMISS_NS_TO_CLKS(fmi, PPN_TIMING_MIN_TRHW_NS)), &command_level, &timeout);

        if (timeout)
        {
            break;
        }

        queue_depth[read_page_ce_index]--;
    }

    for (i = 0; i < PPN_MAX_CES_PER_BUS; i++)
    {
        if (ppnCommand->ceInfo[i].pages > 0)
        {
            h2fmi_ce_t phys_ce = 1 << (ppnCommand->ceInfo[i].ce & (H2FMI_MAX_CE_PER_BUS -1));
            fmiss_put_command(fmi, CMD_ENABLE_CHIP(phys_ce), &command_level, &timeout);
            fmiss_put_command(fmi, CMD_COMMAND(NAND_CMD__GET_NEXT_OPERATION_STATUS), &command_level, &timeout);
        }
    }
    fmiss_put_command(fmi, CMD_ENABLE_CHIP(0), &command_level, &timeout);

#if FMI_VERSION >= 4
    while (!fmi->ppn->general_error && (status_page < page_count))
    {
        op_status = fmiss_get_store(fmi, &store_level, &timeout);
        
        if (fmi->retire_on_invalid_refresh &&
                 ((PPN_OPERATION_STATUS__REFRESH | PPN_OPERATION_STATUS__ERROR) ==
                  (op_status & (PPN_OPERATION_STATUS__REFRESH | PPN_OPERATION_STATUS__ERROR))))
        {
            // Remap 0x43 (refresh + invalid) to 0x45 (retire + invalid)
            op_status = (op_status & ~(PPN_OPERATION_STATUS__REFRESH)) | PPN_OPERATION_STATUS__RETIRE;
        }
        ppnCommand->entry[status_page].status = op_status;
        
        if (timeout)
        {
            break;
        }
        
        overall_status |= op_status;

        if (0 != (overall_status & PPN_OPERATION_STATUS__GENERAL_ERROR))
        {
            const PPNCommandEntry *entry = &ppnCommand->entry[status_page];
            const PPNCommandCeInfo *ceInfo = &ppnCommand->ceInfo[entry->ceIdx];

            fmi->ppn->general_error = TRUE32;
            fmi->ppn->general_error_ce = ceInfo->ce;
        }
        status_page++;
    }
#endif // FMI_VERSION >= 4
    ppnCommand->page_status_summary = overall_status;
    if (timeout)
    {
        iopfmiStatus = _kIOPFMI_STATUS_READY_BUSY_TIMEOUT;
        h2fmi_dma_cancel(h2fmi_dma_meta_chan(fmi));
        h2fmi_dma_cancel(h2fmi_dma_data_chan(fmi));
    }
    else if (overall_status & PPN_OPERATION_STATUS__GENERAL_ERROR)
    {
        iopfmiStatus = _kIOPFMI_STATUS_PPN_GENERAL_ERROR;
        h2fmi_dma_cancel(h2fmi_dma_meta_chan(fmi));
        h2fmi_dma_cancel(h2fmi_dma_data_chan(fmi));

        h2fmi_wr(fmi, FMI_CONTROL, (FMI_CONTROL__RESET_SEQUENCER |
                                    FMI_CONTROL__ENABLE_SEQUENCER));
        WMR_PRINT(ERROR, "IOP returning kIOPFMI_STATUS_PPN_GENERAL_ERROR\n");
    }
    else
    {
        iopfmiStatus = _kIOPFMI_STATUS_SUCCESS;

        if (!h2fmi_dma_wait(h2fmi_dma_data_chan(fmi), H2FMI_PAGE_TIMEOUT_MICROS) ||
            !h2fmi_dma_wait(h2fmi_dma_meta_chan(fmi), H2FMI_PAGE_TIMEOUT_MICROS))
        {
            iopfmiStatus = _kIOPFMI_STATUS_DMA_DONE_TIMEOUT;
            h2fmi_dma_cancel(h2fmi_dma_meta_chan(fmi));
            h2fmi_dma_cancel(h2fmi_dma_data_chan(fmi));
        }
    }

    return iopfmiStatus;
}

#if !(defined(AND_READONLY)) || AND_SUPPORT_NVRAM
Int32 fmiss_ppn_write_multi(h2fmi_t            *fmi,
                            PPNCommandStruct   *ppnCommand,
                            struct dma_segment *data_segment_array,
                            struct dma_segment *meta_segment_array)
{
    const UInt32      page_count        = ppnCommand->num_pages;
    UInt32            pageIndex = 0;
    BOOL32            error[PPN_MAX_CES_PER_BUS] = {FALSE32};
    UInt8             overall_status = 0;
    UInt32            ce_page_count[PPN_MAX_CES_PER_BUS] = {0};
    UInt32            command_level = 0;
    UInt32            operand_level = 0;
    UInt32            fmi_control = (FMI_CONTROL__MODE__WRITE |
                                     FMI_CONTROL__ENABLE_SEQUENCER |
                                     FMI_CONTROL__SEQUENCER_TIMEOUT_ENABLE);
    UInt32            program_status = _kIOPFMI_STATUS_SUCCESS;
    BOOL32            timeout = FALSE32;
    UInt8             errorCeIndex;
#if FMI_VERSION >= 4
    UInt32            status_page   = 0;
    UInt32            store_level   = 0;
    fmiss_cxt_t       cxt;
#endif // FMI_VERSION >= 4

#if FMI_VERSION >= 4
    fmi_control |= FMI_CONTROL__PAUSE_WHEN_STORE_FIFO_FULL;
#endif // FMI_VERSION >= 4

    WMR_ASSERT(page_count > 0);
    WMR_ASSERT(page_count <= fmi->ppn->device_params.prep_function_buffer_size);

    h2fmi_wr(fmi, FMI_CONTROL, FMI_CONTROL__MODE__SOFTWARE_RESET);
    h2fmi_wr(fmi, TIMEOUT_VALUE, FMISS_TIMEOUT_VALUE(fmi));
    h2fmi_wr(fmi, FMI_CONTROL, FMI_CONTROL__ENABLE_SEQUENCER);
    h2fmi_wr(fmi, SEQ_INT_PEND, 0xFFFFFFFF);

    if (page_count > 1)
    {
        if (!fmiss_ppn_prep_write_multi(fmi, ppnCommand, &command_level, &operand_level))
        {
            return _kIOPFMI_STATUS_READY_BUSY_TIMEOUT;
        }
    }

#if FMI_VERSION >= 4
    cxt.command     = ppnCommand;
    cxt.fmi_status  = &program_status;
    cxt.ppn_status  = &overall_status;
    cxt.store_level = &store_level;
    cxt.status_page = &status_page;
    cxt.error       = error;
    fmi->fmiss_cxt  = &cxt;
#endif // FMI_VERSION >= 4

    h2fmi_wr(fmi, FMI_CONFIG, fmi->fmi_config_reg);
    h2fmi_wr(fmi, FMI_DATA_SIZE, (FMI_DATA_SIZE__BYTES_PER_SECTOR(H2FMI_BYTES_PER_SECTOR) |
                                  FMI_DATA_SIZE__SECTORS_PER_PAGE(fmi->logical_page_size / H2FMI_BYTES_PER_SECTOR) |
                                  FMI_DATA_SIZE__META_BYTES_PER_SECTOR(fmi->valid_bytes_per_meta) |
                                  FMI_DATA_SIZE__META_BYTES_PER_PAGE(fmi->valid_bytes_per_meta)));
    h2fmi_wr(fmi, FMC_ADDRNUM, fmi->ppn->bytes_per_row_address - 1);
    h2fmi_wr(fmi, FMI_CONTROL, fmi_control);

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

    do
    {
        const UInt8      ceIndex = ppnCommand->entry[pageIndex].ceIdx;
        const h2fmi_ce_t physCe  = fmiss_ce_index_to_physical(ppnCommand, ceIndex);
        UInt32           lba;

        fmiss_put_command(fmi, CMD_ENABLE_CHIP(error[ceIndex] ? 0 : physCe), &command_level, &timeout);
        // in Async mode, we are currently meeting tCS requirements because the command FIFO
        // is empty on initial CE assertion and register writes from the IOP are relatively slow
        // if the location of the CE assertion command changes, this will have to be reevaluated
        fmiss_put_operand(fmi, ppnCommand->entry[pageIndex].addr.row, &operand_level, &timeout);

        if (ce_page_count[ceIndex] + 1 == ppnCommand->ceInfo[ceIndex].pages)
        {
            fmiss_put_operand(fmi, FMC_CMD__CMD1(NAND_CMD__MULTIPAGE_PROGRAM_LAST), &operand_level, &timeout);
        }
        else
        {
            fmiss_put_operand(fmi, FMC_CMD__CMD1(NAND_CMD__MULTIPAGE_PROGRAM), &operand_level, &timeout);
        }
        fmiss_put_command(fmi, CMD_MACRO(kStartWritePageHead, kStartWritePageLength), &command_level, &timeout);
        for (lba = 0; lba < ppnCommand->entry[pageIndex].lbas; lba++)
        {
            fmiss_put_command(fmi, CMD_TX_PAGE, &command_level, &timeout);
        }
        fmiss_put_command(fmi, CMD_COMMAND(NAND_CMD__MULTIPAGE_PROGRAM_CONFIRM), &command_level, &timeout);

#if FMI_VERSION < 4

        if (!error[ceIndex])
        {
            UInt8 pageStatus = fmiss_get_controller_status(fmi, &command_level, &timeout);

            if (timeout)
            {
                // Don't look at pageStatus at all if we timed out - it could have bogus bits set.
                pageStatus = 0;
            }

            // Always mark the program as good here - if there was a program error or if the page was
            // kicked off but not completed, we'll fix up the status when we pull the program error
            // lists.
            ppnCommand->entry[pageIndex].status = PPN_CONTROLLER_STATUS__READY;

            overall_status |= pageStatus;
            if (timeout)
            {
                // Timeout reading page status
                program_status = _kIOPFMI_STATUS_READY_BUSY_TIMEOUT;
            }
            else if (pageStatus & PPN_CONTROLLER_STATUS__GENERAL_ERROR)
            {
                program_status = _kIOPFMI_STATUS_PPN_GENERAL_ERROR;
            }
            else if (pageStatus & PPN_CONTROLLER_STATUS__PENDING_ERRORS)
            {
                error[ceIndex] = TRUE32;
            }
        }
        else
        {
            // If we've seen a program error on a particular CE, we don't want to issue any more commands to it.
            // But we need to keep the DMA moving so we can complete the programs on other CEs.  So we kick off the
            // program op with the CE disabled and just don't pull status.
            ppnCommand->entry[pageIndex].status = 0;
        }

#else // FMI_VERSION >= 4

        if (!error[ceIndex])
        {
            fmiss_put_command(fmi, error[ceIndex] ? CMD_TIMED_WAIT(0) : CMD_MACRO(kStoreControllerStatusHead, kStoreControllerStatusLength),
                              &command_level, &timeout);
        }

        if (page_count - 1 == pageIndex)
        {
            fmiss_put_command(fmi, CMD_SEND_INTERRUPT(SEQ_INT_CODE_DRAIN_STATUS), &command_level, &timeout);
        }

#endif // FMI_VERSION >= 4

        ce_page_count[ceIndex]++;
        pageIndex++;
    } while ((program_status == _kIOPFMI_STATUS_SUCCESS) && (pageIndex < page_count));

    fmiss_put_command(fmi, CMD_ENABLE_CHIP(0), &command_level, &timeout);

#if FMI_VERSION >= 4

    while ((program_status == _kIOPFMI_STATUS_SUCCESS) && (status_page < page_count))
    {
        fmiss_yield(fmi);
    }

#endif // FMI_VERSION >= 4

    if ((program_status != _kIOPFMI_STATUS_SUCCESS) || timeout ||
        !h2fmi_dma_wait(h2fmi_dma_data_chan(fmi), H2FMI_PAGE_TIMEOUT_MICROS) ||
        !h2fmi_dma_wait(h2fmi_dma_meta_chan(fmi), H2FMI_PAGE_TIMEOUT_MICROS))
    {
        h2fmi_dma_cancel(h2fmi_dma_meta_chan(fmi));
        h2fmi_dma_cancel(h2fmi_dma_data_chan(fmi));
    }

    for (errorCeIndex = 0; errorCeIndex < PPN_MAX_CES_PER_BUS; errorCeIndex++)
    {
        UInt32    temp = 1;
        UInt8     status = 0;

        if (error[errorCeIndex])
        {
            const h2fmi_ce_t physCe = ppnCommand->ceInfo[errorCeIndex].ce;

            program_status = _kIOPFMI_STATUS_PGM_ERROR;

            // Program terminated  early - reset sequencer
            h2fmi_wr(fmi, FMI_CONTROL, (FMI_CONTROL__RESET_SEQUENCER |
                                        FMI_CONTROL__ENABLE_SEQUENCER));

            h2fmi_reset(fmi);

            // Find failed pages...
            h2fmi_ppn_get_feature(fmi,
                                  physCe,
                                  PPN_FEATURE__PROGRAM_FAILED_PAGES,
                                  (UInt8 *)fmi->error_list,
                                  PPN_ERROR_LIST_SIZE,
                                  &status);

            WMR_PRINT(ERROR, "PPN device reports %d pages failed program\n", fmi->error_list[0]);
            h2fmi_ppn_process_error_list(fmi,
                                         ppnCommand,
                                         errorCeIndex,
                                         fmi->error_list,
                                         PPN_PROGRAM_STATUS_FAIL);

            // Find pending pages...
            h2fmi_ppn_get_feature(fmi,
                                  physCe,
                                  PPN_FEATURE__PROGRAM_IGNORED_PAGES,
                                  (UInt8 *)fmi->error_list,
                                  PPN_ERROR_LIST_SIZE,
                                  &status);

            WMR_PRINT(ERROR, "PPN device reports %d pages pending after program failure\n", fmi->error_list[0]);
            h2fmi_ppn_process_error_list(fmi,
                                         ppnCommand,
                                         errorCeIndex,
                                         fmi->error_list,
                                         PPN_PROGRAM_STATUS_NOT_PROGRAMMED);

            // Find pages to retire...
            h2fmi_ppn_get_feature(fmi,
                                  physCe,
                                  PPN_FEATURE__PROGRAM_RETIRED_PAGES,
                                  (UInt8 *)fmi->error_list,
                                  PPN_ERROR_LIST_SIZE,
                                  &status);

            WMR_PRINT(ERROR, "PPN device reports %d pages should be retired after program failure\n", fmi->error_list[0]);
            h2fmi_ppn_process_error_list(fmi,
                                         ppnCommand,
                                         errorCeIndex,
                                         fmi->error_list,
                                         PPN_PROGRAM_STATUS_FAIL);

            // Clear Program error lists
            h2fmi_ppn_set_features(fmi,
                                   physCe,
                                   PPN_FEATURE__CLEAR_PROGRAM_ERROR_LISTS,
                                   (UInt8 *)&temp,
                                   4,
                                   FALSE32,
                                   NULL);

        }
    }

    ppnCommand->page_status_summary = overall_status;

    if (program_status == _kIOPFMI_STATUS_PPN_GENERAL_ERROR)
    {
        WMR_PRINT(ERROR, "Program aborted due to GEB - resetting sequencer\n");
        h2fmi_wr(fmi, FMI_CONTROL, (FMI_CONTROL__RESET_SEQUENCER |
                                    FMI_CONTROL__ENABLE_SEQUENCER));
    }

#if FMI_VERSION >= 4
    fmi->fmiss_cxt                        = NULL;
#endif // FMI_VERSION >= 4

    return program_status;
}
#endif // !(defined(AND_READONLY)) || AND_SUPPORT_NVRAM

static BOOL32 fmiss_ppn_prep_write_multi(h2fmi_t                *fmi,
                                       const PPNCommandStruct *ppnCommand,
                                       UInt32                 *command_level,
                                       UInt32                 *operand_level)
{
    const UInt32 totalPages = ppnCommand->num_pages;
    UInt16             ceIndex;
    UInt32             pageIndex;
    struct dma_segment dma_segment;
    UInt32 fmi_control = (FMI_CONTROL__MODE__WRITE |
                          FMI_CONTROL__ENABLE_SEQUENCER |
                          FMI_CONTROL__SEQUENCER_TIMEOUT_ENABLE);
    BOOL32 timeout = FALSE32;

#if FMI_VERSION >= 4
    fmi_control |= FMI_CONTROL__PAUSE_WHEN_STORE_FIFO_FULL;
#endif // FMI_VERSION >= 4

    for (pageIndex = 0 ; pageIndex < totalPages ; pageIndex++)
    {
        const PPNCommandEntry *entry = &ppnCommand->entry[pageIndex];

        *fmi->ppn->prep_buffer[entry->ceIdx]++ = entry->addr.row;
    }

    for (ceIndex = 0; ceIndex < PPN_MAX_CES_PER_BUS; ceIndex++)
    {
        const UInt32       numPages  = ppnCommand->ceInfo[ceIndex].pages;
        UInt32             addrBytes = numPages * sizeof(UInt32);
        const h2fmi_ce_t   physCe    = fmiss_ce_index_to_physical(ppnCommand, ceIndex);
        UInt32             fullSectors;

        fmi->ppn->prep_buffer[ceIndex] -= numPages;

        if (numPages)
        {
            const UInt32 prepBuffer = (UInt32)fmi->ppn->prep_buffer[ceIndex];

            WMR_PREPARE_WRITE_BUFFER(fmi->ppn->prep_buffer[ceIndex], numPages * sizeof(**fmi->ppn->prep_buffer));

#if WMR_BUILDING_IBOOT
            dma_segment.paddr = mem_static_map_physical(prepBuffer);
#else
            dma_segment.paddr = prepBuffer;
#endif
            dma_segment.length = numPages * sizeof(UInt32);

            h2fmi_dma_execute_async(DMA_CMD_DIR_TX,
                                    h2fmi_dma_data_chan(fmi),
                                    &dma_segment,
                                    h2fmi_dma_data_fifo(fmi),
                                    numPages * sizeof(UInt32),
                                    sizeof(UInt32),
                                    32,
                                    NULL);
            h2fmi_wr(fmi, FMI_CONFIG, FMI_CONFIG__DMA_BURST__32_CYCLES);
            fmiss_put_command(fmi, CMD_ENABLE_CHIP(physCe), command_level, &timeout);
            // in Async mode, we are currently meeting tCS requirements because the command FIFO
            // is empty on initial CE assertion and register writes from the IOP are relatively slow
            // if the location of the CE assertion command changes, this will have to be reevaluated
            fmiss_put_operand(fmi, (numPages << 8) | 0x87, operand_level, &timeout);
            fmiss_put_command(fmi, CMD_MACRO(kPrepWriteHead, kPrepWriteLength), command_level, &timeout);

            fullSectors = addrBytes / H2FMI_BYTES_PER_SECTOR;
            if (fullSectors > 0)
            {
                fmiss_put_command(fmi, CMD_LOAD_NEXT_WORD(FMI_DATA_SIZE), command_level, &timeout);
                fmiss_put_command(fmi, (FMI_DATA_SIZE__BYTES_PER_SECTOR(H2FMI_BYTES_PER_SECTOR)|
                                        FMI_DATA_SIZE__SECTORS_PER_PAGE(fullSectors)), command_level, &timeout);
                fmiss_put_command(fmi, CMD_LOAD_FROM_FIFO(FMI_CONTROL), command_level, &timeout);
                fmiss_put_operand(fmi, fmi_control, operand_level, &timeout);
                // there must be one command spacer between FMI_CONTROL writes and CMD_TX_PAGE,
                // which writes the start bit in FMI_CONTROL (<rdar://problem/9350465>)
                fmiss_put_command(fmi, CMD_TIMED_WAIT(0), command_level, &timeout);
                fmiss_put_command(fmi, CMD_TX_PAGE, command_level, &timeout);

                addrBytes -= fullSectors * H2FMI_BYTES_PER_SECTOR;
            }

            if (addrBytes > 0)
            {
                fmiss_put_command(fmi, CMD_LOAD_NEXT_WORD(FMI_DATA_SIZE), command_level, &timeout);
                fmiss_put_command(fmi, (FMI_DATA_SIZE__BYTES_PER_SECTOR(addrBytes) |
                                        FMI_DATA_SIZE__SECTORS_PER_PAGE(1)), command_level, &timeout);
                fmiss_put_command(fmi, CMD_LOAD_FROM_FIFO(FMI_CONTROL), command_level, &timeout);
                fmiss_put_operand(fmi, fmi_control, operand_level, &timeout);
                // there must be one command spacer between FMI_CONTROL writes and CMD_TX_PAGE,
                // which writes the start bit in FMI_CONTROL (<rdar://problem/9350465>)
                fmiss_put_command(fmi, CMD_TIMED_WAIT(0), command_level, &timeout);
                fmiss_put_command(fmi, CMD_TX_PAGE, command_level, &timeout);
            }

            fmiss_put_command(fmi, CMD_COMMAND(NAND_CMD__MULTIPAGE_PREP__CONFIRM), command_level, &timeout);

            fmiss_get_controller_status(fmi, command_level, &timeout);

            if ((timeout) || !h2fmi_dma_wait(h2fmi_dma_data_chan(fmi), H2FMI_PAGE_TIMEOUT_MICROS))
            {
#if H2FMI_IOP
                WMR_PANIC("Timeout waiting for CDMA channel %d to complete multi prep: ce %d, %d pages",
                          h2fmi_dma_data_chan(fmi), physCe, numPages);
#endif // H2FMI_IOP
                break;
            }
        }
    }

    return !timeout;
}


static UInt8 fmiss_ce_index_to_physical(const PPNCommandStruct *ppnCommand, h2fmi_ce_t ceIndex)
{
    UInt8 physCe = ppnCommand->ceInfo[ceIndex].ce;
    return 1 << (physCe & (H2FMI_MAX_CE_PER_BUS - 1));
}

static UInt32 fmiss_ce_reg_to_logical(h2fmi_t *fmi, UInt32 ceCtrl)
{
    return WMR_LOG2(ceCtrl) + (fmi->bus_id * H2FMI_MAX_CE_PER_BUS);
}

static UInt8 fmiss_get_controller_status(h2fmi_t *fmi, UInt32 *command_level, BOOL32 *timeout)
{
    UInt32 seq_int;
    UInt8  status = 0;

    fmiss_put_command(fmi, CMD_MACRO(kGetControllerStatusHead,
                                     kGetControllerStatusLength), command_level, timeout);

    seq_int = h2fmi_rd(fmi, SEQ_INT_PEND) & (SEQ_INT_PEND__TIMEOUT | SEQ_INT_PEND__SEQUENCER_SIGNAL);

    while ((seq_int == 0) && !*timeout)
    {
        WMR_YIELD();
        seq_int = h2fmi_rd(fmi, SEQ_INT_PEND) & (SEQ_INT_PEND__TIMEOUT | SEQ_INT_PEND__SEQUENCER_SIGNAL);
    }
    h2fmi_wr(fmi, SEQ_INT_PEND, seq_int);

    if (seq_int & SEQ_INT_PEND__SEQUENCER_SIGNAL)
    {
        status = h2fmi_rd(fmi, FMC_NAND_STATUS);
        if (status & PPN_CONTROLLER_STATUS__GENERAL_ERROR)
        {
            fmi->ppn->general_error = TRUE32;
            fmi->ppn->general_error_ce = fmiss_ce_reg_to_logical(fmi, h2fmi_rd(fmi, FMC_CE_CTRL));
        }
    }
    else if (seq_int & SEQ_INT_PEND__TIMEOUT)
    {
        *timeout = TRUE32;
#if H2FMI_IOP
        WMR_PANIC("Timeout reading controller status");
#endif // !H2FMI_IOP
    }

    if (!*timeout)
    {
        h2fmi_wr(fmi, FMI_CONTROL, h2fmi_rd(fmi, FMI_CONTROL) | FMI_CONTROL__ENABLE_SEQUENCER);
    }

    return status;
}


#if FMI_VERSION <= 3
static UInt8 fmiss_get_next_operation_status(h2fmi_t *fmi, UInt32 *command_level, BOOL32 *timeout)
{
    UInt32 seq_int;
    UInt8  operation_status = 0;

    fmiss_put_command(fmi, CMD_MACRO(kGetOperationStatusHead,
                                     kGetOperationStatusLength), command_level, timeout);
    seq_int = h2fmi_rd(fmi, SEQ_INT_PEND) & (SEQ_INT_PEND__TIMEOUT | SEQ_INT_PEND__SEQUENCER_SIGNAL);
    while ((seq_int == 0) && !*timeout)
    {
        WMR_YIELD();
        seq_int = h2fmi_rd(fmi, SEQ_INT_PEND) & (SEQ_INT_PEND__TIMEOUT | SEQ_INT_PEND__SEQUENCER_SIGNAL);
    }

    h2fmi_wr(fmi, SEQ_INT_PEND, seq_int);

    if (seq_int & SEQ_INT_PEND__SEQUENCER_SIGNAL)
    {
        operation_status = h2fmi_rd(fmi, FMC_NAND_STATUS);
        if (operation_status & PPN_OPERATION_STATUS__GENERAL_ERROR)
        {
            fmi->ppn->general_error = TRUE32;
            fmi->ppn->general_error_ce = fmiss_ce_reg_to_logical(fmi, h2fmi_rd(fmi, FMC_CE_CTRL));
        }
        h2fmi_wr(fmi, FMI_CONTROL, h2fmi_rd(fmi, FMI_CONTROL) | FMI_CONTROL__ENABLE_SEQUENCER);
    }
    else if (seq_int & SEQ_INT_PEND__TIMEOUT)
    {
        *timeout = TRUE32;
#if H2FMI_IOP
        WMR_PANIC("Timeout");
#endif // H2FMI_IOP
    }
    else
    {
        WMR_PANIC("Unexpected SEQ_INT status: 0x%08x\n", seq_int);
    }

    return operation_status;
}
#endif // FMI_VERSION <= 3

UInt32 *fmiss_ppn_macros(UInt32 *count)
{
    if (NULL != count)
    {
        *count = sequencer_macro_count;
    }

    return sequencer_macros;
}

#endif /* FMISS_ENABLED */
