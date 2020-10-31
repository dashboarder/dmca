/*
 * Copyright (C) 2007-2013 Apple Inc. All rights reserved.
 * Copyright (C) 2006 Apple Computer, Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */
#include <arch.h>
#include <debug.h>
#include <lib/env.h>
#include <lib/blockdev.h>
#include <lib/nvram.h>
#include <platform.h>
#include <platform/memmap.h>
#include <target.h>

#if WITH_ENV

void sys_setup_default_environment(void)
{
	env_set("boot-partition", "0", 0);
	env_set("boot-path", "/System/Library/Caches/com.apple.kernelcaches/kernelcache", 0);

	env_set("build-style", build_style_string, 0);
	env_set("build-version", build_tag_string, 0);

	env_set("config_board", CONFIG_BOARD_STRING, 0);
	env_set_uint("loadaddr", (uintptr_t)DEFAULT_LOAD_ADDRESS, 0);
	
	// used by PR to figure out max allowed ramdisk size
	env_set_uint("ramdisk-size", (uintptr_t)DEFAULT_RAMDISK_SIZE, 0);

	env_set("boot-command", "fsboot", 0);

#if DEBUG_BUILD
	env_set("bootdelay", "3", 0);
	env_set("auto-boot", "false", 0);
	env_set("idle-off", "false", 0);
	env_set("debug-uarts", "3", 0);
#else
	// release, development builds just boot directly
	env_set("bootdelay", "0", 0);
	env_set("auto-boot", "true", 0);
	env_set("idle-off", "true", 0);
#endif

#if !RELEASE_BUILD
	#if WITH_IMAGE4
		env_set("diags-path", "/AppleInternal/Diags/bin/diag.img4", 0);
	#else
		env_set("diags-path", "/AppleInternal/Diags/bin/diag.img3", 0);
	#endif
#endif // !RELEASE_BUILD

	if (target_config_dev()) {
		// Dev boards shouldn't idle-off
		env_set("idle-off", "false", 0);
	}

	platform_setup_default_environment();
}

void sys_load_environment(void)
{
#if WITH_NVRAM
	nvram_load();
#else
	dprintf(DEBUG_CRITICAL, "WARNING: No nvram available to load persistent environment from.\n");
#endif
}

#endif /* WITH_ENV */
