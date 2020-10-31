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

#define J2_PROTO0_BOARD		(0)	// same as REV1 Dev Board
#define J2_PROTO1_LOCAL_BOARD	(1)
#define J2_PROTO1_CHINA_BOARD	(2)
#define J2_PROTO2_BOARD		(3)     // DEV2 board also
#define J2_PRE_EVT_BOARD	(4)

static u_int32_t ipad3_get_board_rev(void);
static void amelia_init();

#if (APPLICATION_IBOOT && (PRODUCT_IBOOT || PRODUCT_IBEC))
#include <drivers/displayport/displayport.h>
static dp_t dp;
#endif

MIB_CONSTANT(kMIBTargetOsPictureScale,	kOIDTypeUInt32, 2);
MIB_CONSTANT(kMIBTargetPictureRotate,	kOIDTypeInt32,  90);

void target_early_init(void)
{
}

void target_late_init(void)
{
	if (target_config_ap()) {
		// Deprecate Proto0/1/2 boards
		// Since board revs are shared between dev boards and enclosures,
		// depreate only enclosures
		if (ipad3_get_board_rev() < J2_PRE_EVT_BOARD)
			platform_not_supported();
	}
}

void target_init(void)
{
	// Cold-boot defaults for GPIOs where pinconfig.h differs,
	// and it can't wait for the AppleARMFunction creation.

#if WITH_HW_FLASH_NOR
	flash_nor_init(SPI_NOR0);
#endif

	amelia_init();
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
#if (APPLICATION_IBOOT && (PRODUCT_IBOOT || PRODUCT_IBEC))
	dp.mode	=		0x1;
	dp.type	=		0x1;
	dp.min_link_rate =	0x6;
	dp.max_link_rate =	0x6;
	dp.lanes =		0x4;
	dp.ssc =		0x0;
	dp.alpm =		0x0;
	dp.vrr_enable =		0x0;
	dp.vrr_on =		0x0;
	return ((void *)(&dp));
#else
	return NULL;
#endif
}

#if WITH_ENV

void target_setup_default_environment(void)
{
	env_set("boot-device", "nand0", 0);
	env_set("display-color-space","RGB888", 0);
	env_set("display-timing", "ipad3", 0);
}

#endif

#if WITH_DEVICETREE

