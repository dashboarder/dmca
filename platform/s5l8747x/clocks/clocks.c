/*
 * Copyright (C) 2011-2012 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */
#include <debug.h>
#include <platform.h>
#include <lib/libc.h>
#include <platform/clocks.h>
#include <platform/timer.h>
#include <platform/soc/clocks.h>
#include <platform/soc/hwclocks.h>
#include <platform/soc/chipid.h>
#include <sys.h>
#include <sys/boot.h>
#include <sys/menu.h>

#include <drivers/power.h>


#define PLL_FREQ_TARGET(pllx) (1ULL * pllx##_O * pllx##_M / pllx##_P / (1 << pllx##_S))

#define PRESCALE_EN(x)	((x) == 1 ? 0 : 1)
#define CLK_DIV_N(x)	((x) == 1 ? 0 : ((x)/2)-1)

#if APPLICATION_IBOOT

//PLL0 @ 800MHz
#define PLL0		0
#define PLL0_O		OSC_FREQ
#define PLL0_P		12
#define PLL0_M		400
#define PLL0_S		0
#define PLL0_VSEL	1 // High if 600MHz < Fvco < 864MHz (Fvco = Fin * M / P)
#define PLL0_T		PLL_FREQ_TARGET(PLL0)

// PLL1 @ 330MHz
#define PLL1		1
#define PLL1_O		OSC_FREQ
#define PLL1_P		4
#define PLL1_M		110
#define PLL1_S		1
#define PLL1_VSEL	1 // High if 600MHz < Fvco < 864MHz (Fvco = Fin * M / P)
#define PLL1_T		PLL_FREQ_TARGET(PLL1)

// PLL2 @ 432MHz
#define PLL2		2
#define PLL2_O		OSC_FREQ
#define PLL2_P		5
#define PLL2_M		180
#define PLL2_S		1
#define PLL2_VSEL	1 // High if 600MHz < Fvco < 864MHz (Fvco = Fin * M / P)
#define PLL2_T		PLL_FREQ_TARGET(PLL2432/)

#define LOW_PERF_DIV_N		(11)
#define MEDIUM_PERF_DIV_N	(5)
#define MEMORY_PERF_DIV_N	(2)
// With PLL0 @ 800MHz for B0, SCLK_DIV_VAL needs to be at least 2!
#define HIGH_PERF_DIV_N		(2)

#endif /* APPLICATION_IBOOT */

#if APPLICATION_SECUREROM

#define PLL0		0
#define PLL0_O		OSC_FREQ
#define PLL0_P		6
#define PLL0_M		133
#define PLL0_S		2
#define PLL0_VSEL	0
#define PLL0_T		PLL_FREQ_TARGET(PLL0)

#define LOW_PERF_DIV_N		(1)
#define MEDIUM_PERF_DIV_N	(1)
#define MEMORY_PERF_DIV_N	(2)
#define HIGH_PERF_DIV_N		(1)

#endif /* APPLICATION_SECUREROM */

#define SCLK_PLL	0
#define FCLK_DIV_N	(1)
#define DCLK_DIV_N	(1)
#define ACLK_DIV_N	(MEMORY_PERF_DIV_N)
#define HCLK_DIV_N	(MEMORY_PERF_DIV_N)
#define PCLK_DIV_N	(MEMORY_PERF_DIV_N)

#define NCLK_DIV_N	(1)

/* current clock speeds */
static u_int32_t clks[CLKGEN_CLK_COUNT];
static u_int32_t *plls = &clks[CLKGEN_CLK_PLL0];

static u_int32_t perf_level;
static u_int32_t perf_div;

static u_int32_t get_pll(int pll);

