/*
 * Copyright (C) 2011-2013 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */
#include <debug.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <sys.h>
#include <sys/task.h>
#include <platform.h>
#include <platform/soc/hwclocks.h>
#include <platform/soc/hwregbase.h>
#include <drivers/anc_boot.h>
#include "anc_bootrom.h"
#include "anc_bootrom_private.h"

#define NAND_BOOTBLOCK_OFFSET       2

#define NAND_BOOT_MIN_WRITE_CYCLE_NANOS  50
#define NAND_BOOT_MIN_READ_CYCLE_NANOS   50
#define NAND_BOOT_MIN_CA_CYCLE_NANOS     50
#define NAND_BOOT_MIN_WRITE_SETUP_NANOS  20
#define NAND_BOOT_MIN_WRITE_HOLD_NANOS   20
#define NAND_BOOT_MIN_READ_SETUP_NANOS   20
#define NAND_BOOT_MIN_READ_HOLD_NANOS    20
#define NAND_BOOT_MIN_CA_SETUP_NANOS     20
#define NAND_BOOT_MIN_CA_HOLD_NANOS      20

#define T_CERDY_CYCLES                   2675 // 50us @ 53.5MHz

#define ANC_TIMEOUT_MICROS               1000000UL // Applies to reg status waits - not tRST timeout
#define T_PURST_USEC                     (10 * 1000) // 10ms

anc_t g_boot_anc[ANC_BOOT_CONTROLLERS];

static bool anc_boot_init_minimal(anc_t *anc, uint32_t interface);
static void anc_configure_pins(void);
static void anc_boot_reset(anc_t *anc);
static bool anc_boot_nand_reset(int busesToReset);
static int anc_boot_read_boot_pages(uint32_t valid_controllers, void *data, size_t *size);
static void anc_boot_set_init_timings(anc_t *anc);

int anc_bootrom_init(bool reset_devices, int resetMode)
{
    int      i;
    int      busesToReset;

    if (resetMode == ANC_BOOT_MODE_RESET_ALL_CONTROLLERS) {
        busesToReset = ANC_BOOT_CONTROLLERS;
    } else if (resetMode == ANC_BOOT_MODE_RESET_ONE_CONTROLLER) {
        busesToReset = 1;
    } else {
        return -1;
    }

    anc_configure_pins();
    
    for (i = 0; i < ANC_BOOT_CONTROLLERS; i++)
    {
        if (!anc_boot_init_minimal(&g_boot_anc[i], i))
        {
            dprintf(DEBUG_INFO, "failed: unable to initialize ANC%d\n", i);
            return -1;
        }
    }

    if (reset_devices && !anc_boot_nand_reset(busesToReset))
    {
        dprintf(DEBUG_INFO, "failed: unable to reset NAND devices\n");
        return -1;
    }

    return 0;
}

