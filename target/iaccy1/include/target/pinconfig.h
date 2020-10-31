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
#ifndef __TARGET_PINCONFIG_H
#define __TARGET_PINCONFIG_H

/* Default iAccy1 Hardware Pin Configuration */

/* Updated to match B137B165_io_pinlist_013.xls */

#if SUB_TARGET_B137
#define PCON_B137_B165(x, y) (PCON_##x)
#elif SUB_TARGET_B165
#define PCON_B137_B165(x, y) (PCON_##y)
#else
#error "Unknown subtarget"
#endif

static const struct gpio_default_config gpio_default_config[GPIO_GROUP_COUNT] = {
/* P0  */  PINCONFIG(
		PCON_OUT_0,			NO_PUPDN,	// I2S0_MCK ->		FORCE_PWM_3V3_L
		PCON_OUT_1,			NO_PUPDN,	// I2S0_LRCK ->		5V_EN
		PCON_IN,			NO_PUPDN,	// I2S0_BCK ->		TRI_INT
		PCON_B137_B165(DISABLE, OUT_1),	NO_PUPDN,	// I2S0_DOUT ->		FORCE_PWM_1V8
		PCON_OUT_1,			NO_PUPDN,	// I2S0_DIN ->		FORCE_PWM_1V2
		PCON_FUNC2,			NO_PUPDN,	// UART0_RXD ->
		PCON_FUNC2,			NO_PUPDN,	// UART0_TXD ->
		PCON_B137_B165(DISABLE, IN),	NO_PUPDN	// UART0_NRTS ->	DAC_SENSE_STS
	       ),
/* P1  */  PINCONFIG(
		PCON_B137_B165(DISABLE, IN),	NO_PUPDN,	// UART0_NCTS ->	DAC_HEARTBEAT
		PCON_FUNC2,			NO_PUPDN,	// I2C0_SCL ->		I2C0_SCL
		PCON_FUNC2,			NO_PUPDN,	// I2C0_SDA ->		I2C0_SCL
		PCON_DISABLE,			NO_PUPDN,	// SPI0_CEN ->		SPI0_CS
		PCON_DISABLE,			NO_PUPDN,	// SPI0_CLK ->		SPI0_CLK
		PCON_DISABLE,			NO_PUPDN,	// SPI0_MOSI ->		SPI0_MOSI
		PCON_DISABLE,			NO_PUPDN,	// SPI0_MISO ->		SPI0_MISO
		PCON_DISABLE,			NO_PUPDN	// - ->
	       ),
/* P2  */  PINCONFIG(
		PCON_DISABLE,			NO_PUPDN,	// GPIO0 ->		BOARD_REV[0]
		PCON_DISABLE,			NO_PUPDN,	// GPIO1 ->		BOOT_CONFIG[0]
		PCON_DISABLE,			NO_PUPDN,	// GPIO2 ->		BOOT_CONFIG[1]
		PCON_DISABLE,			NO_PUPDN,	// GPIO3 ->		BOARD_ID[1]
		PCON_DISABLE,			NO_PUPDN,	// GPIO4 ->		BOARD_ID[2]
		PCON_DISABLE,			NO_PUPDN,	// GPIO5 ->		BOARD_ID[3]
		PCON_DISABLE,			NO_PUPDN,	// GPIO6 ->		BOARD_ID[4]
		PCON_DISABLE,			NO_PUPDN	// GPIO7 ->		BOARD_REV[1]
	       ),
/* P3  */  PINCONFIG(
		PCON_DISABLE,			NO_PUPDN,	// CLK0_OUT ->		G1_CLK0
		PCON_DISABLE,			NO_PUPDN,	// CLK1_OUT ->
		PCON_B137_B165(FUNC2, DISABLE),	NO_PUPDN,	// HDMI_HPD ->		G1_HPD
		PCON_DISABLE,			NO_PUPDN,	// HDMI_CEC ->		G1_HDMI_CEC
		PCON_DISABLE,			NO_PUPDN,	// FORCE_DFU ->		FORCE_DFU
		PCON_DISABLE,			NO_PUPDN,	// DFU_STATUS ->	DFU_STATUS
		PCON_FUNC2,			NO_PUPDN,	// I2C1_SCL ->		G1_I2C1_SCL
		PCON_FUNC2,			NO_PUPDN	// I2C1_SDA ->		G1_I2C1_SDA
	       ),
/* P4  */  PINCONFIG(
		PCON_FUNC2,			NO_PUPDN,	// UART2_RXD ->		G1_UART_RXFROMPOD
		PCON_FUNC2,			NO_PUPDN,	// UART2_TXD ->		G1_UART_TXTOPOD
		PCON_DISABLE,			NO_PUPDN,	// UART2_NRTS ->
		PCON_DISABLE,			NO_PUPDN,	// UART2_NCTS ->
		PCON_DISABLE,			NO_PUPDN,	// SPI1_CEN ->
		PCON_DISABLE,			NO_PUPDN,	// SPI1_CLK ->
		PCON_DISABLE,			NO_PUPDN,	// SPI1_MOSI ->
		PCON_DISABLE,			NO_PUPDN	// SPI1_MISO ->		BOARD_REV[2]
	       ),
/* P5  */  PINCONFIG(
		PCON_FUNC2,			NO_PUPDN,	// UART1_RXD ->		G1_UART_RXFROMACC
		PCON_FUNC2,			NO_PUPDN,	// UART1_TXD ->		G1_UART_TXTOACC
		PCON_DISABLE,			NO_PUPDN,	// UART1_NRTS ->
		PCON_B137_B165(DISABLE, OUT_1),	NO_PUPDN,	// UART1_NCTS ->	DAC_RESETN
		PCON_DISABLE,			NO_PUPDN,	// VGA_VDEN ->
		PCON_DISABLE,			NO_PUPDN,	// VGA_HSYNC ->
		PCON_DISABLE,			NO_PUPDN,	// VGA_VSYNC ->
		PCON_DISABLE,			NO_PUPDN	// - ->
	       ),
};

/* default push-pull/open-drain configuration */
#define DEFAULT_ODEN		(0x00000007)	// PoR Defaults

/* default drive strength */
#define DEFAULT_DSTR0		(0x00000555)	// PoR Defaults

#define DEFAULT_OSC_DSTR	(0x00000004)

#endif /* ! __TARGET_PINCONFIG_H */
