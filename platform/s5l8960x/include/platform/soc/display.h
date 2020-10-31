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
#if DISPLAY_IPHONE_TUNABLES
	{
		.name = "default",
		.disp_dpcclkcntl[0] = 			(DPCCLKCNTL_FLOOR(0x20)	|
							 DPCCLKCNTL_PIPE_ENABLE |
							 DPCCLKCNTL_LB_ENABLE	|
							 DPCCLKCNTL_GATEENAB),
		.disp_dpbclklvl_clock_off_level[0] =	DPBCLKLVL_OFFLVL(0x7f8),
		.disp_dpbclklvl_clock_on_level[0] =	DPBCLKLVL_ONLVL(0x1e1),
		.disp_dpcqoscnfg[0] =			(DPCQOSCNFG_QOS_TIMER(0xFA0)	|
							 DPCQOSCNFG_PIPE_ENABLE		|
							 DPCQOSCNFG_VIDFIFO_ENABLE	|
							 DPCQOSCNFG_UIFIFO_ENABLE	|
							 DPCQOSCNFG_QOS_ENABLE),
		.disp_dpbqoslvl_med_watermark[0] =	DPBQOSLVL_MED_WATERMARK(0x1d7),
		.disp_dpbqoslvl_high_watermark[0] =	DPBQOSLVL_HIGH_WATERMARK(0x13a),
		.disp_dpureqcfg[0] =			DPUREQCFG_REQ_CNT(0x18),
		.disp_dpusrcstrd[0] =			DPUSRCSTRD_SRCBURST_4BLOCKS,
		.disp_dpvreqcfg[0] =			DPVREQCFG_REQ_CNT(0x6),
		.disp_dpvsrcstrd[0] =			DPVSRCSTRD_SRCBURST_2BLOCKS,
		.disp_dpcwbstrd[0] =			DPCWBSTRD_DSTBURST_4BLOCKS,
	
		.disp_dpcclkcntl[1] = 			(DPCCLKCNTL_FLOOR(0x20)	|
							 DPCCLKCNTL_LB_ENABLE	|
							 DPCCLKCNTL_GATEENAB),
		.disp_dpbclklvl_clock_off_level[1] =	DPBCLKLVL_OFFLVL(0x7f8),
		.disp_dpbclklvl_clock_on_level[1] =	DPBCLKLVL_ONLVL(0x1e1),
		.disp_dpcqoscnfg[1] =			(DPCQOSCNFG_QOS_TIMER(0xFA0)	|
							 DPCQOSCNFG_PIPE_ENABLE		|
							 DPCQOSCNFG_VIDFIFO_ENABLE	|
							 DPCQOSCNFG_UIFIFO_ENABLE	|
							 DPCQOSCNFG_QOS_ENABLE),
		.disp_dpbqoslvl_med_watermark[1] =	DPBQOSLVL_MED_WATERMARK(0x1d7),
		.disp_dpbqoslvl_high_watermark[1] =	DPBQOSLVL_HIGH_WATERMARK(0x13a),
		.disp_dpureqcfg[1] =			DPUREQCFG_REQ_CNT(0x18),
		.disp_dpusrcstrd[1] =			DPUSRCSTRD_SRCBURST_4BLOCKS,
		.disp_dpvreqcfg[1] =			DPVREQCFG_REQ_CNT(0x6),
		.disp_dpvsrcstrd[1] =			DPVSRCSTRD_SRCBURST_2BLOCKS,
		.disp_dpcwbstrd[1] =			DPCWBSTRD_DSTBURST_4BLOCKS,
	},
#endif	//DISPLAY_IPHONE_TUNABLES

//Tunables for targets using the the timing based on landscape ipads
#if DISPLAY_LANDSCAPE_IPAD_TUNABLES || DISPLAY_APPLE_TV_TUNABLES || DISPLAY_FPGA_TUNABLES
	{
		.name = "default",
		.disp_dpcclkcntl[0] = 			(DPCCLKCNTL_FLOOR(0x20)	|
							 DPCCLKCNTL_LB_ENABLE	|
							 DPCCLKCNTL_GATEENAB),
		.disp_dpbclklvl_clock_off_level[0] =	DPBCLKLVL_OFFLVL(0x7f8),
		.disp_dpbclklvl_clock_on_level[0] =	DPBCLKLVL_ONLVL(0x708),
		
		.disp_dpcqoscnfg[0] =			(DPCQOSCNFG_QOS_TIMER(0x5780)	|
							 DPCQOSCNFG_VIDFIFO_ENABLE	|
							 DPCQOSCNFG_UIFIFO_ENABLE	|
							 DPCQOSCNFG_QOS_ENABLE),
		.disp_dpbqoslvl_med_watermark[0] =	DPBQOSLVL_MED_WATERMARK(0x5dc),
		.disp_dpbqoslvl_high_watermark[0] =	DPBQOSLVL_HIGH_WATERMARK(0x3e8),
		.disp_dpureqcfg[0] =			DPUREQCFG_REQ_CNT(0x73),
		.disp_dpusrcstrd[0] =			DPUSRCSTRD_SRCBURST_4BLOCKS,
		.disp_dpvreqcfg[0] =			DPVREQCFG_REQ_CNT(0x39),
		.disp_dpvsrcstrd[0] =			DPVSRCSTRD_SRCBURST_2BLOCKS,
		.disp_dpcwbstrd[0] =			DPCWBSTRD_DSTBURST_4BLOCKS,	

		.disp_dpcclkcntl[1] = 			(DPCCLKCNTL_FLOOR(0x20)	|
							 DPCCLKCNTL_LB_ENABLE	|
							 DPCCLKCNTL_GATEENAB),
		.disp_dpbclklvl_clock_off_level[1] =	DPBCLKLVL_OFFLVL(0x7f8),
		.disp_dpbclklvl_clock_on_level[1] =	DPBCLKLVL_ONLVL(0x708),
		
		.disp_dpcqoscnfg[1] =			(DPCQOSCNFG_QOS_TIMER(0x5780)	|
							 DPCQOSCNFG_VIDFIFO_ENABLE	|
							 DPCQOSCNFG_UIFIFO_ENABLE	|
							 DPCQOSCNFG_QOS_ENABLE),
		.disp_dpbqoslvl_med_watermark[1] =	DPBQOSLVL_MED_WATERMARK(0x5dc),
		.disp_dpbqoslvl_high_watermark[1] =	DPBQOSLVL_HIGH_WATERMARK(0x3e8),
		.disp_dpureqcfg[1] =			DPUREQCFG_REQ_CNT(0x73),
		.disp_dpusrcstrd[1] =			DPUSRCSTRD_SRCBURST_4BLOCKS,
		.disp_dpvreqcfg[1] =			DPVREQCFG_REQ_CNT(0x39),
		.disp_dpvsrcstrd[1] =			DPVSRCSTRD_SRCBURST_2BLOCKS,
		.disp_dpcwbstrd[1] =			DPCWBSTRD_DSTBURST_4BLOCKS,
	},
	
#if DISPLAY_IPHONE_TUNABLES
#error DISPLAY_IPHONE_TUNABLES set for IPAD
#endif
	
#endif //DISPLAY_LANDSCAPE_IPAD_TUNABLES || DISPLAY_APPLE_TV_TUNABLES
};

#endif	/* ! __APPLE_DISPLAY_H */
