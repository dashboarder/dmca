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
/* This file contains platform abstractions, implement the required functions for your platform */
#include <string.h>
#include <debug.h>
#include <drivers/ethernet.h>
#include <lib/net/xp.h>
#include <lib/net/ethernet.h>

void *ethptr;

/* ethernet card is giving us info */
void xp_packet_input(void *ethptr, mymbuf_t *inputbuf)
{
//	printf("xp_packet_input: buf %p, len %d\n", inputbuf, mbuf_getlength(inputbuf));
	ethernet_input(inputbuf);
}

#if 0
mymbuf_t *xp_get_packet(mymbuf_t *inputbuf)
{
	mymbuf_t *mb;
	char *buffer;
	int len = 1500;
	if(!inputbuf) {	
		mb = mbuf_initialize(1500,0,0); 
	} else {
		if(mbuf_getallocatedlength(inputbuf) < 1500) {
			return NULL;
		}
		mb = inputbuf;
	}
	buffer = mbuf_getlinear(mb);

	if(eth_getpkt(ethptr,buffer,&len) < 0) {
		if(inputbuf != mb) {
			mbuf_destroy(mb);
		}
		return 0;
	}
	mbuf_setlength(mb,len);
	return mb;	
}
#endif

int xp_transmit_packet(mymbuf_t *buffer) 
{
	char *rawdata = mbuf_getlinear(buffer);

	if(!rawdata) {
		printf("Handle linked mbuf's\n");
		return -1;
	}
	eth_output_packet(ethptr,rawdata,mbuf_getlength(buffer),0);
return 0;
}

int xp_getmac(uint8_t *addr) 
{
	eth_getmacaddr(ethptr,(char *)addr);
return 0;
}

int xp_initialize(void)
{
	ethptr = eth_get_handle();
	return 0;
}


