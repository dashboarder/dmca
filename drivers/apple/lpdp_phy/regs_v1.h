/*
 * Copyright (C) 2013 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */

#ifndef _LPDP_PHY_REGS_H
#define _LPDP_PHY_REGS_H
						
#define	rLPDP_GEN_VERSION				(*(volatile u_int32_t *)(LPDP_PHY_BASE_ADDR + 0x00000))
#define	rLPDP_GEN_SEQ_0					(*(volatile u_int32_t *)(LPDP_PHY_BASE_ADDR + 0x00004))

#define	rLPDP_GEN_SEQ_1					(*(volatile u_int32_t *)(LPDP_PHY_BASE_ADDR + 0x00008))
#define	 LPDP_GEN_SEQ_1_START_COUNT(n)			((n) << 8)
#define	 LPDP_GEN_SEQ_1_SETUP_COUNT(n)			((n) << 0)

#define	rLPDP_GEN_SEQ_2					(*(volatile u_int32_t *)(LPDP_PHY_BASE_ADDR + 0x0000c))
#define	rLPDP_GEN_SEQ_3					(*(volatile u_int32_t *)(LPDP_PHY_BASE_ADDR + 0x00010))

#define	rLPDP_GEN_SEQ_4					(*(volatile u_int32_t *)(LPDP_PHY_BASE_ADDR + 0x00014))
#define	 LPDP_GEN_SEQ_4_FINISH_COUNT(n)			((n) << 8)
#define	 LPDP_GEN_SEQ_4_UPDATE_COUNT(n)			((n) << 0)

#define	rLPDP_GEN_LINE_COUNTER				(*(volatile u_int32_t *)(LPDP_PHY_BASE_ADDR + 0x00018))
#define	rLPDP_GEN_SPARE_CONFIG				(*(volatile u_int32_t *)(LPDP_PHY_BASE_ADDR + 0x0001c))
#define	rLPDP_GEN_SPARE_STATUS				(*(volatile u_int32_t *)(LPDP_PHY_BASE_ADDR + 0x00020))
#define	rLPDP_GEN_AUX_DRIVER				(*(volatile u_int32_t *)(LPDP_PHY_BASE_ADDR + 0x00024))
#define	 LPDP_GEN_AUX_DRIVER_EXT_SEL			(1 << 15)

#define	rLPDP_PHY_GEN_CTRL				(*(volatile u_int32_t *)(LPDP_PHY_BASE_ADDR + 0x00040))
#define	 LPDP_PHY_GEN_CTRL_SEQ_OW			(1 << 7)
#define	 LPDP_PHY_GEN_CTRL_SLEEP_SW			(1 << 6)
#define	 LPDP_PHY_GEN_CTRL_LANE_PD_OW			(1 << 2)
#define	 LPDP_PHY_GEN_CTRL_BIAS_PWRDN			(1 << 0)

