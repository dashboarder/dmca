/*
 * Copyright (C) 2010-2012 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */
#include <debug.h>
#include <drivers/display.h>
#include <platform/soc/hwclocks.h>

#include "rgbout_regs.h"

static void rgbout_load_gamma_table();

void rgbout_enable_clocks(bool enable)
{
	int device;
	
#ifdef DISP0_BASE_ADDR
	if (RGBOUT_DISPLAYPIPE_BASE_ADDR == DISP0_BASE_ADDR) {
		device = CLK_DISP0;
	} else
#endif
#ifdef DISP1_BASE_ADDR
	if (RGBOUT_DISPLAYPIPE_BASE_ADDR == DISP1_BASE_ADDR) {
		device = CLK_DISP1;
	} else
#endif
		panic("rgb_enable_clocks: unsupported rgbout_displaypipe_base_addr: %p", (void *)RGBOUT_DISPLAYPIPE_BASE_ADDR);

	// Enable/disable displaypipe and clcd clocks
	if (enable) {
		clock_gate(device, true);
		clock_gate(CLK_RGBOUT, true);
	}
	else {
		clock_gate(CLK_RGBOUT, false);
		clock_gate(device, false);
	}
}

void rgbout_init(struct display_timing *timing)
{
	// reset RBOUT
	rRGBOUTSRESET = 1;
	while((rRGBOUTSRESET & 1) != 0);

	//configure RGBOUT
	rRGBOUTDITHCFG = 0;                             // disable dithering
	rRGBOUTOTFCON = 0;                              // no interlace, frame
	rRGBOUTOTFTCON6 = (timing->h_active - 1);       // display width
	rRGBOUTOTFTCON3 = (timing->v_active - 1);       // display height 
	rRGBOUTCFG = (1 << 5) | (0 << 3) | (3);         // color mode - RGB, 
							// color space conversion - off
							// display-module - DP
	rRGBOUTPOPLATENCY = (timing->v_pulse_width + timing->v_back_porch - 2); // <rdar://problem/10134376>
}

bool rgbout_get_enable_timing_generator(void)
{
	return ((rRGBOUTCTL & ((1 << 3) | (1 << 0))) == ((1 << 3) | (1 << 0)));
}

void rgbout_enable_timing_generator(bool enable)
{
	if (enable) {
                rRGBOUTCTL = (1 << 3) | (1 << 0);                       // set rgbout-on, frontend-ready
	}
	else {
                rRGBOUTSRESET = 1;                                      // reset
                while((rRGBOUTSRESET & 1) != 0);
	}
}

