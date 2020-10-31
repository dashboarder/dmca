/*
 * Copyright (C) 2014-2015 Apple Inc. All rights reserved.
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

#define N102_AP_BOARD_ID		(0x10)	// AP board ID
#define N102_DEV_BOARD_ID		(0x11)	// DEV board ID

#define N102_DEV_BOARD_REV		(0xF)

// Combined Board Id/Board Revision
// IMPORTANT:	IF YOU CHANGE THE ORDER OF THESE ENUMERATORS YOU MUST CHECK
//		FOR ANY > or < RELATIONSHIPS THAT MAY NEED TO ALSO CHANGE.
typedef enum {
    BOARD_TYPE_UNKNOWN = 0,
    BOARD_TYPE_N102    = 1
} board_type;

static uint32_t n102_get_board_rev(void);
static board_type n102_get_board_type(void);

static uint32_t	gpio_board_rev;
static bool gpio_board_rev_valid;

#if (APPLICATION_IBOOT && (PRODUCT_IBOOT || PRODUCT_IBEC))
#include <drivers/mipi/mipi.h>
static mipi_t display_panel_configurations[] = {
	{
		.lanes = 4,
		.esc_div = 4,
		.pll_n = 0,
		.pll_m = 0,
		.pll_p = 0,
		.hsfreq = 0,
		.target_phy_settings = { 9, {{0x44, 1, {0xE}}, {0x30, 1, {0x3F}}, {0x20, 1, {0x45}}, {0x32, 1, {0xA8}}, {0x42, 1, {0x80}}, {0x52, 1, {0x80}}, {0x82, 1, {0x80}}, {0x92, 1, {0x80}}, {0x52, 1, {0x80}}}},	//513 Mhz
	},
};
#endif

MIB_CONSTANT(kMIBTargetBacklight0I2CBus,	kOIDTypeUInt32, 0);
MIB_CONSTANT(kMIBTargetBacklight0I2CAddress,	kOIDTypeUInt32, 0xC6);
MIB_CONSTANT(kMIBTargetOsPictureScale,	kOIDTypeUInt32, 2);
MIB_CONSTANT(kMIBTargetPictureRotate,	kOIDTypeInt32,  0);

static uint32_t n102_get_board_rev(void)
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

static board_type n102_get_board_type(void)
{
    static board_type board_type = BOARD_TYPE_UNKNOWN;

    if (board_type != BOARD_TYPE_UNKNOWN) {
        dprintf(DEBUG_INFO, "%s: board type: %d\n", __func__, board_type);
        return (board_type);
    }

    uint32_t board_id = platform_get_board_id();
    uint32_t board_rev = n102_get_board_rev();

    dprintf(DEBUG_INFO, "%s: board id: %d, rev: %d\n",
            __func__, board_id, board_rev);

    switch (board_id) {
    case N102_AP_BOARD_ID:
    case N102_DEV_BOARD_ID:
        return (BOARD_TYPE_N102);

    default:
        break;
    }

    panic("Unknown/unsupported board id/rev combination: 0x%x/0x%x",
          board_id, board_rev);

    return (BOARD_TYPE_UNKNOWN);
}

static void show_pmu_ver(void)
{
}

void target_early_init(void)
{
    uint32_t target_mipi_p, target_mipi_m, target_mipi_s;

    // Temporary workaround to set MIPI P/M/S values for display
    // D403 (@ 513 MHz) until rdar://problem/17985178 is fixed.
    target_mipi_p = 4;
    target_mipi_m = 171;
    target_mipi_s = 1;

    clock_set_frequency(CLK_MIPI, 0, target_mipi_p,
                        target_mipi_m, target_mipi_s, 0);

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
}

void target_init(void)
{
    dprintf(DEBUG_INFO, "%s ...\n", __func__);

#if WITH_HW_FLASH_NOR
    flash_nor_init(SPI_NOR0);
#endif
}

void target_quiesce_hardware(void)
{
    dprintf(DEBUG_INFO, "%s ...\n", __func__);
}

void target_poweroff(void)
{
    dprintf(DEBUG_INFO, "%s ...\n", __func__);
}


int target_bootprep(enum boot_target target)
{
    dprintf(DEBUG_INFO, "%s ...\n", __func__);
    return 0;
}

bool target_should_recover(void)
{
    return platform_get_request_dfu2() && power_has_usb();
}

bool target_should_poweron(bool *cold_button_boot)
{
#if WITH_HW_POWER
    if (power_get_boot_flag() == kPowerBootFlagColdButton)
        *cold_button_boot = true;
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

    dprintf(DEBUG_INFO, "%s ...\n", __func__);
}
#endif // APPLICATION_IBOOT

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

    env_set("display-timing", "D410", 0);
    env_set("adbe-tunables", "D410",0);
    env_set("adfe-tunables", "D410", 0);
}
#endif

#if WITH_HW_CHESTNUT
uint8_t target_get_lcm_ldos(void)
{
    dprintf(DEBUG_INFO, "%s ...\n", __func__);
    return (DISPLAY_PMU_LDO(0) | DISPLAY_PMU_LDO(1));
}
#endif

#if WITH_HW_LM3534
uint8_t target_lm3534_gpr(uint32_t ctlr)
{
    // DISPLAY_PANEL_TYPE_D410
    // default ovp_select setting is 0 (corresponds to 16V OVP)
    // (ramp disable) | (chip enable)
    return ((1 << 1) | (1 << 0));
}
#endif

#if WITH_DEVICETREE

int target_update_device_tree(void)
{
#if WITH_HW_DISPLAY_PINOT
    DTNode *disp0_node, *backlight_node;
#endif
    board_type board = n102_get_board_type();
    DTNode *node;
    uint32_t propSize;
    char *propName, *propStr;
    void *propData;
    //const char *nodeName;

    dprintf(DEBUG_INFO, "%s ...\n", __func__);

#if WITH_HW_DISPLAY_PINOT
    // Find the DISP0 (display-subsystem 0) node
    FindNode(0, "arm-io/disp0", &disp0_node);

    if (FindNode(0, "arm-io/mipi-dsim/lcd", &node)) {
        extern int pinot_update_device_tree(DTNode *pinot_node,
                                            DTNode *clcd_node,
                                            DTNode *backlight_node);

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

    if (FindNode(0, "arm-io/spi2/multi-touch", &node)) {
        propName = "compatible";
        if (FindProperty(node, &propName, &propData, &propSize)) {
            propStr = NULL;
            switch (board) {
            case BOARD_TYPE_N102:
                propStr = "multi-touch,n102";
                dprintf(DEBUG_INFO, "%s: multi-touch,n102\n", __func__);
                break;

            default:
                panic("Unknown display for board type: %d", board);
                break;
            }
        }
    }

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
                ((uint8_t *)propData)[12] =
                    (((uint8_t *)propData)[12] & ~0x1f) | (vpbr & 0x1f);
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

    // Update the backlight calibration data
    if (FindNode(0, "backlight", &node)) {
        propName = "backlight-calibration";
        if (FindProperty(node, &propName, &propData, &propSize)) {
            syscfgCopyDataForTag('BLCl', propData, propSize);
        }

        // Remove the properties for the second backlight controller
        propName = "dual-backlight-controllers";
        if (FindProperty(node, &propName, &propData, &propSize)) {
            propName[0] = '~';
        }
        propName = "function-backlight_enable1";
        if (FindProperty(node, &propName, &propData, &propSize)) {
            propName[0] = '~';
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
        propName = "gyro-trim-calibration";
        if (FindProperty(node, &propName, &propData, &propSize)) {
            memset(propData, 0, propSize);
            syscfgCopyDataForTag('GTCl', propData, propSize);
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
        if (FindProperty(node, &propName, &propData, &propSize)) {
            syscfgCopyDataForTag('CBAT', propData, propSize);
            if ( ((uint32_t*)propData)[2]==0x1 ) ((uint32_t*)propData)[2]=0x0; // compat
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

// Sample DClr_override values for testing devices without DClr syscfg entries
// (enclosure/cover glass):
// gray/black:		setenv DClr_override 00020000B9B5B4003C3B3B0000000000
// gold/white:		setenv DClr_override 00020000B5CCE100E3E4E10000000000
// silver/white:	setenv DClr_override 00020000D8D9D700E3E4E10000000000

static color_policy_invert_t target_cover_glass_color_table[] = {
    {RGB(59, 59, 60), false},    // Black - black background, white logo
    {RGB(225, 228, 227), true},  // White - white background, black logo
};

color_policy_t *target_color_map_init(enum colorspace cs,
                                      color_policy_t *color_policy)
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
