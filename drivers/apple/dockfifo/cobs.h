#ifndef COBS_H
#define COBS_H	1

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

#include <stdbool.h>

// COBS-encode from an input buffer to an output buffer.
// Input and output must not be the same or overlap.
// The byte stuffing algorithm may grow the data size.
// Returns true if the output fits in out_buf_bytes, and sets
// *out_buf_used to the new length.
// Returns false if the output does not fit. Output buffer is clobbered.
bool cobs_encode(void *out_buf, size_t out_buf_bytes,
		 const void *in_buf, size_t in_buf_bytes,
		 size_t *out_buf_used);

// COBS-decode from an input buffer to an output buffer.
// Input and output must not be the same or overlap.
// The byte stuffing algorithm may shrink the data size.
// Returns true if properly COBS-encoded data was decoded successfully,
// and sets *out_buf_used to the new length.
// Returns false if illegal COBS codes are present, or are inconsistent
// with the input buffer size. Output buffer clobbered.
bool cobs_decode(void *out_buf, size_t out_buf_bytes,
		 const void *in_buf, size_t in_buf_bytes,
		 size_t *out_buf_used);

#endif  // defined(COBS_H)
