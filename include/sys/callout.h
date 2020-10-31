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
#ifndef __SYS_CALLOUT_H
#define __SYS_CALLOUT_H

#include <list.h>

__BEGIN_DECLS

struct callout;
typedef void (*callout_func)(struct callout *, void *);

struct callout {
	struct list_node list;

	/* scheduled time */
	uint64_t sched_ticks;
	utime_t delay;

	/* callback and args */
	callout_func callback;
	void *arg;
};

void callout_enqueue(struct callout *, utime_t delay, callout_func, void *arg);
int callout_dequeue(struct callout *);
int callout_reset(struct callout *, utime_t newdelay); // newdelay = 0 to reuse old delay
void callout_reset_deadline(void);

__END_DECLS

#endif

