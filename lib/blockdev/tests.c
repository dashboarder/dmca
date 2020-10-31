#include <unittest.h>
#include <lib/blockdev.h>
#include <string.h>
#include <stdlib.h>

struct mock_blockdev {
	struct blockdev bdev;

	/* local data */
	void *ptr;
};

static int mock_blockdev_read_block(struct blockdev *bdev, void *ptr, block_addr block, uint32_t count)
{
	struct mock_blockdev *mdev = (struct mock_blockdev *)bdev;

	// no overflow
	TEST_ASSERT_GTE(block + count, block);

	// buffer must be properly aligned
	TEST_ASSERT_EQ((uintptr_t)ptr & (bdev->alignment - 1), 0);

	if (block >= mdev->bdev.block_count)
		return 0;
	if ((block + count) > mdev->bdev.block_count)
		count = mdev->bdev.block_count - block;

	memcpy(ptr, (uint8_t *)mdev->ptr + (block << mdev->bdev.block_shift), count << mdev->bdev.block_shift);

	return count;
}

static int mock_blockdev_write_block(struct blockdev *bdev, const void *ptr, block_addr block, uint32_t count)
{
	struct mock_blockdev *mdev = (struct mock_blockdev *)bdev;

	// no overflow
	TEST_ASSERT_GTE(block + count, block);

	// buffer must be properly aligned
	TEST_ASSERT_EQ((uintptr_t)ptr & (bdev->alignment - 1), 0);

	if (block >= mdev->bdev.block_count)
		return 0;
	if ((block + count) > mdev->bdev.block_count)
		count = mdev->bdev.block_count - block;

	memcpy(mdev->ptr + (block << mdev->bdev.block_shift), ptr, count << mdev->bdev.block_shift);

	return count;
}

struct blockdev *create_mock_blockdev(const char *name, void *ptr, uint64_t len, uint32_t block_size)
{
	struct mock_blockdev *mdev;

	mdev = malloc(sizeof(struct mock_blockdev));

	construct_blockdev(&mdev->bdev, name, len, block_size);
	mdev->ptr = ptr;

	mdev->bdev.read_block_hook = &mock_blockdev_read_block;
	mdev->bdev.write_block_hook = &mock_blockdev_write_block;

	return &mdev->bdev;
}

static void do_blockdev_read_tests(struct blockdev *bdev, uint8_t *backing, size_t backing_size, uint32_t block_size)
{
	uint8_t *readback;
	uint8_t *zeroes;
	int result;

	readback = malloc(backing_size + 1);
	TEST_ASSERT_NOT_NULL(readback);
	zeroes = malloc(backing_size);
	TEST_ASSERT_NOT_NULL(zeroes);

	memset(zeroes, 0, backing_size);

	// Reading entire buffer should work
	memset(readback, 0, backing_size);
	result = blockdev_read(bdev, readback, 0, backing_size);
	TEST_ASSERT_EQ(result, backing_size);
	TEST_ASSERT_MEM_EQ(backing, readback, backing_size);

	// Reading entire buffer unaligned should work
	memset(readback + 1, 0, backing_size);
	result = blockdev_read(bdev, readback + 1, 0, backing_size);
	TEST_ASSERT_EQ(result, backing_size);
	TEST_ASSERT_MEM_EQ(backing, readback + 1, backing_size);

	// Reading past the end results in a short read
	memset(readback, 0, backing_size);
	result = blockdev_read(bdev, readback, backing_size / 2, backing_size);
	TEST_ASSERT_EQ(result, backing_size / 2);
	// first half of buffer should have result of read
	TEST_ASSERT_MEM_EQ(readback, backing + backing_size / 2, backing_size / 2);
	// second half should be untouched
	TEST_ASSERT_MEM_EQ(readback + backing_size / 2, zeroes, backing_size / 2);
	
	// Reading totally past the end results in no read
	memset(readback, 0, backing_size);
	result = blockdev_read(bdev, readback, backing_size + 2, backing_size);
	TEST_ASSERT_EQ(result, 0);
	// buffer should be totally untouched
	TEST_ASSERT_MEM_EQ(readback, zeroes, backing_size);

	// Reads that are weird size should work
	uint32_t read_sizes[] = {1, 2, 3, 13, 127, 129, 1023, 1025, 4094, 4097, block_size, block_size - 1, block_size - 2, backing_size - 2, backing_size - 1};
	for (uint32_t i = 0; i < sizeof(read_sizes) / sizeof(read_sizes[0]); i++) {
		uint32_t read_size = read_sizes[i];
		if (read_size == 0 || read_size >= backing_size)
			continue;

		uint32_t read_offsets[] = {0, 1, 2, 3, 13, 127, 129, 1023, 1025, 4094, 4097, block_size, block_size - 1, block_size - 2, backing_size - 2, backing_size - 1};
		for (uint32_t j = 0; j < sizeof(read_offsets) / sizeof(read_offsets[0]); j++) {
			uint32_t read_offset = read_offsets[j];
			if (read_offset >= backing_size || read_offset + read_size >= backing_size)
				continue;
			tprintf(TEST_INFO, "doing offset=%u size=%u\n", read_offset, read_size);

			// make sure the read succeeds
			memset(readback + 1, 0, backing_size);
			result = blockdev_read(bdev, readback, read_offset, read_size);
			TEST_ASSERT_EQ(result, read_size);
			TEST_ASSERT_MEM_EQ(readback, backing + read_offset, read_size);
			TEST_ASSERT_MEM_EQ(readback + read_size, zeroes, backing_size - read_size);

			// make sure read succeeds with alignment forced off
			memset(readback + 1, 0, backing_size);
			result = blockdev_read(bdev, readback + 1, read_offset, read_size);
			TEST_ASSERT_EQ(result, read_size);
			TEST_ASSERT_MEM_EQ(readback + 1, backing + read_offset, read_size);
			TEST_ASSERT_MEM_EQ(readback + 1 + read_size, zeroes, backing_size - read_size);
		}
	}

	free(readback);
	free(zeroes);
}

