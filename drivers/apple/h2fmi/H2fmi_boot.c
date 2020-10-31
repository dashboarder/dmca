// *****************************************************************************
//
// File: H2fmi_boot.c
//
// *****************************************************************************
//
// Notes:
//
//   - This file contains initialization functions for iBoot and IOP
//
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
#include "H2fmi_private.h"
#include "H2fmi_ppn.h"
#if WMR_BUILDING_EFI
#include "SoC.h"
#else
#include "soc.h"
#endif // WMR_BUILDING_EFI
#include "WMROAM.h"

// =============================================================================
// implementation function declarations shared between SecureROM and iBoot
// =============================================================================
static BOOL32 h2fmi_wait_ready(h2fmi_t* fmi, UInt8 io_mask, UInt8 io_val);


BOOL32 h2fmi_is_chipid_invalid(h2fmi_chipid_t* const id)
{
    const h2fmi_chipid_t Zeroes = { 0x00, 0x00, 0x00, 0x00, 0x00 };
    const h2fmi_chipid_t Ones   = { 0xff, 0xff, 0xff, 0xff, 0xff };

    if (WMR_MEMCMP(id, Zeroes, sizeof(h2fmi_chipid_t)) &&
        WMR_MEMCMP(id, Ones, sizeof(h2fmi_chipid_t)))
    {
        return FALSE32;
    }
    else
    {
        return TRUE32;
    }
}

void h2fmi_reset(h2fmi_t* fmi)
{
    // reset the specified FMI subsystem
    WMR_CLOCK_RESET_DEVICE(h2fmi_get_gate(fmi));

#if FMI_VERSION > 0
    h2fmi_wr(fmi, FMI_CONTROL, FMI_CONTROL__MODE__SOFTWARE_RESET);
#endif

    turn_on_fmc(fmi);
    restoreTimingRegs(fmi);
}

BOOL32 h2fmi_wait_done(h2fmi_t* fmi,
                       UInt32   reg,
                       UInt32   mask,
                       UInt32   bits)
{
    BOOL32 result = TRUE32;

    UInt64 check_time = H2FMI_DEFAULT_TIMEOUT_MICROS * WMR_GET_TICKS_PER_US();
    UInt64 start_time = WMR_CLOCK_TICKS();

    // wait for specified bits to be set
    while ((h2fmi_rd(fmi, reg) & mask) != bits)
    {
        if (WMR_HAS_TIME_ELAPSED_TICKS(start_time, check_time))
        {
            h2fmi_fail(result);
            break;
        }

        WMR_YIELD();
    }

    // clear specified bits
    h2fmi_wr(fmi, reg, bits);

    return result;
}

BOOL32 h2fmi_wait_dma_task_pending(h2fmi_t* fmi)
{
    BOOL32 result = TRUE32;

    UInt64 check_time  = H2FMI_DEFAULT_TIMEOUT_MICROS * WMR_GET_TICKS_PER_US();
    UInt64 start_time  = WMR_CLOCK_TICKS();

    // wait for FMI to complete transfer of sector to fifo
    const uint32_t msk = FMI_DEBUG0__DMA_TASKS_PENDING(0x3);

    // wait for FMI to to be ready for at least one dma task
    while ((h2fmi_rd(fmi, FMI_DEBUG0) & msk) == 0)
    {
        if (WMR_HAS_TIME_ELAPSED_TICKS(start_time, check_time))
        {
            h2fmi_fail(result);
            break;
        }

        WMR_YIELD();
    }

    return result;
}

void h2fmi_config_page_addr(h2fmi_t* fmi,
                            UInt32   page)
{
    const UInt32 page_addr_size = 5;

    const UInt32 page_addr_2 = (page >> 16) & 0xFF;
    const UInt32 page_addr_1 = (page >> 8) & 0xFF;
    const UInt32 page_addr_0 = (page >> 0) & 0xFF;

    h2fmi_wr(fmi, FMC_ADDR1, (FMC_ADDR1__SEQ7(0x00) |
                              FMC_ADDR1__SEQ6(0x00) |
                              FMC_ADDR1__SEQ5(0x00) |
                              FMC_ADDR1__SEQ4(page_addr_2)));
    h2fmi_wr(fmi, FMC_ADDR0, (FMC_ADDR0__SEQ3(page_addr_1) |
                              FMC_ADDR0__SEQ2(page_addr_0) |
                              FMC_ADDR0__SEQ1(0x00) |
                              FMC_ADDR0__SEQ0(0x00)));

    h2fmi_wr(fmi, FMC_ADDRNUM, FMC_ADDRNUM__NUM(page_addr_size - 1));
}

