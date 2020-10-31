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
#if (APPLICATION_IBOOT && PRODUCT_IBOOT)
#include <drivers/displayport/displayport.h>
#include <drivers/displayport.h>
#endif
#include <lib/env.h>
#include <lib/mib.h>
#include <lib/paint.h>
#include <lib/syscfg.h>
#include <platform.h>
#include <platform/gpiodef.h>
#include <platform/soc/chipid.h>
#include <target.h>
#include <platform/soc/hwclocks.h>

#define E86_AP_BOARD_ID		(0xE)	// AP board ID
#define E86_DEV_BOARD_ID	(0xF)	// DEV board ID

#define E86_BOARD_REV		(0xF)   //(0xF)

// Combined Board Id/Board Revision
typedef enum {
	BOARD_TYPE_UNKNOWN    = 0,
	BOARD_TYPE_E86_PROTO1 = 1
} board_type;

static uint32_t gpio_board_rev;
static bool gpio_board_rev_valid;

static uint32_t e86_get_board_rev(void);
static board_type e86_get_board_type(void);

#if WITH_HW_DISPLAY_DISPLAYPORT
static dp_t dp_link_config = {
    .mode          = 1,
    .type          = 0,
    .min_link_rate = 0xa,
    .max_link_rate = 0xa,
    .lanes         = 4,
    .ssc           = 0,
    .alpm          = 0,
    .vrr_enable    = 0,
    .vrr_on        = 0,
    .fast_link_training	= false,
};
#endif

MIB_CONSTANT(kMIBTargetOsPictureScale,	kOIDTypeUInt32, 1);
MIB_CONSTANT(kMIBTargetPictureRotate,	kOIDTypeInt32,  0);

static uint32_t e86_get_board_rev (void)
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

static board_type e86_get_board_type (void)
{
    static board_type board_type = BOARD_TYPE_UNKNOWN;

    if (board_type != BOARD_TYPE_UNKNOWN) {
        dprintf(DEBUG_INFO, "%s: board type: %d\n", __func__, board_type);
        return board_type;
    }

    uint32_t board_id = platform_get_board_id();
    uint32_t board_rev = e86_get_board_rev();

    dprintf(DEBUG_INFO, "%s: board id: %d, rev: %d\n",
            __func__, board_id, board_rev);

    if (target_config_ap()) {
        if (E86_AP_BOARD_ID == board_id) {
            if (E86_BOARD_REV == board_rev) {
                return (BOARD_TYPE_E86_PROTO1);
            }
        }
    }

    panic("Unknown/unsupported board id/rev combination: 0x%x/0x%x",
          board_id, board_rev);

    return (BOARD_TYPE_UNKNOWN);
}

static void e86_reset_hoover (bool reset)
{
    power_set_gpio(POWER_GPIO_HVR_RESET_L, 1, !reset);
}

