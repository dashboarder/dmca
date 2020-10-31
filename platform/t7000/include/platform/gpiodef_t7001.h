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

#ifndef __PLATFORM_GPIODEF_T7001_H
#define __PLATFORM_GPIODEF_T7001_H

#if !SUB_PLATFORM_T7001
#error "Only include for T7001 platform"
#endif

/* T7001 specific gpio -> pin mappings - Capri I/O Spreadsheet v10.29 */

/* define platform-specific gpios in a generic fashion here. */
#define GPIO_BOARD_ID0		GPIO( 8, 3)	// 67 - Alias of GPIO_SPI0_SCLK
#define GPIO_BOARD_ID1		GPIO( 8, 4)	// 68 - Alias of GPIO_SPI0_MOSI
#define GPIO_BOARD_ID2		GPIO( 8, 5)	// 69 - Alias of GPIO_SPI0_MISO
#define GPIO_BOARD_ID3		GPIO( 12, 0)	// 96
#define GPIO_BOARD_ID4		GPIO( 13, 5)	// 109

/* Boot configuration pins */
#define GPIO_BOOT_CONFIG0	GPIO( 12, 2)	// 98
#define GPIO_BOOT_CONFIG1	GPIO( 13, 1)	// 105
#define GPIO_BOOT_CONFIG2	GPIO( 13, 4)	// 108
#define GPIO_BOOT_CONFIG3	GPIO( 13, 7)	// 111

/* Boot configuration pin encodings */
#define BOOT_CONFIG_SPI0		0
#define BOOT_CONFIG_SPI0_TEST		1
#define BOOT_CONFIG_ANS			2
#define BOOT_CONFIG_ANS_TEST		3
#define BOOT_CONFIG_NVME0		4
#define BOOT_CONFIG_NVME0_TEST		5
#define BOOT_CONFIG_LOW_POWER_ANS	6
#define BOOT_CONFIG_FAST_SPI0_TEST	7
#define BOOT_CONFIG_NVME1		8
#define BOOT_CONFIG_NVME1_TEST		9

#define BOOT_CONFIG_TBT0_EXTREF		10
#define BOOT_CONFIG_TBT0_EXTREF_TEST	11
#define BOOT_CONFIG_TBT0_INTREF		12
#define BOOT_CONFIG_TBT0_INTREF_TEST	13

#define GPIO_REQUEST_DFU1	GPIO( 0, 1)	// 1 - HOLD_KEY
#define GPIO_REQUEST_DFU2	GPIO( 0, 0)	// 0 - MENU_KEY
#define GPIO_FORCE_DFU		GPIO(13, 2)	// 106
#define GPIO_DFU_STATUS		GPIO(13, 3)	// 107

#define SPI_NOR0		(0)

#define GPIO_SPI0_SCLK		GPIO( 8, 3)	// 67
#define GPIO_SPI0_MOSI		GPIO( 8, 4)	// 68
#define GPIO_SPI0_MISO		GPIO( 8, 5)	// 69
#define GPIO_SPI0_SSIN		GPIO( 8, 6)	// 70

#if SUPPORT_FPGA
#define GPIO_SPI0_CS		GPIO(GPIO_PAD_SPI, 0)
#else
#define GPIO_SPI0_CS		GPIO( 8, 6)	// 70 - Alias of GPIO_SPI0_SSIN
#endif

#define GPIO_IIC0_SCL		GPIO(4, 6)	// 38
#define GPIO_IIC0_SDA		GPIO(4, 5)	// 37
#define GPIO_IIC1_SCL		GPIO(8, 2)	// 66
#define GPIO_IIC1_SDA		GPIO(8, 1)	// 65
#define GPIO_IIC2_SCL		GPIO( 16, 5)	// 133
#define GPIO_IIC2_SDA		GPIO( 16, 4)	// 132
#define GPIO_IIC3_SCL		GPIO(16, 7)	// 135
#define GPIO_IIC3_SDA		GPIO(16, 6)	// 134

#define GPIO_PCIE0_PERST	GPIO( 22, 2)	// 178
#define GPIO_PCIE0_CLKREQ	GPIO( 21, 5)	// 173
#define GPIO_PCIE1_PERST	GPIO( 22, 3) 	// 179
#define GPIO_PCIE1_CLKREQ	GPIO( 21, 6) 	// 174
#define GPIO_PCIE2_PERST	GPIO( 22, 4) 	// 180
#define GPIO_PCIE2_CLKREQ	GPIO( 21, 7) 	// 175
#define GPIO_PCIE3_PERST	GPIO( 22, 5) 	// 181
#define GPIO_PCIE3_CLKREQ	GPIO( 22, 0) 	// 176

#define GPIO_PCIE_PHY01_CLKREQ	GPIO( 21, 1)	// 169
#define GPIO_PCIE_PHY23_CLKREQ	GPIO( 21, 0)	// 168

#define GPIO_S3E_RESETN		GPIO( 20, 7)	// 167
#define GPIO_NAND_SYS_CLK	GPIO( 22, 1)	// 177

#endif /* ! __PLATFORM_GPIODEF_T7001_H */
