/*
 * Copyright (c) 2008-2012 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */

/* ========================================================================== */

/*! 
  @header nand_part

  @abstract definition of device-resident persistent partition table

  @copyright Apple Inc.

  @updated 2012-02-21 
*/

/* ========================================================================== */

#ifndef _NAND_PART_H
#define _NAND_PART_H

#include <sys/types.h>

__BEGIN_DECLS


// =============================================================================
// feature introduction version history

#define kNandPartFeatureInitialVersionMajor            ((uint32_t)0x00000000)
#define kNandPartFeatureFBBTCacheVersionMajor          ((uint32_t)0x00000000)
#define kNandPartFeatureEffaceableVersionMajor         ((uint32_t)0x00000000)
#define kNandPartFeatureIgnoreFBBTVersionMajor         ((uint32_t)0x00000000)
#define kNandPartFeatureTrimDeviceVersionMajor         ((uint32_t)0x00000000)
#define kNandPartFeatureFuneralDirgeVersionMajor       ((uint32_t)0x00000000)
#define kNandPartFeatureCleanEntryFlagsVersionMajor    ((uint32_t)0x00000000)
#define kNandPartFeaturePortableCoreVersionMajor       ((uint32_t)0x00000000)
#define kNandPartFeaturePoolPartitionVersionMajor      ((uint32_t)0x00000001)

#define kNandPartFeatureInitialVersionMinor            ((uint32_t)0x00000001)
#define kNandPartFeatureFBBTCacheVersionMinor          ((uint32_t)0x00000002)
#define kNandPartFeatureEffaceableVersionMinor         ((uint32_t)0x00000003)
#define kNandPartFeatureIgnoreFBBTVersionMinor         ((uint32_t)0x00000004)
#define kNandPartFeatureTrimDeviceVersionMinor         ((uint32_t)0x00000005)
#define kNandPartFeatureFuneralDirgeVersionMinor       ((uint32_t)0x00000006)
#define kNandPartFeatureCleanEntryFlagsVersionMinor    ((uint32_t)0x00000007)
#define kNandPartFeaturePortableCoreVersionMinor       ((uint32_t)0x00000008)
#define kNandPartFeaturePoolPartitionVersionMinor      ((uint32_t)0x00000009)


// =============================================================================
// release version history - for historical reference only

#define kNandPartReleaseNorthstarVersionMajor   kNandPartFeatureFBBTCacheVersionMajor
#define kNandPartReleaseNorthstarVersionMinor   kNandPartFeatureFBBTCacheVersionMinor

#define kNandPartReleaseNorthstar2VersionMajor  kNandPartFeatureFBBTCacheVersionMajor
#define kNandPartReleaseNorthstar2VersionMinor  kNandPartFeatureFBBTCacheVersionMinor

#define kNandPartReleaseApexVersionMajor        kNandPartFeatureIgnoreFBBTVersionMajor
#define kNandPartReleaseApexVersionMinor        kNandPartFeatureIgnoreFBBTVersionMinor

#define kNandPartReleaseBakerVersionMajor       kNandPartFeatureFuneralDirgeVersionMajor
#define kNandPartReleaseBakerVersionMinor       kNandPartFeatureFuneralDirgeVersionMinor

#define kNandPartReleaseDurangoVersionMajor     kNandPartFeatureFuneralDirgeVersionMajor
#define kNandPartReleaseDurangoVersionMinor     kNandPartFeatureFuneralDirgeVersionMinor

#define kNandPartReleaseTellurideVersionMajor   kNandPartFeatureFuneralDirgeVersionMajor
#define kNandPartReleaseTellurideVersionMinor   kNandPartFeatureFuneralDirgeVersionMinor

#define kNandPartReleaseHoodooVersionMajor      kNandPartFeatureFuneralDirgeVersionMajor
#define kNandPartReleaseHoodooVersionMinor      kNandPartFeatureFuneralDirgeVersionMinor

