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
#ifndef __LIB_NET_MBUF_H
#define __LIB_NET_MBUF_H

#include <sys/types.h>

__BEGIN_DECLS

#define USE_BUILTIN_MBUF 1
#if defined(USE_BUILTIN_MBUF) && USE_BUILTIN_MBUF
struct mbufentry;

typedef struct mbufentry {
	int flag;
	int len;
	char *data;
	struct mbufentry *next;
} mbufentry_t;

typedef struct mymbuf {
	mbufentry_t *first;	
	int allocated_length;
	int current_length;
	char *primarydata;
	int offset;
}mymbuf_t;
#else
#error "Unsupported platform"
#endif

mymbuf_t *	mbuf_initialize(int packetsize,int header,int trailer);
void 		mbuf_destroy(mymbuf_t *data);
char* 		mbuf_head(mymbuf_t *buffer, int len);
char* 		mbuf_tail(mymbuf_t *buffer, int len);
int 		mbuf_getlength(mymbuf_t *buffer);
char *		mbuf_getlinear(mymbuf_t *buffer);
void		mbuf_setlength(mymbuf_t *buffer,int len);
void		mbuf_setoffset(mymbuf_t *buffer,int offset);
int			mbuf_getallocatedlength(mymbuf_t *buffer);
int			mbuf_copy(mymbuf_t *dest, const mymbuf_t *src);

__END_DECLS

#endif

