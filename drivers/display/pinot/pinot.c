/*
 * Copyright (C) 2008-2012 Apple Inc. All rights reserved.
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
#include <drivers/mipi.h>
#include <drivers/power.h>
#include <platform/gpio.h>
#include <platform/gpiodef.h>
#include <lib/env.h>
#include <lib/paint.h>
#include <sys/task.h>
#include <target.h>

#include "pinot.h"

/* Update to Display Sequence spec 080-4502-04 v20,
 * <rdar://problem/10885711> hold the specs. The only change is to
 * use 7 ms instead of 6ms for the delay after reset. */

/* This code is compliant with Display Sequence spec 080-4502-04 v19,
 * <rdar://problem/8153868> holds the spec.  As repeated below, Type 1
 * does not adhere to the spec and never will.  Type 2 comments and
 * diagram references are relative to that document. */

/* The Pinot driver type (1, 2 or 3) really should come from the panel
 * ID.  However, we have a timing constraint on Type 2 and 3 drivers that
 * needs to be honored before the panel ID can be read.  Therefore we
 * bind each target to its appropriate driver type and confirm with
 * the ID.  Type 1 is rare/legacy at this point, so most default to 2. 
 * Type 3 was introduced to improve on displayON sequence */
#ifndef PINOT_TYPE
#define PINOT_TYPE             (2)
#endif

static u_int32_t pinot_panel_id;
static u_int32_t pinot_dot_pitch;
static u_int32_t pinot_default_color;
static u_int32_t pinot_voltage_offset;
static u_int32_t pinot_backlight_cal;
static u_int8_t  pinot_raw_panel_id[15];

static void pinot_enable_reset(bool enable);
static void pinot_enable_power(bool enable);

static int panel_id_2_class(u_int32_t panel_id)
{
	int driver, panel_class;

	driver = panel_id & 0x3f;

	if (driver > DISP_IC_NUGGET)
		panel_class = 3;
	else if (driver < DISP_IC_LIBERTY)
		panel_class = 1;
	else
		panel_class = 2;

	return panel_class;
}

static void pinot_read_panel_id(void)
{
	u_int32_t length;
	int	  result;
	int       panel_class, driver;

	if (pinot_panel_id == 0) {
		pinot_panel_id = env_get_uint("pinot-panel-id", 0);
	}
	if (pinot_panel_id == 0) {
		// read the panel ID
		length = sizeof(pinot_raw_panel_id);
		pinot_raw_panel_id[0] = DSIM_CMD_DEVICE_ID;
		result = mipi_dsim_read_long_command(DSIM_TYPE_GEN_READ_1P, pinot_raw_panel_id, &length);
		if ((result != 0) || (length < 3)) {
			dprintf(DEBUG_CRITICAL, "pinot_init(): read of pinot panel id failed\n");
			return;
		} else {
			pinot_panel_id  = pinot_raw_panel_id[0] << 24;			// Build ID
			pinot_panel_id |= pinot_raw_panel_id[1] << 16;			// Panel Vendor ID
			pinot_panel_id |= (pinot_raw_panel_id[3] & 0xF0) << 8;		// Program ID
			pinot_panel_id |= (pinot_raw_panel_id[2] & 0xF0) << 4;		//
			pinot_panel_id |= (pinot_raw_panel_id[3] & 0x07) << 3;		// Driver Vendor ID
			pinot_panel_id |= (pinot_raw_panel_id[2] & 0x07) << 0;		//
			pinot_panel_id |= (pinot_raw_panel_id[2] & 0x08) << 4;		// Auto Boot Bit
			pinot_panel_id |= (pinot_raw_panel_id[3] & 0x08) << 3;		// Default Color Bit

			pinot_default_color = ((pinot_raw_panel_id[3] >> 3) & 1) ? 0x00000000 : 0x00FFFFFF;
			pinot_voltage_offset = pinot_raw_panel_id[4];
			pinot_backlight_cal = pinot_raw_panel_id[5];
		}
	}

	driver = pinot_panel_id & 0x3f;
	if (driver == DISP_IC_LIBERTY)
		pinot_backlight_cal = target_lookup_backlight_cal((pinot_panel_id >> 24) & 3);

	panel_class = panel_id_2_class(pinot_panel_id);

	//PINOT_TYPE should be removed eventually, but validating old HW has the correct values
	if ((PINOT_TYPE != panel_class) && (panel_class > 3))
		panic("Mismatch between PINOT_TYPE and panel ID");

	dprintf(DEBUG_CRITICAL, "pinot_init(): pinot_panel_id:      0x%08x\n", pinot_panel_id);
	dprintf(DEBUG_CRITICAL, "pinot_init(): pinot_default_color: 0x%08x\n", pinot_default_color);
	dprintf(DEBUG_CRITICAL, "pinot_init(): pinot_backlight_cal: 0x%08x\n", pinot_backlight_cal);
}

