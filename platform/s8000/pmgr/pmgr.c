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
#include <drivers/reconfig.h>
#include <platform.h>
#include <platform/pmgr.h>
#include <platform/clocks.h>
#include <platform/gpio.h>
#include <platform/gpiodef.h>
#include <platform/power.h>
#include <platform/timer.h>
#include <platform/tunables.h>
#include <platform/soc/chipid.h>
#include <platform/soc/hwclocks.h>
#include <platform/soc/miu.h>
#include <platform/soc/pmgr.h>
#include <platform/soc/operating_point.h>
#include <platform/soc/dvfmperf.h>
#include <platform/soc/reconfig.h>
#include <sys/boot.h>
#include <target.h>

extern void arm_no_wfe_spin(uint32_t usecs);
extern const struct tunable_chip_struct tunables_pmgr[];
extern const struct tunable_chip_struct tunables_pmgr_product[];

struct clock_config_active {
	uint32_t	clk;
	uint32_t	clock_reg_val;
};

struct clock_source {
	uint32_t	src_clk;
	uint32_t	factor;
};

#define CLOCK_SOURCES_MAX 12

struct clock_config {
	volatile uint32_t	*clock_reg; // CLKCFG Register
	struct clock_source	sources[CLOCK_SOURCES_MAX];    // List of sources
};

struct device_config {
	volatile uint32_t	*ps_reg; // PS Register
};

struct dvfm_data
{
	uint32_t dvfm_state_vmax;
	uint32_t dvfm_state_iboot_cnt;
	uint32_t dvfm_state_vnom;
	uint32_t dvfm_state_vboost;
	uint32_t voltage_states1_count;
	uint32_t voltage_states1_size;
};

#define PLL_FREQ_TARGET(pllx)	(((pllx##_O) * (pllx##_M)) / (pllx##_P) / ((pllx##_S) + (pllx != PLL_PCIE ? 1 : (pllx##_S))))

#if APPLICATION_SECUREROM
static uint32_t active_state = kDVFM_STATE_SECUREROM;
#endif

#if APPLICATION_IBOOT
static uint32_t active_state = kDVFM_STATE_IBOOT;
#endif

#if APPLICATION_SECUREROM
#define SOC_PERF_STATE_ACTIVE kSOC_PERF_STATE_SECUREROM
#endif

#if APPLICATION_IBOOT
#define SOC_PERF_STATE_ACTIVE kSOC_PERF_STATE_VMIN
#endif

struct pmgr_soc_perf_state_src_sel {
	uint16_t clk_index;
	uint8_t shift;
	uint8_t entry;
	uint32_t mask;
};

struct pmgr_soc_perf_state_src_sel pmgr_soc_perf_state_src_sels[] = {
#if SUB_PLATFORM_S8000 || SUB_PLATFORM_S8003
	{PMGR_CLK_VENC, PMGR_SOC_PERF_STATE_ENTRY_0A_VENC_SRC_SEL_SHIFT, 'A', PMGR_SOC_PERF_STATE_ENTRY_0A_VENC_SRC_SEL_UMASK},
	{PMGR_CLK_VDEC, PMGR_SOC_PERF_STATE_ENTRY_0A_VDEC_SRC_SEL_SHIFT, 'A', PMGR_SOC_PERF_STATE_ENTRY_0A_VDEC_SRC_SEL_UMASK},
	{PMGR_CLK_ISP, PMGR_SOC_PERF_STATE_ENTRY_0A_ISP_SRC_SEL_SHIFT, 'A', PMGR_SOC_PERF_STATE_ENTRY_0A_ISP_SRC_SEL_UMASK},
	{PMGR_CLK_ISP_C, PMGR_SOC_PERF_STATE_ENTRY_0A_ISP_C_SRC_SEL_SHIFT, 'A', PMGR_SOC_PERF_STATE_ENTRY_0A_ISP_C_SRC_SEL_UMASK},
	{PMGR_CLK_SBR, PMGR_SOC_PERF_STATE_ENTRY_0A_SBR_SRC_SEL_SHIFT, 'A', PMGR_SOC_PERF_STATE_ENTRY_0A_SBR_SRC_SEL_UMASK},
	{PMGR_CLK_AF, PMGR_SOC_PERF_STATE_ENTRY_0A_AF_SRC_SEL_SHIFT, 'A', PMGR_SOC_PERF_STATE_ENTRY_0A_AF_SRC_SEL_UMASK},
	{PMGR_CLK_MCU_REF, PMGR_SOC_PERF_STATE_ENTRY_0A_MCU_REF_SRC_SEL_SHIFT, 'A', PMGR_SOC_PERF_STATE_ENTRY_0A_MCU_REF_SRC_SEL_UMASK},
	{PMGR_CLK_PMP, PMGR_SOC_PERF_STATE_ENTRY_0B_PMP_SRC_SEL_SHIFT, 'B', PMGR_SOC_PERF_STATE_ENTRY_0B_PMP_SRC_SEL_UMASK},
	{PMGR_CLK_DISP0, PMGR_SOC_PERF_STATE_ENTRY_0B_DISP0_SRC_SEL_SHIFT, 'B', PMGR_SOC_PERF_STATE_ENTRY_0B_DISP0_SRC_SEL_UMASK},
	{PMGR_CLK_VID0, PMGR_SOC_PERF_STATE_ENTRY_0B_VID0_SRC_SEL_SHIFT, 'B', PMGR_SOC_PERF_STATE_ENTRY_0B_VID0_SRC_SEL_UMASK},
	{PMGR_CLK_SIO_C, PMGR_SOC_PERF_STATE_ENTRY_0B_SIO_C_SRC_SEL_SHIFT, 'B', PMGR_SOC_PERF_STATE_ENTRY_0B_SIO_C_SRC_SEL_UMASK},
	{PMGR_CLK_GFX_FENDER, PMGR_SOC_PERF_STATE_ENTRY_0B_GFX_FENDER_SRC_SEL_SHIFT, 'B', PMGR_SOC_PERF_STATE_ENTRY_0B_GFX_FENDER_SRC_SEL_UMASK},
	{PMGR_CLK_MSR, PMGR_SOC_PERF_STATE_ENTRY_0B_MSR_SRC_SEL_SHIFT, 'B', PMGR_SOC_PERF_STATE_ENTRY_0B_MSR_SRC_SEL_UMASK},
	{PMGR_CLK_AJPEG_WRAP, PMGR_SOC_PERF_STATE_ENTRY_0B_AJPEG_WRAP_SRC_SEL_SHIFT, 'B', PMGR_SOC_PERF_STATE_ENTRY_0B_AJPEG_WRAP_SRC_SEL_UMASK},
	{PMGR_CLK_AJPEG_IP, PMGR_SOC_PERF_STATE_ENTRY_0B_AJPEG_IP_SRC_SEL_SHIFT, 'B', PMGR_SOC_PERF_STATE_ENTRY_0B_AJPEG_IP_SRC_SEL_UMASK},
	{PMGR_CLK_SEP, PMGR_SOC_PERF_STATE_ENTRY_0C_SEP_SRC_SEL_SHIFT, 'C', PMGR_SOC_PERF_STATE_ENTRY_0C_SEP_SRC_SEL_UMASK},
#elif SUB_PLATFORM_S8001
	{PMGR_CLK_VENC, PMGR_SOC_PERF_STATE_ENTRY_0A_VENC_SRC_SEL_SHIFT, 'A', PMGR_SOC_PERF_STATE_ENTRY_0A_VENC_SRC_SEL_UMASK},
	{PMGR_CLK_VDEC, PMGR_SOC_PERF_STATE_ENTRY_0A_VDEC_SRC_SEL_SHIFT, 'A', PMGR_SOC_PERF_STATE_ENTRY_0A_VDEC_SRC_SEL_UMASK},
	{PMGR_CLK_ISP, PMGR_SOC_PERF_STATE_ENTRY_0A_ISP_SRC_SEL_SHIFT, 'A', PMGR_SOC_PERF_STATE_ENTRY_0A_ISP_SRC_SEL_UMASK},
	{PMGR_CLK_ISP_C, PMGR_SOC_PERF_STATE_ENTRY_0A_ISP_C_SRC_SEL_SHIFT, 'A', PMGR_SOC_PERF_STATE_ENTRY_0A_ISP_C_SRC_SEL_UMASK},
	{PMGR_CLK_SBR, PMGR_SOC_PERF_STATE_ENTRY_0A_SBR_SRC_SEL_SHIFT, 'A', PMGR_SOC_PERF_STATE_ENTRY_0A_SBR_SRC_SEL_UMASK},
	{PMGR_CLK_AF, PMGR_SOC_PERF_STATE_ENTRY_0A_AF_SRC_SEL_SHIFT, 'A', PMGR_SOC_PERF_STATE_ENTRY_0A_AF_SRC_SEL_UMASK},
	{PMGR_CLK_MCU_REF, PMGR_SOC_PERF_STATE_ENTRY_0A_MCU_REF_SRC_SEL_SHIFT, 'A', PMGR_SOC_PERF_STATE_ENTRY_0A_MCU_REF_SRC_SEL_UMASK},
	{PMGR_CLK_PMP, PMGR_SOC_PERF_STATE_ENTRY_0B_PMP_SRC_SEL_SHIFT, 'B', PMGR_SOC_PERF_STATE_ENTRY_0B_PMP_SRC_SEL_UMASK},
	{PMGR_CLK_DISP0, PMGR_SOC_PERF_STATE_ENTRY_0B_DISP0_SRC_SEL_SHIFT, 'B', PMGR_SOC_PERF_STATE_ENTRY_0B_DISP0_SRC_SEL_UMASK},
	{PMGR_CLK_SIO_C, PMGR_SOC_PERF_STATE_ENTRY_0B_SIO_C_SRC_SEL_SHIFT, 'B', PMGR_SOC_PERF_STATE_ENTRY_0B_SIO_C_SRC_SEL_UMASK},
	{PMGR_CLK_GFX_FENDER, PMGR_SOC_PERF_STATE_ENTRY_0B_GFX_FENDER_SRC_SEL_SHIFT, 'B', PMGR_SOC_PERF_STATE_ENTRY_0B_GFX_FENDER_SRC_SEL_UMASK},
	{PMGR_CLK_MSR, PMGR_SOC_PERF_STATE_ENTRY_0B_MSR_SRC_SEL_SHIFT, 'B', PMGR_SOC_PERF_STATE_ENTRY_0B_MSR_SRC_SEL_UMASK},
	{PMGR_CLK_AJPEG_WRAP, PMGR_SOC_PERF_STATE_ENTRY_0B_AJPEG_WRAP_SRC_SEL_SHIFT, 'B', PMGR_SOC_PERF_STATE_ENTRY_0B_AJPEG_WRAP_SRC_SEL_UMASK},
	{PMGR_CLK_AJPEG_IP, PMGR_SOC_PERF_STATE_ENTRY_0B_AJPEG_IP_SRC_SEL_SHIFT, 'B', PMGR_SOC_PERF_STATE_ENTRY_0B_AJPEG_IP_SRC_SEL_UMASK},
	{PMGR_CLK_SRS, PMGR_SOC_PERF_STATE_ENTRY_0C_SRS_SRC_SEL_SHIFT, 'C', PMGR_SOC_PERF_STATE_ENTRY_0C_SRS_SRC_SEL_UMASK},
	{PMGR_CLK_DISP1, PMGR_SOC_PERF_STATE_ENTRY_0C_DISP1_SRC_SEL_SHIFT, 'C', PMGR_SOC_PERF_STATE_ENTRY_0C_DISP1_SRC_SEL_UMASK},
	{PMGR_CLK_SEP, PMGR_SOC_PERF_STATE_ENTRY_0C_SEP_SRC_SEL_SHIFT, 'C', PMGR_SOC_PERF_STATE_ENTRY_0C_SEP_SRC_SEL_UMASK},
#endif
};

struct pmgr_soc_perf_state {
	uint32_t	entry[4];
};

static struct pmgr_soc_perf_state pmgr_soc_perf_states[] = {
	[kSOC_PERF_STATE_BYPASS] = {
		{0x00000000,
		 0x00000000,
		 0x00000000,
		 0x00000000}},
	
#if APPLICATION_SECUREROM
	[kSOC_PERF_STATE_SECUREROM] = {
		{
#if SUB_PLATFORM_S8000 || SUB_PLATFORM_S8003
			// SBR		= 266.66 / 2
			// AF		= 360 / 2
		0x00008800,
			// PMP		= 200 / 2
			// SIO_C	= 400 / 2
		 0x60070000,

#elif SUB_PLATFORM_S8001
			// SBR          = 400 / 2
			// AF           = 600 / 2
		0x00005500,
			// PMP          = 200 / 2
			// SIO_C        = 600 / 2
		0x60050000,
#endif

		// SEP			= 480 / 2
		 0x00000005,
		 0x00000000}},
#endif

#if APPLICATION_IBOOT
#if SUB_PLATFORM_S8000 || SUB_PLATFORM_S8003
	[kSOC_PERF_STATE_VMIN] = {
		// VENC 		= 360
		// VDEC 		= 266.66
		// ISP 			= 320
		// ISP_C 		= 533.33
		// SBR 			= 266.66
		// AF 			= 360
		// MCU_REF_CLK: Have to use bucket 3 and 50 MHz until dcs_init initializes SPLL, otherwise leads to a hang waiting for handshake
		// MCU_REF_CFG		= 0x3
		// MCU_REF_CLK		= 50
		{0x88878838,

		// PMP			= 200
		// DISP0		= 360
		// VID0			= TARGET_VID0_SRC_SEL / 100MHz
		// SIO_C		= 400
		// GFX_FENDER		= 360
		// MSR			= 320
		// AJPEG_WRAP		= 266.66
		// AJPEG_IP		= 200
#if TARGET_VID0_SRC_SEL
		 0x67078878 | PMGR_SOC_PERF_STATE_ENTRY_VID0(TARGET_VID0_SRC_SEL),
#else
		 0x67A78878,
#endif

		// BIU div low
		// SEP			= 480
		 0x00000005,		
		 0x00000000}},
	
	[kSOC_PERF_STATE_VNOM] = {
		// VENC 		= 533.33
		// VDEC 		= 400
		// ISP 			= 457.14
		// ISP_C 		= 800
		// SBR 			= 400
		// AF 			= 533.33
		// MCU_REF_CFG		= 0x0
		// MCU_REF_CLK		= 100
		{0x55555507,

		// PMP			= 200
		// DISP0		= 480
		// VID0			= TARGET_VID0_SRC_SEL / 100MHz
		// SIO_C		= 533.33
		// GFX_FENDER		= 533.33
		// MSR			= 480
		// AJPEG_WRAP		= 360
		// AJPEG_IP		= 288
#if TARGET_VID0_SRC_SEL
		 0x65055555 | PMGR_SOC_PERF_STATE_ENTRY_VID0(TARGET_VID0_SRC_SEL),
#else
		 0x65A55555,
#endif

		// BIU div high
		// SEP			= 480
		 0x00000015,
		 0x00000000}},
#elif SUB_PLATFORM_S8001
	[kSOC_PERF_STATE_VMIN] = {
		// VENC                 = 533.33
		// VDEC                 = 400
		// ISP                  = 457.14
		// ISP_C                = 800
		// SBR                  = 400
		// AF                   = 600
		// MCU_REF_CLK: Have to use bucket 3 and 50 MHz until dcs_init initializes SPLL, otherwise leads to a hang waiting for handshake
		// MCU_REF_CFG          = 0x3
		// MCU_REF_CLK          = 50
		{0x55555538,

		// PMP                  = 200
		// DISP0                = 533.33
		// SIO_C                = 600
		// GFX_FENDER           = 600
		// MSR                  = 480
		// AJPEG_WRAP           = 360
		// AJPEG_IP             = 288
		0x65055555,

		// BIU div high
		// SRS					= 666
		// SEP                  = 480
		// DISP1		= 533.33
		0x00001555,
		0x00000000}},
	// Same as kSOC_PERF_STATE_VMIN
	[kSOC_PERF_STATE_VNOM] = {
		{0x00000000,
		 0x00000000,
		 0x00000000,
		 0x00000000}},
#endif
#endif
};

#if APPLICATION_SECUREROM
static uint32_t perf_level = kPerformanceHigh;
#endif

#if APPLICATION_IBOOT
static uint32_t perf_level = kPerformanceMemoryMid;
#endif

#if APPLICATION_IBOOT

#if SUB_PLATFORM_S8000
/* LPO @192MHz */
#define LPO_T		LPO_FREQ

/* PLL0 @1600MHz */
#define PLL0		0
#define PLL0_O		OSC_FREQ
#define PLL0_P		3
#define PLL0_M		200
#define PLL0_S		0
#define PLL0_T		PLL_FREQ_TARGET(PLL0)

/* PLL1 @1440MHz */
#define PLL1		1
#define PLL1_O		OSC_FREQ
#define PLL1_P		1
#define PLL1_M		60
#define PLL1_S		0
#define PLL1_T		PLL_FREQ_TARGET(PLL1)

/* PLL2 @1200MHz */
#define PLL2		2
#ifndef PLL2_T
#define PLL2_O		OSC_FREQ
#define PLL2_P		1
#define PLL2_M		50
#define PLL2_S		0
#define PLL2_T		PLL_FREQ_TARGET(PLL2)
#endif

/* PLL3 @99MHz */
#define PLL3		3
#define PLL3_O		OSC_FREQ
#define PLL3_P		1
#define PLL3_M		66
#define PLL3_S		15
#define PLL3_T		PLL_FREQ_TARGET(PLL3)

/* PLL_PCIE 6 @100MHz */
#define PLL_PCIE	6
#define PLL_PCIE_O	OSC_FREQ
#define PLL_PCIE_P	1	
#define PLL_PCIE_M	200
#define PLL_PCIE_S	24
#define PLL_PCIE_T	PLL_FREQ_TARGET(PLL_PCIE)

#elif SUB_PLATFORM_S8001

// Values defined in:
// https://seg-docs.ecs.apple.com/projects/elba//release/specs/Apple/PLL/asg_socpll_spec.pdf

/* LPO @192MHz */
#define LPO_T		LPO_FREQ

/* PLL0 @1600MHz */
#define PLL0		0
#define PLL0_O		OSC_FREQ
#define PLL0_P		3
#define PLL0_M		200
#define PLL0_S		0
#define PLL0_T		PLL_FREQ_TARGET(PLL0)

/* PLL1 @1440MHz */
#define PLL1		1
#define PLL1_O		OSC_FREQ
#define PLL1_P		1
#define PLL1_M		60
#define PLL1_S		0
#define PLL1_T		PLL_FREQ_TARGET(PLL1)

/* PLL2 @1200MHz */
#define PLL2		2
#ifndef PLL2_T
#define PLL2_O		OSC_FREQ
#define PLL2_P		1
#define PLL2_M		50
#define PLL2_S		0
#define PLL2_T		PLL_FREQ_TARGET(PLL2)
#endif

/* PLL3 @132.33MHz */
#define PLL3		3
#define PLL3_O		OSC_FREQ
#define PLL3_P		6
#define PLL3_M		397
#define PLL3_S		11
#define PLL3_T		PLL_FREQ_TARGET(PLL3)

/* PLL6 @600Mhz */
#define PLL6		6
#define PLL6_O		OSC_FREQ
#define PLL6_P		1
#define PLL6_M		50
#define PLL6_S		1
#define PLL6_T		PLL_FREQ_TARGET(PLL6)

/* PLL7 @666Mhz */
#define PLL7		7
#define PLL7_O		OSC_FREQ
#define PLL7_P		2
#define PLL7_M		111
#define PLL7_S		1
#define PLL7_T		PLL_FREQ_TARGET(PLL7)

/* PLL_PCIE (8) @100MHz */
/* Taken from PMGR_PLL_PCIE_CTL default */
#define PLL_PCIE	8
#define PLL_PCIE_O	OSC_FREQ
#define PLL_PCIE_P	1	
#define PLL_PCIE_M	125
#define PLL_PCIE_S	29
#define PLL_PCIE_T	PLL_FREQ_TARGET(PLL_PCIE)

#elif SUB_PLATFORM_S8003

/* LPO @192MHz */
#define LPO_T		LPO_FREQ

/* PLL0 @1600MHz */
#define PLL0		0
#define PLL0_O		OSC_FREQ
#define PLL0_P		3
#define PLL0_M		200
#define PLL0_S		0
#define PLL0_T		PLL_FREQ_TARGET(PLL0)

/* PLL1 @1440MHz */
#define PLL1		1
#define PLL1_O		OSC_FREQ
#define PLL1_P		1
#define PLL1_M		60
#define PLL1_S		0
#define PLL1_T		PLL_FREQ_TARGET(PLL1)

/* PLL2 @1200MHz */
#define PLL2		2
#define PLL2_O		OSC_FREQ
#define PLL2_P		1
#define PLL2_M		50
#define PLL2_S		0
#define PLL2_T		PLL_FREQ_TARGET(PLL2)

/* PLL3 @99MHz */
#define PLL3            3
#define PLL3_O          OSC_FREQ
#define PLL3_P          1
#define PLL3_M          66
#define PLL3_S          15
#define PLL3_T          PLL_FREQ_TARGET(PLL3)

/* PLL_PCIE (6) @100MHz */
#define PLL_PCIE	6
#define PLL_PCIE_O	OSC_FREQ
#define PLL_PCIE_P	1
#define PLL_PCIE_M	125
#define PLL_PCIE_S	29
#define PLL_PCIE_T	PLL_FREQ_TARGET(PLL_PCIE)

#else
#error "Unknown SUB_PLATFORM"
#endif

#ifndef TARGET_SPARE0_CLK_CFG
#define TARGET_SPARE0_CLK_CFG 0x00000000
#endif

#ifndef TARGET_VID0_CLK_CFG
#if SUB_PLATFORM_S8000 || SUB_PLATFORM_S8003
#define TARGET_VID0_CLK_CFG	0x88100000
#elif SUB_PLATFORM_S8001
#define TARGET_VID0_CLK_CFG	0x86100000
#endif
#endif

