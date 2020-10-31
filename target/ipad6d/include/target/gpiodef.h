/*
 * Copyright (C) 2015 Apple Inc. All rights reserved.
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

/* ipad6,x specific gpio -> pin mappings */

#include <platform/gpio.h>

// XXX: Update all of this

/* define target-specific gpios in a generic fashion here. */

/* Board Revision */
#define GPIO_BOARD_REV0		GPIO(14, 4)	// 116 : GPIO[44]
#define GPIO_BOARD_REV1		GPIO(14, 5)	// 117 : GPIO[45]
#define GPIO_BOARD_REV2		GPIO(14, 6)	// 118 : GPIO[46]
#define GPIO_BOARD_REV3		GPIO(15, 1)	// 121 : GPIO[49]

/* Miscellaneous Pins */
#define GPIO_DISPLAY_HPD	GPIO(24, 2)	// 194: EPD_HPD0 -> LPDP_TCON_TO_SOC_HPD

#define GPIO_S3E_BOOT_FROM_HOST	GPIO_ID(76)	// 76 : GPIO[2] -> GPIO_SOC_TO_NAND_FW_STRAP (formerly AP_TO_NAND_FW_STRAP)


/* which IICs to initialize */
#define IICS_MASK		(0xF)

#define TRISTAR_IIC_BUS		(0)
#define BACKLIGHT_IIC_BUS	(0)
#define DISPLAY_PMU_IIC_BUS	(0)

#define GPIO_PMU_LCD_PWR_EN          (17)	// PMU GPIO 18 - GPIO_PMU_TO_LCD_PWREN
#define GPIO_PMU_NAND_LOW_POWER_MODE (15)	// PMU GPIO 16 - GPIO_PMU_TO_NAND_LOW_BATT_L

#endif /* ! __TARGET_GPIODEF_H */
