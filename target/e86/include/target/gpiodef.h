/*
 * Copyright (C) 2014 Apple Inc. All rights reserved.
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

/* iphone7,x specific gpio -> pin mappings */

#include <platform/gpio.h>

/* define target-specific gpios in a generic fashion here. */

/* Board Revision */
#define GPIO_BOARD_REV0		GPIO( 9, 2)     //  74 : GPIO[34]
#define GPIO_BOARD_REV1		GPIO( 9, 1)     //  73 : GPIO[35]
#define GPIO_BOARD_REV2		GPIO( 9, 0)     //  72 : GPIO[36]
#define GPIO_BOARD_REV3		GPIO( 8, 7)     //  71 : GPIO[37]

/* Display reset */
#define GPIO_LCD_RST            GPIO(0, 1)      //  1 : ULPI_STP -> AP_TO_LCM_RESET_L
#define GPIO_LCD_RST_POLARITY   (0)             //  active low

/* D2186 watchdog tickle, 'KEEPACT' */
#define GPIO_WDOG_TICKLE        GPIO(13, 3)     // 107 : GPIO[19] -> AP_TO_PMU_KEEPACT

/* which IICs to initialize */
#define IICS_MASK               (1)

#define TRISTAR_IIC_BUS         (0)
#define DISPLAY_PMU_IIC_BUS     (0)

#define	POWER_GPIO_HVR_RESET_L	(5)

#endif /* ! __TARGET_GPIODEF_H */
