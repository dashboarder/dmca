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

#ifndef _EFFACEABLE_CONTEXT_H_
#define _EFFACEABLE_CONTEXT_H_

// =============================================================================

struct _effaceable_nand_context
{
    void *  buf;

    effaceable_storage_t *   storage;
    effaceable_system_t *    system;
    effaceable_nand_hal_t *  nand;
};

// =============================================================================

struct _effaceable_nor_context
{
    effaceable_storage_t *  storage;
    effaceable_system_t *   system;
    effaceable_nor_hal_t *  nor;
};

// =============================================================================

struct _effaceable_storage_context
{
    bool       enable_full_scan;
    bool       enable_wipe;
    bool       is_formatted;
    uint32_t   generation;
    uint32_t   group;
    void *     cache;
    void *     scratch;
    void *     sniff;
    void *     crypt;
    void *     nonce_hash;

    locker_list_t locker_list;

    effaceable_system_t *  system;
    effaceable_device_t *  device;
};

// =============================================================================

typedef struct _effaceable_nor_context effaceable_nor_context_t;
typedef struct _effaceable_nand_context effaceable_nand_context_t;
typedef struct _effaceable_storage_context effaceable_storage_context_t;

// =============================================================================

#endif /* _EFFACEABLE_CONTEXT_H_ */
