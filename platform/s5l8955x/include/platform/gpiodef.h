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

/* S5L8955X specific gpio -> pin mappings */

#include <platform/gpio.h>
#include <platform/soc/hwregbase.h>

#if WITH_TARGET_CONFIG
# include <target/gpiodef.h>
#endif

/* define platform-specific gpios in a generic fashion here. */
#define GPIO_IIC0_SCL		GPIO(15, 1)
#define GPIO_IIC0_SDA		GPIO(15, 0)
#define GPIO_IIC1_SCL		GPIO(15, 3)
#define GPIO_IIC1_SDA		GPIO(15, 2)
#define GPIO_IIC2_SCL		GPIO(28, 5)
#define GPIO_IIC2_SDA		GPIO(28, 4)

#define GPIO_BOARD_ID0		GPIO(14, 0)
#define GPIO_BOARD_ID1		GPIO(14, 1)
#define GPIO_BOARD_ID2		GPIO(14, 2)
#define GPIO_BOARD_ID3		GPIO( 4, 6)

#define GPIO_BOOT_CONFIG0	GPIO( 5, 0)
#define GPIO_BOOT_CONFIG1	GPIO(24, 5)
#define GPIO_BOOT_CONFIG2	GPIO(25, 0)
#define GPIO_BOOT_CONFIG3	GPIO(25, 1)

#define GPIO_REQUEST_DFU1	GPIO( 0, 1)	// formerly known as HOLD_KEY
#define GPIO_REQUEST_DFU2	GPIO( 0, 0)	// formerly known as MENU_KEY
#define GPIO_FORCE_DFU		GPIO(24, 6)
#define GPIO_DFU_STATUS		GPIO(24, 7)

#define SPI_NOR0		(0)
#define SPI_NOR3		(3)

#if SUPPORT_FPGA
// XXX Need to understand this and possibly correct it.
#define GPIO_SPI0_CS		GPIO(GPIO_PAD_SPI, 0)
#define GPIO_SPI3_CS		GPIO(GPIO_PAD_SPI, 3)
#else
#define GPIO_SPI0_CS		GPIO(14, 3)
#define GPIO_SPI3_CS		GPIO(28, 3)
#endif

#define GPIO_UART1_TXD		GPIO( 5, 2)

#define GPIO_SYSTEM_RESET	GPIO(30, 0)

#endif /* ! __PLATFORM_GPIODEF_H */
