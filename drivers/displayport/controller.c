/*
 * Copyright (C) 2010, 2015 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */


#include <debug.h>
#include <drivers/displayport/displayport.h>
#include <drivers/displayport.h>
#include <sys/task.h>

#if WITH_HW_MCU
#include <drivers/process_edid.h>
#endif

#include "dpcd.h"

/////////////////////////////////////////
////////// debug support

#define DP_DEBUG_MASK ( 		\
		DP_DEBUG_INIT |	        \
		DP_DEBUG_ERROR |	\
		DP_DEBUG_INFO |	        \
		DP_DEBUG_TRAINING |	\
		0)

#undef DP_DEBUG_MASK
#define DP_DEBUG_MASK 		(DP_DEBUG_INIT | DP_DEBUG_ERROR)

#define DP_DEBUG_INIT           (1<<16)  // initialisation
#define DP_DEBUG_ERROR          (1<<17)  // errors
#define DP_DEBUG_INFO           (1<<18)  // info
#define DP_DEBUG_TRAINING       (1<<19)  // link training
#define DP_DEBUG_ALWAYS         (1<<31)  // unconditional output

#define debug(_fac, _fmt, _args...)	        							\
	do {					        						\
		if ((DP_DEBUG_ ## _fac) & (DP_DEBUG_MASK | DP_DEBUG_ALWAYS))    			\
			dprintf(DEBUG_CRITICAL, "DPC: %s, %d: " _fmt, __FUNCTION__, __LINE__, ##_args);	\
	} while(0)


/////////////////////////////////////////
////////// consts, macros

#define kMaxRetry			5
#define kClockRecoveryDelay		100
#define kEQDelay			400
#define kMaxClockRecoveryIterations	100


#define require(assertion, exception_label) \
        do {                                            \
                if (__builtin_expect(!(assertion), 0))  \
                {                                       \
                        goto exception_label;           \
                }                                       \
        } while (0)
        
#define require_action(assertion, exception_label, action) \
        do {                                            \
                if (__builtin_expect(!(assertion), 0))  \
                {                                       \
                        {                               \
                                action;                 \
                        }                               \
                        goto exception_label;           \
                }                                       \
        } while (0)
        
#define require_noerr(error_code, exception_label) \
        do {                                                    \
                if (__builtin_expect(0 != (error_code), 0))     \
                {                                               \
                        goto exception_label;                   \
                }                                               \
        } while (0)
        
#define require_noerr_action(error_code, exception_label, action) \
        do {                                                    \
                if (__builtin_expect(0 != (error_code), 0))     \
                {                                               \
                        {                                       \
                                action;                         \
                        }                                       \
                        goto exception_label;                   \
                }                                               \
        } while (0)



/////////////////////////////////////////
////////// typedefs, enums, structs

enum {
    kLinkTrainingStateIdle,
    kLinkTrainingStateStart,
    kLinkTrainingStateClockRecovery,
    kLinkTrainingStateEQTraining
};


/////////////////////////////////////////
////////// local variables

static u_int8_t dp_link_clock_recovery_retry[kDPMaxLanes];
static bool dp_link_voltage_swing_max[kDPMaxLanes];
static bool dp_link_eq_max[kDPMaxLanes];
static u_int8_t dp_link_eq_retry;
static u_int32_t dp_link_clock_recovery_iterations;
static u_int8_t dp_link_eq_iterations;
static struct video_link_data dp_video_data;
static int eq_pattern = kDPTrainingPattern2;
static dp_t dp_link_config = {
	.mode		= kDPControllerMode_Master,
	.type		= kDPControllerType_DP,
	.min_link_rate	= 0x6,
	.max_link_rate	= 0x6,
	.lanes		= 4,
	.ssc		= 0,
	.alpm		= 0,
	.vrr_enable	= 0,
	.vrr_on		= 0,
};

/////////////////////////////////////////
////////// local functions declaration

static int run_link_training_state_machine(struct dp_link_train_data *data);
static int process_link_state_start(struct dp_link_train_data *data, u_int32_t *state);
static int process_link_state_clock_recovery(struct dp_link_train_data *data, u_int32_t *state);
static int process_link_state_eq_training(struct dp_link_train_data *data, u_int32_t *state);
static int process_link_state_eq_training_internal(struct dp_link_train_data *data, uint32_t lane, bool *done);
static int process_link_state_clock_recovery_internal(struct dp_link_train_data * data, uint32_t lane, bool *p_done);



/////////////////////////////////////////
////////// DisplayPort global functions

int displayport_init(dp_t *dp)
{
	// defaults to External display, and Master mode
        if ( dp_controller_start(dp) != 0 )
                return -1;
        
        return 0;
}

int displayport_init_with_timing_info(struct display_timing *timing_info)
{
	memcpy(&dp_link_config, timing_info->display_config, sizeof(dp_t));

	displayport_set_timings(timing_info);
	
        if ( dp_controller_start(&dp_link_config) != 0 )
                return -1;
        
        return 0;
}

int displayport_set_timings(struct display_timing *timing_info)
{
	u_int8_t mode;
	
	memcpy(&dp_link_config,  timing_info->display_config, sizeof(dp_t));
	mode = dp_link_config.mode;
	
	bzero(&dp_video_data, sizeof(struct video_link_data));

	dp_video_data.mirror_mode = (mode == kDPControllerMode_Slave) ? true : false;
	dp_video_data.test_mode = 0;
	dp_video_data.color.depth = 8;
	dp_video_data.color.space = kDisplayColorSpacesRGB;
	dp_video_data.color.range = kDisplayColorDynamicRangeVESA;
	dp_video_data.color.coefficient = kDisplayColorCoefficientITU601;
	dp_video_data.timing.axis[0].total = timing_info->h_active + timing_info->h_back_porch + 
                                        timing_info->h_front_porch + timing_info->h_pulse_width;
	dp_video_data.timing.axis[0].active = timing_info->h_active;
	dp_video_data.timing.axis[0].sync_width = timing_info->h_pulse_width;
	dp_video_data.timing.axis[0].back_porch = timing_info->h_back_porch;
	dp_video_data.timing.axis[0].front_porch = timing_info->h_front_porch;
	dp_video_data.timing.axis[0].sync_rate = 0;
	// Hsync polarity: 0->negative, 1->positive.
	dp_video_data.timing.axis[0].sync_polarity = timing_info->neg_hsync ? 0 : 1;
	dp_video_data.timing.axis[1].total = timing_info->v_active + timing_info->v_back_porch + 
                                timing_info->v_front_porch + timing_info->v_pulse_width;
	dp_video_data.timing.axis[1].active = timing_info->v_active;
	dp_video_data.timing.axis[1].sync_width = timing_info->v_pulse_width;
	dp_video_data.timing.axis[1].back_porch = timing_info->v_back_porch;
	dp_video_data.timing.axis[1].front_porch = timing_info->v_front_porch;
	dp_video_data.timing.axis[1].sync_rate = (60 << 16);
	// Vsync polarity: 0->negative, 1->positive.
	dp_video_data.timing.axis[1].sync_polarity = timing_info->neg_vsync ? 0 : 1;

	debug(INFO, "vTotal:%d hTotal:%d\n", dp_video_data.timing.axis[1].total, dp_video_data.timing.axis[0].total);

#if WITH_HW_MCU
	// Restrict EDID choices to timings matching these, otherwise the
	// framebuffer dimensions will mismatch.
	restrict_edid(dp_video_data.timing.axis[0].active,
		      dp_video_data.timing.axis[1].active);
#endif

	return 0;
}

void displayport_quiesce()
{
        dp_controller_stop();
}

int displayport_start_video(void)
{
#if WITH_HW_MCU && WITH_HW_HOOVER 
	int downstream_type = get_edid_downstream_type();
	if (downstream_type == kDPDownstreamTypeDVI) {
		uint8_t data;
		dp_controller_read_bytes_dpcd(DPCD_ADDR_HDMI_DVI_MODE_SELECT, &data, sizeof(data));
		data |= DPCD_ADDR_HDMI_DVI_MODE_SELECT_DVI;
		debug(INFO, "Reprogramming Hoover to be in DVI mode: data 0x%x\n", data);
		dp_controller_write_bytes_dpcd(DPCD_ADDR_HDMI_DVI_MODE_SELECT, &data, sizeof(data));
	}
#endif
	return dp_controller_start_video(&dp_video_data);
}

int displayport_enable_alpm(bool enable)
{
	return dp_controller_enable_alpm(enable, &dp_video_data);
}

#if WITH_HW_DISPLAY_EDP
int displayport_get_raw_panel_id(u_int8_t *raw_panel_id)
{
	//caller is responsible for waiting for HPD
	return dp_device_get_raw_panel_id(raw_panel_id);
}
#endif // WITH_HW_DISPLAY_EDP

bool displayport_video_configured()
{
	return dp_controller_video_configured();
}

/////////////////////////////////////////
////////// controller global functions

int dp_controller_train_link(struct dp_link_train_data *data)
{
    int ret;
    u_int32_t lc;
    u_int32_t lr;
    u_int32_t retry = 0;
    
    ret = -1;
    
    lc  = data->lane_count;
    lr  = data->link_rate;

    do {
        
        data->lane_count    = __min((dp_controller_get_max_lane_count()), lc);
        data->link_rate     = __min((dp_controller_get_max_link_rate()), lr);

        do {    
            // check if the device has sufficient bandwidth
            if ( data->lane_count < dp_controller_get_min_lane_count() ) {
                ret = -1;
                break;
            }
            
            ret = run_link_training_state_machine(data);
            
            if ( ret != 0 ) {
                if ( !data->fast ) {
                    data->link_rate  = __min((dp_controller_get_max_link_rate()), lr);
                    data->lane_count = data->lane_count/2;
                } else {
                    data->fast = false;
                }
            }
        } while ( ret != 0 );
        
     } while ( ret != 0 && ++retry < 5 );
        
	return 0;
}


/////////////////////////////////////////
////////// controller local functions

static int run_link_training_state_machine(struct dp_link_train_data *data)
{
    u_int32_t   state = kLinkTrainingStateStart;
    int         ret;

    debug(TRAINING, "Preparing to establish new link\n");

    // establish link training
    do {        
        switch(state) {
            case kLinkTrainingStateStart:
                ret = process_link_state_start(data, &state);
                break;
            case kLinkTrainingStateClockRecovery:
                ret = process_link_state_clock_recovery(data, &state);
                break;
            case kLinkTrainingStateEQTraining:
                ret = process_link_state_eq_training(data, &state);
                break;
            default:
                break;
        }
        
        debug(TRAINING, "Performing Link training state=%d result=%d\n", state, ret);
        
        if ( ret != 0 )
            state = kLinkTrainingStateIdle;
        
    } while ( state != kLinkTrainingStateIdle );

    debug(TRAINING, "Link training result=%d\n", ret);

    return ret;
}

static int process_link_state_start(struct dp_link_train_data *data, u_int32_t *state)
{
    int ret;
    u_int32_t index;
#if DISPLAYPORT_VERSION > 2
	eq_pattern = dp_device_get_supports_training_pattern3() ? kDPTrainingPattern3 : kDPTrainingPattern2;
#endif
    
    ret = -1;
    
    debug(TRAINING, "Begining START phase of link traing. Initial lanecount=%d linkRate=%u bps\n", 
                    data->lane_count, data->link_rate);
    
    bzero(dp_link_clock_recovery_retry, sizeof(dp_link_clock_recovery_retry));
    bzero(dp_link_voltage_swing_max, sizeof(dp_link_voltage_swing_max));
    bzero(dp_link_eq_max, sizeof(dp_link_eq_max));
    dp_link_eq_retry = 0;
    dp_link_clock_recovery_iterations = 0;
    dp_link_eq_iterations = 0;
    
    // Set link rate and count as you want to establish
    // set device
    ret = dp_device_set_enhanced_mode(data->enhanced_mode);
    require_noerr_action(ret, exit, debug(ERROR, "Failed to set device enhanced mode\n"));

    ret = dp_device_set_ASSR(data->assr);
    require_noerr_action(ret, exit, debug(ERROR, "Failed to set device assr\n"));
    
    ret = dp_device_set_downspread(data->downspread);
    require_noerr_action(ret, exit, debug(ERROR, "Failed to set device downspread\n"));

    ret = dp_device_set_link_rate(data->link_rate);
    require_noerr_action(ret, exit, debug(ERROR, "Failed to set device link rate\n"));
    
    ret = dp_device_set_lane_count(data->lane_count);
    require_noerr_action(ret, exit, debug(ERROR, "Failed to set device lane count\n"));

    // set controller
    ret = dp_controller_set_link_rate(data->link_rate);
    require_noerr_action(ret, exit, debug(ERROR, "Failed to set controller link rate\n"));
    
    ret = dp_controller_set_enhanced_mode(data->enhanced_mode);
    require_noerr_action(ret, exit, debug(ERROR, "Failed to set controller enhanced mode\n"));

    ret = dp_controller_set_ASSR(data->assr);
    require_noerr_action(ret, exit, debug(ERROR, "Failed to set controller assr\n"));
    
    ret = dp_controller_set_downspread(data->downspread);
    require_noerr_action(ret, exit, debug(ERROR, "Failed to set controller downspread\n"));

    ret = dp_controller_set_lane_count(data->lane_count);
    require_noerr_action(ret, exit, debug(ERROR, "Failed to set controller lane count\n"));
    
    for ( index=0; index<data->lane_count; index++) {
        ret = dp_controller_set_adjustment_levels(index, kDPVoltageLevelMin, kDPEQLevelMin, NULL, NULL);
        if ( ret != 0 ) {
            debug(ERROR, "Failed to set controller adjustment\n");
            goto exit;
        }

        ret = dp_device_set_adjustment_levels(index, kDPVoltageLevelMin, kDPEQLevelMin, false, false);
        if ( ret != 0 ) {
            debug(ERROR, "Failed to set device adjustment\n");
            goto exit;
        }
    }
    require_noerr_action(ret, exit, debug(ERROR, "Failed to adjustment levels\n"));
    
    ret = dp_controller_set_training_pattern(kDPTrainingPattern1, false);
    require_noerr_action(ret, exit, debug(ERROR, "Failed to set controller training pattern\n"));
    
    ret = dp_device_set_training_pattern(kDPTrainingPattern1, false);
    require_noerr_action(ret, exit, debug(ERROR, "Failed to set device training pattern\n"));

    require(!data->fast, train_fast);
    
    *state = kLinkTrainingStateClockRecovery;
    
    return ret;

train_fast:
    debug(TRAINING, "Skipping to fast link training\n");

    task_sleep(1 * 1000);

    ret = dp_controller_set_training_pattern(eq_pattern, false);
    require_noerr_action(ret, exit, debug(ERROR, "Failed to set controller training pattern\n"));

    ret = dp_device_set_training_pattern(eq_pattern, false);
    require_noerr_action(ret, exit, debug(ERROR, "Failed to set device training pattern\n"));

    task_sleep(1 * 1000);

    ret = dp_controller_set_training_pattern(kDPTrainingPatternNone, true);
    require_noerr_action(ret, exit, debug(ERROR, "Failed to set controller training pattern\n"));

    ret = dp_device_set_training_pattern(kDPTrainingPatternNone, true);
    require_noerr_action(ret, exit, debug(ERROR, "Failed to set device training pattern\n"));
    
exit:
    *state = kLinkTrainingStateIdle;
    return ret;
}

static int process_link_state_clock_recovery(struct dp_link_train_data *data, u_int32_t *state)
{
        uint32_t lane;
        uint32_t alignment_status;
        bool done = true;
        bool succeed = true;
        int ret = 0;    
        
	spin(kClockRecoveryDelay);
	
        ret = dp_device_get_alignment_status_mask(&alignment_status);
        require_noerr(ret, exit);
        
        for ( lane=0; lane<data->lane_count; lane++ ) {
                ret = process_link_state_clock_recovery_internal(data, lane, &done);
                succeed &= ( ret == 0 );
        }
        
        if ( !done ) {
                ret = dp_device_set_training_pattern(kDPTrainingPattern1, false);
                require_noerr(ret, exit);
        }
        
        if ( !succeed ) {
                debug(TRAINING, "loop failed: linkRate=%u bps\n", data->link_rate);
                // traing pattern : Set to Normal
                ret = dp_controller_set_training_pattern(kDPTrainingPatternNone, true); 
                require_noerr(ret, exit);
                ret = dp_device_set_training_pattern(kDPTrainingPatternNone, true);
                require_noerr(ret, exit);
                
                // reduce bit rate
                if ( (data->link_rate > dp_controller_get_min_link_rate()) && (data->link_rate == kLinkRate270Gbps) ) {
                        data->link_rate = kLinkRate162Gbps;
                        debug(TRAINING, "retry @ linkRate=%d\n", data->link_rate);
                        *state = kLinkTrainingStateStart;
                }
                // already in reduced bit-rate
                else {
                        ret = -1;
                }
        } 
        else if ( done ) {
                // set training pattern 2 for EQ
                ret = dp_controller_set_training_pattern(eq_pattern, false);
                require_noerr(ret, exit);
                
                // set the training pattern
                ret = dp_device_set_training_pattern(eq_pattern, false);
                require_noerr(ret, exit);
                
                *state = kLinkTrainingStateEQTraining;
        }
        
exit:	
	if (++dp_link_clock_recovery_iterations > kMaxClockRecoveryIterations) {
		debug(TRAINING, "Max clock recovery iterations exceeded\n");
		ret = -1;
	}
        
        return ret;
}

static int process_link_state_clock_recovery_internal(struct dp_link_train_data * data, uint32_t lane, 
                                                bool *p_done)
{
        int ret = 0;
        bool done = false;
        uint32_t lane_status = 0;
        uint32_t eq = kDPEQLevelMin;
        uint32_t voltage_swing = kDPVoltageLevelMin;
        uint32_t phy_eq;
        uint32_t phy_voltage_swing;
        
        // might as well read the values for lane 
        ret = dp_device_get_lane_status_mask(lane, &lane_status);
        require_noerr(ret, exit);
        
        debug(TRAINING, "Reading lane status: lane=%d status=0x%08x\n",lane, lane_status);
        
        require_action(!(lane_status & kDPLaneStatusFlagClockRecoveryDone), exit, done = true);
        
        ret = dp_device_get_requested_adjustment_levels(lane, &voltage_swing, &eq);
        require_noerr(ret, exit);
        ret = dp_controller_get_adjustment_levels(lane, &phy_voltage_swing, &phy_eq);
        require_noerr(ret, exit);
        
        debug(TRAINING, "Reading lane adjust request: Lane%d: voltage=%d eq=%d\n",lane, voltage_swing, eq);
        debug(TRAINING, "Current phy lane properties: Lane%d: voltage=%d eq=%d\n",lane, phy_voltage_swing, phy_eq);
        
        data->lane[lane].voltage    = voltage_swing;
        data->lane[lane].eq         = eq;
        
        // lane same voltage count
        if (phy_voltage_swing == voltage_swing)
                dp_link_clock_recovery_retry[lane]++;
        else 
                dp_link_clock_recovery_retry[lane] = 0;
        
        debug(TRAINING, "_linkClockRecoveryRetry[%d]=%d\n", lane, dp_link_clock_recovery_retry[lane]);
        
        require_action((dp_link_clock_recovery_retry[lane] < kMaxRetry) && !dp_link_voltage_swing_max[lane],
                        exit, ret=-1);
        
        // increase voltage swing as requested,write an updated value
        debug(TRAINING, "Setting lane adjust request: Lane%d: voltage=%d eq=%d\n",lane, voltage_swing, eq);
        // set voltage drive ONLY on the PHY
        ret = dp_controller_set_adjustment_levels(lane, voltage_swing, eq, &dp_link_voltage_swing_max[lane],
                                        &dp_link_eq_max[lane]);
        require_noerr(ret, exit);
        
        // set voltage drive and EQ on the SINK
        ret = dp_device_set_adjustment_levels(lane, voltage_swing, eq, dp_link_voltage_swing_max[lane], 
                                        dp_link_eq_max[lane]);    
        
exit:
        *p_done &= done;
        
        return ret;
}

static int process_link_state_eq_training(struct dp_link_train_data *data, u_int32_t *state)
{
        uint32_t lane;
        uint32_t alignment_status;
        bool done = true;
        bool succeed = true;
        bool abort = false;
        bool clear = false;
        bool reduce = false;
        bool restart = false;
        int ret = 0;    
        
        debug(TRAINING, "Begining EQ phase of link traing\n");
        
        spin(kEQDelay);
        
        dp_link_eq_retry++;
        
        ret = dp_device_get_alignment_status_mask(&alignment_status);
        require_noerr(ret, exit);
        
        debug(TRAINING, "alignment_status=0x%08x\n", alignment_status);

        // process the eq state for each lane
        for ( lane=0; lane<data->lane_count; lane++ ) {        
                ret = process_link_state_eq_training_internal(data, lane, &done);
                
                if ( ret == 0 )
                        continue;
                
                abort = (ret == -1);
                succeed = false;
                break;
        }
        
        require(!abort, exit);
        
        done &= (alignment_status & kDPAlignmentStatusFlagsDone) != 0;

        // reapply the training pattern
        // this does a bulk write
        if ( !done ) {
                ret = dp_device_set_training_pattern(eq_pattern, false);
                require_noerr(ret, exit);
        }
        
        if ( done ) {
                debug(TRAINING, "Link trainking success @ %d lanes and %u Gbps\n", data->lane_count, 
                                                                                data->link_rate);
                clear = true;
                *state = kLinkTrainingStateIdle;
        } else if ( !abort && (!succeed || dp_link_eq_retry > kMaxRetry) ) {
                reduce = true;
        }
        
        if ( reduce ) {
                debug(TRAINING, "loop failed: laneCount=%d linkRate=%u Gbps\n", data->lane_count, data->link_rate);
                clear = true;
                // reduce bit rate and restart
                if ( (data->link_rate > dp_controller_get_min_link_rate()) && (data->link_rate == kLinkRate270Gbps) ) {
                        data->link_rate = kLinkRate162Gbps;
                        debug(TRAINING, "retry @ linkRate=%d\n", data->link_rate);
                        restart = true;
                }
                // already in reduced bit-rate
                else {
                        ret = -1;
                }
        }
        
        if ( restart ) {
                *state = kLinkTrainingStateStart;
        }
        
        if ( clear ) {
                // traing pattern : Set to Normal
                dp_controller_set_training_pattern(kDPTrainingPatternNone, true); 
                dp_device_set_training_pattern(kDPTrainingPatternNone, true);
        }
        
exit:	
        return ret;
}

static int process_link_state_eq_training_internal(struct dp_link_train_data *data, uint32_t lane, bool *p_done)
{
        int ret = 0;
        bool done = false;
        uint32_t lane_status = 0;
        uint32_t eq;
        uint32_t voltage_swing;
        uint32_t phy_eq;
        uint32_t phy_voltage_swing;
        
        // might as well read the values for lane
        ret = dp_device_get_lane_status_mask(lane, &lane_status);
        require_noerr(ret, exit);

        debug(TRAINING, "Reading lane status: lane=%d lane_status=0x%08x\n",lane, lane_status);

        require_action(lane_status & kDPLaneStatusFlagClockRecoveryDone, exit, ret = -1);

        done = (lane_status & (kDPLaneStatusFlagsEQDone|kDPLaneStatusFlagsSymbolLocked))==(kDPLaneStatusFlagsEQDone|kDPLaneStatusFlagsSymbolLocked);
        require(!done, exit);

        ret = dp_device_get_requested_adjustment_levels(lane, &voltage_swing, &eq);
        require_noerr(ret, exit);
        
        ret = dp_controller_get_adjustment_levels(lane, &phy_voltage_swing, &phy_eq);
        require_noerr(ret, exit);
        
        debug(TRAINING, "Reading lane adjust request: Lane%d: voltage=%d eq=%d\n",lane, voltage_swing, eq);
        debug(TRAINING, "Current phy lane properties: Lane%d: voltage=%d eq=%d\n",lane, phy_voltage_swing, phy_eq);
        
        data->lane[lane].voltage    = voltage_swing;
        data->lane[lane].eq         = eq;
        
        // set voltage drive and EQ on the PHY
        ret = dp_controller_set_adjustment_levels(lane, voltage_swing, eq, 
                                        &dp_link_voltage_swing_max[lane], &dp_link_eq_max[lane]);
        require_noerr(ret, exit);
        
        // set voltage drive and EQ on the SINK
        ret = dp_device_set_adjustment_levels(lane, voltage_swing, eq, 
                                        dp_link_voltage_swing_max[lane], dp_link_eq_max[lane]);
        
exit:
        *p_done &= done;
        
        return ret;
}
