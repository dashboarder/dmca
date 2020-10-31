/*
 * Copyright (C) 2011 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */
#include <arch.h>
#include <drivers/miu.h>
#include <platform.h>
#include <platform/miu.h>
#include <platform/clocks.h>
#include <platform/memmap.h>
#include <platform/power.h>
#include <platform/soc/hwclocks.h>
#include <platform/soc/miu.h>
#include <platform/soc/power.h>
#include <sys.h>

/* memory controller stuff */

#if APPLICATION_IBOOT

int miu_initialize_dram(bool resume)
{
#if APPLICATION_IBOOT
	/* Turn off alias from 0x10000000 to 0x08000000 */
	rDDR1 = 1;

	/* bring up sdram */
	mcu_initialize_dram(resume);
#endif	
	return 0;
}

#endif /* APPLICATION_IBOOT */

int miu_initialize_internal_ram(void)
{
#if APPLICATION_SECUREROM
	// Ensure that rSTATESAVE0 and rSTATESAVE1 get cleared
	rSTATESAVE0 = 0;
	rSTATESAVE1 = 0;
#endif /* APPLICATION_SECUREROM */

	// Save the Security Epoch in the top byte of SAVESTATE0
	rSTATESAVE0 &= ~0xFF000000;
	rSTATESAVE0 |= (platform_get_security_epoch()) << 24;

	return 0;
}

void 
miu_select_remap(enum remap_select sel)
{
	switch (sel) {
		case REMAP_SRAM:
			rREMAP = (rREMAP & ~3) | (1 << 0) | (1 << 1); // remap, remap_sel = 1
			break;
		case REMAP_SDRAM:
			rREMAP = (rREMAP & ~3) | (1 << 0) | (0 << 1); // remap, remap_sel = 0
			break;
	}
}

void
miu_setup_mfc_address_filter(void)
{
#if ((PURPLE_GFX_MEMORY_LEN + PANIC_SIZE) & 0xFFFFF) != 0
#error "PurpleGfxMemory must be 1 MB-aligned"
#endif
	uint32_t range_end = SDRAM_BASE + (uint32_t)platform_get_memory_size();
	uint32_t range_base = range_end - PANIC_SIZE - PURPLE_GFX_MEMORY_LEN;
	rMFC_ARF_BASE_ADDR = range_base;
	rMFC_ARF_ADDR_MASK = ~(range_end - range_base) & 0xFFF00000;
	rMFC_ARF_CON = 1;
}

int miu_init(void)
{
#if APPLICATION_IBOOT && !PRODUCT_IBEC
	// Verify that the Security Epoch in STATESAVE0 matches
	if ((rSTATESAVE0 >> 24) != platform_get_security_epoch()) {
		panic("miu_init: Epoch Mismatch\n");
	}
#endif

	/* remap whatever bank of ram we're in to zero */
	if (TEXT_BASE == SRAM_BASE)
		miu_select_remap(REMAP_SRAM);
	else if ((TEXT_BASE >= SDRAM_BASE) && (TEXT_BASE < SDRAM_END))
		miu_select_remap(REMAP_SDRAM);

#if APPLICATION_IBOOT
	miu_setup_mfc_address_filter();
#endif

	return 0;
}
