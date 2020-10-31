#include <unittest.h>
#include <sys.h>
#include <lib/heap.h>
#include <stdlib.h>
#include <string.h>

void *heap_arena;

#define HEAP_BLOCK_SIZE	64

#if !HEAP_DEBUGGING
 #define HEAP_OVERHEAD (3 * HEAP_BLOCK_SIZE)
#else
 #define HEAP_OVERHEAD (4 * HEAP_BLOCK_SIZE)
#endif

void test_heap_setup_func(void)
{
	extern void heap_reinit(void);
	heap_reinit();
}

void test_heap_cleanup_func(void)
{
	free(heap_arena);
	heap_arena = NULL;
}

void test_heap_setup_arena(size_t size)
{
	TEST_ASSERT_NULL(heap_arena);

	posix_memalign(&heap_arena, HEAP_BLOCK_SIZE, size);
	TEST_ASSERT_NOT_NULL(heap_arena);
}

void test_heap_add_chunk_size(uintptr_t param)
{
	size_t size = (size_t)param;

	test_heap_setup_arena(param);

	TEST_EXPECT_PANIC();

	heap_add_chunk(heap_arena, size, true);

	TEST_EXPECT_PANICKED();
}

void test_heap_malloc_before_add(uintptr_t param)
{
	TEST_EXPECT_PANIC();
	
	heap_malloc(1, "");

	TEST_EXPECT_PANICKED();
}

void test_heap_malloc_too_much1(uintptr_t param)
{
	test_heap_setup_arena(param);

	tprintf(TEST_SPEW, "Adding initial chunk to heap");
	heap_add_chunk(heap_arena, param, true);
	tprintf(TEST_SPEW, "Done adding initial chunk to heap");

	TEST_EXPECT_PANIC();
	// Asking for the size of the heap area should always be a panic
	// because of overhead
	heap_malloc((size_t)param, "");
	TEST_EXPECT_PANICKED();
}

void test_heap_malloc_too_much2(uintptr_t param)
{
	test_heap_setup_arena(param);
	heap_add_chunk(heap_arena, param, true);

	TEST_EXPECT_PANIC();
	// This should be just enough to cause an overflow
	heap_malloc((size_t)param - HEAP_OVERHEAD + 1, "");
	TEST_EXPECT_PANICKED();
}

void test_heap_malloc_too_much3(uintptr_t param)
{
	test_heap_setup_arena(param);
	heap_add_chunk(heap_arena, param, true);

	TEST_EXPECT_PANIC();
	heap_malloc(SIZE_MAX, "");
	TEST_EXPECT_PANICKED();
}

void test_heap_malloc_too_much4(uintptr_t param)
{
	test_heap_setup_arena(param);
	heap_add_chunk(heap_arena, param, true);

	TEST_EXPECT_PANIC();
	heap_malloc(SIZE_MAX - 2 * HEAP_BLOCK_SIZE, "");
	TEST_EXPECT_PANICKED();
}

void test_heap_malloc_double_free(uintptr_t param)
{
	void *ptr;

	test_heap_setup_arena(0x1000);
	heap_add_chunk(heap_arena, 0x1000, true);

	ptr = heap_malloc(0x100, "");
	heap_free(ptr);

	TEST_EXPECT_PANIC();
	heap_free(ptr);
	TEST_EXPECT_PANICKED();
}

void test_heap_free_null(uintptr_t param)
{
	test_heap_setup_arena(0x1000);
	heap_add_chunk(heap_arena, 0x1000, true);

	heap_free(NULL);

	heap_verify();
}

void test_heap_malloc_then_free(uintptr_t param)
{
	uint32_t free_mem;
	size_t size;
	void *ptr;

	test_heap_setup_arena(param);
	heap_add_chunk(heap_arena, param, true);

	free_mem = heap_get_free_mem();

	size = free_mem - HEAP_OVERHEAD;

	ptr = heap_malloc(size, "");
	TEST_ASSERT_NOT_NULL(ptr);
	memset(ptr, 0xff, size);

	heap_free(ptr);

	TEST_ASSERT_EQ(heap_get_free_mem(), free_mem);

	heap_verify();
}

