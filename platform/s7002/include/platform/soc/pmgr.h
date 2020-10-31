/*
 * Copyright (C) 2013-2014 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */
#ifndef __PLATFORM_SOC_PMGR_H
#define __PLATFORM_SOC_PMGR_H

#include <lib/devicetree.h>
#include <platform/soc/hwregbase.h>

enum {
    PMGR_CLK_OSC = 0,
    PMGR_CLK_PLL0,
    PMGR_CLK_GPIO,    
    PMGR_CLK_SOURCE_FIRST = PMGR_CLK_GPIO,
    PMGR_CLK_MCU,
    PMGR_CLK_MCU_FIXED,
    PMGR_CLK_GFX,
    PMGR_CLK_MIPI_DSI,
    PMGR_CLK_VID,
    PMGR_CLK_MEDIA,
    PMGR_CLK_DISP,
    PMGR_CLK_SDIO,
    PMGR_CLK_ANS,
    PMGR_CLK_PIO,
    PMGR_CLK_LIO,
    PMGR_CLK_AUE,
    PMGR_CLK_USB,
    PMGR_CLK_SPI0_N,
    PMGR_CLK_SPI1_N,
    PMGR_CLK_NCO_REF0,
    PMGR_CLK_NCO_REF1,
    PMGR_CLK_NCO_ALG0,
    PMGR_CLK_NCO_ALG1,
    PMGR_CLK_MCA0_M,
    PMGR_CLK_MCA1_M,
    PMGR_CLK_SOURCE_LAST = PMGR_CLK_MCA1_M,
    PMGR_CLK_S0,
    PMGR_CLK_S1,
    PMGR_CLK_COUNT,
    PMGR_CLK_NOT_SUPPORTED
};

#define PMGR_CLK_CFG_INDEX(_clk) ((_clk) - PMGR_CLK_GPIO)

// PLL Config Regs
#define rPMGR_PLL0_CTL              (*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x0000))
    #define PMGR_PLL_ENABLE             (1 << 31)
    #define PMGR_PLL_LOAD               (1 << 29)
    #define PMGR_PLL_PENDING            (1 << 25)
    #define PMGR_PLL_M_SHIFT            (12)
    #define PMGR_PLL_M_MASK             (0x1FF)
    #define PMGR_PLL_P_SHIFT            (4)
    #define PMGR_PLL_P_MASK             (0x1F)
    #define PMGR_PLL_S_SHIFT            (0)
    #define PMGR_PLL_S_MASK             (0xF)
    #define PMGR_PLL_M(_m)              (((_m) & PMGR_PLL_M_MASK) << PMGR_PLL_M_SHIFT)
    #define PMGR_PLL_P(_p)              (((_p) & PMGR_PLL_P_MASK) << PMGR_PLL_P_SHIFT)
    #define PMGR_PLL_S(_s)              (((_s) & PMGR_PLL_S_MASK) << PMGR_PLL_S_SHIFT)
    #define PMGR_PLL_FREQ(_m, _p, _s)   ((((_m) * OSC_FREQ) / (_p))/((_s) + 1))

#define rPMGR_PLL0_EXT_BYPASS_CFG (*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x0004))
    #define PMGR_PLL_BYP_ENABLED        (1 << 31)
    #define PMGR_PLL_PLL_ENABLED        (1 << 30)
    #define PMGR_PLL_EXT_BYPASS         (1 << 0)

#define rPMGR_PLL0_CFG            (*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x0008))
#define rPMGR_PLL0_ANA_PARAMS0    (*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x000c))
#define rPMGR_PLL0_ANA_PARAMS1    (*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x0010))
//#define rPMGR_PLL0_FD_CTL         (*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x0014))
//#define rPMGR_PLL0_FRAC           (*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x0018))
//#define rPMGR_PLL0_DEBUG0         (*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x001c))
//#define rPMGR_PLL0_DEBUG1         (*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x0020))
#define rPMGR_PLL0_PLL_DELAY_CTL0 (*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x0024))
#define rPMGR_PLL0_PLL_DELAY_CTL1 (*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x0028))

