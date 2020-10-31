/*
 * Copyright (C) 2009-2012 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */
#include <debug.h>
#include <drivers/power.h>
#include <platform.h>
#include <platform/clocks.h>
#include <platform/gpio.h>
#include <platform/gpiodef.h>
#include <platform/power.h>
#include <platform/timer.h>
#include <platform/soc/hwclocks.h>
#include <platform/soc/miu.h>
#include <platform/soc/pmgr.h>
#include <platform/soc/chipid.h>
#include <sys/boot.h>
#include <target.h>

#if !APPLICATION_EMBEDDEDIOP

#define PLL_VCO_TARGET(pllx) (2ULL * pllx##_O * pllx##_M / pllx##_P)
#define PLL_FREQ_TARGET(pllx) (2ULL * pllx##_O * pllx##_M / pllx##_P / (1 << pllx##_S))

static u_int32_t clk_divs_bypass[PMGR_CLK_CFG_COUNT] = {
	0x80008421, 0x80000000, 0x80000001, 0x80000001,		// cpu, mcu_fixed, mcu, pclk1
	0x80000001, 0x80000001, 0x80000001, 0x80000001,		// prediv0, prediv1, prediv2, prediv3
	0x80000001, 0x80000001, 0x80000001, 0x80000000,		// prediv4, prediv5, prediv6, managed0
	0x80000000, 0x80000000, 0x80000000, 0x80000000,		// managed1, managed2, managed3, managed4
	0x80000001, 0x80000001, 0x80000001, 0xB0000001,		// vid1, medium0, vid0, i2c
	0x80000001, 0x80000001, 0x80000001, 0x80000001,		// sdio, mipi_dsi, audio, hpark_pclk0
	0x80000001, 0x80000001, 0x80000001, 0x80000001,		// hpark_tclk, uperf, debug, hperf_rt
	0x80000001, 0x80000001, 0x80000001, 0x80000001,		// gfx, gfx_slc, hperf_nrt, isp
	0x80000001, 0x80000001, 0x80000001, 0x80000001,		// iop, cdio, lperfs, pclk0
	0x80000001, 0x80000001, 0x8000001f, 0x80000001,		// pclk2, pclk3, medium1, spi0
	0x80000001, 0x80000001, 0x80000001, 0x80000001,		// spi1, spi2, spi3, spi4
	0x80000021, 0x80000001, 0x80000001, 0x80000001,		// sleep, usbphy, usbohci, usb12
	0x80000001, 0x80000001, 0x80000000, 0x80000001,		// nco_ref0, nco_ref1, venc_mtx, venc
};

static u_int32_t perf_state_bypass[3] = {
	0x01010101, 0x03010101, 0x00000001,			// defaults, slow mcu_cfg
};

struct perf_info {
	u_int8_t perf_state;
	u_int8_t perf_div;
};

#if APPLICATION_IBOOT

#ifndef TARGET_EMA_CTL_CPU_SEL
#define TARGET_EMA_CTL_CPU_SEL 1
#endif

#ifndef TARGET_CPU_SOURCE
#define TARGET_CPU_SOURCE 1
#endif

#ifndef TARGET_SPI2_SOURCE
#define TARGET_SPI2_SOURCE 3
#endif

#ifndef TARGET_MIPI_DSI_SOURCE
#error  TARGET_MIPI_DSI_SOURCE undefined
#endif

#ifndef TARGET_MANAGED2H_SOURCE
#error TARGET_MANAGED2H_SOURCE undefined
#endif

#ifndef TARGET_MANAGED2L_SOURCE
#error TARGET_MANAGED2L_SOURCE undefined
#endif

#ifndef TARGET_MANAGED2H_DIV
#error TARGET_MANAGED2H_DIV undefined
#endif

#ifndef TARGET_MANAGED2L_DIV
#error TARGET_MANAGED2L_DIV undefined
#endif

#ifndef TARGET_GFX_SOURCE
#error  TARGET_GFX_SOURCE undefined
#endif

#ifndef TARGET_GFX_SLC_SOURCE
#error  TARGET_GFX_SLC_SOURCE undefined
#endif

#ifndef TARGET_CPU_850M

#define PLL0		0
#define PLL0_O		OSC_FREQ
#define PLL0_P		6
#define PLL0_M		250
#define PLL0_S		1
#define PLL0_V		PLL_VCO_TARGET(PLL0)
#define PLL0_T		PLL_FREQ_TARGET(PLL0)

#endif

#if TARGET_CPU_SOURCE == 2
#if TARGET_CPU_850M

#define PLL1		1
#define PLL1_O		OSC_FREQ
#define PLL1_P		12
#define PLL1_M		425
#define PLL1_S		1
#define PLL1_V		PLL_VCO_TARGET(PLL1)
#define PLL1_T		PLL_FREQ_TARGET(PLL1)

#else /* ! TARGET_CPU_850M */

#define PLL1		1
#define PLL1_O		OSC_FREQ
#define PLL1_P		6
#define PLL1_M		200
#define PLL1_S		1
#define PLL1_V		PLL_VCO_TARGET(PLL1)
#define PLL1_T		PLL_FREQ_TARGET(PLL1)

#endif /* TARGET_CPU_850M */
#endif

#if TARGET_DDR_533M

// Defined for SUB_PLATFORM_S5L8947X
// Radar - PR-12088495 PR-12142624 PR-12073123 (P & M values changed)
#define PLL2		2
#define PLL2_O		OSC_FREQ
#define PLL2_P		14
#define PLL2_M		311
#define PLL2_S		1
#define PLL2_V		PLL_VCO_TARGET(PLL2)
#define PLL2_T		PLL_FREQ_TARGET(PLL2)

#else /* TARGET_DDR_533M */

#define PLL2		2
#define PLL2_O		OSC_FREQ
#define PLL2_P		6
#define PLL2_M		200
#define PLL2_S		2
#define PLL2_V		PLL_VCO_TARGET(PLL2)
#define PLL2_T		PLL_FREQ_TARGET(PLL2)

#endif /* TARGET_DDR_533M */

#define PLL3		3
#define PLL3_O		OSC_FREQ
#define PLL3_P		4
#define PLL3_M		171
#define PLL3_S		1
#define PLL3_V		PLL_VCO_TARGET(PLL3)
#define PLL3_T		PLL_FREQ_TARGET(PLL3)

#if (TARGET_CPU_850M && TARGET_DDR_533M)

#define PLL4		4
#define PLL4_O		OSC_FREQ
#define PLL4_P		5
#define PLL4_M		130
#define PLL4_S		1
#define PLL4_V		PLL_VCO_TARGET(PLL4)
#define PLL4_T		PLL_FREQ_TARGET(PLL4)

#endif 

#if TARGET_USE_HSIC

#if TARGET_DDR_533M
#define PLLUSB		USB
#define PLLUSB_O	OSC_FREQ
#define PLLUSB_P	5
#define PLLUSB_M	200
#define PLLUSB_S	2
#define PLLUSB_V	PLL_VCO_TARGET(PLLUSB)
#define PLLUSB_T	PLL_FREQ_TARGET(PLLUSB)
#else
#define PLLUSB		USB
#define PLLUSB_O	OSC_FREQ
#define PLLUSB_P	4
#define PLLUSB_M	160
#define PLLUSB_S	2
#define PLLUSB_V	PLL_VCO_TARGET(PLLUSB)
#define PLLUSB_T	PLL_FREQ_TARGET(PLLUSB)
#endif

#define OHCI_SOURCE	(0)
#define D_MEDIUM1	(0x8000000A)

#else /* ! TARGET_USE_HSIC */

#define OHCI_SOURCE	(1)
#define D_MEDIUM1	(0x00000000)

#endif /* TARGET_USE_HSIC */

#ifndef TARGET_CPU_SOURCE
#error  TARGET_CPU_SOURCE undefined
#else
#define D_CPU		(0x80011041 | ((TARGET_CPU_SOURCE) << 28))
#endif

#define D_CLK_DOUBLER	(0x8000001F)

#ifndef TARGET_MIPI_DSI_SOURCE
#error  TARGET_MIPI_DSI_SOURCE undefined
#else
#define D_MIPI_DSI	(0x80000001 | ((TARGET_MIPI_DSI_SOURCE) << 28))
#endif

#ifndef TARGET_SPI2_SOURCE
#error  TARGET_SPI2_SOURCE undefined
#else
#define D_SPI2		(0x80000001 | ((TARGET_SPI2_SOURCE) << 28))
#endif 

#ifndef TARGET_IOP_SOURCE
#error  TARGET_IOP_SOURCE undefined
#else
#define D_IOP		(0x80000001 | ((TARGET_IOP_SOURCE) << 28))
#endif 

#ifndef TARGET_LPERFS_SOURCE
#error  TARGET_LPERFS_SOURCE undefined
#else
#define D_LPERFS	(0x80000002 | ((TARGET_LPERFS_SOURCE) << 28))
#endif 

#ifndef TARGET_HPERFNRT_SOURCE
#error  TARGET_HPERFNRT_SOURCE undefined
#else
#define D_HPERFNRT	(0x80000001 | ((TARGET_HPERFNRT_SOURCE) << 28))
#endif

#if TARGET_USE_PREDIV4
#define D_PREDIV4   (0x80000000 | (((TARGET_PREDIV4_SOURCE) << 28) | ((TARGET_PREDIV4_DIV) << 0)))
#else
#define D_PREDIV4   (0x00000000)
#endif 