void test_heap_malloc_then_free2(uintptr_t param)
{
	uint32_t free_mem;
	void *ptr[2];
	size_t size;

	test_heap_setup_arena(param);
	heap_add_chunk(heap_arena, param, true);

	free_mem = heap_get_free_mem();

	size = free_mem / 2 - HEAP_OVERHEAD;

	ptr[0] = heap_malloc(size, "");
	TEST_ASSERT_NOT_NULL(ptr[0]);
	ptr[1] = heap_malloc(size, "");
	TEST_ASSERT_NOT_NULL(ptr[1]);

	memset(ptr[0], 0xff, size);
	memset(ptr[1], 0xff, size);

	heap_free(ptr[0]);
	heap_free(ptr[1]);

	TEST_ASSERT_EQ(heap_get_free_mem(), free_mem);

	ptr[0] = heap_malloc(size, "");
	TEST_ASSERT_NOT_NULL(ptr[0]);
	ptr[1] = heap_malloc(size, "");
	TEST_ASSERT_NOT_NULL(ptr[1]);

	memset(ptr[0], 0xff, size);
	memset(ptr[1], 0xff, size);

	heap_free(ptr[1]);
	heap_free(ptr[0]);

	TEST_ASSERT_EQ(heap_get_free_mem(), free_mem);

	heap_verify();
}

void test_heap_malloc_then_free3(uintptr_t param)
{
	uint32_t free_mem;
	void *ptrs[4];

	if (param <= 4 * HEAP_OVERHEAD)
		param += 4 * HEAP_OVERHEAD;

	test_heap_setup_arena(param);
	heap_add_chunk(heap_arena, param, true);

	free_mem = heap_get_free_mem();

	for (unsigned i = 0; i < 4; i++) {
		size_t size = free_mem / 4 - HEAP_OVERHEAD;
		ptrs[i] = heap_malloc(size, "");
		TEST_ASSERT_NOT_NULL(ptrs[i]);

		memset(ptrs[i], 0xff, size);
	}

	heap_free(ptrs[1]);
	heap_free(ptrs[3]);
	heap_free(ptrs[0]);
	heap_free(ptrs[2]);

	TEST_ASSERT_EQ(heap_get_free_mem(), free_mem);

	heap_verify();
}

void test_heap_calloc(uintptr_t context)
{
	uint32_t *ptr;
	size_t arena_size = 0x10000;
	test_heap_setup_arena(arena_size);
	heap_add_chunk(heap_arena, arena_size, true);

	ptr = heap_calloc(sizeof(uint32_t), 0x1000, "");

	for (unsigned i = 0; i < 0x1000; i++)
		TEST_ASSERT_EQ(ptr[i], 0);

	heap_free(ptr);

	heap_verify();
}

void test_heap_calloc_overflow(uintptr_t context)
{
	uint32_t *ptr;
	size_t arena_size = 0x10000;
	test_heap_setup_arena(arena_size);
	heap_add_chunk(heap_arena, arena_size, true);

	TEST_EXPECT_PANIC();

	ptr = heap_calloc(0x10000, 0x10000, "");

	TEST_EXPECT_PANICKED();
}

