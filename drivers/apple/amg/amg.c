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

#include <debug.h>
#include <drivers/amc/amc.h>
#include <drivers/amc/amc_phy.h>
#include <drivers/dram.h>
#include <lib/libc.h>
#include <platform/chipid.h>
#include <sys.h>

#include "amg.h"

#define CA_PIVOTS_LIST_SIZE		(7)
#define DQ_PIVOTS_LIST_SIZE		(5)

static const int ca_pivot_ps[CA_PIVOTS_LIST_SIZE] = {-375, -250, -125, 0, 125, 250, 375};
static const int dq_pivot_ps[DQ_PIVOTS_LIST_SIZE] = {-500, -250, 0, 250, 500};
static struct amc_phy_calibration_data amg_calibration_data =  {
	.bit_time_ps 		= 2500,
	.min_ca_window_width_ps = 250,
	.min_dq_window_width_ps = 250,
	.ca_pivot_step_size_ps 	= 125,
	.dq_pivot_step_size_ps 	= 250,
	.max_offset_setting	= 63,
	.min_offset_factor	= 12,
	.ca_pivots_list_size 	= CA_PIVOTS_LIST_SIZE,
	.ca_pivot_ps 		= (int *)ca_pivot_ps,
	.dq_pivots_list_size 	= DQ_PIVOTS_LIST_SIZE,
	.dq_pivot_ps 		= (int *)dq_pivot_ps,
};

void amc_phy_init(bool resume)
{
	int i;

	// overwrite bit_time_ps if presented
	if (amc_phy_params.bit_time_ps)
		amg_calibration_data.bit_time_ps = amc_phy_params.bit_time_ps;
	
	for (i = 0; i < AMC_NUM_CHANNELS; i++) {
		int ch = AMG_INDEX_FOR_CHANNEL(i);		// cope with hole in AMG address space

		rAMG_CORE_CONFIG(ch)  = amc_phy_params.core_config;

#if !SUPPORT_FPGA
		// PHY ZQ calibration
		rAMG_ZQ_CONF(ch)      = 0x00ff0096;		// <rdar://problem/8263127>
		rAMG_ZQ_CONTROL(ch)   = 0x00000009;		// <rdar://problem/7502673>
		rAMG_ZQ_CONTROL(ch)   = 0x0000000b;
		// Not testing the error bit.  This will never happen in normal operation,
		// and in fact _can't_ be set in H4P B0.
		while ((rAMG_ZQ_STATUS(ch) & 0x80) == 0) ;

		// Train master DLL
		rAMG_DLL_CONTROL(ch)  = 0x00110001;
		rAMG_DLL_CONTROL(ch)  = 0x00110005;
		while ((rAMG_DLL_STATUS(ch) & 1) != 1) ;
		// check for timeout
		if ((rAMG_DLL_STATUS(ch) & 0x4) != 0)
			for (;;) ;

		if (amc_phy_params.flags & FLAG_AMG_PARAM_CAL_STATIC) {
			// CA static offset programming; note it's too early to use
			// MRRs, so this relies on the SOC fusing.
			uint16_t fine_lock = (rAMG_DLL_STATUS(ch) >> 5) & 0x3ff;
			uint32_t fine_unit = (amg_calibration_data.bit_time_ps) / fine_lock;
			uint32_t addr_offset;
			uint32_t vendor_id = chipid_get_memory_manufacturer();
			switch (vendor_id) {
			case JEDEC_MANUF_ID_SAMSUNG:
				addr_offset = (110/fine_unit) | (1 << 6); /* -110ps */
				break;
			case JEDEC_MANUF_ID_ELPIDA:
				addr_offset = (50/fine_unit) | (1 << 6); /* -50ps */
				break;
			case JEDEC_MANUF_ID_HYNIX:
				addr_offset = 0; /* 0ps */
				break;
			default:
				addr_offset = 0;
				break;
			}
			rAMG_ADDR_OFFSET(ch) = addr_offset;			
		} else if (resume) {
			amc_phy_calibration_restore_ca_offset(i);
		}

		rAMG_DLL_TIMER(ch)    = 0x0003061a;

		// set frequency scaling parameters
		if (likely(!amc_phy_params.freq_scale))
			rAMG_FREQ_SCALE(ch)   = 0x00e40688;
		else 
			rAMG_FREQ_SCALE(ch)   = amc_phy_params.freq_scale;
		
#endif	// !SUPPORT_FPGA

		rAMG_READ_LATENCY(ch) = amc_phy_params.read_latency;	// [linked to AMC phyrdwrtim]
		rAMG_GATE_OFFSET(ch)  = amc_phy_params.gate_offset;

		rAMG_INIT_CONTROL(ch) = 0x00000001;
	}
}