static void do_blockdev_read_block_tests(struct blockdev *bdev, uint8_t *backing, size_t backing_size, uint32_t block_size)
{
	uint8_t *readback;
	uint8_t *filler;
	int result;
	uint32_t blocks = backing_size / block_size;

	readback = malloc(backing_size);
	TEST_ASSERT_NOT_NULL(readback);
	filler = malloc(backing_size + 1);
	TEST_ASSERT_NOT_NULL(filler);

	memset(filler, 0xee, backing_size);

	// Reading entire buffer should work
	memcpy(readback, filler, backing_size);
	result = blockdev_read_block(bdev, readback, 0, blocks);
	TEST_ASSERT_EQ(result, blocks);
	TEST_ASSERT_MEM_EQ(backing, readback, backing_size);

	// Reading past the end results in a short read
	memcpy(readback, filler, backing_size);
	result = blockdev_read_block(bdev, readback, blocks / 2, blocks);
	TEST_ASSERT_EQ(result, blocks / 2);
	// first half of buffer should have result of read
	TEST_ASSERT_MEM_EQ(readback, backing + backing_size / 2, backing_size / 2);
	// second half should be untouched
	TEST_ASSERT_MEM_EQ(readback + backing_size / 2, filler, backing_size / 2);
	
	// Reading totally past the end results in no read
	memcpy(readback, filler, backing_size);
	result = blockdev_read_block(bdev, readback, blocks + 2, blocks);
	TEST_ASSERT_EQ(result, 0);
	// buffer should be totally untouched
	TEST_ASSERT_MEM_EQ(readback, filler, backing_size);

	free(readback);
	free(filler);
}

void test_mem_blockdev(uintptr_t param)
{
	struct blockdev *bdev;
	uint8_t *backing;
	size_t backing_size = 8192;
	uint32_t block_size = param;

	backing = malloc(backing_size);
	TEST_ASSERT_NOT_NULL(backing);
	for (uint32_t i = 0; i < backing_size / sizeof(uint32_t); i++) {
		uint32_t val = i + 0xaabbccdd;
		memcpy(backing + i * sizeof(uint32_t), &val, sizeof(uint32_t));
	}
	bdev = create_mem_blockdev("dev", backing, backing_size, block_size);

	do_blockdev_read_tests(bdev, backing, backing_size, block_size);
	do_blockdev_read_block_tests(bdev, backing, backing_size, block_size);

	// make sure backing wasn't touched during all the tests
	for (unsigned i = 0; i < backing_size / sizeof(uint32_t); i++) {
		uint32_t val;
		memcpy(&val, backing + i * sizeof(uint32_t), sizeof(uint32_t));
		TEST_ASSERT_EQ(val, i + 0xaabbccdd);
	}

	free(backing);
}

