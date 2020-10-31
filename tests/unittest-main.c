/*
* Copyright (C) 2014 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */
#include <unittest.h>
#include <setjmp.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

#if UNITTEST_UNITTESTS
#include <debug.h>
#endif

#define DEFAULT_VERBOSITY	(TEST_FAILURE)

static void test_unwind(void) __attribute__((noreturn));

struct test_context {
	bool assertion_failed;
	const char *assertion_file;
	const char *assertion_function;
	unsigned assertion_line;

	bool expect_panic;
	const char *expect_panic_file;
	const char *expect_panic_function;
	unsigned expect_panic_line;

	bool panic_occurred;
	const char *panic_function;
	const char *panic_string;

	jmp_buf jmp_env;

	// nested contexts allow testing of the test functions
	struct test_context *outer_context;
	int depth;
};

static struct test_context *current_context;

int tprintf_verbosity;

static bool run_test_case(struct test_suite *suite, struct test_case *test_case, int verbosity) {
	void **cursor;
	bool test_result;

	struct test_context test_context;

	// Set up a context for handling of panics and assertion failures
	memset(&test_context, 0, sizeof(test_context));
	test_context.outer_context = current_context;
	if (current_context != NULL)
		test_context.depth = current_context->depth + 1;
	current_context = &test_context;

	// call any setup hooks that got registered, intended to be used
	// by mocks so that test suites don't need to worry about any
	// setup the mocks may need
	LINKER_SET_FOREACH(cursor, test_setup_hooks) {
		void (*setup_hook)(void);
		setup_hook = *cursor;

		setup_hook();
	}

	if (suite->setup_function != NULL)
		suite->setup_function();

	if (setjmp(test_context.jmp_env) == 0) {
		test_case->function(test_case->context);

		if (test_context.expect_panic) {
			// we should never return from a test that expected to panic;
			// those tests should always panic and therefore do an unwind
			tprintf(TEST_FAILURE, "!!! test returned without panic expected by %s:%s:%u\n",
				current_context->expect_panic_file, current_context->expect_panic_function,
				current_context->expect_panic_line);
			test_result = false;
		} else {
			// if the test returned normally (didn't call unwind), it's a pass
			test_result = true;
		}
	} else {
		if (!(test_context.expect_panic && test_context.panic_occurred)) {
			// if we get here, someone called longjmp, which means the
			// test failed because of a panic or assertion failure
			test_result = false;
		} else {
			// There was a panic, but it was required by the test
			test_result = true;
		}
	}

	if (suite->cleanup_function != NULL) {
		suite->cleanup_function();
	}

	// pop the context
	current_context = current_context->outer_context;

	return test_result;
}

static bool run_test_suite(struct test_suite *suite, int verbosity)
{
	bool result = true;
	struct test_case *test_case;
	int old_verbosity = tprintf_verbosity;

	tprintf_verbosity = verbosity;

	tprintf(TEST_INFO, "\n# Begin test suite: %s\n", suite->name);

	for (test_case = &suite->test_cases[0]; test_case->name != NULL; test_case++) {
		bool test_result;
		tprintf(TEST_INFO, "\n## Begin test case: %s.%s\n", suite->name, test_case->name);

		test_result = run_test_case(suite, test_case, verbosity);
		result &= test_result;


		if (test_result)
			tprintf(TEST_INFO, "## PASSED test case: %s.%s\n", suite->name, test_case->name);
		else
			tprintf(TEST_FAILURE, "## FAILED test case: %s.%s\n", suite->name, test_case->name);
	}

	tprintf(TEST_INFO, "# End test suite: %s\n", suite->name);

	tprintf_verbosity = old_verbosity;

	return result;
}

// Stops the linker from complaining about the setup hook section not being present
// for test binaries that don't have setup hooks
static void dummy_test_setup_hook(void)
{
}
TEST_SETUP_HOOK(dummy_test_setup_hook);

#if !UNITTEST_UNITTESTS

unsigned int verbosity = DEFAULT_VERBOSITY;