#define kNandPartReleaseSundanceVersionMajor    kNandPartFeaturePoolPartitionVersionMajor
#define kNandPartReleaseSundanceVersionMinor    kNandPartFeaturePoolPartitionVersionMinor


// =============================================================================
// general constants

#define kNandPartSectorSize                     512
#define kNandPartSectorsPerPage                 3
#define kNandPartPageSize                       (kNandPartSectorSize * kNandPartSectorsPerPage)
#define kNandPartBankMax                        16


// =============================================================================
// partition header

typedef struct _NandPartHeader {

        uint32_t                                magic;
        uint32_t                                should_be_zero;
        uint32_t                                versionMajor;
        uint32_t                                versionMinor;
        uint32_t                                generation;
        uint8_t                                 reserved[12];

}  __attribute__((__packed__)) NandPartHeader;

#define kNandPartHeaderMagic                    'Grdn'

#define kNandPartHeaderVersionMajor             kNandPartReleaseSundanceVersionMajor
#define kNandPartHeaderVersionMinor             kNandPartReleaseSundanceVersionMinor

#define kNandPartHeaderMaxSize                  (kNandPartSectorSize / 16)


// =============================================================================
// partition factory BBT cache

typedef struct _NandPartFBBTCacheInfo {

        uint32_t                                blockDepth;
        uint32_t                                entryCount;

}  __attribute__((__packed__)) NandPartFBBTCacheInfo;

#define kNandPartFBBTCacheMaxSize               (kNandPartSectorSize - kNandPartHeaderMaxSize)
#define kNandPartFBBTCacheEntriesMaxSize        (kNandPartFBBTCacheMaxSize - sizeof(NandPartFBBTCacheInfo))

typedef struct _NandPartFBBTCacheEntry {

        uint32_t                                bankNum;
        uint32_t                                blockAddr;

}  __attribute__((__packed__)) NandPartFBBTCacheEntry;

#define kNandPartFBBTCacheEntryCount            (kNandPartFBBTCacheEntriesMaxSize / sizeof(NandPartFBBTCacheEntry))

typedef struct _NandPartFBBTCache {

        NandPartFBBTCacheInfo                   info;
        NandPartFBBTCacheEntry                  entries[kNandPartFBBTCacheEntryCount];

}  __attribute__((__packed__)) NandPartFBBTCache;


// =============================================================================
// partition device info

typedef struct _NandPartDevice {

        uint32_t                                should_be_zero[2];
        uint32_t                                should_be_one;

        uint32_t                                numOfCS;
        uint32_t                                blocksPerCS;
        uint32_t                                pagesPerBlock;
        uint32_t                                sectorsPerPage;
        uint32_t                                spareBytesPerPage;

        uint32_t                                flags;

}  __attribute__((__packed__)) NandPartDevice;

#define kNandPartDeviceFlagNone                 ((uint32_t)0x00000000)
#define kNandPartDeviceFlagNeedsCemetery        (((uint32_t)0x1) << 0)
#define kNandPartDeviceFlagExplicitEntries      (((uint32_t)0x1) << 1)
#define kNandPartDeviceFlagAllowPool            (((uint32_t)0x1) << 2)


// =============================================================================
// cemetery
//
// Where bad blocks go to die.

#define kNandPartCemeteryGate                   ((uint32_t) 0xDeadCafe)
#define kNandPartCemeteryCapacity               256
#define nandPartCemeteryPlotsSize(capacity)     (((capacity) / 8) + (((capacity) % 8) ? 1 : 0))
#define kNandPartCemeteryPlotsSize              nandPartCemeteryPlotsSize(kNandPartCemeteryCapacity)

typedef struct _NandPartCemetery {

        uint32_t                                gate;
        uint16_t                                capacity;
        uint8_t                                 flags;
        uint8_t                                 reserved[3];
        uint8_t                                 plots[kNandPartCemeteryPlotsSize];
        uint8_t                                 locks[2];

}  __attribute__((__packed__)) NandPartCemetery;

