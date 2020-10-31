/*
 * Copyright (C) 2007-2014 Apple Inc. All rights reserved.
 * Copyright (C) 2006 Apple Computer, Inc. All rights reserved.
 *
 * This document is the property of Apple Computer, Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Computer, Inc.
 */
/*
 * simple heap -- mpetit
 *
 *   a very simple multi-bin immediate coalescing heap.
 */
#include <debug.h>
#include <errno.h>
#include <lib/cksum.h>
#include <lib/heap.h>
#include <lib/libiboot.h>
#include <lib/random.h>
#include <lib/mib.h>
#include <platform.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <sys.h>
#include <sys/menu.h>

// Right now, we require the chunk size to be at least 2MB to be worth sliding.
#define MINIMUM_HEAP_SIZE_FOR_SLIDE		(2 * 1024 * 1024)

// 12-bits worth of entropy for the mask
#define HEAP_ENTROPY_MASK		(0xfffUL)

// Heap blocks need to be cacheline-size aligned to accomodate legacy drivers
// that don't properly align their DMA buffers. Because the cache line size
// isn't available in libraries, we're going with the largest size of any known
// cache we support
#define HEAP_BLOCK_SIZE		(64)

#define HEAP_CHECKS		1
#define HEAP_PANIC_CHECKS	1
#define HEAP_ERROR_PANIC	1
#define HEAP_ERROR_DUMP		0
#define TRACE_ALLOCATIONS	0
#ifndef HEAP_DEBUGGING
 #define HEAP_DEBUGGING		0
#endif

#if RELEASE_BUILD
 #define HEAP_PANIC(b, x) heap_panic("", NULL, "")
#else
 #define HEAP_PANIC(b, x) heap_panic(__func__, b, x)
#endif

#if TRACE_ALLOCATIONS
# define TRACE(a...) printf(a)
#else
# define TRACE(...) do {} while(0)
#endif

#ifndef HEAP_UNITTEST
// Minimum heap size required for heap sliding
MIB_CONSTANT_WEAK(kMIBTargetMinimumHeapSizeForSlide, kOIDTypeSize, MINIMUM_HEAP_SIZE_FOR_SLIDE);
#endif

enum
{
	NUM_CHUNKS = 4,
	NUM_BINS = 32,
};

struct heap_block
{
	// checksum of all members excep _pad, includes next_in_bin and prev_in_bin iff this_free is set
	uint64_t checksum;
	/*
	 * In order to get allocations that are cacheline-sized, we have to blow an entire line
	 * on the heap block.  This is a bit inefficient, but allocations need to be I/O safe.
	 */
	uint32_t _pad[(HEAP_BLOCK_SIZE - 4 * __SIZEOF_SIZE_T__ - 8) / 4];
	// size of this block in units of sizeof(struct heap_block)
	size_t  this_size;
	size_t  this_free:1;
	size_t  prev_free:1;
	// size of the previous block in units of sizeof(struct heap_block),
	// allows the location of the previous block header to be found
	size_t  prev_size:(__SIZE_WIDTH__ - 2);
	// offset from beginning of block to padding
	size_t  padding_start;
	size_t  padding_bytes;
};


struct free_block
{
	struct heap_block  common;
	struct free_block *next_in_bin;
	struct free_block *prev_in_bin;
};

struct client_data
{
	uint32_t block_type;
	void    *requestor;
	const char *requestor_name;
	utime_t  timestamp;

	uint32_t user_size;
	uint32_t alignment;
};

struct chunk_data
{
	struct heap_block *chunk_base;
	uint32_t           chunk_size;
};

static void heap_panic(char const *func, const void *block, const char *reason);
#if DEBUG_BUILD || HEAP_ERROR_DUMP
static void heap_dump(void);
#endif

static int                 chunk_count;
static struct chunk_data   lead[NUM_CHUNKS];
static struct free_block   bins[NUM_BINS];
static u_int32_t	   free_mem;

// static initializer is a fallback for cases where
// we can't get a random before enabling the heap
static_assert(SIPHASH_KEY_SIZE == HEAP_COOKIE_SIZE, "heap cookie size and MAC key size mismatch");
uint64_t g_heap_cookie[HEAP_COOKIE_SIZE] = { 0x64636b783132322fULL, 0xa7fa3a2e367917fcULL };

