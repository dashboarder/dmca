/*
 * Copyright (C) 2013-2014 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */

#ifndef _LPDP_PHY_REGS_V2_H
#define _LPDP_PHY_REGS_V2_H
						
#define	rLPDP_GEN_VERSION				(0x00000)
#define	rLPDP_GEN_SEQ_0					(0x00004)

#define	rLPDP_GEN_SEQ_1					(0x00008)
#define	 LPDP_GEN_SEQ_1_START_COUNT(n)			((n) << 8)
#define	 LPDP_GEN_SEQ_1_SETUP_COUNT(n)			((n) << 0)

#define	rLPDP_GEN_SEQ_2					(0x0000c)
#define	rLPDP_GEN_SEQ_3					(0x00010)

#define	rLPDP_GEN_SEQ_4					(0x00014)
#define	 LPDP_GEN_SEQ_4_HOLD_COUNT(n)			((n) << 8)
#define	 LPDP_GEN_SEQ_4_UPDATE_COUNT(n)			((n) << 0)

#define	rLPDP_GEN_SEQ_5					(0x00018)
#define	 LPDP_GEN_SEQ_5_FINISH_COUNT(n)			((n) <<	8)
#define	 LPDP_GEN_SEQ_5_WAKEUP_COUNT(n)			((n) << 0)

#define	rLPDP_GEN_LINE_COUNTER				(0x0001c)
#if LPDP_PHY_VERSION < 2
#define	rLPDP_GEN_DISPLAY_SPLIT				(0x00020)
#define	rLPDP_GEN_SPARE_CONFIG				(0x00024)
#define	rLPDP_GEN_SPARE_STATUS				(0x00028)
#elif LPDP_PHY_VERSION == 2
#define	rLPDP_GEN_SPARE_CONFIG				(0x00020)
#define	rLPDP_GEN_SPARE_STATUS				(0x00024)
#endif

#define	rLPDP_PHY_GEN_CTRL				(0x00100)
#define	 LPDP_PHY_GEN_CTRL_BIAS_START_PULSE_EXT		(1 << 12)
#define	 LPDP_PHY_GEN_CTRLBIAS_EXT_START_SEL		(1 << 11)
#define	 LPDP_PHY_GEN_CTRL_WAKEUP_SW			(1 << 10)
#define	 LPDP_PHY_GEN_CTRL_FIFO_RST_MASK		(1 << 9)
#define	 LPDP_PHY_GEN_CTRL_FIFO_RST_HW			(1 << 8)	
#define	 LPDP_PHY_GEN_CTRL_SEQ_OW			(1 << 7)
#define	 LPDP_PHY_GEN_CTRL_SLEEP_SW			(1 << 6)
#define	 LPDP_PHY_GEN_CTRL_LANE_PD_OW			(1 << 2)
#define	 LPDP_PHY_GEN_CTRL_BIAS_PWRDN			(1 << 0)

#define	rLPDP_PHY_LANE(n)				(0x00104 + ((n) << 2))
#define	rLPDP_PHY_LANE_0				(0x00104)
#define	 LPDP_PHY_LANE_CM_PRECHARGE_SLEEP(n)		((n) << 13)
#define	 LPDP_PHY_LANE_CM_PRECHARGE_NORMAL(n)		((n) << 11)
#define	 LPDP_PHY_LANE_LDO_PWRDN			(1 << 10)
#define	 LPDP_PHY_LANE_PWRDN				(1 << 9)
#define	 LPDP_PHY_LANE_HI_Z				(1 << 8)
#define	 LPDP_PHY_LANE_VREG_ADJ(n)			((n) << 0)
#define	 LPDP_PHY_LANE_x_VREG_ADJ_MAX			0xF
#define	 LPDP_PHY_LANE_x_VREG_ADJ_SHIFT			0
#define	 LPDP_PHY_LANE_x_VREG_ADJ_MASK			(0xF << LPDP_PHY_LANE_x_VREG_ADJ_SHIFT)

