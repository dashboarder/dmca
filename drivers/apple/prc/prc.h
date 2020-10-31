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
#ifndef __APPLE_PRC_H
#define __APPLE_PRC_H

#include <platform/soc/hwregbase.h>
#include <platform.h>

#ifdef DISP0_PRC_BASE_ADDR
#define PRC_BASE_ADDR	DISP0_PRC_BASE_ADDR
#endif
#ifndef PRC_BASE_ADDR
#error "PRC base addr for AppleWhitePointCorrection (PRC) Block is not defined"
#endif

#include SUB_PLATFORM_SPDS_HEADER(prc)

#endif //__APPLE_PRC_H