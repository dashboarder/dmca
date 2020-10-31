// *****************************************************************************
//
// File: H2fmi_misc.c
//
// *****************************************************************************
//
// Notes:
//
//   - Add a global for FMC_IF_CTRL values and avoid reading it on the
//     fly to get the values.
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
#include "H2fmi_dma.h"
#if SUPPORT_PPN
#include "H2fmi_ppn.h"
#endif

#if (H2FMI_WAIT_USING_ISR)
#include <platform/soc/hwisr.h>
#include <platform/int.h>
#endif


// =============================================================================
// global variable declarations
// =============================================================================

//AES_KEY_SIZE_128
UInt8 nand_whitening_key[16] = {0x92, 0xa7, 0x42, 0xab, 0x08, 0xc9, 0x69, 0xbf, 0x00, 0x6c, 0x94, 0x12, 0xd3, 0xcc, 0x79, 0xa5};

// =============================================================================
// static implementation function declarations
// =============================================================================

#if (H2FMI_WAIT_USING_ISR)
static void h2fmi_init_isr(h2fmi_t* fmi);
static void h2fmi_isr_handler(void* arg);
#endif

// =============================================================================
// public implementation function definitions
// =============================================================================

void h2fmi_init_sys(h2fmi_t* fmi)
{
#if (H2FMI_WAIT_USING_ISR)
    h2fmi_init_isr(fmi);
#endif

#if (H2FMI_WAIT_USING_ISR)
    // init state machine event
    event_init(&fmi->stateMachine.stateMachineDoneEvent, EVENT_FLAG_AUTO_UNSIGNAL, false);
#endif

    // FMI manages ECC interaction, so make certain we aren't
    // unmasking any interrupt sources
    h2fmi_wr(fmi, ECC_MASK, 0x0);

    fmi->h2fmi_ppn_panic_on_status_timeout = TRUE32;
}

void h2fmi_prepare_wait_status(h2fmi_t* fmi, UInt8 io_mask, UInt8 io_cond)
{
    UInt32 wReadStatusCmd;

    // turn off RBBEN and configure it to use look for mask value on I/O lines
    h2fmi_set_if_ctrl(fmi, (~FMC_IF_CTRL__RBBEN & h2fmi_rd(fmi, FMC_IF_CTRL)));
    h2fmi_wr(fmi, FMC_RBB_CONFIG, (FMC_RBB_CONFIG__POL(io_cond) |
                                   FMC_RBB_CONFIG__POS(io_mask)));

    if ( fmiWrite==fmi->stateMachine.currentMode )
    {
        switch ( fmi->stateMachine.wVendorType )
        {
            case _kVS_TOSHIBA_2P:
                wReadStatusCmd = NAND_CMD__TOS_STATUS_71h;
                break;
            case _kVS_SAMSUNG_2P_2D:
                wReadStatusCmd = ( 0==(fmi->stateMachine.page_idx&2) ? NAND_CMD__READ_SS_DIE1 : NAND_CMD__READ_SS_DIE2 );
                break;
            case _kVS_SAMSUNG_2D:
                wReadStatusCmd = ( 0==(fmi->stateMachine.page_idx&1) ? NAND_CMD__READ_SS_DIE1 : NAND_CMD__READ_SS_DIE2 );
                break;
            case _kVS_NONE:
            default:
                wReadStatusCmd = NAND_CMD__READ_STATUS;
                break;
        }
        
    }
    else
    {
        wReadStatusCmd = NAND_CMD__READ_STATUS;
    }

    // exec Read Status command cycle
    h2fmi_wr(fmi, FMC_CMD, FMC_CMD__CMD1(wReadStatusCmd));
    h2fmi_wr(fmi, FMC_RW_CTRL, FMC_RW_CTRL__CMD1_MODE);

    // busy wait until command cycle completed
    h2fmi_busy_wait(fmi, FMC_STATUS, FMC_STATUS__CMD1DONE, FMC_STATUS__CMD1DONE);
    h2fmi_wr(fmi, FMC_STATUS, FMC_STATUS__CMD1DONE);

    // clear the FMC_STATUS__NSRBBDONE bit in FMC_STATUS
    h2fmi_clear_interrupts_and_reset_masks(fmi);

    // set up read cycle with REBHOLD to wait for ready bit to be
    // set on I/O lines
    h2fmi_wr(fmi, FMC_DATANUM, FMC_DATANUM__NUM(0));
    h2fmi_wr(fmi, FMC_RW_CTRL, (FMC_RW_CTRL__REBHOLD | FMC_RW_CTRL__RD_MODE));

}