static const struct clock_config_active clk_configs_active[] = {
	// Mini-PMGR clocks:
	{PMGR_CLK_AOP,			0x81100000}, // 192 (LPO)
	{PMGR_CLK_UART0,		0x84100000}, // 24 (LPO)
	{PMGR_CLK_UART1,		0x84100000}, // 24 (LPO)
	{PMGR_CLK_UART2,		0x84100000}, // 24 (LPO)
	{PMGR_CLK_AOP_MCA0_M,		0x83100000}, // 24 (LPO)
	{PMGR_CLK_I2CM,			0x83100000}, // 24 (LPO)
	{PMGR_CLK_PROXY_FABRIC,		0x82100000}, // 96 (LPO)
	{PMGR_CLK_PROXY_MCU_REF,	0x81100000}, // 48 (LPO)
	
	// Spares:
	{PMGR_CLK_S0,			TARGET_SPARE0_CLK_CFG}, // TARGET_SPARE0_CLK_CFG
	{PMGR_CLK_S1,			0x00000000}, // 0
	{PMGR_CLK_S2,			0x00000000}, // 0
	{PMGR_CLK_S3,			0x00000000}, // 0
	{PMGR_CLK_ISP_REF0,		0x81000006}, // 48
	{PMGR_CLK_ISP_REF1,		0x81000006}, // 48

	// PMGR clocks:
#if SUB_PLATFORM_S8000 || SUB_PLATFORM_S8003
	{PMGR_CLK_GFX_FENDER,		0x88100000}, // SOC_PERF_STATE
	{PMGR_CLK_MCU_REF,		0x87100000}, // SOC_PERF_STATE
	{PMGR_CLK_PMP,			0x86100000}, // SOC_PERF_STATE
	{PMGR_CLK_TEMP_MIPI_DSI,	0x86100000}, // PLL2
	{PMGR_CLK_NCO_REF0,		0x85100000}, // 800
	{PMGR_CLK_NCO_REF1,		0x85100000}, // 800
	{PMGR_CLK_NCO_ALG0,		0x80100000}, // 24
	{PMGR_CLK_NCO_ALG1,		0x88100000}, // 100
	{PMGR_CLK_HSICPHY_REF_12M,	0x85100000}, // 12
	{PMGR_CLK_USB480_0,		0x85100000}, // 480
	{PMGR_CLK_USB480_1,		0x85100000}, // 480
	{PMGR_CLK_USB_OHCI_48M,		0x85100000}, // 48
	{PMGR_CLK_USB,			0x85100000}, // 120
	{PMGR_CLK_USB_FREE_60M,		0x85100000}, // 60
	{PMGR_CLK_SIO_C,		0x87100000}, // SOC_PERF_STATE
	{PMGR_CLK_SIO_P,		0x80100000}, // 24
	{PMGR_CLK_ISP_C,		0x87100000}, // SOC_PERF_STATE
	{PMGR_CLK_ISP,			0x88100000}, // SOC_PERF_STATE
	{PMGR_CLK_ISP_SENSOR0_REF,	0x81100000}, // ISP_REF0
	{PMGR_CLK_ISP_SENSOR1_REF,	0x82100000}, // ISP_REF1
	{PMGR_CLK_VDEC,			0x88100000}, // SOC_PERF_STATE
	{PMGR_CLK_VENC,			0x88100000}, // SOC_PERF_STATE
	{PMGR_CLK_VID0,			TARGET_VID0_CLK_CFG}, // SOC_PERF_STATE
	{PMGR_CLK_DISP0,                0x87100000}, // SOC_PERF_STATE
	{PMGR_CLK_AJPEG_IP,		0x88100000}, // SOC_PERF_STATE
	{PMGR_CLK_AJPEG_WRAP,		0x87100000}, // SOC_PERF_STATE
	{PMGR_CLK_MSR,                  0x88100000}, // SOC_PERF_STATE
	{PMGR_CLK_AF,                   0x88100000}, // SOC_PERF_STATE
	{PMGR_CLK_SBR,			0x88100000}, // SOC_PERF_STATE
	{PMGR_CLK_MCA0_M,		0x85100000}, // NCO0
	{PMGR_CLK_MCA1_M,		0x86100000}, // NCO1
	{PMGR_CLK_MCA2_M,		0x87100000}, // NCO2
	{PMGR_CLK_MCA3_M,		0x88100000}, // NCO3
	{PMGR_CLK_MCA4_M,		0x89100000}, // NCO4
	{PMGR_CLK_SEP,			0x85100000}, // SOC_PERF_STATE
	{PMGR_CLK_GPIO,			0x85100000}, // 50
	{PMGR_CLK_SPI0_N,		0x85100000}, // 61.54
	{PMGR_CLK_SPI1_N,		0x85100000}, // 61.54
	{PMGR_CLK_SPI2_N,		0x85100000}, // 61.54
	{PMGR_CLK_SPI3_N,		0x85100000}, // 61.54
	{PMGR_CLK_PCIE_APP,		0x87100000}, // 266.66
	{PMGR_CLK_TMPS,			0x85100000}, // 1.2
	{PMGR_CLK_UVD,			0x86100000}, // 533.33
#elif SUB_PLATFORM_S8001
	{PMGR_CLK_GFX_FENDER,		0x85100000}, // SOC_PERF_STATE
	{PMGR_CLK_MCU_REF,		0x86100000}, // SOC_PERF_STATE
	{PMGR_CLK_PMP,			0x85100000}, // SOC_PERF_STATE
	{PMGR_CLK_NCO_REF0,		0x85100000}, // 800
	{PMGR_CLK_NCO_REF1,		0x85100000}, // 800
	{PMGR_CLK_NCO_ALG0,		0x80100000}, // 24
	{PMGR_CLK_NCO_ALG1,		0x88100000}, // 100
	{PMGR_CLK_HSICPHY_REF_12M,	0x85100000}, // 12
	{PMGR_CLK_USB480_0,		0x85100000}, // 480
	{PMGR_CLK_USB_OHCI_48M,		0x85100000}, // 48
	{PMGR_CLK_USB,			0x88100000}, // 48
	{PMGR_CLK_USB_FREE_60M,		0x85100000}, // 60
	{PMGR_CLK_SIO_C,		0x85100000}, // SOC_PERF_STATE
	{PMGR_CLK_SIO_P,		0x80100000}, // 24
	{PMGR_CLK_ISP_C,		0x85100000}, // SOC_PERF_STATE
	{PMGR_CLK_ISP,			0x85100000}, // SOC_PERF_STATE
	{PMGR_CLK_ISP_SENSOR0_REF,	0x81100000}, // ISP_REF0
	{PMGR_CLK_ISP_SENSOR1_REF,	0x82100000}, // ISP_REF1
	{PMGR_CLK_VDEC,			0x85100000}, // SOC_PERF_STATE
	{PMGR_CLK_VENC,			0x85100000}, // SOC_PERF_STATE
	{PMGR_CLK_VID0,			TARGET_VID0_CLK_CFG}, // SOC_PERF_STATE
	{PMGR_CLK_DISP0,                0x85100000}, // SOC_PERF_STATE
	{PMGR_CLK_AJPEG_IP,		0x85100000}, // SOC_PERF_STATE
	{PMGR_CLK_AJPEG_WRAP,		0x85100000}, // SOC_PERF_STATE
	{PMGR_CLK_MSR,                  0x85100000}, // SOC_PERF_STATE
	{PMGR_CLK_AF,                   0x85100000}, // SOC_PERF_STATE
	{PMGR_CLK_SBR,			0x85100000}, // SOC_PERF_STATE
	{PMGR_CLK_MCA0_M,		0x85100000}, // NCO0
	{PMGR_CLK_MCA1_M,		0x86100000}, // NCO1
	{PMGR_CLK_MCA2_M,		0x87100000}, // NCO2
	{PMGR_CLK_MCA3_M,		0x88100000}, // NCO3
	{PMGR_CLK_MCA4_M,		0x89100000}, // NCO4
	{PMGR_CLK_SEP,			0x85100000}, // SOC_PERF_STATE
	{PMGR_CLK_GPIO,			0x85100000}, // 50
	{PMGR_CLK_SPI0_N,		0x85100000}, // 61.54
	{PMGR_CLK_SPI1_N,		0x85100000}, // 61.54
	{PMGR_CLK_SPI2_N,		0x85100000}, // 61.54
	{PMGR_CLK_SPI3_N,		0x85100000}, // 61.54
	{PMGR_CLK_PCIE_APP,		0x85100000}, // 266.66
	{PMGR_CLK_TMPS,			0x85100000}, // 1.2
	{PMGR_CLK_UVD,			0x85100000}, // 600
	{PMGR_CLK_VID1,			0x86101000}, // 600
	{PMGR_CLK_DISP1,		0x85100000}, // 533.33
	{PMGR_CLK_SRS,			0x85100000}, // 666
#endif
};

static void set_gfx_perf_state(uint32_t state_num, enum chipid_voltage_index voltage_index);
#endif /* APPLICATION_IBOOT */

#if APPLICATION_SECUREROM

#if SUB_PLATFORM_S8000

/* PLL0 @800MHz */
#define PLL0		0
#define PLL0_O		OSC_FREQ
#define PLL0_P		3
#define PLL0_M		100
#define PLL0_S		0
#define PLL0_T		PLL_FREQ_TARGET(PLL0)

/* PLL1 @720MHz */
#define PLL1		1
#define PLL1_O		OSC_FREQ
#define PLL1_P		1
#define PLL1_M		180
#define PLL1_S		1
#define PLL1_T		PLL_FREQ_TARGET(PLL1)

/* PLL_PCIE (6) @100MHz */
#define PLL_PCIE	6
#define PLL_PCIE_O	OSC_FREQ
#define PLL_PCIE_P	1	
#define PLL_PCIE_M	200
#define PLL_PCIE_S	24
#define PLL_PCIE_T	PLL_FREQ_TARGET(PLL_PCIE)

#elif SUB_PLATFORM_S8001

// Values defined in:
// https://seg-docs.ecs.apple.com/projects/elba//release/specs/Apple/PLL/asg_socpll_spec.pdf

/* PLL0 @800MHz */
#define PLL0		0
#define PLL0_O		OSC_FREQ
#define PLL0_P		3
#define PLL0_M		100
#define PLL0_S		0
#define PLL0_T		PLL_FREQ_TARGET(PLL0)

/* PLL1 @720MHz */
#define PLL1		1
#define PLL1_O		OSC_FREQ
#define PLL1_P		1
#define PLL1_M		60
#define PLL1_S		1
#define PLL1_T		PLL_FREQ_TARGET(PLL1)

/* PLL6 @300Mhz */
#define PLL6		6
#define PLL6_O		OSC_FREQ
#define PLL6_P		1
#define PLL6_M		50
#define PLL6_S		3
#define PLL6_T		PLL_FREQ_TARGET(PLL6)

/* PLL_PCIE (8) @100MHz */
/* Taken from PMGR_PLL_PCIE_CTL default */
#define PLL_PCIE	8
#define PLL_PCIE_O	OSC_FREQ
#define PLL_PCIE_P	1	
#define PLL_PCIE_M	125
#define PLL_PCIE_S	29
#define PLL_PCIE_T	PLL_FREQ_TARGET(PLL_PCIE)

#elif SUB_PLATFORM_S8003

/* PLL0 @800MHz */
#define PLL0		0
#define PLL0_O		OSC_FREQ
#define PLL0_P		3
#define PLL0_M		100
#define PLL0_S		0
#define PLL0_T		PLL_FREQ_TARGET(PLL0)

/* PLL1 @720MHz */
#define PLL1		1
#define PLL1_O		OSC_FREQ
#define PLL1_P		1
#define PLL1_M		60
#define PLL1_S		1
#define PLL1_T		PLL_FREQ_TARGET(PLL1)

/* PLL_PCIE (6) @100MHz */
#define PLL_PCIE	6
#define PLL_PCIE_O	OSC_FREQ
#define PLL_PCIE_P	1
#define PLL_PCIE_M	125
#define PLL_PCIE_S	29
#define PLL_PCIE_T	PLL_FREQ_TARGET(PLL_PCIE)

#endif

static const struct clock_config_active clk_configs_active[] = {
	// Mini-PMGR clocks:
	
	// Spares:

	// PMGR clocks:
#if SUB_PLATFORM_S8000 || SUB_PLATFORM_S8003
	{PMGR_CLK_USB,		0x86100000}, // 100 / 2
	{PMGR_CLK_SIO_C,	0x87100000}, // SOC_PERF_STATE
	{PMGR_CLK_SIO_P,	0x85100000}, // 120 / 2
	{PMGR_CLK_AF,		0x88100000}, // SOC_PERF_STATE
	{PMGR_CLK_SBR,		0x88100000}, // SOC_PERF_STATE
	{PMGR_CLK_SEP,		0x85100000}, // SOC_PERF_STATE
	{PMGR_CLK_GPIO,		0x85100000}, // 50 / 2
	{PMGR_CLK_PCIE_APP,	0x87100000}, // 266.66 / 2
#elif SUB_PLATFORM_S8001
	{PMGR_CLK_USB,		0x86100000}, // 100 / 2
	{PMGR_CLK_SIO_C,	0x85100000}, // SOC_PERF_STATE
	{PMGR_CLK_SIO_P,	0x86100000}, // 120 / 2
	{PMGR_CLK_AF,		0x85100000}, // SOC_PERF_STATE
	{PMGR_CLK_SBR,		0x85100000}, // SOC_PERF_STATE
	{PMGR_CLK_SEP,		0x85100000}, // SOC_PERF_STATE
	{PMGR_CLK_GPIO,		0x85100000}, // 50 / 2
	{PMGR_CLK_PCIE_APP,	0x85100000}, // 266.66 / 2
#endif
};

#endif /* APPLICATION_SECUREROM */