int main(int argc, char *argv[])
{
	bool result = true;
	void **cursor;
	struct test_suite *suite;	
	int opt;

	while ((opt = getopt(argc, argv, "v")) != -1) {
		switch (opt) {
			case 'v':
				verbosity += 10;
		}
	}

	LINKER_SET_FOREACH(cursor, unit_test_suite) {
		suite = *((struct test_suite **)cursor);
		bool suite_result;

		suite_result = run_test_suite(suite, verbosity);

		result &= suite_result;
	}

	return result ? 0 : -1;
}
#endif

static void tvprintf(int verbosity, const char *fmt, va_list ap)
{
	if (tprintf_verbosity >= verbosity) {
		if (verbosity == TEST_FAILURE)
			printf("\x1b[1;31m");
		vprintf(fmt, ap);
		if (verbosity == TEST_FAILURE)
			printf("\x1b[m");
	}
}

void tprintf(int verbosity, const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	tvprintf(verbosity, fmt, ap);
	va_end(ap);
}

static void test_unwind(void)
{
	longjmp(current_context->jmp_env, 1);
}

void _panic(const char *function, const char *str, ...)
{
	va_list ap;
	bool expected;
	char *tag;
	int threshold;

	current_context->panic_occurred = true;
	current_context->panic_function = function;
	current_context->panic_string = str;

	expected = current_context->expect_panic;

	if (expected) {
		threshold = TEST_INFO;
		tag = "ignored panic";
	} else {
		threshold = TEST_FAILURE;
		tag = "!!! panic";
	}

	tprintf(threshold, "%s: ", tag);
	va_start(ap, str);
	tvprintf(threshold, str, ap);
	va_end(ap);
	tprintf(threshold, "\n");

	test_unwind();
}

void test_assert_fail(const char *file, const char *func, unsigned line, const char *fmt, ...)
{
	va_list ap;

	current_context->assertion_failed = true;
	current_context->assertion_file = file;
	current_context->assertion_function = func;
	current_context->assertion_line = line;

	tprintf(TEST_FAILURE, "!!! %s:%s:%u: assertion failed: ", file, func, line);
	va_start(ap, fmt);
	tvprintf(TEST_FAILURE, fmt, ap);
	va_end(ap);
	tprintf(TEST_FAILURE, "\n");

	test_unwind();
}

void test_expect_panic(const char *file, const char *func, unsigned line)
{
	current_context->expect_panic = true;
	current_context->expect_panic_file = file;
	current_context->expect_panic_function = func;
	current_context->expect_panic_line = line;
}


void test_expect_panicked(const char *file, const char *func, unsigned line)
{
	tprintf(TEST_FAILURE, "!!! panic expected by %s:%s:%u did not occur\n",
	        current_context->expect_panic_file, current_context->expect_panic_function,
	        current_context->expect_panic_line);

	// unwind via longjmp because this function is marked as noreturn
	test_unwind();
}

// Qui proba probatorem?

#if UNITTEST_UNITTESTS
static void test_test_success(uintptr_t context)
{
	if (!context)
		TEST_FAIL("expected failure");
}

static void test_test_assert_mem_eq(uintptr_t context)
{
	switch (context) {
	case 0:
		TEST_ASSERT_MEM_EQ("a", "b", 0);
		TEST_ASSERT_MEM_EQ("a", "a", 1);
		TEST_ASSERT_MEM_EQ("a", "a", 2);
		TEST_ASSERT_MEM_EQ("\0a", "\0a", 3);
		TEST_ASSERT_MEM_EQ("aaaa", "aaab", 3);
		TEST_ASSERT_MEM_EQ("\x00\xff\x80", "\x00\xff\x80", 4);
		break;
	case 1:
		TEST_ASSERT_MEM_EQ("a", "b", 1);
		break;
	case 2:
		TEST_ASSERT_MEM_EQ("a", "b", 2);
		break;
	case 3:
		TEST_ASSERT_MEM_EQ("a", "", 1);
		break;
	default:
		TEST_FAIL("invalid context value");
	}
}

