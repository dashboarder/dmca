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
#ifndef __PLATFORM_SOC_PMGR_H
#define __PLATFORM_SOC_PMGR_H

#include <lib/devicetree.h>
#include <platform/soc/hwregbase.h>

////////////////////////////////////////////////////////////
//
//  PMGR specific
//
////////////////////////////////////////////////////////////

#define PMGR_PLL_COUNT			(6)

#define rPMGR_PLL_CTL(_p)		(*(volatile uint32_t *)(PMGR_BASE_ADDR + ((_p) * 0x1000)))
#define	 PMGR_PLL_ENABLE		(1 << 31)
#define	 PMGR_PLL_LOAD			(1 << 29)
#define	 PMGR_PLL_PENDING		(1 << 25)
#define	 PMGR_PLL_M_SHIFT		(12)
#define	 PMGR_PLL_M_MASK		(0x1FF)
#define	 PMGR_PLL_P_SHIFT		(4)
#define	 PMGR_PLL_P_MASK		(0x1F)
#define	 PMGR_PLL_S_SHIFT		(0)
#define	 PMGR_PLL_S_MASK		(0xF)
#define	 PMGR_PLL_M(_m)			(((_m) & PMGR_PLL_M_MASK) << PMGR_PLL_M_SHIFT)
#define	 PMGR_PLL_P(_p)			(((_p) & PMGR_PLL_P_MASK) << PMGR_PLL_P_SHIFT)
#define	 PMGR_PLL_S(_s)			(((_s) & PMGR_PLL_S_MASK) << PMGR_PLL_S_SHIFT)
#define	 PMGR_PLL_FREQ(_m, _p, _s)	((((_m) * OSC_FREQ) / (_p))/((_s) + 1))

#define rPMGR_PLL_EXT_BYPASS_CTL(_p)	(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x0004 + ((_p) * 0x1000)))
#define	 PMGR_PLL_BYP_ENABLED		(1 << 31)
#define	 PMGR_PLL_ENABLED		(1 << 30)
#define	 PMGR_PLL_EXT_BYPASS		(1 << 0)	// Do not change while PMGR_PLL_PENDING is high

#define rPMGR_PLL_CFG(_p)		(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x0008 + ((_p) * 0x1000)))
#define	 PMGR_PLL_OFF_MODE_MASK		(0x3)
#define	 PMGR_PLL_OFF_MODE(_n)		(((_n) & PMGR_PLL_OFF_MODE_MASK) << 30)
#define	 PMGR_PLL_OFF_MODE_RESET	(0)
#define	 PMGR_PLL_OFF_MODE_CLOCK_GATED	(1)
#define	 PMGR_PLL_OFF_MODE_POWER_DOWN	(2)
#define	 PMGR_PLL_LOCK_MODE_SHIFT	(28)
#define	 PMGR_PLL_LOCK_MODE_MASK	(0x3)
#define	 PMGR_PLL_VCO_OUT_SEL		(1 << 27)
#define	 PMGR_PLL_AUTO_DISABLE		(1 << 26)
#define	 PMGR_PLL_RELOCK_MODE_SHIFT	(24)
#define	 PMGR_PLL_RELOCK_MODE_MASK	(3)
#define	 PMGR_PLL_RELOCK_MODE_STOP	(0)
#define	 PMGR_PLL_RELOCK_MODE_BYPASS	(1)
#define	 PMGR_PLL_FRAC_LOCK_TIME_MASK	(0xFF)
#define	 PMGR_PLL_FRAC_LOCK_TIME(_n)	(((_n) & PMGR_PLL_FRAC_LOCK_TIME_MASK) << 16)
#define	 PMGR_PLL_LOCK_TIME_MASK	(0xFFFF)
#define	 PMGR_PLL_LOCK_TIME(_n)		(((_n) & PMGR_PLL_LOCK_TIME_MASK) << 0)

#define rPMGR_PLL_ANA_PARAMS0(_p)	(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x000c + ((_p) * 0x1000)))
#define  PMGR_PLL_ANA_PARAMS0_VREG_ADJ_SHIFT     	(25)
#define  PMGR_PLL_ANA_PARAMS0_VREG_ADJ_MASK     	(0x7)

#define rPMGR_PLL_ANA_PARAMS1(_p)	(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x0010 + ((_p) * 0x1000)))
#define  PMGR_PLL_ANA_PARAMS1_VCO_RCTRL_SEL 		(1 << 7)
#define  PMGR_PLL_ANA_PARAMS1_VCO_RCTRL_OW_SHIFT	(1)
#define  PMGR_PLL_ANA_PARAMS1_VCO_RCTRL_OW_MASK 	(0x7)

#define rPMGR_PLL_DEBUG1(_p)		(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x001c + ((_p) * 0x1000)))
#define  PMGR_PLL_DEBUG1_RESERVE_IN_SHIFT		(20)
#define  PMGR_PLL_DEBUG1_RESERVE_IN_MASK 		(0x3f)

#define rPMGR_MCU_FIXED_CLK_CFG		(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x10000))
#define rPMGR_MCU_CLK_CFG		(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x10004))

#if SUB_PLATFORM_T7000

#define rPMGR_MIPI_DSI_CLK_CFG		(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x10008))
#define rPMGR_NCO_REF0_CLK_CFG		(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x1000c))
#define rPMGR_NCO_REF1_CLK_CFG		(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x10010))
#define rPMGR_NCO_ALG0_CLK_CFG		(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x10014))
#define rPMGR_NCO_ALG1_CLK_CFG		(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x10018))
#define rPMGR_HSICPHY_REF_12M_CLK_CFG	(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x1001c))
#define rPMGR_USB480_0_CLK_CFG		(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x10020))
#define rPMGR_USB480_1_CLK_CFG		(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x10024))
#define rPMGR_USB_OHCI_48M_CLK_CFG	(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x10028))
#define rPMGR_USB_CLK_CFG		(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x1002c))
#define rPMGR_USB_FREE_60M_CLK_CFG	(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x10030))
#define rPMGR_SIO_C_CLK_CFG		(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x10034))
#define rPMGR_SIO_P_CLK_CFG		(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x10038))
#define rPMGR_ISP_C_CLK_CFG		(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x1003c))
#define rPMGR_ISP_CLK_CFG		(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x10040))
#define rPMGR_ISP_SENSOR0_REF_CLK_CFG	(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x10044))
#define rPMGR_ISP_SENSOR1_REF_CLK_CFG	(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x10048))
#define rPMGR_VDEC_CLK_CFG		(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x1004c))
#define rPMGR_VENC_CLK_CFG		(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x10050))
#define rPMGR_VID0_CLK_CFG		(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x10054))
#define rPMGR_DISP0_CLK_CFG		(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x10058))
#define rPMGR_DISP1_CLK_CFG		(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x1005c))
#define rPMGR_AJPEG_IP_CLK_CFG		(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x10060))
#define rPMGR_AJPEG_WRAP_CLK_CFG	(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x10064))
#define rPMGR_MSR_CLK_CFG		(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x10068))
#define rPMGR_AF_CLK_CFG		(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x1006c))
#define rPMGR_LIO_CLK_CFG		(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x10070))
#define rPMGR_MCA0_M_CLK_CFG		(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x10074))
#define rPMGR_MCA1_M_CLK_CFG		(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x10078))
#define rPMGR_MCA2_M_CLK_CFG		(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x1007c))
#define rPMGR_MCA3_M_CLK_CFG		(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x10080))
#define rPMGR_MCA4_M_CLK_CFG		(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x10084))
#define rPMGR_SEP_CLK_CFG		(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x10088))
#define rPMGR_GPIO_CLK_CFG		(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x1008c))
#define rPMGR_SPI0_N_CLK_CFG		(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x10090))
#define rPMGR_SPI1_N_CLK_CFG		(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x10094))
#define rPMGR_SPI2_N_CLK_CFG		(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x10098))
#define rPMGR_SPI3_N_CLK_CFG		(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x1009c))
#define rPMGR_DEBUG_CLK_CFG		(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x100a0))
#define rPMGR_PCIE_REF_CLK_CFG		(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x100a4))
#define rPMGR_PCIE_APP_CLK_CFG		(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x100a8))
#define rPMGR_TMPS_CLK_CFG		(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x100ac))
#define rPMGR_MEDIA_AF_CLK_CFG		(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x100b0))
#define rPMGR_ISP_AF_CLK_CFG		(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x100b4))
#define rPMGR_GFX_AF_CLK_CFG		(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x100b8))
#define rPMGR_ANS_C_CLK_CFG		(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x100bc))
#define rPMGR_ANC_LINK_CLK_CFG		(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x100c0))

#elif SUB_PLATFORM_T7001

