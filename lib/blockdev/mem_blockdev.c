/*
 * Copyright (C) 2006-2011 Apple Computer, Inc. All rights reserved.
 *
 * This document is the property of Apple Computer, Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Computer, Inc.
 */
#include <debug.h>
#include <sys.h>
#include <string.h>
#include <lib/blockdev.h>
#include <stdlib.h>

struct mem_blockdev {
	struct blockdev bdev;

	/* local data */
	void *ptr;
};

static int mem_blockdev_read_block(struct blockdev *bdev, void *ptr, block_addr block, uint32_t count)
{
	struct mem_blockdev *mdev = (struct mem_blockdev *)bdev;

	// no overflow
	RELEASE_ASSERT(block + count >= block);

	if (block >= mdev->bdev.block_count)
		return 0;
	if ((block + count) > mdev->bdev.block_count)
		count = mdev->bdev.block_count - block;

	dprintf(DEBUG_SPEW, "mem_blockdev_read: dev %p, ptr %p, block %d, count %d\n", bdev, ptr, block, count);

	memcpy(ptr, (uint8_t *)mdev->ptr + (block << mdev->bdev.block_shift), count << mdev->bdev.block_shift);

	return count;
}

static int mem_blockdev_read(struct blockdev *bdev, void *ptr, off_t offset, uint64_t len)
{
	struct mem_blockdev *mdev = (struct mem_blockdev *)bdev;

	RELEASE_ASSERT(offset >= 0);
	// no overflow
	RELEASE_ASSERT((uint64_t)offset + len >= (uint64_t)offset);

	if ((uint64_t)offset >= mdev->bdev.total_len)
		return 0;
	if ((offset + len) > mdev->bdev.total_len)
		len = mdev->bdev.total_len - offset;

	dprintf(DEBUG_SPEW, "mem_blockdev_read: dev %p, ptr %p, off %lld, len %llu\n", bdev, ptr, offset, len);

	memcpy(ptr, (uint8_t *)mdev->ptr + offset, len);

	return len;
}

struct blockdev *create_mem_blockdev(const char *name, void *ptr, uint64_t len, uint32_t block_size)
{
	struct mem_blockdev *mdev;

	mdev = malloc(sizeof(struct mem_blockdev));

	construct_blockdev(&mdev->bdev, name, len, block_size);
	mdev->ptr = ptr;

	mdev->bdev.read_hook = &mem_blockdev_read;
	mdev->bdev.read_block_hook = &mem_blockdev_read_block;

	return &mdev->bdev;
}

