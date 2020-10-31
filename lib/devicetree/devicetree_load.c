/*
 * Copyright (C) 2010-2015 Apple Inc. All rights reserved.
 * Copyright (C) 2006 Apple Computer, Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */
#include <debug.h>
#include <lib/devicetree.h>
#include <lib/env.h>
#include <lib/fs.h>
#include <lib/image.h>
#include <lib/mib.h>
#include <sys/menu.h>
#include <string.h>

/* load the device tree out of flash into a heap-based piece of memory */
int dt_load(void)
{
	int result = 0;
	addr_t dtaddr;
	size_t dtsize;

	/* load to default addresses */
	dtaddr = mib_get_addr(kMIBTargetDefaultDeviceTreeAddress);
	dtsize = mib_get_size(kMIBTargetDefaultDeviceTreeSize);

	/* if we don't already have a devicetree, look for a known image containing one */
	if (dt_get_size() == 0) {
		
		if (image_load_type(&dtaddr, &dtsize, IMAGE_TYPE_DEVTREE, 0) != 0) {
			dprintf(DEBUG_INFO, "load_device_tree: failed to find device tree\n");
			return -1;
		}

		if (!dt_deserialize((void *)dtaddr, dtsize)) {
			dprintf(DEBUG_CRITICAL, "Mal-formed devicetree\n");
			result = -1;
		}
	}
	
	/* consolidate environment */
	security_consolidate_environment();

	return 0;
}

int dt_load_file(const char *path)
{
	int result = 0;
	u_int32_t type;
	addr_t dtaddr;
	size_t dtsize;

	/* load to default addresses */
	dtaddr = mib_get_addr(kMIBTargetDefaultDeviceTreeAddress);
	dtsize = mib_get_size(kMIBTargetDefaultDeviceTreeSize);

	// Image3 requires the non-restore types
	if (mib_get_u32(kMIBPlatformImageFormat) == 4) {
		// Image4
		type = IMAGE_TYPE_DEVTREE_RESTORE;
	} else {
		// Image3
		type = IMAGE_TYPE_DEVTREE;
	}

	if (image_load_file(path, &dtaddr, &dtsize, &type, 1, NULL, 0) != 0) {
		dprintf(DEBUG_INFO, "failed to load devicetree from %s", path);
		dt_init();
		return -1;
	}

	if (!dt_deserialize((void *)dtaddr, dtsize)) {
		dprintf(DEBUG_CRITICAL, "Mal-formed devicetree\n");
		result = -1;
	}
	
	/* consolidate environment */
	security_consolidate_environment();

	return 0;
}

int do_devicetree(int argc, struct cmd_arg *argv)
{
	addr_t addr = mib_get_addr(kMIBTargetDefaultLoadAddress);
	addr_t dtaddr = mib_get_addr(kMIBTargetDefaultDeviceTreeAddress);
	size_t dtsize = mib_get_size(kMIBTargetDefaultDeviceTreeSize);
	size_t len;
	u_int32_t type;

        len = env_get_uint("filesize", 0);

#if !RELEASE_BUILD
	if ((argc > 3) || ((argc > 1) && !strcmp("help", argv[1].str))) {
		printf("usage:\n\t%s [<len>] [<address>]\n", argv[0].str);
		return -1;
	}

	addr = env_get_uint("loadaddr", addr);

	if (argc > 1)
		len = argv[1].u;
	if (argc > 2)
		addr = argv[2].u;
#endif

	if (len == 0) {
		printf("filesize variable invalid or not set, aborting\n");
		return -1;
	}
	if (len > dtsize) {
		printf("Device Tree too large\n");
		return -1;
	}
	if (!security_allow_memory((void *)addr, len)) {
		printf("Permission Denied\n");
		return -1;
	}

	// Image3 requires the non-restore types
	if (mib_get_u32(kMIBPlatformImageFormat) == 4) {
		// Image4
		type = IMAGE_TYPE_DEVTREE_RESTORE;
	} else {
		// Image3
		type = IMAGE_TYPE_DEVTREE;
	}

	if (image_load_memory(addr, len, &dtaddr, &dtsize, &type, 1, NULL, 0) != 0) {
		printf("Device Tree image not valid\n");
		dt_init();
		return -1;
	}

	dt_deserialize((void *)dtaddr, dtsize);

	/* consolidate environment */
	security_consolidate_environment();

	printf("loaded device tree at %p of size 0x%zx, from image at %p\n", (void *)dtaddr, dtsize, (void *)addr);

	return 0;
}

