/*
 * Copyright (C) 2010 Apple, Inc. All rights reserved.
 *
 * This document is the property of Apple Computer, Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Computer, Inc.
 */
#include <stdio.h>
#include <string.h>
#include <sys.h>
#include <sys/menu.h>

#if defined(WITH_MENU) && WITH_MENU

extern void L2VRandomTest(void);

static int do_l2v(int argc, struct cmd_arg *args)
{
    UInt64 start = system_time();
	L2VRandomTest();
	printf("test time: %lldus\n", system_time() - start);
	return(0);
}
MENU_COMMAND_DEBUG(l2v, do_l2v, "run l2v benchmark", NULL);

#endif
