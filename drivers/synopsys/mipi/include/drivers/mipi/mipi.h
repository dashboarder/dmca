/*
 * Copyright (C) 2015 Apple Computer, Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */
#ifndef __MIPI_DISPLAY_H
#define __MIPI_DISPLAY_H

#include <sys/types.h>

__BEGIN_DECLS

#define	MAX_PHY_VALUES			9
#define	MAX_NUM_OF_TEST_DATA		6

struct phy_setting_vals {
	uint8_t	test_code;
	uint8_t num_of_data;
	uint8_t	test_data[MAX_NUM_OF_TEST_DATA];
};

struct phy_settings {
	uint8_t				num_of_values;
	struct phy_setting_vals		phy_setting_values[MAX_PHY_VALUES];
};

typedef struct
{
	uint32_t lanes		: 4;	// 3:0
	uint32_t esc_div	: 4;	// 7:4
	uint32_t pll_n		: 3;	// 10:8
	uint32_t resvd0		: 1;	// 11
	uint32_t pll_m		: 10;	// 21:12
	uint32_t pll_p		: 1;	// 22
	uint32_t resvd1		: 1;	// 23
	uint32_t hsfreq		: 8;	// 31:24
	struct phy_settings	target_phy_settings;	
} mipi_t;

__END_DECLS

#endif /* __MIPI_DISPLAY_H */
