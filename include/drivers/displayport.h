/*
 * Copyright (C) 2010-2014 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */

#ifndef DRIVERS_DISPLAYPORT_H
#define DRIVERS_DISPLAYPORT_H	1

#include <drivers/display.h>
#include <drivers/displayAV.h>

#define kDPMaxLanes 4


enum {
	kLinkRate000Gbps	= 0x0,
	kLinkRate162Gbps	= 0x6,
	kLinkRate270Gbps	= 0xa,
	kLinkRate324Gbps	= 0x0c,
	kLinkRate540Gbps	= 0x14
};

enum {
        kDPVoltageLevel0,
        kDPVoltageLevel1,
        kDPVoltageLevel2,
        kDPVoltageLevel3,
        kDPVoltageLevelMin = kDPVoltageLevel0,
        kDPVoltageLevelMax = kDPVoltageLevel3
};

enum {
        kDPEQLevel0,
        kDPEQLevel1,
        kDPEQLevel2,
        kDPEQLevel3,
        kDPEQLevelMin = kDPEQLevel0,
        kDPEQLevelMax = kDPEQLevel3
};

enum {
        kDPTrainingPatternNone,
        kDPTrainingPattern1,
        kDPTrainingPattern2,
        kDPTrainingPattern3
};

enum {
        kDPLaneStatusFlagClockRecoveryDone      = (1<<0),
        kDPLaneStatusFlagsEQDone                = (1<<1),
        kDPLaneStatusFlagsSymbolLocked          = (1<<2),
        kDPLaneStatusFlagsMask                  = (0x7)
};

enum {
    kDPAlignmentStatusFlagsDone               = (1<<0),
    kDPAlignmentStatusFlagsDownstreamChanged  = (1<<6),
    kDPAlignmentStatusFlagsLinkUpdated        = (1<<7),
};

enum {
    kDPDownstreamTypeDP,
    kDPDownstreamTypeVGA,
    kDPDownstreamTypeDVI,
    kDPDownstreamTypeHDMI,
    kDPDownstreamTypeOther,
    kDPDownstreamTypeCount
};

struct dp_link_training_lane_data {
        u_int32_t       voltage;
        u_int32_t       eq;
};

struct dp_link_train_data {
    bool        enhanced_mode;
    bool	assr;
    bool        downspread;
    bool        fast;
    u_int32_t   lane_count;
    u_int32_t   link_rate;
    struct dp_link_training_lane_data lane[kDPMaxLanes];
};


// controller config flags

enum {
	kDPControllerMode_Master = 0,
	kDPControllerMode_Slave
};
#define kDPControllerMode_Mask		0xf
#define kDPControllerMode_Shift		4

enum {
	kDPControllerType_DP = 0,
	kDPControllerType_EDP
};
#define kDPControllerType_Mask		0xf
#define kDPControllerType_Shift		0

#include <drivers/display.h>

// global
int displayport_init(dp_t *dp);
int displayport_init_with_timing_info(struct display_timing *timing_info);
int displayport_set_timings(struct display_timing *timing_info);
int displayport_start_video(void);
int displayport_enable_alpm(bool enable);
bool displayport_video_configured();
void displayport_quiesce();			

// eDP support
#if WITH_HW_DISPLAY_EDP
#define kEDPRawPanelIdLength	16

int displayport_get_raw_panel_id(u_int8_t *raw_panel_id);
#endif // WITH_HW_DISPLAY_EDP

// controller
int dp_controller_start(dp_t *);
void dp_controller_stop();
int dp_controller_validate_video(struct video_timing_data *data);
int dp_controller_read_bytes_dpcd(u_int32_t addr, u_int8_t *data, u_int32_t length);
int dp_controller_read_bytes_i2c(u_int32_t device_addr, u_int32_t addr, u_int8_t *data, u_int32_t length);
int dp_controller_write_bytes_dpcd(u_int32_t addr, u_int8_t *data, u_int32_t length);
int dp_controller_write_bytes_i2c(u_int32_t device_addr, u_int32_t addr, u_int8_t *data, u_int32_t length);
int dp_controller_train_link(struct dp_link_train_data *data);
int dp_controller_get_adjustment_levels(u_int32_t lane, u_int32_t *voltage_swing, u_int32_t *eq);
int dp_controller_set_adjustment_levels(u_int32_t lane, u_int32_t voltage_swing, u_int32_t eq, 
                                      bool *voltage_max_reached, bool *eq_max_reached);
