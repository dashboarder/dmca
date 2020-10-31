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
#include <AppleEffaceableStorageKeys.h>

#include "effaceable_private_types.h"
#include "effaceable_storage_dev.h"
#include "effaceable_storage_core.h"
#include "effaceable_delegation.h"

// =============================================================================

#define kAppleEffaceableStorageEnableFullScanDefault false
#define kAppleEffaceableStorageEnableWipeDefault false

#define kAppleEffaceableStorageGroupInvalid ((uint32_t) -1)
#define kAppleEffaceableStorageMinGroupCount 2

// =============================================================================

#define debug(...) dlogf(storage, __VA_ARGS__ )
#define hexdump(...) dhexdump(storage, __VA_ARGS__ )

// =============================================================================

// debug-only registry keys

#define kAppleEffaceableStoragePropertyGeneration     "Generation"
#define kAppleEffaceableStoragePropertyCapacity       "Capacity"
#define kAppleEffaceableStoragePropertyGroupCount     "GroupCount"
#define kAppleEffaceableStoragePropertyGroupIndex     "GroupIndex"
#define kAppleEffaceableStoragePropertyUnitsPerGroup  "UnitsPerGroup"
#define kAppleEffaceableStoragePropertyUnitSize       "UnitSize"
#define kAppleEffaceableStoragePropertyCopiesPerUnit  "CopiesPerUnit"
#define kAppleEffaceableStoragePropertyCopySize       "CopySize"

// =============================================================================

extern void
startEffaceableStorage(effaceable_storage_t * storage, effaceable_device_t * device, effaceable_system_t * system)
{
    initContext(storage, device, system);
    initContract(storage);

    setIsFormatted(storage, false);
    setGeneration(storage, kAppleEffaceableStorageGenerationUnknown);
    setGroup(storage, kAppleEffaceableStorageGroupInvalid);

    setFullScanEnabled(storage, kAppleEffaceableStorageEnableFullScanDefault);
    setWipeEnabled(storage, kAppleEffaceableStorageEnableWipeDefault);

    // allocate storage, expecting system contract to do the right thing if no memory available
    setCache(storage, allocMem(storage, getCopySize(storage)));
    setScratch(storage, allocMem(storage, getCopySize(storage)));
    setSniff(storage, allocMem(storage, getCopySize(storage)));
    setCrypt(storage, allocMem(storage, getUnitSize(storage)));

    // start with no lockers in memory
    TAILQ_INIT(getLockerList(storage));

    // update initial (static) properties
    updateInitialProperties(storage);

    // try to find current clone, if any
    findCurrentClone(storage, getScratch(storage), getSniff(storage), getCache(storage));

    debug(INIT, "started");
}

static void
updateInitialProperties(effaceable_storage_t * storage)
{
    // debug properties
    setPropertySys(storage, kAppleEffaceableStoragePropertyCapacity, getCapacity(storage));
    setPropertySys(storage, kAppleEffaceableStoragePropertyGroupCount, getGroupCount(storage));
    setPropertySys(storage, kAppleEffaceableStoragePropertyUnitsPerGroup, getUnitsPerGroup(storage));
    setPropertySys(storage, kAppleEffaceableStoragePropertyUnitSize, getUnitSize(storage));
    setPropertySys(storage, kAppleEffaceableStoragePropertyCopiesPerUnit, getCopiesPerUnit(storage));
    setPropertySys(storage, kAppleEffaceableStoragePropertyCopySize, getCopySize(storage));

    //  common properties
    setPropertySys(storage, kAppleEffaceableStoragePropertyVersionMajor, kAppleEffaceableStorageVersionMajorCurrent);
    setPropertySys(storage, kAppleEffaceableStoragePropertyVersionMinor, kAppleEffaceableStorageVersionMinorCurrent);
}

// =============================================================================

static uint32_t
getCapacity(effaceable_storage_t * storage)
{
    return kAppleEffaceableStorageContentSize;
}

static bool
isFormatted(effaceable_storage_t * storage)
{
    return getIsFormatted(storage);
}

// =============================================================================

static EffaceableReturn
formatStorage(effaceable_storage_t * storage)
{
    EffaceableReturn ok = kEffaceableReturnSuccess;
    AppleEffaceableStorageClone * clone = (AppleEffaceableStorageClone *)getCache(storage);
    void * tmp_buf = getScratch(storage);
    uint32_t group;
    uint32_t generation = kAppleEffaceableStorageGenerationInitial;

    debug(INIT, "formatting storage");

    if (kEffaceableReturnSuccess == (ok = wipeStorage(storage, tmp_buf))) {
        setMem(storage, &(clone->content[0]), 0, getCapacity(storage));
        for (group = 0; group < getGroupCount(storage); group++) {
            if (kEffaceableReturnSuccess == (ok = writeClone(storage, tmp_buf, clone, group, generation))) {
                break;
            } else {
                if (kEffaceableReturnSuccess != (ok = effaceGroup(storage, tmp_buf, group))) {
                    debug(ERR, "group %d will neither store a clone nor can it be effaced", group);
                    break;
                }
            }
        }
        if (kEffaceableReturnSuccess == ok) {
            setGroup(storage, group);
            setGeneration(storage, generation);
            setIsFormatted(storage, true);
            updateDynamicProperties(storage);
            debug(INIT, "format succeeded");
        } else {
            debug(ERR, "unable to commit initial generation to any group");
        }
    }

    return ok;
}

static EffaceableReturn
getBytes(effaceable_storage_t * storage, void * client_buf, uint32_t offset, uint32_t count)
{
    EffaceableReturn ok = kEffaceableReturnSuccess;
    AppleEffaceableStorageClone * clone = (AppleEffaceableStorageClone *)getCache(storage);

    if (0 == client_buf) {
        debug(ERR, "cannot get bytes to buffer referenced by NULL pointer\n");
        ok = kEffaceableReturnBadArgument;
    } else if ((count > getCapacity(storage)) || (offset > (getCapacity(storage) - count))) {
        debug(ERR, "attempting to read past end of content\n");
        ok = kEffaceableReturnBadArgument;
    } else if (!getIsFormatted(storage)) {
        debug(ERR, "couldn't find the droid you were looking for");
        ok = kEffaceableReturnUnformattedMedia;
    } else {
        moveMem(storage, client_buf, &(clone->content[offset]), count);
    }

    cleanBuffers(storage);
    return ok;
}

