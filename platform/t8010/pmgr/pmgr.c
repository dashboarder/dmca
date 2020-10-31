/*
 * Copyright (C) 2012-2015 Apple Inc. All rights reserved.
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
#include <platform/soc/operating_point.h>
#include <platform/soc/dvfmperf.h>
#include <sys/boot.h>
#include <target.h>

extern void arm_no_wfe_spin(uint32_t usecs);

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

#define PLL_FREQ_TARGET(pllx)	(((pllx##_O) * (pllx##_M)) / (pllx##_P) / ((pllx##_S) + 1))

#if APPLICATION_SECUREROM
static uint32_t active_state = kDVFM_STATE_SECUREROM;
#endif

#if APPLICATION_IBOOT
static uint32_t active_state = kDVFM_STATE_IBOOT;
#endif

#if APPLICATION_SECUREROM
#define SOC_PERF_STATE_ACTIVE	kSOC_PERF_STATE_SECUREROM
#endif

#if APPLICATION_IBOOT
#define SOC_PERF_STATE_ACTIVE	kSOC_PERF_STATE_VMIN
#endif

struct pmgr_soc_perf_state {
	uint32_t	entry[4];
};

static struct pmgr_soc_perf_state pmgr_soc_perf_states[] = {
	[kSOC_PERF_STATE_BYPASS] = {
		{
			0x00000000,
			0x00000000,
			0x00000000,
			0x00000000
		}
	},

#if APPLICATION_SECUREROM
	[kSOC_PERF_STATE_SECUREROM] = {
		{
			// SBR		= 266.66 / 2
			// SLOW_AF	= 355.56 / 2
			// FAST_AF	= 533.33 / 2
			0x00066600,
			// SIO_C	= 400 / 2
			0x60000000,
			// PMP		= 200 / 2
			0x00000050,
			0x00000000
		}
	},
#endif

#if APPLICATION_IBOOT
	[kSOC_PERF_STATE_VMIN] = {
		// ISP_PEARL 	= 320
		// ISP_AHFD		= 400
		// ISP_C 		= 800
		// SBR 			= 266.66
		// SLOW_AF 		= 355.56
		// FAST_AF 		= 533.33
		// AMCC_CFG_SEL	= 0x0 // !!!FIXME!!! Finalize value
		// MCU_REF_CFG	= 0x3
		// MCU_REF_CLK	= 133
		{
			0x66566637,

			// SIO_C		= 400
			// GFX_FENDER 	= 533.33
			// MSR			= 355.56
			// AJPEG_WRAP	= 320
			// AJPEG_IP		= 266.67
			// VENC			= 533.33
			// VDEC			= 266.66
			// ISP			= 355.56
			0x66666666,

			// AMCC_AUX_BUCKET_SEL 	= 0x0 // !!!FIXME!!! Finalize value
			// BIU div low
			// PMP 					= 200
			// DISP0 				= 355.56
			0x00000056,
			0x00000000
		}
	},

	[kSOC_PERF_STATE_VNOM] = {
		// ISP_PEARL 	= 400
		// ISP_AHFD		= 533.33
		// ISP_C 		= 800
		// SBR 			= 400
		// SLOW_AF 		= 533.33
		// FAST_AF 		= 800
		// AMCC_CFG_SEL	= 0x0 // !!!FIXME!!! Finalize value
		// MCU_REF_CFG	= 0x0
		// MCU_REF_CLK	= 133
		{
			0x55555505,

			// SIO_C		= 533.33
			// GFX_FENDER 	= 800
			// MSR			= 533.33
			// AJPEG_WRAP	= 480
			// AJPEG_IP		= 360
			// VENC			= 720
			// VDEC			= 400
			// ISP			= 480
			0x55555555,

			// AMCC_AUX_BUCKET_SEL 	= 0x0 // !!!FIXME!!! Finalize value
			// BIU div high
			// PMP 					= 200
			// DISP0 				= 480
			0x00000155,
			0x00000000
		}
	},
#endif	// APPLICATION_IBOOT
};

#if APPLICATION_SECUREROM
static uint32_t perf_level = kPerformanceHigh;
#endif

#if APPLICATION_IBOOT
static uint32_t perf_level = kPerformanceMemoryMid;

/* LPPLL @192MHz */
#define LPPLL_T		LPPLL_FREQ

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

// Spare
// /* PLL3 @101MHz */
// #define PLL3		3
// #define PLL3_O		OSC_FREQ
// #define PLL3_P		2
// #define PLL3_M		101
// #define PLL3_S		11
// #define PLL3_T		PLL_FREQ_TARGET(PLL3)

/* PLL4 @1066MHz */
#define PLL4		4
#define PLL4_O		OSC_FREQ
#define PLL4_P		9
#define PLL4_M		400
#define PLL4_S		0
#define PLL4_T		PLL_FREQ_TARGET(PLL4)

/* PLL5 managed by GFX */

/* PLL_PCIE 6 @100MHz */
#define PLL_PCIE	6
#define PLL_PCIE_O	OSC_FREQ
#define PLL_PCIE_P	1
#define PLL_PCIE_M	125
#define PLL_PCIE_S	29
#define PLL_PCIE_T	PLL_FREQ_TARGET(PLL_PCIE)


static const struct clock_config_active clk_configs_active[] = {
	// Mini-PMGR clocks:
	{PMGR_CLK_AOP,			0x81100000}, // 192 (LPO)
	{PMGR_CLK_UART0,		0x84100000}, // 24 (LPO / 8)
	{PMGR_CLK_UART1,		0x84100000}, // 24 (LPO / 8)
	{PMGR_CLK_UART2,		0x84100000}, // 24 (LPO / 8)
	{PMGR_CLK_AOP_MCA0_M,		0x83100000}, //  24 (LPO / 8)
	{PMGR_CLK_I2CM,			0x83100000}, //  24 (LPO / 8)
	{PMGR_CLK_PDM_REF,		0x81100000}, // 192 (LPO)
	{PMGR_CLK_SENSE_2X,		0x83100000}, //  13.714 (LPO / 14)
	{PMGR_CLK_DETECT,		0x81100000}, //  96 (LPO / 2)
	{PMGR_CLK_PROXY_FABRIC,		0x82100000}, // 96 (LPO / 2)
	{PMGR_CLK_PROXY_MCU_REF,	0x81100000}, //  48 (LPO / 4)

	// Spares:
	{PMGR_CLK_S0,			0x00000000}, // 0
	{PMGR_CLK_S1,			0x00000000}, // 0
	{PMGR_CLK_S2,			0x00000000}, // 0
	{PMGR_CLK_S3,			0x00000000}, // 0
	{PMGR_CLK_ISP_REF0,		0x83000004}, // 50 (PLL0 / 8 / 4)
	{PMGR_CLK_ISP_REF1,		0x83000004}, // 50 (PLL0 / 8 / 4)
	{PMGR_CLK_ISP_REF2,		0x83000004}, // 50 (PLL0 / 8 / 4)

	// PMGR clocks:
	{PMGR_CLK_GFX_FENDER,		0x86100000}, // SOC_PERF_STATE
	{PMGR_CLK_MCU_REF,		0x85100000}, // SOC_PERF_STATE
	{PMGR_CLK_PMP,			0x85100000}, // SOC_PERF_STATE
	{PMGR_CLK_TEMP_MIPI_DSI,	0x03100000}, // 800 (PLL2) -- disabled
	{PMGR_CLK_NCO_REF0,		0x85100000}, // 800 (PLL0 / 2)
	{PMGR_CLK_NCO_REF1,		0x85100000}, // 800 (PLL0 / 2)
	{PMGR_CLK_NCO_ALG0,		0x80100000}, //  24 (OSC)
	{PMGR_CLK_NCO_ALG1,		0x85100000}, // 100 (PLL0 / 16)
	{PMGR_CLK_HSICPHY_REF_12M,	0x85100000}, //  12 (OSC / 2)
	{PMGR_CLK_USB480_0,		0x85100000}, // 480 (PLL1 / 3)
	{PMGR_CLK_USB_OHCI_48M,		0x85100000}, //  48 (PLL1 / 30)
	{PMGR_CLK_USB,			0x85100000}, // 120 (PLL1 / 12)
	{PMGR_CLK_USB_FREE_60M,		0x85100000}, //  60 (PLL1 / 24)
	{PMGR_CLK_SIO_C,		0x86100000}, // SOC_PERF_STATE
	{PMGR_CLK_SIO_P,		0x80100000}, //  24 (OSC)
	{PMGR_CLK_SIO_AES,		0x85100000}, // 533 (PLL4 / 2)
	{PMGR_CLK_HFD,			0x85100000}, //  97 (PLL4 / 11)
	{PMGR_CLK_ISP_C,		0x85100000}, // SOC_PERF_STATE
	{PMGR_CLK_ISP_AHFD,		0x86100000}, // SOC_PERF_STATE
	{PMGR_CLK_ISP_PEARL,		0x86100000}, // SOC_PERF_STATE
	{PMGR_CLK_ISP,			0x86100000}, // SOC_PERF_STATE
	{PMGR_CLK_ISP_SENSOR0_REF,	0x81100000}, // ISP_REF0
	{PMGR_CLK_ISP_SENSOR1_REF,	0x82100000}, // ISP_REF1
	{PMGR_CLK_ISP_SENSOR2_REF,	0x83100000}, // ISP_REF2
	{PMGR_CLK_VDEC,			0x86100000}, // SOC_PERF_STATE
	{PMGR_CLK_VENC,			0x86100000}, // SOC_PERF_STATE
	{PMGR_CLK_VID0,			0x85100000}, // 320 (PLL0 / 5)
	{PMGR_CLK_DISP0,		0x86100000}, // SOC_PERF_STATE
	{PMGR_CLK_AJPEG_IP,		0x86100000}, // SOC_PERF_STATE
	{PMGR_CLK_AJPEG_WRAP,		0x86100000}, // SOC_PERF_STATE
	{PMGR_CLK_MSR,			0x86100000}, // SOC_PERF_STATE
	{PMGR_CLK_FAST_AF,		0x88100000}, // SOC_PERF_STATE
	{PMGR_CLK_SLOW_AF,		0x86100000}, // SOC_PERF_STATE
	{PMGR_CLK_SBR,			0x86100000}, // SOC_PERF_STATE
	{PMGR_CLK_MCA0_M,		0x85100000}, // NCO0
	{PMGR_CLK_MCA1_M,		0x86100000}, // NCO1
	{PMGR_CLK_MCA2_M,		0x87100000}, // NCO2
	{PMGR_CLK_MCA3_M,		0x88100000}, // NCO3
	{PMGR_CLK_MCA4_M,		0x89100000}, // NCO4
	{PMGR_CLK_SEP,			0x85100000}, // 800 (PLL0 / 2)
	{PMGR_CLK_GPIO,			0x85100000}, //  50 (PLL0 / 32)
	{PMGR_CLK_SPI0_N,		0x85100000}, // 61.54 (PLL0 / 26)
	{PMGR_CLK_SPI1_N,		0x85100000}, // 61.54 (PLL0 / 26)
	{PMGR_CLK_SPI2_N,		0x85100000}, // 61.54 (PLL0 / 26)
	{PMGR_CLK_SPI3_N,		0x85100000}, // 61.54 (PLL0 / 26)
	{PMGR_CLK_TMPS,			0x85100000}, // 1.2 (OSC / 20)
	{PMGR_CLK_CPU_UVD,		0x85100000}, // 533 (PLL4 / 2)
	{PMGR_CLK_GFX_UVD,		0x85100000}, // 533 (PLL4 / 2)
	{PMGR_CLK_SOC_UVD,		0x85100000}, // 533 (PLL4 / 2)
	{PMGR_CLK_LPDP_RX_REF,		0x85100000}, // 533 (PLL4 / 2)
};

