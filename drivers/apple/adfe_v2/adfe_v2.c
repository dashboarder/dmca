/*
 * Copyright (C) 2014-2015 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */
#include "adfe_v2_regs.h"

#include <debug.h>
#include <drivers/display.h>
#include <lib/env.h>
#include <lib/paint.h>
#include <platform.h>
#include <platform/chipid.h>
#include <platform/clocks.h>
#include <platform/int.h>
#include <platform/memmap.h>

#include <platform/soc/hwclocks.h>
#include <platform/soc/hwisr.h>
#include <sys.h>
#include <sys/task.h>

#include <drivers/adfe_v2/adfe.h>
#if TARGET_USES_GP_CM || TARGET_USES_BLEND_CM
#include <target/display_gamma_tables.h>
#endif //TARGET_USES_GP_CM || TARGET_USES_BLEND_CM
#if WITH_SYSCFG
#include <lib/syscfg.h>
#endif
#if WITH_DEVICETREE
#include <lib/devicetree.h>
#endif

struct adfe_v2_tuneable {
	char *name;
	uint32_t disp_dpcclkcntl;
	uint32_t disp_dpcclklvl_clock_on_level;
	uint32_t disp_dpcclklvl_clock_off_level;
	uint32_t disp_dpcqoscfg;
	uint32_t disp_dpcqosylvl_yellow_off;
	uint32_t disp_dpcqosylvl_yellow_on;
	uint32_t disp_dpcqosrlvl_red_off;
	uint32_t disp_dpcqosrlvl_red_on;
	uint32_t disp_dpclinkdown;
	uint32_t disp_dpgpreqaggr;
	uint32_t disp_dpcafclkgate; //ADP_VERSION > 1
	uint32_t disp_dpcenab;
};

#include <platform/soc/display.h>

#ifndef ADP_VERSION
#error ADP_VERSION undefined
#endif

static void adfe_set_pixel_pipe_plane(u_int32_t pixel_pipe, u_int32_t plane_num, struct plane *plane);
static void adfe_gp_gamut_adjustment(u_int32_t pixel_pipe);
static void adfe_blend_gamut_adjustment(void);

static uint8_t display_pipe;
static uint8_t gen_pixel_pipe = 0;

static struct adfe_v2_tuneable *adfe_tuneables_info;
static const int32_t adfe_tuneables_list_size = sizeof(adfe_tuneables) / sizeof(struct adfe_v2_tuneable);

static void adfe_irq_handler(void *arg)
{
	int which = (int)arg;
	dprintf(DEBUG_INFO, "adfe_irq_handler: ADPCIRQ(%d) is 0x%x -- attempting to clear\n", which, rADPCIRQ);
	rADPCIRQ = rADPCIRQ;
}

void adfe_init(struct display_timing *timing)
{
	const char		*env;
	int32_t			cnt;
	
	env = env_get("adfe-tunables");
	if (env == 0) env = "default";

	for (cnt = 0; cnt < adfe_tuneables_list_size; cnt++) {
		if (strcmp(env, adfe_tuneables[cnt].name)) continue;

		adfe_tuneables_info = adfe_tuneables + cnt;
	}
	if (adfe_tuneables_info == 0) {
		panic("Failed to find adfe tunables info, bailing adfe_init()\n");
	}
	
	display_pipe = 0;
	gen_pixel_pipe = 0;

#ifdef DISP1_BASE_ADDR
	if (DISPLAYPIPE_BASE_ADDR == DISP1_BASE_ADDR) {
		display_pipe = 1;
		gen_pixel_pipe = 0;
	}
#endif //DISP1_BASE_ADDR

	adfe_enable_error_handler();

	// Turn on AutoMode
	rADPCPFMODE = ADPCPFMODE_AUTOEN | ADPCPFMODE_VBIEN;

	// Set screen size
	rADPCFRAMESIZE = (timing->h_active << 16) | (timing->v_active << 0);

	// Set the pipes clock gating on level  (generic to both pipes)
	rADPCCLKLVL = adfe_tuneables_info->disp_dpcclklvl_clock_off_level | 
		      adfe_tuneables_info->disp_dpcclklvl_clock_on_level;

	// Set the Memory QOS configuration
	rADPCQOSCFG = adfe_tuneables_info->disp_dpcqoscfg;

	//set output FIFO QOS Yellow level
	rADPCQOSYLVL = adfe_tuneables_info->disp_dpcqosylvl_yellow_on | 
		       adfe_tuneables_info->disp_dpcqosylvl_yellow_off;

	//set output FIFO QOS Red level
	rADPCQOSRLVL = adfe_tuneables_info->disp_dpcqosrlvl_red_on | 
		       adfe_tuneables_info->disp_dpcqosrlvl_red_off;

	rADPCCLKCNTL = adfe_tuneables_info->disp_dpcclkcntl;
	rADPCLINKDOWN = adfe_tuneables_info->disp_dpclinkdown;
	
	// Register added in Capri..
#if ADP_VERSION > 1
	rADPCAFCLKGATE = adfe_tuneables_info->disp_dpcafclkgate;
#endif


	// Disable CSC
	rADPCCRCCTRL = 0;
}

