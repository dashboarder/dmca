/*
 * Copyright (C) 2014 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */
#ifndef __ADBE_REGS_V3_H
#define __ADBE_REGS_V3_H

#define	rADBE_VERSION				(*(volatile u_int32_t *)(DISP0_ADBE_BASE_ADDR + 0x0000))

#define	rADBE_NRT_CTL				(*(volatile u_int32_t *)(DISP0_ADBE_BASE_ADDR + 0x0004))
#define	 ADBE_NRT_CTL_VFTG_MODE_COMMAND		(1 << 25)
#define	 ADBE_NRT_CTL_TE_ENABLE_SEL_SW		(1 << 24)
#define	 ADBE_NRT_CTL_TE_MODE_MASK		(3 << 22)
#define	 ADBE_NRT_CTL_TE_MODE_IDLE		(0 << 22)
#define	 ADBE_NRT_CTL_TE_MODE_POS		(1 << 22)
#define	 ADBE_NRT_CTL_TE_MODE_NEG		(2 << 22)
#define	 ADBE_NRT_CTL_TE_MODE_TOGGLE		(3 << 22)
#define	 ADBE_NRT_CTL_AAP_ENABLE		(1 << 21)
#define	 ADBE_NRT_CTL_BN_DITHER_ENABLE		(1 << 20)
#define	 ADBE_NRT_CTL_ST_DITHER_ENABLE		(1 << 19)
#define	 ADBE_NRT_CTL_ADISP_CLK_GATE_ENABLE	(1 << 18)
#define	 ADBE_NRT_CTL_ADBE_CLK_GATE_ENABLE	(1 << 17)
#define	 ADBE_NRT_CTL_BLK_CLK_GATE_ENABLE	(1 << 16)
#define	 ADBE_NRT_CTL_VSYNC_POLARITY(n)		((n) << 9)
#define	 ADBE_NRT_CTL_HSYNC_POLARITY(n)		((n) << 8)
#define	 ADBE_NRT_CTL_SCAN_SELECT_PROG_NOCHG	(0 << 6)
#define	 ADBE_NRT_CTL_SCAN_SELECT_INT_ODD	(1 << 6)
#define	 ADBE_NRT_CTL_SCAN_SELECT_PROG_CHG	(2 << 6)
#define	 ADBE_NRT_CTL_SCAN_SELECT_INT_EVEN	(3 << 6)
#define	 ADBE_NRT_CTL_CH2_SEL_RED		(0 << 4)
#define	 ADBE_NRT_CTL_CH2_SEL_GREEN		(1 << 4)
#define	 ADBE_NRT_CTL_CH2_SEL_BLUE		(2 << 4)
#define	 ADBE_NRT_CTL_CH2_SEL_CONST		(3 << 4)
#define	 ADBE_NRT_CTL_CH1_SEL_RED		(0 << 2)
#define	 ADBE_NRT_CTL_CH1_SEL_GREEN		(1 << 2)
#define	 ADBE_NRT_CTL_CH1_SEL_BLUE		(2 << 2)
#define	 ADBE_NRT_CTL_CH1_SEL_CONST		(3 << 2)
#define	 ADBE_NRT_CTL_CH0_SEL_RED		(0 << 0)
#define	 ADBE_NRT_CTL_CH0_SEL_GREEN		(1 << 0)
#define	 ADBE_NRT_CTL_CH0_SEL_BLUE		(2 << 0)
#define	 ADBE_NRT_CTL_CH0_SEL_CONST		(3 << 0)