void test_heap_realloc(uintptr_t context)
{
	uint32_t *filler;
	void *ptr1;
	void *ptr2;
	size_t initial_free_mem;
	size_t size = 0x1000;
	size_t arena_size = 0x10000;

	test_heap_setup_arena(arena_size);
	heap_add_chunk(heap_arena, arena_size, true);

	initial_free_mem = heap_get_free_mem();

	// reallocing NULL should be just like malloc
	ptr1 = heap_realloc(NULL, size, "");
	TEST_ASSERT_NOT_NULL(ptr1);
	memset(ptr1, 0x55, size);
	heap_free(ptr1);

	// generate some data to verify copy
	filler = malloc(size);
	TEST_ASSERT_NOT_NULL(filler);
	for (size_t i = 0; i < size / sizeof(*filler); i++)
		filler[i] = i;

	ptr1 = heap_malloc(size, "");
	TEST_ASSERT_NOT_NULL(ptr1);
	memcpy(ptr1, filler, size);

	ptr2 = heap_realloc(ptr1, size * 2, "");
	TEST_ASSERT_NOT_NULL(ptr2);

	TEST_ASSERT_MEM_EQ(ptr2, filler, size);

	free(filler);

	heap_free(ptr2);

	heap_verify();

	TEST_ASSERT_EQ(heap_get_free_mem(), initial_free_mem);

	// double freeing realloced pointer should be an error
	TEST_EXPECT_PANIC();
	heap_free(ptr1);
	TEST_EXPECT_PANICKED();
}

void test_heap_realloc_smaller(uintptr_t context)
{
	size_t size = 0x1800;
	size_t arena_size = 0x10000;
	size_t initial_free_mem;
	void *ptr;

	test_heap_setup_arena(arena_size);
	heap_add_chunk(heap_arena, arena_size, true);
	initial_free_mem = heap_get_free_mem();

	ptr = heap_malloc(size, "");
	TEST_ASSERT_NOT_NULL(ptr);

	heap_verify();

	ptr = heap_realloc(ptr, size - 0x800, "");
	TEST_ASSERT_NOT_NULL(ptr);
	
	heap_verify();

	ptr = heap_realloc(ptr, size + 0x400, "");
	TEST_ASSERT_NOT_NULL(ptr);

	heap_verify();

	heap_free(ptr);

	TEST_ASSERT_EQ(heap_get_free_mem(), initial_free_mem);
}

#if !HEAP_DEBUGGING
void test_heap_realloc_corruption(uintptr_t context)
{
	size_t size = 0x1000;
	size_t arena_size = 0x10000;
	uint8_t *ptr;

	test_heap_setup_arena(arena_size);
	heap_add_chunk(heap_arena, arena_size, true);

	ptr = heap_malloc(size, "");
	TEST_ASSERT_NOT_NULL(ptr);

	heap_verify();

	ptr[size + 1] ^= 1;

	TEST_EXPECT_PANIC();
	ptr = heap_realloc(ptr, size * 4, "");
	TEST_EXPECT_PANICKED();
}
#endif

void test_heap_realloc_corruption2(uintptr_t context)
{
	size_t size = 0x1000;
	size_t arena_size = 0x10000;
	uint8_t *ptr;

	test_heap_setup_arena(arena_size);
	heap_add_chunk(heap_arena, arena_size, true);

	ptr = heap_malloc(size, "");
	TEST_ASSERT_NOT_NULL(ptr);

	heap_verify();

	ptr[-1] ^= 1;

	TEST_EXPECT_PANIC();
	ptr = heap_realloc(ptr, size, "");
	TEST_EXPECT_PANICKED();
}

void test_heap_realloc_odd(uintptr_t context)
{
	uint32_t *filler;
	void *ptr1;
	void *ptr2;

	size_t size_start = 0x1000 - 32;
	size_t size_end = 0x1000 + 32;
	size_t arena_size = 0x10000;
	test_heap_setup_arena(arena_size);
	heap_add_chunk(heap_arena, arena_size, true);

	// generate some data to verify copy
	filler = malloc(size_end + 64);
	TEST_ASSERT_NOT_NULL(filler);
	for (size_t i = 0; i < (size_end + 64) / sizeof(*filler); i++)
		filler[i] = i;

	for (size_t size = size_start; size <= size_end; size++) {
		for (ssize_t adjust = -64; adjust <= 64; adjust++) {
			ptr1 = heap_malloc(size + adjust, "");
			TEST_ASSERT_NOT_NULL(ptr1);
			memcpy(ptr1, filler, size + adjust);

			ptr2 = heap_realloc(ptr1, size + adjust, "");
			TEST_ASSERT_NOT_NULL(ptr2);
			TEST_ASSERT_MEM_EQ(ptr2, filler, adjust > 0 ? size : size + adjust);

			heap_free(ptr2);
		}
	}

	free(filler);

	heap_verify();
}