#define D_GFX		(0x80000001 | ((TARGET_GFX_SOURCE) << 28))
#define D_GFX_SLC	(0x80000001 | ((TARGET_GFX_SLC_SOURCE) << 28))

#define D_OHCI		(0x80000001 | ((OHCI_SOURCE) << 28))

#ifndef TARGET_PCLK1_SOURCE
#error TARGET_PCLK1_SOURCE undefined
#else
#define D_PCLK1		(0x80000000 | (((TARGET_PCLK1_SOURCE) << 28) | ((TARGET_PCLK1_DIV) << 0)))
#endif

static u_int32_t clk_divs_active[PMGR_CLK_CFG_COUNT] = {
	D_CPU,      0x80000000, 0x80000001, D_PCLK1,		// cpu, mcu_fixed, mcu, pclk1
	0xA0000005, 0xA0000002, 0xA0000003, 0x00000000,		// prediv0, prediv1, prediv2, prediv3
	D_PREDIV4,  0x00000000, 0xA0000014, 0x80000000,		// prediv4, prediv5, prediv6, managed0
	0x00000000, 0x80000000, 0x80000000, 0x80000000,		// managed1, managed2, managed3, managed4
	0xA0000013, 0x80000004, 0xB0000001, 0x80000008,		// vid1, medium0, vid0, i2c
	0x80000004, D_MIPI_DSI, 0xA0000002, 0x90000004,		// sdio, mipi_dsi, audio, hpark_pclk0
	0xA0000002, 0xA0000007, 0x80000008, 0x80000001,		// hpark_tclk, uperf, debug, hperf_rt
	D_GFX,      D_GFX_SLC,  D_HPERFNRT, 0x80000001,		// gfx, gfx_slc, hperf_nrt, isp
	D_IOP,      0xB0000001, D_LPERFS,   0x80000001,		// iop, cdio, lperfs, pclk0
	0x80000001, 0x80000001, D_MEDIUM1,  0xB0000001,		// pclk2, pclk3, medium1, spi0
	0xB0000001, D_SPI2,     0xB0000001, 0xB0000001,		// spi1, spi2, spi3, spi4
	0x80000314, 0x80000001, D_OHCI,     0x80000002,		// sleep, usbphy, usbohci, usb12
	0xA0000001, 0x00000000, 0x80000000, 0x80000002,		// nco_ref0, nco_ref1, venc_mtx, venc
};

#define PLL_GATES_ACTIVE (1 << 3)

static struct perf_info perf_levels[] = {
	[kPerformanceHigh]   = { kPERF_STATE_IBOOT+0, 1 },
	[kPerformanceMedium] = { kPERF_STATE_IBOOT+1, 2 },
	[kPerformanceLow]    = { kPERF_STATE_IBOOT+2, 4 },
	[kPerformanceMemory] = { kPERF_STATE_IBOOT+4, 4 },
};

// Configure Full Performance MANAGED2 based on TARGET defines
//     TARGETS: n78, n94, k93a, ipad2, j33, s5l8940xfpga, s5l8947xfpga, s5l8942xfpga
#define M_PERF_A0	(0x01000001 | ((((TARGET_MANAGED2H_SOURCE) << 5) | ((TARGET_MANAGED2H_DIV) << 0)) << 16)) // managed3 divide by  1
#define M_PERF_A1	(0x02000002 | ((((TARGET_MANAGED2L_SOURCE) << 5) | ((TARGET_MANAGED2L_DIV) << 0)) << 16)) // managed3 divide by  2
#define M_PERF_A2	(0x04000002 | ((((TARGET_MANAGED2L_SOURCE) << 5) | ((TARGET_MANAGED2L_DIV) << 0)) << 16)) // managed3 divide by  4
#define M_PERF_A3	(0x1F000002 | ((((TARGET_MANAGED2L_SOURCE) << 5) | ((TARGET_MANAGED2L_DIV) << 0)) << 16)) // managed3 divide by 31

#define PERF_STATE_ACTIVE kPERF_STATE_IBOOT

// perf_state_active : These 5 performance state definitions are used in iBoot and as templates 
//                     to then generate the OS Performance Controller states in kext's function  
//                     AppleS5L8940XPerformanceController::generatePerformanceStates
//                               
//                     state 0 : High voltage PERF_STATE template
//                     state 1 : Low voltage PERF_STATE template 
//                     state 2 : Used in iBoot only
//                     state 3 : Frequency managed clocks template 
//                               (dividers of frequency managed clocks must be set to 0x1F)
//                     state 4 : Used in iBoot only for memory calibration
#if SUPPORT_FPGA
static u_int32_t perf_state_active[kPERF_STATE_IBOOT_CNT*3] = {
	// mcu_cfg=3, mcu_clk and mcu_fixed_clk always divide by 1
	M_PERF_A0,  0x03210122, 0x00000021,		// divide by  1
	M_PERF_A1,  0x03210124, 0x00000022,		// divide by  2
	M_PERF_A2,  0x03210124, 0x00000022,		// divide by  4
	M_PERF_A3,  0x03210124, 0x00000022,		// divide by 31
	M_PERF_A2,  0x03210124, 0x00000022,		// divide by  4
};
#else
static u_int32_t perf_state_active[kPERF_STATE_IBOOT_CNT*3] = {
//PERF_STATE_xA, PERF_STATE_xB, PERF_STATE_xC
#if (TARGET_CPU_850M && TARGET_DDR_533M)
	M_PERF_A0,   0x00210141,    0x00000021,		// managed3 divide by  1, mcu_clk /  1, mcu_cfg=0 //533 Mhz
	M_PERF_A1,   0x00210141,    0x00000022,		// managed3 divide by  2, mcu_clk /  1, mcu_cfg=0 //533 Mhz
	M_PERF_A2,   0x02210541,    0x00000022,		// managed3 divide by  4, mcu_clk /  5, mcu_cfg=2 //106 Mhz  
	M_PERF_A3,   0x03210A41,    0x00000022,		// managed3 divide by 31, mcu_clk / 10, mcu_cfg=3 //53 Mhz
	M_PERF_A2,   0x00210141,    0x00000022,		// managed3 divide by  4, mcu_clk /  1, mcu_cfg=0 //533 Mhz
#elif TARGET_DDR_256M
	M_PERF_A0,   0x00440122,    0x00000021,		// managed3 divide by  1, mcu_clk /  1, mcu_cfg=0
	M_PERF_A1,   0x01440224,    0x00000022,		// managed3 divide by  2, mcu_clk /  2, mcu_cfg=1
	M_PERF_A2,   0x02440424,    0x00000022,		// managed3 divide by  4, mcu_clk /  4, mcu_cfg=2
	M_PERF_A3,   0x03440824,    0x00000022,		// managed3 divide by 31, mcu_clk /  8, mcu_cfg=3
	M_PERF_A2,   0x00440124,    0x00000022,		// managed3 divide by  4, mcu_clk /  1, mcu_cfg=0	
#else
	M_PERF_A0,   0x00210122,    0x00000021,		// managed3 divide by  1, mcu_clk /  1, mcu_cfg=0
	M_PERF_A1,   0x01210224,    0x00000022,		// managed3 divide by  2, mcu_clk /  2, mcu_cfg=1
	M_PERF_A2,   0x02210424,    0x00000022,		// managed3 divide by  4, mcu_clk /  4, mcu_cfg=2
	M_PERF_A3,   0x03210824,    0x00000022,		// managed3 divide by 31, mcu_clk /  8, mcu_cfg=3
	M_PERF_A2,   0x00210124,    0x00000022,		// managed3 divide by  4, mcu_clk /  1, mcu_cfg=0
#endif /* (TARGET_CPU_850M && TARGET_DDR_533M) */
};
#endif

#endif

#if APPLICATION_SECUREROM

#define PLL0		0
#define PLL0_O		OSC_FREQ
#define PLL0_P		6
#define PLL0_M		125
#define PLL0_S		2
#define PLL0_V		PLL_VCO_TARGET(PLL0)
#define PLL0_T		PLL_FREQ_TARGET(PLL0)

#define PLL3		3
#define PLL3_O		OSC_FREQ
#define PLL3_P		4
#define PLL3_M		171
#define PLL3_S		3
#define PLL3_V		PLL_VCO_TARGET(PLL3)
#define PLL3_T		PLL_FREQ_TARGET(PLL3)

static u_int32_t clk_divs_active[PMGR_CLK_CFG_COUNT] = {
	0x90021041, 0x80000001, 0x80000001, 0xA0000003,		// cpu, mcu_fixed, mcu, pclk1
	0xA0000001, 0x00000000, 0x00000000, 0x00000000,		// prediv0, prediv1, prediv2, prediv3
	0x00000000, 0x00000000, 0x00000000, 0x80000001,		// prediv4, prediv5, prediv6, managed0
	0x80000001, 0x80000001, 0x80000001, 0x80000001,		// managed1, managed2, managed3, managed4
	0x00000000, 0x00000000, 0x00000000, 0x00000000,		// vid1, medium0, vid0, i2c
	0x00000000, 0x00000000, 0x80000002, 0x00000000,		// sdio, mipi_dsi, audio, hpark_pclk0
	0x00000000, 0x80000008, 0x80000008, 0x00000000,		// hpark_tclk, uperf, debug, hperf_rt
	0x00000000, 0x00000000, 0x00000000, 0x00000000,		// gfx, gfx_slc, hperf_nrt, isp
	0x00000000, 0x80000001, 0x80000001, 0x80000001,		// iop, cdio, lperfs, pclk0
	0x80000001, 0x80000001, 0x00000000, 0xB0000001,		// pclk2, pclk3, medium1, spi0
	0x80000001, 0x80000001, 0xB0000001, 0x80000001,		// spi1, spi2, spi3, spi4
	0x80000314, 0x80000001, 0x80000001, 0x80000002,		// sleep, usbphy, usbohci, usb12
	0x80000001, 0x80000001, 0x80000001, 0x00000000,		// nco_ref0, nco_ref1, venc_mtx, venc
};
#define PLL_GATES_ACTIVE (1 << 3)