#if LPDP_PHY_VERSION < 2
#define	 LPDP_PHY_LANE_VREG_ADJ_570_mV			(15)
#define	 LPDP_PHY_LANE_VREG_ADJ_540_mV			(14)
#define	 LPDP_PHY_LANE_VREG_ADJ_510_mV			(13)
#define	 LPDP_PHY_LANE_VREG_ADJ_480_mV			(12)
#define	 LPDP_PHY_LANE_VREG_ADJ_450_mV			(11)
#define	 LPDP_PHY_LANE_VREG_ADJ_420_mV			(10)
#define	 LPDP_PHY_LANE_VREG_ADJ_390_mV			(9)
#define	 LPDP_PHY_LANE_VREG_ADJ_360_mV			(8)
#define	 LPDP_PHY_LANE_VREG_ADJ_330_mV			(7)
#define	 LPDP_PHY_LANE_VREG_ADJ_300_mV			(6)
#define	 LPDP_PHY_LANE_VREG_ADJ_270_mV			(5)
#define	 LPDP_PHY_LANE_VREG_ADJ_240_mV			(4)
#define	 LPDP_PHY_LANE_VREG_ADJ_210_mV			(3)
#define	 LPDP_PHY_LANE_VREG_ADJ_180_mV			(2)
#define	 LPDP_PHY_LANE_VREG_ADJ_150_mV			(1)
#define	 LPDP_PHY_LANE_VREG_ADJ_120_mV			(0)
#define	 LPDP_PHY_LANE_x_FULL_DISABLE			(LPDP_PHY_LANE_LDO_PWRDN | \
							 LPDP_PHY_LANE_PWRDN)
#elif LPDP_PHY_VERSION == 2
#define	 LPDP_PHY_LANE_VREG_ADJ_530_mV			(7)
#define	 LPDP_PHY_LANE_VREG_ADJ_450_mV			(6)
#define	 LPDP_PHY_LANE_VREG_ADJ_390_mV			(5)
#define	 LPDP_PHY_LANE_VREG_ADJ_310_mV			(4)
#define	 LPDP_PHY_LANE_VREG_ADJ_250_mV			(3)
#define	 LPDP_PHY_LANE_VREG_ADJ_210_mV			(2)
#define	 LPDP_PHY_LANE_VREG_ADJ_190_mV			(1)
#define	 LPDP_PHY_LANE_VREG_ADJ_170_mV			(0)
#define	 LPDP_PHY_LANE_PWRDN_SML			(1 << 7)
#define	 LPDP_PHY_LANE_x_FULL_DISABLE			(LPDP_PHY_LANE_LDO_PWRDN | \
							 LPDP_PHY_LANE_PWRDN | \
							 LPDP_PHY_LANE_PWRDN_SML)
#endif

#define	rLPDP_PHY_LANE_1				(0x00108)
#define	rLPDP_PHY_LANE_2				(0x0010c)
#define	rLPDP_PHY_LANE_3				(0x00110)
#define	rLPDP_PHY_PRBS_0				(0x00114)
#define	rLPDP_PHY_PRBS_1				(0x00118)
#define	rLPDP_PHY_PRBS_2				(0x0011c)
#define	rLPDP_PHY_PRBS_3				(0x00120)
#define	rLPDP_PHY_PRBS_ERR				(0x00124)
#define	rLPDP_PHY_TEST					(0x00128)

#define	rLPDP_PHY_BIST					(0x0012c)
#define	 LPDP_PHY_BIST_DEMUX_RST			(1 << 8)
#define	 LPDP_PHY_BIST_LOOPBACK_SRC_SEL(n)		((n) << 3)
#define	 LPDP_PHY_BIST_LOOPBACK_SRC_SEL_MASK		(0x1f)
#define	 LPDP_PHY_BIST_NONE				0x0
#define	 LPDP_PHY_BIST_LANE0				0x1
#define	 LPDP_PHY_BIST_LANE1				0x2
#define	 LPDP_PHY_BIST_LANE2				0x4
#define	 LPDP_PHY_BIST_LANE3				0x8
#define	 LPDP_PHY_BIST_DEMUX				0x10
#define	 LPDP_PHY_BIST_DEMUX_SEL(n)			((n) << 1)
#define	 LPDP_PHY_BIST_SEL0				0x0
#define	 LPDP_PHY_BIST_SEL1				0x1
#define	 LPDP_PHY_BIST_SEL2				0x2
#define	 LPDP_PHY_BIST_SEL3				0x3
#define	 LPDP_PHY_BIST_ENABLE				(1 << 0)

#define	rLPDP_PHY_CAL					(0x00130)
#define	 LPDP_PHY_CAL_COMP_OUT				(1 << 8)
#define	 LPDP_PHY_CAL_FAIL				(1 << 7)
#define	 LPDP_PHY_CAL_DONE				(1 << 6)
#define	 LPDP_PHY_CAL_AZ_BYP				(1 << 5)
#define	 LPDP_PHY_CAL_MODE_FORCE(n)			((n) << 3)
#define	 LPDP_PHY_CAL_FORCE_EN				(1 << 2)
#define	 LPDP_PHY_CAL_START				(1 << 1)
#define	 LPDP_PHY_CAL_RESET_N				(1 << 0)

