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

#ifndef __PLATFORM_PINCONFIG_H
#define __PLATFORM_PINCONFIG_H

static const uint32_t gpio_default_cfg[GPIO_GROUP_COUNT * GPIOPADPINS] = {

/* Port  0 */
	CFG_DISABLED,							//   0 : SWD_TMS2		->
	CFG_DISABLED,							//   1 : SWD_TMS3		->
	CFG_DISABLED,							//   2 : UART5_RTXD		->
	CFG_DISABLED,							//   3 : I2S4_MCK		->
	CFG_DISABLED,							//   4 : I2S4_BCLK		->
	CFG_DISABLED,							//   5 : I2S4_LRCK		->
	CFG_DISABLED,							//   6 : I2S4_DIN		->
	CFG_DISABLED,							//   7 : I2S4_DOUT		->

/* Port  1 */
	CFG_DISABLED,							//   8 : I2S2_MCK		->
	CFG_DISABLED,							//   9 : I2S2_BCLK		->
	CFG_DISABLED,							//  10 : I2S2_LRCK		->
	CFG_DISABLED,							//  11 : I2S2_DIN		->
	CFG_DISABLED,							//  12 : I2S2_DOUT		->
	CFG_DISABLED,							//  13 : SPI1_SCLK		->
	CFG_DISABLED,							//  14 : SPI1_MOSI		->
	CFG_DISABLED,							//  15 : SPI1_MISO		->

/* Port  2 */
	CFG_DISABLED,							//  16 : SPI1_SSIN		->
	CFG_DISABLED,							//  17 : I2S0_MCK		->
	CFG_DISABLED,							//  18 : I2S0_BCLK		->
	CFG_DISABLED,							//  19 : I2S0_LRCK		->
	CFG_DISABLED,							//  20 : I2S0_DIN		->
	CFG_DISABLED,							//  21 : I2S0_DOUT		->
	CFG_DISABLED,							//  22 : I2C2_SDA		->
	CFG_DISABLED,							//  23 : I2C2_SCL		->

/* Port  3 */
	CFG_DISABLED,							//  24 : UART1_TXD		->
	CFG_DISABLED,							//  25 : UART1_RXD		->
	CFG_DISABLED,							//  26 : UART1_RTSN		->
	CFG_DISABLED,							//  27 : UART1_CTSN		->
	CFG_DISABLED,							//  28 : UART4_TXD		->
	CFG_DISABLED,							//  29 : UART4_RXD		->
	CFG_DISABLED,							//  30 : UART4_RTSN		->
	CFG_DISABLED,							//  31 : UART4_CTSN		->

/* Port  4 */
	CFG_DISABLED,							//  32 : UART7_TXD		->
	CFG_DISABLED,							//  33 : UART7_RXD		->
	CFG_DISABLED,							//  34 : CLK32K_OUT		->
	CFG_DISABLED,							//  35 : DP_WAKEUP		->
	CFG_DISABLED,							//  36 : MIPICSI_MUXSEL		->
	CFG_DISABLED,							//  37 : ISP_I2C1_SDA		->
	CFG_DISABLED,							//  38 : ISP_I2C1_SCL		->
	CFG_DISABLED,							//  39 : ISP_I2C0_SDA		->

/* Port  5 */
	CFG_DISABLED,							//  40 : ISP_I2C0_SCL		->
	CFG_DISABLED,							//  41 : SPI2_SCLK		->
	CFG_DISABLED,							//  42 : SPI2_MOSI		->
	CFG_DISABLED,							//  43 : SPI2_MISO		->
	CFG_DISABLED,							//  44 : SPI2_SSIN		->
	CFG_DISABLED,							//  45 : I2C0_SDA		->
	CFG_DISABLED,							//  46 : I2C0_SCL		->
	CFG_DISABLED,							//  47 : SENSOR0_ISTRB		->

/* Port  6 */
	CFG_DISABLED,							//  48 : SENSOR0_RST		->
	CFG_DISABLED,							//  49 : SENSOR0_CLK		->
	CFG_DISABLED,							//  50 : SENSOR0_XSHUTDOWN	->
	CFG_DISABLED,							//  51 : SENSOR1_ISTRB		->
	CFG_DISABLED,							//  52 : SENSOR1_RST		->
	CFG_DISABLED,							//  53 : SENSOR1_CLK		->
	CFG_DISABLED,							//  54 : SENSOR1_XSHUTDOWN	->
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
	CFG_DISABLED,							//  64 : GPIO[0]		->
	CFG_DISABLED,							//  65 : GPIO[1]		->
	CFG_DISABLED,							//  66 : GPIO[2]		->
	CFG_DISABLED,							//  67 : GPIO[3]		->
	CFG_DISABLED,							//  68 : GPIO[4]		->
	CFG_DISABLED,							//  69 : GPIO[5]		->
	CFG_DISABLED,							//  70 : GPIO[6]		->
	CFG_DISABLED,							//  71 : GPIO[7]		->

/* Port  9 */
	CFG_DISABLED,							//  72 : GPIO[8]		->
	CFG_DISABLED,							//  73 : GPIO[9]		->
	CFG_DISABLED,							//  74 : GPIO[10]		->
	CFG_DISABLED,							//  75 : GPIO[11]		->
	CFG_DISABLED,							//  76 : GPIO[12]		->
	CFG_DISABLED,							//  77 : GPIO[13]		->
	CFG_DISABLED,							//  78 : GPIO[14]		->
	CFG_DISABLED,							//  79 : GPIO[15]		->

/* Port 10 */
	CFG_DISABLED,							//  80 : GPIO[16]		-> BOARD_ID[3]
	CFG_DISABLED,							//  81 : GPIO[17]		->
	CFG_DISABLED,							//  82 : GPIO[18]		-> BOOT_CONFIG[0]
	CFG_DISABLED,							//  83 : I2S1_MCK		->
	CFG_DISABLED,							//  84 : I2S1_BCLK		->
	CFG_DISABLED,							//  85 : I2S1_LRCK		->
	CFG_DISABLED,							//  86 : I2S1_DIN		->
	CFG_DISABLED,							//  87 : I2S1_DOUT		->

/* Port 11 */
	CFG_DISABLED,							//  88 : UART3_TXD		->
	CFG_DISABLED,							//  89 : UART3_RXD		->
	CFG_DISABLED,							//  90 : UART3_RTSN		->
	CFG_DISABLED,							//  91 : UART3_CTSN		->
	CFG_DISABLED,
	CFG_DISABLED,
	CFG_DISABLED,
	CFG_DISABLED,

/* Port 12 */
	CFG_IN,								//  96 : MENU_KEY_L		-> REQUEST_DFU2
	CFG_IN,								//  97 : HOLD_KEY_L		-> REQUEST_DFU1
	CFG_DISABLED,							//  98 : SKEY_L			->
	CFG_DISABLED,							//  99 : SPI3_SCLK		->
	CFG_DISABLED,							// 100 : SPI3_MOSI		->
	CFG_DISABLED,							// 101 : SPI3_MISO		->
	CFG_DISABLED,							// 102 : SPI3_SSIN		->
	CFG_DISABLED,							// 103 : SPI0_SCLK		-> SPI0_SCLK/BOARD_ID[0]

/* Port 13 */
	CFG_DISABLED,							// 104 : SPI0_MOSI		-> SPI0_MOSI/BOARD_ID[1]
	CFG_DISABLED,							// 105 : SPI0_MISO		-> SPI0_MISO/BOARD_ID[2]
	CFG_DISABLED,							// 106 : SPI0_SSIN		-> SPI0_CS
	CFG_DISABLED,							// 107 : UART0_TXD		->
	CFG_DISABLED,							// 108 : UART0_RXD		->
	CFG_DISABLED,							// 109 : UART6_TXD		->
	CFG_DISABLED,							// 110 : UART6_RXD		->
	CFG_DISABLED,							// 111 : TMR32_PWM0		->

/* Port 14 */
	CFG_DISABLED,							// 112 : TMR32_PWM1		->
	CFG_DISABLED,							// 113 : TMR32_PWM2		->
	CFG_DISABLED,							// 114 : I2C1_SDA		->
	CFG_DISABLED,							// 115 : I2C1_SCL		->
	CFG_DISABLED,							// 116 : GPIO[19]		->
	CFG_DISABLED,							// 117 : GPIO[20]		->
	CFG_DISABLED,							// 118 : GPIO[21]		->
	CFG_DISABLED,							// 119 : GPIO[22]		->

/* Port 15 */
	CFG_DISABLED,							// 120 : GPIO[23]		->
	CFG_DISABLED,							// 121 : GPIO[24]		->
	CFG_DISABLED,							// 122 : GPIO[25]		-> BOOT_CONFIG[1]
	CFG_IN | PULL_DOWN,						// 123 : GPIO[26]		-> FORCE_DFU
	CFG_DISABLED | PULL_DOWN,					// 124 : PMGR_MOSI		-> DFU_STATUS
	CFG_DISABLED,							// 125 : PMGR_MISO		->
	CFG_DISABLED,							// 126 : PMGR_SCLK0		->
	CFG_DISABLED,							// 127 : PMGR_SSCLK1		->

/* Port 16 */
	CFG_DISABLED,							// 128 : SOCHOT0		->
	CFG_DISABLED,							// 129 : SOCHOT1		->
	CFG_DISABLED,							// 130 : EDP_HPD		->
	CFG_DISABLED,							// 131 : I2S3_MCK		->
	CFG_DISABLED,							// 132 : I2S3_BCLK		->
	CFG_DISABLED,							// 133 : I2S3_LRCK		->
	CFG_DISABLED,							// 134 : I2S3_DOUT		->
	CFG_DISABLED,							// 135 : I2S3_DIN		->

/* Port 17 */
	CFG_DISABLED,							// 136 : GPIO[27]		->
	CFG_DISABLED,							// 137 : GPIO[28]		-> BOOT_CONFIG[2]
	CFG_DISABLED,							// 138 : GPIO[29]		-> BOARD_ID[4]
	CFG_DISABLED,							// 139 : GPIO[30]		->
	CFG_DISABLED,							// 140 : GPIO[31]		->
	CFG_DISABLED,							// 141 : GPIO[32]		->
	CFG_DISABLED,							// 142 : GPIO[33]		->
	CFG_DISABLED,							// 143 : GPIO[34]		->

/* Port 18 */
	CFG_DISABLED,							// 144 : GPIO[35]		->
	CFG_DISABLED,							// 145 : GPIO[36]		->
	CFG_DISABLED,							// 146 : GPIO[37]		->
	CFG_DISABLED,							// 147 : GPIO[38]		->
	CFG_DISABLED,							// 148 : GPIO[39]		->
	CFG_DISABLED,							// 149 : GPIO[40]		->
	CFG_DISABLED,							// 150 : GPIO[41]		->
	CFG_DISABLED,							// 151 : GPIO[42]		->

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
	CFG_DISABLED,							// 160 : PCIE_PERST0_N		-> NAND_PERST
	CFG_DISABLED,							// 161 : PCIE_PERST1_N		->
	CFG_DISABLED,							// 162 : PCIE_PERST2_N		->
	CFG_DISABLED,							// 163 : PCIE_PERST3_N		->
	CFG_DISABLED | INPUT_SCHMITT,					// 164 : PCIE_CLKREQ0_N		-> NAND_CLKREQ
	CFG_DISABLED,							// 165 : PCIE_CLKREQ1_N		->
	CFG_DISABLED,							// 166 : PCIE_CLKREQ2_N		->
	CFG_DISABLED,							// 167 : PCIE_CLKREQ3_N		->

/* Port 21 */
	CFG_DISABLED,
	CFG_DISABLED,
	CFG_DISABLED,
	CFG_DISABLED,
	CFG_DISABLED,
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
	CFG_DISABLED,							// 192 : UART2_TXD		->
	CFG_DISABLED,							// 193 : UART2_RXD		->
	CFG_DISABLED,							// 194 : UART2_RTSN		->
	CFG_DISABLED,							// 195 : UART2_CTSN		->
	CFG_DISABLED,							// 196 : NAND_SYS_CLK		-> NAND_SYS_CLK
	CFG_DISABLED,							// 197 : S3E_RESETN		-> NAND_RESET
	CFG_DISABLED,							// 198 : TST_CLKOUT		->
	CFG_DISABLED,
};

