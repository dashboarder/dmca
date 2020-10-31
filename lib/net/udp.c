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
#include <lib/net/callbacks.h>
#include <lib/net/ipv4.h>
#include <lib/net/xp.h>
#include <lib/net/udp.h>

static callbacks_t udpcb;

static int udp_workloop(char *data, int offset,int len,void* srcip)
{
	uint16_t sourceport,destport;
	uint16_t length,checksum;
	char *buffer = data+offset;
	callback_handler cb;
	udpinfo_t replyinfo;
//	utime_t perftime = system_time();

	sourceport = ntohs(*(uint16_t *)&buffer[0]);
	destport = ntohs(*(uint16_t *)&buffer[2]);
	
	length = ntohs(*(uint16_t *)&buffer[4]);
	checksum = ntohs(*(uint16_t *)&buffer[6]);

	cb = find_callback(&udpcb,destport);
	if(!cb) {
//		printf("No listener on port %d\n");
	} else {
		replyinfo.srcip = (uint32_t)srcip;
		replyinfo.srcport = sourceport;
		replyinfo.length = length - 8;
		cb(data,offset+8,len,&replyinfo);
	}
	//printf("udp handler took %ld usec\n", system_time() - perftime);
	
	return 0;
}


int udp_layer(bool on) 
{
	static int id = -1;
	if(on) {
		if(id == -1) {
		       id = registerIPV4Handler(IPV4_UDP,udp_workloop);
		}
	} else {
		if(id >= 0) {
			unregisterIPV4Handler(id);
			id = -1;
		}
	}
	return 0;
}

int transmit_udp(mymbuf_t *packet, uint32_t toip,int toport, int fromport)
{
	uint16_t *udpbuf;
	int len;

	udpbuf = (uint16_t *) mbuf_head(packet,8);
	len  = mbuf_getlength(packet);
	if(!udpbuf) {
		printf("No room for udp package. bailing\n");
		return -1;
	}
	udpbuf[0] = htons(fromport);
	udpbuf[1] = htons(toport);
	udpbuf[2] = htons(len);
       //Checksum for UDP is ..crazy? 	
	//XXX: TODO: Checksum!
	udpbuf[3] = 0;
	return transmit_and_add_ipv4(packet,toip,IPV4_UDP);
}
int registerUDPHandler(int port,callback_handler cb) 
{
	return registerCallback(&udpcb,port,cb);
}

int unregisterUDPHandler(int id)
{
	return unregisterCallback(&udpcb,id);

return 0;
}