void h2fmi_clean_ecc(h2fmi_t* fmi)
{
    // clean status bits by setting them
#if FMI_VERSION == 0
    // S5L9820X
    h2fmi_wr(fmi, ECC_PND, (ECC_PND__BCH_ALL_FF |
                            ECC_PND__SECTOR_ERROR_ALERT |
                            ECC_PND__UNCORRECTABLE));

#else
    // S5L8922X and later
    h2fmi_wr(fmi, ECC_PND, (ECC_PND__SOME_ALL_ZERO |
                            ECC_PND__SOME_ALL_FF |
                            ECC_PND__SOME_ERROR_ALERT |
                            ECC_PND__ANY_UNCORRECTABLE));
#endif
}

void h2fmi_fmc_read_data(h2fmi_t* fmi,
                         UInt32   size,
                         UInt8*   data)
{
    register UInt32 idx;
    UInt32          delay_ticks;
    UInt32          if_ctrl;

    // Force-clear FMC timings.  We're manually hitting /RE and the I/O lines
    // so we can take care of timings ourselves.
    if_ctrl = h2fmi_rd(fmi, FMC_IF_CTRL);
    h2fmi_set_if_ctrl(fmi, 0);
    
    delay_ticks = WMR_GET_TICKS_PER_US() / 8; //(125ns)
    
    // employ REBHOLD to hold nRD low so that each byte on the bus can
    // be read, one at a time, from NAND_STATUS
    h2fmi_wr(fmi, FMC_DATANUM, FMC_DATANUM__NUM(0));
    for (idx = 0; idx < size; idx++)
    {
        UInt64  start;
        
        h2fmi_wr(fmi, FMC_RW_CTRL, (FMC_RW_CTRL__REBHOLD |
                                    FMC_RW_CTRL__RD_MODE));
        
        // FMC requires 8 + REBSETUP cycles between asserting REBHOLD and reading FMC_NAND_STATUS.
        start = WMR_CLOCK_TICKS();        
        while(!WMR_HAS_TIME_ELAPSED_TICKS(start, delay_ticks));
        
        data[idx] = (UInt8)h2fmi_rd(fmi, FMC_NAND_STATUS);

        // clear FMC_RW_CTRL, thereby releasing REBHOLD and completing
        // current read cycle
        h2fmi_wr(fmi, FMC_RW_CTRL, 0);
    }

    h2fmi_set_if_ctrl(fmi, if_ctrl);
}

BOOL32 h2fmi_nand_reset(h2fmi_t*   fmi,
                        h2fmi_ce_t ce)
{
    BOOL32 result = TRUE32;

    // enable specified nand device
    h2fmi_fmc_enable_ce(fmi, ce);

// Only for H5X - // <rdar://problem/11677742> Software workaround for Samsung reset and bad chip ID isssue
#if FMI_VERSION >= 5
        WMR_SLEEP_US(H2FMI_EXTRA_CE_SETUP_TIME_US);
#endif // FMI_VERSION >= 5

    // perform reset command
    h2fmi_wr(fmi, FMC_CMD, FMC_CMD__CMD1(NAND_CMD__RESET));
    h2fmi_wr(fmi, FMC_RW_CTRL, FMC_RW_CTRL__CMD1_MODE);

    // wait until cmd sent on bus
    if (!h2fmi_wait_done(fmi, FMC_STATUS,
                         FMC_STATUS__CMD1DONE,
                         FMC_STATUS__CMD1DONE))
    {
        h2fmi_fail(result);
    }

    // disable all nand devices
    h2fmi_fmc_disable_ce(fmi, ce);

#if SUPPORT_PPN && H2FMI_PPN_VERIFY_SET_FEATURES
    if (fmi->is_ppn)
    {
        h2fmi_ppn_reset_feature_shadow(fmi);
    }
#endif // SUPPORT_PPN && H2FMI_PPN_VERIFY_SET_FEATURES

    return result;
}

