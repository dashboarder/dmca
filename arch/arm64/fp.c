/*
 * Copyright (C) 2012-2013 Apple Computer, Inc. All rights reserved.
 *
 * This document is the property of Apple Computer, Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Computer, Inc.
 */
#include <arch.h>
#include <arch/arm/arm.h>
#include <debug.h>
#include <sys/task.h>

/*
 * VFP/Neon support
 */

#if WITH_VFP

extern uint32_t arm_read_cpacr();
extern void arm_write_cpacr(uint32_t);

void arm_fp_init(void)
{
	uint32_t cpacr;

	/* v8: CPACR FPEN bits [21:20] needs to be programmed correctly to avoid a trap */
	cpacr = arm_read_cpacr();
	cpacr |= (3 << 20);
	arm_write_cpacr(cpacr);
}

bool arch_task_fp_enable(bool enable)
{
	return true;
}

#endif /* WITH_VFP */