int clocks_init(void)
{
#if APPLICATION_IBOOT && (PRODUCT_IBOOT || PRODUCT_IBEC)
	
#if SUPPORT_FPGA
	clks[CLKGEN_CLK_SCLK] = 10000000;
	clks[CLKGEN_CLK_FCLK] = 10000000;
	clks[CLKGEN_CLK_DCLK] = 10000000;
	clks[CLKGEN_CLK_ACLK] = 10000000;
	clks[CLKGEN_CLK_HCLK] = 10000000;
	clks[CLKGEN_CLK_PCLK] = 10000000;
	clks[CLKGEN_CLK_NCLK] = 10000000;
	clks[CLKGEN_CLK_USBPHY] = 12000000;

	perf_div = 1;
	perf_level = kPerformanceHigh;

#else
#error "The calculations of vclk0 and vclk1 need to be scrubbed"
	int cnt;
	int sclk_pll	= ((rCLKCON0 >> 12) & 3) - 1;
	int sclk_div_n	= ((rCLKCON0 >>  0) & 0xF) + 1;
	int fclk_div_n	= ((rCLKCON1 >> 29) & 1) ? ((((rCLKCON1 >> 24) & 0x1F) + 1) * 2) : 1;
	int dclk_div_n	= ((rCLKCON1 >> 23) & 1) ? ((((rCLKCON1 >> 18) & 0x1F) + 1) * 2) : 1;
	int hclk_div_n	= ((rCLKCON1 >> 17) & 1) ? ((((rCLKCON1 >> 12) & 0x1F) + 1) * 2) : 1;
	int pclk_div_n	= ((rCLKCON1 >> 11) & 1) ? ((((rCLKCON1 >>  6) & 0x1F) + 1) * 2) : 1;
	int aclk_div_n	= ((rCLKCON1 >>  5) & 1) ? ((((rCLKCON1 >>  0) & 0x1F) + 1) * 2) : 1;
	int vclk0_pll	= ((rCLKCON2 >> 28) & 3) - 1;
	int vclk0_div_n	= ((rCLKCON2 >> 16) & 0xF) + 1;
	int vclk1_pll	= ((rCLKCON2 >> 12) & 3) - 1;
	int vclk1_div_n	= (((rCLKCON2 >> 0) & 0xF) + 1) * (((rCLKCON2 >> 4) & 0xF) + 1);
	
	plls[-1] = OSC_FREQ;
	for (cnt = 0; cnt < 3; cnt++) plls[cnt] = get_pll(cnt);
	
	perf_div = sclk_div_n;

	switch (perf_div) {
		default :
		case HIGH_PERF_DIV_N : perf_level = kPerformanceHigh; break;
		case MEDIUM_PERF_DIV_N : perf_level = kPerformanceMedium; break;
		case LOW_PERF_DIV_N : perf_level = kPerformanceLow; break;
	}

	clks[CLKGEN_CLK_SCLK] = plls[sclk_pll] / HIGH_PERF_DIV_N;
	clks[CLKGEN_CLK_FCLK] = clks[CLKGEN_CLK_SCLK] / fclk_div_n;
	clks[CLKGEN_CLK_DCLK] = clks[CLKGEN_CLK_SCLK] / dclk_div_n;
	clks[CLKGEN_CLK_ACLK] = clks[CLKGEN_CLK_SCLK] / aclk_div_n;
	clks[CLKGEN_CLK_HCLK] = clks[CLKGEN_CLK_SCLK] / hclk_div_n;
	clks[CLKGEN_CLK_PCLK] = clks[CLKGEN_CLK_SCLK] / pclk_div_n;
	clks[CLKGEN_CLK_VCLK0] = plls[vclk0_pll] / vclk0_div_n;
	clks[CLKGEN_CLK_VCLK1] = plls[vclk1_pll] / vclk1_div_n;
	
	clks[CLKGEN_CLK_NCLK] = OSC_FREQ;
	
	clks[CLKGEN_CLK_USBPHY] = OSC_FREQ;
#endif
	
#endif /* APPLICATION_IBOOT && (PRODUCT_IBOOT || PRODUCT_IBEC) */

	return 0;
}

