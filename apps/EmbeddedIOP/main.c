/*
 * Copyright (C) 2007-2014 Apple Inc. All rights reserved.
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
#include <cbuffer.h>
#include <platform.h>
#include <platform/memmap.h>
#include <platform/int.h>
#include <platform/soc/hwisr.h>
#include <platform/timer.h>
#include <sys.h>
#include <sys/boot.h>
#include <sys/menu.h>
#include <sys/task.h>
#include <lib/mib_def.h>

#include "iop.h"
#include "qwi.h"
#include "qwi_protocol.h"
#include "EmbeddedIOPProtocol.h"
#include "clock_management.h"
#include "clock_stepping.h"

/* XXX should get these automagically */
#if WITH_FUNCTION_SDIO
extern int iop_sdio_task(void *arg);
#endif

/* configuration, patched by host before we start */
struct iop_configuration _iop_config = {
	.magic = IOP_CONFIG_MAGIC
};

static int host_channel;

static struct task_event host_command_event;

static int iop_message_channel;
static struct task_event iop_message_event;
static struct task_event iop_console_event;
static union iop_message *iop_message_buffer;
static bool iop_suspended;
struct iop_ping_tracker gControlMessages[128];
int gControlMessageCount = 0;

static int  no_idle_task(void *arg __unused);
static int  host_command_task(void *cfg);
static void host_command_hook(void *user_data);
static bool host_command_process(void);

static union iop_message *iop_message_alloc(utime_t allowed_delay);
static void iop_message_wakeup(void *arg __unused);
static void iop_message_tty(CBUFFER *pcb);
static int host_console_task(void *arg __unused);

/* the host command task is just another function */
IOP_FUNCTION(iop, host_command_task, 1024, IOP_CONTROL_CHANNEL);

/* sleep/wakeup hook */
static void sleep_hook(int mode);
IOP_SLEEP_HOOK(iop, sleep_hook);

/* console */
CBUFFER console_buffer;
#ifndef APPLICATION_CONSOLE_BUFFER
# define APPLICATION_CONSOLE_BUFFER 256
#endif

/* NMI */
static void 		iop_nmi_handler(void *junk);

