/*
 * Copyright (C) 2011 Apple Inc. All rights reserved.
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
#include <platform.h>
#include <platform/soc/hwclocks.h>
#include <platform/timer.h>

extern void arm_write_cntp_ctl(uint64_t);
extern uint64_t arm_read_cntp_ctl();
extern void arm_write_cntp_tval(uint64_t);
extern uint64_t arm_read_cntpct();

static void timer_deadline(void *arg);
static void (* timer_deadline_func)(void);

#define DECR_MAX_COUNT	(INT32_MAX)

int timer_init(uint32_t timer)
{
	uint64_t	val;

	val = (0<<0) | (1<<1);
	arm_write_cntp_ctl(val);

	return 0;
}

void timer_stop_all(void)
{
	uint64_t	val;

	val = (0<<0) | (1<<1);
	arm_write_cntp_ctl(val);
}

void timer_deadline_enter(uint64_t deadline, void (* func)(void))
{
	uint64_t	ticks;
	uint64_t	decr;
	uint64_t	val;

	timer_deadline_func = func;
	if (func) {
		/* convert absolute deadline to relative time */
		ticks = timer_get_ticks();
		if (deadline < ticks) {
//			printf(" %llu too soon (it is now %llu)\n", deadline, ticks);
			deadline = 0;
		} else {
			deadline -= ticks;
		}

		/* clamp the deadline to our maximum */
		decr = (deadline >= DECR_MAX_COUNT) ? DECR_MAX_COUNT : deadline;
		// printf(" using decrementer count %u\n", decr);

		/*
		 * Reset the decrementer by programming the max count, clearing
		 * any pending interrupt status, then programming the desired count.
		 * Note the timer's value is a signed 32 bit number
		 */
		arm_write_cntp_tval(DECR_MAX_COUNT);
		val = (1<<0) | (0<<1);
		arm_write_cntp_ctl(val);
		arm_write_cntp_tval(decr);
	} else {
		/* no deadline, disable the decrementer */
		val = (0<<0) | (1<<1);
		arm_write_cntp_ctl(val);
	}
}

void arm_fiq()
{
	uint64_t	val;

	_irq_enter_critical_section();

	val = arm_read_cntp_ctl();
	if (val & (1 << 2)) {
		/* acknowledge the interrupt and disable the timer */
		val = (0<<0) | (1<<1);
		arm_write_cntp_ctl(val);

		/* if we have a callback, invoke it now */
		if (timer_deadline_func)
			timer_deadline_func();
	}

	_irq_exit_critical_section();
}

#ifdef OSC_FREQ

utime_t timer_ticks_to_usecs(uint64_t ticks)
{
#if (OSC_FREQ % 1000000) == 0
	return ticks / (OSC_FREQ / 1000000);
#else
#error "The code below has overflow issues. Make sure you really want to use it"
	return (ticks * 1000 * 1000) / OSC_FREQ;
#endif
}

uint64_t timer_usecs_to_ticks(utime_t usecs)
{
#if (OSC_FREQ % 1000000) == 0
	return usecs * (OSC_FREQ / 1000000);
#else
#error "The code below has overflow issues. Make sure you really want to use it"
	uint64_t timer_scale = ((uint64_t)(OSC_FREQ) << 20) / (1000 * 1000);
	return (ticks * timer_scale) >> 20;
#endif
}

uint32_t timer_get_tick_rate(void)
{
	return OSC_FREQ;
}

#endif

uint64_t timer_get_ticks(void)
{
	return (arm_read_cntpct());
}

void arm_no_wfe_spin(uint32_t usecs)
{
	uint64_t delta_time = (uint64_t)usecs * (OSC_FREQ / 1000000);
	uint64_t start_time;

	start_time = timer_get_ticks();
	while (true) {
		if ((timer_get_ticks() - start_time) > delta_time)
			break;
	}
}
