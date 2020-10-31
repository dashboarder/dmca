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

#ifndef _H2FMI_REGS_H_
#define _H2FMI_REGS_H_

#include <sys/types.h>

#include <platform/soc/hwregbase.h>

// =============================================================================
// preprocessor compilation control
// =============================================================================

// Change the constant below to true if you want to see all FMI
// subsystem register writes; note that doing so is likely to change
// timing and therefore behavior, so it should be a rare case that you
// would ever want to do this.
#define H2FMI_TRACE_REG_WRITES false


// =============================================================================
// register access macros
// =============================================================================

#define h2fmi_wr(fmi, reg, val)  ((fmi)->regs[(reg) >> 2] = (val))
#define h2fmi_rd(fmi, reg)  ((fmi)->regs[(reg) >> 2])
#define h2fmi_rd8(fmi, reg)  (((volatile uint8_t*)((fmi)->regs))[reg])


// =============================================================================
// register bases and offsets
// =============================================================================

#define FMI0                      ((volatile uint32_t*)FMI0_BASE_ADDR)
#define FMI1                      ((volatile uint32_t*)FMI1_BASE_ADDR)

#if FMI_VERSION == 0

#define FMI_BA(offset)            ((uint32_t)(0x000 + offset))
#define FMC_BA(offset)            ((uint32_t)(0x400 + offset))
#define ECC_BA(offset)            ((uint32_t)(0x800 + offset))

#else

#define FMI_BA(offset)            ((uint32_t)(0x00000 + offset))
#define FMC_BA(offset)            ((uint32_t)(0x40000 + offset))
#define ECC_BA(offset)            ((uint32_t)(0x80000 + offset))

#endif

#define FMI_CONFIG                FMI_BA(0x00)
#define FMI_CONTROL               FMI_BA(0x04)
#define FMI_STATUS                FMI_BA(0x08)
#define FMI_INT_PEND              FMI_BA(0x0C)
#define FMI_INT_EN                FMI_BA(0x10)
#define FMI_DATA_BUF              FMI_BA(0x14)
#define FMI_META_FIFO             FMI_BA(0x18)
#define FMI_DEBUG0                FMI_BA(0x1C)
#define FMI_DEBUG1                FMI_BA(0x20)
#define FMI_DEBUG2                FMI_BA(0x24)
#if FMI_VERSION > 2
#define FMI_SCRAMBLER_SEED_FIFO   FMI_BA(0x3C)
#endif

#if FMI_VERSION > 0

#define FMI_DEBUG3                FMI_BA(0x28)
#define FMI_HW_VERSION            FMI_BA(0x30)
#define FMI_DATA_SIZE             FMI_BA(0x34)

#endif

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
#define FMC_LOCK0_L               FMC_BA(0x50)
#define FMC_LOCK0_H               FMC_BA(0x54)
#define FMC_LOCK1_L               FMC_BA(0x58)
#define FMC_LOCK1_H               FMC_BA(0x5C)
#define FMC_LOCK2_L               FMC_BA(0x60)
#define FMC_LOCK2_H               FMC_BA(0x64)
#define FMC_ALARMCMD              FMC_BA(0x68)
#define FMC_REJECTCMD             FMC_BA(0x6C)

#define ECC_VERSION               ECC_BA(0x00)
#define ECC_CON0                  ECC_BA(0x04)
#define ECC_CON1                  ECC_BA(0x08)
#define ECC_RESULT                ECC_BA(0x0C)
#define ECC_PND                   ECC_BA(0x10)
#define ECC_MASK                  ECC_BA(0x14)
#define ECC_HISTORY               ECC_BA(0x18)
#define ECC_CORE_SW_RESET         ECC_BA(0x1C)
#define ECC_LOC0                  ECC_BA(0x20)
#define ECC_LOC1                  ECC_BA(0x24)
#define ECC_LOC2                  ECC_BA(0x28)
#define ECC_LOC3                  ECC_BA(0x2C)
#define ECC_LOC4                  ECC_BA(0x30)
#define ECC_LOC5                  ECC_BA(0x34)
#define ECC_LOC6                  ECC_BA(0x38)
#define ECC_UNDEF0                ECC_BA(0x3C)
#define ECC_PARITY0               ECC_BA(0x40)
#define ECC_PARITY1               ECC_BA(0x44)
#define ECC_PARITY2               ECC_BA(0x48)
#define ECC_PARITY3               ECC_BA(0x4C)
#define ECC_PARITY4               ECC_BA(0x50)
#define ECC_PARITY5               ECC_BA(0x54)
#define ECC_PARITY6               ECC_BA(0x58)