#define	rLPDP_PHY_LANE(n)				(*(volatile u_int32_t *)(LPDP_PHY_BASE_ADDR + 0x00044 + ((n) << 2)))
#define	rLPDP_PHY_LANE_0				(*(volatile u_int32_t *)(LPDP_PHY_BASE_ADDR + 0x00044))
#define	 LPDP_PHY_LANE_CM_PRECHARGE_SLEEP(n)		((n) << 13)
#define	 LPDP_PHY_LANE_CM_PRECHARGE_NORMAL(n)		((n) << 11)
#define	 LPDP_PHY_LANE_LDO_PWRDN			(1 << 10)
#define	 LPDP_PHY_LANE_PWRDN				(1 << 9)
#define	 LPDP_PHY_LANE_HI_Z				(1 << 8)
#define	 LPDP_PHY_LANE_DE_ENABLE			(1 << 7)
#define	 LPDP_PHY_LANE_PASSMOS_CTRL(n)			((n) << 4)
#define	 LPDP_PHY_LANE_VREG_ADJ(n)			((n) << 0)
#define	 LPDP_PHY_LANE_x_VREG_ADJ_MAX			0xF
#define	 LPDP_PHY_LANE_x_VREG_ADJ_SHIFT			0
#define	 LPDP_PHY_LANE_x_VREG_ADJ_MASK			(0xF << LPDP_PHY_LANE_x_VREG_ADJ_SHIFT)
#define	 LPDP_PHY_LANE_VREG_ADJ_420_mV			(15)
#define	 LPDP_PHY_LANE_VREG_ADJ_400_mV			(14)
#define	 LPDP_PHY_LANE_VREG_ADJ_380_mV			(13)
#define	 LPDP_PHY_LANE_VREG_ADJ_360_mV			(12)
#define	 LPDP_PHY_LANE_VREG_ADJ_340_mV			(11)
#define	 LPDP_PHY_LANE_VREG_ADJ_320_mV			(10)
#define	 LPDP_PHY_LANE_VREG_ADJ_300_mV			(9)
#define	 LPDP_PHY_LANE_VREG_ADJ_280_mV			(8)
#define	 LPDP_PHY_LANE_VREG_ADJ_260_mV			(7)
#define	 LPDP_PHY_LANE_VREG_ADJ_240_mV			(6)
#define	 LPDP_PHY_LANE_VREG_ADJ_220_mV			(5)
#define	 LPDP_PHY_LANE_VREG_ADJ_200_mV			(4)
#define	 LPDP_PHY_LANE_VREG_ADJ_180_mV			(3)
#define	 LPDP_PHY_LANE_VREG_ADJ_160_mV			(2)
#define	 LPDP_PHY_LANE_VREG_ADJ_140_mV			(1)
#define	 LPDP_PHY_LANE_VREG_ADJ_120_mV			(0)
#define	 LPDP_PHY_LANE_x_FULL_DISABLE			(LPDP_PHY_LANE_LDO_PWRDN | \
							 LPDP_PHY_LANE_PWRDN |     \
							 LPDP_PHY_LANE_HI_Z)

#define	rLPDP_PHY_LANE_1				(*(volatile u_int32_t *)(LPDP_PHY_BASE_ADDR + 0x00048))
#define	rLPDP_PHY_LANE_2				(*(volatile u_int32_t *)(LPDP_PHY_BASE_ADDR + 0x0004c))
#define	rLPDP_PHY_LANE_3				(*(volatile u_int32_t *)(LPDP_PHY_BASE_ADDR + 0x00050))
#define	rLPDP_PHY_PRBS_0				(*(volatile u_int32_t *)(LPDP_PHY_BASE_ADDR + 0x00054))
#define	rLPDP_PHY_PRBS_1				(*(volatile u_int32_t *)(LPDP_PHY_BASE_ADDR + 0x00058))
#define	rLPDP_PHY_PRBS_2				(*(volatile u_int32_t *)(LPDP_PHY_BASE_ADDR + 0x0005c))
#define	rLPDP_PHY_PRBS_3				(*(volatile u_int32_t *)(LPDP_PHY_BASE_ADDR + 0x00060))
#define	rLPDP_PHY_PRBS_ERR				(*(volatile u_int32_t *)(LPDP_PHY_BASE_ADDR + 0x00064))
#define	rLPDP_PHY_TEST					(*(volatile u_int32_t *)(LPDP_PHY_BASE_ADDR + 0x00068))
#define	rLPDP_PHY_BIST					(*(volatile u_int32_t *)(LPDP_PHY_BASE_ADDR + 0x0006c))

#define	rLPDP_PHY_CAL_CTRL				(*(volatile u_int32_t *)(LPDP_PHY_BASE_ADDR + 0x00070))
#define	 LPDP_PHY_CAL_CTRL_AUTOZERO_BYP			(1 << 3)
#define	 LPDP_PHY_CAL_CTRL_TYPE_SEL_UP			(1 << 2)
#define	 LPDP_PHY_CAL_CTRL_RST				(1 << 1)
#define	 LPDP_PHY_CAL_CTRL_PWRDN			(1 << 0)

#define	rLPDP_PHY_CAL_EXT				(*(volatile u_int32_t *)(LPDP_PHY_BASE_ADDR + 0x00074))
#define	 LPDP_PHY_CAL_EXT_CTRL_SEL			(1 << 15)

