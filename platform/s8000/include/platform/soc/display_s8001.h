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
//Tunables for targets using the the timing based on landscape ipads
#if DISPLAY_LANDSCAPE_IPAD_TUNABLES ||  DISPLAY_FPGA_TUNABLES || DISPLAY_APPLE_TV_TUNABLES
	{ 
		.name = 				"ipad4",
		.disp_dpcclkcntl = 			(ADPCCLKCNTL_FLOOR(0x20) | \
							 ADPCCLKCNTL_PIPE_ENABLE | \
							 ADPCCLKCNTL_LB_ENABLE	 | \
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
	{ 
		.name = 				"ipad6",
		.disp_dpcclkcntl = 			(ADPCCLKCNTL_FLOOR(0x60) | \
							 ADPCCLKCNTL_PIPE_ENABLE | \
							 ADPCCLKCNTL_LB_ENABLE	 | \
							 ADPCCLKCNTL_PIO_GATE_ENABLE | \
							 ADPCCLKCNTL_STAT_GATE_ENABLE | \
							 ADPCCLKCNTL_EXT_GATE_ENABLE | \
							 ADPCCLKCNTL_DYN_GATE_ENABLE),
		.disp_dpcclklvl_clock_on_level = 	ADPCCLKLVL_ONLVL(0x6c3),
		.disp_dpcclklvl_clock_off_level = 	ADPCCLKLVL_OFFLVL(0x7f8),
		.disp_dpcqoscfg = 			(ADPCQOSCFG_QOS_TIMER(0xaac)	| \
							 ADPCQOSCFG_PIPE_ENABLE | \
							 ADPCQOSCFG_QOS_ENABLE),
		.disp_dpcqosylvl_yellow_off = 		ADPCQOSYLVL_YELLOW_OFF(0x6c1),
		.disp_dpcqosylvl_yellow_on = 		ADPCQOSYLVL_YELLOW_ON(0x6b9),
		.disp_dpcqosrlvl_red_off = 		ADPCQOSRLVL_RED_OFF(0x579),
		.disp_dpcqosrlvl_red_on = 		ADPCQOSRLVL_RED_ON(0x571),
		.disp_dpclinkdown = 			(ADPCLINKDOWN_TEARDOWNNOSCL | \
							 ADPCLINKDOWN_TEARDOWNAGGR),
		.disp_dpgpreqaggr = 			(ADPGPREQAGGR_REQAGGRTHRESH(0x1a9) | \
							 ADPGPREQAGGR_CACHEHINT(0x3) | \
							 ADPGPREQARRG_OPPORTUNISMENAB | \
							 ADPGPREQAGGR_REQAGGRENAB),
		.disp_dpcafclkgate =			(ADPAFIDLECOUNT(0x0) | \
							 ADPAFDYNCLKGATEEN),					
		.disp_dpcenab = 			(ADPCENAB_IDEAL_ARB_ENAB),
	},
	{ 
		.name = 				"ipad6_A1",
		.disp_dpcclkcntl = 			(ADPCCLKCNTL_FLOOR(0x60) | \
							 ADPCCLKCNTL_PIPE_ENABLE | \
							 ADPCCLKCNTL_LB_ENABLE	| \
							 ADPCCLKCNTL_PIO_GATE_ENABLE | \
							 ADPCCLKCNTL_STAT_GATE_ENABLE | \
							 ADPCCLKCNTL_EXT_GATE_ENABLE | \
							 ADPCCLKCNTL_DYN_GATE_ENABLE),
		.disp_dpcclklvl_clock_on_level = 	ADPCCLKLVL_ONLVL(0x7a1),
		.disp_dpcclklvl_clock_off_level = 	ADPCCLKLVL_OFFLVL(0x7f8),
		.disp_dpcqoscfg = 			(ADPCQOSCFG_QOS_TIMER(0xaac)	| \
							 ADPCQOSCFG_PIPE_ENABLE | \
							 ADPCQOSCFG_QOS_ENABLE),
		.disp_dpcqosylvl_yellow_off = 		ADPCQOSYLVL_YELLOW_OFF(0x797),
		.disp_dpcqosylvl_yellow_on = 		ADPCQOSYLVL_YELLOW_ON(0x78f),
		.disp_dpcqosrlvl_red_off = 		ADPCQOSRLVL_RED_OFF(0x725),
		.disp_dpcqosrlvl_red_on = 		ADPCQOSRLVL_RED_ON(0x71d),
		.disp_dpclinkdown = 			(ADPCLINKDOWN_TEARDOWNNOSCL | \
							 ADPCLINKDOWN_TEARDOWNAGGR | \
							 ADPCLINKDOWN_TEARDOWNEN),
		.disp_dpgpreqaggr = 			(ADPGPREQAGGR_REQAGGRTHRESH(0x131) | \
							 ADPGPREQAGGR_CACHEHINT(0x3) | \
							 ADPGPREQARRG_OPPORTUNISMENAB | \
							 ADPGPREQAGGR_REQAGGRENAB),
		.disp_dpcafclkgate =			(ADPAFIDLECOUNT(0x0) | \
							 ADPAFDYNCLKGATEEN),					
		.disp_dpcenab = 			(ADPCENAB_IDEAL_ARB_ENAB),
	},
	{ 
		.name =                                 "ipad6b",
		.disp_dpcclkcntl =                      (ADPCCLKCNTL_FLOOR(0x20) | \
							 ADPCCLKCNTL_PIPE_ENABLE | \
							 ADPCCLKCNTL_LB_ENABLE	 | \
							 ADPCCLKCNTL_PIO_GATE_ENABLE | \
							 ADPCCLKCNTL_STAT_GATE_ENABLE | \
							 ADPCCLKCNTL_EXT_GATE_ENABLE | \
							 ADPCCLKCNTL_DYN_GATE_ENABLE),
		.disp_dpcclklvl_clock_on_level =        ADPCCLKLVL_ONLVL(0x8ab),
		.disp_dpcclklvl_clock_off_level =       ADPCCLKLVL_OFFLVL(0x1ff0),
		.disp_dpcqoscfg =                       (ADPCQOSCFG_QOS_TIMER(0x3840)   | \
							 ADPCQOSCFG_PIPE_ENABLE | \
							 ADPCQOSCFG_QOS_ENABLE),
		.disp_dpcqosylvl_yellow_off =           ADPCQOSYLVL_YELLOW_OFF(0x8aa),
		.disp_dpcqosylvl_yellow_on =            ADPCQOSYLVL_YELLOW_ON(0x8a2),
		.disp_dpcqosrlvl_red_off =              ADPCQOSRLVL_RED_OFF(0x6b2),
		.disp_dpcqosrlvl_red_on =               ADPCQOSRLVL_RED_ON(0x6aa),
		.disp_dpclinkdown =                     (ADPCLINKDOWN_TEARDOWNNOSCL | \
							 ADPCLINKDOWN_TEARDOWNEN),
		.disp_dpgpreqaggr =                     (ADPGPREQAGGR_REQAGGRTHRESH(0x14f) | \
							 ADPGPREQAGGR_CACHEHINT(0x3) | \
							 ADPGPREQARRG_OPPORTUNISMENAB | \
							 ADPGPREQAGGR_REQAGGRENAB),
		.disp_dpcafclkgate =                    (ADPAFIDLECOUNT(0x30) | \
							 ADPAFDYNCLKGATEEN),                                    
		.disp_dpcenab =                         (ADPCENAB_IDEAL_ARB_ENAB),
	},
	{ 
		.name =                                 "ipad6d",
		.disp_dpcclkcntl =                      (ADPCCLKCNTL_FLOOR(0x20) | \
							 ADPCCLKCNTL_PIPE_ENABLE | \
							 ADPCCLKCNTL_LB_ENABLE	 | \
							 ADPCCLKCNTL_PIO_GATE_ENABLE | \
							 ADPCCLKCNTL_STAT_GATE_ENABLE | \
							 ADPCCLKCNTL_EXT_GATE_ENABLE | \
							 ADPCCLKCNTL_DYN_GATE_ENABLE),
		.disp_dpcclklvl_clock_on_level =        ADPCCLKLVL_ONLVL(0xe61),
		.disp_dpcclklvl_clock_off_level =       ADPCCLKLVL_OFFLVL(0x1ff0),
		.disp_dpcqoscfg =                       (ADPCQOSCFG_QOS_TIMER(0x3840)   | \
							 ADPCQOSCFG_PIPE_ENABLE | \
							 ADPCQOSCFG_QOS_ENABLE),
		.disp_dpcqosylvl_yellow_off =           ADPCQOSYLVL_YELLOW_OFF(0xe60),
		.disp_dpcqosylvl_yellow_on =            ADPCQOSYLVL_YELLOW_ON(0xe58),
		.disp_dpcqosrlvl_red_off =              ADPCQOSRLVL_RED_OFF(0xb1a),
		.disp_dpcqosrlvl_red_on =               ADPCQOSRLVL_RED_ON(0xb12),
		.disp_dpclinkdown =                     (ADPCLINKDOWN_TEARDOWNNOSCL | \
							 ADPCLINKDOWN_TEARDOWNEN),
		.disp_dpgpreqaggr =                     (ADPGPREQAGGR_REQAGGRTHRESH(0x29d) | \
							 ADPGPREQAGGR_CACHEHINT(0x3) | \
							 ADPGPREQARRG_OPPORTUNISMENAB | \
							 ADPGPREQAGGR_REQAGGRENAB),
		.disp_dpcafclkgate =                    (ADPAFIDLECOUNT(0x30) | \
							 ADPAFDYNCLKGATEEN),                                    
		.disp_dpcenab =                         (ADPCENAB_IDEAL_ARB_ENAB),
	},
	{
		.name = 				"720p",
		.disp_dpcclkcntl = 			(ADPCCLKCNTL_FLOOR(0x20) | \
							 ADPCCLKCNTL_PIPE_ENABLE | \
							 ADPCCLKCNTL_LB_ENABLE	 | \
							 ADPCCLKCNTL_PIO_GATE_ENABLE | \
							 ADPCCLKCNTL_STAT_GATE_ENABLE | \
							 ADPCCLKCNTL_EXT_GATE_ENABLE | \
							 ADPCCLKCNTL_DYN_GATE_ENABLE),
		.disp_dpcclklvl_clock_on_level = 	ADPCCLKLVL_ONLVL(0x246),
		.disp_dpcclklvl_clock_off_level = 	ADPCCLKLVL_OFFLVL(0x1ff0),
		.disp_dpcqoscfg = 			(ADPCQOSCFG_QOS_TIMER(0x3840)	| \
							 ADPCQOSCFG_PIPE_ENABLE | \
							 ADPCQOSCFG_QOS_ENABLE),
		.disp_dpcqosylvl_yellow_off = 		ADPCQOSYLVL_YELLOW_OFF(0x244),
		.disp_dpcqosylvl_yellow_on = 		ADPCQOSYLVL_YELLOW_ON(0x23c),
		.disp_dpcqosrlvl_red_off = 		ADPCQOSRLVL_RED_OFF(0x1ac),
		.disp_dpcqosrlvl_red_on = 		ADPCQOSRLVL_RED_ON(0x1a4),
		.disp_dpclinkdown = 			(ADPCLINKDOWN_TEARDOWNNOSCL | \
							 ADPCLINKDOWN_TEARDOWNEN),
		.disp_dpgpreqaggr = 			(ADPGPREQAGGR_REQAGGRTHRESH(0x59) | \
							 ADPGPREQAGGR_CACHEHINT(0x3) | \
							 ADPGPREQARRG_OPPORTUNISMENAB | \
							 ADPGPREQAGGR_REQAGGRENAB),
		.disp_dpcafclkgate =			(ADPAFIDLECOUNT(0x30) | \
							 ADPAFDYNCLKGATEEN),
		.disp_dpcenab = 			(ADPCENAB_IDEAL_ARB_ENAB),
	},
#endif //DISPLAY_LANDSCAPE_IPAD_TUNABLES || DISPLAY_APPLE_TV_TUNABLES || DISPLAY_FPGA_TUNABLES
};

#endif	/* ! __APPLE_DISPLAY_H */
