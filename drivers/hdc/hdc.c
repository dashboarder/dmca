/*
 * Copyright (C) 2007-2011 Apple Inc. All rights reserved.
 * Copyright (C) 2006 Apple Computer, Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */
#include <sys.h>
#include <debug.h>
#include <lib/heap.h>
#include <lib/blockdev.h>
#include <lib/partition.h>
//#include <platform/clocks.h>
//#include <sys/menu.h>
#include "hdc_private.h"

static int hdc_read_block_hook(struct blockdev *bdev, void *ptr, block_addr block, uint32_t count)
{
	register uint32_t* dest = (uint32_t*)ptr;

	/* sanity-check the starting block and count */
	if ((block >= bdev->block_count) || (count > bdev->block_count) || (NULL == ptr))
		return 0;

	/* adjust the operation length if it overruns */
	if (block > (bdev->block_count - count))
		count = bdev->block_count - block;

	/* Reset HDC? */
	for (uint32_t block_start = block; block_start < (block + count); block_start++) {
		uint32_t hdc_cmd = 0;

		hdc_cmd = HDC_SET_CMD_CMD(hdc_cmd, HDC_CMD_READ);
		hdc_cmd = HDC_SET_CMD_CMD(hdc_cmd, HDC_CMD_INT);
		hdc_cmd = HDC_SET_CMD_DISK(hdc_cmd, 0);
		hdc_cmd = HDC_SET_CMD_LEN(hdc_cmd, 0);

		rHDC_DMA   = (uint32_t)dest;
		rHDC_FIRST = block_start;
		rHDC_CMD   = hdc_cmd;

		while (HDC_ST_DONE != HDC_GET_STATUS_ST(rHDC_STATUS))
			;
		dest += (HDC_BLKSIZE / 4);
	}

	return count;
}

int hdc_init(void)
{
	struct blockdev *hdc;
	off_t len;
	size_t block_size = (HDC_BLKSIZE);

	hdc = malloc(sizeof(struct blockdev));
	if (!hdc)
		panic("error allocation hdc block device\n");

	len = (rHDC_DRV_SIZE) * HDC_BLKSIZE;
	construct_blockdev(hdc, "hdc0", len, block_size);

	hdc->read_block_hook = &hdc_read_block_hook;

	register_blockdev(hdc);

	/* scan ourselfs for partitions and publish subdevices */
	partition_scan_and_publish_subdevices("hdc0");

	return 0;
}