static EffaceableReturn
setBytes(effaceable_storage_t * storage, const void * client_buf, uint32_t offset, uint32_t count)
{
    const uint32_t max_attempts = 2;

    EffaceableReturn ok = kEffaceableReturnError;
    AppleEffaceableStorageClone * parent_clone;
    AppleEffaceableStorageClone * mutant_clone;
    void * tmp_buf;
    uint32_t parent_group;
    uint32_t mutant_group;
    uint32_t mutant_generation;
    uint32_t attempt;

    if (0 == client_buf) {
        debug(ERR, "cannot set bytes from buffer referenced by 0 pointer");
        ok = kEffaceableReturnBadArgument;
    } else if ((count > getCapacity(storage)) || (offset > (getCapacity(storage) - count))) {
        debug(ERR, "attempting to write past end of content");
        ok = kEffaceableReturnBadArgument;
    } else if (!getIsFormatted(storage)) {
        debug(ERR, "couldn't find the droid you were looking for");
        ok = kEffaceableReturnUnformattedMedia;
    } else {
        parent_clone = (AppleEffaceableStorageClone *)getCache(storage);
        mutant_clone = (AppleEffaceableStorageClone *)getScratch(storage);
        tmp_buf = getSniff(storage);
        parent_group = getGroup(storage);
        moveMem(storage, mutant_clone, parent_clone, sizeof(AppleEffaceableStorageClone));
        moveMem(storage, &(mutant_clone->content[offset]), client_buf, count);
        mutant_generation = parent_clone->header.veiled.generation + 1;

        // <rdar://problem/7861026> AppleEffaceableStorage::setBytesGated needs to attempt commit to N-1 groups before failing
        for (mutant_group = nextGroup(storage, parent_group), ok = kEffaceableReturnError;
             (mutant_group != parent_group) && (ok != kEffaceableReturnSuccess);
             mutant_group = nextGroup(storage, mutant_group)) {

            for (attempt = 1; attempt <= max_attempts; attempt++) {
                if (kEffaceableReturnSuccess == (ok = writeClone(storage, tmp_buf, mutant_clone, mutant_group, mutant_generation))) {
                    setGroup(storage, mutant_group);
                    setGeneration(storage, mutant_generation);
                    break;
                } else if (kEffaceableReturnSuccess != effaceGroup(storage, tmp_buf, mutant_group)) {
                    debug(ERR, "unable to either write modified content to or efface group %d\n", mutant_group);
                    break;
                }
            }
        }

        if (kEffaceableReturnSuccess == ok) {
            if (kEffaceableReturnSuccess != effaceGroup(storage, tmp_buf, parent_group)) {
                // <rdar://problem/7704284> AppleEffaceableStorage should indicate when best efforts to efface prior generation has failed
            }
            moveMem(storage, getCache(storage), mutant_clone, getCopySize(storage));
            updateDynamicProperties(storage);
            debug(INFO, "generation %d successfully committed to group %d", getGeneration(storage), getGroup(storage));
        }
    }

    cleanBuffers(storage);
    return ok;
}

// =============================================================================

static EffaceableReturn
eraseGroupLogged(effaceable_storage_t * storage, uint32_t group)
{
    EffaceableReturn ok = eraseGroup(storage, group);
    if (kEffaceableReturnSuccess == ok) {
        debug(SPEW, "group := %d  =>  success", group);
    } else {
        debug(SPEW, "group := %d  =>  0x%08X", group, ok);
    }
    return ok;
}

static EffaceableReturn
writeUnitLogged(effaceable_storage_t * storage, const void * io_buf, uint32_t group, uint32_t unit)
{
    EffaceableReturn ok = writeUnit(storage, io_buf, group, unit);
    if (kEffaceableReturnSuccess == ok) {
        debug(SPEW, "group := %d, unit := %d, buffer := %p  =>  success", group, unit, io_buf);
    } else {
        debug(SPEW, "group := %d, unit := %d, buffer := %p  =>  0x%08X", group, unit, io_buf, ok);
    }
    return ok;
}

static EffaceableReturn
readUnitLogged(effaceable_storage_t * storage, void * io_buf, uint32_t group, uint32_t unit)
{
    EffaceableReturn ok = readUnit(storage, io_buf, group, unit);
    if (kEffaceableReturnSuccess == ok) {
        debug(SPEW, "group := %d, unit := %d, buffer := %p  =>  success", group, unit, io_buf);
    } else {
        debug(SPEW, "group := %d, unit := %d, buffer := %p  =>  0x%08X", group, unit, io_buf, ok);
    }
    return ok;
}

// =============================================================================

static void
findCurrentClone(effaceable_storage_t * storage, void * tmp_buf, void * scan_buf, void * clone_buf)
{
    AppleEffaceableStorageClone * scan = (AppleEffaceableStorageClone *)scan_buf;
    AppleEffaceableStorageClone * clone = (AppleEffaceableStorageClone *)clone_buf;

    uint32_t scan_generation = kAppleEffaceableStorageGenerationUnknown;
    uint32_t copy_generation;
    uint32_t valid_groups;
    uint32_t valid_copies;
    uint32_t group;
    uint32_t unit;
    uint32_t copy;

    // XXX refactor following mess into more manageable chunks

    valid_groups = 0;
    for (group = 0; group < getGroupCount(storage); group++) {
        valid_copies = 0;
        scan_generation = kAppleEffaceableStorageGenerationUnknown;
        for (unit = 0; unit < getUnitsPerGroup(storage); unit++) {
            if (kEffaceableReturnSuccess != readUnitLogged(storage, getCrypt(storage), group, unit)) {
                debug(SPEW, "unable to read unit %d group %d", unit, group);
            } else {
                for (copy = 0; copy < getCopiesPerUnit(storage); copy++) {
                    if (kEffaceableReturnSuccess != revealCopy(storage, getCrypt(storage), scan, copy)) {
                        debug(SPEW, "unable to decipher copy %d unit %d group %d", copy, unit, group);
                    } else if (!isCopyValid(storage, scan)) {
                        debug(SPEW, "invalid copy %d unit %d group %d", copy, unit, group);
                    } else {
                        copy_generation = scan->header.veiled.generation;
                        debug(INFO, "valid generation %d @ copy %d unit %d group %d",
                              copy_generation, copy, unit, group);
                        if (0 == valid_copies) {
                            scan_generation = copy_generation;
                        } else if (copy_generation != scan_generation) {
                            // XXX inconsistent copies within current scan group
                        }
                        if (getGeneration(storage) < scan_generation) {
                            debug(SPEW, "oldest generation found is now %d; prior was %d",
                                  scan_generation, getGeneration(storage));
                            if (0 != valid_groups) {
                                // this group has older copies
                                // than prior valid oldest, so
                                // efface prior oldest
                                if (kEffaceableReturnSuccess != effaceGroup(storage, tmp_buf, getGroup(storage))) {
                                    // XXX what now?
                                }
                            }
                            setGeneration(storage, scan_generation);
                            setGroup(storage, group);
                            moveMem(storage, clone, scan, getCopySize(storage));
                        }
                        valid_copies++;
                        if (!getFullScanEnabled(storage)) {
                            // if full scan disabled, stop checking
                            // copies after first valid one found
                            break;
                        }
                    }
                }
            }
            if (!getFullScanEnabled(storage) && (0 != valid_copies)) {
                // if full scan enabled, stop checking copies
                // after first valid one found
                break;
            }
        }
        if (0 != valid_copies) {
            if ((0 != valid_groups) && (getGroup(storage) != group)) {
                // already found older valid group; efface current
                if (kEffaceableReturnSuccess != effaceGroup(storage, tmp_buf, group)) {
                    // XXX what now?
                }
            }
            valid_groups++;
        }
    }

    if (0 != valid_groups) {
        foundCurrentCloneInGroup(storage, getGroup(storage));
    } else {
        debug(ERR, "unable to find content");
    }

    cleanBuffers(storage);
}