#define kNandPartCemeteryFlagIgnoreLocks        ((uint8_t)0x01)


// =============================================================================
// spare blocks

#define kNandPartSpareMaxCount                  64
#define kNandPartSpareMagic                     'spAr'

typedef struct _NandPartSpareLocation {

        uint8_t                                 block[3];
        uint8_t                                 ce;
        
}  __attribute__((__packed__)) NandPartSpareLocation;

typedef struct _NandPartSpare {

        uint32_t                                magic;
        uint16_t                                count;
        uint8_t                                 flags;
        uint8_t                                 reserved[3];
        NandPartSpareLocation                   locations[kNandPartSpareMaxCount];
        uint8_t                                 checks[2];

}  __attribute__((__packed__)) NandPartSpare;

#define kNandPartSpareFlagIgnoreChecks          ((uint8_t)0x01)
#define kNandPartSpareMinExtraCount             4


// =============================================================================
// block substitutions

#define kNandPartSubstMaxCount                   kNandPartSpareMaxCount
#define kNandPartSubstMagic                      'subs'

typedef struct _NandPartSubst {

        uint32_t                                magic;
        uint16_t                                count;
        uint8_t                                 flags;
        uint8_t                                 reserved[3];
        uint8_t                                 maps[kNandPartSubstMaxCount];
        uint8_t                                 checks[2];
        
}  __attribute__((__packed__)) NandPartSubst;

#define kNandPartSubstFlagIgnoreChecks           ((uint8_t)0x01)
#define kNandPartSubstFlagCorrectedChecks        ((uint8_t)0x02)


// =============================================================================
// unused

#define kNandPartUnusedSize                     (kNandPartSectorSize    \
                                                 - sizeof(NandPartDevice) \
                                                 - sizeof(NandPartCemetery) \
                                                 - sizeof(NandPartSpare) \
                                                 - sizeof(NandPartSubst))


// =============================================================================
// partition content

#define kNandPartContentUndefined               0x00000000
#define kNandPartContentNone                    'none'
#define kNandPartContentBootBlock               'boot'
#define kNandPartContentDiagData                'diag'
#define kNandPartContentSyscfg                  'scfg'
#define kNandPartContentFirmware                'firm'
#define kNandPartContentNVRAM                   'nvrm'
#define kNandPartContentFilesystem              'fsys'
#define kNandPartContentEffaceable              'plog'
#define kNandPartContentBadBlockTable           'fbbt'
#define kNandPartContentErased                  0xFFFFFFFF

#define kNandPartContentNameUnformatted         "Unformatted"
#define kNandPartContentNameUnallocated         "Unallocated"
#define kNandPartContentNameUnrecognized        "Unrecognized"
#define kNandPartContentNameBootBlock           "Boot Block"
#define kNandPartContentNameDiagData            "Diagnostic Data"
#define kNandPartContentNameSyscfg              "System Config"
#define kNandPartContentNameNVRAM               "NVRAM"
#define kNandPartContentNameFirmware            "Firmware"
#define kNandPartContentNameFilesystem          "Filesystem"
#define kNandPartContentNameEffaceable          "Effaceable"
#define kNandPartContentNameBadBlockTable       "Bad Block Table"


// =============================================================================
// partition entry

typedef struct _NandPartEntryInfo {

        uint32_t                                content;
        uint32_t                                reserved_0;
        uint32_t                                reserved_1;
        uint32_t                                flags;

}  __attribute__((__packed__)) NandPartEntryInfo;

typedef struct _NandPartEntrySlice {

        uint32_t                                content;
        uint32_t                                offset;
        uint32_t                                depth;
        uint32_t                                flags;

}  __attribute__((__packed__)) NandPartEntrySlice;

typedef struct _NandPartEntryPool {

        uint32_t                                content;
        uint32_t                                width;
        uint32_t                                depth;
        uint32_t                                flags;

}  __attribute__((__packed__)) NandPartEntryPool;

