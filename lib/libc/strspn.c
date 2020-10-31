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
strspn(char const *s, char const *accept)
{
	const char *p;
	const char *a;
	size_t count = 0;

	for(p = s; *p != '\0'; ++p) {
		for(a = accept; *a != '\0'; ++a) {
			if(*p == *a)
				break;
		}
		if(*a == '\0')
			return count;
		++count;
	}

	return count;
}
