/*
 * Copyright (C) 2006 Apple Computer, Inc. All rights reserved.
 *
 * This document is the property of Apple Computer, Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Computer, Inc.
 */
#ifndef __LIB_CBUF_H
#define __LIB_CBUF_H

#include <sys/types.h>
#include <sys/task.h>

__BEGIN_DECLS

/* circular buffer implementation */
struct cbuf {
	void *buf;
	unsigned int head;
	unsigned int tail;
	size_t has;
	size_t len;
	uint32_t len_mask;

	/* optional event for locking */
	struct task_event *event;
};

/* create a circular buffer, len must be power of 2 */
struct cbuf *cbuf_create(size_t len, struct task_event *event); // event is optional
void cbuf_destroy(struct cbuf *);

/* read/write unstructured data */
ssize_t cbuf_write_raw(struct cbuf *, const void *data, size_t len);
ssize_t cbuf_read_raw(struct cbuf *, void *data, size_t len);

/* read/write individual chars */
ssize_t cbuf_write_char(struct cbuf *, char c);
ssize_t cbuf_read_char(struct cbuf *, char *c);

/* read/write strings */
ssize_t cbuf_write_string(struct cbuf *, const char *str);
ssize_t cbuf_read_string(struct cbuf *, char *str, size_t maxlen);

__END_DECLS

#endif

