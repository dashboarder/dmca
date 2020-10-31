/*
 * Copyright (C) 2010-2011 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */
#include <arch/arm/assembler.h>

	.text

ARM_FUNCTION _platform_start
	
	// Configure the first half of the cache as RAM
	mov		r2, #0xFF000000			// L2CCONFIG0.AcpMask[7:0] = 0xFF
	orr		r2, r2, #0x000F0000		// L2CCONFIG0.RamWayMask[7:0] = 0x0F
	orr		r2, r2, #0x00000002		// L2CCONFIG0.RamEna = 1
	mcr		p15, 1, r2, c15, c2, 0		// Write to L2CCONFIG0
	isb						// Wait for the write to take effect
	bx		lr

	
ARM_FUNCTION _miu_read_l2cadrmap
	
	mrc		p15, 1, r0, c15, c2, 4
	bx		lr

	
ARM_FUNCTION _miu_write_l2cadrmap
	
	mcr		p15, 1, r0, c15, c2, 4
	isb
	bx 		lr
