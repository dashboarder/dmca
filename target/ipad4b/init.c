/*
 * Copyright (C) 2012 - 2015 Apple Inc. All rights reserved.
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
#include <lib/paint.h>
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

#if SUB_TARGET_J85 || SUB_TARGET_J86 || SUB_TARGET_J87
#define IPAD4B_DEV1_PROTO0_REV		(0x7)
#define IPAD4B_DEV2_PROTO1_REV		(0x6)		// with Beacon backlight controller
#define IPAD4B_EVT_REV			(0x5)		// removed Beacon
#endif

#if SUB_TARGET_J85M || SUB_TARGET_J86M || SUB_TARGET_J87M
#define IPAD4BM_DEV1_PROTO0_REV		(0x7)
#endif

static bool		gpio_board_rev_valid;
static u_int32_t	gpio_board_rev;
#if (APPLICATION_IBOOT && (PRODUCT_IBOOT || PRODUCT_IBEC))
#include <drivers/displayport/displayport.h>
static dp_t		dp;
#endif

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

static u_int32_t ipad4b_get_board_rev(void)
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
	// <rdar://problem/13372834> IRATE needs to be 25 mA on J85
	//	WLED IRATE is in the low 3 bits, and 0x3 corresponds to 25mA. If OTP is
	//	not properly programmed, use test mode to change to 25mA.
	uint8_t value;
	iic_read(0, 0x78, (uint8_t[]){0x06,0x07}, 2, &value, 1, IIC_NORMAL);
	if ((value & 0x7) != 3) {
		iic_write(0, 0x78, (uint8_t[]){0x70,0x00,0x1d},3);
		iic_write(0, 0x78, (uint8_t[]){0x06,0x07,(value & 0xf8) | 0x3}, 3);
		iic_write(0, 0x78, (uint8_t[]){0x70,0x00,0x00},3);
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

bool beacon_supported()
{
	bool supported = false;
#if SUB_TARGET_J85 || SUB_TARGET_J86 || SUB_TARGET_J87
	supported = (ipad4b_get_board_rev() == IPAD4B_DEV2_PROTO1_REV) ? true:false;
#endif
	return supported;
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
	env_set("display-timing", "ipad4b", 0);
}

#endif // WITH_ENV

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

	if (beacon_supported()) {
		panic("Unsupported board revision");
	}

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


	// Update the bottom prox calibration data
	if (FindNode(0, "arm-io/i2c3/prox", &node)) {
		propName = "prox-calibration-blob";
		if (FindProperty(node, &propName, &propData, &propSize)) {
			syscfgCopyDataForTag('CPCl', propData, propSize);
		}
	}

	// Update the als calibration data
	if (FindNode(0, "arm-io/i2c3/als", &node)) {
		propName = "alsCalibration";
		if (FindProperty(node, &propName, &propData, &propSize)) {
			syscfgCopyDataForTag('LSCI', propData, propSize);
		}
	}

	// Update charger calibration data
	if (FindNode(0, "charger", &node)) {
		propName = "usb-input-limit-calibration";
		if (FindProperty(node, &propName, &propData, &propSize)) {		
            if ( syscfgCopyDataForTag('CBAT', propData, propSize)==20 ) {
        		((uint32_t*)propData)[4]=0;
			}
		}
	}


	// Update the speaker calibration data
	if (FindNode(0, "arm-io/i2c2/audio-speaker0", &node)) {
		propName = "speaker-rdc";
		if (FindProperty(node, &propName, &propData, &propSize)) {
			syscfgCopyDataForTag('SRdc', propData, propSize);
		}
				
		propName = "speaker-config";
		if (FindProperty(node, &propName, &propData, &propSize)) {
			uint8_t vpbr[2]; // calibrated @ 3.1V
			if (syscfgCopyDataForTag('VPBR', vpbr, sizeof(vpbr)) == 2) {
			    ((uint8_t *)propData)[12] = (((uint8_t *)propData)[12] & ~0x1f) | ( (vpbr[0]-1) & 0x1f);
			    ((uint8_t *)propData)[32] = (((uint8_t *)propData)[32] & ~0x1f) | ( (vpbr[1]-1) & 0x1f);
			}
			uint8_t vbst[2];
			if (syscfgCopyDataForTag('VBST', vbst, sizeof(vbst)) == 2) {
			    ((uint8_t *)propData)[16] = vbst[0];
			    ((uint8_t *)propData)[36] = vbst[1];
			}
		}
	}

#if SUB_TARGET_J85M || SUB_TARGET_J86M || SUB_TARGET_J87M

	// Update the X162 calibration data
	if (FindNode(0, "arm-io/spi3/mesa", &node)) {
		propName = "calibration-blob";
		if (FindProperty(node, &propName, &propData, &propSize)) {
			syscfgCopyDataForTag('FSCl', propData, propSize);
		}
		propName = "modulation-ratio";
		if (FindProperty(node, &propName, &propData, &propSize)) {
			syscfgCopyDataForTag('NvMR', propData, propSize);
		}
	}
#endif

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

