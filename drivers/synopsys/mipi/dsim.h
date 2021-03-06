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
#ifndef __SYNOPSYS_DSIM_H
#define __SYNOPSYS_DSIM_H

#include <platform/soc/hwregbase.h>

#define	rDSIM_CORE_VERSION 			(*(volatile u_int32_t *)(DSIM_BASE_ADDR + 0x00000))
#define	rDSIM_CORE_PWR_UP 			(*(volatile u_int32_t *)(DSIM_BASE_ADDR + 0x00004))
#define	 DSIM_CORE_PWR_UP_SHUTDOWNZ		(1 << 0)
#define	rDSIM_CORE_CLKMGR_CFG 			(*(volatile u_int32_t *)(DSIM_BASE_ADDR + 0x00008))
#define	 DSIM_CORE_CLKMGR_CFG_TO_CLK_DIVISION(n)	((n) << 8)
#define	 DSIM_CORE_CLKMGR_CFG_TX_ESC_CLK_DIVISION(n)	((n) << 0)
#define	rDSIM_CORE_DPI_VCID	 		(*(volatile u_int32_t *)(DSIM_BASE_ADDR + 0x0000C))
#define	rDSIM_CORE_DPI_COLOR_CODING 		(*(volatile u_int32_t *)(DSIM_BASE_ADDR + 0x00010))
#define  DPI_COLOR_CODING_BIT16_CONF1		(0x0)
#define  DPI_COLOR_CODING_BIT16_CONF2		(0x1)
#define  DPI_COLOR_CODING_BIT16_CONF3		(0x2)
#define  DPI_COLOR_CODING_BIT18_CONF1		(0x3)
#define  DPI_COLOR_CODING_BIT18_CONF2		(0x4)
#define  DPI_COLOR_CODING_BIT24			(0x5)
#define  DPI_COLOR_CODING_BIT20_422_LOOSELY	(0x6)
#define  DPI_COLOR_CODING_BIT24_422		(0x7)
#define  DPI_COLOR_CODING_BIT16_422		(0x8)
#define  DPI_COLOR_CODING_BIT30			(0x9)
#define  DPI_COLOR_CODING_BIT36			(0xA)
#define  DPI_COLOR_CODING_BIT12_420		(0xB)
#define  DPI_COLOR_CODING_BIT12_420_SAME	(0xC)
#define  DPI_COLOR_CODING_BIT12_420_SAME2	(0xD)
#define  DPI_COLOR_CODING_BIT12_420_SAME3	(0xE)
#define  DPI_COLOR_CODING_BIT12_420_SAME4	(0xF)
#define	rDSIM_CORE_DPI_CFG_POL 			(*(volatile u_int32_t *)(DSIM_BASE_ADDR + 0x00014))
#define	rDSIM_CORE_DPI_LP_CMD_TIM 		(*(volatile u_int32_t *)(DSIM_BASE_ADDR + 0x00018))
#define	rDSIM_CORE_PCKHDL_CFG 			(*(volatile u_int32_t *)(DSIM_BASE_ADDR + 0x0002C))
#define	 DSIM_CORE_PCKHDL_CFG_CRC_RX_EN		(1 << 4)
#define  DSIM_CORE_PCKHDL_CFG_ECC_RX_EN		(1 << 3)
#define  DSIM_CORE_PCKHDL_CFG_BTA_EN		(1 << 2)
#define  DSIM_CORE_PCKHDL_CFG_EOTP_RX_EN	(1 << 1)
#define  DSIM_CORE_PCKHDL_CFG_EOTP_TX_EN	(1 << 0)
#define	rDSIM_CORE_GEN_VCID 			(*(volatile u_int32_t *)(DSIM_BASE_ADDR + 0x00030))
#define	rDSIM_CORE_MODE_CFG 			(*(volatile u_int32_t *)(DSIM_BASE_ADDR + 0x00034))
#define	 DSIM_CORE_MODE_CFG_VIDEO_MODE		(0 << 0)
#define	 DSIM_CORE_MODE_CFG_COMMAND_MODE	(1 << 0)
#define	rDSIM_CORE_VID_MODE_CFG 		(*(volatile u_int32_t *)(DSIM_BASE_ADDR + 0x00038))
#define	 DSIM_CORE_VID_MODE_CFG_LP_CMD_EN	(1 << 15)
#define	 DSIM_CORE_VID_MODE_CFG_FRAME_BTA_ACK_EN (1 << 14)
#define	 DSIM_CORE_VID_MODE_CFG_LP_HFP_EN	(1 << 13)
#define	 DSIM_CORE_VID_MODE_CFG_LP_HBP_EN	(1 << 12)
#define	 DSIM_CORE_VID_MODE_CFG_LP_VACT_EN	(1 << 11)
#define	 DSIM_CORE_VID_MODE_CFG_LP_VFP_EN	(1 << 10)
#define	 DSIM_CORE_VID_MODE_CFG_LP_VBP_EN	(1 << 9)
#define	 DSIM_CORE_VID_MODE_CFG_LP_VSA_EN	(1 << 8)
#define	 DSIM_CORE_VID_MODE_CFG_VID_MODE_TYPE_NONBURSTSYNCPULSE	(0 << 0)
#define  DSIM_CORE_VID_MODE_CFG_VID_MODE_TYPE_NONBURSTSYNCEVENT	(1 << 0)
#define  DSIM_CORE_VID_MODE_CFG_VID_MODE_TYPE_BURSTMODE		(2 << 0)
#define  DSIM_CORE_VID_MODE_CFG_VID_MODE_TYPE_HORIZONTALEXPANSION	(3 << 0)
#define	rDSIM_CORE_VID_PKT_SIZE			(*(volatile u_int32_t *)(DSIM_BASE_ADDR + 0x0003C))
#define	rDSIM_CORE_VID_NUM_CHUNKS 		(*(volatile u_int32_t *)(DSIM_BASE_ADDR + 0x00040))
#define	rDSIM_CORE_VID_NULL_SIZE 		(*(volatile u_int32_t *)(DSIM_BASE_ADDR + 0x00044))
#define	rDSIM_CORE_VID_HSA_TIME 		(*(volatile u_int32_t *)(DSIM_BASE_ADDR + 0x00048))
#define	rDSIM_CORE_VID_HBP_TIME 		(*(volatile u_int32_t *)(DSIM_BASE_ADDR + 0x0004C))
#define	rDSIM_CORE_VID_HLINE_TIME 		(*(volatile u_int32_t *)(DSIM_BASE_ADDR + 0x00050))
#define	rDSIM_CORE_VID_VSA_LINES 		(*(volatile u_int32_t *)(DSIM_BASE_ADDR + 0x00054))
#define	rDSIM_CORE_VID_VBP_LINES 		(*(volatile u_int32_t *)(DSIM_BASE_ADDR + 0x00058))
#define	rDSIM_CORE_VID_VFP_LINES 		(*(volatile u_int32_t *)(DSIM_BASE_ADDR + 0x0005C))
#define	rDSIM_CORE_VID_VACTIVE_LINES 		(*(volatile u_int32_t *)(DSIM_BASE_ADDR + 0x00060))
#define	rDSIM_CORE_CMD_MODE_CFG 		(*(volatile u_int32_t *)(DSIM_BASE_ADDR + 0x00068))

