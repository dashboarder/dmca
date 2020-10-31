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
#if DEBUG_BUILD || SUB_TARGET_J1 || SUB_TARGET_J2 || SUB_TARGET_J2A
	{ 
	.display_name =   	"ipad3",	
	.host_clock_id =  	CLK_VCLK0, 
	.pixel_clock = 	  	205200000, 
	.dot_pitch = 	  	264, 
	.h_active =	  	2048, 
	.h_back_porch =   	80, 
	.h_front_porch =  	48, 
	.h_pulse_width =  	32, 
	.v_active = 	  	1536, 
	.v_back_porch =   	6, 
	.v_front_porch =  	3, 
	.v_pulse_width =  	4, 
	.neg_vclk = 	  	0, 
	.neg_hsync = 	  	0, 
	.neg_vsync = 	 	0, 
	.neg_vden = 	  	0, 
	.display_depth =  	24, 
	.display_type =   	DISPLAY_TYPE_EDP, 
	},
#endif
#if DEBUG_BUILD || WITH_HW_DISPLAY_HDMI
	{ 
	.display_name =   	"720p",	
	.host_clock_id =  	CLK_VCLK1, 
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
	.neg_vsync = 	 	0, 
	.neg_vden = 	  	0, 
	.display_depth =  	24, 
	.display_type =   	DISPLAY_TYPE_HDMI,     
	},
#endif
};
#endif	/* ! __APPLE_DISPLAY_TIMINGS_H */
