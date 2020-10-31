/*
 * Copyright (C) 2007-2011 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */
#include <arch.h>
#include <lib/heap.h>
#include <lib/profile.h>
#include <lib/libc.h>
#include <platform.h>
#include <sys.h>
#include <sys/boot.h>

typedef void (* entry_func_t)(void *, int, int, int);
typedef void (* trampoline_func_t)(entry_func_t, void *);

/* quiesce hardware and jump into a new image */
void prepare_and_jump(enum boot_target boot, void *ptr, void *arg)
{
	entry_func_t entry = (entry_func_t)(uintptr_t)ptr;
	trampoline_func_t trampoline = (trampoline_func_t)platform_get_boot_trampoline();

	/* keep alive a little longer */
	platform_watchdog_tickle();

	/* do whatever prep needs to happen before jumping into something */
	platform_bootprep(boot);

	/* make sure timers and whatnot aren't running */
	platform_quiesce_hardware(boot);

	/* verify we haven't had any heap corruption */
	heap_verify();

	/* make sure interrupts are disabled */
	enter_critical_section();

	/* Copy profile buffer from SRAM into panic RAM */
	PROFILE_HANDOFF();

	/* quiesce the cpu, disabling the mmu and caches */
	arch_cpu_quiesce();

	if (trampoline != NULL)
		trampoline(entry, arg);
	else
		entry(arg, 0, 0, 0);

	/* should not get here */
	for(;;) ;
}
