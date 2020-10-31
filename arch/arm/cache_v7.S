/*
 * Copyright (C) 2007-2010 Apple Inc. All rights reserved.
 * Copyright (C) 2006 Apple Computer, Inc. All rights reserved.
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

	/*
	 * Cache functions for ARM v7
	 */

	/* perform loop unrolling to optimize for multi-line operations rdar://problem/6722268 */
ARM_FUNCTION _arm_clean_dcache_line_8

	mcr		p15, 0, r0, c7, c10, 1      // clean L1/L2 line by MVA
	add		r0, r0, #(CPU_CACHELINE_SIZE)

ARM_FUNCTION _arm_clean_dcache_line_7

	mcr		p15, 0, r0, c7, c10, 1      // clean L1/L2 line by MVA
	add		r0, r0, #(CPU_CACHELINE_SIZE)

ARM_FUNCTION _arm_clean_dcache_line_6

	mcr		p15, 0, r0, c7, c10, 1      // clean L1/L2 line by MVA
	add		r0, r0, #(CPU_CACHELINE_SIZE)

ARM_FUNCTION _arm_clean_dcache_line_5

	mcr		p15, 0, r0, c7, c10, 1      // clean L1/L2 line by MVA
	add		r0, r0, #(CPU_CACHELINE_SIZE)

ARM_FUNCTION _arm_clean_dcache_line_4

	mcr		p15, 0, r0, c7, c10, 1      // clean L1/L2 line by MVA
	add		r0, r0, #(CPU_CACHELINE_SIZE)

ARM_FUNCTION _arm_clean_dcache_line_3

	mcr		p15, 0, r0, c7, c10, 1      // clean L1/L2 line by MVA
	add		r0, r0, #(CPU_CACHELINE_SIZE)

ARM_FUNCTION _arm_clean_dcache_line_2

	mcr		p15, 0, r0, c7, c10, 1      // clean L1/L2 line by MVA
	add		r0, r0, #(CPU_CACHELINE_SIZE)

ARM_FUNCTION _arm_clean_dcache_line

	mcr		p15, 0, r0, c7, c10, 1		// clean L1/L2 line by MVA
	dmb
	bx		lr

	/* perform loop unrolling to optimize for multi-line operations rdar://problem/6722268 */
ARM_FUNCTION _arm_invalidate_dcache_line_8

	mcr		p15, 0, r0, c7, c6, 1		  // invalidate L1/L2 line by MVA
	add		r0, r0, #(CPU_CACHELINE_SIZE)

ARM_FUNCTION _arm_invalidate_dcache_line_7

	mcr		p15, 0, r0, c7, c6, 1		  // invalidate L1/L2 line by MVA
	add		r0, r0, #(CPU_CACHELINE_SIZE)

ARM_FUNCTION _arm_invalidate_dcache_line_6

	mcr		p15, 0, r0, c7, c6, 1		  // invalidate L1/L2 line by MVA
	add		r0, r0, #(CPU_CACHELINE_SIZE)

ARM_FUNCTION _arm_invalidate_dcache_line_5

	mcr		p15, 0, r0, c7, c6, 1		  // invalidate L1/L2 line by MVA
	add		r0, r0, #(CPU_CACHELINE_SIZE)

ARM_FUNCTION _arm_invalidate_dcache_line_4

	mcr		p15, 0, r0, c7, c6, 1		  // invalidate L1/L2 line by MVA
	add		r0, r0, #(CPU_CACHELINE_SIZE)

ARM_FUNCTION _arm_invalidate_dcache_line_3

	mcr		p15, 0, r0, c7, c6, 1		  // invalidate L1/L2 line by MVA
	add		r0, r0, #(CPU_CACHELINE_SIZE)

ARM_FUNCTION _arm_invalidate_dcache_line_2

	mcr		p15, 0, r0, c7, c6, 1		  // invalidate L1/L2 line by MVA
	add		r0, r0, #(CPU_CACHELINE_SIZE)

ARM_FUNCTION _arm_invalidate_dcache_line

	mcr		p15, 0, r0, c7, c6, 1		// invalidate L1/L2 line by MVA
	dmb
	bx		lr

	/* perform loop unrolling to optimize for multi-line operations rdar://problem/6722268 */
ARM_FUNCTION _arm_clean_invalidate_dcache_line_8

	mcr		p15, 0, r0, c7, c14, 1		 // clean/invalidate L1/L2 line by MVA
	add		r0, r0, #(CPU_CACHELINE_SIZE)

ARM_FUNCTION _arm_clean_invalidate_dcache_line_7

	mcr		p15, 0, r0, c7, c14, 1		 // clean/invalidate L1/L2 line by MVA
	add		r0, r0, #(CPU_CACHELINE_SIZE)

