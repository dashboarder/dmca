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
#include <lib/net/ipv4.h>
#include <lib/net/callbacks.h>
#include <lib/net/icmp.h>
#include <lib/net/xp.h>

static callbacks_t icmpcb;

static uint16_t icmp_checksum(uint16_t *data,int len)
{
	uint32_t sum =0;
	uint16_t res;

	for(int i=0;i<len;i++)
		sum += data[i];
	sum = (sum &0xffff) + (sum >> 16);
	res = ~sum;
	return res;
}
static int icmp_send_echo(uint32_t toip,uint16_t identifier, uint16_t sequence,char *extradata,int datalen)
{
	mymbuf_t *packet;
	char *data;
	int ret;
	
	packet = mbuf_initialize(8+datalen,14+20,0);
	data = mbuf_tail(packet,8+datalen);

	data[0] = data[1] = 0; //type and code
	data[2] = data[3] = 0; //checksum, will be calculated later
	*(uint16_t *)&data[4] = htons(identifier);
	*(uint16_t *)&data[6] = htons(sequence);
	memcpy(&data[8],extradata,datalen);
	*(uint16_t *)&data[2] = icmp_checksum((uint16_t *)data,(8+datalen)>>1);

	ret = transmit_and_add_ipv4(packet,toip,IPV4_ICMP);
	mbuf_destroy(packet);
	return ret;
}


static int icmp_workloop(char *data, int offset, int len,void *srcip)
{
	char *buffer = data+offset;
	uint8_t type;
	uint8_t code;
	uint16_t checksum;

	type = buffer[0];
	code = buffer[1];
	checksum = ntohs(*(uint16_t *)&(buffer[2])); 
	
	switch(type) {
		case ICMP_ECHO:
			{
			uint16_t identifier = ntohs(*(uint16_t *)&(buffer[4])); 
			uint16_t sequence = ntohs(*(uint16_t *)&(buffer[6])); 
			char *echodata = buffer+8;
			
	//		printf("Echo %s: ident: 0x%x,sequence: 0x%d\n",(data == ICMP_ECHO) ? "echo" : "reply",identifier,sequence);
			icmp_send_echo((uint32_t)srcip,identifier,sequence,echodata,len-offset-8);
			}
			break;
		default:
			printf("Unsupported ICMP(%d)\n",type);
	}
	return 0;
}
int icmp_layer(bool on)
{
	static int id = -1;
	if(on) {
		if(id == -1) {
		       id = registerIPV4Handler(IPV4_ICMP,icmp_workloop);
		}
	} else {
		if(id >= 0) {
			unregisterIPV4Handler(id);
			id = -1;
		}
	}
	return 0;
}
//Just registers a ICMP handler.
int registerICMPHandler(int messagetype,callback_handler cb)
{
	return registerCallback(&icmpcb,messagetype,cb);
}

int unregisterICMPHandler(int id)
{
	return unregisterCallback(&icmpcb,id);
}
