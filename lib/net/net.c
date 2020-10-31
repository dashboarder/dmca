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
#include <drivers/ethernet.h>
#include <lib/env.h>
#include <lib/net.h>
#include <lib/net/xp.h>
#include <lib/net/callbacks.h>
#include <lib/net/ethernet.h>

//For the different layers
#include <lib/net/arp.h>
#include <lib/net/ipv4.h>
#include <lib/net/udp.h>
#include <lib/net/icmp.h>

#include <sys/callout.h>

struct callout ethernet_cb;
static bool netstack_initialized = false;

int start_network_stack(void) 
{
	if (netstack_initialized)
		return 0;

	dprintf(DEBUG_INFO, "starting net stack\n");

	/* get all of the environment variables we need up front */
	uint32_t ipaddr;
	uint8_t ethaddr[6];

	if (env_get_ipaddr("ipaddr", &ipaddr) < 0) {
		printf("ipaddr environment variable not set or malformed, cannot start net stack\n");
		return -1;
	}

	if (env_get_ethaddr("ethaddr", ethaddr)) {
		printf("ethaddr environment variable not set or malformed, cannot start net stack\n");
		return -1;
	}

	ethernet_init();
	xp_initialize();
	//YUCK! XXX: TODO:
	ipv4_layer(true);

	ipv4_set_ip(htonl(ipaddr));
	arp_layer(true);
	icmp_layer(true);
	udp_layer(true);

#if WITH_HW_ETHERNET
	dprintf(DEBUG_INFO, "starting ethernet\n");
	ethstat_t *eth = eth_get_handle();

	eth_setmacaddr(eth, (char *)ethaddr);
	eth_start(eth);
#endif

#if 0
	/* start the ethernet work loop */
	callout_enqueue(&ethernet_cb, 1000, ethernet_workloop, (void *)100);
#endif

	netstack_initialized = true;

	/* gratuitous arp */
	char mac[6];
	arp_get_macaddr(ipv4_get_ip(), mac);

	return 0;
}

int stop_network_stack(void)
{
	if (!netstack_initialized)
		return 0;

#if WITH_HW_ETHERNET
	ethstat_t *eth = eth_get_handle();

	eth_stop(eth);
#endif

	ipv4_layer(false);
	arp_layer(false);
	icmp_layer(false);
	udp_layer(false);
	ethernet_uninit();
	netstack_initialized = false;

	return 0;
}


/* utility routines for dealing with ip and mac address strings */
void mac2str(uint8_t mac[6], char *ascii, size_t size)
{
	snprintf(ascii, size, "%02x:%02x:%02x:%02x:%02x:%02x",mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);
}

void ip2str(const uint8_t *ip, char *str, size_t size)
{
	snprintf(str, size, "%u.%u.%u.%u", ip[3], ip[2], ip[1], ip[0]);
}

