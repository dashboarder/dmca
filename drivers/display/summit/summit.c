/*
 * Copyright (C) 2013 Apple Inc. All rights reserved.
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
#ifdef WITH_HW_DISPLAY_PMU
#include <drivers/display_pmu.h>
#endif //WITH_HW_DISPLAY_PMU

#include "summit.h"

#define	SUMMIT_DISABLE_DSI_PROTOCOL_ERROR_WA	1
static u_int32_t summit_panel_id;
static u_int32_t summit_dot_pitch;
static u_int32_t summit_default_color;
static u_int32_t summit_ldo_voltage;
static u_int32_t summit_boost_voltage;
static u_int32_t summit_backlight_cal;
static u_int8_t  summit_raw_panel_id[15];
static uint32_t	summit_sram_address[4]; //COL:start and end
			               //Page:start and end

static void summit_enable_reset(bool enable);
static void summit_enable_power(bool enable);
static void summit_set_brightness(uint16_t level);
static void summit_enable_brightness_control(bool enable);
static void summit_set_packet_length(uint16_t length);
static void summit_sram_update(struct display_timing *timing);
#ifdef SUMMIT_DISABLE_DSI_PROTOCOL_ERROR_WA
static void summit_disable_dsi_protocol_error_wa(bool lock_mcs);
#endif
struct display_timing *mipi_timing;


static int panel_id_2_class(u_int32_t panel_id)
{
	int driver, panel_class = 1;

	driver = panel_id & 0x3f;

	if (driver > DISP_IC_SUMMIT_A0)
		panel_class = 2;

	return panel_class;
}

static void summit_read_panel_id(void)
{
	u_int32_t length;
	int	  result;
	int       panel_class, driver;

	if (summit_panel_id == 0) {
		summit_panel_id = env_get_uint("summit-panel-id", 0);
	}
	if (summit_panel_id == 0) {
		
		// read the panel ID
		length = sizeof(summit_raw_panel_id);
		summit_raw_panel_id[0] = DSIM_CMD_DEVICE_ID;
		result = mipi_dsim_read_long_command(DSIM_TYPE_GEN_READ_1P, summit_raw_panel_id, &length);
		if ((result != 0) || (length < 3)) {
			dprintf(DEBUG_CRITICAL, "summit_init(): read of summit panel id failed\n");
			return;
		} else {
			summit_panel_id  = summit_raw_panel_id[0] << 24;			// Build ID
			summit_panel_id |= summit_raw_panel_id[1] << 16;			// Panel Vendor ID
			summit_panel_id |= (summit_raw_panel_id[3] & 0xF0) << 8;		// Program ID
			summit_panel_id |= (summit_raw_panel_id[2] & 0xF0) << 4;		//
			summit_panel_id |= (summit_raw_panel_id[3] & 0x07) << 3;		// Driver Vendor ID
			summit_panel_id |= (summit_raw_panel_id[2] & 0x07) << 0;		//
			summit_panel_id |= (summit_raw_panel_id[2] & 0x08) << 4;		// Auto Boot Bit
			summit_panel_id |= (summit_raw_panel_id[3] & 0x08) << 3;		// Default Color Bit

			summit_default_color = ((summit_raw_panel_id[3] >> 3) & 1) ? 0x00000000 : 0x00FFFFFF;
			summit_ldo_voltage = summit_raw_panel_id[4];
			summit_boost_voltage = summit_raw_panel_id[5];
		}
	}

	driver = summit_panel_id & 0x3f;
	panel_class = panel_id_2_class(summit_panel_id);

	dprintf(DEBUG_CRITICAL, "summit_init(): summit_panel_id:      0x%08x\n", summit_panel_id);
	dprintf(DEBUG_CRITICAL, "summit_init(): summit_default_color: 0x%08x\n", summit_default_color);
	dprintf(DEBUG_CRITICAL, "summit_init(): summit_ldo_voltage: 0x%08x\n", summit_ldo_voltage);
	dprintf(DEBUG_CRITICAL, "summit_init(): summit_boost_voltage: 0x%08x\n", summit_boost_voltage);
}

static void summit_enable_reset(bool enable)
{
#ifdef GPIO_PMU_LCD_RST
	power_gpio_configure(GPIO_PMU_LCD_RST, enable ? 0 : 0x38);
#else
	gpio_write(GPIO_LCD_RST, enable ^ GPIO_LCD_RST_POLARITY ^ 1);
#endif
}

static void summit_enable_power(bool enable)
{
#ifdef GPIO_PMU_LCD_PWR_EN
	power_set_gpio(GPIO_PMU_LCD_PWR_EN, 1, 1);
#endif
#ifdef PMU_LCD_PWR_DVDD
#ifdef PMU_LCD_PWR_VCI
	if (enable) {
		power_enable_ldo(PMU_LCD_PWR_DVDD, enable);
		power_enable_ldo(PMU_LCD_PWR_VCI, enable);
	} else {
		power_enable_ldo(PMU_LCD_PWR_VCI, enable);
		spin(54 * 1000);
		power_enable_ldo(PMU_LCD_PWR_DVDD, enable);
	}
#endif // PMU_LCD_PWR_VCI
#endif // PMU_LCD_PWR_DVDD
#ifdef GPIO_LCD_PWR_EN
	gpio_write(GPIO_LCD_PWR_EN, enable);
#endif
}

static int summit_init_video_mode(struct display_timing *timing, enum colorspace color, u_int32_t *display_id)
{
	utime_t   start_time;
	int panel_driver;
	UInt8 data[26] = "";
	uint16_t brightness_level = 0xff03;

	// Reset the display, and make sure it has power
	summit_enable_reset(true);
	summit_enable_power(true);
	// initialize MIPI-DSI while waiting T1
	// Offset mipi_dsim_init giving the panel 100us to see the reset
	// This is important for the iBoot -> iBEC transition
	start_time = system_time();
	spin(100);
	mipi_dsim_init(timing, color);
	while (!time_has_elapsed(start_time, 10 * 1000)) task_yield();

	// Release the display from reset and wait T2.
	summit_enable_reset(false);
	task_sleep(7 * 1000);

	// T4 and T5 are underway, since PWREN is trigger from reset on these drivers.

	// Enable the high speed clock and wait T6, latch the start time for T7.
	// Note that T6 technically ends when the panel ID read reaches the display driver,
	// but we're being conservative and measuring until we queue the LP command.
	mipi_dsim_enable_high_speed(true);
	start_time = system_time();
	task_sleep(25 * 1000);

	// set packet length
	summit_set_packet_length(0x20);

	spin(1 * 1000);

	// Read the panel ID from the display chip.  If we don't have a valid panel ID, we
	// run the quiesce sequence and bail out.
	summit_read_panel_id();
	if (summit_panel_id == 0) {
		mipi_dsim_quiesce();
		return -1;
	}

	panel_driver = summit_panel_id & 0x3f;

	if (panel_driver == DISP_IC_SUMMIT_A0) {
		summit_set_brightness(brightness_level);

		summit_enable_brightness_control(true);

		mipi_dsim_send_short_command(DSIM_TYPE_DSC_WRITE, DSIM_CMD_EXIT_SLEEP, 0x00);

		spin(5 * 1000);

		//Unlock MCS registers
		mipi_dsim_send_short_command(DSIM_TYPE_DSC_WRITE_1P, 0xB0, 0xAC);

		spin(1 * 1000);

		//6. Set to internal oscillator 
		data[0] = 0xC9;
		data[1] = 0xF7;
		data[2] = 0xF8;
		data[3] = 0x02;
		data[4] = 0x02;
		data[5] = 0x02;
		mipi_dsim_send_long_command(DSIM_TYPE_GEN_LONG_WRITE, data, 6);

		spin(5 * 1000);

#if SUMMIT_DISABLE_DSI_PROTOCOL_ERROR_WA
		summit_disable_dsi_protocol_error_wa(false);
#endif //SUMMIT_DISABLE_DSI_PROTOCOL_ERROR_WA


		// Paint the panel's native color, enable the CLCD and wait 100us
		display_set_enable(true, &summit_default_color);
		spin(32*1000);

		// Finish waiting for T7 then send display-on command
		//while (!time_has_elapsed(start_time, 100 * 1000)) task_yield();
		mipi_dsim_send_short_command(DSIM_TYPE_DSC_WRITE, DSIM_CMD_DISPLAY_ON, 0x00);

		//8. Set up some more MCS registers that weren't programmed correctly in OTP
		data[0] = 0xCE;
		data[1] = 0x00;
		data[2] = 0x10;
		data[3] = 0x10;
		data[4] = 0x10;
		mipi_dsim_send_long_command(DSIM_TYPE_GEN_LONG_WRITE, data, 5);

		spin(1 * 1000);

		data[0] = 0xCF;
		data[1] = 0x15;
		mipi_dsim_send_long_command(DSIM_TYPE_GEN_LONG_WRITE, data, 2);

		//lock MCS registers
		mipi_dsim_send_short_command(DSIM_TYPE_DSC_WRITE_1P, 0xB0, 0xCA);

		// We can now enable video data after all LP commands are sent.
		mipi_dsim_enable_video(true);

		// Now the native color is transmitting to the display.  We should guarantee a couple
		// frames of native color before we switch to the black background that we want
		// the user to see.  This delay is part of T9, and we'll do the overlapping T8 
		// delay after setting the background to black.
		display_delay_frames(3);

		// Draw black, wait T8 (and the final two frames of T9) before we allow backlight enable.
		paint_set_bgcolor(0, 0, 0);
		paint_update_image();
		display_delay_frames(2);

#ifdef GPIO_LCD_CHKSUM
		// Turn off pull-down on LCD_CHKSUM
		gpio_configure_pupdn(GPIO_LCD_CHKSUM, GPIO_NO_PUPDN);
#endif
	} else {

#ifdef WITH_HW_DISPLAY_PMU
		display_pmu_init();
#endif // WITH_HW_DISPLAY_PMU

		// We can now enable video data after all LP commands are sent.
		mipi_dsim_enable_video(true);

		// Paint the panel's native color, enable the CLCD and wait 100us
		display_set_enable(true, &summit_default_color);
		display_delay_frames(1);

		mipi_dsim_send_short_command(DSIM_TYPE_DSC_WRITE, DSIM_CMD_EXIT_SLEEP, 0x00);

		spin(5 * 1000);

		summit_set_brightness(brightness_level);
		summit_enable_brightness_control(true);

#if SUMMIT_DISABLE_DSI_PROTOCOL_ERROR_WA
		summit_disable_dsi_protocol_error_wa(true);
#endif //SUMMIT_DISABLE_DSI_PROTOCOL_ERROR_WA

		// Paint the panel's native color, enable the CLCD and wait 100us
		display_set_enable(true, &summit_default_color);
		spin(32*1000);

		// Finish waiting for T7 then send display-on command
		//while (!time_has_elapsed(start_time, 100 * 1000)) task_yield();
		display_delay_frames(5);
		mipi_dsim_send_short_command(DSIM_TYPE_DSC_WRITE, DSIM_CMD_DISPLAY_ON, 0x00);

		// Now the native color is transmitting to the display.  We should guarantee a couple
		// frames of native color before we switch to the black background that we want
		// the user to see.  This delay is part of T9, and we'll do the overlapping T8 
		// delay after setting the background to black.
		display_delay_frames(1);

		// Draw black, wait T8 (and the final two frames of T9) before we allow backlight enable.
		paint_set_bgcolor(0, 0, 0);
		paint_update_image();
		display_delay_frames(2);

#ifdef GPIO_LCD_CHKSUM
		// Turn off pull-down on LCD_CHKSUM
		gpio_configure_pupdn(GPIO_LCD_CHKSUM, GPIO_NO_PUPDN);
#endif
	}
		
	*display_id = summit_panel_id;

	return 0;
}

static int summit_do_panel_calibration()
{
	UInt8 data[] = { 0x51, 0xff, 0x3};

	return mipi_dsim_send_long_command(DSIM_TYPE_GEN_LUT_WRITE, data, 3);
}

static int summit_init_cmd_mode(struct display_timing *timing, enum colorspace color, u_int32_t *display_id)
{
	utime_t		start_time;
	uint32_t	brightness_level;

	mipi_timing = timing;

	// Reset the display, and make sure it has power
	summit_enable_reset(true);
	summit_enable_power(true);

	// initialize MIPI-DSI while waiting T1
	// Offset mipi_dsim_init giving the panel 1.5 ms to see the reset
	start_time = system_time();
	mipi_dsim_init(timing, color);
	while (!time_has_elapsed(start_time, 15 * 100)) task_yield();

	// Release the display from reset and wait T2.
	summit_enable_reset(false);
	task_sleep(5 * 1000);

	// Enable the high speed clock and latch the start time for T4.
	// Note that T4 technically ends when the panel ID read reaches the display driver,
	// but we're being conservative and measuring until we queue the LP command.
	mipi_dsim_enable_high_speed(true);
	start_time = system_time();

	// set packet length
	summit_set_packet_length(0x20);

	spin(1 * 1000);

	// Read the panel ID from the display chip.  If we don't have a valid panel ID, we
	// run the quiesce sequence and bail out.
	summit_read_panel_id();
	if (summit_panel_id == 0) {
		mipi_dsim_quiesce();
		return -1;
	}

#ifdef WITH_HW_DISPLAY_PMU
	display_pmu_init();
#endif // WITH_HW_DISPLAY_PMU

	//Wait for T4 to complete
	while (!time_has_elapsed(start_time, 5 * 1000)) task_yield();

	summit_sram_update(timing);

	//summit based panels set brightness vs backlight.
	//brightness_level = env_get_uint("backlight-level", 0xffff);
	brightness_level = 0xff03;
	summit_set_brightness(brightness_level);

	summit_enable_brightness_control(true);

	//Sleep out and latch for T5a
	start_time = system_time();
	mipi_dsim_send_short_command(DSIM_TYPE_DSC_WRITE, DSIM_CMD_EXIT_SLEEP, 0x00);
	while (!time_has_elapsed(start_time, 5 * 1000)) task_yield();

	// Finish waiting for T5 then send display-on command
	while (!time_has_elapsed(start_time, 108 * 1000)) task_yield();

#if SUMMIT_DISABLE_DSI_PROTOCOL_ERROR_WA
	summit_disable_dsi_protocol_error_wa(true);
#endif //SUMMIT_DISABLE_DSI_PROTOCOL_ERROR_WA
	//Display On
	mipi_dsim_send_short_command(DSIM_TYPE_DSC_WRITE, DSIM_CMD_DISPLAY_ON, 0x00);

	// We can now enable video data after all LP commands are sent.
	mipi_dsim_enable_video(true);

	// Paint the panel's native color, enable adp/adbe
	display_set_enable(true, &summit_default_color);

	paint_set_bgcolor(0, 0, 0);
	paint_update_image();
	display_delay_frames(1);

	*display_id = summit_panel_id;

	return 0;
}

static void summit_set_brightness(uint16_t level)
{
	UInt8 data[3] = "";

	data[0] = 0x51;
	data[1] = (level >> 2) & 0xff;
	data[2] = (level & 0x3);
	mipi_dsim_send_long_command(DSIM_TYPE_GEN_LUT_WRITE, data, 3);

}

static void summit_enable_brightness_control(bool enable)
{
	mipi_dsim_send_short_command(DSIM_TYPE_DSC_WRITE_1P, 0x53, enable ? 0x20 : 0);
}

static void summit_set_packet_length(uint16_t length)
{
	mipi_dsim_send_short_command(0x37, length & 0xff, (length >> 8) & 0xff);
}

#define	SUMMIT_STARTING_COLUMN_ADDRESS	0x0
#define	SUMMIT_STARTING_PAGE_ADDRESS	0x0

static void summit_sram_update(struct display_timing *timing)
{
	uint8_t data[5] = "";
	uint16_t  sram_start_addr, sram_end_addr;

	sram_end_addr = (timing->h_active - 1);
	sram_start_addr = SUMMIT_STARTING_COLUMN_ADDRESS;
	summit_sram_address[0] = sram_start_addr;
	summit_sram_address[1] = sram_end_addr;
	data[0] = 0x2a;
	data[1] = ((sram_start_addr >> 8) & 0xff);
	data[2] = ((sram_start_addr >> 0) & 0xff);
	data[3] = ((sram_end_addr >> 8) & 0xff);
	data[4] = ((sram_end_addr >> 0) & 0xff);
	mipi_dsim_send_long_command(0x39, data, sizeof(data));

	sram_end_addr = (timing->v_active - 1);
	sram_start_addr = SUMMIT_STARTING_COLUMN_ADDRESS;
	summit_sram_address[2] = sram_start_addr;
	summit_sram_address[3] = sram_end_addr;
	data[0] = 0x2b;
	data[1] = ((sram_start_addr >> 8) & 0xff);
	data[2] = ((sram_start_addr >> 0) & 0xff);
	data[3] = ((sram_end_addr >> 8) & 0xff);
	data[4] = ((sram_end_addr >> 0) & 0xff);
	mipi_dsim_send_long_command(0x39, data, sizeof(data));

	data[0] = 0x2c;
	data[1] = 0xff;
	mipi_dsim_send_long_command(0x39, data, 2);
}

//
//Global functions
//

int summit_init(struct display_timing *timing, enum colorspace color, u_int32_t *display_id)
{
	bool is_video_mode = true;

#ifdef TARGET_DISP_VIDEO_MODE
	is_video_mode = TARGET_DISP_VIDEO_MODE;
#endif //TARGET_DISP_VIDEO_MODE

	dprintf(DEBUG_CRITICAL, "summit_init %s mode()\n", is_video_mode ? "video" : "command");

	// Save the display's dot pitch
	summit_dot_pitch = timing->dot_pitch;

	if (is_video_mode) {
		return summit_init_video_mode(timing, color, display_id);
	} else {
		return summit_init_cmd_mode(timing, color, display_id);
	}

}

int summit_quiesce(void)
{
	// send display-off and wait T7
	mipi_dsim_send_short_command(DSIM_TYPE_DSC_WRITE, DSIM_CMD_DISPLAY_OFF, 0x00);
	spin(18 * 1000);

	mipi_dsim_send_short_command(DSIM_TYPE_DSC_WRITE, DSIM_CMD_ENTER_SLEEP, 0x00);

	// Wait for T8 then stop clock, reset MIPI and display driver
	display_delay_frames(6);
	// Because of LP transmission uncertainty of up to 2 frames, we need to pad the 
	// delay.  We only do it for Type 2 displays because Type 1 is frozen.
	display_delay_frames(2);

	mipi_dsim_enable_high_speed(false);
	mipi_dsim_quiesce();
	summit_enable_reset(true);
	summit_enable_power(false);

	return 0;
}

#ifdef SUMMIT_DISABLE_DSI_PROTOCOL_ERROR_WA
static void summit_disable_dsi_protocol_error_wa(bool lock_mcs)
{
	uint8_t data1[6] = "";
	uint32_t length;

	if (lock_mcs) {
		//Unlock MCS registers
		mipi_dsim_send_short_command(DSIM_TYPE_DSC_WRITE_1P, 0xB0, 0xAC);
		spin(1 * 1000);
	}

	data1[0] = 0xd4;
	data1[1] = 0x07;
	data1[2] = 0xc0;
	data1[3] = 0x80;
	data1[4] = 0x00;
	data1[5] = 0x80;
	length = sizeof(data1);
	mipi_dsim_send_long_command(DSIM_TYPE_GEN_LONG_WRITE, data1, length);

	if (lock_mcs) {
		//lock MCS registers
		mipi_dsim_send_short_command(DSIM_TYPE_DSC_WRITE_1P, 0xB0, 0xCA);
	}
}
#endif

int8_t summit_get_raw_panel_id(uint8_t *raw_panel_id, size_t raw_panel_id_length)
{
	RELEASE_ASSERT(raw_panel_id != NULL);
	RELEASE_ASSERT(raw_panel_id_length >= sizeof(summit_raw_panel_id));

	bcopy(summit_raw_panel_id, raw_panel_id, sizeof(summit_raw_panel_id));

	return 0;
}

#if WITH_DEVICETREE

#include <lib/devicetree.h>

int summit_update_device_tree(DTNode *summit_node, DTNode *clcd_node, DTNode *backlight_node)
{
	u_int32_t	propSize;
	char		*propName;
	void		*propData;
	char		*PropNameFuncDisable = "function-backlight_inval";
	char		*PropNameFuncEnable = "function-backlight_enable";
	char		*PropNameFuncUpdate = "function-backlight_update";
	char		*PropNameFuncNoUpdate = "function-backlight_noupdate";

	if (summit_panel_id == 0) return -1;

	propName = "lcd-panel-id";
	if (FindProperty(summit_node, &propName, &propData, &propSize)) {
		((u_int32_t *)propData)[0] = summit_panel_id;
	}

	propName = "raw-panel-id";
	if (FindProperty(summit_node, &propName, &propData, &propSize)) {
		if (propSize > sizeof(summit_raw_panel_id)) propSize = sizeof(summit_raw_panel_id);
		memcpy(propData, summit_raw_panel_id, propSize);
	}
#ifdef TARGET_DISP_VIDEO_MODE
	//tell the os if video or command mode
	propName = "video-mode";
	if (FindProperty(summit_node, &propName, &propData, &propSize)) {
		((u_int32_t *)propData)[0] = TARGET_DISP_VIDEO_MODE;
	}
#endif //TARGET_DISP_VIDEO_MODE
	propName = "sram-addresses";
	if (FindProperty(summit_node, &propName, &propData, &propSize)) {
		memcpy(propData, summit_sram_address, sizeof(summit_sram_address));
	}

	if (clcd_node != NULL) {
		propName = "display-default-color";
		if (FindProperty(clcd_node, &propName, &propData, &propSize)) {
			((u_int32_t *)propData)[0] = summit_default_color;
		}

		propName = "dot-pitch";
		if (FindProperty(clcd_node, &propName, &propData, &propSize)) {
			((u_int32_t *)propData)[0] = summit_dot_pitch;
		}

#ifdef TARGET_DISP_VIDEO_MODE
		//tell the os if video or command mode
		propName = "video-mode";
		if (FindProperty(clcd_node, &propName, &propData, &propSize)) {
			((u_int32_t *)propData)[0] = TARGET_DISP_VIDEO_MODE;
		}
#endif //TARGET_DISP_VIDEO_MODE
	}

	if (backlight_node != NULL) {
		propName = "function-backlight_enable";
		if (FindProperty(backlight_node, &propName, &propData, &propSize)) {
			memset(propName, 0, strlen(propName));
			strlcpy(propName, PropNameFuncDisable, strlen(PropNameFuncDisable) + 1);
		}
		propName = "function-backlight_enable-sum";
		if (FindProperty(backlight_node, &propName, &propData, &propSize)) {
			memset(propName, 0, strlen(propName));
			strlcpy(propName, PropNameFuncEnable, strlen(PropNameFuncEnable) + 1);
		}
		propName = "function-backlight_update";
		if (FindProperty(backlight_node, &propName, &propData, &propSize)) {
			memset(propName, 0, strlen(propName));
			strlcpy(propName, PropNameFuncNoUpdate, strlen(PropNameFuncNoUpdate) + 1);
		}

		propName = "function-backlight_update-sum";
		if (FindProperty(backlight_node, &propName, &propData, &propSize)) {
			memset(propName, 0, strlen(propName));
			strlcpy(propName, PropNameFuncUpdate, strlen(PropNameFuncUpdate) + 1);
		}
		propName = "display-backlight-calibration";
		if (FindProperty(backlight_node, &propName, &propData, &propSize)) {
			((u_int32_t *)propData)[0] = summit_backlight_cal;
		}
		propName = "backlight-id";
		if (FindProperty(backlight_node, &propName, &propData, &propSize)) {
			if (((u_int32_t *)propData)[0] == 0xffffffff) {
				((u_int32_t *)propData)[0] = (summit_panel_id >> 8) & 0xff;
			}
		}
	}
	return 0;
}

#endif

//FOR DEBUG ONLY WHEN NEEDED
#if 0
static int32_t panelFill(uint32_t width, uint32_t height, uint8_t red, uint8_t green, uint8_t blue )
{
	int32_t		err;
	uint32_t	x, y;
	uint8_t		*ptr;
	uint8_t		*dummyLine;
	uint32_t	lineBytes;

	printf("width %d height %d red 0x%x green 0x%x blue 0x%x\n", width, height, red, green, blue);
	lineBytes = width * 3;
	dummyLine = (uint8_t *) malloc(lineBytes + 1);
	if (dummyLine == NULL )		return -1;

	ptr = &dummyLine[1];
	printf("%s %d\n", __FUNCTION__, __LINE__);

	for (x = 0; x < width; x++) {
		*ptr++ = red;
		*ptr++ = green;
		*ptr++ = blue;
	}
	printf("%s %d\n", __FUNCTION__, __LINE__);
#define	MIPI_DSI_LONG_DCS_WRITE			0x39
#define MIPI_DCS_COMMAND_WRITE_MEMORY_START	0x2C
#define MIPI_DCS_COMMAND_WRITE_MEMORY_CONTINUE	0x3C

	for (y = 0; y < height; y++) {
		if (y == 0) {
			dummyLine[0] = MIPI_DCS_COMMAND_WRITE_MEMORY_START;
			err = mipi_dsim_send_long_command( MIPI_DSI_LONG_DCS_WRITE, dummyLine, lineBytes + 1 );
			if (err)
				goto PANEL_FILL_ERROR;
		} else {
			dummyLine[0] = MIPI_DCS_COMMAND_WRITE_MEMORY_CONTINUE;
			err = mipi_dsim_send_long_command( MIPI_DSI_LONG_DCS_WRITE, dummyLine, lineBytes + 1 );
			if (err)
				goto PANEL_FILL_ERROR;
		}

		spin(1000);
	}

PANEL_FILL_ERROR:
	free( dummyLine);

	return err;
}

#include <sys/menu.h>

static int do_mipi(int argc, struct cmd_arg *args)
{
	if (argc < 3) {
		puts("not enough arguments.\n");
usage:
		printf("%s readshort <cmd>\n", args[0].str);
		printf("%s readlong <cmd> <length>\n", args[0].str);
		printf("%s writeshort <cmd> <data0> <data1>\n", args[0].str);
		printf("%s writelong <cmd> <data0> <data1> ...\n", args[0].str);
		printf("%s fill <r> <g> <b> \n", args[0].str);
		return -1;
	}

	int cmd = args[2].u;

	if (!strcmp("readshort", args[1].str)) {
		uint8_t data[32];
		int err;

		err = mipi_dsim_read_short_command(cmd, data);
		if (err < 0) {
			printf("error %d reading from mipi\n", err);
		} else {
			hexdump(data, sizeof(data));
		}
	} else if (!strcmp("readlong", args[1].str)) {
		uint8_t data[16];
		int err;
		uint32_t len = 32;

		data[0] = args[3].u;
		printf("reading from reg 0x%x length %d\n", data[0], len);
		err = mipi_dsim_read_long_command(cmd, data, &len);
		if (err < 0) {
			printf("error %d reading from mipi\n", err);
		} else {
			hexdump(data, len);
		}
	} else if (!strcmp("writeshort", args[1].str)) {
		u_int8_t data[2];
		int err;

		data[0] = args[3].u;
		data[1] = args[4].u;

		printf("writing cmd 0x%x data0 0x%x data1 0x%x\n", cmd, data[0], data[1]);
		err = mipi_dsim_send_short_command(cmd, data[0], data[1]);
	} else if (!strcmp("writelong", args[1].str)) {
		u_int8_t data[64];
		size_t len, i;
		int err;

		len = argc - 3;
		if (len > sizeof(data)) len = sizeof(data);

		for (i = 0; i < len; i++) {
			data[i] = args[i+3].u;
			printf("data[%d]=0x%x\n", i, data[i]);
		}

		printf("writing %d bytes of data\n", len);
		err = mipi_dsim_send_long_command(cmd, data, len);
		if (err < 0)
			printf("error %d writing to mipi\n", err);
	} else if (!strcmp("fill", args[1].str)) {
		int err;

		printf("filling width %d height %d r 0x%x g 0%x b 0x%x\n",  args[2].u, args[3].u, args[4].u, args[5].u, args[6].u);
		err = panelFill(args[2].u, args[3].u, args[4].u, args[5].u, args[6].u);
		if (err < 0) {
			printf("error %d reading from mipi\n", err);
		}
	} else if (!strcmp("writeshort", args[1].str)) {
	} else {
		puts("unrecognized command.\n");
		goto usage;
	}

	return 0;
}
#if WITH_RECOVERY_MODE
MENU_COMMAND_DEBUG(mipi, do_mipi, "[read/write] mipi commands", NULL);
#endif
#endif
