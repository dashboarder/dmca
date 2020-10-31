/*
 * Copyright (C) 2010-2012 Apple Inc. All rights reserved.
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

#define PMGR_PLL_COUNT			(8)

#define	rPMGR_PLL_CTL0(_n)		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x0000 + ((_n) * 0x18)))
#define	 PMGR_PLL_ENABLE		(1 << 31)
#define	 PMGR_PLL_EXT_BYPASS		(1 << 30)
#define	 PMGR_PLL_REAL_LOCK		(1 << 29)
#define	 PMGR_PLL_LOAD			(1 << 27)
#define	 PMGR_PLL_BYPASS		(1 << 23)

#define	rPMGR_PLL_CTL1(_n)		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x0004 + ((_n) * 0x18)))
#define  PMGR_PLL_MASK			(0x1FF)
#define  PMGR_PLL_M_SHIFT		(18)
#define  PMGR_PLL_P_SHIFT		(9)
#define  PMGR_PLL_S_SHIFT		(0)
#define	 PMGR_PLL_P(_p)			(((_p) & PMGR_PLL_MASK) << PMGR_PLL_P_SHIFT)
#define	 PMGR_PLL_M(_m)			(((_m) & PMGR_PLL_MASK) << PMGR_PLL_M_SHIFT)
#define	 PMGR_PLL_S(_s)			(((_s) & PMGR_PLL_MASK) << PMGR_PLL_S_SHIFT)

#define	rPMGR_PLL_PARAM(_n)		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x0008 + ((_n) * 0x18)))
#define  PMGR_PARAM_LOCK_TIME_MASK	(0xFFFF)
#define  PMGR_PARAM_LOCK_TIME(_n)	(((_n) & PMGR_PARAM_LOCK_TIME_MASK) << 0)

#define	rPMGR_PLL_FD(_n)		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x000C + ((_n) * 0x18)))

#define	rPMGR_DOUBLER_CTL		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x0100))
#define	 PMGR_DOUBLER_ENABLE		(1 << 31)
#define	 PMGR_DOUBLER_EXT_BYPASS	(1 << 30)

#define	rPMGR_CPU_CLK_CFG		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x0140))
#define	rPMGR_PLL2_GATE_CFG		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x0150))
#define	rPMGR_PLL4_GATE_CFG		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x0154))
#define	rPMGR_PLL5_GATE_CFG		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x0158))

#define	rPMGR_MCU_FIXED_CLK_CFG		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x0164))
#define	rPMGR_MCU_CLK_CFG		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x0168))
#define	rPMGR_PCLK1_CLK_CFG		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x016C))
#define	rPMGR_GFX_CLK_CFG		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x0170))

#define	rPMGR_PREDIV0_CLK_CFG		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x0190))
#define	rPMGR_PREDIV1_CLK_CFG		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x0194))
#define	rPMGR_PREDIV2_CLK_CFG		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x0198))
#define	rPMGR_PREDIV3_CLK_CFG		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x019C))
#define	rPMGR_PREDIV4_CLK_CFG		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x01A0))
#define	rPMGR_PREDIV5_CLK_CFG		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x01A4))

#define	rPMGR_VENC_MTX_CLK_CFG		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x01D0))
#define	rPMGR_VENC_CLK_CFG		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x01D4))
#define	rPMGR_HPERFRT_CLK_CFG		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x01D8))
#define	rPMGR_GFX_SYS_CLK_CFG		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x01DC))
#define	rPMGR_HPERFNRT_CLK_CFG		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x01E0))
#define	rPMGR_NRT_MEM_CLK_CFG		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x01E4))
#define	rPMGR_VDEC_CLK_CFG		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x01E8))
#define	rPMGR_ISP_CLK_CFG		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x01EC))
#define	rPMGR_IOP_CLK_CFG		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x01F0))
#define	rPMGR_CDIO_CLK_CFG		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x01F4))
#define	rPMGR_LPERFS_CLK_CFG		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x01F8))
#define	rPMGR_PCLK0_CLK_CFG		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x01FC))
#define	rPMGR_PCLK2_CLK_CFG		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x0200))
#define	rPMGR_PCLK3_CLK_CFG		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x0204))
#define	rPMGR_AES_CLK_CFG		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x0208))

#define	rPMGR_MEDIUM0_CLK_CFG		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x0230))
#define	rPMGR_MEDIUM1_CLK_CFG		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x0234))
#define	rPMGR_VID0_CLK_CFG		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x0238))
#define	rPMGR_VID1_CLK_CFG		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x023C))
#define	rPMGR_DISPOUT_CLK_CFG		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x0240))
#define	rPMGR_I2C_CLK_CFG		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x0244))
#define	rPMGR_SDIO_CLK_CFG		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x0248))

#define	rPMGR_AUDIO_CLK_CFG		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x0250))

#define	rPMGR_UPERF_CLK_CFG		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x0260))
#define	rPMGR_DEBUG_CLK_CFG		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x0264))

#define	rPMGR_SCC_PWR_CLK_CFG		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x0268))
#define	rPMGR_SCC_DMA_CLK_CFG		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x026C))

#define	rPMGR_SPI0_CLK_CFG		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x0270))
#define	rPMGR_SPI1_CLK_CFG		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x0274))
#define	rPMGR_SPI2_CLK_CFG		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x0278))
#define	rPMGR_SPI3_CLK_CFG		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x027C))
#define	rPMGR_SPI4_CLK_CFG		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x0280))
#define	rPMGR_SLOW_CLK_CFG		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x0284))
#define	rPMGR_SLEEP_CLK_CFG		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x0288))

#define	rPMGR_USB_PHY0_CLK_CFG		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x028C))
#define	rPMGR_USBOHCI_CLK_CFG		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x0290))
#define	rPMGR_USB12_CLK_CFG		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x0294))

#define	rPMGR_NCO_REF0_CLK_CFG		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x0298))
#define	rPMGR_NCO_REF1_CLK_CFG		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x029C))
#define	rPMGR_NCO_REF2_CLK_CFG		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x02A0))
#define	rPMGR_USB_PHY1_CLK_CFG		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x02A4))
#define	rPMGR_USB_EHCI_FREE_CLK_CFG	(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x02A8))

#define	PMGR_FIRST_CLK_CFG		(&rPMGR_CPU_CLK_CFG)
#define	PMGR_LAST_CLK_CFG		(&rPMGR_USB_EHCI_FREE_CLK_CFG)
#define	PMGR_CLK_CFG_COUNT		(((((u_int32_t)(PMGR_LAST_CLK_CFG)) - ((u_int32_t)(PMGR_FIRST_CLK_CFG))) / 4) + 1)
#define	PMGR_CLK_NUM(_r)		(((u_int32_t)&(rPMGR_ ## _r ## _CLK_CFG) - ((u_int32_t)(PMGR_FIRST_CLK_CFG))) / 4)
#define	PMGR_FIRST_MANAGED_CLK_NUM	PMGR_CLK_NUM(VENC_MTX)

#define	 PMGR_CLK_CFG_ENABLE		(1 << 31)
#define	 PMGR_CLK_CFG_PENDING		(1 << 30)
#define	 PMGR_CLK_CFG_AUTO_DISABLE	(1 << 25)
#define	 PMGR_CLK_CFG_SRC(_d)		(((_d) & 0x3) << 28)
#define	 PMGR_CLK_CFG_SRC_MASK		PMGR_CLK_CFG_SRC(0x3)
#define	 PMGR_CLK_CFG_DIVIDER(_d)	(((_d) & 0x1F) << 0)
#define	 PMGR_CLK_CFG_DIV_MASK		PMGR_CLK_CFG_DIVIDER(0x1F)

#define	 PMGR_PREDIV4_SRC__OSC		PMGR_CLK_CFG_SRC(0x0)
#define	 PMGR_PREDIV4_SRC__PLL5		PMGR_CLK_CFG_SRC(0x1)
#define	 PMGR_PREDIV4_SRC__PLL6		PMGR_CLK_CFG_SRC(0x2)
#define	 PMGR_PREDIV4_SRC__PLL3		PMGR_CLK_CFG_SRC(0x3)

#define	rPMGR_I2S0_NCO_CLK_CFG		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x02B0))
#define	rPMGR_I2S0_NCO_N1		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x02B4))
#define	rPMGR_I2S0_NCO_N2		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x02B8))

#define	rPMGR_I2S1_NCO_CLK_CFG		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x02C0))
#define	rPMGR_I2S1_NCO_N1		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x02C4))
#define	rPMGR_I2S1_NCO_N2		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x02C8))

#define	rPMGR_I2S2_NCO_CLK_CFG		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x02D0))
#define	rPMGR_I2S2_NCO_N1		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x02D4))
#define	rPMGR_I2S2_NCO_N2		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x02D8))

#define	rPMGR_I2S3_NCO_CLK_CFG		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x02E0))
#define	rPMGR_I2S3_NCO_N1		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x02E4))
#define	rPMGR_I2S3_NCO_N2		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x02E8))

#define	rPMGR_SPDIF_NCO_CLK_CFG		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x02F0))
#define	rPMGR_SPDIF_NCO_N1		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x02F4))
#define	rPMGR_SPDIF_NCO_N2		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x02F8))

#define	rPMGR_DP_NCO_CLK_CFG		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x0300))
#define	rPMGR_DP_NCO_N1			(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x0304))
#define	rPMGR_DP_NCO_N2			(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x0308))

#define	rPMGR_MCA_NCO_CLK_CFG		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x0310))
#define	rPMGR_MCA_NCO_N1		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x0314))
#define	rPMGR_MCA_NCO_N2		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x0318))

#define rPMGR_ENABLE_CLK_GATE		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x0330))

#define	kPERF_STATE_BYPASS		(0)
#define	kPERF_STATE_SECUREROM		(1)
#define	kPERF_STATE_IBOOT		(2)
#define	kPERF_STATE_IBOOT_CNT		(5)

#define	PMGR_PERF_STATE_COUNT		(16)
#define	rPMGR_PERF_STATE_A(_s)		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x0400 + ((_s) * 0x20)))
#define	rPMGR_PERF_STATE_B(_s)		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x0404 + ((_s) * 0x20)))
#define	rPMGR_PERF_STATE_C(_s)		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x0408 + ((_s) * 0x20)))
#define	rPMGR_PERF_STATE_D(_s)		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x040C + ((_s) * 0x20)))
#define	rPMGR_PERF_STATE_E(_s)		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x0410 + ((_s) * 0x20)))
#define  PMGR_PERF_STATE_E_VOL_SHIFT	(0)
#define  PMGR_PERF_STATE_E_VOL_MASK	(0xFF)

#define	rPMGR_PERF_STATE_CTL		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x0600))
#define	 PMGR_PERF_STATE_SEL_SHIFT	(0)
#define	 PMGR_PERF_STATE_SEL_MASK	(0xF)
#define	 PMGR_PERF_STATE_SEL(_s)	(((_s) & PMGR_PERF_STATE_SEL_MASK) << PMGR_PERF_STATE_SEL_SHIFT)
#define	 PMGR_PERF_STATE_PENDING	(0xFFFFFFF << 4)

#define	rPMGR_PERF_STATE_DELAY		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x0604))
#define  PMGR_PERF_STATE_DELAY_VOL_CHG_FIX_DLY_SHIFT (8)
#define  PMGR_PERF_STATE_DELAY_VOL_CHG_VAR_DLY_SHIFT (0)

#define	rPMGR_MCU_CLK_DEBUG		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x0620))

#define	rPMGR_AUTOMATIC_CLK_GATE	(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x0640))
#define  PMGR_ACG_CLK_GEN__ENABLE	(1 << 0)
#define  PMGR_ACG_PS_ENABLE		(1 << 1)
#define  PMGR_ACG_NCO_CLK_GEN_ENABLE	(1 << 2)

#define rPMGR_FIRST_PS			(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x1000))
#define	rPMGR_SCC_PS			(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x1004))
#define	rPMGR_CPU0_PS			(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x1008))
#define	rPMGR_CPU1_PS			(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x100C))
#define	rPMGR_L2_BIU_PS			(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x1010))
#define	rPMGR_L2_PS			(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x1014))
#define	rPMGR_MCU_PS			(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x1020))
#define	rPMGR_GFX_SYS_PS		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x1024))
#define	rPMGR_GFX_CORES_PS		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x1028))
#define	rPMGR_HPERFNRT_PS		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x102C))
#define	rPMGR_VDEC_PS			(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x1030))
#define	rPMGR_SCALER0_PS		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x1034))
#define	rPMGR_SCALER1_PS		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x1038))
#define	rPMGR_JPG0_PS			(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x103C))
#define	rPMGR_JPG1_PS			(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x1040))
#define	rPMGR_VENCD_PS			(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x1048))
#define	rPMGR_HPERFRT_PS		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x104C))
#define	rPMGR_ISP_PS			(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x1050))
#define	rPMGR_EDP_PS			(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x1054))
#define	rPMGR_DISP0_PS			(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x1058))
#define	rPMGR_DISP1_PS			(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x105C))
#define	rPMGR_DISPOUT_PS		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x1060))

#define	rPMGR_CLCD_PS			(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x1068))
#define	rPMGR_TVOUT_PS			(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x106C))
#define	rPMGR_RGBOUT_PS			(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x1070))
#define	rPMGR_DPLINK_PS			(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x1074))
#define	rPMGR_CDIO_PS			(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x1078))
#define	rPMGR_CDMA_PS			(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x107C))
#define	rPMGR_IOP_PS			(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x1080))
#define	rPMGR_UPERF_PS			(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x1084))
#define	rPMGR_USBOTG0_PS		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x1088))
#define	rPMGR_USB2HOST0_PS		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x108C))
#define	rPMGR_USB11HOST_PS		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x1090))
#define	rPMGR_USB2HOST0_OHCI_PS		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x1094))
#define	rPMGR_USB11HOST_OHCI_PS		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x1098))
#define	rPMGR_USBREG_PS			(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x109C))
#define	rPMGR_AUDIO_PS			(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x10A0))
#define	rPMGR_I2S0_PS			(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x10A4))
#define	rPMGR_I2S1_PS			(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x10A8))
#define	rPMGR_I2S2_PS			(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x10AC))
#define	rPMGR_I2S3_PS			(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x10B0))
#define	rPMGR_SPDIF_PS			(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x10B4))

#define	rPMGR_SDIO_PS			(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x10BC))
#define	rPMGR_SHA1_PS			(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x10C0))
#define	rPMGR_SHA2_PS			(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x10C4))
#define	rPMGR_FMI0_PS			(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x10C8))
#define	rPMGR_FMI0BCH_PS		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x10CC))
#define	rPMGR_FMI1_PS			(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x10D0))
#define	rPMGR_FMI1BCH_PS		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x10D4))
#define	rPMGR_FMI_DLL_PS		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x10D8))

#define	rPMGR_MCA_PS			(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x10E0))
#define	rPMGR_SPI0_PS			(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x10E4))
#define	rPMGR_SPI1_PS			(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x10E8))
#define	rPMGR_SPI2_PS			(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x10EC))
#define	rPMGR_SPI3_PS			(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x10F0))
#define	rPMGR_SPI4_PS			(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x10F4))
#define	rPMGR_UART0_PS			(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x10F8))
#define	rPMGR_UART1_PS			(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x10FC))
#define	rPMGR_UART2_PS			(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x1100))
#define	rPMGR_UART3_PS			(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x1104))
#define	rPMGR_UART4_PS			(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x1108))
#define	rPMGR_UART5_PS			(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x110C))
#define	rPMGR_UART6_PS			(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x1110))
#define	rPMGR_PKE_PS			(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x1114))
#define	rPMGR_I2C0_PS			(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x1118))
#define	rPMGR_I2C1_PS			(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x111C))
#define	rPMGR_I2C2_PS			(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x1120))
#define	rPMGR_PWM_PS			(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x1124))
#define	rPMGR_SCRT_PS			(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x1128))
#define	rPMGR_GPIO_PS			(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x112C))
#define	rPMGR_SWI_PS			(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x1130))
#define	rPMGR_DWI_PS			(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x1134))
#define	rPMGR_DEBUG_PS			(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x1138))
#define	rPMGR_AIC_PS			(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x113C))
#define	rPMGR_USB2HOST1_PS		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x1140))
#define	rPMGR_USB2HOST1_OHCI_PS		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x1144))

#define  PMGR_PS_RESET			(1 << 31)

#define	PMGR_FIRST_PS			(&rPMGR_FIRST_PS)
#define	PMGR_LAST_PS			(&rPMGR_USB2HOST1_OHCI_PS)
#define	PMGR_PS_COUNT			(((((u_int32_t)(PMGR_LAST_PS)) - ((u_int32_t)(PMGR_FIRST_PS))) / 4) + 1)
#define	PMGR_PS_NUM(_r)			(((u_int32_t)&(rPMGR_ ## _r ## _PS) - ((u_int32_t)(PMGR_FIRST_PS))) / 4)

#define	rPMGR_PWR_GATE_CTL_SET		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x1200))
#define	rPMGR_PWR_GATE_CTL_CLR		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x1204))
#define	 PMGR_PWR_GATE_IOP		(1 << 6)

#define	rPMGR_PWR_GATE_GFX_MODE		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x1208))

#define	rPMGR_CORE_OFFLINE_SET		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x1210))
#define	rPMGR_CORE_OFFLINE_CLR		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x1214))
#define	rPMGR_WAKE_CORES		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x1220))

#define	PMGR_PWR_GATE_TIME_COUNT	(15)
#define	rPMGR_PWR_GATE_TIME_A(_n)	(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x1240 + (((_n) - 1) * 8)))
#define	rPMGR_PWR_GATE_TIME_B(_n)	(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x1244 + (((_n) - 1) * 8)))
#define	rPMGR_PWR_GATE_DBG0		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x12E0))
#define	rPMGR_PWR_GATE_DBG1		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x12E4))

#define rPMGR_VOLMAN_CTL		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x1380))
#define  PMGR_VOLMAN_DISABLE_VOL_CHANGE	(1 << 10)
#define  PMGR_VOLMAN_CPU_SW_OFF_TIME_VALUE(_x) ((_x) << 2)
#define  PMGR_VOLMAN_BIT_ORDER_MSB	(0 << 1)
#define  PMGR_VOLMAN_BIT_ORDER_LSB	(1 << 1)
#define  PMGR_VOLMAN_BYTE_NO_SWAP	(0 << 0)
#define  PMGR_VOLMAM_BYTE_SWAP		(1 << 0)

#define kDVFM_STATE_COUNT		(8)
#define	kDVFM_STATE_BYPASS		(0)
#define	kDVFM_STATE_SECUREROM		(1)
#define	kDVFM_STATE_IBOOT		(2)

#define	PMGR_DVFM_STATE_COUNT		(8)
#define	rPMGR_DVFM_CFG0(_s)		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x1400 + ((_s) * 0x20)))
#define	rPMGR_DVFM_CFG1(_s)		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x1404 + ((_s) * 0x20)))
#define  PMGR_DVFM_CFG1_CLK_SRC_SHIFT	(18)
#define  PMGR_DVFM_CFG1_MIN_VOL_SHIFT	(8)
#define  PMGR_DVFM_CFG1_SAFE_VOL_SHIFT	(0)
#define  PMGR_DVFM_CFG1_VOL_MASK	(0xFF)
#define	rPMGR_DVFM_CFG2(_s)		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x1408 + ((_s) * 0x20)))
#define  PMGR_DVFM_CFG2_SRAM_VOL_SHIFT	(24)
#define  PMGR_DVFM_CFG2_SRAM_VOL_MASK	(0xFF)
#define	rPMGR_DVFM_CFG3(_s)		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x140C + ((_s) * 0x20)))
#define	rPMGR_DVFM_CFG4(_s)		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x1410 + ((_s) * 0x20)))

#define	rPMGR_DVFM_COMMON_CFG		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x1600))
#define	rPMGR_DVFM_STA_CTL		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x1604))
#define	rPMGR_DVFM_CFG_SEL		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x1608))
#define	rPMGR_DVFM_DELAY		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x160C))
#define  PMGR_DVFM_DELAY_VOL_CHG_FIX_DLY_SHIFT (8)
#define  PMGR_DVFM_DELAY_VOL_CHG_VAR_DLY_SHIFT (0)
#define	rPMGR_DVFM_SRAM_VOL_DELAY	(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x1610))
#define  PMGR_DVFM_SRAM_VOL_DELAY_VOL_CHG_FIX_DLY_SHIFT (8)
#define  PMGR_DVFM_SRAM_VOL_DELAY_VOL_CHG_VAR_DLY_SHIFT (0)

#define	rPMGR_SENSOR_CMD		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x1680))
#define	rPMGR_CORE0_TMP_SEN_RSLT	(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x1684))
#define	rPMGR_CORE1_TMP_SEN_RSLT	(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x1688))
#define	rPMGR_CORE0_SPD_SEN_RSLT	(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x168C))
#define	rPMGR_CORE1_SPD_SEN_RSLT	(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x1690))

#define	rPMGR_APSC_STA_CTL		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x1700))
#define  PMGR_APSC_PENDING		(1 << 31)
#define  PMGR_APSC_SCC_MANUAL_CHANGE	(1 << 25)
#define  PMGR_APSC_SOC_MANUAL_CHANGE	(1 << 24)
#define  PMGR_APSC_SCC_MANUAL_STATE(_d)	(((_d) & 0x7) << 20)
#define  PMGR_APSC_SOC_MANUAL_STATE(_p)	(((_p) & 0xF) << 8)
#define  PMGR_APSC_MANUAL_CHANGE(_d, _p) \
	(PMGR_APSC_SCC_MANUAL_CHANGE | PMGR_APSC_SOC_MANUAL_CHANGE | \
	 PMGR_APSC_SCC_MANUAL_STATE(_d) | PMGR_APSC_SOC_MANUAL_STATE(_p))

#define	rPMGR_SCC_SLEEP_TIME_STAMP_HI	(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x1704))
#define	rPMGR_SCC_SLEEP_TIME_STAMP_LO	(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x1708))
#define	rPMGR_SCC_WAKE_TIME_STAMP_HI	(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x170C))
#define	rPMGR_SCC_WAKE_TIME_STAMP_LO	(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x1710))
#define	rPMGR_SCC_MANUAL_TIME_STAMP_HI	(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x1714))
#define	rPMGR_SCC_MANUAL_TIME_STAMP_LO	(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x1718))
#define	rPMGR_SOC_SLEEP_TIME_STAMP_HI	(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x171C))
#define	rPMGR_SOC_SLEEP_TIME_STAMP_LO	(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x1720))
#define	rPMGR_SOC_WAKE_TIME_STAMP_HI	(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x1724))
#define	rPMGR_SOC_WAKE_TIME_STAMP_LO	(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x1728))
#define	rPMGR_SOC_MANUAL_TIME_STAMP_HI	(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x172C))
#define	rPMGR_SOC_MANUAL_TIME_STAMP_LO	(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x1730))

#define	rPMGR_TEST_CLK			(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x2000))

#define	rPMGR_OSC_DEBUG			(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x200C))
#define	rPMGR_PLL_DEBUG(_n)		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x2010 + ((_n) * 4)))
#define  PMGR_PLL_DEBUG_ENABLED			(1 << 31)
#define  PMGR_PLL_DEBUG_BYP_ENABLED		(1 << 30)
#define  PMGR_PLL_DEBUG_DBG_STPCLK_EN		(1 << 24)
#define  PMGR_PLL_DEBUG_FSTATE			(1 << 12)
#define  PMGR_PLL_DEBUG_STATE(_r)		(((_r) >> 0) & 0xFFF)

#define	rPMGR_DOUBLER_DEBUG		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x2030))
#define  PMGR_DOUBLER_DEBUG_ENABLED	(1 << 31)
#define  PMGR_DOUBLER_DEBUG_BYP_ENABLED	(1 << 30)

#define	rPMGR_SRAM_EMA_B0_Pn_LO(_n)	(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x2128 + (((_n) - 1) * 8)))
#define	rPMGR_SRAM_EMA_B0_Pn_HI(_n)	(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x212C + (((_n) - 1) * 8)))

enum {
  EMA_MCU = 1,
  EMA_GFX_PIPES_SYS = 2,
  EMA_GFX_HYDRA = 3,
  EMA_VDEC = 4,
  EMA_VENC = 5,
  EMA_ISP = 6,
  EMA_DISP = 7,
  EMA_NRT = 8,
  EMA_PMGR = 9,
  EMA_IOP = 10,
  EMA_SAUDIO = 11,
  EMA_CDIO = 12,
  EMA_FMI = 13,
  EMA_ADSP = 14,
};

#define	rPMGR_WDOG_TMR			(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x3020))
#define	rPMGR_WDOG_RST			(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x3024))
#define	rPMGR_WDOG_INT			(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x3028))
#define	rPMGR_WDOG_CTL			(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x302C))

#define	rPMGR_PERF_CTL			(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x4000))
#define	rPMGR_PERF_STATUS		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x4004))
#define	rPMGR_PERF_CTR0_LO		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x4020))
#define	rPMGR_PERF_CTR0_HI		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x4024))
#define	rPMGR_PERF_CFG0_LO		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x4028))
#define	rPMGR_PERF_CFG0_MID		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x402C))
#define	rPMGR_PERF_CFG0_HI		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x4030))
#define	rPMGR_PERF_CTR1_LO		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x4040))
#define	rPMGR_PERF_CTR1_HI		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x4044))
#define	rPMGR_PERF_CFG1_LO		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x4048))
#define	rPMGR_PERF_CFG1_MID		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x404C))
#define	rPMGR_PERF_CFG1_HI		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x4050))

#define	rPMGR_SCRATCH0			(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x6000))
#define	rPMGR_SCRATCH1			(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x6004))
#define	rPMGR_SCRATCH2			(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x6008))
#define	rPMGR_SCRATCH3			(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x600C))
#define	rPMGR_SCRATCH4			(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x6010))
#define	rPMGR_SCRATCH5			(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x6014))
#define	rPMGR_SCRATCH6			(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x6018))
#define	rPMGR_SCRATCH7			(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x601C))
#define	rPMGR_SCRATCH8			(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x6020))
#define	rPMGR_SCRATCH9			(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x6024))
#define	rPMGR_SCRATCH10			(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x6028))
#define	rPMGR_SCRATCH11			(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x602C))
#define	rPMGR_SCRATCH12			(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x6030))
#define	rPMGR_SCRATCH13			(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x6034))
#define	rPMGR_SCRATCH14			(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x6038))
#define	rPMGR_SCRATCH15			(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x603C))

#define	rPMGR_DAC_FS			(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x7000))

#define	rPMGR_USB_CTL			(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x8000))

#define	rPMGR_SHA_SEL			(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x9000))

#define	rPMGR_SECURITY			(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0xA000))

#define	rPMGR_GFX_CTL			(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0xB000))


#define rPMGR_THERMAL0_CTL0             (*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0xC000))
#define rPMGR_THERMAL0_CTL1             (*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0xC004))
#define rPMGR_THERMAL0_CTL2             (*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0xC008))
#define rPMGR_THERMAL0_CTL3             (*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0xC00C))
#define rPMGR_THERMAL0_RDBK0            (*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0xC010))
#define rPMGR_THERMAL0_RDBK1            (*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0xC014))
#define rPMGR_THERMAL0_SUM              (*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0xC018))
#define rPMGR_THERMAL0_SUM_CNT          (*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0xC01C))
#define rPMGR_THERMAL1_CTL0             (*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0xC040))
#define rPMGR_THERMAL1_CTL1             (*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0xC044))
#define rPMGR_THERMAL1_CTL2             (*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0xC048))
#define rPMGR_THERMAL1_CTL3             (*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0xC04C))
#define rPMGR_THERMAL1_RDBK0            (*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0xC050))
#define rPMGR_THERMAL1_RDBK1            (*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0xC054))
#define rPMGR_THERMAL1_SUM              (*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0xC058))
#define rPMGR_THERMAL1_SUM_CNT          (*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0xC05C))


#define PGMR_GET_PERF_STATE_INDEX(_n, _x)	(((_x) >> ((_n) * 4)) & 0xF)
#define PGMR_SET_PERF_STATE_INDEX(_n, _i)	(((_i) & 0xF) << ((_n) * 4))
#define PMGR_PERF_STATE_V(_n)			(_n)
#define PMGR_PERF_STATE_P			(3)
#define PMGR_PERF_STATE_M(_n)			(4 + (_n))

enum {
	PMGR_CLK_OSC = 0,
	PMGR_CLK_PLL0,
	PMGR_CLK_PLL1,
	PMGR_CLK_PLL2,
	PMGR_CLK_PLL3,
	PMGR_CLK_PLL4,
	PMGR_CLK_PLL5,
	PMGR_CLK_PLL6,
	PMGR_CLK_PLLUSB,
	PMGR_CLK_DOUBLER,
	PMGR_CLK_PLL2_GATED,
	PMGR_CLK_PLL4_GATED,
	PMGR_CLK_PLL5_GATED,
	PMGR_CLK_MCU_FIXED,
	PMGR_CLK_MCU,
	PMGR_CLK_CPU,
	PMGR_CLK_PREDIV0,
	PMGR_CLK_PREDIV1,
	PMGR_CLK_PREDIV2,
	PMGR_CLK_PREDIV3,
	PMGR_CLK_PREDIV4,
	PMGR_CLK_PREDIV5,
	PMGR_CLK_GFX,
	PMGR_CLK_GFX_SYS,
	PMGR_CLK_HPERFRT,
	PMGR_CLK_DISPOUT,
	PMGR_CLK_VID0,
	PMGR_CLK_VID1,
	PMGR_CLK_HPERFNRT,
	PMGR_CLK_NRT_MEM,
	PMGR_CLK_VENC_MTX,
	PMGR_CLK_VENC,
	PMGR_CLK_VDEC,
	PMGR_CLK_ISP,
	PMGR_CLK_IOP,
	PMGR_CLK_CDIO,
	PMGR_CLK_LPERFS,
	PMGR_CLK_PCLK0,
	PMGR_CLK_PCLK1,
	PMGR_CLK_PCLK2,
	PMGR_CLK_PCLK3,
	PMGR_CLK_AES,
	PMGR_CLK_MEDIUM0,
	PMGR_CLK_MEDIUM1,
	PMGR_CLK_I2C,
	PMGR_CLK_SDIO,
	PMGR_CLK_AUDIO,
	PMGR_CLK_UPERF,
	PMGR_CLK_DEBUG,
	PMGR_CLK_SCC_PWR,
	PMGR_CLK_SCC_DMA,
	PMGR_CLK_SPI0,
	PMGR_CLK_SPI1,
	PMGR_CLK_SPI2,
	PMGR_CLK_SPI3,
	PMGR_CLK_SPI4,
	PMGR_CLK_SLOW,
	PMGR_CLK_SLEEP,
	PMGR_CLK_USB_PHY0,
	PMGR_CLK_USBOHCI,
	PMGR_CLK_USB12,
	PMGR_CLK_NCO_REF0,
	PMGR_CLK_NCO_REF1,
	PMGR_CLK_NCO_REF2,
	PMGR_CLK_USB_PHY1,
	PMGR_CLK_USB_EHCI_FREE,
	PMGR_CLK_COUNT
};

extern void pmgr_update_device_tree(DTNode *pmgr_node);

#endif /* ! __PLATFORM_SOC_PMGR_H */