#if HEAP_UNITTEST
void
heap_reinit(void)
{
	chunk_count = 0;
	memset(lead, 0, sizeof(lead));
	memset(bins, 0, sizeof(bins));
	free_mem = 0;
}
#endif

static
size_t
dequantify_size(size_t size)
{
	return size*sizeof(struct heap_block);
}

static
size_t
quantify_size(size_t size)
{
	return size/sizeof(struct heap_block);
}

static
size_t
round_size(size_t size)
{
	return (size + sizeof(struct heap_block) - 1) & ~(sizeof(struct heap_block) -1);
}

static
unsigned
compute_bin(size_t size)
{
	unsigned result;
	
	size = quantify_size(size);
	RELEASE_ASSERT(size >= 1);

	// clz can't operate on > 32-bit values, so return an invalid bin
	// number to force grab_chunk to fail and free_list_add to assert
	if ((uint64_t)size > UINT32_MAX) {
		return NUM_BINS;
	}

	result = NUM_BINS - __builtin_clz((uint32_t)size);
	RELEASE_ASSERT(result < NUM_BINS);

	return result;
}

static
size_t
required_size(size_t size)
{
	size += 2*sizeof(struct heap_block) - 1;
	
	if (size < (2*sizeof(struct heap_block) - 1)) {
#ifdef HEAP_ERROR_PANIC
		panic("integer overflow in required_size\n");
#endif
	}

	size &= ~(sizeof(struct heap_block) - 1);

	if (size < sizeof(struct free_block)) {
		size = sizeof(struct free_block);
	}

	return size;
}

static
size_t
required_size_for_split(size_t size)
{
	size += sizeof(struct free_block);

	return size;
}
static
size_t
sizeof_block(struct heap_block *b)
{
	return dequantify_size(b->this_size);
}

static
void *
advance_pointer(void const *p, size_t stride)
{
	uintptr_t val;

	val = (uintptr_t)(p);
	val += stride;

	return (void*)(val);
}

static
void *
recess_pointer(void const *p, size_t stride)
{
	uintptr_t val;

	val = (uintptr_t)(p);
	val -= stride;

	return (void*)(val);
}

static
void *
next_block(struct heap_block const *b)
{
	return advance_pointer(b, dequantify_size(b->this_size));
}

static
void *
prev_block(struct heap_block const *b)
{
	return recess_pointer(b, dequantify_size(b->prev_size));
}

static
void
verify_block_checksum(struct heap_block const *b)
{
#if HEAP_CHECKS
	uint64_t checksum;
	size_t checksumlen = sizeof(struct heap_block) - offsetof(struct heap_block, this_size);
	if (b->this_free)
		checksumlen += sizeof(struct free_block) - sizeof(struct heap_block);

	siphash_aligned((uint64_t *)&checksum, (const uint64_t *)&b->this_size, checksumlen, g_heap_cookie);

	if (checksum != b->checksum) {
		HEAP_PANIC(b, "bad block checksum");
	}
#endif
}

static
void
calculate_block_checksum(struct heap_block *b)
{
#if HEAP_CHECKS
	size_t checksumlen = sizeof(struct heap_block) - offsetof(struct heap_block, this_size);
	if (b->this_free)
		checksumlen += sizeof(struct free_block) - sizeof(struct heap_block);

	siphash_aligned((uint64_t *)&b->checksum, (const uint64_t *)&b->this_size, checksumlen, g_heap_cookie);
#endif
}

static
void
verify_block_padding(struct heap_block const *b)
{
#if HEAP_CHECKS
	size_t padding = b->padding_bytes;
	if (padding) {
		uint8_t *pad_end = (uint8_t *)b + b->padding_start + padding;

		for(size_t i = 0; i < padding; i++) {
			if (*(pad_end - 1 - i) != (uint8_t)((size_t)-1 - i)) {
				HEAP_PANIC(b, "bad padding");
			}
		}
	}

	// Also verify the padding inside the heap block metadata
	for (size_t i = 0; i < sizeof(b->_pad) / sizeof(b->_pad[0]); i++) {
		if (b->_pad[i] != 0) {
			HEAP_PANIC(b, "bad padding");
		}
	}
#endif
}