static bool
isGroupEffaced(effaceable_storage_t * storage, void * tmp_buf, uint32_t group)
{
    AppleEffaceableStorageClone * clone = (AppleEffaceableStorageClone *)tmp_buf;
    uint32_t valid_copies = 0;
    uint32_t unit;
    uint32_t copy;

    for (unit = 0; unit < getUnitsPerGroup(storage); unit++) {
        if (kEffaceableReturnSuccess == readUnitLogged(storage, getCrypt(storage), group, unit)) {
            for (copy = 0; copy < getCopiesPerUnit(storage); copy++) {
                if ((kEffaceableReturnSuccess == revealCopy(storage, getCrypt(storage), clone, copy)) && isCopyValid(storage, clone)) {
                    valid_copies++;
                }
            }
        }
    }

    return (valid_copies == 0);
}

static EffaceableReturn
obscureCopy(effaceable_storage_t * storage, void * clear_buf, void * crypt_buf, uint32_t copy)
{
    EffaceableReturn ok = kEffaceableReturnSuccess;

    AppleEffaceableStorageClone * clear_clone = (AppleEffaceableStorageClone *)clear_buf;
    AppleEffaceableStorageClone * crypt_clone = &(((AppleEffaceableStorageClone *)crypt_buf)[copy]);

    const uint32_t naked_size = sizeof(AppleEffaceableStorageNaked);
    const uint32_t salt_size = kAppleEffaceableStorageSaltSize;
    const uint32_t veiled_size = sizeof(AppleEffaceableStorageClone) - naked_size - salt_size;
    const uint32_t pad_count = naked_size / sizeof(uint32_t);

    uint32_t index;

    // XXX assert that naked_size == salt_size

    // generate a per-copy random IV
    readRandom(storage, &(crypt_clone->header.salt[0]), salt_size);

    // use salt as pad to obscure naked header content
    for (index = 0; index < pad_count; index++) {
        const uint32_t pad = ((uint32_t *)&(crypt_clone->header.salt[0]))[index];
        ((uint32_t *)&(crypt_clone->header.naked))[index] = (pad ^ ((uint32_t *)&(clear_clone->header.naked))[index]);
    }

    debug(SPEW, "encrypting content");
    moveMem(storage, &(crypt_clone->header.veiled), &(clear_clone->header.veiled), veiled_size);

    return ok;
}

static EffaceableReturn
revealCopy(effaceable_storage_t * storage, void * crypt_buf, void * clear_buf, uint32_t copy)
{
    EffaceableReturn ok = kEffaceableReturnSuccess;

    AppleEffaceableStorageClone * clear_clone = (AppleEffaceableStorageClone *)clear_buf;
    AppleEffaceableStorageClone * crypt_clone = &(((AppleEffaceableStorageClone *)crypt_buf)[copy]);

    const uint32_t naked_size = sizeof(AppleEffaceableStorageNaked);
    const uint32_t salt_size = kAppleEffaceableStorageSaltSize;
    const uint32_t veiled_size = sizeof(AppleEffaceableStorageClone) - naked_size - salt_size;
    const uint32_t pad_count = naked_size / sizeof(uint32_t);

    uint32_t index;

    // XXX assert that naked_size == salt_size

    // zero clone
    setMem(storage, clear_clone, 0, sizeof(AppleEffaceableStorageClone));

    // use salt as pad to reveal naked header content
    for (index = 0; index < pad_count; index++) {
        const uint32_t pad = ((uint32_t *)&(crypt_clone->header.salt[0]))[index];
        ((uint32_t *)&(clear_clone->header.naked))[index] = (pad ^ ((uint32_t *)&(crypt_clone->header.naked))[index]);
    }

    debug(SPEW, "decrypting content");
    moveMem(storage, &(clear_clone->header.veiled), &(crypt_clone->header.veiled), veiled_size);

    return ok;
}

static EffaceableReturn
commitClone(effaceable_storage_t * storage, void * clone_buf, uint32_t group)
{
    EffaceableReturn ok = kEffaceableReturnError;
    uint32_t written_units = 0;
    uint32_t unit;
    uint32_t copy;

    for (copy = 0; copy < getCopiesPerUnit(storage); copy++) {
        if (kEffaceableReturnSuccess != obscureCopy(storage, clone_buf, getCrypt(storage), copy)) {
            debug(WARN, "unable to obscure clone");
        } else {
            ok = kEffaceableReturnSuccess;
        }
    }

    if (kEffaceableReturnSuccess == ok) {
        for (unit = 0; unit < getUnitsPerGroup(storage); unit++) {
            if (kEffaceableReturnSuccess != (ok = writeUnitLogged(storage, getCrypt(storage), group, unit))) {
                debug(WARN, "unable to write unit %d group %d during commit",
                      unit, group);
            } else {
                written_units++;
            }
        }

        if (0 == written_units) {
            debug(ERR, "unable to write any units in group %d during commit", group);
            ok = kEffaceableReturnIOError;
        } else {
            debug(SPEW, "committed %d of %d units in group %d",
                  written_units, getUnitsPerGroup(storage), group);
            ok = kEffaceableReturnSuccess;
        }
    }

    return ok;
}

static EffaceableReturn
writeClone(effaceable_storage_t * storage, void * tmp_buf, void * clone_buf, uint32_t group, uint32_t generation)
{
    EffaceableReturn ok = kEffaceableReturnError;
    AppleEffaceableStorageClone * clone = (AppleEffaceableStorageClone *)clone_buf;

    setMem(storage, &(clone->header), 0, sizeof(AppleEffaceableStorageHeader));
    clone->header.naked.magic = kAppleEffaceableStorageMagic;
    clone->header.naked.version_major = kAppleEffaceableStorageVersionMajorCurrent;
    clone->header.naked.version_minor = kAppleEffaceableStorageVersionMinorCurrent;
    clone->header.naked.flags = kAppleEffaceableStorageFlagsNone;
    clone->header.veiled.generation = (uint32_t) generation;
    clone->header.veiled.checksum = checksumCopy(storage, clone);

    if (kEffaceableReturnSuccess == (ok = commitClone(storage, clone, group))) {
        ok = confirmClone(storage, tmp_buf, clone, group);
    }

    return ok;
}

static uint32_t
checksumCopy(effaceable_storage_t * storage, void * clone_buf)
{
    AppleEffaceableStorageClone * clone = (AppleEffaceableStorageClone *)clone_buf;
    uint32_t crc = crc32Sys(storage, 0, 0, 0, false);
    crc = crc32Sys(storage, crc, &(clone->header.naked), sizeof(AppleEffaceableStorageNaked), false);
    crc = crc32Sys(storage, crc, &(clone->header.veiled), sizeof(AppleEffaceableStorageVeiled) - sizeof(uint32_t), false);
    crc = crc32Sys(storage, crc, &(clone->content[0]), kAppleEffaceableStorageContentSize, true);
    return crc;
}

