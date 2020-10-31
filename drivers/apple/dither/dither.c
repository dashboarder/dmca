/*
 * Copyright (C) 2009-2013 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */
#include <debug.h>
#include <lib/env.h>
#include <platform/soc/hwregbase.h>
#include <drivers/dither.h>

#if DITHER_VERSION == 2
#include "dither_v2.h"
#elif DITHER_VERSION == 3
#include "dither_v3.h"
#else
#include "dither_v4.h"
#endif

#ifndef DITHER_BASE_ADDR
#error "DITHER base addr for AppleDither Block is not defined"
#endif

static uint8_t dither_type;

#if DITHER_VERSION == 4
#ifndef TARGET_DITHER_INBITSREDUCTION
#define	TARGET_DITHER_INBITSREDUCTION	2
#endif

#ifndef TARGET_DITHER_EDOUTTOSTINWIDTH
#define	TARGET_DITHER_EDOUTTOSTINWIDTH	3
#endif

#ifndef TARGET_DITHER_DITHEROUTWIDTH
#define	TARGET_DITHER_DITHEROUTWIDTH	3
#endif

#ifndef TARGET_DITHER_ST_METHOD_MAX
#define	TARGET_DITHER_ST_METHOD_MAX	3
#endif

#ifndef TARGET_DITHER_ST_METHOD_POP
#define	TARGET_DITHER_ST_METHOD_POP	0
#endif

#ifndef TARGET_DITHER_ST_PHASE
#define	TARGET_DITHER_ST_PHASE		0
#endif

#ifndef TARGET_DITHER_ST_SKEWLOC0
#define	TARGET_DITHER_ST_SKEWLOC0	0
#endif

#ifndef TARGET_DITHER_ST_SKEWLOC1
#define	TARGET_DITHER_ST_SKEWLOC1	0
#endif

#ifndef TARGET_DITHER_ST_SKEWLOC2
#define	TARGET_DITHER_ST_SKEWLOC2	0
#endif

#ifndef TARGET_DITHER_ST_SKEWLOC3
#define	TARGET_DITHER_ST_SKEWLOC3	0
#endif

#ifndef TARGET_DITHER_ST_SKEWLOC4
#define	TARGET_DITHER_ST_SKEWLOC4	0
#endif

#ifndef TARGET_DITHER_ST_SKEWLOC5
#define	TARGET_DITHER_ST_SKEWLOC5	0
#endif

#ifndef TARGET_DITHER_ST_SKEWLOC6
#define	TARGET_DITHER_ST_SKEWLOC6	0
#endif

#ifndef TARGET_DITHER_ST_SKEWLOC7
#define	TARGET_DITHER_ST_SKEWLOC7	0
#endif

#ifndef TARGET_DITHER_ST_SKEWLOC8
#define	TARGET_DITHER_ST_SKEWLOC8	0
#endif

#ifndef TARGET_DITHER_ST_SKEWLOC9
#define	TARGET_DITHER_ST_SKEWLOC9	0
#endif

#ifndef TARGET_DITHER_ST_SKEWLOC10
#define	TARGET_DITHER_ST_SKEWLOC10	0
#endif

#ifndef TARGET_DITHER_ST_SKEWLOC11
#define	TARGET_DITHER_ST_SKEWLOC11	0
#endif

#ifndef TARGET_DITHER_ST_SKEWLOC12
#define	TARGET_DITHER_ST_SKEWLOC12	0
#endif

#ifndef TARGET_DITHER_ST_SKEWLOC13
#define	TARGET_DITHER_ST_SKEWLOC13	0
#endif

#ifndef TARGET_DITHER_ST_SKEWLOC14
#define	TARGET_DITHER_ST_SKEWLOC14	0
#endif