static const struct clock_config clk_configs[PMGR_CLK_COUNT] = {
#if SUB_PLATFORM_S8000 || SUB_PLATFORM_S8003
	[PMGR_CLK_GFX_FENDER]    =	{
		&rPMGR_CLK_CFG(GFX_FENDER),
		{
			{ PMGR_CLK_OSC,	1 },
			{ PMGR_CLK_S0,	1 },
			{ PMGR_CLK_S1,	1 },
			{ PMGR_CLK_S2,	1 },
			{ PMGR_CLK_S3,	1 },
			{ PMGR_CLK_PLL0, 3 },
			{ PMGR_CLK_PLL1, 3 },
			{ PMGR_CLK_PLL0, 4 },
			{ PMGR_CLK_PLL1, 4 },
			{ PMGR_CLK_PLL0, 5 },
			{ PMGR_CLK_PLL0, 8 }
		}
	},

	[PMGR_CLK_MCU_REF]    =	{
		&rPMGR_CLK_CFG(MCU_REF),
		{
			{ PMGR_CLK_OSC,	1 },
			{ PMGR_CLK_S0,	1 },
			{ PMGR_CLK_S1,	1 },
			{ PMGR_CLK_S2,	1 },
			{ PMGR_CLK_S3,	1 },
			{ PMGR_CLK_PLL3, 1 },
			{ PMGR_CLK_PLL0, 12 },
			{ PMGR_CLK_PLL0, 16 },
			{ PMGR_CLK_PLL0, 32 },
			{ PMGR_CLK_PROXY_MCU_REF, 1 }
		}
	},

	[PMGR_CLK_PMP]    =	{
		&rPMGR_CLK_CFG(PMP),
		{
			{ PMGR_CLK_OSC,	1 },
			{ PMGR_CLK_S0,	1 },
			{ PMGR_CLK_S1,	1 },
			{ PMGR_CLK_S2,	1 },
			{ PMGR_CLK_S3,	1 },
			{ PMGR_CLK_PLL0, 6 },
			{ PMGR_CLK_PLL0, 8 },
			{ PMGR_CLK_PLL0, 10 },
			{ PMGR_CLK_PLL0, 16 },
			{ PMGR_CLK_PROXY_FABRIC, 1 }
		}
	},

#if SUB_PLATFORM_S8000 || SUB_PLATFORM_S8003
	[PMGR_CLK_TEMP_MIPI_DSI] =	{
		&rPMGR_CLK_CFG(TEMP_MIPI_DSI),
		{
			{ PMGR_CLK_OSC,	1 },
			{ PMGR_CLK_S0,	1 },
			{ PMGR_CLK_S1,	1 },
			{ PMGR_CLK_S2,	1 },
			{ PMGR_CLK_S3,	1 },
			{ PMGR_CLK_OSC,	1 },
			{ PMGR_CLK_PLL2, 1 },
			{ PMGR_CLK_PLL0, 2 }
		}
	},
#endif

	[PMGR_CLK_NCO_REF0]	    =	{
		&rPMGR_CLK_CFG(NCO_REF0),
		{
			{ PMGR_CLK_OSC, 1 },
			{ PMGR_CLK_S0, 1 },
			{ PMGR_CLK_S1, 1 },
			{ PMGR_CLK_S2, 1 },
			{ PMGR_CLK_S3, 1 },
			{ PMGR_CLK_PLL0, 2 },
			{ PMGR_CLK_PLL1, 4 }
		}
	},

	[PMGR_CLK_NCO_REF1]	    =	{
		&rPMGR_CLK_CFG(NCO_REF1),
		{
			{ PMGR_CLK_OSC, 1 },
			{ PMGR_CLK_S0, 1 },
			{ PMGR_CLK_S1, 1 },
			{ PMGR_CLK_S2, 1 },
			{ PMGR_CLK_S3, 1 },
			{ PMGR_CLK_PLL0, 2 },
			{ PMGR_CLK_PLL1, 4 }
		}
	},

	[PMGR_CLK_NCO_ALG0]	    =	{
		&rPMGR_CLK_CFG(NCO_ALG0),
		{
			{ PMGR_CLK_OSC, 1 },
			{ PMGR_CLK_S0, 1 },
			{ PMGR_CLK_S1, 1 },
			{ PMGR_CLK_S2, 1 },
			{ PMGR_CLK_S3, 1 },
			{ PMGR_CLK_PLL1, 6 },
			{ PMGR_CLK_PLL0, 8 },
			{ PMGR_CLK_PLL0, 10 },
			{ PMGR_CLK_PLL0, 16 },
			{ PMGR_CLK_PLL1, 24 },
			{ PMGR_CLK_PLL1, 30 }
		}
	},

	[PMGR_CLK_NCO_ALG1]	    =	{
		&rPMGR_CLK_CFG(NCO_ALG1),
		{
			{ PMGR_CLK_OSC, 1 },
			{ PMGR_CLK_S0, 1 },
			{ PMGR_CLK_S1, 1 },
			{ PMGR_CLK_S2, 1 },
			{ PMGR_CLK_S3, 1 },
			{ PMGR_CLK_PLL1, 6 },
			{ PMGR_CLK_PLL0, 8 },
			{ PMGR_CLK_PLL0, 10 },
			{ PMGR_CLK_PLL0, 16 },
			{ PMGR_CLK_PLL1, 24 },
			{ PMGR_CLK_PLL1, 30 }
		}
	},

	[PMGR_CLK_HSICPHY_REF_12M]	=   {
		&rPMGR_CLK_CFG(HSICPHY_REF_12M),
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
		&rPMGR_CLK_CFG(USB480_0),
		{
			{ PMGR_CLK_OSC, 1 },
			{ PMGR_CLK_S0, 1 },
			{ PMGR_CLK_S1, 1 },
			{ PMGR_CLK_S2, 1 },
			{ PMGR_CLK_S3, 1 },
			{ PMGR_CLK_PLL1, 3 }
		}
	},

#if SUB_PLATFORM_S8000 || SUB_PLATFORM_S8003
	[PMGR_CLK_USB480_1]	    =	{
		&rPMGR_CLK_CFG(USB480_1),
		{
			{ PMGR_CLK_OSC, 1 },
			{ PMGR_CLK_S0, 1 },
			{ PMGR_CLK_S1, 1 },
			{ PMGR_CLK_S2, 1 },
			{ PMGR_CLK_S3, 1 },
			{ PMGR_CLK_PLL1, 3 }
		}
	},
#endif

	[PMGR_CLK_USB_OHCI_48M]	=   {
		&rPMGR_CLK_CFG(USB_OHCI_48M),
		{
			{ PMGR_CLK_OSC, 1 },
			{ PMGR_CLK_S0, 1 },
			{ PMGR_CLK_S1, 1 },
			{ PMGR_CLK_S2, 1 },
			{ PMGR_CLK_S3, 1 },
			{ PMGR_CLK_PLL1, 30 }
		}
	},

	[PMGR_CLK_USB]	    =	{
		&rPMGR_CLK_CFG(USB),
		{
			{ PMGR_CLK_OSC, 1 },
			{ PMGR_CLK_S0, 1 },
			{ PMGR_CLK_S1, 1 },
			{ PMGR_CLK_S2, 1 },
			{ PMGR_CLK_S3, 1 },
			{ PMGR_CLK_PLL1, 12 },
			{ PMGR_CLK_PLL0, 16 },
			{ PMGR_CLK_PLL1, 24 },
			{ PMGR_CLK_PLL1, 30 },
			{ PMGR_CLK_PLL0, 48 },
			{ PMGR_CLK_PLL1, 48 }
		}
	},

	[PMGR_CLK_USB_FREE_60M]	=   {
		&rPMGR_CLK_CFG(USB_FREE_60M),
		{
			{ PMGR_CLK_OSC, 1 },
			{ PMGR_CLK_S0, 1 },
			{ PMGR_CLK_S1, 1 },
			{ PMGR_CLK_S2, 1 },
			{ PMGR_CLK_S3, 1 },
			{ PMGR_CLK_PLL1, 24 }
		}
	},

	[PMGR_CLK_SIO_C]	    =	{
		&rPMGR_CLK_CFG(SIO_C),
		{
			{ PMGR_CLK_OSC, 1 },
			{ PMGR_CLK_S0, 1 },
			{ PMGR_CLK_S1, 1 },
			{ PMGR_CLK_S2, 1 },
			{ PMGR_CLK_S3, 1 },
			{ PMGR_CLK_PLL0, 3 },
			{ PMGR_CLK_PLL1, 3 },
			{ PMGR_CLK_PLL0, 4 },
			{ PMGR_CLK_PLL1, 4 },
			{ PMGR_CLK_PLL0, 5 },
			{ PMGR_CLK_PLL1, 6 },
			{ PMGR_CLK_PLL0, 32 }
		}
	},

	[PMGR_CLK_SIO_P]	    =	{
		&rPMGR_CLK_CFG(SIO_P),
		{
			{ PMGR_CLK_OSC, 1 },
			{ PMGR_CLK_S0, 1 },
			{ PMGR_CLK_S1, 1 },
			{ PMGR_CLK_S2, 1 },
			{ PMGR_CLK_S3, 1 },
			{ PMGR_CLK_PLL1, 12 },
			{ PMGR_CLK_PLL0, 16 },
			{ PMGR_CLK_PLL0, 20 },
			{ PMGR_CLK_PLL1, 24 },
			{ PMGR_CLK_PLL0, 32 },
			{ PMGR_CLK_OSC,	2 },
			{ PMGR_CLK_OSC,	4 }
		}
	},

	[PMGR_CLK_ISP_C]	    =	{
		&rPMGR_CLK_CFG(ISP_C),
		{
			{ PMGR_CLK_OSC, 1 },
			{ PMGR_CLK_S0, 1 },
			{ PMGR_CLK_S1, 1 },
			{ PMGR_CLK_S2, 1 },
			{ PMGR_CLK_S3, 1 },
			{ PMGR_CLK_PLL0, 2 },
			{ PMGR_CLK_PLL1, 2 },
			{ PMGR_CLK_PLL0, 3 },
			{ PMGR_CLK_PLL1, 3 },
			{ PMGR_CLK_PLL0, 4 },
			{ PMGR_CLK_PLL1, 4 }
		}
	},

	[PMGR_CLK_ISP]	    =	{
		&rPMGR_CLK_CFG(ISP),
		{
			{ PMGR_CLK_OSC, 1 },
			{ PMGR_CLK_S0, 1 },
			{ PMGR_CLK_S1, 1 },
			{ PMGR_CLK_S2, 1 },
			{ PMGR_CLK_S3, 1 },
			{ PMGR_CLK_PLL0, 4 }, // 3.5
			{ PMGR_CLK_PLL0, 4 },
			{ PMGR_CLK_PLL1, 4 },
			{ PMGR_CLK_PLL0, 5 },
			{ PMGR_CLK_PLL1, 6 },
			{ PMGR_CLK_PLL0, 8 }
		}
	},

	[PMGR_CLK_ISP_SENSOR0_REF]	=   {
		&rPMGR_CLK_CFG(ISP_SENSOR0_REF),
		{
			{ PMGR_CLK_OSC, 1 },
			{ PMGR_CLK_ISP_REF0, 1 },
			{ PMGR_CLK_ISP_REF1, 1 }
		}
	},

	[PMGR_CLK_ISP_SENSOR1_REF]	=   {
		&rPMGR_CLK_CFG(ISP_SENSOR1_REF),
		{
			{ PMGR_CLK_OSC, 1 },
			{ PMGR_CLK_ISP_REF0, 1 },
			{ PMGR_CLK_ISP_REF1, 1 }
		}
	},

	[PMGR_CLK_VDEC]	    =	{
		&rPMGR_CLK_CFG(VDEC),
		{
			{ PMGR_CLK_OSC, 1 },
			{ PMGR_CLK_S0, 1 },
			{ PMGR_CLK_S1, 1 },
			{ PMGR_CLK_S2, 1 },
			{ PMGR_CLK_S3, 1 },
			{ PMGR_CLK_PLL0, 4 },
			{ PMGR_CLK_PLL1, 4 },
			{ PMGR_CLK_PLL0, 5 },
			{ PMGR_CLK_PLL0, 6 },
			{ PMGR_CLK_PLL1, 6 },
			{ PMGR_CLK_PLL0, 8 },
			{ PMGR_CLK_PLL0, 10 }
		}
	},

	[PMGR_CLK_VENC]	    =	{
		&rPMGR_CLK_CFG(VENC),
		{
			{ PMGR_CLK_OSC, 1 },
			{ PMGR_CLK_S0, 1 },
			{ PMGR_CLK_S1, 1 },
			{ PMGR_CLK_S2, 1 },
			{ PMGR_CLK_S3, 1 },
			{ PMGR_CLK_PLL0, 3 },
			{ PMGR_CLK_PLL1, 3 },
			{ PMGR_CLK_PLL0, 4 },
			{ PMGR_CLK_PLL1, 4 },
			{ PMGR_CLK_PLL0, 5 },
			{ PMGR_CLK_PLL0, 6 },
			{ PMGR_CLK_PLL0, 8 }
		}
	},

	[PMGR_CLK_VID0]	    =	{
		&rPMGR_CLK_CFG(VID0),
		{
			{ PMGR_CLK_OSC, 1 },
			{ PMGR_CLK_S0, 1 },
			{ PMGR_CLK_S1, 1 },
			{ PMGR_CLK_S2, 1 },
			{ PMGR_CLK_S3, 1 },
			{ PMGR_CLK_PLL2, 1 },
			{ PMGR_CLK_PLL1, 5 },
			{ PMGR_CLK_PLL0, 6 },
			{ PMGR_CLK_PLL1, 6 },
			{ PMGR_CLK_PLL0, 8 },
			{ PMGR_CLK_PLL0, 16 },
			{ PMGR_CLK_PLL0, 26 },
		}
	},

	[PMGR_CLK_DISP0]	    =	{
		&rPMGR_CLK_CFG(DISP0),
		{
			{ PMGR_CLK_OSC, 1 },
			{ PMGR_CLK_S0, 1 },
			{ PMGR_CLK_S1, 1 },
			{ PMGR_CLK_S2, 1 },
			{ PMGR_CLK_S3, 1 },
			{ PMGR_CLK_PLL1, 3 },
			{ PMGR_CLK_PLL0, 4 },
			{ PMGR_CLK_PLL1, 4 },
			{ PMGR_CLK_PLL0, 5 },
			{ PMGR_CLK_PLL0, 6 },
			{ PMGR_CLK_PLL0, 8 },
			{ PMGR_CLK_PLL0, 9 }
		}
	},

	[PMGR_CLK_AJPEG_IP]	    =	{
		&rPMGR_CLK_CFG(AJPEG_IP),
		{
			{ PMGR_CLK_OSC, 1 },
			{ PMGR_CLK_S0, 1 },
			{ PMGR_CLK_S1, 1 },
			{ PMGR_CLK_S2, 1 },
			{ PMGR_CLK_S3, 1 },
			{ PMGR_CLK_PLL1, 5 },
			{ PMGR_CLK_PLL0, 6 },
			{ PMGR_CLK_PLL1, 6 },
			{ PMGR_CLK_PLL0, 8 },
			{ PMGR_CLK_PLL0, 9 },
			{ PMGR_CLK_PLL0, 10 },
			{ PMGR_CLK_PLL0, 16 }
		}
	},

	[PMGR_CLK_AJPEG_WRAP]   =	{
		&rPMGR_CLK_CFG(AJPEG_WRAP),
		{
			{ PMGR_CLK_OSC, 1 },
			{ PMGR_CLK_S0, 1 },
			{ PMGR_CLK_S1, 1 },
			{ PMGR_CLK_S2, 1 },
			{ PMGR_CLK_S3, 1 },
			{ PMGR_CLK_PLL1, 4 },
			{ PMGR_CLK_PLL0, 5 },
			{ PMGR_CLK_PLL0, 6 },
			{ PMGR_CLK_PLL1, 6 },
			{ PMGR_CLK_PLL0, 8 },
			{ PMGR_CLK_PLL0, 9 }
		}
	},

	[PMGR_CLK_MSR]	    =	{
		&rPMGR_CLK_CFG(MSR),
		{
			{ PMGR_CLK_OSC, 1 },
			{ PMGR_CLK_S0, 1 },
			{ PMGR_CLK_S1, 1 },
			{ PMGR_CLK_S2, 1 },
			{ PMGR_CLK_S3, 1 },
			{ PMGR_CLK_PLL1, 3 },
			{ PMGR_CLK_PLL0, 4 },
			{ PMGR_CLK_PLL1, 4 },
			{ PMGR_CLK_PLL0, 5 },
			{ PMGR_CLK_PLL0, 6 },
			{ PMGR_CLK_PLL0, 8 },
			{ PMGR_CLK_PLL0, 10 }
		}
	},

	[PMGR_CLK_AF]	    =	{
		&rPMGR_CLK_CFG(AF),
		{
			{ PMGR_CLK_OSC, 1 },
			{ PMGR_CLK_S0, 1 },
			{ PMGR_CLK_S1, 1 },
			{ PMGR_CLK_S2, 1 },
			{ PMGR_CLK_S3, 1 },
			{ PMGR_CLK_PLL0, 3 },
			{ PMGR_CLK_PLL1, 3 },
			{ PMGR_CLK_PLL0, 4 },
			{ PMGR_CLK_PLL1, 4 },
			{ PMGR_CLK_PLL0, 5 },
			{ PMGR_CLK_PLL0, 8 },
			{ PMGR_CLK_PROXY_FABRIC, 1 }
		}
	},

	[PMGR_CLK_SBR]	    =	{
		&rPMGR_CLK_CFG(SBR),
		{
			{ PMGR_CLK_OSC, 1 },
			{ PMGR_CLK_S0, 1 },
			{ PMGR_CLK_S1, 1 },
			{ PMGR_CLK_S2, 1 },
			{ PMGR_CLK_S3, 1 },
			{ PMGR_CLK_PLL0, 4 },
			{ PMGR_CLK_PLL1, 4 },
			{ PMGR_CLK_PLL0, 5 },
			{ PMGR_CLK_PLL0, 6 },
			{ PMGR_CLK_PLL1, 6 },
			{ PMGR_CLK_PLL0, 8 },
			{ PMGR_CLK_PROXY_FABRIC, 1 }
		}
	},

	[PMGR_CLK_MCA0_M]	    =	{
		&rPMGR_CLK_CFG(MCA0_M),
		{
			{ PMGR_CLK_OSC, 1 },
			{ PMGR_CLK_S0, 1 },
			{ PMGR_CLK_S1, 1 },
			{ PMGR_CLK_S2, 1 },
			{ PMGR_CLK_S3, 1 },
			{ PMGR_CLK_NCO_REF0, 1 },
			{ PMGR_CLK_NCO_REF0, 1 },
			{ PMGR_CLK_NCO_REF0, 1 },
			{ PMGR_CLK_NCO_REF0, 1 },
			{ PMGR_CLK_NCO_REF0, 1 },
			{ PMGR_CLK_OSC, 2 },
			{ PMGR_CLK_OSC, 4 }
		}
	},

	[PMGR_CLK_MCA1_M]	    =	{
		&rPMGR_CLK_CFG(MCA1_M),
		{
			{ PMGR_CLK_OSC, 1 },
			{ PMGR_CLK_S0, 1 },
			{ PMGR_CLK_S1, 1 },
			{ PMGR_CLK_S2, 1 },
			{ PMGR_CLK_S3, 1 },
			{ PMGR_CLK_NCO_REF0, 1 },
			{ PMGR_CLK_NCO_REF0, 1 },
			{ PMGR_CLK_NCO_REF0, 1 },
			{ PMGR_CLK_NCO_REF0, 1 },
			{ PMGR_CLK_NCO_REF0, 1 },
			{ PMGR_CLK_OSC, 2 },
			{ PMGR_CLK_OSC, 4 }
		}
	},

	[PMGR_CLK_MCA2_M]	    =	{
		&rPMGR_CLK_CFG(MCA2_M),
		{
			{ PMGR_CLK_OSC, 1 },
			{ PMGR_CLK_S0, 1 },
			{ PMGR_CLK_S1, 1 },
			{ PMGR_CLK_S2, 1 },
			{ PMGR_CLK_S3, 1 },
			{ PMGR_CLK_NCO_REF0, 1 },
			{ PMGR_CLK_NCO_REF0, 1 },
			{ PMGR_CLK_NCO_REF0, 1 },
			{ PMGR_CLK_NCO_REF0, 1 },
			{ PMGR_CLK_NCO_REF0, 1 },
			{ PMGR_CLK_OSC, 2 },
			{ PMGR_CLK_OSC, 4 }
		}
	},

	[PMGR_CLK_MCA3_M]	    =	{
		&rPMGR_CLK_CFG(MCA3_M),
		{
			{ PMGR_CLK_OSC, 1 },
			{ PMGR_CLK_S0, 1 },
			{ PMGR_CLK_S1, 1 },
			{ PMGR_CLK_S2, 1 },
			{ PMGR_CLK_S3, 1 },
			{ PMGR_CLK_NCO_REF0, 1 },
			{ PMGR_CLK_NCO_REF0, 1 },
			{ PMGR_CLK_NCO_REF0, 1 },
			{ PMGR_CLK_NCO_REF0, 1 },
			{ PMGR_CLK_NCO_REF0, 1 },
			{ PMGR_CLK_OSC, 2 },
			{ PMGR_CLK_OSC, 4 }
		}
	},

	[PMGR_CLK_MCA4_M]	    =	{
		&rPMGR_CLK_CFG(MCA4_M),
		{
			{ PMGR_CLK_OSC, 1 },
			{ PMGR_CLK_S0, 1 },
			{ PMGR_CLK_S1, 1 },
			{ PMGR_CLK_S2, 1 },
			{ PMGR_CLK_S3, 1 },
			{ PMGR_CLK_NCO_REF0, 1 },
			{ PMGR_CLK_NCO_REF0, 1 },
			{ PMGR_CLK_NCO_REF0, 1 },
			{ PMGR_CLK_NCO_REF0, 1 },
			{ PMGR_CLK_NCO_REF0, 1 },
			{ PMGR_CLK_OSC, 2 },
			{ PMGR_CLK_OSC, 4 }
		}
	},

	[PMGR_CLK_SEP]	    =	{
		&rPMGR_CLK_CFG(SEP),
		{
			{ PMGR_CLK_OSC, 1 },
			{ PMGR_CLK_S0, 1 },
			{ PMGR_CLK_S1, 1 },
			{ PMGR_CLK_S2, 1 },
			{ PMGR_CLK_S3, 1 },
			{ PMGR_CLK_PLL1, 3 },
			{ PMGR_CLK_PLL0, 4 },
			{ PMGR_CLK_PLL1, 4 },
			{ PMGR_CLK_PLL0, 5 },
			{ PMGR_CLK_PLL0, 8 },
			{ PMGR_CLK_PLL0, 16 }
		}
	},

	[PMGR_CLK_GPIO]	    =	{
		&rPMGR_CLK_CFG(GPIO),
		{
			{ PMGR_CLK_OSC, 1 },
			{ PMGR_CLK_S0, 1 },
			{ PMGR_CLK_S1, 1 },
			{ PMGR_CLK_S2, 1 },
			{ PMGR_CLK_S3, 1 },
			{ PMGR_CLK_PLL0, 32 },
			{ PMGR_CLK_PLL1, 30 },
			{ PMGR_CLK_OSC, 1 }
		}
	},

	[PMGR_CLK_SPI0_N]	    =	{
		&rPMGR_CLK_CFG(SPI0_N),
		{
			{ PMGR_CLK_OSC, 1 },
			{ PMGR_CLK_S0, 1 },
			{ PMGR_CLK_S1, 1 },
			{ PMGR_CLK_S2, 1 },
			{ PMGR_CLK_S3, 1 },
			{ PMGR_CLK_PLL0, 26 },
			{ PMGR_CLK_PLL1, 24 },
			{ PMGR_CLK_PLL0, 32 }
		}
	},

	[PMGR_CLK_SPI1_N]	    =	{
		&rPMGR_CLK_CFG(SPI1_N),
		{
			{ PMGR_CLK_OSC, 1 },
			{ PMGR_CLK_S0, 1 },
			{ PMGR_CLK_S1, 1 },
			{ PMGR_CLK_S2, 1 },
			{ PMGR_CLK_S3, 1 },
			{ PMGR_CLK_PLL0, 26 },
			{ PMGR_CLK_PLL1, 24 },
			{ PMGR_CLK_PLL0, 32 }
		}
	},

	[PMGR_CLK_SPI2_N]	    =	{
		&rPMGR_CLK_CFG(SPI2_N),
		{
			{ PMGR_CLK_OSC, 1 },
			{ PMGR_CLK_S0, 1 },
			{ PMGR_CLK_S1, 1 },
			{ PMGR_CLK_S2, 1 },
			{ PMGR_CLK_S3, 1 },
			{ PMGR_CLK_PLL0, 26 },
			{ PMGR_CLK_PLL1, 24 },
			{ PMGR_CLK_PLL0, 32 }
		}
	},

	[PMGR_CLK_SPI3_N]	    =	{
		&rPMGR_CLK_CFG(SPI3_N),
		{
			{ PMGR_CLK_OSC, 1 },
			{ PMGR_CLK_S0, 1 },
			{ PMGR_CLK_S1, 1 },
			{ PMGR_CLK_S2, 1 },
			{ PMGR_CLK_S3, 1 },
			{ PMGR_CLK_PLL0, 26 },
			{ PMGR_CLK_PLL1, 24 },
			{ PMGR_CLK_PLL0, 32 }
		}
	},

	[PMGR_CLK_PCIE_APP]	=	{
		&rPMGR_CLK_CFG(PCIE_APP),
		{
			{ PMGR_CLK_OSC, 1 },
			{ PMGR_CLK_S0, 1 },
			{ PMGR_CLK_S1, 1 },
			{ PMGR_CLK_S2, 1 },
			{ PMGR_CLK_S3, 1 },
			{ PMGR_CLK_PLL1, 4 },
			{ PMGR_CLK_PLL0, 5 },
			{ PMGR_CLK_PLL0, 6 },
			{ PMGR_CLK_PLL1, 6 },
			{ PMGR_CLK_PLL0, 8 },
			{ PMGR_CLK_PLL0, 16 }
		}
	},

	[PMGR_CLK_TMPS]		=	{
		&rPMGR_CLK_CFG(TMPS),
		{
			{ PMGR_CLK_OSC, 1 },
			{ PMGR_CLK_S0, 1 },
			{ PMGR_CLK_S1, 1 },
			{ PMGR_CLK_S2, 1 },
			{ PMGR_CLK_S3, 1 },
			{ PMGR_CLK_OSC, 20 },
		}
	},

	[PMGR_CLK_UVD]		=	{
		&rPMGR_CLK_CFG(UVD),
		{
			{ PMGR_CLK_OSC, 1 },
			{ PMGR_CLK_S0, 1 },
			{ PMGR_CLK_S1, 1 },
			{ PMGR_CLK_S2, 1 },
			{ PMGR_CLK_S3, 1 },
			{ PMGR_CLK_PLL0, 8 },
			{ PMGR_CLK_PLL0, 3 },
		}
	},
	[PMGR_CLK_S0]	    =	{
		&rPMGR_CLK_CFG(S0),
		{
			{ PMGR_CLK_PLL0, 2 },
			{ PMGR_CLK_PLL1, 2 },
			{ PMGR_CLK_PLL2, 1 },
			{ PMGR_CLK_PLL4, 2 }
		}
	},

	[PMGR_CLK_S1]	    =	{
		&rPMGR_CLK_CFG(S1),
		{
			{ PMGR_CLK_PLL0, 2 },
			{ PMGR_CLK_PLL1, 2 },
			{ PMGR_CLK_PLL2, 1 },
			{ PMGR_CLK_PLL4, 2 }
		}
	},

	[PMGR_CLK_S2]	    =	{
		&rPMGR_CLK_CFG(S2),
		{
			{ PMGR_CLK_PLL0, 2 },
			{ PMGR_CLK_PLL1, 2 },
			{ PMGR_CLK_PLL2, 1 },
			{ PMGR_CLK_PLL4, 2 }
		}
	},

	[PMGR_CLK_S3]	    =	{
		&rPMGR_CLK_CFG(S3),
		{
			{ PMGR_CLK_PLL0, 2 },
			{ PMGR_CLK_PLL1, 2 },
			{ PMGR_CLK_PLL2, 1 },
			{ PMGR_CLK_PLL4, 2 }
		}
	},

	[PMGR_CLK_ISP_REF0]	    =	{
		&rPMGR_CLK_CFG(ISP_REF0),
		{
			{ PMGR_CLK_PLL0, 5 },
			{ PMGR_CLK_PLL1, 5 },
			{ PMGR_CLK_PLL0, 6 },
			{ PMGR_CLK_PLL1, 6 }
		}
	},

	[PMGR_CLK_ISP_REF1]	    =	{
		&rPMGR_CLK_CFG(ISP_REF1),
		{
			{ PMGR_CLK_PLL0, 5 },
			{ PMGR_CLK_PLL1, 5 },
			{ PMGR_CLK_PLL0, 6 },
			{ PMGR_CLK_PLL1, 6 }
		}
	},
#elif SUB_PLATFORM_S8001
	[PMGR_CLK_GFX_FENDER]	=	{
		&rPMGR_CLK_CFG(GFX_FENDER),
		{
			{ PMGR_CLK_OSC, 1 },
			{ PMGR_CLK_S0, 1 },
			{ PMGR_CLK_S1, 1 },
			{ PMGR_CLK_S2, 1 },
			{ PMGR_CLK_S3, 1 },
			{ PMGR_CLK_PLL6, 1 },
			{ PMGR_CLK_PLL0, 3 },
			{ PMGR_CLK_PLL1, 3 },
			{ PMGR_CLK_PLL0, 4 },
			{ PMGR_CLK_PLL1, 4 },
			{ PMGR_CLK_PLL0, 5 },
			{ PMGR_CLK_PLL0, 8 },
		}
	},
	[PMGR_CLK_MCU_REF]	=	{
		&rPMGR_CLK_CFG(MCU_REF),
		{
			{ PMGR_CLK_OSC, 1 },
			{ PMGR_CLK_S0, 1 },
			{ PMGR_CLK_S1, 1 },
			{ PMGR_CLK_S2, 1 },
			{ PMGR_CLK_S3, 1 },
			{ PMGR_CLK_PLL3, 1 },
			{ PMGR_CLK_PLL0, 12 },
			{ PMGR_CLK_PLL0, 16 },
			{ PMGR_CLK_PLL0, 32 },
			{ PMGR_CLK_PROXY_MCU_REF, 1 },
		}
	},
	[PMGR_CLK_PMP]	=	{
		&rPMGR_CLK_CFG(PMP),
		{
			{ PMGR_CLK_OSC, 1 },
			{ PMGR_CLK_S0, 1 },
			{ PMGR_CLK_S1, 1 },
			{ PMGR_CLK_S2, 1 },
			{ PMGR_CLK_S3, 1 },
			{ PMGR_CLK_PLL0, 6 },
			{ PMGR_CLK_PLL0, 8 },
			{ PMGR_CLK_PLL0, 10 },
			{ PMGR_CLK_PLL0, 16 },
			{ PMGR_CLK_PROXY_FABRIC, 1 },
		}
	},
	[PMGR_CLK_NCO_REF0]	=	{
		&rPMGR_CLK_CFG(NCO_REF0),
		{
			{ PMGR_CLK_OSC, 1 },
			{ PMGR_CLK_S0, 1 },
			{ PMGR_CLK_S1, 1 },
			{ PMGR_CLK_S2, 1 },
			{ PMGR_CLK_S3, 1 },
			{ PMGR_CLK_PLL0, 2 },
			{ PMGR_CLK_PLL1, 4 },
		}
	},
	[PMGR_CLK_NCO_REF1]	=	{
		&rPMGR_CLK_CFG(NCO_REF1),
		{
			{ PMGR_CLK_OSC, 1 },
			{ PMGR_CLK_S0, 1 },
			{ PMGR_CLK_S1, 1 },
			{ PMGR_CLK_S2, 1 },
			{ PMGR_CLK_S3, 1 },
			{ PMGR_CLK_PLL0, 2 },
			{ PMGR_CLK_PLL1, 4 },
		}
	},
	[PMGR_CLK_NCO_ALG0]	=	{
		&rPMGR_CLK_CFG(NCO_ALG0),
		{
			{ PMGR_CLK_OSC, 1 },
			{ PMGR_CLK_S0, 1 },
			{ PMGR_CLK_S1, 1 },
			{ PMGR_CLK_S2, 1 },
			{ PMGR_CLK_S3, 1 },
			{ PMGR_CLK_PLL1, 6 },
			{ PMGR_CLK_PLL0, 8 },
			{ PMGR_CLK_PLL0, 10 },
			{ PMGR_CLK_PLL0, 16 },
			{ PMGR_CLK_PLL1, 24 },
			{ PMGR_CLK_PLL1, 30 },
		}
	},
	[PMGR_CLK_NCO_ALG1]	=	{
		&rPMGR_CLK_CFG(NCO_ALG1),
		{
			{ PMGR_CLK_OSC, 1 },
			{ PMGR_CLK_S0, 1 },
			{ PMGR_CLK_S1, 1 },
			{ PMGR_CLK_S2, 1 },
			{ PMGR_CLK_S3, 1 },
			{ PMGR_CLK_PLL1, 6 },
			{ PMGR_CLK_PLL0, 8 },
			{ PMGR_CLK_PLL0, 10 },
			{ PMGR_CLK_PLL0, 16 },
			{ PMGR_CLK_PLL1, 24 },
			{ PMGR_CLK_PLL1, 30 },
		}
	},
	[PMGR_CLK_HSICPHY_REF_12M]	=	{
		&rPMGR_CLK_CFG(HSICPHY_REF_12M),
		{
			{ PMGR_CLK_OSC, 1 },
			{ PMGR_CLK_S0, 1 },
			{ PMGR_CLK_S1, 1 },
			{ PMGR_CLK_S2, 1 },
			{ PMGR_CLK_S3, 1 },
			{ PMGR_CLK_OSC, 2 },
		}
	},
	[PMGR_CLK_USB480_0]	=	{
		&rPMGR_CLK_CFG(USB480_0),
		{
			{ PMGR_CLK_OSC, 1 },
			{ PMGR_CLK_S0, 1 },
			{ PMGR_CLK_S1, 1 },
			{ PMGR_CLK_S2, 1 },
			{ PMGR_CLK_S3, 1 },
			{ PMGR_CLK_PLL1, 3 },
		}
	},
	[PMGR_CLK_USB_OHCI_48M]	=	{
		&rPMGR_CLK_CFG(USB_OHCI_48M),
		{
			{ PMGR_CLK_OSC, 1 },
			{ PMGR_CLK_S0, 1 },
			{ PMGR_CLK_S1, 1 },
			{ PMGR_CLK_S2, 1 },
			{ PMGR_CLK_S3, 1 },
			{ PMGR_CLK_PLL1, 30 },
		}
	},
	[PMGR_CLK_USB]	=	{
		&rPMGR_CLK_CFG(USB),
		{
			{ PMGR_CLK_OSC, 1 },
			{ PMGR_CLK_S0, 1 },
			{ PMGR_CLK_S1, 1 },
			{ PMGR_CLK_S2, 1 },
			{ PMGR_CLK_S3, 1 },
			{ PMGR_CLK_PLL1, 12 },
			{ PMGR_CLK_PLL0, 16 },
			{ PMGR_CLK_PLL1, 24 },
			{ PMGR_CLK_PLL1, 30 },
			{ PMGR_CLK_PLL0, 48 },
			{ PMGR_CLK_PLL1, 48 },
		}
	},
	[PMGR_CLK_USB_FREE_60M]	=	{
		&rPMGR_CLK_CFG(USB_FREE_60M),
		{
			{ PMGR_CLK_OSC, 1 },
			{ PMGR_CLK_S0, 1 },
			{ PMGR_CLK_S1, 1 },
			{ PMGR_CLK_S2, 1 },
			{ PMGR_CLK_S3, 1 },
			{ PMGR_CLK_PLL1, 24 },
		}
	},
	[PMGR_CLK_SIO_C]	=	{
		&rPMGR_CLK_CFG(SIO_C),
		{
			{ PMGR_CLK_OSC, 1 },
			{ PMGR_CLK_S0, 1 },
			{ PMGR_CLK_S1, 1 },
			{ PMGR_CLK_S2, 1 },
			{ PMGR_CLK_S3, 1 },
			{ PMGR_CLK_PLL6, 1 },
			{ PMGR_CLK_PLL0, 3 },
			{ PMGR_CLK_PLL1, 3 },
			{ PMGR_CLK_PLL0, 4 },
			{ PMGR_CLK_PLL0, 5 },
			{ PMGR_CLK_PLL1, 6 },
			{ PMGR_CLK_PLL0, 32 },
		}
	},
	[PMGR_CLK_SIO_P]	=	{
		&rPMGR_CLK_CFG(SIO_P),
		{
			{ PMGR_CLK_OSC, 1 },
			{ PMGR_CLK_S0, 1 },
			{ PMGR_CLK_S1, 1 },
			{ PMGR_CLK_S2, 1 },
			{ PMGR_CLK_S3, 1 },
			{ PMGR_CLK_PLL0, 8 },
			{ PMGR_CLK_PLL1, 12 },
			{ PMGR_CLK_PLL0, 16 },
			{ PMGR_CLK_PLL0, 20 },
			{ PMGR_CLK_PLL1, 24 },
			{ PMGR_CLK_PLL0, 32 },
			{ PMGR_CLK_OSC, 2 },
		}
	},
	[PMGR_CLK_ISP_C]	=	{
		&rPMGR_CLK_CFG(ISP_C),
		{
			{ PMGR_CLK_OSC, 1 },
			{ PMGR_CLK_S0, 1 },
			{ PMGR_CLK_S1, 1 },
			{ PMGR_CLK_S2, 1 },
			{ PMGR_CLK_S3, 1 },
			{ PMGR_CLK_PLL0, 2 },
			{ PMGR_CLK_PLL1, 2 },
			{ PMGR_CLK_PLL0, 3 },
			{ PMGR_CLK_PLL1, 3 },
			{ PMGR_CLK_PLL0, 4 },
			{ PMGR_CLK_PLL1, 4 },
		}
	},
	[PMGR_CLK_ISP]	=	{
		&rPMGR_CLK_CFG(ISP),
		{
			{ PMGR_CLK_OSC, 1 },
			{ PMGR_CLK_S0, 1 },
			{ PMGR_CLK_S1, 1 },
			{ PMGR_CLK_S2, 1 },
			{ PMGR_CLK_S3, 1 },
			{ PMGR_CLK_PLL0, 3 },
			{ PMGR_CLK_PLL0, 4 },
			{ PMGR_CLK_PLL1, 4 },
			{ PMGR_CLK_PLL0, 5 },
			{ PMGR_CLK_PLL1, 6 },
			{ PMGR_CLK_PLL0, 8 },
		}
	},
	[PMGR_CLK_ISP_SENSOR0_REF]	=	{
		&rPMGR_CLK_CFG(ISP_SENSOR0_REF),
		{
			{ PMGR_CLK_OSC, 1 },
			{ PMGR_CLK_ISP_REF0, 1 },
			{ PMGR_CLK_ISP_REF1, 1 },
		}
	},
	[PMGR_CLK_ISP_SENSOR1_REF]	=	{
		&rPMGR_CLK_CFG(ISP_SENSOR1_REF),
		{
			{ PMGR_CLK_OSC, 1 },
			{ PMGR_CLK_ISP_REF0, 1 },
			{ PMGR_CLK_ISP_REF1, 1 },
		}
	},
	[PMGR_CLK_VDEC]	=	{
		&rPMGR_CLK_CFG(VDEC),
		{
			{ PMGR_CLK_OSC, 1 },
			{ PMGR_CLK_S0, 1 },
			{ PMGR_CLK_S1, 1 },
			{ PMGR_CLK_S2, 1 },
			{ PMGR_CLK_S3, 1 },
			{ PMGR_CLK_PLL0, 4 },
			{ PMGR_CLK_PLL1, 4 },
			{ PMGR_CLK_PLL0, 5 },
			{ PMGR_CLK_PLL0, 6 },
			{ PMGR_CLK_PLL1, 6 },
			{ PMGR_CLK_PLL0, 8 },
			{ PMGR_CLK_PLL0, 10 },
		}
	},
	[PMGR_CLK_VENC]	=	{
		&rPMGR_CLK_CFG(VENC),
		{
			{ PMGR_CLK_OSC, 1 },
			{ PMGR_CLK_S0, 1 },
			{ PMGR_CLK_S1, 1 },
			{ PMGR_CLK_S2, 1 },
			{ PMGR_CLK_S3, 1 },
			{ PMGR_CLK_PLL0, 3 },
			{ PMGR_CLK_PLL1, 3 },
			{ PMGR_CLK_PLL0, 4 },
			{ PMGR_CLK_PLL1, 4 },
			{ PMGR_CLK_PLL0, 5 },
			{ PMGR_CLK_PLL0, 6 },
			{ PMGR_CLK_PLL0, 8 },
		}
	},
	[PMGR_CLK_VID0]	=	{
		&rPMGR_CLK_CFG(VID0),
		{
			{ PMGR_CLK_OSC, 1 },
			{ PMGR_CLK_S0, 1 },
			{ PMGR_CLK_S1, 1 },
			{ PMGR_CLK_S2, 1 },
			{ PMGR_CLK_S3, 1 },
			{ PMGR_CLK_PLL2, 1 },
			{ PMGR_CLK_PLL6, 1 },
			{ PMGR_CLK_PLL0, 4 },
			{ PMGR_CLK_PLL1, 4 },
			{ PMGR_CLK_PLL1, 6 },
			{ PMGR_CLK_PLL0, 7 },
			{ PMGR_CLK_PLL0, 8 },
		}
	},
	[PMGR_CLK_DISP0]	=	{
		&rPMGR_CLK_CFG(DISP0),
		{
			{ PMGR_CLK_OSC, 1 },
			{ PMGR_CLK_S0, 1 },
			{ PMGR_CLK_S1, 1 },
			{ PMGR_CLK_S2, 1 },
			{ PMGR_CLK_S3, 1 },
			{ PMGR_CLK_PLL0, 3 },
			{ PMGR_CLK_PLL1, 3 },
			{ PMGR_CLK_PLL0, 4 },
			{ PMGR_CLK_PLL1, 4 },
			{ PMGR_CLK_PLL0, 5 },
			{ PMGR_CLK_PLL0, 6 },
			{ PMGR_CLK_PLL0, 8 },
		}
	},
	[PMGR_CLK_AJPEG_IP]	=	{
		&rPMGR_CLK_CFG(AJPEG_IP),
		{
			{ PMGR_CLK_OSC, 1 },
			{ PMGR_CLK_S0, 1 },
			{ PMGR_CLK_S1, 1 },
			{ PMGR_CLK_S2, 1 },
			{ PMGR_CLK_S3, 1 },
			{ PMGR_CLK_PLL1, 5 },
			{ PMGR_CLK_PLL0, 6 },
			{ PMGR_CLK_PLL1, 6 },
			{ PMGR_CLK_PLL0, 8 },
			{ PMGR_CLK_PLL0, 9 },
			{ PMGR_CLK_PLL0, 10 },
			{ PMGR_CLK_PLL0, 16 },
		}
	},
	[PMGR_CLK_AJPEG_WRAP]	=	{
		&rPMGR_CLK_CFG(AJPEG_WRAP),
		{
			{ PMGR_CLK_OSC, 1 },
			{ PMGR_CLK_S0, 1 },
			{ PMGR_CLK_S1, 1 },
			{ PMGR_CLK_S2, 1 },
			{ PMGR_CLK_S3, 1 },
			{ PMGR_CLK_PLL1, 4 },
			{ PMGR_CLK_PLL0, 5 },
			{ PMGR_CLK_PLL0, 6 },
			{ PMGR_CLK_PLL1, 6 },
			{ PMGR_CLK_PLL0, 8 },
			{ PMGR_CLK_PLL0, 9 },
		}
	},
	[PMGR_CLK_MSR]	=	{
		&rPMGR_CLK_CFG(MSR),
		{
			{ PMGR_CLK_OSC, 1 },
			{ PMGR_CLK_S0, 1 },
			{ PMGR_CLK_S1, 1 },
			{ PMGR_CLK_S2, 1 },
			{ PMGR_CLK_S3, 1 },
			{ PMGR_CLK_PLL1, 3 },
			{ PMGR_CLK_PLL0, 4 },
			{ PMGR_CLK_PLL1, 4 },
			{ PMGR_CLK_PLL0, 5 },
			{ PMGR_CLK_PLL0, 6 },
			{ PMGR_CLK_PLL0, 8 },
			{ PMGR_CLK_PLL0, 10 },
		}
	},
	[PMGR_CLK_AF]	=	{
		&rPMGR_CLK_CFG(AF),
		{
			{ PMGR_CLK_OSC, 1 },
			{ PMGR_CLK_S0, 1 },
			{ PMGR_CLK_S1, 1 },
			{ PMGR_CLK_S2, 1 },
			{ PMGR_CLK_S3, 1 },
			{ PMGR_CLK_PLL6, 1 },
			{ PMGR_CLK_PLL0, 3 },
			{ PMGR_CLK_PLL1, 3 },
			{ PMGR_CLK_PLL0, 4 },
			{ PMGR_CLK_PLL1, 4 },
			{ PMGR_CLK_PLL0, 5 },
			{ PMGR_CLK_PROXY_FABRIC, 1 },
		}
	},
	[PMGR_CLK_SBR]	=	{
		&rPMGR_CLK_CFG(SBR),
		{
			{ PMGR_CLK_OSC, 1 },
			{ PMGR_CLK_S0, 1 },
			{ PMGR_CLK_S1, 1 },
			{ PMGR_CLK_S2, 1 },
			{ PMGR_CLK_S3, 1 },
			{ PMGR_CLK_PLL0, 4 },
			{ PMGR_CLK_PLL1, 4 },
			{ PMGR_CLK_PLL0, 5 },
			{ PMGR_CLK_PLL0, 6 },
			{ PMGR_CLK_PLL1, 6 },
			{ PMGR_CLK_PLL0, 8 },
			{ PMGR_CLK_PROXY_FABRIC, 1 },
		}
	},
	[PMGR_CLK_MCA0_M]	=	{
		&rPMGR_CLK_CFG(MCA0_M),
		{
			{ PMGR_CLK_OSC, 1 },
			{ PMGR_CLK_S0, 1 },
			{ PMGR_CLK_S1, 1 },
			{ PMGR_CLK_S2, 1 },
			{ PMGR_CLK_S3, 1 },
			{ PMGR_CLK_NCO_REF0, 1 },
			{ PMGR_CLK_NCO_REF0, 1 },
			{ PMGR_CLK_NCO_REF0, 1 },
			{ PMGR_CLK_NCO_REF0, 1 },
			{ PMGR_CLK_NCO_REF0, 1 },
			{ PMGR_CLK_OSC, 2 },
			{ PMGR_CLK_OSC, 4 },
		}
	},
	[PMGR_CLK_MCA1_M]	=	{
		&rPMGR_CLK_CFG(MCA1_M),
		{
			{ PMGR_CLK_OSC, 1 },
			{ PMGR_CLK_S0, 1 },
			{ PMGR_CLK_S1, 1 },
			{ PMGR_CLK_S2, 1 },
			{ PMGR_CLK_S3, 1 },
			{ PMGR_CLK_NCO_REF0, 1 },
			{ PMGR_CLK_NCO_REF0, 1 },
			{ PMGR_CLK_NCO_REF0, 1 },
			{ PMGR_CLK_NCO_REF0, 1 },
			{ PMGR_CLK_NCO_REF0, 1 },
			{ PMGR_CLK_OSC, 2 },
			{ PMGR_CLK_OSC, 4 },
		}
	},
	[PMGR_CLK_MCA2_M]	=	{
		&rPMGR_CLK_CFG(MCA2_M),
		{
			{ PMGR_CLK_OSC, 1 },
			{ PMGR_CLK_S0, 1 },
			{ PMGR_CLK_S1, 1 },
			{ PMGR_CLK_S2, 1 },
			{ PMGR_CLK_S3, 1 },
			{ PMGR_CLK_NCO_REF0, 1 },
			{ PMGR_CLK_NCO_REF0, 1 },
			{ PMGR_CLK_NCO_REF0, 1 },
			{ PMGR_CLK_NCO_REF0, 1 },
			{ PMGR_CLK_NCO_REF0, 1 },
			{ PMGR_CLK_OSC, 2 },
			{ PMGR_CLK_OSC, 4 },
		}
	},
	[PMGR_CLK_MCA3_M]	=	{
		&rPMGR_CLK_CFG(MCA3_M),
		{
			{ PMGR_CLK_OSC, 1 },
			{ PMGR_CLK_S0, 1 },
			{ PMGR_CLK_S1, 1 },
			{ PMGR_CLK_S2, 1 },
			{ PMGR_CLK_S3, 1 },
			{ PMGR_CLK_NCO_REF0, 1 },
			{ PMGR_CLK_NCO_REF0, 1 },
			{ PMGR_CLK_NCO_REF0, 1 },
			{ PMGR_CLK_NCO_REF0, 1 },
			{ PMGR_CLK_NCO_REF0, 1 },
			{ PMGR_CLK_OSC, 2 },
			{ PMGR_CLK_OSC, 4 },
		}
	},
	[PMGR_CLK_MCA4_M]	=	{
		&rPMGR_CLK_CFG(MCA4_M),
		{
			{ PMGR_CLK_OSC, 1 },
			{ PMGR_CLK_S0, 1 },
			{ PMGR_CLK_S1, 1 },
			{ PMGR_CLK_S2, 1 },
			{ PMGR_CLK_S3, 1 },
			{ PMGR_CLK_NCO_REF0, 1 },
			{ PMGR_CLK_NCO_REF0, 1 },
			{ PMGR_CLK_NCO_REF0, 1 },
			{ PMGR_CLK_NCO_REF0, 1 },
			{ PMGR_CLK_NCO_REF0, 1 },
			{ PMGR_CLK_OSC, 2 },
			{ PMGR_CLK_OSC, 4 },
		}
	},
	[PMGR_CLK_SEP]	=	{
		&rPMGR_CLK_CFG(SEP),
		{
			{ PMGR_CLK_OSC, 1 },
			{ PMGR_CLK_S0, 1 },
			{ PMGR_CLK_S1, 1 },
			{ PMGR_CLK_S2, 1 },
			{ PMGR_CLK_S3, 1 },
			{ PMGR_CLK_PLL1, 3 },
			{ PMGR_CLK_PLL0, 4 },
			{ PMGR_CLK_PLL1, 4 },
			{ PMGR_CLK_PLL0, 5 },
			{ PMGR_CLK_PLL0, 8 },
			{ PMGR_CLK_PLL0, 16 },
		}
	},
	[PMGR_CLK_GPIO]	=	{
		&rPMGR_CLK_CFG(GPIO),
		{
			{ PMGR_CLK_OSC, 1 },
			{ PMGR_CLK_S0, 1 },
			{ PMGR_CLK_S1, 1 },
			{ PMGR_CLK_S2, 1 },
			{ PMGR_CLK_S3, 1 },
			{ PMGR_CLK_PLL0, 32 },
			{ PMGR_CLK_PLL1, 30 },
			{ PMGR_CLK_OSC, 1 },
		}
	},
	[PMGR_CLK_SPI0_N]	=	{
		&rPMGR_CLK_CFG(SPI0_N),
		{
			{ PMGR_CLK_OSC, 1 },
			{ PMGR_CLK_S0, 1 },
			{ PMGR_CLK_S1, 1 },
			{ PMGR_CLK_S2, 1 },
			{ PMGR_CLK_S3, 1 },
			{ PMGR_CLK_PLL0, 26 },
			{ PMGR_CLK_PLL1, 24 },
			{ PMGR_CLK_PLL0, 32 },
		}
	},
	[PMGR_CLK_SPI1_N]	=	{
		&rPMGR_CLK_CFG(SPI1_N),
		{
			{ PMGR_CLK_OSC, 1 },
			{ PMGR_CLK_S0, 1 },
			{ PMGR_CLK_S1, 1 },
			{ PMGR_CLK_S2, 1 },
			{ PMGR_CLK_S3, 1 },
			{ PMGR_CLK_PLL0, 26 },
			{ PMGR_CLK_PLL1, 24 },
			{ PMGR_CLK_PLL0, 32 },
		}
	},
	[PMGR_CLK_SPI2_N]	=	{
		&rPMGR_CLK_CFG(SPI2_N),
		{
			{ PMGR_CLK_OSC, 1 },
			{ PMGR_CLK_S0, 1 },
			{ PMGR_CLK_S1, 1 },
			{ PMGR_CLK_S2, 1 },
			{ PMGR_CLK_S3, 1 },
			{ PMGR_CLK_PLL0, 26 },
			{ PMGR_CLK_PLL1, 24 },
			{ PMGR_CLK_PLL0, 32 },
		}
	},
	[PMGR_CLK_SPI3_N]	=	{
		&rPMGR_CLK_CFG(SPI3_N),
		{
			{ PMGR_CLK_OSC, 1 },
			{ PMGR_CLK_S0, 1 },
			{ PMGR_CLK_S1, 1 },
			{ PMGR_CLK_S2, 1 },
			{ PMGR_CLK_S3, 1 },
			{ PMGR_CLK_PLL0, 26 },
			{ PMGR_CLK_PLL1, 24 },
			{ PMGR_CLK_PLL0, 32 },
		}
	},
	[PMGR_CLK_PCIE_APP]	=	{
		&rPMGR_CLK_CFG(PCIE_APP),
		{
			{ PMGR_CLK_OSC, 1 },
			{ PMGR_CLK_S0, 1 },
			{ PMGR_CLK_S1, 1 },
			{ PMGR_CLK_S2, 1 },
			{ PMGR_CLK_S3, 1 },
			{ PMGR_CLK_PLL0, 6 },
			{ PMGR_CLK_PLL1, 6 },
			{ PMGR_CLK_PLL0, 8 },
			{ PMGR_CLK_PLL0, 16 },
		}
	},
	[PMGR_CLK_TMPS]	=	{
		&rPMGR_CLK_CFG(TMPS),
		{
			{ PMGR_CLK_OSC, 1 },
			{ PMGR_CLK_S0, 1 },
			{ PMGR_CLK_S1, 1 },
			{ PMGR_CLK_S2, 1 },
			{ PMGR_CLK_S3, 1 },
			{ PMGR_CLK_OSC, 20 },
		}
	},
	[PMGR_CLK_UVD]	=	{
		&rPMGR_CLK_CFG(UVD),
		{
			{ PMGR_CLK_OSC, 1 },
			{ PMGR_CLK_S0, 1 },
			{ PMGR_CLK_S1, 1 },
			{ PMGR_CLK_S2, 1 },
			{ PMGR_CLK_S3, 1 },
			{ PMGR_CLK_PLL6, 1 },
			{ PMGR_CLK_PLL0, 8 },
			{ PMGR_CLK_PLL0, 3 },
		}
	},
	[PMGR_CLK_VID1]	=	{
		&rPMGR_CLK_CFG(VID1),
		{
			{ PMGR_CLK_OSC, 1 },
			{ PMGR_CLK_S0, 1 },
			{ PMGR_CLK_S1, 1 },
			{ PMGR_CLK_S2, 1 },
			{ PMGR_CLK_S3, 1 },
			{ PMGR_CLK_PLL2, 1 },
			{ PMGR_CLK_PLL6, 1 },
			{ PMGR_CLK_PLL0, 4 },
			{ PMGR_CLK_PLL1, 4 },
			{ PMGR_CLK_PLL1, 6 },
			{ PMGR_CLK_PLL0, 7 },
			{ PMGR_CLK_PLL0, 8 },
		}
	},
	[PMGR_CLK_DISP1]	=	{
		&rPMGR_CLK_CFG(DISP1),
		{
			{ PMGR_CLK_OSC, 1 },
			{ PMGR_CLK_S0, 1 },
			{ PMGR_CLK_S1, 1 },
			{ PMGR_CLK_S2, 1 },
			{ PMGR_CLK_S3, 1 },
			{ PMGR_CLK_PLL0, 3 },
			{ PMGR_CLK_PLL1, 3 },
			{ PMGR_CLK_PLL0, 4 },
			{ PMGR_CLK_PLL1, 4 },
			{ PMGR_CLK_PLL0, 5 },
			{ PMGR_CLK_PLL0, 6 },
			{ PMGR_CLK_PLL0, 8 },
		}
	},
	[PMGR_CLK_SRS]	=	{
		&rPMGR_CLK_CFG(SRS),
		{
			{ PMGR_CLK_OSC, 1 },
			{ PMGR_CLK_S0, 1 },
			{ PMGR_CLK_S1, 1 },
			{ PMGR_CLK_S2, 1 },
			{ PMGR_CLK_S3, 1 },
			{ PMGR_CLK_PLL7, 1 },
			{ PMGR_CLK_PLL6, 1 },
			{ PMGR_CLK_PLL0, 3 },
			{ PMGR_CLK_PLL1, 3 },
			{ PMGR_CLK_PLL1, 4 },
			{ PMGR_CLK_PLL0, 5 },
			{ PMGR_CLK_PLL0, 8 },
		}
	},
	[PMGR_CLK_S0]	=	{
		&rPMGR_CLK_CFG(S0),
		{
			{ PMGR_CLK_PLL0, 2 },
			{ PMGR_CLK_PLL1, 2 },
			{ PMGR_CLK_PLL2, 1 },
			{ PMGR_CLK_PLL4, 1 },
		}
	},
	[PMGR_CLK_S1]	=	{
		&rPMGR_CLK_CFG(S1),
		{
			{ PMGR_CLK_PLL0, 2 },
			{ PMGR_CLK_PLL1, 2 },
			{ PMGR_CLK_PLL2, 1 },
			{ PMGR_CLK_PLL4, 1 },
		}
	},
	[PMGR_CLK_S2]	=	{
		&rPMGR_CLK_CFG(S2),
		{
			{ PMGR_CLK_PLL0, 2 },
			{ PMGR_CLK_PLL1, 2 },
			{ PMGR_CLK_PLL6, 1 },
			{ PMGR_CLK_PLL4, 1 },
		}
	},
	[PMGR_CLK_S3]	=	{
		&rPMGR_CLK_CFG(S3),
		{
			{ PMGR_CLK_PLL0, 2 },
			{ PMGR_CLK_PLL1, 2 },
			{ PMGR_CLK_PLL7, 1 },
			{ PMGR_CLK_PLL4, 1 },
		}
	},
	[PMGR_CLK_ISP_REF0]	=	{
		&rPMGR_CLK_CFG(ISP_REF0),
		{
			{ PMGR_CLK_PLL0, 5 },
			{ PMGR_CLK_PLL1, 5 },
			{ PMGR_CLK_PLL0, 6 },
			{ PMGR_CLK_PLL1, 6 },
		}
	},
	[PMGR_CLK_ISP_REF1]	=	{
		&rPMGR_CLK_CFG(ISP_REF1),
		{
			{ PMGR_CLK_PLL0, 5 },
			{ PMGR_CLK_PLL1, 5 },
			{ PMGR_CLK_PLL0, 6 },
			{ PMGR_CLK_PLL1, 6 },
		}
	},
#endif
	[PMGR_CLK_AOP]	    =	{
		&rMINIPMGR_CLK_CFG(AOP),
		{
			{ PMGR_CLK_OSC, 1 },
			{ PMGR_CLK_LPO, 1 },
			{ PMGR_CLK_LPO, 2 },
			{ PMGR_CLK_LPO, 4 },
			{ PMGR_CLK_LPO, 8 }
		}
	},
	
	[PMGR_CLK_UART0]	    =	{
		&rMINIPMGR_CLK_CFG(UART0),
		{
			{ PMGR_CLK_OSC, 1 },
			{ PMGR_CLK_LPO, 1 },
			{ PMGR_CLK_LPO, 2 },
			{ PMGR_CLK_LPO, 4 },
			{ PMGR_CLK_LPO, 8 }
		}
	},
	
	[PMGR_CLK_UART1]	    =	{
		&rMINIPMGR_CLK_CFG(UART1),
		{
			{ PMGR_CLK_OSC, 1 },
			{ PMGR_CLK_LPO, 1 },
			{ PMGR_CLK_LPO, 2 },
			{ PMGR_CLK_LPO, 4 },
			{ PMGR_CLK_LPO, 8 }
		}
	},
	
	[PMGR_CLK_UART2]	    =	{
		&rMINIPMGR_CLK_CFG(UART2),
		{
			{ PMGR_CLK_OSC, 1 },
			{ PMGR_CLK_LPO, 1 },
			{ PMGR_CLK_LPO, 2 },
			{ PMGR_CLK_LPO, 4 },
			{ PMGR_CLK_LPO, 8 }
		}
	},
	
	[PMGR_CLK_AOP_MCA0_M]	    =	{
		&rMINIPMGR_CLK_CFG(AOP_MCA0_M),
		{
			{ PMGR_CLK_OSC, 1 },
			{ PMGR_CLK_LPO, 2 },
			{ PMGR_CLK_LPO, 4 },
			{ PMGR_CLK_LPO, 8 },
			{ PMGR_CLK_LPO, 1 }
		}
	},
	
	[PMGR_CLK_I2CM]	    =	{
		&rMINIPMGR_CLK_CFG(I2CM),
		{
			{ PMGR_CLK_OSC, 1 },
			{ PMGR_CLK_LPO, 2 },
			{ PMGR_CLK_LPO, 4 },
			{ PMGR_CLK_LPO, 8 },
			{ PMGR_CLK_LPO, 1 }
		}
	},
	
	[PMGR_CLK_PROXY_FABRIC]	    =	{
		&rMINIPMGR_CLK_CFG(PROXY_FABRIC),
		{
			{ PMGR_CLK_OSC, 1 },
			{ PMGR_CLK_LPO, 1 },
			{ PMGR_CLK_LPO, 2 },
			{ PMGR_CLK_LPO, 4 },
			{ PMGR_CLK_LPO, 8 }
		}
	},
	
	[PMGR_CLK_PROXY_MCU_REF]	    =	{
		&rMINIPMGR_CLK_CFG(PROXY_MCU_REF),
		{
			{ PMGR_CLK_OSC, 1 },
			{ PMGR_CLK_LPO, 4 },
			{ PMGR_CLK_LPO, 8 },
			{ PMGR_CLK_LPO, 1 },
			{ PMGR_CLK_LPO, 2 }
		}
	},
};

