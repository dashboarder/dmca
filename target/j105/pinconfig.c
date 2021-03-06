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
   I/O Spreadsheet version: rev 0v16
   I/O Spreadsheet tracker: <rdar://problem/17751195>
   Conversion command: csvtopinconfig.py --soc elba --copyright 2015 --radar '<rdar://problem/17751195>' <filename>
*/

#include <debug.h>
#include <drivers/apple/gpio.h>
#include <platform.h>
#include <platform/soc/hwregbase.h>
#include <stdint.h>

static const uint32_t pinconfig_ap_0[GPIO_GROUP_COUNT * GPIOPADPINS] = {

/* Port  0 */
	CFG_DISABLED,							//   0 : SPI1_SCLK		-> NC
	CFG_DISABLED,							//   1 : SPI1_MOSI		-> NC
	CFG_DISABLED,							//   2 : SPI1_MISO		-> NC
	CFG_DISABLED,							//   3 : SPI1_SSIN		-> NC
	CFG_DISABLED,							//   4 : ISP_I2C1_SDA		-> NC
	CFG_DISABLED,							//   5 : ISP_I2C1_SCL		-> NC
	CFG_DISABLED,							//   6 : ISP_I2C0_SDA		-> NC
	CFG_DISABLED,							//   7 : ISP_I2C0_SCL		-> NC

/* Port  1 */
	CFG_DISABLED,							//   8 : SENSOR0_ISTRB		-> NC
	CFG_DISABLED,							//   9 : SENSOR0_RST		-> NC
	CFG_DISABLED,							//  10 : SENSOR0_CLK		-> NC
	CFG_DISABLED,							//  11 : SENSOR0_XSHUTDOWN	-> NC
	CFG_DISABLED,							//  12 : SENSOR1_ISTRB		-> NC
	CFG_DISABLED,							//  13 : SENSOR1_RST		-> NC
	CFG_DISABLED,							//  14 : SENSOR1_CLK		-> NC
	CFG_DISABLED,							//  15 : SENSOR1_XSHUTDOWN	-> NC

/* Port  2 */
	CFG_IN | PULL_DOWN,						//  16 : GPIO[16]		-> SOC_BRD_ID3
	CFG_IN,								//  17 : GPIO[17]		-> PMU_SOC_IRQ_L
	CFG_IN | PULL_DOWN,						//  18 : GPIO[18]		-> SOC_BOOT_CFG0
	CFG_DISABLED,							//  19 : I2S1_MCK		-> NC
	CFG_DISABLED,							//  20 : I2S1_BCLK		-> NC
	CFG_DISABLED,							//  21 : I2S1_LRCK		-> NC
	CFG_DISABLED,							//  22 : I2S1_DIN		-> NC
	CFG_DISABLED,							//  23 : I2S1_DOUT		-> NC

/* Port  3 */
	CFG_FUNC0 | DRIVE_S8 | SLOW_SLEW,				//  24 : NAND_SYS_CLK		-> SOC_CLK24M_NAND_R
	CFG_FUNC0 | PULL_UP | DRIVE_S4 | SLOW_SLEW,			//  25 : S3E0_RESETN		-> SOC_NAND_RESET0_L
	CFG_FUNC0 | PULL_UP | DRIVE_S4 | SLOW_SLEW,			//  26 : S3E1_RESETN		-> SOC_NAND_RESET1_L
	CFG_DISABLED,							//  27 : UART1_TXD		-> NC
	CFG_DISABLED,							//  28 : UART1_RXD		-> NC
	CFG_DISABLED,							//  29 : UART1_RTSN		-> NC
	CFG_DISABLED,							//  30 : UART1_CTSN		-> NC
	CFG_OUT_0 | PULL_DOWN | DRIVE_S4 | SLOW_SLEW,			//  31 : GPIO[43]		-> SOC_NAND_FW_STRAP

/* Port  4 */
	CFG_DISABLED,							//  32 : I2S0_BCLK		-> NC
	CFG_DISABLED,							//  33 : I2S0_LRCK		-> NC
	CFG_DISABLED,							//  34 : I2S0_DIN		-> NC
	CFG_DISABLED,							//  35 : I2S0_DOUT		-> NC
	CFG_DISABLED,							//  36 : I2S0_MCK		-> NC
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
	CFG_DISABLED,							//  64 : I2S4_MCK		-> NC
	CFG_DISABLED,							//  65 : I2S4_BCLK		-> NC
	CFG_DISABLED,							//  66 : I2S4_LRCK		-> NC
	CFG_DISABLED,							//  67 : I2S4_DIN		-> NC
	CFG_DISABLED,							//  68 : I2S4_DOUT		-> NC
	CFG_DISABLED,							//  69 : I2S2_MCK		-> NC
	CFG_DISABLED,							//  70 : I2S2_BCLK		-> NC
	CFG_DISABLED,							//  71 : I2S2_LRCK		-> NC

/* Port  9 */
	CFG_DISABLED,							//  72 : I2S2_DIN		-> NC
	CFG_DISABLED,							//  73 : I2S2_DOUT		-> NC
	CFG_IN,								//  74 : GPIO[0]		-> HDMI_SOC_5V_OCP_FAULT_L
	CFG_DISABLED,							//  75 : GPIO[1]		-> NC
	CFG_DISABLED,							//  76 : GPIO[2]		-> NC
	CFG_DISABLED,							//  77 : GPIO[3]		-> NC
	CFG_DISABLED,							//  78 : GPIO[4]		-> NC
	CFG_DISABLED,							//  79 : GPIO[5]		-> NC

/* Port 10 */
	CFG_DISABLED,							//  80 : GPIO[6]		-> NC
	CFG_DISABLED,							//  81 : GPIO[7]		-> NC
	CFG_DISABLED,							//  82 : GPIO[8]		-> NC
	CFG_DISABLED,							//  83 : GPIO[9]		-> NC
	CFG_IN,								//  84 : GPIO[10]		-> USBCCTRL_SOC_IRQ
	CFG_DISABLED,							//  85 : GPIO[11]		-> NC
	CFG_DISABLED | PULL_DOWN,					//  86 : GPIO[12]		-> SOC_WLAN_DEVICE_WAKE
	CFG_DISABLED,							//  87 : GPIO[13]		-> NC

/* Port 11 */
	CFG_DISABLED,							//  88 : GPIO[14]		-> NC
	CFG_DISABLED,							//  89 : GPIO[15]		-> NC
	CFG_FUNC0 | DRIVE_S4 | SLOW_SLEW,				//  90 : UART3_TXD		-> UART3_TXD
	CFG_FUNC0,							//  91 : UART3_RXD		-> UART3_RXD
	CFG_FUNC0 | DRIVE_S4 | SLOW_SLEW,				//  92 : UART3_RTSN		-> UART3_RTS_L
	CFG_FUNC0,							//  93 : UART3_CTSN		-> UART3_CTS_L
	CFG_IN | PULL_UP,						//  94 : SPI0_SCLK		-> SOC_BRD_ID0
	CFG_IN | PULL_UP,						//  95 : SPI0_MOSI		-> SOC_BRD_ID1

/* Port 12 */
	CFG_IN | PULL_UP,						//  96 : SPI0_MISO		-> SOC_BRD_ID2
	CFG_DISABLED,							//  97 : SPI0_SSIN		-> NC
	CFG_IN,								//  98 : PCIE_PERST0_N		-> PCIE0_RESET_L
	CFG_IN,								//  99 : PCIE_PERST1_N		-> PCIE1_RESET_L
	CFG_IN,								// 100 : PCIE_PERST2_N		-> PCIE2_RESET_L
	CFG_IN,								// 101 : PCIE_PERST3_N		-> PCIE3_RESET_L
	CFG_IN,								// 102 : PCIE_PERST4_N		-> PCIE4_RESET_L
	CFG_IN,								// 103 : PCIE_PERST5_N		-> PCIE5_RESET_L

/* Port 13 */
	CFG_FUNC0 | DRIVE_S4 | SLOW_SLEW,				// 104 : PCIE_CLKREQ0_N		-> PCIE0_CLKREQ_L
	CFG_FUNC0 | DRIVE_S4 | SLOW_SLEW,				// 105 : PCIE_CLKREQ1_N		-> PCIE1_CLKREQ_L
	CFG_FUNC0 | DRIVE_S4 | SLOW_SLEW,				// 106 : PCIE_CLKREQ2_N		-> PCIE2_CLKREQ_L
	CFG_FUNC0 | DRIVE_S4 | SLOW_SLEW,				// 107 : PCIE_CLKREQ3_N		-> PCIE3_CLKREQ_L
	CFG_FUNC0 | DRIVE_S4 | SLOW_SLEW,				// 108 : PCIE_CLKREQ4_N		-> PCIE4_CLKREQ_L
	CFG_DISABLED,							// 109 : PCIE_CLKREQ5_N		-> PCIE5_CLKREQ_L
	CFG_DISABLED,							// 110 : UART2_TXD		-> NC
	CFG_DISABLED,							// 111 : UART2_RXD		-> NC

/* Port 14 */
	CFG_DISABLED,							// 112 : UART2_RTSN		-> NC
	CFG_DISABLED,							// 113 : UART2_CTSN		-> NC
	CFG_FUNC0 | DRIVE_S4 | SLOW_SLEW,				// 114 : I2C3_SDA		-> I2C3_SDA
	CFG_FUNC0 | DRIVE_S4 | SLOW_SLEW,				// 115 : I2C3_SCL		-> I2C3_SCL
	CFG_DISABLED,							// 116 : GPIO[44]		-> NC
	CFG_IN,								// 117 : GPIO[45]		-> USB3CTRL_SOC_SMI_L
	CFG_DISABLED,							// 118 : GPIO[46]		-> NC
	CFG_OUT_0 | DRIVE_S4 | SLOW_SLEW,				// 119 : GPIO[47]		-> SOC_RDRVR_PD_L

/* Port 15 */
	CFG_DISABLED,							// 120 : GPIO[48]		-> NC
	CFG_IN | PULL_UP,						// 121 : GPIO[49]		-> USB3CTRL_SOC_WAKE_L
	CFG_DISABLED,							// 122 : UNSPECIFIED		-> UNSPECIFIED
	CFG_DISABLED,							// 123 : UNSPECIFIED		-> UNSPECIFIED
	CFG_DISABLED,							// 124 : UNSPECIFIED		-> UNSPECIFIED
	CFG_DISABLED,
	CFG_DISABLED,
	CFG_DISABLED,

/* Port 16 */
	CFG_DISABLED,							// 128 : SWD_TMS2		-> AOP_NAND_SWD_SWDIO
	CFG_DISABLED,							// 129 : SWD_TMS3		-> NC
	CFG_FUNC0 | DRIVE_S4 | SLOW_SLEW,				// 130 : UART5_RTXD		-> UART5_RTXD
	CFG_FUNC0 | DRIVE_S4 | SLOW_SLEW,				// 131 : I2C2_SDA		-> I2C2_SDA
	CFG_FUNC0 | DRIVE_S4 | SLOW_SLEW,				// 132 : I2C2_SCL		-> I2C2_SCL
	CFG_FUNC0 | DRIVE_S4 | SLOW_SLEW,				// 133 : UART4_TXD		-> UART4_TXD
	CFG_FUNC0,							// 134 : UART4_RXD		-> UART4_RXD
	CFG_FUNC0 | DRIVE_S4 | SLOW_SLEW,				// 135 : UART4_RTSN		-> UART4_RTS_L

/* Port 17 */
	CFG_FUNC0,							// 136 : UART4_CTSN		-> UART4_CTS_L
	CFG_FUNC0 | DRIVE_S4 | SLOW_SLEW,				// 137 : UART7_TXD		-> UART7_TXD
	CFG_FUNC0,							// 138 : UART7_RXD		-> UART7_RXD
	CFG_DISABLED,							// 139 : CLK32K_OUT		-> NC
	CFG_DISABLED,							// 140 : DP_WAKEUP0		-> NC
	CFG_FUNC0 | DRIVE_S4 | SLOW_SLEW,				// 141 : DP_WAKEUP1		-> SOC_USBCCTRL_AUX_OE
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
	CFG_DISABLED,							// 160 : SPI2_SCLK		-> NC
	CFG_DISABLED,							// 161 : SPI2_MOSI		-> NC
	CFG_DISABLED,							// 162 : SPI2_MISO		-> NC
	CFG_DISABLED,							// 163 : SPI2_SSIN		-> NC
	CFG_FUNC0 | DRIVE_S4 | SLOW_SLEW,				// 164 : I2C0_SDA		-> I2C0_SDA
	CFG_FUNC0 | DRIVE_S4 | SLOW_SLEW,				// 165 : I2C0_SCL		-> I2C0_SCL
	CFG_FUNC0 | DRIVE_S4 | SLOW_SLEW,				// 166 : SPI3_SCLK		-> SPI3_SCLK
	CFG_FUNC0 | DRIVE_S4 | SLOW_SLEW,				// 167 : SPI3_MOSI		-> SPI3_MOSI

/* Port 21 */
	CFG_FUNC0 | DRIVE_S4 | SLOW_SLEW,				// 168 : SPI3_MISO		-> SPI3_MISO
	CFG_FUNC0 | DRIVE_S4 | SLOW_SLEW,				// 169 : SPI3_SSIN		-> SPI3_SSIN
	CFG_FUNC0 | DRIVE_S4 | SLOW_SLEW,				// 170 : UART0_TXD		-> UART0_TXD
	CFG_FUNC0 | DRIVE_S4 | SLOW_SLEW,				// 171 : UART0_RXD		-> UART0_RXD
	CFG_DISABLED,							// 172 : UART6_TXD		-> NC
	CFG_DISABLED,							// 173 : UART6_RXD		-> NC
	CFG_IN,								// 174 : TMR32_PWM0		-> FAN_SOC_TACH
	CFG_IN,								// 175 : TMR32_PWM1		-> WLAN_SOC_ATSP

/* Port 22 */
	CFG_DISABLED,							// 176 : TMR32_PWM2		-> SOC_FAN_PWM
	CFG_FUNC0 | DRIVE_S4 | SLOW_SLEW,				// 177 : I2C1_SDA		-> I2C1_SDA
	CFG_FUNC0 | DRIVE_S4 | SLOW_SLEW,				// 178 : I2C1_SCL		-> I2C1_SCL
	CFG_DISABLED,							// 179 : GPIO[19]		-> NC
	CFG_IN | PULL_DOWN,						// 180 : GPIO[20]		-> SOC_GPIO_20
	CFG_DISABLED,							// 181 : GPIO[21]		-> SOC_GPU_ALTV_VID
	CFG_IN,								// 182 : GPIO[22]		-> GPU_SOC_PGOOD
	CFG_DISABLED,							// 183 : GPIO[23]		-> SOC_SIL_PT

/* Port 23 */
	CFG_DISABLED,							// 184 : GPIO[24]		-> NC
	CFG_IN | PULL_DOWN,						// 185 : GPIO[25]		-> SOC_BOOT_CFG1
	CFG_IN,								// 186 : GPIO[26]		-> SOC_FORCE_DFU
	CFG_FUNC0 | DRIVE_S7 | SLOW_SLEW,				// 187 : PSPI_MOSI		-> PMGR_SPI_MOSI
	CFG_DISABLED,							// 188 : DWI_DO			-> NC
	CFG_FUNC0 | DRIVE_S7 | SLOW_SLEW,				// 189 : PMGR_MISO		-> PMGR_SPI_MISO
	CFG_FUNC0 | DRIVE_S7 | SLOW_SLEW,				// 190 : PMGR_SCLK0		-> PMGR_SPI_CLK
	CFG_DISABLED,							// 191 : PMGR_SSCLK1		-> NC

/* Port 24 */
	CFG_FUNC0 | DRIVE_S4 | SLOW_SLEW,				// 192 : DROOP			-> COMP_SOC_DROOP_L
	CFG_FUNC0 | DRIVE_S4 | SLOW_SLEW,				// 193 : SOCHOT1		-> SOC_PMU_SOCHOT_L
	CFG_FUNC0 | DRIVE_S4 | SLOW_SLEW,				// 194 : EDP_HPD0		-> DP2HDMI_LPDP0_HPD
	CFG_FUNC0 | DRIVE_S4 | SLOW_SLEW,				// 195 : EDP_HPD1		-> USBCCTRL_SOC_RDRVR_HPD
	CFG_DISABLED,							// 196 : I2S3_MCK		-> NC
	CFG_DISABLED,							// 197 : I2S3_BCLK		-> NC
	CFG_DISABLED,							// 198 : I2S3_LRCK		-> NC
	CFG_DISABLED,							// 199 : I2S3_DOUT		-> NC

/* Port 25 */
	CFG_DISABLED,							// 200 : I2S3_DIN		-> NC
	CFG_DISABLED,							// 201 : GPIO[27]		-> SOC_DFU_STATUS
	CFG_IN | PULL_UP,						// 202 : GPIO[28]		-> SOC_BOOT_CFG2
	CFG_IN | PULL_UP,						// 203 : GPIO[29]		-> SOC_BRD_ID4
	CFG_DISABLED,							// 204 : GPIO[30]		-> SOC_SIL_CTRL1
	CFG_DISABLED,							// 205 : GPIO[31]		-> SOC_SIL_CTRL2
	CFG_DISABLED,							// 206 : GPIO[32]		-> NC
	CFG_DISABLED,							// 207 : GPIO[33]		-> SOC_SIL_CTRL0

/* Port 26 */
	CFG_IN | PULL_DOWN,						// 208 : GPIO[34]		-> SOC_BRD_REV0
	CFG_IN | PULL_DOWN,						// 209 : GPIO[35]		-> SOC_BRD_REV1
	CFG_IN | PULL_DOWN,						// 210 : GPIO[36]		-> SOC_BRD_REV2
	CFG_IN | PULL_DOWN,						// 211 : GPIO[37]		-> SOC_BRD_REV3
	CFG_DISABLED,							// 212 : GPIO[38]		-> NC
	CFG_DISABLED,							// 213 : GPIO[39]		-> NC
	CFG_DISABLED,							// 214 : GPIO[40]		-> NC
	CFG_DISABLED | PULL_DOWN,					// 215 : GPIO[41]		-> SOC_BT_DEVICE_WAKE

/* Port 27 */
	CFG_DISABLED,							// 216 : GPIO[42]		-> NC
	CFG_DISABLED,							// 217 : TST_CLKOUT		-> SOC_PMU_TEST_CLKOUT
	CFG_DISABLED,							// 218 : GPU_TRIGGER1		-> NC
	CFG_DISABLED,							// 219 : GPU_TRIGGER2		-> NC
	CFG_DISABLED,
	CFG_DISABLED,
	CFG_DISABLED,
	CFG_DISABLED,
};

