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
#ifndef __PLATFORM_PINCONFIG_S5L8947X_H
#define __PLATFORM_PINCONFIG_S5L8947X_H

/* Default S5L8947X SoC Pin Configuration */

#define FMI_DRIVE_STR	DRIVE_X2
#define FMI_SLEW_RATE	SLEW_RATE_SLOW

static const u_int32_t gpio_default_cfg[GPIO_GROUP_COUNT * GPIOPADPINS] = {
/* Port  0 */
	CFG_DISABLED,							// I2S0_MCK		->
	CFG_IN,								// I2S0_LRCK		->
	CFG_DISABLED,							// I2S0_BCLK		->
	CFG_DISABLED,							// I2S0_DOUT		->
	CFG_DISABLED,							// I2S0_DIN		->
	CFG_DISABLED,							// UART0_TXD		->
	CFG_DISABLED,							// UART0_RXD		->
	CFG_DISABLED,							// UART1_TXD		->

/* Port  1 */
	CFG_DISABLED,							// UART1_RXD		->
	CFG_DISABLED,							// I2C1_SDA		->
	CFG_DISABLED,							// I2C1_SCL		->
	CFG_DISABLED,							// HDMI_HPD		->
	CFG_DISABLED,							// HDMI_CEC		->
	CFG_DISABLED,							// I2C2_SDA		->
	CFG_DISABLED,							// I2C2_SCL		->
	CFG_DISABLED,							// SPDIF		->

/* Port  2 */
	CFG_DISABLED,							// GPIO22		->
	CFG_DISABLED,							// GPIO23		->
	CFG_DISABLED,							// UART2_TXD		->
	CFG_DISABLED,							// UART2_RXD		->
	CFG_DISABLED,							// UART2_RTSN		->
	CFG_DISABLED,							// UART2_CTSN		->
	CFG_DISABLED,							// UART3_TXD		->
	CFG_DISABLED,							// UART3_RXD		->

/* Port  3 */
	CFG_DISABLED,							// UART4_TXD		->
	CFG_DISABLED,							// UART4_RXD		->
	CFG_DISABLED,							// TST_CLKOUT		->
	CFG_DISABLED | PULL_DOWN,					// TST_STPCLK		->
	CFG_DISABLED,							// WDOG			->
	CFG_DISABLED,							// 			->
	CFG_DISABLED,							// 			->
	CFG_DISABLED,							// 			->

/* Port  4 */
	CFG_DISABLED,							// ENET_MDC		->
	CFG_DISABLED,							// ENET_MDIO		->
	CFG_DISABLED,							// RMII_CLK		->
	CFG_DISABLED,							// RMII_RXER		->
	CFG_DISABLED,							// RMII_TXD[0]		->
	CFG_DISABLED,							// RMII_CRSDV		->
	CFG_DISABLED,							// RMII_RXD[0]		->
	CFG_DISABLED,							// RMII_RXD[1]		->

/* Port  5 */
	CFG_DISABLED,							// RMII_TXD[1]		->
	CFG_DISABLED,							// RMII_TXEN		->
	CFG_DISABLED | FMI_DRIVE_STR,					// FMI0_CEN[1]		->
	CFG_DISABLED | FMI_DRIVE_STR,					// FMI0_CEN[0]		->
	CFG_DISABLED | PULL_DOWN | FMI_DRIVE_STR,			// FMI0_CLE		->
	CFG_DISABLED | PULL_DOWN | FMI_DRIVE_STR,			// FMI0_ALE		->
	CFG_DISABLED | PULL_UP | FMI_DRIVE_STR,				// FMI0_REN		->
	CFG_DISABLED | PULL_UP | FMI_DRIVE_STR,				// FMI0_WEN		->

/* Port  6 */
	CFG_DISABLED | PULL_DOWN | FMI_DRIVE_STR,			// FMI0_IO[7]		->
	CFG_DISABLED | PULL_DOWN | FMI_DRIVE_STR,			// FMI0_IO[6]		->
	CFG_DISABLED | PULL_DOWN | FMI_DRIVE_STR,			// FMI0_IO[5]		->
	CFG_DISABLED | PULL_DOWN | FMI_DRIVE_STR,			// FMI0_IO[4]		->
	CFG_DISABLED | PULL_DOWN | FMI_DRIVE_STR,			// FMI0_DQS		->
	CFG_DISABLED | PULL_DOWN | FMI_DRIVE_STR,			// FMI0_IO[3]		->
	CFG_DISABLED | PULL_DOWN | FMI_DRIVE_STR,			// FMI0_IO[2]		->
	CFG_DISABLED | PULL_DOWN | FMI_DRIVE_STR,			// FMI0_IO[1]		->

/* Port  7 */
	CFG_DISABLED | PULL_DOWN | FMI_DRIVE_STR,			// FMI0_IO[0]		->
	CFG_DISABLED | FMI_DRIVE_STR,					// FMI1_CEN[1]		->
	CFG_DISABLED | FMI_DRIVE_STR,					// FMI1_CEN[0]		->
	CFG_DISABLED | PULL_DOWN | FMI_DRIVE_STR,			// FMI1_CLE		->
	CFG_DISABLED | PULL_DOWN | FMI_DRIVE_STR,			// FMI1_ALE		->
	CFG_DISABLED | PULL_UP | FMI_DRIVE_STR,				// FMI1_REN		->
	CFG_DISABLED | PULL_UP | FMI_DRIVE_STR,				// FMI1_WEN		->
	CFG_DISABLED | PULL_DOWN | FMI_DRIVE_STR,			// FMI1_IO[7]		->

/* Port  8 */
	CFG_DISABLED | PULL_DOWN | FMI_DRIVE_STR,			// FMI1_IO[6]		->
	CFG_DISABLED | PULL_DOWN | FMI_DRIVE_STR,			// FMI1_IO[5]		->
	CFG_DISABLED | PULL_DOWN | FMI_DRIVE_STR,			// FMI1_IO[4]		->
	CFG_DISABLED | PULL_DOWN | FMI_DRIVE_STR,			// FMI1_DQS		->
	CFG_DISABLED | PULL_DOWN | FMI_DRIVE_STR,			// FMI1_IO[3]		->
	CFG_DISABLED | PULL_DOWN | FMI_DRIVE_STR,			// FMI1_IO[2]		->
	CFG_DISABLED | PULL_DOWN | FMI_DRIVE_STR,			// FMI1_IO[1]		->
	CFG_DISABLED | PULL_DOWN | FMI_DRIVE_STR,			// FMI1_IO[0]		->

/* Port  9 */
	CFG_DISABLED,							//
	CFG_DISABLED,							//
	CFG_DISABLED,							//
	CFG_DISABLED,							//
	CFG_DISABLED,							//
	CFG_DISABLED,							//
	CFG_DISABLED,							//
	CFG_DISABLED,							//

/* Port 10 */
	CFG_DISABLED,							//
	CFG_DISABLED,							//
	CFG_DISABLED,							//
	CFG_DISABLED,							//
	CFG_DISABLED,							//
	CFG_DISABLED,							//
	CFG_DISABLED,							//
	CFG_DISABLED,							//

/* Port 11 */
	CFG_DISABLED,							//
	CFG_DISABLED,							//
	CFG_DISABLED,							//
	CFG_DISABLED,							//
	CFG_DISABLED,							//
	CFG_DISABLED,							//
	CFG_DISABLED,							//
	CFG_DISABLED,							//

/* Port 12 */
	CFG_IN,								// GPIO0		-> MENU_KEY
	CFG_IN,								// GPIO1		-> HOLD_KEY
	CFG_DISABLED,							// GPIO2		->
	CFG_DISABLED,							// GPIO3		->
	CFG_DISABLED,							// GPIO4		->
	CFG_DISABLED,							// GPIO5		->
	CFG_DISABLED,							// GPIO6		->
	CFG_DISABLED,							// GPIO7		->

/* Port 13 */
	CFG_DISABLED,							// GPIO8		->
	CFG_DISABLED,							// GPIO9		->
	CFG_DISABLED,							// GPIO10		->
	CFG_DISABLED,							// GPIO11		->
	CFG_DISABLED,							// GPIO12		->
	CFG_DISABLED,							// GPIO13		->
	CFG_DISABLED,							// GPIO14		->
	CFG_DISABLED,							// GPIO15		->

/* Port 14 */
	CFG_DISABLED,							// GPIO16		-> BOARD_ID[3]
	CFG_DISABLED,							// GPIO17		->
	CFG_DISABLED,							// GPIO18		-> BOOT_CONFIG[0]
	CFG_DISABLED,							// GPIO19		-> KEEPACT
	CFG_DISABLED,							// 			->
	CFG_DISABLED,							// 			->
	CFG_DISABLED,							// 			->
	CFG_DISABLED,							// 			->

/* Port 15 */
	CFG_DISABLED,							// 			->
	CFG_DISABLED,							// 			->
	CFG_DISABLED,							// 			->
	CFG_DISABLED,							// 			->
	CFG_DISABLED,							// 			->
	CFG_DISABLED,							// 			->
	CFG_DISABLED,							// 			->
	CFG_DISABLED,							// 			->

/* Port 16 */
	CFG_DISABLED,							// GPIO20		->
	CFG_DISABLED,							// GPIO21		->
	CFG_DISABLED,							// GPIO24		->
	CFG_DISABLED,							// GPIO25		-> BOOT_CONFIG[1]
	CFG_IN | PULL_DOWN,						// GPIO26		-> FORCE_DFU
	CFG_DISABLED | PULL_DOWN,					// GPIO27		-> DFU_STATUS
	CFG_DISABLED,							// GPIO28		-> BOOT_CONFIG{2]
	CFG_DISABLED,							// GPIO29		-> BOOT_CONFIG[3]

/* Port 17 */
	CFG_DISABLED,							// I2C0_SDA		->
	CFG_DISABLED,							// I2C0_SCL		->
	CFG_DISABLED,							// TMR32_PWM0		->
	CFG_DISABLED,							// SPI0_SCLK		-> SPI0_SCLK/BOARD_ID[0]
	CFG_DISABLED,							// SPI0_MOSI		-> SPI0_MOSI/BOARD_ID[1]
	CFG_DISABLED,							// SPI0_MISO		-> SPI0_MISO/BOARD_ID[2]
	CFG_DISABLED | PULL_UP,						// SPI0_SSIN		->
	CFG_DISABLED,							// SWI_DATA		->
};

#endif /* ! __PLATFORM_PINCONFIG_S5L8947X_H */
