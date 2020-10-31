/*
 * Copyright (C) 2012 Apple Inc. All rights reserved.
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

/* CapriRef specific gpio -> pin mappings */

#include <platform/gpio.h>

/* define target-specific gpios in a generic fashion here. */

/* Board Revision */
#define GPIO_BOARD_REV0		GPIO( 20, 1)	//  161 : ULPI_NXT
#define GPIO_BOARD_REV1		GPIO( 19, 7)	//  159 : ULPI_DIR
#define GPIO_BOARD_REV2		GPIO( 21, 0)	//  168 : ULPI_DATA[2]
#define GPIO_BOARD_REV3		GPIO( 20, 3)	//  163 : ULPI_DATA[6]

/* D2207 watchdog tickle, 'KEEPACT' */
#define GPIO_WDOG_TICKLE	GPIO( 10, 7)	//  87 : GPIO[7] -> GPIO_SOC2PMU_KEEPACT

#endif /* ! __TARGET_GPIODEF_H */
