/*
 * Copyright (C) 2012-2014 Apple Inc. All rights reserved.
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
		HDMI_DEBUG_INFO |         \
		HDMI_DEBUG_TRAINING |     \
		0)

#undef HDMI_DEBUG_MASK
#define HDMI_DEBUG_MASK		(HDMI_DEBUG_INIT | HDMI_DEBUG_ERROR)

#define HDMI_DEBUG_INIT           (1<<16)  // initialisation
#define HDMI_DEBUG_INFO           (1<<17)  // info
#define HDMI_DEBUG_TRAINING       (1<<18)  // link training
#define HDMI_DEBUG_WAIT           (1<<19)  // start wait
#define HDMI_DEBUG_ERROR          (1<<20)  // error
#define HDMI_DEBUG_ALWAYS         (1<<31)  // unconditional output

#define debug(_fac, _fmt, _args...)								\
	do {											\
		if ((HDMI_DEBUG_ ## _fac) & (HDMI_DEBUG_MASK | HDMI_DEBUG_ALWAYS))		\
			dprintf(DEBUG_CRITICAL, "DPD: %s, %d: " _fmt, __FUNCTION__, __LINE__, ##_args);	\
	} while(0)


/////////////////////////////////////////
////////// consts

// Do not delay iBoot by more than this amount from power on (microseconds).
// Give up waiting for a boot logo and carry on at this point.
#define kHDMIDeviceStartTimeout (8 * 1000 * 1000)

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


/////////////////////////////////////////
////////// local functions declaration

static int set_power(bool enable);

static struct task_event hdmi_device_start_event =
	EVENT_STATIC_INIT(hdmi_device_start_event, false, 0);
static bool hdmi_device_started;
static bool hdmi_device_start_error;
static utime_t hdmi_device_started_time;

extern utime_t gPowerOnTime;

/////////////////////////////////////////
////////// hdmi-device global functions

int hdmi_device_start()
{
	int ret = -1;
    
    if ( hdmi_device_started )
        return 0;

        
        debug(INIT, "starting\n");

	if ( set_power(true) != 0 ) {
		debug(ERROR, "failed to set power\n");
		goto exit;
	}

	if ( obtain_edid() == 0 ) {
		// Enable info frames if HDMI endpoint connected, so
		// we get the right color space.
	} else {
		// Non-fatal.
		debug(ERROR, "Couldn't get EDID\n");
	}

	if ( hdmi_start_video() != 0 ) {
		debug(ERROR, "failed to start video\n");
		goto exit;
	}

	hdmi_device_started = true;

	ret = 0;

exit:
	if (ret != 0) {
		hdmi_device_start_error = true;
		debug(ERROR, "error starting device\n");
	}
    
	hdmi_device_started_time = system_time();
	event_signal(&hdmi_device_start_event);
    
	return ret;
}

int hdmi_device_wait_started()
{
	utime_t wait_start = system_time();
	utime_t timeout = gPowerOnTime + kHDMIDeviceStartTimeout;

	if (hdmi_device_started || hdmi_device_start_error) {
		debug(WAIT, "HDMI done with %d usecs to spare\n",
		      (int) (wait_start - hdmi_device_started_time));
		goto exit;
	}

	while (!hdmi_device_started && !hdmi_device_start_error) {
		utime_t now = system_time();
		if (now >= timeout) {
			debug(WAIT, "Timeout waiting for HDMI start\n");
			return -1;
		}
		event_wait_timeout(&hdmi_device_start_event, timeout - now);
	}
	debug(WAIT, "Delayed boot by %llu usecs\n", system_time() - wait_start);
 exit:
	debug(WAIT, "Started waiting %llu usecs after power on\n",
	      wait_start - gPowerOnTime);
	return hdmi_device_start_error ? -1 : 0;
}

void hdmi_device_stop()
{
        if ( !hdmi_device_started )
                return;

	hdmi_controller_stop_video();

	set_power(false);
                
        hdmi_device_started = false;               
}


/////////////////////////////////////////
////////// hdmi-device local functions

static int set_power(bool state)
{
    int ret = 0;
    
    return ret;
}


