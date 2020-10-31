/*
 * Copyright (C) 2013-2014 Apple Inc. All rights reserved.
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
#include <platform/soc/chipid.h>
#include <platform/soc/miu.h>
#include <platform/soc/pmgr.h>
#include <platform/clocks.h>
#include <platform/soc/hwclocks.h>
#include <platform/timer.h>

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
	/*
	The ROMADDRREMAP[1:0] is responsible for the remap

	1. ROMADDRREMAP[1] = 1’b0 - ROM mapped to PIO  
	1.1. ROMADDRREMAP[0] = 1’b0 -  ROM mapped to SECURE ROM 
	1.2. ROMADDRREMAP[0] = 1’b1 -  ROM mapped to AOP-SRAM 
	2. ROMADDRREMAP[1] = 1’b1 - ROM mapped to DRAM

	in other words
	ROMADDRREMAP[1:0] 
	2’b00 - SECURE ROM
	2’b01 - AOP-SRAM
	2’b1x - DDR
	*/

	switch (sel) {
		case REMAP_SRAM:
			rPIO_REMAP_CTL = (rPIO_REMAP_CTL & ~3) | (1 << 0);
			break;

		case REMAP_SDRAM:
			rPIO_REMAP_CTL = (rPIO_REMAP_CTL & ~3) | (2 << 0);
			break;

		// reset back to default behavior
		default:
			rPIO_REMAP_CTL = 0;
			break;
	}
}

void miu_bypass_prep(void)
{
}

#if WITH_DEVICETREE

void miu_update_device_tree(DTNode *pmgr_node)
{
	// Nothing to do here
}

#endif