int dp_controller_set_training_pattern(u_int32_t value, bool scramble);
int dp_controller_get_training_pattern(u_int32_t *value, bool *scramble);
int dp_controller_set_link_rate(u_int32_t link_rate);
int dp_controller_get_link_rate(u_int32_t *link_rate);
int dp_controller_set_lane_count(uint32_t lane_count);
int dp_controller_get_lane_count(uint32_t * lane_count);
int dp_controller_set_enhanced_mode(bool mode);
int dp_controller_get_enhanced_mode(bool * mode);
int dp_controller_set_ASSR(bool mode);
int dp_controller_get_ASSR(bool * mode);
int dp_controller_set_downspread(bool state);
int dp_controller_get_downspread(bool *state);
int dp_controller_start_video(struct video_link_data *data);
int dp_controller_stop_video();
u_int32_t dp_controller_get_max_lane_count();
u_int32_t dp_controller_get_max_link_rate();
u_int32_t dp_controller_get_min_lane_count();
u_int32_t dp_controller_get_min_link_rate();
bool dp_controller_get_supports_downspread();
bool dp_controller_get_supports_fast_link_training();
bool dp_controller_get_supports_alpm();
int dp_controller_enable_alpm(bool enable, struct video_link_data *data);

bool dp_controller_video_configured();
bool dp_controller_wait_for_edp_hpd(utime_t *hpd_time);
void dp_controller_wait_for_HPD_to_BL(utime_t hpd_time);

void dp_controller_wait_phy_locked(void);

// device

int dp_device_start(bool edp_panel);
int dp_device_wait_started(utime_t timeout);
void dp_device_stop();
int dp_device_get_alignment_status_mask(u_int32_t *mask);
int dp_device_get_lane_status_mask(uint32_t lane, uint32_t *mask);
int dp_device_get_training_pattern(uint32_t *value, bool *scramble);
int dp_device_set_training_pattern(uint32_t value, bool scramble);
int dp_device_get_requested_adjustment_levels(uint32_t lane, u_int32_t *voltage, u_int32_t *eq);
int dp_device_set_adjustment_levels(uint32_t lane, u_int32_t voltage_swing, u_int32_t eq, 
                                bool voltage_max_reached, bool eq_max_reached);
int dp_device_get_enhanced_mode(bool * value);
int dp_device_set_enhanced_mode(bool value);
int dp_device_get_ASSR(bool * value);
int dp_device_set_ASSR(bool value);
int dp_device_get_downspread(bool * value);
int dp_device_set_downspread(bool value);
int dp_device_get_lane_count(uint32_t * value);
int dp_device_set_lane_count(uint32_t value);
int dp_device_get_link_rate(u_int32_t * value);
int dp_device_set_link_rate(u_int32_t value);
int dp_device_hdcp_enable(bool enable);
int dp_device_get_downstream_port_type(int *ret_value);
int dp_device_get_raw_panel_id(u_int8_t *raw_panel_id);
int dp_device_get_sink_count(uint8_t * value);

bool dp_device_get_supports_fast_link_training();
bool dp_device_get_supports_alpm();
int dp_device_enable_alpm(bool enable);
bool dp_device_is_alpm_enabled();
bool dp_device_get_supports_enhanced_mode();
bool dp_device_get_supports_assr();
bool dp_device_get_supports_downspread();
u_int32_t dp_device_get_revision();
u_int32_t dp_device_get_max_lane_count();
u_int32_t dp_device_get_max_link_rate();



#endif
