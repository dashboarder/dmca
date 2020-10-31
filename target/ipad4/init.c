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
#include <platform/soc/display_color_manager_tables.h>

#define IPAD4_DEV1_PROTO0_BOARD		(0x0)
#define IPAD4_PROTO0_TS2_BOARD		(0x1)
#define IPAD4_PROTO1_TS2_BOARD		(0x2)
#define IPAD4_PROTO1_TS1_BOARD		(0x3)
#define IPAD4_PROTO2_BOARD		(0x5)
#define IPAD4_EVT1_BOARD		(0x6)
#define IPAD4_DVT_BOARD			(0x7)

static bool		gpio_board_rev_valid;
static u_int32_t	gpio_board_rev;

MIB_CONSTANT(kMIBTargetOsPictureScale,	kOIDTypeUInt32, 2);
MIB_CONSTANT(kMIBTargetPictureRotate,	kOIDTypeInt32,  0);
MIB_CONSTANT(kMIBTargetDisplayCMEngammaTablePtr,	kOIDTypeStruct, (void *)linear_rgb_engamma_tables);
MIB_CONSTANT(kMIBTargetDisplayCMEngammaTableCount,	kOIDTypeUInt32, ARRAY_SIZE(linear_rgb_engamma_tables));
MIB_CONSTANT(kMIBTargetDisplayCMDegammaTablePtr,	kOIDTypeStruct, (void *)linear_rgb_degamma_tables);
MIB_CONSTANT(kMIBTargetDisplayCMDegammaTableCount,	kOIDTypeUInt32, ARRAY_SIZE(linear_rgb_degamma_tables));
MIB_CONSTANT(kMIBTargetDisplayCMMatrixTablePtr,		kOIDTypeStruct, (void *)linear_identity_matrix_tables);
MIB_CONSTANT(kMIBTargetDisplayCMMatrixTableCount,	kOIDTypeUInt32, ARRAY_SIZE(linear_identity_matrix_tables));

// Don't panic for these targets because of <rdar://problem/14236421>.
MIB_CONSTANT(kMIBTargetPanicOnUnknownDclrVersion,	kOIDTypeBoolean, false);
#if (APPLICATION_IBOOT && (PRODUCT_IBOOT || PRODUCT_IBEC))
#include <drivers/displayport/displayport.h>
static dp_t dp;
#endif

static u_int32_t ipad4_get_board_rev(void)
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
	unsigned i;

	// <rdar://problem/13712093> and <rdar://problem/13840183> J72 : Change LDO5 voltage for EVT build
        if (ipad4_get_board_rev() == IPAD4_EVT1_BOARD) {
                const uint8_t pmu_data[] = {
                        0x03, 0x28, 0x47,
                };

                for (i = 0; i < sizeof(pmu_data); i += 3)
                        iic_write(0, 0x78, &pmu_data[i], 3);
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
	env_set("boot-device", "asp_nand", 0);
	env_set("display-color-space","RGB888", 0);
	env_set("display-timing", "ipad4", 0);
}

#endif // WITH_ENV

