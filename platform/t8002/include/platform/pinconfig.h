/*
 * Copyright (C) 2015 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */

/* THIS FILE IS AUTOMATICALLY GENERATED BY tools/csvtopinconfig.py. DO NOT EDIT!
   I/O Spreadsheet version: m8 io list. version 148
   I/O Spreadsheet tracker: 20260553
   Conversion command: csvtopinconfig.py --rom --header --soc m8 --config-column 'SecureROM config' --pupd-column PU/PD --copyright 2015 --radar 20260553 --netname-column 'ROM Config' <filename>
*/

#ifndef __PLATFORM_PINCONFIG_H
#define __PLATFORM_PINCONFIG_H

static const uint32_t gpio_default_cfg_0[GPIO_GROUP_COUNT * GPIOPADPINS] = {

/* Port  0 */
	CFG_DISABLED,								//   0 : GPIO[0]	->
	CFG_DISABLED,								//   1 : GPIO[1]	->
	CFG_DISABLED | DRIVE_S4 | VERY_FAST_SLEW,				//   2 : GPIO[2]	-> Boot_config[0]
	CFG_DISABLED | DRIVE_S4 | VERY_FAST_SLEW,				//   3 : GPIO[3]	-> Boot_config[1]
	CFG_DISABLED | DRIVE_S4 | VERY_FAST_SLEW,				//   4 : GPIO[4]	-> Boot_config[2]
	CFG_DISABLED | DRIVE_S4 | VERY_FAST_SLEW,				//   5 : GPIO[5]	-> Boot_config[3]
	CFG_DISABLED | DRIVE_S4 | VERY_FAST_SLEW,				//   6 : GPIO[6]	-> Board_ID[3]
	CFG_DISABLED,								//   7 : GPIO[7]	->

/* Port  1 */
	CFG_DISABLED,								//   8 : GPIO[8]	->
	CFG_DISABLED,								//   9 : GPIO[9]	->
	CFG_DISABLED,								//  10 : GPIO[10]	->
	CFG_DISABLED,								//  11 : GPIO[11]	->
	CFG_DISABLED,								//  12 : GPIO[12]	->
	CFG_DISABLED,								//  13 : GPIO[13]	->
	CFG_DISABLED,								//  14 : GPIO[14]	->
	CFG_DISABLED,								//  15 : GPIO[15]	->

/* Port  2 */
	CFG_DISABLED,								//  16 : GPIO[16]	->
	CFG_DISABLED,								//  17 : GPIO[17]	->
	CFG_DISABLED,								//  18 : TMR32_PWM1	->
	CFG_DISABLED,								//  19 : TMR32_PWM2	->
	CFG_DISABLED,								//  20 : TMR32_PWM0	->
	CFG_DISABLED | DRIVE_S4 | SLOW_SLEW,					//  21 : NAND_CEN[0]	->
	CFG_DISABLED | DRIVE_S4 | SLOW_SLEW,					//  22 : NAND_CEN[1]	->
	CFG_DISABLED | PULL_DOWN | DRIVE_S4 | SLOW_SLEW | INPUT_SCHMITT,	//  23 : NAND_IO[7]	->

/* Port  3 */
	CFG_DISABLED | PULL_DOWN | DRIVE_S4 | SLOW_SLEW | INPUT_SCHMITT,	//  24 : NAND_IO[6]	->
	CFG_DISABLED | PULL_DOWN | DRIVE_S4 | SLOW_SLEW | INPUT_SCHMITT,	//  25 : NAND_IO[5]	->
	CFG_DISABLED | PULL_DOWN | DRIVE_S4 | SLOW_SLEW | INPUT_SCHMITT,	//  26 : NAND_IO[4]	->
	CFG_DISABLED | PULL_UP | DRIVE_S4 | SLOW_SLEW,				//  27 : NAND_REN	->
	CFG_DISABLED | PULL_DOWN | DRIVE_S4 | SLOW_SLEW | INPUT_SCHMITT,	//  28 : NAND_IO[3]	->
	CFG_DISABLED | PULL_DOWN | DRIVE_S4 | SLOW_SLEW | INPUT_SCHMITT,	//  29 : NAND_IO[2]	->
	CFG_DISABLED | PULL_DOWN | DRIVE_S4 | SLOW_SLEW | INPUT_SCHMITT,	//  30 : NAND_IO[1]	->
	CFG_DISABLED | PULL_DOWN | DRIVE_S4 | SLOW_SLEW | INPUT_SCHMITT,	//  31 : NAND_IO[0]	->

/* Port  4 */
	CFG_DISABLED | PULL_UP | DRIVE_S4 | SLOW_SLEW,				//  32 : NAND_WEN	->
	CFG_DISABLED | PULL_DOWN | DRIVE_S4 | SLOW_SLEW,			//  33 : NAND_CLE	->
	CFG_DISABLED | PULL_DOWN | DRIVE_S4 | SLOW_SLEW,			//  34 : NAND_ALE	->
	CFG_DISABLED,								//  35 : I2S0_LRCK	->
	CFG_DISABLED,								//  36 : I2S0_BCLK	->
	CFG_DISABLED,								//  37 : I2S0_DOUT	->
	CFG_DISABLED,								//  38 : I2S0_DIN	->
	CFG_DISABLED,								//  39 : I2S1_LRCK	->

/* Port  5 */
	CFG_DISABLED,								//  40 : I2S1_BCLK	->
	CFG_DISABLED,								//  41 : I2S1_DOUT	->
	CFG_DISABLED,								//  42 : I2S1_DIN	->
	CFG_DISABLED,								//  43 : I2S2_LRCK	->
	CFG_DISABLED,								//  44 : I2S2_BCLK	->
	CFG_DISABLED,								//  45 : I2S2_DOUT	->
	CFG_DISABLED,								//  46 : I2S2_DIN	->
	CFG_DISABLED,								//  47 : SD_CLKOUT	->

/* Port  6 */
	CFG_DISABLED,								//  48 : sDIO_IRQ	->
	CFG_DISABLED,								//  49 : SD_DATA_IO[0]	->
	CFG_DISABLED,								//  50 : SD_DATA_IO[1]	->
	CFG_DISABLED,								//  51 : SD_DATA_IO[2]	->
	CFG_DISABLED,								//  52 : SD_DATA_IO[3]	->
	CFG_DISABLED,								//  53 : SDIO_IRQ	->
	CFG_DISABLED,								//  54 : WL_HOST_WAKE	->
	CFG_DISABLED,								//  55 : ENET_MDC	->

/* Port  7 */
	CFG_DISABLED,								//  56 : ENET_MDIO	->
	CFG_DISABLED,								//  57 : RMII_RXER	->
	CFG_DISABLED,								//  58 : RMII_TXEN	->
	CFG_DISABLED,								//  59 : RMII_CLK	->
	CFG_DISABLED,								//  60 : RMII_TXD[0]	->
	CFG_DISABLED,								//  61 : RMII_TXD[1]	->
	CFG_DISABLED,								//  62 : RMII_RXD[0]	->
	CFG_DISABLED,								//  63 : RMII_RXD[1]	->

/* Port  8 */
	CFG_DISABLED,								//  64 : RMII_CRSDV	->
	CFG_DISABLED | DRIVE_S4 | VERY_FAST_SLEW,				//  65 : SPI0_SCLK	-> SPI0_SCLK/Board_ID[0]
	CFG_DISABLED | DRIVE_S4 | VERY_FAST_SLEW,				//  66 : SPI0_MOSI	-> SPI0_MOSI/Board_ID[1]
	CFG_DISABLED | DRIVE_S4 | VERY_FAST_SLEW,				//  67 : SPI0_MISO	-> SPI0_MISO/Board_ID[2]
	CFG_DISABLED | DRIVE_S4 | VERY_FAST_SLEW,				//  68 : SPI0_SSIN	-> SPI0_CS
	CFG_DISABLED,								//  69 : SPI1_SCLK	->
	CFG_DISABLED,								//  70 : SPI1_MOSI	->
	CFG_DISABLED,								//  71 : SPI1_MISO	->

/* Port  9 */
	CFG_DISABLED,								//  72 : SPI1_SSIN	->
	CFG_DISABLED,								//  73 : I2C0_SDA	->
	CFG_DISABLED,								//  74 : I2C0_SCL	->
	CFG_DISABLED,								//  75 : I2C1_SDA	->
	CFG_DISABLED,								//  76 : I2C1_SCL	->
	CFG_DISABLED,								//  77 : I2C2_0_SDA	->
	CFG_DISABLED,								//  78 : I2C2_0_SCL	->
	CFG_DISABLED,								//  79 : I2C2_1_SDA	->

/* Port 10 */
	CFG_DISABLED,								//  80 : I2C2_1_SCL	->
	CFG_DISABLED,								//  81 : UART0_TXD	->
	CFG_DISABLED,								//  82 : UART0_RXD	->
	CFG_DISABLED,								//  83 : UART3_RTXD	->
	CFG_DISABLED,								//  84 : UART4_TXD	->
	CFG_DISABLED,								//  85 : UART4_RXD	->
	CFG_DISABLED,								//  86 : UART5_TXD	->
	CFG_DISABLED,								//  87 : UART5_RXD	->

/* Port 11 */
	CFG_DISABLED,								//  88 : UART2_TXD	->
	CFG_DISABLED,								//  89 : UART2_RXD	->
	CFG_DISABLED,								//  90 : UART2_RTSN	->
	CFG_DISABLED,								//  91 : UART2_CTSN	->
	CFG_DISABLED,								//  92 : UART1_TXD	->
	CFG_DISABLED,								//  93 : UART1_RXD	->
	CFG_DISABLED,								//  94 : UART1_RTSN	->
	CFG_DISABLED,								//  95 : UART1_CTSN	->

/* Port 12 */
	CFG_DISABLED,								//  96 : ISP0_SDA	->
	CFG_DISABLED,								//  97 : ISP0_SCL	->
	CFG_DISABLED,								//  98 : DISP_TE	->
	CFG_DISABLED,								//  99 : DISP_VSYNC	->
	CFG_IN | DRIVE_S4 | VERY_FAST_SLEW | INPUT_SCHMITT,			// 100 : REQUEST_DFU1	-> REQUEST_DFU1
	CFG_IN | DRIVE_S4 | VERY_FAST_SLEW | INPUT_SCHMITT,			// 101 : REQUEST_DFU2	-> REQUEST_DFU2
	CFG_IN | PULL_DOWN | DRIVE_S4 | VERY_FAST_SLEW | INPUT_SCHMITT,		// 102 : FORCE_DFU	-> FORCE_DFU
	CFG_DISABLED,								// 103 : SWD_TMS2	->

/* Port 13 */
	CFG_DISABLED,								// 104 : SWD_TMS3	->
	CFG_DISABLED,								// 105 : CLK32K_OUT	->
	CFG_DISABLED,
	CFG_DISABLED,
	CFG_DISABLED,
	CFG_DISABLED,
	CFG_DISABLED,
	CFG_DISABLED,

/* Port 14 */
	CFG_DISABLED,
	CFG_DISABLED,
	CFG_DISABLED,
	CFG_DISABLED,
	CFG_DISABLED,
	CFG_DISABLED,
	CFG_DISABLED,
	CFG_DISABLED,

/* Port 15 */
	CFG_DISABLED,
	CFG_DISABLED,
	CFG_DISABLED,
	CFG_DISABLED,
	CFG_DISABLED,
	CFG_DISABLED,
	CFG_DISABLED,
	CFG_DISABLED,

/* Port 16 */
	CFG_DISABLED,								// 128 : MON[0]		->
	CFG_DISABLED,								// 129 : MON[1]		->
	CFG_DISABLED,								// 130 : MON[2]		->
	CFG_DISABLED,								// 131 : MON[3]		->
	CFG_DISABLED,								// 132 : MON[4]		->
	CFG_DISABLED,								// 133 : MON[5]		->
	CFG_DISABLED,								// 134 : MON[6]		->
	CFG_DISABLED,								// 135 : MON[7]		->

/* Port 17 */
	CFG_DISABLED | PULL_DOWN | DRIVE_S4 | VERY_FAST_SLEW,			// 136 : DFU_STATUS	-> DFU_STATUS
	CFG_DISABLED,								// 137 : DROOP_N	->
	CFG_DISABLED,								// 138 : TST_CLKOUT	->
	CFG_DISABLED,
	CFG_DISABLED,
	CFG_DISABLED,
	CFG_DISABLED,
	CFG_DISABLED,
};

