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
#include <sys/menu.h>
#include <lib/tftp.h>
#include <lib/macho.h>
#include <lib/env.h>
#include <lib/net.h>
#include <platform/memmap.h>

static int tftp_cmd_getter( const uchar * address, uint offset, uint * pamount, void * cookie )
{
    uchar * baseAddress = (uchar *)cookie;

    if (!security_allow_memory(baseAddress+offset, *pamount)) {
        printf("Permission Denied\n");
        return -1;
    }

    memcpy( baseAddress+offset, address, *pamount );

    return 0;
}


int do_tftp(int argc, struct cmd_arg *args)
{
	if (argc < 2) {
		printf("not enough args.n\n");
usage:
		printf("%s get <filename> [<address>]\n", args[0].str);
		printf("%s getscript <filename> [<address>]\n", args[0].str);
		return -1;
	}

	if (!strcmp("get", args[1].str) || !strcmp("getscript", args[1].str)) {
		if (argc < 3) {
			printf("not enough arguments.\n");
			goto usage;
		}

		int maxlen = 0xffff*512;
		char *addr = (char *)env_get_uint("loadaddr", DEFAULT_LOAD_ADDRESS);
		int err;

		if (argc >= 4)
			addr = (char *)args[3].u;

		uint32_t serverip;
		if (env_get_ipaddr("serverip", &serverip) < 0) {
			printf("serverip environment variable not set or malformed\n");
			return -1;
		}

	        err = tftp_transfer(args[2].str, htonl(serverip), addr, &maxlen, false);

		if (err < 0)  {
			printf("error fetching file\n");
			return err;
		}

		/* stick the length into the environment */
		env_set_uint("filesize", maxlen, 0);

		if (!strcmp("getscript", args[1].str)) {
			if (err >= 0) {
				addr[maxlen] = 0;
				debug_run_script(addr);
			}
		}
	} else {
		printf("unrecognized command\n");
		goto usage;
	}

	return 0;
}

MENU_COMMAND_DEVELOPMENT(tftp, do_tftp, "tftp via ethernet to/from device", NULL);
