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
#include <drivers/power.h>
#include <drivers/gasgauge.h>
#include <drivers/iic.h>
#include <lib/env.h>
#include <lib/mib.h>
#include <lib/paint.h>
#include <lib/syscfg.h>
#include <platform.h>
#include <platform/gpiodef.h>
#include <platform/soc/chipid.h>
#include <target.h>
#include <platform/soc/hwclocks.h>
#if WITH_HW_DISPLAY_PMU
#include <drivers/display_pmu.h>
#endif

#define N59_AP_BOARD_ID			(0x18)	// AP board ID
#define N59_DEV_BOARD_ID		(0x19)	// DEV board ID

#define N59_DEV_BOARD_REV		(0xF)

#define N59_PROTOMLB1			(0xF)

// Combined Board Id/Board Revision
// IMPORTANT:	IF YOU CHANGE THE ORDER OF THESE ENUMERATORS YOU MUST CHECK
//		FOR ANY > or < RELATIONSHIPS THAT MAY NEED TO ALSO CHANGE.
typedef enum {
	BOARD_TYPE_UNKNOWN		= 0,
	BOARD_TYPE_N59_DEV1		= 1,
	BOARD_TYPE_N59_PROTOMLB1 	= 2,
} board_type;

static board_type iphone7_get_board_type(void);
static uint32_t iphone7_get_board_rev(void);

static bool	gpio_board_rev_valid;
static uint32_t	gpio_board_rev;

static uint32_t display_config;

MIB_CONSTANT(kMIBTargetBacklight0I2CBus,	kOIDTypeUInt32, 0);
MIB_CONSTANT(kMIBTargetBacklight0I2CAddress,	kOIDTypeUInt32, 0xC6);
MIB_CONSTANT(kMIBTargetOsPictureScale,	kOIDTypeUInt32, 2);
MIB_CONSTANT(kMIBTargetPictureRotate,	kOIDTypeInt32,  0);

static uint32_t iphone7_get_board_rev(void)
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

static board_type iphone7_get_board_type(void)
{
	static board_type	board_type = BOARD_TYPE_UNKNOWN;

	if (board_type != BOARD_TYPE_UNKNOWN) {
		return board_type;
	}

	uint32_t board_id = platform_get_board_id();
	uint32_t board_rev = iphone7_get_board_rev();

	switch (board_id) {
	case N59_AP_BOARD_ID:
		switch (board_rev) {
		case N59_PROTOMLB1:
			return BOARD_TYPE_N59_PROTOMLB1;
			
		default:
			goto fail;
		}
	case N59_DEV_BOARD_ID:
		switch (board_rev) {
		case N59_DEV_BOARD_REV:
			return BOARD_TYPE_N59_DEV1;
			
		default:
			goto fail;
		}
		
	default:
		goto fail;
	}

fail:
	platform_not_supported();
	return BOARD_TYPE_UNKNOWN;
}

int target_get_boot_battery_capacity(void) {
	return 50;
}

void target_early_init(void)
{
#if PRODUCT_LLB || PRODUCT_IBSS
	uint32_t target_mipi_p, target_mipi_m, target_mipi_s;

	// Temporary workaround to set MIPI P/M/S values for display
    	// D410 (@ 513 MHz) until rdar://problem/17985178 is fixed.
	target_mipi_p = 4;
	target_mipi_m = 171;
	target_mipi_s = 1;
	
	clock_set_frequency(CLK_MIPI, 0, target_mipi_p, target_mipi_m, target_mipi_s, 0);
#endif
	
#if PRODUCT_LLB || PRODUCT_IBSS
	dprintf(DEBUG_INFO, "%s: chip rev: %d, fuse: %d\n", __func__,
		chipid_get_chip_revision(), chipid_get_fuse_revision());

#if WITH_HW_POWER
	int pmu_get_data(int dev, uint16_t reg, uint8_t *byte);

	int rc;
	uint8_t val;

	if ((rc = pmu_get_data(0, 0x1, &val)) < 0) { // read PMU.OTP version
		dprintf(DEBUG_CRITICAL, "PMU: cannot read OTP Version (%d)\n", rc);
	} else {
		dprintf(DEBUG_INFO, "%s: PMU OTP ver: %x\n", __func__, val);
	}
#endif
#endif
}

void target_late_init(void)
{
	clock_gate(CLK_UART7, 0);
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
	display_config = 0x00000044;
	return ((void *)(&display_config));
}

#if WITH_ENV

void target_setup_default_environment(void)
{
	// boot-device is set in platform's init.c
	env_set("display-color-space","RGB888", 0);
	env_set("display-timing", "D410", 0);
	env_set("adbe-tunables", "D410", 0);
	env_set("adfe-tunables", "D410", 0);
}

#endif

#if WITH_HW_CHESTNUT
uint8_t target_get_lcm_ldos(void)
{
	return (DISPLAY_PMU_LDO(0) | DISPLAY_PMU_LDO(1));
}
#endif

