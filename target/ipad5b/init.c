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
#include <drivers/power.h>
#include <drivers/iic.h>
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

// Board ids
#define J81_AP_BOARD_ID			(0x6)
#define J81_DEV_BOARD_ID		(0x7)

#define J82_AP_BOARD_ID			(0x2)
#define J82_DEV_BOARD_ID		(0x3)

// Board revs
#define J81_DEV1_BOARD_REV		(0x2)
#define J81_PROTO2_BOARD_REV		(0x4)
#define J81_PREEVT_BOARD_REV		(0x5)

static uint32_t ipad5b_get_board_rev(void);

typedef enum {
	DISPLAY_UNKNOWN = 0,
	DISPLAY_J72 = 2,
	DISPLAY_POR = 3,
} ipad5b_display_t;

static bool	gpio_board_rev_valid;
static uint32_t	gpio_board_rev;
static bool	gpio_display_id_valid;
static ipad5b_display_t  display_panel;

// Display panel id sense pins
#define GPIO_DISPLAY_ID0	GPIO( 18, 1)	// 145: I2S4_MCK -> GPIO_DISPLAY_ID0 (JA118)
#define GPIO_DISPLAY_ID1	GPIO( 5, 3)	// 43 : I2S1_MCK -> GPIO_DISPLAY_ID1 (JA119)

MIB_CONSTANT(kMIBTargetOsPictureScale,	kOIDTypeUInt32, 2);
MIB_CONSTANT(kMIBTargetPictureRotate,	kOIDTypeInt32,  0);
#if (APPLICATION_IBOOT && (PRODUCT_IBOOT || PRODUCT_IBEC))
static dp_t dp;
#endif

void ipad5b_detect_display(void) 
{
	/*-----------------------------
	  Display ID [1:0] = [GPIO43: GPIO145]

	  Display ID [1:0]
	  POR			0b11		(no jumpers stuffed)
	  J72			0b10		(JA118 stuffed)
	  Unused		0b01		(JA119 stuffed)
	  Unused		0b00		(both JA118 and JA119 stuffed)

	  S/W Read Flow
	  1. Set GPIO as input
	  2. Enable PU and disable PD
	  3. Read
	  ------------------------------*/

	static const char *display_panel_name[] = {
		[DISPLAY_UNKNOWN] = "Unknown",
		[DISPLAY_J72] = "J72",
		[DISPLAY_POR] = "POR",
	};

	gpio_configure(GPIO_DISPLAY_ID0, GPIO_CFG_IN);
	gpio_configure(GPIO_DISPLAY_ID1, GPIO_CFG_IN);

	gpio_configure_pupdn(GPIO_DISPLAY_ID0, GPIO_PUP);
	gpio_configure_pupdn(GPIO_DISPLAY_ID1, GPIO_PUP);
	
	spin(100); // Wait 100us
	
	display_panel = (gpio_read(GPIO_DISPLAY_ID1) << 1) | gpio_read(GPIO_DISPLAY_ID0);
	
	dprintf(DEBUG_INFO, "Display panel = %s\n",
		display_panel_name[display_panel]);
	
	gpio_configure(GPIO_DISPLAY_ID0, GPIO_CFG_DFLT);
	gpio_configure(GPIO_DISPLAY_ID1, GPIO_CFG_DFLT);
	gpio_display_id_valid = true;
}

static uint32_t ipad5b_get_board_rev(void)
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

uint32_t target_get_display_panel_type(void)
{
	if (!gpio_display_id_valid) {
		ipad5b_detect_display();
	}
	return display_panel;
}

static void target_fixup_pmu(void)
{
#if WITH_HW_POWER
int pmu_get_data(int dev, uint16_t reg, uint8_t *byte);
int pmu_set_data(int dev, uint16_t reg, uint8_t byte, bool do_confirm);
	int rc;
	uint8_t trc;

	rc=pmu_get_data(0, 0x0001, &trc);
	if ( rc<0 ) {
		dprintf(DEBUG_CRITICAL, "PMU: cannot read TRC (%d)\n", rc);			
	} else if ( trc==0x6 || trc==0x7 ) {		
		rc=pmu_set_data(0, 0x7000, 0x1d, 0); 	// test mode
		if ( rc<0 ) {
			dprintf(DEBUG_INFO, "PMU: cannot enter test mode (%d)\n", rc);			
		} else {
			rc=pmu_set_data(0, 0x002A, 0x0B, 0); 
			if ( rc<0 ) dprintf(DEBUG_INFO, "PMU: cannot change PRE_UVLO_ADJ (%d)\n", rc);

			rc=pmu_set_data(0, 0x7000, 0x00, 0); 
			if ( rc<0 ) panic("PMU: cannot exit test mode (%d)", rc);
		}
	}
#endif
}


