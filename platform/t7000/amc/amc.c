/*
 * Copyright (C) 2013 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */
#include <debug.h>
#include <drivers/amc/amc.h>
#include <drivers/amc/amc_phy.h>
#include <drivers/amc/amc_regs.h>
#include <drivers/dram.h>
#include <drivers/miu.h>
#include <sys.h>
#include <platform/soc/hwclocks.h>

int mcu_initialize_dram (bool resume)
{
	// Don't perform DRAM initializion if the memory controller is already
	// initialized (e.g., for SecureROM validation testing).
	if (resume || rAMC_AMCEN == 0) {
		clock_gate(CLK_MCU, true);
		clock_gate(CLK_MCC, true);
		clock_gate(CLK_AMP, true);

		amc_init(resume);
	}

	return 0;
}

void mcu_bypass_prep (int step)
{
	amc_phy_bypass_prep(step);
}

uint64_t mcu_get_memory_size (void)
{
	return amc_get_memory_size();
}

void amc_configure_address_decoding_and_mapping(void)
{
	rAMC_MCSADDRBNKHASH(2) = 0x000036db;
	rAMC_MCSADDRBNKHASH(1) = 0x00005b6d;
	rAMC_MCSADDRBNKHASH(0) = 0x00006db6;

	rAMC_ADDRMAP_MODE = amc_get_params()->addrmapmode;
}

void amc_enable_slow_boot (bool enable)
{
	if (enable) {
		// switch to slow clock
		clocks_set_performance(kPerformanceMemoryLow);
		spin(1);
	}
	else {
#if !SUPPORT_FPGA
		// switch back to full-speed
		clocks_set_performance(kPerformanceMemoryFull);
		spin(1);
#endif		
	}
}

void amc_finalize (bool resume)
{
	// Enabling clock gating and AutoSR only after wrdqcal is done
	rAMC_CLKPWRGATE = 0x050a0000;
	rAMC_CLKPWRGATE2 = 0x00660300;
    
	// <rdar://problem/16239984> Alcatraz, Fiji, Capri: L2_TB: MCU PIO accesses to the same address and device stream getting reordered in CP
	platform_memory_barrier();
	
	rAMC_PWRMNGTEN 	&= ~0x00100000;
	
#if !SUPPORT_FPGA
	rAMC_PWRMNGTEN 	|= 0x00011011;

	// These 2 values are replaced later in the sequence when tunables are programmed
	rAMC_PWRMNGTPARAM = 0x01803000;
	rAMC_PSQWQCTL1 = 0x00000120;
#else
	rAMC_PWRMNGTEN	|= 0x00000010;
#endif
}

void amc_dram_workarounds (bool resume)
{
	const struct amc_memory_device_info *dev_info;

	dev_info = amc_get_memory_device_info();

	switch (dev_info->vendor_id) {
	case JEDEC_MANUF_ID_HYNIX:
		// No work-arounds for Hynix ... yet!
		break;

	case JEDEC_MANUF_ID_ELPIDA:
		/* Filter only for Elpida 25nm to apply test MRS */
		if ((dev_info->rev_id == 0x2) && (dev_info->rev_id2 <= 0x2)) {  // MR6 = 2(25nm) and MR7 = 2(Rev2)
			if (!resume) {
				// Elpida has no TestMRS entry/exit cmd
				// <rdar://problem/15465954> Fiji: Need Duty Cycle Adjust work-around to improve Fiji+Elpida 25nm DDR eye margin
				amc_mrcmd(MR_WRITE, AMC_NUM_CHANNELS, AMC_NUM_RANKS, 0x9, 0xbb);
				amc_mrcmd(MR_WRITE, AMC_NUM_CHANNELS, AMC_NUM_RANKS, 0x9, 0x27);
			}
		}
		break;

	default:
		break;
	}

	return;
}
