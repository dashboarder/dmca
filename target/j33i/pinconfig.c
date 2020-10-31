/*
 * Copyright (C) 2009-2013 Apple Inc. All rights reserved.
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

/* S5L8947X FPGA Pin Configuration */
#define DFU_STATUS_DRIVE_STR	DRIVE_X1
#define FMI_DRIVE_STR		DRIVE_X2

static const u_int32_t gpio_default_cfg[GPIO_GROUP_COUNT * GPIOPADPINS] = {
/* Port 0 */
	CFG_IN,								// 00/I2S0_MCK		-> AP_TO_MCU_RESET_3V0_L
	CFG_IN,								// 01/I2S0_LRCK		-> AP_TO_MCU_TCK_3V0
	CFG_IN,								// 02/I2S0_BCLK		-> AP_MCU_INT
	CFG_DISABLED,							// 03/I2S0_DOUT		->
	CFG_OUT,							// 04/I2S0_DIN		-> VCORE_ADJ_R
	CFG_FUNC0,							// 05/UART0_TXD		-> UART0_TXD
	CFG_FUNC0,							// 06/UART0_RXD		-> UART0_RXD
	CFG_FUNC0,							// 07/UART1_TXD		-> UART1_TXD
/* Port 1 */
	CFG_FUNC0,							// 08/UART1_RXD		-> UART1_RXD
	CFG_FUNC0,							// 09/I2C1_SDA		-> I2C1_SDA_3V0
	CFG_FUNC0,							// 10/I2C1_SCL		-> I2C1_SCL_3V0
	CFG_FUNC0,							// 11/HDMI_HPD		-> HDMI_HDP
	CFG_FUNC0,							// 12/HDMI_CEC		-> HDMI_CEC
	CFG_FUNC0,							// 13/I2C2_SDA		-> AP_DDC_DATA_3V0
	CFG_FUNC0,							// 14/I2C2_SCL		-> AP_DDC_CLK_3V0
	CFG_FUNC0,							// 15/SPDIF		-> AP_SPDIF_OUT_R_3V0
/* Port 2 */
	CFG_IN,								// 16/GPIO22		-> USB_DEVMUX_SEL_C0,
	CFG_DISABLED,							// 17/GPIO23		->								
	CFG_FUNC0,							// 18/UART2_TXD		-> AP_UART2_TXD
	CFG_FUNC0,							// 19/UART2_RXD		-> AP_UART2_RXD
	CFG_OUT_1,							// 20/UART2_RTSN	-> AP_UART2_RTS_L
	CFG_FUNC0,							// 21/UART2_CTSN	-> AP_UART2_CTS_L
	CFG_FUNC0,							// 22/UART3_TXD		-> UART3_TXD
	CFG_FUNC0,							// 23/UART3_RXD		-> UART3_RXD
/* Port 3 */
	CFG_FUNC0,							// 24/UART4_TXD		-> UART4_TXD
	CFG_FUNC0,							// 25/UART4_RXD		-> UART4_RXD
	CFG_DISABLED,							// 26/TST_CLKOUT	->
	CFG_DISABLED,							// 27/TST_STPCLK	->
	CFG_FUNC0,							// 28/WDOG		-> AP_WDOG
	CFG_DISABLED,							// 29
	CFG_DISABLED,							// 30
	CFG_DISABLED,							// 31
/* Port 4 */
	CFG_FUNC0,							// 32/ENET_MDC		-> ENET_MDC
	CFG_FUNC0,							// 33/ENET_MDIO		-> ENET_MDIO
	CFG_FUNC0,							// 34/RMII_CLK		-> RMII_CLK
	CFG_FUNC0,							// 35/RMII_RXER		-> RMII_RXER
	CFG_FUNC0,							// 36/RMII_TXD0		-> RMII_TXD0
	CFG_FUNC0,							// 37/RMII_CRSDV	-> RMII_CRSDV
	CFG_FUNC0,							// 38/RMII_RXD0		-> RMII_RXD0
	CFG_FUNC0,							// 39/RMII_RXD1		-> RMII_RXD1
/* Port 5 */
	CFG_FUNC0,							// 40/RMII_TXD1		-> RMII_TXD1
	CFG_FUNC0,							// 41/RMII_TXEN		-> RMII_TXEN
	CFG_FUNC0 | FMI_DRIVE_STR,					// 42/FMI0_CEN1		-> FMI0_CEN1
	CFG_FUNC0 | FMI_DRIVE_STR,					// 43/FMI0_CEN0		-> FMI0_CEN0
	CFG_FUNC0 | FMI_DRIVE_STR,					// 44/FMI0_CLE		-> FMI0_CLE
	CFG_FUNC0 | FMI_DRIVE_STR,					// 45/FMI0_ALE		-> FMI0_ALE
	CFG_FUNC0 | FMI_DRIVE_STR,					// 46/FMI0_REN		-> FMI0_REN
	CFG_FUNC0 | FMI_DRIVE_STR,					// 47/FMI0_WEN		-> FMI0_WEN
/* Port 6 */	
	CFG_FUNC0 | BUS_HOLD | FMI_DRIVE_STR,				// 48/FMI0_IO7		-> FMI0_IO7
	CFG_FUNC0 | BUS_HOLD | FMI_DRIVE_STR,				// 49/FMI0_IO6		-> FMI0_IO6
	CFG_FUNC0 | BUS_HOLD | FMI_DRIVE_STR,				// 50/FMI0_IO5		-> FMI0_IO5
	CFG_FUNC0 | BUS_HOLD | FMI_DRIVE_STR,				// 51/FMI0_IO4		-> FMI0_IO4
	CFG_FUNC0 | BUS_HOLD | FMI_DRIVE_STR,				// 52/FMI0_DQS		-> FMI0_DQS
	CFG_FUNC0 | BUS_HOLD | FMI_DRIVE_STR,				// 53/FMI0_IO3		-> FMI0_IO3
	CFG_FUNC0 | BUS_HOLD | FMI_DRIVE_STR,				// 54/FMI0_IO2		-> FMI0_IO2
	CFG_FUNC0 | BUS_HOLD | FMI_DRIVE_STR,				// 55/FMI0_IO1		-> FMI0_IO1
/* Port 7 */	
	CFG_FUNC0 | BUS_HOLD | FMI_DRIVE_STR,				// 56/FMI0_IO0		-> FMI0_IO0
	CFG_FUNC0 | FMI_DRIVE_STR,					// 57/FMI1_CEN1		-> FMI1_CEN1
	CFG_FUNC0 | FMI_DRIVE_STR,					// 58/FMI1_CEN0		-> FMI1_CEN0
	CFG_FUNC0 | FMI_DRIVE_STR,					// 59/FMI1_CLE		-> FMI1_CLE
	CFG_FUNC0 | FMI_DRIVE_STR,					// 60/FMI1_ALE		-> FMI1_ALE
	CFG_FUNC0 | FMI_DRIVE_STR,					// 61/FMI1_REN		-> FMI1_REN
	CFG_FUNC0 | FMI_DRIVE_STR,					// 62/FMI1_WEN		-> FMI1_WEN
	CFG_FUNC0 | BUS_HOLD | FMI_DRIVE_STR,				// 63/FMI1_IO7		-> FMI1_IO7
/* Port 8 */
	CFG_FUNC0 | BUS_HOLD | FMI_DRIVE_STR,				// 64/FMI1_IO6		-> FMI1_IO6
	CFG_FUNC0 | BUS_HOLD | FMI_DRIVE_STR,				// 65/FMI1_IO5		-> FMI1_IO5
	CFG_FUNC0 | BUS_HOLD | FMI_DRIVE_STR,				// 66/FMI1_IO4		-> FMI1_IO4
	CFG_FUNC0 | BUS_HOLD | FMI_DRIVE_STR,				// 67/FMI1_DQS		-> FMI1_DQS
	CFG_FUNC0 | BUS_HOLD | FMI_DRIVE_STR,				// 68/FMI1_IO3		-> FMI1_IO3
	CFG_FUNC0 | BUS_HOLD | FMI_DRIVE_STR,				// 69/FMI1_IO2		-> FMI1_IO2
	CFG_FUNC0 | BUS_HOLD | FMI_DRIVE_STR,				// 70/FMI1_IO1		-> FMI1_IO1
	CFG_FUNC0 | BUS_HOLD | FMI_DRIVE_STR,				// 71/FMI1_IO0		-> FMI1_IO0
/* Port 9 */
	CFG_DISABLED,							// 72
	CFG_DISABLED,							// 73
	CFG_DISABLED,							// 74
	CFG_DISABLED,							// 75
	CFG_DISABLED,							// 76
	CFG_DISABLED,							// 77
	CFG_DISABLED,							// 78
	CFG_DISABLED,							// 79
/* Port 10 */
	CFG_DISABLED,							// 80
	CFG_DISABLED,							// 81
	CFG_DISABLED,							// 82
	CFG_DISABLED,							// 83
	CFG_DISABLED,							// 84
	CFG_DISABLED,							// 85
	CFG_DISABLED,							// 86
	CFG_DISABLED,							// 87
/* Port 11 */	
	CFG_DISABLED,							// 88
	CFG_DISABLED,							// 89
	CFG_DISABLED,							// 90
	CFG_DISABLED,							// 91
	CFG_DISABLED,							// 92
	CFG_DISABLED,							// 93
	CFG_DISABLED,							// 94
	CFG_DISABLED,							// 95
/* Port 12 */
	CFG_IN,								// 96/GPIO0		-> MENU_KEY (REQUEST_DFU2)
	CFG_IN,								// 97/GPIO1		-> HOLD_KEY (REQUEST_DFU1)
	CFG_DISABLED,							// 98/GPIO2		->
	CFG_IN | PULL_DOWN,						// 99/GPIO3		-> BOARD_REV[0]
	CFG_IN | PULL_DOWN,						// 00/GPIO4		-> BOARD_REV[1]
	CFG_IN | PULL_DOWN,						// 01/GPIO5		-> BOARD_REV[2]
	CFG_IN | PULL_DOWN,						// 02/GPIO6		-> BOARD_REV[3]
	CFG_DISABLED,							// 03/GPIO7		->
/* Port 13 */
	CFG_OUT_1,							// 04/GPIO8		-> BT_EN
	CFG_IN,								// 05/GPIO9		-> BT_WAKE
	CFG_DISABLED,							// 06/GPIO10		->
	CFG_IN,								// 07/GPIO11		-> LAN_HSIC_DEVICE_RDY
	CFG_DISABLED,							// 08/GPIO12		->
	CFG_IN | PULL_UP,						// 09/GPIO13		-> PMU_IRQ_L
	CFG_DISABLED,							// 10/GPIO14		->
	CFG_DISABLED,							// 11/GPIO15		->
/* Port 14 */
	CFG_IN | PULL_DOWN,						// 12/GPIO16		-> BOARD_ID[3]
	CFG_OUT,							// 13/GPIO17		-> WLAN0_HSIC_HOST_READY
	CFG_IN | PULL_DOWN,						// 14/GPIO18		-> BOOT_CONFIG[0]
	CFG_OUT_0,							// 15/GPIO19		-> KEEPACT
	CFG_DISABLED,							// 16
	CFG_DISABLED,							// 17
	CFG_DISABLED,							// 18
	CFG_DISABLED,							// 19
/* Port 15 */	
	CFG_DISABLED,							// 20
	CFG_DISABLED,							// 21
	CFG_DISABLED,							// 22
	CFG_DISABLED,							// 23
	CFG_DISABLED,							// 24
	CFG_DISABLED,							// 25
	CFG_DISABLED,							// 26
	CFG_DISABLED,							// 27
/* Port 16 */
	CFG_IN,								// 28/GPIO20		-> WLAN0_HSIC_DEVICE_READY
	CFG_IN,								// 29/GPIO21		-> USB_DEVMUX_SEL_C0
	CFG_DISABLED,							// 30/GPIO24		->
	CFG_IN | PULL_DOWN,						// 31/GPIO25		-> BOOT_CONFIG[1]
	CFG_IN | PULL_DOWN,						// 32/GPIO26		-> FORCE_DFU
	CFG_OUT_0 | DFU_STATUS_DRIVE_STR | PULL_DOWN,			// 33/GPIO27		-> DFU_STATUS
	CFG_IN | PULL_DOWN,						// 34/GPIO28		-> BOOT_CONFIG[2]
	CFG_IN | PULL_DOWN,						// 35/GPIO29		-> BOOT_CONFIG[3]
/* Port 17 */
	CFG_FUNC0,							// 36/I2C0_SDA		-> I2C0_SDA_1V8
	CFG_FUNC0,							// 37/I2C0_SCL		-> I2C0_SCL_1V8
	CFG_DISABLED,							// 38/TMR32_PWM0	-> 
	CFG_IN | PULL_DOWN,						// 39/SPI0_SCLK		-> BOARD_ID[0]
	CFG_IN | PULL_DOWN,						// 40/SPI0_MOSI		-> BOARD_ID[1]
	CFG_IN | PULL_DOWN,						// 41/SPI0_MISO		-> BOARD_ID[2]
	CFG_DISABLED | PULL_UP,						// 42/SPI0_SSIN		->
	CFG_FUNC0,							// 43/SWI_DATA		-> SWI_AP
};

const uint32_t *target_get_default_gpio_cfg(uint32_t gpioc)
{
	return gpio_default_cfg;
}
