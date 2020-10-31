/*
 * Copyright (C) 2011-2013 Apple Inc. All rights reserved.
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

/* S5L8960X specific gpio -> pin mappings */

#include <platform/gpio.h>
#include <platform/soc/hwregbase.h>

#if WITH_TARGET_CONFIG
# include <target/gpiodef.h>
#endif

/* define platform-specific gpios in a generic fashion here. */
#define GPIO_BOARD_ID0		GPIO( 9, 0)
#define GPIO_BOARD_ID1		GPIO( 9, 1)
#define GPIO_BOARD_ID2		GPIO( 9, 2)
#define GPIO_BOARD_ID3		GPIO( 2, 2)

#define GPIO_BOOT_CONFIG0	GPIO( 2, 4)
#define GPIO_BOOT_CONFIG1	GPIO(13, 1)
#define GPIO_BOOT_CONFIG2	GPIO(13, 4)
#define GPIO_BOOT_CONFIG3	GPIO(13, 5)

#define GPIO_REQUEST_DFU1	GPIO( 0, 3)	// formerly known as HOLD_KEY
#define GPIO_REQUEST_DFU2	GPIO( 0, 2)	// formerly known as MENU_KEY
#define GPIO_FORCE_DFU		GPIO(13, 2)
#define GPIO_DFU_STATUS		GPIO(13, 3)

#define SPI_NOR0		(0)

#if SUPPORT_FPGA
#define GPIO_SPI0_CS		GPIO(GPIO_PAD_SPI, 0)
#else
#define GPIO_SPI0_CS		GPIO( 9, 3)
#endif

#define GPIO_IIC0_SCL		GPIO(10, 1)
#define GPIO_IIC0_SDA		GPIO(10, 0)
#define GPIO_IIC1_SCL		GPIO(10, 3)
#define GPIO_IIC1_SDA		GPIO(10, 2)
#define GPIO_IIC2_SCL		GPIO(12, 5)
#define GPIO_IIC2_SDA		GPIO(12, 4)
#define GPIO_IIC3_SCL		GPIO(21, 4)
#define GPIO_IIC3_SDA		GPIO(21, 3)

#endif /* ! __PLATFORM_GPIODEF_H */
