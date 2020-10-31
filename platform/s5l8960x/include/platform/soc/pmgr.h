/*
 * Copyright (C) 2011 Apple Inc. All rights reserved.
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

#define PMGR_PLL_COUNT		    (6)

#define rPMGR_PLL_CTL(_n)	    (*(volatile uint32_t *)(PMGR_BASE_ADDR + ((_n) * 0x1000)))
#define	 PMGR_PLL_ENABLE	    (1 << 31)
#define	 PMGR_PLL_LOCKED	    (1 << 30)
#define	 PMGR_PLL_LOAD		    (1 << 29)
#define	 PMGR_PLL_EXT_BYPASS	(1 << 27)
#define	 PMGR_PLL_ENABLED	    (1 << 26)
#define	 PMGR_PLL_BYP_ENABLED	(1 << 25)
#define	 PMGR_PLL_M_SHIFT	    (12)
#define	 PMGR_PLL_M_MASK	    (0x1FF)
#define	 PMGR_PLL_P_SHIFT	    (4)
#define	 PMGR_PLL_P_MASK	    (0x1F)
#define	 PMGR_PLL_S_SHIFT	    (0)
#define	 PMGR_PLL_S_MASK	    (0xF)
#define	 PMGR_PLL_M(_m)		    (((_m) & PMGR_PLL_M_MASK) << PMGR_PLL_M_SHIFT)
#define	 PMGR_PLL_P(_p)		    (((_p) & PMGR_PLL_P_MASK) << PMGR_PLL_P_SHIFT)
#define	 PMGR_PLL_S(_s)		    (((_s) & PMGR_PLL_S_MASK) << PMGR_PLL_S_SHIFT)
#define	 PMGR_PLL_FREQ(_m, _p, _s) ((((_m) * OSC_FREQ) / (_p))/((_s) + 1))

#define rPMGR_PLL_CFG(_n)	    (*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x0004 + ((_n) * 0x1000)))
#define	 PMGR_PLL_OFF_MODE_SHIFT    (30)
#define	 PMGR_PLL_OFF_MODE_MASK	    (0x3)
#define	 PMGR_PLL_VCO_OUT_SEL	    (1 << 27)
#define	 PMGR_PLL_LOCK_TIME_MASK    (0xFFFF)
#define	 PMGR_PLL_LOCK_TIME(_n)	    (((_n) & PMGR_PLL_LOCK_TIME_MASK) << 0)

#define rPMGR_PLL_ANA_PARAMS(_n, _p)	(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x0008 + ((_n) * 0x0004) + ((_p) * 0x1000)))
#define rPMGR_PLL_FD_CTL(_n)			(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x0010 + ((_n) * 0x1000)))
#define rPMGR_PLL_DEBUG(_n)		(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x0014 + ((_n) * 0x1000)))
#define rPMGR_PLL_DELAY_CTL(_n, _p)	(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x0018 + ((_n) * 0x0004) + ((_p) * 0x1000)))

#define rPMGR_MCU_FIXED_CLK_CFG		(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x10000))
#define rPMGR_MCU_CLK_CFG			(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x10004))
#define rPMGR_MIPI_DSI_CLK_CFG		(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x10008))
#define rPMGR_NCO_REF0_CLK_CFG		(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x1000C))
#define rPMGR_NCO_REF1_CLK_CFG		(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x10010))
#define rPMGR_NCO_ALG0_CLK_CFG		(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x10014))
#define rPMGR_NCO_ALG1_CLK_CFG		(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x10018))
#define rPMGR_HSICPHY_REF_12M_CLK_CFG	(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x1001C))
#define rPMGR_USB480_0_CLK_CFG		(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x10020))
#define rPMGR_USB480_1_CLK_CFG		(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x10024))
#define rPMGR_USB_OHCI_48M_CLK_CFG	(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x10028))
#define rPMGR_USB_CLK_CFG			(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x1002C))
#define rPMGR_USB_FREE_60M_CLK_CFG	(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x10030))
#define rPMGR_ADSP_T_CLK_CFG		(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x10034))
#define rPMGR_ADSP_P_CLK_CFG		(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x10038))
#define rPMGR_ADSP_TS_CLK_CFG		(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x1003C))
#define rPMGR_SIO_C_CLK_CFG			(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x10040))
#define rPMGR_SIO_P_CLK_CFG			(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x10044))
#define rPMGR_ISP_C_CLK_CFG			(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x10048))
#define rPMGR_ISP_CLK_CFG			(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x1004C))
#define rPMGR_ISP_SENSOR0_REF_CLK_CFG	(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x10050))
#define rPMGR_ISP_SENSOR1_REF_CLK_CFG	(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x10054))
#define rPMGR_VDEC_CLK_CFG			(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x10058))
#define rPMGR_VENC_CLK_CFG			(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x1005C))
#define rPMGR_VID0_CLK_CFG			(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x10060))
#define rPMGR_DISP0_CLK_CFG			(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x10064))
#define rPMGR_DISP1_CLK_CFG			(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x10068))
#define rPMGR_AJPEG_IP_CLK_CFG		(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x1006C))
#define rPMGR_AJPEG_WRAP_CLK_CFG	(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x10070))
#define rPMGR_MSR_CLK_CFG			(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x10074))
#define rPMGR_AF_CLK_CFG			(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x10078))
#define rPMGR_ANS_DLL_CLK_CFG		(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x1007C))
#define rPMGR_ANS_C_CLK_CFG			(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x10080))
#define rPMGR_ANC_LINK_CLK_CFG		(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x10084))
#define rPMGR_LIO_CLK_CFG			(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x10088))
#define rPMGR_MCA0_M_CLK_CFG		(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x1008C))
#define rPMGR_MCA1_M_CLK_CFG		(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x10090))
#define rPMGR_MCA2_M_CLK_CFG		(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x10094))
#define rPMGR_MCA3_M_CLK_CFG		(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x10098))
#define rPMGR_MCA4_M_CLK_CFG		(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x1009C))
#define rPMGR_SEP_CLK_CFG			(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x100A0))
#define rPMGR_GPIO_CLK_CFG			(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x100A4))
#define rPMGR_SPI0_N_CLK_CFG		(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x100A8))
#define rPMGR_SPI1_N_CLK_CFG		(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x100AC))
#define rPMGR_SPI2_N_CLK_CFG		(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x100B0))
#define rPMGR_SPI3_N_CLK_CFG		(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x100B4))
#define rPMGR_DEBUG_CLK_CFG			(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x100B8))

#define PMGR_FIRST_CLK_CFG		(&rPMGR_MCU_FIXED_CLK_CFG)
#define PMGR_LAST_CLK_CFG		(&rPMGR_DEBUG_CLK_CFG)
#define PMGR_CLK_CFG_COUNT	    (((((void *)(PMGR_LAST_CLK_CFG)) - ((void *)(PMGR_FIRST_CLK_CFG))) / 4) + 1)
#define PMGR_CLK_NUM(_r)	    (((void *)&(rPMGR_ ## _r ## _CLK_CFG) - ((void *)(PMGR_FIRST_CLK_CFG))) / 4)

#define rPMGR_S0_CLK_CFG		(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x10200))
#define rPMGR_S1_CLK_CFG		(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x10204))
#define rPMGR_S2_CLK_CFG		(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x10208))
#define rPMGR_S3_CLK_CFG		(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x1020C))
#define rPMGR_ISP_REF0_CLK_CFG	(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x10210))
#define rPMGR_ISP_REF1_CLK_CFG	(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x10214))

#define PMGR_FIRST_SPARE_CLK_CFG	(&rPMGR_S0_CLK_CFG)
#define PMGR_LAST_SPARE_CLK_CFG		(&rPMGR_ISP_REF1_CLK_CFG)
#define PMGR_SPARE_CLK_CFG_COUNT	(((((void *)(PMGR_LAST_SPARE_CLK_CFG)) - ((void *)(PMGR_FIRST_SPARE_CLK_CFG))) / 4) + 1)

#define rPMGR_CLK_DIVIDER_ACG_CFG	(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x10300))
#define rPMGR_CLK_DIVIDER_ACG_CFG1	(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x10304))
#define rPMGR_PLL_ACG_CFG			(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x10308))
   #define PMGR_ACG_CFG_PLLX_AUTO_DISABLE(_n)	(1 << (_n))

#define PMGR_CLK_CFG_ENABLE			(1 << 31)
#define PMGR_CLK_CFG_PENDING		(1 << 30)
#define PMGR_CLK_CFG_PENDING_SHIFT	(30)
#define PMGR_CLK_CFG_SRC_SEL_MASK	(0x3F)
#define PMGR_CLK_CFG_CFG_SEL_SHIFT	(16)
#define PMGR_CLK_CFG_SRC_SEL_SHIFT	(24)
#define PMGR_CLK_CFG_SRC_SEL(_n)	((_n) << PMGR_CLK_CFG_SRC_SEL_SHIFT)
#define PMGR_CLK_CFG_DIVISOR_MASK	(0x3F)
#define PMGR_CLK_CFG_DIVISOR(_n)	((_n) << 0)

#define rPMGR_NCO_CLK_CFG(_n)	(*(volatile uint32_t *) (PMGR_BASE_ADDR + (_n) * 0x1000 + 0x11000 + 0x0))
#define rPMGR_NCO_DIV_INT(_n)	(*(volatile uint32_t *) (PMGR_BASE_ADDR + (_n) * 0x1000 + 0x11000 + 0x4))
#define rPMGR_NCO_DIV_FRAC_N1(_n)   (*(volatile uint32_t *) (PMGR_BASE_ADDR + (_n) * 0x1000 + 0x11000 + 0x8))
#define rPMGR_NCO_DIV_FRAC_N2(_n)   (*(volatile uint32_t *) (PMGR_BASE_ADDR + (_n) * 0x1000 + 0x11000 + 0xc))
#define rPMGR_NCO_DIV_FRAC_PRIME(_n) (*(volatile uint32_t *) (PMGR_BASE_ADDR + (_n) * 0x1000 + 0x11000 + 0x10))

#define PMGR_NUM_NCO	5

#define rPMGR_AGILE_CLK_CTL		(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x1b000))

#define kPMGR_GFX_STATE_MAX		(16)
#define rPMGR_GFX_PERF_STATE_ENTRY(_n)	(*(volatile uint32_t *)(PMGR_BASE_ADDR + (_n) * 0x10 + 0x1c000))
#define rPMGR_GFX_PERF_STATE_CTL	(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x1c200))
#define rPMGR_GFX_PERF_STATE_SOCHOT	(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x1c300))

#define rPMGR_CPU0_PS		    (*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x20000))
#define rPMGR_CPU1_PS		    (*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x20008))
#define rPMGR_CPM_PS		    (*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x20010))
#define rPMGR_LIO_PS		    (*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x20018))
#define rPMGR_IOMUX_PS		    (*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x20020))
#define rPMGR_AIC_PS		    (*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x20028))
#define rPMGR_DEBUG_PS		    (*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x20030))
#define rPMGR_DWI_PS		    (*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x20038))
#define rPMGR_GPIO_PS		    (*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x20040))
#define rPMGR_MCA0_PS		    (*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x20048))
#define rPMGR_MCA1_PS		    (*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x20050))
#define rPMGR_MCA2_PS		    (*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x20058))
#define rPMGR_MCA3_PS		    (*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x20060))
#define rPMGR_MCA4_PS		    (*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x20068))
#define rPMGR_PWM0_PS		    (*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x20070))
#define rPMGR_I2C0_PS		    (*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x20078))
#define rPMGR_I2C1_PS		    (*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x20080))
#define rPMGR_I2C2_PS		    (*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x20088))
#define rPMGR_I2C3_PS		    (*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x20090))
#define rPMGR_SPI0_PS		    (*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x20098))
#define rPMGR_SPI1_PS		    (*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x200A0))
#define rPMGR_SPI2_PS		    (*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x200A8))
#define rPMGR_SPI3_PS		    (*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x200B0))
#define rPMGR_UART0_PS		    (*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x200B8))
#define rPMGR_UART1_PS		    (*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x200C0))
#define rPMGR_UART2_PS		    (*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x200C8))
#define rPMGR_UART3_PS		    (*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x200D0))
#define rPMGR_UART4_PS		    (*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x200D8))
#define rPMGR_UART5_PS		    (*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x200E0))
#define rPMGR_UART6_PS		    (*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x200E8))
#define rPMGR_SECUART0_PS	    (*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x200F0))
#define rPMGR_SECUART1_PS	    (*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x200F8))
#define rPMGR_AES0_PS		    (*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x20100))
#define rPMGR_SIO_PS		    (*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x20108))
#define rPMGR_SIO_P_PS		    (*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x20110))
#define rPMGR_HSIC0_PS		    (*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x20118))
#define rPMGR_HSIC1_PS		    (*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x20120))
#define rPMGR_HSIC2_PS		    (*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x20128))
#define rPMGR_ISPSENS0_PS	    (*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x20130))
#define rPMGR_ISPSENS1_PS	    (*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x20138))
#define rPMGR_MCC_PS		    (*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x20140))
#define rPMGR_MCU_PS		    (*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x20148))
#define rPMGR_AMP_PS		    (*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x20150))
#define rPMGR_USB_PS		    (*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x20158))
#define rPMGR_USBCTLREG_PS	    (*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x20160))
#define rPMGR_USB2HOST0_OHCI_PS	(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x20168))
#define rPMGR_USB2HOST0_PS	    (*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x20170))
#define rPMGR_USB2HOST1_OHCI_PS	(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x20178))
#define rPMGR_USB2HOST1_PS	    (*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x20180))
#define rPMGR_USB_OTG_PS	    (*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x20188))
#define rPMGR_SMX_PS		    (*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x20190))
#define rPMGR_SF_PS				(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x20198))
#define rPMGR_CP_PS		    	(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x201A0))
#define rPMGR_DISP_BUSMUX_PS	(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x201A8))
#define rPMGR_DISP0_PS		    (*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x201B0))
#define rPMGR_MIPI_DSI_PS	    (*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x201B8))
#define rPMGR_DP_PS		    	(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x201C0))
#define rPMGR_DISP1_PS		    (*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x201C8))
#define rPMGR_ISP_PS		    (*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x201D0))
#define rPMGR_MEDIA_PS		    (*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x201D8))
#define rPMGR_MSR_PS		    (*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x201E0))
#define rPMGR_JPG_PS		    (*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x201E8))
#define rPMGR_VDEC_PS		    (*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x201F0))
#define rPMGR_VENC_PS		    (*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x201F8))
#define rPMGR_ANS_PS		    (*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x20200))
#define rPMGR_ANS_DLL_PS	    (*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x20208))
#define rPMGR_ADSP_PS		    (*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x20210))
#define rPMGR_GFX_PS		    (*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x20218))
#define rPMGR_SEP_PS		    (*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x20268))

#define	 PMGR_PS_RESET			(1 << 31)
#define	 PMGR_PS_ACTUAL_PS_SHIFT	(4)
#define	 PMGR_PS_ACTUAL_PS_MASK		(0xF)
#define	 PMGR_PS_MANUAL_PS_MASK		(0xF)
#define	 PMGR_PS_RUN_MAX		(0xF)
#define	 PMGR_PS_CLOCK_OFF		(0x4)
#define	 PMGR_PS_POWER_OFF		(0x0)

#define PMGR_FIRST_PS			(&rPMGR_CPU0_PS)
#define PMGR_LAST_PS			(&rPMGR_GFX_PS)
#define PMGR_PS_COUNT			(((((void *)(PMGR_LAST_PS)) - ((void *)(PMGR_FIRST_PS))) / 8) + 1)
#define PMGR_PS_NUM(_r)			(((void *)&(rPMGR_ ## _r ## _PS) - ((void *)(PMGR_FIRST_PS))) / 8)

#define rPMGR_PWRGATE_CPU0_CFG0		(*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x20800))

#define rPMGR_PWRGATE_AMC_CFG0		(*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x20830))
#define rPMGR_PWRGATE_AMC_CFG1		(*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x20834))

#define rPMGR_PWRGATE_USB_CFG0		(*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x20840))
#define rPMGR_PWRGATE_USB_CFG1		(*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x20844))

#define rPMGR_PWRGATE_ACS_CFG0		(*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x20850))
#define rPMGR_PWRGATE_ACS_CFG1		(*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x20854))

#define rPMGR_PWRGATE_DISP0_CFG0	(*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x20860))
#define rPMGR_PWRGATE_DISP0_CFG1	(*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x20864))

#define rPMGR_PWRGATE_DISP1_CFG0	(*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x20870))
#define rPMGR_PWRGATE_DISP1_CFG1	(*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x20874))

#define rPMGR_PWRGATE_ISP_CFG0		(*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x20880))
#define rPMGR_PWRGATE_ISP_CFG1		(*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x20884))

#define rPMGR_PWRGATE_MEDIA_CFG0	(*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x20890))
#define rPMGR_PWRGATE_MEDIA_CFG1	(*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x20894))

#define rPMGR_PWRGATE_DEC_CFG0		(*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x208a0))
#define rPMGR_PWRGATE_DEC_CFG1		(*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x208a4))

#define rPMGR_PWRGATE_ENC_CFG0		(*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x208b0))
#define rPMGR_PWRGATE_ENC_CFG1		(*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x208b4))

#define rPMGR_PWRGATE_ANS_CFG0		(*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x208c0))
#define rPMGR_PWRGATE_ANS_CFG1		(*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x208c4))

#define rPMGR_PWRGATE_ADSP_CFG0		(*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x208d0))
#define rPMGR_PWRGATE_ADSP_CFG1		(*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x208d4))

#define rPMGR_PWRGATE_GFX_CFG0		(*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x208e0))
#define rPMGR_PWRGATE_GFX_CFG1		(*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x208e4))

#define rPMGR_PWRGATE_SEP_CFG0		(*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x208f0))
#define rPMGR_PWRGATE_SEP_CFG1		(*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x208f4))

#define rPMGR_MCU_ASYNC_RESET		(*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x20a00))

#define rPMGR_VOLMAN_CTL		(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x20c00))
#define rPMGR_VOLMAN_SOC_VOLTAGE	(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x20c10))
#define rPMGR_VOLMAN_SRAM_VOLTAGE	(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x20c14))
#define rPMGR_VOLMAN_GFX_VOLTAGE	(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x20c18))
#define rPMGR_VOLMAN_CPU_VOLTAGE	(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x20c1C))
#define rPMGR_VOLMAN_SOC_DELAY		(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x20c30))
#define rPMGR_VOLMAN_SRAM_DELAY		(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x20c34))
#define rPMGR_VOLMAN_GFX_DELAY		(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x20c38))
#define rPMGR_VOLMAN_CPU_DELAY		(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x20c3C))

#define	 PMGR_VOLMAN_DISABLE_VOL_CHANGE (1 << 10)

#define rPMGR_EMA_GFX0			(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x26010))
#define rPMGR_EMA_GFX1			(*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x26014))

#define rPMGR_CHIP_WDOG_TMR		(*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x27000))
#define rPMGR_CHIP_WDOG_RST_CNT		(*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x27004))
#define rPMGR_CHIP_WDOG_INTR_CNT	(*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x27008))
#define rPMGR_CHIP_WDOG_CTL		(*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x2700c))
#define rPMGR_SYS_WDOG_TMR		(*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x27010))
#define rPMGR_SYS_WDOG_RST_CNT		(*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x27014))
#define rPMGR_SYS_WDOG_CTL		(*(volatile uint32_t *) (PMGR_BASE_ADDR + 0x2701c))

#define rPMGR_SCRATCH(_n)	    (*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x29000 + ((_n)*4)))
#define rPMGR_SCRATCH0		    rPMGR_SCRATCH(0)			/* Flags */
#define rPMGR_SCRATCH1		    rPMGR_SCRATCH(1)
#define rPMGR_SCRATCH2		    rPMGR_SCRATCH(2)
#define rPMGR_SCRATCH3		    rPMGR_SCRATCH(3)
#define rPMGR_SCRATCH4		    rPMGR_SCRATCH(4)
#define rPMGR_SCRATCH5		    rPMGR_SCRATCH(5)
#define rPMGR_SCRATCH6		    rPMGR_SCRATCH(6)
#define rPMGR_SCRATCH7		    rPMGR_SCRATCH(7)			/* Consistent debug root pointer */
#define rPMGR_SCRATCH8		    rPMGR_SCRATCH(8)
#define rPMGR_SCRATCH9		    rPMGR_SCRATCH(9)
#define rPMGR_SCRATCH10		    rPMGR_SCRATCH(10)
#define rPMGR_SCRATCH11		    rPMGR_SCRATCH(11)
#define rPMGR_SCRATCH12		    rPMGR_SCRATCH(12)
#define rPMGR_SCRATCH13		    rPMGR_SCRATCH(13)			/* Memory info */
#define rPMGR_SCRATCH14		    rPMGR_SCRATCH(14)			/* Boot Nonce 0 */
#define rPMGR_SCRATCH15		    rPMGR_SCRATCH(15)			/* Boot Nonce 1 */
#define rPMGR_SCRATCH16		    rPMGR_SCRATCH(16)			/* Boot Manifest Hash 0 */
#define rPMGR_SCRATCH17		    rPMGR_SCRATCH(17)			/* Boot Manifest Hash 1 */
#define rPMGR_SCRATCH18		    rPMGR_SCRATCH(18)			/* Boot Manifest Hash 2 */
#define rPMGR_SCRATCH19		    rPMGR_SCRATCH(19)			/* Boot Manifest Hash 3 */
#define rPMGR_SCRATCH20		    rPMGR_SCRATCH(20)			/* Boot Manifest Hash 4 */
#define rPMGR_SCRATCH21		    rPMGR_SCRATCH(21)
#define rPMGR_SCRATCH22		    rPMGR_SCRATCH(22)
#define rPMGR_SCRATCH23		    rPMGR_SCRATCH(23)

