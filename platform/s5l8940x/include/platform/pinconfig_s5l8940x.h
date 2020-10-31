/*
 * Copyright (C) 2009-2011 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */
#ifndef __PLATFORM_PINCONFIG_S5L8940X_H
#define __PLATFORM_PINCONFIG_S5L8940X_H

/* Default S5L8940X SoC Pin Configuration */

#define FMI_DRIVE_STR	DRIVE_X3

static const u_int32_t gpio_default_cfg[GPIO_GROUP_COUNT * GPIOPADPINS] = {
/* Port  0 */
	CFG_IN,								// GPIO0		-> MENU_KEY
	CFG_IN,								// GPIO1		-> HOLD_KEY
	CFG_DISABLED,							// GPIO2		->
	CFG_DISABLED,							// GPIO3		->
	CFG_DISABLED,							// GPIO4		->
	CFG_DISABLED | PULL_DOWN,					// GPIO5		->
	CFG_DISABLED,							// GPIO6		->
	CFG_DISABLED | PULL_DOWN,					// GPIO7		->

/* Port  1 */
	CFG_DISABLED,							// GPIO8		->
	CFG_DISABLED,							// GPIO9		->
	CFG_DISABLED,							// GPIO10		->
	CFG_DISABLED,							// GPIO11		->
	CFG_DISABLED,							// GPIO12		->
	CFG_DISABLED,							// GPIO13		->
	CFG_DISABLED,							// GPIO14		->
	CFG_DISABLED,							// GPIO15		->

/* Port  2 */
	CFG_DISABLED,							// GPIO16		-> BOARD_ID[3]
	CFG_DISABLED,							// GPIO17		->
	CFG_DISABLED,							// GPIO18		-> BOOT_CONFIG[0]
	CFG_DISABLED | PULL_DOWN,					// GPIO19		->
	CFG_DISABLED,							// GPIO20		->
	CFG_DISABLED,							// GPIO21		->
	CFG_DISABLED | PULL_DOWN,					// GPIO22		->
	CFG_DISABLED,							// GPIO23		->

/* Port  3 */
	CFG_DISABLED | PULL_DOWN,					// GPIO24		->
	CFG_DISABLED,							// GPIO25		-> BOOT_CONFIG[1]
	CFG_IN | PULL_DOWN,						// GPIO26		-> FORCE_DFU
	CFG_DISABLED | PULL_DOWN,					// GPIO27		-> DFU_STATUS
	CFG_DISABLED,							// GPIO28		-> BOOT_CONFIG[2]
	CFG_DISABLED,							// GPIO29		-> BOOT_CONFIG[3]
	CFG_DISABLED,							// GPIO30		->
	CFG_DISABLED,							// GPIO31		->

/* Port  4 */
	CFG_DISABLED,							// GPIO32		->
	CFG_DISABLED,							// GPIO33		->
	CFG_DISABLED,							// GPIO34		->
	CFG_DISABLED,							// GPIO35		->
	CFG_DISABLED,							// GPIO36		->
	CFG_DISABLED,							// GPIO37		->
	CFG_DISABLED,							// GPIO38		->
	CFG_DISABLED,							// GPIO39		->

/* Port  5 */
	CFG_DISABLED,							// EHCI_PORT_PWR[0]	->
	CFG_DISABLED,							// EHCI_PORT_PWR[1]	->
	CFG_DISABLED,							// EHCI_PORT_PWR[2]	->
	CFG_DISABLED,							// UART1_TXD		->
	CFG_DISABLED,							// UART1_RXD		->
	CFG_DISABLED,							// UART1_RTSN		->
	CFG_DISABLED,							// UART1_CTSN		->
	CFG_DISABLED,							// UART2_TXD		->

/* Port  6 */
	CFG_DISABLED,							// UART2_RXD		->
	CFG_DISABLED,							// UART2_RTSN		->
	CFG_DISABLED,							// UART2_CTSN		->
	CFG_DISABLED,							// UART3_TXD		->
	CFG_DISABLED,							// UART3_RXD		->
	CFG_DISABLED,							// UART3_RTSN		->
	CFG_DISABLED,							// UART3_CTSN		->
	CFG_DISABLED,							// UART5_RTXD		->

/* Port  7 */
	CFG_DISABLED,							// UART6_TXD		->
	CFG_DISABLED,							// UART6_RXD		->
	CFG_DISABLED,							// UART6_RTSN		->
	CFG_DISABLED,							// UART6_CTSN		->
	CFG_DISABLED,							// UART4_TXD/SPI4_MOSI	->
	CFG_DISABLED,							// UART4_RXD/SPI4_MISO	->
	CFG_DISABLED,							// UART4_RTSN/SPI4_SCLK	->
	CFG_DISABLED,							// UART4_CTSN/SPI4_SSIN	->

/* Port  8 */
	CFG_DISABLED | FMI_DRIVE_STR,					// FMI0_CEN3		->
	CFG_DISABLED | FMI_DRIVE_STR,					// FMI0_CEN2		->
	CFG_DISABLED | FMI_DRIVE_STR,					// FMI0_CEN1		->
	CFG_DISABLED | FMI_DRIVE_STR,					// FMI0_CEN0		->
	CFG_DISABLED | PULL_DOWN | FMI_DRIVE_STR,			// FMI0_CLE		->
	CFG_DISABLED | PULL_DOWN | FMI_DRIVE_STR,			// FMI0_ALE		->
	CFG_DISABLED | PULL_UP | FMI_DRIVE_STR,				// FMI0_REN		->
	CFG_DISABLED | PULL_UP | FMI_DRIVE_STR,				// FMI0_WEN		->

/* Port  9 */
	CFG_DISABLED | PULL_DOWN | FMI_DRIVE_STR,			// FMI0_IO7		->
	CFG_DISABLED | PULL_DOWN | FMI_DRIVE_STR,			// FMI0_IO6		->
	CFG_DISABLED | PULL_DOWN | FMI_DRIVE_STR,			// FMI0_IO5		->
	CFG_DISABLED | PULL_DOWN | FMI_DRIVE_STR,			// FMI0_IO4		->
	CFG_DISABLED | PULL_UP | FMI_DRIVE_STR,				// FMI0_DQS		->
	CFG_DISABLED | PULL_DOWN | FMI_DRIVE_STR,			// FMI0_IO3		->
	CFG_DISABLED | PULL_DOWN | FMI_DRIVE_STR,			// FMI0_IO2		->
	CFG_DISABLED | PULL_DOWN | FMI_DRIVE_STR,			// FMI0_IO1		->

/* Port 10 */
	CFG_DISABLED | PULL_DOWN | FMI_DRIVE_STR,			// FMI0_IO0		->
	CFG_DISABLED | FMI_DRIVE_STR,					// FMI1_CEN3		->
	CFG_DISABLED | FMI_DRIVE_STR,					// FMI1_CEN2		->
	CFG_DISABLED | FMI_DRIVE_STR,					// FMI1_CEN1		->
	CFG_DISABLED | FMI_DRIVE_STR,					// FMI1_CEN0		->
	CFG_DISABLED | PULL_DOWN | FMI_DRIVE_STR,			// FMI1_CLE		->
	CFG_DISABLED | PULL_DOWN | FMI_DRIVE_STR,			// FMI1_ALE		->
	CFG_DISABLED | PULL_UP | FMI_DRIVE_STR,				// FMI1_REN		->

/* Port 11 */
	CFG_DISABLED | PULL_UP | FMI_DRIVE_STR,				// FMI1_WEN		->
	CFG_DISABLED | PULL_DOWN | FMI_DRIVE_STR,			// FMI1_IO7		->
	CFG_DISABLED | PULL_DOWN | FMI_DRIVE_STR,			// FMI1_IO6		->
	CFG_DISABLED | PULL_DOWN | FMI_DRIVE_STR,			// FMI1_IO5		->
	CFG_DISABLED | PULL_DOWN | FMI_DRIVE_STR,			// FMI1_IO4		->
	CFG_DISABLED | PULL_UP | FMI_DRIVE_STR,				// FMI1_DQS		->
	CFG_DISABLED | PULL_DOWN | FMI_DRIVE_STR,			// FMI1_IO3		->
	CFG_DISABLED | PULL_DOWN | FMI_DRIVE_STR,			// FMI1_IO2		->

/* Port 12 */
	CFG_DISABLED | PULL_DOWN | FMI_DRIVE_STR,			// FMI1_IO1		->
	CFG_DISABLED | PULL_DOWN | FMI_DRIVE_STR,			// FMI1_IO0		->
	CFG_DISABLED,							// FMI2_CEN3		->
	CFG_DISABLED,							// FMI2_CEN2		->
	CFG_DISABLED,							// FMI2_CEN1		->
	CFG_DISABLED,							// FMI2_CEN0		->
	CFG_DISABLED,							// FMI2_CLE		->
	CFG_DISABLED,							// FMI2_ALE		->

/* Port 13 */
	CFG_DISABLED,							// FMI2_REN		->
	CFG_DISABLED,							// FMI2_WEN		->
	CFG_DISABLED,							// FMI2_IO7		->
	CFG_DISABLED,							// FMI2_IO6		->
	CFG_DISABLED,							// FMI2_IO5		->
	CFG_DISABLED,							// FMI2_IO4		->
	CFG_DISABLED,							// FMI2_DQS		->
	CFG_DISABLED,							// FMI2_IO3		->

/* Port 14 */
	CFG_DISABLED,							// FMI2_IO2		->
	CFG_DISABLED,							// FMI2_IO1		->
	CFG_DISABLED,							// FMI2_IO0		->
	CFG_DISABLED,							// FMI0_CEN7		->
	CFG_DISABLED,							// FMI0_CEN6		->
	CFG_DISABLED,							// FMI0_CEN5		->
	CFG_DISABLED,							// FMI0_CEN4		->
	CFG_DISABLED,							// FMI1_CEN7		->

/* Port 15 */
	CFG_DISABLED,							// FMI1_CEN6		->
	CFG_DISABLED,							// FMI1_CEN5		->
	CFG_DISABLED,							// FMI1_CEN4		->
	CFG_DISABLED,							// FMI3_CEN3		->
	CFG_DISABLED,							// FMI3_CEN2		->
	CFG_DISABLED,							// FMI3_CEN1		->
	CFG_DISABLED,							// FMI3_CEN0		->
	CFG_DISABLED,							// FMI3_CLE		->

/* Port 16 */
	CFG_DISABLED,							// FMI3_ALE		->
	CFG_DISABLED,							// FMI3_REN		->
	CFG_DISABLED,							// FMI3_WEN		->
	CFG_DISABLED,							// FMI3_IO7		->
	CFG_DISABLED,							// FMI3_IO6		->
	CFG_DISABLED,							// FMI3_IO5		->
	CFG_DISABLED,							// FMI3_IO4		->
	CFG_DISABLED,							// FMI3_DQS		->

/* Port 17 */
	CFG_DISABLED,							// FMI3_IO3		->
	CFG_DISABLED,							// FMI3_IO2		->
	CFG_DISABLED,							// FMI3_IO1		->
	CFG_DISABLED,							// FMI3_IO0		->
	CFG_DISABLED,							// FMI2_CEN7		->
	CFG_DISABLED,							// FMI2_CEN6		->
	CFG_DISABLED,							// FMI2_CEN5		->
	CFG_DISABLED,							// FMI2_CEN4		->

/* Port 18 */
	CFG_DISABLED,							// FMI3_CEN7		->
	CFG_DISABLED,							// FMI3_CEN6		->
	CFG_DISABLED,							// FMI3_CEN5		->
	CFG_DISABLED,							// FMI3_CEN4		->
	CFG_DISABLED,							// SPI3_MOSI		-> 
	CFG_DISABLED,							// SPI3_MISO		-> 
	CFG_DISABLED,							// SPI3_SCLK		-> 
	CFG_DISABLED,							// SPI3_SSIN		-> 

/* Port 19 */
	CFG_DISABLED,							// ISP0_PRE_FLASH	->
	CFG_DISABLED,							// ISP0_FLASH		->
	CFG_DISABLED,							// ISP1_PRE_FLASH	->
	CFG_DISABLED,							// ISP1_FLASH		->
	CFG_DISABLED,							// SPI1_SCLK		->
	CFG_DISABLED,							// SPI1_MOSI		->
	CFG_DISABLED,							// SPI1_MISO		->
	CFG_DISABLED,							// SPI1_SSIN		->

/* Port 20 */
	CFG_DISABLED,							// I2C2_SDA		->
	CFG_DISABLED,							// I2C2_SCL		->
	CFG_DISABLED,							// SPI0_SCLK		-> SPI0_SCLK/BOARD_ID[0]
	CFG_DISABLED,							// SPI0_MOSI		-> SPI0_MOSI/BOARD_ID[1]
	CFG_DISABLED,							// SPI0_MISO		-> SPI0_MISO/BOARD_ID[2]
	CFG_DISABLED | PULL_UP,						// SPI0_SSIN		-> SPI0_SSIN
	CFG_DISABLED,							// SPI2_SCLK		->
	CFG_DISABLED,							// SPI2_MOSI		->

/* Port 21 */
	CFG_DISABLED,							// SPI2_MISO		->
	CFG_DISABLED,							// SPI2_SSIN		->
	CFG_DISABLED,							// I2C0_SDA		->
	CFG_DISABLED,							// I2C0_SCL		->
	CFG_DISABLED,							// I2C1_SDA		->
	CFG_DISABLED,							// I2C1_SCL		->
	CFG_DISABLED,							// ISP0_SDA		->
	CFG_DISABLED,							// ISP0_SCL		->

/* Port 22 */
	CFG_DISABLED,							// ISP1_SDA		->
	CFG_DISABLED,							// ISP1_SCL		->
	CFG_DISABLED,							// SDIO_CLK		->
	CFG_DISABLED,							// SDIO_CMD		->
	CFG_DISABLED,							// SDIO_DATA0		->
	CFG_DISABLED,							// SDIO_DATA1		->
	CFG_DISABLED,							// SDIO_DATA2		->
	CFG_DISABLED,							// SDIO_DATA3		->

/* Port 23 */
	CFG_DISABLED,							// MIPI_VSYNC		->
	CFG_DISABLED,							// TMR32_PWM0		->
	CFG_DISABLED,							// TMR32_PWM1		->
	CFG_DISABLED,							// TMR32_PWM2		->
	CFG_DISABLED,							// SWI_DATA		->
	CFG_DISABLED,							// DWI_DI		->
	CFG_DISABLED,							// DWI_DO		->
	CFG_DISABLED,							// DWI_CLK		->

/* Port 24 */
	CFG_DISABLED,							// SENSOR0_RST		->
	CFG_DISABLED,							// SENSOR0_CLK		->
	CFG_DISABLED,							// SENSOR1_RST		->
	CFG_DISABLED,							// SENSOR1_CLK		->
	CFG_DISABLED,							// I2S0_MCK		->
	CFG_DISABLED,							// I2S0_LRCK		->
	CFG_DISABLED,							// I2S0_BCLK		->
	CFG_DISABLED,							// I2S0_DOUT		->

/* Port 25 */
	CFG_DISABLED,							// I2S0_DIN		->
	CFG_DISABLED,							// I2S1_MCK		->
	CFG_DISABLED,							// I2S1_LRCK		->
	CFG_DISABLED,							// I2S1_BCLK		->
	CFG_DISABLED,							// I2S1_DOUT		->
	CFG_DISABLED,							// I2S1_DIN		->
	CFG_DISABLED,							// I2S2_MCK		->
	CFG_DISABLED,							// I2S2_LRCK		->

/* Port 26 */
	CFG_DISABLED,							// I2S2_BCLK		->
	CFG_DISABLED,							// I2S2_DOUT		->
	CFG_DISABLED,							// I2S2_DIN		->
	CFG_DISABLED,							// I2S3_MCK		->
	CFG_DISABLED,							// I2S3_LRCK		->
	CFG_DISABLED,							// I2S3_BCLK		->
	CFG_DISABLED,							// I2S3_DOUT		->
	CFG_DISABLED,							// I2S3_DIN		->

/* Port 27 */
	CFG_DISABLED,							// SPDIF		->
	CFG_DISABLED | PULL_DOWN,					// GPIO217		->
	CFG_DISABLED,							// GPIO218		->
	CFG_DISABLED,							// DP_HPD		->
	CFG_DISABLED,							// UART0_TXD		->
	CFG_DISABLED,							// UART0_RXD		->
	CFG_DISABLED,							// TST_CLKOUT		->
	CFG_DISABLED,							// TST_STPCLK		->

/* Port 28 */
	CFG_DISABLED,							// WDOG			->
	CFG_DISABLED,							// 			->
	CFG_DISABLED,							// 			->
	CFG_DISABLED,							// 			->
	CFG_DISABLED,							// 			->
	CFG_DISABLED,							// 			->
	CFG_DISABLED,							// 			->
	CFG_DISABLED,							// 			->
};

#endif /* ! __PLATFORM_PINCONFIG_S5L8940X_H */