static void test_test_assert_mem_neq(uintptr_t context)
{
	switch (context) {
	case 0:
		TEST_ASSERT_MEM_NEQ("a", "b", 1);
		TEST_ASSERT_MEM_NEQ("a", "b", 2);
		TEST_ASSERT_MEM_NEQ("a", "", 1);
		break;
	case 1:
		TEST_ASSERT_MEM_NEQ("a", "b", 0);
		break;
	case 2:
		TEST_ASSERT_MEM_NEQ("a", "a", 1);
		break;
	case 3:
		TEST_ASSERT_MEM_NEQ("a", "a", 2);
		break;
	case 4:
		TEST_ASSERT_MEM_NEQ("\0a", "\0a", 3);
		break;
	case 5:
		TEST_ASSERT_MEM_NEQ("aaaa", "aaab", 3);
		break;
	case 6:
		TEST_ASSERT_MEM_NEQ("\x00\xff\x80", "\x00\xff\x80", 4);
		break;
	default:
		TEST_FAIL("invalid context value");
	}
}

static void test_test_assert_not_null(uintptr_t context)
{
	TEST_ASSERT_NOT_NULL(context);
}

static void test_test_assert_null(uintptr_t context)
{
	TEST_ASSERT_NULL(context);
}

static void test_test_assert_ptr_eq(uintptr_t context)
{
	void *ptr1 = (void *)0x10000000;
	void *ptr2 = (void *)0x10000000;
	void *ptr3 = (void *)0x10000100;

	switch (context) {
	case 0:
		TEST_ASSERT_PTR_EQ(ptr1, ptr1);
		break;
	case 1:
		TEST_ASSERT_PTR_EQ(ptr1, ptr2);
		break;
	case 2:
		TEST_ASSERT_PTR_EQ(ptr2, ptr3);
		break;
	default:
		TEST_FAIL("invalid context value");
	}
}

static void test_test_assert_ptr_neq(uintptr_t context)
{
	void *ptr1 = (void *)0x10000000;
	void *ptr2 = (void *)0x10000000;
	void *ptr3 = (void *)0x10000100;

	switch (context) {
	case 0:
		TEST_ASSERT_PTR_NEQ(ptr1, ptr1);
		break;
	case 1:
		TEST_ASSERT_PTR_NEQ(ptr1, ptr2);
		break;
	case 2:
		TEST_ASSERT_PTR_NEQ(ptr2, ptr3);
		break;
	default:
		TEST_FAIL("invalid context value");
	}
}

static void test_test_assert_str_eq(uintptr_t context)
{
	switch (context) {
	case 0:
		TEST_ASSERT_STR_EQ("", "");
		TEST_ASSERT_STR_EQ("a", "a");
		TEST_ASSERT_STR_EQ("\0a", "\0b");
		TEST_ASSERT_STR_EQ("\xff\x80\x01", "\xff\x80\x01");
		break;
	case 1:
		TEST_ASSERT_STR_EQ("aa", "ab");
		break;
	case 2:
		TEST_ASSERT_STR_EQ("aa", "ba");
		break;
	case 3:
		TEST_ASSERT_STR_EQ("aa", "a");
		break;
	case 4:
		TEST_ASSERT_STR_EQ("a", "aa");
		break;
	default:
		TEST_FAIL("invalid context value");
	}
}

static void test_test_assert_str_neq(uintptr_t context)
{
	switch (context) {
	case 0:
		TEST_ASSERT_STR_NEQ("aa", "ab");
		TEST_ASSERT_STR_NEQ("aa", "ba");
		TEST_ASSERT_STR_NEQ("aa", "a");
		TEST_ASSERT_STR_NEQ("a", "aa");

		break;
	case 1:
		TEST_ASSERT_STR_NEQ("", "");
		break;
	case 2:
		TEST_ASSERT_STR_NEQ("a", "a");
		break;
	case 3:
		TEST_ASSERT_STR_NEQ("\0a", "\0b");
		break;
	case 4:
		TEST_ASSERT_STR_NEQ("\xff\x80\x01", "\xff\x80\x01");
		break;
	default:
		TEST_FAIL("invalid context value");
	}
}

static void test_test_assert_rel_eq(uintptr_t context)
{
	TEST_ASSERT_REL(context, ==, 50u);

	TEST_ASSERT_EQ(context, 50u);
}

static void test_test_assert_rel_neq(uintptr_t context)
{
	TEST_ASSERT_REL(context, !=, 50u);

	TEST_ASSERT_NEQ(context, 50u);
}

