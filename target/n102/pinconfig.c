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

/* THIS FILE IS AUTOMATICALLY GENERATED BY tools/csvtopinconfig.py. DO NOT EDIT!
   I/O Spreadsheet version: rev 0v8
   I/O Spreadsheet tracker: 18673410
   Conversion command: csvtopinconfig.py --soc fiji --radar 18673410 --copyright 2014 <filename>
*/

#include <debug.h>
#include <drivers/apple/gpio.h>
#include <platform.h>
#include <platform/soc/hwregbase.h>
#include <stdint.h>

static const uint32_t pinconfig_ap_0[GPIO_GROUP_COUNT * GPIOPADPINS] = {

/* Port  0 */
	CFG_DISABLED | PULL_DOWN | SLOW_SLEW,				//   0 : ULPI_DIR		-> LCM_TO_AP_HIFA_BSYNC
	CFG_OUT_0 | PULL_DOWN | SLOW_SLEW,				//   1 : ULPI_STP		-> AP_TO_LCM_RESET_L
	CFG_OUT_0 | PULL_DOWN | SLOW_SLEW,				//   2 : ULPI_NXT		-> AP_TO_TOUCH_RESET_L
	CFG_IN | PULL_UP | SLOW_SLEW,					//   3 : ULPI_DATA[7]		-> TOUCH_TO_AP_INT_L
	CFG_DISABLED | PULL_DOWN | DRIVE_X4 | SLOW_SLEW,		//   4 : ULPI_DATA[6]		-> NC
	CFG_DISABLED | PULL_DOWN | DRIVE_X4 | SLOW_SLEW,		//   5 : ULPI_DATA[5]		-> NC
	CFG_DISABLED | PULL_DOWN | DRIVE_X4 | SLOW_SLEW,		//   6 : ULPI_DATA[4]		-> NC
	CFG_IN | PULL_DOWN | SLOW_SLEW,					//   7 : ULPI_CLK		-> OSCAR_TO_PMU_HOST_WAKE

/* Port  1 */
	CFG_OUT_0 | PULL_DOWN | SLOW_SLEW,				//   8 : ULPI_DATA[3]		-> AP_TO_LEDDRV_EN
	CFG_DISABLED | PULL_DOWN | DRIVE_X4 | SLOW_SLEW,		//   9 : ULPI_DATA[2]		-> NC
	CFG_DISABLED | PULL_DOWN | SLOW_SLEW,				//  10 : ULPI_DATA[1]		-> AP_BI_OSCAR_SWDIO_1V8
	CFG_DISABLED | PULL_DOWN | SLOW_SLEW,				//  11 : ULPI_DATA[0]		-> AP_TO_OSCAR_SWDCLK_1V8
	CFG_FUNC0 | PULL_DOWN | SLOW_SLEW | INPUT_SCHMITT,		//  12 : SPI1_SCLK		-> AP_TO_CODEC_SPI_CLK
	CFG_FUNC0 | PULL_DOWN | SLOW_SLEW,				//  13 : SPI1_MOSI		-> AP_TO_CODEC_SPI_MOSI
	CFG_FUNC0 | PULL_DOWN | SLOW_SLEW | INPUT_SCHMITT,		//  14 : SPI1_MISO		-> CODEC_TO_AP_SPI_MISO
	CFG_FUNC0 | PULL_DOWN | SLOW_SLEW,				//  15 : SPI1_SSIN		-> AP_TO_CODEC_SPI_CS_L

/* Port  2 */
	CFG_DISABLED,
	CFG_DISABLED,
	CFG_DISABLED,
	CFG_DISABLED,
	CFG_DISABLED,
	CFG_DISABLED,
	CFG_DISABLED,
	CFG_DISABLED,

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
	CFG_IN | SLOW_SLEW,						//  32 : GPIO[11]		-> MENU_KEY_L
	CFG_IN | SLOW_SLEW,						//  33 : GPIO[12]		-> HOLD_KEY_L
	CFG_DISABLED | PULL_DOWN | DRIVE_X4 | SLOW_SLEW,		//  34 : I2S3_MCK		-> NC
	CFG_FUNC0 | SLOW_SLEW | INPUT_SCHMITT,				//  35 : I2S3_LRCK		-> NC
	CFG_FUNC0 | SLOW_SLEW | INPUT_SCHMITT,				//  36 : I2S3_BCLK		-> NC
	CFG_FUNC0 | PULL_DOWN | SLOW_SLEW,				//  37 : I2S3_DOUT		-> NC
	CFG_FUNC0 | PULL_DOWN | SLOW_SLEW | INPUT_SCHMITT,		//  38 : I2S3_DIN		-> NC
	CFG_FUNC0 | PULL_DOWN | SLOW_SLEW,				//  39 : CLK32K_OUT		-> 45_AP_TO_TOUCH_CLK32K_RESET_L

/* Port  5 */
	CFG_DISABLED | PULL_DOWN | DRIVE_X4 | SLOW_SLEW,		//  40 : PCIE_CLKREQ0_N		-> NC
	CFG_FUNC0 | PULL_UP | INPUT_SCHMITT,				//  41 : PCIE_CLKREQ1_N		-> WLAN_TO_AP_PCIE1_CLKREQ_L
	CFG_DISABLED | PULL_DOWN | DRIVE_X4 | SLOW_SLEW,		//  42 : NAND_SYS_CLK		-> NC
	CFG_DISABLED | PULL_DOWN | DRIVE_X4 | SLOW_SLEW,		//  43 : GPIO[0]		-> NC
	CFG_DISABLED | PULL_DOWN | DRIVE_X4 | SLOW_SLEW,		//  44 : GPIO[1]		-> NC
	CFG_IN | PULL_UP | SLOW_SLEW,					//  45 : GPIO[2]		-> VOL_UP_L
	CFG_IN | PULL_UP | SLOW_SLEW,					//  46 : GPIO[3]		-> VOL_DWN_L
	CFG_IN | PULL_UP | SLOW_SLEW,					//  47 : GPIO[4]		-> SPKAMP_TO_AP_INT_L

/* Port  6 */
	CFG_OUT_0 | PULL_DOWN | SLOW_SLEW,				//  48 : GPIO[5]		-> AP_TO_SPKAMP_BEE_GEES
	CFG_OUT_0 | PULL_DOWN | SLOW_SLEW,				//  49 : GPIO[6]		-> AP_TO_SPKAMP_RESET_L
	CFG_OUT_0 | PULL_DOWN | SLOW_SLEW,				//  50 : GPIO[7]		-> AP_TO_BT_WAKE
	CFG_DISABLED | PULL_DOWN | DRIVE_X4 | SLOW_SLEW,		//  51 : GPIO[14]		-> NC
	CFG_DISABLED | PULL_UP | SLOW_SLEW,				//  52 : GPIO[16]		-> BOARD_ID3
	CFG_DISABLED | PULL_DOWN | DRIVE_X4 | SLOW_SLEW,		//  53 : GPIO[17]		-> NC
	CFG_DISABLED | PULL_UP | SLOW_SLEW,				//  54 : GPIO[18]		-> BOOT_CONFIG0
	CFG_DISABLED | PULL_DOWN | DRIVE_X4 | SLOW_SLEW,		//  55 : GPIO[20]		-> NC

/* Port  7 */
	CFG_DISABLED | PULL_DOWN | DRIVE_X4 | SLOW_SLEW,		//  56 : GPIO[21]		-> NC
	CFG_DISABLED | PULL_DOWN | DRIVE_X4 | SLOW_SLEW,		//  57 : UART5_RTXD		-> NC
	CFG_FUNC0 | PULL_UP | SLOW_SLEW,				//  58 : UART8_TXD		-> AP_TO_OSCAR_UART_TXD
	CFG_FUNC0 | SLOW_SLEW | INPUT_SCHMITT,				//  59 : UART8_RXD		-> OSCAR_TO_AP_UART_RXD
	CFG_DISABLED | PULL_UP | SLOW_SLEW,				//  60 : SPI0_SCLK		-> BOARD_ID0
	CFG_DISABLED | PULL_UP | SLOW_SLEW,				//  61 : SPI0_MOSI		-> BOARD_ID1
	CFG_DISABLED | PULL_UP | SLOW_SLEW,				//  62 : SPI0_MISO		-> BOARD_ID2
	CFG_DISABLED | PULL_DOWN | DRIVE_X4 | SLOW_SLEW,		//  63 : SPI0_SSIN		-> NC

/* Port  8 */
	CFG_FUNC0 | DRIVE_X2 | SLOW_SLEW | INPUT_SCHMITT,		//  64 : I2C2_SDA		-> AP_BI_I2C2_SDA
	CFG_FUNC0 | DRIVE_X2 | SLOW_SLEW | INPUT_SCHMITT,		//  65 : I2C2_SCL		-> AP_TO_I2C2_SCL
	CFG_DISABLED | PULL_DOWN | DRIVE_X4 | SLOW_SLEW,		//  66 : GPIO[22]		-> NC
	CFG_DISABLED | PULL_DOWN | DRIVE_X4 | SLOW_SLEW,		//  67 : GPIO[23]		-> NC
	CFG_DISABLED | PULL_UP | SLOW_SLEW,				//  68 : GPIO[25]		-> BOOT_CONFIG1
	CFG_DISABLED | PULL_UP | SLOW_SLEW,				//  69 : GPIO[28]		-> BOOT_CONFIG2
	CFG_DISABLED | PULL_UP | SLOW_SLEW,				//  70 : GPIO[29]		-> BOARD_ID4
	CFG_DISABLED | PULL_UP | SLOW_SLEW,				//  71 : GPIO[34]		-> BOARD_REV3

/* Port  9 */
	CFG_DISABLED | PULL_UP | SLOW_SLEW,				//  72 : GPIO[35]		-> BOARD_REV2
	CFG_DISABLED | PULL_UP | SLOW_SLEW,				//  73 : GPIO[36]		-> BOARD_REV1
	CFG_DISABLED | PULL_UP | SLOW_SLEW,				//  74 : GPIO[37]		-> BOARD_REV0
	CFG_DISABLED | PULL_DOWN | DRIVE_X4 | SLOW_SLEW,		//  75 : GPIO[39]		-> NC
	CFG_DISABLED | PULL_DOWN | DRIVE_X4 | SLOW_SLEW,		//  76 : GPIO[42]		-> NC
	CFG_IN | SLOW_SLEW,						//  77 : GPIO[43]		-> AP_TO_WLAN_PCIE1_RST_L
	CFG_DISABLED | PULL_DOWN | DRIVE_X4 | SLOW_SLEW,		//  78 : DISP_VSYNC		-> NC
	CFG_FUNC0 | PULL_UP | SLOW_SLEW,				//  79 : UART0_TXD		-> AP_TO_TRISTAR_DEBUG_UART0_TXD

/* Port 10 */
	CFG_FUNC0 | PULL_UP | SLOW_SLEW | INPUT_SCHMITT,		//  80 : UART0_RXD		-> TRISTAR_TO_AP_DEBUG_UART0_RXD
	CFG_OUT_0 | PULL_DOWN | SLOW_SLEW,				//  81 : TMR32_PWM0		-> OSCAR_BI_AP_TIME_SYNC_HOST_INT
	CFG_DISABLED | PULL_DOWN | DRIVE_X4 | SLOW_SLEW,		//  82 : TMR32_PWM1		-> NC
	CFG_DISABLED | PULL_DOWN | DRIVE_X4 | SLOW_SLEW,		//  83 : TMR32_PWM2		-> NC
	CFG_FUNC0 | PULL_UP | SLOW_SLEW,				//  84 : UART6_TXD		-> AP_TO_TRISTAR_ACC_UART6_TXD
	CFG_FUNC0 | PULL_UP | SLOW_SLEW | INPUT_SCHMITT,		//  85 : UART6_RXD		-> TRISTAR_TO_AP_ACC_UART6_RXD
	CFG_DISABLED | PULL_DOWN | DRIVE_X4 | SLOW_SLEW,		//  86 : I2C3_SDA		-> NC
	CFG_DISABLED | PULL_DOWN | DRIVE_X4 | SLOW_SLEW,		//  87 : I2C3_SCL		-> NC

/* Port 11 */
	CFG_DISABLED,
	CFG_DISABLED,
	CFG_DISABLED,
	CFG_DISABLED,
	CFG_DISABLED,
	CFG_DISABLED,
	CFG_DISABLED,
	CFG_DISABLED,

/* Port 12 */
	CFG_FUNC0 | SLOW_SLEW | INPUT_SCHMITT,				//  96 : I2C0_SDA		-> AP_BI_I2C0_SDA
	CFG_FUNC0 | SLOW_SLEW | INPUT_SCHMITT,				//  97 : I2C0_SCL		-> AP_TO_I2C0_SCL
	CFG_DISABLED | PULL_DOWN | DRIVE_X4 | SLOW_SLEW,		//  98 : GPIO[38]		-> NC
	CFG_DISABLED | PULL_DOWN | DRIVE_X4 | SLOW_SLEW,		//  99 : UART2_TXD		-> NC
	CFG_DISABLED | PULL_DOWN | DRIVE_X4 | SLOW_SLEW,		// 100 : UART2_RXD		-> NC
	CFG_DISABLED | PULL_DOWN | DRIVE_X4 | SLOW_SLEW,		// 101 : UART2_RTSN		-> NC
	CFG_DISABLED | PULL_DOWN | DRIVE_X4 | SLOW_SLEW,		// 102 : UART2_CTSN		-> NC
	CFG_FUNC0 | PULL_DOWN | SLOW_SLEW,				// 103 : DWI_DO			-> 45_AP_TO_PMU_AND_BL_DWI_DO

/* Port 13 */
	CFG_FUNC0 | PULL_DOWN | SLOW_SLEW,				// 104 : DWI_CLK		-> 45_AP_TO_PMU_AND_BL_DWI_CLK
	CFG_FUNC0 | PULL_DOWN | SLOW_SLEW,				// 105 : WDOG			-> AP_TO_PMU_RESET_IN
	CFG_IN | PULL_UP | SLOW_SLEW,					// 106 : GPIO[13]		-> PMU_TO_AP_IRQ_L
	CFG_OUT_0 | PULL_DOWN | SLOW_SLEW,				// 107 : GPIO[19]		-> AP_TO_PMU_KEEPACT
	CFG_DISABLED | PULL_DOWN | SLOW_SLEW,				// 108 : GPIO[26]		-> FORCE_DFU
	CFG_DISABLED | PULL_DOWN | DRIVE_X4 | SLOW_SLEW,		// 109 : GPIO[27]		-> NC
	CFG_FUNC0 | PULL_UP | SLOW_SLEW,				// 110 : SOCHOT0		-> PMU_TO_AP_PRE_UVLO_L_R
	CFG_FUNC0 | SLOW_SLEW,						// 111 : SOCHOT1		-> AP_TO_PMU_SOCHOT1_L_R

/* Port 14 */
	CFG_DISABLED,							// 112 : UNSPECIFIED		-> UNSPECIFIED
	CFG_DISABLED | PULL_DOWN | SLOW_SLEW,				// 113 : TST_CLKOUT		-> AP_TO_PMU_TEST_CLKOUT
	CFG_DISABLED | PULL_DOWN | DRIVE_X4 | SLOW_SLEW,		// 114 : GPIO[8]		-> NC
	CFG_DISABLED | SLOW_SLEW,					// 115 : GPIO[9]		-> AP_TO_WLAN_JTAG_SWCLK
	CFG_DISABLED | SLOW_SLEW,					// 116 : GPIO[10]		-> AP_TO_WLAN_JTAG_SWDIO
	CFG_DISABLED | PULL_DOWN | DRIVE_X4 | SLOW_SLEW,		// 117 : GPIO[15]		-> NC
	CFG_FUNC0 | PULL_UP | SLOW_SLEW,				// 118 : UART4_TXD		-> AP_TO_WLAN_UART4_TXD
	CFG_FUNC0 | SLOW_SLEW | INPUT_SCHMITT,				// 119 : UART4_RXD		-> WLAN_TO_AP_UART4_RXD

/* Port 15 */
	CFG_OUT_1 | PULL_DOWN | SLOW_SLEW,				// 120 : UART4_RTSN		-> AP_TO_WLAN_UART4_RTS_L
	CFG_FUNC0 | SLOW_SLEW,						// 121 : UART4_CTSN		-> WLAN_TO_AP_UART4_CTS_L
	CFG_DISABLED | PULL_DOWN | DRIVE_X4 | SLOW_SLEW,		// 122 : SPI3_MOSI		-> NC
	CFG_DISABLED | PULL_DOWN | DRIVE_X4 | SLOW_SLEW,		// 123 : SPI3_MISO		-> NC
	CFG_DISABLED | PULL_DOWN | DRIVE_X4 | SLOW_SLEW,		// 124 : SPI3_SCLK		-> NC
	CFG_DISABLED | PULL_DOWN | DRIVE_X4 | SLOW_SLEW,		// 125 : SPI3_SSIN		-> NC
	CFG_DISABLED | PULL_DOWN | DRIVE_X4 | SLOW_SLEW,		// 126 : GPIO[24]		-> NC
	CFG_IN | PULL_UP | SLOW_SLEW,					// 127 : GPIO[30]		-> CODEC_TO_AP_INT_L

/* Port 16 */
	CFG_DISABLED | PULL_DOWN | DRIVE_X4 | SLOW_SLEW,		// 128 : GPIO[31]		-> NC
	CFG_DISABLED | PULL_DOWN | DRIVE_X4 | SLOW_SLEW,		// 129 : GPIO[32]		-> NC
	CFG_DISABLED | PULL_DOWN | DRIVE_X4 | SLOW_SLEW,		// 130 : GPIO[33]		-> NC
	CFG_DISABLED | PULL_DOWN | DRIVE_X4 | SLOW_SLEW,		// 131 : GPIO[40]		-> NC
	CFG_OUT_0 | PULL_DOWN | SLOW_SLEW,				// 132 : GPIO[41]		-> AP_TO_TP_DIAGS_CTRL
	CFG_IN | SLOW_SLEW,						// 133 : I2S4_MCK		-> TRISTAR_TO_AP_INT
	CFG_FUNC0 | PULL_DOWN | SLOW_SLEW | INPUT_SCHMITT,		// 134 : I2S4_LRCK		-> AP_TO_CODEC_XSP_I2S4_LRCLK
	CFG_FUNC0 | PULL_DOWN | SLOW_SLEW | INPUT_SCHMITT,		// 135 : I2S4_BCLK		-> 45_AP_TO_CODEC_XSP_I2S4_BCLK

/* Port 17 */
	CFG_FUNC0 | PULL_DOWN | SLOW_SLEW,				// 136 : I2S4_DOUT		-> AP_TO_CODEC_XSP_I2S4_DOUT
	CFG_FUNC0 | PULL_DOWN | SLOW_SLEW | INPUT_SCHMITT,		// 137 : I2S4_DIN		-> CODEC_TO_AP_XSP_I2S4_DIN
	CFG_FUNC0 | SLOW_SLEW | INPUT_SCHMITT,				// 138 : I2C1_SDA		-> AP_BI_I2C1_SDA
	CFG_FUNC0 | SLOW_SLEW | INPUT_SCHMITT,				// 139 : I2C1_SCL		-> AP_TO_I2C1_SCL
	CFG_DISABLED,
	CFG_DISABLED,
	CFG_DISABLED,
	CFG_DISABLED,

/* Port 18 */
	CFG_DISABLED,
	CFG_DISABLED,
	CFG_DISABLED,
	CFG_DISABLED,
	CFG_DISABLED,
	CFG_DISABLED,
	CFG_DISABLED,
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
	CFG_FUNC0 | PULL_DOWN | SLOW_SLEW | INPUT_SCHMITT,		// 160 : I2S0_LRCK		-> AP_TO_CODEC_ASP_I2S0_LRCLK
	CFG_FUNC0 | PULL_DOWN | SLOW_SLEW | INPUT_SCHMITT,		// 161 : I2S0_BCLK		-> 45_AP_TO_CODEC_ASP_I2S0_BCLK
	CFG_FUNC0 | PULL_DOWN | SLOW_SLEW,				// 162 : I2S0_DOUT		-> AP_TO_CODEC_ASP_I2S0_DOUT
	CFG_FUNC0 | PULL_DOWN | SLOW_SLEW | INPUT_SCHMITT,		// 163 : I2S0_DIN		-> CODEC_TO_AP_ASP_I2S0_DIN
	CFG_DISABLED | PULL_DOWN | DRIVE_X4 | SLOW_SLEW,		// 164 : I2S1_MCK		-> NC
	CFG_FUNC0 | PULL_DOWN | SLOW_SLEW | INPUT_SCHMITT,		// 165 : I2S1_LRCK		-> AP_TO_BT_I2S1_LRCLK
	CFG_FUNC0 | PULL_DOWN | SLOW_SLEW | INPUT_SCHMITT,		// 166 : I2S1_BCLK		-> 45_AP_TO_BT_I2S1_BCLK
	CFG_FUNC0 | PULL_DOWN | SLOW_SLEW,				// 167 : I2S1_DOUT		-> AP_TO_BT_I2S1_DOUT

/* Port 21 */
	CFG_FUNC0 | PULL_DOWN | SLOW_SLEW | INPUT_SCHMITT,		// 168 : I2S1_DIN		-> BT_TO_AP_I2S1_DIN
	CFG_FUNC0 | PULL_DOWN | SLOW_SLEW | INPUT_SCHMITT,		// 169 : I2S2_LRCK		-> AP_TO_SPKAMP_I2S2_LRCLK
	CFG_FUNC0 | PULL_DOWN | SLOW_SLEW | INPUT_SCHMITT,		// 170 : I2S2_BCLK		-> 45_AP_TO_SPKAMP_I2S2_BCLK
	CFG_FUNC0 | PULL_DOWN | SLOW_SLEW,				// 171 : I2S2_DOUT		-> AP_TO_SPKAMP_I2S2_DOUT
	CFG_FUNC0 | PULL_DOWN | SLOW_SLEW | INPUT_SCHMITT,		// 172 : I2S2_DIN		-> SPKAMP_TO_AP_I2S2_DIN
	CFG_FUNC0 | PULL_UP | SLOW_SLEW,				// 173 : UART1_TXD		-> AP_TO_BT_UART1_TXD
	CFG_FUNC0 | PULL_UP | SLOW_SLEW | INPUT_SCHMITT,		// 174 : UART1_RXD		-> BT_TO_AP_UART1_RXD
	CFG_OUT_1 | PULL_DOWN | SLOW_SLEW,				// 175 : UART1_RTSN		-> AP_TO_BT_UART1_RTS_L

/* Port 22 */
	CFG_FUNC0 | PULL_UP | SLOW_SLEW,				// 176 : UART1_CTSN		-> BT_TO_AP_UART1_CTS_L
	CFG_DISABLED | PULL_DOWN | DRIVE_X4 | SLOW_SLEW,		// 177 : EDP_HPD		-> NC
	CFG_DISABLED | PULL_DOWN | DRIVE_X4 | SLOW_SLEW,		// 178 : UART3_TXD		-> NC
	CFG_DISABLED | PULL_DOWN | DRIVE_X4 | SLOW_SLEW,		// 179 : UART3_RXD		-> NC
	CFG_DISABLED | PULL_DOWN | DRIVE_X4 | SLOW_SLEW,		// 180 : UART3_RTSN		-> NC
	CFG_DISABLED | PULL_DOWN | DRIVE_X4 | SLOW_SLEW,		// 181 : UART3_CTSN		-> NC
	CFG_FUNC0 | PULL_DOWN | SLOW_SLEW | INPUT_SCHMITT,		// 182 : SPI2_SCLK		-> AP_TO_TOUCH_SPI_CLK
	CFG_FUNC0 | PULL_DOWN | SLOW_SLEW,				// 183 : SPI2_MOSI		-> AP_TO_TOUCH_SPI_MOSI

/* Port 23 */
	CFG_FUNC0 | PULL_DOWN | SLOW_SLEW | INPUT_SCHMITT,		// 184 : SPI2_MISO		-> TOUCH_TO_AP_SPI_MISO
	CFG_FUNC0 | PULL_UP | SLOW_SLEW,				// 185 : SPI2_SSIN		-> AP_TO_TOUCH_SPI_CS_L
	CFG_FUNC0 | SLOW_SLEW | INPUT_SCHMITT,				// 186 : ISP0_SDA		-> AP_BI_RCAM_I2C_SDA
	CFG_FUNC0 | SLOW_SLEW | INPUT_SCHMITT,				// 187 : ISP0_SCL		-> AP_TO_RCAM_I2C_SCL
	CFG_FUNC0 | SLOW_SLEW | INPUT_SCHMITT,				// 188 : ISP1_SDA		-> AP_BI_FCAM_I2C_SDA
	CFG_FUNC0 | SLOW_SLEW | INPUT_SCHMITT,				// 189 : ISP1_SCL		-> AP_TO_FCAM_I2C_SCL
	CFG_OUT_0 | PULL_DOWN | DRIVE_X2 | SLOW_SLEW,			// 190 : SENSOR0_RST		-> AP_TO_RCAM_SHUTDOWN
	CFG_DISABLED | PULL_DOWN | DRIVE_X2 | SLOW_SLEW,		// 191 : SENSOR0_CLK		-> 45_AP_TO_RCAM_CLK_R

/* Port 24 */
	CFG_DISABLED | PULL_DOWN | DRIVE_X4 | SLOW_SLEW,		// 192 : SENSOR0_XSHUTDOWN	-> NC
	CFG_DISABLED | PULL_DOWN | DRIVE_X4 | SLOW_SLEW,		// 193 : SENSOR0_ISTRB		-> NC
	CFG_FUNC0 | PULL_UP | SLOW_SLEW,				// 194 : ISP_UART0_TXD		-> AP_ISP_TO_OSCAR_UART_TXD
	CFG_FUNC0 | SLOW_SLEW | INPUT_SCHMITT,				// 195 : ISP_UART0_RXD		-> OSCAR_TO_AP_ISP_UART_RXD
	CFG_OUT_0 | PULL_DOWN | DRIVE_X2 | SLOW_SLEW,			// 196 : SENSOR1_RST		-> AP_TO_FCAM_SHUTDOWN
	CFG_DISABLED | PULL_DOWN | DRIVE_X2 | SLOW_SLEW,		// 197 : SENSOR1_CLK		-> 45_AP_TO_FCAM_CLK_R
	CFG_OUT_0 | PULL_DOWN | SLOW_SLEW,				// 198 : SENSOR1_XSHUTDOWN	-> CAM_EXT_LDO_EN
	CFG_DISABLED | PULL_DOWN | DRIVE_X4 | SLOW_SLEW,		// 199 : SENSOR1_ISTRB		-> NC

/* Port 25 */
	CFG_OUT_0 | PULL_DOWN | SLOW_SLEW,				// 200 : UART7_TXD		-> AP_TO_WLAN_DEVICE_WAKE
	CFG_DISABLED | PULL_DOWN | DRIVE_X4 | SLOW_SLEW,		// 201 : UART7_RXD		-> NC
	CFG_FUNC0 | PULL_DOWN | SLOW_SLEW | INPUT_SCHMITT,		// 202 : I2S0_MCK		-> 45_AP_TO_CODEC_I2S0_MCLK_R
	CFG_FUNC0 | PULL_DOWN | SLOW_SLEW | INPUT_SCHMITT,		// 203 : I2S2_MCK		-> 45_AP_TO_SPKAMP_I2S2_MCLK_R
	CFG_DISABLED,
	CFG_DISABLED,
	CFG_DISABLED,
	CFG_DISABLED,
};

