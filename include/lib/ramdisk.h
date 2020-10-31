/*
 * Copyright (C) 2014 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */

#ifndef _RAMDISK_H
#define _RAMDISK_H

#include <sys/types.h>

extern int	ramdisk_init(void);
extern int	load_ramdisk_file(const char *path);
extern int	load_ramdisk(addr_t *ramdisk_addr, size_t *ramdisk_size);

#endif /* _RAMDISK_H */
