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
#include <drivers/displayport.h>
#include <drivers/flash_nor.h>
#include <drivers/iic.h>
#include <drivers/power.h>
#include <drivers/mcu.h>
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
#include <target/powerconfig.h>

#define	J34_DEV0A_BOARD		(0xF)	//Has Hoover and Bluesteel
#define	J34_DEV0B_BOARD		(0xE)	//Has Hoover and Bluesteel
#define	J34_DEV1_BOARD		(0xD)	//Hoover Only

#define	J34_POC_BOARD		(0x8)	//Hoover Only
#define	J34_PROTO1_BOARD	(0x9)	//Hoover Only
#define	J34_PROTO2_BOARD	(0xA)	//Hoover Only
#define	J34_EVT_BOARD		(0xB)	//Hoover Only
#define	J34_DVT_BOARD		(0xC)	//Hoover Only
#define	J34_PVT_BOARD		(0xF)	//Hoover Only

// Do not delay iBoot by more than this amount from power on (microseconds).
// Give up waiting for a boot logo and carry on at this point.
#define kDPDeviceStartTimeout (8 * 1000 * 1000)
extern utime_t gPowerOnTime; 
static bool		gpio_board_rev_valid;
static u_int32_t	gpio_board_rev;
#if WITH_HW_DISPLAY_DISPLAYPORT
static dp_t dp_link_config = {
	.mode		= 1,
	.type		= 0,
	.min_link_rate	= 0xa,
	.max_link_rate	= 0xa,
	.lanes		= 4,
	.ssc		= 0,
	.alpm		= 0,
	.vrr_enable	= 0,
	.vrr_on		= 0,
};
#endif

MIB_CONSTANT(kMIBTargetOsPictureScale,	kOIDTypeUInt32, 1);
MIB_CONSTANT(kMIBTargetPictureRotate,	kOIDTypeInt32,  0);

static u_int32_t j34_get_board_rev(void)
{

	if (!gpio_board_rev_valid) {
		gpio_configure(GPIO_BOARD_REV0, GPIO_CFG_IN);
		gpio_configure(GPIO_BOARD_REV1, GPIO_CFG_IN);
		gpio_configure(GPIO_BOARD_REV2, GPIO_CFG_IN);
		gpio_configure(GPIO_BOARD_REV3, GPIO_CFG_IN);

		if (target_config_ap()) {
			gpio_configure_pupdn(GPIO_BOARD_REV0, GPIO_PUP);
			gpio_configure_pupdn(GPIO_BOARD_REV1, GPIO_PUP);
			gpio_configure_pupdn(GPIO_BOARD_REV2, GPIO_PUP);
			gpio_configure_pupdn(GPIO_BOARD_REV3, GPIO_PUP);
		} else {
			gpio_configure_pupdn(GPIO_BOARD_REV0, GPIO_PDN);
			gpio_configure_pupdn(GPIO_BOARD_REV1, GPIO_PDN);
			gpio_configure_pupdn(GPIO_BOARD_REV2, GPIO_PDN);
			gpio_configure_pupdn(GPIO_BOARD_REV3, GPIO_PDN);
		}

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
	
	printf("board_rev 0x%x\n", gpio_board_rev);

	return gpio_board_rev;
}

static void j34_reset_hoover(bool reset)
{
	power_set_gpio(POWER_GPIO_HVR_RESET_L, 1, !reset);
}

void target_early_init(void)
{
}

void target_late_init(void)
{
	if (target_config_dev() && j34_get_board_rev() == J34_DEV0A_BOARD) {
		platform_not_supported();
	}

#if WITH_HW_MCU
	// Late display initialization. Needed to wait for MCU.
	mcu_init();
#if WITH_HW_DISPLAY_DISPLAYPORT
	// Backgrounded. See dp_device_wait_started() call below.
	displayport_init(&dp_link_config);
	j34_reset_hoover(false);
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
#if WITH_HW_MCU
	// radar 9746416 -- UART4 left in interrupt mode will cause kernel panic
	mcu_quiesce_uart();
#endif
}

void target_poweroff(void)
{
}

int target_debug_init(void)
{
#if WITH_HW_MCU
	mcu_start_recover();
#endif

    return 0;
}

int target_bootprep(enum boot_target target)
{
	switch (target) {
		case BOOT_DARWIN:
		case BOOT_DIAGS:
#if WITH_HW_MCU
#if WITH_HW_DISPLAY_DISPLAYPORT && PRODUCT_IBOOT
			/* Potentially delay boot so we have a
			 * logo. We might have to do this if a
			 * connected TV is particularly slow at
			 * responding to EDID. Sorry. */
			dp_device_wait_started(gPowerOnTime + kDPDeviceStartTimeout);
#endif
			mcu_start_boot();
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

#if APPLICATION_IBOOT
void target_watchdog_tickle(void)
{
	u_int32_t value = gpio_read(GPIO_WDOG_TICKLE);
	gpio_write(GPIO_WDOG_TICKLE, value ^ 1);
}
#endif // APPLICATION_IBOOT

void * target_get_display_configuration(void)
{
#if WITH_HW_DISPLAY_DISPLAYPORT
	return ((void *)(&dp_link_config));
#else
	return NULL;
#endif
}

#if WITH_ENV

void target_setup_default_environment(void)
{
	env_set("boot-device", "asp_nand", 0);
	env_set("display-color-space","RGB888", 0);
	env_set("display-timing", "720p-j34", 0);
	env_set("idle-off", "false", 0);	    // do not idle off; there is no battery and this will just reboot (8035422)
}

#endif // WITH_ENV

bool target_has_tristar2(void)
{
	return false;
}

#if WITH_DEVICETREE

int target_update_device_tree(void)
{
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
	//
	// TriStar 1 on older systems
	if (!target_has_tristar2()) {
		if (FindNode(0, "arm-io/i2c2/tristar", &node)) {
			propName = "compatible";
			if (FindProperty(node, &propName, &propData, &propSize)) {
				strlcpy(propData, "tristar,cbtl1608", kPropNameLength);
			}
		}
	}

	if (FindNode(0, "arm-io/mcu0", &node)) {
		propName = "board-rev";
		if (FindProperty(node, &propName, &propData, &propSize))
			((u_int32_t *)propData)[0] = j34_get_board_rev();
	}

	// Store pinto mac address into products node
	if (FindNode(0, "product", &node)) {
		propName = "mac-address-pinto0";
		if (FindProperty(node, &propName, &propData, &propSize)) {
			if (env_get("pintoaddr")) {
				env_get_ethaddr("pintoaddr", propData);
			} else {
				int retlen = 0;
				if (!target_get_property(TARGET_PROPERTY_PINTO_MACADDR, propData, 6, &retlen)) {
					propName[0] = '~';
				}
			}
		}
	}

	// Store pinto mac address into pinto node
	if (FindNode(0, "arm-io/spi1/pinto", &node)) {
		propName = "local-mac-address";
		if (FindProperty(node, &propName, &propData, &propSize)) {
			if (env_get("pintoaddr")) {
				env_get_ethaddr("pintoaddr", propData);
			} else {
				int retlen = 0;
				if (!target_get_property(TARGET_PROPERTY_PINTO_MACADDR, propData, 6, &retlen)) {
					propName[0] = '~';
				}
			}
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
#endif
