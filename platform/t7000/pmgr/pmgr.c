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

#if SUB_TARGET_J81 || SUB_TARGET_J82 || SUB_TARGET_J96 || SUB_TARGET_J97 || SUB_TARGET_J98 || SUB_TARGET_J99
#include <target/boardid.h>
#endif

extern void arm_no_wfe_spin(uint32_t usecs);

struct clock_source {
	uint32_t	src_clk;
	uint32_t	factor;
};

#define CLOCK_SOURCES_MAX 12

struct clock_config {
	volatile uint32_t	*clock_reg; // CLK_CFG Register
	struct clock_source	sources[CLOCK_SOURCES_MAX];    // List of sources
};

#define PLL_VCO_TARGET(pllx)	((2ULL * (pllx##_O) * (pllx##_M)) / (pllx##_P))
#define PLL_FREQ_TARGET(pllx)	(((pllx##_O) * (pllx##_M)) / (pllx##_P) / ((pllx##_S) + 1))

#if APPLICATION_SECUREROM
static uint32_t active_state = kDVFM_STATE_SECUREROM;
#endif

#if APPLICATION_IBOOT
static uint32_t active_state = kDVFM_STATE_IBOOT;
#endif

static uint64_t ccc_dvfm_states[] = {

#if APPLICATION_SECUREROM
	// BIU div = 1, disable voltage change sequence, Clk src = Ref clk.
	[kDVFM_STATE_BYPASS] = (2ULL << 18)|(1ULL << 22),

	// CCC @ 300 MHz, disable voltage change sequence, Clk src = PLL, BIU div = 1.
	[kDVFM_STATE_SECUREROM] = CCC_DVFM_ST_PLL_P(1) | CCC_DVFM_ST_PLL_M(50) |
				  CCC_DVFM_ST_PLL_S(3) |(1ULL << 22)|
				  (1ULL << 21)|(2ULL << 18),
#else
	// BIU div = 1, Clk src = Ref clk.
	[kDVFM_STATE_BYPASS] = (2ULL << 18),
#endif

#if APPLICATION_IBOOT
	// CCC V0
	// CCC @ 396 MHz, enable voltage change sequence, Clk src = PLL, BIU div = 1.
	[kDVFM_STATE_IBOOT] = CCC_DVFM_ST_PLL_P(1) | CCC_DVFM_ST_PLL_M(33) | 
				CCC_DVFM_ST_PLL_S(1) | (1ULL << 21) | (2ULL << 18),

	// CCC V1
	// CCC @ 600 MHz, enable voltage change sequence, Clk src = PLL, BIU div = 1.
	[kDVFM_STATE_V1] = CCC_DVFM_ST_PLL_P(1) | CCC_DVFM_ST_PLL_M(25) | 
				CCC_DVFM_ST_PLL_S(0) | (1ULL << 21) | (2ULL << 18),

	// CCC V2
	// CCC @ 840 MHz, enable voltage change sequence, Clk src = PLL, BIU div = 2.
	[kDVFM_STATE_V2] = CCC_DVFM_ST_PLL_P(1) | CCC_DVFM_ST_PLL_M(35) | 
				CCC_DVFM_ST_PLL_S(0) | (1ULL << 21) | (4ULL << 18),

	// CCC V3
	// CCC @ 1128 MHz, enable voltage change sequence, Clk src = PLL, BIU div = 2
	[kDVFM_STATE_V3] = CCC_DVFM_ST_PLL_P(1) | CCC_DVFM_ST_PLL_M(47) |
				CCC_DVFM_ST_PLL_S(0) | (1ULL << 21) | (4ULL << 18),

#if !SUB_TARGET_N102
	// CCC V4
	// CCC @ 1392 MHz, enable voltage change sequence, Clk src = PLL, BIU div = 3.0
	[kDVFM_STATE_V4] = CCC_DVFM_ST_PLL_P(1) | CCC_DVFM_ST_PLL_M(58) |
				CCC_DVFM_ST_PLL_S(0) | (1ULL << 21) | (6ULL << 18)
#endif
#if SUB_PLATFORM_T7001 || SUB_TARGET_J96 || SUB_TARGET_J97
	// CCC V5
	// CCC @ 1512 MHz, enable voltage change sequence, Clk src = PLL, BIU div = 3.0
	,[kDVFM_STATE_V5] = CCC_DVFM_ST_PLL_P(1) | CCC_DVFM_ST_PLL_M(63) |
				CCC_DVFM_ST_PLL_S(0) | (1ULL << 21) | (6ULL << 18)

	// CCC V6 (pseudo-state)
	// CCC @ 1608 MHz, enable voltage change sequence, Clk src = PLL, BIU div = 3.0
	,[kDVFM_STATE_V6] = CCC_DVFM_ST_PLL_P(1) | CCC_DVFM_ST_PLL_M(67) |
				CCC_DVFM_ST_PLL_S(0) | (1ULL << 21) | (6ULL << 18)

	// CCC V6 Unbinned (pseudo-state)
	// CCC @ 1608 MHz, enable voltage change sequence, Clk src = PLL, BIU div = 3.0
	,[kDVFM_STATE_V6_UNBINNED] = CCC_DVFM_ST_PLL_P(1) | CCC_DVFM_ST_PLL_M(67) |
				CCC_DVFM_ST_PLL_S(0) | (1ULL << 21) | (6ULL << 18)
#endif
#endif
};

#if APPLICATION_SECUREROM
#define SOC_PERF_STATE_ACTIVE kSOC_PERF_STATE_SECUREROM
#endif

#if APPLICATION_IBOOT
#define SOC_PERF_STATE_ACTIVE kSOC_PERF_STATE_IBOOT_MEM_FULL_PERF
#endif

struct pmgr_soc_perf_state {
	uint32_t	entry[3];
};

static struct pmgr_soc_perf_state pmgr_soc_perf_states[] = {
	[kSOC_PERF_STATE_BYPASS] = {
		{0x00000000,
		 0x00000000,
#if SUB_PLATFORM_T7000
		 0x00000070}},
#elif SUB_PLATFORM_T7001
		 0x00000000}},
#endif
	
#if APPLICATION_SECUREROM
	[kSOC_PERF_STATE_SECUREROM] = {
		// af = pll4 div 5
		// lio = pll4 div 10
		{0x00055000,
		 0x00000000,
#if SUB_PLATFORM_T7000
		 0x00000070}},
#elif SUB_PLATFORM_T7001
		 0x00000000}},
#endif
#endif

