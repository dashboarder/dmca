/*
 * Copyright (C) 2011-2014 Apple Inc. All rights reserved.
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

extern void arm_no_wfe_spin(uint32_t usecs);

struct clock_source {
	uint32_t	src_clk;
	uint32_t	factor;
};

struct clock_config {
    volatile uint32_t	*clock_reg;	  // CLK_CFG Register
    struct clock_source sources[12];	// List of sources
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
	[kDVFM_STATE_BYPASS] = (2ULL << 18) | (1ULL << 22),

	// CCC @ 300 MHz, disable voltage change sequence, Clk src = PLL, BIU div = 1.
	[kDVFM_STATE_SECUREROM] = CCC_DVFM_ST_PLL_P(1) | CCC_DVFM_ST_PLL_M(50) | 
					CCC_DVFM_ST_PLL_S(3) | (1ULL << 22) | 
					(1ULL << 21) | (2ULL << 18),
#else
	// BIU div = 1, Clk src = Ref clk.
	[kDVFM_STATE_BYPASS] =  (2ULL << 18),
#endif

#if APPLICATION_IBOOT

#if SUB_TARGET_N51 || SUB_TARGET_N53 || SUB_TARGET_J85 || SUB_TARGET_J86 || SUB_TARGET_J87 || SUB_TARGET_J85M || SUB_TARGET_J86M || SUB_TARGET_J87M
	// CCC V0
	// CCC @ 396 MHz, enable voltage change sequence, Clk src = PLL, BIU div = 1.
	[kDVFM_STATE_IBOOT] = CCC_DVFM_ST_PLL_P(1) | CCC_DVFM_ST_PLL_M(33) | 
				CCC_DVFM_ST_PLL_S(1) | (1ULL << 21) | (2ULL << 18)  |
				CCC_DVFM_ST_VOLADJ0(0x0) | CCC_DVFM_ST_VOLADJ1(0x6) | CCC_DVFM_ST_VOLADJ2(0xB),

	// CCC V1
	// CCC @ 600 MHz, enable voltage change sequence, Clk src = PLL, BIU div = 1.
	[kDVFM_STATE_V1] = CCC_DVFM_ST_PLL_P(1) | CCC_DVFM_ST_PLL_M(25) | 
				CCC_DVFM_ST_PLL_S(0) | (1ULL << 21) | (2ULL << 18)  |
				CCC_DVFM_ST_VOLADJ0(0x0) | CCC_DVFM_ST_VOLADJ1(0x6) | CCC_DVFM_ST_VOLADJ2(0xB),

	// CCC V2
	// CCC @ 840 MHz, enable voltage change sequence, Clk src = PLL, BIU div = 2.
	[kDVFM_STATE_V2] = CCC_DVFM_ST_PLL_P(1) | CCC_DVFM_ST_PLL_M(35) | 
				CCC_DVFM_ST_PLL_S(0) | (1ULL << 21) | (4ULL << 18)  |
				CCC_DVFM_ST_VOLADJ0(0x0) | CCC_DVFM_ST_VOLADJ1(0x4) | CCC_DVFM_ST_VOLADJ2(0x8),

	// CCC V3
	// CCC @ 1128 MHz, enable voltage change sequence, Clk src = PLL, BIU div = 2
	[kDVFM_STATE_V3] = CCC_DVFM_ST_PLL_P(1) | CCC_DVFM_ST_PLL_M(47) |
				CCC_DVFM_ST_PLL_S(0) | (1ULL << 21) | (4ULL << 18)  |
				CCC_DVFM_ST_VOLADJ0(0x0) | CCC_DVFM_ST_VOLADJ1(0x4) | CCC_DVFM_ST_VOLADJ2(0x8),

	// CCC V4
	// CCC @ 1296 MHz, enable voltage change sequence, Clk src = PLL, BIU div = 2.5
	[kDVFM_STATE_V4] = CCC_DVFM_ST_PLL_P(1) | CCC_DVFM_ST_PLL_M(54) |
				CCC_DVFM_ST_PLL_S(0) | (1ULL << 21) | (5ULL << 18)  |
				CCC_DVFM_ST_VOLADJ0(0x0) | CCC_DVFM_ST_VOLADJ1(0x0) | CCC_DVFM_ST_VOLADJ2(0x0)

#elif SUB_TARGET_J34 || SUB_TARGET_J34M || SUB_TARGET_J71 || SUB_TARGET_J72 || SUB_TARGET_J73
	// CCC V0
	// CCC @ 600 MHz, enable voltage change sequence, Clk src = PLL, BIU div = 1.
	[kDVFM_STATE_IBOOT] = CCC_DVFM_ST_PLL_P(1) | CCC_DVFM_ST_PLL_M(25) | 
				CCC_DVFM_ST_PLL_S(0) | (1ULL << 21) | (2ULL << 18)  |
				CCC_DVFM_ST_VOLADJ0(0x0) | CCC_DVFM_ST_VOLADJ1(0x6) | CCC_DVFM_ST_VOLADJ2(0xB),

	// CCC V1
	// CCC @ 840 MHz, enable voltage change sequence, Clk src = PLL, BIU div = 2.
	[kDVFM_STATE_V1] = CCC_DVFM_ST_PLL_P(1) | CCC_DVFM_ST_PLL_M(35) | 
				CCC_DVFM_ST_PLL_S(0) | (1ULL << 21) | (4ULL << 18)  |
				CCC_DVFM_ST_VOLADJ0(0x0) | CCC_DVFM_ST_VOLADJ1(0x4) | CCC_DVFM_ST_VOLADJ2(0x8),

	// CCC V2
	// CCC @ 1128 MHz, enable voltage change sequence, Clk src = PLL, BIU div = 2
	[kDVFM_STATE_V2] = CCC_DVFM_ST_PLL_P(1) | CCC_DVFM_ST_PLL_M(47) |
				CCC_DVFM_ST_PLL_S(0) | (1ULL << 21) | (4ULL << 18)  |
				CCC_DVFM_ST_VOLADJ0(0x0) | CCC_DVFM_ST_VOLADJ1(0x4) | CCC_DVFM_ST_VOLADJ2(0x8),

	// CCC V3
	// CCC @ 1296 MHz, enable voltage change sequence, Clk src = PLL, BIU div = 2.5
	[kDVFM_STATE_V3] = CCC_DVFM_ST_PLL_P(1) | CCC_DVFM_ST_PLL_M(54) |
				CCC_DVFM_ST_PLL_S(0) | (1ULL << 21) | (5ULL << 18)  |
				CCC_DVFM_ST_VOLADJ0(0x0) | CCC_DVFM_ST_VOLADJ1(0x0) | CCC_DVFM_ST_VOLADJ2(0x0),

	// CCC V4
	// CCC @ 1392 MHz, enable voltage change sequence, Clk src = PLL, BIU div = 3
	[kDVFM_STATE_V4] = CCC_DVFM_ST_PLL_P(1) | CCC_DVFM_ST_PLL_M(58) |
				CCC_DVFM_ST_PLL_S(0) | (1ULL << 21) | (6ULL << 18)  |
				CCC_DVFM_ST_VOLADJ0(0x0) | CCC_DVFM_ST_VOLADJ1(0x0) | CCC_DVFM_ST_VOLADJ2(0x0)

#else
	// CCC V0
	// CCC @ 600 MHz, enable voltage change sequence, Clk src = PLL, BIU div = 1.
	[kDVFM_STATE_IBOOT] = CCC_DVFM_ST_PLL_P(1) | CCC_DVFM_ST_PLL_M(25) | 
				CCC_DVFM_ST_PLL_S(0) | (1ULL << 21) | (2ULL << 18)  |
				CCC_DVFM_ST_VOLADJ0(0x0) | CCC_DVFM_ST_VOLADJ1(0x6) | CCC_DVFM_ST_VOLADJ2(0xB),

	// CCC V1
	// CCC @ 672 MHz, enable voltage change sequence, Clk src = PLL, BIU div = 2.
	[kDVFM_STATE_V1] = CCC_DVFM_ST_PLL_P(1) | CCC_DVFM_ST_PLL_M(28) | 
				CCC_DVFM_ST_PLL_S(0) | (1ULL << 21) | (4ULL << 18)  |
				CCC_DVFM_ST_VOLADJ0(0x0) | CCC_DVFM_ST_VOLADJ1(0x6) | CCC_DVFM_ST_VOLADJ2(0xB),

	// CCC V2
	// CCC @ 696 MHz, enable voltage change sequence, Clk src = PLL, BIU div = 2
	[kDVFM_STATE_V2] = CCC_DVFM_ST_PLL_P(1) | CCC_DVFM_ST_PLL_M(29) |
				CCC_DVFM_ST_PLL_S(0) | (1ULL << 21) | (4ULL << 18)  |
				CCC_DVFM_ST_VOLADJ0(0x0) | CCC_DVFM_ST_VOLADJ1(0x4) | CCC_DVFM_ST_VOLADJ2(0x8),

	// CCC V3
	// CCC @ 840 MHz, enable voltage change sequence, Clk src = PLL, BIU div = 2
	[kDVFM_STATE_V3] = CCC_DVFM_ST_PLL_P(1) | CCC_DVFM_ST_PLL_M(35) |
				CCC_DVFM_ST_PLL_S(0) | (1ULL << 21) | (4ULL << 18)  |
				CCC_DVFM_ST_VOLADJ0(0x0) | CCC_DVFM_ST_VOLADJ1(0x4) | CCC_DVFM_ST_VOLADJ2(0x8)
#endif

#endif
};


#if APPLICATION_IBOOT

/* PLL0 @840MHz */
#define PLL0	    0
#define PLL0_O	    OSC_FREQ
#define PLL0_P	    4
#define PLL0_M	    279
#define PLL0_S	    1
#define PLL0_V	    PLL_VCO_TARGET(PLL0)
#define PLL0_T	    PLL_FREQ_TARGET(PLL0)

#if TARGET_DDR_784M
#define PLL1        1
#define PLL1_O      OSC_FREQ
#define PLL1_P      3
#define PLL1_M      98
#define PLL1_S      0
#define PLL1_V      PLL_VCO_TARGET(PLL1)
#define PLL1_T      PLL_FREQ_TARGET(PLL1)

#elif TARGET_DDR_740M

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

#if SUB_TARGET_J34 || SUB_TARGET_J34M
/* PLL2 @297MHz */
#define PLL2	    2
#define PLL2_O	    OSC_FREQ
#define PLL2_P	    2
#define PLL2_M	    99
#define PLL2_S	    15
#define PLL2_V	    PLL_VCO_TARGET(PLL2)
#define PLL2_T	    PLL_FREQ_TARGET(PLL2)
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
#define TARGET_SPARE0_CLK_CFG 0x00100000
#endif

#define VID0_CLKD	TARGET_VID0_CLK_CFG

