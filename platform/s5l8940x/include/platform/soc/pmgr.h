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
#ifndef __PLATFORM_SOC_PMGR_H
#define __PLATFORM_SOC_PMGR_H

#include <lib/devicetree.h>
#include <platform/soc/hwregbase.h>

#define	rPMGR_PLL0_CTL			(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x0000))
#define	rPMGR_PLL0_PARAM		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x0004))
#define	rPMGR_PLL1_CTL			(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x0008))
#define	rPMGR_PLL1_PARAM		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x000C))
#define	rPMGR_PLL2_CTL			(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x0010))
#define	rPMGR_PLL2_PARAM		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x0014))
#define	rPMGR_PLL3_CTL			(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x0018))
#define	rPMGR_PLL3_PARAM		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x001C))
#define	rPMGR_PLL4_CTL			(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x0020))
#define	rPMGR_PLL4_PARAM		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x0024))
#define	rPMGR_PLLUSB_CTL		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x0028))
#define	rPMGR_PLLUSB_PARAM		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x002C))

#define	rPMGR_PLL_ENABLE			(1 << 31)
#define	rPMGR_PLL_EXT_BYPASS			(1 << 30)
#define	rPMGR_PLL_REAL_LOCK			(1 << 29)
#define	rPMGR_PLL_LOAD				(1 << 27)
#define	rPMGR_PLL_BYPASS			(1 << 23)
#define	rPMGR_PLL_P(_p)				(((_p) & 0x03f) << 14)
#if (SUB_PLATFORM_S5L8942X || SUB_PLATFORM_S5L8947X)
#define rPMGR_PLL_VSEL(_p)			(((_p) & 1) << 13)
#endif
#define	rPMGR_PLL_M(_m)				(((_m) & 0x3ff) <<  3)
#define	rPMGR_PLL_S(_s)				(((_s) & 0x007) <<  0)

#define rPMGR_PARAM_EXT_AFC(_n)			((_n) << 17)
#define rPMGR_PARAM_AFC_EN			(1 << 16)
#define rPMGR_PARAM_LOCK_TIME(_n)		((_n) << 0)

#define	rPMGR_PLL_GATES			(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x0030))

#define	rPMGR_DOUBLER_CTL		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x0034))

#define	rPMGR_DOUBLER_ENABLE			(1 << 31)
#define	rPMGR_DOUBLER_EXT_BYPASS		(1 << 30)

#define	rPMGR_CPU_CLK_CFG		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x0038))
#define	rPMGR_MCU_FIXED_CLK_CFG		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x003C))
#define	rPMGR_MCU_CLK_CFG		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x0040))
#define	rPMGR_PCLK1_CLK_CFG		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x0044))

#define	rPMGR_PREDIV0_CLK_CFG		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x0048))
#define	rPMGR_PREDIV1_CLK_CFG		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x004C))
#define	rPMGR_PREDIV2_CLK_CFG		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x0050))
#define	rPMGR_PREDIV3_CLK_CFG		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x0054))
#define	rPMGR_PREDIV4_CLK_CFG		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x0058))
#define	rPMGR_PREDIV5_CLK_CFG		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x005C))
#define	rPMGR_PREDIV6_CLK_CFG		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x0060))

#define	rPMGR_MANAGED0_CLK_CFG		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x0064))
#define	rPMGR_MANAGED1_CLK_CFG		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x0068))
#define	rPMGR_MANAGED2_CLK_CFG		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x006C))
#define	rPMGR_MANAGED3_CLK_CFG		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x0070))
#define	rPMGR_MANAGED4_CLK_CFG		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x0074))

#define	rPMGR_VID1_CLK_CFG		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x0078))
#define	rPMGR_MEDIUM0_CLK_CFG		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x007C))
#define	rPMGR_VID0_CLK_CFG		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x0080))
#define	rPMGR_I2C_CLK_CFG		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x0084))
#define	rPMGR_SDIO_CLK_CFG		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x0088))
#define	rPMGR_MIPI_DSI_CLK_CFG		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x008C))
#define	rPMGR_AUDIO_CLK_CFG		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x0090))
#define	rPMGR_HPARK_PCLK0_CLK_CFG	(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x0094))
#define	rPMGR_HPARK_TCLK_CLK_CFG	(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x0098))
#define	rPMGR_UPERF_CLK_CFG		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x009C))
#define	rPMGR_DEBUG_CLK_CFG		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x00A0))