static const uint32_t gpio_default_cfg_1[GPIO_1_GROUP_COUNT * GPIOPADPINS] = {

/* Port  0 */
	CFG_DISABLED,								//   0 : AOP_I2S1_MCK		->
	CFG_DISABLED,								//   1 : AOP_I2S0_MCK		->
	CFG_DISABLED,								//   2 : AOP_PDM_CLK		->
	CFG_DISABLED,								//   3 : AOP_PDM_DAT		->
	CFG_DISABLED,								//   4 : AOP_PLED[0]		->
	CFG_DISABLED,								//   5 : AOP_PLED[1]		->
	CFG_FUNC0 | PULL_DOWN | DRIVE_S4 | VERY_FAST_SLEW | INPUT_SCHMITT,	//   6 : AOP_DOCK_CONNECT	-> DOCK_CONNECT
	CFG_DISABLED | PULL_DOWN | DRIVE_S4 | VERY_FAST_SLEW,			//   7 : AOP_DOCK_ATTENTION	->

/* Port  1 */
	CFG_DISABLED,
	CFG_DISABLED,
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
	CFG_DISABLED,								//  32 : AOP_PLED[2]		->
	CFG_DISABLED,								//  33 : AOP_PLED[3]		->
	CFG_DISABLED,								//  34 : AOP_PLED[4]		->
	CFG_DISABLED,								//  35 : AOP_PLED[5]		->
	CFG_DISABLED,								//  36 : AOP_PLED[6]		->
	CFG_DISABLED,								//  37 : AOP_PLED[7]		->
	CFG_DISABLED,								//  38 : AOP_MON[0]		->
	CFG_DISABLED,								//  39 : AOP_MON[1]		->

/* Port  5 */
	CFG_DISABLED,								//  40 : AOP_MON[2]		->
	CFG_DISABLED,								//  41 : AOP_MON[3]		->
	CFG_DISABLED,								//  42 : AOP_MON[4]		->
	CFG_DISABLED,								//  43 : AOP_MON[5]		->
	CFG_DISABLED,								//  44 : AOP_MON[6]		->
	CFG_DISABLED,								//  45 : AOP_MON[7]		->
	CFG_DISABLED,								//  46 : AOP_SWD_TCK_OUT	->
	CFG_DISABLED,								//  47 : AOP_SWD_TMS0		->

/* Port  6 */
	CFG_DISABLED,								//  48 : AOP_SWD_TMS1		->
	CFG_DISABLED,								//  49 : AOP_UART0_TXD		->
	CFG_DISABLED,								//  50 : AOP_UART0_RXD		->
	CFG_DISABLED,								//  51 : AOP_UART0_RTSN		->
	CFG_DISABLED,								//  52 : AOP_UART0_CTSN		->
	CFG_DISABLED,								//  53 : AOP_UART1_TXD		->
	CFG_DISABLED,								//  54 : AOP_UART1_RXD		->
	CFG_DISABLED,								//  55 : AOP_UART1_RTSN		->

/* Port  7 */
	CFG_DISABLED,								//  56 : AOP_UART1_CTSN		->
	CFG_DISABLED,								//  57 : AOP_UART2_TXD		->
	CFG_DISABLED,								//  58 : AOP_UART2_RXD		->
	CFG_DISABLED,								//  59 : AOP_SPI_CS_TRIG[0]	->
	CFG_DISABLED,								//  60 : AOP_SPI_CS_TRIG[1]	->
	CFG_DISABLED,								//  61 : AOP_SPI_CS_TRIG[2]	->
	CFG_DISABLED,								//  62 : AOP_SPI_CS_TRIG[3]	->
	CFG_DISABLED,								//  63 : AOP_SPI_CS_TRIG[4]	->

/* Port  8 */
	CFG_DISABLED,								//  64 : AOP_SPI_CS_TRIG[5]	->
	CFG_DISABLED,								//  65 : AOP_SPI_CS_TRIG[6]	->
	CFG_DISABLED,								//  66 : AOP_SPI_CS_TRIG[7]	->
	CFG_DISABLED,								//  67 : AOP_SPI_CS_TRIG[8]	->
	CFG_DISABLED,								//  68 : AOP_SPI_CS_TRIG[9]	->
	CFG_DISABLED,								//  69 : AOP_SPI_CS_TRIG[10]	->
	CFG_DISABLED,								//  70 : AOP_SPI_CS_TRIG[11]	->
	CFG_DISABLED,								//  71 : AOP_SPI_CS_TRIG[12]	->

/* Port  9 */
	CFG_DISABLED,								//  72 : AOP_SPI_CS_TRIG[13]	->
	CFG_DISABLED,								//  73 : AOP_SPI_CS_TRIG[14]	->
	CFG_DISABLED,								//  74 : AOP_SPI_CS_TRIG[15]	->
	CFG_DISABLED,								//  75 : AOP_I2C0_SDA		->
	CFG_DISABLED,								//  76 : AOP_I2C0_SCL		->
	CFG_DISABLED,								//  77 : AOP_I2C1_SDA		->
	CFG_DISABLED,								//  78 : AOP_I2C1_SCL		->
	CFG_DISABLED,								//  79 : AOP_SPI0_SCLK		->

/* Port 10 */
	CFG_DISABLED,								//  80 : AOP_SPI0_MOSI		->
	CFG_DISABLED,								//  81 : AOP_SPI0_MISO		->
	CFG_DISABLED,								//  82 : AOP_PSPI_CS_TRIG[3]	->
	CFG_DISABLED,								//  83 : AOP_PSPI_CS_TRIG[4]	->
	CFG_DISABLED,								//  84 : AOP_PSPI_SCLK		->
	CFG_DISABLED,								//  85 : AOP_PSPI_MOSI		->
	CFG_DISABLED,								//  86 : AOP_PSPI_MISO		->
	CFG_DISABLED,								//  87 : AOP_LSPI_SCLK		->

/* Port 11 */
	CFG_DISABLED,								//  88 : AOP_LSPI_MOSI		->
	CFG_DISABLED,								//  89 : AOP_LSPI_MISO		->
	CFG_DISABLED,								//  90 : AOP_DETECT[0]		->
	CFG_DISABLED,								//  91 : AOP_DETECT[1]		->
	CFG_DISABLED,								//  92 : AOP_PSENSE_CTRL[4]	->
	CFG_DISABLED,								//  93 : AOP_PSENSE_CTRL[5]	->
	CFG_DISABLED,								//  94 : AOP_PSENSE_CTRL[6]	->
	CFG_DISABLED,								//  95 : AOP_PSENSE_CTRL[7]	->
};


#endif /* __PLATFORM_PINCONFIG_H */