int target_update_device_tree(void)
{
	DTNode		*node;
#if WITH_HW_DISPLAY_EDP	
	DTNode		*clcd_node = NULL, *backlight_node = NULL, *lcd_node = NULL;
#endif	
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

#if WITH_HW_DISPLAY_EDP
        if (FindNode(0, DP_DTPATH, &node)) {
                extern int edp_update_device_tree(DTNode *edp_node, DTNode *lcd_node, DTNode *clcd_node, DTNode *backlight_node);
        	FindNode(0, "arm-io/clcd", &clcd_node);
        	FindNode(0, DP_DTPATH "/lcd", &lcd_node);
                FindNode(0, "backlight", &backlight_node);
                edp_update_device_tree(node, lcd_node, clcd_node, backlight_node);
        }
#endif

	if (target_config_dev()) {
		// Below is needed for DEV1 boards

		// Support for A0 Amelia
		if (ipad3_get_board_rev() <= J2_PROTO1_CHINA_BOARD) {
			// <rdar://problem/9001989>
			if (FindNode(0, "arm-io/dwi", &node)) {
				propName = "dwi-version";
				if (FindProperty(node, &propName, &propData, &propSize))
					((u_int32_t *)propData)[0] = 0;
			}
		}

		// Proto0 and Proto1-Local support
		if (ipad3_get_board_rev() <= J2_PROTO1_LOCAL_BOARD) {
			// update wlan node, HSIC_WLAN_READY pin is different
			if (FindNode(0, WIFI_DTPATH, &node)) {
				propName = "function-device_ready";
				if (FindProperty(node, &propName, &propData, &propSize)) {
					// structure looks like this:
					// UInt32 gpioPHandle;
					// UInt32 gpioSignature;
					// UInt32 gpioIndex;
					// UInt8  gpioDirection;
					// UInt8  gpioPolarity;
					// UInt8  gpioDefault;
					// UInt8  gpioAltFunction;
					((u_int32_t *)propData)[2] = 0x00001205;	// GPIO_3V_1	-> HSIC_WLAN_RDY
				}
				propName = "interrupts";
				if (FindProperty(node, &propName, &propData, &propSize)) {
					// structure looks like this:
					// UInt32 vectorNumber;
					// UInt32 vectorType;
					((u_int32_t *)propData)[0] = 0x00000095;	// GPIO_3V_1	-> HSIC_WLAN_RDY
				}
			}
		}
	}
		
	// Update the bottom prox calibration data
	if (FindNode(0, "arm-io/i2c1/prox", &node)) {
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
		propName = "gyro-sens-matrix-inverse";
		if (FindProperty(node, &propName, &propData, &propSize)) {
			syscfgCopyDataForTag('GSCi', propData, propSize);
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

	// Update the gyro1 calibration data
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
		propName = "gyro-sens-matrix-inverse";
		if (FindProperty(node, &propName, &propData, &propSize)) {
			syscfgCopyDataForTag('GSCi', propData, propSize);
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

	return 0;
}

#endif

static u_int32_t ipad3_get_board_rev(void)
{
	u_int32_t gpio_board_rev;

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

	return (gpio_board_rev & 0x7);
}

static void amelia_init() 
{
	uint32_t board_rev;

	struct pmu_iic_data {
		uint8_t addr;
		uint8_t	data;
	};
	struct pmu_iic_data amelia_a0_data[] = {
		{ 0x22, 0x02 },		// BANKSEL: select bank 2
		{ 0xd0, 0x01 },		// TESTMODE: enable test mode
		{ 0x22, 0x01 },		// BANKSEL: select bank 1
		{ 0xa9, 0x00 },		// WLED_CONTROL4: set WLED to IDAC always, WLED_IRATE to 17.5mA
		{ 0x22, 0x02 },		// BANKSEL: select bank 2
		{ 0xd0, 0x00 },		// TESTMODE: disable test mode
		{ 0x22, 0x00 },		// BANKSEL: select bank 1
	};

	struct pmu_iic_data amelia_a1_data[] = {
		{ 0x22, 0x02 },		// BANKSEL: select bank 2
		{ 0xd0, 0x01 },		// TESTMODE: enable test mode
		{ 0x22, 0x01 },		// BANKSEL: select bank 1
		{ 0xa9, 0x79 },		// WLED_CONTROL4: set WLED to IDAC always, WLED_IRATE to 18.5mA
		{ 0xaa, 0x86 }, 	// WLED_CONTROL5: set ALT_MIN_IDAC_CODE 
		{ 0xab, 0x00 }, 	// WLED_CONTROL6: set ALT_PWM_START_CODE
		{ 0xac, 0x18 }, 	// WLED_CONTROL7: set ALT_DITHER_EN and ALT_DITHER_ALLOW
		{ 0xb2, 0xee },		// BUCK_CTRL0: operate into Synchronous mode (forced PWM mode)
		{ 0xb4, 0xee },		// BUCK_CTRL2: operate into Synchronous mode (forced PWM mode)
		{ 0x22, 0x02 },		// BANKSEL: select bank 2
		{ 0xd0, 0x00 },		// TESTMODE: disable test mode
		{ 0x22, 0x00 },		// BANKSEL: select bank 1
	};
	struct pmu_iic_data *data;
	uint8_t entries;
	uint8_t i;

	board_rev = ipad3_get_board_rev();
	
	// <rdar://problem/9059055> J1/J2 default display brightness too high		
	// <rdar://problem/9126569> J1/J2 Pre-Proto MLBs and Dev Board Rev.1 Brightness Too High
	// <rdar://problem/9478595> Switch J1/J2 backlight over to enhanced hybrid mode
	if (target_config_dev() && (board_rev == J2_PROTO0_BOARD || board_rev == J2_PROTO1_LOCAL_BOARD)) {
		data = amelia_a0_data;
		entries = sizeof(amelia_a0_data)/sizeof(amelia_a0_data[0]);
	} else {
		data = amelia_a1_data;
		entries = sizeof(amelia_a1_data)/sizeof(amelia_a1_data[0]);
	}

	// Amelia I2C bus: 0, Amelia Write address: 0x78
	for (i = 0; i < entries; i++)
		iic_write(0, 0x78, (const void *)&data[i], 2);
}
