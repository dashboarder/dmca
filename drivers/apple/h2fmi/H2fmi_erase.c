// *****************************************************************************
//
// File: H2fmi_erase.c
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

/**
 * Define the following to simulate erase status failures.
 */
//??#define SIMULATE_ERASE_FAILURES

#if defined(SIMULATE_ERASE_FAILURES)
#define ERASE_FAILURE_MAGIC_NUMBER  1301
static unsigned long ulFailureCount = 0;
#endif

// =============================================================================
// private implementation function declarations
// =============================================================================

static void h2fmi_start_nand_block_erase(h2fmi_t* fmi, UInt32 page);
static void h2fmi_config_erase_page_addr(h2fmi_t* fmi, UInt32 page);

/**
 * Same as h2fmi_wait_status except also tests to ensure 
 * the write protect bit is off.  Used for erase mode as a 
 * sanity check as some NAND have been known to fail with 
 * 'stuck' write protects. 
 * 
 * @param fmi 
 * 
 * @return CE_STATE 
 */
static BOOL32 h2fmi_wait_erase_status(h2fmi_t* fmi,
                         UInt8    io_mask,
                         UInt8    io_cond,
                         UInt8*   status)
{
    UInt8 bStatus;
    BOOL32 bResult = h2fmi_wait_status(fmi,io_mask,io_cond,&bStatus);

    if ( bResult )
    {
        const UInt32 bWPDisabled = (bStatus & NAND_STATUS__WRITE_PROTECT);
    
        WMR_ASSERT(bWPDisabled);
    }

    if ( NULL!=status )
    {
        *status = bStatus;
    }

    return bResult;
}


// =============================================================================
// public interface function definitions
// =============================================================================

BOOL32 h2fmi_erase_blocks(h2fmi_t* fmi,
                         UInt32   num_elements,
                         UInt16*  ce,
                         UInt32*  block,
                         BOOL32*  status_failed)
{
    enum {
        st_IDLE,
        st_ERASING,
        st_TIMEOUT,
        st_ERROR
    } ceState[ H2FMI_MAX_CE_TOTAL ];
    UInt8 nand_status;
    UInt32 k;
    UInt32 wIndex;
    BOOL32 result = TRUE32;

    for ( k=0; k<( sizeof(ceState)/sizeof(ceState[0]) ); k++ )
    {
        ceState[k] = st_IDLE;
    }
    
    // remember initial FMC_IF_CTRL setting
    const UInt32 fmc_if_ctrl = h2fmi_rd(fmi, FMC_IF_CTRL);

    // disable write protect
    h2fmi_set_if_ctrl(fmi, FMC_IF_CTRL__WPB | fmc_if_ctrl);

    fmi->failureDetails.wNumCE_Executed = fmi->failureDetails.wOverallOperationFailure = 0;
    fmi->failureDetails.wFirstFailingCE = ~0;
    for ( wIndex=0; wIndex<num_elements; )
    {
        const UInt32 wCurrentCE = ce[wIndex];
        // convert block address to page address

        if ( st_ERASING==ceState[wCurrentCE] )
        {
            /**
             * Wrapped around -- must wait for this to finish before 
             * continuing. :(
             */
            // enable specified nand device
            h2fmi_fmc_enable_ce(fmi, wCurrentCE);

            // Increment wIndex -- we're done with this one whether it errors out or not.
            wIndex++;

            // wait for the device to indicate ready
            if (!h2fmi_wait_erase_status(fmi,
                                   NAND_STATUS__DATA_CACHE_RB,
                                   NAND_STATUS__DATA_CACHE_RB_READY,
                                   &nand_status))
            {
                ceState[wCurrentCE] = st_TIMEOUT;
                h2fmi_fail(result);
                fmi->failureDetails.wFirstFailingCE = wCurrentCE;
                /**
                 * Early failure -- break out of this loop as soon as we see a 
                 * failure occur. 
                 */
                break;
            }
            else
            {
                const UInt32 status1 = (nand_status & NAND_STATUS__CHIP_STATUS1);
                
                ceState[wCurrentCE] = ( 0==status1 ? st_IDLE : st_ERROR );
#if defined(SIMULATE_ERASE_FAILURES)
                if ( (ulFailureCount++)>=ERASE_FAILURE_MAGIC_NUMBER )
                {
                    ulFailureCount = 0;
                    ceState[wCurrentCE] = st_ERROR;
                    WMR_PRINT(ALWAYS,"Simulated erase failure on ce %d\n",wCurrentCE);
                }
#endif
                if ( st_ERROR==ceState[wCurrentCE] )
                {
                    h2fmi_fail(result);
                    fmi->failureDetails.wFirstFailingCE = wCurrentCE;
                    break;
                }
            }
        }
        else
        {
            /**
             * Kick off as many as we can -- we know the first one will work 
             * given the above test (if it were in an error state we would 
             * have already exited the top-level for loop). 
             */
            for ( k=wIndex; k<num_elements; k++ )
            {
                const UInt32 wThisCE = ce[k];
                if ( st_IDLE!=ceState[wThisCE] )
                {
                    /**
                     * Break out of this loop as soon as we hit a non-idle CE.
                     * This will cause us to go back and wait on the most recent
                     * wIndex one above, as it will not be in the st_IDLE state.
                     */
                    break;
                }
                else
                {
                    const UInt32 wThisPage = block[k] * fmi->pages_per_block;
    
                    // enable specified nand device
                    h2fmi_fmc_enable_ce(fmi, wThisCE);
                
                    // start nand erase operation
                    h2fmi_start_nand_block_erase(fmi, wThisPage);
                
                    ceState[wThisCE] = st_ERASING;
                    fmi->failureDetails.wNumCE_Executed++;
                }
            }
        }
    }

    /**
     * We're out of the above loop -- for better or for worse. 
     * Ensure we wait until all CE's have completed. 
     */
    for ( k=wIndex; k<num_elements; k++ )
    {
        const UInt32 wThisCE = ce[k];
        if ( st_ERASING==ceState[wThisCE] )
        {
            // enable specified nand device
            h2fmi_fmc_enable_ce(fmi, wThisCE);
            // wait for the device to indicate ready
            if (!h2fmi_wait_erase_status(fmi,
                                   NAND_STATUS__DATA_CACHE_RB,
                                   NAND_STATUS__DATA_CACHE_RB_READY,
                                   &nand_status))
            {
                ceState[wThisCE] = st_TIMEOUT;
                h2fmi_fail(result);
                if ( (UInt32)~0 == fmi->failureDetails.wFirstFailingCE )
                {
                    fmi->failureDetails.wFirstFailingCE = wThisCE;
                }
            }
            else
            {
                const UInt32 status1 = (nand_status & NAND_STATUS__CHIP_STATUS1);
                ceState[wThisCE] = ( 0==status1 ? st_IDLE : st_ERROR );
#if defined(SIMULATE_ERASE_FAILURES)
                if ( (ulFailureCount++)>=ERASE_FAILURE_MAGIC_NUMBER )
                {
                    ulFailureCount = 0;
                    ceState[wThisCE] = st_ERROR;
                    WMR_PRINT(ALWAYS,"Simulated erase failure on ce %d\n",wThisCE);
                }
#endif
                if ( st_ERROR==ceState[wThisCE] )
                {
                    h2fmi_fail(result);
                    if ( (UInt32)~0 == fmi->failureDetails.wFirstFailingCE )
                    {
                        fmi->failureDetails.wFirstFailingCE = wThisCE;
                    }
                }
            }
        }
    }
    
    
    // return write protect to previous state
    h2fmi_set_if_ctrl(fmi, fmc_if_ctrl);

    // disable all nand devices
    h2fmi_fmc_disable_all_ces(fmi);

    // possibly return by reference whether Read Status indicated failure
    if (NULL != status_failed)
    {
        *status_failed = ( TRUE32==result ? FALSE32 : TRUE32 );
    }

    return result;
}

