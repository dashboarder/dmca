/*
 * Copyright (C) 2011-2012 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */
#ifndef __CONSISTENT_DEBUG_HELPER_FNS__
#define __CONSISTENT_DEBUG_HELPER_FNS__

#include "consistent_debug_registry.h"

// C Helper functions that should serve as a pseudo-library
// for other codebases that use Consistent Debug


// Root Pointer encoder/decoder

/**
 * Encode 64-bit physical address of root pointer into a 
 * suitable 32-bit format. 
 * 
 * @param root_pointer 64-bit pointer to the physical address of 
 * the consistent debug registry 
 * @param pointer_type One of the types documented in "Debug 
 * Root Pointer Scratch Register Format" of 
 * consistent_debug_registry.h 
 * 
 * @return uint32_t 0: Cannot encode address with specified 
 * type, or invalid type. Otherwise, returns a 32-bit 
 * representation of the address. 
 */
static inline uint32_t consistent_debug_encode_root_pointer(uint64_t root_pointer, unsigned int pointer_type)
{
    if (pointer_type != 0) {
        return 0; // Only type 0 is supported currently
    }

    if (root_pointer & 0xFFF) {
        return 0; // Not page-aligned, truncation would lose information
    }
    root_pointer >>= 12;

    if (root_pointer >= 0x3FFFFFFFF) {
        return 0; // Too large to represent
    }

    return (uint32_t)root_pointer;
}

/**
 * opposite of consistent_debug_encode_root_pointer; decodes a root 
 * pointer encoded in that manner. 
 * 
 * @return uint64_t 0 if cannot be decoded, otherwise the 
 *         physical address.
 */
static inline uint64_t consistent_debug_decode_root_pointer(uint32_t encoded_value)
{
    if (encoded_value & 0xC0000000) {
        return 0; // Can't decode anything other than a "type 0" address
    }
    uint64_t ret = encoded_value;
    ret <<= 12;
    return ret;
}


#endif // __CONSISTENT_DEBUG_HELPER_FNS__