#if DSIM_VERSION == 1
#define	 DSIM_CORE_CMD_MODE_CFG_MAX_RD_PKT_SIZE_LP (1 << 24)
#define	 DSIM_CORE_CMD_MODE_CFG_DCS_LW_TX_LP	(1 << 19)
#define	 DSIM_CORE_CMD_MODE_CFG_DCS_SR_0P_TX_LP	(1 << 18)
#define	 DSIM_CORE_CMD_MODE_CFG_DCS_SW_1P_TX_LP	(1 << 17)
#define	 DSIM_CORE_CMD_MODE_CFG_DCS_SW_0P_TX_LP	(1 << 16)
#endif //DSIM_VERSION == 1

#define	 DSIM_CORE_CMD_MODE_CFG_GEN_LW_TX_LP	(1 << 14)
#define	 DSIM_CORE_CMD_MODE_CFG_GEN_SR_2P_TX_LP	(1 << 13)
#define	 DSIM_CORE_CMD_MODE_CFG_GEN_SR_1P_TX_LP	(1 << 12)
#define	 DSIM_CORE_CMD_MODE_CFG_GEN_SR_0P_TX_LP	(1 << 11)
#define	 DSIM_CORE_CMD_MODE_CFG_GEN_SW_2P_TX_LP	(1 << 10)
#define	 DSIM_CORE_CMD_MODE_CFG_GEN_SW_1P_TX_LP	(1 << 9)
#define	 DSIM_CORE_CMD_MODE_CFG_GEN_SW_0P_TX_LP	(1 << 8)
#define	 DSIM_CORE_CMD_MODE_CFG_ACK_RQST_EN	(1 << 1)
#define	 DSIM_CORE_CMD_MODE_CFG_TEAR_FX_EN	(1 << 0)
#define	rDSIM_CORE_GEN_HDR 			(*(volatile u_int32_t *)(DSIM_BASE_ADDR + 0x0006C))
#define	 DSIM_CORE_GEN_HDR_GEN_WC_MSBYTE(n)	((n) << 16)
#define	 DSIM_CORE_GEN_HDR_GEN_WC_LSBYTE(n)	((n) << 8)
#define	 DSIM_CORE_GEN_HDR_GEN_VC(n)		((n) << 6)
#define	 DSIM_CORE_GEN_HDR_GEN_DT(n)		((n) << 0)
#define	rDSIM_CORE_GEN_PLD_DATA 		(*(volatile u_int32_t *)(DSIM_BASE_ADDR + 0x00070))
#define	rDSIM_CORE_CMD_PKT_STATUS 		(*(volatile u_int32_t *)(DSIM_BASE_ADDR + 0x00074))
#define	 DSIM_CORE_CMD_PKT_STATUS_GEN_RD_CMD_BUSY	(1 << 6)
#define	 DSIM_CORE_CMD_PKT_STATUS_GEN_PLD_R_FULL	(1 << 5)
#define	 DSIM_CORE_CMD_PKT_STATUS_GEN_PLD_R_EMPTY	(1 << 4)
#define	 DSIM_CORE_CMD_PKT_STATUS_GEN_PLD_W_FULL	(1 << 3)
#define	 DSIM_CORE_CMD_PKT_STATUS_GEN_PLD_W_EMPTY	(1 << 2)
#define	 DSIM_CORE_CMD_PKT_STATUS_GEN_CMD_FULL	(1 << 1)
#define	 DSIM_CORE_CMD_PKT_STATUS_GEN_CMD_EMPTY	(1 << 0)
#define	rDSIM_CORE_TO_CNT_CFG 			(*(volatile u_int32_t *)(DSIM_BASE_ADDR + 0x00078))
#define	rDSIM_CORE_HS_RD_TO_CNT 		(*(volatile u_int32_t *)(DSIM_BASE_ADDR + 0x0007C))
#define	rDSIM_CORE_LP_RD_TO_CNT 		(*(volatile u_int32_t *)(DSIM_BASE_ADDR + 0x00080))
#define	rDSIM_CORE_HS_WR_TO_CNT 		(*(volatile u_int32_t *)(DSIM_BASE_ADDR + 0x00084))
#define	rDSIM_CORE_LP_WR_TO_CNT 		(*(volatile u_int32_t *)(DSIM_BASE_ADDR + 0x00088))
#define	rDSIM_CORE_BTA_TO_CNT 			(*(volatile u_int32_t *)(DSIM_BASE_ADDR + 0x0008C))
#define	rDSIM_CORE_SDF_3D 			(*(volatile u_int32_t *)(DSIM_BASE_ADDR + 0x00090))
#define	rDSIM_CORE_LPCLK_CTRL 			(*(volatile u_int32_t *)(DSIM_BASE_ADDR + 0x00094))
#define	 DSIM_CORE_LPCLK_CTRL_AUTO_CLKLANE_CTRL	(1 << 1)
#define	 DSIM_CORE_LPCLK_CTRL_PHY_TXREQUESTCLKHS	(1 << 0)
#define	rDSIM_CORE_PHY_TMR_LPCLK_CFG 		(*(volatile u_int32_t *)(DSIM_BASE_ADDR + 0x00098))
#define	 DSIM_CORE_PHY_TMR_LPCLK_CFG_PHY_CLKHS2LP_TIME(n)	((n) << 16)
#define	 DSIM_CORE_PHY_TMR_LPCLK_CFG_PHY_CLKLP2HS_TIME(n)	((n) << 0)
#define	rDSIM_CORE_PHY_TMR_CFG 			(*(volatile u_int32_t *)(DSIM_BASE_ADDR + 0x0009C))
#define	 DSIM_CORE_PHY_TMR_CFG_PHY_HS2LP_TIME(n)	((n) << 24)
#define	 DSIM_CORE_PHY_TMR_CFG_PHY_LP2HS_TIME(n)	((n) << 16)
#define	 DSIM_CORE_PHY_TMR_CFG_MAX_RD_TIME(n)	((n) << 0)
#define	rDSIM_CORE_PHY_RSTZ 			(*(volatile u_int32_t *)(DSIM_BASE_ADDR + 0x000A0))
#define	 DSIM_CORE_PHY_RSTZ_PHY_FORCEPLL	(1 << 3)
#define	 DSIM_CORE_PHY_RSTZ_PHY_ENABLECLK	(1 << 2)
#define	 DSIM_CORE_PHY_RSTZ_PHY_RSTZ		(1 << 1)
#define	 DSIM_CORE_PHY_RSTZ_PHY_SHUTDOWNZ	(1 << 0)
#define	rDSIM_CORE_PHY_IF_CFG 			(*(volatile u_int32_t *)(DSIM_BASE_ADDR + 0x000A4))
#define	 DSIM_CORE_PHY_IF_CFG_PHY_STOP_WAIT_TIME(n)	((n) << 8)
#define	 DSIM_CORE_PHY_IF_CFG_N_LANES(n)	((n) << 0)
#define	rDSIM_CORE_PHY_ULPS_CTRL 		(*(volatile u_int32_t *)(DSIM_BASE_ADDR + 0x000A8))
#define	rDSIM_CORE_PHY_TX_TRIGGERS 		(*(volatile u_int32_t *)(DSIM_BASE_ADDR + 0x000AC))
#define	rDSIM_CORE_PHY_STATUS 			(*(volatile u_int32_t *)(DSIM_BASE_ADDR + 0x000B0))
#define	 DSIM_CORE_PHY_STATUS_PHY_ULPSACTIVENOT3LANE	(1 << 12)
#define	 DSIM_CORE_PHY_STATUS_PHY_STOPSTATE3LANE	(1 << 11)
#define	 DSIM_CORE_PHY_STATUS_PHY_ULPSACTIVENOT2LANE	(1 << 10)
#define	 DSIM_CORE_PHY_STATUS_PHY_STOPSTATE2LANE	(1 << 9)
#define	 DSIM_CORE_PHY_STATUS_PHY_ULPSACTIVENOT1LANE	(1 << 8)
#define	 DSIM_CORE_PHY_STATUS_PHY_STOPSTATE1LANE	(1 << 7)
#define	 DSIM_CORE_PHY_STATUS_PHY_RXULPSESC0LANE	(1 << 6)
#define	 DSIM_CORE_PHY_STATUS_PHY_ULPSACTIVENOT0LANE	(1 << 5)
#define	 DSIM_CORE_PHY_STATUS_PHY_STOPSTATE0LANE	(1 << 4)
#define	 DSIM_CORE_PHY_STATUS_PHY_ULPSACTIVENOTCLK	(1 << 3)
#define	 DSIM_CORE_PHY_STATUS_PHY_STOPSTATECLKLANE	(1 << 2)
#define	 DSIM_CORE_PHY_STATUS_PHY_DIRECTION		(1 << 1)
#define	 DSIM_CORE_PHY_STATUS_PHY_LOCK		(1 << 0)
#define	rDSIM_CORE_PHY_TST_CTRL0 		(*(volatile u_int32_t *)(DSIM_BASE_ADDR + 0x000B4))
#define	 DSIM_CORE_PHY_TST_CTRL0_PHY_TESTCLK	(1 << 1)
#define	 DSIM_CORE_PHY_TST_CTRL0_PHY_TESTCLR	(1 << 0)
#define	rDSIM_CORE_PHY_TST_CTRL1 		(*(volatile u_int32_t *)(DSIM_BASE_ADDR + 0x000B8))
#define	 DSIM_CORE_PHY_TST_CTRL1_PHY_TESTEN	(1 << 16)
#define	 DSIM_CORE_PHY_TST_CTRL1_PHY_TESTDOUT(n)(((n) >> 8) & 0xff)
#define	 DSIM_CORE_PHY_TST_CTRL1_PHY_TESTDIN(n)	((n) << 0)
#define	rDSIM_CORE_INT_ST0 			(*(volatile u_int32_t *)(DSIM_BASE_ADDR + 0x000BC))
#define	 DSIM_CORE_INT_ST0_DPHY_ERRORS_4	(1 << 20)
#define	 DSIM_CORE_INT_ST0_PHY_ERRORS_3		(1 << 19)
#define	 DSIM_CORE_INT_ST0_PHY_ERRORS_1		(1 << 17)
#define	 DSIM_CORE_INT_ST0_PHY_ERRORS_0		(1 << 16)
#define	 DSIM_CORE_INT_ST0_ACK_WITH_ERR_15	(1 << 15)
#define	 DSIM_CORE_INT_ST0_ACK_WITH_ERR_14	(1 << 14)
#define	 DSIM_CORE_INT_ST0_ACK_WITH_ERR_13	(1 << 13)
#define	 DSIM_CORE_INT_ST0_ACK_WITH_ERR_12	(1 << 12)
#define	 DSIM_CORE_INT_ST0_ACK_WITH_ERR_11	(1 << 11)
#define	 DSIM_CORE_INT_ST0_ACK_WITH_ERR_10	(1 << 10)
#define	 DSIM_CORE_INT_ST0_ACK_WITH_ERR_9	(1 << 9)
#define	 DSIM_CORE_INT_ST0_ACK_WITH_ERR_8	(1 << 8)
#define	 DSIM_CORE_INT_ST0_ACK_WITH_ERR_7	(1 << 7)
#define	 DSIM_CORE_INT_ST0_ACK_WITH_ERR_6	(1 << 6)
#define	 DSIM_CORE_INT_ST0_ACK_WITH_ERR_5	(1 << 5)
#define	 DSIM_CORE_INT_ST0_ACK_WITH_ERR_4	(1 << 4)
#define	 DSIM_CORE_INT_ST0_ACK_WITH_ERR_3	(1 << 3)
#define	 DSIM_CORE_INT_ST0_ACK_WITH_ERR_2	(1 << 2)
#define	 DSIM_CORE_INT_ST0_ACK_WITH_ERR_1	(1 << 1)
#define	 DSIM_CORE_INT_ST0_ACK_WITH_ERR_0	(1 << 0)
#define	rDSIM_CORE_INT_ST1 			(*(volatile u_int32_t *)(DSIM_BASE_ADDR + 0x000C0))
#define	 DSIM_CORE_INT_ST1_GEN_PLD_RECEV_ERR	(1 << 12)
#define	 DSIM_CORE_INT_ST1_GEN_PLD_RD_ERR	(1 << 11)
#define	 DSIM_CORE_INT_ST1_GEN_PLD_SEND_ERR	(1 << 10)
#define	 DSIM_CORE_INT_ST1_GEN_PLD_WR_ERR	(1 << 9)
#define	 DSIM_CORE_INT_ST1_GEN_CMD_WR_ERR	(1 << 8)
#define	 DSIM_CORE_INT_ST1_DPI_PLD_WR_ERR	(1 << 7)
#define	 DSIM_CORE_INT_ST1_EOPT_ERR		(1 << 6)
#define	 DSIM_CORE_INT_ST1_PKT_SIZE_ERR		(1 << 5)
#define	 DSIM_CORE_INT_ST1_CRC_ERR		(1 << 4)
#define	 DSIM_CORE_INT_ST1_ECC_MILTI_ERR	(1 << 3)
#define	 DSIM_CORE_INT_ST1_ECC_SINGLE_ERR	(1 << 2)
#define	 DSIM_CORE_INT_ST1_TO_LP_RX		(1 << 1)
#define	 DSIM_CORE_INT_ST1_TO_HS_TX		(1 << 0)
#define  DSIM_CORE_INT_ST1_ERRORS		(DSIM_CORE_INT_ST1_GEN_PLD_RECEV_ERR | \
						 DSIM_CORE_INT_ST1_GEN_PLD_RD_ERR | \
						 DSIM_CORE_INT_ST1_GEN_PLD_SEND_ERR | \
						 DSIM_CORE_INT_ST1_GEN_PLD_WR_ERR | \
						 DSIM_CORE_INT_ST1_GEN_CMD_WR_ERR | \
						 DSIM_CORE_INT_ST1_DPI_PLD_WR_ERR | \
						 DSIM_CORE_INT_ST1_EOPT_ERR | \
						 DSIM_CORE_INT_ST1_PKT_SIZE_ERR | \
						 DSIM_CORE_INT_ST1_CRC_ERR | \
						 DSIM_CORE_INT_ST1_ECC_MILTI_ERR | \
						 DSIM_CORE_INT_ST1_ECC_SINGLE_ERR)
