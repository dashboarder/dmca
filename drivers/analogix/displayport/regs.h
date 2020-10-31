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

#ifndef _LPDP_REGS_H
#define _LPDP_REGS_H
						
#define	DPTX_VERSION						0x0010	
						
#define	DPTX_FUNCTION_ENABLE_1					0x0018			
#define	 DPTX_VID_CAP_FUNC_EN_N					(1 << 6)
#define	 DPTX_VID_FIFO_FUNC_EN_N				(1 << 5)
#define	 DPTX_LINK_CONTROLLER_RESET				(1 << 1)
#define	 DPTX_SW_FUNC_EN_N					(1 << 0)
						
#define	DPTX_FUNCTION_ENABLE_2					0x001C			
#define	 DPTX_SSC_FUNC_EN_N					(1 << 7)
#define	 DPTX_AUX_FUNC_EN_N					(1 << 2)
#define	 DPTX_SERDES_FIFO_FUNC_EN_N				(1 << 1)
#define	 DPTX_LS_CLK_DOMAIN_FUNC_EN_N				(1 << 0)
						
#define	DPTX_VIDEO_CONTROL_1					0x0020			
#define	 DPTX_VIDEO_EN						(1 << 7)
#define	 DPTX_VIDEO_MUTE					(1 << 6)
#define	 DPTX_FRAME_CHANGE_EN					(1 << 0)
						
#define	DPTX_VIDEO_CONTROL_2					0x0024			
#define	 DPTX_VIDEO_CTL_2_IN_D_RANGE_SHIFT			7
#define	 DPTX_IN_D_RANGE					(1 << 7)
#define	 DPTX_IN_BPC_12_BITS					(3 << 4)
#define	 DPTX_IN_BPC_10_BITS					(2 << 4)
#define	 DPTX_IN_BPC_8_BITS					(1 << 4)
#define	 DPTX_IN_BPC_6_BITS					(0 << 4)
#define	 DPTX_IN_COLOR_F_YCBCR444				(2 << 0)
#define	 DPTX_IN_COLOR_F_YCBCR422				(1 << 0)
#define	 DPTX_IN_COLOR_F_RGB					(0 << 0)
					
#define	DPTX_VIDEO_CONTROL_3					0x0028			
#define	 DPTX_IN_YC_COEFFI					(1 << 7)
#define	 DPTX_IN_YC_COEFFI_SHIFT				(7)
#define	 DPTX_IN_YC_COEFFI_MASK					(1 << 7)
#define	 DPTX_VID_CHK_UPDATE_TYPE				(1 << 4)
						
#define	DPTX_VIDEO_CONTROL_4					0x002C			
#define	 DPTX_BIST_EN						(1 << 3)
#define	 DPTX_BIST_WIDTH					(1 << 2)
#define	 DPTX_BIST_TYPE(n)					((n) << 0)
						
#define	DPTX_VIDEO_CONTROL_8					0x003C			
#define	 DPTX_VID_HRES_TH(n)					(n << 4)
#define	 DPTX_VID_VRES_TH(n)					(n << 0)
						
#define	DPTX_VIDEO_CONTROL_10					0x0044			
#define	 DPTX_F_SEL						(1 << 4)
#define	 DPTX_SLAVE_I_SCAN_CFG					(1 << 2)
#define	 DPTX_SLAVE_VSYNC_P_CFG					(1 << 1)
#define	 DPTX_SLAVE_HSYNC_P_CFG					(1 << 0)
						
#define	DPTX_TOTAL_LINE_CFG					0x0048			
						
#define	DPTX_ACTIVE_LINE_CFG					0x004C			
						
#define	DPTX_VERTICAL_FRONT_PORCH_CFG				0x0050			
#define	 DPTX_V_F_PORCH_CFG(n)					((n) << 0)
						
#define	DPTX_VERTICAL_SYNC_WIDTH_CFG				0x0054			
#define	 DPTX_V_SYNC_WIDTH_CFG(n)				((n) << 0)
						
#define	DPTX_VERTICAL_BACK_PORCH_CFG				0x0058			
#define	 DPTX_V_B_PORCH_CFG(n)					((n) << 0)
						
#define	DPTX_TOTAL_PIXEL_CFG					0x005C			
						
#define	DPTX_ACTIVE_PIXEL_CFG					0x0060			
						
#define	DPTX_HORIZON_FRONT_PORCH_CFG				0x0064			
#define	 DPTX_H_F_PORCH_CFG(n)					((n) << 0)
						
#define	DPTX_HORIZON_SYNC_WIDTH_CFG				0x0068			
#define	 DPTX_H_SYNC_CFG(n)					((n) << 0)
						
#define	DPTX_HORIZON_BACK_PORCH_CFG				0x006C			
#define	 DPTX_H_B_PORCH_CFG(n)					((n) << 0)

#define DPTX_AUX_LINES_START					0x0070
#define DPTX_AUX_LINES_END					0x0074
						
#define	DPTX_VIDEO_STATUS					0x008C
#define	 DPTX_FIELD_S						(1 << 3)
#define	 DPTX_I_SCAN_S						(1 << 2)
#define	 DPTX_VSYNC_P_S						(1 << 1)
#define	 DPTX_HSYNC_P_S						(1 << 0)
						
#define	DPTX_TOTAL_LINE_STATUS					0x0090			
#define	 DPTX_TOTAL_LINE_STA(n)					((n) << 0)
						
#define	DPTX_ACTIVE_LINE_STATUS					0x0094			
#define	 DPTX_ACTIVE_LINE_STA(n)				((n) << 0)
						
#define	DPTX_VERTICAL_FRONT_PORCH_STATUS			0x0098			
#define	 DPTX_V_F_PORCH_STA(n)					((n) << 0)
						
#define	DPTX_VERTICAL_SYNC_WIDTH_STATUS				0x009C			
#define	 DPTX_V_SYNC_STA(n)					((n) << 0)
						
#define	DPTX_VERTICAL_BACK_PORCH_STATUS				0x00A0			
#define	 DPTX_V_B_PORCH_STA(n)					((n) << 0)
						
#define	DPTX_TOTAL_PIXEL_STATUS					0x00A4			
#define	 DPTX_TOTAL_PIXEL_STA(n)				((n) << 0)
						
#define	DPTX_ACTIVE_PIXEL_STATUS				0x00A8			
#define	 DPTX_ACTIVE_PIXEL_STA(n)				((n) << 0)
						
#define	DPTX_HORIZON_FRONT_PORCH_STATUS				0x00AC			
#define	 DPTX_H_F_PORCH_STA(n)					((n) << 0)
						
#define	DPTX_HORIZON_SYNC_WIDTH_STATUS				0x00B0			
#define	 DPTX_H_SYNC_STA(n)					((n) << 0)
						
#define	DPTX_HORIZON_BACK_PORCH_STATUS				0x00B4			
#define	 DPTX_H_B_PORCH_STA(n)					((n) << 0)
						
#define	DPTX_ADVANCED_LINK_POWER_MANAGEMENT_CONTROL_1		0x0110			
#define	 DPTX_ALPM_PARA_UPDATE_EN				(1 << 7)
#define	 DPTX_ALPM_MODE						(1 << 6)
#define	 DPTX_ML_PHY_SLEEP_EN					(1 << 5)
#define	 DPTX_ML_PHY_STANDBY_EN					(1 << 4)
#define	 DPTX_WAKE_F_CHANGE_EN					(1 << 3)
#define	 DPTX_AUX_WAKE_UP_EN					(1 << 2)
#define	 DPTX_WAKE_ACK_STATE					(1 << 1)
#define	 DPTX_PLL_PWRDWN_TIMER_SEL				(1 << 0)
							
#define	DPTX_ADVANCED_LINK_POWER_MANAGEMENT_CONTROL_2		0x0114			
#define	 DPTX_SEND_SR_OPTION					(1 << 6)
#define	 DPTX_PHY_SLEEP_EN					(1 << 5)
#define	 DPTX_PHY_PLL_PWRDWN_EN					(1 << 4)
#define	 DPTX_AUX_WAKEUP_TO_MAIN_LINK				(1 << 3)
#define	 DPTX_PHY_SLEEP_ASSERT_TIME				(1 << 2)
#define	 DPTX_PLL_PWRDWN_ASSERT_TIME				(1 << 1)
#define	 DPTX_PHY_SLEEP_DEASSERT_TIME				(1 << 0)
							
