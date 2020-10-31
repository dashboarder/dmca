/*
 * COPYRIGHT (c) 2010-11 Apple Inc. All rights reserved.
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
#include "effaceable_nand_core.h"
#include "effaceable_delegation.h"

// =============================================================================

#define debug(...) dlogf(device, __VA_ARGS__ )

// =============================================================================

extern void
startEffaceableNAND(effaceable_device_t * device, effaceable_system_t * system, effaceable_nand_hal_t * nand, effaceable_storage_t * storage)
{
    initContext(device, system, nand, storage);
    initContract(device);

    setBuf(device, allocMem(device, getUnitSize(device)));

    startEffaceableStorage(storage, device, system);
}

// -----------------------------------------------------------------------------

static uint32_t
getGroupCount(effaceable_device_t * device)
{
    return ((uint32_t)(getBankCount(device) * getBlockCount(device)));
}

static uint32_t
getUnitsPerGroup(effaceable_device_t * device)
{
    return ((uint32_t)(getPageCount(device) - kAppleEffaceableNANDTotalMetaPages));
}

static uint32_t
getUnitSize(effaceable_device_t * device)
{
    return getPageSize(device);
}

static EffaceableReturn
eraseGroup(effaceable_device_t * device, uint32_t group)
{
    const uint32_t bank = mapGroupToBank(device, group);
    const uint32_t block = mapGroupToBlock(device, group);

    EffaceableReturn ok = kEffaceableReturnError;

    if (isBlockBad(device, bank, block)) {
        // XXX remap block
        ok = kEffaceableReturnBadMedia;
    } else if (kEffaceableReturnSuccess == (ok = eraseBlock(device, bank, block))) {
        openBlock(device, bank, block);
    }

    return ok;
}

static void
openBlock(effaceable_device_t * device, uint32_t bank, uint32_t block)
{
    uint32_t page;
    const uint32_t start = 0;
    const uint32_t after = kAppleEffaceableNANDOpeningMetaPages;
    void * buf = 0;

    buf = getBuf(device);
    prepOpenCloseMeta(device, kAppleEffaceableNANDMetaOpenMagic, buf);
    for (page = start; page < after; page++) {
        writePage(device, bank, block, page, buf, getPageSize(device));
    }
}

static void
closeBlock(effaceable_device_t * device, uint32_t bank, uint32_t block)
{
    uint32_t page;
    const uint32_t start = (getPageCount(device) - kAppleEffaceableNANDClosingMetaPages);
    const uint32_t after = getPageCount(device);
    void * buf = 0;

    buf = getBuf(device);
    prepOpenCloseMeta(device, kAppleEffaceableNANDMetaCloseMagic, buf);
    prepCloseContent(device, buf);
    for (page = start; page < after; page++) {
        writePage(device, bank, block, page, buf, getPageSize(device));
    }
}

static void
prepOpenCloseMeta(effaceable_device_t * device, uint32_t magic, void * buf)
{
    AppleEffaceableNANDMeta * meta = (AppleEffaceableNANDMeta*)buf;

    whitenMeta(device, meta);
    meta->header.magic = magic;
    meta->header.version_major = kAppleEffaceableNANDMetaVersionMajorCurrent;
    meta->header.version_minor = kAppleEffaceableNANDMetaVersionMinorCurrent;
    meta->header.flags = kAppleEffaceableNANDMetaFlagsNone;
}

static void
prepCloseContent(effaceable_device_t * device, void * buf)
{
    AppleEffaceableNANDMeta * meta = (AppleEffaceableNANDMeta*)buf;

    if (requestPartitionTableDiff(device, meta->content, getMetaContentSize(device))) {
        meta->header.flags |= kAppleEffaceableNANDMetaFlagsHasPTabDiff;
        debug(INFO, "successfully requested PTabDiff");
    } else {
        debug(WARN, "unable to request PTabDiff");
    }
}

static EffaceableReturn
writeUnit(effaceable_device_t * device, const void * buf, uint32_t group, uint32_t unit)
{
    const uint32_t bank = mapGroupToBank(device, group);
    const uint32_t block = mapGroupToBlock(device, group);
    const uint32_t page = mapUnitToPage(device, unit);

    EffaceableReturn ok = kEffaceableReturnError;

    if (isBlockBad(device, bank, block)) {
        // XXX replace block iff passing below critical good block count threshold
        ok = kEffaceableReturnBadMedia;
    } else {
        if (kEffaceableReturnSuccess == (ok = writePage(device, bank, block, page, buf, getPageSize(device)))) {
            // if that was final content page, write block closing meta pages
            if ((getUnitsPerGroup(device) - 1) == unit) {
                closeBlock(device, bank, block);
            }
        }
    }

    return ok;
}

static EffaceableReturn
readUnit(effaceable_device_t * device, void * buf, uint32_t group, uint32_t unit)
{
    const uint32_t bank = mapGroupToBank(device, group);
    const uint32_t block = mapGroupToBlock(device, group);
    const uint32_t page = mapUnitToPage(device, unit);

    EffaceableReturn ok;

    if (isBlockBad(device, bank, block)) {
        // XXX replace block iff passing below critical good block count threshold
        ok = kEffaceableReturnBadMedia;
    } else {
        ok = readPage(device, bank, block, page, buf, getPageSize(device));
    }

    return ok;
}

static uint32_t
mapGroupToBank(effaceable_device_t * device, uint32_t group)
{
    const uint32_t bank = ((uint32_t)group) % getBankCount(device);
    // XXX check bounds
    return bank;
}

static uint32_t
mapGroupToBlock(effaceable_device_t * device, uint32_t group)
{
    const uint32_t block = ((uint32_t)group) / getBankCount(device);
    // XXX check bounds
    return block;
}

static uint32_t
mapUnitToPage(effaceable_device_t * device, uint32_t unit)
{
    const uint32_t page = ((uint32_t)unit) + kAppleEffaceableNANDOpeningMetaPages;
    // XXX check bounds
    return page;
}

// =============================================================================

static uint32_t
getCopiesSize(effaceable_device_t * device)
{
    return getCopySize(device) * getCopiesPerUnit(device);
}

static void
cleanBuffers(effaceable_device_t * device)
{
    setMem(device, getBuf(device), 0, getCopiesSize(device));
}

static void
whitenExtra(effaceable_device_t * device, void * buf)
{
    // XXX seems like this should be getting called; why isn't it?
    const uint32_t copies_size = getCopiesSize(device);
    const uint32_t extra_size = getUnitSize(device) - copies_size;
    readRandom(device, ((uint8_t*)buf) + copies_size, extra_size);
}

static void
whitenMeta(effaceable_device_t * device, void * buf)
{
    readRandom(device, buf, getPageSize(device));
}

static void
foundCurrentCloneInGroup(effaceable_device_t * device, uint32_t group)
{
    const uint32_t magic = kAppleEffaceableNANDMetaCloseMagic;
    const uint32_t major = kAppleEffaceableNANDMetaVersionMajorPTabDiff;
    const uint32_t minor = kAppleEffaceableNANDMetaVersionMinorPTabDiff;
    const uint32_t flag = kAppleEffaceableNANDMetaFlagsHasPTabDiff;

    const uint32_t bank = mapGroupToBank(device, group);
    const uint32_t block = mapGroupToBlock(device, group);
    const uint32_t start = (getPageCount(device) - kAppleEffaceableNANDClosingMetaPages);
    const uint32_t after = getPageCount(device);

    void * buf = getBuf(device);

    uint32_t page;
    AppleEffaceableNANDMeta * meta;

    for (page = start; page < after; page++) {
        debug(INFO, "scanning %lu:%lu:%lu for ptab diff", bank, block, page);
        if (kEffaceableReturnSuccess == readPage(device, bank, block, page, buf, getPageSize(device))) {

            meta = (AppleEffaceableNANDMeta *)buf;
            if (magic != meta->header.magic) {
                debug(ERR, "bad magic for closing page");
            } else if (0 > compareMetaVersions(device, major, minor, meta->header.version_major, meta->header.version_minor)) {
                debug(INFO, "meta version precedes ptab diff implementation");
            } else if (flag != (flag & meta->header.flags)) {
                debug(INFO, "meta doesn't have ptab diff content");
            } else if (!providePartitionTableDiff(device, meta->content, getMetaContentSize(device))) {
                debug(WARN, "ptab diff content rejected");
            } else {
                debug(INFO, "successfully provided ptab diff");
            }

            /* readable closing pages should all be identical; no reason to continue */
            break;
        }
    }
}

