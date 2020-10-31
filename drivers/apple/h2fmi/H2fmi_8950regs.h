// *****************************************************************************
//
// File: H2fmi_regs.h
//
// *****************************************************************************
//
// Notes:
//
//  - register bitfield definitions are only good for creating bitfield in
//    register position
//
//  - consider adding definitions for extracting value from register position
//
// *****************************************************************************
//
// Copyright (C) 2010 Apple Computer, Inc. All rights reserved.
//
// This document is the property of Apple Computer, Inc.
// It is considered confidential and proprietary.
//
// This document may not be reproduced or transmitted in any form,
// in whole or in part, without the express written permission of
// Apple Computer, Inc.
//
// *****************************************************************************

#include "WMRFeatures.h"
#if defined(H2FMI_EFI) && H2FMI_EFI
#include "SoC.h"
#else
#include <platform/soc/hwregbase.h>
#endif

#include <WMRTypes.h>

#ifndef _H2FMI_8945REGS_H_
#define _H2FMI_8945REGS_H_
// =============================================================================
// register bases and offsets
// =============================================================================

#define FMI0                      ((UInt32*)FMI0_BASE_ADDR)
#define FMI1                      ((UInt32*)FMI1_BASE_ADDR)

#define FMI_OFFSET                (0x00000)
#define FMC_OFFSET                (0x40000)
#define ECC_OFFSET                (0x80000)
#define CMD_OFFSET                (0xC0000)

#define FMC0                      ((volatile UInt32*)((UInt32)FMI0 + FMC_OFFSET))
#define FMC1                      ((volatile UInt32*)((UInt32)FMI1 + FMC_OFFSET))

#define FMI_BA(offset)            ((UInt32)(FMI_OFFSET + offset))
#define FMC_BA(offset)            ((UInt32)(FMC_OFFSET + offset))
#define ECC_BA(offset)            ((UInt32)(ECC_OFFSET + offset))
#define CMD_BA(offset)            ((UInt32)(CMD_OFFSET + offset))
#define NAND_DLL_REG(offset)      ((volatile UInt32*)(NAND_DLL_BASE_ADDR + (offset)))

#define FMI_CONFIG                FMI_BA(0x00)
#define FMI_CONTROL               FMI_BA(0x04)
#define FMI_STATUS                FMI_BA(0x08)
#define FMI_INT_PEND              FMI_BA(0x0C)
#define FMI_INT_EN                FMI_BA(0x10)
#define FMI_DATA_BUF              FMI_BA(0x14)
#define FMI_META_FIFO             FMI_BA(0x18)
#define FMI_DEBUG0                FMI_BA(0x1C)
#define FMI_HW_VERSION            FMI_BA(0x30)
#define FMI_DATA_SIZE             FMI_BA(0x34)
#define FMI_RAW_PAGE_SIZE         FMI_BA(0x38)
#define FMI_SCRAMBLER_SEED_FIFO   FMI_BA(0x3C)
#define FMI_SEED_FIFO_LOW_LEVEL   FMI_BA(0x40)
#define FMI_METADATA_FIFO_PTR     FMI_BA(0x44)
#define FMI_SEED_FIFO_PTR         FMI_BA(0x48)
#define FMI_RANDOMIZER_VERSION    FMI_BA(0x4C)

#define FMI_BUFFER_0_BASE          FMI_BA(0x1000)
#define FMI_BUFFER_1_BASE          FMI_BA(0x1800)
#define FMI_BUFFER_LEN             (0x45F)
#define FMI_META_FIFO_BASE         FMI_BA(0x2000)
#define FMI_META_FIFO_LENGTH       (32)
#define FMI_SEED_FIFO_BASE         FMI_BA(0x3000)
#define FMI_SEED_FIFO_LENGTH       (8)
#define FMI_ECC_RESULT_FIFO_BASE   FMI_BA(0x81000)
#define FMI_ECC_RESULT_FIFO_LENGTH (32)
#define FMI_ECC_PAGE_FIFO_BASE     FMI_BA(0xC1000)
#define FMI_ECC_PAGE_FIFO_LENGTH   (32)

#define FMC_ON                    FMC_BA(0x00)
#define FMC_VER                   FMC_BA(0x04)
#define FMC_IF_CTRL               FMC_BA(0x08)
#define FMC_CE_CTRL               FMC_BA(0x0C)
#define FMC_RW_CTRL               FMC_BA(0x10)
#define FMC_CMD                   FMC_BA(0x14)
#define FMC_ADDR0                 FMC_BA(0x18)
#define FMC_ADDR1                 FMC_BA(0x1C)
#define FMC_ADDRNUM               FMC_BA(0x20)
#define FMC_DATANUM               FMC_BA(0x24)
#define FMC_UNDEF0                FMC_BA(0x28)
#define FMC_UNDEF1                FMC_BA(0x2C)
#define FMC_TO_CTRL               FMC_BA(0x30)
#define FMC_TO_CNT                FMC_BA(0x34)
#define FMC_UNDEF2                FMC_BA(0x38)
#define FMC_UNDEF3                FMC_BA(0x3C)
#define FMC_INTMASK               FMC_BA(0x40)
#define FMC_STATUS                FMC_BA(0x44)
#define FMC_NAND_STATUS           FMC_BA(0x48)
#define FMC_RBB_CONFIG            FMC_BA(0x4C)
#define FMC_DQS_TIMING_CTRL       FMC_BA(0x70)
#define FMC_TOGGLE_CTRL_1         FMC_BA(0x74)
#define FMC_TOGGLE_CTRL_2         FMC_BA(0x78)
#define FMC_TOGGLE_CTRL_3         FMC_BA(0x7C)

#define ECC_VERSION               ECC_BA(0x00)
#define ECC_CON0                  ECC_BA(0x04)
#define ECC_CON1                  ECC_BA(0x08)
#define ECC_RESULT                ECC_BA(0x0C)
#define ECC_PND                   ECC_BA(0x10)
#define ECC_MASK                  ECC_BA(0x14)
#define ECC_HISTORY               ECC_BA(0x18)
#define ECC_CORE_SW_RESET         ECC_BA(0x1C)
#define ECC_PAGE_ERROR_FIFO       ECC_BA(0x24)
#define ECC_PAGE_ERROR_FIFO_PTR   ECC_BA(0x28)
#define ECC_RESULTS_FIFO_PTR      ECC_BA(0x2C)