void test_blockdev_read_block(uintptr_t param)
{
	struct blockdev *bdev;
	uint8_t *backing;
	size_t backing_size = 8192;
	uint32_t block_size = param;

	backing = malloc(backing_size);
	TEST_ASSERT_NOT_NULL(backing);
	for (uint32_t i = 0; i < backing_size / sizeof(uint32_t); i++) {
		uint32_t val = i + 0xaabbccdd;
		memcpy(backing + i * sizeof(uint32_t), &val, sizeof(uint32_t));
	}
	bdev = create_mock_blockdev("dev1", backing, backing_size, block_size);
	do_blockdev_read_block_tests(bdev, backing, backing_size, block_size);

	// make sure backing wasn't touched during all the tests
	for (unsigned i = 0; i < backing_size / sizeof(uint32_t); i++) {
		uint32_t val;
		memcpy(&val, backing + i * sizeof(uint32_t), sizeof(uint32_t));
		TEST_ASSERT_EQ(val, i + 0xaabbccdd);
	}

	free(backing);
}

void test_blockdev_read(uintptr_t param)
{
	struct blockdev *bdev;
	uint8_t *backing;
	size_t backing_size = 8192;
	uint32_t block_size = param;

	backing = malloc(backing_size);
	TEST_ASSERT_NOT_NULL(backing);
	for (uint32_t i = 0; i < backing_size / sizeof(uint32_t); i++) {
		uint32_t val = i + 0xaabbccdd;
		memcpy(backing + i * sizeof(uint32_t), &val, sizeof(uint32_t));
	}
	bdev = create_mock_blockdev("dev1", backing, backing_size, block_size);
	do_blockdev_read_tests(bdev, backing, backing_size, block_size);

	// make sure backing wasn't touched during all the tests
	for (unsigned i = 0; i < backing_size / sizeof(uint32_t); i++) {
		uint32_t val;
		memcpy(&val, backing + i * sizeof(uint32_t), sizeof(uint32_t));
		TEST_ASSERT_EQ(val, i + 0xaabbccdd);
	}

	free(backing);
}

void test_blockdev_set_buffer_alignment_read(uintptr_t param)
{
	struct blockdev *bdev;
	uint8_t *backing;
	size_t backing_size = 8192;
	uint32_t block_size = param;

	backing = malloc(backing_size);
	TEST_ASSERT_NOT_NULL(backing);
	for (uint32_t i = 0; i < backing_size / sizeof(uint32_t); i++) {
		uint32_t val = i + 0xaabbccdd;
		memcpy(backing + i * sizeof(uint32_t), &val, sizeof(uint32_t));
	}

	bdev = create_mock_blockdev("dev1", backing, backing_size, block_size);
	blockdev_set_buffer_alignment(bdev, block_size);
	do_blockdev_read_tests(bdev, backing, backing_size, block_size);

	if (block_size > 2) {
		bdev = create_mock_blockdev("dev2", backing, backing_size, block_size);
		blockdev_set_buffer_alignment(bdev, block_size / 2);
		do_blockdev_read_tests(bdev, backing, backing_size, block_size);
	}

	// make sure backing wasn't touched during all the tests
	for (unsigned i = 0; i < backing_size / sizeof(uint32_t); i++) {
		uint32_t val;
		memcpy(&val, backing + i * sizeof(uint32_t), sizeof(uint32_t));
		TEST_ASSERT_EQ(val, i + 0xaabbccdd);
	}

	free(backing);
}

