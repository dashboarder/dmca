/*
 * Copyright (C) 2011 Apple Inc. All rights reserved.
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
static const struct amp_params amc_phy_params = {
	.flags = (FLAG_AMP_PARAM_SUPPORT_FMCLK_OFF | FLAG_AMP_PARAM_FULLDIFFMODE),
	.freq = {
		{
			.scl = 0x08,
			.rdcapcfg = 0x00010308,
		},
		{
			.scl = 0x0b,
			.rdcapcfg = 0x00010306,
		},
		{
			.scl = 0x16,
			.rdcapcfg = 0x00010303,
		},
		{
			.scl = 0x3f,
			.rdcapcfg = 0x00010301,
		},
	},
};
#endif