#define COMMAND_FIFO              CMD_BA(0x00)
#define COMMAND_FIFO_PTR          CMD_BA(0x04)
#define COMMAND_FIFO_LOW          CMD_BA(0x08)
#define COMMAND_COUNT             CMD_BA(0x0C)
#define COMMAND_INT_CODE          CMD_BA(0x10)
#define OPERAND_FIFO              CMD_BA(0x14)
#define OPERAND_FIFO_PTR          CMD_BA(0x18)
#define OPERAND_FIFO_LOW          CMD_BA(0x1C)
#define SEQ_INT_PEND              CMD_BA(0x20)
#define SEQ_INT_EN                CMD_BA(0x24)
#define SEQ_MACRO_CONTROL         CMD_BA(0x28)
#define TIMEOUT_VALUE             CMD_BA(0x2C)
#define STORE_FIFO                CMD_BA(0x30)
#define STORE_FIFO_PTR            CMD_BA(0x34)
#define STORE_FIFO_HIGH           CMD_BA(0x38)

#define SEQUENCER_MACROS(n)       CMD_BA(0x1000 + ((n) & 0xFF))
#define SEQUENCER_COMMANDS(n)     CMD_BA(0x2000 + ((n) & 0x7F))
#define SEQUENCER_OPERANDS(n)     CMD_BA(0x3000 + ((n) & 0x7F))
#define SEQUENCER_STORES(n)       CMD_BA(0x4000 + ((n) & 0x7F))

#define NAND_DLL_CONTROL          NAND_DLL_REG(0x00)
#define NAND_DLL_TIMEOUT_DELAY    NAND_DLL_REG(0x04)
#define NAND_DLL_STATUS           NAND_DLL_REG(0x08)
#define NAND_DLL_FORCE_VALUE      NAND_DLL_REG(0x0C)

#define h2fmi_dll_wr(reg, val)    (*(reg) = (val))
#define h2fmi_dll_rd(reg)         (*(reg))

// =============================================================================
// FMI_CONFIG bitfields
// =============================================================================

#define FMI_CONFIG__DMA_BURST(n)                      (((n) & 0x7) << 0)
#define FMI_CONFIG__META_DMA_BURST_SIZE(n)            (((n) & 0x7) << 9)
#define FMI_CONFIG__META_DMA_BURST_WIDTH(n)           (((n) & 0x3) << 12)
#define FMI_CONFIG__ECC_CORRECTABLE_BITS(m)           (((m) & 0x1f) << 3)
#define FMI_CONFIG__SCRAMBLE_SEED                     (0x1UL << 14)
#define FMI_CONFIG__ENABLE_WHITENING                  (0x1UL << 8)

#define FMI_CONFIG__DMA_BURST__32_CYCLES  FMI_CONFIG__DMA_BURST(5)
#define FMI_CONFIG__DMA_BURST__16_CYCLES  FMI_CONFIG__DMA_BURST(4)
#define FMI_CONFIG__DMA_BURST__8_CYCLES   FMI_CONFIG__DMA_BURST(3)
#define FMI_CONFIG__DMA_BURST__4_CYCLES   FMI_CONFIG__DMA_BURST(2)
#define FMI_CONFIG__DMA_BURST__2_CYCLES   FMI_CONFIG__DMA_BURST(1)
#define FMI_CONFIG__DMA_BURST__1_CYCLES   FMI_CONFIG__DMA_BURST(0)

#define FMI_CONFIG__META_DMA_BURST__32_CYCLES  FMI_CONFIG__META_DMA_BURST_SIZE(5)
#define FMI_CONFIG__META_DMA_BURST__16_CYCLES  FMI_CONFIG__META_DMA_BURST_SIZE(4)
#define FMI_CONFIG__META_DMA_BURST__8_CYCLES   FMI_CONFIG__META_DMA_BURST_SIZE(3)
#define FMI_CONFIG__META_DMA_BURST__4_CYCLES   FMI_CONFIG__META_DMA_BURST_SIZE(2)
#define FMI_CONFIG__META_DMA_BURST__2_CYCLES   FMI_CONFIG__META_DMA_BURST_SIZE(1)
#define FMI_CONFIG__META_DMA_BURST__1_CYCLES   FMI_CONFIG__META_DMA_BURST_SIZE(0)

#define FMI_CONFIG__META_DMA_BURST__4_BYTES    FMI_CONFIG__META_DMA_BURST_WIDTH(2)
#define FMI_CONFIG__META_DMA_BURST__2_BYTES    FMI_CONFIG__META_DMA_BURST_WIDTH(1)
#define FMI_CONFIG__META_DMA_BURST__1_BYTES    FMI_CONFIG__META_DMA_BURST_WIDTH(0)

// =============================================================================
// FMI_CONTROL bitfields
// =============================================================================

#define FMI_CONTROL__PAUSE_WHEN_STORE_FIFO_FULL (0x1UL << 14)
#define FMI_CONTROL__DISABLE_STREAMING        (0x1UL << 13)
#define FMI_CONTROL__SEQUENCER_TIMEOUT_ENABLE (0x1UL << 12)
#define FMI_CONTROL__RESET_SEED               (0x1UL << 11)
#define FMI_CONTROL__RAW_READ_MODE            (0x1UL << 10)
#define FMI_CONTROL__RESET_SEQUENCER          (0x1UL << 9)
#define FMI_CONTROL__ENABLE_SEQUENCER         (0x1UL << 8)
#define FMI_CONTROL__ECC_PAUSE                (0x1UL << 7)
#define FMI_CONTROL__BUFFER_1_LOCK(m)         (((m) & 0x3) << 5)
#define FMI_CONTROL__BUFFER_0_LOCK(m)         (((m) & 0x3) << 3)
#define FMI_CONTROL__MODE(m)                  (((m) & 0x3) << 1)
#define FMI_CONTROL__START_BIT                (0x1UL << 0)

#define FMI_CONTROL__MODE__IDLE            FMI_CONTROL__MODE(0)
#define FMI_CONTROL__MODE__READ            FMI_CONTROL__MODE(1)
#define FMI_CONTROL__MODE__WRITE           FMI_CONTROL__MODE(2)
#define FMI_CONTROL__MODE__SOFTWARE_RESET  FMI_CONTROL__MODE(3)

#define FMI_CONTROL__BUFFER_0_LOCK_FREE    FMI_CONTROL__BUFFER_0_LOCK(0)
#define FMI_CONTROL__BUFFER_0_LOCK_DMA     FMI_CONTROL__BUFFER_0_LOCK(1)
#define FMI_CONTROL__BUFFER_0_LOCK_ECC     FMI_CONTROL__BUFFER_0_LOCK(2)
#define FMI_CONTROL__BUFFER_0_LOCK_FMC     FMI_CONTROL__BUFFER_0_LOCK(3)

