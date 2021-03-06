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
#ifndef __PLATFORM_GPIODEF_S5L8942X_H
#define __PLATFORM_GPIODEF_S5L8942X_H

/* S5L8942X specific gpio -> pin mappings */

#if !SUB_PLATFORM_S5L8942X
#error "Only include for S5L8942X platform"
#endif

/* define platform-specific gpios in a generic fashion here. */
#define GPIO_IIC0_SCL		GPIO( 6, 5)
#define GPIO_IIC0_SDA		GPIO( 6, 4)
#define GPIO_IIC1_SCL		GPIO( 6, 7)
#define GPIO_IIC1_SDA		GPIO( 6, 6)
#define GPIO_IIC2_SCL		GPIO(24, 1)
#define GPIO_IIC2_SDA		GPIO(24, 0)

#define GPIO_BOARD_ID0		GPIO(12, 0)
#define GPIO_BOARD_ID1		GPIO(12, 1)
#define GPIO_BOARD_ID2		GPIO(12, 2)
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
#define GPIO_SPI0_CS		GPIO(12, 3)
#define GPIO_SPI3_CS		GPIO(16, 3)
#endif

#define GPIO_UART1_TXD		GPIO( 5, 0)

#define GPIO_SYSTEM_RESET	GPIO(25, 0)

#endif /* ! __PLATFORM_GPIODEF_S5L8942X_H */
