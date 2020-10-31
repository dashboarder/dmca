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
#include "adbe_regs_v2.h"

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
#include <drivers/wpc.h>
#include <drivers/prc.h>
#include <drivers/adbe/adbe.h>

struct adbe_v2_tuneable {
	char *name;
	uint32_t adbe0_tunable_mode_ctrl;
	uint32_t adbe0_vftgctl_idle_frame_vblank_enable;
	uint32_t adbe0_vblank_pos_vbi_pulse;
	uint32_t adbe0_vblank_clk_gate_wakeup;
	uint32_t adbe0_vblank_clk_gate_idle;
	uint32_t adbe0_vblank_busy_finish;
	uint32_t adbe0_aap_support;
	uint32_t adbe0_aap_format_control_reg1;
};

#include <target/adbe_settings.h>

static struct adbe_v2_tuneable *adbe_tuneables_info;
static const int32_t adbe_tuneables_list_size = sizeof(adbe_tuneables) / sizeof(struct adbe_v2_tuneable);

void adbe_init(struct display_timing *timing)
{
	const char		*env;
	int32_t			cnt;
	
	env = env_get("adbe-tunables");
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

	rDBEVFTGCTL |= (DBEVFTGCT_VSYNC_POLARITY(timing->neg_vsync) | DBEVFTGCT_HSYNC_POLARITY(timing->neg_hsync));	
	rDBEVFTGCTL |= DBEVFTGCT_FRAME_COUNT_ENABLE | DBEVFTGCT_SCAN_SELECT(0x2);	
	rDBEVFTGCTL |= adbe_tuneables_info->adbe0_vftgctl_idle_frame_vblank_enable ? DBEVFTGCT_IDLE_FRAME_VBLANK_ENABLE : 0 ;

	rDBEVBLANK_POSITION = (rDBEVBLANK_POSITION & ~0xFFFF) | adbe_tuneables_info->adbe0_vblank_pos_vbi_pulse;

	rDBEVBLANKCLKGATE = ((adbe_tuneables_info->adbe0_vblank_clk_gate_wakeup << 16) | adbe_tuneables_info->adbe0_vblank_clk_gate_idle);
	
	rDBEVBLANKBUSY = (adbe_tuneables_info->adbe0_vblank_busy_finish << 16);

	//csc lut programming should occur here

	rAAP_FORMAT_CONTROL_REG1 = adbe_tuneables_info->adbe0_aap_format_control_reg1;
		
	//Set AAP
	rDBEMODECNTL |= (adbe_tuneables_info->adbe0_aap_support) ? DBEMODECNTL_AAP_ENABLE: 0;

#ifdef WITH_DPB
	// Need to enable block first in order to access its registers
	rDBEMODECNTL |= DBEMODECNTL_DPB_ENABLE;
#if DPB_VERSION < 2
	dpb_init();
#else
	dpb_init(timing->h_active, timing->v_active);
#endif
#endif

#ifdef WITH_WPC
	// Need to enable block first in order to access its registers
	rDBEMODECNTL |= DBEMODECNTL_WPC_ENABLE;
	wpc_init(timing->h_active, timing->v_active);
#endif

#ifdef WITH_PRC
	// Need to enable block first in order to access its registers
	rDBEMODECNTL |= DBEMODECNTL_PRC_ENABLE;
	prc_init(timing->h_active, timing->v_active);
#endif

// Dither
#if DITHER_VERSION < 4
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

#else // DITHER_VERSION >= 4
#ifdef WITH_HW_DITHER
	// Enable Dither Block
	rDBEMODECNTL |= DBEMODECNTL_DITHER_ENABLE;

	dither_init(timing->h_active, timing->v_active, timing->display_depth);
#endif
#endif

	rDBEMODECNTL |= adbe_tuneables_info->adbe0_tunable_mode_ctrl;

	rDBEFIFO_CONFIG = 0x400003ff;

	//disable interrupts
	rDBEISR = ((1 << 18) | (1 << 17) | (1 << 16));

	rDBEVFTGCTL |= (DBEVFTGCT_UPDATE_ENABLE_TIMING | DBEVFTGCT_UPDATE_REQ_TIMING);
}

void adbe_enable_timing_generator(bool enable)
{
	if (enable) {
		rDBEVFTGCTL |= DBEVFTGCT_VFTG_ENABLE;			// Enable VFTG
		while ((rDBEVFTGCTL & DBEVFTGCT_VFTG_STATUS) == 0);	// Wait for it  to be enabled 
	} else {
		rDBEVFTGCTL &= ~DBEVFTGCT_VFTG_ENABLE;			// Disable VFTG
		while ((rDBEVFTGCTL & DBEVFTGCT_VFTG_STATUS) == DBEVFTGCT_VFTG_STATUS);	// Wait for it  to be disabled 
	}
}

bool adbe_get_enable_timing_generator(void)
{
	return ((rDBEVFTGCTL & DBEVFTGCT_VFTG_ENABLE) == (uint32_t)DBEVFTGCT_VFTG_ENABLE);
}