#define FMI_CONTROL_BUFFER_1_LOCK_FREE     FMI_CONTROL__BUFFER_1_LOCK(0)
#define FMI_CONTROL_BUFFER_1_LOCK_DMA      FMI_CONTROL__BUFFER_1_LOCK(1)
#define FMI_CONTROL_BUFFER_1_LOCK_ECC      FMI_CONTROL__BUFFER_1_LOCK(2)
#define FMI_CONTROL_BUFFER_1_LOCK_FMC      FMI_CONTROL__BUFFER_1_LOCK(3)

// =============================================================================
// FMI_STATUS bitfields
// =============================================================================

#define FMI_STATUS__FMC_ACTIVE  (0x1UL << 3)
#define FMI_STATUS__ECC_ACTIVE  (0x1UL << 2)
#define FMI_STATUS__DMA_ACTIVE  (0x1UL << 1)
#define FMI_STATUS__BUS_ACTIVE  (0x1UL << 0)

// =============================================================================
// FMI_INT_PEND bitfields
// =============================================================================

#define FMI_INT_PEND__SEQUENCER_INTERRUPT        (0x1UL << 28)
#define FMI_INT_PEND__SEED_FIFO_EMPTY            (0x1UL << 27)
#define FMI_INT_PEND__SEED_FIFO_LOW              (0x1UL << 26)
#define FMI_INT_PEND__SEED_FIFO_OVERFLOW         (0x1UL << 25)
#define FMI_INT_PEND__ECC_RESULTS_FIFO_UNDERFLOW (0x1UL << 24)
#define FMI_INT_PEND__ECC_RESULTS_FIFO_OVERFLOW  (0x1UL << 23)
#define FMI_INT_PEND__LAST_ECC_DONE              (0x1UL << 22)
#define FMI_INT_PEND__ECC_INTERRUPT              (0x1UL << 21)
#define FMI_INT_PEND__ECC_READY_BUSY             (0x1UL << 20)
#define FMI_INT_PEND__ECC_DONE                   (0x1UL << 19)
#define FMI_INT_PEND__FMC_INTERRUPT              (0x1UL << 18)
#define FMI_INT_PEND__FMC_READY_BUSY(n)          (((n) & 0xFF) << 10)
#define FMI_INT_PEND__FMC_TIMEOUT                (0x1UL << 9)
#define FMI_INT_PEND__FMC_NSRBBDONE              (0x1UL << 8)
#define FMI_INT_PEND__FMC_NSERR                  (0x1UL << 7)
#define FMI_INT_PEND__FMC_TRANSDONE              (0x1UL << 6)
#define FMI_INT_PEND__FMC_ADDRESSDONE            (0x1UL << 5)
#define FMI_INT_PEND__FMC_CMDDONE                (0x1UL << 4)
#define FMI_INT_PEND__META_FIFO_OVERFLOW         (0x1UL << 3)
#define FMI_INT_PEND__META_FIFO_UNDERFLOW        (0x1UL << 2)
#define FMI_INT_PEND__LAST_FMC_DONE              (0x1UL << 1)
#define FMI_INT_PEND__TRANSACTION_COMPLETE       (0x1UL << 0)

// =============================================================================
// FMI_INT_EN bitfields
// =============================================================================
#define FMI_INT_EN__SEQUENCER_INTERRUPT        (0x1UL << 28)
#define FMI_INT_EN__SEED_FIFO_EMPTY            (0x1UL << 27)
#define FMI_INT_EN__SEED_FIFO_LOW              (0x1UL << 26)
#define FMI_INT_EN__SEED_FIFO_OVERFLOW         (0x1UL << 25)
#define FMI_INT_EN__ECC_RESULTS_FIFO_UNDERFLOW (0x1UL << 24)
#define FMI_INT_EN__ECC_RESULTS_FIFO_OVERFLOW  (0x1UL << 23)
#define FMI_INT_EN__LAST_ECC_DONE              (0x1UL << 22)
#define FMI_INT_EN__ECC_INTERRUPT              (0x1UL << 21)
#define FMI_INT_EN__ECC_READY_BUSY             (0x1UL << 20)
#define FMI_INT_EN__ECC_DONE                   (0x1UL << 19)
#define FMI_INT_EN__FMC_INTERRUPT              (0x1UL << 18)
#define FMI_INT_EN__FMC_READY_BUSY             (0x1UL << 10)
#define FMI_INT_EN__FMC_TIMEOUT                (0x1UL << 9)
#define FMI_INT_EN__FMC_NSRBBDONE              (0x1UL << 8)
#define FMI_INT_EN__FMC_NSERR                  (0x1UL << 7)
#define FMI_INT_EN__FMC_TRANSDONE              (0x1UL << 6)
#define FMI_INT_EN__FMC_ADDRESSDONE            (0x1UL << 5)
#define FMI_INT_EN__FMC_CMDDONE                (0x1UL << 4)
#define FMI_INT_EN__META_FIFO_OVERFLOW         (0x1UL << 3)
#define FMI_INT_EN__META_FIFO_UNDERFLOW        (0x1UL << 2)
#define FMI_INT_EN__LAST_FMC_DONE              (0x1UL << 1)
#define FMI_INT_EN__TRANSACTION_COMPLETE       (0x1UL << 0)

// =============================================================================
// FMI_DEBUG0 bitfields
// =============================================================================

#define FMI_DEBUG0__FMC_TASKS_PENDING(n)  (((n) & 0x3) << 7)
#define FMI_DEBUG0__ECC_TASKS_PENDING(n)  (((n) & 0x3) << 5)
#define FMI_DEBUG0__DMA_TASKS_PENDING(n)  (((n) & 0x3) << 3)
#define FMI_DEBUG0__FMC_TARGET_BUFFER(b)  (((b) & 0x1) << 2)
#define FMI_DEBUG0__ECC_TARGET_BUFFER(b)  (((b) & 0x1) << 1)
#define FMI_DEBUG0__DMA_TARGET_BUFFER(b)  (((b) & 0x1) << 0)

// =============================================================================
// FMI_DATA_SIZE bifields
// =============================================================================

