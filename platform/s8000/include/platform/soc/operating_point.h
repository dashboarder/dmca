/*
 * Copyright (C) 2012-2013 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */
#ifndef __PLATFORM_SOC_OPERATING_POINT_H
#define __PLATFORM_SOC_OPERATING_POINT_H

#include <platform/soc/chipid.h>
struct operating_point_params {
	union {
		struct {
			uint64_t voltage_index:9;
			uint64_t bypass:1;
			uint64_t clkSrc:1;
			uint64_t preDivP:6;
			uint64_t fbkDivM:9;
			uint64_t pstDivS:6;
			uint64_t biuDiv4HiVol:3;
			uint64_t biuDiv4LoVol:3;
			uint64_t dvmrMaxWgt:7;
			uint64_t iexrfCfgWrData:2;
			uint64_t iexrfCfgWrIdxa:2;
			uint64_t iexrfCfgWrIdxb:2;
			uint64_t exrfCfgWrIdxmuxsel:2;
			uint64_t nrgAccScaleTab:8;
			uint64_t lkgEstTab:8;
			uint64_t dpe0DvfmTab:8;
		};
		struct {
			uint64_t voltage_index:9;
			uint64_t bypass:1;
			uint64_t clkSrc:1;
			uint64_t preDivP:6;
			uint64_t fbkDivM:9;
			uint64_t pstDivS:6;
		} gpu;
		struct {
			uint64_t chipid_max:9; // must contains CHIPID_MAX
			uint64_t chip_id:16;
			uint64_t voltage_type:3;
			uint64_t chip_rev:16;
			uint64_t reserved:18;
		};
	};
};
extern const struct operating_point_params *operating_point_get_params(enum chipid_voltage_index voltage_index, enum chipid_voltage_type voltage_type);



#endif /* __PLATFORM_SOC_CHIPID_H */
