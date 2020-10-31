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
		default:
			break;
		}
	}

	if (retlen != NULL) *retlen = length;

	return result;
}

#endif