#define	rDSIM_CORE_INT_MSK0 			(*(volatile u_int32_t *)(DSIM_BASE_ADDR + 0x000C4))
#define	rDSIM_CORE_INT_MSK1 			(*(volatile u_int32_t *)(DSIM_BASE_ADDR + 0x000C8))
#define	rDSIM_CORE_VID_VBP_HS_LINES 		(*(volatile u_int32_t *)(DSIM_BASE_ADDR + 0x000D0))
#define	rDSIM_CORE_VID_VFP_HS_LINES 		(*(volatile u_int32_t *)(DSIM_BASE_ADDR + 0x000D4))

//TOP
#define	rDSIM_VERSION 				(*(volatile u_int32_t *)(DSIM_BASE_ADDR + 0x80000))

#define	rDSIM_GENERAL_CTRL 			(*(volatile u_int32_t *)(DSIM_BASE_ADDR + 0x80004))
#define	 DSIM_GENERAL_CTRL_HSYNC_MASK_STATUS_ON	(1 << 28)
#define	 DSIM_GENERAL_CTRL_HSYNC_MASK_ENABLE	(1 << 27)
#define	 DSIM_GENERAL_CTRL_VID_HTIME_AUTO_TIMER(n) ((n) << 12)
#define	 DSIM_GENERAL_CTRL_VID_HTIME_AUTO_DYN	(1 << 11)
#define	 DSIM_GENERAL_CTRL_VID_HTIME_AUTO_OFFSET(n)	((n) << 9)
#define	 DSIM_GENERAL_CTRL_VID_HTIME_AUTO_ROUND_TOL0	(0 << 7)
#define	 DSIM_GENERAL_CTRL_VID_HTIME_AUTO_ROUND_TOL1	(1 << 7)
#define	 DSIM_GENERAL_CTRL_VID_HTIME_AUTO_ROUND_TOL2	(2 << 7)
#define	 DSIM_GENERAL_CTRL_VID_HTIME_AUTO_ROUND_TOL3	(3 << 7)
#define	 DSIM_GENERAL_CTRL_VID_HTIME_AUTO_VALID	(1 << 6)
#define	 DSIM_GENERAL_CTRL_VID_HTIME_AUTO_SEL	(1 << 5)
#define	 DSIM_GENERAL_CTRL_PHYLOCK_HW_LOCK	(1 << 4)
#define	 DSIM_GENERAL_CTRL_PHYLOCK_SW_LOCK	(1 << 3)
#define	 DSIM_GENERAL_CTRL_DPISHUTDN		(1 << 2)
#define	 DSIM_GENERAL_CTRL_DPICOLORM_REDUCED	(1 << 1)
#define	 DSIM_GENERAL_CTRL_DPIHALT_HALT		(1 << 0)

