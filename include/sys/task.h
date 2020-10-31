/*
 * Copyright (C) 2007-2011 Apple Inc. All rights reserved.
 * Copyright (C) 2006 Apple Computer, Inc. All rights reserved.
 *
 * This document is the property of Apple Computer, Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Computer, Inc.
 */
#ifndef __SYS_TASK_H
#define __SYS_TASK_H

#include <sys/types.h>

#include <arch/arch_task.h>
#include <compiler.h>
#include <list.h>
#include <sys/callout.h>

__BEGIN_DECLS

typedef int (*task_routine)(void *);

enum task_state {
	TASK_INITIAL,
	TASK_READY,
	TASK_RUNNING,
	TASK_BLOCKED,
	TASK_SLEEPING,
	TASK_FINISHED
};

struct task_wait_queue {
	struct list_node task_list;
};

#define TASK_MAGIC 		'task'
#define TASK_MAGIC2		'tsk2'
#define TASK_NAME_MAX		15
#if defined(__LP64__)
#define TASK_STACK_MIN		(16384)
#else
#define TASK_STACK_MIN		(512)
#endif

struct task {
	int magic;

	struct list_node task_list_node;
	struct list_node queue_node;

	/* track the state of the task (READY, RUNNING, etc) */
	enum task_state state;

	/* interrupt disable count */
	int irq_disable_count;

	/* saved registers, stack pointer */
	struct arch_task arch; 

	/* a callout for sleeping */
	struct callout sleep_callout;

	/* to track other tasks waiting on our completion */
	struct task_wait_queue return_waiters;
	int return_code;

	/* starting routine and argument */
	task_routine routine;
	void *arg;

	/* stack */
	void *stack_base;
	size_t stack_len;

	/* debug name of this task */
	char name[TASK_NAME_MAX + 1];

	/* unique id of this task */
	int task_id;

	int magic2;
};

/* statistics */
struct idle_statistics {
	u_int64_t   uptime_ticks;
	u_int64_t   deep_idle_ticks;
	u_int64_t   deep_idles;
	u_int64_t   idle_ticks;
	u_int64_t   idles;
	u_int32_t   ticksHz;
	u_int32_t   threshold_us;
};

/* the currently running task */
extern struct task *current_task;

/* initialize the tasking system */
void task_init(void);

/* create a new task */
struct task *task_create(const char *name, task_routine routine, void *arg, size_t stack_len);
void task_destroy(struct task *);

/* give up the cpu to another task */
void task_yield(void);
void task_start(struct task *);
void task_exit(int return_code) __noreturn;
int task_wait_on(struct task *);
void task_block(struct task_wait_queue *);
void task_sleep(utime_t delay);
struct task *task_get_current_task(void);

/* wait_queue routines */
void wait_queue_init(struct task_wait_queue *);
void wait_queue_destroy(struct task_wait_queue *);
int wait_queue_wake_all(struct task_wait_queue *);
int wait_queue_wake_one(struct task_wait_queue *);

/* profile the task manager */
void task_get_statistics(struct idle_statistics *);

/* define the application's deep idle policy */
void task_set_idle_threshold(u_int32_t us);

/* events */
struct task_event {
	bool signalled;
	uint32_t flags;

	struct task_wait_queue wait;
};

#define EVENT_STATIC_INIT(_struct, _initial, _flags)			\
{									\
	.signalled = _initial,						\
	.flags = _flags,						\
	.wait = { .task_list = LIST_INITIAL_VALUE(_struct.wait.task_list) } \
}

/*
 * event_init
 *
 * Initialise a task_event structure for use.
 */
void event_init(struct task_event *, uint32_t flags, bool initial_state);

#define EVENT_FLAG_AUTO_UNSIGNAL 1

/*
 * event_signal
 *
 * Signal a task_event, potentially waking one or more tasks waiting on the
 * event.  If the event is marked EVENT_FLAG_AUTO_UNSIGNAL, only one waiting
 * task will be woken each time the event is
 * signalled. 
 */
void event_signal(struct task_event *);

/*
 * event_unsignal
 *
 * Clear the is-signalled state of a task_event.  This does not affect the
 * scheduling state of any tasks that may be waiting on the event.
 */
void event_unsignal(struct task_event *);

/*
 * event_wait
 * event_wait_timeout
 *
 * Wait for a task_event to be signalled.  If the event is marked
 * EVENT_FLAG_AUTO_UNSIGNAL, only one waiting task will be woken each time the
 * event is signalled.
 *
 * If the timeout expires and the task_event is not signalled, event_wait_timeout
 * returns false.
 *
 * If the timeout is zero, event_wait_timeout will return immediately with the current
 * state of the task_event.
 */
void event_wait(struct task_event *);
bool event_wait_timeout(struct task_event *, utime_t delay);

__END_DECLS

#endif