#define	DPTX_PLL_POWER_DOWN_TO_ML_WAKEUP_TIMER			0x0118			
#define	 DPTX_PLL_PWD_TO_ML_WAKEUP(n)				((n) << 0)
							
#define	DPTX_WAIT_AUX_PHY_WAKE_ACK_TIMER			0x011C			
#define	 DPTX_WAIT_WAKE_ACK_TIMER(n)				((n) << 0)
							
#define	DPTX_WAKEUP_LINES					0x0120			
							
#define	DPTX_WAKEUP_CR_SYMBOLS					0x0124			
							
#define	DPTX_SYMBOL_LOCK_PATTERN				0x0130			
							
#define	DPTX_PLL_POWER_DOWN_TO_ML_WAKEUP_LINES			0x0134			
#define	 DPTX_PLL_PWD_TO_LINK_LINES(n)				((n) << 0)
							
#define	DPTX_PLL_POWER_DOWN_TO_ML_WAKEUP_TIME			0x0138			
#define	 DPTX_PLL_PWD_TO_LINK_TIME(n)				((n) << 0)
							
#define	DPTX_WAKEUP_F_CHANGE_M_VID_7_0				0x0140			
#define	 DPTX_WAKEUP_F_CHANGE_M_VID_0(n)			((n) << 0)
							
#define	DPTX_WAKEUP_F_CHANGE_M_VID_15_8				0x0144			
#define	 DPTX_WAKEUP_F_CHANGE_M_VID_1(n)			((n) << 0)
							
#define	DPTX_WAKEUP_F_CHANGE_M_VID_23_16			0x0148			
#define	 DPTX_WAKEUP_F_CHANGE_M_VID_2(n)			((n) << 0)

#if DISPLAYPORT_VERSION >= 4
#define	DPTX_SLEEP_STANDBY_DELAY_REG				0x0154
#define	 DPTX_SLEEP_STANDBY_DELAY(n)				((n) << 0)
#endif
							
#define	DPTX_AVI_INFOFRAME_PACKET_DATA_BYTE_1			0x01D0			
#define	 DPTX_AVI_DB1(n)					((n) << 0)
							
#define	DPTX_AVI_INFOFRAME_PACKET_DATA_BYTE_2			0x01D4			
#define	 DPTX_AVI_DB2(n)					((n) << 0)
							
#define	DPTX_AVI_INFOFRAME_PACKET_DATA_BYTE_3			0x01D8			
#define	 DPTX_AVI_DB3(n)					((n) << 0)
							
#define	DPTX_AVI_INFOFRAME_PACKET_DATA_BYTE_4			0x01DC			
#define	 DPTX_AVI_DB4(n)					((n) << 0)
							
#define	DPTX_AVI_INFOFRAME_PACKET_DATA_BYTE_5			0x01E0			
#define	 DPTX_AVI_DB5(n)					((n) << 0)
							
#define	DPTX_AVI_INFOFRAME_PACKET_DATA_BYTE_6			0x01E4			
#define	 DPTX_AVI_DB6(n)					((n) << 0)
							
#define	DPTX_AVI_INFOFRAME_PACKET_DATA_BYTE_7			0x01E8			
#define	 DPTX_AVI_DB7(n)					((n) << 0)
							
#define	DPTX_AVI_INFOFRAME_PACKET_DATA_BYTE_8			0x01EC			
#define	 DPTX_AVI_DB8(n)					((n) << 0)
							
#define	DPTX_AVI_INFOFRAME_PACKET_DATA_BYTE_9			0x01F0			
#define	 DPTX_AVI_DB9(n)					((n) << 0)
								
#define	DPTX_AVI_INFOFRAME_PACKET_DATA_BYTE_10			0x01F4			
#define	 DPTX_AVI_DB10(n)					((n) << 0)
							
#define	DPTX_AVI_INFOFRAME_PACKET_DATA_BYTE_11			0x01F8			
#define	 DPTX_AVI_DB11(n)					((n) << 0)
							
#define	DPTX_AVI_INFOFRAME_PACKET_DATA_BYTE_12			0x01FC			
#define	 DPTX_AVI_DB12(n)					((n) << 0)
							
#define	DPTX_AVI_INFOFRAME_PACKET_DATA_BYTE_13			0x0200			
#define	 DPTX_AVI_DB13(n)					((n) << 0)
							
#define	DPTX_INFOFRAME_PACKET_TYPE_CODE				0x0244			
#define	 DPTX_IF_TYPE(n)					((n) << 0)
							
#define	DPTX_INFOFRAME_PACKET_DATA_BYTE_1			0x0254			
#define	 DPTX_IF_PKT_DB1(n)					((n) << 0)
							
#define	DPTX_INFOFRAME_PACKET_DATA_BYTE_2			0x0258			
#define	 DPTX_IF_PKT_DB2(n)					((n) << 0)
							
#define	DPTX_INFOFRAME_PACKET_DATA_BYTE_3			0x025C			
#define	 DPTX_IF_PKT_DB3(n)					((n) << 0)
							
#define	DPTX_INFOFRAME_PACKET_DATA_BYTE_4			0x0260			
#define	 DPTX_IF_PKT_DB4(n)					((n) << 0)
							
#define	DPTX_INFOFRAME_PACKET_DATA_BYTE_5			0x0264			
#define	 DPTX_IF_PKT_DB5(n)					((n) << 0)
							
#define	DPTX_INFOFRAME_PACKET_DATA_BYTE_6			0x0268			
#define	 DPTX_IF_PKT_DB6(n)					((n) << 0)
							
#define	DPTX_INFOFRAME_PACKET_DATA_BYTE_7			0x026C			
#define	 DPTX_IF_PKT_DB7(n)					((n) << 0)
							
#define	DPTX_INFOFRAME_PACKET_DATA_BYTE_8			0x0270			
#define	 DPTX_IF_PKT_DB8(n)					((n) << 0)
							
#define	DPTX_INFOFRAME_PACKET_DATA_BYTE_9			0x0274			
#define	 DPTX_IF_PKT_DB9(n)					((n) << 0)
							
#define	DPTX_INFOFRAME_PACKET_DATA_BYTE_10			0x0278			
#define	 DPTX_IF_PKT_DB10(n)					((n) << 0)
							
#define	DPTX_INFOFRAME_PACKET_DATA_BYTE_11			0x027C			
#define	 DPTX_IF_PKT_DB11(n)					((n) << 0)
							
#define	DPTX_INFOFRAME_PACKET_DATA_BYTE_12			0x0280			
#define	 DPTX_IF_PKT_DB12(n)					((n) << 0)
							
#define	DPTX_INFOFRAME_PACKET_DATA_BYTE_13			0x0284			
#define	 DPTX_IF_PKT_DB13(n)					((n) << 0)
							
#define	DPTX_INFOFRAME_PACKET_DATA_BYTE_14			0x0288			
#define	 DPTX_IF_PKT_DB14(n)					((n) << 0)
							
#define	DPTX_INFOFRAME_PACKET_DATA_BYTE_15			0x028C			
#define	 DPTX_IF_PKT_DB15(n)					((n) << 0)
							
#define	DPTX_INFOFRAME_PACKET_DATA_BYTE_16			0x0290			
#define	 DPTX_IF_PKT_DB16(n)					((n) << 0)
							
#define	DPTX_INFOFRAME_PACKET_DATA_BYTE_17			0x0294			
#define	 DPTX_IF_PKT_DB17(n)					((n) << 0)
							
#define	DPTX_INFOFRAME_PACKET_DATA_BYTE_18			0x0298			
#define	 DPTX_IF_PKT_DB18(n)					((n) << 0)
							
#define	DPTX_INFOFRAME_PACKET_DATA_BYTE_19			0x029C			
#define	 DPTX_IF_PKT_DB19(n)					((n) << 0)
							
