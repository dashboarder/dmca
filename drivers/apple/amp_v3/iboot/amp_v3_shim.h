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
#ifndef _AMP_T7000_SHIM_H
#define _AMP_T7000_SHIM_H

#include <debug.h>
#include <sys/menu.h>
#include <drivers/amc/amc.h>
#include <drivers/amc/amc_phy.h>
#include <drivers/dram.h>
#include <drivers/miu.h>
#include <drivers/power.h>
#include <platform.h>
#include <platform/chipid.h>
#include <sys.h>

#include <platform/soc/hwregbase.h>
#include "amp_v3.h"
#include "amp_v3_calibration.h"

#define AMP_MAX_RD			(2)	// 4 bytes are split across 2 DQ PHYs with 2 bytes each
#define AMP_MAX_DQ			(8)

#define shim_panic(...)		do { calibration_dump_results(SELECT_CAL_ALL, true); panic(__VA_ARGS__); } while(0)
#define shim_printf(...)		dprintf(DEBUG_CRITICAL, __VA_ARGS__)
#define CSR_READ(x)			*(volatile u_int32_t *) x
#define CSR_WRITE(x, y)		*(volatile u_int32_t *) x = y

// <rdar://problem/13899661>
// "word" for DV requires +1, so for iBoot, should subtract 1 before adding to mem_base
#define MEM_WRITE(mem_base, data, word)	*(uint32_t *)(mem_base + ((word-1) << 2)) = data
#define MEM_READ(mem_base, word)		*(uint32_t *)(mem_base + ((word-1) << 2))

void shim_init_calibration_params(struct amp_calibration_params *cfg_params);
void shim_configure_pre_ca(void);
void shim_enable_rddqcal(bool enable);
void shim_configure_post_wrlvl(struct amp_calibration_params *cfg_params);
void shim_configure_pre_wrdq(bool resume);
void shim_configure_post_prbs_rddq(struct amp_calibration_params *cfg_params);
void shim_mrcmd_to_ch_rnk(uint8_t rw, uint8_t channel, uint8_t rank, int32_t reg, uintptr_t val);
uint64_t shim_compute_dram_addr(uint32_t ch, uint32_t rnk, uint32_t bank, uint32_t row, uint32_t col);
uint32_t shim_get_consecutive_bytes_perchnrnk(void);
void shim_store_memory_calibration(void *cal_values, uint32_t cal_size);
void shim_load_memory_calibration(void *cal_values, uint32_t cal_size);

struct amp_per_freq {
	uint32_t caoutdllscl;
	uint32_t dqsindllscl;
	uint32_t rdcapcfg;
};

struct amp_params {
	struct amp_per_freq freq[AMP_FREQUENCY_SLOTS];
	uint32_t dqdqsds;
	uint32_t nondqds;
	uint32_t ampclk;
};

#include <platform/ampconfig.h>

#endif /* _AMP_T7000_SHIM_H */