static const uint32_t pinconfig_ap_1[GPIO_1_GROUP_COUNT * GPIOPADPINS] = {

/* Port  0 */
	CFG_FUNC0 | DRIVE_S4 | SLOW_SLEW,				//   0 : AOP_SPI_SCLK		-> AOP_SPI_SCLK
	CFG_FUNC0 | DRIVE_S4 | SLOW_SLEW,				//   1 : AOP_SPI_MOSI		-> AOP_SPI_MOSI
	CFG_FUNC0 | DRIVE_S4 | SLOW_SLEW,				//   2 : AOP_SPI_MISO		-> AOP_SPI_MISO
	CFG_DISABLED,							//   3 : AOP_UART1_TXD		-> NC
	CFG_DISABLED,							//   4 : AOP_UART1_RXD		-> NC
	CFG_FUNC0 | DRIVE_S4 | SLOW_SLEW,				//   5 : AOP_UART0_TXD		-> AOP_UART0_TXD
	CFG_FUNC0,							//   6 : AOP_UART0_RXD		-> AOP_UART0_RXD
	CFG_DISABLED,							//   7 : AOP_UART2_TXD		-> NC

/* Port  1 */
	CFG_DISABLED,							//   8 : AOP_UART2_RXD		-> NC
	CFG_FUNC0 | PULL_UP | DRIVE_S4 | SLOW_SLEW,			//   9 : AOP_I2CM_SDA		-> AOP_I2C_SDA
	CFG_FUNC0 | PULL_UP | DRIVE_S4 | SLOW_SLEW,			//  10 : AOP_I2CM_SCL		-> AOP_I2C_SCL
	CFG_IN,								//  11 : AOP_FUNC[0]		-> AOP_DFU_REQ_L
	CFG_DISABLED,							//  12 : AOP_FUNC[1]		-> AOP_DFU_REQCLR
	CFG_IN,								//  13 : AOP_FUNC[2]		-> DP2HDMI_AOP_VDD12ON
	CFG_IN,								//  14 : AOP_FUNC[3]		-> AOP_FUNC_3
	CFG_IN,								//  15 : AOP_FUNC[4]		-> HDMI_AOP_HPD

/* Port  2 */
	CFG_DISABLED,							//  16 : AOP_FUNC[5]		-> AOP_HDMI_CEC
	CFG_DISABLED,							//  17 : AOP_FUNC[6]		-> IRRCVR_OUT_RC_1V8
	CFG_DISABLED,							//  18 : AOP_FUNC[7]		-> IRRCVR_OUT_RC_1V8
	CFG_IN,								//  19 : AOP_FUNC[8]		-> DP2HDMI_PMU_CEC_IRQ
	CFG_DISABLED,							//  20 : AOP_FUNC[9]		-> HDMI_AOP_CEC
	CFG_IN,								//  21 : AOP_SWD_TCK_OUT	-> AOP_SWD_SWCLK
	CFG_DISABLED,							//  22 : AOP_SWD_TMS0		-> NC
	CFG_DISABLED,							//  23 : AOP_SWD_TMS1		-> NC

/* Port  3 */
	CFG_DISABLED,							//  24 : AOP_I2S_MCK		-> AOP_I2S_MCK
	CFG_DISABLED,							//  25 : AOP_I2S_BCLK		-> AOP_I2S_BCLK
	CFG_DISABLED,							//  26 : AOP_I2S_LRCK		-> AOP_I2S_LRCK
	CFG_DISABLED,							//  27 : AOP_I2S_DIN		-> AOP_I2S_DIN
	CFG_DISABLED,
	CFG_DISABLED,
	CFG_DISABLED,
	CFG_DISABLED,
};

