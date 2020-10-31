#include <unittest.h>
#include <lib/blockdev.h>
#include <lib/syscfg.h>
#include <stdlib.h>
#include <string.h>

#define TEST_SYSCFG_BLOCKDEV_SIZE 0x8000

extern void syscfg_reinit(void);

const uint32_t test_syscfg_data1[] =
{
	'SCfg',			// magic
	0x1000,			// size
	0x1000,			// max size
	0x0,			// version
	0x0,			// big endian (NOT)
	9,			// key count

	'Ent0',			// key
	0x0, 0x0, 0x0, 0x0,	// data

	'Ent1',			// key
	0x1, 0x1, 0x1, 0x1,	// data

	'Ent2',			// key
	0x200, 0x200,		// data
	0x200, 0x200,

	'Ent3',			// key
	0x30000, 0x30000,	// data
	0x30000, 0x30000,

	'Ent4',			// key
	0x4000000, 0x4000000,	// data
	0x4000000, 0x4000000,

	'CNTB', 'Ent5',
	4, 0x180, 0,		// size 4 offset 0x100

	'CNTB', 'Ent6',
	8, 0x184, 0,		// size 4 offset 0x100

	'CNTB', 'Ent7',
	1, 0x18c, 0,		// size 1 offset 0x104

	'CNTB', 'Ent8',
	32, 0x190, 0,		// size 1 offset 0x104

	// padding, delete an entry here whenever adding an entry above
	0x0, 0x0, 0x0, 0x0, 0x0,
	0x0, 0x0, 0x0, 0x0, 0x0,
	0x0, 0x0, 0x0, 0x0, 0x0,
	0x0, 0x0, 0x0, 0x0, 0x0,
	0x0, 0x0, 0x0, 0x0, 0x0,
	0x0, 0x0, 0x0, 0x0, 0x0,
	0x0, 0x0, 0x0, 0x0, 0x0,
	0x0, 0x0, 0x0, 0x0, 0x0,
	0x0, 0x0, 0x0, 0x0, 0x0,

	// CNTB block data starts at 0x100
	0xabcd1234,		// for 'Ent5'
	0x11111111,		// for 'Ent6'
	0x22222222,		// .
	0xDDDDDDAA,		// for 'Ent7', only 'AA' part is actually valid
	0x12345678,		// for 'Ent8'
	0x23456789,		// .
	0x3456789a,		// .
	0x456789ab,		// .
	0x567891bc,		// .
	0x6789abcd,		// .
	0x789abcde,		// .
	0x89abcdef,		// .
};

const uint32_t test_syscfg_data3[] =
{
	'SCfg',			// magic
	0x100,			// size
	0x100,			// max size
	0x0,			// version
	0x0,			// big endian (NOT)
	4,			// key count

	'CNTB', 'Ent1',
	8, 0xf8, 0,		// pointer is right at end of advertised size, but ok

	'CNTB', 'Ent2',
	4, 0x200, 0,		// pointer is past advertised size

	'CNTB', 'Ent3',
	2, 0xff, 0,		// pointer plus size puts over the end

	'CNTB', 'Ent4',
	32, 0x8000, 0,		// pointer is past end of bdev

	// actual CNTB block data is just padded out with zeroes
};

// XXX add test with malformed data advertising way too big size

void test_setup_backing(void *backing, const void *data, size_t data_size)
{
	TEST_ASSERT_NOT_NULL(backing);

	memset(backing, 0, TEST_SYSCFG_BLOCKDEV_SIZE);

	memcpy((uint8_t *)backing + 0x4000, data, data_size); 
}


