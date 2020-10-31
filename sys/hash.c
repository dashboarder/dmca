/*
 * Copyright (C) 2014-2015 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */
#include <debug.h>
#include <sys/hash.h>
#include <sys/menu.h>

#if WITH_SHA2_384
void sha384_calculate(const void *buffer, size_t length, void *result, size_t out_len)
{
	RELEASE_ASSERT(out_len >= CCSHA384_OUTPUT_SIZE);
	ccdigest(ccsha384_di(), length, buffer, result);
}
#endif

#if !HOST_TEST

void hash_calculate(const void *in_ptr, size_t in_len, void *out_ptr, size_t out_len)
{
	RELEASE_ASSERT(out_len >= HASH_OUTPUT_SIZE);
#if WITH_SHA2_384
	sha384_calculate(in_ptr, in_len, out_ptr, out_len);
#else	// SHA1
	sha1_calculate(in_ptr, in_len, out_ptr);
#endif
}


#if WITH_SHA2_384 && WITH_MENU

int do_sha384(int argc, struct cmd_arg *args)
{
	u_int8_t  hash[CCSHA384_OUTPUT_SIZE];
	u_int32_t cnt;

	if (argc < 3) {
		printf("not enough arguments.\n");
		printf("%s <address> <len>\n", args[0].str);
		return -1;
	}

	if (!security_allow_memory((void *)args[1].u, args[2].u)) {
		printf("Permission Denied\n");
		return -1;
	}

	sha384_calculate((void *)args[1].u, args[2].u, hash, sizeof(hash));
	printf("sha384 of 0x%08lx for 0x%08lx bytes: ", args[1].u, args[2].u);
	for (cnt = 0; cnt < CCSHA384_OUTPUT_SIZE; cnt++)
		printf("%02x%s", hash[cnt], (cnt != CCSHA384_OUTPUT_SIZE-1) ? ":" : "\n");

	return 0;
}

MENU_COMMAND_DEVELOPMENT(sha384, do_sha384, "SHA_384 hash of specified memory address range.", NULL);

#endif	// WITH_SHA2_384 && WITH_MENU
#endif	// !HOST_TEST */

