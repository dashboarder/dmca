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

#include <debug.h>
#include <sys/menu.h>

#include "qwi_protocol.h"
#include "qwi.h"

#ifndef QWI_MAX_CHANNELS
# error Must define QWI_MAX_CHANNELS
#endif

typedef struct qwi_channel {
	const char		*qwc_name;
	int			qwc_role;
	int			qwc_ring_size;
	int			qwc_next_in;
	int			qwc_next_out;
	qwi_workitem_t		qwc_work_items;
	qwi_channel_hook	qwc_user_hook;
	void			*qwc_user_data;

#if DEBUG_BUILD
# define QWC_STAT_INCR(_cp, _name)	do {(_cp)->qwc_stat_##_name++; } while(0)
	int			qwc_stat_messages;
#else
# define QWC_STAT_INCR(_cp, _name)	do { } while(0)
#endif
} *qwi_channel_t;

/* fetch pointer to workitem */
#define QWC_WORKITEM(_cp, _index) \
	((_cp)->qwc_work_items + (_index))

/* compute next workitem index */
#define QWC_ADVANCE(_cp, _index) \
	((((_index) + 1) >= (_cp)->qwc_ring_size) ? 0 : ((_index) + 1))

static int			qwi_channel_count;
static struct qwi_channel	qwi_channels[QWI_MAX_CHANNELS];

/******************************************************************************
 * qwi_instantiate_channel
 *
 * Create a new channel endpoint using workitems from item_pool.
 */
int
qwi_instantiate_channel(
	const char *name,
	int role,
	int ring_size,
	void *item_pool,
	qwi_channel_hook hook,
	void *user_data)
{
	qwi_workitem_t	ip;
	qwi_channel_t	cp;
	int		i, handle;

	ASSERT((role == QWI_ROLE_PRODUCER) || (role == QWI_ROLE_CONSUMER));
	ASSERT(qwi_channel_count < QWI_MAX_CHANNELS);
	ASSERT(item_pool != NULL);

	qwi_enter_critical();
	
	cp = &qwi_channels[qwi_channel_count];

	/* init the channel */
	cp->qwc_name = name;
	cp->qwc_role = role;
	cp->qwc_ring_size = ring_size;
	cp->qwc_work_items = item_pool;
	cp->qwc_user_hook = hook;
	cp->qwc_user_data = user_data;
	
	handle = qwi_channel_count++;
	
	/* ring initially belongs to the producer, so claim it if we are */
	if (role == QWI_ROLE_PRODUCER) {
		for (i = 0; i < ring_size; i++) {
			ip = QWC_WORKITEM(cp, i);
			ip->qwi_control = QWI_OWNER_PRODUCER;
			qwi_cache_store_line(ip);
		}
		cp->qwc_next_in = ~0;
		cp->qwc_next_out = 0;
		dprintf(DEBUG_INFO, "allocated producer %d with %d slots, item size %zu\n",
		    handle, ring_size, sizeof(*ip));
	} else {
		cp->qwc_next_in = 0;
		cp->qwc_next_out = ~0;
		dprintf(DEBUG_INFO, "allocated consumer %d with %d slots, item size %zu\n",
		    handle, ring_size, sizeof(*ip));
	}

	/* success */
	qwi_exit_critical();
	
	return(handle);
}

/******************************************************************************
 * qwi_send_item
 *
 * Send the next work item to the other end of the channel.
 */
int
qwi_send_item(int channel_handle, uint32_t message)
{
	qwi_workitem_t	ip;
	qwi_channel_t	cp;

	ASSERT((channel_handle >= 0) && (channel_handle < qwi_channel_count));

	/* verify item data is aligned correctly */
	ASSERT(message == (message & QWI_ADDRESS_MASK));

	qwi_enter_critical();
	cp = &qwi_channels[channel_handle];

	/* if we have no items free to go, we can't do this */
	if (cp->qwc_next_out == ~0)
		goto fail;

	/* find the next item for this channel */
	ip = QWC_WORKITEM(cp, cp->qwc_next_out);

	/* set data and assign ownership to the other end */
	ip->qwi_control = QWI_CONTROL(message, cp->qwc_role ^ QWI_OWNER_MASK);

	/* if we didn't have anything outstanding before, we do now */
	if (cp->qwc_next_in == ~0)
		cp->qwc_next_in = cp->qwc_next_out;

	/* find the next item we can send */
	cp->qwc_next_out = QWC_ADVANCE(cp, cp->qwc_next_out);

	/* if we have sent the last item, we have nothing free to go */
	if (cp->qwc_next_in == cp->qwc_next_out)
		cp->qwc_next_out = ~0;

	/* ensure the item is visible to the consumer */
	qwi_cache_store_line(ip);

	QWC_STAT_INCR(cp, messages);
	qwi_exit_critical();

	/* ring the doorbell */
	qwi_ring_doorbell();

	return(0);
fail:
	qwi_exit_critical();
	return(-1);
}

/******************************************************************************
 * qwi_receive_item
 *
 * Receive the next incoming item from the other end of the channel.
 */
