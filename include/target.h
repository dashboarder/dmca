/*
 * Copyright (C) 2007-2015 Apple Inc. All rights reserved.
 * Copyright (C) 2006 Apple Computer, Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */
#ifndef __TARGET_H
#define __TARGET_H

#include <sys/types.h>
#include <sys/boot.h>
#include <lib/paint.h>

__BEGIN_DECLS

enum target_property {
	TARGET_PROEPRTY_NONE = 0,
	TARGET_PROPERTY_WIFI_MACADDR,
	TARGET_PROPERTY_WIFI_CALIBRATION_TX,
	TARGET_PROPERTY_WIFI_CALIBRATION_TX_24,
	TARGET_PROPERTY_WIFI_CALIBRATION_TX_50,
	TARGET_PROPERTY_WIFI_CALIBRATION_RX_24,
	TARGET_PROPERTY_WIFI_CALIBRATION_RX_50,
	TARGET_PROPERTY_WIFI_CALIBRATION_RX_TEMP,
	TARGET_PROPERTY_WIFI_CALIBRATION_FREQ_GROUP_2G,
	TARGET_PROPERTY_WIFI_BOARD_SNUM,
	TARGET_PROPERTY_WIFI_WCAL,
	TARGET_PROPERTY_BT_MACADDR,
	TARGET_PROPERTY_ETH_MACADDR,
	TARGET_PROPERTY_BB_REGION_SKU,
	TARGET_PROPERTY_RESTORE_BACKLIGHT_LEVEL,
	TARGET_PROPERTY_PINTO_MACADDR,
	TARGET_PROPERTY_WIFI1_MACADDR,
	TARGET_PROPERTY_BT1_MACADDR,
	TARGET_PROPERTY_ETH1_MACADDR,
	TARGET_PROPERTY_BLUETOOTH_DEV_MACADDR0,
	TARGET_PROPERTY_MAX,
};

void target_early_init(void);
void target_init(void);
void target_late_init(void);
bool target_config_ap(void);
bool target_config_dev(void);
int target_debug_init(void);
void target_debug(int);
void target_poweroff(void);
void target_setup_default_environment(void);
void target_quiesce_hardware(void);
bool target_should_recover(void);
bool target_should_poweron(bool *cold_boot);
bool target_should_poweroff(bool at_boot);
int target_get_boot_battery_capacity(void);
int target_get_precharge_gg_flag_mask(void);
bool target_needs_chargetrap(void);
bool target_do_chargetrap(void);
void* target_prepare_dali(void);
int target_bootprep(enum boot_target);
int target_init_boot_manifest(void);
int target_pass_boot_manifest(void);
const uint32_t *target_get_default_gpio_cfg(uint32_t gpioic);
bool target_get_property_base(enum target_property prop, void *data, int maxdata, int *retlen);
bool target_get_property(enum target_property prop, void *data, int maxdata, int *retlen);
int target_update_device_tree(void);
void target_watchdog_tickle(void);
u_int32_t target_lookup_backlight_cal(int index);
uint8_t target_get_lcm_ldos(void);
uint8_t target_lm3534_gpr(uint32_t ctlr);
void * target_get_display_configuration(void);
color_policy_t *target_color_map_init(enum colorspace cs, color_policy_t *color_policy);
int target_dclr_from_clrc(uint8_t *buffer, size_t size);
bool target_is_tethered(void);

#define LED_NONE	0
#define LED_GREEN	1
#define	LED_AMBER	2
#define	LED_RED		3
#define	LED_BLUE	4
#define	LED_CYAN	5
#define	LED_WHITE	6

void target_set_led(int color);

__END_DECLS

#endif
