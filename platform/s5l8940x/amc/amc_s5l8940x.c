/*
 * Copyright (C) 2009-2012 Apple Inc. All rights reserved.
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
#include <debug.h>

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
#else // SUPPORT_FPGA
	rAMC_PWRMNGTEN |= 0x00001001;
#endif // SUPPORT_FPGA

	amc_mrcmd(MR_READ, AMC_NUM_CHANNELS, AMC_NUM_RANKS, 0x4, (uintptr_t)&odts);
	rAMC_ODTS         = 0x00010080;			// [16] => TempDrtEn, [9:0] RdIntrvl XXX based on tREFI

	rAMC_PWRMNGTEN |= 0x00010000;			// Enable srclkoff as well
	
	if (!resume) {
		amc_enable_autorefresh();
	}
	
#if !SUPPORT_FPGA
	// Turn of Fast Critical Word Forwarding feature and AIU CWF feature
	// Disabled for NRT <rdar://problem/9416881>
	rAMC_AIUPRT_RD_CWF     = 0x0001011f;
	rAMC_QBRPARAM          = 0x00020200;
	rAMC_QBREN             = 0x00111001;
#endif	
}

void amc_dram_workarounds (bool resume)
{
	const struct amc_memory_device_info *dev_info;
	uint32_t chip_id = platform_get_chip_id();
	uint8_t i;
	
	dev_info = amc_get_memory_device_info();
	
	switch (dev_info->vendor_id) {
	case JEDEC_MANUF_ID_SAMSUNG:

		// <rdar://problem/9454992> H4P Samsung 35nm LPDDR2 Transition for N94
		// TMRS for Samsung 46nm part only
		if ((chip_id == 0x8940) && (!resume) && (dev_info->rev_id == 0)) {
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
			
		// <rdar://problem/11060879> Samsung 3xnm DRAMs - not functional
		else if ((chip_id == 0x8942) && (!resume) && (dev_info->rev_id == 1) && (dev_info->rev_id2 == 0)) {
			// Test MRS Entry
			amc_mrcmd(MR_WRITE, AMC_NUM_CHANNELS, AMC_NUM_RANKS, 0x9, 0x20);
			amc_mrcmd(MR_WRITE, AMC_NUM_CHANNELS, AMC_NUM_RANKS, 0x9, 0x20);
			amc_mrcmd(MR_WRITE, AMC_NUM_CHANNELS, AMC_NUM_RANKS, 0x9, 0x20);
			amc_mrcmd(MR_WRITE, AMC_NUM_CHANNELS, AMC_NUM_RANKS, 0x9, 0xff);
				
			// PDL_M(-500ps)
			amc_mrcmd(MR_WRITE, AMC_NUM_CHANNELS, AMC_NUM_RANKS, 0x9, 0x80);
			amc_mrcmd(MR_WRITE, AMC_NUM_CHANNELS, AMC_NUM_RANKS, 0x9, 0x20);
			amc_mrcmd(MR_WRITE, AMC_NUM_CHANNELS, AMC_NUM_RANKS, 0x9, 0x01);
			
			// Latch Command
			amc_mrcmd(MR_WRITE, AMC_NUM_CHANNELS, AMC_NUM_RANKS, 0x9, 0x00);
			amc_mrcmd(MR_WRITE, AMC_NUM_CHANNELS, AMC_NUM_RANKS, 0x9, 0x00);
		}

		break;

	case JEDEC_MANUF_ID_HYNIX:

		if (dev_info->rev_id == 0) {
			if (!resume) {
				// Test MRS Exit "just in case" <rdar://problem/8769095>
				amc_mrcmd(MR_WRITE, AMC_NUM_CHANNELS, AMC_NUM_RANKS, 0x9, 0x00);

				// Apply Hynix cold clamp disable workaround <rdar://problem/8907487>
				// (but only for revid2 == 0 <rdar://problem/8943860>)
				if ((chip_id == 0x8940) && (dev_info->rev_id2 == 0)) {
					// Test MRS Entry
					amc_mrcmd(MR_WRITE, AMC_NUM_CHANNELS, AMC_NUM_RANKS, 0x9, 0xb0);
					amc_mrcmd(MR_WRITE, AMC_NUM_CHANNELS, AMC_NUM_RANKS, 0x9, 0xe0);
					amc_mrcmd(MR_WRITE, AMC_NUM_CHANNELS, AMC_NUM_RANKS, 0x9, 0x90);

					amc_mrcmd(MR_WRITE, AMC_NUM_CHANNELS, AMC_NUM_RANKS, 0x9, 0xa2);
				}

				// IOSTBP Start Time control (600ps delay)
				if ((chip_id == 0x8942) && (dev_info->density == JEDEC_DENSITY_1Gb)) {
					// Test MRS Entry
					amc_mrcmd(MR_WRITE, AMC_NUM_CHANNELS, AMC_NUM_RANKS, 0x9, 0xb0);
					amc_mrcmd(MR_WRITE, AMC_NUM_CHANNELS, AMC_NUM_RANKS, 0x9, 0xe0);
					amc_mrcmd(MR_WRITE, AMC_NUM_CHANNELS, AMC_NUM_RANKS, 0x9, 0x90);
					
					for (i = 0; i < 13; i++) {
						amc_mrcmd(MR_WRITE, AMC_NUM_CHANNELS, AMC_NUM_RANKS, 0x9, 0x99);
					}
				}
			}

#if !SUB_TARGET_K93 && !SUB_TARGET_K94 && !SUB_TARGET_K95 && !SUB_TARGET_J33
			// <rdar://problem/9585266> H4G UM Update: Modified setting for tMRR timing parameter
			// override tMRR to 3 cycles (only if performing dynamic calibration)
			if (dev_info->density == JEDEC_DENSITY_2Gb) {
				rAMC_MODE_FREQ(0) = amc_params.freq[0].modereg | 0x3;
			}
#endif
 		} else if ((dev_info->rev_id == 1) && (dev_info->rev_id2 == 0)) {
			
			// <rdar://problem/11059277> Hynix 3xnm workaround solution
			if ((chip_id == 0x8942) && (!resume)) {
				// Test MRS Entry
				amc_mrcmd(MR_WRITE, AMC_NUM_CHANNELS, AMC_NUM_RANKS, 0x9, 0xb0);
				amc_mrcmd(MR_WRITE, AMC_NUM_CHANNELS, AMC_NUM_RANKS, 0x9, 0xe0);
				amc_mrcmd(MR_WRITE, AMC_NUM_CHANNELS, AMC_NUM_RANKS, 0x9, 0x90);
				
				// Yi Enable +1 step
				for (i = 0; i < 4; i++) {
					amc_mrcmd(MR_WRITE, AMC_NUM_CHANNELS, AMC_NUM_RANKS, 0x9, 0xf1);
				}
				
				// Yi Pulse +1 step
				for (i = 0; i < 4; i++) {
					amc_mrcmd(MR_WRITE, AMC_NUM_CHANNELS, AMC_NUM_RANKS, 0x9, 0x94);
				}
			}
		}
		break;

	default:
		break;
	}
}

// Shift dq offset as needed.
// In the future, it's possible the shift value changes based on SOC or DRAM vendor.
// Thus, we need to have this function per SOC.
void amc_dram_shift_dq_offset (int8_t *dq_offset, uint8_t num_bytes) {
	int8_t shift_val;
	uint8_t i;
	const struct amc_memory_device_info *dev_info;
    uint32_t chip_id = platform_get_chip_id();

    // Do nothing for H4P
	if (chip_id == 0x8940)
        return;

    // <rdar://problem/11802718> H4A: Need additional DQ WR/RD static offsets
    dev_info = amc_get_memory_device_info();
    shift_val = 0;
    
    switch (dev_info->vendor_id) {			
        
        case JEDEC_MANUF_ID_SAMSUNG:
        case JEDEC_MANUF_ID_HYNIX:
            
            // -3 step shift for DRAMs not used in K93A or J33
            if (dev_info->rev_id > 0)
                shift_val = -3;

            break;            

        case JEDEC_MANUF_ID_ELPIDA:
            
            // -3 step shift for DRAMs not used in K93A or J33
            if (dev_info->rev_id > 1)
                shift_val = -3;

            break;
			
        default:
            dprintf(DEBUG_INFO, "Not applyig any shift to dq offsets for vendor: %d\n", dev_info->vendor_id);
            break;
    }
    
    for (i = 0; i < num_bytes; i++)
        dq_offset[i] += shift_val;
}