#define rPMGR_THERMAL0_CTL0			(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x2B000))
#define rPMGR_THERMAL0_CTL1			(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x2B004))
#define rPMGR_THERMAL0_CTL2			(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x2B008))
#define rPMGR_THERMAL0_CTL3			(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x2B00C))
#define rPMGR_THERMAL0_RDBK0			(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x2B010))
#define rPMGR_THERMAL0_RDBK1			(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x2B014))
#define rPMGR_THERMAL0_SUM			(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x2B018))
#define rPMGR_THERMAL0_SUM_CNT			(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x2B01C))
#define rPMGR_THERMAL1_CTL0			(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x2B040))
#define rPMGR_THERMAL1_CTL1			(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x2B044))
#define rPMGR_THERMAL1_CTL2			(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x2B048))
#define rPMGR_THERMAL1_CTL3			(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x2B04C))
#define rPMGR_THERMAL1_RDBK0			(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x2B050))
#define rPMGR_THERMAL1_RDBK1			(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x2B054))
#define rPMGR_THERMAL1_SUM			(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x2B058))
#define rPMGR_THERMAL1_SUM_CNT			(*(volatile u_int32_t *)(PMGR_BASE_ADDR + 0x2B05C))	

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


#define rPMGR_MISC_ACG		    (*(volatile uint32_t *)(PMGR_BASE_ADDR + 0x32000))

