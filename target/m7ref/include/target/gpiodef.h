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

/* iPod6,x specific gpio -> pin mappings */

#include <platform/gpio.h>

/* define target-specific gpios in a generic fashion here. */
#define GPIO_ALT_BOOST_ID	GPIOC(GPIO_AP,  5, 3)		// GPIO11

/* which IICs to initialize */
#define IICS_MASK		(3)

#define DISPLAY_PMU_IIC_BUS	(1)

/* Miscellaneous Pins */
#define	GPIO_PMU_LCD_RST	(12)

/* 
 * support of video or command mode
 */
extern bool product_target_is_display_in_video_mode();
#define TARGET_DISP_VIDEO_MODE		product_target_is_display_in_video_mode()
#endif /* ! __TARGET_GPIODEF_H */