#define PERF_STATE_ACTIVE kPERF_STATE_SECUREROM
static u_int32_t perf_state_active[3] = {
	0x00000004, 0x03010100, 0x00000000,		// managed0, slow mcu_cfg
};

static struct perf_info perf_levels[] = {
	[kPerformanceHigh]   = { kPERF_STATE_SECUREROM, 1 },
	[kPerformanceMedium] = { kPERF_STATE_SECUREROM, 1 },
	[kPerformanceLow]    = { kPERF_STATE_SECUREROM, 1 },
	[kPerformanceMemory] = { kPERF_STATE_SECUREROM, 1 },
};

#endif

/* current clock speeds */
static u_int32_t clks[PMGR_CLK_COUNT];
static u_int32_t *plls = &clks[PMGR_CLK_PLL0];
static u_int32_t perf_level;
static u_int32_t perf_div;

struct clk_parent {
	volatile u_int32_t *divider_reg;
	u_int32_t	   divider_slot;
	u_int8_t	   parents[4];
};

/* Based on PMGR 1.10 */
static const struct clk_parent clk_parents[PMGR_CLK_COUNT] = {
[PMGR_CLK_OSC] =	{ 0,				0, { 0,				0,			0,			0			} },
[PMGR_CLK_PLL0] =	{ 0,				0, { 0,				0,			0,			0			} },
[PMGR_CLK_PLL1] =	{ 0,				0, { 0,				0,			0,			0			} },
[PMGR_CLK_PLL2] =	{ 0,				0, { 0,				0,			0,			0			} },
[PMGR_CLK_PLL3] =	{ 0,				0, { 0,				0,			0,			0			} },
[PMGR_CLK_PLL4] =	{ 0,				0, { 0,				0,			0,			0			} },
[PMGR_CLK_PLLUSB] =	{ 0,				0, { 0,				0,			0,			0			} },
[PMGR_CLK_DOUBLER] =	{ &rPMGR_DOUBLER_CTL,		0, { PMGR_CLK_OSC,		0,			0,			0			} },
[PMGR_CLK_CPU] =	{ &rPMGR_CPU_CLK_CFG,		1, { PMGR_CLK_OSC,		PMGR_CLK_PLL0,		PMGR_CLK_PLL1,		0			} },
[PMGR_CLK_MEM] =	{ &rPMGR_CPU_CLK_CFG,		2, { PMGR_CLK_CPU,		PMGR_CLK_CPU,		PMGR_CLK_CPU,		PMGR_CLK_CPU		} },
[PMGR_CLK_PIO] =	{ &rPMGR_CPU_CLK_CFG,		3, { PMGR_CLK_CPU,		PMGR_CLK_CPU,		PMGR_CLK_CPU,		PMGR_CLK_CPU		} },
[PMGR_CLK_ACP] =	{ &rPMGR_CPU_CLK_CFG,		4, { PMGR_CLK_CPU,		PMGR_CLK_CPU,		PMGR_CLK_CPU,		PMGR_CLK_CPU		} },
[PMGR_CLK_MCU_FIXED] =	{ &rPMGR_MCU_FIXED_CLK_CFG,	1, { PMGR_CLK_OSC,		PMGR_CLK_PLL2,		PMGR_CLK_PLL3,		PMGR_CLK_PLL4		} },
[PMGR_CLK_MCU] =	{ &rPMGR_MCU_CLK_CFG,		1, { PMGR_CLK_MCU_FIXED,	0,			0,			0			} },
[PMGR_CLK_PREDIV0] =	{ &rPMGR_PREDIV0_CLK_CFG,	1, { PMGR_CLK_OSC,		PMGR_CLK_PLL2,		PMGR_CLK_PLL3,		PMGR_CLK_PLL4		} },
[PMGR_CLK_PREDIV1] =	{ &rPMGR_PREDIV1_CLK_CFG,	1, { PMGR_CLK_OSC,		PMGR_CLK_PLL2,		PMGR_CLK_PLL3,		PMGR_CLK_PLL4		} },
[PMGR_CLK_PREDIV2] =	{ &rPMGR_PREDIV2_CLK_CFG,	1, { PMGR_CLK_OSC,		PMGR_CLK_PLL2,		PMGR_CLK_PLL3,		PMGR_CLK_PLL4		} },
[PMGR_CLK_PREDIV3] =	{ &rPMGR_PREDIV3_CLK_CFG,	1, { PMGR_CLK_OSC,		PMGR_CLK_PLL2,		PMGR_CLK_PLL3,		PMGR_CLK_PLL4		} },
[PMGR_CLK_PREDIV4] =	{ &rPMGR_PREDIV4_CLK_CFG,	1, { PMGR_CLK_OSC,		PMGR_CLK_PLL2,		PMGR_CLK_PLL3,		PMGR_CLK_PLL4		} },
[PMGR_CLK_PREDIV5] =	{ &rPMGR_PREDIV5_CLK_CFG,	1, { PMGR_CLK_OSC,		PMGR_CLK_PLL2,		PMGR_CLK_PLL3,		PMGR_CLK_PLL4		} },
[PMGR_CLK_PREDIV6] =	{ &rPMGR_PREDIV6_CLK_CFG,	1, { PMGR_CLK_OSC,		PMGR_CLK_PLL2,		PMGR_CLK_PLL3,		PMGR_CLK_PLL4		} },
[PMGR_CLK_MANAGED0] =	{ &rPMGR_MANAGED0_CLK_CFG,	1, { PMGR_CLK_PREDIV0,		PMGR_CLK_PREDIV1,	PMGR_CLK_PREDIV4,	PMGR_CLK_PREDIV5	} },
[PMGR_CLK_MANAGED1] =	{ &rPMGR_MANAGED1_CLK_CFG,	1, { PMGR_CLK_PREDIV0,		PMGR_CLK_PREDIV1,	PMGR_CLK_PREDIV2,	PMGR_CLK_PREDIV3	} },
[PMGR_CLK_MANAGED2] =	{ &rPMGR_MANAGED2_CLK_CFG,	1, { PMGR_CLK_PREDIV0,		PMGR_CLK_PREDIV1,	PMGR_CLK_PREDIV2,	PMGR_CLK_PREDIV3	} },
[PMGR_CLK_MANAGED3] =	{ &rPMGR_MANAGED3_CLK_CFG,	1, { PMGR_CLK_PREDIV0,		PMGR_CLK_PREDIV1,	PMGR_CLK_PREDIV2,	PMGR_CLK_PREDIV3	} },
[PMGR_CLK_MANAGED4] =	{ &rPMGR_MANAGED4_CLK_CFG,	1, { PMGR_CLK_PREDIV0,		PMGR_CLK_PREDIV1,	PMGR_CLK_PREDIV4,	PMGR_CLK_PREDIV5	} },
[PMGR_CLK_MEDIUM0] =	{ &rPMGR_MEDIUM0_CLK_CFG,	1, { PMGR_CLK_PREDIV0,		PMGR_CLK_PREDIV1,	PMGR_CLK_PREDIV2,	PMGR_CLK_PREDIV3	} },
[PMGR_CLK_MEDIUM1] =	{ &rPMGR_MEDIUM1_CLK_CFG,	1, { PMGR_CLK_PLLUSB,		0,			0,			0			} },
[PMGR_CLK_VID0] =	{ &rPMGR_VID0_CLK_CFG,		1, { PMGR_CLK_PREDIV0,		PMGR_CLK_PREDIV1,	PMGR_CLK_PREDIV2,	PMGR_CLK_PREDIV6	} },
[PMGR_CLK_VID1] =	{ &rPMGR_VID1_CLK_CFG,		1, { PMGR_CLK_OSC,		PMGR_CLK_PLL2,		PMGR_CLK_PLL3,		PMGR_CLK_PLL4		} },
[PMGR_CLK_I2C] =	{ &rPMGR_I2C_CLK_CFG,		1, { PMGR_CLK_PREDIV0,		PMGR_CLK_PREDIV1,	PMGR_CLK_PREDIV2,	PMGR_CLK_OSC		} },
[PMGR_CLK_SDIO] =	{ &rPMGR_SDIO_CLK_CFG,		1, { PMGR_CLK_PREDIV0,		PMGR_CLK_PREDIV1,	PMGR_CLK_PREDIV2,	PMGR_CLK_PREDIV3	} },
[PMGR_CLK_MIPI_DSI] =	{ &rPMGR_MIPI_DSI_CLK_CFG,	1, { PMGR_CLK_PREDIV0,		PMGR_CLK_PREDIV1,	PMGR_CLK_PREDIV2,	PMGR_CLK_PREDIV3	} },
[PMGR_CLK_AUDIO] =	{ &rPMGR_AUDIO_CLK_CFG,		1, { PMGR_CLK_PREDIV0,		PMGR_CLK_PREDIV1,	PMGR_CLK_PREDIV2,	PMGR_CLK_PREDIV3	} },
[PMGR_CLK_HPARK_PCLK] =	{ &rPMGR_HPARK_PCLK0_CLK_CFG,	1, { PMGR_CLK_PREDIV0,		PMGR_CLK_PREDIV1,	PMGR_CLK_PREDIV2,	PMGR_CLK_PREDIV3	} },
[PMGR_CLK_HPARK_TCLK] =	{ &rPMGR_HPARK_TCLK_CLK_CFG,	1, { PMGR_CLK_PREDIV0,		PMGR_CLK_PREDIV1,	PMGR_CLK_PREDIV2,	PMGR_CLK_PREDIV3	} },
[PMGR_CLK_UPERF] =	{ &rPMGR_UPERF_CLK_CFG,		1, { PMGR_CLK_PREDIV0,		PMGR_CLK_PREDIV1,	PMGR_CLK_PREDIV2,	PMGR_CLK_PREDIV3	} },
[PMGR_CLK_DEBUG] =	{ &rPMGR_DEBUG_CLK_CFG,		1, { PMGR_CLK_PREDIV0,		PMGR_CLK_PREDIV1,	PMGR_CLK_PREDIV2,	PMGR_CLK_OSC		} },
[PMGR_CLK_GFX] =	{ &rPMGR_GFX_CLK_CFG,		1, { PMGR_CLK_MANAGED0,		PMGR_CLK_MANAGED1,	PMGR_CLK_MANAGED2,	PMGR_CLK_MANAGED4	} },
[PMGR_CLK_GFX_SLC] =	{ &rPMGR_GFX_SLC_CLK_CFG,	1, { PMGR_CLK_MANAGED0,		PMGR_CLK_MANAGED1,	PMGR_CLK_MANAGED2,	PMGR_CLK_MANAGED4	} },
[PMGR_CLK_HPERFNRT] =	{ &rPMGR_HPERFNRT_CLK_CFG,	1, { PMGR_CLK_MANAGED0,		PMGR_CLK_MANAGED1,	PMGR_CLK_MANAGED2,	PMGR_CLK_MANAGED3	} },
[PMGR_CLK_HPERFRT] =	{ &rPMGR_HPERFRT_CLK_CFG,	1, { PMGR_CLK_MANAGED0,		PMGR_CLK_MANAGED1,	PMGR_CLK_MANAGED2,	PMGR_CLK_MANAGED3	} },
[PMGR_CLK_ISP] =	{ &rPMGR_ISP_CLK_CFG,		1, { PMGR_CLK_MANAGED0,		PMGR_CLK_MANAGED1,	PMGR_CLK_MANAGED2,	PMGR_CLK_MANAGED3	} },
[PMGR_CLK_IOP] =	{ &rPMGR_IOP_CLK_CFG,		1, { PMGR_CLK_MANAGED0,		PMGR_CLK_MANAGED1,	PMGR_CLK_MANAGED2,	PMGR_CLK_MANAGED3	} },
[PMGR_CLK_CDIO] =	{ &rPMGR_CDIO_CLK_CFG,		1, { PMGR_CLK_MANAGED0,		PMGR_CLK_MANAGED1,	PMGR_CLK_MANAGED2,	PMGR_CLK_MANAGED3	} },
[PMGR_CLK_LPERFS] =	{ &rPMGR_LPERFS_CLK_CFG,	1, { PMGR_CLK_MANAGED0,		PMGR_CLK_MANAGED1,	PMGR_CLK_MANAGED2,	PMGR_CLK_MANAGED3	} },
[PMGR_CLK_PCLK0] =	{ &rPMGR_PCLK0_CLK_CFG,		0, { PMGR_CLK_LPERFS,		0,			0,			0			} },
[PMGR_CLK_PCLK1] =	{ &rPMGR_PCLK1_CLK_CFG,		1, { PMGR_CLK_OSC,		PMGR_CLK_PLL2,		PMGR_CLK_PLL3,		PMGR_CLK_PLL4		} },
[PMGR_CLK_PCLK2] =	{ &rPMGR_PCLK2_CLK_CFG,		0, { PMGR_CLK_LPERFS,		0,			0,			0			} },
[PMGR_CLK_PCLK3] =	{ &rPMGR_PCLK3_CLK_CFG,		0, { PMGR_CLK_LPERFS,		0,			0,			0			} },
[PMGR_CLK_SPI0] =	{ &rPMGR_SPI0_CLK_CFG,		0, { PMGR_CLK_MEDIUM0,		PMGR_CLK_MEDIUM1,	0,			PMGR_CLK_OSC		} },
[PMGR_CLK_SPI1] =	{ &rPMGR_SPI1_CLK_CFG,		0, { PMGR_CLK_MEDIUM0,		PMGR_CLK_MEDIUM1,	0,			PMGR_CLK_OSC		} },
[PMGR_CLK_SPI2] =	{ &rPMGR_SPI2_CLK_CFG,		0, { PMGR_CLK_MEDIUM0,		PMGR_CLK_MEDIUM1,	0,			PMGR_CLK_OSC		} },
[PMGR_CLK_SPI3] =	{ &rPMGR_SPI3_CLK_CFG,		0, { PMGR_CLK_MEDIUM0,		PMGR_CLK_MEDIUM1,	0,			PMGR_CLK_OSC		} },
[PMGR_CLK_SPI4] =	{ &rPMGR_SPI4_CLK_CFG,		0, { PMGR_CLK_MEDIUM0,		PMGR_CLK_MEDIUM1,	0,			PMGR_CLK_OSC		} },
[PMGR_CLK_SLOW] =	{ &rPMGR_SLEEP_CLK_CFG,		2, { PMGR_CLK_OSC,		0,			0,			0			} },
[PMGR_CLK_SLEEP] =	{ &rPMGR_SLEEP_CLK_CFG,		1, { PMGR_CLK_SLOW,		0,			0,			0			} },
[PMGR_CLK_USBPHY] =	{ &rPMGR_USBPHY_CLK_CFG,	0, { PMGR_CLK_PLLUSB,		0,			0,			0			} },
[PMGR_CLK_USBOHCI] =	{ &rPMGR_USBOHCI_CLK_CFG,	0, { PMGR_CLK_MEDIUM1,		PMGR_CLK_DOUBLER,	0,			0			} },
[PMGR_CLK_USB12] =	{ &rPMGR_USB12_CLK_CFG,		1, { PMGR_CLK_OSC,		0,			0,			0			} },
[PMGR_CLK_NCO_REF0] =	{ &rPMGR_NCO_REF0_CLK_CFG,	1, { PMGR_CLK_PREDIV0,		PMGR_CLK_PREDIV1,	PMGR_CLK_PREDIV2,	PMGR_CLK_PREDIV3	} },
[PMGR_CLK_NCO_REF1] =	{ &rPMGR_NCO_REF1_CLK_CFG,	1, { PMGR_CLK_PREDIV0,		PMGR_CLK_PREDIV1,	PMGR_CLK_PREDIV2,	PMGR_CLK_PREDIV3	} },
[PMGR_CLK_VENC_MTX] =	{ &rPMGR_VENC_MTX_CLK_CFG,	1, { PMGR_CLK_OSC,		PMGR_CLK_PLL2,		PMGR_CLK_PLL3,		PMGR_CLK_PLL4		} },
[PMGR_CLK_VENC] =	{ &rPMGR_VENC_CLK_CFG,		1, { PMGR_CLK_VENC_MTX,		0,			0,			0			} },
};

