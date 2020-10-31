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
#ifndef __PLATFORM_PINCONFIG_H
#define __PLATFORM_PINCONFIG_H

/* Default T7000 SoC Pin Configuration - Fiji I/O Spreadsheet v9.3 */

static const uint32_t gpio_default_cfg[GPIO_GROUP_COUNT * GPIOPADPINS] = {

/* Port  0 */
	CFG_DISABLED,							// ULPI_DIR		->
	CFG_DISABLED,							// ULPI_STP		->
	CFG_DISABLED,							// ULPI_NXT		->
	CFG_DISABLED,							// ULPI_DATA7		->
	CFG_DISABLED,							// ULPI_DATA6		->
	CFG_DISABLED,							// ULPI_DATA5		->
	CFG_DISABLED,							// ULPI_DATA4		->
	CFG_DISABLED,							// ULPI_CLK		->

/* Port  1 */
	CFG_DISABLED,							// ULPI_DATA3		->
	CFG_DISABLED,							// ULPI_DATA2		->
	CFG_DISABLED,							// ULPI_DATA1		->
	CFG_DISABLED,							// ULPI_DATA0		->
	CFG_DISABLED,							// SPI1_SCLK		->
	CFG_DISABLED,							// SPI1_MOSI		->
	CFG_DISABLED,							// SPI1_MISO		->
	CFG_DISABLED,							// SPI1_SSIN		->

/* Port  2 */
	CFG_DISABLED,							// NULL			->
	CFG_DISABLED,							// NULL			->
	CFG_DISABLED,							// NULL			->
	CFG_DISABLED,							// NULL			->
	CFG_DISABLED,							// NULL			->
	CFG_DISABLED,							// NULL			->
	CFG_DISABLED,							// NULL			->
	CFG_DISABLED,							// NULL			->

/* Port  3 */
	CFG_DISABLED,							// NULL			->
	CFG_DISABLED,							// NULL			->
	CFG_DISABLED,							// NULL			->
	CFG_DISABLED,							// NULL			->
	CFG_DISABLED,							// NULL			->
	CFG_DISABLED,							// NULL			->
	CFG_DISABLED,							// NULL			->
	CFG_DISABLED,							// NULL			->

/* Port  4 */
	CFG_IN,								// GPIO11		-> REQUEST_DFU2 (formerly known as MENU_KEY)
	CFG_IN,								// GPIO12		-> REQUEST_DFU1 (formerly known as HOLD_KEY)
	CFG_DISABLED,							// I2S3_MCK		->
	CFG_DISABLED,							// I2S3_LRCK		->
	CFG_DISABLED,							// I2S3_BCLK		->
	CFG_DISABLED,							// I2S3_DOUT		->
	CFG_DISABLED,							// I2S3_DIN		->
	CFG_DISABLED,							// CLK32K_OUT		->

/* Port 5 */
	CFG_DISABLED,							// PCIE_CLKREQ0_N	->
	CFG_DISABLED,							// PCIE_CLKREQ1_N	->
	CFG_DISABLED,							// NAND_SYS_CLK		->
	CFG_DISABLED,							// GPIO0		->
	CFG_DISABLED,							// GPIO1		->
	CFG_DISABLED,							// GPIO2		->
	CFG_DISABLED,							// GPIO3		->
	CFG_DISABLED,							// GPIO4		->

/* Port  6 */
	CFG_DISABLED,							// GPIO5		->
	CFG_DISABLED,							// GPIO6		->
	CFG_DISABLED,							// GPIO7		->
	CFG_DISABLED,							// GPIO14		->
	CFG_DISABLED,							// GPIO16		-> BOARD_ID[3]
	CFG_DISABLED,							// GPIO17		->
	CFG_DISABLED,							// GPIO18		-> BOOT_CONFIG[0]
	CFG_DISABLED,							// GPIO20		->

/* Port  7 */
	CFG_DISABLED,							// GPIO21		->
	CFG_DISABLED,							// UART5_RTXD		->
	CFG_DISABLED,							// UART8_TXD		->
	CFG_DISABLED,							// UART8_RXD		->
	CFG_DISABLED,							// SPI0_SCLK		-> SPI0_SCLK/BOARD_ID[0]
	CFG_DISABLED,							// SPI0_MOSI		-> SPI0_MOSI/BOARD_ID[1]
	CFG_DISABLED,							// SPI0_MISO		-> SPI0_MISO/BOARD_ID[2]
	CFG_DISABLED,							// SPI0_SSIN		->

/* Port  8 */
	CFG_DISABLED,							// I2C2_SDA		->
	CFG_DISABLED,							// I2C2_SCL		->
	CFG_DISABLED,							// GPIO22		->
	CFG_DISABLED,							// GPIO23		->
	CFG_DISABLED,							// GPIO25		-> BOOT_CONFIG[1]
	CFG_DISABLED,							// GPIO28		-> BOOT_CONFIG[2]
	CFG_DISABLED,							// GPIO29		-> BOARD_ID[4]
	CFG_DISABLED,							// GPIO34		->

/* Port  9 */
	CFG_DISABLED,							// GPIO35		->
	CFG_DISABLED,							// GPIO36		->
	CFG_DISABLED,							// GPIO37		->
	CFG_DISABLED,							// GPIO39		->
	CFG_DISABLED,							// GPIO42		->
	CFG_DISABLED,							// GPIO43		->
	CFG_DISABLED,							// DISP_VSYNC		->
	CFG_DISABLED,							// UART0_TXD		->

/* Port  10 */
	CFG_DISABLED,							// UART0_RXD		->
	CFG_DISABLED,							// TMR32_PWM0		->
	CFG_DISABLED,							// TMR32_PWM1		->
	CFG_DISABLED,							// TMR32_PWM2		->
	CFG_DISABLED,							// UART6_TXD		->
	CFG_DISABLED,							// UART6_RXD		->
	CFG_DISABLED,							// I2C3_SDA		->
	CFG_DISABLED,							// I2C3_SCL		->

/* Port  11 */
	CFG_DISABLED,							// NULL			->
	CFG_DISABLED,							// NULL			->
	CFG_DISABLED,							// NULL			->
	CFG_DISABLED,							// NULL			->
	CFG_DISABLED,							// NULL			->
	CFG_DISABLED,							// NULL			->
	CFG_DISABLED,							// NULL			->
	CFG_DISABLED,							// NULL			->

/* Port  12 */
	CFG_DISABLED,							// I2C0_SDA		->
	CFG_DISABLED,							// I2C0_SCL		->
	CFG_DISABLED,							// GPIO38		->
	CFG_DISABLED,							// UART2_TXD		->
	CFG_DISABLED,							// UART2_RXD		->
	CFG_DISABLED,							// UART2_RTSN		->
	CFG_DISABLED,							// UART2_CTSN		->
	CFG_DISABLED,							// DWI_DO		->

/* Port  13 */
	CFG_DISABLED,							// DWI_CLK		->
	CFG_FUNC0,							// WDOG			->
	CFG_DISABLED,							// GPIO13		->
	CFG_DISABLED,							// GPIO19		->
	CFG_IN | PULL_DOWN,						// GPIO26		-> FORCE_DFU
	CFG_DISABLED | PULL_DOWN,					// GPIO27		-> DFU_STATUS
	CFG_DISABLED,							// SOCHOT0		->
	CFG_DISABLED,							// SOCHOT1		->

/* Port  14 */
	CFG_DISABLED,							// CPU_SLEEP_STATUS	->
	CFG_DISABLED,							// TST_CLKOUT		->
	CFG_DISABLED,							// GPIO8		->
	CFG_DISABLED,							// GPIO9		->
	CFG_DISABLED,							// GPIO10		->
	CFG_DISABLED,							// GPIO15		->
	CFG_DISABLED,							// UART4_TXD		->
	CFG_DISABLED,							// UART4_RXD		->

/* Port  15 */
	CFG_DISABLED,							// UART4_RTSN		->
	CFG_DISABLED,							// UART4_CTSN		->
	CFG_DISABLED,							// SPI3_MOSI		->
	CFG_DISABLED,							// SPI3_MISO		->
	CFG_DISABLED,							// SPI3_SCLK		->
	CFG_DISABLED,							// SPI3_SSIN		->
	CFG_DISABLED,							// GPIO24		->
	CFG_DISABLED,							// GPIO30		->

/* Port  16 */
	CFG_DISABLED,							// GPIO31		->
	CFG_DISABLED,							// GPIO32		->
	CFG_DISABLED,							// GPIO33		->
	CFG_DISABLED,							// GPIO40		->
	CFG_DISABLED,							// GPIO41		->
	CFG_DISABLED,							// I2S4_MCK		->
	CFG_DISABLED,							// I2S4_LRCK		->
	CFG_DISABLED,							// I2S4_BCLK		->

/* Port  17 */
	CFG_DISABLED,							// I2S4_DOUT		->
	CFG_DISABLED,							// I2S4_DIN		->
	CFG_DISABLED,							// I2C1_SDA		->
	CFG_DISABLED,							// I2C1_SCL		->
	CFG_DISABLED,							// NULL			->
	CFG_DISABLED,							// NULL			->
	CFG_DISABLED,							// NULL			->
	CFG_DISABLED,							// NULL			->

/* Port  18 */
	CFG_DISABLED,							// NULL			->
	CFG_DISABLED,							// NULL			->
	CFG_DISABLED,							// NULL			->
	CFG_DISABLED,							// NULL			->
	CFG_DISABLED,							// NULL			->
	CFG_DISABLED,							// NULL			->
	CFG_DISABLED,							// NULL			->
	CFG_DISABLED,							// NULL			->

/* Port 19 */
	CFG_DISABLED,							// NULL			->
	CFG_DISABLED,							// NULL			->
	CFG_DISABLED,							// NULL			->
	CFG_DISABLED,							// NULL			->
	CFG_DISABLED,							// NULL			->
	CFG_DISABLED,							// NULL			->
	CFG_DISABLED,							// NULL			->
	CFG_DISABLED,							// NULL			->

/* Port 20 */
	CFG_DISABLED,							// I2S0_LRCK		->
	CFG_DISABLED,							// I2S0_BCLK		->
	CFG_DISABLED,							// I2S0_DOUT		->
	CFG_DISABLED,							// I2S0_DIN		->
	CFG_DISABLED,							// I2S1_MCK		->
	CFG_DISABLED,							// I2S1_LRCK		->
	CFG_DISABLED,							// I2S1_BCLK		->
	CFG_DISABLED,							// I2S1_DOUT		->

/* Port 21 */
	CFG_DISABLED,							// I2S1_DIN		->
	CFG_DISABLED,							// I2S2_LRCK		->
	CFG_DISABLED,							// I2S2_BCLK		->
	CFG_DISABLED,							// I2S2_DOUT		->
	CFG_DISABLED,							// I2S2_DIN		->
	CFG_DISABLED,							// UART1_TXD		->
	CFG_DISABLED,							// UART1_RXD		->
	CFG_DISABLED,							// UART1_RTSN		->

/* Port 22 */
	CFG_DISABLED,							// UART1_CTSN		->
	CFG_DISABLED,							// EDP_HPD		->
	CFG_DISABLED,							// UART3_TXD		->
	CFG_DISABLED,							// UART3_RXD		->
	CFG_DISABLED,							// UART3_RTSN		->
	CFG_DISABLED,							// UART3_CTSN		->
	CFG_DISABLED,							// SPI2_SCLK		->
	CFG_DISABLED,							// SPI2_MOSI		->

/* Port 23 */
	CFG_DISABLED,							// SPI2_MISO		->
	CFG_DISABLED,							// SPI2_SSIN		->
	CFG_DISABLED,							// ISP0_SDA		->
	CFG_DISABLED,							// ISP0_SCL		->
	CFG_DISABLED,							// ISP1_SDA		->
	CFG_DISABLED,							// ISP1_SCL		->
	CFG_DISABLED,							// SENSOR0_RST		->
	CFG_DISABLED,							// SENSOR0_CLK		->

/* Port 24 */
	CFG_DISABLED,							// SENSOR0_XSHUTDOWN	->
	CFG_DISABLED,							// SENSOR0_ISTRB	->
	CFG_DISABLED,							// ISP_UART0_TXD	->
	CFG_DISABLED,							// ISP_UART0_RXD	->
	CFG_DISABLED,							// SENSOR1_RST		->
	CFG_DISABLED,							// SENSOR1_CLK		->
	CFG_DISABLED,							// SENSOR1_XSHUTDOWN	->
	CFG_DISABLED,							// SENSOR1_ISTRB	->

/* Port 25 */
	CFG_DISABLED,							// UART7_TXD		->
	CFG_DISABLED,							// UART7_RXD		->
	CFG_DISABLED,							// I2S0_MCK		->
	CFG_DISABLED,							// I2S2_MCK		->
	CFG_DISABLED,							// NULL			->
	CFG_DISABLED,							// NULL			->
	CFG_DISABLED,							// NULL			->
	CFG_DISABLED,							// NULL			->
};

#endif /* ! __PLATFORM_PINCONFIG_H */