void target_early_init(void)
{
	if (target_config_dev()) {
		// Read the panel id early
		target_get_display_panel_type();
	} else {
		if (ipad5b_get_board_rev() < J81_PROTO2_BOARD_REV) {
			platform_not_supported();
		}
	}
#if PRODUCT_LLB || PRODUCT_IBSS
	pmgr_update_dvfm(platform_get_board_id(),ipad5b_get_board_rev());
	pmgr_update_gfx_states(platform_get_board_id(),ipad5b_get_board_rev());
	target_fixup_pmu();
#endif
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
	env_set("display-timing", "ipad4", 0);
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

	if (target_config_dev()) {
		// multi-touch compatibilty to j72
		if (FindNode(0, MULTITOUCH_DTPATH, &node)) {
			propName = "compatible";
			if (FindProperty(node, &propName, &propData, &propSize)) {
				memset(propData, 0, propSize);
				if (DISPLAY_J72 == target_get_display_panel_type()) {
					strlcpy(propData, "multi-touch,j72", propSize);
				} else {
					strlcpy(propData, "multi-touch,j82", propSize);
				}
			}
		}

		if (ipad5b_get_board_rev() == J81_DEV1_BOARD_REV) {
			// mesa pwr hooked up differently on boards with revs earlier than 3
			if (FindNode(0, "arm-io/spi2/mesa", &node)) {
				propName = "function-mesa_pwr_preprotorev3";
				propNamePOR = "function-mesa_pwr";
				if (FindProperty(node, &propName, &propData, &propSize) &&
				    FindProperty(node, &propNamePOR, &propData, &propSize)) {
					strlcpy(propName, "function-mesa_pwr", kPropNameLength);
					propNamePOR[0] = '~';
				}
			}
		}
	}

	// Update the codec node with acoustic transducer scale data
	{
		uint8_t atscData[20];

		if (syscfgCopyDataForTag('ATSc', atscData, sizeof(atscData)) > 0) {
			// Update codec
			if (FindNode(0, "arm-io/spi1/audio-codec", &node)) {
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

	// Update the speaker calibration data
	if (FindNode(0, "arm-io/i2c1/audio-speaker0", &node)) {
		propName = "speaker-calib";
		if (FindProperty(node, &propName, &propData, &propSize)) {
			syscfgCopyDataForTag('SpCl', propData, propSize);
		}
	}
	
	// Update the als calibration data for all nodes that may be present on the dev board
	if (FindNode(0, "arm-io/i2c2/als1", &node)) {
		propName = "alsCalibration";
		if (FindProperty(node, &propName, &propData, &propSize)) {
			syscfgCopyDataForTag('LSCI', propData, propSize);
		}
	}
	if (FindNode(0, "arm-io/i2c2/als2", &node)) {
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
	if (FindNode(0, "arm-io/uart8/oscar/compass", &node)) {
		propName = "compass-calibration";
		if (FindProperty(node, &propName, &propData, &propSize)) {
			syscfgCopyDataForTag('CPAS', propData, propSize);
		}
	}

	// Update the gyro calibration data
	if (FindNode(0, "arm-io/uart8/oscar/gyro", &node)) {
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
	if (FindNode(0, "arm-io/uart8/oscar/accelerometer", &node)) {
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
	if (FindNode(0, "arm-io/uart8/oscar/pressure", &node)) {
		propName = "pressure-offset-calibration";
		if (FindProperty(node, &propName, &propData, &propSize)) {
			syscfgCopyDataForTag('SPPO', propData, propSize);
		}
	}

	// Update charger calibration data
	if (FindNode(0, "charger", &node)) {
		propName = "usb-input-limit-calibration";
		if (FindProperty(node, &propName, &propData, &propSize)) {
			syscfgCopyDataForTag('CBAT', propData, propSize);
		}
	}
	
	// Update halleffect button -- remove when pre-EVT hardware deprecated
	if (FindNode(0, "buttons", &node)) {
		if (ipad5b_get_board_rev() < J81_PREEVT_BOARD_REV)	{
			propName = "function-button_halleffect-preevt";
			propNamePOR = "function-button_halleffect";
			if (FindProperty(node, &propName, &propData, &propSize) &&
			    FindProperty(node, &propNamePOR, &propData, &propSize)) {
				strlcpy(propName, "function-button_halleffect", kPropNameLength);
				propNamePOR[0] = '~';
			}
		}
	}

	// WLAN Capri A0 workarounds -- !!!FIXME!!! Remove this when Capri A0 is deprecated
	// <rdar://problem/16597296> Remove Capri A0 PCIe hacks
	if (FindNode(0, "arm-io/apcie/pci-bridge1/wlan", &node)) {
		if (chipid_get_chip_revision() > CHIP_REVISION_A0) {
			// Disable workarounds for Capri A1 and later.
			propName = "acpie-l1ss-workaround";
			if (FindProperty(node, &propName, &propData, &propSize)) {
				propName[0] = '~';
			}
			propName = "pci-wake-l1pm-disable";
			if (FindProperty(node, &propName, &propData, &propSize)) {
				propName[0] = '~';
			}
		} else {
			// Fix up the hacks for Capri A0
			propName = "pci-l1pm-control";
			if (FindProperty(node, &propName, &propData, &propSize)) {
				propName[0] = '~';
			}
			propName = "pci-l1pm-control-a0";
			if (FindProperty(node, &propName, &propData, &propSize)) {
				propName[16] = '\0';
			}
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
