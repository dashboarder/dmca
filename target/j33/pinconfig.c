/*
 * Copyright (C) 2009-2014 Apple Inc. All rights reserved.
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
#include <target.h>

/* S5L8942X FPGA Pin Configuration */
#define DFU_STATUS_DRIVE_STR	DRIVE_X1
#define FMI_DRIVE_STR		DRIVE_X2

static const u_int32_t gpio_default_cfg[GPIO_GROUP_COUNT * GPIOPADPINS] = {
/* Port  0 */
	CFG_IN,								// GPIO0		-> MENU_KEY (REQUEST_DFU2)
	CFG_IN,								// GPIO1		-> HOLD_KEY (REQUEST_DFU1)
	CFG_DISABLED,							// GPIO2		->
	CFG_DISABLED,							// GPIO3		->
	CFG_DISABLED,							// GPIO4		->
	CFG_DISABLED,							// GPIO5		->
	CFG_DISABLED,							// GPIO6		->
	CFG_DISABLED,							// GPIO7		->

/* Port  1 */
	CFG_OUT_1,							// GPIO8		-> BT_EN
	CFG_IN,								// GPIO9		-> BT_WAKE
	CFG_DISABLED,							// GPIO10		->
	CFG_IN,								// GPIO11		-> LAN_HSIC_DEVICE_RDY
	CFG_DISABLED,							// GPIO12		->
	CFG_IN | PULL_UP,						// GPIO13		-> PMU_IRQ_L
	CFG_DISABLED,							// GPIO14		->
	CFG_DISABLED,							// GPIO15		->

/* Port  2 */
	CFG_IN | PULL_DOWN,						// GPIO16		-> BOARD_ID[3]
	CFG_DISABLED,							// GPIO17		->
	CFG_IN | PULL_DOWN,						// GPIO18		-> BOOT_CONFIG[0]
	CFG_OUT_0,							// GPIO19		-> KEEPACT
	CFG_IN,								// GPIO20		-> WLAN0_HSIC_DEVICE_READY
	CFG_IN,								// GPIO21		-> USB_DEVMUX_SEL_C0
	CFG_IN,								// GPIO22		-> USB_DEVMUX_SEL_C0
	CFG_DISABLED,							// GPIO23		->

/* Port  3 */
	CFG_DISABLED,							// GPIO24		->
	CFG_IN | PULL_DOWN,						// GPIO25		-> BOOT_CONFIG[1]
	CFG_IN | PULL_DOWN,						// GPIO26		-> FORCE_DFU
	CFG_OUT_0 | DFU_STATUS_DRIVE_STR | PULL_DOWN,			// GPIO27		-> DFU_STATUS
	CFG_IN | PULL_DOWN,						// GPIO28		-> BOOT_CONFIG[2]
	CFG_IN | PULL_DOWN,						// GPIO29		-> BOOT_CONFIG[3]
	CFG_DISABLED,							// GPIO30		->
	CFG_OUT,							// GPIO31		-> WLAN0_HSIC_HOST_READY

/* Port  4 */
	CFG_DISABLED,							// GPIO32		->
	CFG_DISABLED,							// GPIO33		->
	CFG_DISABLED,							// GPIO34		->
	CFG_IN,								// GPIO35		-> LAN_PHY_INT
	CFG_DISABLED,							// GPIO36		->
	CFG_IN | PULL_DOWN,						// EHCI_PORT_PWR[0]	-> BOARD_REV[0]
	CFG_IN | PULL_DOWN,						// EHCI_PORT_PWR[1]	-> BOARD_REV[1]
	CFG_IN | PULL_DOWN,						// EHCI_PORT_PWR[2]	-> BOARD_REV[2]

/* Port  5 */
	CFG_FUNC0,							// UART1_TXD		-> BT_UART_TXD
	CFG_FUNC0,							// UART1_RXD		-> BT_UART_RXD
	CFG_OUT_1,							// UART1_RTSN		-> BT_UART_RTSN
	CFG_FUNC0,							// UART1_CTSN		-> BT_UART_CTSN
	CFG_FUNC0,							// UART2_TXD		-> AP_UART2_TXD
	CFG_FUNC0,							// UART2_RXD		-> AP_UART2_RXD
	CFG_DISABLED,							// UART2_RTSN		->
	CFG_DISABLED,							// UART2_CTSN		->

/* Port  6 */
	CFG_FUNC1,							// UART3_TXD		-> AP_SPDIF_OUT_1V8
	CFG_DISABLED,							// UART3_RXD		->
	CFG_DISABLED,							// UART3_RTSN		->
	CFG_DISABLED,							// UART3_CTSN		->
	CFG_FUNC0,							// I2C0_SDA		-> I2C0_SDA_1V8
	CFG_FUNC0,							// I2C0_SCL		-> I2C0_SCL_1V8
	CFG_DISABLED,							// I2C1_SDA		->
	CFG_DISABLED,							// I2C1_SCL		->

/* Port  7 */
	CFG_DISABLED,							// ISP0_SDA		->
	CFG_DISABLED,							// ISP0_SCL		->
	CFG_DISABLED,							// ISP1_SDA		->
	CFG_DISABLED,							// ISP1_SCL		->
	CFG_DISABLED,							// UART5_TXD/MIPI_VSYNC	->
	CFG_IN | PULL_DOWN,						// TMR32_PWM0		-> BOARD_REV[3]
	CFG_DISABLED,							// TMR32_PWM1		-> IRRCVR_OUT_TO_AP_AMR_R
	CFG_DISABLED,							// TMR32_PWM2

/* Port  8 */
	CFG_FUNC0,							// SWI_DATA		-> SWI_AP
	CFG_DISABLED,							// DWI_DI		->
	CFG_DISABLED,							// DWI_DO		->
	CFG_DISABLED,							// DWI_CLK		->
	CFG_DISABLED,							// SENSOR0_RST		->
	CFG_DISABLED,							// SENSOR0_CLK		->
	CFG_DISABLED,							// SENSOR1_RST		->
	CFG_DISABLED,							// SENSOR1_CLK		->

/* Port  9 */
	CFG_DISABLED,							// ISP0_PRE_FLASH	->
	CFG_DISABLED,							// ISP0_FLASH		->
	CFG_DISABLED,							// ISP1_PRE_FLASH	->
	CFG_DISABLED,							// ISP1_FLASH		->
	CFG_DISABLED,							// I2S0_MCK		->
	CFG_FUNC0,							// I2S0_LRCK		-> AP_I2S0_LRCK
	CFG_FUNC0,							// I2S0_SCLK		-> AP_I2S0_SCLK
	CFG_FUNC0,							// I2S0_DOUT		-> AP_I2S0_DOUT

/* Port 10 */
	CFG_DISABLED,							// I2S0_DIN		->
	CFG_DISABLED,							// I2S1_MCK		->
	CFG_DISABLED,							// I2S1_LRCK		->
	CFG_DISABLED,							// I2S1_SCLK		->
	CFG_DISABLED,							// I2S1_DOUT		->
	CFG_DISABLED,							// I2S1_DIN		->
	CFG_DISABLED,							// I2S2_MCK		->
	CFG_DISABLED,							// I2S2_LRCK		->

/* Port 11 */
	CFG_DISABLED,							// I2S2_BCLK		->
	CFG_DISABLED,							// I2S2_DOUT		->
	CFG_DISABLED,							// I2S2_DIN		->
	CFG_DISABLED,							// I2S2_MCK		->
	CFG_DISABLED,							// I2S3_LRCK		->
	CFG_DISABLED,							// I2S3_BCLK		->
	CFG_DISABLED,							// I2S3_DOUT		->
	CFG_DISABLED,							// I2S3_DIN		->

/* Port 12 */
	CFG_IN | PULL_DOWN,						// SPI0_SCLK		-> BOARD_ID[0]
	CFG_IN | PULL_DOWN,						// SPI0_MOSI		-> BOARD_ID[1]
	CFG_IN | PULL_DOWN,						// SPI0_MISO		-> BOARD_ID[2]
	CFG_DISABLED | PULL_UP,						// SPI0_SSIN		->
	CFG_DISABLED,							// SPI1_SCLK		->
	CFG_DISABLED,							// SPI1_MOSI		->
	CFG_DISABLED,							// SPI1_MISO		->
	CFG_DISABLED,							// SPI1_SSIN		->

/* Port 13 */
	CFG_DISABLED,							// SPI2_SCLK		->
	CFG_DISABLED,							// SPI2_MOSI		->
	CFG_DISABLED,							// SPI2_MISO		->
	CFG_DISABLED,							// SPI2_SSIN		->
	CFG_DISABLED,							// 			->
	CFG_DISABLED,							// 			->
	CFG_DISABLED,							// 			->
	CFG_DISABLED,							// 			->

/* Port 14 */
	CFG_DISABLED,							// 			->
	CFG_DISABLED,							// 			->
	CFG_DISABLED,							// 			->
	CFG_DISABLED,							// 			->
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
	CFG_FUNC0,							// UART4_TXD		-> UART4_TXD
	CFG_FUNC0,							// UART4_RXD		-> UART4_RXD
	CFG_DISABLED,							// UART4_RTSN		->
	CFG_DISABLED,							// UART4_CTSN		->
	CFG_IN,								// SDIO0_DATA3		-> AP_MCU_RESET
	CFG_IN,								// SDIO0_DATA2		-> AP_MCU_INT
	CFG_OUT,							// SDIO0_DATA1		-> PME_MODE_SEL
	CFG_IN,								// SDIO0_DATA0		-> AP_MCU_TCK_3V0

/* Port 17 */
	CFG_DISABLED,							// SDIO0_CMD		->
	CFG_DISABLED,							// SDIO0_CLK		->
	CFG_DISABLED,							// FMI0_CEN3		->
	CFG_DISABLED,							// FMI0_CEN2		->
	CFG_FUNC0 | FMI_DRIVE_STR,					// FMI0_CEN1		-> FMI0_CEN1
	CFG_FUNC0 | FMI_DRIVE_STR,					// FMI0_CEN0		-> FMI0_CEN0
	CFG_FUNC0 | FMI_DRIVE_STR,					// FMI0_CLE		-> FMI0_CLE
	CFG_FUNC0 | FMI_DRIVE_STR,					// FMI0_ALE		-> FMI0_ALE

/* Port 18 */
	CFG_FUNC0 | FMI_DRIVE_STR,					// FMI0_REN		-> FMI0_REN
	CFG_FUNC0 | FMI_DRIVE_STR,					// FMI0_WEN		-> FMI0_WEN
	CFG_FUNC0 | BUS_HOLD | FMI_DRIVE_STR,				// FMI0_IO7		-> FMI0_IO7
	CFG_FUNC0 | BUS_HOLD | FMI_DRIVE_STR,				// FMI0_IO6		-> FMI0_IO6
	CFG_FUNC0 | BUS_HOLD | FMI_DRIVE_STR,				// FMI0_IO5		-> FMI0_IO5
	CFG_FUNC0 | BUS_HOLD | FMI_DRIVE_STR,				// FMI0_IO4		-> FMI0_IO4
	CFG_DISABLED,							// 			->
	CFG_DISABLED,							// 			->

/* Port 19 */
	CFG_DISABLED,							// 			->
	CFG_DISABLED,							// 			->
	CFG_DISABLED,							// 			->
	CFG_DISABLED,							// 			->
	CFG_DISABLED,							// 			->
	CFG_DISABLED,							// 			->
	CFG_DISABLED,							// 			->
	CFG_DISABLED,							// 			->

/* Port 20 */
	CFG_DISABLED | PULL_DOWN,					// FMI0_DQS		-> FMI0_DQS
	CFG_FUNC0 | BUS_HOLD | FMI_DRIVE_STR,				// FMI0_IO3		-> FMI0_IO3
	CFG_FUNC0 | BUS_HOLD | FMI_DRIVE_STR,				// FMI0_IO2		-> FMI0_IO2
	CFG_FUNC0 | BUS_HOLD | FMI_DRIVE_STR,				// FMI0_IO1		-> FMI0_IO1
	CFG_FUNC0 | BUS_HOLD | FMI_DRIVE_STR,				// FMI0_IO0		-> FMI0_IO0
	CFG_DISABLED,							// FMI1_CEN3		->
	CFG_DISABLED,							// FMI1_CEN2		->
	CFG_FUNC0 | FMI_DRIVE_STR,					// FMI1_CEN1		-> FMI1_CEN1

/* Port 21 */
	CFG_FUNC0 | FMI_DRIVE_STR,					// FMI1_CEN0		-> FMI1_CEN0
	CFG_FUNC0 | FMI_DRIVE_STR,					// FMI1_CLE		-> FMI1_CLE
	CFG_FUNC0 | FMI_DRIVE_STR,					// FMI1_ALE		-> FMI1_ALE
	CFG_FUNC0 | FMI_DRIVE_STR,					// FMI1_REN		-> FMI1_REN
	CFG_FUNC0 | FMI_DRIVE_STR,					// FMI1_WEN		-> FMI1_WEN
	CFG_FUNC0 | BUS_HOLD | FMI_DRIVE_STR,				// FMI1_IO7		-> FMI1_IO7
	CFG_FUNC0 | BUS_HOLD | FMI_DRIVE_STR,				// FMI1_IO6		-> FMI1_IO6
	CFG_FUNC0 | BUS_HOLD | FMI_DRIVE_STR,				// FMI1_IO5		-> FMI1_IO5

/* Port 22 */
	CFG_FUNC0 | BUS_HOLD | FMI_DRIVE_STR,				// FMI1_IO4		-> FMI1_IO4
	CFG_DISABLED | PULL_DOWN,					// FMI1_DQS		-> FMI1_DQS
	CFG_FUNC0 | BUS_HOLD | FMI_DRIVE_STR,				// FMI1_IO3		-> FMI1_IO3
	CFG_FUNC0 | BUS_HOLD | FMI_DRIVE_STR,				// FMI1_IO2		-> FMI1_IO2
	CFG_FUNC0 | BUS_HOLD | FMI_DRIVE_STR,				// FMI1_IO1		-> FMI1_IO1
	CFG_FUNC0 | BUS_HOLD | FMI_DRIVE_STR,				// FMI1_IO0		-> FMI1_IO7
	CFG_DISABLED,							// 			->
	CFG_DISABLED,							// 			->

/* Port 23 */
	CFG_DISABLED,							// 			->
	CFG_DISABLED,							// 			->
	CFG_DISABLED,							// 			->
	CFG_DISABLED,							// 			->
	CFG_DISABLED,							// 			->
	CFG_DISABLED,							// 			->
	CFG_DISABLED,							// 			->
	CFG_DISABLED,							// 			->

/* Port 24 */
	CFG_FUNC0,							// I2C2_SDA		-> I2C2_SDA_3V0
	CFG_FUNC0,							// I2C2_SCL		-> I2C2_SCL_3V0
	CFG_FUNC0,							// UART0_TXD		-> AP_UART0_TXD
	CFG_FUNC0,							// UART0_RXD		-> AP_UART0_RXD
	CFG_FUNC0,							// UART5_RTXD		->
	CFG_FUNC0,							// DP_HPD		->
	CFG_DISABLED,							// TST_CLKOUT		->
	CFG_DISABLED,							// TST_STPCLK		->

/* Port 25 */
	CFG_FUNC0,							// WDOG			-> AP_WDOG
	CFG_DISABLED,							// 			->
	CFG_DISABLED,							// 			->
	CFG_DISABLED,							// 			->
	CFG_DISABLED,							// 			->
	CFG_DISABLED,							// 			->
	CFG_DISABLED,							// 			->
	CFG_DISABLED,							// 			->
};

const uint32_t *target_get_default_gpio_cfg(uint32_t gpioc)
{
	return gpio_default_cfg;
}
