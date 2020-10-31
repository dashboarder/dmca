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

void *memscan(void *addr, int c, size_t size)
{
	unsigned char *p = (unsigned char *)addr;

	while(size) {
		if(*p == c)
			return (void *)p;
		p++;
		size--;
	}
  	return (void *)p;
}
