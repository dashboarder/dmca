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
#include <list.h>
#include <debug.h>
#include <string.h>
#include <arch.h>
#include <platform.h>
#include <lib/heap.h>
#include <lib/mib.h>
#include <lib/random.h>
#include <sys.h>
#include <sys/callout.h>
#include <sys/task.h>
#include <sys/menu.h>
#include <platform/timer.h>

#if !WITH_HW_TIMER
# error Cannot build task subsystem without timer support
#endif

#define DEFAULT_TASK_STACK_MAGIC 'stak'

/* list of all tasks */
static struct list_node task_list = LIST_INITIAL_VALUE(task_list);
static int task_count;

static int max_task_id;

/* global run queue */
static struct list_node run_q = LIST_INITIAL_VALUE(run_q);
static volatile int run_q_len;

/* initial bootstrapping task, partially constructed */
static struct task bootstrap_task = {
	.magic = TASK_MAGIC,
	.state = TASK_RUNNING,
	.irq_disable_count = 1,
	.routine = NULL,
	.arg = NULL,
	.stack_base = NULL,
	.stack_len = 0,

	.name = "bootstrap",
	.magic2 = TASK_MAGIC2,
	.task_id = 0,
};

/* idle task */
static int idle_task_routine(void *);

#ifndef IDLE_TASK_SIZE
 #define IDLE_TASK_SIZE TASK_STACK_MIN
#endif

static uint8_t idle_task_stack[IDLE_TASK_SIZE]  __attribute__ ((aligned(16)));
static struct task idle_task;
static uint64_t idle_ticks = 0ULL;
static uint64_t deep_idle_ticks = 0ULL;
static uint64_t boot_time_ticks;
static uint64_t shallow_idle_cnt = 0ULL;
static uint64_t deep_idle_cnt = 0ULL;
static uint64_t last_yield;
static uint64_t last_switch;
static uint32_t task_stack_magic = DEFAULT_TASK_STACK_MAGIC;

MIB_VARIABLE(kMIBSystemBootTime, kOIDTypeUInt64, boot_time_ticks);
MIB_VARIABLE(kMIBSystemIdleTime, kOIDTypeUInt64, idle_ticks);

static void
uptime_handler(u_int32_t oid __unused, void *arg __unused, void *data)
{
	u_int64_t *resptr = (u_int64_t *)data;
	u_int64_t now;

	now = timer_get_ticks();
	*resptr = now - boot_time_ticks;
}

MIB_FUNCTION(kMIBSystemUptime, kOIDTypeUInt64, uptime_handler, NULL);

/* currently running task */
struct task *current_task = &bootstrap_task;

#if defined(__arm__)
static uint32_t nullptr_value;
#endif
static bool deep_idle;

#ifndef DEEP_IDLE_THRESHOLD_US
#define DEEP_IDLE_THRESHOLD_US	1000
#endif

static volatile uint32_t deep_idle_threshold_us = 0; // <rdar://problem/8913103>

static struct task *task_create_etc(struct task *t, const char *name, task_routine routine, void *arg, void *stack, size_t stack_len);
static void insert_run_q_head(struct task *t);
static void insert_run_q_tail(struct task *t);

struct callout deep_idle_callout;
static void deep_idle_timeout(struct callout *co, void *arg)
{
	deep_idle = true;
}

static int idle_task_routine(void *arg)
{
	u_int64_t	idle_enter, idle_leave;
        u_int64_t       *ticks_accumulator;
	
	for(;;) {
		task_yield();
		idle_enter = timer_get_ticks();

		enter_critical_section();
		ticks_accumulator = & idle_ticks;
		if (likely(run_q_len == 0)) {
			if (deep_idle) {
				ticks_accumulator = & deep_idle_ticks;
				deep_idle_cnt++;
				platform_deep_idle();
				deep_idle = false;
			} else {
				/* Setup the "deep idle" callout and halt ("shallow idle") */
				if (deep_idle_threshold_us)
					callout_enqueue(&deep_idle_callout, deep_idle_threshold_us, deep_idle_timeout, NULL);
				ticks_accumulator = & idle_ticks;
				shallow_idle_cnt++;
				platform_halt();
			}
		}
		exit_critical_section();

		/* We should never be here with interrupts disabled */
		RELEASE_ASSERT(0 == current_task->irq_disable_count);

		idle_leave = timer_get_ticks();
		*ticks_accumulator += idle_leave - idle_enter;
	}

	return 0;
}