static int
compareMetaVersions(effaceable_device_t * device, uint32_t major1, uint32_t minor1, uint32_t major2, uint32_t minor2)
{
    if (major1 < major2) {
        return -1;
    } else if (major1 > major2) {
        return 1;
    } else if (minor1 < minor2) {
        return -1;
    } else if (minor1 > minor2) {
        return 1;
    }
    return 0;
}

static uint32_t
getMetaContentSize(effaceable_device_t * device)
{
    return (getPageSize(device) - kAppleEffaceableNANDMetaHeaderSize);
}

// -----------------------------------------------------------------------------

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
    device->cleanBuffers              = cleanBuffers;
    device->foundCurrentCloneInGroup  = foundCurrentCloneInGroup;
    device->updateDynamicProperties   = (void *)0;
}

// =============================================================================

static inline effaceable_nand_context_t * context(effaceable_device_t * device)
{
    return (effaceable_nand_context_t *)device->opaque;
}

// -----------------------------------------------------------------------------

static inline void * getBuf(effaceable_device_t * device)
{
    return context(device)->buf;
}

static inline void * setBuf(effaceable_device_t * device, void * buf)
{
    return context(device)->buf = buf;
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

static inline uint32_t getPageSize(effaceable_device_t * device)
{
    return delegateFn(device, nand, getPageSize);
}

static inline uint32_t getPageCount(effaceable_device_t * device)
{
    return delegateFn(device, nand, getPageCount);
}

static inline uint32_t getBlockCount(effaceable_device_t * device)
{
    return delegateFn(device, nand, getBlockCount);
}

static inline uint32_t getBankCount(effaceable_device_t * device)
{
    return delegateFn(device, nand, getBankCount);
}

static inline bool isBlockBad(effaceable_device_t * device, uint32_t bank, uint32_t block)
{
    return delegateFn(device, nand, isBlockBad, bank, block);
}

static inline EffaceableReturn eraseBlock(effaceable_device_t * device, uint32_t bank, uint32_t block)
{
    return delegateFn(device, nand, eraseBlock, bank, block);
}

static inline EffaceableReturn writePage(effaceable_device_t * device, uint32_t bank, uint32_t block, uint32_t page, const void * buf, uint32_t length)
{
    return delegateFn(device, nand, writePage, bank, block, page, buf, length);
}

static inline EffaceableReturn readPage(effaceable_device_t * device, uint32_t bank, uint32_t block, uint32_t page, void * buf, uint32_t length)
{
    return delegateFn(device, nand, readPage, bank, block, page, buf, length);
}

static inline bool requestPartitionTableDiff(effaceable_device_t * device, void * buf, uint32_t length)
{
    return delegateFn(device, nand, requestPartitionTableDiff, buf, length);
}

static inline bool providePartitionTableDiff(effaceable_device_t * device, void * buf, uint32_t length)
{
    return delegateFn(device, nand, providePartitionTableDiff, buf, length);
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

// -----------------------------------------------------------------------------

static void
initContext(effaceable_device_t * device, effaceable_system_t * system, effaceable_nand_hal_t * nand, effaceable_storage_t * storage)
{
    device->opaque = system->allocMem(system, sizeof(effaceable_nand_context_t));
    system->setMem(system, context(device), 0, sizeof(effaceable_nand_context_t));

    context(device)->storage  = storage;
    context(device)->system   = system;
    context(device)->nand     = nand;
}

// =============================================================================