static const uint32_t clk_divs_active[PMGR_CLK_CFG_COUNT] = {
    0x81100000, 0x81100000, 0x80100000, 0x86100000,	// 0x10000: mcu_fixed, mcu, mipi_dsi, nco_ref0,
    0x85100000, 0x80100000, 0x88100000, 0x85100000,	// 0x10010: nco_ref1, nco_alg0, nco_alg1, hsciphy_ref_12m,
    0x85100000, 0x85100000, 0x85100000, 0x85100000,	// 0x10020: usb480_0, usb480_1, usb_ohci_48m, usb,
    0x85100000, 0x00100000, 0x00100000, 0x00100000,	// 0x10030: usb_free_60m, adsp_t, adsp_p, adsp_ts,
    0x85100000, 0x80100000, 0x85100000, 0x85100000,	// 0x10040: sio_c, sio_p, isp_c, isp,
    0x81100000, 0x82100000, 0x85100000, 0x85100000,	// 0x10050: isp_sensor0_ref, isp_sensor1_ref, vdec, venc,
    VID0_CLKD,  0x85100000, 0x85100000, 0x85100000,	// 0x10060: vid0, disp0, disp1, ajpeg_ip,
    0x85100000, 0x85100000, 0x85100000, 0x00100000,	// 0x10070: ajpeg_wrap, msr, af, ans_dll,
    0x8A100000, 0x85100000, 0x85100000, 0x85100000,	// 0x10080: ans_c, ans_link, lio, mca0_m,
    0x86100000, 0x87100000, 0x88100000, 0x89100000,	// 0x10090: mca1_m, mca2_m, mca3_m, mca4_m,
    0x85100000, 0x86100000, 0x85100000, 0x85100000,	// 0x100a0: sep, gpio, spi0_n, spi1_n,
    0x85100000, 0x85100000, 0x85100000			// 0x100b0: spi2_n, spi3_n, debug
};

static const uint32_t spare_divs_active[PMGR_SPARE_CLK_CFG_COUNT] = {
	TARGET_SPARE0_CLK_CFG,		// 0x10200: spare0
	0x00000000,			// 0x10204: spare1
	0x00000000,			// 0x10208: spare2
	0x00000000,			// 0x1020c: spare3
	0x80100028,			// 0x10210: isp_ref0
	0x80100028			// 0x10214: isp_ref1
};

/* GFX table. The voltage information is maintained in chipid.c */
struct gfx_state_info {
	uint32_t dwi_val; /* will be populated by determining the binning info */
	uint32_t fb_div;
	uint32_t pre_div;
	uint32_t op_div;
};

static struct gfx_state_info gfx_states[] = {
	[0] = {0, 0, 0, 0},		/* Power off Rogue state.*/
	[1] = {0, 55, 2, 1},	/* 330 MHz */
	[2] = {0, 65, 2, 1},	/* 390 MHz */
	[3] = {0, 75, 2, 1},	/* 450 MHz */
	[4] = {0, 55, 2, 3},	/* 165 MHz */
	[5] = {0, 65, 2, 3},	/* 195 MHz */
	[6] = {0, 75, 2, 3},	/* 225 MHz */
#if 0
	[7] = {0, 0, 0, 0},		/* Debug state. chipid.c has a valid voltage for this state. */
#endif
};

static void set_gfx_perf_state(uint32_t state_num, struct gfx_state_info *gfx_state);
static int get_amc_cfg_sel(uint32_t performance_level);
static void set_amc_clk_config(uint32_t src_index, uint32_t cfg_sel);
#endif /* APPLICATION_IBOOT */

#if APPLICATION_SECUREROM

/* We need SoC PLL only. */

/* PLL4 @1200MHz (VCO output) */
#define PLL4	    4
#define PLL4_O	    OSC_FREQ
#define PLL4_P	    1
#define PLL4_M	    50
#define PLL4_S	    0
#define PLL4_V	    PLL_VCO_TARGET(PLL4)
#define PLL4_T	    PLL_FREQ_TARGET(PLL4)
#define	PLL4_VCO_ENABLED	0


// We won't touch the clk gen's that aren't necessary during SecureROM.
static const uint32_t clk_divs_active[PMGR_CLK_CFG_COUNT] = {
    0x80100000, 0x80100000, 0x80100000, 0x80100000,	// 0x10000: mcu_fixed, mcu, mipi_dsi, nco_ref0,
    0x80100000, 0x80100000, 0x80100000, 0x80100000,	// 0x10010: nco_ref1, nco_alg0, nco_alg1, hsciphy_ref_12m,
    0x80100000, 0x80100000, 0x80100000, 0x86100000,	// 0x10020: usb480_0, usb480_1, usb_ohci_48m, usb,
    0x80100000, 0x80100000, 0x80100000, 0x80100000,	// 0x10030: usb_free_60m, adsp_t, adsp_p, adsp_ts,
    0x85100000, 0x85100000, 0x80100000, 0x80100000,	// 0x10040: sio_c, sio_p, isp_c, isp,
    0x80100000, 0x80100000, 0x80100000, 0x80100000,	// 0x10050: isp_sensor0_ref, isp_sensor1_ref, vdec, venc,
    0x80100000, 0x80100000, 0x80100000, 0x80100000,	// 0x10060: vid0, disp0, disp1, ajpeg_ip,
    0x80100000, 0x80100000, 0x85100000, 0x80100000,	// 0x10070: ajpeg_wrap, msr, af, ans_dll,
    0x85100000, 0x85100000, 0x85100000, 0x80100000,	// 0x10080: ans_c, ans_link, lio, mca0_m,
    0x80100000, 0x80100000, 0x80100000, 0x80100000,	// 0x10090: mca1_m, mca2_m, mca3_m, mca4_m,
    0x85100000, 0x86100000, 0x80100000, 0x80100000,	// 0x100a0: sep, gpio, spi0_n, spi1_n,
    0x80100000, 0x80100000, 0x80100000		// 0x100b0: spi2_n, spi3_n, debug
};

static const uint32_t spare_divs_active[PMGR_SPARE_CLK_CFG_COUNT] = {
    0x80000001, 0x80000001, 0x80000001, 0x80000001,	    // 0x10200: s0, s1, s2, s3,
    0x80000001, 0x80000001		    // 0x10210: isp_ref0, isp_ref1
};

#endif /* APPLICATION_SECUREROM */


