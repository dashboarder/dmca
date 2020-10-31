/*
 * Copyright (C) 2010-2011 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */

#ifndef _SPEAKER_PROT_H_
#define _SPEAKER_PROT_H_

#include "debug_tap.h"
#include "iop_au_interface.h"
#include <platform.h>

typedef void * loopback_process_t;

/*
 * loopback process is an object that will process an input buffer in-
 * place, applying the passed Audio Unit to it. 
 *
 * debug_taps can be added at specific tap points.
 *
 * It is assumed that the input number of samples to process matches
 * with the call to process.
 *
 */

// channel bitmask is to allow cases when audio is not packed.
loopback_process_t create_loopback_process(audio_unit_t au, uint32_t frames_to_process, uint32_t channels_to_process, uint32_t sample_size, uint32_t channel_bitmask);

void destroy_loopback_process(loopback_process_t loopback);

void set_debug_tap(loopback_process_t loopback, debug_tap_t inputTap, debug_tap_t outputTap);
void set_tap_point(loopback_process_t loopback, uint32_t index);

// Assumes that input is as described when loopback created
void process_data(loopback_process_t loopback, void *processBuffer);

#endif // _SPEAKER_PROT_H_
