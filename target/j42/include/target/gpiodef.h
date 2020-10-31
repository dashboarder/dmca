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

/* j42 specific gpio -> pin mappings */

#include <platform/gpio.h>

/* define target-specific gpios in a generic fashion here. */

/* Board Revision */
#define GPIO_BOARD_REV0		GPIO( 9, 2)	//  74 : GPIO[37]		-> GPIO_BRD_REV0
#define GPIO_BOARD_REV1		GPIO( 9, 1)	//  73 : GPIO[36]		-> GPIO_BRD_REV1
#define GPIO_BOARD_REV2		GPIO( 9, 0)	//  72 : GPIO[35]		-> GPIO_BRD_REV2
#define GPIO_BOARD_REV3		GPIO( 8, 7)	//  71 : GPIO[34]		-> GPIO_BRD_REV3

/* D2186 watchdog tickle, 'KEEPACT' */
#define GPIO_WDOG_TICKLE	GPIO( 8, 2)	//  66 : GPIO[22]		-> GPIO_SOC2PMU_KEEPACT

/* which IICs to initialize */
#define IICS_MASK		(0xD) // i2c0, i2c2, i2c3

#define TRISTAR_IIC_BUS		(2)

#define	POWER_GPIO_HVR_RESET_L	(5)

#define	POWER_GPIO_VBUS_PROT_2V5	(19)

#endif /* ! __TARGET_GPIODEF_H */