void test_heap_memalign(uintptr_t context)
{
	size_t arena_size;
	void *ptr;
	void *ptrs[4];
	void *filler;

	size_t alignments[] = { 0x10, 0x20, 0x40, 0x100, 0x1000 };
	size_t sizes[] = { 1, 0x10, 0x20, 0x40, 0x100, 0x1000, 0x10000, 0x100000 };

	arena_size = 6 * 1024 * 1024;
	test_heap_setup_arena(arena_size);
	heap_add_chunk(heap_arena, arena_size, true);

	for (unsigned i = 0; i < sizeof(alignments) / sizeof(alignments[0]); i++) {
		size_t alignment = alignments[i];
		tprintf(TEST_INFO, "%s: alignment 0x%zx\n", __FUNCTION__, alignment);
		for (unsigned j = 0; j < sizeof(sizes) / sizeof(sizes[0]); j++) {
			size_t size = sizes[j];
			tprintf(TEST_INFO, "%s: size 0x%zx\n", __FUNCTION__, size);

			// allocate a single aligned pointer, fill the buffer, then free it
			ptr = NULL;
			ptr = heap_memalign(size, alignment, "");
			TEST_ASSERT_NOT_NULL(ptr);
			TEST_ASSERT_EQ((uintptr_t)ptr & (alignment - 1), 0);
			memset(ptr, 0xff, size);
			heap_free(ptr);

			filler = malloc(size);
			TEST_ASSERT_NOT_NULL(filler);

			// allocate four alignment pointers
			for (unsigned k = 0; k < sizeof(ptrs) / sizeof(ptrs[0]); k++) {
				ptrs[k] = heap_memalign(size, alignment, "");
				TEST_ASSERT_NOT_NULL(ptrs[k]);
				TEST_ASSERT_EQ((uintptr_t)ptrs[k] & (alignment - 1), 0);
			}

			// fill each buffer
			for (unsigned k = 0; k < sizeof(ptrs) / sizeof(ptrs[0]); k++) {
				memset(filler, k + 1, size);
				memcpy(ptrs[k], filler, size);
			}


			// make sure each buffer has the expected contents
			for (unsigned k = 0; k < sizeof(ptrs) / sizeof(ptrs[0]); k++) {
				memset(filler, k + 1, size);
				TEST_ASSERT_MEM_EQ((uint8_t *)ptrs[k], filler, size);
			}

			// and then free all the buffers
			for (unsigned k = 0; k < sizeof(ptrs) / sizeof(ptrs[0]); k++) {
				heap_free(ptrs[k]);
			}

			free(filler);
			filler = NULL;
		}
	}

	heap_verify();
}


void test_heap_memalign_bad_align(uintptr_t param)
{
	size_t arena_size = 0x10000;
	test_heap_setup_arena(arena_size);
	heap_add_chunk(heap_arena, arena_size, true);

	TEST_EXPECT_PANIC();
	heap_memalign(0x1000, 31, "");
	TEST_EXPECT_PANICKED();
}


void test_heap_memalign_too_much(uintptr_t param)
{
	size_t arena_size = 0x12000;
	test_heap_setup_arena(arena_size);
	heap_add_chunk(heap_arena, arena_size, true);

	TEST_EXPECT_PANIC();
	// size looks fine, but alignment is more than the size of the heap
	heap_memalign(0x1000, 0x10000, "");
	heap_memalign(0x1000, 0x10000, "");
	TEST_EXPECT_PANICKED();
}

