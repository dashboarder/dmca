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
 * Queued Work Item protocol.
 *
 * 
 * QWI provides a mechanism for passing work items between processors using
 * shared memory and a doorbell interrupt. It is a straightforward design using
 * common techniques.
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
 */

/* basic quantum of message passing */
#define QWI_MESSAGE_ALIGNMENT	64

/*
 * In order to avoid racing, the control and payload information must be stored
 * atomically.
 *
 * It is assumed that stores of a uint32_t (when aligned) are atomic, and that
 * user data is 4-byte aligned.
 */

typedef union qwi_workitem {
	uint32_t		qwi_control;
	char			_pad[QWI_MESSAGE_ALIGNMENT];
} *qwi_workitem_t;

#define QWI_POOL_SIZE(_ring_count)	((_ring_count) * sizeof(union qwi_workitem))

/* ownership/role relation */
#define QWI_OWNER_CONSUMER	0
#define QWI_OWNER_PRODUCER	1

/* ownership mask */
#define QWI_OWNER_MASK		1

/* bit 1 in qwi_control is reserved for later use */

/* address mask */
#define QWI_ADDRESS_MASK	(~(uint32_t)0x3)

/* construct the qwi_control value */
#define QWI_CONTROL(_address, _owner) \
	((uint32_t)(_address) | (_owner))

/* fetch the address from a workitem */
#define QWI_ITEM_ADDRESS(_ip) \
	(((_ip)->qwi_control & QWI_ADDRESS_MASK))

/* test ownership of a workitem */
#define QWI_ITEM_TEST_OWNER(_ip, _who) \
	(((_ip)->qwi_control & QWI_OWNER_MASK) == (uint32_t)(_who))

/* en/decode a value so that it can be passed as though it were an item address */
#define QWI_ENCODE_ORDINAL(_n)	((uint32_t)(_n) << 2)
#define QWI_DECODE_ORDINAL(_n)	((uint32_t)(_n) >> 2)
