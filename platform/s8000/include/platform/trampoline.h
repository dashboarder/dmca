/*
 * Copyright (C) 2012 Apple Inc. All rights reserved.
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

#include <platform/soc/hwregbase.h>

/* ROM related registers in AOP MINIPMGR space */

#define SECURITY_REG		(AOP_MINIPMGR_BASE_ADDR + 0xD0010)
#define ROM_READ_DISABLE	(1 << 0)

#define REMAP_REG		(AOP_MINIPMGR_BASE_ADDR + 0xD0014)

#endif /* ! __PLATFORM_TRAMPOLINE_H */
