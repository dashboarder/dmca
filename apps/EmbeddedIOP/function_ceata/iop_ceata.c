/*
 * Copyright (C) 2008 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */

#include <debug.h>

#include <AssertMacros.h>

#include <platform/memmap.h>
#include <platform.h>

#include <sys/task.h>

#include <qwi.h>
#include <EmbeddedIOPProtocol.h>

#include "iop_ceata.h"
#include "iop_ceata_protocol.h"

#include "ceata.h"

static struct task_event ceata_message_event;

static int ceata_channel;
static bool ceata_message_process(void);

static IOPCEATA_status_t	ceata_reset(void);
static IOPCEATA_status_t	ceata_identify(IOPCEATA_Identify *identify);
static IOPCEATA_status_t	ceata_cmd(CEATA_cmd_t cmd);
static IOPCEATA_status_t	ceata_io(CEATA_cmd_t cmd,
    UInt32 starting_lba,
    UInt32 lba_count,
    UInt32 segment_count,
    IOPCEATA_dma_segment *segments);

int
iop_ceata_task(void *cfg)
{
	struct iop_channel_config *channel = (struct iop_channel_config *)cfg;
    
	dprintf(DEBUG_SPEW, "@@ CEATA task starting\n");

	check(kIOPCEATA_COMMAND_SIZE == sizeof(IOPCEATA_Command));

	/* establish the host communications channel */
	event_init(&ceata_message_event, EVENT_FLAG_AUTO_UNSIGNAL, false);
	dprintf(DEBUG_SPEW, "** opening ceata channel\n");
	ceata_channel = qwi_instantiate_channel(
		"ceata command",
		QWI_ROLE_CONSUMER,
		channel->ring_size,
		mem_static_map_cached(channel->ring_base),
		(qwi_channel_hook)event_signal,
		(void *)&ceata_message_event);

	for (;;) {
		dprintf(DEBUG_SPEW, "@@ waiting for message on ceata channel\n");
		while (ceata_message_process()) {
			// eat all available messages
		}
		event_wait(&ceata_message_event);
	}

	return(0);
}

static bool
ceata_message_process(void)
{
	uint32_t         message;
	IOPCEATA_Command*  command;

	dprintf(DEBUG_SPEW, "@@ handling host message\n");
    
	/* look to see if there's an item waiting for us */
	if (qwi_receive_item(ceata_channel, &message) == -1)
		return(false);
    
	dprintf(DEBUG_SPEW, "@@ received ceata message\n");

	/* find the command structure based on the message */
	command = (IOPCEATA_Command*)mem_static_map_cached(message);
    
	/*
	 * Flush any cached item contents we might have lying around - we are guaranteed
	 * that the command size is a multiple of our cacheline size.
	 */
	platform_cache_operation(CACHE_INVALIDATE,
	    (void *)command, 
	    sizeof(*command));

	/* 
	 * TODO: make this part of the API and push this
	 * architecture-specific command handling down into the s5l8920x
	 * platform directory.
	 */
	switch (command->iopceata.opcode) {

	case kIOPCEATA_OPCODE_RESET:
		dprintf(DEBUG_SPEW, "@@ RESET\n");
		command->iopceata.status = ceata_reset();
		break;
		
	case kIOPCEATA_OPCODE_IDENTIFY:
		dprintf(DEBUG_SPEW, "@@ IDENTIFY\n");
		command->iopceata.status = ceata_identify((IOPCEATA_Identify *)command);
		break;
		
	case kIOPCEATA_OPCODE_READ:
		dprintf(DEBUG_SPEW, "@@ READ\n");
		command->iopceata.status = ceata_io(
			CEATA_READ_DMA_EXT,
			command->read_write.starting_lba,
			command->read_write.lba_count,
			command->read_write.segment_count,
			&command->read_write.data_segments[0]);
		break;
		
	case kIOPCEATA_OPCODE_WRITE:
		dprintf(DEBUG_SPEW, "@@ WRITE\n");
		command->iopceata.status = ceata_io(
			CEATA_WRITE_DMA_EXT,
			command->read_write.starting_lba,
			command->read_write.lba_count,
			command->read_write.segment_count,
			&command->read_write.data_segments[0]);
		break;
		
	case kIOPCEATA_OPCODE_STANDBY:
		dprintf(DEBUG_SPEW, "@@ STANDBY\n");
		command->iopceata.status = ceata_cmd(CEATA_STANDBY_IMMEDIATE);
		break;
		
	case kIOPCEATA_OPCODE_FLUSH:
		dprintf(DEBUG_SPEW, "@@ FLUSH\n");
		command->iopceata.status = ceata_cmd(CEATA_FLUSH_CACHE_EXT);
		break;
		
	default:
		dprintf(DEBUG_CRITICAL, "@@ ERROR: unrecognised ceata opcode 0x%x", 
		    command->iopceata.opcode);
		command->iopceata.status = kIOPCEATA_STATUS_PARAM_INVALID;
		break;
	}

	dprintf(DEBUG_SPEW, "@@ done processing ceata message with status 0x%08x\n", command->iopceata.status);

	platform_cache_operation(CACHE_CLEAN, 
	    (void *)command, 
	    sizeof(IOPCEATA_Command));

	qwi_send_item(ceata_channel, message);

	dprintf(DEBUG_SPEW, "@@ signaled completion of ceata message to host\n");

	return(true);
}


static IOPCEATA_status_t
ceata_reset(void)
{
	return(kIOPCEATA_STATUS_UNIMPLEMENTED);
}

static IOPCEATA_status_t
ceata_identify(IOPCEATA_Identify *identify)
{
	return(kIOPCEATA_STATUS_UNIMPLEMENTED);
}

static IOPCEATA_status_t
ceata_cmd(CEATA_cmd_t cmd)
{
	return(kIOPCEATA_STATUS_UNIMPLEMENTED);
}
	
static IOPCEATA_status_t
ceata_io(CEATA_cmd_t cmd,
	UInt32 starting_lba,
	UInt32 lba_count,
	UInt32 segment_count,
	IOPCEATA_dma_segment *segments)
{
	return(kIOPCEATA_STATUS_UNIMPLEMENTED);
}