#define	rPMGR_HPERFRT_CLK_CFG		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x00A4))
#define	rPMGR_GFX_CLK_CFG		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x00A8))
#define	rPMGR_GFX_SLC_CLK_CFG		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x00AC))

#define	rPMGR_HPERFNRT_CLK_CFG		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x00B0))
#define	rPMGR_ISP_CLK_CFG		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x00B4))
#define	rPMGR_IOP_CLK_CFG		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x00B8))
#define	rPMGR_CDIO_CLK_CFG		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x00BC))

#define	rPMGR_LPERFS_CLK_CFG		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x00C0))
#define	rPMGR_PCLK0_CLK_CFG		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x00C4))
#define	rPMGR_PCLK2_CLK_CFG		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x00C8))
#define	rPMGR_PCLK3_CLK_CFG		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x00CC))

#define	rPMGR_MEDIUM1_CLK_CFG		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x00D0))
#define	rPMGR_SPI0_CLK_CFG		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x00D4))
#define	rPMGR_SPI1_CLK_CFG		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x00D8))
#define	rPMGR_SPI2_CLK_CFG		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x00DC))
#define	rPMGR_SPI3_CLK_CFG		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x00E0))
#define	rPMGR_SPI4_CLK_CFG		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x00E4))
#define	rPMGR_SLEEP_CLK_CFG		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x00E8))
#define	rPMGR_USBPHY_CLK_CFG		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x00EC))
#define	rPMGR_USBOHCI_CLK_CFG		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x00F0))
#define	rPMGR_USB12_CLK_CFG		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x00F4))
#define	rPMGR_NCO_REF0_CLK_CFG		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x00F8))
#define	rPMGR_NCO_REF1_CLK_CFG		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x00FC))
#define	rPMGR_VENC_MTX_CLK_CFG		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x0100))
#define	rPMGR_VENC_CLK_CFG		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x0104))

#define	PMGR_FIRST_CLK_CFG		(&rPMGR_CPU_CLK_CFG)
#define	PMGR_LAST_CLK_CFG		(&rPMGR_VENC_CLK_CFG)
#define	PMGR_CLK_CFG_COUNT		(((((u_int32_t)(PMGR_LAST_CLK_CFG)) - ((u_int32_t)(PMGR_FIRST_CLK_CFG))) / 4) + 1)
#define	PMGR_CLK_NUM(_r)		(((u_int32_t)&(rPMGR_ ## _r ## _CLK_CFG) - ((u_int32_t)(PMGR_FIRST_CLK_CFG))) / 4)

#define	rPMGR_I2S0_NCO_CLK_CFG		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x0110))
#define	rPMGR_I2S0_NCO_N1		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x0114))
#define	rPMGR_I2S0_NCO_N2		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x0118))

#define	rPMGR_I2S1_NCO_CLK_CFG		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x0120))
#define	rPMGR_I2S1_NCO_N1		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x0124))
#define	rPMGR_I2S1_NCO_N2		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x0128))

#define	rPMGR_I2S2_NCO_CLK_CFG		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x0130))
#define	rPMGR_I2S2_NCO_N1		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x0134))
#define	rPMGR_I2S2_NCO_N2		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x0138))

#define	rPMGR_I2S3_NCO_CLK_CFG		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x0140))
#define	rPMGR_I2S3_NCO_N1		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x0144))
#define	rPMGR_I2S3_NCO_N2		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x0148))

#define	rPMGR_SPDIF_NCO_CLK_CFG		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x0150))
#define	rPMGR_SPDIF_NCO_N1		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x0154))
#define	rPMGR_SPDIF_NCO_N2		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x0158))

#define	rPMGR_DP_NCO_CLK_CFG		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x0160))
#define	rPMGR_DP_NCO_N1			(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x0164))
#define	rPMGR_DP_NCO_N2			(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x0168))

#define	rPMGR_MCA0_NCO_CLK_CFG		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x0170))
#define	rPMGR_MCA0_NCO_N1		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x0174))
#define	rPMGR_MCA0_NCO_N2		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x0178))

#define	rPMGR_CLK_CFG_ENABLE			(1 << 31)
#define	rPMGR_CLK_CFG_PENDING			(1 << 30)
#define	rPMGR_CLK_CFG_SRC_SEL(_s)		(((_s) & 0x3) << 28)
#define	rPMGR_CLK_CFG_SRC_SEL_MASK		rPMGR_CLK_CFG_SRC_SEL(3)
#define	rPMGR_CLK_CFG_DIVIDER(_d)		(((_d) & 0x1f) << 0)
#define	rPMGR_CLK_CFG_DIV_MASK			rPMGR_CLK_CFG_DIVIDER(0x1f)

