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
#include <debug.h>
#include <lib/heap.h>
#include <lib/net/mbuf.h>

#ifndef CPU_CACHELINE_SIZE
# define CPU_CACHELINE_SIZE	32
#endif

mymbuf_t * mbuf_initialize(int packetsize,int header,int tail)
{
	mymbuf_t *p = calloc(1, sizeof(mymbuf_t));

	p->primarydata = memalign(packetsize + header+tail, CPU_CACHELINE_SIZE);
	p->offset = header;
	p->allocated_length = packetsize+header+tail;
	p->current_length = 0;
	return p;
}

char *mbuf_head(mymbuf_t *buffer,int len)
{
//	mbufentry_t **curr = &buffer->first;
//	mbufentry_t *last;
	
	if(len <= buffer->offset && buffer->first == 0) {
		char *ret;
		//We have space in our buffer, let's just decrease offset and copy the data in
		ret = (buffer->primarydata + buffer->offset)-len;
		buffer->offset -= len;
		buffer->current_length += len;
		return ret;
	} else {
		printf("Request for %d bytes, only %d available\n",len,buffer->offset);
	/*
		while((*curr)->next) {
			curr = &(*curr)->next;
		}
 		// first element
		if(*curr == 0) {
			buffer->first = malloc(sizeof(mbufentry_t));
			last = buffer->first;
		} else {
			last = (*curr)->next = malloc(sizeof(mbufentry_t));
		}
		last->len = len;
		last->data = malloc(len);
		last->flag = ENTRY_FLAG_ALLOCATED;
		last->next = 0;
		memcpy(last->data,indata,len);
		buffer->length += len;
*/
	}
	return 0;
}

char *mbuf_tail(mymbuf_t *buffer,int len)
{
	if(buffer->offset+buffer->current_length+len <= buffer->allocated_length) {
			char *ret;
			ret = buffer->primarydata+buffer->offset + buffer->current_length;
			buffer->current_length += len;
			return ret;
	} else {
		return 0;
	}
}

int mbuf_copy(mymbuf_t *dest, const mymbuf_t *src)
{
	if (dest->allocated_length < src->current_length)
		return -1;

	dest->offset = 0;
	dest->current_length = src->current_length;
	memcpy(dest->primarydata, src->primarydata + src->offset, dest->current_length);

	return 0;
}

int mbuf_getlength(mymbuf_t *buffer)
{
	return buffer->current_length;
}
int mbuf_getallocatedlength(mymbuf_t *buffer)
{
	return buffer->allocated_length;
}

void mbuf_setlength(mymbuf_t *buffer,int length)
{
	buffer->current_length = length;
}
void mbuf_setoffset(mymbuf_t *buffer,int offset)
{
	buffer->offset = offset;	
}

char *mbuf_getlinear(mymbuf_t *buf)
{	
	if(buf->first) {
		printf("getlinjear: not supported\n");
		return 0;
	}
	return buf->primarydata+buf->offset;

}
void mbuf_destroy(mymbuf_t *buff)
{
	if(buff->first) {
		printf("Not supported\n");
		return;
	}
	free(buff->primarydata);
	free(buff);
}