BOOL32 h2fmi_nand_read_id(h2fmi_t*        fmi,
                          h2fmi_ce_t      ce,
                          h2fmi_chipid_t* id,
                          UInt8           addr)
{
    const UInt32 cmd1_addr_done = (FMC_STATUS__ADDRESSDONE |
                                   FMC_STATUS__CMD1DONE);
    BOOL32 result = TRUE32;

#if SUPPORT_TOGGLE_NAND
    WMR_ASSERT(!fmi->is_toggle);
#endif
    // enable specified nand device
    h2fmi_fmc_enable_ce(fmi, ce);

    // set up Read Id command and address cycles
    h2fmi_wr(fmi, FMC_CMD, FMC_CMD__CMD1(NAND_CMD__READ_ID));
    h2fmi_wr(fmi, FMC_ADDR0, FMC_ADDR0__SEQ0(addr));
    h2fmi_wr(fmi, FMC_ADDRNUM, FMC_ADDRNUM__NUM(0));
    h2fmi_wr(fmi, FMC_RW_CTRL, (FMC_RW_CTRL__ADDR_MODE |
                                FMC_RW_CTRL__CMD1_MODE));

    // wait until cmd & addr completion
    if (!h2fmi_wait_done(fmi, FMC_STATUS,
                         cmd1_addr_done,
                         cmd1_addr_done))
    {
        h2fmi_fail(result);
    }
    else
    {
#if FMI_VERSION == 0
        // read correct number of id bytes into local temporary
        h2fmi_fmc_read_data(fmi, H2FMI_NAND_ID_SIZE, *id);
#else
        UInt32 fmi_control = (FMI_CONTROL__MODE__READ |
                              FMI_CONTROL__START_BIT);

#if FMI_VERSION == 4
        if (fmi->read_stream_disable)
        {
            fmi_control |= FMI_CONTROL__DISABLE_STREAMING;
        }
#endif // FMI_VERSION == 4

        h2fmi_wr(fmi, FMI_CONFIG, 0);
        h2fmi_wr(fmi, FMI_DATA_SIZE, (FMI_DATA_SIZE__BYTES_PER_SECTOR( ROUNDUPTO( sizeof(h2fmi_chipid_t), sizeof(UInt32) ) ) |
                                      FMI_DATA_SIZE__SECTORS_PER_PAGE(1)));
        h2fmi_wr(fmi, FMI_CONTROL, fmi_control);
        if (!h2fmi_wait_done(fmi, FMI_INT_PEND, FMI_INT_PEND__LAST_FMC_DONE, FMI_INT_PEND__LAST_FMC_DONE))
        {
            h2fmi_fail(result);
        }
        else
        {
            h2fmi_pio_read_sector(fmi, id, sizeof(h2fmi_chipid_t));
        }

        h2fmi_reset(fmi);
#endif
    }

    // disable all nand devices
    h2fmi_fmc_disable_ce(fmi, ce);

    return result;
}

BOOL32 h2fmi_pio_write_sector(h2fmi_t*       fmi,
                              const void*    buf,
                              UInt32         len)
{
    BOOL32 result = TRUE32;
    UInt32* ptr   = (UInt32*)buf;
    UInt32 i;

    if (!h2fmi_wait_dma_task_pending(fmi))
    {
        // timed out
        WMR_PANIC();

        h2fmi_fail(result);
    }
    else
    {
        for( i = 0; i < len / sizeof(UInt32); i++ )
        {
            h2fmi_wr(fmi, FMI_DATA_BUF, *ptr++);
        }
    }

    return result;
}

BOOL32 h2fmi_pio_read_sector(h2fmi_t* fmi,
                             void*    buf,
                             UInt32   len)
{
    BOOL32  result   = TRUE32;
    UInt32 *word_ptr = (UInt32*)buf;
    UInt8  *byte_ptr;
    int     i;

    // wait for FMI to be ready with data in fifo for DMA to transfer out
    if (!h2fmi_wait_dma_task_pending(fmi))
    {
        // timed out
        h2fmi_fail(result);
    }
    else
    {
        if (((UInt32)word_ptr & 0xF) == 0)
        {
            // Our buffers (particularly boot pages) are generally aligned.
            // Do as many 32 bit operations as we can.
            while (len >= sizeof(UInt32))
            {
                *word_ptr++ = h2fmi_rd(fmi, FMI_DATA_BUF);
                len -= sizeof(UInt32);
            }
        }

        byte_ptr = (UInt8*)word_ptr;
        while (len)
        {
            UInt8  bytes = WMR_MIN(sizeof(UInt32), len);
            UInt32 tmp   = h2fmi_rd(fmi, FMI_DATA_BUF);

            for (i = 0; i < bytes; i++)
            {
                *byte_ptr++ = (tmp >> (i*8)) & 0xFF;
            }

            len -= bytes;
        }
    }

    return result;
}

