/*
 * Copyright (c) 2010-2012 Apple Inc. All rights reserved.
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
  @header nand_part_core

  @abstract prototypes for internal implementation of nand_part_core.c

  @copyright Apple Inc.

  @updated 2012-02-21
*/

/* ========================================================================== */

#ifndef _NAND_PART_CORE_H
#define _NAND_PART_CORE_H


// =============================================================================
// XXX - pass this in from provider rather than hard-code copy

#define AND_SPARE_TYPE_REGION_BFN      (0x40) // allowing BFN to use 4 bits for types


// =============================================================================
// partition mapping
//
// Note that this is a calculated dynamic data structure only.

#define kNandPartMappingMaxCount                kNandPartEntryMaxCount

typedef struct _NandPartMapping {

        uint32_t                                total;
        uint32_t                                counts[kNandPartEntryMaxCount];
        uint32_t                                offsets[kNandPartEntryMaxCount];

} NandPartMapping;


// =============================================================================
// mapped block replacement
//
// Note that this is a dynamic data structure only.  The array of
// these structs is effectively an expanded version of the persistent
// NandPartSubst information, forming a fully populated table indexing
// from block map number to spare index rather the other way around
// for ease and speed of use when remapping in the runtime context.

#define kNandPartReplacementMaxCount            kNandPartCemeteryCapacity
#define kNandPartReplacementInvalidIndex        ((uint32_t)-1)

typedef struct _NandPartReplacement {

        bool                                    isValid;
        bool                                    isReplaced;
        uint32_t                                spareIndex;

} NandPartReplacement;


// =============================================================================
// state of portable core providing interface for nand partitioning logic

typedef struct _NandPartCore {

        NandPartInterface                       interface;
        NandPartProvider *                      provider;

        NandPartEntry                           rawMedia;

        bool                                    isPTabValid;
        bool                                    isBootPartErased;
        bool                                    isFBBTCached;
        uint32_t                                bootIdx;
        uint16_t                                diffSequence[kNandPartEntryMaxCount];

        uint8_t *                               meta;
        NandPartTable *                         ptab;

        NandPartMapping                         mapping;
        NandPartReplacement                     replacements[kNandPartReplacementMaxCount];

} NandPartCore;


/* ========================================================================== */

struct partition_spec {
        uint32_t  type;
        uint32_t  content;
        uint32_t  depth;
        uint32_t  width; /* optional, depending on type */
};


/* ========================================================================== */

#define kNandPartCountInvalid ((uint32_t)-1)
#define kNandPartBlockInvalid ((uint32_t)-1)
#define kNandPartCEInvalid    ((uint32_t)-1)


/* ========================================================================== */

__BEGIN_DECLS


/* ========================================================================== */

#pragma mark - Provider Service Wrappers

static bool hostCreatePartitionDevice(NandPartCore * npc, NandPartEntry * entry, unsigned idx);
static bool hostPublishPartitionDevice(NandPartCore * npc, unsigned idx);
static bool hostReadBootPage(NandPartCore * npc, uint32_t ce, uint32_t addr, void * buf, bool * is_clean);
static bool hostReadFullPage(NandPartCore * npc, uint32_t ce, uint32_t addr, void * buf, void * meta, bool * is_clean);
static bool hostWriteBootPage(NandPartCore * npc, uint32_t ce, uint32_t addr, void * buf);
static bool hostWriteFullPage(NandPartCore * npc, uint32_t ce, uint32_t addr, void * buf, void * meta);
static bool hostEraseBlock(NandPartCore * npc, uint32_t ce, uint32_t addr, uint32_t block);
static void hostLogMessage(NandPartCore * npc, NandPartLogLevel lvl, const char * fmt, ...);


/* ========================================================================== */

#pragma mark - Initialization

static NandPartCore * withNandPartCore(NandPartProvider * provider);
static bool createUnformattedPartitionDevice(NandPartCore * npc);
static bool createPartitionDevices(NandPartCore * npc, bool isNewborn);


