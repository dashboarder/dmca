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

#include <drivers/amc/amc.h>
#include <drivers/amc/amc_phy.h>
#include <drivers/dram.h>
#include <drivers/miu.h>
#include <drivers/power.h>
#include <lib/libc.h>
#include <platform/memmap.h>
#include <platform/soc/hwregbase.h>
#include <sys.h>

// Constants across phys
#define NUM_BYTES			(AMC_CHANNEL_WIDTH)	// channel width in words
#define RD_DQ_RETEST_COUNT		(1)
#define WR_DQ_RETEST_COUNT		(1)
#define BINARY_SEARCH_THRESHOLD		(2)
#define INCR_SEARCH_STEP_SIZE		(4)
#define WRITE_REGION_SIZE		(64)			// in words
#define CA_PIVOTS_LIST_MAX_SIZE		(9)
#define DQ_PIVOTS_LIST_MAX_SIZE		(5)
#define CA_PIVOT_THRESHOLD			(2)

// Phy specific
#define CA_PIVOTS_LIST_SIZE		(calibration_data->ca_pivots_list_size)
#define DQ_PIVOTS_LIST_SIZE		(calibration_data->dq_pivots_list_size)
#define BIT_TIME_PS			(calibration_data->bit_time_ps)
#define MIN_CA_WINDOW_WIDTH_PS		(calibration_data->min_ca_window_width_ps)
#define MIN_DQ_WINDOW_WIDTH_PS		(calibration_data->min_dq_window_width_ps)
#define CA_PIVOT_STEP_SIZE_PS		(calibration_data->ca_pivot_step_size_ps)
#define DQ_PIVOT_STEP_SIZE_PS		(calibration_data->dq_pivot_step_size_ps)
#define MAX_OFFSET_SETTING		(calibration_data->max_offset_setting)
#define MIN_OFFSET_FACTOR		(calibration_data->min_offset_factor)
#define DLL_F2_DELAY_FS		(calibration_data->dll_f2_delay_fs)

// Macros
#define RD_WR_ADDR_OFFSET(offset) 	(((((offset) < 0) ? 1 : 0) << 6) | __abs((offset)))

// Local variables
static int calibration_done;
static const struct amc_phy_calibration_data *calibration_data;

static const int *ca_pivot_ps;
static const int *dq_pivot_ps;
static int ca_pivot_fs[CA_PIVOTS_LIST_MAX_SIZE] = { -1 };
static int dq_pivot_fs[DQ_PIVOTS_LIST_MAX_SIZE] = { -1 };
static int min_offset_fs = -1;
static int max_offset_fs = -1;
static int dq_pivot_step_size_fs = -1;
static int min_dq_window_width_fs = -1;
static uint32_t save_restore_region_content[WRITE_REGION_SIZE];

// Local functions
static void calibration_ca_rddq_cal_init();
static void calibration_ca_rddq_cal_finalize();
static int calibration_find_min_element(int8_t *array, uint8_t array_len);
static int calibration_binary_search_offset(uint8_t ch, uint8_t byte, int8_t sweep_lo, int8_t sweep_hi);
static int calibration_check_rddq(uint8_t ch, uint8_t byte);
static void calibration_rddq_cal(uint8_t ch, int8_t *rd_dq_offset, int8_t *rd_dq_window);
static int8_t calibration_ca_rddq_cal(uint8_t ch);
static void calibration_wrdq_cal(uint8_t ch, int8_t *wr_dq_offset, int8_t *wr_dq_window);
static void calibration_get_save_restore_region(uint8_t ch, uint32_t **region);

// External functions
extern void amc_scheduler_en_workaround(bool init);


///////////////////////////////////////////////////////////////////////////////
////// Global functions
///////////////////////////////////////////////////////////////////////////////

void amc_phy_calibration_restore_ca_offset(uint8_t ch)
{
	// restore CA offsets from PMU
	int8_t ca_offset;

	if (power_get_nvram(kPowerNVRAMiBootMemCalCAOffset0Key + ch, (uint8_t *)&ca_offset) == -1)
		for(;;) ;
	
	amc_phy_set_addr_offset(ch, RD_WR_ADDR_OFFSET(ca_offset));
}

