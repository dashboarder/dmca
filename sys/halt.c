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
#include <arch.h>
#include <lib/libc.h>
#include <lib/random.h>
#include <platform.h>
#include <sys.h>
#include <sys/boot.h>
#include <debug.h>

#if WITH_CONSISTENT_DBG
#include <drivers/consistent_debug.h>
#endif

#if WITH_HW_TIMER
#include <platform/timer.h>
#endif

/*
 * Make the panic info easy to find.
 */
char *		gPanicStr;		/* pointer to current panic string or NULL if not doing panic */
const char *	gPanicFunc;		/* pointer to function calling panic */
#if defined(__LP64__)
static char	panicBuf[1024];		/* buffer for constructed panic message */
#else
static char	panicBuf[512];		/* buffer for constructed panic message */
#endif
static unsigned panicDepth = 0;
static u_int64_t panicStamp;

static const char	doublePanic[] = "double panic in ";

void _panic(const char *func, const char *str, ...)
{
	va_list			ap;
#if WITH_PANIC_HOOKS
	void **			cursor;
	struct panic_hook *	hook;
#endif

	/* There are cases where panic can spiral out of control if the task structure
	 * was demolished.  Detect recursion here before doing anything else, and STOP
	 * (with interrupts known to have been masked). */
	if (++panicDepth > 2)
		arch_spin();

	/* make sure that interrupts are masked and don't get enabled during panic */
	enter_critical_section();

	if (NULL != gPanicStr) {
		/*
		 * We are already trying to panic, so this is very bad.
		 * Do our very best to get the 'double panic' string into
		 * the panic buffer.
		 */
		memcpy(panicBuf, doublePanic, sizeof(doublePanic));
		gPanicFunc = func;
		strlcat(panicBuf, func, sizeof(panicBuf));

		/* clean the cache so that the panic string is in memory */
		platform_cache_operation(CACHE_PANIC | CACHE_CLEAN, 0, 0);

		/* and try to print it */
		puts("\n\n");
		puts(panicBuf);
		puts("\n\n");

		/* spin here - not safe to do anything else */
		arch_spin();
	}

#if WITH_HW_TIMER
	panicStamp = timer_get_ticks();
#endif

	/*
	 * First pass through the panic path
	 */

	/* gPanicFunc can be compared with NULL to test whether we are panicking */
	gPanicFunc = func;

	/* construct the panic string */
	gPanicStr = panicBuf;
	va_start(ap, str);
	vsnprintf(panicBuf, sizeof(panicBuf), str, ap);
	va_end(ap);

	/* clean the cache so that the panic string is in memory */
	platform_cache_operation(CACHE_PANIC | CACHE_CLEAN, 0, 0);

	/* emit it */
	puts("\npanic: ");
	puts(func);
	puts(": ");
	puts(panicBuf);
	puts("\n\n");

#if WITH_CONSISTENT_DBG && (PRODUCT_IBEC || PRODUCT_IBOOT)
	consistent_debug_update_ap_cpr(DBG_CPR_STATE_CRASHED, DBG_CPR_AP_PANICKED_IN_IBOOT);
	// Register location of panic string
	dbg_record_header_t dbghdr;
	dbghdr.physaddr = (uintptr_t)(mem_static_map_physical((uintptr_t)panicBuf));
	dbghdr.length = sizeof(panicBuf);
	dbghdr.record_id = kDbgIdPanicHeaderAP;
	consistent_debug_register_header(dbghdr);
#endif

#if WITH_PANIC_HOOKS
	/* run the set of panic handlers */
	LINKER_SET_FOREACH(cursor, panic_hooks) {
		hook = (struct panic_hook *)*cursor;
		hook->func(hook->arg);
	}

#endif
	/* XXX try to drain the console here */

	/* clean the cache again to get console output out in case the application is saving it */
	platform_cache_operation(CACHE_PANIC | CACHE_CLEAN, 0, 0);

	
#if !DEBUG_BUILD
	platform_reset(true);
#else
	halt();
#endif
}

void abort(void)
{
	panic("abort");
}

void halt()
{
	printf("\nsystem halted, spinning forever...\n");

	/* try to quiesce hardware to make JTAG easier */
	//prepare_and_jump(BOOT_HALT, (void *)(uintptr_t)&arch_spin, NULL);
	arch_spin();
}

#if defined(__SSP_ALL__)
#error "Can't compile stack.c with -fstack-protector-all. That'd emit a stack check fault on sys_init_stack!"
#endif

/*
 * Stack overflow testing.
 */
#ifdef __arm64__
#define	DEFAULT_STACK_COOKIE	0x4752400444303631ull		// 'GRD\0D061'
uint64_t __stack_chk_guard __used = DEFAULT_STACK_COOKIE;
#else
#define	DEFAULT_STACK_COOKIE	'GRD\0'
uint32_t __stack_chk_guard __used = DEFAULT_STACK_COOKIE;
#endif

#if WITH_RANDOM
// this needs to be outside of sys_init_stack_cookie to stop -fstack-protector-strong from tripping
static uint8_t	stack_chk_guard_zero_byte = 0;
#endif

// WARNING: This function cannot be called by anything that returns "normally"
//	    without risking a false stack overflow panic.
void sys_init_stack_cookie(void)
{
#if WITH_RANDOM
	uint8_t	*guard = (uint8_t*)&__stack_chk_guard;

	// This is tricky. You can't do anything in this function that would
	// cause the compiler to insert its stack validation cookie into
	// the stack frame because we're changing the value of the cookie
	// that the stack check code checks.

	if (random_get_bytes((u_int8_t *)&__stack_chk_guard, sizeof(__stack_chk_guard)) != 0) {
		// In case of failure, put back the static cookie just in case.
		__stack_chk_guard = DEFAULT_STACK_COOKIE;
	}

	// Grab a byte of entropy and use that value as an index to stuff a
	// zero byte into the cookie to block string functions from reading
	// past the cookie.
	random_get_bytes(&stack_chk_guard_zero_byte, sizeof(stack_chk_guard_zero_byte));

	guard[stack_chk_guard_zero_byte & (sizeof(__stack_chk_guard) - 1)] = 0;
#endif
}

void __noreturn __used
__stack_chk_fail(void)
{
	panic("stack corrupted");
}
