/*
 * Copyright (C) 2010-2011 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */
#ifndef __TARGET_GPIODEF_H
#define __TARGET_GPIODEF_H

/* N94 specific gpio -> pin mappings */

#include <platform/gpio.h>

/* define target-specific gpios in a generic fashion here. */

#define GPIO_LCD_RST		GPIO( 2, 6)
#define GPIO_LCD_RST_POLARITY	(0)
#define GPIO_LCD_CHKSUM		GPIO( 2, 7)

#define GPIO_RINGER_AB		GPIO( 0, 4)

/* Board Revision */
#define GPIO_BOARD_REV0		GPIO( 5, 0)
#define GPIO_BOARD_REV1		GPIO( 5, 1)
#define GPIO_BOARD_REV2		GPIO( 5, 2)
#define GPIO_BOARD_REV3		GPIO(27, 0)

/* radio gpios */
#define GPIO_WLAN_RESET		GPIO( 0, 6)
#define GPIO_BT_RESET		GPIO( 1, 0)

#define POWER_GPIO_BATTERY_SWI	(5)
#define POWER_GPIO_BATTERY_SWI_CONFIG_OUTPUT	0x03
#define POWER_GPIO_BATTERY_SWI_CONFIG_INPUT	0xD8

/* which IICs to initialize */
#define IICS_MASK		(7)

/* D1881 watchdog tickle, 'KEEPACT' */
#define GPIO_WDOG_TICKLE	GPIO(2, 3)

#endif /* ! __TARGET_GPIODEF_H */
