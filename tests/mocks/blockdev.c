#include <unittest.h>
#include <lib/blockdev.h>
#include <stdlib.h>
#include <sys/stat.h>

struct mock_blockdev {
	struct blockdev bdev;

	/* local data */
	FILE *backing;
};

static int mock_blockdev_read_block(struct blockdev *bdev, void *ptr, block_addr block, uint32_t count)
{
	struct mock_blockdev *mdev = (struct mock_blockdev *)bdev;
	int result;

#if !NO_MOCK_ASSERTS
	// no overflow
	TEST_ASSERT_GTE(block + count, block);

	// buffer must be properly aligned
	TEST_ASSERT_EQ((uintptr_t)ptr & (bdev->alignment - 1), 0);

	TEST_ASSERT_GT(count, 0u);
	TEST_ASSERT_LT(block, mdev->bdev.block_count);
	TEST_ASSERT_LTE(block + count, mdev->bdev.block_count);
#endif

	result = fseek(mdev->backing, block << mdev->bdev.block_shift, SEEK_SET);
#if !NO_MOCK_ASSERTS
	TEST_ASSERT_EQ(result, 0);
#endif
	result = fread(ptr, 1 << mdev->bdev.block_shift, count, mdev->backing);
#if !NO_MOCK_ASSERTS
	TEST_ASSERT_EQ(result, count);
#endif

	return count;
}

struct blockdev *create_mock_blockdev(const char *name, const char *filename, uint32_t block_size)
{
	struct mock_blockdev *mdev;
	struct stat st;
	FILE *f;
	int result;

	result = stat(filename, &st);
	TEST_ASSERT_EQ(result, 0);

	f = fopen(filename, "r");
	TEST_ASSERT_NOT_NULL(f);

	mdev = malloc(sizeof(struct mock_blockdev));

	construct_blockdev(&mdev->bdev, name, st.st_size, block_size);
	mdev->backing = f;
	mdev->bdev.read_block_hook = &mock_blockdev_read_block;

	return &mdev->bdev;
}
