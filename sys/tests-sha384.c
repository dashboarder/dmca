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

#include <unittest.h>
#include <corecrypto/ccsha2.h>
#include <string.h>

void sha384_calculate(const void *in_ptr, size_t in_len, void *out_ptr, size_t out_len);

struct sha384_test {
	const char *data;
	uint32_t len;
	uint8_t expected[CCSHA384_OUTPUT_SIZE];
};

static struct sha384_test sha384_tests[] = {
	{ "", 0,
		{ 0x38, 0xb0, 0x60, 0xa7, 0x51, 0xac, 0x96, 0x38, 0x4c, 0xd9, 0x32, 0x7e, 0xb1, 0xb1, 0xe3, 0x6a, 0x21, 0xfd, 0xb7, 0x11, 0x14, 0xbe, 0x07, 0x43, 0x4c, 0x0c, 0xc7, 0xbf, 0x63, 0xf6, 0xe1, 0xda, 0x27, 0x4e, 0xde, 0xbf, 0xe7, 0x6f, 0x65, 0xfb, 0xd5, 0x1a, 0xd2, 0xf1, 0x48, 0x98, 0xb9, 0x5b} },

	{ "abc", 3,
		{ 0xcb, 0x00, 0x75, 0x3f, 0x45, 0xa3, 0x5e, 0x8b, 0xb5, 0xa0, 0x3d, 0x69, 0x9a, 0xc6, 0x50, 0x07, 0x27, 0x2c, 0x32, 0xab, 0x0e, 0xde, 0xd1, 0x63, 0x1a, 0x8b, 0x60, 0x5a, 0x43, 0xff, 0x5b, 0xed, 0x80, 0x86, 0x07, 0x2b, 0xa1, 0xe7, 0xcc, 0x23, 0x58, 0xba, 0xec, 0xa1, 0x34, 0xc8, 0x25, 0xa7 } },

	{ "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq", 56,
		{ 0x33, 0x91, 0xfd, 0xdd, 0xfc, 0x8d, 0xc7, 0x39, 0x37, 0x07, 0xa6, 0x5b, 0x1b, 0x47, 0x09, 0x39, 0x7c, 0xf8, 0xb1, 0xd1, 0x62, 0xaf, 0x05, 0xab, 0xfe, 0x8f, 0x45, 0x0d, 0xe5, 0xf3, 0x6b, 0xc6, 0xb0, 0x45, 0x5a, 0x85, 0x20, 0xbc, 0x4e, 0x6f, 0x5f, 0xe9, 0x5b, 0x1f, 0xe3, 0xc8, 0x45, 0x2b } },

};

void test_sha384(uintptr_t param)
{
	for (unsigned i = 0; i < sizeof sha384_tests / sizeof(sha384_tests[0]); i++) {
		struct sha384_test *test = &sha384_tests[i];

		uint8_t calculated[CCSHA384_OUTPUT_SIZE];

		sha384_calculate((void *)test->data, test->len, (void *)calculated, sizeof(calculated));

		TEST_ASSERT_MEM_EQ(calculated, test->expected, sizeof(test->expected));
	}
}

struct test_suite sha384_suite = {
	.name = "sha384",
	.test_cases = {
		{ "sha384", test_sha384, 0, "Tests the sha384_calculate function" },
		TEST_CASE_LAST
	}
};

TEST_SUITE(sha384_suite);
