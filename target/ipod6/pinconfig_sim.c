/*
 * Copyright (C) 2013-2014 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */
#include <debug.h>
#include <drivers/apple/gpio.h>
#include <platform/soc/hwregbase.h>
#include <target.h>

/* S7002 Target Pin Configuration */

static const uint32_t gpio_default_cfg[GPIO_0_GROUP_COUNT * GPIOPADPINS] = {
/* Port  0 */
	CFG_DISABLED,							// I2S0_MCK		->
	CFG_DISABLED,							// I2S1_MCK		->
	CFG_DISABLED,							// SD_CLKOUT		->
	CFG_DISABLED,							// SD_CMD_IO		->
	CFG_DISABLED,							// SD_DATA_IO[0]	->
	CFG_DISABLED,							// SD_DATA_IO[1]	->
	CFG_DISABLED,							// SD_DATA_IO[2]	->
	CFG_DISABLED,							// SD_DATA_IO[3]	->

/* Port  1 */
	CFG_DISABLED,							// SDIO_IRQ		->
	CFG_DISABLED,							// WL_HOST_WAKE		->
	CFG_DISABLED,							// NULL			->
	CFG_DISABLED,							// NULL			->
	CFG_DISABLED,							// NULL			->
	CFG_DISABLED,							// NULL			->
	CFG_DISABLED,							// NULL			->
	CFG_DISABLED,							// NULL			->

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
	CFG_DISABLED,							// GPIO0		-> 
	CFG_DISABLED,							// GPIO1		-> 
	CFG_DISABLED,							// GPIO2		-> BOOT_CONFIG[0]
	CFG_DISABLED,							// GPIO3		-> BOOT_CONFIG[1]
	CFG_DISABLED,							// GPIO4		-> BOOT_CONFIG[2]
	CFG_DISABLED,							// GPIO5		-> BOOT_CONFIG[3]
	CFG_DISABLED,							// GPIO6		-> BOARD_ID[3]
	CFG_DISABLED,							// GPIO7		->

/* Port  5 */
	CFG_DISABLED,							// GPIO8		-> 
	CFG_DISABLED,							// GPIO9		-> 
	CFG_DISABLED,							// GPIO10		-> 
	CFG_DISABLED,							// GPIO11		-> 
	CFG_DISABLED,							// GPIO12		->
	CFG_DISABLED,							// GPIO13		->
	CFG_DISABLED,							// GPIO14		->
	CFG_DISABLED,							// GPIO15		->

/* Port  6 */
	CFG_DISABLED,							// I2C1_SDA		->
	CFG_DISABLED,							// I2C1_SCL		->
	CFG_DISABLED,							// I2S0_LRCK		->
	CFG_DISABLED,							// I2S0_BCLK		->
	CFG_DISABLED,							// I2S0_DOUT		->
	CFG_DISABLED,							// I2S0_DIN		->
	CFG_DISABLED,							// I2S1_LRCK		->
	CFG_DISABLED,							// I2S1_BCLK		->

/* Port  7 */
	CFG_DISABLED,							// I2S1_DOUT		->
	CFG_DISABLED,							// I2S1_DIN		->
	CFG_DISABLED,							// UART1_TXD		->
	CFG_DISABLED,							// UART1_RXD		->
	CFG_DISABLED,							// UART1_RTSN		->
	CFG_DISABLED,							// UART1_CTSN		->
	CFG_DISABLED,							// UART2_TXD		->
	CFG_DISABLED,							// UART2_RXD		->

/* Port  8 */
	CFG_DISABLED,							// UART2_RTSN		->
	CFG_DISABLED,							// UART2_CTSN		->
	CFG_DISABLED,							// UART4_TXD		->
	CFG_DISABLED,							// UART4_RXD		->
	CFG_DISABLED,							// UART3_RTXD		->
	CFG_DISABLED,							// CLK32K_OUT		->
	CFG_DISABLED,							// TEST_CLKOUT		->
	CFG_DISABLED,							// GPIO16		-> 

/* Port  9 */
	CFG_DISABLED,							// GPIO17		->
	CFG_DISABLED,							// GPIO18		-> 
	CFG_DISABLED,							// TMR32_PWM1		->
	CFG_DISABLED,							// TMR32_PWM2		->
	CFG_DISABLED,							// TMR32_PWM3		->
	CFG_DISABLED,							// DISP_TE		->
	CFG_DISABLED,							// GPIO19		->
	CFG_DISABLED,							// NAND_CEN[0]		->

/* Port  10 */
	CFG_DISABLED,							// NAND_CEN[1]		->
	CFG_DISABLED,							// NAND_IO[7]		->
	CFG_DISABLED,							// NAND_IO[6]		->
	CFG_DISABLED,							// NAND_IO[5]		->
	CFG_DISABLED,							// NAND_IO[4]		->
	CFG_DISABLED,							// NAND_DQS		->
	CFG_DISABLED,							// NAND_REN		->
	CFG_DISABLED,							// NAND_IO[3]		->

/* Port  11 */
	CFG_DISABLED,							// NAND_IO[2]		->
	CFG_DISABLED,							// NAND_IO[1]		->
	CFG_DISABLED,							// NAND_IO[0]		->
	CFG_DISABLED,							// NAND_WEN		->
	CFG_DISABLED,							// NAND_CLE		->
	CFG_DISABLED,							// NAND_ALE		->
	CFG_DISABLED,							// I2C0_SDA		->
	CFG_DISABLED,							// I2C0_SCL		->

/* Port  12 */
	CFG_DISABLED,							// DISPLAY_SYNC		->
	CFG_DISABLED,							// SPI0_SCLK		-> SPI0_SCLK/BOARD_ID[0]
	CFG_DISABLED,							// SPI0_MOSI		-> SPI0_MOSI/BOARD_ID[1]
	CFG_DISABLED,							// SPI0_MISO		-> SPI0_MISO/BOARD_ID[2]
	CFG_DISABLED,							// SPI0_SSIN		->
	CFG_DISABLED,							// UART0_TXD		->
	CFG_DISABLED,							// UART0_RXD		->
	CFG_DISABLED,							// SPI1_SCLK		->

/* Port  13 */
	CFG_DISABLED,							// SPI1_MOSI		->
	CFG_DISABLED,							// SPI1_MISO		->
	CFG_DISABLED,							// SPI1_SSIN		->
	CFG_DISABLED,							// GPIO20		->
	CFG_DISABLED,							// GPIO21		->
	CFG_DISABLED,							// GPIO22		->
	CFG_DISABLED,							// GPIO23		->
	CFG_DISABLED,							// GPIO24		->

/* Port  14 */
	CFG_DISABLED,							// GPIO25		-> 
	CFG_DISABLED,							// NULL			->
	CFG_DISABLED,							// NULL			->
	CFG_DISABLED,							// NULL			->
	CFG_DISABLED,							// NULL			->
	CFG_DISABLED,							// NULL			->
	CFG_DISABLED,							// NULL			->
	CFG_DISABLED,							// NULL			->

/* Port  15 */
	CFG_DISABLED,							// NULL			->
	CFG_DISABLED,							// NULL			->
	CFG_DISABLED,							// NULL			->
	CFG_DISABLED,							// NULL			->
	CFG_DISABLED,							// NULL			->
	CFG_DISABLED,							// NULL			->
	CFG_DISABLED,							// NULL			->
	CFG_DISABLED,							// NULL			->

/* Port  16 */
	CFG_DISABLED,							// ENET_MDC		->
	CFG_DISABLED,							// ENET_MDIO		->
	CFG_DISABLED,							// RMII_RXER		->
	CFG_DISABLED,							// RMII_TXEN		->
	CFG_DISABLED,							// RMII_CLK		->
	CFG_DISABLED,							// RMII_TXD[0]		->
	CFG_DISABLED,							// RMII_TXD[1]		->
	CFG_DISABLED,							// RMII_RXD[0]		->

/* Port  17 */
	CFG_DISABLED,							// RMII_RXD[1]		->
	CFG_DISABLED,							// RMII_RXD[0]		->
	CFG_DISABLED,							// RMII_CRSDV		->
	CFG_DISABLED,							// NULL			->
	CFG_DISABLED,							// NULL			->
	CFG_DISABLED,							// NULL			->
	CFG_DISABLED,							// NULL			->
	CFG_DISABLED,							// NULL			->
};

