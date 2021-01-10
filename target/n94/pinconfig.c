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

/* Default N94 Hardware Pin Configuration - Proto3A */

#define	BB_UART_DRIVE_STR	DRIVE_X3
#define	UMTS_DRIVE_STR		DRIVE_X3
#define	BT_UART_DRIVE_STR	DRIVE_X3
#define	SWI_UART_DRIVE_STR	DRIVE_X3
#define	GPS_UART_DRIVE_STR	DRIVE_X3
#define	FMI_DRIVE_STR		DRIVE_X3
#define	GRAPE_DRIVE_STR		DRIVE_X3
#define	BB_SPI_DRIVE_STR	DRIVE_X3
#define	I2C_DRIVE_STR		DRIVE_X3
#define	CAM_CLK_DRIVE_STR	DRIVE_X3
#define	I2S_DRIVE_STR		DRIVE_X3
#define	DOCK_UART_DRIVE_STR	DRIVE_X3
#define	IPC_DRIVE_STR		DRIVE_X3

#define	CAM_I2C_DRIVE_STR	DRIVE_X2
#define	CAM_STROBE_DRIVE_STR	DRIVE_X2
#define	DWI_DRIVE_STR		DRIVE_X2
#define	NOR_DRIVE_STR		DRIVE_X2

static const u_int32_t gpio_default_cfg_ap[GPIO_GROUP_COUNT * GPIOPADPINS] = {
/* Port  0 */
	CFG_IN,								// GPIO0		-> MENU_KEY_BUFF_L (REQUEST_DFU2)
	CFG_IN,								// GPIO1		-> HOLD_KEY_L      (REQUEST_DFU1)
	CFG_IN | PULL_UP,						// GPIO2		-> VOL_DWN_L
	CFG_IN | PULL_UP,						// GPIO3		-> VOL_UP_L
	CFG_IN,								// GPIO4		-> RINGER_A
	CFG_OUT_0,							// GPIO5		-> SPKAMP_RESET_L
	CFG_DISABLED,							// GPIO6		->
	CFG_OUT_0,							// GPIO7		-> BT_WAKE

/* Port  1 */
	CFG_OUT_1,							// GPIO8		-> BT_RESET_L
	CFG_DISABLED,							// GPIO9		->
	CFG_IN,								// GPIO10		-> BB_RESET_L
	CFG_IN | PULL_DOWN,						// GPIO11		-> BB_RESET_DET_L
	CFG_IN | PULL_DOWN,						// GPIO12		-> IPC_SRDY
	CFG_IN | PULL_UP,						// GPIO13		-> PMU_IRQ_L
	CFG_IN,								// GPIO14		-> RADIO_ON_L
	CFG_IN,								// GPIO15		-> RESET_BB_PMU_L

/* Port  2 */
	CFG_DISABLED,							// GPIO16		-> BOARD_ID[3]
	CFG_IN | PULL_DOWN,						// GPIO17		-> BB_HSIC_RDY
	CFG_DISABLED,							// GPIO18		-> BOOT_CONFIG[0]
	CFG_OUT_0,							// GPIO19		-> KEEPACT
	CFG_IN,								// GPIO20		-> BB_EMERGENCY_DWLD_L
	CFG_IN | PULL_UP,						// GPIO21		-> GRAPE_INT_L
	CFG_OUT_0,							// GPIO22		-> LCD_RESET_L
	CFG_IN | PULL_UP,						// GPIO23		-> LCD_HIFA

/* Port  3 */
	CFG_DISABLED,							// GPIO24		->
	CFG_DISABLED,							// GPIO25		-> BOOT_CONFIG[1]
	CFG_IN | PULL_DOWN,						// GPIO26		-> FORCE_DFU
	CFG_DISABLED | PULL_DOWN,					// GPIO27		-> DFU_STATUS
	CFG_DISABLED,							// GPIO28		-> BOOT_CONFIG[2]
	CFG_DISABLED,							// GPIO29		-> BOOT_CONFIG[3]
	CFG_IN | PULL_UP,						// GPIO30		-> CODEC_INT_L
	CFG_IN,								// GPIO31		-> WLAN_HSIC_RDY

/* Port  4 */
	CFG_IN | PULL_DOWN,						// GPIO32		-> GYRO_INT2
	CFG_IN | PULL_DOWN,						// GPIO33		-> GYRO_INT
	CFG_IN | PULL_DOWN,						// GPIO34		-> COMPASS_BRD_INT
	CFG_DISABLED,							// GPIO35		->
	CFG_IN | PULL_UP,						// GPIO36		-> ACCEL_INT1_L
	CFG_IN | PULL_UP,						// GPIO37		-> ACCEL_INT2_L
	CFG_IN | PULL_UP,						// GPIO38		-> ALS_INT_L
	CFG_OUT_0,							// GPIO39		-> GRAPE_RESET_L

/* Port  5 */
	CFG_DISABLED,							// EHCI_PORT_PWR[0]	-> BOARD_REV[0]
	CFG_DISABLED,							// EHCI_PORT_PWR[1]	-> BOARD_REV[1]
	CFG_DISABLED,							// EHCI_PORT_PWR[2]	-> BOARD_REV[2]
	CFG_IN | BB_UART_DRIVE_STR,					// UART1_TXD		-> (BB USART)
	CFG_FUNC0,							// UART1_RXD		->
	CFG_FUNC0 | BB_UART_DRIVE_STR,					// UART1_RTSN		->
	CFG_FUNC0,							// UART1_CTSN		->
	CFG_OUT_0,							// UART2_TXD		-> HOST_WLAN_HSIC_RDY

/* Port  6 */
	CFG_IN,								// UART2_RXD		-> IPC_GPIO4
	CFG_DISABLED,							// UART2_RTSN		-> PMU_AMUX_AY_CTRL
	CFG_DISABLED,							// UART2_CTSN		-> PMU_AMUX_BY_CTRL
	CFG_FUNC0 | BT_UART_DRIVE_STR,					// UART3_TXD		-> (BT)
	CFG_FUNC0,							// UART3_RXD		->
	CFG_OUT_1 | BT_UART_DRIVE_STR,					// UART3_RTSN		->
	CFG_FUNC0,							// UART3_CTSN		->
	CFG_FUNC0 | SWI_UART_DRIVE_STR,					// UART5_RTXD		-> BATTERY_SWI

/* Port  7 */
	CFG_OUT_0,							// UART6_TXD		-> HOST_BB_HSIC_RDY
	CFG_DISABLED,							// UART6_RXD		->
	CFG_DISABLED,							// UART6_RTSN		->
	CFG_DISABLED,							// UART6_CTSN		->
	CFG_DISABLED,							// UART4_TXD/SPI4_MOSI	->
	CFG_DISABLED,							// UART4_RXD/SPI4_MISO	->
	CFG_DISABLED,							// UART4_RTSN/SPI4_SCLK	->
	CFG_DISABLED,							// UART4_CTSN/SPI4_SSIN	->

/* Port  8 */
	CFG_FUNC0 | FMI_DRIVE_STR,					// FMI0_CEN3		->
	CFG_FUNC0 | FMI_DRIVE_STR,					// FMI0_CEN2		->
	CFG_FUNC0 | FMI_DRIVE_STR,					// FMI0_CEN1		->
	CFG_FUNC0 | FMI_DRIVE_STR,					// FMI0_CEN0		->
	CFG_FUNC0 | FMI_DRIVE_STR,					// FMI0_CLE		->
	CFG_FUNC0 | FMI_DRIVE_STR,					// FMI0_ALE		->
	CFG_FUNC0 | FMI_DRIVE_STR,					// FMI0_REN		->
	CFG_FUNC0 | FMI_DRIVE_STR,					// FMI0_WEN		->

/* Port  9 */
	CFG_FUNC0 | BUS_HOLD | FMI_DRIVE_STR,				// FMI0_IO7		->
	CFG_FUNC0 | BUS_HOLD | FMI_DRIVE_STR,				// FMI0_IO6		->
	CFG_FUNC0 | BUS_HOLD | FMI_DRIVE_STR,				// FMI0_IO5		->
	CFG_FUNC0 | BUS_HOLD | FMI_DRIVE_STR,				// FMI0_IO4		->
	CFG_DISABLED | PULL_DOWN,					// FMI0_DQS		->
	CFG_FUNC0 | BUS_HOLD | FMI_DRIVE_STR,				// FMI0_IO3		->
	CFG_FUNC0 | BUS_HOLD | FMI_DRIVE_STR,				// FMI0_IO2		->
	CFG_FUNC0 | BUS_HOLD | FMI_DRIVE_STR,				// FMI0_IO1		->

/* Port 10 */
	CFG_FUNC0 | BUS_HOLD | FMI_DRIVE_STR,				// FMI0_IO0		->
	CFG_FUNC0 | FMI_DRIVE_STR,					// FMI1_CEN3		->
	CFG_FUNC0 | FMI_DRIVE_STR,					// FMI1_CEN2		->
	CFG_FUNC0 | FMI_DRIVE_STR,					// FMI1_CEN1		->
	CFG_FUNC0 | FMI_DRIVE_STR,					// FMI1_CEN0		->
	CFG_FUNC0 | FMI_DRIVE_STR,					// FMI1_CLE		->
	CFG_FUNC0 | FMI_DRIVE_STR,					// FMI1_ALE		->
	CFG_FUNC0 | FMI_DRIVE_STR,					// FMI1_REN		->

/* Port 11 */
	CFG_FUNC0 | FMI_DRIVE_STR,					// FMI1_WEN		->
	CFG_FUNC0 | BUS_HOLD | FMI_DRIVE_STR,				// FMI1_IO7		->
	CFG_FUNC0 | BUS_HOLD | FMI_DRIVE_STR,				// FMI1_IO6		->
	CFG_FUNC0 | BUS_HOLD | FMI_DRIVE_STR,				// FMI1_IO5		->
	CFG_FUNC0 | BUS_HOLD | FMI_DRIVE_STR,				// FMI1_IO4		->
	CFG_DISABLED | PULL_DOWN,					// FMI1_DQS		->
	CFG_FUNC0 | BUS_HOLD | FMI_DRIVE_STR,				// FMI1_IO3		->
	CFG_FUNC0 | BUS_HOLD | FMI_DRIVE_STR,				// FMI1_IO2		->

/* Port 12 */
	CFG_FUNC0 | BUS_HOLD | FMI_DRIVE_STR,				// FMI1_IO1		->
	CFG_FUNC0 | BUS_HOLD | FMI_DRIVE_STR,				// FMI1_IO0		->
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
	CFG_OUT_0,							// ISP0_PRE_FLASH	-> CAM0_VDDCORE_EN
	CFG_OUT_0,							// ISP0_FLASH		-> CAM0_RESET_L
	CFG_DISABLED,							// ISP1_PRE_FLASH	->
	CFG_OUT_0,							// ISP1_FLASH		-> CAM1_RESET_L
	CFG_FUNC0 | GRAPE_DRIVE_STR,					// SPI1_SCLK		-> SPI1_SCLK (GRAPE)
	CFG_FUNC0 | GRAPE_DRIVE_STR,					// SPI1_MOSI		-> SPI1_MOSI
	CFG_FUNC0,							// SPI1_MISO		-> SPI1_MISO
	CFG_OUT_1 | GRAPE_DRIVE_STR,					// SPI1_SSIN		-> SPI1_CS_L

/* Port 20 */
	CFG_FUNC0 | I2C_DRIVE_STR,					// I2C2_SDA		-> I2C2_SDA_1V8
	CFG_FUNC0 | I2C_DRIVE_STR,					// I2C2_SCL		-> I2C2_SCL_1V8
	CFG_DISABLED,							// SPI0_SCLK		-> SPI0_SCLK/BOARD_ID[0]
	CFG_DISABLED,							// SPI0_MOSI		-> SPI0_MOSI/BOARD_ID[1]
	CFG_DISABLED,							// SPI0_MISO		-> SPI0_MISO/BOARD_ID[2]
	CFG_DISABLED,							// SPI0_SSIN		-> SPI0_SSIN
	CFG_DISABLED,							// SPI2_SCLK		->
	CFG_DISABLED,							// SPI2_MOSI		->

/* Port 21 */
	CFG_DISABLED,							// SPI2_MISO		->
	CFG_OUT_0 | BB_SPI_DRIVE_STR,					// SPI2_SSIN		-> SPI2_MRDY / IPC_MRDY
	CFG_FUNC0 | I2C_DRIVE_STR,					// I2C0_SDA		-> I2C0_SDA_1V8
	CFG_FUNC0 | I2C_DRIVE_STR,					// I2C0_SCL		-> I2C0_SCL_1V8
	CFG_FUNC0 | I2C_DRIVE_STR,					// I2C1_SDA		-> I2C1_SDA_1V8
	CFG_FUNC0 | I2C_DRIVE_STR,					// I2C1_SCL		-> I2C1_SCL_1V8
	CFG_FUNC0 | CAM_I2C_DRIVE_STR,					// ISP0_SDA		->
	CFG_FUNC0 | CAM_I2C_DRIVE_STR,					// ISP0_SCL		->

/* Port 22 */
	CFG_FUNC0 | CAM_I2C_DRIVE_STR,					// ISP1_SDA		->
	CFG_FUNC0 | CAM_I2C_DRIVE_STR,					// ISP1_SCL		->
	CFG_DISABLED,							// SDIO_CLK		->
	CFG_DISABLED,							// SDIO_CMD		->
	CFG_DISABLED,							// SDIO_DATA0		->
	CFG_DISABLED,							// SDIO_DATA1		->
	CFG_DISABLED,							// SDIO_DATA2		->
	CFG_DISABLED,							// SDIO_DATA3		->

/* Port 23 */
	CFG_DISABLED,							// MIPI_VSYNC		->
	CFG_IN,								// TMR32_PWM0		-> GYRO_INT2 (also GPIO32)
	CFG_IN,								// TMR32_PWM1		-> ACCEL_INT2_L (also GPIO37)
	CFG_OUT_0 | CAM_STROBE_DRIVE_STR,				// TMR32_PWM2		-> CAM_STROBE_EN
	CFG_DISABLED,							// SWI_DATA		->
	CFG_FUNC0 | PULL_DOWN,						// DWI_DI		->
	CFG_FUNC0 | DWI_DRIVE_STR,					// DWI_DO		->
	CFG_FUNC0 | DWI_DRIVE_STR,					// DWI_CLK		->

/* Port 24 */
	CFG_OUT_0,							// SENSOR0_RST		-> CAM0_SHUTDOWN
	CFG_OUT_0 | CAM_CLK_DRIVE_STR,					// SENSOR0_CLK		-> CAM0_CLK
	CFG_OUT_1,							// SENSOR1_RST		-> CAM1_SHUTDOWN
	CFG_OUT_0 | CAM_CLK_DRIVE_STR,					// SENSOR1_CLK		-> CAM1_CLK
	CFG_FUNC0 | I2S_DRIVE_STR,					// I2S0_MCK		-> (codec ASP)
	CFG_FUNC0 | I2S_DRIVE_STR,					// I2S0_LRCK		->
	CFG_FUNC0 | I2S_DRIVE_STR,					// I2S0_BCLK		->
	CFG_FUNC0 | I2S_DRIVE_STR,					// I2S0_DOUT		->

/* Port 25 */
	CFG_FUNC0,							// I2S0_DIN		->
	CFG_DISABLED,							// I2S1_MCK		-> (BB)
	CFG_FUNC0 | I2S_DRIVE_STR,					// I2S1_LRCK		->
	CFG_FUNC0 | I2S_DRIVE_STR,					// I2S1_BCLK		->
	CFG_FUNC0 | I2S_DRIVE_STR,					// I2S1_DOUT		->
	CFG_FUNC0,							// I2S1_DIN		->
	CFG_DISABLED,							// I2S2_MCK		-> (codec VSP, bluetooth)
	CFG_FUNC0 | I2S_DRIVE_STR,					// I2S2_LRCK		->

/* Port 26 */
	CFG_FUNC0 | I2S_DRIVE_STR,					// I2S2_BCLK		->
	CFG_FUNC0 | I2S_DRIVE_STR,					// I2S2_DOUT		->
	CFG_FUNC0,							// I2S2_DIN		->
	CFG_DISABLED,							// I2S3_MCK		-> (codec XSP, Amp)
	CFG_FUNC0 | I2S_DRIVE_STR,					// I2S3_LRCK		->
	CFG_FUNC0 | I2S_DRIVE_STR,					// I2S3_BCLK		->
	CFG_FUNC0 | I2S_DRIVE_STR,					// I2S3_DOUT		->
	CFG_FUNC0,							// I2S3_DIN		->

/* Port 27 */
	CFG_DISABLED,							// SPDIF		-> BOARD_REV[3]
	CFG_OUT_0,							// GPIO217		-> VIDEO_AMP_EN_3V0
	CFG_OUT_1,							// GPIO218		-> HPHONE_REF_CTRL
	CFG_FUNC0,							// DP_HPD		->
	CFG_FUNC0 | DOCK_UART_DRIVE_STR,				// UART0_TXD		->
	CFG_FUNC0,							// UART0_RXD		->
	CFG_DISABLED,							// TST_CLKOUT		->
	CFG_DISABLED,							// TST_STPCLK		->

/* Port 28 */
	CFG_FUNC0,							// WDOG			->
	CFG_DISABLED,							// 			->
	CFG_DISABLED,							// 			->
	CFG_DISABLED,							// 			->
	CFG_DISABLED,							// 			->
	CFG_DISABLED,							// 			->
	CFG_DISABLED,							// 			->
	CFG_DISABLED,							// 			->
};

