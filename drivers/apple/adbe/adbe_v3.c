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
#include "adbe_regs_v3.h"

#include <debug.h>
#include <drivers/display.h>
#include <lib/paint.h>
#include <platform/gpiodef.h>
#include <lib/syscfg.h>
#include <platform.h>
#include <platform/soc/chipid.h>
#include <platform/soc/hwclocks.h>
#include <platform/soc/hwregbase.h>
#include <drivers/dither.h>
#include <drivers/dpb.h>
#include <drivers/adbe/adbe.h>

struct adbe_v3_tuneable {
	char *name;
	uint32_t adbe0_vblank_clk_gate_wakeup;
	uint32_t adbe0_vblank_clk_gate_idle;
	uint32_t adbe0_tunable_nrt_ctl;
	uint32_t adbe0_aap_support;
};

#include <target/adbe_settings.h>

static bool is_video_mode = true;

static struct adbe_v3_tuneable *adbe_tuneables_info;
static const int32_t adbe_tuneables_list_size = sizeof(adbe_tuneables) / sizeof(struct adbe_v3_tuneable);

void adbe_init(struct display_timing *timing)
{
	const char		*env;
	int32_t			cnt;
	
	env = env_get("adbe-tunable");
	if (env == 0) env = "default";

	for (cnt = 0; cnt < adbe_tuneables_list_size; cnt++) {
		if (strcmp(env, adbe_tuneables[cnt].name)) continue;

		adbe_tuneables_info = adbe_tuneables + cnt;
	}
	if (adbe_tuneables_info == 0) {
		dprintf(DEBUG_INFO, "Failed to find adbe tunables info, bailing adbe_init()\n");
		return;
	}

	rADBE_ACTIVE_SIZE = ADBE_ACTIVE_SIZE_VERTICAL(timing->v_active) | ADBE_ACTIVE_SIZE_HORIZONTAL(timing->h_active);
	rADBE_FRONT_PORCH = ADBE_FRONT_PORCH_VERTICAL(timing->v_front_porch) | ADBE_FRONT_PORCH_HORIZONTAL(timing->h_front_porch );
	rADBE_SYNC_PULSE = ADBE_SYNC_PULSE_VERTICAL(timing->v_pulse_width) | ADBE_SYNC_PULSE_HORIZONTAL(timing->h_pulse_width);
	rADBE_BACK_PORCH = ADBE_BACK_PORCH_VERTICAL(timing->v_back_porch) | ADBE_BACK_PORCH_HORIZONTAL(timing->h_back_porch);

	//rdar://16085315 Errata Samsung’s DSIM_MDRESOL.MainStandby register field description. 
	rADBE_NRT_CTL &= ~ADBE_NRT_CTL_TE_MODE_MASK;	
	rADBE_NRT_CTL |= (ADBE_NRT_CTL_VSYNC_POLARITY(timing->neg_vsync) | ADBE_NRT_CTL_HSYNC_POLARITY(timing->neg_hsync));	
	rADBE_RT_CTL |= ADBE_RT_CTL_FRAME_COUNT_ENABLE;	
	rADBE_NRT_CTL |= ADBE_NRT_CTL_SCAN_SELECT_PROG_CHG;	

#ifdef TARGET_ADBE0_VBLANK_POSITION
	rADBE_VBLANK_POSITION = TARGET_ADBE0_VBLANK_POSITION;
#endif
	adbe_tuneables_info->adbe0_vblank_clk_gate_wakeup =  timing->v_back_porch + timing->v_front_porch + timing->v_pulse_width - 5;

	rADBE_VBLANK_CLK_GATE = 
		(ADBE_VBLANK_CLK_GATE_WAKEUP(adbe_tuneables_info->adbe0_vblank_clk_gate_wakeup) | 
		adbe_tuneables_info->adbe0_vblank_clk_gate_idle);

	//Set AAP
	rADBE_NRT_CTL |= (adbe_tuneables_info->adbe0_aap_support) ? ADBE_NRT_CTL_AAP_ENABLE: 0;

	// Dither
#ifdef WITH_HW_DITHER
	uint8_t dither_type = DITHER_SPATIO_TEMPORAL;
#ifdef TARGET_DITHER_TYPE
	dither_type = TARGET_DITHER_TYPE;
#endif //TARGET_DITHER_TYPE

	//Dithering gets enabled in this block, no need to call into dithering to enable
	//We need to enable to block to access their corresponding registers
	switch(dither_type) {
	case DITHER_SPATIO_TEMPORAL:
		rADBE_NRT_CTL |=  ADBE_NRT_CTL_ST_DITHER_ENABLE;
		break;
	case DITHER_BLUE_NOISE:
		rADBE_NRT_CTL |=  ADBE_NRT_CTL_BN_DITHER_ENABLE;
		break;
	case DITHER_NONE:
	default:
		rADBE_NRT_CTL &= ~(ADBE_NRT_CTL_ST_DITHER_ENABLE | ADBE_NRT_CTL_BN_DITHER_ENABLE);
		break;
	}
	
	dither_init(timing->display_depth);
#endif

	rADBE_NRT_CTL |= adbe_tuneables_info->adbe0_tunable_nrt_ctl;

	//Underrun setup
	rADBE_FIFO_CONFIG = ADBE_FIFO_CONFIG_UNDERRUN_MODE_STICKY | ADBE_FIFO_CONFIG_UNDERRUN_CH0(0x3FF);

}