static u_int32_t get_pll(int pll)
{
	volatile u_int32_t *pllpms;
	u_int32_t pllcon, pms;
	u_int64_t freq;
	
	pllcon = rPLLCON;
	
	if (((pllcon >> pll) & 1) == 0) return OSC_FREQ;
	
	switch (pll) {
		case 0:
			pllpms = &rPLL0PMS;
			break;
		case 1:
			pllpms = &rPLL1PMS;
			break;
		case 2:
			pllpms = &rPLL2PMS;
			break;
		case 3:
			pllpms = &rPLL3PMS;
			break;
		default:
			return 0;
			break;
	}
	
	freq = OSC_FREQ;
	pms = *pllpms;
	
	freq *= (pms >> 8) & 0x3FF;  // *M
	
	freq /= (pms >> 24) & 0x3F;  // /P
	
	freq /= 1 << (pms & 0x07);   // /2^S
	
	return freq;
}

static void set_pll(int pll, u_int32_t p, u_int32_t m, u_int32_t s, u_int32_t vsel)
{
	volatile u_int32_t *pllpms;
	volatile u_int32_t *plllcnt;

	switch (pll) {
		case 0:
			pllpms = &rPLL0PMS;
			plllcnt = &rPLL0LCNT;
			break;
		case 1:
			pllpms = &rPLL1PMS;
			plllcnt = &rPLL1LCNT;
			break;
		case 2:
			pllpms = &rPLL2PMS;
			plllcnt = &rPLL2LCNT;
			break;
		case 3:
			pllpms = &rPLL3PMS;
			plllcnt = &rPLL3LCNT;
			break;
		default:
			return;
	}

	*pllpms = (vsel << 30) | (p << 24) | (m << 8) | (s << 0);	// set p/m/s values
	*plllcnt = 300 * 24;						// set lock counter for 300us
	rPLLCON |= (1<<(0+pll));					// pll power up
#if !SUPPORT_FPGA
	while ((rPLLLOCK & (1 << pll)) == 0);				// wait for pll to lock
#endif /* ! SUPPORT_FPGA */
	*pllpms = (vsel << 30) | (p << 24) | (m << 8) | (s << 0);	// set p/m/s values again
	rPLLCON |= (1<<(16+pll));					// pll select output
}