int pinot_init(struct display_timing *timing, enum colorspace color, u_int32_t *display_id)
{
	utime_t   start_time;
	int panel_class;

	dprintf(DEBUG_CRITICAL, "pinot_init()\n");

	// Save the display's dot pitch
	pinot_dot_pitch = timing->dot_pitch;

	if (PINOT_TYPE < 2) {
		/*******************************************************************************
		 * The Type 1 sequence is FROZEN as of Durango and shall NEVER BE TOUCHED AGAIN.
		 * There are known excursions from the display sequencing spec.
		 ******************************************************************************/

		// Reset the display and wait
		pinot_enable_reset(true);
		task_sleep(10 * 1000);

		// Initialize the MIPI-DSI logic
		mipi_dsim_init(timing, color);

		// Release the display from reset and wait
		pinot_enable_reset(false);
		task_sleep(7 * 1000);

		// Send a NOP LP command.  Renasas requires a command before HS clock, but we
		// can't send the sleep-out until after ID read (for Samsung).
		mipi_dsim_send_short_command(DSIM_TYPE_DSC_WRITE, DSIM_CMD_NOP, 0x00);

		// Be sure the NOP has been transmitted completely, before going HS.
		// We "know" that 10us is enough.
		spin(10);

		// Enable the high speed clock and wait
		mipi_dsim_enable_high_speed(true);
		task_sleep(25 * 1000);

		// Read the panel ID from the display chip.  If we don't have a valid panel ID, we
		// run the quiesce sequence and bail out.
		pinot_read_panel_id();
		if (pinot_panel_id == 0) {
			mipi_dsim_quiesce();
			return -1;
		}

		// For Renesas Pinot, it is necessary to video- and CLCD-enable before sleep-out.
		// The delivery of a VSS packet packet lets the chip fix a latchup condition.

		// For old SOCs the MIPI video data output must be enabled before CLCD <rdar://problem/5891347>
		// MIPI will leave the link in LP mode until it receives VSYNC from CLCD.
		mipi_dsim_enable_video(true);

		// Ignore the real default color and paint black; program CLCD, start frame timing, take a breath
		pinot_default_color = 0x00000000;
		display_set_enable(true, &pinot_default_color);
		spin(100);

		// Tell the panel to exit sleep, wait 7 frames
		mipi_dsim_send_short_command(DSIM_TYPE_DSC_WRITE, DSIM_CMD_EXIT_SLEEP, 0x00);
		display_delay_frames(7);

		// Turn display on and wait 7 frames before letting the backlight be turned on
		mipi_dsim_send_short_command(DSIM_TYPE_DSC_WRITE, DSIM_CMD_DISPLAY_ON, 0x00);
		display_delay_frames(7);
	} else {
		// Reset the display, and make sure it has power
		pinot_enable_reset(true);
		pinot_enable_power(true);
		// initialize MIPI-DSI while waiting T1
		// Offset mipi_dsim_init giving the panel 100us to see the reset
		// This is important for the iBoot -> iBEC transition
		start_time = system_time();
		spin(100);
		mipi_dsim_init(timing, color);
		while (!time_has_elapsed(start_time, 10 * 1000)) task_yield();

		// Release the display from reset and wait T2.
		pinot_enable_reset(false);
		task_sleep(7 * 1000);

		// T4 and T5 are underway, since PWREN is trigger from reset on these drivers.

		// Tell the panel to exit sleep, then wait T3 plus transmission time (~8us)
		mipi_dsim_send_short_command(DSIM_TYPE_DSC_WRITE, DSIM_CMD_EXIT_SLEEP, 0x00);
		spin(12);

		// Enable the high speed clock and wait T6, latch the start time for T7.
		// Note that T6 technically ends when the panel ID read reaches the display driver,
		// but we're being conservative and measuring until we queue the LP command.
		mipi_dsim_enable_high_speed(true);
		start_time = system_time();
		task_sleep(25 * 1000);

		// Read the panel ID from the display chip.  If we don't have a valid panel ID, we
		// run the quiesce sequence and bail out.
		pinot_read_panel_id();
		if (pinot_panel_id == 0) {
			mipi_dsim_quiesce();
			return -1;
		}

		// Paint the panel's native color, enable the CLCD and wait 100us
		display_set_enable(true, &pinot_default_color);

		// Finish waiting for T7 then send display-on command
		while (!time_has_elapsed(start_time, 100 * 1000)) task_yield();
		mipi_dsim_send_short_command(DSIM_TYPE_DSC_WRITE, DSIM_CMD_DISPLAY_ON, 0x00);

		// We can now enable video data after all LP commands are sent.
		mipi_dsim_enable_video(true);

		// Now the native color is transmitting to the display.  We should guarantee a couple
		// frames of native color before we switch to the black background that we want
		// the user to see.  This delay is part of T9, and we'll do the overlapping T8 
		// delay after setting the background to black.
		panel_class = panel_id_2_class(pinot_panel_id);
		if (panel_class > 2 ) {
			display_delay_frames(1);
		} else {
			display_delay_frames(3);
		}

		// Draw black, wait T8 (and the final two frames of T9) before we allow backlight enable.
		paint_set_bgcolor(0, 0, 0);
		paint_update_image();
		display_delay_frames(2);
	}

#ifdef GPIO_LCD_CHKSUM
	// Turn off pull-down on LCD_CHKSUM
	gpio_configure_pupdn(GPIO_LCD_CHKSUM, GPIO_NO_PUPDN);
#endif

	*display_id = pinot_panel_id;

#if WITH_HW_POWER
	// Adjust analog display voltage offset (down) if necessary
	if (pinot_voltage_offset != 0) {
		bool fixed_boost = env_get_bool("fixed-lcm-boost", false);
		power_set_display_voltage_offset(pinot_voltage_offset, fixed_boost);
	}
#endif

	return 0;
}

