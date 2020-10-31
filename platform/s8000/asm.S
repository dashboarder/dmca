/*
 * Copyright (c) 2013-2015 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */

#include <platform.h>
#include <platform/memmap.h>
#include <platform/soc/hwregbase.h>
#include SUB_PLATFORM_SPDS_HEADER(minipmgr)

	.text
	.balign 4
	.globl 	_platform_start
_platform_start:
#if !APPLICATION_SECUREROM
	////////////////////////////////////////////////
	//      Must preserve X0 and X1
	////////////////////////////////////////////////

	// Enable the L2 cache load/store prefetcher.
	mrs	x2, s3_0_c15_c5_0		// HID5
	and	x2, x2, #~(3<<44)		// Enable L2C load (bit 44)/store (bit 45) prefetch
	msr	s3_0_c15_c5_0, x2		// HID5
	isb
#endif

	ret

	.balign 8

