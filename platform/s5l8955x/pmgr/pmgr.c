/*
 * Copyright (C) 2010-2014 Apple Inc. All rights reserved.
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
#include <platform/soc/chipid.h>
#include <platform/soc/hwclocks.h>
#include <platform/soc/miu.h>
#include <platform/soc/pmgr.h>
#include <sys/boot.h>
#include <target.h>

#if !APPLICATION_EMBEDDEDIOP

#define PLL_VCO_TARGET(pllx) (2ULL * pllx##_O * ((pllx##_M) + 1) / ((pllx##_P) + 1))
#define PLL_FREQ_TARGET(pllx) (2ULL * pllx##_O * ((pllx##_M) + 1) / ((pllx##_P) + 1) / ((pllx##_S) + 1))

static u_int32_t clk_divs_bypass[PMGR_CLK_CFG_COUNT] = {
	0x80000001, 0x00000000, 0x00000000, 0x00000000,		// 0x140: cpu, rsvd, rsvd, rsvd
	0x80000001, 0x80000001, 0x80000001, 0x00000000,		// 0x150: pll2_gate, pll4_gate, pll5_gate, rsvd
	0x00000000, 0x80000001, 0x80000001, 0x80000001,		// 0x160: rsvd, mcu_fixed, mcu, pclk1
	0x80000001, 0x00000000, 0x00000000, 0x00000000,		// 0x170: gfx, rsvd, rsvd, rsvd
	0x00000000, 0x00000000,	0x00000000, 0x00000000, 	// 0x180: rsvd, rsvd, rsvd, rsvd
	0x80000001, 0x80000001, 0x80000001, 0x80000001,		// 0x190: prediv0, prediv1, prediv2, prediv3
	0x80000001, 0x80000001, 0x00000000, 0x00000000,		// 0x1A0: prediv4, prediv5, rsvd, rsvd
	0x00000000, 0x00000000,	0x00000000, 0x00000000, 	// 0x1B0: rsvd, rsvd, rsvd, rsvd
	0x00000000, 0x00000000,	0x00000000, 0x00000000, 	// 0x1C0: rsvd, rsvd, rsvd, rsvd
	0x80000001, 0x80000001, 0x80000001, 0x80000001,		// 0x1D0: venc_mtx, venc, hperf_rt, gfx_sys
	0x80000001, 0x80000001, 0x80000001, 0x80000001,		// 0x1E0: hperf_nrt, nrt_mem, vdec, isp
	0x80000001, 0x80000001, 0x80000001, 0x80000001,		// 0x1F0: iop, cdio, lperfs, pclk0
	0x80000001, 0x80000001, 0x80000001, 0x00000000,		// 0x200: pclk2, pclk3, aes, rsvd
	0x00000000, 0x00000000,	0x00000000, 0x00000000, 	// 0x210: rsvd, rsvd, rsvd, rsvd
	0x00000000, 0x00000000,	0x00000000, 0x00000000, 	// 0x220: rsvd, rsvd, rsvd, rsvd
	0x80000001, 0x80000001, 0x80000001, 0x80000001,		// 0x230: medium0, medium1, vid0, vid1
	0x80000001, 0x80000001, 0x80000001, 0x00000000,		// 0x240: dispout, i2c, sdio, rsvd
	0x80000001, 0x00000000, 0x00000000, 0x00000000,		// 0x250: audio, rsvd, rsvd, rsvd
	0x80000001, 0x80000001, 0x82000001, 0x80000001,		// 0x260: uperf, debug, scc_pwr, scc_dma
	0x80000001, 0x80000001, 0x80000001, 0x80000001,		// 0x270: spi0, spi1, spi2, spi3
	0x80000001, 0x80000001, 0x82000001, 0x82000001,		// 0x280: spi4, slow, sleep, usb_phy0
	0x80000001, 0x80000001, 0x80000001, 0x80000001,		// 0x290: usbohci, usb12, nco_ref0, nco_ref1
	0x80000001, 0x82000001, 0x82000001,			// 0x2A0: nco_ref2, usb_phy1, usb_ehci_free
};

struct dvfm_state {
	u_int32_t	pll_pms;
	u_int16_t	clk_src;
	u_int8_t	cpu_vi;
	u_int8_t	ram_vi;
};

static const struct dvfm_state dvfm_state_bypass = {
	0,							// no PLL
	0,							// use OSC
	0,							// no cpu voltage
	0							// no ram voltage
};

static u_int32_t perf_state_bypass[1 * 5] = {
	0x01000101, 0x01010101, 0x01000301, 0x01010101, 0x00000100,	// defaults, slow mcu_cfg, no voltage change
};

struct perf_info {
	u_int8_t dvfm_state;
	u_int8_t perf_state;
};

#if APPLICATION_IBOOT

#ifndef TARGET_PREDIV5_DIV
#define TARGET_PREDIV5_DIV 5
#endif

#ifndef TARGET_VID0_DIV
#define TARGET_VID0_DIV 1
#endif

#define PLL2		2
#define PLL2_O		OSC_FREQ
#define PLL2_P		2
#define PLL2_M		49
#define PLL2_S		0
#define PLL2_V		PLL_VCO_TARGET(PLL2)
#define PLL2_T		PLL_FREQ_TARGET(PLL2)

#define PLL3		3
#define PLL3_O		OSC_FREQ
#define PLL3_P		1
#define PLL3_M		86
#define PLL3_S		1
#define PLL3_V		PLL_VCO_TARGET(PLL3)
#define PLL3_T		PLL_FREQ_TARGET(PLL3)

#define PLL4		4
#define PLL4_O		OSC_FREQ
#define PLL4_P		3
#define PLL4_M		170
#define PLL4_S		0
#define PLL4_V		PLL_VCO_TARGET(PLL4)
#define PLL4_T		PLL_FREQ_TARGET(PLL4)

#define PLL5		5
#define PLL5_O		OSC_FREQ
#define PLL5_P		2
#define PLL5_M		96
#define PLL5_S		1
#define PLL5_V		PLL_VCO_TARGET(PLL5)
#define PLL5_T		PLL_FREQ_TARGET(PLL5)

#define PLL6		6
#define PLL6_O		OSC_FREQ
#define PLL6_P		0
#define PLL6_M		54
#define PLL6_S		0
#define PLL6_V		PLL_VCO_TARGET(PLL6)
#define PLL6_T		PLL_FREQ_TARGET(PLL6)

#define PLLUSB		USB
#define PLLUSB_O	OSC_FREQ
#define PLLUSB_P	0
#define PLLUSB_M	19
#define PLLUSB_S	0
#define PLLUSB_V	PLL_VCO_TARGET(PLLUSB)
#define PLLUSB_T	PLL_FREQ_TARGET(PLLUSB)

#define D_PREDIV5	(0x90000000 | (TARGET_PREDIV5_DIV))
#define D_VID0		(0x90000000 | (TARGET_VID0_DIV))

#define D_DISABLED	(0x00000001)
#define D_RESERVED	(0x00000000)
// Clocks configured with this value are managed by the perf table
// This value just enables the clock and puts in a dummy divider
#define D_PRF_TBL	(0x80000001)

static const u_int32_t clk_divs_active[PMGR_CLK_CFG_COUNT] = {
	0x80000001, D_RESERVED, D_RESERVED, D_RESERVED,		// 0x140: cpu, rsvd, rsvd, rsvd
	D_DISABLED, 0x80000001, 0x80000001, D_RESERVED,		// 0x150: pll2_gate, pll4_gate, pll5_gate, rsvd
	D_RESERVED, D_PRF_TBL,  D_PRF_TBL,  0xB0000004,		// 0x160: rsvd, mcu_fixed, mcu, pclk1
	D_PRF_TBL,  D_RESERVED, D_RESERVED, D_RESERVED,		// 0x170: gfx, rsvd, rsvd, rsvd
	D_RESERVED, D_RESERVED,	D_RESERVED, D_RESERVED, 	// 0x180: rsvd, rsvd, rsvd, rsvd
	0x90000005, 0x90000004, 0x90000003, 0xA0000001,		// 0x190: prediv0, prediv1, prediv2, prediv3
	D_DISABLED, D_PREDIV5,  D_RESERVED, D_RESERVED,		// 0x1A0: prediv4, prediv5, rsvd, rsvd
	D_RESERVED, D_RESERVED,	D_RESERVED, D_RESERVED, 	// 0x1B0: rsvd, rsvd, rsvd, rsvd
	D_RESERVED, D_RESERVED,	D_RESERVED, D_RESERVED, 	// 0x1C0: rsvd, rsvd, rsvd, rsvd
	D_PRF_TBL,  0x80000002, D_PRF_TBL,  D_PRF_TBL, 		// 0x1D0: venc_mtx, venc, hperf_rt, gfx_sys
	D_PRF_TBL,  D_PRF_TBL,  D_PRF_TBL,  D_PRF_TBL, 		// 0x1E0: hperf_nrt, nrt_mem, vdec, isp
	D_PRF_TBL,  D_PRF_TBL,  D_PRF_TBL,  D_PRF_TBL, 		// 0x1F0: iop, cdio, lperfs, pclk0
	D_PRF_TBL,  D_PRF_TBL,  D_PRF_TBL,  D_RESERVED,		// 0x200: pclk2, pclk3, aes, rsvd
	D_RESERVED, D_RESERVED,	D_RESERVED, D_RESERVED, 	// 0x210: rsvd, rsvd, rsvd, rsvd
	D_RESERVED, D_RESERVED,	D_RESERVED, D_RESERVED, 	// 0x220: rsvd, rsvd, rsvd, rsvd
	D_DISABLED, 0x8000000A, D_VID0,     0x90000013,		// 0x230: medium0, medium1, vid0, vid1
	0x8000000A, 0xB0000001, D_DISABLED, D_RESERVED,		// 0x240: dispout, i2c, sdio, rsvd
	0x80000001, D_RESERVED, D_RESERVED, D_RESERVED,		// 0x250: audio, rsvd, rsvd, rsvd
	0xA0000007, 0xA0000002, 0x80000001, 0xA000001F,		// 0x260: uperf, debug, scc_pwr, scc_dma
	0xB0000001, 0xB0000001, 0xB0000001, 0xB0000001,		// 0x270: spi0, spi1, spi2, spi3
	0xB0000001, 0x80000018, 0x80000014, 0x80000001,		// 0x280: spi4, slow, sleep, usb_phy0
	0x80000001, 0x80000002, 0xA0000001, D_DISABLED,		// 0x290: usbohci, usb12, nco_ref0, nco_ref1
	D_DISABLED, 0x80000001, 0x80000001,			// 0x2A0: nco_ref2, usb_phy1, usb_ehci_free
};

#define DFVM_STATE_ACTIVE kDVFM_STATE_IBOOT
static const struct dvfm_state dvfm_state_active[] = {
	{
		PMGR_PLL_P(0) | PMGR_PLL_M(24) | PMGR_PLL_S(0),		// PLL = 600MHz
		1,						// Use PLL0
		0,						// Use CPU voltage 0
		CHIPID_RAM_VOLTAGE_LOW			// Use low RAM voltage
	},
	{
		PMGR_PLL_P(0) | PMGR_PLL_M(41) | PMGR_PLL_S(0),		// PLL = 1008MHz
		1,						// Use PLL0
		1,						// Use CPU voltage 1
		CHIPID_RAM_VOLTAGE_HIGH			// Use high RAM voltage
	},
	{
		PMGR_PLL_P(0) | PMGR_PLL_M(49) | PMGR_PLL_S(0),		// PLL = 1200MHz
		1,						// Use PLL0
		2,						// Use CPU voltage 2
		CHIPID_RAM_VOLTAGE_HIGH			// Use high RAM voltage
	},
	{
		PMGR_PLL_P(0) | PMGR_PLL_M(57) | PMGR_PLL_S(0),		// PLL = 1392MHz
		1,						// Use PLL0
		4,						// Use CPU voltage 4
		CHIPID_RAM_VOLTAGE_HIGH			// Use high RAM voltage
	}
};

#define PERF_STATE_ACTIVE kPERF_STATE_IBOOT
static const u_int32_t perf_state_active[kPERF_STATE_IBOOT_CNT * 5] = {
#if ! SUPPORT_FPGA
	0x21004441, 0x02012101, 0x21004041, 0x41010121, CHIPID_SOC_VOLTAGE_MED,	// divide by  1, mcu_cfg=0, med voltage
	0x01004541, 0x23420101, 0x61004121, 0x41424242, CHIPID_SOC_VOLTAGE_LOW,	// divide by  2, mcu_cfg=1, low voltage
	0x01004541, 0x26440201, 0x61004222, 0x41424242, CHIPID_SOC_VOLTAGE_LOW,	// divide by  4, mcu_cfg=2, low voltage
	0x01004541, 0x3f1f1f01, 0x61004328, 0x41424242, CHIPID_SOC_VOLTAGE_LOW,	// divide by 31, mcu_cfg=3, low voltage
	0x01004541, 0x23420101, 0x61004041, 0x41424242, CHIPID_SOC_VOLTAGE_MED,	// divide by  4, mcu_cfg=0, med voltage
#else /* SUPPORT_FPGA */
	0x21002441, 0x02012101, 0x21004041, 0x41010121, CHIPID_SOC_VOLTAGE_MED,	// divide by  1, mcu_cfg=0, med voltage
	0x01002541, 0x23420101, 0x41004121, 0x41424242, CHIPID_SOC_VOLTAGE_LOW,	// divide by  2, mcu_cfg=1, low voltage
	0x01002541, 0x26440201, 0x41004222, 0x41424242, CHIPID_SOC_VOLTAGE_LOW,	// divide by  4, mcu_cfg=2, low voltage
	0x01002541, 0x3f1f1f01, 0x41004328, 0x41424242, CHIPID_SOC_VOLTAGE_LOW,	// divide by 31, mcu_cfg=3, low voltage
	0x01002541, 0x23420101, 0x41004041, 0x41424242, CHIPID_SOC_VOLTAGE_MED,	// divide by  4, mcu_cfg=0, med voltage
