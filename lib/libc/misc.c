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
#include <compiler.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>

void hexdump(const void *_ptr, size_t len)
{
	const unsigned char *ptr = _ptr;
	unsigned int i;

	printf("hexdump ptr %p, len %zd\n", ptr, len);
	
	while (len > 0) {
		printf("%p: ", ptr);
		for (i=0; i < __min(len, 16u); i++) {
			printf("%02x ", *ptr);
			ptr++;
		}
		printf("\n");
		if (len < 16)
			break;
		len -= 16;
	}
}

