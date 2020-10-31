
// *****************************************************************************
//
// File: H2fmi_private.h
//
// *****************************************************************************
//
// Notes:
//
//  - register bitfield definitions are only good for creating bitfield in
//    register position; add definitions for extracting value from register
//    position
//
// *****************************************************************************
//
// Copyright (C) 2008-2010 Apple, Inc. All rights reserved.
//
// This document is the property of Apple Computer, Inc.
// It is considered confidential and proprietary.
//
// This document may not be reproduced or transmitted in any form,
// in whole or in part, without the express written permission of
// Apple Computer, Inc.
//
// *****************************************************************************

#ifndef _H2FMI_PRIVATE_H_
#define _H2FMI_PRIVATE_H_

#include "WMRFeatures.h"
#include "WMROAM.h"

#if (H2FMI_WAIT_USING_ISR)
#include <sys/task.h>
#endif //H2FMI_WAIT_USING_ISR

#if H2FMI_EFI
#include "SoC.h"
#else //!H2FMI_EFI
#include <platform/soc/hwregbase.h>
#endif

#if FMI_VERSION == 0
#include "H2fmi_8920regs.h"
#elif FMI_VERSION <= 2
#include "H2fmi_8922regs.h"
#elif FMI_VERSION <= 3
#include "H2fmi_8940regs.h"
#elif FMI_VERSION <= 4
#include "H2fmi_8945regs.h"
#elif FMI_VERSION <= 5
#include "H2fmi_8950regs.h"
#endif

#if FMI_VERSION > 0
#define SUPPORT_PPN (1)
#endif

#if FMI_VERSION > 2
#define SUPPORT_TOGGLE_NAND (1)
#endif

#if FMI_VERSION >= 5
#define SUPPORT_NAND_DLL (1)
#endif

#include "H2fmi_dma_types.h"

// =============================================================================
// preprocessor constants
// =============================================================================
#define H2FMI_EARLY_EXIT_ARBITRARY_LOOP_CONST   1000

#define H2FMI_MAX_RESET_DELAY     (50 * 1000)
#define H2FMI_MAX_CE_PER_BUS_POW2 (3UL)
#define H2FMI_MAX_CE_PER_BUS      (1UL << H2FMI_MAX_CE_PER_BUS_POW2)
#define H2FMI_MAX_NUM_BUS         (2UL)
#define H2FMI_MAX_CE_TOTAL        (H2FMI_MAX_CE_PER_BUS * H2FMI_MAX_NUM_BUS)
#define H2FMI_MAX_BANKS_PER_CE    (4)
#define H2FMI_MAX_BANKS_TOTAL     (H2FMI_MAX_CE_TOTAL * H2FMI_MAX_BANKS_PER_CE)

#if FMI_VERSION == 0
#define H2FMI_BYTES_PER_SECTOR       (512UL)
#define H2FMI_MAX_META_PER_ENVELOPE  (32)
#define H2FMI_MAX_META_PER_LBA       (32)
#define H2FMI_MAX_LBA_PER_PAGE       (1)
#else
#define H2FMI_BYTES_PER_SECTOR       (1024UL)
#define H2FMI_MAX_META_PER_ENVELOPE  (63)
#define H2FMI_MAX_META_PER_LBA       (16)
#define H2FMI_MAX_LBA_PER_PAGE       (4)
#endif
#define H2FMI_WORDS_PER_SECTOR  (H2FMI_BYTES_PER_SECTOR / sizeof(UInt32))

#define H2FMI_BOOT_SECTORS_PER_PAGE  (3UL)
#define H2FMI_BYTES_PER_BOOT_SECTOR  (512)
#define H2FMI_BOOT_BYTES_PER_PAGE    (H2FMI_BYTES_PER_BOOT_SECTOR * H2FMI_BOOT_SECTORS_PER_PAGE)
#define H2FMI_MAX_SGL_SEGMENTS_PER_RAW (20)

#define H2FMI_DEFAULT_TIMEOUT_MICROS  ((100 * 1000))
#define H2FMI_PAGE_TIMEOUT_MICROS     ((2 * 1000 * 1000))

#define H2FMI_NAND_ID_SIZE            (8)
#define H2FMI_NAND_ID_COMPARISON_SIZE (4)

#define UNIQUE_ID_LEN                 (16)

#if FMI_VERSION == 0
#define MAX_ECC_CORRECTION        (16)
#else
#define MAX_ECC_CORRECTION        (30)
#endif

