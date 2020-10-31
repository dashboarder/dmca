/*
 * Copyright (c) 2011-2012 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */

#include <effaceable_contract.h>
#include <effaceable_debug.h>
#include <lib/effaceable_impl.h>

#include <string.h>
#include <lib/random.h>
#include <lib/cksum.h>
#include <lib/libc.h>
#include <drivers/sha1.h>
#include <sys.h>

// =============================================================================

#if DEBUG_BUILD
# define USING_STRONG_SEED false
#else
# define USING_STRONG_SEED true
#endif

// =============================================================================

static int logf(void * ignored, const char * fmt, ...);

#define debug(...) dlogf((void*)0, __VA_ARGS__ )

// =============================================================================

void * allocMemIboot(effaceable_system_t * system, uint32_t size)
{
    return malloc(size);
}

void freeMemIboot(effaceable_system_t * system, void * buf, uint32_t size)
{
    free(buf);
}

void * setMemIboot(effaceable_system_t * system, void * buf, uint8_t val, uint32_t size)
{
    return memset(buf, val, size);
}

void * moveMemIboot(effaceable_system_t * system, void * dst, const void * src, uint32_t size)
{
    return memmove(dst, src, size);
}

int cmpMemIboot(effaceable_system_t * system, const void * lhs, const void * rhs, uint32_t size)
{
    return memcmp(lhs, rhs, size);
}

void readRandomIboot(effaceable_system_t * system, void * buf, uint32_t size)
{
    static bool is_seeded = false;
    const unsigned increment = sizeof(int);
    uint8_t * cursor = (uint8_t *)buf;
    unsigned idx;

    // if we're using a strong seed, draw from the entropy pool to
    // re-seed psuedo-random generator the first time function is called
    if (!is_seeded && USING_STRONG_SEED) {
        unsigned int seed;
        random_get_bytes((u_int8_t*)&seed, sizeof(seed));
        srand(seed);
        is_seeded = true;
    }

    // fill with psuedo-random values by 'int'-sized chunks as long as possible
    for (idx = 0; (idx + increment) <= size; idx += increment, cursor += increment) {
        *((int*)cursor) = rand();
    }

    // if size doesn't divide evenly by increment, fill fringe bytewise
    for (; idx < size; idx++, cursor++) {
        *cursor = (uint8_t)rand();
    }
}

void calcSHA1Iboot(effaceable_system_t * system, const void * buf, uint32_t size, void * hash)
{
    sha1_calculate(buf, size, hash);
}

uint32_t crc32Iboot(effaceable_system_t * system, uint32_t crc, const void * buf, uint32_t size, bool finish)
{
    uint32_t val;

    if ((0 == crc) && (0 == buf) && (0 == size)) {
        val = update_crc32(0xffffffffL, 0, 0);
    } else {
        val = update_crc32(crc, (unsigned char *)buf, size);
    }

    return (finish ? val ^ 0xffffffffL : val);
}

bool setPropertyIboot(effaceable_system_t * system, const char * key, uint32_t value)
{
    debug(INFO, "%s := %u", key, value);
    return true;
}

void panicIboot(effaceable_system_t * system, const char * msg)
{
    panic("%s", msg);
}

int vlogfIboot(effaceable_system_t * system, const char * fmt, va_list ap)
{
    return vprintf(fmt, ap);
}

// =============================================================================

extern EffaceableReturn
setupEffaceableSystemContract(effaceable_system_t * system)
{
    system->opaque      = 0;

    system->allocMem    = allocMemIboot;
    system->freeMem     = freeMemIboot;
    system->setMem      = setMemIboot;
    system->moveMem     = moveMemIboot;
    system->cmpMem      = cmpMemIboot;
    system->readRandom  = readRandomIboot;
    system->calcSHA1    = calcSHA1Iboot;
    system->crc32       = crc32Iboot;
    system->setProperty = setPropertyIboot;
    system->panicSys    = panicIboot;
    system->vlogf       = vlogfIboot;

    return kEffaceableReturnSuccess;
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
