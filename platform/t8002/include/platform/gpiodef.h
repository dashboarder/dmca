/*
 * Copyright (C) 2014 Apple Inc. All rights reserved.
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

/* T8002 specific gpio -> pin mappings */

#include <platform/gpio.h>
#include <platform/soc/hwregbase.h>

#if WITH_TARGET_CONFIG
# include <target/gpiodef.h>
#endif

/* define platform-specific gpios in a generic fashion here. */
#define GPIO_BOARD_ID0		GPIOC(GPIO_AP,  8,  1)
#define GPIO_BOARD_ID1		GPIOC(GPIO_AP,  8,  2)
#define GPIO_BOARD_ID2		GPIOC(GPIO_AP,  8,  3)
#define GPIO_BOARD_ID3		GPIOC(GPIO_AP,  0,  6)

#define GPIO_BOOT_CONFIG0	GPIOC(GPIO_AP,  0,  2)
#define GPIO_BOOT_CONFIG1	GPIOC(GPIO_AP,  0,  3)
#define GPIO_BOOT_CONFIG2	GPIOC(GPIO_AP,  0,  4)

#define BOOT_CONFIG_SPI0		(0)
#define BOOT_CONFIG_SPI0_TEST		(1)
#define BOOT_CONFIG_ANS			(2)
#define BOOT_CONFIG_ANS_TEST		(3)
#define BOOT_CONFIG_SLOW_SPI0_TEST	(6)

#define GPIO_BOARD_REV0		GPIOC(GPIO_AP,  0,  7)
#define GPIO_BOARD_REV1		GPIOC(GPIO_AP,  1,  0)
#define GPIO_BOARD_REV2		GPIOC(GPIO_AP,  1,  1)
#define GPIO_BOARD_REV3		GPIOC(GPIO_AP,  1,  2)

#define GPIO_REQUEST_DFU1	GPIOC(GPIO_AP, 12,  4)
#define GPIO_REQUEST_DFU2	GPIOC(GPIO_AP, 12,  5)
#define GPIO_FORCE_DFU		GPIOC(GPIO_AP, 12,  6)
#define GPIO_DFU_STATUS		GPIOC(GPIO_AP, 17,  0)
#define GPIO_DOCK_CONNECT	GPIOC(GPIO_AOP, 0,  6)
#define GPIO_DOCK_ATTENTION	GPIOC(GPIO_AOP, 0,  7)

#define SPI_NOR0		(0)

#define GPIO_SPI0_CS		GPIOC(GPIO_AP,  8, 4)
#define GPIO_SPI0_SSIN		GPIO_SPI0_CS
#define GPIO_SPI0_SCLK		GPIOC(GPIO_AP,  8, 1)
#define GPIO_SPI0_MOSI		GPIOC(GPIO_AP,	8, 2)
#define GPIO_SPI0_MISO		GPIOC(GPIO_AP,  8, 3)

#define GPIO_NAND_CEN0		GPIOC(GPIO_AP,  2, 5) 
#define GPIO_NAND_CEN1		GPIOC(GPIO_AP,  2, 6)
#define GPIO_NAND_CLE		GPIOC(GPIO_AP,  4, 1)
#define GPIO_NAND_ALE		GPIOC(GPIO_AP,  4, 2)
#define GPIO_NAND_REN		GPIOC(GPIO_AP,  3, 3)
#define GPIO_NAND_WEN		GPIOC(GPIO_AP,  4, 0)
#define GPIO_NAND_IO0		GPIOC(GPIO_AP,  3, 7)
#define GPIO_NAND_IO1		GPIOC(GPIO_AP,  3, 6)
#define GPIO_NAND_IO2		GPIOC(GPIO_AP,  3, 5)
#define GPIO_NAND_IO3		GPIOC(GPIO_AP,  3, 4)
#define GPIO_NAND_IO4		GPIOC(GPIO_AP,  3, 2)
#define GPIO_NAND_IO5		GPIOC(GPIO_AP,  3, 1)
#define GPIO_NAND_IO6		GPIOC(GPIO_AP,  3, 0)
#define GPIO_NAND_IO7		GPIOC(GPIO_AP,  2, 7)

#define GPIO_IIC0_SCL		GPIOC(GPIO_AP,  9, 2)
#define GPIO_IIC0_SDA		GPIOC(GPIO_AP,  9, 1)
#define GPIO_IIC1_SCL		GPIOC(GPIO_AP,  9, 4)
#define GPIO_IIC1_SDA		GPIOC(GPIO_AP,  9, 3)
#define GPIO_IIC2_SCL		GPIOC(GPIO_AP,  9, 6)
#define GPIO_IIC2_SDA		GPIOC(GPIO_AP,  9, 5)

#endif /* ! __PLATFORM_GPIODEF_H */
