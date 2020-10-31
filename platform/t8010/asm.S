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
#include <platform/memmap.h>

	.text
	.balign	4
	.globl	_platform_start
_platform_start:
	////////////////////////////////////////////////
	//      Must preserve X0 and X1
	////////////////////////////////////////////////

#if !APPLICATION_SECUREROM
	// Enable the L2 cache load/store prefetcher.
	mrs	x2, s3_0_c15_c5_0		// HID5
	and	x2, x2, #~(3<<44)		// Enable L2C load (bit 44)/store (bit 45) prefetch
	msr	s3_0_c15_c5_0, x2		// HID5
	isb
#endif

#if WITH_L2_AS_RAM
#if APPLICATION_SECUREROM
	// Enable L2 as RAM
	mrs	x2, s3_3_c15_c7_0		// L2_CRAMCONFIG
	orr	x2, x2, #2			// L2_CRAMCONFIG.EnableSize (2 MiB SRAM)
	msr	s3_3_c15_c7_0, x2		// L2_CRAMCONFIG
1:
	// Poll for RAM ready
	mrs	x2, s3_3_c15_c7_0		// L2_CRAMCONFIG
	and	x2, x2, #(1<<63)		// L2_CRAMCONFIG.Ready
	cbz	x2, 1b				// Not ready
#endif
#if PRODUCT_IBOOT || PRODUCT_IBEC
	// Disable L2 as RAM
	mrs	x2, s3_3_c15_c7_0		// L2_CRAMCONFIG
	and	x2, x2, #~0x3F			// L2_CRAMCONFIG.EnableSize (disable SRAM)
	msr	s3_3_c15_c7_0, x2		// L2_CRAMCONFIG
1:
	// Poll for RAM not ready
	mrs	x2, s3_3_c15_c7_0		// L2_CRAMCONFIG
	and	x2, x2, #(1<<63)		// L2_CRAMCONFIG.Ready
	cbnz	x2, 1b				// Still ready
#endif
#endif
	ret