static EffaceableReturn
wipeStorage(effaceable_storage_t * storage, void * tmp_buf)
{
    EffaceableReturn ok = kEffaceableReturnSuccess;
    uint32_t efface_count = 0;
    uint32_t group;

    if (getIsFormatted(storage) && !getWipeEnabled(storage)) {
        debug(ERR, "wipe of formatted storage not permitted without 'effaceable-enable-wipe=1' boot-arg");
        ok = kEffaceableReturnNotPermitted;
    } else {
        setGroup(storage, kAppleEffaceableStorageGroupInvalid);
        setIsFormatted(storage, false);

        for (group = 0; group < getGroupCount(storage); group++) {
            if (kEffaceableReturnSuccess == effaceGroup(storage, tmp_buf, group)) {
                efface_count++;
            }
        }

        if (kAppleEffaceableStorageMinGroupCount <= efface_count) {
            if (getGroupCount(storage) > efface_count) {
                debug(WARN, "only able to efface %d out of %d groups", efface_count, getGroupCount(storage));
            }
            ok = kEffaceableReturnSuccess;
        } else {
            debug(ERR, "unable to efface at least %d out of %d groups", kAppleEffaceableStorageMinGroupCount, getGroupCount(storage));
            ok = kEffaceableReturnBadMedia;
        }
    }
    debug(INIT, "%s", (kEffaceableReturnSuccess == ok) ? "succeeded" : "failed");

    return ok;
}

static bool
isCopyValid(effaceable_storage_t * storage, const void * clone_buf)
{
    bool ok = false;
    AppleEffaceableStorageClone * clone = (AppleEffaceableStorageClone *)clone_buf;
    uint32_t checksum;

    if (kAppleEffaceableStorageMagic != clone->header.naked.magic) {
        // this is the expected result during scan for all copies other than current generation
        debug(SPEW, "bad magic - expect 0x%08X, have 0x%08X",
              kAppleEffaceableStorageMagic, clone->header.naked.magic);
    } else if (kAppleEffaceableStorageGenerationUnknown == clone->header.veiled.generation) {
        debug(ERR, "have 'unknown' generation: 0x%08X",
              kAppleEffaceableStorageGenerationUnknown);
    } else if (kAppleEffaceableStorageGenerationInvalid == clone->header.veiled.generation) {
        debug(ERR, "have 'invalid' generation: 0x%08X",
              kAppleEffaceableStorageGenerationInvalid);
    } else if (kAppleEffaceableStorageVersionMajorCurrent != clone->header.naked.version_major) {
        debug(ERR, "major version mismatch - expect 0x%08X, have 0x%08X",
              kAppleEffaceableStorageVersionMajorCurrent, clone->header.naked.version_major);
    } else if (kAppleEffaceableStorageVersionMinorUnknown == clone->header.naked.version_minor) {
        debug(ERR, "have 'unknown' minor version: 0x%08X",
              kAppleEffaceableStorageVersionMinorUnknown);
    } else if (kAppleEffaceableStorageVersionMinorInvalid == clone->header.naked.version_minor) {
        debug(ERR, "have 'invalid' minor version: 0x%08X",
              kAppleEffaceableStorageVersionMinorInvalid);
    } else if ((checksum = checksumCopy(storage, clone)) != clone->header.veiled.checksum) {
        debug(ERR, "checksum failed - calculated 0x%08X, have 0x%08X",
              checksum, clone->header.veiled.checksum);
    } else {
        debug(SPEW, "copy appears valid; checksum is 0x%08X", checksum);
        ok = true;
    }
    return ok;
}

static EffaceableReturn
confirmClone(effaceable_storage_t * storage, void * tmp_buf, void * clone_buf, uint32_t group)
{
    EffaceableReturn ok = kEffaceableReturnError;

    AppleEffaceableStorageClone * clone = (AppleEffaceableStorageClone *)clone_buf;
    AppleEffaceableStorageClone * twin = (AppleEffaceableStorageClone *)tmp_buf;
    uint32_t readable_units = 0;
    uint32_t valid_copies = 0;
    uint32_t unit;
    uint32_t copy;

    for (unit = 0; unit < getUnitsPerGroup(storage); unit++) {
        if (kEffaceableReturnSuccess != readUnitLogged(storage, getCrypt(storage), group, unit)) {
            debug(WARN, "unable to read back unit %d group %d", unit, group);
        } else {
            readable_units++;
            ok = kEffaceableReturnNotFound;
            for (copy = 0; copy < getCopiesPerUnit(storage); copy++) {
                if (kEffaceableReturnSuccess != revealCopy(storage, getCrypt(storage), twin, copy)) {
                    debug(WARN, "couldn't decipher copy %d unit %d group %d", copy, unit, group);
                } if (!isCopyValid(storage, twin)) {
                    debug(WARN, "copy %d unit %d group %d not valid", copy, unit, group);
                } else if (0 != cmpMem(storage, &(clone->content), &(twin->content), getCapacity(storage))) {
                    debug(WARN, "read back differs from original: copy %d unit %d group %d", copy, unit, group);
                } else {
                    valid_copies++;
                    ok = kEffaceableReturnSuccess;
                }
            }
            if (kEffaceableReturnSuccess != ok) {
                debug(WARN, "failed confirmation of all copies in unit %d group %d", unit, group);
                debug(SPEW, "clone @ %p:", clone);
                debug(SPEW, "crypt @ %p:", getCrypt(storage));
                debug(SPEW, "twin @ %p:", twin);
            }
        }
    }

    if (0 != valid_copies) {
        debug(SPEW, "commit confirmed for %d of %d copies found in %d of %d readable units of group %d",
              valid_copies, getUnitsPerGroup(storage) * getCopiesPerUnit(storage), readable_units, getUnitsPerGroup(storage), group);
        ok = kEffaceableReturnSuccess;
    } else if (0 != readable_units) {
        debug(ERR, "no valid copies found given %d of %d readable units during confirmation of group %d",
              readable_units, getUnitsPerGroup(storage), group);
        ok = kEffaceableReturnNotFound;
    } else {
        debug(ERR, "all %d units unreadable during confirmation of group %d",
              getUnitsPerGroup(storage), group);
        ok = kEffaceableReturnIOError;
    }


    return ok;
}

static uint32_t
nextGroup(effaceable_storage_t * storage, uint32_t group)
{
    uint32_t subsequent = group + 1;
    return ((getGroupCount(storage) <= subsequent) ? 0 : subsequent);
}

static EffaceableReturn
effaceGroup(effaceable_storage_t * storage, void * tmp_buf, uint32_t group)
{
    EffaceableReturn ok = kEffaceableReturnError;

    const uint32_t max_attempts = 3;

    uint32_t attempt;
    uint32_t unit;

    setMem(storage, getCrypt(storage), 0, getUnitSize(storage));
    for (attempt = 0; attempt < max_attempts; attempt++) {
        // erase the group
        if (kEffaceableReturnSuccess != eraseGroupLogged(storage, group)) {
            debug(WARN, "erase failed on group %d\n", group);
        }
        // whether or not the erase succeeded, attempt to read each
        // unit from the group
        if (isGroupEffaced(storage, tmp_buf, group)) {
            // if we were unable to read any unit, the group has been
            // properly effaced
            debug(SPEW, "effaced group %d on attempt %d after erase", group, attempt + 1);
            break;
        } else {
            // if any valid unit was able to be read, attempt to
            // overwrite every unit in group with zeros before
            // next erase attempt
            debug(WARN, "found valid copies after erase on group %d\n", group);
            for (unit = 0; unit < getUnitsPerGroup(storage); unit++) {
                writeUnitLogged(storage, getCrypt(storage), group, unit);
            }
        }
    }

    if (max_attempts == attempt) {
        debug(ERR, "unable to efface group %d after %d attempts", group, attempt);
        ok = kEffaceableReturnBadMedia;
    } else {
        ok = kEffaceableReturnSuccess;
    }

    return ok;
}