#define rPMGR_NCO_REF0_CLK_CFG		(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x10008))
#define rPMGR_NCO_REF1_CLK_CFG		(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x1000c))
#define rPMGR_NCO_ALG0_CLK_CFG		(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x10010))
#define rPMGR_NCO_ALG1_CLK_CFG		(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x10014))
#define rPMGR_HSICPHY_REF_12M_CLK_CFG		(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x10018))
#define rPMGR_USB480_0_CLK_CFG	(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x1001c))
#define rPMGR_USB480_1_CLK_CFG		(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x10020))
#define rPMGR_USB_OHCI_48M_CLK_CFG		(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x10024))
#define rPMGR_USB_CLK_CFG	(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x10028))
#define rPMGR_USB_FREE_60M_CLK_CFG		(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x1002c))
#define rPMGR_SIO_C_CLK_CFG	(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x10030))
#define rPMGR_SIO_P_CLK_CFG		(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x10034))
#define rPMGR_ISP_C_CLK_CFG		(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x10038))
#define rPMGR_ISP_CLK_CFG		(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x1003c))
#define rPMGR_ISP_SENSOR0_REF_CLK_CFG		(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x10040))
#define rPMGR_ISP_SENSOR1_REF_CLK_CFG	(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x10044))
#define rPMGR_VDEC_CLK_CFG	(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x10048))
#define rPMGR_VENC_CLK_CFG		(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x1004c))
#define rPMGR_VID0_CLK_CFG		(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x10050))
#define rPMGR_DISP0_CLK_CFG		(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x10054))
#define rPMGR_DISP1_CLK_CFG		(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x10058))
#define rPMGR_AJPEG_IP_CLK_CFG		(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x1005c))
#define rPMGR_AJPEG_WRAP_CLK_CFG		(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x10060))
#define rPMGR_MSR_CLK_CFG	(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x10064))
#define rPMGR_AF_CLK_CFG		(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x10068))
#define rPMGR_LIO_CLK_CFG		(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x1006c))
#define rPMGR_MCA0_M_CLK_CFG		(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x10070))
#define rPMGR_MCA1_M_CLK_CFG		(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x10074))
#define rPMGR_MCA2_M_CLK_CFG		(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x10078))
#define rPMGR_MCA3_M_CLK_CFG		(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x1007c))
#define rPMGR_MCA4_M_CLK_CFG		(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x10080))
#define rPMGR_SEP_CLK_CFG		(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x10084))
#define rPMGR_GPIO_CLK_CFG		(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x10088))
#define rPMGR_SPI0_N_CLK_CFG		(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x1008c))
#define rPMGR_SPI1_N_CLK_CFG		(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x10090))
#define rPMGR_SPI2_N_CLK_CFG		(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x10094))
#define rPMGR_SPI3_N_CLK_CFG		(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x10098))
#define rPMGR_DEBUG_CLK_CFG		(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x1009c))
#define rPMGR_PCIE_APP_CLK_CFG		(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x100a0))
#define rPMGR_TMPS_CLK_CFG		(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x100a4))
#define rPMGR_MEDIA_AF_CLK_CFG		(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x100a8))
#define rPMGR_ISP_AF_CLK_CFG		(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x100ac))
#define rPMGR_GFX_AF_CLK_CFG		(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x100b0))
#define rPMGR_ANS_C_CLK_CFG		(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x100b4))
#define rPMGR_ANC_LINK_CLK_CFG		(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x100b8))

#endif

#define PMGR_FIRST_CLK_CFG		(&rPMGR_MCU_FIXED_CLK_CFG)
#define PMGR_LAST_CLK_CFG		(&rPMGR_ANC_LINK_CLK_CFG)
#define PMGR_CLK_CFG_COUNT		(((((void *)(PMGR_LAST_CLK_CFG)) - ((void *)(PMGR_FIRST_CLK_CFG))) / 4) + 1)
#define PMGR_CLK_NUM(_r)		(((void *)&(rPMGR_ ## _r ## _CLK_CFG) - ((void *)(PMGR_FIRST_CLK_CFG))) / 4)

#define rPMGR_S0_CLK_CFG		(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x10208))
#define rPMGR_S1_CLK_CFG		(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x1020c))
#define rPMGR_S2_CLK_CFG		(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x10210))
#define rPMGR_S3_CLK_CFG		(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x10214))
#define rPMGR_ISP_REF0_CLK_CFG		(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x10218))
#define rPMGR_ISP_REF1_CLK_CFG		(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x1021c))

#define PMGR_FIRST_SPARE_CLK_CFG	(&rPMGR_S0_CLK_CFG)
#define PMGR_LAST_SPARE_CLK_CFG		(&rPMGR_ISP_REF1_CLK_CFG)
#define PMGR_SPARE_CLK_CFG_COUNT	(((((void *)(PMGR_LAST_SPARE_CLK_CFG)) - ((void *)(PMGR_FIRST_SPARE_CLK_CFG))) / 4) + 1)

#define rPMGR_CLK_DIVIDER_ACG_CFG	(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x10308))

#define PMGR_CLK_CFG_ENABLE		(1 << 31)
#define PMGR_CLK_CFG_PENDING		(1 << 30)
#define PMGR_CLK_CFG_PENDING_SHIFT	(30)
#define PMGR_CLK_CFG_SRC_SEL_MASK	(0x3F)
#define PMGR_CLK_CFG_CFG_SEL_SHIFT	(16)
#define PMGR_CLK_CFG_SRC_SEL_SHIFT	(24)
#define PMGR_CLK_CFG_SRC_SEL(_n)	((_n) << PMGR_CLK_CFG_SRC_SEL_SHIFT)
#define PMGR_CLK_CFG_DIVISOR_MASK	(0x3F)
#define PMGR_CLK_CFG_DIVISOR(_n)	((_n) << 0)

#define rPMGR_NCO_CLK_CFG(_n)		(*(volatile uint32_t *) (PMGR_BASE_ADDR + (_n) * 0x1000 + 0x11000 + 0x0))
#define rPMGR_NCO_DIV_INT(_n)		(*(volatile uint32_t *) (PMGR_BASE_ADDR + (_n) * 0x1000 + 0x11000 + 0x4))
#define rPMGR_NCO_DIV_FRAC_N1(_n)	(*(volatile uint32_t *) (PMGR_BASE_ADDR + (_n) * 0x1000 + 0x11000 + 0x8))
#define rPMGR_NCO_DIV_FRAC_N2(_n)	(*(volatile uint32_t *) (PMGR_BASE_ADDR + (_n) * 0x1000 + 0x11000 + 0xc))
#define rPMGR_NCO_DIV_FRAC_PRIME(_n)	(*(volatile uint32_t *) (PMGR_BASE_ADDR + (_n) * 0x1000 + 0x11000 + 0x10))

#define PMGR_NCO_CLK_CFG_PENDING	(1 << 30)

#define PMGR_NUM_NCO	5

#define kSOC_PERF_STATE_BYPASS			(0)
#define kSOC_PERF_STATE_SECUREROM		(kSOC_PERF_STATE_BYPASS + 1)
#define kSOC_PERF_STATE_IBOOT			(kSOC_PERF_STATE_BYPASS + 2)
#define kSOC_PERF_STATE_IBOOT_MEM_LOW_PERF	(kSOC_PERF_STATE_IBOOT + 0)
#define kSOC_PERF_STATE_IBOOT_MEM_FULL_PERF	(kSOC_PERF_STATE_IBOOT + 1)
#define kSOC_PERF_STATE_VMIN			(kSOC_PERF_STATE_IBOOT_MEM_FULL_PERF + 0)
#define kSOC_PERF_STATE_VNOM			(kSOC_PERF_STATE_IBOOT_MEM_FULL_PERF + 1)

#define kSOC_PERF_STATE_VMAX		(kSOC_PERF_STATE_VNOM)

#define kSOC_PERF_STATE_IBOOT_CNT	(kSOC_PERF_STATE_VMAX + 1)

#define rPMGR_SOC_PERF_STATE_CTL		(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x1a000))

#define PMGR_SOC_PERF_STATE_CTL_PENDING_MASK		(0xFFFF0000)
#define PMGR_SOC_PERF_STATE_CTL_CURRENT_SELECT_SHIFT	(8)
#define PMGR_SOC_PERF_STATE_CTL_CURRENT_SELECT_MASK	(0xF)

#define rPMGR_SOC_PERF_STATE_SOCHOT		(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x1a004))
#define PMGR_SOC_PERF_STATE_SOCHOT_ENABLE_TRIG1		(1 << 31)
#define PMGR_SOC_PERF_STATE_SOCHOT_ENABLE_TRIG0		(1 << 30)
#define PMGR_SOC_PERF_STATE_SOCHOT_TRIG1_SELECT(_n)     (((_n) & 0xf) << 4)
#define PMGR_SOC_PERF_STATE_SOCHOT_TRIG0_SELECT(_n)     (((_n) & 0xf) << 0)

#define rPMGR_SOC_PERF_STATE_ENTRY_A(_n)	(*(volatile uint32_t *)(PMGR_BASE_ADDR + (_n) * 0x10 + 0x1a010))
#define rPMGR_SOC_PERF_STATE_ENTRY_B(_n)	(*(volatile uint32_t *)(PMGR_BASE_ADDR + (_n) * 0x10 + 0x1a014))
#define rPMGR_SOC_PERF_STATE_ENTRY_C(_n)	(*(volatile uint32_t *)(PMGR_BASE_ADDR + (_n) * 0x10 + 0x1a018))
#define rPMGR_SOC_PERF_STATE_ENTRY_D(_n)	(*(volatile uint32_t *)(PMGR_BASE_ADDR + (_n) * 0x10 + 0x1a01c))
#define PMGR_SOC_PERF_STATE_VOL_ADJ0(_n)	((_n & 0x3f) << (0 * 8))
#define PMGR_SOC_PERF_STATE_VOL_ADJ1(_n)	((_n & 0x3f) << (1 * 8))
#define PMGR_SOC_PERF_STATE_VOL_ADJ2(_n)	((_n & 0x3f) << (2 * 8))
#define PMGR_SOC_PERF_STATE_VOL_ADJ3(_n)	((_n & 0x3f) << (3 * 8))


#define PMGR_SOC_PERF_STATE_ENTRY_MCU_CFG_SEL_MASK		(0x00000300)
#define PMGR_SOC_PERF_STATE_ENTRY_MCU_SRC_SEL_MASK		(0x000000F0)
#define PMGR_SOC_PERF_STATE_ENTRY_MCU_FIXED_SRC_SEL_MASK	(0x00000001)
#define PMGR_SOC_PERF_STATE_ENTRY_MCU_MASK			(PMGR_SOC_PERF_STATE_ENTRY_MCU_CFG_SEL_MASK |		\
								 PMGR_SOC_PERF_STATE_ENTRY_MCU_SRC_SEL_MASK |		\
								 PMGR_SOC_PERF_STATE_ENTRY_MCU_FIXED_SRC_SEL_MASK)

#define PMGR_SOC_PERF_STATE_COUNT (16)

#define rPMGR_AGILE_CLK_CTL		(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x1b000))