static void anc_configure_pins()
{
    uint32_t *npl = (uint32_t *)ANS_PPN_N_PL_BASE_ADDR;
    anc_npl_wr(npl, R_PPNNPL_PPN0_CLE, M_PPNNPL_PPN0_CLE_OUTPUT_ENABLE);
    anc_npl_wr(npl, R_PPNNPL_PPN0_ALE, M_PPNNPL_PPN0_ALE_OUTPUT_ENABLE);
    anc_npl_wr(npl, R_PPNNPL_PPN0_REN, M_PPNNPL_PPN0_REN_OUTPUT_ENABLE);
    anc_npl_wr(npl, R_PPNNPL_PPN0_WEN, M_PPNNPL_PPN0_WEN_OUTPUT_ENABLE);
    anc_npl_wr(npl, R_PPNNPL_PPN0_CEN, M_PPNNPL_PPN0_CEN_OUTPUT_ENABLE);
    anc_npl_wr(npl, R_PPNNPL_PPN0_IO, (V_PPNNPL_PPN0_IO_INPUT_ENABLE(0xFF) |
                                       V_PPNNPL_PPN0_IO_PULL_ENABLE(0xFF)));
#if ANC_TOGGLE_SUPPORTED == 1
    anc_npl_wr(npl, R_PPNNPL_PPN0_DQS, (M_PPNNPL_PPN0_DQS_INPUT_ENABLE |
                                        M_PPNNPL_PPN0_DQS_PULL_ENABLE));
#endif
    anc_npl_wr(npl, R_PPNNPL_PPN0_DS, V_PPNNPL_PPN0_DS_DRIVE_STRENGTH(ANC_PPNNPL_DS_DRIVE_STRENGTH));
    anc_npl_wr(npl, R_PPNNPL_CONFIG, M_PPNNPL_CONFIG_AUTO_DISABLE_IO_PULLDOWN);
#if APPLICATION_SECUREROM && defined(ANC_PPNNPL_INPUT_SELECT_SCHMITT)
    // <rdar://problem/15132151> N56, High power on 1V8_H7P_IO - rail - PPN_INPUT_SELECT
    anc_npl_wr(npl, R_PPNNPL_PPN0_INPUT_SELECT, M_PPNNPL_PPN0_INPUT_SELECT_INPUT_SELECT |
                                                M_PPNNPL_PPN0_INPUT_SELECT_INPUT_SELECT_SCHMITT);
#endif

#if ANC_BOOT_CONTROLLERS > 1
    anc_npl_wr(npl, R_PPNNPL_PPN1_CLE, M_PPNNPL_PPN1_CLE_OUTPUT_ENABLE);
    anc_npl_wr(npl, R_PPNNPL_PPN1_ALE, M_PPNNPL_PPN1_ALE_OUTPUT_ENABLE);
    anc_npl_wr(npl, R_PPNNPL_PPN1_REN, M_PPNNPL_PPN1_REN_OUTPUT_ENABLE);
    anc_npl_wr(npl, R_PPNNPL_PPN1_WEN, M_PPNNPL_PPN1_WEN_OUTPUT_ENABLE);
    anc_npl_wr(npl, R_PPNNPL_PPN1_CEN, M_PPNNPL_PPN1_CEN_OUTPUT_ENABLE);
    anc_npl_wr(npl, R_PPNNPL_PPN1_IO, (V_PPNNPL_PPN1_IO_INPUT_ENABLE(0xFF) |
                                       V_PPNNPL_PPN1_IO_PULL_ENABLE(0xFF)));
#if ANC_TOGGLE_SUPPORTED == 1
    anc_npl_wr(npl, R_PPNNPL_PPN1_DQS, (M_PPNNPL_PPN1_DQS_INPUT_ENABLE |
                                        M_PPNNPL_PPN1_DQS_PULL_ENABLE));
#endif
    anc_npl_wr(npl, R_PPNNPL_PPN1_DS, V_PPNNPL_PPN1_DS_DRIVE_STRENGTH(ANC_PPNNPL_DS_DRIVE_STRENGTH));
#if APPLICATION_SECUREROM && defined(ANC_PPNNPL_INPUT_SELECT_SCHMITT)
    // <rdar://problem/15132151> N56, High power on 1V8_H7P_IO - rail - PPN_INPUT_SELECT
    anc_npl_wr(npl, R_PPNNPL_PPN1_INPUT_SELECT, M_PPNNPL_PPN1_INPUT_SELECT_INPUT_SELECT |
                                                M_PPNNPL_PPN1_INPUT_SELECT_INPUT_SELECT_SCHMITT);
#endif
#endif
}