BOOL32 h2fmi_wait_status(h2fmi_t* fmi,
                         UInt8    io_mask,
                         UInt8    io_cond,
                         UInt8*   status)
{
    BOOL32 result = TRUE32;

    h2fmi_prepare_wait_status(fmi, io_mask, io_cond);

#if (H2FMI_WAIT_USING_ISR)
    {
        // use NSRBBDone interrupt source to wait for I/O ready state
        h2fmi_wr(fmi, FMC_INTMASK, FMC_INTMASK__NSRBBDONE);
        h2fmi_wr(fmi, FMI_INT_EN, FMI_INT_EN__FMC_NSRBBDONE);
        if (!event_wait_timeout(&fmi->isr_event, H2FMI_PAGE_TIMEOUT_MICROS))
        {
            h2fmi_fail(result);
        }
        else if (!(fmi->isr_condition & FMI_INT_PEND__FMC_NSRBBDONE))
        {
            h2fmi_fail(result);
        }
    }
#else
    {
        // wait until NSRBB done signaled
        if (!h2fmi_wait_done(fmi, FMC_STATUS,
                             FMC_STATUS__NSRBBDONE,
                             FMC_STATUS__NSRBBDONE))
        {
            h2fmi_fail(result);
        }
    }
#endif

    // if status collection desired, grab it while REBHOLD still active
    if (NULL != status)
    {
        *status = (UInt8)h2fmi_rd(fmi, FMC_NAND_STATUS);
    }

    // clear FMC_RW_CTRL, thereby releasing REBHOLD and completing
    // current read cycle
    h2fmi_wr(fmi, FMC_RW_CTRL, 0);

    return result;
}
void h2fmi_nand_read_chipid(h2fmi_t*        fmi,
                            h2fmi_ce_t      ce,
                            h2fmi_chipid_t* id)
{
    UInt32 if_ctrl;
    const UInt32 cmd1_addr_done = (FMC_STATUS__ADDRESSDONE |
                                   FMC_STATUS__CMD1DONE);

    if_ctrl = h2fmi_rd(fmi, FMC_IF_CTRL);

    h2fmi_set_if_ctrl(fmi, (FMC_IF_CTRL__DCCYCLE(0) |
                            FMC_IF_CTRL__REB_SETUP(FMC_IF_CTRL__TIMING_MAX_CLOCKS) |
                            FMC_IF_CTRL__REB_HOLD(FMC_IF_CTRL__TIMING_MAX_CLOCKS) |
                            FMC_IF_CTRL__WEB_SETUP(FMC_IF_CTRL__TIMING_MAX_CLOCKS) |
                            FMC_IF_CTRL__WEB_HOLD(FMC_IF_CTRL__TIMING_MAX_CLOCKS)));

    // enable specified nand device
    h2fmi_fmc_enable_ce(fmi, ce);

    // set up Read Id command and address cycles
    h2fmi_wr(fmi, FMC_CMD, FMC_CMD__CMD1(NAND_CMD__READ_ID));
    h2fmi_wr(fmi, FMC_ADDR0, FMC_ADDR0__SEQ0(0x00));
    h2fmi_wr(fmi, FMC_ADDRNUM, FMC_ADDRNUM__NUM(0));
    h2fmi_wr(fmi, FMC_RW_CTRL, (FMC_RW_CTRL__ADDR_MODE |
                                FMC_RW_CTRL__CMD1_MODE));

    // wait until cmd & addr completion
    h2fmi_busy_wait(fmi, FMC_STATUS, cmd1_addr_done, cmd1_addr_done);
    h2fmi_wr(fmi, FMC_STATUS, cmd1_addr_done);   

    // read correct number of id bytes
    h2fmi_fmc_read_data(fmi, H2FMI_NAND_ID_SIZE, (UInt8*)id);

    // disable all nand devices
    h2fmi_fmc_disable_all_ces(fmi);

    h2fmi_set_if_ctrl(fmi, if_ctrl);
}

// Builds the valid_ces bitmask for all chip IDs that match id_filter
void h2fmi_build_ce_bitmask(h2fmi_t*        fmi,
                            h2fmi_chipid_t* ids,
                            h2fmi_chipid_t* id_filter,
                            UInt8           addr)
{
    UInt32 wIdx;
    h2fmi_chipid_t all_zeros = {0};
    
    // skip it if the filter is zeroes
    if (!WMR_MEMCMP(all_zeros, id_filter, sizeof(h2fmi_chipid_t)))
    {
        return;
    }
    

    for (wIdx = 0; wIdx < H2FMI_MAX_CE_TOTAL; ++wIdx)
    {
        if (WMR_MEMCMP(ids[wIdx], *id_filter, H2FMI_NAND_ID_COMPARISON_SIZE) == 0)
        {
            // Chip ID matches
            if(CHIPID_ADDR == addr)
            {
                _WMR_PRINT("[NAND] Found Chip ID %02X %02X %02X %02X %02X %02X on FMI%d:CE%d\n",
                           (UInt32)ids[wIdx][0],
                           (UInt32)ids[wIdx][1],
                           (UInt32)ids[wIdx][2],
                           (UInt32)ids[wIdx][3],
                           (UInt32)ids[wIdx][4],
                           (UInt32)ids[wIdx][5],
                           fmi->bus_id,
                           wIdx);    
            }			
            fmi->valid_ces |= (1UL << wIdx);
            fmi->num_of_ce += 1;
        }
        else if(!h2fmi_is_chipid_invalid(&ids[wIdx]))
        {
            _WMR_PRINT("[NAND] Ignoring mismatched Chip ID %02X %02X %02X %02X %02X %02X on FMI%d:CE%d\n",
                       (UInt32)ids[wIdx][0],
                       (UInt32)ids[wIdx][1],
                       (UInt32)ids[wIdx][2],
                       (UInt32)ids[wIdx][3],
                       (UInt32)ids[wIdx][4],
                       (UInt32)ids[wIdx][5],
                       fmi->bus_id,
                       wIdx);
        }        
    }    
}

