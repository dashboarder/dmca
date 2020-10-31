/*
 * Copyright (C) 2012-2014 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */

#ifndef __PLATFORM_GPIODEF_t8010_H
#define __PLATFORM_GPIODEF_t8010_H

#if !SUB_PLATFORM_T8010
#error "Only include for t8010 platform"
#endif

/* t8010 specific gpio -> pin mappings - Cayman I/O Spreadsheet v2.05  */

/* Boot configuration pins */
#define GPIO_BOOT_CONFIG0		GPIO(167)
#define GPIO_BOOT_CONFIG1		GPIO(204)
#define GPIO_BOOT_CONFIG2		GPIO(207)

/* Boot configuration pin encodings */
#define BOOT_CONFIG_SPI0		(0)
#define BOOT_CONFIG_SPI0_TEST		(1)
#define BOOT_CONFIG_NVME0_X2		(2)
#define BOOT_CONFIG_NVME0_X2_TEST	(3)
#define BOOT_CONFIG_NVME0_X1		(4)
#define BOOT_CONFIG_NVME0_X1_TEST	(5)
#define BOOT_CONFIG_SLOW_SPI0_TEST	(6)
#define BOOT_CONFIG_FAST_SPI0_TEST	(7)

#define GPIO_REQUEST_DFU1		GPIO(179)	// Formerly HOLD_KEY
#define GPIO_REQUEST_DFU2		GPIO(180)	// Formerly MENU_KEY
#define GPIO_FORCE_DFU			GPIO(205)	// FORCE_DFU
#define GPIO_DFU_STATUS			GPIO(206)	// DFU_STATUS

#define SPI_NOR0			(0)

#define GPIO_SPI0_SCLK			GPIO(184)
#define GPIO_SPI0_MOSI			GPIO(185)
#define GPIO_SPI0_MISO			GPIO(186)
#define GPIO_SPI0_SSIN			GPIO(187)

#if SUPPORT_FPGA
#define GPIO_SPI0_CS			GPIO(GPIO_PAD_SPI, 0)
#else
#define GPIO_SPI0_CS			GPIO(187)	// Alias of GPIO_SPI0_SSIN
#endif

#define GPIO_IIC0_SCL			GPIO(197)
#define GPIO_IIC0_SDA			GPIO(196)
#define GPIO_IIC1_SCL			GPIO(40)
#define GPIO_IIC1_SDA			GPIO(39)
#define GPIO_IIC2_SCL			GPIO(133)
#define GPIO_IIC2_SDA			GPIO(132)
#define GPIO_IIC3_SCL			GPIO(42)
#define GPIO_IIC3_SDA			GPIO(41)

#define GPIO_PCIE0_PERST		GPIO(12)
#define GPIO_PCIE1_PERST		GPIO(13)
#define GPIO_PCIE2_PERST		GPIO(14)
#define GPIO_PCIE3_PERST		GPIO(15)
#define GPIO_PCIE0_CLKREQ		GPIO(16)
#define GPIO_PCIE1_CLKREQ		GPIO(17)
#define GPIO_PCIE2_CLKREQ		GPIO(18)
#define GPIO_PCIE3_CLKREQ		GPIO(19)

#define GPIO_NAND_SYS_CLK		GPIO(10)

#endif /* ! __PLATFORM_GPIODEF_t8010_H */