enum {
    PMGR_CLK_OSC = 0,
    PMGR_CLK_PLL0,
    PMGR_CLK_PLL1,
    PMGR_CLK_PLL2,
    PMGR_CLK_PLL3,
    PMGR_CLK_PLL4,
    PMGR_CLK_PLL5,
    PMGR_CLK_CPU,
    PMGR_CLK_MCU_FIXED,
    PMGR_CLK_MCU,
    PMGR_CLK_MIPI_DSI,
    PMGR_CLK_NCO_REF0,
    PMGR_CLK_NCO_REF1,
    PMGR_CLK_NCO_ALG0,
    PMGR_CLK_NCO_ALG1,
    PMGR_CLK_HSICPHY_REF_12M,
    PMGR_CLK_USB480_0,
    PMGR_CLK_USB480_1,
    PMGR_CLK_USB_OHCI_48M,
    PMGR_CLK_USB,
    PMGR_CLK_USB_FREE_60M,
    PMGR_CLK_ADSP_T,
    PMGR_CLK_ADSP_P,
    PMGR_CLK_ADSP_TS,
    PMGR_CLK_SIO_C,
    PMGR_CLK_SIO_P,
    PMGR_CLK_ISP_C,
    PMGR_CLK_ISP,
    PMGR_CLK_ISP_SENSOR0_REF,
    PMGR_CLK_ISP_SENSOR1_REF,
    PMGR_CLK_VDEC,
    PMGR_CLK_VENC,
    PMGR_CLK_VID0,
    PMGR_CLK_DISP0,
    PMGR_CLK_DISP1,
    PMGR_CLK_AJPEG_IP,
    PMGR_CLK_AJPEG_WRAP,
    PMGR_CLK_MSR,
    PMGR_CLK_AF,
    PMGR_CLK_ANS_DLL,
    PMGR_CLK_ANS_C,
    PMGR_CLK_ANC_LINK,
    PMGR_CLK_LIO,
    PMGR_CLK_MCA0_M,
    PMGR_CLK_MCA1_M,
    PMGR_CLK_MCA2_M,
    PMGR_CLK_MCA3_M,
    PMGR_CLK_MCA4_M,
    PMGR_CLK_SEP,
    PMGR_CLK_GPIO,
    PMGR_CLK_SPI0_N,
    PMGR_CLK_SPI1_N,
    PMGR_CLK_SPI2_N,
    PMGR_CLK_SPI3_N,
    PMGR_CLK_DEBUG,
    PMGR_CLK_S0,
    PMGR_CLK_S1,
    PMGR_CLK_S2,
    PMGR_CLK_S3,
    PMGR_CLK_ISP_REF0,
    PMGR_CLK_ISP_REF1,
    PMGR_CLK_COUNT,
    PMGR_CLK_NOT_SUPPORTED
};

