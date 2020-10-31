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

/* iPhone6,x specific gpio -> pin mappings */

#include <platform/gpio.h>

/* define target-specific gpios in a generic fashion here. */
#define GPIO_SOCHOT0_IN		GPIO(15, 0)		// BB_TO_AP_SOCHOT0

#define GPIO_LCD_RST            GPIO(12, 6)		// AP_TO_LCM_RESET_L
#define GPIO_LCD_RST_POLARITY   (0)

#define GPIO_X162_INT		GPIO( 9, 7)		// NAVAJO_TO_PMU_INT_H

#define GPIO_RINGER_AB		GPIO( 2, 0)

#define GPIO_WDOG		GPIO( 0, 1)		// WDOG

/* Board Revision */
#define GPIO_BOARD_REV0		GPIO(14, 5)
#define GPIO_BOARD_REV1		GPIO(14, 4)
#define GPIO_BOARD_REV2		GPIO(14, 3)
#define GPIO_BOARD_REV3		GPIO(14, 2)

/* which IICs to initialize */
#define IICS_MASK		(3)

#define TRISTAR_IIC_BUS		(0)

#define DISPLAY_PMU_IIC_BUS	(0)

/* D2094 watchdog tickle, 'KEEPACT' */
#define GPIO_WDOG_TICKLE	GPIO(2, 5)

#endif /* ! __TARGET_GPIODEF_H */