#if APPLICATION_IBOOT
	// Memory Low Performance:
	[kSOC_PERF_STATE_IBOOT_MEM_LOW_PERF] = {
		// mcu_fixed = pll1
		// mcu = pll1 div 16
		// mcu_cfg = 0x3
		// af = pll4 div 5
		// lio = pll4 div 10
		// media_af = pll4 div 6
		// isp_af = pll4 div 6
		{0x06655361,
		// isp_c = pll4 div 6
		// isp = pll4 div 8
		// vdec = pll4 div 10
		// venc = pll4 div 7
		// ajpeg_ip = pll4 div 14
		// ajpeg_wrap = pll4 div 11
		// msr = pll4 div 8
		 0x07887876,
		 0x00000000}},
	
	// Memory Full Performance / VMIN:
	[kSOC_PERF_STATE_IBOOT_MEM_FULL_PERF] = {
#if !SUPPORT_FPGA
		// mcu_fixed = pll1
		// mcu = pll1
		// mcu_cfg = 0x0
		// af = pll4 div 5
		// lio = pll4 div 10
		// media_af = pll4 div 6
		// isp_af = pll4 div 6
		{0x06655081,
#else
		// mcu_fixed = pll1
		// mcu = pll1 div 16
		// mcu_cfg = 0x3
		// af = pll4 div 5
		// lio = pll4 div 10
		// media_af = pll4 div 6
		// isp_af = pll4 div 6
		{0x06655361,
#endif
		// isp_c = pll4 div 6
		// isp = pll4 div 8
		// vdec = pll4 div 10
		// venc = pll4 div 7
		// ajpeg_ip = pll4 div 14
		// ajpeg_wrap = pll4 div 11
		// msr = pll4 div 8
		 0x07887876,
		 0x00000000}},

	// VNOM:
	[kSOC_PERF_STATE_VNOM] = {
#if !SUPPORT_FPGA
		// mcu_fixed = pll1
		// mcu = pll1
		// mcu_cfg = 0x0
		// af = pll4 div 5
		// lio = pll4 div 10
		// media_af = pll4 div 5
		// isp_af = pll4 div 5
		{0x05555081,
#else
		// mcu_fixed = pll1
		// mcu = pll1 div 16
		// mcu_cfg = 0x3
		// af = pll4 div 5
		// lio = pll4 div 10
		// media_af = pll4 div 5
		// isp_af = pll4 div 5
		{0x05555361,
#endif
		// isp_c = pll4 div 5
		// isp = pll4 div 7
		// vdec = pll4 div 7
		// venc = pll4 div 5
		// ajpeg_ip = pll4 div 10
		// ajpeg_wrap = pll4 div 8
		// msr = pll4 div 6
		 0x05555565,
		 0x00000000}},
#endif
};

#if APPLICATION_SECUREROM
static uint32_t perf_level = kPerformanceHigh;
#endif

#if APPLICATION_IBOOT
static uint32_t perf_level = kPerformanceMemoryFull;
#endif


#if APPLICATION_IBOOT

#if SUB_PLATFORM_T7000 && WITH_HW_MIPI
/* MIPI: PLL0 @900MHz */
#define PLL0	    0
#define PLL0_O	    OSC_FREQ
#define PLL0_P	    2
#define PLL0_M	    150
#define PLL0_S	    1
#define PLL0_V	    PLL_VCO_TARGET(PLL0)
#define PLL0_T	    PLL_FREQ_TARGET(PLL0)
#endif

#if TARGET_DDR_740M

/* PLL1 @740MHz */
#define PLL1	    1
#define PLL1_O	    OSC_FREQ
#define PLL1_P	    6
#define PLL1_M	    185
#define PLL1_S	    0
#define PLL1_V	    PLL_VCO_TARGET(PLL1)
#define PLL1_T	    PLL_FREQ_TARGET(PLL1)

#elif TARGET_DDR_798M

/* PLL1 @798MHz */
#define PLL1	    1
#define PLL1_O	    OSC_FREQ
#define PLL1_P	    4
#define PLL1_M	    133
#define PLL1_S	    0
#define PLL1_V	    PLL_VCO_TARGET(PLL1)
#define PLL1_T	    PLL_FREQ_TARGET(PLL1)

#elif TARGET_DDR_792M

/* PLL1 @792MHz */
#define PLL1	   1
#define PLL1_O      OSC_FREQ
#define PLL1_P      1
#define PLL1_M      66
#define PLL1_S      1
#define PLL1_V      PLL_VCO_TARGET(PLL1)
#define PLL1_T      PLL_FREQ_TARGET(PLL1)

#else

/* PLL1 @800MHz */
#define PLL1	    1
#define PLL1_O	    OSC_FREQ
#define PLL1_P	    3
#define PLL1_M	    100
#define PLL1_S	    0
#define PLL1_V	    PLL_VCO_TARGET(PLL1)
#define PLL1_T	    PLL_FREQ_TARGET(PLL1)

#endif

// Support 100 MHz PLL3 on Capri for PCIe
#if SUB_PLATFORM_T7001
/* PLL3 @100MHz */
#define PLL3	    3
#define PLL3_O	    OSC_FREQ
#define PLL3_P	    1
#define PLL3_M	    50
#define PLL3_S	    11
#define PLL3_V	    PLL_VCO_TARGET(PLL3)
#define PLL3_T	    PLL_FREQ_TARGET(PLL3)
#endif

/* PLL4 @2400MHz (VCO output) */
#define PLL4	    4
#define PLL4_O	    OSC_FREQ
#define PLL4_P	    1
#define PLL4_M	    50
#define PLL4_S	    0
#define PLL4_V	    PLL_VCO_TARGET(PLL4)
#define PLL4_T	    PLL_FREQ_TARGET(PLL4)
#define	PLL4_VCO_ENABLED	1

#ifndef TARGET_SPARE0_CLK_CFG
#define TARGET_SPARE0_CLK_CFG 0x00000000
#endif

#define VID0_CLKD	TARGET_VID0_CLK_CFG

#if SUB_PLATFORM_T7000
#if WITH_HW_MIPI
#define MIPI_CLKD 0x88100000
#else
#define MIPI_CLKD 0x80100000
#endif

static const uint32_t clk_divs_active[PMGR_CLK_CFG_COUNT] = {
	0x81100000, 0x88100000, MIPI_CLKD,  0x86100000, // 0x10000: mcu_fixed, mcu, mipi_dsi, nco_ref0,
	0x85100000, 0x80100000, 0x88100000, 0x85100000, // 0x10010: nco_ref1, nco_alg0, nco_alg1, hsciphy_ref_12m,
	0x85100000, 0x85100000, 0x85100000, 0x85100000, // 0x10020: usb480_0, usb480_1, usb_ohci_48m, usb,
	0x85100000, 0x85100000, 0x80100000, 0x85100000, // 0x10030: usb_free_60m, sio_c, sio_p, isp_c,
	0x86100000, 0x81100000, 0x82100000, 0x85100000, // 0x10040: isp, isp_sensor0_ref, isp_sensor1_ref, vdec,
	0x85100000, VID0_CLKD,  0x86100000, 0x86100000, // 0x10050: venc, vid0, disp0, disp1,
	0x85100000, 0x85100000, 0x85100000, 0x85100000, // 0x10060: ajpeg_ip, ajpeg_wrap, msr, af,
	0x85100000, 0x85100000, 0x86100000, 0x87100000, // 0x10070: lio, mca0_m, mca1_m, mca2_m
	0x88100000, 0x89100000, 0x85100000, 0x86100000, // 0x10080: mca3_m, mca4_m, sep, gpio,
	0x85100000, 0x85100000, 0x85100000, 0x85100000, // 0x10090: spi0_n, spi1_n, spi2_n, spi3_n
	0x80100000, 0x80100000, 0x85100000, 0x85100000, // 0x100A0: debug, pcie_ref, pcie_app, tmps,
	0x85100000, 0x85100000, 0x86100000, 0x85100000, // 0x100B0: media_af, isp_af, gfx_af, ans_c
	0x85100000                      // 0x100C0: anc_link
};
#elif SUB_PLATFORM_T7001
static const uint32_t clk_divs_active[PMGR_CLK_CFG_COUNT] = {
	0x81100000, 0x88100000, 0x86100000, 0x85100000, // 0x10000: mcu_fixed, mcu, nco_ref0, nco_ref1,
	0x80100000, 0x88100000, 0x85100000, 0x85100000, // 0x10010: nco_alg0, nco_alg1, hsciphy_ref_12m, usb480_0,
	0x85100000, 0x85100000, 0x85100000, 0x85100000, // 0x10020: usb480_1, usb_ohci_48m, usb, usb_free_60m,
	0x85100000, 0x80100000, 0x85100000, 0x86100000, // 0x10030: sio_c, sio_p, isp_c, isp,
	0x81100000, 0x82100000, 0x85100000, 0x85100000, // 0x10040: isp_sensor0_ref, isp_sensor1_ref, vdec, venc,
	VID0_CLKD,  0x85100000, 0x86100000, 0x85100000, // 0x10050: vid0, disp0, disp1, ajpeg_ip,
	0x85100000, 0x85100000, 0x85100000, 0x85100000, // 0x10060: ajpeg_wrap, msr, af, lio,
	0x85100000, 0x86100000, 0x87100000, 0x88100000, // 0x10070: mca0_m, mca1_m, mca2_m, mca3_m,
	0x89100000, 0x85100000, 0x86100000, 0x85100000, // 0x10080: mca4_m, sep, gpio, spi0_n,
	0x85100000, 0x85100000, 0x85100000, 0x80100000, // 0x10090: spi1_n, spi2_n, spi3_n, debug,
	0x85100000, 0x85100000, 0x85100000, 0x85100000, // 0x100A0: pcie_app, tmps, media_af, isp_af,
	0x86100000, 0x85100000, 0x85100000 // 0x100B0:  gfx_af, ans_c, anc_link
};
#endif

static const uint32_t spare_divs_active[PMGR_SPARE_CLK_CFG_COUNT] = {
	TARGET_SPARE0_CLK_CFG,	// 0x10208: spare0
	0x00000000,		// 0x1020C: spare1
	0x00000000,		// 0x10210: spare2
	0x00000000,		// 0x10214: spare3
	0x80100028,		// 0x10218: isp_ref0
	0x80100028 		// 0x1021C: isp_ref1
};

/* GFX table. The voltage information is maintained in chipid.c */
struct gfx_state_info {
	uint32_t dwi_val; /* will be populated by determining the binning info */
	uint32_t fb_div;
	uint32_t pre_div;
	uint32_t op_div;
	uint32_t dwi_sram_val; /* Only meaningfull for SUB_PLATFORM_T7001 */
	
};

static struct gfx_state_info gfx_states[] = {
	[CHIPID_GPU_VOLTAGE_OFF] = { 0, 0, 0, 0, 0 },		/* Power off Templar4 state.*/
	[CHIPID_GPU_VOLTAGE_V0] = { 0, 365, 6, 3, 0 },		/* 365 MHz */
	[CHIPID_GPU_VOLTAGE_V1] = { 0, 75, 2, 1, 0 },		/* 450 MHz */
	[CHIPID_GPU_VOLTAGE_V2] = { 0, 125, 3, 1, 0 },		/* 500 MHz */
#if SUB_TARGET_N56 || SUB_TARGET_J42 || SUB_TARGET_J42D ||  SUB_PLATFORM_T7001 || SUB_TARGET_J96 || SUB_TARGET_J97
	[CHIPID_GPU_VOLTAGE_V3] = { 0, 275, 6, 1, 0 },		/* 550 MHz */
#endif
#if SUB_PLATFORM_T7001 || SUB_TARGET_J96 || SUB_TARGET_J97
	[CHIPID_GPU_VOLTAGE_V4] = { 0,  50, 1, 1, 0 },		/* 600 MHz */
	[CHIPID_GPU_VOLTAGE_V5] = { 0, 325, 6, 1, 0 },		/* 650 MHz */
#endif
	[CHIPID_GPU_VOLTAGE_V0_DIDT] = { 0, 365, 6, 7, 0 },	/* 182.5 MHz */
	[CHIPID_GPU_VOLTAGE_V1_DIDT] = { 0, 75, 2, 3, 0 },	/* 225 MHz */
	[CHIPID_GPU_VOLTAGE_V2_DIDT] = { 0, 125, 3, 3, 0 },	/* 250 MHz */
#if SUB_TARGET_N56 || SUB_TARGET_J42 || SUB_TARGET_J42D ||  SUB_PLATFORM_T7001 || SUB_TARGET_J96 || SUB_TARGET_J97
	[CHIPID_GPU_VOLTAGE_V3_DIDT] = { 0, 275, 6, 3, 0 },	/* 275 MHz */
#endif
#if SUB_PLATFORM_T7001 || SUB_TARGET_J96 || SUB_TARGET_J97
	[CHIPID_GPU_VOLTAGE_V4_DIDT] = { 0,  50, 1, 3, 0 },     /* 300 MHz */
	[CHIPID_GPU_VOLTAGE_V5_DIDT] = { 0, 325, 6, 3, 0 },	/* 325 MHz */
#endif
#if 0
	[5] = {0, 0, 0, 0},     /* Debug state. chipid.c has a valid voltage for this state. */
#endif
};

static void set_gfx_perf_state(uint32_t state_num, struct gfx_state_info *gfx_state);
#endif /* APPLICATION_IBOOT */

#if APPLICATION_SECUREROM

#if SUB_PLATFORM_T7001 && !SUPPORT_FPGA
/* PLL3 @100MHz */
#define PLL3	    3
#define PLL3_O	    OSC_FREQ
#define PLL3_P	    1
#define PLL3_M	    50
#define PLL3_S	    11
#define PLL3_V	    PLL_VCO_TARGET(PLL3)
#define PLL3_T	    PLL_FREQ_TARGET(PLL3)
#endif

/* PLL4 @1200MHz */
#define PLL4	    4
#define PLL4_O	    OSC_FREQ
#define PLL4_P	    1
#define PLL4_M	    50
#define PLL4_S	    0
#define PLL4_V	    PLL_VCO_TARGET(PLL4)
#define PLL4_T	    PLL_FREQ_TARGET(PLL4)
#define	PLL4_VCO_ENABLED	0


// We won't touch the clk gen's that aren't necessary during SecureROM.
#if SUB_PLATFORM_T7000
static const uint32_t clk_divs_active[PMGR_CLK_CFG_COUNT] = {
	0x80100000, 0x80100000, 0x80100000, 0x80100000, // 0x10000: mcu_fixed, mcu, mipi_dsi, nco_ref0,
	0x80100000, 0x80100000, 0x80100000, 0x80100000, // 0x10010: nco_ref1, nco_alg0, nco_alg1, hsciphy_ref_12m,
	0x80100000, 0x80100000, 0x80100000, 0x86100000, // 0x10020: usb480_0, usb480_1, usb_ohci_48m, usb,
	0x80100000, 0x85100000, 0x85100000, 0x80100000, // 0x10030: usb_free_60m, sio_c, sio_p, isp_c
	0x80100000, 0x80100000, 0x80100000, 0x80100000, // 0x10040: isp, isp_sensor0_ref, isp_sensor1_ref, vdec
	0x80100000, 0x80100000, 0x80100000, 0x80100000, // 0x10050: venc, vid0, disp0, disp1
	0x80100000, 0x80100000, 0x80100000, 0x85100000, // 0x10060: ajpeg_ip, ajpeg_wrap, msr, af
	0x85100000, 0x80100000, 0x80100000, 0x80100000, // 0x10070: lio, mca0_m, mca1_m, mca2_m
	0x80100000, 0x80100000, 0x85100000, 0x86100000, // 0x10080: mca3_m, mca4_m, sep, gpio
	0x80100000, 0x80100000, 0x80100000, 0x80100000, // 0x10090: spi0_n, spi1_n, spi2_n, spi3_n
	0x80100000, 0x80100000, 0x85100000, 0x80100000, // 0x100a0: debug, pcie_ref, pcie_app, tmps
	0x80100000, 0x80100000, 0x80100000, 0x85100000, // 0x100b0: media, isp_af, gfx, ans_c
	0x85100000                                      // 0x100c0: anc_link
};
#elif SUB_PLATFORM_T7001
static const uint32_t clk_divs_active[PMGR_CLK_CFG_COUNT] = {
	0x80100000, 0x80100000, 0x80100000, 0x80100000, // 0x10000: mcu_fixed, mcu, nco_ref0, nco_ref1,
	0x80100000, 0x80100000, 0x80100000, 0x80100000, // 0x10010: nco_alg0, nco_alg1, hsciphy_ref_12m, usb480_0,
	0x80100000, 0x80100000, 0x86100000, 0x80100000, // 0x10020: usb480_1, usb_ohci_48m, usb, usb_free_60m,
	0x85100000, 0x85100000, 0x80100000, 0x80100000, // 0x10030: sio_c, sio_p, isp_c, isp,
	0x80100000, 0x80100000, 0x80100000, 0x80100000, // 0x10040: isp_sensor0_ref, isp_sensor1_ref, vdec, venc,
	0x80100000, 0x80100000, 0x80100000, 0x80100000, // 0x10050: vid0, disp0, disp1, ajpeg_ip,
	0x80100000, 0x80100000, 0x85100000, 0x85100000, // 0x10060: ajpeg_wrap, msr, af, lio,
	0x80100000, 0x80100000, 0x80100000, 0x80100000, // 0x10070: mca0_m, mca1_m, mca2_m, mca3_m,
	0x80100000, 0x85100000, 0x86100000, 0x80100000, // 0x10080: mca4_m, sep, gpio, spi0_n,
	0x80100000, 0x80100000, 0x80100000, 0x80100000, // 0x10090: spi1_n, spi2_n, spi3_n, debug,
	0x85100000, 0x80100000, 0x80100000, 0x80100000, // 0x100a0: pcie_app, tmps, media, isp_af,
	0x80100000, 0x85100000, 0x85100000 // 0x100b0: gfx, ans_c, anc_link
};
#endif

static const uint32_t spare_divs_active[PMGR_SPARE_CLK_CFG_COUNT] = {
	0x80000001,         // 0x10208: s0
	0x80000001,         // 0x1020C: s1
	0x80000001,         // 0x10210: s2
	0x80000001,         // 0x10214: s3,
	0x80000001,         // 0x10218: isp_ref0
	0x80000001          // 0x1021c: isp_ref1
};

#endif /* APPLICATION_SECUREROM */


static const struct clock_config clk_configs[PMGR_CLK_COUNT] = {
	[PMGR_CLK_MCU_FIXED]    =	{
		&rPMGR_MCU_FIXED_CLK_CFG,
		{
			{ PMGR_CLK_OSC,	1 },
			{ PMGR_CLK_PLL1, 1 }
		}
	},

	[PMGR_CLK_MCU]	    =	{
		&rPMGR_MCU_CLK_CFG,
		{
			{ PMGR_CLK_OSC, 1 },
			{ PMGR_CLK_OSC, 1 },
			{ PMGR_CLK_OSC, 1 },
			{ PMGR_CLK_PLL1, 2 },
			{ PMGR_CLK_PLL1, 4 },
			{ PMGR_CLK_PLL1, 8 },
			{ PMGR_CLK_PLL1, 16 },
			{ PMGR_CLK_PLL4, 48 },
			{ PMGR_CLK_PLL1, 1 }
		}
	},
#if SUB_PLATFORM_T7000
	[PMGR_CLK_MIPI_DSI]	    =	{
		&rPMGR_MIPI_DSI_CLK_CFG,
		{
			{ PMGR_CLK_OSC, 1 },
			{ PMGR_CLK_S0, 1 },
			{ PMGR_CLK_S1, 1 },
			{ PMGR_CLK_S2, 1 },
			{ PMGR_CLK_S3, 1 },
			{ PMGR_CLK_OSC, 1 },
			{ PMGR_CLK_PLL2, 1 },
			{ PMGR_CLK_PLL4, 3 },
			{ PMGR_CLK_PLL0, 1 }
		}
	},
#endif

	[PMGR_CLK_NCO_REF0]	    =	{
		&rPMGR_NCO_REF0_CLK_CFG,
		{
			{ PMGR_CLK_OSC, 1 },
			{ PMGR_CLK_S0, 1 },
			{ PMGR_CLK_S1, 1 },
			{ PMGR_CLK_S2, 1 },
			{ PMGR_CLK_S3, 1 },
			{ PMGR_CLK_PLL4, 2 },
			{ PMGR_CLK_PLL4, 7 }
		}
	},

	[PMGR_CLK_NCO_REF1]	    =	{
		&rPMGR_NCO_REF1_CLK_CFG,
		{
			{ PMGR_CLK_OSC, 1 },
			{ PMGR_CLK_S0, 1 },
			{ PMGR_CLK_S1, 1 },
			{ PMGR_CLK_S2, 1 },
			{ PMGR_CLK_S3, 1 },
			{ PMGR_CLK_PLL4, 2 },
			{ PMGR_CLK_PLL4, 7 }
		}
	},

	[PMGR_CLK_NCO_ALG0]	    =	{
		&rPMGR_NCO_ALG0_CLK_CFG,
		{
			{ PMGR_CLK_OSC, 1 },
			{ PMGR_CLK_S0, 1 },
			{ PMGR_CLK_S1, 1 },
			{ PMGR_CLK_S2, 1 },
			{ PMGR_CLK_S3, 1 },
			{ PMGR_CLK_PLL4, 10 },
			{ PMGR_CLK_PLL4, 12 },
			{ PMGR_CLK_PLL4, 16 },
			{ PMGR_CLK_PLL4, 24 },
			{ PMGR_CLK_PLL4, 40 },
			{ PMGR_CLK_PLL4, 50 }
		}
	},

	[PMGR_CLK_NCO_ALG1]	    =	{
		&rPMGR_NCO_ALG1_CLK_CFG,
		{
			{ PMGR_CLK_OSC, 1 },
			{ PMGR_CLK_S0, 1 },
			{ PMGR_CLK_S1, 1 },
			{ PMGR_CLK_S2, 1 },
			{ PMGR_CLK_S3, 1 },
			{ PMGR_CLK_PLL4, 10 },
			{ PMGR_CLK_PLL4, 12 },
			{ PMGR_CLK_PLL4, 16 },
			{ PMGR_CLK_PLL4, 24 },
			{ PMGR_CLK_PLL4, 40 },
			{ PMGR_CLK_PLL4, 50 }
		}
	},

	[PMGR_CLK_HSICPHY_REF_12M]	=   {
		&rPMGR_HSICPHY_REF_12M_CLK_CFG,
		{
			{ PMGR_CLK_OSC, 1 },
			{ PMGR_CLK_S0, 1 },
			{ PMGR_CLK_S1, 1 },
			{ PMGR_CLK_S2, 1 },
			{ PMGR_CLK_S3, 1 },
			{ PMGR_CLK_OSC, 2 }
		}
	},

	[PMGR_CLK_USB480_0]	    =	{
		&rPMGR_USB480_0_CLK_CFG,
		{
			{ PMGR_CLK_OSC, 1 },
			{ PMGR_CLK_S0, 1 },
			{ PMGR_CLK_S1, 1 },
			{ PMGR_CLK_S2, 1 },
			{ PMGR_CLK_S3, 1 },
			{ PMGR_CLK_PLL4, 5 }
		}
	},

	[PMGR_CLK_USB480_1]	    =	{
		&rPMGR_USB480_1_CLK_CFG,
		{
			{ PMGR_CLK_OSC, 1 },
			{ PMGR_CLK_S0, 1 },
			{ PMGR_CLK_S1, 1 },
			{ PMGR_CLK_S2, 1 },
			{ PMGR_CLK_S3, 1 },
			{ PMGR_CLK_PLL4, 5 }
		}
	},

	[PMGR_CLK_USB_OHCI_48M]	=   {
		&rPMGR_USB_OHCI_48M_CLK_CFG,
		{
			{ PMGR_CLK_OSC, 1 },
			{ PMGR_CLK_S0, 1 },
			{ PMGR_CLK_S1, 1 },
			{ PMGR_CLK_S2, 1 },
			{ PMGR_CLK_S3, 1 },
			{ PMGR_CLK_PLL4, 50 }
		}
	},

	[PMGR_CLK_USB]	    =	{
		&rPMGR_USB_CLK_CFG,
		{
			{ PMGR_CLK_OSC, 1 },
			{ PMGR_CLK_S0, 1 },
			{ PMGR_CLK_S1, 1 },
			{ PMGR_CLK_S2, 1 },
			{ PMGR_CLK_S3, 1 },
			{ PMGR_CLK_PLL4, 20 },
			{ PMGR_CLK_PLL4, 24 },
			{ PMGR_CLK_PLL4, 40 },
			{ PMGR_CLK_PLL4, 50 },
			{ PMGR_CLK_PLL4, 72 },
			{ PMGR_CLK_PLL4, 80 }
		}
	},

	[PMGR_CLK_USB_FREE_60M]	=   {
		&rPMGR_USB_FREE_60M_CLK_CFG,
		{
			{ PMGR_CLK_OSC, 1 },
			{ PMGR_CLK_S0, 1 },
			{ PMGR_CLK_S1, 1 },
			{ PMGR_CLK_S2, 1 },
			{ PMGR_CLK_S3, 1 },
			{ PMGR_CLK_PLL4, 40 }
		}
	},

	[PMGR_CLK_SIO_C]	    =	{
		&rPMGR_SIO_C_CLK_CFG,
		{
			{ PMGR_CLK_OSC, 1 },
			{ PMGR_CLK_S0, 1 },
			{ PMGR_CLK_S1, 1 },
			{ PMGR_CLK_S2, 1 },
			{ PMGR_CLK_S3, 1 },
			{ PMGR_CLK_PLL4, 5 },
			{ PMGR_CLK_PLL4, 6 },
			{ PMGR_CLK_PLL4, 7 },
			{ PMGR_CLK_PLL4, 8 },
			{ PMGR_CLK_PLL4, 9 },
			{ PMGR_CLK_PLL4, 10 },
			{ PMGR_CLK_PLL4, 48 }
		}
	},

	[PMGR_CLK_SIO_P]	    =	{
		&rPMGR_SIO_P_CLK_CFG,
		{
			{ PMGR_CLK_OSC, 1 },
			{ PMGR_CLK_S0, 1 },
			{ PMGR_CLK_S1, 1 },
			{ PMGR_CLK_S2, 1 },
			{ PMGR_CLK_S3, 1 },
			{ PMGR_CLK_PLL4, 20 },
			{ PMGR_CLK_PLL4, 24 },
			{ PMGR_CLK_PLL4, 30 },
			{ PMGR_CLK_PLL4, 40 },
			{ PMGR_CLK_PLL4, 48 },
			{ PMGR_CLK_OSC,	2 },
			{ PMGR_CLK_OSC,	4 }
		}
	},

	[PMGR_CLK_ISP_C]	    =	{
		&rPMGR_ISP_C_CLK_CFG,
		{
			{ PMGR_CLK_OSC, 1 },
			{ PMGR_CLK_S0, 1 },
			{ PMGR_CLK_S1, 1 },
			{ PMGR_CLK_S2, 1 },
			{ PMGR_CLK_S3, 1 },
			{ PMGR_CLK_PLL4, 5 },
			{ PMGR_CLK_PLL4, 6 },
			{ PMGR_CLK_PLL4, 7 },
			{ PMGR_CLK_PLL4, 8 },
			{ PMGR_CLK_PLL4, 9 },
			{ PMGR_CLK_PLL4, 10 },
			{ PMGR_CLK_PLL4, 48 }
		}
	},

	[PMGR_CLK_ISP]	    =	{
		&rPMGR_ISP_CLK_CFG,
		{
			{ PMGR_CLK_OSC, 1 },
			{ PMGR_CLK_S0, 1 },
			{ PMGR_CLK_S1, 1 },
			{ PMGR_CLK_S2, 1 },
			{ PMGR_CLK_S3, 1 },
			{ PMGR_CLK_PLL4, 6 },
			{ PMGR_CLK_PLL4, 7 },
			{ PMGR_CLK_PLL4, 8 },
			{ PMGR_CLK_PLL4, 9 },
			{ PMGR_CLK_PLL4, 10 },
			{ PMGR_CLK_PLL4, 12 }
		}
	},

	[PMGR_CLK_ISP_SENSOR0_REF]	=   {
		&rPMGR_ISP_SENSOR0_REF_CLK_CFG,
		{
			{ PMGR_CLK_OSC, 1 },
			{ PMGR_CLK_ISP_REF0, 1 },
			{ PMGR_CLK_ISP_REF1, 1 }
		}
	},

	[PMGR_CLK_ISP_SENSOR1_REF]	=   {
		&rPMGR_ISP_SENSOR1_REF_CLK_CFG,
		{
			{ PMGR_CLK_OSC, 1 },
			{ PMGR_CLK_ISP_REF0, 1 },
			{ PMGR_CLK_ISP_REF1, 1 }
		}
	},

	[PMGR_CLK_VDEC]	    =	{
		&rPMGR_VDEC_CLK_CFG,
		{
			{ PMGR_CLK_OSC, 1 },
			{ PMGR_CLK_S0, 1 },
			{ PMGR_CLK_S1, 1 },
			{ PMGR_CLK_S2, 1 },
			{ PMGR_CLK_S3, 1 },
			{ PMGR_CLK_PLL4, 7 },
			{ PMGR_CLK_PLL4, 8 },
			{ PMGR_CLK_PLL4, 9 },
			{ PMGR_CLK_PLL4, 10 },
			{ PMGR_CLK_PLL4, 12 },
			{ PMGR_CLK_PLL4, 16 },
			{ PMGR_CLK_PLL4, 24 }
		}
	},

	[PMGR_CLK_VENC]	    =	{
		&rPMGR_VENC_CLK_CFG,
		{
			{ PMGR_CLK_OSC, 1 },
			{ PMGR_CLK_S0, 1 },
			{ PMGR_CLK_S1, 1 },
			{ PMGR_CLK_S2, 1 },
			{ PMGR_CLK_S3, 1 },
			{ PMGR_CLK_PLL4, 5 },
			{ PMGR_CLK_PLL4, 6 },
			{ PMGR_CLK_PLL4, 7 },
			{ PMGR_CLK_PLL4, 8 },
			{ PMGR_CLK_PLL4, 9 },
			{ PMGR_CLK_PLL4, 12 },
			{ PMGR_CLK_PLL4, 24 }
		}
	},

	[PMGR_CLK_VID0]	    =	{
		&rPMGR_VID0_CLK_CFG,
		{
			{ PMGR_CLK_OSC, 1 },
			{ PMGR_CLK_S0, 1 },
			{ PMGR_CLK_S1, 1 },
			{ PMGR_CLK_S2, 1 },
			{ PMGR_CLK_S3, 1 },
			{ PMGR_CLK_PLL4, 11 },
			{ PMGR_CLK_PLL4, 39 }
#if SUB_PLATFORM_T7001
			,{ PMGR_CLK_PLL4, 30 },
			{ PMGR_CLK_PLL4, 12 },
			{ PMGR_CLK_PLL4, 7 }
#endif
		}
	},

#if SUB_PLATFORM_T7000
	[PMGR_CLK_DISP0]	    =	{
		&rPMGR_DISP0_CLK_CFG,
		{
			{ PMGR_CLK_OSC, 1 },
			{ PMGR_CLK_S0, 1 },
			{ PMGR_CLK_S1, 1 },
			{ PMGR_CLK_S2, 1 },
			{ PMGR_CLK_S3, 1 },
			{ PMGR_CLK_PLL4, 6 },
			{ PMGR_CLK_PLL4, 7 },
			{ PMGR_CLK_PLL4, 8 },
			{ PMGR_CLK_PLL4, 9 },
			{ PMGR_CLK_PLL4, 12 },
			{ PMGR_CLK_PLL4, 14 },
			{ PMGR_CLK_PLL4, 24 }
		}
	},

	[PMGR_CLK_DISP1]	    =	{
		&rPMGR_DISP1_CLK_CFG,
		{
			{ PMGR_CLK_OSC, 1 },
			{ PMGR_CLK_S0, 1 },
			{ PMGR_CLK_S1, 1 },
			{ PMGR_CLK_S2, 1 },
			{ PMGR_CLK_S3, 1 },
			{ PMGR_CLK_PLL4, 6 },
			{ PMGR_CLK_PLL4, 7 },
			{ PMGR_CLK_PLL4, 8 },
			{ PMGR_CLK_PLL4, 9 },
			{ PMGR_CLK_PLL4, 12 },
			{ PMGR_CLK_PLL4, 14 },
			{ PMGR_CLK_PLL4, 24 }
		}
	},
#elif SUB_PLATFORM_T7001
	[PMGR_CLK_DISP0]	    =	{
		&rPMGR_DISP0_CLK_CFG,
		{
			{ PMGR_CLK_OSC, 1 },
			{ PMGR_CLK_S0, 1 },
			{ PMGR_CLK_S1, 1 },
			{ PMGR_CLK_S2, 1 },
			{ PMGR_CLK_S3, 1 },
			{ PMGR_CLK_PLL4, 5 },
			{ PMGR_CLK_PLL4, 6 },
			{ PMGR_CLK_PLL4, 7 },
			{ PMGR_CLK_PLL4, 8 },
			{ PMGR_CLK_PLL4, 9 },
			{ PMGR_CLK_PLL4, 12 },
			{ PMGR_CLK_PLL4, 14 }
		}
	},

	[PMGR_CLK_DISP1]	    =	{
		&rPMGR_DISP1_CLK_CFG,
		{
			{ PMGR_CLK_OSC, 1 },
			{ PMGR_CLK_S0, 1 },
			{ PMGR_CLK_S1, 1 },
			{ PMGR_CLK_S2, 1 },
			{ PMGR_CLK_S3, 1 },
			{ PMGR_CLK_PLL4, 5 },
			{ PMGR_CLK_PLL4, 6 },
			{ PMGR_CLK_PLL4, 7 },
			{ PMGR_CLK_PLL4, 8 },
			{ PMGR_CLK_PLL4, 9 },
			{ PMGR_CLK_PLL4, 12 },
			{ PMGR_CLK_PLL4, 14 }
		}
	},
#endif	

	[PMGR_CLK_AJPEG_IP]	    =	{
		&rPMGR_AJPEG_IP_CLK_CFG,
		{
			{ PMGR_CLK_OSC, 1 },
			{ PMGR_CLK_S0, 1 },
			{ PMGR_CLK_S1, 1 },
			{ PMGR_CLK_S2, 1 },
			{ PMGR_CLK_S3, 1 },
			{ PMGR_CLK_PLL4, 10 },
			{ PMGR_CLK_PLL4, 11 },
			{ PMGR_CLK_PLL4, 12 },
			{ PMGR_CLK_PLL4, 14 },
			{ PMGR_CLK_PLL4, 16 },
			{ PMGR_CLK_PLL4, 24 }
		}
	},

	[PMGR_CLK_AJPEG_WRAP]   =	{
		&rPMGR_AJPEG_WRAP_CLK_CFG,
		{
			{ PMGR_CLK_OSC, 1 },
			{ PMGR_CLK_S0, 1 },
			{ PMGR_CLK_S1, 1 },
			{ PMGR_CLK_S2, 1 },
			{ PMGR_CLK_S3, 1 },
			{ PMGR_CLK_PLL4, 8 },
			{ PMGR_CLK_PLL4, 9 },
			{ PMGR_CLK_PLL4, 10 },
			{ PMGR_CLK_PLL4, 11 },
			{ PMGR_CLK_PLL4, 12 },
			{ PMGR_CLK_PLL4, 14 },
			{ PMGR_CLK_PLL4, 16 }
		}
	},

	[PMGR_CLK_MSR]	    =	{
		&rPMGR_MSR_CLK_CFG,
		{
			{ PMGR_CLK_OSC, 1 },
			{ PMGR_CLK_S0, 1 },
			{ PMGR_CLK_S1, 1 },
			{ PMGR_CLK_S2, 1 },
			{ PMGR_CLK_S3, 1 },
			{ PMGR_CLK_PLL4, 6 },
			{ PMGR_CLK_PLL4, 7 },
			{ PMGR_CLK_PLL4, 8 },
			{ PMGR_CLK_PLL4, 9 },
			{ PMGR_CLK_PLL4, 12 },
			{ PMGR_CLK_PLL4, 16 },
			{ PMGR_CLK_PLL4, 24 }
		}
	},

	[PMGR_CLK_AF]	    =	{
		&rPMGR_AF_CLK_CFG,
		{
			{ PMGR_CLK_OSC, 1 },
			{ PMGR_CLK_S0, 1 },
			{ PMGR_CLK_S1, 1 },
			{ PMGR_CLK_S2, 1 },
			{ PMGR_CLK_S3, 1 },
			{ PMGR_CLK_PLL4, 5 },
			{ PMGR_CLK_PLL4, 6 },
			{ PMGR_CLK_PLL4, 7 },
			{ PMGR_CLK_PLL4, 8 },
			{ PMGR_CLK_PLL4, 9 },
			{ PMGR_CLK_PLL4, 12 },
			{ PMGR_CLK_PLL4, 24 }
		}
	},

	[PMGR_CLK_LIO]	    =	{
		&rPMGR_LIO_CLK_CFG,
		{
			{ PMGR_CLK_OSC, 1 },
			{ PMGR_CLK_S0, 1 },
			{ PMGR_CLK_S1, 1 },
			{ PMGR_CLK_S2, 1 },
			{ PMGR_CLK_S3, 1 },
			{ PMGR_CLK_PLL4, 10 },
			{ PMGR_CLK_PLL4, 12 },
			{ PMGR_CLK_PLL4, 14 },
			{ PMGR_CLK_PLL4, 16 },
			{ PMGR_CLK_PLL4, 18 },
			{ PMGR_CLK_PLL4, 20 },
			{ PMGR_CLK_PLL4, 48 }
		}
	},

	[PMGR_CLK_MCA0_M]	    =	{
		&rPMGR_NCO_CLK_CFG(0),
		{
			{ PMGR_CLK_OSC, 1 },
			{ PMGR_CLK_S0, 1 },
			{ PMGR_CLK_S1, 1 },
			{ PMGR_CLK_S2, 1 },
			{ PMGR_CLK_S3, 1 },
			{ PMGR_CLK_NOT_SUPPORTED, 1 },
			{ PMGR_CLK_NOT_SUPPORTED, 1 },
			{ PMGR_CLK_NOT_SUPPORTED, 1 },
			{ PMGR_CLK_NOT_SUPPORTED, 1 },
			{ PMGR_CLK_NOT_SUPPORTED, 1 },
			{ PMGR_CLK_OSC, 2 },
			{ PMGR_CLK_OSC, 4 }
		}
	},

	[PMGR_CLK_MCA1_M]	    =	{
		&rPMGR_NCO_CLK_CFG(1),
		{
			{ PMGR_CLK_OSC, 1 },
			{ PMGR_CLK_S0, 1 },
			{ PMGR_CLK_S1, 1 },
			{ PMGR_CLK_S2, 1 },
			{ PMGR_CLK_S3, 1 },
			{ PMGR_CLK_NOT_SUPPORTED, 1 },
			{ PMGR_CLK_NOT_SUPPORTED, 1 },
			{ PMGR_CLK_NOT_SUPPORTED, 1 },
			{ PMGR_CLK_NOT_SUPPORTED, 1 },
			{ PMGR_CLK_NOT_SUPPORTED, 1 },
			{ PMGR_CLK_OSC, 2 },
			{ PMGR_CLK_OSC, 4 }
		}
	},

	[PMGR_CLK_MCA2_M]	    =	{
		&rPMGR_NCO_CLK_CFG(2),
		{
			{ PMGR_CLK_OSC, 1 },
			{ PMGR_CLK_S0, 1 },
			{ PMGR_CLK_S1, 1 },
			{ PMGR_CLK_S2, 1 },
			{ PMGR_CLK_S3, 1 },
			{ PMGR_CLK_NOT_SUPPORTED, 1 },
			{ PMGR_CLK_NOT_SUPPORTED, 1 },
			{ PMGR_CLK_NOT_SUPPORTED, 1 },
			{ PMGR_CLK_NOT_SUPPORTED, 1 },
			{ PMGR_CLK_NOT_SUPPORTED, 1 },
			{ PMGR_CLK_OSC, 2 },
			{ PMGR_CLK_OSC, 4 }
		}
	},

	[PMGR_CLK_MCA3_M]	    =	{
		&rPMGR_NCO_CLK_CFG(3),
		{
			{ PMGR_CLK_OSC, 1 },
			{ PMGR_CLK_S0, 1 },
			{ PMGR_CLK_S1, 1 },
			{ PMGR_CLK_S2, 1 },
			{ PMGR_CLK_S3, 1 },
			{ PMGR_CLK_NOT_SUPPORTED, 1 },
			{ PMGR_CLK_NOT_SUPPORTED, 1 },
			{ PMGR_CLK_NOT_SUPPORTED, 1 },
			{ PMGR_CLK_NOT_SUPPORTED, 1 },
			{ PMGR_CLK_NOT_SUPPORTED, 1 },
			{ PMGR_CLK_OSC, 2 },
			{ PMGR_CLK_OSC, 4 }
		}
	},

	[PMGR_CLK_MCA4_M]	    =	{
		&rPMGR_NCO_CLK_CFG(4),
		{
			{ PMGR_CLK_OSC, 1 },
			{ PMGR_CLK_S0, 1 },
			{ PMGR_CLK_S1, 1 },
			{ PMGR_CLK_S2, 1 },
			{ PMGR_CLK_S3, 1 },
			{ PMGR_CLK_NOT_SUPPORTED, 1 },
			{ PMGR_CLK_NOT_SUPPORTED, 1 },
			{ PMGR_CLK_NOT_SUPPORTED, 1 },
			{ PMGR_CLK_NOT_SUPPORTED, 1 },
			{ PMGR_CLK_NOT_SUPPORTED, 1 },
			{ PMGR_CLK_OSC, 2 },
			{ PMGR_CLK_OSC, 4 }
		}
	},

	[PMGR_CLK_SEP]	    =	{
		&rPMGR_SEP_CLK_CFG,
		{
			{ PMGR_CLK_OSC, 1 },
			{ PMGR_CLK_S0, 1 },
			{ PMGR_CLK_S1, 1 },
			{ PMGR_CLK_S2, 1 },
			{ PMGR_CLK_S3, 1 },
			{ PMGR_CLK_PLL4, 6 },
			{ PMGR_CLK_PLL4, 7 },
			{ PMGR_CLK_PLL4, 12 },
			{ PMGR_CLK_PLL4, 24 }
		}
	},

	[PMGR_CLK_GPIO]	    =	{
		&rPMGR_GPIO_CLK_CFG,
		{
			{ PMGR_CLK_OSC, 1 },
			{ PMGR_CLK_S0, 1 },
			{ PMGR_CLK_S1, 1 },
			{ PMGR_CLK_S2, 1 },
			{ PMGR_CLK_S3, 1 },
			{ PMGR_CLK_PLL4, 100 },
			{ PMGR_CLK_PLL4, 50 }
		}
	},

	[PMGR_CLK_SPI0_N]	    =	{
		&rPMGR_SPI0_N_CLK_CFG,
		{
			{ PMGR_CLK_OSC, 1 },
			{ PMGR_CLK_S0, 1 },
			{ PMGR_CLK_S1, 1 },
			{ PMGR_CLK_S2, 1 },
			{ PMGR_CLK_S3, 1 },
			{ PMGR_CLK_PLL4, 40 },
			{ PMGR_CLK_PLL4, 48 },
			{ PMGR_CLK_PLL4, 50 }
		}
	},

	[PMGR_CLK_SPI1_N]	    =	{
		&rPMGR_SPI1_N_CLK_CFG,
		{
			{ PMGR_CLK_OSC, 1 },
			{ PMGR_CLK_S0, 1 },
			{ PMGR_CLK_S1, 1 },
			{ PMGR_CLK_S2, 1 },
			{ PMGR_CLK_S3, 1 },
			{ PMGR_CLK_PLL4, 40 },
			{ PMGR_CLK_PLL4, 48 },
			{ PMGR_CLK_PLL4, 50 }
		}
	},

	[PMGR_CLK_SPI2_N]	    =	{
		&rPMGR_SPI2_N_CLK_CFG,
		{
			{ PMGR_CLK_OSC, 1 },
			{ PMGR_CLK_S0, 1 },
			{ PMGR_CLK_S1, 1 },
			{ PMGR_CLK_S2, 1 },
			{ PMGR_CLK_S3, 1 },
			{ PMGR_CLK_PLL4, 40 },
			{ PMGR_CLK_PLL4, 48 },
			{ PMGR_CLK_PLL4, 50 }
		}
	},

	[PMGR_CLK_SPI3_N]	    =	{
		&rPMGR_SPI3_N_CLK_CFG,
		{
			{ PMGR_CLK_OSC, 1 },
			{ PMGR_CLK_S0, 1 },
			{ PMGR_CLK_S1, 1 },
			{ PMGR_CLK_S2, 1 },
			{ PMGR_CLK_S3, 1 },
			{ PMGR_CLK_PLL4, 40 },
			{ PMGR_CLK_PLL4, 48 },
			{ PMGR_CLK_PLL4, 50 }
		}
	},

	[PMGR_CLK_DEBUG]	    =	{
		&rPMGR_DEBUG_CLK_CFG,
		{
			{ PMGR_CLK_OSC, 1 },
			{ PMGR_CLK_S0, 1 },
			{ PMGR_CLK_S1, 1 },
			{ PMGR_CLK_S2, 1 },
			{ PMGR_CLK_S3, 1 },
			{ PMGR_CLK_PLL4, 20 },
			{ PMGR_CLK_PLL4, 24 },
			{ PMGR_CLK_PLL4, 30 },
			{ PMGR_CLK_PLL4, 40 },
			{ PMGR_CLK_PLL4, 48 }
		}
	},

#if SUB_PLATFORM_T7000
	[PMGR_CLK_PCIE_REF]	=	{
		&rPMGR_PCIE_REF_CLK_CFG,
		{
			{ PMGR_CLK_PLL4, 24 },
			{ PMGR_CLK_PLL3, 1 },
		}
	},
#endif

	[PMGR_CLK_PCIE_APP]	=	{
		&rPMGR_PCIE_APP_CLK_CFG,
		{
			{ PMGR_CLK_OSC, 1 },
			{ PMGR_CLK_S0, 1 },
			{ PMGR_CLK_S1, 1 },
			{ PMGR_CLK_S2, 1 },
			{ PMGR_CLK_S3, 1 },
			{ PMGR_CLK_PLL4, 9 },
			{ PMGR_CLK_PLL4, 10 },
			{ PMGR_CLK_PLL4, 12 },
			{ PMGR_CLK_PLL4, 24 }
		}
	},

	[PMGR_CLK_TMPS]		=	{
		&rPMGR_TMPS_CLK_CFG,
		{
			{ PMGR_CLK_OSC, 1 },
			{ PMGR_CLK_S0, 1 },
			{ PMGR_CLK_S1, 1 },
			{ PMGR_CLK_S2, 1 },
			{ PMGR_CLK_S3, 1 },
			{ PMGR_CLK_OSC, 20 },
		}
	},

	[PMGR_CLK_MEDIA_AF]	=	{
		&rPMGR_MEDIA_AF_CLK_CFG,
		{
			{ PMGR_CLK_OSC, 1 },
			{ PMGR_CLK_S0, 1 },
			{ PMGR_CLK_S1, 1 },
			{ PMGR_CLK_S2, 1 },
			{ PMGR_CLK_S3, 1 },
			{ PMGR_CLK_PLL4, 5 },
			{ PMGR_CLK_PLL4, 6 },
			{ PMGR_CLK_PLL4, 7 },
			{ PMGR_CLK_PLL4, 8 },
			{ PMGR_CLK_PLL4, 9 },
			{ PMGR_CLK_PLL4, 12 },
			{ PMGR_CLK_PLL4, 24 },
		}
	},

	[PMGR_CLK_ISP_AF]	=	{
		&rPMGR_ISP_AF_CLK_CFG,
		{
			{ PMGR_CLK_OSC, 1 },
			{ PMGR_CLK_S0, 1 },
			{ PMGR_CLK_S1, 1 },
			{ PMGR_CLK_S2, 1 },
			{ PMGR_CLK_S3, 1 },
			{ PMGR_CLK_PLL4, 5 },
			{ PMGR_CLK_PLL4, 6 },
			{ PMGR_CLK_PLL4, 7 },
			{ PMGR_CLK_PLL4, 8 },
			{ PMGR_CLK_PLL4, 9 },
			{ PMGR_CLK_PLL4, 12 },
			{ PMGR_CLK_PLL4, 24 },
		}
	},

	[PMGR_CLK_GFX_AF]	=	{
		&rPMGR_GFX_AF_CLK_CFG,
		{
			{ PMGR_CLK_OSC, 1 },
			{ PMGR_CLK_S0, 1 },
			{ PMGR_CLK_S1, 1 },
			{ PMGR_CLK_S2, 1 },
			{ PMGR_CLK_S3, 1 },
			{ PMGR_CLK_PLL4, 5 },
			{ PMGR_CLK_PLL4, 6 },
			{ PMGR_CLK_PLL4, 7 },
			{ PMGR_CLK_PLL4, 8 },
			{ PMGR_CLK_PLL4, 9 },
			{ PMGR_CLK_PLL4, 12 },
			{ PMGR_CLK_PLL4, 24 },
		}
	},

	[PMGR_CLK_ANS_C]	    =	{
		&rPMGR_ANS_C_CLK_CFG,
		{
			{ PMGR_CLK_OSC, 1 },
			{ PMGR_CLK_S0, 1 },
			{ PMGR_CLK_S1, 1 },
			{ PMGR_CLK_S2, 1 },
			{ PMGR_CLK_S3, 1 },
			{ PMGR_CLK_PLL4, 5 },
			{ PMGR_CLK_PLL4, 6 },
			{ PMGR_CLK_PLL4, 7 },
			{ PMGR_CLK_PLL4, 8 },
			{ PMGR_CLK_PLL4, 9 },
			{ PMGR_CLK_PLL4, 10 },
			{ PMGR_CLK_PLL4, 48 }
		}
	},

	[PMGR_CLK_ANC_LINK]	    =	{
		&rPMGR_ANC_LINK_CLK_CFG,
		{
			{ PMGR_CLK_OSC, 1 },
			{ PMGR_CLK_S0, 1 },
			{ PMGR_CLK_S1, 1 },
			{ PMGR_CLK_S2, 1 },
			{ PMGR_CLK_S3, 1 },
			{ PMGR_CLK_PLL4, 9 },
			{ PMGR_CLK_PLL4, 7 },
			{ PMGR_CLK_PLL4, 8 },
			{ PMGR_CLK_PLL4, 12 },
			{ PMGR_CLK_PLL4, 18 },
			{ PMGR_CLK_PLL4, 24 }
		}
	},

	[PMGR_CLK_S0]	    =	{
		&rPMGR_S0_CLK_CFG,
		{
			{ PMGR_CLK_PLL4, 3 },
			{ PMGR_CLK_PLL2, 1 },
#if SUB_PLATFORM_T7000
			{ PMGR_CLK_PLL3, 1 },
#elif SUB_PLATFORM_T7001
			{ PMGR_CLK_PLL0, 1 },
#endif
			{ PMGR_CLK_PLL4, 2 }
		}
	},

	[PMGR_CLK_S1]	    =	{
		&rPMGR_S1_CLK_CFG,
		{
			{ PMGR_CLK_PLL4, 3 },
			{ PMGR_CLK_PLL2, 1 },
#if SUB_PLATFORM_T7000
			{ PMGR_CLK_PLL3, 1 },
#elif SUB_PLATFORM_T7001
			{ PMGR_CLK_PLL0, 1 },
#endif
			{ PMGR_CLK_PLL4, 2 }
		}
	},

	[PMGR_CLK_S2]	    =	{
		&rPMGR_S2_CLK_CFG,
		{
			{ PMGR_CLK_PLL4, 3 },
			{ PMGR_CLK_PLL2, 1 },
#if SUB_PLATFORM_T7000
			{ PMGR_CLK_PLL3, 1 },
#elif SUB_PLATFORM_T7001
			{ PMGR_CLK_PLL0, 1 },
#endif
			{ PMGR_CLK_PLL4, 2 }
		}
	},

	[PMGR_CLK_S3]	    =	{
		&rPMGR_S3_CLK_CFG,
		{
			{ PMGR_CLK_PLL4, 3 },
			{ PMGR_CLK_PLL2, 1 },
#if SUB_PLATFORM_T7000
			{ PMGR_CLK_PLL3, 1 },
#elif SUB_PLATFORM_T7001
			{ PMGR_CLK_PLL0, 1 },
#endif
			{ PMGR_CLK_PLL4, 2 }
		}
	},

	[PMGR_CLK_ISP_REF0]	    =	{
		&rPMGR_ISP_REF0_CLK_CFG,
		{
			{ PMGR_CLK_PLL4, 5 },
			{ PMGR_CLK_PLL4, 7 },
			{ PMGR_CLK_PLL4, 8 },
			{ PMGR_CLK_PLL4, 9 }
		}
	},

	[PMGR_CLK_ISP_REF1]	    =	{
		&rPMGR_ISP_REF1_CLK_CFG,
		{
			{ PMGR_CLK_PLL4, 5 },
			{ PMGR_CLK_PLL4, 7 },
			{ PMGR_CLK_PLL4, 8 },
			{ PMGR_CLK_PLL4, 9 }
		}
	},
};

static uint32_t get_apsc_ccc_state(void);
static void set_apsc_ccc_state(uint32_t target_state);
#if (APPLICATION_IBOOT && !PRODUCT_IBOOT && !PRODUCT_IBEC)
static void init_soc_thermal_sensors(void);
static void init_soc_sochot(void);
static void init_cpu_thermal_sensors(void);
static void init_cpu_sochot(void);
static void init_soc_tvm_tunables(void);
#endif
static void clocks_get_frequencies(void);
static void clocks_get_frequencies_range(uint32_t start_clk, uint32_t end_clk);
static uint32_t get_pll(int32_t pll);
static uint32_t get_pll_cpu(void);
static void set_pll(int32_t pll, uint32_t p, uint32_t m, uint32_t s, bool vco_output);
static uint32_t is_pll_running(int32_t pll);
static void set_running_pll(int32_t pll, uint32_t p, uint32_t m, uint32_t s);
static uint64_t pmgr_get_offset_from_diff_uV(uint32_t buck, uint32_t uV);
static uint32_t get_spare(int32_t spare);
static void clocks_set_gates(uint64_t *devices);
static void clocks_quiesce_internal(void);
static void power_on_sep(void);

// current clock frequencies
static uint32_t clks[PMGR_CLK_COUNT + 1];

void platform_power_spin(uint32_t usecs)
{
	arm_no_wfe_spin(usecs);
}

int clocks_init(void)
{
#if (APPLICATION_IBOOT && (PRODUCT_IBOOT || PRODUCT_IBEC || WITH_RECOVERY_MODE_IBSS))
	clks[PMGR_CLK_OSC] = OSC_FREQ;

	clocks_get_frequencies();

#endif /* (APPLICATION_IBOOT && (PRODUCT_IBOOT || PRODUCT_IBEC || WITH_RECOVERY_MODE_IBSS)) */

	return 0;
}

#if (APPLICATION_IBOOT && !PRODUCT_IBOOT && !PRODUCT_IBEC)
/* 
 * To avoid di/dt voltage drop at the end of PLL relock. rdar://problem/13112194.
 */
static void set_ccc_pll_relock_div2(void)
{
	uint32_t reg;
	
	reg = rCCC_PLL_CFG2;
	reg &= ~CCC_PLL_CFG2_RELOCKBYS2_S4_MASK;
	reg |= CCC_PLL_CFG2_RELOCKBYS2_S4_DIV2;
	rCCC_PLL_CFG2 = reg;
}

static void set_nco_clocks(void)
{
	uint32_t i;

	// Enable this NCO with alg_ref0_clk and nco_ref0_clk.
	for (i = 0; i < 5; i++)
		rPMGR_NCO_CLK_CFG(i) |= (1 << 31);
}

static void apply_pmgr_tunables()
{
#define CLAMP_TIME_MASK  (((1 << 8) - 1) << 8)
#define CLK_EN_TIME_MASK (((1 << 8) - 1) << 16)
#define RETN_SAVE_TIME_MASK (((1 << 3) - 1) << 24)
#define RETN_RESTORE_TIME_MASK (((1 << 3) - 1) << 27)
#define _PWRGATE_CFG0(rCFG0, RETN_RESTORE_TIME, RETN_SAVE_TIME, CLK_EN_TIME, CLAMP_TIME) \
	regTemp = rCFG0; \
	regTemp &= ~CLAMP_TIME_MASK; \
	regTemp |= (CLAMP_TIME << 8); \
	regTemp &= ~CLK_EN_TIME_MASK; \
	regTemp |= (CLK_EN_TIME << 16); \
	regTemp &= ~RETN_SAVE_TIME_MASK; \
	regTemp |= (RETN_SAVE_TIME << 24); \
	regTemp &= ~RETN_RESTORE_TIME_MASK; \
	regTemp |= (RETN_RESTORE_TIME << 27); \
	rCFG0 = regTemp;

#define RAMP_PRE_TIME_MASK (((1 << 12) - 1) << 0)
#define RAMP_ALL_TIME_MASK (((1 << 12) - 1) << 16)

#define _PWRGATE_CFG1(rCFG1, RAMP_ALL_TIME, RAMP_PRE_TIME) \
	regTemp = rCFG1; \
	regTemp &= ~RAMP_PRE_TIME_MASK;	\
	regTemp |= (RAMP_PRE_TIME << 0); \
	regTemp &= ~RAMP_ALL_TIME_MASK; \
	regTemp |= (RAMP_ALL_TIME << 16); \
	rCFG1 = regTemp;

#define RESET_DOWN_TIME_MASK (((1 << 8) - 1) << 0)
#define RESET_UP_TIME_MASK (((1 << 8) - 1) << 8)
#define RESET_OFF_TIME_MASK (((1 << 8) - 1) << 16)

#define _PWRGATE_CFG2(rCFG2, RESET_OFF_TIME, RESET_UP_TIME, RESET_DOWN_TIME) \
	regTemp = rCFG2; \
	regTemp &= ~RESET_DOWN_TIME_MASK;	\
	regTemp |= (RESET_DOWN_TIME << 0); \
	regTemp &= ~RESET_UP_TIME_MASK; \
	regTemp |= (RESET_UP_TIME << 8); \
	regTemp &= ~RESET_OFF_TIME_MASK; \
	regTemp |= (RESET_OFF_TIME << 16); \
	rCFG2 = regTemp;

#define DBG_CLK_ON_RAMP_MASK (1 << 4)

#define _PWRGATE_DBG(rDBG, DBG_CLK_ON_RAMP) \
	regTemp = rDBG; \
	regTemp &= ~DBG_CLK_ON_RAMP_MASK; \
	regTemp |= (DBG_CLK_ON_RAMP << 4); \
	rDBG = regTemp;

	uint32_t regTemp;

	// A0 tunables.

	_PWRGATE_CFG0(rPMGR_PWRGATE_AMC_CFG0, 0x1, 0x1, 0x7, 0x3);
#if SUB_PLATFORM_T7000
	_PWRGATE_CFG1(rPMGR_PWRGATE_AMC_CFG1, 0x13, 0x1);
#elif SUB_PLATFORM_T7001
	_PWRGATE_CFG1(rPMGR_PWRGATE_AMC_CFG1, 0x12, 0x1);
#endif
	_PWRGATE_CFG2(rPMGR_PWRGATE_AMC_CFG2, 0x7, 0x7, 0x0);

	_PWRGATE_CFG0(rPMGR_PWRGATE_USB_CFG0, 0x1, 0x1, 0x4, 0x3);
	_PWRGATE_CFG1(rPMGR_PWRGATE_USB_CFG1, 0xe, 0x1);
	_PWRGATE_CFG2(rPMGR_PWRGATE_USB_CFG2, 0x8, 0x8, 0x8);	

	_PWRGATE_CFG0(rPMGR_PWRGATE_ACS_CFG0, 0x1, 0x1, 0x6, 0x3);
#if SUB_PLATFORM_T7000
	_PWRGATE_CFG1(rPMGR_PWRGATE_ACS_CFG1, 0xe, 0x1);
#elif SUB_PLATFORM_T7001
	_PWRGATE_CFG1(rPMGR_PWRGATE_ACS_CFG1, 0x12, 0x1);
#endif
	_PWRGATE_CFG2(rPMGR_PWRGATE_ACS_CFG2, 0x6, 0x6, 0x0);

	_PWRGATE_CFG0(rPMGR_PWRGATE_DISP0_CFG0, 0x1, 0x1, 0x4, 0x3);
#if SUB_PLATFORM_T7000
	_PWRGATE_CFG1(rPMGR_PWRGATE_DISP0_CFG1, 0x33, 0x2);
#elif SUB_PLATFORM_T7001
	_PWRGATE_CFG1(rPMGR_PWRGATE_DISP0_CFG1, 0x3b, 0x3);
#endif
	_PWRGATE_CFG2(rPMGR_PWRGATE_DISP0_CFG2, 0x8, 0x8, 0x8);

	_PWRGATE_CFG0(rPMGR_PWRGATE_DISP1_CFG0, 0x1, 0x1, 0x4, 0x3);
#if SUB_PLATFORM_T7000
	_PWRGATE_CFG1(rPMGR_PWRGATE_DISP1_CFG1, 0x31, 0x3);
#elif SUB_PLATFORM_T7001
	_PWRGATE_CFG1(rPMGR_PWRGATE_DISP1_CFG1, 0x38, 0x3);
#endif
	_PWRGATE_CFG2(rPMGR_PWRGATE_DISP1_CFG2, 0x8, 0x8, 0x8);

	_PWRGATE_CFG0(rPMGR_PWRGATE_ISP_CFG0, 0x1, 0x1, 0x4, 0x3);
#if SUB_PLATFORM_T7000
	_PWRGATE_CFG1(rPMGR_PWRGATE_ISP_CFG1, 0x78, 0x3);
#elif SUB_PLATFORM_T7001
	_PWRGATE_CFG1(rPMGR_PWRGATE_ISP_CFG1, 0x5c, 0x2);
#endif
	_PWRGATE_CFG2(rPMGR_PWRGATE_ISP_CFG2, 0x8, 0x8, 0x8);
	_PWRGATE_DBG(rPMGR_PWRGATE_ISP_DBG, 0x1);

	_PWRGATE_CFG0(rPMGR_PWRGATE_MEDIA_CFG0, 0x1, 0x1, 0x4, 0x3);
#if SUB_PLATFORM_T7000
	_PWRGATE_CFG1(rPMGR_PWRGATE_MEDIA_CFG1, 0x95, 0x4);
#elif SUB_PLATFORM_T7001
	_PWRGATE_CFG1(rPMGR_PWRGATE_MEDIA_CFG1, 0x72, 0x3);	
#endif
	_PWRGATE_CFG2(rPMGR_PWRGATE_MEDIA_CFG2, 0x8, 0x8, 0x8);

	_PWRGATE_CFG0(rPMGR_PWRGATE_VDEC0_CFG0, 0x1, 0x1, 0x4, 0x3);
#if SUB_PLATFORM_T7000
	_PWRGATE_CFG1(rPMGR_PWRGATE_VDEC0_CFG1, 0x53, 0x2);
#elif SUB_PLATFORM_T7001
	_PWRGATE_CFG1(rPMGR_PWRGATE_VDEC0_CFG1, 0x36, 0x2);
#endif
	_PWRGATE_CFG2(rPMGR_PWRGATE_VDEC0_CFG2, 0x8, 0x8, 0x8);

	_PWRGATE_CFG0(rPMGR_PWRGATE_VENC_CPU_CFG0, 0x1, 0x1, 0x4, 0x3);
#if SUB_PLATFORM_T7000
	_PWRGATE_CFG1(rPMGR_PWRGATE_VENC_CPU_CFG1, 0x65, 0x3);
#elif SUB_PLATFORM_T7001
	_PWRGATE_CFG1(rPMGR_PWRGATE_VENC_CPU_CFG1, 0x58, 0x2);
#endif
	_PWRGATE_CFG2(rPMGR_PWRGATE_VENC_CPU_CFG2, 0x8, 0x8, 0x8);
	_PWRGATE_DBG(rPMGR_PWRGATE_VENC_CPU_DBG, 0x1);

	_PWRGATE_CFG0(rPMGR_PWRGATE_PCIE_CFG0, 0x1, 0x1, 0x4, 0x3);
#if SUB_PLATFORM_T7000
	_PWRGATE_CFG1(rPMGR_PWRGATE_PCIE_CFG1, 0x2b, 0x2);
#elif SUB_PLATFORM_T7001
	_PWRGATE_CFG1(rPMGR_PWRGATE_PCIE_CFG1, 0x56, 0x2);
#endif
	_PWRGATE_CFG2(rPMGR_PWRGATE_PCIE_CFG2, 0x8, 0x8, 0x8);

	_PWRGATE_CFG0(rPMGR_PWRGATE_ANS_CFG0, 0x1, 0x1, 0x4, 0x3);
#if SUB_PLATFORM_T7000
	_PWRGATE_CFG1(rPMGR_PWRGATE_ANS_CFG1, 0x3a, 0x2);
#elif SUB_PLATFORM_T7001
	_PWRGATE_CFG1(rPMGR_PWRGATE_ANS_CFG1, 0x2a, 0x2);
#endif
	_PWRGATE_CFG2(rPMGR_PWRGATE_ANS_CFG2, 0x8, 0x8, 0x8);

	_PWRGATE_CFG0(rPMGR_PWRGATE_GFX_CFG0, 0x1, 0x1, 0x4, 0x3);
#if SUB_PLATFORM_T7000
	_PWRGATE_CFG1(rPMGR_PWRGATE_GFX_CFG1, 0x13, 0x1);
#elif SUB_PLATFORM_T7001
	_PWRGATE_CFG1(rPMGR_PWRGATE_GFX_CFG1, 0x1f, 0x1);
#endif
	_PWRGATE_CFG2(rPMGR_PWRGATE_GFX_CFG2, 0x8, 0x8, 0x8);

	_PWRGATE_CFG0(rPMGR_PWRGATE_VENC_PIPE_CFG0, 0x1, 0x1, 0x4, 0x3);
#if SUB_PLATFORM_T7000
	_PWRGATE_CFG1(rPMGR_PWRGATE_VENC_PIPE_CFG1, 0x7c, 0x2);
#elif SUB_PLATFORM_T7001
	_PWRGATE_CFG1(rPMGR_PWRGATE_VENC_PIPE_CFG1, 0x5e, 0x2);
#endif
	_PWRGATE_CFG2(rPMGR_PWRGATE_VENC_PIPE_CFG2, 0x8, 0x8, 0x8);
	_PWRGATE_DBG(rPMGR_PWRGATE_VENC_PIPE_DBG, 0x1);

	_PWRGATE_CFG0(rPMGR_PWRGATE_VENC_ME0_CFG0, 0x1, 0x1, 0x4, 0x3);
#if SUB_PLATFORM_T7000
	_PWRGATE_CFG1(rPMGR_PWRGATE_VENC_ME0_CFG1, 0x55, 0x2);
#elif SUB_PLATFORM_T7001
	_PWRGATE_CFG1(rPMGR_PWRGATE_VENC_ME0_CFG1, 0x40, 0x2);
#endif
	_PWRGATE_CFG2(rPMGR_PWRGATE_VENC_ME0_CFG2, 0x8, 0x8, 0x8);
	_PWRGATE_DBG(rPMGR_PWRGATE_VENC_ME0_DBG, 0x1);

	_PWRGATE_CFG0(rPMGR_PWRGATE_VENC_ME1_CFG0, 0x1, 0x1, 0x4, 0x3);
#if SUB_PLATFORM_T7000
	_PWRGATE_CFG1(rPMGR_PWRGATE_VENC_ME1_CFG1, 0x55, 0x2);
#elif SUB_PLATFORM_T7001
	_PWRGATE_CFG1(rPMGR_PWRGATE_VENC_ME1_CFG1, 0x40, 0x2);
#endif
	_PWRGATE_CFG2(rPMGR_PWRGATE_VENC_ME1_CFG2, 0x8, 0x8, 0x8);
	_PWRGATE_DBG(rPMGR_PWRGATE_VENC_ME1_DBG, 0x1);

	// 2. Apply MCU Async reset timing.

	rPMGR_PWRGATE_MCU_ASYNC_RESET = (0x1 << 0) | (0x1 << 4) | (0x1 << 8);

	// 3. Apply VolMan tunables

	rPMGR_VOLMAN_VAR_SOC_DELAY  =  (0x5 << 0) | (0x6b << 10) | (0xbb8 << 20);
	rPMGR_VOLMAN_FIXED_DELAY =  (0x5 << 0) | (0x6b << 10) | (0xbb8 << 20);
	rPMGR_VOLMAN_GFX_DELAY  =  (0x5 << 0) | (0x6b << 10) | (0xbb8 << 20);
	rPMGR_VOLMAN_CPU_DELAY  =  (0x5 << 0) | (0x6b << 10) | (0xbb8 << 20);

	// 4. Apply Misc Spare tunable

	rPMGR_MISC_SPARE0 = 0x100;

	// 5. Apply PLL4 tunable
#if SUB_PLATFORM_T7000
	regTemp = rPMGR_PLL_DEBUG1(4);
	regTemp &= ~(PMGR_PLL_DEBUG1_RESERVE_IN_MASK << PMGR_PLL_DEBUG1_RESERVE_IN_SHIFT);
	regTemp |= (0x10 << PMGR_PLL_DEBUG1_RESERVE_IN_SHIFT);
	rPMGR_PLL_DEBUG1(4) = regTemp;
#endif

	return;
}

static void apply_ccc_tunables(void)
{
	rCCC_PRE_TD_TMR = 0x4;
	rCCC_PRE_FLUSH_TMR = 0x1000;
#if SUB_PLATFORM_T7000
	rCCC_PLL_DLY_CFG0 = (0x0 << 10) | 0x5;
#else
	rCCC_PLL_DLY_CFG0 = (0x0 << 10) | 0x5;
	rCCC_PSW_DLY = (0x7f << 22) | (0x1 << 16) | 0x60;
#endif
	rCCC_DCD_NRG_WGHT = 0x0c803ce24838c28aULL;
	rCCC_IEX_NRG_WGHT = 0x018a00a90046c597ULL;
	rCCC_LSU_NRG_WGHT = 0x62d37934a9854c2aULL;
	rCCC_NEX_NRG_WGHT0 = 0x6666574475795557ULL;
	rCCC_NEX_NRG_WGHT1 = 0xc6b7b54676896658ULL;
	rCCC_NEX_NRG_WGHT2 = 0x000000ba787a7aabULL;
	
}
#endif

struct dvfm_data
{
	uint32_t dvfm_state_vmax;
	uint32_t dvfm_state_iboot_cnt;
	uint32_t dvfm_state_vnom;
	uint32_t dvfm_state_vboost;
	uint32_t voltage_states1_count;
	uint32_t voltage_states1_size;
};

#if SUB_PLATFORM_T7001 || SUB_TARGET_J96 || SUB_TARGET_J97
#if APPLICATION_IBOOT

#if SUB_TARGET_J98 || SUB_TARGET_J99
static bool pmgr_check_use1608MHz_unbinned(uint32_t board_id, uint32_t board_rev)
{
	bool	use1608MHz_unbinned = false;
	switch (board_id & 0xf) {
		case TARGET_BOARD_ID_J98DEV:
		case TARGET_BOARD_ID_J99DEV:
			break;
		case TARGET_BOARD_ID_J98AP:
		case TARGET_BOARD_ID_J99AP:
			if ((board_rev & 0xf) == J99_PROTO3_BOARD_REV)
				use1608MHz_unbinned = true;
			break;
		default:
			panic("Unknown board id %d\n", board_id);
	}
	return use1608MHz_unbinned;
}

static bool pmgr_check_use1608MHz_binned(uint32_t board_id, uint32_t board_rev)
{
	bool	use1608MHz_binned = false;
	switch (board_id & 0xf) {
		case TARGET_BOARD_ID_J98DEV:
		case TARGET_BOARD_ID_J99DEV:
			break;
		case TARGET_BOARD_ID_J98AP:
		case TARGET_BOARD_ID_J99AP:
			if ((board_rev & 0xf) <= J99_PROTO3_P9_BOARD_REV)
				use1608MHz_binned = true;
			break;
		default:
			panic("Unknown board id %d\n", board_id);
	}
	return use1608MHz_binned;
}
#endif

bool pmgr_check_gpu_use650MHz_unbinned(uint32_t board_id, uint32_t board_rev)
{
	bool	use_gpu_650MHz_unbinned = false;
	switch (board_id & 0xf) {
#if SUB_TARGET_J98 || SUB_TARGET_J99
		case TARGET_BOARD_ID_J98AP:
		case TARGET_BOARD_ID_J99AP:
			if ((board_rev & 0xf) <= J99_EVT_BOARD_REV && chipid_get_fuse_revision() < MINIMUM_BINNED_GPU_P6_REVISION)
				use_gpu_650MHz_unbinned = true;
			break;
#endif
		default:
			break;
	}
	return use_gpu_650MHz_unbinned;
}

bool pmgr_check_gpu_use650MHz_binned(uint32_t board_id, uint32_t board_rev)
{
	bool	use_gpu_650MHz_binned = false;
	switch (board_id & 0xf) {
#if SUB_TARGET_J98 || SUB_TARGET_J99
		case TARGET_BOARD_ID_J98AP:
		case TARGET_BOARD_ID_J99AP:
			if ((board_rev & 0xf) <= J99_EVT_BOARD_REV && chipid_get_fuse_revision() >= MINIMUM_BINNED_GPU_P6_REVISION)
				use_gpu_650MHz_binned = true;
			break;
#endif
		default:
			break;
	}
	return use_gpu_650MHz_binned;
}

bool pmgr_check_gpu_use600MHz_unbinned(uint32_t board_id, uint32_t board_rev)
{
	bool    use_gpu_600MHz_unbinned = false;
#if SUB_TARGET_J96 || SUB_TARGET_J97
	use_gpu_600MHz_unbinned = true;
#endif
	return use_gpu_600MHz_unbinned;
}

bool pmgr_check_gpu_use600MHz_binned(uint32_t board_id, uint32_t board_rev)
{
        bool    use_gpu_600MHz_binned = false;
#if SUB_TARGET_J96 || SUB_TARGET_J97
        use_gpu_600MHz_binned = false;
#endif
        return use_gpu_600MHz_binned;
}

static bool pmgr_check_use1512MHz(uint32_t board_id, uint32_t board_rev)
{
	bool	use1512MHz = false;
	switch (board_id & 0xf) {
#if SUB_TARGET_J98 || SUB_TARGET_J99
		case TARGET_BOARD_ID_J98DEV:
		case TARGET_BOARD_ID_J99DEV:
			break;
		case TARGET_BOARD_ID_J98AP:
		case TARGET_BOARD_ID_J99AP:
			if ((board_rev & 0xf) <= J99_PROTO1_BOARD_REV_LOCAL)
				use1512MHz = true;
			break;
#endif
#if SUB_TARGET_J81 || SUB_TARGET_J82
		case TARGET_BOARD_ID_J81DEV:
		case TARGET_BOARD_ID_J82DEV:
			if ((board_rev & 0xf) == 0x3)
				use1512MHz = true;
		case TARGET_BOARD_ID_J81AP:
		case TARGET_BOARD_ID_J82AP:
			if ((board_rev & 0xf) >= 0x4)
				use1512MHz = true;
			break;
#endif
#if SUB_TARGET_J96 || SUB_TARGET_J97
		case TARGET_BOARD_ID_J96AP:
		case TARGET_BOARD_ID_J97AP:
		case TARGET_BOARD_ID_J96DEV:
		case TARGET_BOARD_ID_J97DEV:
			use1512MHz = true;
			break;
#endif
		default:
			panic("Unknown board id %d\n", board_id);
	}
	return use1512MHz;
}

static void pmgr_get_dvfm_data_from_vmax(struct dvfm_data *dvfm, uint32_t dvfm_state_vmax)
{
	dvfm->dvfm_state_vmax = dvfm_state_vmax;
	dvfm->voltage_states1_count = dvfm_state_vmax - kDVFM_STATE_V0 + 1;
	dvfm->voltage_states1_size = dvfm->voltage_states1_count * 2;
	dvfm->dvfm_state_iboot_cnt = dvfm_state_vmax + 1;
	dvfm->dvfm_state_vnom = kDVFM_STATE_V2;
	dvfm->dvfm_state_vboost = dvfm_state_vmax;
}

static void pmgr_get_dvfm_data(struct dvfm_data *dvfm)
{
	uint32_t	dvfm_state_vmax;
	// DVFM table is filled with kDVFM_STATE_IBOOT copy after the last valid state. Use it to find the last valid state.
	for (dvfm_state_vmax = CCC_DVFM_STATE_COUNT - 1; dvfm_state_vmax > kDVFM_STATE_IBOOT; dvfm_state_vmax--)
		if (rCCC_DVFM_ST(dvfm_state_vmax) != rCCC_DVFM_ST(kDVFM_STATE_IBOOT))
			break;
	pmgr_get_dvfm_data_from_vmax(dvfm, dvfm_state_vmax);
}

void pmgr_update_gfx_states(uint32_t board_id, uint32_t board_rev)
{
	uint32_t states = sizeof(gfx_states) / sizeof(gfx_states[0]);
	uint32_t cnt, invalid_cnt = 0;

	bool gpu_use650MHz = pmgr_check_gpu_use650MHz_binned(board_id, board_rev) || 
						 pmgr_check_gpu_use650MHz_unbinned(board_id, board_rev);
	bool gpu_use600MHz = pmgr_check_gpu_use600MHz_binned(board_id, board_rev) ||
						pmgr_check_gpu_use600MHz_unbinned(board_id, board_rev);

	// Default 365MHz 450MHz 500MHz 550MHz
	bool states_enable[] = {true, true, true, true, true, false, false,  true, true, true, true, false, false};
	if (gpu_use650MHz) {
		// 365MHz 450MHz 550MHz 650MHz
		states_enable[CHIPID_GPU_VOLTAGE_V2] = false;	// 500MHz
		states_enable[CHIPID_GPU_VOLTAGE_V2_DIDT] = false; 
		states_enable[CHIPID_GPU_VOLTAGE_V5] = true; // 650MHz
		states_enable[CHIPID_GPU_VOLTAGE_V5_DIDT] = true;
	} else if (gpu_use600MHz) {
		// 365MHz 450MHz 550MHz 600MHz
		states_enable[CHIPID_GPU_VOLTAGE_V2] = false; // 500MHz
		states_enable[CHIPID_GPU_VOLTAGE_V2_DIDT] = false;
		states_enable[CHIPID_GPU_VOLTAGE_V4] = true; // 600MHz
		states_enable[CHIPID_GPU_VOLTAGE_V4_DIDT] = true;
	} else if ((chipid_get_fuse_revision() < 0x1) && (chipid_get_chip_id() == 0x7001)) { // Capri A0
		// 365MHz 450MHz 500MMHz
		states_enable[CHIPID_GPU_VOLTAGE_V3] = false;	// 550MHz
		states_enable[CHIPID_GPU_VOLTAGE_V3_DIDT] = false;
	}

	for (cnt = 1; cnt < states; cnt++) {
		// The states are in this order: off,V0,V1,..,Vi,V0_DIDT,V1_DIDT,...,Vi_DIDT
		if (!states_enable[cnt])
			invalid_cnt++;
		else
			gfx_states[cnt - invalid_cnt] = gfx_states[cnt];
	}

	for (cnt = 1; cnt < states; cnt++) {
		// Invalidate the GFX states that are now duplicates from the shifting by setting them to 0
		if (cnt >= states - invalid_cnt)
			gfx_states[cnt] = gfx_states[CHIPID_GPU_VOLTAGE_OFF];

		set_gfx_perf_state(cnt, &gfx_states[cnt]);
	}

	dprintf(DEBUG_CRITICAL, "board_id=0x%x board_rev=0x%x gpu_use650MHz_binned=%d gpu_use650MHz_unbinned=%d  gpu_use600MHz_binned=%d gpu_use600MHz_unbinned=%d\n", 
		board_id, board_rev, (int)pmgr_check_gpu_use650MHz_binned(board_id, board_rev), 
		(int)pmgr_check_gpu_use650MHz_unbinned(board_id, board_rev),
		(int)pmgr_check_gpu_use600MHz_binned(board_id, board_rev),
		(int)pmgr_check_gpu_use600MHz_unbinned(board_id, board_rev));
}

/*
 * Set-up DVFM table with new states. This will override all previous states.
 */
void pmgr_update_dvfm(uint32_t board_id, uint32_t board_rev)
{
	bool		use1608MHz_binned = false;
	bool		use1608MHz_unbinned = false;
	bool		use1512MHz = pmgr_check_use1512MHz(board_id, board_rev);

	u_int32_t	cpu_vid;
#if SUB_PLATFORM_T7001
	u_int32_t	cpu_sram_vid;
	u_int64_t	cpu_dvfm_sram = 0;
#endif
	u_int32_t	dvfm_state;
	struct 		chipid_vol_adj adj;
	uint64_t 	vol_adj_temp = 0;

#if SUB_TARGET_J98 || SUB_TARGET_J99
	use1608MHz_binned = pmgr_check_use1608MHz_binned(board_id, board_rev);
	use1608MHz_unbinned = pmgr_check_use1608MHz_unbinned(board_id, board_rev);
#endif

	dprintf(DEBUG_CRITICAL, "board_id=0x%x board_rev=0x%x fuse_rev=%d use1512MHz=%d use1608MHz_binned=%d use1608MHz_unbinned=%d \n", 
			board_id, board_rev, chipid_get_fuse_revision(), (int)use1512MHz, (int)use1608MHz_binned, (int)use1608MHz_unbinned);

	if (use1608MHz_unbinned)
		dvfm_state = kDVFM_STATE_V6_UNBINNED;
	else if (use1608MHz_binned)
		dvfm_state = kDVFM_STATE_V6;
	else if (use1512MHz)
		return;
	else {
		// kDVFM_STATE_V5 and kDVFM_STATE_V0 are the same, so pmgr_get_dvfm_data() will detect that the max state is kDVFM_STATE_V4
		rCCC_DVFM_ST(kDVFM_STATE_V5) = rCCC_DVFM_ST(kDVFM_STATE_V0);
		return;
	}

	cpu_vid = chipid_get_cpu_voltage(dvfm_state);
	platform_convert_voltages(BUCK_CPU, 1, &cpu_vid);

	chipid_get_vol_adj(CHIPID_VOL_ADJ_CPU, dvfm_state, &adj);
	vol_adj_temp |= CCC_DVFM_ST_VOLADJ0(pmgr_get_offset_from_diff_uV(BUCK_CPU, adj.region_uV[0]));
	vol_adj_temp |= CCC_DVFM_ST_VOLADJ1(pmgr_get_offset_from_diff_uV(BUCK_CPU, adj.region_uV[1]));
	vol_adj_temp |= CCC_DVFM_ST_VOLADJ2(pmgr_get_offset_from_diff_uV(BUCK_CPU, adj.region_uV[2]));
	vol_adj_temp |= CCC_DVFM_ST_VOLADJ3(pmgr_get_offset_from_diff_uV(BUCK_CPU, adj.region_uV[3]));
	rCCC_DVFM_ST(kDVFM_STATE_V5) = ccc_dvfm_states[dvfm_state] | CCC_DVFM_ST_SAFEVOL(cpu_vid) | vol_adj_temp;

#if SUB_PLATFORM_T7001
	cpu_sram_vid = chipid_get_ram_voltage(dvfm_state);
	platform_convert_voltages(BUCK_RAM, 1, &cpu_sram_vid);
	cpu_dvfm_sram = CCC_DVFM_SRAM_STATE(cpu_dvfm_sram, kDVFM_STATE_V5, cpu_sram_vid);

	rCCC_DVFM_SRAM &= ~CCC_DVFM_SRAM_STATE(0ULL, kDVFM_STATE_V5, ~0ULL);
	rCCC_DVFM_SRAM |= cpu_dvfm_sram;
#endif
}
#endif
#endif

static uint64_t pmgr_get_offset_from_diff_uV(uint32_t buck, uint32_t uV)
{
// power API is only able to convert absolute value
// So, we need to convert abolute value and do the diff.
	uint32_t mV_prev = 0;
	uint32_t mV;
	uint32_t offset;
	uint32_t zero_mV;
	if (platform_get_dwi_to_mv(buck, 1) <= 0) // even if offset 0 is 0mV, offset 1 must be >0 mV.
		return 0; // This platform doesn't have HW PMU or emulation

	zero_mV = platform_get_dwi_to_mv(buck, 0);
	for (offset = 0; offset < 0xff; offset++) {
		mV = platform_get_dwi_to_mv(buck, offset);
		if ((mV - zero_mV) * 1000 >= uV)
		{
			// We look for the closest offset.
			// It can be offset or offset -1
			if ((uV - (mV_prev - zero_mV) * 1000) < ((mV - zero_mV) * 1000 - uV))
				return offset - 1;
			else
				return offset;
		}
		mV_prev = mV;
	}
	panic("No offset for %d uV on buck %d\n", uV, buck);
	return 0;
}

/*
 * clocks_set_default - called by SecureROM, LLB, iBSS main via
 * platform_init_setup_clocks, so the current state of the chip is
 * either POR, or whatever 'quiesce' did when leaving SecureROM.
 */

int clocks_set_default(void)
{
	uint32_t cnt;
#if APPLICATION_IBOOT
	u_int32_t	cpu_vid[kDVFM_STATE_IBOOT_CNT];
#if SUB_PLATFORM_T7000
	u_int32_t	soc_vid[kSOC_PERF_STATE_IBOOT_CNT];
#elif SUB_PLATFORM_T7001
	u_int32_t	gpu_sram_vid[kPMGR_GFX_STATE_MAX];
	u_int32_t	cpu_sram_vid[kDVFM_STATE_IBOOT_CNT];
	u_int64_t	cpu_dvfm_sram = 0;
#endif
	u_int32_t	gpu_vid[kPMGR_GFX_STATE_MAX];
#endif

	clks[PMGR_CLK_OSC] = OSC_FREQ;

	volatile uint32_t *clkcfgs = PMGR_FIRST_CLK_CFG;
	volatile uint32_t *spare_clkcfgs = PMGR_FIRST_SPARE_CLK_CFG;

	// Setup bypass DVFM state
	rCCC_DVFM_ST(kDVFM_STATE_BYPASS) = ccc_dvfm_states[kDVFM_STATE_BYPASS];

	// Begin: <rdar://problem/14933881> Secure ROM PLL calibration sequence changes
	{
		uint32_t	vco_override = rCFG_FUSE9;

		// Shift the override value for the CPU PLL to the LSB position.
		vco_override >>= CFG_FUSE9_PLL_CPU_SHIFT;

		// Is the override data for this PLL valid?
		if (vco_override & CFG_FUSE9_PLL_VCO_RCTRL_SEL) {
			uint32_t reg = rCCC_PLL_CFG1;
			reg &= ~(CCC_PLL_CFG1_PLL_VCO_RCTRL_OW_MASK
				 << CCC_PLL_CFG1_PLL_VCO_RCTRL_OW_SHIFT);
			reg |= (vco_override & CFG_FUSE9_PLL_VCO_RCTRL_OW_MASK)
					<< CCC_PLL_CFG1_PLL_VCO_RCTRL_OW_SHIFT;
			reg |= CCC_PLL_CFG1_PLL_VCO_RCTRL_SEL;
			rCCC_PLL_CFG1 = reg;
		}
	}
	// End: <rdar://problem/14933881> Secure ROM PLL calibration sequence changes

	// Change all clocks to something safe
	clocks_quiesce_internal();

	// Setup active DVFM and SOC PERF states for the stage of boot.
#if SUB_PLATFORM_T7001
#if APPLICATION_IBOOT
	/* T7001 doesn't have kSOC_PERF_STATE_VMIN */
	pmgr_soc_perf_states[kSOC_PERF_STATE_VMIN] = pmgr_soc_perf_states[kSOC_PERF_STATE_VNOM];
#endif
#endif
#if APPLICATION_SECUREROM
	rCCC_DVFM_ST(kDVFM_STATE_SECUREROM) = ccc_dvfm_states[kDVFM_STATE_SECUREROM];
	
	rPMGR_SOC_PERF_STATE_ENTRY_A(kSOC_PERF_STATE_SECUREROM) = pmgr_soc_perf_states[kSOC_PERF_STATE_SECUREROM].entry[0];
	rPMGR_SOC_PERF_STATE_ENTRY_B(kSOC_PERF_STATE_SECUREROM) = pmgr_soc_perf_states[kSOC_PERF_STATE_SECUREROM].entry[1];
	rPMGR_SOC_PERF_STATE_ENTRY_C(kSOC_PERF_STATE_SECUREROM) = pmgr_soc_perf_states[kSOC_PERF_STATE_SECUREROM].entry[2];
	rPMGR_SOC_PERF_STATE_ENTRY_D(kSOC_PERF_STATE_SECUREROM) = 0;
#endif

#if APPLICATION_IBOOT
#ifndef BUCK_CPU
#error BUCK_CPU not defined for this platform
#endif
	// Get the binned voltages and update the CCC DVFM state registers.
	platform_get_cpu_voltages(kDVFM_STATE_IBOOT_CNT, cpu_vid);
	platform_convert_voltages(BUCK_CPU, kDVFM_STATE_IBOOT_CNT, cpu_vid);
#if SUB_PLATFORM_T7001
#ifndef BUCK_RAM
#error BUCK_RAM not defined for this platform
#endif
	platform_get_ram_voltages(kDVFM_STATE_IBOOT_CNT, cpu_sram_vid);
	platform_convert_voltages(BUCK_RAM, kDVFM_STATE_IBOOT_CNT, cpu_sram_vid);
#endif

	for (cnt = kDVFM_STATE_IBOOT; cnt < kDVFM_STATE_IBOOT_CNT; cnt++) {
		struct chipid_vol_adj adj;
		uint64_t vol_adj_temp = 0;
		chipid_get_vol_adj(CHIPID_VOL_ADJ_CPU, cnt, &adj);
		vol_adj_temp |= CCC_DVFM_ST_VOLADJ0(pmgr_get_offset_from_diff_uV(BUCK_CPU, adj.region_uV[0]));
		vol_adj_temp |= CCC_DVFM_ST_VOLADJ1(pmgr_get_offset_from_diff_uV(BUCK_CPU, adj.region_uV[1]));
		vol_adj_temp |= CCC_DVFM_ST_VOLADJ2(pmgr_get_offset_from_diff_uV(BUCK_CPU, adj.region_uV[2]));
		vol_adj_temp |= CCC_DVFM_ST_VOLADJ3(pmgr_get_offset_from_diff_uV(BUCK_CPU, adj.region_uV[3]));
		rCCC_DVFM_ST(cnt) = ccc_dvfm_states[cnt] | CCC_DVFM_ST_SAFEVOL(cpu_vid[cnt]) | vol_adj_temp;
	}
#if SUB_PLATFORM_T7001
	cpu_dvfm_sram = CCC_DVFM_SRAM_STATE(cpu_dvfm_sram, kDVFM_STATE_BYPASS, cpu_sram_vid[kDVFM_STATE_BYPASS]);
	for (cnt = kDVFM_STATE_IBOOT; cnt < kDVFM_STATE_IBOOT_CNT; cnt++) {
		cpu_dvfm_sram = CCC_DVFM_SRAM_STATE(cpu_dvfm_sram, cnt, cpu_sram_vid[cnt]);
	}
#endif
	// APSC sleep state will use the bypass state with V0.

	rCCC_DVFM_ST(kDVFM_STATE_BYPASS) = ccc_dvfm_states[kDVFM_STATE_BYPASS] | CCC_DVFM_ST_SAFEVOL(cpu_vid[kDVFM_STATE_V0]);
	
	// To prevent crashes/hangs during update install due to 
	// mismatch of CCC clock config info between an old LLB and a new EDT+OS
	// we can populate the unused entries of the DVFM table with that of
	// Vmin, Fmin. That way we will always have a workable entry in the DVFM
	// table.
	// Assumptions: 
	// There is always a valid state for iBoot and we will use that to populate the
	// empty table entries.

	for (cnt = kDVFM_STATE_IBOOT_CNT; cnt < CCC_DVFM_STATE_COUNT; cnt++) {
		rCCC_DVFM_ST(cnt) = rCCC_DVFM_ST(kDVFM_STATE_IBOOT);
	}
#if SUB_PLATFORM_T7001
	for (cnt = kDVFM_STATE_IBOOT_CNT; cnt < CCC_DVFM_STATE_COUNT; cnt++) {
		cpu_dvfm_sram = CCC_DVFM_SRAM_STATE(cpu_dvfm_sram, cnt, cpu_sram_vid[kDVFM_STATE_IBOOT]);
	}
	
	rCCC_DVFM_SRAM = cpu_dvfm_sram;	
#endif

	// Configure temperature ranges and measurement offsets for DVFM/DVTM
	rCCC_DVFM_CFG = CCC_DVFM_CFG_TEMPTHRESH0(0x8) | CCC_DVFM_CFG_TEMPTHRESH1(0x21) | CCC_DVFM_CFG_TEMPOFFSET0(0) | CCC_DVFM_CFG_TEMPOFFSET1(0);
	rCCC_DVFM_CFG1 = CCC_DVFM_CFG1_TEMPTHRESH2(0x3A) | CCC_DVFM_CFG1_TEMPOFFSET2(0);
#endif

#if APPLICATION_IBOOT
	for (cnt = kSOC_PERF_STATE_IBOOT; cnt < kSOC_PERF_STATE_IBOOT_CNT; cnt++) {
		uint32_t vol_adj_temp = 0;
#if SUB_PLATFORM_T7000
		struct chipid_vol_adj adj;
		chipid_get_vol_adj(CHIPID_VOL_ADJ_SOC, cnt, &adj);
		vol_adj_temp |= PMGR_SOC_PERF_STATE_VOL_ADJ0(pmgr_get_offset_from_diff_uV(BUCK_CPU, adj.region_uV[0]));
		vol_adj_temp |= PMGR_SOC_PERF_STATE_VOL_ADJ1(pmgr_get_offset_from_diff_uV(BUCK_CPU, adj.region_uV[1]));
		vol_adj_temp |= PMGR_SOC_PERF_STATE_VOL_ADJ2(pmgr_get_offset_from_diff_uV(BUCK_CPU, adj.region_uV[2]));
		vol_adj_temp |= PMGR_SOC_PERF_STATE_VOL_ADJ3(pmgr_get_offset_from_diff_uV(BUCK_CPU, adj.region_uV[3]));
#endif

		rPMGR_SOC_PERF_STATE_ENTRY_A(cnt) = pmgr_soc_perf_states[cnt].entry[0];
		rPMGR_SOC_PERF_STATE_ENTRY_B(cnt) = pmgr_soc_perf_states[cnt].entry[1];
		rPMGR_SOC_PERF_STATE_ENTRY_C(cnt) = pmgr_soc_perf_states[cnt].entry[2];
		rPMGR_SOC_PERF_STATE_ENTRY_D(cnt) = vol_adj_temp;
	}
	
	for (cnt = kSOC_PERF_STATE_IBOOT_CNT; cnt < PMGR_SOC_PERF_STATE_COUNT; cnt++) {
		rPMGR_SOC_PERF_STATE_ENTRY_A(cnt) = rPMGR_SOC_PERF_STATE_ENTRY_A(kSOC_PERF_STATE_VMIN);
		rPMGR_SOC_PERF_STATE_ENTRY_B(cnt) = rPMGR_SOC_PERF_STATE_ENTRY_B(kSOC_PERF_STATE_VMIN);
		rPMGR_SOC_PERF_STATE_ENTRY_C(cnt) = rPMGR_SOC_PERF_STATE_ENTRY_C(kSOC_PERF_STATE_VMIN);
		rPMGR_SOC_PERF_STATE_ENTRY_D(cnt) = rPMGR_SOC_PERF_STATE_ENTRY_D(kSOC_PERF_STATE_VMIN);
	}

#if SUB_PLATFORM_T7000
#ifndef BUCK_SOC
#error BUCK_SOC not defined for this platform
#endif
	platform_get_soc_voltages(kSOC_PERF_STATE_IBOOT_CNT, soc_vid);
	platform_convert_voltages(BUCK_SOC, kSOC_PERF_STATE_IBOOT_CNT, soc_vid);

	for (cnt = kSOC_PERF_STATE_IBOOT; cnt < kSOC_PERF_STATE_IBOOT_CNT; cnt++) {
		rPMGR_SOC_PERF_STATE_ENTRY_C(cnt) = soc_vid[cnt];
	}
	
	for (cnt = kSOC_PERF_STATE_IBOOT_CNT; cnt < PMGR_SOC_PERF_STATE_COUNT; cnt++) {
		rPMGR_SOC_PERF_STATE_ENTRY_C(cnt) = soc_vid[kSOC_PERF_STATE_VMIN];
	}
#endif
#endif

#if APPLICATION_IBOOT
#if WITH_HW_DWI
	extern int dwi_init(void);
	dwi_init();
#endif
#endif

	set_apsc_ccc_state(active_state); // CCC clock set for this stage of boot.

#if APPLICATION_IBOOT
#ifndef BUCK_GPU
#error BUCK_GPU not defined for this platform
#endif
	
	platform_get_gpu_voltages(sizeof(gfx_states) / sizeof(gfx_states[0]), gpu_vid);
#if SUB_PLATFORM_T7000
	// Workaround T7000 <rdar://problem/18252610> start
	for (cnt = 2; cnt < sizeof(gfx_states) / sizeof(gfx_states[0]); cnt++) {
		if (gpu_vid[cnt] < gpu_vid[1]) {
			gpu_vid[cnt] = gpu_vid[1];
		}
	}
	// Workaround T7000 <rdar://problem/18252610> end
#endif
	platform_convert_voltages(BUCK_GPU, sizeof(gfx_states) / sizeof(gfx_states[0]), gpu_vid);

	for (cnt = 0; cnt < sizeof(gfx_states) / sizeof(gfx_states[0]); cnt++) {			
		if (cnt == 0)
			gfx_states[cnt].dwi_val = 0;
		else
			gfx_states[cnt].dwi_val = gpu_vid[cnt];
	}
#if SUB_PLATFORM_T7001
#ifndef BUCK_RAM
#error BUCK_RAM not defined for this platform
#endif
	platform_get_gpu_ram_voltages(sizeof(gfx_states) / sizeof(gfx_states[0]), gpu_sram_vid);
	platform_convert_voltages(BUCK_RAM, sizeof(gfx_states) / sizeof(gfx_states[0]), gpu_sram_vid);

	for (cnt = 0; cnt < sizeof(gfx_states) / sizeof(gfx_states[0]); cnt++) {
		if (cnt == 0)
			gfx_states[cnt].dwi_sram_val = 0;
		else
			gfx_states[cnt].dwi_sram_val = gpu_sram_vid[cnt];
	}
#endif
	for (cnt = 0; cnt < sizeof(gfx_states) / sizeof(gfx_states[0]); cnt++) {
		set_gfx_perf_state(cnt, &gfx_states[cnt]);
	}
	// Complete the PERF table with GPU_VOLTAGE_OFF value
	for (cnt = sizeof(gfx_states) / sizeof(gfx_states[0]); cnt < kPMGR_GFX_STATE_MAX; cnt++) {
		set_gfx_perf_state(cnt, &gfx_states[CHIPID_GPU_VOLTAGE_OFF]);
	}
	// As per GFX Performance state software Sequence
	// Clear the bypass bit in PLL5
	while (rPMGR_PLL_CTL(5) & PMGR_PLL_PENDING);
	rPMGR_PLL_EXT_BYPASS_CTL(5) &= ~PMGR_PLL_EXT_BYPASS;
	// PLL5 relock mode is set to 1 to switch to bypass mode while re-locking.
	rPMGR_PLL_CFG(5) |= (PMGR_PLL_RELOCK_MODE_BYPASS << PMGR_PLL_RELOCK_MODE_SHIFT);
	// Enable performance state table to control PLL5
	rPMGR_GFX_PERF_STATE_CTL |= (1 << 31);

	// The initial SRAM, SOC, CPU voltages are set by IIC writes to the PMU (in the pmu driver).
#endif

#ifdef PLL0_T
#if WITH_HW_AGC_MIPI
	rPMGR_AGILE_CLK_CTL &= ~(1 << 29);
#endif
	set_pll(0, PLL0_P, PLL0_M, PLL0_S, 0);
#endif

#ifdef PLL1_T
	set_pll(1, PLL1_P, PLL1_M, PLL1_S, 0);
#endif

#ifdef PLL2_T
	set_pll(2, PLL2_P, PLL2_M, PLL2_S, 0);
#endif

#ifdef PLL3_T
	set_pll(3, PLL3_P, PLL3_M, PLL3_S, 0);
#if SUB_PLATFORM_T7001
	uint32_t	pll_vreg_adj = (rCFG_FUSE8 >> CFG_FUSE8_PCIE_REF_PLL_VREG_ADJ_SHIFT) & CFG_FUSE8_PCIE_REF_PLL_VREG_ADJ_MASK;
	if (pll_vreg_adj == 5) {
		uint32_t	reg = rPMGR_PLL_ANA_PARAMS0(3);
		reg &= ~(PMGR_PLL_ANA_PARAMS0_VREG_ADJ_MASK << PMGR_PLL_ANA_PARAMS0_VREG_ADJ_SHIFT);
		reg |= pll_vreg_adj << PMGR_PLL_ANA_PARAMS0_VREG_ADJ_SHIFT;
		rPMGR_PLL_ANA_PARAMS0(3) = reg;
	}
#endif
#endif

#ifdef PLL4_T
	set_pll(4, PLL4_P, PLL4_M, PLL4_S, PLL4_VCO_ENABLED);
#endif

#ifdef PLL5_T
	set_pll(5, PLL5_P, PLL5_M, PLL5_S, 0);
#endif

#if (APPLICATION_IBOOT && !PRODUCT_IBOOT && !PRODUCT_IBEC)
	// Turn on NCO clocks before enabling MCA clocks.
	set_nco_clocks();
#endif

	rPMGR_SOC_PERF_STATE_CTL = SOC_PERF_STATE_ACTIVE;
	while (rPMGR_SOC_PERF_STATE_CTL & PMGR_SOC_PERF_STATE_CTL_PENDING_MASK);

	// Set all spare clock divs to their active values
	for (cnt = 0; cnt < PMGR_SPARE_CLK_CFG_COUNT; cnt++) {
		spare_clkcfgs[cnt] = spare_divs_active[cnt];
		while ((spare_clkcfgs[cnt] & PMGR_CLK_CFG_PENDING) != 0);
	}

	// Set all but the spare clock divs to their active values
	for (cnt = 0; cnt < PMGR_CLK_CFG_COUNT; cnt++) {
		clkcfgs[cnt] = clk_divs_active[cnt];
		while ((clkcfgs[cnt] & PMGR_CLK_CFG_PENDING) != 0);
	}

	power_on_sep();

	clocks_get_frequencies();

#if (APPLICATION_IBOOT && !PRODUCT_IBOOT && !PRODUCT_IBEC)
	apply_pmgr_tunables();
	apply_ccc_tunables();
#endif

	return 0;
}

static	uint32_t get_apsc_ccc_state(void)
{
	return ((rCCC_DVFM_CFG_SEL >> 3) & 0x7);
}

static void set_apsc_ccc_state(uint32_t target_state)
{
	// DWI clock must be enabled for any state change that has voltage control enabled.
	rCCC_APSC_SCR = CCC_APSC_MANUAL_CHANGE(target_state);
	while ((rCCC_APSC_SCR & CCC_APSC_PENDING) != 0);

	return;
}

/*
 * set_pll - called by SecureROM, LLB, iBSS with PLLs in default reset state.
 * See restore_clock_config_state(). PLL1 is not enabled on entry to LLB or
 * or iBSS, since not configured by SecureROM.
 */
static void set_pll(int32_t pll, uint32_t p, uint32_t m, uint32_t s, bool vco_output)
{
	if (pll >= PMGR_PLL_COUNT)
		return;

	// <rdar://problem/14933881> Secure ROM PLL calibration sequence changes
	{
		uint32_t	vco_override = rCFG_FUSE9;

		// Each PMGR PLL's override values occupy contiguous 4-bit fields.
		// Shift the override value for this PLL to the LSB position.
		vco_override >>= (pll * 4);

		// Is the override data for this PLL valid?
		if (vco_override & CFG_FUSE9_PLL_VCO_RCTRL_SEL) {
			uint32_t reg = rPMGR_PLL_ANA_PARAMS1(pll);
			reg &= ~(PMGR_PLL_ANA_PARAMS1_VCO_RCTRL_OW_MASK
				 << PMGR_PLL_ANA_PARAMS1_VCO_RCTRL_OW_SHIFT);
			reg |= (vco_override & CFG_FUSE9_PLL_VCO_RCTRL_OW_MASK)
				 << PMGR_PLL_ANA_PARAMS1_VCO_RCTRL_OW_SHIFT;
			reg |= PMGR_PLL_ANA_PARAMS1_VCO_RCTRL_SEL;
			rPMGR_PLL_ANA_PARAMS1(pll) = reg;
		}
	}

	if (vco_output)
		rPMGR_PLL_CFG(pll) |= PMGR_PLL_VCO_OUT_SEL;

	if (p == 0) // not a PLL setting
		return;

	rPMGR_PLL_CTL(pll) = (PMGR_PLL_P(p) | PMGR_PLL_M(m) | PMGR_PLL_S(s) |
			      PMGR_PLL_LOAD);
	while (rPMGR_PLL_CTL(pll) & PMGR_PLL_PENDING);

	rPMGR_PLL_EXT_BYPASS_CTL(pll) &= ~PMGR_PLL_EXT_BYPASS;
	while ((rPMGR_PLL_EXT_BYPASS_CTL(pll) & PMGR_PLL_BYP_ENABLED) != 0);

	rPMGR_PLL_CTL(pll) |= PMGR_PLL_ENABLE;
	while (rPMGR_PLL_CTL(pll) & PMGR_PLL_PENDING);
}

static uint32_t is_pll_running(int32_t pll)
{
	if (pll >= PMGR_PLL_COUNT)
		return 0;

	if ((rPMGR_PLL_CTL(pll) & PMGR_PLL_ENABLE) == 0)
		return 0;
	else
		return 1;
}

/*
 * set_running_pll - program PLL that is already running.
 */
static void set_running_pll(int32_t pll, uint32_t p, uint32_t m, uint32_t s)
{
	if (pll >= PMGR_PLL_COUNT)
		return;

	if (!is_pll_running(pll))
		return;

	rPMGR_PLL_CTL(pll) = (PMGR_PLL_ENABLE | PMGR_PLL_P(p) | PMGR_PLL_M(m) | PMGR_PLL_S(s) | PMGR_PLL_LOAD);
	while (rPMGR_PLL_CTL(pll) & PMGR_PLL_PENDING);
}

static uint32_t get_pll_cpu(void)
{
	uint32_t freq;
	uint32_t pllctl;

	pllctl = rCCC_PWRCTRL_PLL_SCR1;

	// Fcpu <= ((OSC * M) / P / S+1)
	freq = OSC_FREQ;
	freq *= (pllctl >> CCC_PLL_M_SHIFT) & CCC_PLL_M_MASK;
	freq /= (pllctl >> CCC_PLL_P_SHIFT) & CCC_PLL_P_MASK;
	freq /= 1 + ((pllctl >> CCC_PLL_S_SHIFT) & CCC_PLL_S_MASK);

	return freq;
}

static uint32_t get_pll(int32_t pll)
{
	uint32_t pllctl;
	uint32_t pllbyp;
	uint64_t freq = 0;

	pllctl = rPMGR_PLL_CTL(pll);
	pllbyp = rPMGR_PLL_EXT_BYPASS_CTL(pll);

	// If PLL is not enabled, check for External Bypass
	if ((pllctl & PMGR_PLL_ENABLE) == 0) {
		if ((pllbyp & PMGR_PLL_EXT_BYPASS) == 0)
			return 0;
		else
			return OSC_FREQ;
	}

	freq = OSC_FREQ;

	freq *= ((pllctl >> PMGR_PLL_M_SHIFT) & PMGR_PLL_M_MASK);

	if ((rPMGR_PLL_CFG(pll) & PMGR_PLL_VCO_OUT_SEL) == 0) {
		freq /= (1 + ((pllctl >> PMGR_PLL_S_SHIFT) & PMGR_PLL_S_MASK));
	} else {
		freq *= 2;
	}

	freq /= ((pllctl >> PMGR_PLL_P_SHIFT) & PMGR_PLL_P_MASK);

#if DEBUG_BUILD
	if (freq > 0xFFFFFFFF)
		panic("Frequency value does not fit in uint32_t");
#endif

	return (uint32_t)freq;
}


static uint32_t get_spare(int32_t spare)
{
	uint32_t reg_val, src_idx, src_clk, src_factor, div;
	volatile uint32_t *spare_clkcfg = clk_configs[PMGR_CLK_S0 + spare].clock_reg;

	reg_val = *spare_clkcfg;

	div = reg_val & 0x3F;

	if (((reg_val & PMGR_CLK_CFG_ENABLE) == 0) || div == 0)
		return 0;

	src_idx = (reg_val >> PMGR_CLK_CFG_SRC_SEL_SHIFT) & PMGR_CLK_CFG_SRC_SEL_MASK;
	src_clk = clk_configs[PMGR_CLK_S0 + spare].sources[src_idx].src_clk;
	src_factor = clk_configs[PMGR_CLK_S0 + spare].sources[src_idx].factor;

	return (clks[src_clk] / src_factor) / div;
}

static void clocks_get_frequencies(void)
{
#if SUPPORT_FPGA
	uint32_t cnt;
	uint32_t freq = OSC_FREQ;

	for (cnt = 0; cnt < PMGR_CLK_COUNT; cnt++)
		clks[cnt] = freq;

	clks[PMGR_CLK_CPU] = 5000000;
	clks[PMGR_CLK_MCU] = 10000000;
	clks[PMGR_CLK_MCU_FIXED] = 10000000;
	clks[PMGR_CLK_USB] = 12000000;

#elif SUB_TARGET_TYPHONIC
	uint32_t cnt;
	uint32_t freq = OSC_FREQ;

	for (cnt = 0; cnt < PMGR_CLK_COUNT; cnt++)
		clks[cnt] = freq;

#else
	uint32_t cnt;

	clks[PMGR_CLK_OSC] = OSC_FREQ;

	// Use get_pll() to establish the frequencies (unconfigured PLLs will bypass OSC)
	for (cnt = 0; cnt < PMGR_PLL_COUNT; cnt++)
		clks[PMGR_CLK_PLL0 + cnt] = get_pll(cnt);

	// Use get_spare() to establish the frequencies for spare clocks (unconfigured will be skipped)
	for (cnt = 0; cnt < PMGR_SPARE_CLK_CFG_COUNT; cnt++)
		clks[PMGR_CLK_S0 + cnt] = get_spare(cnt);

	clks[PMGR_CLK_CPU] = get_pll_cpu();

	clocks_get_frequencies_range(PMGR_CLK_MCU_FIXED, PMGR_CLK_ANC_LINK);
#endif
}

static void clocks_get_frequencies_range(uint32_t start_clk, uint32_t end_clk)
{
	volatile uint32_t *reg;
	uint32_t cnt, val, src_idx, src_clk, src_factor;

	if (start_clk < PMGR_CLK_SOURCE_FIRST || end_clk > PMGR_CLK_SOURCE_LAST)
		return;

	for (cnt = start_clk; cnt <= end_clk; cnt++) {
		reg = clk_configs[cnt].clock_reg;
		val = *reg;

		if ((val & PMGR_CLK_CFG_ENABLE) == 0) {
			clks[cnt] = 0;
			continue;
		}

		src_idx = (val >> PMGR_CLK_CFG_SRC_SEL_SHIFT) & PMGR_CLK_CFG_SRC_SEL_MASK;
		src_clk = clk_configs[cnt].sources[src_idx].src_clk;
		src_factor = clk_configs[cnt].sources[src_idx].factor;
		clks[cnt] = clks[src_clk] / src_factor;
	}
}

void dump_clock_frequencies()
{
	uint32_t i;

	for (i = 0; i < PMGR_CLK_COUNT; i++) {
		dprintf(DEBUG_CRITICAL, "clk[%d] -> %u\n", i, clks[i]);
	}
}

static void clock_update_range(uint32_t first, uint32_t last, const uint32_t clkdata)
{
	volatile uint32_t *clkcfgs = PMGR_FIRST_CLK_CFG;
	uint32_t reg;

	reg = first;
	while (reg <= last) {
		clkcfgs[reg] = clkdata;
		while (clkcfgs[reg] & PMGR_CLK_CFG_PENDING);
		reg++;
	}
}

static void clock_update_frequency(uint32_t clk, uint32_t freq)
{
#define ROUND_10E4(_x) ((((_x) + 5000) / 10000) * 10000)

	uint32_t src_idx, src_clk, src_factor, reg;
	bool freq_supported = false;
	volatile uint32_t *clkcfg = clk_configs[clk].clock_reg;

	if (freq == 0)
		return;

	for (src_idx = 0; src_idx < CLOCK_SOURCES_MAX && clk_configs[clk].sources[src_idx].factor != 0; src_idx++)
	{
		src_clk = clk_configs[clk].sources[src_idx].src_clk;
		src_factor = clk_configs[clk].sources[src_idx].factor;

		if (ROUND_10E4(clks[src_clk] / src_factor) == freq) {
			freq_supported = true;
			break;
		}	
	}
			
	if (!freq_supported) {
		// Frequency not supported by CLK_CFG register, try spare
		uint32_t spare, spare_src_idx, spare_src_freq, spare_div;
		const struct clock_source *spare_clk_src;
		volatile uint32_t *spare_clkcfgs = PMGR_FIRST_SPARE_CLK_CFG;

		switch (clk)
		{
			case PMGR_CLK_VID0:
				// Use spare 0 with PLL4 div 3 as source clock
				spare = 0;
				spare_src_idx = 0;

				// Set VID0 clock source to spare 0
				src_idx = 1;

				break;

			default:
			return;
		}

		spare_clk_src = &clk_configs[PMGR_CLK_S0 + spare].sources[spare_src_idx];
		spare_src_freq = clks[spare_clk_src->src_clk] / spare_clk_src->factor;

		for (spare_div = 1; spare_div < 16; spare_div++) {
			if (ROUND_10E4(spare_src_freq / spare_div) == freq) {
				freq_supported = true;
				break;
		}
}

		if (freq_supported) {
			// Configure spare
			spare_clkcfgs[spare] = PMGR_CLK_CFG_ENABLE | (spare_src_idx << PMGR_CLK_CFG_SRC_SEL_SHIFT) | PMGR_CLK_CFG_DIVISOR(spare_div);
			while ((spare_clkcfgs[spare] & PMGR_CLK_CFG_PENDING) != 0);

			clks[PMGR_CLK_S0 + spare] = get_spare(spare);
		}
	}
	
	if (freq_supported) {
		// Configure clock
		reg = *clkcfg;
		reg &= ~(PMGR_CLK_CFG_SRC_SEL_MASK << PMGR_CLK_CFG_SRC_SEL_SHIFT);
		reg |= (src_idx & PMGR_CLK_CFG_SRC_SEL_MASK) << PMGR_CLK_CFG_SRC_SEL_SHIFT;
		*clkcfg = reg;
		while (*clkcfg & PMGR_CLK_CFG_PENDING);
	}
}

static void restore_clock_config_state(void)
{
	uint32_t cnt;
	uint32_t soc_perf_state;

	// 3. Write reset value to ACG, CLK_DIVIDER_ACG_CFG
	rPMGR_MISC_ACG = 0;
	rPMGR_CLK_DIVIDER_ACG_CFG = 0;

#if (APPLICATION_IBOOT && !PRODUCT_IBOOT && !PRODUCT_IBEC)
	// 4. Write the reset value to all MCAx_CLK_CFG registers
	clock_update_range(PMGR_CLK_NUM(MCA0_M), PMGR_CLK_NUM(MCA4_M), 0x80100000);
	
	// 5. Put the NCOs in reset state
	for (cnt = 0; cnt < PMGR_NUM_NCO; cnt++)
	{
		rPMGR_NCO_CLK_CFG(cnt) = 0;
		while (rPMGR_NCO_CLK_CFG(cnt) & PMGR_NCO_CLK_CFG_PENDING);
	}
#endif

	// 6. Write reset value for all mux clock configs (excluding spares)
	clock_update_range(PMGR_CLK_NUM(MCU_FIXED), PMGR_CLK_NUM(ANC_LINK), 0x80100000);

	soc_perf_state = (rPMGR_SOC_PERF_STATE_CTL >> PMGR_SOC_PERF_STATE_CTL_CURRENT_SELECT_SHIFT) &
		PMGR_SOC_PERF_STATE_CTL_CURRENT_SELECT_MASK;

	if (soc_perf_state != kSOC_PERF_STATE_BYPASS) {
		// 7a. Write the desired DRAM clock configuration state into the SOC_PERF_STATE_ENTRY_0A register
		rPMGR_SOC_PERF_STATE_ENTRY_A(kSOC_PERF_STATE_BYPASS) =
			(pmgr_soc_perf_states[kSOC_PERF_STATE_BYPASS].entry[0] & ~PMGR_SOC_PERF_STATE_ENTRY_MCU_MASK) |
			(rPMGR_SOC_PERF_STATE_ENTRY_A(soc_perf_state) & PMGR_SOC_PERF_STATE_ENTRY_MCU_MASK);
		// 7b. Write the reset value to all other fields in ENTRY_0A
		rPMGR_SOC_PERF_STATE_ENTRY_B(kSOC_PERF_STATE_BYPASS) = pmgr_soc_perf_states[kSOC_PERF_STATE_BYPASS].entry[1];
		rPMGR_SOC_PERF_STATE_ENTRY_C(kSOC_PERF_STATE_BYPASS) = pmgr_soc_perf_states[kSOC_PERF_STATE_BYPASS].entry[2];
		rPMGR_SOC_PERF_STATE_ENTRY_D(kSOC_PERF_STATE_BYPASS) = 0;

		// 8. Write the reset value to the SOC_PERF_STATE_CTL register
		rPMGR_SOC_PERF_STATE_CTL = kSOC_PERF_STATE_BYPASS;
		while (rPMGR_SOC_PERF_STATE_CTL & PMGR_SOC_PERF_STATE_CTL_PENDING_MASK);

		// 9. Write the reset values to the SOC_PERF_STATE entry registers
		for (cnt = 0; cnt < PMGR_SOC_PERF_STATE_COUNT; cnt++) {
			if (cnt == kSOC_PERF_STATE_BYPASS)
				continue;
				
			rPMGR_SOC_PERF_STATE_ENTRY_A(cnt) = 0;
			rPMGR_SOC_PERF_STATE_ENTRY_B(cnt) = 0;
			rPMGR_SOC_PERF_STATE_ENTRY_C(cnt) = 0;
			rPMGR_SOC_PERF_STATE_ENTRY_D(cnt) = 0;
		}
	}	

	// 11. Write all PLLx_EXT_BYPASS_CTL.EXT_BYPASS to 1
	for (cnt = 0; cnt < PMGR_PLL_COUNT; cnt++) {
		if (cnt == 1)
			continue; // Mem PLL

		// Make sure the PLL can accept a bypass state change.
		while (rPMGR_PLL_CTL(cnt) & PMGR_PLL_PENDING);

		// Switch to bypass mode.
		rPMGR_PLL_EXT_BYPASS_CTL(cnt) |= PMGR_PLL_EXT_BYPASS;
		while ((rPMGR_PLL_EXT_BYPASS_CTL(cnt) & PMGR_PLL_BYP_ENABLED) == 0);
	}

	// 14. Write reset value to all PLLx_CTL
	for (cnt = 0; cnt < PMGR_PLL_COUNT; cnt++) {
		if (cnt == 1)
			continue; // Mem PLL

		rPMGR_PLL_CTL(cnt) = PMGR_PLL_M(1) | PMGR_PLL_P(1); // fb_div = 1, pre_div = 1
		while (rPMGR_PLL_CTL(cnt) & PMGR_PLL_PENDING);
	}

	// 15. Write reset value to all other PLL registers
	for (cnt = 0; cnt < PMGR_PLL_COUNT; cnt++) {
		if (cnt == 1)
			continue; // Mem PLL

		rPMGR_PLL_CFG(cnt) = PMGR_PLL_OFF_MODE(PMGR_PLL_OFF_MODE_POWER_DOWN) |
			PMGR_PLL_FRAC_LOCK_TIME(0x48) | PMGR_PLL_LOCK_TIME(0x348);
	}

	// 16. Write reset value to spare and ISP_REF0/1
	clock_update_range(PMGR_CLK_NUM(S0), PMGR_CLK_NUM(ISP_REF1), 0x80000001);

	// 17. Put CPU clock back into its reset default.
	set_apsc_ccc_state(kDVFM_STATE_BYPASS);
}


static void power_on_sep(void)
{
	volatile uint32_t *reg = (volatile uint32_t *)((uint64_t *)PMGR_FIRST_PS + CLK_SEP);
	uint32_t val = *reg;

	val &= ~(PMGR_PS_AUTO_PM_EN | PMGR_PS_FORCE_NOACCESS);
	*reg = val;
	while (((*reg >> 4) & 0xF) != 0xF); // Wait for SEP to turn on.

	return;
}

static void set_device(uint64_t *devices, uint64_t device)
{
#if DEBUG_BUILD
	if (device >= 128)
		panic("Invalid device clock index: %llu", device);
#endif
	if (device >= 64)
		devices[1] |= (0x1ULL << (device - 64));
	else
		devices[0] |= (0x1ULL << device);
}

static void clocks_quiesce_internal(void)
{
	uint64_t devices[2] = { 0, 0 };

	// Disable all voltage changes everywhere!
	rPMGR_VOLMAN_CTL |= (PMGR_VOLMAN_DISABLE_CPU_VOL_CHANGE | PMGR_VOLMAN_DISABLE_GFX_VOL_CHANGE |
		PMGR_VOLMAN_DISABLE_FIXED_VOL_CHANGE | PMGR_VOLMAN_DISABLE_VAR_SOC_VOL_CHANGE);

	// We don't touch CPU0, CPU1, CPM but the following devices need to be on.
	set_device(devices, CLK_LIO);
	set_device(devices, CLK_IOMUX);
	set_device(devices, CLK_AIC);
	set_device(devices, CLK_DEBUG);
	set_device(devices, CLK_DWI);
	set_device(devices, CLK_GPIO);
	set_device(devices, CLK_SIO);
	set_device(devices, CLK_SIO_P);
	set_device(devices, CLK_ANS);
	set_device(devices, CLK_MCC);
	set_device(devices, CLK_MCU);
	set_device(devices, CLK_AMP);
	set_device(devices, CLK_USB);
	set_device(devices, CLK_USBCTLREG);
	set_device(devices, CLK_USB_OTG);
	set_device(devices, CLK_SMX);
	set_device(devices, CLK_SF);
	set_device(devices, CLK_CP);
	set_device(devices, CLK_PCIE_AUX);

	// Turn on/off critical device clocks
	clocks_set_gates(devices);

	// Disable performance state table to control PLL5
	rPMGR_GFX_PERF_STATE_CTL = 0;

	restore_clock_config_state();
}

void clocks_quiesce(void)
{
	clocks_quiesce_internal();
}

uint32_t clocks_set_performance(uint32_t performance_level)
{
	uint32_t old_perf_level = perf_level;
	uint32_t ccc_state = get_apsc_ccc_state();

	if (ccc_state != active_state)
		set_apsc_ccc_state(active_state);

#if APPLICATION_IBOOT
	if (performance_level == kPerformanceMemoryLow || performance_level == kPerformanceMemoryFull) {
		
		rPMGR_SOC_PERF_STATE_CTL = (performance_level == kPerformanceMemoryLow) ? kSOC_PERF_STATE_IBOOT_MEM_LOW_PERF : kSOC_PERF_STATE_IBOOT_MEM_FULL_PERF;
		// Wait for Pending bits to clear.
		while (rPMGR_SOC_PERF_STATE_CTL & PMGR_SOC_PERF_STATE_CTL_PENDING_MASK);
		
		perf_level = performance_level;
		
		clocks_get_frequencies_range(PMGR_CLK_MCU_FIXED, PMGR_CLK_MCU);
	}
#endif
	// At this point we should have CCC clock set for this stage of boot.
	return old_perf_level;
}

void clock_get_frequencies(uint32_t *clocks, uint32_t count)
{
	uint32_t cnt = PMGR_CLK_COUNT;

	if (cnt > count)
		cnt = count;

	memcpy(clocks, clks, cnt * sizeof(uint32_t));
}

uint32_t clock_get_frequency(int clock)
{
	uint32_t freq = 0;

	// XXX TODO

	switch (clock) {
	case CLK_NCLK:
	case CLK_FIXED:
	case CLK_TIMEBASE:
		freq = clks[PMGR_CLK_OSC];
		break;
	case CLK_PCLK:
	case CLK_PERIPH:
	case CLK_I2C0:
	case CLK_I2C1:
	case CLK_I2C2:
		freq = clks[PMGR_CLK_SIO_P];
		break;
	case CLK_MEM:
		freq = clks[PMGR_CLK_MCU];
		break;
	case CLK_ANS_LINK:
		freq = clks[PMGR_CLK_ANC_LINK];
		break;
	case CLK_BUS:
		freq = clks[PMGR_CLK_LIO];
		break;
	case CLK_CPU:
		freq = clks[PMGR_CLK_CPU];
		break;
#if SUB_PLATFORM_T7000
	case CLK_MIPI:
		freq = clks[PMGR_CLK_MIPI_DSI];
		break;
#endif
	case CLK_VCLK0:
		freq = clks[PMGR_CLK_VID0];
		break;
	default:
		break;
	}
	return freq;
}

void clock_set_frequency(int clock, uint32_t divider, uint32_t pll_p, uint32_t pll_m, uint32_t pll_s, uint32_t pll_t)
{
	uint32_t clk = PMGR_CLK_OSC;

	switch (clock) {
	case CLK_VCLK0:
		clk = PMGR_CLK_VID0;
		break;
#if SUB_PLATFORM_T7000
	case CLK_MIPI:
		clk = PMGR_CLK_PLL0;
		break;
#endif
	default:
		break;
	}

	if (clk >= PMGR_CLK_PLL0 && clk <= PMGR_CLK_PLL5) {
		int32_t pll = clk - PMGR_CLK_PLL0;

		if (!is_pll_running(pll))
			set_pll(pll, pll_p, pll_m, pll_s, false);
		else
			set_running_pll(pll, pll_p, pll_m, pll_s);

		clks[clk] = get_pll(pll);

#if SUB_PLATFORM_T7000
		if (clock == CLK_MIPI) {
			clocks_get_frequencies_range(PMGR_CLK_MIPI_DSI, PMGR_CLK_MIPI_DSI);
		}
#endif
	}

	if (clk >= PMGR_CLK_SOURCE_FIRST && clk <= PMGR_CLK_SOURCE_LAST) {
		clock_update_frequency(clk, pll_t);
		clocks_get_frequencies_range(clk, clk);
	}

	return;
}

void clock_gate(int device, bool enable)
{
	volatile uint32_t *reg = (volatile uint32_t *)((uint64_t *)PMGR_FIRST_PS + device);

	// Make sure we are within limits.
	if (reg > PMGR_LAST_PS)
		return;

#if APPLICATION_IBOOT && SUPPORT_FPGA
	// Old versions of FPGA Secure ROM set FORCE_NOACCESS bit when powering off a block. Make sure bit is cleared.
	*reg &= ~PMGR_PS_FORCE_NOACCESS;
#endif

	// Set the PS field to the requested level
	if (enable) {
		*reg |= PMGR_PS_RUN_MAX;
	} else {
		*reg &= ~PMGR_PS_RUN_MAX;
	}

	// Wait for the MANUAL_PS and ACTUAL_PS fields to be equal
	while ((*reg & PMGR_PS_MANUAL_PS_MASK) != ((*reg >> PMGR_PS_ACTUAL_PS_SHIFT) & PMGR_PS_ACTUAL_PS_MASK));
}

static void clocks_set_gates(uint64_t *devices)
{
	uint32_t i, idx;

	// For future platforms that clone from this.
	if (PMGR_LAST_DEVICE >= 128)
		panic("Rewrite this to deal with more than 128 clock ids");

	// Make sure SIO_P is on before SIO. <rdar://problem/13038483>
	clock_gate(CLK_SIO_P, true);

	// Turn on devices from lo to hi (to meet dependencies).
	for (idx = 0, i = PMGR_FIRST_DEVICE; i < PMGR_LAST_DEVICE; i++) {
		if (i && ((i % 64) == 0))
			idx++;
		if ((devices[idx] >> ((uint64_t)(i % 64))) & 0x1)
			clock_gate(i, true);
	}

	// Turn off devices hi to lo order (to meet dependencies).
	for (idx = 1, i = PMGR_LAST_DEVICE - 1; i >= PMGR_FIRST_DEVICE; i--) {
		if (!((devices[idx] >> ((uint64_t)(i % 64))) & 0x1))
			clock_gate(i, false);
		if (i && ((i % 64) == 0))
			idx--;
	}

	return;
}

void platform_system_reset(bool panic)
{
#if WITH_BOOT_STAGE
	if (!panic)
		boot_set_stage(kPowerNVRAMiBootStageOff);
#endif

	wdt_system_reset();

	while (1);
}

void platform_reset(bool panic)
{
#if WITH_BOOT_STAGE
	if (!panic)
		boot_set_stage(kPowerNVRAMiBootStageOff);
#endif

	wdt_chip_reset();

	while (1);
}

void platform_power_init(void)
{
	/* Disable Power gating for CPU0, this is needed to avoid WFI from powering down the core */
	/* Remove this once following is addressed: <rdar://problem/10869952> Core 0 power gating enabled by default */
	rPMGR_PWRGATE_CPU0_CFG0 &= ~(1UL << 31);

#if (APPLICATION_IBOOT && !PRODUCT_IBOOT && !PRODUCT_IBEC)

	// Initialize temperature sensors, thermal failsafe blocks, and TVM tunables
	init_soc_thermal_sensors();
	init_cpu_thermal_sensors();
	init_soc_sochot();
	init_cpu_sochot();
	init_soc_tvm_tunables();
#endif
}

void platform_watchdog_tickle(void)
{
	// Varies by target. This layer between is necessary so that
	// we don't go straight from generic code to target.
	target_watchdog_tickle();
}

void clock_reset_device(int device)
{
	volatile uint32_t *reg = (volatile uint32_t *)((uint64_t *)PMGR_FIRST_PS + device);
	uint32_t lpdp_override = 4;
	bool lpdp_fuse_ow_enable = false;

	// Make sure we are within limits.
	if (device >= PMGR_LAST_DEVICE)
		return;

	// XXX TODO
	switch (device) {
	case CLK_MCU:
	case CLK_ANS:
	case CLK_SIO:
	case CLK_EDPLINK:
#if SUB_PLATFORM_T7000
	case CLK_MIPI:
#endif
		*reg |= PMGR_PS_FORCE_NOACCESS;
		spin(1);
		*reg |= PMGR_PS_RESET;
		spin(1);
		*reg &= ~(PMGR_PS_RESET | PMGR_PS_FORCE_NOACCESS);

		if (device == CLK_EDPLINK) {
			//Begin : <rdar://problem/14956874> SecureROM PLL calibration sequence changes

			lpdp_override = rCFG_FUSE9;
			lpdp_override >>= CFG_FUSE9_PLL_LPDP_SHIFT;
			lpdp_fuse_ow_enable = lpdp_override & CFG_FUSE9_PLL_VCO_RCTRL_SEL;

			// Is the override data for this PLL valid?
			if (lpdp_fuse_ow_enable) {
				#define	rLPDP_PLL_CLK	(*(volatile u_int32_t *)(LPDP_PHY_BASE_ADDR + 0x00084))
				#define	LPDP_PLL_CLK_VCO_RCTRL_OW_SHIFT		2
				#define	LPDP_PLL_CLK_VCO_RCTRL_OW_MASK		0x7
				#define	LPDP_PLL_CLK_VCO_RCTRL_SEL_ENABLE	0x1
				#define	LPDP_PLL_CLK_VCO_RCTRL_SEL_SHIFT	0x5
				uint32_t reg = rLPDP_PLL_CLK;
				reg &= ~(LPDP_PLL_CLK_VCO_RCTRL_OW_MASK << LPDP_PLL_CLK_VCO_RCTRL_OW_SHIFT);
				reg |= (lpdp_override & LPDP_PLL_CLK_VCO_RCTRL_OW_MASK) << LPDP_PLL_CLK_VCO_RCTRL_OW_SHIFT;
				reg |= (LPDP_PLL_CLK_VCO_RCTRL_SEL_ENABLE << LPDP_PLL_CLK_VCO_RCTRL_SEL_SHIFT);
				rLPDP_PLL_CLK = reg;
			}
			// End : <rdar://problem/14956874> SecureROM PLL calibration sequence changes
		}
		break;

	default:
		break;
	}
}

void clock_set_device_reset(int device, bool set)
{
	volatile uint32_t *reg = (volatile uint32_t *)((uint64_t *)PMGR_FIRST_PS + device);

	switch (device) {
	case CLK_PCIE:
		if (set) {
			*reg |= PMGR_PS_FORCE_NOACCESS;
			spin(1);
			*reg |= PMGR_PS_RESET;
		}
		else {
			*reg &= ~(PMGR_PS_RESET | PMGR_PS_FORCE_NOACCESS);
		}
	}
}

bool clock_get_pcie_refclk_good(void)
{
#if SUB_PLATFORM_T7000
	return true;
#elif SUB_PLATFORM_T7001
	return (rPMGR_DEBUG_PMGR_DEBUG18 & (1 << 18)) != 0;
#endif
}

#if APPLICATION_IBOOT
static void set_gfx_perf_state(uint32_t state_num, struct gfx_state_info *gfx_state)
{
	uint32_t pll_enable = 0;

	// This is deductive. If feedback divider is 0 the PLL shouldn't output anything.
	pll_enable = gfx_state->fb_div ? 1 : 0;

	rPMGR_GFX_PERF_STATE_ENTRY(state_num) = ((gfx_state->dwi_val & 0xFF) << 24) |
	((pll_enable & 0x1) << 21) |
	((gfx_state->fb_div & 0x1FF) << 12) |
	((gfx_state->pre_div & 0x1F) << 4) |
	(gfx_state->op_div & 0xF);
#if SUB_PLATFORM_T7001
	rPMGR_GFX_SRAM_PERF_STATE_ENTRY(state_num) = gfx_state->dwi_sram_val & 0xff;
#endif
}
#endif

#if WITH_DEVICETREE
static uint64_t get_freq_from_ccc_state(uint64_t state_entry)
{
	// Fcpu <= ((OSC * M) / P / S+1)
	uint64_t freq = OSC_FREQ;
	freq *= ((state_entry >> 4) & 0x1FF);
	freq /= ((state_entry >> 13) & 0x1F) ? ((state_entry >> 13) & 0x1F) : 1;
	freq /= (1 + ((state_entry >> 0) & 0xF));

	return freq;
}

void pmgr_update_device_tree(DTNode *pmgr_node)
{
	uint32_t num_freqs = 0;
	uint32_t propSize;
	uint64_t period_ns;
	uint64_t freq = 0;
	uint32_t volt;
	
	char *propName;
	void *propData;
	
	struct dvfm_data dvfm = {
		.dvfm_state_vmax = kDVFM_STATE_VMAX,
		.dvfm_state_iboot_cnt = kDVFM_STATE_IBOOT_CNT,
		.dvfm_state_vnom = kDVFM_STATE_VNOM,
		.dvfm_state_vboost = kDVFM_STATE_VBOOST,
		.voltage_states1_count = kVOLTAGE_STATES1_COUNT,
	};

#if SUB_PLATFORM_T7001
	pmgr_get_dvfm_data(&dvfm);
#endif

	// Populate the devicetree with relevant values.
	propName = "nominal-performance1";
	if (FindProperty(pmgr_node, &propName, &propData, &propSize)) {
		if (propSize != sizeof(uint32_t))
			panic("pmgr property nominal-performance1 is of wrong size.");

		freq = get_freq_from_ccc_state(rCCC_DVFM_ST(dvfm.dvfm_state_vnom));
		if (freq == 0)
			panic("pmgr Fnom Operating point not defined correctly");

		period_ns = 1000000000ULL << 16;
		period_ns /= freq;

		((uint32_t *)propData)[0] = period_ns;
	} else {
		panic("pmgr property nominal-performance1 not found.");
	}

	propName = "boost-performance1";
	// Note: Boost is not mandatory in all platforms.
	if (FindProperty(pmgr_node, &propName, &propData, &propSize)) {
		if (propSize != sizeof(uint32_t))
			panic("pmgr property boost-performance1 is of wrong size");

		freq = get_freq_from_ccc_state(rCCC_DVFM_ST(dvfm.dvfm_state_vboost));
		if (freq == 0)
			panic("pmgr Fboost Operating point not defined correctly");
		period_ns = 1000000000ULL << 16;
		period_ns /= freq;
		((uint32_t *)propData)[0] = period_ns;
	}

	propName = "voltage-states1";
	if (FindProperty(pmgr_node, &propName, &propData, &propSize)) {
		if (propSize / sizeof(uint32_t) < dvfm.voltage_states1_size) {
			panic("pmgr number of states less than required for voltage-states1");
		}

		num_freqs = dvfm.voltage_states1_count;
		for (int32_t i = num_freqs - 1, j = 0; i >= 0; i--, j++) {
			// Getting voltage and frequencies directly from rCCC_DVFM_ST allows
			// including all the changes done before launching the kernel:
			// - voltage adjusting coming from converting voltage to PMU value,
			// - voltage knobs from LLB,
			// - astris changes done while beeing in iBoot console,
			freq = get_freq_from_ccc_state(rCCC_DVFM_ST(dvfm.dvfm_state_vmax - j));
			volt = platform_get_dwi_to_mv(BUCK_CPU, CCC_DVFM_ST_TO_SAFEVOL(rCCC_DVFM_ST(dvfm.dvfm_state_vmax - j)));
			if (freq != 0) {
				period_ns = 1000000000ULL << 16;
				period_ns /= freq;
				((uint32_t *)propData)[2*i] = period_ns;
				((uint32_t *)propData)[2*i+1] = volt;
			}
		}
	}

	propName = "total-rails-leakage";
	if (FindProperty(pmgr_node, &propName, &propData, &propSize)) {
		if (propSize >= sizeof(uint32_t)) {
			*(uint32_t *)propData = chipid_get_total_rails_leakage();
		}
	}

	return;
}

void pmgr_gfx_update_device_tree(DTNode *gfx_node)
{
	u_int32_t	count, propSize, num_states = 0, state_val;
	char		*propName;
	void		*propData;

	propName = "perf-states";
	if (FindProperty(gfx_node, &propName, &propData, &propSize)) {
		if (propSize != (2 * sizeof(u_int32_t) * kPMGR_GFX_STATE_MAX)) {
			panic("gfx property perf-state has wrong size");
		}
		// Read the values programmed into the GFX perf state table
		// and populate the device tree.
		for (count = 0; count < kPMGR_GFX_STATE_MAX; count++) {
			state_val = rPMGR_GFX_PERF_STATE_ENTRY(count);

			// Any but the first entry with a value of 0 marks the end of the number of valid states.
			if ((count != 0) && (state_val == rPMGR_GFX_PERF_STATE_ENTRY(CHIPID_GPU_VOLTAGE_OFF))) {
				num_states = count;
				break;
			}

			((u_int32_t *)propData)[count * 2 + 0] = PMGR_PLL_FREQ((state_val >> 12) & 0x1FF, (state_val >> 4) & 0x1F, state_val & 0xF);
			((u_int32_t *)propData)[count * 2 + 1] = platform_get_dwi_to_mv(BUCK_GPU, (state_val >> 24) & 0xFF);
		}

		// If all the entries are valid.
		if (count == kPMGR_GFX_STATE_MAX) {
			num_states = kPMGR_GFX_STATE_MAX;
		}

	}

	propName = "perf-state-count";
	if (FindProperty(gfx_node, &propName, &propData, &propSize)) {
		*(u_int32_t *)propData = kPMGR_GFX_STATE_MAX; // This is the value given before <rdar://problem/16814218>
	}

	propName = "gpu-num-perf-states";
	/* We don't count DIDT states and OFF state */
	if (FindProperty(gfx_node, &propName, &propData, &propSize)) {
		*(u_int32_t *)propData = num_states / 2;
	}
	return;
}

void sochot_pmgr_update_device_tree(DTNode *node)
{
	uint32_t	propSize;
	char		*propName;
	void		*propData;

	propName = "target-dvfm-state-0";
	if (FindProperty(node, &propName, &propData, &propSize)) {
		if (propSize >= sizeof(uint32_t)) {
			*(uint32_t *)propData = CCC_DFVM_FSHOT_IDX_THERMAL0_STATE;
		}
	}

	propName = "target-dvfm-state-1";
	if (FindProperty(node, &propName, &propData, &propSize)) {
		if (propSize >= sizeof(uint32_t)) {
			*(uint32_t *)propData = CCC_DFVM_FSHOT_IDX_THERMAL1_STATE;
		}
	}
}
#endif

#if (APPLICATION_IBOOT && !PRODUCT_IBOOT && !PRODUCT_IBEC)

static void init_soc_thermal_sensors(void)
{
	// Keep sampling rate at default 1 kHz (pending <rdar://problem/14785339> Tune temperature sensor sampling rates for Fiji/Capri)
	
	// Define piecewise-linear relationship (start_codes, offsets, slopes) between output and temperature for SOC/PMGR sensors
	rPMGR_THERMAL0_PIECE0 = PMGR_THERMAL_PIECE(0x000, 0x1d4, 0x036);
	rPMGR_THERMAL0_PIECE1 = PMGR_THERMAL_PIECE(0x1aa, 0x1e3, 0x02d);
	rPMGR_THERMAL0_PIECE2 = PMGR_THERMAL_PIECE(0x270, 0x1f9, 0x024);

	rPMGR_THERMAL1_PIECE0 = PMGR_THERMAL_PIECE(0x000, 0x1d4, 0x036);
	rPMGR_THERMAL1_PIECE1 = PMGR_THERMAL_PIECE(0x1aa, 0x1e3, 0x02d);
	rPMGR_THERMAL1_PIECE2 = PMGR_THERMAL_PIECE(0x270, 0x1f9, 0x024);

	// Read temperature trim values from efuse and write to corresponding registers (CPU sensors handle this step in hardware)
	rPMGR_THERMAL0_PARAM = PMGR_THERMAL_PARAM_SET_TRIM(rPMGR_THERMAL0_PARAM, chipid_get_soc_temp_sensor_trim(0));
	rPMGR_THERMAL1_PARAM = PMGR_THERMAL_PARAM_SET_TRIM(rPMGR_THERMAL1_PARAM, chipid_get_soc_temp_sensor_trim(1));

	rPMGR_THERMAL0_CTL1 = 0xea60;
	rPMGR_THERMAL1_CTL1 = 0xea60;

	// Enable temperature sensors (need to do this before enabling either DVTM or thermal failsafe)
	rPMGR_THERMAL0_CTL0_SET = PMGR_THERMAL_CTL0_ENABLE;
	rPMGR_THERMAL1_CTL0_SET = PMGR_THERMAL_CTL0_ENABLE;
}

static void init_soc_sochot(void)
{
	// Set target state for when SOCHOT triggers
	rPMGR_GFX_PERF_STATE_SOCHOT = PMGR_GFX_PERF_STATE_SOCHOT_ENABLE_TRIG0 | PMGR_GFX_PERF_STATE_SOCHOT_TRIG0_SELECT(1);
	
	// SOC SOCHOT not supported due to certain media use cases having hard clock requirements
}

static void init_cpu_thermal_sensors(void)
{
	// Keep sampling rate at default 1 kHz (pending <rdar://problem/14785339> Tune temperature sensor sampling rates for Fiji/Capri)

	// Define piecewise-linear relationship (start_codes, offsets, slopes) between output and temperature for CPU sensors
	rCCC_THRM0_PIECE0 = CCC_THRM_PIECE(0x000, 0x0d4, 0x036);
	rCCC_THRM0_PIECE1 = CCC_THRM_PIECE(0x1aa, 0x0e3, 0x02d);
	rCCC_THRM0_PIECE2 = CCC_THRM_PIECE(0x270, 0x0f9, 0x024);

	rCCC_THRM1_PIECE0 = CCC_THRM_PIECE(0x000, 0x0d4, 0x036);
	rCCC_THRM1_PIECE1 = CCC_THRM_PIECE(0x1aa, 0x0e3, 0x02d);
	rCCC_THRM1_PIECE2 = CCC_THRM_PIECE(0x270, 0x0f9, 0x024);

	rCCC_THRM2_PIECE0 = CCC_THRM_PIECE(0x000, 0x0d4, 0x036);
	rCCC_THRM2_PIECE1 = CCC_THRM_PIECE(0x1aa, 0x0e3, 0x02d);
	rCCC_THRM2_PIECE2 = CCC_THRM_PIECE(0x270, 0x0f9, 0x024);

#if SUB_PLATFORM_T7001
	rCCC_THRM3_PIECE0 = CCC_THRM_PIECE(0x000, 0x0d4, 0x036);
	rCCC_THRM3_PIECE1 = CCC_THRM_PIECE(0x1aa, 0x0e3, 0x02d);
	rCCC_THRM3_PIECE2 = CCC_THRM_PIECE(0x270, 0x0f9, 0x024);
#endif

	// Per <rdar://problem/14797455>, although the CPU sensors handle the step of reading temperature trim from efuse and writing 
	// this to THRMn_PARAM in hardware, the value doesn't actually take effect until an explicit register write is done -- so the 
	// workaround for this errata is to read out and then write back the contents of each CCC_THRMn_PARAM register, unchanged.
	rCCC_THRM0_PARAM = rCCC_THRM0_PARAM;
	rCCC_THRM1_PARAM = rCCC_THRM1_PARAM;
	rCCC_THRM2_PARAM = rCCC_THRM2_PARAM;
#if SUB_PLATFORM_T7001
	rCCC_THRM3_PARAM = rCCC_THRM3_PARAM;
#endif

	rCCC_THRM0_CTL1 = 0xea60;
	rCCC_THRM1_CTL1 = 0xea60;
	rCCC_THRM2_CTL1 = 0xea60;
#if SUB_PLATFORM_T7001
	rCCC_THRM3_CTL1 = 0xea60;
#endif

	// Enable temperature sensors (need to do this before enabling either DVTM or thermal failsafe)
	rCCC_THRM0_CTL0_SET = CCC_THRM_CTL0_ENABLE;
	rCCC_THRM1_CTL0_SET = CCC_THRM_CTL0_ENABLE;
	rCCC_THRM2_CTL0_SET = CCC_THRM_CTL0_ENABLE;
#if SUB_PLATFORM_T7001
	rCCC_THRM3_CTL0_SET = CCC_THRM_CTL0_ENABLE;
#endif
}

static void init_cpu_sochot(void)
{
	// Set target state for when SOCHOT triggers
	rCCC_DVFM_FSHOT_IDX = CCC_DFVM_FSHOT_IDX_THERMAL0(kDVFM_STATE_V0);
}

static void init_soc_tvm_tunables(void)
{
	rPMGR_TVM_THRESH0 = PMGR_TVM_THRESH(0x00);
	rPMGR_TVM_THRESH1 = PMGR_TVM_THRESH(0x19);
	rPMGR_TVM_THRESH2 = PMGR_TVM_THRESH(0x32);	
	rPMGR_TVM_TEMP0_CFG = PMGR_TVM_TEMP_CFG_MAX_OFFSET(0x5) | PMGR_TVM_TEMP_CFG_MIN_OFFSET(0x1F6);
	rPMGR_TVM_TEMP1_CFG = PMGR_TVM_TEMP_CFG_MAX_OFFSET(0x5) | PMGR_TVM_TEMP_CFG_MIN_OFFSET(0x1FB);
}

#endif
