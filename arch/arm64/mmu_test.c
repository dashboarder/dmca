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

#include <stdbool.h>
#include <assert.h>
#include <inttypes.h>
#include <setjmp.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arch/arm/arm.h>
#include <arch/arm64/proc_reg.h>

#define VERBOSITY	FAILURE

#define FAILURE		0
#define INFO		1
#define SPEW		2

static size_t tests_run, tests_passed;

enum Reason {
	REASON_OK = 0,
	REASON_EXPECTED_PANIC,
	REASON_UNEXPECTED_PANIC,
	REASON_PARAMS,
	REASON_LEVEL,
	REASON_OOM,
	REASON_PTE_ALIGNMENT,
	REASON_PTE_BASE,
	REASON_PTE_INDEX,
	REASON_PTE_ACCESS,
	REASON_PTE_MISMATCH
};

static void test_unwind(enum Reason reason) __attribute__((noreturn));

#define TEST_ASSERT(reason, c)						\
	do {								\
		if (!(c)) {						\
			printf("!!! Test assertion failure: %s\n",	\
			       #c );					\
			test_unwind(reason);				\
		}							\
	} while (0)

struct State {
	// Environment constants for an individual test.
	const char *test_name;
	const char *expect_panic_func;
	const char *expect_panic_substr;
	size_t level_entries[4];
	void *page_tables_base;
	size_t page_tables_size;
	size_t total_stack_size;
	unsigned page_granule_shift;
	jmp_buf jmp_env;

	// Updated during test.
	bool arm_flush_tlbs_called;

	// Callbacks from test.
	void (^mmu_setup_func)();
	void (^get_table_entry_hook)();
	void (^set_table_entry_hook)();
};

static struct State state;

unsigned get_page_granule_shift(void) { return state.page_granule_shift; }

void mmu_get_memmap(addr_t *page_tables_base,
		    size_t *page_tables_size)
{
	*page_tables_base = (addr_t) state.page_tables_base;
	*page_tables_size = state.page_tables_size;
}

// Access to mmu.c static functions and variables.
uint64_t *mmu_get_tt_top_level(void);
unsigned mmu_get_start_level(void);
uint64_t mmu_get_tcr(void);
addr_t mmu_tt_alloc(unsigned level);
size_t mmu_get_level_entries(unsigned level);
uint64_t mmu_get_table_flags(void);
uint64_t mmu_get_block_flags(unsigned level, arm_mmu_extended_attr_t attr);
size_t mmu_map_at_level(unsigned level, addr_t table_base,
			addr_t vaddr, addr_t paddr, size_t size,
			arm_mmu_extended_attr_t attr);
void mmu_get_tt_alloc_range(addr_t *start, addr_t *next, addr_t *end);
void mmu_reset_state(void);

const char *reason_to_string(int reason)
{
	switch (reason) {
	case REASON_OK: return "OK";
	case REASON_EXPECTED_PANIC: return "Expected panic() called";
	case REASON_UNEXPECTED_PANIC: return "Unexpected panic() called";
	case REASON_PARAMS: return "Bad parameters detected";
	case REASON_LEVEL: return "Invalid level used";
	case REASON_OOM: return "Test ran out of memory for structures";
	case REASON_PTE_ALIGNMENT: return "Bad page table alignment";
	case REASON_PTE_BASE: return "Page table base does not match allocated";
	case REASON_PTE_INDEX: return "Page table index out of bounds";
	case REASON_PTE_ACCESS: return "Page table unexpected access";
	case REASON_PTE_MISMATCH: return "Page table value mismatch";
	default:
		printf("Bad reason code %d\n", (int) reason);
		abort();
	}
}

void mmu_printf(int verbosity, const char *fmt, ...)
{
	if (VERBOSITY >= verbosity) {
		va_list va;
		va_start(va, fmt);
		vprintf(fmt, va);
		va_end(va);
	}
}

static void test_unwind(enum Reason reason)
{
	assert(reason != REASON_OK);
	longjmp(state.jmp_env, (int) reason);
}

void _panic(const char *func, const char *fmt, ...)
{
	char *buf = malloc(4096);
	va_list va;

	va_start(va, fmt);
	vsnprintf(buf, 4096, fmt, va);
	va_end(va);

	bool expected = state.expect_panic_func && state.expect_panic_substr &&
		strstr(func, state.expect_panic_func) != NULL &&
		strstr(buf, state.expect_panic_substr) != NULL;

	if (!expected) {
		mmu_printf(FAILURE, "!!! Unexpected panic in function %s: %s\n", func, buf);
	} else {
		mmu_printf(INFO, "Expected panic in function %s: %s\n", func, buf);
	}

	free(buf);

	if (expected) {
		test_unwind(REASON_EXPECTED_PANIC);
	} else if (strstr(buf, "Out of table allocation space")) {
		test_unwind(REASON_OOM);
	} else {
		test_unwind(REASON_UNEXPECTED_PANIC);
	}
}

static void expect_panic(const char *func, const char *substr)
{
	state.expect_panic_func = func;
	state.expect_panic_substr = substr;
}

uint64_t check_valid_index(unsigned level, addr_t table_base, size_t index)
{
	if (level < mmu_get_start_level() || level > 3) {
		mmu_printf(FAILURE, "L%u invalid level\n", level);
		test_unwind(REASON_LEVEL);
	}
	void *p = NULL;

	size_t table_size =
		mmu_get_level_entries(level) * sizeof(uint64_t);
	size_t table_entries = mmu_get_level_entries(level);
	uint64_t alloc_start, alloc_next, alloc_end;
	mmu_get_tt_alloc_range(&alloc_start, &alloc_next, &alloc_end);
	if (table_base < alloc_start ||
	    table_size > alloc_next ||
	    table_base > alloc_next - table_size) {
		mmu_printf(FAILURE, "L%u base 0x%" PRIx64 " size 0x%" PRIx64
			   " outside allocated space\n",
			   level,
			   (uint64_t) table_base,
			   (uint64_t) table_size);
		test_unwind(REASON_PTE_BASE);
	}
	if (index > table_entries) {
		mmu_printf(FAILURE, "L%u entry %u >= table_entries %u\n",
			   level, (unsigned) index,
			   (unsigned) table_entries);
		test_unwind(REASON_PTE_INDEX);
	}
	p = (void *) (table_base + index * 8);

	uint64_t result;
	memcpy(&result, p, sizeof(result));
	return result;
}

static void get_table_entry_internal(unsigned level, addr_t table_base,
				     size_t index)
{
	mmu_printf(SPEW, "get_table_entry(%u, 0x%" PRIx64 ", %u) ",
		   level, (uint64_t) table_base, (unsigned) index);
	uint64_t result = check_valid_index(level, table_base, index);
	mmu_printf(SPEW, "= 0x%016" PRIx64 "\n", result);
}

void get_table_entry_hook(unsigned level, addr_t table_base, size_t index)
{
	if (state.get_table_entry_hook) {
		state.get_table_entry_hook(level, table_base, index);
		return;
	} else {
		get_table_entry_internal(level, table_base, index);
	}
}

static void set_table_entry_internal(unsigned level, addr_t table_base,
				     size_t index, uint64_t value)
{
	mmu_printf(SPEW, "set_table_entry(%u, 0x%" PRIx64 ", %u, "
		   "0x%" PRIx64 ") ",
		   level, (uint64_t) table_base, (unsigned) index,
		   value);
	uint64_t old = check_valid_index(level, table_base, index);
	mmu_printf(SPEW, " was 0x%016" PRIx64 "\n", old);
}

void set_table_entry_hook(unsigned level, addr_t table_base, size_t index,
			  uint64_t value)
{
	if (state.set_table_entry_hook) {
		state.set_table_entry_hook(level, table_base, index, value);
		return;
	} else {
		set_table_entry_internal(level, table_base, index, value);
	}
}

// Mock HAL.

uint64_t arm_read_sctlr(void)
{
	mmu_printf(INFO, "arm_read_sctlr()\n");
	return 0;
}

void arm_write_mair(uint64_t mair)
{
	mmu_printf(INFO, "arm_write_mair(0x%016" PRIx64 ")\n", (uint64_t) mair);
}

void arm_write_tcr(uint64_t tcr)
{
	mmu_printf(INFO, "arm_write_tcr(0x%016" PRIx64 ")\n", (uint64_t) tcr);
}

void arm_write_ttbr0(void *ttbr0)
{
	mmu_printf(INFO, "arm_write_ttbr0(%p)\n", ttbr0);
	if ((addr_t) ttbr0 != (addr_t) mmu_get_tt_top_level()) {
		mmu_printf(FAILURE, "TTBR0 written with %p expected %p\n",
		           ttbr0, mmu_get_tt_top_level());
		test_unwind(REASON_PTE_BASE);
	}
}

void arm_flush_tlbs(void)
{
	mmu_printf(INFO, "arm_flush_tlbs()\n");
	state.arm_flush_tlbs_called = true;
}

void platform_mmu_setup(bool resume)
{
	(void) resume;
	uint64_t alloc_start, alloc_next, alloc_end;
	mmu_get_tt_alloc_range(&alloc_start, &alloc_next, &alloc_end);
	mmu_printf(INFO, "platform_mmu_setup(%d)\n", (int) resume);
	mmu_printf(INFO, "tt_top_level = %p\n", mmu_get_tt_top_level());
	mmu_printf(INFO, "page_tables_base = %p\n", state.page_tables_base);
	mmu_printf(INFO, "page_tables_size = 0x%" PRIx64 "\n",
	           (uint64_t) state.page_tables_size);
	mmu_printf(INFO, "tt_alloc_start = 0x%" PRIx64 "\n"
	           "tt_alloc_next = 0x%" PRIx64 "\n"
	           "tt_alloc_end = 0x%" PRIx64 "\n",
	           (uint64_t) alloc_start,
	           (uint64_t) alloc_next,
	           (uint64_t) alloc_end);
	if (state.mmu_setup_func) {
		state.mmu_setup_func();
	}
}

// Test setup functions.

static void reset_state(void)
{
	mmu_reset_state();
	assert(state.page_tables_base == NULL);  // Prevent leak.
	memset(&state, 0, sizeof(state));
}

static void mmu_setup_flat(void)
{
	uint64_t io_base = 0x200000000ULL;
	uint64_t io_size = 256ULL * 1024 * 1024;
	uint64_t sdram_base = 0x800000000ULL;
	uint64_t sdram_len = 1ULL << 30;
	arm_mmu_map_rw(sdram_base, sdram_len);
	arm_mmu_map_device_rw(io_base, io_size);
}

static void mmu_setup_fail_remap_existing(void)
{
	uint64_t io_base = 0x200000000ULL;
	uint64_t io_size = 256ULL * 1024 * 1024;
	arm_mmu_map_rw(io_base, io_size);
	// Map again
	arm_mmu_map_device_rw(io_base, io_size);
}

static void mmu_setup_fail_breaking_down(void)
{
	uint64_t io_base = 0x200000000ULL;
	uint64_t io_size = 256ULL * 1024 * 1024;
	arm_mmu_map_rw(io_base, io_size);
	// Map one L3 page again
	arm_mmu_map_rw(io_base, 4096);
}

static void mmu_setup_misaligned(void)
{
	uint64_t sdram_base = 0x800000000ULL;
	uint64_t sdram_len = 1ULL << 30;
	arm_mmu_map_rw(sdram_base, sdram_len + 512);
}

static void test_delete(void)
{
	uint64_t alloc_start, alloc_next, alloc_end;
	mmu_get_tt_alloc_range(&alloc_start, &alloc_next, &alloc_end);
	mmu_printf(INFO, "Allocation space used: 0x%" PRIx64 "\n",
	           alloc_next - alloc_start);
	mmu_printf(INFO, "Allocation space remaining: 0x%" PRIx64 "\n",
	           alloc_end - alloc_next);
	if (state.page_tables_base) {
		free(state.page_tables_base);
		state.page_tables_base = NULL;
	}
	reset_state();
}

static int mmu_test(void)
{
	mmu_printf(INFO, "Test \"%s\" running\n", state.test_name);
	int ret = posix_memalign(&state.page_tables_base,
				 1 << state.page_granule_shift,
				 state.page_tables_size);
	assert(ret == 0);
	memset(state.page_tables_base, 0, state.page_tables_size);
	arm_mmu_init(false);
	// Calls back on platform_mmu_setup(resume)
	if (state.arm_flush_tlbs_called) {
		return true;
	} else {
		mmu_printf(FAILURE, "!!! arm_flush_tlbs() was not called\n");
		return false;
	}
}

static void do_test(enum Reason expect_reason, int (^test_func)(void))
{
	assert(state.test_name != NULL);
	mmu_printf(INFO, "----------------------------------------"
	                 "--------------------------------------\n");
	mmu_printf(INFO, "Test: %s\n", state.test_name);

	// Increment number of tests run.
	++tests_run;

	// Set a jump environment for panic/abort exit path.
	int reason_code = setjmp(state.jmp_env);
	if (reason_code != 0) {
		// Returns non-zero if longjmp() invoked to return here.
		if (reason_code == (int) expect_reason) {
			// Failure for an expected reason: exercising error
			// detection.
			mmu_printf(INFO, "Test \"%s\" aborted for expected reason: %s\n",
			           state.test_name,
			           reason_to_string(expect_reason));
			++tests_passed;
		} else {
			mmu_printf(FAILURE, "!!! Test \"%s\" failed: %s\n",
			           state.test_name,
			           reason_to_string(reason_code));
			mmu_printf(FAILURE, "!!! Expected outcome: %s\n",
			           reason_to_string(expect_reason));
		}
		test_delete();
		reset_state();
		return;
	}

	if (test_func()) {
		mmu_printf(INFO, "Test \"%s\" completed, passed\n",
		           state.test_name);
		++tests_passed;
	} else {
		mmu_printf(FAILURE, "!!! Test \"%s\" failed at completion\n",
		           state.test_name);
		mmu_printf(FAILURE, "!!! Expected outcome: %s\n",
		           reason_to_string(expect_reason));
	}

	test_delete();
	reset_state();
}

static void setup_standard_granule(size_t granule)
{
	switch (granule) {
	case 4096:
		state.page_granule_shift = 12;
		state.page_tables_size = 4096 * 4;
		break;

	case 16384:
		state.page_granule_shift = 14;
		state.page_tables_size = 16384 * 4;
		break;

	case 65536:
		state.page_granule_shift = 16;
		state.page_tables_size = 65536 * 4;
		break;

	default:
		mmu_printf(FAILURE, "Bad granule %u\n", (unsigned) granule);
		abort();
	}
}

static void test_granule(size_t bytes)
{
	state.mmu_setup_func = ^{ mmu_setup_flat(); };
	setup_standard_granule(bytes);
	do_test(REASON_OK, ^{ return mmu_test(); });
}

static void test_granule_4kb(void)
{
	state.test_name = "4KB granule";
	test_granule(4096);
}

static void test_granule_16kb(void)
{
	state.test_name = "16KB granule";
	test_granule(16384);
}

static void test_granule_64kb(void)
{
	state.test_name = "64KB granule";
	test_granule(65536);
}

static void test_fail_oom(void)
{
	state.test_name = "Detect out-of-memory PTE allocation";
	state.mmu_setup_func = ^{ mmu_setup_flat(); };
	setup_standard_granule(4096);
	// Too small.
	state.page_tables_size = 16384;
	expect_panic("tt_alloc", "Out of table allocation space L2");
	do_test(REASON_EXPECTED_PANIC, ^{ return mmu_test(); });
}

static void test_fail_range_misaligned(void)
{
	state.test_name = "Detect misaligned mapping request";
	state.mmu_setup_func = ^{ mmu_setup_misaligned(); };
	setup_standard_granule(4096);
	// Expect vaddr, paddr, size to misaligned.
	expect_panic("arm_mmu_map_range", "vaddr | paddr | size");
	do_test(REASON_EXPECTED_PANIC, ^{ return mmu_test(); });
}

static void test_fail_bad_granule(void)
{
	state.test_name = "Detect bad granule size";
	state.mmu_setup_func = ^{
		// Alter the granule to an illegal value.
		++state.page_granule_shift;
		mmu_setup_flat();
	};
	setup_standard_granule(4096);
	expect_panic("get_level_shift", "Granule 2^13");
	do_test(REASON_EXPECTED_PANIC, ^{ return mmu_test(); });
}

static void test_fail_remap_existing(void)
{
	state.test_name = "Detect remapping existing page";
	state.mmu_setup_func = ^{ mmu_setup_fail_remap_existing(); };
	setup_standard_granule(4096);
	expect_panic("map_at_level", "Remapping an existing L2");
	do_test(REASON_EXPECTED_PANIC, ^{ return mmu_test(); });
}

static void test_fail_breaking_down(void)
{
	state.test_name = "Detect breaking down existing mapping";
	state.mmu_setup_func = ^{ mmu_setup_fail_breaking_down(); };
	setup_standard_granule(4096);
	expect_panic("map_at_level", "Breaking down");
	do_test(REASON_EXPECTED_PANIC, ^{ return mmu_test(); });
}

static void test_get_tcr(void)
{
	state.test_name = "get_tcr()";
	do_test(REASON_OK,
		^{
			state.page_granule_shift = 12;
			mmu_get_tcr();
			state.page_granule_shift = 14;
			mmu_get_tcr();
			state.page_granule_shift = 16;
			mmu_get_tcr();
			return true;
		});
}

static void test_fail_get_tcr_bad_granule(void)
{
	state.test_name = "Detect get_tcr() with bad granule";
	expect_panic("get_tcr", "Bad granule");
	state.page_granule_shift = 13;
	do_test(REASON_EXPECTED_PANIC, ^{ mmu_get_tcr(); return false; });
}

static void test_get_block_flags_for_level(unsigned level)
{
	// [63:59] SBZ
	// [58:55] S/W use SBZ
	// [54:53] XN,PXN = execute never, 0 for RX, 3 for R/RW
	// [52]    Contiguous hint, SBZ
	// [51:12] SBZ and address bits, also SBZ
	// [11]    nG - non-global, we don't use ASID - SBZ
	// [10]    AF - access flag, should set - SBO
	// [9:8]   SH - shareability - 0 for device, 2 for normal
	// [7:6]   AP - access permission - 0 for RW, 2 for RO
	// [5]     NS - non-secure, should set - SBO
	// [4:2]   AttrIndx - 0 for device, 1 for normal
	// [1]     Walk - L0,L1,L2 0=Block; L3 1=Block
	// [0]     V - 0=fault, 1=valid
	const uint64_t sbo = (1 << 10) | (1 << 5) | (1 << 0);
	const uint64_t normal = (2 << 8) | (1 << 2);
	const uint64_t device = (0 << 8) | (0 << 2);
	const uint64_t read_write = 0 << 6;
	const uint64_t read_only = 2 << 6;
	const uint64_t execute = 0ULL << 53;
	const uint64_t no_execute = 3ULL << 53;
	static const struct {
		arm_mmu_extended_attr_t attr;
		uint64_t expect;
	} tests[] = {
		{ kARMMMUDeviceR, sbo | device | read_only | no_execute },
		{ kARMMMUDeviceRX, sbo | device | read_only | execute },
		{ kARMMMUDeviceRW, sbo | device | read_write | no_execute },
		{ kARMMMUNormalR, sbo | normal | read_only | no_execute },
		{ kARMMMUNormalRX, sbo | normal | read_only | execute },
		{ kARMMMUNormalRW, sbo | normal | read_write | no_execute },
	};
	for (size_t i = 0; i < sizeof(tests) / sizeof(tests[0]); ++i) {
		arm_mmu_extended_attr_t attr = tests[i].attr;
		uint64_t expect = tests[i].expect;
		if (level == 3) {
			expect |= 1 << 1;  // "Walk" flag
		}
		uint64_t value = mmu_get_block_flags(level, attr);
		if (value != expect) {
			mmu_printf(FAILURE, "get_block_flags level %u test %u enum %u\n",
			           level, (unsigned) i, (unsigned) attr);
			mmu_printf(FAILURE, "got 0x%" PRIx64 "\n", value);
			mmu_printf(FAILURE, "expect 0x%" PRIx64 "\n", expect);
			test_unwind(REASON_PTE_MISMATCH);
		}
	}
}

static void test_get_block_flags(void)
{
	state.test_name = "get_block_flags()";
	state.page_granule_shift = 12;
	do_test(REASON_OK, ^{
			for (unsigned level = 1; level <= 3; ++level) {
				test_get_block_flags_for_level(level);
			}
			return true;
		});
}

static void test_get_table_flags(void)
{
	state.test_name = "get_tte_flags()";
	state.page_granule_shift = 12;
	// [63]    NS
	// [62:61] AP
	// [60]    XN
	// [59]    PXN
	// [58:52] ignored
	// [51:48] zero, sbz
	// [47:12] TableOutputAddress
	// [11:2]  ignored
	// [1]     type=1, sbo
	// [0]     valid=1, sbo
	const uint64_t expect = (1ULL << 63) | (1ULL << 1) | (1ULL << 0);
	do_test(REASON_OK, ^{
			uint64_t value = mmu_get_table_flags();
			if (value != expect) {
				mmu_printf(FAILURE, "get_table_flags() mismatch\n");
				mmu_printf(FAILURE, "Got 0x%" PRIx64 "\n", value);
				mmu_printf(FAILURE, "Expect 0x%" PRIx64 "\n", expect);
				test_unwind(REASON_PTE_MISMATCH);
			}
			return true;
		});
}

static void test_fail_get_block_flags_bad_attr(void)
{
	state.test_name = "Detect get_block_flags() bad attribute";
	state.page_granule_shift = 12;
	expect_panic("get_block_flags", "Bad attr");
	do_test(REASON_EXPECTED_PANIC,
		^{ mmu_get_block_flags(3, -1); return false; });
}

static void test_fail_get_block_flags_bad_level(void)
{
	state.test_name = "Detect get_pte_flags() bad level";
	state.page_granule_shift = 16;
	expect_panic("get_block_flags", "level_permits_blocks");
	// Should not allow a block at L1 with 64KB granule.
	do_test(REASON_EXPECTED_PANIC,
		^{ mmu_get_block_flags(1, kARMMMUNormalRW); return false; });
}

static void test_tt_alloc(unsigned granule_shift)
{
	state.test_name = "tt_alloc()";
	state.page_granule_shift = granule_shift;
	state.page_tables_size = (1 << granule_shift) * 3;
	state.mmu_setup_func = ^{};
	do_test(REASON_OK,
		^{
			arm_mmu_init(false);
			if (granule_shift == 12)
				mmu_tt_alloc(2);
			mmu_tt_alloc(3);
			return true;
		});
}

static void test_tt_alloc_bad_level(void)
{
	state.test_name = "Detect tt_alloc() bad level";
	state.page_granule_shift = 14;
	state.page_tables_size = 16384 * 3;
	state.mmu_setup_func = ^{};
	expect_panic("tt_alloc", "level > ");
	do_test(REASON_EXPECTED_PANIC,
		^{
			arm_mmu_init(false);
			mmu_tt_alloc(1);
			return false;
		});
}

static void test_tt_alloc_bad_alignment(void)
{
	state.test_name = "Detect tt_alloc() bad alignment";
	state.page_granule_shift = 14;
	state.page_tables_size = 65536 * 3;  // Enough space.
	state.mmu_setup_func = ^{};
	expect_panic("tt_alloc", "Bad aligned pte alloc");
	do_test(REASON_EXPECTED_PANIC,
		^{
			arm_mmu_init(false);
			mmu_tt_alloc(3);
			// Allocation is aligned, so only way to trip it up
			// is to artificially tamper with the granule.
			state.page_granule_shift = 16;
			mmu_tt_alloc(3);
			return false;
		});
}

static void test_map_at_top_level_large(void)
{
	state.test_name = "map_at_level() at top level traverses only L1";
	setup_standard_granule(4096);
	__block uint64_t alloc_start, alloc_next, alloc_end;
	state.mmu_setup_func = ^{
		mmu_get_tt_alloc_range(&alloc_start, &alloc_next, &alloc_end);
		mmu_map_at_level(1,
				 (addr_t) mmu_get_tt_top_level(),
				 1ULL << 30, 1ULL << 30, 1ULL << 30,
				 kARMMMUNormalRW);
	};
	__block size_t calls = 0;
	state.set_table_entry_hook = ^(unsigned level,
				       uint64_t base,
				       size_t index,
				       uint64_t value) {
		(void) value;
		++calls;
		mmu_printf(INFO, "calls:%u %u 0x%" PRIx64 " 0x%" PRIx64
		           " 0x%" PRIx64 "\n",
		           (unsigned) calls,
		           level, base, (uint64_t) index, value);
		if (calls == 1) {
			if (level != 1 ||
			    base != (uint64_t) mmu_get_tt_top_level() ||
			    index != 1) {
				mmu_printf(FAILURE, "Did not write L0 index 0\n");
				test_unwind(REASON_PTE_ACCESS);
			}
		} else {
			mmu_printf(FAILURE, "Too many calls to get_table_entry()\n");
			test_unwind(REASON_PTE_ACCESS);
		}
	};
	do_test(REASON_OK,
		^{
			if (!mmu_test()) {
				return false;
			}
			return true;
		});
}

static void test_map_at_l3(void)
{
	state.test_name = "map_at_level() at l3";
	setup_standard_granule(16384);
	__block uint64_t alloc_start = 0, alloc_next = 0, alloc_end = 0;
	__block uint64_t l3_base = 0;
	state.mmu_setup_func = ^{
		mmu_get_tt_alloc_range(&alloc_start, &alloc_next, &alloc_end);
		l3_base = mmu_tt_alloc(3);
		mmu_map_at_level(3, l3_base,
				 (1ULL << 30) + 16384 * 3,
				 16384 * 7,
				 16384,
				 kARMMMUDeviceRW);
	};
	__block size_t calls = 0;
	state.set_table_entry_hook = ^(unsigned level,
				       uint64_t base,
				       size_t index,
				       uint64_t value) {
		(void) value;
		++calls;
		mmu_printf(INFO, "calls:%u %u 0x%" PRIx64 " 0x%" PRIx64
		           " 0x%" PRIx64 "\n",
		           (unsigned) calls,
		           level, base, (uint64_t) index, value);
		if (calls == 1) {
			if (level != 3 ||
			    base != l3_base ||
			    index != 3) {
				mmu_printf(FAILURE, "Did not write L3 index 3\n");
				test_unwind(REASON_PTE_ACCESS);
			}
		} else {
			mmu_printf(FAILURE, "Too many calls to get_table_entry()\n");
			test_unwind(REASON_PTE_ACCESS);
		}
	};
	do_test(REASON_OK,
		^{
			if (!mmu_test()) {
				return false;
			}
			return true;
		});
}

int main()
{
	reset_state();

	test_granule_4kb();
	test_granule_16kb();
	test_granule_64kb();

	test_fail_oom();
	test_fail_range_misaligned();
	test_fail_bad_granule();

	test_fail_remap_existing();
	test_fail_breaking_down();

	test_get_tcr();
	// test_fail_get_tcr_bad_granule();	!!!FIXME!!! <rdar://problem/19225114> Unexpected panic in function get_virt_address_bits: Bad granule

	test_get_block_flags();
	test_get_table_flags();
	test_fail_get_block_flags_bad_attr();
	test_fail_get_block_flags_bad_level();

	test_tt_alloc(12);
	test_tt_alloc(14);
	test_tt_alloc(16);
	test_tt_alloc_bad_level();
	test_tt_alloc_bad_alignment();

	test_map_at_top_level_large();
	test_map_at_l3();

	mmu_printf(INFO, "%" PRIu64 "/%" PRIu64 " tests passed\n",
	           (uint64_t) tests_passed, (uint64_t) tests_run);
	assert(tests_passed <= tests_run);
	if (tests_passed == tests_run) {
		mmu_printf(INFO, "Pass\n");
		return 0;
	} else {
		mmu_printf(INFO, "!!! Fail\n");
		return 1;
	}
}
