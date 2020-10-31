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
#include <stdio.h>
#include <stdlib.h>
#include <lib/net/callbacks.h>

int registerCallback(callbacks_t *cbs,int port,callback_handler cb)
{
	listentry_t *cur = cbs->first;
	int i = 1;
	
	while(cur && cur->next) {
		cur  = cur->next;
		i++;
	}
	if(!cur) { //no elems
		cur = cbs->first = malloc(sizeof(listentry_t));
		i = 0; //first.
	} else {
		cur->next = malloc(sizeof(listentry_t));
		cur = cur->next;
	}
	cur->next = 0;
	cur->callback = cb;
	cur->port = port;
	cur->cb_id = i;
	return cur->cb_id;
}
int unregisterCallback(callbacks_t *cbs,int cb_id)
{
	listentry_t *cur = cbs->first;
	listentry_t *prev = 0;
	
	while(cur) {
		if(cur->cb_id == cb_id) {
			if(prev == 0) {
				cbs->first = cur->next;
			} else {
				prev->next = cur->next;
			}	
			free(cur);
			return 0;
		}
		prev = cur;
		cur = cur->next;
	}
return -1;
}

callback_handler find_callback(callbacks_t *cbs,int port) 
{
	listentry_t *cur = cbs->first;
	
	while(cur) {
		if(cur->port == port) {
			return cur->callback;
		}
		cur = cur->next;
	}
return 0;


}
