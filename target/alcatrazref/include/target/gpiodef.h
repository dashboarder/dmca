/*
 * Copyright (C) 2012-2013 Apple Inc. All rights reserved.
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

/* iRef,x specific gpio -> pin mappings */

#include <platform/gpio.h>

/* define target-specific gpios in a generic fashion here. */

/* define target-specific gpios in a generic fashion here. */
#define GPIO_RINGER_AB		GPIO( 2, 0)

#define GPIO_WDOG		GPIO( 0, 1)		// WDOG

/* D2094 watchdog tickle, 'KEEPACT' */
#define GPIO_WDOG_TICKLE	GPIO(2, 5)

#endif /* ! __TARGET_GPIODEF_H */
