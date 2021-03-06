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

/*
 * The amc_phy_params struct is defined in drivers/apple/amp_v3/amp_v3.h, and this
 * file should only by included from amp_v3.c.
 */

static const struct amp_params amc_phy_params = {
	.freq = {
		{
			.caoutdllscl = 0x00000008,
			.dqsindllscl = 0x00000008,
#if SUB_PLATFORM_T7000
			.rdcapcfg = 0x0101050f,
#elif SUB_PLATFORM_T7001
			.rdcapcfg = 0x0101050e,
#endif
		},
		{
			.caoutdllscl = 0x00000010,
			.dqsindllscl = 0x00000010,
			.rdcapcfg = 0x21010509,
		},
		{
			.caoutdllscl = 0x00000020,
			.dqsindllscl = 0x00000020,
			.rdcapcfg = 0x41010506,
		},
		{
			.caoutdllscl = 0x0000003f,
			.dqsindllscl = 0x0000003f,
			.rdcapcfg = 0x61010504,
		},
	},
	.dqdqsds		= 0x00001212,
	.nondqds		= 0x12124a52,
#if SUB_PLATFORM_T7000
	.ampclk			= 0x00010000,
#endif
};
