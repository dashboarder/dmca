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

#ifndef _EFFACEABLE_NOR_CORE_H_
#define _EFFACEABLE_NOR_CORE_H_

// =============================================================================

__BEGIN_DECLS

// -----------------------------------------------------------------------------

static uint32_t getGroupCount(effaceable_device_t * device);
static uint32_t getUnitsPerGroup(effaceable_device_t * device);
static uint32_t getUnitSize(effaceable_device_t * device);
static EffaceableReturn eraseGroup(effaceable_device_t * device, uint32_t group);
static EffaceableReturn writeUnit(effaceable_device_t * device, const void * buf, uint32_t group, uint32_t unit);
static EffaceableReturn readUnit(effaceable_device_t * device, void * buf, uint32_t group, uint32_t unit);

// -----------------------------------------------------------------------------

static void initContract(effaceable_device_t * device);

// -----------------------------------------------------------------------------

static inline uint32_t getCopiesPerUnit(effaceable_device_t * device);
static inline uint32_t getCopySize(effaceable_device_t * device);

// -----------------------------------------------------------------------------

static inline uint32_t getRegionCount(effaceable_device_t * device);
static inline uint32_t getRegionSize(effaceable_device_t * device, uint32_t index);
static inline EffaceableReturn eraseRegion(effaceable_device_t * device, uint32_t index, uint32_t length);
static inline uint32_t writeRegion(effaceable_device_t * device, uint32_t index, const void * buf, uint32_t length);
static inline bool readRegion(effaceable_device_t * device, uint32_t index, void * buf, uint32_t length);

// -----------------------------------------------------------------------------

static inline void * allocMem(effaceable_device_t * device, uint32_t size);
static inline void freeMem(effaceable_device_t * device, void * buf, uint32_t size);
static inline void * setMem(effaceable_device_t * device, void * buf, uint8_t val, uint32_t size);
static inline void * moveMem(effaceable_device_t * device, void * dst, const void * src, uint32_t size);
static inline int cmpMem(effaceable_device_t * device, const void * lhs, const void * rhs, uint32_t size);
static inline void readRandom(effaceable_device_t * device, void * buf, uint32_t size);
static inline uint32_t crc32Sys(effaceable_device_t * device, uint32_t crc, const void * buf, uint32_t size, bool finish);
static inline bool setPropertySys(effaceable_device_t * device, const char * key, uint32_t value);
static inline void panicSys(effaceable_device_t * device, const char * msg);

static int logf(effaceable_device_t * device, const char * fmt, ...);

// -----------------------------------------------------------------------------

static void initContext(effaceable_device_t * device, effaceable_system_t * system, effaceable_nor_hal_t * nor, effaceable_storage_t * storage);

// -----------------------------------------------------------------------------

__END_DECLS

// =============================================================================

#endif /* _EFFACEABLE_NOR_CORE_H_ */
