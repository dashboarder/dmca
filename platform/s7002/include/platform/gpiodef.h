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
#ifndef __PLATFORM_GPIODEF_H
#define __PLATFORM_GPIODEF_H

/* S7002 specific gpio -> pin mappings */

#include <platform/gpio.h>
#include <platform/soc/hwregbase.h>

#if WITH_TARGET_CONFIG
# include <target/gpiodef.h>
#endif

/* define platform-specific gpios in a generic fashion here. */
#define GPIO_BOARD_ID0		GPIOC(GPIO_AP, 12,  1)
#define GPIO_BOARD_ID1		GPIOC(GPIO_AP, 12,  2)
#define GPIO_BOARD_ID2		GPIOC(GPIO_AP, 12,  3)
#define GPIO_BOARD_ID3		GPIOC(GPIO_AP,  4,  6)
#define GPIO_BERMUDA_TO_AP_CTS  GPIOC(GPIO_AP,  8,  1)

#define GPIO_BOOT_CONFIG0	GPIOC(GPIO_AP,  4,  2)
#define GPIO_BOOT_CONFIG1	GPIOC(GPIO_AP,  4,  3)
#define GPIO_BOOT_CONFIG2	GPIOC(GPIO_AP,  4,  4)
#define GPIO_BOOT_CONFIG3	GPIOC(GPIO_AP,  4,  5)

#define GPIO_BOARD_REV0		GPIOC(GPIO_AP,  4,  7)
#define GPIO_BOARD_REV1		GPIOC(GPIO_AP,  5,  0)
#define GPIO_BOARD_REV2		GPIOC(GPIO_AP,  5,  1)
#define GPIO_BOARD_REV3		GPIOC(GPIO_AP,  5,  2)

#define GPIO_REQUEST_DFU1	GPIOC(GPIO_SPU, 0,  0)
#define GPIO_REQUEST_DFU2	GPIOC(GPIO_SPU, 0,  1)
#define GPIO_FORCE_DFU		GPIOC(GPIO_SPU, 2,  6)
#define GPIO_DFU_STATUS		GPIOC(GPIO_SPU, 2,  5)
#define GPIO_DOCK_CONNECT	GPIOC(GPIO_SPU, 0,  2)
#define GPIO_DOCK_ATTENTION	GPIOC(GPIO_SPU, 3,  2)

#define SPI_NOR0		(0)

#define GPIO_SPI0_CS		GPIOC(GPIO_AP, 12, 4)

#define GPIO_IIC0_SCL		GPIOC(GPIO_AP, 11, 7)
#define GPIO_IIC0_SDA		GPIOC(GPIO_AP, 11, 6)
#define GPIO_IIC1_SCL		GPIOC(GPIO_AP,  6, 1)
#define GPIO_IIC1_SDA		GPIOC(GPIO_AP,  6, 0)

#endif /* ! __PLATFORM_GPIODEF_H */
