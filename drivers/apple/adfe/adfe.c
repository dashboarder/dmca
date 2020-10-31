/*
 * Copyright (C) 2009-2015 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */
#include "adfe_regs.h"

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

#include <drivers/adfe/adfe.h>

/* Display Tunables structures */
struct adfe_v1_tuneable {
	char *name;
	uint32_t disp_dpcclkcntl[2];
	uint32_t disp_dpbclklvl_clock_off_level[2];
	uint32_t disp_dpbclklvl_clock_on_level[2];
	uint32_t disp_dpusrcstrd[2];
	uint32_t disp_dpcqoscnfg[2];
	uint32_t disp_dpbqoslvl_med_watermark[2];
	uint32_t disp_dpbqoslvl_high_watermark[2];
	uint32_t disp_dpureqcfg[2];
	uint32_t disp_dpvreqcfg[2];
	uint32_t disp_dpvsrcstrd[2];
	uint32_t disp_dpcwbstrd[2];
};

#include <platform/soc/display.h>

#ifndef DISP_VERSION
#error DISP_VERSION undefined
#endif

static struct adfe_v1_tuneable *adfe_tuneables_info;
static const int32_t adfe_tuneables_list_size = sizeof(adfe_tuneables) / sizeof(struct adfe_v1_tuneable);

static void adfe_irq_handler(void *arg)
{
	int which = (int)arg;
	dprintf(DEBUG_INFO, "adfe_irq_handler: DPCIRQ(%d) is 0x%x -- attempting to clear\n", which, rDPCIRQ);
	rDPCIRQ = rDPCIRQ;
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
		dprintf(DEBUG_INFO, "Failed to find adfe tunables info, bailing adfe_init()\n");
		return;
	}
	
	adfe_enable_error_handler();

#if DISP_VERSION < 5
	// Turn on AutoMode
	rDPCPFDMA |= DPCPFDMA_AUTOMODE;

	// Set FIFO DMA Burst Size to 2 Words
	rDPCPFDMA = (rDPCPFDMA & ~DPCPFDMA_BURSTSIZE_MASK) | DPCPFDMA_BURSTSIZE_2WORDS;

	// Set FIFO DMA Water Mark to 0x400
	rDPCPFDMA = (rDPCPFDMA & ~DPCPFDMA_WATERMARK_MASK) | DPCPFDMA_WATERMARK(0x400);
#else
	// Turn on AutoMode
	rDPCPFMODE = DPCPFMODE_AUTOEN;
#endif

	// Set screen size
	rDPCSIZE = (timing->h_active << 16) | (timing->v_active << 0);

	// Set the blend layer clock gating on level based on which adfe
#ifdef DISP0_BASE_ADDR
	if (DISPLAYPIPE_BASE_ADDR == DISP0_BASE_ADDR) {
		rDPBCLKLVL = 
			adfe_tuneables_info->disp_dpbclklvl_clock_off_level[0] | 
			adfe_tuneables_info->disp_dpbclklvl_clock_on_level[0];
	} else 
#endif
#ifdef DISP1_BASE_ADDR
	if (DISPLAYPIPE_BASE_ADDR == DISP1_BASE_ADDR) {
		rDPBCLKLVL = 
			adfe_tuneables_info->disp_dpbclklvl_clock_off_level[1] | 
			adfe_tuneables_info->disp_dpbclklvl_clock_on_level[1];
	} else
#endif	
	panic("adfe_init: unsupported adfe_base_addr: %p", (void *)DISPLAYPIPE_BASE_ADDR);

#if (DISP_VERSION < 3)
	// Set blend layer panic level to 0x90
	rDPBPANLVL = 0x90;

	// Enable memory panic for blender with 5000 cycle delay
	rDPCPANCNFG = DPCPANCNFG_PANENAB | DPCPANCNFG_PANBENAB | DPCPANCNFG_PANTIMER(5000);