void test_syscfg1(uintptr_t param)
{
	struct blockdev *bdev;

	void *backing = malloc(TEST_SYSCFG_BLOCKDEV_SIZE);

	bdev = create_mem_blockdev("syscfg_bdev1", backing, TEST_SYSCFG_BLOCKDEV_SIZE, 0x1000);
	TEST_ASSERT_NOT_NULL(bdev);

	register_blockdev(bdev);

	// Initializing with wrong name should result in failure
	TEST_ASSERT_EQ(syscfgInitWithBdev("wrong"), false);
	// and then a subsequent read should fail
	{
		uint8_t resultData[16];
		TEST_ASSERT_EQ(syscfgCopyDataForTag('Ent0', resultData, sizeof(resultData)), -1);
	}

	// Initializing with empty syscfg should result in failure
	memset(backing, 0, TEST_SYSCFG_BLOCKDEV_SIZE);
	TEST_ASSERT_EQ(syscfgInitWithBdev("syscfg_bdev1"), false);
	// and then a subsequent read should fail
	{
		uint8_t resultData[16];
		TEST_ASSERT_EQ(syscfgCopyDataForTag('Ent0', resultData, sizeof(resultData)), -1);
	}

	test_setup_backing(backing, test_syscfg_data1, sizeof(test_syscfg_data1));

	// initializing with a valid syscfg should work
	TEST_ASSERT_EQ(syscfgInitWithBdev("syscfg_bdev1"), true);

	{
		uint8_t resultData[16];

		// looking for a non-existant tag should return failure
		TEST_ASSERT_EQ(syscfgCopyDataForTag('EntX', resultData, sizeof(resultData)), -1);

		// looking for ones that exist should work
		TEST_ASSERT_EQ(syscfgCopyDataForTag('Ent0', resultData, sizeof(resultData)), sizeof(resultData));
		TEST_ASSERT_MEM_EQ(resultData, TEST_ARRAY(uint32_t, 0, 0, 0, 0), sizeof(resultData));

		TEST_ASSERT_EQ(syscfgCopyDataForTag('Ent1', resultData, sizeof(resultData)), sizeof(resultData));
		TEST_ASSERT_MEM_EQ(resultData, TEST_ARRAY(uint32_t, 1, 1, 1, 1), sizeof(resultData));

		TEST_ASSERT_EQ(syscfgCopyDataForTag('Ent2', resultData, sizeof(resultData)), sizeof(resultData));
		TEST_ASSERT_MEM_EQ(resultData, TEST_ARRAY(uint32_t, 0x200, 0x200, 0x200, 0x200), sizeof(resultData));

		TEST_ASSERT_EQ(syscfgCopyDataForTag('Ent3', resultData, sizeof(resultData)), sizeof(resultData));
		TEST_ASSERT_MEM_EQ(resultData, TEST_ARRAY(uint32_t, 0x30000, 0x30000, 0x30000, 0x30000), sizeof(resultData));

		TEST_ASSERT_EQ(syscfgCopyDataForTag('Ent4', resultData, sizeof(resultData)), sizeof(resultData));
		TEST_ASSERT_MEM_EQ(resultData, TEST_ARRAY(uint32_t, 0x4000000, 0x4000000, 0x4000000, 0x4000000), sizeof(resultData));

		// Requesting an amount smaller than the entry size results in just that amount
		memset(resultData, 0xff, sizeof(resultData));
		TEST_ASSERT_EQ(syscfgCopyDataForTag('Ent1', resultData, 4), 4);
		TEST_ASSERT_MEM_EQ(resultData, TEST_ARRAY(uint32_t, 1, 0xffffffff, 0xffffffff, 0xffffffff), sizeof(resultData));

	}

	{
		uint8_t resultData[32];

		// Requesting an amount bigger than the entry size results in a short read
		memset(resultData, 0xff, sizeof(resultData));
		TEST_ASSERT_EQ(syscfgCopyDataForTag('Ent1', resultData, sizeof(resultData)), 16);
		TEST_ASSERT_MEM_EQ(resultData, TEST_ARRAY(uint32_t, 1, 1, 1, 1), 16);
		TEST_ASSERT_MEM_EQ(resultData + 16, TEST_ARRAY(uint32_t, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff), 16);
	}

	{
		uint8_t resultData[4];

		// Reading from a CNTB-style entry should work and return the size given in the entry
		memset(resultData, 0xff, sizeof(resultData));
		TEST_ASSERT_EQ(syscfgCopyDataForTag('Ent5', resultData, sizeof(resultData)), sizeof(resultData));
		TEST_ASSERT_MEM_EQ(resultData, TEST_ARRAY(uint32_t, 0xabcd1234), sizeof(resultData));
	}

	{
		uint8_t resultData[8];

		// Short reads from CNTB blocks should still result in the short size being returned
		memset(resultData, 0xff, sizeof(resultData));
		TEST_ASSERT_EQ(syscfgCopyDataForTag('Ent6', resultData, 4), 4);
		TEST_ASSERT_MEM_EQ(resultData, TEST_ARRAY(uint32_t, 0x11111111, 0xffffffff), sizeof(resultData));
	}
}

void test_syscfg2(uintptr_t param)
{
	struct blockdev *bdev;
	void *backing = malloc(TEST_SYSCFG_BLOCKDEV_SIZE);
	test_setup_backing(backing, test_syscfg_data1, sizeof(test_syscfg_data1));

	// bdev has the right data in it, but that data is past the end of its advertised length
	bdev = create_mem_blockdev("syscfg_bdev2", backing, 0x4000, 0x1000);

	register_blockdev(bdev);

	TEST_ASSERT_EQ(syscfgInitWithBdev("syscfg_bdev2"), false);
}

void test_syscfg3(uintptr_t param)
{
	struct blockdev *bdev;
	uint8_t resultData[8];
	void *backing = malloc(TEST_SYSCFG_BLOCKDEV_SIZE);

	test_setup_backing(backing, test_syscfg_data3, sizeof(test_syscfg_data3));

	bdev = create_mem_blockdev("syscfg_bdev3", backing, TEST_SYSCFG_BLOCKDEV_SIZE, 0x1000);

	register_blockdev(bdev);

	TEST_ASSERT_EQ(syscfgInitWithBdev("syscfg_bdev3"), true);

	TEST_ASSERT_EQ(syscfgCopyDataForTag('Ent1', resultData, 8), 8);

	TEST_ASSERT_EQ(syscfgCopyDataForTag('Ent2', resultData, 8), -1);
	TEST_ASSERT_EQ(syscfgCopyDataForTag('Ent3', resultData, 8), -1);
	TEST_ASSERT_EQ(syscfgCopyDataForTag('Ent4', resultData, 8), -1);
}

// XXX add a test for syscfg_find_tag

static struct test_suite syscfg_test_suite = {
	.name = "syscfg",
	.description = "tests the syscfg library",
	.setup_function = syscfg_reinit,
	.test_cases = {
		{ "syscfg", test_syscfg1, 0 },
		{ "syscfg", test_syscfg2, 0 },
		{ "syscfg", test_syscfg3, 0 },
		TEST_CASE_LAST
	}
};

TEST_SUITE(syscfg_test_suite);