#define	DPTX_INFOFRAME_PACKET_DATA_BYTE_20			0x02A0			
#define	 DPTX_IF_PKT_DB20(n)					((n) << 0)
							
#define	DPTX_INFOFRAME_PACKET_DATA_BYTE_21			0x02A4			
#define	 DPTX_IF_PKT_DB21(n)					((n) << 0)
							
#define	DPTX_INFOFRAME_PACKET_DATA_BYTE_22			0x02A8			
#define	 DPTX_IF_PKT_DB22(n)					((n) << 0)
							
#define	DPTX_INFOFRAME_PACKET_DATA_BYTE_23			0x02AC			
#define	 DPTX_IF_PKT_DB23(n)					((n) << 0)
							
#define	DPTX_INFOFRAME_PACKET_DATA_BYTE_24			0x02B0			
#define	 DPTX_IF_PKT_DB24(n)					((n) << 0)
							
#define	DPTX_INFOFRAME_PACKET_DATA_BYTE_25			0x02B4			
#define	 DPTX_IF_PKT_DB25(n)					((n) << 0)
							
#define	DPTX_MPEG_SOURCE_INFOFRAME_PACKET_DATA_BYTE_1		0x02D0			
#define	 DPTX_MPEG_DB1(n)					((n) << 0)
							
#define	DPTX_MPEG_SOURCE_INFOFRAME_PACKET_DATA_BYTE_2		0x02D4			
#define	 DPTX_MPEG_DB2(n)					((n) << 0)
							
#define	DPTX_MPEG_SOURCE_INFOFRAME_PACKET_DATA_BYTE_3		0x02D8			
#define	 DPTX_MPEG_DB3(n)					((n) << 0)
							
#define	DPTX_MPEG_SOURCE_INFOFRAME_PACKET_DATA_BYTE_4		0x02DC			
#define	 DPTX_MPEG_DB4(n)					((n) << 0)
							
#define	DPTX_MPEG_SOURCE_INFOFRAME_PACKET_DATA_BYTE_5		0x02E0			
#define	 DPTX_MPEG_DB5(n)					((n) << 0)
							
#define	DPTX_MPEG_SOURCE_INFOFRAME_PACKET_DATA_BYTE_6		0x02E4			
#define	 DPTX_MPEG_DB6(n)					((n) << 0)
							
#define	DPTX_MPEG_SOURCE_INFOFRAME_PACKET_DATA_BYTE_7		0x02E8			
#define	 DPTX_MPEG_DB7(n)					((n) << 0)
							
#define	DPTX_MPEG_SOURCE_INFOFRAME_PACKET_DATA_BYTE_8		0x02EC			
#define	 DPTX_MPEG_DB8(n)					((n) << 0)
							
#define	DPTX_MPEG_SOURCE_INFOFRAME_PACKET_DATA_BYTE_9		0x02F0			
#define	 DPTX_MPEG_DB9(n)					((n) << 0)
							
#define	DPTX_MPEG_SOURCE_INFOFRAME_PACKET_DATA_BYTE_10		0x02F4			
#define	 DPTX_MPEG_DB10(n)					((n) << 0)
							
#define	DPTX_BIST_AUX_CH_RELATED_REGISTER			0x0390			
#define	 DPTX_BIST_AUX_AUX_TC_MASK				3
#define	 DPTX_BIST_YCBCR422_CONTROL				(1 << 5)
#define	 DPTX_AUX_TC_1us					(3 << 3)
#define	 DPTX_AUX_TC_d999					(2 << 3)
#define	 DPTX_AUX_TC_d799					(1 << 3)
#define	 DPTX_AUX_TC_d599					(0 << 3)
#define	 DPTX_BIST_AUX_AUX_RETRY_TIMER_SHIFT			0
#define	 DPTX_BIST_AUX_AUX_RETRY_TIMER_MASK			(7 << DPTX_BIST_AUX_AUX_RETRY_TIMER_SHIFT)
#define	 DPTX_BIST_AUX_AUX_RETRY_TIMER(n)			((n) << 0)
							
#define	DPTX_INTERRUPT_STATUS					0x03C0			
#define	 DPTX_INT_STATE						(1 << 0)
							
#define	DPTX_COMMON_INTERRUPT_STATUS_1				0x03C4			
#define	 DPTX_VSYNC_DET						(1 << 7)
#define	 DPTX_PLL_LOCK_CHG					(1 << 6)
#define	 DPTX_VID_FORMAT_CHG					(1 << 3)
#define	 DPTX_VID_CLK_STABLE					(1 << 1)
#define	 DPTX_SW_INT						(1 << 0)
							
#define	DPTX_COMMON_INTERRUPT_STATUS_4				0x03D0			
#define	 DPTX_HOTPLUG_CHG					(1 << 2)
#define	 DPTX_HPD_LOST						(1 << 1)
#define	 DPTX_PLUG						(1 << 0)
							
#define	DPTX_DISPLAYPORT_INTERRUPT_STATUS			0x03DC
#define	 DPTX_INT_HPD						(1 << 6)
#define	 DPTX_HW_TRAINING_FINISH				(1 << 5)
#define	 DPTX_POLLING_ERROR					(1 << 4)
#define	 DPTX_SINK_LOST						(1 << 3)
#define	 DPTX_LINK_LOST						(1 << 2)
#define	 DPTX_RPLY_RECEIV					(1 << 1)
#define	 DPTX_AUX_ERR						(1 << 0)
							
#define	DPTX_INTERRUPT_MASK_1					0x03E0			
#define	DPTX_INTERRUPT_MASK_4					0x03EC			

#define	DPTX_INTERRUPT_ENABLE					0x03F8			
							
#define	DPTX_INTERRUPT_CONTROL					0x03FC			
#define	 DPTX_SOFT_INT_CTRL					(1 << 2)
#define	 DPTX_INT_POL_HIGH					(1 << 0)
#define	 DPTX_INT_POL_LOW					(0 << 0)
							
#define	DPTX_SYSTEM_CONTROL_1					0x0600			
#define	 DPTX_AUX_DEGLITCH_BYPASS				(1 << 3)
#define	 DPTX_DET_STA						(1 << 2)
#define	 DPTX_FORCE_DET						(1 << 1)
#define	 DPTX_DET_CTRL						(1 << 0)
							
#define	DPTX_SYSTEM_CONTROL_2					0x0604			
#define	 DPTX_CHA_CRI(n)					((n) << 4)
#define	 DPTX_CHA_STA						(1 << 2)
#define	 DPTX_FORCE_CHA						(1 << 1)
#define	 DPTX_CHA_CTRL						(1 << 0)
							
#define	DPTX_SYSTEM_CONTROL_3					0x0608			
#define	 DPTX_HPD_POLARITY					(1 << 7)
#define	 DPTX_HPD_STATUS					(1 << 6)
#define	 DPTX_F_HPD						(1 << 5)
#define	 DPTX_HPD_CTRL						(1 << 4)
#define	 DPTX_STRM_VALID					(1 << 2)
#define	 DPTX_VIDEO_SENDING_EN					(1 << 1)
							
#define	DPTX_SYSTEM_CONTROL_4					0x060C			
#define	 DPTX_ENHANCED						(1 << 3)
#define	 DPTX_FIX_M_VID						(1 << 2)
#define	 DPTX_M_VID_UPDATE_CTRL(n)				((n) << 0)
							
#define	DPTX_DP_VIDEO_CONTROL					0x0610			
#define	 DPTX_YC_COEFF						(1 << 4)
#define	 DPTX_D_RANGE						(1 << 3)
#define	 DPTX_DP_VIDEO_CONTROL_BPC_SHIFT			5
#define	 DPTX_DP_VIDEO_CONTROL_BPC_MASK				(0x7<<DPTX_DP_VIDEO_CONTROL_BPC_SHIFT)
#define	 DPTX_DP_VIDEO_CONTROL_YC_COEFF_SHIFT			4
#define	 DPTX_DP_VIDEO_CONTROL_YC_COEFF_MASK			(0x1<<DPTX_DP_VIDEO_CONTROL_YC_COEFF_SHIFT)
#define	 DPTX_DP_VIDEO_CONTROL_D_RANGE_SHIFT			3
#define	 DPTX_DP_VIDEO_CONTROL_D_RANGE_MASK			(0x1<<DPTX_DP_VIDEO_CONTROL_D_RANGE_SHIFT)
#define	 DPTX_DP_VIDEO_CONTROL_COLOR_F_SHIFT			1
#define	 DPTX_DP_VIDEO_CONTROL_COLOR_F_MASK			(0x3<<DPTX_DP_VIDEO_CONTROL_COLOR_F_SHIFT)
							