static void task_init_stack_magic_cookie(void)
{
#if WITH_RANDOM
	uint8_t	*guard = (uint8_t*)&task_stack_magic;

	if (random_get_bytes((u_int8_t *)&task_stack_magic, sizeof(task_stack_magic)) != 0) {
		// In case of failure, put back the static cookie just in case.
		task_stack_magic = DEFAULT_TASK_STACK_MAGIC;
	}

	guard[task_stack_magic & (sizeof(task_stack_magic) - 1)] = 0;
#endif
}


void task_init(void)
{
	
	dprintf(DEBUG_INFO, "initializing task system\n");

	boot_time_ticks = timer_get_ticks();

#if !WITH_NO_RANDOM_STACK_COOKIE
	/* initialize the random cookie used for overflow detection */
	task_init_stack_magic_cookie();
#endif

	/* go back and fill in some more details of the bootstrap task */
	wait_queue_init(&bootstrap_task.return_waiters);

	/* construct an idle task */
	task_create_etc(&idle_task, "idle task", &idle_task_routine, NULL, idle_task_stack, sizeof(idle_task_stack));
	idle_task.state = TASK_READY; /* it never gets started properly */

	task_count = 2;	/* bootstrap task, idle task */

#if defined(__arm__) && !defined(__clang_analyzer__)
	nullptr_value = *(volatile uint32_t *)NULL;
#endif 
}

struct task *task_get_current_task(void)
{
	return(current_task);
}

static struct task *task_create_etc(struct task *t, const char *name, task_routine routine, void *arg, void *stack, size_t stack_len)
{
	uint32_t	i;

	/* clear the storage for the task */
	memset(t, 0, sizeof(struct task));

	/* set up the new task */
	t->magic = TASK_MAGIC;
	t->magic2 = TASK_MAGIC2;
	t->stack_base = stack;
	t->stack_len = stack_len;
	strlcpy(t->name, name, sizeof(t->name));
	t->routine = routine;
	t->arg = arg;

	enter_critical_section();
	t->task_id = ++max_task_id;
	exit_critical_section();

	list_clear_node(&t->queue_node);
	wait_queue_init(&t->return_waiters);

	t->state = TASK_INITIAL;
	t->irq_disable_count = 1; /* start off as if we were inside a critical section */

	/* fill the stack with 'magic' */
	for (i = 0; i < (stack_len / sizeof(uint32_t)); i++)
		*((uint32_t *)t->stack_base + i) = task_stack_magic;
	
	arch_task_create(t);

	/* add ourselves to the global task list */
	list_add_head(&task_list, &t->task_list_node);

	task_count++;

	return t;
}

struct task *task_create(const char *name, task_routine routine, void *arg, size_t stack_len)
{
	struct task *t;
	void *stack;

	/* stacks must be at least TASK_STACK_MIN bytes */
	if (stack_len < TASK_STACK_MIN)
		stack_len = TASK_STACK_MIN;

	/* it will panic if not to be able to allocate stack */
	t = malloc(sizeof(struct task));
	/* assumption: malloc always returns cache-line size aligned buffer */
	stack = malloc(stack_len);

	return task_create_etc(t, name, routine, arg, stack, stack_len);
}

void task_destroy(struct task *t)
{
	list_delete(&t->task_list_node);
	free(t->stack_base);
	free(t);
}

static void insert_run_q_head(struct task *t)
{
	ASSERT(t != &idle_task);	
	ASSERT(run_q_len < task_count);
	
	list_add_head(&run_q, &t->queue_node);
	run_q_len++;
}

static void insert_run_q_tail(struct task *t)
{
	ASSERT(t != &idle_task);
	ASSERT(run_q_len < task_count);
	
	list_add_tail(&run_q, &t->queue_node);
	run_q_len++;
}

static struct task *pop_run_q_head(void)
{
	struct task *t = list_remove_head_type(&run_q, struct task, queue_node);
	if (t) {
		ASSERT(run_q_len > 0);
		run_q_len--;
	}
	return t;
}