static const uint32_t gpio_1_default_cfg[GPIO_1_GROUP_COUNT * GPIOPADPINS] = {
/* Port  0 */
	CFG_DISABLED,							// REQUEST_DFU1		->
	CFG_DISABLED,							// REQUEST_DFU2		->
	CFG_DISABLED,							// DOCK_CONNECT		->
	CFG_DISABLED,							// CLK32K_IN		->
	CFG_DISABLED,							// SPU_UART0_TXD	->
	CFG_DISABLED,							// SPU_UART0_RXD	->
	CFG_DISABLED,							// SPU_UART0_RTSN	->
	CFG_DISABLED,							// SPU_UART0_CTSN	->

/* Port  1 */
	CFG_DISABLED,							// SPU_UART1_TXD	->
	CFG_DISABLED,							// SPU_UART1_RXD	->
	CFG_DISABLED,							// SPU_UART1_RTSN	->
	CFG_DISABLED,							// SPU_UART1_CTSN	->
	CFG_DISABLED,							// SPU_GPIO[0]		->
	CFG_DISABLED,							// SPU_GPIO[1]		->
	CFG_DISABLED,							// SPU_GPIO[2]		->
	CFG_DISABLED,							// SPU_GPIO[3]		->

/* Port  2 */
	CFG_DISABLED,							// SPU_GPIO[4]		->
	CFG_DISABLED,							// SPU_GPIO[5]		->
	CFG_DISABLED,							// SPU_GPIO[6]		->
	CFG_DISABLED,							// SPU_GPIO[7]		->
	CFG_DISABLED,							// SPU_GPIO[8]		->
	CFG_DISABLED,							// DFU_STATUS		->
	CFG_DISABLED,							// FORCE_DFU		->
	CFG_DISABLED,							// POWER_GOOD		->

/* Port  3 */
	CFG_DISABLED,							// SOC_VDD_HI_LO	->
	CFG_DISABLED,							// SOC_VDD_ALL_ON	->
	CFG_DISABLED,							// DOCK_ATTENTION	->
	CFG_DISABLED,							// PMU_HOST_WAKE	->
	CFG_DISABLED,							// SPU_SPI_SCLK		->
	CFG_DISABLED,							// SPU_SPI_MOSI		->
	CFG_DISABLED,							// SPU_SPI_MISO		->
	CFG_DISABLED,							// SPU_SPI_CS_TRIG[0]	->

/* Port  4 */
	CFG_DISABLED,							// SPU_SPI_CS_TRIG[1]	->
	CFG_DISABLED,							// SPU_SPI_CS_TRIG[2]	->
	CFG_DISABLED,							// SPU_SPI_CS_TRIG[3]	->
	CFG_DISABLED,							// SPU_SPI_CS_TRIG[4]	->
	CFG_DISABLED,							// SPU_SPI_CS_TRIG[5]	->
	CFG_DISABLED,							// SPU_SPI_CS_TRIG[6]	->
	CFG_DISABLED,							// SPU_SPI_CS_TRIG[7]	->
	CFG_DISABLED,							// SPU_SPI_CS_TRIG[8]	->

/* Port  5 */
	CFG_DISABLED,							// SPU_SPI_CS_TRIG[9]	->
	CFG_DISABLED,							// SPU_SPI_CS_TRIG[10]	->
	CFG_DISABLED,							// SPU_SPI_CS_TRIG[11]	->
	CFG_DISABLED,							// SPU_SPI_CS_TRIG[12]	->
	CFG_DISABLED,							// SPU_SPI_CS_TRIG[13]	->
	CFG_DISABLED,							// SPU_SPI_CS_TRIG[14]	->
	CFG_DISABLED,							// SPU_SPI_CS_TRIG[15]	->
	CFG_DISABLED,							// SPU_GPIO[9]		->

/* Port  6 */
	CFG_DISABLED,							// SPU_I2C_SDA		->
	CFG_DISABLED,							// SPU_I2C_SCL		->
	CFG_DISABLED,							// WDOG			->
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
	CFG_DISABLED,							// NULL			->
	CFG_DISABLED,							// RESETN		->	RESET_L
	CFG_DISABLED,							// JTAG_TRSTN		->
	CFG_DISABLED,							// JTAG_SEL		->
	CFG_DISABLED,							// JTAG_TCK		->
	CFG_DISABLED,							// JTAG_TMS		->
	CFG_DISABLED,							// JTAG_TRTCK		->
	CFG_DISABLED,							// JTAG_TDO		->

/* Port  9 */
	CFG_DISABLED,							// JTAG_TDI		->
	CFG_DISABLED,							// NULL			->
	CFG_DISABLED,							// NULL			->
	CFG_DISABLED,							// NULL			->
	CFG_DISABLED,							// NULL			->
	CFG_DISABLED,							// NULL			->
	CFG_DISABLED,							// NULL			->
	CFG_DISABLED,							// NULL			->
};

const uint32_t *target_get_default_gpio_cfg(uint32_t gpioc)
{
	switch (gpioc) {
		case 0:
			return gpio_default_cfg;
		case 1:
			return gpio_1_default_cfg;
		default:
			panic("unknown GPIO controller");
	}
}
