/*
 * Copyright (C) 2012 Apple Inc. All rights reserved.
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
#include <lib/paint.h>
#include <lib/syscfg.h>
#include <drivers/dpb.h>
#include "dpb.h"

static void dpb_load_gamma_table(u_int32_t lut_id, u_int32_t *lut_data);
static void dpb_convert_gamma_tables(u_int32_t *red_lut, u_int32_t *green_lut, u_int32_t *blue_lut, u_int32_t *mod_red_lut, u_int32_t *mod_green_lut, u_int32_t *mod_blue_lut, struct syscfg_wpcl *wpcl);

#define DPB_LUT_SIZE_PER_CHANNEL	129

void dpb_init(void)
{
	//do not bypass any of the blocks
	//<rdar://problem/13436089> Leave LUT bypass config in default state in DPB::CONTROL_REG

#ifdef DPB_DETECT_VIB_CONTROL_TUNABLE
	rDPB_DETECT_VBI_CONTROL_REG = DPB_DETECT_VIB_CONTROL_TUNABLE;
#endif
}

bool dpb_get_enable(void)
{
	return false;
}

void dpb_set_enable(bool enable)
{
	if (enable) {
		rDPB_CONTROL_REG |=  DPB_CTRL_ENABLE_CFG;
	} else {
		rDPB_CONTROL_REG &=  ~DPB_CTRL_ENABLE_CFG;
	}
}

void dpb_install_gamma_table(u_int32_t *red_lut, u_int32_t *green_lut, u_int32_t *blue_lut, struct syscfg_wpcl *wpcl)
{
	u_int32_t mod_red_lut[DPB_LUT_SIZE_PER_CHANNEL], mod_green_lut[DPB_LUT_SIZE_PER_CHANNEL], mod_blue_lut[DPB_LUT_SIZE_PER_CHANNEL];

	dpb_convert_gamma_tables(red_lut, green_lut, blue_lut, mod_red_lut, mod_green_lut, mod_blue_lut, wpcl);

	// Load the Gamma Correction Table
	dpb_load_gamma_table(DPB_RED, mod_red_lut);
	dpb_load_gamma_table(DPB_GREEN, mod_green_lut);
	dpb_load_gamma_table(DPB_BLUE, mod_blue_lut);

	// Turn on Gamma Correction
	rDPB_CONTROL_REG |= DPB_CTRL_CGM_ENABLE_CFG;

	// Load the Gamma Correction Table
	dpb_load_gamma_table(DPB_RED, mod_red_lut);
	dpb_load_gamma_table(DPB_GREEN, mod_green_lut);
	dpb_load_gamma_table(DPB_BLUE, mod_blue_lut);

	rDPB_TABLE_CONTROL_REG |= DPB_UPDATE_REGISTERS | DPB_IMMEDIATE_UPDATE | DPB_UPDATE_CGM_GREEN_TABLE | DPB_UPDATE_CGM_RED_TABLE | DPB_UPDATE_CGM_BLUE_TABLE; 

	//Wait till HW is cleared
	while ((rDPB_TABLE_CONTROL_REG & DPB_UPDATE_CGM_GREEN_TABLE) && (rDPB_TABLE_CONTROL_REG & DPB_UPDATE_CGM_RED_TABLE) && (rDPB_TABLE_CONTROL_REG & DPB_UPDATE_CGM_BLUE_TABLE));

	// Load the Gamma Correction Table again since the OS reads them (double buffered registers)
	dpb_load_gamma_table(DPB_RED, mod_red_lut);
	dpb_load_gamma_table(DPB_GREEN, mod_green_lut);
	dpb_load_gamma_table(DPB_BLUE, mod_blue_lut);
}

static void dpb_load_gamma_table(u_int32_t lut_id, u_int32_t *lut_data)
{
	u_int32_t cnt;
	u_int32_t lut_size = DPB_LUT_SIZE_PER_CHANNEL; 
	volatile u_int32_t * lut_addr;

	switch(lut_id) {
	case DPB_RED:
		lut_addr = &DPB_CGM_RED;
		break;
	case DPB_GREEN:
		lut_addr = &DPB_CGM_GREEN;
		break;
	case DPB_BLUE:
		lut_addr = &DPB_CGM_BLUE;
		break;
	default:
		panic("invalid gamma lut id");
	}

	for (cnt = 0; cnt < lut_size; cnt++, lut_addr++) {
		*lut_addr = lut_data[cnt];
	}
}

//Gamma table is passed in the old clcd format, which is not compatible "as is" with dpb HW format.
// The table provided has the following charasteristics:
// 1. its values are compressed
// 2. the layout is linear layout 1..n
// For H6 , we need the following table charasterictics:
// 1. decompress
// 2. layout is bankbased entry = (2N+1) << 16 | 2N
static void dpb_convert_gamma_tables(u_int32_t *red_lut, u_int32_t *green_lut, u_int32_t *blue_lut, u_int32_t *mod_red_lut, u_int32_t *mod_green_lut, u_int32_t *mod_blue_lut, struct syscfg_wpcl *wpcl)
{
	int i, j;
	u_int32_t oddValue, evenValue;
	u_int32_t reg;
	const u_int32_t *orig_luts[3];
	u_int32_t *mod_luts[3];
	uint32_t coeffs[3];

	orig_luts[0] = red_lut;
	orig_luts[1] = green_lut;
	orig_luts[2] = blue_lut;
	mod_luts[0] = mod_red_lut;
	mod_luts[1] = mod_green_lut;
	mod_luts[2] = mod_blue_lut;
	coeffs[0] = wpcl->red;
	coeffs[1] = wpcl->green;
	coeffs[2] = wpcl->blue;

	// For H6, there are 129 10-bit entries on 2 banks, totalling 258:
	// RESERVED1                BANK 1                        RESERVED0                        BANK 0
	// [31:26]                  [25:16]                        [15:10]                          [9:0]
	// X                           1                              X                               0
	// X                           3                              X                               2
	// ...
	// X                          257                             X                               256
	for (i = 0; i < DPB_LUT_SIZE_PER_CHANNEL; i++)
	{
		for (j = 0; j < 3; j++)
		{
			//extract 2n+1
			oddValue = (2*i+1 >= 257) ? 0 : orig_luts[j][2*i+1];
			//decompress
			oddValue = ((oddValue * coeffs[j]) >> WpCl_Quotation_Denominator) & 0x03FF;

			//extract 2n
			evenValue = orig_luts[j][2*i];
			//decompress
			evenValue = ((evenValue * coeffs[j]) >> WpCl_Quotation_Denominator) & 0x03FF;

			reg = (oddValue << 16) | evenValue;
			mod_luts[j][i] = reg;
		}
	}
}
