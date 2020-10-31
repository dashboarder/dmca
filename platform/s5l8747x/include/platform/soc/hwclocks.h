/*
 * Copyright (C) 2011 Apple Inc. All rights reserved.
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
#define OSC_FREQ	(27000000UL)
#else
#define OSC_FREQ	(24000000UL)
#endif

#define CLK_FCLK	(HWCLOCK_BASE+0)
#define CLK_ACLK	(HWCLOCK_BASE+1)
#define CLK_HCLK	(HWCLOCK_BASE+2)
#define CLK_PCLK	(HWCLOCK_BASE+3)
#define CLK_VCLK0	(HWCLOCK_BASE+4)
#define CLK_VCLK1	(HWCLOCK_BASE+5)
#define CLK_MCLK	(HWCLOCK_BASE+6)
#define CLK_NCLK	(HWCLOCK_BASE+7)
#define CLK_USBPHYCLK	(HWCLOCK_BASE+8)
#define CLK_DCLK	(HWCLOCK_BASE+9)

/* S5L8747X clock gate devices */
enum {
	CLK_AES,
	CLK_CHIPID,
	CLK_DMAC0,
	CLK_DMAC1,
	CLK_GPIO,
	CLK_IIC0,
	CLK_IIC1,
	CLK_IIC2,
	CLK_PKE,
	CLK_SHA1,
	CLK_SPI0,
	CLK_SPI1,
	CLK_TIMER,
	CLK_UART0,
	CLK_UART1,
	CLK_UART2,
	CLK_USBOTG0,
	CLK_USBOTG1,
	CLK_USBPHY,
	CLK_ROSC,
};

// USB PHY driver expects CLK_USBOTG, so give them what they want
#define CLK_USBOTG	CLK_USBOTG1

#endif /* ! __PLATFORM_SOC_HWCLOCKS_H */
