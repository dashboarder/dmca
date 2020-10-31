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
#ifndef __CALLBACKS_H__
#define __CALLBACKS_H__

#include <sys/types.h>

__BEGIN_DECLS

typedef int (*callback_handler)(char *data,int offset,int len,void *prevlayerdata);

struct listentry;

typedef struct listentry {
	int cb_id;
	int port; //port/service/etc
	callback_handler callback;
	struct listentry *next;
} listentry_t;

typedef struct callbacks {
	listentry_t *first;
}callbacks_t;


int registerCallback(callbacks_t *,int port,callback_handler cb);
int unregisterCallback(callbacks_t *cbs,int cb_id);
callback_handler find_callback(callbacks_t *cbs,int port);

__END_DECLS

#endif

