#include <stdint.h>
#include <unittest.h>
#include <lib/blockdev.h>
#include <lib/fs.h>
#include <stdlib.h>
#include <string.h>

struct blockdev *create_mock_blockdev(const char *name, const char *filename, uint32_t block_size);

void test_fs(uintptr_t context) {
	struct blockdev *bdev;
	int result;
	int handle;
	void *buffer;
	size_t load_size;
	size_t buf_size;

	const char *filename = getenv("TESTFS_IMAGE");
	TEST_ASSERT_NOT_NULL(filename);

	bdev = create_mock_blockdev("dev", filename, 4096);
	register_blockdev(bdev);

	result = fs_mount("dev", "hfs", "/boot");
	TEST_ASSERT_EQ(result, 0);

	fs_dump_dir("/boot", true);
	fs_dump_dir("/boot/dir", true);

	// Opening a non-existant path should fail
	handle = fs_opendir("/boot/fake");
	TEST_ASSERT_LT(handle, 0);

	TEST_ASSERT_GTE((handle = fs_opendir("/boot")), 0);
	TEST_ASSERT_EQ(fs_closedir(handle), 0);

	TEST_ASSERT_GTE((handle = fs_opendir("/boot/a")), 0);
	TEST_ASSERT_EQ(fs_closedir(handle), 0);

	TEST_ASSERT_GTE((handle = fs_opendir("/boot/a/a")), 0);
	TEST_ASSERT_EQ(fs_closedir(handle), 0);

	TEST_ASSERT_GTE((handle = fs_opendir("/boot/bb")), 0);
	TEST_ASSERT_EQ(fs_closedir(handle), 0);

	TEST_ASSERT_GTE((handle = fs_opendir("/boot/bb/bb-a")), 0);
	TEST_ASSERT_EQ(fs_closedir(handle), 0);

	TEST_ASSERT_GTE((handle = fs_opendir("/boot/bb/bb-b")), 0);
	TEST_ASSERT_EQ(fs_closedir(handle), 0);

	TEST_ASSERT_GTE((handle = fs_opendir("/boot/ccc")), 0);
	TEST_ASSERT_EQ(fs_closedir(handle), 0);

	TEST_ASSERT_GTE((handle = fs_opendir("/boot/dddd")), 0);
	TEST_ASSERT_EQ(fs_closedir(handle), 0);

	TEST_ASSERT_GTE((handle = fs_opendir("/boot/eeeee")), 0);
	TEST_ASSERT_EQ(fs_closedir(handle), 0);

	TEST_ASSERT_GTE((handle = fs_opendir("/boot/ffffff")), 0);
	TEST_ASSERT_EQ(fs_closedir(handle), 0);

	TEST_ASSERT_GTE((handle = fs_opendir("/boot/ggggggg")), 0);
	TEST_ASSERT_EQ(fs_closedir(handle), 0);

	TEST_ASSERT_GTE((handle = fs_opendir("/boot/hhhhhhhh")), 0);
	TEST_ASSERT_EQ(fs_closedir(handle), 0);

	TEST_ASSERT_GTE((handle = fs_opendir("/boot/iiiiiiiii")), 0);
	TEST_ASSERT_EQ(fs_closedir(handle), 0);

	TEST_ASSERT_GTE((handle = fs_opendir("/boot/jjjjjjjjjj")), 0);
	TEST_ASSERT_EQ(fs_closedir(handle), 0);

	TEST_ASSERT_GTE((handle = fs_opendir("/boot/kkkkkkkkkkk")), 0);
	TEST_ASSERT_EQ(fs_closedir(handle), 0);

	TEST_ASSERT_GTE((handle = fs_opendir("/boot/lllllllllllllllllllllllllllllllllllll")), 0);
	TEST_ASSERT_EQ(fs_closedir(handle), 0);

	TEST_ASSERT_GTE((handle = fs_opendir("/boot/m")), 0);
	TEST_ASSERT_EQ(fs_closedir(handle), 0);

	TEST_ASSERT_GTE((handle = fs_opendir("/boot/n")), 0);
	TEST_ASSERT_EQ(fs_closedir(handle), 0);

	TEST_ASSERT_GTE((handle = fs_opendir("/boot/o")), 0);
	TEST_ASSERT_EQ(fs_closedir(handle), 0);

	TEST_ASSERT_GTE((handle = fs_opendir("/boot/p")), 0);
	TEST_ASSERT_EQ(fs_closedir(handle), 0);

	TEST_ASSERT_GTE((handle = fs_opendir("/boot/q")), 0);
	TEST_ASSERT_EQ(fs_closedir(handle), 0);

	TEST_ASSERT_GTE((handle = fs_opendir("/boot/dir")), 0);
	TEST_ASSERT_EQ(fs_closedir(handle), 0);

	buf_size = 1024 * 1024;
	buffer = malloc(buf_size);

	// Loading non-existant file should fail
	load_size = buf_size;
	result = fs_load_file("/boot/testfile1", (addr_t)buffer, &load_size);
	TEST_ASSERT_EQ(result, -1);

	// Load a file with 1 byte of zeroes
	load_size = buf_size;
	result = fs_load_file("/boot/dir/testfile1", (addr_t)buffer, &load_size);
	TEST_ASSERT_EQ(result, 0);
	TEST_ASSERT_EQ(load_size, 1);

	// Load a file with 2 bytes of zeroes
	load_size = buf_size;
	result = fs_load_file("/boot/dir/testfile2", (addr_t)buffer, &load_size);
	TEST_ASSERT_EQ(result, 0);
	TEST_ASSERT_EQ(load_size, 2);

	// Load a file with 512 bytes of zeroes
	load_size = buf_size;
	result = fs_load_file("/boot/dir/testfile512", (addr_t)buffer, &load_size);
	TEST_ASSERT_EQ(result, 0);
	TEST_ASSERT_EQ(load_size, 512);

	// Load a file with 1024 bytes of zeroes
	load_size = buf_size;
	result = fs_load_file("/boot/dir/testfile1024", (addr_t)buffer, &load_size);
	TEST_ASSERT_EQ(result, 0);
	TEST_ASSERT_EQ(load_size, 1024);

	// Load a file with a repeating data pattern
	const char *pattern = "evYya7KUSI1y5Tc05IQftxDuoIz8SlIRHDQLTA\n";
	size_t pattern_len = strlen(pattern);
	load_size = buf_size;
	result = fs_load_file("/boot/dir/testfile-evYya7KUSI1y5Tc05IQftxDuoIz8SlIRHDQLTA", (addr_t)buffer, &load_size);
	TEST_ASSERT_EQ(result, 0);
	TEST_ASSERT_EQ(load_size, pattern_len * 1024);
	char *current = buffer;
	for (unsigned i = 0; i < 1024; i++) {
		TEST_ASSERT_MEM_EQ(current, pattern, pattern_len);
		current += pattern_len;
	}

	// Verify a non-existant file is not found in a directory that exists
	load_size = buf_size;
	result = fs_load_file("/boot/dir/non-existant", (addr_t)buffer, &load_size);
	TEST_ASSERT_EQ(result, -1);

	// Verify a existant file is not found in the wrong directory
	load_size = buf_size;
	result = fs_load_file("/boot/non-existant/testfile1", (addr_t)buffer, &load_size);
	TEST_ASSERT_EQ(result, -1);

	free(buffer);
}

struct test_suite fs_suite = {
	.name = "fs",
	.test_cases = {
		{ "fs", test_fs, 0, "" },
		TEST_CASE_LAST
	}
};

TEST_SUITE(fs_suite);

// Mock 
bool
env_blacklist(const char *name, bool write)
{
        return false;
}

// Mock 
bool
env_blacklist_nvram(const char *name)
{
        return false;
}
