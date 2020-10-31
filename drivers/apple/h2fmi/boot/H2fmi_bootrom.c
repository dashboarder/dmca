// *****************************************************************************
//
// File: H2fmi_bootrom.c
//
// *****************************************************************************
//
// Notes:
//
//   - all code implementing the SecureROM nand boot are contained in this
//     file and the headers "H2fmi_regs.h" and "H2fmi_private.h"
//
//
//   - care has been taken to make certain that SecureROM-only code will only
//     get compiled when building SecureROM or when building for debug in
//     order to facilitate testing
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

#include <drivers/flash_nand.h>
#include <platform/soc/hwclocks.h>
#include <platform/soc/pmgr.h>

#include <sys/task.h>

#include <debug.h>


// =============================================================================
// SecureROM-only preprocessor constants
// =============================================================================

// this symbol normally comes from <include/drivers/flash_nand.h>, but
// it is not normally available outside of SecureROM builds; for our
// testing purposes, make certain it is present for the moment
#if (!defined(NAND_BOOTBLOCK_OFFSET))
#define NAND_BOOTBLOCK_OFFSET 2
#endif


// =============================================================================
// SecureROM-only static variable declarations
// =============================================================================

h2fmi_t g_boot_fmi;


// =============================================================================
// implementation function declarations shared between SecureROM and iBoot
// =============================================================================

static int h2fmi_get_clock(h2fmi_t* fmi);
static int h2fmi_get_gate(h2fmi_t* fmi);
static int h2fmi_get_bch_gate(h2fmi_t* fmi);
static uint32_t h2fmi_nanos_to_clocks(uint64_t nanos_per_clock, uint32_t min_nanos);
static void h2fmi_calc_init_timings(h2fmi_t* fmi);
static void h2fmi_calc_timing_clocks(uint64_t nanos_per_clock, uint32_t min_cycle_nanos, uint32_t min_setup_nanos, uint32_t min_hold_nanos, uint32_t* setup_clocks, uint32_t* hold_clocks);
static uint32_t h2fmi_calc_read_capture(uint32_t hold_clocks);
static uint32_t h2fmi_min(uint32_t first, uint32_t second);
static bool h2fmi_start_nand_page_read(h2fmi_t* fmi, uint32_t page);
static bool h2fmi_wait_ready(h2fmi_t* fmi, uint8_t io_mask, uint8_t io_val);
static void h2fmi_wait_millis(uint32_t millis);


// =============================================================================
// implementation function declarations used by SecureROM only
// =============================================================================

static bool h2fmi_setup_bootpage_read(h2fmi_t* fmi, uint32_t cs, uint32_t page);
static bool h2fmi_pio_read_bootpage(h2fmi_t* fmi, uint32_t cs, uint32_t page, uint8_t* buf);
static bool h2fmi_pio_discard_bootpage(h2fmi_t* fmi, uint32_t cs, uint32_t page);


// =============================================================================
// SecureROM-only public interface function definitions
// =============================================================================

