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

#include <debug.h>
#include <drivers/amc/amc.h>
#include <drivers/amc/amc_phy.h>
#include <drivers/amp/amp_v1.h>
#include <drivers/dram.h>
#include <drivers/miu.h>
#include <platform.h>
#include <platform/chipid.h>
#include <sys.h>

#define CA_PIVOTS_LIST_SIZE		(9)
#define DQ_PIVOTS_LIST_SIZE		(5)

static const int ca_pivot_ps[CA_PIVOTS_LIST_SIZE] = {-400, -300, -200, -100, 0, 100, 200, 300, 400};
static const int dq_pivot_ps[DQ_PIVOTS_LIST_SIZE] = {-300, -150, 0, 150, 300};
static const struct amc_phy_calibration_data amp_calibration_data =  {
	.bit_time_ps 		= 1875,
	.min_ca_window_width_ps = 150,
	.min_dq_window_width_ps = 150,
	.ca_pivot_step_size_ps 	= 100,
	.dq_pivot_step_size_ps 	= 150,
	.max_offset_setting	= 63,
	.min_offset_factor	= 0,
	.dll_f2_delay_fs	= 10,
	.ca_pivots_list_size 	= CA_PIVOTS_LIST_SIZE,
	.ca_pivot_ps 		= (int *)ca_pivot_ps,
	.dq_pivots_list_size 	= DQ_PIVOTS_LIST_SIZE,
	.dq_pivot_ps 		= (int *)dq_pivot_ps,
	.use_resume_boot_flow = true,  // to use static CA but dynamic DQ calibration
};
static uint32_t amp_dllupdtctrl[AMC_NUM_CHANNELS];

void amc_phy_init(bool resume)
{
	int i, d, f;

	for (i = 0; i < AMC_NUM_CHANNELS; i++) {
		int ch = AMP_INDEX_FOR_CHANNEL(i);			// cope with hole in AMP address space

		rAMP_AMPEN(ch) = 1;

#if !SUPPORT_FPGA
		rAMP_DQDQSDS(ch) = 0x0a0a0a0a;
		rAMP_NONDQDS(ch) = 0x0a0a0a0a;
#endif

		if (amc_phy_params.flags & FLAG_AMP_PARAM_FULLDIFFMODE) {
			for (f = 0; f < AMP_FREQUENCY_SLOTS; f++)
				rAMP_DIFFMODE_FREQ(ch, f) = 0x00000121;
		}
		
		for (f = 0; f < AMP_FREQUENCY_SLOTS; f++) {
			for (d = 0; d < AMP_NUM_DLL; d++) {
				rAMP_DQSINDLLSCL_FREQ(ch,d,f) = amc_phy_params.freq[f].scl;
				rAMP_DQOUTDLLSCL_FREQ(ch,d,f) = amc_phy_params.freq[f].scl;
			}
			rAMP_CAOUTDLLSCL_FREQ(ch,f) = amc_phy_params.freq[f].scl;
			rAMP_CTLOUTDLLSCL_FREQ(ch,f) = amc_phy_params.freq[f].scl;
		}

#if !SUPPORT_FPGA		
		for (f = 0; f < AMP_FREQUENCY_SLOTS; f++) {
			rAMP_RDCAPCFG_FREQ(ch,f) = amc_phy_params.freq[f].rdcapcfg;
		}

		rAMP_DLLEN(ch) = 0x100;
		// Reset the master DLL to make sure that master DLL locking happens from scratch
		rAMP_DLLEN(ch) |= 0x1;
		rAMP_DLLEN(ch) &= ~0x1;
		amc_phy_run_dll_update(ch);
#endif

		rAMP_AMPINIT(ch) = 0x1;

#if !SUPPORT_FPGA
		rAMP_IMPCALCMD(ch) = 0x101;
		while (rAMP_IMPCALCMD(ch) & 0x1) ;
#endif
	}
}

void amc_phy_enable_dqs_pulldown(bool enable)
{
}

void amc_phy_scale_dll(int freqsel, int factor)
{
	int i;

	for (i=0; i<AMC_NUM_CHANNELS; i++) {
		int ch = AMP_INDEX_FOR_CHANNEL(i);			// cope with hole in AMP address space
		
		rAMP_AMPCLK(ch) = freqsel << 16;
		amc_phy_run_dll_update(ch);
	}
}

bool amc_phy_rddq_cal()
{
    	// use static CA offset and calibrated & shifted DQ offsets
	return false;
}

void amc_phy_bypass_prep(int step)
{
	int i;

	// Make sure AMP was inited.
	// Assumptions: all supported channels are inited together, 
	// 		and channel 0 is always used.
	if ((rAMP_AMPINIT(0) & 1) == 0)
		return;

	switch (step) {
	case 0:
		for (i = 0; i < AMC_NUM_CHANNELS; i++) {
			int ch = AMP_INDEX_FOR_CHANNEL(i);			// cope with hole in AMP address space

			// change the DllUpdtIntvl and DllFastUpdtIntvl to 0.  
			// DllUpdtAlwaysOn and DllFastUpdtAlwaysOn should be enabled
			rAMP_DLLUPDTINTVL(ch) = (1 << 28) | (1 << 12);
		
			// disable DLLEn
			rAMP_DLLEN(ch) = 0;
		};
	break;
		
	case 1:
		for (i = 0; i < AMC_NUM_CHANNELS; i++) {
			int ch = AMP_INDEX_FOR_CHANNEL(i);			// cope with hole in AMP address space
		
			// Turn off periodic DLL update
			// Taken care of in step(0)
			
			// Turn off periodic impedance auto-calibration
			rAMP_IMPAUTOCAL(ch) = 0;
		};
	break;
	}
}

