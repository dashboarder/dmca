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

#include <platform/soc/hwclocks.h>
#include <platform/soc/pmgr.h>
#include <platform/memmap.h>
#include <platform.h>

#include <sys/task.h>

#include <iop.h>
#include <qwi.h>
#include <EmbeddedIOPProtocol.h>

#include "iop_sdio_protocol.h"
#include "iop_sdio_wrapper.h"

#define cache_op_size(buf_size) (((buf_size) + (CPU_CACHELINE_SIZE-1)) & ~(CPU_CACHELINE_SIZE-1))


static struct task_event sdio_message_event;

static int sdio_channel;
static void sdio_message_wakeup(void *arg);
static bool sdio_message_process(void);

static int iop_sdio_task(void *cfg);

IOP_FUNCTION(sdio, iop_sdio_task, 1024, SDIO_CONTROL_CHANNEL);

static int
iop_sdio_task(void *cfg)
{
	struct iop_channel_config *channel = (struct iop_channel_config *)cfg;
    
	dprintf(DEBUG_SPEW, "++ SDIO task starting\n");

	check(kIOPSDIO_COMMAND_SIZE == sizeof(IOPSDIO_Command));

	/* establish the host communications channel */
	event_init(&sdio_message_event, EVENT_FLAG_AUTO_UNSIGNAL, false);
	dprintf(DEBUG_SPEW, "++ opening sdio channel\n");
	sdio_channel = qwi_instantiate_channel(
		"sdio message",
		QWI_ROLE_CONSUMER,
		channel->ring_size,
		(void *)mem_static_map_cached(channel->ring_base),
		sdio_message_wakeup,
		NULL);

	for (;;) {
		dprintf(DEBUG_SPEW, "++ waiting for message on sdio channel\n");
		while (sdio_message_process())
		{
			// eat all available messages
		}
		event_wait(&sdio_message_event);
	}

	return(0);
}

static void
sdio_message_wakeup(void *arg __unused)
{
	event_signal(&sdio_message_event);
}

extern unsigned int do_lookahead;
extern void iopsdio_cacheTransferSDIOData(struct IOPSDIOTargetSDHC *targetSDHC);


static bool
sdio_message_process(void)
{
	uint32_t         messageAddr;

	dprintf(DEBUG_SPEW, "++ handling host message\n");
    
    do_lookahead = 0;
    
	/* look to see if there's an item waiting for us */
	if (qwi_receive_item(sdio_channel, &messageAddr) == -1)
		return(false);
	
	dprintf(DEBUG_SPEW, "++ received sdio message\n");

	/* find the command structure based on the message */
	union IOPSDIOMessage *message = (union IOPSDIOMessage*)mem_static_map_cached(messageAddr);

	/*
	 * Flush any cached item contents we might have lying around - we are guaranteed
	 * that the message size is a multiple of our cacheline size.
	 */
	platform_cache_operation(CACHE_INVALIDATE, 
	    (void *)message, 
	    cache_op_size(sizeof(*message)));


	switch (message->header.opcode) {
	
	case kIOPSDIOOpcodeInit:
	{
		dprintf(DEBUG_INFO, "SDHC @ 0x%X: Init\n",
		    message->targetSDHC.basePhysicalAddr);

		message->header.status = iopsdio_init(&message->targetSDHC, &message->initCmd);
		break;
	}
	case kIOPSDIOOpcodeFree:
	{
		dprintf(DEBUG_INFO, "SDHC @ 0x%X: Free\n",
		    message->targetSDHC.basePhysicalAddr);

		message->header.status = iopsdio_free(&message->targetSDHC, &message->freeCmd);
		break;
	}
	case kIOPSDIOOpcodeReset:
	{
		dprintf(DEBUG_INFO, "SDHC @ 0x%X: Reset 0x%X\n",
		    message->targetSDHC.basePhysicalAddr, message->resetCmd.resetFlags);

		message->header.status = iopsdio_reset(&message->targetSDHC, &message->resetCmd);
		break;
	}
	case kIOPSDIOOpcodeSetBusParam:
	{
/*		dprintf(DEBUG_INFO, "SDHC @ 0x%X: clk = %+d, baseClk = %u, clkRate = %u Hz, width = %u, speed = %d\n",
		    message->targetSDHC.basePhysicalAddr,
			message->setBusParamCmd.clockMode,   message->setBusParamCmd.baseClockRateHz,
			message->setBusParamCmd.clockRateHz, message->setBusParamCmd.busWidth,
			message->setBusParamCmd.busSpeedMode); 
*/
		message->header.status = iopsdio_setBusConfig(&message->targetSDHC, &message->setBusParamCmd);
		break;
	}
	case kIOPSDIOOpcodeSendCommand:
	{
/*		dprintf(DEBUG_INFO, "SDHC @ 0x%X: Send SDIO Cmd: index = %u, arg = 0x%X\n",
		    message->targetSDHC.basePhysicalAddr,
			message->sendSDIOCmd.sdioCmdIndex, message->sendSDIOCmd.sdioCmdArgument);		
*/	
		message->header.status = iopsdio_sendSDIOCmd(&message->targetSDHC, &message->commandCmd);
		break;
	}
	case kIOPSDIOOpcodeTransferData:
	{
/*		dprintf(DEBUG_INFO, "SDHC @ 0x%X: Send SDIO DMA Cmd: index = %u, arg = 0x%X\n",
		    message->targetSDHC.basePhysicalAddr,
			message->sdioDataCmd.sdioCmdIndex, message->sdioDataCmd.sdioCmdArgument);		
*/	
		message->header.status = iopsdio_transferSDIOData(&message->targetSDHC, &message->transferCmd);
		break;
	}
	case kIOPSDIOOpcodePing:
		dprintf(DEBUG_CRITICAL, "SDHC: Ping received, IOKit <--> IOP Connection established\n");
		message->header.status = kIOPSDIOStatusSuccess;
		break;
	default:
		dprintf(DEBUG_CRITICAL, "SDHC @ 0x%X: ERROR: unrecognised sdio opcode 0x%x\n", 
		    message->targetSDHC.basePhysicalAddr, message->header.opcode);
		message->header.status = kIOPSDIOParameterInvalid;
		break;
	}

	dprintf(DEBUG_SPEW, "++ done processing sdio message with status 0x%08x\n", message->header.status);

	platform_cache_operation(CACHE_CLEAN, 
	    (void *)message, 
	    sizeof(union IOPSDIOMessage));

	qwi_send_item(sdio_channel, messageAddr);

    
    {
        if (do_lookahead) {
            iopsdio_cacheTransferSDIOData(&message->targetSDHC);
            do_lookahead = 0;
        }
        
        
    }
    
	dprintf(DEBUG_SPEW, "++ signaled completion of sdio message to host\n");

	return(true);
}
