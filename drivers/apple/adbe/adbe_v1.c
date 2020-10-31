/*
 * Copyright (C) 2012-2015 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */
#include "adbe_regs_v1.h"

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
#include <lib/mib.h>

struct adbe_v1_tuneable {
	char *name;
	uint32_t adbe0_tunable_mode_ctrl;
	uint32_t adbe0_vblank_pos_vbi_pulse;
	uint32_t adbe0_vblank_clk_gate_wakeup;
	uint32_t adbe0_vblank_clk_gate_idle;
	uint32_t adbe0_vblank_blk_idle_dpb;
	uint32_t adbe0_vblank_blk_idle_aap;
	uint32_t adbe0_aap_support;
};

#include <target/adbe_settings.h>

static struct adbe_v1_tuneable *adbe_tuneables_info;
static const int32_t adbe_tuneables_list_size = sizeof(adbe_tuneables) / sizeof(struct adbe_v1_tuneable);

static void adbe_program_color_manager()
{
	uint32_t i;
	uint32_t engamma_table_count = mib_get_u32(kMIBTargetDisplayCMEngammaTableCount);
	uint32_t degamma_table_count = mib_get_u32(kMIBTargetDisplayCMDegammaTableCount);
	uint32_t matrix_table_count = mib_get_u32(kMIBTargetDisplayCMMatrixTableCount);

	if (engamma_table_count != 0 && degamma_table_count != 0 && matrix_table_count != 0) {
		const uint32_t * engamma_table_base = (const uint32_t *)mib_get_ptr(kMIBTargetDisplayCMEngammaTablePtr);
		const uint32_t * degamma_table_base = (const uint32_t *)mib_get_ptr(kMIBTargetDisplayCMDegammaTablePtr);
		const uint32_t * matrix_table_base = (const uint32_t *)mib_get_ptr(kMIBTargetDisplayCMMatrixTablePtr);

		for (i = 0; i < degamma_table_count; i++) {
			rCM_DEGAMMA_RED(i) = degamma_table_base[i];
			rCM_DEGAMMA_GREEN(i) = degamma_table_base[i];
			rCM_DEGAMMA_BLUE(i) = degamma_table_base[i];
		}	

		for (i = 0; i < matrix_table_count; i++) {
			rCM_MATRIX_BASE(i) = matrix_table_base[i];
		}	
		for (i = 0; i < engamma_table_count; i++) {
			rCM_ENGAMMA_RED(i) = engamma_table_base[i];
			rCM_ENGAMMA_GREEN(i) = engamma_table_base[i];
			rCM_ENGAMMA_BLUE(i) = engamma_table_base[i];
		}	


		//enabe LUTs in the lut register
		rCM_LUT_CTL = CM_LUT_CTL_UPDATE_ENABLE_ENG_BLUE |
		    CM_LUT_CTL_UPDATE_REQ_ENG_BLUE |
		    CM_LUT_CTL_UPDATE_ENABLE_DEG_BLUE |
		    CM_LUT_CTL_UPDATE_REQ_DEG_BLUE |
		    CM_LUT_CTL_UPDATE_ENABLE_ENG_GREEN |
		    CM_LUT_CTL_UPDATE_REQ_ENG_GREEN |
		    CM_LUT_CTL_UPDATE_ENABLE_DEG_GREEN |
		    CM_LUT_CTL_UPDATE_REQ_DEG_GREEN |
		    CM_LUT_CTL_UPDATE_ENABLE_ENG_RED |
		    CM_LUT_CTL_UPDATE_REQ_ENG_RED |
		    CM_LUT_CTL_UPDATE_ENABLE_DEG_RED |
		    CM_LUT_CTL_UPDATE_REQ_DEG_RED;
	}
}

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

	rDBESCRNSZ = (timing->v_active << 16) | (timing->h_active << 0);
	rDBEFRONTPORCH = (timing->v_front_porch << 16) | (timing->h_front_porch << 0);
	rDBESYNCPULSE = (timing->v_pulse_width  << 16) | (timing->h_pulse_width << 0);
	rDBEBACKPORCH = (timing->v_back_porch << 16) | (timing->h_back_porch << 0);

	rDBEVBLANKPOS = adbe_tuneables_info->adbe0_vblank_pos_vbi_pulse;

