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

#define MAX_PARTITIONS          8

struct partition_entry {
	bool        valid;
	uint32_t    id;
	off_t       offset;
	uint64_t    len;
};

extern int gpt_scan(struct blockdev *dev, struct partition_entry *entry_list);

#define MBR_PARTITIONS  4
#define MBR_ADDRESS     0x1be

struct mbr_partition {
	uint8_t bootid;
	uint8_t starthead;
	uint8_t startsect;
	uint8_t startcyl;
	uint8_t sysid;
	uint8_t endhead;
	uint8_t endsect;
	uint8_t endcyl;
	uint32_t startlba;
	uint32_t size;
} __packed;

extern int mbr_scan(struct mbr_partition *part, struct blockdev *dev, struct partition_entry *entry_list);

extern int lwvm_scan(struct blockdev *dev);
