/*
 * Copyright (C) 2006,2009 Apple Computer, Inc. All rights reserved.
 *
 * This document is the property of Apple Computer, Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Computer, Inc.
 */
#include <arch.h>
#include <stdio.h>
#include <debug.h>
#include <sys.h>
#include <sys/task.h>

#if DEBUG_CRITICAL_SECTIONS
void _enter_critical_section(const char *from)
{
	dprintf(DEBUG_SPEW, "enter_critical_section: %s %p %d\n", from, current_task, current_task->irq_disable_count);
#else
void enter_critical_section(void)
{
#endif
	RELEASE_ASSERT(current_task->irq_disable_count >= 0);
	RELEASE_ASSERT(current_task->irq_disable_count < 1000);
	current_task->irq_disable_count++;
	if (current_task->irq_disable_count == 1) {
		arch_disable_ints();
	}
}

#if DEBUG_CRITICAL_SECTIONS
void _exit_critical_section(const char *from)
{
	dprintf(DEBUG_SPEW, "exit_critical_section: %s %p %d\n", from, current_task, current_task->irq_disable_count);
#else
void exit_critical_section(void)
{
#endif
	RELEASE_ASSERT(current_task->irq_disable_count > 0);
	current_task->irq_disable_count--;
	if (current_task->irq_disable_count == 0) {
		arch_enable_ints();
	}
}

/* the following are only used in interrupt handler glue */
void _irq_enter_critical_section(void)
{
	RELEASE_ASSERT(current_task->irq_disable_count >= 0);
	RELEASE_ASSERT(current_task->irq_disable_count < 1000);
	current_task->irq_disable_count += 100;
}

void _irq_exit_critical_section(void)
{
	RELEASE_ASSERT(current_task->irq_disable_count >= 100);
	current_task->irq_disable_count -= 100;
}

