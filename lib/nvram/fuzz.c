#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <lib/blockdev.h>
#include <lib/env.h>
#include <lib/nvram.h>

struct blockdev *create_mock_blockdev(const char *name, const char *filename, uint32_t block_size);

int do_printenv(int argc, void *args);

int fuzz_main(const char *filename)
{
	struct blockdev *bdev;
	int result;

	bdev = create_mock_blockdev("nvram", filename, 1);
	if (bdev != NULL)
		register_blockdev(bdev);

	result = nvram_load();
	if (result != 0) {
		do_printenv(0, NULL);
		nvram_save();
	}

	return result;
}

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