static void clocks_get_frequencies(void);
static u_int32_t get_pll(int pll);
static void set_pll(int pll, u_int32_t p, u_int32_t m, u_int32_t s, u_int32_t v);
static void clocks_set_gates(u_int64_t *devices, bool enable);
static void clocks_quiesce_internal(void);
static void update_perf_state(u_int32_t new_perf_state);

void platform_power_init(void)
{

#if (!SUB_PLATFORM_S5L8947X)
	// Set Power Gating Parameters for all the power domains
	rPMGR_PWR_GATE_TIME_A(1)  = (208 << 16);					// CPU0
	rPMGR_PWR_GATE_TIME_B(1)  = (32  << 26);
	rPMGR_PWR_GATE_TIME_A(2)  = (208 << 16);					// CPU1
	rPMGR_PWR_GATE_TIME_B(2)  = (32  << 26);
	rPMGR_PWR_GATE_TIME_A(3)  = (54  << 16);					// SCU
	rPMGR_PWR_GATE_TIME_A(4)  = (36  << 16);					// L2RAM0
	rPMGR_PWR_GATE_TIME_A(5)  = (36  << 16);					// L2RAM1
	rPMGR_PWR_GATE_TIME_A(6)  = (192 << 16) | (2 << 0);				// IOP
	rPMGR_PWR_GATE_TIME_B(6)  = (32 << 26) | (2 << 16) | (2 << 8) | (4 << 0);
	rPMGR_PWR_GATE_TIME_A(7)  = (568 << 16) | (7 << 0);				// GFX
	rPMGR_PWR_GATE_TIME_B(7)  = (2 << 16) | (2 << 8) | (4 << 0);
	rPMGR_PWR_GATE_TIME_A(8)  = (621 << 16) | (6 << 0);				// HPERF-RT
	rPMGR_PWR_GATE_TIME_B(8)  = (2 << 16) | (2 << 8) | (9 << 0);
	rPMGR_PWR_GATE_TIME_A(9)  = (605 << 16) | (9 << 0);				// ISP
	rPMGR_PWR_GATE_TIME_B(9)  = (2 << 16) | (2 << 8) | (4 << 0);
	rPMGR_PWR_GATE_TIME_A(10) = (366 << 16) | (4 << 0);				// HPERF-NRT
	rPMGR_PWR_GATE_TIME_B(10) = (2 << 16) | (2 << 8) | (4 << 0);
	rPMGR_PWR_GATE_TIME_A(11) = (472 << 16) | (6 << 0);				// VDEC
	rPMGR_PWR_GATE_TIME_B(11) = (2 << 16) | (2 << 8) | (4 << 0);
	rPMGR_PWR_GATE_TIME_A(12) = (529 << 16) | (8 << 0);				// VENC
	rPMGR_PWR_GATE_TIME_B(12) = (2 << 16) | (2 << 8) | (4 << 0);
	rPMGR_PWR_GATE_TIME_A(13) = (275 << 16) | (4 << 0);				// FMI
	rPMGR_PWR_GATE_TIME_B(13) = (2 << 16) | (2 << 8) | (4 << 0);
	rPMGR_PWR_GATE_TIME_A(14) = (149 << 16) | (1 << 0);				// HPARK
	rPMGR_PWR_GATE_TIME_B(14) = (2 << 16) | (2 << 8) | (4 << 0);
	
#else	
	// Set Power Gating Parameters for all the power domains
	rPMGR_PWR_GATE_TIME_A(1)  = (180 << 16);					// CPU0
	rPMGR_PWR_GATE_TIME_B(1)  = (32  << 26);
	rPMGR_PWR_GATE_TIME_A(3)  = (36  << 16);					// SCU
	rPMGR_PWR_GATE_TIME_A(4)  = (15  << 16);					// L2RAM0
	rPMGR_PWR_GATE_TIME_A(6)  = (35 << 16) | (1 << 0);				// IOP
	rPMGR_PWR_GATE_TIME_B(6)  = (32 << 26) | (2 << 16) | (2 << 8) | (4 << 0);
	rPMGR_PWR_GATE_TIME_A(7)  = (160 << 16) | (3 << 0);				// GFX
	rPMGR_PWR_GATE_TIME_B(7)  = (2 << 16) | (2 << 8) | (4 << 0);
	rPMGR_PWR_GATE_TIME_A(8)  = (126 << 16) | (3 << 0);				// HPERF-RT
	rPMGR_PWR_GATE_TIME_B(8)  = (2 << 16) | (2 << 8) | (9 << 0);
	rPMGR_PWR_GATE_TIME_A(10) = (76 << 16) | (2 << 0);				// HPERF-NRT
	rPMGR_PWR_GATE_TIME_B(10) = (2 << 16) | (2 << 8) | (4 << 0);
	rPMGR_PWR_GATE_TIME_A(11) = (78 << 16) | (2 << 0);				// VDEC
	rPMGR_PWR_GATE_TIME_B(11) = (2 << 16) | (2 << 8) | (4 << 0);
	rPMGR_PWR_GATE_TIME_A(13) = (33 << 16) | (1 << 0);				// FMI
	rPMGR_PWR_GATE_TIME_B(13) = (2 << 16) | (2 << 8) | (4 << 0);
#endif
	
#if APPLICATION_IBOOT
	/* clear CPU1's reset; it will still be powered down */
	clock_reset_device(CLK_CPU1);
#endif
}