static void
cleanBuffers(effaceable_storage_t * storage)
{
    deviceCleanBuffers(storage);

    setMem(storage, getScratch(storage), 0, getCopySize(storage));
    setMem(storage, getSniff(storage), 0, getCopySize(storage));
    setMem(storage, getCrypt(storage), 0, getUnitSize(storage));
}

static uint32_t
getCopySize(effaceable_storage_t * storage)
{
    return sizeof(AppleEffaceableStorageClone);
}

static uint32_t
getCopiesPerUnit(effaceable_storage_t * storage)
{
    return (getUnitSize(storage) / getCopySize(storage));
}

static void
foundCurrentCloneInGroup(effaceable_storage_t * storage, uint32_t group)
{
    setIsFormatted(storage, true);
    deviceFoundCurrentCloneInGroup(storage, group);
    updateDynamicProperties(storage);
    debug(INIT, "found current generation, %d, in group %d", getGeneration(storage), group);
}

static void
updateDynamicProperties(effaceable_storage_t * storage)
{
    deviceUpdateDynamicProperties(storage);

    // debug
    //
    // XXX conditionalize these
    setPropertySys(storage, kAppleEffaceableStoragePropertyGeneration, getGeneration(storage));
    setPropertySys(storage, kAppleEffaceableStoragePropertyGroupIndex, getGroup(storage));

    // common
    setPropertySys(storage, kAppleEffaceableStoragePropertyIsFormatted, getIsFormatted(storage));
}

// =============================================================================

static EffaceableReturn
getLocker(effaceable_storage_t * storage, uint32_t type_id, void *data, uint32_t *data_size, bool untrusted)
{
    EffaceableReturn  result;
    bool notprivate = false; 

    result = getLockerInternal(storage, type_id, data, data_size, untrusted, notprivate);
    return(result);
}

static EffaceableReturn
getLockerExternal(effaceable_storage_t * storage, uint32_t type_id, void *data, uint32_t *data_size, bool untrusted)
{
    EffaceableReturn  result;
    bool notprivate = true; 

    result = getLockerInternal(storage, type_id, data, data_size, untrusted, notprivate);
    return(result);
}

static EffaceableReturn
getLockerInternal(effaceable_storage_t * storage, uint32_t type_id, void *data, uint32_t *data_size, bool untrusted, bool notprivate)
{
    EffaceableReturn  result;
    locker_t             *lp;
 
    if (0 != (lp = findLockerForRequest(storage, type_id))) {
 
        // check permissions on this locker 
        if (untrusted && (lp->header.type_id & kAppleEffaceableStorageLockerProtected)) {
            debug(ERR, "untrusted client tried to read protected key");
            result = kEffaceableReturnNotPrivileged;
            goto out;
        }

        // check permissions to see if an external client is trying to read driver private locker 
        if (notprivate && (lp->header.type_id & kAppleEffaceableStorageLockerPrivate)) {
            debug(ERR, "external client tried to read private locker");
            result = kEffaceableReturnNotPrivileged;
            goto out;
        }

        // if the caller supplied a buffer, copy what data will fit
        if ((0 != data) && (0 != data_size) && (0 != *data_size))
            moveMem(storage, data, &lp->data[0], min(*data_size, lp->header.data_size));

        // return actual data size if requested
        if (0 != data_size)
            *data_size = lp->header.data_size;

        result = kEffaceableReturnSuccess;
    } else {
        debug(INFO, "getLocker for type not present");
        result = kEffaceableReturnNotFound;
    }
out:
    discardLockers(storage);
    return(result);
}

static EffaceableReturn
setLocker(effaceable_storage_t * storage, uint32_t type_id, const void *data, uint32_t data_size, bool untrusted)
{
    EffaceableReturn        result;
    bool notprivate = false;

    result = setLockerInternal(storage, type_id, data, data_size, untrusted, notprivate);
    return result;
}

static EffaceableReturn
setLockerExternal(effaceable_storage_t * storage, uint32_t type_id, const void *data, uint32_t data_size, bool untrusted)
{
    EffaceableReturn        result;
    bool notprivate = true;

    result = setLockerInternal(storage, type_id, data, data_size, untrusted, notprivate);
    return result;
}

static EffaceableReturn
setLockerInternal(effaceable_storage_t * storage, uint32_t type_id, const void *data, uint32_t data_size, bool untrusted, bool notprivate)
{
    EffaceableReturn        result;
    locker_t                   *lp;
    uint16_t                     dataSize;

    // verify that kernel-only options aren't being set by non-kernel clients
    if (untrusted && (type_id & kAppleEffaceableStorageLockerProtected)) {
        debug(ERR, "untrusted client trying to create protected locker");
        return kEffaceableReturnNotPrivileged;
    }

    // check permissions to see if an external client isn't trying to set a driver private locker
    if (notprivate && (type_id & kAppleEffaceableStorageLockerPrivate)) {
        debug(ERR, "external client tried to write private locker");
        return kEffaceableReturnNotPrivileged;
    }

    // cannot set sentinel type
    if (type_id == kAppleEffaceableStorageLockerSentinel) {
        debug(ERR, "client trying to create locker sentinel");
        return kEffaceableReturnBadArgument;
    }

    // look for an existing locker that we might be overwriting and toss it
    if (0 != (lp = findLockerForRequest(storage, type_id))) {

        // untrusted client cannot overwrite protected entry
        if (untrusted && (lp->header.type_id & kAppleEffaceableStorageLockerProtected)) {
            debug(ERR, "untrusted client trying to overwrite protected locker");
            result = kEffaceableReturnNotPrivileged;
            goto out;
        }

        // external client cannot overwrite driver private locker 
        if (notprivate && (lp->header.type_id & kAppleEffaceableStorageLockerPrivate)) {
            debug(ERR, "external client tried to overwrite private locker");
            result = kEffaceableReturnNotPrivileged;
            goto out;
        }

        // unlink, overwrite with zeros and then free
        TAILQ_REMOVE(getLockerList(storage), lp, link);
        dataSize = lp->header.data_size;
        setMem(storage, lp, 0, sizeof(*lp) + dataSize);
        freeMem(storage, lp, sizeof(*lp) + dataSize);
    }

    // allocate a new locker
    lp = (locker_t *)allocMem(storage, sizeof(*lp) + data_size);

    // copy data from the request
    lp->header.magic             = kAppleEffaceableStorageLockerMagic;
    lp->header.type_id           = type_id;
    lp->header.data_size         = data_size;
    if (data_size > 0)
        moveMem(storage, &lp->data[0], data, data_size);

    // insert it into the list
    TAILQ_INSERT_HEAD(getLockerList(storage), lp, link);

    // push the updated list back to storage
    result = putLockersInStorage(storage);
out:
    // discard lockers (may still have them on the error path)
    discardLockers(storage);
    return result;
}

