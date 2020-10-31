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

/* P101/P102/P103 specific gpio -> pin mappings */

#include <platform/gpio.h>

/* define target-specific gpios in a generic fashion here. */

#define GPIO_LCD_PWR_EN		GPIO(26, 0)

#define GPIO_RINGER_AB		GPIO(25, 2)

/* Board Revision */
#define GPIO_BOARD_REV0		GPIO( 4, 2)
#define GPIO_BOARD_REV1		GPIO( 4, 3)
#define GPIO_BOARD_REV2		GPIO( 4, 4)

/* which IICs to initialize */
#define IICS_MASK		(7)

#define TRISTAR_IIC_BUS		(0)

/* PMU watchdog tickle; connected to GPIO_PMU_KEEPACT */
#define GPIO_WDOG_TICKLE	GPIO(24, 2)

// Configure both FMI0 & FMI1 to use differential DQS with DQ set to DQVREF
#define FMI_DIFF_SEL (0x36)

#endif /* ! __TARGET_GPIODEF_H */