void adfe_set_axis_flip(uint32_t pixel_pipe, bool flipX, bool flipY)
{
	rADPGPSRCFMT(pixel_pipe) |= ((!!flipX) << 2) | ((!!flipY) << 1);
}

// Algorithm provided by display team to do fixed point rounding.
static uint64_t fixed_to_fixed( uint64_t in, int inprec, int outprec )
{
	long long sval = *((long long *) &in);
	bool neg = sval < 0;

	if( neg ) sval = -sval;
	if( outprec < inprec ) {
		sval = ((sval >> (inprec - outprec - 1)) + 1) >> 1;
	} else { 
		sval <<= outprec - inprec;
	}

	if( neg ) sval = -sval;
	return *((uint64_t *) &sval);
}

/*
 * The ADP has 2 Generic Pixel Pipes. These pipes process pixels from memory before entering the Blend Unit.
 * A Plane is a single contigous location in memory containing pixel information
 * No scaling of any component is required (i.e. DDAInitX/DDAInitY are equal to SrcX/SrcY, and all DDA steps are equal to 1).
 * The Scale Region is equal to the Destination Region.
 * All pixels in the scale region are also in an Active Region (ARGB formats only).
 * 2DSN is disabled (YCbCr 4:4:4 formats only).
 */
void adfe_set_ui_layer(u_int32_t pixel_pipe, enum colorspace color, struct plane *plane0, struct plane *plane1,
	u_int32_t width, u_int32_t height)
{
	uint8_t num_planes;
	uint8_t pixel_size, pixel_fmt;
	uint8_t is_ARGB;
	uint32_t reg = 0;

	if (pixel_pipe > 1)  return;

	switch(color) {
	case CS_RGB565 :
		is_ARGB = true;
		pixel_size = 2;
		pixel_fmt = 2;
		num_planes = 1; 
		break;
	case CS_RGB888 :
		is_ARGB = true;
		pixel_size = 1;
		pixel_fmt = 1;
		num_planes = 1; 
		break;
	case CS_ARGB8101010 :
		is_ARGB = true;
		pixel_size = 3;
		pixel_fmt = 0;
		num_planes = 2; 
		break;
	default:
		return;
	}
	RELEASE_ASSERT(plane0 != NULL);
	if (num_planes > 1) RELEASE_ASSERT(plane1 != NULL);


#if defined(__arm64__)
	rADPSMMU_BYPASS_ADDR = (((((addr_t)plane0->fb_virt) >> 32) & 0xF) << 8);
#endif
	rADPSMMU_DIAG_BOGUS_ACCESS = 0x20ffff;
	
	rADPDART_DIAG_CONFIG = 0x100;	
	rADPDART_DIAG_BOGUS_ACCESS = 0x20ffff;
	rADPDART_FETCH_REQ_CONFIG = 0xe0303;

	// Set displaypixel_pipe color, no swap and linear frame buffer
	reg = rADPGPSRCFMT(pixel_pipe);
	//clean up the fields we are about to set
	reg &= ~((0x3 << 26) | (0x3 << 24) | (1 << 22) | (1 << 21) | (1 << 20) | (0x1f << 4) | (1 << 0));
	reg |= (pixel_fmt << 26) | (pixel_size << 24) | ((num_planes - 1 ) << 22) |  (is_ARGB << 21) | (0 << 4) | (1 << 0);
	rADPGPSRCFMT(pixel_pipe) = reg;

        adfe_set_pixel_pipe_plane(pixel_pipe, 0, plane0);
	if (num_planes > 1) {
		adfe_set_pixel_pipe_plane(pixel_pipe, 1, plane1);
	}

	// Enable source active region 0
	rADPGPSRCCFG(pixel_pipe) = (1 << 0);

	// Set the destination frame geometry
	rADPGPDSTXY(pixel_pipe) = 0;
	rADPGPDSTWH(pixel_pipe) = (width << 16) | (height << 0);

	//No Scale Mode
	rADPGPCFG(pixel_pipe) = ADPGPCFG_NO_SCALE;

	//Dithering off
	rADPGPNOISECFG(pixel_pipe) = 0;

	//No color Space Conversion
	rADPGPCSCCFG(pixel_pipe)= ADPGPCSCCFG_CSC_BYPASS;

	// No Color Management
	rADPGPCMCFG(pixel_pipe)= ADPGPCMCFG_CM_BYPASS;

	//Bypass Color space conversion
	rADPBCSCCFG = ADPBCSCCFG_CSCBYPASS;

	adfe_gp_gamut_adjustment(pixel_pipe);

	// Bypass Blend Color Management
	rADPBCMCFG = ADPBCMCFG_CMBYPASS;

	//Tunable
	rADPGPREQAGGR(pixel_pipe, 0) = adfe_tuneables_info->disp_dpgpreqaggr;
		
	rADPGPREQAGGR(pixel_pipe, 1) = adfe_tuneables_info->disp_dpgpreqaggr;

	//No Need for Blend
#define BLEND_BYPASS 1
#if BLEND_BYPASS
	rADPBCFG = ADPBCFG_BLEND_BYPASS;
#endif //BLEND_BYPASS
	rADPBLAYCFG(pixel_pipe) = (1 << 4) | (pixel_pipe << 0);
	rADPBLAYCFG(1-pixel_pipe) = (0 << 4) | ((1-pixel_pipe) << 0);
	//rADPBBACKCOLOR = 0xf;
	
	adfe_blend_gamut_adjustment();
	
}

