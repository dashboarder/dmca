/*
 * Copyright (C) 2010 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */

#if WITH_HW_MCU

#ifndef PROCESS_EDID_H
#define PROCESS_EDID_H	1

#if WITH_HW_DISPLAY_DISPLAYPORT
#include <drivers/displayport.h>
#endif

#if WITH_HW_DISPLAY_HDMI
#include <drivers/hdmi.h>
#endif

// Collect EDID data and decide on the best scoring display mode.
int obtain_edid(void);

// Break out of the EDID polling loop.
void abort_edid(void);

// Reject timings not matching these dimensions.
void restrict_edid(uint32_t h_active, uint32_t v_active);

// Get previously collected best display mode.
int get_edid_timings(struct video_timing_data *data);

// Get the detected downstream port type, e.g kDPDownstreamTypeHDMI
int get_edid_downstream_type(void);

#endif // PROCESS_EDID_H

#endif // WITH_HW_MCU
