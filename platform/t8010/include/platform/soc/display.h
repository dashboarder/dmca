/*
 * Copyright (C) 2013-2014 Apple Inc. All rights reserved.
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

// Platform-specific displaypipe tunables

// XXX - disp_dpcafclkgate is listed as TBD
// D620 is most likely something new based on the values. Update once we know more.

static struct adfe_v2_tuneable adfe_tuneables[] = 
{
#if DISPLAY_IPHONE_TUNABLES
#if DISPLAY_D620_TUNABLES
	{ 
		.name = 				"D620",
		.disp_dpcclkcntl = 			(ADPCCLKCNTL_FLOOR(0x60) | \
					 	 	 ADPCCLKCNTL_PIPE_ENABLE | \
					 	 	 ADPCCLKCNTL_LB_ENABLE	| \
					 	 	 ADPCCLKCNTL_PIO_GATE_ENABLE | \
					 	 	 ADPCCLKCNTL_STAT_GATE_ENABLE | \
					 	 	 ADPCCLKCNTL_EXT_GATE_ENABLE | \
					 	 	 ADPCCLKCNTL_DYN_GATE_ENABLE),
		.disp_dpcclklvl_clock_on_level = 	ADPCCLKLVL_ONLVL(0x709),
		.disp_dpcclklvl_clock_off_level = 	ADPCCLKLVL_OFFLVL(0x7f8),
		.disp_dpcqoscfg = 			(ADPCQOSCFG_QOS_TIMER(0xffff)	| \
					 		 ADPCQOSCFG_PIPE_ENABLE		| \
					 		 ADPCQOSCFG_QOS_ENABLE),
		.disp_dpcqosylvl_yellow_off = 		ADPCQOSYLVL_YELLOW_OFF(0x6ff),
		.disp_dpcqosylvl_yellow_on = 		ADPCQOSYLVL_YELLOW_ON(0x6f7),
		.disp_dpcqosrlvl_red_off = 		ADPCQOSRLVL_RED_OFF(0x5f6),
		.disp_dpcqosrlvl_red_on = 		ADPCQOSRLVL_RED_ON(0x5ee),
		.disp_dpclinkdown = 			(ADPCLINKDOWN_TEARDOWNNOSCL | \
							 ADPCLINKDOWN_TEARDOWNEN),
		.disp_dpgpreqaggr = 			(ADPGPREQAGGR_REQAGGRTHRESH(0xe1) | \
							 ADPGPREQAGGR_CACHEHINT(0x3) | \
							 ADPGPREQARRG_OPPORTUNISMENAB | \
							 ADPGPREQAGGR_REQAGGRENAB),
		.disp_dpcafclkgate =			(ADPAFIDLECOUNT(0) | \
							 ADPAFDYNCLKGATEEN),
		.disp_dpcenab = 			(ADPCENAB_IDEAL_ARB_ENAB),
	},
#endif //DISPLAY_D620_TUNABLES

#if DISPLAY_D520_TUNABLES
	{ 
		.name = 				"D520",
		.disp_dpcclkcntl = 			(ADPCCLKCNTL_FLOOR(0x60) | \
							 ADPCCLKCNTL_PIPE_ENABLE | \
							 ADPCCLKCNTL_LB_ENABLE	| \
							 ADPCCLKCNTL_PIO_GATE_ENABLE | \
							 ADPCCLKCNTL_STAT_GATE_ENABLE | \
							 ADPCCLKCNTL_EXT_GATE_ENABLE | \
							 ADPCCLKCNTL_DYN_GATE_ENABLE),
		.disp_dpcclklvl_clock_on_level = 	ADPCCLKLVL_ONLVL(0x3db),
		.disp_dpcclklvl_clock_off_level = 	ADPCCLKLVL_OFFLVL(0x7f8),
		.disp_dpcqoscfg = 			(ADPCQOSCFG_QOS_TIMER(0x7a7)	| \
							 ADPCQOSCFG_PIPE_ENABLE		| \
							 ADPCQOSCFG_QOS_ENABLE),
		.disp_dpcqosylvl_yellow_off = 		ADPCQOSYLVL_YELLOW_OFF(0x3d1),
		.disp_dpcqosylvl_yellow_on = 		ADPCQOSYLVL_YELLOW_ON(0x3c9),
		.disp_dpcqosrlvl_red_off = 		ADPCQOSRLVL_RED_OFF(0x2ff),
		.disp_dpcqosrlvl_red_on = 		ADPCQOSRLVL_RED_ON(0x2f7),
		.disp_dpclinkdown = 			(ADPCLINKDOWN_TEARDOWNNOSCL | \
							 ADPCLINKDOWN_TEARDOWNEN),
		.disp_dpgpreqaggr = 			(ADPGPREQAGGR_REQAGGRTHRESH(0x6b) | \
							 ADPGPREQAGGR_CACHEHINT(0x3) | \
							 ADPGPREQARRG_OPPORTUNISMENAB | \
							 ADPGPREQAGGR_REQAGGRENAB),
		.disp_dpcafclkgate =			(ADPAFIDLECOUNT(0) | \
							 ADPAFDYNCLKGATEEN),
		.disp_dpcenab = 			(ADPCENAB_IDEAL_ARB_ENAB),
	},
#endif //DISPLAY_D520_TUNABLES
#endif	//DISPLAY_IPHONE_TUNABLES

//Tunables for targets using the the timing based on landscape ipads
#if DISPLAY_LANDSCAPE_IPAD_TUNABLES || DISPLAY_APPLE_TV_TUNABLES || DISPLAY_FPGA_TUNABLES
	{ 
		.name = 				"default",
		.disp_dpcclkcntl = 			(ADPCCLKCNTL_FLOOR(0x60) | \
							 ADPCCLKCNTL_LB_ENABLE	| \
							 ADPCCLKCNTL_PIO_GATE_ENABLE | \
							 ADPCCLKCNTL_STAT_GATE_ENABLE | \
							 ADPCCLKCNTL_EXT_GATE_ENABLE | \
							 ADPCCLKCNTL_DYN_GATE_ENABLE),
		.disp_dpcclklvl_clock_on_level = 	ADPCCLKLVL_ONLVL(0x547),
		.disp_dpcclklvl_clock_off_level = 	ADPCCLKLVL_OFFLVL(0x7f8),
		.disp_dpcqoscfg = 			(ADPCQOSCFG_QOS_TIMER(0x3a2c)	| \
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
							 ADPGPREQARRG_OPPORTUNISMENAB | \
							 ADPGPREQAGGR_REQAGGRENAB),
		.disp_dpcafclkgate =			(ADPAFIDLECOUNT(0) | \
							 ADPAFDYNCLKGATEEN),
		.disp_dpcenab = 			(ADPCENAB_IDEAL_ARB_ENAB),
	},
#if DISPLAY_IPHONE_TUNABLES
#error DISPLAY_IPHONE_TUNABLES set for IPAD
#endif

#endif //DISPLAY_LANDSCAPE_IPAD_TUNABLES || DISPLAY_APPLE_TV_TUNABLES || DISPLAY_FPGA_TUNABLES
};

#endif	/* ! __APPLE_DISPLAY_H */
