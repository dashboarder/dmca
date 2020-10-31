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
#include <drivers/flash_nor.h>
#include <drivers/power.h>
#include <lib/env.h>
#include <lib/mib.h>
#include <platform.h>
#include <platform/gpiodef.h>
#include <platform/memmap.h>
#include <platform/soc/chipid.h>
#include <target.h>
#if WITH_HW_DISPLAY_PMU
#include <drivers/display_pmu.h>
#endif
#if (ELBAUIREF && APPLICATION_IBOOT && (PRODUCT_IBOOT || PRODUCT_IBEC))
#include <drivers/displayport/displayport.h>
#include <drivers/displayport.h>
dp_t dp;
#endif

MIB_CONSTANT(kMIBTargetOsPictureScale,	kOIDTypeUInt32, 2);
MIB_CONSTANT(kMIBTargetPictureRotate,	kOIDTypeInt32,  -90);

void
target_early_init (void)
{
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
#if (ELBAUIREF && APPLICATION_IBOOT && (PRODUCT_IBOOT || PRODUCT_IBEC))
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
    env_set("boot-args", " debug=0x14e serial=3 amfi=3 amfi_get_out_of_my_way=1 cs_enforcement_disable=1 fips_mode=0", 0);
    // boot-device is set in platform's init.c
    env_set("boot-partition", "0", 0);
    env_set("boot-path", "/System/Library/Caches/com.apple.kernelcaches/kernelcache", 0);

    env_set("display-color-space","RGB888", 0);

    env_set("display-timing", "ipad6d", 0);
    env_set("adbe-tunables", "ipad6d", 0);
    env_set("adfe-tunables", "ipad6d", 0);
}

#endif

#if WITH_DEVICETREE

int
target_update_device_tree (void)
{
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

   return 0;
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
