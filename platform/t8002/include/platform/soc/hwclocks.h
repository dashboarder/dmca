/*
 * Copyright (C) 2013 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */
#ifndef __PLATFORM_SOC_HWCLOCKS_H
#define __PLATFORM_SOC_HWCLOCKS_H

#include <platform/clocks.h>

#if SUPPORT_FPGA
#define OSC_FREQ	(5000000UL)
#define LPPLL_FREQ	(OSC_FREQ)
#else
#define OSC_FREQ	(24000000UL)
#define LPPLL_FREQ	(96000000UL)
#endif

// Start from an offset of 128. So that we are safely outside the PS indexes.
#define CLK_FCLK	(HWCLOCK_BASE+128)
#define CLK_ACLK	(HWCLOCK_BASE+129)
#define CLK_HCLK	(HWCLOCK_BASE+130)
#define CLK_PCLK	(HWCLOCK_BASE+131)
#define CLK_VCLK0	(HWCLOCK_BASE+132)
#define CLK_VCLK1	(HWCLOCK_BASE+133)
#define CLK_MCLK	(HWCLOCK_BASE+134)
#define CLK_NCLK	(HWCLOCK_BASE+135)
#define CLK_USBPHYCLK	(HWCLOCK_BASE+136)
#define CLK_NCOREF	(HWCLOCK_BASE+137)
#define CLK_ANS_LINK	(HWCLOCK_BASE+138)

#define PMGR_DEVICE_INDEX(_clk) ((_clk) - HWCLOCK_BASE)
#define PMGR_DEVICE_COUNT (CLK_LAST - HWCLOCK_BASE)

#define PMGR_FIRST_DEVICE CLK_AOP
#define PMGR_LAST_DEVICE CLK_USB2HOST1

#define PMGR_VALID_DEVICE(_clk) ((_clk) >= PMGR_FIRST_DEVICE && (_clk) <= PMGR_LAST_DEVICE)

/* 
 * T8002 clock gate devices. 
 */
enum {
	// Mini-PMGR
	CLK_AOP = HWCLOCK_BASE,
	CLK_DEBUG,
	CLK_AOP_GPIO,
	CLK_AOP_I2CM1,
	CLK_AOP_CPU,
	CLK_AOP_RTCPU,
	CLK_AOP_FILTER,
	CLK_AOP_UART0,
	CLK_AOP_UART1,
	CLK_AOP_UART2,
	CLK_AOP_I2CM,
	CLK_AOP_FILTER_DMA,
	CLK_AOP_LPD0,
	CLK_AOP_HPDS,
	CLK_AOP_HPDSC,
	CLK_AOP_HPDD,
	CLK_MARCONI,

	// PMGR
	CLK_SCU,
	CLK_CPU0,
	CLK_CPU1,
	CLK_PIO,
	CLK_CPU_FAB,
	CLK_NRT_FAB,
	CLK_RT_FAB,
	CLK_AIC,
	CLK_GPIO,
	CLK_ISPSENS0,
	CLK_UVD,
	CLK_HSIC0PHY,
	CLK_AMC,
	CLK_LIO_FAB,
	CLK_LIO_LOGIC,
	CLK_LIO,
	CLK_AES0,
	CLK_MCA0,
	CLK_MCA1,
	CLK_MCA2,
	CLK_HFD,
	CLK_SPI0,
	CLK_SPI1,
	CLK_UART0,
	CLK_UART1,
	CLK_UART2,
	CLK_UART3,
	CLK_UART4,
	CLK_UART5,
	CLK_UART6,
	CLK_UART7,
	CLK_I2C0,
	CLK_I2C1,
	CLK_I2C2,
	CLK_PWM0,
	CLK_USB,
	CLK_USBCTLREG = CLK_USB,
	CLK_USB_OTG = CLK_USB,
	CLK_ETH,
	CLK_ANS,
	CLK_SDIO,
	CLK_DISP,
	CLK_DISP0 = CLK_DISP,
	CLK_MIPI_DSI,
	CLK_MIPI = CLK_MIPI_DSI,
	CLK_ISP,
	CLK_GFX,
	CLK_MEDIA_FAB,
	CLK_MSR,
	CLK_VDEC,
	CLK_RESERVED,
	CLK_JPG,
	CLK_VENC_CPU,
	CLK_USB2HOST1,
	CLK_SEP,
	CLK_VENC_PIPE,
	CLK_VENC_ME,

	CLK_LAST,
};

#endif /* ! __PLATFORM_SOC_HWCLOCKS_H */