/*
 * Copyright (C) 2014-2015 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */

/* THIS FILE IS AUTOMATICALLY GENERATED BY tools/csvtopinconfig.py. DO NOT EDIT!
   I/O Spreadsheet version: rev 0.4.1
   I/O Spreadsheet tracker: 15171729
   Conversion command: csvtopinconfig.py --radar 15171729 --soc m7 --prefix evt --copyright 2014-2015 <filename>
*/

#include <debug.h>
#include <drivers/apple/gpio.h>
#include <platform.h>
#include <platform/soc/hwregbase.h>
#include <stdint.h>

static const uint32_t pinconfig_evt_ap_0[GPIO_GROUP_COUNT * GPIOPADPINS] = {

/* Port  0 */
	CFG_DISABLED,							//   0 : I2S0_MCK	-> GPIO_AP_TO_BERMUDA_CLK_XTAL1
	CFG_FUNC0 | SLOW_SLEW,						//   1 : I2S1_MCK	-> I2S1_AP_TO_CODEC_MCLK_12M
	CFG_FUNC0 | SLOW_SLEW,						//   2 : SD_CLKOUT	-> SDIO_AP_TO_WLAN_CLK
	CFG_FUNC0 | PULL_UP | SLOW_SLEW | INPUT_SCHMITT,		//   3 : SD_CMD_IO	-> SDIO_AP_BI_WLAN_CMD
	CFG_FUNC0 | PULL_UP | SLOW_SLEW | INPUT_SCHMITT,		//   4 : SD_DATA_IO[0]	-> SDIO_AP_BI_WLAN_DATA0
	CFG_FUNC0 | PULL_UP | SLOW_SLEW | INPUT_SCHMITT,		//   5 : SD_DATA_IO[1]	-> SDIO_AP_BI_WLAN_DATA1
	CFG_FUNC0 | PULL_UP | SLOW_SLEW | INPUT_SCHMITT,		//   6 : SD_DATA_IO[2]	-> SDIO_AP_BI_WLAN_DATA2
	CFG_FUNC0 | PULL_UP | SLOW_SLEW | INPUT_SCHMITT,		//   7 : SD_DATA_IO[3]	-> SDIO_AP_BI_WLAN_DATA3

/* Port  1 */
	CFG_FUNC0 | INPUT_SCHMITT,					//   8 : SDIO_IRQ	-> GPIO_WLAN_TO_SPU_HOST_WAKE_SDIO_IRQ
	CFG_FUNC0 | PULL_DOWN | INPUT_SCHMITT,				//   9 : WL_HOST_WAKE	-> GPIO_WLAN_TO_AP_IRQ
	CFG_DISABLED,
	CFG_DISABLED,
	CFG_DISABLED,
	CFG_DISABLED,
	CFG_DISABLED,
	CFG_DISABLED,

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
	CFG_DISABLED,							//  32 : GPIO[0]	-> NC_AP_GPIO0
	CFG_DISABLED,							//  33 : GPIO[1]	-> NC_AP_GPIO1
	CFG_DISABLED,							//  34 : GPIO[2]	-> GPIO_BOOT_CONFIG_0
	CFG_DISABLED,							//  35 : GPIO[3]	-> PP1V8_SW3C_SNS
	CFG_DISABLED,							//  36 : GPIO[4]	-> NC_GPIO_BOOT_CONFIG_2
	CFG_DISABLED,							//  37 : GPIO[5]	-> NC_GPIO_BOOT_CONFIG_3
	CFG_DISABLED,							//  38 : GPIO[6]	-> GPIO_BOARD_ID_3
	CFG_DISABLED,							//  39 : GPIO[7]	-> GPIO_BOARD_REV_0

/* Port  5 */
	CFG_DISABLED,							//  40 : GPIO[8]	-> GPIO_BOARD_REV_1
	CFG_DISABLED,							//  41 : GPIO[9]	-> GPIO_BOARD_REV_2
	CFG_DISABLED,							//  42 : GPIO[10]	-> GPIO_BOARD_REV_3
	CFG_DISABLED,							//  43 : GPIO[11]	-> NC_GPIO_DEV_BOOST_ALT_ID
	CFG_OUT_0 | SLOW_SLEW,						//  44 : GPIO[12]	-> GPIO_AP_TO_BT_DEVICE_WAKE
	CFG_IN | PULL_UP | INPUT_SCHMITT,				//  45 : GPIO[13]	-> GPIO_CODEC_TO_AP_IRQ_L
	CFG_DISABLED,							//  46 : GPIO[14]	-> NC_AP_GPIO14
	CFG_OUT_0 | SLOW_SLEW,						//  47 : GPIO[15]	-> GPIO_AP_TO_BERMUDA_FW_DWLD_REQ

/* Port  6 */
	CFG_FUNC0 | SLOW_SLEW | INPUT_SCHMITT,				//  48 : I2C1_SDA	-> I2C1_AP_1V8_SDA
	CFG_FUNC0 | SLOW_SLEW | INPUT_SCHMITT,				//  49 : I2C1_SCL	-> I2C1_AP_1V8_SCL
	CFG_DISABLED,							//  50 : I2S0_LRCK	-> NC_I2S0_LRCLK
	CFG_DISABLED,							//  51 : I2S0_BCLK	-> NC_I2S0_BCLK
	CFG_DISABLED,							//  52 : I2S0_DOUT	-> NC_I2S0_DOUT
	CFG_DISABLED,							//  53 : I2S0_DIN	-> NC_I2S0_DIN
	CFG_FUNC0,							//  54 : I2S1_LRCK	-> I2S1_CODEC_TO_AP_WCLK
	CFG_FUNC0,							//  55 : I2S1_BCLK	-> I2S1_CODEC_TO_AP_BCLK

/* Port  7 */
	CFG_FUNC0 | SLOW_SLEW,						//  56 : I2S1_DOUT	-> I2S1_AP_TO_CODEC_DATA
	CFG_FUNC0,							//  57 : I2S1_DIN	-> I2S1_CODEC_TO_AP_DATA
	CFG_FUNC0 | SLOW_SLEW,						//  58 : UART1_TXD	-> UART1_AP_TO_BT_TXD
	CFG_FUNC0 | PULL_UP | INPUT_SCHMITT,				//  59 : UART1_RXD	-> UART1_BT_TO_AP_RXD
	CFG_FUNC0 | SLOW_SLEW,						//  60 : UART1_RTSN	-> UART1_AP_TO_BT_RTS_L
	CFG_FUNC0 | PULL_UP | INPUT_SCHMITT,				//  61 : UART1_CTSN	-> UART1_BT_TO_AP_CTS_L
	CFG_FUNC0 | SLOW_SLEW,						//  62 : UART2_TXD	-> UART2_AP_TO_BERMUDA_TXD
	CFG_FUNC0 | PULL_UP | INPUT_SCHMITT,				//  63 : UART2_RXD	-> UART2_BERMUDA_TO_AP_RXD

/* Port  8 */
	CFG_FUNC0 | SLOW_SLEW,						//  64 : UART2_RTSN	-> UART2_AP_TO_BERMUDA_RTS_L
	CFG_DISABLED | INPUT_SCHMITT,					//  65 : UART2_CTSN	-> UART2_BERMUDA_TO_AP_CTS_L
	CFG_FUNC0 | SLOW_SLEW,						//  66 : UART4_TXD	-> UART4_AP_TO_WLAN_TXD
	CFG_FUNC0 | PULL_UP | INPUT_SCHMITT,				//  67 : UART4_RXD	-> UART4_WLAN_TO_AP_RXD
	CFG_FUNC0 | SLOW_SLEW,						//  68 : UART3_RTXD	-> UART3_AP_BI_GG_SWI
	CFG_DISABLED,							//  69 : CLK32K_OUT	-> NC_CLK32K_OUT
	CFG_DISABLED,							//  70 : TST_CLKOUT	-> AP_TST_CLKOUT
	CFG_OUT_0 | SLOW_SLEW,						//  71 : GPIO[16]	-> AP_TO_SEAJAY_I2C_SDA_ENABLE

/* Port  9 */
	CFG_DISABLED,							//  72 : GPIO[17]	-> GPIO_AP_TO_BSYNC_RESET_EN_L
	CFG_DISABLED,							//  73 : GPIO[18]	-> GPIO_GRAPE_SWDCLK
	CFG_DISABLED,							//  74 : TMR32_PWM1	-> NC_TMR32_PWM1
	CFG_IN | INPUT_SCHMITT,						//  75 : TMR32_PWM2	-> DISP_BSYNC
	CFG_FUNC0,							//  76 : TMR32_PWM0	-> PMU_VDD_OK
	CFG_FUNC0 | INPUT_SCHMITT,					//  77 : DISP_TE	-> DISP_BSYNC
	CFG_DISABLED,							//  78 : GPIO[19]	-> GPIO_GRAPE_SWDIO
	CFG_FUNC0 | DRIVE_X2 | SLOW_SLEW | INPUT_SCHMITT,		//  79 : NAND_CEN[0]	-> ANC_AP_TO_NAND_CEN<0>

/* Port 10 */
	CFG_DISABLED,							//  80 : NAND_CEN[1]	-> NC_NAND_CEN1
	CFG_FUNC0 | DRIVE_X2 | SLOW_SLEW | INPUT_SCHMITT,		//  81 : NAND_IO[7]	-> ANC_AP_BI_NAND_IO<7>
	CFG_FUNC0 | DRIVE_X2 | SLOW_SLEW | INPUT_SCHMITT,		//  82 : NAND_IO[6]	-> ANC_AP_BI_NAND_IO<6>
	CFG_FUNC0 | DRIVE_X2 | SLOW_SLEW | INPUT_SCHMITT,		//  83 : NAND_IO[5]	-> ANC_AP_BI_NAND_IO<5>
	CFG_FUNC0 | DRIVE_X2 | SLOW_SLEW | INPUT_SCHMITT,		//  84 : NAND_IO[4]	-> ANC_AP_BI_NAND_IO<4>
	CFG_DISABLED,							//  85 : NAND_DQS	-> NC_NAND_DQS
	CFG_FUNC0 | DRIVE_X2 | SLOW_SLEW,				//  86 : NAND_REN	-> ANC_AP_TO_NAND_RE_L
	CFG_FUNC0 | DRIVE_X2 | SLOW_SLEW | INPUT_SCHMITT,		//  87 : NAND_IO[3]	-> ANC_AP_BI_NAND_IO<3>

/* Port 11 */
	CFG_FUNC0 | DRIVE_X2 | SLOW_SLEW | INPUT_SCHMITT,		//  88 : NAND_IO[2]	-> ANC_AP_BI_NAND_IO<2>
	CFG_FUNC0 | DRIVE_X2 | SLOW_SLEW | INPUT_SCHMITT,		//  89 : NAND_IO[1]	-> ANC_AP_BI_NAND_IO<1>
	CFG_FUNC0 | DRIVE_X2 | SLOW_SLEW | INPUT_SCHMITT,		//  90 : NAND_IO[0]	-> ANC_AP_BI_NAND_IO<0>
	CFG_FUNC0 | DRIVE_X2 | SLOW_SLEW,				//  91 : NAND_WEN	-> ANC_AP_TO_NAND_WE_L
	CFG_FUNC0 | DRIVE_X2 | SLOW_SLEW,				//  92 : NAND_CLE	-> ANC_AP_TO_NAND_CLE
	CFG_FUNC0 | DRIVE_X2 | SLOW_SLEW,				//  93 : NAND_ALE	-> ANC_AP_TO_NAND_ALE
	CFG_FUNC0 | SLOW_SLEW | INPUT_SCHMITT,				//  94 : I2C0_SDA	-> I2C0_AP_1V8_SDA
	CFG_FUNC0 | SLOW_SLEW | INPUT_SCHMITT,				//  95 : I2C0_SCL	-> I2C0_AP_1V8_SCL

/* Port 12 */
	CFG_DISABLED,							//  96 : DISP_VSYNC	-> NC_DISP_VSYNC
	CFG_DISABLED,							//  97 : SPI0_SCLK	-> NC_GPIO_BOARD_ID_0
	CFG_DISABLED,							//  98 : SPI0_MOSI	-> GPIO_BOARD_ID_1
	CFG_DISABLED,							//  99 : SPI0_MISO	-> GPIO_BOARD_ID_2
	CFG_DISABLED,							// 100 : SPI0_SSIN	-> NC_SPI0_MCK
	CFG_FUNC0 | SLOW_SLEW,						// 101 : UART0_TXD	-> UART0_TXD
	CFG_FUNC0 | PULL_DOWN | INPUT_SCHMITT,				// 102 : UART0_RXD	-> UART0_RXD
	CFG_DISABLED,							// 103 : SPI1_SCLK	-> NC_SPI1_SCLK

/* Port 13 */
	CFG_DISABLED,							// 104 : SPI1_MOSI	-> NC_SPI1_MOSI
	CFG_DISABLED,							// 105 : SPI1_MISO	-> NC_SPI1_MISO
	CFG_DISABLED,							// 106 : SPI1_SSIN	-> NC_SPI1_SSIN
	CFG_DISABLED,							// 107 : GPIO[20]	-> NC_AP_GPIO20
	CFG_OUT_0 | SLOW_SLEW,						// 108 : GPIO[21]	-> GPIO_AP_TO_WLAN_DEVICE_WAKE
	CFG_IN | INPUT_SCHMITT,						// 109 : GPIO[22]	-> GPIO_TRISTAR_INT
	CFG_DISABLED,							// 110 : GPIO[23]	-> GPIO_AP_TO_OPAL_LVBOOST_BUSY_L
	CFG_IN,								// 111 : GPIO[24]	-> GPIO_OPAL_SWDIO

/* Port 14 */
	CFG_IN,								// 112 : GPIO[25]	-> GPIO_OPAL_SWDCLK
	CFG_DISABLED,							// 113 : UNSPECIFIED	-> UNSPECIFIED
	CFG_DISABLED,							// 114 : UNSPECIFIED	-> UNSPECIFIED
	CFG_DISABLED,							// 115 : UNSPECIFIED	-> UNSPECIFIED
	CFG_DISABLED,							// 116 : UNSPECIFIED	-> UNSPECIFIED
	CFG_DISABLED,							// 117 : UNSPECIFIED	-> UNSPECIFIED
	CFG_DISABLED,							// 118 : UNSPECIFIED	-> UNSPECIFIED
	CFG_DISABLED,							// 119 : UNSPECIFIED	-> UNSPECIFIED

/* Port 15 */
	CFG_DISABLED,							// 120 : UNSPECIFIED	-> UNSPECIFIED
	CFG_DISABLED,							// 121 : UNSPECIFIED	-> UNSPECIFIED
	CFG_DISABLED,							// 122 : UNSPECIFIED	-> UNSPECIFIED
	CFG_DISABLED,							// 123 : UNSPECIFIED	-> UNSPECIFIED
	CFG_DISABLED,							// 124 : UNSPECIFIED	-> UNSPECIFIED
	CFG_DISABLED,							// 125 : UNSPECIFIED	-> UNSPECIFIED
	CFG_DISABLED,							// 126 : UNSPECIFIED	-> UNSPECIFIED
	CFG_DISABLED,							// 127 : UNSPECIFIED	-> UNSPECIFIED

/* Port 16 */
	CFG_DISABLED,							// 128 : ENET_MDC	-> NC_ENET_MDC
	CFG_DISABLED,							// 129 : ENET_MDIO	-> NC_ENET_MDIO
	CFG_DISABLED,							// 130 : RMII_RXER	-> GPIO_AP_TO_ORB_DEBUG
	CFG_DISABLED,							// 131 : RMII_TXEN	-> NC_RMII_TXEN
	CFG_DISABLED,							// 132 : RMII_CLK	-> NC_GPIO_DEV_DISPLAY_TO_AP_ID3
	CFG_DISABLED,							// 133 : RMII_TXD[0]	-> NC_RMII_TXD0
	CFG_DISABLED,							// 134 : RMII_TXD[1]	-> NC_FPGA_WL_REG_ON
	CFG_DISABLED,							// 135 : RMII_RXD[0]	-> NC_GPIO_DEV_DISPLAY_TO_AP_ID1

/* Port 17 */
	CFG_DISABLED,							// 136 : RMII_RXD[1]	-> NC_GPIO_DEV_DISPLAY_TO_AP_ID2
	CFG_DISABLED,							// 137 : RMII_CRSDV	-> NC_GPIO_DEV_DISPLAY_TO_AP_ID0
	CFG_DISABLED,
	CFG_DISABLED,
	CFG_DISABLED,
	CFG_DISABLED,
	CFG_DISABLED,
	CFG_DISABLED,
};