// =============================================================================
// FMI_CONFIG bitfields
// =============================================================================

#if FMI_VERSION == 0

#define FMI_CONFIG__DMA_BURST(n)          (((n) & 0x7) << 17)
#define FMI_CONFIG__LBA_MODE(v)           (((v) & 0x3) << 15)
#define FMI_CONFIG__ECC_MODE(m)           (((m) & 0x1) << 14)
#define FMI_CONFIG__META_PER_ENVELOPE(n)  (((n) & 0x3F) << 8)
#define FMI_CONFIG__META_PER_PAGE(n)      (((n) & 0x3F) << 2)
#define FMI_CONFIG__PAGE_SIZE(n)          (((n) & 0x03) << 0)

#else

#define FMI_CONFIG__ECC_CORRECTABLE_BITS(m) (((m) & 0x1F) << 3)
#define FMI_CONFIG__DMA_BURST(n)            (((n) & 0x7) << 0)

#endif

#define FMI_CONFIG__DMA_BURST__32_CYCLES  FMI_CONFIG__DMA_BURST(5)
#define FMI_CONFIG__DMA_BURST__16_CYCLES  FMI_CONFIG__DMA_BURST(4)
#define FMI_CONFIG__DMA_BURST__8_CYCLES   FMI_CONFIG__DMA_BURST(3)
#define FMI_CONFIG__DMA_BURST__4_CYCLES   FMI_CONFIG__DMA_BURST(2)
#define FMI_CONFIG__DMA_BURST__2_CYCLES   FMI_CONFIG__DMA_BURST(1)
#define FMI_CONFIG__DMA_BURST__1_CYCLES   FMI_CONFIG__DMA_BURST(0)

#if FMI_VERSION == 0

#define FMI_CONFIG__LBA_MODE__NORMAL      FMI_CONFIG__LBA_MODE(0)
#define FMI_CONFIG__LBA_MODE__BYPASS_ECC  FMI_CONFIG__LBA_MODE(1)
#define FMI_CONFIG__LBA_MODE__LBA_TYPE_AB FMI_CONFIG__LBA_MODE(2)
#define FMI_CONFIG__LBA_MODE__LBA_TYPE_C  FMI_CONFIG__LBA_MODE(3)

#define FMI_CONFIG__ECC_MODE__16BIT       FMI_CONFIG__ECC_MODE(1)
#define FMI_CONFIG__ECC_MODE__8BIT        FMI_CONFIG__ECC_MODE(0)

#define FMI_CONFIG__PAGE_SIZE__2K         FMI_CONFIG__PAGE_SIZE(0)
#define FMI_CONFIG__PAGE_SIZE__4K         FMI_CONFIG__PAGE_SIZE(1)
#define FMI_CONFIG__PAGE_SIZE__8K         FMI_CONFIG__PAGE_SIZE(2)
#define FMI_CONFIG__PAGE_SIZE__512        FMI_CONFIG__PAGE_SIZE(3)

#else

#define FMI_CONFIG__ECC_MAX_CORRECTION    FMI_CONFIG__ECC_CORRECTABLE_BITS(30)

#endif

#if FMI_VERSION > 2
#define FMI_CONFIG__ENABLE_WHITENING                  (0x1UL << 8)
#endif

#if FMI_VERSION > 3
#define FMI_CONFIG__SCRAMBLE_SEED                     (0x1UL << 14)
#endif

// =============================================================================
// FMI_CONTROL bitfields
// =============================================================================

#if FMI_VERSION == 0

#define FMI_CONTROL__MODE(m)       (((m) & 0x3) << 1)
#define FMI_CONTROL__START_BIT     (0x1UL << 0)

#define FMI_CONTROL__MODE__IDLE   FMI_CONTROL__MODE(0)
#define FMI_CONTROL__MODE__READ   FMI_CONTROL__MODE(1)
#define FMI_CONTROL__MODE__WRITE  FMI_CONTROL__MODE(2)
#define FMI_CONTROL__MODE__UNDEF  FMI_CONTROL__MODE(3)

#else

#if FMI_VERSION > 3
#define FMI_CONTROL__RESET_SEED        (0x1UL << 11)
#endif