#define	rLPDP_PHY_CAL_RESULT				(*(volatile u_int32_t *)(LPDP_PHY_BASE_ADDR + 0x00078))
#define	 LPDP_PHY_CAL_RESULT_COMPLETE			(1 << 15)
#define	 LPDP_PHY_CAL_RESULT_CO_PD			(1 << 14)
#define	 LPDP_PHY_CAL_RESULT_PD(n)			(((n) >> 8) & 0x3F)
#define	 LPDP_PHY_CAL_RESULT_CO_PU			(1 << 7)
#define	 LPDP_PHY_CAL_RESULT_PU(n)			(n & 0x7F) 
#define	rLPDP_PHY_AUX_CTRL				(*(volatile u_int32_t *)(LPDP_PHY_BASE_ADDR + 0x0007C))
#define	 LPDP_PHY_AUX_CTRL_PWRDN			(1 << 4)
#define	 LPDP_PHY_AUX_CTRL_VREG_ADJ_SHIFT		0
#define	 LPDP_PHY_AUX_CTRL_VREG_ADJ_MASK		(0xF << LPDP_PHY_AUX_CTRL_VREG_ADJ_SHIFT)
#define	 LPDP_PHY_AUX_CTRL_VREG_ADJ_MAX			0xF



#define	rLPDP_PLL_GEN					(*(volatile u_int32_t *)(LPDP_PHY_BASE_ADDR + 0x00080))
#define	 LPDP_PLL_GEN_TEST_SEL_VSSA			(0 << 10)
#define	 LPDP_PLL_GEN_TEST_SEL_VDD_SOC			(1 << 10)
#define	 LPDP_PLL_GEN_TEST_SEL_VREG_1VA			(2 << 10)
#define	 LPDP_PLL_GEN_TEST_SEL_PLL_LOCK_OUT		(3 << 10)
#define	 LPDP_PLL_GEN_TEST_SEL_CP_OUT			(4 << 10)
#define	 LPDP_PLL_GEN_TEST_SEL_VCO_TOP			(5 << 10)
#define	 LPDP_PLL_GEN_TEST_SEL_VREF_CAL			(6 << 10)
#define	 LPDP_PLL_GEN_TEST_SEL_VBTST			(7 << 10)
#define	 LPDP_PLL_GEN_TEST_EN				(1 << 9)
#define	 LPDP_PLL_GEN_GCLK_SEL_VCLK			(1 << 8)
#define	 LPDP_PLL_GEN_GCLK_DIV(n)			(n << 4)
#define	 LPDP_PLL_GEN_STOP_CLK				(1 << 3)
#define	 LPDP_PLL_GEN_BYPASS				(1 << 2)
#define	 LPDP_PLL_GEN_RST				(1 << 1)
#define	 LPDP_PLL_GEN_PWRDN				(1 << 0)

#define	rLPDP_PLL_CLK					(*(volatile u_int32_t *)(LPDP_PHY_BASE_ADDR + 0x00084))
#define	 LPDP_PLL_CLK_FLIN_SEL				(1 << 12)
#define	 LPDP_PLL_CLK_BG_START_SEL			(1 << 11)
#define	 LPDP_PLL_CLK_PFDCP_RST_SEL			(1 << 10)
#define	 LPDP_PLL_CLK_VCLK_SEL_VCO			(1 << 9)
#define	 LPDP_PLL_CLK_VCO_BLK_RSTPD			(1 << 8)
#define	 LPDP_PLL_CLK_VCO_BLK_SCLK			(1 << 7)
#define	 LPDP_PLL_CLK_VCO_BLK_VCLK			(1 << 6)
#define	 LPDP_PLL_CLK_VCO_RCTRL_SEL_ENABLE		(1 << 5)
#define	 LPDP_PLL_CLK_VCO_RCTRL_OW_SHIFT		2
#define	 LPDP_PLL_CLK_VCO_RCTRL_OW_MASK			(0x7 << LPDP_PLL_CLK_VCO_RCTRL_OW_SHIFT)
#define	 LPDP_PLL_CLK_VCO_PCTRL(n)			((n) << 0)

