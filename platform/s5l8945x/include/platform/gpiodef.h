/*
 * Copyright (C) 2010-2013 Apple Inc. All rights reserved.
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

/* S5L8945X specific gpio -> pin mappings */

#include <platform/gpio.h>
#include <platform/soc/hwregbase.h>

#if WITH_TARGET_CONFIG
# include <target/gpiodef.h>
#endif

/* define platform-specific gpios in a generic fashion here. */
#define GPIO_IIC0_SCL		GPIO(12, 4)
#define GPIO_IIC0_SDA		GPIO(12, 3)
#define GPIO_IIC1_SCL		GPIO(12, 6)
#define GPIO_IIC1_SDA		GPIO(12, 5)
#define GPIO_IIC2_SCL		GPIO(10, 1)
#define GPIO_IIC2_SDA		GPIO(10, 0)

#define GPIO_BOARD_ID0		GPIO( 1, 0)
#define GPIO_BOARD_ID1		GPIO( 1, 1)
#define GPIO_BOARD_ID2		GPIO( 1, 2)
#define GPIO_BOARD_ID3		GPIO( 6, 4)

#define GPIO_BOOT_CONFIG0	GPIO( 6, 6)
#define GPIO_BOOT_CONFIG1	GPIO( 7, 5)
#define GPIO_BOOT_CONFIG2	GPIO( 8, 0)
#define GPIO_BOOT_CONFIG3	GPIO( 8, 1)

#define GPIO_REQUEST_DFU1	GPIO( 4, 5)	// formerly known as HOLD_KEY
#define GPIO_REQUEST_DFU2	GPIO( 4, 4)	// formerly known as MENU_KEY
#define GPIO_FORCE_DFU		GPIO( 7, 6)
#define GPIO_DFU_STATUS		GPIO( 7, 7)

#define SPI_NOR0		(0)
#define SPI_NOR3		(3)

#if SUPPORT_FPGA
#define GPIO_SPI0_CS		GPIO(GPIO_PAD_SPI, 0)
#define GPIO_SPI3_CS		GPIO(GPIO_PAD_SPI, 3)
#else
#define GPIO_SPI0_CS		GPIO( 1, 3)
#define GPIO_SPI3_CS		GPIO(19, 5)
#endif

#define GPIO_UART1_TXD		GPIO(10, 2)

#define GPIO_SYSTEM_RESET	GPIO(17, 2)

#endif /* ! __PLATFORM_GPIODEF_H */