#define	rDSIM_LANE_SWAP				(*(volatile u_int32_t *)(DSIM_BASE_ADDR + 0x80008))
#define	rDSIM_TOP_PIXEL_REMAP 			(*(volatile u_int32_t *)(DSIM_BASE_ADDR + 0x8000C))
#define	rDSIM_PIXEL_SWAP 			(*(volatile u_int32_t *)(DSIM_BASE_ADDR + 0x80010))
#define	rDSIM_TEST_COLOR 			(*(volatile u_int32_t *)(DSIM_BASE_ADDR + 0x80014))
#define	rDSIM_VID_HSA_TIME 			(*(volatile u_int32_t *)(DSIM_BASE_ADDR + 0x80018))
#define	rDSIM_VID_HBP_TIME 			(*(volatile u_int32_t *)(DSIM_BASE_ADDR + 0x8001C))
#define	rDSIM_VID_HLINE_TIME 			(*(volatile u_int32_t *)(DSIM_BASE_ADDR + 0x80020))

#if DSIM_VERSION > 1
#define	rDSIM_TOP_AGILE_CTRL			(*(volatile u_int32_t *)(DSIM_BASE_ADDR + 0x80024))
#define  DSIM_TOP_AGILE_CTRL_HS_PACKET_LATENCY(n)	((n) << 24)
#define  DSIM_TOP_AGILE_CTRL_HS_PACKET_STATUS_HS	(1 << 23)
#define  DSIM_TOP_AGILE_CTRL_LINECOUNT_ENABLE_STATUS	(1 << 22)
#define  DSIM_TOP_AGILE_CTRL_LINECOUNT_RESET		(1 << 21)
#define  DSIM_TOP_AGILE_CTRL_LINECOUNT_TRIGGER		(1 << 20)
#define  DSIM_TOP_AGILE_CTRL_LINECOUNT_FINISH(n)	((n) << 18)
#define	 DSIM_TOP_AGILE_CTRL_LINECOUNT_FINISH_VFRONT	DSIM_TOP_AGILE_CTRL_LINECOUNT_FINISH(0x0)
#define	 DSIM_TOP_AGILE_CTRL_LINECOUNT_FINISH_VSYNC	DSIM_TOP_AGILE_CTRL_LINECOUNT_FINISH(0x1)
#define	 DSIM_TOP_AGILE_CTRL_LINECOUNT_FINISH_VBACK	DSIM_TOP_AGILE_CTRL_LINECOUNT_FINISH(0x2)
#define	 DSIM_TOP_AGILE_CTRL_LINECOUNT_FINISH_VACTIVE	DSIM_TOP_AGILE_CTRL_LINECOUNT_FINISH(0x3)
#define  DSIM_TOP_AGILE_CTRL_LINECOUNT_START(n)		((n) << 16)
#define	 DSIM_TOP_AGILE_CTRL_LINECOUNT_START_VFRONT	DSIM_TOP_AGILE_CTRL_LINECOUNT_START(0x0)
#define	 DSIM_TOP_AGILE_CTRL_LINECOUNT_START_VSYNC	DSIM_TOP_AGILE_CTRL_LINECOUNT_START(0x1)
#define	 DSIM_TOP_AGILE_CTRL_LINECOUNT_START_VBACK	DSIM_TOP_AGILE_CTRL_LINECOUNT_START(0x2)
#define	 DSIM_TOP_AGILE_CTRL_LINECOUNT_START_VACTIVE	DSIM_TOP_AGILE_CTRL_LINECOUNT_START(0x3)
#define  DSIM_TOP_AGILE_CTRL_TIMEBASE(n)		((n) << 8)
#define  DSIM_TOP_AGILE_CTRL_STATE(n)			((n) << 4)
#define  DSIM_TOP_AGILE_CTRL_ENABLE			(1 << 3)
#define  DSIM_TOP_AGILE_CTRL_USE_TEMP_CLK		(1 << 2)
#define  DSIM_TOP_AGILE_CTRL_IDLE_FRAME			(1 << 1)
#define  DSIM_TOP_AGILE_CTRL_VBLANK_ON			(1 << 0)