static EffaceableReturn
spaceForLocker(effaceable_storage_t * storage, uint32_t type_id, uint32_t *data_space)
{
    EffaceableReturn  result;
    uint32_t          consumed = 0;
    locker_t       *lp;

    if (0 == data_space)
        return(kEffaceableReturnBadArgument);

    // get lockers if we don't have them already cached
    if (TAILQ_EMPTY(getLockerList(storage)) &&
        (kEffaceableReturnSuccess != (result = getLockersFromStorage(storage)))) {
        debug(ERR, "failed to get lockers");
        return result;
    }

    // scan lockers accounting for space
    TAILQ_FOREACH(lp, getLockerList(storage), link) {
        // always account for header
        consumed += sizeof(*lp);

        // if not the candidate type, data space is spoken for
        if ((type_id & ~kAppleEffaceableStorageLockerProtected) !=
            (lp->header.type_id & ~kAppleEffaceableStorageLockerProtected))
            consumed += lp->header.data_size;
    }

    // sanity-check the result
    if (consumed > kAppleEffaceableStorageLockerSize) {
        debug(ERR, "internal locker overflow");
        result = kEffaceableReturnInternalError;
    } else {
        *data_space = kAppleEffaceableStorageLockerSize - consumed;
        result = kEffaceableReturnSuccess;
    }

    // dump lockers
    discardLockers(storage);

    return result;
}

static EffaceableReturn
effaceLocker(effaceable_storage_t * storage, uint32_t type_id, bool untrusted)
{
    EffaceableReturn  result;
    bool notprivate = false;

    result = effaceLockerInternal(storage, type_id, untrusted, notprivate);
    return result;
}

static EffaceableReturn
effaceLockerExternal(effaceable_storage_t * storage, uint32_t type_id, bool untrusted)
{
    EffaceableReturn  result;
    bool notprivate = true;

    result = effaceLockerInternal(storage, type_id, untrusted, notprivate);
    return result;
}

static EffaceableReturn
effaceLockerInternal(effaceable_storage_t * storage, uint32_t type_id, bool untrusted, bool notprivate)
{
    EffaceableReturn  result;
    locker_t             *lp;
    uint16_t            dataSize;

    // cannot efface sentinel type
    if (type_id == kAppleEffaceableStorageLockerSentinel) {
        debug(ERR, "attempt to efface locker sentinel");
        return kEffaceableReturnBadArgument;
    }

    // wildcard efface?
    if (kAppleEffaceableStorageLockerWildcard == type_id) {

        // Even discards private lockers    
        if (untrusted) {
            debug(ERR, "attempt to wildcard efface from untrusted client");
            return kEffaceableReturnNotPermitted;
        }

        // discard lockers and push to storage - this will effectively write zeroes
        // to the entire locker space
        discardLockers(storage);
        return putLockersInStorage(storage);

    }
    // find a locker matching
    if (0 != (lp = findLockerForRequest(storage, type_id))) {

        // check permissions on this locker
        if (untrusted && (lp->header.type_id & kAppleEffaceableStorageLockerProtected)) {
            debug(ERR, "unprivileged client attempting to efface protected locker");
            result = kEffaceableReturnNotPrivileged;
            goto out;
        }

        // check permissions to see if external client trying to efface driver private locker 
        if (notprivate && (lp->header.type_id & kAppleEffaceableStorageLockerPrivate)) {
            debug(ERR, "external client tried to efface driver private locker");
            result = kEffaceableReturnNotPrivileged;
            goto out;
        }

        // yank from the list, zero and free
        TAILQ_REMOVE(getLockerList(storage), lp, link);
        dataSize = lp->header.data_size;
        setMem(storage, lp, 0, sizeof(*lp) + dataSize);
        freeMem(storage, lp, sizeof(*lp) + dataSize);

        // push the updated list back to storage
        result = putLockersInStorage(storage);
    } else {
        debug(INFO, "attempt to efface nonexistent locker");
        result = kEffaceableReturnNotFound;
    }

out:
    discardLockers(storage);
    return result;
}

// =============================================================================

static void
appendSentinel(effaceable_storage_t * storage)
{
    locker_t *lp;

    // construct the tail sentinel
    lp = (locker_t *)allocMem(storage, sizeof(*lp));
    setMem(storage, lp, 0, sizeof(*lp));
    lp->header.magic = kAppleEffaceableStorageLockerMagic;
    lp->header.type_id = kAppleEffaceableStorageLockerSentinel;
    TAILQ_INSERT_TAIL(getLockerList(storage), lp, link);
}

static EffaceableReturn
getLockersFromStorage(effaceable_storage_t * storage)
{
    EffaceableReturn                            result;
    uint8_t                               *lockerData;
    uint32_t                         cursor;
    AppleEffaceableStorageLockerHeader  header;
    locker_t                            *lp;

    lockerData = (uint8_t *)allocMem(storage, kAppleEffaceableStorageLockerSize);

    if (kEffaceableReturnSuccess == (result = getBytes(storage, lockerData,
                                                       kAppleEffaceableStorageLockerBase,
                                                       kAppleEffaceableStorageLockerSize))) {

        TAILQ_INIT(getLockerList(storage));
        cursor = 0;
        for (;;) {
            // get the next header
            moveMem(storage, &header, lockerData + cursor, sizeof(header));

            // check for header magic
            if (header.magic != kAppleEffaceableStorageLockerMagic)
                break;

            // check for tail sentinel but don't copy - we will
            // re-create when we finish here
            if (header.type_id == kAppleEffaceableStorageLockerSentinel)
                break;

            // Check for overflow in two phases; we don't
            // trust the header data size, so check that
            // it's sufficiently small that it's not going
            // to overflow anything first.  Then check that the
            // allocation doesn't overflow our buffer.
            if (header.data_size > kAppleEffaceableStorageLockerSize)
                break;
            if ((cursor + sizeof(header) + header.data_size) > kAppleEffaceableStorageLockerSize)
                break;

            // allocate a node
            lp = (locker_t *)allocMem(storage, sizeof(*lp) + header.data_size);
            moveMem(storage, &lp->header, &header, sizeof(header));
            if (header.data_size > 0)
                moveMem(storage, &lp->data, lockerData + cursor + sizeof(header), header.data_size);

            TAILQ_INSERT_TAIL(getLockerList(storage), lp, link);

            // move the cursor
            cursor += sizeof(header) + header.data_size;

            // if there's no room for another header, we're done
            if ((cursor + sizeof(header)) >= kAppleEffaceableStorageLockerSize)
                break;
        }

        // add the tail sentinel
        appendSentinel(storage);
    }
    // zero key data out of the buffer before freeing
    setMem(storage, lockerData, 0, kAppleEffaceableStorageLockerSize);
    freeMem(storage, lockerData, kAppleEffaceableStorageLockerSize);

    return(result);
}

