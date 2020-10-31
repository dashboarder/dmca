/*
 * Copyright (C) 2009-2015 Apple Inc. All rights reserved.
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

void
arm_fp_init(void)
{
	u_int32_t	cr;

	cr = arm_read_cp_access_cr();
	cr |= CPACR_CP_BITS(10, CPACR_CP_FULL) | CPACR_CP_BITS(11, CPACR_CP_FULL);
	arm_write_cp_access_cr(cr);
}

#if WITH_VFP_ALWAYS_ON
void
arm_fp_enable(bool enable)
{
	arm_write_fpexc(FPEXC_EN);

	/* reset VFP/NEON state */
	arm_write_fpscr(FPSCR_DEFAULT);

	/* zero init VFP/NEON registers */
	arm_init_fp_regs();
}
#endif /* WITH_VFP_ALWAYS_ON */

bool
arch_task_fp_enable(bool enable)
{
	bool			current;
	u_int32_t		cr;
	struct task		*t;
	struct arch_task	*at;

	/* get the current FP enable state */
	t = task_get_current_task();
	at = &t->arch;
	current = (at->fpexc & FPEXC_EN);

	if (enable != current) {
		
		/* fetch current hardware state */
		cr = arm_read_fpexc();
		if (enable) {
			/* enable VFP/NEON */
			at->fpexc = FPEXC_EN;
			arm_write_fpexc(at->fpexc);

			/* reset VFP/NEON state */
			at->fpscr = 0;
			arm_write_fpscr(FPSCR_DEFAULT);

		} else {
			/* disable VFP/NEON */
			at->fpexc = cr & ~FPEXC_EN;
			arm_write_fpexc(at->fpexc);
		}

		return(!enable);
	}
	return(enable);
}

#endif /* WITH_VFP */
