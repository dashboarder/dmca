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

#if WITH_SYSCFG

#include <debug.h>
#include <lib/blockdev.h>
#include <lib/syscfg.h>

#include "FPart.h" 
#include "VFLBuffer.h"
#include "WMRConfig.h"
#include "WhimoryBoot.h"

extern uint32_t nand_fsys_block_offset;

// The syscfg library assumes that syscfg will be 0x4000 bytes into
// the device regardles of whether it's NAND or NOR.  nand1 will ONLY
// expose syscfg, so this doesn't make any sense, but we'll say we're
// big enough to read that far in anyway.
#define kSysCfgActualSize ( 0x4000 )
#define kFakeBdevBlockSize (kSysCfgBdevOffset)
#define kFakeBdevCapacity (kSysCfgBdevOffset + kSysCfgActualSize)



static FPartFunctions *_fpart = NULL;

int ppn_syscfg_init(void)
{
    WhimoryBootCode boot_code;

    boot_code = WMR_Start(0, 0, 0);
    if (kWhimoryOk == boot_code) {
        _fpart = WMR_PPN_GetFPart();
        if (NULL != _fpart->ReadSpecialBlock) {
            return 0;
        } else {
            dprintf(DEBUG_CRITICAL, "ppn syscfg is not supported!\n");
            return -1;
        }
    }
    return -1;
}

int ppn_read_syscfg_hook(struct blockdev *_dev, void *ptr, block_addr block, size_t count)
{
	bool ok = true;

	// Only allow reading the "second" block
	if ((block != 1) || (count != 1)) {
		dprintf(DEBUG_CRITICAL, "attempted to read ppn syscfg at non-zero offset\n");
		ok = false;
    
	} else if (count > 0) {
		if (!_fpart->ReadSpecialBlock(ptr, kFakeBdevBlockSize, AND_SB_TYPE_UNIQUE_INFO)) {
			dprintf(DEBUG_CRITICAL, "ppn_read_syscfg_hook: failure reading syscfg\n");
			ok = false;
		}
	}

	if (!ok)
		return -1;

	return count;
}

#endif /* WITH_SYSCFG */