#define	PMGR_PERF_STATE_COUNT		(16)
#define	rPMGR_PERF_STATE_A(_s)		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x0200 + ((_s) * 0x10)))
#define	rPMGR_PERF_STATE_B(_s)		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x0204 + ((_s) * 0x10)))
#define	rPMGR_PERF_STATE_C(_s)		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x0208 + ((_s) * 0x10)))
#define	rPMGR_PERF_STATE_CTL		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x0300))

#define	PMGR_PERF_STATE_SEL(_s)			(((_s) & 0xF) << 0)
#define	PMGR_PERF_STATE_PENDING			(0xFF << 16)

#define	kPERF_STATE_BYPASS			(0)
#define	kPERF_STATE_SECUREROM			(1)
#define	kPERF_STATE_IBOOT			(2)
#define	kPERF_STATE_IBOOT_CNT			(5)

#define	rPMGR_MCU_CLK_DEBUG		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x0400))

#define	rPMGR_CPU0_PS			(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x1008))
#define	rPMGR_CPU1_PS			(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x100C))
#define	rPMGR_SCU_PS			(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x1010))
#define	rPMGR_L2RAM0_PS			(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x1014))
#define	rPMGR_L2RAM1_PS			(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x1018))
#define	rPMGR_CPU_REGS_PS		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x101C))
#define	rPMGR_MCU_PS			(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x1020))
#define	rPMGR_GFX_SYS_PS		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x1024))
#define	rPMGR_GFX_CORES_PS		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x1028))
#define	rPMGR_HPERFNRT_PS		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x102C))
#define	rPMGR_VDEC_PS			(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x1030))
#define	rPMGR_SCALER0_PS		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x1034))
#define	rPMGR_SCALER1_PS		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x1038))
#define	rPMGR_JPG_PS			(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x103C))
#define	rPMGR_VENCD_PS			(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x1040))
#define	rPMGR_HPERFRT_PS		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x1044))
#define	rPMGR_ISP_PS			(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x1048))
#define	rPMGR_DISP0_PS			(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x1054))
#define	rPMGR_DISP1_PS			(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x1058))
#define	rPMGR_DISPOUT_PS		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x105C))
#define	rPMGR_MIPI_DSI_PS		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x1060))
#define	rPMGR_CLCD_PS			(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x1064))
#define	rPMGR_TVOUT_PS			(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x1068))
#define	rPMGR_RGBOUT_PS			(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x106C))
#define	rPMGR_DPLINK_PS			(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x1070))
#define	rPMGR_CDIO_PS			(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x1074))
#define	rPMGR_CDMA_PS			(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x1078))
#define	rPMGR_IOP_PS			(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x107C))
#define	rPMGR_UPERF_PS			(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x1080))
#define	rPMGR_USBOTG0_PS		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x1084))
#define	rPMGR_USB20_PS			(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x1088))
#define	rPMGR_USB11_PS			(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x108C))
#define	rPMGR_USBOHCI0_PS		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x1090))
#define	rPMGR_USBOHCI1_PS		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x1094))
#define	rPMGR_USBREG_PS			(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x1098))
#define	rPMGR_AUDIO_PS			(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x109C))
#define	rPMGR_I2S0_PS			(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x10A0))
#define	rPMGR_I2S1_PS			(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x10A4))
#define	rPMGR_I2S2_PS			(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x10A8))
#define	rPMGR_I2S3_PS			(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x10AC))
#define	rPMGR_SPDIF_PS			(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x10B0))
#define	rPMGR_HPARK_PS			(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x10B4))
#define	rPMGR_SDIO_PS			(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x10B8))
#define	rPMGR_SHA1_PS			(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x10BC))
#define	rPMGR_SHA2_PS			(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x10C0))
#define	rPMGR_FMI0_PS			(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x10C4))
#define	rPMGR_FMI0BCH_PS		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x10C8))
#define	rPMGR_FMI1_PS			(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x10CC))
#define	rPMGR_FMI1BCH_PS		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x10D0))
#define	rPMGR_FMI2_PS			(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x10D4))
#define	rPMGR_FMI2BCH_PS		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x10D8))
#define	rPMGR_FMI3_PS			(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x10DC))
#define	rPMGR_FMI3BCH_PS		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x10E0))
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
#define	rPMGR_MCA0_PS			(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x1140))
#ifdef SUB_PLATFORM_S5L8947X
#define	rPMGR_ETH_PS			(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x1144))
#define	rPMGR_HDMI_PS			(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x1148))
#define	rPMGR_ADC_PS			(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x114C))
#define	rPMGR_USBOTG1_PS		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x1150))
#define	rPMGR_USB20_1_PS		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x1154))
#define	rPMGR_USBOHCI0_1_PS		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x1158))
#define	rPMGR_USBREG1_PS		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x115C))
#endif
#define rPMGR_PS_RESET				(1 << 31)