#define	DPTX_DP_EDP_CONTROL					0x0614			
#define	 DPTX_LINK_TRAINING_WRITE_FIRST_EN			(1 << 7)
#define	 DPTX_CR_TRAINING_LOOP(n)				((n) << 4)
#define	 DPTX_INVERTED_LT_BIT_EN				(1 << 2)
#define	 DPTX_FRAMING_CHANGE_EN					(1 << 1)
#define	 DPTX_ALTERNATE_SR_EN					(1 << 0)
							
#define	DPTX_LINE_STATE_DEBUG					0x0618			
#define	 DPTX_LINE_CURRENT_STATE_DEBUG(n)			((n) << 14)
#define	 DPTX_LINE_COUNTER_DEBUG(n)				((n) << 0)
							
#define	DPTX_VERTICAL_SYNC_DEBUG_COUNTER			0x061C			
#define	 DPTX_VSYNC_DEBUG_COUNTER(n)				((n) << 0)
							
#define	DPTX_VERTICAL_BACK_DEBUG_COUNTER			0x0620			
#define	 DPTX_VBACK_DEBUG_COUNTER(n)				((n) << 0)
							
#define	DPTX_VERTICAL_ACTIVE_DEBUG_COUNTER			0x0624			
#define	 DPTX_VACTIVE_DEBUG_COUNTER(n)				((n) << 0)
							
#define	DPTX_VERTICAL_FRONT_DEBUG_COUNTER			0x0628			
#define	 DPTX_VFRONT_DEBUG_COUNTER(n)				((n) << 0)
							
#define	DPTX_PACKET_SEND_CONTROL				0x0640			
#define	 DPTX_AVI_INFO_UP					(1 << 6)
#define	 DPTX_MPEG_INFO_UP					(1 << 5)
#define	 DPTX_IF_UP						(1 << 4)
#define	 DPTX_AVI_INFO_EN					(1 << 2)
#define	 DPTX_MPEG_INFO_EN					(1 << 1)
#define	 DPTX_IF_EN						(1 << 0)
							
#define	DPTX_SCAN_LINK_CLK_SELECT				0x0648			
#define	 DPTX_LINK_CLK_SEL(n)					((n) << 4)
							
#define	DPTX_MAIN_LINK_BANDWIDTH_SETTING			0x0680			
							
#define	DPTX_MAIN_LINK_LANE_COUNT				0x0684			
							
#define	DPTX_DP_TRAINING_PATTERN_SET				0x0688			
#define	 DPTX_LINK_TRAINING_DOWNGRADE				(1 << 6)
#define	 DPTX_SCRAMBLING_DISABLE				(1 << 5)
#define	 DPTX_TRAINING_PATTERN_SEL				(1 << 4)
#define	 DPTX_LINK_QUAL_PATTERN_SET(n)				((n) << 2)
#define	 DPTX_SW_TRAINING_PATTERN_SET(n)			((n) << 0)
#define	 DPTX_SW_TRAINING_PTRN_SET_SW_MASK			0x3
							
#define	DPTX_DP_LANE_0_LINK_TRAINING_CONTROL			0x068C			
#define	 DPTX_MAX_PRE_REACH_0					(1 << 5)
#define	 DPTX_PRE_EMPHASIS_SET_0_9_5_DB				(3 << 3)
#define	 DPTX_PRE_EMPHASIS_SET_0_6_0_DB				(2 << 3)
#define	 DPTX_PRE_EMPHASIS_SET_0_3_5_DB				(1 << 3)
#define	 DPTX_PRE_EMPHASIS_SET_0_0_0_DB				(0 << 3)
#define	 DPTX_MAX_DRIVE_REACH_0					(1 << 2)
#define	 DPTX_DRIVE_CURRENT_SET_0(n)				((n) << 0)
							
#define	DPTX_DP_LANE_1_LINK_TRAINING_CONTROL			0x0690			
#define	 DPTX_MAX_PRE_REACH_1					(1 << 5)
#define	 DPTX_PRE_EMPHASIS_SET_1(n)				((n) << 3)
#define	 DPTX_MAX_DRIVE_REACH_1					(1 << 2)
#define	 DPTX_DRIVE_CURRENT_SET_1(n)				((n) << 0)
							
#define	DPTX_DP_LANE_2_LINK_TRAINING_CONTROL			0x0694			
#define	 DPTX_MAX_PRE_REACH_2					(1 << 5)
#define	 DPTX_PRE_EMPHASIS_SET_2(n)				((n) << 3)
#define	 DPTX_MAX_DRIVE_REACH_2					(1 << 2)
#define	 DPTX_DRIVE_CURRENT_SET_2(n)				((n) << 0)
							
#define	DPTX_DP_LANE_3_LINK_TRAINING_CONTROL			0x0698			
#define	 DPTX_MAX_PRE_REACH_3					(1 << 5)
#define	 DPTX_PRE_EMPHASIS_SET_3(n)				((n) << 3)
#define	 DPTX_MAX_DRIVE_REACH_3					(1 << 2)
#define	 DPTX_DRIVE_CURRENT_SET_3(n)				((n) << 0)
							
#define	DPTX_DP_DOWN_SPREADING_CONTROL				0x069C			
#define	 DPTX_SPREAD_AMP_0_5					(1 << 4)
#define	 DPTX_SPREAD_AMP_0_0					(0 << 4)
#define	 DPTX_MODULATION_FREQ_33KHZ				(1 << 0)
#define	 DPTX_MODULATION_FREQ_30KHZ				(0 << 0)
							
#define	DPTX_DP_HARDWARE_TRAINING_CONTROL			0x06A0			
#define	 DPTX_LINK_TRAINING_ERROR				(1 << 7)
#define	 DPTX_HW_TRAINING_ERROR_CODE(n)				((n) << 4)
#define	 DPTX_HW_TRAINING_EN					(1 << 0)
							
#define	DPTX_DP_DEBUG_CONTROL_1					0x06C0			
#define	 DPTX_PLL_LOCK						(1 << 7)
#define	 DPTX_F_PLL_LOCK					(1 << 6)
#define	 DPTX_PLL_LOCK_CTRL					(1 << 5)
#define	 DPTX_POLLING_EN					(1 << 4)
#define	 DPTX_INVERT_LANE3_OUTPUT_POLARITY			(1 << 3)
#define	 DPTX_INVERT_LANE2_OUTPUT_POLARITY			(1 << 2)
#define	 DPTX_INVERT_LANE1_OUTPUT_POLARITY			(1 << 1)
#define	 DPTX_INVERT_LANE0_OUTPUT_POLARITY			(1 << 0)

#define	DPTX_DP_HPD_DE_GLITCH					0x06C4			
#define	 DPTX_HPD_DEGLITCH(n)					((n) << 0)
#define	 DPTX_HPD_DEGLITCH_DEFAULT				0x1A5E
							
#define	DPTX_POLLING_PERIOD					0x06CC			
							
#define	DPTX_DP_LINK_DEBUG_CONTROL				0x06E0			
#define	 DPTX_NEW_PRBS7						(1 << 4)
#define	 DPTX_DIS_FIFO_RST					(1 << 3)
#define	 DPTX_DISABLE_AUTO_RESET_ENCODER			(1 << 2)
#define	 DPTX_INSERT_ER						(1 << 1)
#define	 DPTX_PRBS31_EN						(1 << 0)
							
#define	DPTX_SINK_COUNT						0x06E4			
							
#define	DPTX_IRQ_VECTOR						0x06E8			
							
