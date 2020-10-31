/*
 * Copyright (C) 2012-2013 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */

#ifndef __PLATFORM_GPIODEF_T7000_H
#define __PLATFORM_GPIODEF_T7000_H

#if !SUB_PLATFORM_T7000
#error "Only include for T7000 platform"
#endif

/* T7000 specific gpio -> pin mappings - Fiji I/O Spreadsheet v9.3  */

/* define platform-specific gpios in a generic fashion here. */
#define GPIO_BOARD_ID0		GPIO( 7, 4)	// 60 - Alias of GPIO_SPI0_SCLK
#define GPIO_BOARD_ID1		GPIO( 7, 5)	// 61 - Alias of GPIO_SPI0_MOSI
#define GPIO_BOARD_ID2		GPIO( 7, 6)	// 62 - Alias of GPIO_SPI0_MISO
#define GPIO_BOARD_ID3		GPIO( 6, 4)	// 52
#define GPIO_BOARD_ID4		GPIO( 8, 6)	// 70

/* Boot configuration pins */
#define GPIO_BOOT_CONFIG0	GPIO( 6, 6)	// 54
#define GPIO_BOOT_CONFIG1	GPIO( 8, 4)	// 68
#define GPIO_BOOT_CONFIG2	GPIO( 8, 5)	// 69

/* Boot configuration pin encodings */
#define BOOT_CONFIG_SPI0		0
#define BOOT_CONFIG_SPI0_TEST		1
#define BOOT_CONFIG_ANS			2
#define BOOT_CONFIG_ANS_TEST		3
#define BOOT_CONFIG_NVME0		4
#define BOOT_CONFIG_NVME0_TEST		5
// #define BOOT_CONFIG_UNUSED		6
#define BOOT_CONFIG_FAST_SPI0_TEST	7

#define GPIO_REQUEST_DFU1	GPIO( 4, 1)	// 33 - formerly known as HOLD_KEY
#define GPIO_REQUEST_DFU2	GPIO( 4, 0)	// 32 - formerly known as MENU_KEY
#define GPIO_FORCE_DFU		GPIO(13, 4)	// 108
#define GPIO_DFU_STATUS		GPIO(13, 5)	// 109

#define SPI_NOR0		(0)

#define GPIO_SPI0_SCLK		GPIO( 7, 4)	// 60
#define GPIO_SPI0_MOSI		GPIO( 7, 5)	// 61
#define GPIO_SPI0_MISO		GPIO( 7, 6)	// 62
#define GPIO_SPI0_SSIN		GPIO( 7, 7)	// 63

#if SUPPORT_FPGA
#define GPIO_SPI0_CS		GPIO(GPIO_PAD_SPI, 0)
#else
#define GPIO_SPI0_CS		GPIO( 7, 7)	// 63 - Alias of GPIO_SPI0_SSIN
#endif

#define GPIO_IIC0_SCL		GPIO(12, 1)	// 97
#define GPIO_IIC0_SDA		GPIO(12, 0)	// 96
#define GPIO_IIC1_SCL		GPIO(17, 3)	// 139
#define GPIO_IIC1_SDA		GPIO(17, 2)	// 138
#define GPIO_IIC2_SCL		GPIO( 8, 1)	// 65
#define GPIO_IIC2_SDA		GPIO( 8, 0)	// 64
#define GPIO_IIC3_SCL		GPIO(10, 7)	// 87
#define GPIO_IIC3_SDA		GPIO(10, 6)	// 86

#define GPIO_PCIE0_PERST	GPIO( 9, 3)	// 75
#define GPIO_PCIE0_CLKREQ	GPIO( 5, 0)	// 40
#define GPIO_PCIE1_PERST	GPIO( 9, 5) 	// 77
#define GPIO_PCIE1_CLKREQ	GPIO( 5, 1) 	// 41

#define GPIO_S3E_RESETN		GPIO( 1, 1)	// 9
#define GPIO_NAND_SYS_CLK	GPIO( 5, 2)	// 42

#endif /* ! __PLATFORM_GPIODEF_T7000_H */
