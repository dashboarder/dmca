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
#include <drivers/apple/gpio.h>
#include <platform/soc/hwregbase.h>
#include <stdint.h>

/* Pin configuration for FPGA */

static const uint32_t gpio_default_cfg[GPIO_GROUP_COUNT * GPIOPADPINS] = {
/* Port  0 */
	CFG_DISABLED,							// TST_CLKOUT		->
	CFG_DISABLED,							// WDOG			->
	CFG_IN,								// GPIO0		-> MENU_KEY (REQUEST_DFU2)
	CFG_IN,								// GPIO1		-> HOLD_KEY (REQUEST_DFU1)
	CFG_DISABLED,							// GPIO2		->
	CFG_DISABLED,							// GPIO3		->
	CFG_DISABLED,							// GPIO4		->
	CFG_DISABLED,							// GPIO5		->

/* Port  1 */
	CFG_DISABLED,							// GPIO6		->
	CFG_DISABLED,							// GPIO7		->
	CFG_DISABLED,							// GPIO8		->
	CFG_DISABLED,							// GPIO9		->
	CFG_DISABLED,							// GPIO10		->
	CFG_DISABLED,							// GPIO11		->
	CFG_DISABLED,							// GPIO12		->
	CFG_DISABLED,							// GPIO13		->

/* Port  2 */
	CFG_DISABLED,							// GPIO14		->
	CFG_DISABLED,							// GPIO15		->
	CFG_DISABLED,							// GPIO16		-> BOARD_ID[3]
	CFG_DISABLED,							// GPIO17		->
	CFG_DISABLED,							// GPIO18		-> BOOT_CONFIG[0]
	CFG_DISABLED,							// GPIO19		->
	CFG_DISABLED,							// GPIO20		->
	CFG_DISABLED,							// GPIO21		->

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
	CFG_DISABLED,							// UART1_TXD		->
	CFG_DISABLED,							// UART1_RXD		->
	CFG_DISABLED,							// UART1_RTSN		->
	CFG_DISABLED,							// UART1_CTSN		->
	CFG_DISABLED,							// UART2_TXD		->
	CFG_DISABLED,							// UART2_RXD		->
	CFG_DISABLED,							// UART2_RTSN		->
	CFG_DISABLED,							// UART2_CTSN		->

/* Port 5 */
	CFG_DISABLED,							// UART3_TXD		->
	CFG_DISABLED,							// UART3_RXD		->
	CFG_DISABLED,							// UART3_RTSN		->
	CFG_DISABLED,							// UART3_CTSN		->
	CFG_DISABLED,							// UART5_RXD		->
	CFG_DISABLED,							// NULL			->
	CFG_DISABLED,							// NULL			->
	CFG_DISABLED,							// NULL			->

/* Port  6 */
	CFG_DISABLED,							// NULL			->
	CFG_DISABLED,							// NULL			->
	CFG_DISABLED,							// NULL			->
	CFG_DISABLED,							// NULL			->
	CFG_DISABLED,							// NULL			->
	CFG_DISABLED,							// NULL			->
	CFG_DISABLED,							// NULL			->
	CFG_DISABLED,							// NULL			->

/* Port  7 */
	CFG_DISABLED,							// NULL			->
	CFG_DISABLED,							// NULL			->
	CFG_DISABLED,							// NULL			->
	CFG_DISABLED,							// NULL			->
	CFG_DISABLED,							// NULL			->
	CFG_DISABLED,							// NULL			->
	CFG_DISABLED,							// NULL			->
	CFG_DISABLED,							// NULL			->

/* Port  8 */
	CFG_DISABLED,							// UART4_TXD		->
	CFG_DISABLED,							// UART4_RXD		->
	CFG_DISABLED,							// UART4_RTSN		->
	CFG_DISABLED,							// UART4_CTSN		->
	CFG_DISABLED,							// SPI1_SCLK		->
	CFG_DISABLED,							// SPI1_MOSI		->
	CFG_DISABLED,							// SPI1_MISO		->
	CFG_DISABLED,							// SPI1_SSIN		->

/* Port  9 */
	CFG_FUNC0,							// SPI0_SCLK		-> SPI0_SCLK/BOARD_ID[0]
	CFG_FUNC0,							// SPI0_MOSI		-> SPI0_MOSI/BOARD_ID[1]
	CFG_FUNC0,							// SPI0_MISO		-> SPI0_MISO/BOARD_ID[2]
	CFG_OUT_1,							// SPI0_SSIN		->
	CFG_DISABLED,							// SPI2_SCLK		->
	CFG_DISABLED,							// SPI2_MOSI		->
	CFG_DISABLED,							// SPI2_MISO		->
	CFG_DISABLED,							// SPI2_SSIN		->

/* Port  10 */
	CFG_DISABLED,							// I2C0_SDA		->
	CFG_DISABLED,							// I2C0_SCL		->
	CFG_DISABLED,							// I2C1_SDA		->
	CFG_DISABLED,							// I2C1_SCL		->
	CFG_DISABLED,							// ISP0_SDA		->
	CFG_DISABLED,							// ISP0_SCL		->
	CFG_DISABLED,							// ISP1_SDA		->
	CFG_DISABLED,							// ISP1_SCL		->

/* Port  11 */
	CFG_DISABLED,							// SENSOR0_RST		->
	CFG_DISABLED,							// SENSOR0_CLK		->
	CFG_DISABLED,							// SENSOR0_XSHUTDOWN	->
	CFG_DISABLED,							// SENSOR0_ISTRB	->
	CFG_DISABLED,							// SENSOR1_RST		->
	CFG_DISABLED,							// SENSOR1_CLK		->
	CFG_DISABLED,							// SENSOR1_XSHUTDOWN	->
	CFG_DISABLED,							// SENSOR1_ISTRB	->

/* Port  12 */
	CFG_DISABLED,							// SPI3_MOSI		->
	CFG_DISABLED,							// SPI3_MISO		->
	CFG_DISABLED,							// SPI3_SCLK		->
	CFG_DISABLED,							// SPI3_SSIN		->
	CFG_DISABLED,							// I2C2_SDA		->
	CFG_DISABLED,							// I2C2_SCL		->
	CFG_DISABLED,							// GPIO22		->
	CFG_DISABLED,							// GPIO23		->

/* Port  13 */
	CFG_DISABLED,							// GPIO24		->
	CFG_DISABLED,							// GPIO25		-> BOOT_CONFIG[1]
	CFG_IN,								// GPIO26		-> FORCE_DFU
	CFG_DISABLED,							// GPIO27		-> DFU_STATUS
	CFG_DISABLED,							// GPIO28		-> BOOT_CONFIG[2]
	CFG_DISABLED,							// GPIO29		-> BOOT_CONFIG[3]
	CFG_DISABLED,							// GPIO30		->
	CFG_DISABLED,							// GPIO31		->

/* Port  14 */
	CFG_DISABLED,							// GPIO32		->
	CFG_DISABLED,							// GPIO33		->
	CFG_DISABLED,							// GPIO34		->
	CFG_DISABLED,							// GPIO35		->
	CFG_DISABLED,							// GPIO36		->
	CFG_DISABLED,							// GPIO37		->
	CFG_DISABLED,							// GPIO38		->
	CFG_DISABLED,							// GPIO39		->

/* Port  15 */
	CFG_DISABLED,							// SOCHOT0		->
	CFG_DISABLED,							// SOCHOT1		->
	CFG_FUNC0,							// UART0_TXD		->
	CFG_FUNC0,							// UART0_RXD		->
	CFG_DISABLED,							// DWI_DI		->
	CFG_DISABLED,							// DWI_D0		->
	CFG_DISABLED,							// DWI_CLK		->
	CFG_DISABLED,							// NULL			->

/* Port  16 */
	CFG_DISABLED,							// I2S0_LRCK		->
	CFG_DISABLED,							// I2S0_BCLK		->
	CFG_DISABLED,							// I2S0_DOUT		->
	CFG_DISABLED,							// I2S0_DIN		->
	CFG_DISABLED,							// I2S1_MCK		->
	CFG_DISABLED,							// I2S1_LRCK		->
	CFG_DISABLED,							// I2S1_BCLK		->
	CFG_DISABLED,							// I2S1_DOUT		->
	
/* Port  17 */	
	CFG_DISABLED,							// I2S1_DIN		->
	CFG_DISABLED,							// I2S2_LRCK		->
	CFG_DISABLED,							// I2S2_BCLK		->
	CFG_DISABLED,							// I2S2_DOUT		->
	CFG_DISABLED,							// I2S2_DIN		->
	CFG_DISABLED,							// I2S3_MCK		->
	CFG_DISABLED,							// I2S3_LRCK		->
	CFG_DISABLED,							// I2S3_BCLK		->

/* Port  18 */	
	CFG_DISABLED,							// I2S3_DOUT		->
	CFG_DISABLED,							// I2S3_DIN		->
	CFG_DISABLED,							// I2S4_LRCK		->
	CFG_DISABLED,							// I2S4_BCLK		->
	CFG_DISABLED,							// I2S4_DOUT		->
	CFG_DISABLED,							// I2S4_DIN		->
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
	CFG_DISABLED,							// TMR32_PWM0		->
	CFG_DISABLED,							// TMR32_PWM1		->
	CFG_DISABLED,							// TMR32_PWM2		->
	CFG_DISABLED,							// SIO_7816UART0_SDA	->
	CFG_DISABLED,							// SIO_7816UART0_SCL	->
	CFG_DISABLED,							// SIO_7816UART0_RST	->
	CFG_DISABLED,							// SIO_7816UART1_SDA	->
	CFG_DISABLED,							// SIO_7816UART1_SCL	->

/* Port 21 */
	CFG_DISABLED,							// SIO_7816UART0_RST	->
	CFG_DISABLED,							// UART6_TXD		->
	CFG_DISABLED,							// UART6_RXD		->
	CFG_DISABLED,							// I2C3_SDA		->
	CFG_DISABLED,							// I2C3_SCL		->
	CFG_DISABLED,							// NULL			->
	CFG_DISABLED,							// NULL			->
	CFG_DISABLED,							// NULL			->

/* Port 22 */
	CFG_DISABLED,							// NULL			->
	CFG_DISABLED,							// NULL			->
	CFG_DISABLED,							// NULL			->
	CFG_DISABLED,							// NULL			->
	CFG_DISABLED,							// NULL			->
	CFG_DISABLED,							// NULL			->
	CFG_DISABLED,							// NULL			->
	CFG_DISABLED,							// NULL			->

/* Port 23 */
	CFG_DISABLED,							// NULL			->
	CFG_DISABLED,							// NULL			->
	CFG_DISABLED,							// NULL			->
	CFG_DISABLED,							// NULL			->
	CFG_DISABLED,							// NULL			->
	CFG_DISABLED,							// NULL			->
	CFG_DISABLED,							// NULL			->
	CFG_DISABLED,							// NULL			->

/* Port 24 */
	CFG_DISABLED,							// EPD_HPD		->
	CFG_DISABLED,							// I2S0_MCK		->
	CFG_DISABLED,							// I2S2_MCK		->
	CFG_DISABLED,							// NULL			->
	CFG_DISABLED,							// NULL			->
	CFG_DISABLED,							// NULL			->
	CFG_DISABLED,							// NULL			->
	CFG_DISABLED,							// NULL			->
};

const uint32_t *target_get_default_gpio_cfg(uint32_t gpioc)
{
	return gpio_default_cfg;
}
