/*
 * Copyright (C) 2007-2010 Apple Inc. All rights reserved.
 * Copyright (C) 2006 Apple Computer, Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */
#include <debug.h>
#include <drivers/flash_nor.h>
#include <lib/blockdev.h>
#include <sys.h>

static int nor_blockdev_read_block(struct blockdev *bdev, void *ptr, block_addr block, uint32_t count)
{
	struct nor_blockdev *ndev = (struct nor_blockdev *)bdev;
	int err;

	/* sanity-check the starting block and count */
	if ((block >= ndev->bdev.block_count) || (count > ndev->bdev.block_count))
		return 0;

	/* adjust the operation length if it overruns */
	if (block > (ndev->bdev.block_count - count))
		count = ndev->bdev.block_count - block;

//	dprintf(DEBUG_SPEW, "nor_blockdev_read: dev %p, block %d, count %d\n", bdev, block, count);

	uint32_t cb = count << ndev->bdev.block_shift;
	err = ndev->readRange(ndev->handle, ptr, block << ndev->bdev.block_shift, cb);
	if (err < 0) return err;

	return count;
}

#if !READ_ONLY

static int nor_blockdev_erase_hook(struct blockdev *bdev, off_t offset, uint64_t len)
{
	struct nor_blockdev *ndev = (struct nor_blockdev *)bdev;
	int err;

	RELEASE_ASSERT(offset >= 0);
	if ((uint64_t)offset >= ndev->bdev.total_len)
		return 0;
	if ((offset + len) > ndev->bdev.total_len)
		len = ndev->bdev.total_len - offset;

	uint32_t cb = len;
	err = ndev->eraseRange(ndev->handle, offset, cb);
	if (err < 0) return err;

	return 0;
}

static int nor_blockdev_write_hook(struct blockdev *bdev, const void *ptr, block_addr block, uint32_t count)
{
	struct nor_blockdev *ndev = (struct nor_blockdev *)bdev;
	int err;
	
	if (block >= ndev->bdev.block_count)
		return 0;
	if ((block + count) > ndev->bdev.block_count)
		count = ndev->bdev.block_count - block;

	uint32_t cb = count << ndev->bdev.block_shift;
	err = ndev->writeRange(ndev->handle, ptr, block << ndev->bdev.block_shift, cb);
	if (err < 0) return err;

	return count;
}

#endif /* ! READ_ONLY */

int flash_nor_register(struct nor_blockdev *nor_bdev, const char *name, uint64_t len, uint32_t block_size)
{
	/* create the block device */
	construct_blockdev(&nor_bdev->bdev, name, len, block_size);
	nor_bdev->bdev.read_block_hook = &nor_blockdev_read_block;
#if !READ_ONLY
	nor_bdev->bdev.write_block_hook = &nor_blockdev_write_hook;
	nor_bdev->bdev.erase_hook = &nor_blockdev_erase_hook;
#endif

	/* register it in the block device namespace */
	register_blockdev((struct blockdev *)nor_bdev);

	return 0;
}

int flash_nor_init(int which_device)
{
	int ret_spi = -1;
	
#if WITH_HW_FLASH_NOR_SPI
	extern int flash_spi_init(int which_bus);

	ret_spi = flash_spi_init(which_device);
#else
# error Must configure a supported NOR flash interface.
#endif

	if (ret_spi < 0) {
		dprintf(DEBUG_INFO, "flash_nor_init: failed to find any nor devices\n");
		return -1;
	}

	return 0;
}
