/*
 * Copyright (C) 2013-2014 Apple Inc. All rights reserved.
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

/* iphone8,x specific gpio -> pin mappings */

#include <platform/gpio.h>

/* define target-specific gpios in a generic fashion here. */

/* Board Revision */
#define GPIO_BOARD_REV0		GPIO_ID(146)	//  74 : GPIO[34]
#define GPIO_BOARD_REV1		GPIO_ID(145)	//  73 : GPIO[35]
#define GPIO_BOARD_REV2		GPIO_ID(144)	//  72 : GPIO[36]
#define GPIO_BOARD_REV3		GPIO_ID(143)	//  71 : GPIO[37]

/* Miscellaneous Pins */
#define GPIO_DISPLAY_ID1	GPIO( 4, 0)	//  32 : UART7_TXD -> DISPLAY_TO_AP_ID0 (DEV BOARD only)
#define GPIO_DISPLAY_ID0	GPIO( 4, 1)	//  33 : UART7_RXD -> DISPLAY_TO_AP_ID1 (DEV BOARD only)
#define GPIO_PROX_SELECT	GPIO( 4, 1)	//  32 : UART7_RXD -> PROX_SELECT       (AP BOARD only)

/* Display reset */
#define GPIO_LCD_RST            GPIO( 9, 4)	//    GPIO[12]               -> AP_TO_LCM_RESET_L
#define GPIO_LCD_RST_POLARITY   (0)		//  active low

/* Ringer Switch */
#define GPIO_RINGER_AB		GPIO(16, 3)	// 131 : GPIO[40] -> BUTTON_TO_AP_RINGER_A

#define GPIO_S3E_BOOT_FROM_HOST	GPIO_ID(141)	// 141 : GPIO[32] -> AP_TO_NAND_FW_STRAP

/* which IICs to initialize */
#define IICS_MASK		(7)

#define TRISTAR_IIC_BUS		(1)

#define DISPLAY_PMU_IIC_BUS	(0)

#define GPIO_PMU_NAND_LOW_POWER_MODE	(5)	// PMU_TO_NAND_LOW_BATT_BOOT

#endif /* ! __TARGET_GPIODEF_H */
