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

#ifndef _DEBUG_TAP_H_
#define _DEBUG_TAP_H_

#include <platform.h>

/*
 * A debug_tap is a way of tapping off or tapping in an audio signal from
 * user space.  Pass the pointer to the beginning, end, and start to create
 * a tap.  You send or get data from a tap, which causes the write or read
 * head to progress around the buffer circularly.  When the process wraps,
 * the send or get call returns the total number of bytes sent, otherwise
 * it returns 0.
 */

typedef void * debug_tap_t;

debug_tap_t create_debug_tap(uint8_t *inIOBegin, uint8_t *inIOEnd, uint8_t *inIO);

void destroy_debug_tap(debug_tap_t tap);

// return value is nonzero on calls where we wrap IO buffer.  Returned
// value indicates number bytes to give to "timestamp" function
size_t send_to_tap(debug_tap_t tap, const uint8_t* src, uint32_t sizeInBytes);
size_t get_from_tap(debug_tap_t tap, uint8_t* dst, uint32_t sizeInBytes);

#endif // _DEBUG_TAP_H_

