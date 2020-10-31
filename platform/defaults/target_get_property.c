/*
 * Copyright (C) 2012-2013 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */
#include <target.h>

bool
target_get_property(enum target_property prop, void *data, int maxdata, int *retlen)
{
	return target_get_property_base(prop, data, maxdata, retlen);
}