#undef BB_UART_DRIVE_STR
#undef UMTS_DRIVE_STR
#undef BT_UART_DRIVE_STR
#undef SWI_UART_DRIVE_STR
#undef GPS_UART_DRIVE_STR
#undef FMI_DRIVE_STR
#undef GRAPE_DRIVE_STR
#undef BB_SPI_DRIVE_STR
#undef I2C_DRIVE_STR
#undef CAM_CLK_DRIVE_STR
#undef I2S_DRIVE_STR
#undef DOCK_UART_DRIVE_STR
#undef IPC_DRIVE_STR

#define	BB_UART_DRIVE_STR	DRIVE_X4
#define	UMTS_DRIVE_STR		DRIVE_X4
#define	BT_UART_DRIVE_STR	DRIVE_X4
#define	SWI_UART_DRIVE_STR	DRIVE_X4
#define	GPS_UART_DRIVE_STR	DRIVE_X4
#define	FMI_DRIVE_STR		DRIVE_X4
#define	GRAPE_DRIVE_STR		DRIVE_X4
#define	BB_SPI_DRIVE_STR	DRIVE_X4
#define	I2C_DRIVE_STR		DRIVE_X4
#define	CAM_CLK_DRIVE_STR	DRIVE_X4
#define	I2S_DRIVE_STR		DRIVE_X4
#define	DOCK_UART_DRIVE_STR	DRIVE_X4
#define	IPC_DRIVE_STR		DRIVE_X4