// Write a predictable pattern from the end of the user-requested
// portion of the heap block to the end of the actual heap block.
// This allows overflows to be detected even if they don't touch
// the next heap block's metadata
static
void
pad_block(struct heap_block *b, size_t user_size)
{
#if HEAP_CHECKS
	uint8_t *pad_start = advance_pointer(b + 1, user_size);
	uint8_t *pad_end = (uint8_t *)next_block(b);
	size_t padding;
	
#if HEAP_DEBUGGING
	pad_end -= round_size(sizeof(struct client_data));
#endif

	padding = (pad_end - pad_start);

	b->padding_bytes = padding;
	b->padding_start = pad_start - (uint8_t *)b;

	for (size_t i = 0; i < padding; i++) {
		*(pad_end - 1 - i) = (uint8_t)((size_t)-1 - i);
	}

	// Also zero out the padding section of the heap block's
	// header
	memset(b->_pad, 0, sizeof(b->_pad));
#endif
}

static
void
free_list_remove(struct free_block *b)
{
	if (b->prev_in_bin == b)
		HEAP_PANIC(b, "free list unlink error");
	if (b->next_in_bin == b)
		HEAP_PANIC(b, "free list unlink error");
	if (b->prev_in_bin->next_in_bin != b)
		HEAP_PANIC(b, "free list unlink error");
	if (b->next_in_bin && b->next_in_bin->prev_in_bin != b)
		HEAP_PANIC(b, "free list unlink error");

	verify_block_checksum(&b->prev_in_bin->common);
	b->prev_in_bin->next_in_bin = b->next_in_bin;
	calculate_block_checksum(&b->prev_in_bin->common);

	if (b->next_in_bin) {
		verify_block_checksum(&b->next_in_bin->common);
		b->next_in_bin->prev_in_bin = b->prev_in_bin;
		calculate_block_checksum(&b->next_in_bin->common);
	}
}

static
void
free_list_add(struct free_block *b)
{
	struct free_block *bin;
	unsigned bin_num;

	bin_num = compute_bin(sizeof_block(&b->common));
	RELEASE_ASSERT(bin_num < NUM_BINS);

	bin = bins + bin_num;

	b->next_in_bin = bin->next_in_bin;
	b->prev_in_bin = bin;
	if (bin->next_in_bin) {
		if (bin->next_in_bin == bin)
			HEAP_PANIC(bin, "free list unlink error");
		if (bin->next_in_bin->prev_in_bin != bin)
			HEAP_PANIC(bin, "free list unlink error");

		verify_block_checksum(&bin->next_in_bin->common);
		bin->next_in_bin->prev_in_bin = b;
		calculate_block_checksum(&bin->next_in_bin->common);
	}
	bin->next_in_bin = b;

	calculate_block_checksum(&bin->common);
}

static
void *
split_head(struct heap_block *b, size_t size)
{
	if (!size) {
		return b;
	} else if (size < sizeof(struct free_block)) {
		/*
		 * the preamble is too small to contain a free block
		 * so we just append the preamble to the previous block
		 *
		 * and edge condition would be that this block is the
		 * first one, but this situation is skipped by making
		 * sure the arena starts at an address congurent with
		 * the size of a free block
		 */
		struct heap_block *prev;
		struct heap_block *next;
		struct heap_block *retval;

		prev = prev_block(b);
		verify_block_checksum(prev);
		next = next_block(b);
		verify_block_checksum(next);

		retval = advance_pointer(b, size);

		prev->this_size += quantify_size(size);
		next->prev_size -= quantify_size(size);

		retval->prev_free = b->prev_free;
		retval->this_free = b->this_free;
		retval->prev_size = b->prev_size + quantify_size(size);
		retval->this_size = b->this_size - quantify_size(size);

		calculate_block_checksum(retval);
		calculate_block_checksum(prev);
		calculate_block_checksum(next);

		return retval;
	} else {
		struct heap_block *next;
		struct heap_block *retval;

		next = next_block(b);
		verify_block_checksum(next);

		retval = advance_pointer(b, size);

		next->prev_size -= quantify_size(size);

		retval->prev_free = 1;
		retval->this_free = b->this_free;
		retval->prev_size = quantify_size(size);
		retval->this_size = b->this_size - quantify_size(size);

		b->this_free = 1;
		b->this_size = quantify_size(size);
		free_list_add((struct free_block *)b);

		calculate_block_checksum(b);
		calculate_block_checksum(retval);
		calculate_block_checksum(next);

		return retval;
	}
}

