// *****************************************************************************
//
// File: H2fmi_write.c
//
// *****************************************************************************
//
// Notes:
//
// *****************************************************************************
//
// Copyright (C) 2008-2013 Apple, Inc. All rights reserved.
//
// This document is the property of Apple, Inc.
// It is considered confidential and proprietary.
//
// This document may not be reproduced or transmitted in any form,
// in whole or in part, without the express written permission of
// Apple Computer, Inc.
//
// *****************************************************************************

#include "H2fmi_private.h"
#include "H2fmi_dma.h"

/**
 * Define the following to 'simulate' NAND programming errors.
 */
//??#define SIMULATE_WRITE_FAILURES
//??#define SIMULATE_WRITE_TIMEOUTS
//??#define SIMULATE_WRITE_LOCKUP

#if defined(SIMULATE_WRITE_TIMEOUTS)
#define WRITE_TIMEOUT_MAGIC_NUMBER ( 159871/2 )
static unsigned long ulWriteTimeoutCount = 0;
#endif

#if defined(SIMULATE_WRITE_FAILURES)
#define WRITE_FAILURE_MAGIC_NUMBER 159871
static unsigned long ulFailureCount = 0;
static unsigned long _gForcedFailure[H2FMI_MAX_NUM_BUS] = { 0, 0 };
#endif

#if defined(SIMULATE_WRITE_LOCKUP)
#define WRITE_LOCKUP_COUNT_TRIGGER 1000
static unsigned long ulWriteLockoutCount = 0;
#endif

// =============================================================================
// private implementation function declarations
// =============================================================================

static void h2fmi_start_nand_page_program(h2fmi_t* fmi, UInt32 page);
static void h2fmi_send_write_confirm(h2fmi_t* fmi);
static BOOL32 h2fmi_tx_raw_page(h2fmi_t* fmi, UInt8* data_buf, h2fmi_ce_t ce);
static BOOL32 h2fmi_tx_wait_page_done(h2fmi_t* fmi);
static BOOL32 h2fmi_tx_bootpage_pio(h2fmi_t* fmi, UInt8* data_buf, h2fmi_ce_t ce);
static void h2fmi_prepare_write_confirm(h2fmi_t* fmi);
static void h2fmi_write_Xfer_data_handler2(h2fmi_t * fmi);

/**
 * Same as h2fmi_get_current_CE_state except also tests to 
 * ensure the write protect bit is off.  Used for write 
 * mode as a sanity check as some NAND have been known to fail 
 * with 'stuck' write protects. 
 * 
 * @param fmi 
 * 
 * @return CE_STATE 
 */
static CE_STATE h2fmi_get_current_writable_CE_state( h2fmi_t* fmi )
{
    CE_STATE ceState = h2fmi_get_current_CE_state(fmi);

    const UInt8 bNandStatus = (UInt8)h2fmi_rd(fmi, FMC_NAND_STATUS);
    const UInt32 bWPDisabled = (bNandStatus & NAND_STATUS__WRITE_PROTECT);

    WMR_ASSERT(bWPDisabled);

    return ceState;
}

/**
 * Called when a general failure has been detected.  This is 
 * either a timeout (in which case the status pointer is NULL) 
 * or a valid pointer to a 'generic' status is passed in. 
 *  
 * It is up to this routine to report the correct failing CE 
 * 
 */
static void h2fmi_recordFirstFailingCE( h2fmi_t *fmi )
{
    if ( ((UInt32) ~0) == fmi->failureDetails.wFirstFailingCE )
    {
        /**
         * Only record the FIRST failing CE ...
         */
        fmi->failureDetails.wFirstFailingCE = fmi->stateMachine.currentCE;
    }
}


// =============================================================================
// public implementation function definitions
// =============================================================================

BOOL32 h2fmi_write_page(h2fmi_t* fmi,
                        UInt16   ce,
                        UInt32   page,
                        UInt8*   data_buf,
                        UInt8*   meta_buf,
                        BOOL32*  status_failed)
{
    BOOL32 result = TRUE32;
    struct dma_segment data_sgl;
    struct dma_segment meta_sgl;

    data_sgl.paddr  = (UInt32)data_buf;
    data_sgl.length = fmi->bytes_per_page;
    meta_sgl.paddr  = (UInt32)meta_buf;
    meta_sgl.length = fmi->valid_bytes_per_meta;

    result = h2fmi_write_multi(fmi, 1, &ce, &page, &data_sgl, &meta_sgl, status_failed, _kVS_NONE);

    return result;
}

