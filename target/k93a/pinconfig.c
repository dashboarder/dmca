/*
 * Copyright (C) 2009-2014 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */
#include <drivers/apple/gpio.h>
#include <platform/soc/hwregbase.h>
#include <target.h>

#define PIN_CFG_AP 1
#include <target/pinconfig.h>
#undef PIN_CFG_AP

#define PIN_CFG_DEV 1
#include <target/pinconfig.h>
#undef PIN_CFG_DEV

const uint32_t *target_get_default_gpio_cfg(uint32_t gpioc)
{
	if (target_config_ap())
		return gpio_default_cfg_ap;
	else
		return gpio_default_cfg_dev;
}
