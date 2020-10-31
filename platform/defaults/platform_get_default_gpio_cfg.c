/*
 * Copyright (C) 2014-2015 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */
#include <debug.h>
#include <platform.h>
#include <target.h>
#if !WITH_TARGET_CONFIG && WITH_HW_GPIO
#include <drivers/apple/gpio.h>
#include <platform/pinconfig.h>
#endif

const uint32_t* platform_get_default_gpio_cfg(uint32_t gpioc)
{
#if WITH_TARGET_CONFIG
	return target_get_default_gpio_cfg(gpioc);
#elif WITH_HW_GPIO
	if (gpioc == 0)
		return gpio_default_cfg_0;
#ifdef GPIO_1_GROUP_COUNT
	if (gpioc == 1)
		return gpio_default_cfg_0;
#endif
	panic("unknown gpio controller %u", gpioc);
#else
	panic("no default pinconfig source");
#endif
}
