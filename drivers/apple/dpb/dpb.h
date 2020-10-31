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

#define	DPB_RED					0
#define	DPB_GREEN				1
#define	DPB_BLUE				2

#define	rDPB_CONTROL_REG			(*(volatile u_int32_t *)(DPB_BASE_ADDR + 0x0000))	//Main Control Register

#define	DPB_CTRL_CGM_BYPASSCFG			(1 << 9)
#define	DPB_CTRL_TMF_BYPASS_CFG			(1 << 8)
#define	DPB_CTRL_PMR_BYPASS_CFG			(1 << 7)
#define	DPB_CTRL_MIN_PIXEL_MASK_ENABLE_CFG	(1 << 6)
#define	DPB_CTRL_CGM_ENABLE_CFG			(1 << 5)
#define	DPB_CTRL_PMR_ENABLE_CFG			(1 << 4)
#define	DPB_CTRL_LOW_PASS_FILTER_BYPASS_CFG	(1 << 3)
#define	DPB_CTRL_SMOOTH_OFF_CFG			(1 << 2)
#define DPB_CTRL_DYNAMIC_TMF_CFG		(1 << 1)
#define DPB_CTRL_ENABLE_CFG			(1 << 0)

#define DPB_CTRL_BCL_ENABLE			(1 << 7)
#define DPB_CTRL_DYNAMIC_BACKLIGHT		(1 << 6)
#define DPB_CTRL_TMF_BYPASS			(1 << 5)
#define DPB_CTRL_MIN_PIXEL_MASK_ENABLE		(1 << 4)
#define DPB_CTRL_TEMPORTAL_FILTER_BYPASS	(1 << 3)
#define DPB_CTRL_SMOOTH_OFF			(1 << 2)
#define DPB_CTRL_DPB_DYNAMIC_TMF		(1 << 1)

#define	rDPB_HISTOGRAM_CONTROL_REG		(*(volatile u_int32_t *)(DPB_BASE_ADDR + 0x0004))	//Definition of luminance calculation for histogram
#define	rDPB_WINDOW_UPPER_LEFT_REG		(*(volatile u_int32_t *)(DPB_BASE_ADDR + 0x0008))	//Register defining top and left boundaries of active pixel window
#define	rDPB_WINDOW_BOTTOM_RIGHT_REG		(*(volatile u_int32_t *)(DPB_BASE_ADDR + 0x000c))	//Register defining bottom and right boundaries of active pixel window
#define	rDPB_LOW_PASS_FILTER_CONTROL_REG	(*(volatile u_int32_t *)(DPB_BASE_ADDR + 0x0010))	//Temporal Filter Control Register 1
#define	rDPB_LOW_PASS_FILTER_CONTROL2_REG	(*(volatile u_int32_t *)(DPB_BASE_ADDR + 0x0014))	//Temporal Filter Control Register 2
#define	rDPB_SLOPE_CONTROL_REG			(*(volatile u_int32_t *)(DPB_BASE_ADDR + 0x0018))	//Slope Control Register
#define DPB_SLOPEM2_SHIFT			(20)
#define DPB_SLOPEM2_MASK			(0x1FF << DPB_SLOPEM2_SHIFT)

#define	rDPB_TABLE_CONTROL_REG			(*(volatile u_int32_t *)(DPB_BASE_ADDR + 0x001c))	//Synchronize LUT and register updates to next vertical blank interval
#define DPB_IMMEDIATE_UPDATE			(1 << 15)
#define DPB_DUMP_HISTOGRAM			(1 << 14)
#define DPB_UPDATE_REGISTERS			(1 << 13)
#define DPB_UPDATE_CGM_BLUE_TABLE		(1 << 12)
#define DPB_UPDATE_CGM_GREEN_TABLE		(1 << 11)
#define DPB_UPDATE_CGM_RED_TABLE		(1 << 10)
#define DPB_UPDATE_PMR_BLUE_TABLE		(1 << 9)
#define DPB_UPDATE_PMR_GREEN_TABLE		(1 << 8)
#define DPB_UPDATE_PMR_RED_TABLE		(1 << 7)
#define DPB_UPDATE_ENGAMMA_BLUE_TABLE		(1 << 6)
#define DPB_UPDATE_ENGAMMA_GREEN_TABLE		(1 << 5)
#define DPB_UPDATE_ENGAMMA_RED_TABLE		(1 << 4)
#define DPB_UPDATE_DEGAMMA_BLUE_TABLE		(1 << 2)
#define DPB_UPDATE_DEGAMMA_GREEN_TABLE		(1 << 1)
#define DPB_UPDATE_DEGAMMA_RED_TABLE		(1 << 0)

