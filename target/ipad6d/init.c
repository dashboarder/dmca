/*
 * Copyright (C) 2015 Apple Inc. All rights reserved.
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
#include <target/boardid.h>
#include <platform/soc/hwclocks.h>
#if WITH_HW_DISPLAY_PMU
#include <drivers/display_pmu.h>
#endif
#if (APPLICATION_IBOOT && (PRODUCT_IBOOT || PRODUCT_IBEC))
#include <drivers/displayport/displayport.h>
#include <drivers/displayport.h>
#endif
#if TARGET_USES_BLEND_CM
#include <target/display_gamma_tables.h>
#endif

MIB_CONSTANT(kMIBTargetOsPictureScale,	kOIDTypeUInt32, 2);
MIB_CONSTANT(kMIBTargetPictureRotate,	kOIDTypeInt32,  -90);

#if (APPLICATION_IBOOT && (PRODUCT_IBOOT || PRODUCT_IBEC))
dp_t dp;
#endif

typedef enum {
    DISPLAY_PANEL_TYPE_TULIP = 0,
    DISPLAY_PANEL_TYPE_COUNT
} ipad6d_display_t;

#if WITH_ENV
static const char *display_panel_name[] = {
    [DISPLAY_PANEL_TYPE_TULIP] = "ipad6d",
};
#endif

// AP settings for:
//
// IO GPIO 298: AOP_I2CM_SCL (AOP GPIO 10  ) -> SPI_ACCEL_CS_L
// IO GPIO 299: AOP_FUNC[0]  (AOP GPIO 11) -> SPI_MAGNESIUM_CS_L
// IO GPIO 303: AOP_FUNC[4]  (AOP GPIO 15) -> SPI_CARBON_CS_L
// IO GPIO 305: AOP_FUNC[6]  (AOP GPIO 17) -> SPI_PHOSPHORUS_CS_L
static const uint32_t fixup_list_ap[] = {
    GPIOC(GPIOC_1, (10 / 8), (10 % 8)),
    (FUNC_ALT0 | PULL_UP | DRIVE_S7 | SLOW_SLEW),

    GPIOC(GPIOC_1, (11 / 8), (11 % 8)),
    (FUNC_ALT0 | PULL_UP | DRIVE_S7 | SLOW_SLEW),

    GPIOC(GPIOC_1, (15 / 8), (15 % 8)),
    (FUNC_ALT0 | PULL_UP | DRIVE_S7 | SLOW_SLEW),

    GPIOC(GPIOC_1, (17 / 8), (17 % 8)),
    (FUNC_ALT0 | PULL_UP | DRIVE_S7 | SLOW_SLEW),

    UINT32_MAX, UINT32_MAX
};

static uint32_t	gpio_board_rev;
static bool gpio_board_rev_valid = false;

static uint32_t ipad6d_get_board_rev(void);
static void ipad6d_assert_display_type(void);

#if WITH_HW_POWER
    int pmu_set_data(int dev, uint16_t reg, uint8_t byte, bool do_confirm);
    int pmu_get_data(int dev, uint16_t reg, uint8_t *byte);
#endif

static uint32_t
ipad6d_get_board_rev (void)
{
    /*
     * Sequence:
     * - Set GPIO as input.
     * - Enable PU and disable PD.
     * - Read
     */
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

    return (gpio_board_rev);
}

static uint32_t
ipad6d_get_display_type (void)
{
    return (DISPLAY_PANEL_TYPE_TULIP);
}

int
target_get_boot_battery_capacity (void)
{
    int temp = 0;
#if WITH_HW_GASGAUGE
    if (gasgauge_read_temperature(&temp) != 0) {
        temp = 0; // defensive
    }
#endif

    if (temp > 500) {
        return (0);  // rely on SOC1 clear only
    }

    return (50);  // @see rdar://16587240
}

uint32_t
target_get_display_panel_type (void)
{
    return (ipad6d_get_display_type());
}

uint32_t
target_get_board_rev (void)
{
    if (!gpio_board_rev_valid) {
        ipad6d_get_board_rev();
    }

    return (gpio_board_rev);
}

void
target_early_init (void)
{
    uint32_t board_rev = target_get_board_rev();
    uint32_t board_id = platform_get_board_id();

    dprintf(DEBUG_INFO, "%s: board id: %x, board rev: %x\n",
            __func__, board_id, board_rev);
}

void
target_late_init (void)
{
    if (target_config_ap() && target_get_board_rev() < J99a_AP_DVT_BOARD_REV_1) {
        dprintf(DEBUG_INFO, "PRE-DVT MLBs are deprecated\n");
        platform_not_supported();
    }
    else if (target_config_dev() && target_get_board_rev() < J99a_DEV2_BOARD_REV) {
        dprintf(DEBUG_INFO, "PRE-DEV2 boards are deprecated\n");
        platform_not_supported();
    }
    power_set_gpio(GPIO_PMU_NAND_LOW_POWER_MODE, 1, 1);
}

