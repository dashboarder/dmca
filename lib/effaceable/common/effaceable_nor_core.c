/*
 * Copyright (c) 2010-11 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */

// =============================================================================

#include <effaceable_contract.h>
#include <effaceable_debug.h>

#include "effaceable_private_types.h"
#include "effaceable_storage_dev.h"
#include "effaceable_nor_core.h"
#include "effaceable_delegation.h"

// =============================================================================

#define debug(...) dlogf(device, __VA_ARGS__ )

// =============================================================================

extern void
startEffaceableNOR(effaceable_device_t * device, effaceable_system_t * system, effaceable_nor_hal_t * nor, effaceable_storage_t * storage)
{
    initContext(device, system, nor, storage);
    initContract(device);

    startEffaceableStorage(storage, device, system);
}

// =============================================================================

static uint32_t
getGroupCount(effaceable_device_t * device)
{
    return getRegionCount(device);
}

static uint32_t
getUnitsPerGroup(effaceable_device_t * device)
{
    return 1;
}

static uint32_t
getUnitSize(effaceable_device_t * device)
{
    return getRegionSize(device, 0);
}

static EffaceableReturn
eraseGroup(effaceable_device_t * device, uint32_t group)
{
    return eraseRegion(device, group, getUnitSize(device));
}

static EffaceableReturn
writeUnit(effaceable_device_t * device, const void * buf, uint32_t group, uint32_t unit)
{
    // XXX assert unit == 0
    return writeRegion(device, group, buf, getUnitSize(device));
}

static EffaceableReturn
readUnit(effaceable_device_t * device, void * buf, uint32_t group, uint32_t unit)
{
    // XXX assert unit == 0
    return readRegion(device, group, buf, getUnitSize(device));
}

// =============================================================================

static void
initContract(effaceable_device_t * device)
{
    // hook required operations
    device->getGroupCount             = getGroupCount;
    device->getUnitsPerGroup          = getUnitsPerGroup;
    device->getUnitSize               = getUnitSize;
    device->eraseGroup                = eraseGroup;
    device->writeUnit                 = writeUnit;
    device->readUnit                  = readUnit;

    // hook optional operations
    device->cleanBuffers              = (void *)0;
    device->foundCurrentCloneInGroup  = (void *)0;
    device->updateDynamicProperties   = (void *)0;
}

// =============================================================================

static inline effaceable_nor_context_t * context(effaceable_device_t * device)
{
    return (effaceable_nor_context_t *)device->opaque;
}

// -----------------------------------------------------------------------------

static inline uint32_t getCopiesPerUnit(effaceable_device_t * device)
{
    return delegateFn(device, storage, getCopiesPerUnit);
}

static inline uint32_t getCopySize(effaceable_device_t * device)
{
    return delegateFn(device, storage, getCopySize);
}

// -----------------------------------------------------------------------------

static inline uint32_t getRegionCount(effaceable_device_t * device)
{
    return delegateFn(device, nor, getRegionCount);
}

static inline uint32_t getRegionSize(effaceable_device_t * device, uint32_t index)
{
    return delegateFn(device, nor, getRegionSize, index);
}

static inline EffaceableReturn eraseRegion(effaceable_device_t * device, uint32_t index, uint32_t length)
{
    return delegateFn(device, nor, eraseRegion, index, length);
}

static inline uint32_t writeRegion(effaceable_device_t * device, uint32_t index, const void * buf, uint32_t length)
{
    return delegateFn(device, nor, writeRegion, index, buf, length);
}

static inline bool readRegion(effaceable_device_t * device, uint32_t index, void * buf, uint32_t length)
{
    return delegateFn(device, nor, readRegion, index, buf, length);
}

// -----------------------------------------------------------------------------

static inline void * allocMem(effaceable_device_t * device, uint32_t size)
{
    return delegateFn(device, system, allocMem, size);
}

static inline void freeMem(effaceable_device_t * device, void * buf, uint32_t size)
{
    delegateFn(device, system, freeMem, buf, size);
}

static inline void * setMem(effaceable_device_t * device, void * buf, uint8_t val, uint32_t size)
{
    return delegateFn(device, system, setMem, buf, val, size);
}

static inline void * moveMem(effaceable_device_t * device, void * dst, const void * src, uint32_t size)
{
    return delegateFn(device, system, moveMem, dst, src, size);
}

static inline int cmpMem(effaceable_device_t * device, const void * lhs, const void * rhs, uint32_t size)
{
    return delegateFn(device, system, cmpMem, lhs, rhs, size);
}

static inline void readRandom(effaceable_device_t * device, void * buf, uint32_t size)
{
    delegateFn(device, system, readRandom, buf, size);
}

static inline uint32_t crc32Sys(effaceable_device_t * device, uint32_t crc, const void * buf, uint32_t size, bool finish)
{
    return delegateFn(device, system, crc32, crc, buf, size, finish);
}

static inline bool setPropertySys(effaceable_device_t * device, const char * key, uint32_t value)
{
    return delegateFn(device, system, setProperty, key, value);
}

static inline void panicSys(effaceable_device_t * device, const char * msg)
{
    delegateFn(device, system, panicSys, msg);
}

static int logf(effaceable_device_t * device, const char * fmt, ...)
{
    int err;

    va_list ap;
    va_start(ap, fmt);
    err = delegateFn(device, system, vlogf, fmt, ap);
    va_end(ap);

    return err;
}

// =============================================================================

static void
initContext(effaceable_device_t * device, effaceable_system_t * system, effaceable_nor_hal_t * nor, effaceable_storage_t * storage)
{
    device->opaque = system->allocMem(system, sizeof(effaceable_nor_context_t));
    system->setMem(system, context(device), 0, sizeof(effaceable_nor_context_t));

    context(device)->system   = system;
    context(device)->nor      = nor;
    context(device)->storage  = storage;
}

// =============================================================================
