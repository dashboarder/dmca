/*
 * Copyright (C) 2006 Apple Computer, Inc. All rights reserved.
 * Copyright (C) 2007-2012 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Computer, Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Computer, Inc.
 */
#ifndef __DRIVERS_FLASH_NOR_H
#define __DRIVERS_FLASH_NOR_H

#include <sys/types.h>
#include <lib/blockdev.h>

__BEGIN_DECLS

struct nor_blockdev {
	struct blockdev bdev;

	uintptr_t handle;

	int (* readRange)(uintptr_t handle, uint8_t *ptr, uint32_t offset, uint32_t length);
	int (* writeRange)(uintptr_t handle, const uint8_t *ptr, uint32_t offset, uint32_t length);
	int (* eraseRange)(uintptr_t handle, uint32_t offset, uint32_t length);
};

/* pass -1 for which_device to use the target-defined default device */
int flash_nor_init(int which_device);
int flash_nor_register(struct nor_blockdev *nor_bdev, const char *name, uint64_t len, uint32_t block_size);

__END_DECLS

#endif /* __DRIVERS_FLASH_NOR_H */
