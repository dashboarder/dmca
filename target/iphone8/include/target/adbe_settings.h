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

static struct adbe_v2_tuneable adbe_tuneables[] = 
{
	{
		.name = "D620",
		.adbe0_vblank_pos_vbi_pulse	= 0x30b,
		.adbe0_vblank_clk_gate_wakeup 	= 0x0007,
		.adbe0_vblank_clk_gate_idle 	= 0x0000,
		.adbe0_vblank_busy_finish	= 0x2,
		.adbe0_tunable_mode_ctrl	= (DBEMODECNTL_DPB_BUSY_MASK | \
						   DBEMODECNTL_PMGR_CLK_GATE_ENABLE | \
						   DBEMODECNTL_DYN_CLK_GATE_ENABLE | \
						   DBEMODECNTL_BLK_CLK_GATE_ENABLE),
		.adbe0_aap_support		= 1,
		.adbe0_aap_format_control_reg1  = (AAP_FORMAT_CONTROL_REG1_AUTOPOS | AAP_FORMAT_CONTROL_REG1_RSVD(9) | AAP_FORMAT_CONTROL_REG1_VS_POL),
	},
	
	{
		.name = "D520",
		.adbe0_vblank_pos_vbi_pulse	= 0x218,
		.adbe0_vblank_clk_gate_wakeup 	= 0x0007,
		.adbe0_vblank_clk_gate_idle 	= 0x0000,
		.adbe0_vblank_busy_finish	= 0x2,
		.adbe0_tunable_mode_ctrl	= (DBEMODECNTL_DPB_BUSY_MASK | \
						   DBEMODECNTL_PMGR_CLK_GATE_ENABLE | \
						   DBEMODECNTL_DYN_CLK_GATE_ENABLE | \
						   DBEMODECNTL_BLK_CLK_GATE_ENABLE),
		.adbe0_aap_support		= 1,
		.adbe0_aap_format_control_reg1  = (AAP_FORMAT_CONTROL_REG1_AUTOPOS | AAP_FORMAT_CONTROL_REG1_RSVD(9) | AAP_FORMAT_CONTROL_REG1_VS_POL),
	},
};
					
#endif