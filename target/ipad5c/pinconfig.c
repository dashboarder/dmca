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
#include <debug.h>
#include <drivers/apple/gpio.h>
#include <platform/gpiodef.h>
#include <platform/soc/hwregbase.h>
#include <target.h>
#include <target/boardid.h>

extern uint32_t ipad5c_get_board_rev(void);
extern const uint32_t* target_get_proto2_gpio_cfg(uint32_t gpioc);
extern const uint32_t* target_get_proto3_gpio_cfg(uint32_t gpioc);
extern const uint32_t* target_get_evt_gpio_cfg(uint32_t gpioc);
extern const uint32_t* target_get_evt2_gpio_cfg(uint32_t gpioc);

extern bool check_is_board_deprecated;

static bool board_rev_determined;
static bool use_dummy_pinconfig;

const uint32_t *target_get_default_gpio_cfg(uint32_t gpioc)
{
	// HACK: -- the first call into target_get_default_gpio_cfg() reads
	// and attempts to restore BOARD_REV[3:0] to GPIO_CFG_DEFAULT.
	//
	// This will result in a recursive call back into target_get_default_gpio_cfg.
	// To break the recursion, while determining board rev we can reply with any pinconfig
	// since it's only being used for BOARD_REV[3:0] and it's CFG_DISABLED in all pinconfigs
	//
	// Remove this after we no longer have BOARD_REV-dependent pinconfigs.	
	if (use_dummy_pinconfig)
	{
		return target_get_evt_gpio_cfg(gpioc);
	}
	if (!board_rev_determined)
	{
		use_dummy_pinconfig = true;
		(void)ipad5c_get_board_rev();
		use_dummy_pinconfig = false;

		board_rev_determined = true;
	}

	if (!target_config_ap()) {
		//For dev boards, just use the current settings for proto 1 until it changes
		switch (ipad5c_get_board_rev()) {
		case J98_DEV2_BOARD_REV:
			return target_get_proto3_gpio_cfg(gpioc);

		default:
			goto fail;
		}
	} else {
		// Current AP products have several pinconfigs depending on board revision.
		switch (ipad5c_get_board_rev()) {
		case J99_PROTO2_BOARD_REV:
			return target_get_proto2_gpio_cfg(gpioc);

		case J99_PROTO3_BOARD_REV:
		case J99_PROTO3_P9_BOARD_REV:
			return target_get_proto3_gpio_cfg(gpioc);
		case J99_EVT_BOARD_REV:
			return target_get_evt_gpio_cfg(gpioc);
		case J99_EVT2_BOARD_REV:
			return target_get_evt2_gpio_cfg(gpioc);
		default:
			goto fail;
		}
	}
fail:
	//Allow more of the HW to get far enough to detect a deprecated board
	check_is_board_deprecated = true;

	return target_get_evt_gpio_cfg(gpioc);
}
