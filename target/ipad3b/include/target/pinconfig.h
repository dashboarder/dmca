/*
 * Copyright (C) 2011-2012 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */

/* Default P101/P102/P103 SoC Pin Configuration
   
   Based on I/O spreadsheet rev2p3
   <rdar://problem/10363068> X140 IO Spreadsheet tracker
 
   Do not modify without first requesting an update to the I/O spreadsheet
 */

#define		CFG_RESERVED		CFG_DISABLED


/* Different drive strength for AP and DEV. */
#if PIN_CFG_AP
#define AP_DEV_DRIVE_STR(ap, dev) DRIVE_##ap
#elif PIN_CFG_DEV
#define AP_DEV_DRIVE_STR(ap, dev) DRIVE_##dev
#endif

#if TARGET_HAS_BASEBAND
#	define _IFF_BB(cfg)  cfg
#else
#	define _IFF_BB(cfg)  CFG_DISABLED
#endif

#if PIN_CFG_AP
static const u_int32_t gpio_default_cfg_ap[GPIO_GROUP_COUNT * GPIOPADPINS] = {
#elif PIN_CFG_DEV
static const u_int32_t gpio_default_cfg_dev[GPIO_GROUP_COUNT * GPIOPADPINS] = {
#endif

/*
 *	config								   idx	pad			   net				notes
 *	========							   ===	========		   ================		========================
 */

/* Port  0 */
	CFG_IN,								//   0: GPIO0			-> GPIO_BTN_HOME_L		also connected to PMU
	CFG_IN,								//   1: GPIO1			-> GPIO_BTN_ONOFF_L		also connected to PMU
	_IFF_BB(CFG_IN | PULL_DOWN),					//   2: GPIO2			-> GPIO_BB_HSIC_DEV_RDY		baseband models only
	CFG_IN | PULL_UP,						//   3: GPIO3			-> GPIO_BTN_VOL_UP_L
	CFG_IN | PULL_UP,						//   4: GPIO4			-> GPIO_ALS_IRQ_L
	CFG_OUT_0 | AP_DEV_DRIVE_STR(X2, X4),				//   5: GPIO5			-> GPIO_BT_WAKE
	_IFF_BB(CFG_OUT_0 | AP_DEV_DRIVE_STR(X2, X4)),			//   6: GPIO6			-> GPIO_AP_MODEM_WAKE		baseband models only
	CFG_DISABLED | DRIVE_X2,					//   7: GPIO7			-> BB_JTAG_TMS_RF

/* Port  1 */
	_IFF_BB(CFG_IN | PULL_DOWN),					//   8: GPIO10			-> GPIO_BB_RESET_DET_L		baseband models only
	CFG_IN | PULL_UP,						//   9: GPIO11			-> GPIO_ACCEL_IRQ2_L
	_IFF_BB(CFG_IN | PULL_DOWN),					//  10: GPIO12			-> GPIO_BB_HSIC_RESUME		baseband models only
	CFG_OUT_0 | AP_DEV_DRIVE_STR(X2, X4),				//  11: GPIO13			-> GPIO_WLAN_HSIC_HOST_RDY
	CFG_IN | PULL_UP,						//  12: GPIO14			-> GPIO_BTN_VOL_DOWN_L
	_IFF_BB(CFG_IN),						//  13: GPIO15			-> GPIO_BB_GSM_TXBURST		baseband models only
	CFG_RESERVED,							//  14: (reserved)
	CFG_RESERVED,							//  15: (reserved)

/* Port  2 */
	CFG_RESERVED,							//  16: (reserved)
	CFG_RESERVED,							//  17: (reserved)
	CFG_RESERVED,							//  18: (reserved)
	CFG_RESERVED,							//  19: (reserved)
	CFG_RESERVED,							//  20: (reserved)
	CFG_RESERVED,							//  21: (reserved)
	CFG_RESERVED,							//  22: (reserved)
	CFG_RESERVED,							//  23: (reserved)

/* Port  3 */
	CFG_RESERVED,							//  24: (reserved)
	CFG_RESERVED,							//  25: (reserved)
	CFG_RESERVED,							//  26: (reserved)
	CFG_RESERVED,							//  27: (reserved)
	CFG_RESERVED,							//  28: (reserved)
	CFG_RESERVED,							//  29: (reserved)
	CFG_RESERVED,							//  30: (reserved)
	CFG_RESERVED,							//  31: (reserved)

/* Port  4 */
	_IFF_BB(CFG_IN | DRIVE_X2),					//  32: GPIO8			-> GPIO_BB_RST_L		baseband models only
	_IFF_BB(CFG_IN | AP_DEV_DRIVE_STR(X2, X4)),			//  33: GPIO9			-> GPIO_BB_RADIO_ON		baseband models only
	CFG_DISABLED,							//  34: EHCI_PORT_PWR0		-> GPIO_BRD_REV0
	CFG_DISABLED,							//  35: EHCI_PORT_PWR1		-> GPIO_BRD_REV1
	CFG_DISABLED,							//  36: EHCI_PORT_PWR2		-> GPIO_BRD_REV2
	CFG_DISABLED,							//  37: EHCI_PORT_PWR3		-> NC_EHCI_PORT_PWR3_AP
	CFG_DISABLED,							//  38: GPIO16			-> NC_GPIO_BOARD_ID_3
	CFG_IN,								//  39: GPIO17			-> PMU_GPIO_TS_INT

/* Port  5 */
	CFG_DISABLED,							//  40: GPIO18			-> GPIO_BOOT_CONFIG_0
	_IFF_BB(CFG_IN | DRIVE_X2),					//  41: GPIO19			-> GPIO_BB_GPS_SYNC		baseband models only
	_IFF_BB(CFG_IN | AP_DEV_DRIVE_STR(X2, X6)),			//  42: UART1_TXD		-> UART1_BB_TXD			baseband models only
	_IFF_BB(CFG_FUNC0),						//  43: UART1_RXD		-> UART1_BB_RXD			baseband models only
	_IFF_BB(CFG_IN | AP_DEV_DRIVE_STR(X2, X6)),			//  44: UART1_RTSN		-> UART1_BB_RTS_L		baseband models only
	_IFF_BB(CFG_FUNC0),						//  45: UART1_CTSN		-> UART1_BB_CTS_L		baseband models only
	CFG_DISABLED,							//  46: FMI0_CEN3		-> NC_FMI0_CE3_L
	CFG_DISABLED,							//  47: FMI0_CEN2		-> NC_FMI0_CE2_L

/* Port  6 */
	CFG_DISABLED,							//  48: FMI0_CEN1		-> NC_FMI0_CE1_L
	CFG_FUNC0 | DRIVE_X2 | SLOW_SLEW,				//  49: FMI0_CEN0		-> FMI0_CE0_L
	CFG_FUNC0 | DRIVE_X2 | SLOW_SLEW,				//  50: FMI0_CLE		-> FMI0_CLE
	CFG_FUNC0 | DRIVE_X2 | SLOW_SLEW,				//  51: FMI0_ALE		-> FMI0_ALE
	CFG_FUNC0 | DRIVE_X2 | SLOW_SLEW,				//  52: FMI0_REN		-> FMI0_RE_L
	CFG_FUNC0 | DRIVE_X2 | SLOW_SLEW,				//  53: FMI0_WEN		-> FMI0_WE_L
	CFG_DISABLED,							//  54: FMI0_WENN		-> NC_FMI0_RE
	CFG_FUNC0 | BUS_HOLD | DRIVE_X2 | SLOW_SLEW,			//  55: FMI0_IO7		-> FMI0_AD<7>

/* Port  7 */
	CFG_FUNC0 | BUS_HOLD | DRIVE_X2 | SLOW_SLEW,			//  56: FMI0_IO6		-> FMI0_AD<6>
	CFG_FUNC0 | BUS_HOLD | DRIVE_X2 | SLOW_SLEW,			//  57: FMI0_IO5		-> FMI0_AD<5>
	CFG_FUNC0 | BUS_HOLD | DRIVE_X2 | SLOW_SLEW,			//  58: FMI0_IO4		-> FMI0_AD<4>
	CFG_FUNC0 | BUS_HOLD | DRIVE_X2 | SLOW_SLEW,			//  59: FMI0_DQS		-> FMI0_DQS
	CFG_DISABLED,							//  60: FMI0_DQSN		-> NC_FMI0_DQSN
	CFG_FUNC0 | BUS_HOLD | DRIVE_X2 | SLOW_SLEW,			//  61: FMI0_IO3		-> FMI0_AD<3>
	CFG_FUNC0 | BUS_HOLD | DRIVE_X2 | SLOW_SLEW,			//  62: FMI0_IO2		-> FMI0_AD<2>
	CFG_FUNC0 | BUS_HOLD | DRIVE_X2 | SLOW_SLEW,			//  63: FMI0_IO1		-> FMI0_AD<1>

/* Port  8 */
	CFG_FUNC0 | BUS_HOLD | DRIVE_X2 | SLOW_SLEW,			//  64: FMI0_IO0		-> FMI0_AD<0>
	CFG_DISABLED,							//  65: FMI0_CEN7		-> NC_FMI0_CE7_L
	CFG_DISABLED,							//  66: FMI0_CEN6		-> NC_FMI0_CE6_L
	CFG_DISABLED,							//  67: FMI0_CEN5		-> NC_FMI0_CE5_L
	CFG_DISABLED,							//  68: FMI0_CEN4		-> NC_FMI0_CE4_L
	CFG_DISABLED,							//  69: FMI1_CEN3		-> NC_FMI1_CE3_L
	CFG_DISABLED,							//  70: FMI1_CEN2		-> NC_FMI1_CE2_L
	CFG_DISABLED,							//  71: FMI1_CEN1		-> NC_FMI1_CE1_L

/* Port  9 */
	CFG_FUNC0 | DRIVE_X2 | SLOW_SLEW,				//  72: FMI1_CEN0		-> FMI1_CE0_L
	CFG_FUNC0 | DRIVE_X2 | SLOW_SLEW,				//  73: FMI1_CLE		-> FMI1_CLE
	CFG_FUNC0 | DRIVE_X2 | SLOW_SLEW,				//  74: FMI1_ALE		-> FMI1_ALE
	CFG_FUNC0 | DRIVE_X2 | SLOW_SLEW,				//  75: FMI1_REN		-> FMI1_RE_L
	CFG_FUNC0 | DRIVE_X2 | SLOW_SLEW,				//  76: FMI1_WEN		-> FMI1_WE_L
	CFG_DISABLED,							//  77: FMI1_WENN		-> 
	CFG_FUNC0 | BUS_HOLD | DRIVE_X2 | SLOW_SLEW,			//  78: FMI1_IO7		-> FMI1_AD<7>
	CFG_FUNC0 | BUS_HOLD | DRIVE_X2 | SLOW_SLEW,			//  79: FMI1_IO6		-> FMI1_AD<6>

/* Port 10 */
	CFG_FUNC0 | BUS_HOLD | DRIVE_X2 | SLOW_SLEW,			//  80: FMI1_IO5		-> FMI1_AD<5>
	CFG_FUNC0 | BUS_HOLD | DRIVE_X2 | SLOW_SLEW,			//  81: FMI1_IO4		-> FMI1_AD<4>
	CFG_FUNC0 | BUS_HOLD | DRIVE_X2 | SLOW_SLEW,			//  82: FMI1_DQS		-> FMI1_DQS
	CFG_DISABLED,							//  83: FMI1_DQSN		-> FMI1_DQSN
	CFG_FUNC0 | BUS_HOLD | DRIVE_X2 | SLOW_SLEW,			//  84: FMI1_IO3		-> FMI1_AD<3>
	CFG_FUNC0 | BUS_HOLD | DRIVE_X2 | SLOW_SLEW,			//  85: FMI1_IO2		-> FMI1_AD<2>
	CFG_FUNC0 | BUS_HOLD | DRIVE_X2 | SLOW_SLEW,			//  86: FMI1_IO1		-> FMI1_AD<1>
	CFG_FUNC0 | BUS_HOLD | DRIVE_X2 | SLOW_SLEW,			//  87: FMI1_IO0		-> FMI1_AD<0>

/* Port 11 */
	CFG_DISABLED,							//  88: FMI1_CEN7		-> NC_FMI1_CE7_L
	CFG_DISABLED,							//  89: FMI1_CEN6		-> NC_FMI1_CE6_L
	CFG_DISABLED,							//  90: FMI1_CEN5		-> NC_FMI1_CE5_L
	CFG_DISABLED,							//  91: FMI1_CEN4		-> NC_FMI1_CE4_L
	CFG_FUNC0 | AP_DEV_DRIVE_STR(X2, X6),				//  92: UART2_TXD		-> UART2_TS_ACC_TXD
	CFG_FUNC0 | PULL_UP,						//  93: UART2_RXD		-> UART2_TS_ACC_RXD
	CFG_DISABLED,							//  94: UART2_RTSN		-> NC_UART2_RTSN
	CFG_DISABLED | DRIVE_X2,					//  95: UART2_CTSN		-> BB_JTAG_TCK_RF

/* Port 12 */
	CFG_FUNC0 | AP_DEV_DRIVE_STR(X2, X6),				//  96: UART3_TXD		-> UART3_BT_TXD
	CFG_FUNC0 | PULL_UP,						//  97: UART3_RXD		-> UART3_BT_RXD
	CFG_OUT_1 | AP_DEV_DRIVE_STR(X2, X6),				//  98: UART3_RTSN		-> UART3_BT_RTS_L
	CFG_FUNC0 | PULL_UP,						//  99: UART3_CTSN		-> UART3_BT_CTS_L
	CFG_FUNC0 | AP_DEV_DRIVE_STR(X2, X6),				// 100: UART4_TXD		-> UART4_WLAN_TXD
	CFG_FUNC0 | PULL_UP,						// 101: UART4_RXD		-> UART4_WLAN_RXD
	CFG_DISABLED,							// 102: UART4_RTSN		-> NC_UART4_RTS_L
	CFG_DISABLED,							// 103: UART4_CTSN		-> NC_UART4_CTS_L

/* Port 13 */
	CFG_FUNC0 | AP_DEV_DRIVE_STR(X2, X6),				// 104: UART6_TXD		-> UART6_AP_TXD
	CFG_FUNC0 | PULL_UP,						// 105: UART6_RXD		-> UART6_AP_RXD
	CFG_DISABLED,							// 106: UART6_RTSN		-> NC_UART6_RTSN
	CFG_DISABLED,							// 107: UART6_CTSN		-> NC_UART6_CTSN
	CFG_FUNC0 | AP_DEV_DRIVE_STR(X4, X6),				// 108: SPI1_SCLK		-> SPI1_CODEC_SCLK
	CFG_FUNC0 | AP_DEV_DRIVE_STR(X4, X6),				// 109: SPI1_MOSI		-> SPI1_CODEC_MOSI
	CFG_FUNC0,							// 110: SPI1_MISO		-> SPI1_CODEC_MISO
	CFG_OUT_1 | AP_DEV_DRIVE_STR(X4, X6),				// 111: SPI1_SSIN		-> SPI1_CODEC_CS_L

/* Port 14 */
	CFG_DISABLED,							// 112: SPI0_SCLK		-> GPIO_BOARD_ID_0
	CFG_DISABLED,							// 113: SPI0_MOSI		-> GPIO_BOARD_ID_1
	CFG_DISABLED,							// 114: SPI0_MISO		-> GPIO_BOARD_ID_2
	CFG_DISABLED,							// 115: SPI0_SSIN		-> TP_SPI0_SSIN
	CFG_DISABLED,							// 116: SPI2_SCLK		-> BB_JTAG_TDO_RF
	CFG_DISABLED | DRIVE_X2,					// 117: SPI2_MOSI		-> BB_JTAG_TDI_RF
	CFG_DISABLED | DRIVE_X2,					// 118: SPI2_MISO		-> BB_JTAG_TRST_RF_L
	_IFF_BB(CFG_OUT_0 | AP_DEV_DRIVE_STR(X2, X4)),			// 119: SPI2_SSIN		-> GPIO_BB_HSIC_HOST_RDY	baseband models only

/* Port 15 */
	CFG_FUNC0 | AP_DEV_DRIVE_STR(X4, X6),				// 120: I2C0_SDA		-> I2C0_SDA_1V8
	CFG_FUNC0 | AP_DEV_DRIVE_STR(X4, X6),				// 121: I2C0_SCL		-> I2C0_SCL_1V8
	CFG_FUNC0 | AP_DEV_DRIVE_STR(X2, X6),				// 122: I2C1_SDA		-> I2C1_SDA_1V8
	CFG_FUNC0 | AP_DEV_DRIVE_STR(X2, X6),				// 123: I2C1_SCL		-> I2C1_SCL_1V8
	CFG_FUNC0 | AP_DEV_DRIVE_STR(X4, X6),				// 124: ISP0_SDA		-> ISP0_CAM_RF_I2C_SDA
	CFG_FUNC0 | AP_DEV_DRIVE_STR(X4, X6),				// 125: ISP0_SCL		-> ISP0_CAM_RF_I2C_SCL
	CFG_FUNC0 | AP_DEV_DRIVE_STR(X4, X6),				// 126: ISP1_SDA		-> ISP1_CAM_FF_I2C_SDA
	CFG_FUNC0 | AP_DEV_DRIVE_STR(X4, X6),				// 127: ISP1_SCL		-> ISP1_CAM_FF_I2C_SCL

/* Port 16 */
	CFG_DISABLED,							// 128: MIPI_VSYNC		-> NC_MIPI_VSYNC_H5
	CFG_FUNC0 | PULL_DOWN,						// 129: TMR32_PWM0		-> GPIO_GYRO_IRQ2
	CFG_FUNC0 | PULL_UP,						// 130: TMR32_PWM1		-> GPIO_ACCEL_IRQ1_L
	CFG_DISABLED,							// 131: TMR32_PWM2		-> NC_TMR32_PWM2_AP
	CFG_DISABLED,							// 132: SWI_DATA		-> NC_SWI_AP
	CFG_FUNC0 | PULL_DOWN,						// 133: DWI_DI			-> DWI_AP_DI
	CFG_FUNC0 | AP_DEV_DRIVE_STR(X4, X6),				// 134: DWI_DO			-> DWI_AP_DO
	CFG_FUNC0 | AP_DEV_DRIVE_STR(X4, X6),				// 135: DWI_CLK			-> DWI_AP_CLK

/* Port 17 */
	CFG_OUT_1 | DRIVE_X2,						// 136: SENSOR0_RST		-> ISP0_CAM_RF_SHUTDOWN
	CFG_OUT_0 | AP_DEV_DRIVE_STR(X4, X6),				// 137: SENSOR0_CLK		-> ISP0_CAM_RF_CLK_R
	CFG_OUT_0 | PULL_DOWN | DRIVE_X2,				// 138: SENSOR1_RST		-> ISP1_CAM_FF_SHUTDOWN_L
	CFG_OUT_0 | AP_DEV_DRIVE_STR(X4, X6),				// 139: SENSOR1_CLK		-> ISP1_CAM_FF_CLK_R
	CFG_RESERVED,							// 140: (reserved)
	CFG_RESERVED,							// 141: (reserved)
	CFG_RESERVED,							// 142: (reserved)
	CFG_RESERVED,							// 143: (reserved)

/* Port 18 */
	CFG_RESERVED,							// 144: (reserved)
	CFG_RESERVED,							// 145: (reserved)
	CFG_RESERVED,							// 146: (reserved)
	CFG_RESERVED,							// 147: (reserved)
	CFG_RESERVED,							// 148: (reserved)
	CFG_RESERVED,							// 149: (reserved)
	CFG_RESERVED,							// 150: (reserved)
	CFG_RESERVED,							// 151: (reserved)

/* Port 19 */
	CFG_RESERVED,							// 152: (reserved)
	CFG_RESERVED,							// 153: (reserved)
	CFG_RESERVED,							// 154: (reserved)
	CFG_RESERVED,							// 155: (reserved)
	CFG_RESERVED,							// 156: (reserved)
	CFG_RESERVED,							// 157: (reserved)
	CFG_RESERVED,							// 158: (reserved)
	CFG_RESERVED,							// 159: (reserved)

/* Port 20 */
	CFG_FUNC0 | DRIVE_X4,						// 160: CPU0_SWITCH		-> CPU0_SWITCH
	CFG_FUNC0 | DRIVE_X4,						// 161: CPU1_SWITCH		-> CPU1_SWITCH
	CFG_DISABLED,							// 162: ISP0_PRE_FLASH		-> NC_ISP0_CAM_RF_FLASH
	CFG_OUT_0 | DRIVE_X4,						// 163: ISP0_FLASH		-> ISP0_CAM_RF_RST_L
	CFG_DISABLED | DRIVE_X4,					// 164: ISP1_PRE_FLASH		-> SOCHOT0_L
	CFG_FUNC1 | DRIVE_X4,						// 165: ISP1_FLASH		-> SOCHOT1_L
	CFG_FUNC0 | AP_DEV_DRIVE_STR(X4, X6),				// 166: I2S0_MCK		-> I2S0_CODEC_ASP_MCK
	CFG_FUNC0 | AP_DEV_DRIVE_STR(X4, X6),				// 167: I2S0_LRCK		-> I2S0_CODEC_ASP_LRCK

/* Port 21 */
	CFG_FUNC0 | AP_DEV_DRIVE_STR(X4, X6),				// 168: I2S0_BCLK		-> I2S0_CODEC_ASP_BCLK
	CFG_FUNC0 | AP_DEV_DRIVE_STR(X4, X6),				// 169: I2S0_DOUT		-> I2S0_CODEC_ASP_DOUT
	CFG_FUNC0,							// 170: I2S0_DIN		-> I2S0_CODEC_ASP_DIN
	CFG_FUNC0 | DRIVE_X2,						// 171: I2S1_MCK		-> I2S1_SPKAMP_MCK
	CFG_FUNC0 | DRIVE_X2,						// 172: I2S1_LRCK		-> I2S1_SPKAMP_LRCK
	CFG_FUNC0 | DRIVE_X2,						// 173: I2S1_BCLK		-> I2S1_SPKAMP_BCLK
	CFG_FUNC0 | DRIVE_X2,						// 174: I2S1_DOUT		-> I2S1_SPKAMP_DOUT
	CFG_FUNC0,							// 175: I2S1_DIN		-> I2S1_SPKAMP_DIN

/* Port 22 */
	CFG_DISABLED,							// 176: I2S2_MCK		-> NC_I2S2_MCK
	CFG_FUNC0 | AP_DEV_DRIVE_STR(X4, X6),				// 177: I2S2_LRCK		-> I2S2_BT_LRCK
	CFG_FUNC0 | AP_DEV_DRIVE_STR(X4, X6),				// 178: I2S2_BCLK		-> I2S2_BT_BCLK
	CFG_FUNC0 | AP_DEV_DRIVE_STR(X4, X6),				// 179: I2S2_DOUT		-> I2S2_BT_DOUT
	CFG_FUNC0,							// 180: I2S2_DIN		-> I2S2_BT_DIN
	CFG_DISABLED,							// 181: I2S3_MCK		-> NC_I2S3_MCK
	CFG_FUNC0 | AP_DEV_DRIVE_STR(X4, X6),				// 182: I2S3_LRCK		-> I2S3_CODEC_XSP_LRCK
	CFG_FUNC0 | AP_DEV_DRIVE_STR(X4, X6),				// 183: I2S3_BCLK		-> I2S3_CODEC_XSP_BCLK

/* Port 23 */
	CFG_FUNC0 | AP_DEV_DRIVE_STR(X4, X6),				// 184: I2S3_DOUT		-> I2S3_CODEC_XSP_DOUT
	CFG_FUNC0,							// 185: I2S3_DIN		-> I2S3_CODEC_XSP_DIN
	CFG_DISABLED,							// 186: I2S4_MCK		-> NC_I2S4_MCK
	CFG_DISABLED,							// 187: I2S4_LRCK		-> NC_I2S4_LRCK
	CFG_DISABLED,							// 188: I2S4_BCLK		-> NC_I2S4_BCLK
	CFG_DISABLED,							// 189: I2S4_DOUT		-> NC_I2S4_DOUT
	CFG_DISABLED,							// 190: I2S4_DIN		-> NC_I2S4_DIN
	CFG_DISABLED,							// 191: SPDIF			-> NC_AP_GPIO216

/* Port 24 */
	CFG_IN,								// 192: GPIO20			-> GPIO_PROX_IRQ_L
	CFG_IN | PULL_DOWN,						// 193: GPIO21			-> GPIO_GYRO_IRQ1
	CFG_OUT_0 | DRIVE_X2,						// 194: GPIO22			-> GPIO_PMU_KEEPACT
	CFG_IN | PULL_UP,						// 195: GPIO23			-> GPIO_PMU_IRQ_L
	CFG_IN | PULL_DOWN,						// 196: GPIO24			-> GPIO_WLAN_HSIC_DEV_RDY
	CFG_DISABLED,							// 197: GPIO25			-> GPIO_BOOT_CONFIG_1
	CFG_IN | PULL_DOWN,						// 198: GPIO26			-> GPIO_FORCE_DFU
	CFG_OUT_0 | PULL_DOWN | DRIVE_X1,				// 199: GPIO27			-> GPIO_DFU_STATUS

/* Port 25 */
	CFG_DISABLED,							// 200: GPIO28			-> GPIO_BOOT_CONFIG_2
	CFG_DISABLED,							// 201: GPIO29			-> GPIO_BOOT_CONFIG_3
	CFG_IN,								// 202: GPIO30			-> GPIO_BTN_SRL_L		a.k.a. ringer switch
	CFG_IN | PULL_UP,						// 203: GPIO31			-> GPIO_GRAPE_IRQ_L
	CFG_IN | PULL_DOWN,						// 204: GPIO32			-> GPIO_WL_HSIC_RESUME
	CFG_OUT_0 | DRIVE_X2,						// 205: GPIO33			-> GPIO_SPKAMP_RST_L
	CFG_OUT_0 | DRIVE_X2,						// 206: GPIO34			-> GPIO_SPKAMP_KEEPALIVE
	CFG_IN | PULL_UP,						// 207: GPIO35			-> GPIO_SPKAMP_RIGHT_IRQ_L

/* Port 26 */
	CFG_OUT_0 | DRIVE_X2,						// 208: GPIO36			-> PM_LCDVDD_PWREN
	CFG_IN | PULL_UP,						// 209: GPIO37			-> GPIO_SPKAMP_LEFT_IRQ_L
	CFG_IN | PULL_UP,						// 210: GPIO38			-> SPK_ID
	CFG_IN | PULL_UP,						// 211: GPIO39			-> GPIO_CODEC_IRQ_L
	CFG_RESERVED,							// 212: (reserved)
	CFG_RESERVED,							// 213: (reserved)
	CFG_RESERVED,							// 214: (reserved)
	CFG_RESERVED,							// 215: (reserved)

/* Port 27 */
	CFG_RESERVED,							// 216: (reserved)
	CFG_RESERVED,							// 217: (reserved)
	CFG_RESERVED,							// 218: (reserved)
	CFG_RESERVED,							// 219: (reserved)
	CFG_RESERVED,							// 220: (reserved)
	CFG_RESERVED,							// 221: (reserved)
	CFG_RESERVED,							// 222: (reserved)
	CFG_RESERVED,							// 223: (reserved)

/* Port 28 */
	CFG_FUNC0 | DRIVE_X4,						// 224: SPI3_MOSI		-> SPI3_GRAPE_MOSI
	CFG_FUNC0,							// 225: SPI3_MISO		-> SPI3_GRAPE_MISO
	CFG_FUNC0 | DRIVE_X4,						// 226: SPI3_SCLK		-> SPI3_GRAPE_SCLK
	CFG_OUT_1 | AP_DEV_DRIVE_STR(X4, X6),				// 227: SPI3_SSIN		-> SPI3_GRAPE_CS_L
	CFG_FUNC0 | AP_DEV_DRIVE_STR(X4, X6),				// 228: I2C2_SDA		-> I2C2_SDA_3V0
	CFG_FUNC0 | AP_DEV_DRIVE_STR(X4, X6),				// 229: I2C2_SCL		-> I2C2_SCL_3V0
	CFG_OUT_1 | DRIVE_X2,						// 230: GPIO_3V0		-> GPIO_GRAPE_FW_DNLD_EN_L
	CFG_OUT_0 | DRIVE_X2,						// 231: GPIO_3V1		-> GPIO_GRAPE_RST_L

/* Port 29 */
	CFG_DISABLED,							// 232: DP_HPD			-> NC_DP_HPD
	CFG_FUNC0,							// 233: EDP_HPD			-> EDP_HPD
	CFG_DISABLED,							// 234: UART0_TXD		-> NC_UART0_TXD
	CFG_DISABLED,							// 235: UART0_RXD		-> NC_UART0_RXD
	CFG_FUNC1 | AP_DEV_DRIVE_STR(X1, X6),				// 236: UART5_RXD		-> UART5_BATTERY_TRXD
	CFG_DISABLED,							// 237: UART5_TXD		-> NC_UART5_TXD
	CFG_DISABLED,							// 238: TST_CLKOUT		-> TP_AP_TST_CLKOUT
	CFG_IN,								// 239: TST_STPCLK		-> AP_TST_STPCLK

/* Port 30 */
	CFG_FUNC0 | DRIVE_X2,						// 240: WDOG			-> AP_WDOG
	CFG_RESERVED,							// 241: (reserved)
	CFG_RESERVED,							// 242: (reserved)
	CFG_RESERVED,							// 243: (reserved)
	CFG_RESERVED,							// 244: (reserved)
	CFG_RESERVED,							// 245: (reserved)
	CFG_RESERVED,							// 246: (reserved)
	CFG_RESERVED,							// 247: (reserved)
};

#undef AP_DEV_DRIVE_STR
#undef _IFF_BB