static const struct clock_config clk_configs[PMGR_CLK_COUNT] = {
    [PMGR_CLK_MCU_FIXED]    =	{ 
				   &rPMGR_MCU_FIXED_CLK_CFG, 
				  { 
					{PMGR_CLK_OSC,	1}, 
					{PMGR_CLK_PLL1, 1}
				  }
				},
					
    [PMGR_CLK_MCU]	    =	{ 
				    &rPMGR_MCU_CLK_CFG,
				    {
					{PMGR_CLK_OSC, 1},
					{PMGR_CLK_PLL1, 1},
					{PMGR_CLK_PLL1, 2},
					{PMGR_CLK_PLL1, 4},
					{PMGR_CLK_PLL1, 8},
					{PMGR_CLK_PLL1, 16},
					{PMGR_CLK_PLL4, 48}
				    }
				},
			    
    [PMGR_CLK_MIPI_DSI]	    =	{
				    &rPMGR_MIPI_DSI_CLK_CFG,
				    {
					{PMGR_CLK_PLL0, 1},
					{PMGR_CLK_S0, 1},
					{PMGR_CLK_S1, 1},
					{PMGR_CLK_S2, 1},
					{PMGR_CLK_S3, 1},
					{PMGR_CLK_PLL4, 7},
					{PMGR_CLK_PLL2, 1}
				    }
				},
			    
    [PMGR_CLK_NCO_REF0]	    =	{ 
				    &rPMGR_NCO_REF0_CLK_CFG,
				    {
					{PMGR_CLK_OSC, 1},
					{PMGR_CLK_S0, 1},
					{PMGR_CLK_S1, 1},
					{PMGR_CLK_S2, 1},
					{PMGR_CLK_S3, 1},
					{PMGR_CLK_PLL4, 2},
					{PMGR_CLK_PLL4, 7}
				    }
				},
			    
    [PMGR_CLK_NCO_REF1]	    =	{ 
				    &rPMGR_NCO_REF1_CLK_CFG,
				    {
					{PMGR_CLK_OSC, 1},
					{PMGR_CLK_S0, 1},
					{PMGR_CLK_S1, 1},
					{PMGR_CLK_S2, 1},
					{PMGR_CLK_S3, 1},
					{PMGR_CLK_PLL4, 2},
					{PMGR_CLK_PLL4, 7}
				    }
				},
			    
    [PMGR_CLK_NCO_ALG0]	    =	{ 
				    &rPMGR_NCO_ALG0_CLK_CFG,
				    {
					{PMGR_CLK_OSC, 1},
					{PMGR_CLK_S0, 1},
					{PMGR_CLK_S1, 1},
					{PMGR_CLK_S2, 1},
					{PMGR_CLK_S3, 1},
					{PMGR_CLK_PLL4, 10},
					{PMGR_CLK_PLL4, 12},
					{PMGR_CLK_PLL4, 16},
					{PMGR_CLK_PLL4, 24},
					{PMGR_CLK_PLL4, 40},
					{PMGR_CLK_PLL4, 50}
				    }
				},
			    
    [PMGR_CLK_NCO_ALG1]	    =	{ 
				    &rPMGR_NCO_ALG1_CLK_CFG,
				    {
					{PMGR_CLK_OSC, 1},
					{PMGR_CLK_S0, 1},
					{PMGR_CLK_S1, 1},
					{PMGR_CLK_S2, 1},
					{PMGR_CLK_S3, 1},
					{PMGR_CLK_PLL4, 10},
					{PMGR_CLK_PLL4, 12},
					{PMGR_CLK_PLL4, 16},
					{PMGR_CLK_PLL4, 24},
					{PMGR_CLK_PLL4, 40},
					{PMGR_CLK_PLL4, 50}
				    }
				},
			    
    [PMGR_CLK_HSICPHY_REF_12M]	=   {
					&rPMGR_HSICPHY_REF_12M_CLK_CFG,
					{
					    {PMGR_CLK_OSC, 1},
					    {PMGR_CLK_S0, 1},
					    {PMGR_CLK_S1, 1},
					    {PMGR_CLK_S2, 1},
					    {PMGR_CLK_S3, 1},
					    {PMGR_CLK_OSC, 2}
					}
				    },
				
    [PMGR_CLK_USB480_0]	    =	{
					&rPMGR_USB480_0_CLK_CFG,
					{
					    {PMGR_CLK_OSC, 1},
					    {PMGR_CLK_S0, 1},
					    {PMGR_CLK_S1, 1},
					    {PMGR_CLK_S2, 1},
					    {PMGR_CLK_S3, 1},
					    {PMGR_CLK_PLL4, 5}
					}
				},
			    
    [PMGR_CLK_USB480_1]	    =	{
				    &rPMGR_USB480_1_CLK_CFG,
					{
					    {PMGR_CLK_OSC, 1},
					    {PMGR_CLK_S0, 1},
					    {PMGR_CLK_S1, 1},
					    {PMGR_CLK_S2, 1},
					    {PMGR_CLK_S3, 1},
					    {PMGR_CLK_PLL4, 5}
					}
				},

    [PMGR_CLK_USB_OHCI_48M]	=   {
					&rPMGR_USB_OHCI_48M_CLK_CFG,
					{
					    {PMGR_CLK_OSC, 1},
					    {PMGR_CLK_S0, 1},
					    {PMGR_CLK_S1, 1},
					    {PMGR_CLK_S2, 1},
					    {PMGR_CLK_S3, 1},
					    {PMGR_CLK_PLL4, 50}
					}
				    },
			    
    [PMGR_CLK_USB]	    =	{
				    &rPMGR_USB_CLK_CFG,
				    {
					{PMGR_CLK_OSC, 1},
					{PMGR_CLK_S0, 1},
					{PMGR_CLK_S1, 1},
					{PMGR_CLK_S2, 1},
					{PMGR_CLK_S3, 1},
					{PMGR_CLK_PLL4, 20},
					{PMGR_CLK_PLL4, 24},
					{PMGR_CLK_PLL4, 40},
					{PMGR_CLK_PLL4, 50},
					{PMGR_CLK_PLL4, 72},
					{PMGR_CLK_PLL4, 80}
				    }
				},
			    
    [PMGR_CLK_USB_FREE_60M]	=   { 
					&rPMGR_USB_FREE_60M_CLK_CFG,
					{
					    {PMGR_CLK_OSC, 1},
					    {PMGR_CLK_S0, 1},
					    {PMGR_CLK_S1, 1},
					    {PMGR_CLK_S2, 1},
					    {PMGR_CLK_S3, 1},
					    {PMGR_CLK_PLL4, 40}
					}
				    },
				
    [PMGR_CLK_ADSP_T]	    =	{
				    &rPMGR_ADSP_T_CLK_CFG,
				    {
					{PMGR_CLK_OSC, 1},
					{PMGR_CLK_S0, 1},
					{PMGR_CLK_S1, 1},
					{PMGR_CLK_S2, 1},
					{PMGR_CLK_S3, 1},
					{PMGR_CLK_PLL4, 12},
					{PMGR_CLK_PLL4, 11},
					{PMGR_CLK_PLL4, 14},
					{PMGR_CLK_PLL4, 16}
				    }
				},
			    
    [PMGR_CLK_ADSP_P]	    =	{
				    &rPMGR_ADSP_P_CLK_CFG,
				    {
					{PMGR_CLK_OSC, 1},
					{PMGR_CLK_S0, 1},
					{PMGR_CLK_S1, 1},
					{PMGR_CLK_S2, 1},
					{PMGR_CLK_S3, 1},
					{PMGR_CLK_PLL4, 18},
					{PMGR_CLK_PLL4, 24}
				    }
				},
			    
    [PMGR_CLK_ADSP_TS]	    =	{
				    &rPMGR_ADSP_TS_CLK_CFG,
				    {
					{PMGR_CLK_OSC, 1},
					{PMGR_CLK_S0, 1},
					{PMGR_CLK_S1, 1},
					{PMGR_CLK_S2, 1},
					{PMGR_CLK_S3, 1},
					{PMGR_CLK_PLL4, 9},
				    }
				},
			    
    [PMGR_CLK_SIO_C]	    =	{
				    &rPMGR_SIO_C_CLK_CFG,
				    {
					{PMGR_CLK_OSC, 1},
					{PMGR_CLK_S0, 1},
					{PMGR_CLK_S1, 1},
					{PMGR_CLK_S2, 1},
					{PMGR_CLK_S3, 1},
					{PMGR_CLK_PLL4, 5},
					{PMGR_CLK_PLL4, 7},
					{PMGR_CLK_PLL4, 8},
					{PMGR_CLK_PLL4, 9},
					{PMGR_CLK_PLL4, 10},
					{PMGR_CLK_PLL4, 48}
				    }
				},
			    
    [PMGR_CLK_SIO_P]	    =	{ 
				    &rPMGR_SIO_P_CLK_CFG,
				    {
					{PMGR_CLK_OSC, 1},
					{PMGR_CLK_S0, 1},
					{PMGR_CLK_S1, 1},
					{PMGR_CLK_S2, 1},
					{PMGR_CLK_S3, 1},
					{PMGR_CLK_PLL4, 20},
					{PMGR_CLK_PLL4, 24},
					{PMGR_CLK_PLL4, 30},
					{PMGR_CLK_PLL4, 40},
					{PMGR_CLK_PLL4, 48},
					{PMGR_CLK_OSC,	2},
					{PMGR_CLK_OSC,	4}
				    }
				},
			    
    [PMGR_CLK_ISP_C]	    =	{
				    &rPMGR_ISP_C_CLK_CFG,
				    {
					{PMGR_CLK_OSC, 1},
					{PMGR_CLK_S0, 1},
					{PMGR_CLK_S1, 1},
					{PMGR_CLK_S2, 1},
					{PMGR_CLK_S3, 1},
					{PMGR_CLK_PLL4, 5},
					{PMGR_CLK_PLL4, 6},
					{PMGR_CLK_PLL4, 7},
					{PMGR_CLK_PLL4, 8},
					{PMGR_CLK_PLL4, 9},
					{PMGR_CLK_PLL4, 10},
					{PMGR_CLK_PLL4, 48}
				    }
				},
			    
    [PMGR_CLK_ISP]	    =	{
				    &rPMGR_ISP_CLK_CFG,
				    {
					{PMGR_CLK_OSC, 1},
					{PMGR_CLK_S0, 1},
					{PMGR_CLK_S1, 1},
					{PMGR_CLK_S2, 1},
					{PMGR_CLK_S3, 1},
					{PMGR_CLK_PLL4, 7},
					{PMGR_CLK_PLL4, 6},
					{PMGR_CLK_PLL4, 8},
					{PMGR_CLK_PLL4, 9},
					{PMGR_CLK_PLL4, 10},
					{PMGR_CLK_PLL4, 12}
				    }
				},
			    
    [PMGR_CLK_ISP_SENSOR0_REF]	=   { 
					&rPMGR_ISP_SENSOR0_REF_CLK_CFG,
					{
					    {PMGR_CLK_OSC, 1},
					    {PMGR_CLK_ISP_REF0, 1},
					    {PMGR_CLK_ISP_REF1, 1}
					}
				    },
				
    [PMGR_CLK_ISP_SENSOR1_REF]	=   {
					&rPMGR_ISP_SENSOR1_REF_CLK_CFG,
					{
					    {PMGR_CLK_OSC, 1},
					    {PMGR_CLK_ISP_REF0, 1},
					    {PMGR_CLK_ISP_REF1, 1}
					}
				    },
				
    [PMGR_CLK_VDEC]	    =	{
				    &rPMGR_VDEC_CLK_CFG,
				    {
					{PMGR_CLK_OSC, 1},
					{PMGR_CLK_S0, 1},
					{PMGR_CLK_S1, 1},
					{PMGR_CLK_S2, 1},
					{PMGR_CLK_S3, 1},
					{PMGR_CLK_PLL4, 8},
					{PMGR_CLK_PLL4, 9},
					{PMGR_CLK_PLL4, 12},
					{PMGR_CLK_PLL4, 16},
					{PMGR_CLK_PLL4, 24}
				    }
				},
			    
    [PMGR_CLK_VENC]	    =	{
				    &rPMGR_VENC_CLK_CFG,
				    {
					{PMGR_CLK_OSC, 1},
					{PMGR_CLK_S0, 1},
					{PMGR_CLK_S1, 1},
					{PMGR_CLK_S2, 1},
					{PMGR_CLK_S3, 1},
					{PMGR_CLK_PLL4, 7},
					{PMGR_CLK_PLL4, 8},
					{PMGR_CLK_PLL4, 9},
					{PMGR_CLK_PLL4, 12},
					{PMGR_CLK_PLL4, 16},
					{PMGR_CLK_PLL4, 24}
				    }
				},
			    
    [PMGR_CLK_VID0]	    =	{
				    &rPMGR_VID0_CLK_CFG,
				    {
					{PMGR_CLK_OSC, 1},
					{PMGR_CLK_S0, 1},
					{PMGR_CLK_S1, 1},
					{PMGR_CLK_S2, 1},
					{PMGR_CLK_S3, 1},
					{PMGR_CLK_PLL4, 11},
					{PMGR_CLK_PLL4, 39}
				    }
				},
			    
    [PMGR_CLK_DISP0]	    =	{
				    &rPMGR_DISP0_CLK_CFG,
				    {
					{PMGR_CLK_OSC, 1},
					{PMGR_CLK_S0, 1},
					{PMGR_CLK_S1, 1},
					{PMGR_CLK_S2, 1},
					{PMGR_CLK_S3, 1},
					{PMGR_CLK_PLL4, 6},
					{PMGR_CLK_PLL4, 7},
					{PMGR_CLK_PLL4, 8},
					{PMGR_CLK_PLL4, 9},
					{PMGR_CLK_PLL4, 12},
					{PMGR_CLK_PLL4, 14},
					{PMGR_CLK_PLL4, 24}
				    }
				},
			    
    [PMGR_CLK_DISP1]	    =	{
				    &rPMGR_DISP1_CLK_CFG,
				    {
					{PMGR_CLK_OSC, 1},
					{PMGR_CLK_S0, 1},
					{PMGR_CLK_S1, 1},
					{PMGR_CLK_S2, 1},
					{PMGR_CLK_S3, 1},
					{PMGR_CLK_PLL4, 6},
					{PMGR_CLK_PLL4, 7},
					{PMGR_CLK_PLL4, 8},
					{PMGR_CLK_PLL4, 9},
					{PMGR_CLK_PLL4, 12},
					{PMGR_CLK_PLL4, 14},
					{PMGR_CLK_PLL4, 24}
				    }
				},
			    
    [PMGR_CLK_AJPEG_IP]	    =	{
				    &rPMGR_AJPEG_IP_CLK_CFG,
				    {
					{PMGR_CLK_OSC, 1},
					{PMGR_CLK_S0, 1},
					{PMGR_CLK_S1, 1},
					{PMGR_CLK_S2, 1},
					{PMGR_CLK_S3, 1},
					{PMGR_CLK_PLL4, 12},
					{PMGR_CLK_PLL4, 11},
					{PMGR_CLK_PLL4, 16},
					{PMGR_CLK_PLL4, 24}				    
				    }
				},
			    
    [PMGR_CLK_AJPEG_WRAP]   =	{
				    &rPMGR_AJPEG_WRAP_CLK_CFG,
				    {
					{PMGR_CLK_OSC, 1},
					{PMGR_CLK_S0, 1},
					{PMGR_CLK_S1, 1},
					{PMGR_CLK_S2, 1},
					{PMGR_CLK_S3, 1},
					{PMGR_CLK_PLL4, 9},
					{PMGR_CLK_PLL4, 11},
					{PMGR_CLK_PLL4, 12},
					{PMGR_CLK_PLL4, 16},
					{PMGR_CLK_PLL4, 24}				
				    }
				},
			    
    [PMGR_CLK_MSR]	    =	{
				    &rPMGR_MSR_CLK_CFG,
				    {
					{PMGR_CLK_OSC, 1},
					{PMGR_CLK_S0, 1},
					{PMGR_CLK_S1, 1},
					{PMGR_CLK_S2, 1},
					{PMGR_CLK_S3, 1},
					{PMGR_CLK_PLL4, 6},
					{PMGR_CLK_PLL4, 7},
					{PMGR_CLK_PLL4, 8},
					{PMGR_CLK_PLL4, 9},
					{PMGR_CLK_PLL4, 12},
					{PMGR_CLK_PLL4, 16},
					{PMGR_CLK_PLL4, 24}
				    }
				},
			    
    [PMGR_CLK_AF]	    =	{
				    &rPMGR_AF_CLK_CFG,
				    {
					{PMGR_CLK_OSC, 1},
					{PMGR_CLK_S0, 1},
					{PMGR_CLK_S1, 1},
					{PMGR_CLK_S2, 1},
					{PMGR_CLK_S3, 1},
					{PMGR_CLK_PLL4, 5},
					{PMGR_CLK_PLL4, 6},
					{PMGR_CLK_PLL4, 7},
					{PMGR_CLK_PLL4, 8},
					{PMGR_CLK_PLL4, 9},
					{PMGR_CLK_PLL4, 10},
					{PMGR_CLK_PLL4, 12}
				    }
				},
			    
    [PMGR_CLK_ANS_DLL]	    =	{
				    &rPMGR_ANS_DLL_CLK_CFG,
				    {
					{PMGR_CLK_OSC, 1},
					{PMGR_CLK_S0, 1},
					{PMGR_CLK_S1, 1},
					{PMGR_CLK_S2, 1},
					{PMGR_CLK_S3, 1},
					{PMGR_CLK_PLL4, 7},
					{PMGR_CLK_PLL4, 6}
				    }
				},
			    
    [PMGR_CLK_ANS_C]	    =	{
				    &rPMGR_ANS_C_CLK_CFG,
				    {
					{PMGR_CLK_OSC, 1},
					{PMGR_CLK_S0, 1},
					{PMGR_CLK_S1, 1},
					{PMGR_CLK_S2, 1},
					{PMGR_CLK_S3, 1},
					{PMGR_CLK_PLL4, 5},
					{PMGR_CLK_PLL4, 6},
					{PMGR_CLK_PLL4, 7},
					{PMGR_CLK_PLL4, 8},
					{PMGR_CLK_PLL4, 9},
					{PMGR_CLK_PLL4, 10},
					{PMGR_CLK_PLL4, 48}
				    }
				},
			    
    [PMGR_CLK_ANC_LINK]	    =	{
				    &rPMGR_ANC_LINK_CLK_CFG,
				    {
					{PMGR_CLK_OSC, 1},
					{PMGR_CLK_S0, 1},
					{PMGR_CLK_S1, 1},
					{PMGR_CLK_S2, 1},
					{PMGR_CLK_S3, 1},
					{PMGR_CLK_PLL4, 9},
					{PMGR_CLK_PLL4, 7},
					{PMGR_CLK_PLL4, 7}, // TODO: rdar://problem/10749966
					{PMGR_CLK_PLL4, 8},
					{PMGR_CLK_PLL4, 12},
					{PMGR_CLK_PLL4, 18},
					{PMGR_CLK_PLL4, 24}
				    }
				},
			    
    [PMGR_CLK_LIO]	    =	{
				    &rPMGR_LIO_CLK_CFG,
				    {
					{PMGR_CLK_OSC, 1},
					{PMGR_CLK_S0, 1},
					{PMGR_CLK_S1, 1},
					{PMGR_CLK_S2, 1},
					{PMGR_CLK_S3, 1},
					{PMGR_CLK_PLL4, 10},
					{PMGR_CLK_PLL4, 12},
					{PMGR_CLK_PLL4, 14},
					{PMGR_CLK_PLL4, 16},
					{PMGR_CLK_PLL4, 18},
					{PMGR_CLK_PLL4, 20},
					{PMGR_CLK_PLL4, 48}
				    }
				},
			    
    [PMGR_CLK_MCA0_M]	    =	{
				    &rPMGR_NCO_CLK_CFG(0),
				    {
					{PMGR_CLK_OSC, 1},
					{PMGR_CLK_S0, 1},
					{PMGR_CLK_S1, 1},
					{PMGR_CLK_S2, 1},
					{PMGR_CLK_S3, 1},
					{PMGR_CLK_NOT_SUPPORTED, 1},
					{PMGR_CLK_NOT_SUPPORTED, 1},
					{PMGR_CLK_NOT_SUPPORTED, 1},
					{PMGR_CLK_NOT_SUPPORTED, 1},
					{PMGR_CLK_NOT_SUPPORTED, 1},
					{PMGR_CLK_OSC, 2},
					{PMGR_CLK_OSC, 4}
				    }
				},
			    
    [PMGR_CLK_MCA1_M]	    =	{
				    &rPMGR_NCO_CLK_CFG(1),
				    {
					{PMGR_CLK_OSC, 1},
					{PMGR_CLK_S0, 1},
					{PMGR_CLK_S1, 1},
					{PMGR_CLK_S2, 1},
					{PMGR_CLK_S3, 1},
					{PMGR_CLK_NOT_SUPPORTED, 1},
					{PMGR_CLK_NOT_SUPPORTED, 1},
					{PMGR_CLK_NOT_SUPPORTED, 1},
					{PMGR_CLK_NOT_SUPPORTED, 1},
					{PMGR_CLK_NOT_SUPPORTED, 1},
					{PMGR_CLK_OSC, 2},
					{PMGR_CLK_OSC, 4}
				    }
				},
			    
    [PMGR_CLK_MCA2_M]	    =	{
				    &rPMGR_NCO_CLK_CFG(2),
				    {
					{PMGR_CLK_OSC, 1},
					{PMGR_CLK_S0, 1},
					{PMGR_CLK_S1, 1},
					{PMGR_CLK_S2, 1},
					{PMGR_CLK_S3, 1},
					{PMGR_CLK_NOT_SUPPORTED, 1},
					{PMGR_CLK_NOT_SUPPORTED, 1},
					{PMGR_CLK_NOT_SUPPORTED, 1},
					{PMGR_CLK_NOT_SUPPORTED, 1},
					{PMGR_CLK_NOT_SUPPORTED, 1},
					{PMGR_CLK_OSC, 2},
					{PMGR_CLK_OSC, 4}
				    }
				},
			    
    [PMGR_CLK_MCA3_M]	    =	{
				    &rPMGR_NCO_CLK_CFG(3),
				    {
					{PMGR_CLK_OSC, 1},
					{PMGR_CLK_S0, 1},
					{PMGR_CLK_S1, 1},
					{PMGR_CLK_S2, 1},
					{PMGR_CLK_S3, 1},
					{PMGR_CLK_NOT_SUPPORTED, 1},
					{PMGR_CLK_NOT_SUPPORTED, 1},
					{PMGR_CLK_NOT_SUPPORTED, 1},
					{PMGR_CLK_NOT_SUPPORTED, 1},
					{PMGR_CLK_NOT_SUPPORTED, 1},
					{PMGR_CLK_OSC, 2},
					{PMGR_CLK_OSC, 4}
				    }
				},
			    
    [PMGR_CLK_MCA4_M]	    =	{
				    &rPMGR_NCO_CLK_CFG(4),
				    {
					{PMGR_CLK_OSC, 1},
					{PMGR_CLK_S0, 1},
					{PMGR_CLK_S1, 1},
					{PMGR_CLK_S2, 1},
					{PMGR_CLK_S3, 1},
					{PMGR_CLK_NOT_SUPPORTED, 1},
					{PMGR_CLK_NOT_SUPPORTED, 1},
					{PMGR_CLK_NOT_SUPPORTED, 1},
					{PMGR_CLK_NOT_SUPPORTED, 1},
					{PMGR_CLK_NOT_SUPPORTED, 1},
					{PMGR_CLK_OSC, 2},
					{PMGR_CLK_OSC, 4}
				    }
				},

    [PMGR_CLK_SEP]	    =	{
				    &rPMGR_SEP_CLK_CFG,
				    {
					{PMGR_CLK_OSC, 1},
					{PMGR_CLK_S0, 1},
					{PMGR_CLK_S1, 1},
					{PMGR_CLK_S2, 1},
					{PMGR_CLK_S3, 1},
					{PMGR_CLK_PLL4, 6},
					{PMGR_CLK_PLL4, 7},
					{PMGR_CLK_PLL4, 12},
					{PMGR_CLK_PLL4, 24}	
				    }
				},
			    
    [PMGR_CLK_GPIO]	    =	{
				    &rPMGR_GPIO_CLK_CFG,
				    {
					{PMGR_CLK_OSC, 1},
					{PMGR_CLK_S0, 1},
					{PMGR_CLK_S1, 1},
					{PMGR_CLK_S2, 1},
					{PMGR_CLK_S3, 1},
					{PMGR_CLK_PLL4, 100},
					{PMGR_CLK_PLL4, 50}
				    }
				},

    [PMGR_CLK_SPI0_N]	    =	{
				    &rPMGR_SPI0_N_CLK_CFG,
				    {
					{PMGR_CLK_OSC, 1},
					{PMGR_CLK_S0, 1},
					{PMGR_CLK_S1, 1},
					{PMGR_CLK_S2, 1},
					{PMGR_CLK_S3, 1},
					{PMGR_CLK_PLL4, 40},
					{PMGR_CLK_PLL4, 48},
					{PMGR_CLK_PLL4, 50}
				    }
				},

    [PMGR_CLK_SPI1_N]	    =	{
				    &rPMGR_SPI1_N_CLK_CFG,
				    {
					{PMGR_CLK_OSC, 1},
					{PMGR_CLK_S0, 1},
					{PMGR_CLK_S1, 1},
					{PMGR_CLK_S2, 1},
					{PMGR_CLK_S3, 1},
					{PMGR_CLK_PLL4, 40},
					{PMGR_CLK_PLL4, 48},
					{PMGR_CLK_PLL4, 50}
				    }
				},
			    
    [PMGR_CLK_SPI2_N]	    =	{
				    &rPMGR_SPI2_N_CLK_CFG,
					{
					{PMGR_CLK_OSC, 1},
					{PMGR_CLK_S0, 1},
					{PMGR_CLK_S1, 1},
					{PMGR_CLK_S2, 1},
					{PMGR_CLK_S3, 1},
					{PMGR_CLK_PLL4, 40},
					{PMGR_CLK_PLL4, 48},
					{PMGR_CLK_PLL4, 50}
				    }
				},
		    
    [PMGR_CLK_SPI3_N]	    =	{
				    &rPMGR_SPI3_N_CLK_CFG,
				    {
					{PMGR_CLK_OSC, 1},
					{PMGR_CLK_S0, 1},
					{PMGR_CLK_S1, 1},
					{PMGR_CLK_S2, 1},
					{PMGR_CLK_S3, 1},
					{PMGR_CLK_PLL4, 40},
					{PMGR_CLK_PLL4, 48},
					{PMGR_CLK_PLL4, 50}
				    }
				},

    [PMGR_CLK_DEBUG]	    =	{
				    &rPMGR_DEBUG_CLK_CFG,
				    {
					{PMGR_CLK_OSC, 1},
					{PMGR_CLK_S0, 1},
					{PMGR_CLK_S1, 1},
					{PMGR_CLK_S2, 1},
					{PMGR_CLK_S3, 1},
					{PMGR_CLK_PLL4, 20},
					{PMGR_CLK_PLL4, 24},
					{PMGR_CLK_PLL4, 30},
					{PMGR_CLK_PLL4, 40},
					{PMGR_CLK_PLL4, 48}
				    }
				},
			    
    [PMGR_CLK_S0]	    =	{
				    &rPMGR_S0_CLK_CFG,
				    {
					{PMGR_CLK_PLL1, 1},
					{PMGR_CLK_PLL2, 1},
					{PMGR_CLK_PLL3, 1},
					{PMGR_CLK_PLL4, 2}
				    }
				},
			    
    [PMGR_CLK_S1]	    =	{
				    &rPMGR_S1_CLK_CFG,
				    {
					{PMGR_CLK_PLL1, 1},
					{PMGR_CLK_PLL2, 1},
					{PMGR_CLK_PLL3, 1},
					{PMGR_CLK_PLL4, 2}				
				    }
				},
			    
    [PMGR_CLK_S2]	    =	{
				    &rPMGR_S2_CLK_CFG,
				    {
					{PMGR_CLK_PLL1, 1},
					{PMGR_CLK_PLL2, 1},
					{PMGR_CLK_PLL3, 1},
					{PMGR_CLK_PLL4, 2}				
				    }				    
				},
			    
    [PMGR_CLK_S3]	    =	{
				    &rPMGR_S3_CLK_CFG,
				    {
					{PMGR_CLK_PLL1, 1},
					{PMGR_CLK_PLL2, 1},
					{PMGR_CLK_PLL3, 1},
					{PMGR_CLK_PLL4, 2}				
				    }
				},
			    
    [PMGR_CLK_ISP_REF0]	    =	{
				    &rPMGR_ISP_REF0_CLK_CFG,
				    {
					{PMGR_CLK_PLL4, 5},
					{PMGR_CLK_PLL4, 7},
					{PMGR_CLK_PLL4, 8},
					{PMGR_CLK_PLL4, 9}
				    }
				},
			    
    [PMGR_CLK_ISP_REF1]	    =	{
				    &rPMGR_ISP_REF1_CLK_CFG,
				    {
					{PMGR_CLK_PLL4, 5},
					{PMGR_CLK_PLL4, 7},
					{PMGR_CLK_PLL4, 8},
					{PMGR_CLK_PLL4, 9}
				    }				    
				},
};

