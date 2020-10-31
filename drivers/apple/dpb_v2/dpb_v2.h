/*
 * Copyright (C) 2012 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */
#ifndef __APPLE_DPB_H
#define __APPLE_DPB_H

#include <platform/soc/hwregbase.h>

#ifdef DISP0_DPB_BASE_ADDR
#define DPB_BASE_ADDR	DISP0_DPB_BASE_ADDR
#endif
#ifndef DPB_BASE_ADDR
#error "DPB base addr for AppleDynamicPixelBackligh (DPB) Block is not defined"
#endif

#define	rDPB_CONTROL_REG			(*(volatile u_int32_t *)(DPB_BASE_ADDR + 0x0000))	//Main Control Register

#define DPB_CTRL_BCL_ENABLE			(1 << 7)
#define DPB_CTRL_DYNAMIC_BACKLIGHT		(1 << 6)
#define DPB_CTRL_TMF_BYPASS			(1 << 5)
#define DPB_CTRL_MIN_PIXEL_MASK_ENABLE		(1 << 4)
#define DPB_CTRL_TEMPORTAL_FILTER_BYPASS	(1 << 3)
#define DPB_CTRL_SMOOTH_OFF			(1 << 2)
#define DPB_CTRL_DPB_DYNAMIC_TMF		(1 << 1)

#define	rDPB_HISTOGRAM_CONTROL_REG		(*(volatile u_int32_t *)(DPB_BASE_ADDR + 0x0004))	//Definition of luminance calculation for histogram
#define	rDPB_ACTIVE_REGION_START_REG		(*(volatile u_int32_t *)(DPB_BASE_ADDR + 0x0008))	//Register defining top and left boundaries of active pixel window
#define	rDPB_ACTIVE_REGION_SIZE_REG		(*(volatile u_int32_t *)(DPB_BASE_ADDR + 0x000c))	//Register defining bottom and right boundaries of active pixel window
#define	rDPB_TEMPORAL_FILTER_CONTROL_REG	(*(volatile u_int32_t *)(DPB_BASE_ADDR + 0x0010))	//Temporal Filter Control Register 1
#define	rDPB_TEMPORAL_FILTER_CONTROL2_REG	(*(volatile u_int32_t *)(DPB_BASE_ADDR + 0x0014))	//Temporal Filter Control Register 2
#define	rDPB_SLOPE_CONTROL_REG			(*(volatile u_int32_t *)(DPB_BASE_ADDR + 0x0018))	//Slope Control Register
#define DPB_SLOPEM2_SHIFT			(20)
#define DPB_SLOPEM2_MASK			(0x1FF << DPB_SLOPEM2_SHIFT)

#define	rDPB_UPDATE_CONTROL_REG			(*(volatile u_int32_t *)(DPB_BASE_ADDR + 0x001c))	//Synchronize LUT and register updates to next vertical blank interval

#define	rDPB_DETECT_VBI_CONTROL_REG		(*(volatile u_int32_t *)(DPB_BASE_ADDR + 0x0020))	//Detect VBI Register
#define	rDPB_MAX_SLOPE_CONTROL_REG		(*(volatile u_int32_t *)(DPB_BASE_ADDR + 0x0024))	//Max Slope Control Register
#define	rDPB_SLOPE_STATUS_REG			(*(volatile u_int32_t *)(DPB_BASE_ADDR + 0x0028))	//Slope Status Register
#define	rDPB_HISTOGRAM_BIN_STATUS_REG		(*(volatile u_int32_t *)(DPB_BASE_ADDR + 0x002c))	//Histogram Bin Status Register
#define	rDPB_HISTOGRAM_PIXEL_CNT_STATUS_REG	(*(volatile u_int32_t *)(DPB_BASE_ADDR + 0x0030))	//Histogram Pixel Count Status Register
#define	rDPB_BACK_LIGHT_LEVEL_STATUS_REG	(*(volatile u_int32_t *)(DPB_BASE_ADDR + 0x0034))	//Backlight Level Status Register

