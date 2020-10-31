#ifndef UNITTEST_H
#define UNITTEST_H

#include <sys/linker_set.h>
#include <compiler.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

// Used for logging progress during tests
#define TEST_SILENT	(0)
#define TEST_FAILURE	(10)
#define TEST_INFO 	(20)
#define TEST_SPEW 	(30)
void tprintf(int verbosity, const char *fmt, ...) __printflike(2, 3);

typedef void (*test_case_func_t)(uintptr_t);
typedef void (*test_suite_setup_func_t)(void);
typedef void (*test_suite_cleanup_func_t)(void);

struct test_case {
	const char *name;
	test_case_func_t function;
	uintptr_t context;
	const char *description;
};

struct test_suite {
	const char *name;
	const char *description;
	test_suite_setup_func_t setup_function;
	test_suite_cleanup_func_t cleanup_function;
	struct test_case test_cases[];
};

#define TEST_CASE_LAST 		\
	{ NULL, NULL, 0, NULL }

#define TEST_SUITE(_suite) LINKER_SET_ENTRY(unit_test_suite, _suite)

#define TEST_SETUP_HOOK(_func) LINKER_SET_ENTRY(test_setup_hooks, _func)

// constructs a literal array that can be passed into the assert macros
#define TEST_ARRAY(type, params...) &(const type[]){params}

// generates the appropriate printf format string for _val's type
#define _FMT(_val)					\
	_Generic(_val,					\
		bool: "%u",				\
		unsigned char: "%u",			\
		const unsigned char: "%u",		\
		signed char: "%d",			\
		const signed char: "%d",		\
		unsigned short: "%u",			\
		const unsigned short: "%u",		\
		signed short: "%d",			\
		const signed short: "%d",		\
		unsigned int: "%u",			\
		const unsigned int: "%u",		\
		signed int: "%d",			\
		const signed int: "%d",			\
		unsigned long: "%lu",			\
		const unsigned long: "%lu",		\
		signed long: "%ld",			\
		const signed long: "%ld",		\
		unsigned long long: "%llu",		\
		const unsigned long long: "%llu",	\
		signed long long: "%llu",		\
		const signed long long: "%llu"		\
	)