static void target_fixup_pmu(void)
{
#if WITH_HW_POWER
    int pmu_set_data(int dev, uint16_t reg, uint8_t byte, bool do_confirm);
    int pmu_get_data(int dev, uint16_t reg, uint8_t *byte);

    int rc;
    uint8_t val;
    bool fix_ana_trim = false;

    rc = pmu_get_data(0, 0x1, &val);         // read PMU.OTP version

    if (rc < 0) {
        dprintf(DEBUG_CRITICAL, "PMU: cannot read OTP Version (%d)\n", rc);
    } else {
        dprintf(DEBUG_INFO, "%s: PMU OTP ver: %x\n", __func__, val);

        if (val < 0x0d) {
            rc = pmu_get_data(0, 0x01d7, &val);  // read kD2186_BUCK6_ANA_TRIM4

            if (rc < 0) {
                dprintf(DEBUG_INFO, "PMU: cannot read "
                        "kD2186_BUCK6_ANA_TRIM4 (%d)\n", rc);			
            } else {
                fix_ana_trim = true;
            }
        }
    }

    dprintf(DEBUG_INFO, "%s: fix_ana_trim: %d\n", __func__, fix_ana_trim);

    rc = pmu_set_data(0, 0x7000, 0x1d, 0); 	// test mode

    if (rc < 0) {
        dprintf(DEBUG_INFO, "PMU: cannot enter test mode (%d)\n", rc);
    } else {
	// <rdar://problem/16775541> Remove N56 EVT iBoot Overwrites for PMU 
	// to set high gain mode for kD2186_BUCK6_ANA_TRIM4 0x02 has to be
        // OR'd in to preserve exiting bits
        if (fix_ana_trim) {
            // merge in high gain mode
            rc = pmu_set_data(0, 0x01d7, ( val | 0x02 ), 1);

            if (rc < 0) {
                dprintf(DEBUG_INFO, "cannot change "
                        "kD2186_BUCK6_ANA_TRIM4 (%d)\n", rc);
            }
        }

	// <rdar://problem/16871166> N61/12A265: All wakes appended with
        // "button4"
        rc = pmu_set_data(0, 0x0443, 0x35, 1);

        if (rc < 0) {
            dprintf(DEBUG_INFO, "cannot change button4 (%d)\n", rc);
        }

        rc = pmu_set_data(0, 0x7000, 0x00, 0); 

        if (rc < 0) {
            panic("PMU: cannot exit test mode (%d)", rc);
        }
    }
#endif
}

void target_early_init (void)
{
#if PRODUCT_LLB || PRODUCT_IBSS
    dprintf(DEBUG_INFO, "%s: chip rev: %d, fuse: %d\n", __func__,
           chipid_get_chip_revision(), chipid_get_fuse_revision());

    target_fixup_pmu();
#endif
}

void target_late_init (void)
{
    clock_gate(CLK_UART7, 0);

#if WITH_HW_DISPLAY_DISPLAYPORT
    displayport_init(&dp_link_config);
    e86_reset_hoover(false);
#endif
}

void target_init (void)
{
#if WITH_HW_FLASH_NOR
    flash_nor_init(SPI_NOR0);
#endif
}

void target_quiesce_hardware (void)
{
    dprintf(DEBUG_INFO, "%s ...\n", __func__);
}

void target_poweroff (void)
{
    dprintf(DEBUG_INFO, "%s ...\n", __func__);
}

int target_bootprep (enum boot_target target)
{
    dprintf(DEBUG_INFO, "%s ...\n", __func__);
    return 0;
}

bool target_should_recover (void)
{
    return platform_get_request_dfu2() && power_has_usb();
}

bool target_should_poweron (bool *cold_button_boot)
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

bool target_should_poweroff (bool at_boot)
{
    return (platform_get_request_dfu1() && (!at_boot || !power_has_usb()));
}

#if APPLICATION_IBOOT
void target_watchdog_tickle (void)
{
    uint32_t value = gpio_read(GPIO_WDOG_TICKLE);
    gpio_write(GPIO_WDOG_TICKLE, value ^ 1);
    dprintf(DEBUG_INFO, "%s ...\n", __func__);
}
#endif // APPLICATION_IBOOT

void * target_get_display_configuration(void)
{
#if WITH_HW_DISPLAY_DISPLAYPORT
	return ((void *)(&dp_link_config));
#else
	return (NULL);
#endif
}

#if WITH_ENV
void target_setup_default_environment (void)
{
    dprintf(DEBUG_INFO, "%s ...\n", __func__);

    // boot-device is set in platform's init.c
    env_set("wifiaddr", "7E:77:77:77:77:77", 0);
    env_set("display-timing", "720p-e86", 0);
    env_set("adbe-tunables", "default", 0);
}
#endif

#if WITH_DEVICETREE
int target_update_device_tree (void)
{

    dprintf(DEBUG_INFO, "%s ...\n", __func__);

    return 0;
}
#endif // WITH_DEVICETREE