#define kPMGR_GFX_STATE_MAX		(16)
#define rPMGR_GFX_PERF_STATE_ENTRY(_n)	(*(volatile uint32_t *)(PMGR_BASE_ADDR + (_n) * 0x10 + 0x1c000))
#if SUB_PLATFORM_T7001
#define rPMGR_GFX_SRAM_PERF_STATE_ENTRY(_n)	(*(volatile uint32_t *)(PMGR_BASE_ADDR + (_n) * 0x10 + 0x1c004))
#endif
#define rPMGR_GFX_PERF_STATE_CTL	(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x1c200))
#define rPMGR_GFX_PERF_STATE_SOCHOT	(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x1c300))
#define PMGR_GFX_PERF_STATE_SOCHOT_ENABLE_TRIG1		(1 << 31)
#define PMGR_GFX_PERF_STATE_SOCHOT_ENABLE_TRIG0		(1 << 30)
#define PMGR_GFX_PERF_STATE_SOCHOT_TRIG1_SELECT(_n)     (((_n) & 0xf) << 4)
#define PMGR_GFX_PERF_STATE_SOCHOT_TRIG0_SELECT(_n)     (((_n) & 0xf) << 0)

#define rPMGR_CPU0_PS			(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x20000))
#define rPMGR_CPU1_PS			(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x20008))
#if SUB_PLATFORM_T7001
#define rPMGR_CPU2_PS			(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x20010))
#endif
#define rPMGR_CPM_PS			(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x20040))
#define rPMGR_LIO_PS			(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x20100))
#define rPMGR_IOMUX_PS			(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x20108))
#define rPMGR_AIC_PS			(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x20110))
#define rPMGR_DEBUG_PS			(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x20118))
#define rPMGR_DWI_PS			(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x20120))
#define rPMGR_GPIO_PS			(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x20128))
#define rPMGR_MCA0_PS			(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x20130))
#define rPMGR_MCA1_PS			(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x20138))
#define rPMGR_MCA2_PS			(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x20140))
#define rPMGR_MCA3_PS			(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x20148))
#define rPMGR_MCA4_PS			(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x20150))
#define rPMGR_PWM0_PS			(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x20158))
#define rPMGR_I2C0_PS			(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x20160))
#define rPMGR_I2C1_PS			(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x20168))
#define rPMGR_I2C2_PS			(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x20170))
#define rPMGR_I2C3_PS			(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x20178))
#define rPMGR_SPI0_PS			(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x20180))
#define rPMGR_SPI1_PS			(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x20188))
#define rPMGR_SPI2_PS			(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x20190))
#define rPMGR_SPI3_PS			(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x20198))
#define rPMGR_UART0_PS			(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x201a0))
#define rPMGR_UART1_PS			(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x201a8))
#define rPMGR_UART2_PS			(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x201b0))
#define rPMGR_UART3_PS			(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x201b8))
#define rPMGR_UART4_PS			(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x201c0))
#define rPMGR_UART5_PS			(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x201c8))
#define rPMGR_UART6_PS			(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x201d0))
#define rPMGR_UART7_PS			(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x201d8))
#define rPMGR_UART8_PS			(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x201e0))
#define rPMGR_AES0_PS			(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x201e8))
#define rPMGR_SIO_PS			(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x201f0))
#define rPMGR_SIO_P_PS			(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x201f8))
#define rPMGR_HSIC0PHY_PS		(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x20200))
#define rPMGR_HSIC1PHY_PS		(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x20208))
#define rPMGR_ISPSENS0_PS		(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x20210))
#define rPMGR_ISPSENS1_PS		(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x20218))
#define rPMGR_PCIE_REF_PS		(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x20220))
#define rPMGR_ANP_PS			(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x20228))
#define rPMGR_MCC_PS			(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x20230))
#define rPMGR_MCU_PS			(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x20238))
#define rPMGR_AMP_PS			(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x20240))
#define rPMGR_USB_PS			(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x20248))
#define rPMGR_USBCTLREG_PS		(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x20250))
#define rPMGR_USB2HOST0_PS		(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x20258))
#define rPMGR_USB2HOST0_OHCI_PS		(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x20260))
#define rPMGR_USB2HOST1_PS		(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x20268))
#define rPMGR_USB2HOST1_OHCI_PS		(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x20270))
#define rPMGR_USB2HOST2_PS		(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x20278))
#define rPMGR_USB2HOST2_OHCI_PS		(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x20280))
#define rPMGR_USB_OTG_PS		(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x20288))
#define rPMGR_SMX_PS			(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x20290))
#define rPMGR_SF_PS			(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x20298))
#define rPMGR_CP_PS			(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x202a0))
#if SUB_PLATFORM_T7000
#define rPMGR_DISP_BUSMUX_PS		(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x202a8))
#elif SUB_PLATFORM_T7001
#define rPMGR_DISP0_BUSIF_PS		(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x202a8))
#endif
#define rPMGR_DISP0_PS			(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x202b0))
#if SUB_PLATFORM_T7000
#define rPMGR_MIPI_DSI_PS		(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x202b8))
#define rPMGR_DP_PS			(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x202c0))
#elif SUB_PLATFORM_T7001
#define rPMGR_DP_PS			(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x202b8))
#define rPMGR_DISP1_BUSIF_PS		(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x202c0))
#endif
#define rPMGR_DISP1_PS			(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x202c8))
#define rPMGR_ISP_PS			(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x202d0))
#define rPMGR_MEDIA_PS			(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x202d8))
#define rPMGR_MSR_PS			(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x202e0))
#define rPMGR_JPG_PS			(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x202e8))
#define rPMGR_VDEC0_PS			(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x202f0))
#define rPMGR_VENC_CPU_PS		(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x20300))
#define rPMGR_PCIE_PS			(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x20308))
#define rPMGR_PCIE_AUX_PS		(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x20310))
#define rPMGR_ANS_PS			(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x20318))
#define rPMGR_GFX_PS			(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x20320))
#define rPMGR_SEP_PS			(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x20400))
#define rPMGR_VENC_PIPE_PS		(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x21000))
#define rPMGR_VENC_ME0_PS		(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x21008))
#define rPMGR_VENC_ME1_PS		(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x21010))

#define	 PMGR_PS_RESET			(1 << 31)
#define	 PMGR_PS_AUTO_PM_EN		(1 << 28)
#define	 PMGR_PS_FORCE_NOACCESS		(1 << 10)
#define	 PMGR_PS_ACTUAL_PS_SHIFT	(4)
#define	 PMGR_PS_ACTUAL_PS_MASK		(0xF)
#define	 PMGR_PS_MANUAL_PS_MASK		(0xF)
#define	 PMGR_PS_RUN_MAX		(0xF)
#define	 PMGR_PS_CLOCK_OFF		(0x4)
#define	 PMGR_PS_POWER_OFF		(0x0)

#define PMGR_FIRST_PS			(&rPMGR_CPU0_PS)
#define PMGR_LAST_PS			(&rPMGR_VENC_ME1_PS)
#define PMGR_PS_COUNT			(((((void *)(PMGR_LAST_PS)) - ((void *)(PMGR_FIRST_PS))) / 8) + 1)
#define PMGR_PS_NUM(_r)			(((void *)&(rPMGR_ ## _r ## _PS) - ((void *)(PMGR_FIRST_PS))) / 8)