static
void
split_tail(struct heap_block *b, size_t size, size_t user_size)
{
	struct heap_block *next_one;
	size_t block_size;

	verify_block_checksum(b);

	next_one = next_block(b);
	verify_block_checksum(next_one);

	block_size = sizeof_block(b);

	if (block_size >= required_size_for_split(size)) {
		struct free_block *remainder;

		remainder = advance_pointer(b, size);

		remainder->common.prev_free = 0;
		remainder->common.this_free = 1;
		remainder->common.prev_size = quantify_size(size);
		remainder->common.this_size = b->this_size - remainder->common.prev_size;
		free_list_add(remainder);
		calculate_block_checksum(&remainder->common);

		b->this_size = quantify_size(size);

		next_one->prev_size = remainder->common.this_size;
	} else {
		next_one->prev_free = 0;
	}

	b->this_free = 0;

	pad_block(b, user_size);

	calculate_block_checksum(b);
	calculate_block_checksum(next_one);
}

static
void *
grab_chunk(size_t size, size_t user_size)
{
	size_t bin;

	for (bin= compute_bin(size); bin< NUM_BINS; bin++) {
		struct free_block *curr;

		// the first element of the list isn't a real free block; it just guarantees
		// every real free block has a previous block
		curr = &bins[bin];
		curr = curr->next_in_bin;

		// iterate the real blocks in the bin
		while (curr) {
			if (sizeof_block(&curr->common) >= size) {
				verify_block_checksum(&curr->common);

				// take the current block out of its bin
				free_list_remove(curr);
				// if possible, add back any unneeded part of the
				// current block to the appropriate bin;
				split_tail(&curr->common, size, user_size);

				return curr;
			}

			curr = curr->next_in_bin;
		}
	}

#if HEAP_ERROR_PANIC
#if HEAP_ERROR_DUMP
	heap_dump();
#endif
	panic("heap overflow");
#endif
	return NULL;
}

// used by heap_memalign to find a chunk of memory whose base satisfies
// the given alignment constraint
static
void *
grab_chunk__constrained(size_t size, size_t constraint, size_t user_size)
{
	size_t bin;

	// search through all the bins, skipping any that are guaranteed to be too small
	for (bin= compute_bin(size); bin< NUM_BINS; bin++) {
		struct free_block *curr;

		// the first element of the list isn't a real free block; it just guarantees
		// every real free block has a previous block
		curr = &bins[bin];
		curr = curr->next_in_bin;

		// iterate the real blocks in the bin
		while (curr) {
			uintptr_t base;
			uintptr_t limit;
			size_t usable_size;

			// calculate the address of the memory the candidate block holds rounded up to
			// required alignment
			base = ((uintptr_t)(curr) + sizeof(struct heap_block) + constraint - 1) & ~(constraint - 1);
			// and then figure out where the metadata for memory at the required
			// alignment would sit
			base -= sizeof(struct heap_block);

			// the address of the block following the candidate block
			limit = (uintptr_t)(curr) + sizeof_block(&curr->common);

			// first condition, aligned address can't be past the end of the candidate block
			if (base < limit) {
				// if the requested amount of memory is available between the aligned address
				// and the end of the block, we're good
				usable_size = limit - base;
				if (usable_size >= size) {
					verify_block_checksum(&curr->common);

					// pull this block out of its bin
					free_list_remove(curr);

					// put the unused portions of the block before and after
					// the aligned address onto the appropriate free lists
					curr = split_head(&curr->common, base - (uintptr_t)(curr));
					split_tail(&curr->common, size, user_size);

					return curr;
				}
			}

			curr = curr->next_in_bin;
		}
	}

#if HEAP_ERROR_PANIC
	panic("heap overflow");
#endif
	return NULL;
}


static
struct heap_block *
merge_blocks_left(struct heap_block *left, struct heap_block *right)
{
	// caller will add the new block back into the free list
	// (possibly in a new bin to reflect the larger size)
	free_list_remove((struct free_block *)(left));

	left->this_size += right->this_size;

	if (HEAP_CHECKS) {
		// the right block is going away, so make sure it no longer
		// looks like a real block
		right->this_size = 0;
		right->prev_size = 0;
		right->checksum = 0;
	}

	return left;
}

static
struct heap_block *
merge_blocks_right(struct heap_block *left, struct heap_block *right)
{
	free_list_remove((struct free_block *)(right));

	left->this_size += right->this_size;

	if (HEAP_CHECKS) {
		// the right block is going away, so make sure it no longer
		// looks like a real block
		right->this_size = 0;
		right->prev_size = 0;
		right->checksum = 0;
	}

	return left;
}

