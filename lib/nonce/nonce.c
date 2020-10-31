/*
 * Copyright (c) 2012-2015 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */

#include <stdlib.h>

#include <lib/env.h>
#include <lib/mib.h>
#include <lib/nonce.h>
#include <lib/nvram.h>
#include <lib/random.h>
#include <debug.h>
#include <platform.h>
#include <sys/hash.h>
#include <sys/menu.h>

static const char	*kBootNoncePropertyKey = "com.apple.System.boot-nonce";
static const size_t	kExpectedNonceLength = (sizeof(uint64_t) * 2) + 2;

int
mobile_ap_nonce_consume_nonce(uint64_t *nonce)
{
	const char*	nonce_string;
	size_t		nonce_length = 0;
	int		result = 0;

	ASSERT(nonce != NULL);

	*nonce = 0;
	nonce_string = env_get(kBootNoncePropertyKey);

	if (nonce_string != NULL) {
		dprintf(DEBUG_SPEW, "%s: nonce_string='%s'\n", __func__, nonce_string);
		nonce_length = strlen(nonce_string);
		dprintf(DEBUG_SPEW, "%s: nonce_length='%zu'\n", __func__, nonce_length);
		if (nonce_length == kExpectedNonceLength) {
			*nonce = strtoull(nonce_string, NULL, 16);
			result = 1;
		} else {
#if DEBUG_BUILD
			dprintf(DEBUG_CRITICAL,
				"%s: nonce value has invalid length %zu %s\n",
				__func__, nonce_length, nonce_string);
#else
			dprintf(DEBUG_CRITICAL,
				"%s: nonce value has invalid length\n",
				__func__);
#endif
		}

		// The nonce environment variable exists.
		// Remove it whether it is valid or not.
		if (env_unset(kBootNoncePropertyKey)) {
			// Save the updated nvram contents.
			if (nvram_save() != 0) {
				dprintf(DEBUG_CRITICAL,
					"%s: Could not save nvram contents\n",
					__func__);
			}
		} else {
			dprintf(DEBUG_CRITICAL,
				"%s: could not clear nonce\n",
				__func__);
		}
	}

	return result;
}


// =============================================================================

static void
print_usage(int argc, struct cmd_arg *args)
{
	printf("USAGE: %s <subcmd>\n\n", ((0 < argc) ? args[0].str : "?"));
	printf("  Where <subcmd> is one of following\n\n");
	printf("    get     - read the nonce and clear it, then report its hash and value\n");
	printf("              if no nonce was previously set, one is generated\n");
	if (mib_get_bool(kMIBTargetWithEffaceable)) {
		return;
	}
	printf("    clear   - clear the nonce\n");
	printf("    consume - read the raw nonce, report its hash and value, and comsume it\n");
	printf("    read    - read the raw nonce, report its hash and value, but don't consume it\n");
}

static void
print_nonce_and_hash(uint64_t nonce)
{
	uint8_t		hash[HASH_OUTPUT_SIZE];
	int		idx;

	hash_calculate(&nonce, sizeof(nonce), hash, sizeof(hash));

	printf("boot-nonce: 0x%016llx\n", nonce);
	printf("boot-nonce hash:");
	for (idx = 0; idx < HASH_OUTPUT_SIZE; idx++) {
		printf(" %02X", hash[idx]);
	}
	printf("\n");
}

static int
get_nonce(void)
{
	uint64_t nonce;

	nonce = platform_get_nonce();
	print_nonce_and_hash(nonce);

	return 0;
}

static int
clear_nonce(void)
{
	const char*	nonce_string;

	nonce_string = env_get(kBootNoncePropertyKey);
	if (nonce_string != NULL) {
		if (env_unset(kBootNoncePropertyKey)) {
			// Save the updated nvram contents.
			if (nvram_save() == 0) {
				printf("Boot nonce cleared\n");
			} else {
				printf("Failed to update nvram\n");
			}
		} else {
			printf("Failed to clear boot nonce\n");
		}
	} else {
		printf("Boot nonce not found\n");
	}

	return 0;
}

static int
consume_nonce(void)
{
	uint64_t	nonce;

	nonce = platform_consume_nonce();
	print_nonce_and_hash(nonce);

	return 0;
}

static int
read_nonce(void)
{
	uint64_t	nonce;
	const char*	nonce_string;
	size_t		nonce_length = 0;

	nonce_string = env_get(kBootNoncePropertyKey);
	if (nonce_string != NULL)
		nonce_length = strlen(nonce_string);

	if (nonce_length != 0) {
		// The nonce string must be the exact length of a
		// 64-bit integer plus 2 for the "0x" prefix.
		if (nonce_length == kExpectedNonceLength) {
			nonce = strtoull(nonce_string, NULL, 16);
			print_nonce_and_hash(nonce);
		} else {
			printf("Boot nonce value (%s) has invalid length %zu should be %zu",
			       nonce_string, nonce_length, kExpectedNonceLength);
		}
	} else {
		printf("Boot nonce not found\n");
	}

	return 0;
}

int
do_nonce(int argc, struct cmd_arg *args)
{
	int	err;
	bool	with_effaceable = mib_get_bool(kMIBTargetWithEffaceable);

	if (1 >= argc) {
		print_usage(argc, args);
		err = 0;
	} else if (0 == strcmp("get", args[1].str)) {
		err = get_nonce();
	} else if (!with_effaceable && (0 == strcmp("clear", args[1].str))) {
		err = clear_nonce();
	} else if (!with_effaceable && (0 == strcmp("consume", args[1].str))) {
		err = consume_nonce();
	} else if (!with_effaceable && (0 == strcmp("read", args[1].str))) {
		err = read_nonce();
	} else {
		print_usage(argc, args);
		printf("unrecognized subcommand\n");
		err = -1;
	}

	return err;
}

