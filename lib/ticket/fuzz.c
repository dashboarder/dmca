#include <unittest.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>

#include <platform.h>
#include <lib/blockdev.h>
#include <lib/DERApTicket.h>
#include <lib/mib.h>
#include <lib/ticket.h>
#include <lib/pki.h>
#include <arch.h>
#include <AssertMacros.h>

struct blockdev *create_mock_blockdev(const char *name, const char *filename, uint32_t block_size);

int do_printenv(int argc, void *args);

int fuzz_main(const char *filename)
{
	void *in_buffer;
	size_t in_size;
	struct stat st;
	FILE *f;
	int result;
	struct blockdev *bdev;

	// Get input size
	result = stat(filename, &st);
	TEST_ASSERT_EQ(result, 0);
	in_size = st.st_size;

	// Read it
	f = fopen(filename, "r");
	TEST_ASSERT_NOT_NULL(f);
	in_buffer = malloc(in_size);
	TEST_ASSERT_NOT_NULL(in_buffer);
	result = fread(in_buffer, 1, in_size, f);
	TEST_ASSERT_EQ(result, in_size);

	bdev = create_mem_blockdev("nand", in_buffer, in_size, 1);
	TEST_ASSERT_NOT_NULL(bdev);

	image_search_bdev(bdev, 0, 0);

	ticket_load();

	free(in_buffer);

	return 0;
}

// mocks

uint32_t platform_get_board_id(void)
{
	return 0;
}

uint32_t platform_get_chip_id(void)
{
	return 0x8747;
}

uint64_t platform_get_ecid_id(void)
{
	return 0;
}

uint64_t platform_get_nonce(void)
{
	return 0;
}

uint32_t platform_get_security_domain(void)
{
	return 1;
}

bool platform_get_raw_production_mode(void)
{
	return false;
}

uint32_t platform_get_hardware_epoch(void)
{
	return 0;
}

uint32_t platform_get_security_epoch(void)
{
	return 0;
}

const char *build_tag_string = "iBoot-1234";

MIB_CONSTANT_PTR(kMIBBuildTag,    kOIDTypeString|kOIDDataIndirect, (void *)&build_tag_string);

struct mib_description mib_desc[] = {};

int fs_load_file(const char *path, addr_t addr, size_t *maxsize)
{
	return -1;
}

bool security_validate_image(security_image_t image_validity)
{
	return true;
}

bool platform_get_ecid_image_personalization_required(void)
{
	return false;
}

void security_set_production_override(bool override)
{
}

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