UInt32 h2fmi_get_gate(h2fmi_t* fmi)
{
    return h2fmi_select_by_bus(fmi, 0, 1 );
}

// =============================================================================
// static implementation function definitions used by both SecureROM and iBoot
// =============================================================================

BOOL32 h2fmi_start_nand_page_read(h2fmi_t* fmi,
                                  UInt32   page)
{
    const UInt32 cmd1_addr_cmd2_done = (FMC_STATUS__ADDRESSDONE |
                                        FMC_STATUS__CMD2DONE |
                                        FMC_STATUS__CMD1DONE);

    BOOL32 result = TRUE32;

    // configure FMC for cmd1/addr/cmd2 sequence
    h2fmi_config_page_addr(fmi, page);
    h2fmi_wr(fmi, FMC_CMD, (FMC_CMD__CMD2(NAND_CMD__READ_CONFIRM) |
                            FMC_CMD__CMD1(NAND_CMD__READ)));

    // start cmd1/addr/cmd2 sequence
    h2fmi_wr(fmi, FMC_RW_CTRL, (FMC_RW_CTRL__ADDR_MODE |
                                FMC_RW_CTRL__CMD2_MODE |
                                FMC_RW_CTRL__CMD1_MODE));

    // wait until cmd1/addr/cmd2 sequence completed
    if (!h2fmi_wait_done(fmi, FMC_STATUS,
                         cmd1_addr_cmd2_done,
                         cmd1_addr_cmd2_done))
    {
        h2fmi_fail(result);
    }
    // wait for ready using Read Status command
    else if (!h2fmi_wait_ready(fmi,
                               NAND_STATUS__DATA_CACHE_RB,
                               NAND_STATUS__DATA_CACHE_RB_READY))
    {
        h2fmi_fail(result);
    }
    else
    {
        // reissue cmd1 and wait until completed
        h2fmi_wr(fmi, FMC_CMD, FMC_CMD__CMD1(NAND_CMD__READ));
        h2fmi_wr(fmi, FMC_RW_CTRL, FMC_RW_CTRL__CMD1_MODE);
        if (!h2fmi_wait_done(fmi, FMC_STATUS,
                             FMC_STATUS__CMD1DONE,
                             FMC_STATUS__CMD1DONE))
        {
            h2fmi_fail(result);
        }
    }

    return result;
}

static BOOL32 h2fmi_wait_ready(h2fmi_t* fmi,
                               UInt8    io_mask,
                               UInt8    io_val)
{
    BOOL32 result = TRUE32;

    // turn off RBBEN and configure it to use look for I/O bit 6 (0x40)
    h2fmi_set_if_ctrl(fmi, (~FMC_IF_CTRL__RBBEN & h2fmi_rd(fmi, FMC_IF_CTRL)));
    h2fmi_wr(fmi, FMC_RBB_CONFIG, (FMC_RBB_CONFIG__POL(io_val) |
                                   FMC_RBB_CONFIG__POS(io_mask)));

    // exec Read Status command cycle
    h2fmi_wr(fmi, FMC_CMD, FMC_CMD__CMD1(NAND_CMD__READ_STATUS));
    h2fmi_wr(fmi, FMC_RW_CTRL, FMC_RW_CTRL__CMD1_MODE);

    // wait until command cycle completed
    if (!h2fmi_wait_done(fmi, FMC_STATUS,
                         FMC_STATUS__CMD1DONE,
                         FMC_STATUS__CMD1DONE))
    {
        h2fmi_fail(result);
    }
    else
    {
        // clear the FMC_STATUS__NSRBBDONE bit in FMC_STATUS
        h2fmi_wr(fmi, FMC_STATUS, FMC_STATUS__NSRBBDONE);

        // set up read cycle with REBHOLD to wait for ready bit to be
        // set on I/O lines
        h2fmi_wr(fmi, FMC_DATANUM, FMC_DATANUM__NUM(0));
        h2fmi_wr(fmi, FMC_RW_CTRL, (FMC_RW_CTRL__REBHOLD | FMC_RW_CTRL__RD_MODE));

        // wait until NSRBB done signaled
        if (!h2fmi_wait_done(fmi, FMC_STATUS,
                             FMC_STATUS__NSRBBDONE,
                             FMC_STATUS__NSRBBDONE))
        {
            h2fmi_fail(result);
        }

        // clear FMC_RW_CTRL, thereby releasing REBHOLD and completing
        // current read cycle
        h2fmi_wr(fmi, FMC_RW_CTRL, 0);
    }

    return result;
}

// ********************************** EOF **************************************