void amc_phy_calibration_ca_rddq_cal(bool resume)
{
	uint8_t ch;
	int8_t ca_offset;
	
	if (amc_phy_rddq_cal() == true) {
		calibration_done = 1;		
		return;
	}
	
	calibration_data = amc_phy_get_calibration_data();
	ca_pivot_ps = calibration_data->ca_pivot_ps;
	dq_pivot_ps = calibration_data->dq_pivot_ps;
	
	// force resume boot flow if indicated by phy data (for AMP)
	if (calibration_data->use_resume_boot_flow)
		resume = true;
	
	calibration_ca_rddq_cal_init();
	
	if (!resume) {
		for (ch = 0; ch < AMC_NUM_CHANNELS; ch++) {
			// find offset, and save address offset to PMU
			ca_offset = calibration_ca_rddq_cal(ch);
			if (power_set_nvram(kPowerNVRAMiBootMemCalCAOffset0Key + ch, ca_offset) != 0)
				for (;;) ;
		}
	}
	else {
		int8_t rd_dq_offset[NUM_BYTES];
		int8_t rd_dq_window[NUM_BYTES];
		uint8_t byte;
		
		for (ch = 0; ch < AMC_NUM_CHANNELS; ch++) {
			calibration_rddq_cal(ch, rd_dq_offset, rd_dq_window);

			// shift it, if needed
			amc_phy_shift_dq_offset(rd_dq_offset, NUM_BYTES);
			
			for (byte = 0; byte < NUM_BYTES; byte++)
				amc_phy_set_rd_offset(ch, byte, RD_WR_ADDR_OFFSET(rd_dq_offset[byte]));
			
			amc_phy_run_dll_update(ch);
		}
	}
	
	calibration_ca_rddq_cal_finalize();	
}

void amc_phy_calibration_wrdq_cal(bool resume)
{
	uint8_t ch;
	uint32_t saved_addr_map_mode, chnselhibits;
	const struct amc_memory_device_info *dev_info;
	
	if (calibration_done)
		return;
	
	saved_addr_map_mode = 0;

	// disable pulse-mode dll operation
	for (ch = 0; ch < AMC_NUM_CHANNELS; ch++)
		amc_phy_configure_dll_pulse_mode(ch, false);
	
	// save aiucfg.aiuchnldec
	saved_addr_map_mode = *(volatile uint32_t *)(AMC_BASE_ADDR + 0x0A4);
	
	dev_info = amc_get_memory_device_info();
	if (dev_info->density == JEDEC_DENSITY_4Gb)
		chnselhibits = 5;
	else if (dev_info->density == JEDEC_DENSITY_2Gb)
		chnselhibits = 4;
	else
		chnselhibits = 3;
	
	// aiucfg.aiuchnldec = (DENSITY = 2Gb) ? 0x0004_0101) : 0x0003_0101
	*(volatile uint32_t *)(AMC_BASE_ADDR + 0x0A4) = ((chnselhibits << 16) | (1 << 8) | (1 << 0));
	
	for (ch = 0; ch < AMC_NUM_CHANNELS; ch++) {
		int8_t wr_dq_window[NUM_BYTES];
		int8_t wr_dq_offset[NUM_BYTES];
		uint8_t byte;
		
		if (resume) {
			uint32_t word;
			uint32_t *region;

			memset(save_restore_region_content, 0, sizeof(save_restore_region_content));
			
			calibration_get_save_restore_region(ch, &region);

			for(word = 0; word < WRITE_REGION_SIZE; word++)
				save_restore_region_content[word] = *(region + word);
		}

		memset(wr_dq_window, 0, sizeof(wr_dq_window));
		memset(wr_dq_offset, 0, sizeof(wr_dq_offset));

		calibration_wrdq_cal(ch, wr_dq_offset, wr_dq_window);
		
		// shift it, if needed
		amc_phy_shift_dq_offset(wr_dq_offset, NUM_BYTES);
		
		for (byte = 0; byte < NUM_BYTES; byte++) 
			amc_phy_set_wr_offset(ch, byte, RD_WR_ADDR_OFFSET(wr_dq_offset[byte]));

		amc_phy_run_dll_update(ch);
		
		if (resume) {
			uint32_t word;
			uint32_t *region;

			calibration_get_save_restore_region(ch, &region);

			for(word = 0; word < WRITE_REGION_SIZE; word++)
				*(region + word) = save_restore_region_content[word];
		}
	}
	
	// restore aiucfg.aiuchnldec
	*(volatile uint32_t *)(AMC_BASE_ADDR + 0x0A4) = saved_addr_map_mode;	
	
	// re-enable pulse mode
	for (ch = 0; ch < AMC_NUM_CHANNELS; ch++)
		amc_phy_configure_dll_pulse_mode(ch, true);

	calibration_done = 1;
}

