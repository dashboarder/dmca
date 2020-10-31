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
#include <drivers/display.h>
#include <lib/paint.h>
#include <platform/soc/hwclocks.h>
#include <drivers/dither.h>

#include "clcd_regs.h"

#ifndef CLCD_VERSION
#error CLCD_VERSION undefined
#endif

static void clcd_load_gamma_table(u_int32_t lut_id, u_int32_t *lut_data, uint32_t coeff);

void clcd_enable_clocks(bool enable)
{
	int device;

	if (CLCD_DISPLAYPIPE_BASE_ADDR == DISP0_BASE_ADDR)
		device = CLK_DISP0;
	else
		device = CLK_DISP1;

	// Enable/disable displaypipe and clcd clocks
	if (enable) {
		clock_gate(device, true);
        	clock_gate(CLK_CLCD, true);
	}
	else {
		clock_gate(CLK_CLCD, false);
		clock_gate(device, false);
	}
}

void clcd_init(struct display_timing *timing)
{
	rCLCD_STATUS = (1 << 8);		// Reset CLCD
	while (rCLCD_STATUS & (1 << 8));	// Wait for reset complete...
	spin(1);				// ... and wait some more: <rdar://problem/6090842>

	// Enable sync enable
	rCLCD_STATUS = (1 << 2);

	// Bypass and clock gate MIE
	rCLCD_CFG = (1 << 1) | (1 << 0);


#if DITHER_VERSION == 1
	rCLCD_DITH_CFG = (1 << 31) | (1 << 0);

	// Set dithering constants
	rCLCD_DITH_CONST = (2 << 16) | (4 << 8) | (8 << 0);

	if (timing->display_depth <= 18) {
#if CLCD_DITHER_BASE_ADDR
		rCLCD_DITHER_METHOD = (4 << 4) | (2 << 0);
		rCLCD_DITHER_ENABLE = 1;
#else
		rCLCD_DITH_CFG |= (1 << 24) | (1 << 20) | (1 << 16);
#endif
	}
#endif

#if DITHER_VERSION == 2
	//Need to force the bypass dithering in the clcd block
	rCLCD_DITH_CFG = (1 << 31) | (1 << 29) | (1 << 0);
	rCLCD_DITH_CONST = 0;

	dither_init(timing->display_depth);
	dither_set_enable(true);
#endif

	rCLCD_OTF_CON1 = 0;
	rCLCD_OTF_CON2 = (timing->neg_vclk << 3) | (timing->neg_hsync << 2) | (timing->neg_vsync << 1) | (timing->neg_vden << 0);

	rCLCD_OTF_TCON1 = (CLCD_OTF_TCON1_VSPP |
			   CLCD_OTF_TCON1_VBPD(timing->v_back_porch) |
			   CLCD_OTF_TCON1_VFPD(timing->v_front_porch) |
			   CLCD_OTF_TCON1_VSPW(timing->v_pulse_width));
#if defined(TARGET_CLCD_VSPP) && (TARGET_CLCD_VSPP == 0)
	rCLCD_OTF_TCON1 &= ~CLCD_OTF_TCON1_VSPP;
#endif
	rCLCD_OTF_TCON2 = (CLCD_OTF_TCON2_HBPD(timing->h_back_porch) |
			   CLCD_OTF_TCON2_HFPD(timing->h_front_porch) |
			   CLCD_OTF_TCON2_HSPW(timing->h_pulse_width));
	rCLCD_OTF_TCON3 = (CLCD_OTF_TCON3_HOZVAL(timing->h_active) |
			   CLCD_OTF_TCON3_LINEVAL(timing->v_active));
}

bool clcd_get_enable_timing_generator(void)
{
	return ((rCLCD_OTF_CON1 & 1) == 1);
}

void clcd_enable_timing_generator(bool enable)
{
	if (enable) {
		rCLCD_OTF_CON1 |= 1;					// Turn OTF on
	}
	else {
                rCLCD_OTF_CON1 &= ~1;					// Turn off OTF
                while ((rCLCD_OTF_CON1 & (1 << 1)) == 0);		// Wait for OTF off
	}
}

void clcd_install_gamma_table(u_int32_t *red_lut, u_int32_t *green_lut, u_int32_t *blue_lut, struct syscfg_wpcl *wpcl)
{
	RELEASE_ASSERT(red_lut != NULL);
	RELEASE_ASSERT(green_lut != NULL);
	RELEASE_ASSERT(blue_lut != NULL);
	RELEASE_ASSERT(wpcl != NULL);

	// Load the Gamma Correction Table
	clcd_load_gamma_table(CLCD_GM_COR_RED, red_lut, wpcl->red);
	clcd_load_gamma_table(CLCD_GM_COR_GREEN, green_lut, wpcl->green);
	clcd_load_gamma_table(CLCD_GM_COR_BLUE, blue_lut, wpcl->blue);

#if CLCD_VERSION >= 1
	// Swap to the new Gamma Correction Table
	rCLCD_GM_CON = CLCD_GM_CON_GM_CO_TBL_UPDATED;

	// Wait for the swap to complete
	while ((rCLCD_GM_CON & CLCD_GM_CON_GM_CO_TBL_UPDATED) != 0);

	// Load the Alternate Gamma Correction Table
	clcd_load_gamma_table(CLCD_GM_COR_RED, red_lut, wpcl->red);
	clcd_load_gamma_table(CLCD_GM_COR_GREEN, green_lut, wpcl->green);
	clcd_load_gamma_table(CLCD_GM_COR_BLUE, blue_lut, wpcl->blue);
#endif

	// Turn on Gamma Correction
	rCLCD_GM_CON = CLCD_GM_CON_GM_CO_ENABLE;
}

static void clcd_load_gamma_table(u_int32_t lut_id, u_int32_t *lut_data, uint32_t coeff)
{
	u_int32_t cnt;
	u_int32_t lut_size = (lut_id == CLCD_GM_ENC) ? 225 : 256;
	u_int32_t lut_value, lut_value2;

	rCLCD_GM_TBL_ACC_CON = (1 << 16) | (lut_id << 12) | (0 << 0);

	for (cnt = 0; cnt < lut_size; cnt++) {
		lut_value  = lut_data[cnt];

		switch (lut_id) {
			case CLCD_GM_DEC_RED :
			case CLCD_GM_DEC_GREEN :
			case CLCD_GM_DEC_BLUE :
				break;

			case CLCD_GM_COR_RED :
			case CLCD_GM_COR_GREEN :
			case CLCD_GM_COR_BLUE :
				lut_value2 = (lut_data[cnt + 1] * coeff) >> WpCl_Quotation_Denominator;
				lut_value  = (lut_value * coeff) >> WpCl_Quotation_Denominator;
				lut_value = ((lut_value << 10) & 0x000FFC00) | (lut_value2 & 0x000003FF);
				break;

			case CLCD_GM_ENC :
				break;
		}

		rCLCD_GM_TBL_WDATA = (1 << 31) | lut_value;
	}

	rCLCD_GM_TBL_ACC_CON = 0;
}
