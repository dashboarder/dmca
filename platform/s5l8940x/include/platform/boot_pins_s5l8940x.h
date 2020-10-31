/*
 * Copyright (C) 2009-2011 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */

#ifndef __PLATFORM_BOOT_PINS_S5L8940X_H
#define __PLATFORM_BOOT_PINS_S5L8940X_H

/*
 *  boot_interface_pin tables
 *    tables are executed in order for disable and reverse order for enable
 *
 */

#if WITH_HW_FLASH_NOR && WITH_HW_FLASH_NOR_SPI
static const struct boot_interface_pin spi0_boot_interface_pins[] =
{
#if SUPPORT_FPGA
	{ GPIO(20, 5), GPIO_CFG_FUNC0, GPIO_CFG_IN },	// SPI0_SSIN
#else
	{ GPIO(20, 5), GPIO_CFG_OUT_1, GPIO_CFG_IN },	// SPI0_SSIN
#endif
	{ GPIO(20, 2), GPIO_CFG_FUNC0, GPIO_CFG_IN },	// SPI0_SCLK
	{ GPIO(20, 3), GPIO_CFG_FUNC0, GPIO_CFG_IN },	// SPI0_MOSI
	{ GPIO(20, 4), GPIO_CFG_FUNC0, GPIO_CFG_IN }	// SPI0_MISO
};

static const struct boot_interface_pin spi3_boot_interface_pins[] =
{
#if SUPPORT_FPGA
	{ GPIO(18, 7), GPIO_CFG_FUNC0, GPIO_CFG_IN },	// SPI3_SSIN
#else
	{ GPIO(18, 7), GPIO_CFG_OUT_1, GPIO_CFG_IN },	// SPI3_SSIN
#endif
	{ GPIO(18, 6), GPIO_CFG_FUNC0, GPIO_CFG_IN },	// SPI3_SCLK
	{ GPIO(18, 4), GPIO_CFG_FUNC0, GPIO_CFG_IN },	// SPI3_MOSI
	{ GPIO(18, 5), GPIO_CFG_FUNC0, GPIO_CFG_IN }	// SPI3_MISO
};
#endif /* WITH_HW_FLASH_NOR && WITH_HW_FLASH_NOR_SPI */

#if WITH_HW_BOOTROM_NAND

/* Note: The code in init.c expects this table to begin with CS3 through CS0 */
static const struct boot_interface_pin fmi0_boot_interface_pins[] =
{
	{ GPIO( 8, 0), GPIO_CFG_FUNC0, GPIO_CFG_IN },	// FMI0_CEN3
	{ GPIO( 8, 1), GPIO_CFG_FUNC0, GPIO_CFG_IN },	// FMI0_CEN2
	{ GPIO( 8, 2), GPIO_CFG_FUNC0, GPIO_CFG_IN },	// FMI0_CEN1
	{ GPIO( 8, 3), GPIO_CFG_FUNC0, GPIO_CFG_IN },	// FMI0_CEN0
	{ GPIO( 8, 4), GPIO_CFG_FUNC0, GPIO_CFG_IN },	// FMI0_CLE
	{ GPIO( 8, 5), GPIO_CFG_FUNC0, GPIO_CFG_IN },	// FMI0_ALE
	{ GPIO( 8, 6), GPIO_CFG_FUNC0, GPIO_CFG_IN },	// FMI0_REN
	{ GPIO( 8, 7), GPIO_CFG_FUNC0, GPIO_CFG_IN },	// FMI0_WEN
	{ GPIO(10, 0), GPIO_CFG_FUNC0, GPIO_CFG_IN },	// FMI0_IO0
	{ GPIO( 9, 7), GPIO_CFG_FUNC0, GPIO_CFG_IN },	// FMI0_IO1
	{ GPIO( 9, 6), GPIO_CFG_FUNC0, GPIO_CFG_IN },	// FMI0_IO2
	{ GPIO( 9, 5), GPIO_CFG_FUNC0, GPIO_CFG_IN },	// FMI0_IO3
	{ GPIO( 9, 3), GPIO_CFG_FUNC0, GPIO_CFG_IN },	// FMI0_IO4
	{ GPIO( 9, 2), GPIO_CFG_FUNC0, GPIO_CFG_IN },	// FMI0_IO5
	{ GPIO( 9, 1), GPIO_CFG_FUNC0, GPIO_CFG_IN },	// FMI0_IO6
	{ GPIO( 9, 0), GPIO_CFG_FUNC0, GPIO_CFG_IN }	// FMI0_IO7
};

/* Note: The code in init.c expects this table to begin with CS3 through CS0 */
static const struct boot_interface_pin fmi1_boot_interface_pins[] =
{
	{ GPIO(10, 1), GPIO_CFG_FUNC0, GPIO_CFG_IN },	// FMI1_CEN3
	{ GPIO(10, 2), GPIO_CFG_FUNC0, GPIO_CFG_IN },	// FMI1_CEN2
	{ GPIO(10, 3), GPIO_CFG_FUNC0, GPIO_CFG_IN },	// FMI1_CEN1
	{ GPIO(10, 4), GPIO_CFG_FUNC0, GPIO_CFG_IN },	// FMI1_CEN0
	{ GPIO(10, 5), GPIO_CFG_FUNC0, GPIO_CFG_IN },	// FMI1_CLE
	{ GPIO(10, 6), GPIO_CFG_FUNC0, GPIO_CFG_IN },	// FMI1_ALE
	{ GPIO(10, 7), GPIO_CFG_FUNC0, GPIO_CFG_IN },	// FMI1_REN
	{ GPIO(11, 0), GPIO_CFG_FUNC0, GPIO_CFG_IN },	// FMI1_WEN
	{ GPIO(12, 0), GPIO_CFG_FUNC0, GPIO_CFG_IN },	// FMI1_IO0
	{ GPIO(12, 1), GPIO_CFG_FUNC0, GPIO_CFG_IN },	// FMI1_IO1
	{ GPIO(11, 7), GPIO_CFG_FUNC0, GPIO_CFG_IN },	// FMI1_IO2
	{ GPIO(11, 6), GPIO_CFG_FUNC0, GPIO_CFG_IN },	// FMI1_IO3
	{ GPIO(11, 4), GPIO_CFG_FUNC0, GPIO_CFG_IN },	// FMI1_IO4
	{ GPIO(11, 3), GPIO_CFG_FUNC0, GPIO_CFG_IN },	// FMI1_IO5
	{ GPIO(11, 2), GPIO_CFG_FUNC0, GPIO_CFG_IN },	// FMI1_IO6
	{ GPIO(11, 1), GPIO_CFG_FUNC0, GPIO_CFG_IN }	// FMI1_IO7
};
#endif /* WITH_HW_BOOTROM_NAND */

#endif /* ! __PLATFORM_BOOT_PINS_S5L8940X_H */