#define	rDSIM_TOP_AGILE_LINECOUNT		(*(volatile u_int32_t *)(DSIM_BASE_ADDR + 0x80028))
#define	 DSIM_TOP_AGILE_LINECOUNT_CONFIG_MAX	(0xFFFF)

#define	rDSIM_TOP_AGILE_SEQ1			(*(volatile u_int32_t *)(DSIM_BASE_ADDR + 0x8002c))
#define	rDSIM_TOP_AGILE_SEQ2			(*(volatile u_int32_t *)(DSIM_BASE_ADDR + 0x80030))

#define	rDSIM_TOP_PLL_CTRL			(*(volatile u_int32_t *)(DSIM_BASE_ADDR + 0x80034))
#define	DSIM_TOP_PLL_CTRL_PLL_CFG_SEL_PHY	(1 << 31)
#define	DSIM_TOP_PLL_CTRL_PLL_SHADOW_CONTROL_PHY (1 << 30)
#define	DSIM_TOP_PLL_CTRL_PLL_SHADOW_CONTROL_OBS_PHY (1 << 29)
#define	DSIM_TOP_PLL_CTRL_SEQ_OW_ON		(1 << 28)
#define	DSIM_TOP_PLL_CTRL_UPDATEPLL_ON		(1 << 27)
#define	DSIM_TOP_PLL_CTRL_CLKSELPLL_MASK	(3 << 25)
#define	DSIM_TOP_PLL_CTRL_CLKSELPLL_PLLCLK	(1 << 25)
#define	DSIM_TOP_PLL_CTRL_CLKSELPLL_TEMPCLK	(2 << 25)
#define	DSIM_TOP_PLL_CTRL_TEMP_MIPI_DSI_CLK_REQ	(1 << 24)
#define	DSIM_TOP_PLL_CTRL_TEMP_MIPI_DSI_CLK_ACK_ON (1 << 23)
#define	DSIM_TOP_PLL_CTRL_CSI_SEL_CSI2		(1 << 10
#define	DSIM_TOP_PLL_CTRL_HSFREQRANGE(n)	((n) << 4)
#define	DSIM_TOP_PLL_CTRL_FORCEPLL_ON		(1 << 3)
#define	DSIM_TOP_PLL_CTRL_SHADOW_CLEAR		(1 << 2)
#define	DSIM_TOP_PLL_CTRL_FORCE_LOCK		(1 << 1)
#define	DSIM_TOP_PLL_CTRL_ONPLL_PWRUP		(1 << 0)