#define FMI_DATA_SIZE__META_BYTES_PER_SECTOR(n)   (((n) & 0x3F) << 25)
#define FMI_DATA_SIZE__META_BYTES_PER_PAGE(n)     (((n) & 0x3F) << 19)
#define FMI_DATA_SIZE__BYTES_PER_SECTOR(n)        (((n) & 0x7FF) << 8)
#define FMI_DATA_SIZE__SECTORS_PER_PAGE(n)        (((n) & 0xFF) << 0)

// =============================================================================
// FMI_SEED_FIFO_LOW_LEVEL bitfields
// =============================================================================

#define FMI_SEED_FIFO_LOW_LEVEL__LEVEL(x) (((x) & 0x7) << 0)

// =============================================================================
// METADATA_FIFO_PTR bitfields
// =============================================================================

#define METADATA_FIFO_PTR__READ(regval)   (((regval) >> 16) & 0x1F)
#define METADATA_FIFO_PTR__WRITE(regval)  (((regval) >> 8) & 0x1F)
#define METADATA_FIFO_PTR__LEVEL(regval)  (((regval) >> 0) & 0x3F)

// =============================================================================
// SEED_FIFO_PTR bitfields
// =============================================================================

#define SEED_FIFO_PTR__READ(regval)   (((regval) >> 16) & 0x7)
#define SEED_FIFO_PTR__WRITE(regval)  (((regval) >> 8) & 0x7)
#define SEED_FIFO_PTR__LEVEL(regval)  (((regval) >> 0) & 0xF)

// =============================================================================
// FMC_ON bitfields
// =============================================================================

#define FMC_ON__SWRST                  (0x1UL << 8)
#define FMC_ON__ASYNC_DRIVE_DQS        (0x1UL << 6)
#define FMC_ON__ASYNC_CEB_AUTO_DISABLE (0x1UL << 5)
#define FMC_ON__DDR_NAND_TYPE_TOGGLE   (0x0UL << 3)
#define FMC_ON__DDR_NAND_TYPE_ONFI     (0x1UL << 3)
#define FMC_ON__DDR_ENABLE             (0x1UL << 2)
#define FMC_ON__CLKGATING_RDY          (0x1UL << 1)
#define FMC_ON__ENABLE                 (0x1UL << 0)
#define FMC_ON__DISABLE                (!FMC_ON__ENABLE)

// =============================================================================
// FMC_IF_CTRL bitfields
// =============================================================================

#define FMC_IF_CTRL__WPB          (0x1UL << 21)
#define FMC_IF_CTRL__RBBEN        (0x1UL << 20)
#define FMC_IF_CTRL__DCCYCLE(v)   (((v) & 0xF) << 16)
#define FMC_IF_CTRL__REB_SETUP(v) (((v) & 0xF) << 12)
#define FMC_IF_CTRL__REB_HOLD(v)  (((v) & 0xF) << 8)
#define FMC_IF_CTRL__WEB_SETUP(v) (((v) & 0xF) << 4)
#define FMC_IF_CTRL__WEB_HOLD(v)  (((v) & 0xF) << 0)
#define FMC_IF_CTRL__GET_DCCYCLE(v)     (((v) & 0xF0000) >> 16)
#define FMC_IF_CTRL__GET_REB_SETUP(v)   (((v) & 0x0F000) >> 12)
#define FMC_IF_CTRL__GET_REB_HOLD(v)    (((v) & 0x00F00) >> 8)
#define FMC_IF_CTRL__GET_WEB_SETUP(v)   (((v) & 0x000F0) >> 4)
#define FMC_IF_CTRL__GET_WEB_HOLD(v)    (((v) & 0x0000F) >> 0)

#define FMC_IF_CTRL__REB_SETUP_MASK     (0x0000000F << 12)

#define FMC_IF_CTRL__TIMING_MAX_CLOCKS  (0xFUL)

// =============================================================================
// FMC_CE_CTRL bitfields
// =============================================================================

#define FMC_CE_CTRL__CE_COUNT     (8)
#define FMC_CE_CTRL__CEB(n)       (0x1UL << ((n) & 0x7))

#define FMC_CE_CTRL__CEB7         FMC_CE_CTRL__CEB(7)
#define FMC_CE_CTRL__CEB6         FMC_CE_CTRL__CEB(6)
#define FMC_CE_CTRL__CEB5         FMC_CE_CTRL__CEB(5)
#define FMC_CE_CTRL__CEB4         FMC_CE_CTRL__CEB(4)
#define FMC_CE_CTRL__CEB3         FMC_CE_CTRL__CEB(3)
#define FMC_CE_CTRL__CEB2         FMC_CE_CTRL__CEB(2)
#define FMC_CE_CTRL__CEB1         FMC_CE_CTRL__CEB(1)
#define FMC_CE_CTRL__CEB0         FMC_CE_CTRL__CEB(0)

// =============================================================================
// FMC_RW_CTRL bitfields
// =============================================================================

#define FMC_RW_CTRL__NAND_TIMING_USER_TAT(x) (((x) & 0xFF) << 24)
#define FMC_RW_CTRL__CMD1_5_MODE             (0x1UL << 9)
#define FMC_RW_CTRL__WAIT_NSRBB_FLD          (0x1UL << 8)
#define FMC_RW_CTRL__RD_DONE_REB_CEB_HOLD    (0x1UL << 7)
#define FMC_RW_CTRL__REBHOLD                 (0x1UL << 6)
#define FMC_RW_CTRL__WR_MODE                 (0x1UL << 5)
#define FMC_RW_CTRL__RD_MODE                 (0x1UL << 4)
#define FMC_RW_CTRL__ADDR_MODE               (0x1UL << 3)
#define FMC_RW_CTRL__CMD3_MODE               (0x1UL << 2)
#define FMC_RW_CTRL__CMD2_MODE               (0x1UL << 1)
#define FMC_RW_CTRL__CMD1_MODE               (0x1UL << 0)

// =============================================================================
// FMC_CMD bitfields
// =============================================================================

#define FMC_CMD__CMD1_5(c)       (((c) & 0xFF) << 24)
#define FMC_CMD__CMD3(c)         (((c) & 0xFF) << 16)
#define FMC_CMD__CMD2(c)         (((c) & 0xFF) << 8)
#define FMC_CMD__CMD1(c)         (((c) & 0xFF) << 0)

// =============================================================================
// FMC_ADDR0 bitfields
// =============================================================================

#define FMC_ADDR0__SEQ3(a)       (((a) & 0xFF) << 24)
#define FMC_ADDR0__SEQ2(a)       (((a) & 0xFF) << 16)
#define FMC_ADDR0__SEQ1(a)       (((a) & 0xFF) << 8)
#define FMC_ADDR0__SEQ0(a)       (((a) & 0xFF) << 0)

