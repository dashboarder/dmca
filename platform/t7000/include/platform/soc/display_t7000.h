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
#if DISPLAY_IPHONE_TUNABLES
#if DISPLAY_D600_TUNABLES
	{ 
		.name = 				"D600",
		.disp_dpcclkcntl = 			(ADPCCLKCNTL_FLOOR(0x20) | \
					 	 	 ADPCCLKCNTL_PIPE_ENABLE | \
					 	 	 ADPCCLKCNTL_LB_ENABLE	| \
					 	 	 ADPCCLKCNTL_PIO_GATE_ENABLE | \
					 	 	 ADPCCLKCNTL_STAT_GATE_ENABLE | \
					 	 	 ADPCCLKCNTL_EXT_GATE_ENABLE | \
					 	 	 ADPCCLKCNTL_DYN_GATE_ENABLE),
		.disp_dpcclklvl_clock_on_level = 	ADPCCLKLVL_ONLVL(0x617),
		.disp_dpcclklvl_clock_off_level = 	ADPCCLKLVL_OFFLVL(0x7f8),
		.disp_dpcqoscfg = 			(ADPCQOSCFG_QOS_TIMER(0x1082)	| \
					 		 ADPCQOSCFG_PIPE_ENABLE		| \
					 		 ADPCQOSCFG_QOS_ENABLE),
		.disp_dpcqosylvl_yellow_off = 		ADPCQOSYLVL_YELLOW_OFF(0x61d),
		.disp_dpcqosylvl_yellow_on = 		ADPCQOSYLVL_YELLOW_ON(0x60d),
		.disp_dpcqosrlvl_red_off = 		ADPCQOSRLVL_RED_OFF(0x453),
		.disp_dpcqosrlvl_red_on = 		ADPCQOSRLVL_RED_ON(0x443),
		.disp_dpclinkdown = 			(ADPCLINKDOWN_TEARDOWNNOSCL | \
							 ADPCLINKDOWN_TEARDOWNEN),
		.disp_dpgpreqaggr = 			(ADPGPREQAGGR_REQAGGRTHRESH(0x4b) | \
							 ADPGPREQAGGR_CACHEHINT(0x3) | \
							 ADPGPREQAGGR_REQAGGRENAB),
		.disp_dpcafclkgate =			0,
		.disp_dpcenab = 			(ADPCENAB_IDEAL_ARB_ENAB),
	},
#endif //DISPLAY_D600_TUNABLES


#if DISPLAY_D500_TUNABLES || DISPLAY_D410_TUNABLES
	{ 
#if DISPLAY_D500_TUNABLES
		.name = 				"D500",
#else
		.name = 				"D410",
#endif
		.disp_dpcclkcntl = 			(ADPCCLKCNTL_FLOOR(0x20) | \
							 ADPCCLKCNTL_PIPE_ENABLE | \
							 ADPCCLKCNTL_LB_ENABLE	| \
							 ADPCCLKCNTL_PIO_GATE_ENABLE | \
							 ADPCCLKCNTL_STAT_GATE_ENABLE | \
							 ADPCCLKCNTL_EXT_GATE_ENABLE | \
							 ADPCCLKCNTL_DYN_GATE_ENABLE),
		.disp_dpcclklvl_clock_on_level = 	ADPCCLKLVL_ONLVL(0x282),
		.disp_dpcclklvl_clock_off_level = 	ADPCCLKLVL_OFFLVL(0x7f8),
		.disp_dpcqoscfg = 			(ADPCQOSCFG_QOS_TIMER(0x17c5)	| \
							 ADPCQOSCFG_PIPE_ENABLE		| \
							 ADPCQOSCFG_QOS_ENABLE),
		.disp_dpcqosylvl_yellow_off = 		ADPCQOSYLVL_YELLOW_OFF(0x280),
		.disp_dpcqosylvl_yellow_on = 		ADPCQOSYLVL_YELLOW_ON(0x278),
		.disp_dpcqosrlvl_red_off = 		ADPCQOSRLVL_RED_OFF(0x1c5),
		.disp_dpcqosrlvl_red_on = 		ADPCQOSRLVL_RED_ON(0x1bd),
		.disp_dpclinkdown = 			(ADPCLINKDOWN_TEARDOWNNOSCL | \
							 ADPCLINKDOWN_TEARDOWNEN),
		.disp_dpgpreqaggr = 			(ADPGPREQAGGR_REQAGGRTHRESH(0x4b) | \
							 ADPGPREQAGGR_CACHEHINT(0x3) | \
							 ADPGPREQAGGR_REQAGGRENAB),
		.disp_dpcafclkgate =			0,
		.disp_dpcenab = 			(ADPCENAB_IDEAL_ARB_ENAB),
	},
#endif //DISPLAY_D500_TUNABLES || DISPLAY_D410_TUNABLES

