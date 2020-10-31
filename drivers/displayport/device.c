/*
 * Copyright (C) 2010-2015 Apple Inc. All rights reserved.
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
#include "dpcd.h"

#if WITH_HW_MCU
#include <drivers/mcu.h>
#include <drivers/process_edid.h>
#endif


/////////////////////////////////////////
////////// debug support

#define DP_DEBUG_MASK ( 		\
		DP_DEBUG_INIT |	        \
		DP_DEBUG_ERROR |	\
		DP_DEBUG_INFO |         \
		DP_DEBUG_TRAINING |     \
		0)

#undef DP_DEBUG_MASK
#define DP_DEBUG_MASK  		(DP_DEBUG_INIT | DP_DEBUG_ERROR)

#define DP_DEBUG_INIT           (1<<16)  // initialisation
#define DP_DEBUG_INFO           (1<<17)  // info
#define DP_DEBUG_TRAINING       (1<<18)  // link training
#define DP_DEBUG_WAIT           (1<<19)  // start wait
#define DP_DEBUG_ERROR          (1<<20)  // error
#define DP_DEBUG_ALWAYS         (1<<31)  // unconditional output

#define debug(_fac, _fmt, _args...)	        							\
	do {					        						\
		if ((DP_DEBUG_ ## _fac) & (DP_DEBUG_MASK | DP_DEBUG_ALWAYS))    			\
			dprintf(DEBUG_CRITICAL, "DPD: %s, %d: " _fmt, __FUNCTION__, __LINE__, ##_args);	\
	} while(0)


/////////////////////////////////////////
////////// consts

#define kDPDeviceMaxCapabilityBytes 0x0c
#define kDPDeviceMaxDownstreamPorts 16
#define kEDPRawPanelIdReadTimeout (1 * 1000 * 1000)

/////////////////////////////////////////
////////// typedefs, enums, structs

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

/////////////////////////////////////////
////////// local variables

static uint8_t dp_device_downstream_port_capability_bytes[kDPDeviceMaxDownstreamPorts];
static uint8_t dp_device_capability_bytes[kDPDeviceMaxCapabilityBytes];
static u_int32_t dp_device_voltage_adjustment_level[kDPMaxLanes];
static u_int32_t dp_device_eq_adjustment_level[kDPMaxLanes];
static uint8_t dp_device_adjustment_bytes[5];
static uint8_t dp_device_lane_status_mask[kDPMaxLanes];
static bool alpm_enabled;
#if WITH_HW_DISPLAY_EDP
static u_int8_t dp_device_raw_panel_id[kEDPRawPanelIdLength];
#endif // WITH_HW_DISPLAY_EDP

/////////////////////////////////////////
////////// local functions declaration

static int set_power(bool enable);
static int train_link();
static int enable_alpm(bool enable);
static int cache_capabilities();
static int update_lane_status_mask();
static int update_requested_adjustment_levels();
static int read_raw_panel_id();

static struct task_event dp_device_start_event =
	EVENT_STATIC_INIT(dp_device_start_event, false, EVENT_FLAG_AUTO_UNSIGNAL);
static bool dp_device_started;
static bool dp_device_start_error;
static utime_t dp_device_started_time;
static struct task_event dp_device_raw_panel_id_ready_event;

extern utime_t gPowerOnTime;

/////////////////////////////////////////
////////// dp-device global functions

int dp_device_start(bool edp_panel)
{
	int ret = -1;
    
	if ( dp_device_started )
		return 0;

        debug(INIT, "starting\n");
        
	event_init(&dp_device_raw_panel_id_ready_event, EVENT_FLAG_AUTO_UNSIGNAL, false);

	if ( set_power(true) != 0 ) {
		debug(ERROR, "failed to set power\n");
		goto exit;
	}

    if ( cache_capabilities() != 0 ) {
        debug(ERROR, "failed to cache capabilities\n");
		goto exit;
    }

#if WITH_HW_DISPLAY_EDP
	if ( edp_panel ) {
		if ( read_raw_panel_id() != 0 ) {
			debug(ERROR, "failed to read raw panel id\n");
			goto exit;
		} else {
			debug(INFO, "successfully read panel id: ");
			for ( unsigned i=0; i<sizeof(dp_device_raw_panel_id); i++)
				debug(INFO, "%02x ", dp_device_raw_panel_id[i]);

			debug(INFO, "\n");
		}
		event_signal(&dp_device_raw_panel_id_ready_event);
	}
#endif // WITH_HW_DISPLAY_EDP

	if ( train_link() != 0 ) {
		debug(ERROR, "failed to train link\n");
		goto exit;
	}
        
#if WITH_HW_MCU
	uint8_t sink_count;
	dp_device_get_sink_count(&sink_count);
	if (sink_count == 0) {
		debug(ERROR, "sink count is 0\n");
		goto exit;
	}
	// Trained link first, validate timings vs trained bandwidth.
	if ( obtain_edid() == 0 ) {
		// Enable info frames if HDMI endpoint connected, so
		// we get the right color space.
		if (get_edid_downstream_type() == kDPDownstreamTypeHDMI) {
			debug(ALWAYS, "Sending AVI info frames\n");
#if WITH_HW_BS
			(void) mcu_send_info_frames(true);
#endif
		}
	} else {
		// Non-fatal.
		debug(ERROR, "Couldn't get EDID\n");
	}
#endif

	if ( displayport_start_video() != 0 ) {
		debug(ERROR, "failed to start video\n");
		goto exit;
	}

	alpm_enabled = false;

	if (enable_alpm(true) != 0) {
		debug(ERROR, "failed enable alpm\n");
		goto exit;
	}

	if ( dp_device_hdcp_enable(false) != 0 ) {
		debug(ERROR, "failed to Disable HDCP\n");
		goto exit;
	}
	debug(ALWAYS, "HDCP Disabled\n");

	dp_device_started = true;

	ret = 0;

 exit:
	if (ret != 0) {
		dp_device_start_error = true;
		debug(ERROR, "error starting device\n");
	}
    
	dp_device_started_time = system_time();
	event_signal(&dp_device_start_event);
    
	return ret;
}

int dp_device_wait_started(utime_t timeout)
{
	utime_t wait_start = system_time();

	if (dp_device_started || dp_device_start_error) {
		debug(WAIT, "DisplayPort done with %d usecs to spare\n",
		      (int) (wait_start - dp_device_started_time));
		goto exit;
	}

	while (!dp_device_started && !dp_device_start_error) {
		utime_t now = system_time();
		if (now >= timeout) {
			debug(WAIT, "Timeout waiting for displayport start\n");
			return -1;
		}
		event_wait_timeout(&dp_device_start_event, timeout - now);
	}
	debug(WAIT, "Delayed boot by %lld usecs\n", system_time() - wait_start);
 exit:
	debug(WAIT, "Started waiting %lld usecs after power on\n",
	      wait_start - gPowerOnTime);
	return dp_device_start_error ? -1 : 0;
}

void dp_device_stop()
{
        if ( !dp_device_started )
                return;

	set_power(false);
                
        dp_device_started = false;               
	alpm_enabled = false;
}

bool dp_device_is_alpm_enabled()
{
	return alpm_enabled;
}

#if WITH_HW_DISPLAY_EDP
int dp_device_get_raw_panel_id(u_int8_t *raw_panel_id)
{
	if (event_wait_timeout(&dp_device_raw_panel_id_ready_event, kEDPRawPanelIdReadTimeout) == false)
		return -1;
		
	if (raw_panel_id == NULL)
		return -1;
		
	memset(raw_panel_id, 0, kEDPRawPanelIdLength);
	
	memcpy(raw_panel_id, dp_device_raw_panel_id, kEDPRawPanelIdLength);
	
	return 0;
}
#endif // WITH_HW_DISPLAY_EDP

int dp_device_get_alignment_status_mask(u_int32_t *mask)
{
        int ret;
        uint8_t value;
        
        ret = dp_controller_read_bytes_dpcd(DPCD_ADDR_LANE_ALIGN_STATUS_UPDATED, &value, 1);
        
        update_lane_status_mask();
        update_requested_adjustment_levels();
        
        if ( mask )
                *mask = value;
                
        return ret;               
}

int dp_device_get_lane_status_mask(uint32_t lane, uint32_t *mask)
{
        if ( lane > dp_device_get_max_lane_count() )
                return -1;
        
        *mask = dp_device_lane_status_mask[lane];
        
        return 0;
}

int dp_device_get_training_pattern(uint32_t *value, bool *scramble)
{
        int ret = 0;
        uint8_t val;
        
        ret = dp_controller_read_bytes_dpcd(DPCD_ADDR_TRAINING_PATTERN_SET, &val, 1);
        if ( ret == 0 ) {
                if ( value )
                    *value = (u_int32_t)(val & DPCD_ADDR_PATTERN_SET_MASK);
                if ( scramble )
                    *scramble = (val & DPCD_ADDR_TRAINING_PATTERN_SET_SCRMB_DISABLE) == 0;
        }
        
        return ret;
}

int dp_device_set_training_pattern(uint32_t value, bool scramble)
{
        uint8_t reg_val = value & DPCD_ADDR_PATTERN_SET_MASK;
        
        if ( !scramble )
                reg_val |= DPCD_ADDR_TRAINING_PATTERN_SET_SCRMB_DISABLE;
        
        dp_device_adjustment_bytes[0] = reg_val;
        
        return dp_controller_write_bytes_dpcd(DPCD_ADDR_TRAINING_PATTERN_SET, dp_device_adjustment_bytes, 5);
}

int dp_device_get_requested_adjustment_levels(uint32_t lane, u_int32_t *voltage, u_int32_t *eq)
{
        if ( lane > dp_device_get_max_lane_count() )
                return -1;
        
        *voltage   = dp_device_voltage_adjustment_level[lane];
        *eq        = dp_device_eq_adjustment_level[lane];
   
        return 0;   
}

int dp_device_set_adjustment_levels(uint32_t lane, u_int32_t voltage_swing, u_int32_t eq, 
                                bool voltage_max_reached, bool eq_max_reached)
{
        uint8_t value;
        
        if ( lane > dp_device_get_max_lane_count() )
                return -1;
        
        value = (eq << DPCD_ADDR_TRAINNIG_SET_PRE_EMPH_SHIFT) | 
                (voltage_swing << DPCD_ADDR_TRAINNIG_SET_VOL_SWING_SHIFT);
        
        if (voltage_max_reached) {
                value |= DPCD_ADDR_TRAINNIG_SET_VOL_SWING_MAX;
        }
        
        if ( eq_max_reached ) {
                value |= DPCD_ADDR_TRAINNIG_SET_PRE_EMPH_MAX;
        }
        
        dp_device_adjustment_bytes[lane+1] = value;
        
        return 0;
}

int dp_device_get_enhanced_mode(bool * value)
{
        int ret = 0;
        uint8_t val = 0;
        
        ret = dp_controller_read_bytes_dpcd(DPCD_ADDR_LANE_COUNT_SET, &val, 1);
        if ( ret == 0 ) {
                *value = val & DPCD_ADDR_LANE_COUNT_SET_ENHANCED;
        }
        
        return ret;
}

int dp_device_get_sink_count(uint8_t * value)
{
        int ret = 0;
        uint8_t val = 0;
        
        ret = dp_controller_read_bytes_dpcd(DPCD_ADDR_SINK_COUNT, &val, 1);
        if ( ret == 0 ) {
                *value = val;
        }
        
        return ret;
}

int dp_device_set_enhanced_mode(bool value)
{
        uint32_t lane_count = 0;
        
        dp_device_get_lane_count(&lane_count);
        lane_count &= DPCD_ADDR_LANE_COUNT_SET_COUNT_MASK;
        
        if ( value )
                lane_count |= DPCD_ADDR_LANE_COUNT_SET_ENHANCED;
        
        return dp_controller_write_bytes_dpcd(DPCD_ADDR_LANE_COUNT_SET, (uint8_t *)&lane_count, 1);
}

int dp_device_get_ASSR(bool * value)
{
        int ret = 0;
        uint8_t val = 0;
        
        ret = dp_controller_read_bytes_dpcd(DPCD_ADDR_RECEIVER_EDP_CONFIG, &val, 1);
        if ( ret == 0 ) {
                *value = val & DPCD_ADDR_RECEIVER_EDP_CONFIG_ASSR_ENABLE;
        }
        
        return ret;
}

int dp_device_set_ASSR(bool value)
{
	uint8_t val = 0;

        if ( value )
                val |= DPCD_ADDR_RECEIVER_EDP_CONFIG_ASSR_ENABLE;
        
        return dp_controller_write_bytes_dpcd(DPCD_ADDR_RECEIVER_EDP_CONFIG, (uint8_t *)&val, 1);
}

int dp_device_get_downspread(bool * value)
{
    int ret = 0;
    uint8_t val = 0;
    
    ret = dp_controller_read_bytes_dpcd(DPCD_ADDR_DOWNSPREAD_CTRL, &val, 1);
    if ( ret == 0 ) {
        *value = val & DPCD_ADDR_DOWNSPREAD_ENABLE;
    }
    
    return ret;
}

int dp_device_set_downspread(bool value)
{
    uint8_t reg_val = 0;
    
    reg_val = value ? DPCD_ADDR_DOWNSPREAD_ENABLE : 0;
    
    return dp_controller_write_bytes_dpcd(DPCD_ADDR_DOWNSPREAD_CTRL, (uint8_t *)&reg_val, 1);
}

int dp_device_get_lane_count(uint32_t * value)
{
        int ret = 0;
        uint8_t val;
        
        ret = dp_controller_read_bytes_dpcd(DPCD_ADDR_LANE_COUNT_SET, &val, 1);
        if ( ret == 0 ) {
                *value = val & DPCD_ADDR_LANE_COUNT_SET_COUNT_MASK;
        }
        
        return ret;
}

int dp_device_set_lane_count(uint32_t value)
{
        bool enhanced_mode = false;
        
        value &= DPCD_ADDR_LANE_COUNT_SET_COUNT_MASK;
        
        dp_device_get_enhanced_mode(&enhanced_mode);
        if ( enhanced_mode )
                value |= DPCD_ADDR_LANE_COUNT_SET_ENHANCED;
        
        return dp_controller_write_bytes_dpcd(DPCD_ADDR_LANE_COUNT_SET, (uint8_t *)&value, 1);
}

int dp_device_get_link_rate(u_int32_t * value)
{
        return dp_controller_read_bytes_dpcd(DPCD_ADDR_LINK_BW_SET, (uint8_t*)value, 1);
}

int dp_device_set_link_rate(u_int32_t value)
{
        return dp_controller_write_bytes_dpcd(DPCD_ADDR_LINK_BW_SET, (uint8_t *)&value, 1);
}

int dp_device_hdcp_enable(bool enable)
{
#if WITH_HW_HOOVER
        uint8_t value;
        dp_controller_read_bytes_dpcd(DPCD_ADDR_HDMI_DVI_MODE_SELECT, &value, 1);

	if (!enable)  value |= DPCD_HDCP_DISABLE;

        return dp_controller_write_bytes_dpcd(DPCD_ADDR_HDMI_DVI_MODE_SELECT, &value, 1);
#else
	return 0;
#endif
}

int dp_device_get_downstream_port_type(int *ret_value)
{
#if WITH_HW_MCU
	// BlueSteel is reporting DP, but we know for sure it's HDMI.
	if (ret_value) *ret_value = kDPDownstreamTypeHDMI;
	return 0;
#else
	int ret = 0;
	int value = kDPDownstreamTypeOther;
	if (dp_device_get_revision() == 0x11) {
		value = dp_device_downstream_port_capability_bytes[0] & 0x07;
	}

	if (value == kDPDownstreamTypeDP) {
		uint8_t reg_val;
		ret = dp_controller_read_bytes_dpcd(DPCD_ADDR_DOWNSTREAMPORT_PRESENT, &reg_val, 1);
		if (ret != 0) goto exit;

		switch (((reg_val >> 1 ) & 3)) {
		case 0:
			value = kDPDownstreamTypeDP;
			break;
		case 1: 
			value = kDPDownstreamTypeVGA;
			break;
		case 2:
			value = kDPDownstreamTypeHDMI;
			break;
		case 3:
			value = kDPDownstreamTypeOther;
			break;
		}
	}

 exit:
	if (ret_value) *ret_value = value;
	return ret;
#endif
}

int dp_device_enable_alpm(bool enable)
{
	uint8_t alpm_data = enable;
	return dp_controller_write_bytes_dpcd(DPCD_ADDR_ALPM_CTRL, &alpm_data, 1);
}

/////////////////////////////////////////
////////// dp-device local functions

static int set_power(bool state)
{
    int ret;
    u_int8_t byte;
    bool current_state;
    
    ret = dp_controller_read_bytes_dpcd(DPCD_ADDR_SINK_POWER_STATE, &byte, 1);
    debug(INFO, "Read sink power state: ret=%d\n", ret);
    require_noerr(ret, exit);
    
    current_state = (byte == DPCD_ADDR_SINK_POWER_STATE_ON);
    
    require_action(state != current_state, exit, debug(INFO, "power level already = %d\n", state));
    
    debug(INFO, "setting power level = %d\n", state);
    
    byte = state ? DPCD_ADDR_SINK_POWER_STATE_ON : DPCD_ADDR_SINK_POWER_STATE_OFF;
    ret = dp_controller_write_bytes_dpcd(DPCD_ADDR_SINK_POWER_STATE, &byte, 1);

    // see DportV1.1a section 5.2.5
    if ( state )
        task_sleep(20 * 1000);

exit:    
    return ret;
}

static int cache_capabilities()
{
        int ret;
        
        ret = dp_controller_read_bytes_dpcd(DPCD_ADDR_DPCD_REV, dp_device_capability_bytes, 
                        (uint32_t)sizeof(dp_device_capability_bytes));
        if ( ret != 0 ) {
                debug(ERROR, "failed to read capabilities\n");
                return -1;
        }

        ret = dp_controller_read_bytes_dpcd(DPCD_ADDR_DOWNSTREAMPORT_0_CAPS, 
                                dp_device_downstream_port_capability_bytes, 
                        (uint32_t)sizeof(dp_device_downstream_port_capability_bytes));
        if ( ret != 0 ) {
                debug(ERROR, "failed to read downstream ports capabilities\n");
                return -1;
        }
        
        return 0;
}

#if WITH_HW_DISPLAY_EDP
static int read_raw_panel_id()
{
	int ret;
		
	ret = dp_controller_read_bytes_dpcd(DPCP_ADDR_VENDOR_BYTES, dp_device_raw_panel_id, kEDPRawPanelIdLength);
	if (ret != 0) {
		debug(ERROR, "failed to read raw panel id\n");
		return -1;
	}
	
	return 0;
}
#endif // WITH_HW_DISPLAY_EDP

u_int32_t dp_device_get_revision()
{
        return dp_device_capability_bytes[DPCD_ADDR_DPCD_REV];
}

u_int32_t dp_device_get_max_lane_count()
{
        return dp_device_capability_bytes[DPCD_ADDR_MAX_LANE_COUNT] & DPCD_ADDR_LANE_COUNT_SET_COUNT_MASK;
}

u_int32_t dp_device_get_max_link_rate()
{
        return dp_device_capability_bytes[DPCD_ADDR_MAX_LINK_RATE];
}

bool dp_device_get_supports_enhanced_mode()
{
        if ( dp_device_get_revision() < 0x11 )
                return false;
        
        return dp_device_capability_bytes[DPCD_ADDR_MAX_LANE_COUNT] & DPCD_ADDR_LANE_COUNT_SET_ENHANCED;
}

bool dp_device_get_supports_training_pattern3()
{
        if ( dp_device_get_revision() < 0x12 )
                return false;

        return dp_device_capability_bytes[DPCD_ADDR_MAX_LANE_COUNT] & DPCD_ADDR_LANE_COUNT_TPS3_SUPPORTED;
}

bool dp_device_get_supports_fast_link_training()
{
	int ret;
	uint8_t value = 0;

	ret = dp_controller_read_bytes_dpcd(DPCD_ADDR_MAX_DOWNSPREAD, &value, 1);
	require_noerr(ret, exit);

exit:    
#if WITH_HW_HOOVER
	return (false);
#else
	return (value & (1<<6) ? true : false);
#endif
}

bool dp_device_get_supports_alpm()
{
	int ret;
	uint8_t value = 0;

	ret = dp_controller_read_bytes_dpcd(DPCD_ADDR_ALPM_CAP, &value, 1);
	require_noerr(ret, exit);

exit:    
	return (value & 1 ? true : false);
}

bool dp_device_get_supports_assr()
{
	int ret;
	uint8_t value = 0;
	
	ret = dp_controller_read_bytes_dpcd(DPCD_ADDR_ALTERNATE_SCRAMBLE, &value, 1);
	require_noerr(ret, exit);

exit:    
	return (value & DPCD_ADDR_ALTERNATE_SCRAMBLER_RESET_CAP ? true : false);
}

bool dp_device_get_supports_downspread()
{
	int ret;
	uint8_t value = 0;

    if ( dp_device_get_revision() >= 0x11 )
        return true;
    
	ret = dp_controller_read_bytes_dpcd(DPCD_ADDR_MAX_DOWNSPREAD, &value, 1);
	require_noerr(ret, exit);
    
exit:    
	return (value & (1<<0) ? true : false);
}

#define min(a,b) ((a) < (b) ? (a) : (b))
#define max(a,b) ((a) > (b) ? (a) : (b))

static int train_link()
{
        struct dp_link_train_data data;
        int ret;
        uint8_t ignore[8];
        
        bzero(&data, sizeof(struct dp_link_train_data));
        
        // collect data needed for link training
        data.lane_count     = dp_device_get_max_lane_count();
        data.link_rate      = dp_device_get_max_link_rate();
        data.enhanced_mode  = dp_device_get_supports_enhanced_mode();
	data.assr	    = dp_device_get_supports_assr();
        data.fast           = dp_device_get_supports_fast_link_training() && dp_controller_get_supports_fast_link_training();
        data.downspread     = min(dp_device_get_supports_downspread(),dp_controller_get_supports_downspread());

        debug(INIT, "lane_count: %d, link_rate: %d, enhanced_mode: %d, assr: %d, fast: %d downspread: %d\n", 
                data.lane_count, data.link_rate, data.enhanced_mode, data.assr, data.fast, data.downspread);

        if ( !data.lane_count || !data.link_rate ) {
                return -1;
        }

        ret = dp_controller_train_link(&data);
        if ( ret != 0 )
                return -1;

        // clear error bits
        dp_controller_read_bytes_dpcd(DPCD_ADDR_SYMBOL_ERROR_COUNT_LANE0_BYTE0, ignore, sizeof(ignore));
                                
        return 0;                                
}

static int enable_alpm(bool enable)
{
	bool supports_alpm;
	int ret = 0;

	supports_alpm = dp_device_get_supports_alpm() && dp_controller_get_supports_alpm(); 

	printf("supports_alpm %d\n", supports_alpm);
	if (supports_alpm) {
		ret = dp_device_enable_alpm(enable);
		if (ret != 0) {
			debug(ERROR, "Failed to %s device alpm\n", enable ? "enable" : "disable");
			return ret;
		}

		ret = displayport_enable_alpm(enable);
		if (ret != 0) {
			debug(ERROR, "Failed to %s controller alpm\n", enable ? "enable" : "disable");
		}
		alpm_enabled =true;
	} 


        return ret;                                
}

static int update_lane_status_mask()
{
        int ret = 0;
        uint8_t value, lane;
        
        for (lane=0; lane<dp_device_get_max_lane_count(); lane++) {
        
                if ( !(lane % 2) )
                        ret = dp_controller_read_bytes_dpcd(DPCD_ADDR_LANE0_1_STATUS+(lane>>1), &value, 1);
                
                if ( ret != 0 )
                        break;
                
                if ( lane % 2 )
                        dp_device_lane_status_mask[lane] = (value >> DPCD_ADDR_LANEX_Y_STATUS_Y_SHIFT) & 
                                                        kDPLaneStatusFlagsMask;
                else
                        dp_device_lane_status_mask[lane] = (value >> DPCD_ADDR_LANEX_Y_STATUS_X_SHIFT) & 
                                                        kDPLaneStatusFlagsMask;
        }
        
        return ret;
}

static int update_requested_adjustment_levels()
{
        int ret;
        uint8_t value, lane;
        
        ret = 0;
        
        for (lane=0; lane<dp_device_get_max_lane_count(); lane++) {
        
                if ( !(lane % 2) )
                        ret = dp_controller_read_bytes_dpcd(DPCD_ADDR_ADJUST_REQUEST_LANE0_1+(lane>>1), &value, 1);
                
                if ( ret != 0 )
                        break;
                
                if ( lane % 2 ) {
                        dp_device_eq_adjustment_level[lane] = ((value & DPCD_ADDR_ADJUST_REQUEST_LANEX_Y_PRE_EMP_Y_MASK) >> DPCD_ADDR_ADJUST_REQUEST_LANEX_Y_PRE_EMP_Y_SHIFT);
                        dp_device_voltage_adjustment_level[lane] = ((value & DPCD_ADDR_ADJUST_REQUEST_LANEX_Y_VOL_SWG_Y_MASK) >> DPCD_ADDR_ADJUST_REQUEST_LANEX_Y_VOL_SWG_Y_SHIFT);
                }
                else {
                        dp_device_eq_adjustment_level[lane] = ((value & DPCD_ADDR_ADJUST_REQUEST_LANEX_Y_PRE_EMP_X_MASK) >> DPCD_ADDR_ADJUST_REQUEST_LANEX_Y_PRE_EMP_X_SHIFT);
                        dp_device_voltage_adjustment_level[lane] = ((value & DPCD_ADDR_ADJUST_REQUEST_LANEX_Y_VOL_SWG_X_MASK) >> DPCD_ADDR_ADJUST_REQUEST_LANEX_Y_VOL_SWG_X_SHIFT);
                }         
        }
        
        return ret;
}