// =============================================================================
// FMC_ADDR1 bitfields
// =============================================================================

#define FMC_ADDR1__SEQ7(a)       (((a) & 0xFF) << 24)
#define FMC_ADDR1__SEQ6(a)       (((a) & 0xFF) << 16)
#define FMC_ADDR1__SEQ5(a)       (((a) & 0xFF) << 8)
#define FMC_ADDR1__SEQ4(a)       (((a) & 0xFF) << 0)

// =============================================================================
// FMC_ADDRNUM bitfields
// =============================================================================

#define FMC_ADDRNUM__NUM(n)      (((n) & 0x7) << 0)

// =============================================================================
// FMC_DATANUM bitfields
// =============================================================================

#define FMC_DATANUM__NUM(n)      (((n) & 0x3FF) << 0)

// =============================================================================
// FMC_TO_CTRL bitfields
// =============================================================================

#define FMC_TO_CTRL__EN          (0x1UL << 12)
#define FMC_TO_CTRL__DIV(d)      (((d) & 0xF) << 8)
#define FMC_TO_CTRL__VAL(v)      (((v) & 0xFF) << 0)

// =============================================================================
// FMC_TO_CNT bitfields
// =============================================================================

#define FMC_TO_CNT__CNT(c)       (((c) & 0xFF) << 0)

// =============================================================================
// FMC_INTMASK bitfields
// =============================================================================

#define FMC_INTMASK__RBBDONE(n)  (0x1UL << (((n) & 0x7) + 8))

#define FMC_INTMASK__CMD1_5DONE    (0x1UL << 17)
#define FMC_INTMASK__RBBDONE7    FMC_INTMASK__RBBDONE(7)
#define FMC_INTMASK__RBBDONE6    FMC_INTMASK__RBBDONE(6)
#define FMC_INTMASK__RBBDONE5    FMC_INTMASK__RBBDONE(5)
#define FMC_INTMASK__RBBDONE4    FMC_INTMASK__RBBDONE(4)
#define FMC_INTMASK__RBBDONE3    FMC_INTMASK__RBBDONE(3)
#define FMC_INTMASK__RBBDONE2    FMC_INTMASK__RBBDONE(2)
#define FMC_INTMASK__RBBDONE1    FMC_INTMASK__RBBDONE(1)
#define FMC_INTMASK__RBBDONE0    FMC_INTMASK__RBBDONE(0)
#define FMC_INTMASK__TIMEOUT     (0x1UL << 7)
#define FMC_INTMASK__NSERR       (0x1UL << 6)
#define FMC_INTMASK__NSRBBDONE   (0x1UL << 5)
#define FMC_INTMASK__TRANSDONE   (0x1UL << 4)
#define FMC_INTMASK__ADDRESSDONE (0x1UL << 3)
#define FMC_INTMASK__CMD3DONE    (0x1UL << 2)
#define FMC_INTMASK__CMD2DONE    (0x1UL << 1)
#define FMC_INTMASK__CMD1DONE    (0x1UL << 0)

// =============================================================================
// FMC_STATUS bitfields
// =============================================================================

#define FMC_STATUS__RBBDONE(n)   (0x1UL << (((n) & 0x7) + 8))

#define FMC_STATUS__EMERGENCY2   (0x1UL << 21)
#define FMC_STATUS__EMERGENCY01  (0x1UL << 20)
#define FMC_STATUS__RDATADIRTY   (0x1UL << 16)
#define FMC_STATUS__RBBDONE7     FMC_STATUS__RBBDONE(7)
#define FMC_STATUS__RBBDONE6     FMC_STATUS__RBBDONE(6)
#define FMC_STATUS__RBBDONE5     FMC_STATUS__RBBDONE(5)
#define FMC_STATUS__RBBDONE4     FMC_STATUS__RBBDONE(4)
#define FMC_STATUS__RBBDONE3     FMC_STATUS__RBBDONE(3)
#define FMC_STATUS__RBBDONE2     FMC_STATUS__RBBDONE(2)
#define FMC_STATUS__RBBDONE1     FMC_STATUS__RBBDONE(1)
#define FMC_STATUS__RBBDONE0     FMC_STATUS__RBBDONE(0)
#define FMC_STATUS__TIMEOUT      (0x1UL << 7)
#define FMC_STATUS__NSERR        (0x1UL << 6)
#define FMC_STATUS__NSRBBDONE    (0x1UL << 5)
#define FMC_STATUS__TRANSDONE    (0x1UL << 4)
#define FMC_STATUS__ADDRESSDONE  (0x1UL << 3)
#define FMC_STATUS__CMD3DONE     (0x1UL << 2)
#define FMC_STATUS__CMD2DONE     (0x1UL << 1)
#define FMC_STATUS__CMD1DONE     (0x1UL << 0)

// =============================================================================
// FMC_RBB_CONFIG bitfields
// =============================================================================

#define FMC_RBB_CONFIG__POL(v)   (((v) & 0xFF) << 8)
#define FMC_RBB_CONFIG__POS(v)   (((v) & 0xFF) << 0)

// =============================================================================
// FMC_DQS_TIMING_CTRL bitfields
// =============================================================================

#define FMC_DQS_TIMING_CTRL__USE_DLL                       (0x1UL << 31)
#define FMC_DQS_TIMING_CTRL__DELTAV_SUSPENDS_READS         (0x1UL << 30)
#define FMC_DQS_TIMING_CTRL__DQS_READ_DELAY_CTRL(x)        (((x) & 0x1FF) << 16)
#define FMC_DQS_TIMING_CTRL__DQS_READ_DELAY_CTRL_GET(x)    (((x) & 0x01FF0000) >> 16)
#define FMC_DQS_TIMING_CTRL__DQS_WRITE_TIMING_DIR_POS      (0 << 3)
#define FMC_DQS_TIMING_CTRL__DQS_WRITE_TIMING_DIR_NEG      (1 << 3)
#define FMC_DQS_TIMING_CTRL__DQS_WRITE_DELAY_TIMING(x)     (((x) & 0x7) << 0)

#define FMC_DQS_TIMING_CTRL__DEFAULT_VAL                   (0x01FF0000)

// =============================================================================
// FMC_TOGGLE_CTRL_1 bitfields
// =============================================================================

