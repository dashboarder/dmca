/*
 * Copyright (C) 2010-2012 Apple Inc. All rights reserved.
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

	rAMC_AREFPARAM |= (0x00001000);			// Turn on freq change waiting for refresh and self-refresh exit

	amc_mrcmd(MR_READ, AMC_NUM_CHANNELS, AMC_NUM_RANKS, 0x4, (uintptr_t)&odts);
	rAMC_ODTS      = 0x00010080;			// [16] => TempDrtEn, [9:0] RdIntrvl XXX based on tREFI

	if (!resume) {
		amc_enable_autorefresh();
	}

#if !SUPPORT_FPGA
	// Turn of Fast Critical Word Forwarding feature and AIU CWF feature
	// Disabled for NRT <rdar://problem/9416881>
	rAMC_AIUPRT_RD_CWF     = 0x00363600;
	rAMC_QBRPARAM          = 0x00810000;
	rAMC_QBREN             = 0x00111001;
#endif
}

void amc_dram_workarounds (bool resume)
{
	const struct amc_memory_device_info *dev_info;
	
	dev_info = amc_get_memory_device_info();
	
	switch (dev_info->vendor_id) {
	case JEDEC_MANUF_ID_SAMSUNG:

		// <rdar://problem/9454992> H4P Samsung 35nm LPDDR2 Transition for N94
		// TMRS for Samsung 46nm part only
		if ((!resume) && (dev_info->rev_id == 0)) {
			// Test MRS Entry
			amc_mrcmd(MR_WRITE, AMC_NUM_CHANNELS, AMC_NUM_RANKS, 0x9, 0x20);
			amc_mrcmd(MR_WRITE, AMC_NUM_CHANNELS, AMC_NUM_RANKS, 0x9, 0x20);
			amc_mrcmd(MR_WRITE, AMC_NUM_CHANNELS, AMC_NUM_RANKS, 0x9, 0x20);
			amc_mrcmd(MR_WRITE, AMC_NUM_CHANNELS, AMC_NUM_RANKS, 0x9, 0xff);

			// Apply Samsung workaround <rdar://problem/9032765>

			// Column Select Line pull in (230ps) TMRS(1)
			amc_mrcmd(MR_WRITE, AMC_NUM_CHANNELS, AMC_NUM_RANKS, 0x9, 0x80);
			amc_mrcmd(MR_WRITE, AMC_NUM_CHANNELS, AMC_NUM_RANKS, 0x9, 0x04);
			amc_mrcmd(MR_WRITE, AMC_NUM_CHANNELS, AMC_NUM_RANKS, 0x9, 0x08);

			// Latch command
			amc_mrcmd(MR_WRITE, AMC_NUM_CHANNELS, AMC_NUM_RANKS, 0x9, 0x00);
			amc_mrcmd(MR_WRITE, AMC_NUM_CHANNELS, AMC_NUM_RANKS, 0x9, 0x00);

			// Column Select Line pull in (230ps) TMRS(2)
			amc_mrcmd(MR_WRITE, AMC_NUM_CHANNELS, AMC_NUM_RANKS, 0x9, 0x40);
			amc_mrcmd(MR_WRITE, AMC_NUM_CHANNELS, AMC_NUM_RANKS, 0x9, 0x01);
			amc_mrcmd(MR_WRITE, AMC_NUM_CHANNELS, AMC_NUM_RANKS, 0x9, 0x20);

			// Latch command
			amc_mrcmd(MR_WRITE, AMC_NUM_CHANNELS, AMC_NUM_RANKS, 0x9, 0x00);
			amc_mrcmd(MR_WRITE, AMC_NUM_CHANNELS, AMC_NUM_RANKS, 0x9, 0x00);
		}
		break;

	case JEDEC_MANUF_ID_HYNIX:

		if (dev_info->rev_id == 0) {
			if (!resume && (dev_info->rev_id2 == 0)) {
				// Test MRS Exit "just in case" <rdar://problem/8769095>
				amc_mrcmd(MR_WRITE, AMC_NUM_CHANNELS, AMC_NUM_RANKS, 0x9, 0x00);

				// Test MRS Entry
				amc_mrcmd(MR_WRITE, AMC_NUM_CHANNELS, AMC_NUM_RANKS, 0x9, 0xb0);
				amc_mrcmd(MR_WRITE, AMC_NUM_CHANNELS, AMC_NUM_RANKS, 0x9, 0xe0);
				amc_mrcmd(MR_WRITE, AMC_NUM_CHANNELS, AMC_NUM_RANKS, 0x9, 0x90);

				amc_mrcmd(MR_WRITE, AMC_NUM_CHANNELS, AMC_NUM_RANKS, 0x9, 0xa2);
			}

			// <rdar://problem/9585266> H4G UM Update: Modified setting for tMRR timing parameter
			// override tMRR to 3 cycles
			if (dev_info->density == JEDEC_DENSITY_2Gb)
				rAMC_MODE_FREQ(0) = amc_params.freq[0].modereg | 0x3;
		}
		
		break;
	default:
		break;
	}
}

void amc_dram_shift_dq_offset (int8_t *dq_offset, uint8_t num_bytes) {
    // Do nothing for H4G
}