#define	rLPDP_PHY_CAL_PD				(0x00134)
#define	 LPDP_PHY_CAL_PD_CODE_ERR			(1 << 13)
#define	 LPDP_PHY_CAL_PD_EXT_SEL			(1 << 12)
#define	 LPDP_PHY_CAL_PD_CODE_FSM(n)			((n) << 6)
#define	 LPDP_PHY_CAL_PD_CODE_EXT(n)			((n) << 0)

#define	rLPDP_PHY_CAL_PU				(0x00138)
#define	 LPDP_PHY_CAL_P_CODE_ERR			(1 << 13)
#define	 LPDP_PHY_CAL_P_EXT_SEL				(1 << 12)
#define	 LPDP_PHY_CAL_P_CODE_FSM(n)			((n) << 6)
#define	 LPDP_PHY_CAL_P_CODE_EXT(n)			((n) << 0)

#define	rLPDP_PHY_CAL_TX				(0x0013C)
#define	 LPDP_PHY_CAL_TX_EXT_SEL			(1 << 12)
#define  LPDP_PHY_CAL_TX_CODE_FSM_SHIFT			(6)
#define	 LPDP_PHY_CAL_TX_CODE_FSM(n)			((n) << LPDP_PHY_CAL_TX_CODE_FSM_SHIFT)
#define  LPDP_PHY_CAL_TX_CODE_FSM_MASK			((0x3F) << LPDP_PHY_CAL_TX_CODE_FSM_SHIFT)
#define	 LPDP_PHY_CAL_TX_CODE_EXT(n)			((n) << 0)

#define	rLPDP_PHY_AUX_CTRL				(0x00140)
#define	LPDP_PHY_AUX_CTRL_TX_AUX_ENABLE			(1 << 5)
#define	LPDP_PHY_AUX_CTRL_PWRDN				(1 << 4)
#define	LPDP_PHY_AUX_CTRL_VREG_ADJ(n)			((n) << 0)
#define	 LPDP_PHY_AUX_CTRL_VREG_ADJ_SHIFT		0
#define	 LPDP_PHY_AUX_CTRL_VREG_ADJ_MASK		(0xF << LPDP_PHY_AUX_CTRL_VREG_ADJ_SHIFT)
#define	 LPDP_PHY_AUX_CTRL_VREG_ADJ_MAX			0xF

#define	rLPDP_PHY_AUX_DRIVER				(0x00144)
#define	LPDP_PHY_AUX_DRIVER_EXT_SEL			(1 << 15)
#define	LPDP_PHY_AUX_DRIVER_EXT_PD(n)			((n) << 8)
#define	LPDP_PHY_AUX_DRIVER_EXT_PU(n)			((n) << 0)

#define	rLPDP_PHY_CLK_CTRL				(0x00148)
#define	 LPDP_PHY_CLK_CTRL_CLKDIV_SHFTEN		(1 << 2)
#define	 LPDP_PHY_CLK_CTRL_CLK_ENABLE			(1 << 1)
#define	 LPDP_PHY_CLK_CTRL_CLKDIV5_RESET		(1 << 0)

#define	rLPDP_PHY_CLK_LDO_CTRL				(0x0014c)
#if LPDP_PHY_VERSION == 2
#define	 LPDP_PHY_CLK_LDO_CTRL_LDOCLK_REF_ADJ(n)	((n) << 15)
#define	 LPDP_PHY_CLK_LDO_CTRL_LDOCLK_PWRDN_SML(n)	((n) << 11)
#define	 LPDP_PHY_CLK_LDO_CTRL_LDOCLK_PWRDN_SML_MAX	(0xf)
#endif
#define	 LPDP_PHY_CLK_LDO_CTRL_LDOCLK_SB_EN		(1 << 10)
#define	 LPDP_PHY_CLK_LDO_CTRL_LDOCLK_BGREF_BYPASS	(1 << 9)
#define	 LPDP_PHY_CLK_LDO_CTRL_LDOCLK_BYPASS		(1 << 8)
#define	 LPDP_PHY_CLK_LDO_CTRL_LDOCLK_VREG_ADJ(n)	((n) << 4)
#define	 LPDP_PHY_CLK_LDO_CTRL_LDOCLK_PWRDN(n)		((n) << 0)
#define	 LPDP_PHY_CLK_LDO_CTRL_LDOCLK_PWRDN_MAX		(0xf)