#define rPMGR_PWRGATE_CPU0_CFG0		(*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x22000))
#define rPMGR_PWRGATE_CPU1_CFG0		(*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x22010))
#if SUB_PLATFORM_T7001
#define rPMGR_PWRGATE_CPU2_CFG0		(*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x22020))
#endif
#define rPMGR_PWRGATE_CPM_CFG0		(*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x22080))
#define rPMGR_PWRGATE_AMC_CFG0		(*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x22090))
#define rPMGR_PWRGATE_AMC_CFG1		(*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x22094))
#define rPMGR_PWRGATE_AMC_CFG2		(*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x22098))
#define rPMGR_PWRGATE_AMC_DBG		(*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x2209c))
#define rPMGR_PWRGATE_USB_CFG0		(*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x220a0))
#define rPMGR_PWRGATE_USB_CFG1		(*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x220a4))
#define rPMGR_PWRGATE_USB_CFG2		(*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x220a8))
#define rPMGR_PWRGATE_USB_DBG		(*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x220ac))
#define rPMGR_PWRGATE_ACS_CFG0		(*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x220b0))
#define rPMGR_PWRGATE_ACS_CFG1		(*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x220b4))
#define rPMGR_PWRGATE_ACS_CFG2		(*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x220b8))
#define rPMGR_PWRGATE_ACS_DBG		(*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x220bc))
#define rPMGR_PWRGATE_DISP0_CFG0	(*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x220c0))
#define rPMGR_PWRGATE_DISP0_CFG1	(*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x220c4))
#define rPMGR_PWRGATE_DISP0_CFG2	(*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x220c8))
#define rPMGR_PWRGATE_DISP0_DBG		(*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x220cc))
#define rPMGR_PWRGATE_DISP1_CFG0	(*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x220d0))
#define rPMGR_PWRGATE_DISP1_CFG1	(*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x220d4))
#define rPMGR_PWRGATE_DISP1_CFG2	(*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x220d8))
#define rPMGR_PWRGATE_DISP1_DBG		(*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x220dc))
#define rPMGR_PWRGATE_ISP_CFG0		(*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x220e0))
#define rPMGR_PWRGATE_ISP_CFG1		(*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x220e4))
#define rPMGR_PWRGATE_ISP_CFG2		(*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x220e8))
#define rPMGR_PWRGATE_ISP_DBG		(*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x220ec))
#define rPMGR_PWRGATE_MEDIA_CFG0	(*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x220f0))
#define rPMGR_PWRGATE_MEDIA_CFG1	(*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x220f4))
#define rPMGR_PWRGATE_MEDIA_CFG2	(*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x220f8))
#define rPMGR_PWRGATE_MEDIA_DBG		(*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x220fc))
#define rPMGR_PWRGATE_VDEC0_CFG0	(*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x22100))
#define rPMGR_PWRGATE_VDEC0_CFG1	(*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x22104))
#define rPMGR_PWRGATE_VDEC0_CFG2	(*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x22108))
#define rPMGR_PWRGATE_VDEC0_DBG	(*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x2210c))
#if SUB_PLATFORM_T7000
#define rPMGR_PWRGATE_VENC_CPU_CFG0	(*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x22120))
#define rPMGR_PWRGATE_VENC_CPU_CFG1	(*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x22124))
#define rPMGR_PWRGATE_VENC_CPU_CFG2	(*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x22128))
#define rPMGR_PWRGATE_VENC_CPU_DBG	(*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x2212c))
#define rPMGR_PWRGATE_PCIE_CFG0		(*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x22130))
#define rPMGR_PWRGATE_PCIE_CFG1		(*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x22134))
#define rPMGR_PWRGATE_PCIE_CFG2		(*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x22138))
#define rPMGR_PWRGATE_PCIE_DBG		(*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x2213c))
#define rPMGR_PWRGATE_ANS_CFG0		(*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x22140))
#define rPMGR_PWRGATE_ANS_CFG1		(*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x22144))
#define rPMGR_PWRGATE_ANS_CFG2		(*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x22148))
#define rPMGR_PWRGATE_ANS_DBG		(*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x2214c))
#define rPMGR_PWRGATE_GFX_CFG0		(*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x22150))
#define rPMGR_PWRGATE_GFX_CFG1		(*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x22154))
#define rPMGR_PWRGATE_GFX_CFG2		(*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x22158))
#define rPMGR_PWRGATE_GFX_DBG		(*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x2215c))
#define rPMGR_PWRGATE_SEP_CFG0		(*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x22160))
#define rPMGR_PWRGATE_SEP_CFG1		(*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x22164))
#define rPMGR_PWRGATE_SEP_CFG2		(*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x22168))
#define rPMGR_PWRGATE_SEP_DBG		(*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x2216c))
#define rPMGR_PWRGATE_VENC_PIPE_CFG0	(*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x22170))
#define rPMGR_PWRGATE_VENC_PIPE_CFG1	(*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x22174))
#define rPMGR_PWRGATE_VENC_PIPE_CFG2	(*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x22178))
#define rPMGR_PWRGATE_VENC_PIPE_DBG	(*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x2217c))
#define rPMGR_PWRGATE_VENC_ME0_CFG0	(*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x22180))
#define rPMGR_PWRGATE_VENC_ME0_CFG1	(*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x22184))
#define rPMGR_PWRGATE_VENC_ME0_CFG2	(*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x22188))
#define rPMGR_PWRGATE_VENC_ME0_DBG	(*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x2218c))
#define rPMGR_PWRGATE_VENC_ME1_CFG0	(*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x22190))
#define rPMGR_PWRGATE_VENC_ME1_CFG1	(*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x22194))
#define rPMGR_PWRGATE_VENC_ME1_CFG2	(*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x22198))
#define rPMGR_PWRGATE_VENC_ME1_DBG	(*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x2219c))
#define rPMGR_PWRGATE_MCU_ASYNC_RESET	(*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x22300))
#elif SUB_PLATFORM_T7001
#define rPMGR_PWRGATE_VENC_CPU_CFG0	(*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x22110))
#define rPMGR_PWRGATE_VENC_CPU_CFG1	(*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x22114))
#define rPMGR_PWRGATE_VENC_CPU_CFG2	(*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x22118))
#define rPMGR_PWRGATE_VENC_CPU_DBG	(*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x2211c))
#define rPMGR_PWRGATE_PCIE_CFG0		(*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x22120))
#define rPMGR_PWRGATE_PCIE_CFG1		(*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x22124))
#define rPMGR_PWRGATE_PCIE_CFG2		(*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x22128))
#define rPMGR_PWRGATE_PCIE_DBG		(*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x2212c))
#define rPMGR_PWRGATE_ANS_CFG0		(*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x22130))
#define rPMGR_PWRGATE_ANS_CFG1		(*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x22134))
#define rPMGR_PWRGATE_ANS_CFG2		(*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x22138))
#define rPMGR_PWRGATE_ANS_DBG		(*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x2213c))
#define rPMGR_PWRGATE_GFX_CFG0		(*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x22140))
#define rPMGR_PWRGATE_GFX_CFG1		(*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x22144))
#define rPMGR_PWRGATE_GFX_CFG2		(*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x22148))
#define rPMGR_PWRGATE_GFX_DBG		(*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x2214c))
#define rPMGR_PWRGATE_SEP_CFG0		(*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x22150))
#define rPMGR_PWRGATE_SEP_CFG1		(*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x22154))
#define rPMGR_PWRGATE_SEP_CFG2		(*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x22158))
#define rPMGR_PWRGATE_SEP_DBG		(*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x2215c))
#define rPMGR_PWRGATE_VENC_PIPE_CFG0	(*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x22160))
#define rPMGR_PWRGATE_VENC_PIPE_CFG1	(*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x22164))
#define rPMGR_PWRGATE_VENC_PIPE_CFG2	(*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x22168))
#define rPMGR_PWRGATE_VENC_PIPE_DBG	(*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x2216c))
#define rPMGR_PWRGATE_VENC_ME0_CFG0	(*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x22170))
#define rPMGR_PWRGATE_VENC_ME0_CFG1	(*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x22174))
#define rPMGR_PWRGATE_VENC_ME0_CFG2	(*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x22178))
#define rPMGR_PWRGATE_VENC_ME0_DBG	(*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x2217c))
#define rPMGR_PWRGATE_VENC_ME1_CFG0	(*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x22180))
#define rPMGR_PWRGATE_VENC_ME1_CFG1	(*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x22184))
#define rPMGR_PWRGATE_VENC_ME1_CFG2	(*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x22188))
#define rPMGR_PWRGATE_VENC_ME1_DBG	(*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x2218c))
#endif
#define rPMGR_PWRGATE_MCU_ASYNC_RESET	(*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x22300))

#define rPMGR_VOLMAN_CTL		(*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x23000))
#define rPMGR_VOLMAN_VAR_SOC_VOLTAGE	(*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x23004))
#define rPMGR_VOLMAN_FIXED_VOLTAGE	(*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x23008))
#define rPMGR_VOLMAN_GFX_VOLTAGE	(*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x2300c))
#define rPMGR_VOLMAN_CPU_VOLTAGE	(*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x23010))
#define rPMGR_VOLMAN_VAR_SOC_DELAY	(*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x23014))
#define rPMGR_VOLMAN_FIXED_DELAY	(*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x23018))
#define rPMGR_VOLMAN_GFX_DELAY		(*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x2301c))
#define rPMGR_VOLMAN_CPU_DELAY		(*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x23020))
#define	 PMGR_VOLMAN_DISABLE_CPU_VOL_CHANGE		(1 << 11)
#define	 PMGR_VOLMAN_DISABLE_GFX_VOL_CHANGE		(1 << 10)
#define	 PMGR_VOLMAN_DISABLE_FIXED_VOL_CHANGE		(1 << 9)
#define	 PMGR_VOLMAN_DISABLE_VAR_SOC_VOL_CHANGE		(1 << 8)

#define rPMGR_EMA_FIXED_GFX0		(*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x26010))
#define rPMGR_EMA_FIXED_GFX1		(*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x26014))

#define rPMGR_CHIP_WDOG_TMR		(*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x27000))
#define rPMGR_CHIP_WDOG_RST_CNT		(*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x27004))
#define rPMGR_CHIP_WDOG_INTR_CNT	(*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x27008))
#define rPMGR_CHIP_WDOG_CTL		(*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x2700c))
#define rPMGR_SYS_WDOG_TMR		(*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x27010))
#define rPMGR_SYS_WDOG_RST_CNT		(*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x27014))
#define rPMGR_SYS_WDOG_CTL		(*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x2701c))

#define rPMGR_SCRATCH(_n)		(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x29000 + ((_n)*4)))
#define rPMGR_SCRATCH0			rPMGR_SCRATCH(0)			/* Flags */
#define rPMGR_SCRATCH1			rPMGR_SCRATCH(1)
#define rPMGR_SCRATCH2			rPMGR_SCRATCH(2)
#define rPMGR_SCRATCH3			rPMGR_SCRATCH(3)
#define rPMGR_SCRATCH4			rPMGR_SCRATCH(4)
#define rPMGR_SCRATCH5			rPMGR_SCRATCH(5)
#define rPMGR_SCRATCH6			rPMGR_SCRATCH(6)
#define rPMGR_SCRATCH7			rPMGR_SCRATCH(7)		/* Consistent debug root pointer */
#define rPMGR_SCRATCH8			rPMGR_SCRATCH(8)
#define rPMGR_SCRATCH9			rPMGR_SCRATCH(9)
#define rPMGR_SCRATCH10			rPMGR_SCRATCH(10)
#define rPMGR_SCRATCH11			rPMGR_SCRATCH(11)
#define rPMGR_SCRATCH12			rPMGR_SCRATCH(12)
#define rPMGR_SCRATCH13			rPMGR_SCRATCH(13)		/* Memory info */
#define rPMGR_SCRATCH14			rPMGR_SCRATCH(14)		/* Boot Nonce 0 */
#define rPMGR_SCRATCH15			rPMGR_SCRATCH(15)		/* Boot Nonce 1 */
#define rPMGR_SCRATCH16			rPMGR_SCRATCH(16)		/* Boot Manifest Hash 0 */
#define rPMGR_SCRATCH17			rPMGR_SCRATCH(17)		/* Boot Manifest Hash 1 */
#define rPMGR_SCRATCH18			rPMGR_SCRATCH(18)		/* Boot Manifest Hash 2 */
#define rPMGR_SCRATCH19			rPMGR_SCRATCH(19)		/* Boot Manifest Hash 3 */
#define rPMGR_SCRATCH20			rPMGR_SCRATCH(20)		/* Boot Manifest Hash 4 */
#define rPMGR_SCRATCH21			rPMGR_SCRATCH(21)
#define rPMGR_SCRATCH22			rPMGR_SCRATCH(22)
#define rPMGR_SCRATCH23			rPMGR_SCRATCH(23)