BOOL32 h2fmi_nand_read_id_all(h2fmi_t*        fmi,
                              h2fmi_chipid_t* ids,
                              UInt8           addr)
{
    UInt32 wIdx;
    BOOL32 result = TRUE32;

    WMR_MEMSET(ids, 0, (sizeof(h2fmi_chipid_t) * H2FMI_MAX_CE_TOTAL));
    fmi->valid_ces = 0;
    fmi->num_of_ce = 0;

    for (wIdx = 0; wIdx < H2FMI_MAX_CE_TOTAL; wIdx++)
    {
        if (!h2fmi_nand_read_id(fmi, wIdx, &ids[wIdx], addr))
        {
            h2fmi_fail(result);
            break;
        }
    }

    return result;
}

BOOL32 h2fmi_reset_and_read_chipids(h2fmi_t*        fmi,
                                    h2fmi_chipid_t* ids,
                                    UInt8           addr)
{
    BOOL32 result = TRUE32;

    if (!h2fmi_nand_reset_all(fmi))
    {
        h2fmi_fail(result);
    }
    else
    {
        result = h2fmi_nand_read_id_all(fmi, ids, addr);
    }

    return result;
}

BOOL32 h2fmi_nand_reset_all(h2fmi_t* fmi)
{
    BOOL32 result = TRUE32;

    UInt32 ce_idx;

    for (ce_idx = 0; ce_idx < H2FMI_MAX_CE_TOTAL; ce_idx++)
    {
        // reset next device
        if (!h2fmi_nand_reset(fmi, ce_idx))
        {
            // fail, but continue on to next device regardless
            h2fmi_fail(result);
        }
    }

    // <rdar://problem/8023322> Fix wait periods for initial reset
    // wait 50 ms to allow nand to reset
    WMR_SLEEP_US(H2FMI_MAX_RESET_DELAY);

#if SUPPORT_PPN
#if !H2FMI_IOP
    if (fmi->is_ppn)
    {
        fmi->if_ctrl = H2FMI_IF_CTRL_LOW_POWER;
        restoreTimingRegs(fmi);
    }
#endif // !H2FMI_IOP
#endif // SUPPORT_PPN

    return result;
}

#if FMI_VERSION == 0
UInt32 h2fmi_config_sectors_to_page_size(h2fmi_t* fmi)
{
    UInt32 fmi_config = 0xFFFFFFFF;

    // configure page size based device id table
    switch (fmi->sectors_per_page)
    {
        case (4):
            fmi_config = FMI_CONFIG__PAGE_SIZE__2K;
            break;

        case (8):
            fmi_config = FMI_CONFIG__PAGE_SIZE__4K;
            break;

        case (16):
            fmi_config = FMI_CONFIG__PAGE_SIZE__8K;
            break;

        case (1):
            fmi_config = FMI_CONFIG__PAGE_SIZE__512;
            break;

        default:
            WMR_PRINT(ERROR,
                      "[FIL:ERR] (512 x %d)-byte page size unsupported\n",
                      fmi->sectors_per_page);
    }

    return fmi_config;
}
#endif // S5L8920X

void h2fmi_set_raw_write_data_format(h2fmi_t* fmi, UInt32 spare_per_sector)
{
#if FMI_VERSION == 0
    // Use 512 bytes per page when writing.  The H2P FMI_CONFIG register doesn't give us enough
    // bits in the META_PER_PAGE field to do an entire 4KB page's worth of spare so we'll
    // do four 512 byte reads in a row instead.
    //
    // If we have no spare, use LBA_AB rather than BYPASS_ECC.  Otherwise FMI will hang waiting
    // for a DMA operation on the meta channel that'll never come.
    h2fmi_wr(fmi, FMI_CONFIG, ((spare_per_sector ? FMI_CONFIG__LBA_MODE__BYPASS_ECC : FMI_CONFIG__LBA_MODE__LBA_TYPE_AB) |
                               FMI_CONFIG__META_PER_ENVELOPE(spare_per_sector) |
                               FMI_CONFIG__META_PER_PAGE(spare_per_sector) |
                               FMI_CONFIG__PAGE_SIZE__512));
#else
    // With H2A and later we have arbitrary control over page and sector size.  We can make our life easier
    // by just setting the sector size to be sector_data_size + sector_spare_size and pusing
    // everything through the data FIFO.
    h2fmi_wr(fmi, FMI_CONFIG, FMI_CONFIG__ECC_CORRECTABLE_BITS( 0 ) | FMI_CONFIG__DMA_BURST__32_CYCLES);
    h2fmi_wr(fmi, FMI_DATA_SIZE, (FMI_DATA_SIZE__META_BYTES_PER_SECTOR( spare_per_sector ) |
                                  FMI_DATA_SIZE__META_BYTES_PER_PAGE( spare_per_sector ) |
                                  FMI_DATA_SIZE__BYTES_PER_SECTOR( 1024 ) |
                                  FMI_DATA_SIZE__SECTORS_PER_PAGE( 1 )));
#endif
}

