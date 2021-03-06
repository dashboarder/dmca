/*
 * Copyright (C) 2012-2014 Apple Inc. All rights reserved.
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
#include <soc/t8010/a0/pmgr.h>

#if SUPPORT_FPGA
#define OSC_FREQ	(5000000UL)
#define LPPLL_FREQ	(OSC_FREQ)
#else
#define OSC_FREQ	(24000000UL)
#define LPPLL_FREQ	(192000000UL)
#endif

#if SUB_PLATFORM_T8010
/* 
 * T8010 clock gate devices.
 */
enum {
	// Mini-PMGR
	CLK_AOP = HWCLOCK_BASE,
	CLK_DEBUG,
	CLK_AOP_GPIO,
	CLK_AOP_UART0,
	CLK_AOP_UART1,
	CLK_AOP_UART2,
	CLK_AOP_I2CM,
	CLK_AOP_MCA0,
	CLK_AOP_PDM_REF,
	CLK_AOP_CPU,
	CLK_AOP_FILTER,
	CLK_AOP_BUSIF,
	CLK_AOP_LPD0,
	CLK_AOP_LPD1,
	CLK_AOP_HPDS,
	CLK_AOP_HPDSC,
	CLK_AOP_HPDD,

	// PMGR
	CLK_SBR,
	CLK_AIC,
	CLK_DWI,
	CLK_GPIO,
	CLK_PMS,
	CLK_HSIC0PHY,
	CLK_ISPSENS0,
	CLK_ISPSENS1,
	CLK_ISPSENS2,
	CLK_PCIE_REF,
	CLK_SOCUVD,
	CLK_SIO_BUSIF,
	CLK_SIO_P,
	CLK_SIO,
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
	CLK_UART7,
	CLK_UART8,
	CLK_AES0,
	CLK_HFD0,
	CLK_MCC,
	CLK_DCS0,
	CLK_DCS1,
	CLK_DCS2,
	CLK_DCS3,
	CLK_USB,
	CLK_USBCTLREG,
	CLK_USB2HOST0,
	CLK_USB2HOST0_OHCI,
	CLK_USB2HOST1,
	CLK_USB_OTG,
	CLK_SMX,
	CLK_SF,
	CLK_RTMUX,
	CLK_DISP0_FE,
	CLK_DISP0_BE,
	CLK_MIPI_DSI,
	CLK_MIPI = CLK_MIPI_DSI,
	CLK_DP,
	CLK_EDPLINK = CLK_DP,
	CLK_DPLINK = CLK_DP,
	CLK_ISP_SYS,
	CLK_MEDIA,
	CLK_JPG,
	CLK_MSR,
	CLK_PMP,
	CLK_PMS_SRAM,
	CLK_VDEC0,
	CLK_VENC,
	CLK_PCIE,
	CLK_PCIE_AUX,
	CLK_GFX,

	// Unmanaged
	CLK_CPU0,
	CLK_CPU1,
	CLK_CPM,
	CLK_SEP,
	CLK_ISP_RSTS0,
	CLK_ISP_RSTS1,
	CLK_ISP_VIS,
	CLK_ISP_BE,
	CLK_ISP_PEARL,
	CLK_DPRX,
	CLK_VENC_PIPE4,
	CLK_VENC_PIPE5,
	CLK_VENC_ME0,
	CLK_VENC_ME1,

	CLK_LAST,
};

#define PMGR_FIRST_DEVICE CLK_AOP
#define PMGR_LAST_DEVICE CLK_GFX

#else
#error "Unknown Platform"
#endif

#define PMGR_DEVICE_INDEX(_clk)		((_clk) - HWCLOCK_BASE)
#define PMGR_DEVICE_COUNT		(CLK_LAST - HWCLOCK_BASE)

#define PMGR_VALID_DEVICE(_clk)		((_clk) >= PMGR_FIRST_DEVICE && (_clk) <= PMGR_LAST_DEVICE)

// Legacy clocks, no associated device or PS register
enum {
	CLK_FCLK = CLK_LAST,
	CLK_ACLK,
	CLK_HCLK,
	CLK_PCLK,
	CLK_VCLK0,
	CLK_VCLK1,
	CLK_MCLK,
	CLK_NCLK,
	CLK_USBPHYCLK,
	CLK_NCOREF,
};

#endif /* ! __PLATFORM_SOC_HWCLOCKS_H */