#define H2FMI_ALLOWED_STUCK_BITS_IN_FP (2)

// conservative bus timings used for boot and Read Id during init
#define H2FMI_INIT_READ_CYCLE_NANOS   (200)
#define H2FMI_INIT_READ_SETUP_NANOS   (200)
#define H2FMI_INIT_READ_HOLD_NANOS    (200)
#define H2FMI_INIT_READ_DELAY_NANOS   (0)
#define H2FMI_INIT_WRITE_CYCLE_NANOS  (200)
#define H2FMI_INIT_WRITE_SETUP_NANOS  (200)
#define H2FMI_INIT_WRITE_HOLD_NANOS   (200)

#define H2FMI_INIT_SOC_TO_NAND_RISE_NANOS (10)
#define H2FMI_INIT_SOC_TO_NAND_FALL_NANOS (10)
#define H2FMI_INIT_NAND_TO_SOC_RISE_NANOS (10)
#define H2FMI_INIT_NAND_TO_SOC_FALL_NANOS (10)

#define METADATA_LOOKUP_SIZE        (256)
#define METADATA_MULTIPLY           (1664525)
#define METADATA_INCREMENT          (1013904223)
#define METADATA_ITERATIONS_TABLE   (763)
#define METADATA_ITERATIONS_CRYPT   (3)

#if (FMI_VERSION <= 2) || ((FMI_VERSION == 3) && (FMI_VERSION_MINOR == 0))
#define H2FMI_DMA_BURST_CYCLES      (32)
#else // ((FMI_VERSION == 3) && (FMI_VERSION_MINOR != 0)) || (FMI_VERSION >= 4)
#define H2FMI_DMA_BURST_CYCLES      (8)
#endif // ((FMI_VERSION == 3) && (FMI_VERSION_MINOR != 0)) || (FMI_VERSION >= 4)

#if FMI_VERSION >= 3
#define FMISS_ENABLED    (1)
#endif

#if FMI_VERSION >= 5
#define H2FMI_EXTRA_CE_SETUP_TIME_US (10)
#endif // FMI_VERSION >= 5

#define H2FMI_IF_CTRL_LOW_POWER                                 \
    (FMC_IF_CTRL__DCCYCLE(0) |                                  \
     FMC_IF_CTRL__REB_SETUP(FMC_IF_CTRL__TIMING_MAX_CLOCKS) |   \
     FMC_IF_CTRL__REB_HOLD(FMC_IF_CTRL__TIMING_MAX_CLOCKS) |    \
     FMC_IF_CTRL__WEB_SETUP(FMC_IF_CTRL__TIMING_MAX_CLOCKS) |   \
     FMC_IF_CTRL__WEB_HOLD(FMC_IF_CTRL__TIMING_MAX_CLOCKS))

// =============================================================================
// type declarations
// =============================================================================

typedef UInt16 h2fmi_ce_t;

typedef UInt8  h2fmi_chipid_t[H2FMI_NAND_ID_SIZE];

typedef enum
{
    writeIdle = 0,
    writeSetup,
    writeWaitForCE,
    writeXferData,
    writeEndingWrite,
    writeEndingWriteWaitCE,
    writeDone,
    writeWaitingForNumCELimit
} WRITE_STATE;

typedef enum
{
    readIdle        = 0,
    readSetup       = 1,
    readStartWait   = 2,
    readXferData    = 3,
    readWaitECCDone = 4,
    readDone        = 5,
} READ_STATE;


typedef enum
{
    fmiNone  = 0,
    fmiRead  = 1,
    fmiWrite = 2
} FMI_MODE;

#define INVALID_CE_INDEX    ( (UInt32)~0 )