int nand_read_bootblock(u_int8_t interface,
                        u_int8_t ce,
                        void*    address,
                        size_t*  size)
{
    uint8_t* buffer = (uint8_t*)address;
    size_t resid;
    uint32_t page;
    uint32_t chipid;
    h2fmi_t* fmi = &g_boot_fmi;

    dprintf(DEBUG_INFO, "reading bootcode from FMI%d:CE%d to %p\n",
            interface, ce, buffer);

    // check range of interface parameter
    if ((0 != interface) && (1 != interface))
    {
        dprintf(DEBUG_INFO, "failed: invalid interface, %d\n", interface);
        return -1;
    }

    // check range of chip enable parameter
    if (ce >= H2FMI_MAX_DEVICE)
    {
        dprintf(DEBUG_INFO, "failed: invalid ce, %d\n", ce);
        return -1;
    }

    // perform minimum FMI initialization
    if (!h2fmi_init_minimal(fmi, interface))
    {
        dprintf(DEBUG_INFO, "failed: unable to initialize FMI%d\n", interface);
        return -1;
    }

    // reset the nand device
    if (!h2fmi_nand_reset(fmi, ce))
    {
        dprintf(DEBUG_INFO, "failed: unable to reset nand device on CE%d\n", ce);
        return -1;
    }

    // read id from nand device
    if (!h2fmi_nand_read_id(fmi, ce, &chipid))
    {
        dprintf(DEBUG_INFO, "failed: unable to read chip id\n");
        return -1;
    }

    // confirm that chipid is not invalid
    if (h2fmi_is_chipid_invalid(chipid))
    {
        dprintf(DEBUG_INFO, "failed: invalid chip id, 0x%08X\n", chipid);
        return -1;
    }

    // skip a couple of initial pages in the bootblock
    for (page = 0; page < NAND_BOOTBLOCK_OFFSET; page++)
    {
        dprintf(DEBUG_SPEW, "discarding page %d\n", page);
        if (!h2fmi_pio_discard_bootpage(fmi, ce, page))
        {
            dprintf(DEBUG_INFO, "failed: unable to discard initial page, %d\n", page);
            return -1;
        }
    }

    // read out the pages...
    for (resid = *size;
         resid >= H2FMI_BOOT_BYTES_PER_PAGE;
         resid -= H2FMI_BOOT_BYTES_PER_PAGE)
    {
        dprintf(DEBUG_SPEW, "reading page %d to %p\n", page, buffer);
        if (!h2fmi_pio_read_bootpage(fmi, ce, page, buffer))
            break;

        page++;
        buffer += H2FMI_BOOT_BYTES_PER_PAGE;
    }

    // gate the FMI subsystem and gate its dedicated HCLK clock domain
    h2fmi_gate(fmi);

    // compute transferred size
    *size = (page - NAND_BOOTBLOCK_OFFSET) * H2FMI_BOOT_BYTES_PER_PAGE;

    if (0 == *size)
    {
        dprintf(DEBUG_INFO, "failed: read 0 bytes\n");
        return -1;
    }

    dprintf(DEBUG_INFO, "read %u bytes\n", *size);
    return 0;
}


// =============================================================================
// implementation function definitions shared between SecureROM and iBoot
// =============================================================================

bool h2fmi_init_minimal(h2fmi_t* fmi,
                        uint32_t interface)
{
    bool result = true;

    // only interface numbers 0 and 1 are valid
    if ((interface != 0) && (interface != 1))
    {
        h2fmi_fail(result);
    }
    else
    {
        // record which interface is being used
        fmi->regs = (interface == 0) ? FMI0 : FMI1;

        // "get the clock rolling" for the dedicated HCLK clock domain
        h2fmi_ungate(fmi);

        // reset the FMI subsystem
        h2fmi_reset(fmi);

        // configure safe initial bus timings
        h2fmi_calc_init_timings(fmi);

        // configure geometry as a "least common denominator" device
        fmi->num_of_ce = 1;
        fmi->pages_per_block  = 128;
        fmi->sectors_per_page = 4;
        fmi->bytes_per_spare  = 64;
        fmi->blocks_per_ce = 1;
        fmi->bytes_per_meta   = 0;
        fmi->banks_per_ce = 1;
    }

    return result;
}


bool h2fmi_is_chipid_invalid(uint32_t id)
{
    return (id == 0x00000000) || (id == 0xFFFFFFFF);
}


void h2fmi_ungate(h2fmi_t* fmi)
{
    // enable the general FMI clock
    clock_gate(h2fmi_get_gate(fmi), true);
    
#if FMI_VERSION >= 2
    // H3 needs the BCH clock enabled as well
    clock_gate(h2fmi_get_bch_gate(fmi), true);
#endif
}


void h2fmi_gate(h2fmi_t* fmi)
{
    // disable the specified FMI block
    clock_gate(h2fmi_get_gate(fmi), false);

#if FMI_VERSION >= 2
    clock_gate(h2fmi_get_bch_gate(fmi), false);    
#endif
}