// General Clocks Config Regs
#define rPMGR_GPIO_CLK_CFG          (*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x10000))
#define rPMGR_MCU_CLK_CFG           (*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x10004))
#define rPMGR_MCU_FIXED_CLK_CFG     (*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x10008))
#define rPMGR_GFX_CLK_CFG           (*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x1000c))
#define rPMGR_MIPI_DSI_CLK_CFG      (*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x10010))
#define rPMGR_VID_CLK_CFG           (*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x10014))
#define  PMGR_MEDIA_CLK_CFG         (0x10018)
#define rPMGR_MEDIA_CLK_CFG         (*(volatile uint32_t *) (PMGR_BASE_ADDR + PMGR_MEDIA_CLK_CFG))
#define rPMGR_DISP_CLK_CFG          (*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x1001c))
#define rPMGR_SDIO_CLK_CFG          (*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x10020))
#define rPMGR_ANS_CLK_CFG           (*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x10024))
#define rPMGR_PIO_CLK_CFG           (*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x10028))
#define rPMGR_LIO_CLK_CFG           (*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x1002c))
#define rPMGR_AUE_CLK_CFG           (*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x10030))
#define rPMGR_USB_CLK_CFG           (*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x10034))
#define rPMGR_SPI0_N_CLK_CFG        (*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x10038))
#define rPMGR_SPI1_N_CLK_CFG        (*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x1003c))
#define rPMGR_NCO_REF0_CLK_CFG      (*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x10040))
#define rPMGR_NCO_REF1_CLK_CFG      (*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x10044))
#define rPMGR_NCO_ALG0_CLK_CFG      (*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x10048))
#define rPMGR_NCO_ALG1_CLK_CFG      (*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x1004c))
#define rPMGR_MCA0_M_CLK_CFG        (*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x10050))
#define rPMGR_MCA1_M_CLK_CFG        (*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x10054))

#define PMGR_FIRST_CLK_CFG          (&rPMGR_GPIO_CLK_CFG)
#define PMGR_LAST_CLK_CFG           (&rPMGR_MCA1_M_CLK_CFG)
#define PMGR_CLK_CFG_COUNT          (((((void *)(PMGR_LAST_CLK_CFG)) - ((void *)(PMGR_FIRST_CLK_CFG))) / 4) + 1)
#define PMGR_CLK_NUM(_r)            (((void *)&(rPMGR_ ## _r ## _CLK_CFG) - ((void *)(PMGR_FIRST_CLK_CFG))) / 4)

// Spare Clocks Config Regs
#define rPMGR_S0_CLK_CFG            (*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x10200))
#define rPMGR_S1_CLK_CFG            (*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x10204))

#define PMGR_FIRST_SPARE_CLK_CFG    (&rPMGR_S0_CLK_CFG)
#define PMGR_LAST_SPARE_CLK_CFG     (&rPMGR_S1_CLK_CFG)
#define PMGR_SPARE_CLK_CFG_COUNT    (((((void *)(PMGR_LAST_SPARE_CLK_CFG)) - ((void *)(PMGR_FIRST_SPARE_CLK_CFG))) / 4) + 1)
#define PMGR_SPARE_CLK_NUM(_r)      (((void *)&(rPMGR_ ## _r ## _CLK_CFG) - ((void *)(PMGR_FIRST_SPARE_CLK_CFG))) / 4)

// CLK_CFG Bits
#define PMGR_CLK_CFG_ENABLE         (1 << 31)
#define PMGR_CLK_CFG_PENDING        (1 << 30)
#define PMGR_CLK_CFG_PENDING_SHIFT  (30)
#define PMGR_CLK_CFG_SRC_SEL_MASK   (0x3F)
#define PMGR_CLK_CFG_CFG_SEL_SHIFT  (16)
#define PMGR_CLK_CFG_SRC_SEL_SHIFT  (24)
#define PMGR_CLK_CFG_SRC_SEL(_n)    ((_n) << PMGR_CLK_CFG_SRC_SEL_SHIFT)
#define PMGR_CLK_CFG_DIVISOR_MASK   (0x3F)
#define PMGR_CLK_CFG_DIVISOR(_n)    ((_n) << 0)