#define	rADBE_RT_CTL				(*(volatile u_int32_t *)(DISP0_ADBE_BASE_ADDR + 0x0008))
#define	 ADBE_RT_CTL_VFTG_STATUS_EN		(1 << 16)
#define	 ADBE_RT_CTL_VERTICAL_STATUS_IDLE	(0 << 12)
#define	 ADBE_RT_CTL_VERTICAL_STATUS_ACTIVE	(8 << 12)
#define	 ADBE_RT_CTL_VERTICAL_STATUS_FRONT	(4 << 12)
#define	 ADBE_RT_CTL_VERTICAL_STATUS_SYNC	(2 << 12)
#define	 ADBE_RT_CTL_VERTICAL_STATUS_BACK	(1 << 12)
#define	 ADBE_RT_CTL_HORIZONTAL_STATUS_IDLE	(0 << 8)
#define	 ADBE_RT_CTL_HORIZONTAL_STATUS_ACTIVE	(8 << 8)
#define	 ADBE_RT_CTL_HORIZONTAL_STATUS_FRONT	(4 << 8)
#define	 ADBE_RT_CTL_HORIZONTAL_STATUS_SYNC	(2 << 8)
#define	 ADBE_RT_CTL_HORIZONTAL_STATUS_BACK	(1 << 8)
#define	 ADBE_RT_CTL_VFTG_ENABLE_CMD_EN		(1 << 5)
#define	 ADBE_RT_CTL_VFTG_ENABLE_VID_EN		(1 << 4)
#define	 ADBE_RT_CTL_UPDATE_ENABLE_TIMING_READY	(1 << 3)
#define	 ADBE_RT_CTL_UPDATE_REQ_TIMING_NOSYNC	(1 << 2)
#define	 ADBE_RT_CTL_FRAME_COUNT_ENABLE		(1 << 1)
#define	 ADBE_RT_CTL_FRAME_COUNT_RESET_EN	(1 << 0)

#define	rADBE_ACTIVE_SIZE			(*(volatile u_int32_t *)(DISP0_ADBE_BASE_ADDR + 0x000c))
#define	 ADBE_ACTIVE_SIZE_VERTICAL(n)		((n) << 16)
#define	 ADBE_ACTIVE_SIZE_HORIZONTAL(n)		((n) << 0)

#define	rADBE_FRONT_PORCH			(*(volatile u_int32_t *)(DISP0_ADBE_BASE_ADDR + 0x0010))
#define	 ADBE_FRONT_PORCH_VERTICAL(n)		((n) << 16)
#define	 ADBE_FRONT_PORCH_HORIZONTAL(n)		((n) << 0)

#define	rADBE_SYNC_PULSE			(*(volatile u_int32_t *)(DISP0_ADBE_BASE_ADDR + 0x0014))
#define	 ADBE_SYNC_PULSE_VERTICAL(n)		((n) << 16)
#define	 ADBE_SYNC_PULSE_HORIZONTAL(n)		((n) << 0)

#define	rADBE_BACK_PORCH			(*(volatile u_int32_t *)(DISP0_ADBE_BASE_ADDR + 0x0018))
#define	 ADBE_BACK_PORCH_VERTICAL(n)		((n) << 16)
#define	 ADBE_BACK_PORCH_HORIZONTAL(n)		((n) << 0)

#define	rADBE_COUNTER_STATUS			(*(volatile u_int32_t *)(DISP0_ADBE_BASE_ADDR + 0x001c))
#define	 ADBE_COUNTER_STATUS_FRAME_COUNTER(n)	((n) << 16)
#define	 ADBE_COUNTER_STATUS_LINE_COUNTER(n)	((n) << 0)

#define	rADBE_COUNTER_POSITION			(*(volatile u_int32_t *)(DISP0_ADBE_BASE_ADDR + 0x0020))
#define	 ADBE_COUNTER_POSITION_FRAME_COUNTER(n)	((n) << 16)
#define	 ADBE_COUNTER_POSITION_LINE_COUNTER(n)	((n) << 0)

#define	rADBE_VBLANK_POSITION			(*(volatile u_int32_t *)(DISP0_ADBE_BASE_ADDR + 0x0024))
#define	 ADBE_VBLANK_POSITION_FIELD_OUT(n)	((n) << 16)
#define	 ADBE_VBLANK_POSITION_VBI_PULSE(n)	((n) << 0)

#define	rADBE_VBLANK_CLK_GATE			(*(volatile u_int32_t *)(DISP0_ADBE_BASE_ADDR + 0x0028))
#define	 ADBE_VBLANK_CLK_GATE_WAKEUP(n)		((n) << 16)
#define	 ADBE_VBLANK_CLK_GATE_IDLE(n)		((n) << 0)

#define	rADBE_VBLANK_BUSY			(*(volatile u_int32_t *)(DISP0_ADBE_BASE_ADDR + 0x002c))
#define	 ADBE_VBLANK_BUSY_FINISH(n)		((n) << 16)
#define	 ADBE_VBLANK_BUSY_START(n)			((n) << 0)

#define	rADBE_INTERRUPT				(*(volatile u_int32_t *)(DISP0_ADBE_BASE_ADDR + 0x0030))
#define	 ADBE_INTERRUPT_COUNTER_MASK		(1 << 18)
#define	 ADBE_INTERRUPT_VFTG_DOWN_MASK		(1 << 17)
#define	 ADBE_INTERRUPT_VBI_TE_MASK		(1 << 16)
#define	 ADBE_INTERRUPT_COUNTER_STATUS		(1 << 2)
#define	 ADBE_INTERRUPT_VFTG_DOWN_STATUS	(1 << 1)
#define	 ADBE_INTERRUPT_VBI_TE_STATUS		(1 << 0	)