ARM_FUNCTION _arm_clean_invalidate_dcache_line_6

	mcr		p15, 0, r0, c7, c14, 1		 // clean/invalidate L1/L2 line by MVA
	add		r0, r0, #(CPU_CACHELINE_SIZE)

ARM_FUNCTION _arm_clean_invalidate_dcache_line_5

	mcr		p15, 0, r0, c7, c14, 1		 // clean/invalidate L1/L2 line by MVA
	add		r0, r0, #(CPU_CACHELINE_SIZE)

ARM_FUNCTION _arm_clean_invalidate_dcache_line_4

	mcr		p15, 0, r0, c7, c14, 1		 // clean/invalidate L1/L2 line by MVA
	add		r0, r0, #(CPU_CACHELINE_SIZE)

ARM_FUNCTION _arm_clean_invalidate_dcache_line_3

	mcr		p15, 0, r0, c7, c14, 1		 // clean/invalidate L1/L2 line by MVA
	add		r0, r0, #(CPU_CACHELINE_SIZE)

ARM_FUNCTION _arm_clean_invalidate_dcache_line_2

	mcr		p15, 0, r0, c7, c14, 1		 // clean/invalidate L1/L2 line by MVA
	add		r0, r0, #(CPU_CACHELINE_SIZE)

ARM_FUNCTION _arm_clean_invalidate_dcache_line

	mcr		p15, 0, r0, c7, c14, 1		// clean/invalidate L1/L2 line by MVA
	dmb
	bx		lr
	
	
ARM_FUNCTION _arm_clean_dcache

	mov		r2, #((1 << CPU_CACHEINDEX_SHIFT) << CPU_CACHELINE_SHIFT)
1:
	sub		r2, r2, #(1 << CPU_CACHELINE_SHIFT)
	mov		r1, #((1 << CPU_CACHESET_SHIFT) << (32-CPU_CACHESET_SHIFT))
2:
	subs		r1, r1, #(1 << (32-CPU_CACHESET_SHIFT))
	orr		r0, r1, r2
	mcr		p15, 0, r0, c7, c10, 2		// clean one line by way and set
	bne		2b
	cmp		r2, #0
	bne		1b

#if L2_CACHE_SIZE
	mov		r2, #((1 << L2_CACHE_SETS) << 6)
1:
	sub		r2, r2, #(1 << 6)
	mov		r1, #((1 << 3) << 29)
2:
	subs		r1, r1, #(1 << 29)
	orr		r0, r1, r2
	orr		r0, r0, #(1 << 1)
	mcr		p15, 0, r0, c7, c10, 2		// clean one line by way and set
	bne		2b
	cmp		r2, #0
	bne		1b
#endif

        mov		r0, #0
	dmb
	bx		lr

ARM_FUNCTION _arm_clean_invalidate_dcache

	mov		r2, #((1 << CPU_CACHEINDEX_SHIFT) << CPU_CACHELINE_SHIFT)
1:
	sub		r2, r2, #(1 << CPU_CACHELINE_SHIFT)
	mov		r1, #((1 << CPU_CACHESET_SHIFT) << (32-CPU_CACHESET_SHIFT))
2:
	subs		r1, r1, #(1 << (32-CPU_CACHESET_SHIFT))
	orr		r0, r1, r2
	mcr		p15, 0, r0, c7, c14, 2		// clean and invalidate one line by way and set
	bne		2b
	cmp		r2, #0
	bne		1b

#if L2_CACHE_SIZE
	mov		r2, #((1 << L2_CACHE_SETS) << 6)
1:
	sub		r2, r2, #(1 << 6)
	mov		r1, #((1 << 3) << 29)
2:
	subs		r1, r1, #(1 << 29)
	orr		r0, r1, r2
	orr		r0, r0, #(1 << 1)
	mcr		p15, 0, r0, c7, c14, 2		// clean and invalidate one line by way and set
	bne		2b
	cmp		r2, #0
	bne		1b
#endif

        mov		r0, #0
	dmb
	bx		lr

ARM_FUNCTION _arm_invalidate_dcache

	// This is a fancy way of writing "0". Thr first sub will make this (1<<CPU_CACHESET_SHIFT)-1
	mov		r1, #((1 << CPU_CACHESET_SHIFT) << (32-CPU_CACHESET_SHIFT))
	mov		r2, #((1 << CPU_CACHEINDEX_SHIFT) << CPU_CACHELINE_SHIFT)
1:
	sub		r1, r1, #(1 << (32-CPU_CACHESET_SHIFT))
	orr		r0, r1, r2
