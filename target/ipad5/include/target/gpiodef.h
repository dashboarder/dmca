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
#define GPIO_BOARD_REV0		GPIO( 14, 2)	//  118 : GPIO[8]
#define GPIO_BOARD_REV1		GPIO( 14, 3)	//  117 : GPIO[9]
#define GPIO_BOARD_REV2		GPIO( 14, 4)	//  116 : GPIO[10]

/* Miscellaneous Pins */
#define	GPIO_PMU_LCD_PWR_EN	(17)

/* Ringer Switch */
#define GPIO_RINGER_AB		GPIO( 5 , 7)	// 131 : GPIO[4] -> gpio_btn_srl_l

/* D2207 watchdog tickle, 'KEEPACT' */
#define GPIO_WDOG_TICKLE	GPIO( 6, 7)	//  55 : GPIO[20] -> gpio_soc2pmu_keepact

/* which IICs to initialize */
#define IICS_MASK		(0xD)

#define TRISTAR_IIC_BUS		(0)

#define BACKLIGHT_IIC_BUS	(0)

#endif /* ! __TARGET_GPIODEF_H */
