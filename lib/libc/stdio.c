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

int errno = 0;

int puts(const char *str)
{
	enter_critical_section();
	while(*str) {
		putchar(*str);
		str++;
	}
	exit_critical_section();

	return 0;
}

int getchar(void)
{
	char c;

	c = debug_getchar();
	if (c == '\r')
		c = '\n';
	return c;
}

int putchar(int c)
{
	if (c == '\n')
		debug_putchar('\r');
	debug_putchar(c);

	return 0;
}

