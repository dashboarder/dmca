/*
 * Copyright (c) 2011 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */

#ifndef _EFFACEABLE_STORAGE_API_H_
#define _EFFACEABLE_STORAGE_API_H_

// =============================================================================

#include <sys/types.h>

// =============================================================================
//
// iBoot-specific presentation of the effaceable storage API for client use.
//
// =============================================================================

bool effaceable_get_locker(uint32_t type_id, void * data, uint32_t * data_size);
bool effaceable_set_locker(uint32_t type_id, const void * data, uint32_t data_size);
bool effaceable_space_for_locker(uint32_t type_id, uint32_t * data_space);
bool effaceable_efface_locker(uint32_t type_id);
bool effaceable_consume_nonce(uint64_t * nonce);

// =============================================================================
//
// iBoot-specific presentation of the effaceable storage API for util/devel.
//
// =============================================================================

bool effaceable_get_bytes(void * client_buf, uint32_t offset, uint32_t count);
uint32_t effaceable_get_capacity(void);

// =============================================================================

#endif /* _EFFACEABLE_STORAGE_API_H_ */