void h2fmi_set_bootpage_data_format(h2fmi_t* fmi)
{
#if FMI_VERSION == 0
    h2fmi_wr(fmi, FMI_CONFIG, (FMI_CONFIG__LBA_MODE__NORMAL |
                               FMI_CONFIG__META_PER_ENVELOPE(0) |
                               FMI_CONFIG__META_PER_PAGE(0) |
                               FMI_CONFIG__PAGE_SIZE__512 |
                               FMI_CONFIG__ECC_MODE__16BIT |
                               FMI_CONFIG__DMA_BURST__32_CYCLES));    
#elif FMI_VERSION <= 2
    h2fmi_wr(fmi, FMI_CONFIG, FMI_CONFIG__ECC_CORRECTABLE_BITS( MAX_ECC_CORRECTION ));
    h2fmi_wr(fmi, FMI_DATA_SIZE, (FMI_DATA_SIZE__META_BYTES_PER_SECTOR(0) |
                                  FMI_DATA_SIZE__META_BYTES_PER_PAGE(0) |
                                  FMI_DATA_SIZE__BYTES_PER_SECTOR(512) |
                                  FMI_DATA_SIZE__SECTORS_PER_PAGE(1)));    
#elif FMI_VERSION <= 3
    h2fmi_wr(fmi, FMI_CONFIG, FMI_CONFIG__ECC_CORRECTABLE_BITS( MAX_ECC_CORRECTION ));
    h2fmi_wr(fmi, FMI_DATA_SIZE, (FMI_DATA_SIZE__META_BYTES_PER_SECTOR(0) |
                                  FMI_DATA_SIZE__META_BYTES_PER_PAGE(0) |
                                  FMI_DATA_SIZE__BYTES_PER_SECTOR(512) |
                                  FMI_DATA_SIZE__SECTORS_PER_PAGE(1)));   
    // Do not use written-mark
    h2fmi_wr(fmi, ECC_CON1, (ECC_CON1__ERROR_ALERT_LEVEL(0) |
                             ECC_CON1__INT_ENABLE(0) |
                             ECC_CON1__ALLOWED_STUCK_BIT_IN_FP(MAX_ECC_CORRECTION)));
#else
    // Use randomizer
    h2fmi_wr(fmi, FMI_CONFIG, FMI_CONFIG__ECC_CORRECTABLE_BITS( MAX_ECC_CORRECTION ) |
                              FMI_CONFIG__SCRAMBLE_SEED |
                              FMI_CONFIG__ENABLE_WHITENING);

    h2fmi_wr(fmi, FMI_DATA_SIZE, (FMI_DATA_SIZE__META_BYTES_PER_SECTOR(0) |
                                  FMI_DATA_SIZE__META_BYTES_PER_PAGE(0) |
                                  FMI_DATA_SIZE__BYTES_PER_SECTOR(512) |
                                  FMI_DATA_SIZE__SECTORS_PER_PAGE(1)));
    // Fewer stuck bits are allowed
    h2fmi_wr(fmi, ECC_CON1, (ECC_CON1__ERROR_ALERT_LEVEL(0) |
                             ECC_CON1__INT_ENABLE(0) |
                             ECC_CON1__ALLOWED_STUCK_BIT_IN_FP(2)));
#endif
}


void h2fmi_set_page_format(h2fmi_t* fmi)
{
    h2fmi_wr(fmi, FMI_CONFIG, fmi->fmi_config_reg);
# if FMI_VERSION > 0
    h2fmi_wr(fmi, FMI_DATA_SIZE, fmi->fmi_data_size_reg);
#endif
}


typedef struct
{
    UInt32 eccBytesPerSector;
    UInt32 eccCorrectableBits;
} h2fmi_spare_layout;

#if FMI_VERSION > 0
// H2A, H3P
static const h2fmi_spare_layout _h2fmi_spare_layout [] =
{
    { 53, 30 },
    { 51, 29 },
    { 44, 25 },
    { 28, 16 },
    { 27, 15 },
    {  0, 0 }
};
#else
// H2P
static const h2fmi_spare_layout _h2fmi_spare_layout [] =
{
    { 26, 16 },
    { 13, 8  },
    {  0, 0  }
};
#endif   

UInt32 h2fmi_calculate_ecc_bits(h2fmi_t* fmi)
{
    UInt32 eccMode = 0;
    const h2fmi_spare_layout* layout = _h2fmi_spare_layout;
    const UInt32 bytesForEccPerSector = ((fmi->bytes_per_spare - fmi->valid_bytes_per_meta) /
                                         (fmi->bytes_per_page / H2FMI_BYTES_PER_SECTOR));
    for( ; layout->eccBytesPerSector != 0; layout++ )
    {
        if( layout->eccBytesPerSector <= bytesForEccPerSector )
        {
            eccMode = layout->eccCorrectableBits;
            break;
        }
    }

    WMR_ASSERT(eccMode != 0);

    return eccMode;

}

