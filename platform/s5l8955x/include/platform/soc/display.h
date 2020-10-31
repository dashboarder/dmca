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

// Platform-specific displaypipe tunables
static struct adfe_v1_tuneable adfe_tuneables[] = 
{
	{
		.name = "default",
		.disp_dpcclkcntl[0] = 			0,
		.disp_dpbclklvl_clock_off_level[0] =	DPBCLKLVL_OFFLVL(0x09F0),
		.disp_dpbclklvl_clock_on_level[0] =	DPBCLKLVL_ONLVL(0x0780),
		.disp_dpusrcstrd[0] =			DPUSRCSTRD_SRCBURST_8BLOCKS,
		.disp_dpcqoscnfg[0] =			(DPCQOSCNFG_QOS_TIMER(0x0040)	| \
							 DPCQOSCNFG_UIFIFO_ENABLE	| \
							 DPCQOSCNFG_VIDFIFO_ENABLE	| \
							 DPCQOSCNFG_QOS_ENABLE),
		.disp_dpbqoslvl_med_watermark[0] =	DPBQOSLVL_MED_WATERMARK(0x0500),
		.disp_dpbqoslvl_high_watermark[0] =	DPBQOSLVL_HIGH_WATERMARK(0x0280),
		.disp_dpureqcfg[0] =			0,

		.disp_dpcclkcntl[1] = 			0,
		.disp_dpbclklvl_clock_off_level[1] =	DPBCLKLVL_OFFLVL(0x09F0),
		.disp_dpbclklvl_clock_on_level[1] =	DPBCLKLVL_ONLVL(0x0780),
		.disp_dpusrcstrd[1] =			DPUSRCSTRD_SRCBURST_4BLOCKS,
		.disp_dpcqoscnfg[1] =			(DPCQOSCNFG_QOS_TIMER(0x2000)	| \
							 DPCQOSCNFG_UIFIFO_ENABLE	| \
							 DPCQOSCNFG_VIDFIFO_ENABLE	| \
							 DPCQOSCNFG_QOS_ENABLE),
		.disp_dpbqoslvl_med_watermark[1] =	DPBQOSLVL_MED_WATERMARK(0x0500),
		.disp_dpbqoslvl_high_watermark[1] =	DPBQOSLVL_HIGH_WATERMARK(0x0280),
		.disp_dpureqcfg[1] =			0,
	},
};	

#endif	/* ! __APPLE_DISPLAY_H */
