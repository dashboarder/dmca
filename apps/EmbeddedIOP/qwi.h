/*
 * Copyright (C) 2007-2010 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Computer, Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Computer, Inc.
 */
/*
 * Queued Work Item protocol library.
 *
 * 
 * QWI provides a mechanism for passing work items between processors using
 * shared memory and a doorbell interrupt. It is a straightforward design using
 * common techniques.
 *
 * This library supports multiple instances of the QWI protocol (channels) 
 * operating in either directioion with a single remote endpoint.  There is
 * assumed to be a single doorbell available at either end.
 * 
 * A QWI channel comprises a fixed-size ring of items. The ring should be sized
 * to account for doorbell latency and to permit coalescing of doorbell
 * interrupts; it need not be large if the work item throughput is low.
 * 
 * Each channel moves work items in one direction and responses in the other.
 * For command/response oriented communication, one channel is thus sufficient.
 * For asynchronous interactions, a channel is required in each direction. The
 * parties at each end of a channel are referred to as the producer and the
 * consumer respectively.
 * 
 * Work items in the ring are owned by either the producer or the consumer.
 * Ownership is tracked by a single bit, which can only be set by the current
 * owner. Ownership changes always move forwards in the ring, they are never made
 * out of order. For example, this ring of eight work items has three currently
 * owned by the consumer:
 * 
 *   0 1 2 3 4 5 6 7
 *   C C C P P P P P
 * 
 * Work item 0 is always returned to the producer before item 1 is returned.
 * 
 * The payload of a work item is a client-supplied pointer. QWI does not manage
 * coherency for the payload; this is a client responsibility.
 * 
 * When attention is signalled via the doorbell, the doorbell must be
 * acknowledged and re-armed before any check is made of channels in order to
 * prevent items being lost and the queue potentially stalling.
 * 
 *
 * The following parameters must be defined:
 *
 * QWI_MAX_CHANNELS
 *
 *	Establishes the size of the channel table.
 *
 */

#include <stdint.h>

/* must match the protocol definition */
#define QWI_MESSAGE_ALIGNMENT	64

/* must match QWI_OWNER_* values in the protocol definition */
#define QWI_ROLE_CONSUMER	0
#define QWI_ROLE_PRODUCER	1

typedef void (* qwi_channel_hook)(void *user_data);

/******************************************************************************
 * API 
 */

extern int qwi_instantiate_channel(
	const char *name,
	int role,
	int ring_size,
	void *item_pool,
	qwi_channel_hook hook,
	void *user_data);
/*
 * Instantiates a new channel endpoint and returns a corresponding
 * channel handle.  Endpoints must be co-ordinated between the
 * processors; there is no setup channel.
 * 
 * 	role
 * 		One of QWI_ROLE_PRODUCER or QWI_ROLE_CONSUMER depending
 * 		on whether the endpoint is the producer or consumer.
 * 		
 * 	ring_size
 * 		The number of entries in the ring.
 * 		
 * 	item_pool
 * 		The buffer allocated for ring items.  This can be sized by the
 * 		client using the QWI_POOOL_SIZE() macro.  The pool must be
 *		in uncached memory.
 *
 * 	hook
 * 	user_data
 * 		If a hook is supplied for the channel, when the doorbell is
 * 		rung the hook will be called with user_data as an argument.
 */

extern int qwi_send_item(int channel_handle, uint32_t message);
/*
 * Sends a work item to the other end of the channel.
 * 
 * Any memory referenced by item_data should be in a state where it may
 * be instantly consumed by the other processor.
 * 
 * Returns -1 if there are no free work items available.
 * 
 * 	channel_handle
 * 		A handle returned from qwi_instantiate_channel().
 * 
 * 	message
 * 		The message to be passed to the other end.
 */

extern int qwi_receive_item(int channel_handle, uint32_t *message);
/*
 * Attempts to receive a work item.  If successful, returns the message as
 * supplied by the sender.
 * 
 * Returns -1 if there are no waiting items to be received.
 * 
 * 	channel_handle
 * 		A handle returned from qwi_instantiate_channel().
 * 
 * 	message
 * 		Local pointer storage for the value passed by the other end.
 */

extern int qwi_peek_item(int channel_handle, uint32_t *message);
/*
 * Attempts to peek a work item.  If successful, returns the message as
 * supplied by the sender.  Does not remove the item from the queue.
 * 
 * Returns -1 if there are no waiting items to be received.
 * 
 * 	channel_handle
 * 		A handle returned from qwi_instantiate_channel().
 * 
 * 	message
 * 		Local pointer storage for the value passed by the other end.
 */

extern int qwi_next_send_index(int channel_handle);
/*
 * Returns the ring index for the next item to be sent on the given
 * channel.  If the index returned is -1 there are no free items to send.
 * For protocols where items are sent and replied in strict order, the
 * index can be used by the client to avoid tracking work item buffer
 * ownership separately.
 * 
 * Note that the client is required to co-ordinate between calls to
 * qwi_next_send_index() and qwi_send_item() to avoid races.
 * 
 * 	channel_handle
 * 		A handle returned from qwi_instantiate_channel()
 */

/******************************************************************************
 * Dependencies
 */

#include <platform.h>
#include <sys/task.h>

extern void qwi_doorbell(void);
/*
 * Must be called when the local doorbell is rung.
 */

/*extern void qwi_cache_store_line(void *address);*/
static inline void qwi_cache_store_line(void *address)
{
	address = (void *)((uint32_t)address & ~(QWI_MESSAGE_ALIGNMENT - 1));
	platform_cache_operation(CACHE_CLEAN, address, QWI_MESSAGE_ALIGNMENT);
}
/*
 * Causes the cache line containing (address) to be stored to coherent memory.
 */

/*extern void qwi_cache_invalidate_line(void *address);*/
static inline void qwi_cache_invalidate_line(void *address)
{
	address = (void *)((uint32_t)address & ~(QWI_MESSAGE_ALIGNMENT - 1));
	platform_cache_operation(CACHE_INVALIDATE, address, QWI_MESSAGE_ALIGNMENT);
}
/*
 * Causes the cache line containing (address) to be invalidated, forcing the
 * next access to read from coherent memory.
 */

/*extern void qwi_ring_doorbell(void);*/
static inline void qwi_ring_doorbell(void)
{
	platform_ring_host_doorbell();	
}
/*
 * Rings the other processor's doorbell.
 */

/*extern void qwi_enter_critical(void);*/
/*extern void qwi_exit_critical(void);*/
static inline void qwi_enter_critical(void)
{
	enter_critical_section();
}
static inline void qwi_exit_critical(void)
{
	exit_critical_section();
}
/*
 * These calls are required to avoid re-entry of critical sections of the
 * code.  Note that the qwi_cache_*() functions must be safely callable
 * from within the critical region.
 */