int pinot_quiesce(void)
{
	dprintf(DEBUG_CRITICAL, "pinot_quiesce()\n");

	/*******************************************************************************
	 * The Type 1 sequence is FROZEN as of Durango and shall NEVER BE TOUCHED AGAIN.
	 * There are known excursions from the display sequencing spec.  Any changes to
	 * the common code below must be done in a way that preserves Type 1.
	 ******************************************************************************/

	// Wait for T10 (not for Type 1) then send display-off and sleep-in
	if (PINOT_TYPE >= 2)
		display_delay_frames(1);
	mipi_dsim_send_short_command(DSIM_TYPE_DSC_WRITE, DSIM_CMD_DISPLAY_OFF, 0x00);
	mipi_dsim_send_short_command(DSIM_TYPE_DSC_WRITE, DSIM_CMD_ENTER_SLEEP, 0x00);

	// Wait for T12 (which covers T11) then stop clock, reset MIPI and display driver
	display_delay_frames(6);
	// Because of LP transmission uncertainty of up to 2 frames, we need to pad the 
	// delay.  We only do it for Type 2 displays because Type 1 is frozen.
	if (PINOT_TYPE >= 2)
		display_delay_frames(2);

	display_set_enable(false, NULL);

	mipi_dsim_enable_high_speed(false);
	mipi_dsim_quiesce();
	pinot_enable_reset(true);
	pinot_enable_power(false);

	return 0;
}

static void pinot_enable_reset(bool enable)
{
	gpio_write(GPIO_LCD_RST, enable ^ GPIO_LCD_RST_POLARITY ^ 1);
}

static void pinot_enable_power(bool enable)
{
#ifdef GPIO_LCD_PWR_EN
	gpio_write(GPIO_LCD_PWR_EN, enable);
#endif
}

#if WITH_DEVICETREE

#include <lib/devicetree.h>

int pinot_update_device_tree(DTNode *pinot_node, DTNode *clcd_node, DTNode *backlight_node)
{
	u_int32_t	propSize;
	char		*propName;
	void		*propData;

	if (pinot_panel_id == 0) return -1;

	propName = "lcd-panel-id";
	if (FindProperty(pinot_node, &propName, &propData, &propSize)) {
		((u_int32_t *)propData)[0] = pinot_panel_id;
	}

	propName = "raw-panel-id";
	if (FindProperty(pinot_node, &propName, &propData, &propSize)) {
		if (propSize > sizeof(pinot_raw_panel_id)) propSize = sizeof(pinot_raw_panel_id);
		memcpy(propData, pinot_raw_panel_id, propSize);
	}

	if (clcd_node != NULL) {
		propName = "display-default-color";
		if (FindProperty(clcd_node, &propName, &propData, &propSize)) {
			((u_int32_t *)propData)[0] = pinot_default_color;
		}

		propName = "dot-pitch";
		if (FindProperty(clcd_node, &propName, &propData, &propSize)) {
			((u_int32_t *)propData)[0] = pinot_dot_pitch;
		}
	}

	if (backlight_node != NULL) {
		propName = "display-backlight-calibration";
		if (FindProperty(backlight_node, &propName, &propData, &propSize)) {
			((u_int32_t *)propData)[0] = pinot_backlight_cal;
		}
		propName = "backlight-id";
		if (FindProperty(backlight_node, &propName, &propData, &propSize)) {
			if (((u_int32_t *)propData)[0] == 0xffffffff) {
				((u_int32_t *)propData)[0] = (pinot_panel_id >> 8) & 0xff;
			}
		}
	}

	return 0;
}

#endif
