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

#include <stdint.h>
#include <IOKit/IOLib.h>

#include "AppleEffaceableNAND.h"

#include "effaceable_contract.h"
#include "effaceable_debug.h"

// =============================================================================

#define debug(...) dlogf((void*)0, __VA_ARGS__ )

// =============================================================================

#ifdef super
#undef super
#endif
#define super AppleEffaceableStorage

OSDefineMetaClassAndStructors(AppleEffaceableNAND, AppleEffaceableStorage);

// =============================================================================

bool
AppleEffaceableNAND::start(IOService *provider)
{
    bool ok = false;

    debug(INFO, "starting");

    setupNandContract();

    if (0 == (_partition = OSDynamicCast(IOFlashStoragePartition, provider))) {
        debug(ERR, "provider is not an IOFlashStoragePartition");
    } else if (!super::start(provider)) {
        debug(ERR, "superclass unable to start");
    } else {
        debug(INIT, "started");
        ok = true;
    }

    return ok;
}

// =============================================================================

effaceable_nand_hal_t *
AppleEffaceableNAND::nand(void)
{
    return &_nand_hal;
}

void
AppleEffaceableNAND::setupDeviceContract(void)
{
    startEffaceableNAND(device(), system(), nand(), storage());
}

// =============================================================================

AppleEffaceableNAND * context(effaceable_nand_hal_t * nand)
{
    AppleEffaceableNAND * context;

    // XXX assert instead?
    if (0 == (context = (AppleEffaceableNAND *)nand->opaque))
        panic("null context");

    return context;
}

uint32_t getPageSizeHook(effaceable_nand_hal_t * nand)
{
    return context(nand)->getPageSizeNand();
}

uint32_t getPageCountHook(effaceable_nand_hal_t * nand)
{
    return context(nand)->getPageCountNand();
}

uint32_t getBlockCountHook(effaceable_nand_hal_t * nand)
{
    return context(nand)->getBlockCountNand();
}

uint32_t getBankCountHook(effaceable_nand_hal_t * nand)
{
    return context(nand)->getBankCountNand();
}

bool isBlockBadHook(effaceable_nand_hal_t * nand, uint32_t bank, uint32_t block)
{
    return context(nand)->isBlockBadNand(bank, block);
}

EffaceableReturn eraseBlockHook(effaceable_nand_hal_t * nand, uint32_t bank, uint32_t block)
{
    return context(nand)->eraseBlockNand(bank, block);
}

EffaceableReturn writePageHook(effaceable_nand_hal_t * nand, uint32_t bank, uint32_t block, uint32_t page, const void * buf, uint32_t length)
{
    return context(nand)->writePageNand(bank, block, page, buf, length);
}

EffaceableReturn readPageHook(effaceable_nand_hal_t * nand, uint32_t bank, uint32_t block, uint32_t page, void * buf, uint32_t length)
{
    return context(nand)->readPageNand(bank, block, page, buf, length);
}

bool requestPartitionTableDiffHook(effaceable_nand_hal_t * nand, void * buf, uint32_t length)
{
    return context(nand)->requestPartitionTableDiffNand(buf, length);
}

bool providePartitionTableDiffHook(effaceable_nand_hal_t * nand, void * buf, uint32_t length)
{
    return context(nand)->providePartitionTableDiffNand(buf, length);
}

// =============================================================================

void
AppleEffaceableNAND::setupNandContract(void)
{
    nand()->opaque = this;

    nand()->getPageSize               = getPageSizeHook;
    nand()->getPageCount              = getPageCountHook;
    nand()->getBlockCount             = getBlockCountHook;
    nand()->getBankCount              = getBankCountHook;
    nand()->isBlockBad                = isBlockBadHook;
    nand()->eraseBlock                = eraseBlockHook;
    nand()->writePage                 = writePageHook;
    nand()->readPage                  = readPageHook;
    nand()->requestPartitionTableDiff = requestPartitionTableDiffHook;
    nand()->providePartitionTableDiff = providePartitionTableDiffHook;
}

uint32_t AppleEffaceableNAND::getPageSizeNand(void)
{
    return (IOByteCount)_partition->getPageSize();
}

uint32_t AppleEffaceableNAND::getPageCountNand(void)
{
    return _partition->getPagesPerBlock();
}

uint32_t AppleEffaceableNAND::getBlockCountNand(void)
{
    return _partition->getBlockCount();
}

uint32_t AppleEffaceableNAND::getBankCountNand(void)
{
    return _partition->getBankCount();
}

bool AppleEffaceableNAND::isBlockBadNand(uint32_t bank, uint32_t block)
{
    return _partition->isBlockBad(bank, block);
}

EffaceableReturn AppleEffaceableNAND::eraseBlockNand(uint32_t bank, uint32_t block)
{
    return efReturn(_partition->eraseBlock(bank, block));
}

EffaceableReturn AppleEffaceableNAND::writePageNand(uint32_t bank, uint32_t block, uint32_t page, const void * buf, uint32_t length)
{
    IOReturn ok;
    IOBufferMemoryDescriptor * bmd;

    // XXX It's unfortunate that the IOBMD is necessary; think about how/where to clean it up.

    bmd = IOBufferMemoryDescriptor::withBytes(buf, length, kIODirectionInOut, true);
    if (0 == bmd)
        panic("no memory");

    ok = _partition->writePage(bank, block, page, bmd);

    bmd->release();

    return efReturn(ok);
}

EffaceableReturn AppleEffaceableNAND::readPageNand(uint32_t bank, uint32_t block, uint32_t page, void * buf, uint32_t length)
{
    IOReturn ok;
    IOBufferMemoryDescriptor * bmd;

    // XXX It's unfortunate that the IOBMD is necessary; think about how/where to clean it up.

    bmd = IOBufferMemoryDescriptor::withCapacity(length, kIODirectionInOut, true);
    if (0 == bmd)
        panic("no memory");

    ok = _partition->readPage(bank, block, page, bmd);

    if (kIOReturnSuccess == ok)
        bmd->readBytes(0, buf, length);
    bmd->release();

    return efReturn(ok);
}

bool AppleEffaceableNAND::requestPartitionTableDiffNand(void * buf, uint32_t length)
{
    return _partition->requestPartitionTableDiff(buf, length);
}

bool AppleEffaceableNAND::providePartitionTableDiffNand(void * buf, uint32_t length)
{
    return _partition->providePartitionTableDiff(buf, length);
}

// =============================================================================