static
void
fixup_next_after_free(struct heap_block *block)
{
	struct heap_block *next;

	next = next_block(block);
	verify_block_checksum(next);

	next->prev_free = 1;
	next->prev_size = block->this_size;

	calculate_block_checksum(next);
}

void *
heap_malloc(size_t size, const char *caller_name)
{
	size_t size_r;
	size_t size_d;
	struct heap_block *retval;

	/* must allocate at least one byte */
	if (size < 1) {
#if HEAP_ERROR_PANIC
		panic("heap_malloc must allocate at least one byte");
#endif
		return 0;
	}
	enter_critical_section();
	
	size_r = required_size(size);
	if (HEAP_DEBUGGING) {
		size_d = round_size(sizeof(struct client_data));
	} else {
		size_d = 0;
	}
	retval = grab_chunk(size_r + size_d, size);

	free_mem -= sizeof_block(retval);

	if (HEAP_DEBUGGING) {
		struct client_data *client;

		client = recess_pointer(next_block(retval), size_d);

		client->block_type = HEAP_BLOCK__MALLOC;
		client->requestor  = __return_address(0);
		client->requestor_name = caller_name ? caller_name : "unknown";
		client->timestamp  = system_time();
		client->user_size  = size;
		client->alignment  = 0;
	}

	TRACE(
		"malloc(%zx, %s) %p -- %p [0x%zx bytes]\n",
		size, caller_name ? : "unknown",
		retval+1,
		(uint8_t *)(retval+1) + sizeof_block(retval) - sizeof(struct heap_block),
		sizeof_block(retval) - sizeof(struct heap_block)
	);

	exit_critical_section();

	if (retval) {
		retval += 1;
		memset(retval, 0, size);
		return retval;
	} else {
		return NULL;
	}
}

void *heap_calloc(size_t count, size_t size, const char *caller_name)
{

	// Check for potential 32-bit overflow
	if (size && count > (UINT32_MAX / size)) {
#if HEAP_ERROR_PANIC
		panic("size * count will overflow 32-bits");
#endif
		return NULL;
	}

	// malloc zeroes all returned buffers, no need to do it here
	return heap_malloc(count * size, caller_name);
}

void *heap_realloc(void *ptr, size_t size, const char *caller_name)
{
	struct heap_block *old_block = ((struct heap_block *)ptr) - 1;
	void *retval = ptr;
	size_t size_o = 0, size_r, size_d;

	enter_critical_section();

	if (NULL == ptr) {
		retval = heap_malloc(size, caller_name);
	} else if (0 == size) {
#if HEAP_ERROR_PANIC
		panic("size should not be zero");
#endif
		retval = heap_malloc(1, caller_name);
		heap_free(ptr);
	} else {
		verify_block_checksum(old_block);

		size_r = required_size(size);
		if (HEAP_DEBUGGING) {
			size_d = round_size(sizeof(struct client_data));
		} else {
			size_d = 0;
		}
		size_o = sizeof_block(old_block);
		if (size_o < size_r + size_d) {
			// pass size_r here because heao_malloc will add size_d
			retval = heap_malloc(size_r, caller_name);
			if (NULL != retval) {
				memcpy(retval, ptr, (size_o<size)?size_o:size);
				heap_free(ptr);
			}
#if HEAP_ERROR_PANIC
			else {
				panic("heap_malloc returned null");
			}
#endif
		} else {
			if (HEAP_DEBUGGING) {
				struct client_data *client;

				client = recess_pointer(next_block(old_block), size_d);

				client->timestamp  = system_time();
				client->user_size  = size;
			}

			// we're returning the same block, but need to adjust the amount
			// of padding for when we verify it. only adjust downward so that
			// we don't have to fill the buffer with padding bytes if the used
			// size shrank. the padding doesn't provide real security because
			// the pattern is predictable
#if HEAP_CHECKS
			uint8_t *pad_start = advance_pointer(old_block + 1, size);
			uint8_t *pad_end = (uint8_t *)next_block(old_block);
			size_t padding;
			
#if HEAP_DEBUGGING
			pad_end -= round_size(sizeof(struct client_data));
#endif
			padding = (pad_end - pad_start);

			if (padding < old_block->padding_bytes) {
				old_block->padding_bytes = padding;
				old_block->padding_start = pad_start - (uint8_t *)old_block;

				calculate_block_checksum(old_block);
			}
#endif
		}
	}

	TRACE(
		"realloc(%p, %zx, %s) %p -- %p\n",
		ptr, size,
		caller_name ? : "unknown",
		retval,
		ptr);

	exit_critical_section();

	return retval;
}