void
target_init (void)
{
#if WITH_HW_FLASH_NOR
    flash_nor_init(SPI_NOR0);
#endif

    // apply AOP fixups after the above are done
    gpio_fixup_pinconfig(fixup_list_ap);
}

void
target_quiesce_hardware (void)
{
}

void
target_poweroff (void)
{
}

int
target_bootprep (enum boot_target target)
{
    return 0;
}

bool
target_should_recover (void)
{
    return (platform_get_request_dfu2() && power_has_usb());
}

bool
target_should_poweron (bool *cold_button_boot)
{
#if WITH_HW_POWER
    if (power_get_boot_flag() == kPowerBootFlagColdButton) {
        *cold_button_boot = true;
    }
#else
    *cold_button_boot = false;
#endif // WITH_HW_POWER

    return (!*cold_button_boot || platform_get_request_dfu1());
}

bool
target_should_poweroff (bool at_boot)
{
    return (platform_get_request_dfu1() && (!at_boot || !power_has_usb()));
}

void * target_get_display_configuration(void)
{
#if (APPLICATION_IBOOT && (PRODUCT_IBOOT || PRODUCT_IBEC))
	dp.mode	=		kDPControllerMode_Slave;
	dp.type	=		kDPControllerType_EDP;
	dp.min_link_rate =	kLinkRate324Gbps;
	dp.max_link_rate =	kLinkRate324Gbps;
	dp.lanes =		0x4;
	dp.ssc =		0x0;
	dp.alpm =		0x1;
	dp.vrr_enable =		0x1;
	dp.vrr_on =		0x1;
	dp.rx_n1=		25;
	dp.rx_n2=		2;
	dp.rx_n3=		16030;
	dp.rx_n5=		0x2;
	dp.fast_link_training =	true;
	return ((void *)(&dp));
#else
	return NULL;
#endif
}

#if WITH_ENV

void
target_setup_default_environment (void)
{
    uint32_t display_id = ipad6d_get_display_type();

    // boot-device is set in platform's init.c
    env_set("boot-partition", "0", 0);

    env_set("boot-path",
           "/System/Library/Caches/com.apple.kernelcaches/kernelcache", 0);

    env_set("display-color-space","RGB888", 0);

    env_set("display-timing", display_panel_name[display_id], 0);
    env_set("adbe-tunables", display_panel_name[display_id], 0);
    env_set("adfe-tunables", display_panel_name[display_id], 0);
}

#endif

#if WITH_HW_LM3534
uint8_t
target_lm3534_gpr (uint32_t ctlr)
{
    uint8_t gpr = (1 << 1) | (1 << 0);	// (ramp disable) | (chip enable)

#if TARGET_DISPLAY_D520
    gpr |= (1 << 4);		// (ovp_select_21v)
#elif TARGET_DISPLAY_D620
    if (ctlr == 1)
        gpr |= (1 << 4);	// (ovp_select_21v)
#else
#error "unknown display type"
#endif

    return gpr;
}
#endif

#if WITH_DEVICETREE

void 
hid_update_device_tree(void)
{
	DTNode 		*node;
	char 		*propName;
	uint32_t 	propSize;
	void 		*propData;

	if (dt_find_node(0, "arm-io/spi3/multi-touch", &node)) {
		propName = "multitouch-to-display-offset";
		if (dt_get_prop(node, &propName, &propData, &propSize)) {
			syscfgCopyDataForTag('MtDO', propData, propSize);
		}

		propName = "firefly-calibration";
		if (dt_get_prop(node, &propName, &propData, &propSize)) {
			syscfgCopyDataForTag('FfCl', propData, propSize);
		}
	}
}