#if DEBUG_BUILD
static char logo[] = {
  0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
  0x20, 0x20, 0x2f, 0x5c, 0x2c, 0x25, 0x2c, 0x5f, 0x0a, 0x20, 0x20, 0x20,
  0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x5c,
  0x25, 0x25, 0x25, 0x2f, 0x2c, 0x5c, 0x0a, 0x20, 0x20, 0x20, 0x20, 0x20,
  0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x5f, 0x2e, 0x2d, 0x22, 0x25,
  0x25, 0x7c, 0x2f, 0x2f, 0x25, 0x0a, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
  0x20, 0x20, 0x20, 0x20, 0x2e, 0x27, 0x20, 0x20, 0x2e, 0x2d, 0x22, 0x20,
  0x20, 0x2f, 0x25, 0x25, 0x25, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
  0x20, 0x46, 0x4f, 0x52, 0x20, 0x50, 0x4f, 0x4e, 0x59, 0x21, 0x0a, 0x20,
  0x20, 0x20, 0x20, 0x20, 0x20, 0x5f, 0x2e, 0x2d, 0x27, 0x5f, 0x2e, 0x2d,
  0x22, 0x20, 0x30, 0x29, 0x20, 0x20, 0x20, 0x5c, 0x25, 0x25, 0x25, 0x0a,
  0x20, 0x20, 0x20, 0x20, 0x20, 0x2f, 0x2e, 0x5c, 0x2e, 0x27, 0x20, 0x20,
  0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x5c, 0x25, 0x25,
  0x25, 0x0a, 0x20, 0x20, 0x20, 0x20, 0x20, 0x5c, 0x20, 0x2f, 0x20, 0x20,
  0x20, 0x20, 0x20, 0x20, 0x5f, 0x2c, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
  0x25, 0x25, 0x25, 0x0a, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x60, 0x22,
  0x2d, 0x2d, 0x2d, 0x22, 0x7e, 0x60, 0x5c, 0x20, 0x20, 0x20, 0x5f, 0x2c,
  0x2a, 0x27, 0x5c, 0x25, 0x25, 0x27, 0x20, 0x20, 0x20, 0x5f, 0x2c, 0x2d,
  0x2d, 0x22, 0x22, 0x22, 0x22, 0x2d, 0x2c, 0x25, 0x25, 0x2c, 0x0a, 0x20,
  0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
  0x20, 0x20, 0x29, 0x2a, 0x5e, 0x20, 0x20, 0x20, 0x20, 0x20, 0x60, 0x22,
  0x22, 0x7e, 0x7e, 0x60, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
  0x20, 0x20, 0x5c, 0x25, 0x25, 0x25, 0x2c, 0x0a, 0x20, 0x20, 0x20, 0x20,
  0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x5f, 0x2f, 0x20,
  0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
  0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
  0x5c, 0x25, 0x25, 0x25, 0x0a, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
  0x20, 0x20, 0x5f, 0x2e, 0x2d, 0x60, 0x2f, 0x20, 0x20, 0x20, 0x20, 0x20,
  0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
  0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x7c, 0x25,
  0x25, 0x2c, 0x5f, 0x5f, 0x5f, 0x0a, 0x20, 0x20, 0x20, 0x20, 0x20, 0x5f,
  0x2e, 0x2d, 0x22, 0x20, 0x20, 0x20, 0x2f, 0x20, 0x20, 0x20, 0x20, 0x20,
  0x20, 0x2c, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
  0x20, 0x2c, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x2c, 0x7c,
  0x25, 0x25, 0x20, 0x20, 0x20, 0x2e, 0x60, 0x5c, 0x0a, 0x20, 0x20, 0x20,
  0x20, 0x2f, 0x5c, 0x20, 0x20, 0x20, 0x20, 0x20, 0x2f, 0x20, 0x20, 0x20,
  0x20, 0x20, 0x20, 0x2f, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
  0x20, 0x20, 0x20, 0x20, 0x20, 0x60, 0x5c, 0x20, 0x20, 0x20, 0x20, 0x20,
  0x20, 0x20, 0x5c, 0x25, 0x27, 0x20, 0x20, 0x20, 0x5c, 0x20, 0x2f, 0x0a,
  0x20, 0x20, 0x20, 0x20, 0x5c, 0x20, 0x5c, 0x20, 0x5f, 0x2c, 0x2f, 0x20,
  0x20, 0x20, 0x20, 0x20, 0x20, 0x2f, 0x60, 0x7e, 0x2d, 0x2e, 0x5f, 0x20,
  0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x5f, 0x2c, 0x60, 0x5c,
  0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x5c, 0x60, 0x22, 0x22, 0x7e, 0x7e,
  0x60, 0x0a, 0x20, 0x20, 0x20, 0x20, 0x20, 0x60, 0x22, 0x60, 0x20, 0x2f,
  0x2d, 0x2e, 0x2c, 0x5f, 0x20, 0x2f, 0x27, 0x20, 0x20, 0x20, 0x20, 0x20,
  0x20, 0x60, 0x7e, 0x22, 0x2d, 0x2d, 0x2d, 0x2d, 0x22, 0x7e, 0x20, 0x20,
  0x20, 0x20, 0x60, 0x5c, 0x20, 0x20, 0x20, 0x20, 0x20, 0x5c, 0x0a, 0x20,
  0x20, 0x20, 0x6a, 0x67, 0x73, 0x20, 0x20, 0x20, 0x5c, 0x5f, 0x5f, 0x5f,
  0x2c, 0x27, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
  0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
  0x20, 0x5c, 0x2e, 0x2d, 0x22, 0x60, 0x2f, 0x0a, 0x20, 0x20, 0x20, 0x20,
  0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
  0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
  0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x60,
  0x2d, 0x2d, 0x27, 0x0a, 0x00
};
#endif