#define FMC_TOGGLE_CTRL_1_DDR_RD_PRE_TIME(x)   (((x) & 0xFF) << 24)
#define FMC_TOGGLE_CTRL_1_DDR_RD_POST_TIME(x)  (((x) & 0xFF) << 16)
#define FMC_TOGGLE_CTRL_1_DDR_WR_PRE_TIME(x)   (((x) & 0xFF) << 8)
#define FMC_TOGGLE_CTRL_1_DDR_WR_POST_TIME(x)  (((x) & 0xFF) << 0)

#define FMC_TOGGLE_CTRL_1__TIMING_MAX_CLOCKS   (0xFF)

// =============================================================================
// FMC_TOGGLE_CTRL_2 bitfields
// =============================================================================

#define FMC_TOGGLE_CTRL_2_CE_SETUP_TIME(x)       (((x) & 0xFF) << 24)
#define FMC_TOGGLE_CTRL_2_CE_HOLD_TIME(x)        (((x) & 0xFF) << 16)
#define FMC_TOGGLE_CTRL_2_NAND_TIMING_ADL(x)     (((x) & 0xFF) << 8)
#define FMC_TOGGLE_CTRL_2_NAND_TIMING_WHR(x)     (((x) & 0xFF) << 0)

#define FMC_TOGGLE_CTRL_2__TIMING_MAX_CLOCKS     (0xFF)

// =============================================================================
// FMC_TOGGLE_CTRL_2 bitfields
// =============================================================================

#define FMC_TOGGLE_CTRL_3_ONFI_WR_CLK_STOP_EN                   (0x1UL << 31)
#define FMC_TOGGLE_CTRL_3_DISABLE_ONFI_READ_STATUS_CLE_ALE_HOLD (0x1UL << 30)
#define FMC_TOGGLE_CTRL_3_DISABLE_ONFI_ONFI_IDLE_CLOCK_ON       (0x1UL << 29)
#define FMC_TOGGLE_CTRL_3_ONFI_TIMING_CAD                       (((x) & 0xF) << 0)

// =============================================================================
// ECC_CON0 bitfields
// =============================================================================

#define ECC_CON0__INFORMATION_LENGTH(n)      (((n) & 0x7FF) << 16)
#define ECC_CON0__ECC_CORRECTION_MODE(m)     (((m) & 0x1F) << 4)
#define ECC_CON0__START_ENC                  (0x1UL << 1)
#define ECC_CON0__START_DEC                  (0x1UL << 0)

// =============================================================================
// ECC_CON1 bitfields
// =============================================================================
#define ECC_CON1__IMPLICIT_WRITE_MARK         (1UL << 31)
#define ECC_CON1__SLAVE_MODE                  (1UL << 29)
#define ECC_CON1__SKIP_FREE_PAGE_FLD          (1UL << 25)
#define ECC_CON1__ALLOWED_STUCK_BIT_IN_FP(n)  (((n) & 0x1F) << 16)
#define ECC_CON1__ERROR_ALERT_LEVEL(n)        (((n) & 0x1F) << 8)
#define ECC_CON1__INT_ENABLE(b)               (((b) & 0x1) << 7)

#define ECC_CON1__INT_ENABLE__ENABLE    ECC_CON1__INT_ENABLE(1)
#define ECC_CON1__INT_ENABLE__DISABLE   ECC_CON1__INT_ENABLE(0)

// =============================================================================
// ECC_RESULT bitfields
// =============================================================================

#define ECC_RESULT__ERROR_ALERT         (0x1UL << 21)
#define ECC_RESULT__ERROR_CNT(n)        (((n) & 0x1F) << 16)
#define ECC_RESULT__ERROR_CNT_SHIFT     (16)
#define ECC_RESULT__ERROR_CNT_MASK      (0x1FUL << ECC_RESULT__ERROR_CNT_SHIFT)
#define ECC_RESULT__STUCK_BIT_EXCEEDED  (0x1UL << 14)
#define ECC_RESULT__STUCK_BIT_CNT(n)    (((n) & 0x3F) << 8)
#define ECC_RESULT__DECODED_BUFFER_IDX  (0x1UL << 6)
#define ECC_RESULT__ALL_FF              (0x1UL << 3)
#define ECC_RESULT__ALL_ZERO            (0x1UL << 2)
#define ECC_RESULT__FREE_PAGE           (0x1UL << 1)
#define ECC_RESULT__UNCORRECTABLE       (0x1UL << 0)
#define ECC_RESULT__FIFO_RESET          (0x1UL << 0) // Same as uncorrectable bit

// =============================================================================
// ECC_PND bitfields
// =============================================================================

#define ECC_PND__MAX_STUCK_BIT_CNT(n)      (((n) & 0x3F) << 24)
#define ECC_PND__MAX_ERROR_CNT(n)          (((n) & 0x1F) << 16)
#define ECC_PND__MAX_ERROR_CNT_SHIFT       (16)
#define ECC_PND__MAX_ERROR_CNT_MASK        (0x1FUL << ECC_PND__MAX_ERROR_CNT_SHIFT)
#define ECC_PND__DETECTOR_DONE             (0x1UL << 14)
#define ECC_PND__B1_DEC_DONE               (0x1UL << 13)
#define ECC_PND__B0_DEC_DONE               (0x1UL << 12)
#define ECC_PND__SOME_STUCK_BIT_EXCEEDED   (0x1UL << 9)
#define ECC_PND__SOME_ALL_FF               (0x1UL << 8)
#define ECC_PND__SOME_ALL_ZERO             (0x1UL << 7)
#define ECC_PND__ALL_FREE_PAGE             (0x1UL << 6)
#define ECC_PND__SOME_ERROR_ALERT          (0x1UL << 5)
#define ECC_PND__READY_BUSY                (0x1UL << 4)
#define ECC_PND__ANY_UNCORRECTABLE         (0x1UL << 3)
#define ECC_PND__ENC_DONE                  (0x1UL << 2)
#define ECC_PND__DEC_DONE                  (0x1UL << 1)
#define ECC_PND__ENC_DEC_DONE              (0x1UL << 0)

// =============================================================================
// ECC_MASK bitfields
// =============================================================================

#define ECC_MASK__SOME_STUCK_BIT_EXCEEDED    (0x1UL << 9)
#define ECC_MASK__SOME_ALL_FF_MASK           (0x1UL << 8)
#define ECC_MASK__SOME_ALL_ZERO_MASK         (0x1UL << 7)
#define ECC_MASK__SOME_ERROR_ALERT_MASK      (0x1UL << 5)
#define ECC_MASK__ANY_UNCORRECTABLE_MASK     (0x1UL << 3)
#define ECC_MASK__ENC_DONE_MASK              (0x1UL << 2)
#define ECC_MASK__DEC_DONE_MASK              (0x1UL << 1)
#define ECC_MASK__ENC_DEC_DONE_MASK          (0x1UL << 0)