void h2fmi_reset(h2fmi_t* fmi)
{
    // reset the specified FMI subsystem
    clock_reset_device(h2fmi_get_gate(fmi));

    // turn the FMC on
    h2fmi_wr(fmi, FMC_ON, FMC_ON__ENABLE);

    // restore interface timings
    h2fmi_wr(fmi, FMC_IF_CTRL, fmi->if_ctrl);
}


#define timing_nanos(cycles, hz) ((((uint64_t)1000000000) * cycles) / hz)

void h2fmi_calc_bus_timings(h2fmi_t* fmi,
                            uint32_t min_read_cycle_nanos,
                            uint32_t min_read_setup_nanos,
                            uint32_t min_read_hold_nanos,
                            uint32_t min_write_cycle_nanos,
                            uint32_t min_write_setup_nanos,
                            uint32_t min_write_hold_nanos)
{
    uint32_t read_setup_clocks;
    uint32_t read_hold_clocks;
    uint32_t read_capture_clocks;
    uint32_t read_dccycle_clocks;
    uint32_t write_setup_clocks;
    uint32_t write_hold_clocks;

    // Get nand subsystem clock speed in Hz, and use that to
    // calculate how many nanoseconds per clock.
    const uint64_t fmi_clock_hz = clock_get_frequency(h2fmi_get_clock(fmi));
    const uint64_t fmi_nanos_per_clock = timing_nanos(1, fmi_clock_hz);

    // Calculate setup and hold clocks to meet write timings.
    h2fmi_calc_timing_clocks(fmi_nanos_per_clock,
                             min_write_cycle_nanos, min_write_setup_nanos, min_write_hold_nanos,
                             &write_setup_clocks, &write_hold_clocks);

    // Calculate setup and hold clocks as well as capture clocks to meet read timings.
    h2fmi_calc_timing_clocks(fmi_nanos_per_clock,
                             min_read_cycle_nanos, min_read_setup_nanos, min_read_hold_nanos,
                             &read_setup_clocks, &read_hold_clocks);
    read_capture_clocks = h2fmi_calc_read_capture(read_hold_clocks);

    // since we are using very slow cycles use FMC_IF_CTRL__DCCYCLE = 0
    read_dccycle_clocks = 0;

    // Calculate and record FMC_IF_CTRL setting from these calculated timings.
    fmi->if_ctrl = (FMC_IF_CTRL__DCCYCLE(read_dccycle_clocks) |
                    FMC_IF_CTRL__REB_SETUP(read_setup_clocks) |
                    FMC_IF_CTRL__REB_HOLD(read_hold_clocks) |
                    FMC_IF_CTRL__WEB_SETUP(write_setup_clocks) |
                    FMC_IF_CTRL__WEB_HOLD(write_hold_clocks));

    // When debugging, report the timing values that were calculated.
    if (H2FMI_DEBUG)
    {
        const uint64_t read_setup_nanos  = timing_nanos(read_setup_clocks,
                                                        fmi_clock_hz);
        const uint64_t read_hold_nanos   = timing_nanos(read_hold_clocks,
                                                        fmi_clock_hz);
        const uint64_t read_sample_nanos = timing_nanos(read_dccycle_clocks,
                                                        fmi_clock_hz);
        const uint64_t write_setup_nanos = timing_nanos(write_setup_clocks,
                                                        fmi_clock_hz);
        const uint64_t write_hold_nanos  = timing_nanos(write_hold_clocks,
                                                        fmi_clock_hz);

        dprintf(DEBUG_SPEW, "FMI clock is %d Hz.\n", fmi_clock_hz);
        dprintf(DEBUG_SPEW, "read:  setup = %d ns; hold = %d ns; sample = %d ns\n",
                read_setup_nanos, read_hold_nanos, read_sample_nanos);
        dprintf(DEBUG_SPEW, "write: setup = %d ns; hold = %d ns\n",
                write_setup_nanos, write_hold_nanos);
        dprintf(DEBUG_SPEW, "FMC_IF_CTRL__DCCYCLE = %d\n", read_dccycle_clocks);
        dprintf(DEBUG_SPEW, "FMC_IF_CTRL__REB_SETUP = %d\n", read_setup_clocks);
        dprintf(DEBUG_SPEW, "FMC_IF_CTRL__REB_HOLD = %d\n", read_hold_clocks);
        dprintf(DEBUG_SPEW, "FMC_IF_CTRL__WEB_SETUP = %d\n", write_setup_clocks);
        dprintf(DEBUG_SPEW, "FMC_IF_CTRL__WEB_HOLD = %d\n", write_hold_clocks);
    }

    // NOTE: hard code timing for temporary workaround...investigation to be continued
    fmi->if_ctrl = 0x2121;

    // Apply calculated bus timings.
    h2fmi_wr(fmi, FMC_IF_CTRL, fmi->if_ctrl);
}


