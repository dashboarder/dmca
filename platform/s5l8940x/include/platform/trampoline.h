/*
 * Copyright (C) 2010 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */
#ifndef __PLATFORM_TRAMPOLINE_H
#define __PLATFORM_TRAMPOLINE_H

#include <platform/memmap.h>
#include <platform/soc/hwregbase.h>

#define SECURITY_REG		(PMGR_BASE_ADDR + 0xA000)
#define ROM_READ_DISABLE	(1 << 0)

#endif /* ! __PLATFORM_TRAMPOLINE_H */