static bool anc_boot_init_minimal(anc_t *anc, uint32_t interface)
{
#if ANC_BOOT_CONTROLLERS > 1
    anc->regs = (uint32_t *)((interface == 0) ? ANS_ANC_0_BASE_ADDR : ANS_ANC_1_BASE_ADDR);
#else
    anc->regs = (uint32_t *)ANS_ANC_BASE_ADDR;
    if (interface > 0)
    {
        return false;
    }
#endif
    if (interface == 0)
    {
        clock_gate(CLK_ANS, true);
#ifdef WITH_ANS_DLL
        clock_gate(CLK_ANS_DLL, true);
#endif
    }
    anc_boot_reset(anc);
    anc_boot_set_init_timings(anc);

    anc->bus_id = interface;
    anc->initialized = true;
    anc->pages_per_block   = NAND_BOOT_PAGES_PER_BLOCK;
    anc->bytes_per_meta    = NAND_BOOT_BYTES_PER_META;
    anc->meta_words        = NAND_BOOT_BYTES_PER_META / sizeof(uint32_t);
    anc->bytes_per_page    = NAND_BOOT_BYTES_PER_PAGE;
    anc->logical_page_size = NAND_BOOT_LOGICAL_PAGE_SIZE;
    anc->lbas_per_page     = NAND_BOOT_BYTES_PER_PAGE / NAND_BOOT_LOGICAL_PAGE_SIZE;

    return true;
}

static void anc_boot_reset(anc_t *anc)
{
    anc_wr(anc, R_ANC_LINK_CONFIG, (V_ANC_LINK_CONFIG_ENABLE_CRC(0) |
                                    V_ANC_LINK_CONFIG_DDR_MODE(0)));
    anc_wr(anc, R_ANC_LINK_CMD_TIMEOUT, (V_ANC_LINK_CMD_TIMEOUT_VALUE(60 * 1000 * 1000) |
                                         V_ANC_LINK_CMD_TIMEOUT_ENABLE(1)));
    anc_wr(anc, R_ANC_DMA_CONTROL, V_ANC_DMA_CONTROL_START(1));
    anc_wr(anc, R_ANC_LINK_CONTROL, V_ANC_LINK_CONTROL_START(1));
}

static void anc_boot_set_init_timings(anc_t *anc)
{
    // All timings are fixed and recommend by DV: <rdar://problem/11051297>
    anc_wr(anc, R_ANC_LINK_SDR_DATA_TIMING, (V_ANC_LINK_SDR_DATA_TIMING_REN_HOLD_TIME(ANC_LINK_SDR_REN_HOLD_TIME) |
                                             V_ANC_LINK_SDR_DATA_TIMING_REN_SETUP_TIME(ANC_LINK_SDR_REN_SETUP_TIME) |
                                             V_ANC_LINK_SDR_DATA_TIMING_WEN_HOLD_TIME(ANC_LINK_SDR_WEN_HOLD_TIME) |
                                             V_ANC_LINK_SDR_DATA_TIMING_WEN_SETUP_TIME(ANC_LINK_SDR_REN_SETUP_TIME)));
    anc_wr(anc, R_ANC_LINK_COMMAND_ADDRESS_PULSE_TIMING, 
           (V_ANC_LINK_COMMAND_ADDRESS_PULSE_TIMING_CA_HOLD_TIME(ANC_LINK_CMD_ADDR_PULSE_TIMING_CA_HOLD_TIME) |
            V_ANC_LINK_COMMAND_ADDRESS_PULSE_TIMING_CA_SETUP_TIME(ANC_LINK_CMD_ADDR_PULSE_TIMING_CA_SETUP_TIME)));
    anc_wr(anc, R_ANC_LINK_READ_STATUS_CONFIG, (V_ANC_LINK_READ_STATUS_CONFIG_POSITION(0x40) |
                                                V_ANC_LINK_READ_STATUS_CONFIG_POLARITY(0x40) |
                                                V_ANC_LINK_READ_STATUS_CONFIG_STATUS_CAPTURE_DELAY(10) |
                                                V_ANC_LINK_READ_STATUS_CONFIG_READY_SAMPLE_DELAY(8)));

    anc_wr(anc, R_ANC_LINK_SDR_TIMING, 
           (V_ANC_LINK_SDR_TIMING_DATA_CAPTURE_DELAY(ANC_LINK_SDR_DATA_CAPTURE_DELAY) |
            V_ANC_LINK_SDR_TIMING_CLE_ALE_SETUP_TIME(ANC_LINK_SDR_CLE_ALE_SETUP_TIME) |
            V_ANC_LINK_SDR_TIMING_CLE_ALE_HOLD_TIME(0)));
}