// =============================================================================
// private implementation function definitions
// =============================================================================

static void h2fmi_start_nand_block_erase(h2fmi_t* fmi,
                                         UInt32   page)
{
    const UInt32 cmd1_addr_cmd2_done = (FMC_STATUS__ADDRESSDONE |
                                        FMC_STATUS__CMD2DONE |
                                        FMC_STATUS__CMD1DONE);

    // configure FMC for cmd1/addr/cmd2 sequence
    h2fmi_config_erase_page_addr(fmi, page);
    h2fmi_wr(fmi, FMC_CMD, (FMC_CMD__CMD1(NAND_CMD__ERASE) |
                            FMC_CMD__CMD2(NAND_CMD__ERASE_CONFIRM)));

    // start cmd1/addr/cmd2 sequence
    h2fmi_wr(fmi, FMC_RW_CTRL, (FMC_RW_CTRL__ADDR_MODE |
                                FMC_RW_CTRL__CMD2_MODE |
                                FMC_RW_CTRL__CMD1_MODE));

    // busy wait until cmd1/addr/cmd2 sequence completed
    h2fmi_busy_wait(fmi, FMC_STATUS, cmd1_addr_cmd2_done, cmd1_addr_cmd2_done);
    h2fmi_wr(fmi, FMC_STATUS, cmd1_addr_cmd2_done);    
}

static void h2fmi_config_erase_page_addr(h2fmi_t* fmi,
                                         UInt32   page)
{
    const UInt32 page_addr_size = 3;
    const UInt32 page_addr_2 = (page >> 16) & 0xFF;
    const UInt32 page_addr_1 = (page >> 8) & 0xFF;
    const UInt32 page_addr_0 = (page >> 0) & 0xFF;

    h2fmi_wr(fmi, FMC_ADDR0, (FMC_ADDR0__SEQ3(0x00) |
                              FMC_ADDR0__SEQ2(page_addr_2) |
                              FMC_ADDR0__SEQ1(page_addr_1) |
                              FMC_ADDR0__SEQ0(page_addr_0)));
    h2fmi_wr(fmi, FMC_ADDRNUM, FMC_ADDRNUM__NUM(page_addr_size - 1));
}

// ********************************** EOF **************************************