// =============================================================================
// ECC_HISTORY bitfields
// =============================================================================

#define ECC_HISTORY__ERROR_COUNT_CLEAR          (0x1UL << 26)
#define ECC_HISTORY__ERROR_COUNT(n)             (((n) & 0x7FFF) << 11)
#define ECC_HISTORY__DECODING_NUMBER_CLEAR      (0x1UL << 10)
#define ECC_HISTORY__DECODING_NUMBER(n)         (((n) & 0x1FF) << 0)

// =============================================================================
// RESULT_FIFO_PTR bitfields
// =============================================================================

#define ECC_RESULTS_FIFO_PTR__READ(regval)   (((regval) >> 16) & 0x1F)
#define ECC_RESULTS_FIFO_PTR__WRITE(regval)  (((regval) >> 8) & 0x1F)
#define ECC_RESULTS_FIFO_PTR__LEVEL(regval)  (((regval) >> 0) & 0x3F)

// =============================================================================
// COMMAND_FIFO bitfields
// =============================================================================

#define COMMAND_FIFO_SIZE     (32)
#define OPERAND_FIFO_SIZE     (32)
#define STORE_FIFO_SIZE       (32)

#define COMMAND_FIFO__COMMAND(cmd)          ((cmd) << 24)
#define COMMAND_FIFO__OPERAND(op)           (((op) & 0xFFFFFF) << 0)

#define OPCODE_ADDR(n)        (n)
#define OPCODE_COMMAND        (0x08)
#define OPCODE_ENABLE_CE      (0x09)
#define OPCODE_WAIT_FOR_READY (0x0A)
#define OPCODE_TX_PAGE        (0x0B)
#define OPCODE_SEND_INTERRUPT (0x0C)
#define OPCODE_PAUSE          (0x0D)
#define OPCODE_TIMED_WAIT     (0x0E)
#define OPCODE_LOAD_NEXT_WORD (0x0F)
#define OPCODE_LOAD_FROM_FIFO (0x10)
#define OPCODE_MACRO          (0x11)
#define OPCODE_POLL           (0x12)
#define OPCODE_WAIT_FOR_INT   (0x13)
#define OPCODE_STORE_TO_FIFO  (0x14)

#define CMD_ADDRESS(bytes, addr)           (COMMAND_FIFO__COMMAND(OPCODE_ADDR(bytes)) | \
                                            COMMAND_FIFO__OPERAND((addr) & 0xFFFFFF))

#define CMD_COMMAND(cmd)                   (COMMAND_FIFO__COMMAND(OPCODE_COMMAND) | \
                                            COMMAND_FIFO__OPERAND((cmd) & 0xFF))
                                            
#define CMD_ENABLE_CHIP(mask)              (COMMAND_FIFO__COMMAND(OPCODE_ENABLE_CE) | \
                                            COMMAND_FIFO__OPERAND((mask) & 0xFF))
                                            
#define CMD_WAIT_FOR_READY(code,mask,cond) (COMMAND_FIFO__COMMAND(OPCODE_WAIT_FOR_READY) | \
                                            COMMAND_FIFO__OPERAND((code) << 16 | (mask) << 8 | (cond) << 0))
                                            
#define CMD_TX_PAGE                        (COMMAND_FIFO__COMMAND(OPCODE_TX_PAGE))

#define CMD_SEND_INTERRUPT(code)           (COMMAND_FIFO__COMMAND(OPCODE_SEND_INTERRUPT) | \
                                            COMMAND_FIFO__OPERAND(code))
                                            
#define CMD_PAUSE                          (COMMAND_FIFO__COMMAND(OPCODE_PAUSE))

#define CMD_TIMED_WAIT(clks)               (COMMAND_FIFO__COMMAND(OPCODE_TIMED_WAIT) | \
                                            COMMAND_FIFO__OPERAND(clks))
                                            
#define CMD_LOAD_NEXT_WORD(addr)           (COMMAND_FIFO__COMMAND(OPCODE_LOAD_NEXT_WORD) | \
                                            COMMAND_FIFO__OPERAND((addr) & 0xFFFFF))
                                            
#define CMD_LOAD_FROM_FIFO(addr)           (COMMAND_FIFO__COMMAND(OPCODE_LOAD_FROM_FIFO) | \
                                            COMMAND_FIFO__OPERAND((addr) & 0xFFFFF))
                                            
#define CMD_MACRO(seq,len)                 (COMMAND_FIFO__COMMAND(OPCODE_MACRO) | \
                                            COMMAND_FIFO__OPERAND(((seq) << 8) | (len)))

#define CMD_POLL(addr)                     (COMMAND_FIFO__COMMAND(OPCODE_POLL) | \
                                            COMMAND_FIFO__OPERAND((addr) & 0xFFFFF))
                                            
#define CMD_WAIT_FOR_INT(irq,bit)          (COMMAND_FIFO__COMMAND(OPCODE_WAIT_FOR_INT) | \
                                            COMMAND_FIFO__OPERAND((irq) << 8 | (bit) << 0))

#define CMD_STORE_TO_FIFO(reg)             (COMMAND_FIFO__COMMAND(OPCODE_STORE_TO_FIFO) | \
                                            COMMAND_FIFO__OPERAND((reg) & 0xFFFFF))


// =============================================================================
// COMMAND_FIFO_PTR bitfields
// =============================================================================

#define COMMAND_FIFO_PTR__READ(regval)   (((regval) >> 16) & 0x1F)
#define COMMAND_FIFO_PTR__WRITE(regval)  (((regval) >> 8) & 0x1F)
#define COMMAND_FIFO_PTR__LEVEL(regval)  (((regval) >> 0) & 0x3F)

// =============================================================================
// COMMAND_FIFO_LOW bitfields
// =============================================================================

#define COMMAND_FIFO_LOW__LEVEL(x)  (((x) & 0x3F) << 0)

// =============================================================================
// COMMAND_COUNT bitfields
// =============================================================================

#define COMMAND_COUNT__MAX_COUNT      (32)

// =============================================================================
// COMMAND_INT_CODE bitfields
// =============================================================================