#define	PMGR_FIRST_PS			(&rPMGR_CPU0_PS)
#if SUB_PLATFORM_S5L8942X
#define	PMGR_LAST_PS			(&rPMGR_MCA0_PS)
#elif SUB_PLATFORM_S5L8947X
#define	PMGR_LAST_PS			(&rPMGR_USBREG1_PS)
#else
#define	PMGR_LAST_PS			(&rPMGR_AIC_PS)
#endif
#define	PMGR_DEV_PS_COUNT		(((((u_int32_t)(PMGR_LAST_PS)) - ((u_int32_t)(PMGR_FIRST_PS))) / 4) + 1)
#define	PMGR_PS_NUM(_r)			(((u_int32_t)&(rPMGR_ ## _r ## _PS) - ((u_int32_t)(PMGR_FIRST_PS))) / 4)

#define	rPMGR_CPU_CFG			(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x1180))

#define	rPMGR_PWR_GATE_CTL_SET		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x1200))
#define	rPMGR_PWR_GATE_CTL_CLR		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x1204))
#define	rPMGR_PWR_GATE_IOP			(1 << 6)
#define	rPMGR_PWR_GATE_GFX_MODE		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x1208))

#define	rPMGR_CORE_OFFLINE_SET		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x1210))
#define	rPMGR_CORE_OFFLINE_CLR		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x1214))
#define	rPMGR_WAKE_CORES		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x1220))

#define	PMGR_PWR_GATE_TIME_COUNT	(15)
#define	rPMGR_PWR_GATE_TIME_A(_n)	(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x1238 + ((_n) * 8)))
#define	rPMGR_PWR_GATE_TIME_B(_n)	(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x123C + ((_n) * 8)))
#define	rPMGR_PWR_GATE_DBG0		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x12E0))
#define	rPMGR_PWR_GATE_DBG1		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x12E4))

#define	rPMGR_TEST_CLK			(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x2000))
#define	rPMGR_PLL0_DEBUG		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x2010))
#define	rPMGR_PLL1_DEBUG		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x2014))
#define	rPMGR_PLL2_DEBUG		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x2018))
#define	rPMGR_PLL3_DEBUG		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x201C))
#define	rPMGR_PLL4_DEBUG		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x2020))
#define	rPMGR_PLLUSB_DEBUG		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x2024))
#define	rPMGR_DOUBLER_DEBUG		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x2028))

#define	rPMGR_PLL0_TUNE			(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x2080))
#define	rPMGR_PLL1_TUNE			(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x2084))
#define	rPMGR_PLL2_TUNE			(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x2088))
#define	rPMGR_PLL3_TUNE			(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x208C))
#define	rPMGR_PLL4_TUNE			(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x2090))
#define	rPMGR_PLLUSB_TUNE		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x2094))

#define rPMGR_PLL_DEBUG_ENABLED			(1 << 31)
#define rPMGR_PLL_DEBUG_BYP_ENABLED		(1 << 30)
#define rPMGR_PLL_DEBUG_DBG_STPCLK_EN		(1 << 24)
#define rPMGR_PLL_DEBUG_LOST_LOCK		(1 << 16)
#define rPMGR_PLL_DEBUG_STATE(_r)		(((_r) >> 0) & 0xff)

#define rPMGR_EMA_CTL_CPU		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x2100))
#define rPMGR_EMA_CTL_CPU_SPIN			(0x01 << 16)
#define rPMGR_EMA_CTL_SOC		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x2104))
#define rPMGR_EMA_CTL_SOC_SEL			(0x1f << 0)
#define rPMGR_EMA_CTL_SOC_SPIN			(0x01 << 16)