#define	DPTX_LANE0_AND_LANE1_STATUS				0x06EC			
#define	 DPTX_LN1_SYBOL_LOCK					(1 << 6)
#define	 DPTX_LN_EQ_DONE					(1 << 5)
#define	 DPTX_LN_CR_DONE					(1 << 4)
#define	 DPTX_LN0_SYBOL_LOCK					(1 << 2)
#define	 DPTX_LN0_EQ_DONE					(1 << 1)
#define	 DPTX_LN0_CR_DONE					(1 << 0)
							
#define	DPTX_LANE2_AND_LANE3_STATUS				0x06F0			
#define	 DPTX_INTER_LN_ALIGN					(1 << 7)
#define	 DPTX_LN3_SYMBOL_LOCK					(1 << 6)
#define	 DPTX_LN3_EQ_DONE					(1 << 5)
#define	 DPTX_LN3_CR_DONE					(1 << 4)
#define	 DPTX_LN2_SYMBOL_LOCK					(1 << 2)
#define	 DPTX_LN2_EQ_DONE					(1 << 1)
#define	 DPTX_LN2_CR_DONE					(1 << 0)
							
#define	DPTX_ALIGN_STATUS					0x06F4			
							
#define	DPTX_SINK_STATUS					0x06F8			
#define	 DPTX_SINK_STA_1					(1 << 1)
#define	 DPTX_SINK_STA_0					(1 << 0)
							
#define	DPTX_M_VID_CFG_0					0x0700			
#define	 DPTX_M_VID_0(n)					((n) << 0)
							
#define	DPTX_M_VID_CFG_1					0x0704			
#define	 DPTX_M_VID_1(n)					((n) << 0)
							
#define	DPTX_M_VID_CFG_2					0x0708			
#define	 DPTX_M_VID_2(n)					((n) << 0)
							
#define	DPTX_N_VID_CFG_0					0x070C			
#define	 DPTX_N_VID_0(n)					((n) << 0)
							
#define	DPTX_N_VID_CFG_1					0x0710			
#define	 DPTX_N_VID_1(n)					((n) << 0)
							
#define	DPTX_N_VID_CFG_2					0x0714			
#define	 DPTX_N_VID_2(n)					((n) << 0)
							
#define	DPTX_M_VID_VALUE_MONITOR				0x0718			
#define	 DPTX_M_VID_MON(n)					((n) << 0)
							
#define	DPTX_DP_PLL_CONTROL					0x071C			
#define	 DPTX_DP_PLL_PD						(1 << 7)
#define	 DPTX_DP_PLL_RESET					(1 << 6)
#define	 DPTX_DP_PLL_LOOP_BIT(n)				((n) << 4)
#define	 DPTX_DP_PLL_REF_BIT(n)					((n) << 0)
							
#define	DPTX_DP_ANALOG_POWER_DOWN				0x0720			
#define	 DPTX_DP_PHY_PD						(1 << 5)
#define	 DPTX_DP_PHY_CH_MASK					(0xF)
#define	 DPTX_AUX_PD						(1 << 4)
#define	 DPTX_CH3_PD						(1 << 3)
#define	 DPTX_CH2_PD						(1 << 2)
#define	 DPTX_CH1_PD						(1 << 1)
#define	 DPTX_CH0_PD						(1 << 0)
							
#define	DPTX_DP_ANALOG_TEST					0x0724			
#define	 DPTX_MACRO_RST						(1 << 5)
#define	 DPTX_DP_PLL_TEST					(1 << 4)
#define	 DPTX_CH3_TEST						(1 << 3)
#define	 DPTX_CH2_TEST						(1 << 2)
#define	 DPTX_CH1_TEST						(1 << 1)
#define	 DPTX_CH0_TEST						(1 << 0)
							
#define	DPTX_DP_SCRAMBLER_RESET_COUNTER				0x0728			
#define	 DPTX_SCRAMBLER_RESET_COUNTER(n)			((n) << 0)
							
#define	DPTX_DP_VIDEO_DATA_FIFO_THRESHOLD			0x0730			
#define	 DPTX_VIDEO_TH_CTRL					(1 << 8)
#define  DPRX_VIDEO_TH_VALUE_SHIFT				(4)
#define	 DPTX_VIDEO_TH_VALUE(n)					((n) << DPRX_VIDEO_TH_VALUE_SHIFT)
#define  DPRX_VIDEO_TH_VALUE_MASK				(DPTX_VIDEO_TH_VALUE(0xf))
#define	 DPTX_VIDEO_TH_VALUE_STA(n)				((n) << 0)
							
#define	DPTX_DP_GNS_CONTROL_REGISTER				0x0734			
#define	 DPTX_EQ_TRAINING_LOOP_CONTROL				(1 << 6)
#define	 DPTX_SCRAMBLE_CTRL					(1 << 4)
#define	 DPTX_IN_EX						(1 << 3)
#define	 DPTX_DISABLE_SERDES_FIFO_RSET				(1 << 2)
#define	 DPTX_VIDEO_MAP_CTRL					(1 << 1)
#define	 DPTX_RS_CTRL						(1 << 0)
							
#define	DPTX_DOWN_SPREADING_CONTROL_1				0x0740			
#define	 DPTX_SSC_D_VALUE(n)					((n) << 0)
							
#define	DPTX_DOWN_SPREADING_CONTROL_2				0x0744			
#define	 DPTX_PHASE_DIR						(1 << 7)
#define	 DPTX_SSC_D_CTRL					(1 << 6)
#define	 DPTX_FS_CTRL_TH_CTRL					(1 << 5)
#define	 DPTX_FS_CTRL_TH(n)					((n) << 0)
							
#define	DPTX_DP_M_VID_CALCULATION_CONTROL			0x0760			
#define	 DPTX_M_VID_GEN_FILTER_EN				(1 << 2)
#define	 DPTX_M_GEN_CLK_SEL					(1 << 0)
							
#define	DPTX_THRESHOLD_OF_M_VID_GENERATION_FILTER		0x0764			
#define	 DPTX_M_VID_GEN_FILTER_TH(n)				((n) << 0)
							
#define	DPTX_AUX_CHANNEL_BIT_PERIOD_ADJUST			0x0774			
#define	 DPTX_AUX_BIT_PERIOD_ADJUST(n)				((n) << 0)
							
#define	DPTX_AUX_CHANNEL_LOCK_LOST_COUNTER			0x0778			
#define	 DPTX_AUX_LOCK_LOST_COUNTER(n)				((n) << 0)
							
#define	DPTX_AUX_CHANNEL_TX_TIMER				0x077C			
#define	 DPTX_AUX_TX_WAKEUP_SYNC_TIMER(n)			((n) << 12)
#define	 DPTX_AUX_TX_SYNC_TIMER(n)				((n) << 6)
#define	 DPTX_AUX_TX_DUMMY_TIMER(n)				((n) << 0)
							
#define	DPTX_AUX_CHANNEL_ACCESS_STATUS				0x0780			
#define	 DPTX_AUX_BUSY						(1 << 4)
#define	 DPTX_I2C_NACK_ERROR					(8)
#define	 DPTX_NACK_WITHOUT_M_ERROR				(7)
#define	 DPTX_RX_SHORT_ERROR					(6)
#define	 DPTX_TX_SHORT_ERROR					(5)
#define	 DPTX_MUCH_DEFER_ERROR					(4)
#define	 DPTX_UNKNOWN_ERROR					(3)
#define	 DPTX_TIMEOUT_ERROR					(2)
#define	 DPTX_NACK_ERROR					(1)
#define	 DPTX_AUX_STATUS_OK					(0)
							
#define	DPTX_AUX_CHANNEL_ACCESS_ERROR_CODE			0x0784			
#define	 DPTX_AUX_ERR_NUM(n)					((n) << 0)
							
#define	DPTX_DP_AUX_CH_DEFER_CONTROL				0x0788			
#define	 DPTX_DEFER_CTRL_EN					(1 << 7)
#define	 DPTX_DEFER_COUNT(n)					((n) << 0)
							
#define	DPTX_DP_AUX_RX_COMMAND					0x078C			
#define	 DPTX_AUX_RX_COMM(n)					((n) << 0)
#define	 LPTX_BUF_DATA_COUNT					16
							
