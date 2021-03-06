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
#if DEBUG_BUILD || TARGET_DISPLAY_D520
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
#if DEBUG_BUILD || TARGET_DISPLAY_D620
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
#if DEBUG_BUILD || TARGET_DISPLAY_D403
 	{ 
	.display_name = 	"D403",	
	.host_clock_id = 	CLK_VCLK0, 
	.pixel_clock = 		61540000, 
	.dot_pitch = 		326, 
	.h_active =		640,
	.h_pulse_width = 	32,  
	.h_back_porch = 	4,  
	.h_front_porch = 	6,  
	.v_active = 		1136,
	.v_pulse_width = 	3,  
	.v_back_porch = 	13, 
	.v_front_porch = 	352, 
	.neg_vclk = 		0, 
	.neg_hsync = 		0, 
	.neg_vsync = 		0, 
	.neg_vden = 		0, 
	.display_depth = 	24, 
	.display_type = 	DISPLAY_TYPE_PINOT, 
	.display_config = 	NULL,
	},
#endif
#if DEBUG_BUILD || SUB_TARGET_S8000FPGA || SUB_TARGET_S8001FPGA
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
#if DEBUG_BUILD || SUB_TARGET_S8001SIM
	{ 
	.display_name = 	"ipad4",	
	.host_clock_id = 	CLK_VCLK0, 
	.pixel_clock = 	        200000000, 
	.dot_pitch = 		264, 
	.h_active =		1536,
	.h_pulse_width = 	15, 
	.h_back_porch = 	52, 
	.h_front_porch = 	16, 
	.v_active = 		2048,
	.v_pulse_width = 	1,  
	.v_back_porch = 	3, 
	.v_front_porch = 	7, 
	.neg_vclk = 		0, 
	.neg_hsync = 		0, 
	.neg_vsync = 		0, 
	.neg_vden = 		0, 
	.display_depth = 	24, 
	.display_type = 	DISPLAY_TYPE_EDP, 
	.display_config = 	NULL,
	},
#endif
#if DEBUG_BUILD || SUB_TARGET_J127 || SUB_TARGET_J128
	{ 
	.display_name = 	"ipad6",	
	.host_clock_id = 	CLK_VCLK0, 
	.pixel_clock = 	        240000000, 
	.dot_pitch = 		264,
	.h_active =		1536,
	.h_pulse_width = 	32, 
	.h_back_porch = 	4, 
	.h_front_porch = 	126, 
	.v_active = 		2048,
	.v_pulse_width = 	2,  
	.v_back_porch = 	4, 
	.v_front_porch = 	302, 
	.neg_vclk = 		0, 
	.neg_hsync = 		0, 
	.neg_vsync = 		0, 
	.neg_vden = 		0, 
	.display_depth = 	24, 
	.display_type = 	DISPLAY_TYPE_EDP, 
	.display_config = 	NULL,
	},
#endif
#if DEBUG_BUILD || SUB_TARGET_J98A || SUB_TARGET_J99A
	{ 
	.display_name = 	"ipad6d",	
	.host_clock_id = 	CLK_VCLK0, 
	.pixel_clock = 	        400000000, 
	.dot_pitch = 		264, 
	.h_active =		2732,
	.h_pulse_width = 	20, 
	.h_back_porch = 	56, 
	.h_front_porch = 	16, 
	.v_active = 		2048,
	.v_pulse_width = 	2,  
	.v_back_porch = 	4, 
	.v_front_porch = 	307,	// No IFP
	.neg_vclk = 		0, 
	.neg_hsync = 		0, 
	.neg_vsync = 		0, 
	.neg_vden = 		0, 
	.display_depth = 	24, 
	.display_type = 	DISPLAY_TYPE_EDP, 
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
	.display_config = 	NULL,
	},
#endif
};
#endif	/* ! __APPLE_DISPLAY_TIMINGS_H */