///////////////////////////////////////////////////////////////////////////////
////// Local functions
///////////////////////////////////////////////////////////////////////////////

static void calibration_ca_rddq_cal_init()
{
	uint8_t i;
	uint32_t fine_lock_ps;
		
	amc_phy_calibration_init(&fine_lock_ps);

	// workaround for AMC scheduler enable issue
	amc_scheduler_en_workaround(true);

	max_offset_fs = __min((int)MAX_OFFSET_SETTING, ((BIT_TIME_PS / 4) / (int)fine_lock_ps));
	min_offset_fs = __max(-((int)MAX_OFFSET_SETTING), (-((BIT_TIME_PS / 4) / (int)fine_lock_ps) + MIN_OFFSET_FACTOR + DLL_F2_DELAY_FS));
	
	for (i = 0; i < CA_PIVOTS_LIST_SIZE; i++) {
		if (ca_pivot_ps[i] > 0)
			ca_pivot_fs[i] = __min((ca_pivot_ps[i] / (int)fine_lock_ps), 
								   ((BIT_TIME_PS / 4) / (int)fine_lock_ps));
		else
			ca_pivot_fs[i] = __max((ca_pivot_ps[i] / (int)fine_lock_ps), 
								   (-((BIT_TIME_PS / 4) / (int)fine_lock_ps) + MIN_OFFSET_FACTOR + DLL_F2_DELAY_FS));
	}

	for (i = 0; i < DQ_PIVOTS_LIST_SIZE; i++) {
		if (dq_pivot_ps[i] > 0)
			dq_pivot_fs[i] = __min((dq_pivot_ps[i] / (int)fine_lock_ps), 
								   ((BIT_TIME_PS / 4) / (int)fine_lock_ps));		
		else 
			dq_pivot_fs[i] = __max((dq_pivot_ps[i] / (int)fine_lock_ps), 
							   (-((BIT_TIME_PS / 4) / (int)fine_lock_ps) + MIN_OFFSET_FACTOR));
	}
	
	dq_pivot_step_size_fs = DQ_PIVOT_STEP_SIZE_PS / (int)fine_lock_ps;
	min_dq_window_width_fs = MIN_DQ_WINDOW_WIDTH_PS / (int)fine_lock_ps;	
}

static void calibration_ca_rddq_cal_finalize()
{
	// workaround for AMC scheduler enable issue
	amc_scheduler_en_workaround(false);
}

static int calibration_find_min_element(int8_t *array, uint8_t array_len)
{
	uint8_t i;
	int8_t min;
	
	min = array[0];
	for (i = 1; i < array_len; i++) {
		if (array[i] < min)
			min = array[i];
	}
	
	return (min);
}

static int calibration_check_rddq(uint8_t ch, uint8_t byte)
{
	uint8_t byte_result;
	uint8_t i;

	for (i = 0; i < RD_DQ_RETEST_COUNT; i++) {
		amc_phy_run_dq_training(ch);
		
		if ((byte_result = amc_phy_get_dq_training_status(ch, byte, NUM_BYTES)) == 0)
			break;
	}
		
	// '1' pass, '0' fail
	return (byte_result);
}