void task_yield(void)
{
	struct task *old_task;
	struct task *new_task;

	last_yield = timer_get_ticks();

	/*
	 * If the current task is still ready to run and there's no other tasks ready, just
	 * continue.
	 * Note that this is simply an optimisation to avoid entering the critical section;
	 * we will later check again whether the current task is runnable to avoid racing
	 * with wakeups.
	 */
	if (likely(run_q_len == 0) && likely(current_task->state == TASK_RUNNING))
		return;

	/*
	 * It's not safe to yield in panic context.
	 */
	ASSERT(NULL == gPanicStr);

	/*
	 * Sanity check the world before we yield from this task.
	 *
	 * XXX we could avoid this for the idle task to improve latency for
	 * tasks made runnable in interrupt context.
	 */
	if (likely(&bootstrap_task != current_task)) {
#if defined(__arm__) && !defined(__clang_analyzer__)
		/* the bootstrap task might yield before calling task_init() */
		/* XXX really? */
		if (unlikely(nullptr_value != *(volatile uint32_t *)NULL))
			panic("reset vector overwritten while executing task '%s' (0x%08x)",
			    current_task->name, *(volatile uint32_t *)NULL);
#endif

		/* we don't know the base of the bootstrap stack, so it has no magic */
		if (unlikely(task_stack_magic != *(uint32_t *)current_task->stack_base))
			panic("task '%s' has overflowed its stack", current_task->name);
	}

	enter_critical_section();

	/* 
	 * We're headed to another task, so cancel the deep idle callout. Clear the
	 * flag too, just in case.
	 */
	callout_dequeue(&deep_idle_callout);
	deep_idle = false;

	/* if it's blocked or finished, it shouldn't go back to the run queue */
	old_task = current_task;
	if (old_task->state == TASK_RUNNING) {
		old_task->state = TASK_READY;
		/* the idle task gets scheduled when the run_q is empty; don't put it in there */
		if (old_task != &idle_task)
			insert_run_q_tail(old_task);
	}

	/* find the next task */
	new_task = pop_run_q_head();
	if (new_task == NULL) 
		new_task = &idle_task;

#if 0
	dprintf(DEBUG_SPEW, "task_yield: switching from task %p (%s) %d %p to task %p (%s) %d %p\n", 
		old_task, old_task->name, old_task->irq_disable_count, old_task->stack_base, 
		new_task, new_task->name, new_task->irq_disable_count, new_task->stack_base);
#endif

	ASSERT(new_task != old_task);
	
	new_task->state = TASK_RUNNING;
	current_task = new_task;

	/* XXX we could sanity-check the new task here before switching to it */
	arch_task_context_switch(&old_task->arch, &new_task->arch);

	last_switch = timer_get_ticks();

	exit_critical_section();
}

void task_start(struct task *t)
{
	ASSERT(NULL != t);

	if (t->state == TASK_INITIAL) {
		/* stick it in the run queue */
		t->state = TASK_READY;
		enter_critical_section();
		insert_run_q_tail(t);
		exit_critical_section();
	}
}

void task_exit(int return_code)
{
	/* set our return code and wake anyone waiting on us */
	current_task->return_code = return_code;
	current_task->state = TASK_FINISHED;
	wait_queue_wake_all(&current_task->return_waiters);

	task_count--;
	task_yield();

	/* should never reach here */
	panic("task_exit: should not be here\n");
}

int task_wait_on(struct task *t)
{
	ASSERT(NULL != t);

	if (t->state == TASK_FINISHED)
		return t->return_code;

	/* block on the task's wait queue and return with it's return code */
	// XXX race condition here for someone deleting the finished task before a 
	// waiter gets to read it's return code
	task_block(&t->return_waiters);
	ASSERT(t->state == TASK_FINISHED);

	return t->return_code;
}

void task_block(struct task_wait_queue *q)
{
	/* stick ourselves into the wait queue, set our state to BLOCKED, and yield */
	enter_critical_section();
	current_task->state = TASK_BLOCKED;
	list_add_tail(&q->task_list, &current_task->queue_node);

	task_yield();
	exit_critical_section();
}

/* happens at interrupt context */
static void task_sleep_callback(struct callout *c, void *_task)
{
	struct task *t = (struct task *)_task;

	ASSERT(c == &t->sleep_callout);
	ASSERT(t->state == TASK_SLEEPING);
	
	t->state = TASK_READY;
	insert_run_q_tail(t);
}

void task_sleep(utime_t delay)
{
	/* queue our sleep callout and reschedule */
	enter_critical_section();
	current_task->state = TASK_SLEEPING;
	callout_enqueue(&current_task->sleep_callout, delay, &task_sleep_callback, current_task);

	task_yield();
	exit_critical_section();
}

/* wait queue stuff */

void wait_queue_init(struct task_wait_queue *q)
{
	ASSERT(NULL != q);

	list_initialize(&q->task_list);
}

void wait_queue_destroy(struct task_wait_queue *q)
{
	wait_queue_wake_all(q);
}

