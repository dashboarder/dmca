/*
 * Copyright (C) 2006 Apple Computer, Inc. All rights reserved.
 *
 * This document is the property of Apple Computer, Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Computer, Inc.
 */
#include <lib/libiboot.h>

int log2_int(unsigned int val)
{
	if (val == 0)
		return 0;

	return 31 - __builtin_clz(val);
}

