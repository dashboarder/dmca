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

static u_int32_t e88_get_board_rev(void);

MIB_CONSTANT(kMIBTargetOsPictureScale,	kOIDTypeUInt32, 2);
MIB_CONSTANT(kMIBTargetPictureRotate,	kOIDTypeInt32,  0);

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

#if WITH_ENV

void target_setup_default_environment(void)
{
	env_set("boot-device", "nand0", 0);
	env_set("boot-partition", "0", 0);
	env_set("boot-path", "/System/Library/Caches/com.apple.kernelcaches/kernelcache", 0);
}

#endif

static bool		gpio_board_rev_valid;
static u_int32_t	gpio_board_rev;

static u_int32_t e88_get_board_rev(void)
{
	if (!gpio_board_rev_valid) {
		gpio_configure(GPIO_BOARD_REV0, GPIO_CFG_IN);
		gpio_configure(GPIO_BOARD_REV1, GPIO_CFG_IN);
		gpio_configure(GPIO_BOARD_REV2, GPIO_CFG_IN);
		gpio_configure(GPIO_BOARD_REV3, GPIO_CFG_IN);

		gpio_configure_pupdn(GPIO_BOARD_REV0, GPIO_PDN);
		gpio_configure_pupdn(GPIO_BOARD_REV1, GPIO_PDN);
		gpio_configure_pupdn(GPIO_BOARD_REV2, GPIO_PDN);
		gpio_configure_pupdn(GPIO_BOARD_REV3, GPIO_PDN);

		spin(100); // Wait 100us

		uint32_t brd_rev3_inverted = (gpio_read(GPIO_BOARD_REV3) ? 0UL : 1UL);
		gpio_board_rev =
			(brd_rev3_inverted << 3) |
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

#if WITH_DEVICETREE

int target_update_device_tree(void)
{
	DTNode		*node;
	uint32_t	propSize, propCnt;
	char		*propName;
	void		*propData;

	// Update the DWI node with the actual voltages
	if (FindNode(0, "arm-io/dwi", &node)) {
		propName = "voltages-buck2";
		if (FindProperty(node, &propName, &propData, &propSize)) {
			propCnt = propSize / sizeof(u_int32_t);
			platform_get_cpu_voltages(propCnt, propData);
			platform_convert_voltages(2, propCnt, propData);
		}
		propName = "voltages-buck0";
		if (FindProperty(node, &propName, &propData, &propSize)) {
			propCnt = propSize / sizeof(u_int32_t);
			platform_get_soc_voltages(propCnt, propData);
			platform_convert_voltages(0, propCnt, propData);
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
		propName = "accel-interrupt-calibration";
		if (FindProperty(node, &propName, &propData, &propSize)) {
			syscfgCopyDataForTag('AICl', propData, propSize);
		}
	}

	// Updte the speaker calibration data
	if (FindNode(0, "arm-io/i2c0/audio-speaker", &node)) {
		propName = "speaker-rdc";
		if (FindProperty(node, &propName, &propData, &propSize)) {
			syscfgCopyDataForTag('SRdc', propData, propSize);
		}
		propName = "speaker-config";
		if (FindProperty(node, &propName, &propData, &propSize)) {
			uint8_t vpbr;
			if (syscfgCopyDataForTag('VPBR', &vpbr, sizeof(vpbr)) == 1) {
			    ((uint8_t *)propData)[12] = (((uint8_t *)propData)[12] & ~0x1f) | (vpbr & 0x1f);
			}
			uint8_t vbst;
			if (syscfgCopyDataForTag('VBST', &vbst, sizeof(vbst)) == 1) {
			    ((uint8_t *)propData)[16] = vbst;
			}
		}
	}

	return 0;
}

#endif
