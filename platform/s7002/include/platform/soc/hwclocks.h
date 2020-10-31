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
#else
#define OSC_FREQ	(24000000UL)
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

/* 
 * S7002 clock gate devices. 
 * Refers to the ps register offsets.
 */
enum {
	CLK_CPU0,
	CLK_AIC,
	CLK_SPU,
	CLK_SPU_AKF,
	CLK_SPU_UART0,
	CLK_SPU_UART1,
	CLK_SPU_SMB,
	CLK_SPU_SGPIO,
	CLK_DOCKFIFO,
	CLK_MCU,
	CLK_AMP,
	CLK_PIOSYS,
	CLK_DISP0,
	CLK_MIPI_DSI,
	CLK_MIPI = CLK_MIPI_DSI,
	CLK_MSR,
	CLK_MEDIA,
	CLK_ANS,
	CLK_GFX,
	CLK_SDIO,
	CLK_LIO,
	CLK_GPIO,
	CLK_MCA0,
	CLK_MCA1,
	CLK_SPI0,
	CLK_SPI1,
	CLK_DMATX,
	CLK_DMARX,
	CLK_UART0,
	CLK_UART1,
	CLK_UART2,
	CLK_UART3,
	CLK_UART4,
	CLK_AES0,
	CLK_I2C0,
	CLK_I2C1,
	CLK_PWM0,
	CLK_AUE,
	CLK_USB = CLK_AUE,
	CLK_USB_M7,
	CLK_USBCTLREG = CLK_USB_M7,
	CLK_USB_OTG = CLK_USB_M7,
	CLK_ETH,
	CLK_VDEC,
	PMGR_LAST_DEVICE,
};

#endif /* ! __PLATFORM_SOC_HWCLOCKS_H */
