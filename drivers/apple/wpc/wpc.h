/*
 * Copyright (C) 2015 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */
#ifndef __APPLE_WPC_H
#define __APPLE_WPC_H

#include <platform/soc/hwregbase.h>
#include <platform.h>

#ifdef DISP0_WPC_BASE_ADDR
#define WPC_BASE_ADDR	DISP0_WPC_BASE_ADDR
#endif
#ifndef WPC_BASE_ADDR
#error "WPC base addr for AppleWhitePointCorrection (WPC) Block is not defined"
#endif

#include SUB_PLATFORM_SPDS_HEADER(wpc)

#endif //__APPLE_WPC_H