#define PMGR_CLK_CFG_INDEX(_clk) ((_clk) - PMGR_CLK_MCU_FIXED)
////////////////////////////////////////////////////////////
//
//  CCC specific
//
////////////////////////////////////////////////////////////

#define CCC_PWRCTL_BASE		    (CCC_ROM_TABLE_BASE_ADDR + 0x220000)
#define CCC_CPM_THERMAL_BASE        (CCC_ROM_TABLE_BASE_ADDR + 0x230000)

#define rCCC_PRE_TD_TMR		    (*(volatile uint32_t *)(CCC_PWRCTL_BASE + 0x008))
#define rCCC_PRE_FLUSH_TMR	    (*(volatile uint32_t *)(CCC_PWRCTL_BASE + 0x010))

#define rCCC_APSC_SCR		    (*(volatile uint64_t *)(CCC_PWRCTL_BASE + 0x020))
#define CCC_APSC_PENDING	    (1 << 31)
#define CCC_APSC_MANUAL_CHANGE(_d)  ((1 << 25) | (((_d) & 0x7) << 22))


#define rCCC_DVFM_CFG	            (*(volatile uint32_t *)(CCC_PWRCTL_BASE + 0x040))
#define rCCC_DVFM_SCR		    (*(volatile uint64_t *)(CCC_PWRCTL_BASE + 0x048))
#define rCCC_DVFM_CFG_SEL	    (*(volatile uint32_t *)(CCC_PWRCTL_BASE + 0x050))
#define rCCC_DVFM_DLY		    (*(volatile uint32_t *)(CCC_PWRCTL_BASE + 0x058))

