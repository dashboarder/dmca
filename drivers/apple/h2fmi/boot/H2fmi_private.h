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

#ifndef _H2FMI_PRIVATE_H_
#define _H2FMI_PRIVATE_H_

#include <sys/task.h>

#include "H2fmi_regs.h"


// =============================================================================
// preprocessor platform identification
// =============================================================================

#if (defined(APPLICATION_SECUREROM) && APPLICATION_SECUREROM)
#define H2FMI_BOOTROM true
#else
#define H2FMI_BOOTROM false
#endif

#if (defined(APPLICATION_EMBEDDEDIOP) && APPLICATION_EMBEDDEDIOP)
#define H2FMI_IOP true
#else
#define H2FMI_IOP false
#endif

#if (defined(APPLICATION_IBOOT) && APPLICATION_IBOOT)
#define H2FMI_IBOOT true
#else
#define H2FMI_IBOOT false
#endif

// TODO: add EFI platform identification


// =============================================================================
// configurable preprocessor compilation control
// =============================================================================

// Set H2FMI_DEBUG below to true if you want to build with extra
// debugging features (default to false).
#define H2FMI_DEBUG false

// Set H2FMI_TEST_HOOK below to true if you want to insert tests at the end
// of FIL_Init in iBoot (default to false).
#define H2FMI_TEST_HOOK false

// Set H2FMI_DMA_SYNC_READ and/or H2FMI_DMA_SYNC_WRITE to true in order to
// force read and/or write operations to only use the synchronous interface
// to the CDMA.  This is a workaround for a problem with async dma that is
// currently under investigation.
#define H2FMI_DMA_SYNC_READ false
#define H2FMI_DMA_SYNC_WRITE false

// Set H2FMI_WAIT_USING_ISR to true if you want operations to wait for
// dma and bus events by hooking an interrupt service routine to the
// FMI interrupt vector; set to false for waiting using register
// polling with yield (default to true).
#define H2FMI_WAIT_USING_ISR true


// =============================================================================
// fixed preprocessor compilation control
// =============================================================================

// Always build SecureROM read-only.
#if (H2FMI_BOOTROM)
#define H2FMI_READONLY true
// Also, EmbeddedIOP builds should always be read/write.
#elif (H2FMI_IOP)
#define H2FMI_READONLY false
// Otherwise, ignore nand driver read-only configuration in iBoot
// debug builds so that erase/write are available for use in testing
// read operations.
#elif (H2FMI_IBOOT && defined(AND_READONLY) && H2FMI_DEBUG)
#define H2FMI_READONLY false
// Otherwise, mirror nand driver configuration in iBoot.
#elif (defined(AND_READONLY))
#define H2FMI_READONLY true
#else
#define H2FMI_READONLY false
#endif


// =============================================================================
// preprocessor constant definitions
// =============================================================================

#define H2FMI_META_PER_ENVELOPE 10
#define H2FMI_BYTES_PER_META    10

#define H2FMI_MAX_DEVICE        8UL
#define H2FMI_BYTES_PER_SECTOR  512UL
#define H2FMI_WORDS_PER_SECTOR  (H2FMI_BYTES_PER_SECTOR / sizeof(uint32_t))

#define H2FMI_BOOT_SECTORS_PER_PAGE  3UL
#define H2FMI_BOOT_BYTES_PER_PAGE    (H2FMI_BYTES_PER_SECTOR * H2FMI_BOOT_SECTORS_PER_PAGE)

#define H2FMI_DEFAULT_TIMEOUT_MICROS  ((utime_t)(2 * 1000 * 1000))
#define H2FMI_PAGE_TIMEOUT_MICROS     ((utime_t)(2 * 1000 * 1000))

#define H2FMI_NAND_ID_SIZE  5

// conservative bus timings used for boot and Read Id during init
#define H2FMI_INIT_READ_CYCLE_NANOS   200
#define H2FMI_INIT_READ_SETUP_NANOS   100
#define H2FMI_INIT_READ_HOLD_NANOS    100
#define H2FMI_INIT_WRITE_CYCLE_NANOS  200
#define H2FMI_INIT_WRITE_SETUP_NANOS  100
#define H2FMI_INIT_WRITE_HOLD_NANOS   100


// =============================================================================
// type declarations
// =============================================================================

typedef uint32_t h2fmi_ce_t;

typedef uint32_t h2fmi_chipid_t;

struct _h2fmi_t
{
    volatile uint32_t* regs;

    uint32_t           num_of_ce;
    uint32_t           pages_per_block;
    uint32_t           sectors_per_page;
    uint32_t           bytes_per_spare;
    uint32_t           blocks_per_ce;
    uint32_t           banks_per_ce;
    uint32_t           bytes_per_meta;
    uint32_t           if_ctrl;

    uint32_t           isr_condition;

    struct task_event  dma_data_done_event;
    struct task_event  dma_meta_done_event;

    struct task_event  isr_event;

    bool               initialized;
};
typedef struct _h2fmi_t h2fmi_t;

