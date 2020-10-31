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
//a very minimal Ethernet stack.
//protocol handlers can register for a certain service, and this layer will call their functions when a packet of that type arrive
#include <debug.h>
#include <stdint.h>
#include <sys.h>
#include <sys/callout.h>
#include <lib/net/xp.h>
#include <lib/net/callbacks.h>
#include <lib/net/ethernet.h>

//For the different layers
#include <lib/net/arp.h>
#include <lib/net/ipv4.h>
#include <lib/net/udp.h>
#include <lib/net/icmp.h>


static callbacks_t ethercb;

int registerEtherTypeHandler(int ethertype,callback_handler cb) 
{
	return registerCallback(&ethercb,ethertype,cb);
}

int unregisterEtherTypeHandler(int id) 
{
	return unregisterCallback(&ethercb,id);
}


static int parseEtherHeader(char *buff,int len,uint16_t *ethertype) 
{
	
	//XXX: Check that the destination macaddress matches.
	if(ethertype) {
		*ethertype = ntohs(*(uint16_t *)(buff+12));
	}
	return 0;
}

void ethernet_init(void)
{
}

void ethernet_uninit(void) 
{
}

void ethernet_input(mymbuf_t *inputbuf)
{
	uint16_t ethertype;
	int len;
	char *rawbuf;
	callback_handler cb;

	len = mbuf_getlength(inputbuf);
	rawbuf =  mbuf_getlinear(inputbuf);
	//This fails if the packet was not directed to us (ie, wrong macaddr)
	if(parseEtherHeader(rawbuf,len,&ethertype) == 0) {
		cb = find_callback(&ethercb,ethertype);
		if(cb) { 
			cb(rawbuf,ETHERNET_OFFSET,len,0);
		} else {
//			printf("Unsupported ethertype 0x%x\n",ethertype);
		}
	}	
}

#if 0
//Called from timer (background) or from tsys commandprompt if not background
//Handles maxpkt packages before returning. (Or until it would have to block)
void ethernet_workloop(struct callout *c, void *param)
{
	int i;
	uint16_t ethertype;
	int len;
	char *rawbuf;
	callback_handler cb;
	int maxpkt = (int)param;
//	utime_t now;
//

	callout_reset(c, 0);

	if(!buf) 
		return;

	for(i = 0;i<maxpkt;i++) {
//		now = system_time();
		mbuf_setlength(buf,0);
		mbuf_setoffset(buf,0);
		if(xp_get_packet(buf)) {
			len = mbuf_getlength(buf);
			rawbuf =  mbuf_getlinear(buf);
			//This fails if the packet was not directed to us (ie, wrong macaddr)
			if(parseEtherHeader(rawbuf,len,&ethertype) == 0) {
				cb = find_callback(&ethercb,ethertype);
				if(cb) { 
					cb(rawbuf,ETHERNET_OFFSET,len,0);
				} else {
//					printf("Unsupported ethertype 0x%x\n",ethertype);
				}
				
			}
		} else {
			return;
		}
//		now = system_time() - now;
//		printf("a package took %ld usec\n",now);
	}	
	
return;
}
#endif

int add_eth_and_transmit(mymbuf_t *buffer,char *macaddrto,int ethertype) 
{
	char *ethernethead;
//	utime_t perftime = system_time();
	
	ethernethead = mbuf_head(buffer,14);
	if(!ethernethead) {
		dprintf(DEBUG_INFO, "Ethernetheader: Not enough space in mbuf.\n");
		return -1;
	}
	
	memcpy(ethernethead,macaddrto,6);
	xp_getmac((uint8_t*)&ethernethead[6]);
	*(uint16_t *)&ethernethead[12] = htons(ethertype);
	/*
	tailcrc = mbuf_tail(buffer,4);

	//XXX: Calculate CRC
	printf("Calculate CRC!\n");	
*/ // Is done by our ethernet driver...
	return xp_transmit_packet(buffer);
//	printf("transmit took %ld usec\n", system_time() - perftime);
}




