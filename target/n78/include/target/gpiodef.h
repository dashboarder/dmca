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

/* N78 specific gpio -> pin mappings */

#include <platform/gpio.h>

/* define target-specific gpios in a generic fashion here. */
#define GPIO_LCD_RST		GPIO( 2, 6)
#define GPIO_LCD_RST_POLARITY	(0)

#define GPIO_RINGER_AB		GPIO( 0, 4)

/* Board Revision */
#define GPIO_BOARD_REV0		GPIO( 4, 5)
#define GPIO_BOARD_REV1		GPIO( 4, 6)
#define GPIO_BOARD_REV2		GPIO( 4, 7)
#define GPIO_BOARD_REV3		GPIO( 2, 1)

/* Grape ID */
#define GPIO_GRAPE_ID0		GPIO( 3, 7)
#define GPIO_GRAPE_ID1		GPIO( 4, 0)
#define GPIO_GRAPE_ID2		GPIO( 4, 3)

/* which IICs to initialize */
#define IICS_MASK		(5)

#define TRISTAR_IIC_BUS		(0)

/* D1881 watchdog tickle, 'KEEPACT' */
#define GPIO_WDOG_TICKLE	GPIO(2, 3)

#endif /* ! __TARGET_GPIODEF_H */