static const uint32_t gpio_1_default_cfg[GPIO_1_GROUP_COUNT * GPIOPADPINS] = {

/* Port  0 */
	CFG_DISABLED,							//   0 : AOP_SPI_SCLK		->
	CFG_DISABLED,							//   1 : AOP_SPI_MOSI		->
	CFG_DISABLED,							//   2 : AOP_SPI_MISO		->
	CFG_DISABLED,							//   3 : AOP_UART1_TXD		->
	CFG_DISABLED,							//   4 : AOP_UART1_RXD		->
	CFG_DISABLED,							//   5 : AOP_UART0_TXD		->
	CFG_DISABLED,							//   6 : AOP_UART0_RXD		->
	CFG_DISABLED,							//   7 : AOP_UART2_TXD		->

/* Port  1 */
	CFG_DISABLED,							//   8 : AOP_UART2_RXD		->
	CFG_DISABLED,							//   9 : AOP_I2CM_SDA		->
	CFG_DISABLED,							//  10 : AOP_I2CM_SCL		->
	CFG_DISABLED,							//  11 : AOP_FUNC[0]		->
	CFG_DISABLED,							//  12 : AOP_FUNC[1]		->
	CFG_DISABLED,							//  13 : AOP_FUNC[2]		->
	CFG_DISABLED,							//  14 : AOP_FUNC[3]		->
	CFG_DISABLED,							//  15 : AOP_FUNC[4]		->

/* Port  2 */
	CFG_DISABLED,							//  16 : AOP_FUNC[5]		->
	CFG_DISABLED,							//  17 : AOP_FUNC[6]		->
	CFG_DISABLED,							//  18 : AOP_FUNC[7]		->
	CFG_DISABLED,							//  19 : AOP_FUNC[8]		->
	CFG_DISABLED,							//  20 : AOP_FUNC[9]		->
	CFG_DISABLED,							//  21 : AOP_SWD_TCK_OUT	->
	CFG_DISABLED,							//  22 : AOP_SWD_TMS0		->
	CFG_DISABLED,							//  23 : AOP_SWD_TMS1		->

/* Port  3 */
	CFG_DISABLED,							//  24 : AOP_I2S_MCK		->
	CFG_DISABLED,							//  25 : AOP_I2S_BCLK		->
	CFG_DISABLED,							//  26 : AOP_I2S_LRCK		->
	CFG_DISABLED,							//  27 : AOP_I2S_DIN		->
	CFG_DISABLED,
	CFG_DISABLED,
	CFG_DISABLED,
	CFG_DISABLED,
};

#endif /* ! __PLATFORM_PINCONFIG_H */