typedef struct tagFMI_ISR_STATE_MACHINE
{
    FMI_MODE currentMode;

    union
    {
        WRITE_STATE wr;
        READ_STATE rd;
    } state;

    UInt32   dirty_ce_mask;
    UInt32   page_idx;
    UInt32   page_count;
    UInt32   currentCE;
    UInt32   savedCurrentCE;
    UInt32   wMaxOutstandingCEWriteOperations;
    UInt32   wOutstandingCEWriteOperations;
    struct 
    {
        UInt32  head,tail,count;
        UInt32  previousCEIndex[ H2FMI_MAX_CE_PER_BUS ];
    } ceCommitQueue;
    
    UInt32   lastPageIndex[ H2FMI_MAX_CE_TOTAL ];
    UInt32   currentWriteDie;
    UInt32   wVendorType;
    UInt32   currentPage;
    UInt32  wNumTimesHoldoffExecuted;

    h2fmi_ce_t*  chip_enable_array;
    UInt32*  page_number_array;

    struct dma_segment* data_segment_array;
    struct dma_segment* meta_segment_array;

    UInt64   wCmdStartTicks;
    UInt64   waitStartTicks;
    UInt64   wPageTimeoutTicks;
    UInt64   wQuarterPageTimeoutTicks;

    BOOL32   fSuccessful;

    BOOL32   needs_prepare;
    UInt32   lastCE;
    UInt8*   max_corrected_array;
    UInt32   clean_pages;
    UInt32   fused_pages;
    UInt32   uecc_pages;
    UInt8*   sector_stats;

#if (H2FMI_WAIT_USING_ISR)
    struct task_event stateMachineDoneEvent;
#endif

} FMI_ISR_STATE_MACHINE, * PFMI_ISR_STATE_MACHINE;

struct _hfmi_aes_iv
{
    UInt8 aes_iv_bytes[16];
};
typedef struct _hfmi_aes_iv hfmi_aes_iv;

#define _kIOPFMI_STATUS_UNKNOWN            ((UInt32) 0)
#define _kIOPFMI_STATUS_SUCCESS            ((UInt32) 1)
#define _kIOPFMI_STATUS_BLANK              ((UInt32) 2)
#define _kIOPFMI_STATUS_PARAM_INVALID      ((UInt32) 0x80000004)
#define _kIOPFMI_STATUS_FMC_DONE_TIMEOUT     ((UInt32) 0x8000001C)
#define _kIOPFMI_STATUS_READY_BUSY_TIMEOUT     ((UInt32) 0x8000001D)
#define _kIOPFMI_STATUS_ECC_CLEANUP     ((UInt32) 0x8000001E)
#define _kIOPFMI_STATUS_ECC_DONE_TIMEOUT     ((UInt32) 0x8000001F)
#define _kIOPFMI_STATUS_PPN_GENERAL_ERROR    ((UInt32) 0x80000020)
/**
 * Special status -- status code from NAND is in lower 16-bits.
 */
#define _kIOPFMI_STATUS_PGM_ERROR     ((UInt32) 0x80200000)
#define _kIOPFMI_STATUS_ERASE_ERROR     ((UInt32) 0x80210000)
#define _kIOPFMI_STATUS_DMA_DONE_TIMEOUT     ((UInt32) 0x80000022)
#define _kIOPFMI_STATUS_NOT_ALL_CLEAN     ((UInt32) 0x80000023)
#define _kIOPFMI_STATUS_AT_LEAST_ONE_UECC     ((UInt32) 0x80000024)
#define _kIOPFMI_STATUS_FUSED                 ((UInt32) 0x80000025)

/**
 * Vendor-specific codes ...
 */
#define _kVS_NONE       0
#define _kVS_HYNIX_2P      1
#define _kVS_TOSHIBA_2P    2
#define _kVS_MICRON_2P     3
#define _kVS_SAMSUNG_2D    4
#define _kVS_SAMSUNG_2P_2D 5

