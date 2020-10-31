/*
 * Copyright (C) 2011-2012 Apple Inc. All rights reserved.
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

/* S5L8747X specific gpio -> pin mappings */

#include <platform/gpio.h>
#include <platform/soc/hwregbase.h>

#if WITH_TARGET_CONFIG
# include <target/gpiodef.h>
#endif

/* define platform-specific gpios in a generic fashion here. */
#define GPIO_IIC0_SCL		GPIO( 1, 1)
#define GPIO_IIC0_SDA		GPIO( 1, 2)
#define GPIO_IIC1_SCL		GPIO( 3, 6)
#define GPIO_IIC1_SDA		GPIO( 3, 7)

#define GPIO_BOARD_ID0		GPIO( 2, 3)
#define GPIO_BOARD_ID1		GPIO( 2, 4)
#define GPIO_BOARD_ID2		GPIO( 2, 5)
#define GPIO_BOARD_ID3		GPIO( 2, 6)

#define GPIO_BOOT_CONFIG0	GPIO( 2, 1)
#define GPIO_BOOT_CONFIG1	GPIO( 2, 2)

#define GPIO_FORCE_DFU		GPIO(3, 4)
#define GPIO_DFU_STATUS		GPIO(3, 5)

#define GPIO_CLK0_OUT		GPIO( 3, 0)

#define SPI_NOR0		(0)
#define SPI_NOR1		(1)

#if SUPPORT_FPGA
#define GPIO_SPI0_CS		GPIO(GPIO_PAD_SPI, 0)
#define GPIO_SPI1_CS		GPIO(GPIO_PAD_SPI, 1)
#else
#define GPIO_SPI0_CS		GPIO( 1, 3)
#define GPIO_SPI3_CS		GPIO( 4, 4)
#endif

#endif /* ! __PLATFORM_GPIODEF_H */