#define rCCC_PLL_CFG2		    (*(volatile uint32_t *)(CCC_PWRCTL_BASE + 0x0F8))
#define CCC_PLL_CFG2_RELOCKBYS2_S4_MASK	(0x6)
#define CCC_PLL_CFG2_RELOCKBYS2_S4_DIV2	(0x6)

#define kDVFM_STATE_BYPASS		(0)
#define kDVFM_STATE_SECUREROM		(kDVFM_STATE_BYPASS + 1)
#define kDVFM_STATE_IBOOT		(kDVFM_STATE_BYPASS + 2)
#define kDVFM_STATE_V0			(kDVFM_STATE_IBOOT + 0)
#define kDVFM_STATE_V1			(kDVFM_STATE_IBOOT + 1)
#define kDVFM_STATE_V2			(kDVFM_STATE_IBOOT + 2)
#define kDVFM_STATE_V3			(kDVFM_STATE_IBOOT + 3)
#define kDVFM_STATE_V4			(kDVFM_STATE_IBOOT + 4)

#if SUB_TARGET_N51 || SUB_TARGET_N53 || SUB_TARGET_J85 || SUB_TARGET_J86 || SUB_TARGET_J87 || SUB_TARGET_J85M || SUB_TARGET_J86M || SUB_TARGET_J87M
#define kDVFM_STATE_VMAX		(kDVFM_STATE_V4)
#define kDVFM_STATE_VNOM		(kDVFM_STATE_V3)
#elif SUB_TARGET_J34 || SUB_TARGET_J34M || SUB_TARGET_J71 || SUB_TARGET_J72 || SUB_TARGET_J73
#define kDVFM_STATE_VMAX		(kDVFM_STATE_V4)
#define kDVFM_STATE_VNOM		(kDVFM_STATE_V2)
#else
#define kDVFM_STATE_VMAX		(kDVFM_STATE_V3)
#define kDVFM_STATE_VNOM		(kDVFM_STATE_V2)
#endif

