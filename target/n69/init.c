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

#define BOARD_REV_AP_PROTO2	0x2

static uint32_t n69_get_board_rev(void);

#if WITH_HW_LM3534
MIB_CONSTANT(kMIBTargetBacklight0I2CBus,	kOIDTypeUInt32, 0);
MIB_CONSTANT(kMIBTargetBacklight0I2CAddress,	kOIDTypeUInt32, 0xC4);
#endif

MIB_CONSTANT(kMIBTargetOsPictureScale,	kOIDTypeUInt32, 2);
MIB_CONSTANT(kMIBTargetPictureRotate,	kOIDTypeInt32,  0);

static bool	gpio_board_rev_valid;
static uint32_t	gpio_board_rev;

#if (APPLICATION_IBOOT && (PRODUCT_IBOOT || PRODUCT_IBEC))
#include <drivers/mipi/mipi.h>
static mipi_t display_panel_configurations[] = {
	{
		.lanes = 2,
		.esc_div = 6,
		.pll_n = 0x3,
		.pll_m = 0x115,
		.pll_p = 0x1,
		.hsfreq = 0x29,
		.target_phy_settings = {4, {{0x44, 1, {0x52}}, {0x30, 1, {0x3F}}, {0x20, 1, {0x45}}, {0x32, 1, {0x28}}}},	//843 Mhz
	},
};
#endif

// Read the board rev GPIOs. Note that the encoding is done
// with active-low GPIOs, so we need to invert the values
// we read from the GPIOs
static uint32_t n69_get_board_rev(void)
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

		// Invert 
		gpio_board_rev ^= 0xf;

		gpio_configure(GPIO_BOARD_REV0, GPIO_CFG_DFLT);
		gpio_configure(GPIO_BOARD_REV1, GPIO_CFG_DFLT);
		gpio_configure(GPIO_BOARD_REV2, GPIO_CFG_DFLT);
		gpio_configure(GPIO_BOARD_REV3, GPIO_CFG_DFLT);

		gpio_board_rev_valid = true;
	}

	return gpio_board_rev;
}

int target_get_boot_battery_capacity(void)
{
	int temp=0;
#if WITH_HW_GASGAUGE
	if ( gasgauge_read_temperature(&temp) != 0 ) temp=0; // defensive
#endif

	const uint32_t charge_current=power_get_available_charge_current();
	if ( temp>500 && charge_current>500 ) return 0;   // rely on SOC1 clear only

	return 50;  // @see rdar://16587240
}

void target_early_init(void)
{	

}

void target_late_init(void)
{
	if (target_config_ap() && n69_get_board_rev() < BOARD_REV_AP_PROTO2) {
		dprintf(DEBUG_INFO, "Proto1 is deprecated\n");
		platform_not_supported();
	}

	power_set_gpio(GPIO_PMU_NAND_LOW_POWER_MODE, 1, 1);
}

