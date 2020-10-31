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

// BRD_REV3 is pulled down to 0 until EVT3, but we are inverting its value
#define N78_PROTO0A_BOARD		(15)
#define N78_PROTO0B_BOARD		(13)
#define N78_PROTO1_BOARD		(12)
#define N78_EVT1_BOARD			(9)
#define N78_EVT2_BOARD			(8)
// Starting with EVT3, BRD_REV3 is pulled up to 1, but we are inverting its value
#define N78_EVT3_BOARD			(7)

static u_int32_t n78_get_board_rev(void);

static uint32_t display_config;

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

	// <rdar://problem/11625584> N78 LDO7 needs to be off for units older than EVT3
	// NOTE: A board is EARLIER than an EVT3, because its board ID is larger (according to #defines above)
	if (n78_get_board_rev() > N78_EVT3_BOARD) {

		unsigned int i;
		
		const uint8_t pmu_data[] = {
			0x21, 0xB3,		// kD1881_ACTIVE2: LDO4, LDO5, LDO8, LDO9, LDO11 enabled
		};

		for (i = 0; i < sizeof(pmu_data); i += 2)
			iic_write(0, 0xe8, &pmu_data[i], 2);
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
	display_config = 0x00000964;
	return ((void *)(&display_config));
}

#if WITH_ENV

void target_setup_default_environment(void)
{
	env_set("boot-device", "nand0", 0);
	env_set("display-color-space","RGB888", 0);
	env_set("display-timing", "n41", 0);
}

bool n78_get_e75(void)
{
	return env_get_uint("e75", n78_get_board_rev() != N78_PROTO0A_BOARD) != 0;
}

#endif

static bool		gpio_board_rev_valid;
static u_int32_t	gpio_board_rev;

static u_int32_t n78_get_board_rev(void)
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
#if WITH_HW_DISPLAY_PINOT
	DTNode		*clcd_node, *backlight_node;
#endif
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

#if WITH_HW_DISPLAY_PINOT
	// Find the CLCD node
	FindNode(0, "arm-io/clcd", &clcd_node);

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


	if (FindNode(0, "product", &node)) {

		// N78a: If the NoBC syscfg key present, and it have bit 0 set
		uint32_t sysCfgData;
		int result = syscfgCopyDataForTag('NoBC', (uint8_t *)(&sysCfgData), sizeof(sysCfgData));

		int isn78a = 0;
		if (result == sizeof(sysCfgData) && (sysCfgData & 0x1) ) isn78a = 1;

		// this isn't an N78a, so get rid of the n78a-mode property
		propName = "n78a-mode";
		if (isn78a==0 && FindProperty(node, &propName, &propData, &propSize) )  propName[0] = '~';
		
		if (isn78a && FindNode(0, "product/camera", &node)) {
			// isp/camera-rear and other product/camera rear camera related properties
			// will be deleted by common code in lib/macho/dt.c.
			// Here, remove the product/camera/auto-focus entry
			propName = "auto-focus";
			if (FindProperty(node, &propName, &propData, &propSize) ) {
				propName[0] = '~';
			}

			propName = "rear-hdr-on";
			if (FindProperty(node, &propName, &propData, &propSize) ) {
				//n78a: set rear-hdr-on to 0
				memset(propData, 0, propSize );
			}

			propName = "rear-burst-image-duration";
			if (FindProperty(node, &propName, &propData, &propSize) ) {
				//n78a: set rear-burst-image-duration to 0
				memset(propData, 0, propSize );
			}
		}
	}

	// if E75 config, reconfigure everything
	if (n78_get_e75()) {
		// enable uart2
		if (FindNode(0, "arm-io/uart2", &node)) {
			propName = "AAPL,ignore";
			if (FindProperty(node, &propName, &propData, &propSize)) {
			    propName[0] = '~';
			}
		}

		// reconfigure uart1
		if (FindNode(0, "arm-io/uart1/iap", &node)) {
			propName = "name";
			if (FindProperty(node, &propName, &propData, &propSize)) {
			    strlcpy(propData, "debug-console", kPropNameLength);
			}
			propName = "function-dock_parent";
			if (FindProperty(node, &propName, &propData, &propSize)) {
			    propName[0] = '~';
			}
		}
		
		// enable tristar
		uint32_t tristarPHandle = -1;
		if (FindNode(0, "arm-io/i2c0/tristar", &node)) {
			propName = "AAPL,ignore";
			if (FindProperty(node, &propName, &propData, &propSize)) {
			    propName[0] = '~';
			}
			propName = "AAPL,phandle";
			if (FindProperty(node, &propName, &propData, &propSize)) {
			    tristarPHandle = *(uint32_t *)propData;
			}
		}
		
		// disconnect BRICK_ID from USB
		if (FindNode(0, "arm-io/otgphyctrl", &node)) {
			propName = "function-dmon_voltage";
			if (FindProperty(node, &propName, &propData, &propSize)) {
			    propName[0] = '~';
			}
		}
		
		// change dock from 30-pin to 9-pin
		uint32_t dockPHandle = -1;
		if (FindNode(0, "dock", &node)) {
			propName = "compatible";
			if (FindProperty(node, &propName, &propData, &propSize)) {
			    strlcpy(propData, "dock,9pin", kPropNameLength);
			}
			propName = "function-read_acc";
			if (FindProperty(node, &propName, &propData, &propSize)) {
			    propName[0] = '~';
			}
			propName = "function-acc_detect";
			if (FindProperty(node, &propName, &propData, &propSize)) {
			    propName[0] = '~';
			}
			propName = "interrupt-parent";
			if (FindProperty(node, &propName, &propData, &propSize)) {
			    propName[0] = '~';
			}
			propName = "interrupts";
			if (FindProperty(node, &propName, &propData, &propSize)) {
			    propName[0] = '~';
			}
			propName = "AAPL,phandle";
			if (FindProperty(node, &propName, &propData, &propSize)) {
			    dockPHandle = *(uint32_t *)propData;
			}
		}

		// use TriStar for brick detect
		if (FindNode(0, "charger", &node)) {
			propName = "function-usb_dmonitor";
			if (FindProperty(node, &propName, &propData, &propSize)) {
			    ((uint32_t *)propData)[0] = tristarPHandle;
			}
		}
	}

	// pre-EVT1 Grape clock config
	if (n78_get_board_rev() > N78_EVT1_BOARD) {
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
		// configure SoC PWM2 to be disabled
		power_gpio_configure(0, 0x09);
		gpio_configure(GPIO(7, 7), GPIO_CFG_IN);
	}

	// pre-EVT2 codec config
	if (n78_get_board_rev() > N78_EVT2_BOARD) {
		if (FindNode(0, "arm-io/spi2/audio-codec", &node)) {
		    	propName = "class-h-control";
			if (FindProperty(node, &propName, &propData, &propSize)) {
			    	((uint32_t *)propData)[0] = 0;
			}
		}
	}

	return 0;
}

#endif
