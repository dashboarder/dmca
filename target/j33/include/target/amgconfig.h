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
 * The amg_param struct is defined in drivers/apple/amg/amg.h, and this
 * file should only by included from amg.c.
 */

static const struct amg_params amc_phy_params = {
	.flags         = 0,
	.core_config   = 0x00001053,
	.read_latency  = 0x0000669b,
	.gate_offset   = 0x08080808,
	.freq_scale    = 0x00e40488,
	.bit_time_ps   = 3900,
};