BOOL32 h2fmi_write_multi(h2fmi_t* fmi,
                         UInt32   page_count,
                         h2fmi_ce_t*  chip_enable_array,
                         UInt32*  page_number_array,
                         struct dma_segment* data_segment_array,
                         struct dma_segment* meta_segment_array,
                         BOOL32*  write_failed,
                         UInt32 wVendorType)
{
    BOOL32 result = TRUE32;

#if defined(SIMULATE_WRITE_LOCKUP)
    if ( ++ulWriteLockoutCount >= WRITE_LOCKUP_COUNT_TRIGGER )
    {
        volatile int iHoldForLockup = 1;
        while ( iHoldForLockup )
        {
        }
        
    }
#endif

#if ( H2FMI_INSTRUMENT_BUS_1 )
    h2fmi_instrument_bit_clear(0);
    h2fmi_instrument_bit_clear(1);
    h2fmi_instrument_bit_clear(2);
    h2fmi_instrument_bit_clear(3);
#endif

    WMR_ASSERT(fmi->stateMachine.currentMode == fmiNone);

#if ( H2FMI_INSTRUMENT_BUS_1 )
    h2fmi_instrument_bit_set(0);
    h2fmi_instrument_bit_clear(0);
    h2fmi_instrument_bit_set(0);
    h2fmi_instrument_bit_clear(0);
    h2fmi_instrument_bit_set(0);
#endif
    const UInt32 fmc_if_ctrl = h2fmi_rd(fmi, FMC_IF_CTRL);

    WMR_ENTER_CRITICAL_SECTION();
    {
        fmi->stateMachine.chip_enable_array  = chip_enable_array;
        fmi->stateMachine.page_number_array  = page_number_array;
        fmi->stateMachine.data_segment_array = data_segment_array;
        fmi->stateMachine.meta_segment_array = meta_segment_array;
        fmi->stateMachine.state.wr   = writeIdle;
        fmi->stateMachine.page_count = page_count;
        fmi->stateMachine.wVendorType = wVendorType;

        h2fmi_reset(fmi);

        // disable write protect
        h2fmi_set_if_ctrl(fmi, FMC_IF_CTRL__WPB | fmc_if_ctrl);

        fmi->stateMachine.currentMode = fmiWrite;
    }
    WMR_EXIT_CRITICAL_SECTION();

#if ( H2FMI_INSTRUMENT_BUS_1 )
    h2fmi_instrument_bit_set(1);
#endif
#if (H2FMI_WAIT_USING_ISR)
    static const utime_t clockDelay_uSec = 50000; // every 50 mSec
#endif
    volatile WRITE_STATE* pState = &fmi->stateMachine.state.wr;
#if (H2FMI_WAIT_USING_ISR)
    event_unsignal(&fmi->stateMachine.stateMachineDoneEvent);
#endif
    do
    {
        if ( writeDone == (*pState) )
        {
            break;
        }

        WMR_ENTER_CRITICAL_SECTION();
        h2fmi_handle_write_ISR_state_machine(fmi);
        WMR_EXIT_CRITICAL_SECTION();

#if !(H2FMI_WAIT_USING_ISR)
        WMR_YIELD();
    }
    while ( writeDone != (*pState) );
#else
    }
    while ( !event_wait_timeout(&fmi->stateMachine.stateMachineDoneEvent, clockDelay_uSec) );
    /**
     * Allow for early wakeup
     */
    {
        UInt32 iLoopCount = 0;
        while ( writeDone != (*pState) )
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
                h2fmi_handle_write_ISR_state_machine(fmi);
                WMR_EXIT_CRITICAL_SECTION();
            }
        }
    }
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
         *
         * This wouldn't strictly be necessary on a write (since the DMA had to have completed
         * for the FMC operation to complete), but we need to make sure the event is consumed
         * otherwise it'll stay pending and casue the next DMA to appear to have completed
         * immediately.
         */
        // both dma streams should be complete now
        if (!h2fmi_dma_wait(h2fmi_dma_data_chan(fmi), H2FMI_PAGE_TIMEOUT_MICROS) ||
            !h2fmi_dma_wait(h2fmi_dma_meta_chan(fmi), H2FMI_PAGE_TIMEOUT_MICROS))
        {
            h2fmi_fail(result);

            // If the write operation succeeded, but we never get a CDMA completion event,
            // something is broken in the FMI or CDMA layers - no legit NAND problem should case this.
            WMR_PANIC("Timeout waiting for CDMA during successful NAND write operation");

            h2fmi_dma_cancel(h2fmi_dma_data_chan(fmi));
            h2fmi_dma_cancel(h2fmi_dma_meta_chan(fmi));
            fmi->failureDetails.wOverallOperationFailure = _kIOPFMI_STATUS_DMA_DONE_TIMEOUT;
        }
    }
    else
    {
        h2fmi_dma_cancel(h2fmi_dma_data_chan(fmi));
        h2fmi_dma_cancel(h2fmi_dma_meta_chan(fmi));
    }

#if ( H2FMI_INSTRUMENT_BUS_1 )
    h2fmi_instrument_bit_clear(1);
#endif

    // return write protect to previous state
    h2fmi_set_if_ctrl(fmi, fmc_if_ctrl);

    /**
     * Reset our current mode so that interrupts are not re-directed
     * to our state machine.
     */
    fmi->stateMachine.currentMode = fmiNone;

#if ( H2FMI_INSTRUMENT_BUS_1 )
    h2fmi_instrument_bit_clear(0);
#endif

    // presume all write operations will succeed
    if (NULL != write_failed)
    {
        *write_failed = ( result ? FALSE32 : TRUE32 );
    }

#if defined(SIMULATE_WRITE_FAILURES)
    if ( _gForcedFailure[ fmi->bus_id ] )
    {
        /**
         * Since our simulated failures may fail the dummy commit within 
         * a VS operation we need to reset our NAND back to a known good 
         * state...  We could (should?) try to be more intelligent about 
         * this but for now we just do a reset any time we generate a 
         * failure. 
         */
        h2fmi_nand_reset_all(fmi);
        _gForcedFailure[ fmi->bus_id ] = 0;
    }
#endif

    return result;
}

static BOOL32 h2fmi_tx_wait_page_done(h2fmi_t* fmi)
{
    BOOL32 result = TRUE32;

#if H2FMI_WAIT_USING_ISR
    if (!event_wait_timeout(&fmi->isr_event, H2FMI_PAGE_TIMEOUT_MICROS))
    {
        h2fmi_fail(result);
    }

    if (result && (!(fmi->isr_condition & FMI_INT_PEND__LAST_FMC_DONE)))
    {
        h2fmi_fail(result);
    }

#else
    result = h2fmi_wait_done(fmi, FMI_INT_PEND, FMI_INT_PEND__LAST_FMC_DONE, FMI_INT_PEND__LAST_FMC_DONE);
#endif

    if (!result )
    {
        h2fmi_fmc_disable_all_ces(fmi);
    }

    return result;
}


