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

#define IPAD2B_PROTO0_DEV1_BOARD (0x0)
#define IPAD2B_PROTO1_BOARD (0x1)
#define IPAD2B_PROTO2_BOARD (0x2)
#define IPAD2B_EVT1_BOARD (0x3)

static uint32_t ipad2b_get_board_rev(void);

MIB_CONSTANT_PTR(kMIBTargetDisplayGammaTablePtr,kOIDTypeStruct, (void *)ipad2b_display_gamma_tables);
MIB_CONSTANT(kMIBTargetDisplayGammaTableCount,	kOIDTypeUInt32, ARRAY_SIZE(ipad2b_display_gamma_tables));
MIB_CONSTANT(kMIBTargetOsPictureScale,		kOIDTypeUInt32, 1);
MIB_CONSTANT(kMIBTargetPictureRotate,		kOIDTypeInt32,  0);
static uint32_t display_config;


void target_early_init(void)
{
	unsigned int i;

	// For proto0, go back to 2.8 V: <rdar://problem/10759719> P105 LDO10 needs to be set to 2.5v for Proto1
	if (ipad2b_get_board_rev() == IPAD2B_PROTO0_DEV1_BOARD) {
		const uint8_t pmu_data[] = {
			0x38, 0x06,
		};

		for (i = 0; i < sizeof(pmu_data); i += 2)
			iic_write(0, 0x78, &pmu_data[i], 2);
		
	}
}

void target_late_init(void)
{
}

void target_init(void)
{
#if WITH_HW_FLASH_NOR
	flash_nor_init(SPI_NOR0);
#endif

	// <rdar://problem/11492806> P105 EVT: Disable Alison LCM boost soft start to fix some grape fallouts
 	// <rdar://problem/11792109> P105's LCM Boost: SW to disable OC every time boot
	if (ipad2b_get_board_rev() <= IPAD2B_EVT1_BOARD) {

		unsigned int i;

		const uint8_t pmu_data[] = {
			0x1e, 0x02,	// bank 2
			0xd0, 0x01,	// enable test mode
			0xef, 0x10,	// disable OC for LCM boost
			0x1e, 0x01,	// bank 1
			0xee, 0x0c,	// trim_conf15 = 0x0c ; disable soft-start
			0x1e, 0x02,	// bank 2
			0xd0, 0x00,	// disable test mode
			0x1e, 0x00,	// bank 0
		};

		for (i = 0; i < sizeof(pmu_data); i += 2)
			iic_write(0, 0x78, &pmu_data[i], 2);
		
	}
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
	display_config = 0x00000944;
	// <rdar://problem/10632067> X124 LVDS panel timing parameters workaround for proto0
	if (ipad2b_get_board_rev() == IPAD2B_PROTO0_DEV1_BOARD) {
		display_config = 0x00000644;
	}
	return ((void *)(&display_config));
}

#if WITH_ENV