bool h2fmi_wait_done(h2fmi_t* fmi,
                     uint32_t reg,
                     uint32_t mask,
                     uint32_t bits)
{
    bool result = true;

    utime_t check_time = H2FMI_DEFAULT_TIMEOUT_MICROS;
    utime_t start_time = system_time();

    // wait for specified bits to be set
    while ((h2fmi_rd(fmi, reg) & mask) != bits)
    {
        if (time_has_elapsed(start_time, check_time))
        {
            h2fmi_fail(result);
            break;
        }

        task_yield();
    }

    // clear specified bits
    h2fmi_wr(fmi, reg, bits);

    return result;
}


bool h2fmi_wait_dma_task_pending(h2fmi_t* fmi)
{
    bool result = true;

    utime_t check_time = H2FMI_DEFAULT_TIMEOUT_MICROS;
    utime_t start_time = system_time();

    // wait for FMI to complete transfer of sector to fifo
    const uint32_t msk = FMI_DEBUG0__DMA_TASKS_PENDING(0x3);

    // wait for FMI to to be ready for at least one dma task
    while ((h2fmi_rd(fmi, FMI_DEBUG0) & msk) == 0)
    {
        if (time_has_elapsed(start_time, check_time))
        {
            h2fmi_fail(result);
            break;
        }

        task_yield();
    }

    return result;
}


void h2fmi_config_page_addr(h2fmi_t* fmi,
                            uint32_t page)
{
    const uint32_t page_addr_size = 5;

    const uint32_t page_addr_2 = (page >> 16) & 0xFF;
    const uint32_t page_addr_1 = (page >> 8) & 0xFF;
    const uint32_t page_addr_0 = (page >> 0) & 0xFF;

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
    h2fmi_wr(fmi, ECC_PND, (ECC_PND__BCH_ALL_FF |
                            ECC_PND__SECTOR_ERROR_ALERT |
                            ECC_PND__UNCORRECTABLE));

    // clean all sector results by setting the error flag
    h2fmi_wr(fmi, ECC_RESULT, ECC_RESULT__ERROR_FLAG);
#else
    h2fmi_wr(fmi, ECC_PND, (ECC_PND__SOME_ALL_FF |
                            ECC_PND__SOME_ERROR_ALERT |
                            ECC_PND__UNCORRECTABLE));

    h2fmi_wr(fmi, ECC_RESULT, ECC_RESULT_FIFO__RESET_FIFO);
#endif
}


void h2fmi_fmc_read_data(h2fmi_t* fmi,
                         uint32_t size,
                         uint8_t* data)
{
    register uint32_t idx;

    // employ REBHOLD to hold nRD low so that each byte on the bus can
    // be read, one at a time, from NAND_STATUS
    h2fmi_wr(fmi, FMC_DATANUM, FMC_DATANUM__NUM(0));
    for (idx = 0; idx < size; idx++)
    {
        h2fmi_wr(fmi, FMC_RW_CTRL, (FMC_RW_CTRL__REBHOLD |
                                    FMC_RW_CTRL__RD_MODE));

        data[idx] = (uint8_t)h2fmi_rd(fmi, FMC_NAND_STATUS);

        // clear FMC_RW_CTRL, thereby releasing REBHOLD and completing
        // current read cycle
        h2fmi_wr(fmi, FMC_RW_CTRL, 0);
    }
}


