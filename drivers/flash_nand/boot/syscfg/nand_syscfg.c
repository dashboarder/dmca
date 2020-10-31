/*
 * Copyright (c) 2008-2012 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */

#if WITH_SYSCFG

// XXX include list is bloated; trim it down

#include <arch/arm/arm.h>
#include <debug.h>
#include <lib/heap.h>
#include <lib/blockdev.h>
#include <lib/partition.h>
#include <lib/syscfg.h>
#include <AssertMacros.h>

#include "WMRFeatures.h"
#include "WMRConfig.h" 
#include "ANDTypes.h"
#if WITH_RAW_NAND
#include "WMRExam.h"
#endif // WITH_RAW_NAND
#include "WMROAM.h"
#include "FIL.h"

#include "nand_part.h"
#include "nand_part_interface.h"
#include "nand_export.h"

// Ugly, ugly, ugly!
extern uint32_t nand_fsys_block_offset;

#if WITH_PPN_SYSCFG
extern int ppn_syscfg_init(void);
extern int ppn_read_syscfg_hook(struct blockdev *_dev, void *ptr, block_addr block, uint32_t count);
#endif //WITH_PPN_SYSCFG

// The syscfg library assumes that syscfg will be 0x4000 bytes into
// the device regardles of whether it's NAND or NOR.  nand1 will ONLY
// expose syscfg, so this doesn't make any sense, but we'll say we're
// big enough to read that far in anyway.
#define kSysCfgActualSize ( 0x4000 )
#define kFakeBdevBlockSize (kSysCfgBdevOffset)
#define kFakeBdevCapacity (kSysCfgBdevOffset + kSysCfgActualSize)


#if WITH_RAW_NAND

static int nand_read_syscfg_hook(struct blockdev *_dev, void *ptr, block_addr block, uint32_t count)
{
	bool ok = true;
	BOOL32 result;

	// Only allow reading the "second" block
	if ((block != 1) || (count != 1)) {
		dprintf(DEBUG_CRITICAL, "attempted to read nand syscfg at non-zero offset\n");
		ok = false;
    
	} else if (count > 0) {

		result = WMR_GetFPart()->ReadDeviceUniqueInfo(ptr, kFakeBdevBlockSize);
		if (!result) {

			dprintf(DEBUG_CRITICAL, "nand_read_block_hook: failure reading syscfg\n");
			ok = false;
		}
	}

	if (!ok)
		return -1;

	return count;
}

#endif // WITH_RAW_NAND

int nand_syscfg_init(NandPartInterface * npi)
{
	int err = 0;
	struct blockdev *nand_syscfg;
	bool is_ppn;
	
	dprintf(DEBUG_INFO, "nand_syscfg_init()\n");

    is_ppn = FIL_GetFuncTbl()->GetDeviceInfo(AND_DEVINFO_PPN_DEVICE);

	nand_syscfg = malloc(sizeof(struct blockdev));
#if WITH_PPN_SYSCFG
        if (is_ppn) {
            if (0 == ppn_syscfg_init()) {
                construct_blockdev(nand_syscfg, "nand_syscfg", kFakeBdevCapacity, kFakeBdevBlockSize);
                nand_syscfg->read_block_hook = &ppn_read_syscfg_hook;
                register_blockdev(nand_syscfg);
                syscfgInitWithBdev("nand_syscfg");
            } else {
                err = -1;
            }
        }
#else //WITH_PPN_SYSCFG
        check(!is_ppn);
#endif //WITH_PPN_SYSCFG

#if WITH_RAW_NAND
        if (!is_ppn) {
            if (WMR_SUCCESS == WMR_PreInit(gWhimoryInitParameters, nand_fsys_block_offset)) {
                construct_blockdev(nand_syscfg, "nand_syscfg", kFakeBdevCapacity, kFakeBdevBlockSize);
                nand_syscfg->read_block_hook = &nand_read_syscfg_hook;
                register_blockdev(nand_syscfg);
                syscfgInitWithBdev("nand_syscfg");
            } else {
                err = -1;
            }
        }
#else // WITH_RAW_NAND
        check(is_ppn);
#endif // WITH_RAW_NAND

	return err;
}

#endif /* WITH_SYSCFG */
