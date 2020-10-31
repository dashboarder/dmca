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

#ifndef _EFFACEABLE_NAND_CORE_H_
#define _EFFACEABLE_NAND_CORE_H_

// =============================================================================

// reserve pages at beginning and end of block for future metadata
#define kAppleEffaceableNANDOpeningMetaPages  16
#define kAppleEffaceableNANDClosingMetaPages  16
#define kAppleEffaceableNANDTotalMetaPages    (kAppleEffaceableNANDOpeningMetaPages + kAppleEffaceableNANDClosingMetaPages)

#define kAppleEffaceableNANDMetaOpenMagic  'open'
#define kAppleEffaceableNANDMetaCloseMagic 'clos'

#define kAppleEffaceableNANDMetaVersionMajorUnknown  ((uint32_t)0x00000000)
#define kAppleEffaceableNANDMetaVersionMajorInitial  ((uint32_t)0x00000001)
#define kAppleEffaceableNANDMetaVersionMajorBungled  ((uint32_t)0x00000002)
#define kAppleEffaceableNANDMetaVersionMajorPTabDiff ((uint32_t)0x00000002)
#define kAppleEffaceableNANDMetaVersionMajorCurrent  kAppleEffaceableNANDMetaVersionMajorPTabDiff
#define kAppleEffaceableNANDMetaVersionMajorInvalid  ((uint32_t)0xFFFFFFFF)

#define kAppleEffaceableNANDMetaVersionMinorUnknown  ((uint32_t)0x00000000)
#define kAppleEffaceableNANDMetaVersionMinorInitial  ((uint32_t)0x00000001)
#define kAppleEffaceableNANDMetaVersionMinorBungled  ((uint32_t)0x00000002)
#define kAppleEffaceableNANDMetaVersionMinorPTabDiff ((uint32_t)0x00000003)
#define kAppleEffaceableNANDMetaVersionMinorCurrent  kAppleEffaceableNANDMetaVersionMinorPTabDiff
#define kAppleEffaceableNANDMetaVersionMinorInvalid  ((uint32_t)0xFFFFFFFF)

#define kAppleEffaceableNANDMetaFlagsNone            ((uint32_t)0x00000000)
#define kAppleEffaceableNANDMetaFlagsHasPTabDiff     ((uint32_t)0x00000001)

// =============================================================================

// All structures defined here are on-media formats.

typedef struct
{
    uint32_t  magic;
    uint32_t  version_major;
    uint32_t  version_minor;
    uint32_t  flags;

} __attribute__((__packed__)) AppleEffaceableNANDMetaHeader;

#define kAppleEffaceableNANDMetaHeaderSize   sizeof(AppleEffaceableNANDMetaHeader)

typedef struct
{
    AppleEffaceableNANDMetaHeader  header;
    uint8_t                        content[0];

} __attribute__((__packed__)) AppleEffaceableNANDMeta;

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

static void openBlock(effaceable_device_t * device, uint32_t bank, uint32_t block);
static void closeBlock(effaceable_device_t * device, uint32_t bank, uint32_t block);
static void prepOpenCloseMeta(effaceable_device_t * device, uint32_t magic, void * buf);
static void prepCloseContent(effaceable_device_t * device, void * buf);

// -----------------------------------------------------------------------------

static uint32_t mapGroupToBank(effaceable_device_t * device, uint32_t group);
static uint32_t mapGroupToBlock(effaceable_device_t * device, uint32_t group);
static uint32_t mapUnitToPage(effaceable_device_t * device, uint32_t unit);

// -----------------------------------------------------------------------------

static uint32_t getCopiesSize(effaceable_device_t * device);
static void cleanBuffers(effaceable_device_t * device);
static void whitenExtra(effaceable_device_t * device, void * buf);
static void whitenMeta(effaceable_device_t * device, void * buf);
static void foundCurrentCloneInGroup(effaceable_device_t * device, uint32_t group);
static int compareMetaVersions(effaceable_device_t * device, uint32_t major1, uint32_t minor1, uint32_t major2, uint32_t minor2);
static uint32_t getMetaContentSize(effaceable_device_t * device);

// -----------------------------------------------------------------------------

static void initContract(effaceable_device_t * device);

// =============================================================================

static inline void * getBuf(effaceable_device_t * device);
static inline void * setBuf(effaceable_device_t * device, void * buf);

// -----------------------------------------------------------------------------

static inline uint32_t getCopiesPerUnit(effaceable_device_t * device);
static inline uint32_t getCopySize(effaceable_device_t * device);

// -----------------------------------------------------------------------------

static inline uint32_t getPageSize(effaceable_device_t * device);
static inline uint32_t getPageCount(effaceable_device_t * device);
static inline uint32_t getBlockCount(effaceable_device_t * device);
static inline uint32_t getBankCount(effaceable_device_t * device);
static inline bool isBlockBad(effaceable_device_t * device, uint32_t bank, uint32_t block);
static inline EffaceableReturn eraseBlock(effaceable_device_t * device, uint32_t bank, uint32_t block);
static inline EffaceableReturn writePage(effaceable_device_t * device, uint32_t bank, uint32_t block, uint32_t page, const void * buf, uint32_t length);
static inline EffaceableReturn readPage(effaceable_device_t * device, uint32_t bank, uint32_t block, uint32_t page, void * buf, uint32_t length);
static inline bool requestPartitionTableDiff(effaceable_device_t * device, void * buf, uint32_t length);
static inline bool providePartitionTableDiff(effaceable_device_t * device, void * buf, uint32_t length);

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

static void initContext(effaceable_device_t * device, effaceable_system_t * system, effaceable_nand_hal_t * nand, effaceable_storage_t * storage);

// -----------------------------------------------------------------------------

__END_DECLS

// =============================================================================

#endif /* _EFFACEABLE_NAND_CORE_H_ */
