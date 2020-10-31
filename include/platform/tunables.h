/*
 * Copyright (C) 2015 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */

#ifndef __PLATFORM_SOC_TUNABLES_H
#define __PLATFORM_SOC_TUNABLES_H

#include <sys/types.h>

struct __attribute__((packed)) tunable_struct {
	int32_t offset:27;
	int32_t size:5;
	uint32_t mask;
	uint32_t value;
};

struct __attribute__((packed)) tunable_struct64 {
	int32_t offset:27;
	int32_t size:5;
	uint64_t mask;
	uint64_t value;
};

struct tunable_struct_unpacked {
	int32_t offset;
        int32_t size;
        uint64_t mask;
        uint64_t value;
};

struct tunable_chip_struct {
	uint32_t chip_rev;
	uint64_t base_address;
	const struct tunable_struct *tunable;
	const struct tunable_struct64 *tunable64;
	bool reconfig;
};

struct tunable_filtered_chip_struct {
	uint32_t chip_rev;
	uint32_t starting_address;
	uint32_t ending_address;
	bool cold_boot;
	bool reconfig;
};

struct tunable_chip_array {
	const struct tunable_chip_struct *tunable_chip;
	size_t num_tunable_chips;
	uintptr_t dt_base;
};

#define TUNABLE_TABLE_END_MARKER	{ -1, -1, -1, -1 }

void platform_apply_tunables(const struct tunable_chip_struct *tunable_chips,
				uint32_t num_tunable_chips, const char* type);

uint8_t *platform_apply_dt_tunables(const struct tunable_chip_struct *tunable_chips,
					uint32_t num_tunable_chips, uint8_t *buffer,
					uintptr_t dt_base, const char *type);
#endif