static const uint32_t pinconfig_evt_ap_1[GPIO_1_GROUP_COUNT * GPIOPADPINS] = {

/* Port  0 */
	CFG_IN | INPUT_SCHMITT,						//   0 : REQUEST_DFU1		-> BUTTON1_BUFF_L
	CFG_IN | INPUT_SCHMITT,						//   1 : REQUEST_DFU2		-> LISA_BTN_BUFF_L
	CFG_FUNC0 | INPUT_SCHMITT,					//   2 : DOCK_CONNECT		-> DOCK_CONNECT
	CFG_DISABLED,							//   3 : UNSPECIFIED		-> UNSPECIFIED
	CFG_OUT_0 | SLOW_SLEW,						//   4 : SPU_UART0_TXD		-> GPIO_SPU_TO_WLAN_CONTEXT_B
	CFG_OUT_0 | SLOW_SLEW,						//   5 : SPU_UART0_RXD		-> GPIO_SPU_TO_WLAN_CONTEXT_A
	CFG_DISABLED,							//   6 : SPU_UART0_RTSN		-> NC_SPU_UART0_RTSN
	CFG_DISABLED,							//   7 : SPU_UART0_CTSN		-> NC_SPU_UART0_CTSN

/* Port  1 */
	CFG_DISABLED,							//   8 : SPU_UART1_TXD		-> NC_SPU_UART1_TXD
	CFG_DISABLED,							//   9 : SPU_UART1_RXD		-> NC_SPU_UART1_RXD
	CFG_DISABLED,							//  10 : SPU_UART1_RTSN		-> NC_SPU_UART1_RTSN
	CFG_DISABLED,							//  11 : SPU_UART1_CTSN		-> NC_SPU_UART1_CTSN
	CFG_DISABLED,							//  12 : SPU_GPIO[0]		-> NC_SPU_GPIO0
	CFG_IN | PULL_DOWN | INPUT_SCHMITT,				//  13 : SPU_GPIO[1]		-> GPIO_WLAN_TO_SPU_HOST_WAKE_SDIO_IRQ
	CFG_IN | INPUT_SCHMITT,						//  14 : SPU_GPIO[2]		-> GPIO_BT_TO_SPU_HOST_WAKE
	CFG_IN | PULL_UP | INPUT_SCHMITT,				//  15 : SPU_GPIO[3]		-> GPIO_SUNFISH_TO_SPU_INT_L

/* Port  2 */
	CFG_IN | PULL_UP | INPUT_SCHMITT,				//  16 : SPU_GPIO[4]		-> GPIO_PMU_TO_SPU_IRQ_L
	CFG_OUT_0 | SLOW_SLEW,						//  17 : SPU_GPIO[5]		-> GPIO_AP_TO_SEAJAY_RESET
	CFG_OUT_0 | SLOW_SLEW,						//  18 : SPU_GPIO[6]		-> GPIO_SPU_TO_LVBOOST_EN
	CFG_IN | PULL_DOWN | INPUT_SCHMITT,				//  19 : SPU_GPIO[7]		-> GPIO_BERMUDA_TO_SPU_HOST_WAKE
	CFG_DISABLED,							//  20 : SPU_GPIO[8]		-> NC_SPU_GPIO8
	CFG_OUT_0,							//  21 : DFU_STATUS		-> TP_DFU_STATUS
	CFG_IN | PULL_DOWN,						//  22 : FORCE_DFU		-> TP_FORCE_DFU
	CFG_FUNC0 | INPUT_SCHMITT,					//  23 : PWR_GOOD		-> PMU_TO_SPU_PWR_GOOD

/* Port  3 */
	CFG_FUNC0 | SLOW_SLEW,						//  24 : SOC_VDD_HI_LO		-> SPU_TO_PMU_VDD_HI_LO
	CFG_DISABLED,							//  25 : UNSPECIFIED		-> UNSPECIFIED
	CFG_DISABLED,							//  26 : DOCK_ATTENTION		-> NC_DOCK_ATTENTION
	CFG_DISABLED,							//  27 : PMU_HOST_WAKE		-> NC_PMU_HOST_WAKE
	CFG_FUNC0 | DRIVE_X2 | SLOW_SLEW,				//  28 : SPU_SPI_SCLK		-> SPI_SPU_SCLK
	CFG_FUNC0 | DRIVE_X2 | SLOW_SLEW,				//  29 : SPU_SPI_MOSI		-> SPI_SPU_MOSI
	CFG_FUNC0 | PULL_DOWN,						//  30 : SPU_SPI_MISO		-> SPI_SPU_MISO
	CFG_FUNC0 | SLOW_SLEW,						//  31 : SPU_SPI_CS_TRIG[0]	-> SCM_SPU_TO_GRAPE_CS_L

/* Port  4 */
	CFG_IN | PULL_UP | INPUT_SCHMITT,				//  32 : SPU_SPI_CS_TRIG[1]	-> SCM_GRAPE_TO_SPU_HINT_L
	CFG_IN | PULL_UP | INPUT_SCHMITT,				//  33 : SPU_SPI_CS_TRIG[2]	-> SCM_OSMIUM_TO_SPU_INT_L
	CFG_IN | PULL_UP | INPUT_SCHMITT,				//  34 : SPU_SPI_CS_TRIG[3]	-> SCM_LISA_TO_SPU_INT_L
	CFG_FUNC0 | SLOW_SLEW,						//  35 : SPU_SPI_CS_TRIG[4]	-> SCM_SPU_TO_CARBON_CS_L
	CFG_IN | PULL_DOWN | INPUT_SCHMITT,				//  36 : SPU_SPI_CS_TRIG[5]	-> SCM_CARBON_TO_SPU_INT1
	CFG_IN | PULL_DOWN | INPUT_SCHMITT,				//  37 : SPU_SPI_CS_TRIG[6]	-> SCM_CARBON_TO_SPU_INT2
	CFG_DISABLED,							//  38 : SPU_SPI_CS_TRIG[7]	-> NC_SCM_SPU_TO_COMPASS_CS_L
	CFG_DISABLED,							//  39 : SPU_SPI_CS_TRIG[8]	-> NC_SCM_COMPASS_TO_SPU_INT

/* Port  5 */
	CFG_DISABLED,							//  40 : SPU_SPI_CS_TRIG[9]	-> NC_SPU_SPI_CS_TRIG9
	CFG_FUNC0 | SLOW_SLEW,						//  41 : SPU_SPI_CS_TRIG[10]	-> SCM_SPU_TO_OPAL_CS_L
	CFG_DISABLED,							//  42 : SPU_SPI_CS_TRIG[11]	-> NC_SPU_SPI_CS_TRIG11
	CFG_IN | PULL_UP | INPUT_SCHMITT,				//  43 : SPU_SPI_CS_TRIG[12]	-> SCM_PT_TO_SPU_INT_L
	CFG_DISABLED,							//  44 : SPU_SPI_CS_TRIG[13]	-> NC_SPU_SPI_CS_TRIG13
	CFG_DISABLED,							//  45 : SPU_SPI_CS_TRIG[14]	-> NC_SPU_SPI_CS_TRIG14
	CFG_DISABLED,							//  46 : SPU_SPI_CS_TRIG[15]	-> NC_SPU_SPI_CS_TRIG15
	CFG_IN | PULL_UP,						//  47 : SPU_GPIO[9]		-> GPIO_SEAJAY_TO_SPU_PING_DET_L

/* Port  6 */
	CFG_FUNC0 | SLOW_SLEW | INPUT_SCHMITT,				//  48 : SPU_I2C_SDA		-> I2C_SPU_1V8_SDA
	CFG_FUNC0 | SLOW_SLEW | INPUT_SCHMITT,				//  49 : SPU_I2C_SCL		-> I2C_SPU_1V8_SCL
	CFG_DISABLED,
	CFG_DISABLED,
	CFG_DISABLED,
	CFG_DISABLED,
	CFG_DISABLED,
	CFG_DISABLED,
};