#else
#ifdef DISP0_BASE_ADDR
	if (DISPLAYPIPE_BASE_ADDR == DISP0_BASE_ADDR) {
		// Set the blend QOS level
		rDPBQOSLVL = 
			adfe_tuneables_info->disp_dpbqoslvl_med_watermark[0] | 
			adfe_tuneables_info->disp_dpbqoslvl_high_watermark[0];

		// Enable memory panic
		rDPCQOSCNFG = adfe_tuneables_info->disp_dpcqoscnfg[0];
	} else 
#endif
#ifdef DISP1_BASE_ADDR
	if (DISPLAYPIPE_BASE_ADDR == DISP1_BASE_ADDR) {
		// Set the blend QOS level
		rDPBQOSLVL = 
			adfe_tuneables_info->disp_dpbqoslvl_med_watermark[1] | 
			adfe_tuneables_info->disp_dpbqoslvl_high_watermark[1];

		// Enable memory panic
		rDPCQOSCNFG = adfe_tuneables_info->disp_dpcqoscnfg[1];
	} else
#endif	
	panic("adfe_init: unsupported adfe_base_addr: %p", (void *)DISPLAYPIPE_BASE_ADDR);

	// Set the CSC to pass through
	rDPBCSCCOEFR(0) = 0x1000;
	rDPBCSCCOEFG(1) = 0x1000;
	rDPBCSCCOEFB(2) = 0x1000;
#endif

#if 0
	// Enable clock gating but not in U0, U1 or V
	rDPCCLKCNTL = DPCCLKCNTL_GATEENAB;
#endif
#if (DISP_VERSION > 4)
	uint32_t i;
#ifdef DISP0_BASE_ADDR
	if (DISPLAYPIPE_BASE_ADDR == DISP0_BASE_ADDR) {

		for (i = 0; i < 2; i ++) {
			rDPCWBSTRD(i) = adfe_tuneables_info->disp_dpcwbstrd[0];
			rDPUSRCSTRD(i) = adfe_tuneables_info->disp_dpusrcstrd[0];
			rDPUREQCFG(i) = (rDPUREQCFG(i) & DPUREQCFG_CACHEHINT_MASK) | adfe_tuneables_info->disp_dpureqcfg[0];
		}
		
		for (i = 0; i < 3; i ++) {
			rDPVREQCFG(i) = (rDPVREQCFG(i) & DPVREQCFG_CACHEHINT_MASK) | adfe_tuneables_info->disp_dpvreqcfg[0];
			rDPVSRCSTRD(i) = adfe_tuneables_info->disp_dpvsrcstrd[0];
		}
		
		rDPCCLKCNTL = adfe_tuneables_info->disp_dpcclkcntl[0];
	} else 
#endif
#ifdef DISP1_BASE_ADDR
	if (DISPLAYPIPE_BASE_ADDR == DISP1_BASE_ADDR) {
		
		for (i = 0; i < 2; i ++) {
			rDPCWBSTRD(i) = adfe_tuneables_info->disp_dpcwbstrd[1];
			rDPUSRCSTRD(i) = adfe_tuneables_info->disp_dpusrcstrd[1];
			rDPUREQCFG(i) = (rDPUREQCFG(i) & DPUREQCFG_CACHEHINT_MASK) | adfe_tuneables_info->disp_dpureqcfg[1];
		}
		
		for (i = 0; i < 3; i ++) {
			rDPVREQCFG(i) = (rDPVREQCFG(i) & DPVREQCFG_CACHEHINT_MASK) | adfe_tuneables_info->disp_dpvreqcfg[1];
			rDPVSRCSTRD(i) = adfe_tuneables_info->disp_dpvsrcstrd[1];
		}
		
		rDPCCLKCNTL = adfe_tuneables_info->disp_dpcclkcntl[1];
	} else
#endif
		panic("adfe_init: unsupported adfe_base_addr: %p", (void *)DISPLAYPIPE_BASE_ADDR);
#endif

	// Set the blend pipe underrun color to red
	rDPBUNDRCNFG = (2 << 30) | (0x3FF << 20);
}