#define rPMGR_THERMAL0_CTL0_SET		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x2b000))
#define rPMGR_THERMAL0_CTL0_CLR		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x2b004))
#define rPMGR_THERMAL0_CTL1		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x2b008))
#define rPMGR_THERMAL0_CTL2		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x2b00c))
#define rPMGR_THERMAL0_STATUS		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x2b010))
#define rPMGR_THERMAL0_PARAM		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x2b014))
#define rPMGR_THERMAL0_RDBK0		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x2b018))
#define rPMGR_THERMAL0_RDBK1		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x2b01c))
#define rPMGR_THERMAL0_SUM		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x2b020))
#define rPMGR_THERMAL0_SUM_CNT		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x2b024))
#define rPMGR_THERMAL0_PIECE0		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x2b028))
#define rPMGR_THERMAL0_PIECE1		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x2b02c))
#define rPMGR_THERMAL0_PIECE2		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x2b030))
#define rPMGR_THERMAL0_ALARM0		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x2b034))
#define rPMGR_THERMAL0_ALARM1		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x2b038))
#define rPMGR_THERMAL0_ALARM2		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x2b03c))
#define rPMGR_THERMAL0_ALARM3		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x2b040))
#define rPMGR_THERMAL1_CTL0_SET		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x2b044))
#define rPMGR_THERMAL1_CTL0_CLR		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x2b048))
#define rPMGR_THERMAL1_CTL1		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x2b04c))
#define rPMGR_THERMAL1_CTL2		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x2b050))
#define rPMGR_THERMAL1_STATUS		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x2b054))
#define rPMGR_THERMAL1_PARAM		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x2b058))
#define rPMGR_THERMAL1_RDBK0		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x2b05c))
#define rPMGR_THERMAL1_RDBK1		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x2b060))
#define rPMGR_THERMAL1_SUM		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x2b064))
#define rPMGR_THERMAL1_SUM_CNT		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x2b068))
#define rPMGR_THERMAL1_PIECE0		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x2b06c))
#define rPMGR_THERMAL1_PIECE1		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x2b070))
#define rPMGR_THERMAL1_PIECE2		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x2b074))
#define rPMGR_THERMAL1_ALARM0		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x2b078))
#define rPMGR_THERMAL1_ALARM1		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x2b07c))
#define rPMGR_THERMAL1_ALARM2		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x2b080))
#define rPMGR_THERMAL1_ALARM3		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x2b084))

#define  PMGR_THERMAL_CTL0_ENABLE		(1 << 0)
#define  PMGR_THERMAL_PARAM_TRIM_MASK		(0x1F)
#define  PMGR_THERMAL_PARAM_TRIM_SHIFT		(0)
#define  PMGR_THERMAL_PARAM_TRIM(_t)		(((_t) & PMGR_THERMAL_PARAM_TRIM_MASK) << PMGR_THERMAL_PARAM_TRIM_SHIFT)
#define  PMGR_THERMAL_PARAM_SET_TRIM(_reg, _t)	(((_reg) & ~PMGR_THERMAL_PARAM_TRIM(PMGR_THERMAL_PARAM_TRIM_MASK)) | PMGR_THERMAL_PARAM_TRIM(_t))
#define  PMGR_THERMAL_PIECE_START_CODE_MASK	(0x3FF)
#define  PMGR_THERMAL_PIECE_OFFSET_MASK		(0x1FF)
#define  PMGR_THERMAL_PIECE_SLOPE_MASK		(0x3FF)
#define  PMGR_THERMAL_PIECE_START_CODE_SHIFT	(22)
#define  PMGR_THERMAL_PIECE_OFFSET_SHIFT	(10)
#define  PMGR_THERMAL_PIECE_SLOPE_SHIFT		(0)
#define  PMGR_THERMAL_PIECE_START_CODE(_c)	(((_c) & PMGR_THERMAL_PIECE_START_CODE_MASK) << PMGR_THERMAL_PIECE_START_CODE_SHIFT)
#define  PMGR_THERMAL_PIECE_OFFSET(_o)		(((_o) & PMGR_THERMAL_PIECE_OFFSET_MASK) << PMGR_THERMAL_PIECE_OFFSET_SHIFT)
#define  PMGR_THERMAL_PIECE_SLOPE(_s)		(((_s) & PMGR_THERMAL_PIECE_SLOPE_MASK) << PMGR_THERMAL_PIECE_SLOPE_SHIFT)
#define  PMGR_THERMAL_PIECE(_c, _o, _s)		(PMGR_THERMAL_PIECE_START_CODE(_c) | PMGR_THERMAL_PIECE_OFFSET(_o) | PMGR_THERMAL_PIECE_SLOPE(_s))

#define rPMGR_SOCHOT_FAILSAFE_CTL		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x2C000))
#define rPMGR_SOCHOT_FAILSAFE_TRIP_TEMP_0	(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x2C010))
#define rPMGR_SOCHOT_FAILSAFE_TRIP_TEMP_1	(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x2C014))
#define rPMGR_SOCHOT_FAILSAFE_ASSERT_COUNT_0	(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x2C018))
#define rPMGR_SOCHOT_FAILSAFE_ASSERT_COUNT_1	(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x2C01C))
#define rPMGR_SOCHOT_FAILSAFE_DEASSERT_COUNT_0	(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x2C020))
#define rPMGR_SOCHOT_FAILSAFE_DEASSERT_COUNT_1	(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x2C024))
#define rPMGR_SOCHOT_FAILSAFE_SOCHOT0_COUNT_LO	(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x2C080))
#define rPMGR_SOCHOT_FAILSAFE_SOCHOT0_COUNT_HI	(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x2C084))
#define rPMGR_SOCHOT_FAILSAFE_SOCHOT1_COUNT_LO	(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x2C088))
#define rPMGR_SOCHOT_FAILSAFE_SOCHOT1_COUNT_HI	(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x2C08C))

#define rPMGR_TVM_CTL				(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x2D000))
#define rPMGR_TVM_THRESH0			(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x2D004))
#define rPMGR_TVM_THRESH1			(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x2D008))
#define rPMGR_TVM_THRESH2			(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x2D00C))
#define rPMGR_TVM_TEMP0_CFG			(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x2D010))
#define rPMGR_TVM_TEMP1_CFG			(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x2D014))
#define rPMGR_TVM_DEBUG0			(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x2D100))
#define rPMGR_TVM_DEBUG1			(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x2D104))
#define rPMGR_TVM_DEBUG2			(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x2D108))
#define  PMGR_TVM_THRESH(_t)			((_t) & 0x1FF)
#define  PMGR_TVM_TEMP_CFG_MAX_OFFSET(_mo)	(((_mo) & 0x1FF) << 16)
#define  PMGR_TVM_TEMP_CFG_MIN_OFFSET(_mo)	(((_mo) & 0x1FF) << 0)

#define rPMGR_MISC_ACG			(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x32000))

#define rPMGR_MISC_SPARE0		(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x33000))

#if SUB_PLATFORM_T7001
#define rPMGR_DEBUG_PMGR_DEBUG18	(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x40048))
#endif

#if SUB_PLATFORM_T7000
enum {						// clock-id
    PMGR_CLK_OSC = 0,				// 0x00
    PMGR_CLK_PLL0,				// 0x01
    PMGR_CLK_PLL1,				// 0x02
    PMGR_CLK_PLL2,				// 0x03
    PMGR_CLK_PLL3,				// 0x04
    PMGR_CLK_PLL4,				// 0x05
    PMGR_CLK_PLL5,				// 0x06
    PMGR_CLK_CPU,				// 0x07
    PMGR_CLK_MCU_FIXED,				// 0x08
    PMGR_CLK_SOURCE_FIRST = PMGR_CLK_MCU_FIXED,
    PMGR_CLK_MCU,				// 0x09
    PMGR_CLK_MIPI_DSI,				// 0x0A
    PMGR_CLK_NCO_REF0,				// 0x0B
    PMGR_CLK_NCO_REF1,				// 0x0C
    PMGR_CLK_NCO_ALG0,				// 0x0D
    PMGR_CLK_NCO_ALG1,				// 0x0E
    PMGR_CLK_HSICPHY_REF_12M,			// 0x0F
    PMGR_CLK_USB480_0,				// 0x10
    PMGR_CLK_USB480_1,				// 0x11
    PMGR_CLK_USB_OHCI_48M,			// 0x12
    PMGR_CLK_USB,				// 0x13
    PMGR_CLK_USB_FREE_60M,			// 0x14
    PMGR_CLK_SIO_C,				// 0x15
    PMGR_CLK_SIO_P,				// 0x16
    PMGR_CLK_ISP_C,				// 0x17
    PMGR_CLK_ISP,				// 0x18
    PMGR_CLK_ISP_SENSOR0_REF,			// 0x19
    PMGR_CLK_ISP_SENSOR1_REF,			// 0x1A
    PMGR_CLK_VDEC,				// 0x1B
    PMGR_CLK_VENC,				// 0x1C
    PMGR_CLK_VID0,				// 0x1D
    PMGR_CLK_DISP0,				// 0x1E
    PMGR_CLK_DISP1,				// 0x1F
    PMGR_CLK_AJPEG_IP,				// 0x20
    PMGR_CLK_AJPEG_WRAP,			// 0x21
    PMGR_CLK_MSR,				// 0x22
    PMGR_CLK_AF,				// 0x23
    PMGR_CLK_LIO,				// 0x24
    PMGR_CLK_MCA0_M,				// 0x25
    PMGR_CLK_MCA1_M,				// 0x26
    PMGR_CLK_MCA2_M,				// 0x27
    PMGR_CLK_MCA3_M,				// 0x28
    PMGR_CLK_MCA4_M,				// 0x29
    PMGR_CLK_SEP,				// 0x2A
    PMGR_CLK_GPIO,				// 0x2B
    PMGR_CLK_SPI0_N,				// 0x2C
    PMGR_CLK_SPI1_N,				// 0x2D
    PMGR_CLK_SPI2_N,				// 0x2E
    PMGR_CLK_SPI3_N,				// 0x2F
    PMGR_CLK_DEBUG,				// 0x30
    PMGR_CLK_PCIE_REF,				// 0x31
    PMGR_CLK_PCIE_APP,				// 0x32
    PMGR_CLK_TMPS,				// 0x33
    PMGR_CLK_MEDIA_AF,				// 0x34
    PMGR_CLK_ISP_AF,				// 0x35
    PMGR_CLK_GFX_AF,				// 0x36
    PMGR_CLK_ANS_C,				// 0x37
    PMGR_CLK_ANC_LINK,				// 0x38
    PMGR_CLK_SOURCE_LAST = PMGR_CLK_ANC_LINK,
    PMGR_CLK_S0,				// 0x39
    PMGR_CLK_S1,				// 0x3A
    PMGR_CLK_S2,				// 0x3B
    PMGR_CLK_S3,				// 0x3C
    PMGR_CLK_ISP_REF0,				// 0x3D
    PMGR_CLK_ISP_REF1,				// 0x3E
    PMGR_CLK_COUNT,				// 0x3F
    PMGR_CLK_NOT_SUPPORTED
};

