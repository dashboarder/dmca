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

#ifndef __PLATFORM_BOOT_PINS_S5L8942X_H
#define __PLATFORM_BOOT_PINS_S5L8942X_H

/*
 *  boot_interface_pin tables
 *    tables are executed in order for disable and reverse order for enable
 *
 */

#if WITH_HW_FLASH_NOR && WITH_HW_FLASH_NOR_SPI
static const struct boot_interface_pin spi0_boot_interface_pins[] =
{
#if SUPPORT_FPGA
	{ GPIO(12, 3), GPIO_CFG_FUNC0, GPIO_CFG_DFLT },	// SPI0_SSIN
#else
	{ GPIO(12, 3), GPIO_CFG_OUT_1, GPIO_CFG_DFLT },	// SPI0_SSIN
#endif
	{ GPIO(12, 0), GPIO_CFG_FUNC0, GPIO_CFG_DFLT },	// SPI0_SCLK
	{ GPIO(12, 1), GPIO_CFG_FUNC0, GPIO_CFG_DFLT },	// SPI0_MOSI
	{ GPIO(12, 2), GPIO_CFG_FUNC0, GPIO_CFG_DFLT }	// SPI0_MISO
};

static const struct boot_interface_pin spi3_boot_interface_pins[] =
{
#if SUPPORT_FPGA
	{ GPIO(16, 3), GPIO_CFG_FUNC1, GPIO_CFG_DFLT },	// SPI3_SSIN
#else
	{ GPIO(16, 3), GPIO_CFG_OUT_1, GPIO_CFG_DFLT },	// SPI3_SSIN
#endif
	{ GPIO(16, 0), GPIO_CFG_FUNC1, GPIO_CFG_DFLT },	// SPI3_SCLK
	{ GPIO(16, 1), GPIO_CFG_FUNC1, GPIO_CFG_DFLT },	// SPI3_MOSI
	{ GPIO(16, 2), GPIO_CFG_FUNC1, GPIO_CFG_DFLT }	// SPI3_MISO
};
#endif /* WITH_HW_FLASH_NOR && WITH_HW_FLASH_NOR_SPI */

#if WITH_HW_BOOTROM_NAND

/* Note: The code in init.c expects this table to begin with CS3 through CS0 */
static const struct boot_interface_pin fmi0_boot_interface_pins[] =
{
	{ GPIO(17, 2), GPIO_CFG_FUNC0, GPIO_CFG_DFLT },	// FMI0_CEN3
	{ GPIO(17, 3), GPIO_CFG_FUNC0, GPIO_CFG_DFLT },	// FMI0_CEN2
	{ GPIO(17, 4), GPIO_CFG_FUNC0, GPIO_CFG_DFLT },	// FMI0_CEN1
	{ GPIO(17, 5), GPIO_CFG_FUNC0, GPIO_CFG_DFLT },	// FMI0_CEN0
	{ GPIO(17, 6), GPIO_CFG_FUNC0, GPIO_CFG_DFLT },	// FMI0_CLE
	{ GPIO(17, 7), GPIO_CFG_FUNC0, GPIO_CFG_DFLT },	// FMI0_ALE
	{ GPIO(18, 0), GPIO_CFG_FUNC0, GPIO_CFG_DFLT },	// FMI0_REN
	{ GPIO(18, 1), GPIO_CFG_FUNC0, GPIO_CFG_DFLT },	// FMI0_WEN
	{ GPIO(20, 4), GPIO_CFG_FUNC0, GPIO_CFG_DFLT },	// FMI0_IO0
	{ GPIO(20, 3), GPIO_CFG_FUNC0, GPIO_CFG_DFLT },	// FMI0_IO1
	{ GPIO(20, 2), GPIO_CFG_FUNC0, GPIO_CFG_DFLT },	// FMI0_IO2
	{ GPIO(20, 1), GPIO_CFG_FUNC0, GPIO_CFG_DFLT },	// FMI0_IO3
	{ GPIO(18, 5), GPIO_CFG_FUNC0, GPIO_CFG_DFLT },	// FMI0_IO4
	{ GPIO(18, 4), GPIO_CFG_FUNC0, GPIO_CFG_DFLT },	// FMI0_IO5
	{ GPIO(18, 3), GPIO_CFG_FUNC0, GPIO_CFG_DFLT },	// FMI0_IO6
	{ GPIO(18, 2), GPIO_CFG_FUNC0, GPIO_CFG_DFLT }	// FMI0_IO7
};

/* Note: The code in init.c expects this table to begin with CS3 through CS0 */
static const struct boot_interface_pin fmi1_boot_interface_pins[] =
{
	{ GPIO(20, 5), GPIO_CFG_FUNC0, GPIO_CFG_DFLT },	// FMI1_CEN3
	{ GPIO(20, 6), GPIO_CFG_FUNC0, GPIO_CFG_DFLT },	// FMI1_CEN2
	{ GPIO(20, 7), GPIO_CFG_FUNC0, GPIO_CFG_DFLT },	// FMI1_CEN1
	{ GPIO(21, 0), GPIO_CFG_FUNC0, GPIO_CFG_DFLT },	// FMI1_CEN0
	{ GPIO(21, 1), GPIO_CFG_FUNC0, GPIO_CFG_DFLT },	// FMI1_CLE
	{ GPIO(21, 2), GPIO_CFG_FUNC0, GPIO_CFG_DFLT },	// FMI1_ALE
	{ GPIO(21, 3), GPIO_CFG_FUNC0, GPIO_CFG_DFLT },	// FMI1_REN
	{ GPIO(21, 4), GPIO_CFG_FUNC0, GPIO_CFG_DFLT },	// FMI1_WEN
	{ GPIO(22, 5), GPIO_CFG_FUNC0, GPIO_CFG_DFLT },	// FMI1_IO0
	{ GPIO(22, 4), GPIO_CFG_FUNC0, GPIO_CFG_DFLT },	// FMI1_IO1
	{ GPIO(22, 3), GPIO_CFG_FUNC0, GPIO_CFG_DFLT },	// FMI1_IO2
	{ GPIO(22, 2), GPIO_CFG_FUNC0, GPIO_CFG_DFLT },	// FMI1_IO3
	{ GPIO(22, 0), GPIO_CFG_FUNC0, GPIO_CFG_DFLT },	// FMI1_IO4
	{ GPIO(21, 7), GPIO_CFG_FUNC0, GPIO_CFG_DFLT },	// FMI1_IO5
	{ GPIO(21, 6), GPIO_CFG_FUNC0, GPIO_CFG_DFLT },	// FMI1_IO6
	{ GPIO(21, 5), GPIO_CFG_FUNC0, GPIO_CFG_DFLT }	// FMI1_IO7
};
#endif /* WITH_HW_BOOTROM_NAND */

#endif /* ! __PLATFORM_BOOT_PINS_S5L8942X_H */
