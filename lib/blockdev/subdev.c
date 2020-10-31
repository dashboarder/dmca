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
#include <lib/blockdev.h>
#include <stdlib.h>

struct subdev_blockdev {
	struct blockdev bdev;

	/* local data */
	struct blockdev *parent_dev;
	off_t offset;
};

static int subdev_blockdev_read(struct blockdev *bdev, void *ptr, off_t offset, uint64_t len)
{
	struct subdev_blockdev *sdev = (struct subdev_blockdev *)bdev;

	RELEASE_ASSERT(offset >= 0);
	if ((uint64_t)offset >= sdev->bdev.total_len)
		return 0;
	if ((offset + len) > sdev->bdev.total_len)
		len = sdev->bdev.total_len - offset;

	return blockdev_read(sdev->parent_dev, ptr, offset + sdev->offset, len);
}

static int subdev_blockdev_read_block(struct blockdev *bdev, void *ptr, block_addr block, uint32_t count)
{
	struct subdev_blockdev *sdev = (struct subdev_blockdev *)bdev;

	/* since our blocks might not be aligned on parent block boundaries, just punch through to the
	 * arbitrary alignment read handler and let it deal with it
	 */
	return subdev_blockdev_read(bdev, ptr, block << sdev->bdev.block_shift, count << sdev->bdev.block_shift);
}

static int subdev_blockdev_write(struct blockdev *bdev, const void *ptr, off_t offset, uint64_t len)
{
	struct subdev_blockdev *sdev = (struct subdev_blockdev *)bdev;

	RELEASE_ASSERT(offset >= 0);
	if ((uint64_t)offset >= sdev->bdev.total_len)
		return 0;
	if ((offset + len) > sdev->bdev.total_len)
		len = sdev->bdev.total_len - offset;

	return blockdev_write(sdev->parent_dev, ptr, offset + sdev->offset, len);
}

static int subdev_blockdev_write_block(struct blockdev *bdev, const void *ptr, block_addr block, uint32_t count)
{
	struct subdev_blockdev *sdev = (struct subdev_blockdev *)bdev;

	/* since our blocks might not be aligned on parent block boundaries, just punch through to the
	 * arbitrary alignment write handler and let it deal with it
	 */
	return subdev_blockdev_write(bdev, ptr, block << sdev->bdev.block_shift, count << sdev->bdev.block_shift);
}

struct blockdev *create_subdev_blockdev(const char *name, struct blockdev *parent, off_t off, uint64_t len, uint32_t block_size)
{
	struct subdev_blockdev *dev;

	ASSERT(parent);

	dprintf(DEBUG_SPEW, "create_subdev: name '%s' parent '%s' off 0x%llx len 0x%llx bsize 0x%x\n",
			name, parent->name, off, len, block_size);

	dev = malloc(sizeof(struct subdev_blockdev));

	construct_blockdev(&dev->bdev, name, len, block_size);
	blockdev_set_buffer_alignment(&dev->bdev, parent->alignment);
	dev->parent_dev = parent;
	dev->offset = off;

	dev->bdev.read_hook = &subdev_blockdev_read;
	dev->bdev.read_block_hook = &subdev_blockdev_read_block;
	dev->bdev.write_hook = &subdev_blockdev_write;
	dev->bdev.write_block_hook = &subdev_blockdev_write_block;

	return (struct blockdev *)dev;
}

