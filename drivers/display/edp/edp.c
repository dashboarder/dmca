/*
 * Copyright (C) 2011,2015 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */
#include <debug.h>
#include <drivers/display.h>
#include <drivers/displayport/displayport.h>
#include <drivers/displayport.h>
#include <drivers/power.h>
#include <platform/gpio.h>
#include <platform/gpiodef.h>
#include <lib/env.h>
#include <sys/task.h>

#include "edp.h"

#define	kHPDTimeout	(186 * 1000)

/*
 * Panel ID is based on "DP Panel Vendor ID spec, v1.5"
 */

static u_int32_t edp_panel_id;
static u_int32_t edp_dot_pitch;
static u_int32_t edp_default_color;
static u_int32_t edp_voltage_offset;
static u_int32_t edp_backlight_cal;
static u_int8_t  edp_raw_panel_id[kEDPRawPanelIdLength];
static dp_t edp_display_config;

int edp_quiesce(void);
static void edp_panel_enable(bool enable);

int edp_init(struct display_timing *timing, enum colorspace color, u_int32_t *display_id)
{
	int	  result;
	utime_t	  HPDTime = 0;

	dprintf(DEBUG_CRITICAL, "edp_init()\n");

	// Save the display's dot pitch
	edp_dot_pitch = timing->dot_pitch;
	memcpy(&edp_display_config,  timing->display_config, sizeof(dp_t));

	// Kick off eDP controller
	if ((result = displayport_init_with_timing_info(timing)) != 0)
		goto error;

	// Power-up the display
	edp_panel_enable(true);

	//Wait for HPD to allow enough time for TCON to to be ready
	if (dp_controller_wait_for_edp_hpd(&HPDTime) == false) {
		dprintf(DEBUG_CRITICAL, "timeout waiting for HPD\n");
		goto error;
	}

	if (edp_panel_id == 0) {
		edp_panel_id = env_get_uint("pinot-panel-id", 0);
	}
	if (edp_panel_id == 0) {
		// read the panel ID
		result = displayport_get_raw_panel_id(edp_raw_panel_id);
		if (result != 0) {
			dprintf(DEBUG_CRITICAL, "edp_init(): read of edp panel id failed\n");
		} else {			
			edp_panel_id  = edp_raw_panel_id[0] << 24;	// Build ID
			edp_panel_id |= edp_raw_panel_id[1] << 16;	// Panel Vendor ID
			edp_panel_id |= edp_raw_panel_id[2] << 8;	// Program ID
			edp_panel_id |= edp_raw_panel_id[3] << 0;	// Silicon Vendor/Model ID

			edp_default_color = (edp_raw_panel_id[0] & 1) ? 0x00000000 : 0x00FFFFFF;
			edp_voltage_offset = edp_raw_panel_id[4];
			edp_backlight_cal = edp_raw_panel_id[5];
		}
	}

	dprintf(DEBUG_CRITICAL, "edp_init(): edp_panel_id:      0x%08x\n", edp_panel_id);
	dprintf(DEBUG_CRITICAL, "edp_init(): edp_default_color: 0x%08x\n", edp_default_color);
	dprintf(DEBUG_CRITICAL, "edp_init(): edp_backlight_cal: 0x%08x\n", edp_backlight_cal);

	// Ignore the real default color and use black
	edp_default_color = 0x00000000;

	if (edp_panel_id == 0) {
		dprintf(DEBUG_CRITICAL, "panel_id = 0\n");
		goto error;
	}

	// the HPD signal causes the rest of the DP initialization to occur on the dp_controller_task
	// we hold the rest of the enable to prevent racing this task and hence violating the panel's timing requirement
	utime_t timeout = HPDTime + kHPDTimeout;
	dp_device_wait_started(timeout);

	// Enable the CLCD
	display_set_enable(true, &edp_default_color);

	*display_id = edp_panel_id;

	// Adjust display voltage offset
#if WITH_HW_POWER_DIALOG
	if (edp_voltage_offset != 0) {
		bool fixed_boost = env_get_bool("fixed-lcm-boost", false);
		power_set_display_voltage_offset(edp_voltage_offset, fixed_boost);
	}
#endif

	//Allow enough time before enabling backlight. Per spec at least 134ms (T4 + T5 + T6 + T7) after seeing HPD. 
	dp_controller_wait_for_HPD_to_BL(HPDTime);

	return 0;
	
error:
	edp_quiesce();
	return -1;
}

