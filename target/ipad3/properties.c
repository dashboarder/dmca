/*
 * Copyright (C) 2011-2012 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */
#include <debug.h>
#include <target.h>
#include <target/powerconfig.h>

#if WITH_DEVICETREE

bool
target_get_property(enum target_property prop, void *data, int maxdata, int *retlen)
{
	int length = 0;
	bool result = false;

	if ((result = target_get_property_base(prop, data, maxdata, retlen)) == false) {
		switch(prop) {
		case TARGET_PROPERTY_RESTORE_BACKLIGHT_LEVEL:
			// fix the restore backlight level at the precharge level,
			// to make sure the backlight does not cause the battery to
			// drain during restore
			if (maxdata >= (ssize_t)sizeof(uint32_t)) {
				result = true;
				length = sizeof(uint32_t);
				*(uint32_t *)data = PRECHARGE_BACKLIGHT_LEVEL;
			}
			break;
			
		default:
			break;
		}
	}

	if (retlen != NULL) *retlen = length;

	return result;
}

#endif
