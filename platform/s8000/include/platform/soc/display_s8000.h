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
		.disp_dpcclklvl_clock_on_level = 	ADPCCLKLVL_ONLVL(0x637),
		.disp_dpcclklvl_clock_off_level = 	ADPCCLKLVL_OFFLVL(0x7f8),
		.disp_dpcqoscfg = 			(ADPCQOSCFG_QOS_TIMER(0x1b1d)	| \
					 		 ADPCQOSCFG_PIPE_ENABLE		| \
					 		 ADPCQOSCFG_QOS_ENABLE),
		.disp_dpcqosylvl_yellow_off = 		ADPCQOSYLVL_YELLOW_OFF(0x635),
		.disp_dpcqosylvl_yellow_on = 		ADPCQOSYLVL_YELLOW_ON(0x62d),
		.disp_dpcqosrlvl_red_off = 		ADPCQOSRLVL_RED_OFF(0x491),
		.disp_dpcqosrlvl_red_on = 		ADPCQOSRLVL_RED_ON(0x489),
		.disp_dpclinkdown = 			(ADPCLINKDOWN_TEARDOWNNOSCL | \
							 ADPCLINKDOWN_TEARDOWNEN),
		.disp_dpgpreqaggr = 			(ADPGPREQAGGR_REQAGGRTHRESH(0x159) | \
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
		.disp_dpcclklvl_clock_on_level = 	ADPCCLKLVL_ONLVL(0x321),
		.disp_dpcclklvl_clock_off_level = 	ADPCCLKLVL_OFFLVL(0x7f8),
		.disp_dpcqoscfg = 			(ADPCQOSCFG_QOS_TIMER(0x2d3a)	| \
							 ADPCQOSCFG_PIPE_ENABLE		| \
							 ADPCQOSCFG_QOS_ENABLE),
		.disp_dpcqosylvl_yellow_off = 		ADPCQOSYLVL_YELLOW_OFF(0x31f),
		.disp_dpcqosylvl_yellow_on = 		ADPCQOSYLVL_YELLOW_ON(0x317),
		.disp_dpcqosrlvl_red_off = 		ADPCQOSRLVL_RED_OFF(0x24d),
		.disp_dpcqosrlvl_red_on = 		ADPCQOSRLVL_RED_ON(0x245),
		.disp_dpclinkdown = 			(ADPCLINKDOWN_TEARDOWNNOSCL | \
							 ADPCLINKDOWN_TEARDOWNEN),
		.disp_dpgpreqaggr = 			(ADPGPREQAGGR_REQAGGRTHRESH(0xa6) | \
							 ADPGPREQAGGR_CACHEHINT(0x3) | \
							 ADPGPREQARRG_OPPORTUNISMENAB | \
							 ADPGPREQAGGR_REQAGGRENAB),
		.disp_dpcafclkgate =			(ADPAFIDLECOUNT(0) | \
							 ADPAFDYNCLKGATEEN),
		.disp_dpcenab = 			(ADPCENAB_IDEAL_ARB_ENAB),
	},
#endif //DISPLAY_D520_TUNABLES

#if DISPLAY_D403_TUNABLES
	{
		.name = 				"D403",
		.disp_dpcclkcntl = 			(ADPCCLKCNTL_FLOOR(0x60) | \
							 ADPCCLKCNTL_PIPE_ENABLE | \
							 ADPCCLKCNTL_LB_ENABLE	| \
							 ADPCCLKCNTL_PIO_GATE_ENABLE | \
							 ADPCCLKCNTL_STAT_GATE_ENABLE | \
							 ADPCCLKCNTL_EXT_GATE_ENABLE | \
							 ADPCCLKCNTL_DYN_GATE_ENABLE),
		.disp_dpcclklvl_clock_on_level = 	ADPCCLKLVL_ONLVL(0x321),
		.disp_dpcclklvl_clock_off_level = 	ADPCCLKLVL_OFFLVL(0x7f8),
		.disp_dpcqoscfg = 			(ADPCQOSCFG_QOS_TIMER(0x2d3a)	| \
							 ADPCQOSCFG_PIPE_ENABLE		| \
							 ADPCQOSCFG_QOS_ENABLE),
		.disp_dpcqosylvl_yellow_off = 		ADPCQOSYLVL_YELLOW_OFF(0x31f),
		.disp_dpcqosylvl_yellow_on = 		ADPCQOSYLVL_YELLOW_ON(0x317),
		.disp_dpcqosrlvl_red_off = 		ADPCQOSRLVL_RED_OFF(0x24d),
		.disp_dpcqosrlvl_red_on = 		ADPCQOSRLVL_RED_ON(0x245),
		.disp_dpclinkdown = 			(ADPCLINKDOWN_TEARDOWNNOSCL  | \
							 ADPCLINKDOWN_TEARDOWNEN),
		.disp_dpgpreqaggr = 			(ADPGPREQAGGR_REQAGGRTHRESH(0xa6) | \
							 ADPGPREQAGGR_CACHEHINT(0x3) | \
							 ADPGPREQARRG_OPPORTUNISMENAB | \
							 ADPGPREQAGGR_REQAGGRENAB),
		.disp_dpcafclkgate =			(ADPAFIDLECOUNT(0) | \
							 ADPAFDYNCLKGATEEN),
		.disp_dpcenab = 			(ADPCENAB_IDEAL_ARB_ENAB),
	},

#endif //DISPLAY_D403_TUNABLES

#endif	//DISPLAY_IPHONE_TUNABLES

};	


#endif	/* ! __APPLE_DISPLAY_H */