#define COMMAND_INT_CODE__READ(regval) ((regval) & 0xFFFFFF)
#define COMMAND_INT_CODE__WAIT_FOR_READY_CODE(regval)   ((regval >> 16) & 0xFF)
#define COMMAND_INT_CODE__WAIT_FOR_READY_STATUS(regval) ((regval >> 0) & 0xFF)

// =============================================================================
// OPERAND_FIFO_PTR bitfields
// =============================================================================

#define OPERAND_FIFO_PTR__READ(regval)   (((regval) >> 16) & 0x1F)
#define OPERAND_FIFO_PTR__WRITE(regval)  (((regval) >> 8) & 0x1F)
#define OPERAND_FIFO_PTR__LEVEL(regval)  (((regval) >> 0) & 0x3F)

// =============================================================================
// OPERAND_FIFO_LOW bitfields
// =============================================================================

#define OPERAND_FIFO_LOW__LEVEL(x)  (((x) & 0x3F) << 0)

// =============================================================================
// STORE_FIFO_PTR bitfields
// =============================================================================

#define STORE_FIFO_PTR__READ(regval)   (((regval) >> 16) & 0x1F)
#define STORE_FIFO_PTR__WRITE(regval)  (((regval) >> 8) & 0x1F)
#define STORE_FIFO_PTR__LEVEL(regval)  (((regval) >> 0) & 0x3F)

// =============================================================================
// STORE_FIFO_HIGH bitfields
// =============================================================================

#define STORE_FIFO_HIGH__LEVEL(x)      (((x) & 0x3F) << 0)

// =============================================================================
// SEQ_INT_PEND bitfields
// =============================================================================

#define SEQ_INT_PEND__STORE_FIFO_FULL       (0x1UL << 10)
#define SEQ_INT_PEND__STORE_FIFO_HIGH       (0x1UL << 9)
#define SEQ_INT_PEND__STORE_FIFO_OVERFLOW   (0x1UL << 8)
#define SEQ_INT_PEND__TIMEOUT               (0x1UL << 7)
#define SEQ_INT_PEND__OPERAND_FIFO_EMPTY    (0x1UL << 6)
#define SEQ_INT_PEND__OPERAND_FIFO_LOW      (0x1UL << 5)
#define SEQ_INT_PEND__OPERAND_FIFO_OVERFLOW (0x1UL << 4)
#define SEQ_INT_PEND__COMMAND_FIFO_EMPTY    (0x1UL << 3)
#define SEQ_INT_PEND__COMMAND_FIFO_LOW      (0x1UL << 2)
#define SEQ_INT_PEND__COMMAND_FIFO_OVERFLOW (0x1UL << 1)
#define SEQ_INT_PEND__SEQUENCER_SIGNAL      (0x1UL << 0)

// =============================================================================
// SEQ_INT_EN bitfields
// =============================================================================

#define SEQ_INT_EN__TIMEOUT               (0x1UL << 7)
#define SEQ_INT_EN__OPERAND_FIFO_EMPTY    (0x1UL << 6)
#define SEQ_INT_EN__OPERAND_FIFO_LOW      (0x1UL << 5)
#define SEQ_INT_EN__OPERAND_FIFO_OVERFLOW (0x1UL << 4)
#define SEQ_INT_EN__COMMAND_FIFO_EMPTY    (0x1UL << 3)
#define SEQ_INT_EN__COMMAND_FIFO_LOW      (0x1UL << 2)
#define SEQ_INT_EN__COMMAND_FIFO_OVERFLOW (0x1UL << 1)
#define SEQ_INT_EN__SEQUENCER_SIGNAL      (0x1UL << 0)

// =============================================================================
// SEQ_MACRO_CONTROL bitfields
// =============================================================================

#define SEQ_MACRO_CONTROL__WORD_COUNT(cnt)     (((cnt) & 0x3F) << 8)
#define SEQ_MACRO_CONTROL__MACRO_ADDRESS(addr) (((addr) & 0x3F) << 0)
#define SEQ_MACRO_CONTROL__GET_WORD_COUNT(regval)    (((regval) >> 8) & 0x3F)
#define SEQ_MACRO_CONTROL__GET_MACRO_ADDRESS(regval) (((regval) >> 0) & 0x3F)

// =============================================================================
// NAND_DLL_CONTROL bitfields
// =============================================================================

#define NAND_DLL_CONTROL__REFERENCE(x)           (((x) & 0xF) << 24)
#define NAND_DLL_CONTROL__STEP_SIZE(x)           (((x) & 0xFF) << 16)
#define NAND_DLL_CONTROL__START_POINT(x)         (((x) & 0xFF) << 8)
#define NAND_DLL_CONTROL__HALF                   (1 << 7)
#define NAND_DLL_CONTROL__OVERRIDE               (1 << 5)
#define NAND_DLL_CONTROL__CHANNEL_ACTIVE_START   (1 << 4)
#define NAND_DLL_CONTROL__VOLTAGE_SHIFT_START    (1 << 3)
#define NAND_DLL_CONTROL__TIMEOUT_START          (1 << 2)
#define NAND_DLL_CONTROL__SOFTWARE_START         (1 << 1)
#define NAND_DLL_CONTROL__ENABLE                 (1 << 0)

#define NAND_DLL_CONTROL__REFERENCE_DEFAULT      (0x8)
#define NAND_DLL_CONTROL__STEP_SIZE_DEFAULT      (0x01)
#define NAND_DLL_CONTROL__START_POINT_DEFAULT    (0x10)

#define NAND_DLL_TRAIN_TIME_US                   (1)

// =============================================================================
// NAND_DLL_TIMEOUT bitfields
// =============================================================================

// =============================================================================
// NAND_DLL_STATUS bitfields
// =============================================================================

#define NAND_DLL_STATUS__LOCK_VALUE_GET(x)          (((x) >> 3) & 0x3FF)
#define NAND_DLL_STATUS__COARSE_LOCK_ON_GET(x)      (((x) >> 2) & 0x1)
#define NAND_DLL_STATUS__FINE_LOCK_ON_GET(x)        (((x) >> 1) & 0x1)
#define NAND_DLL_STATUS__DLL_LOCKED_GET(x)          (((x) >> 0) & 0x1)

// =============================================================================
// NAND_DLL_FORCE_VALUE bitfields
// =============================================================================

// =============================================================================
// SoC-Specific Tuning
// =============================================================================

#define FMI_TX_PROP_DELAY_NS                  (6)
#define FMI_RX_PROP_DELAY_NS                  (6)

#endif // _H2FMI_8945REGS_H_

// ********************************** EOF *************************************
