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

/* J33 specific gpio -> pin mappings */

#include <platform/gpio.h>

/* define target-specific gpios in a generic fashion here. */

#define GPIO_BOARD_REV0		GPIO(4, 5)
#define GPIO_BOARD_REV1		GPIO(4, 6)
#define GPIO_BOARD_REV2		GPIO(4, 7)
#define GPIO_BOARD_REV3		GPIO(7, 5)

#endif /* ! __TARGET_GPIODEF_H */