static const uint32_t pinconfig_evt_dev_0[GPIO_GROUP_COUNT * GPIOPADPINS] = {

/* Port  0 */
	CFG_DISABLED,							//   0 : I2S0_MCK	-> GPIO_AP_TO_BERMUDA_CLK_XTAL1
	CFG_FUNC0 | DRIVE_X2 | SLOW_SLEW,				//   1 : I2S1_MCK	-> I2S1_AP_TO_CODEC_MCLK_12M
	CFG_FUNC0 | DRIVE_X2 | SLOW_SLEW,				//   2 : SD_CLKOUT	-> SDIO_AP_TO_WLAN_CLK
	CFG_FUNC0 | PULL_UP | DRIVE_X2 | SLOW_SLEW | INPUT_SCHMITT,	//   3 : SD_CMD_IO	-> SDIO_AP_BI_WLAN_CMD
	CFG_FUNC0 | PULL_UP | DRIVE_X2 | SLOW_SLEW | INPUT_SCHMITT,	//   4 : SD_DATA_IO[0]	-> SDIO_AP_BI_WLAN_DATA0
	CFG_FUNC0 | PULL_UP | DRIVE_X2 | SLOW_SLEW | INPUT_SCHMITT,	//   5 : SD_DATA_IO[1]	-> SDIO_AP_BI_WLAN_DATA1
	CFG_FUNC0 | PULL_UP | DRIVE_X2 | SLOW_SLEW | INPUT_SCHMITT,	//   6 : SD_DATA_IO[2]	-> SDIO_AP_BI_WLAN_DATA2
	CFG_FUNC0 | PULL_UP | DRIVE_X2 | SLOW_SLEW | INPUT_SCHMITT,	//   7 : SD_DATA_IO[3]	-> SDIO_AP_BI_WLAN_DATA3

/* Port  1 */
	CFG_FUNC0 | INPUT_SCHMITT,					//   8 : SDIO_IRQ	-> GPIO_WLAN_TO_SPU_HOST_WAKE_SDIO_IRQ
	CFG_FUNC0 | PULL_DOWN | INPUT_SCHMITT,				//   9 : WL_HOST_WAKE	-> GPIO_WLAN_TO_AP_IRQ
	CFG_DISABLED,
	CFG_DISABLED,
	CFG_DISABLED,
	CFG_DISABLED,
	CFG_DISABLED,
	CFG_DISABLED,

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
	CFG_DISABLED,							//  32 : GPIO[0]	-> NC_AP_GPIO0
	CFG_DISABLED,							//  33 : GPIO[1]	-> NC_AP_GPIO1
	CFG_DISABLED,							//  34 : GPIO[2]	-> GPIO_BOOT_CONFIG_0
	CFG_DISABLED,							//  35 : GPIO[3]	-> PP1V8_SW3C_SNS
	CFG_DISABLED,							//  36 : GPIO[4]	-> NC_GPIO_BOOT_CONFIG_2
	CFG_DISABLED,							//  37 : GPIO[5]	-> NC_GPIO_BOOT_CONFIG_3
	CFG_DISABLED,							//  38 : GPIO[6]	-> GPIO_BOARD_ID_3
	CFG_DISABLED,							//  39 : GPIO[7]	-> GPIO_BOARD_REV_0

/* Port  5 */
	CFG_DISABLED,							//  40 : GPIO[8]	-> GPIO_BOARD_REV_1
	CFG_DISABLED,							//  41 : GPIO[9]	-> GPIO_BOARD_REV_2
	CFG_DISABLED,							//  42 : GPIO[10]	-> GPIO_BOARD_REV_3
	CFG_DISABLED,							//  43 : GPIO[11]	-> NC_GPIO_DEV_BOOST_ALT_ID
	CFG_OUT_0 | DRIVE_X2 | SLOW_SLEW,				//  44 : GPIO[12]	-> GPIO_AP_TO_BT_DEVICE_WAKE
	CFG_IN | PULL_UP | INPUT_SCHMITT,				//  45 : GPIO[13]	-> GPIO_CODEC_TO_AP_IRQ_L
	CFG_DISABLED,							//  46 : GPIO[14]	-> NC_AP_GPIO14
	CFG_OUT_0 | DRIVE_X2 | SLOW_SLEW,				//  47 : GPIO[15]	-> GPIO_AP_TO_BERMUDA_FW_DWLD_REQ

/* Port  6 */
	CFG_FUNC0 | DRIVE_X2 | SLOW_SLEW | INPUT_SCHMITT,		//  48 : I2C1_SDA	-> I2C1_AP_1V8_SDA
	CFG_FUNC0 | DRIVE_X2 | SLOW_SLEW | INPUT_SCHMITT,		//  49 : I2C1_SCL	-> I2C1_AP_1V8_SCL
	CFG_DISABLED,							//  50 : I2S0_LRCK	-> NC_I2S0_LRCLK
	CFG_DISABLED,							//  51 : I2S0_BCLK	-> NC_I2S0_BCLK
	CFG_DISABLED,							//  52 : I2S0_DOUT	-> NC_I2S0_DOUT
	CFG_DISABLED,							//  53 : I2S0_DIN	-> NC_I2S0_DIN
	CFG_FUNC0,							//  54 : I2S1_LRCK	-> I2S1_CODEC_TO_AP_WCLK
	CFG_FUNC0,							//  55 : I2S1_BCLK	-> I2S1_CODEC_TO_AP_BCLK

/* Port  7 */
	CFG_FUNC0 | DRIVE_X2 | SLOW_SLEW,				//  56 : I2S1_DOUT	-> I2S1_AP_TO_CODEC_DATA
	CFG_FUNC0,							//  57 : I2S1_DIN	-> I2S1_CODEC_TO_AP_DATA
	CFG_FUNC0 | DRIVE_X2 | SLOW_SLEW,				//  58 : UART1_TXD	-> UART1_AP_TO_BT_TXD
	CFG_FUNC0 | PULL_UP | INPUT_SCHMITT,				//  59 : UART1_RXD	-> UART1_BT_TO_AP_RXD
	CFG_FUNC0 | DRIVE_X2 | SLOW_SLEW,				//  60 : UART1_RTSN	-> UART1_AP_TO_BT_RTS_L
	CFG_FUNC0 | PULL_UP | INPUT_SCHMITT,				//  61 : UART1_CTSN	-> UART1_BT_TO_AP_CTS_L
	CFG_FUNC0 | DRIVE_X2 | SLOW_SLEW,				//  62 : UART2_TXD	-> UART2_AP_TO_BERMUDA_TXD
	CFG_FUNC0 | PULL_UP | INPUT_SCHMITT,				//  63 : UART2_RXD	-> UART2_BERMUDA_TO_AP_RXD

/* Port  8 */
	CFG_FUNC0 | DRIVE_X2 | SLOW_SLEW,				//  64 : UART2_RTSN	-> UART2_AP_TO_BERMUDA_RTS_L
	CFG_DISABLED | INPUT_SCHMITT,					//  65 : UART2_CTSN	-> UART2_BERMUDA_TO_AP_CTS_L
	CFG_FUNC0 | DRIVE_X2 | SLOW_SLEW,				//  66 : UART4_TXD	-> UART4_AP_TO_WLAN_TXD
	CFG_FUNC0 | PULL_UP | INPUT_SCHMITT,				//  67 : UART4_RXD	-> UART4_WLAN_TO_AP_RXD
	CFG_FUNC0 | DRIVE_X2 | SLOW_SLEW,				//  68 : UART3_RTXD	-> UART3_AP_BI_GG_SWI
	CFG_DISABLED,							//  69 : CLK32K_OUT	-> NC_CLK32K_OUT
	CFG_DISABLED,							//  70 : TST_CLKOUT	-> AP_TST_CLKOUT
	CFG_OUT_0 | DRIVE_X2 | SLOW_SLEW,				//  71 : GPIO[16]	-> AP_TO_SEAJAY_I2C_SDA_ENABLE

/* Port  9 */
	CFG_DISABLED,							//  72 : GPIO[17]	-> GPIO_AP_TO_BSYNC_RESET_EN_L
	CFG_DISABLED,							//  73 : GPIO[18]	-> GPIO_GRAPE_SWDCLK
	CFG_DISABLED,							//  74 : TMR32_PWM1	-> NC_TMR32_PWM1
	CFG_IN | INPUT_SCHMITT,						//  75 : TMR32_PWM2	-> DISP_BSYNC
	CFG_FUNC0,							//  76 : TMR32_PWM0	-> PMU_VDD_OK
	CFG_FUNC0 | INPUT_SCHMITT,					//  77 : DISP_TE	-> DISP_BSYNC
	CFG_DISABLED,							//  78 : GPIO[19]	-> GPIO_GRAPE_SWDIO
	CFG_FUNC0 | DRIVE_X2 | SLOW_SLEW | INPUT_SCHMITT,		//  79 : NAND_CEN[0]	-> ANC_AP_TO_NAND_CEN<0>

/* Port 10 */
	CFG_DISABLED,							//  80 : NAND_CEN[1]	-> NC_NAND_CEN1
	CFG_FUNC0 | DRIVE_X2 | SLOW_SLEW | INPUT_SCHMITT,		//  81 : NAND_IO[7]	-> ANC_AP_BI_NAND_IO<7>
	CFG_FUNC0 | DRIVE_X2 | SLOW_SLEW | INPUT_SCHMITT,		//  82 : NAND_IO[6]	-> ANC_AP_BI_NAND_IO<6>
	CFG_FUNC0 | DRIVE_X2 | SLOW_SLEW | INPUT_SCHMITT,		//  83 : NAND_IO[5]	-> ANC_AP_BI_NAND_IO<5>
	CFG_FUNC0 | DRIVE_X2 | SLOW_SLEW | INPUT_SCHMITT,		//  84 : NAND_IO[4]	-> ANC_AP_BI_NAND_IO<4>
	CFG_DISABLED,							//  85 : NAND_DQS	-> NC_NAND_DQS
	CFG_FUNC0 | DRIVE_X2 | SLOW_SLEW,				//  86 : NAND_REN	-> ANC_AP_TO_NAND_RE_L
	CFG_FUNC0 | DRIVE_X2 | SLOW_SLEW | INPUT_SCHMITT,		//  87 : NAND_IO[3]	-> ANC_AP_BI_NAND_IO<3>

/* Port 11 */
	CFG_FUNC0 | DRIVE_X2 | SLOW_SLEW | INPUT_SCHMITT,		//  88 : NAND_IO[2]	-> ANC_AP_BI_NAND_IO<2>
	CFG_FUNC0 | DRIVE_X2 | SLOW_SLEW | INPUT_SCHMITT,		//  89 : NAND_IO[1]	-> ANC_AP_BI_NAND_IO<1>
	CFG_FUNC0 | DRIVE_X2 | SLOW_SLEW | INPUT_SCHMITT,		//  90 : NAND_IO[0]	-> ANC_AP_BI_NAND_IO<0>
	CFG_FUNC0 | DRIVE_X2 | SLOW_SLEW,				//  91 : NAND_WEN	-> ANC_AP_TO_NAND_WE_L
	CFG_FUNC0 | DRIVE_X2 | SLOW_SLEW,				//  92 : NAND_CLE	-> ANC_AP_TO_NAND_CLE
	CFG_FUNC0 | DRIVE_X2 | SLOW_SLEW,				//  93 : NAND_ALE	-> ANC_AP_TO_NAND_ALE
	CFG_FUNC0 | DRIVE_X2 | SLOW_SLEW | INPUT_SCHMITT,		//  94 : I2C0_SDA	-> I2C0_AP_1V8_SDA
	CFG_FUNC0 | DRIVE_X2 | SLOW_SLEW | INPUT_SCHMITT,		//  95 : I2C0_SCL	-> I2C0_AP_1V8_SCL

/* Port 12 */
	CFG_DISABLED,							//  96 : DISP_VSYNC	-> NC_DISP_VSYNC
	CFG_DISABLED,							//  97 : SPI0_SCLK	-> NC_GPIO_BOARD_ID_0
	CFG_DISABLED,							//  98 : SPI0_MOSI	-> GPIO_BOARD_ID_1
	CFG_DISABLED,							//  99 : SPI0_MISO	-> GPIO_BOARD_ID_2
	CFG_DISABLED,							// 100 : SPI0_SSIN	-> NC_SPI0_MCK
	CFG_FUNC0 | DRIVE_X2 | SLOW_SLEW,				// 101 : UART0_TXD	-> UART0_TXD
	CFG_FUNC0 | PULL_DOWN | INPUT_SCHMITT,				// 102 : UART0_RXD	-> UART0_RXD
	CFG_DISABLED,							// 103 : SPI1_SCLK	-> NC_SPI1_SCLK

/* Port 13 */
	CFG_DISABLED,							// 104 : SPI1_MOSI	-> NC_SPI1_MOSI
	CFG_DISABLED,							// 105 : SPI1_MISO	-> NC_SPI1_MISO
	CFG_DISABLED,							// 106 : SPI1_SSIN	-> NC_SPI1_SSIN
	CFG_DISABLED,							// 107 : GPIO[20]	-> NC_AP_GPIO20
	CFG_OUT_0 | DRIVE_X2 | SLOW_SLEW,				// 108 : GPIO[21]	-> GPIO_AP_TO_WLAN_DEVICE_WAKE
	CFG_IN | INPUT_SCHMITT,						// 109 : GPIO[22]	-> GPIO_TRISTAR_INT
	CFG_DISABLED,							// 110 : GPIO[23]	-> GPIO_AP_TO_OPAL_LVBOOST_BUSY_L
	CFG_IN,								// 111 : GPIO[24]	-> GPIO_OPAL_SWDIO

/* Port 14 */
	CFG_IN,								// 112 : GPIO[25]	-> GPIO_OPAL_SWDCLK
	CFG_DISABLED,							// 113 : UNSPECIFIED	-> UNSPECIFIED
	CFG_DISABLED,							// 114 : UNSPECIFIED	-> UNSPECIFIED
	CFG_DISABLED,							// 115 : UNSPECIFIED	-> UNSPECIFIED
	CFG_DISABLED,							// 116 : UNSPECIFIED	-> UNSPECIFIED
	CFG_DISABLED,							// 117 : UNSPECIFIED	-> UNSPECIFIED
	CFG_DISABLED,							// 118 : UNSPECIFIED	-> UNSPECIFIED
	CFG_DISABLED,							// 119 : UNSPECIFIED	-> UNSPECIFIED

/* Port 15 */
	CFG_DISABLED,							// 120 : UNSPECIFIED	-> UNSPECIFIED
	CFG_DISABLED,							// 121 : UNSPECIFIED	-> UNSPECIFIED
	CFG_DISABLED,							// 122 : UNSPECIFIED	-> UNSPECIFIED
	CFG_DISABLED,							// 123 : UNSPECIFIED	-> UNSPECIFIED
	CFG_DISABLED,							// 124 : UNSPECIFIED	-> UNSPECIFIED
	CFG_DISABLED,							// 125 : UNSPECIFIED	-> UNSPECIFIED
	CFG_DISABLED,							// 126 : UNSPECIFIED	-> UNSPECIFIED
	CFG_DISABLED,							// 127 : UNSPECIFIED	-> UNSPECIFIED

/* Port 16 */
	CFG_DISABLED,							// 128 : ENET_MDC	-> NC_ENET_MDC
	CFG_DISABLED,							// 129 : ENET_MDIO	-> NC_ENET_MDIO
	CFG_DISABLED,							// 130 : RMII_RXER	-> GPIO_AP_TO_ORB_DEBUG
	CFG_DISABLED,							// 131 : RMII_TXEN	-> NC_RMII_TXEN
	CFG_DISABLED,							// 132 : RMII_CLK	-> NC_GPIO_DEV_DISPLAY_TO_AP_ID3
	CFG_DISABLED,							// 133 : RMII_TXD[0]	-> NC_RMII_TXD0
	CFG_DISABLED,							// 134 : RMII_TXD[1]	-> NC_FPGA_WL_REG_ON
	CFG_DISABLED,							// 135 : RMII_RXD[0]	-> NC_GPIO_DEV_DISPLAY_TO_AP_ID1

/* Port 17 */
	CFG_DISABLED,							// 136 : RMII_RXD[1]	-> NC_GPIO_DEV_DISPLAY_TO_AP_ID2
	CFG_DISABLED,							// 137 : RMII_CRSDV	-> NC_GPIO_DEV_DISPLAY_TO_AP_ID0
	CFG_DISABLED,
	CFG_DISABLED,
	CFG_DISABLED,
	CFG_DISABLED,
	CFG_DISABLED,
	CFG_DISABLED,
};