extern void aic_spin(u_int32_t usecs);

void platform_power_spin(u_int32_t usecs)
{
	aic_spin(usecs);
}

int clocks_init(void)
{
#if APPLICATION_IBOOT && (PRODUCT_IBOOT || PRODUCT_IBEC)
	
	u_int32_t cnt;

	clks[PMGR_CLK_OSC] = OSC_FREQ;

	for (cnt = 0; cnt < 6; cnt++) plls[cnt] = get_pll(cnt);

	/* Calculate our initial performance divider based on CPU_DIVISOR */
	perf_div = (rPMGR_CPU_CLK_CFG & rPMGR_CLK_CFG_DIV_MASK) / (clk_divs_active[0] & rPMGR_CLK_CFG_DIV_MASK);

	/* Match the divider to one of the performance levels */
	if (perf_div == perf_levels[kPerformanceHigh].perf_div) perf_level = kPerformanceHigh;
	else if (perf_div == perf_levels[kPerformanceMedium].perf_div) perf_level = kPerformanceMedium;
	else if (perf_div == perf_levels[kPerformanceLow].perf_div) perf_level = kPerformanceLow;

	clocks_get_frequencies();

#endif /* APPLICATION_IBOOT && (PRODUCT_IBOOT || PRODUCT_IBEC) */
	
	return 0;
}

/* clocks_set_default - called by SecureROM, LLB, iBSS main via
   platform_init_setup_clocks, so the current state of the chip is
   either POR, or whatever 'quiesce' did when leaving SecureROM. */
int clocks_set_default(void)
{
	u_int32_t cnt, reg, val, cpu_div;
	volatile u_int32_t *clkcfgs = PMGR_FIRST_CLK_CFG;

	/* Be sure the bypass performance state is set up */
	rPMGR_PERF_STATE_A(kPERF_STATE_BYPASS) = perf_state_bypass[0];
	rPMGR_PERF_STATE_B(kPERF_STATE_BYPASS) = perf_state_bypass[1];
	rPMGR_PERF_STATE_C(kPERF_STATE_BYPASS) = perf_state_bypass[2];

#if APPLICATION_SECUREROM
	rPMGR_PERF_STATE_A(kPERF_STATE_SECUREROM) = perf_state_active[0];
	rPMGR_PERF_STATE_B(kPERF_STATE_SECUREROM) = perf_state_active[1];
	rPMGR_PERF_STATE_C(kPERF_STATE_SECUREROM) = perf_state_active[2];
#endif

#if APPLICATION_IBOOT
	for (cnt = 0; cnt < kPERF_STATE_IBOOT_CNT; cnt++) {
		rPMGR_PERF_STATE_A(kPERF_STATE_IBOOT + cnt) = perf_state_active[(cnt*3) + 0];
		rPMGR_PERF_STATE_B(kPERF_STATE_IBOOT + cnt) = perf_state_active[(cnt*3) + 1];
		rPMGR_PERF_STATE_C(kPERF_STATE_IBOOT + cnt) = perf_state_active[(cnt*3) + 2];
	}

	// Save the PERF_STATE configuration in rPMGR_SCRATCH1
	rPMGR_SCRATCH1 |= PGMR_SET_PERF_STATE_INDEX(PMGR_PERF_STATE_V(0), kPERF_STATE_IBOOT + 0);
	rPMGR_SCRATCH1 |= PGMR_SET_PERF_STATE_INDEX(PMGR_PERF_STATE_V(1), kPERF_STATE_IBOOT + 1);
	rPMGR_SCRATCH1 |= PGMR_SET_PERF_STATE_INDEX(PMGR_PERF_STATE_P, kPERF_STATE_IBOOT + 3);
	rPMGR_SCRATCH1 |= PGMR_SET_PERF_STATE_INDEX(PMGR_PERF_STATE_M(0), kPERF_STATE_IBOOT + 0);
	rPMGR_SCRATCH1 |= PGMR_SET_PERF_STATE_INDEX(PMGR_PERF_STATE_M(1), kPERF_STATE_IBOOT + 1);
	rPMGR_SCRATCH1 |= PGMR_SET_PERF_STATE_INDEX(PMGR_PERF_STATE_M(2), kPERF_STATE_IBOOT + 2);
	rPMGR_SCRATCH1 |= PGMR_SET_PERF_STATE_INDEX(PMGR_PERF_STATE_M(3), kPERF_STATE_IBOOT + 3);
#endif

	/* Change all the clocks to something safe */
	clocks_quiesce_internal();

#if APPLICATION_IBOOT && !SUPPORT_FPGA
	// We must be running at Vnom or greater at this point.  Move to the fast EMA bank
	// so we can update the incorrect reset values in the slow bank.  The fast bank is
	// safe as long as we stay above Vmin.
	rPMGR_EMA_CTL_CPU = TARGET_EMA_CTL_CPU_SEL;
	while (rPMGR_EMA_CTL_CPU & rPMGR_EMA_CTL_CPU_SPIN) ;
	rPMGR_EMA_CTL_SOC = rPMGR_EMA_CTL_SOC_SEL;
	while (rPMGR_EMA_CTL_SOC & rPMGR_EMA_CTL_SOC_SPIN) ;
#endif /* APPLICATION_IBOOT && !SUPPORT_FPGA*/

	clks[PMGR_CLK_OSC] = OSC_FREQ;

#ifdef PLL0_T
	set_pll(0, PLL0_P, PLL0_M, PLL0_S, PLL0_V);
#endif

#ifdef PLL1_T
	set_pll(1, PLL1_P, PLL1_M, PLL1_S, PLL1_V);
#endif

#ifdef PLL2_T
	set_pll(2, PLL2_P, PLL2_M, PLL2_S, PLL2_V);
#endif

#ifdef PLL3_T
	set_pll(3, PLL3_P, PLL3_M, PLL3_S, PLL3_V);
#endif

#ifdef PLL4_T
	set_pll(4, PLL4_P, PLL4_M, PLL4_S, PLL4_V);
#endif

#ifdef PLLUSB_T
	set_pll(5, PLLUSB_P, PLLUSB_M, PLLUSB_S, PLLUSB_V);
#endif

	// Use get_pll() to establish the frequencies (unconfigured PLLs will bypass OSC)
	for (cnt = 0; cnt < 6; cnt++) plls[cnt] = get_pll(cnt);

#if APPLICATION_IBOOT && !TARGET_USE_HSIC
	// turn on clock doubler
	rPMGR_DOUBLER_CTL = D_CLK_DOUBLER;
#if !SUPPORT_FPGA
	while (!(rPMGR_DOUBLER_DEBUG & rPMGR_PLL_DEBUG_ENABLED)) ;
#endif
#endif

	// perf_div needs to be established before touching PMGR_CPU_CLK_CFG
	perf_level = kPerformanceLow;
	perf_div = perf_levels[perf_level].perf_div;

	// Open the active PLL gates
	rPMGR_PLL_GATES = PLL_GATES_ACTIVE;

	// Set all clock dividers to their active values
	// Start with CPU then work backwards
	for (cnt = 0; cnt < PMGR_CLK_CFG_COUNT; cnt++) {
		reg = PMGR_CLK_CFG_COUNT - cnt;
		if (reg == PMGR_CLK_CFG_COUNT) reg = 0;

		// Take care of managed clocks before predivs
		if (reg == PMGR_CLK_NUM(MANAGED0))
			update_perf_state(kPerformanceLow);

		val = clk_divs_active[reg];

		// Factor perf_div into PMGR_CPU_CLK_CFG
		if (reg == PMGR_CLK_NUM(CPU)) {
			cpu_div = val & rPMGR_CLK_CFG_DIV_MASK;
			val &= ~rPMGR_CLK_CFG_DIV_MASK;
			val |= cpu_div * perf_div;
		}
		clkcfgs[reg] = val;
		// Sleep clock needs special attention: <rdar://problem/7556576>
		// instead, we just make sure not to disable it.

		while (clkcfgs[reg] & rPMGR_CLK_CFG_PENDING);
	}

	clocks_get_frequencies();

	return 0;
}

