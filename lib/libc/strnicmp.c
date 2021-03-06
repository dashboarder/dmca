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
#include <ctype.h>
#include <sys/types.h>

int
strnicmp(char const *s1, char const *s2, size_t len)
{
	unsigned char c1 = '\0';
	unsigned char c2 = '\0';

	if(len > 0) {
		do {
			c1 = *s1; c2 = *s2;
			s1++; s2++;
			if(!c1)
				break;
			if(!c2)
				break;
			if(c1 == c2)
				continue;
			c1 = tolower(c1);
			c2 = tolower(c2);
			if (c1 != c2)
				break;
		} while(--len);
	}
	return (int)c1 - (int)c2;
}