#endif /* ! SUPPORT_FPGA */
};

static const struct perf_info perf_levels[] = {
	[kPerformanceHigh]   = { kDVFM_STATE_IBOOT+0, kPERF_STATE_IBOOT+0 },
	[kPerformanceMedium] = { kDVFM_STATE_IBOOT+0, kPERF_STATE_IBOOT+1 },
	[kPerformanceLow]    = { kDVFM_STATE_IBOOT+0, kPERF_STATE_IBOOT+2 },
	[kPerformanceMemory] = { kDVFM_STATE_IBOOT+0, kPERF_STATE_IBOOT+4 },
};

#endif /* APPLICATION_IBOOT */

#if APPLICATION_SECUREROM

#define PLL4		4
#define PLL4_O		OSC_FREQ
#define PLL4_P		3
#define PLL4_M		170
#define PLL4_S		3
#define PLL4_V		PLL_VCO_TARGET(PLL4)
#define PLL4_T		PLL_FREQ_TARGET(PLL4)

static const u_int32_t clk_divs_active[PMGR_CLK_CFG_COUNT] = {
	0x80000001, 0x00000000, 0x00000000, 0x00000000,		// 0x140: cpu, rsvd, rsvd, rsvd
	0x00000001, 0x80000001, 0x00000001, 0x00000000,		// 0x150: pll2_gate, pll4_gate, pll5_gate, rsvd
	0x00000000, 0x80000001, 0x80000001, 0xB0000003,		// 0x160: rsvd, mcu_fixed, mcu, pclk1
	0x00000001, 0x00000000, 0x00000000, 0x00000000,		// 0x170: gfx, rsvd, rsvd, rsvd
	0x00000000, 0x00000000,	0x00000000, 0x00000000, 	// 0x180: rsvd, rsvd, rsvd, rsvd
	0x90000004, 0x00000001, 0x00000001, 0x00000001,		// 0x190: prediv0, prediv1, prediv2, prediv3
	0x00000001, 0x00000001, 0x00000000, 0x00000000,		// 0x1A0: prediv4, prediv5, rsvd, rsvd
	0x00000000, 0x00000000,	0x00000000, 0x00000000, 	// 0x1B0: rsvd, rsvd, rsvd, rsvd
	0x00000000, 0x00000000,	0x00000000, 0x00000000, 	// 0x1C0: rsvd, rsvd, rsvd, rsvd
	0x00000001, 0x00000001, 0x00000001, 0x00000001,		// 0x1D0: venc_mtx, venc, hperf_rt, gfx_sys
	0x00000001, 0x00000001, 0x00000001, 0x00000001,		// 0x1E0: hperf_nrt, nrt_mem, vdec, isp
	0x80000001, 0x80000001, 0x80000001, 0x80000001,		// 0x1F0: iop, cdio, lperfs, pclk0
	0x80000001, 0x80000001, 0x80000001, 0x00000000,		// 0x200: pclk2, pclk3, aes, rsvd
	0x00000000, 0x00000000,	0x00000000, 0x00000000, 	// 0x210: rsvd, rsvd, rsvd, rsvd
	0x00000000, 0x00000000,	0x00000000, 0x00000000, 	// 0x220: rsvd, rsvd, rsvd, rsvd
	0x00000001, 0x00000001, 0x00000001, 0x00000001,		// 0x230: medium0, medium1, vid0, vid1
	0x00000001, 0x00000001, 0x00000001, 0x00000000,		// 0x240: dispout, i2c, sdio, rsvd
	0x80000001, 0x00000000, 0x00000000, 0x00000000,		// 0x250: audio, rsvd, rsvd, rsvd
	0x80000002, 0xB0000001, 0x80000001, 0x80000006,		// 0x260: uperf, debug, scc_pwr, scc_dma
	0xB0000001, 0x00000001, 0x00000001, 0xB0000001,		// 0x270: spi0, spi1, spi2, spi3
	0x00000001, 0x80000018, 0x80000014, 0x80000001,		// 0x280: spi4, slow, sleep, usb_phy0
	0x00000001, 0x00000001, 0x00000001, 0x00000001,		// 0x290: usbohci, usb12, nco_ref0, nco_ref1
	0x00000001, 0x00000001, 0x00000001,			// 0x2A0: nco_ref2, usb_phy1, usb_ehci_free
};