static void set_gfx_perf_state(uint32_t state_num, enum chipid_voltage_index voltage_index);

#endif /* APPLICATION_IBOOT */

#if APPLICATION_SECUREROM

/* PLL0 @800MHz */
#define PLL0		0
#define PLL0_O		OSC_FREQ
#define PLL0_P		3
#define PLL0_M		200
#define PLL0_S		1
#define PLL0_T		PLL_FREQ_TARGET(PLL0)

/* PLL1 @720MHz */
#define PLL1		1
#define PLL1_O		OSC_FREQ
#define PLL1_P		1
#define PLL1_M		60
#define PLL1_S		1
#define PLL1_T		PLL_FREQ_TARGET(PLL1)

/* PLL4 @533.33MHz */
#define PLL4		4
#define PLL4_O		OSC_FREQ
#define PLL4_P		9
#define PLL4_M		400
#define PLL4_S		1
#define PLL4_T		PLL_FREQ_TARGET(PLL4)

/* PLL_PCIE (6) @100MHz */
#define PLL_PCIE	6
#define PLL_PCIE_O	OSC_FREQ
#define PLL_PCIE_P	1
#define PLL_PCIE_M	125
#define PLL_PCIE_S	29
#define PLL_PCIE_T	PLL_FREQ_TARGET(PLL_PCIE)

static const struct clock_config_active clk_configs_active[] = {
	// Mini-PMGR clocks:

	// Spares:

	// PMGR clocks:
	{PMGR_CLK_PMP,			0x85100000}, // SOC_PERF_STATE
	{PMGR_CLK_USB,			0x87100000}, //  50 ((PLL0 / 16) / 2
	{PMGR_CLK_SIO_C,		0x86100000}, // SOC_PERF_STATE
	{PMGR_CLK_SIO_P,		0x88100000}, //  40 (PLL0 / 20) / 2
	{PMGR_CLK_FAST_AF,		0x88100000}, // SOC_PERF_STATE
	{PMGR_CLK_SLOW_AF,		0x86100000}, // SOC_PERF_STATE
	{PMGR_CLK_SBR,			0x86100000}, // SOC_PERF_STATE
	{PMGR_CLK_SEP,			0x85100000}, // 400 ((PLL0 / 2) / 2)
	{PMGR_CLK_GPIO,			0x85100000}, //  25 ((PLL0 / 32) / 2)
	{PMGR_CLK_TMPS,			0x85100000}, // 1.2 (OSC / 20)
};

#endif /* APPLICATION_SECUREROM */