static uint32_t get_apsc_ccc_state(void);
static void set_apsc_ccc_state(uint32_t target_state);
#if (APPLICATION_IBOOT && !PRODUCT_IBOOT && !PRODUCT_IBEC)
static void init_thermal_sensors(void);
static void init_sochot(void);
static void init_ccc_thermal_sensors(void);
static void init_ccc_sochot(void);
#endif 
static void clocks_get_frequencies(void);
static void clocks_get_frequencies_range(uint32_t start_clk, uint32_t end_clk);
static uint32_t get_pll(int32_t pll);
static uint32_t get_pll_cpu(void);
static void set_pll(int32_t pll, uint32_t p, uint32_t m, uint32_t s, bool vco_output);
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
 * Look into rdar://problem/11647718 for further details.
 * S/W workaround below:
 * We need to turn ON the power partitions once.
 * Note: CCC, GFX are not effected so we won't touch them.
 *       ADSP is not used so we leave it that way.
 * After SecureROM hands off control we have the following partitions ON
 *  - Always on (Obviously!)
 *  - AMC
 *  - USB
 *  - ACS
 *  - ANS
 * So we will power ON 
 *  - DISP0
 *  - DISP1
 *  - MEDIA
 *  - ISP
 *  - VDEC
 *  - VENC.
 * and power them down.
 */