#define	rDPB_DETECT_VBI_CONTROL_REG		(*(volatile u_int32_t *)(DPB_BASE_ADDR + 0x0020))	//Detect VBI Register
#define	rDPB_MAX_SLOPE_CONTROL_REG		(*(volatile u_int32_t *)(DPB_BASE_ADDR + 0x0024))	//Max Slope Control Register
#define	rDPB_SLOPE_STATUS_REG			(*(volatile u_int32_t *)(DPB_BASE_ADDR + 0x0028))	//Slope Status Register
#define	rDPB_HISTOGRAM_BIN_STATUS_REG		(*(volatile u_int32_t *)(DPB_BASE_ADDR + 0x002c))	//Histogram Bin Status Register
#define	rDPB_HISTOGRAM_PIXEL_CNT_STATUS_REG	(*(volatile u_int32_t *)(DPB_BASE_ADDR + 0x0030))	//Histogram Pixel Count Status Register
#define	rDPB_BACK_LIGHT_LEVEL_STATUS_REG	(*(volatile u_int32_t *)(DPB_BASE_ADDR + 0x0034))	//Backlight Level Status Register
#define	rDPB_FRAME_COUNTER_REG			(*(volatile u_int32_t *)(DPB_BASE_ADDR + 0x0038))	//Frame Counter Register
#define	rDPB_HISTOGRAM_MIN_PIXEL_CNT_STATUS_REG	(*(volatile u_int32_t *)(DPB_BASE_ADDR + 0x003c))	//Histogram Min Pixel Count Status Register
#define	rDPB_MIN_PIXEL_REG			(*(volatile u_int32_t *)(DPB_BASE_ADDR + 0x0040))	//Minimum Pixel Threshold Register
#define	rDPB_CLIP_PIXEL_CNT_REG			(*(volatile u_int32_t *)(DPB_BASE_ADDR + 0x0044))	//Pixel Count Register
#define	rDPB_MAX_KNEE_POINT_DELTA_REG		(*(volatile u_int32_t *)(DPB_BASE_ADDR + 0x0048))	//Max Kneepoint Delta Register
#define	rDPB_CRC_CONTROL_REG			(*(volatile u_int32_t *)(DPB_BASE_ADDR + 0x004c))	//CRC Control Register
#define	rDPB_CRC_WINDOW_START_REG		(*(volatile u_int32_t *)(DPB_BASE_ADDR + 0x0050))	//CRC Window Start Register
#define	rDPB_CRC_WINDOW_END_REG			(*(volatile u_int32_t *)(DPB_BASE_ADDR + 0x0054))	//CRC Window End Register
#define	rDPB_CRC0_STATUS_REG			(*(volatile u_int32_t *)(DPB_BASE_ADDR + 0x0058))	//CRC0 Status Register
#define	rDPB_CRC1_STATUS_REG			(*(volatile u_int32_t *)(DPB_BASE_ADDR + 0x005c))	//CRC1 Status Register (reserved)
#define	rDPB_BACK_LIGHT_CONTROL_REG		(*(volatile u_int32_t *)(DPB_BASE_ADDR + 0x0060))	//Back light control register used to override computed back light value
#define	rDPB_INTERRUPT_CONTROL_REG		(*(volatile u_int32_t *)(DPB_BASE_ADDR + 0x0064))	//DPB Interrupt Control Register
#define	rDPB_INTERRUPT_STATUS_REG		(*(volatile u_int32_t *)(DPB_BASE_ADDR + 0x0068))	//DPB Interrupt Status Register
#define	rDPB_DEGAMMA_KNEE_POINT_STATUS_REG	(*(volatile u_int32_t *)(DPB_BASE_ADDR + 0x006c))	//Knee Point values in gamma decoded domain
#define	rDPB_DEGAMMA_MAX_BRIGHT_STATUS_REG	(*(volatile u_int32_t *)(DPB_BASE_ADDR + 0x0070))	//Maximum Brightness value in gamma decoded domain
#define	rDPB_VERSION_REG			(*(volatile u_int32_t *)(DPB_BASE_ADDR + 0x0074))	//DPB Version


