/*
 * Copyright (C) 2012-2015 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */
#include <debug.h>
#include <drivers/flash_nor.h>
#include <drivers/iic.h>
#include <drivers/power.h>
#include <lib/devicetree.h>
#include <lib/env.h>
#include <lib/mib.h>
#include <lib/syscfg.h>
#include <platform.h>
#include <platform/gpio.h>
#include <platform/gpiodef.h>
#include <platform/memmap.h>
#include <sys.h>
#include <sys/boot.h>
#include <sys/menu.h>
#include <sys/task.h>
#include <target.h>

static bool		gpio_board_rev_valid;
static u_int32_t	gpio_board_rev;

MIB_CONSTANT(kMIBTargetOsPictureScale,	kOIDTypeUInt32, 2);
MIB_CONSTANT(kMIBTargetPictureRotate,	kOIDTypeInt32,  0);

static u_int32_t b184_get_board_rev(void)
{

	if (!gpio_board_rev_valid) {
		gpio_configure(GPIO_BOARD_REV0, GPIO_CFG_IN);
		gpio_configure(GPIO_BOARD_REV1, GPIO_CFG_IN);
		gpio_configure(GPIO_BOARD_REV2, GPIO_CFG_IN);
		gpio_configure(GPIO_BOARD_REV3, GPIO_CFG_IN);

		gpio_configure_pupdn(GPIO_BOARD_REV0, GPIO_PUP);
		gpio_configure_pupdn(GPIO_BOARD_REV1, GPIO_PUP);
		gpio_configure_pupdn(GPIO_BOARD_REV2, GPIO_PUP);
		gpio_configure_pupdn(GPIO_BOARD_REV3, GPIO_PUP);

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

		gpio_board_rev_valid = true;
	}

	return gpio_board_rev;
}


void target_early_init(void)
{

}

void target_late_init(void)
{
}

void target_init(void)
{
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


int target_bootprep(enum boot_target target)
{
	return 0;
}

bool target_should_recover(void)
{
	return platform_get_request_dfu2() && power_has_usb();
}

bool target_should_poweron(bool *cold_button_boot)
{
#if WITH_HW_POWER
	if (power_get_boot_flag() == kPowerBootFlagColdButton) *cold_button_boot = true;
#else
	*cold_button_boot = false;
#endif // WITH_HW_POWER

	return !*cold_button_boot || platform_get_request_dfu1();
}

bool target_should_poweroff(bool at_boot)
{
	return platform_get_request_dfu1() && (!at_boot || !power_has_usb());
}

#if APPLICATION_IBOOT
void target_watchdog_tickle(void)
{
	u_int32_t value = gpio_read(GPIO_WDOG_TICKLE);
	gpio_write(GPIO_WDOG_TICKLE, value ^ 1);
}
#endif // APPLICATION_IBOOT

#if WITH_ENV

void target_setup_default_environment(void)
{
	env_set("boot-device", "asp_nand", 0);
}

#endif // WITH_ENV

bool target_has_tristar2(void)
{
	return true; // TODO: Does it have tristar2?
}

#if WITH_DEVICETREE

int target_update_device_tree(void)
{
#if WITH_HW_DISPLAY_EDP	
	DTNode		*clcd_node = NULL, *backlight_node = NULL, *lcd_node = NULL;
#endif	
	DTNode		*node;
	uint32_t	propSize;
	char		*propName;
	void		*propData;

	// Store secondary mac addresses into products node
	if (FindNode(0, "product", &node)) {
		propName = "mac-address-wifi1";
		if (FindProperty(node, &propName, &propData, &propSize)) {
			if (env_get("wifi1addr")) {
				env_get_ethaddr("wifi1addr", propData);
			} else {
				int retlen = 0;
				if (!target_get_property(TARGET_PROPERTY_WIFI1_MACADDR, propData, 6, &retlen)) {
					propName[0] = '~';
				}
			}
		}

		propName = "mac-address-ethernet1";
		if (FindProperty(node, &propName, &propData, &propSize)) {
			if (env_get("eth1addr")) {
				env_get_ethaddr("eth1addr", propData);
			} else {
				int retlen = 0;
				if (!target_get_property(TARGET_PROPERTY_ETH1_MACADDR, propData, 6, &retlen)) {
					propName[0] = '~';
				}
			}
		}

		propName = "mac-address-bluetooth1";
		if (FindProperty(node, &propName, &propData, &propSize)) {
			if (env_get("bt1addr")) {
				env_get_ethaddr("bt1addr", propData);
			} else {
				int retlen = 0;
				if (!target_get_property(TARGET_PROPERTY_BT1_MACADDR, propData, 6, &retlen)) {
					propName[0] = '~';
				}
			}
		}
	}

	// Store second wlan mac address into wlan1 node
	if (FindNode(0, "arm-io/usb-complex/usb-ehci1/wlan1", &node)) {
		propName = "local-mac-address";
		if (FindProperty(node, &propName, &propData, &propSize)) {
			if (env_get("wifi1addr")) {
				env_get_ethaddr("wifi1addr", propData);
			} else {
				int retlen = 0;
				if (!target_get_property(TARGET_PROPERTY_WIFI1_MACADDR, propData, 6, &retlen)) {
					propName[0] = '~';
				}
			}
		}
	}

	// Add bluetooth, ethernet second mac address when node is available 
	// <rdar://problem/14324725> B184 /12A43 : bluetooth not responding

	// TriStar 1 on older systems
	if (!target_has_tristar2()) {
		if (FindNode(0, "arm-io/i2c2/tristar", &node)) {
			propName = "compatible";
			if (FindProperty(node, &propName, &propData, &propSize)) {
				strlcpy(propData, "tristar,cbtl1608", kPropNameLength);
			}
		}
	}

	// Update charger calibration data
	if (FindNode(0, "charger", &node)) {
		propName = "usb-input-limit-calibration";
		if (FindProperty(node, &propName, &propData, &propSize)) {
			syscfgCopyDataForTag('CBAT', propData, propSize);
		}
	}

	return 0;
}

#endif // WITH_DEVICETREE