static const struct device_config device_configs[PMGR_DEVICE_COUNT] = {
	// Mini PMGR
	[PMGR_DEVICE_INDEX(CLK_AOP)] 				= {&rMINIPMGR_PS(AOP)},
	[PMGR_DEVICE_INDEX(CLK_DEBUG)] 				= {&rMINIPMGR_PS(DEBUG)},
	[PMGR_DEVICE_INDEX(CLK_AOP_GPIO)] 			= {&rMINIPMGR_PS(AOP_GPIO)},
	[PMGR_DEVICE_INDEX(CLK_AOP_UART0)] 			= {&rMINIPMGR_PS(AOP_UART0)},
	[PMGR_DEVICE_INDEX(CLK_AOP_UART1)] 			= {&rMINIPMGR_PS(AOP_UART1)},
	[PMGR_DEVICE_INDEX(CLK_AOP_UART2)] 			= {&rMINIPMGR_PS(AOP_UART2)},
	[PMGR_DEVICE_INDEX(CLK_AOP_I2CM)] 			= {&rMINIPMGR_PS(AOP_I2CM)},
	[PMGR_DEVICE_INDEX(CLK_AOP_MCA0)] 			= {&rMINIPMGR_PS(AOP_MCA0)},
	[PMGR_DEVICE_INDEX(CLK_AOP_CPU)] 			= {&rMINIPMGR_PS(AOP_CPU)},
	[PMGR_DEVICE_INDEX(CLK_AOP_FILTER)] 			= {&rMINIPMGR_PS(AOP_FILTER)},
	[PMGR_DEVICE_INDEX(CLK_AOP_BUSIF)] 			= {&rMINIPMGR_PS(AOP_BUSIF)},
	// PMGR
	[PMGR_DEVICE_INDEX(CLK_SBR)] 				= {&rPMGR_PS(SBR)},
	[PMGR_DEVICE_INDEX(CLK_AIC)] 				= {&rPMGR_PS(AIC)},
	[PMGR_DEVICE_INDEX(CLK_DWI)] 				= {&rPMGR_PS(DWI)},
	[PMGR_DEVICE_INDEX(CLK_GPIO)] 				= {&rPMGR_PS(GPIO)},
	[PMGR_DEVICE_INDEX(CLK_PMS)] 				= {&rPMGR_PS(PMS)},
	[PMGR_DEVICE_INDEX(CLK_HSIC0PHY)] 			= {&rPMGR_PS(HSIC0PHY)},
#if SUB_PLATFORM_S8000 || SUB_PLATFORM_S8003
	[PMGR_DEVICE_INDEX(CLK_HSIC1PHY)] 			= {&rPMGR_PS(HSIC1PHY)},
#endif
	[PMGR_DEVICE_INDEX(CLK_ISPSENS0)] 			= {&rPMGR_PS(ISPSENS0)},
	[PMGR_DEVICE_INDEX(CLK_ISPSENS1)] 			= {&rPMGR_PS(ISPSENS1)},
	[PMGR_DEVICE_INDEX(CLK_PCIE_REF)] 			= {&rPMGR_PS(PCIE_REF)},
	[PMGR_DEVICE_INDEX(CLK_SIO_BUSIF)] 			= {&rPMGR_PS(SIO_BUSIF)},
	[PMGR_DEVICE_INDEX(CLK_SIO_P)] 				= {&rPMGR_PS(SIO_P)},
	[PMGR_DEVICE_INDEX(CLK_SIO)] 				= {&rPMGR_PS(SIO)},
	[PMGR_DEVICE_INDEX(CLK_MCA0)] 				= {&rPMGR_PS(MCA0)},
	[PMGR_DEVICE_INDEX(CLK_MCA1)] 				= {&rPMGR_PS(MCA1)},
	[PMGR_DEVICE_INDEX(CLK_MCA2)] 				= {&rPMGR_PS(MCA2)},
	[PMGR_DEVICE_INDEX(CLK_MCA3)] 				= {&rPMGR_PS(MCA3)},
	[PMGR_DEVICE_INDEX(CLK_MCA4)] 				= {&rPMGR_PS(MCA4)},
	[PMGR_DEVICE_INDEX(CLK_PWM0)] 				= {&rPMGR_PS(PWM0)},
	[PMGR_DEVICE_INDEX(CLK_I2C0)] 				= {&rPMGR_PS(I2C0)},
	[PMGR_DEVICE_INDEX(CLK_I2C1)] 				= {&rPMGR_PS(I2C1)},
	[PMGR_DEVICE_INDEX(CLK_I2C2)] 				= {&rPMGR_PS(I2C2)},
	[PMGR_DEVICE_INDEX(CLK_I2C3)] 				= {&rPMGR_PS(I2C3)},
	[PMGR_DEVICE_INDEX(CLK_SPI0)] 				= {&rPMGR_PS(SPI0)},
	[PMGR_DEVICE_INDEX(CLK_SPI1)] 				= {&rPMGR_PS(SPI1)},
	[PMGR_DEVICE_INDEX(CLK_SPI2)] 				= {&rPMGR_PS(SPI2)},
	[PMGR_DEVICE_INDEX(CLK_SPI3)] 				= {&rPMGR_PS(SPI3)},
	[PMGR_DEVICE_INDEX(CLK_UART0)] 				= {&rPMGR_PS(UART0)},
	[PMGR_DEVICE_INDEX(CLK_UART1)] 				= {&rPMGR_PS(UART1)},
	[PMGR_DEVICE_INDEX(CLK_UART2)] 				= {&rPMGR_PS(UART2)},
	[PMGR_DEVICE_INDEX(CLK_UART3)] 				= {&rPMGR_PS(UART3)},
	[PMGR_DEVICE_INDEX(CLK_UART4)] 				= {&rPMGR_PS(UART4)},
	[PMGR_DEVICE_INDEX(CLK_UART5)] 				= {&rPMGR_PS(UART5)},
	[PMGR_DEVICE_INDEX(CLK_UART6)] 				= {&rPMGR_PS(UART6)},
	[PMGR_DEVICE_INDEX(CLK_UART7)] 				= {&rPMGR_PS(UART7)},
	[PMGR_DEVICE_INDEX(CLK_UART8)] 				= {&rPMGR_PS(UART8)},
	[PMGR_DEVICE_INDEX(CLK_AES0)] 				= {&rPMGR_PS(AES0)},
#if SUB_PLATFORM_S8001
	[PMGR_DEVICE_INDEX(CLK_DPA0)] 				= {&rPMGR_PS(DPA0)},
	[PMGR_DEVICE_INDEX(CLK_DPA1)] 				= {&rPMGR_PS(DPA1)},
#endif
	[PMGR_DEVICE_INDEX(CLK_MCC)] 				= {&rPMGR_PS(MCC)},
	[PMGR_DEVICE_INDEX(CLK_DCS0)] 				= {&rPMGR_PS(DCS0)},
	[PMGR_DEVICE_INDEX(CLK_DCS1)] 				= {&rPMGR_PS(DCS1)},
	[PMGR_DEVICE_INDEX(CLK_DCS2)] 				= {&rPMGR_PS(DCS2)},
	[PMGR_DEVICE_INDEX(CLK_DCS3)] 				= {&rPMGR_PS(DCS3)},
#if SUB_PLATFORM_S8001
	[PMGR_DEVICE_INDEX(CLK_DCS4)] 				= {&rPMGR_PS(DCS4)},
	[PMGR_DEVICE_INDEX(CLK_DCS5)] 				= {&rPMGR_PS(DCS5)},
	[PMGR_DEVICE_INDEX(CLK_DCS6)] 				= {&rPMGR_PS(DCS6)},
	[PMGR_DEVICE_INDEX(CLK_DCS7)] 				= {&rPMGR_PS(DCS7)},
#endif	
	[PMGR_DEVICE_INDEX(CLK_USB)] 				= {&rPMGR_PS(USB)},
	[PMGR_DEVICE_INDEX(CLK_USBCTLREG)] 			= {&rPMGR_PS(USBCTLREG)},
	[PMGR_DEVICE_INDEX(CLK_USB2HOST0)] 			= {&rPMGR_PS(USB2HOST0)},
	[PMGR_DEVICE_INDEX(CLK_USB2HOST0_OHCI)] 		= {&rPMGR_PS(USB2HOST0_OHCI)},
	[PMGR_DEVICE_INDEX(CLK_USB2HOST1)] 			= {&rPMGR_PS(USB2HOST1)},
	[PMGR_DEVICE_INDEX(CLK_USB2HOST1_OHCI)] 		= {&rPMGR_PS(USB2HOST1_OHCI)},
	[PMGR_DEVICE_INDEX(CLK_USB2HOST2)] 			= {&rPMGR_PS(USB2HOST2)},
	[PMGR_DEVICE_INDEX(CLK_USB2HOST2_OHCI)] 		= {&rPMGR_PS(USB2HOST2_OHCI)},
	[PMGR_DEVICE_INDEX(CLK_USB_OTG)] 			= {&rPMGR_PS(USB_OTG)},
	[PMGR_DEVICE_INDEX(CLK_SMX)] 				= {&rPMGR_PS(SMX)},
	[PMGR_DEVICE_INDEX(CLK_SF)] 				= {&rPMGR_PS(SF)},
	[PMGR_DEVICE_INDEX(CLK_RTMUX)] 				= {&rPMGR_PS(RTMUX)},
	[PMGR_DEVICE_INDEX(CLK_DISP0)] 				= {&rPMGR_PS(DISP0)},
#if SUB_PLATFORM_S8000 || SUB_PLATFORM_S8003
	[PMGR_DEVICE_INDEX(CLK_MIPI_DSI)] 			= {&rPMGR_PS(MIPI_DSI)},
	[PMGR_DEVICE_INDEX(CLK_DP)] 				= {&rPMGR_PS(DP)},
#endif
#if SUB_PLATFORM_S8001
	[PMGR_DEVICE_INDEX(CLK_DP0)] 				= {&rPMGR_PS(DP0)},
	[PMGR_DEVICE_INDEX(CLK_DISP1MUX)] 			= {&rPMGR_PS(DISP1MUX)},
	[PMGR_DEVICE_INDEX(CLK_DISP1)] 				= {&rPMGR_PS(DISP1)},
	[PMGR_DEVICE_INDEX(CLK_DP1)] 				= {&rPMGR_PS(DP1)},
#endif
	[PMGR_DEVICE_INDEX(CLK_ISP)] 				= {&rPMGR_PS(ISP)},
	[PMGR_DEVICE_INDEX(CLK_MEDIA)] 				= {&rPMGR_PS(MEDIA)},
	[PMGR_DEVICE_INDEX(CLK_JPG)] 				= {&rPMGR_PS(JPG)},
	[PMGR_DEVICE_INDEX(CLK_MSR)] 				= {&rPMGR_PS(MSR)},
	[PMGR_DEVICE_INDEX(CLK_PMP)] 				= {&rPMGR_PS(PMP)},
	[PMGR_DEVICE_INDEX(CLK_PMS_SRAM)] 			= {&rPMGR_PS(PMS_SRAM)},
	[PMGR_DEVICE_INDEX(CLK_VDEC0)] 				= {&rPMGR_PS(VDEC0)},
	[PMGR_DEVICE_INDEX(CLK_VENC_CPU)] 			= {&rPMGR_PS(VENC_CPU)},
	[PMGR_DEVICE_INDEX(CLK_PCIE)] 				= {&rPMGR_PS(PCIE)},
	[PMGR_DEVICE_INDEX(CLK_PCIE_AUX)] 			= {&rPMGR_PS(PCIE_AUX)},
	[PMGR_DEVICE_INDEX(CLK_PCIE_LINK0)] 			= {&rPMGR_PS(PCIE_LINK0)},
	[PMGR_DEVICE_INDEX(CLK_PCIE_LINK1)] 			= {&rPMGR_PS(PCIE_LINK1)},
	[PMGR_DEVICE_INDEX(CLK_PCIE_LINK2)] 			= {&rPMGR_PS(PCIE_LINK2)},
	[PMGR_DEVICE_INDEX(CLK_PCIE_LINK3)] 			= {&rPMGR_PS(PCIE_LINK3)},
#if SUB_PLATFORM_S8001
	[PMGR_DEVICE_INDEX(CLK_PCIE_LINK4)] 			= {&rPMGR_PS(PCIE_LINK4)},
	[PMGR_DEVICE_INDEX(CLK_PCIE_LINK5)] 			= {&rPMGR_PS(PCIE_LINK5)},
#endif	
	[PMGR_DEVICE_INDEX(CLK_GFX)] 				= {&rPMGR_PS(GFX)},
#if SUB_PLATFORM_S8001
	[PMGR_DEVICE_INDEX(CLK_SRS)] 				= {&rPMGR_PS(SRS)},
#endif
	// Unmanaged
	[PMGR_DEVICE_INDEX(CLK_CPU0)] 				= {&rPMGR_PS(CPU0)},
	[PMGR_DEVICE_INDEX(CLK_CPU1)] 				= {&rPMGR_PS(CPU1)},
	[PMGR_DEVICE_INDEX(CLK_CPM)] 				= {&rPMGR_PS(CPM)},
	[PMGR_DEVICE_INDEX(CLK_SEP)] 				= {&rPMGR_PS(SEP)},
	[PMGR_DEVICE_INDEX(CLK_VENC_PIPE)] 			= {&rPMGR_PS(VENC_PIPE)},
	[PMGR_DEVICE_INDEX(CLK_VENC_ME0)] 			= {&rPMGR_PS(VENC_ME0)},
	[PMGR_DEVICE_INDEX(CLK_VENC_ME1)] 			= {&rPMGR_PS(VENC_ME1)},
};