void rgbout_install_gamma_table(u_int32_t *red_lut, u_int32_t *green_lut, u_int32_t *blue_lut)
{
	const u_int32_t gamma_data[256] = {
		0x10044,0x11048,0x1204c,0x13050,0x14050,0x14054,0x15058,0x1605c,0x17060,0x18064,0x19068,0x1a06c,0x1b06c,0x1b070,0x1c074,0x1d078,0x1e07c,0x1f080,0x20084,0x21088,0x22088,0x2208c,0x23090,0x24094,0x25098,0x2609c,0x270a0,0x280a4,0x290a4,0x290a8,0x2a0ac,0x2b0b0,0x2c0b4,0x2d0b8,0x2e0bc,0x2f0c0,0x300c4,0x310c4,0x310c8,0x320cc,0x330d0,0x340d4,0x350d8,0x360dc,0x370e0,0x380e0,0x380e4,0x390e8,0x3a0ec,0x3b0f0,0x3c0f4,0x3d0f8,0x3e0fc,0x3f0fc,0x3f100,0x40104,0x41108,0x4210c,0x43110,0x44114,0x45118,0x46118,0x4611c,0x47120,0x48124,0x49128,0x4a12c,0x4b130,0x4c134,0x4d134,0x4d138,0x4e13c,0x4f140,0x50144,0x51148,0x5214c,0x53150,0x54154,0x55154,0x55158,0x5615c,0x57160,0x58164,0x59168,0x5a16c,0x5b170,0x5c170,0x5c174,0x5d178,0x5e17c,0x5f180,0x60184,0x61188,0x6218c,0x6318c,0x63190,0x64194,0x65198,0x6619c,0x671a0,0x681a4,0x691a8,0x6a1a8,0x6a1ac,0x6b1b0,0x6c1b4,0x6d1b8,0x6e1bc,0x6f1c0,0x701c4,0x711c8,0x721c8,0x721cc,0x731d0,0x741d4,0x751d8,0x761dc,0x771e0,0x781e4,0x791e4,0x791e8,0x7a1ec,0x7b1f0,0x7c1f4,0x7d1f8,0x7e1fc,0x7f200,0x80200,0x80204,0x81208,0x8220c,0x83210,0x84214,0x85218,0x8621c,0x8721c,0x87220,0x88224,0x89228,0x8a22c,0x8b230,0x8c234,0x8d238,0x8e238,0x8e23c,0x8f240,0x90244,0x91248,0x9224c,0x93250,0x94254,0x95258,0x96258,0x9625c,0x97260,0x98264,0x99268,0x9a26c,0x9b270,0x9c274,0x9d274,0x9d278,0x9e27c,0x9f280,0xa0284,0xa1288,0xa228c,0xa3290,0xa4290,0xa4294,0xa5298,0xa629c,0xa72a0,0xa82a4,0xa92a8,0xaa2ac,0xab2ac,0xab2b0,0xac2b4,0xad2b8,0xae2bc,0xaf2c0,0xb02c4,0xb12c8,0xb22cc,0xb32cc,0xb32d0,0xb42d4,0xb52d8,0xb62dc,0xb72e0,0xb82e4,0xb92e8,0xba2e8,0xba2ec,0xbb2f0,0xbc2f4,0xbd2f8,0xbe2fc,0xbf300,0xc0304,0xc1304,0xc1308,0xc230c,0xc3310,0xc4314,0xc5318,0xc631c,0xc7320,0xc8320,0xc8324,0xc9328,0xca32c,0xcb330,0xcc334,0xcd338,0xce33c,0xcf33c,0xcf340,0xd0344,0xd1348,0xd234c,0xd3350,0xd4354,0xd5358,0xd635c,0xd735c,0xd7360,0xd8364,0xd9368,0xda36c,0xdb370,0xdc374,0xdd378,0xde378,0xde37c,0xdf380,0xe0384,0xe1388,0xe238c,0xe3390,0xe4394,0xe5394,0xe5398,0xe639c,0xe73a0,0xe83a4,0xe93a8,0xea3ac,0xeb3b0,0xec3b0,0xec3b4,0xed3b8,0xee3bc,0xef3c0,0xef3c0
	};
	int i;
	u_int32_t reg_val;

	// set write access for gamma table
	reg_val = rRGBOUTGMTBLACCCON;
	rRGBOUTGMTBLACCCON = reg_val & ~((1<<16) | (0xf<<12) | (0xff));

	for (i = 0; i < 256; i++) {
		rRGBOUTGMTBLACCCON = reg_val | ((1 << 16) | (0x4 << 12) | (i)); // 0100 gamma for R
		rRGBOUTGMTBLWDATA = (0 << 31) | (gamma_data[i] & 0x000FFFFF);
	}

	for (i = 0; i < 256; i++) {
		rRGBOUTGMTBLACCCON = reg_val | ((1 << 16) | (0x5 << 12) | (i)); // 0101 gamma for G
		rRGBOUTGMTBLWDATA = (0 << 31) | (gamma_data[i] & 0x000FFFFF);
	}

	for (i = 0; i < 256; i++) {
		rRGBOUTGMTBLACCCON = reg_val | ((1 << 16) | (0x6 << 12) | (i)); // 0110 gamma for B
		rRGBOUTGMTBLWDATA = (0 << 31) | (gamma_data[i] & 0x000FFFFF);
	}

	rRGBOUTGMCON = 0x9;                             // enable gamma correction
}