#define FMI_CONTROL__BUFFER_1_LOCK(m)  (((m) & 0x3) << 5)
#define FMI_CONTROL__BUFFER_0_LOCK(m)  (((m) & 0x3) << 3)
#define FMI_CONTROL__MODE(m)           (((m) & 0x3) << 1)
#define FMI_CONTROL__START_BIT         (0x1UL << 0)

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

#endif


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


#if FMI_VERSION == 0

#define FMI_INT_PEND__ECC_INTERRUPT         (0x1UL << 14)
#define FMI_INT_PEND__ECC_READY_BUSY        (0x1UL << 13)
#define FMI_INT_PEND__ECC_DONE              (0x1UL << 12)
#define FMI_INT_PEND__FMC_INTERRUPT         (0x1UL << 11)
#define FMI_INT_PEND__FMC_READY_BUSY        (0x1UL << 10)

#else

#define FMI_INT_PEND__ECC_INTERRUPT         (0x1UL << 21)
#define FMI_INT_PEND__ECC_READY_BUSY        (0x1UL << 20)
#define FMI_INT_PEND__ECC_DONE              (0x1UL << 19)
#define FMI_INT_PEND__FMC_INTERRUPT         (0x1UL << 18)
#define FMI_INT_PEND__FMC_READY_BUSY(n)     (((n) & 0xFF << 10)

#endif

#define FMI_INT_PEND__FMC_TIMEOUT           (0x1UL << 9)
#define FMI_INT_PEND__FMC_NSRBBDONE         (0x1UL << 8)
#define FMI_INT_PEND__FMC_NSERR             (0x1UL << 7)
#define FMI_INT_PEND__FMC_TRANSDONE         (0x1UL << 6)
#define FMI_INT_PEND__FMC_ADDRESSDONE       (0x1UL << 5)
#define FMI_INT_PEND__FMC_CMDDONE           (0x1UL << 4)
#define FMI_INT_PEND__META_FIFO_OVERFLOW    (0x1UL << 3)
#define FMI_INT_PEND__META_FIFO_UNDERFLOW   (0x1UL << 2)
#define FMI_INT_PEND__LAST_FMC_DONE         (0x1UL << 1)
#define FMI_INT_PEND__TRANSACTION_COMPLETE  (0x1UL << 0)



// =============================================================================
// FMI_INT_EN bitfields
// =============================================================================

#define FMI_INT_EN__ECC_INTERRUPT         (0x1UL << 14)
#define FMI_INT_EN__ECC_READY_BUSY        (0x1UL << 13)
#define FMI_INT_EN__ECC_DONE              (0x1UL << 12)
#define FMI_INT_EN__FMC_INTERRUPT         (0x1UL << 11)
#define FMI_INT_EN__FMC_READY_BUSY        (0x1UL << 10)
#define FMI_INT_EN__FMC_TIMEOUT           (0x1UL << 9)
#define FMI_INT_EN__FMC_NSRBBDONE         (0x1UL << 8)
#define FMI_INT_EN__FMC_NSERR             (0x1UL << 7)
#define FMI_INT_EN__FMC_TRANSDONE         (0x1UL << 6)
#define FMI_INT_EN__FMC_ADDRESSDONE       (0x1UL << 5)
#define FMI_INT_EN__FMC_CMDDONE           (0x1UL << 4)
#define FMI_INT_EN__META_FIFO_OVERFLOW    (0x1UL << 3)
#define FMI_INT_EN__META_FIFO_UNDERFLOW   (0x1UL << 2)
#define FMI_INT_EN__LAST_FMC_DONE         (0x1UL << 1)
#define FMI_INT_EN__TRANSACTION_COMPLETE  (0x1UL << 0)


// =============================================================================
// FMI_DEBUG0 bitfields
// =============================================================================

#if FMI_VERSION == 0

#define FMI_DEBUG0__DMA_BLOCK_COUNT(n)    (((n) & 0x1F) << 24)
#define FMI_DEBUG0__DATA_TC_RECEIVED      (0x1UL << 23)
#define FMI_DEBUG0__DATA_DMA_DONE         (0x1UL << 22)
#define FMI_DEBUG0__DMA_DATA_RW_PTR(n)    (((n) & 0x7F) << 15)

#else

#define FMI_DEBUG0__DMA_SECTOR_COUNT(n)   (((n) & 0x7F) << 25)
#define FMI_DEBUG0__DATA_TC_RECEIVED      (0x1UL << 24)
#define FMI_DEBUG0__DMA_WORD_COUNT(n)     (((n) & 0x1FF << 15)

#endif

