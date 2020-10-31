/*
 * Copyright (c) 2012-2014 Apple Inc. All rights reserved.
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

	// Disable RSS non-speculative streaming: <rdar://problem/14349246>
	// Do this as early as possible is it can cause incorrect SPR writes.
	mrs	x2, s3_0_c15_c1_0		// HID1
	orr	x2, x2, #(1<<2)			// rssDisNsStreaming
	msr	s3_0_c15_c1_0, x2		// HID1
	isb

	// Enable the L2 cache load/store prefetcher on B0 (and later): <rdar://problem/12096765>
	mrs	x2, s3_0_c15_c5_0		// HID5
	and	x2, x2, #~(3<<44)		// B0+: Enable L2C load (bit 44)/store (bit 45) prefetch
	msr	s3_0_c15_c5_0, x2		// HID5
	isb
	ret


	.align 	2
	.globl 	_platform_halt
_platform_halt:

	// For Cyclone, we must disable the MTLB prefetcher before doing WFI.
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