#define	rPMGR_SRAM_EMA_B0_Pn_LO(_n)	(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x2120 + ((_n) * 8)))
#define	rPMGR_SRAM_EMA_B0_Pn_HI(_n)	(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x2124 + ((_n) * 8)))
#define	rPMGR_SRAM_EMA_B1_Pn_LO(_n)	(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x21C0 + ((_n) * 8)))
#define	rPMGR_SRAM_EMA_B1_Pn_HI(_n)	(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x21C4 + ((_n) * 8)))
#if (SUB_PLATFORM_S5L8942X || SUB_PLATFORM_S5L8947X)
#define	rPMGR_SRAM_EMA_B2_Pn_LO(_n)	(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x2200 + ((_n) * 8)))
#define	rPMGR_SRAM_EMA_B2_Pn_HI(_n)	(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x2204 + ((_n) * 8)))
#endif

enum {
  EMA_CPU = 0,
  EMA_MCU = 1,
  EMA_GFX_PIPES_SYS = 2,
  EMA_GFX_HYDRA = 3,
  EMA_VDEC = 4,
  EMA_VENC = 5,
  EMA_ISP = 6,
  EMA_DISP = 7,
  EMA_NRT = 8,
  EMA_CIF = 9,
  EMA_IOP = 10,
  EMA_SAUDIO = 11,
  EMA_CDMA = 12,
  EMA_FMI = 13,
  EMA_HPARK = 14,
};

#define	rPMGR_WDOG_TMR			(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x3020))
#define	rPMGR_WDOG_RST			(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x3024))
#define	rPMGR_WDOG_INT			(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x3028))
#define	rPMGR_WDOG_CTL			(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x302C))

#define	rPMGR_EVENT_TMR			(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x3100))
#define	rPMGR_EVENT_PERIOD		(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x3104))
#define	rPMGR_EVENT_CTL			(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x3108))

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
	PMGR_CLK_PLLUSB,
	PMGR_CLK_DOUBLER,
	PMGR_CLK_CPU,
	PMGR_CLK_MEM,
	PMGR_CLK_PIO,
	PMGR_CLK_ACP,
	PMGR_CLK_MCU_FIXED,
	PMGR_CLK_MCU,
	PMGR_CLK_PREDIV0,
	PMGR_CLK_PREDIV1,
	PMGR_CLK_PREDIV2,
	PMGR_CLK_PREDIV3,
	PMGR_CLK_PREDIV4,
	PMGR_CLK_PREDIV5,
	PMGR_CLK_PREDIV6,
	PMGR_CLK_MANAGED0,
	PMGR_CLK_MANAGED1,
	PMGR_CLK_MANAGED2,
	PMGR_CLK_MANAGED3,
	PMGR_CLK_MANAGED4,
	PMGR_CLK_MEDIUM0,
	PMGR_CLK_MEDIUM1,
	PMGR_CLK_VID0,
	PMGR_CLK_VID1,
	PMGR_CLK_I2C,
	PMGR_CLK_SDIO,
	PMGR_CLK_MIPI_DSI,
	PMGR_CLK_AUDIO,
	PMGR_CLK_HPARK_PCLK,
	PMGR_CLK_HPARK_TCLK,
	PMGR_CLK_UPERF,
	PMGR_CLK_DEBUG,
	PMGR_CLK_HPERFRT,
	PMGR_CLK_GFX,
	PMGR_CLK_GFX_SLC,
	PMGR_CLK_HPERFNRT,
	PMGR_CLK_ISP,
	PMGR_CLK_IOP,
	PMGR_CLK_CDIO,
	PMGR_CLK_LPERFS,
	PMGR_CLK_PCLK0,
	PMGR_CLK_PCLK1,
	PMGR_CLK_PCLK2,
	PMGR_CLK_PCLK3,
	PMGR_CLK_SPI0,
	PMGR_CLK_SPI1,
	PMGR_CLK_SPI2,
	PMGR_CLK_SPI3,
	PMGR_CLK_SPI4,
	PMGR_CLK_SLOW,
	PMGR_CLK_SLEEP,
	PMGR_CLK_USBPHY,
	PMGR_CLK_USBOHCI,
	PMGR_CLK_USB12,
	PMGR_CLK_NCO_REF0,
	PMGR_CLK_NCO_REF1,
	PMGR_CLK_VENC_MTX,
	PMGR_CLK_VENC,
	PMGR_CLK_COUNT
};

extern void pmgr_update_device_tree(DTNode *pmgr_node);

#endif /* ! __PLATFORM_SOC_PMGR_H */
