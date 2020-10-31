/*
 * Copyright (C) 2007-2009 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */
#include <arch.h>
#include <debug.h>
#include <drivers/arm7m/arm7m.h>
#include <platform.h>
#include <platform/clocks.h>
#include <platform/int.h>
#include <platform/soc/arm7m.h>
#include <platform/soc/hwisr.h>
#include <sys.h>
#include <sys/boot.h>
#include <sys/callout.h>
#include <sys/task.h>

/*
 * Early init is done every time the ARM7 starts.
 */
int
arm7m_init(void)
{
	
	/*
	 * Cache starts off.  Invalidate and then turn it on.
	 */
	rARM7M_CACHE_INV = 1;
	rARM7M_CACHE_ON = ARM7M_CACHE_ON_ENABLE;

	return(0);
}

/*
 * Halt waiting for an interrupt.
 */
void
arm7m_halt(void)
{
	/* XXX need to avoid doing this until cache operations are complete */
	rARM7M_SLEEP = 1;
}

/*
 * Cache control
 */
void
arm7m_cache_operation(int operation, void *address, u_int32_t length)
{
	uint32_t	actual_base, lines;

	if (length == 0) {
		/*
		 * To perform whole-cache operations, we have to turn the cache off.
		 *
		 * Since this introduces both a coherency risk and also since our use
		 * of the hardware needs to be exclusive, we disable interrupts.
		 */
		enter_critical_section();
		rARM7M_CACHE_ON = 0;

		if (operation & CACHE_CLEAN)
			rARM7M_CACHE_SYNC = 0;
		if (operation & CACHE_INVALIDATE)		
			rARM7M_CACHE_INV = 0;

		/* on the panic path, leave the cache off */
		if (!(operation & CACHE_PANIC))
			rARM7M_CACHE_ON = ARM7M_CACHE_ON_ENABLE;
		
		exit_critical_section();
	} else {
		actual_base = (uint32_t)address;
		ASSERT((actual_base & ARM7M_CACHE_LINE_MASK) == actual_base);
		ASSERT((length % ARM7M_CACHE_LINE_SIZE) == 0);

		lines = length >> ARM7M_CACHE_LINE_SHIFT;
		ASSERT(lines > 0);

		/*
		 * Disable interrupts to prevent re-entrance from an interrupt
		 * handler.
		 */
		enter_critical_section();
		for (;;) {
			rARM7M_CLINE_ADDR = actual_base;
			if (operation & CACHE_CLEAN)
				rARM7M_CLINE_SYNC = 0;
			if (operation & CACHE_INVALIDATE)
				rARM7M_CLINE_INV = 0;
			if (--lines < 1)
				break;
			actual_base += ARM7M_CACHE_LINE_SIZE;
		}
		exit_critical_section();
	}
}

#if SUPPORT_SLEEP
static struct task	*sleeping_task;
extern u_int32_t	arch_sleep_magic;

void
platform_sleep(void)
{
	/*
	 * Disable interrupts; this is matched by the
	 * exit_critical_section call in interrupt_init on the wake
	 * path.
	 */
	enter_critical_section();

	/* we are the sleeping task */
	sleeping_task = task_get_current_task();

	/* fool the upcoming test on the sleep path */
	arch_sleep_magic = SLEEP_MAGIC_NOT_ASLEEP;
	
	/* save our current context - on wake we will return from this call */
	arch_task_context_switch(&sleeping_task->arch, &sleeping_task->arch);

	/* if we are on the wake path, go back */
	if (SLEEP_MAGIC_WAKEY_WAKEY == arch_sleep_magic)
		return;

	/* flip the switch and wait for us to go to sleep */
	arch_sleep_magic = SLEEP_MAGIC_WAKEY_WAKEY;

	/* disable and flush the cache, since we're about to be turned off */
	rARM7M_CACHE_ON = 0;
	rARM7M_CACHE_SYNC = 0;

	/* XXX really need to know how long the cache operation will take to complete */
	for (;;) {
	}
}

void
platform_wakeup(void)
{
	/*
	 * Cache starts off.  Invalidate and then turn it on.
	 */
	rARM7M_CACHE_INV = 1;
	rARM7M_CACHE_ON = ARM7M_CACHE_ON_ENABLE;

	/*
	 * The VIC init is sleep/wake friendly - note this unmasks
	 * interrupts that were unmasked at sleep time, so handlers
	 * must be ready (or interrupt sources should be masked before
	 * sleep).
	 *
	 * This call also exits the critical section that was entered
	 * in platform_sleep (current_task has not changed).
	 */
	interrupt_init();

	/* kick the callout system to re-evaluate deadlines */
	callout_reset_deadline();

	/* and switch back to the context that went to sleep */
	arch_task_context_restore(&sleeping_task->arch);
	
	panic("arch_task_context_restore didn't");
}
#endif	/* SUPPORT_SLEEP */