static void adfe_set_pixel_pipe_plane(u_int32_t pixel_pipe, u_int32_t plane_num, struct plane *plane)
{
	// Set base address of this pixel_pipe (lower 32 bits, in case of 64 bits address)
	rADPGPSRCBASE(pixel_pipe, plane_num) = (uint32_t)plane->fb_virt;

	// Set burst size and stride bytes
	rADPGPSRCSTRD(pixel_pipe, plane_num) = plane->stride;

	// Set the source scale region geometry
	rADPGPSRCXY(pixel_pipe, plane_num) = 0;
	rADPGPSRCWH(pixel_pipe, plane_num) = (plane->width << 16) | (plane->height << 0);

	//Note that if the pipe is being used in No Scale or Scaler Bypass mode DdaInitX must equal SrcX
	rADPGPDDAINITX(pixel_pipe, plane_num) = 0;
	rADPGPDDASTEPX(pixel_pipe, plane_num) = 0x100000;
	//Note that if the pipe is being used in No Scale or Scaler Bypass mode DdaInitY must equal SrcY
	rADPGPDDAINITY(pixel_pipe, plane_num) = 0;
	rADPGPDDASTEPY(pixel_pipe, plane_num) = 0x100000;

	// Set first region for the requested window
	rADPGPSRCACTXY(pixel_pipe, plane_num) = (0 << 16) | (0 << 0);
	rADPGPSRCACTWH(pixel_pipe, plane_num) = (plane->width << 16) | (plane->height << 0);
}

void adfe_enable_error_handler(void)
{
	rADPCIRQENAB |= ADPCIRQ_OUTUNDERIRQ;
	install_int_handler(INT_DISP0, adfe_irq_handler, 0);
	unmask_int(INT_DISP0);
}