void test_heap_posix_memalign(uintptr_t param)
{
	size_t arena_size;
	void *ptr;
	void *ptrs[4];
	void *filler;

	size_t alignments[] = { 0x10, 0x20, 0x40, 0x100, 0x1000 };
	size_t sizes[] = { 1, 0x10, 0x20, 0x40, 0x100, 0x1000, 0x10000, 0x100000 };

	arena_size = 6 * 1024 * 1024;
	test_heap_setup_arena(arena_size);
	heap_add_chunk(heap_arena, arena_size, true);

	for (unsigned i = 0; i < sizeof(alignments) / sizeof(alignments[0]); i++) {
		size_t alignment = alignments[i];
		tprintf(TEST_INFO, "%s: alignment 0x%zx\n", __FUNCTION__, alignment);
		for (unsigned j = 0; j < sizeof(sizes) / sizeof(sizes[0]); j++) {
			size_t size = sizes[j];
			tprintf(TEST_INFO, "%s: size 0x%zx\n", __FUNCTION__, size);

			// allocate a single aligned pointer, fill the buffer, then free it
			ptr = NULL;
			TEST_ASSERT_EQ(heap_posix_memalign(&ptr, alignment, size, ""), 0);
			TEST_ASSERT_NOT_NULL(ptr);
			TEST_ASSERT_EQ((uintptr_t)ptr & (alignment - 1), 0);
			memset(ptr, 0xff, size);
			heap_free(ptr);

			filler = malloc(size);
			TEST_ASSERT_NOT_NULL(filler);

			// allocate four alignment pointers
			for (unsigned k = 0; k < sizeof(ptrs) / sizeof(ptrs[0]); k++) {
				TEST_ASSERT_EQ(heap_posix_memalign(&ptrs[k], alignment, size, ""), 0);
				TEST_ASSERT_NOT_NULL(ptrs[k]);
				TEST_ASSERT_EQ((uintptr_t)ptrs[k] & (alignment - 1), 0);
			}

			// fill each buffer
			for (unsigned k = 0; k < sizeof(ptrs) / sizeof(ptrs[0]); k++) {
				memset(filler, k + 1, size);
				memcpy(ptrs[k], filler, size);
			}


			// make sure each buffer has the expected contents
			for (unsigned k = 0; k < sizeof(ptrs) / sizeof(ptrs[0]); k++) {
				memset(filler, k + 1, size);
				TEST_ASSERT_MEM_EQ((uint8_t *)ptrs[k], filler, size);
			}

			// and then free all the buffers
			for (unsigned k = 0; k < sizeof(ptrs) / sizeof(ptrs[0]); k++) {
				heap_free(ptrs[k]);
			}

			free(filler);
			filler = NULL;
		}
	}

	heap_verify();
}

void test_heap_posix_memalign_bad_align(uintptr_t param)
{
	void *ptr;

	size_t arena_size = 0x10000;
	test_heap_setup_arena(arena_size);
	heap_add_chunk(heap_arena, arena_size, true);

	TEST_EXPECT_PANIC();
	heap_posix_memalign(&ptr, 31, 0x1000, "");
	TEST_EXPECT_PANICKED();
}


void test_heap_posix_memalign_too_much(uintptr_t param)
{
	void *ptr;

	size_t arena_size = 0x12000;
	test_heap_setup_arena(arena_size);
	heap_add_chunk(heap_arena, arena_size, true);

	TEST_EXPECT_PANIC();
	// size looks fine, but alignment is more than the size of the heap
	heap_posix_memalign(&ptr, 0x10000, 0x1000, "");
	heap_posix_memalign(&ptr, 0x10000, 0x1000, "");
	TEST_EXPECT_PANICKED();
}

void test_heap_corruption(uintptr_t param)
{
	uint8_t *ptr;

	size_t arena_size = 0x10000;
	test_heap_setup_arena(arena_size);
	heap_add_chunk(heap_arena, arena_size, true);

	ptr = heap_malloc(0x100, "");
	ptr[0x100 + param] ^= 1;

	TEST_EXPECT_PANIC();
	heap_free(ptr);
	TEST_EXPECT_PANICKED();
}