#define TEST_ASSERT(_a)								\
	do {									\
		if (!(_a)) {							\
			test_assert_fail(__FILE__, __FUNCTION__, __LINE__,	\
			                 "%s", #_a);				\
		}								\
	} while (0)

// For equality testing, we just coerce everything to  uint64_t to avoid spurious
// comparison of signed and unsigned errors; we don't do this for > and friends
// because that would change the sense of the comparison
#define TEST_ASSERT_REL_EQ(_a, _rel, _b)					\
	do {									\
		typeof(_a) _val_a = (_a);					\
		typeof(_b) _val_b = (_b);					\
		uint64_t _unsigned_a = (uint64_t)_val_a;			\
		uint64_t _unsigned_b = (uint64_t)_val_b;			\
		if (!(_unsigned_a _rel _unsigned_b)) {				\
			char _str_a[40], _str_b[40];				\
			snprintf(_str_a, sizeof(_str_a), _FMT(_a), _val_a);	\
			snprintf(_str_b, sizeof(_str_b), _FMT(_b), _val_b);	\
			test_assert_fail(__FILE__, __FUNCTION__, __LINE__,	\
			                 "\"%s %s %s\" (%s %s %s)",		\
			                 #_a, #_rel, #_b, _str_a, #_rel, _str_b);	\
		}								\
	} while(0)

#define TEST_ASSERT_EQ(_a, _b) TEST_ASSERT_REL_EQ(_a, ==, _b)
#define TEST_ASSERT_NEQ(_a, _b) TEST_ASSERT_REL_EQ(_a, !=, _b)

#define TEST_ASSERT_REL(_a, _rel, _b)						\
	do {									\
		typeof(_a) _val_a = (_a);					\
		typeof(_b) _val_b = (_b);					\
		if (!(_val_a _rel _val_b)) {					\
			char _str_a[40], _str_b[40];				\
			snprintf(_str_a, sizeof(_str_a), _FMT(_a), _val_a);	\
			snprintf(_str_b, sizeof(_str_b), _FMT(_b), _val_b);	\
			test_assert_fail(__FILE__, __FUNCTION__, __LINE__,	\
			                 "\"%s %s %s\" (%s %s %s)",		\
			                 #_a, #_rel, #_b, _str_a, #_rel, _str_b);	\
		}								\
	} while(0)

#define TEST_ASSERT_GT(_a, _b) TEST_ASSERT_REL(_a, >, _b)
#define TEST_ASSERT_GTE(_a, _b) TEST_ASSERT_REL(_a, >=, _b)
#define TEST_ASSERT_LT(_a, _b) TEST_ASSERT_REL(_a, <, _b)
#define TEST_ASSERT_LTE(_a, _b) TEST_ASSERT_REL(_a, <=, _b)

#define TEST_ASSERT_PTR_EQ(_a, _b)						\
	do {									\
		const void *_val_a = (_a);					\
		const void *_val_b = (_b);					\
		if (!(_val_a == _val_b)) {					\
			test_assert_fail(__FILE__, __FUNCTION__, __LINE__,	\
			                 "\"%s == %s\" (%p == %p)",		\
			                 #_a, #_b, _val_a, _val_b);		\
		}								\
	} while(0)

#define TEST_ASSERT_PTR_NEQ(_a, _b)						\
	do {									\
		const void *_val_a = (_a);					\
		const void *_val_b = (_b);					\
		if (!(_val_a != _val_b)) {					\
			test_assert_fail(__FILE__, __FUNCTION__, __LINE__,	\
			                 "\"%s != %s\" (%p == %p)",		\
			                 #_a, #_b, _val_a, _val_b);		\
		}								\
	} while(0)

#define TEST_ASSERT_STR_EQ(_a, _b)						\
	do {									\
		const char *_val_a = (_a);					\
		const char *_val_b = (_b);					\
		if (_val_a == NULL || _val_b == NULL || strcmp(_val_a, _val_b) != 0) {		\
			test_assert_fail(__FILE__, __FUNCTION__, __LINE__,	\
			                 "\"strcmp(%s, %s) == 0\" (strcmp(\"%s\", \"%s\"))",	\
			                 #_a, #_b, _val_a, _val_b);		\
		}								\
	} while (0)


#define TEST_ASSERT_STR_NEQ(_a, _b)						\
	do {									\
		const char *_val_a = (_a);					\
		const char *_val_b = (_b);					\
		if (strcmp(_val_a, _val_b) == 0) {				\
			test_assert_fail(__FILE__, __FUNCTION__, __LINE__,	\
			                 "\"strcmp(%s, %s) != 0\" (!strcmp(%s, %s))",	\
			                 #_a, #_b, _val_a, _val_b);		\
		}								\
	} while (0)


#define TEST_ASSERT_MEM_EQ(_a, _b, _size)					\
	do {									\
		const uint8_t *_val_a = (const uint8_t *)(_a);			\
		const uint8_t *_val_b = (const uint8_t *)(_b);			\
		unsigned long long _val_size = (_size);				\
		unsigned long long _i;						\
		for (_i = 0; _i < _val_size; _i++) {				\
			if (_val_a[_i] != _val_b[_i])				\
				break;						\
		}								\
		if (_i != _val_size) {						\
			test_assert_fail(__FILE__, __FUNCTION__, __LINE__,	\
			                 "\"memcmp(%s, %s, %llu) == 0\" "	\
			                 "(differs at byte %llu, 0x%02x vs 0x%02x)",	\
			                 #_a, #_b, _val_size, _i, _val_a[_i], _val_b[_i]);	\
		}								\
	} while (0)


#define TEST_ASSERT_MEM_NEQ(_a, _b, _size)					\
	do {									\
		const void *_val_a = (const uint8_t *)(_a);			\
		const void *_val_b = (const uint8_t *)(_b);			\
		unsigned long long _val_size = (_size);				\
		if (memcmp(_val_a, _val_b, _val_size) == 0) {			\
			test_assert_fail(__FILE__, __FUNCTION__, __LINE__,	\
			                 "\"memcmp(%s, %s, %llu) != 0\"",	\
			                 #_a, #_b, _val_size);			\
		}								\
	} while (0)

#define TEST_ASSERT_NOT_NULL(_a)						\
	do {									\
		const void *_val = (const void *)_a;				\
		if (!(_val != NULL)) {						\
			test_assert_fail(__FILE__, __FUNCTION__, __LINE__,	\
			"\"%s != NULL\" (%p == NULL)", #_a, _val);		\
		}								\
	} while (0)

#define TEST_ASSERT_NULL(_a)							\
	do {									\
		const void *_val = (const void *)_a;				\
		if (!(_val == NULL)) {						\
			test_assert_fail(__FILE__, __FUNCTION__, __LINE__,	\
			"\"%s == NULL\" (%p == NULL)", #_a, _val);		\
		}								\
	} while (0)


#define TEST_EXPECT_PANIC() test_expect_panic(__FILE__, __FUNCTION__, __LINE__)

#define TEST_EXPECT_PANICKED() test_expect_panicked(__FILE__, __FUNCTION__, __LINE__)

#define TEST_FAIL(args...) test_assert_fail(__FILE__, __FUNCTION__, __LINE__, args)

// helper functions for the assertion macros above; don't call directly
void test_assert_fail(const char *file, const char *func, unsigned line, const char *fmt, ...) __noreturn;
void test_expect_panic(const char *file, const char *func, unsigned line);
void test_expect_panicked(const char *file, const char *func, unsigned line) __noreturn;

#endif