static void update_perf_state(u_int32_t new_perf_level)
{
	u_int32_t val, cpu_div;

	/* Change the CPU speed (factor old perf_div out, multiple new one in) */
	if (perf_levels[new_perf_level].perf_div != perf_div) {
		val = rPMGR_CPU_CLK_CFG;
		cpu_div = val & rPMGR_CLK_CFG_DIV_MASK;
		cpu_div = (cpu_div / perf_div) * perf_levels[new_perf_level].perf_div;
		val = (val & ~rPMGR_CLK_CFG_DIV_MASK) | cpu_div;
		rPMGR_CPU_CLK_CFG = val;
		while (rPMGR_CPU_CLK_CFG & rPMGR_CLK_CFG_PENDING);
		perf_div = perf_levels[new_perf_level].perf_div;
	}

	/* Write the new select value */
	rPMGR_PERF_STATE_CTL = PMGR_PERF_STATE_SEL(perf_levels[new_perf_level].perf_state);

	/* Spin while any pending bits are asserted */
	while (rPMGR_PERF_STATE_CTL & PMGR_PERF_STATE_PENDING);
}

void clocks_quiesce(void)
{
	/* mcu_clk will be changed to bypass clock */
	clks[PMGR_CLK_MCU] = OSC_FREQ;

	/* Change all the clocks to something safe */
	clocks_quiesce_internal();

	/* effectively full performance */
	perf_level = kPerformanceHigh;
	perf_div = perf_levels[kPerformanceHigh].perf_div;
}

static void clock_update_range(u_int32_t first, u_int32_t last, u_int32_t clkdata[])
{
	volatile u_int32_t *clkcfgs = PMGR_FIRST_CLK_CFG;
	u_int32_t val, reg;

	reg = first;
	while (reg <= last) {
		val = clkdata[reg];
		clkcfgs[reg] = val;
		while (clkcfgs[reg] & rPMGR_CLK_CFG_PENDING);
		reg++;
	}
}

static void clocks_quiesce_internal(void)
{
	u_int64_t devices[2];

	// Critical: AIC, Debug, DWI, GPIO, AUDIO, UPERF, CDMA, CDIO,
	//           MCU, L2RAM, SCU, CPU0
	devices[0] = 0x000000205800005DULL;
	devices[1] = 0x0000000000003A00ULL;

	// Turn on critical device clocks
	clocks_set_gates(devices, true);

	// Turn off non-critical device clocks
	clocks_set_gates(devices, false);

	// Simplified from PMGR Spec 1.10 Section 3.13.6 (plus changes from Erik)

	// Reset top-level dividers to bypass
	clock_update_range(PMGR_CLK_NUM(PCLK1), PMGR_CLK_NUM(PREDIV6), clk_divs_bypass);
	clock_update_range(PMGR_CLK_NUM(VID1), PMGR_CLK_NUM(VID1), clk_divs_bypass);
#if APPLICATION_IBOOT
	// Prepare to move memory to bypass clock (ensure not high frequency, enable DLL force mode)
	rPMGR_PERF_STATE_CTL = PMGR_PERF_STATE_SEL(perf_levels[kPerformanceMedium].perf_state);
	while (rPMGR_PERF_STATE_CTL & PMGR_PERF_STATE_PENDING);
	miu_bypass_prep();
#endif
	// Reset managed clocks and mcu, venc_mtx
	rPMGR_PERF_STATE_CTL = PMGR_PERF_STATE_SEL(kPERF_STATE_BYPASS);
	while (rPMGR_PERF_STATE_CTL & PMGR_PERF_STATE_PENDING);
	// Reset PLLs and Doubler
	rPMGR_PLL0_CTL	 = rPMGR_PLL_EXT_BYPASS;
	rPMGR_PLL1_CTL	 = rPMGR_PLL_EXT_BYPASS;
	rPMGR_PLL2_CTL	 = rPMGR_PLL_EXT_BYPASS;
	rPMGR_PLL3_CTL	 = rPMGR_PLL_EXT_BYPASS;
	rPMGR_PLL4_CTL	 = rPMGR_PLL_EXT_BYPASS;
	rPMGR_PLLUSB_CTL = rPMGR_PLL_EXT_BYPASS;
	rPMGR_DOUBLER_CTL = (rPMGR_PLL_ENABLE | rPMGR_PLL_EXT_BYPASS);
#if !SUPPORT_FPGA
	while (!(rPMGR_PLL0_DEBUG   & rPMGR_PLL_DEBUG_BYP_ENABLED)) ;
	while (!(rPMGR_PLL1_DEBUG   & rPMGR_PLL_DEBUG_BYP_ENABLED)) ;
	while (!(rPMGR_PLL2_DEBUG   & rPMGR_PLL_DEBUG_BYP_ENABLED)) ;
	while (!(rPMGR_PLL3_DEBUG   & rPMGR_PLL_DEBUG_BYP_ENABLED)) ;
	while (!(rPMGR_PLL4_DEBUG   & rPMGR_PLL_DEBUG_BYP_ENABLED)) ;
	while (!(rPMGR_PLLUSB_DEBUG & rPMGR_PLL_DEBUG_BYP_ENABLED)) ;
	while (!(rPMGR_DOUBLER_DEBUG & rPMGR_PLL_DEBUG_BYP_ENABLED)) ;
#endif

	// <rdar://problem/8744636> Doubler external bypass uses gated version of 24 MHz
	// Leave Doubler Enable bit set (for OHCI)

	// Open the PLL Gates
	rPMGR_PLL_GATES = (1 << 4) | (1 << 3) | (1 << 2);
	
	// Reset the lower-level clocks
	clock_update_range(PMGR_CLK_NUM(MANAGED0), PMGR_CLK_NUM(VENC), clk_divs_bypass);

	// Clear Enable bit, Doubler will still remain in bypass mode
	rPMGR_DOUBLER_CTL &= ~rPMGR_PLL_ENABLE;
	
	// Reset the CPU clocks
	clock_update_range(PMGR_CLK_NUM(CPU), PMGR_CLK_NUM(MCU), clk_divs_bypass);
}

u_int32_t clocks_set_performance(u_int32_t performance_level)
{
	u_int32_t old_perf_level = perf_level;

	update_perf_state(performance_level);
	perf_level = performance_level;
	return old_perf_level;
}

void clock_get_frequencies(u_int32_t *clocks, u_int32_t count)
{
	u_int32_t cnt = PMGR_CLK_COUNT;

	if (cnt > count) cnt = count;

	memcpy(clocks, clks, cnt * sizeof(u_int32_t));
}

u_int32_t clock_get_frequency(int clock)
{
	switch (clock) {
		case CLK_CPU:
		case CLK_FCLK:
			return clks[PMGR_CLK_CPU];
		case CLK_ACLK:
		case CLK_MEM:
			return clks[PMGR_CLK_MCU];
		case CLK_HCLK:
		case CLK_BUS:
			return clks[PMGR_CLK_PIO];
		case CLK_PERIPH:
		case CLK_PCLK:
			return clks[PMGR_CLK_PCLK0];
		case CLK_FMI:
			return clks[PMGR_CLK_PCLK1];
		case CLK_NCLK:
		case CLK_FIXED:
		case CLK_TIMEBASE:
			return clks[PMGR_CLK_OSC];
		case CLK_USBPHYCLK:
#if SUPPORT_FPGA
			return clks[PMGR_CLK_USBPHY]; /* The reference is special on FPGA */
#else
			return clks[PMGR_CLK_OSC]; /* This is ref_24_clk, not usb_phy_clk */
#endif
		case CLK_NCOREF:
			return clks[PMGR_CLK_NCO_REF0];
		case CLK_VCLK0:
			return clks[PMGR_CLK_VID0];
		case CLK_I2C0:
		case CLK_I2C1:
		case CLK_I2C2:
			return clks[PMGR_CLK_I2C];
		case CLK_MIPI:
			return clks[PMGR_CLK_MIPI_DSI];
		case CLK_MCLK:
		default:
			return 0;
	}
}

