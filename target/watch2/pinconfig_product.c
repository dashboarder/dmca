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

#define PIN_CFG_AP 1
#include <target/pinconfig.h>
#undef PIN_CFG_AP

#define PIN_CFG_DEV 1
#include <target/pinconfig.h>
#undef PIN_CFG_DEV

extern const uint32_t* target_get_proto2a_gpio_cfg(uint32_t gpioc);
extern const uint32_t* target_get_proto2b_gpio_cfg(uint32_t gpioc);
extern const uint32_t* target_get_evt_gpio_cfg(uint32_t gpioc);

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
		return gpio_default_cfg_ap;
	}
	if (!board_rev_determined)
	{
		use_dummy_pinconfig = true;
		(void)ipod6_get_board_rev();
		use_dummy_pinconfig = false;

		board_rev_determined = true;
	}

	if (ipod6_get_board_rev() == BOARD_REV_PROTON /* BOARD_REV_DEV4 == BOARD_REV_PROTON == 0xF */)
	{
		// ProtoN and Dev3 share a GPIO spreadsheet. Fortunately they both have the same BOARD_REV
		switch (gpioc)
		{
		case 0:
			if (target_config_ap())
				return gpio_default_cfg_ap;
			else
				return gpio_default_cfg_dev;
			break;
		case 1:
			if (target_config_ap())
				return gpio_1_default_cfg_ap;
			else
				return gpio_1_default_cfg_dev;
			break;
		default:
			panic("unknown gpio controller");
		}		
	} else if (target_config_dev()) {
		// Dev boards 0xE through 0xA are currently all based off the proto2b Dev config, per
		// Board Rev tracker and email clarification from HW EE
		//
		if (ipod6_get_board_rev() >= 0xA)
			return target_get_proto2b_gpio_cfg(gpioc);
		else
			panic("Unknown BOARD_REV 0x%x", ipod6_get_board_rev());
	} else {
		// All that remains are post-ProtoN AP products.
		// The story gets more complicated here -- see Board Rev tracker spreadsheet
		switch (ipod6_get_board_rev())
		{
			case BOARD_REV_PROTO2A:
				return target_get_proto2a_gpio_cfg(gpioc);
				break;

			case BOARD_REV_PROTO2B:
			case BOARD_REV_PROTO2X_CARDINAL:
			case BOARD_REV_PROTO2B_ALT_CARBON:
			case BOARD_REV_PROTO2D:
			case BOARD_REV_PROTO2F:
				return target_get_proto2b_gpio_cfg(gpioc);
				break;
			case BOARD_REV_EVT:
			case BOARD_REV_EVT_ALT_CARBON:
				return target_get_evt_gpio_cfg(gpioc);
				break;
			default:
				panic("Unknown board revision 0x%x", ipod6_get_board_rev());
		}
	}

}
