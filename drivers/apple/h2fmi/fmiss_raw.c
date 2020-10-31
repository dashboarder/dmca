// *****************************************************************************
//
// File: fmiss_raw.c
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
#include <FIL.h>
#include <FILTypes.h>


#if FMISS_ENABLED

#define FMISS_TIMEOUT_VALUE(f) (1 * 1000 * f->clock_speed_khz)
#define FMISS_NS_TO_CLKS(f, t) (t * f->clock_speed_khz / (1000 * 1000))

#define FMI_INT_ENABLE \
    (FMI_INT_EN__LAST_ECC_DONE | \
     FMI_INT_EN__SEQUENCER_INTERRUPT | \
     FMI_INT_EN__ECC_RESULTS_FIFO_OVERFLOW | \
     FMI_INT_EN__ECC_RESULTS_FIFO_UNDERFLOW)

#define FMI_INT_EXPECTED \
    (FMI_INT_PEND__LAST_ECC_DONE | \
     FMI_INT_PEND__SEQUENCER_INTERRUPT)

#define FMI_INT_ERRORS \
    (FMI_INT_PEND__ECC_RESULTS_FIFO_OVERFLOW | \
     FMI_INT_PEND__ECC_RESULTS_FIFO_UNDERFLOW)

#define SEQ_INT_ENABLE \
    (SEQ_INT_EN__TIMEOUT | \
     SEQ_INT_EN__COMMAND_FIFO_OVERFLOW | \
     SEQ_INT_EN__OPERAND_FIFO_OVERFLOW)

#define SEQ_INT_ERRORS \
    (SEQ_INT_PEND__TIMEOUT | \
     SEQ_INT_PEND__COMMAND_FIFO_OVERFLOW | \
     SEQ_INT_PEND__OPERAND_FIFO_OVERFLOW)

#define MACRO_LENGTH(s) \
    (sizeof((UInt32[]){s})/sizeof(UInt32))

#define PREP_PAGE_SEQ \
    CMD_LOAD_FROM_FIFO(FMC_ADDR1), \
    CMD_LOAD_FROM_FIFO(FMC_ADDR0), \
    CMD_LOAD_NEXT_WORD(FMC_CMD), \
    FMC_CMD__CMD1(NAND_CMD__READ) | FMC_CMD__CMD2(NAND_CMD__READ_CONFIRM), \
    CMD_LOAD_NEXT_WORD(FMC_RW_CTRL), \
    FMC_RW_CTRL__ADDR_MODE | FMC_RW_CTRL__CMD1_MODE | FMC_RW_CTRL__CMD2_MODE, \
    CMD_POLL(FMC_STATUS), \
    FMC_STATUS__ADDRESSDONE | FMC_STATUS__CMD1DONE | FMC_STATUS__CMD2DONE, \
    FMC_STATUS__ADDRESSDONE | FMC_STATUS__CMD1DONE | FMC_STATUS__CMD2DONE, \
    CMD_LOAD_NEXT_WORD(FMC_STATUS), \
    FMC_STATUS__ADDRESSDONE | FMC_STATUS__CMD1DONE | FMC_STATUS__CMD2DONE

// CMD_TIMED_WAIT() takes an index into timed_wait_ns[]
#define READ_PAGE_SEQ \
    CMD_COMMAND(NAND_CMD__READ_STATUS), \
    CMD_WAIT_FOR_READY(0x01, 0x40, 0x40), \
    CMD_TIMED_WAIT(0), \
    CMD_COMMAND(NAND_CMD__READ), \
    CMD_TIMED_WAIT(1), \
    CMD_TX_PAGE


typedef struct
{
    h2fmi_ce_t *chip_enable_array;
    UInt8      *max_corrected_array;
    UInt8      *sector_stats;
    UInt32      clean_pages;
    UInt32      fused_pages;
    UInt32      uecc_pages;
    UInt32      ecc_idx;
    BOOL32      timeout;
} fmiss_cxt_t;


static UInt32 sequencer_macros[] =
{
    PREP_PAGE_SEQ,
    READ_PAGE_SEQ,
};

