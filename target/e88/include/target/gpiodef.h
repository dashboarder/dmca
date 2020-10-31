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

/* E88 specific gpio -> pin mappings */

#include <platform/gpio.h>

/* define target-specific gpios in a generic fashion here. */
#define GPIO_RINGER_AB		GPIO( 0, 4)

/* Board Revision */
#define GPIO_BOARD_REV0		GPIO( 4, 5)  // EHCI_PORT_PWR0
#define GPIO_BOARD_REV1		GPIO( 4, 6)  // EHCI_PORT_PWR1
#define GPIO_BOARD_REV2		GPIO( 4, 7)  // EHCI_PORT_PWR2
#define GPIO_BOARD_REV3		GPIO( 2, 1)  // GPIO17

/* which IICs to initialize */
#define IICS_MASK		(5)

#define TRISTAR_IIC_BUS		(0)

/* D1881 watchdog tickle, 'KEEPACT' */
#define GPIO_WDOG_TICKLE	GPIO(2, 3)

#endif /* ! __TARGET_GPIODEF_H */