static const uint32_t pinconfig_dev_0[GPIO_GROUP_COUNT * GPIOPADPINS] = {

/* Port  0 */
	CFG_DISABLED,							//   0 : SPI1_SCLK		-> NC
	CFG_DISABLED,							//   1 : SPI1_MOSI		-> NC
	CFG_DISABLED,							//   2 : SPI1_MISO		-> NC
	CFG_DISABLED,							//   3 : SPI1_SSIN		-> NC
	CFG_DISABLED,							//   4 : ISP_I2C1_SDA		-> NC
	CFG_DISABLED,							//   5 : ISP_I2C1_SCL		-> NC
	CFG_DISABLED,							//   6 : ISP_I2C0_SDA		-> NC
	CFG_DISABLED,							//   7 : ISP_I2C0_SCL		-> NC

/* Port  1 */
	CFG_DISABLED,							//   8 : SENSOR0_ISTRB		-> NC
	CFG_DISABLED,							//   9 : SENSOR0_RST		-> NC
	CFG_DISABLED,							//  10 : SENSOR0_CLK		-> NC
	CFG_DISABLED,							//  11 : SENSOR0_XSHUTDOWN	-> NC
	CFG_DISABLED,							//  12 : SENSOR1_ISTRB		-> NC
	CFG_DISABLED,							//  13 : SENSOR1_RST		-> NC
	CFG_DISABLED,							//  14 : SENSOR1_CLK		-> NC
	CFG_DISABLED,							//  15 : SENSOR1_XSHUTDOWN	-> NC

/* Port  2 */
	CFG_IN | PULL_DOWN,						//  16 : GPIO[16]		-> SOC_BRD_ID3
	CFG_IN,								//  17 : GPIO[17]		-> PMU_SOC_IRQ_L
	CFG_IN | PULL_DOWN,						//  18 : GPIO[18]		-> SOC_BOOT_CFG0
	CFG_DISABLED,							//  19 : I2S1_MCK		-> NC
	CFG_DISABLED,							//  20 : I2S1_BCLK		-> NC
	CFG_DISABLED,							//  21 : I2S1_LRCK		-> NC
	CFG_DISABLED,							//  22 : I2S1_DIN		-> NC
	CFG_DISABLED,							//  23 : I2S1_DOUT		-> NC

/* Port  3 */
	CFG_FUNC0 | DRIVE_S8 | SLOW_SLEW,				//  24 : NAND_SYS_CLK		-> SOC_CLK24M_NAND_R
	CFG_FUNC0 | PULL_UP | DRIVE_S4 | SLOW_SLEW,			//  25 : S3E0_RESETN		-> SOC_NAND_RESET0_L
	CFG_FUNC0 | PULL_UP | DRIVE_S4 | SLOW_SLEW,			//  26 : S3E1_RESETN		-> SOC_NAND_RESET1_L
	CFG_DISABLED,							//  27 : UART1_TXD		-> NC
	CFG_DISABLED,							//  28 : UART1_RXD		-> NC
	CFG_DISABLED,							//  29 : UART1_RTSN		-> NC
	CFG_DISABLED,							//  30 : UART1_CTSN		-> NC
	CFG_OUT_0 | PULL_DOWN | DRIVE_S4 | SLOW_SLEW,			//  31 : GPIO[43]		-> SOC_NAND_FW_STRAP

/* Port  4 */
	CFG_DISABLED,							//  32 : I2S0_BCLK		-> NC
	CFG_DISABLED,							//  33 : I2S0_LRCK		-> NC
	CFG_DISABLED,							//  34 : I2S0_DIN		-> NC
	CFG_DISABLED,							//  35 : I2S0_DOUT		-> NC
	CFG_DISABLED,							//  36 : I2S0_MCK		-> NC
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
	CFG_DISABLED,							//  64 : I2S4_MCK		-> NC
	CFG_DISABLED,							//  65 : I2S4_BCLK		-> NC
	CFG_DISABLED,							//  66 : I2S4_LRCK		-> NC
	CFG_DISABLED,							//  67 : I2S4_DIN		-> NC
	CFG_DISABLED,							//  68 : I2S4_DOUT		-> NC
	CFG_DISABLED,							//  69 : I2S2_MCK		-> NC
	CFG_DISABLED,							//  70 : I2S2_BCLK		-> NC
	CFG_DISABLED,							//  71 : I2S2_LRCK		-> NC

/* Port  9 */
	CFG_DISABLED,							//  72 : I2S2_DIN		-> NC
	CFG_DISABLED,							//  73 : I2S2_DOUT		-> NC
	CFG_IN,								//  74 : GPIO[0]		-> HDMI_SOC_5V_OCP_FAULT_L
	CFG_DISABLED,							//  75 : GPIO[1]		-> NC
	CFG_DISABLED,							//  76 : GPIO[2]		-> NC
	CFG_DISABLED,							//  77 : GPIO[3]		-> NC
	CFG_DISABLED,							//  78 : GPIO[4]		-> NC
	CFG_DISABLED,							//  79 : GPIO[5]		-> NC

/* Port 10 */
	CFG_DISABLED,							//  80 : GPIO[6]		-> NC
	CFG_DISABLED,							//  81 : GPIO[7]		-> NC
	CFG_DISABLED,							//  82 : GPIO[8]		-> NC
	CFG_DISABLED,							//  83 : GPIO[9]		-> NC
	CFG_IN,								//  84 : GPIO[10]		-> USBCCTRL_SOC_IRQ
	CFG_DISABLED,							//  85 : GPIO[11]		-> NC
	CFG_DISABLED | PULL_DOWN,					//  86 : GPIO[12]		-> SOC_WLAN_DEVICE_WAKE
	CFG_DISABLED,							//  87 : GPIO[13]		-> NC

/* Port 11 */
	CFG_DISABLED,							//  88 : GPIO[14]		-> NC
	CFG_DISABLED,							//  89 : GPIO[15]		-> NC
	CFG_FUNC0 | DRIVE_S4 | SLOW_SLEW,				//  90 : UART3_TXD		-> UART3_TXD
	CFG_FUNC0,							//  91 : UART3_RXD		-> UART3_RXD
	CFG_FUNC0 | DRIVE_S4 | SLOW_SLEW,				//  92 : UART3_RTSN		-> UART3_RTS_L
	CFG_FUNC0,							//  93 : UART3_CTSN		-> UART3_CTS_L
	CFG_IN | PULL_UP,						//  94 : SPI0_SCLK		-> SOC_BRD_ID0
	CFG_IN | PULL_UP,						//  95 : SPI0_MOSI		-> SOC_BRD_ID1

/* Port 12 */
	CFG_IN | PULL_UP,						//  96 : SPI0_MISO		-> SOC_BRD_ID2
	CFG_DISABLED,							//  97 : SPI0_SSIN		-> NC
	CFG_IN,								//  98 : PCIE_PERST0_N		-> PCIE0_RESET_L
	CFG_IN,								//  99 : PCIE_PERST1_N		-> PCIE1_RESET_L
	CFG_IN,								// 100 : PCIE_PERST2_N		-> PCIE2_RESET_L
	CFG_IN,								// 101 : PCIE_PERST3_N		-> PCIE3_RESET_L
	CFG_IN,								// 102 : PCIE_PERST4_N		-> PCIE4_RESET_L
	CFG_IN,								// 103 : PCIE_PERST5_N		-> PCIE5_RESET_L

/* Port 13 */
	CFG_FUNC0 | DRIVE_S4 | SLOW_SLEW,				// 104 : PCIE_CLKREQ0_N		-> PCIE0_CLKREQ_L
	CFG_FUNC0 | DRIVE_S4 | SLOW_SLEW,				// 105 : PCIE_CLKREQ1_N		-> PCIE1_CLKREQ_L
	CFG_FUNC0 | DRIVE_S4 | SLOW_SLEW,				// 106 : PCIE_CLKREQ2_N		-> PCIE2_CLKREQ_L
	CFG_FUNC0 | DRIVE_S4 | SLOW_SLEW,				// 107 : PCIE_CLKREQ3_N		-> PCIE3_CLKREQ_L
	CFG_FUNC0 | DRIVE_S4 | SLOW_SLEW,				// 108 : PCIE_CLKREQ4_N		-> PCIE4_CLKREQ_L
	CFG_DISABLED,							// 109 : PCIE_CLKREQ5_N		-> PCIE5_CLKREQ_L
	CFG_DISABLED,							// 110 : UART2_TXD		-> NC
	CFG_DISABLED,							// 111 : UART2_RXD		-> NC

/* Port 14 */
	CFG_DISABLED,							// 112 : UART2_RTSN		-> NC
	CFG_DISABLED,							// 113 : UART2_CTSN		-> NC
	CFG_FUNC0 | DRIVE_S4 | SLOW_SLEW,				// 114 : I2C3_SDA		-> I2C3_SDA
	CFG_FUNC0 | DRIVE_S4 | SLOW_SLEW,				// 115 : I2C3_SCL		-> I2C3_SCL
	CFG_DISABLED,							// 116 : GPIO[44]		-> NC
	CFG_IN,								// 117 : GPIO[45]		-> USB3CTRL_SOC_SMI_L
	CFG_DISABLED,							// 118 : GPIO[46]		-> NC
	CFG_OUT_0 | DRIVE_S4 | SLOW_SLEW,				// 119 : GPIO[47]		-> SOC_RDRVR_PD_L

/* Port 15 */
	CFG_DISABLED,							// 120 : GPIO[48]		-> NC
	CFG_IN | PULL_UP,						// 121 : GPIO[49]		-> USB3CTRL_SOC_WAKE_L
	CFG_DISABLED,							// 122 : UNSPECIFIED		-> UNSPECIFIED
	CFG_DISABLED,							// 123 : UNSPECIFIED		-> UNSPECIFIED
	CFG_DISABLED,							// 124 : UNSPECIFIED		-> UNSPECIFIED
	CFG_DISABLED,
	CFG_DISABLED,
	CFG_DISABLED,

/* Port 16 */
	CFG_DISABLED,							// 128 : SWD_TMS2		-> AOP_NAND_SWD_SWDIO
	CFG_DISABLED,							// 129 : SWD_TMS3		-> NC
	CFG_FUNC0 | DRIVE_S4 | SLOW_SLEW,				// 130 : UART5_RTXD		-> UART5_RTXD
	CFG_FUNC0 | DRIVE_S4 | SLOW_SLEW,				// 131 : I2C2_SDA		-> I2C2_SDA
	CFG_FUNC0 | DRIVE_S4 | SLOW_SLEW,				// 132 : I2C2_SCL		-> I2C2_SCL
	CFG_FUNC0 | DRIVE_S4 | SLOW_SLEW,				// 133 : UART4_TXD		-> UART4_TXD
	CFG_FUNC0,							// 134 : UART4_RXD		-> UART4_RXD
	CFG_FUNC0 | DRIVE_S4 | SLOW_SLEW,				// 135 : UART4_RTSN		-> UART4_RTS_L

/* Port 17 */
	CFG_FUNC0,							// 136 : UART4_CTSN		-> UART4_CTS_L
	CFG_FUNC0 | DRIVE_S4 | SLOW_SLEW,				// 137 : UART7_TXD		-> UART7_TXD
	CFG_FUNC0,							// 138 : UART7_RXD		-> UART7_RXD
	CFG_DISABLED,							// 139 : CLK32K_OUT		-> NC
	CFG_DISABLED,							// 140 : DP_WAKEUP0		-> NC
	CFG_FUNC0 | DRIVE_S4 | SLOW_SLEW,				// 141 : DP_WAKEUP1		-> SOC_USBCCTRL_AUX_OE
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
	CFG_DISABLED,							// 160 : SPI2_SCLK		-> NC
	CFG_DISABLED,							// 161 : SPI2_MOSI		-> NC
	CFG_DISABLED,							// 162 : SPI2_MISO		-> NC
	CFG_DISABLED,							// 163 : SPI2_SSIN		-> NC
	CFG_FUNC0 | DRIVE_S4 | SLOW_SLEW,				// 164 : I2C0_SDA		-> I2C0_SDA
	CFG_FUNC0 | DRIVE_S4 | SLOW_SLEW,				// 165 : I2C0_SCL		-> I2C0_SCL
	CFG_FUNC0 | DRIVE_S4 | SLOW_SLEW,				// 166 : SPI3_SCLK		-> SPI3_SCLK
	CFG_FUNC0 | DRIVE_S4 | SLOW_SLEW,				// 167 : SPI3_MOSI		-> SPI3_MOSI

/* Port 21 */
	CFG_FUNC0 | DRIVE_S4 | SLOW_SLEW,				// 168 : SPI3_MISO		-> SPI3_MISO
	CFG_FUNC0 | DRIVE_S4 | SLOW_SLEW,				// 169 : SPI3_SSIN		-> SPI3_SSIN
	CFG_FUNC0 | DRIVE_S4 | SLOW_SLEW,				// 170 : UART0_TXD		-> UART0_TXD
	CFG_FUNC0 | DRIVE_S4 | SLOW_SLEW,				// 171 : UART0_RXD		-> UART0_RXD
	CFG_DISABLED,							// 172 : UART6_TXD		-> NC
	CFG_DISABLED,							// 173 : UART6_RXD		-> NC
	CFG_IN,								// 174 : TMR32_PWM0		-> FAN_SOC_TACH
	CFG_IN,								// 175 : TMR32_PWM1		-> WLAN_SOC_ATSP

/* Port 22 */
	CFG_DISABLED,							// 176 : TMR32_PWM2		-> SOC_FAN_PWM
	CFG_FUNC0 | DRIVE_S4 | SLOW_SLEW,				// 177 : I2C1_SDA		-> I2C1_SDA
	CFG_FUNC0 | DRIVE_S4 | SLOW_SLEW,				// 178 : I2C1_SCL		-> I2C1_SCL
	CFG_DISABLED,							// 179 : GPIO[19]		-> NC
	CFG_IN | PULL_DOWN,						// 180 : GPIO[20]		-> SOC_GPIO_20
	CFG_DISABLED,							// 181 : GPIO[21]		-> SOC_GPU_ALTV_VID
	CFG_IN,								// 182 : GPIO[22]		-> GPU_SOC_PGOOD
	CFG_DISABLED,							// 183 : GPIO[23]		-> SOC_SIL_PT

/* Port 23 */
	CFG_DISABLED,							// 184 : GPIO[24]		-> NC
	CFG_IN | PULL_DOWN,						// 185 : GPIO[25]		-> SOC_BOOT_CFG1
	CFG_IN,								// 186 : GPIO[26]		-> SOC_FORCE_DFU
	CFG_FUNC0 | DRIVE_S7 | SLOW_SLEW,				// 187 : PSPI_MOSI		-> PMGR_SPI_MOSI
	CFG_DISABLED,							// 188 : DWI_DO			-> NC
	CFG_FUNC0 | DRIVE_S7 | SLOW_SLEW,				// 189 : PMGR_MISO		-> PMGR_SPI_MISO
	CFG_FUNC0 | DRIVE_S7 | SLOW_SLEW,				// 190 : PMGR_SCLK0		-> PMGR_SPI_CLK
	CFG_DISABLED,							// 191 : PMGR_SSCLK1		-> NC

/* Port 24 */
	CFG_FUNC0 | DRIVE_S4 | SLOW_SLEW,				// 192 : DROOP			-> COMP_SOC_DROOP_L
	CFG_FUNC0 | DRIVE_S4 | SLOW_SLEW,				// 193 : SOCHOT1		-> SOC_PMU_SOCHOT_L
	CFG_FUNC0 | DRIVE_S4 | SLOW_SLEW,				// 194 : EDP_HPD0		-> DP2HDMI_LPDP0_HPD
	CFG_FUNC0 | DRIVE_S4 | SLOW_SLEW,				// 195 : EDP_HPD1		-> USBCCTRL_SOC_RDRVR_HPD
	CFG_DISABLED,							// 196 : I2S3_MCK		-> NC
	CFG_DISABLED,							// 197 : I2S3_BCLK		-> NC
	CFG_DISABLED,							// 198 : I2S3_LRCK		-> NC
	CFG_DISABLED,							// 199 : I2S3_DOUT		-> NC

/* Port 25 */
	CFG_DISABLED,							// 200 : I2S3_DIN		-> NC
	CFG_DISABLED,							// 201 : GPIO[27]		-> SOC_DFU_STATUS
	CFG_IN | PULL_UP,						// 202 : GPIO[28]		-> SOC_BOOT_CFG2
	CFG_IN | PULL_UP,						// 203 : GPIO[29]		-> SOC_BRD_ID4
	CFG_DISABLED,							// 204 : GPIO[30]		-> SOC_SIL_CTRL1
	CFG_DISABLED,							// 205 : GPIO[31]		-> SOC_SIL_CTRL2
	CFG_DISABLED,							// 206 : GPIO[32]		-> NC
	CFG_DISABLED,							// 207 : GPIO[33]		-> SOC_SIL_CTRL0

/* Port 26 */
	CFG_IN | PULL_DOWN,						// 208 : GPIO[34]		-> SOC_BRD_REV0
	CFG_IN | PULL_DOWN,						// 209 : GPIO[35]		-> SOC_BRD_REV1
	CFG_IN | PULL_DOWN,						// 210 : GPIO[36]		-> SOC_BRD_REV2
	CFG_IN | PULL_DOWN,						// 211 : GPIO[37]		-> SOC_BRD_REV3
	CFG_DISABLED,							// 212 : GPIO[38]		-> NC
	CFG_DISABLED,							// 213 : GPIO[39]		-> NC
	CFG_DISABLED,							// 214 : GPIO[40]		-> NC
	CFG_DISABLED | PULL_DOWN,					// 215 : GPIO[41]		-> SOC_BT_DEVICE_WAKE

/* Port 27 */
	CFG_DISABLED,							// 216 : GPIO[42]		-> NC
	CFG_DISABLED,							// 217 : TST_CLKOUT		-> SOC_PMU_TEST_CLKOUT
	CFG_DISABLED,							// 218 : GPU_TRIGGER1		-> NC
	CFG_DISABLED,							// 219 : GPU_TRIGGER2		-> NC
	CFG_DISABLED,
	CFG_DISABLED,
	CFG_DISABLED,
	CFG_DISABLED,
};

