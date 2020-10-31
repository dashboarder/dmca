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
			.rdcapcfg = 0x01010304,
		},
		{
			.caoutdllscl = 0x00000010,
			.dqsindllscl = 0x00000010,
			.rdcapcfg = 0x21010309,
		},
		{
			.caoutdllscl = 0x00000020,
			.dqsindllscl = 0x00000020,
			.rdcapcfg = 0x41010306,
		},
		{
			.caoutdllscl = 0x0000003f,
			.dqsindllscl = 0x0000003f,
			.rdcapcfg = 0x61010304,
		},
	},
	.ampclk			= 0x00010000,
};