void clock_set_frequency(int clock, u_int32_t divider, u_int32_t pll_p, u_int32_t pll_m, u_int32_t pll_s, u_int32_t pll_t)
{
	u_int32_t total_div, prediv6_div, vid0_div;

	switch (clock) {
		case CLK_VCLK0:
			// Calculate the total divider required
			total_div = clks[PMGR_CLK_PLL3] / pll_t;

			// Find the largest prediv6_div that will
			// produce the correct total_div
			for (prediv6_div = 31; prediv6_div > 1; prediv6_div--) {
				if ((total_div % prediv6_div) == 0) break;
			}

			// Calculate vid0_div based on the part of
			// total_div not in prediv6_div
			vid0_div = total_div / prediv6_div;

			// Set the clock dividers to their new values
			rPMGR_PREDIV6_CLK_CFG = (rPMGR_PREDIV6_CLK_CFG & ~0x1f) | prediv6_div;
			rPMGR_VID0_CLK_CFG = (rPMGR_VID0_CLK_CFG & ~0x1f) | vid0_div;

			// Update the list of frequencies
			clks[PMGR_CLK_PREDIV6] = clks[PMGR_CLK_PLL3] / prediv6_div;
			clks[PMGR_CLK_VID0] = clks[PMGR_CLK_PREDIV6] / vid0_div;
			break;
		default:
			break;
	}
}

void clock_gate(int device, bool enable)
{
	volatile u_int32_t *reg = PMGR_FIRST_PS + device;

	if (reg > PMGR_LAST_PS) return;

	// Set the PS field to the requested level
	if (enable) *reg |= 0xF;
	else *reg &= ~0xF;

	// Wait for the PS and ACTUAL_PS fields to be equal
	while ((*reg & 0xF) != ((*reg >> 4) & 0xF));
}

static void clocks_set_gates(u_int64_t *devices, bool enable)
{
	u_int32_t dev, index;
	volatile u_int32_t *devpss = PMGR_FIRST_PS;
	u_int64_t mask = 1, devmask = 0;

	for (dev = 0, index = -1; dev < PMGR_DEV_PS_COUNT; dev++, mask <<= 1) {
		if ((dev % 64) == 0) {
			devmask = devices[++index];
			if (enable == false)
				devmask ^= -1ULL;
			mask = 1;
		}
		// Skip CPUs
		if (dev < PMGR_PS_NUM(SCU)) continue;
		if ((devmask & mask) != 0) {
			if (enable) devpss[dev] |= 0xF;
			else devpss[dev] &= ~0xF;

			// Wait for the PS and ACTUAL_PS fields to be equal
			while ((devpss[dev] & 0xF) != ((devpss[dev] >> 4) & 0xF));
		}
	}
}

void platform_diag_gate_clocks(void)
{
}

void platform_system_reset(bool panic)
{
#if WITH_BOOT_STAGE
	if (!panic) boot_set_stage(kPowerNVRAMiBootStageOff);
#endif

	// Use WDOG pin to cause a PMU reset
	gpio_configure_out(GPIO_SYSTEM_RESET, 1);

	while (1);
}

void platform_reset(bool panic)
{
#if WITH_BOOT_STAGE
	if (!panic) boot_set_stage(kPowerNVRAMiBootStageOff);
#endif

	wdt_chip_reset();

	while (1);
}

void platform_watchdog_tickle(void)
{
	// Varies by target. This layer between is necessary so that
	// we don't go straight from generic code to target.
	target_watchdog_tickle();
}

static const u_int32_t divider_slot_table[8 * 2] = {
	0x00000000, 0,
	0x0000001F, 0,
	0x000003E0, 5,
	0x00007C00, 10,
	0x000F8000, 15,
	0x00001F00, 8,
	0x001F0000, 16,
	0x1F000000, 24
};

static void clocks_get_frequencies(void)
{
#if SUPPORT_FPGA
	u_int32_t cnt;
	u_int32_t freq = OSC_FREQ;

	for (cnt = 0; cnt < PMGR_CLK_COUNT; cnt++)
		clks[cnt] = freq;

	clks[PMGR_CLK_CPU]	= 15000000;
	clks[PMGR_CLK_MCU]	= 10000000;
	clks[PMGR_CLK_MCU_FIXED]= 10000000;
	clks[PMGR_CLK_USBPHY]	= 12000000;

	// keep compiler happy
	cnt = (u_int32_t)clk_parents;
	cnt = (u_int32_t)divider_slot_table;

#else
	volatile u_int32_t *reg;
	u_int32_t cnt, val, src_shift, parent_idx, slot, mask, shift, divider, perf_div_tmp;
	u_int64_t freq;

	for (cnt = 0; cnt < PMGR_CLK_COUNT; cnt++) {
		reg = clk_parents[cnt].divider_reg;
		if (reg == 0) continue;

		switch (cnt) {
			case PMGR_CLK_MANAGED0 :
				src_shift = 5;
				slot = 1;
				val = (*reg & rPMGR_CLK_CFG_ENABLE) | (perf_state_active[0] & ~rPMGR_CLK_CFG_ENABLE);
				break;

			case PMGR_CLK_MANAGED1 :
				src_shift = 13;
				slot = 5;
				val = (*reg & rPMGR_CLK_CFG_ENABLE) | (perf_state_active[0] & ~rPMGR_CLK_CFG_ENABLE);
				break;

			case PMGR_CLK_MANAGED2 :
				src_shift = 21;
				slot = 6;
				val = (*reg & rPMGR_CLK_CFG_ENABLE) | (perf_state_active[0] & ~rPMGR_CLK_CFG_ENABLE);
				break;

			case PMGR_CLK_MANAGED3 :
				src_shift = 29;
				slot = 7;
				val = (*reg & rPMGR_CLK_CFG_ENABLE) | (perf_state_active[0] & ~rPMGR_CLK_CFG_ENABLE);
				break;

			case PMGR_CLK_MANAGED4 :
				src_shift = 5;
				slot = 1;
				val = (*reg & rPMGR_CLK_CFG_ENABLE) | (perf_state_active[1] & ~rPMGR_CLK_CFG_ENABLE);
				break;

			case PMGR_CLK_MCU :
				src_shift = 32;
				slot = 5;
				val = (*reg & rPMGR_CLK_CFG_ENABLE) | (perf_state_active[1] & ~rPMGR_CLK_CFG_ENABLE);
				break;

			case PMGR_CLK_MCU_FIXED :
				src_shift = 21;
				slot = 6;
				val = (*reg & rPMGR_CLK_CFG_ENABLE) | (perf_state_active[1] & ~rPMGR_CLK_CFG_ENABLE);
				break;

			case PMGR_CLK_VENC_MTX :
				src_shift = 5;
				slot = 1;
				val = (*reg & rPMGR_CLK_CFG_ENABLE) | (perf_state_active[2] & ~rPMGR_CLK_CFG_ENABLE);
				break;

			default :
				src_shift = 28;
				slot = clk_parents[cnt].divider_slot;
				val = *reg;
				break;
		}

		if ((val & rPMGR_CLK_CFG_ENABLE) == 0) continue;

		parent_idx = clk_parents[cnt].parents[(val >> src_shift) & 3];
		freq = clks[parent_idx];

		if ((cnt == PMGR_CLK_DOUBLER) && !(val & rPMGR_DOUBLER_EXT_BYPASS))
			freq *= 2;

		if (slot != 0) {
			mask  = divider_slot_table[slot * 2];
			shift = divider_slot_table[slot * 2 + 1];
			divider = (val & mask) >> shift;

			if (divider == 0) continue;

			switch (cnt) {
				case PMGR_CLK_CPU	: perf_div_tmp = perf_div;	break;
				default			: perf_div_tmp = 1;		break;
			}
			freq *= perf_div_tmp;
			freq /= divider;
		}

		clks[cnt] = freq;
	}
#endif
}

static u_int32_t get_pll(int pll)
{
	u_int32_t pllctl;
	u_int64_t freq = 0;

	switch (pll) {
		case 0: pllctl = rPMGR_PLL0_CTL; break;
		case 1: pllctl = rPMGR_PLL1_CTL; break;
		case 2: pllctl = rPMGR_PLL2_CTL; break;
		case 3: pllctl = rPMGR_PLL3_CTL; break;
		case 4: pllctl = rPMGR_PLL4_CTL; break;
		case 5: pllctl = rPMGR_PLLUSB_CTL; break;
		default: goto exit; break;
	}

	if ((pllctl & rPMGR_PLL_ENABLE) == 0) goto exit;

	if ((pllctl & (rPMGR_PLL_EXT_BYPASS | rPMGR_PLL_BYPASS))) {
		freq = OSC_FREQ;
	} else {
		freq = OSC_FREQ * 2;
		freq *= (pllctl >> 3) & 0x3FF;  // *M
		freq /= (pllctl >> 14) & 0x3F;  // /P
		freq /= 1 << ((pllctl >> 0) & 0x07);   // /2^S
	}

exit:
	return freq;
}