#define	DPTX_DP_BUFFER_DATA_COUNT				0x0790			
#define	 DPTX_BUF_CLR						(1 << 9)
#define	 DPTX_BUF_DATA_COUNT					256
#define	 DPTX_BUFFER_DATA_COUNT_MASK				(0x1FF)
							
#define	DPTX_DP_AUX_CHANNEL_LENGTH				0x0794			
#define	 DPTX_AUX_LENGTH(n)					((n) << 0)
							
#define	DPTX_DP_AUX_CH_ADDRESS_0				0x0798			
#define	 DPTX_AUX_ADDR_7_0(n)					((n) << 0)
							
#define	DPTX_DP_AUX_CH_ADDRESS_1				0x079C			
#define	 DPTX_AUX_ADDR_15_8(n)					((n) << 0)
							
#define	DPTX_DP_AUX_CH_CONTROL_1				0x07A0
#define	 DPTX_AUX_CH_CTL_1_AUX_TX_COMM_SHIFT			4
#define	 DPTX_AUX_CH_CTL_1_AUX_TX_COMM_MASK			(0xF << DPTX_AUX_CH_CTL_1_AUX_TX_COMM_SHIFT)
#define	 DPTX_AUX_CH_CTL_1_AUX_TX_COMM_NATIVE			(0x8 << DPTX_AUX_CH_CTL_1_AUX_TX_COMM_SHIFT)
#define	 DPTX_AUX_CH_CTL_1_AUX_TX_COMM_MOT			(0x4 << DPTX_AUX_CH_CTL_1_AUX_TX_COMM_SHIFT)
#define	 DPTX_AUX_CH_CTL_1_AUX_TX_COMM_COMMAND_MASK		(0x3 << DPTX_AUX_CH_CTL_1_AUX_TX_COMM_SHIFT)
#define	 DPTX_AUX_CH_CTL_1_AUX_TX_COMM_WRITE			(0x0 << DPTX_AUX_CH_CTL_1_AUX_TX_COMM_SHIFT)
#define	 DPTX_AUX_CH_CTL_1_AUX_TX_COMM_READ			(0x1 << DPTX_AUX_CH_CTL_1_AUX_TX_COMM_SHIFT)
#define	 DPTX_AUX_CH_CTL_1_AUX_TX_COMM_I2C_WRITE_STATUS_UPDATE	(0x2 << DPTX_AUX_CH_CTL_1_AUX_TX_COMM_SHIFT)

#define	DPTX_DP_AUX_CH_CONTROL_2				0x07A4			
#define	 DPTX_AUX_PN_INV					(1 << 2)
#define	 DPTX_ADDR_ONLY						(1 << 1)
#define	 DPTX_AUX_EN						(1 << 0)
							
#define	DPTX_CRC_CHECK_CONTROL					0x0890			
#define	 DPTX_VID_CRC_FLUSH					(1 << 2)
#define	 DPTX_VID_CRC_ENABLE					(1 << 0)
							
#define	DPTX_CRC_RESULT						0x0894			
#define	 DPTX_VID_CRC_RESULT(n)					((n) << 0)
							
#define	DPTX_MAIN_LINK_TEST_PATTERN				0x0900			
#define	 DPTX_P_TEST_PATTERN(n)					((n) << 0)
							
#define	DPTX_ERROR_INJECTION_CONTROL_1				0x0904			
#define	 DPTX_R_ERROR_ENABLE_LANE1				(1 << 27)
#define	 DPTX_R_ERROR_INJECTION_MODE_LANE1			(1 << 26)
#define	 DPTX_R_ERROR_BIT_POSITION_LANE1(n)			((n) << 16)
#define	 DPTX_R_ERROR_ENABLE_LANE0				(1 << 11)
#define	 DPTX_R_ERROR_INJECTION_MODE_LANE0			(1 << 10)
#define	 DPTX_R_ERROR_BIT_POSITION_LANE0(n)			((n) << 0)
							
#define	DPTX_ERROR_INJECTION_CONTROL_2				0x0908			
#define	 DPTX_R_ERROR_ENABLE_LANE3				(1 << 27)
#define	 DPTX_R_ERROR_INJECTION_MODE_LANE3			(1 << 26)
#define	 DPTX_R_ERROR_BIT_POSITION_LANE3(n)			((n) << 16)
#define	 DPTX_R_ERROR_ENABLE_LANE2				(1 << 11)
#define	 DPTX_R_ERROR_INJECTION_MODE_LANE2			(1 << 10)
#define	 DPTX_R_ERROR_BIT_POSITION_LANE2(n)			((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_0				0x0C00			
#define	 DPTX_BUF_DATA_0(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_1				0x0C04			
#define	 DPTX_BUF_DATA_1(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_2				0x0C08			
#define	 DPTX_BUF_DATA_2(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_3				0x0C0C			
#define	 DPTX_BUF_DATA_3(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_4				0x0C10			
#define	 DPTX_BUF_DATA_4(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_5				0x0C14			
#define	 DPTX_BUF_DATA_5(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_6				0x0C18			
#define	 DPTX_BUF_DATA_6(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_7				0x0C1C			
#define	 DPTX_BUF_DATA_7(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_8				0x0C20			
#define	 DPTX_BUF_DATA_8(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_9				0x0C24			
#define	 DPTX_BUF_DATA_9(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_10				0x0C28			
#define	 DPTX_BUF_DATA_10(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_11				0x0C2C			
#define	 DPTX_BUF_DATA_11(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_12				0x0C30			
#define	 DPTX_BUF_DATA_12(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_13				0x0C34			
#define	 DPTX_BUF_DATA_13(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_14				0x0C38			
#define	 DPTX_BUF_DATA_14(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_15				0x0C3C			
#define	 DPTX_BUF_DATA_15(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_16				0x0C40			
#define	 DPTX_BUF_DATA_16(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_17				0x0C44			
#define	 DPTX_BUF_DATA_17(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_18				0x0C48			
#define	 DPTX_BUF_DATA_18(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_19				0x0C4C			
#define	 DPTX_BUF_DATA_19(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_20				0x0C50			
#define	 DPTX_BUF_DATA_20(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_21				0x0C54			
#define	 DPTX_BUF_DATA_21(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_22				0x0C58			
#define	 DPTX_BUF_DATA_22(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_23				0x0C5C			
#define	 DPTX_BUF_DATA_23(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_24				0x0C60			
#define	 DPTX_BUF_DATA_24(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_25				0x0C64			
#define	 DPTX_BUF_DATA_25(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_26				0x0C68			
#define	 DPTX_BUF_DATA_26(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_27				0x0C6C			
#define	 DPTX_BUF_DATA_27(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_28				0x0C70			
#define	 DPTX_BUF_DATA_28(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_29				0x0C74			
#define	 DPTX_BUF_DATA_29(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_30				0x0C78			
#define	 DPTX_BUF_DATA_30(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_31				0x0C7C			
#define	 DPTX_BUF_DATA_31(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_32				0x0C80			
#define	 DPTX_BUF_DATA_32(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_33				0x0C84			
#define	 DPTX_BUF_DATA_33(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_34				0x0C88			
#define	 DPTX_BUF_DATA_34(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_35				0x0C8C			
#define	 DPTX_BUF_DATA_35(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_36				0x0C90			
#define	 DPTX_BUF_DATA_36(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_37				0x0C94			
#define	 DPTX_BUF_DATA_37(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_38				0x0C98			
#define	 DPTX_BUF_DATA_38(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_39				0x0C9C			
#define	 DPTX_BUF_DATA_39(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_40				0x0CA0			
#define	 DPTX_BUF_DATA_40(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_41				0x0CA4			
#define	 DPTX_BUF_DATA_41(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_42				0x0CA8			
#define	 DPTX_BUF_DATA_42(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_43				0x0CAC			
#define	 DPTX_BUF_DATA_43(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_44				0x0CB0			
#define	 DPTX_BUF_DATA_44(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_45				0x0CB4			
#define	 DPTX_BUF_DATA_45(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_46				0x0CB8			
#define	 DPTX_BUF_DATA_46(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_47				0x0CBC			
#define	 DPTX_BUF_DATA_47(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_48				0x0CC0			
#define	 DPTX_BUF_DATA_48(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_49				0x0CC4			
#define	 DPTX_BUF_DATA_49(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_50				0x0CC8			
#define	 DPTX_BUF_DATA_50(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_51				0x0CCC			
#define	 DPTX_BUF_DATA_51(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_52				0x0CD0			
#define	 DPTX_BUF_DATA_52(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_53				0x0CD4			
#define	 DPTX_BUF_DATA_53(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_54				0x0CD8			
#define	 DPTX_BUF_DATA_54(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_55				0x0CDC			
#define	 DPTX_BUF_DATA_55(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_56				0x0CE0			
#define	 DPTX_BUF_DATA_56(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_57				0x0CE4			
#define	 DPTX_BUF_DATA_57(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_58				0x0CE8			
#define	 DPTX_BUF_DATA_58(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_59				0x0CEC			
#define	 DPTX_BUF_DATA_59(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_60				0x0CF0			
#define	 DPTX_BUF_DATA_60(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_61				0x0CF4			
#define	 DPTX_BUF_DATA_61(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_62				0x0CF8			
#define	 DPTX_BUF_DATA_62(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_63				0x0CFC			
#define	 DPTX_BUF_DATA_63(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_64				0x0D00			
#define	 DPTX_BUF_DATA_64(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_65				0x0D04			
#define	 DPTX_BUF_DATA_65(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_66				0x0D08			
#define	 DPTX_BUF_DATA_66(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_67				0x0D0C			
#define	 DPTX_BUF_DATA_67(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_68				0x0D10			
#define	 DPTX_BUF_DATA_68(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_69				0x0D14			
#define	 DPTX_BUF_DATA_69(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_70				0x0D18			
#define	 DPTX_BUF_DATA_70(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_71				0x0D1C			
#define	 DPTX_BUF_DATA_71(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_72				0x0D20			
#define	 DPTX_BUF_DATA_72(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_73				0x0D24			
#define	 DPTX_BUF_DATA_73(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_74				0x0D28			
#define	 DPTX_BUF_DATA_74(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_75				0x0D2C			
#define	 DPTX_BUF_DATA_75(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_76				0x0D30			
#define	 DPTX_BUF_DATA_76(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_77				0x0D34			
#define	 DPTX_BUF_DATA_77(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_78				0x0D38			
#define	 DPTX_BUF_DATA_78(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_79				0x0D3C			
#define	 DPTX_BUF_DATA_79(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_80				0x0D40			
#define	 DPTX_BUF_DATA_80(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_81				0x0D44			
#define	 DPTX_BUF_DATA_81(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_82				0x0D48			
#define	 DPTX_BUF_DATA_82(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_83				0x0D4C			
#define	 DPTX_BUF_DATA_83(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_84				0x0D50			
#define	 DPTX_BUF_DATA_84(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_85				0x0D54			
#define	 DPTX_BUF_DATA_85(n)					((n) << 0)
								
