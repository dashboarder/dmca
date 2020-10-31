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
#ifndef __ARP_H__
#define __ARP_H__

#include <sys/types.h>

__BEGIN_DECLS

#define ARP_OFFSET_HTYPE 0
#define ARP_OFFSET_PTYPE 2
#define ARP_OFFSET_HLEN  4
#define ARP_OFFSET_PLEN  5
#define ARP_OFFSET_OPER  6
#define ARP_OFFSET_SHA   8

int arp_layer(bool on);
int arp_get_macaddr(uint32_t ip,char *mac);
int arp_fill(uint32_t ip, utime_t timeout); /* send arp requests until it is satisfied or timeout */
void arp_dump_table(void);

#define ARP_CACHE_SIZE 32

typedef struct arp_cache_entry {
	uint32_t ip;
	char mac[6];
	struct arp_cache_entry *next;
} arp_cache_entry_t;

typedef struct arp_cache {
	arp_cache_entry_t cache[ARP_CACHE_SIZE];
} arp_cache_t;

__END_DECLS

#endif