int wait_queue_wake_all(struct task_wait_queue *q)
{
	struct task *t;
	struct task *temp;
	int ret = 0;

	/* iterate through the wait queue, sticking each of the waiting threads in the run queue */
	enter_critical_section();
	list_for_every_entry_safe(&q->task_list, t, temp, struct task, queue_node) {
		list_delete(&t->queue_node);

		ASSERT(t->state == TASK_BLOCKED);
		t->state = TASK_READY;
		insert_run_q_tail(t);
		ret++;
	}
	exit_critical_section();
	return ret;
}

int wait_queue_wake_one(struct task_wait_queue *q)
{
	struct task *t;
	int ret = 0;

	/* pop the head of the wait queue and stick in the run queue */
	enter_critical_section();
	t = list_remove_head_type(&q->task_list, struct task, queue_node);

	if (t) {
		ASSERT(t->state == TASK_BLOCKED);
		t->state = TASK_READY;
		insert_run_q_tail(t);
		ret = 1;
	}
	exit_critical_section();

	return ret;
}

/* profile the task manager */
void task_get_statistics(struct idle_statistics *s)
{
	u_int64_t now;

	enter_critical_section();
	now = timer_get_ticks();
	s->uptime_ticks = now - boot_time_ticks;
	s->ticksHz = timer_get_tick_rate();
	s->idle_ticks = idle_ticks;
	s->idles = shallow_idle_cnt;
	s->deep_idle_ticks = deep_idle_ticks;
	s->deep_idles = deep_idle_cnt;
	s->threshold_us = deep_idle_threshold_us;
	exit_critical_section();
}


void task_set_idle_threshold(u_int32_t us)
{
        deep_idle_threshold_us = us;
}

/* event stuff */

void event_init(struct task_event *event, uint32_t flags, bool initial_state)
{
	ASSERT(NULL != event);
	
	wait_queue_init(&event->wait);
	event->signalled = initial_state;
	event->flags = flags;
}

void event_signal(struct task_event *event)
{
	enter_critical_section();
	event->signalled = true;

	if (event->flags & EVENT_FLAG_AUTO_UNSIGNAL) {
		/* only allow one task to get through and unsignal */
		if (wait_queue_wake_one(&event->wait) > 0) {
			event->signalled = false;
		}
	} else {
		wait_queue_wake_all(&event->wait);
	}

	exit_critical_section();
}

void event_unsignal(struct task_event *event)
{
	event->signalled = false;
}

void event_wait(struct task_event *event)
{
	enter_critical_section();
	if (event->signalled == false) {
		/* block on the wait queue */
		task_block(&event->wait);
	} else {
		/* we're the first task here, unsignal the lock */
		if (event->flags & EVENT_FLAG_AUTO_UNSIGNAL)
			event->signalled = false;
	}
	exit_critical_section();
}

/* happens at interrupt context */
struct event_wait_callback_info {
    struct task_event *event;
    struct task *task;
    bool timed_out;
}; 

static void event_wait_callback(struct callout *c, void *_info)
{
	struct event_wait_callback_info *info = (struct event_wait_callback_info *)_info;
	struct task_wait_queue *q = &info->event->wait;
	struct task *task = info->task;
	struct task *t, *temp;

	/* we may be racing with a legitimate wakeup, in which case we have not timed out */
	if (TASK_BLOCKED != task->state)
		return;

	/* verify that we are blocked on the queue we think we're blocked on */
	list_for_every_entry_safe(&q->task_list, t, temp, struct task, queue_node) {
		if (t == task) {		
			list_delete(&t->queue_node);

			ASSERT(t->state == TASK_BLOCKED);
			t->state = TASK_READY;
			insert_run_q_tail(t);
			
			info->timed_out = true;
		}
	}
}

bool event_wait_timeout(struct task_event *event, utime_t delay)
{
	bool ret;

	enter_critical_section();
	ret = event->signalled;
	if (false == ret) {

		/* are we allowed to wait? */
		if (delay > 0) {
			struct event_wait_callback_info info;
			info.event = event;
			info.task = current_task;
			info.timed_out = false;

			/* block on the wait queue */
			callout_enqueue(&current_task->sleep_callout, delay, event_wait_callback, &info);
			task_block(&event->wait);
			callout_dequeue(&current_task->sleep_callout);

			/* if we didn't time out, the event was signalled and we woke because of it */
			if (false == info.timed_out)
				ret = true;
		}
	} else {
		/* we're the first task here, unsignal the lock */
		if (event->flags & EVENT_FLAG_AUTO_UNSIGNAL)
			event->signalled = false;
	}

	exit_critical_section();

	return ret;
}