int edp_quiesce(void)
{
	dprintf(DEBUG_CRITICAL, "edp_quiesce()\n");

	displayport_quiesce();
	edp_panel_enable(false);

	return 0;
}

static void edp_panel_enable(bool enable)
{
#if PMU_LCD_PWR_EN
	power_set_gpio(GPIO_PMU_LCD_PWR_EN, 1, enable);
#else
	gpio_write(GPIO_LCD_PWR_EN, enable);
#endif
}

#if WITH_DEVICETREE

#include <lib/devicetree.h>

int edp_update_device_tree(DTNode *edp_node, DTNode *lcd_node, DTNode *clcd_node, DTNode *backlight_node)
{
	u_int32_t	propSize;
	char		*propName;
	void		*propData;

	if (edp_panel_id == 0) return -1;

	propName = "lcd-panel-id";
	if (FindProperty(edp_node, &propName, &propData, &propSize)) {
		((u_int32_t *)propData)[0] = edp_panel_id;
	}

	propName = "raw-panel-id";
	if (FindProperty(edp_node, &propName, &propData, &propSize)) {
		if (propSize > sizeof(edp_raw_panel_id)) propSize = sizeof(edp_raw_panel_id);
		memcpy(propData, edp_raw_panel_id, propSize);
	}
	
	propName = "pre_configured";
	if (FindProperty(edp_node, &propName, &propData, &propSize)) {
		if (propSize >= sizeof(u_int8_t))
			((u_int32_t *)propData)[0] = displayport_video_configured() ? 1 : 0;
	}
	

	if (lcd_node != NULL) {

		propName = "alpm-enable";
		if (FindProperty(lcd_node, &propName, &propData, &propSize)) {
			if (propSize == sizeof(uint32_t)) {
				uint32_t data = dp_device_is_alpm_enabled();
				((uint32_t*)propData)[0] = data;
			}
		}

		propName = "lcd-panel-id";
		if (FindProperty(lcd_node, &propName, &propData, &propSize)) {
			((u_int32_t *)propData)[0] = edp_panel_id;
		}

		propName = "raw-panel-id";
		if (FindProperty(lcd_node, &propName, &propData, &propSize)) {
			if (propSize > sizeof(edp_raw_panel_id)) propSize = sizeof(edp_raw_panel_id);
			memcpy(propData, edp_raw_panel_id, propSize);
		}
	}

	if (clcd_node != NULL) {
		propName = "display-default-color";
		if (FindProperty(clcd_node, &propName, &propData, &propSize)) {
			((u_int32_t *)propData)[0] = edp_default_color;
		}

		propName = "dot-pitch";
		if (FindProperty(clcd_node, &propName, &propData, &propSize)) {
			((u_int32_t *)propData)[0] = edp_dot_pitch;
		}
		propName = "vrr-version";
		if (FindProperty(clcd_node, &propName, &propData, &propSize)) {
			if (propSize == sizeof(uint32_t)) {
				uint32_t data = edp_display_config.vrr_on << 31 | edp_display_config.vrr_enable;
				((uint32_t*)propData)[0] = data;
			}
		}
	}

	if (backlight_node != NULL) {
		propName = "display-backlight-calibration";
		if (FindProperty(backlight_node, &propName, &propData, &propSize)) {
			((u_int32_t *)propData)[0] = edp_backlight_cal;
		}
		propName = "backlight-id";
		if (FindProperty(backlight_node, &propName, &propData, &propSize)) {
			if (((u_int32_t *)propData)[0] == 0xffffffff) {
				// We are moving DP panel program-id by 1000 to not collide with 
				// MIPI panel program ids
				((u_int32_t *)propData)[0] = ((edp_panel_id >> 8) & 0xff) + 1000;
			}
		}
	}

	return 0;
}

#endif
