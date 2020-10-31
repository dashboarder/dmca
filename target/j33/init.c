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
#if (APPLICATION_IBOOT && PRODUCT_IBOOT)
#include <drivers/displayport/displayport.h>
#include <drivers/displayport.h>
#endif
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

// Do not delay iBoot by more than this amount from power on (microseconds).
// Give up waiting for a boot logo and carry on at this point.
#define kDPDeviceStartTimeout (8 * 1000 * 1000)

static u_int32_t j33_get_board_rev(void);
extern utime_t gPowerOnTime;
#if (APPLICATION_IBOOT && PRODUCT_IBOOT )
#if WITH_HW_DISPLAY_DISPLAYPORT
static dp_t dp_link_config = {
	.mode		= 0,
	.type		= 0,
	.min_link_rate	= 0x6,
	.max_link_rate	= 0x6,
	.lanes		= 2,
	.ssc		= 0,
	.alpm		= 0,
	.vrr_enable	= 0,
	.vrr_on		= 0,
};
#endif
#endif //(APPLICATION_IBOOT && PRODUCT_IBOOT)

MIB_CONSTANT(kMIBTargetOsPictureScale,	kOIDTypeUInt32, 1);
MIB_CONSTANT(kMIBTargetPictureRotate,	kOIDTypeInt32,  0);

void target_early_init(void)
{
}

void target_late_init(void)
{

#if WITH_HW_MCU
	// Late display initialization. Needed to wait for MCU.
	mcu_init();
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
//	u_int32_t value = gpio_read(GPIO_WDOG_TICKLE);
//	gpio_write(GPIO_WDOG_TICKLE, value ^ 1);
}
#endif // APPLICATION_IBOOT


void * target_get_display_configuration(void)
{
#if (APPLICATION_IBOOT && PRODUCT_IBOOT)
#if WITH_HW_DISPLAY_DISPLAYPORT
	return ((void *)(&dp_link_config));
#endif
#else
	return NULL;
#endif //(APPLICATION_IBOOT && PRODUCT_IBOOT)
}
#if WITH_ENV

void target_setup_default_environment(void)
{
	env_set("boot-device", "nand0", 0);
	env_set("display-color-space","RGB888", 0);
	env_set("display-timing", "720p", 0);
	env_set("idle-off", "false", 0);	    // do not idle off; there is no battery and this will just reboot (8035422)
}

#endif

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

	if (FindNode(0, "arm-io/mcu0", &node)) {
		propName = "board-rev";
		if (FindProperty(node, &propName, &propData, &propSize))
			((u_int32_t *)propData)[0] = j33_get_board_rev();
	}

	return 0;
}

#endif


static u_int32_t j33_get_board_rev(void)
{
	u_int32_t gpio_board_rev;

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

	return (gpio_board_rev &0xF);
}
