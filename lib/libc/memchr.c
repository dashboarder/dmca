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

void *
memchr(void const *buf, int c, size_t len)
{
	size_t i;
	unsigned char const *b= buf;
	unsigned char        x= (c&0xff);

	for(i= 0; i< len; i++) {
		if(b[i]== x) {
			return (void*)(b+i);
		}
	}

	return NULL;
}

