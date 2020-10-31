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
#include <stdint.h>
#include <sys.h>
#include <sys/task.h>
#include <sys/callout.h>
#include <sys/security.h>

#include <lib/net.h>
#include <lib/net/xp.h>
#include <lib/net/arp.h>
#include <lib/net/ipv4.h>
#include <lib/net/udp.h>

static struct {
	uint32_t	destip;
	uint16_t	destport;
	uint16_t	srcport;
	mymbuf_t *	outgoingbuf;
	int		errcount;
	struct callout timer;

	volatile int 	complete;

	int		operation;
	union {
		struct r {
			char *		destbuffer;
			int 		destlen;
			int		pos;
			int		blocknum;
		}r;
		struct {
			char *		sourcebuffer;
			int		left;
			int		blocknum;
		}w;
	} u;
} tftp_stat;

#define TFTP_RECEIVE	0
#define TFTP_SEND	1

#define TFTP_BLOCKSIZE 512
#define TFTP_RRQ 1
#define TFTP_WRQ 2
#define TFTP_DATA 3
#define TFTP_ACK 4
#define TFTP_ERR 5

static void tftp_resetbuf(void) 
{
	mbuf_setlength(tftp_stat.outgoingbuf,0);
	mbuf_setoffset(tftp_stat.outgoingbuf,14+20+8);	
}

static int tftp_sendack(uint32_t destip,uint32_t destport,int blocknum)
{
//	mymbuf_t *data = mbuf_initialize(4,14+20+8,0);
	uint16_t *rawdata;
	tftp_resetbuf();
	rawdata	= (uint16_t *)mbuf_tail(tftp_stat.outgoingbuf,4);

	rawdata[0] = htons(TFTP_ACK);
	rawdata[1] = htons(blocknum);
	
	transmit_udp(tftp_stat.outgoingbuf,destip,destport,tftp_stat.srcport);
//	mbuf_destroy(data);
	return 0;
}

static int tftp_senddata(uint32_t destip,uint32_t destport, int blocknum, char *inputdata,int datalen)
{
//	mymbuf_t *data = mbuf_initialize(4+datalen,14+20+8,0);
	uint16_t *rawdata;
	tftp_resetbuf();
	rawdata = (uint16_t *)mbuf_tail(tftp_stat.outgoingbuf,4+datalen);
	
	rawdata[0] = htons(TFTP_DATA);
	rawdata[1] = htons(blocknum);
	memcpy((char*)&rawdata[2],inputdata,datalen);
	
	transmit_udp(tftp_stat.outgoingbuf,destip,destport,tftp_stat.srcport);
//	mbuf_destroy(data);
	return 0;
}


static int tftp_receive_listener(char *buf, int offset,int len, void *arg)
{
	uint16_t *databuf =(uint16_t *)(buf+offset);
	uint16_t opcode = ntohs(databuf[0]);
	int blocknum;
	udpinfo_t *info = (udpinfo_t *)arg;
	int sendlength;
	int errnum;
	int datalen = info->length - 4;  //len - offset - 4; //<- would fail if the ethernet added a trailer.
	static const char *spinchars = "|/-\\";
	static int spinidx = 0;
	static int spincount = 0;
	
	callout_reset(&tftp_stat.timer, 0);
	tftp_stat.destip = info->srcip;
	tftp_stat.destport = info->srcport;
	blocknum = ntohs(databuf[1]);

//	printf("tftp_recv: opcode %d, blocknum %d\n", opcode, blocknum);

	switch(opcode) {
		case TFTP_DATA:
			//send ack
			if(tftp_stat.u.r.pos + datalen <= tftp_stat.u.r.destlen) {
				memcpy(tftp_stat.u.r.destbuffer+tftp_stat.u.r.pos,&databuf[2],datalen);
			} else {
				printf("Not enough space in databuffer(pos:%d,datalen: %d, destlen: %d\n",tftp_stat.u.r.pos,datalen,tftp_stat.u.r.destlen);
				tftp_stat.complete = -1;
				return -1;
			}
			tftp_stat.u.r.pos += datalen;
			if(datalen != TFTP_BLOCKSIZE) {
				tftp_sendack(info->srcip,info->srcport,blocknum);
				tftp_stat.complete = 1;
				break;
			}
			tftp_stat.u.r.blocknum = blocknum;
			tftp_sendack(info->srcip,info->srcport,blocknum);

			// a user-comfort UI that uses less space than a bunch of dots
			spincount += datalen;
			if (spincount >= 65536) {
				spincount = 0;
				spinidx++;
				if (spinchars[spinidx] == 0)
					spinidx = 0;
				putchar(spinchars[spinidx]);
				putchar('\r');
			}
			break;
		case TFTP_ACK: 
			//blocknum 0 is ack for our metadata.
			if(blocknum != 0) {
				tftp_stat.u.w.left -= 512;
				if(tftp_stat.u.w.left <= 0) {
					tftp_stat.u.w.left = 0;
					tftp_stat.complete = 1;
					return 0;
				}
			} 
			sendlength =  __min(tftp_stat.u.w.left,TFTP_BLOCKSIZE);
			tftp_stat.u.w.blocknum = blocknum + 1;

			tftp_senddata(tftp_stat.destip,tftp_stat.destport,tftp_stat.u.w.blocknum,tftp_stat.u.w.sourcebuffer+((tftp_stat.u.w.blocknum-1) * TFTP_BLOCKSIZE),sendlength);
			break;
		case TFTP_ERR:
			errnum = ntohs(databuf[1]);
			printf("tftp error (%d) (%s)\n",errnum,(char*)&databuf[2]);
			tftp_stat.complete = -1;
			break;
		default:
			printf("unkown package %d\n",opcode);
	}
	return 0;
}