#define kDVFM_STATE_VBOOST		(kDVFM_STATE_VMAX)

#define kDVFM_STATE_IBOOT_CNT		(kDVFM_STATE_VMAX + 1)

#define kVOLTAGE_STATES1_COUNT      	(kDVFM_STATE_VMAX - kDVFM_STATE_V0 + 1)
#define kVOLTAGE_STATES1_SIZE       	(kVOLTAGE_STATES1_COUNT*2)  // Minimum size of voltage-state1 property in devicetree

#define CCC_DVFM_STATE_COUNT	(8)
#define rCCC_DVFM_ST(_n)	    (*(volatile uint64_t *)(CCC_PWRCTL_BASE + 0x068 + ((_n) * 8)))

#define	 CCC_DVFM_ST_PLL_P(_p)	    (((_p) & 0x1F) << 13)
#define	 CCC_DVFM_ST_PLL_M(_m)	    (((_m) & 0x1FF) << 4)
#define	 CCC_DVFM_ST_PLL_S(_s)	    (((_s) & 0xF) << 0)
#define	 CCC_DVFM_ST_SAFE_VOL_SHIFT (56)
#define	 CCC_DVFM_ST_VOLADJ0(_adj0) ((((uint64_t)_adj0) & 0x3F) << 30)
#define	 CCC_DVFM_ST_VOLADJ1(_adj1) ((((uint64_t)_adj1) & 0x3F) << 36)
#define	 CCC_DVFM_ST_VOLADJ2(_adj2) ((((uint64_t)_adj2) & 0x3F) << 42)
#define	 CCC_DVFM_ST_SAFEVOL(_svol) ((((uint64_t)_svol) & 0xFF) << CCC_DVFM_ST_SAFE_VOL_SHIFT)