void adfe_set_ui_layer(u_int32_t layer, enum colorspace color, struct plane *plane0, struct plane *plane1,
				     u_int32_t width, u_int32_t height)
{
	uint32_t adfe_color;
	addr_t base;

	if (layer > 1) return;

	RELEASE_ASSERT(plane0 != NULL);

	base = (addr_t)(plane0->fb_virt);

	switch (color) {
	case CS_RGB565 :
		adfe_color = 4;
		break;
	case CS_RGB888 :
		adfe_color = 0;
		break;
	default:
		panic("unsupported color 0x%x", color);
		return;
	}
	// Set adfe color, no swap and linear frame buffer
	rDPUSRCFMT(layer) = (adfe_color << 8) | (0 << 4) | (1 << 0);

	// Set base address of this layer (lower 32 bits, in case of 64 bits address)
	rDPUSRCBASE(layer) = (uint32_t)(base);

	// Set stride bytes
	uint32_t rmw = rDPUSRCSTRD(layer);
	rmw &= ~DPUSRCSTRD_SRCSTRIDE_MASK;
	rmw |= (plane0->stride & DPUSRCSTRD_SRCSTRIDE_MASK);
	rDPUSRCSTRD(layer) = rmw;

#if (DISP_VERSION >= 3)
	// Set the source scale region geometry
	rDPUSRCXY(layer) = 0;
	rDPUSRCWH(layer) = (width << 16) | (height << 0);
	// Set the destination frame geometry
	rDPUDSTXY(layer) = 0;
	rDPUDSTWH(layer) = (width << 16) | (height << 0);
	// Enable source active region 0
	rDPUSRCRGN(layer) = (1 << 0);
#if (DISP_VERSION < 5)
	// No MMU for UI requests
	// This should not be done on DISP_VERSION >= 5, as it clears
	// HighAddr.
	rDPUMMUCNTL(layer) = 0;
#endif
	// No scaling (init=0,0 step=1.0,1.0)
	rDPUDDAINITX(layer) = 0;
	rDPUDDAINITY(layer) = 0;
	rDPUDDASTEPX(layer) = 0x100000;
	rDPUDDASTEPY(layer) = 0x100000;
#endif

#if (DISP_VERSION > 4)
	rDPUMMUCNTL(layer) = ((base >> 32) & 0xf) << 4;
#endif

	// Set first region for the requested window
	rDPUSRCSTRXY(layer, 0) = (0 << 16) | (0 << 0);
	rDPUSRCENDXY(layer, 0) = (width << 16) | (height << 0);

	// Set blend layer 1 to pass through layer 1
	rDPBLAY1CNFG = (0xFF << 24) | (0xFF << 16) | (2 << 8) | (2 << 0);

#if (DISP_VERSION < 3)
	// Set first region enabled
	rDPUSRCRGNENAB(layer) = (1 << 0);

	// Set ui layer fifo on / off levels to 0x20 / 0x60
	rDPUCLKLVL(layer) = (0x20 << 16) | (0x60 << 0);

	// Set ui layer panic level to 0x20
	rDPUPANLVL(layer) = (0x20 << 0);
#endif
}

void adfe_enable_error_handler(void)
{
#if (DISP_VERSION < 5)
// Error handling not implemented before version 5.
#else
	rDPCIRQENAB |= DPCIRQ_MSTRERR;
	install_int_handler(INT_DISP0, adfe_irq_handler, 0);
	unmask_int(INT_DISP0);
#endif
}

void adfe_disable_error_handler(void)
{
#if (DISP_VERSION < 5)
#else
	rDPCIRQENAB &= ~DPCIRQ_MSTRERR;
	mask_int(INT_DISP0);
#endif
}

void adfe_set_background_color(u_int32_t color)
{
	rDPBBACKCOLOR = color;
}

void adfe_activate_window(void)
{
	u_int32_t layer = 0;

	if (layer > 1) return;

	// Enable the requested layer
	rDPCENAB |= (1 << (layer + 8));
#if (DISP_VERSION > 4)
	// Enable VBI, its needed for AutoMode
	rDPCENAB |= (1 << 0);
#endif
}

bool adfe_get_enable(void)
{
#if (DISP_VERSION < 5)
	return (rDPCPFDMA & DPCPFDMA_AUTOMODE) != 0;
#else
	return (rDPCIRQENAB & DPCIRQ_MSTRERR) != 0;
#endif
}