UInt32 h2fmi_write_bootpage(h2fmi_t* fmi,
                            UInt32   ce,
                            UInt32   page,
                            UInt8*   data_buf)
{
    UInt32 fmc_if_ctrl = h2fmi_rd(fmi, FMC_IF_CTRL);
    UInt8  nand_status;
    UInt32 result = _kIOPFMI_STATUS_SUCCESS;
#if FMI_VERSION > 3
    UInt32 sector;
#endif

    // Enable specified NAND device
    h2fmi_fmc_enable_ce(fmi, ce);

    // Disable write protection
    h2fmi_set_if_ctrl(fmi, FMC_IF_CTRL__WPB | fmc_if_ctrl);

    // Prep for program (all steps prior to data transfer)
    h2fmi_start_nand_page_program(fmi, page);
    h2fmi_prepare_write_confirm(fmi);
    
    h2fmi_set_bootpage_data_format(fmi);
    
#if FMI_VERSION > 3
    // Set page format first to enable seed scrambling
    h2fmi_wr(fmi, FMI_CONTROL, FMI_CONTROL__RESET_SEED);
    // Fill randomizer seeds based on page address
    for (sector = 0; sector < H2FMI_BOOT_SECTORS_PER_PAGE; sector++)
    {
        h2fmi_wr(fmi, FMI_SCRAMBLER_SEED_FIFO, page + sector);
    }
#endif

    // Transfer data for page to FMI
    if (!h2fmi_tx_bootpage_pio(fmi, data_buf, ce))
    {
        result = _kIOPFMI_STATUS_FMC_DONE_TIMEOUT;
    }
    else
    {
        // End program operation
        h2fmi_send_write_confirm(fmi);

        // wait for the program to complete
        if (!h2fmi_wait_status(fmi,
                               NAND_STATUS__DATA_CACHE_RB,
                               NAND_STATUS__DATA_CACHE_RB_READY,
                               &nand_status))
        {
            result = _kIOPFMI_STATUS_READY_BUSY_TIMEOUT;
        }
        else if ((nand_status & NAND_STATUS__CHIP_STATUS1) == NAND_STATUS__CHIP_STATUS1_FAIL)
        {
            // NAND reported Program Failed
            result = _kIOPFMI_STATUS_PGM_ERROR;
        }
    }

    // disable all FMI interrupt sources
    h2fmi_wr(fmi, FMI_INT_EN, 0);

    // restore write-protect
    h2fmi_set_if_ctrl(fmi, fmc_if_ctrl);

    // Disable NAND device
    h2fmi_fmc_disable_ce(fmi, ce);

    // Hardware still thinks there should be a meta DMA complete signal here somewhere.
    // It'll never come, so reset the core to move on.
    h2fmi_reset(fmi);

    return result;
}

static BOOL32 h2fmi_tx_bootpage_pio(h2fmi_t*   fmi,
                                    UInt8*     buf,
                                    h2fmi_ce_t ce)
{
    BOOL32 result = TRUE32;
    UInt32 i;
    UInt32 fmi_control = (FMI_CONTROL__MODE__WRITE |
                          FMI_CONTROL__START_BIT);

    for (i = 0; i < H2FMI_BOOT_SECTORS_PER_PAGE; i++)
    {      
        // Start FMI write
#if !WMR_BUILDING_EFI
        h2fmi_wr(fmi, FMI_INT_EN, FMI_INT_EN__LAST_FMC_DONE);
#endif
        h2fmi_wr(fmi, FMI_CONTROL, fmi_control);

        h2fmi_pio_write_sector(fmi, buf, H2FMI_BYTES_PER_BOOT_SECTOR);
        buf += H2FMI_BYTES_PER_BOOT_SECTOR;        

        // wait for the page to finish transferring
        if (!h2fmi_tx_wait_page_done(fmi))
        {
            h2fmi_fail(result);
            break;
        }  
    }

    return result;    
}


UInt32 h2fmi_write_raw_page(h2fmi_t* fmi,
                            UInt32   ce,
                            UInt32   page,
                            UInt8*   data_buf)
{
    UInt32 result = _kIOPFMI_STATUS_SUCCESS;
    UInt8  nand_status;
    UInt32 fmc_if_ctrl = h2fmi_rd(fmi, FMC_IF_CTRL);
    
    // If using PIO, make certain that the FMI subsystem gets reset
    // between each write to read transition; might want to do this on
    // read to write to just be safe, so I'm taking the most
    // pessimistic option and doing it before each PIO-based
    // operation).
    //
    // TODO: This workaround appears to also make dma-based write function correctly,
    // which is unexpected and should be tracked down to root cause soon.
    h2fmi_reset(fmi);

    // enable specified nand device
    h2fmi_fmc_enable_ce(fmi, ce);

    // disable write protect
    h2fmi_set_if_ctrl(fmi, FMC_IF_CTRL__WPB | fmc_if_ctrl);

    h2fmi_wr(fmi, ECC_MASK, 0 );

    // prep for program (all the step prior to xfer).
    h2fmi_start_nand_page_program(fmi, page);
    h2fmi_prepare_write_confirm(fmi);
    
    // transfer data for page and meta to FMI
    if (!h2fmi_tx_raw_page(fmi, data_buf, ce))
    {
        result = _kIOPFMI_STATUS_DMA_DONE_TIMEOUT;
        h2fmi_fail(result);
    }
    else
    {
        // end program operation
        h2fmi_send_write_confirm(fmi);

        // wait for the device to come back
        if (!h2fmi_wait_status(fmi,
                               NAND_STATUS__DATA_CACHE_RB,
                               NAND_STATUS__DATA_CACHE_RB_READY,
                               &nand_status))
        {
            result = _kIOPFMI_STATUS_READY_BUSY_TIMEOUT;
        }
        else if ((nand_status & NAND_STATUS__CHIP_STATUS1) == NAND_STATUS__CHIP_STATUS1_FAIL)
        {
            result = _kIOPFMI_STATUS_PGM_ERROR;
        }
    }

    // disable all FMI interrupt sources
    h2fmi_wr(fmi, FMI_INT_EN, 0);

    // return write protect to previous state
    h2fmi_set_if_ctrl(fmi, fmc_if_ctrl);

    // disable all nand devices
    h2fmi_fmc_disable_ce(fmi, ce);

    if( result != _kIOPFMI_STATUS_SUCCESS)
    {
        h2fmi_fmc_disable_all_ces(fmi);
    }

    return result;
}

