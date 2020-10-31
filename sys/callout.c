/*
 * Copyright (C) 2007-2009 Apple Inc. All rights reserved.
 * Copyright (C) 2006 Apple Computer, Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */
#include <debug.h>
#include <list.h>
#include <sys.h>
#include <sys/callout.h>
#include <platform/timer.h>

static struct list_node callout_queue;

static int callout_init(void);
static int _callout_dequeue(struct callout *c);
static void _callout_reset_deadline(void);
static void callout_deadline(void);

static bool ready;

/*
 * Initialise the callout system.
 */
static int callout_init(void)
{
	list_initialize(&callout_queue);

	ready = true;

	return 0;
}

/*
 * Enqueue (func) to be called after at least (delay) microseconds.
 */
void callout_enqueue(struct callout *c, utime_t delay, callout_func func, void *arg)
{
	struct callout	*entry;
	uint64_t	now;

	/* if we have not been initialised, do so now */
	if (unlikely(!ready))
		callout_init();

	enter_critical_section();

	/* remove it if it's already in a list */
	_callout_dequeue(c);

	/* convert utime_t delay to absolute clock deadline */
	now = timer_get_ticks();
	c->sched_ticks = now + timer_usecs_to_ticks(delay);
//	printf("callout %p: %lluus at %llu ticks (now %llu)\n", c, delay, c->sched_ticks, now);
	c->delay = delay;
	c->callback = func;
	c->arg = arg;

	list_for_every_entry(&callout_queue, entry, struct callout, list) {
		/* search through the callout queue, finding the right sorted slot */
		if (c->sched_ticks < entry->sched_ticks) {
			list_add_before(&entry->list, &c->list);
			goto done;
		}
	}

	/* either the queue was empty or our entry is the last */
	list_add_tail(&callout_queue, &c->list);

done:
	_callout_reset_deadline();
	
	exit_critical_section();
}

/*
 * Dequeue the callout (c)
 */
int callout_dequeue(struct callout *c)
{
	int err;

	enter_critical_section();
	err = _callout_dequeue(c);
	exit_critical_section();
	return err;
}

/*
 * Reset the callout (c) to be called after (newdelay) microseconds.
 */
int callout_reset(struct callout *c, utime_t newdelay)
{
	int err = 0;

	enter_critical_section();

	/* remove it if it's already in a list */
	_callout_dequeue(c);

	if (newdelay != 0)
		c->delay = newdelay;

	if (c->delay == 0) {
		err = -1;
	}  else {
		callout_enqueue(c, c->delay, c->callback, c->arg);
	}

	exit_critical_section();

	return err;
}

/*
 * Reset the timer deadline as the queue may have changed.
 */
static void _callout_reset_deadline(void)
{
	struct callout *c;

	c = list_peek_head_type(&callout_queue, struct callout, list);

	if (c == NULL) {
		timer_deadline_enter(0, NULL);
	} else {
		timer_deadline_enter(c->sched_ticks, callout_deadline);
	}
}

void callout_reset_deadline(void)
{
	enter_critical_section();
	_callout_reset_deadline();
	exit_critical_section();
}

/*
 * Pull callout (c) from the queue (while locked)
 */
static int _callout_dequeue(struct callout *c)
{
	
	if (list_in_list(&c->list)) {
		list_delete(&c->list);
		_callout_reset_deadline();
		return 1;
	}

	return 0;
}

/*
 * Callback from the timer when the deadline expires; call all of the
 * pending callouts and reschedule the deadline accordingly.
 *
 * Note that the timer is allowed to call us early; if we have set a
 * deadline beyond its ability to handle we will do no work and just re-schedule.
 */
static void callout_deadline(void)
{
	struct callout	*c;
	uint64_t	t;	

	while ((c = list_peek_head_type(&callout_queue, struct callout, list)) != NULL) {
		t = timer_get_ticks();

		/* if the event has expired */
		if (c->sched_ticks <= t) {
//			printf("callout %p: invoked, scheduled %llu now %llu\n", c, c->sched_ticks, t);
			
			/* remove from the list and call out */
			list_delete(&c->list);
			c->callback(c, c->arg);
		} else {

			/* callout in the future, schedule callback and we're done */
			timer_deadline_enter(c->sched_ticks, callout_deadline);
			break;
		}
	}
}