static void test_test_assert_rel_gt(uintptr_t context)
{
	TEST_ASSERT_REL(context, >, 50u);
	TEST_ASSERT_GT(context, 50u);
}

static void test_test_assert_rel_lt(uintptr_t context)
{
	TEST_ASSERT_REL(context, <, 50u);
	TEST_ASSERT_LT(context, 50u);
}

static void test_test_assert_rel_uint8(uintptr_t context)
{
	TEST_ASSERT_REL((uint8_t)context, ==, (const uint8_t)50);
}

static void test_test_assert_rel_int8(uintptr_t context)
{
	TEST_ASSERT_REL((int8_t)context, ==, (const int8_t)50);
}

static void test_test_assert_rel_uint16(uintptr_t context)
{
	TEST_ASSERT_REL((uint16_t)context, ==, (const uint16_t)50);
}

static void test_test_assert_rel_int16(uintptr_t context)
{
	TEST_ASSERT_REL((int16_t)context, ==, (const int16_t)50);
}

static void test_test_assert_rel_uint32(uintptr_t context)
{
	TEST_ASSERT_REL((uint32_t)context, ==, (const uint32_t)50);
}

static void test_test_assert_rel_int32(uintptr_t context)
{
	TEST_ASSERT_REL((int32_t)context, ==, (const int32_t)50);
}

static void test_test_assert_rel_uint64(uintptr_t context)
{
	TEST_ASSERT_REL((uint64_t)context, ==, (const uint64_t)50);
}

static void test_test_assert_rel_int64(uintptr_t context)
{
	TEST_ASSERT_REL((int64_t)context, ==, (const int64_t)50);
}

static void test_test_expect_panic(uintptr_t context)
{
	TEST_EXPECT_PANIC();

	if (context) {
		panic("test panic");
	}

	TEST_EXPECT_PANICKED();
}

static void test_test_expect_panic2(uintptr_t context)
{
	TEST_EXPECT_PANIC();

	if (context) {
		panic("test panic");
	}
}

static void test_test_fail(uintptr_t context)
{
	TEST_FAIL("expected failure");
}

static void test_test_panic(uintptr_t context)
{
	if (context) {
		panic("test panic");
	}
}