#define DFVM_STATE_ACTIVE kDVFM_STATE_SECUREROM
static const struct dvfm_state dvfm_state_active = {
	PMGR_PLL_P(1) | PMGR_PLL_M(74) | PMGR_PLL_S(3),		// PLL = 225MHz
	1,							// Use PLL0
	0,							// no cpu voltage
	0							// no ram voltage
};

#define PERF_STATE_ACTIVE kPERF_STATE_SECUREROM
static const u_int32_t perf_state_active[1 * 5] = {
	0x01000101, 0x01010101, 0x01000301, 0x01010101, 0x00000100,	// defaults, slow mcu_cfg, no voltage change
};

static const struct perf_info perf_levels[] = {
	[kPerformanceHigh]   = { kDVFM_STATE_SECUREROM, kPERF_STATE_SECUREROM },
	[kPerformanceMedium] = { kDVFM_STATE_SECUREROM, kPERF_STATE_SECUREROM },
	[kPerformanceLow]    = { kDVFM_STATE_SECUREROM, kPERF_STATE_SECUREROM },
	[kPerformanceMemory] = { kDVFM_STATE_SECUREROM, kPERF_STATE_SECUREROM },
};

#endif /* APPLICATION_SECUREROM */

/* current clock speeds */
static u_int32_t cpu_clks[kDVFM_STATE_COUNT];
static u_int32_t clks[PMGR_CLK_COUNT];
static u_int32_t *plls = &clks[PMGR_CLK_PLL0];
static u_int32_t perf_level;

struct clk_parent {
	volatile u_int32_t *divider_reg;
	u_int32_t	   divider_type;
	u_int32_t	   divider_offset;
	u_int8_t	   parents[4];
};

