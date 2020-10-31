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
#include <string.h>
#include <sys/types.h>

int
strcmp(char const *cs, char const *ct)
{
	signed char __res;

	while(1) {
		if((__res = *cs - *ct++) != 0 || !*cs++)
			break;
	}

	return __res;
}
