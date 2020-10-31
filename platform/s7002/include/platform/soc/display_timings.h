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
#if DEBUG_BUILD || SUB_TARGET_N27 || SUB_TARGET_N28 || SUB_TARGET_N27A || SUB_TARGET_N28A || SUB_TARGET_S7002SIM
	{ 
	.display_name =   	"C1",       
	.host_clock_id =  	CLK_VCLK0, 
	.pixel_clock = 	  	6412500,
	.dot_pitch = 	  	326, 
	.h_active =	  	232,
	.h_pulse_width =  	2,  
	.h_back_porch =   	44, 
	.h_front_porch =  	42, 
	.v_active = 	  	288,
	.v_pulse_width =  	1, 
	.v_back_porch =   	23, 
	.v_front_porch =  	22, 
	.neg_vclk = 	  	0, 
	.neg_hsync = 	  	0, 
	.neg_vsync = 	  	0, 
	.neg_vden = 	  	0, 
	.display_depth =  	24, 
	.display_type =   	DISPLAY_TYPE_SUMMIT, 
	.display_config = 	NULL,
	},
	
	{ 
	.display_name =   	"POR_DISP_S",       
	.host_clock_id =  	CLK_VCLK0, 
	.pixel_clock = 	  	22200000, 
	.dot_pitch = 	  	326, 
	.h_active =	  	272,
	.h_pulse_width =  	8,  
	.h_back_porch =   	8, 
	.h_front_porch =  	16, 
	.v_active = 	  	340,
	.v_pulse_width =  	1,  
	.v_back_porch =   	1, 
	.v_front_porch =  	43, 
	.neg_vclk = 	  	0, 
	.neg_hsync = 	  	0, 
	.neg_vsync = 	  	0, 
	.neg_vden = 	  	0, 
	.display_depth =  	24, 
	.display_type =   	DISPLAY_TYPE_SUMMIT, 
	.display_config = 	NULL,
	},
	
	{ 
	.display_name =   	"POR_DISP_B",       
	.host_clock_id =  	CLK_VCLK0, 
	.pixel_clock = 	  	22200000, 
	.dot_pitch = 	  	326, 
	.h_active =	  	312,
	.h_pulse_width =  	8, 
	.h_back_porch =   	8, 
	.h_front_porch =  	16, 
	.v_active = 	  	390,
	.v_pulse_width =  	1,
	.v_back_porch =   	1, 
	.v_front_porch =  	38, 
	.neg_vclk = 	  	0, 
	.neg_hsync = 	  	0, 
	.neg_vsync = 	  	0, 
	.neg_vden = 	  	0, 
	.display_depth =  	24, 
	.display_type =   	DISPLAY_TYPE_SUMMIT, 
	.display_config = 	NULL,
	},

	{ 
	.display_name =   	"POR_DISP_B_VID",       
	.host_clock_id =  	CLK_VCLK0, 
	.pixel_clock = 	  	12000000, 
	.dot_pitch = 	  	326, 
	.h_active =	  	312,
	.h_pulse_width =  	4, 
	.h_back_porch =   	102, 
	.h_front_porch =  	42, 
	.v_active = 	  	390,
	.v_pulse_width =  	1,
	.v_back_porch =   	23, 
	.v_front_porch =  	22, 
	.neg_vclk = 	  	0, 
	.neg_hsync = 	  	0, 
	.neg_vsync = 	  	0, 
	.neg_vden = 	  	0, 
	.display_depth =  	24, 
	.display_type =   	DISPLAY_TYPE_SUMMIT, 
	.display_config = 	NULL,
	},
#endif
};
#endif	/* ! __APPLE_DISPLAY_TIMINGS_H */
