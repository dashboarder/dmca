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
#ifndef __UDP_H__
#define __UDP_H__

#include <sys/types.h>

__BEGIN_DECLS

int transmit_udp(mymbuf_t *data, uint32_t toip,int toport, int fromport);
int unregisterUDPHandler(int id);
int registerUDPHandler(int port,callback_handler cb); 
int udp_layer(bool on);


typedef struct udpinfo {
	uint32_t srcip;
	uint16_t srcport;
	uint16_t length;
} udpinfo_t;

__END_DECLS

#endif
