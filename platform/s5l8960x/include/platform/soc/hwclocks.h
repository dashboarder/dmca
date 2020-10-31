/*
 * Copyright (C) 2012 Apple Inc. All rights reserved.
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
 * S5L8960X clock gate devices. 
 * Refers to the ps register offsets.
 */
enum {
	CLK_CPU0,
	CLK_CPU1,
	CLK_CPM,
	CLK_LIO,
	CLK_IOMUX,
	CLK_AIC,
	CLK_DEBUG,
	CLK_DWI,
	CLK_GPIO,
	CLK_MCA0,
	CLK_MCA1,
	CLK_MCA2,
	CLK_MCA3,
	CLK_MCA4,
	CLK_PWM0,
	CLK_I2C0,
	CLK_I2C1,
	CLK_I2C2,
	CLK_I2C3,
	CLK_SPI0,
	CLK_SPI1,
	CLK_SPI2,
	CLK_SPI3,
	CLK_UART0,
	CLK_UART1,
	CLK_UART2,
	CLK_UART3,
	CLK_UART4,
	CLK_UART5,
	CLK_UART6,
	CLK_SECUART0,
	CLK_SECUART1,
	CLK_AES0,
	CLK_SIO,
	CLK_SIO_P,
	CLK_HSIC0_PHY,
	CLK_HSIC1_PHY,
	CLK_HSIC2_PHY,
	CLK_ISPSENS0,
	CLK_ISPSENS1,
	CLK_MCC,
	CLK_MCU,
	CLK_AMP,
	CLK_USB,
	CLK_USBCTLREG,
	CLK_USB2HOST0_OHCI,
	CLK_USB2HOST0,
	CLK_USB2HOST1_OHCI,
	CLK_USB2HOST1,
	CLK_USB_OTG,
	CLK_SMX,
	CLK_SF,
	CLK_CP,
	CLK_DISP_BUSMUX,
	CLK_DISP0,
	CLK_MIPI_DSI,
	CLK_MIPI = CLK_MIPI_DSI,
	CLK_EDPLINK,
	CLK_DPLINK = CLK_EDPLINK,
	CLK_DISP1,
	CLK_ISP,
	CLK_MEDIA,
	CLK_MSR,
	CLK_JPG,
	CLK_VDEC,
	CLK_VENC,
	CLK_ANS,
	CLK_ANS_DLL,
	CLK_ADSP,
	CLK_GFX,
	PMGR_LAST_DEVICE,
	CLK_SEP = CLK_GFX + 10,
};

#endif /* ! __PLATFORM_SOC_HWCLOCKS_H */