/* debug stuff */
static const char *task_state_to_string(enum task_state state)
{
	switch (state) {
		case TASK_INITIAL:
			return "initial";
		case TASK_READY:
			return "ready";
		case TASK_RUNNING:
			return "running";
		case TASK_BLOCKED:
			return "blocked";
		case TASK_SLEEPING:
			return "sleeping";
		case TASK_FINISHED:
			return "finished";
		default:
			return "unknown";
	}
}

static void dump_task(struct task *t)
{
	uint32_t	i;

	dprintf(DEBUG_CRITICAL, "task %p (%s):\n", t, t->name);
	dprintf(DEBUG_CRITICAL, "    state %d (%s) interrupts %sabled (%d)\n", 
		t->state, 
		task_state_to_string(t->state),
		(t->irq_disable_count == 0) ? "en" : "dis",
		t->irq_disable_count);
	dprintf(DEBUG_CRITICAL, "    stack base %p, len 0x%zx, ", t->stack_base, t->stack_len);

	if (task_stack_magic != *(uint32_t *)t->stack_base) {
		dprintf(DEBUG_CRITICAL, "overflowed\n");
	} else {
		for (i = 0; i < (t->stack_len / sizeof(uint32_t)); i++)
			if (task_stack_magic != *((uint32_t *)t->stack_base + i))
				break;
		dprintf(DEBUG_CRITICAL, "used 0x%lx\n", t->stack_len - (i * sizeof(uint32_t)));
	}
	printf("    start routine %p, args %p\n", t->routine, t->arg);
}

static int do_task(int argc, struct cmd_arg *args)
{
	if (argc < 2) {
		printf("not enough arguments\n");
usage:
		printf("usage: \n");
		printf("%s list\n", args[0].str);
		printf("%s uptime\n", args[0].str);
		return -1;
	}

	if (!strcmp(args[1].str, "list")) {
		struct task *t;

		list_for_every_entry(&task_list, t, struct task, task_list_node) {
			dump_task(t);
		}
	} else if (!strcmp(args[1].str, "uptime")) {
		uint64_t	now;
		utime_t		elapsed, idle, active;

		now = timer_get_ticks();

		/* convert to microseconds */
		elapsed = timer_ticks_to_usecs(now - boot_time_ticks);
		idle = timer_ticks_to_usecs(idle_ticks);
		active = elapsed - idle;
		printf("up %u.%06u  idle %u.%06u  active %u.%06u\n",
		    (unsigned)(elapsed / (1000 * 1000)), (unsigned)(elapsed % (1000 * 1000)),
		    (unsigned)(idle / (1000 * 1000)), (unsigned)(idle % (1000 * 1000)),
		    (unsigned)(active / (1000 * 1000)), (unsigned)(active % (1000 * 1000)));
	} else {
		printf("unrecognized subcommand\n");
		goto usage;
	}

	return 0;
}

MENU_COMMAND_DEVELOPMENT(task, do_task, "examine system tasks", NULL);

/*
 * Validate that a task pointer points to something that looks like a task.
 */
static bool task_ptr_sanity(struct task *t)
{
	if ((u_int32_t)t % 4) {
		dprintf(DEBUG_CRITICAL, "task pointer %p bad, task list corrupt\n", t);
		return(false);
	}
	/* add extra bad-address checks here */

	if ((TASK_MAGIC != t->magic) || (TASK_MAGIC2 != t->magic2)) {
		dprintf(DEBUG_CRITICAL, "task %p (corrupt)\n", t);
		return(false);
	}
	return(true);
}

static void do_task_panic(void *junk)
{
	struct task	*t;
	int		tcount;

	dprintf(DEBUG_CRITICAL, "current ");
	if (task_ptr_sanity(current_task)) {
		dump_task(current_task);
		arch_backtrace_current_task(current_task->stack_base, current_task->stack_len);
	}
	tcount = 0;
	list_for_every_entry(&task_list, t, struct task, task_list_node) {
		if (current_task != t) {
			if (!task_ptr_sanity(t))
				break;

			/* paranoia */
			t->name[TASK_NAME_MAX] = 0;
			dump_task(t);
			arch_backtrace_task(&t->arch, t->stack_base, t->stack_len);
		}
		/* XXX tasks aren't properly reaped so many tasks exiting will confuse this */
		if (tcount++ > (task_count + 2)) {
			dprintf(DEBUG_CRITICAL, "too many tasks, task list probably corrupt\n");
			break;
		}
	}
}

PANIC_HOOK(task, do_task_panic, NULL);
