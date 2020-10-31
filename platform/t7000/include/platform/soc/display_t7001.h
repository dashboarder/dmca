/*
 * Copyright (C) 2012-2014 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */
#ifndef __APPLE_DISPLAY_H
#define __APPLE_DISPLAY_H

static struct adfe_v2_tuneable adfe_tuneables[] = 
{
//Tunables for targets using the the timing based on landscape ipads
#if DISPLAY_LANDSCAPE_IPAD_TUNABLES || DISPLAY_APPLE_TV_TUNABLES || DISPLAY_FPGA_TUNABLES
#if DISPLAY_X136_TUNABLES
	{ 
		.name = 				"default",
		.disp_dpcclkcntl = 			(ADPCCLKCNTL_FLOOR(0x20) | \
							 ADPCCLKCNTL_PIPE_ENABLE | \
							 ADPCCLKCNTL_LB_ENABLE	| \
							 ADPCCLKCNTL_PIO_GATE_ENABLE | \
							 ADPCCLKCNTL_STAT_GATE_ENABLE | \
							 ADPCCLKCNTL_EXT_GATE_ENABLE | \
							 ADPCCLKCNTL_DYN_GATE_ENABLE),
		.disp_dpcclklvl_clock_on_level = 	ADPCCLKLVL_ONLVL(0x904),
		.disp_dpcclklvl_clock_off_level = 	ADPCCLKLVL_OFFLVL(0xff8),
		.disp_dpcqoscfg = 			(ADPCQOSCFG_QOS_TIMER(0x1a24)	| \
							 ADPCQOSCFG_PIPE_ENABLE | \
							 ADPCQOSCFG_QOS_ENABLE),
		.disp_dpcqosylvl_yellow_off = 		ADPCQOSYLVL_YELLOW_OFF(0x90e),
		.disp_dpcqosylvl_yellow_on = 		ADPCQOSYLVL_YELLOW_ON(0x8fa),
		.disp_dpcqosrlvl_red_off = 		ADPCQOSRLVL_RED_OFF(0x63e),
		.disp_dpcqosrlvl_red_on = 		ADPCQOSRLVL_RED_ON(0x62a),
		.disp_dpclinkdown = 			(ADPCLINKDOWN_TEARDOWNNOSCL | \
							 ADPCLINKDOWN_TEARDOWNEN),
		.disp_dpgpreqaggr = 			(ADPGPREQAGGR_REQAGGRTHRESH(0x4b) | \
							 ADPGPREQAGGR_CACHEHINT(0x3) | \
							 ADPGPREQAGGR_REQAGGRENAB),
		.disp_dpcafclkgate =			(ADPAFIDLECOUNT(0x30) | \
							 ADPAFDYNCLKGATEEN),			
		.disp_dpcenab = 			(ADPCENAB_IDEAL_ARB_ENAB),
	},
	
	{ 
		// XXX - needs updated with correct values!
		.name = 				"camelia",
		.disp_dpcclkcntl = 			(0),
		.disp_dpcclklvl_clock_on_level = 	ADPCCLKLVL_ONLVL(0x800),
		.disp_dpcclklvl_clock_off_level = 	ADPCCLKLVL_OFFLVL(0x800),
		.disp_dpcqoscfg = 			(0),
		.disp_dpcqosylvl_yellow_off = 		ADPCQOSYLVL_YELLOW_OFF(0),
		.disp_dpcqosylvl_yellow_on = 		ADPCQOSYLVL_YELLOW_ON(0),
		.disp_dpcqosrlvl_red_off = 		ADPCQOSRLVL_RED_OFF(0),
		.disp_dpcqosrlvl_red_on = 		ADPCQOSRLVL_RED_ON(0),
		.disp_dpclinkdown = 			(0),
		.disp_dpgpreqaggr = 			(0),
		.disp_dpcafclkgate =			(ADPAFIDLECOUNT(0x18)),			
		.disp_dpcenab = 			(0),
	},
#endif

#if !DISPLAY_X136_TUNABLES
	{ 
		.name = 				"default",
		.disp_dpcclkcntl = 			(ADPCCLKCNTL_FLOOR(0x20) | \
							 ADPCCLKCNTL_PIPE_ENABLE | \
							 ADPCCLKCNTL_LB_ENABLE	| \
							 ADPCCLKCNTL_PIO_GATE_ENABLE | \
							 ADPCCLKCNTL_STAT_GATE_ENABLE | \
							 ADPCCLKCNTL_EXT_GATE_ENABLE | \
							 ADPCCLKCNTL_DYN_GATE_ENABLE),
		.disp_dpcclklvl_clock_on_level = 	ADPCCLKLVL_ONLVL(0x547),
		.disp_dpcclklvl_clock_off_level = 	ADPCCLKLVL_OFFLVL(0xff8),
		.disp_dpcqoscfg = 			(ADPCQOSCFG_QOS_TIMER(0x15b5)	| \
							 ADPCQOSCFG_PIPE_ENABLE | \
							 ADPCQOSCFG_QOS_ENABLE),
		.disp_dpcqosylvl_yellow_off = 		ADPCQOSYLVL_YELLOW_OFF(0x545),
		.disp_dpcqosylvl_yellow_on = 		ADPCQOSYLVL_YELLOW_ON(0x53d),
		.disp_dpcqosrlvl_red_off = 		ADPCQOSRLVL_RED_OFF(0x3a1),
		.disp_dpcqosrlvl_red_on = 		ADPCQOSRLVL_RED_ON(0x399),
		.disp_dpclinkdown = 			(ADPCLINKDOWN_TEARDOWNNOSCL | \
							 ADPCLINKDOWN_TEARDOWNEN),
		.disp_dpgpreqaggr = 			(ADPGPREQAGGR_REQAGGRTHRESH(0x4b) | \
							 ADPGPREQAGGR_CACHEHINT(0x3) | \
							 ADPGPREQAGGR_REQAGGRENAB),
		.disp_dpcafclkgate =			(ADPAFIDLECOUNT(0x30) | \
							 ADPAFDYNCLKGATEEN),					
		.disp_dpcenab = 			(ADPCENAB_IDEAL_ARB_ENAB),
	},
#endif //!DISPLAY_X136_TUNABLES
	
#if DISPLAY_IPHONE_TUNABLES
#error DISPLAY_IPHONE_TUNABLES set for IPAD
#endif
	
#endif //DISPLAY_LANDSCAPE_IPAD_TUNABLES || DISPLAY_APPLE_TV_TUNABLES || DISPLAY_FPGA_TUNABLES
};

#endif	/* ! __APPLE_DISPLAY_H */