static BOOL32 h2fmi_tx_raw_page(h2fmi_t*   fmi,
                                UInt8*     buf,
                                h2fmi_ce_t ce)
{
    BOOL32 result = TRUE32;
    UInt32 i;
    UInt32 spare  = fmi->bytes_per_spare;
    UInt32 fmi_control = (FMI_CONTROL__MODE__WRITE |
                          FMI_CONTROL__START_BIT);

    for( i = 0; i < fmi->sectors_per_page; i++ )
    {
        const UInt32 spare_per_sector = WMR_MIN(spare, H2FMI_MAX_META_PER_ENVELOPE);

        h2fmi_set_raw_write_data_format(fmi, spare_per_sector);

        // start FMI write
        h2fmi_wr(fmi, FMI_INT_EN, FMI_INT_EN__LAST_FMC_DONE);

        h2fmi_wr(fmi, FMI_CONTROL, fmi_control);

        h2fmi_dma_execute_cmd(DMA_CMD_DIR_TX,
                         h2fmi_dma_data_chan(fmi),
                         buf,
                         h2fmi_dma_data_fifo(fmi),
                         H2FMI_BYTES_PER_SECTOR,
                         sizeof(UInt32),
                         1,
                         NULL);

        buf += H2FMI_BYTES_PER_SECTOR;

        if( spare_per_sector )
        {
            h2fmi_dma_execute_cmd(DMA_CMD_DIR_TX,
                             h2fmi_dma_meta_chan(fmi),
                             buf,
                             h2fmi_dma_meta_fifo(fmi),
                             spare_per_sector,
                             sizeof(UInt8),
                             1,
                             NULL);

            buf   += spare_per_sector;
            spare -= spare_per_sector;
        }

        // wait for the page to finish transferring
        if ( !h2fmi_tx_wait_page_done(fmi) )
        {
            h2fmi_fail(result);
            break;
        }
    }

    if (!result)
    {
        h2fmi_fmc_disable_all_ces(fmi);
    }

    return result;
}

// =============================================================================
// private implementation function definitions
// =============================================================================

static void h2fmi_start_nand_page_program(h2fmi_t* fmi,
                                          UInt32   page)
{
    UInt32 wCmd1;

    const UInt32 cmd1_addr_done = (FMC_STATUS__CMD1DONE |
                                   FMC_STATUS__ADDRESSDONE);

    // configure FMC for cmd1/addr sequence
    h2fmi_config_page_addr(fmi, page);

    if ( fmiNone == fmi->stateMachine.currentMode )
    {
            wCmd1 = NAND_CMD__WRITE;
    }
    else
    {
        switch ( fmi->stateMachine.wVendorType )
        {
            case _kVS_SAMSUNG_2P_2D:
                wCmd1 = ( 0==(fmi->stateMachine.page_idx & 1) ? NAND_CMD__WRITE : NAND_CMD__WRITE2_SAMSUNG  );
                break;
            case _kVS_HYNIX_2P:
                wCmd1 = ( 0==(fmi->stateMachine.page_idx & 1) ? NAND_CMD__WRITE : NAND_CMD__WRITE2_HYNIX  );
                break;
            case _kVS_TOSHIBA_2P:
            case _kVS_MICRON_2P:
            case _kVS_SAMSUNG_2D:
            default:
                wCmd1 = NAND_CMD__WRITE;
                break;
        }
    }
    h2fmi_wr(fmi, FMC_CMD, FMC_CMD__CMD1(wCmd1));

    // start cmd1/addr/cmd2 sequence
    h2fmi_wr(fmi, FMC_RW_CTRL, (FMC_RW_CTRL__CMD1_MODE |
                                FMC_RW_CTRL__ADDR_MODE));
    
    // busy wait until cmd1/addr/cmd2 sequence completed
    h2fmi_busy_wait(fmi, FMC_STATUS, cmd1_addr_done, cmd1_addr_done);
    h2fmi_wr(fmi, FMC_STATUS, cmd1_addr_done);    
}

static void h2fmi_incrementOutstandingOperations(h2fmi_t* fmi)
{
    if ( (_kVS_NONE == fmi->stateMachine.wVendorType ) || ( 1 == ( fmi->stateMachine.page_idx & 1 ) ) )
    {
        const UInt32 maxQueueElements = sizeof(fmi->stateMachine.ceCommitQueue.previousCEIndex)/sizeof(fmi->stateMachine.ceCommitQueue.previousCEIndex[0]);

        /**
         * Only increment count on "terminal" commits -- we assume here 
         * that VS will only be 2P or 2D, not 2D+2P. 
         */
        fmi->stateMachine.wOutstandingCEWriteOperations++;
        /**
         * Save the working index in case we need it for holdoff 
         * processing. 
         */
        fmi->stateMachine.ceCommitQueue.previousCEIndex[ fmi->stateMachine.ceCommitQueue.head++ ] = fmi->stateMachine.page_idx;
        if ( fmi->stateMachine.ceCommitQueue.head >= maxQueueElements )
        {
            fmi->stateMachine.ceCommitQueue.head = 0;
        }
        if ( fmi->stateMachine.ceCommitQueue.count < maxQueueElements )
        {
            fmi->stateMachine.ceCommitQueue.count++;
        }
    }
}