typedef struct _h2fmi_t
{
    UInt32                bus_id;
    
    UInt32*               regs;

    UInt32                clock_speed_khz;
    
    UInt32                num_of_ce;
    UInt32                valid_ces;
    BOOL32                is_ppn;
    BOOL32                is_ppn_channel_init;
    
    UInt32                pages_per_block;
    UInt32                sectors_per_page;
    UInt32                bytes_per_spare;
    UInt32                blocks_per_ce;
    UInt32                banks_per_ce;
    UInt32                total_bytes_per_meta;
    UInt32                valid_bytes_per_meta;
    UInt32                bytes_per_page;
    UInt32                logical_page_size;
    UInt32                dies_per_cs;

    UInt32                if_ctrl;
#if !H2FMI_IOP
    UInt32                if_ctrl_normal;
#endif // !H2FMI_IOP
    UInt32                fmi_config_reg;
    UInt32                fmi_data_size_reg;

#if FMI_VERSION == 4
    BOOL32                read_stream_disable;
#endif // FMI_VERSION == 4
#if FMI_VERSION >= 5
    UInt32                read_tx_page_delay;
#endif // FMI_VERSION >= 5
    BOOL32                retire_on_invalid_refresh;
    
    UInt8                 correctable_bits;

#if SUPPORT_TOGGLE_NAND
    BOOL32                is_toggle_system; // primary mode of operation (DDR or SDR)
    BOOL32                is_toggle;        // current state of FMI (DDR or SDR)
    UInt32                toggle_if_ctrl;
    UInt32                dqs_ctrl;
    UInt32                timing_ctrl_1;
    UInt32                timing_ctrl_2;
    UInt32                useDiffDQS;
    UInt32                useDiffRE;
    UInt32                useVREF;
#endif // SUPPORT_TOGGLE_NAND

    UInt8                 refresh_threshold_bits;
    
    UInt32                isr_condition;

    struct dma_aes_config        aes_cxt;
    struct dma_aes_config*       current_aes_cxt;

    hfmi_aes_iv*          current_aes_iv_array;
    UInt32*               iv_seed_array;

#if (H2FMI_WAIT_USING_ISR)
    struct task_event     isr_event;
#endif

    FMI_ISR_STATE_MACHINE stateMachine;

    struct 
    {
        UInt32  wNumCE_Executed;
        UInt32 wOverallOperationFailure;
        UInt32  wSingleCEStatus;
        UInt32* wCEStatusArray;
        UInt32 wCEStatusArrayModulusMask;
        UInt32 wFirstFailingCE;
    } failureDetails;

    UInt32 wMaxOutstandingCEWriteOperations;

    BOOL32                h2fmi_ppn_panic_on_status_timeout;

    BOOL32                initialized;
    struct _h2fmi_ppn_t   *ppn;
    h2fmi_ce_t             activeCe;
    UInt8                  ceMap[H2FMI_MAX_CE_PER_BUS];

#if FMISS_ENABLED
    void                  *fmiss_cxt;
    void                 (*isr_handler)(void *arg);
#endif // FMISS_ENABLED

    UInt32 *               error_list;
} h2fmi_t;

struct _h2fmi_virt_to_phys_map_t
{
    UInt32     bus;
    h2fmi_ce_t ce;
};
typedef struct _h2fmi_virt_to_phys_map_t h2fmi_virt_to_phys_map_t;

// =============================================================================
// general fail macro
// =============================================================================

#define h2fmi_fail(b)                           \
    do {                                        \
        WMR_PRINT(ERROR, \
                  "[FIL:ERR] FAIL -> %s@%d\n",    \
                  "", __LINE__);          \
        b = FALSE32;                            \
    } while (0)

// =============================================================================
// fmi bus selection macros
//
// Note: this doesn't scale nicely past two buses
// =============================================================================

#define h2fmi_select_by_bus(fmi, sel0, sel1) ((fmi->regs == FMI0) ? sel0 : sel1)
#define h2fmi_bus_index(fmi) h2fmi_select_by_bus(fmi, 0, 1)

// =============================================================================
// dma-related macros
// =============================================================================

#define h2fmi_dma_data_chan(fmi) h2fmi_select_by_bus(fmi, DMA_FMI0DATA, DMA_FMI1DATA)
#define h2fmi_dma_meta_chan(fmi) h2fmi_select_by_bus(fmi, DMA_FMI0CHECK, DMA_FMI1CHECK)

#define h2fmi_dma_meta_fifo(fmi) ((void*)(((UInt32)fmi->regs) + FMI_META_FIFO))
#define h2fmi_dma_data_fifo(fmi) ((void*)(((UInt32)fmi->regs) + FMI_DATA_BUF))

// =============================================================================
// busy wait macro
//
// this should only be used for very short waits that should never halt
// during code sequences where performance matters
// =============================================================================

#define h2fmi_busy_wait(fmi, reg, mask, cond)                           \
    do {                                                                \
        UInt32 val;                                                   \
        do {                                                            \
            val = h2fmi_rd(fmi, reg);                                   \
        } while ((val & (mask)) != (cond));                             \
    } while (0)

// =============================================================================
// register access macros
// =============================================================================