#define	rADBE_CONST_COLOR			(*(volatile u_int32_t *)(DISP0_ADBE_BASE_ADDR + 0x0034))
#define	 ADBE_CONST_COLOR_VALUE_CH2(n)		((n) << 20)
#define	 ADBE_CONST_COLOR_VALUE_CH1(n)		((n) << 10)
#define	 ADBE_CONST_COLOR_VALUE_CH0(n)		((n) << 0)

#define	rADBE_CRC_CTL				(*(volatile u_int32_t *)(DISP0_ADBE_BASE_ADDR + 0x0038))
#define	 ADBE_CRC_CTL_ENABLE			(1 << 3)
#define	 ADBE_CRC_CTL_MULTIFRAME_ENABLE		(1 << 2)
#define	 ADBE_CRC_CTL_WINDOW_ENABLE		(1 << 1)
#define	 ADBE_CRC_CTL_VALID			(1 << 0)

#define	rADBE_CRC_WINDOW			(*(volatile u_int32_t *)(DISP0_ADBE_BASE_ADDR + 0x003c))
#define	 ADBE_CRC_WINDOW_RIGHT			((n) << 16)
#define	 ADBE_CRC_WINDOW_LEFT			((n) << 0)

#define	rADBE_CRC_RESULT			(*(volatile u_int32_t *)(DISP0_ADBE_BASE_ADDR + 0x0040))

#define	rADBE_FIFO_CONFIG			(*(volatile u_int32_t *)(DISP0_ADBE_BASE_ADDR + 0x0044))
#define	 ADBE_FIFO_CONFIG_UNDERRUN_MODE_DEFAULT	(0 << 30)
#define	 ADBE_FIFO_CONFIG_UNDERRUN_MODE_NORMAL	(1 << 30)
#define	 ADBE_FIFO_CONFIG_UNDERRUN_MODE_FRAME	(2 << 30)
#define	 ADBE_FIFO_CONFIG_UNDERRUN_MODE_STICKY	(3 << 30)
#define	 ADBE_FIFO_CONFIG_UNDERRUN_CH2(n)	((n) << 20)
#define	 ADBE_FIFO_CONFIG_UNDERRUN_CH1(n)	((n) << 10)
#define	 ADBE_FIFO_CONFIG_UNDERRUN_CH0(n)	((n) << 0)

#define	rADBE_FIFO_STATUS			(*(volatile u_int32_t *)(DISP0_ADBE_BASE_ADDR + 0x0048))
#define	 ADBE_FIFO_STATUS_FULL			(1 << 25)
#define	 ADBE_FIFO_STATUS_EMPTY			(1 << 24)
#define	 ADBE_FIFO_STATUS_PUSH_COUNT_GRAY(n)	((n) << 12)
#define	 ADBE_FIFO_STATUS_POP_COUNT_GRAY(n)	((n) << 0)

#define	rADBE_SPARE_CONFIG0			(*(volatile u_int32_t *)(DISP0_ADBE_BASE_ADDR + 0x004c))
#define	rADBE_SPARE_CONFIG1			(*(volatile u_int32_t *)(DISP0_ADBE_BASE_ADDR + 0x0050))
#define	rADBE_SPARE_CONFIG2			(*(volatile u_int32_t *)(DISP0_ADBE_BASE_ADDR + 0x0054))
#define	rADBE_SPARE_CONFIG3			(*(volatile u_int32_t *)(DISP0_ADBE_BASE_ADDR + 0x0058))

#define	rADBE_SPARE_STATUS0			(*(volatile u_int32_t *)(DISP0_ADBE_BASE_ADDR + 0x005c))
#define	rADBE_SPARE_STATUS1			(*(volatile u_int32_t *)(DISP0_ADBE_BASE_ADDR + 0x0060))
#define	rADBE_SPARE_STATUS2			(*(volatile u_int32_t *)(DISP0_ADBE_BASE_ADDR + 0x0064))
#define	rADBE_SPARE_STATUS3			(*(volatile u_int32_t *)(DISP0_ADBE_BASE_ADDR + 0x0068))
#endif // __ADBE_REGS_V3_H
