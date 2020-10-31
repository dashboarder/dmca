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

/* B238 specific gpio -> pin mappings */

#include <platform/gpio.h>

/* define target-specific gpios in a generic fashion here. */

#define GPIO_BOARD_REV0		GPIO( 9, 2)	//  74 : GPIO[34]
#define GPIO_BOARD_REV1		GPIO( 9, 1)	//  73 : GPIO[35]
#define GPIO_BOARD_REV2		GPIO( 9, 0)	//  72 : GPIO[36]
#define GPIO_BOARD_REV3		GPIO( 8, 7)	//  71 : GPIO[37]

/* D2186 watchdog tickle, 'KEEPACT' */
#define GPIO_WDOG_TICKLE	GPIO(13, 3)	// 107 : GPIO[19] -> AP_TO_PMU_KEEPACT

/* which IICs to initialize */
// for t7000/fiji IICS_COUNT is 4.   
// if IICS_MASK is not defined it is set to 0xF
// iic0 - ADI PMU
// iic1 - Accel
// iic2 - Mic Codec
// iic3 - Nitrogen
#define IICS_MASK		(0xF)

#endif /* ! __TARGET_GPIODEF_H */