UInt32 h2fmi_calculate_ecc_output_bytes(h2fmi_t* fmi)
{
    const h2fmi_spare_layout* layout = _h2fmi_spare_layout;
    const UInt32 bytesForEccPerSector = ((fmi->bytes_per_spare - fmi->valid_bytes_per_meta) /
                                         (fmi->bytes_per_page / H2FMI_BYTES_PER_SECTOR));
    for( ; layout->eccBytesPerSector != 0; layout++ )
    {
        if( layout->eccBytesPerSector <= bytesForEccPerSector )
        {
            return layout->eccBytesPerSector;
        }
    }

    return 0;
}

UInt32 h2fmi_calculate_fmi_config(h2fmi_t* fmi)
{
    UInt32  ret;
    
#if FMI_VERSION == 0
    UInt32 eccMode;
    eccMode = (fmi->correctable_bits == 16) ? FMI_CONFIG__ECC_MODE__16BIT :
              FMI_CONFIG__ECC_MODE__8BIT;

    ret = (FMI_CONFIG__LBA_MODE__NORMAL |
           FMI_CONFIG__META_PER_ENVELOPE(fmi->valid_bytes_per_meta) |
           FMI_CONFIG__META_PER_PAGE(fmi->valid_bytes_per_meta) |
           h2fmi_config_sectors_to_page_size(fmi) |
           FMI_CONFIG__DMA_BURST__8_CYCLES |
           eccMode);
#else
    ret = (FMI_CONFIG__ECC_CORRECTABLE_BITS( fmi->correctable_bits ) |
           FMI_CONFIG__DMA_BURST__32_CYCLES);
#endif
    return ret;
}

#if FMI_VERSION > 0
UInt32 h2fmi_calculate_fmi_data_size(h2fmi_t* fmi)
{
    UInt32  ret;
    
    ret = (FMI_DATA_SIZE__META_BYTES_PER_SECTOR( fmi->valid_bytes_per_meta ) |
           FMI_DATA_SIZE__META_BYTES_PER_PAGE( fmi->valid_bytes_per_meta ) |
           FMI_DATA_SIZE__BYTES_PER_SECTOR( H2FMI_BYTES_PER_SECTOR ) |
           FMI_DATA_SIZE__SECTORS_PER_PAGE( fmi->sectors_per_page ));
    return ret;
}
#endif
    

// =============================================================================
// isr-only static implementation function definitions
// =============================================================================

#if (H2FMI_WAIT_USING_ISR)

static void h2fmi_init_isr(h2fmi_t* fmi)
{
    WMR_PRINT(IRQ, "initializing interrupts for FMI%d\n",
              h2fmi_bus_index(fmi));

    // lookup fmi interrupt vector to use based upon fmi block
    UInt32 vector = h2fmi_select_by_bus(fmi, INT_FMI0, INT_FMI1);

    // init isr event and condition
    event_init(&fmi->isr_event, EVENT_FLAG_AUTO_UNSIGNAL, false);
    fmi->isr_condition = 0;
#if FMISS_ENABLED
    fmi->isr_handler = NULL;
#endif // FMISS_ENABLED

    // clear out any preexisting interrupts and mask sources
    h2fmi_clear_interrupts_and_reset_masks(fmi);

    // install interrupt handler
    set_int_type(vector, INT_TYPE_IRQ | INT_TYPE_LEVEL);
    install_int_handler(vector, h2fmi_isr_handler, (void*)fmi);
    unmask_int(vector);
}


static void h2fmi_default_isr_handler(h2fmi_t* fmi)
{
    // capture condition triggering isr
    fmi->isr_condition = h2fmi_rd(fmi, FMI_INT_PEND);

    // clear all interrupts and mask sources
    h2fmi_clear_interrupts_and_reset_masks(fmi);

    // signal isr event
    event_signal(&fmi->isr_event);
}

typedef void (*h2fmi_dispatch_handler)(h2fmi_t* fmi);

static const h2fmi_dispatch_handler h2fmi_isr_dispatchTable[] = {
    h2fmi_default_isr_handler,                  // fmiNone
    h2fmi_handle_read_ISR_state_machine,            // fmiRead
    h2fmi_handle_write_ISR_state_machine            // fmiWrite
};

static void h2fmi_isr_handler(void* arg)
{
    h2fmi_t* fmi = (h2fmi_t*)arg;

#if FMISS_ENABLED
    if (NULL != fmi->isr_handler)
    {
        fmi->isr_handler(fmi);
    }
    else
#endif // FMISS_ENABLED
    {
        WMR_ASSERT( fmi->stateMachine.currentMode <= fmiWrite );
        h2fmi_isr_dispatchTable[ fmi->stateMachine.currentMode ]( fmi );
    }
}

#endif

