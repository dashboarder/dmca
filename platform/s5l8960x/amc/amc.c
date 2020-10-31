/*
 * Copyright (C) 2011-2013 Apple Inc. All rights reserved.
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

		// Configure MCC
		rMCC_MCCEN	= 0x00000190;
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
	rAMC_ADDRCFG = 0x00020201;
	
	// <rdar://problem/16239984> Alcatraz, Fiji, Capri: L2_TB: MCU PIO accesses to the same address and device stream getting reordered in CP
	platform_memory_barrier();
	
	if (AMC_NUM_RANKS > 1)
		rAMC_ADDRCFG |= 0x01000000;

	rAMC_CHNLDEC = amc_get_params()->chnldec;

	rAMC_ADDRMAP_MODE = 2;				// RIBI2

	rAMC_MCSADDRBNKHASH(0) = 0x00006db6;
	rAMC_MCSADDRBNKHASH(1) = 0x00005b6d;
	rAMC_MCSADDRBNKHASH(2) = 0x000036db;
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

// Some AMC features to be changed before calibration starts, and restored after calibration is complete
void amc_calibration_start(bool start)
{
	// <rdar://problem/16239984> Alcatraz, Fiji, Capri: L2_TB: MCU PIO accesses to the same address and device stream getting reordered in CP
	platform_memory_barrier();
	
	if (start) {
		// Disable OdtsRdIntrvl
		rAMC_ODTS &= 0xFFFFFC00;
	} else {
		// Re-enable OdtsRdIntrvl
		rAMC_ODTS |= amc_get_params()->odts;
	}
}


void amc_enable_rddqcal(bool enable)
{
	// <rdar://problem/16239984> Alcatraz, Fiji, Capri: L2_TB: MCU PIO accesses to the same address and device stream getting reordered in CP
	platform_memory_barrier();
	
	static uint8_t lppr_freq0 = 0;

	if (enable) {
		lppr_freq0 = (rAMC_AREFEN_FREQ(0) >> 16) & 1;
		rAMC_AREFEN_FREQ(0) &= ~(1 << 16);
		rAMC_READ_LEVELING |= 1;
	} else {
 		rAMC_READ_LEVELING &= ~1;
		rAMC_AREFEN_FREQ(0) |= (lppr_freq0 << 16);
	}
}

void amc_wrdqcal_start(bool start)
{
	if (start) {
		
		// Enable WriteMergeEn and WqInOrderEn
		rAMC_PSQWQCTL0 = (1 << 8) | (1 << 0);
		
		// <rdar://problem/16239984> Alcatraz, Fiji, Capri: L2_TB: MCU PIO accesses to the same address and device stream getting reordered in CP
		platform_memory_barrier();
		
		// Set SelfRefTmrVal to max
		rAMC_PWRMNGTPARAM |= (0xFFFF << 16);
		
		// Designer suggests 0xBB, so that writes do not age out
		rAMC_PSQWQCTL1 = 0xBB;
		
	} else {
		rAMC_PSQWQCTL0 = 0;
		rAMC_PWRMNGTPARAM = amc_get_params()->pwrmngtparam_guided;
		// WqAgeOutVal has to be set to 3/4 of SelfRefTmrVal
		rAMC_PSQWQCTL1 = (3 * ((amc_get_params()->pwrmngtparam_guided & 0xFFFF0000) >> 16)) >> 2;
		
		// Enabling clock gating and AutoSR only after wrdqcal is done
		rAMC_CLKPWRGATE = 0x050a0000;
		rAMC_CLKPWRGATE2 = 0x00010300;
		
		// <rdar://problem/16239984> Alcatraz, Fiji, Capri: L2_TB: MCU PIO accesses to the same address and device stream getting reordered in CP
		platform_memory_barrier();
		
		rAMC_PWRMNGTEN 	|= 0x00011011;
	}
}

void amc_finalize (bool resume)
{
	// <rdar://problem/16239984> Alcatraz, Fiji, Capri: L2_TB: MCU PIO accesses to the same address and device stream getting reordered in CP
	platform_memory_barrier();
	
	rAMC_AREFPARAM	|= 0x00001000;			// Turn on freq change waiting for refresh and self-refresh exit
    
	rAMC_QBREN	= 0x00110001;

	if (!resume) {
	#if SUPPORT_FPGA
		rAMC_ODTS	= amc_get_params()->odts;
	#else
		rAMC_ODTS	= 0x00010000 | amc_get_params()->odts;
	#endif
		amc_enable_autorefresh();
	}

	rAMC_ZQC = 0x010c03ff;
	rAMC_QBRPARAM	= 0x00030000;
	rAMC_QBREN	= 0x00111001;

#if SUPPORT_FPGA
	rAMC_CLKPWRGATE = 0x050a0000;
	rAMC_CLKPWRGATE2 = 0x00010300;
	
	// <rdar://problem/16239984> Alcatraz, Fiji, Capri: L2_TB: MCU PIO accesses to the same address and device stream getting reordered in CP
	platform_memory_barrier();
	
	rAMC_PWRMNGTEN 	|= 0x00000010;
#endif
}

void amc_dram_workarounds (bool resume)
{
	const struct amc_memory_device_info *dev_info;
	
	dev_info = amc_get_memory_device_info();
	
	switch (dev_info->vendor_id) {
		case JEDEC_MANUF_ID_HYNIX:
			
			// <rdar://problem/15486279> Alcatraz: Hynix 29nm DDR margin improvement settings for N5x/J72/J85 PRQ - iBoot
			if (dev_info->rev_id == 2 && ((dev_info->rev_id2 == 0) || (dev_info->rev_id2 == 1) || (dev_info->rev_id2 == 2))) {
				if (!resume) {
					// Test MRS Exit
					amc_mrcmd(MR_WRITE, AMC_NUM_CHANNELS, AMC_NUM_RANKS, 0x9, 0x00);

					// Test MRS Entry
					amc_mrcmd(MR_WRITE, AMC_NUM_CHANNELS, AMC_NUM_RANKS, 0x9, 0xb0);
					amc_mrcmd(MR_WRITE, AMC_NUM_CHANNELS, AMC_NUM_RANKS, 0x9, 0xe0);
					amc_mrcmd(MR_WRITE, AMC_NUM_CHANNELS, AMC_NUM_RANKS, 0x9, 0x90);

					// DCD -13ps
					amc_mrcmd(MR_WRITE, AMC_NUM_CHANNELS, AMC_NUM_RANKS, 0x9, 0xb4);
					amc_mrcmd(MR_WRITE, AMC_NUM_CHANNELS, AMC_NUM_RANKS, 0x9, 0x94);
					amc_mrcmd(MR_WRITE, AMC_NUM_CHANNELS, AMC_NUM_RANKS, 0x9, 0x86);
					amc_mrcmd(MR_WRITE, AMC_NUM_CHANNELS, AMC_NUM_RANKS, 0x9, 0x83);

					// DCD +35ps
					amc_mrcmd(MR_WRITE, AMC_NUM_CHANNELS, AMC_NUM_RANKS, 0x9, 0xb4);
					amc_mrcmd(MR_WRITE, AMC_NUM_CHANNELS, AMC_NUM_RANKS, 0x9, 0x90);
					amc_mrcmd(MR_WRITE, AMC_NUM_CHANNELS, AMC_NUM_RANKS, 0x9, 0xba);
					amc_mrcmd(MR_WRITE, AMC_NUM_CHANNELS, AMC_NUM_RANKS, 0x9, 0x83);

					// Duty Control: 49%
					amc_mrcmd(MR_WRITE, AMC_NUM_CHANNELS, AMC_NUM_RANKS, 0x9, 0xb4);
					amc_mrcmd(MR_WRITE, AMC_NUM_CHANNELS, AMC_NUM_RANKS, 0x9, 0x84);
					amc_mrcmd(MR_WRITE, AMC_NUM_CHANNELS, AMC_NUM_RANKS, 0x9, 0x86);
					amc_mrcmd(MR_WRITE, AMC_NUM_CHANNELS, AMC_NUM_RANKS, 0x9, 0x83);
				}
			}
			if (dev_info->rev_id == 1 && dev_info->rev_id2 == 0) {
				if (!resume) {
					
					// TestMRS Exit
					amc_mrcmd(MR_WRITE, AMC_NUM_CHANNELS, AMC_NUM_RANKS, 0x9, 0x00);
					
					// TestMRS entry
					amc_mrcmd(MR_WRITE, AMC_NUM_CHANNELS, AMC_NUM_RANKS, 0x9, 0xb0);
					amc_mrcmd(MR_WRITE, AMC_NUM_CHANNELS, AMC_NUM_RANKS, 0x9, 0xe0);
					amc_mrcmd(MR_WRITE, AMC_NUM_CHANNELS, AMC_NUM_RANKS, 0x9, 0x90);
					
					// <rdar://problem/12674968>
					// DCD -13ps
					amc_mrcmd(MR_WRITE, AMC_NUM_CHANNELS, AMC_NUM_RANKS, 0x9, 0x87);
					amc_mrcmd(MR_WRITE, AMC_NUM_CHANNELS, AMC_NUM_RANKS, 0x9, 0xc0);
					amc_mrcmd(MR_WRITE, AMC_NUM_CHANNELS, AMC_NUM_RANKS, 0x9, 0x90);
					amc_mrcmd(MR_WRITE, AMC_NUM_CHANNELS, AMC_NUM_RANKS, 0x9, 0xe0);

					// DCD +35ps
					amc_mrcmd(MR_WRITE, AMC_NUM_CHANNELS, AMC_NUM_RANKS, 0x9, 0xbb);
					amc_mrcmd(MR_WRITE, AMC_NUM_CHANNELS, AMC_NUM_RANKS, 0x9, 0xc0);
					amc_mrcmd(MR_WRITE, AMC_NUM_CHANNELS, AMC_NUM_RANKS, 0x9, 0x8c);
					amc_mrcmd(MR_WRITE, AMC_NUM_CHANNELS, AMC_NUM_RANKS, 0x9, 0xe0);

					// DS up: 10%
					amc_mrcmd(MR_WRITE, AMC_NUM_CHANNELS, AMC_NUM_RANKS, 0x9, 0x8f);
					amc_mrcmd(MR_WRITE, AMC_NUM_CHANNELS, AMC_NUM_RANKS, 0x9, 0x94);
					amc_mrcmd(MR_WRITE, AMC_NUM_CHANNELS, AMC_NUM_RANKS, 0x9, 0xe0);
				}
			}
			break;
		
		case JEDEC_MANUF_ID_ELPIDA:
			
			if (dev_info->rev_id == 2 && ((dev_info->rev_id2 == 0) || (dev_info->rev_id2 == 1) || (dev_info->rev_id2 == 2))) {
				if (!resume) {
					// Elpida has no TestMRS entry/exit cmd
					// <rdar://problem/15581848> Alcatraz: Elpida 25nm DRAM read DQS DCD improvement for N5x/J72/J85 PRQ - iBoot
					amc_mrcmd(MR_WRITE, AMC_NUM_CHANNELS, AMC_NUM_RANKS, 0x9, 0xbb);
					amc_mrcmd(MR_WRITE, AMC_NUM_CHANNELS, AMC_NUM_RANKS, 0x9, 0x27);
				}
			}
			if (dev_info->rev_id == 0 && dev_info->rev_id2 == 0) {
				if (!resume) {
					// Elpida has no TestMRS entry/exit cmd
					// <rdar://problem/13495888> Elpida 38nm needs duty cycle adjustment
					amc_mrcmd(MR_WRITE, AMC_NUM_CHANNELS, AMC_NUM_RANKS, 0x9, 0xbb);
					amc_mrcmd(MR_WRITE, AMC_NUM_CHANNELS, AMC_NUM_RANKS, 0x9, 0x03);
				}
			}
			break;
			
		default:
			break;
	}
	
	return;
}

// Shift dq offset (for rd and wr) as needed <rdar://problem/13888162>
void amc_dram_shift_dq_offset (int8_t *dq_offset, uint8_t num_bytes) {
	int8_t shift_val;
	uint8_t i;
	const struct amc_memory_device_info *dev_info;
	
	dev_info = amc_get_memory_device_info();
	
	// shift for each vendor defined in platform amcconfig (for PoP) or target amcconfig (for Discrete)
	switch (dev_info->vendor_id) {
		case JEDEC_MANUF_ID_HYNIX:
			shift_val = amc_get_params()->offset_shift_hynix;
			break;
			
		case JEDEC_MANUF_ID_ELPIDA:
			// shift 1 step to the negative side
			shift_val = amc_get_params()->offset_shift_elpida;

#if SUB_TARGET_J71 || SUB_TARGET_J72 || SUB_TARGET_J73
			if (dev_info->rev_id == 2 && (dev_info->rev_id2 <= 2))
				shift_val = amc_get_params()->offset_shift_elpida25nm;
#endif
			break;
			
		default:
			shift_val = 0;
			break;
	}
	
	// "num_bytes" is actually size of the array, which is number of bits in this case (32)
	for (i = 0; i < num_bytes; i++)
		dq_offset[i] += shift_val;
}
