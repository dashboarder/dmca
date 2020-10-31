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
#include <debug.h>
#include <stdint.h>
#include <drivers/apple/gpio.h>
#include <platform/soc/hwregbase.h>


static const uint32_t gpio_default_cfg[GPIO_GROUP_COUNT * GPIOPADPINS] = {

/* Port  0 */
	CFG_DISABLED,							//   0 : SPI1_SCLK		->
	CFG_DISABLED,							//   1 : SPI1_MOSI		->
	CFG_DISABLED,							//   2 : SPI1_MISO		->
	CFG_DISABLED,							//   3 : SPI1_SSIN		->
	CFG_DISABLED,							//   4 : ISP_I2C0_SDA		->
	CFG_DISABLED,							//   5 : ISP_I2C0_SCL		->
	CFG_DISABLED,							//   6 : ISP_I2C0_SDA		->
	CFG_DISABLED,							//   7 : ISP_I2C0_SCL		->

/* Port  1 */
	CFG_DISABLED,							//   8 : SENSOR0_ISTRB		->
	CFG_DISABLED,							//   9 : SENSOR0_RST		->
	CFG_DISABLED,							//  10 : SENSOR0_CLK		->
	CFG_DISABLED,							//  11 : SENSOR0_XSHUTDOWN	->
	CFG_DISABLED,							//  12 : SENSOR1_ISTRB		->
	CFG_DISABLED,							//  13 : SENSOR1_RST		->
	CFG_DISABLED,							//  14 : SENSOR1_CLK		->
	CFG_DISABLED,							//  15 : SENSOR1_XSHUTDOWN	->

/* Port  2 */
	CFG_DISABLED,							//  16 : GPIO[16]		-> BOARD_ID[3]
	CFG_DISABLED,							//  17 : GPIO[17]		-> 
	CFG_DISABLED,							//  18 : GPIO[18]		-> BOOT_CONFIG[0]
	CFG_DISABLED,							//  19 : I2S1_MCK		->
	CFG_DISABLED,							//  20 : I2S1_BLCK		->
	CFG_DISABLED,							//  21 : I2S1_LRCK		->
	CFG_DISABLED,							//  22 : I2S1_DIN		->
	CFG_DISABLED,							//  23 : I2S1_DOUT		->

/* Port  3 */
	CFG_DISABLED,							//  24 : NAND_SYS_CLK		->
	CFG_DISABLED,							//  25 : S3E0_RESETN		->
	CFG_DISABLED,							//  26 : S3E1_RESETN		->
	CFG_DISABLED,							//  27 : UART1_TXD		->
	CFG_DISABLED,							//  28 : UART1_RXD		->
	CFG_DISABLED,							//  29 : UART1_RTSN		->
	CFG_DISABLED,							//  30 : UART1_CTSN		->
	CFG_DISABLED,							//  31 : GPIO43			->

/* Port  4 */
	CFG_DISABLED,							//  32 : I2S0_BCLK		->
	CFG_DISABLED,							//  33 : I2S0_LRCK		->
	CFG_DISABLED,							//  34 : I2S0_DIN		->
	CFG_DISABLED,							//  35 : I2S0_DOUT		->
	CFG_DISABLED,							//  36 : I2S0_MCK		->
	CFG_DISABLED,							
	CFG_DISABLED,							
	CFG_DISABLED,							

/* Port  5 */
	CFG_DISABLED,							
	CFG_DISABLED,							
	CFG_DISABLED,							
	CFG_DISABLED,							
	CFG_DISABLED,							
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
	CFG_DISABLED,							//  64 : I2S4_MCK		->
	CFG_DISABLED,							//  65 : I2S4_BCLK		->
	CFG_DISABLED,							//  66 : I2S4_LRCK		->
	CFG_DISABLED,							//  67 : I2S4_DIN		->
	CFG_DISABLED,							//  68 : I2S4_DOUT		->
	CFG_DISABLED,							//  69 : I2S2_MCK		->
	CFG_DISABLED,							//  70 : I2S2_BCLK		->
	CFG_DISABLED,							//  71 : I2S2_LRCK		->

/* Port  9 */
	CFG_DISABLED,							//  72 : I2S2_DIN		->
	CFG_DISABLED,							//  73 : I2S2_DOUT		->
	CFG_DISABLED,							//  74 : GPIO[0]		->
	CFG_DISABLED,							//  75 : GPIO[1]		->
	CFG_DISABLED,							//  76 : GPIO[2]		->
	CFG_DISABLED,							//  77 : GPIO[3]		->
	CFG_DISABLED,							//  78 : GPIO[4]		->
	CFG_DISABLED,							//  79 : GPIO[5]		->

/* Port 10 */
	CFG_DISABLED,							//  80 : GPIO[6]		-> 
	CFG_DISABLED,							//  81 : GPIO[7]		->
	CFG_DISABLED,							//  82 : GPIO[8]		-> 
	CFG_DISABLED,							//  83 : GPIO[9]		->
	CFG_DISABLED,							//  84 : GPIO[10]		->
	CFG_DISABLED,							//  85 : GPIO[11]		->
	CFG_DISABLED,							//  86 : GPIO[12]		->
	CFG_DISABLED,							//  87 : GPIO[13]		->

/* Port 11 */
	CFG_DISABLED,							//  88 : GPIO[14]		->
	CFG_DISABLED,							//  89 : GPIO[15]		->
	CFG_DISABLED,							//  90 : UART3_TXD		->
	CFG_DISABLED,							//  91 : UART3_RXD		->
	CFG_DISABLED,							//  92 : UART3_RTSN		->
	CFG_DISABLED,							//  93 : UART3_CTSN		->
	CFG_FUNC0,							//  94 : SPI0_SCLK		-> SPI0_SCLK/BOARD_ID[0]
	CFG_FUNC0,							//  95 : SPI0_MOSI		-> SPI0_MOSI/BOARD_ID[1]

/* Port 12 */
	CFG_FUNC0,							//  96 : SPI0_MISO		-> SPI0_MISO/BOARD_ID[2]
	CFG_OUT_1,							//  97 : SPI0_SSIN		-> SPI0_CS
	CFG_DISABLED,							//  98 : PCIE_PERST0_N		-> NAND_PERST
	CFG_DISABLED,							//  99 : PCIE_PERST1_N		->
	CFG_DISABLED,							// 100 : PCIE_PERST2_N		->
	CFG_DISABLED,							// 101 : PCIE_PERST3_N		->
	CFG_DISABLED,							// 102 : PCIE_PERST4_N		->
	CFG_DISABLED,							// 103 : PCIE_PERST5_N		-> 

/* Port 13 */
	CFG_DISABLED,							// 104 : PCIE_CLKREQ0_N		-> NAND_CLKREQ
	CFG_DISABLED,							// 105 : PCIE_CLKREQ1_N		-> 
	CFG_DISABLED,							// 106 : PCIE_CLKREQ2_N		-> 
	CFG_DISABLED,							// 107 : PCIE_CLKREQ3_N		->
	CFG_DISABLED,							// 108 : PCIE_CLKREQ4_N		->
	CFG_DISABLED,							// 109 : PCIE_CLKREQ5_N		->
	CFG_DISABLED,							// 110 : UART2_TXD		->
	CFG_DISABLED,							// 111 : UART2_RXD		->

/* Port 14 */
	CFG_DISABLED,							// 112 : UART2_RTSN		->
	CFG_DISABLED,							// 113 : UART2_CTSN		->
	CFG_DISABLED,							// 114 : I2C3_SDA		->
	CFG_DISABLED,							// 115 : I2C3_SCL		->
	CFG_DISABLED,							// 116 : GPIO[44]		->
	CFG_DISABLED,							// 117 : GPIO[45]		->
	CFG_DISABLED,							// 118 : GPIO[46]		->
	CFG_DISABLED,							// 119 : GPIO[47]		->

/* Port 15 */
	CFG_DISABLED,							// 120 : GPIO[48]		->
	CFG_DISABLED,							// 121 : GPIO[49]		->
	CFG_DISABLED,							
	CFG_DISABLED,						
	CFG_DISABLED,					
	CFG_DISABLED,							
	CFG_DISABLED,							
	CFG_DISABLED,							

/* Port 16 */
	CFG_IN,								// 128 : MENU_KEY_L		-> REQUEST_DFU2
	CFG_IN,								// 129 : HOLD_KEY_L		-> REQUEST_DFU1
	CFG_DISABLED,							// 130 : SKEY_L			->
	CFG_DISABLED,							// 131 : SWD_TMS2		->
	CFG_DISABLED,							// 132 : SWD_TMS3		->
	CFG_DISABLED,							// 133 : UART5_RTXD		->
	CFG_DISABLED,							// 134 : I2C2_SDA		->
	CFG_DISABLED,							// 135 : I2C2_SCL		->

/* Port 17 */
	CFG_DISABLED,							// 136 : UART4_TXD		->
	CFG_DISABLED,							// 137 : UART4_RXD		-> 
	CFG_DISABLED,							// 138 : UART4_RTSN		-> 
	CFG_DISABLED,							// 139 : UART4_CTSN		->
	CFG_DISABLED,							// 140 : UART7_TXD		->
	CFG_DISABLED,							// 141 : UART7_RXD		->
	CFG_DISABLED,							// 142 : CLK32K_OUT		->
	CFG_DISABLED,							// 143 : DP_WAKEUP0		->

/* Port 18 */
	CFG_DISABLED,							// 144 : DP_WAKEUP0		->
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
	CFG_DISABLED,							// 160 : SPI2_SCLK		-> 
	CFG_DISABLED,							// 161 : SPI2_MOSI		->
	CFG_DISABLED,							// 162 : SPI2_MISO		->
	CFG_DISABLED,							// 163 : SPI2_SSIN		->
	CFG_DISABLED,							// 164 : I2C0_SDA		-> 
	CFG_DISABLED,							// 165 : I2C0_SCL		->
	CFG_DISABLED,							// 166 : SPI3_SCLK		->
	CFG_DISABLED,							// 167 : SPI3_MOSI		->

/* Port 21 */
	CFG_DISABLED,							// 168 : SPI3_MISO		->
	CFG_DISABLED,							// 169 : SPI3_SSIN		->
	CFG_DISABLED,							// 170 : UART0_TXD		->
	CFG_DISABLED,							// 171 : UART0_RXD		->
	CFG_DISABLED,							// 172 : UART6_TXD		->
	CFG_DISABLED,							// 173 : UART6_RXD		->
	CFG_DISABLED,							// 174 : TMR32_PWM0		->
	CFG_DISABLED,							// 175 : TMR32_PWM1		->

/* Port 22 */
	CFG_DISABLED,							// 176 : TMR32_PWM2		->
	CFG_DISABLED,							// 177 : I2C1_SDA		->
	CFG_DISABLED,							// 178 : I2C1_SCL		->
	CFG_DISABLED,							// 179 : GPIO[19]		->
	CFG_DISABLED,							// 180 : GPIO[20]		->
	CFG_DISABLED,							// 181 : GPIO[21]		->
	CFG_DISABLED,							// 182 : GPIO[22]		->
	CFG_DISABLED,							// 183 : GPIO[23]		->

/* Port 23 */
	CFG_DISABLED,							// 184 : GPIO[24]		-> 
	CFG_DISABLED,							// 185 : GPIO[25]		-> BOOT_CONFIG[1]
	CFG_IN | PULL_DOWN,						// 186 : GPIO[26]		-> FORCE_DFU
	CFG_DISABLED,							// 187 : PSPI_MOSI		->
	CFG_DISABLED,							// 188 : DWI_DO			->
	CFG_DISABLED,							// 189 : PMGR_MISO		->
	CFG_DISABLED,							// 190 : PMGR_SCLK0		->
	CFG_DISABLED,							// 191 : PMGR_SSCLK1		->

/* Port 24 */
	CFG_DISABLED,							// 192 : DROOP			->
	CFG_DISABLED,							// 193 : SOCHOT1		->
	CFG_DISABLED,							// 194 : EDP_HPD0		->
	CFG_DISABLED,							// 195 : EDP_HPD1		->
	CFG_DISABLED,							// 196 : I2S3_MCK		-> 
	CFG_DISABLED,							// 197 : I2S3_BCLK		-> 
	CFG_DISABLED,							// 198 : I2S3_LRCK		->
	CFG_DISABLED,							// 199 : I2S3_DOUT		->
	
/* Port 25 */
	CFG_DISABLED,							// 200 : I2S3_DIN		->
	CFG_DISABLED | PULL_DOWN,					// 201 : GPIO[27]		-> DFU_STATUS
	CFG_DISABLED,							// 202 : GPIO[28]		-> BOOT_CONFIG[2]
	CFG_DISABLED,							// 203 : GPIO[29]		-> BOARD_ID[4]
	CFG_DISABLED,							// 204 : GPIO[30]		-> 
	CFG_DISABLED,							// 205 : GPIO[31]		-> 
	CFG_DISABLED,							// 206 : GPIO[32]		->
	CFG_DISABLED,							// 207 : GPIO[33]		->
		
/* Port 26 */
	CFG_DISABLED,							// 208 : GPIO[34]		->
	CFG_DISABLED,							// 209 : GPIO[35]		->
	CFG_DISABLED,							// 210 : GPIO[36]		->
	CFG_DISABLED,							// 211 : GPIO[37]		->
	CFG_DISABLED,							// 212 : GPIO[38]		-> 
	CFG_DISABLED,							// 213 : GPIO[39]		-> 
	CFG_DISABLED,							// 214 : GPIO[40]		->
	CFG_DISABLED,							// 215 : GPIO[41]		->
	
/* Port 27 */
	CFG_DISABLED,							// 216 : GPIO[42]		->
	CFG_DISABLED,							// 217 : TST_CLKOUT		->
	CFG_DISABLED,							// 218 : GPU_TRIGGER1		->
	CFG_DISABLED,							// 219 : GPU_TRIGGER2		->
	CFG_DISABLED,	
	CFG_DISABLED,	
	CFG_DISABLED,
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

const uint32_t *target_get_default_gpio_cfg(uint32_t gpioc)
{
	switch (gpioc) {
		case 0:
			return gpio_default_cfg;
		case 1:
			return gpio_1_default_cfg;
		default:
			panic("unknown gpio controller %u", gpioc);
	}
}
