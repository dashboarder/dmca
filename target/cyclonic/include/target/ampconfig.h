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

static struct amp_params amc_phy_params = {
	.freq = {
			{
				.caoutdllscl = 0x00000008,
				.dqsindllscl = 0x00000008,
				.rdcapcfg = 0x0002040a
			},
			{
				.caoutdllscl = 0x00000010,
				.dqsindllscl = 0x00000010,
				.rdcapcfg = 0x20020308
			},
			{
				.caoutdllscl = 0x00000020,
				.dqsindllscl = 0x00000020,
				.rdcapcfg = 0x40020308
			},
			{
				.caoutdllscl = 0x0000003f,
				.dqsindllscl = 0x0000003f,
				.rdcapcfg = 0x60020308
			}
		}
};
