/*
 * Copyright (C) 2012-2013 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */
#include <drivers/display.h>
#include <lib/paint.h>
#include <platform.h>
#include <platform/memmap.h>
#include <target.h>

int display_get_info(struct display_info *info)
{
	// Relatively sane defaults of 1136x640 @2x
	info->stride = 640 * 4;
	info->width = 640;
	info->height = 1136;

	// 32bpp and @2x scale
	info->depth = 32 | 1 << 16;
	info->framebuffer = (uintptr_t)PANIC_BASE - (((info->stride * info->height + 0xFFFUL) & ~0xFFFUL) * 3);

	return 0;
}
