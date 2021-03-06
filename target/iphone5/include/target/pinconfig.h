/*
 * Copyright (C) 2011-2014 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */

/* Default iPhone5,x SoC Pin Configuration */

#if PIN_CFG_AP
#define HI_DRIVE_STR    DRIVE_X4
#elif PIN_CFG_DEV
#define HI_DRIVE_STR    DRIVE_X6
#endif

#define	BB_UART_DRIVE_STR	DRIVE_X2
#define	BT_UART_DRIVE_STR	DRIVE_X2
#define	DOCK_UART_DRIVE_STR	DRIVE_X2
#define	SWI_UART_DRIVE_STR	DRIVE_X1
#define	WIFI_UART_DRIVE_STR	DRIVE_X2
#define	FMI_LO_SPEED_DRIVE_STR	(DRIVE_X2 | SLOW_SLEW)
#define	FMI_HI_SPEED_DRIVE_STR	(DRIVE_X2 | SLOW_SLEW)
#define	GRAPE_IO_DRIVE_STR	DRIVE_X1
#define	GRAPE_CLK_DRIVE_STR	DRIVE_X2
#define	DWI_DRIVE_STR		DRIVE_X2
#define	I2C_DRIVE_STR		DRIVE_X1
#define	I2S_DRIVE_STR		DRIVE_X2
#define	CAM_I2C_DRIVE_STR	DRIVE_X1
#define	CAM_STROBE_DRIVE_STR	DRIVE_X1
#define	CAM_CLK_DRIVE_STR	DRIVE_X2

#define DISABLE_OPTIONAL_SIGNALS (1)