#define FMI_DEBUG0__META_BYTE_COUNT(n)    (((n) & 0x3F) << 9)
#define FMI_DEBUG0__FMC_TASKS_PENDING(n)  (((n) & 0x3) << 7)
#define FMI_DEBUG0__ECC_TASKS_PENDING(n)  (((n) & 0x3) << 5)
#define FMI_DEBUG0__DMA_TASKS_PENDING(n)  (((n) & 0x3) << 3)
#define FMI_DEBUG0__FMC_TARGET_BUFFER(b)  (((b) & 0x1) << 2)
#define FMI_DEBUG0__ECC_TARGET_BUFFER(b)  (((b) & 0x1) << 1)
#define FMI_DEBUG0__DMA_TARGET_BUFFER(b)  (((b) & 0x1) << 0)


// =============================================================================
// FMI_DEBUG1 bitfields
// =============================================================================

#if FMI_VERSION == 0

#define FMI_DEBUG1__META_TC_RECEIVED        (0x1UL << 28)
#define FMI_DEBUG1__NEXT_META_LOCATION(n)   (((n) & 0x3F) << 22)
#define FMI_DEBUG1__FMC_PAGE_DONE           (0x1UL << 21)
#define FMI_DEBUG1__FMC_BLOCK_COUNT(n)      (((n) & 0x1F) << 16)

#else

#define FMI_DEBUG1__METADATA_TC_RECEIVED    (0x1UL << 27)
#define FMI_DEBUG1__METADATA_ADDRESS(n)     (((n) & 0x7FF) << 16)

#endif

#define FMI_DEBUG1__META_FIFO_LEVEL(n)      (((n) & 0x3F) << 10)
#define FMI_DEBUG1__META_FIFO_WRITE_PTR(n)  (((n) & 0x1F) << 5)
#define FMI_DEBUG1__META_FIFO_READ_PTR(n)   (((n) & 0x1F) << 0)

// =============================================================================
// FMI_DEBUG2 bitfields
// =============================================================================

#if FMI_VERSION == 0

#define FMI_DEBUG2__BUFFER_1_META_BYTES(n)   (((n) & 0x3F) << 24)
#define FMI_DEBUG2__BUFFER_0_META_BYTES(n)   (((n) & 0x3F) << 18)
#define FMI_DEBUG2__META_BYTES_REMAINING(n)  (((n) & 0x3F) << 12)
#define FMI_DEBUG2__PAUSED_AHB_ADDRESS(n)    (((n) & 0xFF) << 4)
#define FMI_DEBUG2__AHB_STATE(n)             (((n) & 0x3) << 1)
#define FMI_DEBUG2__PAUSED_AHB_BUS           (0x1UL << 0)

#else

#define FMI_DEBUG2__BUFFER_1_META_BYTES(n)   (((n) & 0x3F) << 19)
#define FMI_DEBUG2__BUFFER_0_META_BYTES(n)   (((n) & 0x3F) << 13)
#define FMI_DEBUG2__META_BYTES_REMAINING(n)  (((n) & 0x3F) << 7)
#define FMI_DEBUG2__DATA_STATE(n)            (((n) & 0xF) << 3)
#define FMI_DEBUG2__AHB_STATE(n)             (((n) & 0x7) << 0)

#endif

// =============================================================================
// FMI_DEBUG3 bitfields
// =============================================================================

#if FMI_VERSION > 0

#define FMI_DEBUG3__PAUSED_AHB_ADDRESS(n)    (((n) & 0xFF) << 9)
#define FMI_DEBUG3__PAUSE_AHB_BUS            (0x1UL << 8)
#define FMI_DEBUG3__FMC_PAGE_DONE            (0x1UL << 7)
#define FMI_DEBUG3__FMC_BLOCK_COUNT(n)       (((n) & 0x7F) << 0)

#endif

// =============================================================================
// FMI_DATA_SIZE bitfields
// =============================================================================

#if FMI_VERSION > 0

#define FMI_DATA_SIZE__META_BYTES_PER_ENVELOPE(n) (((n) & 0x3F) << 25)
#define FMI_DATA_SIZE__META_BYTES_PER_PAGE(n)     (((n) & 0x3F) << 19)
#define FMI_DATA_SIZE__BYTES_PER_SECTOR(n)        (((n) & 0x7FF) << 8)
#define FMI_DATA_SIZE__SECTORS_PER_PAGE(n)        (((n) & 0xFF) << 0)

#endif

// =============================================================================
// FMC_ON bitfields
// =============================================================================