static bool anc_boot_nand_reset(int busesToReset)
{
    bool           ret;
    int            controller;
    anc_t         *anc;
    const uint32_t int_mask = (M_ANC_CHAN_INT_STATUS_LINK_CMD_FLAG |
                               M_ANC_CHAN_INT_STATUS_READ_STATUS_ERR_RESPONSE |
                               M_ANC_CHAN_INT_STATUS_LINK_CMD_TIMEOUT);
    uint32_t int_status;
    utime_t  current_time;

    // Wait 10ms after SoC reset to ensure we don't violate tPURST
    current_time = system_time();
    if (current_time < T_PURST_USEC)
    {
        task_sleep(T_PURST_USEC - current_time);
    }

    // First, issue a reset on all CEs attached to each bus
    for (controller = 0; controller < busesToReset; controller++)
    {
        anc = &g_boot_anc[controller];

        anc_boot_put_link_command(anc, LINK_COMMAND__CE(1));
        anc_boot_put_link_command(anc, LINK_COMMAND__WAIT_TIME(T_CERDY_CYCLES));
        anc_boot_put_link_command(anc, LINK_COMMAND__CMD1(NAND_CMD__RESET));
        anc_boot_put_link_command(anc, LINK_COMMAND__CE(0));
    }

    // Wait for bus 0, ce 0 to complete
    anc = &g_boot_anc[0];
    anc_boot_put_link_command(anc, LINK_COMMAND__CE(1));
    anc_boot_put_link_command(anc, LINK_COMMAND__CMD1(NAND_CMD__LOW_POWER_READ_STATUS));
    anc_boot_put_link_command(anc, LINK_COMMAND__WAIT_TIME(1));
    anc_boot_put_link_command(anc, LINK_COMMAND__READ_STATUS(0, PPN_LOW_POWER_STATUS__READY, PPN_LOW_POWER_STATUS__READY));
    anc_boot_put_link_command(anc, LINK_COMMAND__SEND_INTERRUPT(0, 0));

    int_status = anc_boot_wait_interrupt(anc, int_mask);
    if (!int_status)
    {
        // Timeout waiting for any status bits (including timeout bit - if we hit this, the ANC hw timeout
        // parts aren't working right
        dprintf(DEBUG_INFO, "Timeout waiting for CHAN_INT_STATUS != 0 - this should never happen\n");
        return false;
    }

    if (G_ANC_CHAN_INT_STATUS_LINK_CMD_FLAG(int_status))
    {
        // Successful status read
        ret = true;
    }
    else
    {
        // Timeout or invalid status
        dprintf(DEBUG_INFO, "Unexpected status 0x%08X\n", int_status);
        dprintf(DEBUG_INFO, "NAND Status: 0x%02X\n", anc_rd(anc, R_ANC_LINK_NAND_STATUS));
        ret = false;
    }

    return ret;
}

