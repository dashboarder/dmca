#include <unittest.h>
#include <lib/libc.h>
#include <string.h>

int libc_putchar(int c)
{
	return c;
}

void test_snprintf(uintptr_t param)
{
	char buf[200];
	int result;

	////////////////////////////////////////////////////////////
	// Tests for format strings without conversion specifiers //
	////////////////////////////////////////////////////////////

	memset(buf, 1, sizeof(buf));
	result = libc_snprintf(buf, sizeof(buf), "");
	TEST_ASSERT_STR_EQ(buf, "");
	TEST_ASSERT_EQ(result, 0);

	memset(buf, 1, sizeof(buf));
	result = libc_snprintf(buf, sizeof(buf), "abc");
	TEST_ASSERT_STR_EQ(buf, "abc");
	TEST_ASSERT_EQ(result, 3);

	// short length truncates output, but returns untruncated length
	memset(buf, 1, sizeof(buf));
	result = libc_snprintf(buf, 1, "abc");
	TEST_ASSERT_STR_EQ(buf, "");
	TEST_ASSERT_EQ(result, 3);

	// short length truncates output, but returns untruncated length
	memset(buf, 1, sizeof(buf));
	result = libc_snprintf(buf, 2, "abc");
	TEST_ASSERT_STR_EQ(buf, "a");
	TEST_ASSERT_EQ(result, 3);

	// length of 0 results in untouched buffer
	memset(buf, 1, sizeof(buf));
	result = libc_snprintf(buf, 0, "abc");
	TEST_ASSERT_EQ((uint8_t)buf[0], 1);
	TEST_ASSERT_EQ((uint8_t)buf[1], 1);
	TEST_ASSERT_EQ((uint8_t)buf[2], 1);
	TEST_ASSERT_EQ((uint8_t)buf[3], 1);
	TEST_ASSERT_EQ(result, 3);

	//////////////////
	// Tests for %% //
	//////////////////

	memset(buf, 1, sizeof(buf));
	result = libc_snprintf(buf, sizeof(buf), "%%");
	TEST_ASSERT_STR_EQ(buf, "%");
	TEST_ASSERT_EQ(result, 1);

	memset(buf, 1, sizeof(buf));
	result = libc_snprintf(buf, 1, "%%");
	TEST_ASSERT_STR_EQ(buf, "");
	TEST_ASSERT_EQ(result, 1);

	memset(buf, 1, sizeof(buf));
	result = libc_snprintf(buf, sizeof(buf), "%%");
	TEST_ASSERT_STR_EQ(buf, "%");
	TEST_ASSERT_EQ(result, 1);

	//////////////////
	// Tests for %s //
	//////////////////

	memset(buf, 1, sizeof(buf));
	result = libc_snprintf(buf, sizeof(buf), "%s", "abc");
	TEST_ASSERT_STR_EQ(buf, "abc");
	TEST_ASSERT_EQ(result, 3);

	memset(buf, 1, sizeof(buf));
	result = libc_snprintf(buf, sizeof(buf), "123%sXYZ", "abc");
	TEST_ASSERT_STR_EQ(buf, "123abcXYZ");
	TEST_ASSERT_EQ(result, 9);

	// truncating in middle of conversion
	memset(buf, 1, sizeof(buf));
	result = libc_snprintf(buf, 5, "%s", "abcdefgh");
	TEST_ASSERT_STR_EQ(buf, "abcd");
	TEST_ASSERT_EQ(result, 8);

	memset(buf, 1, sizeof(buf));
	result = libc_snprintf(buf, sizeof(buf), "%10s", "abc");
	TEST_ASSERT_STR_EQ(buf, "       abc");
	TEST_ASSERT_EQ(result, 10);

	memset(buf, 1, sizeof(buf));
	result = libc_snprintf(buf, sizeof(buf), "%-10s", "abc");
	TEST_ASSERT_STR_EQ(buf, "abc       ");
	TEST_ASSERT_EQ(result, 10);

	memset(buf, 1, sizeof(buf));
	result = libc_snprintf(buf, sizeof(buf), "%1s", "abc");
	TEST_ASSERT_STR_EQ(buf, "abc");
	TEST_ASSERT_EQ(result, 3);

	memset(buf, 1, sizeof(buf));
	result = libc_snprintf(buf, sizeof(buf), "%-1s", "abc");
	TEST_ASSERT_STR_EQ(buf, "abc");
	TEST_ASSERT_EQ(result, 3);

	// TODO: truncate in middle of padding

	//////////////////
	// Tests for %c //
	//////////////////

	memset(buf, 1, sizeof(buf));
	result = libc_snprintf(buf, sizeof(buf), "%c", 0x20);
	TEST_ASSERT_STR_EQ(buf, " ");
	TEST_ASSERT_EQ(result, 1);

	//////////////////
	// Tests for %d //
	//////////////////

	memset(buf, 1, sizeof(buf));
	result = libc_snprintf(buf, sizeof(buf), "%d", 0);
	TEST_ASSERT_STR_EQ(buf, "0");
	TEST_ASSERT_EQ(result, 1);

	memset(buf, 1, sizeof(buf));
	result = libc_snprintf(buf, sizeof(buf), "%d", 1);
	TEST_ASSERT_STR_EQ(buf, "1");
	TEST_ASSERT_EQ(result, 1);

	memset(buf, 1, sizeof(buf));
	result = libc_snprintf(buf, sizeof(buf), "%d", 1234567890);
	TEST_ASSERT_STR_EQ(buf, "1234567890");
	TEST_ASSERT_EQ(result, 10);

	memset(buf, 1, sizeof(buf));
	result = libc_snprintf(buf, sizeof(buf), "%d", -1);
	TEST_ASSERT_STR_EQ(buf, "-1");
	TEST_ASSERT_EQ(result, 2);

	memset(buf, 1, sizeof(buf));
	result = libc_snprintf(buf, sizeof(buf), "%d", -1234567890);
	TEST_ASSERT_STR_EQ(buf, "-1234567890");
	TEST_ASSERT_EQ(result, 11);

	memset(buf, 1, sizeof(buf));
	result = libc_snprintf(buf, sizeof(buf), "%d", -1234567890);
	TEST_ASSERT_STR_EQ(buf, "-1234567890");
	TEST_ASSERT_EQ(result, 11);

	memset(buf, 1, sizeof(buf));
	result = libc_snprintf(buf, sizeof(buf), "%d", 2147483647);
	TEST_ASSERT_STR_EQ(buf, "2147483647");
	TEST_ASSERT_EQ(result, 10);

	memset(buf, 1, sizeof(buf));
	result = libc_snprintf(buf, sizeof(buf), "%d", (int)-2147483648);
	TEST_ASSERT_STR_EQ(buf, "-2147483648");
	TEST_ASSERT_EQ(result, 11);

	memset(buf, 1, sizeof(buf));
	result = libc_snprintf(buf, sizeof(buf), "%04d", 1);
	TEST_ASSERT_STR_EQ(buf, "0001");
	TEST_ASSERT_EQ(result, 4);

	memset(buf, 1, sizeof(buf));
	result = libc_snprintf(buf, sizeof(buf), "%04d", 12345);
	TEST_ASSERT_STR_EQ(buf, "12345");
	TEST_ASSERT_EQ(result, 5);

	memset(buf, 1, sizeof(buf));
	result = libc_snprintf(buf, sizeof(buf), "%04d", -123);
	TEST_ASSERT_STR_EQ(buf, "-123");
	TEST_ASSERT_EQ(result, 4);

	memset(buf, 1, sizeof(buf));
	result = libc_snprintf(buf, sizeof(buf), "%04d", -1234);
	TEST_ASSERT_STR_EQ(buf, "-1234");
	TEST_ASSERT_EQ(result, 5);

	memset(buf, 1, sizeof(buf));
	result = libc_snprintf(buf, sizeof(buf), "%04d", -12345);
	TEST_ASSERT_STR_EQ(buf, "-12345");
	TEST_ASSERT_EQ(result, 6);

	// TODO left and right padding and 0 and ' ' padding in all combos,
	// with too much, just enough, and too little

	// TODO '+' flag

	//////////////////
	// Tests for %u //
	//////////////////

	memset(buf, 1, sizeof(buf));
	result = libc_snprintf(buf, sizeof(buf), "%u", 0);
	TEST_ASSERT_STR_EQ(buf, "0");
	TEST_ASSERT_EQ(result, 1);

	memset(buf, 1, sizeof(buf));
	result = libc_snprintf(buf, sizeof(buf), "%u", 1);
	TEST_ASSERT_STR_EQ(buf, "1");
	TEST_ASSERT_EQ(result, 1);

	memset(buf, 1, sizeof(buf));
	result = libc_snprintf(buf, sizeof(buf), "%u", 1234567890);
	TEST_ASSERT_STR_EQ(buf, "1234567890");
	TEST_ASSERT_EQ(result, 10);

	memset(buf, 1, sizeof(buf));
	result = libc_snprintf(buf, sizeof(buf), "%u", 2147483647);
	TEST_ASSERT_STR_EQ(buf, "2147483647");
	TEST_ASSERT_EQ(result, 10);

	memset(buf, 1, sizeof(buf));
	result = libc_snprintf(buf, sizeof(buf), "%u", 4294967295u);
	TEST_ASSERT_STR_EQ(buf, "4294967295");
	TEST_ASSERT_EQ(result, 10);

	memset(buf, 1, sizeof(buf));
	result = libc_snprintf(buf, sizeof(buf), "%04u", 1);
	TEST_ASSERT_STR_EQ(buf, "0001");
	TEST_ASSERT_EQ(result, 4);

	memset(buf, 1, sizeof(buf));
	result = libc_snprintf(buf, sizeof(buf), "%04u", 12345);
	TEST_ASSERT_STR_EQ(buf, "12345");
	TEST_ASSERT_EQ(result, 5);

	//////////////////
	// Tests for %x //
	//////////////////

	memset(buf, 1, sizeof(buf));
	result = libc_snprintf(buf, sizeof(buf), "%x", 0);
	TEST_ASSERT_STR_EQ(buf, "0");
	TEST_ASSERT_EQ(result, 1);

	memset(buf, 1, sizeof(buf));
	result = libc_snprintf(buf, sizeof(buf), "%#x", 0);
	TEST_ASSERT_STR_EQ(buf, "0x0");
	TEST_ASSERT_EQ(result, 3);

	memset(buf, 1, sizeof(buf));
	result = libc_snprintf(buf, sizeof(buf), "%x", 1);
	TEST_ASSERT_STR_EQ(buf, "1");
	TEST_ASSERT_EQ(result, 1);

	memset(buf, 1, sizeof(buf));
	result = libc_snprintf(buf, sizeof(buf), "%#x", 1);
	TEST_ASSERT_STR_EQ(buf, "0x1");
	TEST_ASSERT_EQ(result, 3);

	memset(buf, 1, sizeof(buf));
	result = libc_snprintf(buf, sizeof(buf), "%x", 0x1234abcd);
	TEST_ASSERT_STR_EQ(buf, "1234abcd");
	TEST_ASSERT_EQ(result, 8);

	memset(buf, 1, sizeof(buf));
	result = libc_snprintf(buf, sizeof(buf), "%x", 0x567890ef);
	TEST_ASSERT_STR_EQ(buf, "567890ef");
	TEST_ASSERT_EQ(result, 8);

	memset(buf, 1, sizeof(buf));
	result = libc_snprintf(buf, sizeof(buf), "%x", 0xffffffff);
	TEST_ASSERT_STR_EQ(buf, "ffffffff");
	TEST_ASSERT_EQ(result, 8);

	memset(buf, 1, sizeof(buf));
	result = libc_snprintf(buf, sizeof(buf), "%8x", 0x1234);
	TEST_ASSERT_STR_EQ(buf, "    1234");
	TEST_ASSERT_EQ(result, 8);

	memset(buf, 1, sizeof(buf));
	result = libc_snprintf(buf, sizeof(buf), "%#8x", 0x1234);
	TEST_ASSERT_STR_EQ(buf, "  0x1234");
	TEST_ASSERT_EQ(result, 8);

	memset(buf, 1, sizeof(buf));
	result = libc_snprintf(buf, sizeof(buf), "%08x", 0x1234);
	TEST_ASSERT_STR_EQ(buf, "00001234");
	TEST_ASSERT_EQ(result, 8);

	memset(buf, 1, sizeof(buf));
	result = libc_snprintf(buf, sizeof(buf), "%#08x", 0x1234);
	TEST_ASSERT_STR_EQ(buf, "0x001234");
	TEST_ASSERT_EQ(result, 8);

	//////////////////
	// Tests for %X //
	//////////////////

	memset(buf, 1, sizeof(buf));
	result = libc_snprintf(buf, sizeof(buf), "%X", 0x1234abcd);
	TEST_ASSERT_STR_EQ(buf, "1234ABCD");
	TEST_ASSERT_EQ(result, 8);

	memset(buf, 1, sizeof(buf));
	result = libc_snprintf(buf, sizeof(buf), "%X", 0x567890ef);
	TEST_ASSERT_STR_EQ(buf, "567890EF");
	TEST_ASSERT_EQ(result, 8);

	memset(buf, 1, sizeof(buf));
	result = libc_snprintf(buf, sizeof(buf), "%8x", 0x1234);
	TEST_ASSERT_STR_EQ(buf, "    1234");
	TEST_ASSERT_EQ(result, 8);

	//////////////////
	// Tests for %p //
	//////////////////

	memset(buf, 1, sizeof(buf));
	result = libc_snprintf(buf, sizeof(buf), "%p", (void *)0);
#if !RELEASE_BUILD
	TEST_ASSERT_STR_EQ(buf, "0x0");
	TEST_ASSERT_EQ(result, 3);
#else
	TEST_ASSERT_STR_EQ(buf, "<ptr>");
	TEST_ASSERT_EQ(result, 5);
#endif

	memset(buf, 1, sizeof(buf));
	result = libc_snprintf(buf, sizeof(buf), "%p", (void *)0x12345678);
#if !RELEASE_BUILD
	TEST_ASSERT_STR_EQ(buf, "0x12345678");
	TEST_ASSERT_EQ(result, 10);
#else
	TEST_ASSERT_STR_EQ(buf, "<ptr>");
	TEST_ASSERT_EQ(result, 5);
#endif

	memset(buf, 1, sizeof(buf));
	result = libc_snprintf(buf, sizeof(buf), "%p %d", (void *)0x12345678, 5555);
#if !RELEASE_BUILD
	TEST_ASSERT_STR_EQ(buf, "0x12345678 5555");
	TEST_ASSERT_EQ(result, 15);
#else
	TEST_ASSERT_STR_EQ(buf, "<ptr> 5555");
	TEST_ASSERT_EQ(result, 10);
#endif

	// make sure we don't truncate %p conversions when the output is longer
	// than the minimum field width
	memset(buf, 1, sizeof(buf));
	result = libc_snprintf(buf, sizeof(buf), "%3p", (void *)0x12345);
#if !RELEASE_BUILD
	TEST_ASSERT_STR_EQ(buf, "0x12345");
	TEST_ASSERT_EQ(result, 7);
#else
	TEST_ASSERT_STR_EQ(buf, "<ptr>");
	TEST_ASSERT_EQ(result, 5);
#endif

	// make sure we don't truncate %p conversions when the output minus the
	// 0x is equal to the minimum field width
	memset(buf, 1, sizeof(buf));
	result = libc_snprintf(buf, sizeof(buf), "%5p", (void *)0x12345);
#if !RELEASE_BUILD
	TEST_ASSERT_STR_EQ(buf, "0x12345");
	TEST_ASSERT_EQ(result, 7);
#else
	TEST_ASSERT_STR_EQ(buf, "<ptr>");
	TEST_ASSERT_EQ(result, 5);
#endif

	// minimum field width is inclusive of the 0x
	memset(buf, 1, sizeof(buf));
	result = libc_snprintf(buf, sizeof(buf), "%7p", (void *)0x12345);
#if !RELEASE_BUILD
	TEST_ASSERT_STR_EQ(buf, "0x12345");
	TEST_ASSERT_EQ(result, 7);
#else
	TEST_ASSERT_STR_EQ(buf, "  <ptr>");
	TEST_ASSERT_EQ(result, 7);
#endif

	// verify padding occurs on %p when output is smaller than
	// minimum field width
	memset(buf, 1, sizeof(buf));
	result = libc_snprintf(buf, sizeof(buf), "%9p", (void *)0x12345);
#if !RELEASE_BUILD
	TEST_ASSERT_STR_EQ(buf, "  0x12345");
	TEST_ASSERT_EQ(result, 9);
#else
	TEST_ASSERT_STR_EQ(buf, "    <ptr>");
	TEST_ASSERT_EQ(result, 9);
#endif
}

struct test_suite printf_suite = {
	.name = "printf",
	.test_cases = {
		{ "snprintf", test_snprintf, 0, "Tests the snprintf function" },
		TEST_CASE_LAST
	}
};

TEST_SUITE(printf_suite);