int
target_update_device_tree (void)
{
	uint32_t        propSize;
	char            *propName;
	void            *propData;
	DTNode		*node;
	
#if WITH_HW_DISPLAY_EDP 
	DTNode		*clcd_node = NULL, *backlight_node = NULL, *lcd_node = NULL;
	DTNode		*lpdp_node = NULL;
#endif

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
#if TARGET_USES_BLEND_CM
	if (dt_find_node(0, "arm-io/disp0", &node)) {
		dt_set_prop(node, "cm-post-blend-degamma", target_blend_degamma_tables, sizeof(target_blend_degamma_tables));
		dt_set_prop(node, "cm-post-blend-engamma", target_blend_engamma_tables, sizeof(target_blend_engamma_tables));
	}
#endif

	// Update the als calibration data for all nodes that may be present on the dev board
	if (dt_find_node(0, "arm-io/i2c3/als1", &node)) {
		propName = "alsCalibration";
		if (dt_get_prop(node, &propName, &propData, &propSize)) {
			syscfgCopyDataForTag('LSCI', propData, propSize);
		}
	}

	if (dt_find_node(0, "arm-io/i2c3/als2", &node)) {
		propName = "alsCalibration";
		if (dt_get_prop(node, &propName, &propData, &propSize)) {
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
    if (dt_find_node(0, "arm-io/aop/iop-aop-nub/compass", &node)) {
        propName = "compass-calibration";
        if (dt_get_prop(node, &propName, &propData, &propSize)) {
            syscfgCopyDataForTag('CPAS', propData, propSize);
        }
        propName = "compass-orientation";
        if (dt_get_prop(node, &propName, &propData, &propSize)) {
            syscfgCopyDataForTag('CRot', propData, propSize);
        }
    }
    
    // Update the gyro calibration data
    if (dt_find_node(0, "arm-io/aop/iop-aop-nub/gyro", &node)) {
        propName = "gyro-orientation";
        if (dt_get_prop(node, &propName, &propData, &propSize)) {
            syscfgCopyDataForTag('GRot', propData, propSize);
        }
        propName = "gyro-sensitivity-calibration";
        if (dt_get_prop(node, &propName, &propData, &propSize)) {
            syscfgCopyDataForTag('GSCl', propData, propSize);
        }
        propName = "gyro-temp-table";
        if (dt_get_prop(node, &propName, &propData, &propSize)) {
            syscfgCopyDataForTag('GYTT', propData, propSize);
        }
        propName = "gyro-interrupt-calibration";
        if (dt_get_prop(node, &propName, &propData, &propSize)) {
            syscfgCopyDataForTag('GICl', propData, propSize);
        }
    }
    
    // Update accelerometer calibration data
    if (dt_find_node(0, "arm-io/aop/iop-aop-nub/accel", &node)) {
        propName = "accel-orientation";
        if (dt_get_prop(node, &propName, &propData, &propSize)) {
            syscfgCopyDataForTag('ARot', propData, propSize);
        }
        propName = "low-temp-accel-offset";
        if (dt_get_prop(node, &propName, &propData, &propSize)) {
            syscfgCopyDataForTag('LTAO', propData, propSize);
        }
        propName = "accel-sensitivity-calibration";
        if (dt_get_prop(node, &propName, &propData, &propSize)) {
            syscfgCopyDataForTag('ASCl', propData, propSize);
        }
        propName = "accel-interrupt-calibration";
        if (dt_get_prop(node, &propName, &propData, &propSize)) {
            syscfgCopyDataForTag('AICl', propData, propSize);
        }
    }
    
    // Update pressure sensor calibration data
    if (dt_find_node(0, "arm-io/aop/iop-aop-nub/pressure", &node)) {
        propName = "pressure-offset-calibration";
        if (dt_get_prop(node, &propName, &propData, &propSize)) {
            syscfgCopyDataForTag('SPPO', propData, propSize);
        }
    }

    

    if (FindNode(0, "arm-io/i2c1/audio-speaker-cn-left", &node)) {
      propName = "speaker-calib";
      if (FindProperty(node, &propName, &propData, &propSize)) {
          syscfgCopyDataForTag('SpCl', propData, propSize);
      }
    }

    hid_update_device_tree();
 
    return (0);
}

#endif // WITH_DEVICETREE

#if WITH_PAINT

// The default background is expected to to be black and the artwork is expected
// to be white. This arrangement will be inverted depending upon the cover glass
// color of the device.

// Sample DClr_override values for testing devices without DClr syscfg entries
// (enclosure/cover glass):
// gray/black:		setenv DClr_override 000200009B9899003C3B3B0000000000
// silver/white:	setenv DClr_override 00020000D8D9D700E3E4E10000000000

static color_policy_invert_t target_cover_glass_color_table[] = {
    { RGB( 59,  59,  60), false }, // Black - black background, white logo
    { RGB(225, 228, 227), true  }, // White - white background, black logo
};

color_policy_t *
target_color_map_init (enum colorspace cs, color_policy_t *color_policy)
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

bool
power_needs_precharge (void)
{
	return false;
}

void
power_cancel_buttonwait (void)
{
}

bool
power_do_chargetrap (void)
{
	return false;
}

bool
power_is_suspended (void)
{
	return false;
}

void
power_will_resume (void)
{
}

bool
power_has_usb (void)
{
	return false;
}

int
power_read_dock_id (unsigned *id)
{
	return -1;
}

bool
power_get_diags_dock (void)
{
	return false;
}

uint32_t
power_get_boot_battery_level (void)
{
	return 0;
}

int
power_get_nvram (uint8_t key, uint8_t *data)
{
	return -1;
}

int
power_set_nvram (uint8_t key, uint8_t data)
{
	return -1;
}

int
power_set_soc_voltage (unsigned mv, int override)
{
	return -1;
}

bool force_usb_power = false;

void
power_set_usb_state (bool configured, bool suspended)
{
}

int
power_backlight_enable (uint32_t backlight_level)
{
	return 0;
}

#endif /* ! WITH_HW_POWER */