static void set_pll(int pll, u_int32_t p, u_int32_t m, u_int32_t s, u_int32_t vco)
{
	volatile u_int32_t *pllctl, *pllparam;
#if (SUB_PLATFORM_S5L8942X || SUB_PLATFORM_S5L8947X)
	u_int32_t vsel;
#else
	u_int32_t afc;
#endif

	switch (pll) {
		case 0: pllctl = &rPMGR_PLL0_CTL; pllparam = &rPMGR_PLL0_PARAM; break;
		case 1: pllctl = &rPMGR_PLL1_CTL; pllparam = &rPMGR_PLL1_PARAM; break;
		case 2: pllctl = &rPMGR_PLL2_CTL; pllparam = &rPMGR_PLL2_PARAM; break;
		case 3: pllctl = &rPMGR_PLL3_CTL; pllparam = &rPMGR_PLL3_PARAM; break;
		case 4: pllctl = &rPMGR_PLL4_CTL; pllparam = &rPMGR_PLL4_PARAM; break;
		case 5: pllctl = &rPMGR_PLLUSB_CTL; pllparam = &rPMGR_PLLUSB_PARAM; break;
		default: return; break;
	}

#if (SUB_PLATFORM_S5L8942X || SUB_PLATFORM_S5L8947X)
	// Find the VSEL setting for the desired VCO frequency
	if (vco < 1400000000UL) vsel = 0; // 960MHz <= VCO <= 1400MHz
	else vsel = 1; // 1400MHz <= VCO <= 2060MHz

	// Set the default lock time (2400 cycles), and enable AFC
	// Note: 2400 requires Fref>=2MHz (P<=12) on H4A.
	*pllparam = rPMGR_PARAM_AFC_EN | rPMGR_PARAM_LOCK_TIME(2400);
	*pllctl = (rPMGR_PLL_ENABLE | rPMGR_PLL_LOAD |
		   rPMGR_PLL_P(p) | rPMGR_PLL_M(m) | rPMGR_PLL_S(s) |
		   rPMGR_PLL_VSEL(vsel));
#else
	// Find the AFC setting for the desired VCO frequency
	afc = 13;
	if (vco < 1100000000UL) afc = 5;
	if (vco > 1500000000UL) afc = 28;

	// Set the default lock time (2400 cycles), and disable AFC and use EXT AFC
	// Note: 2400 requires Fref>=1MHz (P<=24) on H4P/H4G
	*pllparam = rPMGR_PARAM_EXT_AFC(afc) | rPMGR_PARAM_LOCK_TIME(2400);
	*pllctl = (rPMGR_PLL_ENABLE | rPMGR_PLL_LOAD |
		   rPMGR_PLL_P(p) | rPMGR_PLL_M(m) | rPMGR_PLL_S(s));
#endif

#if !SUPPORT_FPGA
	while ((*pllctl & rPMGR_PLL_REAL_LOCK) == 0);				// wait for pll to lock
#endif /* ! SUPPORT_FPGA */
}

#endif

void clock_reset_device(int device)
{
	volatile u_int32_t *reg = PMGR_FIRST_PS + device;

	switch (device) {
		case CLK_CPU1 :
		case CLK_FMI0 :
		case CLK_FMI1 :
		case CLK_FMI2 :
		case CLK_FMI3 :
		case CLK_IOP :
		case CLK_SDIO :
#if SUB_PLATFORM_S5L8947X
		case CLK_HDMI :
#endif
			*reg |= rPMGR_PS_RESET;
			spin(1);
			*reg &= ~rPMGR_PS_RESET;
			break;

		case CLK_MCU:
			// Make sure resets are asserted/deasserted to gfx0, gfx1, hperfrt, hperfnrt
			// <rdar://problem/7269959>
			rPMGR_GFX_SYS_PS |= rPMGR_PS_RESET;
			rPMGR_GFX_CORES_PS |= rPMGR_PS_RESET;
			rPMGR_HPERFNRT_PS |= rPMGR_PS_RESET;
			rPMGR_HPERFRT_PS |= rPMGR_PS_RESET;
			*reg |= rPMGR_PS_RESET;
			spin(1);
			*reg &= ~rPMGR_PS_RESET;
			rPMGR_GFX_SYS_PS &= ~rPMGR_PS_RESET;
			rPMGR_GFX_CORES_PS &= ~rPMGR_PS_RESET;
			rPMGR_HPERFNRT_PS &= ~rPMGR_PS_RESET;
			rPMGR_HPERFRT_PS &= ~rPMGR_PS_RESET;
			break;

		default :
			break;
	}
}

#if WITH_DEVICETREE

static const u_int32_t cpu_srcs[] = { 1, 2, 1 };
static const u_int32_t cpu_divs[] = { 2, 1, 1 };

void pmgr_update_device_tree(DTNode *pmgr_node)
{
	u_int32_t	cnt, tmp, propSize, propSize2, perf_state_config;
	u_int32_t	*states, *configs;
	u_int64_t	pll_freq, period_ns;
	u_int32_t	cpu_volt;
	char		*propName;
	void		*propData;

	// Fill in the voltage1-states and cpu-clk-cfgs properties

	propName  = "voltage-states1";
	if (!FindProperty(pmgr_node, &propName, (void *)&states, &propSize)) {
		panic("pmgr property voltage-states1 is missing");
	}

	propName = "cpu-clk-cfgs";
	if (!FindProperty(pmgr_node, &propName, (void *)&configs, &propSize2)) {
		panic("pmgr property cpu-clk-cfgs is missing");
	}

	if (propSize != (propSize2 * 2)) {
		panic("pmgr properties voltage-states1 (vsPerformanceLimit) and cpu-clk-cfgs are not the same size");
	}
	if (propSize < (3 * 2 * sizeof(u_int32_t))) {
		panic("pmgr property voltage-states1 is too small");
	}

	// Creat a template for the CPU_CLK_CFG values
	tmp = rPMGR_CPU_CLK_CFG & ~(rPMGR_CLK_CFG_SRC_SEL_MASK | rPMGR_CLK_CFG_DIV_MASK);

	// Attempt to create voltage states for each of the possible CPU_CLK_CFGs
	for (cnt = 0; cnt < 3; cnt++) {
		// Get the PLL frequency for this CPU_CLK_CFG
		pll_freq = plls[cpu_srcs[cnt] - 1];
		cpu_volt = chipid_get_cpu_voltage(cnt);
		if (pll_freq == 0) continue;

		// Calculate the period is ns as a 16.16 fixed point number
		period_ns = cpu_divs[cnt] * 1000000000ULL << 16;
		period_ns /= pll_freq;

		// Save the period for this voltage state
		*(states++) = period_ns;
		*(states++) = cpu_volt;

		// Save the CPU_CLK_CFG for this voltage state
		*(configs++) = tmp | rPMGR_CLK_CFG_SRC_SEL(cpu_srcs[cnt]) | rPMGR_CLK_CFG_DIVIDER(cpu_divs[cnt]);
	}

	// Get the PERF_STATE configuration generated at hardware init
	perf_state_config = rPMGR_SCRATCH1;
	if (perf_state_config == 0) return;

	// Fill in the firmware-v-perf-states property
	propName = "firmware-v-perf-states";
	if (FindProperty(pmgr_node, &propName, &propData, &propSize)) {
		if (propSize != (2 * sizeof(u_int32_t))) {
			panic("pmgr property firmware-v-perf-states is the wrong size");
		}
		// Voltage states are in reverse order
		((u_int32_t *)propData)[0] = PGMR_GET_PERF_STATE_INDEX(PMGR_PERF_STATE_V(1), perf_state_config);
		((u_int32_t *)propData)[1] = PGMR_GET_PERF_STATE_INDEX(PMGR_PERF_STATE_V(0), perf_state_config);
	}

	// Fill in the firmware-p-perf-state property
	propName = "firmware-p-perf-state";
	if (FindProperty(pmgr_node, &propName, &propData, &propSize)) {
		if (propSize != (1 * sizeof(u_int32_t))) {
			panic("pmgr property firmware-p-perf-states is the wrong size");
		}
		// There is only one Frequency Managed / Performance state
		((u_int32_t *)propData)[0] = PGMR_GET_PERF_STATE_INDEX(PMGR_PERF_STATE_P, perf_state_config);
	}

	// Fill in the firmware-m-perf-states property
	propName = "firmware-m-perf-states";
	if (FindProperty(pmgr_node, &propName, &propData, &propSize)) {
		if (propSize != (4 * sizeof(u_int32_t))) {
			panic("pmgr property firmware-m-perf-states is the wrong size");
		}
		// Memory states are in the same order
		((u_int32_t *)propData)[0] = PGMR_GET_PERF_STATE_INDEX(PMGR_PERF_STATE_M(0), perf_state_config);
		((u_int32_t *)propData)[1] = PGMR_GET_PERF_STATE_INDEX(PMGR_PERF_STATE_M(1), perf_state_config);
		((u_int32_t *)propData)[2] = PGMR_GET_PERF_STATE_INDEX(PMGR_PERF_STATE_M(2), perf_state_config);
		((u_int32_t *)propData)[3] = PGMR_GET_PERF_STATE_INDEX(PMGR_PERF_STATE_M(3), perf_state_config);
	}
}

#endif