#if DISPLAY_D403_TUNABLES
	{ 
		.name = 				"D403",
		.disp_dpcclkcntl = 			(ADPCCLKCNTL_FLOOR(0x20) | \
							 ADPCCLKCNTL_PIPE_ENABLE | \
							 ADPCCLKCNTL_LB_ENABLE	| \
							 ADPCCLKCNTL_PIO_GATE_ENABLE | \
							 ADPCCLKCNTL_STAT_GATE_ENABLE | \
							 ADPCCLKCNTL_EXT_GATE_ENABLE | \
							 ADPCCLKCNTL_DYN_GATE_ENABLE),
		.disp_dpcclklvl_clock_on_level = 	ADPCCLKLVL_ONLVL(0x1BF),
		.disp_dpcclklvl_clock_off_level = 	ADPCCLKLVL_OFFLVL(0x7f8),
		.disp_dpcqoscfg = 			(ADPCQOSCFG_QOS_TIMER(0x74A)	| \
							 ADPCQOSCFG_PIPE_ENABLE		| \
							 ADPCQOSCFG_QOS_ENABLE),
		.disp_dpcqosylvl_yellow_off = 		ADPCQOSYLVL_YELLOW_OFF(0x1C5),
		.disp_dpcqosylvl_yellow_on = 		ADPCQOSYLVL_YELLOW_ON(0x1B5),
		.disp_dpcqosrlvl_red_off = 		ADPCQOSRLVL_RED_OFF(0x144),
		.disp_dpcqosrlvl_red_on = 		ADPCQOSRLVL_RED_ON(0x134),
		.disp_dpclinkdown = 			(ADPCLINKDOWN_TEARDOWNNOSCL | \
							 ADPCLINKDOWN_TEARDOWNEN),
		.disp_dpgpreqaggr = 			(ADPGPREQAGGR_REQAGGRTHRESH(0x4b) | \
							 ADPGPREQAGGR_CACHEHINT(0x3) | \
							 ADPGPREQAGGR_REQAGGRENAB),
		.disp_dpcafclkgate =			0,
		.disp_dpcenab = 			(ADPCENAB_IDEAL_ARB_ENAB),
	},
#endif // DISPLAY_D403_TUNABLES

#if DISPLAY_D500_2LANE_TUNABLES
	{ 
		.name = 				"D500_2LANE",
		.disp_dpcclkcntl = 			(ADPCCLKCNTL_FLOOR(0x20) | \
							 ADPCCLKCNTL_PIPE_ENABLE | \
							 ADPCCLKCNTL_LB_ENABLE	| \
							 ADPCCLKCNTL_PIO_GATE_ENABLE | \
							 ADPCCLKCNTL_STAT_GATE_ENABLE | \
							 ADPCCLKCNTL_EXT_GATE_ENABLE | \
							 ADPCCLKCNTL_DYN_GATE_ENABLE),
		.disp_dpcclklvl_clock_on_level = 	ADPCCLKLVL_ONLVL(0x282),
		.disp_dpcclklvl_clock_off_level = 	ADPCCLKLVL_OFFLVL(0x7f8),
		.disp_dpcqoscfg = 			(ADPCQOSCFG_QOS_TIMER(0x17c5)	| \
							 ADPCQOSCFG_PIPE_ENABLE		| \
							 ADPCQOSCFG_QOS_ENABLE),
		.disp_dpcqosylvl_yellow_off = 		ADPCQOSYLVL_YELLOW_OFF(0x280),
		.disp_dpcqosylvl_yellow_on = 		ADPCQOSYLVL_YELLOW_ON(0x278),
		.disp_dpcqosrlvl_red_off = 		ADPCQOSRLVL_RED_OFF(0x1c5),
		.disp_dpcqosrlvl_red_on = 		ADPCQOSRLVL_RED_ON(0x1bd),
		.disp_dpclinkdown = 			(ADPCLINKDOWN_TEARDOWNNOSCL | \
							 ADPCLINKDOWN_TEARDOWNEN),
		.disp_dpgpreqaggr = 			(ADPGPREQAGGR_REQAGGRTHRESH(0x4b) | \
							 ADPGPREQAGGR_CACHEHINT(0x3) | \
							 ADPGPREQAGGR_REQAGGRENAB),
		.disp_dpcafclkgate =			0,
		.disp_dpcenab = 			(ADPCENAB_IDEAL_ARB_ENAB),
	},
#endif //DISPLAY_D500_2LANE_TUNABLES
#endif	//DISPLAY_IPHONE_TUNABLES

//Tunables for targets using the the timing based on landscape ipads
#if DISPLAY_LANDSCAPE_IPAD_TUNABLES || DISPLAY_APPLE_TV_TUNABLES || DISPLAY_FPGA_TUNABLES
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
		.disp_dpcclklvl_clock_off_level = 	ADPCCLKLVL_OFFLVL(0x7f8),
		.disp_dpcqoscfg = 			(ADPCQOSCFG_QOS_TIMER(0x159a)	| \
							 ADPCQOSCFG_PIPE_ENABLE | \
							 ADPCQOSCFG_QOS_ENABLE),
		.disp_dpcqosylvl_yellow_off = 		ADPCQOSYLVL_YELLOW_OFF(0x54d),
		.disp_dpcqosylvl_yellow_on = 		ADPCQOSYLVL_YELLOW_ON(0x53d),
		.disp_dpcqosrlvl_red_off = 		ADPCQOSRLVL_RED_OFF(0x3a9),
		.disp_dpcqosrlvl_red_on = 		ADPCQOSRLVL_RED_ON(0x399),
		.disp_dpclinkdown = 			(ADPCLINKDOWN_TEARDOWNNOSCL | \
							 ADPCLINKDOWN_TEARDOWNEN),
		.disp_dpgpreqaggr = 			(ADPGPREQAGGR_REQAGGRTHRESH(0x4b) | \
							 ADPGPREQAGGR_CACHEHINT(0x3) | \
							 ADPGPREQAGGR_REQAGGRENAB),
		.disp_dpcafclkgate =			0,
		.disp_dpcenab = 			(ADPCENAB_IDEAL_ARB_ENAB),
	},
	
#if DISPLAY_IPHONE_TUNABLES
#error DISPLAY_IPHONE_TUNABLES set for IPAD
#endif
	
#endif //DISPLAY_LANDSCAPE_IPAD_TUNABLES || DISPLAY_APPLE_TV_TUNABLES || DISPLAY_FPGA_TUNABLES
};

#endif	/* ! __APPLE_DISPLAY_H */