#define	DPTX_AUX_CH_BUFFER_DATA_86				0x0D58			
#define	 DPTX_BUF_DATA_86(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_87				0x0D5C			
#define	 DPTX_BUF_DATA_87(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_88				0x0D60			
#define	 DPTX_BUF_DATA_88(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_89				0x0D64			
#define	 DPTX_BUF_DATA_89(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_90				0x0D68			
#define	 DPTX_BUF_DATA_90(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_91				0x0D6C			
#define	 DPTX_BUF_DATA_91(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_92				0x0D70			
#define	 DPTX_BUF_DATA_92(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_93				0x0D74			
#define	 DPTX_BUF_DATA_93(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_94				0x0D78			
#define	 DPTX_BUF_DATA_94(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_95				0x0D7C			
#define	 DPTX_BUF_DATA_95(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_96				0x0D80			
#define	 DPTX_BUF_DATA_96(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_97				0x0D84			
#define	 DPTX_BUF_DATA_97(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_98				0x0D88			
#define	 DPTX_BUF_DATA_98(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_99				0x0D8C			
#define	 DPTX_BUF_DATA_99(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_100				0x0D90			
#define	 DPTX_BUF_DATA_100(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_101				0x0D94			
#define	 DPTX_BUF_DATA_101(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_102				0x0D98			
#define	 DPTX_BUF_DATA_102(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_103				0x0D9C			
#define	 DPTX_BUF_DATA_103(n)					((n) << 0)
								
#define	DPTX_AUX_CH_BUFFER_DATA_104				0x0DA0			
#define	 DPTX_BUF_DATA_104(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_105				0x0DA4			
#define	 DPTX_BUF_DATA_105(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_106				0x0DA8			
#define	 DPTX_BUF_DATA_106(n)					((n) << 0)
								
