/*
 * Copyright (C) 2006 Apple Computer, Inc. All rights reserved.
 *
 * This document is the property of Apple Computer, Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Computer, Inc.
 */
#include <debug.h>
#include <sys/types.h>
#include <sys.h>
#include <sys/task.h>

static void arch_task_trampoline(void) __noreturn;

static void 
arch_task_trampoline(void)
{
	/* exit the critical section we implicitly started with */
	exit_critical_section();

	dprintf(DEBUG_SPEW, "arch_task_trampoline: task %p (%s) routine %p, arg %p\n", 
			current_task, current_task->name, current_task->routine, current_task->arg);

	/* call the start routine */
	int ret = current_task->routine(current_task->arg);

	dprintf(DEBUG_SPEW, "arch_task_trampoline: task %p (%s) exits with %d\n",
			current_task, current_task->name, ret);

	task_exit(ret);

	/* never return */
}

int 
arch_task_create(struct task *t)
{
	int i;

	for (i=0; i < 10; i++)
		t->arch.regs[i] = 0;

	/* set the default starting point to be arch_task_trampoline */
	t->arch.regs[ARM_ARCH_TASK_LR] = (uint32_t)&arch_task_trampoline;
	/* the ARM describes some encodings of SP ops (pre/post increment and/or decrement) as
	   potentially reaching above the SP by a word. Our toolchain might not emit such encodings,
	   but we'd rather be safe than sorry. Originally, we took off 4 bytes, but
	   to better align the stacks for some of the newer ABIs (e.g. armv7k), we'll consistently
	   take off 16 bytes from the top of the stack */
	t->arch.regs[ARM_ARCH_TASK_SP] = (uint32_t)t->stack_base + t->stack_len - 16;
	t->arch.regs[ARM_ARCH_TASK_FP] = 0;

#if WITH_VFP
	t->arch.fpexc = 0;	/* default state is FP disabled */
#endif
	
	return 0;
}