#define	rLPDP_PLL_IDIV					(*(volatile u_int32_t *)(LPDP_PHY_BASE_ADDR + 0x00088))
#define	 LPDP_PLL_IDIV_UPDT				(1 << 14)
#define	 LPDP_PLL_IDIV_FB(n)				(n << 5)
#define	 LPDP_PLL_IDIV_PRE(n)				(n << 0)
#define	 LPDP_PLL_IDIV_FB_SHIFT				5
#define	 LPDP_PLL_IDIV_FB_MASK				(0x1FF << LPDP_PLL_IDIV_FB_SHIFT)
#define	 LPDP_PLL_IDIV_PRE_SHIFT			0
#define	 LPDP_PLL_IDIV_PRE_MASK				(0x1F << LPDP_PLL_IDIV_PRE_SHIFT)


#define	rLPDP_PLL_CP					(*(volatile u_int32_t *)(LPDP_PHY_BASE_ADDR + 0x0008C))
#define	 LPDP_PLL_CP_BOOST_MODE				(1 << 14)
#define	 LPDP_PLL_CP_BOOST_EN				(1 << 13)
#define	 LPDP_PLL_CP_ENABLE				(1 << 12)
#define	 LPDP_PLL_CP_RSTN				(1 << 11)
#define	 LPDP_PLL_CP_LIN_EN				(1 << 10)
#define	 LPDP_PLL_CP_R_SET(n)				(n << 3)
#define	 LPDP_PLL_CP_I_SET(n)				(n << 0)

#define	rLPDP_PLL_LOCK					(*(volatile u_int32_t *)(LPDP_PHY_BASE_ADDR + 0x00090))
#define	 LPDP_PLL_LOCK_OUT_OFF				(0 << 9)
#define	 LPDP_PLL_LOCK_OUT_ON				(1 << 9)
#define	 LPDP_PLL_LOCK_RST_OFF				(0 << 8)
#define	 LPDP_PLL_LOCK_RST_ON				(1 << 8)
#define	 LPDP_PLL_LOCK_POR_OFF				(0 << 7)
#define	 LPDP_PLL_LOCK_POR_ON				(1 << 7)
#define	 LPDP_PLL_LOCK_DIV(n)				((n) << 0)

#define	rLPDP_PLL_VCO					(*(volatile u_int32_t *)(LPDP_PHY_BASE_ADDR + 0x00094))
#define	 LPDP_PLL_VCO_BUF_PWRDN_OFF			(0 << 14)
#define	 LPDP_PLL_VCO_BUF_PWRDN_ON			(1 << 14)
#define	 LPDP_PLL_VCO_DEC_BYPASS_OFF			(0 << 13)
#define	 LPDP_PLL_VCO_DEC_BYPASS_ON			(1 << 13)
#define	 LPDP_PLL_VCO_REG_PWRDN_OFF			(0 << 12)
#define	 LPDP_PLL_VCO_REG_PWRDN_ON			(1 << 12)
#define	 LPDP_PLL_VCO_REG_BYPASS_OFF			(0 << 11)
#define	 LPDP_PLL_VCO_REG_BYPASS_ON			(1 << 11)
#define	 LPDP_PLL_VCO_VREG_ADJ(n)			((n) << 8)
#define	 LPDP_PLL_VCO_K_VCO_CTRL(n)			((n) << 4)
#define	 LPDP_PLL_VCO_RESDIV_SEL(n)			((n) << 1)
#define	 LPDP_PLL_VCO_RESDIV_VREF_EN_OFF		(0 << 0)
#define	 LPDP_PLL_VCO_RESDIV_VREF_EN_ON			(1 << 0)

#define	rLPDP_PLL_SDM					(*(volatile u_int32_t *)(LPDP_PHY_BASE_ADDR + 0x00098))
#define	rLPDP_PLL_SSC					(*(volatile u_int32_t *)(LPDP_PHY_BASE_ADDR + 0x0009c))
#define	rLPDP_PLL_FMC					(*(volatile u_int32_t *)(LPDP_PHY_BASE_ADDR + 0x000a0))
#define	rLPDP_PLL_SPARE					(*(volatile u_int32_t *)(LPDP_PHY_BASE_ADDR + 0x000a4))
#endif //_LPDP_PHY_REGS_H