#define	rDPB_HISTOGRAM_MIN_PIXEL_CNT_STATUS_REG	(*(volatile u_int32_t *)(DPB_BASE_ADDR + 0x0038))	//Histogram Min Pixel Count Status Register
#define	rDPB_MIN_PIXEL_REG			(*(volatile u_int32_t *)(DPB_BASE_ADDR + 0x003c))	//Minimum Pixel Threshold Register
#define	rDPB_CLIP_PIXEL_CNT_REG			(*(volatile u_int32_t *)(DPB_BASE_ADDR + 0x0040))	//Pixel Count Register
#define	rDPB_MAX_KNEE_POINT_DELTA_REG		(*(volatile u_int32_t *)(DPB_BASE_ADDR + 0x0044))	//Max Kneepoint Delta Register
#define	rDPB_BACK_LIGHT_CONTROL_REG		(*(volatile u_int32_t *)(DPB_BASE_ADDR + 0x0048))	//Back light control register used to override computed back light value
#define rDPB_BACK_LIGHT_SELECT_REG		(*(volatile u_int32_t *)(DPB_BASE_ADDR + 0x004c))
#define rDPB_DIMMING_DELAY_REG			(*(volatile u_int32_t *)(DPB_BASE_ADDR + 0x0050))
#define	rDPB_INTERRUPT_CONTROL_REG		(*(volatile u_int32_t *)(DPB_BASE_ADDR + 0x0054))	//DPB Interrupt Control Register
#define	rDPB_INTERRUPT_STATUS_REG		(*(volatile u_int32_t *)(DPB_BASE_ADDR + 0x0058))	//DPB Interrupt Status Register
#define	rDPB_DEGAMMA_KNEE_POINT_STATUS_REG	(*(volatile u_int32_t *)(DPB_BASE_ADDR + 0x005c))	//Knee Point values in gamma decoded domain
#define	rDPB_TF_DEGAMMA_KNEE_POINT_STATUS_REG	(*(volatile u_int32_t *)(DPB_BASE_ADDR + 0x0060))
#define	rDPB_DEGAMMA_MAX_BRIGHT_STATUS_REG	(*(volatile u_int32_t *)(DPB_BASE_ADDR + 0x0064))	//Maximum Brightness value in gamma decoded domain
#define	rDPB_FIXED_SLOPE_REG			(*(volatile u_int32_t *)(DPB_BASE_ADDR + 0x0068))

#define	rDPB_TF_FIFO_PTRS_REG			(*(volatile u_int32_t *)(DPB_BASE_ADDR + 0x006c))
#define	rDPB_EXCLUDE_CONTROL_REG		(*(volatile u_int32_t *)(DPB_BASE_ADDR + 0x0070))
#define	rDPB_EXCLUDE_REGION_START_REG		(*(volatile u_int32_t *)(DPB_BASE_ADDR + 0x0074))
#define	rDPB_EXCLUDE_REGION_SIZE_REG		(*(volatile u_int32_t *)(DPB_BASE_ADDR + 0x0088))
#define	rDPB_TF_DEBUG_REG			(*(volatile u_int32_t *)(DPB_BASE_ADDR + 0x009c))
#define	rDPB_ENGAMMA_LAST_REG			(*(volatile u_int32_t *)(DPB_BASE_ADDR + 0x00a0))
#define	rDPB_DEGAMMA_LAST_REG			(*(volatile u_int32_t *)(DPB_BASE_ADDR + 0x00a4))
#define	rDPB_BCL_LUT_LAST_REG			(*(volatile u_int32_t *)(DPB_BASE_ADDR + 0x00a8))
#define	rDPB_ENGAMMA_REG			(*(volatile u_int32_t *)(DPB_BASE_ADDR + 0x00ac))
#define	rDPB_DEGAMMA_REG			(*(volatile u_int32_t *)(DPB_BASE_ADDR + 0x042c))

#endif /* ! __APPLE_DPB_H */