int
_main(void)
{
	void				**func_cursor;
	const struct iop_function	*func;

#if DEBUG_BUILD
	printf("\n%s", logo);
#endif
	dprintf(DEBUG_INFO, "####\n#### " CONFIG_PROGNAME_STRING ": " XBS_BUILD_TAG "\n####\n");

	/* initialize the cpu */
	dprintf(DEBUG_INFO, "doing CPU init\n");
	arch_cpu_init(false);

        /* do early initialization of hardware */
        dprintf(DEBUG_INFO, "doing early platform hardware init\n");
        platform_early_init();

	/* bring up system services (cpu, tasks, callout) */
	dprintf(DEBUG_INFO, "doing system init\n");
	sys_init();

#ifdef HEAP_EXT_SIZE
	/* 
	 * If we have extra memory for the heap, add it.  This needs
	 * to happen after main memory has been initialized.
	 */
	dprintf(DEBUG_INFO, "Adding %llu bytes at %p to heap.\n", HEAP_EXT_SIZE, (void *)HEAP_EXT_BASE);
	heap_add_chunk((void *)HEAP_EXT_BASE, HEAP_EXT_SIZE, true);
#endif

	/* register our doorbell handler and enable the doorbell */
	platform_init_iop_doorbell((int_handler)qwi_doorbell, NULL);

	/* 
	 * Start the host console task and give it a chance to run, so that we have
	 * a console past this point.
	 */
	task_start(task_create("console", host_console_task, NULL, 512));
	task_yield();

	/*
	 * Iterate IOP function tasks, starting each as we go.
	 */
        LINKER_SET_FOREACH(func_cursor, iop_function) {
		func = (const struct iop_function *)*func_cursor;
		if (NULL != func->entrypoint)  {
			dprintf(DEBUG_INFO, "starting %s\n", func->function_name);
			task_start(task_create(
				    func->function_name,
				    func->entrypoint,
				    (void *)&_iop_config.channel[func->control_channel],
				    func->stack_allocation));
		}
	}
#if DEBUG_BUILD
	dprintf(DEBUG_INFO, "starting debug console\n");
	task_start(task_create("menu", menu_task, NULL, 8192));
#endif

	/* start the slopsucker task if we don't want to go idle */
	if (_iop_config.options & IOP_OPTION_NO_IDLE)
		task_start(task_create("slopsucker", no_idle_task, NULL, 512));

	/* start the slopsucker task if we don't want to go idle */
	if (_iop_config.options & IOP_OPTION_DO_CLOCK_MGMNT)
		SetParticipateInClockStateManagement(true);

        /* if the task manager needs retuning, do so */
        if (_iop_config.deep_idle_us != 0)
		task_set_idle_threshold(_iop_config.deep_idle_us);

	/* register our NMI handler */
	platform_init_nmi(iop_nmi_handler, NULL);
	
	dprintf(DEBUG_INFO, "bootstrap task terminating\n");

	task_exit(0);
}


/*
 * This task is run when we don't want the IOP to idle, normally when
 * JTAG debugging is enabled on the host.
 */
static int
no_idle_task(void *arg __unused)
{
	for (;;) {
		task_yield();
	}
	return(0);
}

static int
host_command_task(void *cfg)
{
	struct iop_channel_config *channel = (struct iop_channel_config *)cfg;
	
	dprintf(DEBUG_INFO, "## IOP control task starting\n");

	/* establish the host communications channel */
	event_init(&host_command_event, EVENT_FLAG_AUTO_UNSIGNAL, false);
	dprintf(DEBUG_INFO, "## opening host channel\n");
	host_channel = qwi_instantiate_channel(
		"iop control",
		QWI_ROLE_CONSUMER,
		channel->ring_size,
		(void *)mem_static_map_cached(channel->ring_base),
		host_command_hook,
		&host_command_event);

	iop_suspended = false;
	for (;;) {
		/*
		 * Spin handling host commands
		 */
		while (host_command_process())
			;

		if (!iop_suspended) {
			/*
			 * Normal operation; wait for an event signalling new work.
			 */
			dprintf(DEBUG_SPEW, "## waiting for host command\n");
			event_wait(&host_command_event);
		} else {
			/*
			 * We are suspended; act as though we had received a doorbell
			 * interrupt.
			 */
			host_command_hook(&host_command_event);
		}
	}
	return(0);
}

