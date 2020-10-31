/*
 * Copyright (C) 2012 Apple Inc. All rights reserved.
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

/* J33i specific gpio -> pin mappings */

#include <platform/gpio.h>

/* define target-specific gpios in a generic fashion here. */

#define GPIO_BOARD_REV0		GPIO(12, 3)
#define GPIO_BOARD_REV1		GPIO(12, 4)
#define GPIO_BOARD_REV2		GPIO(12, 5)
#define GPIO_BOARD_REV3		GPIO(12, 6)

#define	HDMI_DDC_IIC_BUS	2

#endif /* ! __TARGET_GPIODEF_H */