int clocks_set_default(void)
{
	uint32_t cnt;

	/* Change all the clocks to something safe */
	clocks_quiesce();
	
	/* set up the clock tree to our default setting */
	clks[CLKGEN_CLK_OSC] = OSC_FREQ;
	
#ifdef PLL0_T
	set_pll(0, PLL0_P, PLL0_M, PLL0_S, PLL0_VSEL);
#endif
	
#ifdef PLL1_T
	set_pll(1, PLL1_P, PLL1_M, PLL1_S, PLL2_VSEL);
#endif
	
#ifdef PLL2_T
	set_pll(2, PLL2_P, PLL2_M, PLL2_S, PLL2_VSEL);
#endif

#ifdef PLL3_T
	set_pll(3, PLL3_P, PLL3_M, PLL3_S, PLL3_VSEL);
#endif

	// Use get_pll() to establish the frequencies (unconfigured PLLs will bypass OSC)
	for (cnt = 0; cnt < 4; cnt++) plls[cnt] = get_pll(cnt);
	
	perf_level = kPerformanceHigh;
	perf_div = HIGH_PERF_DIV_N;
	
	clks[CLKGEN_CLK_SCLK] = plls[SCLK_PLL] / perf_div;
	clks[CLKGEN_CLK_FCLK] = clks[CLKGEN_CLK_SCLK] / FCLK_DIV_N;
	clks[CLKGEN_CLK_DCLK] = clks[CLKGEN_CLK_SCLK] / DCLK_DIV_N;
	clks[CLKGEN_CLK_ACLK] = clks[CLKGEN_CLK_SCLK] / ACLK_DIV_N;
	clks[CLKGEN_CLK_HCLK] = clks[CLKGEN_CLK_SCLK] / HCLK_DIV_N;
	clks[CLKGEN_CLK_PCLK] = clks[CLKGEN_CLK_SCLK] / PCLK_DIV_N;
	clks[CLKGEN_CLK_NCLK] = clks[CLKGEN_CLK_OSC] / NCLK_DIV_N;
	clks[CLKGEN_CLK_USBPHY] = clks[CLKGEN_CLK_OSC];
	
	// switch the useful clocks back to their plls
	rCLKCON1 = 
		   (PRESCALE_EN(FCLK_DIV_N)<<29)|(CLK_DIV_N(FCLK_DIV_N)<<24) |		// FCLK
		   (PRESCALE_EN(DCLK_DIV_N)<<23)|(CLK_DIV_N(DCLK_DIV_N)<<18) |		// DCLK
		   (PRESCALE_EN(HCLK_DIV_N)<<17)|(CLK_DIV_N(HCLK_DIV_N)<<12) |		// HCLK
		   (PRESCALE_EN(PCLK_DIV_N)<<11)|(CLK_DIV_N(PCLK_DIV_N)<< 6) |		// PCLK
		   (PRESCALE_EN(ACLK_DIV_N)<< 5)|(CLK_DIV_N(ACLK_DIV_N)<< 0);		// ACLK
	// must set divider before switching to PLL because of delay in divider taking effect
	// <rdar://problem/11951206> Race condition in CLKCON register setting causes unpredictable behavior
	rCLKCON0 = ((perf_div-1)<<0);
	rCLKCON0 |= ((SCLK_PLL+1)<<12);
	rCLKCON2 = (0<<15)|(1<<31);				// CLK_OUT0 enable, CLK_OUT1 disable
	rCLKCON3 = (1<<31)|(2<<28)|(7<<16)			// VCLK_DAC source PLL2, div 7+1, disable
		 | (1<<15)|(3<<12)|(0<<4)|(0<<0);		// RGBOUT_CLK source PLL3, div 1, disable
	rCLKCON4 = (1<<31)|(0<<28)|(0<<16)			// MCLK1 source OSC,  div 1, disable
		|(1<<15)|(3<<12)|(0<<4)|(0<<0);			// MCLK0 source PLL3, div 1, disable
	rCLKCON5 = (1<<31)|(0<<28)|(0<<16)			// MCLK3 source OSC, div 1, disable
		|(1<<15)|(0<<12)|(0<<0); 				// MCLK2 source OSC, div 1, disable
		
	rCLKCON6 = (0<<28)|((NCLK_DIV_N-1)<<16)		// NCLK source OSC, enable
		|(1<<15)|(2<<12);						// MFC_CLK source PLL1, div 0+1, disable
	
	rCLKCON7 = (0x1F<<27)|(1<<24)|(3<<16)		// VMHCLKx source PLL0, div 3+1, disable
		|(1<<15)|(1<<12)|(2<<0);		// SCALER source PLL0, div 2+1, disable
	
#if SUPPORT_FPGA
	clks[CLKGEN_CLK_SCLK] = 10000000;
	clks[CLKGEN_CLK_FCLK] = 10000000;
	clks[CLKGEN_CLK_DCLK] = 10000000;
	clks[CLKGEN_CLK_ACLK] = 10000000;
	clks[CLKGEN_CLK_HCLK] = 10000000;
	clks[CLKGEN_CLK_PCLK] = 10000000;
	clks[CLKGEN_CLK_NCLK] = 10000000;
	clks[CLKGEN_CLK_USBPHY] = 12000000;
#endif
	
	return 0;
}

void clocks_quiesce(void)
{
	// Turn off non-critical device clocks
	rCGCON0 = 0x00001CBF;	// HCLK to VROM
	rCGCON1 = 0x3FC5EFFF;	// PCLKs to DREX, Timer64, Cortex-A5 Debug and Sys, AXI, and GPIO
	rCGCON2 = 0x0000001F;
	rCGCON3 = 0x00000000;	// ACLKs to AXI, XMC, and SRAM
	rCGCON4 = 0x003DFEFF;	// NCLK to DREX and Timer64 on

	// Set OSC to select the 12/24MHz clock
	//rPLLCON &= ~(1 << 8); // FIXME

	// Set all of the clocks to use OSC bypass.
	// Disable the clocks that will not be used.
	rCLKCON0 &= ~(0x3<<12);		// Use OSC bypass for SCLK ahead of turning off divider
	rCLKCON0 = (1<<30);		// MCLK disable, SCLK enable, use OSC bypass, disable prescalar
	rCLKCON1 = 0;			// FCLK/HCLK/PCLK/ACLK enable, use OSC bypass, disable prescalar
	rCLKCON2 = (1<<31)|(1<<15);	// CLK_OUT1/CLK_OUT0 disable
	rCLKCON3 = (1<<31)|(1<<15);	// VCLK_DAC/RGBOUT_VCLK disable
	rCLKCON4 = (1<<31)|(1<<15);	// MCLK1/MCLK0 disable
	rCLKCON5 = (1<<31)|(1<<15);	// MCLK3/MCLK2 disable
	rCLKCON6 = (1<<15);		// NCLK enable, MFC_CLK disable
	rCLKCON7 = (0x1F<<27)|(1<<15);	// VMHCLKx/SCALER disable

	// Bypass and shut down all PLLs
	rPLLCON &= ~0xF0000;
	rPLLCON &= ~0x0000F;
}

