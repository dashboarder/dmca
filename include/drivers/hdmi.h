/*
 * Copyright (C) 2012 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */

#ifndef DRIVERS_HDMI_H
#define DRIVERS_HDMI_H	1

#include <drivers/displayAV.h>
#include <drivers/display.h>

// Color component limits for limited range color
#define kVideoColorLimitedRangeYMin     16     // Also used for RGB
#define kVideoColorLimitedRangeYMax     235    // Also used for RGB
#define kVideoColorLimitedRangeCMin     16
#define kVideoColorLimitedRangeCMax     240

// Timing Generator Limits (max value + 1)
#define kHorizontalTotalLimit           8192u
#define kHorizontalActiveLimit          4096u
#define kHorizontalBlankLimit           4096u
#define kVerticalTotalLimit             2048u
#define kVerticalActiveLimit            2048u
#define kVerticalBlankLimit             2048u

#define kPHYReadyPollIntervalMS         10
#define kVideoStabilizationIntervalMS   60

// controller config flags

enum {
	kHDMIControllerMode_Master = 0,
	kHDMIControllerMode_Slave
};
#define kHDMIControllerMode_Mask		0xf
#define kHDMIControllerMode_Shift		4

enum {
	kHDMIControllerType_HDMI	= 0,
};
#define kHDMIControllerType_Mask		0xf
#define kHDMIControllerType_Shift		0

enum {
	kHDMI_tx_mode_None = 0,
	kHDMI_tx_mode_HDMI = 1,
	kHDMI_tx_mode_DVI = 2,
} TX_Mode;

struct pixel {
	uint16_t    r;  // R or Cr
	uint16_t    g;  // G or Y
	uint16_t    b;  // B or Cb
};

// global
int hdmi_init();
int hdmi_init_with_timing_info(struct display_timing *timing_info);
int hdmi_set_timings(struct display_timing *timing_info);
int hdmi_start_video(void);
bool hdmi_video_configured();
void hdmi_quiesce();			

// controller
int hdmi_controller_start(u_int8_t type, u_int8_t mode);
void hdmi_controller_stop();
int hdmi_controller_validate_video(struct video_timing_data *data);
int hdmi_controller_read_bytes_i2c(u_int32_t device_addr, u_int8_t addr, u_int8_t *data, u_int32_t length);
int hdmi_controller_write_bytes_i2c(u_int32_t device_addr, u_int32_t addr, u_int8_t *data, u_int32_t length);
int hdmi_controller_start_video(struct video_link_data *data, uint32_t pixel_clock);
int hdmi_controller_stop_video();

uint8_t hdmi_read_reg(uint32_t offset);
void hdmi_write_reg(uint32_t offset, uint8_t value);

bool hdmi_controller_video_configured();

// device

int hdmi_device_start();
int hdmi_device_wait_started();
void hdmi_device_stop();
#endif
