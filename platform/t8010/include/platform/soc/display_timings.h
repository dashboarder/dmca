/*
 * Copyright (C) 2013-2015 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */
#ifndef __APPLE_DISPLAY_TIMINGS_H
#define __APPLE_DISPLAY_TIMINGS_H

//Supported timings by this platform
static struct display_timing timing_list[] =
{
#if DEBUG_BUILD || SUB_TARGET_T8010SIM
 	{ 
	.display_name = 	"D520",
        .host_clock_id = 	CLK_VCLK0, 
	.pixel_clock = 		100000000, 
	.dot_pitch = 		326,
	.h_active =		750,
	.h_pulse_width = 	32, 
	.h_back_porch = 	4, 
	.h_front_porch = 	102,  
	.v_active = 		1334,
	.v_pulse_width = 	3, 
	.v_back_porch = 	4, 
	.v_front_porch = 	536,
	.neg_vclk = 		0, 
	.neg_hsync = 		0, 
	.neg_vsync = 		0, 
	.neg_vden = 		0, 
	.display_depth = 	24, 
	.display_type = 	DISPLAY_TYPE_PINOT, 
	.display_config = 	NULL,
	},
#endif
#if DEBUG_BUILD
 	{ 
	.display_name = 	"D620",
	.host_clock_id = 	CLK_VCLK0, 
	.pixel_clock = 		200000000, 
	.dot_pitch = 		326, 
	.h_active =		1080,
	.h_pulse_width = 	32, 
	.h_back_porch = 	4,  
	.h_front_porch = 	116,  
	.v_active = 		1920,
	.v_pulse_width = 	3,  
	.v_back_porch = 	5, 
	.v_front_porch = 	778, 
	.neg_vclk = 		0, 
	.neg_hsync = 		0, 
	.neg_vsync = 		0, 
	.neg_vden = 		0, 
	.display_depth = 	24, 
	.display_type = 	DISPLAY_TYPE_PINOT, 
	.display_config = 	NULL,
	},
#endif
#if DEBUG_BUILD || SUB_TARGET_T8010FPGA
	{ 
	.display_name = 	"fpga-wsvga",
	.host_clock_id = 	CLK_VCLK0,  
	.pixel_clock = 	        5000000, 
	.dot_pitch = 		243, 
	.h_active =		1024,
	.h_pulse_width = 	50,  
	.h_back_porch = 	50, 
	.h_front_porch = 	51,
	.v_active = 		600,
	.v_pulse_width = 	50,  
	.v_back_porch = 	218,   
	.v_front_porch = 	6,  
	.neg_vclk = 		0, 
	.neg_hsync = 		0, 
	.neg_vsync = 		0, 
	.neg_vden = 		0, 
	.display_depth = 	18, 
	.display_type = 	DISPLAY_TYPE_DUMB, 
	.display_config = 	NULL,
	},
#endif
#if DEBUG_BUILD || WITH_HW_DISPLAY_DISPLAYPORT
	{ 
	.display_name = 	"720p",
	.host_clock_id = 	CLK_VCLK0, 
	.pixel_clock = 	        74250000, 
	.dot_pitch = 		243, 
	.h_active =		1280,
	.h_pulse_width = 	40,
	.h_back_porch = 	110, 
	.h_front_porch = 	220,  
	.v_active = 		720,
	.v_pulse_width = 	5, 
	.v_back_porch = 	5, 
	.v_front_porch = 	20, 
	.neg_vclk = 		0, 
	.neg_hsync = 		0, 
	.neg_vsync = 		0, 
	.neg_vden = 		0, 
	.display_depth = 	24, 
	.display_type = 	DISPLAY_TYPE_DP,     
	.display_config.dp.mode	=		0x0,
	.display_config.dp.type	=		0x1,
	.display_config.dp.min_link_rate =	0x6,
	.display_config.dp.max_link_rate =	0x6,
	.display_config.dp.lanes =		0x2,
	.display_config.dp.ssc =		0x0,
	.display_config.dp.alpm =		0x0,
	.display_config.dp.vrr_enable =		0x0,
	.display_config.dp.vrr_on =		0x0,
	},
#endif
};
#endif	/* ! __APPLE_DISPLAY_TIMINGS_H */
