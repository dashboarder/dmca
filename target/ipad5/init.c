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
#include <drivers/display.h>
#if (APPLICATION_IBOOT && (PRODUCT_IBOOT || PRODUCT_IBEC))
#include <drivers/displayport/displayport.h>
#endif

#include <platform/soc/chipid.h>
// Board ids
// pick up these defines from boardid.h
#define J96_AP_BOARD_ID			TARGET_BOARD_ID_J96AP
#define J96_DEV_BOARD_ID		TARGET_BOARD_ID_J96DEV

#define J97_AP_BOARD_ID			TARGET_BOARD_ID_J97AP
#define J97_DEV_BOARD_ID		TARGET_BOARD_ID_J97DEV

// Board revs
// the following apply to both j96 and j97
#define J96_DEV1_BOARD_REV		(0x7)
#define J96_DEV2_BOARD_REV		(0x5)
#define J96_DEV3_BOARD_REV		(0x4)
#define J96_DEV3a_BOARD_REV		(0x3)
#define J96_DEV4_BOARD_REV		(0x2)	// 1GB -> 2GB migration

#define J96_PROTO0_BOARD_REV		(0x7)
#define J96_PROTO0b_BOARD_REV		(0x6)
#define J96_PROTO1_BOARD_REV		(0x5)
#define J96_PROTO1a_BOARD_REV		(0x4)
#define J96_PROTO2_BOARD_REV		(0x3)
#define J96_EVT_BOARD_REV		(0x2)
#define J96_DVT_BOARD_REV		(0x1)
#define J96_DVT_2GB_BOARD_REV		(0x0)

#define T7000_2GB_SOC_PID		(0x2)

static uint32_t ipad5_get_board_rev(void);

static bool	gpio_board_rev_valid;
static uint32_t	gpio_board_rev;
#if (APPLICATION_IBOOT && (PRODUCT_IBOOT || PRODUCT_IBEC))
static dp_t	dp;
#endif

MIB_CONSTANT(kMIBTargetOsPictureScale,	kOIDTypeUInt32, 2);
MIB_CONSTANT(kMIBTargetPictureRotate,	kOIDTypeInt32,  0);

