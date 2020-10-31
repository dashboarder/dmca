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

char *
strrchr(char const *s, int c)
{
	char const *last= c?0:s;


	while(*s) {
		if(*s== c) {
			last= s;
		}

		s+= 1;
	}

	return (char *)last;
}