static void config_apsc_acc_state(uint32_t state, enum chipid_voltage_index voltage_index, bool securerom);
static uint32_t get_apsc_acc_state(void);
static void set_apsc_acc_state(uint32_t target_state);
static void enable_cpu_pll_reset(bool enable);
static void config_soc_perf_state(uint32_t state, enum chipid_voltage_index voltage_index);
static void set_soc_perf_state(uint32_t target_state);
#if (APPLICATION_IBOOT && !PRODUCT_IBOOT && !PRODUCT_IBEC)
static void init_thermal_registers(void);
#endif
static void clocks_get_frequencies(void);
static void clocks_get_frequencies_range(uint32_t start_clk, uint32_t end_clk);
static uint32_t get_pll(int32_t pll);
static uint32_t get_pll_cpu(void);
static void set_lpo(void);
static void set_pll(int32_t pll, uint32_t p, uint32_t m, uint32_t s);
static uint32_t get_spare(int32_t spare);
static void clocks_set_gates(uint64_t *devices);
static void clocks_quiesce_internal(void);
static void power_on_sep(void);

bool pmgr_platform_set_perf_state(bool gpu, uint32_t state_num, uint32_t *voltage_indexes, uint32_t voltage_index_size)
{
	uint32_t index = 0;
	uint32_t max;
	uint32_t min;
	if (gpu) {
		max = kPMGR_GFX_STATE_MAX - 1;
		min = 1;
	} else {
		max = kDVFM_STATE_MAX_CNT - 1;
		min = 3;
	}

	while (index < voltage_index_size) {
		if (pmgr_platform_get_freq(gpu, voltage_indexes[index]) == 0) {
			return false; // wrong voltage index;
		}
		if ((state_num + index < min) || (state_num + index > max)) {
			return false;
		}
		if (gpu) {
			 set_gfx_perf_state(state_num + index, voltage_indexes[index]);
		} else {
			config_apsc_acc_state(state_num + index, voltage_indexes[index], false);
		}
		index++;
	}

	while (state_num + index <= max) {
		// Always fill following gpu/cpu with default values
		if (gpu) {
			set_gfx_perf_state(state_num + index, CHIPID_GPU_VOLTAGE_OFF);
		} else {
			config_apsc_acc_state(state_num + index, CHIPID_CPU_VOLTAGE_396, false);
		}
		index++;
	}
	return true;
}

uint32_t pmgr_platform_get_freq(bool gpu, uint32_t voltage_index)
{
	if (gpu) {
		const struct operating_point_params *params = operating_point_get_params(voltage_index, CHIPID_GPU_VOLTAGE);
		if (params == NULL) {
			return 0;
		}
		if (params->preDivP == 0) {
			return 0;
		}
		return 24 * params->fbkDivM / params->preDivP / (1 + params->pstDivS);
	} else {
		const struct operating_point_params *params = operating_point_get_params(voltage_index, CHIPID_CPU_VOLTAGE);
		if (params == NULL) {
			return 0;
		}
		return 24 * params->fbkDivM / params->preDivP / (1 + params->pstDivS);
	}
}

bool pmgr_platform_get_perf_state(enum pmgr_binning_type_t type, uint32_t state, uint32_t *voltage, uint32_t *freq)
{
	switch (type) {
		case PMGR_BINNING_CPU:
		case PMGR_BINNING_CPU_SRAM:
			if (state >= kDVFM_STATE_MAX_CNT) {
				return false;
			}
			break;
		case PMGR_BINNING_GPU:
		case PMGR_BINNING_GPU_SRAM:
			if (state >= kPMGR_GFX_STATE_MAX) {
				return false;
			}
			break;
		case PMGR_BINNING_SOC:
			if (state >= kSOC_PERF_STATE_MAX_CNT) {
				return false;
			}
			break;
		default:
			return false;
	}
	switch (type) {
                case PMGR_BINNING_CPU: {
			uint64_t state_entry =  rACC_DVFM_ST(state);
			*freq = OSC_FREQ;
			*freq *= ((state_entry >> 4) & 0x1FF);
			*freq /= ((state_entry >> 13) & 0x1F) ? ((state_entry >> 13) & 0x1F) : 1;
			*freq /= (1 + ((state_entry >> 0) & 0xF));
                        *voltage = platform_get_dwi_to_mv(BUCK_CPU, ACC_PWRCTL_DVFM_ST0_SAFE_VOL_XTRCT(rACC_DVFM_ST(state)));
			break;
		}
		case PMGR_BINNING_CPU_SRAM:
			*freq = 0;
			*voltage = platform_get_dwi_to_mv(BUCK_CPU_RAM, ACC_PWRCTL_DVFM_ST0_EXT_SRAM_VOL_XTRCT(rACC_DVFM_ST_EXT(state)));
			break;
		case PMGR_BINNING_GPU: {
			uint32_t state_val = rPMGR_GFX_PERF_STATE_ENTRY_A(state);
			*freq = PMGR_PLL_FREQ((state_val >> 12) & 0x1FF, (state_val >> 4) & 0x1F, state_val & 0xF);
			*voltage = platform_get_dwi_to_mv(BUCK_GPU, PMGR_GFX_PERF_STATE_ENTRY0A_VOLTAGE_XTRCT(rPMGR_GFX_PERF_STATE_ENTRY_A(state)));
			break;
		}
		case PMGR_BINNING_GPU_SRAM:
			*freq = 0;
			*voltage = platform_get_dwi_to_mv(BUCK_GPU_RAM, PMGR_GFX_PERF_STATE_ENTRY0B_SRAM_VOLTAGE_XTRCT(rPMGR_GFX_PERF_STATE_ENTRY_B(state)));
			break;
		case PMGR_BINNING_SOC:
			*freq = 0;
			*voltage = platform_get_dwi_to_mv(BUCK_SOC, PMGR_SOC_PERF_STATE_ENTRY_0C_VOLTAGE_XTRCT(rPMGR_SOC_PERF_STATE_ENTRY_C(state)));
			break;
		default:
			return false;
	}
	return true;
}

// current clock frequencies
static uint32_t clks[PMGR_CLK_COUNT];

void platform_power_spin(uint32_t usecs)
{
	arm_no_wfe_spin(usecs);
}

void pmgr_platform_config_uvwarn(void)
{
	uint32_t i, voltage, freq;
	uint32_t prev_voltage = 0, overdrive_voltage = 0, arm_threshold = 0;

	for (i = 1; i < kPMGR_GFX_STATE_MAX; i++) {
		pmgr_platform_get_perf_state(PMGR_BINNING_GPU, i, &voltage, &freq);

		if (freq == 0)
			break;

		prev_voltage = overdrive_voltage;
		overdrive_voltage = voltage;
	}

	// Check voltages in mV
	if (overdrive_voltage == 0 || prev_voltage == 0)
		panic("Invalid binning or operating points for UV-WARN");

	platform_convert_voltages(BUCK_GPU, 1, &prev_voltage);
	platform_convert_voltages(BUCK_GPU, 1, &overdrive_voltage);
	arm_threshold = overdrive_voltage - 1;
	arm_threshold = platform_get_dwi_to_mv(BUCK_GPU, arm_threshold);

	// Check voltages in voltage code
	if (overdrive_voltage <= prev_voltage)
		panic("Invalid binning for UV-WARN");

	// Arm threshold is 1 voltage code below overdrive voltage
	pmu_uvwarn_config(0, arm_threshold);
}

int clocks_init(void)
{
#if (APPLICATION_IBOOT && (PRODUCT_IBOOT || PRODUCT_IBEC || WITH_RECOVERY_MODE_IBSS))
	clocks_get_frequencies();
#endif

	return 0;
}

#if (APPLICATION_IBOOT && !PRODUCT_IBOOT && !PRODUCT_IBEC)

static void set_nco_clocks(void)
{
	uint32_t i;

	// Enable this NCO with alg_ref0_clk and nco_ref0_clk.
	// Note: clk_configs assumes all NCOs use nco_ref0_clk
	for (i = 0; i < PMGR_NUM_NCO; i++) {
		rPMGR_NCO_CLK_CFG(i) |= PMGR_NCO_CLK_CFG_ENABLE;
	}
}
#endif


static void apply_tunables_clkcfg(uint32_t first, uint32_t last)
{
	uint32_t j;

	// Set the clocks
	for (j = first; j < last; j++) {
		uint64_t reg = (uint64_t)clk_configs[j].clock_reg;
		if (reg == 0) {
			continue;
		}
		if (reg >= AOP_MINIPMGR_BASE_ADDR) {
			continue; // Don't touch MINIPMGR clock
		}
		reconfig_command_write(AOP_DDR_AWAKE_POST, reg, *((uint32_t *)reg), false);
		reconfig_command_read(AOP_DDR_AWAKE_POST, reg, 0, PMGR_CLK_CFG_PENDING, 255, false);
	}
}

// apply_tunables(false) is idempotent: it doesn't write to a register if a register contains the right tunable
static void apply_tunables(bool reconfig)
{
	uint32_t i, j;
	const struct tunable_chip_struct (*tunables)[];
#if WITH_HW_RECONFIG
	if (reconfig) {
		volatile uint64_t *reg = (volatile uint64_t *)(PMGR_BASE_ADDR + PMGR_PWRGATE_CPM_CFG0_OFFSET);
		reconfig_command_write(AOP_DDR_AWAKE_POST,
			(uint64_t)reg,
			*reg &  ~PMGR_PWRGATE_CPM_CFG0_PWR_DOM_EN_UMASK,
			0);
	}
#endif

	for (j = 0; j < 2; j++) {
		if (j < 1)
			tunables = &tunables_pmgr;
		else
			tunables = &tunables_pmgr_product;

		for (i = 0; ((*tunables)[i].tunable != NULL) || ((*tunables)[i].tunable64 != NULL); i++) {
			void *tunable_base = (void *)(*tunables)[i].base_address;
			uint32_t tunable_index;

			if ((*tunables)[i].chip_rev != chipid_get_chip_revision()) {
				continue;
			}

			if ((reconfig) && (!(*tunables)[i].reconfig)) {
				continue;
			}

			for (tunable_index = 0; 1 ; tunable_index++) {
				uint32_t tunable_size;
				uint32_t tunable_offset;
				uint64_t tunable_mask;
				uint64_t tunable_value;

				if ((*tunables)[i].tunable) {
					tunable_size = (*tunables)[i].tunable[tunable_index].size;
					tunable_offset = (*tunables)[i].tunable[tunable_index].offset;
					tunable_mask = (*tunables)[i].tunable[tunable_index].mask;
					tunable_value = (*tunables)[i].tunable[tunable_index].value;
				} else {
					tunable_size = (*tunables)[i].tunable64[tunable_index].size;
					tunable_offset = (*tunables)[i].tunable64[tunable_index].offset;
					tunable_mask = (*tunables)[i].tunable64[tunable_index].mask;
					tunable_value = (*tunables)[i].tunable64[tunable_index].value;
				}
				if (tunable_offset == UINT32_MAX) {
					break;
				}

				uint32_t register_size = tunable_size * 8;
				 if ((tunable_base + tunable_offset == (void *)AOP_MINIPMGR_BASE_ADDR + MINIPMGR_MINI_CLKCFG_PROXY_OSC_CLK_CFG_OFFSET)
					|| (tunable_base + tunable_offset == (void *)AOP_MINIPMGR_BASE_ADDR + MINIPMGR_MINI_CLKCFG_XI0MUX_CLK_CFG_OFFSET)
					|| (tunable_base + tunable_offset == (void *)AOP_MINIPMGR_BASE_ADDR + MINIPMGR_MINI_MISC_CFG_ACG_OFFSET)) {
					// Done in the kernel driver
					continue;
				}

				if (tunable_base + tunable_offset == (void *)PMGR_BASE_ADDR + PMGR_MISC_CFG_ACG_OFFSET) {
					// Will be done in the Kernel driver
					continue;
				}

				if ((tunable_base + tunable_offset >= (void *)PMGR_BASE_ADDR + PMGR_CLKCFG_OFFSET) &&
					(tunable_base + tunable_offset <= (void *)PMGR_BASE_ADDR + PMGR_CLKCTL_OFFSET)) {
					// For CLKCFG tunables are interleaved with the clock mesh.
					// Since, we don't have a read-modify-write, we let the CLKCFG reconfig restore the Clockmesh
					// and the tunables at same time.
					if (reconfig) {
						continue;
					}
				}

				if ((tunable_base + tunable_offset < (void *)(ACC_BASE_ADDR + ACC_PWRCTL_OFFSET))
					&& (tunable_base + tunable_offset >= (void *)(ACC_BASE_ADDR))) {
					// We let xnu taking care of CPUi_IMPL_HID*, since setting ACC_cpui_IMPL_HID4 would prevent
					// the end of LLB/iBSS which require set/way ops. Also, CPUi for i > 0 are off and so CPUi_IMPL_HID*
					// are not accessible, but it would not be a problem here since synchronous error are masked in LLB/iBSS.
					continue;
				}
				if ((tunable_base + tunable_offset >= (void *)(ACC_BASE_ADDR + ACC_IMPL_OFFSET))
					&& (tunable_base + tunable_offset < (void *)(ACC_BASE_ADDR + ACC_CNTCTL_OFFSET))) {
					// ACC_IMPL_* are lost after a cluster power gating. Let's xnu taking care of them.
					continue;
				}

				uint64_t val;
				if (register_size == 64) {
					val = *((volatile uint64_t *)(tunable_base + tunable_offset));
				} else {
					val = *((volatile uint32_t *)(tunable_base + tunable_offset));
				}

				// Hack for <rdar://problem/19929273> Maui MCC EMA setting change to 3%
				if (tunable_base + tunable_offset == (void *)(PMGR_BASE_ADDR + PMGR_EMA_FIXED_SOC1_OFFSET)) {
					if (chipid_get_fuse_revision() >= 0x7) {
						continue;
					}
				}

				if (!reconfig) {
					if ((val & tunable_mask) == tunable_value) {
						// Tunable already applied. No need to apply it again
						continue;
					}

					val &= ~tunable_mask;
					val |= tunable_value;
					if (register_size == 64) {
						*((volatile uint64_t *)(tunable_base + tunable_offset)) = val;
					} else {
						*((volatile uint32_t *)(tunable_base + tunable_offset)) = (uint32_t)val;
					}
				} else {
	#if WITH_HW_RECONFIG
					reconfig_command_write(AOP_DDR_AWAKE_POST,
						(uint64_t)(tunable_base + tunable_offset),
						val,
						register_size == 64);
	#endif
				}
			}
		}
	}
}

static void pmgr_reconfig_pll(void)
{
#if WITH_HW_RECONFIG
	uint32_t value;
	{
		uint32_t pll;
		for (pll = 0; pll < PMGR_PLL_COUNT; pll++) {
			if (pll == PLL_PCIE) {
				reconfig_command_write(AOP_DDR_AWAKE_POST, (uint64_t)&rPMGR_PLL_ANA_PARAMS2(pll), rPMGR_PLL_ANA_PARAMS2(pll), false);
				reconfig_command_write(AOP_DDR_AWAKE_POST, (uint64_t)&rPMGR_PLL_ANA_PARAMS3(pll), rPMGR_PLL_ANA_PARAMS3(pll), false);
				reconfig_command_write(AOP_DDR_AWAKE_POST, (uint64_t)&rPMGR_PLL_PCIE_DELAY_CTL0(pll), rPMGR_PLL_PCIE_DELAY_CTL0(pll), false);
			}
			reconfig_command_write(AOP_DDR_AWAKE_POST, (uint64_t)&rPMGR_PLL_CTL(pll), rPMGR_PLL_CTL(pll), false);
			reconfig_command_read(AOP_DDR_AWAKE_POST, (uint64_t)&rPMGR_PLL_CTL(pll), 0, PMGR_PLL_PENDING, 0, false); // Infinite wait

			if (pll == PLL_PCIE) {
				reconfig_command_write(AOP_DDR_AWAKE_POST, (uint64_t)&rPMGR_MISC_PCIE_PCIE_CTL, rPMGR_MISC_PCIE_PCIE_CTL, false);
			}
		}
	}

	// Restore SOC STATE 0, with same state than clocks_set_performance(kPerformanceMemoryLow);
	value = rPMGR_SOC_PERF_STATE_ENTRY_A(PMGR_SOC_PERF_STATE_TO_ENTRY(kSOC_PERF_STATE_VMIN));
	value &= ~PMGR_SOC_PERF_STATE_ENTRY_0A_MCU_REF_CFG_SEL_UMASK;
#if SUB_PLATFORM_S8000 || SUB_PLATFORM_S8003
	value |= PMGR_SOC_PERF_STATE_ENTRY_0A_MCU_REF_CFG_SEL_INSRT(1); // we keep the SRC_SEL, but move to bucket 1
#elif SUB_PLATFORM_S8001
	value |= PMGR_SOC_PERF_STATE_ENTRY_0A_MCU_REF_CFG_SEL_INSRT(0); // we keep the SRC_SEL, but move to bucket 0
#else
#error "Unknown platform"
#endif

	reconfig_command_write(AOP_DDR_AWAKE_POST, (uint64_t)&rPMGR_SOC_PERF_STATE_ENTRY_A(0), value, false);
	reconfig_command_write(AOP_DDR_AWAKE_POST, (uint64_t)&rPMGR_SOC_PERF_STATE_ENTRY_B(0), rPMGR_SOC_PERF_STATE_ENTRY_B(PMGR_SOC_PERF_STATE_TO_ENTRY(kSOC_PERF_STATE_VMIN)), false);
	reconfig_command_write(AOP_DDR_AWAKE_POST, (uint64_t)&rPMGR_SOC_PERF_STATE_ENTRY_C(0), rPMGR_SOC_PERF_STATE_ENTRY_C(PMGR_SOC_PERF_STATE_TO_ENTRY(kSOC_PERF_STATE_VMIN)), false);
	reconfig_command_write(AOP_DDR_AWAKE_POST, (uint64_t)&rPMGR_SOC_PERF_STATE_ENTRY_D(0), rPMGR_SOC_PERF_STATE_ENTRY_D(PMGR_SOC_PERF_STATE_TO_ENTRY(kSOC_PERF_STATE_VMIN)), false);

	// Switch SOC to 0 and wait for the transition
	reconfig_command_write(AOP_DDR_AWAKE_POST, (uint64_t)&rPMGR_SOC_PERF_STATE_CTL, 0, false);
	reconfig_command_read(AOP_DDR_AWAKE_POST, (uint64_t)&rPMGR_SOC_PERF_STATE_CTL, 0, PMGR_SOC_PERF_STATE_CTL_PENDING_MASK, 0, false);

#endif
}