static void do_blockdev_write_tests(struct blockdev *bdev, uint8_t *backing, size_t backing_size, uint32_t block_size)
{
	uint8_t *data;
	uint8_t *filler;
	int result;

	// need some source data we can write to the blockdev with
	// easily recognizable patterns
	data = malloc(backing_size + sizeof(uint32_t));
	TEST_ASSERT_NOT_NULL(data);
	for (uint32_t i = 0; i < backing_size / sizeof(uint32_t) + 1; i++) {
		uint32_t val = i + 0xaabbccdd;
		memcpy(data + i * sizeof(uint32_t), &val, sizeof(uint32_t));
	}

	filler = malloc(backing_size);
	TEST_ASSERT_NOT_NULL(filler);

	memset(filler, 0xee, backing_size);

	// Writing entire buffer should work
	memcpy(backing, filler, backing_size);
	result = blockdev_write(bdev, data, 0, backing_size);
	TEST_ASSERT_EQ(result, backing_size);
	TEST_ASSERT_MEM_EQ(backing, data, backing_size);

	// Writing entire buffer unaligned should work
	memcpy(backing, filler, backing_size);
	result = blockdev_write(bdev, data + 1, 0, backing_size);
	TEST_ASSERT_EQ(result, backing_size);
	TEST_ASSERT_MEM_EQ(backing, data + 1, backing_size);

	// Writing past the end results in a short write
	memcpy(backing, filler, backing_size);
	result = blockdev_write(bdev, data, backing_size / 2, backing_size);
	TEST_ASSERT_EQ(result, backing_size / 2);
	// first half of backing should be untouched
	TEST_ASSERT_MEM_EQ(backing, filler, backing_size / 2);
	// second half of buffer should have result of write
	TEST_ASSERT_MEM_EQ(backing + backing_size / 2, data, backing_size / 2);
	
	// Reading totally past the end results in no write
	memcpy(backing, filler, backing_size);
	result = blockdev_write(bdev, data, backing_size + 2, backing_size);
	TEST_ASSERT_EQ(result, 0);
	// buffer should be totally untouched
	TEST_ASSERT_MEM_EQ(backing, filler, backing_size);

	//  weird size writes should work
	uint32_t write_sizes[] = {1, 2, 3, 13, 127, 129, 1023, 1025, 4094, 4097, block_size, block_size - 1, block_size - 2, backing_size - 2, backing_size - 1};
	for (uint32_t i = 0; i < sizeof(write_sizes) / sizeof(write_sizes[0]); i++) {
		uint32_t write_size = write_sizes[i];
		if (write_size == 0 || write_size >= backing_size)
			continue;

		uint32_t write_offsets[] = {0, 1, 2, 3, 13, 127, 129, 1023, 1025, 4094, 4097, block_size, block_size - 1, block_size - 2, backing_size - 2, backing_size - 1};
		for (uint32_t j = 0; j < sizeof(write_offsets) / sizeof(write_offsets[0]); j++) {
			uint32_t write_offset = write_offsets[j];
			if (write_offset >= backing_size || write_offset + write_size >= backing_size)
				continue;
			tprintf(TEST_INFO, "doing offset=%u size=%u\n", write_offset, write_size);

			memcpy(backing, filler, backing_size);
			result = blockdev_write(bdev, data, write_offset, write_size);
			TEST_ASSERT_EQ(result, write_size);
			TEST_ASSERT_MEM_EQ(backing + write_offset, data, write_size);
			// data before and after write should be untouched
			TEST_ASSERT_MEM_EQ(backing, filler, write_offset);
			TEST_ASSERT_MEM_EQ(backing + write_offset + write_size, filler, backing_size - write_size - write_offset);
		}
	}

	free(data);
	free(filler);
}

void test_blockdev_write(uintptr_t param)
{
	struct blockdev *bdev;
	uint8_t *backing;
	size_t backing_size = 8192;
	uint32_t block_size = param;

	backing = malloc(backing_size);

	bdev = create_mock_blockdev("dev1", backing, backing_size, block_size);
	do_blockdev_write_tests(bdev, backing, backing_size, block_size);

	if (block_size > 1) {
		bdev = create_mock_blockdev("dev2", backing, backing_size, block_size);
		blockdev_set_buffer_alignment(bdev, block_size);
		do_blockdev_write_tests(bdev, backing, backing_size, block_size);
	}

	if (block_size > 2) {
		bdev = create_mock_blockdev("dev3", backing, backing_size, block_size);
		blockdev_set_buffer_alignment(bdev, block_size / 2);
		do_blockdev_write_tests(bdev, backing, backing_size, block_size);
	}

	free(backing);
}