#define FMC_ON__SWRST             (0x1UL << 8)
#define FMC_ON__LOCK_ON           (0x1UL << 4)
#define FMC_ON__CLKGATING_RDY     (0x1UL << 1)
#define FMC_ON__ENABLE            (0x1UL << 0)
#define FMC_ON__DISABLE           (!FMC_ON__ENABLE)


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

#define FMC_RW_CTRL__REBHOLD     (0x1UL << 6)
#define FMC_RW_CTRL__WR_MODE     (0x1UL << 5)
#define FMC_RW_CTRL__RD_MODE     (0x1UL << 4)
#define FMC_RW_CTRL__ADDR_MODE   (0x1UL << 3)
#define FMC_RW_CTRL__CMD3_MODE   (0x1UL << 2)
#define FMC_RW_CTRL__CMD2_MODE   (0x1UL << 1)
#define FMC_RW_CTRL__CMD1_MODE   (0x1UL << 0)


// =============================================================================
// FMC_CMD bitfields
// =============================================================================

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

#if FMI_VERSION == 0

#define FMC_DATANUM__NUM(n)      (((n) & 0x3F) << 0)

#else

#define FMC_DATANUM__NUM(n)      (((n) & 0x3FF) << 0)

#endif


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
// FMC_ALARMCMD bitfields
// =============================================================================

#define FMC_ALARMCMD__READ1(v)   (((v) & 0xFF) << 16)
#define FMC_ALARMCMD__PROG1(v)   (((v) & 0xFF) << 8)
#define FMC_ALARMCMD__ERASE1(v)  (((v) & 0xFF) << 0)


// =============================================================================
// FMC_REJECTCMD bitfields
// =============================================================================

#define FMC_REJECTCMD__READ2(v)  (((v) & 0xFF) << 16)
#define FMC_REJECTCMD__PROG2(v)  (((v) & 0xFF) << 8)
#define FMC_REJECTCMD__ERASE2(v) (((v) & 0xFF) << 0)


// =============================================================================
// ECC_CON0 bitfields
// =============================================================================

#define ECC_CON0__START_ENC       (0x1UL << 1)
#define ECC_CON0__START_DEC       (0x1UL << 0)

#if FMI_VERSION == 0

#define ECC_CON0__META_LENGTH(n)  (((n) & 0x1F) << 8)
#define ECC_CON0__MODE_SEL(m)     (((m) & 0x1) << 2)
#define ECC_CON0__MODE_SEL__16BIT ECC_CON0__MODE_SEL(1)
#define ECC_CON0__MODE_SEL__8BIT  ECC_CON0__MODE_SEL(0)

#else

#define ECC_CON0__INFORMATION_LENGTH(n) (((n) & 0x7FF) << 16)
#define ECC_CON0__MODE_SEL(m)           (((m) & 0x1F) << 4)
#define ECC_CON0__MAX_CORRECTION        ECC_CON0__MODE_SEL(30)

#endif


// =============================================================================
// ECC_CON1 bitfields
// =============================================================================

#if FMI_VERSION > 0

#define ECC_CON1__ALLOWED_STUCK_BIT_IN_FP(n)  (((n) & 0x1F) << 16)

#endif

#if FMI_VERSION > 2
#define ECC_CON1__IMPLICIT_WRITE_MARK         (1UL << 31)
#endif

#define ECC_CON1__ERROR_ALERT_LEVEL(n)  (((n) & 0x1F) << 8)
#define ECC_CON1__INT_ENABLE(b)         (((b) & 0x1) << 7)

#define ECC_CON1__INT_ENABLE__ENABLE    ECC_CON1__INT_ENABLE(1)
#define ECC_CON1__INT_ENABLE__DISABLE   ECC_CON1__INT_ENABLE(0)


// =============================================================================
// ECC_RESULT bitfields
// =============================================================================

#if FMI_VERSION == 0

#define ECC_RESULT__ERROR_CNT(n)  (((n) & 0x1F) << 16)
#define ECC_RESULT__ERROR_FLAG    (0x1UL << 0)

#else

#define ECC_RESULT_FIFO__ERROR_CNT(n)       (((n) & 0x1F) << 16)
#define ECC_RESULT_FIFO__STUCK_BIT_EXCEEDED (0x1UL << 14)
#define ECC_RESULT_FIFO__STUCK_BIT_CNT(n)   (((n) & 0x3F) << 8)
#define ECC_RESULT_FIFO__ALL_FF             (0x1UL << 3)
#define ECC_RESULT_FIFO__ALL_ZERO           (0x1UL << 2)
#define ECC_RESULT_FIFO__FREE_PAGE          (0x1UL << 1)
#define ECC_RESULT_FIFO__UNCORRECTABLE      (0x1UL << 0)
#define ECC_RESULT_FIFO__RESET_FIFO         (0x1UL << 0)

