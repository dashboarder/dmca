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

#ifndef _APPLE_EFFACEABLE_STORAGE_FORMAT_H
#define _APPLE_EFFACEABLE_STORAGE_FORMAT_H

// =============================================================================

#include <stdint.h>

// =============================================================================

#define kAppleEffaceableStorageMagic 'Face'

#define kAppleEffaceableStorageGenerationUnknown ((uint32_t)0x00000000)
#define kAppleEffaceableStorageGenerationInitial ((uint32_t)0x00000001)
#define kAppleEffaceableStorageGenerationInvalid ((uint32_t)0xFFFFFFFF)

#define kAppleEffaceableStorageVersionMajorUnknown ((uint32_t)0x00000000)
#define kAppleEffaceableStorageVersionMajorInitial ((uint32_t)0x00000001)
#define kAppleEffaceableStorageVersionMajorNonce   ((uint32_t)0x00000001)
#define kAppleEffaceableStorageVersionMajorCurrent kAppleEffaceableStorageVersionMajorNonce
#define kAppleEffaceableStorageVersionMajorInvalid ((uint32_t)0xFFFFFFFF)

#define kAppleEffaceableStorageVersionMinorUnknown ((uint32_t)0x00000000)
#define kAppleEffaceableStorageVersionMinorInitial ((uint32_t)0x00000001)
#define kAppleEffaceableStorageVersionMinorNonce   ((uint32_t)0x00000002)
#define kAppleEffaceableStorageVersionMinorCurrent kAppleEffaceableStorageVersionMajorNonce
#define kAppleEffaceableStorageVersionMinorInvalid ((uint32_t)0xFFFFFFFF)

#define kAppleEffaceableStorageFlagsNone              ((uint32_t)0x00000000)

#define kAppleEffaceableStorageSize         1024
#define kAppleEffaceableStorageSaltSize     16
#define kAppleEffaceableStorageReservedSize 24
#define kAppleEffaceableStorageContentSize  (kAppleEffaceableStorageSize - sizeof(AppleEffaceableStorageHeader))

#define kAppleEffaceableStorageLockerBase   0
#define kAppleEffaceableStorageLockerSize   (kAppleEffaceableStorageContentSize - kAppleEffaceableStorageLockerBase)

// =============================================================================

// All structures defined here are on-media formats.

typedef struct
{
    uint32_t  magic;
    uint32_t  version_major;
    uint32_t  version_minor;
    uint32_t  flags;

} __attribute__((__packed__)) AppleEffaceableStorageNaked;

typedef struct
{
    uint8_t   reserved[kAppleEffaceableStorageReservedSize];
    uint32_t  generation;
    uint32_t  checksum;

} __attribute__((__packed__)) AppleEffaceableStorageVeiled;

typedef struct
{
    AppleEffaceableStorageNaked   naked;
    uint8_t                       salt[kAppleEffaceableStorageSaltSize];
    AppleEffaceableStorageVeiled  veiled;

} __attribute__((__packed__)) AppleEffaceableStorageHeader;

typedef struct
{
    AppleEffaceableStorageHeader  header;
    uint8_t                       content[kAppleEffaceableStorageContentSize];

} __attribute__((__packed__)) AppleEffaceableStorageClone;

typedef struct
{
    uint16_t        magic;
    uint16_t        data_size;
    uint32_t        type_id;
} __attribute__((__packed__)) AppleEffaceableStorageLockerHeader;

#define kAppleEffaceableStorageLockerMagic      0x4c6b  // 'Lk'
#define kAppleEffaceableStorageLockerSentinel   'DONE'


// =============================================================================

#endif /* _APPLE_EFFACEABLE_STORAGE_FORMAT_H */