/* ========================================================================== */

#pragma mark - Version-Dependent Fixups

static void makeEntryTypesExplicit(NandPartCore * npc);
static void addIgnoreFBBTFlag(NandPartCore * npc);
static void cleanEntryFlags(NandPartCore * npc, NandPartEntry * entry, bool isNewborn);


/* ========================================================================== */

#pragma mark - Mature Device Preparation

static bool prepareMatureState(NandPartCore * npc, bool *isReborn);
static void setPartitionTableProperties(NandPartCore * npc, bool isNewborn, bool isReborn);


/* ========================================================================== */

#pragma mark - Reborn/Newborn Device Preparation

static bool prepareRebornState(NandPartCore * npc);
static bool prepareNewbornState(NandPartCore * npc, bool isReborn);
static uint32_t countSparesNeeded(NandPartCore * npc, bool useWorstCase);
static uint32_t countSparesExtra(NandPartCore * npc);
static uint32_t scrubMappedPartitionsForStillborn(NandPartCore * npc);
static bool scrubBlockForStillborn(NandPartCore * npc, uint32_t part, uint32_t bank, uint32_t block, void * patternBuf);


/* ========================================================================== */

#pragma mark - Initial Format/Partitioning

static bool formatWithSpecs(NandPartCore * npc);
static void initPartitionTable(NandPartCore * npc);
static uint32_t majorVersion(NandPartCore * npc);
static bool addPartition(NandPartCore *npc, NandPartSpec *spec, uint32_t index, bool * haveBulk);


/* ========================================================================== */

#pragma mark - Runtime Parameters

static bool isBootPartErasable(NandPartCore * npc);
static bool isBootPartWritable(NandPartCore * npc);
static bool isFormatEnabled(NandPartCore * npc);
static bool isDebugEnabled(NandPartCore * npc);
static bool isPoolAllowed(NandPartCore * npc);
static uint32_t maxMajorVersion(NandPartCore * npc);


/* ========================================================================== */

#pragma mark - Miscellaneous Global Information

static bool isPpnDevice(NandPartCore * npc);
static bool findBootPartIndex(NandPartCore * npc, uint32_t * partIndex);
static bool isEntryIndexValid(NandPartCore * npc, uint32_t partIndex, bool allowRawIndex);
static bool isRawEntryIndex(NandPartCore * npc, uint32_t partIndex);
static bool allowPoolPartitions(NandPartCore * npc);


/* ========================================================================== */

#pragma mark - Partition Entry Information

static const char * entryContentName(NandPartCore * npc, uint32_t partIndex);
static bool areFlagsSet(NandPartCore * npc, uint32_t partIndex, uint32_t flagMask);
static bool usingSLCBlocks(NandPartCore * npc, uint32_t partIndex);
static bool usingFullPages(NandPartCore * npc, uint32_t partIndex);
static uint32_t getBlockOffset(NandPartCore * npc, uint32_t partIndex);


/* ========================================================================== */

#pragma mark - Virtual to Physical Address Calculation

static uint32_t mapNandBlock(NandPartCore * npc, uint32_t partIndex, uint32_t partBank, uint32_t partBlock, bool allowSpare);
static uint32_t mapNandCE(NandPartCore * npc, uint32_t partIndex, uint32_t partBank, uint32_t partBlock, bool allowSpare);
static uint32_t calcNandAddress(NandPartCore * npc, uint32_t nandBlock, uint32_t pageInBlock, bool useSLC);

#if NAND_PART_CORE_ORPHAN
static uint32_t calcNandBlock(NandPartCore * npc, uint32_t nandAddress, bool useSLC);
#endif


/* ========================================================================== */

#pragma mark - Meta Buffer Proxy

static uint8_t * getMetaBuf(NandPartCore * npc);
static uint8_t * resetMetaBuf(NandPartCore * npc);
static uint8_t * endorseMetaBuf(NandPartCore * npc, uint32_t partIndex);
static void verifyMetaBuf(NandPartCore * npc, uint32_t partIndex);


