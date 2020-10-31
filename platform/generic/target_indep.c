/*
 * Copyright (C) 2012-2014 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */

#include <platform.h>
#include <stdbool.h>
#include <target.h>

bool target_config_ap(void)
{
	static bool valid = false;
	static bool result;

	if (!valid) {
		result = (platform_get_board_id() & 1) == 0;
		valid = true;
	}

	return result;
}

bool target_config_dev(void)
{
	static bool valid = false;
	static bool result;

	if (!valid) {
		result = (platform_get_board_id() & 1) != 0;
		valid = true;
	}

	return result;
}
