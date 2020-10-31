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

#if WITH_TARGET_AMG_PARAMS
#include <target/amgconfig.h>
#else
static const struct amg_params amc_phy_params = {
	.flags         = 0,
	.core_config   = 0x00001053,
	.read_latency  = 0x0000669b,
	.gate_offset   = 0x08080808,
};
#endif