#if PIN_CFG_AP
static const uint32_t gpio_default_cfg_ap[GPIO_GROUP_COUNT * GPIOPADPINS] = {
#elif PIN_CFG_DEV
static const uint32_t gpio_default_cfg_dev[GPIO_GROUP_COUNT * GPIOPADPINS] = {
#endif
/* Port  0 */
	CFG_IN,								// GPIO0		-> MENU_KEY_BUFF_L (REQUEST_DFU2)
	CFG_IN,								// GPIO1		-> HOLD_KEY_BUFF_L (REQUEST_DFU1)
	CFG_IN | PULL_UP,						// GPIO2		-> VOL_UP_L
	CFG_IN | PULL_UP,						// GPIO3		-> VOL_DWN_L
	CFG_IN,								// GPIO4		-> RINGER_A
	CFG_IN | PULL_UP,						// GPIO5		-> SPKAMP_INT_L
	CFG_IN | PULL_UP,						// GPIO6		-> PMU_IRQ_L
	CFG_OUT_0,							// GPIO7		-> BT_WAKE

/* Port  1 */
	CFG_IN,								// GPIO8		-> BB_PMU_FET_ON
	CFG_OUT_0,							// GPIO9		-> BEE_GEES
	CFG_IN,								// GPIO10		-> BB_HSIC1_REMOTE_WAKE
	CFG_IN | PULL_UP,						// GPIO11		-> WLAN_SDIO_DATA[2]
	CFG_IN | PULL_UP,						// GPIO12		-> WLAN_SDIO_DATA[1]
	CFG_IN | PULL_UP,						// GPIO13		-> WLAN_SDIO_DATA[0]
	CFG_IN,								// GPIO14		-> WLAN_SDIO_CMP
	CFG_IN,								// GPIO15		-> WLAN_SDIO_CLK

/* Port  2 */
	CFG_DISABLED,							// GPIO16		-> BOARD_ID[3]
	CFG_OUT_0,							// GPIO17		-> AP_HSIC1_RDY
	CFG_DISABLED,							// GPIO18		-> BOOT_CONFIG[0]
	CFG_OUT_0,							// GPIO19		-> KEEPACT
	CFG_DISABLED,							// EHCI_PORT_PWR[0]	-> BOARD_REV0
	CFG_DISABLED,							// EHCI_PORT_PWR[1]	-> BOARD_REV1
	CFG_DISABLED,							// EHCI_PORT_PWR[2]	-> BOARD_REV2
	CFG_DISABLED,							// EHCI_PORT_PWR[3]	-> BOARD_REV3

/* Port  3 */
	CFG_IN | BB_UART_DRIVE_STR,					// UART1_TXD		-> UART1_TXD
	CFG_FUNC0,							// UART1_RXD		-> UART1_RXD
	CFG_FUNC0 | BB_UART_DRIVE_STR,					// UART1_RTSN		-> UART1_RTS_L
	CFG_FUNC0,							// UART1_CTSN		-> UART1_CTS_L
	CFG_FUNC0 | DOCK_UART_DRIVE_STR,				// UART2_TXD		-> UART2_TXD
	CFG_FUNC0,							// UART2_RXD		-> UART2_RXD
	CFG_IN | PULL_DOWN,						// UART2_RTSN		-> ACCEL_INT1
	CFG_IN,								// UART2_CTSN		-> TRISTAR_INT

/* Port  4 */
	CFG_FUNC0 | BT_UART_DRIVE_STR,					// UART3_TXD		-> UART3_TXD
	CFG_FUNC0,							// UART3_RXD		-> UART3_RXD
	CFG_OUT_1 | BT_UART_DRIVE_STR,					// UART3_RTSN		-> UART3_RTS_L
	CFG_FUNC0,							// UART3_CTSN		-> UART3_CTS_L
	CFG_FUNC0 | WIFI_UART_DRIVE_STR,				// UART4_TXD/SPI4_MOSI	-> UART4_TXD
	CFG_FUNC0,							// UART4_RXD/SPI4_MISO	-> UART4_RXD
	CFG_OUT_0,							// UART4_RTSN/SPI4_SCLK	-> CAM0_VDDCORE_EN
#if PIN_CFG_AP
	CFG_OUT_0,							// UART4_CTSN/SPI4_SSIN	-> BB_JTAG_TRST_L
#elif PIN_CFG_DEV
	CFG_IN,							        // UART4_CTSN/SPI4_SSIN	-> LCD_ERSTB_L
#else
#error "unknown config"
#endif

/* Port  5 */
	CFG_FUNC0 | DOCK_UART_DRIVE_STR,				// UART6_TXD		-> UART6_TXD
	CFG_FUNC0,							// UART6_RXD		-> UART6_RXD
	CFG_OUT_0,							// UART6_RTSN		-> SPKAMP_RESET_L
	CFG_IN | PULL_DOWN,						// UART6_CTSN		-> BB_PP_SYNC
	CFG_FUNC1 | SWI_UART_DRIVE_STR,					// UART5_RXD/UART5_RTXD	-> UART5_RXD
	CFG_IN | PULL_DOWN,						// UART5_TXD		-> BB_RESET_DET_L
	CFG_FUNC0 | GRAPE_CLK_DRIVE_STR,				// SPI1_SCLK		-> SPI1_SCLK
	CFG_FUNC0 | GRAPE_IO_DRIVE_STR,					// SPI1_MOSI		-> SPI1_MOSI

/* Port  6 */
	CFG_FUNC0,							// SPI1_MISO		-> SPI1_MISO
	CFG_OUT_0 | GRAPE_IO_DRIVE_STR,					// SPI1_SSIN		-> SPI1_CS_L
	CFG_DISABLED,							// SPI0_SCLK		-> SPI0_SCLK/BOARD_ID[0]
	CFG_DISABLED,							// SPI0_MOSI		-> SPI0_MOSI/BOARD_ID[1]
	CFG_DISABLED,							// SPI0_MISO		-> SPI0_MISO/BOARD_ID[2]
	CFG_DISABLED,							// SPI0_SSIN		-> SPI0_SSIN
	CFG_OUT_0,							// SPI2_SCLK		-> AP_HSIC3_RDY
	CFG_OUT_0,							// SPI2_MOSI		-> PMU_AMUX_BY_CTRL

/* Port  7 */
	CFG_OUT_0,							// SPI2_MISO		-> PMU_AMUX_AY_CTRL
	CFG_IN | PULL_UP,						// SPI2_SSIN		-> DEV_HSIC3_RDY
	CFG_FUNC0 | I2C_DRIVE_STR,					// I2C0_SDA		-> I2C0_SDA_1V8
	CFG_FUNC0 | I2C_DRIVE_STR,					// I2C0_SCL		-> I2C0_SCL_1V8
	CFG_FUNC0 | I2C_DRIVE_STR,					// I2C1_SDA		-> I2C1_SDA_1V8
	CFG_FUNC0 | I2C_DRIVE_STR,					// I2C1_SCL		-> I2C1_SCL_1V8
	CFG_FUNC0 | CAM_I2C_DRIVE_STR,					// ISP0_SDA		-> CAM0_I2C_SDA
	CFG_FUNC0 | CAM_I2C_DRIVE_STR,					// ISP0_SCL		-> CAM0_I2C_SCL

/* Port  8 */
	CFG_FUNC0 | CAM_I2C_DRIVE_STR,					// ISP1_SDA		-> CAM1_I2C_SDA
	CFG_FUNC0 | CAM_I2C_DRIVE_STR,					// ISP1_SCL		-> CAM1_I2C_SCL
	CFG_DISABLED,							// MIPI_VSYNC		-> MIPI_VSYNC
	CFG_FUNC0 | PULL_DOWN,						// TMR32_PWM0		-> GYRO_INT2
	CFG_FUNC0,							// TMR32_PWM1		-> VIB_PWM
	CFG_FUNC0 | GRAPE_IO_DRIVE_STR,					// TMR32_PWM2		-> CLK32K_GRAPE_RESET_SOC_L
	CFG_DISABLED | PULL_DOWN,					// SWI_DATA		-> SWI_BLCTL
	CFG_FUNC0,							// DWI_DI		-> DWI_AP_DI

/* Port  9 */
	CFG_FUNC0 | DWI_DRIVE_STR,					// DWI_DO		-> DWI_AP_DO
	CFG_FUNC0 | DWI_DRIVE_STR,					// DWI_CLK		-> DWI_AP_CLK
	CFG_OUT_0,							// SENSOR0_RST		-> CAM0_SHUTDOWN
	CFG_FUNC0 | CAM_CLK_DRIVE_STR,					// SENSOR0_CLK		-> CAM0_CLK
	CFG_OUT_0,							// SENSOR1_RST		-> CAM1_SHUTDOWN
	CFG_FUNC0 | CAM_CLK_DRIVE_STR,					// SENSOR1_CLK		-> CAM1_CLK
	CFG_FUNC0,							// CPU0_SWITCH		-> CPU0_SWITCH
	CFG_FUNC0,							// CPU1_SWITCH		-> CPU1_SWITCH

/* Port 10 */
	CFG_OUT_0,							// ISP0_PRE_FLASH	-> CAM0_TORCH
	CFG_DISABLED,							// ISP0_FLASH		-> CLK32K_GRAPE_RESET_L
	CFG_DISABLED | PULL_DOWN,					// ISP1_PRE_FLASH	-> ISP1_PRE_FLASH
	CFG_DISABLED,							// ISP1_FLASH		-> 
	CFG_FUNC0,							// SPI3_MOSI		-> CODEC_SPI_DIN
	CFG_FUNC0,							// SPI3_MISO		-> CODEC_SPI_DOUT
	CFG_FUNC0,							// SPI3_SCLK		-> CODEC_SPI_CLK
	CFG_OUT_1,							// SPI3_SSIN		-> CODEC_SPI_CS

/* Port 11 */
	CFG_FUNC0,							// I2C2_SDA		-> I2C2_SDA_3V0
	CFG_FUNC0,							// I2C2_SCL		-> I2C2_SCL_3V0
	CFG_OUT_0,							// GPIO_3V0		-> HS3_CONTROL
	CFG_OUT_0,							// GPIO_3V1		-> HS4_CONTROL
	CFG_FUNC0 | PULL_DOWN,						// DP_HPD		-> DP_HPD
	CFG_DISABLED | PULL_DOWN,					// LPDP_HPD		-> LPDP_HPD
	CFG_OUT_1,							// UART0_TXD		-> FLASH_ENABLE
	CFG_OUT_0,							// UART0_RXD		-> VIB_PWM_EN

/* Port 12 */
	CFG_DISABLED,							// TST_CLKOUT		-> TST_CLKOUT
	CFG_DISABLED,							// TST_STPCLK		-> TST_STPCLK
	CFG_FUNC0,							// WDOG			-> PMU_RESET_IN
	CFG_DISABLED,							// 			-> 
	CFG_DISABLED,							// 			-> 
	CFG_DISABLED,							// 			-> 
	CFG_DISABLED,							// 			-> 
	CFG_DISABLED,							// 			-> 

/* Port 13 */
	CFG_DISABLED,							// 			-> 
	CFG_DISABLED,							// 			-> 
	CFG_DISABLED,							// 			-> 
	CFG_DISABLED,							// 			-> 
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
	CFG_FUNC0 | I2S_DRIVE_STR,					// I2S0_MCK		-> I2S0_MCLK
	CFG_FUNC0 | I2S_DRIVE_STR,					// I2S0_LRCK		-> I2S0_LRCLK
	CFG_FUNC0 | I2S_DRIVE_STR,					// I2S0_BCLK		-> I2S0_BCLK
	CFG_FUNC0 | I2S_DRIVE_STR,					// I2S0_DOUT		-> I2S0_DOUT
	CFG_FUNC0,							// I2S0_DIN		-> I2S0_DIN
	CFG_DISABLED,							// I2S1_MCK		-> I2S1_MCLK
	CFG_FUNC0 | I2S_DRIVE_STR,					// I2S1_LRCK		-> I2S1_LRCLK
	CFG_FUNC0 | I2S_DRIVE_STR,					// I2S1_BCLK		-> I2S1_BCLK

/* Port 17 */
	CFG_FUNC0 | I2S_DRIVE_STR,					// I2S1_DOUT		-> I2S1_DOUT
	CFG_FUNC0,							// I2S1_DIN		-> I2S1_DIN
	CFG_FUNC0 | I2S_DRIVE_STR,					// I2S2_MCK		-> I2S_MCLK
	CFG_FUNC0 | I2S_DRIVE_STR,					// I2S2_LRCK		-> I2S2_LRCK
	CFG_FUNC0 | I2S_DRIVE_STR,					// I2S2_BCLK		-> I2S2_BCLK
	CFG_FUNC0 | I2S_DRIVE_STR,					// I2S2_DOUT		-> I2S2_DOUT
	CFG_FUNC0,							// I2S2_DIN		-> I2S2_DIN
	CFG_DISABLED,							// I2S3_MCK		-> I2S3_MCLK

/* Port 18 */
	CFG_FUNC0 | I2S_DRIVE_STR,					// I2S3_LRCK		-> I2S3_LRCK
	CFG_FUNC0 | I2S_DRIVE_STR,					// I2S3_BCLK		-> I2S3_BCLK
	CFG_FUNC0 | I2S_DRIVE_STR,					// I2S3_DOUT		-> I2S3_DOUT
	CFG_FUNC0,							// I2S3_DIN		-> I2S3_DIN
	CFG_DISABLED,							// I2S4_MCK		-> I2S4_MCLK
	CFG_FUNC0 | I2S_DRIVE_STR,					// I2S4_LRCK		-> I2S4_LRCK
	CFG_FUNC0 | I2S_DRIVE_STR,					// I2S4_BCLK		-> I2S4_BCLK
	CFG_FUNC0 | I2S_DRIVE_STR,					// I2S4_DOUT		-> I2S4_DOUT

/* Port 19 */
	CFG_FUNC0,							// I2S4_DIN		-> I2S4_DIN
#if PIN_CFG_DEV
	CFG_FUNC0,							// SPDIF		-> SPDIF
#else
	CFG_DISABLED,						// SPDIF		-> SPDIF
#endif
	CFG_FUNC0 | FMI_LO_SPEED_DRIVE_STR,				// FMI0_CEN3		-> FMI0_CEN3_L
	CFG_FUNC0 | FMI_LO_SPEED_DRIVE_STR,				// FMI0_CEN2		-> FMI0_CEN2_L
	CFG_FUNC0 | FMI_LO_SPEED_DRIVE_STR,				// FMI0_CEN1		-> FMI0_CEN1_L
	CFG_FUNC0 | FMI_LO_SPEED_DRIVE_STR,				// FMI0_CEN0		-> FMI0_CEN0_L
	CFG_FUNC0 | FMI_LO_SPEED_DRIVE_STR,				// FMI0_CLE		-> FMI0_CLE
	CFG_FUNC0 | FMI_LO_SPEED_DRIVE_STR,				// FMI0_ALE		-> FMI0_ALE

/* Port 20 */
	CFG_FUNC0 | FMI_HI_SPEED_DRIVE_STR,				// FMI0_REN		-> FMI0_RE_N
	CFG_FUNC0 | FMI_HI_SPEED_DRIVE_STR,				// FMI0_WEN		-> FMI0_WE_L
#if DISABLE_OPTIONAL_SIGNALS
	CFG_DISABLED | PULL_DOWN | FMI_HI_SPEED_DRIVE_STR,		// FMI0_WENN		-> FMI0_RE_P
#else
	CFG_FUNC0 | FMI_HI_SPEED_DRIVE_STR,				// FMI0_WENN		-> FMI0_RE_P
#endif
	CFG_FUNC0 | BUS_HOLD | FMI_HI_SPEED_DRIVE_STR,			// FMI0_IO7		-> FMI0_IO[7]
	CFG_FUNC0 | BUS_HOLD | FMI_HI_SPEED_DRIVE_STR,			// FMI0_IO6		-> FMI0_IO[6]
	CFG_FUNC0 | BUS_HOLD | FMI_HI_SPEED_DRIVE_STR,			// FMI0_IO5		-> FMI0_IO[5]
	CFG_FUNC0 | BUS_HOLD | FMI_HI_SPEED_DRIVE_STR,			// FMI0_IO4		-> FMI0_IO[4]
	CFG_FUNC0 | BUS_HOLD | FMI_HI_SPEED_DRIVE_STR,			// FMI0_DQS		-> FMI0_DQS_P

/* Port 21 */
#if DISABLE_OPTIONAL_SIGNALS
	CFG_DISABLED | PULL_UP | FMI_HI_SPEED_DRIVE_STR,		// FMI0_DQSN		-> FMI0_DQS_N
#else
	CFG_FUNC0 | BUS_HOLD | FMI_HI_SPEED_DRIVE_STR,			// FMI0_DQSN		-> FMI0_DQS_N
#endif
	CFG_FUNC0 | BUS_HOLD | FMI_HI_SPEED_DRIVE_STR,			// FMI0_IO3		-> FMI0_IO[3]
	CFG_FUNC0 | BUS_HOLD | FMI_HI_SPEED_DRIVE_STR,			// FMI0_IO2		-> FMI0_IO[2]
	CFG_FUNC0 | BUS_HOLD | FMI_HI_SPEED_DRIVE_STR,			// FMI0_IO1		-> FMI0_IO[1]
	CFG_FUNC0 | BUS_HOLD | FMI_HI_SPEED_DRIVE_STR,			// FMI0_IO0		-> FMI0_IO[0]
	CFG_DISABLED | FMI_LO_SPEED_DRIVE_STR,				// FMI0_CEN7		-> 
	CFG_DISABLED | FMI_LO_SPEED_DRIVE_STR,				// FMI0_CEN6		-> 
	CFG_DISABLED | FMI_LO_SPEED_DRIVE_STR,				// FMI0_CEN5		-> 

/* Port 22 */
	CFG_DISABLED | FMI_LO_SPEED_DRIVE_STR,				// FMI0_CEN4		-> 
	CFG_FUNC0 | FMI_LO_SPEED_DRIVE_STR,				// FMI1_CEN3		-> FMI1_CEN3_L
	CFG_FUNC0 | FMI_LO_SPEED_DRIVE_STR,				// FMI1_CEN2		-> FMI1_CEN2_L
	CFG_FUNC0 | FMI_LO_SPEED_DRIVE_STR,				// FMI1_CEN1		-> FMI1_CEN1_L
	CFG_FUNC0 | FMI_LO_SPEED_DRIVE_STR,				// FMI1_CEN0		-> FMI1_CEN0_L
	CFG_FUNC0 | FMI_LO_SPEED_DRIVE_STR,				// FMI1_CLE		-> FMI1_CLE
	CFG_FUNC0 | FMI_LO_SPEED_DRIVE_STR,				// FMI1_ALE		-> FMI1_ALE
	CFG_FUNC0 | FMI_HI_SPEED_DRIVE_STR,				// FMI1_REN		-> FMI1_RE_N

/* Port 23 */
	CFG_FUNC0 | FMI_HI_SPEED_DRIVE_STR,				// FMI1_WEN		-> FMI1_WE_L
#if DISABLE_OPTIONAL_SIGNALS
	CFG_DISABLED | PULL_DOWN | FMI_HI_SPEED_DRIVE_STR,		// FMI1_WENN		-> FMI1_RE_P
#else
	CFG_FUNC0 | FMI_HI_SPEED_DRIVE_STR,				// FMI1_WENN		-> FMI1_RE_P
#endif
	CFG_FUNC0 | BUS_HOLD | FMI_HI_SPEED_DRIVE_STR,			// FMI1_IO7		-> FMI1_IO[7]
	CFG_FUNC0 | BUS_HOLD | FMI_HI_SPEED_DRIVE_STR,			// FMI1_IO6		-> FMI1_IO[6]
	CFG_FUNC0 | BUS_HOLD | FMI_HI_SPEED_DRIVE_STR,			// FMI1_IO5		-> FMI1_IO[5]
	CFG_FUNC0 | BUS_HOLD | FMI_HI_SPEED_DRIVE_STR,			// FMI1_IO4		-> FMI1_IO[4]
	CFG_FUNC0 | BUS_HOLD | FMI_HI_SPEED_DRIVE_STR,			// FMI1_DQS		-> FMI1_DQS_P
#if DISABLE_OPTIONAL_SIGNALS
	CFG_DISABLED | PULL_UP | FMI_HI_SPEED_DRIVE_STR,		// FMI1_DQSN		-> FMI1_DQS_N
#else
	CFG_FUNC0 | BUS_HOLD | FMI_HI_SPEED_DRIVE_STR,			// FMI1_DQSN		-> FMI1_DQS_N
#endif

/* Port 24 */
	CFG_FUNC0 | BUS_HOLD | FMI_HI_SPEED_DRIVE_STR,			// FMI1_IO3		-> FMI1_IO[3]
	CFG_FUNC0 | BUS_HOLD | FMI_HI_SPEED_DRIVE_STR,			// FMI1_IO2		-> FMI1_IO[2]
	CFG_FUNC0 | BUS_HOLD | FMI_HI_SPEED_DRIVE_STR,			// FMI1_IO1		-> FMI1_IO[1]
	CFG_FUNC0 | BUS_HOLD | FMI_HI_SPEED_DRIVE_STR,			// FMI1_IO0		-> FMI1_IO[0]
	CFG_DISABLED | FMI_LO_SPEED_DRIVE_STR,				// FMI1_CEN7		-> 
	CFG_DISABLED | FMI_LO_SPEED_DRIVE_STR,				// FMI1_CEN6		-> 
	CFG_DISABLED | FMI_LO_SPEED_DRIVE_STR,				// FMI1_CEN5		-> 
	CFG_DISABLED | FMI_LO_SPEED_DRIVE_STR,				// FMI1_CEN4		-> 

/* Port 25 */
	CFG_OUT_0 | PULL_DOWN,						// GPIO20		-> WLAN_HSIC3_RESUME
	CFG_IN | PULL_DOWN,						// GPIO21		-> GRAPE_INT_L
	CFG_OUT_0,							// GPIO22		-> LCD_RESET_L
	CFG_IN,								// GPIO23		-> LCD_HIFA_BSYNC
	CFG_IN,								// GPIO24		-> BB_RST_L
	CFG_DISABLED,							// GPIO25		-> BOOT_CONFIG[1]
	CFG_IN | PULL_DOWN,						// GPIO26		-> FORCE_DFU
	CFG_DISABLED | PULL_DOWN,					// GPIO27		-> DFU_STATUS

/* Port 26 */
	CFG_DISABLED,							// GPIO28		-> BOOT_CONFIG[2]
	CFG_DISABLED,							// GPIO29		-> BOOT_CONFIG[3]
	CFG_IN | PULL_UP,						// GPIO30		-> CODEC_INT_L
	CFG_IN | PULL_DOWN,						// GPIO31		-> DEV_HSIC1_RDY
	CFG_DISABLED,							//			-> 
	CFG_DISABLED,							//			-> 
	CFG_DISABLED,							//			-> 
	CFG_DISABLED,							//			-> 

/* Port 27 */
	CFG_DISABLED,							//			-> 
	CFG_DISABLED,							//			-> 
	CFG_DISABLED,							//			-> 
	CFG_DISABLED,							//			-> 
	CFG_DISABLED,							//			-> 
	CFG_DISABLED,							//			-> 
	CFG_DISABLED,							//			-> 
	CFG_DISABLED,							//			-> 

/* Port 28 */
	CFG_IN,								// GPIO32		-> RADIO_ON_L
	CFG_IN | PULL_DOWN,						// GPIO33		-> GYRO_INT1
	CFG_IN | PULL_UP,						// GPIO34		-> COMPASS_BRD_INT
	CFG_OUT_0,							// GPIO35		-> AP_WAKE_MODEM
	CFG_IN | PULL_DOWN,						// GPIO36		-> ACCEL_INT2_L
	CFG_OUT_0,							// GPIO37		-> THS7380_SEL
	CFG_IN | PULL_UP,						// GPIO38		-> ALS_INT_L
	CFG_OUT_0,							// GPIO39		-> GRAPE_RESET_L
};

#undef HI_DRIVE_STR
#undef BB_UART_DRIVE_STR
#undef BT_UART_DRIVE_STR
#undef DOCK_UART_DRIVE_STR
#undef SWI_UART_DRIVE_STR
#undef WIFI_UART_DRIVE_STR
#undef FMI_LO_SPEED_DRIVE_STR
#undef FMI_HI_SPEED_DRIVE_STR
#undef GRAPE_IO_DRIVE_STR
#undef GRAPE_CLK_DRIVE_STR
#undef DWI_DRIVE_STR
#undef I2C_DRIVE_STR
#undef I2S_DRIVE_STR
#undef CAM_I2C_DRIVE_STR
#undef CAM_STROBE_DRIVE_STR
#undef CAM_CLK_DRIVE_STR
#undef DISABLE_OPTIONAL_SIGNALS
