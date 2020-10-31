/*
 * Copyright (C) 2007-2012 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */
#include <platform.h>
#include <platform/chipid.h>

bool chipid_get_raw_production_mode(void)
{
	bool current_production_mode = chipid_get_current_production_mode();
	u_int32_t minimum_epoch = chipid_get_minimum_epoch();

	// Pre-H6 platforms only support a "current" view of fuses.
	// However, pre-H6 platforms only used epoch greater than 0 for
	// production fused chips, so we can infer demotion.
	return current_production_mode || (minimum_epoch > 0);
}