void h2fmi_clear_interrupts_and_reset_masks(h2fmi_t* fmi)
{
    // mask further interrupts
    h2fmi_wr(fmi, FMC_INTMASK, 0x0);
    h2fmi_wr(fmi, FMI_INT_EN, 0x0);

    // clear interrupt source flags
    h2fmi_wr(fmi, FMC_STATUS, (FMC_STATUS__EMERGENCY2 |
                               FMC_STATUS__EMERGENCY01 |
                               FMC_STATUS__RDATADIRTY |
                               FMC_STATUS__RBBDONE7 |
                               FMC_STATUS__RBBDONE6 |
                               FMC_STATUS__RBBDONE5 |
                               FMC_STATUS__RBBDONE4 |
                               FMC_STATUS__RBBDONE3 |
                               FMC_STATUS__RBBDONE2 |
                               FMC_STATUS__RBBDONE1 |
                               FMC_STATUS__RBBDONE0 |
                               FMC_STATUS__TIMEOUT |
                               FMC_STATUS__NSERR |
                               FMC_STATUS__NSRBBDONE |
                               FMC_STATUS__TRANSDONE |
                               FMC_STATUS__ADDRESSDONE |
                               FMC_STATUS__CMD3DONE |
                               FMC_STATUS__CMD2DONE |
                               FMC_STATUS__CMD1DONE));
    h2fmi_wr(fmi, FMI_INT_PEND, (FMI_INT_PEND__META_FIFO_OVERFLOW |
                                 FMI_INT_PEND__META_FIFO_UNDERFLOW |
                                 FMI_INT_PEND__LAST_FMC_DONE |
                                 FMI_INT_PEND__TRANSACTION_COMPLETE));
}


/**
 * Maps the absolute CE to the required register bit # and
 * provides which bus number that CE is on.
 *
 * Note that CE's are absolute on our system -- CE0 is ALWAYS
 * from FMC0, regardless of which bus we are dealing with.  CE8
 * is ALWAYS bit zero of FMC1 as well.
 *
 * @param ce - desired absolute CE #
 * @param p_wTargetBus pointer to store translated bus number.
 * @param p_targetCE - pointer to store translated CE bit.
 */
static inline void h2fmi_calc_bus_ce(h2fmi_ce_t  ce,
                                     UInt32*     p_wTargetBus,
                                     h2fmi_ce_t* p_targetCE)
{
    *p_wTargetBus = ( ce >> H2FMI_MAX_CE_PER_BUS_POW2 );
    *p_targetCE   = ( ce & (H2FMI_MAX_CE_PER_BUS - 1) );
}


/**
 * Enable the specified CE while ensuring that only one CE is
 * enabled at a time on the given bus.
 *
 * @param fmi - context
 * @param ce - Absolute CE #
 */
void h2fmi_fmc_enable_ce(h2fmi_t* fmi, h2fmi_ce_t ce)
{
    h2fmi_ce_t targetCE;
    UInt32 wTargetBus;

    h2fmi_calc_bus_ce(ce, &wTargetBus, &targetCE);

    if(wTargetBus == fmi->bus_id)
    {
        h2fmi_wr(fmi, FMC_CE_CTRL, FMC_CE_CTRL__CEB(targetCE));
    }
    else
    {
        h2fmi_fmc_disable_all_ces(fmi);
    }
}

/**
 * Disable ONLY the specified CE (using read/modify/write).
 *
 * @param ce - Absolute CE #
 */
void h2fmi_fmc_disable_ce(h2fmi_t* fmi, h2fmi_ce_t ce)
{
    h2fmi_ce_t targetCE;
    UInt32 wTargetBus;

    h2fmi_calc_bus_ce(ce, &wTargetBus, &targetCE);

    if(wTargetBus == fmi->bus_id)
    {
        UInt32 val = h2fmi_rd(fmi, FMC_CE_CTRL);
        h2fmi_wr(fmi, FMC_CE_CTRL, val & ~FMC_CE_CTRL__CEB(targetCE));
    }
}

void h2fmi_fmc_disable_all_ces(h2fmi_t* fmi)
{
    h2fmi_wr(fmi, FMC_CE_CTRL, 0);
}

void h2fmi_prepare_for_ready_busy_interrupt(h2fmi_t* fmi)
{
    h2fmi_prepare_wait_status(fmi,
                              NAND_STATUS__DATA_CACHE_RB,
                              NAND_STATUS__DATA_CACHE_RB_READY);

    // use NSRBBDone interrupt source to wait for I/O ready state
    h2fmi_wr(fmi, FMC_INTMASK, FMC_INTMASK__NSRBBDONE);
    h2fmi_wr(fmi, FMI_INT_EN, FMI_INT_EN__FMC_NSRBBDONE);

}