static void
host_command_hook(void *user_data)
{
	struct task_event	*event = (struct task_event *)user_data;

	/* pass it through to task context */
	event_signal(event);
}

static bool
host_command_process(void)
{
	uint32_t		message;
	union iop_command	*command;
	bool			do_sleep = false;
	void			**hook_cursor;
	const struct iop_sleep_hook *hook;
	struct idle_statistics stats; // XXX this is 'big', almost never used; ok on stack?

	/* look to see if there's an item waiting for us */
	if (qwi_receive_item(host_channel, &message) == -1)
		return(false);

	/* find the command structure based on the message */
	command = (union iop_command *)mem_static_map_cached(message);
	if (command == MAP_FAILED)
		panic("received bad command pointer on control channel");

	/*
	 * Flush any cached item contents we might have lying around - we are guaranteed
	 * that the command size is a multiple of our cacheline size.
	 */
	platform_cache_operation(CACHE_INVALIDATE, (void *)command, sizeof(*command));

	switch (command->generic.opcode) {
	case IOP_CMD_NOP:
//		dprintf(DEBUG_INFO, "## host NOP\n");
		// Store information on ping commands received, in a global structure
		// For use during debug of watchdog time outs
		command->generic.result = IOP_RESULT_SUCCESS;
		gControlMessages[gControlMessageCount].timestamp = timer_get_ticks();
		gControlMessages[gControlMessageCount].record.opcode = command->ping.opcode;
		gControlMessages[gControlMessageCount].record.result = command->ping.result;
		gControlMessages[gControlMessageCount].record.ping_id = command->ping.ping_id;
		gControlMessageCount = (gControlMessageCount + 1) % 128;
		break;

	case IOP_CMD_TTYIN:
		debug_pushchar(command->ttyin.c);
		command->generic.result = IOP_RESULT_SUCCESS;
		break;
		
	case IOP_CMD_SLEEP:
		dprintf(DEBUG_INFO, "## host SLEEP\n");
		do_sleep = true;
		command->generic.result = IOP_RESULT_SUCCESS;
		break;

	case IOP_CMD_SUSPEND:
		dprintf(DEBUG_INFO, "## host SUSPEND\n");
		iop_suspended = true;
		platform_cache_operation(CACHE_CLEAN, 0, 0);
		command->generic.result = IOP_RESULT_SUCCESS;
		break;
		
	case IOP_CMD_RESUME:
		dprintf(DEBUG_INFO, "## host RESUME\n");
		iop_suspended = false;
		command->generic.result = IOP_RESULT_SUCCESS;
		break;
		
	case IOP_CMD_INSTRUMENT:
		dprintf(DEBUG_INFO, "## host INSTRUMENT\n");
		iop_suspended = false;
		task_get_statistics(& stats);
		command->generic.result = IOP_RESULT_SUCCESS;
		command->instr.uptime_ticks = stats.uptime_ticks;
		command->instr.idles = stats.idles;
		command->instr.deep_idles = stats.deep_idles;
		command->instr.deep_idle_ticks = stats.deep_idle_ticks;
		command->instr.idle_ticks = stats.idle_ticks;
		command->instr.threshold_us = stats.threshold_us;
		command->instr.ticksHz = stats.ticksHz;
		break;
		
	default:
		dprintf(DEBUG_CRITICAL, "## unrecognised host opcode 0x%x in command %p\n", command->generic.opcode, command);
		command->generic.result = IOP_RESULT_ERROR;
		break;
	}
	
	/* this should never fail because we just pulled an item out and so we must own a slot */
	platform_cache_operation(CACHE_CLEAN, (void *)command, sizeof(*command));
	qwi_send_item(host_channel, message);

	/* sleep *after* we have replied */
	if (do_sleep) {
		
		/* tell anyone that cares that we're sleeping */
		LINKER_SET_FOREACH(hook_cursor, iop_sleep_hook) {
			hook = (const struct iop_sleep_hook *)*hook_cursor;
			hook->func(IOP_SLEEP_MODE_SLEEPING);
		}
		
		/* go to sleep */
		platform_sleep();

		/* and tell them we've woken up again */
		LINKER_SET_FOREACH(hook_cursor, iop_sleep_hook) {
			hook = (const struct iop_sleep_hook *)*hook_cursor;
			hook->func(IOP_SLEEP_MODE_WAKING);
		}
	}
	
	return(true);
}

