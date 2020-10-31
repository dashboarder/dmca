/*
 * Copyright (C) 2009-2015 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */
#include <debug.h>
//#include <drivers/flash_nor.h>
//#include <drivers/iic.h>
#include <drivers/power.h>
//#include <drivers/mcu.h>
//#include <lib/devicetree.h>
#include <lib/env.h>
#include <lib/mib.h>
#include <lib/syscfg.h>
#include <platform.h>
//#include <platform/gpio.h>
#include <platform/gpiodef.h>
#include <platform/memmap.h>
#include <sys.h>
#include <sys/boot.h>
#include <sys/menu.h>
#include <sys/task.h>
#include <target.h>
//#include <target/powerconfig.h>

// Do not delay iBoot by more than this amount from power on (microseconds).
// Give up waiting for a boot logo and carry on at this point.
#define kDPDeviceStartTimeout (8 * 1000 * 1000)

static u_int32_t b238_get_board_rev(void);
extern utime_t gPowerOnTime;

MIB_CONSTANT(kMIBTargetOsPictureScale,	kOIDTypeUInt32, 1);
MIB_CONSTANT(kMIBTargetPictureRotate,	kOIDTypeInt32,  0);

void target_early_init(void)
{
#if PRODUCT_LLB || PRODUCT_IBSS
//    dprintf(DEBUG_INFO, "%s: chip rev: %d, fuse: %d\n", __func__,
//            chipid_get_chip_revision(), chipid_get_fuse_revision());
#endif
}

void target_late_init(void)
{
}

void target_init(void)
{
    dprintf(DEBUG_INFO, "%s ...\n", __func__);

#if WITH_HW_FLASH_NOR
	flash_nor_init(SPI_NOR0);
#endif
}

void target_quiesce_hardware(void)
{
}

void target_poweroff(void)
{
}

int target_debug_init(void)
{
    return 0;
}

int target_bootprep(enum boot_target target)
{
	switch (target) {
		case BOOT_DARWIN:
		case BOOT_DIAGS:
			break;
		case BOOT_DARWIN_RESTORE:
			break;
		default:
			// do nothing
			break;
	}

	return 0;
}

bool target_should_recover(void)
{
	// trigger recovery mode whenever USB is attached, unless
	// environment variable says otherwise.
	
	return power_has_usb() && !env_get_bool("auto-boot-usb", true);
}

bool target_should_poweron(bool *cold_button_boot)
{
	// always powers on
	return true;
}

bool target_should_poweroff(bool at_boot)
{
	// never power off
	return false;
}

#if APPLICATION_IBOOT
void target_watchdog_tickle(void)
{
//	u_int32_t value = gpio_read(GPIO_WDOG_TICKLE);
//	gpio_write(GPIO_WDOG_TICKLE, value ^ 1);
}
#endif // APPLICATION_IBOOT

#if WITH_ENV

void target_setup_default_environment(void)
{
	env_set("boot-device", "asp_nand", 0);
	env_set("idle-off", "false", 0);	    // do not idle off; there is no battery and this will just reboot (8035422)
}

#endif

#if WITH_DEVICETREE

int target_update_device_tree(void)
{
#if 0
	DTNode		*node;
	uint32_t	propSize;
	char		*propName;
	void		*propData;

	// Update the pmu node with the actual vcore values
	if (FindNode(0, "arm-io/i2c0/pmu", &node)) {
		propName = "swi-vcores";
		if (FindProperty(node, &propName, &propData, &propSize)) {
			platform_get_soc_voltages(propSize / sizeof(u_int32_t), propData);
		}
	}
#endif
	return 0;
}

#endif


static u_int32_t b238_get_board_rev(void)
{
	u_int32_t gpio_board_rev=0;

#if 0
	gpio_configure(GPIO_BOARD_REV0, GPIO_CFG_IN);
	gpio_configure(GPIO_BOARD_REV1, GPIO_CFG_IN);
	gpio_configure(GPIO_BOARD_REV2, GPIO_CFG_IN);
	gpio_configure(GPIO_BOARD_REV3, GPIO_CFG_IN);

	gpio_configure_pupdn(GPIO_BOARD_REV0, GPIO_PDN);
	gpio_configure_pupdn(GPIO_BOARD_REV1, GPIO_PDN);
	gpio_configure_pupdn(GPIO_BOARD_REV2, GPIO_PDN);
	gpio_configure_pupdn(GPIO_BOARD_REV3, GPIO_PDN);

	spin(100); // Wait 100us

	gpio_board_rev = 
		(gpio_read(GPIO_BOARD_REV3) << 3) |
		(gpio_read(GPIO_BOARD_REV2) << 2) |
		(gpio_read(GPIO_BOARD_REV1) << 1) |
		(gpio_read(GPIO_BOARD_REV0) << 0);

	gpio_configure(GPIO_BOARD_REV0, GPIO_CFG_DFLT);
	gpio_configure(GPIO_BOARD_REV1, GPIO_CFG_DFLT);
	gpio_configure(GPIO_BOARD_REV2, GPIO_CFG_DFLT);
	gpio_configure(GPIO_BOARD_REV3, GPIO_CFG_DFLT);
#endif
	return (gpio_board_rev &0xF);
}
