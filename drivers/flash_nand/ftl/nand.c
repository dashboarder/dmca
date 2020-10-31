/*
 * Copyright (c) 2007-2010 Apple Inc. All rights reserved.
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
#include <drivers/flash_nand.h>

#include "FIL.h"
#include "ANDTypes.h"
#include "WMRConfig.h"
#include "WMROAM.h"

extern int ppn_init(void);
extern int raw_nand_init(void);

uint32_t nand_fsys_block_offset = WMR_BOOT_AREA_DEFAULT_SIZE;

int flash_nand_init(void)
{
	int ret = -1;
	LowFuncTbl *fil;
	
	dprintf(DEBUG_INFO, "flash_nand_init()\n");

	/* FIL_Init was already executed when flash_nand_id() was called in platform_late_init() */
	fil = FIL_GetFuncTbl();
	if ((NULL == fil) || (NULL == fil->GetDeviceInfo))
	{
		dprintf(DEBUG_INFO, "GetDeviceInfo not populated\n");
		return -1;
	}

#if WITH_PPN
	if (fil->GetDeviceInfo(AND_DEVINFO_PPN_DEVICE) > 0)
	{
		ret = ppn_init();
	}
#if WITH_RAW_NAND
	else
#endif // HAS_RAW_NAND
#endif
#if WITH_RAW_NAND
	{
		ret = raw_nand_init();
	}
#endif

#if (!WITH_RAW_NAND && !WITH_PPN)
#error nand.c included without a raw or ppn stack
#endif

	return ret;
}

