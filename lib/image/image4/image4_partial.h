/*
 * Copyright (C) 2012 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */

#ifndef IMAGE4_PARTIAL_H
#define IMAGE4_PARTIAL_H	1

#include <stdint.h>

/*
 * Maximum number of bytes needed to ID an Image4 file.
 *
 * Image4 header for a large (but <4GB) file contains:
 *
 * SEQUENCE {			30 84 xx xx xx xx
 *   IA5String 'IMG4'           16 04 49 4d 47 34
 *   SEQUENCE {                 30 84 yy yy yy yy
 *     IA5String 'IM4P'         16 04 53 45 50 4f
 *     IA5String 'abcd'         16 04 aa bb cc dd
 */ 
#define IMAGE4_ID_BYTES		30

// Parse a partial buffer enough to identify and get type.
// Return pointers can be NULL if not needed.
int image4_get_partial(const void *buf, size_t buf_bytes,
		       uint32_t *ret_type, uint32_t *ret_size);

#endif // defined(IMAGE4_PARTIAL_H)