#define	DPTX_AUX_CH_BUFFER_DATA_107				0x0DAC			
#define	 DPTX_BUF_DATA_107(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_108				0x0DB0			
#define	 DPTX_BUF_DATA_108(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_109				0x0DB4			
#define	 DPTX_BUF_DATA_109(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_110				0x0DB8			
#define	 DPTX_BUF_DATA_110(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_111				0x0DBC			
#define	 DPTX_BUF_DATA_111(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_112				0x0DC0			
#define	 DPTX_BUF_DATA_112(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_113				0x0DC4			
#define	 DPTX_BUF_DATA_113(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_114				0x0DC8			
#define	 DPTX_BUF_DATA_114(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_115				0x0DCC			
#define	 DPTX_BUF_DATA_115(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_116				0x0DD0			
#define	 DPTX_BUF_DATA_116(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_117				0x0DD4			
#define	 DPTX_BUF_DATA_117(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_118				0x0DD8			
#define	 DPTX_BUF_DATA_118(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_119				0x0DDC			
#define	 DPTX_BUF_DATA_119(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_120				0x0DE0			
#define	 DPTX_BUF_DATA_120(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_121				0x0DE4			
#define	 DPTX_BUF_DATA_121(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_122				0x0DE8			
#define	 DPTX_BUF_DATA_122(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_123				0x0DEC			
#define	 DPTX_BUF_DATA_123(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_124				0x0DF0			
#define	 DPTX_BUF_DATA_124(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_125				0x0DF4			
#define	 DPTX_BUF_DATA_125(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_126				0x0DF8			
#define	 DPTX_BUF_DATA_126(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_127				0x0DFC			
#define	 DPTX_BUF_DATA_127(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_128				0x0E00			
#define	 DPTX_BUF_DATA_128(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_129				0x0E04			
#define	 DPTX_BUF_DATA_129(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_130				0x0E08			
#define	 DPTX_BUF_DATA_130(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_131				0x0E0C			
#define	 DPTX_BUF_DATA_131(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_132				0x0E10			
#define	 DPTX_BUF_DATA_132(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_133				0x0E14			
#define	 DPTX_BUF_DATA_133(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_134				0x0E18			
#define	 DPTX_BUF_DATA_134(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_135				0x0E1C			
#define	 DPTX_BUF_DATA_135(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_136				0x0E20			
#define	 DPTX_BUF_DATA_136(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_137				0x0E24			
#define	 DPTX_BUF_DATA_137(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_138				0x0E28			
#define	 DPTX_BUF_DATA_138(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_139				0x0E2C			
#define	 DPTX_BUF_DATA_139(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_140				0x0E30			
#define	 DPTX_BUF_DATA_140(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_141				0x0E34			
#define	 DPTX_BUF_DATA_141(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_142				0x0E38			
#define	 DPTX_BUF_DATA_142(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_143				0x0E3C			
#define	 DPTX_BUF_DATA_143(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_144				0x0E40			
#define	 DPTX_BUF_DATA_144(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_145				0x0E44			
#define	 DPTX_BUF_DATA_145(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_146				0x0E48			
#define	 DPTX_BUF_DATA_146(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_147				0x0E4C			
#define	 DPTX_BUF_DATA_147(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_148				0x0E50			
#define	 DPTX_BUF_DATA_148(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_149				0x0E54			
#define	 DPTX_BUF_DATA_149(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_150				0x0E58			
#define	 DPTX_BUF_DATA_150(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_151				0x0E5C			
#define	 DPTX_BUF_DATA_151(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_152				0x0E60			
#define	 DPTX_BUF_DATA_152(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_153				0x0E64			
#define	 DPTX_BUF_DATA_153(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_154				0x0E68			
#define	 DPTX_BUF_DATA_154(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_155				0x0E6C			
#define	 DPTX_BUF_DATA_155(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_156				0x0E70			
#define	 DPTX_BUF_DATA_156(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_157				0x0E74			
#define	 DPTX_BUF_DATA_157(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_158				0x0E78			
#define	 DPTX_BUF_DATA_158(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_159				0x0E7C			
#define	 DPTX_BUF_DATA_159(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_160				0x0E80			
#define	 DPTX_BUF_DATA_160(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_161				0x0E84			
#define	 DPTX_BUF_DATA_161(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_162				0x0E88			
#define	 DPTX_BUF_DATA_162(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_163				0x0E8C			
#define	 DPTX_BUF_DATA_163(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_164				0x0E90			
#define	 DPTX_BUF_DATA_164(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_165				0x0E94			
#define	 DPTX_BUF_DATA_165(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_166				0x0E98			
#define	 DPTX_BUF_DATA_166(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_167				0x0E9C			
#define	 DPTX_BUF_DATA_167(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_168				0x0EA0			
#define	 DPTX_BUF_DATA_168(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_169				0x0EA4			
#define	 DPTX_BUF_DATA_169(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_170				0x0EA8			
#define	 DPTX_BUF_DATA_170(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_171				0x0EAC			
#define	 DPTX_BUF_DATA_171(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_172				0x0EB0			
#define	 DPTX_BUF_DATA_172(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_173				0x0EB4			
#define	 DPTX_BUF_DATA_173(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_174				0x0EB8			
#define	 DPTX_BUF_DATA_174(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_175				0x0EBC			
#define	 DPTX_BUF_DATA_175(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_176				0x0EC0			
#define	 DPTX_BUF_DATA_176(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_177				0x0EC4			
#define	 DPTX_BUF_DATA_177(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_178				0x0EC8			
#define	 DPTX_BUF_DATA_178(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_179				0x0ECC			
#define	 DPTX_BUF_DATA_179(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_180				0x0ED0			
#define	 DPTX_BUF_DATA_180(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_181				0x0ED4			
#define	 DPTX_BUF_DATA_181(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_182				0x0ED8			
#define	 DPTX_BUF_DATA_182(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_183				0x0EDC			
#define	 DPTX_BUF_DATA_183(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_184				0x0EE0			
#define	 DPTX_BUF_DATA_184(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_185				0x0EE4			
#define	 DPTX_BUF_DATA_185(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_186				0x0EE8			
#define	 DPTX_BUF_DATA_186(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_187				0x0EEC			
#define	 DPTX_BUF_DATA_187(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_188				0x0EF0			
#define	 DPTX_BUF_DATA_188(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_189				0x0EF4			
#define	 DPTX_BUF_DATA_189(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_190				0x0EF8			
#define	 DPTX_BUF_DATA_190(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_191				0x0EFC			
#define	 DPTX_BUF_DATA_191(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_192				0x0F00			
#define	 DPTX_BUF_DATA_192(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_193				0x0F04			
#define	 DPTX_BUF_DATA_193(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_194				0x0F08			
#define	 DPTX_BUF_DATA_194(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_195				0x0F0C			
#define	 DPTX_BUF_DATA_195(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_196				0x0F10			
#define	 DPTX_BUF_DATA_196(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_197				0x0F14			
#define	 DPTX_BUF_DATA_197(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_198				0x0F18			
#define	 DPTX_BUF_DATA_198(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_199				0x0F1C			
#define	 DPTX_BUF_DATA_199(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_200				0x0F20			
#define	 DPTX_BUF_DATA_200(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_201				0x0F24			
#define	 DPTX_BUF_DATA_201(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_202				0x0F28			
#define	 DPTX_BUF_DATA_202(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_203				0x0F2C			
#define	 DPTX_BUF_DATA_203(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_204				0x0F30			
#define	 DPTX_BUF_DATA_204(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_205				0x0F34			
#define	 DPTX_BUF_DATA_205(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_206				0x0F38			
#define	 DPTX_BUF_DATA_206(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_207				0x0F3C			
#define	 DPTX_BUF_DATA_207(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_208				0x0F40			
#define	 DPTX_BUF_DATA_208(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_209				0x0F44			
#define	 DPTX_BUF_DATA_209(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_210				0x0F48			
#define	 DPTX_BUF_DATA_210(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_211				0x0F4C			
#define	 DPTX_BUF_DATA_211(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_212				0x0F50			
#define	 DPTX_BUF_DATA_212(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_213				0x0F54			
#define	 DPTX_BUF_DATA_213(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_214				0x0F58			
#define	 DPTX_BUF_DATA_214(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_215				0x0F5C			
#define	 DPTX_BUF_DATA_215(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_216				0x0F60			
#define	 DPTX_BUF_DATA_216(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_217				0x0F64			
#define	 DPTX_BUF_DATA_217(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_218				0x0F68			
#define	 DPTX_BUF_DATA_218(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_219				0x0F6C			
#define	 DPTX_BUF_DATA_219(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_220				0x0F70			
#define	 DPTX_BUF_DATA_220(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_221				0x0F74			
#define	 DPTX_BUF_DATA_221(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_222				0x0F78			
#define	 DPTX_BUF_DATA_222(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_223				0x0F7C			
#define	 DPTX_BUF_DATA_223(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_224				0x0F80			
#define	 DPTX_BUF_DATA_224(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_225				0x0F84			
#define	 DPTX_BUF_DATA_225(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_226				0x0F88			
#define	 DPTX_BUF_DATA_226(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_227				0x0F8C			
#define	 DPTX_BUF_DATA_227(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_228				0x0F90			
#define	 DPTX_BUF_DATA_228(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_229				0x0F94			
#define	 DPTX_BUF_DATA_229(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_230				0x0F98			
#define	 DPTX_BUF_DATA_230(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_231				0x0F9C			
#define	 DPTX_BUF_DATA_231(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_232				0x0FA0			
#define	 DPTX_BUF_DATA_232(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_233				0x0FA4			
#define	 DPTX_BUF_DATA_233(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_234				0x0FA8			
#define	 DPTX_BUF_DATA_234(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_235				0x0FAC			
#define	 DPTX_BUF_DATA_235(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_236				0x0FB0			
#define	 DPTX_BUF_DATA_236(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_237				0x0FB4			
#define	 DPTX_BUF_DATA_237(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_238				0x0FB8			
#define	 DPTX_BUF_DATA_238(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_239				0x0FBC			
#define	 DPTX_BUF_DATA_239(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_240				0x0FC0			
#define	 DPTX_BUF_DATA_240(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_241				0x0FC4			
#define	 DPTX_BUF_DATA_241(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_242				0x0FC8			
#define	 DPTX_BUF_DATA_242(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_243				0x0FCC			
#define	 DPTX_BUF_DATA_243(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_244				0x0FD0			
#define	 DPTX_BUF_DATA_244(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_245				0x0FD4			
#define	 DPTX_BUF_DATA_245(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_246				0x0FD8			
#define	 DPTX_BUF_DATA_246(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_247				0x0FDC			
#define	 DPTX_BUF_DATA_247(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_248				0x0FE0			
#define	 DPTX_BUF_DATA_248(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_249				0x0FE4			
#define	 DPTX_BUF_DATA_249(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_250				0x0FE8			
#define	 DPTX_BUF_DATA_250(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_251				0x0FEC			
#define	 DPTX_BUF_DATA_251(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_252				0x0FF0			
#define	 DPTX_BUF_DATA_252(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_253				0x0FF4			
#define	 DPTX_BUF_DATA_253(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_254				0x0FF8			
#define	 DPTX_BUF_DATA_254(n)					((n) << 0)
							
#define	DPTX_AUX_CH_BUFFER_DATA_255				0x0FFC			
#define	 DPTX_BUF_DATA_255(n)					((n) << 0)

#endif //_LPDP_REGS_H