static const uint32_t pinconfig_evt_dev_1[GPIO_1_GROUP_COUNT * GPIOPADPINS] = {

/* Port  0 */
	CFG_IN | INPUT_SCHMITT,						//   0 : REQUEST_DFU1		-> BUTTON1_BUFF_L
	CFG_IN | INPUT_SCHMITT,						//   1 : REQUEST_DFU2		-> LISA_BTN_BUFF_L
	CFG_FUNC0 | INPUT_SCHMITT,					//   2 : DOCK_CONNECT		-> DOCK_CONNECT
	CFG_DISABLED,							//   3 : UNSPECIFIED		-> UNSPECIFIED
	CFG_OUT_0 | DRIVE_X2 | SLOW_SLEW,				//   4 : SPU_UART0_TXD		-> GPIO_SPU_TO_WLAN_CONTEXT_B
	CFG_OUT_0 | DRIVE_X2 | SLOW_SLEW,				//   5 : SPU_UART0_RXD		-> GPIO_SPU_TO_WLAN_CONTEXT_A
	CFG_DISABLED,							//   6 : SPU_UART0_RTSN		-> NC_SPU_UART0_RTSN
	CFG_DISABLED,							//   7 : SPU_UART0_CTSN		-> NC_SPU_UART0_CTSN

/* Port  1 */
	CFG_DISABLED,							//   8 : SPU_UART1_TXD		-> NC_SPU_UART1_TXD
	CFG_DISABLED,							//   9 : SPU_UART1_RXD		-> NC_SPU_UART1_RXD
	CFG_DISABLED,							//  10 : SPU_UART1_RTSN		-> NC_SPU_UART1_RTSN
	CFG_DISABLED,							//  11 : SPU_UART1_CTSN		-> NC_SPU_UART1_CTSN
	CFG_DISABLED,							//  12 : SPU_GPIO[0]		-> NC_SPU_GPIO0
	CFG_IN | PULL_DOWN | INPUT_SCHMITT,				//  13 : SPU_GPIO[1]		-> GPIO_WLAN_TO_SPU_HOST_WAKE_SDIO_IRQ
	CFG_IN | INPUT_SCHMITT,						//  14 : SPU_GPIO[2]		-> GPIO_BT_TO_SPU_HOST_WAKE
	CFG_IN | PULL_UP | INPUT_SCHMITT,				//  15 : SPU_GPIO[3]		-> GPIO_SUNFISH_TO_SPU_INT_L

/* Port  2 */
	CFG_IN | PULL_UP | INPUT_SCHMITT,				//  16 : SPU_GPIO[4]		-> GPIO_PMU_TO_SPU_IRQ_L
	CFG_OUT_0 | DRIVE_X2 | SLOW_SLEW,				//  17 : SPU_GPIO[5]		-> GPIO_AP_TO_SEAJAY_RESET
	CFG_OUT_0 | DRIVE_X2 | SLOW_SLEW,				//  18 : SPU_GPIO[6]		-> GPIO_SPU_TO_LVBOOST_EN
	CFG_IN | PULL_DOWN | INPUT_SCHMITT,				//  19 : SPU_GPIO[7]		-> GPIO_BERMUDA_TO_SPU_HOST_WAKE
	CFG_DISABLED,							//  20 : SPU_GPIO[8]		-> NC_SPU_GPIO8
	CFG_OUT_0,							//  21 : DFU_STATUS		-> TP_DFU_STATUS
	CFG_IN | PULL_DOWN,						//  22 : FORCE_DFU		-> TP_FORCE_DFU
	CFG_FUNC0 | INPUT_SCHMITT,					//  23 : PWR_GOOD		-> PMU_TO_SPU_PWR_GOOD

/* Port  3 */
	CFG_FUNC0 | DRIVE_X2 | SLOW_SLEW,				//  24 : SOC_VDD_HI_LO		-> SPU_TO_PMU_VDD_HI_LO
	CFG_DISABLED,							//  25 : UNSPECIFIED		-> UNSPECIFIED
	CFG_DISABLED,							//  26 : DOCK_ATTENTION		-> NC_DOCK_ATTENTION
	CFG_DISABLED,							//  27 : PMU_HOST_WAKE		-> NC_PMU_HOST_WAKE
	CFG_FUNC0 | DRIVE_X2 | SLOW_SLEW,				//  28 : SPU_SPI_SCLK		-> SPI_SPU_SCLK
	CFG_FUNC0 | DRIVE_X2 | SLOW_SLEW,				//  29 : SPU_SPI_MOSI		-> SPI_SPU_MOSI
	CFG_FUNC0 | PULL_DOWN,						//  30 : SPU_SPI_MISO		-> SPI_SPU_MISO
	CFG_FUNC0 | DRIVE_X2 | SLOW_SLEW,				//  31 : SPU_SPI_CS_TRIG[0]	-> SCM_SPU_TO_GRAPE_CS_L

/* Port  4 */
	CFG_IN | PULL_UP | INPUT_SCHMITT,				//  32 : SPU_SPI_CS_TRIG[1]	-> SCM_GRAPE_TO_SPU_HINT_L
	CFG_IN | PULL_UP | INPUT_SCHMITT,				//  33 : SPU_SPI_CS_TRIG[2]	-> SCM_OSMIUM_TO_SPU_INT_L
	CFG_IN | PULL_UP | INPUT_SCHMITT,				//  34 : SPU_SPI_CS_TRIG[3]	-> SCM_LISA_TO_SPU_INT_L
	CFG_FUNC0 | DRIVE_X2 | SLOW_SLEW,				//  35 : SPU_SPI_CS_TRIG[4]	-> SCM_SPU_TO_CARBON_CS_L
	CFG_IN | PULL_DOWN | INPUT_SCHMITT,				//  36 : SPU_SPI_CS_TRIG[5]	-> SCM_CARBON_TO_SPU_INT1
	CFG_IN | PULL_DOWN | INPUT_SCHMITT,				//  37 : SPU_SPI_CS_TRIG[6]	-> SCM_CARBON_TO_SPU_INT2
	CFG_DISABLED,							//  38 : SPU_SPI_CS_TRIG[7]	-> NC_SCM_SPU_TO_COMPASS_CS_L
	CFG_DISABLED,							//  39 : SPU_SPI_CS_TRIG[8]	-> NC_SCM_COMPASS_TO_SPU_INT

/* Port  5 */
	CFG_DISABLED,							//  40 : SPU_SPI_CS_TRIG[9]	-> NC_SPU_SPI_CS_TRIG9
	CFG_FUNC0 | DRIVE_X2 | SLOW_SLEW,				//  41 : SPU_SPI_CS_TRIG[10]	-> SCM_SPU_TO_OPAL_CS_L
	CFG_DISABLED,							//  42 : SPU_SPI_CS_TRIG[11]	-> NC_SPU_SPI_CS_TRIG11
	CFG_IN | PULL_UP | INPUT_SCHMITT,				//  43 : SPU_SPI_CS_TRIG[12]	-> SCM_PT_TO_SPU_INT_L
	CFG_DISABLED,							//  44 : SPU_SPI_CS_TRIG[13]	-> NC_SPU_SPI_CS_TRIG13
	CFG_DISABLED,							//  45 : SPU_SPI_CS_TRIG[14]	-> NC_SPU_SPI_CS_TRIG14
	CFG_DISABLED,							//  46 : SPU_SPI_CS_TRIG[15]	-> NC_SPU_SPI_CS_TRIG15
	CFG_IN | PULL_UP,						//  47 : SPU_GPIO[9]		-> GPIO_SEAJAY_TO_SPU_PING_DET_L

/* Port  6 */
	CFG_FUNC0 | DRIVE_X2 | SLOW_SLEW | INPUT_SCHMITT,		//  48 : SPU_I2C_SDA		-> I2C_SPU_1V8_SDA
	CFG_FUNC0 | DRIVE_X2 | SLOW_SLEW | INPUT_SCHMITT,		//  49 : SPU_I2C_SCL		-> I2C_SPU_1V8_SCL
	CFG_DISABLED,
	CFG_DISABLED,
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
	{ 0, 1, { pinconfig_evt_ap_0, pinconfig_evt_ap_1 } },
	{ 1, 1, { pinconfig_evt_dev_0, pinconfig_evt_dev_1 } },
};

const uint32_t * target_get_evt_gpio_cfg(int gpioc)
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
