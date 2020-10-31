/*
 * Copyright (C) 2010-2011 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */

#include "timestamper.h"

#include <debug.h>
#include <AssertMacros.h>
#include <platform.h>
#include <platform/timer.h>
#include <sys/task.h>

#include <iop.h>
#include <qwi.h>
#include <qwi_protocol.h>
#include <EmbeddedIOPProtocol.h>
#include <arch.h>
#include <arch/arm/arm.h>
#include <drivers/audio/audio.h>

#define kLogTimestamps 0

static IOPAUDIODSP_Command *timestamper_message_buffer;

void set_timestamper_message_buffer(IOPAUDIODSP_Command *buffer)
{
	timestamper_message_buffer = buffer;
}

static int timestamper_message_channel;
static struct task_event timestamper_message_event;
static struct task_event timestamper_runtask_event;

static int timestamper_task(void *cfg);
static void timestamper_sleep(int mode);

IOP_FUNCTION(timestamper, timestamper_task, 8*1024, AUDIODSP_TIMER_CHANNEL);
IOP_SLEEP_HOOK(timestamper, timestamper_sleep);

/*
 * Get a message to send to the Host
 */
static IOPAUDIODSP_Command *
get_timestamper_message(utime_t allowed_delay)
{
	utime_t			deadline, t;
	int			idx;
	
	/* nothing we can do if we don't have a buffer */
	if (NULL == timestamper_message_buffer)
		return(NULL);
	
	/* try to get a slot right away */
	if (-1 != (idx = qwi_next_send_index(timestamper_message_channel)))
		return(timestamper_message_buffer + idx);
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
		event_wait_timeout(&timestamper_message_event, deadline - t);
		
		/* try again to get a message slot */
		if (-1 != (idx = qwi_next_send_index(timestamper_message_channel)))
			return(timestamper_message_buffer + idx);
	}
}

/*
 * Reclaim a message the host has accepted.
 */
static void
timestamper_message_wakeup(void *arg __unused)
{
	bool		replies;
	uint32_t	message;
	
	/* reap message replies */
	replies = false;
	while (qwi_receive_item(timestamper_message_channel, &message) != -1)
		replies = true;
	
	/* if we got at least one reply, wake anyone trying to send */
	if (replies)
	{
		event_signal(&timestamper_message_event);
	}
}

/*
 * Try to send a trace message to the host.
 */
void
send_timestamp(uint32_t samplesTransferred, uint64_t timestamp)
{
	IOPAUDIODSP_Command	*msg;
	
	/* now get a message - wait a little while for it but not forever as we might deadlock the host */
	if (NULL != (msg = get_timestamper_message(1000))) {
		
		/* populate the message */
		msg->timestamp.mIOPHeader.mOpcode = kIOPAUDIODSP_OPCODE_TIMESTAMP;
		msg->timestamp.mIOPHeader.mStatus = kIOPAUDIODSP_STATUS_SUCCESS;
		msg->timestamp.mSampleCount = samplesTransferred;
		msg->timestamp.mTimeStamp = timestamp;
		
#if kLogTimestamps
		dprintf(DEBUG_CRITICAL, "Timestamp %ull\n", timestamp);
#endif
		
		/* push it to memory */
		platform_cache_operation(CACHE_CLEAN, msg, sizeof(*msg));
		
		/* and give it to the host */
		qwi_send_item(timestamper_message_channel, QWI_ENCODE_ORDINAL(msg - timestamper_message_buffer));
	}
	else
	{
		dprintf(DEBUG_CRITICAL, "couldn't get a timestamp message\n");
	}
}


static int
timestamper_task(void *cfg)
{
	struct iop_channel_config *channel = (struct iop_channel_config *)cfg;
	
	dprintf(DEBUG_SPEW, "@@ Timestamper task starting\n");
	
	/* establish the host communications channel */
	event_init(&timestamper_message_event, EVENT_FLAG_AUTO_UNSIGNAL, false);
	event_init(&timestamper_runtask_event, EVENT_FLAG_AUTO_UNSIGNAL, false);
	
	dprintf(DEBUG_SPEW, "** opening audiodsp channel\n");
	timestamper_message_channel = qwi_instantiate_channel("audio timestamp",
							  QWI_ROLE_PRODUCER,
							  channel->ring_size,
							  (void *)mem_static_map_cached(channel->ring_base),
							  timestamper_message_wakeup,
							  NULL);
	
#if WITH_VFP && !WITH_VFP_ALWAYS_ON
	/* Little doubt we'll need VFP/Neon */
	arch_task_fp_enable(true);
#endif

	for (;;) {
		dprintf(DEBUG_SPEW, "Waiting on run event\n");
		event_wait(&timestamper_runtask_event);
	}
	
	return(0);
}

static void
timestamper_sleep(int mode)
{
}

