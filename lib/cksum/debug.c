/*
 * Copyright (C) 2007-2009, 2013-2014 Apple Inc. All rights reserved.
 * Copyright (C) 2006 Apple Computer, Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */
#include <debug.h>
#include <lib/cksum.h>
#include <sys/menu.h>
#include <drivers/sha1.h>

int do_crc(int argc, struct cmd_arg *args)
{
	if (argc < 3) {
		printf("not enough arguments.\n");
		printf("%s <address> <len>\n", args[0].str);
		return -1;
	}

	if (!security_allow_memory((void *)args[1].u, args[2].u)) {
		printf("Permission Denied\n");
		return -1;
	}

	uint32_t crc32 = crc((void *)args[1].u, args[2].u);
	printf("crc %u (length %zu)\n", crc32, args[2].u);

	return 0;
}