/* ========================================================================== */

#pragma mark - Block Replacement

static bool initReplacements(NandPartCore * npc);
static bool refreshReplacements(NandPartCore * npc);
static bool validateSpare(NandPartCore * npc, NandPartSpare * spare);
static bool validateSubst(NandPartCore * npc, NandPartSubst * subst);
static bool isBlockReplaced(NandPartCore * npc, uint32_t partIndex, uint32_t partBank, uint32_t partBlock, uint32_t * spareIndex);
static NandPartReplacement * getMappedReplacement(NandPartCore * npc, uint32_t map);
static NandPartSpareLocation * getSpareLocation(NandPartCore * npc, uint32_t spareIndex);
static uint32_t getSpareLocationBlock(NandPartCore * npc, NandPartSpareLocation * location);
static void setSpareLocationBlock(NandPartCore * npc, NandPartSpareLocation * location, uint32_t block);
static bool substituteSparesForBadBlocks(NandPartCore * npc);
static void initMapping(NandPartCore * npc);
static bool isPartitionMapped(NandPartCore * npc, uint32_t partIndex);
static uint32_t calcPartitionMapCount(NandPartCore * npc, uint32_t partIndex);
static uint32_t getPartitionMapOffset(NandPartCore * npc, uint32_t partIndex);
static uint32_t getTotalMapCount(NandPartCore * npc);
static uint32_t calcBlockMap(NandPartCore * npc, uint32_t partIndex, uint32_t partBank, uint32_t partBlock, bool allowSpare);

#if NAND_PART_CORE_ORPHAN
static uint32_t getPartitionMapCount(NandPartCore * npc, uint32_t partIndex);
#endif


/* ========================================================================== */

#pragma mark - Bad Block Management

static bool initCemetery(NandPartCore * npc, bool isNewborn);
static void fletcher16(NandPartCore * npc, uint8_t *checkA, uint8_t *checkB, const void *buf, size_t len);
static bool isCemeterySafe(NandPartCore * npc, const NandPartCemetery *cemetery);
static void changeCemeteryLocks(NandPartCore * npc, NandPartCemetery * cemetery);
static bool mergePartitionCemetery(NandPartCore * npc, uint32_t partIndex, const NandPartCemetery * partCemetery, NandPartCemetery * mainCemetery);
static void markBlockDead(NandPartCore * npc, uint32_t partIndex, uint32_t partBank, uint32_t partBlock, bool allowSpare);
static bool isBlockDead(NandPartCore * npc, uint32_t partIndex, uint32_t partBank, uint32_t partBlock, bool allowSpare);
static NandPartFBBTCache * getFBBTCache(NandPartCore * npc);
static bool isBlockFactoryBad(NandPartCore * npc, uint32_t partIndex, uint32_t partBank, uint32_t partBlock, bool allowSpare);
static void cacheFactoryBadBlockTable(NandPartCore * npc);


/* ========================================================================== */

#pragma mark - Partition Table I/O

static bool readPTab(NandPartCore * npc);
static bool validatePTab(NandPartCore * npc, const NandPartTable * ptab);
static bool writePTab(NandPartCore * npc);


/* ========================================================================== */

#pragma mark - Partition Table Diffs

static void initDiffSequence(NandPartCore * npc);
static bool validatePTabDiff(NandPartCore * npc, NandPartDiff * diff, unsigned size);


/* ========================================================================== */

#pragma mark - Structure Dumping

