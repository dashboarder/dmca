/*
 * Copyright (c) 2013-2014 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */

	.text
	.align 	2
	.globl 	_platform_start
_platform_start:
	////////////////////////////////////////////////
	//      Must preserve X0 and X1
	////////////////////////////////////////////////

	// Enable the L2 cache load/store prefetcher.
	mrs	x2, s3_0_c15_c5_0		// HID5
	and	x2, x2, #~(3<<44)		// Enable L2C load (bit 44)/store (bit 45) prefetch
	msr	s3_0_c15_c5_0, x2		// HID5
	isb
	ret


	.align 	2
	.globl 	_platform_halt
_platform_halt:

	// For Typhoon, we must disable the MTLB prefetcher before doing WFI.
	// <rdar://problem/15837639> iBoot: implement Cyclone/Typhoon WFI workaround

	mrs	x1, s3_0_c15_c2_0		// Read HID2
	orr	x0, x1, #(1 << 13)		// Set HID2.DisableMTLBPrefetch
	msr	s3_0_c15_c2_0, x0		// Write HID2
	dsb	sy
	isb	sy

	wfi

	// Re-enable MTLB prefetcher after WFI falls through.
	msr	s3_0_c15_c2_0, x1		// Restore HID2
	dsb	sy
	isb	sy

	ret