static EffaceableReturn
putLockersInStorage(effaceable_storage_t * storage)
{
    EffaceableReturn                 result;
    uint8_t                               *lockerData;
    uint32_t                         cursor;
    locker_t                            *lp;

    lockerData = (uint8_t *)allocMem(storage, kAppleEffaceableStorageLockerSize);

    cursor = 0;
    result = kEffaceableReturnSuccess;
    TAILQ_FOREACH(lp, getLockerList(storage), link) {

        // verify the node will fit
        if ((cursor + sizeof(lp->header) + lp->header.data_size) >= kAppleEffaceableStorageLockerSize) {
            debug(ERR, "locker overflow");
            result = kEffaceableReturnNoSpace;
            break;
        }

        // copy the header
        moveMem(storage, lockerData + cursor, &lp->header, sizeof(lp->header));
        cursor += sizeof(lp->header);

        // copy the data
        if (lp->header.data_size > 0) {
            moveMem(storage, lockerData + cursor, &lp->data[0], lp->header.data_size);
            cursor += lp->header.data_size;
        }
    }

    // only write back if everything fit
    if (kEffaceableReturnSuccess == result)
        result = setBytes(storage, lockerData,
                          kAppleEffaceableStorageLockerBase,
                          kAppleEffaceableStorageLockerSize);

    // zero key data out of the buffer before freeing
    setMem(storage, lockerData, 0, kAppleEffaceableStorageLockerSize);
    freeMem(storage, lockerData, kAppleEffaceableStorageLockerSize);

    return(result);
}

static EffaceableReturn
discardLockers(effaceable_storage_t * storage)
{
    locker_t        *lp, *lp_temp;
    uint16_t          dataSize;

    TAILQ_FOREACH_SAFE(lp, getLockerList(storage), link, lp_temp) {
        TAILQ_REMOVE(getLockerList(storage), lp, link);
        // zero the key data out of the locker before we free it
        dataSize = lp->header.data_size;
        setMem(storage, lp, 0, sizeof(*lp) + dataSize);
        freeMem(storage, lp, sizeof(*lp) + dataSize);
    }
    TAILQ_INIT(getLockerList(storage));
    return kEffaceableReturnSuccess;
}

static locker_t *
findLockerForRequest(effaceable_storage_t * storage, uint32_t type_id)
{
    locker_t        *lp;
    uint32_t          request_type, locker_type;

    // do we have lockers in memory?
    if (TAILQ_EMPTY(getLockerList(storage))) {
        // if we can't get lockers, there's nothing much we can do
        if (kEffaceableReturnSuccess != getLockersFromStorage(storage))
            return 0;
    }

    // iterate lockers
    request_type = type_id & ~kAppleEffaceableStorageLockerProtected;
    TAILQ_FOREACH(lp, getLockerList(storage), link) {

        // mask out attribute bits during lookup
        locker_type = lp->header.type_id & ~kAppleEffaceableStorageLockerProtected;
        if (request_type == locker_type)
            return lp;
    }
    // not found
    return 0;
}

// -----------------------------------------------------------------------------

#define SHA1_HASH_SIZE 20
#define kEffaceableNonceKey ((uint32_t)0x4E6F6E63 /* 'Nonc' */)
#define kEffaceableNonceId  (kEffaceableNonceKey | ((uint32_t)kAppleEffaceableStorageLockerProtected))

static void logNonce(effaceable_storage_t * storage, uint64_t nonce, void * hash)
{
    // XXX reduce verbosity level prior to submitting
    debug(SPEW, "nonce: 0x%016llX", nonce);

    if (0 != hash) {
        uint8_t * b = (uint8_t*)hash;
        debug(SPEW, "hash: "
              "%02X %02X %02X %02X %02X %02X %02X %02X %02X %02X "
              "%02X %02X %02X %02X %02X %02X %02X %02X %02X %02X",
              b[ 0], b[ 1], b[ 2], b[ 3], b[ 4], b[ 5], b[ 6], b[ 7], b[ 8], b[ 9],
              b[10], b[11], b[12], b[13], b[14], b[15], b[16], b[17], b[18], b[19]);
    }
}

static EffaceableReturn generateNonce(effaceable_storage_t * storage, void * hash)
{
    EffaceableReturn ok = kEffaceableReturnSuccess;
    uint64_t nonce;
    void * nonce_hash = getNonceHash(storage);

    // Generate and store nonce only if one hasn't already been
    // generated since system start.
    if (0 != nonce_hash) {
        debug(INFO, "nonce previously generated");
    } else {

        // Generate a 64-bit random number from entropy pool to serve as nonce
        // and attempt to store in protected nonce locker.
        readRandom(storage, &nonce, sizeof(nonce));
        ok = setLocker(storage, kEffaceableNonceId, &nonce, sizeof(nonce), false);

        // If nonce was successfully stored, calculate and save its SHA1 hash.
        if (kEffaceableReturnSuccess != ok) {
            debug(ERR, "unable to set nonce");
        } else {
            nonce_hash = allocMem(storage, SHA1_HASH_SIZE);
            setMem(storage, nonce_hash, 0, SHA1_HASH_SIZE);

            calcSHA1(storage, &nonce, sizeof(nonce), nonce_hash);
            setNonceHash(storage, nonce_hash);

            logNonce(storage, nonce, nonce_hash);
            debug(INFO, "nonce generated");
        }
    }

    // If successful up to this point, copy our copy of nonce hash
    // to the buffer supplied by client.
    if (kEffaceableReturnSuccess == ok) {
        moveMem(storage, hash, nonce_hash, SHA1_HASH_SIZE);
    }

    return ok;
}

static EffaceableReturn consumeNonce(effaceable_storage_t * storage, uint64_t * nonce)
{
    EffaceableReturn ok = kEffaceableReturnSuccess;
    uint64_t val;
    uint32_t size = sizeof(val);

    ok = getLocker(storage, kEffaceableNonceId, &val, &size, false);

    if (kEffaceableReturnSuccess == ok) {

        // Nonce always gets effaced upon consumption.
        ok = effaceLocker(storage, kEffaceableNonceId, false);

        if (kEffaceableReturnSuccess != ok) {
            debug(ERR, "failed to efface nonce");
        } else {

            logNonce(storage, val, 0);
            if (0 != nonce) {
                *nonce = val;
                debug(INFO, "nonce consumed");
            } else {
                debug(INFO, "nonce discarded");
            }
        }
    }

    return ok;
}

// =============================================================================

static void
initContract(effaceable_storage_t * storage)
{
    // hook operations intended for external use
    storage->getLocker         = getLockerExternal;
    storage->setLocker         = setLockerExternal;
    storage->spaceForLocker    = spaceForLocker;
    storage->effaceLocker      = effaceLockerExternal;
    storage->generateNonce     = generateNonce;
    storage->consumeNonce      = consumeNonce;
    storage->formatStorage     = formatStorage;
    storage->getBytes          = getBytes;
    storage->wipeStorage       = wipeStorage;
    storage->getCapacity       = getCapacity;
    storage->isFormatted       = isFormatted;

    // hook operations intended for use by device contracts
    storage->getCopiesPerUnit  = getCopiesPerUnit;
    storage->getCopySize       = getCopySize;
}