static void
sleep_hook(int mode)
{
	switch(mode) {
	case IOP_SLEEP_MODE_WAKING:
		dprintf(DEBUG_INFO, "enabling doorbell\n");
		platform_unmask_doorbell();
		// On some system (like AE2), put the system into lowest power state
		// SetClockState will be no-op on other systems
		SetClockState(kClockRequestPowerManager, kClockValueLow);
		break;

	case IOP_SLEEP_MODE_SLEEPING:
		dprintf(DEBUG_INFO, "disabling doorbell\n");
		// on some system(like AE2), restore clocks to high before we go to sleep
		SetClockState(kClockRequestPowerManager, kClockValueHigh);
		platform_mask_doorbell();
		break;
	}
}


/*
 * Putchar hook.
 */

char		gIOPPanicLog[IOP_PANIC_LOG_SIZE];
u_int32_t	gIOPPanicBytes;

void
application_putchar(int c)
{
	/* if we are panicking, append the data to the panic log */
	if (NULL != gPanicStr) {
		/*
		 * Printf emits nuls because it's not smart enough to tell the difference between
		 * strings and streams, so strip them here.
		 */
		if ((0 != c) && (gIOPPanicBytes < IOP_PANIC_LOG_SIZE))
			gIOPPanicLog[gIOPPanicBytes++] = c;

	} else {
		/*
		 * If we have a console buffer, and this isn't a nul, try to stuff it
		 * there and wake up the output task.
		 */
		if ((true == cb_initialized(&console_buffer)) && (0 != c)) {
#if APPLICATION_CONSOLE_RELIABLE
			/* Try to be lossless - yield if the buffer is full */
			while (!cb_free_space(&console_buffer)) {
				task_yield();
			}
#endif
			(void)cb_putc(&console_buffer, c);
			event_signal(&iop_console_event);
		}
	}
}

/*
 * NMI handling
 */
static void 
iop_nmi_handler(void *junk)
{
	panic("NMI");
}

/*******************************************************************************
 * IOP-to-host messaging support
 *
 * N.B. Since this is used to carry console output, (d)printf should not be
 * called in this code.
 */

/*
 * Allocate a message to send to the host.
 *
 * Note that it is not safe to block between allocating a message and sending it.
 */
static union iop_message *
iop_message_alloc(utime_t allowed_delay)
{
	utime_t			deadline, t;
	int			idx;

	/* nothing we can do if we don't have a buffer */
	if (NULL == iop_message_buffer)
		return(NULL);
	
	/* try to get a slot right away */
	if (-1 != (idx = qwi_next_send_index(iop_message_channel)))
		return(iop_message_buffer + idx);
	if (IOP_MESSAGE_NO_WAIT == allowed_delay)
		return(NULL);

	/* spin waiting for a free slot */
	deadline = system_time() + allowed_delay;
	for (;;) {
		/* get the current time and see if we've timed out */
		t = system_time();
		if (t >= deadline)
			return(NULL);

		/* sleep waiting for notification or timeout */
		event_wait_timeout(&iop_message_event, deadline - t);

		/* try again to get a message slot */
		if (-1 != (idx = qwi_next_send_index(iop_message_channel)))
			return(iop_message_buffer + idx);
	}
}

