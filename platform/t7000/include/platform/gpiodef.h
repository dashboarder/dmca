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

#ifndef __PLATFORM_GPIODEF_H
#define __PLATFORM_GPIODEF_H

#include <platform.h>
#include <platform/gpio.h>
#include <platform/soc/hwregbase.h>

#if WITH_TARGET_CONFIG
# include <target/gpiodef.h>
#endif

#include SUB_PLATFORM_HEADER(gpiodef)

#endif /* ! __PLATFORM_GPIODEF_H */
