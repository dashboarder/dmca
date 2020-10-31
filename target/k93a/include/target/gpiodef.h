/*
 * Copyright (C) 2011 Apple Inc. All rights reserved.
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

/* K93a specific gpio -> pin mappings */

#include <platform/gpio.h>

/* define target-specific gpios in a generic fashion here. */
#define GPIO_LCD_RST		GPIO(16, 0)
#define GPIO_LCD_RST_POLARITY	(0)
#define GPIO_LCD_PWR_EN		GPIO(16, 1)

#define GPIO_RINGER_AB		GPIO( 0, 4)

#define POWER_GPIO_BATTERY_SWI	(5)
#define POWER_GPIO_BATTERY_SWI_CONFIG_OUTPUT	0x0B
#define POWER_GPIO_BATTERY_SWI_CONFIG_INPUT	0xD8

/* Board Revision */
#define GPIO_BOARD_REV0		GPIO( 4, 5)
#define GPIO_BOARD_REV1		GPIO( 4, 6)
#define GPIO_BOARD_REV2		GPIO( 4, 7)

/* which IICs to initialize */
#define IICS_MASK		(7)

/* D1946 watchdog tickle, 'KEEPACT' */
#define GPIO_WDOG_TICKLE	GPIO(2, 3)

#endif /* ! __TARGET_GPIODEF_H */
