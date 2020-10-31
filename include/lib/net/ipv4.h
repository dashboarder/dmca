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
#ifndef __IPV4_H__
#define __IPV4_H__

#include <sys/types.h>
#include "callbacks.h"
#include "xp.h"

__BEGIN_DECLS

#define IPV4_ICMP 1
#define IPV4_TCP 6
#define IPV4_UDP 17

int ipv4_layer(bool on);
int registerIPV4Handler(int protocol,callback_handler handler);
int unregisterIPV4Handler(int id);

uint32_t ipv4_get_ip();
void ipv4_set_ip(uint32_t ip);

int transmit_and_add_ipv4(mymbuf_t *mbuf,uint32_t dest,int protocol);

__END_DECLS

#endif
