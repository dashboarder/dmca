/*
 * Copyright (C) 2009-2010 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */
#include <arch.h>
#include <arch/arm/arm.h>
#include <debug.h>
#include <drivers/a5iop/a5iop.h>
#include <platform.h>
#include <platform/int.h>
#include <platform/timer.h>
#include <sys/callout.h>
#include <sys/task.h>

/*
 * Cache control
 */
void
a5iop_cache_operation(int operation, void *address, u_int32_t length)
{
	uint32_t	actual_base, lines, extra;

	if (unlikely(length == 0)) {
		enter_critical_section();
		if (operation & CACHE_CLEAN)
			arm_clean_dcache();
		if (operation & CACHE_INVALIDATE)
			arm_invalidate_dcache();
		exit_critical_section();

		/* on the panic path, disable the cache? XXX */
		if (operation & CACHE_PANIC) { }
	} else {
		actual_base = (uint32_t)address;
		ASSERT((actual_base & ~(CPU_CACHELINE_SIZE-1)) == actual_base);
		ASSERT((length % CPU_CACHELINE_SIZE) == 0);

		lines = length >> CPU_CACHELINE_SHIFT;
		ASSERT(lines > 0);

		extra = lines & 7;
		lines -= extra;

		/* perform loop unrolling to optimize multi-line operations rdar://problem/6722268 */
		if (operation & (CACHE_CLEAN | CACHE_INVALIDATE)) {
			switch (extra) {
				case 0:
					break;
				case 1:
					arm_clean_invalidate_dcache_line(actual_base);
					break;
				case 2:
					arm_clean_invalidate_dcache_line_2(actual_base);
					break;
				case 3:
					arm_clean_invalidate_dcache_line_3(actual_base);
					break;
				case 4:
					arm_clean_invalidate_dcache_line_4(actual_base);
					break;
				case 5:
					arm_clean_invalidate_dcache_line_5(actual_base);
					break;
				case 6:
					arm_clean_invalidate_dcache_line_6(actual_base);
					break;
				case 7:
					arm_clean_invalidate_dcache_line_7(actual_base);
					break;
			}
			actual_base += CPU_CACHELINE_SIZE * extra;

			while (lines) {
				arm_clean_invalidate_dcache_line_8(actual_base);
				lines -= 8;
				actual_base += CPU_CACHELINE_SIZE * 8;
			}
		} else if (operation & CACHE_CLEAN) {
			switch (extra) {
				case 0:
					break;
				case 1:
					arm_clean_dcache_line(actual_base);
					break;
				case 2:
					arm_clean_dcache_line_2(actual_base);
					break;
				case 3:
					arm_clean_dcache_line_3(actual_base);
					break;
				case 4:
					arm_clean_dcache_line_4(actual_base);
					break;
				case 5:
					arm_clean_dcache_line_5(actual_base);
					break;
				case 6:
					arm_clean_dcache_line_6(actual_base);
					break;
				case 7:
					arm_clean_dcache_line_7(actual_base);
					break;
			}
			actual_base += CPU_CACHELINE_SIZE * extra;

			while (lines) {
				arm_clean_dcache_line_8(actual_base);
				lines -= 8;
				actual_base += CPU_CACHELINE_SIZE * 8;
			}
		} else {// operation & CACHE_INVALIDATE
			switch (extra) {
				case 0:
					break;
				case 1:
					arm_invalidate_dcache_line(actual_base);
					break;
				case 2:
					arm_invalidate_dcache_line_2(actual_base);
					break;
				case 3:
					arm_invalidate_dcache_line_3(actual_base);
					break;
				case 4:
					arm_invalidate_dcache_line_4(actual_base);
					break;
				case 5:
					arm_invalidate_dcache_line_5(actual_base);
					break;
				case 6:
					arm_invalidate_dcache_line_6(actual_base);
					break;
				case 7:
					arm_invalidate_dcache_line_7(actual_base);
					break;
			}
			actual_base += CPU_CACHELINE_SIZE * extra;

			while (lines) {
				arm_invalidate_dcache_line_8(actual_base);
				lines -= 8;
				actual_base += CPU_CACHELINE_SIZE * 8;
			}
		}
	}
}

#if SUPPORT_SLEEP
static struct task	*sleeping_task;
static u_int32_t	idle_sleep;
static u_int32_t	powergate_cnt, powerwake_cnt, powerfall_cnt;
static u_int32_t	suspend_cnt, wake_cnt;
static u_int64_t	sleep_stamp, wake_stamp;
extern u_int32_t	arch_sleep_magic;

void
a5iop_sleep(u_int32_t going_idle)
{
	idle_sleep = going_idle;
	if (idle_sleep)
		powergate_cnt++;
	else
		suspend_cnt++;

	/* disable interrupts; this is matched in the wake path */
	enter_critical_section();

	/* we are the sleeping task */
	sleeping_task = task_get_current_task();

	sleep_stamp = timer_get_ticks();

	/* fool the upcoming test on the sleep path */
	arch_sleep_magic = SLEEP_MAGIC_NOT_ASLEEP;
	
	/* save our current context - on wake we will return from this call */
	arch_task_context_switch(&sleeping_task->arch, &sleeping_task->arch);

	/* if we are on the wake path, go back */
	if (SLEEP_MAGIC_WAKEY_WAKEY == arch_sleep_magic) {
		RELEASE_ASSERT(powergate_cnt == powerfall_cnt + powerwake_cnt);
		RELEASE_ASSERT(suspend_cnt == wake_cnt);
		return;
	}

	/* flip the switch and wait for us to go to sleep */
	arch_sleep_magic = SLEEP_MAGIC_WAKEY_WAKEY;

	/* Flush and invalidate the the entire dcache 
	 * Don't call a5iop_cache_operation directly because the 
	 * irq_disable count won't get flushed */
	arm_clean_dcache();

	/* Use WFI to fully quiesce */
	arch_halt();

	if (idle_sleep) {
		/* It's possible to fall through the WFI without
		 * power-gating, so carry on in that case. */
		powerfall_cnt++;
		exit_critical_section();
	} else {
		/* On the sleep/suspend path, it's not acceptible to
		 * return from this routine.  We don't expect to fall
		 * through the WFI but better safe than sorry. */
		while (1) arch_halt();
	}
}

void
platform_wakeup(void)
{
	/* Reestablish the correct CPU configuration */
	arch_cpu_init(true);

	wake_stamp = timer_get_ticks();

	if (idle_sleep) {
		/* Waking from idle power-gating; timer and interrupt
		 * state is intact; we just need to pop a level of IRQ
		 * disable and start running. */
		powerwake_cnt++;
		exit_critical_section();
	} else {
		wake_cnt++;
		platform_early_init();
	}
	/* There should be a better abstracted way to enable NMI <rdar://problem/8269765> */
	arm_enable_fiqs();

	/* kick the callout system to re-evaluate deadlines */
	callout_reset_deadline();

	/* and switch back to the context that went to sleep */
	arch_task_context_restore(&sleeping_task->arch);
	
	panic("arch_task_context_restore didn't");
}

#endif	/* SUPPORT_SLEEP */