#if (H2FMI_TRACE_REG_WRITES)
UInt32 h2fmi_trace_wr(h2fmi_t* fmi, UInt32 reg, UInt32 val);
UInt8 h2fmi_trace_wr8(h2fmi_t* fmi, UInt32 reg, UInt8 val);
UInt32 h2fmc_trace_wr(volatile UInt32* h2fmc_regs, UInt32 reg, UInt32 val);
#define h2fmi_wr(fmi, reg, val) h2fmi_trace_wr(fmi, reg, val)
#define h2fmi_wr8(fmi, reg, val) h2fmi_trace_wr8(fmi, reg, val)
#if H2FMI_INSTRUMENT_BUS_1
#define h2fmc_wr(fmc_regs, reg, val) h2fmc_trace_wr(fmc_regs, reg, val)
#endif // H2FMI_INSTRUMENT_BUS_1
#else
#define h2fmi_wr(fmi, reg, val)  (*((fmi)->regs + ((reg) >> 2)) = (val))
#define h2fmi_wr8(fmi, reg, val) (((volatile UInt8*)fmi->regs)[(UInt32)reg] = (UInt8)val)
#if H2FMI_INSTRUMENT_BUS_1
#define h2fmc_wr(fmc_regs, reg, val) (fmc_regs[((UInt32)reg) >> 2] = (UInt32)val)
#endif // H2FMI_INSTRUMENT_BUS_1
#endif

#if (H2FMI_TRACE_REG_READS)
UInt32 h2fmi_trace_rd(h2fmi_t* fmi, UInt32 reg);
UInt8 h2fmi_trace_rd8(h2fmi_t* fmi, UInt32 reg);
#define h2fmi_rd(fmi, reg)   h2fmi_trace_rd(fmi, reg)
#define h2fmi_rd8(fmi, reg)  h2fmi_trace_rd8(fmi, reg)
#if H2FMI_INSTRUMENT_BUS_1
#define h2fmc_rd(fmc_regs, reg) h2fmc_trace_rd(fmc_regs, reg)
#endif // H2FMI_INSTRUMENT_BUS_1
#else
#define h2fmi_rd(fmi, reg)   (*(volatile UInt32*)((UInt8*)((fmi)->regs) + (reg)))
#define h2fmi_rd8(fmi, reg)  (((volatile UInt8*)fmi->regs)[(UInt32)reg])
#if H2FMI_INSTRUMENT_BUS_1
#define h2fmc_rd(fmc_regs, reg)  (fmc_regs[((UInt32)reg) >> 2])
#endif // H2FMI_INSTRUMENT_BUS_1
#endif

// =============================================================================
// nand device command bytes
// =============================================================================

#define NAND_CMD__RESET          ((UInt8)0xFF)
#define NAND_CMD__READ_ID        ((UInt8)0x90)
#define NAND_CMD__READ_STATUS    ((UInt8)0x70)
#define NAND_CMD__READ_SS_DIE1   ((UInt8)0xF1)
#define NAND_CMD__READ_SS_DIE2   ((UInt8)0xF2)
#define NAND_CMD__READ_MICRON_DIE_STATUS    ((UInt8)0x78)
#define NAND_CMD__READ           ((UInt8)0x00)
#define NAND_CMD__READ_CONFIRM   ((UInt8)0x30)
#define NAND_CMD__ERASE          ((UInt8)0x60)
#define NAND_CMD__ERASE_CONFIRM  ((UInt8)0xD0)
#define NAND_CMD__WRITE          ((UInt8)0x80)
#define NAND_CMD__WRITE2_HYNIX   ((UInt8)0x81)
#define NAND_CMD__WRITE2_SAMSUNG   ((UInt8)0x81)
#define NAND_CMD__WRITE_CONFIRM  ((UInt8)0x10)
#define NAND_CMD__DUMMY_CONFIRM_HYNIX  ((UInt8)0x11)
#define NAND_CMD__DUMMY_CONFIRM_MICRON  ((UInt8)0x11)
#define NAND_CMD__DUMMY_CONFIRM_TOSHIBA  ((UInt8)0x11)
#define NAND_CMD__DUMMY_CONFIRM_SAMSUNG  ((UInt8)0x11)
#define NAND_CMD__CONFIRM_TOSHIBA  ((UInt8)0x15)
#define NAND_CMD__TOS_STATUS_71h ((UInt8)0x71)

#define CHIPID_ADDR      (0x00)
#define MfgID_ADDR       (0x30)
//Mask corresponding to max of ChipId size and MfgID size
#define MAX_ID_SIZE_MASK (0x0000FFFFFFFFFFFF) 
// =============================================================================
// nand operations timeout specifications
// =============================================================================

#define TIMEOUT_MICROSEC_READ    ((UInt32)1000)
#define TIMEOUT_MICROSEC_WRITE   ((UInt32)5000)
#define TIMEOUT_MICROSEC_ERASE   ((UInt32)15000)

// =============================================================================
// nand operations status specifications
// =============================================================================

