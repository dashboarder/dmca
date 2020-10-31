/*
 * Copyright (C) 2006-2011 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */
#ifndef __ARCH_TASK_H
#define __ARCH_TASK_H

/*
 * Note that this structure is known implicitly by code in context.S, and
 * must be kept in sync.
 */
#if defined(__arm__)
#ifndef __ASSEMBLY__
struct arch_task {
	/*
	 * Core registers.
	 */
	uint32_t	regs[10]; 	// r4-r11, r13, r14

#if WITH_VFP
	/*
	 * VFP/Neon registers.
	 *
	 * Note that layout must be kept in sync with offset defines below.
	 */
	uint32_t	fpexc;
	uint32_t	fpscr;
	uint64_t	fpregs[32];
#endif
};
#endif /* __ASSEMBLY__ */

/*
 * Indices for useful core registers.
 */
#define ARM_ARCH_TASK_FP	3
#define ARM_ARCH_TASK_SP	8
#define ARM_ARCH_TASK_LR	9

/*
 * VFP/Neon field offsets - it would be nice to generate these automatically.
 */
#define ARM_ARCH_TASK_FP_OFFSET		40
#define ARM_ARCH_TASK_FPEXC		(ARM_ARCH_TASK_FP_OFFSET + 0)
#define ARM_ARCH_TASK_FPSCR		(ARM_ARCH_TASK_FP_OFFSET + 4)
#define ARM_ARCH_TASK_FPREGS		(ARM_ARCH_TASK_FP_OFFSET + 8)


#elif defined(__arm64__)

#ifndef __ASSEMBLY__

typedef __uint128_t uint128_t;

struct arch_task {
	/* Core registers */
	uint64_t		regs[29];	// x0-x28
	uint64_t		fp;
	uint64_t		lr;
	uint64_t		sp;	
#if WITH_VFP		
	/* Neon registers */
	union {
		uint128_t	q;
		uint64_t	d;
		uint32_t	s;
	} vregs[32];				// v0-v31
	uint32_t		fpsr;
	uint32_t		fpcr;
#endif	
};
#endif /* __ASSEMBLY__ */

#endif /* __arm64__ */

#endif /* ! __ARCH_TASK_H */
