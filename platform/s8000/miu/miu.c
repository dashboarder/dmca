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
#include <arch.h>
#include <debug.h>
#include <drivers/miu.h>
#include <platform.h>
#include <platform/memmap.h>
#include <platform/miu.h>
#include <platform/soc/miu.h>
#include <platform/soc/pmgr.h>
#include <platform/clocks.h>
#include <platform/soc/chipid.h>
#include <platform/soc/hwclocks.h>

#if WITH_DEVICETREE
#if SUPPORT_FPGA

struct bridge_list_t {
	char		*bridge_settings;
	uint32_t	fpga_enable_mask;
};

#define BRIDGE(b, m)	{ "bridge-settings-" #b, (m) }

// ALERT: The ordering effects the device tree entries under pmgr node's reg dictionary.
//	  It also affects the bridge id used in the pmgr device-clock nodes. The
//	  bridge id must match the id on the device tree bridge-settings-<n> properties.
// NOTE:  The second parameter on the BRIDGE macro is used to dynamically determine
//	  which bridge settings properties should exist based upon the hardware
//	  blocks actually present on the device.

#if SUB_PLATFORM_S8000 || SUB_PLATFORM_S8003
static const struct bridge_list_t bridge_list[] = {
	BRIDGE(0, FPGA_HAS_MEDIA),	// Media Bus Mux
	BRIDGE(1, FPGA_HAS_MSR),	// MSR_AXI2AF
	BRIDGE(2, FPGA_HAS_JPEG),	// AJPEG_AXI2AF
	BRIDGE(3, FPGA_HAS_AVE),	// AVE_AXI2AF
	BRIDGE(4, FPGA_HAS_VXD),	// VDEC0_AXI2AF
	BRIDGE(5, FPGA_HAS_ISP),	// ISP_DMA_AXI2AF + ISAP_KF_AXI2AF
	BRIDGE(6, (FPGA_HAS_DISP)),	// DISPBusMux
	BRIDGE(7, FPGA_HAS_DISP),	// Disp0
	BRIDGE(8, FPGA_HAS_GFX),	// GFX
	BRIDGE(9, FPGA_HAS_GFX),	// GFX_AFUSER
	BRIDGE(10, FPGA_HAS_ALWAYS),	// USB
	BRIDGE(11, FPGA_HAS_ALWAYS),	// PCIe
};

#elif SUB_PLATFORM_S8001
static const struct bridge_list_t bridge_list[] = {
	BRIDGE(0, FPGA_HAS_MEDIA),	// Media Bus Mux
	BRIDGE(1, FPGA_HAS_MSR),	// MSR_AXI2AF
	BRIDGE(2, FPGA_HAS_JPEG),	// AJPEG_AXI2AF
	BRIDGE(3, FPGA_HAS_AVE),	// AVE_AXI2AF
	BRIDGE(4, FPGA_HAS_VXD),	// VDEC0_AXI2AF
	BRIDGE(5, FPGA_HAS_ISP),	// ISP_DMA_AXI2AF + ISAP_KF_AXI2AF
	BRIDGE(6, FPGA_HAS_DISP),	// RTBusMux
	BRIDGE(7, FPGA_HAS_DISP),	// Disp0
	BRIDGE(8, FPGA_HAS_DISP),	// Disp1
	BRIDGE(9, FPGA_HAS_DISP),	// Disp1BusMux
	BRIDGE(10, FPGA_HAS_GFX),	// GFX
	BRIDGE(11, FPGA_HAS_GFX),	// GFX_AFUSER
	BRIDGE(12, FPGA_HAS_ALWAYS),	// USB
	BRIDGE(13, FPGA_HAS_ALWAYS),	// PCIe
};
#endif

#endif // SUPPORT_FPGA
#endif // WITH_DEVICETREE

extern void ausb_setup_widgets();

int miu_initialize_internal_ram(void)
{
#if APPLICATION_SECUREROM
	// Ensure that rPMGR_SCRATCH0-3 get cleared
	rPMGR_SCRATCH0 = 0;
	rPMGR_SCRATCH1 = 0;
	rPMGR_SCRATCH2 = 0;
	rPMGR_SCRATCH3 = 0;
#endif /* APPLICATION_SECUREROM */
	
	// Save the Security Epoch in the top byte of PMGR_SCRATCH0
	rPMGR_SCRATCH0 &= ~0xFF000000;
	rPMGR_SCRATCH0 |= (platform_get_security_epoch()) << 24;

	return 0;
}

int miu_init(void)
{
#if APPLICATION_IBOOT && !PRODUCT_IBEC
	// Verify that the Security Epoch in PMGR_SCRATCH0 matches
	if ((rPMGR_SCRATCH0 >> 24) != platform_get_security_epoch()) {
		panic("miu_init: Epoch Mismatch\n");
	}
#endif
	ausb_setup_widgets();

	return 0;
}

void miu_suspend(void)
{
	/* nothing required for suspend */
}

int miu_initialize_dram(bool resume)
{
#if APPLICATION_IBOOT && WITH_HW_DCS
	mcu_initialize_dram(resume);
#endif
	return 0;
}

void miu_select_remap(enum remap_select sel)
{
	switch (sel) {
		case REMAP_SRAM:
			rSECUREROMCTRL_ROMADDRREMAP = (rSECUREROMCTRL_ROMADDRREMAP & ~3) | (1 << 0);
			break;

		case REMAP_SDRAM:
			rSECUREROMCTRL_ROMADDRREMAP = (rSECUREROMCTRL_ROMADDRREMAP & ~3) | (2 << 0);
			break;
	}
}

void miu_bypass_prep(void)
{
}

#if WITH_DEVICETREE

#define DEBUG_BRIDGE	DEBUG_SPEW		// Debug level for bridge messages

void miu_update_device_tree(DTNode *pmgr_node)
{
#if SUPPORT_FPGA
	DTNode		*node;
	char		*propName;
	void		*propData;
	uint32_t	propSize;
	uint32_t 	fpga_blocks = chipid_get_fpga_block_instantiation();
	uint32_t	i;

		dprintf(DEBUG_INFO, "chipid_get_fpga_block_instantiation() = 0x%08X\n",
		fpga_blocks);

		if (FindNode(0, "arm-io/pmgr", &node)) {

		// For each bridge...
		for (i = 0; i < ARRAY_SIZE(bridge_list); i++) {

			// Filter out if not supported by FPGA
			if (!(fpga_blocks & bridge_list[i].fpga_enable_mask)) {

				propName = bridge_list[i].bridge_settings;

				if (FindProperty(node, &propName, &propData, &propSize)) {
					dprintf(DEBUG_CRITICAL, "Eliding %s\n", propName);
					propName[0] = '~';
				}
			}
		}
	}
#endif // SUPPORT_FPGA
}

#endif // WITH_DEVICETREE
