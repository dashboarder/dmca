/*
 * Copyright (C) 20124 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */

#ifndef __PLATFORM_GPIODEF_S8001_H
#define __PLATFORM_GPIODEF_S8001_H

#if !SUB_PLATFORM_S8001
#error "Only include for S8001 platform"
#endif

/* S8001 specific gpio -> pin mappings - Elba I/O Spreadsheet v.27  */

/* define platform-specific gpios in a generic fashion here. */
#define GPIO_BOARD_ID0			GPIO(94)
#define GPIO_BOARD_ID1			GPIO(95)
#define GPIO_BOARD_ID2			GPIO(96)
#define GPIO_BOARD_ID3			GPIO(16)
#define GPIO_BOARD_ID4			GPIO(203)

/* Boot configuration pins */
#define GPIO_BOOT_CONFIG0		GPIO(18)
#define GPIO_BOOT_CONFIG1		GPIO(185)
#define GPIO_BOOT_CONFIG2		GPIO(202)

/* Boot configuration pin encodings */
#define BOOT_CONFIG_SPI0		(0)
#define BOOT_CONFIG_SPI0_TEST		(1)
#define BOOT_CONFIG_NVME0_X2		(2)
#define BOOT_CONFIG_NVME0_X2_TEST	(3)
#define BOOT_CONFIG_NVME0_X1		(4)
#define BOOT_CONFIG_NVME0_X1_TEST	(5)
#define BOOT_CONFIG_SLOW_SPI0_TEST	(6)
#define BOOT_CONFIG_FAST_SPI0_TEST	(7)

#define GPIO_REQUEST_DFU1		GPIO(123)	// HOLD_KEY
#define GPIO_REQUEST_DFU2		GPIO(122)	// MENU_KEY
#define GPIO_FORCE_DFU			GPIO(186)	// FORCE_DFU
#define GPIO_DFU_STATUS			GPIO(201)	// DFU_STATUS

#define SPI_NOR0			(0)

#define GPIO_SPI0_SCLK			GPIO(94)
#define GPIO_SPI0_MOSI			GPIO(95)
#define GPIO_SPI0_MISO			GPIO(96)
#define GPIO_SPI0_SSIN			GPIO(97)

#if SUPPORT_FPGA
#define GPIO_SPI0_CS			GPIO(GPIO_PAD_SPI, 0)
#else
#define GPIO_SPI0_CS			GPIO(97)	// Alias of GPIO_SPI0_SSIN
#endif

#define GPIO_IIC0_SCL			GPIO(165)
#define GPIO_IIC0_SDA			GPIO(164)
#define GPIO_IIC1_SCL			GPIO(178)
#define GPIO_IIC1_SDA			GPIO(177)
#define GPIO_IIC2_SCL			GPIO(132)
#define GPIO_IIC2_SDA			GPIO(131)
#define GPIO_IIC3_SCL			GPIO(115)
#define GPIO_IIC3_SDA			GPIO(114)

#define GPIO_PCIE0_PERST		GPIO(98)
#define GPIO_PCIE1_PERST		GPIO(99)
#define GPIO_PCIE2_PERST		GPIO(100)
#define GPIO_PCIE3_PERST		GPIO(101)
#define GPIO_PCIE4_PERST		GPIO(102)
#define GPIO_PCIE5_PERST		GPIO(103)
#define GPIO_PCIE0_CLKREQ		GPIO(104)
#define GPIO_PCIE1_CLKREQ		GPIO(105)
#define GPIO_PCIE2_CLKREQ		GPIO(106)
#define GPIO_PCIE3_CLKREQ		GPIO(107)
#define GPIO_PCIE4_CLKREQ		GPIO(108)
#define GPIO_PCIE5_CLKREQ		GPIO(109)

#define GPIO_NAND_SYS_CLK		GPIO(24)

#endif /* ! __PLATFORM_GPIODEF_S8001_H */
