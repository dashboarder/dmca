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
#ifndef __LIB_RANDOM_H
#define __LIB_RANDOM_H

#include <sys/types.h>

__BEGIN_DECLS

// Fills the provided buffer with random data
int random_get_bytes(u_int8_t *buffer, size_t length);
int random_get_bytes_noheap(uint8_t *buffer, size_t length, void *entropy_buffer, size_t entropy_buffer_size);

#if DEBUG_BUILD
// Fills the provided buffer with random data
// Allows setting the entropy ratio
// This is for debug only
int random_get_bytes_debug(u_int8_t *buffer, size_t length, u_int32_t entropy_ratio);
#endif

__END_DECLS

#endif /* __LIB_RANDOM_H */
