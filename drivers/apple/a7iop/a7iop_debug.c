/*
 * Copyright (C) 2011 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */

#include <debug.h>
#include <drivers/a7iop/a7iop.h>
#include <platform/memmap.h>
#include <sys.h>
#include <sys/menu.h>
#include <sys/task.h>

#if defined(WITH_MENU) && WITH_MENU

static int _a7iop_debug_main(int argc, struct cmd_arg *args);
static int _a7iop_debug_start_kf(KFWRAPPER_TYPE_T type);
static int _a7iop_debug_stop_kf(KFWRAPPER_TYPE_T type);

MENU_COMMAND_DEBUG(a7iop_test, _a7iop_debug_main, "Test A7IOP interface", NULL);

static int _a7iop_debug_main(int argc, struct cmd_arg *args) {
	const char *s;
	KFWRAPPER_TYPE_T type;
	
	if (argc < 3) {
		goto print_usage;
	}

	s = args[1].str;
	if (strcmp(s, "ans") == 0) {
		type = KFW_ANS;
	}
	else if (strcmp(s, "sio") == 0) {
		type = KFW_SIO;
	}
	else if (strcmp(s, "sep") == 0) {
		type = KFW_SEP;
	} else {
		goto print_usage;
	}
	
	s = args[2].str;
	if (strcmp(s, "start") == 0) {
		return _a7iop_debug_start_kf(type);
	}
	else if (strcmp(s, "stop") == 0) {
		return _a7iop_debug_stop_kf(type);
	} else {
		goto print_usage;
	}
	
print_usage:
	printf("Usage: ");
	printf("a7iop_test [ans/sio/sep] [start/stop]\n");
	return -1;
}

static int _a7iop_debug_start_kf(KFWRAPPER_TYPE_T type)
{
	addr_t fw_base;
	uint32_t code[] = { 0xe3a00000, 0xe3a01001, 0xe3a02002, 0xe3a03003, 0xe3a04004, 0xe320f003 };

	fw_base = (addr_t)DEFAULT_LOAD_ADDRESS;
	bcopy((void *)code, (void *)fw_base, sizeof(code));
	
	akf_start(type, fw_base, sizeof(code));

	return 0;
}

static int _a7iop_debug_stop_kf(KFWRAPPER_TYPE_T type)
{
	akf_stop(type);
	
	return 0;
}

#endif // defined(WITH_MENU) && WITH_MENU
