/*
 * Copyright (C) 2013 Apple Inc. All rights reserved.
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

/* ipad5,x specific gpio -> pin mappings */

#include <platform/gpio.h>

/* define target-specific gpios in a generic fashion here. */

/* Board Revision */
#define GPIO_BOARD_REV0		GPIO( 20, 1)	//  161 : ULPI_NXT
#define GPIO_BOARD_REV1		GPIO( 19, 7)	//  159 : ULPI_DIR
#define GPIO_BOARD_REV2		GPIO( 21, 0)	//  168 : ULPI_DATA[2]
#define GPIO_BOARD_REV3		GPIO( 20, 3)	//  163 : ULPI_DATA[6]

/* Miscellaneous Pins */
#define	GPIO_PMU_LCD_PWR_EN	(17)

/* D2207 watchdog tickle, 'KEEPACT' */
#define GPIO_WDOG_TICKLE	GPIO( 10, 7)	//  87 : GPIO[7] -> GPIO_SOC2PMU_KEEPACT

/* Enable for GPU UVD */
#define GPIO_SOC2GPUUVD_EN	GPIO( 10, 0)

/* which IICs to initialize */
#define IICS_MASK		(0xF)

#define TRISTAR_IIC_BUS		(0)

#define BACKLIGHT_IIC_BUS	(0)

#endif /* ! __TARGET_GPIODEF_H */