static const uint32_t pinconfig_dev_0[GPIO_GROUP_COUNT * GPIOPADPINS] = {

/* Port  0 */
	CFG_DISABLED | PULL_DOWN | SLOW_SLEW,				//   0 : ULPI_DIR		-> LCM_TO_AP_HIFA_BSYNC
	CFG_OUT_0 | PULL_DOWN | DRIVE_X4 | SLOW_SLEW,			//   1 : ULPI_STP		-> AP_TO_LCM_RESET_L
	CFG_OUT_0 | PULL_DOWN | DRIVE_X4 | SLOW_SLEW,			//   2 : ULPI_NXT		-> AP_TO_TOUCH_RESET_L
	CFG_IN | PULL_UP | SLOW_SLEW,					//   3 : ULPI_DATA[7]		-> TOUCH_TO_AP_INT_L
	CFG_DISABLED | PULL_DOWN | DRIVE_X4 | SLOW_SLEW,		//   4 : ULPI_DATA[6]		-> NC
	CFG_DISABLED | PULL_DOWN | DRIVE_X4 | SLOW_SLEW,		//   5 : ULPI_DATA[5]		-> NC
	CFG_DISABLED | PULL_DOWN | DRIVE_X4 | SLOW_SLEW,		//   6 : ULPI_DATA[4]		-> NC
	CFG_IN | PULL_DOWN | SLOW_SLEW,					//   7 : ULPI_CLK		-> OSCAR_TO_PMU_HOST_WAKE

/* Port  1 */
	CFG_OUT_0 | PULL_DOWN | DRIVE_X4 | SLOW_SLEW,			//   8 : ULPI_DATA[3]		-> AP_TO_LEDDRV_EN
	CFG_DISABLED | PULL_DOWN | DRIVE_X4 | SLOW_SLEW,		//   9 : ULPI_DATA[2]		-> NC
	CFG_DISABLED | PULL_DOWN | DRIVE_X4 | SLOW_SLEW,		//  10 : ULPI_DATA[1]		-> AP_BI_OSCAR_SWDIO_1V8
	CFG_DISABLED | PULL_DOWN | DRIVE_X4 | SLOW_SLEW,		//  11 : ULPI_DATA[0]		-> AP_TO_OSCAR_SWDCLK_1V8
	CFG_FUNC0 | PULL_DOWN | SLOW_SLEW | INPUT_SCHMITT,		//  12 : SPI1_SCLK		-> AP_TO_CODEC_SPI_CLK
	CFG_FUNC0 | PULL_DOWN | SLOW_SLEW,				//  13 : SPI1_MOSI		-> AP_TO_CODEC_SPI_MOSI
	CFG_FUNC0 | PULL_DOWN | SLOW_SLEW | INPUT_SCHMITT,		//  14 : SPI1_MISO		-> CODEC_TO_AP_SPI_MISO
	CFG_FUNC0 | PULL_DOWN | SLOW_SLEW,				//  15 : SPI1_SSIN		-> AP_TO_CODEC_SPI_CS_L

/* Port  2 */
	CFG_DISABLED,
	CFG_DISABLED,
	CFG_DISABLED,
	CFG_DISABLED,
	CFG_DISABLED,
	CFG_DISABLED,
	CFG_DISABLED,
	CFG_DISABLED,

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
	CFG_IN | SLOW_SLEW,						//  32 : GPIO[11]		-> MENU_KEY_L
	CFG_IN | SLOW_SLEW,						//  33 : GPIO[12]		-> HOLD_KEY_L
	CFG_DISABLED | PULL_DOWN | DRIVE_X4 | SLOW_SLEW,		//  34 : I2S3_MCK		-> NC
	CFG_FUNC0 | DRIVE_X4 | SLOW_SLEW | INPUT_SCHMITT,		//  35 : I2S3_LRCK		-> NC
	CFG_FUNC0 | DRIVE_X4 | SLOW_SLEW | INPUT_SCHMITT,		//  36 : I2S3_BCLK		-> NC
	CFG_FUNC0 | PULL_DOWN | DRIVE_X4 | SLOW_SLEW,			//  37 : I2S3_DOUT		-> NC
	CFG_FUNC0 | PULL_DOWN | SLOW_SLEW | INPUT_SCHMITT,		//  38 : I2S3_DIN		-> NC
	CFG_FUNC0 | PULL_DOWN | DRIVE_X4 | SLOW_SLEW,			//  39 : CLK32K_OUT		-> 45_AP_TO_TOUCH_CLK32K_RESET_L

/* Port  5 */
	CFG_DISABLED | PULL_DOWN | DRIVE_X4 | SLOW_SLEW,		//  40 : PCIE_CLKREQ0_N		-> NC
	CFG_FUNC0 | PULL_UP | INPUT_SCHMITT,				//  41 : PCIE_CLKREQ1_N		-> WLAN_TO_AP_PCIE1_CLKREQ_L
	CFG_DISABLED | PULL_DOWN | DRIVE_X4 | SLOW_SLEW,		//  42 : NAND_SYS_CLK		-> NC
	CFG_DISABLED | PULL_DOWN | DRIVE_X4 | SLOW_SLEW,		//  43 : GPIO[0]		-> NC
	CFG_DISABLED | PULL_DOWN | DRIVE_X4 | SLOW_SLEW,		//  44 : GPIO[1]		-> NC
	CFG_IN | PULL_UP | SLOW_SLEW,					//  45 : GPIO[2]		-> VOL_UP_L
	CFG_IN | PULL_UP | SLOW_SLEW,					//  46 : GPIO[3]		-> VOL_DWN_L
	CFG_IN | PULL_UP | SLOW_SLEW,					//  47 : GPIO[4]		-> SPKAMP_TO_AP_INT_L

/* Port  6 */
	CFG_OUT_0 | PULL_DOWN | DRIVE_X4 | SLOW_SLEW,			//  48 : GPIO[5]		-> AP_TO_SPKAMP_BEE_GEES
	CFG_OUT_0 | PULL_DOWN | DRIVE_X4 | SLOW_SLEW,			//  49 : GPIO[6]		-> AP_TO_SPKAMP_RESET_L
	CFG_OUT_0 | PULL_DOWN | DRIVE_X4 | SLOW_SLEW,			//  50 : GPIO[7]		-> AP_TO_BT_WAKE
	CFG_DISABLED | PULL_DOWN | DRIVE_X4 | SLOW_SLEW,		//  51 : GPIO[14]		-> NC
	CFG_DISABLED | PULL_UP | SLOW_SLEW,				//  52 : GPIO[16]		-> BOARD_ID3
	CFG_DISABLED | PULL_DOWN | DRIVE_X4 | SLOW_SLEW,		//  53 : GPIO[17]		-> NC
	CFG_DISABLED | PULL_UP | SLOW_SLEW,				//  54 : GPIO[18]		-> BOOT_CONFIG0
	CFG_DISABLED | PULL_DOWN | DRIVE_X4 | SLOW_SLEW,		//  55 : GPIO[20]		-> NC

/* Port  7 */
	CFG_DISABLED | PULL_DOWN | DRIVE_X4 | SLOW_SLEW,		//  56 : GPIO[21]		-> NC
	CFG_DISABLED | PULL_DOWN | DRIVE_X4 | SLOW_SLEW,		//  57 : UART5_RTXD		-> NC
	CFG_FUNC0 | PULL_UP | DRIVE_X4 | SLOW_SLEW,			//  58 : UART8_TXD		-> AP_TO_OSCAR_UART_TXD
	CFG_FUNC0 | SLOW_SLEW | INPUT_SCHMITT,				//  59 : UART8_RXD		-> OSCAR_TO_AP_UART_RXD
	CFG_DISABLED | PULL_UP | SLOW_SLEW,				//  60 : SPI0_SCLK		-> BOARD_ID0
	CFG_DISABLED | PULL_UP | SLOW_SLEW,				//  61 : SPI0_MOSI		-> BOARD_ID1
	CFG_DISABLED | PULL_UP | SLOW_SLEW,				//  62 : SPI0_MISO		-> BOARD_ID2
	CFG_DISABLED | PULL_DOWN | DRIVE_X4 | SLOW_SLEW,		//  63 : SPI0_SSIN		-> NC

/* Port  8 */
	CFG_FUNC0 | DRIVE_X4 | SLOW_SLEW | INPUT_SCHMITT,		//  64 : I2C2_SDA		-> AP_BI_I2C2_SDA
	CFG_FUNC0 | DRIVE_X4 | SLOW_SLEW | INPUT_SCHMITT,		//  65 : I2C2_SCL		-> AP_TO_I2C2_SCL
	CFG_DISABLED | PULL_DOWN | DRIVE_X4 | SLOW_SLEW,		//  66 : GPIO[22]		-> NC
	CFG_DISABLED | PULL_DOWN | DRIVE_X4 | SLOW_SLEW,		//  67 : GPIO[23]		-> NC
	CFG_DISABLED | PULL_UP | SLOW_SLEW,				//  68 : GPIO[25]		-> BOOT_CONFIG1
	CFG_DISABLED | PULL_UP | SLOW_SLEW,				//  69 : GPIO[28]		-> BOOT_CONFIG2
	CFG_DISABLED | PULL_UP | SLOW_SLEW,				//  70 : GPIO[29]		-> BOARD_ID4
	CFG_DISABLED | PULL_UP | SLOW_SLEW,				//  71 : GPIO[34]		-> BOARD_REV3

/* Port  9 */
	CFG_DISABLED | PULL_UP | SLOW_SLEW,				//  72 : GPIO[35]		-> BOARD_REV2
	CFG_DISABLED | PULL_UP | SLOW_SLEW,				//  73 : GPIO[36]		-> BOARD_REV1
	CFG_DISABLED | PULL_UP | SLOW_SLEW,				//  74 : GPIO[37]		-> BOARD_REV0
	CFG_DISABLED | PULL_DOWN | DRIVE_X4 | SLOW_SLEW,		//  75 : GPIO[39]		-> NC
	CFG_DISABLED | PULL_DOWN | DRIVE_X4 | SLOW_SLEW,		//  76 : GPIO[42]		-> NC
	CFG_IN | DRIVE_X4 | SLOW_SLEW,					//  77 : GPIO[43]		-> AP_TO_WLAN_PCIE1_RST_L
	CFG_DISABLED | PULL_DOWN | DRIVE_X4 | SLOW_SLEW,		//  78 : DISP_VSYNC		-> NC
	CFG_FUNC0 | PULL_UP | DRIVE_X4 | SLOW_SLEW,			//  79 : UART0_TXD		-> AP_TO_TRISTAR_DEBUG_UART0_TXD

/* Port 10 */
	CFG_FUNC0 | PULL_UP | SLOW_SLEW | INPUT_SCHMITT,		//  80 : UART0_RXD		-> TRISTAR_TO_AP_DEBUG_UART0_RXD
	CFG_OUT_0 | PULL_DOWN | DRIVE_X4 | SLOW_SLEW,			//  81 : TMR32_PWM0		-> OSCAR_BI_AP_TIME_SYNC_HOST_INT
	CFG_DISABLED | PULL_DOWN | DRIVE_X4 | SLOW_SLEW,		//  82 : TMR32_PWM1		-> NC
	CFG_DISABLED | PULL_DOWN | DRIVE_X4 | SLOW_SLEW,		//  83 : TMR32_PWM2		-> NC
	CFG_FUNC0 | PULL_UP | DRIVE_X4 | SLOW_SLEW,			//  84 : UART6_TXD		-> AP_TO_TRISTAR_ACC_UART6_TXD
	CFG_FUNC0 | PULL_UP | SLOW_SLEW | INPUT_SCHMITT,		//  85 : UART6_RXD		-> TRISTAR_TO_AP_ACC_UART6_RXD
	CFG_DISABLED | PULL_DOWN | DRIVE_X4 | SLOW_SLEW,		//  86 : I2C3_SDA		-> NC
	CFG_DISABLED | PULL_DOWN | DRIVE_X4 | SLOW_SLEW,		//  87 : I2C3_SCL		-> NC

/* Port 11 */
	CFG_DISABLED,
	CFG_DISABLED,
	CFG_DISABLED,
	CFG_DISABLED,
	CFG_DISABLED,
	CFG_DISABLED,
	CFG_DISABLED,
	CFG_DISABLED,

/* Port 12 */
	CFG_FUNC0 | DRIVE_X4 | SLOW_SLEW | INPUT_SCHMITT,		//  96 : I2C0_SDA		-> AP_BI_I2C0_SDA
	CFG_FUNC0 | DRIVE_X4 | SLOW_SLEW | INPUT_SCHMITT,		//  97 : I2C0_SCL		-> AP_TO_I2C0_SCL
	CFG_DISABLED | PULL_DOWN | DRIVE_X4 | SLOW_SLEW,		//  98 : GPIO[38]		-> NC
	CFG_DISABLED | PULL_DOWN | DRIVE_X4 | SLOW_SLEW,		//  99 : UART2_TXD		-> NC
	CFG_DISABLED | PULL_DOWN | DRIVE_X4 | SLOW_SLEW,		// 100 : UART2_RXD		-> NC
	CFG_DISABLED | PULL_DOWN | DRIVE_X4 | SLOW_SLEW,		// 101 : UART2_RTSN		-> NC
	CFG_DISABLED | PULL_DOWN | DRIVE_X4 | SLOW_SLEW,		// 102 : UART2_CTSN		-> NC
	CFG_FUNC0 | PULL_DOWN | DRIVE_X4 | SLOW_SLEW,			// 103 : DWI_DO			-> 45_AP_TO_PMU_AND_BL_DWI_DO

/* Port 13 */
	CFG_FUNC0 | PULL_DOWN | DRIVE_X4 | SLOW_SLEW,			// 104 : DWI_CLK		-> 45_AP_TO_PMU_AND_BL_DWI_CLK
	CFG_FUNC0 | PULL_DOWN | DRIVE_X4 | SLOW_SLEW,			// 105 : WDOG			-> AP_TO_PMU_RESET_IN
	CFG_IN | PULL_UP | SLOW_SLEW,					// 106 : GPIO[13]		-> PMU_TO_AP_IRQ_L
	CFG_OUT_0 | PULL_DOWN | DRIVE_X4 | SLOW_SLEW,			// 107 : GPIO[19]		-> AP_TO_PMU_KEEPACT
	CFG_DISABLED | PULL_DOWN | SLOW_SLEW,				// 108 : GPIO[26]		-> FORCE_DFU
	CFG_DISABLED | PULL_DOWN | DRIVE_X4 | SLOW_SLEW,		// 109 : GPIO[27]		-> NC
	CFG_FUNC0 | PULL_UP | SLOW_SLEW,				// 110 : SOCHOT0		-> PMU_TO_AP_PRE_UVLO_L_R
	CFG_FUNC0 | DRIVE_X4 | SLOW_SLEW,				// 111 : SOCHOT1		-> AP_TO_PMU_SOCHOT1_L_R

/* Port 14 */
	CFG_DISABLED,							// 112 : UNSPECIFIED		-> UNSPECIFIED
	CFG_DISABLED | PULL_DOWN | DRIVE_X4 | SLOW_SLEW,		// 113 : TST_CLKOUT		-> AP_TO_PMU_TEST_CLKOUT
	CFG_DISABLED | PULL_DOWN | DRIVE_X4 | SLOW_SLEW,		// 114 : GPIO[8]		-> NC
	CFG_DISABLED | DRIVE_X4 | SLOW_SLEW,				// 115 : GPIO[9]		-> AP_TO_WLAN_JTAG_SWCLK
	CFG_DISABLED | DRIVE_X4 | SLOW_SLEW,				// 116 : GPIO[10]		-> AP_TO_WLAN_JTAG_SWDIO
	CFG_DISABLED | PULL_DOWN | DRIVE_X4 | SLOW_SLEW,		// 117 : GPIO[15]		-> NC
	CFG_FUNC0 | PULL_UP | DRIVE_X4 | SLOW_SLEW,			// 118 : UART4_TXD		-> AP_TO_WLAN_UART4_TXD
	CFG_FUNC0 | SLOW_SLEW | INPUT_SCHMITT,				// 119 : UART4_RXD		-> WLAN_TO_AP_UART4_RXD

/* Port 15 */
	CFG_OUT_1 | PULL_DOWN | DRIVE_X4 | SLOW_SLEW,			// 120 : UART4_RTSN		-> AP_TO_WLAN_UART4_RTS_L
	CFG_FUNC0 | SLOW_SLEW,						// 121 : UART4_CTSN		-> WLAN_TO_AP_UART4_CTS_L
	CFG_DISABLED | PULL_DOWN | DRIVE_X4 | SLOW_SLEW,		// 122 : SPI3_MOSI		-> NC
	CFG_DISABLED | PULL_DOWN | DRIVE_X4 | SLOW_SLEW,		// 123 : SPI3_MISO		-> NC
	CFG_DISABLED | PULL_DOWN | DRIVE_X4 | SLOW_SLEW,		// 124 : SPI3_SCLK		-> NC
	CFG_DISABLED | PULL_DOWN | DRIVE_X4 | SLOW_SLEW,		// 125 : SPI3_SSIN		-> NC
	CFG_DISABLED | PULL_DOWN | DRIVE_X4 | SLOW_SLEW,		// 126 : GPIO[24]		-> NC
	CFG_IN | PULL_UP | SLOW_SLEW,					// 127 : GPIO[30]		-> CODEC_TO_AP_INT_L

/* Port 16 */
	CFG_DISABLED | PULL_DOWN | DRIVE_X4 | SLOW_SLEW,		// 128 : GPIO[31]		-> NC
	CFG_DISABLED | PULL_DOWN | DRIVE_X4 | SLOW_SLEW,		// 129 : GPIO[32]		-> NC
	CFG_DISABLED | PULL_DOWN | DRIVE_X4 | SLOW_SLEW,		// 130 : GPIO[33]		-> NC
	CFG_DISABLED | PULL_DOWN | DRIVE_X4 | SLOW_SLEW,		// 131 : GPIO[40]		-> NC
	CFG_OUT_0 | PULL_DOWN | DRIVE_X4 | SLOW_SLEW,			// 132 : GPIO[41]		-> AP_TO_TP_DIAGS_CTRL
	CFG_IN | SLOW_SLEW,						// 133 : I2S4_MCK		-> TRISTAR_TO_AP_INT
	CFG_FUNC0 | PULL_DOWN | DRIVE_X4 | SLOW_SLEW | INPUT_SCHMITT,	// 134 : I2S4_LRCK		-> AP_TO_CODEC_XSP_I2S4_LRCLK
	CFG_FUNC0 | PULL_DOWN | DRIVE_X4 | SLOW_SLEW | INPUT_SCHMITT,	// 135 : I2S4_BCLK		-> 45_AP_TO_CODEC_XSP_I2S4_BCLK

/* Port 17 */
	CFG_FUNC0 | PULL_DOWN | DRIVE_X4 | SLOW_SLEW,			// 136 : I2S4_DOUT		-> AP_TO_CODEC_XSP_I2S4_DOUT
	CFG_FUNC0 | PULL_DOWN | SLOW_SLEW | INPUT_SCHMITT,		// 137 : I2S4_DIN		-> CODEC_TO_AP_XSP_I2S4_DIN
	CFG_FUNC0 | DRIVE_X4 | SLOW_SLEW | INPUT_SCHMITT,		// 138 : I2C1_SDA		-> AP_BI_I2C1_SDA
	CFG_FUNC0 | DRIVE_X4 | SLOW_SLEW | INPUT_SCHMITT,		// 139 : I2C1_SCL		-> AP_TO_I2C1_SCL
	CFG_DISABLED,
	CFG_DISABLED,
	CFG_DISABLED,
	CFG_DISABLED,

/* Port 18 */
	CFG_DISABLED,
	CFG_DISABLED,
	CFG_DISABLED,
	CFG_DISABLED,
	CFG_DISABLED,
	CFG_DISABLED,
	CFG_DISABLED,
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
	CFG_FUNC0 | PULL_DOWN | DRIVE_X4 | SLOW_SLEW | INPUT_SCHMITT,	// 160 : I2S0_LRCK		-> AP_TO_CODEC_ASP_I2S0_LRCLK
	CFG_FUNC0 | PULL_DOWN | DRIVE_X4 | SLOW_SLEW | INPUT_SCHMITT,	// 161 : I2S0_BCLK		-> 45_AP_TO_CODEC_ASP_I2S0_BCLK
	CFG_FUNC0 | PULL_DOWN | DRIVE_X4 | SLOW_SLEW,			// 162 : I2S0_DOUT		-> AP_TO_CODEC_ASP_I2S0_DOUT
	CFG_FUNC0 | PULL_DOWN | SLOW_SLEW | INPUT_SCHMITT,		// 163 : I2S0_DIN		-> CODEC_TO_AP_ASP_I2S0_DIN
	CFG_DISABLED | PULL_DOWN | DRIVE_X4 | SLOW_SLEW,		// 164 : I2S1_MCK		-> NC
	CFG_FUNC0 | PULL_DOWN | DRIVE_X4 | SLOW_SLEW | INPUT_SCHMITT,	// 165 : I2S1_LRCK		-> AP_TO_BT_I2S1_LRCLK
	CFG_FUNC0 | PULL_DOWN | DRIVE_X4 | SLOW_SLEW | INPUT_SCHMITT,	// 166 : I2S1_BCLK		-> 45_AP_TO_BT_I2S1_BCLK
	CFG_FUNC0 | PULL_DOWN | DRIVE_X4 | SLOW_SLEW,			// 167 : I2S1_DOUT		-> AP_TO_BT_I2S1_DOUT

/* Port 21 */
	CFG_FUNC0 | PULL_DOWN | SLOW_SLEW | INPUT_SCHMITT,		// 168 : I2S1_DIN		-> BT_TO_AP_I2S1_DIN
	CFG_FUNC0 | PULL_DOWN | DRIVE_X4 | SLOW_SLEW | INPUT_SCHMITT,	// 169 : I2S2_LRCK		-> AP_TO_SPKAMP_I2S2_LRCLK
	CFG_FUNC0 | PULL_DOWN | DRIVE_X4 | SLOW_SLEW | INPUT_SCHMITT,	// 170 : I2S2_BCLK		-> 45_AP_TO_SPKAMP_I2S2_BCLK
	CFG_FUNC0 | PULL_DOWN | DRIVE_X4 | SLOW_SLEW,			// 171 : I2S2_DOUT		-> AP_TO_SPKAMP_I2S2_DOUT
	CFG_FUNC0 | PULL_DOWN | SLOW_SLEW | INPUT_SCHMITT,		// 172 : I2S2_DIN		-> SPKAMP_TO_AP_I2S2_DIN
	CFG_FUNC0 | PULL_UP | DRIVE_X4 | SLOW_SLEW,			// 173 : UART1_TXD		-> AP_TO_BT_UART1_TXD
	CFG_FUNC0 | PULL_UP | SLOW_SLEW | INPUT_SCHMITT,		// 174 : UART1_RXD		-> BT_TO_AP_UART1_RXD
	CFG_OUT_1 | PULL_DOWN | DRIVE_X4 | SLOW_SLEW,			// 175 : UART1_RTSN		-> AP_TO_BT_UART1_RTS_L

/* Port 22 */
	CFG_FUNC0 | PULL_UP | SLOW_SLEW,				// 176 : UART1_CTSN		-> BT_TO_AP_UART1_CTS_L
	CFG_DISABLED | PULL_DOWN | DRIVE_X4 | SLOW_SLEW,		// 177 : EDP_HPD		-> NC
	CFG_DISABLED | PULL_DOWN | DRIVE_X4 | SLOW_SLEW,		// 178 : UART3_TXD		-> NC
	CFG_DISABLED | PULL_DOWN | DRIVE_X4 | SLOW_SLEW,		// 179 : UART3_RXD		-> NC
	CFG_DISABLED | PULL_DOWN | DRIVE_X4 | SLOW_SLEW,		// 180 : UART3_RTSN		-> NC
	CFG_DISABLED | PULL_DOWN | DRIVE_X4 | SLOW_SLEW,		// 181 : UART3_CTSN		-> NC
	CFG_FUNC0 | PULL_DOWN | DRIVE_X4 | SLOW_SLEW | INPUT_SCHMITT,	// 182 : SPI2_SCLK		-> AP_TO_TOUCH_SPI_CLK
	CFG_FUNC0 | PULL_DOWN | DRIVE_X4 | SLOW_SLEW,			// 183 : SPI2_MOSI		-> AP_TO_TOUCH_SPI_MOSI

/* Port 23 */
	CFG_FUNC0 | PULL_DOWN | SLOW_SLEW | INPUT_SCHMITT,		// 184 : SPI2_MISO		-> TOUCH_TO_AP_SPI_MISO
	CFG_FUNC0 | PULL_UP | DRIVE_X4 | SLOW_SLEW,			// 185 : SPI2_SSIN		-> AP_TO_TOUCH_SPI_CS_L
	CFG_FUNC0 | DRIVE_X2 | SLOW_SLEW | INPUT_SCHMITT,		// 186 : ISP0_SDA		-> AP_BI_RCAM_I2C_SDA
	CFG_FUNC0 | DRIVE_X2 | SLOW_SLEW | INPUT_SCHMITT,		// 187 : ISP0_SCL		-> AP_TO_RCAM_I2C_SCL
	CFG_FUNC0 | DRIVE_X2 | SLOW_SLEW | INPUT_SCHMITT,		// 188 : ISP1_SDA		-> AP_BI_FCAM_I2C_SDA
	CFG_FUNC0 | DRIVE_X2 | SLOW_SLEW | INPUT_SCHMITT,		// 189 : ISP1_SCL		-> AP_TO_FCAM_I2C_SCL
	CFG_OUT_0 | PULL_DOWN | DRIVE_X2 | SLOW_SLEW,			// 190 : SENSOR0_RST		-> AP_TO_RCAM_SHUTDOWN
	CFG_DISABLED | PULL_DOWN | DRIVE_X4 | SLOW_SLEW,		// 191 : SENSOR0_CLK		-> 45_AP_TO_RCAM_CLK_R

/* Port 24 */
	CFG_DISABLED | PULL_DOWN | DRIVE_X4 | SLOW_SLEW,		// 192 : SENSOR0_XSHUTDOWN	-> NC
	CFG_DISABLED | PULL_DOWN | DRIVE_X4 | SLOW_SLEW,		// 193 : SENSOR0_ISTRB		-> NC
	CFG_FUNC0 | PULL_UP | DRIVE_X4 | SLOW_SLEW,			// 194 : ISP_UART0_TXD		-> AP_ISP_TO_OSCAR_UART_TXD
	CFG_FUNC0 | SLOW_SLEW | INPUT_SCHMITT,				// 195 : ISP_UART0_RXD		-> OSCAR_TO_AP_ISP_UART_RXD
	CFG_OUT_0 | PULL_DOWN | DRIVE_X2 | SLOW_SLEW,			// 196 : SENSOR1_RST		-> AP_TO_FCAM_SHUTDOWN
	CFG_DISABLED | PULL_DOWN | DRIVE_X4 | SLOW_SLEW,		// 197 : SENSOR1_CLK		-> 45_AP_TO_FCAM_CLK_R
	CFG_OUT_0 | PULL_DOWN | DRIVE_X4 | SLOW_SLEW,			// 198 : SENSOR1_XSHUTDOWN	-> CAM_EXT_LDO_EN
	CFG_DISABLED | PULL_DOWN | DRIVE_X4 | SLOW_SLEW,		// 199 : SENSOR1_ISTRB		-> NC

/* Port 25 */
	CFG_OUT_0 | PULL_DOWN | DRIVE_X4 | SLOW_SLEW,			// 200 : UART7_TXD		-> AP_TO_WLAN_DEVICE_WAKE
	CFG_DISABLED | PULL_DOWN | DRIVE_X4 | SLOW_SLEW,		// 201 : UART7_RXD		-> NC
	CFG_FUNC0 | PULL_DOWN | DRIVE_X4 | SLOW_SLEW | INPUT_SCHMITT,	// 202 : I2S0_MCK		-> 45_AP_TO_CODEC_I2S0_MCLK_R
	CFG_FUNC0 | PULL_DOWN | DRIVE_X4 | SLOW_SLEW | INPUT_SCHMITT,	// 203 : I2S2_MCK		-> 45_AP_TO_SPKAMP_I2S2_MCLK_R
	CFG_DISABLED,
	CFG_DISABLED,
	CFG_DISABLED,
	CFG_DISABLED,
};

struct pinconfig_map {
	uint32_t board_id;
	uint32_t board_id_mask;
	const uint32_t *pinconfigs[GPIOC_COUNT];
};

static const struct pinconfig_map cfg_map[] = {
	{ 0, 1, { pinconfig_ap_0 } },
	{ 1, 1, { pinconfig_dev_0 } },
};

const uint32_t * target_get_default_gpio_cfg(int gpioc)
{
	static const struct pinconfig_map *selected_map = NULL;

	if (selected_map == NULL) {
		uint32_t board_id = platform_get_board_id();
		for (unsigned i = 0; i < sizeof(cfg_map)/sizeof(cfg_map[0]); i++) {
			if ((board_id & cfg_map[i].board_id_mask) == cfg_map[i].board_id) {
				selected_map = &cfg_map[i];
				break;
			}
		}

		if (selected_map == NULL)
			panic("no default pinconfig for board id %u", board_id);
	}

	ASSERT(gpioc < GPIOC_COUNT);
	return selected_map->pinconfigs[gpioc];
}
