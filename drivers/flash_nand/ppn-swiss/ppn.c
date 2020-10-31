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
#include "WhimoryBoot.h"
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

int ppn_init(void)
{
	uint32_t block_size, block_count;
	uint32_t index_cache = 0;
	WhimoryBootCode boot_code;
	
	dprintf(DEBUG_INFO, "swiss_ppn_init()\n");

	// Find a signature
	boot_code = WMR_Start(0, 0, gWhimoryInitParameters);
	if (kWhimoryOk == boot_code) {
	   // Open the FTL
	   boot_code = WMR_Open(&block_count, &block_size, &index_cache, gWhimoryInitParameters);
	   if (kWhimoryOk == boot_code) {
	       // Publish a block device
	       if (0 != nand_filesys_init(block_size, block_count)) {
	           return 0;
	       }
	   }
    } else if (kWhimoryWarningUnformatted == boot_code) {
        printf("***NAND driver is not formatted. Run restore process.***\n");
    }
    return -1;
}

static int nand_filesys_init(uint32_t block_size, uint32_t block_count)
{
	struct blockdev *nand;

	TLFuncPtr = WMR_PPN_GetFTL();
	// build a block device around this NAND
	nand = malloc(sizeof(struct blockdev));
	
	construct_blockdev(nand, "nand0", (off_t)block_count * block_size, block_size);

	nand->read_block_hook = &nand_read_block_hook;

	register_blockdev(nand);

	/* scan ourselves for partitions and publish subdevices */
	partition_scan_and_publish_subdevices("nand0");

	return 0;
}
