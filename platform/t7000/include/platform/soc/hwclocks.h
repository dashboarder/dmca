/*
 * Copyright (C) 2012-2013 Apple Inc. All rights reserved.
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
 * T7000 clock gate devices.
 * Refers to the ps register offsets.
 */
enum {					//  enum : reg
	CLK_CPU0,       		//   0: 0x20000
	CLK_CPU1,       		//   1: 0x20008
#if SUB_PLATFORM_T7001
	CLK_CPU2,       		//   1: 0x20010
#endif
	CLK_CPM = 8,			//   8: 0x20040
	CLK_LIO = 32,			//  32: 0x20100
	CLK_IOMUX,      		//  33: 0x20108
	CLK_AIC,			//  34: 0x20110
	CLK_DEBUG,      		//  35: 0x20118
	CLK_DWI,			//  36: 0x20120
	CLK_GPIO,       		//  37: 0x20128
	CLK_MCA0,       		//  38: 0x20130
	CLK_MCA1,       		//  39: 0x20138
	CLK_MCA2,       		//  40: 0x20140
	CLK_MCA3,       		//  41: 0x20148
	CLK_MCA4,       		//  42: 0x20150
	CLK_PWM0,       		//  43: 0x20158
	CLK_I2C0,       		//  44: 0x20160
	CLK_I2C1,       		//  45: 0x20168
	CLK_I2C2,       		//  46: 0x20170
	CLK_I2C3,       		//  47: 0x20178
	CLK_SPI0,       		//  48: 0x20180
	CLK_SPI1,       		//  49: 0x20188
	CLK_SPI2,       		//  50: 0x20190
	CLK_SPI3,       		//  51: 0x20198
	CLK_UART0,      		//  52: 0x201a0
	CLK_UART1,      		//  53: 0x201a8
	CLK_UART2,      		//  54: 0x201b0
	CLK_UART3,      		//  55: 0x201b8
	CLK_UART4,      		//  56: 0x201c0
	CLK_UART5,      		//  57: 0x201c8
	CLK_UART6,      		//  58: 0x201d0
	CLK_UART7,      		//  59: 0x201d8
	CLK_UART8,      		//  60: 0x201e0
	CLK_AES0,       		//  61: 0x201e8
	CLK_SIO,			//  62: 0x201f0
	CLK_SIO_P,      		//  63: 0x201f8
	CLK_HSIC0PHY,   		//  64: 0x20200
	CLK_HSIC1PHY,   		//  65: 0x20208
	CLK_ISPSENS0,   		//  66: 0x20210
	CLK_ISPSENS1,   		//  67: 0x20218
	CLK_PCIE_REF,   		//  68: 0x20220
	CLK_ANP,			//  69: 0x20228
	CLK_MCC,			//  70: 0x20230
	CLK_MCU,			//  71: 0x20238
	CLK_AMP,			//  72: 0x20240
	CLK_USB,			//  73: 0x20248
	CLK_USBCTLREG,  		//  74: 0x20250
	CLK_USB2HOST0,  		//  75: 0x20258
	CLK_USB2HOST0_OHCI,     	//  76: 0x20260
	CLK_USB2HOST1,  		//  77: 0x20268
	CLK_USB2HOST1_OHCI,     	//  78: 0x20270
	CLK_USB2HOST2,  		//  79: 0x20278
	CLK_USB2HOST2_OHCI,     	//  80: 0x20280
	CLK_USB_OTG,    		//  81: 0x20288
	CLK_SMX,			//  82: 0x20290
	CLK_SF, 			//  83: 0x20298
	CLK_CP, 			//  84: 0x202a0
#if SUB_PLATFORM_T7000
	CLK_DISP_BUSMUX,		//  85: 0x202a8
#elif SUB_PLATFORM_T7001
	CLK_DISP0_BUSIF,		//  85: 0x202a8
#endif
	CLK_DISP0,      		//  86: 0x202b0
#if SUB_PLATFORM_T7000
	CLK_MIPI_DSI,   		//  87: 0x202b8
	CLK_MIPI = CLK_MIPI_DSI,
	CLK_DP, 			//  88: 0x202c0
	CLK_EDPLINK = CLK_DP,
	CLK_DPLINK = CLK_DP,
#elif SUB_PLATFORM_T7001
	CLK_DP,   		//  87: 0x202b8
	CLK_EDPLINK = CLK_DP,
	CLK_DPLINK = CLK_DP,
	CLK_DISP1_BUSIF, 			//  88: 0x202c0
#endif
	CLK_DISP1,      		//  89: 0x202c8
	CLK_ISP,			//  90: 0x202d0
	CLK_MEDIA,      		//  91: 0x202d8
	CLK_MSR,			//  92: 0x202e0
	CLK_JPG,			//  93: 0x202e8
	CLK_VDEC0,      		//  94: 0x202f0
	CLK_UNUSED_0,			//  95: 0x202f8	UNUSED
	CLK_VENC_CPU,   		//  96: 0x20300
	CLK_PCIE,       		//  97: 0x20308
	CLK_PCIE_AUX,   		//  98: 0x20310
	CLK_ANS,			//  99: 0x20318
	CLK_GFX,			// 100: 0x20320

	PMGR_LAST_DEVICE,
	PMGR_FIRST_DEVICE = CLK_LIO,
	CLK_SEP = 128,			// 128: 0x20400
	CLK_VENC_PIPE = 512,		// 512: 0x21000
	CLK_VENC_ME0,			// 513: 0x21008
	CLK_VENC_ME1,			// 514: 0x21010
};

#endif /* ! __PLATFORM_SOC_HWCLOCKS_H */