static void dumpNandGeometry(NandPartCore * npc, NandPartGeometry * geometry);
static void dumpPTabHeader(NandPartCore * npc, NandPartHeader * header);
static void dumpCemetery(NandPartCore * npc, NandPartCemetery * cemetery);
static void dumpSpare(NandPartCore * npc, NandPartSpare * spare);
static void dumpSubst(NandPartCore * npc, NandPartSubst * subst);
static void dumpEntries(NandPartCore * npc, NandPartEntry * entries);
static void dumpPTab(NandPartCore * npc, NandPartTable * ptab);
static void dumpMapping(NandPartCore * npc, NandPartMapping * mapping);
static void dumpReplacements(NandPartCore * npc, NandPartReplacement * replacements);
static void dumpTranslationTables(NandPartCore * npc);


/* ========================================================================== */

#pragma mark - Interface Implementation

static const NandPartEntry * getEntry(NandPartCore * npc, uint32_t partIndex);
static uint32_t getBankWidth(NandPartCore * npc, uint32_t partIndex);
static uint32_t getBlockDepth(NandPartCore * npc, uint32_t partIndex);
static uint32_t getPagesPerBlock(NandPartCore * npc, uint32_t partIndex);
static uint32_t getBytesPerPage(NandPartCore * npc, uint32_t partIndex);
static bool isBlockBad(NandPartCore * npc, uint32_t partIndex, uint32_t partBank, uint32_t partBlock, bool allowSpare);
static bool readPage(NandPartCore * npc, uint32_t partIndex, uint32_t partBank, uint32_t partBlock, uint32_t pageInBlock, void * buf, bool * isClean, bool allowSpare);
static bool writePage(NandPartCore * npc, uint32_t partIndex, uint32_t partBank, uint32_t partBlock, uint32_t pageInBlock, void * buf, bool allowSpare);
static bool eraseBlock(NandPartCore * npc, uint32_t partIndex, uint32_t partBank, uint32_t partBlock, bool allowSpare);
static bool requestPTabDiff(NandPartCore * npc, uint32_t partIndex, void * buf, unsigned size);
static bool providePTabDiff(NandPartCore * npc, uint32_t partIndex, const void * buf, unsigned size);
static bool formatPTab(NandPartCore * npc);
static unsigned getPTabBytes(NandPartCore * npc, void * buf, unsigned size);
static bool loadPartitionTable(NandPartCore * npc);
static bool eraseBootPartition(NandPartCore * npc);
static bool writeBootPartition(NandPartCore * npc, uint8_t * llbBuf, unsigned llbSize);


/* ========================================================================== */

#pragma mark - Unlocked External Interface Wrappers

static const NandPartEntry * getEntryExternal(void * core, uint32_t partIndex);
static uint32_t getBankWidthExternal(void * core, uint32_t partIndex);
static uint32_t getBlockDepthExternal(void * core, uint32_t partIndex);
static uint32_t getPagesPerBlockExternal(void * core, uint32_t partIndex);
static uint32_t getBytesPerPageExternal(void * core, uint32_t partIndex);


/* ========================================================================== */

#pragma mark - Locked External Interface Wrappers

static bool loadPartitionTableExternal(void * core);
static bool eraseBootPartitionExternal(void * core);
static bool writeBootPartitionExternal(void * core, void * llbBuf, unsigned llbSize);
static bool isBlockBadExternal(void * core, uint32_t partIndex, uint32_t partBank, uint32_t partBlock);
static bool readPageExternal(void * core, uint32_t partIndex, uint32_t partBank, uint32_t partBlock, uint32_t pageInBlock, void * buf, bool * isClean);
static bool writePageExternal(void * core, uint32_t partIndex, uint32_t partBank, uint32_t partBlock, uint32_t pageInBlock, void * buf);
static bool eraseBlockExternal(void * core, uint32_t partIndex, uint32_t partBank, uint32_t partBlock);
static bool requestPTabDiffExternal(void * core, uint32_t partIndex, void * buf, unsigned size);
static bool providePTabDiffExternal(void * core, uint32_t partIndex, const void * buf, unsigned size);
static bool formatPTabExternal(void * core);
static unsigned getPTabBytesExternal(void * core, void * buf, unsigned size);


/* ========================================================================== */

__END_DECLS

#endif /* _NAND_PART_CORE_H */