static void adbe_enable_timing_generator_v3_video_mode(bool enable)
{
	if (enable) {
		rADBE_RT_CTL |= ADBE_RT_CTL_VFTG_ENABLE_VID_EN;			// Enable VFTG
		while ((rADBE_RT_CTL & ADBE_RT_CTL_VFTG_STATUS_EN) == 0);	// Wait for it  to be enabled 
	} else {
		rADBE_RT_CTL &= ~ADBE_RT_CTL_VFTG_ENABLE_VID_EN;			// Disable VFTG
		while ((rADBE_RT_CTL & ADBE_RT_CTL_VFTG_STATUS_EN) == ADBE_RT_CTL_VFTG_STATUS_EN);	// Wait for it  to be disabled 
	}
}

static void adbe_enable_timing_generator_v3_command_mode(bool enable)
{
	if (enable) {
		rADBE_RT_CTL |= ADBE_RT_CTL_VFTG_ENABLE_CMD_EN;			// Enable VFTG
		while ((rADBE_RT_CTL & ADBE_RT_CTL_VFTG_STATUS_EN) == 0);	// Wait for it  to be enabled 
	} else {
		rADBE_RT_CTL &= ~ADBE_RT_CTL_VFTG_ENABLE_CMD_EN;			// Disable VFTG
		while ((rADBE_RT_CTL & ADBE_RT_CTL_VFTG_STATUS_EN) == ADBE_RT_CTL_VFTG_STATUS_EN);	// Wait for it  to be disabled 
	}
}

void adbe_enable_timing_generator(bool enable)
{
#ifdef TARGET_DISP_VIDEO_MODE
	is_video_mode = TARGET_DISP_VIDEO_MODE;
#endif //TARGET_DISP_VIDEO_MODE

	if (is_video_mode) {
		rADBE_RT_CTL |= (ADBE_RT_CTL_UPDATE_ENABLE_TIMING_READY | ADBE_RT_CTL_UPDATE_REQ_TIMING_NOSYNC);
		adbe_enable_timing_generator_v3_video_mode(enable);
	} else {
		rADBE_NRT_CTL |= ADBE_NRT_CTL_TE_MODE_POS | ADBE_NRT_CTL_VFTG_MODE_COMMAND | ADBE_NRT_CTL_TE_ENABLE_SEL_SW;
		rADBE_RT_CTL |= (ADBE_RT_CTL_UPDATE_ENABLE_TIMING_READY | ADBE_RT_CTL_UPDATE_REQ_TIMING_NOSYNC);
		adbe_enable_timing_generator_v3_command_mode(enable);
	}
}

bool adbe_get_enable_timing_generator(void)
{
#ifdef TARGET_DISP_VIDEO_MODE
	is_video_mode = TARGET_DISP_VIDEO_MODE;
#endif //TARGET_DISP_VIDEO_MODE

	if (is_video_mode) {
		return ((rADBE_RT_CTL & ADBE_RT_CTL_VFTG_ENABLE_VID_EN) == (uint32_t)ADBE_RT_CTL_VFTG_ENABLE_VID_EN);
	} else {
		return ((rADBE_RT_CTL & ADBE_RT_CTL_VFTG_ENABLE_CMD_EN) == (uint32_t)ADBE_RT_CTL_VFTG_ENABLE_CMD_EN);
	}
}