#define TEST_TEST_CASE(_func, _context, _should_succeed) \
	{ #_func "," #_context, _func, _context, _should_succeed }

struct test_test_case {
	const char *name;
	test_case_func_t function;
	uintptr_t context;
	bool should_succeed;
} test_test_cases[] = {
	TEST_TEST_CASE(test_test_assert_mem_eq, 0, true),
	TEST_TEST_CASE(test_test_assert_mem_eq, 1, false),
	TEST_TEST_CASE(test_test_assert_mem_eq, 2, false),
	TEST_TEST_CASE(test_test_assert_mem_eq, 3, false),
	TEST_TEST_CASE(test_test_assert_mem_neq, 0, true),
	TEST_TEST_CASE(test_test_assert_mem_neq, 1, false),
	TEST_TEST_CASE(test_test_assert_mem_neq, 2, false),
	TEST_TEST_CASE(test_test_assert_mem_neq, 3, false),
	TEST_TEST_CASE(test_test_assert_mem_neq, 4, false),
	TEST_TEST_CASE(test_test_assert_mem_neq, 5, false),
	TEST_TEST_CASE(test_test_assert_mem_neq, 6, false),

	TEST_TEST_CASE(test_test_assert_not_null, 0, false),
	TEST_TEST_CASE(test_test_assert_not_null, 1, true),
	TEST_TEST_CASE(test_test_assert_null, 0, true),
	TEST_TEST_CASE(test_test_assert_null, 1, false),

	TEST_TEST_CASE(test_test_assert_ptr_eq, 0, true),
	TEST_TEST_CASE(test_test_assert_ptr_eq, 1, true),
	TEST_TEST_CASE(test_test_assert_ptr_eq, 2, false),
	TEST_TEST_CASE(test_test_assert_ptr_neq, 0, false),
	TEST_TEST_CASE(test_test_assert_ptr_neq, 1, false),
	TEST_TEST_CASE(test_test_assert_ptr_neq, 2, true),

	TEST_TEST_CASE(test_test_assert_str_eq, 0, true),
	TEST_TEST_CASE(test_test_assert_str_eq, 1, false),
	TEST_TEST_CASE(test_test_assert_str_eq, 2, false),
	TEST_TEST_CASE(test_test_assert_str_eq, 3, false),
	TEST_TEST_CASE(test_test_assert_str_eq, 4, false),
	TEST_TEST_CASE(test_test_assert_str_neq, 0, true),
	TEST_TEST_CASE(test_test_assert_str_neq, 1, false),
	TEST_TEST_CASE(test_test_assert_str_neq, 2, false),
	TEST_TEST_CASE(test_test_assert_str_neq, 3, false),
	TEST_TEST_CASE(test_test_assert_str_neq, 4, false),

	TEST_TEST_CASE(test_test_assert_rel_eq, 0, false),
	TEST_TEST_CASE(test_test_assert_rel_eq, 1, false),
	TEST_TEST_CASE(test_test_assert_rel_eq, 50, true),
	TEST_TEST_CASE(test_test_assert_rel_eq, 51, false),
	TEST_TEST_CASE(test_test_assert_rel_neq, 0, true),
	TEST_TEST_CASE(test_test_assert_rel_neq, 1, true),
	TEST_TEST_CASE(test_test_assert_rel_neq, 50, false),
	TEST_TEST_CASE(test_test_assert_rel_neq, 51, true),
	TEST_TEST_CASE(test_test_assert_rel_gt, 0, false),
	TEST_TEST_CASE(test_test_assert_rel_gt, 1, false),
	TEST_TEST_CASE(test_test_assert_rel_gt, 50, false),
	TEST_TEST_CASE(test_test_assert_rel_gt, 51, true),
	TEST_TEST_CASE(test_test_assert_rel_lt, 0, true),
	TEST_TEST_CASE(test_test_assert_rel_lt, 1, true),
	TEST_TEST_CASE(test_test_assert_rel_lt, 50, false),
	TEST_TEST_CASE(test_test_assert_rel_lt, 51, false),
	TEST_TEST_CASE(test_test_assert_rel_uint8, 50, true),
	TEST_TEST_CASE(test_test_assert_rel_uint8, 51, false),
	TEST_TEST_CASE(test_test_assert_rel_int8, 50, true),
	TEST_TEST_CASE(test_test_assert_rel_int8, 51, false),
	TEST_TEST_CASE(test_test_assert_rel_uint16, 50, true),
	TEST_TEST_CASE(test_test_assert_rel_uint16, 51, false),
	TEST_TEST_CASE(test_test_assert_rel_int16, 50, true),
	TEST_TEST_CASE(test_test_assert_rel_int16, 51, false),
	TEST_TEST_CASE(test_test_assert_rel_uint32, 50, true),
	TEST_TEST_CASE(test_test_assert_rel_uint32, 51, false),
	TEST_TEST_CASE(test_test_assert_rel_int32, 50, true),
	TEST_TEST_CASE(test_test_assert_rel_int32, 51, false),
	TEST_TEST_CASE(test_test_assert_rel_uint64, 50, true),
	TEST_TEST_CASE(test_test_assert_rel_int64, 51, false),

	TEST_TEST_CASE(test_test_expect_panic, 0, false),
	TEST_TEST_CASE(test_test_expect_panic, 1, true),
	TEST_TEST_CASE(test_test_expect_panic2, 0, false),
	TEST_TEST_CASE(test_test_expect_panic2, 1, true),

	TEST_TEST_CASE(test_test_fail, 0, false),

	TEST_TEST_CASE(test_test_panic, 0, true),
	TEST_TEST_CASE(test_test_panic, 1, false),
};

static void run_test_test_case(uintptr_t context)
{
	bool result;
	struct test_test_case *test_test_case;
	
	test_test_case = (struct test_test_case *)context;

	struct test_suite *suite = calloc(sizeof(*suite) + 2 * sizeof(suite->test_cases[0]), 1);

	suite->name = "synth";
	suite->description = "synthesized test case suite";
	suite->setup_function = NULL;
	suite->test_cases[0].name = "synth";
	suite->test_cases[0].function = test_test_case->function;
	suite->test_cases[0].context = test_test_case->context;
	suite->test_cases[0].description = "synthesized test case";

	result = run_test_suite(suite, TEST_SILENT);

	if (result && !test_test_case->should_succeed)
		TEST_FAIL("test passed that should have failed");
	if (!result && test_test_case->should_succeed)
		TEST_FAIL("test failed that should have passed");
	// returning from the bottom of a test function indicates success
}


static struct test_suite test_test_suite = {
	.name = "unittest-simple",
	.description = "simple single test suites",
	.setup_function = NULL,
	.cleanup_function = NULL,
	.test_cases = {
		TEST_CASE_LAST,
		TEST_CASE_LAST,
		TEST_CASE_LAST,
	}
};

// Test the test functions themselves
int main(int argc, char *argv[])
{
	bool suite_result;
	bool result = true;
	struct test_suite *suite;
	int verbosity = DEFAULT_VERBOSITY;

	// First confirm simple tests work as expected
	// We drive these ones manually since we need to prove
	// test suites do the expected things before we start
	// relying on them for our other tests

	test_test_suite.name = "simple";
	test_test_suite.description = "verifies test results get carried into suites correctly";
	test_test_suite.test_cases[0].function = test_test_success;
	test_test_suite.test_cases[0].description = "";
	test_test_suite.test_cases[1].function = test_test_success;
	test_test_suite.test_cases[1].description = "";

	test_test_suite.test_cases[0].name = "success";
	test_test_suite.test_cases[0].context = 1;
	test_test_suite.test_cases[1].name = NULL;
	suite_result = run_test_suite(&test_test_suite, TEST_SILENT) == true;

	result &= suite_result;

	if (suite_result)
		tprintf(TEST_INFO, "## PASSED test case: simple.success\n");
	else
		tprintf(TEST_FAILURE, "## !!! FAILED test case: simple.success\n");

	test_test_suite.test_cases[0].name = "failure";
	test_test_suite.test_cases[0].context = 0;
	test_test_suite.test_cases[1].name = NULL;
	suite_result = run_test_suite(&test_test_suite, TEST_SILENT) == false;

	result &= suite_result;

	if (suite_result)
		tprintf(TEST_INFO, "## PASSED test case: simple.failure\n");
	else
		tprintf(TEST_FAILURE, "## !!! FAILED test case: simple.failure\n");

	test_test_suite.test_cases[0].name = "success";
	test_test_suite.test_cases[0].context = 1;
	test_test_suite.test_cases[1].name = "success2";
	test_test_suite.test_cases[1].context = 1;
	suite_result = run_test_suite(&test_test_suite, TEST_SILENT) == true;

	result &= suite_result;

	if (suite_result)
		tprintf(TEST_INFO, "## PASSED test case: simple.success2\n");
	else
		tprintf(TEST_FAILURE, "## !!! FAILED test case: simple.success2\n");

	test_test_suite.test_cases[0].name = "success";
	test_test_suite.test_cases[0].context = 1;
	test_test_suite.test_cases[1].name = "failure";
	test_test_suite.test_cases[1].context = 0;
	suite_result = run_test_suite(&test_test_suite, TEST_SILENT) == false;

	result &= suite_result;

	if (suite_result)
		tprintf(TEST_INFO, "## PASSED test case: simple.successfailure\n");
	else
		tprintf(TEST_FAILURE, "## !!! FAILED test case: simple.successfailure\n");

	// Now that we've verified that the test suites do the right thing,
	// start-running table-driven test suites to test the assertion macros
	size_t num_cases = sizeof(test_test_cases) / sizeof(test_test_cases[0]);

	suite = calloc(sizeof(*suite) + sizeof(suite->test_cases[0]) * (num_cases + 1), 1);

	suite->name = "assertions";
	suite->description = "tests the assertion macros";

	for (size_t i = 0; i < num_cases; i++) {
		suite->test_cases[i].name = test_test_cases[i].name;
		suite->test_cases[i].function = run_test_test_case;
		suite->test_cases[i].context = (uintptr_t)&test_test_cases[i];
		suite->test_cases[i].description = test_test_cases[i].name;
	}

	result &= run_test_suite(suite, verbosity);

	return result ? 0 : 1;
}

#endif
