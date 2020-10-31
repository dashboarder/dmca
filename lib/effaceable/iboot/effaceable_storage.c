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

// =============================================================================

#include <lib/effaceable.h>
#include <lib/effaceable_impl.h>
#include <lib/libc.h>

#include <effaceable_contract.h>
#include <effaceable_debug.h>

// =============================================================================

static int logf(void * ignored, const char * fmt, ...);

#define debug(...) dlogf((void*)0, __VA_ARGS__ )

// =============================================================================

static effaceable_storage_t * gEffaceableStorage = (void*)0;

// =============================================================================

extern void registerEffaceableStorage(effaceable_storage_t * storage)
{
    gEffaceableStorage = storage;
}

// =============================================================================

effaceable_storage_t * locateEffaceableStorage(void)
{
    if (0 == gEffaceableStorage) {
        debug(ERR, "no storage available");
    }

    return gEffaceableStorage;
}

bool trustEffaceableContext(void)
{
#if DEBUG_BUILD
    return true;
#else
    return false;
#endif
}

// =============================================================================

bool effaceable_get_locker(uint32_t type_id, void * data, uint32_t * data_size)
{
    bool trust = trustEffaceableContext();
    effaceable_storage_t * storage = locateEffaceableStorage();

    if (0 == storage) {
        return false;
    }

    if (kEffaceableReturnSuccess != storage->getLocker(storage, type_id, data, data_size, !trust)) {
        return false;
    }
    
    return true;
}

bool effaceable_set_locker(uint32_t type_id, const void * data, uint32_t data_size)
{
    bool trust = trustEffaceableContext();
    effaceable_storage_t * storage = locateEffaceableStorage();

    if (0 == storage) {
        return false;
    }

    if (kEffaceableReturnSuccess != storage->setLocker(storage, type_id, data, data_size, !trust)) {
        return false;
    }
    
    return true;
}

bool effaceable_space_for_locker(uint32_t type_id, uint32_t * data_space)
{
    effaceable_storage_t * storage = locateEffaceableStorage();

    if (0 == storage) {
        return false;
    }

    if (kEffaceableReturnSuccess != storage->spaceForLocker(storage, type_id, data_space)) {
        return false;
    }
    
    return true;
}

bool effaceable_efface_locker(uint32_t type_id)
{
    bool trust = trustEffaceableContext();
    effaceable_storage_t * storage = locateEffaceableStorage();

    if (0 == storage) {
        return false;
    }

    if (kEffaceableReturnSuccess != storage->effaceLocker(storage, type_id, !trust)) {
        return false;
    }
    
    return true;
}

bool effaceable_consume_nonce(uint64_t * nonce)
{
    effaceable_storage_t * storage = locateEffaceableStorage();

    if (0 == storage) {
        return false;
    }

    if (kEffaceableReturnSuccess != storage->consumeNonce(storage, nonce)) {
        return false;
    }
    
    return true;
}

// =============================================================================

#if DEBUG_BUILD

bool effaceable_get_bytes(void * client_buf, uint32_t offset, uint32_t count)
{
    effaceable_storage_t * storage = locateEffaceableStorage();

    if (0 == storage) {
        return false;
    }

    if (kEffaceableReturnSuccess != storage->getBytes(storage, client_buf, offset, count)) {
        return false;
    }
    
    return true;
}

uint32_t effaceable_get_capacity(void)
{
    effaceable_storage_t * storage = locateEffaceableStorage();
    uint32_t capacity = 0;

    if (0 != storage) {
        capacity = storage->getCapacity(storage);
    }

    debug(INFO, "capacity is %d bytes", capacity);

    return capacity;
}

#endif

// =============================================================================

static int logf(void * ignored, const char * fmt, ...)
{
    int err;
    
    va_list ap;
    va_start(ap, fmt);
    err = vprintf(fmt, ap);
    va_end(ap);

    return err;
}

// =============================================================================
