#include <stdint.h>
#include <unittest.h>
#include <lib/blockdev.h>
#include <lib/env.h>
#include <lib/nvram.h>
#include <stdlib.h>
#include <string.h>

// There aren't any useful test cases here yet, as the nvram
// testing infrastructure was mainly added to allow fuzzing.
// For unit testing, we need a sample NVRAM and then some
// getenv calls

// For now, this just exercises the the "empty NVRAM" case
static void test_nvram_load(uintptr_t ctx)
{
	struct blockdev *bdev;
	int result;
	void *backing;

	backing = calloc(8192, 1);

	bdev = create_mem_blockdev("nvram", backing, 8192, 4096);
	if (bdev != NULL)
		register_blockdev(bdev);

	result = nvram_load();
}

struct test_suite nvram_suite = {
	.name = "nvram",
	.test_cases = {
		{ "nvram_load", test_nvram_load, 0, "" },
		TEST_CASE_LAST
	}
};

TEST_SUITE(nvram_suite);

// Mock for lib/env
bool
env_blacklist(const char *name, bool write)
{
        return false;
}

bool
env_blacklist_nvram(const char *name)
{
        return false;
}