#define NAND_STATUS__CHIP_STATUS1                 ((UInt8)1 << 0)
#define NAND_STATUS__CHIP_STATUS1_FAIL            ((UInt8)1 << 0)
#define NAND_STATUS__CHIP_STATUS1_PASS            ((UInt8)0 << 0)
#define NAND_STATUS__TOS_CHIP_STATUS_MASK         ((UInt8)0x1f)
#define NAND_STATUS__TOS_CHIP_STATUS_D0           ((UInt8)0x0a)
#define NAND_STATUS__TOS_CHIP_STATUS_D1           ((UInt8)0x014)
#define NAND_STATUS__SS_CHIP_STATUS_P0            ((UInt8)0x2)
#define NAND_STATUS__CHIP_STATUS2                 ((UInt8)1 << 1)
#define NAND_STATUS__CHIP_STATUS2_FAIL            ((UInt8)1 << 1)
#define NAND_STATUS__CHIP_STATUS2_PASS            ((UInt8)0 << 1)
// bits 2-4 are not used
#define NAND_STATUS__PAGE_BUFFER_RB               ((UInt8)1 << 5)
#define NAND_STATUS__PAGE_BUFFER_RB_READY         ((UInt8)1 << 5)
#define NAND_STATUS__PAGE_BUFFER_RB_BUSY          ((UInt8)0 << 5)
#define NAND_STATUS__DATA_CACHE_RB                ((UInt8)1 << 6)
#define NAND_STATUS__DATA_CACHE_RB_READY          ((UInt8)1 << 6)
#define NAND_STATUS__DATA_CACHE_RB_BUSY           ((UInt8)0 << 6)
#define NAND_STATUS__WRITE_PROTECT                ((UInt8)1 << 7)
#define NAND_STATUS__WRITE_PROTECT_NOT_PROTECTED  ((UInt8)1 << 7)
#define NAND_STATUS__WRITE_PROTECT_PROTECTED      ((UInt8)0 << 7)

// =============================================================================
// implementation function declarations shared between SecureROM and iBoot
// =============================================================================

void h2fmi_reset(h2fmi_t* fmi);

BOOL32 h2fmi_is_chipid_invalid(h2fmi_chipid_t* const id);

BOOL32 h2fmi_wait_done(h2fmi_t* fmi, UInt32 reg, UInt32 mask, UInt32 bits);
BOOL32 h2fmi_wait_dma_task_pending(h2fmi_t* fmi);

void h2fmi_clean_ecc(h2fmi_t* fmi);
void h2fmi_fmc_read_data(h2fmi_t* fmi, UInt32 size, UInt8* data);

BOOL32 h2fmi_nand_reset(h2fmi_t* fmi, h2fmi_ce_t ce);
BOOL32 h2fmi_nand_read_id(h2fmi_t* fmi, h2fmi_ce_t ce, h2fmi_chipid_t* id, UInt8 addr);

BOOL32 h2fmi_pio_write_sector(h2fmi_t* fmi, const void* buf, UInt32 len);
BOOL32 h2fmi_pio_read_sector(h2fmi_t* fmi, void* buf, UInt32 len);

// =============================================================================
// implementation function declarations not used by SecureROM
// =============================================================================

UInt32 h2fmi_get_gate(h2fmi_t* fmi);

void h2fmi_init_sys(h2fmi_t* fmi);

#if (!H2FMI_READONLY || AND_SUPPORT_NVRAM)
BOOL32 h2fmi_erase_blocks(h2fmi_t* fmi, UInt32   num_elements, UInt16* ce, UInt32* block, BOOL32* status_failed);
UInt32 h2fmi_write_bootpage(h2fmi_t* fmi, UInt32 ce, UInt32 page, UInt8* data_buf);
#endif //!H2FMI_READONLY

#if (!H2FMI_READONLY)
BOOL32 h2fmi_write_page(h2fmi_t* fmi, UInt16 ce, UInt32 page, UInt8* data_buf, UInt8* meta_buf, BOOL32* status_failed);
UInt32 h2fmi_write_raw_page(h2fmi_t* fmi, UInt32 ce, UInt32 page, UInt8* data_buf);
#endif //!H2FMI_READONLY

UInt32 h2fmi_read_page(h2fmi_t* fmi, h2fmi_ce_t ce, UInt32 page, UInt8* data_buf, UInt8* meta_buf, UInt8* max_corrected, UInt8* sector_stats);

BOOL32 h2fmi_read_multi(h2fmi_t* fmi, UInt32 page_count, h2fmi_ce_t* chip_enable_array, UInt32*  page_number_array, struct dma_segment* data_segment_array, struct dma_segment* meta_segment_array, UInt8* max_corrected_array, UInt8* sector_stats);

