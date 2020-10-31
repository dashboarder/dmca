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

#ifndef __LOOPBACK_DEVICE_H__
#define __LOOPBACK_DEVICE_H__

#include "iop_au_interface.h"

typedef void * loopback_device_t;

/*
 * loopback_device_t is an object that handles looping device rx to device tx
 */

typedef enum {
	kI2S_0 = 0,
	kI2S_1,
	kI2S_2,
	kI2S_3,
	kMCA_0,
	kMCA_1,
	kAudioDevice_Last
} AudioDevice_Index;

// create a loopback_device that will read from device rx, optional process, and send to device tx
loopback_device_t create_loopback_device(AudioDevice_Index device, audio_unit_t optional_au, size_t bytes_per_frame);
void destroy_loopback_device(loopback_device_t device);

typedef enum {
	kFrameError = 0,
	kRXOverrun,
	kRXUnderrun,
	kTXOverrun,
	kTXUnderrun,
	kError_last
} Error_Index;

static const char* const kErrorTypeStr[kError_last] = {
	"Frame_Error",
	"RX_Overrun",
	"RX_Underrun",
	"TX_Overrun",
	"TX_Underrun",
};

uint32_t getErrorCount(loopback_device_t device, Error_Index which);

// return true on success
bool start_loopback_device(loopback_device_t device);
bool stop_loopback_device(loopback_device_t device);

#endif /* __LOOPBACK_DEVICE_H__ */

