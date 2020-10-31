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

/* j105 specific gpio -> pin mappings */

#include <platform/gpio.h>

/* define target-specific gpios in a generic fashion here. */

/* Board Revision */
#define GPIO_BOARD_REV0		GPIO( 26, 0)	//  208 : GPIO[34]		-> SOC_BRD_REV0
#define GPIO_BOARD_REV1		GPIO( 26, 1)	//  209 : GPIO[35]		-> SOC_BRD_REV1
#define GPIO_BOARD_REV2		GPIO( 26, 2)	//  210 : GPIO[36]		-> SOC_BRD_REV2
#define GPIO_BOARD_REV3		GPIO( 26, 3)	//  211 : GPIO[37]		-> SOC_BRD_REV3

#define GPIO_S3E_BOOT_FROM_HOST	GPIO_ID(31)	// 31 : GPIO[43] -> SOC_NAND_FW_STRAP

/* which IICs to initialize */
#define IICS_MASK		(0xF) // i2c0, i2c1, i2c2, i2c3

#endif /* ! __TARGET_GPIODEF_H */
