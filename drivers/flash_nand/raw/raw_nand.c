/*
 * Copyright (c) 2007-2009 Apple Inc. All rights reserved.
 * Copyright (c) 2006 Apple Computer, Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */
#include <debug.h>
#include <lib/heap.h>
#include <lib/blockdev.h>
#include <lib/partition.h>
#include <platform/clocks.h>
#include <sys/menu.h>

#include "WMRConfig.h" 
#include "ANDTypes.h"
#include "FIL.h"
#include "VFLBuffer.h"
#include "VFL.h"
#include "FTL.h"
#include "WMRExam.h"
#include "WMROAM.h"

extern uint32_t nand_fsys_block_offset;

static int nand_filesys_init(uint32_t block_size, uint32_t block_count);
static FTLFunctions *TLFuncPtr=NULL;
static int nand_read_block_hook(struct blockdev *_dev, void *ptr, block_addr block, uint32_t count)
{
	Int32 err;
    
	if (count > 0) {
		err = TLFuncPtr->Read(block, count, ptr);
		//err = FTL_Read(block, count, ptr);
	
		if (err != FIL_SUCCESS) {
			printf("nand_read_block_hook: failure %d reading block %u, count %zd\n", err, block, count);
			return -1;
		}
	}
	return count;
}

int raw_nand_init(void)
{
	Int32 intRes;
	uint32_t block_size, block_count;
	uint32_t index_cache = 0;
	
	dprintf(DEBUG_INFO,"raw_nand_init()\n");

	// Initialise the FTL
	intRes = WMR_Init(&block_count, &block_size, &index_cache, gWhimoryInitParameters, nand_fsys_block_offset);

	if (intRes != WMR_SUCCESS) {
		dprintf(DEBUG_CRITICAL, "Raw NAND FTL failed initialisation\n");
		return -1;
	}
		
	if (0 != nand_filesys_init(block_size, block_count))
		return -1;

	return 0;
}

static int nand_filesys_init(uint32_t block_size, uint32_t block_count)
{
	struct blockdev *nand;

	TLFuncPtr = WMR_GetFTL();
	// build a block device around this NAND
	nand = malloc(sizeof(struct blockdev));
	
	construct_blockdev(nand, "nand0", (off_t)block_count * block_size, block_size);

	nand->read_block_hook = &nand_read_block_hook;

	register_blockdev(nand);

	/* scan ourselves for partitions and publish subdevices */
	partition_scan_and_publish_subdevices("nand0");

	return 0;
}