#if (defined(ENABLE_VENDOR_UNIQUE_QUERIES) && ENABLE_VENDOR_UNIQUE_QUERIES)
BOOL32 h2fmiGenericNandReadSequence(h2fmi_t *whichFmi, UInt8 phyCE, GenericReadRequest *genericRead);
#endif //(defined(ENABLE_VENDOR_UNIQUE_QUERIES) && ENABLE_VENDOR_UNIQUE_QUERIES)

BOOL32 h2fmi_reset_and_read_chipids(h2fmi_t* fmi, h2fmi_chipid_t* ids, UInt8 addr);
BOOL32 h2fmi_nand_read_id_all(h2fmi_t* fmi, h2fmi_chipid_t* ids, UInt8 addr);
BOOL32 h2fmi_nand_reset_all(h2fmi_t* fmi);

BOOL32 h2fmi_write_multi(h2fmi_t* fmi, UInt32 page_count, h2fmi_ce_t* chip_enable_array, UInt32* page_number_array, struct dma_segment* data_segment_array, struct dma_segment* meta_segment_array, BOOL32* status_failed, UInt32 wVendorType);

void h2fmi_nand_read_chipid(h2fmi_t* fmi, h2fmi_ce_t ce, h2fmi_chipid_t* id);
void h2fmi_build_ce_bitmask(h2fmi_t* fmi, h2fmi_chipid_t* ids, h2fmi_chipid_t* id_filter, UInt8 addr);

void h2fmi_set_bootpage_data_format(h2fmi_t* fmi);
void h2fmi_set_raw_write_data_format(h2fmi_t* fmi, UInt32 spare_per_sector);
void h2fmi_set_page_format(h2fmi_t* fmi);
void h2fmi_config_page_addr(h2fmi_t* fmi, UInt32 page);

UInt32 h2fmi_calculate_ecc_bits(h2fmi_t* fmi);
UInt32 h2fmi_calculate_ecc_output_bytes(h2fmi_t* fmi);

BOOL32 h2fmi_read_raw_page(h2fmi_t* fmi, UInt32 ce, UInt32 page, UInt8* data_buf, UInt8* spare_buf);
UInt32 h2fmi_read_bootpage(h2fmi_t* fmi, UInt32 ce, UInt32 page, UInt8* data_buf, UInt8* max_corrected);
UInt32 h2fmi_read_bootpage_pio(h2fmi_t* fmi, UInt32 ce, UInt32 page, UInt8* data_buf, UInt8* max_corrected);

void h2fmi_prepare_wait_status(h2fmi_t* fmi, UInt8 io_mask, UInt8 io_cond);
BOOL32 h2fmi_wait_status(h2fmi_t* fmi, UInt8 io_mask, UInt8 io_cond, UInt8* status);

h2fmi_t *h2fmiTranslateVirtualCEToBus(UInt32 virtualCE);
h2fmi_ce_t h2fmiTranslateVirtualCEToCe(UInt32 virutalCE);

void h2fmi_set_if_ctrl(h2fmi_t* fmi, UInt32 if_ctrl);
void restoreTimingRegs(h2fmi_t* fmi);

// =============================================================================
// implementation function declarations
// =============================================================================
BOOL32 h2fmi_init(h2fmi_t* fmi, UInt32 interface, h2fmi_chipid_t* ids);
Int32 h2fmiInitVirtToPhysMap(UInt16 busCount, UInt16 bankCount);
void h2fmiMapBankToBusAndEnable(UInt32 bank, UInt32 bus, h2fmi_ce_t enable);
h2fmi_t* h2fmiTranslateBankToBus(UInt32 bank);
h2fmi_ce_t h2fmiTranslateBankToCe(UInt32 bank);
void h2fmi_fmc_enable_ce(h2fmi_t* fmi, h2fmi_ce_t ce);
void h2fmi_fmc_disable_ce(h2fmi_t* fmi, h2fmi_ce_t ce);
void h2fmi_fmc_disable_all_ces(h2fmi_t* fmi);
void h2fmi_aes_iv(void* arg, UInt32 chunk_index, void* iv_buffer);
void h2fmi_whitening_iv(void* arg, UInt32 chunk_index, void* iv_buffer);

// =============================================================================
// implementation function declarations used only for debug during development
//
// note that these are designed so that the calls will disappear when
// not compiling for debug
// =============================================================================