#endif

// =============================================================================
// ECC_PND bitfields
// =============================================================================

#if FMI_VERSION == 0

#define ECC_PND__BCH_ALL_FF          (0x1UL << 6)
#define ECC_PND__SECTOR_ERROR_ALERT  (0x1UL << 5)
#define ECC_PND__READY_BUSY          (0x1UL << 4)
#define ECC_PND__UNCORRECTABLE       (0x1UL << 3)
#define ECC_PND__DEC_DONE            (0x1UL << 2)
#define ECC_PND__ENC_DONE            (0x1UL << 1)
#define ECC_PND__ENC_DEC_DONE        (0x1UL << 0)

#else

#define ECC_PND__MAX_STUCK_BIT_CNT(n)    (((n) & 0x3F) << 24)
#define ECC_PND__MAX_ERROR_CNT(n)        (((n) & 0x1F) << 16)
#define ECC_PND__SOME_STUCK_BIT_EXCEEDED (0x1UL << 9)
#define ECC_PND__SOME_ALL_FF             (0x1UL << 8)
#define ECC_PND__SOME_ALL_ZERO           (0x1UL << 7)
#define ECC_PND__ALL_FREE_PAGE           (0x1UL << 6)
#define ECC_PND__SOME_ERROR_ALERT        (0x1UL << 5)
#define ECC_PND__READY_BUSY              (0x1UL << 4)
#define ECC_PND__UNCORRECTABLE           (0x1UL << 3)
#define ECC_PND__ENC_DONE                (0x1UL << 2) // note swap from 8920
#define ECC_PND__DEC_DONE                (0x1UL << 1) // note swap from 8920
#define ECC_PND__ENC_DEC_DONE            (0x1UL << 0)

#endif

// =============================================================================
// ECC_MASK bitfields
// =============================================================================

#if FMI_VERSION == 0

#define ECC_MASK__SECTOR_ERR_ALERT_MASK  (0x1UL << 5)
#define ECC_MASK__UNCORRECTABLE_MASK     (0x1UL << 3)
#define ECC_MASK__DEC_DONE_MASK          (0x1UL << 2)
#define ECC_MASK__ENC_DONE_MASK          (0x1UL << 1)
#define ECC_MASK__ENC_DEC_DONE_MASK      (0x1UL << 0)

#else

#define ECC_MASK__SOME_STUCK_BIT_EXCEEDED    (0x1UL << 9)
#define ECC_MASK__SOME_ALL_FF_MASK           (0x1UL << 8)
#define ECC_MASK__SOME_ALL_ZERO_MASK         (0x1UL << 7)
#define ECC_MASK__SOME_ERROR_ALERT_MASK      (0x1UL << 5)
#define ECC_MASK__ANY_UNCORRECTABLE_MASK     (0x1UL << 3)
#define ECC_MASK__ENC_DONE_MASK              (0x1UL << 2) // note swap from 8920
#define ECC_MASK__DEC_DONE_MASK              (0x1UL << 1) // note swap from 8920
#define ECC_MASK__ENC_DEC_DONE_MASK          (0x1UL << 0)

#endif

// =============================================================================
// ECC_HISTORY bitfields
// =============================================================================

#if FMI_VERSION == 0

#define ECC_HISTORY__ERROR_COUNT_CLEAR          (0x1UL << 24)
#define ECC_HISTORY__ERROR_COUNT(n)             (((n) & 0x1FFF) << 11)
#define ECC_HISTORY__DECODING_NUMBER_CLEAR      (0x1UL << 10)
#define ECC_HISTORY__DECODING_NUMBER(n)         (((n) & 0x3FF) << 0)

#else

#define ECC_HISTORY__ERROR_COUNT_CLEAR          (0x1UL << 26)
#define ECC_HISTORY__ERROR_COUNT(n)             (((n) & 0x7FFF) << 11)
#define ECC_HISTORY__DECODING_NUMBER_CLEAR      (0x1UL << 10)
#define ECC_HISTORY__DECODING_NUMBER(n)         (((n) & 0x3FF) << 0)

#endif


#endif // _H2FMI_REGS_H_

// ********************************** EOF **************************************