static void h2fmi_decrementOutstandingOperations(h2fmi_t* fmi)
{

    const UInt32 lastIndex = fmi->stateMachine.lastPageIndex[ fmi->stateMachine.currentCE ];

    if ( (_kVS_NONE == fmi->stateMachine.wVendorType ) || ( 0 == ( (lastIndex+1) & 1 ) ) )
    {
        /**
         * Only decrement count on "terminal" waits -- we assume here 
         * that VS will only be 2P or 2D, not 2D+2P. 
         */
        fmi->stateMachine.wOutstandingCEWriteOperations--;
    }
    WMR_ASSERT( ( (Int32)fmi->stateMachine.wOutstandingCEWriteOperations ) >= 0 );
}

static void h2fmi_send_write_confirm(h2fmi_t* fmi)
{
    // CMD1 should have been filled out already in h2fmi_prep_program_confirm().
    // start cmd1
    const UInt32 cmd2_done = FMC_STATUS__CMD2DONE;
    
    h2fmi_wr(fmi, FMC_RW_CTRL, FMC_RW_CTRL__CMD2_MODE);
    
    // wait until cmd1 sequence completed
    h2fmi_busy_wait(fmi, FMC_STATUS, cmd2_done, cmd2_done);
    h2fmi_wr(fmi, FMC_STATUS, cmd2_done);
}

// =============================================================================
// dma-only private implementation function definitions
// =============================================================================

BOOL32 h2fmi_do_write_setup( h2fmi_t* fmi )
{
    BOOL32 fContinue = TRUE32;
    fmi->stateMachine.currentCE = fmi->stateMachine.chip_enable_array[ fmi->stateMachine.page_idx ];

    // enable specified nand device
    h2fmi_fmc_enable_ce(fmi, fmi->stateMachine.currentCE);

    if (fmi->stateMachine.dirty_ce_mask & (0x1U << fmi->stateMachine.currentCE))
    {
        // We must wait for the device to become ready as we have already started a write to it from
        // a previous pass.
        h2fmi_prepare_for_ready_busy_interrupt(fmi);

        fContinue = FALSE32;
    }

    return fContinue ;
}

void h2fmi_ISR_state_machine_start_page( h2fmi_t* fmi )
{
    UInt32 fmi_control = (FMI_CONTROL__MODE__WRITE |
                          FMI_CONTROL__START_BIT);

    fmi->stateMachine.currentPage    = fmi->stateMachine.page_number_array[ fmi->stateMachine.page_idx ];

    // mark the current chip enable dirty so that it will be
    // checked for ready before next use
    fmi->stateMachine.dirty_ce_mask |= (0x1UL << fmi->stateMachine.currentCE);

    // prep for program (all the step prior to xfer).
    h2fmi_start_nand_page_program(fmi, fmi->stateMachine.currentPage);

    // enable appropriate interrupt source
    h2fmi_wr(fmi, FMI_INT_EN, FMI_INT_EN__LAST_FMC_DONE);

    // start FMI write of next page
    h2fmi_wr(fmi, FMI_CONTROL, fmi_control);


    // Mark when we started out wait ...
    fmi->stateMachine.waitStartTicks = WMR_CLOCK_TICKS();
    // Increment the # of pages 'started' across the bus ...
    fmi->failureDetails.wNumCE_Executed++;

    h2fmi_prepare_write_confirm(fmi);
}

static void h2fmi_prepare_write_confirm(h2fmi_t *fmi)
{
    UInt32 wCmd2;
    
    if ( fmiNone == fmi->stateMachine.currentMode )
    {
        wCmd2 = NAND_CMD__WRITE_CONFIRM;
    }
    else
    {
        switch ( fmi->stateMachine.wVendorType )
        {
            case _kVS_HYNIX_2P:
                wCmd2 = ( 0 == (fmi->stateMachine.page_idx & 1) ? NAND_CMD__DUMMY_CONFIRM_HYNIX : NAND_CMD__WRITE_CONFIRM );
                break;
            case _kVS_MICRON_2P:
                wCmd2 = ( 0 == (fmi->stateMachine.page_idx & 1) ? NAND_CMD__DUMMY_CONFIRM_MICRON : NAND_CMD__WRITE_CONFIRM );
                break;
            case _kVS_SAMSUNG_2P_2D:
                wCmd2 = ( 0 == (fmi->stateMachine.page_idx & 1) ? NAND_CMD__DUMMY_CONFIRM_SAMSUNG : NAND_CMD__WRITE_CONFIRM );
                break;
            case _kVS_TOSHIBA_2P:
                wCmd2 = ( 0 == (fmi->stateMachine.page_idx & 1) ? NAND_CMD__DUMMY_CONFIRM_TOSHIBA : NAND_CMD__WRITE_CONFIRM );
                break;
            default:
                wCmd2 = NAND_CMD__WRITE_CONFIRM;
                break;
        }
    }
    h2fmi_wr(fmi, FMC_CMD, FMC_CMD__CMD2(wCmd2));
}


static void h2fmi_write_done_handler( h2fmi_t* fmi )
{
    h2fmi_clear_interrupts_and_reset_masks(fmi);
    h2fmi_fmc_disable_all_ces( fmi );
#if (H2FMI_WAIT_USING_ISR)
    event_signal( &fmi->stateMachine.stateMachineDoneEvent );
#endif
}

static void h2fmi_write_ending_write_handler( h2fmi_t* fmi );

