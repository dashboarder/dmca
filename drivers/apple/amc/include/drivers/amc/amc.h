/*
 * Copyright (C) 2009-2013 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */
#ifndef __AMC_H
#define __AMC_H

#include <sys/types.h>

/* Apple Memory Controller */

#define AMC_MAX_FREQUENCY_SLOTS		4
#define AMC_MAX_TUNABLES		50

#define FLAG_AMC_PARAM_BUSTAT23		0x0001
#define FLAG_AMC_PARAM_LEGACY		0x0002
#define FLAG_AMC_PARAM_SLOW_BOOT	0x0004
#define FLAG_AMC_PARAM_ENABLE_AIU	0x0008
#define FLAG_AMC_PARAM_RECONFIG		0x0010
#define FLAG_AMC_PARAM_MRR_BYTE_SWIZZLE	0x0020
#define FLAG_AMC_PARAM_MRR_BIT_SWIZZLE  0x0040
#define FLAG_AMC_PARAM_ZQCL		0x0080

#define AMP_SWIZZLE_PER_J34M	2	// ...and This is the second Swizzle ever needed for H6
					// per <rdar://15498696>

#define __abs(x) 			(((x) < 0) ? -(x) : (x))

typedef enum {
	MR_READ,
	MR_WRITE
} amc_mrcmd_op_t;

struct amc_per_freq {
	uint32_t	cas;
	uint32_t	pch;
	uint32_t	act;
	uint32_t	autoref;
	uint32_t	selfref;
	uint32_t	modereg;
	uint32_t	mifcassch;
	uint32_t	mifqmaxctrl;
	uint32_t	arefparam;
	uint32_t	trefbwbasecyc;	
};

struct amc_param {
	uint32_t		flags;
	uint32_t		freqsel;
	uint32_t		tREFi;
	uint32_t		tREFi_1Gb;
	uint32_t		longsrcnt;
	uint32_t		longsrcnt_1Gb;
	uint32_t		srextrarefcnt;
	uint32_t		rdlat;
	uint32_t		wrlat;
	uint32_t		phyrdlat;
	uint32_t		phywrlat;
	uint32_t		pdn;
	uint32_t		read;
	uint32_t		bustat;
	uint32_t		bustat2;
	uint32_t		derate;
	uint32_t		mr1;
	uint32_t		mr2;
	uint32_t		mr3;
	uint32_t		mr63;
	uint32_t		mcphyupdate;	
	uint32_t		autoref_params;
	uint32_t		pwrmngtparam_small;
	uint32_t		pwrmngtparam_guided;
	uint32_t		arefparam;
	uint32_t		chnldec;
	uint32_t		chnldec2;
	uint32_t		chnldec4;
	uint32_t		aref_freq0;
	uint32_t		aref_freq1;
	uint32_t		bootclkdivsr;
	uint32_t		schen_default;
	uint32_t		pwrmngten_default;
	uint32_t		mcphyupdate1;
	uint32_t		odts;
	uint32_t		readleveling;
	uint32_t		lat;
	uint32_t		phyrdwrtim;
	uint32_t		longsr;
	uint32_t		addrcfg;
	uint32_t		mccchnldec;
	uint32_t		mcuchnhash;
	uint32_t		mcuchnhash2;
	uint32_t		mcschnldec;
	uint32_t		qbrparam;
	uint32_t		addrmapmode;
	uint32_t		bankhash0;
	uint32_t		bankhash1;
	uint32_t		bankhash2;
	int8_t			offset_shift_hynix;
	int8_t			offset_shift_elpida;
	int8_t			offset_shift_elpida25nm;
	struct amc_per_freq 	freq[AMC_MAX_FREQUENCY_SLOTS];
};

struct amc_tunable {
	volatile uint32_t	*reg;
	uint32_t		value;
};

struct amc_memory_device_info {
	uint32_t	vendor_id;
	uint32_t	rev_id;
	uint32_t	rev_id2;
	uint32_t	type;
	uint32_t	width;		// Device width in Bytes
	uint32_t	density;	// Device density in MBytes
};


// AMC protected functions (meant to be called from per platform/target amc implementation)
int32_t amc_init(bool resume);
uint64_t amc_get_memory_size(void);
const struct amc_memory_device_info *amc_get_memory_device_info(void);
void amc_mrcmd(amc_mrcmd_op_t op, uint8_t channels, uint8_t ranks, int32_t reg, uintptr_t val);
void amc_mrcmd_to_ch_rnk(amc_mrcmd_op_t op, uint8_t channel, uint8_t rank, int32_t reg, uintptr_t val);
void amc_enable_autorefresh(void);
void amc_finalize(bool resume);
void amc_dram_workarounds(bool resume);
void amc_dram_shift_dq_offset(int8_t *dq_offset, uint8_t num_bytes);
void amc_enable_slow_boot(bool enable);
void amc_configure_address_decoding_and_mapping(void);
void amc_configure_default_address_decoding_and_mapping(void);
addr_t amc_get_uncached_dram_virt_addr(uint32_t ch, uint32_t rnk, uint32_t bank, uint32_t row, uint32_t col);
uint32_t amc_get_consecutive_bytes_perchnrnk(void);
const struct amc_param *amc_get_params(void);

// AMC function to be called from AMC PHY calibration code
void amc_calibration_start(bool start);
void amc_enable_rddqcal(bool enable);
void amc_wrdqcal_start(bool start);

#endif /* ! __AMC_H */
