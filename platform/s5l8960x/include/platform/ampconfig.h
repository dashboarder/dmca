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

/*
 * The amc_param struct is defined in drivers/apple/amp/amp.h, and this
 * file should only by included from that amp.c.
 */

#if WITH_TARGET_AMP_PARAMS
#include <target/ampconfig.h>
#else
static struct amp_params amc_phy_params = {
	.freq = {
		{
			.caoutdllscl = 0x00000008,
			.dqsindllscl = 0x00000008,
			.rdcapcfg = 0x0102050f,
		},
		{
			.caoutdllscl = 0x00000010,
			.dqsindllscl = 0x00000010,
			.rdcapcfg = 0x21020509,
		},
		{
			.caoutdllscl = 0x00000020,
			.dqsindllscl = 0x00000020,
			.rdcapcfg = 0x41020506,
		},
		{
			.caoutdllscl = 0x0000003f,
			.dqsindllscl = 0x0000003f,
			.rdcapcfg = 0x61020504,
		},
	},
	.drive_strength = 0x14141414,
	.cacalib_hw_loops = 8,
	.cacalib_sw_loops = 8,
};
#endif
