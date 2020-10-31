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
#include <platform/soc/pmgr.h>
#if (APPLICATION_IBOOT && (PRODUCT_IBOOT || PRODUCT_IBEC))
#include <drivers/displayport/displayport.h>
#include <drivers/displayport.h>
#endif
#if TARGET_USES_BLEND_CM
#include <target/display_gamma_tables.h>
#endif

MIB_CONSTANT(kMIBTargetOsPictureScale,	kOIDTypeUInt32, 2);
MIB_CONSTANT(kMIBTargetPictureRotate,	kOIDTypeInt32,  0);

#if (APPLICATION_IBOOT && (PRODUCT_IBOOT || PRODUCT_IBEC))
static dp_t dp;
#endif

// DEV settings for:
//
// IO GPIO 298: AOP_I2CM_SCL (AOP GPIO 10) -> SPI_ACCEL_CS_L
// IO GPIO 299: AOP_FUNC[0]  (AOP GPIO 11) -> SPI_MAGNESIUM_CS_L
// IO GPIO 303: AOP_FUNC[4]  (AOP GPIO 15) -> SPI_CARBON_CS_L
// IO GPIO 305: AOP_FUNC[6]  (AOP GPIO 17) -> SPI_PHOSPHORUS_CS_L
static const uint32_t fixup_list_dev[] = {
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

// AP settings for:
//
// IO GPIO 298: AOP_I2CM_SCL (AOP GPIO 10) -> SPI_ACCEL_CS_L
// IO GPIO 299: AOP_FUNC[0]  (AOP GPIO 11) -> SPI_MAGNESIUM_CS_L
// IO GPIO 303: AOP_FUNC[4]  (AOP GPIO 15) -> SPI_CARBON_CS_L
// IO GPIO 305: AOP_FUNC[6]  (AOP GPIO 17) -> SPI_PHOSPHORUS_CS_L
static const uint32_t fixup_list_ap[] = {
    GPIOC(GPIOC_1, (10 / 8), (10 % 8)),
    (FUNC_ALT0 | PULL_UP | DRIVE_S4 | SLOW_SLEW),

    GPIOC(GPIOC_1, (11 / 8), (11 % 8)),
    (FUNC_ALT0 | PULL_UP | DRIVE_S4 | SLOW_SLEW),

    GPIOC(GPIOC_1, (15 / 8), (15 % 8)),
    (FUNC_ALT0 | PULL_UP | DRIVE_S4 | SLOW_SLEW),

    GPIOC(GPIOC_1, (17 / 8), (17 % 8)),
    (FUNC_ALT0 | PULL_UP | DRIVE_S4 | SLOW_SLEW),

    UINT32_MAX, UINT32_MAX
};

static uint32_t	gpio_board_rev;
static bool gpio_board_rev_valid;

#if WITH_HW_POWER
    int pmu_set_data(int dev, uint16_t reg, uint8_t byte, bool do_confirm);
    int pmu_get_data(int dev, uint16_t reg, uint8_t *byte);
#endif

uint32_t
ipad6b_get_board_rev (void)
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
target_get_board_rev (void)
{
    if (!gpio_board_rev_valid) {
        ipad6b_get_board_rev();
    }

    return (gpio_board_rev);
}

static
void target_fixup_power (uint32_t board_rev)
{
#if WITH_HW_POWER
int charger_set_data(int dev, uint16_t reg, uint8_t byte, bool do_confirm);

    int rc;

    // <rdar://problem/22952749> Update PMU UV comparator settings for J127 units prior to EVT
    // NOTE: kD2257_IPK_UV_BUCK1_UV_EN_THR, kD2257_IPK_UV_BUCK1_UV_DIG_CONF_1 (0xf8c,0x0f8a) set in pmu_uvwarn_config()
    uint8_t  data=0xff;
    pmu_get_data(0, 0x0201, &data);
    if ( data < 0x09 ) { // 0x09 = BJ
        rc = pmu_set_data(0, 0x7000, 0x1d, false);
        if ( rc<0 ) {
            dprintf(DEBUG_CRITICAL, "pmu: cannot enter test mode (%d)\n", rc);
        } else {
            rc = pmu_set_data(0, 0x0f8e, 0x0f, true); // was 0xa
            if ( rc==0 ) rc = pmu_set_data(0, 0x0f87, 0x00, true); // required
            if ( rc==0 ) rc = pmu_set_data(0, 0x0f8d, 0x0f, true); // ok
            if ( rc==0 ) rc = pmu_set_data(0, 0x1108, 0xff, true);

            if ( rc!=0 ) dprintf(DEBUG_INFO, "pmu: failed UV_WARN reconfig(%d)\n", rc);

            int temp=pmu_set_data(0, 0x700, 0x1d, false);
            if ( temp<0 ) panic("pmu: cannot exit test mode\n");
        }
    }

    rc=charger_set_data(0, 0x04d7, 0x04, true);
    if ( rc<0 ) dprintf(DEBUG_CRITICAL, "chg: cannot update ATV table (%d)\n", rc);
#endif
}

void
target_early_init (void)
{
    uint32_t board_rev = target_get_board_rev();
    uint32_t board_id = platform_get_board_id();

    dprintf(DEBUG_INFO, "%s: board id: 0x%02x, board rev: 0x%02x\n",
            __func__, board_id, board_rev);

#if PRODUCT_LLB || PRODUCT_IBSS
    // Refer rdar://18848645
    target_fixup_power(board_rev);
    pmgr_platform_config_uvwarn();
#endif
}

void
target_late_init (void)
{
    power_set_gpio(GPIO_PMU_NAND_LOW_POWER_MODE, 1, 1);
}

void
target_init (void)
{
#if WITH_HW_FLASH_NOR
    flash_nor_init(SPI_NOR0);
#endif

    if (target_config_dev()) {
        gpio_fixup_pinconfig(fixup_list_dev);
    } else {
        gpio_fixup_pinconfig(fixup_list_ap);
    }        
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
	dp.min_link_rate =	kLinkRate270Gbps;
	dp.max_link_rate =	kLinkRate270Gbps;
	dp.lanes =		0x4;
	dp.ssc =		0x0;
	dp.alpm =		0x1;
	dp.vrr_enable =		0x1; 
	dp.vrr_on =		0x1;
	dp.rx_n1=		25;
	dp.rx_n2=		2;
	dp.rx_n3=		13380;
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
    // boot-device is set in platform's init.c
    env_set("display-color-space","ARGB8101010", 0);

    env_set("display-timing", "ipad6", 0);
    env_set("adbe-tunables", "ipad6b", 0);
    env_set("adfe-tunables", "ipad6b", 0);
}

#endif

#if WITH_DEVICETREE

int
target_update_device_tree (void)
{
#if WITH_HW_DISPLAY_EDP 
    DTNode *clcd_node = NULL, *backlight_node = NULL, *lcd_node = NULL;
    DTNode *lpdp_node = NULL;
#endif

    DTNode *node;
    void *propData;
    uint32_t propSize;
    char *propName;

#if WITH_HW_DISPLAY_EDP
    if (FindNode(0, DP_DTPATH, &node)) {
        extern int edp_update_device_tree(DTNode *edp_node,
                                          DTNode *lcd_node,
                                          DTNode *clcd_node,
                                          DTNode *backlight_node);
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
    if (dt_find_node(0, "arm-io/i2c3/als-1-fh", &node)) {
        propName = "alsCalibration";
        if (dt_get_prop(node, &propName, &propData, &propSize)) {
            //First, copy the CSCI key is present
            if(syscfgCopyDataForTag('CSCI', propData, propSize) == -1) {
                //Otherwise try to copy the CTMP key
                syscfgCopyDataForTag('CTMP', propData, propSize);
            }
        }
    }

    if (dt_find_node(0, "arm-io/i2c3/als-2-cn", &node)) {
        propName = "alsCalibration";
        if (dt_get_prop(node, &propName, &propData, &propSize)) {
            //First, copy the CSCI key is present
            if(syscfgCopyDataForTag('CSCI', propData, propSize) == -1) {
                //Otherwise try to copy the CTMP key
                syscfgCopyDataForTag('CTMP', propData, propSize);
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

    if (dt_find_node(0, "arm-io/spi3/multi-touch", &node)) {
        propName = "multitouch-to-display-offset";
        if (dt_get_prop(node, &propName, &propData, &propSize)) {
            syscfgCopyDataForTag('MtDO', propData, propSize);
        }

        propName = "firefly-calibration";
        if (dt_get_prop(node, &propName, &propData, &propSize)) {
            syscfgCopyDataForTag('FfCl', propData, propSize);
        }

        //
        // rdar://problem/20508636 - Orb Cal Sys Cfg Keys
        //
        propName = "orb-afe-cal";
        if (dt_get_prop(node, &propName, &propData, &propSize)) {
            syscfgCopyDataForTag('OrbF', propData, propSize);
        }

        propName = "orb-sum-cal";
        if (dt_get_prop(node, &propName, &propData, &propSize)) {
            syscfgCopyDataForTag('OrbS', propData, propSize);
        }

        propName = "orb-dhybrid-cal";
        if (dt_get_prop(node, &propName, &propData, &propSize)) {
            syscfgCopyDataForTag('OrbD', propData, propSize);
        }

        propName = "orb-plmf-cal";
        if (dt_get_prop(node, &propName, &propData, &propSize)) {
            syscfgCopyDataForTag('OrbP', propData, propSize);
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
    
    //
    // <rdar://problem/21492027> J127: Unexpected speaker calibration values
    //
    if (FindNode(0, "arm-io/i2c1/audio-speaker-cn-left", &node)) {
        propName = "speaker-calib";

        if (FindProperty(node, &propName, &propData, &propSize)) {
            syscfgCopyDataForTag('SpCl', propData, propSize);
        }
    }

    return (0);
}

#endif // WITH_DEVICETREE

#if WITH_PAINT

// The default background is expected to to be black and the artwork is expected
// to be white. This arrangement will be inverted depending upon the cover glass
// color of the device.

// Sample DClr_override values for testing devices without DClr syscfg entries
// (enclosure/cover glass):
// gray/black:		setenv DClr_override 000200009B9899002827270000000000
// silver/white:	setenv DClr_override 00020000D8D9D700E4E7E80000000000

static color_policy_invert_t target_cover_glass_color_table[] = {
    { RGB( 39,  39,  40), false },	// Blacker - black background, white logo
    { RGB(228, 231, 232), true  },	// Whiter  - white background, black logo
    // !!!FIXME!!! Remove the colors below when proto devices have been deprecated
    // <rdar://problem/22951458> Remove old J127/J128 DClr values after Proto 2 units are deprecated
    { RGB( 59,  59,  60), false }, // Black - black background, white logo
    { RGB(225, 228, 227), true  }, // White - white background, black logo
};

color_policy_t *
target_color_map_init (enum colorspace cs, color_policy_t *color_policy)
{
    // Must have a color policy structure passed in.
    if (color_policy == NULL)
        goto fail;

    if ((cs != CS_RGB888) && (cs != CS_ARGB8101010))
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