#elif SUB_PLATFORM_T7001
enum {						// clock-id
    PMGR_CLK_OSC = 0,				// 0x00
    PMGR_CLK_PLL0,				// 0x01
    PMGR_CLK_PLL1,				// 0x02
    PMGR_CLK_PLL2,				// 0x03
    PMGR_CLK_PLL3,				// 0x04
    PMGR_CLK_PLL4,				// 0x05
    PMGR_CLK_PLL5,				// 0x06
    PMGR_CLK_CPU,				// 0x07
    PMGR_CLK_MCU_FIXED,				// 0x08
    PMGR_CLK_SOURCE_FIRST = PMGR_CLK_MCU_FIXED,
    PMGR_CLK_MCU,				// 0x09
    PMGR_CLK_NCO_REF0,				// 0x0A
    PMGR_CLK_NCO_REF1,				// 0x0B
    PMGR_CLK_NCO_ALG0,				// 0x0C
    PMGR_CLK_NCO_ALG1,				// 0x0D
    PMGR_CLK_HSICPHY_REF_12M,			// 0x0E
    PMGR_CLK_USB480_0,				// 0x0F
    PMGR_CLK_USB480_1,				// 0x10
    PMGR_CLK_USB_OHCI_48M,			// 0x11
    PMGR_CLK_USB,				// 0x12
    PMGR_CLK_USB_FREE_60M,			// 0x13
    PMGR_CLK_SIO_C,				// 0x14
    PMGR_CLK_SIO_P,				// 0x15
    PMGR_CLK_ISP_C,				// 0x16
    PMGR_CLK_ISP,				// 0x17
    PMGR_CLK_ISP_SENSOR0_REF,			// 0x18
    PMGR_CLK_ISP_SENSOR1_REF,			// 0x19
    PMGR_CLK_VDEC,				// 0x1A
    PMGR_CLK_VENC,				// 0x1B
    PMGR_CLK_VID0,				// 0x1C
    PMGR_CLK_DISP0,				// 0x1D
    PMGR_CLK_DISP1,				// 0x1E
    PMGR_CLK_AJPEG_IP,				// 0x1F
    PMGR_CLK_AJPEG_WRAP,			// 0x20
    PMGR_CLK_MSR,				// 0x21
    PMGR_CLK_AF,				// 0x22
    PMGR_CLK_LIO,				// 0x23
    PMGR_CLK_MCA0_M,				// 0x24
    PMGR_CLK_MCA1_M,				// 0x25
    PMGR_CLK_MCA2_M,				// 0x26
    PMGR_CLK_MCA3_M,				// 0x27
    PMGR_CLK_MCA4_M,				// 0x28
    PMGR_CLK_SEP,				// 0x29
    PMGR_CLK_GPIO,				// 0x2A
    PMGR_CLK_SPI0_N,				// 0x2B
    PMGR_CLK_SPI1_N,				// 0x2C
    PMGR_CLK_SPI2_N,				// 0x2D
    PMGR_CLK_SPI3_N,				// 0x2E
    PMGR_CLK_DEBUG,				// 0x2F
    PMGR_CLK_PCIE_APP,				// 0x30
    PMGR_CLK_TMPS,				// 0x31
    PMGR_CLK_MEDIA_AF,				// 0x32
    PMGR_CLK_ISP_AF,				// 0x33
    PMGR_CLK_GFX_AF,				// 0x34
    PMGR_CLK_ANS_C,				// 0x35
    PMGR_CLK_ANC_LINK,				// 0x36
    PMGR_CLK_SOURCE_LAST = PMGR_CLK_ANC_LINK,
    PMGR_CLK_S0,				// 0x37
    PMGR_CLK_S1,				// 0x38
    PMGR_CLK_S2,				// 0x39
    PMGR_CLK_S3,				// 0x3A
    PMGR_CLK_ISP_REF0,				// 0x3B
    PMGR_CLK_ISP_REF1,				// 0x3C
    PMGR_CLK_COUNT,				// 0x3D
    PMGR_CLK_NOT_SUPPORTED
};

#endif

#define PMGR_CLK_CFG_INDEX(_clk) ((_clk) - PMGR_CLK_MCU_FIXED)
////////////////////////////////////////////////////////////
//
//  CCC specific
//
////////////////////////////////////////////////////////////

#define CCC_PWRCTL_BASE			(CCC_ROM_TABLE_BASE_ADDR + 0x220000)
#define CCC_CPM_THERMAL_BASE		(CCC_ROM_TABLE_BASE_ADDR + 0x230000)

#define rCCC_PSW_DLY			(*(volatile uint32_t *)(CCC_PWRCTL_BASE + 0x000))

#define rCCC_PRE_TD_TMR		    (*(volatile uint32_t *)(CCC_PWRCTL_BASE + 0x008))
#define rCCC_PRE_FLUSH_TMR	    (*(volatile uint32_t *)(CCC_PWRCTL_BASE + 0x010))

#define rCCC_APSC_SCR			(*(volatile uint64_t *)(CCC_PWRCTL_BASE + 0x020))
#define CCC_APSC_PENDING		(1 << 31)
#define CCC_APSC_MANUAL_CHANGE(_d)	((1 << 25) | (((_d) & 0x7) << 22))

#define rCCC_DVFM_CFG			(*(volatile uint32_t *)(CCC_PWRCTL_BASE + 0x040))
#define  CCC_DVFM_CFG_TEMPOFFSET1(_to)	(((_to) & 0x7F) << 21)
#define  CCC_DVFM_CFG_TEMPOFFSET0(_to)	(((_to) & 0x7F) << 14)
#define  CCC_DVFM_CFG_TEMPTHRESH1(_tt)	(((_tt) & 0x7F) << 7)
#define  CCC_DVFM_CFG_TEMPTHRESH0(_tt)	(((_tt) & 0x7F) << 0)

#define rCCC_DVFM_SCR			(*(volatile uint64_t *)(CCC_PWRCTL_BASE + 0x048))	// 64-bit
#define  CCC_DVFM_SCR_TEMPSENSORMODE	(1)

#define rCCC_DVFM_CFG_SEL		(*(volatile uint32_t *)(CCC_PWRCTL_BASE + 0x050))

#define rCCC_DVFM_CFG1			(*(volatile uint32_t *)(CCC_PWRCTL_BASE + 0x058))
#define  CCC_DVFM_CFG1_TEMPOFFSET2(_to)	(((_to) & 0x7F) << 7)
#define  CCC_DVFM_CFG1_TEMPTHRESH2(_tt)	(((_tt) & 0x7F) << 0)

#define rCCC_EFUSE_DVFM0		(*(volatile uint32_t *)(CCC_PWRCTL_BASE + 0x0A8))
#define rCCC_EFUSE_REV			(*(volatile uint32_t *)(CCC_PWRCTL_BASE + 0x0B0))

#define rCCC_PLL_DLY_CFG0 		(*(volatile uint32_t *)(CCC_PWRCTL_BASE + 0x0E0))
#define rCCC_PLL_DLY_CFG1		(*(volatile uint32_t *)(CCC_PWRCTL_BASE + 0x0E8))
#define rCCC_PLL_CFG0			(*(volatile uint32_t *)(CCC_PWRCTL_BASE + 0x0D0))		

#define rCCC_PLL_CFG1		    (*(volatile uint32_t *)(CCC_PWRCTL_BASE + 0x0D8))
#define CCC_PLL_CFG1_PLL_VCO_RCTRL_SEL	(1 << 11)
#define CCC_PLL_CFG1_PLL_VCO_RCTRL_OW_SHIFT	(14)
#define CCC_PLL_CFG1_PLL_VCO_RCTRL_OW_MASK	(0x7)

#define rCCC_PLL_CFG2		    (*(volatile uint32_t *)(CCC_PWRCTL_BASE + 0x0F8))
#define CCC_PLL_CFG2_RELOCKBYS2_S4_MASK	(0x6)
#define CCC_PLL_CFG2_RELOCKBYS2_S4_DIV2	(0x6)

#define rCCC_EFUSE_DVFM1		(*(volatile uint32_t *)(CCC_PWRCTL_BASE + 0x100))