static uint32_t ipad5_get_board_rev(void)
{
	if (!gpio_board_rev_valid) {
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

static bool board_type_preproto(void) {
	bool preproto = false;
	const uint32_t rev = ipad5_get_board_rev();

	if (target_config_dev()) {
		if (rev > J96_DEV2_BOARD_REV)
			preproto = true;	
	} else if (target_config_ap()) {
		if (rev > J96_PROTO0b_BOARD_REV)
			preproto = true;
	}

	return preproto;

}

static bool board_type_proto(void) {
	bool proto = false;
	const uint32_t rev = ipad5_get_board_rev();

	if (target_config_dev()) {
		if (rev > J96_DEV3_BOARD_REV)
			proto = true;
	} else if (target_config_ap()) {
		if (rev > J96_PROTO1_BOARD_REV)
			proto = true;
	}

	return proto;
}

static bool board_type_proto1(void) {
	bool proto1 = false;
	const uint32_t rev = ipad5_get_board_rev();

	if (target_config_dev()) {
		if (rev > J96_DEV3a_BOARD_REV)
			proto1 = true;
	} else if (target_config_ap()) {
		if (rev > J96_PROTO1a_BOARD_REV)
			proto1 = true;
	}

	return proto1;
}

static bool support_hall0(void){
	bool support = true;
	const uint32_t rev = ipad5_get_board_rev();

	if (target_config_ap()) {
		if ((rev == J96_PROTO1_BOARD_REV) || (rev == J96_PROTO1a_BOARD_REV))
			support = false;
	}

	return support;
}

static void check_board_supported(void) 
{
	bool supported = true;

	// as of june 2015, j96/j97 is moving to 2GB RAM...obsolete the rest
	// differentiated by the fues0 PID values of 2
	if ( chipid_get_pid() != T7000_2GB_SOC_PID )
	{
		supported = false;	
	    	dprintf(DEBUG_INFO, "%s-%d (chipid_get_pid()=0x%x,  T7000_2GB_SOC_PID=0xx%x)  \n", 
	    		__FUNCTION__, __LINE__ ,chipid_get_pid(),  T7000_2GB_SOC_PID );
	}

	if ( !supported) 
		platform_not_supported();
}


void target_early_init(void)
{	
#if PRODUCT_LLB || PRODUCT_IBSS
	pmgr_update_dvfm(platform_get_board_id(),ipad5_get_board_rev());
	pmgr_update_gfx_states(platform_get_board_id(),ipad5_get_board_rev());
#endif

	check_board_supported();

}

void target_late_init(void)
{
	if(board_type_preproto() || board_type_proto())
		platform_not_supported();

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
	uint32_t value = gpio_read(GPIO_WDOG_TICKLE);
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
	dp.rx_n1=		0x0;
	dp.rx_n2=		0x0;
	dp.rx_n3=		0x0;
	dp.rx_n5=		0x0;
	dp.fast_link_training =	true;
	return ((void *)(&dp));
#else
	return NULL;
#endif
}

#if WITH_ENV

void target_setup_default_environment(void)
{
	// boot-device is set in platform's init.c
	env_set("display-color-space","RGB888", 0);
	env_set("display-timing", "ipad5", 0);
}

#endif

#if WITH_DEVICETREE

int target_update_device_tree(void)
{
#if WITH_HW_DISPLAY_EDP 
	DTNode		*clcd_node = NULL, *backlight_node = NULL, *lcd_node = NULL;
	DTNode		*lpdp_node = NULL;
	DTNode		*node;
#endif

	uint32_t        propSize;
	char            *propName, *propNamePOR;
	void            *propData;

#if WITH_HW_DISPLAY_EDP
	if (FindNode(0, DP_DTPATH, &node)) {
		extern int edp_update_device_tree(DTNode *edp_node, DTNode *lcd_node, DTNode *clcd_node, DTNode *backlight_node);
		FindNode(0, "arm-io/disp0", &clcd_node);
		FindNode(0, DP_DTPATH "/lcd", &lcd_node);
		FindNode(0, "backlight", &backlight_node);
		edp_update_device_tree(node, lcd_node, clcd_node, backlight_node);

		extern int lpdp_phy_update_device_tree(DTNode *lpdp_node);
		FindNode(0, DPPHY_DTPATH, &lpdp_node);
		lpdp_phy_update_device_tree(lpdp_node);
	}
#endif
    
	// Update the codec node with acoustic transducer scale data
	{
		uint8_t atscData[20];

		if (syscfgCopyDataForTag('ATSc', atscData, sizeof(atscData)) > 0) {
			// Update codec
			if (FindNode(0, "arm-io/spi2/audio-codec", &node)) {
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
    
	if (board_type_proto1()) {
		if (FindNode(0, "arm-io/uart7/oscar", &node)) {
			propName = "function-oscar_power1_proto1";
			propNamePOR = "function-oscar_power1";
			if (FindProperty(node, &propName, &propData, &propSize) &&
				FindProperty(node, &propNamePOR, &propData, &propSize)) {
					strlcpy(propName, "function-oscar_power1", kPropNameLength);
					propNamePOR[0] = '~';
			}

		}
	}

	// Update the speaker calibration data
	if (FindNode(0, "arm-io/i2c2/audio-speaker0", &node)) {
		propName = "speaker-calib";
		if (FindProperty(node, &propName, &propData, &propSize)) {
			syscfgCopyDataForTag('SpCl', propData, propSize);
		}
	}
	
	// Update the als calibration data for all nodes that may be present
	if (FindNode(0, "arm-io/i2c3/als1", &node)) {
		propName = "alsCalibration";
		if (FindProperty(node, &propName, &propData, &propSize)) {
			syscfgCopyDataForTag('LSCI', propData, propSize);
		}
	}
	if (FindNode(0, "arm-io/i2c3/als2", &node)) {
		propName = "alsCalibration";
		if (FindProperty(node, &propName, &propData, &propSize)) {
			syscfgCopyDataForTag('LSCI', propData, propSize);
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
	if (FindNode(0, "arm-io/uart7/oscar/compass", &node)) {
		propName = "compass-calibration";
		if (FindProperty(node, &propName, &propData, &propSize)) {
			syscfgCopyDataForTag('CPAS', propData, propSize);
		}
		propName = "compass-orientation";
		if (FindProperty(node, &propName, &propData, &propSize)) {
			syscfgCopyDataForTag('CRot', propData, propSize);
		}
	}

	// Update the gyro calibration data
	if (FindNode(0, "arm-io/uart7/oscar/gyro", &node)) {
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
	if (FindNode(0, "arm-io/uart7/oscar/accelerometer", &node)) {
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

	// Update pressure sensor calibration data
	if (FindNode(0, "arm-io/uart7/oscar/pressure", &node)) {
		propName = "pressure-offset-calibration";
		if (FindProperty(node, &propName, &propData, &propSize)) {
			syscfgCopyDataForTag('SPPO', propData, propSize);
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
#endif // WITH_PAINT
