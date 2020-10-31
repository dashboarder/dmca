/*
 * Copyright (C) 2009 Apple Computer, Inc. All rights reserved.
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
#include <lib/env.h>
#include <platform/memmap.h>
#include <platform/soc/hwregbase.h>
#include <drivers/audio/audio.h>

#if WITH_MENU
#if DEBUG_BUILD

#define AE2_ALIGN		(1 << 12)
#define AE2_ALIGN_MASK		(AE2_ALIGN - 1)

int do_ae2_go(int argc, struct cmd_arg *args)
{
	addr_t	addr;

	addr = env_get_uint("loadaddr", DEFAULT_LOAD_ADDRESS);

	if (addr & AE2_ALIGN_MASK) {
		dprintf(DEBUG_CRITICAL, "load address not on a page boundary, cannot remap\n");
		return(-1);
	}
	dprintf(DEBUG_INFO, "configuring AE2 for image @ 0x%08x\n", addr);

	/* reset the AE2 processor */
	rAE2_MCSCSR = 0;

	/* set up translation */
	rAE2_MCSATB = addr;

	/* release reset */
	rAE2_MCSCSR = 1;

	return(0);
}

MENU_COMMAND_DEBUG(ae2_go, do_ae2_go, "start AE2 with a downloaded image", NULL);

int do_ae2_stop(int argc, struct cmd_arg *args)
{
	/* reset the AE2 processor */
	rAE2_MCSCSR = 0;
	
	return(0);
}

MENU_COMMAND_DEBUG(ae2_stop, do_ae2_stop, "stop the AE2 processor", NULL);

#endif /* DEBUG_BUILD */
#endif /* WITH_MENU */