bool anc_bootrom_read_phys_page(uint32_t band,
								uint32_t dip,
								uint32_t page,
								uint32_t num_lbas,
								void     *data,
								uint32_t *meta)
{
    anc_t         *anc = &g_boot_anc[0];
    const uint32_t intmask = (M_ANC_CHAN_INT_STATUS_LINK_CMD_FLAG |
                              M_ANC_CHAN_INT_STATUS_LINK_CMD_TIMEOUT |
                              M_ANC_CHAN_INT_STATUS_READ_STATUS_ERR_RESPONSE);
    uint32_t     intstatus;
    unsigned int lba;
	addr_t		 paddr;
    bool         ret = true;

	paddr = mem_static_map_physical((addr_t)data);
    if (!paddr)
    {
        dprintf(DEBUG_INFO, "No buffer\n");
        return false;
    }

    if ((paddr & (16 - 1)) != 0)
    {
        dprintf(DEBUG_INFO, "ANC buffers must be 16 byte aligned\n");
        return false;
    }
    if (!meta)
    {
        dprintf(DEBUG_INFO, "No meta buffer\n");
        return false;
    }
    if (num_lbas == 0)
    {
        dprintf(DEBUG_INFO, "Invalid num_lbas %d\n", num_lbas);
        return false;
    }

#if WITH_NON_COHERENT_DMA
    platform_cache_operation(CACHE_CLEAN | CACHE_INVALIDATE, data, NAND_BOOT_LOGICAL_PAGE_SIZE * num_lbas);
#endif

    anc_boot_put_dma_command(anc, DMA_COMMAND_CONFIG(DMA_DIRECTION_N2M, false));
    anc_boot_put_link_command(anc, LINK_COMMAND__CE(1));
    anc_boot_put_link_command(anc, LINK_COMMAND__CMD1(NAND_CMD__READ));

    // Can't use LINK_CMD_ADDR6: rdar://problem/10941493
    anc_boot_put_link_command(anc, LINK_COMMAND__OPCODE(LINK_CMD_OP_ADDR6) | ((page & 0xFF) << 16));
    anc_boot_put_link_command(anc, page >> 8);

    anc_boot_put_link_command(anc, LINK_COMMAND__CMD2(NAND_CMD__READ_CONFIRM, NAND_CMD__READ_STATUS));
    anc_boot_put_link_command(anc, LINK_COMMAND__READ_STATUS(0x00,
                                                             (PPN_LOW_POWER_STATUS__READY |
                                                              PPN_LOW_POWER_STATUS__FAIL),
                                                             PPN_LOW_POWER_STATUS__READY));

    // Wait 27 clocks for RW turnaround
    anc_boot_put_link_command(anc, LINK_COMMAND__OPCODE(LINK_CMD_OP_WAIT_TIME) | 0x1b);
    anc_boot_put_link_command(anc, LINK_COMMAND__CMD1(NAND_CMD__READ));
    anc_boot_put_link_command(anc, LINK_COMMAND__OPCODE(LINK_CMD_OP_WAIT_TIME) | 0x1b);
    for (lba = 0; lba < num_lbas; lba++)
    {
        anc_boot_put_link_command(anc, LINK_COMMAND__READ_PIO(NAND_BOOT_BYTES_PER_META, false, false));
        if (lba == (num_lbas - 1))
        {
            anc_boot_put_link_command(anc, LINK_COMMAND__SEND_INTERRUPT(0, 0));
        }
        anc_boot_put_link_command(anc, LINK_COMMAND__READ_DMA(NAND_BOOT_LOGICAL_PAGE_SIZE, false, false));
        anc_boot_put_dma_command(anc, DMA_COMMAND_BUFDESC(NAND_BOOT_LOGICAL_PAGE_SIZE,
                                                          (uint64_t)paddr + lba * NAND_BOOT_LOGICAL_PAGE_SIZE));
    }

    anc_boot_put_dma_command(anc, DMA_COMMAND_FLAG(0, 0));

    intstatus = anc_boot_wait_interrupt(anc, intmask);
    if (!intstatus)
    {
        dprintf(DEBUG_INFO, "Timeout waiting for CHAN_INT_STATUS != 0 - this should never happen\n");
        return false;
    }

    // We've either timed out or gotten our status by now
    if (intstatus & M_ANC_CHAN_INT_STATUS_LINK_CMD_TIMEOUT)
    {
        // Timeout waiting for status: flush LINK CmdQ and return error
        dprintf(DEBUG_INFO, "Timeout waiting for NAND status\n");
        anc_wr(anc, R_ANC_LINK_CONTROL, M_ANC_LINK_CONTROL_RESET_CMDQ);
        return false;
    }
    else if (intstatus & M_ANC_CHAN_INT_STATUS_READ_STATUS_ERR_RESPONSE)
    {
        // Invalid status (probably has error bit set).  Reset CmdQ and return error
        dprintf(DEBUG_INFO, "Invalid NAND status 0x%02X\n", anc_rd(anc, R_ANC_LINK_NAND_STATUS));
        anc_wr(anc, R_ANC_LINK_CONTROL, M_ANC_LINK_CONTROL_RESET_CMDQ);

        return false;
    }

    // We've got a good status and metadata should be available now.
 
    for (lba = 0; lba < num_lbas; lba++)
    {
        uint32_t meta_index;
        for (meta_index = 0; meta_index < (NAND_BOOT_BYTES_PER_META/ sizeof(uint32_t)); meta_index++)
        {
            uint32_t meta_word = anc_rd(anc, R_ANC_CHAN_LINK_PIO_READ_FIFO);
            if ((meta_index == 0) && (meta_word != meta[0]))
            {
                dprintf(DEBUG_INFO, "Invalid meta: expected 0x%08X got 0x%08X\n", meta[0], meta_word);
                ret = false;
            }
        }
    }

#if APPLICATION_SECUREROM
    if (!anc_boot_wait_reg(anc, R_ANC_CHAN_INT_STATUS,
                           M_ANC_CHAN_INT_STATUS_DMA_CMD_FLAG,
                           M_ANC_CHAN_INT_STATUS_DMA_CMD_FLAG))
    {
        dprintf(DEBUG_INFO, "Timeout waiting for DMA to complete\n");
        return false;
    }
#else // APPLICATION_SECUREROM
    intstatus = anc_boot_wait_interrupt(anc, M_ANC_CHAN_INT_STATUS_DMA_CMD_FLAG | M_ANC_CHAN_INT_STATUS_DMA_CMD_TIMEOUT);
    if (!intstatus || (intstatus & M_ANC_CHAN_INT_STATUS_DMA_CMD_TIMEOUT))
    {
        dprintf(DEBUG_INFO, "Timeout waiting for DMA to complete - 0x%08x\n", intstatus);
        return false;
    }
#endif // APPLICATION_SECUREROM

    anc_wr(anc, R_ANC_CHAN_INT_STATUS, M_ANC_CHAN_INT_STATUS_DMA_CMD_FLAG);

#if WITH_NON_COHERENT_DMA
    platform_cache_operation(CACHE_INVALIDATE, data, NAND_BOOT_LOGICAL_PAGE_SIZE * num_lbas);
#endif

    return ret;
}

