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
#ifndef __ETHERNET_H__
#define __ETHERNET_H__

#include <sys/types.h>
#include <sys/callout.h>

#include "callbacks.h"
#include "xp.h"

__BEGIN_DECLS

#define ETHERTYPE_ARP 0x806
#define ETHERTYPE_IPV4 0x800

#define ETHERNET_OFFSET 14

int registerEtherTypeHandler(int type,callback_handler cb);
int unregisterEtherTypeHandler(int id);

int add_eth_and_transmit(mymbuf_t *,char *dstmac,int ethertype);

void ethernet_init(void);
void ethernet_uninit(void);
void ethernet_workloop(struct callout *c, void *param);
void ethernet_input(mymbuf_t *inputbuf);

__END_DECLS

#endif
