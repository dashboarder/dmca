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

#include <IOKit/IOLib.h>

#include "AppleEffaceableNOR.h"

#include "effaceable_contract.h"
#include "effaceable_debug.h"

// =============================================================================

#define debug(...) dlogf((void*)0, __VA_ARGS__ )

// =============================================================================


#ifdef super
#undef super
#endif
#define super AppleEffaceableStorage

OSDefineMetaClassAndStructors(AppleEffaceableNOR, AppleEffaceableStorage);

// =============================================================================

bool
AppleEffaceableNOR::start(IOService *provider)
{
    bool ok = false;

    debug(INFO, "starting");

    setupNorContract();

    if (_is_started) {
        debug(INFO, "already started");
        ok = true;
    } else if (0 == (_nor_flash = OSDynamicCast(AppleARMNORFlashDevice, provider))) {
        debug(ERR, "bug in kext matching");
    } else if (!super::start(provider)) {
        debug(ERR, "super failed to start");
    } else {
        debug(INIT, "started");
        _is_started = true;
        ok = true;
    }

    return ok;
}

// =============================================================================

effaceable_nor_hal_t *
AppleEffaceableNOR::nor(void)
{
    return &_nor_hal;
}

void
AppleEffaceableNOR::setupDeviceContract(void)
{
    startEffaceableNOR(device(), system(), nor(), storage());
}

// =============================================================================

AppleEffaceableNOR * context(effaceable_nor_hal_t * nor)
{
    AppleEffaceableNOR * context;

    // XXX assert instead?
    if (0 == (context = (AppleEffaceableNOR *)nor->opaque))
        panic("null context");

    return context;
}

uint32_t getRegionCountHook(effaceable_nor_hal_t * nor)
{
    return context(nor)->getRegionCountNor();
}

uint32_t getRegionSizeHook(effaceable_nor_hal_t * nor, uint32_t index)
{
    return context(nor)->getRegionSizeNor(index);
}

EffaceableReturn eraseRegionHook(effaceable_nor_hal_t * nor, uint32_t index, uint32_t length)
{
    return context(nor)->eraseRegionNor(index, length);
}

uint32_t writeRegionHook(effaceable_nor_hal_t * nor, uint32_t index, const void * buf, uint32_t length)
{
    return context(nor)->writeRegionNor(index, buf, length);
}

bool readRegionHook(effaceable_nor_hal_t * nor, uint32_t index, void * buf, uint32_t length)
{
    return context(nor)->readRegionNor(index, buf, length);
}

// =============================================================================

void
AppleEffaceableNOR::setupNorContract(void)
{
    nor()->opaque = this;

    nor()->getRegionCount = getRegionCountHook;
    nor()->getRegionSize = getRegionSizeHook;
    nor()->eraseRegion = eraseRegionHook;
    nor()->writeRegion = writeRegionHook;
    nor()->readRegion = readRegionHook;
}

uint32_t
AppleEffaceableNOR::getRegionCountNor(void)
{
    return _nor_flash->getRegionCount();
}

uint32_t
AppleEffaceableNOR::getRegionSizeNor(uint32_t index)
{
    return _nor_flash->getRegionSize(index);
}

EffaceableReturn
AppleEffaceableNOR::eraseRegionNor(uint32_t index, uint32_t length)
{
    return efReturn(_nor_flash->eraseRegion(index, 0, length));
}

uint32_t
AppleEffaceableNOR::writeRegionNor(uint32_t index, const void * buf, uint32_t length)
{
    return _nor_flash->writeRegion(index, (const UInt8*)buf, length);
}

bool
AppleEffaceableNOR::readRegionNor(uint32_t index, void * buf, uint32_t length)
{
    return _nor_flash->readRegion(index, (UInt8*)buf, length);
}

// =============================================================================