static int calibration_binary_search_offset(uint8_t ch, uint8_t byte, int8_t sweep_lo, int8_t sweep_hi)
{
	int8_t mid;

	amc_phy_set_rd_offset(ch, byte, RD_WR_ADDR_OFFSET(sweep_hi));
	amc_phy_run_dll_update(ch);
	if (calibration_check_rddq(ch, byte) != 0)
		return sweep_hi;
	
	do {
		mid = (sweep_lo + sweep_hi) / 2;
		
		amc_phy_set_rd_offset(ch, byte, RD_WR_ADDR_OFFSET(mid));
		amc_phy_run_dll_update(ch);
		if (calibration_check_rddq(ch, byte) != 0)
			sweep_lo = mid;
		else 
			sweep_hi = mid;
			
	} while (__abs(sweep_hi - sweep_lo) > BINARY_SEARCH_THRESHOLD);

	return sweep_lo;
}

static void calibration_rddq_cal(uint8_t ch, int8_t *rd_dq_offset, int8_t *rd_dq_window)
{
	int8_t min_pivot[NUM_BYTES];
	int8_t max_pivot[NUM_BYTES];
	int8_t pivot_count[NUM_BYTES];
	uint8_t i, pivot, byte;
	int8_t pos_sweep_hi, pos_sweep_lo, neg_sweep_hi, neg_sweep_lo;
	int8_t pos_offset, neg_offset;
	
	for (i = 0; i < NUM_BYTES; i++) {
		min_pivot[i] = max_offset_fs;
		max_pivot[i] = min_offset_fs;
		pivot_count[i] = 0;
		rd_dq_offset[i] = 0;
		rd_dq_window[i] = 0;
	}
	
	for (pivot = 0; pivot < DQ_PIVOTS_LIST_SIZE; pivot++) {
		for (byte = 0; byte < NUM_BYTES; byte++)
			amc_phy_set_rd_offset(ch, byte, RD_WR_ADDR_OFFSET(dq_pivot_fs[pivot]));
		amc_phy_run_dll_update(ch);

		for (byte = 0; byte < NUM_BYTES; byte++) {
			if (0 != calibration_check_rddq(ch, byte)) {
				if (dq_pivot_fs[pivot] < min_pivot[byte])
					min_pivot[byte] = dq_pivot_fs[pivot];
				if (dq_pivot_fs[pivot] > max_pivot[byte])
					max_pivot[byte] = dq_pivot_fs[pivot];
				
				pivot_count[byte]++;
			}
		}
	}

	if (calibration_find_min_element(pivot_count, NUM_BYTES) < 
						(MIN_DQ_WINDOW_WIDTH_PS / DQ_PIVOT_STEP_SIZE_PS))
		return;

	for (byte = 0; byte < NUM_BYTES; byte++) {
		pos_sweep_hi = __min(max_offset_fs, (max_pivot[byte] + dq_pivot_step_size_fs));
		neg_sweep_hi = __max(min_offset_fs, (min_pivot[byte] - dq_pivot_step_size_fs));
		
		pos_sweep_lo = (pos_sweep_hi + neg_sweep_hi) / 2;
		neg_sweep_lo = pos_sweep_lo;
		
		pos_offset = calibration_binary_search_offset(ch, byte, pos_sweep_lo, pos_sweep_hi);
		neg_offset = calibration_binary_search_offset(ch, byte, neg_sweep_lo, neg_sweep_hi);
		
		rd_dq_offset[byte] = (pos_offset + neg_offset) / 2;
		rd_dq_window[byte] = __abs(pos_offset - neg_offset);
	}
}