void test_heap_corruption_pad(uintptr_t param)
{
	uint8_t *ptr;

	size_t arena_size = 0x10000;
	test_heap_setup_arena(arena_size);
	heap_add_chunk(heap_arena, arena_size, true);

	ptr = heap_malloc(0x101, "");
	ptr[0x101 + param] ^= 1;

	TEST_EXPECT_PANIC();
	heap_free(ptr);
	TEST_EXPECT_PANICKED();
}

void test_heap_corruption_pad2(uintptr_t param)
{
	uint8_t *ptr;

	size_t arena_size = 0x10000;
	test_heap_setup_arena(arena_size);
	heap_add_chunk(heap_arena, arena_size, true);

	ptr = heap_malloc(0xFF, "");
	ptr[0xFF] ^= 1;

	TEST_EXPECT_PANIC();
	heap_free(ptr);
	TEST_EXPECT_PANICKED();
}

void test_heap_verify(uintptr_t param)
{
	void *ptr;
	void *ptrs[500];

	size_t arena_size = 0x100000;
	test_heap_setup_arena(arena_size);
	heap_add_chunk(heap_arena, arena_size, true);

	ptr = heap_calloc(1, 1, "");
	TEST_ASSERT_NOT_NULL(ptr);
	
	ptr = heap_calloc(31, 1, "");
	TEST_ASSERT_NOT_NULL(ptr);

	heap_verify();

	ptr = heap_calloc(0x81, 1, "");
	TEST_ASSERT_NOT_NULL(ptr);

	heap_verify();

	ptr = heap_calloc(0x1000, 1, "");
	TEST_ASSERT_NOT_NULL(ptr);
	heap_free(ptr);

	heap_verify();

	for (unsigned i = 0; i < sizeof(ptrs) / sizeof(ptrs[0]); i++) {
		ptrs[i] = heap_calloc(0x100, 1, "");
		TEST_ASSERT_NOT_NULL(ptrs[i]);
	}

	heap_verify();

	for (unsigned i = 0; i < sizeof(ptrs) / sizeof(ptrs[0]); i += 2) {
		heap_free(ptrs[i]);
	}

	heap_verify();

	for (unsigned i = 0; i < sizeof(ptrs) / sizeof(ptrs[0]); i += 2) {
		ptrs[i] = heap_calloc(1, 1, "");
		TEST_ASSERT_NOT_NULL(ptrs[i]);
	}

	heap_verify();

	for (unsigned i = 0; i < sizeof(ptrs) / sizeof(ptrs[0]); i++) {
		heap_free(ptrs[i]);
	}

	heap_verify();
}

void test_heap_slide_large(uintptr_t param)
{
	void *ptr;
	size_t arena_size = 0x600000;
	uintptr_t heap_arena_calculated_offset;

	test_heap_setup_arena(arena_size);
	heap_add_chunk(heap_arena, arena_size, true);

	ptr = heap_calloc(1, 1, "");
	TEST_ASSERT_NOT_NULL(ptr);

	/* we have 2 sentinel blocks of 64 bytes each. */
	heap_arena_calculated_offset = (uintptr_t)heap_arena + (HEAP_BLOCK_SIZE * 2);

	TEST_ASSERT_EQ(((uintptr_t)ptr != heap_arena_calculated_offset), true);
}

void test_heap_slide_small(uintptr_t param)
{
	void *ptr;
	size_t arena_size = 0x100000;
	uintptr_t heap_arena_calculated_offset;

	test_heap_setup_arena(arena_size);
	heap_add_chunk(heap_arena, arena_size, true);

	ptr = heap_calloc(1, 1, "");
	TEST_ASSERT_NOT_NULL(ptr);

	/* we have 2 sentinel blocks of 64 bytes each. */
	heap_arena_calculated_offset = (uintptr_t)heap_arena + (HEAP_BLOCK_SIZE * 2);

	TEST_ASSERT_EQ(((uintptr_t)ptr == heap_arena_calculated_offset), true);
}

