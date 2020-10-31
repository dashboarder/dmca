/*
 * Copyright (C) 2007, 2011, 2014 Apple Inc. All rights reserved.
 * Copyright (C) 2006 Apple Computer, Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */
#include <debug.h>
#include <sys.h>
#include <lib/heap.h>
#include <lib/cbuf.h>
#include <lib/libiboot.h>

#define INC_HEAD(cbuf) ((cbuf)->head = ((cbuf)->head + 1) & (cbuf)->len_mask)
#define INC_TAIL(cbuf) ((cbuf)->tail = ((cbuf)->tail + 1) & (cbuf)->len_mask)

struct cbuf *cbuf_create(size_t len, struct task_event *event)
{
	struct cbuf *buf;

	if (!is_pow2(len))
		return NULL;

	buf = malloc(sizeof(struct cbuf));

	buf->buf = malloc(len);

	buf->head = 0;
	buf->tail = 0;
	buf->has = 0;
	buf->len = len;
	buf->len_mask = len - 1;

	buf->event = event;
	if (buf->event)
		event_init(buf->event, 0, false);

	return buf;
}

void cbuf_destroy(struct cbuf *buf)
{
	if (buf == 0) return;

	free(buf->buf);
	free(buf);
}

ssize_t cbuf_write_char(struct cbuf *buf, char c)
{
	ssize_t ret;

	/* if cbuf not valid, drop the character */
	if (buf == 0) return 1;

	enter_critical_section();
	
	if (buf->has == buf->len) {
		ret = 0;
		goto out;
	}

	((char *)buf->buf)[buf->head] = c;
	INC_HEAD(buf);
	buf->has++;
	ret = 1;

	if (buf->event)
		event_signal(buf->event);

out:
	exit_critical_section();
	return ret;
}

ssize_t cbuf_read_char(struct cbuf *buf, char *c)
{
	ssize_t ret;

	/* if cbuf not valid, return nothing */
	if (buf == 0) return 0;

	enter_critical_section();
	
	if (buf->has == 0) {
		ret = 0;
		goto out;
	}

	*c = ((char *)buf->buf)[buf->tail];
	INC_TAIL(buf);
	buf->has--;
	ret = 1;

out:
	if (buf->event && buf->has == 0)
		event_unsignal(buf->event);

	exit_critical_section();
	return ret;
}
