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

size_t
strlen(char const *s)
{
	size_t i;

	i= 0;
	while(s[i]) {
		i+= 1;
	}

	return i;
}