struct _h2fmi_virt_to_phys_map_t
{
    uint32_t   bus;
    h2fmi_ce_t ce;
};
typedef struct _h2fmi_virt_to_phys_map_t h2fmi_virt_to_phys_map_t;

// =============================================================================
// general fail macro
// =============================================================================

#define h2fmi_fail(b)                           \
    do {                                        \
        dprintf(DEBUG_CRITICAL,                 \
                "[FIL:ERR] FAIL -> %s@%d\n",    \
                __FILE__, __LINE__);            \
        b = false;                              \
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

#define h2fmi_dma_meta_fifo(fmi) ((void*)(((uint32_t)fmi->regs) + FMI_META_FIFO))
#define h2fmi_dma_data_fifo(fmi) ((void*)(((uint32_t)fmi->regs) + FMI_DATA_BUF))


// =============================================================================
// busy wait macro
//
// this should only be used for very short waits that should never halt
// during code sequences where performance matters
// =============================================================================

#define h2fmi_busy_wait(fmi, reg, mask, cond)                           \
    do {                                                                \
        uint32_t val;                                                   \
        do {                                                            \
            val = h2fmi_rd(fmi, reg);                                   \
        } while ((val & (mask)) != (cond));                             \
        h2fmi_wr(fmi, reg, (val & ~(mask)) | (~(cond) & (mask)));       \
    } while (0)


// =============================================================================
// cache operation size macro
// =============================================================================

// TODO: refactor this cache line rounding functionality
#if (H2FMI_IOP)
#define cache_op_size(buf_size) (((buf_size) + (CPU_CACHELINE_SIZE-1)) & ~(CPU_CACHELINE_SIZE-1))
#else
#define cache_op_size(buf_size) buf_size
#endif


// =============================================================================
// nand device command bytes
// =============================================================================

#define NAND_CMD__RESET          ((uint8_t)0xFF)
#define NAND_CMD__READ_ID        ((uint8_t)0x90)
#define NAND_CMD__READ_STATUS    ((uint8_t)0x70)
#define NAND_CMD__READ           ((uint8_t)0x00)
#define NAND_CMD__READ_CONFIRM   ((uint8_t)0x30)
#define NAND_CMD__ERASE          ((uint8_t)0x60)
#define NAND_CMD__ERASE_CONFIRM  ((uint8_t)0xD0)
#define NAND_CMD__WRITE          ((uint8_t)0x80)
#define NAND_CMD__WRITE_CONFIRM  ((uint8_t)0x10)


// =============================================================================
// nand operations timeout specifications
// =============================================================================

#define TIMEOUT_MICROSEC_READ    ((uint32_t)1000)
#define TIMEOUT_MICROSEC_WRITE   ((uint32_t)5000)
#define TIMEOUT_MICROSEC_ERASE   ((uint32_t)15000)


// =============================================================================
// nand operations status specifications
// =============================================================================

#define NAND_STATUS__CHIP_STATUS1                 ((uint8_t)1 << 0)
#define NAND_STATUS__CHIP_STATUS1_FAIL            ((uint8_t)1 << 0)
#define NAND_STATUS__CHIP_STATUS1_PASS            ((uint8_t)0 << 0)
#define NAND_STATUS__CHIP_STATUS2                 ((uint8_t)1 << 1)
#define NAND_STATUS__CHIP_STATUS2_FAIL            ((uint8_t)1 << 1)
#define NAND_STATUS__CHIP_STATUS2_PASS            ((uint8_t)0 << 1)
// bits 2-4 are not used
#define NAND_STATUS__PAGE_BUFFER_RB               ((uint8_t)1 << 5)
#define NAND_STATUS__PAGE_BUFFER_RB_READY         ((uint8_t)1 << 5)
#define NAND_STATUS__PAGE_BUFFER_RB_BUSY          ((uint8_t)0 << 5)
#define NAND_STATUS__DATA_CACHE_RB                ((uint8_t)1 << 6)
#define NAND_STATUS__DATA_CACHE_RB_READY          ((uint8_t)1 << 6)
#define NAND_STATUS__DATA_CACHE_RB_BUSY           ((uint8_t)0 << 6)
#define NAND_STATUS__WRITE_PROTECT                ((uint8_t)1 << 7)
#define NAND_STATUS__WRITE_PROTECT_NOT_PROTECTED  ((uint8_t)1 << 7)
#define NAND_STATUS__WRITE_PROTECT_PROTECTED      ((uint8_t)0 << 7)


// =============================================================================
// implementation function declarations shared between SecureROM and iBoot
// =============================================================================

bool h2fmi_init_minimal(h2fmi_t* fmi, uint32_t interface);

void h2fmi_ungate(h2fmi_t* fmi);
void h2fmi_gate(h2fmi_t* fmi);
void h2fmi_reset(h2fmi_t* fmi);

void h2fmi_calc_bus_timings(h2fmi_t* fmi, uint32_t min_read_cycle_nanos, uint32_t min_read_setup_nanos, uint32_t min_read_hold_nanos, uint32_t min_write_cycle_nanos, uint32_t min_write_setup_nanos, uint32_t min_write_hold_nanos);

bool h2fmi_is_chipid_invalid(uint32_t id);

bool h2fmi_wait_done(h2fmi_t* fmi, uint32_t reg, uint32_t mask, uint32_t bits);
bool h2fmi_wait_dma_task_pending(h2fmi_t* fmi);

void h2fmi_clean_ecc(h2fmi_t* fmi);
void h2fmi_fmc_read_data(h2fmi_t* fmi, uint32_t size, uint8_t* data);

bool h2fmi_nand_reset(h2fmi_t* fmi, h2fmi_ce_t ce);
bool h2fmi_nand_read_id(h2fmi_t* fmi, h2fmi_ce_t ce, uint32_t* id);

bool h2fmi_pio_read_sector(h2fmi_t* fmi, void* buf);


// =============================================================================
// implementation function declarations not used by SecureROM
// =============================================================================

#if (!H2FMI_BOOTROM)

bool h2fmi_init_fil(h2fmi_t* fmi, uint32_t interface, void* scratch_buf);

void h2fmi_init_sys(h2fmi_t* fmi);

bool h2fmi_erase_blocks(h2fmi_t* fmi, uint32_t ce, uint32_t block, bool* status_failed);
bool h2fmi_write_page(h2fmi_t* fmi, uint32_t ce, uint32_t page, uint8_t* data_buf, uint8_t* meta_buf, bool* status_failed);
bool h2fmi_read_page(h2fmi_t* fmi, uint32_t ce, uint32_t page, uint8_t* data_buf, uint8_t* meta_buf, uint8_t* max_corrected, bool* is_clean);

bool h2fmi_write_multi(h2fmi_t* fmi, uint32_t page_count, uint32_t* chip_enable_array, uint32_t* page_number_array, uint8_t** data_buf_array, uint8_t** meta_buf_array, bool* status_failed);

bool h2fmi_reset_and_read_chipids(h2fmi_t* fmi, h2fmi_chipid_t* ids);
bool h2fmi_nand_reset_all(h2fmi_t* fmi);

void h2fmi_nand_read_chipid(h2fmi_t* fmi, h2fmi_ce_t ce, h2fmi_chipid_t* id);

uint32_t h2fmi_config_raw_data_format(h2fmi_t* fmi);
uint32_t h2fmi_config_raw_spare_format(void);
uint32_t h2fmi_config_page_format(h2fmi_t* fmi);
void h2fmi_config_page_addr(h2fmi_t* fmi, uint32_t page);

bool h2fmi_using_16bit_ecc(h2fmi_t* fmi);

bool h2fmi_read_raw_page(h2fmi_t* fmi, uint32_t ce, uint32_t page, uint8_t* data_buf, uint8_t* spare_buf);

bool h2fmi_wait_status(h2fmi_t* fmi, uint8_t io_mask, uint8_t io_cond, uint8_t* status);

void h2fmi_dma_data_done_handler(void* arg);
void h2fmi_dma_meta_done_handler(void* arg);

void h2fmiInitVirtToPhysMap(void);
void h2fmiMapBankToBusAndEnable(uint32_t bank, uint32_t bus, h2fmi_ce_t enable);
h2fmi_t* h2fmiTranslateBankToBus(uint32_t bank);
h2fmi_ce_t h2fmiTranslateBankToCe(uint32_t bank);

#endif // !H2FMI_BOOTROM


// =============================================================================
// implementation function declarations used only for debug during development
//
// note that these are designed so that the calls will disappear when
// not compiling for debug
// =============================================================================

#if (H2FMI_DEBUG)

bool h2fmi_test_hook(h2fmi_t* fmi);
void h2fmi_spew_config(h2fmi_t* fmi);
void h2fmi_spew_status_regs(h2fmi_t* fmi, const char* prefix);
void h2fmi_spew_fmi_regs(h2fmi_t* fmi, const char* prefix);
void h2fmi_spew_fmc_regs(h2fmi_t* fmi, const char* prefix);
void h2fmi_spew_ecc_regs(h2fmi_t* fmi, const char* prefix);
void h2fmi_spew_regs(h2fmi_t* fmi, const char* prefix);
void h2fmi_spew_buffer(uint8_t* buf, size_t size);

#else

#define h2fmi_test_hook(fmi_ignored) true
#define h2fmi_spew_config(fmi_ignored)
#define h2fmi_spew_status_regs(fmi_ignored, prefix_ignored)
#define h2fmi_spew_fmi_regs(fmi_ignored, prefix_ignored)
#define h2fmi_spew_fmc_regs(fmi_ignored, prefix_ignored)
#define h2fmi_spew_ecc_regs(fmi_ignored, prefix_ignored)
#define h2fmi_spew_regs(fmi_ignored, prefix_ignored)
#define h2fmi_spew_buffer(buf_ignored, size_ignored)

#endif


#endif // _H2FMI_PRIVATE_H_

// ********************************** EOF **************************************
