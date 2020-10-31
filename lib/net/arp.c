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
#include <sys.h>
#include <sys/task.h>
#include <lib/net.h>
#include <lib/net/ethernet.h>
#include <lib/net/ipv4.h>
#include <lib/net/arp.h>
#include <lib/net/xp.h>

arp_cache_t arpcache;

static int arp_transmit(bool reply,char* tomacaddr,uint32_t toip)
{
//generate packet and send it down to the ethernet layer
	mymbuf_t *packet;
	char *data;
	char dstmac[] = {0xff,0xff,0xff,0xff,0xff,0xff};
	uint32_t srcip;
	
	
	packet = mbuf_initialize(28,14,4); //28byte arp, 14 ethernet, 4 ethernet trailer
	data = mbuf_tail(packet,28);
	*(uint16_t *)&data[ARP_OFFSET_HTYPE] = htons(1); //ethernet
	*(uint16_t *)&data[ARP_OFFSET_PTYPE] = htons(0x0800); //ipv4
	data[ARP_OFFSET_HLEN] = 6; //macaddr is 6 bytes
	data[ARP_OFFSET_PLEN] = 4; //ip is 4 bytes
	*(uint16_t *)&data[ARP_OFFSET_OPER] = htons(reply ? 2 : 1);

	xp_getmac((uint8_t*)&data[ARP_OFFSET_SHA]);
       	
	srcip = ipv4_get_ip();
	*(uint32_t *)&data[ARP_OFFSET_SHA+6] = htonl(srcip);

	if(tomacaddr == 0) {
		memcpy(&data[ARP_OFFSET_SHA+10],"\0\0\0\0\0\0",6);
	} else {
		memcpy(&data[ARP_OFFSET_SHA+10],tomacaddr,6);
	}
	toip = htonl(toip);
	memcpy(&data[ARP_OFFSET_SHA+16],&toip,4);

	add_eth_and_transmit(packet,dstmac,ETHERTYPE_ARP);
	mbuf_destroy(packet);
	return 0;	
}
int arp_hash(uint32_t ip)
{
	return ip % ARP_CACHE_SIZE;
}

int arp_cache_add(uint32_t ip, const char *mac)
{
	arp_cache_entry_t *ce = &arpcache.cache[arp_hash(ip)];
	if (ce->ip == 0 || ce->ip == ip)
		goto update;
	while (ce->next) {
		ce = ce->next;
		if(ce->ip == ip)
			goto update;
	}
	// Not in the list, add a new entry
	ce->next = malloc(sizeof(arp_cache_entry_t));
	ce = ce->next;
	ce->next = 0;
update:
	ce->ip = ip;
	memcpy(ce->mac,mac,6);
//	printf("arp: learned %#I at %M\n", &ce->ip, &ce->mac);
	return 0;
}

int arp_cache_get(uint32_t ip,char *mac)
{
	arp_cache_entry_t *ce = &arpcache.cache[arp_hash(ip)];

	if(ip == ce->ip)
		goto found;
	while(ce->next) {
		ce = ce->next;
		if(ip == ce->ip)
			goto found;
	}
	return -1;
found:
//	printf("arp: found %#I at %M\n", &ce->ip, &ce->mac);
	if (mac)
		memcpy(mac,ce->mac,6);
	return 0;
}

void arp_cache_delete(void)
{
	arp_cache_entry_t *ce;
	int i;
       
	for(i=0;i<ARP_CACHE_SIZE;i++) {
		ce = arpcache.cache[i].next;
		while(ce) {
			arp_cache_entry_t *next;
			next = ce->next;
			free(ce);
			ce = next;
		}
	}
}


void arp_dump_table(void)
{
	arp_cache_entry_t *ce;
	int i;
       
	for(i=0;i<ARP_CACHE_SIZE;i++) {
		ce = &arpcache.cache[i];
		while (ce) {
			if (ce->ip != 0)
				printf("ip: %#I\tmac: %M\n", &ce->ip, &ce->mac);
			ce = ce->next;
		}
	}
}


int arp_get_macaddr(uint32_t ip, char *outmac)
{
	if(arp_cache_get(ip,outmac) < 0) {
//		printf("Sending request for 0x%x\n",ip);
		arp_transmit(false,0,ip);
		return -1;
	}
	return 0;
}

/* send arp requests until it is satisfied or timeout */
int arp_fill(uint32_t ip, utime_t timeout)
{
	utime_t t;

	t = system_time();
	do {
		if (arp_cache_get(ip, NULL) >= 0)
			return 0;

		arp_transmit(false, 0, ip);
		task_sleep(100*1000);
	} while (system_time() - t < timeout);

	return -1;
}

static int arp_workloop(char *packet,int offset,int len,void *prevlayerdata)
{
	uint16_t htype,ptype;
	uint8_t hlen,plen;
	uint16_t oper;
	unsigned char *buffer = (unsigned char *)packet+offset;
	uint32_t itoip,ifromip;
	unsigned char *tmp;
	
	htype = ntohs(*(uint16_t*)&buffer[ARP_OFFSET_HTYPE]);
	ptype = ntohs(*(uint16_t*)&buffer[ARP_OFFSET_PTYPE]);
	hlen = buffer[ARP_OFFSET_HLEN];
	plen = buffer[ARP_OFFSET_PLEN];
	oper = ntohs(*(uint16_t*)&buffer[ARP_OFFSET_OPER]);

//	printf("arp: 0x%x 0x%x %d %d op %d\n", htype, ptype, hlen, plen, oper);

	if ((htype != 1) ||		/* ethernet */
	    (ptype != 0x800) ||		/* IPv4 */
	    (hlen != 6) ||		/* 6-byte MAC */
	    (plen != 4)) {		/* 4-byte protocol address */
//		printf("Not ethernet. Bailing\n");
		return -1;
	}
	tmp = &buffer[ARP_OFFSET_SHA+hlen];
	ifromip = tmp[0] << 24 | tmp[1] << 16 | tmp[2] << 8 | tmp[3];
	
	tmp = &buffer[ARP_OFFSET_SHA+(2*hlen)+plen];
	itoip = tmp[0] << 24 | tmp[1] << 16 | tmp[2] << 8 | tmp[3];

//	printf("arp: from %#I to %#I\n", &ifromip, &itoip);

	//someone sent out their address
	arp_cache_add(ifromip,(char *)&buffer[ARP_OFFSET_SHA]);
	
	if (oper == 1) {
		if (itoip == ipv4_get_ip()) {
//			printf("arp: sending reply to %#I\n", &ifromip);
			arp_transmit(true,(char *)&buffer[ARP_OFFSET_SHA],ifromip);
		} else {
//			printf("arp: dest %I not us\n", &itoip);
		}
	}

	return 0;
}

int arp_layer(bool on) 
{
	static int id = -1;
	if(on) {
		if(id == -1) {
			memset(&arpcache,0,sizeof(arpcache));
		       id = registerEtherTypeHandler(ETHERTYPE_ARP,arp_workloop);
		}
	} else {
		if(id >= 0) {
			unregisterEtherTypeHandler(id);
			arp_cache_delete();
			id = -1;
		}
	}
	return 0;
}
