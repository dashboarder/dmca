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
#if DEBUG_BUILD || SUB_TARGET_N51 || SUB_TARGET_N53 || SUB_TARGET_CYCLONIC
	{ 
	.display_name =   	"n51",	
	.host_clock_id =  	CLK_VCLK0, 
	.pixel_clock = 	  	61540000, 
	.dot_pitch = 	  	326, 
	.h_active =	  	640,
	.h_pulse_width =  	6,  
	.h_back_porch =   	32,  
	.h_front_porch =  	4,  
	.v_active = 	  	1136, 
	.v_pulse_width =  	3,  
	.v_back_porch =   	13, 
	.v_front_porch =  	352, 
	.neg_vclk = 	  	0, 
	.neg_hsync = 	  	0, 
	.neg_vsync = 	  	0, 
	.neg_vden = 	  	0, 
	.display_depth =  	24, 
	.display_type =   	DISPLAY_TYPE_PINOT, 
	.display_config =	NULL,
	},
#endif
#if DEBUG_BUILD || WITH_HW_DISPLAY_DISPLAYPORT
	{ 
	.display_name =   	"720p",	
	.host_clock_id =  	CLK_VCLK1, 
	.pixel_clock = 	  	00000000, 
	.dot_pitch = 	  	243, 
	.h_active =	  	1280,
	.h_pulse_width =  	40,  
	.h_back_porch =   	110, 
	.h_front_porch =  	220, 
	.v_active = 	  	720,
	.v_pulse_width =  	5,  
	.v_back_porch =   	5, 
	.v_front_porch =  	20, 
	.neg_vclk = 	  	0, 
	.neg_hsync = 	  	0, 
	.neg_vsync = 	  	0, 
	.neg_vden = 	  	0, 
	.display_depth =  	24, 
	.display_type =   	DISPLAY_TYPE_DP,     
	.display_config =	NULL,
	},
	
	{ 
	.display_name =   	"720p-j34",	
	.host_clock_id =  	CLK_VCLK0, 
	.pixel_clock = 	  	74250000, 
	.dot_pitch = 	  	243, 
	.h_active =	  	1280,
	.h_pulse_width =  	40, 
	.h_back_porch =   	110, 
	.h_front_porch =  	220, 
	.v_active = 	  	720, 
	.v_pulse_width =  	5, 
	.v_back_porch =   	5, 
	.v_front_porch =  	20, 
	.neg_vclk = 	  	0, 
	.neg_hsync = 	  	0, 
	.neg_vsync = 	  	0, 
	.neg_vden = 	  	0, 
	.display_depth =  	24, 
	.display_type =   	DISPLAY_TYPE_DP,     
	.display_config =	NULL,
	},
#endif
#if DEBUG_BUILD || SUB_TARGET_J71 || SUB_TARGET_J72 || SUB_TARGET_J73
	{ 
	.display_name =   	"ipad4",	
	.host_clock_id =  	CLK_VCLK0, 
	.pixel_clock = 	  	200000000, 
	.dot_pitch = 	  	264, 
	.h_active =	  	1536,
	.h_pulse_width =  	15, 
	.h_back_porch =   	52, 
	.h_front_porch =  	16, 
	.v_active = 	  	2048,
	.v_pulse_width =  	1,  
	.v_back_porch =   	3, 
	.v_front_porch =  	7, 
	.neg_vclk = 	  	0, 
	.neg_hsync = 	  	0, 
	.neg_vsync = 	  	0, 
	.neg_vden = 	  	0, 
	.display_depth =  	24, 
	.display_type =   	DISPLAY_TYPE_EDP, 
	.display_config =	NULL,
	},
#endif
#if DEBUG_BUILD || SUB_TARGET_J85 || SUB_TARGET_J86 || SUB_TARGET_J87 || SUB_TARGET_J85M || SUB_TARGET_J86M || SUB_TARGET_J87M
	{ 
	.display_name =   	"ipad4b",	
	.host_clock_id =  	CLK_VCLK0, 
	.pixel_clock = 	  	200000000, 
	.dot_pitch = 	  	326, 
	.h_active =	  	1536,
	.h_pulse_width =  	16,  
	.h_back_porch =   	48, 
	.h_front_porch =  	12, 
	.v_active = 	  	2048,
	.v_pulse_width =  	4,  
	.v_back_porch =   	8, 
	.v_front_porch =  	8, 
	.neg_vclk = 	  	0, 
	.neg_hsync = 	  	0, 
	.neg_vsync = 	  	0, 
	.neg_vden = 	  	0, 
	.display_depth =  	24, 
	.display_type =   	DISPLAY_TYPE_EDP, 
	.display_config =	NULL,
	},
#endif
};
#endif	/* ! __APPLE_DISPLAY_TIMINGS_H */