#ifdef TARGET_ADBE0_VBLANK_POSITION
	rDBEVBLANKPOS |= TARGET_ADBE0_VBLANK_POSITION;
#endif

	adbe_tuneables_info->adbe0_vblank_clk_gate_wakeup = timing->v_back_porch + timing->v_front_porch + timing->v_pulse_width - 5;

	rDBEVBLANKCLKGATE = ((adbe_tuneables_info->adbe0_vblank_clk_gate_wakeup << 16) | adbe_tuneables_info->adbe0_vblank_clk_gate_idle);
	rDBEVBLANKIDLE = ((adbe_tuneables_info->adbe0_vblank_blk_idle_dpb << 16) | adbe_tuneables_info->adbe0_vblank_blk_idle_aap);

	//csc lut programming should occur here

	//Set AAP
	rDBEMODECNTL |= (adbe_tuneables_info->adbe0_aap_support) ? DBEMODECNTL_AAP_ENABLE: 0;

#ifdef WITH_DPB
	// Need to enable block first in order to access its registers
	rDBEMODECNTL |= DBEMODECNTL_DPB_ENABLE;
	dpb_init();
#endif

	// Dither
#ifdef WITH_HW_DITHER
	//Currently, no product uses BN on H6.
	uint8_t dither_type = DITHER_SPATIO_TEMPORAL;
#ifdef TARGET_DITHER_TYPE
	dither_type = TARGET_DITHER_TYPE;
#endif //TARGET_DITHER_TYPE

	//Dithering gets enabled in this block, no need to call into dithering to enable
	//We need to enable to block to access their corresponding registers
	switch(dither_type) {
	case DITHER_SPATIO_TEMPORAL:
		rDBEMODECNTL |=  DBEMODECNTL_ST_DITHER_ENABLE;
		break;
	case DITHER_BLUE_NOISE:
		rDBEMODECNTL |=  DBEMODECNTL_BN_DITHER_ENABLE;
		break;
	case DITHER_NONE:
	default:
		rDBEMODECNTL &= ~(DBEMODECNTL_ST_DITHER_ENABLE | DBEMODECNTL_BN_DITHER_ENABLE);
		break;
	}
	
	dither_init(timing->display_depth);
#endif
#ifdef TARGET_ENABLE_CM
	rDBEMODECNTL |= (DBEMODECNTL_PRE_CSC_LUT_ENABLE | DBEMODECNTL_CSC_ENABLE | DBEMODECNTL_POST_CSC_LUT_ENABLE);
	adbe_program_color_manager();

#endif

	rDBEMODECNTL |= adbe_tuneables_info->adbe0_tunable_mode_ctrl;

	// Radar <rdar://problem/13402498> J72dev2 - Innsbruck11A289 | Playing a video in full screen causes the screen to go blank
	// need to force an update to the timing before enabling the timing generator
	rDBEMODECNTL |= (DBEMODECNTL_UPDATE_ENABLE_TIMING | DBEMODECNTL_UPDATE_REQ_TIMING);
}

void adbe_enable_timing_generator(bool enable)
{
	if (enable) {
		rDBEMODECNTL |= DBEMODECNTL_ENABLE;			// Enable VFTG
		while ((rDBEMODECNTL & DBEMODECNTL_VFTG_STATUS) == 0);	// Wait for it  to be enabled 
	}
	else {
		rDBEMODECNTL &= ~DBEMODECNTL_ENABLE;			// Enable VFTG
		while ((rDBEMODECNTL & DBEMODECNTL_VFTG_STATUS) == 1);	// Wait for it  to be disabled 
	}
}

bool adbe_get_enable_timing_generator(void)
{
	return ((rDBEMODECNTL & DBEMODECNTL_ENABLE) == (uint32_t)DBEMODECNTL_ENABLE);
}