static void tftp_timeout(struct callout *c, void *arg) 
{
	callout_reset(c, 0);
	tftp_stat.errcount++;
	if(tftp_stat.errcount > 20) { // 10 seconds worth of retransmits
		tftp_stat.complete = -1;
		return;
	}
	if(tftp_stat.operation == TFTP_RECEIVE) {
		tftp_sendack(tftp_stat.destip,tftp_stat.destport,tftp_stat.u.r.blocknum);
	} else if(tftp_stat.operation == TFTP_SEND) {
			tftp_senddata(tftp_stat.destip, tftp_stat.destport, tftp_stat.u.w.blocknum,
					tftp_stat.u.w.sourcebuffer+((tftp_stat.u.w.blocknum-1) * TFTP_BLOCKSIZE),
					__min(tftp_stat.u.w.left,TFTP_BLOCKSIZE));
	} else {
		printf("Unknown mode\n");
		tftp_stat.complete = -1;
	}
	printf("Timeout\n");
}

int tftp_transfer(const char *filename, uint32_t ipaddr, char *buffer, int *maxlen, bool write)
{
#if WITH_NET
	char *pktbuf;
	int handlerid;
	int ret = 0;
	utime_t perftime = system_time();
	int kb, filename_length, buf_length;
	char serveripstr[32];

	if(*maxlen > (0xffff * 512)) {
		printf("TFTP (without extensions) can only handle max 32M tranfers\n");
		return -1;
	}

	if(!security_allow_memory(buffer, *maxlen)) {
		printf("Permission Denied\n");
		return -1;
	}

	/* try to start the network stack, if it isn't already running */
	ret = start_network_stack();
	if (ret < 0)
		return ret;

	ip2str((uint8_t *)&ipaddr, serveripstr, 32);
	dprintf(DEBUG_INFO, "tftp: starting transfer of file '%s' on server %s to address %p\n", filename, serveripstr, buffer);

	tftp_stat.outgoingbuf = mbuf_initialize(516,14+20+8,0); //ip head + ether head

	tftp_resetbuf();
	memset(&tftp_stat.u,0,sizeof(tftp_stat.u));

	if(write) {
		tftp_stat.u.w.sourcebuffer = buffer;
		tftp_stat.u.w.left = *maxlen;
	} else {
		tftp_stat.u.r.destbuffer = buffer;
		tftp_stat.u.r.destlen = *maxlen;
	}
	tftp_stat.operation = write ? TFTP_SEND : TFTP_RECEIVE;
	tftp_stat.destip = ipaddr;
	tftp_stat.destport = 69;
	tftp_stat.complete = tftp_stat.errcount = 0;
	tftp_stat.srcport = (rand() + 1024) % 65536;

	filename_length = strlen(filename);
	pktbuf = mbuf_tail(tftp_stat.outgoingbuf,2+filename_length+1+6);
	if (pktbuf == 0) return -1;
	buf_length = mbuf_getlength(tftp_stat.outgoingbuf);
	*(uint16_t *)&pktbuf[0] = ntohs(write ? TFTP_WRQ : TFTP_RRQ);
	strlcpy(&pktbuf[2],filename,buf_length-2);
	strlcpy(&pktbuf[filename_length+3],"octet",buf_length-filename_length-3);

	/* try to prepopulate the arp cache */
	arp_fill(tftp_stat.destip, 5*1000*1000);

	handlerid = registerUDPHandler(tftp_stat.srcport,tftp_receive_listener);
	if(transmit_udp(tftp_stat.outgoingbuf, tftp_stat.destip, tftp_stat.destport, tftp_stat.srcport) < 0) {
		printf("Not able to transmit package.\n");
		unregisterUDPHandler(handlerid);
		return -1;
	}
	//Start timeout handler

	callout_enqueue(&tftp_stat.timer, 500*1000, tftp_timeout, 0); //500ms without activity, send a new package.
	while(tftp_stat.complete == 0) 
		task_yield();

	if(tftp_stat.complete < 0) {
		ret = -1;
		goto out;
	} 

	if(write == false) {
		*maxlen = tftp_stat.u.r.pos;
	}
	perftime = system_time() - perftime;
	if(write) {
		kb = (*maxlen - tftp_stat.u.w.left)/1024;
	} else {
		kb = tftp_stat.u.r.pos/1024;
	}

	printf("%d KiB (%d bytes) tftp transfer completed at %d.%03d KiB/s\n",
	       kb, *maxlen, (kb*1000000)/perftime, (kb*1000000)%perftime);

out:
	unregisterUDPHandler(handlerid);
	callout_dequeue(&tftp_stat.timer);
	mbuf_destroy(tftp_stat.outgoingbuf);
	tftp_stat.outgoingbuf = 0;

	return ret;
#else /* !WITH_NET */
	printf ("network not available\n");
	return -1;
#endif /* WITH_NET */
}