void anc_boot_put_link_command(anc_t *anc, uint32_t value)
{
#if APPLICATION_SECUREROM
    anc_boot_wait_reg(anc, R_ANC_CHAN_LINK_CMDQ_FIFO_STATUS, M_ANC_CHAN_LINK_CMDQ_FIFO_STATUS_FULL, 0);
    anc_wr(anc, R_ANC_CHAN_LINK_CMDQ_FIFO, value);
#else // APPLICATION_SECUREROM
    if (anc_boot_wait_reg_int_timeout(anc, R_ANC_CHAN_LINK_CMDQ_FIFO_STATUS, M_ANC_CHAN_LINK_CMDQ_FIFO_STATUS_FULL, 0, M_ANC_CHAN_INT_STATUS_LINK_CMD_TIMEOUT))
    {
        anc_wr(anc, R_ANC_CHAN_LINK_CMDQ_FIFO, value);
    }
    else
    {
        dprintf(DEBUG_INFO, "Timeout waiting for room in link command FIFO\n");
    }
#endif // APPLICATION_SECUREROM
}

void anc_boot_put_dma_command(anc_t *anc, uint64_t value)
{
#if APPLICATION_SECUREROM
    anc_boot_wait_reg(anc, R_ANC_CHAN_LINK_CMDQ_FIFO_STATUS, M_ANC_CHAN_DMA_CMDQ_FIFO_STATUS_FULL, 0);
    anc_wr(anc, R_ANC_CHAN_DMA_CMDQ_FIFO, value & 0xFFFFFFFFUL);
    anc_wr(anc, R_ANC_CHAN_DMA_CMDQ_FIFO, (value >> 32UL));
#else // APPLICATION_SECUREROM
    if (anc_boot_wait_reg_int_timeout(anc, R_ANC_CHAN_LINK_CMDQ_FIFO_STATUS, M_ANC_CHAN_DMA_CMDQ_FIFO_STATUS_FULL, 0, M_ANC_CHAN_INT_STATUS_DMA_CMD_TIMEOUT))
    {
        anc_wr(anc, R_ANC_CHAN_DMA_CMDQ_FIFO, value & 0xFFFFFFFFUL);
        anc_wr(anc, R_ANC_CHAN_DMA_CMDQ_FIFO, (value >> 32UL));
    }
    else
    {
        dprintf(DEBUG_INFO, "Timeout waiting for room in DMA command FIFO\n");
    }
#endif // APPLICATION_SECUREROM
}

