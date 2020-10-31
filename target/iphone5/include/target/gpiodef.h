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

/* iPhone5,x specific gpio -> pin mappings */

#include <platform/gpio.h>

/* define target-specific gpios in a generic fashion here. */

#define GPIO_LCD_RST		GPIO(25, 2)
#define GPIO_LCD_RST_POLARITY	(0)
#define GPIO_LCD_CHKSUM		GPIO( 4, 7)

#define GPIO_RINGER_AB		GPIO( 0, 4)

/* Board Revision */
#define GPIO_BOARD_REV0		GPIO( 2, 4)
#define GPIO_BOARD_REV1		GPIO( 2, 5)
#define GPIO_BOARD_REV2		GPIO( 2, 6)
#define GPIO_BOARD_REV3		GPIO( 2, 7)

/* Grape ID */
#define GPIO_GRAPE_ID0		GPIO( 1, 5)
#define GPIO_GRAPE_ID1		GPIO( 1, 4)
#define GPIO_GRAPE_ID2		GPIO( 1, 3)

/* which IICs to initialize */
#define IICS_MASK		(7)

#define TRISTAR_IIC_BUS		(0)

/* D1972 watchdog tickle, 'KEEPACT' */
#define GPIO_WDOG_TICKLE	GPIO(2, 3)

// Configure both FMI0 & FMI1 to use differential DQS with DQ set to DQVREF
#define FMI_DIFF_SEL (0x36)

#endif /* ! __TARGET_GPIODEF_H */
