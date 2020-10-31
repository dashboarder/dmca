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
#include <lib/net.h>
#include <lib/net/xp.h>
#include <lib/net/ethernet.h>
#include <lib/net/callbacks.h>
#include <lib/net/arp.h>
#include <lib/net/ipv4.h>

static callbacks_t ipcb;
static uint32_t sCurrentIP;


static int calculate_ipv4_checksum(uint16_t *data, int len)
{
	uint32_t sum = 0;
	uint16_t res;
	for(int i=0;i<len;i++) {
		sum += (data[i]);
	}
	sum = (sum &0xffff) + (sum >> 16);
	res = ~sum;
	return res;
}
int transmit_and_add_ipv4(mymbuf_t *mbuf,uint32_t dest,int protocol)
{
	int datalen;
	uint16_t *header;
	char outmac[6];
	uint32_t source;
	
	datalen = mbuf_getlength(mbuf);
	if(datalen > 1480) {
		dprintf(DEBUG_INFO, "ipv4: fragmented ip packages not supported\n");
		return -1;
	}
	header = (uint16_t *)mbuf_head(mbuf,20);
	if(!header) {
		dprintf(DEBUG_INFO, "ipv4: No room to prepend the ipv4 header\n");
		return -1;
	}
	header[0] = htons((0x4 << 12) | (5 << 8) );
	header[1] = htons(datalen+20); 
	header[2] = htons(0xBEEF);
	header[3] = 0;
	header[4] = htons((0xff << 8) | ((protocol & 0xff )));
	header[5] = 0; //cksum
	
	source = ipv4_get_ip();
	char *ip = (char*)&header[6];
	ip[0] = (source & 0xff000000) >> 24;
	ip[1] = (source & 0xff0000) >> 16;
	ip[2] = (source & 0xff00) >> 8;
	ip[3] = (source & 0xff);
	
	ip = (char*)&header[8];
	ip[0] = (dest & 0xff000000) >> 24;
	ip[1] = (dest & 0xff0000) >> 16;
	ip[2] = (dest & 0xff00) >> 8;
	ip[3] = (dest & 0xff);
	
	header[5] = (calculate_ipv4_checksum(header,10));
	if(arp_get_macaddr(dest,outmac) < 0) {
		dprintf(DEBUG_SPEW, "ipv4: No mac address available for 0x%x, dropping and requesting mac for that host.\n",dest);
		return -1;	
	}
	add_eth_and_transmit(mbuf,outmac,ETHERTYPE_IPV4);

	return 0;
}

void ipv4_set_ip(uint32_t ip) 
{
	char ipstr[32];
	ip2str((uint8_t *)&ip, ipstr, 32);
	dprintf(DEBUG_INFO, "Setting ip to %s\n", ipstr);
	sCurrentIP = ip;
}
//Returns our ip in host order.
uint32_t ipv4_get_ip() 
{
	return sCurrentIP;
}

//Called by the ethernet layer when a IPV4 packet was received.
static int ipv4_workloop(char *data,int offset, int len,void *unused) 
{
	int headerlen;
	int totallen;
	int ttl,flags,cksum,protocol;
//	char sourceip[16],destip[16];
	char *buf = data+offset;
	callback_handler cb;
	uint32_t srcip;
//	utime_t perftime = system_time();
	
	if(((buf[0] & 0xf0)>>4) != 0x4) {
		printf("not ipv4 (0x%x)\n",buf[0]&0xf0);
		return -1;
	}
	headerlen = buf[0] & 0xf;
	totallen = ntohs(*(uint16_t*)&buf[2]);
	flags = buf[6] & 0x7;
	if(flags) {
		dprintf(DEBUG_SPEW, "ipv4: Fragmented/Packets with flags not supported (0x%x)\n",flags);
		return -1;
	}
	ttl = buf[8];
	protocol = buf[9];
	cksum = ntohs(*(uint16_t*)&buf[9]);
//	ipv42str(&buf[12],sourceip);
//	ipv42str(&buf[16],destip);
//	printf("IPV4 %s -> %s, protocol: %d( headerlen: %d, totallen: %d)\n",sourceip,destip,protocol,headerlen,totallen);
	cb = find_callback(&ipcb,protocol);
	if(!cb) {
//		dprintf(DEBUG_SPEW, "ipv4: Unsupported protocol(0x%x)\n",protocol);
		return -1;
	}
	srcip = buf[12] << 24 | buf[13] << 16 | buf[14] << 8 | buf[15];
	cb(data,offset+(headerlen*sizeof(4)),len,(void*)srcip);
//	printf("ipv4 took %ld usec\n", system_time() - perftime);

	return 0;
}

int ipv4_layer(bool on) 
{
	static int id = -1;
	if(on) {
		if(id == -1) {
		       id = registerEtherTypeHandler(ETHERTYPE_IPV4,ipv4_workloop);
		}
	} else {
		if(id >= 0) {
			unregisterEtherTypeHandler(id);
			id = -1;
		}
	}
	return 0;
}

int registerIPV4Handler(int protocol,callback_handler handler)
{
	return registerCallback(&ipcb,protocol,handler);
}

int unregisterIPV4Handler(int id)
{
	return unregisterCallback(&ipcb,id);
}
