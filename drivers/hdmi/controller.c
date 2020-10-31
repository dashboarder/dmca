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
#include <drivers/display.h>
#include <drivers/hdmi.h>
#include <sys/task.h>

#include <drivers/process_edid.h>

/////////////////////////////////////////
////////// debug support

#define HDMI_DEBUG_MASK (		\
		HDMI_DEBUG_INIT |	        \
		HDMI_DEBUG_ERROR |	\
		HDMI_DEBUG_INFO |	        \
		HDMI_DEBUG_TRAINING |	\
		0)

#undef HDMI_DEBUG_MASK
#define HDMI_DEBUG_MASK		(HDMI_DEBUG_INIT | HDMI_DEBUG_ERROR)

#define HDMI_DEBUG_INIT           (1<<16)  // initialisation
#define HDMI_DEBUG_ERROR          (1<<17)  // errors
#define HDMI_DEBUG_INFO           (1<<18)  // info
#define HDMI_DEBUG_ALWAYS         (1<<31)  // unconditional output

#define debug(_fac, _fmt, _args...)								\
	do {											\
		if ((HDMI_DEBUG_ ## _fac) & (HDMI_DEBUG_MASK | HDMI_DEBUG_ALWAYS))		\
			dprintf(DEBUG_CRITICAL, "HDMIC: %s, %d: " _fmt, __FUNCTION__, __LINE__, ##_args);	\
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

/////////////////////////////////////////
////////// local variables

static struct video_link_data hdmi_video_data;
static uint32_t hdmi_pixel_clock;

/////////////////////////////////////////
////////// local functions declaration


/////////////////////////////////////////
////////// HDMI global functions

int hdmi_init()
{
	// defaults to External display, and Master mode
	if ( hdmi_controller_start(kHDMIControllerType_HDMI, kHDMIControllerMode_Master) != 0 )
		return -1;
        
	return 0;
}

int hdmi_init_with_timing_info(struct display_timing *timing_info)
{
	u_int8_t type;
	u_int8_t mode;
	u_int32_t display_config;

	display_config = *((uint32_t *)(timing_info->display_config));
	type = (display_config >> kHDMIControllerType_Shift) & kHDMIControllerType_Mask;
	mode = (display_config >> kHDMIControllerMode_Shift) & kHDMIControllerMode_Mask;

	hdmi_set_timings(timing_info);
	
        if ( hdmi_controller_start(type, mode) != 0 )
                return -1;
        return 0;
}

int hdmi_set_timings(struct display_timing *timing_info)
{
	u_int8_t mode;
	u_int32_t display_config;
	
	display_config = *((uint32_t *)(timing_info->display_config));
	mode = (display_config >> kHDMIControllerMode_Shift) & kHDMIControllerMode_Mask;
	
        bzero(&hdmi_video_data, sizeof(struct video_link_data));

	hdmi_pixel_clock = timing_info->pixel_clock;

        hdmi_video_data.mirror_mode = (mode == kHDMIControllerMode_Slave) ? true : false;
        hdmi_video_data.test_mode = 0;
        hdmi_video_data.color.depth = 8;
        hdmi_video_data.color.space = kDisplayColorSpacesRGB;
        hdmi_video_data.color.range = kDisplayColorDynamicRangeVESA;
        hdmi_video_data.color.coefficient = kDisplayColorCoefficientITU601;
        hdmi_video_data.timing.axis[0].total = timing_info->h_active + timing_info->h_back_porch + 
                                        timing_info->h_front_porch + timing_info->h_pulse_width;
        hdmi_video_data.timing.axis[0].active = timing_info->h_active;
        hdmi_video_data.timing.axis[0].sync_width = timing_info->h_pulse_width;
        hdmi_video_data.timing.axis[0].back_porch = timing_info->h_back_porch;
        hdmi_video_data.timing.axis[0].front_porch = timing_info->h_front_porch;
        hdmi_video_data.timing.axis[0].sync_rate = 0;
	// Hsync polarity: 0 active high, 1 active low
        hdmi_video_data.timing.axis[0].sync_polarity = timing_info->neg_hsync ? 0 : 1;
        hdmi_video_data.timing.axis[1].total = timing_info->v_active + timing_info->v_back_porch + 
                                timing_info->v_front_porch + timing_info->v_pulse_width;
        hdmi_video_data.timing.axis[1].active = timing_info->v_active;
        hdmi_video_data.timing.axis[1].sync_width = timing_info->v_pulse_width;
        hdmi_video_data.timing.axis[1].back_porch = timing_info->v_back_porch;
        hdmi_video_data.timing.axis[1].front_porch = timing_info->v_front_porch;
        hdmi_video_data.timing.axis[1].sync_rate = (60 << 16);
	// Vsync polarity: 0 active high, 1 active low
        hdmi_video_data.timing.axis[1].sync_polarity = timing_info->neg_vsync ? 0 : 1;

        debug(INFO, "vTotal:%d hTotal:%d\n", hdmi_video_data.timing.axis[1].total, hdmi_video_data.timing.axis[0].total);

	// Restrict EDID choices to timings matching these, otherwise the
	// framebuffer dimensions will mismatch.
	restrict_edid(hdmi_video_data.timing.axis[0].active,
		      hdmi_video_data.timing.axis[1].active);

	return 0;
}

void hdmi_quiesce()
{
        hdmi_controller_stop();
}

int hdmi_start_video(void)
{
	return hdmi_controller_start_video(&hdmi_video_data, hdmi_pixel_clock);
}

bool hdmi_video_configured()
{
	return hdmi_controller_video_configured();
}

/////////////////////////////////////////
////////// controller global functions

/////////////////////////////////////////
////////// controller local functions