void test_register_blockdev(uintptr_t param)
{
	static uint8_t backing;
	struct blockdev *bdev0;
	struct blockdev *bdev1;
	uint32_t found = 0;

	bdev0 = create_mock_blockdev("lookup-bdev0", &backing, 1, 1);
	TEST_ASSERT_NOT_NULL(bdev0);
	bdev1 = create_mock_blockdev("lookup-bdev1", &backing, 1, 1);
	TEST_ASSERT_NOT_NULL(bdev1);

	TEST_ASSERT_NULL(lookup_blockdev("lookup-bdev0"));
	TEST_ASSERT_NULL(lookup_blockdev("lookup-bdev1"));

	TEST_ASSERT_NULL(first_blockdev());

	register_blockdev(bdev0);

	TEST_ASSERT_PTR_EQ(lookup_blockdev("lookup-bdev0"), bdev0);
	TEST_ASSERT_PTR_EQ(first_blockdev(), bdev0);
	TEST_ASSERT_NULL(lookup_blockdev("lookup-bdev1"));

	register_blockdev(bdev1);

	TEST_ASSERT_PTR_EQ(lookup_blockdev("lookup-bdev0"), bdev0);
	TEST_ASSERT_PTR_EQ(lookup_blockdev("lookup-bdev1"), bdev1);

	for (struct blockdev *bdev = first_blockdev(); bdev != NULL; bdev = next_blockdev(bdev)) {
		if (bdev == bdev0)
			found |= 1;
		if (bdev == bdev1)
			found |= 2;
	}
	TEST_ASSERT_EQ(found, 3);
}

struct test_suite blockdev_suite = {
	.name = "blockdev",
	.test_cases = {
		{ "register_blockdev", test_register_blockdev, 0, "" },
		{ "mem_blockdev,1", test_mem_blockdev, 1, "" },
		{ "mem_blockdev,2", test_mem_blockdev, 2, "" },
		{ "mem_blockdev,4", test_mem_blockdev, 4, "" },
		{ "mem_blockdev,1024", test_mem_blockdev, 1024, "" },
		{ "blockdev_read_block,1", test_blockdev_read_block, 1, "" },
		{ "blockdev_read_block,2", test_blockdev_read_block, 2, "" },
		{ "blockdev_read_block,4", test_blockdev_read_block, 4, "" },
		{ "blockdev_read_block,1024", test_blockdev_read_block, 1024, "" },
		{ "blockdev_read_block,4096", test_blockdev_read_block, 4096, "" },
		{ "blockdev_read,1", test_blockdev_read, 1, "" },
		{ "blockdev_read,2", test_blockdev_read, 2, "" },
		{ "blockdev_read,4", test_blockdev_read, 4, "" },
		{ "blockdev_read,1024", test_blockdev_read, 1024, "" },
		{ "blockdev_read,4096", test_blockdev_read, 4096, "" },
		{ "blockdev_read,8192", test_blockdev_read, 8192, "" },
		{ "blockdev_write,1", test_blockdev_write, 1, "" },
		{ "blockdev_write,2", test_blockdev_write, 2, "" },
		{ "blockdev_write,4", test_blockdev_write, 4, "" },
		{ "blockdev_write,1024", test_blockdev_write, 1024, "" },
		{ "blockdev_write,4096", test_blockdev_write, 4096, "" },
		{ "blockdev_write,8192", test_blockdev_write, 8192, "" },
		{ "blockdev_set_buffer_alignment_read,2", test_blockdev_set_buffer_alignment_read, 2, "" },
		{ "blockdev_set_buffer_alignment_read,8", test_blockdev_set_buffer_alignment_read, 8, "" },
		{ "blockdev_set_buffer_alignment_read,1024", test_blockdev_set_buffer_alignment_read, 1024, "" },
		{ "blockdev_set_buffer_alignment_read,4096", test_blockdev_set_buffer_alignment_read, 4096, "" },
		TEST_CASE_LAST
	}
};

TEST_SUITE(blockdev_suite);
