/*
 * Copyright (C) 2007-2011 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */

#include "sw_h2fmi_private.h"

#include <debug.h>
#include <drivers/sw_h2fmi.h>
#include <lib/blockdev.h>
#include <sys.h>

static int sw_h2fmi_blockdev_read_block(struct blockdev *bdev, void *ptr, block_addr block, uint32_t count)
{
	uint32_t addr_size;
	int ret = 0;
	uint32_t page_start, page_end;
	register uint32_t* dest = (uint32_t*)ptr;

	/* sanity-check the starting block and count */
	if ((block >= bdev->block_count) || (count > bdev->block_count) || (NULL == ptr))
		return ret;

	/* adjust the operation length if it overruns */
	if (block > (bdev->block_count - count))
		count = bdev->block_count - block;

	addr_size = 4; // Any number  > 0 will do
	page_start = (block / 3);
	page_end = ((block + count + 2) / 3);

	/* Reset the device */
	rFASTSIM_H2FMI_CMD = FASTSIM_H2FMI_RESET;

	for (uint32_t page = page_start; page < page_end; page++) {
		uint32_t block_addr2, block_addr1, block_addr0;
		const uint32_t words_per_loop = 8;

		block_addr2 = ((page) >> 16) & 0xFF;
		block_addr1 = ((page) >> 8) & 0xFF;
		block_addr0 = ((page) >> 0) & 0xFF;

		/* Send the offset to device */
		rFASTSIM_H2FMI_ADDR0 = ((block_addr1 << 24) | (block_addr0 << 16));
		rFASTSIM_H2FMI_ADDR1 = block_addr2;
		rFASTSIM_H2FMI_ADDRNUM = (addr_size & 0x7);

		/* Roll in the data */
		for (uint32_t sector = 0; sector < FASTSIM_H2FMI_BLOCK_PER_PAGE; sector++) {
			register uint32_t index = ((FASTSIM_H2FMI_BLOCK_SIZE / sizeof(uint32_t)) / words_per_loop);
			do {
				register uint32_t tmp0;
				register uint32_t tmp1;
				register uint32_t tmp2;
				register uint32_t tmp3;
				register uint32_t tmp4;
				register uint32_t tmp5;
				register uint32_t tmp6;
				register uint32_t tmp7;

				tmp0 = rFASTSIM_H2FMI_DATA_BUF;
				tmp1 = rFASTSIM_H2FMI_DATA_BUF;
				tmp2 = rFASTSIM_H2FMI_DATA_BUF;
				tmp3 = rFASTSIM_H2FMI_DATA_BUF;
				tmp4 = rFASTSIM_H2FMI_DATA_BUF;
				tmp5 = rFASTSIM_H2FMI_DATA_BUF;
				tmp6 = rFASTSIM_H2FMI_DATA_BUF;
				tmp7 = rFASTSIM_H2FMI_DATA_BUF;

				if ((((page * 3) + sector) >= block) && (((page * 3) + sector) < (block + count))) {
					*dest++ = tmp0;
					*dest++ = tmp1;
					*dest++ = tmp2;
					*dest++ = tmp3;
					*dest++ = tmp4;
					*dest++ = tmp5;
					*dest++ = tmp6;
					*dest++ = tmp7;
				}
			} while (--index);
		}
	}

	return count;
}

/* to-do: write and erase */

int sw_h2fmi_init(void)
{
	struct blockdev *sw_h2fmi_dev;

	sw_h2fmi_dev = malloc(sizeof(struct blockdev));
	if (sw_h2fmi_dev == NULL) {
		dprintf(DEBUG_CRITICAL, "sw_h2fmi_init: failed to create blockdev\n");
		return -3;
	}
	memset(sw_h2fmi_dev, 0, sizeof(struct blockdev));

	/* create the block device */
	construct_blockdev(sw_h2fmi_dev, "sw-h2fmi", FASTSIM_H2FMI_BLOCK_SIZE * FASTSIM_H2FMI_BLOCK_NUMBER, FASTSIM_H2FMI_BLOCK_SIZE);
	sw_h2fmi_dev->read_block_hook = &sw_h2fmi_blockdev_read_block;

	/* register it in the block device namespace */
	register_blockdev(sw_h2fmi_dev);

	return 0;
}

