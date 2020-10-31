/*
 * Copyright (C) 2006 Apple Computer, Inc. All rights reserved.
 *
 * This document is the property of Apple Computer, Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Computer, Inc.
 */

#include <debug.h>
#include <compiler.h>
#include <lib/partition.h>
#include <lib/blockdev.h>

#include "partition_private.h"

int
mbr_scan(struct mbr_partition *part, struct blockdev *dev, struct partition_entry *entry_list)
{
	uint32_t i;
	int scanned_entries = 0;

	for (i=0; i < MBR_PARTITIONS; i++) {
		dprintf(DEBUG_SPEW, "MBR part %d: id 0x%x, startlba %d, sizelba %d\n", i, part[i].sysid, part[i].startlba, part[i].size);

		/* needs to be spiffed up a bit, but detect based on id */
		if (part[i].sysid != 0) {
			off_t offset;
			off_t len;

			offset = (off_t)part[i].startlba << dev->block_shift;
			len = (off_t)part[i].size << dev->block_shift;

			entry_list[scanned_entries].id = part[i].sysid;
			entry_list[scanned_entries].valid = true;
			entry_list[scanned_entries].offset = offset;
			entry_list[scanned_entries].len = len;
			dprintf(DEBUG_SPEW, "MBR part %d: id 0x%x, off 0x%llx, size 0x%llx, blocksize %d\n", scanned_entries, part[i].sysid, 
					entry_list[scanned_entries].offset, entry_list[scanned_entries].len, dev->block_size);
			scanned_entries++;
		}
	}

	return scanned_entries;
}