/* Based on PMGR 1.10 */
static const struct clk_parent clk_parents[PMGR_CLK_COUNT] = {
[PMGR_CLK_OSC] =	{ 0,				0, 0x00, { 0,			0,			0,			0			} },
[PMGR_CLK_PLL0] =	{ 0,				0, 0x00, { 0,			0,			0,			0			} },
[PMGR_CLK_PLL1] =	{ 0,				0, 0x00, { 0,			0,			0,			0			} },
[PMGR_CLK_PLL2] =	{ 0,				0, 0x00, { 0,			0,			0,			0			} },
[PMGR_CLK_PLL3] =	{ 0,				0, 0x00, { 0,			0,			0,			0			} },
[PMGR_CLK_PLL4] =	{ 0,				0, 0x00, { 0,			0,			0,			0			} },
[PMGR_CLK_PLL5] =	{ 0,				0, 0x00, { 0,			0,			0,			0			} },
[PMGR_CLK_PLL6] =	{ 0,				0, 0x00, { 0,			0,			0,			0			} },
[PMGR_CLK_PLLUSB] =	{ 0,				0, 0x00, { 0,			0,			0,			0			} },
[PMGR_CLK_DOUBLER] =	{ &rPMGR_DOUBLER_CTL,		0, 0x00, { PMGR_CLK_OSC,	0,			0,			0			} },
[PMGR_CLK_PLL2_GATED] =	{ &rPMGR_PLL2_GATE_CFG,		0, 0x00, { PMGR_CLK_PLL2,	0,			0,			0,			} },
[PMGR_CLK_PLL4_GATED] =	{ &rPMGR_PLL4_GATE_CFG,		0, 0x00, { PMGR_CLK_PLL4,	0,			0,			0,			} },
[PMGR_CLK_PLL5_GATED] =	{ &rPMGR_PLL5_GATE_CFG,		0, 0x00, { PMGR_CLK_PLL5,	0,			0,			0,			} },
[PMGR_CLK_MCU_FIXED] =	{ &rPMGR_MCU_FIXED_CLK_CFG,	2, 0x09, { PMGR_CLK_OSC,	PMGR_CLK_PLL2,		PMGR_CLK_PLL3,		PMGR_CLK_PLL4_GATED	} },
[PMGR_CLK_MCU] =	{ &rPMGR_MCU_CLK_CFG,		3, 0x08, { PMGR_CLK_OSC,	PMGR_CLK_PLL2,		PMGR_CLK_PLL3,		PMGR_CLK_PLL4_GATED	} },
[PMGR_CLK_CPU] =	{ &rPMGR_CPU_CLK_CFG,		1, 0x00, { PMGR_CLK_OSC,	PMGR_CLK_PLL0,		PMGR_CLK_PLL1,		PMGR_CLK_MCU		} },
[PMGR_CLK_PREDIV0] =	{ &rPMGR_PREDIV0_CLK_CFG,	1, 0x00, { PMGR_CLK_OSC,	PMGR_CLK_PLL4,		PMGR_CLK_PLL5_GATED,	PMGR_CLK_PLL2_GATED	} },
[PMGR_CLK_PREDIV1] =	{ &rPMGR_PREDIV1_CLK_CFG,	1, 0x00, { PMGR_CLK_OSC,	PMGR_CLK_PLL4,		PMGR_CLK_PLL5_GATED,	PMGR_CLK_PLL2_GATED	} },
[PMGR_CLK_PREDIV2] =	{ &rPMGR_PREDIV2_CLK_CFG,	1, 0x00, { PMGR_CLK_OSC,	PMGR_CLK_PLL4,		PMGR_CLK_PLL5_GATED,	PMGR_CLK_PLL2_GATED	} },
[PMGR_CLK_PREDIV3] =	{ &rPMGR_PREDIV3_CLK_CFG,	1, 0x00, { PMGR_CLK_OSC,	PMGR_CLK_PLL4,		PMGR_CLK_PLL5_GATED,	PMGR_CLK_PLL2_GATED	} },
[PMGR_CLK_PREDIV4] =	{ &rPMGR_PREDIV4_CLK_CFG,	1, 0x00, { PMGR_CLK_OSC,	PMGR_CLK_PLL5,		PMGR_CLK_PLL6,		PMGR_CLK_PLL3		} },
[PMGR_CLK_PREDIV5] =	{ &rPMGR_PREDIV5_CLK_CFG,	1, 0x00, { PMGR_CLK_OSC,	PMGR_CLK_PLL4,		PMGR_CLK_PLL5_GATED,	PMGR_CLK_PLL2_GATED	} },
[PMGR_CLK_GFX] =	{ &rPMGR_GFX_CLK_CFG,		3, 0x01, { PMGR_CLK_OSC,	PMGR_CLK_PLL5,		PMGR_CLK_PLL6,		PMGR_CLK_PLL4		} },
[PMGR_CLK_GFX_SYS] =	{ &rPMGR_GFX_SYS_CLK_CFG,	3, 0x03, { PMGR_CLK_PREDIV0,	PMGR_CLK_PREDIV1,	PMGR_CLK_PREDIV2,	PMGR_CLK_PREDIV3	} },
[PMGR_CLK_HPERFRT] =	{ &rPMGR_HPERFRT_CLK_CFG,	3, 0x00, { PMGR_CLK_PREDIV0,	PMGR_CLK_PREDIV1,	PMGR_CLK_PREDIV2,	PMGR_CLK_PREDIV3	} },
[PMGR_CLK_DISPOUT] =	{ &rPMGR_DISPOUT_CLK_CFG,	1, 0x00, { PMGR_CLK_PREDIV0,	PMGR_CLK_PREDIV1,	PMGR_CLK_PREDIV2,	PMGR_CLK_PREDIV3	} },
[PMGR_CLK_VID0] =	{ &rPMGR_VID0_CLK_CFG,		1, 0x00, { PMGR_CLK_PREDIV0,	PMGR_CLK_PREDIV5,	PMGR_CLK_PREDIV2,	PMGR_CLK_PREDIV3	} },
[PMGR_CLK_VID1] =	{ &rPMGR_VID1_CLK_CFG,		1, 0x00, { PMGR_CLK_OSC,	PMGR_CLK_PLL4,		PMGR_CLK_PLL5_GATED,	PMGR_CLK_PLL2_GATED	} },
[PMGR_CLK_HPERFNRT] =	{ &rPMGR_HPERFNRT_CLK_CFG,	3, 0x0D, { PMGR_CLK_PREDIV0,	PMGR_CLK_PREDIV1,	PMGR_CLK_PREDIV2,	PMGR_CLK_PREDIV3	} },
[PMGR_CLK_NRT_MEM] =	{ &rPMGR_NRT_MEM_CLK_CFG,	3, 0x0C, { PMGR_CLK_PREDIV0,	PMGR_CLK_PREDIV1,	PMGR_CLK_PREDIV2,	PMGR_CLK_PREDIV3	} },
[PMGR_CLK_VENC_MTX] =	{ &rPMGR_VENC_MTX_CLK_CFG,	3, 0x0B, { PMGR_CLK_PREDIV1,	PMGR_CLK_PLLUSB,	PMGR_CLK_PREDIV2,	PMGR_CLK_PREDIV3	} },
[PMGR_CLK_VENC] =	{ &rPMGR_VENC_CLK_CFG,		1, 0x00, { PMGR_CLK_VENC_MTX,	0,			0,			0			} },
[PMGR_CLK_VDEC] =	{ &rPMGR_VDEC_CLK_CFG,		3, 0x0E, { PMGR_CLK_PREDIV0,	PMGR_CLK_PREDIV1,	PMGR_CLK_PREDIV2,	PMGR_CLK_PREDIV3	} },
[PMGR_CLK_ISP] =	{ &rPMGR_ISP_CLK_CFG,		3, 0x04, { PMGR_CLK_PREDIV0,	PMGR_CLK_PREDIV1,	PMGR_CLK_PREDIV2,	PMGR_CLK_PREDIV3	} },
[PMGR_CLK_IOP] =	{ &rPMGR_IOP_CLK_CFG,		3, 0x05, { PMGR_CLK_PREDIV0,	PMGR_CLK_PREDIV1,	PMGR_CLK_PREDIV2,	PMGR_CLK_PREDIV3	} },
[PMGR_CLK_CDIO] =	{ &rPMGR_CDIO_CLK_CFG,		3, 0x06, { PMGR_CLK_PREDIV0,	PMGR_CLK_PREDIV1,	PMGR_CLK_PREDIV2,	PMGR_CLK_PREDIV3	} },
[PMGR_CLK_LPERFS] =	{ &rPMGR_LPERFS_CLK_CFG,	3, 0x07, { PMGR_CLK_PREDIV0,	PMGR_CLK_PREDIV1,	PMGR_CLK_PREDIV2,	PMGR_CLK_PREDIV3	} },
[PMGR_CLK_PCLK0] =	{ &rPMGR_PCLK0_CLK_CFG,		0, 0x00, { PMGR_CLK_LPERFS,	0,			0,			0			} },
[PMGR_CLK_PCLK1] =	{ &rPMGR_PCLK1_CLK_CFG,		1, 0x00, { PMGR_CLK_OSC,	PMGR_CLK_PLL2,		PMGR_CLK_PLL3,		PMGR_CLK_PLL4_GATED	} },
[PMGR_CLK_PCLK2] =	{ &rPMGR_PCLK2_CLK_CFG,		0, 0x00, { PMGR_CLK_LPERFS,	0,			0,			0			} },
[PMGR_CLK_PCLK3] =	{ &rPMGR_PCLK3_CLK_CFG,		0, 0x00, { PMGR_CLK_LPERFS,	0,			0,			0			} },
[PMGR_CLK_AES] =	{ &rPMGR_AES_CLK_CFG,		3, 0x0F, { PMGR_CLK_PREDIV0,	PMGR_CLK_PREDIV1,	PMGR_CLK_PREDIV2,	PMGR_CLK_PREDIV3	} },
[PMGR_CLK_MEDIUM0] =	{ &rPMGR_MEDIUM0_CLK_CFG,	1, 0x00, { PMGR_CLK_PREDIV0,	PMGR_CLK_PREDIV1,	PMGR_CLK_PREDIV2,	PMGR_CLK_PREDIV3	} },
[PMGR_CLK_MEDIUM1] =	{ &rPMGR_MEDIUM1_CLK_CFG,	1, 0x00, { PMGR_CLK_PLLUSB,	0,			0,			0			} },
[PMGR_CLK_I2C] =	{ &rPMGR_I2C_CLK_CFG,		1, 0x00, { PMGR_CLK_PREDIV0,	PMGR_CLK_PREDIV1,	PMGR_CLK_PREDIV3,	PMGR_CLK_OSC		} },
[PMGR_CLK_SDIO] =	{ &rPMGR_SDIO_CLK_CFG,		1, 0x00, { PMGR_CLK_PREDIV0,	PMGR_CLK_PREDIV1,	PMGR_CLK_PREDIV2,	PMGR_CLK_PREDIV3	} },
[PMGR_CLK_AUDIO] =	{ &rPMGR_AUDIO_CLK_CFG,		1, 0x00, { PMGR_CLK_PREDIV0,	PMGR_CLK_PREDIV1,	PMGR_CLK_PREDIV2,	PMGR_CLK_PREDIV3	} },
[PMGR_CLK_UPERF] =	{ &rPMGR_UPERF_CLK_CFG,		1, 0x00, { PMGR_CLK_PREDIV0,	PMGR_CLK_PREDIV1,	PMGR_CLK_PREDIV2,	PMGR_CLK_PREDIV3	} },
[PMGR_CLK_DEBUG] =	{ &rPMGR_DEBUG_CLK_CFG,		1, 0x00, { PMGR_CLK_PREDIV0,	PMGR_CLK_PREDIV1,	PMGR_CLK_PREDIV2,	PMGR_CLK_OSC		} },
[PMGR_CLK_SCC_PWR] =	{ &rPMGR_SCC_PWR_CLK_CFG,	1, 0x00, { PMGR_CLK_PREDIV0,	PMGR_CLK_PREDIV1,	PMGR_CLK_PREDIV2,	PMGR_CLK_PREDIV3	} },
[PMGR_CLK_SCC_DMA] =	{ &rPMGR_SCC_DMA_CLK_CFG,	1, 0x00, { PMGR_CLK_PREDIV0,	PMGR_CLK_PREDIV1,	PMGR_CLK_PREDIV2,	PMGR_CLK_PREDIV3	} },
[PMGR_CLK_SPI0] =	{ &rPMGR_SPI0_CLK_CFG,		0, 0x00, { PMGR_CLK_MEDIUM0,	PMGR_CLK_MEDIUM1,	0,			PMGR_CLK_OSC		} },
[PMGR_CLK_SPI1] =	{ &rPMGR_SPI1_CLK_CFG,		0, 0x00, { PMGR_CLK_MEDIUM0,	PMGR_CLK_MEDIUM1,	0,			PMGR_CLK_OSC		} },
[PMGR_CLK_SPI2] =	{ &rPMGR_SPI2_CLK_CFG,		0, 0x00, { PMGR_CLK_MEDIUM0,	PMGR_CLK_MEDIUM1,	0,			PMGR_CLK_OSC		} },
[PMGR_CLK_SPI3] =	{ &rPMGR_SPI3_CLK_CFG,		0, 0x00, { PMGR_CLK_MEDIUM0,	PMGR_CLK_MEDIUM1,	0,			PMGR_CLK_OSC		} },
[PMGR_CLK_SPI4] =	{ &rPMGR_SPI4_CLK_CFG,		0, 0x00, { PMGR_CLK_MEDIUM0,	PMGR_CLK_MEDIUM1,	0,			PMGR_CLK_OSC		} },
[PMGR_CLK_SLOW] =	{ &rPMGR_SLOW_CLK_CFG,		1, 0x00, { PMGR_CLK_OSC,	0,			0,			0			} },
[PMGR_CLK_SLEEP] =	{ &rPMGR_SLEEP_CLK_CFG,		1, 0x00, { PMGR_CLK_SLOW,	0,			0,			0			} },
[PMGR_CLK_USB_PHY0] =	{ &rPMGR_USB_PHY0_CLK_CFG,	0, 0x00, { PMGR_CLK_PLLUSB,	0,			0,			0			} },
[PMGR_CLK_USBOHCI] =	{ &rPMGR_USBOHCI_CLK_CFG,	0, 0x00, { PMGR_CLK_MEDIUM1,	PMGR_CLK_DOUBLER,	0,			0			} },
[PMGR_CLK_USB12] =	{ &rPMGR_USB12_CLK_CFG,		1, 0x00, { PMGR_CLK_OSC,	0,			0,			0			} },
[PMGR_CLK_NCO_REF0] =	{ &rPMGR_NCO_REF0_CLK_CFG,	1, 0x00, { PMGR_CLK_PREDIV0,	PMGR_CLK_PREDIV1,	PMGR_CLK_PREDIV2,	PMGR_CLK_PREDIV3	} },
[PMGR_CLK_NCO_REF1] =	{ &rPMGR_NCO_REF1_CLK_CFG,	1, 0x00, { PMGR_CLK_PREDIV0,	PMGR_CLK_PREDIV1,	PMGR_CLK_PREDIV2,	PMGR_CLK_PREDIV3	} },
[PMGR_CLK_NCO_REF2] =	{ &rPMGR_NCO_REF2_CLK_CFG,	1, 0x00, { PMGR_CLK_PREDIV0,	PMGR_CLK_PREDIV1,	PMGR_CLK_PREDIV2,	PMGR_CLK_PREDIV3	} },
[PMGR_CLK_USB_PHY1] =	{ &rPMGR_USB_PHY1_CLK_CFG,	1, 0x00, { PMGR_CLK_PLLUSB,	0,			0,			0,			} },
[PMGR_CLK_USB_EHCI_FREE]={ &rPMGR_USB_EHCI_FREE_CLK_CFG, 0, 0x00, { PMGR_CLK_MEDIUM1,	PMGR_CLK_DOUBLER,	0,			0			} },
};

