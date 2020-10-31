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
#include <lib/libc.h>
#include <sys.h>

static int randseed = 12345;

int system(const char *string)
{
	printf("unsupported: system(%s)\n", string);
	return -1;
}

void exit(int status)
{
	panic("exit(%d) called\n", status);

	for (;;)
		;
}

int rand(void)
{
	return (randseed = randseed * 12345 + 17);
}

void srand(unsigned int seed)
{
	randseed = (int)seed;
}