void adfe_disable_error_handler(void)
{
	rADPCIRQENAB &= ~ADPCIRQ_OUTUNDERIRQ;
	mask_int(INT_DISP0);
}

void adfe_set_background_color(u_int32_t color)
{
	rADPBBACKCOLOR = color;
}

void adfe_activate_window(void)
{
	if (gen_pixel_pipe > 1) return;

	//Set SID to bypass due to
	//<rdar://problem/13098516> SMMU Configuration Timing
	// Enable the requested generic pixel pipe
	rADPCENAB = adfe_tuneables_info->disp_dpcenab | ADPCENAB_SIDSEL_BYPASS | (1 << (gen_pixel_pipe * 4));
}

bool adfe_get_enable(void)
{
	return (rADPCIRQENAB & ADPCIRQ_OUTUNDERIRQ) != 0;
}

static void adfe_blend_gamut_adjustment(void)
{
#if TARGET_USES_BLEND_CM
#if WITH_SYSCFG
	struct primary_calibration_matrix {
		uint32_t version;
		uint32_t matrix[3][3];
	};

	struct primary_calibration_matrix pcm;

	if (syscfgCopyDataForTag('DPCl', (u_int8_t *)&pcm, sizeof(pcm)) > 0) {

		int row, col;

		//Program degamma LUT
		for (row = 0; row < ADP_CM_DEGAMMA_PAIRED_LUT_SIZE; row++) {
			rADPBDEGAMMATABLE(row) = target_blend_degamma_tables[row];
		}

		//Program Linear Degamma
		rADPBDEGAMMALINEAR = target_blend_degamma_tables[row];

		for(row = 0; row < 3; row ++) {
			for(col = 0; col < 3; col++) {
				// Convert from 8.24 to 4.12
				rADPBCMCOEF(row, col) = (uint32_t)(fixed_to_fixed( pcm.matrix[row][col], 24, 12 ) & 0xFFFF);
			}
		}

		for (row = 0; row < ADP_CM_ENGAMMA_PAIRED_LUT_SIZE; row++) {
			rADPBGAMMATABLE_R(row) = target_blend_engamma_tables[row];
			rADPBGAMMATABLE_G(row) = target_blend_engamma_tables[row];
			rADPBGAMMATABLE_B(row) = target_blend_engamma_tables[row];
		}

		rADPBGAMMALINEAR_R = target_blend_engamma_tables[row];
		rADPBGAMMALINEAR_G = target_blend_engamma_tables[row];
		rADPBGAMMALINEAR_B = target_blend_engamma_tables[row];

		// Turn off Blend Color Management Bypass
		rADPBCMCFG &= ~ADPBCMCFG_CMBYPASS;
	}
#endif
#endif
}

static void adfe_gp_gamut_adjustment(u_int32_t pixel_pipe)
{
#if TARGET_USES_GP_CM
	int col;
	int row;

	//Program degamma LUT
	for (row = 0; row < ADP_CM_DEGAMMA_PAIRED_LUT_SIZE; row++) {
		rADPGPDEGAMMATABLE(pixel_pipe, row) = target_gp_degamma_tables[row];
	}
	
	//Program Linear Degamma
	rADPGPDEGAMMALINEAR(pixel_pipe) = target_gp_degamma_tables[row];

	for(row = 0; row < 3; row ++ ) {
		for(col = 0; col < 3; col ++ ) {
			rADPGPCMCOEF(pixel_pipe, row, col) = target_gp_csc_matrix[row][col];
		}
	}

	for (row = 0; row < ADP_CM_ENGAMMA_PAIRED_LUT_SIZE; row++) {
		rADPGPGAMMATABLE(pixel_pipe, row) = target_gp_engamma_tables[row];
	}

	rADPGPGAMMALINEAR(pixel_pipe) = target_gp_engamma_tables[row];

	//Unbypass the gamut adjustment block
	rADPGPCMCFG(pixel_pipe) &= ~ADPGPCMCFG_CM_BYPASS;

#endif //TARGET_USES_GP_CM
}
