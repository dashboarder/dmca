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

/*  maltese specific gpio -> pin mappings */
/*  TODO! change as actual schematic is available */

#include <platform/gpio.h>

/* define target-specific gpios in a generic fashion here. */

/* Board Revision */
#define GPIO_BOARD_REV0		GPIO( 20, 1)	//  161 : ULPI_NXT
#define GPIO_BOARD_REV1		GPIO( 19, 7)	//  159 : ULPI_DIR
#define GPIO_BOARD_REV2		GPIO( 21, 0)	//  168 : ULPI_DATA[2]
#define GPIO_BOARD_REV3		GPIO( 20, 3)	//  163 : ULPI_DATA[6]

/* which IICs to initialize */
#define IICS_MASK		(7)

#define BACKLIGHT_IIC_BUS	(0)
#define BACKLIGHT2_IIC_BUS	(1)
#define DISPLAY_PMU_IIC_BUS	(0)
#define TRISTAR_IIC_BUS		(0)

/* Display reset */
#define GPIO_LCD_RST            GPIO( 9, 4)	//    GPIO[12]               -> AP_TO_LCM_RESET_L
#define GPIO_LCD_RST_POLARITY   (0)		//  active low


#define GPIO_PMU_NAND_LOW_POWER_MODE	(5)	// PMU_TO_NAND_LOW_BATT_BOOT

#endif /* ! __TARGET_GPIODEF_H */