#define	rLPDP_PHY_PRE_LDO_CTRL				(0x00150)
#define	 LPDP_PHY_PRE_LDO_CTR_LDOPRE_SB_EN		(1 << 10)
#define	 LPDP_PHY_PRE_LDO_CTR_LDOPRE_BGREF_BYPASS	(1 << 9)
#define	 LPDP_PHY_PRE_LDO_CTR_LDOPRE_BYPASS		(1 << 8)
#define	 LPDP_PHY_PRE_LDO_CTR_LDOPRE_VREG_ADJ(n)	((n) << 4)
#define	 LPDP_PHY_PRE_LDO_CTR_LDOPRE_PWRDN(n)		((n) << 0)
#define	 LPDP_PHY_PRE_LDO_CTR_LDOPRE_PWRDN_MAX		(0xf)

#define	rLPDP_PHY_POST_LDO_CTRL				(0x00154)
#define	 LPDP_PHY_POST_LDO_CTRL_LDOPOST_VR_TEST_EN	(1 << 12)
#define	 LPDP_PHY_POST_LDO_CTRL_LDOPOST_LEAKEN		(1 << 11)
#define	 LPDP_PHY_POST_LDO_CTRL_LDOPOST_SB_EN		(1 << 10)
#define	 LPDP_PHY_POST_LDO_CTRL_LDOPOST_BGREF_BYPASS	(1 << 9)
#if LPDP_PHY_VERSION == 2
#define LPDP_PHY_POST_LDO_CTRL_LDOPOST_BYPASS		(1 << 0)
#endif

#define	rLPDP_PHY_AUX_LDO_CTRL				(0x00158)
#if LPDP_PHY_VERSION < 2
#define	 LPDP_PHY_AUX_LDO_CTRL_AUXVREG_BGREF_BYPASS	(1 << 9)
#define	 LPDP_PHY_AUX_LDO_CTRL_AUXVREG_BYPASS		(1 << 8)
#define	 LPDP_PHY_AUX_LDO_CTRL_AUXVREG_ADJ(n)		((n) << 4)
#elif LPDP_PHY_VERSION == 2
#define	 LPDP_PHY_AUX_LDO_CTRL_LDOPOST_REF_ADJ(n)	((n) << 27)
#define	 LPDP_PHY_AUX_LDO_CTRL_LDOPOST_PWRDN_BIG	(1 << 26)
#define	 LPDP_PHY_AUX_LDO_CTRL_LDOPOST_PWRDN_SML	(1 << 25)
#define	 LPDP_PHY_AUX_LDO_CTRL_LDOCLK_VREG_ADJ(n)	((n) <<	21)
#define	 LPDP_PHY_AUX_LDO_CTRL_LDOCLK_REF_ADJ(n)	((n) <<	18)
#define	 LPDP_PHY_AUX_LDO_CTRL_LDOCLK_PWRDN_BIG		(1 << 17)
#define	 LPDP_PHY_AUX_LDO_CTRL_LDOCLK_PWRDN_SML		(1 << 16)
#endif
#define	 LPDP_PHY_AUX_LDO_CTRL_AUX_CM_CTRL(n)		((n) <<	1)
#if LPDP_PHY_VERSION < 2
#define	 LPDP_PHY_AUX_LDO_CTRL_AUXLDO_PWRDN		(1 << 0)
#endif

#define	rLPDP_PHY_DE_CTRL				(0x0015c)
#if LPDP_PHY_VERSION < 2
#define	 LPDP_PHY_DE_CTRL_DE_EN				(1 << 2)
#elif LPDP_PHY_VERSION == 2
#define	 LPDP_PHY_DE_CTRL_DE2_EN			(1 << 6)
#define	 LPDP_PHY_DE_CTRL_DE1_EN			(1 << 2)
#endif
#define	 LPDP_PHY_DE_CTRL_DE_EN_LSB(n)			((n) << 0)

#define	rLPDP_PHY_RESERVED				(0x00160)