static void h2fmi_write_wait_for_CE_handlerImpl( h2fmi_t* fmi, const BOOL32 fContinue )
{
    CE_STATE ceState = h2fmi_get_current_writable_CE_state(fmi);
#if defined(SIMULATE_WRITE_TIMEOUTS)
    if ( ++ulWriteTimeoutCount > WRITE_TIMEOUT_MAGIC_NUMBER )
    {
        ulWriteTimeoutCount = 0;
        ceState = CE_TIMEOUT;
    }
#endif
    if ( CE_IDLE == ceState )
    {
        h2fmi_clear_interrupts_and_reset_masks(fmi);

        const UInt8 bNandStatus = (UInt8)h2fmi_rd(fmi, FMC_NAND_STATUS);
        UInt32 status1;
        BOOL32 fSuccess;

        switch ( fmi->stateMachine.wVendorType )
        {
            case _kVS_TOSHIBA_2P:
                status1 = (bNandStatus & NAND_STATUS__TOS_CHIP_STATUS_MASK);
                break;
            default:
                status1 = (bNandStatus & NAND_STATUS__CHIP_STATUS1);
                break;
        }
        
        // clear FMC_RW_CTRL, thereby releasing REBHOLD and completing
        // current cycle
        h2fmi_wr(fmi, FMC_RW_CTRL, 0);

#if defined(SIMULATE_WRITE_FAILURES)
        if ( (ulFailureCount++)>=WRITE_FAILURE_MAGIC_NUMBER )
        {
            ulFailureCount = 0;
            status1 = ~NAND_STATUS__CHIP_STATUS1_PASS;
            WMR_PRINT(ALWAYS,"Simulated write failure on ce %d, index: %d\n",
                      fmi->stateMachine.currentCE, fmi->stateMachine.page_idx);
            _gForcedFailure[ fmi->bus_id ] = 1;
        }
#endif

        switch ( fmi->stateMachine.wVendorType )
        {
            default:
                fSuccess = ( NAND_STATUS__CHIP_STATUS1_PASS == status1 );
                break;
        }
        
        fmi->stateMachine.dirty_ce_mask &= ~(0x1U << fmi->stateMachine.currentCE);
        if ( !fSuccess )
        {
            // Failed our write -- indicate such ...
            fmi->stateMachine.fSuccessful = FALSE32;
            // and now go to the exiting handler to wait for dirty CE's to complete.
            fmi->stateMachine.state.wr = writeEndingWrite;
            h2fmi_recordFirstFailingCE(fmi);
#if (H2FMI_WAIT_USING_ISR)
            event_signal( &fmi->stateMachine.stateMachineDoneEvent );
#endif
            h2fmi_write_ending_write_handler( fmi );
        }
        else
        {
            fmi->stateMachine.state.wr = writeXferData;
            if ( fContinue )
            {
                h2fmi_ISR_state_machine_start_page(fmi);
            }
        }
        h2fmi_decrementOutstandingOperations( fmi );
        fmi->stateMachine.lastPageIndex[ fmi->stateMachine.currentCE ] = INVALID_CE_INDEX;
    }
    else if ( CE_TIMEOUT == ceState )
    {
        // We failed to get what we wanted -- signal error state and move to ending write stage to wait
        // for additional CE's to complete.  Also clear this CE to indicate that we are no longer waiting on
        // it as we timed out.
        fmi->stateMachine.fSuccessful = FALSE32;
        fmi->stateMachine.state.wr = writeEndingWrite;
        fmi->stateMachine.dirty_ce_mask &= ~(0x1U << fmi->stateMachine.currentCE);
        h2fmi_recordFirstFailingCE(fmi);
        h2fmi_decrementOutstandingOperations( fmi );
        fmi->stateMachine.lastPageIndex[ fmi->stateMachine.currentCE ] = INVALID_CE_INDEX;
#if (H2FMI_WAIT_USING_ISR)
        event_signal( &fmi->stateMachine.stateMachineDoneEvent );
#endif
        h2fmi_write_ending_write_handler(fmi);
    }
}

static void h2fmi_write_wait_for_CE_handler( h2fmi_t* fmi )
{
    h2fmi_write_wait_for_CE_handlerImpl(fmi,TRUE32);
}

static void h2fmi_write_wait_for_CE_holdOff_handler( h2fmi_t* fmi )
{
    fmi->stateMachine.wNumTimesHoldoffExecuted++;

    h2fmi_write_wait_for_CE_handlerImpl(fmi,FALSE32);
    if ( writeXferData == fmi->stateMachine.state.wr )
    {
        /**
         * Return the HW to our current CE...
         */
        h2fmi_fmc_enable_ce(fmi, fmi->stateMachine.savedCurrentCE);
        h2fmi_prepare_write_confirm(fmi);
        fmi->stateMachine.currentCE = fmi->stateMachine.savedCurrentCE;
        h2fmi_write_Xfer_data_handler2(fmi);
    }
}


static void h2fmi_write_ending_write_wait_CE_handler( h2fmi_t* fmi )
{
    CE_STATE ceState = h2fmi_get_current_writable_CE_state(fmi);
    if ( CE_IDLE == ceState )
    {
        h2fmi_clear_interrupts_and_reset_masks(fmi);

        const UInt8 bNandStatus = (UInt8)h2fmi_rd(fmi, FMC_NAND_STATUS);
        const UInt32 status1 = (bNandStatus & NAND_STATUS__CHIP_STATUS1);

        if (NAND_STATUS__CHIP_STATUS1_PASS != status1)
        {
            fmi->stateMachine.fSuccessful = FALSE32;
            // We will still wait for Ready/Busy CE's to complete ...
            h2fmi_recordFirstFailingCE(fmi);
        }

        // clear FMC_RW_CTRL, thereby releasing REBHOLD and completing
        // current cycle
        h2fmi_wr(fmi, FMC_RW_CTRL, 0);
        UInt32 wCEIndex = 0;
        // Find the CE that was dirty ... (we only enter this state when dirty_ce_mask is non-zero)
        while ( 0 == (fmi->stateMachine.dirty_ce_mask & (1 << wCEIndex)) )
        {
            wCEIndex++;
        }
        switch ( fmi->stateMachine.wVendorType )
        {
            case _kVS_SAMSUNG_2D:
            case _kVS_SAMSUNG_2P_2D:
                if ( 0 == fmi->stateMachine.currentWriteDie )
                {
                    fmi->stateMachine.currentWriteDie++;
                }
                else
                {
                    fmi->stateMachine.currentWriteDie = 0;
                    fmi->stateMachine.dirty_ce_mask &= ~(1 << wCEIndex);
                }
                fmi->stateMachine.page_idx++;
                break;
            default:
                fmi->stateMachine.dirty_ce_mask &= ~(1 << wCEIndex);
                break;
        }
        
        fmi->stateMachine.currentCE = wCEIndex;
        fmi->stateMachine.state.wr = writeEndingWrite;
        h2fmi_write_ending_write_handler(fmi);
    }
    else if ( CE_TIMEOUT == ceState )
    {
        // We failed to get what we wanted -- signal error state but continue to wait for any
        // other CE's that have not been completed.
        h2fmi_recordFirstFailingCE(fmi);
        fmi->stateMachine.fSuccessful = FALSE32;
        fmi->stateMachine.dirty_ce_mask &= ~(0x1U << fmi->stateMachine.currentCE);
        fmi->stateMachine.state.wr = writeEndingWrite;
        h2fmi_write_ending_write_handler(fmi);
    }
}

