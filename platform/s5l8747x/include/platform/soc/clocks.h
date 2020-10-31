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
#ifndef __PLATFORM_SOC_CLOCKS_H
#define __PLATFORM_SOC_CLOCKS_H

#include <platform/soc/hwregbase.h>

#define	rCLKCON0			(*(volatile u_int32_t *)(CLKCON_BASE_ADDR + 0x00))
#define	rCLKCON1			(*(volatile u_int32_t *)(CLKCON_BASE_ADDR + 0x10))
#define	rCLKCON2			(*(volatile u_int32_t *)(CLKCON_BASE_ADDR + 0x20))
#define	rCLKCON3			(*(volatile u_int32_t *)(CLKCON_BASE_ADDR + 0x30))
#define	rCLKCON4			(*(volatile u_int32_t *)(CLKCON_BASE_ADDR + 0x40))
#define	rCLKCON5			(*(volatile u_int32_t *)(CLKCON_BASE_ADDR + 0x50))
#define	rCLKCON6			(*(volatile u_int32_t *)(CLKCON_BASE_ADDR + 0x60))
#define	rCLKCON7			(*(volatile u_int32_t *)(CLKCON_BASE_ADDR + 0x70))
#define	rPLL0PMS			(*(volatile u_int32_t *)(CLKCON_BASE_ADDR + 0x100))
#define	rPLL1PMS			(*(volatile u_int32_t *)(CLKCON_BASE_ADDR + 0x104))
#define	rPLL2PMS			(*(volatile u_int32_t *)(CLKCON_BASE_ADDR + 0x108))
#define	rPLL3PMS			(*(volatile u_int32_t *)(CLKCON_BASE_ADDR + 0x10C))
#define	rPLL0LCNT			(*(volatile u_int32_t *)(CLKCON_BASE_ADDR + 0x110))
#define	rPLL1LCNT			(*(volatile u_int32_t *)(CLKCON_BASE_ADDR + 0x114))
#define	rPLL2LCNT			(*(volatile u_int32_t *)(CLKCON_BASE_ADDR + 0x118))
#define	rPLL3LCNT			(*(volatile u_int32_t *)(CLKCON_BASE_ADDR + 0x11C))
#define	rPLLLOCKINT			(*(volatile u_int32_t *)(CLKCON_BASE_ADDR + 0x150))
#define	rPLLLOCK			(*(volatile u_int32_t *)(CLKCON_BASE_ADDR + 0x154))
#define	rPLLCON				(*(volatile u_int32_t *)(CLKCON_BASE_ADDR + 0x158))
#define	rCGCON0				(*(volatile u_int32_t *)(CLKCON_BASE_ADDR + 0x200))
#define	rCGCON1				(*(volatile u_int32_t *)(CLKCON_BASE_ADDR + 0x204))
#define	rCGCON2				(*(volatile u_int32_t *)(CLKCON_BASE_ADDR + 0x208))
#define	rCGCON3				(*(volatile u_int32_t *)(CLKCON_BASE_ADDR + 0x20C))
#define	rCGCON4				(*(volatile u_int32_t *)(CLKCON_BASE_ADDR + 0x210))
#define	rSWRCON				(*(volatile u_int32_t *)(CLKCON_BASE_ADDR + 0x250))
#define	rRSTSR				(*(volatile u_int32_t *)(CLKCON_BASE_ADDR + 0x254))
#define	rKEEPMAP			(*(volatile u_int32_t *)(CLKCON_BASE_ADDR + 0x300))
#define	rVERSION			(*(volatile u_int32_t *)(CLKCON_BASE_ADDR + 0xA00))
#define	rMONITOR01			(*(volatile u_int32_t *)(CLKCON_BASE_ADDR + 0x400))
#define	rMONITOR2			(*(volatile u_int32_t *)(CLKCON_BASE_ADDR + 0x404))

enum {
	CLKGEN_CLK_OSC = 0,
	CLKGEN_CLK_PLL0,
	CLKGEN_CLK_PLL1,
	CLKGEN_CLK_PLL2,
	CLKGEN_CLK_PLL3,
	CLKGEN_CLK_SCLK,
	CLKGEN_CLK_FCLK,
	CLKGEN_CLK_DCLK,
	CLKGEN_CLK_ACLK,
	CLKGEN_CLK_HCLK,
	CLKGEN_CLK_PCLK,
	CLKGEN_CLK_NCLK,
	CLKGEN_CLK_USBPHY,
	CLKGEN_CLK_VCLK0,
	CLKGEN_CLK_VCLK1,
	CLKGEN_CLK_COUNT
};


#endif /* ! __PLATFORM_SOC_CLOCKS_H */