void amc_phy_scale_dll(int freqsel, int factor)
{
	int i;

	// Make sure AMG was inited.
	// Assumptions: all supported channels are inited together, 
	// 		and channel 0 is always used.
	if ((rAMG_INIT_CONTROL(0) & 1) == 0)
		return;

	// Scale the current lock value
	for (i = 0; i < AMC_NUM_CHANNELS; i++) {
		int ch = AMG_INDEX_FOR_CHANNEL(i);
		uint16_t new_lock;
		uint16_t fine_lock; 

		// The fine_lock is 10 bits and coarse is 8.  Since the override
		// is in coarse units, we do shifty math on the fine units.
		fine_lock = (rAMG_DLL_STATUS(ch) >> 5) & 0x3ff;
		if (factor == 1)
			new_lock = fine_lock >> 2;
		else if (factor == 8)
			new_lock = fine_lock << 1;
		else
			panic("amc_phy_scale_dll: unsupported DLL scale factor");
		if (new_lock > 0xff)
			new_lock = 0xff;
		rAMG_DLL_OVERRIDE(ch) = new_lock;
		rAMG_DLL_CONTROL(ch) = 0x00110004;
		while (!(rAMG_DLL_STATUS(ch) & 1)) ;
	}
}

bool amc_phy_rddq_cal()
{
	uint32_t *dq_cal_data;
	bool result;
	uint8_t index, i;
	uint32_t device_width;
	uint8_t slot, ch;
	uint16_t fine_lock;
	uint32_t fine_unit;
	uint32_t offset;
	
	result = false;
	
	if (amc_phy_params.flags & FLAG_AMG_PARAM_CAL_STATIC) {
		dq_cal_data = malloc(AMC_NUM_CHANNELS * 2 * sizeof(uint32_t));
		
		if (chipid_get_memory_dqcal(dq_cal_data)) {
			index = 0;
			device_width = (amc_get_memory_device_info())->width;
		
			for (i = 0; i < AMC_NUM_CHANNELS; i++) {
				ch = AMG_INDEX_FOR_CHANNEL(i); // cope with hole in AMG address space
				fine_lock = (rAMG_DLL_STATUS(ch) >> 5) & 0x3ff;
				fine_unit = (amg_calibration_data.bit_time_ps) / fine_lock;
			
				// Loop across all bytes twice, first for read then for write,
				// which matches the order in which the data is given to us.
				for (slot = 0; slot < device_width * 2; slot++) {
					offset = (((dq_cal_data[index] & 0x8) << 3) |
					    	 ((((dq_cal_data[index] & 0x7) * 40) / fine_unit) & 0x3f));
					dq_cal_data[index] >>= 4;
					if (slot < device_width)
						rAMG_RD_OFFSET(ch, slot) = offset;
					else
						rAMG_WR_OFFSET(ch, (slot - device_width)) = offset;
					// Bump at end of "device_width" bytes of read and write
					if ((slot % device_width) == (device_width - 1))
						index++;
				}
			}
		
			result = true;
		}
		
		free(dq_cal_data);
	}
	
	return result;
}

void amc_phy_bypass_prep(int step)
{
	// Make sure AMG was inited.
	// Assumptions: all supported channels are inited together, 
	// 		and channel 0 is always used.
	if ((rAMG_INIT_CONTROL(0) & 1) == 0)
		return;

	// We need to be sure the DLL is in forced mode.  Reload the 
	// original full-speed lock result.  <rdar://problem/7269959>
	// Note the 0 is the unused freqsel argument.
	amc_phy_scale_dll(0, 1);
}

void amc_phy_finalize()
{
	int i;

	// Enable pulse-mode DLL training
	for (i = 0; i < AMC_NUM_CHANNELS; i++) {
		int ch = AMG_INDEX_FOR_CHANNEL(i); // cope with hole in AMG address space
		rAMG_DLL_FILTER(ch)  = 0x0000002e;
		rAMG_DLL_CONTROL(ch) = 0x00110006;
	}
}

void amc_phy_enable_dqs_pulldown(bool enable)
{
}

void amc_phy_reset_fifos()
{	
}

const struct amc_phy_calibration_data * amc_phy_get_calibration_data()
{
	return &amg_calibration_data;
}