static void h2fmi_write_ending_write_handler( h2fmi_t* fmi )
{
    if (0 == fmi->stateMachine.dirty_ce_mask)
    {
        /**
         * We're done waiting -- indicate that we're done and break.
         */
        h2fmi_fmc_disable_all_ces(fmi);
        fmi->stateMachine.state.wr = writeDone;
    }
    else
    {
        UInt32 wCEIndex = 0;
        // Find the CE that is dirty ... (we only enter this state when dirty_ce_mask is non-zero)
        while ( 0 == (fmi->stateMachine.dirty_ce_mask & (1 << wCEIndex)) )
        {
            wCEIndex++;
        }
        h2fmi_fmc_enable_ce(fmi, wCEIndex);
        fmi->stateMachine.currentCE = wCEIndex;
        switch ( fmi->stateMachine.wVendorType )
        {
            case _kVS_SAMSUNG_2P_2D:
            case _kVS_SAMSUNG_2D:
                fmi->stateMachine.page_idx++;
                break;
            default:
                break;
        }
        
        h2fmi_prepare_for_ready_busy_interrupt(fmi);

        fmi->stateMachine.waitStartTicks = WMR_CLOCK_TICKS();
        fmi->stateMachine.state.wr = writeEndingWriteWaitCE;
        // *should* execute writeEndingWriteWaitCE state.
    }
}


static void h2fmi_write_Xfer_data_handler2(h2fmi_t * fmi)
{
    const BOOL32 contExec = (0 == fmi->wMaxOutstandingCEWriteOperations) 
                            || ( fmi->stateMachine.wOutstandingCEWriteOperations < fmi->stateMachine.wMaxOutstandingCEWriteOperations )
                            || ( (_kVS_NONE != fmi->stateMachine.wVendorType) && ( 0 == ( fmi->stateMachine.page_idx & 1 ) ) );

    if ( contExec )
    {
        h2fmi_send_write_confirm(fmi);
        h2fmi_incrementOutstandingOperations(fmi);
        WMR_ASSERT( fmi->stateMachine.currentCE == fmi->stateMachine.chip_enable_array[ fmi->stateMachine.page_idx ] );
        WMR_ASSERT( fmi->stateMachine.currentCE < ( sizeof(fmi->stateMachine.lastPageIndex) / sizeof(fmi->stateMachine.lastPageIndex[0]) ) );
        fmi->stateMachine.lastPageIndex[ fmi->stateMachine.currentCE ] = fmi->stateMachine.page_idx++;
        
        if ( fmi->stateMachine.page_idx < fmi->stateMachine.page_count )
        {
            /**
             * More to do -- loop back and do this again!
             */
            if (!h2fmi_do_write_setup(fmi))
            {
                /**
                 * We have to wait for a dirty CE -- our state machine will
                 * continue to cycle thru in the writeWaitForCE state below when
                 * it gets called from the FMC_NSRBBDONE interrupt.
                 */
                fmi->stateMachine.state.wr = writeWaitForCE;
                fmi->stateMachine.waitStartTicks = WMR_CLOCK_TICKS();
            }
            else
            {
                /**
                 * else start a page xfer and break, waiting for our
                 * LAST_FMC_DONE interrupt again in this state.
                 */
                h2fmi_ISR_state_machine_start_page(fmi);
            }
        }
        else
        {
            /**
             * We need to finish up waiting on any and all dirty CE's ...
             * signal our main routine that we are finishing up so that it
             * is (hopefully) ready by the time we really complete.
             *
             * Fall through to endingWrite stage.
             */
            fmi->stateMachine.state.wr = writeEndingWrite;
#if (H2FMI_WAIT_USING_ISR)
            event_signal( &fmi->stateMachine.stateMachineDoneEvent );
#endif
            h2fmi_write_ending_write_handler(fmi);
        }
    }
    else
    {
        const UInt32 maxQueueElements = sizeof(fmi->stateMachine.ceCommitQueue.previousCEIndex)/sizeof(fmi->stateMachine.ceCommitQueue.previousCEIndex[0]);
        /**
         * We need to hang out and wait on older CE's until enough 
         * finish to allow us to continue. 
         */
        fmi->stateMachine.savedCurrentCE = fmi->stateMachine.currentCE;
        WMR_ASSERT( fmi->stateMachine.ceCommitQueue.count > 0 );

        fmi->stateMachine.currentCE = fmi->stateMachine.chip_enable_array[ fmi->stateMachine.ceCommitQueue.previousCEIndex[ fmi->stateMachine.ceCommitQueue.tail++ ] ];
        if ( fmi->stateMachine.ceCommitQueue.tail >= maxQueueElements )
        {
            fmi->stateMachine.ceCommitQueue.tail = 0;
        }
        fmi->stateMachine.ceCommitQueue.count--;

        /**
         * Enable oldest CE and set up to wait for status interrupt.
         */
        h2fmi_fmc_enable_ce(fmi, fmi->stateMachine.currentCE);

        h2fmi_prepare_for_ready_busy_interrupt(fmi);
        fmi->stateMachine.state.wr = writeWaitingForNumCELimit;
        fmi->stateMachine.waitStartTicks = WMR_CLOCK_TICKS();
    }
}