// NCO Config block
#define rPMGR_NCO_CLK_CFG(_n)       (*(volatile uint32_t *) (PMGR_BASE_ADDR + (_n) * 0x1000 + 0x11000 + 0x0))

// Misc Clock Tree Power Saving Regs
#define rPMGR_CLK_DIVIDER_ACG_CFG   (*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x10300))
#define rPMGR_CLK_POWER_CONFIG      (*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x10304))

//PERF_STATE
#define rPMGR_SOC_PERF_STATE_CTL    (*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x1a000))

// Power State Regs
#define rPMGR_CPU_PS                (*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x20000))
#define rPMGR_AIC_PS                (*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x20008))
#define rPMGR_SPU_PS                (*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x20010))
#define rPMGR_SPU_AKF_PS            (*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x20018))
#define rPMGR_SPU_UART0_PS          (*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x20020))
#define rPMGR_SPU_UART1_PS          (*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x20028))
#define rPMGR_SPU_SMB_PS            (*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x20030))
#define rPMGR_SPU_SGPIO_PS          (*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x20038))
#define rPMGR_DOCKFIFO_PS           (*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x20040))
#define rPMGR_MCU_PS                (*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x20048))
#define rPMGR_AMP_PS                (*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x20050))
#define rPMGR_PIOSYS_PS             (*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x20058))
#define rPMGR_DISP_PS               (*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x20060))
#define rPMGR_MIPI_DSI_PS           (*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x20068))
#define  PMGR_MSR_PS                (0x20070)
#define rPMGR_MSR_PS                (*(volatile uint32_t *) (PMGR_BASE_ADDR + PMGR_MSR_PS))
#define rPMGR_MEDIA_PS              (*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x20078))
#define rPMGR_ANS_PS                (*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x20080))
#define rPMGR_GFX_PS                (*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x20088))
#define rPMGR_SDIO_PS               (*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x20090))
#define rPMGR_LIO_PS                (*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x20098))
#define rPMGR_GPIO_PS               (*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x200a0))
#define rPMGR_MCA0_PS               (*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x200a8))
#define rPMGR_MCA1_PS               (*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x200b0))
#define rPMGR_SPI0_PS               (*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x200b8))
#define rPMGR_SPI1_PS               (*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x200c0))
#define rPMGR_DMATX_PS              (*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x200c8))
#define rPMGR_DMARX_PS              (*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x200d0))
#define rPMGR_UART0_PS              (*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x200d8))
#define rPMGR_UART1_PS              (*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x200e0))
#define rPMGR_UART2_PS              (*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x200e8))	
#define rPMGR_UART3_PS              (*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x200f0))
#define rPMGR_UART4_PS              (*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x200f8))
#define rPMGR_AES_PS                (*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x20100))
#define rPMGR_I2C0_PS               (*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x20108))	
#define rPMGR_I2C1_PS               (*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x20110))
#define rPMGR_PWM0_PS               (*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x20118))
#define rPMGR_AUE_PS                (*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x20120))
#define rPMGR_USB_PS                (*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x20128))
#define rPMGR_ETH_PS                (*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x20130))
#define rPMGR_VDEC_PS               (*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x20138))

#define rSPU_PMGR_DEBUG_PS          (*(volatile uint32_t *) (SPU_SEQUENCER_BASE_ADDR + 0x03004))

#define	 PMGR_PS_RESET              (1 << 31)
#define	 PMGR_PS_ACTUAL_PS_SHIFT    (4)
#define	 PMGR_PS_ACTUAL_PS_MASK     (0xF)
#define	 PMGR_PS_MANUAL_PS_MASK     (0xF)
#define	 PMGR_PS_RUN_MAX            (0xF)
#define	 PMGR_PS_CLOCK_OFF          (0x4)
#define	 PMGR_PS_POWER_OFF          (0x0)