2:
#if (1 << CPU_CACHEINDEX_SHIFT) < 16
#error "Not enough cache sets for unroll in _arm_invalidate_dcache"
#endif
	sub		r0, r0, #(1 << CPU_CACHELINE_SHIFT)
	mcr		p15, 0, r0, c7, c6, 2		// invalidate one line by way and set
	sub		r0, r0, #(1 << CPU_CACHELINE_SHIFT)
	mcr		p15, 0, r0, c7, c6, 2		// invalidate one line by way and set
	sub		r0, r0, #(1 << CPU_CACHELINE_SHIFT)
	mcr		p15, 0, r0, c7, c6, 2		// invalidate one line by way and set
	sub		r0, r0, #(1 << CPU_CACHELINE_SHIFT)
	mcr		p15, 0, r0, c7, c6, 2		// invalidate one line by way and set
	sub		r0, r0, #(1 << CPU_CACHELINE_SHIFT)
	mcr		p15, 0, r0, c7, c6, 2		// invalidate one line by way and set
	sub		r0, r0, #(1 << CPU_CACHELINE_SHIFT)
	mcr		p15, 0, r0, c7, c6, 2		// invalidate one line by way and set
	sub		r0, r0, #(1 << CPU_CACHELINE_SHIFT)
	mcr		p15, 0, r0, c7, c6, 2		// invalidate one line by way and set
	sub		r0, r0, #(1 << CPU_CACHELINE_SHIFT)
	mcr		p15, 0, r0, c7, c6, 2		// invalidate one line by way and set
	sub		r0, r0, #(1 << CPU_CACHELINE_SHIFT)
	mcr		p15, 0, r0, c7, c6, 2		// invalidate one line by way and set
	sub		r0, r0, #(1 << CPU_CACHELINE_SHIFT)
	mcr		p15, 0, r0, c7, c6, 2		// invalidate one line by way and set
	sub		r0, r0, #(1 << CPU_CACHELINE_SHIFT)
	mcr		p15, 0, r0, c7, c6, 2		// invalidate one line by way and set
	sub		r0, r0, #(1 << CPU_CACHELINE_SHIFT)
	mcr		p15, 0, r0, c7, c6, 2		// invalidate one line by way and set
	sub		r0, r0, #(1 << CPU_CACHELINE_SHIFT)
	mcr		p15, 0, r0, c7, c6, 2		// invalidate one line by way and set
	sub		r0, r0, #(1 << CPU_CACHELINE_SHIFT)
	mcr		p15, 0, r0, c7, c6, 2		// invalidate one line by way and set
	sub		r0, r0, #(1 << CPU_CACHELINE_SHIFT)
	mcr		p15, 0, r0, c7, c6, 2		// invalidate one line by way and set
	sub		r0, r0, #(1 << CPU_CACHELINE_SHIFT)
	mcr		p15, 0, r0, c7, c6, 2		// invalidate one line by way and set
	cmp		r0, r1
	bne		2b
	cmp		r1, #0
	bne		1b

#if L2_CACHE_SIZE
	mov		r2, #((1 << L2_CACHE_SETS) << 6)
1:
	sub		r2, r2, #(1 << 6)
	mov		r1, #((1 << 3) << 29)
2:
	subs		r1, r1, #(1 << 29)
	orr		r0, r1, r2
	orr		r0, r0, #(1 << 1)
	mcr		p15, 0, r0, c7, c6, 2		// invalidate one line by way and set
	bne		2b
	cmp		r2, #0
	bne		1b
#endif

	dmb
	bx		lr

ARM_FUNCTION _arm_invalidate_icache

	mov		r0, #0
	mcr		p15, 0, r0, c7, c5, 0		/* invalidate icache + btb (if supported) */
	dsb
	isb
	bx		lr

ARM_FUNCTION _arm_clean_caches

	stmfd		sp!, {lr}

	bl		_arm_clean_dcache
	bl		_arm_invalidate_icache

	ldmfd		sp!, {lr}
	bx		lr

#if WITH_L1_PARITY
ARM_FUNCTION _arm_enable_l1parity

	mrc		p15, 0, r0, c1, c0, 1
#if CPU_ARM_CORTEX_A8
	orr		r0, r0, #(1 << 3)		// set the L1PE bit
#elif	CPU_ARM_CORTEX_A5
	orr		r0, r0, #(1 << 9)		// set the "parity on" bit
#endif
	mcr		p15, 0, r0, c1, c0, 1
	isb
	bx		lr
#endif

ARM_FUNCTION _arm_drain_write_buffer

	mov		r0, #0
	dsb
	isb
	bx		lr


