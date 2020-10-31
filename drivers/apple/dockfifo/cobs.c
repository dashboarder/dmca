/*
 * Copyright (C) 2013 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>

#if defined(TEST)
# include <stdio.h>
# define DEBUG_INFO 0
# define DEBUG_SPEW 0
#endif

#include "cobs.h"

bool cobs_encode(void *out_buf, size_t out_buf_bytes,
		 const void *in_buf, size_t in_buf_bytes,
		 size_t *out_buf_used)
{
	// Output buffer iterator, skipping first code byte.
	uint8_t *restrict out = (uint8_t *) out_buf;
	size_t out_pos = 1;

	// Input buffer iterator, starting at beginning of buffer.
	const uint8_t *restrict in = (const uint8_t *) in_buf;
	size_t in_pos = 0;
	
	// Position of the code prefixing the current non-zero run of data.
	size_t code_pos = 0;
	uint8_t code = 1;
	if (out_buf_bytes < 1) {
		dprintf(DEBUG_INFO,
			"COBS-encoding always has at least one byte output\n");
		return false;
	}

	// Copy non-zero runs of data
	while (in_pos < in_buf_bytes) {
		uint8_t byte = in[in_pos++];
		if (byte != 0) {
			// Copy non-0x00 byte across.
			if (out_pos >= out_buf_bytes - 1) {
				// Must be space for this data byte,
				// and then at least one more code byte.
				dprintf(DEBUG_INFO,
					"COBS-encode ran out of space\n");
				return false;
			}
			out[out_pos++] = byte;
			// Increment code. 8 bit limit checked below.
			++code;
		}
		if (byte == 0 || code >= 0xff) {
			// Input is 0x00 byte, or we just hit the max code.
			// Fill in the skipped code byte before the run.
			out[code_pos] = code;
			code = 1;
			// Already checked for code byte space above.
			code_pos = out_pos++;
		}
	}
	// Output final code assuming an implicit 0x00 terminator.
	// Invariant: code_pos < out_buf_bytes.
	out[code_pos] = code;
	*out_buf_used = out_pos;
	return true;
}

bool cobs_decode(void *out_buf, size_t out_buf_bytes,
		 const void *in_buf, size_t in_buf_bytes,
		 size_t *out_buf_used)
{
	// Output buffer iterator.
	uint8_t *restrict out = (uint8_t *) out_buf;
	size_t out_pos = 0;

	// Input buffer iterator.
	const uint8_t *restrict in = (const uint8_t *) in_buf;
	size_t in_pos = 0;

	while (in_pos < in_buf_bytes) {
		uint8_t code = in[in_pos++];
		if (code == 0) {
			dprintf(DEBUG_INFO,
				"COBS-decode found illegal 0x00 byte\n");
			return false;
		}
		// Copy one less data bytes than the code.
		for (uint8_t i = 1; i < code; ++i) {
			if (in_pos >= in_buf_bytes) {
				dprintf(DEBUG_INFO,
					"COBS-decode found truncated input\n");
				return false;
			} else if (out_pos >= out_buf_bytes) {
				dprintf(DEBUG_INFO,
					"COBS-decode ran out of space %d/%d\n",
					(int) out_pos, (int) out_buf_bytes);
				return false;
			}
			out[out_pos++] = in[in_pos++];
		}
		// Output trailing 0x00, unless it's the implicit terminator,
		// or it was the max value code.
		if (code != 0xff && in_pos < in_buf_bytes) {
			if (out_pos >= out_buf_bytes) {
				dprintf(DEBUG_INFO,
					"COBS-decode ran out of space\n");
				return false;
			}
			out[out_pos++] = 0x00;
		}
	}
	*out_buf_used = out_pos;
	return true;
}
 
#ifdef TEST

#include <stdlib.h>
#include <string.h>

static void dump_buf(const uint8_t *buf, size_t bytes) {
	for (size_t i = 0; i < bytes; ++i) {
		printf(" %02x", buf[i]);
	}
	printf("\n");
}

static void test_case(const uint8_t *in_buf, size_t in_buf_size,
		      const uint8_t *expect_buf, size_t expect_buf_size)
{
	uint8_t out_buf[1024];
	uint8_t out_buf2[1024];
	assert(sizeof(out_buf) == sizeof(out_buf2));
	assert(in_buf_size <= sizeof(out_buf) &&
	       expect_buf_size <= sizeof(out_buf));
	size_t out_used = 0;

	// Test forward encoding matches expected output.
	bool ok = cobs_encode(out_buf, sizeof(out_buf),
			      in_buf, in_buf_size,
			      &out_used);
	assert(ok);
	if (out_used != expect_buf_size) {
		printf("Encode generated %d bytes instead of %d\n",
		       (int) out_used, (int) expect_buf_size);
		dump_buf(out_buf, out_used);
		abort();
	}
	assert(memcmp(out_buf, expect_buf, expect_buf_size) == 0);
	
	// Test round-trip decoding matches original input.
	ok = cobs_decode(out_buf2, sizeof(out_buf2),
			 out_buf, expect_buf_size,
			 &out_used);
	assert(ok);
	if (out_used != in_buf_size) {
		printf("Decode generated %d bytes instead of %d\n",
		       (int) out_used, (int) in_buf_size);
		dump_buf(out_buf2, out_used);
		abort();
	}
	assert(memcmp(out_buf2, in_buf, in_buf_size) == 0);
}

int main()
{
	printf("COBS test\n");

	{
		// Empty packet encodes a single 0x01.
		static const uint8_t expect[1] = { 0x01 };
		test_case(NULL, 0, expect, sizeof(expect));
	}

	{
		// Short run, no zeroes.
		static const uint8_t in_buf[3] = { 0x01, 0x02, 0x03 };
		static const uint8_t expect[4] = { 0x04, 0x01, 0x02, 0x03 };
		test_case(in_buf, sizeof(in_buf), expect, sizeof(expect));
	}

	{
		// Single zero encodes to a double 0x01.
		static const uint8_t in_buf[1] = { 0x00 };
		static const uint8_t expect[2] = { 0x01, 0x01 };
		test_case(in_buf, sizeof(in_buf), expect, sizeof(expect));
	}

	{
		// 254 bytes encode to 0xff <bytes> 0x01
		uint8_t in_buf[254];
		memset(in_buf, 0xff, sizeof(in_buf));
		uint8_t expect[256];
		expect[0] = 0xff;
		memset(expect + 1, 0xff, sizeof(expect) - 2);
		expect[sizeof(expect) - 1] = 0x01;
		test_case(in_buf, sizeof(in_buf), expect, sizeof(expect));
	}

	{
		// 254 bytes and a zero encode to 0xff <bytes> 0x01 0x01
		uint8_t in_buf[255];
		memset(in_buf, 0xff, sizeof(in_buf));
		in_buf[sizeof(in_buf) - 1] = 0x00;
		uint8_t expect[257];
		expect[0] = 0xff;
		memset(expect + 1, 0xff, sizeof(expect) - 3);
		expect[sizeof(expect) - 2] = 0x01;
		expect[sizeof(expect) - 1] = 0x01;
		test_case(in_buf, sizeof(in_buf), expect, sizeof(expect));
	}

	printf("COBS test passed\n");
	return 0;
}

#endif  // defined(TEST)
