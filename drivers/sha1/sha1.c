/*
 * Copyright (C) 2007-2012 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */
#include <debug.h>
#include <drivers/sha1.h>
#include <sys/menu.h>

#if WITH_CORECRYPTO

#include <arch.h>
#include <corecrypto/ccdigest.h>
#include <corecrypto/ccsha1.h>

#if WITH_VFP
# define USE_CC_SHA1_OPTIMIZED_IMPLEMENTATION	1 
#endif

const struct ccdigest_info *sha1_get_ccsha1_ccdigest_info()
{
#if USE_CC_SHA1_OPTIMIZED_IMPLEMENTATION
	return ((const struct ccdigest_info *)ccsha1_di());
#else
	return (&ccsha1_ltc_di);
#endif	
}

void sha1_calculate(const void *buffer, size_t length, void *result)
{
#if USE_CC_SHA1_OPTIMIZED_IMPLEMENTATION && !WITH_VFP_ALWAYS_ON
	arch_task_fp_enable(true);
#endif

	ccdigest(sha1_get_ccsha1_ccdigest_info(), length, buffer, result);
}

#elif WITH_HW_SHA1
extern void sha1_hw_calculate(const void *buf, size_t len, u_int32_t *result);

void sha1_calculate(const void *buffer, size_t length, void *result)
{
	if ((NULL == buffer) ||
	    (NULL == result))
		panic("SHA1: bad parameters");

	sha1_hw_calculate(buffer, length, result);
}

#else
# include "mozilla_sha.h"

void sha1_calculate(const void *buffer, size_t length, void *result)
{
	SHA_CTX	ctx;

	shaInit(&ctx);
	shaUpdate(&ctx, (unsigned char *)buffer, length);
	shaFinal(&ctx, (unsigned char *)result);
	/* software sha-1 implementation? */
}
#endif

#if WITH_MENU && !WITH_SIMPLE_MENU
static int do_sha1(int argc, struct cmd_arg *args)
{
	u_int8_t  hash[20];
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

	sha1_calculate((void *)args[1].u, args[2].u, hash);
	printf("sha1 of 0x%08lx for 0x%08lx bytes: ", args[1].u, args[2].u);
	for (cnt = 0; cnt < 20; cnt++)
		printf("%02x%s", hash[cnt], (cnt != 19) ? ":" : "\n");

	return 0;
}

MENU_COMMAND_DEVELOPMENT(sha1, do_sha1, "SHA-1 hash of specified memory address range.", NULL);
#endif