#define PMGR_FIRST_PS               (&rPMGR_CPU_PS)
#define PMGR_LAST_PS                (&rPMGR_VDEC_PS)
#define PMGR_PS_COUNT               (((((void *)(PMGR_LAST_PS)) - ((void *)(PMGR_FIRST_PS))) / 8) + 1)
#define PMGR_PS_NUM(_r)             (((void *)&(rPMGR_ ## _r ## _PS) - ((void *)(PMGR_FIRST_PS))) / 8)

// Power Gate Tunable Regs
#define rPMGR_PWR_CPU_CFG0          (*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x22000))
#define rPMGR_PWR_CPU_CFG1          (*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x22004))
#define rPMGR_PWR_CPU_CFG2          (*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x22008))
#define rPMGR_PWR_CPU_DBG           (*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x2200c))
#define rPMGR_PWR_AMCPIO_CFG0       (*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x22010))
#define rPMGR_PWR_AMCPIO_CFG1       (*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x22014))
#define rPMGR_PWR_AMCPIO_CFG2       (*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x22018))
#define rPMGR_PWR_AMCPIO_DBG        (*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x2201c))
#define rPMGR_PWR_DISP_CFG0         (*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x22020))
#define rPMGR_PWR_DISP_CFG1         (*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x22024))
#define rPMGR_PWR_DISP_CFG2         (*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x22028))
#define rPMGR_PWR_DISP_DBG          (*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x2202c))
#define rPMGR_PWR_DISP_BE_CFG0      (*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x22030))
#define rPMGR_PWR_DISP_BE_CFG1      (*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x22034))
#define rPMGR_PWR_DISP_BE_CFG2      (*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x22038))
#define rPMGR_PWR_DISP_BE_DBG       (*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x2203c))
#define rPMGR_PWR_MSR_CFG0          (*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x22040))
#define rPMGR_PWR_MSR_CFG1          (*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x22044))
#define rPMGR_PWR_MSR_CFG2          (*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x22048))
#define rPMGR_PWR_MSR_DBG           (*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x2204c))
#define rPMGR_PWR_ANS_CFG0          (*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x22050))
#define rPMGR_PWR_ANS_CFG1          (*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x22054))
#define rPMGR_PWR_ANS_CFG2          (*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x22058))
#define rPMGR_PWR_ANS_DBG           (*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x2205c))
#define rPMGR_PWR_GFX_CFG0          (*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x22060))
#define rPMGR_PWR_GFX_CFG1          (*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x22064))
#define rPMGR_PWR_GFX_CFG2          (*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x22068))
#define rPMGR_PWR_GFX_DBG           (*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x2206c))
#define rPMGR_PWR_SDIO_CFG0         (*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x22070))
#define rPMGR_PWR_SDIO_CFG1         (*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x22074))
#define rPMGR_PWR_SDIO_CFG2         (*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x22078))
#define rPMGR_PWR_SDIO_DBG          (*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x2207c))
#define rPMGR_PWR_LIO_CFG0          (*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x22080))
#define rPMGR_PWR_LIO_CFG1          (*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x22084))
#define rPMGR_PWR_LIO_CFG2          (*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x22088))
#define rPMGR_PWR_LIO_DBG           (*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x2208c))
#define rPMGR_PWR_AUE_CFG0          (*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x22090))
#define rPMGR_PWR_AUE_CFG1          (*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x22094))
#define rPMGR_PWR_AUE_CFG2          (*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x22098))
#define rPMGR_PWR_AUE_DBG           (*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x2209c))
#define rPMGR_PWR_VDEC_CFG0         (*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x220a0))
#define rPMGR_PWR_VDEC_CFG1         (*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x220a4))
#define rPMGR_PWR_VDEC_CFG2         (*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x220a8))
#define rPMGR_PWR_VDEC_DBG          (*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x220ac))
#define rPMGR_MCU_ASYNC_RESET       (*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x22300))