static const u_int32_t gpio_default_cfg_dev[GPIO_GROUP_COUNT * GPIOPADPINS] = {
/* Port  0 */
	CFG_IN,								// GPIO0		-> MENU_KEY_BUFF_L (REQUEST_DFU2)
	CFG_IN,								// GPIO1		-> HOLD_KEY_L      (REQUEST_DFU1)
	CFG_IN | PULL_UP,						// GPIO2		-> VOL_DWN_L
	CFG_IN | PULL_UP,						// GPIO3		-> VOL_UP_L
	CFG_IN,								// GPIO4		-> RINGER_A
	CFG_OUT_0,							// GPIO5		-> SPKAMP_RESET_L
	CFG_DISABLED,							// GPIO6		->
	CFG_OUT_0,							// GPIO7		-> BT_WAKE

/* Port  1 */
	CFG_OUT_1,							// GPIO8		-> BT_RESET_L
	CFG_DISABLED,							// GPIO9		->
	CFG_IN,								// GPIO10		-> BB_RESET_L
	CFG_IN | PULL_DOWN,						// GPIO11		-> BB_RESET_DET_L
	CFG_IN | PULL_DOWN,						// GPIO12		-> IPC_SRDY
	CFG_IN | PULL_UP,						// GPIO13		-> PMU_IRQ_L
	CFG_IN,								// GPIO14		-> RADIO_ON_L
	CFG_IN,								// GPIO15		-> RESET_BB_PMU_L

/* Port  2 */
	CFG_DISABLED,							// GPIO16		-> BOARD_ID[3]
	CFG_IN | PULL_DOWN,						// GPIO17		-> BB_HSIC_RDY
	CFG_DISABLED,							// GPIO18		-> BOOT_CONFIG[0]
	CFG_OUT_0,							// GPIO19		-> KEEPACT
	CFG_IN,								// GPIO20		-> BB_EMERGENCY_DWLD_L
	CFG_IN | PULL_UP,						// GPIO21		-> GRAPE_INT_L
	CFG_OUT_0,							// GPIO22		-> LCD_RESET_L
	CFG_IN | PULL_UP,						// GPIO23		-> LCD_HIFA

/* Port  3 */
	CFG_DISABLED,							// GPIO24		->
	CFG_DISABLED,							// GPIO25		-> BOOT_CONFIG[1]
	CFG_IN | PULL_DOWN,						// GPIO26		-> FORCE_DFU
	CFG_DISABLED | PULL_DOWN,					// GPIO27		-> DFU_STATUS
	CFG_DISABLED,							// GPIO28		-> BOOT_CONFIG[2]
	CFG_DISABLED,							// GPIO29		-> BOOT_CONFIG[3]
	CFG_IN | PULL_UP,						// GPIO30		-> CODEC_INT_L
	CFG_IN,								// GPIO31		-> WLAN_HSIC_RDY

/* Port  4 */
	CFG_IN | PULL_DOWN,						// GPIO32		-> GYRO_INT2
	CFG_IN | PULL_DOWN,						// GPIO33		-> GYRO_INT
	CFG_IN | PULL_DOWN,						// GPIO34		-> COMPASS_BRD_INT
	CFG_DISABLED,							// GPIO35		->
	CFG_IN | PULL_UP,						// GPIO36		-> ACCEL_INT1_L
	CFG_IN | PULL_UP,						// GPIO37		-> ACCEL_INT2_L
	CFG_IN | PULL_UP,						// GPIO38		-> ALS_INT_L
	CFG_OUT_0,							// GPIO39		-> GRAPE_RESET_L

/* Port  5 */
	CFG_DISABLED,							// EHCI_PORT_PWR[0]	-> BOARD_REV[0]
	CFG_DISABLED,							// EHCI_PORT_PWR[1]	-> BOARD_REV[1]
	CFG_DISABLED,							// EHCI_PORT_PWR[2]	-> BOARD_REV[2]
	CFG_IN | BB_UART_DRIVE_STR,					// UART1_TXD		-> (BB USART)
	CFG_FUNC0,							// UART1_RXD		->
	CFG_FUNC0 | BB_UART_DRIVE_STR,					// UART1_RTSN		->
	CFG_FUNC0,							// UART1_CTSN		->
	CFG_OUT_0,							// UART2_TXD		-> HOST_WLAN_HSIC_RDY

/* Port  6 */
	CFG_IN,								// UART2_RXD		-> IPC_GPIO4
	CFG_DISABLED,							// UART2_RTSN		-> PMU_AMUX_AY_CTRL
	CFG_DISABLED,							// UART2_CTSN		-> PMU_AMUX_BY_CTRL
	CFG_FUNC0 | BT_UART_DRIVE_STR,					// UART3_TXD		-> (BT)
	CFG_FUNC0,							// UART3_RXD		->
	CFG_OUT_1 | BT_UART_DRIVE_STR,					// UART3_RTSN		->
	CFG_FUNC0,							// UART3_CTSN		->
	CFG_FUNC0 | SWI_UART_DRIVE_STR,					// UART5_RTXD		-> BATTERY_SWI

/* Port  7 */
	CFG_OUT_0,							// UART6_TXD		-> HOST_BB_HSIC_RDY
	CFG_DISABLED,							// UART6_RXD		->
	CFG_DISABLED,							// UART6_RTSN		->
	CFG_DISABLED,							// UART6_CTSN		->
	CFG_DISABLED,							// UART4_TXD/SPI4_MOSI	->
	CFG_DISABLED,							// UART4_RXD/SPI4_MISO	->
	CFG_DISABLED,							// UART4_RTSN/SPI4_SCLK	->
	CFG_DISABLED,							// UART4_CTSN/SPI4_SSIN	->

/* Port  8 */
	CFG_FUNC0 | FMI_DRIVE_STR,					// FMI0_CEN3		->
	CFG_FUNC0 | FMI_DRIVE_STR,					// FMI0_CEN2		->
	CFG_FUNC0 | FMI_DRIVE_STR,					// FMI0_CEN1		->
	CFG_FUNC0 | FMI_DRIVE_STR,					// FMI0_CEN0		->
	CFG_FUNC0 | FMI_DRIVE_STR,					// FMI0_CLE		->
	CFG_FUNC0 | FMI_DRIVE_STR,					// FMI0_ALE		->
	CFG_FUNC0 | FMI_DRIVE_STR,					// FMI0_REN		->
	CFG_FUNC0 | FMI_DRIVE_STR,					// FMI0_WEN		->

/* Port  9 */
	CFG_FUNC0 | BUS_HOLD | FMI_DRIVE_STR,				// FMI0_IO7		->
	CFG_FUNC0 | BUS_HOLD | FMI_DRIVE_STR,				// FMI0_IO6		->
	CFG_FUNC0 | BUS_HOLD | FMI_DRIVE_STR,				// FMI0_IO5		->
	CFG_FUNC0 | BUS_HOLD | FMI_DRIVE_STR,				// FMI0_IO4		->
	CFG_DISABLED | PULL_DOWN,					// FMI0_DQS		->
	CFG_FUNC0 | BUS_HOLD | FMI_DRIVE_STR,				// FMI0_IO3		->
	CFG_FUNC0 | BUS_HOLD | FMI_DRIVE_STR,				// FMI0_IO2		->
	CFG_FUNC0 | BUS_HOLD | FMI_DRIVE_STR,				// FMI0_IO1		->

/* Port 10 */
	CFG_FUNC0 | BUS_HOLD | FMI_DRIVE_STR,				// FMI0_IO0		->
	CFG_FUNC0 | FMI_DRIVE_STR,					// FMI1_CEN3		->
	CFG_FUNC0 | FMI_DRIVE_STR,					// FMI1_CEN2		->
	CFG_FUNC0 | FMI_DRIVE_STR,					// FMI1_CEN1		->
	CFG_FUNC0 | FMI_DRIVE_STR,					// FMI1_CEN0		->
	CFG_FUNC0 | FMI_DRIVE_STR,					// FMI1_CLE		->
	CFG_FUNC0 | FMI_DRIVE_STR,					// FMI1_ALE		->
	CFG_FUNC0 | FMI_DRIVE_STR,					// FMI1_REN		->

/* Port 11 */
	CFG_FUNC0 | FMI_DRIVE_STR,					// FMI1_WEN		->
	CFG_FUNC0 | BUS_HOLD | FMI_DRIVE_STR,				// FMI1_IO7		->
	CFG_FUNC0 | BUS_HOLD | FMI_DRIVE_STR,				// FMI1_IO6		->
	CFG_FUNC0 | BUS_HOLD | FMI_DRIVE_STR,				// FMI1_IO5		->
	CFG_FUNC0 | BUS_HOLD | FMI_DRIVE_STR,				// FMI1_IO4		->
	CFG_DISABLED | PULL_DOWN,					// FMI1_DQS		->
	CFG_FUNC0 | BUS_HOLD | FMI_DRIVE_STR,				// FMI1_IO3		->
	CFG_FUNC0 | BUS_HOLD | FMI_DRIVE_STR,				// FMI1_IO2		->

/* Port 12 */
	CFG_FUNC0 | BUS_HOLD | FMI_DRIVE_STR,				// FMI1_IO1		->
	CFG_FUNC0 | BUS_HOLD | FMI_DRIVE_STR,				// FMI1_IO0		->
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
	CFG_OUT_0,							// ISP0_PRE_FLASH	-> CAM0_VDDCORE_EN
	CFG_OUT_0,							// ISP0_FLASH		-> CAM0_RESET_L
	CFG_DISABLED,							// ISP1_PRE_FLASH	->
	CFG_OUT_0,							// ISP1_FLASH		-> CAM1_RESET_L
	CFG_FUNC0 | GRAPE_DRIVE_STR,					// SPI1_SCLK		-> SPI1_SCLK (GRAPE)
	CFG_FUNC0 | GRAPE_DRIVE_STR,					// SPI1_MOSI		-> SPI1_MOSI
	CFG_FUNC0,							// SPI1_MISO		-> SPI1_MISO
	CFG_OUT_1 | GRAPE_DRIVE_STR,					// SPI1_SSIN		-> SPI1_CS_L

/* Port 20 */
	CFG_FUNC0 | I2C_DRIVE_STR,					// I2C2_SDA		-> I2C2_SDA_1V8
	CFG_FUNC0 | I2C_DRIVE_STR,					// I2C2_SCL		-> I2C2_SCL_1V8
	CFG_DISABLED,							// SPI0_SCLK		-> SPI0_SCLK/BOARD_ID[0]
	CFG_DISABLED,							// SPI0_MOSI		-> SPI0_MOSI/BOARD_ID[1]
	CFG_DISABLED,							// SPI0_MISO		-> SPI0_MISO/BOARD_ID[2]
	CFG_DISABLED,							// SPI0_SSIN		-> SPI0_SSIN
	CFG_DISABLED,							// SPI2_SCLK		->
	CFG_DISABLED,							// SPI2_MOSI		->

/* Port 21 */
	CFG_DISABLED,							// SPI2_MISO		->
	CFG_OUT_0 | BB_SPI_DRIVE_STR,					// SPI2_SSIN		-> SPI2_MRDY / IPC_MRDY
	CFG_FUNC0 | I2C_DRIVE_STR,					// I2C0_SDA		-> I2C0_SDA_1V8
	CFG_FUNC0 | I2C_DRIVE_STR,					// I2C0_SCL		-> I2C0_SCL_1V8
	CFG_FUNC0 | I2C_DRIVE_STR,					// I2C1_SDA		-> I2C1_SDA_1V8
	CFG_FUNC0 | I2C_DRIVE_STR,					// I2C1_SCL		-> I2C1_SCL_1V8
	CFG_FUNC0 | CAM_I2C_DRIVE_STR,					// ISP0_SDA		->
	CFG_FUNC0 | CAM_I2C_DRIVE_STR,					// ISP0_SCL		->

/* Port 22 */
	CFG_FUNC0 | CAM_I2C_DRIVE_STR,					// ISP1_SDA		->
	CFG_FUNC0 | CAM_I2C_DRIVE_STR,					// ISP1_SCL		->
	CFG_DISABLED,							// SDIO_CLK		->
	CFG_DISABLED,							// SDIO_CMD		->
	CFG_DISABLED,							// SDIO_DATA0		->
	CFG_DISABLED,							// SDIO_DATA1		->
	CFG_DISABLED,							// SDIO_DATA2		->
	CFG_DISABLED,							// SDIO_DATA3		->

/* Port 23 */
	CFG_DISABLED,							// MIPI_VSYNC		->
	CFG_IN,								// TMR32_PWM0		-> GYRO_INT2 (also GPIO32)
	CFG_IN,								// TMR32_PWM1		-> ACCEL_INT2_L (also GPIO37)
	CFG_OUT_0 | CAM_STROBE_DRIVE_STR,				// TMR32_PWM2		-> CAM_STROBE_EN
	CFG_DISABLED,							// SWI_DATA		->
	CFG_FUNC0 | PULL_DOWN,						// DWI_DI		->
	CFG_FUNC0 | DWI_DRIVE_STR,					// DWI_DO		->
	CFG_FUNC0 | DWI_DRIVE_STR,					// DWI_CLK		->

/* Port 24 */
	CFG_OUT_0,							// SENSOR0_RST		-> CAM0_SHUTDOWN
	CFG_OUT_0 | CAM_CLK_DRIVE_STR,					// SENSOR0_CLK		-> CAM0_CLK
	CFG_OUT_1,							// SENSOR1_RST		-> CAM1_SHUTDOWN
	CFG_OUT_0 | CAM_CLK_DRIVE_STR,					// SENSOR1_CLK		-> CAM1_CLK
	CFG_FUNC0 | I2S_DRIVE_STR,					// I2S0_MCK		-> (codec ASP)
	CFG_FUNC0 | I2S_DRIVE_STR,					// I2S0_LRCK		->
	CFG_FUNC0 | I2S_DRIVE_STR,					// I2S0_BCLK		->
	CFG_FUNC0 | I2S_DRIVE_STR,					// I2S0_DOUT		->

/* Port 25 */
	CFG_FUNC0,							// I2S0_DIN		->
	CFG_DISABLED,							// I2S1_MCK		-> (BB)
	CFG_FUNC0 | I2S_DRIVE_STR,					// I2S1_LRCK		->
	CFG_FUNC0 | I2S_DRIVE_STR,					// I2S1_BCLK		->
	CFG_FUNC0 | I2S_DRIVE_STR,					// I2S1_DOUT		->
	CFG_FUNC0,							// I2S1_DIN		->
	CFG_DISABLED,							// I2S2_MCK		-> (codec VSP, bluetooth)
	CFG_FUNC0 | I2S_DRIVE_STR,					// I2S2_LRCK		->

/* Port 26 */
	CFG_FUNC0 | I2S_DRIVE_STR,					// I2S2_BCLK		->
	CFG_FUNC0 | I2S_DRIVE_STR,					// I2S2_DOUT		->
	CFG_FUNC0,							// I2S2_DIN		->
	CFG_DISABLED,							// I2S3_MCK		-> (codec XSP, Amp)
	CFG_FUNC0 | I2S_DRIVE_STR,					// I2S3_LRCK		->
	CFG_FUNC0 | I2S_DRIVE_STR,					// I2S3_BCLK		->
	CFG_FUNC0 | I2S_DRIVE_STR,					// I2S3_DOUT		->
	CFG_FUNC0,							// I2S3_DIN		->

/* Port 27 */
	CFG_FUNC0,							// SPDIF		-> BOARD_REV[3] / SPDIF
	CFG_OUT_0,							// GPIO217		-> VIDEO_AMP_EN_3V0
	CFG_OUT_1,							// GPIO218		-> HPHONE_REF_CTRL
	CFG_FUNC0,							// DP_HPD		->
	CFG_FUNC0 | DOCK_UART_DRIVE_STR,				// UART0_TXD		->
	CFG_FUNC0,							// UART0_RXD		->
	CFG_DISABLED,							// TST_CLKOUT		->
	CFG_DISABLED,							// TST_STPCLK		->

/* Port 28 */
	CFG_FUNC0,							// WDOG			->
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
	if (target_config_ap())
		return gpio_default_cfg_ap;
	else
		return gpio_default_cfg_dev;
}