bool h2fmi_nand_reset(h2fmi_t*   fmi,
                      h2fmi_ce_t ce)
{
    bool result = true;

    // enable specified nand device
    h2fmi_wr(fmi, FMC_CE_CTRL, FMC_CE_CTRL__CEB(ce));

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

    // wait 20 ms to allow nand to reset
    h2fmi_wait_millis(20);

    // disable all nand devices
    h2fmi_wr(fmi, FMC_CE_CTRL, 0x0);

    return result;
}


bool h2fmi_nand_read_id(h2fmi_t*   fmi,
                        h2fmi_ce_t ce,
                        uint32_t*  id)
{
    const uint32_t cmd1_addr_done = (FMC_STATUS__ADDRESSDONE |
                                     FMC_STATUS__CMD1DONE);
    bool result = true;

    uint8_t tmp[H2FMI_NAND_ID_SIZE];

    // enable specified nand device
    h2fmi_wr(fmi, FMC_CE_CTRL, FMC_CE_CTRL__CEB(ce));

    // set up Read Id command and address cycles
    h2fmi_wr(fmi, FMC_CMD, FMC_CMD__CMD1(NAND_CMD__READ_ID));
    h2fmi_wr(fmi, FMC_ADDR0, FMC_ADDR0__SEQ0(0x00));
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
        // read correct number of id bytes into local temporary
        h2fmi_fmc_read_data(fmi, H2FMI_NAND_ID_SIZE, tmp);

        // drop last byte and endian flop the rest to get condensed
        // 32-bit id format
        *id = ((((uint32_t)tmp[3]) << 24) |
               (((uint32_t)tmp[2]) << 16) |
               (((uint32_t)tmp[1]) << 8) |
               (((uint32_t)tmp[0]) << 0));
    }

    // disable all nand devices
    h2fmi_wr(fmi, FMC_CE_CTRL, 0x0);

    return result;
}


bool h2fmi_pio_read_sector(h2fmi_t* fmi,
                           void*    buf)
{
    bool result = true;

    const uint32_t words_per_loop = 8;
    register uint32_t* dest = (uint32_t*)buf;
    register uint32_t word_idx = (H2FMI_WORDS_PER_SECTOR / words_per_loop);

    // wait for FMI to be ready with data in fifo for DMA to transfer out
    if (!h2fmi_wait_dma_task_pending(fmi))
    {
        // timed out
        h2fmi_fail(result);
    }
    else
    {
        // read data from the fifo
        do
        {
            register uint32_t tmp0;
            register uint32_t tmp1;
            register uint32_t tmp2;
            register uint32_t tmp3;
            register uint32_t tmp4;
            register uint32_t tmp5;
            register uint32_t tmp6;
            register uint32_t tmp7;

            tmp0 = h2fmi_rd(fmi, FMI_DATA_BUF);
            tmp1 = h2fmi_rd(fmi, FMI_DATA_BUF);
            tmp2 = h2fmi_rd(fmi, FMI_DATA_BUF);
            tmp3 = h2fmi_rd(fmi, FMI_DATA_BUF);
            tmp4 = h2fmi_rd(fmi, FMI_DATA_BUF);
            tmp5 = h2fmi_rd(fmi, FMI_DATA_BUF);
            tmp6 = h2fmi_rd(fmi, FMI_DATA_BUF);
            tmp7 = h2fmi_rd(fmi, FMI_DATA_BUF);

            *dest++ = tmp0;
            *dest++ = tmp1;
            *dest++ = tmp2;
            *dest++ = tmp3;
            *dest++ = tmp4;
            *dest++ = tmp5;
            *dest++ = tmp6;
            *dest++ = tmp7;
        }
        while (--word_idx);
    }

    return result;
}


// =============================================================================
// static implementation function definitions used by both SecureROM and iBoot
// =============================================================================