#define kDVFM_STATE_BYPASS		(0)
#define kDVFM_STATE_SECUREROM		(kDVFM_STATE_BYPASS + 1)
#define kDVFM_STATE_IBOOT		(kDVFM_STATE_BYPASS + 2)
#define kDVFM_STATE_V0			(kDVFM_STATE_IBOOT + 0)
#define kDVFM_STATE_V1			(kDVFM_STATE_IBOOT + 1)
#define kDVFM_STATE_V2			(kDVFM_STATE_IBOOT + 2)
#define kDVFM_STATE_V3			(kDVFM_STATE_IBOOT + 3)
#if !SUB_TARGET_N102
#define kDVFM_STATE_V4			(kDVFM_STATE_IBOOT + 4)
#endif
#if SUB_TARGET_N102
#define kDVFM_STATE_VMAX                (kDVFM_STATE_V3)
#elif SUB_PLATFORM_T7000 && !SUB_TARGET_J96 && !SUB_TARGET_J97
#define kDVFM_STATE_VMAX		(kDVFM_STATE_V4)
#else
#define kDVFM_STATE_V5			(kDVFM_STATE_IBOOT + 5)
#define kDVFM_STATE_V6			(kDVFM_STATE_IBOOT + 6)
#define kDVFM_STATE_V6_UNBINNED		(kDVFM_STATE_IBOOT + 7)
#define kDVFM_STATE_VMAX		(kDVFM_STATE_V5)
#endif

#define kDVFM_STATE_VNOM		(kDVFM_STATE_V2)

#define kDVFM_STATE_VBOOST		(kDVFM_STATE_VMAX)

#define kDVFM_STATE_IBOOT_CNT		(kDVFM_STATE_VMAX + 1)

#define kVOLTAGE_STATES1_COUNT      	(kDVFM_STATE_VMAX - kDVFM_STATE_V0 + 1)
#define kVOLTAGE_STATES1_SIZE       (kVOLTAGE_STATES1_COUNT*2)  // Minimum size of voltage-state1 property in devicetree

#define CCC_DVFM_STATE_COUNT		(8)
#define rCCC_DVFM_ST(_n)		(*(volatile uint64_t *)(CCC_PWRCTL_BASE + 0x068 + ((_n) * 8)))
#if SUB_PLATFORM_T7001
#define CCC_DVFM_SRAM_STATE(_int,_state,_vid) (_int |  (_vid & 0xffULL) << (8 * _state))
#define rCCC_DVFM_SRAM	(*(volatile uint64_t *)(CCC_PWRCTL_BASE + 0x110))
#endif

#define  CCC_DVFM_ST_TO_SAFEVOL(_st)	(((uint64_t)(_st) >> 56) & 0xff)
#define  CCC_DVFM_ST_SAFEVOL(_sv)	(((uint64_t)(_sv) & 0xFF) << 56)
#define  CCC_DVFM_ST_VOLADJ3(_va)	(((uint64_t)(_va) & 0x3F) << 48)
#define  CCC_DVFM_ST_VOLADJ2(_va)	(((uint64_t)(_va) & 0x3F) << 42)
#define  CCC_DVFM_ST_VOLADJ1(_va)	(((uint64_t)(_va) & 0x3F) << 36)
#define  CCC_DVFM_ST_VOLADJ0(_va)	(((uint64_t)(_va) & 0x3F) << 30)
#define  CCC_DVFM_ST_PLL_P(_p)		(((_p) & 0x1F) << 13)
#define  CCC_DVFM_ST_PLL_M(_m)		(((_m) & 0x1FF) << 4)
#define  CCC_DVFM_ST_PLL_S(_s)		(((_s) & 0xF) << 0)

#define rCCC_PWRCTRL_MSTR_PLLCTL	(*(volatile uint32_t *)(CCC_PWRCTL_BASE + 0x0B8))

#define rCCC_PWRCTRL_PLL_SCR0		(*(volatile uint32_t *)(CCC_PWRCTL_BASE + 0x0C0))

#define rCCC_PWRCTRL_PLL_SCR1		(*(volatile uint32_t *)(CCC_PWRCTL_BASE + 0x0C8))

#define rCCC_IEX_NRG_WGHT		(*(volatile uint64_t *)(CCC_PWRCTL_BASE + 0x240))
#define rCCC_LSU_NRG_WGHT		(*(volatile uint64_t *)(CCC_PWRCTL_BASE + 0x250))
#define rCCC_DCD_NRG_WGHT		(*(volatile uint64_t *)(CCC_PWRCTL_BASE + 0x258))
#define rCCC_NEX_NRG_WGHT0		(*(volatile uint64_t *)(CCC_PWRCTL_BASE + 0x260))
#define rCCC_NEX_NRG_WGHT1		(*(volatile uint64_t *)(CCC_PWRCTL_BASE + 0x268))
#define rCCC_NEX_NRG_WGHT2		(*(volatile uint64_t *)(CCC_PWRCTL_BASE + 0x270))