void *
heap_memalign(size_t size, size_t constraint, const char *caller_name)
{
	size_t size_r;
	size_t size_d;
	struct heap_block *retval;

	/* must allocate at least one byte */
	if (size < 1) {
#if HEAP_ERROR_PANIC
		panic("_memalign must allocate at least one byte");
#endif
		return 0;
	}

	if (constraint & (constraint-1)) {
		/* alignment must be a power of two */
#if HEAP_ERROR_PANIC
		panic("_memalign must be a power of two");
#endif
		return 0;
	}

	if (constraint < sizeof(struct free_block)) {
		constraint = sizeof(struct free_block);
	}

	enter_critical_section();

	size_r = required_size(size);
	if (HEAP_DEBUGGING) {
		size_d = round_size(sizeof(struct client_data));
	} else {
		size_d = 0;
	}
	retval = grab_chunk__constrained(size_r + size_d, constraint, size);

	free_mem -= sizeof_block(retval);

	if (HEAP_DEBUGGING) {
		struct client_data *client;

		client = recess_pointer(next_block(retval), size_d);

		client->block_type = HEAP_BLOCK__MEMALIGN;
		client->requestor  = __return_address(0);
		client->timestamp  = system_time();
		client->user_size  = size;
		client->alignment  = constraint;
	}

	TRACE(
		"memalign(%zx, %zx, %s) %p -- %p [0x%zx bytes]\n",
		size, constraint, caller_name ? : "unknown",
		retval+1,
		(uint8_t *)(retval+1) + sizeof_block(retval) - sizeof(struct heap_block),
		sizeof_block(retval) - sizeof(struct heap_block)
	);

	exit_critical_section();

	if (retval) {
		retval += 1;
		memset(retval, 0, size);
		return retval;
	} else {
		return NULL;
	}
}

int
heap_posix_memalign(void **memptr, size_t constraint, size_t size, const char *caller_name)
{
	void *result;

	if (size < 1) {
#if HEAP_ERROR_PANIC
		panic("posix_memalign must allocate at least one byte");
#endif
		return EINVAL;
	}

	if (constraint & (constraint-1)) {
		/* alignment must be a power of two */
#if HEAP_ERROR_PANIC
		panic("posix_memalign must be a power of two");
#endif
		return EINVAL;
	}

	result = heap_memalign(size, constraint, caller_name);
	
	if (result == 0)
		return ENOMEM;

	*memptr = result;
	return 0;
}

void
heap_free(void *ptr)
{
	struct heap_block *block;
	struct heap_block *left;
	struct heap_block *right;

	if (!ptr) {
		return;
	}

	enter_critical_section();

	block = ptr;
	block -= 1;

	free_mem += sizeof_block(block);

	TRACE(
		"free %p -- %p [0x%zx bytes]\n",
		block+1,
		(uint8_t *)(block+1) + sizeof_block(block) - sizeof(struct heap_block),
		sizeof_block(block) - sizeof(struct heap_block)
	);

	verify_block_checksum(block);

	left = prev_block(block);
	right = next_block(block);

	if (HEAP_CHECKS) {
		if (right == block) {
			HEAP_PANIC(block, "unlink error this_size");
		}
		if (left == block) {
			HEAP_PANIC(block, "unlink error prev_size");
		}
		if (prev_block(right) != block) {
			HEAP_PANIC(block, "unlink error next's prev_size");
		}
		if (next_block(left) != block) {
			HEAP_PANIC(block, "unlink error prev's this_size");
		}
		if (block->this_free) {
			HEAP_PANIC(block, "double free");
		}

		if (block->prev_free && !left->this_free) {
			HEAP_PANIC(block, "free bit corrupted");
		}

		verify_block_padding(block);

		// zero the block to detect use after free
		memset(block + 1, 0, sizeof_block(block) - sizeof(*block));

		block->this_free = 1;
	}

	if (block->prev_free) {
		verify_block_checksum(left);
		block = merge_blocks_left(left, block);
	}

	verify_block_checksum(right);
	if (right->this_free) {
		block = merge_blocks_right(block, right);
	}

	fixup_next_after_free(block);

	block->this_free = 1;
	free_list_add((struct free_block *)block);
	calculate_block_checksum(block);

	exit_critical_section();
}