#if (H2FMI_DEBUG)

BOOL32 h2fmi_test_hook(h2fmi_t* fmi);

void h2fmi_dump_config(h2fmi_t* fmi);
void h2fmi_dump_status_regs(h2fmi_t* fmi, const char* prefix);
void h2fmi_dump_fmi_regs(h2fmi_t* fmi, const char* prefix);
void h2fmi_dump_fmc_regs(h2fmi_t* fmi, const char* prefix);
void h2fmi_dump_ecc_regs(h2fmi_t* fmi, const char* prefix);
void h2fmi_dump_regs(h2fmi_t* fmi, const char* prefix);
void h2fmi_dump_buffer(UInt8* buf, UInt32 size);
void h2fmi_spew_regs(h2fmi_t* fmi, const char* prefix);

#else // H2FMI_DEBUG

#define h2fmi_test_hook(fmi_ignored)  (TRUE32)

#define h2fmi_dump_config(fmi_ignored)
#define h2fmi_dump_status_regs(fmi_ignored, prefix_ignored)
#define h2fmi_dump_fmi_regs(fmi_ignored, prefix_ignored)
#define h2fmi_dump_fmc_regs(fmi_ignored, prefix_ignored)
#define h2fmi_dump_ecc_regs(fmi_ignored, prefix_ignored)
#define h2fmi_dump_regs(fmi_ignored, prefix_ignored)
#define h2fmi_dump_buffer(buf_ignored, size_ignored)

#endif // H2FMI_DEBUG

#if SUPPORT_TOGGLE_NAND
#define turn_on_fmc(fmi)    (h2fmi_wr((fmi), FMC_ON, FMC_ON__ENABLE | (((fmi)->is_toggle) ? (FMC_ON__DDR_NAND_TYPE_TOGGLE | FMC_ON__DDR_ENABLE) : 0)))
#else
#define turn_on_fmc(fmi)    (h2fmi_wr((fmi), FMC_ON, FMC_ON__ENABLE))
#endif

void h2fmi_handle_write_ISR_state_machine( h2fmi_t* fmi );
void h2fmi_handle_read_ISR_state_machine( h2fmi_t* fmi );

typedef enum
{
    CE_TIMEOUT,
    CE_BUSY,
    CE_IDLE
} CE_STATE;

CE_STATE h2fmi_get_current_CE_state( h2fmi_t* fmi );
BOOL32 h2fmi_tick_delta_timeout(UInt64 wStartTick, UInt64 wMaxDeltaTick);

void h2fmi_clear_interrupts_and_reset_masks(h2fmi_t* fmi);
void h2fmi_prepare_for_ready_busy_interrupt(h2fmi_t* fmi);
void h2fmi_set_page_format_and_ECC_level(h2fmi_t* fmi, UInt32 wErrorAlertLevel);
BOOL32 h2fmi_common_idle_handler( h2fmi_t* fmi );

BOOL32 h2fmi_start_dma( h2fmi_t* fmi );

#if ( H2FMI_INSTRUMENT_BUS_1 )
extern void h2fmi_instrument_bit_set(int iBit);
extern void h2fmi_instrument_bit_clear(int iBit);
#endif

UInt32 h2fmi_calculate_fmi_config(h2fmi_t* fmi);
UInt32 h2fmi_calculate_fmi_data_size(h2fmi_t* fmi);

void h2fmi_restoreFmiRegs(h2fmi_t *fmi);
void fmiss_init_sequences(h2fmi_t *fmi);
UInt32 h2fmi_rx_check_page_ecc(h2fmi_t* fmi, UInt32 ecc_pnd, UInt8* max_corrected, UInt8* sector_stats, UInt32 sectors_per_page);

#if FMISS_ENABLED
void fmiss_raw_init_sequences(h2fmi_t *fmi);
#endif // FMISS_ENABLED

#if SUPPORT_TOGGLE_NAND && !APPLICATION_EMBEDDEDIOP
void transitionWorldFromDDR(UInt32 powerstate_to);
void transitionWorldToDDR(UInt32 powerstate_from);
#endif

#if !APPLICATION_EMBEDDEDIOP
void h2fmi_ppn_all_channel_power_state_transition(UInt32 ps_tran);
#endif // !APPLICATION_EMBEDDEDIOP


#if SUPPORT_NAND_DLL
BOOL32 h2fmiTrainDLL(UInt32 *lock);
#endif

#endif // _H2FMI_PRIVATE_H_

// ********************************** EOF **************************************
