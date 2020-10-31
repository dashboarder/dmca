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

static struct adfe_v2_tuneable adfe_tuneables[] = 
{
	{ 
		.name = 				"default",
		.disp_dpcclkcntl = 			(ADPCCLKCNTL_FLOOR(0x20) | \
							 ADPCCLKCNTL_PIPE_ENABLE | \
							 ADPCCLKCNTL_LB_ENABLE	| \
							 ADPCCLKCNTL_PIO_GATE_ENABLE | \
							 ADPCCLKCNTL_STAT_GATE_ENABLE | \
							 ADPCCLKCNTL_EXT_GATE_ENABLE | \
							 ADPCCLKCNTL_DYN_GATE_ENABLE),
		.disp_dpcclklvl_clock_on_level = 	ADPCCLKLVL_ONLVL(0x130),
		.disp_dpcclklvl_clock_off_level = 	ADPCCLKLVL_OFFLVL(0x1f8),
		.disp_dpcqoscfg = 			(ADPCQOSCFG_QOS_TIMER(0x7d0)	| \
							 ADPCQOSCFG_PIPE_ENABLE | \
							 ADPCQOSCFG_QOS_ENABLE),
		.disp_dpcqosylvl_yellow_off = 		ADPCQOSYLVL_YELLOW_OFF(0x130),
		.disp_dpcqosylvl_yellow_on = 		ADPCQOSYLVL_YELLOW_ON(0x128),
		.disp_dpcqosrlvl_red_off = 		ADPCQOSRLVL_RED_OFF(0x118),
		.disp_dpcqosrlvl_red_on = 		ADPCQOSRLVL_RED_ON(0x114),
		.disp_dpclinkdown = 			(ADPCLINKDOWN_TEARDOWNNOSCL | \
							 ADPCLINKDOWN_TEARDOWNEN),
		.disp_dpgpreqaggr = 			(ADPGPREQAGGR_REQAGGRTHRESH(0x24) | \
							 ADPGPREQAGGR_CACHEHINT(0x3) | \
							 ADPGPREQAGGR_REQAGGRENAB),
		.disp_dpcenab = 			ADPCENAB_IDEAL_ARB_ENAB,
	},
};
				
#endif	/* ! __APPLE_DISPLAY_H */
