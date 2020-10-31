/*
 * Copyright (C) 2014 Apple Inc. All rights reserved.
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
#include <platform/gpiodef.h>
#include <lib/syscfg.h>
#include <platform.h>
#include <platform/soc/chipid.h>
#include <platform/soc/hwclocks.h>
#include <platform/soc/hwregbase.h>
#include <drivers/dither.h>
#include <drivers/dpb.h>
#include <drivers/wpc.h>
#include <drivers/prc.h>
#include <drivers/adbe/adbe.h>

void adbe_enable_clocks(bool enable)
{
	int device;

	if (ADBE_DISPLAYPIPE_BASE_ADDR == DISP0_BASE_ADDR)
		device = CLK_DISP0;
#ifdef DISP1_BASE_ADDR
	else
		device = CLK_DISP1;
#endif //DISP1_BASE_ADDR

	if (enable)
	        clock_gate(device, true);
	else
	        clock_gate(device, false);
}

void adbe_install_gamma_table(u_int32_t *red_lut, u_int32_t *green_lut, u_int32_t *blue_lut, struct syscfg_wpcl *wpcl)
{
	RELEASE_ASSERT(red_lut != NULL);
	RELEASE_ASSERT(green_lut != NULL);
	RELEASE_ASSERT(blue_lut != NULL);
	RELEASE_ASSERT(wpcl != NULL);
	
#ifdef WITH_DPB
	//gamma correction is done in the dpb block
	dpb_install_gamma_table(red_lut, green_lut, blue_lut, wpcl);
#endif

#ifdef WITH_WPC
	//gamma correction is done in the wpc block
	wpc_install_gamma_table(red_lut, green_lut, blue_lut, wpcl);
#endif

#ifdef WITH_PRC
	//gamma correction is done in the prc block
	prc_install_gamma_table(red_lut, green_lut, blue_lut, wpcl);
#endif
}
