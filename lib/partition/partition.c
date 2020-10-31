/*
 * Copyright (C) 2007, 2009-2011, 2014 Apple Inc. All rights reserved.
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
#include <compiler.h>
#include <lib/mib.h>
#include <lib/partition.h>
#include <lib/blockdev.h>

#include "partition_private.h"

static bool
partition_overlap_check(struct partition_entry *part, int num_entries)
{
	int i, j;
	bool overlap;

	overlap = false;
	for (i=0; i < num_entries; i++) {
		off_t ioff = part[i].offset;
		off_t iend = part[i].offset + part[i].len;
		for (j=0; j < num_entries; j++) {
			if (i == j)
				continue;

			off_t joff = part[j].offset;
			off_t jend = part[j].offset + part[j].len;

			/* is j's starting offset within i's range */
			if ((joff >= ioff) && (joff < iend))
				overlap = true;
	
			/* is j's end within i's range */
			if ((jend <= iend) && (jend > ioff))
				overlap = true;

			/* does j completely cover i */
			if ((joff <= ioff) && (jend >= iend))
				overlap = true;
		}
	}

	if (overlap)
		dprintf(DEBUG_INFO, "partition_scan: some partitions overlap, probably garbage partition data\n");
	return overlap;
}

static bool
partition_extents_check(struct partition_entry *part, int num_entries, struct blockdev *dev)
{
	int i;
	bool toobig;

	toobig = false;
	for (i=0; i < num_entries; i++) {
		if (part[i].offset + part[i].len > dev->total_len) {
			dprintf(DEBUG_INFO, "partition %i overshoots size of device\n", i);
			toobig = true;
		}
	}

	return toobig;
}

static int
partition_scan(struct blockdev *dev, struct partition_entry *entry_list, size_t num_entries)
{
	int scanned_entries;
	int err;
	struct mbr_partition *part;
	int mbr_bytes;

	scanned_entries = 0;
	part = NULL;
	mbr_bytes = MBR_PARTITIONS * sizeof(struct mbr_partition);

	/* read what we expect to be an MBR from the beginning of the blockdev */
	part = memalign(mbr_bytes, mib_get_size(kMIBPlatformCacheLineSize));

	/* read *just* the partition table */
	err = blockdev_read(dev, part, MBR_ADDRESS, mbr_bytes);
	if (err != mbr_bytes) {
		dprintf(DEBUG_INFO, "mbr: read error\n");
		goto out;
	}

	/* scan it as an mbr */
	scanned_entries = mbr_scan(part, dev, entry_list);

       /* does it look like it might be GPT? */
	if ((scanned_entries == 1) && (entry_list[0].id == 0xee))
		scanned_entries = gpt_scan(dev, entry_list);

	/* do an overlap check to make sure this is a sort of reasonable layout */
	if (partition_overlap_check(entry_list, scanned_entries)) {
		scanned_entries = 0;
		goto out;
	}
out:
	if (NULL != part)
		free(part);
	return scanned_entries;
}

int partition_scan_and_publish_subdevices(const char *dev_name)
{
	struct partition_entry entries[MAX_PARTITIONS];
	int found;
	int i;
	struct blockdev *dev;

	dev = lookup_blockdev(dev_name);
	if (!dev)
		return -1;

	/* scan for LwVM partitions */
	found = lwvm_scan(dev);
	if (found > 0)
		return found;

	/* scan for regular partitions */
	found = partition_scan(dev, entries, MAX_PARTITIONS);
	for (i=0; i < found; i++) {
		/* publish subdevices */
		struct blockdev *subdev;
		char subdev_name[32];

		snprintf(subdev_name, 32, "%s%c", dev_name, 'a' + i);

		// make sure it doesn't already exist
		if (lookup_blockdev(subdev_name))
			continue;

		subdev = create_subdev_blockdev(subdev_name, dev, entries[i].offset, entries[i].len, dev->block_size);
		if (subdev)
			register_blockdev(subdev);
	}

	return found;
}