void amc_phy_calibration_init(uint32_t *fine_lock_ps)
{
	uint8_t ch;
	uint16_t lock_result;
	const struct amc_memory_device_info *dev_info;
	
	ASSERT(fine_lock_ps != NULL);
	
	*fine_lock_ps = 0;
	dev_info = amc_get_memory_device_info();
	
	for (ch = 0; ch < AMC_NUM_CHANNELS; ch++) {
		rAMG_DLL_FILTER(AMG_INDEX_FOR_CHANNEL(ch)) = 0;
		
		lock_result = ((rAMG_DLL_STATUS(AMG_INDEX_FOR_CHANNEL(ch)) >> 5) & 0x3ff);	// fine value
		rAMG_DLL_OVERRIDE(AMG_INDEX_FOR_CHANNEL(ch)) = lock_result / 4;			// coarse value
		amc_phy_run_dll_update(ch);
		
		*fine_lock_ps += (amg_calibration_data.bit_time_ps) / (lock_result);
		
		// <rdar://problem/10996362> Modified guidance for AMG bit_sample field setting
		if (((dev_info->vendor_id == JEDEC_MANUF_ID_SAMSUNG) && (dev_info->density == JEDEC_DENSITY_2Gb) && (dev_info->rev_id == 0)) ||
			((dev_info->vendor_id == JEDEC_MANUF_ID_SAMSUNG) && (dev_info->density == JEDEC_DENSITY_4Gb) && (dev_info->rev_id == 1)) ||
			((dev_info->vendor_id == JEDEC_MANUF_ID_HYNIX) && (dev_info->density == JEDEC_DENSITY_1Gb)))
			rAMG_DQCAL_CONTROL(AMG_INDEX_FOR_CHANNEL(ch)) = 0x00001ffb;
		else
			rAMG_DQCAL_CONTROL(AMG_INDEX_FOR_CHANNEL(ch)) = 0x00003ffb;
	}

	*fine_lock_ps = *fine_lock_ps / AMC_NUM_CHANNELS;
}

void amc_phy_run_dll_update(uint8_t ch)
{
	uint8_t ch_amg_idx = AMG_INDEX_FOR_CHANNEL(ch);
	
	rAMG_DLL_CONTROL(ch_amg_idx) = 0x00110000 | (1 << 2) | (0 << 0);
	while ((rAMG_DLL_STATUS(ch_amg_idx) & 1) == 0) ;	
}

void amc_phy_set_addr_offset(uint8_t ch, uint8_t value)
{
	rAMG_ADDR_OFFSET(AMG_INDEX_FOR_CHANNEL(ch)) = value;
}

void amc_phy_set_rd_offset(uint8_t ch, uint8_t byte, uint8_t value)
{
	rAMG_RD_OFFSET(AMG_INDEX_FOR_CHANNEL(ch), byte) = value;
}

void amc_phy_set_wr_offset(uint8_t ch, uint8_t byte, uint8_t value)
{
	rAMG_WR_OFFSET(AMG_INDEX_FOR_CHANNEL(ch), byte) = value;
}

void amc_phy_configure_dll_pulse_mode(uint8_t ch, bool enable)
{
	if (!enable) {
		rAMG_DLL_FILTER(AMG_INDEX_FOR_CHANNEL(ch)) = 0x0000000a;
	}
	else {
		rAMG_DLL_FILTER(AMG_INDEX_FOR_CHANNEL(ch)) = 0x0000002e;
		rAMG_DLL_CONTROL(AMG_INDEX_FOR_CHANNEL(ch)) = 0x00110006;
	}
}

void amc_phy_run_dq_training(uint8_t ch)
{
	uint8_t ch_amg_idx = AMG_INDEX_FOR_CHANNEL(ch);
	
	rAMG_DQCAL_CONTROL(ch_amg_idx) |= (1 << 14); 		// start
	while ((rAMG_DQCAL_STATUS(ch_amg_idx) & 1) == 0) ; 	// wait for 'Done' bit to set
}

uint32_t amc_phy_get_dq_training_status(uint8_t ch, uint8_t byte, uint8_t num_bytes)
{
	uint8_t byte_result;
	uint32_t status;
	
	status = rAMG_DQCAL_STATUS(AMG_INDEX_FOR_CHANNEL(ch));
	
	// bits[MAX_BYTE-1:0] of the byte_result field are for pos DQS strobe,
	// and bits [2*MAX_BYTE-1:MAX_BYTE] for neg DQS strobe. Final result is 
	// 'AND' of two, for specified byte
	byte_result = (status & (1 << (byte + 1)));
	byte_result = byte_result && (status & (1 << (byte + num_bytes + 1)));

	return byte_result;
}

void amc_phy_shift_dq_offset(int8_t *dq_offset, uint8_t num_bytes) {
	amc_dram_shift_dq_offset(dq_offset, num_bytes);
}
