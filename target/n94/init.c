/*
 * Copyright (C) 2010-2015 Apple Inc. All rights reserved.
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

// These are sequential revision numbers, reordered from the hardware straps
#define N94_ILLEGAL_BOARD	(255)
#define N94_PROTO1_BOARD	(10)
#define N94_PROTO1a_BOARD	(9)
#define N94_PROTO2_BOARD	(8)
#define N94_PROTO3_BOARD	(7)
#define N94_PROTO3a_BOARD	(6)
#define N94_EVT1_BOARD		(5)
#define N94_EVT1a_BOARD		(4)
#define N94_EVT2_BOARD		(3)
#define N94_DVT_BOARD		(2)
#define N94_PVT_BOARD		(1)
#define N94_PRQ_BOARD		(0)

// Convert the 4-bit hardware board rev to a monotonic software rev
static u_int8_t hw_rev_to_sw[] = {
	N94_PRQ_BOARD,
	N94_DVT_BOARD,
	N94_EVT1_BOARD,
	N94_PROTO3a_BOARD,	// also DEV3a, N78 D300 DEV
	N94_PROTO3_BOARD,	// also PreProto3, DEV3
	N94_PROTO2_BOARD,
	N94_PROTO1_BOARD,	// and DEV2
	N94_PROTO1a_BOARD,	// and DEV1
	N94_PVT_BOARD,
	N94_ILLEGAL_BOARD,
	N94_ILLEGAL_BOARD,
	N94_ILLEGAL_BOARD,
	N94_ILLEGAL_BOARD,
	N94_ILLEGAL_BOARD,
	N94_EVT2_BOARD,		// and DEV4 (probably)
	N94_EVT1a_BOARD,
};

static u_int32_t n94_get_board_rev(void);

static u_int32_t display_config;

MIB_CONSTANT(kMIBTargetOsPictureScale,	kOIDTypeUInt32, 2);
MIB_CONSTANT(kMIBTargetPictureRotate,	kOIDTypeInt32,  0);

void target_early_init(void)
{
	// OTP-AO matching code for TellurideNanshan DVT: <rdar://problem/9876366>
	if (N94_DVT_BOARD == n94_get_board_rev()) {
		uint8_t pmu_setup[][2] = {
			// buck0 and buck2 controls are set in powerconfig.h for all revs
			{ 0x18, 0x01 },	// BANKSEL=1
			{ 0xe0, 0x01 },	// Enable test mode
			{ 0xa9, 0x48 },	// 0us blanking time
			{ 0xd2, 0x33 },	// 0-180 phasing
			{ 0xa3, 0xa2 },	// reduce charger headroom by 50mV
			{ 0xe0, 0x00 },	// Disable test mode
			{ 0x18, 0x00 },	// BANKSEL=0
		};
		int entries = sizeof(pmu_setup) / (2*sizeof(uint8_t)), index;
		for (index = 0; index < entries; index++)
			iic_write(0, 0xe8, pmu_setup[index], 2);
	}
}

void target_late_init(void)
{
	if (target_config_dev()) {
		if (n94_get_board_rev() >= N94_PROTO2_BOARD)
			platform_not_supported();
	} else {
		if (n94_get_board_rev() >= N94_EVT1_BOARD)
			platform_not_supported();
	}
}

void target_init(void)
{
#if WITH_HW_FLASH_NOR
	flash_nor_init(SPI_NOR0);
#endif

	// All AP and PMU GPIOS for the radios are set by pinconfig.h 
	// and powerconfig.h.  While the AP settings are compatible with wake,
	// the BB PMU will be in reset thanks to powerconfig.h providing
	// dedicated cold boot settings.  The AppleARMFunctions in the device
	// tree will take care of the rest for a cold boot.
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
	display_config = 0x00000963;
	return ((void *)(&display_config));
}

#if WITH_ENV

void target_setup_default_environment(void)
{
	env_set("boot-device", "nand0", 0);
	env_set("display-color-space","RGB888", 0);
	env_set("display-timing", "n94", 0);
}

#endif

u_int32_t target_lookup_backlight_cal(int index)
{
	return (index == 2) ? 0x09 : 0;
}

#if WITH_DEVICETREE

int target_update_device_tree(void)
{
	u_int32_t board_rev = n94_get_board_rev();

#if WITH_HW_DISPLAY_PINOT
	DTNode		*clcd_node, *backlight_node;
#endif
	DTNode		*node;
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

#if WITH_HW_DISPLAY_PINOT
	// Find the CLCD node
	FindNode(0, "arm-io/clcd", &clcd_node);

	if (FindNode(0, "arm-io/mipi-dsim/lcd", &node)) {
		extern int pinot_update_device_tree(DTNode *pinot_node, DTNode *clcd_node, DTNode *backlight_node);

		FindNode(0, "backlight", &backlight_node);
		pinot_update_device_tree(node, clcd_node, backlight_node);
 	}
#endif

	if (board_rev > N94_EVT1a_BOARD)
	{
		// old headset circutry
		if (FindNode(0, "arm-io/i2c0/headset-switch", &node)) {
			propName = "function-ext_switch";
			if (FindProperty(node, &propName, &propData, &propSize)) {
				strlcpy(propName, "unused-ext_switch", kPropNameLength);
			}
		}
		if (FindNode(0, "arm-io/i2c0/audio0", &node)) {
			propName = "unused-hpout_ref_control";
			if (FindProperty(node, &propName, &propData, &propSize)) {
				strlcpy(propName, "function-hpout_ref_control", kPropNameLength);
			}
		}
	}
	else if (board_rev > N94_EVT1_BOARD)
	{
		// old headset circutry
		if (FindNode(0, "arm-io/i2c0/headset-switch", &node)) {
			propName = "switch-config";
			if (FindProperty(node, &propName, &propData, &propSize)) {
				*(uint32_t *)propData = 0;
			}
		}
		if (FindNode(0, "arm-io/i2c0/audio0", &node)) {
			propName = "headphone-pop-workaround";
			if (FindProperty(node, &propName, &propData, &propSize)) {
				*(uint32_t *)propData = 1;
			}
		}
	}
	else if (board_rev > N94_DVT_BOARD)
	{
		if (FindNode(0, "arm-io/i2c0/headset-switch", &node)) {
			propName = "switch-config";
			if (FindProperty(node, &propName, &propData, &propSize)) {
				*(uint32_t *)propData = 0x1a;
			}
		}
	}

	if (board_rev > N94_PRQ_BOARD)
	{
		if (FindNode(0, "arm-io/i2c0/audio0", &node)) {
			propName = "headphone-pop-workaround";
			if (FindProperty(node, &propName, &propData, &propSize)) {
				*(uint32_t *)propData |= 0x8;
			}
		}
	}

	if (board_rev > N94_PROTO3a_BOARD) {
		// <rdar://problem/8777592>
		if (FindNode(0, "arm-io/dwi", &node)) {
			propName = "dwi-version";
			if (FindProperty(node, &propName, &propData, &propSize))
				((u_int32_t *)propData)[0] = 0;
		}
		// Pulldown on BB_WAKE_AP, removed for <rdar://problem/8929181>
		power_gpio_configure(1, 0xBB);
	}

	// Update the highland-park node with acoustic transducer scale data
	if (FindNode(0, "arm-io/highland-park", &node)) {
		propName = "at-scale";
		if (FindProperty(node, &propName, &propData, &propSize)) {
			syscfgCopyDataForTag('ATSc', propData, propSize);
		}
	}
	if (FindNode(0, "arm-io/i2c0/audio0", &node)) {
		propName = "at-scale";
		if (FindProperty(node, &propName, &propData, &propSize)) {
			syscfgCopyDataForTag('ATSc', propData, propSize);
		}
	}

	// Update the als calibration data
	if (FindNode(0, "arm-io/i2c1/als", &node)) {
		propName = "alsCalibration";
		if (FindProperty(node, &propName, &propData, &propSize)) {
			syscfgCopyDataForTag('AlsC', propData, propSize);
		}
		propName = "als-colorCfg";
		if (FindProperty(node, &propName, &propData, &propSize)) {
			syscfgCopyDataForTag('ClrC', propData, propSize);
		}
	}

	// Update the compass calibration data
	if (FindNode(0, "arm-io/i2c1/compass", &node)) {
		propName = "compass-calibration";
		if (FindProperty(node, &propName, &propData, &propSize)) {
			syscfgCopyDataForTag('CPAS', propData, propSize);
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
	
	// Update the gyro calibration data
	if (FindNode(0, "arm-io/i2c2/gyro1", &node)) {
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

	// Update prox calibration data
	if (FindNode(0, "arm-io/spi1/multi-touch", &node)) {
		propName = "prox-calibration";
		if (FindProperty(node, &propName, &propData, &propSize)) {
			syscfgCopyDataForTag('PxCl', propData, propSize);
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

#endif

static u_int32_t n94_get_board_rev(void)
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

	// Convert and sanity-check
	gpio_board_rev = hw_rev_to_sw[gpio_board_rev & 0xf];
	if (N94_ILLEGAL_BOARD == gpio_board_rev)
		panic("Illegal board revision");

	return (gpio_board_rev);
}
