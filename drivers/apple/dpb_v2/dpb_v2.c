/*
 * Copyright (C) 2015 Apple Inc. All rights reserved.
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
#include "dpb_v2.h"

void dpb_init(uint32_t display_width, uint32_t display_height)
{
	uint32_t control;
	
	// <rdar://problem/19520310> ELBA: ADBE::DPB: Missing enable in DPB (WA to disable DPB)
	control = rDPB_CONTROL_REG;
	
	control |= DPB_CTRL_TMF_BYPASS;
	control &= ~DPB_CTRL_DPB_DYNAMIC_TMF;
	control |= DPB_CTRL_TEMPORTAL_FILTER_BYPASS;
	control &= ~DPB_CTRL_DYNAMIC_BACKLIGHT;
	control &= ~DPB_CTRL_BCL_ENABLE;
	
	rDPB_CONTROL_REG = control;
	
	rDPB_ACTIVE_REGION_SIZE_REG = 0x00010001;
	
	// Tell the DPB double buffered registers to update immediatly.
	rDPB_UPDATE_CONTROL_REG |= (1 << 2);
}


void dpb_install_gamma_table(u_int32_t *red_lut, u_int32_t *green_lut, u_int32_t *blue_lut, struct syscfg_wpcl *wpcl)
{
	// Do Nothing.. DPB is bypassed
}