static int8_t calibration_ca_rddq_cal(uint8_t ch)
{
	bool prev_ca_status; // 0: fail, 1: pass
	int8_t ca_pass_count, ca_offset;
	int8_t ca_weighted_offset[CA_PIVOTS_LIST_SIZE][2];
	int8_t saved_rd_dq_offset[NUM_BYTES];
	int8_t saved_rd_dq_window[NUM_BYTES];
	int8_t rd_dq_offset[NUM_BYTES];
	int8_t rd_dq_window[NUM_BYTES];
	uint8_t i, byte;
	int8_t min_rd_dq_window;
	int32_t sum;

	prev_ca_status = false;
	ca_pass_count = 0;
	memset(saved_rd_dq_offset, 0, sizeof(saved_rd_dq_offset));
	memset(saved_rd_dq_window, 0, sizeof(saved_rd_dq_window));
	
	for (i = 0; i < CA_PIVOTS_LIST_SIZE; i++) {

		//if the saturation point and the next CA pivot is closer than 2 fine-steps,
		// jump to the next CA pivot
		if (i > 0 && ( (ca_pivot_fs[i] - ca_pivot_fs[i-1]) < CA_PIVOT_THRESHOLD ) )
			continue;
		
		amc_phy_set_addr_offset(ch, RD_WR_ADDR_OFFSET(ca_pivot_fs[i]));
		amc_phy_run_dll_update(ch);
		
		calibration_rddq_cal(ch, rd_dq_offset, rd_dq_window);
		
		min_rd_dq_window = calibration_find_min_element(rd_dq_window, NUM_BYTES);
		
		amc_phy_reset_fifos();
		
		if (min_rd_dq_window < min_dq_window_width_fs) {
			// XXX soft reset?
			
			if (prev_ca_status == true)
				break;
				
			continue;	
		}
		
		prev_ca_status = true;
		
		ca_weighted_offset[ca_pass_count][0] = ca_pivot_fs[i];
		ca_weighted_offset[ca_pass_count][1] = min_rd_dq_window;
		ca_pass_count++;	

		for (byte = 0; byte < NUM_BYTES; byte++) {
			if (rd_dq_window[byte] > saved_rd_dq_window[byte]) {
				saved_rd_dq_window[byte] = rd_dq_window[byte];
				saved_rd_dq_offset[byte] = rd_dq_offset[byte];
			}
		}
	}

	// if ca window too small, spin
	if (ca_pass_count < (MIN_CA_WINDOW_WIDTH_PS / CA_PIVOT_STEP_SIZE_PS))
		for(;;) ;
	
	sum = 0;
	for (i = 0; i < ca_pass_count; i++)
		sum += ca_weighted_offset[i][1];
	
	ca_offset = 0;
	for (i = 0; i < ca_pass_count; i++)
		ca_offset += (ca_weighted_offset[i][0] * ca_weighted_offset[i][1] / sum);
		
	amc_phy_set_addr_offset(ch, RD_WR_ADDR_OFFSET(ca_offset));
	
	// Shift the rd_dq offset, if needed
	amc_phy_shift_dq_offset(saved_rd_dq_offset, NUM_BYTES);
    
	for(byte = 0; byte < NUM_BYTES; byte++)
		amc_phy_set_rd_offset(ch, byte, RD_WR_ADDR_OFFSET(saved_rd_dq_offset[byte]));
	
	amc_phy_run_dll_update(ch);
	
	return (ca_offset);
}

static void calibration_get_save_restore_region(uint8_t channel, uint32_t **region)
{
	uint32_t device_size;	// in bytes
	
	device_size = (amc_get_memory_size() / AMC_NUM_CHANNELS) * 1024 * 1024;
	*region = (uint32_t *)(SDRAM_BASE + ((channel + 1) * device_size)) - WRITE_REGION_SIZE;
}

static uint8_t calibration_check_wrdq(uint8_t channel)
{
	uint8_t i, byte, word;
	static const uint32_t patterns[] = { 
						0x00000000, 0xffffffff, 0x00000000, 0xffffffff, 
						0x00000000, 0x00000000, 0xffffffff, 0xffffffff
					};
	uint32_t *save_restore_region;
	uint8_t result;
	uint32_t temp;
	uint32_t num_patterns;

	result = 0;
	num_patterns = sizeof(patterns) / sizeof(patterns[0]);
	calibration_get_save_restore_region(channel, &save_restore_region);

	for (i = 0; i < WR_DQ_RETEST_COUNT; i++) {
		for (word = 0; word < WRITE_REGION_SIZE; word++) {
			*(save_restore_region + word) = patterns[word % num_patterns];
		}
		
		for (word = 0; word < WRITE_REGION_SIZE; word++)
			if ((temp = (*(save_restore_region + word) ^ patterns[word % num_patterns])) != 0) {
				for (byte = 0; byte < NUM_BYTES; byte++)
					if (temp & (0xf << byte))
						result |= (1 << byte);
				break;
			}
	}
	
	return result;
}