static struct test_suite heap_test_suite = {
	.name = "heap",
	.description = "tests the heap malloc implementation",
	.setup_function = test_heap_setup_func,
	.cleanup_function = test_heap_cleanup_func,
	.test_cases = {
		{ "chunk_too_small0", test_heap_add_chunk_size, 0 },
		{ "chunk_too_small1", test_heap_add_chunk_size, 1 },
		{ "chunk_too_small32", test_heap_add_chunk_size, 32 },
		{ "chunk_too_small_almost", test_heap_add_chunk_size, 3 * HEAP_BLOCK_SIZE },
		{ "malloc_before_add", test_heap_malloc_before_add, 0 },
		{ "malloc_too_much1,1k", test_heap_malloc_too_much1, 1024 },
		{ "malloc_too_much1,4k", test_heap_malloc_too_much1, 4 * 1024 },
		{ "malloc_too_much1,1m", test_heap_malloc_too_much1, 1024 * 1024 },
		{ "malloc_too_much2,1k", test_heap_malloc_too_much2, 1024 },
		{ "malloc_too_much2,4k", test_heap_malloc_too_much2, 4 * 1024 },
		{ "malloc_too_much2,1m", test_heap_malloc_too_much2, 1024 * 1024 },
		{ "malloc_too_much3,1k", test_heap_malloc_too_much3, 1024 },
		{ "malloc_too_much3,4k", test_heap_malloc_too_much3, 4 * 1024 },
		{ "malloc_too_much3,1m", test_heap_malloc_too_much3, 1024 * 1024 },
		{ "malloc_too_much4,1k", test_heap_malloc_too_much4, 1024 },
		{ "malloc_too_much4,4k", test_heap_malloc_too_much4, 4 * 1024 },
		{ "malloc_too_much4,1m", test_heap_malloc_too_much4, 1024 * 1024 },
		{ "malloc_double_free", test_heap_malloc_double_free },
		{ "free_null", test_heap_free_null },
		{ "malloc_then_free,1k", test_heap_malloc_then_free, 1024 },
		{ "malloc_then_free,4k", test_heap_malloc_then_free, 4 * 1024 },
		{ "malloc_then_free,1m", test_heap_malloc_then_free, 1024 * 1024 },
		{ "malloc_then_free2,1k", test_heap_malloc_then_free2, 1024 },
		{ "malloc_then_free2,4k", test_heap_malloc_then_free2, 4 * 1024 },
		{ "malloc_then_free2,1m", test_heap_malloc_then_free2, 1024 * 1024 },
		{ "malloc_then_free3,1k", test_heap_malloc_then_free3, 1024 },
		{ "malloc_then_free3,4k", test_heap_malloc_then_free3, 4 * 1024 },
		{ "malloc_then_free3,1m", test_heap_malloc_then_free3, 1024 * 1024 },
		{ "calloc", test_heap_calloc },
		{ "calloc_overflow", test_heap_calloc_overflow },
		{ "realloc", test_heap_realloc },
		{ "realloc_smaller", test_heap_realloc_smaller },
		{ "realloc_corruption2", test_heap_realloc_corruption2 },
		{ "realloc_odd", test_heap_realloc_odd },
		{ "memalign", test_heap_memalign },
		{ "posix_memalign", test_heap_posix_memalign },
		{ "posix_memalign_bad_align", test_heap_posix_memalign_bad_align },
		{ "posix_memalign_too_much", test_heap_posix_memalign_too_much },
		// the offsets provided by these tests make some assumptions
		// about sizeof(size_t) and CPU_CACHELINE_SIZE and assume no debug data
#if !HEAP_DEBUGGING
		{ "heap_corruption,0", test_heap_corruption, 0 },
		{ "heap_corruption,1", test_heap_corruption, 1 },
		{ "heap_corruption,2", test_heap_corruption, 2 },
		{ "heap_corruption,3", test_heap_corruption, 3 },
		{ "heap_corruption,4", test_heap_corruption, 4 },
		{ "heap_corruption,5", test_heap_corruption, 5 },
		{ "heap_corruption,6", test_heap_corruption, 6 },
		{ "heap_corruption,7", test_heap_corruption, 7 },
		{ "heap_corruption,8", test_heap_corruption, 8 },
		{ "heap_corruption,9", test_heap_corruption, 9 },
		{ "heap_corruption,10", test_heap_corruption, 10 },
		{ "heap_corruption,11", test_heap_corruption, 11 },
		{ "heap_corruption,12", test_heap_corruption, 12 },
		{ "heap_corruption,13", test_heap_corruption, 13 },
		{ "heap_corruption,14", test_heap_corruption, 14 },
		{ "heap_corruption,15", test_heap_corruption, 15 },
		{ "heap_corruption,16", test_heap_corruption, 16 },
		{ "heap_corruption,17", test_heap_corruption, 17 },
		{ "heap_corruption,18", test_heap_corruption, 18 },
		{ "heap_corruption,19", test_heap_corruption, 19 },
		{ "heap_corruption,20", test_heap_corruption, 20 },
		{ "heap_corruption,21", test_heap_corruption, 21 },
		{ "heap_corruption,22", test_heap_corruption, 22 },
		{ "heap_corruption,23", test_heap_corruption, 23 },
		{ "heap_corruption,32", test_heap_corruption, 32 },
		{ "heap_corruption,33", test_heap_corruption, 33 },
		{ "heap_corruption,34", test_heap_corruption, 34 },
		{ "heap_corruption,35", test_heap_corruption, 35 },
		{ "heap_corruption,36", test_heap_corruption, 36 },
		{ "heap_corruption,37", test_heap_corruption, 37 },
		{ "heap_corruption,38", test_heap_corruption, 38 },
		{ "heap_corruption,39", test_heap_corruption, 39 },
		{ "heap_corruption,40", test_heap_corruption, 40 },
		{ "heap_corruption,41", test_heap_corruption, 41 },
		{ "heap_corruption,42", test_heap_corruption, 42 },
		{ "heap_corruption,43", test_heap_corruption, 43 },
		{ "heap_corruption,44", test_heap_corruption, 44 },
		{ "heap_corruption,45", test_heap_corruption, 45 },
		{ "heap_corruption,46", test_heap_corruption, 46 },
		{ "heap_corruption,47", test_heap_corruption, 47 },
		{ "realloc_corruption", test_heap_realloc_corruption },
#endif
		{ "heap_corruption_pad,0", test_heap_corruption_pad, 0 },
		{ "heap_corruption_pad,1", test_heap_corruption_pad, 1 },
		{ "heap_corruption_pad,2", test_heap_corruption_pad, 2 },
		{ "heap_corruption_pad,3", test_heap_corruption_pad, 3 },
		{ "heap_corruption_pad,4", test_heap_corruption_pad, 4 },
		{ "heap_corruption_pad,5", test_heap_corruption_pad, 5 },
		{ "heap_corruption_pad,6", test_heap_corruption_pad, 6 },
		{ "heap_corruption_pad,7", test_heap_corruption_pad, 7 },
		{ "heap_corruption_pad,8", test_heap_corruption_pad, 8 },
		{ "heap_corruption_pad,31", test_heap_corruption_pad, 31 },
		{ "heap_corruption_pad2", test_heap_corruption_pad2 },
		{ "heap_verify", test_heap_verify },
		{ "heap_slide_large", test_heap_slide_large },
		{ "heap_slide_small", test_heap_slide_small },
		TEST_CASE_LAST
	}
};

TEST_SUITE(heap_test_suite);

// for when debugging is enabled
utime_t system_time()
{
	return 0;
}