static void pmgr_reconfig_post(void)
{
#if WITH_HW_RECONFIG
	{
		uint32_t i;
		reconfig_command_write(AOP_DDR_AWAKE_POST, (uint64_t)&rPMGR_VOLMAN_BUCK_MAP, rPMGR_VOLMAN_BUCK_MAP, false);

		// Leakage and energy accumulator configuration
		reconfig_command_write(AOP_DDR_AWAKE_POST, (uint64_t)&rACC_NRG_ACC_SCALE_TAB, rACC_NRG_ACC_SCALE_TAB, true);
		reconfig_command_write(AOP_DDR_AWAKE_POST, (uint64_t)&rACC_NRG_ACC_SCALE_TAB_EXT, rACC_NRG_ACC_SCALE_TAB_EXT, true);
		reconfig_command_write(AOP_DDR_AWAKE_POST, (uint64_t)&rACC_LKG_EST_TAB, rACC_LKG_EST_TAB, true);
		reconfig_command_write(AOP_DDR_AWAKE_POST, (uint64_t)&rACC_LKG_EST_TAB_EXT, rACC_LKG_EST_TAB_EXT, true);

		// In the end, DVFM/SOC/GFX states are only tunables!

		// Do not change ACC state 0, it's current state and it is different than the one we had in iBoot!
		for (i = 1; i < kDVFM_STATE_MAX_CNT; i++) {
			reconfig_command_write(AOP_DDR_AWAKE_POST, (uint64_t)&rACC_DVFM_ST(i), rACC_DVFM_ST(i), true);
			reconfig_command_write(AOP_DDR_AWAKE_POST, (uint64_t)&rACC_DVFM_ST_EXT(i), rACC_DVFM_ST_EXT(i), true);
		}

		// We can change state 0, because GPU is OFF.
		for (i = 0; i < kPMGR_GFX_STATE_MAX; i++) {
			reconfig_command_write(AOP_DDR_AWAKE_POST, (uint64_t)&rPMGR_GFX_PERF_STATE_ENTRY_A(i), rPMGR_GFX_PERF_STATE_ENTRY_A(i), false);
			reconfig_command_write(AOP_DDR_AWAKE_POST, (uint64_t)&rPMGR_GFX_PERF_STATE_ENTRY_B(i), rPMGR_GFX_PERF_STATE_ENTRY_B(i), false);
			reconfig_command_write(AOP_DDR_AWAKE_POST, (uint64_t)&rPMGR_GFX_PERF_STATE_ENTRY_C(i), rPMGR_GFX_PERF_STATE_ENTRY_C(i), false);
		}
		// Do not change SOC state 0, it's current state and it is different than the one we had in iBoot!
		for (i = 1; i < kSOC_PERF_STATE_MAX_CNT; i++) {
			reconfig_command_write(AOP_DDR_AWAKE_POST, (uint64_t)&rPMGR_SOC_PERF_STATE_ENTRY_A(i), rPMGR_SOC_PERF_STATE_ENTRY_A(i), false);
			reconfig_command_write(AOP_DDR_AWAKE_POST, (uint64_t)&rPMGR_SOC_PERF_STATE_ENTRY_B(i), rPMGR_SOC_PERF_STATE_ENTRY_B(i), false);
			reconfig_command_write(AOP_DDR_AWAKE_POST, (uint64_t)&rPMGR_SOC_PERF_STATE_ENTRY_C(i), rPMGR_SOC_PERF_STATE_ENTRY_C(i), false);
			reconfig_command_write(AOP_DDR_AWAKE_POST, (uint64_t)&rPMGR_SOC_PERF_STATE_ENTRY_D(i), rPMGR_SOC_PERF_STATE_ENTRY_D(i), false);
		}

		reconfig_command_write(AOP_DDR_AWAKE_POST, (uint64_t)&rACC_MSTR_PLLCTL, rACC_MSTR_PLLCTL & ~ACC_PWRCTL_MSTR_PLLCTL_SRC_SEL_UMASK, false);
		reconfig_command_read(AOP_DDR_AWAKE_POST, (uint64_t)&rACC_MSTR_PLLCTL, 0, ACC_PWRCTL_MSTR_PLLCTL_SRC_SEL_UMASK, 0, false); // Infinite wait

#if SUB_PLATFORM_S8000
		// Switch ACC to 396MHz workaround state
		reconfig_command_write(AOP_DDR_AWAKE_POST, (uint64_t)&rACC_APSC_SCR, (rACC_APSC_SCR & ~ACC_PWRCTL_APSC_SCR_MANUAL_ST_UMASK) | ACC_APSC_MANUAL_CHANGE(kDVFM_STATE_IBOOT_VCO_WA), true);
		reconfig_command_read(AOP_DDR_AWAKE_POST, (uint64_t)&rACC_APSC_SCR, 0, ACC_APSC_PENDING, 255, true);
#endif

		// Switch ACC to 396MHz.
		reconfig_command_write(AOP_DDR_AWAKE_POST, (uint64_t)&rACC_APSC_SCR, (rACC_APSC_SCR & ~ACC_PWRCTL_APSC_SCR_MANUAL_ST_UMASK) | ACC_APSC_MANUAL_CHANGE(2), true);
		// Wait the end of ACC change to 396MHz, we can't do after settings tunable since some of them touch ACC PLL.
		reconfig_command_read(AOP_DDR_AWAKE_POST, (uint64_t)&rACC_APSC_SCR, 0, ACC_APSC_PENDING, 255, true);

		// Restore ENABLE bit of rPMGR_GFX_PERF_STATE_CTL
		reconfig_command_write(AOP_DDR_AWAKE_POST, (uint64_t)&rPMGR_GFX_PERF_STATE_CTL, rPMGR_GFX_PERF_STATE_CTL, false);

		// Start Si, followed by ISPi followed by normal clock.
		apply_tunables_clkcfg(PMGR_CLK_S0, PMGR_CLK_ISP_REF1 + 1);

		// Restore NCO
		for (i = 0; i < PMGR_NUM_NCO; i++) {
			reconfig_command_write(AOP_DDR_AWAKE_POST, (uint64_t)&rPMGR_NCO_CLK_CFG(i), rPMGR_NCO_CLK_CFG(i), false);
		}

		apply_tunables_clkcfg(0, PMGR_CLK_S0);

		// Switch SOC to SOC_PERF_STATE_ACTIVE and wait for the transition
		reconfig_command_write(AOP_DDR_AWAKE_POST, (uint64_t)&rPMGR_SOC_PERF_STATE_CTL, rPMGR_SOC_PERF_STATE_CTL, false);
		reconfig_command_read(AOP_DDR_AWAKE_POST, (uint64_t)&rPMGR_SOC_PERF_STATE_CTL, 0, PMGR_SOC_PERF_STATE_CTL_PENDING_MASK, 255, false);

		// Right time to update State 0, which is no more the current state.
		reconfig_command_write(AOP_DDR_AWAKE_POST, (uint64_t)&rACC_DVFM_ST(0), rACC_DVFM_ST(0), true);
		reconfig_command_write(AOP_DDR_AWAKE_POST, (uint64_t)&rACC_DVFM_ST_EXT(0), rACC_DVFM_ST_EXT(0), true);
	}
#endif
}

#if WITH_DEVICETREE
static uint64_t get_freq_from_acc_state(uint32_t state_index);

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
	uint32_t dvfm_state_vmax;
	// DVFM table is filled with kDVFM_STATE_IBOOT copy after the last valid state. Use it to find the last valid state.
	for (dvfm_state_vmax = kDVFM_STATE_IBOOT; dvfm_state_vmax < kDVFM_STATE_MAX_CNT - 1; dvfm_state_vmax++) {
		if (get_freq_from_acc_state(dvfm_state_vmax) >  get_freq_from_acc_state(dvfm_state_vmax + 1)) {
			break;
		}
	}
	pmgr_get_dvfm_data_from_vmax(dvfm, dvfm_state_vmax);
}
#endif

static void enable_bira_work_around()
{
#if SUB_PLATFORM_S8000 || SUB_PLATFORM_S8003
	// Domain order come from <rdar://problem/20183074>
	// "Functional access to memory could be corrupted when a partition is being wake up while another partition is in repair loading"
	// Note: The reconfig part of this workaround is not done by pmgr driver.
	static const uint32_t domain[] = {
		CLK_DISP0,
		CLK_ISP,
		CLK_MEDIA,
		CLK_MSR,
		CLK_VDEC0,
		CLK_VENC_CPU,
		CLK_VENC_PIPE,
		CLK_VENC_ME0,
		CLK_VENC_ME1,
		CLK_GFX,
	};
#elif SUB_PLATFORM_S8001
	static const uint32_t domain[] = {
		CLK_DISP0,
		CLK_ISP,
		CLK_MEDIA,
		CLK_MSR,
		CLK_VDEC0,
		CLK_VENC_CPU,
		CLK_VENC_PIPE,
		CLK_VENC_ME0,
		CLK_VENC_ME1,
		CLK_GFX,
		CLK_DISP1,
		CLK_SRS,
	};
#else
#error "Unknown platform"
#endif

        // Turn on the blocks
	for (uint32_t i = 0; i < sizeof(domain)/sizeof(domain[0]); i++) {
		clock_gate(domain[i], true);
	}
	// Turn off the blocks
	for (uint32_t i = sizeof(domain)/sizeof(domain[0]); i > 0; i--) {
		clock_gate(domain[i - 1], false);
	}
}

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

static void pmgr_set_clkcfg(volatile uint32_t *clkcfg, uint32_t value)
{
	// Keep wait COUNTER tunables
	uint32_t val;
	val = (*clkcfg) & PMGR_CLK_CFG_WAIT_COUNTER_UMASK;
	val |= value  & ~PMGR_CLK_CFG_WAIT_COUNTER_UMASK;
	*clkcfg = val;
	while ((*clkcfg & PMGR_CLK_CFG_PENDING) != 0);
}

/*
 * clocks_set_default - called by SecureROM, LLB, iBSS main via
 * platform_init_setup_clocks, so the current state of the chip is
 * either POR, or whatever 'quiesce' did when leaving SecureROM.
 */

int clocks_set_default(void)
{
	uint32_t cnt;
#if APPLICATION_SECUREROM
	bool securerom = true;
#else
	bool securerom = false;
#endif
	volatile uint32_t *clkcfg;

#if SUB_PLATFORM_S8001
	pmgr_soc_perf_states[kSOC_PERF_STATE_VNOM] = pmgr_soc_perf_states[kSOC_PERF_STATE_VMIN];
#endif

#if (APPLICATION_IBOOT && !PRODUCT_IBOOT && !PRODUCT_IBEC)
	enable_bira_work_around();
#endif

	clks[PMGR_CLK_OSC] = OSC_FREQ;

	// Setup bypass DVFM state
	config_apsc_acc_state(kDVFM_STATE_BYPASS, CHIPID_CPU_VOLTAGE_BYPASS, securerom);

	// Change all clocks to something safe
	clocks_quiesce_internal();

#if (APPLICATION_IBOOT && !PRODUCT_IBOOT && !PRODUCT_IBEC)
	apply_tunables(false);
#endif

	// Setup active DVFM and SOC PERF states for the stage of boot.
#if APPLICATION_SECUREROM
	config_apsc_acc_state(kDVFM_STATE_SECUREROM, CHIPID_CPU_VOLTAGE_SECUREROM, securerom);
	config_soc_perf_state(kSOC_PERF_STATE_SECUREROM, CHIPID_SOC_VOLTAGE_SECUREROM);
#endif

#if APPLICATION_IBOOT
#ifndef BUCK_CPU
#error BUCK_CPU not defined for this platform
#endif

#ifndef BUCK_CPU_RAM
#error BUCK_CPU_RAM not defined for this platform
#endif

#ifndef BUCK_SOC
#error BUCK_SOC not defined for this platform
#endif

#ifndef BUCK_GPU
#error BUCK_GPU not defined for this platform
#endif

#ifndef BUCK_GPU_RAM
#error BUCK_GPU_RAM not defined for this platform
#endif

	rPMGR_VOLMAN_BUCK_MAP =
		PMGR_VOLMAN_BUCK_MAP_CPU_INSRT(BUCK_CPU) |
		PMGR_VOLMAN_BUCK_MAP_CPU_SRAM_INSRT(BUCK_CPU_RAM) |
		PMGR_VOLMAN_BUCK_MAP_SOC_INSRT(BUCK_SOC) |
		PMGR_VOLMAN_BUCK_MAP_GFX_INSRT(BUCK_GPU) |
		PMGR_VOLMAN_BUCK_MAP_GFX_SRAM_INSRT(BUCK_GPU_RAM);
	
	// APSC sleep state will use the bypass state with V0.
	config_apsc_acc_state(kDVFM_STATE_BYPASS, CHIPID_CPU_VOLTAGE_BYPASS, securerom);

	for (cnt = kDVFM_STATE_IBOOT; cnt < kDVFM_STATE_MAX_CNT; cnt++) {
		enum chipid_voltage_index voltage_index = dvfmperf_get_voltage_index(cnt, CHIPID_CPU_VOLTAGE);
		config_apsc_acc_state(cnt, voltage_index, securerom);
	}

	for (cnt = kSOC_PERF_STATE_IBOOT; cnt < kSOC_PERF_STATE_MAX_CNT; cnt++) {
		enum chipid_voltage_index voltage_index = dvfmperf_get_voltage_index(cnt, CHIPID_SOC_VOLTAGE);
		config_soc_perf_state(cnt, voltage_index);
	}
#if WITH_HW_DWI
	extern int dwi_init(void);
	dwi_init();
#endif

#if SUB_PLATFORM_S8000
	// Don't allow CPU PLL to reset on sleep entry (WFI)
	enable_cpu_pll_reset(false);

	// Go to workaround state
	set_apsc_acc_state(kDVFM_STATE_IBOOT_VCO_WA);
#endif
#endif
	
	// ACC clock set for this stage of boot.
	set_apsc_acc_state(active_state);

#if APPLICATION_IBOOT
	for (cnt = 0; cnt <= kPMGR_GFX_STATE_MAX; cnt++) {
		enum chipid_voltage_index index = dvfmperf_get_voltage_index(cnt, CHIPID_GPU_VOLTAGE);
		set_gfx_perf_state(cnt, index);
	}
	
	rPMGR_PLL_CFG(5) |= (PMGR_PLL_RELOCK_MODE_BYPASS << PMGR_PLL_RELOCK_MODE_SHIFT);
	rPMGR_PLL_CTL(5) &= ~PMGR_PLL_BYPASS;
	while (rPMGR_PLL_CTL(5) & PMGR_PLL_PENDING);
	
	rPMGR_GFX_PERF_STATE_CTL |= PMGR_GFX_PERF_STATE_CTL_ENABLE;
#endif

#ifdef LPO_T
	set_lpo();
#endif

#ifdef PLL0_T
	set_pll(PLL0, PLL0_P, PLL0_M, PLL0_S);
#endif

#ifdef PLL1_T
	set_pll(PLL1, PLL1_P, PLL1_M, PLL1_S);
#endif

#ifdef PLL2_T
	set_pll(PLL2, PLL2_P, PLL2_M, PLL2_S);
#endif

#ifdef PLL3_T
#ifndef TARGET_DDR_800M
	if (((chipid_get_chip_id() == 0x8000) && (chipid_get_chip_revision() >= CHIP_REVISION_B0))
		|| (chipid_get_chip_id() == 0x8001) || (chipid_get_chip_id() == 0x8003)) {
		set_pll(PLL3, PLL3_P, PLL3_M, PLL3_S);
	}
#endif
#endif

#ifdef PLL4_T
	set_pll(PLL4, PLL4_P, PLL4_M, PLL4_S);
#endif

#ifdef PLL5_T
	set_pll(PLL5, PLL5_P, PLL5_M, PLL5_S);
#endif

#ifdef PLL6_T
	set_pll(PLL6, PLL6_P, PLL6_M, PLL6_S);
#endif

#ifdef PLL7_T
	set_pll(PLL7, PLL7_P, PLL7_M, PLL7_S);
#endif

#ifdef PLL_PCIE_T
	set_pll(PLL_PCIE, PLL_PCIE_P, PLL_PCIE_M, PLL_PCIE_S);
#endif

#if (APPLICATION_IBOOT && !PRODUCT_IBOOT && !PRODUCT_IBEC)
	// Turn on NCO clocks before enabling MCA clocks.
	set_nco_clocks();
#endif

	set_soc_perf_state(SOC_PERF_STATE_ACTIVE);

	// Set all clock divs to their active values
	for (cnt = 0; cnt < sizeof(clk_configs_active) / sizeof(clk_configs_active[0]); cnt++) {
		clkcfg = clk_configs[clk_configs_active[cnt].clk].clock_reg;
		pmgr_set_clkcfg(clkcfg, clk_configs_active[cnt].clock_reg_val);
	}

	power_on_sep();

	clocks_get_frequencies();

	return 0;
}

static void config_apsc_acc_state(uint32_t state, enum chipid_voltage_index voltage_index, bool securerom)
{
	uint64_t value = 0;
	uint64_t value_ext = 0;

	uint32_t dwi_val = 0;
	uint32_t dwi_sram_val = 0;

	if (state >= kDVFM_STATE_MAX_CNT)
		panic("Unsupported DVFM state");
	
	dwi_val = chipid_get_cpu_voltage(voltage_index);

#if SUB_PLATFORM_S8000
	// Workaround for <rdar://problem/20791889> N66/N71 add up to 30mV to Maui Fuse Rev 8 CPU Mode 6 due to L2C failures
	// Amended in <rdar://problem/20881255> if Maui Fuse Rev 8 CPU Mode 6 bin voltage is above 1100mV, do not perform workaround
	// Amended in <rdar://problem/21069140> N66/N71 add up to 25mV to Maui Fuse Rev 8 CPU Mode 6 changes in binning
	// Amended in <rdar://problem/21116877> N66/N71: Maui Fuse Rev 8 and greater : set MC7 voltage to be equal to MC6 voltage
	if (((voltage_index == CHIPID_CPU_VOLTAGE_1800) || (voltage_index == CHIPID_CPU_VOLTAGE_1848))  && (chipid_get_fuse_revision() == 0x8)) {
		dwi_val = dwi_val + 25;
		if (dwi_val > 1125) {
			dwi_val = 1125;
		}
	}
#endif

	platform_convert_voltages(BUCK_CPU, 1, &dwi_val);
	dwi_sram_val = chipid_get_cpu_sram_voltage(voltage_index);
	platform_convert_voltages(BUCK_CPU_RAM, 1, &dwi_sram_val);
	
	const struct operating_point_params *params = operating_point_get_params(voltage_index, CHIPID_CPU_VOLTAGE);
	if (params == NULL) {
		return;	// !!!FIXME!!! <rdar://problem/19148772> S8003: Populate chipid_voltadj_entry table
	}

	value |= ACC_DVFM_ST_PLL_P(params->preDivP);
	value |= ACC_DVFM_ST_PLL_M(params->fbkDivM);
	value |= ACC_DVFM_ST_PLL_S(params->pstDivS);
	value |= ACC_DVFM_ST_SAFE_VOL(dwi_val);
	value_ext |= ACC_DVFM_ST_BIU_DIV_HI_VOL(params->biuDiv4HiVol);
	value_ext |= ACC_DVFM_ST_BIU_DIV_LO_VOL(params->biuDiv4LoVol);
	value_ext |= ACC_DVFM_ST_DVMR_MAX_WGT(params->dvmrMaxWgt);
	value_ext |= ACC_PWRCTL_DVFM_ST0_EXT_IEXRF_CFG_WR_DATA_INSRT(params->iexrfCfgWrData);
	value_ext |= ACC_PWRCTL_DVFM_ST0_EXT_IEXRF_CFG_WR_IDXA_INSRT(params->iexrfCfgWrIdxa);
	value_ext |= ACC_PWRCTL_DVFM_ST0_EXT_IEXRF_CFG_WR_IDXB_INSRT(params->iexrfCfgWrIdxb);
	value_ext |= ACC_PWRCTL_DVFM_ST0_EXT_IEXRF_CFG_WR_IDXMUXSEL_INSRT(params->exrfCfgWrIdxmuxsel);
	value_ext |= ACC_DVFM_ST_SRAM_VOL(dwi_sram_val);

	if (securerom) {
		value |= ACC_DVFM_ST_CLK_SRC(params->clkSrc);
		value |= ACC_DVFM_ST_VOL_BYP(1);
		value_ext |= ACC_DVFM_ST_DVFM_MAX_ADJ(0x7f);
	} else {
		const struct pmgr_binning_vol_adj *adj = pmgr_binning_get_vol_adj(chipid_get_chip_id(), chipid_get_chip_revision(), voltage_index);
		if (adj == NULL) {
			return;	// !!!FIXME!!! <rdar://problem/19148772> S8003: Populate chipid_voltadj_entry table
		}
		value |= ACC_DVFM_ST_CLK_SRC(params->clkSrc);
		value |= ACC_DVFM_ST_VOL_BYP(params->bypass);
		value |= ACC_DVFM_ST_VOL_ADJ0(pmgr_get_offset_from_diff_uV(BUCK_CPU, adj->volAdj0));
		value |= ACC_DVFM_ST_VOL_ADJ1(pmgr_get_offset_from_diff_uV(BUCK_CPU, adj->volAdj1));
		value |= ACC_DVFM_ST_VOL_ADJ2(pmgr_get_offset_from_diff_uV(BUCK_CPU, adj->volAdj2));
		value |= ACC_DVFM_ST_VOL_ADJ3(pmgr_get_offset_from_diff_uV(BUCK_CPU, adj->volAdj3));
		
		value_ext |= ACC_DVFM_ST_DVFM_MAX_ADJ(pmgr_get_offset_from_diff_uV(BUCK_CPU, adj->dvfmMaxAdj));
		value_ext |= ACC_DVFM_ST_DVMR_ADJ0(pmgr_get_offset_from_diff_uV(BUCK_CPU, adj->dvmrAdj0));
		value_ext |= ACC_DVFM_ST_DVMR_ADJ1(pmgr_get_offset_from_diff_uV(BUCK_CPU, adj->dvmrAdj1));
		value_ext |= ACC_DVFM_ST_DVMR_ADJ2(pmgr_get_offset_from_diff_uV(BUCK_CPU, adj->dvmrAdj2));

		if (state < 8) {
			rACC_NRG_ACC_SCALE_TAB &= ~((uint64_t)ACC_PWRCTL_NRG_ACC_SCALE_TAB_DVFM_ST0_SCALE_UMASK << (state * 8));
			rACC_NRG_ACC_SCALE_TAB |= (uint64_t)params->nrgAccScaleTab << (state * 8);
			rACC_LKG_EST_TAB &= ~((uint64_t)ACC_PWRCTL_LKG_EST_TAB_DVFM_ST0_LKG_UMASK << (state * 8));
			rACC_LKG_EST_TAB |= (uint64_t)params->lkgEstTab << (state * 8);
			rACC_DPE0_DVFM_TAB &= ~((uint64_t)ACC_PWRCTL_DPE0_DVFM_TAB_DVFM_ST0_THRESH_UMASK << (state * 8));
			rACC_DPE0_DVFM_TAB |= (uint64_t)params->dpe0DvfmTab << (state * 8);
		}
		else if (state < 16) {
			rACC_NRG_ACC_SCALE_TAB_EXT &= ~((uint64_t)ACC_PWRCTL_NRG_ACC_SCALE_TAB_EXT_DVFM_ST8_SCALE_UMASK << ((state - 8) * 8));
			rACC_NRG_ACC_SCALE_TAB_EXT |= (uint64_t)params->nrgAccScaleTab << ((state - 8) * 8);
			rACC_LKG_EST_TAB_EXT &= ~((uint64_t)ACC_PWRCTL_LKG_EST_TAB_EXT_DVFM_ST8_LKG_UMASK << ((state - 8) * 8));
			rACC_LKG_EST_TAB_EXT |= (uint64_t)params->lkgEstTab << ((state - 8) * 8);
			rACC_DPE0_DVFM_TAB_EXT &= ~((uint64_t)ACC_PWRCTL_DPE0_DVFM_TAB_DVFM_ST0_THRESH_UMASK << ((state - 8) * 8));
			rACC_DPE0_DVFM_TAB_EXT |= (uint64_t)params->dpe0DvfmTab << ((state - 8) * 8);
		}
	}

	rACC_DVFM_ST(state) = value;
	rACC_DVFM_ST_EXT(state) = value_ext;
}

static	uint32_t get_apsc_acc_state(void)
{
	return ACC_PWRCTL_DVFM_CFG_SEL_CUR_CFG_XTRCT(rACC_DVFM_CFG_SEL);
}

static void set_apsc_acc_state(uint32_t target_state)
{
	// DWI clock must be enabled for any state change that has voltage control enabled.
	uint64_t apsc_scr = rACC_APSC_SCR;
	apsc_scr &=  ~ACC_PWRCTL_APSC_SCR_MANUAL_ST_UMASK;
	rACC_APSC_SCR = apsc_scr | ACC_APSC_MANUAL_CHANGE(target_state);
	while ((rACC_APSC_SCR & ACC_APSC_PENDING) != 0);

	return;
}

static void enable_cpu_pll_reset(bool enable)
{
	uint64_t apsc_scr = rACC_APSC_SCR;

	if (enable)
		apsc_scr &= ~ACC_PWRCTL_APSC_SCR_DIS_RESET_PLL_IN_SLEEP_UMASK;
	else
		apsc_scr |= ACC_PWRCTL_APSC_SCR_DIS_RESET_PLL_IN_SLEEP_UMASK;

	rACC_APSC_SCR = apsc_scr;
}

static void config_soc_perf_state(uint32_t state, enum chipid_voltage_index voltage_index)
{
	uint32_t dwi_val = 0;

	dwi_val = chipid_get_soc_voltage(voltage_index);
	platform_convert_voltages(BUCK_SOC, 1, &dwi_val);

#if !APPLICATION_SECUREROM && !SUB_PLATFORM_S8001
	const struct pmgr_binning_vol_adj *adj;
#endif
	uint32_t vol_adj_temp = 0;

	rPMGR_SOC_PERF_STATE_ENTRY_A(PMGR_SOC_PERF_STATE_TO_ENTRY(state)) = pmgr_soc_perf_states[voltage_index].entry[0];
	rPMGR_SOC_PERF_STATE_ENTRY_B(PMGR_SOC_PERF_STATE_TO_ENTRY(state)) = pmgr_soc_perf_states[voltage_index].entry[1];
	rPMGR_SOC_PERF_STATE_ENTRY_C(PMGR_SOC_PERF_STATE_TO_ENTRY(state)) = pmgr_soc_perf_states[voltage_index].entry[2] | PMGR_SOC_PERF_STATE_ENTRY_VOLTAGE(dwi_val);

#if !APPLICATION_SECUREROM && !SUB_PLATFORM_S8001
	adj =  pmgr_binning_get_vol_adj(chipid_get_chip_id(), chipid_get_chip_revision(), voltage_index);
	vol_adj_temp |= PMGR_SOC_PERF_STATE_ENTRY_0D_REGION0_VOL_OFFSET_INSRT(pmgr_get_offset_from_diff_uV(BUCK_SOC, adj->volAdj0));
	vol_adj_temp |= PMGR_SOC_PERF_STATE_ENTRY_0D_REGION1_VOL_OFFSET_INSRT(pmgr_get_offset_from_diff_uV(BUCK_SOC, adj->volAdj1));
	vol_adj_temp |= PMGR_SOC_PERF_STATE_ENTRY_0D_REGION2_VOL_OFFSET_INSRT(pmgr_get_offset_from_diff_uV(BUCK_SOC, adj->volAdj2));
	vol_adj_temp |= PMGR_SOC_PERF_STATE_ENTRY_0D_REGION3_VOL_OFFSET_INSRT(pmgr_get_offset_from_diff_uV(BUCK_SOC, adj->volAdj3));
#endif

	rPMGR_SOC_PERF_STATE_ENTRY_D(PMGR_SOC_PERF_STATE_TO_ENTRY(state)) = vol_adj_temp;
}