static void init_thermal_sensors(void);
static void clocks_get_frequencies(void);
static u_int32_t get_pll(int pll);
static void set_pll(int pll, u_int32_t p, u_int32_t m, u_int32_t s, u_int32_t v);
static void clocks_set_gates(u_int64_t *devices, bool enable);
static void clocks_quiesce_internal(void);
static void update_perf_state(u_int32_t new_perf_state);

static void wait_for_dev_ps_change(u_int32_t dev);
static void wait_for_pending_apsc_change(void);
static void wait_for_pending_clk_cfg_change(u_int32_t reg);
static void wait_for_pll_bypass_enabled(u_int32_t idx);
static void wait_for_pll_lock(u_int32_t idx);
static void wait_for_doubler_bypass_enabled(void);

void platform_power_init(void)
{
//#if 0 	/* XXX Update for Bali. */

	// Configure SCC CCXPWRCTRL to settings from H5P Tunables Revision 0.25
	rSCC_CCXPWRCTRL = 0x88800000;

	// Set Power Gating Parameters for all the power domains
	// VDD_CPU RAMP_ALL time is larger then expected, rdar://problem/9376764
	rPMGR_PWR_GATE_TIME_A(1)  = (720 << 16);					// CPU0
	rPMGR_PWR_GATE_TIME_A(2)  = (720 << 16);					// CPU1
	rPMGR_PWR_GATE_TIME_A(6)  = (34 << 16) | (1 << 0);				// IOP
	rPMGR_PWR_GATE_TIME_B(6)  = (20 << 26) | (2 << 16) | (2 << 8) | (4 << 0);
	rPMGR_PWR_GATE_TIME_A(7)  = (147 << 16) | (2 << 0);				// GFX
	rPMGR_PWR_GATE_TIME_B(7)  = (2 << 16) | (2 << 8) | (4 << 0);
	rPMGR_PWR_GATE_TIME_A(8)  = (123 << 16) | (2 << 0);				// HPERF-RT
	rPMGR_PWR_GATE_TIME_B(8)  = (2 << 16) | (2 << 8) | (9 << 0);
	rPMGR_PWR_GATE_TIME_A(9)  = (139 << 16) | (2 << 0);				// ISP
	rPMGR_PWR_GATE_TIME_B(9)  = (2 << 16) | (2 << 8) | (4 << 0);
	rPMGR_PWR_GATE_TIME_A(10) = (94 << 16) | (2 << 0);				// HPERF-NRT
	rPMGR_PWR_GATE_TIME_B(10) = (2 << 16) | (4 << 8) | (4 << 0);
	rPMGR_PWR_GATE_TIME_A(11) = (70 << 16) | (1 << 0);				// VDEC
	rPMGR_PWR_GATE_TIME_B(11) = (2 << 16) | (2 << 8) | (4 << 0);
	rPMGR_PWR_GATE_TIME_A(12) = (107 << 16) | (2 << 0);				// VENC
	rPMGR_PWR_GATE_TIME_B(12) = (2 << 16) | (4 << 8) | (4 << 0);
	rPMGR_PWR_GATE_TIME_A(13) = (29 << 16) | (2 << 0);				// FMI
	rPMGR_PWR_GATE_TIME_B(13) = (2 << 16) | (2 << 8) | (4 << 0);
//#endif

	// Enable MCU_FIXED_CLK to be enabled/disabled based on request from MCU
	rPMGR_ENABLE_CLK_GATE     = (1 << 1);

#if APPLICATION_IBOOT
	/* clear CPU1's reset; it will still be powered down */
	clock_reset_device(CLK_CPU1);

	init_thermal_sensors();
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
	
	u_int32_t cnt, tmp;

	clks[PMGR_CLK_OSC] = OSC_FREQ;

	for (cnt = 0; cnt < PMGR_PLL_COUNT; cnt++) plls[cnt] = get_pll(cnt);

	// Find the perf_level using PERF_STATE_CTL
	tmp = (rPMGR_PERF_STATE_CTL >> PMGR_PERF_STATE_SEL_SHIFT) & PMGR_PERF_STATE_SEL_MASK;
	for (perf_level = kPerformanceHigh; perf_level < kPerformanceLow; perf_level++) {
		if (perf_levels[perf_level].perf_state == tmp) break;
	}

	clocks_get_frequencies();

#endif /* APPLICATION_IBOOT && (PRODUCT_IBOOT || PRODUCT_IBEC) */
	
	return 0;
}

/* clocks_set_default - called by SecureROM, LLB, iBSS main via
   platform_init_setup_clocks, so the current state of the chip is
   either POR, or whatever 'quiesce' did when leaving SecureROM. */