static int h2fmi_get_clock(h2fmi_t* fmi)
{
    return CLK_PCLK;
}


static int h2fmi_get_gate(h2fmi_t* fmi)
{
    return (FMI0 == fmi->regs) ? CLK_FMI0 : CLK_FMI1;
}

#if FMI_VERSION >= 2
static int h2fmi_get_bch_gate(h2fmi_t* fmi)
{
    return (FMI0 == fmi->regs) ? CLK_FMI0BCH : CLK_FMI1BCH;
}
#endif

static uint32_t h2fmi_nanos_to_clocks(uint64_t nanos_per_clock,
                                      uint32_t min_nanos)
{
    return (uint32_t)(((uint64_t)min_nanos / nanos_per_clock) +
                      ((uint64_t)min_nanos % nanos_per_clock ? 1 : 0));
}


static void h2fmi_calc_init_timings(h2fmi_t* fmi)
{
    h2fmi_calc_bus_timings(fmi,
                           H2FMI_INIT_READ_CYCLE_NANOS,
                           H2FMI_INIT_READ_SETUP_NANOS,
                           H2FMI_INIT_READ_HOLD_NANOS,
                           H2FMI_INIT_WRITE_CYCLE_NANOS,
                           H2FMI_INIT_WRITE_SETUP_NANOS,
                           H2FMI_INIT_WRITE_HOLD_NANOS);
}


static void h2fmi_calc_timing_clocks(uint64_t  nanos_per_clock,
                                     uint32_t  min_cycle_nanos,
                                     uint32_t  min_setup_nanos,
                                     uint32_t  min_hold_nanos,
                                     uint32_t* setup_clocks,
                                     uint32_t* hold_clocks)
{
    // Calculate clock cycle counts from the nand's specified parameters.
    uint32_t cycle = h2fmi_nanos_to_clocks(nanos_per_clock, min_cycle_nanos);
    uint32_t setup = h2fmi_nanos_to_clocks(nanos_per_clock, min_setup_nanos);
    uint32_t hold  = h2fmi_nanos_to_clocks(nanos_per_clock, min_hold_nanos);

    // If setup and hold combined are less than minimum cycle time, extend the hold time.
    if ((hold + setup) < cycle)
    {
        hold = cycle - setup;
    }

    // Hold and setup clock counts are expressed zero based.
    hold--;
    setup--;

    // Don't allow single clock setup or hold times.  As observed
    // using a scope, such timings create degenerate sawtooth
    // waveforms that fall short of reaching Vmax.
    if (hold == 0)
    {
        hold = 1;
    }
    if (setup == 0)
    {
        setup = 1;
    }

    // Pass the calculated values back up by reference, making certain
    // that none exceed the maximum timing clock counts H2 can use.
    *setup_clocks = h2fmi_min(setup, FMC_IF_CTRL__TIMING_MAX_CLOCKS);
    *hold_clocks  = h2fmi_min(hold, FMC_IF_CTRL__TIMING_MAX_CLOCKS);
}


static uint32_t h2fmi_min(uint32_t first,
                          uint32_t second)
{
    return (first < second) ? first : second;
}


