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
#ifndef __TARGET_GPIODEF_H
#define __TARGET_GPIODEF_H

/* iPad4,x specific gpio -> pin mappings */

#include <platform/gpio.h>

/* define target-specific gpios in a generic fashion here. */
#define GPIO_LCD_PWR_EN		GPIO( 2, 7)

#define GPIO_RINGER_AB		GPIO(13, 6)

/* Board Revision */
#define GPIO_BOARD_REV0		GPIO(14, 6)
#define GPIO_BOARD_REV1		GPIO(14, 5)
#define GPIO_BOARD_REV2		GPIO(14, 4)
#define GPIO_BOARD_REV3		GPIO(14, 3)

/* which IICs to initialize */
#define IICS_MASK		(0x5)

#define TRISTAR_IIC_BUS		(2)

/* D2089 watchdog tickle, 'KEEPACT' */
#define GPIO_WDOG_TICKLE	GPIO(12, 6)

#endif /* ! __TARGET_GPIODEF_H */
