/*
 * Copyright (C) 2013-2015 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */
#include <debug.h>
#if (APPLICATION_IBOOT && (PRODUCT_IBOOT || PRODUCT_IBEC))
#include <drivers/displayport/displayport.h>
#include <drivers/displayport.h>
#endif
#include <drivers/apple/gpio.h>
#include <drivers/iic.h>
#include <drivers/power.h>
#include <lib/env.h>
#include <lib/mib.h>
#include <lib/syscfg.h>
#include <platform.h>
#include <platform/gpiodef.h>
#include <platform/soc/chipid.h>
#include <target.h>

// Board ids
#define J105_AP_BOARD_ID		(0x04)
#define J105_DEV_BOARD_ID		(0x05)

// Board revs
//#define J105_DEV0_BOARD_REV		(0x0)

// Do not delay iBoot by more than this amount from power on (microseconds).
// Give up waiting for a boot logo and carry on at this point.
#define kDPDeviceStartTimeout (8 * 1000 * 1000)
extern utime_t gPowerOnTime; 
static bool	gpio_board_rev_valid;
static uint32_t	gpio_board_rev;
#if (APPLICATION_IBOOT && (PRODUCT_IBOOT || PRODUCT_IBEC))
#if WITH_HW_DISPLAY_DISPLAYPORT
static dp_t dp_link_config = {
	.mode		= kDPControllerMode_Slave,
	.type		= kDPControllerType_DP,
	.min_link_rate	= kLinkRate270Gbps,
	.max_link_rate	= kLinkRate270Gbps,
	.lanes		= 2,
	.ssc		= 0,
	.alpm		= 0,
	.vrr_enable	= 0,
	.vrr_on		= 0,
	.fast_link_training =	true,
};
#endif
#endif

MIB_CONSTANT(kMIBTargetOsPictureScale,	kOIDTypeUInt32, 1);
MIB_CONSTANT(kMIBTargetPictureRotate,	kOIDTypeInt32,  0);

static uint32_t j105_get_board_rev(void)
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

		printf("board_rev 0x%x\n", gpio_board_rev);

	}

	return gpio_board_rev;
}

void target_early_init(void)
{
#if PRODUCT_LLB || PRODUCT_IBSS
    dprintf(DEBUG_INFO, "%s: chip rev: %d, fuse: %d\n", __func__,
           chipid_get_chip_revision(), chipid_get_fuse_revision());

    //    target_fixup_pmu();
#endif
}

void target_late_init(void)
{
#if 0
	if (target_config_dev() && j105_get_board_rev() > J105_DEV_BOARD_LAST) {
		platform_not_supported();
	}
	if (target_config_ap() && j105_get_board_rev() < J105_AP_BOARD_FIRST) {
		platform_not_supported();
	}
#endif

#if (APPLICATION_IBOOT && (PRODUCT_IBOOT || PRODUCT_IBEC))
#if WITH_HW_DISPLAY_DISPLAYPORT
	// Backgrounded. See dp_device_wait_started() call below.
	displayport_init(&dp_link_config);
#endif
#endif
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

int target_debug_init(void)
{
    return 0;
}

int target_bootprep(enum boot_target target)
{
	switch (target) {
		case BOOT_DARWIN:
		case BOOT_DIAGS:
#if WITH_HW_DISPLAY_DISPLAYPORT && PRODUCT_IBOOT
			/* Potentially delay boot so we have a
			 * logo. We might have to do this if a
			 * connected TV is particularly slow at
			 * responding to EDID. Sorry. */
			dp_device_wait_started(gPowerOnTime + kDPDeviceStartTimeout);
#endif
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

void * target_get_display_configuration(void)
{
#if (APPLICATION_IBOOT && (PRODUCT_IBOOT || PRODUCT_IBEC))
#if WITH_HW_DISPLAY_DISPLAYPORT
	return ((void *)(&dp_link_config));
#endif
#else
	return NULL;
#endif
}

#if WITH_ENV

void target_setup_default_environment(void)
{
	env_set("display-color-space","RGB888", 0);
	env_set("display-timing", "720p", 0);
	env_set("idle-off", "false", 0);	    // do not idle off; there is no battery and this will just reboot (8035422)
	env_set("adbe-tunables", "default", 0);
	env_set("adfe-tunables", "720p", 0);
}

#endif // WITH_ENV

#if WITH_DEVICETREE

int target_update_device_tree(void)
{
	DTNode		*node;
	uint32_t	propSize;
	char		*propName;
	void		*propData;

	// Update the pmu node with the actual vcore values
	if (FindNode(0, "arm-io/i2c1/pmu", &node)) {
		propName = "swi-vcores";
		if (FindProperty(node, &propName, &propData, &propSize)) {
			platform_get_soc_voltages(propSize / sizeof(u_int32_t), propData);
		}
	}

	// Store RBDA (Remote Bluetooth Device Address) into products node
	if (FindNode(0, "product", &node)) {
		propName = "bluetooth-dev-addr0";
		if (FindProperty(node, &propName, &propData, &propSize)) {
			if (env_get("rbdaddr0")) {
				env_get_ethaddr("rbdaddr0", propData);
			} else {
				int retlen = 0;
				if (!target_get_property(TARGET_PROPERTY_BLUETOOTH_DEV_MACADDR0, propData, 6, &retlen)) {
					propName[0] = '~';
				}
			}
		}
	}

	return 0;
}

#endif // WITH_DEVICETREE

