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

#include <effaceable_contract.h>
#include <effaceable_debug.h>

#include <sys.h>
#include <AssertMacros.h>
#include <lib/blockdev.h>

// =============================================================================

#define WITH_EFFACEABLE_NOR_DEBUG 0

// =============================================================================

static int logf(void * ignored, const char * fmt, ...);

#define debug(...) dlogf((void*)0, __VA_ARGS__ )

// =============================================================================
//
// Effaceable NOR region definition is currently only supplied by means
// of publishing device tree up to iOS.  Currently, in order to get this off
// its feet, I'm about to hard code equivalent information here; however,
// that's obviously not a very good solution going forward (for any of the
// NOR memory map, honestly).
//
// =============================================================================

#define NOR_BDEV "nor0"
#define REGION_SIZE ((uint32_t)0x1000)

static const uint32_t region_offsets[] = {0xFA000, 0xFB000};

// =============================================================================

void validate(effaceable_nor_hal_t * nor, uint32_t index, uint32_t length)
{
    require(nor != 0, fail_contract);
    require(nor->opaque != 0, fail_context);
    require(index < sizeof(region_offsets)/sizeof(region_offsets[0]), fail_index);
    require(length <= REGION_SIZE, fail_length);
    return;

fail_contract:
    panic("contract missing");

fail_context:
    panic("context missing");

fail_index:
    panic("bad index");

fail_length:
    panic("bad length");
}

uint32_t getNorOffset(uint32_t index)
{
    return region_offsets[index];
}

static void
doHexdump(const void * buf, uint32_t count)
{
    uint32_t i, j;
    for (i = 0; i < count; i += 16) {
        printf("0x%08x:", ((uint32_t) buf) + i);
        for (j = 0; (j < 16) && ((i + j) < count); j++) {
            printf(" %2.2x", ((uint8_t*)buf)[i+j]);
        }
        printf("\n");
    }
}


// =============================================================================

#define context(nor) ((struct blockdev *)nor->opaque)

// =============================================================================

uint32_t getRegionCountIboot(effaceable_nor_hal_t * nor)
{
    validate(nor, 0, 0);
    return sizeof(region_offsets)/sizeof(region_offsets[0]);
}

uint32_t getRegionSizeIboot(effaceable_nor_hal_t * nor, uint32_t index)
{
    validate(nor, index, 0);
    return REGION_SIZE;
}

EffaceableReturn eraseRegionIboot(effaceable_nor_hal_t * nor, uint32_t index, uint32_t length)
{
    EffaceableReturn ok = kEffaceableReturnSuccess;
    int err;

    validate(nor, index, length);

    err = blockdev_erase(context(nor), getNorOffset(index), length);

    if (err < 0) {
        ok = kEffaceableReturnIOError;
    }
    debug(INFO, "blockdev_erase on '%s' %s", NOR_BDEV, (kEffaceableReturnSuccess == ok) ? "succeeded" : "failed");
    return ok;
}

uint32_t writeRegionIboot(effaceable_nor_hal_t * nor, uint32_t index, const void * buf, uint32_t length)
{
    EffaceableReturn ok = kEffaceableReturnSuccess;
    int err;

    validate(nor, index, length);

    err = blockdev_write(context(nor), buf, getNorOffset(index), length);

    if (err <= 0) {
        ok = kEffaceableReturnIOError;
    }
    debug(INFO, "blockdev_write to '%s' %s", NOR_BDEV, (kEffaceableReturnSuccess == ok) ? "succeeded" : "failed");
    return ok;
}

bool readRegionIboot(effaceable_nor_hal_t * nor, uint32_t index, void * buf, uint32_t length)
{
    EffaceableReturn ok = kEffaceableReturnSuccess;
    int err;

    validate(nor, index, length);

    err = blockdev_read(context(nor), buf, getNorOffset(index), length);

    if (err <= 0) {
        ok = kEffaceableReturnIOError;
    } else {
#if WITH_EFFACEABLE_NOR_DEBUG
        doHexdump(buf, length);
#endif
    }
    debug(INFO, "blockdev_read from '%s' %s", NOR_BDEV, (kEffaceableReturnSuccess == ok) ? "succeeded" : "failed");
    return kEffaceableReturnSuccess;
}

// =============================================================================

EffaceableReturn
setupEffaceableNorContract(effaceable_nor_hal_t * nor)
{
    nor->opaque = lookup_blockdev(NOR_BDEV);

    if (0 == nor->opaque) {
        debug(ERR, "unable to find '%s' blockdev", NOR_BDEV);
        return kEffaceableReturnBadMedia;
    }

    nor->getRegionCount = getRegionCountIboot;
    nor->getRegionSize = getRegionSizeIboot;
    nor->eraseRegion = eraseRegionIboot;
    nor->writeRegion = writeRegionIboot;
    nor->readRegion = readRegionIboot;

    debug(INFO, "nor contract setup complete");

    return kEffaceableReturnSuccess;
}

// =============================================================================

extern void
effaceable_nor_init(void)
{
    effaceable_nor_hal_t * nor = (effaceable_nor_hal_t *)malloc(sizeof(effaceable_nor_hal_t));
    effaceable_device_t * device = (effaceable_device_t *)malloc(sizeof(effaceable_device_t));
    effaceable_system_t * system = (effaceable_system_t *)malloc(sizeof(effaceable_system_t));
    effaceable_storage_t * storage = (effaceable_storage_t *)malloc(sizeof(effaceable_storage_t));

    if ((kEffaceableReturnSuccess == setupEffaceableNorContract(nor)) &&
        (kEffaceableReturnSuccess == setupEffaceableSystemContract(system))) {

        startEffaceableNOR(device, system, nor, storage);
        registerEffaceableStorage(storage);
        debug(INFO, "registered nor-backed effaceable storage");

    } else {

        debug(ERR, "failed to init nor-backed effaceable storage");
    }
}

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