#define  rCCC_EFUSE_DVFM(_n)	(*(volatile uint32_t *)(CCC_PWRCTL_BASE + 0x0a8 + ((_n) * 8)))

#define	 CCC_DVFM_CFG_TEMPTHRES0(_tt0)	(((_tt0) & 0x7F) << 0)
#define	 CCC_DVFM_CFG_TEMPTHRES1(_tt1)	(((_tt1) & 0x7F) << 7)
#define	 CCC_DVFM_CFG_TEMPOFFST0(_to0)	(((_to0) & 0x7F) << 14)
#define	 CCC_DVFM_CFG_TEMPOFFST1(_to1)	(((_to1) & 0x7F) << 21)

#define rCCC_PWRCTRL_MSTR_PLLCTL    (*(volatile uint32_t *)(CCC_PWRCTL_BASE + 0x0B8))

#define rCCC_PWRCTRL_PLL_SCR0	    (*(volatile uint32_t *)(CCC_PWRCTL_BASE + 0x0C0))

#define rCCC_PWRCTRL_PLL_SCR1	    (*(volatile uint32_t *)(CCC_PWRCTL_BASE + 0x0C8))


#define rCCC_THRM0_CTL0             (*(volatile uint32_t*)(CCC_CPM_THERMAL_BASE + 0x00))
#define rCCC_THRM0_CTL1             (*(volatile uint32_t*)(CCC_CPM_THERMAL_BASE + 0x08))
#define rCCC_THRM0_CTL2             (*(volatile uint32_t*)(CCC_CPM_THERMAL_BASE + 0x10))
#define rCCC_THRM0_CTL3             (*(volatile uint32_t*)(CCC_CPM_THERMAL_BASE + 0x18))
#define rCCC_THRM0_RD_BK0           (*(volatile uint32_t*)(CCC_CPM_THERMAL_BASE + 0x20))
#define rCCC_THRM0_RD_BK1           (*(volatile uint32_t*)(CCC_CPM_THERMAL_BASE + 0x28))
#define rCCC_THRM0_SUM              (*(volatile uint32_t*)(CCC_CPM_THERMAL_BASE + 0x30))
#define rCCC_THRM0_SUM_CNT          (*(volatile uint32_t*)(CCC_CPM_THERMAL_BASE + 0x38))
#define rCCC_THRM1_CTL0             (*(volatile uint32_t*)(CCC_CPM_THERMAL_BASE + 0x40))
#define rCCC_THRM1_CTL1             (*(volatile uint32_t*)(CCC_CPM_THERMAL_BASE + 0x48))
#define rCCC_THRM1_CTL2             (*(volatile uint32_t*)(CCC_CPM_THERMAL_BASE + 0x50))
#define rCCC_THRM1_CTL3             (*(volatile uint32_t*)(CCC_CPM_THERMAL_BASE + 0x58))
#define rCCC_THRM1_RD_BK0           (*(volatile uint32_t*)(CCC_CPM_THERMAL_BASE + 0x60))
#define rCCC_THRM1_RD_BK1           (*(volatile uint32_t*)(CCC_CPM_THERMAL_BASE + 0x68))
#define rCCC_THRM1_SUM              (*(volatile uint32_t*)(CCC_CPM_THERMAL_BASE + 0x70))
#define rCCC_THRM1_SUM_CNT          (*(volatile uint32_t*)(CCC_CPM_THERMAL_BASE + 0x78))
#define rCCC_THRM_FSCTL             (*(volatile uint32_t*)(CCC_CPM_THERMAL_BASE + 0x80))
#define rCCC_THRM_FSTRIP_T0         (*(volatile uint32_t*)(CCC_CPM_THERMAL_BASE + 0x90))
#define rCCC_THRM_FSTRIP_T1         (*(volatile uint32_t*)(CCC_CPM_THERMAL_BASE + 0x98))
#define rCCC_THRM_FSASRT_CNT0       (*(volatile uint32_t*)(CCC_CPM_THERMAL_BASE + 0xA0))
#define rCCC_THRM_FSASRT_CNT1       (*(volatile uint32_t*)(CCC_CPM_THERMAL_BASE + 0xA8))
#define rCCC_THRM_FSDE_ASRT_CNT0    (*(volatile uint32_t*)(CCC_CPM_THERMAL_BASE + 0xB0))
#define rCCC_THRM_FSDE_ASRT_CNT1    (*(volatile uint32_t*)(CCC_CPM_THERMAL_BASE + 0xB8))
#define rCCC_THRM_FSHOT0_TSTAMP     (*(volatile uint64_t*)(CCC_CPM_THERMAL_BASE + 0xC0))
#define rCCC_THRM_FSHOT1_TSTAMP     (*(volatile uint64_t*)(CCC_CPM_THERMAL_BASE + 0xC8))
#define rCCC_DVFM_EFUSE_TADC0       (*(volatile uint32_t*)(CCC_CPM_THERMAL_BASE + 0xD0))
#define rCCC_DVFM_EFUSE_TADC1       (*(volatile uint32_t*)(CCC_CPM_THERMAL_BASE + 0xD8))
#define rCCC_THEM_EFUSE_TADC0       (*(volatile uint32_t*)(CCC_CPM_THERMAL_BASE + 0xE0))
#define rCCC_THEM_EFUSE_TADC1       (*(volatile uint32_t*)(CCC_CPM_THERMAL_BASE + 0xE8))
#define rCCC_DVFM_FSHOT_IDX         (*(volatile uint32_t*)(CCC_CPM_THERMAL_BASE + 0xF0))