void target_setup_default_environment(void)
{
	env_set("boot-device", "nand0", 0);
	env_set("display-color-space","RGB888", 0);
	env_set("display-timing", "ipad2b", 0);

	// <rdar://problem/10632067> X124 LVDS panel timing parameters workaround for proto0
	if (ipad2b_get_board_rev() == IPAD2B_PROTO0_DEV1_BOARD) {
		env_set("display-timing", "ipad2b-p0", 0);
	}
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
	// Update the backlight calibration data
	if (FindNode(0, "backlight", &node)) {
		propName = "backlight-calibration";
		if (FindProperty(node, &propName, &propData, &propSize)) {
			syscfgCopyDataForTag('BLCl', propData, propSize);
		}
	}

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

	// Update the compass calibration data
	if (FindNode(0, "arm-io/i2c1/compass", &node)) {
		propName = "compass-calibration";
		if (FindProperty(node, &propName, &propData, &propSize)) {
			syscfgCopyDataForTag('CPAS', propData, propSize);
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
		propName = "accel-interrupt-calibration";
		if (FindProperty(node, &propName, &propData, &propSize)) {
			syscfgCopyDataForTag('AICl', propData, propSize);
		}
	}

	// Update camera calibration data
	if (FindNode(0, "arm-io/isp", &node)) {
		propName = "camera-calibration";
		if (FindProperty(node, &propName, &propData, &propSize)) {
			syscfgCopyDataForTag('CmCl', propData, propSize);
		}
	}

	// Updte the speaker calibration data
	if (FindNode(0, "arm-io/i2c0/audio-speaker0", &node)) {
		propName = "speaker-rdc";
		if (FindProperty(node, &propName, &propData, &propSize)) {
			syscfgCopyDataForTag('SRdc', propData, propSize);
		}
		propName = "speaker-config";
		if (FindProperty(node, &propName, &propData, &propSize)) {
			uint8_t vpbr[2];
			if (syscfgCopyDataForTag('VPBR', vpbr, sizeof(vpbr)) == 2) {
			    ((uint8_t *)propData)[12] = (((uint8_t *)propData)[12] & ~0x1f) | (vpbr[0] & 0x1f);
			    ((uint8_t *)propData)[32] = (((uint8_t *)propData)[12] & ~0x1f) | (vpbr[1] & 0x1f);
			}
			uint8_t vbst[2];
			if (syscfgCopyDataForTag('VBST', vbst, sizeof(vbst)) == 2) {
			    ((uint8_t *)propData)[16] = vbst[0];
			    ((uint8_t *)propData)[36] = vbst[1];
			}
		}
	}

	// Update USB input current limit
	if (FindNode(0, "charger", &node)) {
		propName = "usb-input-limit-max";
		if (FindProperty(node, &propName, &propData, &propSize)) {
			*(uint32_t *)propData = TARGET_MAX_USB_INPUT_CURRENT;
		}
	}


	// pre-Proto2 Grape clock config
	if (ipad2b_get_board_rev() <= IPAD2B_PROTO1_BOARD) {
		if (FindNode(0, "arm-io/spi1/multi-touch", &node)) {
			propName = "function-clock_enable";
			if (FindProperty(node, &propName, &propData, &propSize)) {
				propName[0] = '~';
			}
			propName = "function-clock_enable-pmu";
			if (FindProperty(node, &propName, &propData, &propSize)) {
				propName[21] = '\0';
			}
		}

		// configure PMU GPIO1 to be Grape clock (off until enabled)
		// configure SoC PWM2 to be disabled (
		power_gpio_configure(0, 0x11);
		gpio_configure(GPIO(7, 7), GPIO_CFG_IN);
	}

	// pre-EVT Grape LDO config
	if (ipad2b_get_board_rev() <= IPAD2B_PROTO2_BOARD) {
		if (FindNode(0, "arm-io/spi1/multi-touch", &node)) {
			propName = "function-power_ana";
			if (FindProperty(node, &propName, &propData, &propSize)) {
				propName[0] = '~';
			}
			propName = "function-power_ldo";
			if (FindProperty(node, &propName, &propData, &propSize)) {
				propName[0] = '~';
			}
			propName = "function-power_ldo-pre-evt";
			if (FindProperty(node, &propName, &propData, &propSize)) {
				propName[18] = '\0';
			}
		}	
	}
	
	// pre-EVT speaker amp interrupt assignments
	if (ipad2b_get_board_rev() <= IPAD2B_PROTO2_BOARD) {
		if (FindNode(0, "arm-io/i2c0/audio-speaker0", &node)) {
			propName = "interrupts";
			if (FindProperty(node, &propName, &propData, &propSize)) {
				*(uint32_t *)propData = 0x07;
			}
		}	
		if (FindNode(0, "arm-io/i2c0/audio-speaker1", &node)) {
			propName = "interrupts";
			if (FindProperty(node, &propName, &propData, &propSize)) {
				*(uint32_t *)propData = 0x2e;
			}
		}	
	}

	return 0;
}

#endif

static bool		gpio_board_rev_valid;
static uint32_t		gpio_board_rev;

static uint32_t ipad2b_get_board_rev(void)
{
	if(!gpio_board_rev_valid) {
		gpio_configure(GPIO_BOARD_REV0, GPIO_CFG_IN);
		gpio_configure(GPIO_BOARD_REV1, GPIO_CFG_IN);
		gpio_configure(GPIO_BOARD_REV2, GPIO_CFG_IN);

		gpio_configure_pupdn(GPIO_BOARD_REV0, GPIO_PUP);
		gpio_configure_pupdn(GPIO_BOARD_REV1, GPIO_PUP);
		gpio_configure_pupdn(GPIO_BOARD_REV2, GPIO_PUP);

		spin(100); // Wait 100us

		gpio_board_rev = 
			(gpio_read(GPIO_BOARD_REV2) << 2) |
			(gpio_read(GPIO_BOARD_REV1) << 1) |
			(gpio_read(GPIO_BOARD_REV0) << 0);

		gpio_configure(GPIO_BOARD_REV0, GPIO_CFG_DFLT);
		gpio_configure(GPIO_BOARD_REV1, GPIO_CFG_DFLT);
		gpio_configure(GPIO_BOARD_REV2, GPIO_CFG_DFLT);

		gpio_board_rev_valid = true;
	}

	return gpio_board_rev;
}

