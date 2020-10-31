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

static struct adbe_v1_tuneable adbe_tuneables[] = {
	{
		.name = "default",
		.adbe0_vblank_pos_vbi_pulse	= 0x0000,
		.adbe0_vblank_clk_gate_wakeup 	= 0x0000,
		.adbe0_vblank_clk_gate_idle 	= 0x0001,
		.adbe0_vblank_blk_idle_dpb	= 0x0000,
		.adbe0_vblank_blk_idle_aap	= 0x0000,
		
		/*
		 * disable all clock gating until SEG resolves:
		 * <rdar://problem/13305313> J34: adbe dynamic clock gating causes no video on dp_tx
		 */
		.adbe0_tunable_mode_ctrl	= 0,
		.adbe0_aap_support		= 1,
	},
};

#endif