#define	rDSIM_TOP_PLL_PARAM			(*(volatile u_int32_t *)(DSIM_BASE_ADDR + 0x80038))
#define	 DSIM_TOP_PLL_PARAM_VCOCAP(n)		((n) << 27)
#define	 DSIM_TOP_PLL_PARAM_VCORANGE(n)		((n) << 25)
#define	 DSIM_TOP_PLL_PARAM_LPFCTRL(n)		((n) << 19)
#define	 DSIM_TOP_PLL_PARAM_ICPCTRL(n)		((n) << 15)
#define	 DSIM_TOP_PLL_PARAM_P(n)		((n) << 14)
#define	 DSIM_TOP_PLL_PARAM_M(n)		((n) << 4)
#define	 DSIM_TOP_PLL_PARAM_N(n)		((n) << 0)

#define	rDSIM_TOP_PLL_PARAM_OBS			(*(volatile u_int32_t *)(DSIM_BASE_ADDR + 0x8003c))
#define	rDSIM_TOP_PLL_LOCK			(*(volatile u_int32_t *)(DSIM_BASE_ADDR + 0x80040))
#define	rDSIM_TOP_PLL_TEST			(*(volatile u_int32_t *)(DSIM_BASE_ADDR + 0x80044))
#define	rDSIM_TOP_BIST_CTRL			(*(volatile u_int32_t *)(DSIM_BASE_ADDR + 0x80048))
#define	rDSIM_TOP_HSYNC_PREDICT			(*(volatile u_int32_t *)(DSIM_BASE_ADDR + 0x8004c))
#endif //DSIM_VERSION

#endif /* __SYNOPSYS_DSIM_H */