static void set_soc_perf_state(uint32_t target_state)
{
	rPMGR_SOC_PERF_STATE_CTL = PMGR_SOC_PERF_STATE_TO_ENTRY(target_state);
	while (rPMGR_SOC_PERF_STATE_CTL & PMGR_SOC_PERF_STATE_CTL_PENDING_MASK);
}

static void set_lpo(void)
{
	uint32_t cfg;
	
	cfg = rMINIPMGR_LPO_CFG;
	cfg &= ~MINIPMGR_LPO_CFG_TRIM_UMASK;
	cfg |= chipid_get_lpo_trim() << MINIPMGR_LPO_CFG_TRIM_SHIFT;
#if SUB_PLATFORM_S8003
	// <rdar://problem/20185169> Workaround for LPO issue - iboot
	if (chipid_get_lpo_trim() == 0) {
		cfg |= 0x9b << MINIPMGR_LPO_CFG_TRIM_SHIFT;
		cfg &= ~MINIPMGR_LPO_CFG_LOCK_TIME_UMASK;
		cfg |= 0x62 >> MINIPMGR_LPO_CFG_LOCK_TIME_SHIFT;
	}
#endif
	rMINIPMGR_LPO_CFG = cfg;

	rMINIPMGR_LPO_CTL = MINIPMGR_LPO_CTL_ENABLE;

	while (rMINIPMGR_LPO_CTL & MINIPMGR_LPO_CTL_PENDING);
}

/*
 * set_pll - called by SecureROM, LLB, iBSS with PLLs in default reset state.
 * See restore_clock_config_state().
 */
static void set_pll(int32_t pll, uint32_t p, uint32_t m, uint32_t s)
{
	if (pll >= PMGR_PLL_COUNT)
		panic("Invalid PLL %u", pll);

	if (pll != PLL_PCIE) {
		rPMGR_PLL_CTL(pll) = PMGR_PLL_ENABLE | PMGR_PLL_LOAD | PMGR_PLL_P(p) | PMGR_PLL_M(m) | PMGR_PLL_S(s);
	} else {
		uint32_t delay_ctl;
		
#if !APPLICATION_SECUREROM		
		uint32_t cfg_params;

		// Set PCIE_PLL Lock Mode Tunable. (Maui and Elba)
		cfg_params = rPMGR_PLL_CFG(pll);
		cfg_params &= ~PMGR_PLL_PCIE_CFG_LOCK_MODE_UMASK;
		cfg_params |= PMGR_PLL_LOCK_MODE(PMGR_PLL_LOCK_MODE_LOCK);
		rPMGR_PLL_CFG(pll) = cfg_params;
#endif

#if SUB_PLATFORM_S8000
		uint32_t ana_params;
		// The PCIe PLL needs data loaded from the fuses (<rdar://problem/16410810>)
		ana_params = rPMGR_PLL_ANA_PARAMS2(pll);
		ana_params &= ~(PMGR_PLL_PCIE_ANA_PARAMS2_V2I_I_SET_UMASK | PMGR_PLL_PCIE_ANA_PARAMS2_V2I_PI_SET_UMASK);
		ana_params |= PMGR_PLL_PCIE_ANA_PARAMS2_V2I_PI_SET_INSRT(chipid_get_pcie_refpll_vco_v2i_pi_set());
		ana_params |= PMGR_PLL_PCIE_ANA_PARAMS2_V2I_I_SET_INSRT(chipid_get_pcie_refpll_vco_v2i_i_set());
		rPMGR_PLL_ANA_PARAMS2(pll) = ana_params;

		// <rdar://problem/16513606> Maui RefPLL new default value for pll_lpf_c3 - update tunables
		ana_params = rPMGR_PLL_ANA_PARAMS3(pll);
		ana_params &= ~PMGR_PLL_PCIE_ANA_PARAMS3_LPF_C3_SEL_UMASK;
		rPMGR_PLL_ANA_PARAMS3(pll) = ana_params;
		
#elif SUB_PLATFORM_S8001 || SUB_PLATFORM_S8003
		uint32_t ana_params;
		// The PCIe PLL needs data loaded from the fuses (<rdar://problem/17602718>/<rdar://problem/17925034>)
		ana_params = rPMGR_PLL_ANA_PARAMS3(pll);
		ana_params &= ~(PMGR_PLL_PCIE_ANA_PARAMS3_FCAL_VCODIGCTRL_UMASK |
				PMGR_PLL_PCIE_ANA_PARAMS3_FCAL_BYPASS_UMASK);
		ana_params |= PMGR_PLL_PCIE_ANA_PARAMS3_FCAL_VCODIGCTRL_INSRT(chipid_get_pcie_refpll_fcal_vco_digctrl()) | 
			      PMGR_PLL_PCIE_ANA_PARAMS3_FCAL_BYPASS_INSRT(1);
		rPMGR_PLL_ANA_PARAMS3(pll) = ana_params;
#endif
		// <rdar://problem/16654219> Fix up PCIE PLL timing register descriptions
		delay_ctl = rPMGR_PLL_PCIE_DELAY_CTL0(pll);
		delay_ctl &= ~PMGR_PLL_PCIE_PLL_DELAY_CTL0_PLL_PREUPDATE_TIME_SMASK;
		delay_ctl |= PMGR_PLL_PCIE_PLL_DELAY_CTL0_PLL_PREUPDATE_TIME_INSRT(0xc);
		rPMGR_PLL_PCIE_DELAY_CTL0(pll) = delay_ctl;

		rPMGR_PLL_CTL(pll) = PMGR_PLL_ENABLE | PMGR_PLL_LOAD | PMGR_PLL_M(m) | PMGR_PLL_PCIE_S(s);
	}

	while (rPMGR_PLL_CTL(pll) & PMGR_PLL_PENDING);

	if (pll == PLL_PCIE) {
		rPMGR_MISC_PCIE_PCIE_CTL = rPMGR_MISC_PCIE_PCIE_CTL | PMGR_MISC_PCIE_PCIE_CTL_OFF_MODE_OVERRIDE_INSRT(1);
	}
}

static uint32_t get_pll_cpu(void)
{
	uint32_t freq;
	uint32_t pllctl;

	pllctl = rACC_PLL_SCR1;

	// Fcpu <= ((OSC * M) / P / S+1)
	freq = OSC_FREQ;
	freq *= ACC_PWRCTL_PLL_SCR1_FB_DIVN_XTRCT(pllctl);
	freq /= ACC_PWRCTL_PLL_SCR1_PRE_DIVN_XTRCT(pllctl);
	freq /= 1 + ACC_PWRCTL_PLL_SCR1_OP_DIVN_XTRCT(pllctl);

	return freq;
}

static uint32_t get_pll(int32_t pll)
{
	uint32_t pllctl;
	uint32_t opdiv;
	uint64_t freq = OSC_FREQ;

	pllctl = rPMGR_PLL_CTL(pll);

	if ((pllctl & PMGR_PLL_ENABLE) == 0) {
		return 0;
	}
	else if (pllctl & PMGR_PLL_BYPASS) {
		return freq;
	}

	freq *= ((pllctl >> PMGR_PLL_M_SHIFT) & PMGR_PLL_M_MASK);

	if (pll != PLL_PCIE) {
		opdiv = ((pllctl >> PMGR_PLL_S_SHIFT) & PMGR_PLL_S_MASK);
		freq /= opdiv + 1;
		freq /= ((pllctl >> PMGR_PLL_P_SHIFT) & PMGR_PLL_P_MASK);
	} else {
		opdiv = ((pllctl >> PMGR_PLL_S_SHIFT) & PMGR_PLL_PCIE_S_MASK);
		if (opdiv >= 3 && opdiv <= 7) {
			freq /= 16;
		} else if (opdiv > 7) {
			freq /= 2 * opdiv;
		}
	}

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

	div = reg_val & PMGR_CLK_CFG_DIVISOR_MASK;

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
	clks[PMGR_CLK_USB] = 12000000;
	clks[PMGR_CLK_LPO] = LPO_FREQ;

#elif SUB_TARGET_S8000SIM || SUB_TARGET_S8001SIM
	uint32_t cnt;
	uint32_t freq = OSC_FREQ;

	for (cnt = 0; cnt < PMGR_CLK_COUNT; cnt++)
		clks[cnt] = freq;

#else
	uint32_t cnt;

	clks[PMGR_CLK_OSC] = OSC_FREQ;
	clks[PMGR_CLK_LPO] = LPO_FREQ;

	// Use get_pll() to establish the frequencies (unconfigured PLLs will bypass OSC)
	for (cnt = 0; cnt < PMGR_PLL_COUNT; cnt++)
		clks[PMGR_CLK_PLL0 + cnt] = get_pll(cnt);

	// Use get_spare() to establish the frequencies for spare clocks (unconfigured will be skipped)
	for (cnt = 0; cnt < PMGR_SPARE_COUNT; cnt++)
		clks[PMGR_CLK_S0 + cnt] = get_spare(cnt);

	clks[PMGR_CLK_CPU] = get_pll_cpu();

	clocks_get_frequencies_range(PMGR_CLK_MINI_FIRST, PMGR_CLK_MINI_LAST);
	clocks_get_frequencies_range(PMGR_CLK_FIRST, PMGR_CLK_LAST);
#endif
}

static void clocks_get_frequencies_range(uint32_t start_clk, uint32_t end_clk)
{
	volatile uint32_t *reg;
	uint32_t cnt, val, src_idx, src_clk, src_factor;

	if ((start_clk >= PMGR_CLK_FIRST && end_clk <= PMGR_CLK_LAST) ||
		(start_clk >= PMGR_CLK_MINI_FIRST && end_clk <= PMGR_CLK_MINI_LAST)) {
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
	volatile uint32_t *reg;
	uint32_t cnt;

	for (cnt = first; cnt <= last; cnt++) {
		reg = clk_configs[cnt].clock_reg;
		pmgr_set_clkcfg(reg, clkdata);
	}
}

static void clock_update_frequency(uint32_t clk, uint32_t freq)
{
#define ROUND_10E4(_x) ((((_x) + 5000) / 10000) * 10000)

	uint32_t src_idx, reg, i;
	bool freq_supported = false;
	volatile uint32_t *clkcfg = clk_configs[clk].clock_reg;

	if (freq == 0)
		return;

	for (i = 0; i < CLOCK_SOURCES_MAX && clk_configs[clk].sources[i].factor != 0; i++)
	{
		uint32_t src_clk = clk_configs[clk].sources[i].src_clk;
		uint32_t src_factor = clk_configs[clk].sources[i].factor;

		if (ROUND_10E4(clks[src_clk] / src_factor) == freq) {
			freq_supported = true;
			src_idx = i; // Choose latest one to pick up PLL instead of Si if available.
			break;
		}	
	}
	
	if (freq_supported) {
		// Configure clock
		uint32_t i;
		uint32_t j;
		uint32_t size = sizeof(pmgr_soc_perf_state_src_sels)/sizeof(pmgr_soc_perf_state_src_sels[0]);
		
		for (i = 0; i < size; i++) {
			if (pmgr_soc_perf_state_src_sels[i].clk_index == clk) {
				break;
			}
		}
		
		if (i < size) {
			for (j = kSOC_PERF_STATE_IBOOT; j < kSOC_PERF_STATE_MAX_CNT; j++) {
				volatile uint32_t *soc_perf_state_entry = NULL;
				volatile uint32_t soc_perf_state_entry_value;
				switch (pmgr_soc_perf_state_src_sels[i].entry) {
					case 'A':
						soc_perf_state_entry = &rPMGR_SOC_PERF_STATE_ENTRY_A(j);
						break;
					case 'B':
						soc_perf_state_entry = &rPMGR_SOC_PERF_STATE_ENTRY_B(j);
						break;
					case 'C':
						soc_perf_state_entry = &rPMGR_SOC_PERF_STATE_ENTRY_C(j);
						break;
					default:
						panic("Unknwon entry\n");
						break;
				}
				soc_perf_state_entry_value = *soc_perf_state_entry;
				soc_perf_state_entry_value &= ~pmgr_soc_perf_state_src_sels[i].mask;
				soc_perf_state_entry_value |= (src_idx << pmgr_soc_perf_state_src_sels[i].shift) & pmgr_soc_perf_state_src_sels[i].mask;
				*soc_perf_state_entry = soc_perf_state_entry_value;
			}
		}
		reg = *clkcfg;
		reg &= ~(PMGR_CLK_CFG_SRC_SEL_MASK << PMGR_CLK_CFG_SRC_SEL_SHIFT);
		reg |= (src_idx & PMGR_CLK_CFG_SRC_SEL_MASK) << PMGR_CLK_CFG_SRC_SEL_SHIFT;
		pmgr_set_clkcfg(clkcfg, reg);
	}
}

static void restore_clock_config_state(void)
{
	uint32_t cnt;
	uint32_t current_select, entry_a;

	// 2. Write reset value to ACG, CLK_DIVIDER_ACG_CFG
	rPMGR_MISC_CFG_ACG = 0;
	rPMGR_CLK_DIVIDER_ACG_CFG = 0;

	// 3. Write the reset value to all MCAx_CLK_CFG registers
	clock_update_range(PMGR_CLK_MCA0_M, PMGR_CLK_MCA4_M, 0x80100000);

	// 4. Put the NCOs in reset state
	for (cnt = 0; cnt < PMGR_NUM_NCO; cnt++)
	{
		rPMGR_NCO_CLK_CFG(cnt) = 0;
		while (rPMGR_NCO_CLK_CFG(cnt) & PMGR_NCO_CLK_CFG_PENDING);
	}

	// 5a. Write reset value for all mux clock configs (excluding spares)
	clock_update_range(PMGR_CLK_FIRST, PMGR_CLK_TMPS - 1, 0x80100000);
	clock_update_range(PMGR_CLK_TMPS, PMGR_CLK_TMPS, 0x85100000);
	clock_update_range(PMGR_CLK_TMPS + 1, PMGR_CLK_LAST, 0x80100000);

	// Since AOP config engine will use SOC perf table entries 0 and 1, we start at the other end of the table to avoid stepping on each other.

	// 5b. Write the desired DRAM clock configuration state into the SOC_PERF_STATE_ENTRY_xA register
	current_select = PMGR_SOC_PERF_STATE_CTL_CURRENT_SELECT_XTRCT(rPMGR_SOC_PERF_STATE_CTL);
	entry_a = pmgr_soc_perf_states[kSOC_PERF_STATE_BYPASS].entry[0];
	entry_a &= ~PMGR_SOC_PERF_STATE_ENTRY_MCU_REF_MASK;
	entry_a |= rPMGR_SOC_PERF_STATE_ENTRY_A(current_select) & PMGR_SOC_PERF_STATE_ENTRY_MCU_REF_MASK;

	rPMGR_SOC_PERF_STATE_ENTRY_A(PMGR_SOC_PERF_STATE_TO_ENTRY(kSOC_PERF_STATE_BYPASS)) = entry_a;

	// 5c. Write the reset value to all other fields in ENTRY_xA
	rPMGR_SOC_PERF_STATE_ENTRY_B(PMGR_SOC_PERF_STATE_TO_ENTRY(kSOC_PERF_STATE_BYPASS)) = pmgr_soc_perf_states[kSOC_PERF_STATE_BYPASS].entry[1];
	rPMGR_SOC_PERF_STATE_ENTRY_C(PMGR_SOC_PERF_STATE_TO_ENTRY(kSOC_PERF_STATE_BYPASS)) = pmgr_soc_perf_states[kSOC_PERF_STATE_BYPASS].entry[2];
	rPMGR_SOC_PERF_STATE_ENTRY_D(PMGR_SOC_PERF_STATE_TO_ENTRY(kSOC_PERF_STATE_BYPASS)) = pmgr_soc_perf_states[kSOC_PERF_STATE_BYPASS].entry[3];

	// 6. Write the reset value to the SOC_PERF_STATE_CTL register
	set_soc_perf_state(kSOC_PERF_STATE_BYPASS);

	// 7. Write the reset values to the SOC_PERF_STATE entry registers, except for ENTRY_xA
	for (cnt = PMGR_SOC_PERF_STATE_FIRST_ENTRY; cnt < PMGR_SOC_PERF_STATE_ENTRY_COUNT; cnt++) {
		if (cnt == PMGR_SOC_PERF_STATE_TO_ENTRY(kSOC_PERF_STATE_BYPASS))
			continue;

		rPMGR_SOC_PERF_STATE_ENTRY_A(cnt) = 0;
		rPMGR_SOC_PERF_STATE_ENTRY_B(cnt) = 0;
		rPMGR_SOC_PERF_STATE_ENTRY_C(cnt) = 0;
		rPMGR_SOC_PERF_STATE_ENTRY_D(cnt) = 0;
	}

	// 11. Write reset value to all PLLx_CTL
	for (cnt = 0; cnt < PMGR_PLL_COUNT; cnt++) {
		if ((cnt == 0) || (cnt == 3))
			continue; // Mem PLL

		if (cnt != PLL_PCIE) {
			rPMGR_PLL_CTL(cnt) = PMGR_PLL_ENABLE | PMGR_PLL_BYPASS | PMGR_PLL_M(1) | PMGR_PLL_P(1); // fb_div = 1, pre_div = 1
		}
		else {
			rPMGR_PLL_CTL(cnt) = PMGR_PLL_M(0xc8) | PMGR_PLL_PCIE_S(0x18); // fb_div = 0xc8, op_div = 0x18
		}
		
		while (rPMGR_PLL_CTL(cnt) & PMGR_PLL_PENDING);
	}

	// 12. Write reset value to all other PLL registers
	for (cnt = 0; cnt < PMGR_PLL_COUNT; cnt++) {
		if (cnt == 0)
			continue; // Mem PLL

		rPMGR_PLL_CFG(cnt) = PMGR_PLL_OFF_MODE(PMGR_PLL_OFF_MODE_POWER_DOWN) |
			PMGR_PLL_FRAC_LOCK_TIME(0x48) | PMGR_PLL_LOCK_TIME(0x348);
	}

	// 13. Write reset value to spare and ISP_REF0/1
	clock_update_range(PMGR_CLK_SPARE_FIRST, PMGR_CLK_SPARE_LAST, 0x80000001);

	// 14. Put CPU clock back into its reset default.
	set_apsc_acc_state(kDVFM_STATE_BYPASS);

	// Mini-PMGR

	// 2. Write the reset value to the CLK_DIVIDER_ACG_CFG register
	rMINIPMGR_CLK_DIVIDER_ACG_CFG = 0;

	// 3. Write the reset value to all mux clock config registers
	clock_update_range(PMGR_CLK_MINI_FIRST, PMGR_CLK_MINI_LAST, 0x80100000);

	// 4. Write the reset value to LPO_CTL register
	rMINIPMGR_LPO_CTL = PMGR_PLL_ENABLE | PMGR_PLL_BYPASS;

	// 5. Write the reset value to LPO_CFG register
	rMINIPMGR_LPO_CFG = 0x40000010;
	while (rMINIPMGR_LPO_CTL & PMGR_PLL_PENDING);

	// 6. Write the reset value to the MISC_CFG_ACG register
	rMINIPMGR_MISC_CFG_ACG = 0;
}


static void power_on_sep(void)
{
	volatile uint32_t *reg;
	uint32_t val;

	reg = device_configs[PMGR_DEVICE_INDEX(CLK_SEP)].ps_reg;

	val = *reg;
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
	rPMGR_VOLMAN_CTL |= (PMGR_VOLMAN_DISABLE_CPU_VOL_CHANGE |
		PMGR_VOLMAN_DISABLE_GFX_VOL_CHANGE |
		PMGR_VOLMAN_DISABLE_CPU_SRAM_VOL_CHANGE |
		PMGR_VOLMAN_DISABLE_GFX_SRAM_VOL_CHANGE |
		PMGR_VOLMAN_DISABLE_SOC_VOL_CHANGE);

	// The following devices need to be on
	set_device(devices, CLK_AOP);
	set_device(devices, CLK_DEBUG);
	set_device(devices, CLK_AOP_GPIO);
	set_device(devices, CLK_AOP_CPU);
	set_device(devices, CLK_AOP_FILTER);
	set_device(devices, CLK_AOP_BUSIF);
	set_device(devices, CLK_SBR);
	set_device(devices, CLK_AIC);
	set_device(devices, CLK_DWI);
	set_device(devices, CLK_GPIO);
	set_device(devices, CLK_PMS);
	set_device(devices, CLK_PMS_SRAM); // Until <rdar://problem/18858472> is fixed
	set_device(devices, CLK_SIO_BUSIF);
	set_device(devices, CLK_SIO_P);
	set_device(devices, CLK_SIO);
	set_device(devices, CLK_MCC);
	set_device(devices, CLK_DCS0);
	set_device(devices, CLK_DCS1);
	set_device(devices, CLK_DCS2);
	set_device(devices, CLK_DCS3);
#if SUB_PLATFORM_S8001 && DCS_NUM_CHANNELS > 4
	set_device(devices, CLK_DCS4);
	set_device(devices, CLK_DCS5);
	set_device(devices, CLK_DCS6);
	set_device(devices, CLK_DCS7);
#endif	
	set_device(devices, CLK_USB);
	set_device(devices, CLK_USBCTLREG);
	set_device(devices, CLK_USB_OTG);
	set_device(devices, CLK_SMX);
	set_device(devices, CLK_SF);

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
	uint32_t acc_state = get_apsc_acc_state();

	if (acc_state != active_state)
		set_apsc_acc_state(active_state);

#if APPLICATION_IBOOT
	uint32_t entry_a;
	
	// Enable SOC voltage changes
	rPMGR_VOLMAN_CTL &= ~PMGR_VOLMAN_DISABLE_SOC_VOL_CHANGE;

#if SUB_PLATFORM_S8001
	if (performance_level == kPerformanceMemoryFull) {
		performance_level = kPerformanceMemoryMid;
	}
#endif
	
	switch (performance_level) {
		case kPerformanceMemoryLow:
		case kPerformanceMemoryMid:
			entry_a = rPMGR_SOC_PERF_STATE_ENTRY_A(PMGR_SOC_PERF_STATE_TO_ENTRY(kSOC_PERF_STATE_VMIN));
			entry_a &= ~PMGR_SOC_PERF_STATE_ENTRY_MCU_REF_MASK;

#if SUB_PLATFORM_S8000 || SUB_PLATFORM_S8003
			if (performance_level == kPerformanceMemoryLow)
				entry_a |= PMGR_SOC_PERF_STATE_ENTRY_MCU_REF(0x3, 0x8);
			else {
				if (get_pll(3) > OSC_FREQ) {
					entry_a |= PMGR_SOC_PERF_STATE_ENTRY_MCU_REF(0x1, 0x5); // 792 MHz MCU Clock
				} else {
					entry_a |= PMGR_SOC_PERF_STATE_ENTRY_MCU_REF(0x1, 0x7); // 800 MHz MCU Clock
				}
			}
#elif SUB_PLATFORM_S8001
			if (performance_level == kPerformanceMemoryLow)
				entry_a |= PMGR_SOC_PERF_STATE_ENTRY_MCU_REF(0x3, 0x8);
			else {
				if (get_pll(3) > OSC_FREQ) {
					entry_a |= PMGR_SOC_PERF_STATE_ENTRY_MCU_REF(0x0, 0x5); // 1588 MHz MCU CLock
				} else {
					entry_a |= PMGR_SOC_PERF_STATE_ENTRY_MCU_REF(0x0, 0x6); // 1588 MHz MCU CLock
				}
			}
#endif

			rPMGR_SOC_PERF_STATE_ENTRY_A(PMGR_SOC_PERF_STATE_TO_ENTRY(kSOC_PERF_STATE_VMIN)) = entry_a;
			
			set_soc_perf_state(kSOC_PERF_STATE_VMIN);
			perf_level = performance_level;
			break;
			
		case kPerformanceMemoryFull:

			// VNOM: Copy MCU_REF SRC_SEL from VMIN
			if ((chipid_get_chip_id() == 0x8003) || (chipid_get_chip_id() == 0x8000)) {
				uint32_t mcu_src_sel;
				entry_a = rPMGR_SOC_PERF_STATE_ENTRY_A(PMGR_SOC_PERF_STATE_TO_ENTRY(kSOC_PERF_STATE_VMIN));
				mcu_src_sel = PMGR_SOC_PERF_STATE_ENTRY_0A_MCU_REF_SRC_SEL_XTRCT(entry_a);

				entry_a = rPMGR_SOC_PERF_STATE_ENTRY_A(PMGR_SOC_PERF_STATE_TO_ENTRY(kSOC_PERF_STATE_VNOM));
				entry_a &= ~PMGR_SOC_PERF_STATE_ENTRY_0A_MCU_REF_SRC_SEL_UMASK;
				entry_a |= PMGR_SOC_PERF_STATE_ENTRY_0A_MCU_REF_SRC_SEL_INSRT(mcu_src_sel);
				rPMGR_SOC_PERF_STATE_ENTRY_A(PMGR_SOC_PERF_STATE_TO_ENTRY(kSOC_PERF_STATE_VNOM)) = entry_a;
			}

			set_soc_perf_state(kSOC_PERF_STATE_VNOM);
			perf_level = performance_level;
			break;
			
		default:
			break;
	}
	
	// Disable SOC voltage changes
	rPMGR_VOLMAN_CTL |= PMGR_VOLMAN_DISABLE_SOC_VOL_CHANGE;
	
	clocks_get_frequencies_range(PMGR_CLK_MCU_REF, PMGR_CLK_MCU_REF);
#endif

	// At this point we should have ACC clock set for this stage of boot.
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
#if SUPPORT_FPGA
		freq = 1000000;
#elif SUB_TARGET_S8000SIM || SUB_TARGET_S8001SIM
		freq = OSC_FREQ;
#else
		// XXX Frequency is a function of MCU_REF_CLK and MCU_REF_CFG_SEL
		freq = clks[PMGR_CLK_MCU_REF];
#endif
		break;
	case CLK_BUS:
		freq = clks[PMGR_CLK_SBR];
		break;
	case CLK_CPU:
		freq = clks[PMGR_CLK_CPU];
		break;
#if SUB_PLATFORM_S8000 || SUB_PLATFORM_S8003
	case CLK_MIPI:
		freq = clks[PMGR_CLK_TEMP_MIPI_DSI];
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
#if SUB_PLATFORM_S8000 || SUB_PLATFORM_S8003
	case CLK_MIPI:
		clk = PMGR_CLK_PLL2;
		break;
#endif
	case CLK_VCLK0:
		clk = PMGR_CLK_VID0;
		break;

	default:
		break;
	}

	if (clk >= PMGR_CLK_PLL0 && clk <= PMGR_CLK_PLL5) {
		int32_t pll = clk - PMGR_CLK_PLL0;

		set_pll(pll, pll_p, pll_m, pll_s);

		clks[clk] = get_pll(pll);
#if SUB_PLATFORM_S8000 || SUB_PLATFORM_S8003
		if (clock == CLK_MIPI) {
			clocks_get_frequencies_range(PMGR_CLK_TEMP_MIPI_DSI, PMGR_CLK_TEMP_MIPI_DSI);
		}
#endif
	}

	if (clk >= PMGR_CLK_FIRST && clk <= PMGR_CLK_LAST) {
		clock_update_frequency(clk, pll_t);
		clocks_get_frequencies_range(clk, clk);
	}

	return;
}