u_int32_t clocks_get_performance_divider(void)
{
	return perf_div;
}

u_int32_t clocks_set_performance(u_int32_t performance_level)
{
	u_int32_t old_perf_level = perf_level;

	switch (performance_level) {
		default : performance_level = kPerformanceHigh;
		case kPerformanceHigh: perf_div = HIGH_PERF_DIV_N; break;
		case kPerformanceMedium: perf_div = MEDIUM_PERF_DIV_N; break;
		case kPerformanceLow: perf_div = LOW_PERF_DIV_N; break;

		case kPerformanceMemory :
			if (old_perf_level == kPerformanceHigh) goto done;

			perf_div = MEMORY_PERF_DIV_N;
			break;
	}

	perf_level = performance_level;

	if (old_perf_level == kPerformanceMemory) {
		rCLKCON1 =
			   (PRESCALE_EN(HCLK_DIV_N)<<22)|(CLK_DIV_N(HCLK_DIV_N)<<17) |			// HCLK
			   (PRESCALE_EN(PCLK_DIV_N)<<14)|(CLK_DIV_N(PCLK_DIV_N)<< 9) |			// PCLK
			   (PRESCALE_EN(ACLK_DIV_N)<< 6)|(CLK_DIV_N(ACLK_DIV_N)<< 1);			// ACLK
	}

	rCLKCON0 = (rCLKCON0 & ~0xF) | ((perf_div - 1) & 0xF);

	if (perf_level == kPerformanceMemory) {
		rCLKCON1 =
			   (PRESCALE_EN(1)<<22)|(CLK_DIV_N(1)<<17) |					// HCLK
			   (PRESCALE_EN(1)<<14)|(CLK_DIV_N(1)<< 9) |					// PCLK
			   (PRESCALE_EN(1)<< 6)|(CLK_DIV_N(1)<< 1);					// ACLK
	}
done:
	return old_perf_level;
}

u_int32_t clock_get_frequency(int clock)
{
	switch (clock) {
		case CLK_CPU:
		case CLK_FCLK:
			return clks[CLKGEN_CLK_FCLK];
		case CLK_DCLK:
		case CLK_MEM:
			return clks[CLKGEN_CLK_DCLK];
		case CLK_ACLK:
			return clks[CLKGEN_CLK_ACLK];
		case CLK_HCLK:
		case CLK_BUS:
			return clks[CLKGEN_CLK_HCLK];
		case CLK_PERIPH:
		case CLK_PCLK:
			return clks[CLKGEN_CLK_PCLK];
		case CLK_NCLK:
		case CLK_FIXED:
			return clks[CLKGEN_CLK_NCLK];
		case CLK_TIMEBASE:
#if SUPPORT_FPGA
			// <rdar://problem/10442644> Rig clock frequency data for G1 FPGA
			return clks[CLKGEN_CLK_NCLK] / 2;
#else
			return clks[CLKGEN_CLK_NCLK];
#endif
		case CLK_USBPHYCLK:
			return clks[CLKGEN_CLK_USBPHY];
		case CLK_MCLK:
		default:
			return 0;
	}
}

void platform_reset(bool panic)
{
#if WITH_BOOT_STAGE
	if (!panic) boot_set_stage(kPowerNVRAMiBootStageOff);
#endif

	/* reset ourselves */
	wdt_chip_reset();

	while (1);
}


