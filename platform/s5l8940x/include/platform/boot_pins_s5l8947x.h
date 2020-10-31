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

#ifndef __PLATFORM_BOOT_PINS_S5L8947X_H
#define __PLATFORM_BOOT_PINS_S5L8947X_H

/*
 *  boot_interface_pin tables
 *    tables are executed in order for disable and reverse order for enable
 *
 */

#if WITH_HW_FLASH_NOR && WITH_HW_FLASH_NOR_SPI
static const struct boot_interface_pin spi0_boot_interface_pins[] =
{
#if SUPPORT_FPGA
	{ GPIO(17, 3), GPIO_CFG_FUNC0, GPIO_CFG_DFLT },	// SPI0_SSIN
#else
	{ GPIO(17, 3), GPIO_CFG_OUT_1, GPIO_CFG_DFLT },	// SPI0_SSIN
#endif
	{ GPIO(17, 0), GPIO_CFG_FUNC0, GPIO_CFG_DFLT },	// SPI0_SCLK
	{ GPIO(17, 1), GPIO_CFG_FUNC0, GPIO_CFG_DFLT },	// SPI0_MOSI
	{ GPIO(17, 2), GPIO_CFG_FUNC0, GPIO_CFG_DFLT }	// SPI0_MISO
};
#endif /* WITH_HW_FLASH_NOR && WITH_HW_FLASH_NOR_SPI */

#if WITH_HW_BOOTROM_NAND

/* Note: The code in init.c expects this table to begin with CS3 through CS0 */
static const struct boot_interface_pin fmi0_boot_interface_pins[] =
{
	{ GPIO( 5, 2), GPIO_CFG_FUNC0, GPIO_CFG_DFLT },	// FMI0_CEN1
	{ GPIO( 5, 3), GPIO_CFG_FUNC0, GPIO_CFG_DFLT },	// FMI0_CEN0
	{ GPIO( 5, 4), GPIO_CFG_FUNC0, GPIO_CFG_DFLT },	// FMI0_CLE
	{ GPIO( 5, 5), GPIO_CFG_FUNC0, GPIO_CFG_DFLT },	// FMI0_ALE
	{ GPIO( 5, 6), GPIO_CFG_FUNC0, GPIO_CFG_DFLT },	// FMI0_REN
	{ GPIO( 5, 7), GPIO_CFG_FUNC0, GPIO_CFG_DFLT },	// FMI0_WEN
	{ GPIO( 7, 0), GPIO_CFG_FUNC0, GPIO_CFG_DFLT },	// FMI0_IO0
	{ GPIO( 6, 7), GPIO_CFG_FUNC0, GPIO_CFG_DFLT },	// FMI0_IO1
	{ GPIO( 6, 6), GPIO_CFG_FUNC0, GPIO_CFG_DFLT },	// FMI0_IO2
	{ GPIO( 6, 5), GPIO_CFG_FUNC0, GPIO_CFG_DFLT },	// FMI0_IO3
	{ GPIO( 6, 3), GPIO_CFG_FUNC0, GPIO_CFG_DFLT },	// FMI0_IO4
	{ GPIO( 6, 2), GPIO_CFG_FUNC0, GPIO_CFG_DFLT },	// FMI0_IO5
	{ GPIO( 6, 1), GPIO_CFG_FUNC0, GPIO_CFG_DFLT },	// FMI0_IO6
	{ GPIO( 6, 0), GPIO_CFG_FUNC0, GPIO_CFG_DFLT }	// FMI0_IO7
};

/* Note: The code in init.c expects this table to begin with CS3 through CS0 */
static const struct boot_interface_pin fmi1_boot_interface_pins[] =
{
	{ GPIO( 7, 1), GPIO_CFG_FUNC0, GPIO_CFG_DFLT },	// FMI1_CEN1
	{ GPIO( 7, 2), GPIO_CFG_FUNC0, GPIO_CFG_DFLT },	// FMI1_CEN0
	{ GPIO( 7, 3), GPIO_CFG_FUNC0, GPIO_CFG_DFLT },	// FMI1_CLE
	{ GPIO( 7, 4), GPIO_CFG_FUNC0, GPIO_CFG_DFLT },	// FMI1_ALE
	{ GPIO( 7, 5), GPIO_CFG_FUNC0, GPIO_CFG_DFLT },	// FMI1_REN
	{ GPIO( 7, 6), GPIO_CFG_FUNC0, GPIO_CFG_DFLT },	// FMI1_WEN
	{ GPIO( 8, 7), GPIO_CFG_FUNC0, GPIO_CFG_DFLT },	// FMI1_IO0
	{ GPIO( 8, 6), GPIO_CFG_FUNC0, GPIO_CFG_DFLT },	// FMI1_IO1
	{ GPIO( 8, 5), GPIO_CFG_FUNC0, GPIO_CFG_DFLT },	// FMI1_IO2
	{ GPIO( 8, 4), GPIO_CFG_FUNC0, GPIO_CFG_DFLT },	// FMI1_IO3
	{ GPIO( 8, 2), GPIO_CFG_FUNC0, GPIO_CFG_DFLT },	// FMI1_IO4
	{ GPIO( 8, 1), GPIO_CFG_FUNC0, GPIO_CFG_DFLT },	// FMI1_IO5
	{ GPIO( 8, 0), GPIO_CFG_FUNC0, GPIO_CFG_DFLT },	// FMI1_IO6
	{ GPIO( 7, 7), GPIO_CFG_FUNC0, GPIO_CFG_DFLT }	// FMI1_IO7
};
#endif /* WITH_HW_BOOTROM_NAND */

#endif /* ! __PLATFORM_BOOT_PINS_S5L8947X_H */
