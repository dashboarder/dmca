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
#if WITH_HW_DISPLAY_PMU
#include <drivers/display_pmu.h>
#endif
#include <drivers/power.h>
#include <lib/env.h>
#include <lib/mib.h>
#include <platform.h>
#include <platform/gpiodef.h>
#include <platform/soc/chipid.h>
#include <platform/soc/hwclocks.h>
#include <target.h>

#if WITH_HW_LM3534
MIB_CONSTANT(kMIBTargetBacklight0I2CBus,	kOIDTypeUInt32, BACKLIGHT_IIC_BUS);
MIB_CONSTANT(kMIBTargetBacklight0I2CAddress,	kOIDTypeUInt32, 0xc6);
MIB_CONSTANT(kMIBTargetBacklight1I2CBus,	kOIDTypeUInt32, BACKLIGHT2_IIC_BUS);
MIB_CONSTANT(kMIBTargetBacklight1I2CAddress,	kOIDTypeUInt32, 0xc6);
#endif

MIB_CONSTANT(kMIBTargetOsPictureScale, kOIDTypeUInt32, 3);
#if TARGET_DISPLAY_D620
#if (APPLICATION_IBOOT && (PRODUCT_IBOOT || PRODUCT_IBEC))
#include <drivers/mipi/mipi.h>
static mipi_t display_panel_configurations[] = {
	{
		.lanes = 4,
		.esc_div = 8,
		.pll_n = 0x0,
		.pll_m = 0x30,
		.pll_p = 0x0,
		.hsfreq = 0x1b,
		.target_phy_settings = { 4, {{0x44, 1, {0x36}}, {0x30, 1, {0x3F}}, {0x20, 1, {0x45}}, {0x32, 1, {0x28}}}},	//1200 Mhz
	},
};
#endif
#endif
MIB_CONSTANT(kMIBTargetPictureRotate, kOIDTypeInt32, 0);

void target_early_init(void)
{
}

void target_late_init(void)
{
    power_set_gpio(GPIO_PMU_NAND_LOW_POWER_MODE, 1, 1);
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

void * target_get_display_configuration(void)
{
#if (TARGET_DISPLAY_D620 && APPLICATION_IBOOT && (PRODUCT_IBOOT || PRODUCT_IBEC))
	return ((void *)(&display_panel_configurations[0]));
#else
	return NULL;
#endif
}

#if WITH_ENV
void target_setup_default_environment(void)
{
    // boot-device is set in platform's init.c
    env_set("boot-partition", "0", 0);
    env_set("boot-path", "/System/Library/Caches/com.apple.kernelcaches/kernelcache", 0);
    env_set("wifiaddr", "7E:77:77:77:77:77", 0);
    env_set("display-color-space","RGB888", 0);
#if TARGET_DISPLAY_D620
    env_set("display-timing", "D620", 0);
    env_set("adbe-tunables", "D620", 0);
    env_set("adfe-tunables", "D620", 0);
#endif
}
#endif

#if WITH_HW_CHESTNUT
uint8_t target_get_lcm_ldos(void)
{
    return (DISPLAY_PMU_LDO(0) | DISPLAY_PMU_LDO(1));
}
#endif

#if WITH_DEVICETREE
int target_update_device_tree(void)
{
#if WITH_HW_DISPLAY_PINOT
    DTNode      *disp0_node, *backlight_node;
#endif
#if WITH_HW_MIPI_DSIM
    DTNode      *node;
#endif

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

#if WITH_HW_DISPLAY_PMU
    display_pmu_update_device_tree("arm-io/i2c0/display-pmu");
#endif

    return 0;
}
#endif // WITH_DEVICETREE

bool charger_set_usb_brick_detect(int dock, int select)
{
    return platform_set_usb_brick_detect(select);
}
