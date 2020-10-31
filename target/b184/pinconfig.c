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
#include <drivers/apple/gpio.h>
#include <platform/soc/hwregbase.h>
#include <stdint.h>

/* THIS STRUCT IS AUTOMATICALLY GENERATED BY tools/csvtopinconfig.py. DO NOT EDIT!
   I/O Spreadsheet version: version 1.16
   I/O Spreadsheet tracker: 12739020&15521285
*/

static const uint32_t gpio_default_cfg[GPIO_GROUP_COUNT * GPIOPADPINS] = {

/* Port 0 */
	CFG_FUNC0,				//   0 : TST_CLKOUT		-> SOC_TST_CLKOUT
	CFG_FUNC0 | FAST_SLEW,			//   1 : WDOG			-> SOC_WDOG
	CFG_IN,					//   2 : GPIO[0]		-> PMU_BUTTON1
	CFG_IN,					//   3 : GPIO[1]		-> PMU_BUTTON2
	CFG_IN,					//   4 : GPIO[2]		-> PMU_BUTTON3
	CFG_IN,					//   5 : GPIO[3]		-> PMU_BUTTON4
	CFG_IN,					//   6 : GPIO[4]		-> ACCEL_HOST_WAKE1
	CFG_IN,					//   7 : GPIO[5]		-> PS_ON_SOC

/* Port 1 */
	CFG_IN,					//   8 : GPIO[6]		-> AUD_ARRAY_HOST_WAKE
	CFG_IN,					//   9 : GPIO[7]		-> BT2_BTWAKE
	CFG_IN,					//  10 : GPIO[8]		-> ENET_PHY_HOST_WAKE
	CFG_IN,					//  11 : GPIO[9]		-> MCU_HOST_WAKE
	CFG_OUT_0 | PULL_DOWN | SLOW_SLEW,	//  12 : GPIO[10]		-> SOC_TO_ENET_PME_MODE_SEL
	CFG_IN,					//  13 : GPIO[11]		-> SOC_TO_MCU_RESET_L
	CFG_IN,					//  14 : GPIO[12]		-> SOC_TO_MCU_BOOTLOAD
	CFG_IN | PULL_UP,			//  15 : GPIO[13]		-> PMU_IRQ_L

/* Port 2 */
	CFG_IN,					//  16 : GPIO[14]		-> SOC_WLAN_ASTP_SEL
	CFG_IN,					//  17 : GPIO[15]		-> ENET_HSIC_DEVICE_READY
	CFG_IN | PULL_DOWN,			//  18 : GPIO[16]		-> BOARDID_3
	CFG_IN,					//  19 : GPIO[17]		-> TRI_INT
	CFG_IN | PULL_DOWN,			//  20 : GPIO[18]		-> BOOTCONFIG_0
	CFG_OUT_0 | SLOW_SLEW,			//  21 : GPIO[19]		-> PMU_KEEPACT
	CFG_DISABLED,				//  22 : GPIO[20]		-> NC_SPDIF_LOCK_L
	CFG_DISABLED,				//  23 : GPIO[21]		-> NC_SPDIF_INT_L

/* Port 3 */
	CFG_DISABLED,
	CFG_DISABLED,
	CFG_DISABLED,
	CFG_DISABLED,
	CFG_DISABLED,
	CFG_DISABLED,
	CFG_DISABLED,
	CFG_DISABLED,

/* Port 4 */
	CFG_FUNC0 | SLOW_SLEW,			//  32 : UART1_TXD		-> SOC_UART1_TO_MCU_TXD
	CFG_FUNC0,				//  33 : UART1_RXD		-> MCU_TO_SOC_UART1_RXD
	CFG_OUT_0,				//  34 : UART1_RTSN		-> MCU_SWCLK
	CFG_IN | SLOW_SLEW,			//  35 : UART1_CTSN		-> MCU_SWDIO
	CFG_FUNC0 | SLOW_SLEW,			//  36 : UART2_TXD		-> SOC_UART2_TO_BT_TXD
	CFG_FUNC0,				//  37 : UART2_RXD		-> BT_TO_SOC_UART2_RXD
	CFG_FUNC0,				//  38 : UART2_RTSN		-> SOC_UART2_TO_BT_RTSB
	CFG_FUNC0,				//  39 : UART2_CTSN		-> BT_TO_SOC_UART2_CTSB

/* Port 5 */
	CFG_FUNC0 | SLOW_SLEW,			//  40 : UART3_TXD		-> WLAN1_TO_SOC_UART3_RXD
	CFG_FUNC0,				//  41 : UART3_RXD		-> SOC_UART3_TO_WLAN1_TXD
	CFG_DISABLED,				//  42 : UART3_RTSN		-> NC_UART3_RTSN
	CFG_DISABLED,				//  43 : UART3_CTSN		-> NC_UART3_CTSN
	CFG_DISABLED,				//  44 : UART5_RTXD		-> UART5_RTXD
	CFG_DISABLED,
	CFG_DISABLED,
	CFG_DISABLED,

/* Port 6 */
	CFG_DISABLED,
	CFG_DISABLED,
	CFG_DISABLED,
	CFG_DISABLED,
	CFG_DISABLED,
	CFG_DISABLED,
	CFG_DISABLED,
	CFG_DISABLED,

/* Port 7 */
	CFG_DISABLED,
	CFG_DISABLED,
	CFG_DISABLED,
	CFG_DISABLED,
	CFG_DISABLED,
	CFG_DISABLED,
	CFG_DISABLED,
	CFG_DISABLED,

/* Port 8 */
	CFG_FUNC0 | SLOW_SLEW,			//  64 : UART4_TXD		-> SOC_UART4_TO_WLAN2_TXD
	CFG_FUNC0,				//  65 : UART4_RXD		-> WLAN2_TO_SOC_UART4_RXD
	CFG_DISABLED,				//  66 : UART4_RTSN		-> NC_UART4_RTSN
	CFG_DISABLED,				//  67 : UART4_CTSN		-> NC_UART4_CTSN
	CFG_DISABLED,				//  68 : SPI1_SCLK		-> NC_SPI1_SCLK
	CFG_DISABLED,				//  69 : SPI1_MOSI		-> NC_SPI1_MOSI
	CFG_DISABLED,				//  70 : SPI1_MISO		-> NC_SPI1_MISO
	CFG_DISABLED,				//  71 : SPI1_SSIN		-> NC_SPI1_SSIN

/* Port 9 */
	CFG_IN | PULL_DOWN,			//  72 : SPI0_SCLK		-> BOARDID_0
	CFG_IN | PULL_DOWN,			//  73 : SPI0_MOSI		-> BOARDID_1
	CFG_IN | PULL_DOWN,			//  74 : SPI0_MISO		-> BOARDID_2
	CFG_DISABLED,				//  75 : SPI0_SSIN		-> NC_SPI0_SSIN
	CFG_DISABLED,				//  76 : SPI2_SCLK		-> NC_SPI2_SCLK
	CFG_DISABLED,				//  77 : SPI2_MOSI		-> NC_SPI2_MOSI
	CFG_DISABLED,				//  78 : SPI2_MISO		-> NC_SPI2_MISO
	CFG_DISABLED,				//  79 : SPI2_SSIN		-> NC_SPI2_SSIN

/* Port 10 */
	CFG_FUNC0 | SLOW_SLEW,			//  80 : I2C0_SDA		-> I2C0_SDA_1V8
	CFG_FUNC0 | SLOW_SLEW,			//  81 : I2C0_SCL		-> I2C0_SCL_1V8
	CFG_FUNC0 | SLOW_SLEW,			//  82 : I2C1_SDA		-> I2C1_SDA_1V8
	CFG_FUNC0 | SLOW_SLEW,			//  83 : I2C1_SCL		-> I2C1_SCL_1V8
	CFG_DISABLED,				//  84 : ISP0_SDA		-> (NC)
	CFG_DISABLED,				//  85 : ISP0_SCL		-> (NC)
	CFG_DISABLED,				//  86 : ISP1_SDA		-> (NC)
	CFG_DISABLED,				//  87 : ISP1_SCL		-> (NC)

/* Port 11 */
	CFG_DISABLED,				//  88 : SENSOR0_RST		->
	CFG_DISABLED,				//  89 : SENSOR0_CLK		->
	CFG_DISABLED,				//  90 : SENSOR0_XSHUTDOWN	->
	CFG_DISABLED,				//  91 : SENSOR0_ISTRB		->
	CFG_DISABLED,				//  92 : SENSOR1_RST		->
	CFG_DISABLED,				//  93 : SENSOR1_CLK		->
	CFG_DISABLED,				//  94 : SENSOR1_XSHUTDOWN	->
	CFG_DISABLED,				//  95 : SENSOR1_ISTRB		->

/* Port 12 */
	CFG_DISABLED,				//  96 : SPI3_MOSI		-> NC_SPI3_MOSI
	CFG_DISABLED,				//  97 : SPI3_MISO		-> NC_SPI3_MISO
	CFG_DISABLED,				//  98 : SPI3_SCLK		-> NC_SPI3_SCLK
	CFG_DISABLED,				//  99 : SPI3_SSIN		-> NC_SPI3_SSIN
	CFG_DISABLED,				// 100 : I2C2_SDA		-> I2C2_SPDIF_SDA_1V8
	CFG_DISABLED,				// 101 : I2C2_SCL		-> I2C2_SPDIF_SCL_1V8
	CFG_DISABLED,				// 102 : GPIO[22]		-> NC_SPDIF_RESET_L
	CFG_OUT_0 | PULL_UP | SLOW_SLEW,	// 103 : GPIO[23]		-> ENET_RESET_L

/* Port 13 */
	CFG_IN,					// 104 : GPIO[24]		-> ENET_PME_1V8
	CFG_IN | PULL_DOWN,			// 105 : GPIO[25]		-> BOOTCONFIG_1
	CFG_IN,					// 106 : GPIO[26]		-> FORCE_DFU
	CFG_OUT_0 | SLOW_SLEW,			// 107 : GPIO[27]		-> DFU_STATUS
	CFG_IN | PULL_DOWN,			// 108 : GPIO[28]		-> BOOTCONFIG_2
	CFG_IN | PULL_DOWN,			// 109 : GPIO[29]		-> BOOTCONFIG_3
	CFG_IN,					// 110 : GPIO[30]		-> BT1_BTWAKE
	CFG_DISABLED,				// 111 : GPIO[31]		-> SOC_ANALOG_SEL

/* Port 14 */
	CFG_DISABLED,				// 112 : GPIO[32]		-> SOC_SPDIF_SEL
	CFG_DISABLED,				// 113 : GPIO[33]		-> SOC_ANALOG_PWR_EN
	CFG_IN | PULL_DOWN,			// 114 : GPIO[34]		-> BOARDREV_3
	CFG_IN | PULL_DOWN,			// 115 : GPIO[35]		-> BOARDREV_2
	CFG_IN | PULL_DOWN,			// 116 : GPIO[36]		-> BOARDREV_1
	CFG_IN | PULL_DOWN,			// 117 : GPIO[37]		-> BOARDREV_0
	CFG_OUT_0 | SLOW_SLEW,			// 118 : GPIO[38]		-> SOC_BT_SEL
	CFG_DISABLED,				// 119 : DISP_VSYNC		-> SOC_DISP_VSYNC

/* Port 15 */
	CFG_FUNC0,				// 120 : SOCHOT0		-> SOCHOT0_L
	CFG_FUNC0,				// 121 : SOCHOT1		-> SOCHOT1_L
	CFG_FUNC0 | FAST_SLEW,			// 122 : UART0_TXD		-> UART0_TXD
	CFG_FUNC0,				// 123 : UART0_RXD		-> UART0_RXD
	CFG_DISABLED,				// 124 : UNSPECIFIED		-> UNSPECIFIED
	CFG_FUNC0 | DRIVE_X2 | FAST_SLEW,	// 125 : DWI_DO			-> DWI_AP_DO
	CFG_FUNC0 | DRIVE_X2 | FAST_SLEW,	// 126 : DWI_CLK		-> DWI_AP_CLK
	CFG_DISABLED,				// 127 : UNSPECIFIED		-> UNSPECIFIED

/* Port 16 */
	CFG_FUNC0,				// 128 : I2S0_LRCK		-> MCA_FSYNC_1V8
	CFG_FUNC0,				// 129 : I2S0_BCLK		-> MCA_BCLK_1V8
	CFG_FUNC0 | DRIVE_X2 | FAST_SLEW,	// 130 : I2S0_DOUT		-> MCA0_DOUT
	CFG_FUNC0,				// 131 : I2S0_DIN		-> MIC_DATA_1V8
	CFG_DISABLED,				// 132 : I2S1_MCK		-> I2S0_MCK
	CFG_FUNC0,				// 133 : I2S1_LRCK		-> MCA_FSYNC_1V8
	CFG_FUNC0,				// 134 : I2S1_BCLK		-> MCA_BCLK_1V8
	CFG_FUNC0 | DRIVE_X2 | FAST_SLEW,	// 135 : I2S1_DOUT		-> MCA1_DOUT

/* Port 17 */
	CFG_DISABLED,				// 136 : I2S1_DIN		-> MCA1_DIN
	CFG_FUNC0,				// 137 : I2S2_LRCK		-> MCA_FSYNC_1V8
	CFG_FUNC0,				// 138 : I2S2_BCLK		-> MCA_BCLK_1V8
	CFG_FUNC0 | DRIVE_X2 | FAST_SLEW,	// 139 : I2S2_DOUT		-> MCA2_DOUT
	CFG_FUNC0,				// 140 : I2S2_DIN		-> MCU_MISO_1V8
	CFG_DISABLED,				// 141 : I2S3_MCK		-> NC_I2S2_MCK
	CFG_FUNC0,				// 142 : I2S3_LRCK		-> MCA_FSYNC_1V8
	CFG_FUNC0,				// 143 : I2S3_BCLK		-> MCA_BCLK_1V8

/* Port 18 */
	CFG_FUNC0 | DRIVE_X2 | FAST_SLEW,	// 144 : I2S3_DOUT		-> MCA3_DOUT
	CFG_FUNC0,				// 145 : I2S3_DIN		-> MCA3_DIN
	CFG_DISABLED,				// 146 : I2S4_MCK		-> NC_I2S4_MCK
	CFG_DISABLED,				// 147 : I2S4_LRCK		-> NC_I2S4_LRCLK
	CFG_DISABLED,				// 148 : I2S4_BCLK		-> NC_I2S4_BCLK
	CFG_DISABLED,				// 149 : I2S4_DOUT		-> NC_I2S4_DOUT
	CFG_DISABLED,				// 150 : I2S4_DIN		-> NC_I2S4_DIN
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
	CFG_DISABLED,				// 160 : TMR32_PWM0		-> NC_TMR32_PWM0
	CFG_DISABLED,				// 161 : TMR32_PWM1		-> NC_TMR32_PWM1
	CFG_DISABLED,				// 162 : TMR32_PWM2		-> NC_TMR32_PWM2
	CFG_OUT_0 | FAST_SLEW,			// 163 : SIO_7816UART0_SDA	-> WLAN2_HOST_READY
	CFG_IN,					// 164 : SIO_7816UART0_SCL	-> WLAN2_DEVICE_READY
	CFG_IN,					// 165 : SIO_7816UART0_RST	-> WLAN2_HSIC_RESUME
	CFG_OUT_0,				// 166 : SIO_7816UART1_SDA	-> WLAN1_HOST_READY
	CFG_IN,					// 167 : SIO_7816UART1_SCL	-> WLAN1_DEVICE_READY

/* Port 21 */
	CFG_IN,					// 168 : SIO_7816UART1_RST	-> WLAN1_HSIC_RESUME
	CFG_FUNC0 | FAST_SLEW,			// 169 : UART6_TXD		-> UART6_TXD
	CFG_FUNC0,				// 170 : UART6_RXD		-> UART6_RXD
	CFG_DISABLED,				// 171 : I2C3_SDA		-> I2C_ARRAY_SCL_1V8
	CFG_DISABLED,				// 172 : I2C3_SCL		-> I2C_ARRAY_SDA_1V8
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
	CFG_DISABLED,				// 192 : EDP_HPD		-> (NC)
	CFG_DISABLED,				// 193 : I2S0_MCK		-> I2S0_MCK
	CFG_DISABLED,				// 194 : I2S2_MCK		-> NC_I2S2_MCK
};

const uint32_t *target_get_default_gpio_cfg(uint32_t gpioc)
{
	return gpio_default_cfg;
}
