/*
 * Copyright (C) 2010-2014 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */

#include <drivers/amc/amc.h>
#include <drivers/amc/amc_phy.h>
#include <drivers/amc/amc_regs.h>
#include <drivers/dram.h>
#include <drivers/miu.h>
#include <sys.h>

int mcu_initialize_dram (bool resume)
{
	return amc_init(resume);
}

void mcu_bypass_prep (int step)
{
	amc_phy_bypass_prep(step);
}

uint64_t mcu_get_memory_size (void)
{
	return amc_get_memory_size();
}

void amc_configure_address_decoding_and_mapping (void)
{
	amc_configure_default_address_decoding_and_mapping();
}

void amc_enable_slow_boot (bool enable)
{
	int slow_freqsel;
	
	if (enable) {
		if ((amc_params.flags & FLAG_AMC_PARAM_SLOW_BOOT) == false) {
			rAMC_FREQSEL = amc_params.freqsel;
			return;
		}

		slow_freqsel = 3;
		rAMC_BOOTCLK = (amc_params.bootclkdivsr << 8) | 1;
		spin(5);
		rAMC_FREQSEL = slow_freqsel;
		amc_phy_scale_dll(slow_freqsel, amc_params.bootclkdivsr);		
	} else {
		if ((amc_params.flags & FLAG_AMC_PARAM_SLOW_BOOT) == false) {
			return;
		}
		
		rAMC_PWRMNGTPARAM = amc_params.pwrmngtparam_small;
		spin(2);

		rAMC_BOOTCLK &= ~1;
		spin(5);
		rAMC_FREQSEL = amc_params.freqsel;
		amc_phy_scale_dll(amc_params.freqsel, 1);

		rAMC_PWRMNGTPARAM = amc_params.pwrmngtparam_guided;
	}
}

void amc_finalize (bool resume)
{
	uint8_t odts;

#if SUPPORT_FPGA
	rAMC_PWRMNGTEN |= 0x00000001;			// Sharing clocks between channels so do not enable clock stopping for PD or SR
#else
	rAMC_PWRMNGTEN |= 0x00011011;
#endif

	rAMC_AREFPARAM    |= (0x00001000);		// Turn on freq change waiting for refresh and self-refresh exit

	amc_mrcmd(MR_READ, AMC_NUM_CHANNELS, AMC_NUM_RANKS, 0x4, (uintptr_t)&odts);
	rAMC_ODTS         = 0x00010320;			// [16] => TempDrtEn, [9:0] RdIntrvl XXX based on tREFI

	if (!resume) {
		amc_enable_autorefresh();
	}

#if SUPPORT_FPGA
	rAMC_FREQSEL = amc_params.freqsel;
#endif

#if !SUPPORT_FPGA
	// Turn of Fast Critical Word Forwarding feature and AIU CWF feature
	// Disabled for NRT <rdar://problem/9416881>
	rAMC_AIUPRT_RD_CWF     = 0x00161600;
	rAMC_QBRPARAM          = 0x00810000;
	rAMC_QBREN             = 0x00111001;
#endif
}

void amc_dram_workarounds (bool resume)
{
	// XXX - Possibly look in s5l8950x amc.c to see possible workarounds.
	return;
}

// Shift dq offset as needed.
// In the future, it's possible the shift value changes based on SOC or DRAM vendor.
// Thus, we need to have this function per SOC.
void amc_dram_shift_dq_offset (int8_t *dq_offset, uint8_t num_bytes) {
	int8_t shift_val;
	uint8_t i;
	const struct amc_memory_device_info *dev_info;
	
	dev_info = amc_get_memory_device_info();
	
	switch (dev_info->vendor_id) {			
		case JEDEC_MANUF_ID_HYNIX:
		case JEDEC_MANUF_ID_SAMSUNG:
		case JEDEC_MANUF_ID_ELPIDA:
			
			// shift 2 steps
			shift_val = -2;
			break;
			
		default:
			shift_val = 0;
			break;
	}
	
	for (i = 0; i < num_bytes; i++)
		dq_offset[i] += shift_val;	
}