#define	 CCC_PLL_P_SHIFT	    (9)
#define	 CCC_PLL_P_MASK		    (0x1F)
#define	 CCC_PLL_M_SHIFT	    (18)
#define	 CCC_PLL_M_MASK		    (0x1FF)
#define	 CCC_PLL_S_SHIFT	    (0)
#define	 CCC_PLL_S_MASK		    (0xF)
#define	 CCC_PLL_M(_m)		    (((_m) & CCC_PLL_M_MASK) << CCC_PLL_M_SHIFT)
#define	 CCC_PLL_P(_p)		    (((_p) & CCC_PLL_P_MASK) << CCC_PLL_P_SHIFT)
#define	 CCC_PLL_S(_s)		    (((_s) & CCC_PLL_S_MASK) << CCC_PLL_S_SHIFT)
#define	 CCC_PLL_ENABLE		    (1 << 31)
#define	 CCC_PLL_LOCKED		    (1 << 29)
#define	 CCC_PLL_LOAD		    (1 << 27)


////////////////////////////////////////////////////////////
//
//  Externs
//
////////////////////////////////////////////////////////////

extern void pmgr_update_device_tree(DTNode *pmgr_node);
extern void pmgr_gfx_update_device_tree(DTNode *gfx_node);
void sochot_pmgr_update_device_tree(DTNode *node);
void sochot_ccc_update_device_tree(DTNode *node);
void temp_sensor_pmgr_update_device_tree(DTNode *node);
void temp_sensor_ccc_update_device_tree(DTNode *node);
void thermal_init(void);

#endif /* ! __PLATFORM_SOC_PMGR_H */
