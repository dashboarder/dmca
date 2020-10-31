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
#include <drivers/ethernet.h>
#include <lib/macho.h>
#include <lib/net.h>
#include <lib/net/arp.h>
#include <sys/menu.h>

#if defined(WITH_MENU) && WITH_MENU

int do_net(int argc, struct cmd_arg *args)
{
	if (argc < 2) {
		puts("not enough arguments.\n");
usage:
		printf("%s init\n", args[0].str);
		printf("%s arp\n", args[0].str);
		return -1;
	}

	if (strcmp("init", args[1].str) == 0) {
		start_network_stack();
	} else if (strcmp("arp", args[1].str) == 0) {
		arp_dump_table();
	} else {
		puts("unrecognized command.\n");
		goto usage;
	}

	return 0;
}

MENU_COMMAND_DEVELOPMENT(net, do_net, "network/ethernet stuff", NULL);

#endif