#define	rLPDP_PLL_GEN					(0x00180)
#define	 LPDP_PLL_GEN_CLKDIV2_RESET_N			(1 << 15)
#define	 LPDP_PLL_GEN_TEST_SEL(n)			((n) <<	10)
#define	 LPDP_PLL_GEN_VSSA				0x0
#define	 LPDP_PLL_GEN_VDD_SOC				0x1
#define	 LPDP_PLL_GEN_VREG_1VA				0x2
#define	 LPDP_PLL_GEN_PLL_LOCK_OUT			0x3
#define	 LPDP_PLL_GEN_CP_OUT				0x4
#define	 LPDP_PLL_GEN_VCO_TOP				0x5
#define	 LPDP_PLL_GEN_VREF_CAL				0x6
#define	 LPDP_PLL_GEN_VBTST				0x7
#define	 LPDP_PLL_GEN_TEST_EN				(1 << 9)
#define	 LPDP_PLL_GEN_GCLK_SEL				(1 << 8)
#define	 LPDP_PLL_GEN_GCLK_DIV(n)			((n) << 4)
#define	 LPDP_PLL_GEN_GCLK_DIV_MASK			(0xf)
#define	 LPDP_PLL_GEN_STOP_CLK				(1 << 3)
#define	 LPDP_PLL_GEN_BYPASS				(1 << 2)
#define	 LPDP_PLL_GEN_RST				(1 << 1)
#define	 LPDP_PLL_GEN_PWRDN				(1 << 0)

#define	rLPDP_PLL_CLK					(0x00184)
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

#define	rLPDP_PLL_IDIV					(0x00188)
#define	 LPDP_PLL_IDIV_UPDT				(1 << 14)
#define	 LPDP_PLL_IDIV_FB(n)				(n << 5)
#define	 LPDP_PLL_IDIV_PRE(n)				(n << 0)
#define	 LPDP_PLL_IDIV_FB_SHIFT				5
#define	 LPDP_PLL_IDIV_FB_MASK				(0x1FF << LPDP_PLL_IDIV_FB_SHIFT)
#define	 LPDP_PLL_IDIV_PRE_SHIFT			0
#define	 LPDP_PLL_IDIV_PRE_MASK				(0x1F << LPDP_PLL_IDIV_PRE_SHIFT)

#define	rLPDP_PLL_CP					(0x0018C)
#define	 LPDP_PLL_CP_BOOST_MODE				(1 << 14)
#define	 LPDP_PLL_CP_BOOST_EN				(1 << 13)
#define	 LPDP_PLL_CP_ENABLE				(1 << 12)
#define	 LPDP_PLL_CP_RSTN				(1 << 11)
#define	 LPDP_PLL_CP_LIN_EN				(1 << 10)
#define	 LPDP_PLL_CP_R_SET(n)				(n << 3)
#define	 LPDP_PLL_CP_I_SET(n)				(n << 0)

#define	rLPDP_PLL_LOCK					(0x00190)
#define	 LPDP_PLL_LOCK_OUT_OFF				(0 << 9)
#define	 LPDP_PLL_LOCK_OUT_ON				(1 << 9)
#define	 LPDP_PLL_LOCK_RST_OFF				(0 << 8)
#define	 LPDP_PLL_LOCK_RST_ON				(1 << 8)
#define	 LPDP_PLL_LOCK_POR_OFF				(0 << 7)
#define	 LPDP_PLL_LOCK_POR_ON				(1 << 7)
#define	 LPDP_PLL_LOCK_DIV(n)				((n) << 0)

#define	rLPDP_PLL_VCO					(0x00194)
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

#define	rLPDP_PLL_SDM					(0x00198)
#define	rLPDP_PLL_SSC					(0x0019c)
#define	rLPDP_PLL_FMC					(0x001a0)

#define	rLPDP_PLL_SEL					(0x001a4)
#define	RLPDP_PLL_SEL_REF_OUT				(1 << 7)
#define	RLPDP_PLL_SEL_VCO_OUT				(1 << 6)
#define	RLPDP_PLL_SEL_PLL_OUT				(1 << 5)
#define	RLPDP_PLL_SEL_TESTWRAPPER			(1 << 4)
#define	RLPDP_PLL_SEL_CK_DFT				(1 << 3)
#define	RLPDP_PLL_SEL_REF_DFT				(1 << 2)
#define	RLPDP_PLL_SEL_VCO_DFT				(1 << 1)
#define	RLPDP_PLL_SEL_PLL_DFT				(1 << 0)

#if LPDP_PHY_VERSION < 2
#define	rLPDP_PLL_SPARE					(0x001a8)
#elif LPDP_PHY_VERSION == 2
#define	rLPDP_PLL_LPF					(0x001a8)
#define	rLPDP_PLL_FCAL					(0x001ac)
#define	 LPDP_PLL_FCAL_RESET				(1 << 18)
#define	rLPDP_PLL_SPARE					(0x001b0)
#endif
#endif //_LPDP_PHY_REGS_V2_H