static const uint32_t pinconfig_dev_1[GPIO_1_GROUP_COUNT * GPIOPADPINS] = {

/* Port  0 */
	CFG_FUNC0 | DRIVE_S4 | SLOW_SLEW,				//   0 : AOP_SPI_SCLK		-> AOP_SPI_SCLK
	CFG_FUNC0 | DRIVE_S4 | SLOW_SLEW,				//   1 : AOP_SPI_MOSI		-> AOP_SPI_MOSI
	CFG_FUNC0 | DRIVE_S4 | SLOW_SLEW,				//   2 : AOP_SPI_MISO		-> AOP_SPI_MISO
	CFG_DISABLED,							//   3 : AOP_UART1_TXD		-> NC
	CFG_DISABLED,							//   4 : AOP_UART1_RXD		-> NC
	CFG_FUNC0 | DRIVE_S4 | SLOW_SLEW,				//   5 : AOP_UART0_TXD		-> AOP_UART0_TXD
	CFG_FUNC0,							//   6 : AOP_UART0_RXD		-> AOP_UART0_RXD
	CFG_DISABLED,							//   7 : AOP_UART2_TXD		-> NC

/* Port  1 */
	CFG_DISABLED,							//   8 : AOP_UART2_RXD		-> NC
	CFG_FUNC0 | PULL_UP | DRIVE_S4 | SLOW_SLEW,			//   9 : AOP_I2CM_SDA		-> AOP_I2C_SDA
	CFG_FUNC0 | PULL_UP | DRIVE_S4 | SLOW_SLEW,			//  10 : AOP_I2CM_SCL		-> AOP_I2C_SCL
	CFG_IN,								//  11 : AOP_FUNC[0]		-> AOP_DFU_REQ_L
	CFG_DISABLED,							//  12 : AOP_FUNC[1]		-> AOP_DFU_REQCLR
	CFG_IN,								//  13 : AOP_FUNC[2]		-> DP2HDMI_AOP_VDD12ON
	CFG_IN,								//  14 : AOP_FUNC[3]		-> AOP_FUNC_3
	CFG_IN,								//  15 : AOP_FUNC[4]		-> HDMI_AOP_HPD

/* Port  2 */
	CFG_DISABLED,							//  16 : AOP_FUNC[5]		-> AOP_HDMI_CEC
	CFG_DISABLED,							//  17 : AOP_FUNC[6]		-> IRRCVR_OUT_RC_1V8
	CFG_DISABLED,							//  18 : AOP_FUNC[7]		-> IRRCVR_OUT_RC_1V8
	CFG_IN,								//  19 : AOP_FUNC[8]		-> DP2HDMI_PMU_CEC_IRQ
	CFG_DISABLED,							//  20 : AOP_FUNC[9]		-> HDMI_AOP_CEC
	CFG_IN,								//  21 : AOP_SWD_TCK_OUT	-> AOP_SWD_SWCLK
	CFG_DISABLED,							//  22 : AOP_SWD_TMS0		-> NC
	CFG_DISABLED,							//  23 : AOP_SWD_TMS1		-> NC

/* Port  3 */
	CFG_DISABLED,							//  24 : AOP_I2S_MCK		-> AOP_I2S_MCK
	CFG_DISABLED,							//  25 : AOP_I2S_BCLK		-> AOP_I2S_BCLK
	CFG_DISABLED,							//  26 : AOP_I2S_LRCK		-> AOP_I2S_LRCK
	CFG_DISABLED,							//  27 : AOP_I2S_DIN		-> AOP_I2S_DIN
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
	{ 0, 1, { pinconfig_ap_0, pinconfig_ap_1 } },
	{ 1, 1, { pinconfig_dev_0, pinconfig_dev_1 } },
};

const uint32_t * target_get_default_gpio_cfg(uint32_t gpioc)
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
