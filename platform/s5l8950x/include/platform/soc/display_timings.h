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
#if DEBUG_BUILD || SUB_TARGET_N41 || SUB_TARGET_N42 || SUB_TARGET_N48 || SUB_TARGET_N49
	{ 
	.display_name = 	"n41",	
	.host_clock_id = 	CLK_VCLK0, 
	.pixel_clock = 		68400000, 
	.dot_pitch = 		326, 
	.h_active =		640,
	.h_pulse_width = 	40, 
	.h_back_porch = 	40, 
	.h_front_porch =        40, 
	.v_active = 		1136,
	.v_pulse_width =  	3, 
	.v_back_porch = 	13, 
	.v_front_porch =        348, 
	.neg_vclk = 		0, 
	.neg_hsync = 		0, 
	.neg_vsync = 		0, 
	.neg_vden = 		0, 
	.display_depth = 	24, 
	.display_type = 	DISPLAY_TYPE_PINOT, 
	.display_config		= NULL,
	},	// mipi_dsi_clk @ 513MHz, 4-lane
#endif
};
#endif	/* ! __APPLE_DISPLAY_TIMINGS_H */