static void enable_bira_work_around(void)
{

	// Turn on the blocks 
	clock_gate(CLK_DISP_BUSMUX, true);
	clock_gate(CLK_DISP1, true);
	clock_gate(CLK_MEDIA, true);
	clock_gate(CLK_VDEC, true);
	clock_gate(CLK_VENC, true);

	// FPGA doesn't have ISP
#if !SUPPORT_FPGA
	clock_gate(CLK_ISP, true);
	clock_gate(CLK_ISP, false);
#endif

	// Power down in reverse order.
	// Note: The PWR_XXX_CFG0 register for these partitions
	// enable power down.
	clock_gate(CLK_VENC, false);
	clock_gate(CLK_VDEC, false);
	clock_gate(CLK_MEDIA, false);
	clock_gate(CLK_DISP1, false);
	clock_gate(CLK_DISP_BUSMUX, false);
}

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
	for(i = 0; i < 5; i++)
		rPMGR_NCO_CLK_CFG(i) |= (1 << 31); 
}

static void apply_pmgr_tunables()
{
#define RESET_TIME_MASK  (((1 << 8) - 1) << 0)
#define CLAMP_TIME_MASK  (((1 << 8) - 1) << 8)
#define CLK_EN_TIME_MASK (((1 << 8) - 1) << 16)
#define _PWRGATE_CFG0(rCFG0, CLK_EN_TIME, CLAMP_TIME, RESET_TIME) \
	regTemp = rCFG0; \
	regTemp &= ~RESET_TIME_MASK; \
	regTemp |= (RESET_TIME << 0); \
	regTemp &= ~CLAMP_TIME_MASK; \
	regTemp |= (CLAMP_TIME << 8); \
	regTemp &= ~CLK_EN_TIME_MASK; \
	regTemp |= (CLK_EN_TIME << 16); \
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

	uint32_t regTemp, i;

	// B0, B1 tunables.

	_PWRGATE_CFG0(rPMGR_PWRGATE_AMC_CFG0, 0x2, 0x2, 0x8);
	_PWRGATE_CFG1(rPMGR_PWRGATE_AMC_CFG1, 0x25, 0x0);

	_PWRGATE_CFG0(rPMGR_PWRGATE_USB_CFG0, 0x3, 0x2, 0x4);
	_PWRGATE_CFG1(rPMGR_PWRGATE_USB_CFG1, 0xd, 0x0);	

	_PWRGATE_CFG0(rPMGR_PWRGATE_ACS_CFG0, 0x2, 0x2, 0x7);
	_PWRGATE_CFG1(rPMGR_PWRGATE_ACS_CFG1, 0x21, 0x0);

	_PWRGATE_CFG0(rPMGR_PWRGATE_DISP0_CFG0, 0x3, 0x2, 0x4);
	_PWRGATE_CFG1(rPMGR_PWRGATE_DISP0_CFG1, 0x6a, 0x0);

	_PWRGATE_CFG0(rPMGR_PWRGATE_DISP1_CFG0, 0x3, 0x2, 0x4);
	_PWRGATE_CFG1(rPMGR_PWRGATE_DISP1_CFG1, 0x40, 0x0);

	_PWRGATE_CFG0(rPMGR_PWRGATE_ISP_CFG0, 0x3, 0x2, 0x4);
	_PWRGATE_CFG1(rPMGR_PWRGATE_ISP_CFG1, 0x63, 0x1);

	_PWRGATE_CFG0(rPMGR_PWRGATE_MEDIA_CFG0, 0x3, 0x2, 0x4);
	_PWRGATE_CFG1(rPMGR_PWRGATE_MEDIA_CFG1, 0x50, 0x0);

	_PWRGATE_CFG0(rPMGR_PWRGATE_DEC_CFG0, 0x3, 0x2, 0x4);
	_PWRGATE_CFG1(rPMGR_PWRGATE_DEC_CFG1, 0x5a, 0x0);

	_PWRGATE_CFG0(rPMGR_PWRGATE_ENC_CFG0, 0x3, 0x2, 0x4);
	_PWRGATE_CFG1(rPMGR_PWRGATE_ENC_CFG1, 0x67, 0x0);

	_PWRGATE_CFG0(rPMGR_PWRGATE_ANS_CFG0, 0x3, 0x2, 0x4);
	_PWRGATE_CFG1(rPMGR_PWRGATE_ANS_CFG1, 0x49, 0x2);

	_PWRGATE_CFG0(rPMGR_PWRGATE_ADSP_CFG0, 0x3, 0x2, 0x4);
	_PWRGATE_CFG1(rPMGR_PWRGATE_ADSP_CFG1, 0x1c, 0x0);

	_PWRGATE_CFG0(rPMGR_PWRGATE_GFX_CFG0, 0x3, 0x2, 0x4);
	_PWRGATE_CFG1(rPMGR_PWRGATE_GFX_CFG1, 0x16, 0x0);

	_PWRGATE_CFG0(rPMGR_PWRGATE_SEP_CFG0, 0x3, 0x2, 0x4);
	_PWRGATE_CFG1(rPMGR_PWRGATE_SEP_CFG1, 0x3c, 0x0);

	// 2. Apply MCU Async reset timing.

	rPMGR_MCU_ASYNC_RESET = (0x1 << 0) | (0x1 << 4) | (0x1 << 8);

	// 3. Apply VolMan tunables

	rPMGR_VOLMAN_SOC_DELAY  =  (0x5 << 0) | (0x6b << 10) | (0xbb8 << 20);
	rPMGR_VOLMAN_SRAM_DELAY =  (0x5 << 0) | (0x6b << 10) | (0xbb8 << 20);
	rPMGR_VOLMAN_GFX_DELAY  =  (0x5 << 0) | (0x6b << 10) | (0xbb8 << 20);
	rPMGR_VOLMAN_CPU_DELAY  =  (0x5 << 0) | (0x6b << 10) | (0xbb8 << 20);

	// 4. Apply GFX EMA0 & EMA1 tunables. 

	rPMGR_EMA_GFX0 = (0x4 << 0) | (0x4 << 5) | (0x4 << 11) | (0x4 << 16) | (0xa << 22) | (0x7 << 26) | (0x7 << 29);
	rPMGR_EMA_GFX1 = (0x5 << 0) | (0x4 << 3) | (0x1 << 6) | (0x4 << 10) | (0x4 << 13) | (0x5 << 20) | (0x4 << 23) | (0x3 << 29);

	// 5. PLLx_ANA_PARAMSy tunables
	for (i = 0; i < PMGR_PLL_COUNT; i++) {
		rPMGR_PLL_ANA_PARAMS(0, i) = 0x61a8308;
		rPMGR_PLL_ANA_PARAMS(1, i) = 0xcec0000;
	}

	// WA for the lack of a clean SW reset of PD_ANS.
	// By enabling power gating we will have a reset when 
	// PD_ANS is turned back on.
	// <rdar://problem/13486314>, <rdar://problem/13403412> 
	// has further details on this WA.
	rPMGR_PWRGATE_ANS_CFG0 |= (1 << 31);
	return;
}