void target_init(void)
{
	uint32_t board_rev = n69_get_board_rev();
#if WITH_HW_FLASH_NOR
	flash_nor_init(SPI_NOR0);
#endif

	dprintf(DEBUG_INFO, "board rev 0x%x (raw 0x%x)\n", board_rev, board_rev ^ 0xf);
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

void * target_get_display_configuration(void)
{
#if (APPLICATION_IBOOT && (PRODUCT_IBOOT || PRODUCT_IBEC))
	return ((void *)(&display_panel_configurations[0]));
#else
	return NULL;
#endif
}

#if WITH_ENV

void target_setup_default_environment(void)
{
	// boot-device is set in platform's init.c
	env_set("display-color-space","RGB888", 0);
	env_set("display-timing", "D403", 0);
	env_set("adbe-tunables", "D403", 0);
	env_set("adfe-tunables", "D403", 0);
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
	return ((1 << 1) | (1 << 0) | (1 << 4)); // (ramp disable) | (chip enable) | (auto mode);
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

	if (FindNode(0, "arm-io/i2c1/audio-actuator", &node)) {
		propName = "actuator-calibration";
		if (FindProperty(node, &propName, &propData, &propSize)) {
			syscfgCopyDataForTag('TCal', propData, propSize);
		}
	}

	// Update the codec with acoustic transducer scale data
	if (FindNode(0, "arm-io/spi1/audio-codec", &node)) {
		propName = "acoustic-trim-gains";
		if (FindProperty(node, &propName, &propData, &propSize)) {
			syscfgCopyDataForTag('ATGa', propData, propSize);
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
	if (FindNode(0, "arm-io/aop/iop-aop-nub/compass", &node)) {
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
	if (FindNode(0, "arm-io/aop/iop-aop-nub/gyro", &node)) {
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
	if (FindNode(0, "arm-io/aop/iop-aop-nub/accel", &node)) {
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
	if (FindNode(0, "arm-io/aop/iop-aop-nub/pressure", &node)) {
		propName = "pressure-offset-calibration";
		if (FindProperty(node, &propName, &propData, &propSize)) {
			syscfgCopyDataForTag('SPPO', propData, propSize);
		}
	}

	// Update prox calibration data
	if (FindNode(0, "arm-io/spi2/multi-touch", &node)) {
		propName = "prox-calibration";
		if (FindProperty(node, &propName, &propData, &propSize))
			syscfgCopyDataForTag('PxCl', propData, propSize);
		propName = "orb-gap-cal";
		if (FindProperty(node, &propName, &propData, &propSize))
			syscfgCopyDataForTag('OrbG', propData, propSize);
		propName = "orb-accel-cal";
		if (FindProperty(node, &propName, &propData, &propSize))
			syscfgCopyDataForTag('OICo', propData, propSize);
		propName = "orb-force-cal";
		if (FindProperty(node, &propName, &propData, &propSize))
			syscfgCopyDataForTag('OFCl', propData, propSize);
		propName = "orb-dynamic-accel-cal";
		if (FindProperty(node, &propName, &propData, &propSize))
			syscfgCopyDataForTag('FDAC', propData, propSize);
	}
	
	// Stockholm calibration
	if (FindNode(0, "arm-io/uart2/stockholm", &node)) {
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

	// vibrator calibration
	if (FindNode(0, "arm-io/pwm/vibrator", &node)) {
		propName = "duty-cycle";
		if (FindProperty(node, &propName, &propData, &propSize)) {
			uint32_t pct;
			if (syscfgCopyDataForTag('VLSg', (void *)&pct, sizeof(pct)) > 0) {
				((uint32_t *)propData)[0] = ((uint64_t)pct << 32) / 100;
			}
		}
		propName = "intensity-config";
		if (FindProperty(node, &propName, &propData, &propSize)) {
			uint32_t pct;
			if (syscfgCopyDataForTag('VLWk', (void *)&pct, sizeof(pct)) > 0) {
				((uint32_t *)propData)[1] = ((uint64_t)pct << 32) / 100;
			}
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
	{ RGB( 18,  18,  17), false },	// Black - black background, white logo
	{ RGB(200, 202, 202), true  },	// White - white background, black logo
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

#if !WITH_HW_POWER

bool power_needs_precharge(void)
{
	return false;
}

void power_cancel_buttonwait(void)
{
}

bool power_do_chargetrap(void)
{
	return false;
}

bool power_is_suspended(void)
{
	return false;
}

void power_will_resume(void)
{
}

bool power_has_usb(void)
{
	return false;
}

int power_read_dock_id(unsigned *id)
{
	return -1;
}

bool power_get_diags_dock(void)
{
	return false;
}

uint32_t power_get_boot_battery_level(void)
{
	return 0;
}

int power_get_nvram(uint8_t key, uint8_t *data)
{
	return -1;
}

int power_set_nvram(uint8_t key, uint8_t data)
{
	return -1;
}

int power_set_soc_voltage(unsigned mv, int override)
{
	return -1;
}

bool force_usb_power = false;

void power_set_usb_state(bool configured, bool suspended)
{
}

int power_backlight_enable(uint32_t backlight_level)
{
	return 0;
}

#endif /* ! WITH_HW_POWER */