int clocks_set_default(void)
{
	u_int32_t cnt, count, reg, val;
	u_int32_t cpu_vid[kDVFM_STATE_COUNT], ram_vid[2], soc_vid[3];
	volatile u_int32_t *clkcfgs = PMGR_FIRST_CLK_CFG;

	clks[PMGR_CLK_OSC] = OSC_FREQ;
	
	count = 0;

	for (cnt = 0; cnt < kDVFM_STATE_COUNT; cnt++) {
		cpu_vid[cnt] = 0;	
	}
	ram_vid[0] = ram_vid[1] = 0;
	soc_vid[0] = soc_vid[1] = soc_vid[2] = 0;

	// Set up the bypass dvfm and performance states
	rPMGR_DVFM_CFG0(kDVFM_STATE_BYPASS) = dvfm_state_bypass.pll_pms;
	rPMGR_DVFM_CFG1(kDVFM_STATE_BYPASS) = dvfm_state_bypass.clk_src << PMGR_DVFM_CFG1_CLK_SRC_SHIFT;
	rPMGR_DVFM_CFG2(kDVFM_STATE_BYPASS) = 0;
	rPMGR_DVFM_CFG3(kDVFM_STATE_BYPASS) = 0;
	rPMGR_DVFM_CFG4(kDVFM_STATE_BYPASS) = 0;
	rPMGR_PERF_STATE_A(kPERF_STATE_BYPASS) = perf_state_bypass[0];
	rPMGR_PERF_STATE_B(kPERF_STATE_BYPASS) = perf_state_bypass[1];
	rPMGR_PERF_STATE_C(kPERF_STATE_BYPASS) = perf_state_bypass[2];
	rPMGR_PERF_STATE_D(kPERF_STATE_BYPASS) = perf_state_bypass[3];
	rPMGR_PERF_STATE_E(kPERF_STATE_BYPASS) = perf_state_bypass[4];

	/* Change all the clocks to something safe */
	clocks_quiesce_internal();

	// Set default PLL parameters and lock time (1260 cycles)
	for (cnt = 0; cnt < PMGR_PLL_COUNT; cnt++) {
		rPMGR_PLL_PARAM(cnt) = 0x9AF10000 | PMGR_PARAM_LOCK_TIME(1260);
	}

#if APPLICATION_SECUREROM
	// Set up the active dvfm and performance states for the SecureROM
	rPMGR_DVFM_CFG0(kDVFM_STATE_SECUREROM) = dvfm_state_active.pll_pms;
	rPMGR_DVFM_CFG1(kDVFM_STATE_SECUREROM) = dvfm_state_active.clk_src << PMGR_DVFM_CFG1_CLK_SRC_SHIFT;
	rPMGR_DVFM_CFG2(kDVFM_STATE_SECUREROM) = 0;
	rPMGR_DVFM_CFG3(kDVFM_STATE_SECUREROM) = 0;
	rPMGR_DVFM_CFG4(kDVFM_STATE_SECUREROM) = 0;
	rPMGR_PERF_STATE_A(kPERF_STATE_SECUREROM) = perf_state_active[0];
	rPMGR_PERF_STATE_B(kPERF_STATE_SECUREROM) = perf_state_active[1];
	rPMGR_PERF_STATE_C(kPERF_STATE_SECUREROM) = perf_state_active[2];
	rPMGR_PERF_STATE_D(kPERF_STATE_SECUREROM) = perf_state_active[3];
	rPMGR_PERF_STATE_E(kPERF_STATE_SECUREROM) = perf_state_active[4];
#endif

#if APPLICATION_IBOOT
	// Find the VID codes for the CPU, RAM and SoC rails
	platform_get_cpu_voltages(kDVFM_STATE_COUNT, cpu_vid);
	platform_get_ram_voltages(2, ram_vid);
	platform_get_soc_voltages(3, soc_vid);
	platform_convert_voltages(0, kDVFM_STATE_COUNT, cpu_vid);
	platform_convert_voltages(16, 2, ram_vid);
	platform_convert_voltages(2, 3, soc_vid);

	count = sizeof(dvfm_state_active) / sizeof(struct dvfm_state);
	// Set up the active dvfm and performance states for iBoot
	for (cnt = 0; cnt < count; cnt++) {
		// Set PLL Configuration
		rPMGR_DVFM_CFG0(kDVFM_STATE_IBOOT + cnt) = dvfm_state_active[cnt].pll_pms;
		
		// Set PLL Source and CPU Voltage Configuration
		val = dvfm_state_active[cnt].clk_src << PMGR_DVFM_CFG1_CLK_SRC_SHIFT;
		val |= (cpu_vid[dvfm_state_active[cnt].cpu_vi] & PMGR_DVFM_CFG1_VOL_MASK) << PMGR_DVFM_CFG1_SAFE_VOL_SHIFT;
		rPMGR_DVFM_CFG1(kDVFM_STATE_IBOOT + cnt) = val;
		
		// Set SRAM Voltage and Temperature Sensor Region Configuration (0mV, 18.75mV & 31.25mV)
		// H5 Tunables Rev 0.63
		val = (0xA << 12) | (0x5 << 6) | (0x0 << 0);
		val |= (ram_vid[dvfm_state_active[cnt].ram_vi] & PMGR_DVFM_CFG2_SRAM_VOL_MASK) << PMGR_DVFM_CFG2_SRAM_VOL_SHIFT;
		rPMGR_DVFM_CFG2(kDVFM_STATE_IBOOT + cnt) = val;
		
		// Set Speed Sensor Configuration
		rPMGR_DVFM_CFG3(kDVFM_STATE_IBOOT + cnt) = 0;
		rPMGR_DVFM_CFG4(kDVFM_STATE_IBOOT + cnt) = 0;
	}

	for (cnt = 0; cnt < kPERF_STATE_IBOOT_CNT; cnt++) {
		// Set the Clock Sources and Dividers
		rPMGR_PERF_STATE_A(kPERF_STATE_IBOOT + cnt) = perf_state_active[(cnt * 5) + 0];
		rPMGR_PERF_STATE_B(kPERF_STATE_IBOOT + cnt) = perf_state_active[(cnt * 5) + 1];
		rPMGR_PERF_STATE_C(kPERF_STATE_IBOOT + cnt) = perf_state_active[(cnt * 5) + 2];
		rPMGR_PERF_STATE_D(kPERF_STATE_IBOOT + cnt) = perf_state_active[(cnt * 5) + 3];

		// Set the SoC Voltage Configuration
		val = perf_state_active[(cnt * 5) + 4];
		reg = val & PMGR_PERF_STATE_E_VOL_MASK;
		val = (val & ~PMGR_PERF_STATE_E_VOL_MASK) |
			((soc_vid[reg] & PMGR_PERF_STATE_E_VOL_MASK) << PMGR_PERF_STATE_E_VOL_SHIFT);
		rPMGR_PERF_STATE_E(kPERF_STATE_IBOOT + cnt) = val;
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

#ifdef PLL2_T
	set_pll(2, PLL2_P, PLL2_M, PLL2_S, PLL2_V);
#endif

#ifdef PLL3_T
	set_pll(3, PLL3_P, PLL3_M, PLL3_S, PLL3_V);
#endif

#ifdef PLL4_T
	set_pll(4, PLL4_P, PLL4_M, PLL4_S, PLL4_V);
#endif

#ifdef PLL5_T
	set_pll(5, PLL5_P, PLL5_M, PLL5_S, PLL5_V);
#endif

#ifdef PLL6_T
	set_pll(6, PLL6_P, PLL6_M, PLL6_S, PLL6_V);
#endif

#ifdef PLLUSB_T
	set_pll(7, PLLUSB_P, PLLUSB_M, PLLUSB_S, PLLUSB_V);
#endif

	// Use get_pll() to establish the frequencies (unconfigured PLLs will bypass OSC)
	for (cnt = 0; cnt < PMGR_PLL_COUNT; cnt++) plls[cnt] = get_pll(cnt);

	perf_level = kPerformanceLow;

	// Set all clock dividers to their active values
	// Start with CPU then work backwards
	for (cnt = 0; cnt < PMGR_CLK_CFG_COUNT; cnt++) {
		reg = PMGR_CLK_CFG_COUNT - cnt;
		if (reg == PMGR_CLK_CFG_COUNT) reg = 0;

		// Take care of managed clocks before predivs
		if (reg == PMGR_FIRST_MANAGED_CLK_NUM)
			update_perf_state(kPerformanceLow);

		val = clk_divs_active[reg];

		clkcfgs[reg] = val;
		// Sleep clock needs special attention: <rdar://problem/7556576>
		// instead, we just make sure not to disable it.

		wait_for_pending_clk_cfg_change(reg);
	}

	clocks_get_frequencies();

#if APPLICATION_IBOOT
	// Configure SoC Voltage Delay for 12uS plus 12.5mV/uS
	rPMGR_PERF_STATE_DELAY =
		(144 << PMGR_PERF_STATE_DELAY_VOL_CHG_FIX_DLY_SHIFT) | (6 << PMGR_PERF_STATE_DELAY_VOL_CHG_VAR_DLY_SHIFT);

	// Configure Sensor Interval to about 1ms and
	// Configure CPU Voltage Delay for 12uS plus 12.5mV/uS
	rPMGR_DVFM_DELAY =
		(0 << 28) | (6 << 18) |
		(144 << PMGR_DVFM_DELAY_VOL_CHG_FIX_DLY_SHIFT) | (6 << PMGR_DVFM_DELAY_VOL_CHG_VAR_DLY_SHIFT);

	// Configure SRAM Voltage Delay for 12uS plus 12.5mV/uS
	rPMGR_DVFM_SRAM_VOL_DELAY =
		(144 << PMGR_DVFM_SRAM_VOL_DELAY_VOL_CHG_FIX_DLY_SHIFT) | (6 << PMGR_DVFM_SRAM_VOL_DELAY_VOL_CHG_VAR_DLY_SHIFT);

	// Set Static Temperature Sensor Configuration
	rPMGR_DVFM_COMMON_CFG =
		((chipid_get_cpu_temp_offset(1) & 0x7F) << 21) |
		((chipid_get_cpu_temp_offset(0) & 0x7F) << 14) |
		(0x47 << 7) | (0x32 << 0);

	// Configure Temperature Sensors
	rPMGR_SENSOR_CMD = (1 << 20) | (1 << 16) | (0x827 << 0);

	// Set Static DVFM Configuration
	rPMGR_DVFM_STA_CTL = 0;

	rPMGR_VOLMAN_CTL = PMGR_VOLMAN_CPU_SW_OFF_TIME_VALUE(45) | PMGR_VOLMAN_BIT_ORDER_MSB | PMGR_VOLMAM_BYTE_SWAP;
#endif

	return 0;
}

static void update_perf_state(u_int32_t new_perf_level)
{
	// Write the new select value for scc and soc
	rPMGR_APSC_STA_CTL = PMGR_APSC_MANUAL_CHANGE(perf_levels[new_perf_level].dvfm_state, perf_levels[new_perf_level].perf_state);
	wait_for_pending_apsc_change();
}

void clocks_quiesce(void)
{
	/* mcu_clk will be changed to bypass clock */
	clks[PMGR_CLK_MCU] = OSC_FREQ;

	/* Change all the clocks to something safe */
	clocks_quiesce_internal();

	/* effectively full performance */
	perf_level = kPerformanceHigh;
}

static void clock_update_range(u_int32_t first, u_int32_t last, const u_int32_t clkdata[])
{
	volatile u_int32_t *clkcfgs = PMGR_FIRST_CLK_CFG;
	u_int32_t val, reg;

	reg = first;
	while (reg <= last) {
		val = clkdata[reg];
		clkcfgs[reg] = val;
        wait_for_pending_clk_cfg_change(reg);
		reg++;
	}
}

static void clocks_quiesce_internal(void)
{
	u_int32_t cnt;
	u_int64_t devices[2];

	// Disable voltage changes
	rPMGR_VOLMAN_CTL = PMGR_VOLMAN_DISABLE_VOL_CHANGE;

	// Critical: AIC, DEBUG, GPIO, UPERF, CDMA, CDIO,
	//           MCU, L2, L2_BIU, CPU0, SCC
	devices[0] = 0x00000002C0000136ULL;
	devices[1] = 0x000000000000C800ULL;

	// Turn on critical device clocks
	clocks_set_gates(devices, true);

	// Turn off non-critical device clocks
	clocks_set_gates(devices, false);

	// Simplified from PMGR Spec 0.045 Section 2.14.4.1

	// Reset top-level dividers to bypass
	clock_update_range(PMGR_CLK_NUM(PCLK1), PMGR_CLK_NUM(PREDIV5), clk_divs_bypass);
	clock_update_range(PMGR_CLK_NUM(VID1), PMGR_CLK_NUM(VID1), clk_divs_bypass);

#if APPLICATION_IBOOT
	miu_bypass_prep(0);
	
	// Prepare to move memory to bypass clock (ensure not high frequency, enable DLL force mode)
	rPMGR_APSC_STA_CTL = PMGR_APSC_MANUAL_CHANGE(perf_levels[kPerformanceMedium].dvfm_state, perf_levels[kPerformanceMedium].perf_state);
	wait_for_pending_apsc_change();

	miu_bypass_prep(1);
#endif

	// Reset scc and soc managed clocks
	rPMGR_APSC_STA_CTL = PMGR_APSC_MANUAL_CHANGE(kDVFM_STATE_BYPASS, kPERF_STATE_BYPASS);
	wait_for_pending_apsc_change();
    
	// Reset PLLs
	for (cnt = 0; cnt < PMGR_PLL_COUNT; cnt++) {
		rPMGR_PLL_CTL0(cnt) = PMGR_PLL_EXT_BYPASS;

		// Wait for Bypass Enabled for PLL2 or higher
		if (cnt >=2) {
			wait_for_pll_bypass_enabled(cnt);
		}
	}

	// Reset Doubler
	rPMGR_DOUBLER_CTL = PMGR_DOUBLER_EXT_BYPASS;
	wait_for_doubler_bypass_enabled();

	// Reset the lower-level clocks
	clock_update_range(PMGR_CLK_NUM(VENC_MTX), PMGR_CLK_NUM(NCO_REF1), clk_divs_bypass);

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
			return clks[PMGR_CLK_CDIO];
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
			return clks[PMGR_CLK_USB_PHY0]; /* The reference is special on FPGA */
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
		case CLK_MCLK:
		default:
			return 0;
	}
}

