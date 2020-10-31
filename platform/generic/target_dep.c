/*
 * Copyright (C) 2014 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */

#include <lib/syscfg.h>
#include <lib/env.h>
#include <stdint.h>

// Target/product dependent functions that aren't expected to be overridden by targets

bool target_is_tethered(void)
{
#if WITH_SYSCFG
	uint8_t tethcfg[16];

	if ((syscfgCopyDataForTag('Teth', tethcfg, 16) != -1) && (tethcfg[0] != 0x0)) {
		return true;
	}
#endif

	return 0 != env_get_uint("is-tethered", 0);
}
