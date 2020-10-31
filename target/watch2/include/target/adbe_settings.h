/*
 * Copyright (C) 2014 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Computer, Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */
#ifndef __TARGET_ADBE_SETTINGS_H
#define __TARGET_ADBE_SETTINGS_H

static struct adbe_v3_tuneable adbe_tuneables[] = {
	{
		.name = "default",
		.adbe0_vblank_clk_gate_wakeup 	= 0x0000,
		.adbe0_vblank_clk_gate_idle 	= 0x0001,
		.adbe0_tunable_nrt_ctl		= (ADBE_NRT_CTL_ADISP_CLK_GATE_ENABLE | ADBE_NRT_CTL_ADBE_CLK_GATE_ENABLE | ADBE_NRT_CTL_BLK_CLK_GATE_ENABLE),
		.adbe0_aap_support		= 0,
	},
};

#endif