static bool h2fmi_start_nand_page_read(h2fmi_t* fmi,
                                       uint32_t page)
{
    const uint32_t cmd1_addr_cmd2_done = (FMC_STATUS__ADDRESSDONE |
                                          FMC_STATUS__CMD2DONE |
                                          FMC_STATUS__CMD1DONE);

    bool result = true;

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


static uint32_t h2fmi_calc_read_capture(uint32_t hold_clocks)
{
    // As long as max timing clock count is not exceeded, set it to
    // one greater than hold_clocks.
    return h2fmi_min(hold_clocks + 1, FMC_IF_CTRL__TIMING_MAX_CLOCKS);
}


static bool h2fmi_wait_ready(h2fmi_t* fmi,
                             uint8_t  io_mask,
                             uint8_t  io_val)
{
    bool result = true;

    // turn off RBBEN and configure it to use look for I/O bit 6 (0x40)
    h2fmi_wr(fmi, FMC_IF_CTRL, (~FMC_IF_CTRL__RBBEN & h2fmi_rd(fmi, FMC_IF_CTRL)));
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


static void h2fmi_wait_millis(uint32_t millis)
{
    // task_sleep for millis * 1000 us.
    task_sleep(millis * 1000);
}


// =============================================================================
// static implementation function definitions used by SecureROM only
// =============================================================================

static bool h2fmi_setup_bootpage_read(h2fmi_t* fmi,
                                      uint32_t ce,
                                      uint32_t page)
{
    bool result = true;
#if FMI_VERSION > 3
    uint32_t sector_idx;
#endif
    // enable specified nand device
    h2fmi_wr(fmi, FMC_CE_CTRL, FMC_CE_CTRL__CEB(ce));

    // start a nand page read operation
    if (!h2fmi_start_nand_page_read(fmi, page))
    {
        h2fmi_fail(result);
    }
    else
    {
#if FMI_VERSION	== 0
        // configure bootblock sector format: 512 bytes, no meta, 16-bit ECC
        h2fmi_wr(fmi, FMI_CONFIG, (FMI_CONFIG__LBA_MODE__NORMAL |
                                   FMI_CONFIG__META_PER_ENVELOPE(0) |
                                   FMI_CONFIG__META_PER_PAGE(0) |
                                   FMI_CONFIG__PAGE_SIZE__512 |
                                   FMI_CONFIG__ECC_MODE__16BIT));

        // we don't care about sector error alerts
        h2fmi_wr(fmi, ECC_CON1, (ECC_CON1__ERROR_ALERT_LEVEL(17) |
                                 ECC_CON1__INT_ENABLE(0)));
#elif FMI_VERSION <= 3
        // configure bootblock ecc: 30-bit ECC
        h2fmi_wr(fmi, FMI_CONFIG, FMI_CONFIG__ECC_MAX_CORRECTION);

        // configure bootblock sector format: 512 bytes, no meta
        h2fmi_wr(fmi, FMI_DATA_SIZE, (FMI_DATA_SIZE__BYTES_PER_SECTOR(512) |
                                      FMI_DATA_SIZE__SECTORS_PER_PAGE(1) |
                                      FMI_DATA_SIZE__META_BYTES_PER_ENVELOPE(0) |
                                      FMI_DATA_SIZE__META_BYTES_PER_PAGE(0)));

        // we don't care about sector error alerts
        h2fmi_wr(fmi, ECC_CON1, (ECC_CON1__ERROR_ALERT_LEVEL(0) |
                                 ECC_CON1__INT_ENABLE(0)));
#else
        // Use randomizer
        h2fmi_wr(fmi, FMI_CONFIG, FMI_CONFIG__ECC_MAX_CORRECTION |
                                  FMI_CONFIG__SCRAMBLE_SEED |
                                  FMI_CONFIG__ENABLE_WHITENING);
        h2fmi_wr(fmi, FMI_DATA_SIZE, (FMI_DATA_SIZE__META_BYTES_PER_ENVELOPE(0) |
                                      FMI_DATA_SIZE__META_BYTES_PER_PAGE(0) |
                                      FMI_DATA_SIZE__BYTES_PER_SECTOR(512) |
                                      FMI_DATA_SIZE__SECTORS_PER_PAGE(1)));
       // Use written-mark, so fewer stuck bits are allowed
        h2fmi_wr(fmi, ECC_CON1, (ECC_CON1__ERROR_ALERT_LEVEL(0) |
                                 ECC_CON1__INT_ENABLE(0)));
        h2fmi_wr(fmi, FMI_CONTROL, FMI_CONTROL__RESET_SEED);
        for (sector_idx = 0; sector_idx < H2FMI_BOOT_SECTORS_PER_PAGE; sector_idx++)
        {
            h2fmi_wr(fmi, FMI_SCRAMBLER_SEED_FIFO, (page + sector_idx));
        }
#endif

        // FMI manages ECC interaction, so make certain we aren't
        // unmasking any interrupt sources
        h2fmi_wr(fmi, ECC_MASK, 0x0);
    }

    return result;
}


static bool h2fmi_pio_read_bootpage(h2fmi_t* fmi,
                                    uint32_t ce,
                                    uint32_t page,
                                    uint8_t* buf)
{
#if FMI_VERSION	> 0
    bool foundDirty = false;
#endif
    bool result = true;

    uint32_t sector_idx;

    // setup a nand bootpage read operation
    if (!h2fmi_setup_bootpage_read(fmi, ce, page))
    {
        h2fmi_fail(result);
    }
    else
    {
        // for each sector...
        for (sector_idx = 0; sector_idx < H2FMI_BOOT_SECTORS_PER_PAGE; sector_idx++)
        {
            const uint32_t buf_offset = sector_idx * H2FMI_BYTES_PER_SECTOR;

            // clean up any ECC block state before starting transfer
            h2fmi_clean_ecc(fmi);

            // start FMI read
            h2fmi_wr(fmi, FMI_CONTROL, (FMI_CONTROL__MODE__READ |
                                        FMI_CONTROL__START_BIT));

            // read boot sector data from fifo
            if (!h2fmi_pio_read_sector(fmi, &buf[buf_offset]))
            {
                h2fmi_fail(result);
                break;
            }
            // handle interesting ECC conditions
            else
            {
                const uint32_t ecc_pnd = h2fmi_rd(fmi, ECC_PND);

                if (ecc_pnd & ECC_PND__UNCORRECTABLE)
                {
                    h2fmi_fail(result);
                    break;
                }
#if FMI_VERSION	== 0
                else if (ecc_pnd & ECC_PND__BCH_ALL_FF)
#else
                else if (ecc_pnd & ECC_PND__SOME_ALL_FF)
#endif
                {
#if FMI_VERSION	== 0
                    // return false, but don't necessarily "fail"
                    result = false;
                    break;
#else
                    dprintf(DEBUG_SPEW, "clean sector page %d sector %d\n", page, sector_idx);
#endif
                }
#if FMI_VERSION	> 0
                else
                {
                    // since BCH of 0xFF is 0xFF, check all sectors for clean
                    foundDirty = true;
                }
#endif
            }
        }
    }

    // disable all nand devices
    h2fmi_wr(fmi, FMC_CE_CTRL, 0x0);

#if FMI_VERSION	> 0
    if (result && !foundDirty)
    {
        // page is clean
        result = false;
    }
#endif

    return result;
}


static bool h2fmi_pio_discard_bootpage(h2fmi_t* fmi,
                                       uint32_t ce,
                                       uint32_t page)
{
    bool result = true;

    uint32_t sector_idx;

    // setup a nand bootpage read operation
    if (!h2fmi_setup_bootpage_read(fmi, ce, page))
    {
        h2fmi_fail(result);
    }
    else
    {
        // for each sector...
        for (sector_idx = 0; sector_idx < H2FMI_BOOT_SECTORS_PER_PAGE; sector_idx++)
        {
            // clean up any ECC block state before starting transfer
            h2fmi_clean_ecc(fmi);

            // start FMI read
            h2fmi_wr(fmi, FMI_CONTROL, (FMI_CONTROL__MODE__READ |
                                        FMI_CONTROL__START_BIT));

            // wait for FMI to be ready with data in fifo for DMA to transfer out
            if (!h2fmi_wait_dma_task_pending(fmi))
            {
                // timed out
                h2fmi_fail(result);
            }
            else
            {
                register uint32_t word_idx = H2FMI_WORDS_PER_SECTOR;
                register uint32_t tmp0 = 0;

                do
                {
                    // read each word from the fifo, discarding it
                    tmp0 |= h2fmi_rd(fmi, FMI_DATA_BUF);
                }
                while (--word_idx);
            }
        }
    }

    // disable all nand devices
    h2fmi_wr(fmi, FMC_CE_CTRL, 0x0);

    return result;
}



// ********************************** EOF **************************************