void
heap_add_chunk(void *chunk, size_t size, bool clear_memory)
{
	struct heap_block *front_sentinel;
	struct heap_block *slab;
	struct heap_block *back_sentinel;
	uintptr_t chunk_base;
	size_t original_size;
	size_t minimum_size_for_slide;

#if HEAP_CHECKS
	ASSERT(sizeof(struct heap_block) == HEAP_BLOCK_SIZE);
#endif

	if (chunk_count >= NUM_CHUNKS)
		panic("too many heap chunks (max %d)", NUM_CHUNKS);

	/* unless the caller knowns the chunk is all zeroes, clear it */
	if (clear_memory)
		bzero(chunk, size);

	/* align the passed-in chunk and size */
	original_size = size;
	chunk_base = ROUNDUP((uintptr_t)chunk, sizeof(struct heap_block));
	size -= chunk_base - (uintptr_t)chunk;
	chunk = (void *)chunk_base;
	size = round_size(size);

	/* check to make sure that it's larger than the required slide amount */
#ifndef HEAP_UNITTEST
	minimum_size_for_slide = mib_get_size(kMIBTargetMinimumHeapSizeForSlide);
#else
	minimum_size_for_slide = MINIMUM_HEAP_SIZE_FOR_SLIDE;
#endif

	/* if it is, we will slide it around */
	if(size > minimum_size_for_slide) {
		uint64_t seed = g_heap_cookie[0];
		uint64_t slide_block_size = (HEAP_ENTROPY_MASK * HEAP_BLOCK_SIZE);
		uint32_t slide = (seed & HEAP_ENTROPY_MASK) * HEAP_BLOCK_SIZE;

		/* block size must never be larger than the minimum slide size */
		RELEASE_ASSERT(slide_block_size < minimum_size_for_slide);

		/* cut the heap size slightly in order to slide it around. */
		chunk_base += slide;
		chunk = (void *)chunk_base;
		size -= slide_block_size;
	}

	/* verify no underflow */
	if (size > original_size)
		panic("heap chunk size underflowed");

	/* need room for the front and back sentinels, plus
	   the actual free memory block */
	if (size <= 3 * sizeof(struct heap_block))
		panic("heap chunk too small");

	/* build out the chunk */
	lead[chunk_count].chunk_base = chunk;
	lead[chunk_count].chunk_size = size;
	chunk_count++;

	/* front sentinel */
	front_sentinel = chunk;
	front_sentinel->prev_free = 0;
	front_sentinel->this_free = 0;
	front_sentinel->prev_size = 0;
	front_sentinel->this_size = 1;
	front_sentinel->padding_bytes = 0;
	calculate_block_checksum(front_sentinel);

	/* free space */
	slab = next_block(front_sentinel);
	slab->prev_free = 0;
	slab->this_free = 0;
	slab->prev_size = 1;
	slab->this_size = quantify_size(size) - 2;
	slab->padding_bytes = 0;
	calculate_block_checksum(slab);

	/* back sentinel */
	back_sentinel = next_block(slab);
	back_sentinel->prev_free = 0;
	back_sentinel->this_free = 0;
	back_sentinel->prev_size = slab->this_size;
	back_sentinel->this_size = 1;
	back_sentinel->padding_bytes = 0;
	calculate_block_checksum(back_sentinel);

	/* free the free space back into the heap */
	heap_free(slab + 1);
}

void
heap_set_cookie(uint8_t *cookie)
{
	RELEASE_ASSERT(!heap_is_initialized());
	memcpy(g_heap_cookie, cookie, sizeof(g_heap_cookie));
}

u_int32_t
heap_get_free_mem(void)
{
	return free_mem;
}

bool
heap_is_initialized(void)
{
	return (chunk_count != 0);
}

