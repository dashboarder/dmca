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

#ifndef __PLATFORM_GPIODEF_S8000_H
#define __PLATFORM_GPIODEF_S8000_H

#if !SUB_PLATFORM_S8000 && !SUB_PLATFORM_S8003
#error "Only include for S8000 platform"
#endif

/* S8000 specific gpio -> pin mappings - Fiji I/O Spreadsheet v9.3  */

/* define platform-specific gpios in a generic fashion here. */
#define GPIO_BOARD_ID0			GPIO(103)
#define GPIO_BOARD_ID1			GPIO(104)
#define GPIO_BOARD_ID2			GPIO(105)
#define GPIO_BOARD_ID3			GPIO(80)
#define GPIO_BOARD_ID4			GPIO(138)

/* Boot configuration pins */
#define GPIO_BOOT_CONFIG0		GPIO(82)
#define GPIO_BOOT_CONFIG1		GPIO(122)
#define GPIO_BOOT_CONFIG2		GPIO(137)

/* Boot configuration pin encodings */
#define BOOT_CONFIG_SPI0		(0)
#define BOOT_CONFIG_SPI0_TEST		(1)
#define BOOT_CONFIG_NVME0_X2		(2)
#define BOOT_CONFIG_NVME0_X2_TEST	(3)
#define BOOT_CONFIG_NVME0_X1		(4)
#define BOOT_CONFIG_NVME0_X1_TEST	(5)
#define BOOT_CONFIG_SLOW_SPI0_TEST	(6)
#define BOOT_CONFIG_FAST_SPI0_TEST	(7)

#define GPIO_REQUEST_DFU1		GPIO(97)	// HOLD_KEY
#define GPIO_REQUEST_DFU2		GPIO(96)	// MENU_KEY
#define GPIO_FORCE_DFU			GPIO(123)	// FORCE_DFU
#define GPIO_DFU_STATUS			GPIO(136)	// DFU_STATUS

#define SPI_NOR0			(0)

#define GPIO_SPI0_SCLK			GPIO(103)
#define GPIO_SPI0_MOSI			GPIO(104)
#define GPIO_SPI0_MISO			GPIO(105)
#define GPIO_SPI0_SSIN			GPIO(106)

#if SUPPORT_FPGA
#define GPIO_SPI0_CS			GPIO(GPIO_PAD_SPI, 0)
#else
#define GPIO_SPI0_CS			GPIO(106)	// Alias of GPIO_SPI0_SSIN
#endif

#define GPIO_IIC0_SCL			GPIO(46)
#define GPIO_IIC0_SDA			GPIO(45)
#define GPIO_IIC1_SCL			GPIO(115)
#define GPIO_IIC1_SDA			GPIO(114)
#define GPIO_IIC2_SCL			GPIO(23)
#define GPIO_IIC2_SDA			GPIO(22)

#define GPIO_PCIE0_PERST		GPIO(160)
#define GPIO_PCIE1_PERST		GPIO(161)
#define GPIO_PCIE2_PERST		GPIO(162)
#define GPIO_PCIE3_PERST		GPIO(163)
#define GPIO_PCIE0_CLKREQ		GPIO(164)
#define GPIO_PCIE1_CLKREQ		GPIO(165)
#define GPIO_PCIE2_CLKREQ		GPIO(166)
#define GPIO_PCIE3_CLKREQ		GPIO(167)

#define GPIO_NAND_SYS_CLK		GPIO(196)

#endif /* ! __PLATFORM_GPIODEF_S8000_H */