/*
 * Reclaim a message the host has accepted.
 */
static void
iop_message_wakeup(void *arg __unused)
{
	bool		replies;
	uint32_t	message;

	/* reap message replies */
	replies = false;
	while (qwi_receive_item(iop_message_channel, &message) != -1)
		replies = true;

	/* if we got at least one reply, wake anyone trying to send */
	if (replies)
		event_signal(&iop_message_event);
}

/*
 * Try to send a trace message to the host.
 */
void
iop_message_trace(const char *ident, uint32_t arg0, uint32_t arg1, uint32_t arg2, uint32_t arg3)
{
	union iop_message	*msg;
	uint64_t		timestamp;

	/* get the raw time shared with the host */
	timestamp = timer_get_ticks();

	/* now get a message - wait a little while for it but not forever as we might deadlock the host */
	if (NULL != (msg = iop_message_alloc(1000))) {

		/* populate the message */
		msg->gen.opcode = IOP_MSG_TRACE;
		msg->gen.size = sizeof(msg->trace);
		msg->trace.ident = mem_static_map_physical((uint32_t)ident);
		msg->trace.arg[0] = arg0;
		msg->trace.arg[1] = arg1;
		msg->trace.arg[2] = arg2;
		msg->trace.arg[3] = arg3;
		msg->trace.timestamp = timestamp;

		/* push it to memory */
		platform_cache_operation(CACHE_CLEAN, msg, IOP_MESSAGE_MAX);

		/* and give it to the host */
		qwi_send_item(iop_message_channel, QWI_ENCODE_ORDINAL(msg - iop_message_buffer));
	}
}

/*
 * Try to send a tty message to the host.
 */
static void
iop_message_tty(CBUFFER *pcb)
{
	union iop_message	*msg;
	unsigned int		count;

	/* get a message */
	if (NULL == (msg = iop_message_alloc(IOP_MESSAGE_WAIT_FOREVER)))
		panic("the console is dead, Jim");
		
	/* get as many  bytes from the cbuffer into our message as we can */
	count = cb_read(pcb, (unsigned char *)msg->tty.bytes, IOP_MSG_TTY_MAXLEN);
	msg->gen.opcode = IOP_MSG_TTY;
	msg->gen.size = sizeof(msg->gen) + count;
		
	/* push it to memory */
	platform_cache_operation(CACHE_CLEAN, msg, IOP_MESSAGE_MAX);

	/* and give it to the host */
	qwi_send_item(iop_message_channel, QWI_ENCODE_ORDINAL(msg - iop_message_buffer));
}

/*
 * Buffer task outputting to the console.
 */
static int
host_console_task(void *arg __unused)
{

	/* allocate a cbuffer for host console output */
	cb_create(&console_buffer, APPLICATION_CONSOLE_BUFFER);
	
	/* establish the message channel */
	iop_message_buffer = (union iop_message *)mem_static_map_cached(_iop_config.message_buffer);
	event_init(&iop_message_event, EVENT_FLAG_AUTO_UNSIGNAL, false);
	event_init(&iop_console_event, EVENT_FLAG_AUTO_UNSIGNAL, false);
	dprintf(DEBUG_INFO, "## opening IOP message channel\n");
	iop_message_channel = qwi_instantiate_channel(
		"iop message",
		QWI_ROLE_PRODUCER,
		_iop_config.channel[IOP_MESSAGE_CHANNEL].ring_size,
		(void *)mem_static_map_cached(_iop_config.channel[IOP_MESSAGE_CHANNEL].ring_base),
		iop_message_wakeup,
		NULL);

	for (;;) {
		/* drain the console buffer */
		while (cb_readable_size(&console_buffer) > 0)
			iop_message_tty(&console_buffer);
		/* wait for more */
		event_wait(&iop_console_event);
	}

	return(0);
}