static const struct clock_config clk_configs[PMGR_CLK_COUNT] = {
[PMGR_CLK_GFX_FENDER]	=	{
		&rPMGR_CLK_CFG(GFX_FENDER),
		{
			{ PMGR_CLK_OSC, 1 },
			{ PMGR_CLK_S0, 1 },
			{ PMGR_CLK_S1, 1 },
			{ PMGR_CLK_S2, 1 },
			{ PMGR_CLK_S3, 1 },
			{ PMGR_CLK_PLL0, 2 },
			{ PMGR_CLK_PLL4, 2 },
			{ PMGR_CLK_PLL1, 2 },
			{ PMGR_CLK_PLL0, 4 },
			{ PMGR_CLK_PLL4, 3 },
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
			{ PMGR_CLK_PLL4, 8 },
			{ PMGR_CLK_PLL0, 16 },
			{ PMGR_CLK_PLL0, 32 },
			{ PMGR_CLK_PLL3, 1 },
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
			{ PMGR_CLK_PLL0, 8 },
			{ PMGR_CLK_PLL4, 4 },
			{ PMGR_CLK_PLL0, 10 },
			{ PMGR_CLK_PLL0, 16 },
			{ PMGR_CLK_PROXY_FABRIC, 1 },
		}
	},
	[PMGR_CLK_TEMP_MIPI_DSI]	=	{
		&rPMGR_CLK_CFG(TEMP_MIPI_DSI),
		{
			{ PMGR_CLK_OSC, 1 },
			{ PMGR_CLK_S0, 1 },
			{ PMGR_CLK_S1, 1 },
			{ PMGR_CLK_S2, 1 },
			{ PMGR_CLK_S3, 1 },
			{ PMGR_CLK_PLL0, 2 },
			{ PMGR_CLK_OSC, 1 },
			{ PMGR_CLK_PLL2, 1 },
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
			{ PMGR_CLK_PLL4, 3 },
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
			{ PMGR_CLK_PLL4, 3 },
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
			{ PMGR_CLK_PLL0, 16 },
			{ PMGR_CLK_PLL0, 8 },
			{ PMGR_CLK_PLL0, 10 },
			{ PMGR_CLK_PLL1, 24 },
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
			{ PMGR_CLK_PLL0, 16 },
			{ PMGR_CLK_PLL0, 8 },
			{ PMGR_CLK_PLL0, 10 },
			{ PMGR_CLK_PLL1, 24 },
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
			{ PMGR_CLK_PLL1, 30 },
			{ PMGR_CLK_PLL0, 16 },
			{ PMGR_CLK_PLL1, 24 },
			{ PMGR_CLK_PLL4, 32 },
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
			{ PMGR_CLK_PLL4, 2 },
			{ PMGR_CLK_PLL0, 4 },
			{ PMGR_CLK_PLL1, 3 },
			{ PMGR_CLK_PLL4, 3 },
			{ PMGR_CLK_PLL4, 4 },
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
			{ PMGR_CLK_PLL4, 9 },
			{ PMGR_CLK_PLL0, 16 },
			{ PMGR_CLK_PLL0, 20 },
			{ PMGR_CLK_OSC, 2 },
		}
	},
	[PMGR_CLK_SIO_AES]	=	{
		&rPMGR_CLK_CFG(SIO_AES),
		{
			{ PMGR_CLK_OSC, 1 },
			{ PMGR_CLK_S0, 1 },
			{ PMGR_CLK_S1, 1 },
			{ PMGR_CLK_S2, 1 },
			{ PMGR_CLK_S3, 1 },
			{ PMGR_CLK_PLL4, 2 },
			{ PMGR_CLK_PLL1, 3 },
			{ PMGR_CLK_PLL0, 4 },
		}
	},
	[PMGR_CLK_HFD]	=	{
		&rPMGR_CLK_CFG(HFD),
		{
			{ PMGR_CLK_OSC, 1 },
			{ PMGR_CLK_S0, 1 },
			{ PMGR_CLK_S1, 1 },
			{ PMGR_CLK_S2, 1 },
			{ PMGR_CLK_S3, 1 },
			{ PMGR_CLK_PLL4, 11 },
			{ PMGR_CLK_PLL4, 22 },
			{ PMGR_CLK_PLL0, 16 },
			{ PMGR_CLK_PLL0, 32 },
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
			{ PMGR_CLK_PLL4, 2 },
			{ PMGR_CLK_PLL1, 2 },
			{ PMGR_CLK_PLL0, 4 },
			{ PMGR_CLK_PLL4, 3 },
		}
	},
	[PMGR_CLK_ISP_AHFD]	=	{
		&rPMGR_CLK_CFG(ISP_AHFD),
		{
			{ PMGR_CLK_OSC, 1 },
			{ PMGR_CLK_S0, 1 },
			{ PMGR_CLK_S1, 1 },
			{ PMGR_CLK_S2, 1 },
			{ PMGR_CLK_S3, 1 },
			{ PMGR_CLK_PLL4, 2 },
			{ PMGR_CLK_PLL0, 4 },
			{ PMGR_CLK_PLL1, 3 },
			{ PMGR_CLK_PLL4, 3 },
			{ PMGR_CLK_PLL4, 4 },
		}
	},
	[PMGR_CLK_ISP_PEARL]	=	{
		&rPMGR_CLK_CFG(ISP_PEARL),
		{
			{ PMGR_CLK_OSC, 1 },
			{ PMGR_CLK_S0, 1 },
			{ PMGR_CLK_S1, 1 },
			{ PMGR_CLK_S2, 1 },
			{ PMGR_CLK_S3, 1 },
			{ PMGR_CLK_PLL0, 4 },
			{ PMGR_CLK_PLL0, 5 },
			{ PMGR_CLK_PLL4, 4 },
			{ PMGR_CLK_PLL0, 8 },
			{ PMGR_CLK_PLL0, 10 },
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
			{ PMGR_CLK_PLL1, 3 },
			{ PMGR_CLK_PLL4, 3 },
			{ PMGR_CLK_PLL0, 4 },
			{ PMGR_CLK_PLL0, 5 },
			{ PMGR_CLK_PLL4, 4 },
		}
	},
	[PMGR_CLK_ISP_SENSOR0_REF]	=	{
		&rPMGR_CLK_CFG(ISP_SENSOR0_REF),
		{
			{ PMGR_CLK_OSC, 1 },
			{ PMGR_CLK_ISP_REF0, 1 },
			{ PMGR_CLK_ISP_REF1, 1 },
			{ PMGR_CLK_ISP_REF2, 1 },
		}
	},
	[PMGR_CLK_ISP_SENSOR1_REF]	=	{
		&rPMGR_CLK_CFG(ISP_SENSOR1_REF),
		{
			{ PMGR_CLK_OSC, 1 },
			{ PMGR_CLK_ISP_REF0, 1 },
			{ PMGR_CLK_ISP_REF1, 1 },
			{ PMGR_CLK_ISP_REF2, 1 },
		}
	},
	[PMGR_CLK_ISP_SENSOR2_REF]	=	{
		&rPMGR_CLK_CFG(ISP_SENSOR2_REF),
		{
			{ PMGR_CLK_OSC, 1 },
			{ PMGR_CLK_ISP_REF0, 1 },
			{ PMGR_CLK_ISP_REF1, 1 },
			{ PMGR_CLK_ISP_REF2, 1 },
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
			{ PMGR_CLK_PLL4, 4 },
			{ PMGR_CLK_PLL4, 3 },
			{ PMGR_CLK_PLL4, 5 },
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
			{ PMGR_CLK_PLL1, 2 },
			{ PMGR_CLK_PLL4, 2 },
			{ PMGR_CLK_PLL0, 4 },
			{ PMGR_CLK_PLL4, 3 },
			{ PMGR_CLK_PLL4, 4 },
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
			{ PMGR_CLK_PLL0, 5 },
			{ PMGR_CLK_PLL4, 4 },
			{ PMGR_CLK_PLL0, 7 },
			{ PMGR_CLK_PLL4, 5 },
			{ PMGR_CLK_PLL0, 8 },
			{ PMGR_CLK_PLL0, 16 },
			{ PMGR_CLK_PLL2, 1 },
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
			{ PMGR_CLK_PLL1, 3 },
			{ PMGR_CLK_PLL4, 3 },
			{ PMGR_CLK_PLL0, 4 },
			{ PMGR_CLK_PLL4, 4 },
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
			{ PMGR_CLK_PLL1, 4 },
			{ PMGR_CLK_PLL4, 4 },
			{ PMGR_CLK_PLL0, 5 },
			{ PMGR_CLK_PLL0, 8 },
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
			{ PMGR_CLK_PLL1, 3 },
			{ PMGR_CLK_PLL0, 5 },
			{ PMGR_CLK_PLL0, 4 },
			{ PMGR_CLK_PLL4, 4 },
			{ PMGR_CLK_PLL0, 8 },
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
			{ PMGR_CLK_PLL4, 2 },
			{ PMGR_CLK_PLL4, 3 },
			{ PMGR_CLK_PLL1, 3 },
			{ PMGR_CLK_PLL0, 5 },
			{ PMGR_CLK_PLL4, 4 },
		}
	},
	[PMGR_CLK_FAST_AF]	=	{
		&rPMGR_CLK_CFG(FAST_AF),
		{
			{ PMGR_CLK_OSC, 1 },
			{ PMGR_CLK_S0, 1 },
			{ PMGR_CLK_S1, 1 },
			{ PMGR_CLK_S2, 1 },
			{ PMGR_CLK_S3, 1 },
			{ PMGR_CLK_PLL0, 2 },
			{ PMGR_CLK_PLL4, 2 },
			{ PMGR_CLK_PLL1, 2 },
			{ PMGR_CLK_PLL0, 4 },
			{ PMGR_CLK_PROXY_FABRIC, 1 },
		}
	},
	[PMGR_CLK_SLOW_AF]	=	{
		&rPMGR_CLK_CFG(SLOW_AF),
		{
			{ PMGR_CLK_OSC, 1 },
			{ PMGR_CLK_S0, 1 },
			{ PMGR_CLK_S1, 1 },
			{ PMGR_CLK_S2, 1 },
			{ PMGR_CLK_S3, 1 },
			{ PMGR_CLK_PLL4, 2 },
			{ PMGR_CLK_PLL4, 3 },
			{ PMGR_CLK_PLL1, 3 },
			{ PMGR_CLK_PLL4, 4 },
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
			{ PMGR_CLK_PLL4, 4 },
			{ PMGR_CLK_PLL4, 3 },
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
			{ PMGR_CLK_PLL0, 2 },
			{ PMGR_CLK_PLL4, 2 },
			{ PMGR_CLK_PLL1, 2 },
			{ PMGR_CLK_PLL0, 4 },
			{ PMGR_CLK_PLL4, 3 },
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
			{ PMGR_CLK_PLL4, 32 },
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
			{ PMGR_CLK_PLL0, 32 },
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
	[PMGR_CLK_CPU_UVD]	=	{
		&rPMGR_CLK_CFG(CPU_UVD),
		{
			{ PMGR_CLK_OSC, 1 },
			{ PMGR_CLK_S0, 1 },
			{ PMGR_CLK_S1, 1 },
			{ PMGR_CLK_S2, 1 },
			{ PMGR_CLK_S3, 1 },
			{ PMGR_CLK_PLL4, 2 },
			{ PMGR_CLK_PLL0, 8 },
		}
	},
	[PMGR_CLK_GFX_UVD]	=	{
		&rPMGR_CLK_CFG(GFX_UVD),
		{
			{ PMGR_CLK_OSC, 1 },
			{ PMGR_CLK_S0, 1 },
			{ PMGR_CLK_S1, 1 },
			{ PMGR_CLK_S2, 1 },
			{ PMGR_CLK_S3, 1 },
			{ PMGR_CLK_PLL4, 2 },
			{ PMGR_CLK_PLL0, 8 },
		}
	},
	[PMGR_CLK_SOC_UVD]	=	{
		&rPMGR_CLK_CFG(SOC_UVD),
		{
			{ PMGR_CLK_OSC, 1 },
			{ PMGR_CLK_S0, 1 },
			{ PMGR_CLK_S1, 1 },
			{ PMGR_CLK_S2, 1 },
			{ PMGR_CLK_S3, 1 },
			{ PMGR_CLK_PLL4, 2 },
			{ PMGR_CLK_PLL0, 8 },
		}
	},
	[PMGR_CLK_LPDP_RX_REF]	=	{
		&rPMGR_CLK_CFG(LPDP_RX_REF),
		{
			{ PMGR_CLK_OSC, 1 },
			{ PMGR_CLK_S0, 1 },
			{ PMGR_CLK_S1, 1 },
			{ PMGR_CLK_S2, 1 },
			{ PMGR_CLK_S3, 1 },
			{ PMGR_CLK_PLL4, 2 },
			{ PMGR_CLK_PLL0, 4 },
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
			{ PMGR_CLK_PLL2, 1 },
			{ PMGR_CLK_PLL4, 1 },
		}
	},
	[PMGR_CLK_S3]	=	{
		&rPMGR_CLK_CFG(S3),
		{
			{ PMGR_CLK_PLL0, 2 },
			{ PMGR_CLK_PLL1, 2 },
			{ PMGR_CLK_PLL2, 1 },
			{ PMGR_CLK_PLL4, 1 },
		}
	},
	[PMGR_CLK_ISP_REF0]	=	{
		&rPMGR_CLK_CFG(ISP_REF0),
		{
			{ PMGR_CLK_PLL0, 5 },
			{ PMGR_CLK_PLL4, 4 },
			{ PMGR_CLK_PLL4, 5 },
			{ PMGR_CLK_PLL0, 8 },
		}
	},
	[PMGR_CLK_ISP_REF1]	=	{
		&rPMGR_CLK_CFG(ISP_REF1),
		{
			{ PMGR_CLK_PLL0, 5 },
			{ PMGR_CLK_PLL4, 4 },
			{ PMGR_CLK_PLL4, 5 },
			{ PMGR_CLK_PLL0, 8 },
		}
	},
	[PMGR_CLK_ISP_REF2]	=	{
		&rPMGR_CLK_CFG(ISP_REF2),
		{
			{ PMGR_CLK_PLL0, 5 },
			{ PMGR_CLK_PLL4, 4 },
			{ PMGR_CLK_PLL4, 5 },
			{ PMGR_CLK_PLL0, 8 },
		}
	},
	[PMGR_CLK_AOP]		=	{
		&rMINIPMGR_CLK_CFG(AOP),
		{
			{ PMGR_CLK_OSC, 1 },
			{ PMGR_CLK_LPO, 1 },
			{ PMGR_CLK_LPO, 2 },
			{ PMGR_CLK_LPO, 4 },
			{ PMGR_CLK_LPO, 8 }
		}
	},
	[PMGR_CLK_UART0]	=	{
		&rMINIPMGR_CLK_CFG(UART0),
		{
			{ PMGR_CLK_OSC, 1 },
			{ PMGR_CLK_LPO, 1 },
			{ PMGR_CLK_LPO, 2 },
			{ PMGR_CLK_LPO, 4 },
			{ PMGR_CLK_LPO, 8 }
		}
	},
	[PMGR_CLK_UART1]	=	{
		&rMINIPMGR_CLK_CFG(UART1),
		{
			{ PMGR_CLK_OSC, 1 },
			{ PMGR_CLK_LPO, 1 },
			{ PMGR_CLK_LPO, 2 },
			{ PMGR_CLK_LPO, 4 },
			{ PMGR_CLK_LPO, 8 }
		}
	},
	[PMGR_CLK_UART2]	=	{
		&rMINIPMGR_CLK_CFG(UART2),
		{
			{ PMGR_CLK_OSC, 1 },
			{ PMGR_CLK_LPO, 1 },
			{ PMGR_CLK_LPO, 2 },
			{ PMGR_CLK_LPO, 4 },
			{ PMGR_CLK_LPO, 8 }
		}
	},
	[PMGR_CLK_AOP_MCA0_M]	=	{
		&rMINIPMGR_CLK_CFG(AOP_MCA0_M),
		{
			{ PMGR_CLK_OSC, 1 },
			{ PMGR_CLK_LPO, 2 },
			{ PMGR_CLK_LPO, 4 },
			{ PMGR_CLK_LPO, 8 },
			{ PMGR_CLK_LPO, 1 }
		}
	},
	[PMGR_CLK_I2CM]		=	{
		&rMINIPMGR_CLK_CFG(I2CM),
		{
			{ PMGR_CLK_OSC, 1 },
			{ PMGR_CLK_LPO, 2 },
			{ PMGR_CLK_LPO, 4 },
			{ PMGR_CLK_LPO, 8 },
			{ PMGR_CLK_LPO, 1 }
		}
	},
	[PMGR_CLK_PDM_REF]	=	{
		&rMINIPMGR_CLK_CFG(PDM_REF),
		{
			{ PMGR_CLK_OSC, 1 },
			{ PMGR_CLK_LPO, 1 }
		}
	},
	[PMGR_CLK_SENSE_2X]	=	{
		&rMINIPMGR_CLK_CFG(PDM_REF),
		{
			{ PMGR_CLK_OSC, 1 },
			{ PMGR_CLK_LPO, 4 },
			{ PMGR_CLK_LPO, 8 },
			{ PMGR_CLK_LPO, 14 },
			{ PMGR_CLK_LPO, 28 },
			{ PMGR_CLK_LPO, 40 },
			{ PMGR_CLK_LPO, 56 }
		}
	},
	[PMGR_CLK_DETECT]	=	{
		&rMINIPMGR_CLK_CFG(I2CM),
		{
			{ PMGR_CLK_OSC, 1 },
			{ PMGR_CLK_LPO, 2 },
			{ PMGR_CLK_LPO, 4 },
			{ PMGR_CLK_LPO, 8 },
			{ PMGR_CLK_LPO, 1 }
		}
	},
	[PMGR_CLK_PROXY_FABRIC]	=	{
		&rMINIPMGR_CLK_CFG(PROXY_FABRIC),
		{
			{ PMGR_CLK_OSC, 1 },
			{ PMGR_CLK_LPO, 1 },
			{ PMGR_CLK_LPO, 2 },
			{ PMGR_CLK_LPO, 4 },
			{ PMGR_CLK_LPO, 8 }
		}
	},
	[PMGR_CLK_PROXY_MCU_REF] =	{
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

static const struct device_config device_configs[PMGR_DEVICE_COUNT]	= {
	// Mini PMGR
	[PMGR_DEVICE_INDEX(CLK_AOP)]				= {&rMINIPMGR_PS(AOP)},
	[PMGR_DEVICE_INDEX(CLK_DEBUG)]				= {&rMINIPMGR_PS(DEBUG)},
	[PMGR_DEVICE_INDEX(CLK_AOP_GPIO)]			= {&rMINIPMGR_PS(AOP_GPIO)},
	[PMGR_DEVICE_INDEX(CLK_AOP_UART0)]			= {&rMINIPMGR_PS(AOP_UART0)},
	[PMGR_DEVICE_INDEX(CLK_AOP_UART1)]			= {&rMINIPMGR_PS(AOP_UART1)},
	[PMGR_DEVICE_INDEX(CLK_AOP_UART2)]			= {&rMINIPMGR_PS(AOP_UART2)},
	[PMGR_DEVICE_INDEX(CLK_AOP_I2CM)]			= {&rMINIPMGR_PS(AOP_I2CM)},
	[PMGR_DEVICE_INDEX(CLK_AOP_MCA0)]			= {&rMINIPMGR_PS(AOP_MCA0)},
	[PMGR_DEVICE_INDEX(CLK_AOP_PDM_REF)]			= {&rMINIPMGR_PS(AOP_PDM_REF)},
	[PMGR_DEVICE_INDEX(CLK_AOP_CPU)]			= {&rMINIPMGR_PS(AOP_CPU)},
	[PMGR_DEVICE_INDEX(CLK_AOP_FILTER)]			= {&rMINIPMGR_PS(AOP_FILTER)},
	[PMGR_DEVICE_INDEX(CLK_AOP_BUSIF)]			= {&rMINIPMGR_PS(AOP_BUSIF)},
	[PMGR_DEVICE_INDEX(CLK_AOP_LPD0)]			= {&rMINIPMGR_PS(AOP_LPD0)},
	[PMGR_DEVICE_INDEX(CLK_AOP_LPD1)]			= {&rMINIPMGR_PS(AOP_LPD1)},
	[PMGR_DEVICE_INDEX(CLK_AOP_HPDS)]			= {&rMINIPMGR_PS(AOP_HPDS)},
	[PMGR_DEVICE_INDEX(CLK_AOP_HPDSC)]			= {&rMINIPMGR_PS(AOP_HPDSC)},
	[PMGR_DEVICE_INDEX(CLK_AOP_HPDD)]			= {&rMINIPMGR_PS(AOP_HPDD)},
	// PMGR
	[PMGR_DEVICE_INDEX(CLK_SBR)]				= {&rPMGR_PS(SBR)},
	[PMGR_DEVICE_INDEX(CLK_AIC)]				= {&rPMGR_PS(AIC)},
	[PMGR_DEVICE_INDEX(CLK_DWI)]				= {&rPMGR_PS(DWI)},
	[PMGR_DEVICE_INDEX(CLK_GPIO)]				= {&rPMGR_PS(GPIO)},
	[PMGR_DEVICE_INDEX(CLK_PMS)]				= {&rPMGR_PS(PMS)},
	[PMGR_DEVICE_INDEX(CLK_HSIC0PHY)]			= {&rPMGR_PS(HSIC0PHY)},
	[PMGR_DEVICE_INDEX(CLK_ISPSENS0)]			= {&rPMGR_PS(ISPSENS0)},
	[PMGR_DEVICE_INDEX(CLK_ISPSENS1)]			= {&rPMGR_PS(ISPSENS1)},
	[PMGR_DEVICE_INDEX(CLK_ISPSENS2)]			= {&rPMGR_PS(ISPSENS2)},
	[PMGR_DEVICE_INDEX(CLK_PCIE_REF)]			= {&rPMGR_PS(PCIE_REF)},
	[PMGR_DEVICE_INDEX(CLK_SOCUVD)]				= {&rPMGR_PS(SOCUVD)},
	[PMGR_DEVICE_INDEX(CLK_SIO_BUSIF)]			= {&rPMGR_PS(SIO_BUSIF)},
	[PMGR_DEVICE_INDEX(CLK_SIO_P)]				= {&rPMGR_PS(SIO_P)},
	[PMGR_DEVICE_INDEX(CLK_SIO)]				= {&rPMGR_PS(SIO)},
	[PMGR_DEVICE_INDEX(CLK_MCA0)]				= {&rPMGR_PS(MCA0)},
	[PMGR_DEVICE_INDEX(CLK_MCA1)]				= {&rPMGR_PS(MCA1)},
	[PMGR_DEVICE_INDEX(CLK_MCA2)]				= {&rPMGR_PS(MCA2)},
	[PMGR_DEVICE_INDEX(CLK_MCA3)]				= {&rPMGR_PS(MCA3)},
	[PMGR_DEVICE_INDEX(CLK_MCA4)]				= {&rPMGR_PS(MCA4)},
	[PMGR_DEVICE_INDEX(CLK_PWM0)]				= {&rPMGR_PS(PWM0)},
	[PMGR_DEVICE_INDEX(CLK_I2C0)]				= {&rPMGR_PS(I2C0)},
	[PMGR_DEVICE_INDEX(CLK_I2C1)]				= {&rPMGR_PS(I2C1)},
	[PMGR_DEVICE_INDEX(CLK_I2C2)]				= {&rPMGR_PS(I2C2)},
	[PMGR_DEVICE_INDEX(CLK_I2C3)]				= {&rPMGR_PS(I2C3)},
	[PMGR_DEVICE_INDEX(CLK_SPI0)]				= {&rPMGR_PS(SPI0)},
	[PMGR_DEVICE_INDEX(CLK_SPI1)]				= {&rPMGR_PS(SPI1)},
	[PMGR_DEVICE_INDEX(CLK_SPI2)]				= {&rPMGR_PS(SPI2)},
	[PMGR_DEVICE_INDEX(CLK_SPI3)]				= {&rPMGR_PS(SPI3)},
	[PMGR_DEVICE_INDEX(CLK_UART0)]				= {&rPMGR_PS(UART0)},
	[PMGR_DEVICE_INDEX(CLK_UART1)]				= {&rPMGR_PS(UART1)},
	[PMGR_DEVICE_INDEX(CLK_UART2)]				= {&rPMGR_PS(UART2)},
	[PMGR_DEVICE_INDEX(CLK_UART3)]				= {&rPMGR_PS(UART3)},
	[PMGR_DEVICE_INDEX(CLK_UART4)]				= {&rPMGR_PS(UART4)},
	[PMGR_DEVICE_INDEX(CLK_UART5)]				= {&rPMGR_PS(UART5)},
	[PMGR_DEVICE_INDEX(CLK_UART6)]				= {&rPMGR_PS(UART6)},
	[PMGR_DEVICE_INDEX(CLK_UART7)]				= {&rPMGR_PS(UART7)},
	[PMGR_DEVICE_INDEX(CLK_UART8)]				= {&rPMGR_PS(UART8)},
	[PMGR_DEVICE_INDEX(CLK_AES0)]				= {&rPMGR_PS(AES0)},
	[PMGR_DEVICE_INDEX(CLK_HFD0)]				= {&rPMGR_PS(HFD0)},
	[PMGR_DEVICE_INDEX(CLK_MCC)]				= {&rPMGR_PS(MCC)},
	[PMGR_DEVICE_INDEX(CLK_DCS0)]				= {&rPMGR_PS(DCS0)},
	[PMGR_DEVICE_INDEX(CLK_DCS1)]				= {&rPMGR_PS(DCS1)},
	[PMGR_DEVICE_INDEX(CLK_DCS2)]				= {&rPMGR_PS(DCS2)},
	[PMGR_DEVICE_INDEX(CLK_DCS3)]				= {&rPMGR_PS(DCS3)},
	[PMGR_DEVICE_INDEX(CLK_USB)]				= {&rPMGR_PS(USB)},
	[PMGR_DEVICE_INDEX(CLK_USBCTLREG)]			= {&rPMGR_PS(USBCTLREG)},
	[PMGR_DEVICE_INDEX(CLK_USB2HOST0)]			= {&rPMGR_PS(USB2HOST0)},
	[PMGR_DEVICE_INDEX(CLK_USB2HOST0_OHCI)]			= {&rPMGR_PS(USB2HOST0_OHCI)},
	[PMGR_DEVICE_INDEX(CLK_USB2HOST1)]			= {&rPMGR_PS(USB2HOST1)},
	[PMGR_DEVICE_INDEX(CLK_USB_OTG)]			= {&rPMGR_PS(USB_OTG)},
	[PMGR_DEVICE_INDEX(CLK_SMX)]				= {&rPMGR_PS(SMX)},
	[PMGR_DEVICE_INDEX(CLK_SF)]				= {&rPMGR_PS(SF)},
	[PMGR_DEVICE_INDEX(CLK_RTMUX)]				= {&rPMGR_PS(RTMUX)},
	[PMGR_DEVICE_INDEX(CLK_DISP0_FE)]			= {&rPMGR_PS(DISP0_FE)},
	[PMGR_DEVICE_INDEX(CLK_DISP0_BE)]			= {&rPMGR_PS(DISP0_BE)},
	[PMGR_DEVICE_INDEX(CLK_MIPI_DSI)]			= {&rPMGR_PS(MIPI_DSI)},
	[PMGR_DEVICE_INDEX(CLK_DP)]				= {&rPMGR_PS(DP)},
	[PMGR_DEVICE_INDEX(CLK_ISP_SYS)]			= {&rPMGR_PS(ISP_SYS)},
	[PMGR_DEVICE_INDEX(CLK_MEDIA)]				= {&rPMGR_PS(MEDIA)},
	[PMGR_DEVICE_INDEX(CLK_JPG)]				= {&rPMGR_PS(JPG)},
	[PMGR_DEVICE_INDEX(CLK_MSR)]				= {&rPMGR_PS(MSR)},
	[PMGR_DEVICE_INDEX(CLK_PMP)]				= {&rPMGR_PS(PMP)},
	[PMGR_DEVICE_INDEX(CLK_PMS_SRAM)]			= {&rPMGR_PS(PMS_SRAM)},
	[PMGR_DEVICE_INDEX(CLK_VDEC0)]				= {&rPMGR_PS(VDEC0)},
	[PMGR_DEVICE_INDEX(CLK_VENC)]				= {&rPMGR_PS(VENC_SYS)},	// NOTE: Name discrepancy between _CLK_CFG and _PS register
	[PMGR_DEVICE_INDEX(CLK_PCIE)]				= {&rPMGR_PS(PCIE)},
	[PMGR_DEVICE_INDEX(CLK_PCIE_AUX)]			= {&rPMGR_PS(PCIE_AUX)},
	[PMGR_DEVICE_INDEX(CLK_GFX)]				= {&rPMGR_PS(GFX)},
	// Unmanaged
	[PMGR_DEVICE_INDEX(CLK_CPU0)]				= {&rPMGR_PS(CPU0)},
	[PMGR_DEVICE_INDEX(CLK_CPU1)]				= {&rPMGR_PS(CPU1)},
	[PMGR_DEVICE_INDEX(CLK_CPM)]				= {&rPMGR_PS(CPM)},
	[PMGR_DEVICE_INDEX(CLK_SEP)]				= {&rPMGR_PS(SEP)},
	[PMGR_DEVICE_INDEX(CLK_ISP_RSTS0)]			= {&rPMGR_PS(ISP_RSTS0)},
	[PMGR_DEVICE_INDEX(CLK_ISP_RSTS1)]			= {&rPMGR_PS(ISP_RSTS1)},
	[PMGR_DEVICE_INDEX(CLK_ISP_VIS)]			= {&rPMGR_PS(ISP_VIS)},
	[PMGR_DEVICE_INDEX(CLK_ISP_BE)]				= {&rPMGR_PS(ISP_BE)},
	[PMGR_DEVICE_INDEX(CLK_ISP_PEARL)]			= {&rPMGR_PS(ISP_PEARL)},
	[PMGR_DEVICE_INDEX(CLK_DPRX)]				= {&rPMGR_PS(DPRX)},
	[PMGR_DEVICE_INDEX(CLK_VENC_PIPE4)]			= {&rPMGR_PS(VENC_PIPE5)},
	[PMGR_DEVICE_INDEX(CLK_VENC_PIPE5)]			= {&rPMGR_PS(VENC_PIPE5)},
	[PMGR_DEVICE_INDEX(CLK_VENC_ME0)]			= {&rPMGR_PS(VENC_ME0)},
	[PMGR_DEVICE_INDEX(CLK_VENC_ME1)]			= {&rPMGR_PS(VENC_ME1)},
};

static void config_apsc_acc_state(uint32_t state, enum chipid_voltage_index voltage_index);
static uint32_t get_apsc_acc_state(void);
static void set_apsc_acc_state(uint32_t target_state);
static void config_soc_perf_state(uint32_t state, uint32_t safevol);
static void set_soc_perf_state(uint32_t target_state);
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
static void set_lppll(void);
static void set_pll(int32_t pll, uint32_t p, uint32_t m, uint32_t s);
static uint32_t get_spare(int32_t spare);
static void clocks_set_gates(uint64_t *devices, uint64_t *power);
static void clocks_quiesce_internal(void);
static void power_on_sep(void);

// current clock frequencies
static uint32_t clks[PMGR_CLK_COUNT];

void platform_power_spin(uint32_t usecs)
{
	arm_no_wfe_spin(usecs);
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

static void apply_pmgr_tunables()
{
	rPMGR_VOLMAN_SOC_DELAY = (0x5 << 0) | (0x1e << 10) | (0xbb8 << 20);

	return;
}
#endif

#if WITH_DEVICETREE
static uint64_t get_freq_from_acc_state(uint32_t state_index);

static void pmgr_get_dvfm_data_from_vmax(struct dvfm_data *dvfm, uint32_t dvfm_state_vmax)
{
	dvfm->dvfm_state_vmax = dvfm_state_vmax;
	dvfm->voltage_states1_count = dvfm_state_vmax - kDVFM_STATE_V0 + 1;
	dvfm->voltage_states1_size = dvfm->voltage_states1_count * 2;
	dvfm->dvfm_state_iboot_cnt = dvfm_state_vmax + 1;
	dvfm->dvfm_state_vnom = kDVFM_STATE_VNOM;
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
	uint32_t	soc_vid[kSOC_PERF_STATE_MAX_CNT];
#endif
	volatile uint32_t *clkcfg;

	clks[PMGR_CLK_OSC] = OSC_FREQ;

	// Setup bypass DVFM state
	config_apsc_acc_state(kDVFM_STATE_BYPASS, CHIPID_CPU_VOLTAGE_BYPASS);

	// Change all clocks to something safe
	clocks_quiesce_internal();

	// Setup active DVFM and SOC PERF states for the stage of boot.
#if APPLICATION_SECUREROM
	config_apsc_acc_state(kDVFM_STATE_SECUREROM, CHIPID_CPU_VOLTAGE_SECUREROM);
	config_soc_perf_state(kSOC_PERF_STATE_SECUREROM, 0);
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
	config_apsc_acc_state(kDVFM_STATE_BYPASS, CHIPID_CPU_VOLTAGE_BYPASS);

	for (cnt = kDVFM_STATE_IBOOT; cnt < kDVFM_STATE_MAX_CNT; cnt++) {
		enum chipid_voltage_index voltage_index = dvfmperf_get_voltage_index(cnt, CHIPID_CPU_VOLTAGE);
		config_apsc_acc_state(cnt, voltage_index);
	}

	platform_get_soc_voltages(kSOC_PERF_STATE_IBOOT_CNT, soc_vid);
	platform_convert_voltages(BUCK_SOC, kSOC_PERF_STATE_IBOOT_CNT, soc_vid);

	for (cnt = kSOC_PERF_STATE_IBOOT; cnt < kSOC_PERF_STATE_IBOOT_CNT; cnt++) {
		config_soc_perf_state(cnt, soc_vid[cnt]);
	}

	// Populate unused states with something safe
	for (cnt = kSOC_PERF_STATE_IBOOT_CNT; cnt < kSOC_PERF_STATE_MAX_CNT; cnt++) {
		config_soc_perf_state(cnt, soc_vid[kSOC_PERF_STATE_VMIN]);
	}

	// TODO: Finalize DVFM/DVTM settings. This is a placeholder from Maui.
	// Configure temperature ranges and measurement offsets for DVFM/DVTM
	// <rdar://problem/20044269> Cayman FPGA: iBSS encounters abort when accessing DWI_PS register
#if 0
	rACC_DVFM_CFG = ACC_PWRCTL_DVFM_CFG_TEMP_THRES0_INSRT(0x8) | ACC_PWRCTL_DVFM_CFG_TEMP_THRES1_INSRT(0x21) |
			ACC_PWRCTL_DVFM_CFG_TEMP_OFFST0_INSRT(0) | ACC_PWRCTL_DVFM_CFG_TEMP_OFFST1_INSRT(0);
	rACC_DVFM_CFG1 = ACC_PWRCTL_DVFM_CFG1_TEMP_THRES2_INSRT(0x3A) | ACC_PWRCTL_DVFM_CFG1_TEMP_THRES2_INSRT(0);
#endif

#if WITH_HW_DWI
	extern int dwi_init(void);
	dwi_init();
#endif
#endif	// APPLICATION_IBOOT

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

#ifdef LPPLL_T
	set_lppll();
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
	set_pll(PLL3, PLL3_P, PLL3_M, PLL3_S);
#endif

#ifdef PLL4_T
	set_pll(PLL4, PLL4_P, PLL4_M, PLL4_S);
#endif

#ifdef PLL5_T
	set_pll(PLL5, PLL5_P, PLL5_M, PLL5_S);
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

		*clkcfg = clk_configs_active[cnt].clock_reg_val;
		while ((*clkcfg & PMGR_CLK_CFG_PENDING) != 0);
	}

	power_on_sep();

	clocks_get_frequencies();

#if (APPLICATION_IBOOT && !PRODUCT_IBOOT && !PRODUCT_IBEC)
	apply_pmgr_tunables();
#endif

	return 0;
}

static void config_apsc_acc_state(uint32_t state, enum chipid_voltage_index voltage_index)
{
	uint64_t value = 0;
	uint64_t value_ext = 0;
	uint32_t dwi_val = 0;
	uint32_t dwi_sram_val = 0;
	uint32_t fused_core_typ = 0;

#if APPLICATION_IBOOT
	dwi_val = chipid_get_cpu_voltage(voltage_index);
	platform_convert_voltages(BUCK_CPU, 1, &dwi_val);
	dwi_sram_val = chipid_get_cpu_sram_voltage(voltage_index);
	platform_convert_voltages(BUCK_CPU_RAM, 1, &dwi_sram_val);
#endif

	const struct operating_point_params *params = operating_point_get_params(voltage_index, CHIPID_CPU_VOLTAGE);
	if (params == NULL) {
		return;
	}

	value |= ACC_DVFM_ST_FCW_FRAC_INSRT(params->cpu.fcwFrac);
	value |= ACC_DVFM_ST_FCW_INT_INSRT(params->cpu.fcwInt);
	value |= ACC_DVFM_ST_PST_DIV_S_INSRT(params->cpu.pstDivS);
	value |= ACC_DVFM_ST_MIG_DIV_S(params->cpu.migDivS);
	value |= ACC_DVFM_ST_CLK_SRC(params->cpu.clkSrc);
	value |= ACC_DVFM_ST_VOL_BYP(params->cpu.bypass);
	value |= ACC_DVFM_ST_SAFE_VOL_INSRT(dwi_val);
	value_ext |= ACC_DVFM_ST_BIU_DIV_HI_VOL(params->cpu.biuDiv4HiVol);
	value_ext |= ACC_DVFM_ST_BIU_DIV_LO_VOL(params->cpu.biuDiv4LoVol);
	value_ext |= ACC_DVFM_ST_DVMR_MAX_WGT(params->cpu.dvmrMaxWgt);
	value_ext |= ACC_DVFM_ST_SRAM_VOL(dwi_sram_val);

	// Get the core we're booting on (0 == eCore, 1 == pCore)
	fused_core_typ = MINIPMGR_MINI_MISC_CFG_CPU_CFG_BOOT_PCORE_XTRCT(rMINIPMGR_MISC_CFG_CPU);

	if (params->cpu.coreTyp == kACC_CORETYP_FUSED)
		value |= ACC_DVFM_ST_CORE_TYP(fused_core_typ);
	else
		value |= ACC_DVFM_ST_CORE_TYP(params->cpu.coreTyp);

#if APPLICATION_IBOOT
	const struct chipid_vol_adj *adj = chipid_get_vol_adj(voltage_index);
	if (adj == NULL) {
		return;
	}

	value |= ACC_DVFM_ST_VOL_ADJ0(pmgr_get_offset_from_diff_uV(BUCK_CPU, adj->volAdj0));
	value |= ACC_DVFM_ST_VOL_ADJ1(pmgr_get_offset_from_diff_uV(BUCK_CPU, adj->volAdj1));
	value |= ACC_DVFM_ST_VOL_ADJ2(pmgr_get_offset_from_diff_uV(BUCK_CPU, adj->volAdj2));
	value |= ACC_DVFM_ST_VOL_ADJ3(pmgr_get_offset_from_diff_uV(BUCK_CPU, adj->volAdj3));

	value_ext |= ACC_DVFM_ST_DVFM_MAX_ADJ(pmgr_get_offset_from_diff_uV(BUCK_CPU, adj->dvfmMaxAdj));
	value_ext |= ACC_DVFM_ST_DVMR_ADJ0(pmgr_get_offset_from_diff_uV(BUCK_CPU, adj->dvmrAdj0));
	value_ext |= ACC_DVFM_ST_DVMR_ADJ1(pmgr_get_offset_from_diff_uV(BUCK_CPU, adj->dvmrAdj1));
	value_ext |= ACC_DVFM_ST_DVMR_ADJ2(pmgr_get_offset_from_diff_uV(BUCK_CPU, adj->dvmrAdj2));
#endif

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
	rACC_APSC_SCR = ACC_APSC_MANUAL_CHANGE(target_state);
	while ((rACC_APSC_SCR & ACC_APSC_PENDING) != 0);

	return;
}

static void config_soc_perf_state(uint32_t state, uint32_t safevol)
{
	uint32_t index = state;

#if APPLICATION_IBOOT
	if (index >= kSOC_PERF_STATE_IBOOT_CNT)
		index = kSOC_PERF_STATE_VMIN;
#endif

	rPMGR_SOC_PERF_STATE_ENTRY_A(PMGR_SOC_PERF_STATE_TO_ENTRY(state)) = pmgr_soc_perf_states[index].entry[0];
	rPMGR_SOC_PERF_STATE_ENTRY_B(PMGR_SOC_PERF_STATE_TO_ENTRY(state)) = pmgr_soc_perf_states[index].entry[1];
	rPMGR_SOC_PERF_STATE_ENTRY_C(PMGR_SOC_PERF_STATE_TO_ENTRY(state)) = pmgr_soc_perf_states[index].entry[2] | PMGR_SOC_PERF_STATE_ENTRY_VOLTAGE(safevol);
	rPMGR_SOC_PERF_STATE_ENTRY_D(PMGR_SOC_PERF_STATE_TO_ENTRY(state)) = pmgr_soc_perf_states[index].entry[3];
}

static void set_soc_perf_state(uint32_t target_state)
{
	rPMGR_SOC_PERF_STATE_CTL = PMGR_SOC_PERF_STATE_TO_ENTRY(target_state);
	while (rPMGR_SOC_PERF_STATE_CTL & PMGR_SOC_PERF_STATE_CTL_PENDING_MASK);
}

static void set_lppll(void)
{
	rMINIPMGR_LPPLL_CTL = MINIPMGR_LPPLL_CTL_ENABLE;

	while (rMINIPMGR_LPPLL_CTL & MINIPMGR_LPPLL_CTL_PENDING);
}

/*
 * set_pll - called by SecureROM, LLB, iBSS with PLLs in default reset state.
 * See restore_clock_config_state().
 */
static void set_pll(int32_t pll, uint32_t p, uint32_t m, uint32_t s)
{
	if (pll >= PMGR_PLL_COUNT)
		panic("Invalid PLL %u", pll);

	if (pll == PLL_PCIE) {
		uint32_t ana_params;
		uint32_t cfg_params;

		// Set PCIE_PLL Lock Mode Tunable.
		cfg_params = rPMGR_PLL_CFG(pll);
		cfg_params &= ~PMGR_PLL_PCIE_CFG_LOCK_MODE_UMASK;

#if APPLICATION_IBOOT
		cfg_params |= PMGR_PLL_LOCK_MODE(PMGR_PLL_LOCK_MODE_LOCK);
#endif
		rPMGR_PLL_CFG(pll) = cfg_params;

		// The PCIe PLL needs data loaded from the fuses (<rdar://problem/17602718>)
		ana_params = rPMGR_PLL_ANA_PARAMS3(pll);
		ana_params &= ~PMGR_PLL_PCIE_ANA_PARAMS3_FCAL_VCODIGCTRL_UMASK;
		ana_params |= PMGR_PLL_PCIE_ANA_PARAMS3_FCAL_VCODIGCTRL_INSRT(chipid_get_pcie_refpll_fcal_vco_digctrl()) |
				PMGR_PLL_PCIE_ANA_PARAMS3_FCAL_BYPASS_UMASK;
		rPMGR_PLL_ANA_PARAMS3(pll) = ana_params;

		rPMGR_PLL_CTL(pll) = PMGR_PLL_ENABLE | PMGR_PLL_LOAD | PMGR_PLL_M(m) | PMGR_PLL_PCIE_S(s);
	} else {
		rPMGR_PLL_CTL(pll) = PMGR_PLL_ENABLE | PMGR_PLL_LOAD | PMGR_PLL_P(p) | PMGR_PLL_M(m) | PMGR_PLL_S(s);
	}

	while (rPMGR_PLL_CTL(pll) & PMGR_PLL_PENDING);
}

static uint32_t get_pll_cpu(void)
{
	uint32_t freq;
	uint32_t pllctl;

	pllctl = rACC_PLL_SCR1;

	// Fcpu <= (OSC * (PLL_FCW_INT + PLL_FCW_FRAC / 5)) / (2 * (S + 1))
	freq = OSC_FREQ;
	freq *= (ACC_PWRCTL_PLL_SCR1_FCW_INT_XTRCT(pllctl) + (ACC_PWRCTL_PLL_SCR1_FCW_FRAC_XTRCT(pllctl) / 5.0));
	freq /= (2 * (1 + ACC_PWRCTL_PLL_SCR1_OP_DIVN_XTRCT(pllctl)));

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

	if (pll == PLL_PCIE) {
		opdiv = ((pllctl >> PMGR_PLL_S_SHIFT) & PMGR_PLL_PCIE_S_MASK);
		if (opdiv >= 3 && opdiv <= 7) {
			freq /= 16;
		} else if (opdiv > 7) {
			freq /= 2 * opdiv;
		}
	} else {
		opdiv = ((pllctl >> PMGR_PLL_S_SHIFT) & PMGR_PLL_S_MASK);
		freq /= opdiv + 1;
		freq /= ((pllctl >> PMGR_PLL_P_SHIFT) & PMGR_PLL_P_MASK);
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
	clks[PMGR_CLK_LPO] = LPPLL_FREQ;

#elif SUB_TARGET_T8010SIM
	uint32_t cnt;
	uint32_t freq = OSC_FREQ;

	for (cnt = 0; cnt < PMGR_CLK_COUNT; cnt++)
		clks[cnt] = freq;

#else
	uint32_t cnt;

	clks[PMGR_CLK_OSC] = OSC_FREQ;
	clks[PMGR_CLK_LPO] = LPPLL_FREQ;

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
		*reg = clkdata;
		while (*reg & PMGR_CLK_CFG_PENDING);
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
		if (cnt == 0)
			continue; // Mem PLL

		if (cnt == PLL_PCIE) {
			rPMGR_PLL_CTL(cnt) = PMGR_PLL_M(0x7d) | PMGR_PLL_PCIE_S(0x1d); // fb_div = 0x7d, op_div = 0x1d
		}
		else {
			rPMGR_PLL_CTL(cnt) = PMGR_PLL_ENABLE | PMGR_PLL_BYPASS | PMGR_PLL_M(1) | PMGR_PLL_P(1); // fb_div = 1, pre_div = 1
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

	// 4. Write the reset value to LPPLL_CTL register
	rMINIPMGR_LPPLL_CTL = PMGR_PLL_ENABLE | PMGR_PLL_BYPASS;

	// 5. Write the reset value to LPPLL_CFG register
	rMINIPMGR_LPPLL_CFG = 0x1f;
	while (rMINIPMGR_LPPLL_CTL & PMGR_PLL_PENDING);

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
	uint64_t power[2] = { 0, 0 };

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
	set_device(devices, CLK_SIO_BUSIF);
	set_device(devices, CLK_SIO_P);
	set_device(devices, CLK_SIO);
	set_device(devices, CLK_MCC);
	set_device(devices, CLK_DCS0);
	set_device(devices, CLK_DCS1);
	set_device(devices, CLK_DCS2);
	set_device(devices, CLK_DCS3);
	set_device(devices, CLK_USB);
	set_device(devices, CLK_USBCTLREG);
	set_device(devices, CLK_USB_OTG);
	set_device(devices, CLK_SMX);
	set_device(devices, CLK_SF);

#if APPLICATION_SECUREROM
	// These devices need power but their clocks don't need to be on.
	set_device(power, CLK_PCIE);
	set_device(power, CLK_PCIE_REF);
	set_device(power, CLK_PCIE_AUX);
#endif

	// Turn on/off critical device clocks
	clocks_set_gates(devices, power);

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

	switch (performance_level) {
		case kPerformanceMemoryLow:
		case kPerformanceMemoryMid:
			entry_a = rPMGR_SOC_PERF_STATE_ENTRY_A(PMGR_SOC_PERF_STATE_TO_ENTRY(kSOC_PERF_STATE_VMIN));
			entry_a &= ~PMGR_SOC_PERF_STATE_ENTRY_MCU_REF_MASK;

			if (performance_level == kPerformanceMemoryLow)
				entry_a |= PMGR_SOC_PERF_STATE_ENTRY_MCU_REF(0x3, 0x7);
			else
				entry_a |= PMGR_SOC_PERF_STATE_ENTRY_MCU_REF(0x1, 0x6);

			rPMGR_SOC_PERF_STATE_ENTRY_A(PMGR_SOC_PERF_STATE_TO_ENTRY(kSOC_PERF_STATE_VMIN)) = entry_a;

			set_soc_perf_state(kSOC_PERF_STATE_VMIN);
			perf_level = performance_level;
			break;

		case kPerformanceMemoryFull:
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
#elif SUB_TARGET_T8010SIM
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
	case CLK_MIPI:
		freq = clks[PMGR_CLK_TEMP_MIPI_DSI];
		break;
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
	case CLK_MIPI:
		clk = PMGR_CLK_PLL2;
		break;
	default:
		break;
	}

	if (clk >= PMGR_CLK_PLL0 && clk <= PMGR_CLK_PLL5) {
		int32_t pll = clk - PMGR_CLK_PLL0;

		set_pll(pll, pll_p, pll_m, pll_s);

		clks[clk] = get_pll(pll);
		if (clock == CLK_MIPI) {
			clocks_get_frequencies_range(PMGR_CLK_TEMP_MIPI_DSI, PMGR_CLK_TEMP_MIPI_DSI);
		}
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

void power_on(int device)
{
	volatile uint32_t *reg;

	// Make sure we are within limits.
	if (!PMGR_VALID_DEVICE(device))
		return;

	reg = device_configs[PMGR_DEVICE_INDEX(device)].ps_reg;

	*reg = (*reg & ~PMGR_PS_RUN_MAX) | PMGR_PS_CLOCK_OFF;

	// Wait for the MANUAL_PS and ACTUAL_PS fields to be equal
	while ((*reg & PMGR_PS_MANUAL_PS_MASK) != ((*reg >> PMGR_PS_ACTUAL_PS_SHIFT) & PMGR_PS_ACTUAL_PS_MASK));
}

static void clocks_set_gates(uint64_t *devices, uint64_t *power)
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
		if (!((devices[idx] >> ((uint64_t)(i % 64))) & 0x1)) {
			if ((power[idx] >> ((uint64_t)(i % 64))) & 0x1)
				power_on(i);
			else
				clock_gate(i, false);
		}
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
#if (APPLICATION_IBOOT && !PRODUCT_IBOOT && !PRODUCT_IBEC)
	// Initialize temperature sensors, thermal failsafe blocks, and TVM tunables
	init_soc_thermal_sensors();
	init_cpu_thermal_sensors();
	init_soc_sochot();
	init_cpu_sochot();
	init_soc_tvm_tunables();
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
	return (rPMGR_PCIE_REFCLK_GOOD & PMGR_PCIE_REFCLK_GOOD_UMASK) != 0;
}

#if APPLICATION_IBOOT
static void set_gfx_perf_state(uint32_t state_num, enum chipid_voltage_index voltage_index)
{
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
	pll_enable = params->gpu.fbkDivM ? 1 : 0;

	rPMGR_GFX_PERF_STATE_ENTRY_A(state_num) = ((dwi_val & 0xFF) << 24) |
	((pll_enable & 0x1) << 21) |
	((params->gpu.fbkDivM & 0x1FF) << 12) |
	((params->gpu.preDivP & 0x1F) << 4) |
	(params->gpu.pstDivS & 0xF);

	rPMGR_GFX_PERF_STATE_ENTRY_B(state_num) = dwi_sram_val & 0xFF;

	return;
}
#endif

#if WITH_DEVICETREE
static uint64_t get_freq_from_acc_state(uint32_t state_index)
{
	uint64_t state_entry = rACC_DVFM_ST(state_index);
	uint64_t freq = OSC_FREQ;

	// Fcpu <= (OSC * (PLL_FCW_INT + PLL_FCW_FRAC / 5)) / (2 * (S + 1))
	freq *= (ACC_DVFM_ST_FCW_INT_XTRCT(state_entry) + (ACC_DVFM_ST_FCW_FRAC_XTRCT(state_entry) / 5.0));
	freq /= (2 * (1 + ACC_DVFM_ST_PST_DIV_S_XTRCT(state_entry)));

	return freq;
}

void pmgr_update_device_tree(DTNode *pmgr_node)
{
#if 0	// !!!FIXME!!! <rdar://problem/19722938> T8010FPGA: panic: chipid_get_cpu_voltage: Invalid CPU voltage index 1880788768
	uint32_t num_freqs = 0;
	uint32_t propSize;
	uint64_t period_ns;
	uint64_t freq = 0;
	uint32_t volt;

	char *propName;
	void *propData;

	struct dvfm_data dvfm;

	pmgr_get_dvfm_data(&dvfm);

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

		freq = get_freq_from_acc_state(dvfm.dvfm_state_vboost);
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
#endif
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
}
#endif

#if (APPLICATION_IBOOT && !PRODUCT_IBOOT && !PRODUCT_IBEC)

static void init_soc_thermal_sensors(void)
{
	// Define piecewise-linear relationship (start_codes, offsets, slopes) between output and temperature for SOC/PMGR sensors
	rPMGR_THERMAL0_PIECE0 = PMGR_THERMAL0_PIECE0(0x000, 0x1e5, 0x03b);
	rPMGR_THERMAL0_PIECE1 = PMGR_THERMAL0_PIECE1(0x140, 0x1f4, 0x02f);
	rPMGR_THERMAL0_PIECE2 = PMGR_THERMAL0_PIECE2(0x257, 0x009, 0x026);

	rPMGR_THERMAL1_PIECE0 = PMGR_THERMAL1_PIECE0(0x000, 0x1e5, 0x03b);
	rPMGR_THERMAL1_PIECE1 = PMGR_THERMAL1_PIECE1(0x140, 0x1f4, 0x02f);
	rPMGR_THERMAL1_PIECE2 = PMGR_THERMAL1_PIECE2(0x257, 0x009, 0x026);

	rPMGR_THERMAL2_PIECE0 = PMGR_THERMAL2_PIECE0(0x000, 0x1e5, 0x03b);
	rPMGR_THERMAL2_PIECE1 = PMGR_THERMAL2_PIECE1(0x140, 0x1f4, 0x02f);
	rPMGR_THERMAL2_PIECE2 = PMGR_THERMAL2_PIECE2(0x257, 0x009, 0x026);

	// Read temperature trim values from efuse and write to corresponding registers
#if 0
	rPMGR_THERMAL0_PARAM = chipid_get_soc_temp_sensor_trim(0);
	rPMGR_THERMAL1_PARAM = chipid_get_soc_temp_sensor_trim(1);
	rPMGR_THERMAL2_PARAM = chipid_get_soc_temp_sensor_trim(2);
#else
	// Fix up ACC temp sensor gain/offset values--see 18151623
	rPMGR_THERMAL0_PARAM = PMGR_THERMAL_0_PARAM_TRIMO_INSRT(0x00) | PMGR_THERMAL_0_PARAM_TRIMG_INSRT(0x0F);
	rPMGR_THERMAL1_PARAM = PMGR_THERMAL_1_PARAM_TRIMO_INSRT(0x00) | PMGR_THERMAL_1_PARAM_TRIMG_INSRT(0x0F);
	rPMGR_THERMAL2_PARAM = PMGR_THERMAL_2_PARAM_TRIMO_INSRT(0x00) | PMGR_THERMAL_2_PARAM_TRIMG_INSRT(0x0F);
#endif

	// Define sampling rate for each sensor
	rPMGR_THERMAL0_CTL1 = 0xea60;
	rPMGR_THERMAL1_CTL1 = 0xea60;
	rPMGR_THERMAL2_CTL1 = 0xea60;

	// Enable temperature sensors (need to do this before enabling either DVTM or SOCHOT)
	rPMGR_THERMAL0_CTL0_SET = PMGR_THERMAL_0_CTL0_SET_ENABLE_INSRT(1);
	rPMGR_THERMAL1_CTL0_SET = PMGR_THERMAL_1_CTL0_SET_ENABLE_INSRT(1);
	rPMGR_THERMAL2_CTL0_SET = PMGR_THERMAL_2_CTL0_SET_ENABLE_INSRT(1);
}

static void init_soc_sochot(void)
{
	// PMGR SOCHOT threshold temperatures have moved from the PMGR SOCHOT register block to the PMGR THERMAL block, ostensibly to provide greater
	// flexibility via the ability to vary failsafe temperature threshold with PMGR location.  The SOCHOT and temperature sensor drivers are separate
	// entities currently, such that it would be inconvienient to coordinate how they come up (can't enable SOCHOT action until non-zero thresholds
	// have been set, the SOCHOT driver doesn't really need to know how many temperature sensors are present, etc.)  Accordingly, let's set the
	// trip *thresholds* here and leave the drivers to do the rest, as usual.
	rPMGR_THERMAL0_FAILSAFE_TRIP_TEMP_0 = 120;
	rPMGR_THERMAL1_FAILSAFE_TRIP_TEMP_0 = 120;
	rPMGR_THERMAL2_FAILSAFE_TRIP_TEMP_0 = 120;

	rPMGR_THERMAL0_FAILSAFE_TRIP_TEMP_1 = 125;
	rPMGR_THERMAL1_FAILSAFE_TRIP_TEMP_1 = 125;
	rPMGR_THERMAL2_FAILSAFE_TRIP_TEMP_1 = 125;
}

static void init_cpu_thermal_sensors(void)
{
	// Define piecewise-linear relationship (start_codes, offsets, slopes) between output and temperature for CPU sensors
	rACC_THRM0_PIECE0 = ACC_THRM0_PIECE0(0x000, 0x0e5, 0x03b);
	rACC_THRM0_PIECE1 = ACC_THRM0_PIECE1(0x140, 0x0f4, 0x02f);
	rACC_THRM0_PIECE2 = ACC_THRM0_PIECE2(0x257, 0x009, 0x026);

	rACC_THRM1_PIECE0 = ACC_THRM1_PIECE0(0x000, 0x0e5, 0x03b);
	rACC_THRM1_PIECE1 = ACC_THRM1_PIECE1(0x140, 0x0f4, 0x02f);
	rACC_THRM1_PIECE2 = ACC_THRM1_PIECE2(0x257, 0x009, 0x026);

	rACC_THRM2_PIECE0 = ACC_THRM2_PIECE0(0x000, 0x0e5, 0x03b);
	rACC_THRM2_PIECE1 = ACC_THRM2_PIECE1(0x140, 0x0f4, 0x02f);
	rACC_THRM2_PIECE2 = ACC_THRM2_PIECE2(0x257, 0x009, 0x026);

	// Fix up ACC temp sensor gain/offset values--see 18151623
	rACC_THRM0_PARAM = ACC_THERMAL_THRM0_PARAM_TRIMO_INSRT(0x00) | ACC_THERMAL_THRM0_PARAM_TRIMG_INSRT(0x0F);
	rACC_THRM1_PARAM = ACC_THERMAL_THRM1_PARAM_TRIMO_INSRT(0x00) | ACC_THERMAL_THRM1_PARAM_TRIMG_INSRT(0x0F);
	rACC_THRM2_PARAM = ACC_THERMAL_THRM2_PARAM_TRIMO_INSRT(0x00) | ACC_THERMAL_THRM2_PARAM_TRIMG_INSRT(0x0F);

	// Define sampling rate for each sensor
	rACC_THRM0_CTL1 = 0xea60;
	rACC_THRM1_CTL1 = 0xea60;
	rACC_THRM2_CTL1 = 0xea60;

	// Enable temperature sensors (need to do this before enabling either DVTM or SOCHOT)
	rACC_THRM0_CTL0_SET = ACC_THERMAL_THRM0_CTL0_SET_ENABLE_INSRT(1);
	rACC_THRM1_CTL0_SET = ACC_THERMAL_THRM1_CTL0_SET_ENABLE_INSRT(1);
	rACC_THRM2_CTL0_SET = ACC_THERMAL_THRM2_CTL0_SET_ENABLE_INSRT(1);
}

static void init_cpu_sochot(void)
{
	// Set target state for when SOCHOT0 triggers
	rACC_DVFM_FSHOT_IDX = ACC_THERMAL_DVFM_FSHOT_IDX_0_ST_INSRT(kDVFM_STATE_V0);
}

static void init_soc_tvm_tunables(void)
{
}

#endif