void clock_set_frequency(int clock, u_int32_t divider, u_int32_t pll_p, u_int32_t pll_m, u_int32_t pll_s, u_int32_t pll_t)
{
	u_int32_t total_div, prediv5_div, vid0_div;

	switch (clock) {
		case CLK_VCLK0:
			// Calculate the total divider required
			total_div = clks[PMGR_CLK_PLL4] / pll_t;

			// Find the largest prediv5_div that will
			// produce the correct total_div
			for (prediv5_div = 31; prediv5_div > 1; prediv5_div--) {
				if ((total_div % prediv5_div) == 0) break;
			}

			// Calculate vid0_div based on the part of
			// total_div not in prediv5_div
			vid0_div = total_div / prediv5_div;

			// Set the clock dividers to their new values
			rPMGR_PREDIV5_CLK_CFG = (rPMGR_PREDIV5_CLK_CFG & ~0x1f) | prediv5_div;
			rPMGR_VID0_CLK_CFG = (rPMGR_VID0_CLK_CFG & ~0x1f) | vid0_div;

			// Update the list of frequencies
			clks[PMGR_CLK_PREDIV5] = clks[PMGR_CLK_PLL4] / prediv5_div;
			clks[PMGR_CLK_VID0] = clks[PMGR_CLK_PREDIV5] / vid0_div;
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
	wait_for_dev_ps_change(device);
}

static void clocks_set_gates(u_int64_t *devices, bool enable)
{
	u_int32_t dev, index;
	volatile u_int32_t *devpss = PMGR_FIRST_PS;
	u_int64_t mask = 1, devmask = 0;

	for (dev = 0, index = -1; dev < PMGR_PS_COUNT; dev++, mask <<= 1) {
		if ((dev % 64) == 0) {
			devmask = devices[++index];
			if (enable == false)
				devmask ^= -1ULL;
			mask = 1;
		}
		// Skip SCC
		if (dev < PMGR_PS_NUM(MCU)) continue;
		if ((devmask & mask) != 0) {
			if (enable) devpss[dev] |= 0xF;
			else devpss[dev] &= ~0xF;

			// Wait for the PS and ACTUAL_PS fields to be equal
			wait_for_dev_ps_change(dev);
		}
	}
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

static void clocks_get_frequencies(void)
{
#if SUPPORT_FPGA
	u_int32_t cnt;
	u_int32_t freq = OSC_FREQ;

	for (cnt = 0; cnt < PMGR_CLK_COUNT; cnt++)
		clks[cnt] = freq;

	// From 'https://soc.apple.com/bali/public/fpga/release/a0/int3c_54/int3c_54-1.4/readme.html':
	//
	// Clock settings:
	// ---------------
	// MAIN_CLK: 7 MHz (need change UART Baud-rate: mem -memap 0 0x32500028 0xc000e)
	//  CPU_CLK: 3.5 MHz
	//  MCU_CLK: 4.5 MHz
	//  CPM_CLK: 64 MHz
	clks[PMGR_CLK_CPU]		=  3500000; // CPU
	clks[PMGR_CLK_MCU]		=  4500000; // DDR
	clks[PMGR_CLK_MCU_FIXED]	=  7000000; // NCLK (MAIN)

	// XXX Why doesn't the USB clock correspond to the 30MHz
	// claimed by fpga team?  I'm not touching it because it
	// appears to work as-is; however, I'd like to understand why
	// this setting is (and has been, looking at H5P FPGA release
	// notes) different from their claims.
	clks[PMGR_CLK_USB_PHY0]		= 12000000; // USB

	for (cnt = 0; cnt < (kDVFM_STATE_IBOOT + 1); cnt++) {
		cpu_clks[cnt] = clks[PMGR_CLK_CPU];
	}
	// keep compiler happy
	cnt = (u_int32_t)clk_parents;

#else
	volatile u_int32_t *reg;
	u_int32_t cnt, val, p, m, s, source, divider, parent_idx;
	u_int32_t divider_type, divider_offset;
	u_int64_t freq;
	u_int8_t  *managed = (u_int8_t *)perf_state_active;

	for (cnt = 0; cnt < kDVFM_STATE_COUNT; cnt++) {
		// ignore DVFM states with no voltage
		if ((rPMGR_DVFM_CFG1(cnt) & 0xFF) == 0) continue;
		
		// decode the PLL settings
		val = rPMGR_DVFM_CFG0(cnt);
		p = 1 + ((val >>  9) & 0x1FF);
		m = 1 + ((val >> 18) & 0x1FF);
		s = 1 + ((val >>  0) & 0x1FF);
		
		// calculate the frequency
		freq = OSC_FREQ;
		freq *= m;
		freq /= p * s;
		
		// save the frequency
		cpu_clks[cnt] = freq;
	}

	for (cnt = 0; cnt < PMGR_CLK_COUNT; cnt++) {
		reg = clk_parents[cnt].divider_reg;
		if (reg == 0) continue;
		val = *reg;

		// Check if the clock in enabled
		if ((val & PMGR_CLK_CFG_ENABLE) == 0) continue;

		divider_type = clk_parents[cnt].divider_type;
		divider_offset = clk_parents[cnt].divider_offset;

		source = 0;
		divider = 1;

		switch (divider_type) {
			case 1 : // Normal with divider
				divider = (val >> 0) & PMGR_CLK_CFG_DIV_MASK;
				// fall through

			case 0 : // Normal without divider
				source = (val >> 28) & 3;
				break;

			case 3 : // Managed with divider
				divider = (managed[divider_offset] >> 0) & PMGR_CLK_CFG_DIV_MASK;
				// fall through

			case 2 : // Managed without divider
				source = (managed[divider_offset] >> 5) & 3;
				break;
		}

		parent_idx = clk_parents[cnt].parents[source];
		freq = clks[parent_idx];

		if ((cnt == PMGR_CLK_DOUBLER) && ((val & PMGR_DOUBLER_EXT_BYPASS) != 0))
			freq *= 2;

		freq /= divider;

		clks[cnt] = freq;
	}
#endif
}

static u_int32_t get_pll(int pll)
{
	u_int32_t pll_ctl0, pll_ctl1;
	u_int64_t freq = 0;

	if (pll >= PMGR_PLL_COUNT) return 0;

	pll_ctl0 = rPMGR_PLL_CTL0(pll);
	pll_ctl1 = rPMGR_PLL_CTL1(pll);

	if ((pll_ctl0 & PMGR_PLL_ENABLE) == 0) return 0;

	if ((pll_ctl0 & (PMGR_PLL_EXT_BYPASS | PMGR_PLL_BYPASS))) {
		freq = OSC_FREQ;
	} else {
		freq = OSC_FREQ;
		freq *= 1 + ((pll_ctl1 >> PMGR_PLL_M_SHIFT) & PMGR_PLL_MASK);	// *M
		freq /= 1 + ((pll_ctl1 >> PMGR_PLL_P_SHIFT) & PMGR_PLL_MASK);	// /P
		freq /= 1 + ((pll_ctl1 >> PMGR_PLL_S_SHIFT) & PMGR_PLL_MASK);	// /S
	}

	return freq;
}

static void set_pll(int pll, u_int32_t p, u_int32_t m, u_int32_t s, u_int32_t vco)
{
	if (pll >= PMGR_PLL_COUNT) return;

	// Set the P, M & S values
	rPMGR_PLL_CTL1(pll) = PMGR_PLL_P(p) | PMGR_PLL_M(m) | PMGR_PLL_S(s);

	// Enable the PLL and request it load the configuration
	rPMGR_PLL_CTL0(pll) = PMGR_PLL_ENABLE | PMGR_PLL_LOAD;

	// Wait for the PLL to lock
	wait_for_pll_lock(pll);
}

#endif

void clock_reset_device(int device)
{
	volatile u_int32_t *reg = PMGR_FIRST_PS + device;

	switch (device) {
		case CLK_CPU1 :
		case CLK_FMI0 :
		case CLK_FMI1 :
		case CLK_IOP :
		case CLK_MCU :
		case CLK_SDIO :
			*reg |= PMGR_PS_RESET;
			spin(1);
			*reg &= ~PMGR_PS_RESET;
			break;

		default :
			break;
	}
}

#if WITH_DEVICETREE

void pmgr_update_device_tree(DTNode *pmgr_node)
{
	u_int32_t	cnt, count, propSize, perf_state_config;
	u_int64_t	freq, period_ns;
	u_int32_t	cpu_volt[kDVFM_STATE_COUNT];
	char		*propName;
	void		*propData;

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
	
	// Fill in the voltage-states1 property
	propName = "voltage-states1";
	if (FindProperty(pmgr_node, &propName, &propData, &propSize)) {
		count = propSize / sizeof(u_int32_t);
		platform_get_cpu_voltages(kDVFM_STATE_COUNT, cpu_volt);
		if (count > (kDVFM_STATE_COUNT - kDVFM_STATE_IBOOT)) count = kDVFM_STATE_COUNT - kDVFM_STATE_IBOOT;
		for (cnt = 0; cnt < count; cnt++) {
			freq = cpu_clks[kDVFM_STATE_IBOOT + cnt];
			if (freq == 0) break;
			
			// Calculate the period is ns as a 16.16 fixed point number
			period_ns = 1000000000ULL << 16;
			period_ns /= freq;
			
			// Save the period and voltage
			((u_int32_t *)propData)[2*cnt] = period_ns;
			((u_int32_t *)propData)[2*cnt+1] = cpu_volt[dvfm_state_active[cnt].cpu_vi];
		}
	}
}

#endif

void init_thermal_sensors(void)
{
	// Grab the 4 bit fuse revision 
	u_int32_t fuseRevision = chipid_get_fuse_revision(); 
	
	// Bits [1  -  0] PWRDN Mode and enable.
	// Bits [3  -  2] Reserved.
	// Bits [   4   ] Stat mode for Avg Max on.
	// Bits [18 -  5] TADC_CFG Tmpsadc configuration bits.
	// Bits [31 - 19] Misc interrupt/Alarm stick bits not used.
	rPMGR_THERMAL0_CTL0 = 0x000184F0;
	rPMGR_THERMAL1_CTL0 = 0x000184F0;

	// Bits [15 -  0] PWRDN_START 20us wait
	// Bits [31 - 16] PWRDN_GAP Gap between two readings.
	rPMGR_THERMAL0_CTL1 = 0x005B01E4;
	rPMGR_THERMAL1_CTL1 = 0x005B01E4;

	// Bits [7  -  0] Conv_Cycle, cycles to wait before data is valid.
	// Bits [15 -  8] Enable_Cycle, cycles to wait before adc_en can be deasserted.
	// Bits [23 - 16] Finish_gap, Cycles to wait before adc_en can be reasserted.
	// Bits [31 - 24] Reserved.
	rPMGR_THERMAL0_CTL2 = 0x00100848;
	rPMGR_THERMAL1_CTL2 = 0x00100848;
    
	for (u_int32_t sensorID = 0; sensorID < 2; sensorID++) {
        
		u_int32_t fusedTempValueAt70 = chipid_get_fused_thermal_sensor_70C(sensorID);
		u_int32_t fusedTempValueAt25 = chipid_get_fused_thermal_sensor_25C(sensorID);
		u_int32_t tempSlope = 0x100;

		// If we read nothing from the fuse, then we need to supply some default for
		// 1 pt calibration, see: <rdar://problem/10688137> 
		if ( (fusedTempValueAt25 == 0) && (fusedTempValueAt70 == 0) ) {

			if (sensorID) {
				fusedTempValueAt25 = 29;
			} else {
				fusedTempValueAt25 = 20;
			}

			// Forces us to generate a slope of 1.0
			fusedTempValueAt70 = fusedTempValueAt25 + 45;
		}

		// Should probably make sure we don't divide by zero.
		if (fusedTempValueAt25 == fusedTempValueAt70) {
			fusedTempValueAt70 = 70;
			fusedTempValueAt25 = 25;
			dprintf(DEBUG_INFO, "Invalid soc thermal fuse values\n");
		}
        
        	if (fuseRevision > 0) {
			// 45 = 70 - 25
			tempSlope = (45*256) / (fusedTempValueAt70 - fusedTempValueAt25);
        	} else {
			dprintf(DEBUG_INFO, "Invalid soc thermal sensor fuse revision number.\n");
		}

		// Calculate the real offset.
		u_int32_t realOffset = 25 - ((tempSlope * fusedTempValueAt25) / (tempSlope==0x100 ? 1:256));

		// Bits 	  23 		      16
		// Temp_OFFSET => [8 bit signed integer] 
		// Bits          9                            0
		// TEMP_SLOPE => [2 bit integer| 8 bit decimal]
		switch (sensorID) {
			case 0:
				rPMGR_THERMAL0_CTL3 = (realOffset & 0xFF) << 16;
				rPMGR_THERMAL0_CTL3 |= tempSlope  & 0x3FF;
				break;
			case 1:
				rPMGR_THERMAL1_CTL3 = (realOffset & 0xFF) << 16;
				rPMGR_THERMAL1_CTL3 |= tempSlope  & 0x3FF;
				break;
		}
	}
}

static void wait_for_dev_ps_change(u_int32_t dev)
{
//#if ! SUPPORT_FPGA
	// Wait for the PS and ACTUAL_PS fields to be equal
	volatile u_int32_t *devpss = PMGR_FIRST_PS;
	while ((devpss[dev] & 0xF) != ((devpss[dev] >> 4) & 0xF));
//#endif
}

static void wait_for_pending_apsc_change(void)
{
#if ! SUPPORT_FPGA
	// Spin while the pending bit is asserted
	while ((rPMGR_APSC_STA_CTL & PMGR_APSC_PENDING) != 0);
#endif
}

static void wait_for_pending_clk_cfg_change(u_int32_t clk)
{
#if ! SUPPORT_FPGA
	// Spin while the pending bit is asserted
	volatile u_int32_t *clkcfgs = PMGR_FIRST_CLK_CFG;
	while (clkcfgs[clk] & PMGR_CLK_CFG_PENDING);
#endif
}

static void wait_for_pll_bypass_enabled(u_int32_t pll)
{
#if ! SUPPORT_FPGA
	while (!(rPMGR_PLL_DEBUG(pll) & PMGR_PLL_DEBUG_BYP_ENABLED));
#endif
}

static void wait_for_pll_lock(u_int32_t pll)
{
#if ! SUPPORT_FPGA
	while ((rPMGR_PLL_CTL0(pll) & PMGR_PLL_REAL_LOCK) == 0);
#endif
}

static void wait_for_doubler_bypass_enabled(void)
{
#if ! SUPPORT_FPGA
	while (!(rPMGR_DOUBLER_DEBUG & PMGR_DOUBLER_DEBUG_BYP_ENABLED)) ;
#endif
}