void h2fmi_set_page_format_and_ECC_level(h2fmi_t* fmi, UInt32 wErrorAlertLevel)
{
    // configure FMI with page format
    h2fmi_set_page_format(fmi);

#if FMI_VERSION == 0
    h2fmi_wr(fmi, ECC_CON1, (ECC_CON1__ERROR_ALERT_LEVEL(wErrorAlertLevel) |
                             ECC_CON1__INT_ENABLE(0)));
#elif FMI_VERSION <= 2
    h2fmi_wr(fmi, ECC_CON1, (ECC_CON1__ERROR_ALERT_LEVEL(wErrorAlertLevel) |
                             ECC_CON1__INT_ENABLE(0) |
                             ECC_CON1__ALLOWED_STUCK_BIT_IN_FP(fmi->correctable_bits)));
#else
    h2fmi_wr(fmi, ECC_CON1, (ECC_CON1__ERROR_ALERT_LEVEL(wErrorAlertLevel) |
                             ECC_CON1__INT_ENABLE(0) |
                             ECC_CON1__ALLOWED_STUCK_BIT_IN_FP(fmi->correctable_bits) |
                             ECC_CON1__IMPLICIT_WRITE_MARK));
#endif
}


/**
 * Checks the current state of the (previously selected) CE,
 * looking to see if it is ready or busy.  Uses the
 * stateMachine's "waitStartTick" and "wPageTimeoutTicks" to
 * determine if the wait has timed out.  Note that both of these
 * fields must be initialized before calling this function.
 *
 * @param fmi
 *
 * @return CE_STATE
 */
CE_STATE h2fmi_get_current_CE_state( h2fmi_t* fmi )
{
    CE_STATE ceState = CE_BUSY;

    if (!(h2fmi_rd(fmi, FMI_INT_PEND) & FMI_INT_PEND__FMC_NSRBBDONE))
    {
        BOOL32 fTimeout = WMR_HAS_TIME_ELAPSED_TICKS( fmi->stateMachine.waitStartTicks, fmi->stateMachine.wPageTimeoutTicks );

        if ( fTimeout )
        {
            ceState = CE_TIMEOUT;
        }
    }
    else
    {
        ceState = CE_IDLE;
    }

    return ceState ;
}

#if ( H2FMI_INSTRUMENT_BUS_1 )
/**
 * Sets the specified CE on FMI1.
 *
 * @param iBit - CE, zero is the first.
 */
void h2fmi_instrument_bit_set(int iBit)
{
    iBit &= 0x03;
    volatile UInt32* wFMCValue = ( 0 == (iBit & 0x02) ? FMI0 : FMI1 );
    iBit  = ( 0 == (iBit & 0x01) ? 2 : 3 );
    UInt32 wCurrentValue = h2fmc_rd(wFMCValue, FMC_CE_CTRL);
    wCurrentValue &= ~( 1 << iBit );
    h2fmc_wr(wFMCValue, FMC_CE_CTRL, wCurrentValue );
}

/**
 * Clears the specified CE on FMI1.
 *
 * @param iBit - CE, zero is the first.
 */
void h2fmi_instrument_bit_clear(int iBit)
{
    iBit &= 0x03;
    volatile UInt32* wFMCValue = ( 0 == (iBit & 0x02) ? FMI0 : FMI1 );
    iBit  = ( 0 == (iBit & 0x01) ? 2 : 3 );
    UInt32 wCurrentValue = h2fmc_rd(wFMCValue, FMC_CE_CTRL);
    wCurrentValue |= ( 1 << iBit );
    h2fmc_wr(wFMCValue, FMC_CE_CTRL, wCurrentValue );
}
#endif

static void h2fmi_rx_prep(h2fmi_t* fmi)
{
    h2fmi_set_page_format_and_ECC_level(fmi, 15);
}

static void h2fmi_tx_prep(h2fmi_t* fmi)
{
    // undocumented silicon "feature": on write, program error alert
    // level greater than bit correction capability to avoid spurious
    // ECC error
    // regardless, I was advised to disable interrupt to FMI from ECC
    h2fmi_set_page_format_and_ECC_level(fmi, fmi->correctable_bits + 1);
    // clean up any ECC block state before starting transfer
    h2fmi_clean_ecc(fmi);

#if (H2FMI_WAIT_USING_ISR)
    // enable appropriate interrupt source
    h2fmi_wr(fmi, FMI_INT_EN, FMI_INT_EN__LAST_FMC_DONE);
#endif
}


BOOL32 h2fmi_common_idle_handler( h2fmi_t* fmi )
{
    /**
     * Called initially from client in non-interrupt context to let
     * us set everything up and start the process.
     */
    BOOL32 bSuccessful = TRUE32;

    fmi->failureDetails.wNumCE_Executed = 0;
    fmi->failureDetails.wOverallOperationFailure = 0;
    
    fmi->failureDetails.wFirstFailingCE = ~0;
    
    fmi->stateMachine.wPageTimeoutTicks = H2FMI_PAGE_TIMEOUT_MICROS * WMR_GET_TICKS_PER_US();
    fmi->stateMachine.wQuarterPageTimeoutTicks = fmi->stateMachine.wPageTimeoutTicks / 4;

    if ( fmiRead == fmi->stateMachine.currentMode)
    {
        h2fmi_rx_prep(fmi);
    }
    else
    {
        h2fmi_tx_prep(fmi);
    }

    return bSuccessful;
}

