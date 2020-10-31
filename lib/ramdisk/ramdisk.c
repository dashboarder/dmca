/*
 * Copyright (C) 2007-2014 Apple Inc. All rights reserved.
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
#include <lib/env.h>
#include <lib/image.h>
#include <lib/mib.h>
#include <sys/menu.h>

addr_t gRAMDiskAddr = 0;
size_t gRAMDiskSize = 0;

int ramdisk_init(void)
{
	gRAMDiskAddr = 0;
	gRAMDiskSize = 0;
  
	return 0;
}

int do_ramdisk(int argc, struct cmd_arg *argv)
{
	addr_t addr = mib_get_addr(kMIBTargetDefaultLoadAddress);
	uint32_t len, type;

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

	if (!security_allow_memory((void *)addr, len)) {
		printf("Permission Denied\n");
		return -1;
	}

	gRAMDiskAddr = mib_get_addr(kMIBTargetDefaultRamdiskAddress);
	gRAMDiskSize = len;
	type = IMAGE_TYPE_RAMDISK;
	if (image_load_memory(addr, len, &gRAMDiskAddr, &gRAMDiskSize, &type, 1, NULL, 0)) {
		printf("Ramdisk image not valid\n");
		gRAMDiskAddr = 0;
		gRAMDiskSize = 0;
		return -1;
	}

	/* consolidate environment */
	security_consolidate_environment();

	dprintf(DEBUG_INFO, "loaded ramdisk at %p of size %#zx, from image at %p\n", (void *)gRAMDiskAddr, gRAMDiskSize, (void *)addr);

	return 0;
}

int
load_ramdisk_file(const char *path)
{
	u_int32_t type;

	if (!mib_get_bool(kMIBTargetWithFileSystem)) {
		return -1;
	}

	/* load to default addresses */
	gRAMDiskAddr = mib_get_addr(kMIBTargetDefaultRamdiskAddress);
	gRAMDiskSize = mib_get_size(kMIBTargetDefaultRamdiskSize);

	type = IMAGE_TYPE_RAMDISK;
	if (image_load_file(path, &gRAMDiskAddr, &gRAMDiskSize, &type, 1, NULL, 0)) {
		dprintf(DEBUG_INFO, "failed to load ramdisk from %s\n", path);
		gRAMDiskAddr = 0;
		gRAMDiskSize = 0;
		return -1;
	}

	/* consolidate environment */
	security_consolidate_environment();

	return 0;
}

int load_ramdisk(addr_t *ramdisk_addr, size_t *ramdisk_size)
{
	if ((gRAMDiskAddr == 0) || (gRAMDiskSize == 0)) {
		gRAMDiskAddr = mib_get_addr(kMIBTargetDefaultRamdiskAddress);
		gRAMDiskSize = mib_get_size(kMIBTargetDefaultRamdiskSize);

		if (image_load_type(&gRAMDiskAddr, &gRAMDiskSize, IMAGE_TYPE_RAMDISK, 0)) {
			gRAMDiskAddr = 0;
			gRAMDiskSize = 0;
			return -1;
		}
	}

	if ((gRAMDiskAddr == 0) || (gRAMDiskSize == 0)) {
		dprintf(DEBUG_INFO, "ramdisk: failed to find ramdisk\n");
		return -1;
	}

	/* consolidate environment */
	security_consolidate_environment();

	*ramdisk_addr = gRAMDiskAddr;
	*ramdisk_size = gRAMDiskSize;

	return 0;
}