// =============================================================================

static inline effaceable_storage_context_t * context(effaceable_storage_t * storage)
{
    return (effaceable_storage_context_t *)storage->opaque;
}

// -----------------------------------------------------------------------------

static inline void * getCache(effaceable_storage_t * storage)
{
    return context(storage)->cache;
}

static inline void * setCache(effaceable_storage_t * storage, void * buf)
{
    return context(storage)->cache = buf;
}

static inline void * getScratch(effaceable_storage_t * storage)
{
    return context(storage)->scratch;
}

static inline void * setScratch(effaceable_storage_t * storage, void * buf)
{
    return context(storage)->scratch = buf;
}

static inline void * getSniff(effaceable_storage_t * storage)
{
    return context(storage)->sniff;
}

static inline void * setSniff(effaceable_storage_t * storage, void * buf)
{
    return context(storage)->sniff = buf;
}

static inline void * getCrypt(effaceable_storage_t * storage)
{
    return context(storage)->crypt;
}

static inline void * setCrypt(effaceable_storage_t * storage, void * buf)
{
    return context(storage)->crypt = buf;
}

static inline void * getNonceHash(effaceable_storage_t * storage)
{
    return context(storage)->nonce_hash;
}

static inline void * setNonceHash(effaceable_storage_t * storage, void * buf)
{
    return context(storage)->nonce_hash = buf;
}

static inline uint32_t getGeneration(effaceable_storage_t * storage)
{
    return context(storage)->generation;
}

static inline uint32_t setGeneration(effaceable_storage_t * storage, uint32_t generation)
{
    return context(storage)->generation = generation;
}

static inline uint32_t getGroup(effaceable_storage_t * storage)
{
    return context(storage)->group;
}

static inline uint32_t setGroup(effaceable_storage_t * storage, uint32_t group)
{
    return context(storage)->group = group;
}

static inline bool getIsFormatted(effaceable_storage_t * storage)
{
    return context(storage)->is_formatted;
}

static inline bool setIsFormatted(effaceable_storage_t * storage, bool is_formatted)
{
    return context(storage)->is_formatted = is_formatted;
}

static inline bool getFullScanEnabled(effaceable_storage_t * storage)
{
    return context(storage)->enable_full_scan;
}

static inline bool setFullScanEnabled(effaceable_storage_t * storage, bool enable)
{
    return context(storage)->enable_full_scan = enable;
}

static inline bool getWipeEnabled(effaceable_storage_t * storage)
{
    return context(storage)->enable_wipe;
}

static inline bool setWipeEnabled(effaceable_storage_t * storage, bool enable)
{
    return context(storage)->enable_wipe = enable;
}

static inline locker_list_t * getLockerList(effaceable_storage_t * storage)
{
    return &context(storage)->locker_list;
}

// -----------------------------------------------------------------------------

static inline uint32_t getGroupCount(effaceable_storage_t * storage)
{
    return delegateFn(storage, device, getGroupCount);
}

static inline uint32_t getUnitsPerGroup(effaceable_storage_t * storage)
{
    return delegateFn(storage, device, getUnitsPerGroup);
}

static inline uint32_t getUnitSize(effaceable_storage_t * storage)
{
    return delegateFn(storage, device, getUnitSize);
}

static inline bool eraseGroup(effaceable_storage_t * storage, uint32_t group)
{
    return delegateFn(storage, device, eraseGroup, group);
}

static inline bool writeUnit(effaceable_storage_t * storage, const void * buf, uint32_t group, uint32_t unit)
{
    return delegateFn(storage, device, writeUnit, buf, group, unit);
}

static inline bool readUnit(effaceable_storage_t * storage, void * buf, uint32_t group, uint32_t unit)
{
    return delegateFn(storage, device, readUnit, buf, group, unit);
}

// -----------------------------------------------------------------------------

static inline void deviceUpdateDynamicProperties(effaceable_storage_t * storage)
{
    if (isFnAvailable(storage, device, updateDynamicProperties)) {
        delegateFn(storage, device, updateDynamicProperties);
    }
}

static inline void deviceFoundCurrentCloneInGroup(effaceable_storage_t * storage, uint32_t group)
{
    if (isFnAvailable(storage, device, foundCurrentCloneInGroup)) {
        delegateFn(storage, device, foundCurrentCloneInGroup, group);
    }
}

static inline void deviceCleanBuffers(effaceable_storage_t * storage)
{
    if (isFnAvailable(storage, device, cleanBuffers)) {
        delegateFn(storage, device, cleanBuffers);
    }
}

// -----------------------------------------------------------------------------

static inline void * allocMem(effaceable_storage_t * storage, uint32_t size)
{
    return delegateFn(storage, system, allocMem, size);
}

static inline void freeMem(effaceable_storage_t * storage, void * buf, uint32_t size)
{
    delegateFn(storage, system, freeMem, buf, size);
}

static inline void * setMem(effaceable_storage_t * storage, void * buf, uint8_t val, uint32_t size)
{
    return delegateFn(storage, system, setMem, buf, val, size);
}

static inline void * moveMem(effaceable_storage_t * storage, void * dst, const void * src, uint32_t size)
{
    return delegateFn(storage, system, moveMem, dst, src, size);
}

static inline int cmpMem(effaceable_storage_t * storage, const void * lhs, const void * rhs, uint32_t size)
{
    return delegateFn(storage, system, cmpMem, lhs, rhs, size);
}

static inline void readRandom(effaceable_storage_t * storage, void * buf, uint32_t size)
{
    delegateFn(storage, system, readRandom, buf, size);
}

static inline void calcSHA1(effaceable_storage_t * storage, const void * buf, uint32_t size, void * hash)
{
    delegateFn(storage, system, calcSHA1, buf, size, hash);
}

static inline uint32_t crc32Sys(effaceable_storage_t * storage, uint32_t crc, const void * buf, uint32_t size, bool finish)
{
    return delegateFn(storage, system, crc32, crc, buf, size, finish);
}

static inline bool setPropertySys(effaceable_storage_t * storage, const char * key, uint32_t value)
{
    return delegateFn(storage, system, setProperty, key, value);
}

static inline void panicSys(effaceable_storage_t * storage, const char * msg)
{
    delegateFn(storage, system, panicSys, msg);
}

static int logf(effaceable_storage_t * storage, const char * fmt, ...)
{
    int err;

    va_list ap;
    va_start(ap, fmt);
    err = delegateFn(storage, system, vlogf, fmt, ap);
    va_end(ap);

    return err;
}

// -----------------------------------------------------------------------------

static void
initContext(effaceable_storage_t * storage, effaceable_device_t * device, effaceable_system_t * system)
{
    storage->opaque = system->allocMem(system, sizeof(effaceable_storage_context_t));
    system->setMem(system, context(storage), 0, sizeof(effaceable_storage_context_t));

    context(storage)->system = system;
    context(storage)->device = device;
}

// =============================================================================