int
qwi_receive_item(int channel_handle, uint32_t *message)
{
	qwi_workitem_t	ip;
	qwi_channel_t	cp;

	ASSERT((channel_handle >= 0) && (channel_handle < qwi_channel_count));
	ASSERT(message != NULL);

	qwi_enter_critical();
	
	cp = &qwi_channels[channel_handle];	

	/* if we own the entire ring, there's nothing waiting for us */
	if (cp->qwc_next_in == ~0)
		goto fail;
	
	/* find the next item for this channel */
	ip = QWC_WORKITEM(cp, cp->qwc_next_in);

	/* make sure that we own it */
	qwi_cache_invalidate_line(ip);
	if (!QWI_ITEM_TEST_OWNER(ip, cp->qwc_role))
		goto fail;

	/* we do, so pass the reply pointer back */
	*message = QWI_ITEM_ADDRESS(ip);

	/* if we didn't have anything free to send with, we do now */
	if (cp->qwc_next_out == ~0)
		cp->qwc_next_out = cp->qwc_next_in;

	/* find the next item coming in */
	cp->qwc_next_in = QWC_ADVANCE(cp, cp->qwc_next_in);
	
	/* if we've received the last item, we own the entire ring again */
	if (cp->qwc_next_out == cp->qwc_next_in)
		cp->qwc_next_in = ~0;

	QWC_STAT_INCR(cp, messages);
	qwi_exit_critical();	

	return(0);
fail:
	qwi_exit_critical();
	return(-1);
}


/******************************************************************************
 * qwi_peek_item
 *
 * Peek at the next item from the other end of the channel.
 */
int
qwi_peek_item(int channel_handle, uint32_t *message)
{
	qwi_workitem_t	ip;
	qwi_channel_t	cp;

	ASSERT((channel_handle >= 0) && (channel_handle < qwi_channel_count));

	qwi_enter_critical();
	
	cp = &qwi_channels[channel_handle];	

	/* if we own the entire ring, there's nothing waiting for us */
	if (cp->qwc_next_in == ~0)
		goto fail;
	
	/* find the next item for this channel */
	ip = QWC_WORKITEM(cp, cp->qwc_next_in);

	/* make sure that we own it */
	qwi_cache_invalidate_line(ip);
	if (!QWI_ITEM_TEST_OWNER(ip, cp->qwc_role))
		goto fail;

	/* we do, so pass the reply pointer back */
	if (message)
		*message = QWI_ITEM_ADDRESS(ip);

	qwi_exit_critical();	
	return(0);
fail:
	qwi_exit_critical();
	return(-1);
}

/******************************************************************************
 * qwi_next_send_index
 *
 * Return the ring index of the next item that will be sent for the given
 * channel.
 *
 * For protocols that operate strictly in-order this allows the client to avoid
 * separately managing their command buffers.
 */
int
qwi_next_send_index(int channel_handle)
{
	qwi_channel_t	cp;
	int		result;

	ASSERT((channel_handle >= 0) && (channel_handle < qwi_channel_count));

	qwi_enter_critical();
	cp = &qwi_channels[channel_handle];

	result = (cp->qwc_next_out == ~0) ? -1 : cp->qwc_next_out;
	
	qwi_exit_critical();
	return(result);
}

/******************************************************************************
 * qwi_doorbell
 *
 * Called when the doorbell is rung by the other end.
 */
void
qwi_doorbell(void)
{
	qwi_channel_t	cp;
	int		i, count;
	
	qwi_enter_critical();
	count = qwi_channel_count;
	qwi_exit_critical();

	/*
	 * Iterate the channel set, and for channels that have user hooks
	 * and which have at least one pending item, call the hook.
	 */
	for (i = 0; i < count; i++) {
		cp = &qwi_channels[i];
		if ((cp->qwc_user_hook) && (0 == qwi_peek_item(i, NULL)))
			cp->qwc_user_hook(cp->qwc_user_data);
	}
}

#if WITH_MENU

static int
qwi_dump(int argc __unused, struct cmd_arg *args __unused)
{
	qwi_workitem_t	ip;
	qwi_channel_t	cp;
	int		chn, i;

	for (chn = 0; chn < qwi_channel_count; chn++) {
		cp = &qwi_channels[chn];
		printf("%02d(%s): %s, 0x%x slots, next in 0x%x, next out 0x%x, ring at %p\n",
		    chn,
		    cp->qwc_name,
		    (cp->qwc_role == QWI_ROLE_PRODUCER) ? "producer" : "consumer",
		    cp->qwc_ring_size,
		    cp->qwc_next_in,
		    cp->qwc_next_out,
		    cp->qwc_work_items);
#if DEBUG_BUILD
		printf("    %d messages passed\n", cp->qwc_stat_messages);
#endif
		for (i = 0; i < cp->qwc_ring_size; i++) {
			ip = QWC_WORKITEM(cp, i);
			printf("    %02d@%p: 0x%08x %c %c\n",
			    i, ip,
			    ip->qwi_control & QWI_ADDRESS_MASK,
			    ((ip->qwi_control & QWI_OWNER_MASK) == QWI_ROLE_PRODUCER) ? 'P' : 'C',
			    (i == cp->qwc_next_in) ? 'i' : (i == cp->qwc_next_out) ? 'o' : ' ');
		}
	}
	return(0);
}

MENU_COMMAND_DEBUG(qwi, qwi_dump, "print QWI channels", NULL);
#endif