// Wait up to ANC_TIMEOUT_MICROS for ((reg & mask) == value)
bool anc_boot_wait_reg(anc_t *anc, uint32_t reg, uint32_t mask, uint32_t value)
{
    utime_t  start_time = system_time();
    uint32_t regval;

    do
    {
        regval = anc_rd(anc, reg);
        if ((regval & mask) == value)
        {
            return true;
        }
    } while (!time_has_elapsed(start_time, ANC_TIMEOUT_MICROS));

    return false;
}

#if !APPLICATION_SECUREROM
// Like anc_boot_wait_reg, but waits until (R_ANC_CHAN_INT_MASK & tomask) to determine the timeout instead of using ANC_TIMEOUT_MICROS. Only used by LLB.
bool anc_boot_wait_reg_int_timeout(anc_t *anc, uint32_t reg, uint32_t mask, uint32_t value, uint32_t tomask)
{
    uint32_t regval, intval;

    do
    {
        regval = anc_rd(anc, reg);
        if ((regval & mask) == value)
        {
            return true;
        }
        intval = anc_rd(anc, R_ANC_CHAN_INT_STATUS);
    } while (!(intval & tomask));

    anc_wr(anc, R_ANC_CHAN_INT_STATUS, (intval & tomask));

    regval = anc_rd(anc, reg);
    if ((regval & mask) == value)
    {
        return true;
    }

    return false;
}
#endif // !APPLICATION_SECUREROM

uint32_t anc_boot_wait_interrupt(anc_t *anc, uint32_t mask)
{
#if APPLICATION_SECUREROM
    utime_t  start_time = system_time();
#endif // APPLICATION_SECUREROM
    uint32_t regval;

    do
    {
        regval = anc_rd(anc, R_ANC_CHAN_INT_STATUS);
        if (regval & mask)
        {
            // Only clear the bits we're looking for
            anc_wr(anc, R_ANC_CHAN_INT_STATUS, (regval & mask));
            return regval;
        }
#if APPLICATION_SECUREROM
    } while (!time_has_elapsed(start_time, ANC_TIMEOUT_MICROS));
#else // APPLICATION_SECUREROM
    } while (1);
#endif // APPLICATION_SECUREROM

    return 0;
}

static void anc_disable_uid(anc_t *anc)
{
    // The DISABLE_UID bit doesn't work unless AES is enabled. Force a DMA_CONFIG to enable it
    anc_boot_put_dma_command(anc, DMA_COMMAND_CONFIG(DMA_DIRECTION_N2M, true));

    // Wait for AES to become enabled
    anc_boot_wait_reg(anc, R_ANC_DMA_STATUS, M_ANC_DMA_STATUS_AES_ENABLED, M_ANC_DMA_STATUS_AES_ENABLED);
#ifdef M_ANC_DMA_UID_CONFIG_DISABLE_UID
    // Disable UID1 (doesn't exist on M7)
    anc_wr(anc, R_ANC_DMA_UID_CONFIG, M_ANC_DMA_UID_CONFIG_DISABLE_UID);
#endif
}

void anc_disable_uid_key(void)
{
    int i;

    if (!g_boot_anc[0].initialized)
    {
        // If we came in here through the DFU path, ANC isn't configured.
        anc_bootrom_init(false, ANC_BOOT_MODE_RESET_ALL_CONTROLLERS);
    }

    for (i = 0; i < ANC_BOOT_CONTROLLERS; i++)
    {
        anc_disable_uid(&g_boot_anc[i]);
    }
}