static void h2fmi_write_Xfer_data_handler( h2fmi_t* fmi )
{
    fmi->isr_condition = h2fmi_rd(fmi, FMI_INT_PEND);
    if (FMI_INT_PEND__LAST_FMC_DONE != (fmi->isr_condition & FMI_INT_PEND__LAST_FMC_DONE))
    {
        BOOL32 fTimeout = WMR_HAS_TIME_ELAPSED_TICKS(fmi->stateMachine.waitStartTicks, fmi->stateMachine.wPageTimeoutTicks);

        if ( fTimeout )
        {
            fmi->stateMachine.fSuccessful = FALSE32;
            // We failed to get what we wanted -- goto error state and complete.
            fmi->stateMachine.state.wr = writeEndingWrite;
            h2fmi_write_ending_write_handler(fmi);
        }
    }
    else
    {
        h2fmi_clear_interrupts_and_reset_masks(fmi);
        h2fmi_write_Xfer_data_handler2(fmi);
    }
}

static void h2fmi_write_setup_handler( h2fmi_t* fmi )
{
    if (!h2fmi_do_write_setup(fmi))
    {
        /**
         * We have to wait for a dirty CE -- break so that our state
         * machine continues to cycle thru in the writeWaitForCE state
         * below.
         */
        fmi->stateMachine.state.wr = writeWaitForCE;
        fmi->stateMachine.waitStartTicks = WMR_CLOCK_TICKS();
    }
    else
    {
        /**
         * else start a page xfer and fall through to the xfer data
         * stage below.
         */
        h2fmi_ISR_state_machine_start_page(fmi);
        fmi->stateMachine.state.wr = writeXferData;
        h2fmi_write_Xfer_data_handler(fmi);
    }
}

static void h2fmi_write_idle_handler( h2fmi_t* fmi )
{
    UInt32 Idx = 0;

    fmi->stateMachine.dirty_ce_mask = 0;
    fmi->stateMachine.currentWriteDie = 0;
    fmi->stateMachine.page_idx = 0;
    fmi->stateMachine.fSuccessful   = TRUE32;
    fmi->stateMachine.wNumTimesHoldoffExecuted = 0;
    fmi->stateMachine.wOutstandingCEWriteOperations = 0;
    fmi->stateMachine.wMaxOutstandingCEWriteOperations = ( 0 == fmi->stateMachine.wVendorType ? 
                                                           fmi->wMaxOutstandingCEWriteOperations : 1*fmi->wMaxOutstandingCEWriteOperations );

    for(Idx = 0; Idx < H2FMI_MAX_CE_TOTAL; Idx++)
    {
        fmi->stateMachine.lastPageIndex[Idx] = INVALID_CE_INDEX;
    }

    fmi->stateMachine.ceCommitQueue.count = 0;
    fmi->stateMachine.ceCommitQueue.head = 0;
    fmi->stateMachine.ceCommitQueue.tail = 0;

    BOOL32 bSuccess = h2fmi_common_idle_handler(fmi);
    WMR_ASSERT(bSuccess == TRUE32);

    // Enter write mode so DMA transfer can start early
#if FMI_VERSION >= 3
    h2fmi_wr(fmi, FMI_CONTROL, FMI_CONTROL__MODE__WRITE);
#endif // FMI_VERSION >= 3

    bSuccess = h2fmi_start_dma(fmi);
    WMR_ASSERT(bSuccess == TRUE32);

    fmi->stateMachine.state.wr = writeSetup;
    h2fmi_write_setup_handler(fmi);
}

typedef void (*h2fmi_isrWriteStateHandler)( h2fmi_t* fmi );

#if (!H2FMI_READONLY)
static const h2fmi_isrWriteStateHandler writeStateHandlerTable[] = {
    h2fmi_write_idle_handler,                 //  writeIdle = 0,
    h2fmi_write_setup_handler,                //  writeSetup,
    h2fmi_write_wait_for_CE_handler,            //  writeWaitForCE,
    h2fmi_write_Xfer_data_handler,             //  writeXferData,
    h2fmi_write_ending_write_handler,          //  writeEndingWrite,
    h2fmi_write_ending_write_wait_CE_handler,    //  writeEndingWriteWaitCE,
    h2fmi_write_done_handler,                  //  writeDone
    h2fmi_write_wait_for_CE_holdOff_handler     // writeWaitingForNumCELimit
};

void h2fmi_handle_write_ISR_state_machine( h2fmi_t* fmi )
{
    WMR_ASSERT(fmi->stateMachine.currentMode == fmiWrite);
    WMR_ASSERT(fmi->stateMachine.state.wr <= writeWaitingForNumCELimit);

#if ( H2FMI_INSTRUMENT_BUS_1 )
    {
        int k = (int)fmi->stateMachine.state.wr;
        h2fmi_instrument_bit_set(2);
        while (k-- > 0)
        {
            h2fmi_instrument_bit_clear(2);
            h2fmi_instrument_bit_set(2);
        }
    }
#endif

    writeStateHandlerTable[ fmi->stateMachine.state.wr ]( fmi );

#if ( H2FMI_INSTRUMENT_BUS_1 )
    h2fmi_instrument_bit_clear(2);
#endif
}
#else
void h2fmi_handle_write_ISR_state_machine( h2fmi_t* fmi )
{
    // Someone's trying to write in a read-only config...
    WMR_ASSERT(0);
}
#endif /* !H2FMI_READONLY */


// ********************************** EOF **************************************
