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

/*
 * The amc_param struct is defined in drivers/apple/amp/amp.h, and this
 * file should only by included from that amp.c.
 */

#if SUPPORT_FPGA

static struct amp_params amc_phy_params = {
	.freq = {
			{
				.caoutdllscl = 0x00000008,
				.dqsindllscl = 0x00000008,
				.rdcapcfg = 0x21020304
			},
			{
				.caoutdllscl = 0x00000010,
				.dqsindllscl = 0x00000010,
				.rdcapcfg = 0x31020307
			},
			{
				.caoutdllscl = 0x00000020,
				.dqsindllscl = 0x00000020,
				.rdcapcfg = 0x51020305
			},
			{
				.caoutdllscl = 0x0000003f,
				.dqsindllscl = 0x0000003f,
				.rdcapcfg = 0x61020304
			}
		},
	.drive_strength = 0x12121212,
};

#else

#error "Not supported for this target"

#endif
