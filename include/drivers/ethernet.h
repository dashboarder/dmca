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
#ifndef __DRIVERS_ETHERNET_H
#define __DRIVERS_ETHERNET_H

#include <sys/types.h>
#include <lib/net/mbuf.h>

__BEGIN_DECLS

typedef struct ethq {
	int read,write;
	int numentries;
	int queue_len;
	int queue_len_mask;

	mymbuf_t **packets; // pointer to an array of pointers to mbufs
} ethqueue_t;

typedef struct ethdstats {
	int rxerrors; //Sum of all errors
	int txerrors; //Sum of all errors
	int runt; //?
	int extra; //?
	int crc; // crc error
	int dribble; //?
	int rxmissed;
	int txcollisions;
	int rxpktok;
	int txpktok;
	int rxbytes;
	int txbytes;
} ethdstats_t;


typedef struct ethpriv
{
	ethdstats_t dstats;
	ethqueue_t rxqueue;
	ethqueue_t txqueue;
	char macaddr[6];
} ethstat_t;

//Function declarations
//
//eth_init()
//NULL on failure, ptr private data otherwise.
int eth_init(void);

int eth_start(ethstat_t *);

int eth_stop(ethstat_t *);

ethstat_t *eth_get_handle(void);

int eth_printstatus(ethstat_t *);

int eth_getpkt(ethstat_t *,char *buffer,int *len);

int eth_output_packet(ethstat_t *d, const char *pkt,int len,int flag);

void eth_setmacaddr(ethstat_t *data, const char macaddr[6]);

void eth_getmacaddr(ethstat_t *data,char *buf);

void eth_uninit(ethstat_t *data);

#if WITH_HW_ETHERNET_SWITCH
int eth_switch_init(void);
#endif



__END_DECLS

#endif /* __DRIVERS_ETHERNET_H */