void clock_gate(int device, bool enable)
{
	volatile uint32_t *reg;

	// Make sure we are within limits.
	if (!PMGR_VALID_DEVICE(device))
		return;

	reg = device_configs[PMGR_DEVICE_INDEX(device)].ps_reg;

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

	// Turn on devices from lo to hi (to meet dependencies).
	for (idx = 0, i = PMGR_FIRST_DEVICE; i <= PMGR_LAST_DEVICE; i++) {
		if (i && ((i % 64) == 0))
			idx++;
		if ((devices[idx] >> ((uint64_t)(i % 64))) & 0x1)
			clock_gate(i, true);
	}

	// Turn off devices hi to lo order (to meet dependencies).
	for (idx = 1, i = PMGR_LAST_DEVICE; i >= PMGR_FIRST_DEVICE; i--) {
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

#if APPLICATION_IBOOT
	// <rdar://problem/22226525>
	// Due to a silicon issue, doing a chip reset (as opposed to a system reset) on Maui/Malta
	// may lead to LLB failing to initialize DRAM and panic, resulting in an infinite panic loop
	// (since _panic calls this reset function, not platform_system_reset)
	//
	// So, for S8000 family, we will upgrade all chip resets to a system reset to prevent a panic loop.
	//
	// Also, this may go away after <rdar://problem/22299338> Should iBoot panic always reset via wdt_system_reset instead of wdt_chip_reset?
	wdt_system_reset();
#else
	wdt_chip_reset();
#endif

	while (1);
}

void platform_power_init(void)
{
#if (APPLICATION_IBOOT && !PRODUCT_IBOOT && !PRODUCT_IBEC)
	init_thermal_registers();
#endif
}

void clock_reset_device(int device)
{
	volatile uint32_t *reg;

	// Make sure we are within limits.
	if (!PMGR_VALID_DEVICE(device))
		return;

	reg = device_configs[PMGR_DEVICE_INDEX(device)].ps_reg;

	switch (device) {
	case CLK_SIO:
	case CLK_EDPLINK:
		*reg |= PMGR_PS_RESET;
		spin(1);
		*reg &= ~PMGR_PS_RESET;
		break;

	default:
		break;
	}
}

void clock_set_device_reset(int device, bool set)
{
	volatile uint32_t *reg;

	// Make sure we are within limits.
	if (!PMGR_VALID_DEVICE(device))
		return;

	reg = device_configs[PMGR_DEVICE_INDEX(device)].ps_reg;

	switch (device) {
	case CLK_PCIE:
		if (set)
			*reg |= PMGR_PS_RESET;
		else
			*reg &= ~PMGR_PS_RESET;
	}
}

bool clock_get_pcie_refclk_good(void)
{
	// XXX JF: new register and bit number for S8000?
	return (rPMGR_DEBUG_PMGR_DEBUG17 & (1 << 20)) != 0;
}

#if APPLICATION_IBOOT
static void set_gfx_perf_state(uint32_t state_num, enum chipid_voltage_index voltage_index)
{
	const struct pmgr_binning_vol_adj *vol_adj = pmgr_binning_get_vol_adj(chipid_get_chip_id(), chipid_get_chip_revision(), voltage_index);
	uint32_t pll_enable = 0;
	const struct operating_point_params *params;
	params = operating_point_get_params(voltage_index, CHIPID_GPU_VOLTAGE);
	uint32_t dwi_val = 0;
	uint32_t dwi_sram_val = 0;

	if (voltage_index != CHIPID_GPU_VOLTAGE_OFF) {
		dwi_val = chipid_get_gpu_voltage(voltage_index);
		platform_convert_voltages(BUCK_GPU, 1, &dwi_val);
		dwi_sram_val = chipid_get_gpu_sram_voltage(voltage_index);
		platform_convert_voltages(BUCK_GPU_RAM, 1, &dwi_sram_val);
	}


	// This is deductive. If feedback divider is 0 the PLL shouldn't output anything.
	pll_enable = params->fbkDivM ? 1 : 0;

	rPMGR_GFX_PERF_STATE_ENTRY_A(state_num) = ((dwi_val & 0xFF) << 24) |
	((pll_enable & 0x1) << 21) |
	((params->fbkDivM & 0x1FF) << 12) |
	((params->preDivP & 0x1F) << 4) |
	(params->pstDivS & 0xF);
	
	rPMGR_GFX_PERF_STATE_ENTRY_B(state_num) = dwi_sram_val & 0xFF;

	if (vol_adj != NULL) {
		uint32_t entry_c = 0;
		entry_c |= PMGR_GFX_PERF_STATE_ENTRY0C_REGION0_VOL_OFFSET_INSRT(pmgr_get_offset_from_diff_uV(BUCK_GPU, vol_adj->volAdj0));
		entry_c |= PMGR_GFX_PERF_STATE_ENTRY0C_REGION1_VOL_OFFSET_INSRT(pmgr_get_offset_from_diff_uV(BUCK_GPU, vol_adj->volAdj1));
		entry_c |= PMGR_GFX_PERF_STATE_ENTRY0C_REGION2_VOL_OFFSET_INSRT(pmgr_get_offset_from_diff_uV(BUCK_GPU, vol_adj->volAdj2));
		entry_c |= PMGR_GFX_PERF_STATE_ENTRY0C_REGION3_VOL_OFFSET_INSRT(pmgr_get_offset_from_diff_uV(BUCK_GPU, vol_adj->volAdj3));
		rPMGR_GFX_PERF_STATE_ENTRY_C(state_num) = entry_c;
	}

	return;
}
#endif

#if WITH_DEVICETREE
static uint64_t get_freq_from_acc_state(uint32_t state_index)
{
	uint64_t state_entry = 	rACC_DVFM_ST(state_index);
	uint64_t freq = OSC_FREQ;

	// Fcpu <= ((OSC * M) / P / S+1)
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
		.voltage_states1_size = kVOLTAGE_STATES1_SIZE
	};

#if SUB_PLATFORM_S8000
	uint32_t vco_hack = 0;
	uint32_t index = 0;
	uint32_t i, j;

#if (APPLICATION_IBOOT && PRODUCT_IBEC)
	if (rACC_DVFM_ST(kDVFM_STATE_IBOOT_VCO_WA) == rACC_DVFM_ST(kDVFM_STATE_IBOOT)) {
		// VCO workaround DVFM states not configured by LLB / iBSS
		for (i = kDVFM_STATE_IBOOT_VCO_WA; i > kDVFM_STATE_IBOOT_VCO_WA - kDVFM_STATE_VCO_WA_MAX_CNT; i--) {
			enum chipid_voltage_index voltage_index = dvfmperf_get_voltage_index(i, CHIPID_CPU_VOLTAGE);
			config_apsc_acc_state(i, voltage_index, false);
		}
	}
#endif

	for (i = kDVFM_STATE_IBOOT_VCO_WA; i > kDVFM_STATE_IBOOT_VCO_WA - kDVFM_STATE_VCO_WA_MAX_CNT; i--) {
		uint64_t dvfm_st;
		bool found = false;

		dvfm_st = rACC_DVFM_ST(i);

		if (dvfm_st == rACC_DVFM_ST(kDVFM_STATE_IBOOT))
			break;

		dvfm_st &= ~ACC_DVFM_ST_PLL_PMS_MASK;

		for (j = kDVFM_STATE_IBOOT; j <= kDVFM_STATE_IBOOT_VCO_WA - kDVFM_STATE_VCO_WA_MAX_CNT; j++) {
			if ((rACC_DVFM_ST(j) & ~ACC_DVFM_ST_PLL_PMS_MASK) == dvfm_st) {
				found = true;
				break;
			}
		}

		if (!found)
			break;

		vco_hack |= ((i << 4) | j) << index * 8;

		index++;
	}

	propName = "vco-hack";
	if (FindProperty(pmgr_node, &propName, &propData, &propSize)) {
		if (propSize == sizeof(uint32_t))
			((uint32_t *)propData)[0] = vco_hack;
	}
#endif

#if !RELEASE_BUILD && WITH_MENU
        pmgr_binning_menu_update_states(); // must be set before setting reconfig sequence
#endif

	 pmgr_get_dvfm_data(&dvfm);

#if SUPPORT_FPGA
	propName = "l2c-acc-sleep";
	if (FindProperty(pmgr_node, &propName, &propData, &propSize)) {
		if (propSize == sizeof(uint32_t))
			((uint32_t *)propData)[0] = 0;
	}
#endif

	// Populate the devicetree with relevant values.
	propName = "nominal-performance1";
	if (FindProperty(pmgr_node, &propName, &propData, &propSize)) {
		if (propSize != sizeof(uint32_t))
			panic("pmgr property nominal-performance1 is of wrong size.");

		freq = get_freq_from_acc_state(dvfm.dvfm_state_vnom);
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

		freq = get_freq_from_acc_state(kDVFM_STATE_VBOOST);
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
			freq = get_freq_from_acc_state(dvfm.dvfm_state_vmax - j);
			volt = platform_get_dwi_to_mv(BUCK_CPU, ACC_PWRCTL_DVFM_ST0_SAFE_VOL_XTRCT(rACC_DVFM_ST(dvfm.dvfm_state_vmax - j)));

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
	uint32_t	count, propSize, num_states = 0, state_val;
	char		*propName;
	void		*propData;

	propName = "perf-states";
	if (FindProperty(gfx_node, &propName, &propData, &propSize)) {
		if (propSize != (2 * sizeof(uint32_t) * kPMGR_GFX_STATE_MAX)) {
			panic("gfx property perf-state has wrong size");
		}
		// Read the values programmed into the GFX perf state table
		// and populate the device tree.
		for (count = 0; count < kPMGR_GFX_STATE_MAX; count++) {
			state_val = rPMGR_GFX_PERF_STATE_ENTRY_A(count);

			// Any but the first entry with a value of 0 marks the end of the number of valid states.
			if ((count != 0) && (state_val == rPMGR_GFX_PERF_STATE_ENTRY_A(CHIPID_GPU_VOLTAGE_OFF))) {
				num_states = count;
				break;
			}

			((uint32_t *)propData)[count * 2 + 0] = PMGR_PLL_FREQ((state_val >> 12) & 0x1FF, (state_val >> 4) & 0x1F, state_val & 0xF);
			((uint32_t *)propData)[count * 2 + 1] = platform_get_dwi_to_mv(BUCK_GPU, (state_val >> 24) & 0xFF);
		}

		// If all the entries are valid.
		if (count == kPMGR_GFX_STATE_MAX) {
			num_states = kPMGR_GFX_STATE_MAX;
		}

	}

	propName = "perf-state-count";
	if (FindProperty(gfx_node, &propName, &propData, &propSize)) {
		*(uint32_t *)propData = kPMGR_GFX_STATE_MAX;
	}

	propName = "gpu-num-perf-states";
	/* We don't count OFF state */
	if (FindProperty(gfx_node, &propName, &propData, &propSize)) {
		*(uint32_t *)propData = num_states - 1;
	}

	return;
}
#endif

struct register_config {
	uint64_t addr;
	uint64_t value;
	uint32_t is_reg64;
};

#define USECS_TO_TICKS(_usecs)		((_usecs) * (OSC_FREQ / 1000000))

static const struct register_config thermal_registers[] = 
{
	// Define sampling rate for each temperature sensor
	{PMGR_THERMAL0_CTL1, USECS_TO_TICKS(2500), 0},
	{PMGR_THERMAL1_CTL1, USECS_TO_TICKS(2500), 0},
	{PMGR_THERMAL2_CTL1, USECS_TO_TICKS(2500), 0},
#if SUB_PLATFORM_S8001
	{PMGR_THERMAL3_CTL1, USECS_TO_TICKS(2500), 0},
#endif
	{ACC_THRM0_CTL1, USECS_TO_TICKS(2500), 0},
	{ACC_THRM1_CTL1, USECS_TO_TICKS(2500), 0},
	{ACC_THRM2_CTL1, USECS_TO_TICKS(2500), 0},

	// PMGR SOCHOT threshold temperatures have moved from the PMGR SOCHOT register block to the PMGR THERMAL block, ostensibly to provide greater
	// flexibility via the ability to vary failsafe temperature threshold with PMGR location.  The SOCHOT and temperature sensor drivers are separate
	// entities currently, such that it would be inconvienient to coordinate how they come up (can't enable SOCHOT action until non-zero thresholds 
	// have been set, the SOCHOT driver doesn't really need to know how many temperature sensors are present, etc.)  Accordingly, let's set the
	// trip *thresholds* here and leave the drivers to do the rest, as usual.  
	{PMGR_THERMAL0_FAILSAFE_TRIP_TEMP_0, PMGR_THERMAL_0_FAILSAFE_TRIP_TEMP_0_TRIP_TEMP_0_INSRT(120), 0},
	{PMGR_THERMAL1_FAILSAFE_TRIP_TEMP_0, PMGR_THERMAL_1_FAILSAFE_TRIP_TEMP_0_TRIP_TEMP_0_INSRT(120), 0},
	{PMGR_THERMAL2_FAILSAFE_TRIP_TEMP_0, PMGR_THERMAL_2_FAILSAFE_TRIP_TEMP_0_TRIP_TEMP_0_INSRT(120), 0},
#if SUB_PLATFORM_S8001
	{PMGR_THERMAL3_FAILSAFE_TRIP_TEMP_0, PMGR_THERMAL_3_FAILSAFE_TRIP_TEMP_0_TRIP_TEMP_0_INSRT(120), 0},
#endif
	{PMGR_THERMAL0_FAILSAFE_TRIP_TEMP_1, PMGR_THERMAL_0_FAILSAFE_TRIP_TEMP_1_TRIP_TEMP_1_INSRT(125), 0},
	{PMGR_THERMAL1_FAILSAFE_TRIP_TEMP_1, PMGR_THERMAL_1_FAILSAFE_TRIP_TEMP_1_TRIP_TEMP_1_INSRT(125), 0},
	{PMGR_THERMAL2_FAILSAFE_TRIP_TEMP_1, PMGR_THERMAL_2_FAILSAFE_TRIP_TEMP_1_TRIP_TEMP_1_INSRT(125), 0},
#if SUB_PLATFORM_S8001
	{PMGR_THERMAL3_FAILSAFE_TRIP_TEMP_1, PMGR_THERMAL_3_FAILSAFE_TRIP_TEMP_1_TRIP_TEMP_1_INSRT(125), 0},
#endif

	// Set target state for when SOCHOT0 triggers (see also <rdar://problem/20208933> [Fix S8000-family SOCHOT1 config])
	{ACC_DVFM_FSHOT_IDX, ACC_THERMAL_DVFM_FSHOT_IDX_0_ST_INSRT(kDVFM_STATE_V0) | ACC_THERMAL_DVFM_FSHOT_IDX_1_ST_INSRT(kDVFM_STATE_V0), 0},
	{PMGR_BASE_ADDR + PMGR_SOC_PERF_STATE_SOCHOT_OFFSET, 0, 0},
	{PMGR_BASE_ADDR + PMGR_GFX_PERF_STATE_SOCHOT_OFFSET, PMGR_GFX_PERF_STATE_SOCHOT_ENABLE_TRIG0_INSRT(1), 0},

	// Enable temperature sensors (need to do this before enabling either DVTM or SOCHOT)
	{PMGR_THERMAL0_CTL0_SET, PMGR_THERMAL_0_CTL0_SET_ENABLE_INSRT(1), 0},
	{PMGR_THERMAL1_CTL0_SET, PMGR_THERMAL_1_CTL0_SET_ENABLE_INSRT(1), 0},
	{PMGR_THERMAL2_CTL0_SET, PMGR_THERMAL_2_CTL0_SET_ENABLE_INSRT(1), 0},
#if SUB_PLATFORM_S8001
	{PMGR_THERMAL3_CTL0_SET, PMGR_THERMAL_3_CTL0_SET_ENABLE_INSRT(1), 0},
#endif
	{ACC_THRM0_CTL0_SET, ACC_THERMAL_THRM0_CTL0_SET_ENABLE_INSRT(1), 0},
	{ACC_THRM1_CTL0_SET, ACC_THERMAL_THRM1_CTL0_SET_ENABLE_INSRT(1), 0},
	{ACC_THRM2_CTL0_SET, ACC_THERMAL_THRM2_CTL0_SET_ENABLE_INSRT(1), 0},
};

#if (APPLICATION_IBOOT && !PRODUCT_IBOOT && !PRODUCT_IBEC)

static void init_thermal_registers(void)
{
	// Read temperature trim values from efuse and write to corresponding registers
	rPMGR_THERMAL0_PARAM = chipid_get_soc_temp_sensor_trim(0);
	rPMGR_THERMAL1_PARAM = chipid_get_soc_temp_sensor_trim(1);
	rPMGR_THERMAL2_PARAM = chipid_get_soc_temp_sensor_trim(2);
#if SUB_PLATFORM_S8001
	rPMGR_THERMAL3_PARAM = chipid_get_soc_temp_sensor_trim(3);
#endif

	// Apply static config items
	for (size_t i = 0; i < sizeof(thermal_registers) / sizeof(thermal_registers[0]); i++) {
		if (thermal_registers[i].is_reg64) {
			*(volatile uint64_t*)thermal_registers[i].addr = thermal_registers[i].value;
		} else {
			*(volatile uint32_t*)thermal_registers[i].addr = thermal_registers[i].value;
		}
	}
}

#endif

void pmgr_awake_to_aop_ddr_pre_sequence_insert(void)
{
	// Disable temperature sensors
	reconfig_command_write(AWAKE_AOP_DDR_PRE, PMGR_THERMAL0_CTL0_CLR, PMGR_THERMAL_0_CTL0_CLR_ENABLE_INSRT(1), 0);
	reconfig_command_write(AWAKE_AOP_DDR_PRE, PMGR_THERMAL1_CTL0_CLR, PMGR_THERMAL_1_CTL0_CLR_ENABLE_INSRT(1), 0);
	reconfig_command_write(AWAKE_AOP_DDR_PRE, PMGR_THERMAL2_CTL0_CLR, PMGR_THERMAL_2_CTL0_CLR_ENABLE_INSRT(1), 0);
#if SUB_PLATFORM_S8001
	reconfig_command_write(AWAKE_AOP_DDR_PRE, PMGR_THERMAL3_CTL0_CLR, PMGR_THERMAL_3_CTL0_CLR_ENABLE_INSRT(1), 0);
#endif
	reconfig_command_write(AWAKE_AOP_DDR_PRE, ACC_THRM0_CTL0_CLR, ACC_THERMAL_THRM0_CTL0_CLR_ENABLE_INSRT(1), 0);
	reconfig_command_write(AWAKE_AOP_DDR_PRE, ACC_THRM1_CTL0_CLR, ACC_THERMAL_THRM1_CTL0_CLR_ENABLE_INSRT(1), 0);
	reconfig_command_write(AWAKE_AOP_DDR_PRE, ACC_THRM2_CTL0_CLR, ACC_THERMAL_THRM2_CTL0_CLR_ENABLE_INSRT(1), 0);
}

void pmgr_aop_ddr_to_awake_post_sequence_insert_pll(void)
{
	apply_tunables(true);
	pmgr_reconfig_pll();
}

void pmgr_aop_ddr_to_awake_post_sequence_insert(void)
{
	pmgr_reconfig_post();

	// Read PMGR temperature trim values from efuse
	reconfig_command_write(AOP_DDR_AWAKE_POST, PMGR_THERMAL0_PARAM, chipid_get_soc_temp_sensor_trim(0), 0);
	reconfig_command_write(AOP_DDR_AWAKE_POST, PMGR_THERMAL1_PARAM, chipid_get_soc_temp_sensor_trim(1), 0);
	reconfig_command_write(AOP_DDR_AWAKE_POST, PMGR_THERMAL2_PARAM, chipid_get_soc_temp_sensor_trim(2), 0);
#if SUB_PLATFORM_S8001
	reconfig_command_write(AOP_DDR_AWAKE_POST, PMGR_THERMAL3_PARAM, chipid_get_soc_temp_sensor_trim(3), 0);
#endif

	// Reapply temperature sensor tunables and then reenable temperature sensors
	for (size_t i = 0; i < sizeof(thermal_registers) / sizeof(thermal_registers[0]); i++) {
		reconfig_command_write(AOP_DDR_AWAKE_POST, thermal_registers[i].addr, thermal_registers[i].value, thermal_registers[i].is_reg64);		
	}
}

void pmgr_s2r_aop_to_aop_ddr_post_sequence_insert_pwrgate(void)
{
	reconfig_command_write(S2R_AOP_AOP_DDR_POST, (uint64_t)&rPMGR_PWRGATE_MCC_CFG0, rPMGR_PWRGATE_MCC_CFG0, false);
	reconfig_command_write(S2R_AOP_AOP_DDR_POST, (uint64_t)&rPMGR_PWRGATE_MCC_CFG1, rPMGR_PWRGATE_MCC_CFG1, false);
	reconfig_command_write(S2R_AOP_AOP_DDR_POST, (uint64_t)&rPMGR_PWRGATE_MCC_CFG2, rPMGR_PWRGATE_MCC_CFG2, false);
#if SUB_PLATFORM_S8000 || SUB_PLATFORM_S8003
	reconfig_command_write(S2R_AOP_AOP_DDR_POST, (uint64_t)&rPMGR_PWRGATE_DCS01_CFG0, rPMGR_PWRGATE_DCS01_CFG0, false);
	reconfig_command_write(S2R_AOP_AOP_DDR_POST, (uint64_t)&rPMGR_PWRGATE_DCS01_CFG1, rPMGR_PWRGATE_DCS01_CFG1, false);
	reconfig_command_write(S2R_AOP_AOP_DDR_POST, (uint64_t)&rPMGR_PWRGATE_DCS01_CFG2, rPMGR_PWRGATE_DCS01_CFG2, false);
	reconfig_command_write(S2R_AOP_AOP_DDR_POST, (uint64_t)&rPMGR_PWRGATE_DCS23_CFG0, rPMGR_PWRGATE_DCS23_CFG0, false);
	reconfig_command_write(S2R_AOP_AOP_DDR_POST, (uint64_t)&rPMGR_PWRGATE_DCS23_CFG1, rPMGR_PWRGATE_DCS23_CFG1, false);
	reconfig_command_write(S2R_AOP_AOP_DDR_POST, (uint64_t)&rPMGR_PWRGATE_DCS23_CFG2, rPMGR_PWRGATE_DCS23_CFG2, false);
#elif SUB_PLATFORM_S8001
	reconfig_command_write(S2R_AOP_AOP_DDR_POST, (uint64_t)&rPMGR_PWRGATE_DCS0123_CFG0, rPMGR_PWRGATE_DCS0123_CFG0, false);
	reconfig_command_write(S2R_AOP_AOP_DDR_POST, (uint64_t)&rPMGR_PWRGATE_DCS0123_CFG1, rPMGR_PWRGATE_DCS0123_CFG1, false);
	reconfig_command_write(S2R_AOP_AOP_DDR_POST, (uint64_t)&rPMGR_PWRGATE_DCS0123_CFG2, rPMGR_PWRGATE_DCS0123_CFG2, false);
	reconfig_command_write(S2R_AOP_AOP_DDR_POST, (uint64_t)&rPMGR_PWRGATE_DCS4567_CFG0, rPMGR_PWRGATE_DCS4567_CFG0, false);
	reconfig_command_write(S2R_AOP_AOP_DDR_POST, (uint64_t)&rPMGR_PWRGATE_DCS4567_CFG1, rPMGR_PWRGATE_DCS4567_CFG1, false);
	reconfig_command_write(S2R_AOP_AOP_DDR_POST, (uint64_t)&rPMGR_PWRGATE_DCS4567_CFG2, rPMGR_PWRGATE_DCS4567_CFG2, false);
#endif
	reconfig_command_write(S2R_AOP_AOP_DDR_POST, (uint64_t)&rPMGR_PWRGATE_ACS_CFG0, rPMGR_PWRGATE_ACS_CFG0, false);
	reconfig_command_write(S2R_AOP_AOP_DDR_POST, (uint64_t)&rPMGR_PWRGATE_ACS_CFG1, rPMGR_PWRGATE_ACS_CFG1, false);
	reconfig_command_write(S2R_AOP_AOP_DDR_POST, (uint64_t)&rPMGR_PWRGATE_ACS_CFG2, rPMGR_PWRGATE_ACS_CFG2, false);
}