static void calibration_incr_search(uint8_t ch, int8_t sweep_lo, int8_t sweep_hi, int8_t step, int8_t *passing_offsets)
{
	uint8_t fail_flag, result, mask;
	uint8_t byte;
	int8_t sweep;

	fail_flag = 0;

	for (sweep = sweep_lo; __abs(sweep + step) <= __abs(sweep_hi); sweep += step)
	{
		for (byte = 0; byte < NUM_BYTES; byte++)
			amc_phy_set_wr_offset(ch, byte, RD_WR_ADDR_OFFSET(sweep));
		amc_phy_run_dll_update(ch);
		
		result = calibration_check_wrdq(ch);
		
		for (byte = 0; byte < NUM_BYTES; byte++) {
			mask = (1 << byte);
			
			if (fail_flag & mask)
				continue;
				
			if ((result & mask) == 0)
				passing_offsets[byte] = sweep;
			else
				fail_flag |= mask;
		}
		
		// all failed
		if (fail_flag & 0xf)
			break;
	}
}

static void calibration_wrdq_cal(uint8_t ch, int8_t *wr_dq_offset, int8_t *wr_dq_window)
{
	uint8_t byte;
	int8_t min_pivot, max_pivot;
	uint32_t pivot_count;
	int8_t pivot;
	int8_t pos_sweep_hi, pos_sweep_lo, neg_sweep_hi, neg_sweep_lo;
	int8_t pos_offset[NUM_BYTES], neg_offset[NUM_BYTES];
	
	min_pivot = max_offset_fs;
	max_pivot = min_offset_fs;
	pivot_count = 0;
	memset(pos_offset, 0, sizeof(pos_offset));
	memset(neg_offset, 0, sizeof(neg_offset));
	
	for (pivot = 0; pivot < DQ_PIVOTS_LIST_SIZE; pivot++) {
		for (byte = 0; byte < NUM_BYTES; byte++) 
			amc_phy_set_wr_offset(ch, byte, RD_WR_ADDR_OFFSET(dq_pivot_fs[pivot]));
		amc_phy_run_dll_update(ch);
		
		if (calibration_check_wrdq(ch) == 0) {
			if (dq_pivot_fs[pivot] < min_pivot)
				min_pivot = dq_pivot_fs[pivot];
			if (dq_pivot_fs[pivot] > max_pivot)
				max_pivot = dq_pivot_fs[pivot];
			pivot_count++;
		}
	}
	
	//if wr dq window too small, spin
	if (pivot_count < (uint32_t)(MIN_DQ_WINDOW_WIDTH_PS / DQ_PIVOT_STEP_SIZE_PS))
		for(;;) ;
		
	pos_sweep_hi = __min(max_offset_fs, (max_pivot + dq_pivot_step_size_fs));
	neg_sweep_hi = __max(min_offset_fs, (min_pivot - dq_pivot_step_size_fs));
	
	pos_sweep_lo = (pos_sweep_hi + neg_sweep_hi) / 2;
	neg_sweep_lo = pos_sweep_lo;
	
	calibration_incr_search(ch, pos_sweep_lo, pos_sweep_hi, INCR_SEARCH_STEP_SIZE, pos_offset);
	calibration_incr_search(ch, neg_sweep_lo, neg_sweep_hi, -INCR_SEARCH_STEP_SIZE, neg_offset);
	
	for (byte = 0; byte < NUM_BYTES; byte++) {
		wr_dq_offset[byte] = (pos_offset[byte] + neg_offset[byte]) / 2;
		wr_dq_window[byte] = __abs(pos_offset[byte] - neg_offset[byte]);
	}	
}
