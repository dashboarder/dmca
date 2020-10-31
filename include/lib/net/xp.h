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
#ifndef __XP_H__
#define __XP_H__

#include <sys/types.h>
#include <stdint.h>
#include <lib/net/mbuf.h>

__BEGIN_DECLS

//Card specific instructions.

int xp_initialize(void);
int xp_getmac(uint8_t *addr);
void xp_packet_input(void *ethptr, mymbuf_t *inputbuf);
//mymbuf_t * xp_get_packet(mymbuf_t *);
int xp_transmit_packet(mymbuf_t *buffer);
#define htons(A) ( (((A) & 0xff00) >> 8) | \
                    ((A) & 0x00ff) << 8)
#define htonl(A) ( (((A) & 0xff000000) >> 24) | \
                   (((A) & 0x00ff0000) >> 8) | \
                   (((A) & 0x0000ff00) << 8) | \
		   (((A) & 0x000000ff) << 24)) 
#define ntohs     htons
#define ntohl     htonl

__END_DECLS

#endif