bool target_has_tristar2(void)
{
	return ipad4_get_board_rev() != IPAD4_DEV1_PROTO0_BOARD && ipad4_get_board_rev() != IPAD4_PROTO1_TS1_BOARD;
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

#if WITH_HW_DISPLAY_EDP
        if (FindNode(0, DP_DTPATH, &node)) {
                extern int edp_update_device_tree(DTNode *edp_node, DTNode *lcd_node, DTNode *clcd_node, DTNode *backlight_node);
        	FindNode(0, "arm-io/disp0", &clcd_node);
        	FindNode(0, DP_DTPATH "/lcd", &lcd_node);
                FindNode(0, "backlight", &backlight_node);
                edp_update_device_tree(node, lcd_node, clcd_node, backlight_node);
        }
#endif

	// Update the codec node with acoustic transducer scale data
	{
		uint8_t atscData[20];

		if (syscfgCopyDataForTag('ATSc', atscData, sizeof(atscData)) > 0) {
			// Update codec
			if (FindNode(0, "arm-io/spi3/audio-codec", &node)) {
				propName = "at-scale-imic";
				if (FindProperty(node, &propName, &propData, &propSize)) {
					memcpy(propData, &atscData[0], sizeof(uint32_t));
				}
				propName = "at-scale-smic";
				if (FindProperty(node, &propName, &propData, &propSize)) {
					memcpy(propData, &atscData[4], sizeof(uint32_t));
				}
			}
		}
	}

	// Update the backlight calibration data
	if (FindNode(0, "backlight", &node)) {
		propName = "backlight-calibration";
		if (FindProperty(node, &propName, &propData, &propSize)) {
			syscfgCopyDataForTag('BLCl', propData, propSize);
		}
	}

	// Update the compass calibration data
	if (FindNode(0, "arm-io/uart4/oscar/compass", &node)) {
		propName = "compass-calibration";
		if (FindProperty(node, &propName, &propData, &propSize)) {
			syscfgCopyDataForTag('CPAS', propData, propSize);
		}
	}
    
	// Update the gyro calibration data
	if (FindNode(0, "arm-io/uart4/oscar/gyro", &node)) {
		propName = "gyro-orientation";
		if (FindProperty(node, &propName, &propData, &propSize)) {
			syscfgCopyDataForTag('GRot', propData, propSize);
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
	if (FindNode(0, "arm-io/uart4/oscar/accelerometer", &node)) {
		propName = "accel-orientation";
		if (FindProperty(node, &propName, &propData, &propSize)) {
			syscfgCopyDataForTag('ARot', propData, propSize);
		}
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

	{
		char *prox_node_name = "arm-io/i2c3/prox-i2c3";
		char *als_node_name = "arm-io/i2c3/als-i2c3";

		if (ipad4_get_board_rev() < IPAD4_DVT_BOARD) {
			prox_node_name = "arm-io/i2c0/prox-i2c0";
			als_node_name = "arm-io/i2c0/als-i2c0";
		}

		// Update the bottom prox calibration data
		if (FindNode(0, prox_node_name, &node)) {
			propName = "AAPL,ignore";
			if (FindProperty(node, &propName, &propData, &propSize)) {
				propName[0] = '~';
			}

			propName = "prox-calibration-blob";
			if (FindProperty(node, &propName, &propData, &propSize)) {
				syscfgCopyDataForTag('CPCl', propData, propSize);
			}
		}

		// Update the als calibration data
		if (FindNode(0, als_node_name, &node)) {
			propName = "AAPL,ignore";
			if (FindProperty(node, &propName, &propData, &propSize)) {
				propName[0] = '~';
			}

			propName = "alsCalibration";
			if (FindProperty(node, &propName, &propData, &propSize)) {
				syscfgCopyDataForTag('LSCI', propData, propSize);
			}
		}
	}

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

#if WITH_PAINT

// The default background is expected to to be black and the artwork is expected
// to be white. This arrangement will be inverted depending upon the cover glass
// color of the device.

// Sample DClr_override values for testing devices without DClr syscfg entries (enclosure/cover glass):
// gray/black:		setenv DClr_override 000200009B9899003C3B3B0000000000
// silver/white:	setenv DClr_override 00020000D8D9D700E3E4E10000000000

static color_policy_invert_t target_cover_glass_color_table[] = {
	{ RGB( 59,  59,  60), false },	// Black - black background, white logo
	{ RGB(225, 228, 227), true  },	// White - white background, black logo
};

color_policy_t *target_color_map_init(enum colorspace cs, color_policy_t *color_policy)
{
	// Must have a color policy structure passed in.
	if (color_policy == NULL)
		goto fail;

	// We only support the RGB888 colorspace.
	if (cs != CS_RGB888)
		goto fail;

	color_policy->policy_type  = COLOR_MAP_POLICY_INVERT;
	color_policy->color_table  = (void *)target_cover_glass_color_table;
	color_policy->color_count  = ARRAY_SIZE(target_cover_glass_color_table);
	color_policy->map_color    = NULL;	// Use standard remapper

	return color_policy;

fail:
	return NULL;
}
#endif	// WITH_PAINT