void platform_system_reset(bool panic)
{
	dprintf(DEBUG_INFO, "no platform system reset, using normal reset\n");
	platform_reset(panic);
}

struct clock_gate_entry {
	u_int32_t	gccon0_mask;
	u_int32_t	gccon1_mask;
	u_int32_t	gccon2_mask;
	u_int32_t	gccon3_mask;
	u_int32_t	gccon4_mask;
};

static const struct clock_gate_entry clock_gate_map[] =
{
	[CLK_AES] =	{ 0x00000080, 0x00000000, 0x00000000, 0x00000000, 0x00000000 },
	[CLK_CHIPID] =	{ 0x00000000, 0x00020000, 0x00000000, 0x00000000, 0x00000000 },
	[CLK_DMAC0] =	{ 0x00000800, 0x00000000, 0x00000000, 0x00000000, 0x00000000 },
	[CLK_DMAC1] =	{ 0x00001000, 0x00000000, 0x00000000, 0x00000000, 0x00000000 },
	[CLK_GPIO] =	{ 0x00000000, 0x00001000, 0x00000000, 0x00000000, 0x00000000 },
	[CLK_IIC0] =	{ 0x00000000, 0x00000010, 0x00000000, 0x00000000, 0x00001000 },
	[CLK_IIC1] =	{ 0x00000000, 0x00000020, 0x00000000, 0x00000000, 0x00002000 },
	[CLK_IIC2] =	{ 0x00000000, 0x00000040, 0x00000000, 0x00000000, 0x00004000 },
	[CLK_PKE] =	{ 0x00000000, 0x00010000, 0x00000000, 0x00000000, 0x00000000 },
	[CLK_SHA1] =	{ 0x00000001, 0x00000000, 0x00000000, 0x00000000, 0x00000000 },
	[CLK_SPI0] =	{ 0x00000000, 0x00000004, 0x00000000, 0x00000000, 0x00008000 },
	[CLK_SPI1] =	{ 0x00000000, 0x00000008, 0x00000000, 0x00000000, 0x00010000 },
	[CLK_TIMER] =	{ 0x00000000, 0x7FC00000, 0x00000000, 0x00000000, 0x000001FF },
	[CLK_UART0] =	{ 0x00000000, 0x00002000, 0x00000000, 0x00000000, 0x00000200 },
	[CLK_UART1] =	{ 0x00000000, 0x00004000, 0x00000000, 0x00000000, 0x00000400 },
	[CLK_UART2] =	{ 0x00000000, 0x00008000, 0x00000000, 0x00000000, 0x00000800 },
	[CLK_USBOTG0] =	{ 0x00000004, 0x00000000, 0x00000000, 0x00000000, 0x00000000 },
	[CLK_USBOTG1] =	{ 0x00000008, 0x00000000, 0x00000000, 0x00000000, 0x00000000 },
	[CLK_USBPHY] =	{ 0x00000020, 0x00000800, 0x00000000, 0x00000000, 0x00000000 },
	[CLK_ROSC] = 	{ 0x00000000, 0x00000000, 0x00000001, 0x00000000, 0x00000000 },
};
#define CLOCK_GATE_MAP_COUNT (sizeof(clock_gate_map) / sizeof(struct clock_gate_entry))

void clock_gate(int device, bool enable)
{
	const struct clock_gate_entry	*entry = 0;

	if (device > (int)CLOCK_GATE_MAP_COUNT)
		panic("Invalid clock gate");

	entry = &clock_gate_map[device];

	if (enable) {
		rCGCON0 &= ~entry->gccon0_mask;
		rCGCON1 &= ~entry->gccon1_mask;
		rCGCON2 &= ~entry->gccon2_mask;
		rCGCON3 &= ~entry->gccon3_mask;
		rCGCON4 &= ~entry->gccon4_mask;
	} else {
		rCGCON0 |= entry->gccon0_mask;
		rCGCON1 |= entry->gccon1_mask;
		rCGCON2 |= entry->gccon2_mask;
		rCGCON3 |= entry->gccon3_mask;
		rCGCON4 |= entry->gccon4_mask;
	}
}