static const UInt32 sequencer_macro_count = sizeof(sequencer_macros)/sizeof(sequencer_macros[0]);

static const UInt32 timed_wait_ns[] =
{
    100, // tRHW
     60, // tWHR
};

static const UInt32 timed_wait_count = sizeof(timed_wait_ns)/sizeof(timed_wait_ns[0]);

typedef enum
{
    kPrepPageHead,
    kPrepPageLength = MACRO_LENGTH(PREP_PAGE_SEQ),
    kPrepPageTail   = kPrepPageHead + kPrepPageLength - 1,

    kReadPageHead,
    kReadPageLength = MACRO_LENGTH(READ_PAGE_SEQ),
    kReadPageTail   = kReadPageHead + kReadPageLength - 1,
} sequence_entry_t;


static void fmiss_raw_init(h2fmi_t *fmi)
{
    BOOL32 initialized = FALSE32;

    if (!initialized)
    {
        UInt32 i, j;

        for (i = 0 ; i < sequencer_macro_count ; i++)
        {
            if (CMD_TIMED_WAIT(0) != (COMMAND_FIFO__COMMAND(~0UL) & sequencer_macros[i]))
            {
                continue;
            }
            for (j = 0 ; j < timed_wait_count ; j++)
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
}

void fmiss_raw_init_sequences(h2fmi_t *fmi)
{
    UInt32 i, j;

    fmiss_raw_init(fmi);

    for (i = 0, j = 0 ; i < sequencer_macro_count ; i++, j += sizeof(*sequencer_macros))
    {
        h2fmi_wr(fmi, SEQUENCER_MACROS(j), sequencer_macros[i]);
    }
}

static UInt8 fmiss_ce_to_mask(h2fmi_ce_t ce)
{
    return FMC_CE_CTRL__CEB(ce & (H2FMI_MAX_CE_PER_BUS - 1));
}

static BOOL32 fmiss_process_ecc(h2fmi_t *fmi)
{
    BOOL32 ret = FALSE32;
    fmiss_cxt_t *cxt = fmi->fmiss_cxt;
    UInt32 sector_idx = 0;
    BOOL32 clean = TRUE32;
    BOOL32 fused = TRUE32;
    BOOL32 uecc = FALSE32;

    if (cxt->max_corrected_array)
    {
        *cxt->max_corrected_array = 0;
    }

    for (sector_idx = 0; sector_idx < fmi->sectors_per_page; sector_idx++)
    {
        const UInt32 ecc_result    = h2fmi_rd(fmi, ECC_RESULT);
        const BOOL32 free_page     = (ecc_result & ECC_RESULT__FREE_PAGE) ? TRUE32 : FALSE32;
        const BOOL32 all_zero      = (ecc_result & ECC_RESULT__ALL_ZERO) ? TRUE32 : FALSE32;
        const BOOL32 uncorrectable = (ecc_result & ECC_RESULT__UNCORRECTABLE) ? TRUE32 : FALSE32;
        const BOOL32 stuck_bit_err = ((ecc_result & ECC_RESULT__STUCK_BIT_CNT(~0)) >
                                      ECC_RESULT__STUCK_BIT_CNT(H2FMI_ALLOWED_STUCK_BITS_IN_FP)) ?
                                     TRUE32 : FALSE32;
        UInt32 err_cnt             = (ecc_result & ECC_RESULT__ERROR_CNT_MASK) >> ECC_RESULT__ERROR_CNT_SHIFT;

        if (free_page)
        {
            if (stuck_bit_err)
            {
                clean = FALSE32;
                fused = FALSE32;
                uecc = TRUE32;
                err_cnt = 0xFF;
            }
            else
            {
                fused = FALSE32;
                err_cnt = 0xFE;
            }
        }
        else if (all_zero)
        {
            clean = FALSE32;
            err_cnt = 0xFF;
        }
        else if (uncorrectable)
        {
            clean = FALSE32;
            fused = FALSE32;
            uecc = TRUE32;
            err_cnt = 0xFF;
        }
        else
        {
            clean = FALSE32;
            fused = FALSE32;
        }

        if (cxt->sector_stats)
        {
            *cxt->sector_stats = err_cnt;
            cxt->sector_stats++;
        }

        if (cxt->max_corrected_array && (err_cnt > *cxt->max_corrected_array))
        {
            *cxt->max_corrected_array = err_cnt;
        }
    }
    cxt->max_corrected_array++;

    if (clean)
    {
        cxt->clean_pages++;
    }
    else if (fused)
    {
        cxt->fused_pages++;
    }
    else if (uecc)
    {
        cxt->uecc_pages++;
    }
    else
    {
        ret = TRUE32;
    }

    if (!ret && (((UInt32) ~0) == fmi->failureDetails.wFirstFailingCE))
    {
        fmi->failureDetails.wFirstFailingCE = *cxt->chip_enable_array;
    }
    cxt->chip_enable_array++;
    cxt->ecc_idx++;

    return ret;
}

static void fmiss_interrupt_handler(void *arg)
{
    h2fmi_t* fmi = (h2fmi_t*)arg;
    const UInt32 fmi_int_pend = h2fmi_rd(fmi, FMI_INT_PEND);

    h2fmi_wr(fmi, FMI_INT_PEND, fmi_int_pend & (FMI_INT_EXPECTED | FMI_INT_ERRORS));

    if (0 != (FMI_INT_ERRORS & fmi_int_pend))
    {
        WMR_PANIC("Unexpected FMI interrupt: 0x%08x", (uint32_t)(FMI_INT_ERRORS & fmi_int_pend));
    }

    if (0 != (FMI_INT_PEND__SEQUENCER_INTERRUPT & fmi_int_pend))
    {
        const UInt32 seq_int_pend = h2fmi_rd(fmi, SEQ_INT_PEND);

        h2fmi_wr(fmi, SEQ_INT_PEND, seq_int_pend & SEQ_INT_ERRORS);

        if (0 != (SEQ_INT_ERRORS & seq_int_pend))
        {
#if H2FMI_IOP
            if (0 != (SEQ_INT_PEND__TIMEOUT & seq_int_pend))
            {
                fmiss_cxt_t *cxt = fmi->fmiss_cxt;
                cxt->timeout = TRUE32;
            }
            else
#endif // H2FMI_IOP
            {
                WMR_PANIC("Unexpected SEQ interrupt: 0x%08x", (uint32_t)(SEQ_INT_ERRORS & seq_int_pend));
            }
        }
    }

    if (0 != (FMI_INT_PEND__LAST_ECC_DONE & fmi_int_pend))
    {
        while (fmi->sectors_per_page <= ECC_RESULTS_FIFO_PTR__LEVEL(h2fmi_rd(fmi, ECC_RESULTS_FIFO_PTR)))
        {
            fmiss_process_ecc(fmi);
        }
    }
}

static void fmiss_yield(h2fmi_t* fmi)
{
    WMR_YIELD();

#if !(H2FMI_WAIT_USING_ISR)
    fmiss_interrupt_handler(fmi);
#endif // !H2FMI_WAIT_USING_ISR
}


static void fmiss_put_command(h2fmi_t *fmi, UInt32 value, UInt8 *level)
{
    const fmiss_cxt_t *cxt = fmi->fmiss_cxt;
    BOOL32 first = TRUE32;

    while ((COMMAND_FIFO_SIZE <= *level) && !cxt->timeout)
    {
        if (!first)
        {
            fmiss_yield(fmi);
        }
        else
        {
            first = FALSE32;
        }
        *level = COMMAND_FIFO_PTR__LEVEL(h2fmi_rd(fmi, COMMAND_FIFO_PTR));
    }

    if (!cxt->timeout)
    {
        h2fmi_wr(fmi, COMMAND_FIFO, value);
        (*level)++;
    }
}

static void fmiss_put_operand(h2fmi_t *fmi, UInt32 value, UInt8 *level)
{
    const fmiss_cxt_t *cxt = fmi->fmiss_cxt;
    BOOL32 first = TRUE32;

    while ((OPERAND_FIFO_SIZE <= *level) && !cxt->timeout)
    {
        if (!first)
        {
            fmiss_yield(fmi);
        }
        else
        {
            first = FALSE32;
        }
        *level = OPERAND_FIFO_PTR__LEVEL(h2fmi_rd(fmi, OPERAND_FIFO_PTR));
    }

    if (!cxt->timeout)
    {
        h2fmi_wr(fmi, OPERAND_FIFO, value);
        (*level)++;
    }
}

UInt32 h2fmi_read_multi(h2fmi_t* fmi,
                        UInt32   page_count,
                        h2fmi_ce_t*  chip_enable_array,
                        UInt32*  page_number_array,
                        struct dma_segment*    data_segment_array,
                        struct dma_segment*    meta_segment_array,
                        UInt8*   max_corrected_array,
                        UInt8*   sector_stats)
{
    UInt32 ret                            = _kIOPFMI_STATUS_SUCCESS;
    UInt32 prep_idx                       = 0;
    UInt32 read_idx                       = 0;
    UInt32 prep_ce_mask                   = 0;
    UInt32 last_ce                        = 0;
    UInt8  command_level                  = 0;
    UInt8  operand_level                  = 0;
    UInt32 fmi_control                    = (FMI_CONTROL__RESET_SEQUENCER |
                                             FMI_CONTROL__ENABLE_SEQUENCER |
                                             FMI_CONTROL__SEQUENCER_TIMEOUT_ENABLE |
                                             FMI_CONTROL__MODE__READ);
    fmiss_cxt_t cxt;

#if FMI_VERSION == 4
    if (fmi->read_stream_disable)
    {
        fmi_control                      |= FMI_CONTROL__DISABLE_STREAMING;
    }
#endif // FMI_VERSION == 4

    // Updated by the ECC interrupt handler
    cxt.chip_enable_array                 = chip_enable_array;
    cxt.max_corrected_array               = max_corrected_array;
    cxt.sector_stats                      = sector_stats;
    cxt.clean_pages                       = 0;
    cxt.fused_pages                       = 0;
    cxt.uecc_pages                        = 0;
    cxt.ecc_idx                           = 0;
    cxt.timeout                           = FALSE32;
    fmi->fmiss_cxt                        = &cxt;

    // Used by IOP layer
    fmi->failureDetails.wNumCE_Executed   = 0;
    fmi->failureDetails.wOverallOperationFailure = 0;
    fmi->failureDetails.wFirstFailingCE   = ~0;

    fmi->isr_handler                      = fmiss_interrupt_handler;

    h2fmi_reset(fmi);
    h2fmi_set_page_format_and_ECC_level(fmi, fmi->correctable_bits + 1);

    h2fmi_wr(fmi, TIMEOUT_VALUE, FMISS_TIMEOUT_VALUE(fmi));
    h2fmi_wr(fmi, FMI_CONTROL, fmi_control);

    h2fmi_wr(fmi, SEQ_INT_EN, SEQ_INT_ENABLE);
    h2fmi_wr(fmi, FMI_INT_EN, FMI_INT_ENABLE);
    h2fmi_wr(fmi, SEQ_INT_PEND, 0xFFFFFFFF);
    h2fmi_wr(fmi, FMI_INT_PEND, 0xFFFFFFFF);

    h2fmi_wr(fmi, FMC_RW_CTRL, 0);
    h2fmi_wr(fmi, ECC_RESULT, ECC_RESULT__FIFO_RESET);
    h2fmi_wr(fmi, FMC_ADDRNUM, FMC_ADDRNUM__NUM(4));

    for (read_idx = 0 ; page_count > read_idx ; read_idx++)
    {
        const UInt8 read_ce = fmiss_ce_to_mask(chip_enable_array[read_idx]);

        while (page_count > prep_idx)
        {
            const UInt8 prep_ce = fmiss_ce_to_mask(chip_enable_array[prep_idx]);
            const UInt32 prep_page = page_number_array[prep_idx];

            if (0 != (prep_ce_mask & prep_ce))
            {
                break;
            }

            if (last_ce != prep_ce)
            {
                fmiss_put_command(fmi, CMD_ENABLE_CHIP(prep_ce), &command_level);
                last_ce = prep_ce;
            }

            fmiss_put_operand(fmi, FMC_ADDR1__SEQ4((prep_page >> 16) & 0xFF), &operand_level);
            fmiss_put_operand(fmi, FMC_ADDR0__SEQ3((prep_page >>  8) & 0xFF) |
                                   FMC_ADDR0__SEQ2((prep_page >>  0) & 0xFF), &operand_level);
            fmiss_put_command(fmi, CMD_MACRO(kPrepPageHead, kPrepPageLength), &command_level);

            if (cxt.timeout)
            {
                break;
            }

            prep_ce_mask |= prep_ce;
            prep_idx++;
            fmi->failureDetails.wNumCE_Executed++;
        }

        if (last_ce != read_ce)
        {
            fmiss_put_command(fmi, CMD_ENABLE_CHIP(read_ce), &command_level);
            last_ce = read_ce;
        }
        fmiss_put_command(fmi, CMD_MACRO(kReadPageHead, kReadPageLength), &command_level);

        if (cxt.timeout)
        {
            break;
        }

        if (0 == read_idx)
        {
            h2fmi_dma_execute_async(DMA_CMD_DIR_RX,
                                    h2fmi_dma_data_chan(fmi),
                                    data_segment_array,
                                    h2fmi_dma_data_fifo(fmi),
                                    page_count * fmi->bytes_per_page,
                                    sizeof(UInt32),
                                    8,
                                    fmi->current_aes_cxt);

            h2fmi_dma_execute_async(DMA_CMD_DIR_RX,
                                    h2fmi_dma_meta_chan(fmi),
                                    meta_segment_array,
                                    h2fmi_dma_meta_fifo(fmi),
                                    page_count * fmi->valid_bytes_per_meta,
                                    sizeof(UInt8),
                                    1,
                                    NULL);
        }

        prep_ce_mask &= ~read_ce;
    }
    fmiss_put_command(fmi, CMD_ENABLE_CHIP(0), &command_level);

    while (page_count > cxt.ecc_idx)
    {
        fmiss_yield(fmi);
        if (cxt.timeout)
        {
            break;
        }
    }

    if (cxt.timeout ||
        !h2fmi_dma_wait(h2fmi_dma_data_chan(fmi), H2FMI_PAGE_TIMEOUT_MICROS) ||
        !h2fmi_dma_wait(h2fmi_dma_meta_chan(fmi), H2FMI_PAGE_TIMEOUT_MICROS))
    {
        h2fmi_dma_cancel(h2fmi_dma_meta_chan(fmi));
        h2fmi_dma_cancel(h2fmi_dma_data_chan(fmi));
        ret = _kIOPFMI_STATUS_DMA_DONE_TIMEOUT;
    }

    if (_kIOPFMI_STATUS_SUCCESS == ret)
    {
        if(0 < cxt.clean_pages)
        {
            ret = (page_count > cxt.clean_pages ) ? _kIOPFMI_STATUS_NOT_ALL_CLEAN : _kIOPFMI_STATUS_BLANK;
        }
        else if (0 < cxt.fused_pages)
        {
            ret = (page_count > cxt.fused_pages ) ? _kIOPFMI_STATUS_AT_LEAST_ONE_UECC : _kIOPFMI_STATUS_FUSED;
        }
        else if (0 < cxt.uecc_pages)
        {
            ret = _kIOPFMI_STATUS_AT_LEAST_ONE_UECC;
        }
    }

    h2fmi_wr(fmi, SEQ_INT_EN, 0);
    h2fmi_wr(fmi, FMI_INT_EN, 0);

    fmi->isr_handler                      = NULL;
    fmi->fmiss_cxt                        = NULL;

    fmi->failureDetails.wOverallOperationFailure = ret;

    return ret;
}

UInt32 *fmiss_raw_macros(UInt32 *count)
{
    if (NULL != count)
    {
        *count = sequencer_macro_count;
    }

    return sequencer_macros;
}

#endif /* FMISS_ENABLED */