//DPB_MEMORY Register 
#define	DPB_HISTOGRAM_STATS			(*(volatile u_int32_t *)(DPB_BASE_ADDR + 0x0400))	// [32][256] Histogram of binned brightness values over one frame
#define	DPB_TMF_RED				(*(volatile u_int32_t *)(DPB_BASE_ADDR + 0x1000))	// [32][33]  Tone Modifier LUT, Red component
#define	DPB_TMF_GREEN				(*(volatile u_int32_t *)(DPB_BASE_ADDR + 0x1400))	// [32][33]  Tone Modifier LUT, Green component
#define	DPB_TMF_BLUE				(*(volatile u_int32_t *)(DPB_BASE_ADDR + 0x1800))	// [32][33]  Tone Modifier LUT, Blue component
#define	DPB_PMR_RED				(*(volatile u_int32_t *)(DPB_BASE_ADDR + 0x2000))	// [32][129] Pixel Manager LUT, Red component
#define	DPB_PMR_GREEN				(*(volatile u_int32_t *)(DPB_BASE_ADDR + 0x2400))	// [32][129] Pixel Manager LUT, Green component
#define	DPB_PMR_BLUE				(*(volatile u_int32_t *)(DPB_BASE_ADDR + 0x2800))	// [32][129] Pixel Manager LUT, Blue component
#define	DPB_CGM_RED				(*(volatile u_int32_t *)(DPB_BASE_ADDR + 0x3000))	// [32][129] Gamma Correction LUT, Red component
#define	DPB_CGM_GREEN				(*(volatile u_int32_t *)(DPB_BASE_ADDR + 0x3400))	// [32][129] Gamma Correction LUT, Green component
#define	DPB_CGM_BLUE				(*(volatile u_int32_t *)(DPB_BASE_ADDR + 0x3800))	// [32][129] Gamma Correction LUT, Blue component
#define	DPB_DEGAMMA_RED				(*(volatile u_int32_t *)(DPB_BASE_ADDR + 0x4000))	// [32][65]  Gamma Decode LUT, Red component
#define	DPB_DEGAMMA_GREEN			(*(volatile u_int32_t *)(DPB_BASE_ADDR + 0x4400))	// [32][65]  Gamma Decode LUT, Green component
#define	DPB_DEGAMMA_BLUE			(*(volatile u_int32_t *)(DPB_BASE_ADDR + 0x4800))	// [32][65]  Gamma Decode LUT, Blue component
#define	DPB_DEGAMMA_SLOPE			(*(volatile u_int32_t *)(DPB_BASE_ADDR + 0x4c00))	// [32][65]  Slope LUT in gamma-decoded domain
#define	DPB_ENGAMMA_RED				(*(volatile u_int32_t *)(DPB_BASE_ADDR + 0x5000))	// [32][113] Gamma encode LUT, Red component
#define	DPB_ENGAMMA_GREEN			(*(volatile u_int32_t *)(DPB_BASE_ADDR + 0x5400))	// [32][113] Gamma encode LUT, Green component
#define	DPB_ENGAMMA_BLUE			(*(volatile u_int32_t *)(DPB_BASE_ADDR + 0x5800))	// [32][113] Gamma encode LUT, Blue component

#endif /* ! __APPLE_DPB_H */
