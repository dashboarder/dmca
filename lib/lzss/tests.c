#include <unittest.h>
#include <lib/lzss.h>
#include <stdlib.h>
#include <string.h>

struct lzss_test {
	const uint8_t *input;
	size_t input_len;
	const uint8_t *expected;
	size_t expected_len;
	size_t buf_len;
};

struct lzss_test tests[] = {
	{
		(const uint8_t *)&(const uint8_t[]){ 0xff, 0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87 },
		9,
		(const uint8_t *)&(const uint8_t[]){ 0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87 },
		8,
		4096
	},
#if 0
	{
		(const uint8_t *)&(const uint8_t[]){ 0xff, 0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x00, 0x00, 0x00 },
		12,
		(const uint8_t *)&(const uint8_t[]){ 0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x80, 0x81, 0x82 },
		11,
		4096
	}
#endif
};

void test_lzss(uintptr_t param)
{
	struct lzss_test *test = (struct lzss_test *)param;
	uint32_t guard = 0xdeadbeef;
	uint8_t *dst;
	uint32_t result;

	dst = malloc(test->buf_len + sizeof(guard));
	memset(dst, 0xcc, test->buf_len);

	// If the test is expected to succeed, put a guard right
	// after the last expected output byte of the buffer; if it's
	// expected to fail, just put it at the end of the buffer
	if (test->expected_len != 0)
		memcpy(dst + test->expected_len, &guard, sizeof(guard));
	else
		memcpy(dst + test->buf_len, &guard, sizeof(guard));

	result = decompress_lzss(dst, test->buf_len, test->input, test->input_len);
	TEST_ASSERT_EQ(result, test->expected_len);

	if (test->expected_len != 0) {
		TEST_ASSERT_MEM_EQ(dst, test->expected, test->expected_len);
		TEST_ASSERT_MEM_EQ(dst + test->expected_len, &guard, sizeof(guard));
	} else {
		TEST_ASSERT_MEM_EQ(dst + test->buf_len, &guard, sizeof(guard));
	}

	free(dst);
}

static struct test_suite lzss_test_suite = {
	.name = "lzss",
	.description = "tests the lzss implementation",
	.test_cases = {
		{ "lzss", test_lzss, (uintptr_t)&tests[0] },
#if 0
		{ "lzss", test_lzss, (uintptr_t)&tests[1] },
#endif
		TEST_CASE_LAST
	}
};

TEST_SUITE(lzss_test_suite);