void dither_init(uint32_t display_width, uint32_t display_height, uint32_t display_depth)
#else
void dither_init(uint32_t display_depth)
#endif
{
	//Fixing the value for now.  When the request comes to use a Blue noise, please ask how to make
	//the decision: Based on Target or based on number of dithering bits 
	dither_type = DITHER_SPATIO_TEMPORAL;

#ifdef TARGET_DITHER_TYPE
	dither_type = TARGET_DITHER_TYPE;
#endif //TARGET_DITHER_TYPE

#if DITHER_VERSION == 2
	//Number of bits reduced by the dithering algorithm core.
	rDITHER_METHOD = (4 << 4) | ((display_depth <= 18) ? 0x3 : 0x2); 
	rDITHER_BOOST_POP = (display_depth <= 18) ? 0x1 : 0x0;
	rDITHER_OUTPUT_WIDTH = (display_depth <=18) ? 0x0 : 0x1;
#endif

#if DITHER_VERSION == 3
	switch(dither_type) {
	case DITHER_SPATIO_TEMPORAL:
		dprintf(DEBUG_SPEW, "Spatial_temporal Dither\n");
		//Bypass BN
		//<rdar://problem/11389221> [FPGA DEBUG]: Disabling certains backend blocks via ADBE0:MODE_CTL doesn't disable them
		rDITHER_BN_OP_CFG = (1 << 29);

		rDITHER_ST_METHOD = (0x4 << 4);
		if (display_depth == 24)
			rDITHER_ST_METHOD |= (0x1 << 13) | (0x2 << 0);
		else if (display_depth <= 18)
			rDITHER_ST_METHOD |= (0x1 << 13) | (0x3 << 0);
		else
			panic("unexpected display depth");

		break;

	case DITHER_BLUE_NOISE:
		// Note: If bn_dither_enable is set and st_dither_enable is cleared, the
		// spatio/temporal dither stage must also be disabled internally
		// (see DITHER Register Spec.) by setting SPAT_TEMP_METHOD.mode to 0x4 and
		// SPAT_TEMP_METHOD.depth to 0x0
		dprintf(DEBUG_SPEW, "Blue_noise  Dither\n");
		rDITHER_ST_METHOD |= (0x4 << 13);
		rDITHER_ST_METHOD &= ~(0xf);	//clear depth
		rDITHER_BN_OP_CFG = 1;
		uint32_t thr_bit;
		switch(display_depth) {
		case 24:
			thr_bit = DITHER_BN_CONST_THR2BIT;
			break;
		case 18:
			thr_bit = DITHER_BN_CONST_THR4BIT;
			rDITHER_BN_OP_CFG |= (0x1 << 24) | (0x1 << 20) | (0x1 << 16);
			break;
		default:
			panic("unexpected display depth");
		}
		rDITHER_BN_CONST =  (thr_bit << 16 | thr_bit << 8 | thr_bit << 0);
		break;

	case DITHER_NONE:
	default:
		dprintf(DEBUG_SPEW, "%d is not a supported dithering type. Disabling dithering\n", dither_type);
		//bypassed and clock-gated if both st_dither_enable and bn_dither_enable are
		//cleared
		rDITHER_ST_METHOD = 0;
		rDITHER_BN_OP_CFG = (1 << 29);
		break;
	}
#endif

#if DITHER_VERSION == 4
	rDITHER_ACTIVEREGIONSTART = 0 << 16 | 0; //Set active region start
	rDITHER_ACTIVEREGIONSIZE = display_height << 16 | display_width; //Set Active Region

	// Setup for 8-bit panel with VRR
	rDITHER_INBITSREDUCTION = TARGET_DITHER_INBITSREDUCTION;
	rDITHER_EDOUTTOSTINWIDTH = TARGET_DITHER_EDOUTTOSTINWIDTH;
	rDITHER_DITHEROUTWIDTH = TARGET_DITHER_DITHEROUTWIDTH;
	
	switch(dither_type) {
	case DITHER_SPATIO_TEMPORAL:
		dprintf(DEBUG_SPEW, "Spatial_temporal Dither\n");

		rDITHER_ST_METHOD |= (TARGET_DITHER_ST_METHOD_MAX << 13) | (TARGET_DITHER_ST_METHOD_POP << 8);
		if (display_depth == 24)
			rDITHER_ST_METHOD |= (0x2 << 0);
		else if (display_depth <= 18)
			rDITHER_ST_METHOD |= (0x3 << 0);
		else
			panic("unexpected display depth");
	
		rDITHER_ST_PHASE = TARGET_DITHER_ST_PHASE;
		rDITHER_ST_SKEWLOC0 = TARGET_DITHER_ST_SKEWLOC0;
		rDITHER_ST_SKEWLOC1 = TARGET_DITHER_ST_SKEWLOC1;
		rDITHER_ST_SKEWLOC2 = TARGET_DITHER_ST_SKEWLOC2;
		rDITHER_ST_SKEWLOC3 = TARGET_DITHER_ST_SKEWLOC3;
		rDITHER_ST_SKEWLOC4 = TARGET_DITHER_ST_SKEWLOC4;
		rDITHER_ST_SKEWLOC5 = TARGET_DITHER_ST_SKEWLOC5;
		rDITHER_ST_SKEWLOC6 = TARGET_DITHER_ST_SKEWLOC6;
		rDITHER_ST_SKEWLOC7 = TARGET_DITHER_ST_SKEWLOC7;
		rDITHER_ST_SKEWLOC8 = TARGET_DITHER_ST_SKEWLOC8;
		rDITHER_ST_SKEWLOC9 = TARGET_DITHER_ST_SKEWLOC9;
		rDITHER_ST_SKEWLOC10 = TARGET_DITHER_ST_SKEWLOC10;
		rDITHER_ST_SKEWLOC11 = TARGET_DITHER_ST_SKEWLOC11;
		rDITHER_ST_SKEWLOC12 = TARGET_DITHER_ST_SKEWLOC12;
		rDITHER_ST_SKEWLOC13 = TARGET_DITHER_ST_SKEWLOC13;
		rDITHER_ST_SKEWLOC14 = TARGET_DITHER_ST_SKEWLOC14;

		rDITHER_ENABLE |= (1 << 1);
		break;

	case DITHER_ERROR_DIFFUSION:
		dprintf(DEBUG_SPEW, "Error Diffusion Dither\n");
		
		rDITHER_ED_THRESHOLDRED = (8192 << 2);
		rDITHER_ED_THRESHOLDGREEN = (8192 << 2);
		rDITHER_ED_THRESHOLDBLUE = (8192 << 2);

		rDITHER_ENABLE |= (1 << 0);
		break;
		
	case DITHER_NONE:
	default:
		dprintf(DEBUG_SPEW, "%d is not a supported dithering type. Disabling dithering\n", dither_type);
		//bypassed and clock-gated if both st_dither_enable and bn_dither_enable are
		//cleared
		rDITHER_ST_METHOD = 0;
		break;
	
	}
	//double buffer registers. force them to update
	rDITHER_UPDATECONTROL = DITHER_UPDATECONTROL_UPDATEENABLETIMING |
	     DITHER_UPDATECONTROL_UPDATEREQTIMING ;
#endif
}


void dither_set_enable(bool enable)
{
#if DITHER_VERSION == 2
	if (enable) {
		rDITHER_ENABLE = (dither_type == DITHER_SPATIO_TEMPORAL)? (1 << 0) : (1 << 1);
	} else {
		rDITHER_ENABLE = 0;
	}
#endif
}