static void apply_ccc_tunables(void)
{
	rCCC_PRE_TD_TMR = 0x4;
	rCCC_PRE_FLUSH_TMR = 0x1000;
}
#endif

/* 
 * clocks_set_default - called by SecureROM, LLB, iBSS main via
 * platform_init_setup_clocks, so the current state of the chip is
 * either POR, or whatever 'quiesce' did when leaving SecureROM.
 */
 
int clocks_set_default(void)
{
    uint32_t cnt;
#if APPLICATION_IBOOT
	u_int32_t cpu_vid[kDVFM_STATE_IBOOT_CNT];
	u_int32_t	gpu_vid[kPMGR_GFX_STATE_MAX];
#endif

    clks[PMGR_CLK_OSC] = OSC_FREQ;

    volatile uint32_t *clkcfgs = PMGR_FIRST_CLK_CFG;
    volatile uint32_t *spare_clkcfgs = PMGR_FIRST_SPARE_CLK_CFG;

    // Setup bypass DVFM state
    rCCC_DVFM_ST(kDVFM_STATE_BYPASS) = ccc_dvfm_states[kDVFM_STATE_BYPASS];

#if (APPLICATION_IBOOT && !PRODUCT_IBOOT && !PRODUCT_IBEC)
	if (platform_get_chip_revision() > CHIP_REVISION_B0) {
		set_ccc_pll_relock_div2();
	}
#endif
    // Change all clocks to something safe
    clocks_quiesce_internal();

    // Setup active DVFM state for the stage of boot.
#if APPLICATION_SECUREROM
    rCCC_DVFM_ST(kDVFM_STATE_SECUREROM) = ccc_dvfm_states[kDVFM_STATE_SECUREROM];
#endif

#if APPLICATION_IBOOT
#ifndef BUCK_CPU
#error BUCK_CPU not defined for this platform
#endif
	// Get the binned voltages and update the CCC DVFM state registers.
	platform_get_cpu_voltages(kDVFM_STATE_IBOOT_CNT, cpu_vid);
	platform_convert_voltages(BUCK_CPU, kDVFM_STATE_IBOOT_CNT, cpu_vid);

	for (cnt = kDVFM_STATE_IBOOT; cnt < kDVFM_STATE_IBOOT_CNT; cnt++) {
    		rCCC_DVFM_ST(cnt) = ccc_dvfm_states[cnt] | ((unsigned long long)cpu_vid[cnt] << CCC_DVFM_ST_SAFE_VOL_SHIFT);
	}
	// APSC sleep state will use the bypass state with V0.

	rCCC_DVFM_ST(kDVFM_STATE_BYPASS) = ccc_dvfm_states[kDVFM_STATE_BYPASS] | ((unsigned long long)cpu_vid[kDVFM_STATE_V0] << CCC_DVFM_ST_SAFE_VOL_SHIFT);

	// To prevent crashes/hangs during update install due to 
	// mismatch of CCC clock config info between an old LLB and a new EDT+OS
	// we can populate the unused entries of the DVFM table with that of
	// Vmin, Fmin. That way we will always have a workable entry in the DVFM
	// table.
	// Assumptions: 
	// There is always a valid state for iBoot and we will use that to populate the
	// empty table entries.

	for (cnt = kDVFM_STATE_IBOOT_CNT; cnt < CCC_DVFM_STATE_COUNT; cnt++) {
		rCCC_DVFM_ST(cnt) = ccc_dvfm_states[kDVFM_STATE_IBOOT] | 
			((unsigned long long)cpu_vid[kDVFM_STATE_IBOOT] << CCC_DVFM_ST_SAFE_VOL_SHIFT);
	}

	// Using one point calibration at the lower reading. 
	uint32_t tempOffset0 = (rCCC_DVFM_EFUSE_TADC0 & 0x1FF) >> 2;
	uint32_t tempOffset1 = (rCCC_DVFM_EFUSE_TADC1 & 0x1FF) >> 2;

	// If device is calibrated with DVFM FUSE values, then we need to subtract this offset. 
	if (0 != tempOffset0) {
		tempOffset0 = (0x38 - tempOffset0) & 0x7F;
	}
	if (0 != tempOffset1) {
		tempOffset1 = (0x38 - tempOffset1) & 0x7F;
	}

	// Program TVM thresholds
	rCCC_DVFM_CFG |= CCC_DVFM_CFG_TEMPTHRES0(0x34)        | CCC_DVFM_CFG_TEMPTHRES1(0x48) |
			 CCC_DVFM_CFG_TEMPOFFST0(tempOffset0) | CCC_DVFM_CFG_TEMPOFFST1(tempOffset1);

	// Using default programmed value of 1ms.  
	// rCCC_DVFM_DLY = 0x180000;
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
    platform_get_gpu_voltages(sizeof(gfx_states)/sizeof(gfx_states[0]), gpu_vid);
    platform_convert_voltages(BUCK_GPU, sizeof(gfx_states)/sizeof(gfx_states[0]), gpu_vid);

    for (cnt = 0; cnt < sizeof(gfx_states)/sizeof(gfx_states[0]); cnt++) {
		if (cnt == 0)
			gfx_states[cnt].dwi_val = 0;
		else
			gfx_states[cnt].dwi_val = gpu_vid[cnt];

        set_gfx_perf_state(cnt, gfx_states);
    }
	// As per GFX Performance state software Sequence
	// Clear the bypass bit in PLL5
	rPMGR_PLL_CTL(5) &= ~(1 << 27);
	// PLL5 relock mode is set to 1 to switch to bypass mode while re-locking.
	rPMGR_PLL_CFG(5) |= (1 << 24);
	// Enable performance state table to control PLL5
	rPMGR_GFX_PERF_STATE_CTL |= (1 << 31);

	// The initial SRAM, SOC, CPU voltages are set by IIC writes to the PMU (in the pmu driver).
#endif

#ifdef PLL0_T	//MIPI
#if WITH_HW_AGC_MIPI
	rPMGR_AGILE_CLK_CTL = (1 << 28);
	rPMGR_PLL_ACG_CFG |=  PMGR_ACG_CFG_PLLX_AUTO_DISABLE(0);
#endif
    set_pll(0, PLL0_P, PLL0_M, PLL0_S, 0);
#endif

#ifdef PLL1_T	//Mem
    set_pll(1, PLL1_P, PLL1_M, PLL1_S, 0);
#endif

#ifdef PLL2_T	//Unused
    set_pll(2, PLL2_P, PLL2_M, PLL2_S, 0);
#endif

#ifdef PLL3_T	//Unused
    set_pll(3, PLL3_P, PLL3_M, PLL3_S, 0);
#endif

#ifdef PLL4_T	// SOC
    set_pll(4, PLL4_P, PLL4_M, PLL4_S, PLL4_VCO_ENABLED);
#endif

#ifdef PLL5_T	// GFX
    set_pll(5, PLL5_P, PLL5_M, PLL5_S, 0);
#endif

#if (APPLICATION_IBOOT && !PRODUCT_IBOOT && !PRODUCT_IBEC)
	// Turn on NCO clocks before enabling MCA clocks.
	set_nco_clocks();
#endif
    // Set all spare clock divs to their active values
    for (cnt = 0; cnt < PMGR_SPARE_CLK_CFG_COUNT; cnt++) {
		spare_clkcfgs[cnt] = spare_divs_active[cnt];
		SPIN_W_TMO_WHILE((spare_clkcfgs[cnt] & PMGR_CLK_CFG_PENDING) != 0);
    }

    // Set all but the spare clock divs to their active values
    for (cnt = 0; cnt < PMGR_CLK_CFG_COUNT; cnt++) {
		clkcfgs[cnt] = clk_divs_active[cnt];
		SPIN_W_TMO_WHILE((clkcfgs[cnt] & PMGR_CLK_CFG_PENDING) != 0);
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
    rCCC_APSC_SCR = CCC_APSC_MANUAL_CHANGE(target_state);
    SPIN_W_TMO_WHILE((rCCC_APSC_SCR & CCC_APSC_PENDING) != 0);

    return;
}

static void set_pll(int32_t pll, uint32_t p, uint32_t m, uint32_t s, bool vco_output)
{
    if (pll >= PMGR_PLL_COUNT) return;

    if (vco_output) rPMGR_PLL_CFG(pll) |= PMGR_PLL_VCO_OUT_SEL;

    rPMGR_PLL_CTL(pll) = (PMGR_PLL_P(p) | PMGR_PLL_M(m) | PMGR_PLL_S(s) |
			     PMGR_PLL_ENABLE | PMGR_PLL_LOAD);

    SPIN_W_TMO_WHILE((rPMGR_PLL_CTL(pll) & PMGR_PLL_LOCKED) == 0);
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
    uint64_t freq = 0;

    pllctl = rPMGR_PLL_CTL(pll);

    // If PLL is not enabled, check for External Bypass
    if ((pllctl & PMGR_PLL_ENABLE) == 0) {
		if ((pllctl & PMGR_PLL_EXT_BYPASS) == 0) 
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
	if (freq> 0xFFFFFFFF)
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

    src_idx = (reg_val >> 24) & PMGR_CLK_CFG_SRC_SEL_MASK;
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

    clks[PMGR_CLK_CPU]	= 5000000;
    clks[PMGR_CLK_MCU]	= 10000000;
    clks[PMGR_CLK_MCU_FIXED]= 10000000;
    clks[PMGR_CLK_USB]	= 12000000;

#elif SUB_TARGET_CYCLONIC
    uint32_t cnt;
    uint32_t freq = OSC_FREQ;

    for (cnt = 0; cnt < PMGR_CLK_COUNT; cnt++)
	clks[cnt] = freq;

#else
    uint32_t cnt;

    clks[PMGR_CLK_OSC] = OSC_FREQ;

    // Use get_pll() to establish the frequencies (unconfigured PLLs will bypass OSC)
    for (cnt = 0; cnt < PMGR_PLL_COUNT; cnt++) clks[PMGR_CLK_PLL0 + cnt] = get_pll(cnt);

    // Use get_spare() to establish the frequencies for spare clocks (unconfigured will be skipped)
    for (cnt = 0; cnt < PMGR_SPARE_CLK_CFG_COUNT; cnt++) clks[PMGR_CLK_S0 + cnt] = get_spare(cnt);

    clks[PMGR_CLK_CPU] = get_pll_cpu();

	clocks_get_frequencies_range(PMGR_CLK_MCU_FIXED, PMGR_CLK_DEBUG);
#endif
}

static void clocks_get_frequencies_range(uint32_t start_clk, uint32_t end_clk)
{
    volatile uint32_t *reg;
    uint32_t cnt, val, src_idx, src_clk, src_factor;

    if (start_clk < PMGR_CLK_MCU_FIXED || end_clk > PMGR_CLK_DEBUG)
        return;

    for (cnt = start_clk; cnt <= end_clk; cnt++) {
        reg = clk_configs[cnt].clock_reg;
        val = *reg;

        if ((val & PMGR_CLK_CFG_ENABLE) == 0) {
             clks[cnt] = 0;
             continue;
        }

        src_idx = (val >> 24) & PMGR_CLK_CFG_SRC_SEL_MASK;
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
		SPIN_W_TMO_WHILE(clkcfgs[reg] & PMGR_CLK_CFG_PENDING);
		reg++;
    }
}

static void restore_clock_config_state(void)
{
    uint32_t cnt;

    // 2. Write reset value to ACG, CLK_DIVIDER_ACG_CFG, CLK_DIVIDER_ACG_CFG1, and PLL_ACG_CFG 
    rPMGR_MISC_ACG = 0;
    rPMGR_CLK_DIVIDER_ACG_CFG = 0;
    rPMGR_CLK_DIVIDER_ACG_CFG1 = 0;
    rPMGR_PLL_ACG_CFG = 0;

   // 5. Write reset value for all mux clock configs (excluding spares, mcu, mcu_fixed)
    clock_update_range(PMGR_CLK_NUM(MIPI_DSI), PMGR_CLK_NUM(DEBUG), 0x80100000);

    // 6. Write all PLLx_CTL.EXT_BYPASS to 1
    for (cnt = 0; cnt < PMGR_PLL_COUNT; cnt++) {
		if (cnt == 1) continue;

		rPMGR_PLL_CTL(cnt) |= PMGR_PLL_EXT_BYPASS;
		SPIN_W_TMO_WHILE((rPMGR_PLL_CTL(cnt) & PMGR_PLL_BYP_ENABLED) == 0);
    }

    // 8. Make sure PLLs OFF mode is set to powered down.
    for (cnt = 0; cnt < PMGR_PLL_COUNT; cnt++) {
		if (cnt == 1) continue; // Mem PLL

		rPMGR_PLL_CFG(cnt) |= (2 << 30); // PLL.OFF_MODE = Powered down.
		rPMGR_PLL_CTL(cnt) |= (PMGR_PLL_ENABLE | PMGR_PLL_LOAD);
		SPIN_W_TMO_WHILE((rPMGR_PLL_CTL(cnt) & PMGR_PLL_LOCKED) == 0);
    }

    // 9. Write reset value to all PLLx_CTL
    for (cnt = 0; cnt < PMGR_PLL_COUNT; cnt++) {
		if (cnt == 1) continue; // Mem PLL
	    
		rPMGR_PLL_CTL(cnt) = 0x0A001010; // Ext bypass, fb_div = 1, pre_div = 1
		SPIN_W_TMO_WHILE((rPMGR_PLL_CTL(cnt) & PMGR_PLL_BYP_ENABLED) == 0);
    }

    // 11. Write reset value to spare and ISP_REF0/1
    clock_update_range(PMGR_CLK_NUM(S0), PMGR_CLK_NUM(ISP_REF1), 0x80000001);


    // 12. Put CPU clock back into its reset default.
    rCCC_DVFM_SCR = (1 << 2); // CPU PLL goes to reset mode when off.
    set_apsc_ccc_state(kDVFM_STATE_BYPASS);
}


static void power_on_sep(void)
{
	volatile uint32_t *reg = (volatile uint32_t *)((uint64_t *)PMGR_FIRST_PS + CLK_SEP);
	uint32_t val = *reg;

	val &= ~(1 << 28); // Clear Auto_PM_EN	
	*reg = 	val;	
	SPIN_W_TMO_WHILE(((*reg >> 4) & 0xF) != 0xF); // Wait for SEP to turn on.

	return;
}

static void clocks_quiesce_internal(void)
{
    uint64_t devices[2];

    // Disable all voltage changes everywhere!
	rPMGR_VOLMAN_CTL |= PMGR_VOLMAN_DISABLE_VOL_CHANGE;
#if APPLICATION_IBOOT
	rPMGR_VOLMAN_CTL |= 1; // Temporary till B0 chip arrives
#endif
    // We don't touch CPU0, CPU1, CPM
    //	Following devices needs to be on:
    //	LIO, IOMUX, AIC, DEBUG
    //	GPIO, SIO_P, SIO, MCC, MCU, 
    //	AMP, USB, USBCTRL, USB OTG, SMX, SF, CP
    devices[0] = (0x1ULL << 3) | (0x1ULL << 4) | (0x1ULL << 5) | (0x1ULL << 6) |
			 (0x1ULL << 8) | (0x1ULL << 33) | (0x1ULL << 34) | (0x1ULL << 40) | (0x1ULL << 41) | 
		 (0x1ULL << 42) | (0x1ULL << 43) | (0x1ULL << 44) | (0x1ULL << 49) | 
		 (0x1ULL << 50) | (0x1ULL << 51) | (0x1ULL << 52);

    // ANS.
    devices[1] = (0x1ULL << 0);

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
#if APPLICATION_IBOOT
    int cfg_sel = -1;
    uint32_t src_index;
#endif
    uint32_t ccc_state = get_apsc_ccc_state();

    if (ccc_state != active_state)
        set_apsc_ccc_state(active_state);

#if APPLICATION_IBOOT
    if (performance_level == kPerformanceMemoryLow || performance_level == kPerformanceMemoryFull) {
        cfg_sel = get_amc_cfg_sel(performance_level);

        if (cfg_sel == -1)
            panic("pmgr:cfg_sel not set correctly for configuring amc clock");

        src_index = (performance_level == kPerformanceMemoryLow) ? 6 : 1;

        set_amc_clk_config(src_index, cfg_sel);
        clocks_get_frequencies_range(PMGR_CLK_MCU_FIXED, PMGR_CLK_MCU);
    }
#endif
    // At this point we should have CCC clock set for this stage of boot.
    return kPerformanceHigh;
}

void clock_get_frequencies(uint32_t *clocks, uint32_t count)
{
    uint32_t cnt = PMGR_CLK_COUNT;

    if (cnt > count) cnt = count;

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
	case CLK_MIPI:
		freq = clks[PMGR_CLK_MIPI_DSI];
		break;
    default:
		break;
    }
    return freq;
}

void clock_set_frequency(int clock, uint32_t divider, uint32_t pll_p, uint32_t pll_m, uint32_t pll_s, uint32_t pll_t)
{
	return;
}

void clock_gate(int device, bool enable)
{
    volatile uint32_t *reg = (volatile uint32_t *)((uint64_t *)PMGR_FIRST_PS + device);

	// Make sure we are within limits. Also we won't touch ADSP.
    if ((reg > PMGR_LAST_PS) || (device == CLK_ADSP))
		return;

    // Set the PS field to the requested level
    if (enable) 
		*reg |= PMGR_PS_RUN_MAX;
    else
		*reg &= ~PMGR_PS_RUN_MAX;

    // Wait for the MANUAL_PS and ACTUAL_PS fields to be equal
    SPIN_W_TMO_WHILE((*reg & PMGR_PS_MANUAL_PS_MASK) != ((*reg >> PMGR_PS_ACTUAL_PS_SHIFT) & PMGR_PS_ACTUAL_PS_MASK));
}

static void clocks_set_gates(uint64_t *devices)
{
    uint32_t i, idx = 1;

	// Turn off devices hi to lo order (to meet dependencies).
	for (i = PMGR_LAST_DEVICE - 1; i >= CLK_LIO;) {
		clock_gate(i, ( (devices[idx] >> ((uint64_t) (i % 64)) ) & 0x1) );
		if (i && ((i % 64) == 0))
			idx--;

		i--;
	}

    idx = 0;

	// Turn on devices from lo to hi (to meet dependencies).
    for (i = CLK_LIO; ( (i < 128) && (i < PMGR_LAST_DEVICE) ); ) {
		if (i && ((i % 64) == 0))
		    idx++;
		clock_gate(i, ( (devices[idx] >> ((uint64_t) (i % 64)) ) & 0x1) );
		i++;
    }

    return;
}

void platform_system_reset(bool panic)
{
#if WITH_BOOT_STAGE
    if (!panic) boot_set_stage(kPowerNVRAMiBootStageOff);
#endif

    wdt_system_reset();

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

void platform_power_init(void)
{
    /* Disable Power gating for CPU0, this is needed to avoid WFI from powering down the core */
    /* Remove this once following is addressed: <rdar://problem/10869952> Core 0 power gating enabled by default */
    rPMGR_PWRGATE_CPU0_CFG0 &= ~(1UL << 31);
}

void thermal_init(void)
{
#if (APPLICATION_IBOOT && !PRODUCT_IBOOT && !PRODUCT_IBEC)
	/* Read thermal fused values and store into thermal registers */
	init_thermal_sensors();
	/* Read thermal fused values and store into thermal registers & init instantenous read for sochot */
	init_ccc_thermal_sensors();

	/* Setup SoC Sochot 0 & 1 */
	init_sochot();
	/* Setup CCC Sochot 0 & 1 */
	init_ccc_sochot();
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

	// Make sure we are within limits. Also we won't touch ADSP.
    if ((device >= PMGR_LAST_DEVICE) || (device == CLK_ADSP))
		return;

    // XXX TODO
    switch(device) {
    case CLK_MCU:
    case CLK_ANS:
    case CLK_SIO:
		*reg |= PMGR_PS_RESET;
		spin(1);
		*reg &= ~PMGR_PS_RESET;
		break;

    default:
		break;
    }
}

#if APPLICATION_IBOOT
static void set_gfx_perf_state(uint32_t state_num, struct gfx_state_info *gfx_state)
{
	uint32_t pll_enable = 0;

	if (state_num >= sizeof(gfx_states)/sizeof(gfx_states[0]))	return;

	// This is deductive. If feedback divider is 0 the PLL shouldn't output anything. 
	pll_enable = gfx_state[state_num].fb_div ? 1 : 0;

	rPMGR_GFX_PERF_STATE_ENTRY(state_num) = ((gfx_state[state_num].dwi_val & 0xFF) << 24) |
											((pll_enable & 0x1) << 21) |
											((gfx_state[state_num].fb_div & 0x1FF) << 12) |
											((gfx_state[state_num].pre_div & 0x1F) << 4) |
											(gfx_state[state_num].op_div & 0xF);
	return;
}

static int get_amc_cfg_sel(uint32_t performance_level)
{
	int cfg_sel;
	switch(performance_level) {
	case kPerformanceMemoryLow:
		cfg_sel = 3;
		break;
	case kPerformanceMemoryFull:
		cfg_sel = 0;
		break;
	default:
		cfg_sel = -1;
		break;
	}
	return cfg_sel;
}

static void set_amc_clk_config(uint32_t src_index, uint32_t cfg_sel)
{
	volatile uint32_t *clkcfgs = PMGR_FIRST_CLK_CFG;
	uint32_t mcu_clk_cfg_reg;

	mcu_clk_cfg_reg = clkcfgs[PMGR_CLK_CFG_INDEX(PMGR_CLK_MCU)];

	mcu_clk_cfg_reg &= ~(0x7 << PMGR_CLK_CFG_SRC_SEL_SHIFT);
	mcu_clk_cfg_reg &= ~(0x3 << PMGR_CLK_CFG_CFG_SEL_SHIFT);
	mcu_clk_cfg_reg |= ((src_index & 0x7) << PMGR_CLK_CFG_SRC_SEL_SHIFT);
	mcu_clk_cfg_reg |= ((cfg_sel & 0x3) << PMGR_CLK_CFG_CFG_SEL_SHIFT);
	
	clkcfgs[PMGR_CLK_CFG_INDEX(PMGR_CLK_MCU)] = mcu_clk_cfg_reg;
	
	// Wait for Pending bit to clear.
	SPIN_W_TMO_WHILE((clkcfgs[PMGR_CLK_CFG_INDEX(PMGR_CLK_MCU)] >> PMGR_CLK_CFG_PENDING_SHIFT) & 0x1);

	return;
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
	uint32_t cpu_vid[kDVFM_STATE_IBOOT_CNT];

	char *propName;
	void *propData;

	// Populate the devicetree with relevant values.
	propName = "nominal-performance1";
	if (FindProperty(pmgr_node, &propName, &propData, &propSize)) {
		if (propSize != sizeof(uint32_t))
			panic("pmgr property nominal-performance1 is of wrong size.");

		freq = get_freq_from_ccc_state(ccc_dvfm_states[kDVFM_STATE_VNOM]);
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

		freq = get_freq_from_ccc_state(ccc_dvfm_states[kDVFM_STATE_VBOOST]);
		if (freq == 0)
			panic("pmgr Fboost Operating point not defined correctly");
		period_ns = 1000000000ULL << 16;
		period_ns /= freq;
		((uint32_t *)propData)[0] = period_ns;
	}

	propName = "voltage-states1";
	if (FindProperty(pmgr_node, &propName, &propData, &propSize)) {
		if (propSize/sizeof(uint32_t) < kVOLTAGE_STATES1_SIZE) {
			panic("pmgr number of states less than required for voltage-states1");
		}
		num_freqs = kVOLTAGE_STATES1_COUNT;
		platform_get_cpu_voltages(kDVFM_STATE_IBOOT_CNT, cpu_vid);
		for (int32_t i = num_freqs - 1, j = 0; i >= 0; i--, j++) {
			freq = get_freq_from_ccc_state(ccc_dvfm_states[kDVFM_STATE_VMAX - j]);
			volt = cpu_vid[kDVFM_STATE_VMAX - j];
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
			if ((count != 0) && (state_val == 0)) {
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
		*(u_int32_t *)propData = num_states;
	}

	return;
}

static void override_device_tree_property_uint32(DTNode *node, char *propName, uint32_t propValue, bool panicIfMissing)
{
	void *propData;
	uint32_t propSize;

	if (FindProperty(node, &propName, &propData, &propSize)) {
		if (propSize != sizeof(uint32_t)) {
			panic("%s property is of wrong size.", propName);
		}

		((uint32_t *)propData)[0] = propValue;
	} else if (panicIfMissing) {
		panic("%s property not found.", propName);
	}
}

void sochot_pmgr_update_device_tree(DTNode *node)
{
}

void sochot_ccc_update_device_tree(DTNode *node)
{
}

void temp_sensor_pmgr_update_device_tree(DTNode *node)
{
}

void temp_sensor_ccc_update_device_tree(DTNode *node)
{
}
#endif

#if (APPLICATION_IBOOT && !PRODUCT_IBOOT && !PRODUCT_IBEC)

void init_thermal_sensors(void)
{
	// Bits [1  -  0] PWRDN Mode and enable.
	// Bits [   2   ] Reserved.
	// Bits [   3   ] Stat mode for Avg Max on.
	// Bits [18 -  4] TADC_CFG Tmpsadc configuration bits.
	// Bits [31 - 19] Misc interrupt/Alarm stick bits not used.
	rPMGR_THERMAL0_CTL0 = 0x5278;
	rPMGR_THERMAL1_CTL0 = 0x5278;

	// Bits [15 -  0] PWRDN_START 20us wait
	// Bits [31 - 16] PWRDN_GAP Gap between two readings.
	rPMGR_THERMAL0_CTL1 = 0x019401E4;
	rPMGR_THERMAL1_CTL1 = 0x03A801E4;

	// Bits [7  -  0] Conv_Cycle, cycles to wait before data is valid.
	// Bits [15 -  8] Enable_Cycle, cycles to wait before adc_en can be deasserted.
	// Bits [23 - 16] Finish_gap, Cycles to wait before adc_en can be reasserted.
	// Bits [31 - 24] Reserved.
	// 
	// Use default
	// 

	for (u_int32_t sensorID = 0; sensorID < 2; sensorID++) {
		u_int32_t fusedTempValueAt70 = chipid_get_fused_pmgr_thermal_sensor_cal_70C(sensorID);
		u_int32_t fusedTempValueAt25 = chipid_get_fused_pmgr_thermal_sensor_cal_25C(sensorID);
		u_int32_t tempSlope = 0x100;

		if ((fusedTempValueAt25 == 0) || (fusedTempValueAt70 == 0)) {
			if (chipid_valid_thermal_sensor_cal_data_expected()) {
				panic("SoC temperature sensor calibration data not found");
			}
		}

		// Should probably make sure we don't divide by zero.
		if (fusedTempValueAt25 == fusedTempValueAt70) {
			fusedTempValueAt70 = 70;
			fusedTempValueAt25 = 25;
			dprintf(DEBUG_INFO, "Invalid soc thermal fuse values\n");
		}

		// 45 = 70 - 25
		tempSlope = (45*256) / (fusedTempValueAt70 - fusedTempValueAt25);

		// Calculate the real offset.
		u_int32_t realOffset = 25 - ((tempSlope * fusedTempValueAt25) / 256);

		// Bits           23                  16
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

	// Per 13799299, enable the temperature sensors after applying calibration efuse info and before enabling DVTM
	rPMGR_THERMAL0_CTL0 |= 0x01;
	rPMGR_THERMAL1_CTL0 |= 0x01;
}


void init_sochot(void)
{
	// Majority of work done in kext with EDT configuration
	// Setup GFX clock divider
	rPMGR_GFX_PERF_STATE_SOCHOT = 0x1;
}


void init_ccc_thermal_sensors(void)
{
	// Per 13146352, CCC thermal sensors for B0+ adopt same register interface as PMGR sensors
	rCCC_THRM0_CTL0 |= 0x08;
	rCCC_THRM1_CTL0 |= 0x08;

	// Bits [15 -  0] PWRDN_START 20us wait
	// Bits [31 - 16] PWRDN_GAP Gap between two readings.
	rCCC_THRM0_CTL1 = 0x019401E4;
	rCCC_THRM1_CTL1 = 0x019401E4;

	// Bits [7  -  0] Conv_Cycle, cycles to wait before data is valid.
	// Bits [15 -  8] Enable_Cycle, cycles to wait before adc_en can be deasserted.
	// Bits [23 - 16] Finish_gap, Cycles to wait before adc_en can be reasserted.
	// Bits [31 - 24] Reserved.
	//
	// Use default
	//

	for (u_int32_t sensorID = 0; sensorID < 2; sensorID++) {
		u_int32_t fusedTempValueAt70 = chipid_get_fused_ccc_thermal_sensor_cal_70C(sensorID);
		u_int32_t fusedTempValueAt25 = chipid_get_fused_ccc_thermal_sensor_cal_25C(sensorID);
		u_int32_t tempSlope = 0x100;

		if ((fusedTempValueAt25 == 0) || (fusedTempValueAt70 == 0)) {
			if (chipid_valid_thermal_sensor_cal_data_expected()) {
				panic("CCC temperature sensor calibration data not found");
			}
		}

		// Should probably make sure we don't divide by zero.
		if (fusedTempValueAt25 == fusedTempValueAt70) {
			fusedTempValueAt70 = 70;
			fusedTempValueAt25 = 25;
			dprintf(DEBUG_INFO, "Invalid soc thermal fuse values\n");
		}

		// 45 = 70 - 25
		tempSlope = (45*256) / (fusedTempValueAt70 - fusedTempValueAt25);

		// Calculate the real offset.
		u_int32_t realOffset = 25 - ((tempSlope * fusedTempValueAt25) / 256);

		// Bits           23                  16
		// Temp_OFFSET => [8 bit signed integer] 
		// Bits          9                            0
		// TEMP_SLOPE => [2 bit integer| 8 bit decimal]
		switch (sensorID) {
			case 0:
				rCCC_THRM0_CTL3 = (realOffset & 0xFF) << 16;
				rCCC_THRM0_CTL3 |= tempSlope  & 0x3FF;
				break;
			case 1:
				rCCC_THRM1_CTL3 = (realOffset & 0xFF) << 16;
				rCCC_THRM1_CTL3 |= tempSlope  & 0x3FF;
				break;
		}
	}

	// Per 13799299, enable the temperature sensors after applying calibration efuse info and before enabling DVTM
	rCCC_THRM0_CTL0 |= 0x01;
	rCCC_THRM1_CTL0 |= 0x01;
		
}


void init_ccc_sochot(void)
{
	// Setup CCC clock frequency by specifying index into DVFM table.
	rCCC_DVFM_FSHOT_IDX = 0x2;
}

#endif