#define rCCC_THRM0_CTL0_SET		(*(volatile uint32_t*)(CCC_CPM_THERMAL_BASE + 0x000))
#define rCCC_THRM0_CTL1			(*(volatile uint32_t*)(CCC_CPM_THERMAL_BASE + 0x008))
#define rCCC_THRM0_CTL2			(*(volatile uint32_t*)(CCC_CPM_THERMAL_BASE + 0x010))
#define rCCC_THRM0_PARAM		(*(volatile uint32_t*)(CCC_CPM_THERMAL_BASE + 0x018))
#define rCCC_THRM0_RD_BK0		(*(volatile uint32_t*)(CCC_CPM_THERMAL_BASE + 0x020))
#define rCCC_THRM0_RD_BK1		(*(volatile uint32_t*)(CCC_CPM_THERMAL_BASE + 0x028))
#define rCCC_THRM0_SUM			(*(volatile uint32_t*)(CCC_CPM_THERMAL_BASE + 0x030))
#define rCCC_THRM0_SUM_CNT		(*(volatile uint32_t*)(CCC_CPM_THERMAL_BASE + 0x038))
#define rCCC_THRM1_CTL0_SET		(*(volatile uint32_t*)(CCC_CPM_THERMAL_BASE + 0x040))
#define rCCC_THRM1_CTL1			(*(volatile uint32_t*)(CCC_CPM_THERMAL_BASE + 0x048))
#define rCCC_THRM1_CTL2			(*(volatile uint32_t*)(CCC_CPM_THERMAL_BASE + 0x050))
#define rCCC_THRM1_PARAM		(*(volatile uint32_t*)(CCC_CPM_THERMAL_BASE + 0x058))
#define rCCC_THRM1_RD_BK0		(*(volatile uint32_t*)(CCC_CPM_THERMAL_BASE + 0x060))
#define rCCC_THRM1_RD_BK1		(*(volatile uint32_t*)(CCC_CPM_THERMAL_BASE + 0x068))
#define rCCC_THRM1_SUM			(*(volatile uint32_t*)(CCC_CPM_THERMAL_BASE + 0x070))
#define rCCC_THRM1_SUM_CNT		(*(volatile uint32_t*)(CCC_CPM_THERMAL_BASE + 0x078))
#define rCCC_THRM_FSCTL			(*(volatile uint32_t*)(CCC_CPM_THERMAL_BASE + 0x080))
#define rCCC_THRM_FSTRIP_T0		(*(volatile uint32_t*)(CCC_CPM_THERMAL_BASE + 0x090))
#define rCCC_THRM_FSTRIP_T1		(*(volatile uint32_t*)(CCC_CPM_THERMAL_BASE + 0x098))
#define rCCC_THRM_FSASRT_CNT0		(*(volatile uint32_t*)(CCC_CPM_THERMAL_BASE + 0x0a0))
#define rCCC_THRM_FSASRT_CNT1		(*(volatile uint32_t*)(CCC_CPM_THERMAL_BASE + 0x0a8))
#define rCCC_THRM_FSDE_ASRT_CNT0	(*(volatile uint32_t*)(CCC_CPM_THERMAL_BASE + 0x0b0))
#define rCCC_THRM_FSDE_ASRT_CNT1	(*(volatile uint32_t*)(CCC_CPM_THERMAL_BASE + 0x0b8))
#define rCCC_THRM_FSHOT0_TSTAMP		(*(volatile uint64_t*)(CCC_CPM_THERMAL_BASE + 0x0c0))	// 64-bit
#define rCCC_THRM_FSHOT1_TSTAMP		(*(volatile uint64_t*)(CCC_CPM_THERMAL_BASE + 0x0c8))	// 64-bit
#define rCCC_DVFM_FSHOT_IDX		(*(volatile uint32_t*)(CCC_CPM_THERMAL_BASE + 0x0f0))
#define rCCC_THRM0_PIECE0		(*(volatile uint32_t*)(CCC_CPM_THERMAL_BASE + 0x100))
#define rCCC_THRM0_PIECE1		(*(volatile uint32_t*)(CCC_CPM_THERMAL_BASE + 0x108))
#define rCCC_THRM0_PIECE2		(*(volatile uint32_t*)(CCC_CPM_THERMAL_BASE + 0x110))
#define rCCC_THRM1_PIECE0		(*(volatile uint32_t*)(CCC_CPM_THERMAL_BASE + 0x118))
#define rCCC_THRM1_PIECE1		(*(volatile uint32_t*)(CCC_CPM_THERMAL_BASE + 0x120))
#define rCCC_THRM1_PIECE2		(*(volatile uint32_t*)(CCC_CPM_THERMAL_BASE + 0x128))
#define rCCC_THRM2_PIECE0		(*(volatile uint32_t*)(CCC_CPM_THERMAL_BASE + 0x130))
#define rCCC_THRM2_PIECE1		(*(volatile uint32_t*)(CCC_CPM_THERMAL_BASE + 0x138))
#define rCCC_THRM2_PIECE2		(*(volatile uint32_t*)(CCC_CPM_THERMAL_BASE + 0x140))
#if SUB_PLATFORM_T7001
#define rCCC_THRM3_PIECE0		(*(volatile uint32_t*)(CCC_CPM_THERMAL_BASE + 0x150))
#define rCCC_THRM3_PIECE1		(*(volatile uint32_t*)(CCC_CPM_THERMAL_BASE + 0x158))
#define rCCC_THRM3_PIECE2		(*(volatile uint32_t*)(CCC_CPM_THERMAL_BASE + 0x160))
#endif
#define rCCC_THRM2_CTL0_SET		(*(volatile uint32_t*)(CCC_CPM_THERMAL_BASE + 0x200))
#define rCCC_THRM2_CTL1			(*(volatile uint32_t*)(CCC_CPM_THERMAL_BASE + 0x208))
#define rCCC_THRM2_CTL2			(*(volatile uint32_t*)(CCC_CPM_THERMAL_BASE + 0x210))
#define rCCC_THRM2_PARAM		(*(volatile uint32_t*)(CCC_CPM_THERMAL_BASE + 0x218))
#define rCCC_THRM2_RD_BK0		(*(volatile uint32_t*)(CCC_CPM_THERMAL_BASE + 0x220))
#define rCCC_THRM2_RD_BK1		(*(volatile uint32_t*)(CCC_CPM_THERMAL_BASE + 0x228))
#define rCCC_THRM2_SUM			(*(volatile uint32_t*)(CCC_CPM_THERMAL_BASE + 0x230))
#define rCCC_THRM2_SUM_CNT		(*(volatile uint32_t*)(CCC_CPM_THERMAL_BASE + 0x238))
#define rCCC_THRM0_CTL0_STAT		(*(volatile uint32_t*)(CCC_CPM_THERMAL_BASE + 0x400))
#define rCCC_THRM0_CTL0_CLR		(*(volatile uint32_t*)(CCC_CPM_THERMAL_BASE + 0x408))
#define rCCC_THRM0_ALARM0		(*(volatile uint32_t*)(CCC_CPM_THERMAL_BASE + 0x410))
#define rCCC_THRM0_ALARM1		(*(volatile uint32_t*)(CCC_CPM_THERMAL_BASE + 0x418))
#define rCCC_THRM0_ALARM2		(*(volatile uint32_t*)(CCC_CPM_THERMAL_BASE + 0x420))
#define rCCC_THRM0_ALARM3		(*(volatile uint32_t*)(CCC_CPM_THERMAL_BASE + 0x428))
#define rCCC_THRM1_CTL0_STAT		(*(volatile uint32_t*)(CCC_CPM_THERMAL_BASE + 0x430))
#define rCCC_THRM1_CTL0_CLR		(*(volatile uint32_t*)(CCC_CPM_THERMAL_BASE + 0x438))
#define rCCC_THRM1_ALARM0		(*(volatile uint32_t*)(CCC_CPM_THERMAL_BASE + 0x440))
#define rCCC_THRM1_ALARM1		(*(volatile uint32_t*)(CCC_CPM_THERMAL_BASE + 0x448))
#define rCCC_THRM1_ALARM2		(*(volatile uint32_t*)(CCC_CPM_THERMAL_BASE + 0x450))
#define rCCC_THRM1_ALARM3		(*(volatile uint32_t*)(CCC_CPM_THERMAL_BASE + 0x458))
#define rCCC_THRM2_CTL0_STAT		(*(volatile uint32_t*)(CCC_CPM_THERMAL_BASE + 0x460))
#define rCCC_THRM2_CTL0_CLR		(*(volatile uint32_t*)(CCC_CPM_THERMAL_BASE + 0x468))
#define rCCC_THRM2_ALARM0		(*(volatile uint32_t*)(CCC_CPM_THERMAL_BASE + 0x470))
#define rCCC_THRM2_ALARM1		(*(volatile uint32_t*)(CCC_CPM_THERMAL_BASE + 0x478))
#define rCCC_THRM2_ALARM2		(*(volatile uint32_t*)(CCC_CPM_THERMAL_BASE + 0x480))
#define rCCC_THRM2_ALARM3		(*(volatile uint32_t*)(CCC_CPM_THERMAL_BASE + 0x488))
#if SUB_PLATFORM_T7001
#define rCCC_THRM3_CTL0_SET		(*(volatile uint32_t*)(CCC_CPM_THERMAL_BASE + 0x500))
#define rCCC_THRM3_CTL1			(*(volatile uint32_t*)(CCC_CPM_THERMAL_BASE + 0x508))
#define rCCC_THRM3_CTL2			(*(volatile uint32_t*)(CCC_CPM_THERMAL_BASE + 0x510))
#define rCCC_THRM3_PARAM		(*(volatile uint32_t*)(CCC_CPM_THERMAL_BASE + 0x518))
#define rCCC_THRM3_RD_BK0		(*(volatile uint32_t*)(CCC_CPM_THERMAL_BASE + 0x520))
#define rCCC_THRM3_RD_BK1		(*(volatile uint32_t*)(CCC_CPM_THERMAL_BASE + 0x528))
#define rCCC_THRM3_SUM			(*(volatile uint32_t*)(CCC_CPM_THERMAL_BASE + 0x530))
#define rCCC_THRM3_SUM_CNT		(*(volatile uint32_t*)(CCC_CPM_THERMAL_BASE + 0x538))
#define rCCC_THRM3_CTL0_STAT		(*(volatile uint32_t*)(CCC_CPM_THERMAL_BASE + 0x760))
#define rCCC_THRM3_CTL0_CLR		(*(volatile uint32_t*)(CCC_CPM_THERMAL_BASE + 0x768))
#define rCCC_THRM3_ALARM0		(*(volatile uint32_t*)(CCC_CPM_THERMAL_BASE + 0x770))
#define rCCC_THRM3_ALARM1		(*(volatile uint32_t*)(CCC_CPM_THERMAL_BASE + 0x778))
#define rCCC_THRM3_ALARM2		(*(volatile uint32_t*)(CCC_CPM_THERMAL_BASE + 0x780))
#define rCCC_THRM3_ALARM3		(*(volatile uint32_t*)(CCC_CPM_THERMAL_BASE + 0x788))
#endif

#define  CCC_THRM_CTL0_ENABLE			(1 << 0)
#define  CCC_THRM_PIECE_START_CODE_MASK		PMGR_THERMAL_PIECE_START_CODE_MASK
#define  CCC_THRM_PIECE_OFFSET_MASK		(0x0FF)
#define  CCC_THRM_PIECE_SLOPE_MASK		PMGR_THERMAL_PIECE_SLOPE_MASK
#define  CCC_THRM_PIECE_START_CODE_SHIFT	PMGR_THERMAL_PIECE_START_CODE_SHIFT
#define  CCC_THRM_PIECE_OFFSET_SHIFT		PMGR_THERMAL_PIECE_OFFSET_SHIFT
#define  CCC_THRM_PIECE_SLOPE_SHIFT		PMGR_THERMAL_PIECE_SLOPE_SHIFT
#define  CCC_THRM_PIECE_START_CODE(_c)		(((_c) & CCC_THRM_PIECE_START_CODE_MASK) << CCC_THRM_PIECE_START_CODE_SHIFT)
#define  CCC_THRM_PIECE_OFFSET(_o)		(((_o) & CCC_THRM_PIECE_OFFSET_MASK) << CCC_THRM_PIECE_OFFSET_SHIFT)
#define  CCC_THRM_PIECE_SLOPE(_s)		(((_s) & CCC_THRM_PIECE_SLOPE_MASK) << CCC_THRM_PIECE_SLOPE_SHIFT)
#define  CCC_THRM_PIECE(_c, _o, _s)		(CCC_THRM_PIECE_START_CODE(_c) | CCC_THRM_PIECE_OFFSET(_o) | CCC_THRM_PIECE_SLOPE(_s))
#define  CCC_DFVM_FSHOT_IDX_THERMAL1(_n)        (((_n) & 0x7) << 16)
#define  CCC_DFVM_FSHOT_IDX_THERMAL0(_n)        (((_n) & 0x7) << 0)
#define  CCC_DFVM_FSHOT_IDX_THERMAL1_STATE      ((rCCC_DVFM_FSHOT_IDX >> 16) & 0x7)
#define  CCC_DFVM_FSHOT_IDX_THERMAL0_STATE      ((rCCC_DVFM_FSHOT_IDX >> 0) & 0x7)

#define  CCC_PLL_P_SHIFT		(9)
#define  CCC_PLL_P_MASK			(0x1F)
#define  CCC_PLL_M_SHIFT		(18)
#define  CCC_PLL_M_MASK			(0x1FF)
#define  CCC_PLL_S_SHIFT		(0)
#define  CCC_PLL_S_MASK			(0xF)
#define  CCC_PLL_M(_m)			(((_m) & CCC_PLL_M_MASK) << CCC_PLL_M_SHIFT)
#define  CCC_PLL_P(_p)			(((_p) & CCC_PLL_P_MASK) << CCC_PLL_P_SHIFT)
#define  CCC_PLL_S(_s)			(((_s) & CCC_PLL_S_MASK) << CCC_PLL_S_SHIFT)
#define  CCC_PLL_ENABLE			(1 << 31)
#define  CCC_PLL_LOCKED			(1 << 29)
#define  CCC_PLL_LOAD			(1 << 27)


////////////////////////////////////////////////////////////
//
//  Externs
//
////////////////////////////////////////////////////////////

extern void pmgr_update_device_tree(DTNode *pmgr_node);
extern void pmgr_gfx_update_device_tree(DTNode *gfx_node);
void sochot_pmgr_update_device_tree(DTNode *node);

void pmgr_update_dvfm(uint32_t board_id, uint32_t board_rev);
void pmgr_update_gfx_states(uint32_t board_id, uint32_t board_rev);
bool pmgr_check_gpu_use650MHz_binned(uint32_t board_id, uint32_t board_rev);
bool pmgr_check_gpu_use650MHz_unbinned(uint32_t board_id, uint32_t board_rev);

#endif /* ! __PLATFORM_SOC_PMGR_H */