static
int
heap_walk(heap_walker_f *walker, void *cookie)
{
	int retval, chunk;
	struct heap_block    *iter;
	struct walker_info_t  winfo;

	retval = 0;

	for (chunk = 0; chunk < chunk_count; chunk++) {
		
		iter = next_block(lead[chunk].chunk_base);

		while (iter && (retval >= 0)) {

			if (iter->this_free) {
				winfo.block_type = HEAP_BLOCK__FREE;
				winfo.extended_info = 0;
			} else if (iter->this_size == 1) {
				/* only sentinels can have size 1 */
				winfo.block_type = HEAP_BLOCK__SENTINEL;
				winfo.extended_info = 0;
			} else {
				/* assume malloc for now */
				winfo.block_type = HEAP_BLOCK__MALLOC;
				winfo.extended_info = HEAP_DEBUGGING;
			}
			winfo.raw_block = iter;
			winfo.block_size = sizeof_block(iter);


			if (winfo.block_type == HEAP_BLOCK__MALLOC) {
				winfo.user_block = iter + 1;
				winfo.user_size  = sizeof_block(iter) - sizeof(struct heap_block);
				winfo.timestamp  = 0;
			}

			if (HEAP_DEBUGGING && (winfo.block_type == HEAP_BLOCK__MALLOC)) {
				size_t size_d;
				struct client_data *client;

				size_d = round_size(sizeof(struct client_data));

				client = recess_pointer(next_block(iter), size_d);

				winfo.block_type = client->block_type;
				winfo.user_size  = client->user_size;
				winfo.timestamp  = client->timestamp;
				winfo.requestor  = client->requestor;
				winfo.alignment  = client->alignment;
			}

			if (winfo.block_type == HEAP_BLOCK__SENTINEL) {
				/* just in case some idiotic user modifies winfo, update loop iterator before calling */
				iter = 0;
			} else {
				iter = next_block(iter);
			}
			retval = walker(cookie, &winfo);
		}
	}

	return 0;
}

static
int
heap_verify_callback(void *cookie, struct walker_info_t const *info)
{
	struct heap_block *b = info->raw_block;
	verify_block_checksum(b);

	// if the block is not free and not a sentinel, it needs to
	// have valid padding
	if (!b->this_free && b->this_size > 1) {
		verify_block_padding(b);
	}

	return 0;
}

void
heap_verify(void)
{
	heap_walk(heap_verify_callback, NULL);
}

#if HEAP_CHECKS
static
void
heap_panic(char const *func, const void *block, const char *reason)
{
	if (HEAP_PANIC_CHECKS) {
		panic("%s: heap error: block %p, %s\n", func, block, reason);
	} else {
		printf("%s: heap error: %s : please enable HEAP_PANIC_CHECKS\n", func, reason);
	}
}
#endif

#if DEBUG_BUILD || HEAP_ERROR_DUMP
struct malloc_summary {
	uint32_t	bytes_allocated;
	uint32_t	bytes_free;
	uint32_t	allocations;
};
	
static int heap_dump_callback(void *cookie, struct walker_info_t const *info)
{
	struct malloc_summary *summary;

	summary = (struct malloc_summary *)cookie;
	summary->allocations++;
	
	switch (info->block_type) {
	case HEAP_BLOCK__FREE:
		printf("  %p 0x%zx (free)\n", info->raw_block, info->block_size);
		summary->bytes_free += info->block_size;
		break;
	case HEAP_BLOCK__MALLOC:
		printf("  %p 0x%zx malloc  @ %p", info->raw_block, info->block_size, info->user_block);
		if (info->extended_info) {
			printf("  size 0x%zx  time %llu  requestor %p\n",
			    info->user_size, info->timestamp, info->requestor);
		}
		printf("\n");
		summary->bytes_allocated += info->block_size;
		break;
	case HEAP_BLOCK__MEMALIGN:
		printf("  %p 0x%zx aligned @ %p", info->raw_block, info->block_size, info->user_block);
		if (info->extended_info) {
			printf("  alignment 0x%zx  size 0x%zx  time %llu  requestor %p\n",
			    info->alignment, info->user_size, info->timestamp, info->requestor);
		}
		printf("\n");
		summary->bytes_allocated += info->block_size;
		break;
	case HEAP_BLOCK__SENTINEL:
		break;
	default:
		break;
	}
	return(0);
}

/* dump the heap */
static void heap_dump(void)
{
	struct malloc_summary	summary;

	summary.bytes_allocated = 0;
	summary.bytes_free = 0;
	summary.allocations = 0;

	printf("dumping heap\n");
	
	heap_walk(heap_dump_callback, (void *)&summary);

	printf("%d allocations totalling %d bytes in use, %d bytes free\n",
	    summary.allocations, summary.bytes_allocated, summary.bytes_free);
}
#endif //

#if DEBUG_BUILD
/* dump the heap */
int do_malloc(int argc, struct cmd_arg *args)
{
	heap_dump();
	return(0);
}

#endif // DEBUG_BUILD