#if WITH_HW_LM3534
uint8_t target_lm3534_gpr(uint32_t ctlr)
{
	// DISPLAY_PANEL_TYPE_D410:
	// (ovp_select_21v) | (ramp disable) | (chip enable)
	return ((1 << 4) | (1 << 1) | (1 << 0));
}
#endif

#if WITH_DEVICETREE

int target_update_device_tree(void)
{
#if WITH_HW_DISPLAY_PINOT
	DTNode		*disp0_node, *backlight_node;
#endif
	DTNode		*node;
	uint32_t	propSize;
	char		*propName;
	void		*propData;

#if WITH_HW_DISPLAY_PINOT
	// Find the DISP0 (display-subsystem 0) node
	FindNode(0, "arm-io/disp0", &disp0_node);

	if (FindNode(0, "arm-io/mipi-dsim/lcd", &node)) {
		extern int pinot_update_device_tree(DTNode *pinot_node, DTNode *clcd_node, DTNode *backlight_node);

		FindNode(0, "backlight", &backlight_node);
		pinot_update_device_tree(node, disp0_node, backlight_node);
	}
#endif

#if WITH_HW_MIPI_DSIM
	// Find the mipi node
	if (FindNode(0, "arm-io/mipi-dsim", &node)) {
		extern int mipi_update_device_tree(DTNode *mipi_node);
		mipi_update_device_tree(node);
	}
#endif

	// Update the speaker calibration data
	if (FindNode(0, "arm-io/i2c1/audio-speaker", &node)) {
		propName = "speaker-rdc";
		if (FindProperty(node, &propName, &propData, &propSize)) {
			syscfgCopyDataForTag('SRdc', propData, propSize);
		}
		propName = "speaker-calib";
		if (FindProperty(node, &propName, &propData, &propSize)) {
			syscfgCopyDataForTag('SpCl', propData, propSize);
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

	// Update the codec and speaker nodes with acoustic transducer scale data
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
				propName = "at-scale-fmic";
				if (FindProperty(node, &propName, &propData, &propSize)) {
				    memcpy(propData, &atscData[8], sizeof(uint32_t));
				}
				propName = "at-scale-rcvr";
				if (FindProperty(node, &propName, &propData, &propSize)) {
				    memcpy(propData, &atscData[12], sizeof(uint32_t));
				}
			}
			// Update speaker
			if (FindNode(0, "arm-io/i2c1/audio-speaker", &node)) {
				propName = "at-scale";
				if (FindProperty(node, &propName, &propData, &propSize)) {
				    memcpy(propData, &atscData[16], sizeof(uint32_t));
				}
			}
		}
	}

	// Update the als calibration data
	if (FindNode(0, "arm-io/i2c2/als", &node)) {
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
		propName = "compass-orientation";
		if (FindProperty(node, &propName, &propData, &propSize)) {
			syscfgCopyDataForTag('CRot', propData, propSize);
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

	// Update prox calibration data
	if (FindNode(0, "arm-io/spi2/multi-touch", &node)) {
		propName = "prox-calibration";
		if (FindProperty(node, &propName, &propData, &propSize))
			syscfgCopyDataForTag('PxCl', propData, propSize);
	}
	
	// Stockholm calibration
	if (FindNode(0, "arm-io/uart3/stockholm", &node)) {
		propName = "calibration";
		if (FindProperty(node, &propName, &propData, &propSize))
			syscfgCopyDataForTag('NFCl', propData, propSize);
	}

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

	// Update cover glass type
	if (FindNode(0, "product", &node)) {
		propName = "cover-glass";
		if (FindProperty(node, &propName, &propData, &propSize)) {
			syscfgCopyDataForTag('CGSp', propData, propSize);
		}
	}

	// Update charger calibration data
	if (FindNode(0, "charger", &node)) {
		propName = "usb-input-limit-calibration";
		if (FindProperty(node, &propName, &propData, &propSize))
			syscfgCopyDataForTag('CBAT', propData, propSize);
	}

	// Update Vibe calibration data
	if (FindNode(0, "arm-io/i2c1/vib-pwm", &node)) {
		propName = "calibration-data";
		if (FindProperty(node, &propName, &propData, &propSize)) {
			const uint32_t count=syscfgCopyDataForTag('VbCl', propData, propSize);
			if ( count!=propSize ) *((uint32_t*)propData)=0x00a46d0d;
		}
	}

#if WITH_HW_DISPLAY_PMU
	display_pmu_update_device_tree("arm-io/i2c0/display-pmu");
#endif

	return 0;
}

#endif // WITH_DEVICETREE

#if WITH_PAINT

// The default background is expected to to be black and the artwork is expected
// to be white. This arrangement will be inverted depending upon the cover glass
// color of the device.

// Sample DClr_override values for testing devices without DClr syscfg entries (enclosure/cover glass):
// gray/black:		setenv DClr_override 00020000B9B5B4003C3B3B0000000000
// gold/white:		setenv DClr_override 00020000B5CCE100E3E4E10000000000
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
