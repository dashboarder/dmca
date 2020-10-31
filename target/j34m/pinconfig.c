/*
 * Copyright (C) 2014 Apple Inc. All rights reserved.
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

/* THIS STRUCT IS AUTOMATICALLY GENERATED BY tools/csvtopinconfig.py. DO NOT EDIT!
   I/O Spreadsheet version: rev 0.40
   I/O Spreadsheet tracker: 16419127
*/

static const uint32_t gpio_default_cfg[GPIO_GROUP_COUNT * GPIOPADPINS] = {

/* Port  0 */
	CFG_DISABLED,							//   0 : TST_CLKOUT		->
	CFG_FUNC0 | FAST_SLEW,						//   1 : WDOG			-> WDOG_SOC
	CFG_IN,								//   2 : GPIO0			-> MCU_AP_RECOVERY_MODE_L
	CFG_IN,								//   3 : GPIO1			-> MCU_PMU_WAKE_L
	CFG_DISABLED,							//   4 : GPIO2			-> GPIO_BTN_VOL_DOWN_L
	CFG_DISABLED,							//   5 : GPIO3			-> GPIO_BTN_VOL_UP_L
	CFG_DISABLED,							//   6 : GPIO4			-> NC_GPIO_MOCA_PLUG_DETECT
	CFG_OUT_0 | FAST_SLEW,						//   7 : GPIO5			-> GPIO_SOC2BT_WAKE

/* Port  1 */
	CFG_OUT_0 | FAST_SLEW,						//   8 : GPIO6			-> AP_MCU_INT_1V8
	CFG_OUT_0 | PULL_UP,						//   9 : GPIO7			-> AP_TO_MCU_TCK_1V8
	CFG_IN,								//  10 : GPIO8			-> LAN_PHY_INT_1V8
	CFG_DISABLED,							//  11 : GPIO9			->
	CFG_OUT_0 | FAST_SLEW,						//  12 : GPIO10			-> LAN_PME_MODE_SEL_AP_1V8
	CFG_DISABLED,							//  13 : GPIO11			->
	CFG_IN,								//  14 : GPIO12			-> HVR_IRQ
	CFG_IN | PULL_DOWN,						//  15 : GPIO13			-> LAN_HSIC_DEVICE_READY_1V8

/* Port  2 */
	CFG_DISABLED,							//  16 : GPIO14			->
	CFG_IN,								//  17 : GPIO15			-> MOCA_IRQ_1.8V
	CFG_IN | PULL_DOWN,						//  18 : GPIO16			-> GPIO_BOARD_ID_3
	CFG_IN,								//  19 : GPIO17			-> GPIO_TS2SOC2PMU_INT
	CFG_IN | PULL_DOWN,						//  20 : GPIO18			-> GPIO_BOOT_CONFIG_0
	CFG_OUT_0 | FAST_SLEW,						//  21 : GPIO19			-> PMON_SWCLK
	CFG_IN,								//  22 : GPIO20			-> PMON_MCU_TO_SOC_2V05_1
	CFG_IN | FAST_SLEW,						//  23 : GPIO21			-> PMON_SWDIO

/* Port  3 */
	CFG_DISABLED,
	CFG_DISABLED,
	CFG_DISABLED,
	CFG_DISABLED,
	CFG_DISABLED,
	CFG_DISABLED,
	CFG_DISABLED,
	CFG_DISABLED,

/* Port  4 */
	CFG_FUNC0 | FAST_SLEW,						//  32 : UART1_TXD		-> AP_UART1_TXD_1V8
	CFG_FUNC0,							//  33 : UART1_RXD		-> AP_UART1_RXD_1V8
	CFG_DISABLED,							//  34 : UART1_RTSN		-> NC_UART1_RTS_L
	CFG_DISABLED,							//  35 : UART1_CTSN		-> NC_UART1_RTS_L
	CFG_FUNC0 | FAST_SLEW,						//  36 : UART2_TXD		-> UART2_SOC_TO_PMON_R
	CFG_FUNC0,							//  37 : UART2_RXD		-> UART2_PMON_TO_SOC
	CFG_DISABLED,							//  38 : UART2_RTSN		-> NC_GPIO_UART2_RTSN
	CFG_DISABLED,							//  39 : UART2_CTSN		-> NC_GPIO_UART2_CTSN

/* Port  5 */
	CFG_FUNC0 | DRIVE_X2 | FAST_SLEW,				//  40 : UART3_TXD		-> AP_UART3_TXD_1V8
	CFG_FUNC0,							//  41 : UART3_RXD		-> AP_UART3_RXD_1V8
	CFG_DISABLED,							//  42 : UART3_RTSN		-> NC_UART3_BB_RTS_L
	CFG_DISABLED,							//  43 : UART3_CTSN		-> NC_UART3_BB_CTS_:
	CFG_FUNC0,							//  44 : UART5_RTXD		-> UART5_WLAN_RXD
	CFG_DISABLED,
	CFG_DISABLED,
	CFG_DISABLED,

/* Port  6 */
	CFG_DISABLED,
	CFG_DISABLED,
	CFG_DISABLED,
	CFG_DISABLED,
	CFG_DISABLED,
	CFG_DISABLED,
	CFG_DISABLED,
	CFG_DISABLED,

/* Port  7 */
	CFG_DISABLED,
	CFG_DISABLED,
	CFG_DISABLED,
	CFG_DISABLED,
	CFG_DISABLED,
	CFG_DISABLED,
	CFG_DISABLED,
	CFG_DISABLED,

/* Port  8 */
	CFG_FUNC0 | FAST_SLEW,						//  64 : UART4_TXD		-> UART4_BT_TXD
	CFG_FUNC0,							//  65 : UART4_RXD		-> UART4_BT_RXD
	CFG_FUNC0 | FAST_SLEW,						//  66 : UART4_RTSN		-> UART4_BT_RTS_L
	CFG_FUNC0,							//  67 : UART4_CTSN		-> UART4_BT_CTS_L
	CFG_FUNC0 | FAST_SLEW,						//  68 : SPI1_SCLK		-> SPI1_SOC_SCLK_1V8
	CFG_FUNC0 | FAST_SLEW,						//  69 : SPI1_MOSI		-> SPI1_SOC_MOSI_1V8
	CFG_FUNC0,							//  70 : SPI1_MISO		-> SPI1_SOC_MISO_1V8
	CFG_FUNC0 | FAST_SLEW,						//  71 : SPI1_SSIN		-> SPI1_SOC_CS_L_1V8

/* Port  9 */
	CFG_IN | PULL_DOWN,						//  72 : SPI0_SCLK		-> GPIO_BOARD_ID_0
	CFG_IN | PULL_DOWN,						//  73 : SPI0_MOSI		-> GPIO_BOARD_ID_1
	CFG_IN | PULL_DOWN,						//  74 : SPI0_MISO		-> GPIO_BOARD_ID_2
	CFG_DISABLED,							//  75 : SPI0_SSIN		-> NC_SPI0_SSIN
	CFG_DISABLED,							//  76 : SPI2_SCLK		-> NC_SPI2_SCLK
	CFG_DISABLED,							//  77 : SPI2_MOSI		-> NC_SPI2_MOSI
	CFG_DISABLED,							//  78 : SPI2_MISO		-> NC_SPI2_MISO
	CFG_DISABLED,							//  79 : SPI2_SSIN		-> NC_SPI2_CS_L

/* Port 10 */
	CFG_FUNC0 | DRIVE_X2 | FAST_SLEW,				//  80 : I2C0_SDA		-> I2C0_SDA_1V8
	CFG_FUNC0 | DRIVE_X2 | FAST_SLEW,				//  81 : I2C0_SCL		-> I2C0_SCL_1V8
	CFG_FUNC0 | DRIVE_X2 | FAST_SLEW,				//  82 : I2C1_SDA		-> I2C1_HOOVR_SDA
	CFG_FUNC0 | DRIVE_X2 | FAST_SLEW,				//  83 : I2C1_SCL		-> I2C1_HOOVR_SCL
	CFG_DISABLED,							//  84 : ISP0_SDA		-> TP_ISP0_CAM_REAR_SDA
	CFG_DISABLED,							//  85 : ISP0_SCL		-> TP_ISP0_CAM_REAR_SCL
	CFG_DISABLED,							//  86 : ISP1_SDA		-> TP_ISP1_CAM_FRONT_SDA
	CFG_DISABLED,							//  87 : ISP1_SCL		-> TP_ISP1_CAM_FRONT_SCL

/* Port 11 */
	CFG_DISABLED,							//  88 : SENSOR0_RST		-> NC_ISP0_CAM_REAR_CLK
	CFG_DISABLED,							//  89 : SENSOR0_CLK		-> NC_ISP0_CAM_REAR_SHUTDOWN
	CFG_DISABLED,							//  90 : SENSOR0_XSHUTDOWN	-> NC_SENSOR0_XSHUTDOWN
	CFG_DISABLED,							//  91 : SENSOR0_ISTRB		-> NC_SENSOR0_ISTRB
	CFG_DISABLED,							//  92 : SENSOR1_RST		-> NC_ISP1_CAM_FRONT_SHUTDOWN_L
	CFG_DISABLED,							//  93 : SENSOR1_CLK		-> NC_ISP1_CAM_FRONT_CLK
	CFG_DISABLED,							//  94 : SENSOR1_XSHUTDOWN	-> NC_SENSOR1_XSHUTDOWN
	CFG_DISABLED,							//  95 : SENSOR1_ISTRB		-> NC_SENSOR1_ISTRB

/* Port 12 */
	CFG_DISABLED,							//  96 : SPI3_MOSI		-> NC_SPI3_MOSI
	CFG_DISABLED,							//  97 : SPI3_MISO		-> NC_SPI3_MISO
	CFG_DISABLED,							//  98 : SPI3_SCLK		-> NC_SPI3_SCLK
	CFG_DISABLED,							//  99 : SPI3_SSIN		-> NC_SPI3_CS_L
	CFG_FUNC0 | DRIVE_X2 | FAST_SLEW,				// 100 : I2C2_SDA		-> I2C2_SDA_1V8
	CFG_FUNC0 | DRIVE_X2 | FAST_SLEW,				// 101 : I2C2_SCL		-> I2C2_SCL_1V8
	CFG_OUT_0 | FAST_SLEW,						// 102 : GPIO22			-> GPIO_SOC2PMU_KEEPACT
	CFG_IN | PULL_UP,						// 103 : GPIO23			-> GPIO_PMU2SOC_IRQ_L

/* Port 13 */
	CFG_DISABLED,							// 104 : GPIO24			-> NC_GPIO24
	CFG_IN | PULL_DOWN,						// 105 : GPIO25			-> GPIO_BOOT_CONFIG_1
	CFG_IN,								// 106 : GPIO26			-> GPIO_FORCE_DFU
	CFG_OUT_0 | FAST_SLEW,						// 107 : GPIO27			-> MCU_DFU_STATUS_1V8
	CFG_IN | PULL_DOWN,						// 108 : GPIO28			-> GPIO_BOOT_CONFIG_2
	CFG_IN | PULL_DOWN,						// 109 : GPIO29			-> GPIO_BOOT_CONFIG_3
	CFG_DISABLED,							// 110 : GPIO30			-> GPIO_BTN_SRL_L
	CFG_OUT_0,							// 111 : GPIO31			-> SOC_TO_PMON_RESET_1V8_L

/* Port 14 */
	CFG_DISABLED,							// 112 : GPIO32			-> NC
	CFG_OUT_0,							// 113 : GPIO33			-> PMON_ISP_EN_L
	CFG_IN,								// 114 : GPIO34			-> SOC_TO_MCU_RESET_1V8_L
	CFG_IN | PULL_DOWN,						// 115 : GPIO35			-> GPIO_BRD_REV3
	CFG_IN | PULL_DOWN,						// 116 : GPIO36			-> GPIO_BRD_REV2
	CFG_IN | PULL_DOWN,						// 117 : GPIO37			-> GPIO_BRD_REV1
	CFG_IN | PULL_DOWN,						// 118 : GPIO38			-> GPIO_BRD_REV0
	CFG_DISABLED,							// 119 : DISP_VSYNC		-> NC_DISPLAY_SYNC

/* Port 15 */
	CFG_FUNC0,							// 120 : SOCHOT0		-> SOCHOT0_L
	CFG_FUNC0 | FAST_SLEW,						// 121 : SOCHOT1		-> SOCHOT1_L
	CFG_FUNC0,							// 122 : UART0_TXD		-> AP_UART0_TXD_1V8
	CFG_FUNC0,							// 123 : UART0_RXD		-> AP_UART0_RXD_1V8
	CFG_OUT_0,							// 124 : DWI_DI			-> SOC_TST_CPUSWITCH_OUT
	CFG_FUNC0 | DRIVE_X2 | FAST_SLEW,				// 125 : DWI_DO			-> DWI_AP_DO
	CFG_FUNC0 | DRIVE_X2 | FAST_SLEW,				// 126 : DWI_CLK		-> DWI_AP_CLK
	CFG_DISABLED,							// 127 : UNSPECIFIED		-> UNSPECIFIED

/* Port 16 */
	CFG_FUNC0 | DRIVE_X2 | FAST_SLEW,				// 128 : I2S0_LRCK		-> I2S0_HOOVR_LRCK
	CFG_FUNC0 | DRIVE_X2 | FAST_SLEW,				// 129 : I2S0_BCLK		-> I2S0_HOOVR_BCLK
	CFG_FUNC0 | DRIVE_X2 | FAST_SLEW,				// 130 : I2S0_DOUT		-> I2S0_HOOVR_DOUT
	CFG_DISABLED,							// 131 : I2S0_DIN		-> TP_I2S0_DIN
	CFG_DISABLED,							// 132 : I2S1_MCK		-> NC_I2S1_MCK
	CFG_DISABLED,							// 133 : I2S1_LRCK		-> TP_I2S1_LRCK
	CFG_DISABLED,							// 134 : I2S1_BCLK		-> TP_I2S1_BCLK
	CFG_DISABLED,							// 135 : I2S1_DOUT		-> TP_I2S1_DOUT

/* Port 17 */
	CFG_DISABLED,							// 136 : I2S1_DIN		-> TP_I2S1_DIN
	CFG_FUNC0 | DRIVE_X2 | FAST_SLEW,				// 137 : I2S2_LRCK		-> TP_I2S2_LRCK
	CFG_FUNC0 | DRIVE_X2 | FAST_SLEW,				// 138 : I2S2_BCLK		-> TP_I2S2_BCLK
	CFG_FUNC0 | DRIVE_X2 | FAST_SLEW,				// 139 : I2S2_DOUT		-> TP_I2S2_DOUT
	CFG_DISABLED,							// 140 : I2S2_DIN		-> TP_I2S2_DIN
	CFG_DISABLED,							// 141 : I2S3_MCK		-> TP_I2S3_MCK
	CFG_DISABLED,							// 142 : I2S3_LRCK		-> TP_I2S3_LRCK
	CFG_DISABLED,							// 143 : I2S3_BCLK		-> TP_I2S3_BCLK

/* Port 18 */
	CFG_FUNC0 | FAST_SLEW,						// 144 : I2S3_DOUT		-> I2S3_WLAN_TXD
	CFG_DISABLED,							// 145 : I2S3_DIN		-> TP_I2S3_DIN
	CFG_DISABLED,							// 146 : I2S4_MCK		-> NC_I2S4_MCK
	CFG_FUNC0 | DRIVE_X2 | FAST_SLEW,				// 147 : I2S4_LRCK		-> I2S4_BT_LRCK
	CFG_FUNC0 | DRIVE_X2 | FAST_SLEW,				// 148 : I2S4_BCLK		-> I2S4_BT_BCLK
	CFG_FUNC0 | DRIVE_X2 | FAST_SLEW,				// 149 : I2S4_DOUT		-> I2S4_BT_DOUT
	CFG_FUNC0 | DRIVE_X2 | FAST_SLEW,				// 150 : I2S4_DIN		-> I2S4_BT_DIN
	CFG_DISABLED,

/* Port 19 */
	CFG_DISABLED,
	CFG_DISABLED,
	CFG_DISABLED,
	CFG_DISABLED,
	CFG_DISABLED,
	CFG_DISABLED,
	CFG_DISABLED,
	CFG_DISABLED,

/* Port 20 */
	CFG_DISABLED,							// 160 : TMR32_PWM0		-> NC_TMR32_PWM0
	CFG_DISABLED,							// 161 : TMR32_PWM1		-> NC_TMR32_PWM1
	CFG_DISABLED,							// 162 : TMR32_PWM2		-> NC_TMR32_PWM2
	CFG_OUT_0 | FAST_SLEW,						// 163 : SIO_7816UART0_SDA	-> HSIC1_SOC2WLAN_HOST_RDY
	CFG_IN,								// 164 : SIO_7816UART0_SCL	-> HSIC1_WLAN2SOC_DEVICE_RDY
	CFG_IN,								// 165 : SIO_7816UART0_RST	-> HSIC1_WLAN2SOC_REMOTE_WAKE
	CFG_DISABLED,							// 166 : SIO_7816UART1_SDA	-> NC_SIO_SDA
	CFG_DISABLED,							// 167 : SIO_7816UART1_SCL	-> NC_SIO_SCL

/* Port 21 */
	CFG_DISABLED,							// 168 : SIO_7816UART1_RST	-> NC_SIO_RST
	CFG_FUNC0 | FAST_SLEW,						// 169 : UART6_TXD		-> UART6_TS_ACC_TXD
	CFG_FUNC0,							// 170 : UART6_RXD		-> UART6_TS_ACC_RXD
	CFG_DISABLED,							// 171 : I2C3_SDA		-> NC_I2C3_SDA_1V8
	CFG_DISABLED,							// 172 : I2C3_SCL		-> NC_I2C3_SCL_1V8
	CFG_DISABLED,
	CFG_DISABLED,
	CFG_DISABLED,

/* Port 22 */
	CFG_DISABLED,
	CFG_DISABLED,
	CFG_DISABLED,
	CFG_DISABLED,
	CFG_DISABLED,
	CFG_DISABLED,
	CFG_DISABLED,
	CFG_DISABLED,

/* Port 23 */
	CFG_DISABLED,
	CFG_DISABLED,
	CFG_DISABLED,
	CFG_DISABLED,
	CFG_DISABLED,
	CFG_DISABLED,
	CFG_DISABLED,
	CFG_DISABLED,

/* Port 24 */
	CFG_FUNC0 | FAST_SLEW,						// 192 : EDP_HPD		-> EDP_HPD
	CFG_FUNC0 | FAST_SLEW,						// 193 : I2S0_MCK		-> I2S0_HOOVR_MCK
	CFG_DISABLED,							// 194 : I2S2_MCK		-> TP_I2S2_MCK
	CFG_DISABLED,
	CFG_DISABLED,
	CFG_DISABLED,
	CFG_DISABLED,
	CFG_DISABLED,
};

const uint32_t *target_get_default_gpio_cfg(uint32_t gpioc)
{
	return gpio_default_cfg;
}