void amc_phy_finalize()
{
	uint8_t i;
	
	// Change Master DLL from Always-on mode to periodic-on mode
	for(i = 0; i < AMC_NUM_CHANNELS; i++) {
		int ch = AMP_INDEX_FOR_CHANNEL(i);			// cope with hole in AMP address space
		
		rAMP_DLLUPDTINTVL(ch) = ((1 << 28) | (0 << 12) | (0x80 << 0));
		if (amc_phy_params.flags & FLAG_AMP_PARAM_SUPPORT_FMCLK_OFF)
			rAMP_AMPCLK(ch) |= (1 << 8);

		// Make sure the pmgr_ENABLE_CLK_GATE.MCU_CLK_FIXED_GATE_ENABLE is enabled in the PMGR
		
		// AMC should be enabled so that it can respond to phyupd_req from AMP
		rAMP_IMPAUTOCAL(ch) = 0x00010514;
	}
}

void amc_phy_reset_fifos()
{
}

const struct amc_phy_calibration_data * amc_phy_get_calibration_data()
{
	return &amp_calibration_data;
}

void amc_phy_calibration_init(uint32_t *fine_lock_ps)
{
	uint8_t i;
	
	const struct amc_memory_device_info *dev_info;
	dev_info = amc_get_memory_device_info();
	
	ASSERT(fine_lock_ps != NULL);
	
	*fine_lock_ps = 0;
	
	for (i = 0; i < AMC_NUM_CHANNELS; i++) {
		int ch = AMP_INDEX_FOR_CHANNEL(i);			// cope with hole in AMP address space
		amp_dllupdtctrl[ch] = rAMP_DLLUPDTCTRL(ch);
		
		rAMP_DLLUPDTCTRL(ch) &= ~((3 << 20) | (1 << 16));	// DLLUptMode = 0, DllUpdtPhyUpdtTyp = 0
		
		*fine_lock_ps += amp_calibration_data.bit_time_ps / (((rAMP_MDLLCODE(ch) & 0x7f) + amp_calibration_data.dll_f2_delay_fs) * 4);
		
		rAMP_CALPATCFG(ch) = (0x3 << 8);
		rAMP_DQTRNCFG(ch) = 0x3f;
		
		if ( ((dev_info->vendor_id == JEDEC_MANUF_ID_SAMSUNG) && (dev_info->rev_id == 0)) ||
			 ((dev_info->vendor_id == JEDEC_MANUF_ID_HYNIX) && (dev_info->density == JEDEC_DENSITY_1Gb)) )
			rAMP_CALDQMSK(ch) = 0xfefefefe;
		else
			rAMP_CALDQMSK(ch) = 0x00000000;
	}

	*fine_lock_ps = *fine_lock_ps / AMC_NUM_CHANNELS;
}

void amc_phy_run_dll_update(uint8_t ch)
{
	ch = AMP_INDEX_FOR_CHANNEL(ch);					// cope with hole in AMP address space
	rAMP_DLLUPDTCMD(ch) = 0x1;
	while ((rAMP_DLLUPDTCMD(ch) & 0x1) != 0) ;	
}

void amc_phy_set_addr_offset(uint8_t ch, uint8_t value)
{
	rAMP_CAOUTDLLOVRRD(AMP_INDEX_FOR_CHANNEL(ch)) = value;
}

void amc_phy_set_rd_offset(uint8_t ch, uint8_t byte, uint8_t value)
{
	rAMP_DQSINDLLOVRRD(AMP_INDEX_FOR_CHANNEL(ch), byte) = value;
}

void amc_phy_set_wr_offset(uint8_t ch, uint8_t byte, uint8_t value)
{
	rAMP_DQOUTDLLOVRRD(AMP_INDEX_FOR_CHANNEL(ch), byte) = value;
}

void amc_phy_run_dq_training(uint8_t ch)
{
	ch = AMP_INDEX_FOR_CHANNEL(ch);					// cope with hole in AMP address space
	
	amc_phy_run_dll_update(ch);
	rAMP_DQTRNCMD(ch) = 0x1;					// execute DQ training sequence
	while ((rAMP_DQTRNCMD(ch) & 0x1) != 0) ;			// wait for training to be finished	
}

uint32_t amc_phy_get_dq_training_status(uint8_t ch, uint8_t byte, uint8_t num_bytes)
{
	uint8_t byte_result;
	uint32_t status;
	
	status = rAMP_DQTRNSTS(AMP_INDEX_FOR_CHANNEL(ch));
	byte_result = (status & (1 << (byte + 16)));

	return byte_result;
}

void amc_phy_configure_dll_pulse_mode(uint8_t ch, bool enable)
{
	ch = AMP_INDEX_FOR_CHANNEL(ch);					// cope with hole in AMP address space
	
	if (!enable) {
		rAMP_DLLUPDTCTRL(ch) &= ~((3 << 20) | (1 << 16));	// DLLUptMode = 0, DllUpdtPhyUpdtTyp = 0
	}
	else {
		rAMP_DLLUPDTCTRL(ch) = amp_dllupdtctrl[ch];
		amc_phy_run_dll_update(ch);
	}
}

void amc_phy_shift_dq_offset(int8_t *dq_offset, uint8_t num_bytes) {
	amc_dram_shift_dq_offset(dq_offset, num_bytes);
}