// EMA Tunable Regs
#define rPMGR_EMA_SOC0              (*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x26000))
#define rPMGR_EMA_SOC1              (*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x26004))

// Timer Regs
#define rPMGR_CHIP_WDOG_TMR         (*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x27000))
#define rPMGR_CHIP_WDOG_RST_CNT     (*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x27004))
#define rPMGR_CHIP_WDOG_INTR_CNT    (*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x27008))
#define rPMGR_CHIP_WDOG_CTL         (*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x2700c))
#define rPMGR_SYS_WDOG_TMR          (*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x27010))
#define rPMGR_SYS_WDOG_RST_CNT      (*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x27014))
#define rPMGR_SYS_WDOG_CTL          (*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x2701c))
#define rPMGR_EVENT_TMR             (*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x27100))
#define rPMGR_EVENT_TMR_PERIOD      (*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x27104))
#define rPMGR_EVENT_TMR_CTL         (*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x27108))
#define rPMGR_INTERVAL_TMR          (*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x27200))
#define rPMGR_INTERVAL_TMR_CTL      (*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x27204))

#define rPMGR_SCRATCH(_n)       (*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x29000 + ((_n)*4)))
#define rPMGR_SCRATCH0          rPMGR_SCRATCH(0)			/* Flags */
#define rPMGR_SCRATCH1          rPMGR_SCRATCH(1)
#define rPMGR_SCRATCH2          rPMGR_SCRATCH(2)
#define rPMGR_SCRATCH3          rPMGR_SCRATCH(3)
#define rPMGR_SCRATCH4          rPMGR_SCRATCH(4)
#define rPMGR_SCRATCH5          rPMGR_SCRATCH(5)
#define rPMGR_SCRATCH6          rPMGR_SCRATCH(6)
#define rPMGR_SCRATCH7          rPMGR_SCRATCH(7)			/* Consistent debug root pointer */
#define rPMGR_SCRATCH8          rPMGR_SCRATCH(8)
#define rPMGR_SCRATCH9          rPMGR_SCRATCH(9)
#define rPMGR_SCRATCH10         rPMGR_SCRATCH(10)
#define rPMGR_SCRATCH11         rPMGR_SCRATCH(11)
#define rPMGR_SCRATCH12         rPMGR_SCRATCH(12)
#define rPMGR_SCRATCH13         rPMGR_SCRATCH(13)			/* Memory info */
#define rPMGR_SCRATCH14         rPMGR_SCRATCH(14)			/* Boot Nonce 0 */
#define rPMGR_SCRATCH15         rPMGR_SCRATCH(15)			/* Boot Nonce 1 */
#define rPMGR_SCRATCH16         rPMGR_SCRATCH(16)			/* Boot Manifest Hash 0 */
#define rPMGR_SCRATCH17         rPMGR_SCRATCH(17)			/* Boot Manifest Hash 1 */
#define rPMGR_SCRATCH18         rPMGR_SCRATCH(18)			/* Boot Manifest Hash 2 */
#define rPMGR_SCRATCH19         rPMGR_SCRATCH(19)			/* Boot Manifest Hash 3 */
#define rPMGR_SCRATCH20         rPMGR_SCRATCH(20)			/* Boot Manifest Hash 4 */
#define rPMGR_SCRATCH21         rPMGR_SCRATCH(21)
#define rPMGR_SCRATCH22         rPMGR_SCRATCH(22)
#define rPMGR_SCRATCH23         rPMGR_SCRATCH(23)

#define rPMGR_SECURITY          (*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x31020))

#define rPMGR_MISC_GFX_CTL      (*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x31040))

#define rPMGR_MISC_ACG          (*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x32000))

extern void pmgr_update_device_tree(DTNode *pmgr_node);

#endif /* ! __PLATFORM_SOC_PMGR_H */
