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
#ifndef __PLATFORM_GPIODEF_S5L8940X_H
#define __PLATFORM_GPIODEF_S5L8940X_H

/* S5L8940X specific gpio -> pin mappings */

#if !SUB_PLATFORM_S5L8940X
#error "Only include for S5L8940X platform"
#endif

/* define platform-specific gpios in a generic fashion here. */
#define GPIO_IIC0_SCL		GPIO(21, 3)
#define GPIO_IIC0_SDA		GPIO(21, 2)
#define GPIO_IIC1_SCL		GPIO(21, 5)
#define GPIO_IIC1_SDA		GPIO(21, 4)
#define GPIO_IIC2_SCL		GPIO(20, 1)
#define GPIO_IIC2_SDA		GPIO(20, 0)

#define GPIO_BOARD_ID0		GPIO(20, 2)
#define GPIO_BOARD_ID1		GPIO(20, 3)
#define GPIO_BOARD_ID2		GPIO(20, 4)
#define GPIO_BOARD_ID3		GPIO( 2, 0)

#define GPIO_BOOT_CONFIG0	GPIO( 2, 2)
#define GPIO_BOOT_CONFIG1	GPIO( 3, 1)
#define GPIO_BOOT_CONFIG2	GPIO( 3, 4)
#define GPIO_BOOT_CONFIG3	GPIO( 3, 5)

#define GPIO_REQUEST_DFU1	GPIO( 0, 1)	// formerly known as HOLD_KEY
#define GPIO_REQUEST_DFU2	GPIO( 0, 0)	// formerly known as MENU_KEY
#define GPIO_FORCE_DFU		GPIO( 3, 2)
#define GPIO_DFU_STATUS		GPIO( 3, 3)

#define SPI_NOR0		(0)
#define SPI_NOR3		(3)

#if SUPPORT_FPGA
#define GPIO_SPI0_CS		GPIO(GPIO_PAD_SPI, 0)
#define GPIO_SPI3_CS		GPIO(GPIO_PAD_SPI, 3)
#else
#define GPIO_SPI0_CS		GPIO(20, 5)
#define GPIO_SPI3_CS		GPIO(18, 7)
#endif

#define GPIO_UART1_TXD		GPIO( 5, 3)

#define GPIO_SYSTEM_RESET	GPIO(28, 0)

#endif /* ! __PLATFORM_GPIODEF_S5L8940X_H */