typedef union _NandPartEntry {

        struct {

                uint32_t                        content;
                uint32_t                        reserved_0;
                uint32_t                        reserved_1;
                uint32_t                        flags;

        } __attribute__((__packed__));

        NandPartEntryInfo                       info;
        NandPartEntrySlice                      slice;
        NandPartEntryPool                       pool;

}  NandPartEntry;

#define kNandPartEntryMaxCount                  (kNandPartSectorSize / sizeof(NandPartEntry))

#define kNandPartEntryFlagsNone                 0x00000000

#define kNandPartEntryFlagsTypeInfo             0x00000001
#define kNandPartEntryFlagsTypeSlice            0x00000002
#define kNandPartEntryFlagsTypePool             0x00000008
#define kNandPartEntryFlagsUnpartitioned        0x80000000

#define kNandPartEntryFlagsTypeMask             (kNandPartEntryFlagsTypeInfo        | \
                                                 kNandPartEntryFlagsTypeSlice       | \
                                                 kNandPartEntryFlagsTypePool        | \
                                                 kNandPartEntryFlagsUnpartitioned)

#define kNandPartEntryFlagsIsBulk               0x00000004
#define kNandPartEntryFlagsUsingSLCBlocks       0x00000100
#define kNandPartEntryFlagsUsingFullPages       0x00000200
#define kNandPartEntryFlagsIgnoreFBBT           0x40000000

#define kNandPartEntryFlagsMiscMask             (kNandPartEntryFlagsIsBulk          | \
                                                 kNandPartEntryFlagsUsingSLCBlocks  | \
                                                 kNandPartEntryFlagsUsingFullPages  | \
                                                 kNandPartEntryFlagsIgnoreFBBT)

#define kNandPartEntryFlagsMask                 (kNandPartEntryFlagsTypeMask        | \
                                                 kNandPartEntryFlagsMiscMask)


// =============================================================================
// partition table

typedef struct _NandPartTable {

        NandPartHeader                          header;
        NandPartFBBTCache                       fbbtCache;
        NandPartDevice                          device;
        NandPartCemetery                        cemetery;
        NandPartSpare                           spare;
        NandPartSubst                           subst;
        uint8_t                                 unused[kNandPartUnusedSize];
        NandPartEntry                           entries[kNandPartEntryMaxCount];

}  __attribute__((__packed__)) NandPartTable;

#define kNandPartTableLayoutIsValid             (sizeof(NandPartTable) == kNandPartPageSize)
#define kNandPartTableCopies                    2


// =============================================================================
// partition table incremental differences
//
// incremental difference information about the current partition
// table state that is stored in metadata of other on-nand containers
// (e.g. nvram, effaceable) rather than in partition table itself

typedef struct _NandPartDiffHeader {
    
        uint32_t                                magic;
        uint16_t                                size;
        uint16_t                                sequence;
        uint32_t                                versionMajor;
        uint32_t                                versionMinor;
        uint32_t                                generation;
        uint16_t                                flags;
        uint8_t                                 reserved[10];

}  __attribute__((__packed__)) NandPartDiffHeader;

typedef struct _NandPartDiff {
        
        NandPartDiffHeader                      header;
        NandPartCemetery                        cemetery;
        uint8_t                                 checks[2];

}  __attribute__((__packed__)) NandPartDiff;

#define kNandPartDiffMagic                      'Diff'
#define kNandPartDiffFlagIgnoreChecks           ((uint16_t)0x0001)


// =============================================================================
// boot loader

#define kNandPartLoaderPageCount                128
#define kNandPartLoaderMaxPages                 (kNandPartLoaderPageCount - kNandPartTableCopies - 1)
#define kNandPartLoaderMaxSize                  (kNandPartLoaderMaxPages * kNandPartPageSize)


// =============================================================================
// generally useful macros

#define isMaskSet(_msk, _val) ((_msk) == ((_msk) & (_val)))


__END_DECLS

#endif /* ! _NAND_PART_H */