BOOL32 h2fmi_start_dma( h2fmi_t* fmi )
{
    BOOL32 bSuccessful = TRUE32;

    bSuccessful = h2fmi_dma_execute_async(
        ( fmiRead == fmi->stateMachine.currentMode ? DMA_CMD_DIR_RX : DMA_CMD_DIR_TX ),
        h2fmi_dma_data_chan(fmi),
        fmi->stateMachine.data_segment_array,
        h2fmi_dma_data_fifo(fmi),
        fmi->bytes_per_page * fmi->stateMachine.page_count,
        sizeof(UInt32),
        8,
        fmi->current_aes_cxt);

    if (bSuccessful)
    {
        // use CDMA to feed meta buffer to FMI
        bSuccessful = h2fmi_dma_execute_async(
            ( fmiRead == fmi->stateMachine.currentMode ? DMA_CMD_DIR_RX : DMA_CMD_DIR_TX ),
            h2fmi_dma_meta_chan(fmi),
            fmi->stateMachine.meta_segment_array,
            h2fmi_dma_meta_fifo(fmi),
            fmi->valid_bytes_per_meta * fmi->stateMachine.page_count,
            sizeof(UInt8),
            1,
            NULL);
    }

    return bSuccessful;
}

#define LFSR32(seed) ((seed) & 1) ? ((seed) >> 1) ^ 0x80000061 : ((seed) >> 1)
void h2fmi_aes_iv(void* arg, UInt32 chunk_index, void* iv_buffer)
{
    const h2fmi_t* fmi = (h2fmi_t*)arg;
    if (fmi->current_aes_iv_array)
    {
        const UInt8* chunk_iv = fmi->current_aes_iv_array[chunk_index].aes_iv_bytes;

        WMR_MEMCPY(iv_buffer, chunk_iv, 16);
        WMR_PRINT(CRYPTO, "IV: 0x%08x 0x%08x 0x%08x 0x%08x\n",
                  ((UInt32*)chunk_iv)[0],
                  ((UInt32*)chunk_iv)[1],
                  ((UInt32*)chunk_iv)[2],
                  ((UInt32*)chunk_iv)[3]);
    }
    else
    {
        const UInt32 pPage = fmi->iv_seed_array[chunk_index];
        UInt32* iv = (UInt32*)iv_buffer;
        WMR_PRINT(CRYPTO, "Whitening chunk %d\n", chunk_index);

        iv[0] = LFSR32(pPage);
        iv[1] = LFSR32(iv[0]);
        iv[2] = LFSR32(iv[1]);
        iv[3] = LFSR32(iv[2]);
    }
}

void h2fmi_set_if_ctrl(h2fmi_t* fmi, UInt32 if_ctrl)
{
#if FMI_VERSION == 4
    // there are currently limitations on timings that can be used in H4G,
    // some of which can be over come by disabling streaming (<rdar://problem/9033500>)
#if SUPPORT_TOGGLE_NAND
    if (fmi->is_toggle)
    {
        const UInt32 reb_setup = FMC_IF_CTRL__GET_REB_SETUP(if_ctrl);

        fmi->read_stream_disable = (3 < reb_setup) ? TRUE32 : FALSE32;
    }
    else
#endif // SUPPORT_TOGGLE_NAND
    {
        fmi->read_stream_disable = TRUE32;
    }
#endif // FMI_VERSION == 4

#if FMI_VERSION >= 5
#if SUPPORT_TOGGLE_NAND
    if (fmi->is_toggle)
    {
        fmi->read_tx_page_delay = 0;
    }
    else
#endif // SUPPORT_TOGGLE_NAND
    {
        const UInt32 dccycle = FMC_IF_CTRL__GET_DCCYCLE(if_ctrl);
        const UInt32 reb_hold = FMC_IF_CTRL__GET_REB_HOLD(if_ctrl);

        // <rdar://problem/10987609> Timeout during flash data transfer in SDR mode with slow timings
        fmi->read_tx_page_delay = (reb_hold > (dccycle + 4) ? reb_hold - (dccycle + 4) : 0);
    }
#endif // FMI_VERSION >= 5

    h2fmi_wr(fmi, FMC_IF_CTRL, if_ctrl);
}

void restoreTimingRegs(h2fmi_t* fmi)
{
#if SUPPORT_TOGGLE_NAND
    if (fmi->is_toggle)
    {
        h2fmi_wr(fmi, FMC_DQS_TIMING_CTRL, fmi->dqs_ctrl);
        h2fmi_wr(fmi, FMC_TOGGLE_CTRL_1, fmi->timing_ctrl_1);
        h2fmi_wr(fmi, FMC_TOGGLE_CTRL_2, fmi->timing_ctrl_2);
        h2fmi_set_if_ctrl(fmi, fmi->toggle_if_ctrl);
    }
    else
    {    
#endif
    h2fmi_set_if_ctrl(fmi, fmi->if_ctrl);
#if SUPPORT_TOGGLE_NAND
    }
#endif
}
// ********************************** EOF **************************************
