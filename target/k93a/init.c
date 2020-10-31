/*
 * Copyright (C) 2011-2015 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */
#include <debug.h>
#include <drivers/display.h>
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
#include <target/powerconfig.h>
#include <target/display_gamma_tables.h>

MIB_CONSTANT_PTR(kMIBTargetDisplayGammaTablePtr,kOIDTypeStruct, (void *)k93a_display_gamma_tables);
MIB_CONSTANT(kMIBTargetDisplayGammaTableCount,	kOIDTypeUInt32, ARRAY_SIZE(k93a_display_gamma_tables));
MIB_CONSTANT(kMIBTargetOsPictureScale,		kOIDTypeUInt32, 1);
MIB_CONSTANT(kMIBTargetPictureRotate,		kOIDTypeInt32,  90);

static uint32_t display_config;

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
	switch (target) {
		case BOOT_DARWIN:
		case BOOT_DARWIN_RESTORE:
		case BOOT_DIAGS:
			break;

		default:
			; // do nothing
	}

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
#endif

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

void * target_get_display_configuration(void)
{
	display_config = 0x00000644;
	return ((void *)(&display_config));
}

#if WITH_ENV

void target_setup_default_environment(void)
{
	env_set("boot-device", "nand0", 0);
	env_set("display-color-space","RGB888", 0);
	env_set("display-timing", "k48", 0);
}

#endif

#if WITH_DEVICETREE

int target_update_device_tree(void)
{
	DTNode		*node, *clcd_node, *backlight_node;
	uint32_t	propSize, propCnt;
	char		*propName;
	void		*propData;

	// Update the DWI node with the actual voltages
	if (FindNode(0, "arm-io/dwi", &node)) {
		propName = "voltages-buck0";
		if (FindProperty(node, &propName, &propData, &propSize)) {
			propCnt = propSize / sizeof(u_int32_t);
			platform_get_cpu_voltages(propCnt, propData);
			platform_convert_voltages(0, propCnt, propData);
		}
		propName = "voltages-buck2";
		if (FindProperty(node, &propName, &propData, &propSize)) {
			propCnt = propSize / sizeof(u_int32_t);
			platform_get_soc_voltages(propCnt, propData);
			platform_convert_voltages(2, propCnt, propData);
		}
	}

	// Find the CLCD node
	FindNode(0, "arm-io/clcd", &clcd_node);

#if WITH_HW_DISPLAY_PINOT
	if (FindNode(0, "arm-io/mipi-dsim/lcd", &node)) {
		extern int pinot_update_device_tree(DTNode *pinot_node, DTNode *clcd_node, DTNode *backlight_node);

		FindNode(0, "backlight", &backlight_node);
		pinot_update_device_tree(node, clcd_node, backlight_node);
 	}
#endif

	// Update the bottom prox calibration data
	if (FindNode(0, "arm-io/i2c1/prox", &node)) {
		propName = "prox-bottom-calibration";
		if (FindProperty(node, &propName, &propData, &propSize)) {
			syscfgCopyDataForTag('PBCl', propData, propSize);
		}
		propName = "prox-temp-base";
		if (FindProperty(node, &propName, &propData, &propSize)) {
			syscfgCopyDataForTag('PBTb', propData, propSize);
		}
		propName = "prox-centerpoint-base";
		if (FindProperty(node, &propName, &propData, &propSize)) {
			syscfgCopyDataForTag('PBCb', propData, propSize);
		}
		propName = "prox-calibration-blob";
		if (FindProperty(node, &propName, &propData, &propSize)) {
			syscfgCopyDataForTag('CPCl', propData, propSize);
		}
	}

	// Update als node with calibration data
	if (FindNode(0, "arm-io/i2c2/als", &node)) {
		propName = "backlight-leakage-channel0";
		if (FindProperty(node, &propName, &propData, &propSize)) {
			syscfgCopyDataForTag('ALC0', propData, propSize);
		}
		propName = "backlight-leakage-channel1";
		if (FindProperty(node, &propName, &propData, &propSize)) {
			syscfgCopyDataForTag('ALC1', propData, propSize);
		}
		propName = "alsCalibration";
		if (FindProperty(node, &propName, &propData, &propSize)) {
			syscfgCopyDataForTag('AlsC', propData, propSize);
		}
		propName = "als-colorCfg";
		if (FindProperty(node, &propName, &propData, &propSize)) {
			syscfgCopyDataForTag('ClrC', propData, propSize);
		}
	}

	// Update the gyro calibration data
	if (FindNode(0, "arm-io/i2c2/gyro", &node)) {
		propName = "low-temp-offset";
		if (FindProperty(node, &propName, &propData, &propSize)) {
			syscfgCopyDataForTag('LTGO', propData, propSize);
		}
		propName = "high-temp-offset";
		if (FindProperty(node, &propName, &propData, &propSize)) {
			syscfgCopyDataForTag('HTGO', propData, propSize);
		}
		propName = "gyro-sensitivity-calibration";
		if (FindProperty(node, &propName, &propData, &propSize)) {
			syscfgCopyDataForTag('GSCl', propData, propSize);
		}
		propName = "gyro-temp-table";
		if (FindProperty(node, &propName, &propData, &propSize)) {
			syscfgCopyDataForTag('GYTT', propData, propSize);
		}
		propName = "gyro-interrupt-calibration";
		if (FindProperty(node, &propName, &propData, &propSize)) {
			syscfgCopyDataForTag('GICl', propData, propSize);
		}
	}

	// Update accelerometer calibration data
	if (FindNode(0, "arm-io/i2c2/accelerometer", &node)) {
		propName = "low-temp-accel-offset";
		if (FindProperty(node, &propName, &propData, &propSize)) {
			syscfgCopyDataForTag('LTAO', propData, propSize);
		}
		propName = "accel-sensitivity-calibration";
		if (FindProperty(node, &propName, &propData, &propSize)) {
			syscfgCopyDataForTag('ASCl', propData, propSize);
		}
	}

	// Update camera calibration data
	if (FindNode(0, "arm-io/isp", &node)) {
		propName = "camera-calibration";
		if (FindProperty(node, &propName, &propData, &propSize)) {
			syscfgCopyDataForTag('CmCl', propData, propSize);
		}
	}

	// Update USB input current limit
	if (FindNode(0, "charger", &node)) {
		propName = "usb-input-limit-max";
		if (FindProperty(node, &propName, &propData, &propSize)) {
			*(uint32_t *)propData = TARGET_MAX_USB_INPUT_CURRENT;
		}
	}

	return 0;
}

#endif
