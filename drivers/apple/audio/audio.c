/*
 * Copyright (C) 2009-2013 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */
#include <debug.h>
#include <platform.h>
#include <platform/int.h>
#include <platform/soc/hwclocks.h>
#include <platform/soc/hwregbase.h>
#include <platform/soc/pmgr.h>
#include <platform/timer.h>
#include <arch/arm/arm.h>
#include <drivers/audio/audio.h>

/*
 * AE2 Decrementer
 *
 * Note that for absolute time, we rely on externally defined timer_get_ticks and
 * it is assumed that the tick rate matches our decrementer's.
 */

static void timer_deadline(void *arg);
static void (* timer_deadline_func)(void);

#define DECR_MAX_COUNT	(0x7fffffffULL)

static uint32_t timer_count = 0;
static uint32_t timer_late_count = 0;

int timer_init(u_int32_t timer)
{
	if (timer) return -1;

	/* Disable timer */
	rAE2_ADECSR = 0;

	/* Install handler but mask for now */
	install_int_handler(AE2_INT_DEC, (int_handler)timer_deadline, NULL);
	set_int_type(AE2_INT_DEC, INT_TYPE_IRQ | INT_TYPE_LEVEL);
	mask_int(AE2_INT_DEC);

	return 0;
}

void timer_deadline_enter(u_int64_t deadline, void (* func)(void))
{
	uint64_t	ticks;
	uint32_t	decr;

	timer_deadline_func = func;
	if (func) {
		/* convert absolute deadline to relative time */
		ticks = timer_get_ticks();
		timer_count++;
		if (deadline < ticks) {
			timer_late_count++;
			deadline = 0;
		} else {
			deadline -= ticks;
		}

		/* clamp the deadline to our maximum, which is about 89 seconds */
		decr = (deadline > DECR_MAX_COUNT) ? DECR_MAX_COUNT : deadline;

		/*
		 * Stop the decrementer and clear the value to ensure
		 * no pending interrupts.  Load the value and go.
		 */
		rAE2_ADECSR = 0;
		rAE2_ADECTR = 0;
		unmask_int(AE2_INT_DEC);
		rAE2_ADECTR = decr;
		rAE2_ADECSR = 1;
	} else {
		/* no deadline, disable the decrementer and its interrupt */
		dprintf(DEBUG_INFO, "timer off\n");
		mask_int(AE2_INT_DEC);
		rAE2_ADECSR = 0;
	}
}

static void
timer_deadline(void *arg)
{
        /* acknowledge the interrupt and disable the decrementer */
	mask_int(AE2_INT_DEC);

        /* if we have a callback, invoke it now */
        if (timer_deadline_func)
                timer_deadline_func();
}

#if OSC_FREQ == 24000000

utime_t timer_ticks_to_usecs(uint64_t ticks)
{
	return ticks / 24;
}

uint64_t timer_usecs_to_ticks(utime_t usecs)
{
	return usecs * 24;
}

#endif

uint32_t timer_get_tick_rate(void)
{
	return OSC_FREQ;
}

uint64_t timer_get_ticks(void)
{
	extern uint64_t aic_get_ticks();
	return aic_get_ticks();
}

/*
 * AE2 interrupts
 */

static struct interrupt_entry handlers[AE2_MAX_INTS];

#if SUPPORT_SLEEP
struct ae2_int_state {
	u_int32_t	irqs;
	u_int32_t	fiqs;
} ae2_saved_state;
#endif

int interrupt_init(void)
{
	rAE2_AIRQCR = 0xffffffff;
	rAE2_AFIQCR = 0xffffffff;

#if SUPPORT_SLEEP
	rAE2_AIRQSR = ae2_saved_state.irqs;
	rAE2_AFIQSR = ae2_saved_state.fiqs;
#endif

	/* also enable it */
	exit_critical_section();

	return 0;
}

void unmask_int(uint32_t vector)
{
	if (handlers[vector].edge)
		rAE2_AFIQSR = 1 << vector;
	else
		rAE2_AIRQSR = 1 << vector;
#if SUPPORT_SLEEP
	ae2_saved_state.irqs = rAE2_AIRQER;
	ae2_saved_state.fiqs = rAE2_AFIQER;
#endif
}

void mask_int(uint32_t vector)
{
	if (handlers[vector].edge)
		rAE2_AFIQCR = 1 << vector;
	else
		rAE2_AIRQCR = 1 << vector;
#if SUPPORT_SLEEP
	ae2_saved_state.irqs = rAE2_AIRQER;
	ae2_saved_state.fiqs = rAE2_AFIQER;
#endif
}

void set_int_type(uint32_t vector, int type)
{
	/* Overload the "edge" member to indicate FIQ */
	handlers[vector].edge = (type & INT_TYPE_FIQ) != 0;
	if (type & INT_TYPE_FIQ)
		arm_enable_fiqs();
}

int install_int_handler(uint32_t vector, int_handler handler, void *arg)
{
	ASSERT(vector < AE2_MAX_INTS);

	enter_critical_section();

	handlers[vector].handler = handler;
	handlers[vector].arg = arg;

	exit_critical_section();

	return 0;
}

void platform_irq(void)
{
	u_int32_t pending, vector;

	if (0 == (pending = (rAE2_AIRQRR & rAE2_AIRQER))) {
		return;
	}
	vector = __builtin_ctz(pending);
	if (handlers[vector].handler)
		handlers[vector].handler(handlers[vector].arg);
}

void platform_fiq(void)
{
	u_int32_t pending, vector;

	if (0 == (pending = (rAE2_AFIQRR & rAE2_AFIQER))) {
		return;
	}
	vector = __builtin_ctz(pending);
	if (handlers[vector].handler)
		handlers[vector].handler(handlers[vector].arg);
}

void interrupt_generate_ipc(u_int32_t vector)
{
	rAE2_ASFISR = 1 << vector;
}

void interrupt_clear_ipc(u_int32_t vector)
{
	rAE2_ASFICR = 1 << vector;
}
