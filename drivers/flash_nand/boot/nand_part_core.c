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
  @abstract portable core for (mostly boot-related) nand partitioning logic

  @copyright Apple Inc.

  @updated 2012-02-21
*/

/* ========================================================================== */

#include <stdarg.h>
#include <stdbool.h>

#include "nand_part.h"
#include "nand_part_interface.h"
#include "nand_part_core.h"

#define DEBUG_ASSERT_COMPONENT_NAME_STRING "nand_part_core"
#include <AssertMacros.h>


/* ========================================================================== */

#pragma mark - Misc Macros

#ifndef NULL
#define NULL ((void *)0)
#endif

#define byteN(_x, _idx) ((uint8_t)((_x) >> (8 * _idx)))
#define bool2Char(_b) ((_b) ? 'T' : 'F')

#define ptabPrecedesFeature(ptab, feature)                              \
        (ptab->header.versionMinor < kNandPartFeature##feature##VersionMinor)

#define _hostExec(npc, fn, args...) npc->provider->fn(npc->provider->context, ##args)

#define funcLocked(_lval, _func, _core, ...)                    \
        do {                                                    \
                NandPartCore * npc = (NandPartCore *)(_core);   \
                hostLockInterface(npc);                         \
                *(_lval) = (_func)(npc, ##__VA_ARGS__);         \
                hostUnlockInterface(npc);                       \
        } while (0)

#define min(x, y) (((x) < (y)) ? (x) : (y))


/* ========================================================================== */

#pragma mark - Logging Infrastructure

const char * kNandPartLogTagBug   = "BUG";
const char * kNandPartLogTagError = "ERR";
const char * kNandPartLogTagInfo  = "INF";
const char * kNandPartLogTagWarn  = "WRN";
const char * kNandPartLogTagDebug = "DBG";
const char * kNandPartLogTagSpew  = "SPW";

static const char * kLogOrigin = "nand_part_core";

#define _logCont(npc, fac, fmt, args...) hostLogMessage(npc, kNandPartLogLevel##fac, fmt, ##args)
#define _logHead(npc, fac, fmt, args...) _logCont(npc, fac, "[%s:%s@%4d] " fmt, kLogOrigin, kNandPartLogTag##fac, __LINE__, ##args)
#define _logLine(npc, fac, fmt, args...) _logHead(npc, fac, fmt "\n", ##args)

#if NAND_PART_EXTENDED_LOGGING

# define bug(npc, args...)       _logLine(npc, Bug,   ##args)
# define error(npc, args...)     _logLine(npc, Error, ##args)

# define info(npc, args...)      _logLine(npc, Info,  ##args)

# define warn(npc, args...)      _logLine(npc, Warn,  ##args)
# define debugCont(npc, args...) _logCont(npc, Debug, ##args)
# define debugHead(npc, args...) _logHead(npc, Debug, ##args)
# define debug(npc, args...)     _logLine(npc, Debug, ##args)

# if NAND_PART_LOCAL_DEBUG_ONLY
#  define spew(npc, args...)     _logLine(npc, Spew,  ##args)
# else
#  define spew(npc, args...)     /* do nothing */
# endif

#else /* NAND_PART_EXTENDED_LOGGING */

# define bug(npc, args...)       _logLine(npc, Bug,   "")
# define error(npc, args...)     _logLine(npc, Error, "")

# define info(npc, args...)      /* do nothing */

# define warn(npc, args...)      _logLine(npc, Warn, "")
# define debugCont(npc, args...) /* do nothing */
# define debugHead(npc, args...) /* do nothing */
# define debug(npc, args...)     /* do nothing */

# define spew(npc, args...)      /* do nothing */

#endif /* NAND_PART_EXTENDED_LOGGING */


// =============================================================================

#pragma mark - Provider Service Wrappers

#define hostLockInterface(npc)                                  _hostExec(npc, lock_interface)
#define hostUnlockInterface(npc)                                _hostExec(npc, unlock_interface)

#define hostAllocMem(npc, size, align)                          _hostExec(npc, alloc_mem, size, align)
#define hostCompareMem(npc, left, right, size)                  _hostExec(npc, compare_mem, left, right, size)
#define hostMoveMem(npc, dst, src, size)                        _hostExec(npc, move_mem, dst, src, size)
#define hostSetMem(npc, mem, val, size)                         _hostExec(npc, set_mem, mem, val, size)
#define hostFreeMem(npc, mem, size)                             _hostExec(npc, free_mem, mem, size)

#define hostReadRandom(npc, buf, size)                          _hostExec(npc, read_random, buf, size)

#define hostIsBlockFactoryBad(npc, ce, block)                   _hostExec(npc, is_block_factory_bad, ce, block)
#define hostValidateSpareList(npc, ces, blocks, count)          _hostExec(npc, validate_spare_list, ces, blocks, count)
#define hostRequestSpareBlocks(npc, ces, blocks, count, qty)    _hostExec(npc, request_spare_blocks, ces, blocks, count, qty)

#define hostSetDataProperty(npc, key, buf, size)                _hostExec(npc, set_data_property, key, buf, size)
#define hostSetNumberProperty(npc, key, num, bits)              _hostExec(npc, set_number_property, key, num, bits)
#define hostSetStringProperty(npc, key, cstring)                _hostExec(npc, set_string_property, key, cstring)
#define hostSetBooleanProperty(npc, key, val)                   _hostExec(npc, set_boolean_property, key, val)

#define hostWaitForFBBTService(npc)                             _hostExec(npc, wait_for_fbbt_service)

static bool
hostCreatePartitionDevice(NandPartCore * npc, NandPartEntry * entry, unsigned idx)
{
        bool ok = _hostExec(npc, create_partition_device, entry, idx);
        if (ok) {
                info(npc, "created partition %u as '%s'", idx, entryContentName(npc, idx));
        } else {
                bug(npc, "failed to create device for partition %u with '%s' content", idx, entryContentName(npc, idx));
        }
        return ok;
}

static bool
hostPublishPartitionDevice(NandPartCore * npc, unsigned idx)
{
        bool ok = _hostExec(npc, publish_partition_device, idx);
        if (ok) {
                info(npc, "published partition %u", idx);
        } else {
                bug(npc, "failed to publish partition %u", idx);
        }
        return ok;
}

static bool 
hostReadBootPage(NandPartCore * npc, uint32_t ce, uint32_t addr, void * buf, bool * is_clean)
{
        bool ok = false;
        if (npc->provider->params.isHostTraceEnabled) {
                debug(npc, "hostReadBootPage(%u, 0x%08x, %p, %p)", ce, addr, buf, is_clean);
        }
        ok = _hostExec(npc, read_boot_page, ce, addr, buf, is_clean);
        if (npc->provider->params.isHostTraceEnabled) {
                debug(npc, "hostReadBootPage => %c (%c)", bool2Char(ok), is_clean ? bool2Char(*is_clean) : '-');
        }
        return ok;
}

static bool 
hostReadFullPage(NandPartCore * npc, uint32_t ce, uint32_t addr, void * buf, void * meta, bool * is_clean)
{
        bool ok = false;
        if (npc->provider->params.isHostTraceEnabled) {
                debug(npc, "hostReadFullPage(%u, 0x%08x, %p, %p, %p)", ce, addr, buf, meta, is_clean);
        }
        ok = _hostExec(npc, read_full_page, ce, addr, buf, meta, is_clean);
        if (npc->provider->params.isHostTraceEnabled) {
                debug(npc, "hostReadFullPage => %c (%c)", bool2Char(ok), is_clean ? bool2Char(*is_clean) : '-');
        }
        return ok;
}

static bool 
hostWriteBootPage(NandPartCore * npc, uint32_t ce, uint32_t addr, void * buf)
{
        bool ok = false;
        if (npc->provider->params.isHostTraceEnabled) {
                debug(npc, "hostWriteBootPage(%u, 0x%08x, %p)", ce, addr, buf);
        }
        ok = _hostExec(npc, write_boot_page, ce, addr, buf);
        if (npc->provider->params.isHostTraceEnabled) {
                debug(npc, "hostWriteBootPage => %c", bool2Char(ok));
        }
        return ok;
}

static bool 
hostWriteFullPage(NandPartCore * npc, uint32_t ce, uint32_t addr, void * buf, void * meta)
{
        bool ok = false;
        if (npc->provider->params.isHostTraceEnabled) {
                debug(npc, "hostWriteFullPage(%u, 0x%08x, %p, %p)", ce, addr, buf, meta);
        }
        ok = _hostExec(npc, write_full_page, ce, addr, buf, meta);
        if (npc->provider->params.isHostTraceEnabled) {
                debug(npc, "hostWriteFullPage => %c", bool2Char(ok));
        }
        return ok;
}

static bool 
hostEraseBlock(NandPartCore * npc, uint32_t ce, uint32_t addr, uint32_t block)
{
        bool ok = false;
        if (npc->provider->params.isHostTraceEnabled) {
                debug(npc, "hostEraseBlock(%u, 0x%08x)", ce, addr);
        }
        ok = _hostExec(npc, erase_block, ce, addr, block);
        if (npc->provider->params.isHostTraceEnabled) {
                debug(npc, "hostEraseBlock => %c", bool2Char(ok));
        }
        return ok;
}

static void
hostLogMessage(NandPartCore * npc, NandPartLogLevel lvl, const char * fmt, ...)
{
        va_list ap;

        va_start(ap, fmt);
        _hostExec(npc, vlogf_message, lvl, fmt, ap);
        va_end(ap);
}


/* ========================================================================== */

#pragma mark - Initialization

extern bool
nand_part_init(NandPartInterface ** interface, NandPartProvider * provider)
{
        NandPartCore * npc = NULL;

        require((NULL != provider) && (NULL != provider->context), fail_mute);
        require((NULL != interface) && (NULL == *interface), fail_params);

        npc = withNandPartCore(provider);
        require(NULL != npc, fail_construct);

        debug(npc, "nand_part_init()");
        dumpNandGeometry(npc, &(npc->provider->geometry));

        // initialize artificial "raw media" entry
        npc->rawMedia.content    = kNandPartContentUndefined;
        npc->rawMedia.reserved_0 = 0;
        npc->rawMedia.reserved_1 = 0;
        npc->rawMedia.flags      = kNandPartEntryFlagsUnpartitioned;

        // allocate and prepare scratch meta buffer
        npc->meta = (uint8_t *)hostAllocMem(npc, npc->provider->geometry.bytesPerFullPageMeta, false);
        require(NULL != npc->meta, fail_alloc);
        resetMetaBuf(npc);

        // allocate and prepare ptab buffer
        npc->ptab = (NandPartTable *)hostAllocMem(npc, npc->provider->geometry.bytesPerBootPage, true);
        require(NULL != npc->ptab, fail_alloc);
        hostSetMem(npc, npc->ptab, 0, npc->provider->geometry.bytesPerBootPage);

        npc->bootIdx = ~0;
        npc->isBootPartErased = false;
        npc->isPTabValid = false;

        *interface = &npc->interface;
        return(true);

fail_alloc:
        bug(npc, "alloc failed");
        if (NULL != npc->meta) {
                hostFreeMem(npc, npc->meta, npc->provider->geometry.bytesPerFullPageMeta);
                npc->meta = NULL;
        }
        if (NULL != npc->ptab) {
                hostFreeMem(npc, npc->ptab, npc->provider->geometry.bytesPerBootPage);
                npc->ptab = NULL;
        }
        return(false);

fail_construct:
        // XXX - can't use log macros yet
        //error(npc, "unable to contruct NandPartCore");
        return(false);

fail_params:
        // XXX - can't use log macros yet
        //error(npc, "bad params");
        return(false);

fail_mute:
        // with no provider/context, we're unable to even whine about nature of failure
        return(false);
}

static NandPartCore *
withNandPartCore(NandPartProvider * provider)
{
        // Until the NandPartCore structure is initialized, none of the
        // convenience macros for host services can be expected to work.

        NandPartCore * npc = NULL;
        npc = (NandPartCore *) provider->alloc_mem(provider->context, sizeof(NandPartCore), false);
        if (NULL != npc) {

                provider->set_mem(provider->context, npc, 0, sizeof(NandPartCore));
        
                npc->provider                        = provider;
                npc->interface.core                  = npc;

                npc->interface.get_entry             = getEntryExternal;
                npc->interface.get_bank_width        = getBankWidthExternal;
                npc->interface.get_block_depth       = getBlockDepthExternal;
                npc->interface.get_pages_per_block   = getPagesPerBlockExternal;
                npc->interface.get_bytes_per_page    = getBytesPerPageExternal;

                npc->interface.load_ptab             = loadPartitionTableExternal;
                npc->interface.erase_boot_partition  = eraseBootPartitionExternal;
                npc->interface.write_boot_partition  = writeBootPartitionExternal;
                npc->interface.is_block_bad          = isBlockBadExternal;   
                npc->interface.read_page             = readPageExternal;
                npc->interface.write_page            = writePageExternal;
                npc->interface.erase_block           = eraseBlockExternal;
                npc->interface.request_ptab_diff     = requestPTabDiffExternal;
                npc->interface.provide_ptab_diff     = providePTabDiffExternal;
                npc->interface.format_ptab           = formatPTabExternal;
                npc->interface.get_ptab_bytes        = getPTabBytesExternal;
        }

        return(npc);
}

static bool
createUnformattedPartitionDevice(NandPartCore * npc)
{
        return (hostCreatePartitionDevice(npc, &npc->rawMedia, kNandPartEntryMaxCount) && 
                hostPublishPartitionDevice(npc, kNandPartEntryMaxCount));
}

static bool
createPartitionDevices(NandPartCore * npc, bool isNewborn)
{
        bool       ok = false;
        uint32_t   i;
        uint32_t   created_count = 0;
        uint32_t   created_mask = 0x0;
        uint32_t   publish_count = 0;
        uint32_t   publish_mask = 0x0;
        bool       isReborn = false;

        spew(npc, "creating partition devices for %s state", isNewborn ? "newborn" : "mature");

        // Initialize the dead block cemetery.
        ok = initCemetery(npc, isNewborn);
        require_action(ok, finish_up, bug(npc, "unable to init cemetery"));

        // If debug enabled, dump the portions of the partition table
        // expected to have valid content for inspection at this point,
        // regardless of whether this is a newborn device.
        if (isDebugEnabled(npc)) {

                dumpPTabHeader(npc, &(npc->ptab->header));
                dumpCemetery(npc, &(npc->ptab->cemetery));
                dumpEntries(npc, &(npc->ptab->entries[0]));
        }

        // create storage partition objects; however, defer publication of most of
        // these in order to properly sequence startup
        for (i = 0; i < kNandPartEntryMaxCount; i++) {

                NandPartEntry * entry = &(npc->ptab->entries[i]);

                const uint32_t content = entry->content;
                const uint32_t partMask = (0x1 << i);

                cleanEntryFlags(npc, entry, isNewborn);

                if ((content != kNandPartContentUndefined) &&
                    (content != kNandPartContentNone) &&
                    (content != kNandPartContentErased)) {

                        hostCreatePartitionDevice(npc, entry, i);
                        
                        created_count++;
                        created_mask |= partMask;

                        if ((kNandPartContentFilesystem == content) ||
                            (kNandPartContentBadBlockTable == content)) {

                                hostPublishPartitionDevice(npc, i);

                                publish_count++;
                                publish_mask |= partMask;

                        } else if (kNandPartContentBootBlock == content) {

                                npc->bootIdx = i;
                        }

                }
        }
        debug(npc, "created %u partitions", created_count);

        // Fail immediately if there were no valid partitions defined.
        ok = (0 < created_count);
        require_action(ok, finish_up, error(npc, "no valid partitions created"));

        // Wait for AppleNANDFTL's services to become available as
        // implied by AppleNANDFactoryBBT service availability.
        debug(npc, "waiting for fbbt service");
        ok = hostWaitForFBBTService(npc);
        require_action(ok, finish_up, error(npc, "no fbbt service available"));
        debug(npc, "fbbt service now available");

        // Initialize dynamic meta data.
        initDiffSequence(npc);
        ok = initReplacements(npc);
        require_action(ok, finish_up, bug(npc, "failed to init replacements"));

        // If device appears to be in newborn state and only if format
        // is enabled, perform the appropriate preparation for initial
        // paritioning; otherwise, when prior partition table exists,
        // prepare based upon its content.
        if (!isNewborn) {

                ok = prepareMatureState(npc, &isReborn);
                require_action(ok, finish_up, error(npc, "couldn't prepare %s state", isReborn ? "reborn" : "mature"));

        } else if (isFormatEnabled(npc)) {

                ok = prepareNewbornState(npc, false);
                require_action(ok, finish_up, error(npc, "couldn't prepare newborn state"));
        }

        // If debug enabled, dump remaining portions of the partition table.
        if (isDebugEnabled(npc)) {

                dumpSpare(npc, &(npc->ptab->spare));
                dumpSubst(npc, &(npc->ptab->subst));
                dumpTranslationTables(npc);
        }

        // If format is enabled and device is either newborn or
        // reborn, erase all blocks in mapped partitions other than
        // boot partition.
        if (isFormatEnabled(npc) && (isNewborn || isReborn)) {

                uint32_t part;
                uint32_t bank;
                uint32_t block;

                for (part = 0; part < kNandPartEntryMaxCount; part++) {
                        if (isPartitionMapped(npc, part) && (npc->bootIdx != part)) {
                                for (bank = 0; bank < getBankWidth(npc, part); bank++) {
                                        for (block = 0; block < getBlockDepth(npc, part); block++) {
                                                debug(npc, "erasing %u:%u:%u", part, bank, block);
                                                if (!eraseBlock(npc, part, bank, block, true)) {
                                                        warn(npc, "failed erase of %u:%u:%u", part, bank, block);
                                                }
                                        }
                                }
                        }
                }
        }

        // Iterate pubishing partition device for each remaining valid entry not previously published
        for (i = 0; i < kNandPartEntryMaxCount; i++) {

                const uint32_t partMask = (0x1 << i);
                if (isMaskSet(partMask, created_mask) && !isMaskSet(partMask, publish_mask)) {

                        hostPublishPartitionDevice(npc, i);
                        publish_count++;
                        publish_mask |= partMask;
                }
        }
        debug(npc, "published %u partitions", publish_count);

        ok = (created_count == publish_count);
        require_action(ok, finish_up, 
                       bug(npc, "created_count:%u != publish_count:%u; created_mask:0x%08x; publish_mask:0x%08x", 
                           created_count, publish_count, created_mask, publish_mask));

        setPartitionTableProperties(npc, isNewborn, isReborn);

finish_up:
        return(ok);
}


// =============================================================================

#pragma mark - Version-Dependent Fixups

static void
makeEntryTypesExplicit(NandPartCore * npc)
{
        // Make certain each entry explicitly identifies partition
        // type iff the entries haven't already been made explicit and
        // the version on partition table precedes the PoolPartition
        // feature.
        if (!isMaskSet(kNandPartDeviceFlagExplicitEntries, npc->ptab->device.flags) && 
            ptabPrecedesFeature(npc->ptab, PoolPartition)) {

                uint32_t part;
                for (part = 0; part < kNandPartEntryMaxCount; part++) {

                        NandPartEntry * entry = &(npc->ptab->entries[part]);
                        uint32_t content = entry->content;
                        uint32_t type = (entry->flags & kNandPartEntryFlagsTypeMask);
                        
                        if (0 != type) {

                                debug(npc, "part %u with content '%c%c%c%c' already has type 0x%08x",
                                      part, 
                                      byteN(content, 3), byteN(content, 2), byteN(content, 1), byteN(content, 0),
                                      type);

                        } else {

                                switch (content) {

                                case kNandPartContentBootBlock:      // fall through
                                case kNandPartContentFirmware:       // fall through
                                case kNandPartContentNVRAM:          // fall through
                                case kNandPartContentEffaceable:

                                        type = kNandPartEntryFlagsTypeSlice;
                                        break;

                                case kNandPartContentDiagData:       // fall through
                                case kNandPartContentSyscfg:         // fall through
                                case kNandPartContentBadBlockTable:  // fall through

                                        type = kNandPartEntryFlagsTypeInfo;
                                        break;

                                case kNandPartContentFilesystem:

                                        type = (kNandPartEntryFlagsTypeSlice |
                                                kNandPartEntryFlagsIsBulk);
                                        break;

                                default:

                                        // XXX - This case probably shouldn't be possible and
                                        // might require error reporting/handling.
                                        type = kNandPartEntryFlagsUnpartitioned;
                                }

                                entry->flags |= type;
                                debug(npc, "marked part %u with content '%c%c%c%c' as type 0x%08x",
                                      part, 
                                      byteN(content, 3), byteN(content, 2), byteN(content, 1), byteN(content, 0),
                                      type);
                        }
                }
        }
}

static void
addIgnoreFBBTFlag(NandPartCore * npc)
{
        // Add kNandPartEntryFlagsIgnoreFBBT to appropriate partition entries
        // iff version on partition table precedes the IgnoreFBBT feature.
        if (ptabPrecedesFeature(npc->ptab, IgnoreFBBT)) {

                uint32_t entryIdx;
                NandPartEntry * entry;

                for (entryIdx = 0; entryIdx < kNandPartEntryMaxCount; entryIdx++) {

                        entry = &(npc->ptab->entries[entryIdx]);
                        switch (entry->content) {

                        case kNandPartContentEffaceable:  /* fall through */
                        case kNandPartContentFirmware:    /* fall through */
                        case kNandPartContentNVRAM:

                                // add flag for these partition types
                                entry->flags = entry->flags | kNandPartEntryFlagsIgnoreFBBT;
                                debug(npc, "adding kNandPartEntryFlagsIgnoreFBBT to partition %u", entryIdx);
                                break; 
                        
                        default:
                                // don't add flag for any other partition type
                                break;
                        }
                }
        }
}

static void
cleanEntryFlags(NandPartCore * npc, NandPartEntry * entry, bool isNewborn)
{
        const uint32_t content = entry->content;

        // If newborn or ptab version precedes addition of
        // CleanEntryFlags feature, make certain that unused partition
        // entries contain only the unpartitioned flag and that all
        // other entries are pruned of any unknown flags.
        if (isNewborn || ptabPrecedesFeature(npc->ptab, PortableCore)) {

                if ((kNandPartContentUndefined == content) ||
                    (kNandPartContentErased    == content) ||
                    (kNandPartContentNone      == content)) {
                        
                        entry->flags = kNandPartEntryFlagsUnpartitioned;

                } else {

                        entry->flags &= kNandPartEntryFlagsMask;
                }
        }
}


/* ========================================================================== */

#pragma mark - Mature Device Preparation

static bool
prepareMatureState(NandPartCore * npc, bool *isReborn)
{
        bool ok = true;
        bool isSpareValid = false;
        bool isSubstValid = false;
        uint32_t * ceList = NULL;
        uint32_t * blockList = NULL;
        const uint32_t count = npc->ptab->spare.count;
        const uint32_t size = sizeof(uint32_t) * count;
        uint32_t idx;

        spew(npc, "preparing mature state");

        // For older devices, it is not an error to fail either subst
        // or spare validation; however, in either case, we certainly
        // shouldn't proceed with the rest of the preparation below.
        isSubstValid = validateSubst(npc, &npc->ptab->subst);
        require_action(isSubstValid, clean_up, warn(npc, "no subst info available"));
        isSpareValid = validateSpare(npc, &npc->ptab->spare);
        require_action(isSpareValid, clean_up, warn(npc, "no spare info available"));

        spew(npc, "spare info in partition table appears to be valid; count:%u, size:%u", count, size);

        ceList = (uint32_t*)hostAllocMem(npc, size, false);
        blockList = (uint32_t*)hostAllocMem(npc, size, false);

        ok = ((NULL != ceList) && (NULL != blockList));
        require_action(ok, clean_up, bug(npc, "alloc failed"));
        
        for (idx = 0; idx < count; idx++) {
                NandPartSpareLocation * loc = &npc->ptab->spare.locations[idx];
                ceList[idx] = loc->ce;
                blockList[idx] = getSpareLocationBlock(npc, loc);
        }

        spew(npc, "notifying Whimory of spare list");
        ok = hostValidateSpareList(npc, ceList, blockList, count);
        require_action(ok || (0 < count), clean_up, error(npc, "failed host spare validation"));

        if (ok) {
                *isReborn = false;
                ok = refreshReplacements(npc);
        } else if (0 < count) {
                *isReborn = true;
                ok = prepareRebornState(npc);
        }

clean_up:
        if (NULL != ceList) {
                hostFreeMem(npc, ceList, size);
        }
        if (NULL != blockList) {
                hostFreeMem(npc, blockList, size);
        }

        return ok;
}

static void 
setPartitionTableProperties(NandPartCore * npc, bool isNewborn, bool isReborn)
{
        hostSetNumberProperty(npc, kNandPartCoreVersionMajorKey, npc->ptab->header.versionMajor, 32);
        hostSetNumberProperty(npc, kNandPartCoreVersionMinorKey, npc->ptab->header.versionMajor, 32);
        hostSetNumberProperty(npc, kNandPartCoreGenerationKey, npc->ptab->header.generation, 32);

        if (isDebugEnabled(npc)) {

                hostSetBooleanProperty(npc, kNandPartCoreIsNewbornKey, isNewborn);
                hostSetBooleanProperty(npc, kNandPartCoreIsRebornKey, isReborn);

                //hostSetDataProperty(npc, "*ptab", npc->ptab, sizeof(*npc->ptab));
                //hostSetDataProperty(npc, "ptab->header", &npc->ptab->header, sizeof(npc->ptab->header));
                //hostSetDataProperty(npc, "ptab->fbbtCache", &npc->ptab->fbbtCache, sizeof(npc->ptab->fbbtCache));
                //hostSetDataProperty(npc, "ptab->device", &npc->ptab->device, sizeof(npc->ptab->device));

                hostSetDataProperty(npc, "ptab->cemetery", &npc->ptab->cemetery, sizeof(npc->ptab->cemetery));
                hostSetDataProperty(npc, "ptab->spare", &npc->ptab->spare, sizeof(npc->ptab->spare));
                hostSetDataProperty(npc, "ptab->subst", &npc->ptab->subst, sizeof(npc->ptab->subst));

                //hostSetDataProperty(npc, "ptab->unused", &npc->ptab->unused, sizeof(npc->ptab->unused));
                //hostSetDataProperty(npc, "ptab->entries", &npc->ptab->entries, sizeof(npc->ptab->entries));
        }
}


// =============================================================================

#pragma mark - Reborn/Newborn Device Preparation

static bool
prepareRebornState(NandPartCore * npc)
{
        // XXX - don't we need to clean the substitution table here and have another go at it?

        npc->isFBBTCached = false;

        // prepare to be reborn by becoming newborn...
        return(prepareNewbornState(npc, true));
}

static bool
prepareNewbornState(NandPartCore * npc, bool isReborn)
{
        bool ok = false;
        size_t sparesSize = 0;
        uint32_t sparesDesired = 0;
        uint32_t sparesReceived = kNandPartSpareMaxCount;
        uint32_t * ceList = NULL;
        uint32_t * blockList = NULL;
        uint32_t idx;
        uint32_t sparesNeeded = 0;
        uint32_t sparesExtra = 0;

        spew(npc, "preparing %s state", isReborn ? "reborn" : "newborn");

        ok = (NULL != npc->ptab);
        require_action(ok, clean_up, bug(npc, "ptab is NULL"));

        sparesNeeded = countSparesNeeded(npc, false);

        ok = (kNandPartCountInvalid != sparesNeeded);
        require_action(ok, clean_up, bug(npc, "invalid needed spares count"));

        ok = (kNandPartSpareMaxCount >= sparesNeeded);
        require_action(ok, clean_up, bug(npc, "too many spares requested - %u", sparesNeeded));

        sparesExtra = countSparesExtra(npc);

        sparesDesired = sparesNeeded + sparesExtra;
        if (sparesDesired > kNandPartSpareMaxCount) {
                debug(npc, "limiting spares from %u to %u", sparesDesired, kNandPartSpareMaxCount);
                sparesDesired = kNandPartSpareMaxCount;
        }
        debug(npc, "spares - desired:%u", sparesDesired);

        sparesSize = sizeof(uint32_t) * sparesReceived;
        ceList = (uint32_t*) hostAllocMem(npc, sparesSize, false);
        blockList = (uint32_t*) hostAllocMem(npc, sparesSize, false);

        ok = ((NULL != ceList) && (NULL != blockList));
        require_action(ok, clean_up, bug(npc, "alloc failed"));

        // Request spare blocks
        debug(npc, "requesting %d spare blocks", sparesDesired);
        ok = hostRequestSpareBlocks(npc, ceList, blockList, sparesDesired, &sparesReceived);
        require_action(ok, clean_up, error(npc, "spare request failed"));
        debug(npc, "spares - received:%u", sparesReceived);

        ok = (kNandPartSpareMaxCount >= sparesReceived);
        require_action(ok, clean_up, bug(npc, "too many spares"));

        ok = (sparesNeeded <= sparesReceived);
        require_action(ok, clean_up, 
                       error(npc, "insufficient spares - needed:%u received:%u",
                             sparesNeeded, sparesReceived));

        if (sparesDesired > sparesReceived) {
                warn(npc, "not enough spares");
        }

        spew(npc, "adding blocks from spare list to ptab");
        hostSetMem(npc, &npc->ptab->spare, 0, sizeof(NandPartSpare));
        npc->ptab->spare.count = sparesReceived;
        for (idx = 0; idx < npc->ptab->spare.count; idx++) {
                const uint32_t ce = ceList[idx];
                const uint32_t block = blockList[idx];
                spew(npc, "    %2u - ce:%2u block:0x%08X", idx, ce, block);
                npc->ptab->spare.locations[idx].ce = (uint8_t) ce;
                setSpareLocationBlock(npc, &npc->ptab->spare.locations[idx], block);
        }
        npc->ptab->spare.magic = kNandPartSpareMagic;
        fletcher16(npc,
                   &npc->ptab->spare.checks[0], 
                   &npc->ptab->spare.checks[1], 
                   &npc->ptab->spare, 
                   sizeof(npc->ptab->spare) - sizeof(npc->ptab->spare.checks));
        
        ok = substituteSparesForBadBlocks(npc);
        require_action(ok, clean_up, error(npc, "spare substitution failed"));

        // Now we have valid spares and substitutions
        // assigned, we can finally build the real
        // replacement table.
        refreshReplacements(npc);

        // record good partition table as soon as possible,
        // the extra erase cycle is worth minimizing the
        // window during which the information just built
        // can be lost during fragile device initialization
        ok = writePTab(npc);
        if (!ok) {
                error(npc, "unable to store newborn partition table");
        }

clean_up:
        if (NULL != ceList)
                hostFreeMem(npc, ceList, sparesSize);
        if (NULL != blockList)
                hostFreeMem(npc, blockList, sparesSize);

        return ok;
}

static uint32_t
countSparesNeeded(NandPartCore * npc, bool useWorstCase)
{
        uint32_t need = kNandPartCountInvalid;
        uint32_t scrub = 0;
        uint32_t pool = 0;
        uint32_t part;

        // determine how many spares are needed to supply blocks
        // for pool partitions
        for (part = 0; part < kNandPartEntryMaxCount; part++) {
                if (isPartitionMapped(npc, part) && areFlagsSet(npc, part, kNandPartEntryFlagsTypePool)) {
                        pool += (getBankWidth(npc, part) * getBlockDepth(npc, part));
                }
        }

        if (useWorstCase) {
                uint32_t part;
                uint32_t total = 0;

                for (part = 0; part < kNandPartMappingMaxCount; part++) {
                        if (isPartitionMapped(npc, part) && !areFlagsSet(npc, part, kNandPartEntryFlagsTypePool)) {
                                const uint32_t count = calcPartitionMapCount(npc, part);
                                total += count;
                        }
                }

                scrub = total - npc->provider->geometry.ceCount;
        } else {
                // determine how many spares are needed due to stillborn
                // blocks from slice partitions
                scrub = scrubMappedPartitionsForStillborn(npc);
        }
        require_action(kNandPartCountInvalid != scrub, finish_up, bug(npc, "invalid scrub count"));

        // total number of spares needed is the sum of the two
        need = scrub + pool;
        debug(npc, "counting spares - need:%u = scrub:%u + pool:%u", need, scrub, pool);

finish_up:
        return(need);
}

static uint32_t
countSparesExtra(NandPartCore * npc)
{
        uint32_t extra = kNandPartCountInvalid;
        const uint32_t oldExtra = (kNandPartBankMax - npc->provider->geometry.ceCount) / 2;
        const uint32_t newExtra = kNandPartSpareMinExtraCount;

        // A request has been made to reduce the extras count; however, I'd like for
        // new builds of launched products to continue to behave the same way.  Thus,
        // we conditionally decide which quantity to return depending upon whether
        // this specific device allows pool partitioning.
        extra = (allowPoolPartitions(npc) ? newExtra : oldExtra);
        debug(npc, "counting spares - extra:%u = {newExtra:%u,oldExtra:%u}", extra, newExtra, oldExtra);

        return(extra);
}

static uint32_t
countMaxSpares(NandPartCore * npc)
{
        const uint32_t needed = countSparesNeeded(npc, true);
        const uint32_t extra = countSparesExtra(npc);

        return min(needed + extra, kNandPartSpareMaxCount);
}

static uint32_t
scrubMappedPartitionsForStillborn(NandPartCore * npc)
{
        bool ok = true;
        uint32_t part;
        uint32_t bank;
        uint32_t block;
        void * patternBuf = NULL;
        uint32_t bytesPerBlock = 0;
        uint32_t count = 0;

        spew(npc, "scrubbing mapped partitions");

        for (part = 0; part < kNandPartEntryMaxCount; part++) {
                if (isPartitionMapped(npc, part) && !areFlagsSet(npc, part, kNandPartEntryFlagsTypePool)) {

                        // XXX - the filter above allows boot blocks through; should it?

                        const uint32_t bytesPerPage = getBytesPerPage(npc, part);
                        const uint32_t pagesPerBlock = getPagesPerBlock(npc, part);

                        if ((NULL == patternBuf) || ((bytesPerPage * pagesPerBlock) != bytesPerBlock)) {

                                // Physical geometry for current partition different from prior partition;
                                // therefore, we need to free previous pattern buffer.
                                if (NULL != patternBuf) {
                                        hostFreeMem(npc, patternBuf, bytesPerBlock);
                                        patternBuf = NULL;
                                }

                                // Note that this is a large allocation (512 KiB <= bytesPerBlock ~< 4 MiB, for
                                // relevant historical and near term configurations); however, this allocation
                                // only happens in Restore OS during first-time init of device, so memory pressure
                                // shouldn't be a significant issue.
                                bytesPerBlock = (bytesPerPage * pagesPerBlock);
                                patternBuf = hostAllocMem(npc, bytesPerBlock, true);
                                ok = (NULL != patternBuf);
                                require_action(ok, clean_up, bug(npc, "alloc failed"));

                                hostReadRandom(npc, patternBuf, bytesPerBlock);
                        }

                        for (bank = 0; bank < getBankWidth(npc, part); bank++) {
                                for (block = 0; block < getBlockDepth(npc, part); block++) {
                                        spew(npc, "scrubbing %u:%u:%u", part, bank, block);
                                        if (!scrubBlockForStillborn(npc, part, bank, block, patternBuf)) {
                                                count++;
                                                debug(npc, "block %u:%u:%u is stillborn", part, bank, block);
                                                markBlockDead(npc, part, bank, block, false);
                                        }
                                }
                        }
                }
        }

        debug(npc, "during initial partitioning, %u out of %u blocks found dead in mapped partitions",
              count, getTotalMapCount(npc));

clean_up:

        if (NULL != patternBuf) {
                hostFreeMem(npc, patternBuf, bytesPerBlock);
                patternBuf = NULL;
        }

        if (!ok) {
                return kNandPartCountInvalid; 
        }

        return(count);
}

static bool
scrubBlockForStillborn(NandPartCore * npc, uint32_t part, uint32_t bank, uint32_t block, void * patternBuf)
{
        const uint32_t initialScrubCount = 2;
        const uint32_t map = calcBlockMap(npc, part, bank, block, false);
        const size_t pageSize = getBytesPerPage(npc, part);
        const uint32_t pagesPerBlock = getPagesPerBlock(npc, part);
        uint8_t * pagePatternBuf = NULL;
        uint8_t * readBuf = NULL;
        bool isClean;
        uint32_t pass;
        uint32_t page;
        bool ok = true;

        ok = isFormatEnabled(npc);
        require_action(ok, clean_up, bug(npc, "attempted to scrub without format enabled"));

        readBuf = (uint8_t *)hostAllocMem(npc, pageSize, true);
        ok = (NULL != readBuf);
        require_action(ok, clean_up, bug(npc, "alloc failed"));

        for (pass = 0; pass <= initialScrubCount; pass++) {

                ok = eraseBlock(npc, part, bank, block, false);
                require_action(ok, clean_up, spew(npc, "failed erase of %u:%u:%u", part, bank, block));

                // final pass is erase only; not actually a "scrub" per se, 
                // and solely intended to leave good blocks in an erased state
                if (pass != initialScrubCount) {

                        // attempt to write to all pages in block
                        for (page = 0; page < pagesPerBlock; page++) {

                                const uint32_t patternPage = ((map + page) % pagesPerBlock);
                                const size_t patternOffset = patternPage * pageSize;

                                pagePatternBuf = (((uint8_t*)patternBuf) + patternOffset);

                                ok = writePage(npc, part, bank, block, page, pagePatternBuf, false);
                                require_action(ok, clean_up, spew(npc, "failed write - %u:%u:%u:%u", part, bank, block, page));
                        }

                        // after successfully writing to all pages readback and compare
                        for (page = 0; page < pagesPerBlock; page++) {

                                const uint32_t patternPage = ((map + page) % pagesPerBlock);
                                const size_t patternOffset = patternPage * pageSize;

                                pagePatternBuf = (((uint8_t*)patternBuf) + patternOffset);

                                ok = readPage(npc, part, bank, block, page, readBuf, &isClean, false);
                                require_action(ok, clean_up, spew(npc, "failed read - %u:%u:%u:%u (%c)", part, bank, block, page, bool2Char(isClean)));

                                ok = (0 == hostCompareMem(npc, pagePatternBuf, readBuf, pageSize));
                                require_action(ok, clean_up, spew(npc, "failed compare - %u:%u:%u:%u", part, bank, block, page));
                        }
                }
        }

clean_up:
        if (NULL != readBuf) {
                hostFreeMem(npc, readBuf, pageSize);
        }

        debug(npc, "block %u:%u:%u %s scrub", part, bank, block, (ok ? "passed" : "failed"));

        return(ok);
}


// =============================================================================

#pragma mark - Initial Format/Partitioning

static bool
formatWithSpecs(NandPartCore * npc)
{
        uint32_t                index;
        bool                    haveBulk = false;

        initPartitionTable(npc);

        /* add partitions */
        for (index = 0; index < kNandPartEntryMaxCount; index++) {
                require_action(addPartition(npc, &(npc->provider->params.partitionSpecs[index]), index, &haveBulk),
                               fail, error(npc, "unable to add partition %d of %d", index, kNandPartEntryMaxCount));
        }
        require_action(haveBulk, fail, error(npc, "no bulk partition defined"));

        /* success - return the array */
        return(true);

fail:
        return(false);
}

static void
initPartitionTable(NandPartCore * npc)
{
        unsigned idx;
        NandPartTable * ptab = npc->ptab;

        /* initialize buffer with random content so that unused areas
         * will cause some overall whitening effect
         */
        hostReadRandom(npc, ptab, sizeof(NandPartTable));

        /* immutable magic key */
        ptab->header.magic = kNandPartHeaderMagic;

        /* continue to set the historically unused epoch field to zero */
        ptab->header.should_be_zero = 0;

        /* record current partition major and minor versions */
        ptab->header.versionMajor = majorVersion(npc);
        ptab->header.versionMinor = kNandPartHeaderVersionMinor;

        /* generation starts at zero and should get updated every time
         * partition tables are rewritten 
         */
        ptab->header.generation = 1;

        /* copy device information */
        ptab->device.should_be_zero[0]  = 0;
        ptab->device.should_be_zero[1]  = 0;
        ptab->device.should_be_one      = 1;
        ptab->device.numOfCS            = npc->provider->geometry.ceCount;
        ptab->device.blocksPerCS        = npc->provider->geometry.blocksPerCE;
        ptab->device.pagesPerBlock      = npc->provider->geometry.pagesPerMLCBlock;
        ptab->device.sectorsPerPage     = npc->provider->geometry.bytesPerFullPage / kNandPartSectorSize;
        ptab->device.spareBytesPerPage  = npc->provider->geometry.bytesPerFullPageMeta;

        /* in retrospect, this wasn't the most logical place to add a global flags field */
        ptab->device.flags = (isPoolAllowed(npc) ? kNandPartDeviceFlagAllowPool : kNandPartDeviceFlagNone);
        ptab->device.flags |= (kNandPartDeviceFlagNeedsCemetery | kNandPartDeviceFlagExplicitEntries);

        /* make certain that all entries have reasonable initial content */
        for (idx = 1; idx < kNandPartEntryMaxCount; idx++) {
                ptab->entries[idx].content    = kNandPartContentNone;
                ptab->entries[idx].reserved_0 = 0;
                ptab->entries[idx].reserved_1 = 1;
                ptab->entries[idx].flags      = kNandPartEntryFlagsUnpartitioned;
        }
}

static uint32_t
majorVersion(NandPartCore * npc)
{
        // Use minimum of current software major version and
        // product-specific device tree property.  If the property was
        // not set in the device tree, the value for it will be set to
        // the maximum unsigned integer size, thus defaulting to the
        // current software major version.

        return(min(kNandPartHeaderVersionMajor, maxMajorVersion(npc)));
}

static bool
addPartition(NandPartCore *npc, NandPartSpec *spec, uint32_t index, bool * haveBulk)
{
        bool result = true;
        NandPartTable * ptab = npc->ptab;
        static uint32_t managedCount = 0;
        static uint32_t baseOffset = 0;

        ptab->entries[index].content = spec->content;
        ptab->entries[index].flags   = spec->type | spec->flags;

        switch (spec->type) {

        case kNandPartEntryFlagsIsBulk:
                result = !*haveBulk;
                require_action(result, finish_up, error(npc, "bulk spec'ed more than once"));
                ptab->entries[index].slice.offset = baseOffset;
                ptab->entries[index].slice.depth  = (npc->provider->geometry.blocksPerCE - baseOffset);
                ptab->entries[index].flags        |= kNandPartEntryFlagsTypeSlice;
                baseOffset = npc->provider->geometry.blocksPerCE;
                *haveBulk = true;
                break;

        case kNandPartEntryFlagsTypeSlice:
                result = !*haveBulk;
                require_action(result, finish_up, error(npc, "slice spec'ed after bulk"));
                ptab->entries[index].slice.offset = baseOffset;
                ptab->entries[index].slice.depth  = spec->depth;
                baseOffset += spec->depth;
                managedCount += (npc->provider->geometry.ceCount * spec->depth);
                break;

        case kNandPartEntryFlagsTypePool:
                ptab->entries[index].pool.width = spec->width;
                ptab->entries[index].pool.depth = spec->depth;
                managedCount += (spec->width * spec->depth);
                break;

        case kNandPartEntryFlagsTypeInfo:
                break;

        default:
                result = false;
                require_action(result, finish_up, error(npc, "unrecognized partition type"));
        }

        result = (managedCount <= kNandPartCemeteryCapacity);
        require_action(result, finish_up, error(npc, "too many blocks allocated"));

        result = (baseOffset <= npc->provider->geometry.blocksPerCE);
        require_action(result, finish_up, error(npc, "offset exceeds bank length"));

finish_up: 
        return(result);
}


// =============================================================================

#pragma mark - Runtime Parameters

static bool
isBootPartErasable(NandPartCore * npc)
{
        return npc->provider->params.isBootPartErasable;
}

static bool
isBootPartWritable(NandPartCore * npc)
{
        return npc->provider->params.isBootPartWritable;
}

static bool
isFormatEnabled(NandPartCore * npc)
{
        return npc->provider->params.isFormatEnabled;
}

static bool
isDebugEnabled(NandPartCore * npc)
{
        return npc->provider->params.isDebugEnabled;
}

static bool
isPoolAllowed(NandPartCore * npc)
{
        return npc->provider->params.isPoolAllowed;
}

static uint32_t
maxMajorVersion(NandPartCore * npc)
{
        return npc->provider->params.maxMajorVersion;
}


// =============================================================================

#pragma mark - Miscellaneous Global Information

static bool 
isPpnDevice(NandPartCore * npc)
{
        return(npc->provider->geometry.isPpn);
}

static bool
findBootPartIndex(NandPartCore * npc, uint32_t * partIndex)
{
        bool      ok = false;
        uint32_t  idx;

        for (idx = 0; !ok && (idx < kNandPartEntryMaxCount); idx++) {
                if (kNandPartContentBootBlock == getEntry(npc, idx)->content) {
                        *partIndex = idx;
                        ok = true;
                }
        }

        return(ok);
};

static bool
isEntryIndexValid(NandPartCore * npc, uint32_t partIndex, bool allowRawIndex)
{
        return((kNandPartEntryMaxCount > partIndex) || (allowRawIndex && isRawEntryIndex(npc, partIndex)));
}

static bool
isRawEntryIndex(NandPartCore * npc, uint32_t partIndex)
{
        return(kNandPartEntryMaxCount == partIndex);
}

static bool
allowPoolPartitions(NandPartCore * npc)
{
        const bool isPTabCompatible = (kNandPartFeaturePoolPartitionVersionMajor <= npc->ptab->header.versionMajor);
        const bool isAllowPoolFlagged = isMaskSet(kNandPartDeviceFlagAllowPool, npc->ptab->device.flags);
        return(isPTabCompatible && isAllowPoolFlagged);
}


// =============================================================================

#pragma mark - Partition Entry Information

static const char *
entryContentName(NandPartCore * npc, uint32_t partIndex)
{
        const char * name;

        if (isRawEntryIndex(npc, partIndex)) {
                name = kNandPartContentNameUnformatted;
        } else {
                switch (getEntry(npc, partIndex)->content) {
                case kNandPartContentBootBlock:
                        name = kNandPartContentNameBootBlock;
                        break;
                case kNandPartContentDiagData:
                        name = kNandPartContentNameDiagData;
                        break;
                case kNandPartContentSyscfg:
                        name = kNandPartContentNameSyscfg;
                        break;
                case kNandPartContentNVRAM:
                        name = kNandPartContentNameNVRAM;
                        break;
                case kNandPartContentFirmware:
                        name = kNandPartContentNameFirmware;
                        break;
                case kNandPartContentFilesystem:
                        name = kNandPartContentNameFilesystem;
                        break;
                case kNandPartContentEffaceable:
                        name = kNandPartContentNameEffaceable;
                        break;
                case kNandPartContentBadBlockTable:
                        name = kNandPartContentNameBadBlockTable;
                        break;
                case kNandPartContentNone:
                        name = kNandPartContentNameUnallocated;
                        break;
                default:
                        name = kNandPartContentNameUnrecognized;
                }
        }

        return name;
}

static bool
areFlagsSet(NandPartCore * npc, uint32_t partIndex, uint32_t flagMask)
{
        return(isMaskSet(flagMask, getEntry(npc, partIndex)->flags));
}

static bool
usingSLCBlocks(NandPartCore * npc, uint32_t partIndex)
{
        // pool partitioning block allocations should probably
        // redundantly record whether specific block is being treated
        // as SLC

        return(areFlagsSet(npc, partIndex, kNandPartEntryFlagsUsingSLCBlocks));
}

static bool
usingFullPages(NandPartCore * npc, uint32_t partIndex)
{
        return(areFlagsSet(npc, partIndex, kNandPartEntryFlagsUsingFullPages));
}

static uint32_t
getBlockOffset(NandPartCore * npc, uint32_t partIndex)
{
        uint32_t offset = 0;

        require_action(isEntryIndexValid(npc, partIndex, true), finish_up, 
                       bug(npc, "out of bounds - part:%u", partIndex));

        if (isRawEntryIndex(npc, partIndex)) {
                offset = 0;
        } else if (areFlagsSet(npc, partIndex, kNandPartEntryFlagsTypeSlice)) {
                offset = getEntry(npc, partIndex)->slice.offset;
        } else {
                warn(npc, "invalid request for block offset of partition 0x%08x", partIndex);
        }

finish_up:
        return(offset);
}


// =============================================================================

#pragma mark - Virtual to Physical Address Calculation

static uint32_t
mapNandBlock(NandPartCore * npc, uint32_t partIndex, uint32_t partBank, uint32_t partBlock, bool allowSpare)
{
        uint32_t block = kNandPartBlockInvalid;

        require_action(getBlockDepth(npc, partIndex) > partBlock, finish_up,
                       error(npc, "out of bounds - part:%u block:%u", partIndex, partBlock));

        // this hook will be where the table lookup of nand block for
        // pool partitioning will be substituted for the direct
        // mapping below

        if (allowSpare && isPartitionMapped(npc, partIndex)) {
                uint32_t spareIndex;
                if (isBlockReplaced(npc, partIndex, partBank, partBlock, &spareIndex)) {
                        NandPartSpareLocation * location = getSpareLocation(npc, spareIndex);
                        require_action(NULL != location, finish_up, bug(npc, "missing location"));
                        block = getSpareLocationBlock(npc, location);
                } else {
                        block = mapNandBlock(npc, partIndex, partBank, partBlock, false);
                }
        } else {
                block = (partBlock + getBlockOffset(npc, partIndex));
        }

finish_up:
        return(block);
}

static uint32_t
mapNandCE(NandPartCore * npc, uint32_t partIndex, uint32_t partBank, uint32_t partBlock, bool allowSpare)
{
        uint32_t ce = kNandPartCEInvalid;

        require_action(getBankWidth(npc, partIndex) > partBank, finish_up, 
                       error(npc, "out of bounds - part:%u bank:%u", partIndex, partBank));

        // this hook will be where the table lookup of nand ce for
        // pool partitioning will be substituted for the direct
        // mapping below

        if (allowSpare && isPartitionMapped(npc, partIndex)) {
                uint32_t spareIndex;
                if (isBlockReplaced(npc, partIndex, partBank, partBlock, &spareIndex)) {
                        NandPartSpareLocation * location = getSpareLocation(npc, spareIndex);
                        require_action(NULL != location, finish_up, bug(npc, "missing location"));
                        ce = location->ce;
                } else {
                        ce = mapNandCE(npc, partIndex, partBank, partBlock, false);
                }
        } else {
                ce = partBank;
        }

finish_up:
        return(ce);
}

static uint32_t
calcNandAddress(NandPartCore * npc, uint32_t nandBlock, uint32_t pageInBlock, bool useSLC)
{
        uint32_t address;

        if (isPpnDevice(npc)) {

                const uint32_t blockShift = npc->provider->geometry.bitsPerPageAddr;
                const uint32_t slcBitShift = (npc->provider->geometry.bitsPerPageAddr +
                                              npc->provider->geometry.bitsPerBlockAddr +
                                              npc->provider->geometry.bitsPerCauAddr);
                const uint32_t slcBit = ((useSLC ? 1UL : 0UL) << slcBitShift);

                address = pageInBlock | (nandBlock << blockShift) | slcBit;

        } else {

                address = pageInBlock + (nandBlock * npc->provider->geometry.blockStride);
        }

        return (address);
}

#if NAND_PART_CORE_ORPHAN
static uint32_t
calcNandBlock(NandPartCore * npc, uint32_t nandAddress, bool useSLC)
{
        uint32_t block;

        if (isPpnDevice(npc)) {

                const uint32_t blockShift = npc->provider->geometry.bitsPerPageAddr;
                const uint32_t slcBitShift = (npc->provider->geometry.bitsPerPageAddr +
                                              npc->provider->geometry.bitsPerBlockAddr +
                                              npc->provider->geometry.bitsPerCauAddr);

                // XXX - should SLC bit get removed if block previously programmed as SLC?
                const uint32_t slcBit = ((useSLC ? 1UL : 0UL) << slcBitShift);

                block = (nandAddress & ~slcBit) >> blockShift;

        } else {

                block = nandAddress / npc->provider->geometry.blockStride;
        }

        return (block);
}
#endif


// =============================================================================

#pragma mark - Meta Buffer Proxy

static uint8_t *
getMetaBuf(NandPartCore * npc)
{
        return(npc->meta);
}

static uint8_t *
resetMetaBuf(NandPartCore * npc)
{
        uint8_t * meta = getMetaBuf(npc);
        hostSetMem(npc, meta, 0, npc->provider->geometry.bytesPerFullPageMeta);
        return(meta);
}

static uint8_t *
endorseMetaBuf(NandPartCore * npc, uint32_t partIndex)
{
        const uint8_t signature = (partIndex & 0xF) | AND_SPARE_TYPE_REGION_BFN;
        uint8_t * meta = resetMetaBuf(npc);
        meta[0] = signature;
        return(meta);
}

static void
verifyMetaBuf(NandPartCore * npc, uint32_t partIndex)
{
        const uint8_t signature = (partIndex & 0xF) | AND_SPARE_TYPE_REGION_BFN;
        uint8_t * meta = getMetaBuf(npc);
        if (signature != meta[0]) {

                // Not an error condition since prior versions haven't been
                // rigorous about endorsing meta data; regardless, could be
                // quite useful in certain debug scenarios.
                //
                // Also, it doesn't seem as if all (any?) of the contexts are
                // actually faithfully writing this information out.  At this
                // point, although this has parity with current implementation,
                // it may be more dangerous than it's actually worth.
                spew(npc, "meta mis-match; signature:0x%02x, meta[0]:0x%02x", signature, meta[0]);
        }
}


// =============================================================================

#pragma mark - Block Replacement

static bool
initReplacements(NandPartCore * npc)
{
        bool ok = false;
        unsigned map;
        unsigned count;
        NandPartReplacement * replacement;

        spew(npc, "initializing block replacements");

        // initialize mapping
        initMapping(npc);

        // sanity check the mapping count
        count = getTotalMapCount(npc);
        ok = (kNandPartReplacementMaxCount >= count);
        require_action(ok, finish_up, 
                       bug(npc, "total map count, %u, greater than max replacement count, %u",
                           count, kNandPartReplacementMaxCount));

        // reset all valid replacement entries
        spew(npc, "marking %u replacement entries as valid but not replaced", count);
        for (map = 0; map < count; map++) {

                replacement = getMappedReplacement(npc, map);
                ok = (NULL != replacement);
                require_action(ok, finish_up, bug(npc, "invalid replacement - map:%u", map));

                replacement->isValid = true;
                replacement->isReplaced = false;
                replacement->spareIndex = kNandPartReplacementInvalidIndex;
        }

        // init all invalid replacement entries
        spew(npc, "marking remaining %u replacement entries as invalid and not replaced", 
             (kNandPartReplacementMaxCount - count));
        for (; map < kNandPartReplacementMaxCount; map++) {

                replacement = getMappedReplacement(npc, map);
                ok = (NULL != replacement);
                require_action(ok, finish_up, bug(npc, "invalid replacement - map:%u", map));

                replacement->isValid = false;
                replacement->isReplaced = false;
                replacement->spareIndex = kNandPartReplacementInvalidIndex;
        }

finish_up:
        return ok;
}

static bool
refreshReplacements(NandPartCore * npc)
{
        bool ok = true;
        unsigned spare;
        
        spew(npc, "refreshing block replacements");
        
        if (validateSpare(npc, &npc->ptab->spare) && validateSubst(npc, &npc->ptab->subst)) {
                
                // walk through substitution table, updating the matching
                // replacement for each substitution defined therein
                for (spare = 0; spare < npc->ptab->subst.count; spare++) {
                        
                        const unsigned map = npc->ptab->subst.maps[spare];
                        // walk through only till substitution entries are valid
                        if (0 == map)
                        {
                            break;
                        }
                        NandPartReplacement * replacement = getMappedReplacement(npc, map);

                        ok = (NULL != replacement);
                        require_action(ok, finish_up, bug(npc, "invalid replacement - map:%u", map));

                        ok = (replacement->isValid);
                        require_action(ok, finish_up, bug(npc, "invalid replacement - spare:%u map:%u", spare, map));

                        replacement->isReplaced = true;
                        replacement->spareIndex = (uint32_t)spare;
                }
        }

        if (isDebugEnabled(npc)) {
                const unsigned size = sizeof(NandPartReplacement) * kNandPartReplacementMaxCount;
                hostSetDataProperty(npc, "replacements", &(npc->replacements[0]), size);
        }

finish_up:
        return(ok);
}

static bool
validateSpare(NandPartCore * npc, NandPartSpare * spare)
{
        bool ok = true;

        ok = (kNandPartSpareMagic == spare->magic);
        require_action(ok, finish_up, warn(npc, "bad magic - expected:0x%08x found:0x%08x", 
                                           kNandPartSpareMagic, spare->magic));

        ok = (kNandPartSpareMaxCount >= spare->count);
        require_action(ok, finish_up, bug(npc, "out of bounds - limit:%u >= count:%u",
                                          kNandPartSpareMaxCount, spare->count));

        if (!isMaskSet(kNandPartSpareFlagIgnoreChecks, spare->flags)) {

                const size_t len = (sizeof(*spare) - sizeof(spare->checks));
                uint8_t checkA;
                uint8_t checkB;

                fletcher16(npc, &checkA, &checkB, spare, len);
                ok = ((checkA == spare->checks[0]) && (checkB == spare->checks[1]));
                require_action(ok, finish_up, bug(npc, "incorrect checksum"));
        }

finish_up:
        return(ok);
}

static bool
validateSubst(NandPartCore * npc, NandPartSubst * subst)
{
        bool ok = true;

        ok = (kNandPartSubstMagic == subst->magic);
        require_action(ok, finish_up, warn(npc, "bad magic - expected:0x%08x found:0x%08x", 
                                           kNandPartSubstMagic, subst->magic));

        ok = (kNandPartSubstMaxCount >= subst->count);
        require_action(ok, finish_up, bug(npc, "out of bounds - limit:%u >= count:%u",
                                          kNandPartSubstMaxCount, subst->count));

        if (!isMaskSet(kNandPartSubstFlagIgnoreChecks, subst->flags) &&
            isMaskSet(kNandPartSubstFlagCorrectedChecks, subst->flags)) {

                const size_t len = (sizeof(*subst) - sizeof(subst->checks));
                uint8_t checkA;
                uint8_t checkB;

                fletcher16(npc, &checkA, &checkB, subst, len);
                ok = ((checkA == subst->checks[0]) && (checkB == subst->checks[1]));
                require_action(ok, finish_up, bug(npc, "incorrect checksum"));
        }

finish_up:
        return(ok);
}

static bool
isBlockReplaced(NandPartCore * npc, uint32_t partIndex, uint32_t partBank, uint32_t partBlock, uint32_t * spareIndex)
{
        bool isReplaced = false;
        uint32_t spare = (uint32_t)-1;
        const uint32_t map = calcBlockMap(npc, partIndex, partBank, partBlock, false);
        const NandPartReplacement * replacement = getMappedReplacement(npc, map);

        require_action(NULL != replacement, finish_up, bug(npc, "invalid replacement - map:%u", map));

        isReplaced = replacement->isReplaced;
        if (isReplaced && (NULL != spareIndex)) {
                *spareIndex = spare = (uint32_t)replacement->spareIndex;
        }

        spew(npc, "block is%s replaced - partIndex:%u partBank:%u partBlock:%u map:%u %s:%u",
             isReplaced ? "" : " not", partIndex, partBank, partBlock, map, isReplaced ? "spare" : "N/A", spare);

finish_up:
        return(isReplaced);
}

static NandPartReplacement *
getMappedReplacement(NandPartCore * npc, uint32_t map)
{
        NandPartReplacement * replacement = NULL;

        require_action(kNandPartReplacementMaxCount > map, finish_up, 
                       bug(npc, "out of bounds - map:%u", map));
        replacement = &(npc->replacements[map]);

finish_up:
        return(replacement);
}

static NandPartSpareLocation * 
getSpareLocation(NandPartCore * npc, uint32_t spareIndex)
{
        NandPartSpareLocation * location = NULL;

        require_action(kNandPartSpareMaxCount > spareIndex, finish_up, 
                       bug(npc, "out of bounds - spareIndex:%u", spareIndex));
        location = &npc->ptab->spare.locations[spareIndex];

finish_up:
        return(location);                      
}

static uint32_t
getSpareLocationBlock(NandPartCore * npc, NandPartSpareLocation * location)
{
        return((((uint32_t)location->block[0]) <<  0) +
               (((uint32_t)location->block[1]) <<  8) +
               (((uint32_t)location->block[2]) << 16));
}

static void
setSpareLocationBlock(NandPartCore * npc, NandPartSpareLocation * location, uint32_t block)
{
        location->block[0] = (uint8_t) (block >>  0);
        location->block[1] = (uint8_t) (block >>  8);
        location->block[2] = (uint8_t) (block >> 16);
}

static bool
substituteSparesForBadBlocks(NandPartCore * npc)
{
        NandPartSubst * subst = &(npc->ptab->subst);
        bool ok = true;
        uint32_t idx = 0;
        uint32_t part;
        uint32_t bank;
        uint32_t block;

        spew(npc, "substituting spares for bad blocks");

        hostSetMem(npc, subst, 0x00, sizeof(NandPartSubst));
        subst->count = npc->ptab->spare.count;

        for (part = 0; ok && part < kNandPartEntryMaxCount; part++) {

                if (!isPartitionMapped(npc, part)) {
                        continue;
                }

                for (bank = 0; ok && bank < getBankWidth(npc, part); bank++) {
                        for (block = 0; ok && block < getBlockDepth(npc, part); block++) {

                                if (areFlagsSet(npc, part, kNandPartEntryFlagsTypePool) ||
                                    isBlockBad(npc, part, bank, block, false)) {

                                        ok = (subst->count > idx);
                                        require_action(ok, finish_up, bug(npc, "out of spares - idx:%u", idx));

                                        debug(npc, "substituting %u:%u:%u with spare idx:%u", part, bank, block, idx);
                                        subst->maps[idx++] = calcBlockMap(npc, part, bank, block, false);
                                }
                        }
                }
        }

        if (ok) {
                const size_t len = (sizeof(*subst) - sizeof(subst->checks));
                subst->magic = kNandPartSubstMagic;
                subst->flags = kNandPartSubstFlagCorrectedChecks;
                fletcher16(npc, &subst->checks[0], &subst->checks[1], subst, len);
        }

finish_up:
        return ok;
}

static void
initMapping(NandPartCore * npc)
{
        unsigned idx;
        NandPartMapping * mapping =  &(npc->mapping);
        uint32_t total = 0;

        spew(npc, "initializing block mappings");

        for (idx = 0; idx < kNandPartMappingMaxCount; idx++) {

                const uint32_t count = calcPartitionMapCount(npc, idx);
                mapping->counts[idx] = count;
                mapping->offsets[idx] = total;
                total += count;
        }
        mapping->total = total;

        if (isDebugEnabled(npc)) {
                hostSetDataProperty(npc, "mapping", mapping, sizeof(*mapping));
        }
}

static bool 
isPartitionMapped(NandPartCore * npc, uint32_t partIndex)
{
        // XXX: this is ugly and inflexible; introduce better flags
        // for recognizing mapped partition types

        const NandPartEntry * entry = getEntry(npc, partIndex);
        return((kNandPartContentBootBlock == entry->content) ||
               (kNandPartContentFirmware == entry->content) ||
               (kNandPartContentNVRAM == entry->content) ||
               (kNandPartContentEffaceable == entry->content));
}

static uint32_t
calcPartitionMapCount(NandPartCore * npc, uint32_t partIndex)
{
        if ((partIndex >= kNandPartEntryMaxCount) || !isPartitionMapped(npc, partIndex)) {
                return 0;
        }
        return(getBankWidth(npc, partIndex) * getBlockDepth(npc, partIndex));
}

static uint32_t
getPartitionMapOffset(NandPartCore * npc, uint32_t partIndex)
{
        return(npc->mapping.offsets[partIndex]);
}

static uint32_t
getTotalMapCount(NandPartCore * npc)
{
        return(npc->mapping.total);
}

static uint32_t 
calcBlockMap(NandPartCore * npc, uint32_t partIndex, uint32_t partBank, uint32_t partBlock, bool allowSpare)
{
        uint32_t map = (uint32_t)-1;

        require_action(isEntryIndexValid(npc, partIndex, false), finish_up, 
                       bug(npc, "out of bounds - partIndex:%u", partIndex));
        require_action(isPartitionMapped(npc, partIndex), finish_up,
                       bug(npc, "not mapped - partIndex:%u", partIndex));

        if (allowSpare) {

                uint32_t spareIndex = (uint32_t)-1;

                if (isBlockReplaced(npc, partIndex, partBank, partBlock, &spareIndex)) {
                        map = getTotalMapCount(npc) + spareIndex;
                } else {
                        map = calcBlockMap(npc, partIndex, partBank, partBlock, false);
                }

        } else {

                const uint32_t width = getBankWidth(npc, partIndex);
                const uint32_t depth = getBlockDepth(npc, partIndex);

                const uint32_t base = getPartitionMapOffset(npc, partIndex);
                const uint32_t row = (width * partBlock);
                const uint32_t col = partBank;

                require_action(width > partBank, finish_up,
                               bug(npc, "out of bounds - width:%u bank:%u", width, partBank));
                require_action(depth > partBlock, finish_up,
                               bug(npc, "out of bounds - depth:%u block:%u", depth, partBlock));

                map = (base + row + col);
        }

finish_up:
        return(map);
}

#if NAND_PART_CORE_ORPHAN
// previously only used for now obsolete assetion
static uint32_t
getPartitionMapCount(NandPartCore * npc, uint32_t partIndex)
{
        return(npc->mapping.counts[partIndex]);
}
#endif


// =============================================================================

#pragma mark - Bad Block Management

static bool
initCemetery(NandPartCore * npc, bool isNewborn)
{
        bool ok = true;
        NandPartCemetery * cemetery = &(npc->ptab->cemetery);

        debug(npc, "initializing cemetery state");

        if (isNewborn
            || ptabPrecedesFeature(npc->ptab, FuneralDirge)
            || isMaskSet(kNandPartDeviceFlagNeedsCemetery, npc->ptab->device.flags)) {

                hostSetMem(npc, cemetery, 0x00, sizeof(NandPartCemetery));
                cemetery->gate = kNandPartCemeteryGate;
                changeCemeteryLocks(npc, cemetery);
                npc->ptab->device.flags &= ~kNandPartDeviceFlagNeedsCemetery;

        } else if (!isCemeterySafe(npc, cemetery)) {

                error(npc, "cemetery is haunted");
                ok = false;
        }
        
        return(ok);
}

static void 
fletcher16(NandPartCore * npc, uint8_t *checkA, uint8_t *checkB, const void *buf, size_t len)
{
        // derived from: http://en.wikipedia.org/wiki/Fletcher%27s_checksum @ 2010-06-26

        const uint8_t * data = (uint8_t*)buf;
        uint16_t sum1 = 0xff, sum2 = 0xff;
 
        while (len) {
                size_t tlen = len > 21 ? 21 : len;
                len -= tlen;
                do {
                        sum1 += *data++;
                        sum2 += sum1;
                } while (--tlen);
                sum1 = (sum1 & 0xff) + (sum1 >> 8);
                sum2 = (sum2 & 0xff) + (sum2 >> 8);
        }

        /* Second reduction step to reduce sums to 8 bits */
        sum1 = (sum1 & 0xff) + (sum1 >> 8);
        sum2 = (sum2 & 0xff) + (sum2 >> 8);
        *checkA = (uint8_t)sum1;
        *checkB = (uint8_t)sum2;
}

static bool
isCemeterySafe(NandPartCore * npc, const NandPartCemetery *cemetery)
{
        bool ok = true;

        // is the cemetery gate present?
        ok = (kNandPartCemeteryGate == cemetery->gate);
        require_action(ok, finish_up, 
                       bug(npc, "missing gate - expected:0x%08X; found:0x%08X", 
                           kNandPartCemeteryGate, cemetery->gate));

        // XXX check to make certain none are marked dead beyond total mapped block count

        // check the locks, if they're not being explicitly ignored
        if (!isMaskSet(kNandPartCemeteryFlagIgnoreLocks, cemetery->flags)) {

                const size_t len = (sizeof(*cemetery) - sizeof(cemetery->locks));
                uint8_t checkA;
                uint8_t checkB;

                fletcher16(npc, &checkA, &checkB, cemetery, len);
                ok = ((checkA == cemetery->locks[0]) && (checkB == cemetery->locks[1]));
                require_action(ok, finish_up,
                               bug(npc, "broken locks - expected:{0x%02X, 0x%02X}; found:{0x%02X, 0x%02X}",
                                   checkA, checkB, cemetery->locks[0], cemetery->locks[1]));
        }

finish_up:
        return(ok);
}

static void
changeCemeteryLocks(NandPartCore * npc, NandPartCemetery * cemetery)
{
        const size_t len = (sizeof(*cemetery) - sizeof(cemetery->locks));
        cemetery->locks[0] = cemetery->locks[1] = 0;
        fletcher16(npc, &cemetery->locks[0], &cemetery->locks[1], cemetery, len);
}

static bool
mergePartitionCemetery(NandPartCore * npc, uint32_t partIndex, const NandPartCemetery * partCemetery, NandPartCemetery * mainCemetery)
{
        bool ok = false;
        const uint32_t numOfHeadstoneRows = kNandPartCemeteryPlotsSize / sizeof(uint32_t);
        uint32_t * oldPlots = (uint32_t *) mainCemetery->plots;
        uint32_t * newPlots = (uint32_t *) partCemetery->plots;
        uint32_t row;

        debug(npc, "merging cemetery from part:%u", partIndex);

        ok = (kNandPartCemeteryPlotsSize == (sizeof(uint32_t) * numOfHeadstoneRows));
        require_action(ok, finish_up, bug(npc, "expecting plot count evenly divisible by 32"));

        ok = isCemeterySafe(npc, mainCemetery);
        require_action(ok, finish_up, bug(npc, "main cemetery unsafe"));

        ok = isCemeterySafe(npc, partCemetery);
        require_action(ok, finish_up, bug(npc, "part:%u cemetery unsafe", partIndex));

        // apply diffs regadless of partition providing the information
        for (row = 0; row < numOfHeadstoneRows; row++) {

                // the dead must remain in their graves; if they
                // aren't, report the problem, and bury them again
                ok = isMaskSet(oldPlots[row], newPlots[row]);
                require_action(ok, finish_up, 
                               bug(npc, "dead have risen - oldPlots[%u]:0x%08x, newPlots[%u]:0x%08x", 
                                   row, oldPlots[row], row, newPlots[row]));
                        
                oldPlots[row] |= newPlots[row];
        }

        // update checksums
        changeCemeteryLocks(npc, mainCemetery);
        ok = isCemeterySafe(npc, mainCemetery);
        require_action(ok, finish_up, bug(npc, "main cemetery unsafe after merge"));

finish_up:
        return (ok);
}

static void 
markBlockDead(NandPartCore * npc, uint32_t partIndex, uint32_t partBank, uint32_t partBlock, bool allowSpare)
{
        const uint32_t map = calcBlockMap(npc, partIndex, partBank, partBlock, allowSpare);
        const uint32_t idx = (map / 32);
        const uint32_t mask = (((uint32_t)0x1) << (map % 32));  
        uint32_t * plots = (uint32_t*) npc->ptab->cemetery.plots;
        plots[idx] |= mask;
        changeCemeteryLocks(npc, &npc->ptab->cemetery);
        debug(npc, "block %u:%u:%u is dead", partIndex, partBank, partBlock);
}

static bool 
isBlockDead(NandPartCore * npc, uint32_t partIndex, uint32_t partBank, uint32_t partBlock, bool allowSpare)
{
        const uint32_t map = calcBlockMap(npc, partIndex, partBank, partBlock, allowSpare);
        const uint32_t idx = (map / 32);
        const uint32_t mask = (((uint32_t)0x1) << (map % 32));
        const uint32_t * plots = (uint32_t*) npc->ptab->cemetery.plots;
        const bool isDead = isMaskSet(mask, plots[idx]);
        if (isDead) {
                debug(npc, "block %u:%u:%u is dead", partIndex, partBank, partBlock);
        }
        return(isDead);
}

static NandPartFBBTCache *
getFBBTCache(NandPartCore * npc)
{
        NandPartFBBTCache * cache = NULL;
        if (0 != npc->ptab->fbbtCache.info.entryCount) {
                cache = &npc->ptab->fbbtCache;
        }
        return(cache);
}

static bool 
isBlockFactoryBad(NandPartCore * npc, uint32_t partIndex, uint32_t partBank, uint32_t partBlock, bool allowSpare)
{
        bool isBad = false;

        if (!isMaskSet(kNandPartEntryFlagsIgnoreFBBT, getEntry(npc, partIndex)->flags) &&
            !(allowSpare && isBlockReplaced(npc, partIndex, partBank, partBlock, NULL))) {

                uint32_t ce = mapNandCE(npc, partIndex, partBank, partBlock, allowSpare);
                uint32_t block = mapNandBlock(npc, partIndex, partBank, partBlock, allowSpare);
                NandPartFBBTCache * cache;

                isBad = ((kNandPartCEInvalid == ce) || (kNandPartBlockInvalid == block));
                require_action(!isBad, finish_up, bug(npc, "unable to map - part:%u bank:%u block:%u spare:%c",
                                                      partIndex, partBank, partBlock, bool2Char(allowSpare)));

                cacheFactoryBadBlockTable(npc);
                cache = getFBBTCache(npc);
                if ((NULL != cache) && (block < cache->info.blockDepth)) {

                        unsigned idx;

                        for (idx = 0; idx < cache->info.entryCount; idx++) {
                                if ((block == cache->entries[idx].blockAddr) &&
                                    (ce == cache->entries[idx].bankNum)) {
                                        isBad = true;
                                }
                        }

                } else {

                        isBad = hostIsBlockFactoryBad(npc, ce, block);
                }
        }

finish_up:
        return(isBad);
}

static void 
cacheFactoryBadBlockTable(NandPartCore * npc)
{
        uint32_t entryIdx = 0;
        uint32_t blockIdx;
        uint32_t ceIdx;

        // XXX - The FBBT cache is quite irrelevant anymore due to the
        // grown bad block table (a.k.a. cemetery) and, thus, should
        // be deprecated sometime.

        if (!npc->isFBBTCached) {
                for (blockIdx = 0; blockIdx < npc->provider->geometry.blocksPerCE; blockIdx++) {
                        for (ceIdx = 0; ceIdx < npc->provider->geometry.ceCount; ceIdx++) {
                                if ((kNandPartFBBTCacheEntryCount > entryIdx) &&
                                    (hostIsBlockFactoryBad(npc, ceIdx, blockIdx))) {
                                        npc->ptab->fbbtCache.entries[entryIdx].bankNum = ceIdx;
                                        npc->ptab->fbbtCache.entries[entryIdx].blockAddr = blockIdx;
                                        entryIdx++;
                                }
                        }
                        if (kNandPartFBBTCacheEntryCount <= entryIdx) {
                                break;
                        }
                }
                npc->ptab->fbbtCache.info.entryCount = entryIdx;
                npc->ptab->fbbtCache.info.blockDepth = blockIdx;
                npc->isFBBTCached = true;
        }
}


/* ========================================================================== */

#pragma mark - Partition Table I/O

static bool 
readPTab(NandPartCore * npc)
{
        bool             ok = false;
        uint32_t         ce;
        const uint32_t   block = 0;
        uint32_t         page;
        bool             isClean;

        debug(npc, "reading partition tables from boot blocks");
        for (ce = 0; !ok && (ce < npc->provider->geometry.ceCount); ce++) {
                for (page = 0; !ok && (page < kNandPartTableCopies); page++) {
                        const uint32_t address = calcNandAddress(npc, block, page, false);
                        spew(npc, "retrieving boot block information from %u:%u", ce, address);
                        if (!hostReadBootPage(npc, ce, address, npc->ptab, &isClean)) {
                                debug(npc, "unable to read boot block at %u:%u (%c)", ce, address, bool2Char(isClean));
                        } else {
                                spew(npc, "checking whether %u:%u contains valid partition table", ce, address);
                                if (validatePTab(npc, npc->ptab)) {
                                        info(npc, "read valid partition table");
                                        ok = true;
                                }
                        }
                }
        }

        if (!ok) {
                info(npc, "no partition table found");
        }

        return(ok);
};

static bool
validatePTab(NandPartCore * npc, const NandPartTable * ptab)
{
        bool ok = false;

        // Before anything else, the header magic must match in order
        // for partition table validation to succeed.
        ok = (kNandPartHeaderMagic == ptab->header.magic);
        require_action(ok, finish_up, error(npc, "bad magic - %c%c%c%c", 
                                            byteN(ptab->header.magic, 3), 
                                            byteN(ptab->header.magic, 2), 
                                            byteN(ptab->header.magic, 1), 
                                            byteN(ptab->header.magic, 0)));

        // Next, check the major version.  Major version changes only happen when
        // structural incompatibilities are introduced that cannot be supported
        // by older versions of the software; however, newer software is expected
        // to maintain compatibility with older versions of the structure.
        // Thus, the software's major version must be greater than or equal to
        // the structure's major version.
        ok = (kNandPartHeaderVersionMajor >= ptab->header.versionMajor);
        require_action(ok, finish_up, error(npc, "incompatible major version - 0x%08X",
                                            ptab->header.versionMajor));

        // Finally, in some cases, it might be valuable to warn about
        // minor version differences; however, this should never cause
        // structure validation to fail.
        if (kNandPartHeaderVersionMinor > ptab->header.versionMinor) {
                warn(npc, "minor version on device, 0x%08X, older than driver, 0x%08X",
                     ptab->header.versionMinor, kNandPartHeaderVersionMinor);
        } else if (kNandPartHeaderVersionMinor < ptab->header.versionMinor) {
                warn(npc, "minor version on device, 0x%08X, newer than driver, 0x%08X",
                     ptab->header.versionMinor, kNandPartHeaderVersionMinor);
        }

        npc->isPTabValid = true;
        
finish_up:
        return(ok);
};

static bool 
writePTab(NandPartCore * npc)
{
        bool            ok = false;
        uint32_t        part;
        uint32_t        ce;
        const uint32_t  block = 0;
        uint32_t        page;
        uint32_t        goodCopies = 0;
        uint32_t        len;
        bool            isClean;
        void *          buf = NULL;

        ok = findBootPartIndex(npc, &part);
        require_action(ok, finish_up, bug(npc, "missing boot partition"));

        // Acquire temp buffer for readback verification.
        len = getBytesPerPage(npc, part);
        buf = hostAllocMem(npc, len, true);
        ok = (NULL != buf);
        require_action(ok, finish_up, bug(npc, "alloc failed"));

        // Update minor version information on partition table every write.
        npc->ptab->header.versionMinor = kNandPartHeaderVersionMinor;

        // Increment generation information with every write.
        npc->ptab->header.generation++;

        // Make certain factory BBT cache is current.
        cacheFactoryBadBlockTable(npc);

        for (ce = 0; ce < npc->provider->geometry.ceCount; ce++) {
                const uint32_t blockAddr = calcNandAddress(npc, block, 0, false);
                if (!hostEraseBlock(npc, ce, blockAddr, block)) {
                        warn(npc, "failed erase of boot block %u:%u", ce, blockAddr);
                } else {

                        for (page = 0; page < kNandPartTableCopies; page++) {
                                const uint32_t pageAddr = calcNandAddress(npc, block, page, false);
                                if (!hostWriteBootPage(npc, ce, pageAddr, npc->ptab)) {
                                        warn(npc, "failed write to boot page %u:%u", ce, pageAddr);
                                }
                        }
                        
                        // perform readback and verification                        
                        for (page = 0; page < kNandPartTableCopies; page++) {
                                const uint32_t pageAddr = calcNandAddress(npc, block, page, false);
                                if (!hostReadBootPage(npc, ce, pageAddr, buf, &isClean)) {
                                        warn(npc, "failed read from boot page %u:%u (%c)", ce, pageAddr, bool2Char(isClean));
                                } else if (0 != hostCompareMem(npc, npc->ptab, buf, len)) {
                                        warn(npc, "failed compare for boot page %u:%u", ce, pageAddr);
                                } else {
                                        goodCopies++;
                                }
                        }
                }
        }

        if (0 < goodCopies) {
                ok = true;
                info(npc, "first %u pages of boot blocks written with copies of partition table", 
                     kNandPartTableCopies);
        }

finish_up:
        if (NULL != buf) {
                hostFreeMem(npc, buf, len);
                buf = NULL;
        }
        
        return(ok);
};


/* ========================================================================== */

#pragma mark - Partition Table Diffs

static void
initDiffSequence(NandPartCore * npc)
{
        unsigned idx;
        for (idx = 0; idx < kNandPartEntryMaxCount; idx++) {
                npc->diffSequence[idx] = 0;
        }
}

static bool
validatePTabDiff(NandPartCore * npc, NandPartDiff * diff, unsigned size)
{
        bool ok = true;

        ok = (sizeof(NandPartDiffHeader) <= size);
        require_action(ok, finish_up, bug(npc, "diff way too small"));

        ok = (kNandPartDiffMagic == diff->header.magic);
        require_action(ok, finish_up, warn(npc, "bad diff magic"));

        ok = (sizeof(NandPartDiff) == diff->header.size);
        require_action(ok, finish_up, bug(npc, "bad diff size"));

        ok = (diff->header.size <= size);
        require_action(ok, finish_up, bug(npc, "diff still too small"));

        ok = (diff->header.versionMajor == npc->ptab->header.versionMajor);
        require_action(ok, finish_up, bug(npc, "diff major version mismatch - diff: %u, ptab: %u",
                                          diff->header.versionMajor, npc->ptab->header.versionMajor));

        if (diff->header.generation < npc->ptab->header.generation) {
                warn(npc, "diff generation earlier than ptab - diff: %u, ptab: %u",
                     diff->header.generation, npc->ptab->header.generation);
        }
        
        if (0 != diff->header.sequence) {
                warn(npc, "diff has sequence of 0");
        }

        if (!isMaskSet(kNandPartDiffFlagIgnoreChecks, diff->header.flags)) {

                const size_t len = (diff->header.size - sizeof(diff->checks));
                uint8_t checkA;
                uint8_t checkB;

                fletcher16(npc, &checkA, &checkB, diff, len);
                ok = ((checkA == diff->checks[0]) && (checkB == diff->checks[1]));
                require_action(ok, finish_up, error(npc, "diff checksum fail"));
        }

        debug(npc, "ptab diff is valid");

finish_up:
        return ok;
}


// =============================================================================

#pragma mark - Structure Dumping

static void
dumpNandGeometry(NandPartCore * npc, NandPartGeometry * geometry)
{
        debug(npc, "NandPartGeometry @        %p: {", geometry);
        debug(npc, "    isPpn:                %c", bool2Char(geometry->isPpn));
        debug(npc, "    ceCount:              %u", geometry->ceCount);
        debug(npc, "    blocksPerCE:          %u", geometry->blocksPerCE);
        debug(npc, "    blockStride:          %u", geometry->blockStride);
        debug(npc, "    pagesPerMLCBlock:     %u", geometry->pagesPerMLCBlock);
        debug(npc, "    pagesPerSLCBlock:     %u", geometry->pagesPerSLCBlock);
        debug(npc, "    bytesPerBootPage:     %u", geometry->bytesPerBootPage);
        debug(npc, "    bytesPerFullPage:     %u", geometry->bytesPerFullPage);
        debug(npc, "    bytesPerFullPageMeta: %u", geometry->bytesPerFullPageMeta);
        debug(npc, "    bitsPerCauAddr:       %u", geometry->bitsPerCauAddr);
        debug(npc, "    bitsPerPageAddr:      %u", geometry->bitsPerPageAddr);
        debug(npc, "    bitsPerBlockAddr:     %u", geometry->bitsPerBlockAddr);
        debug(npc, "}");
}

static void
dumpPTabHeader(NandPartCore * npc, NandPartHeader * header)
{
        debug(npc, "NandPartHeader @    %p: {", header);
        debug(npc, "    magic:          '%c%c%c%c'", byteN(header->magic, 3), byteN(header->magic, 2), byteN(header->magic, 1), byteN(header->magic, 0));
        debug(npc, "    should_be_zero: 0x%08x", header->should_be_zero);
        debug(npc, "    versionMajor:   0x%08x", header->versionMajor);
        debug(npc, "    versionMinor:   0x%08x", header->versionMinor);
        debug(npc, "    generation:     0x%08x", header->generation);
        debug(npc, "    reserved[]:     {");
        debug(npc, "                        0x%02x, 0x%02x, 0x%02x, 0x%02x,", header->reserved[ 0], header->reserved[ 1], header->reserved[ 2], header->reserved[ 3]);
        debug(npc, "                        0x%02x, 0x%02x, 0x%02x, 0x%02x,", header->reserved[ 4], header->reserved[ 5], header->reserved[ 6], header->reserved[ 7]);
        debug(npc, "                        0x%02x, 0x%02x, 0x%02x, 0x%02x" , header->reserved[ 8], header->reserved[ 9], header->reserved[10], header->reserved[11]);
        debug(npc, "    }");
        debug(npc, "}");
}

static void
dumpCemetery(NandPartCore * npc, NandPartCemetery * cemetery)
{
        debug(npc, "NandPartCemetery @ %p: {", cemetery);
        debug(npc, "    gate:  0x%08X", cemetery->gate);
        debug(npc, "    flags: 0x%02X", cemetery->flags);
        debug(npc, "    plots: {");
        debug(npc, "               0x%02X, 0x%02X, 0x%02X, 0x%02X,", cemetery->plots[ 0], cemetery->plots[ 1], cemetery->plots[ 2], cemetery->plots[ 3]);
        debug(npc, "               0x%02X, 0x%02X, 0x%02X, 0x%02X,", cemetery->plots[ 4], cemetery->plots[ 5], cemetery->plots[ 6], cemetery->plots[ 7]);
        debug(npc, "               0x%02X, 0x%02X, 0x%02X, 0x%02X,", cemetery->plots[ 8], cemetery->plots[ 9], cemetery->plots[10], cemetery->plots[11]);
        debug(npc, "               0x%02X, 0x%02X, 0x%02X, 0x%02X,", cemetery->plots[12], cemetery->plots[13], cemetery->plots[14], cemetery->plots[15]);
        debug(npc, "               0x%02X, 0x%02X, 0x%02X, 0x%02X,", cemetery->plots[16], cemetery->plots[17], cemetery->plots[18], cemetery->plots[19]);
        debug(npc, "               0x%02X, 0x%02X, 0x%02X, 0x%02X,", cemetery->plots[20], cemetery->plots[21], cemetery->plots[22], cemetery->plots[23]);
        debug(npc, "               0x%02X, 0x%02X, 0x%02X, 0x%02X,", cemetery->plots[24], cemetery->plots[25], cemetery->plots[26], cemetery->plots[27]);
        debug(npc, "               0x%02X, 0x%02X, 0x%02X, 0x%02X", cemetery->plots[28], cemetery->plots[29], cemetery->plots[30], cemetery->plots[31]);
        debug(npc, "    }");
        debug(npc, "    locks: { 0x%02X, 0x%02X }", cemetery->locks[0], cemetery->locks[1]);
        debug(npc, "}");
}

static void
dumpSpare(NandPartCore * npc, NandPartSpare * spare)
{
        unsigned idx;
        debug(npc, "NandPartSpare @    %p: {", spare);
        debug(npc, "    magic:         '%c%c%c%c'", byteN(spare->magic, 3), byteN(spare->magic, 2), byteN(spare->magic, 1), byteN(spare->magic, 0));
        debug(npc, "    count:         %u", spare->count);
        debug(npc, "    flags:         0x%02x", spare->flags);
        debug(npc, "    reserved:      { 0x%02x, 0x%02x, 0x%02x }", spare->reserved[0], spare->reserved[1], spare->reserved[2]);
        debug(npc, "    locations[]:   {");
        for (idx = 0; idx < kNandPartSpareMaxCount; idx++) {
                debug(npc, "        [%2d]:          { block: 0x%06x, ce: %u }", 
                      idx, getSpareLocationBlock(npc, &(spare->locations[idx])), spare->locations[idx].ce);
        }
        debug(npc, "    }");
        debug(npc, "    checks:        { 0x%02x, 0x%02x }", spare->checks[0], spare->checks[1]);
        debug(npc, "}");
}

static void
dumpSubst(NandPartCore * npc, NandPartSubst * subst)
{
        unsigned idx;
        debug(npc, "NandPartSubst @    %p: {", subst);
        debug(npc, "    magic:         '%c%c%c%c'", byteN(subst->magic, 3), byteN(subst->magic, 2), byteN(subst->magic, 1), byteN(subst->magic, 0));
        debug(npc, "    count:         %u", subst->count);
        debug(npc, "    flags:         0x%02x", subst->flags);
        debug(npc, "    reserved:      { 0x%02x, 0x%02x, 0x%02x }", subst->reserved[0], subst->reserved[1], subst->reserved[2]);
        debug(npc, "    maps[]:        {");
        for (idx = 0; idx < kNandPartSubstMaxCount; idx++) {
                debug(npc, "        [%2d]:          %u", idx, subst->maps[idx]);
        }
        debug(npc, "    }");
        debug(npc, "    checks:        { 0x%02x, 0x%02x }", subst->checks[0], subst->checks[1]);
        debug(npc, "}");
}

static void 
dumpEntries(NandPartCore * npc, NandPartEntry * entries)
{
        uint32_t part;

        debug(npc, "NandPartEntry[] @ %p: {", entries);
        for (part = 0; part < kNandPartEntryMaxCount; part++) {

                debug(npc, "    [%u]: {", part);                
                debug(npc, "        content:      '%c%c%c%c'", 
                      byteN(entries[part].content, 3), 
                      byteN(entries[part].content, 2), 
                      byteN(entries[part].content, 1), 
                      byteN(entries[part].content, 0));
                
                if (areFlagsSet(npc, part, kNandPartEntryFlagsTypeSlice)) {

                        debug(npc, "        slice.offset: %u", entries[part].slice.offset);
                        debug(npc, "        slice.depth:  %u", entries[part].slice.depth);

                } else if (areFlagsSet(npc, part, kNandPartEntryFlagsTypePool)) {

                        debug(npc, "        pool.width:   %u", entries[part].pool.width);
                        debug(npc, "        pool.depth:   %u", entries[part].pool.depth);

                } else {

                        debug(npc, "        reserved_0:   %u", entries[part].reserved_0);
                        debug(npc, "        reserved_1:   %u", entries[part].reserved_1);
                }
                
                if (isPartitionMapped(npc, part)) {
                        
                        debug(npc, "        flags:        0x%08x (usingFullPages:%c usingSLCBlocks:%c)", 
                              entries[part].flags, bool2Char(usingFullPages(npc, part)), bool2Char(usingSLCBlocks(npc, part)));

                } else {

                        debug(npc, "        flags:        0x%08x", entries[part].flags);
                }

                debug(npc, "    }");
        }
        debug(npc, "}");
}

static void
dumpPTab(NandPartCore * npc, NandPartTable * ptab)
{
        dumpPTabHeader(npc, &(ptab->header));
        dumpCemetery(npc, &(ptab->cemetery));
        dumpSpare(npc, &(ptab->spare));
        dumpSubst(npc, &(ptab->subst));
        dumpEntries(npc, &(ptab->entries[0]));
}

static void
dumpMapping(NandPartCore * npc, NandPartMapping * mapping)
{
        unsigned idx;

        debug(npc, "NandPartMapping @ %p: {", mapping);
        debug(npc, "    total:        %u", mapping->total);
        debug(npc, "    counts[]:     {");
        for (idx = 0; idx < kNandPartMappingMaxCount; idx++) {
                debug(npc, "           [%3u]:      %u", idx, mapping->counts[idx]);
        }
        debug(npc, "    }");
        debug(npc, "    offsets[]:    {");
        for (idx = 0; idx < kNandPartMappingMaxCount; idx++) {
                debug(npc, "           [%3u]:      %u", idx, mapping->offsets[idx]);
        }
        debug(npc, "    }");
        debug(npc, "}");
}

static void
dumpReplacements(NandPartCore * npc, NandPartReplacement * replacements)
{
        unsigned idx;

        debug(npc, "NandPartReplacement[] @ %p: {", replacements);
        for (idx = 0; idx < kNandPartReplacementMaxCount; idx++) {
                const NandPartReplacement *replacement = getMappedReplacement(npc, idx);
                require_action(NULL != replacement, finish_up, bug(npc, "unmapped replacement - idx:%u", idx));
                debug(npc, "    [%3u]: { isValid: %c; isReplaced: %c; spareIndex: %2u }", 
                      idx, bool2Char(replacement->isValid), bool2Char(replacement->isReplaced), replacement->spareIndex);
        }
        debug(npc, "}");

finish_up:
        return;
}

static void 
dumpSeparator(NandPartCore * npc, unsigned char sep, unsigned cnt)
{
        unsigned idx;
        for (idx = 0; idx < cnt; idx++) {
                debugCont(npc, "%c", sep);
        }
}

static void 
dumpTranslationTables(NandPartCore * npc)
{
        uint32_t partIndex;
        uint32_t partBank;
        uint32_t partBlock;

        dumpMapping(npc, &(npc->mapping));
        dumpReplacements(npc, &(npc->replacements[0]));

        debug(npc, "logical to physical translation tables");

        for (partIndex = 0; partIndex < kNandPartEntryMaxCount; partIndex++) {
                if (isPartitionMapped(npc, partIndex)) {

                        // output translation table header
                        debugHead(npc, "    partIndex: %2d", partIndex);
                        for (partBank = 0; partBank < getBankWidth(npc, partIndex); partBank++) {
                                debugCont(npc, " | partBank: %2d", partBank);
                                dumpSeparator(npc, ' ', 35);
                        }
                        debugCont(npc, "\n");

                        for (partBlock = 0; partBlock < getBlockDepth(npc, partIndex); partBlock++) {

                                // output translation table row seperator
                                debugHead(npc, "    ");
                                dumpSeparator(npc, '-', 13);
                                for (partBank = 0; partBank < getBankWidth(npc, partIndex); partBank++) {
                                        debugCont(npc, "-+");
                                        dumpSeparator(npc, '-', 48);
                                }
                                debugCont(npc, "\n");

                                // output translation table row
                                debugHead(npc, "    partBlock: %2d", partBlock);
                                for (partBank = 0; partBank < getBankWidth(npc, partIndex); partBank++) {
                                        debugCont(npc, " | { ce: %2d, block: 0x%08x, addr: 0x%08x }", 
                                                  mapNandCE(npc, partIndex, partBank, partBlock, true), 
                                                  mapNandBlock(npc, partIndex, partBank, partBlock, true), 
                                                  calcNandAddress(npc, mapNandBlock(npc, partIndex, partBank, partBlock, true), 
                                                                  0, usingSLCBlocks(npc, partIndex)));
                                }
                                debugCont(npc, "\n");
                        }

                        debugHead(npc, "    ");
                        dumpSeparator(npc, '=', 15);
                        debugCont(npc, "\n");
                }
        }
}

extern void
nand_part_dump(NandPartInterface * interface)
{
        NandPartCore * npc = (NandPartCore *)interface->core;
        dumpNandGeometry(npc, &(npc->provider->geometry));
        dumpPTab(npc, npc->ptab);
        dumpTranslationTables(npc);
}


/* ========================================================================== */

#pragma mark - Interface Implementation

static const NandPartEntry *
getEntry(NandPartCore * npc, uint32_t partIndex)
{
        NandPartEntry * entry = NULL;

        require_action(isEntryIndexValid(npc, partIndex, true), finish_up, 
                       bug(npc, "out of bounds - part:%u", partIndex));

        if (isRawEntryIndex(npc, partIndex)) {
                debug(npc, "part:%u indicates unformatted nand", partIndex);
                entry = &(npc->rawMedia);
        } else {
                entry = &(npc->ptab->entries[partIndex]);
        }

finish_up:
        return entry;
}

static uint32_t
getBankWidth(NandPartCore * npc, uint32_t partIndex)
{
        uint32_t width = 0;

        require_action(isEntryIndexValid(npc, partIndex, true), finish_up, 
                       bug(npc, "out of bounds - part:%u", partIndex));

        if ((isRawEntryIndex(npc, partIndex)) || 
            (areFlagsSet(npc, partIndex, kNandPartEntryFlagsTypeSlice))) {
                width = npc->provider->geometry.ceCount;
        } else if (areFlagsSet(npc, partIndex, kNandPartEntryFlagsTypePool)) {
                width = getEntry(npc, partIndex)->pool.width;
        } else {
                warn(npc, "invalid request for bank width of partition 0x%08x", partIndex);
        }

finish_up:
        return(width);
}

static uint32_t
getBlockDepth(NandPartCore * npc, uint32_t partIndex)
{
        uint32_t depth = 0;

        require_action(isEntryIndexValid(npc, partIndex, true), finish_up, 
                       bug(npc, "out of bounds - part:%u", partIndex));

        if (isRawEntryIndex(npc, partIndex)) {
                depth = npc->provider->geometry.blocksPerCE;
        } else if (areFlagsSet(npc, partIndex, kNandPartEntryFlagsTypeSlice)) {
                depth = getEntry(npc, partIndex)->slice.depth;
        } else if (areFlagsSet(npc, partIndex, kNandPartEntryFlagsTypePool)) {
                depth = getEntry(npc, partIndex)->pool.depth;
        } else {
                warn(npc, "invalid request for block depth of partition 0x%08x", partIndex);
        }

finish_up:
        return(depth);
}

static uint32_t
getPagesPerBlock(NandPartCore * npc, uint32_t partIndex)
{
        uint32_t count = 0;

        require_action(isEntryIndexValid(npc, partIndex, true), finish_up, 
                       bug(npc, "out of bounds - part:%u", partIndex));

        if (usingSLCBlocks(npc, partIndex)) {
                count = npc->provider->geometry.pagesPerSLCBlock;
        } else {
                count = npc->provider->geometry.pagesPerMLCBlock;
        }

finish_up:
        return(count);
}

static uint32_t
getBytesPerPage(NandPartCore * npc, uint32_t partIndex)
{
        uint32_t count = 0;

        require_action(isEntryIndexValid(npc, partIndex, true), finish_up, 
                       bug(npc, "out of bounds - part:%u", partIndex));

        if (usingFullPages(npc, partIndex)) {
                count = npc->provider->geometry.bytesPerFullPage;
        } else {
                count = npc->provider->geometry.bytesPerBootPage;
        }

finish_up:
        return(count);
}

static bool 
isBlockBad(NandPartCore * npc, uint32_t partIndex, uint32_t partBank, uint32_t partBlock, bool allowSpare)
{
        bool bad = true;

        require_action(isEntryIndexValid(npc, partIndex, true), finish_up, 
                       bug(npc, "out of bounds - part:%u", partIndex));

        bad = (isBlockDead(npc, partIndex, partBank, partBlock, allowSpare) || 
               isBlockFactoryBad(npc, partIndex, partBank, partBlock, allowSpare));

        if (bad) {
                spew(npc, "bad block - part:%u bank:%u block:%u spare:%c", 
                     partIndex, partBank, partBlock, bool2Char(allowSpare));
        }

finish_up:
        return bad;
}

static bool 
readPage(NandPartCore * npc, uint32_t partIndex, uint32_t partBank, uint32_t partBlock, uint32_t pageInBlock, void * buf, bool * isClean, bool allowSpare)
{
        bool ok = false;
        uint32_t ce;
        uint32_t block;
        uint32_t addr;

        ok = isEntryIndexValid(npc, partIndex, false);
        require_action(ok, finish_up, bug(npc, "out of bounds - part:%u", partIndex));

        ce = mapNandCE(npc, partIndex, partBank, partBlock, allowSpare);
        block = mapNandBlock(npc, partIndex, partBank, partBlock, allowSpare);
        addr = calcNandAddress(npc, block, pageInBlock, usingSLCBlocks(npc, partIndex));

        ok = ((kNandPartCEInvalid != ce) && (kNandPartBlockInvalid != block));
        require_action(ok, finish_up, bug(npc, "unable to map - part:%u bank:%u block:%u spare:%c",
                                          partIndex, partBank, partBlock, bool2Char(allowSpare)));

        if (npc->provider->params.isClientTraceEnabled) {
                debug(npc, "readPage(%u, %u, %u, %u, %p, %p, %c)", 
                      partIndex, partBank, partBlock, pageInBlock, buf, isClean, bool2Char(allowSpare));
        }

        if (usingFullPages(npc, partIndex)) {

                ok = hostReadFullPage(npc, ce, addr, buf, resetMetaBuf(npc), isClean);
                require(ok, finish_up);

                // given that driver has not always historically been
                // specifying meta buffer content for full page
                // operations, it is not considered a failure of the
                // read operation if this verification fails;
                // therefore, perhaps this should be done only when
                // debug is enabled
                verifyMetaBuf(npc, partIndex);

        } else {
                ok = hostReadBootPage(npc, ce, addr, buf, isClean);
        }

finish_up:
        if (npc->provider->params.isClientTraceEnabled) {
                debug(npc, "readPage => %c (%c)", bool2Char(ok), isClean ? bool2Char(*isClean) : '-');
        }
        return(ok);
}

static bool 
writePage(NandPartCore * npc, uint32_t partIndex, uint32_t partBank, uint32_t partBlock, uint32_t pageInBlock, void * buf, bool allowSpare)
{
        bool ok = false;
        uint32_t ce;
        uint32_t block;
        uint32_t addr;

        ok = isEntryIndexValid(npc, partIndex, false);
        require_action(ok, finish_up, bug(npc, "out of bounds - part:%u", partIndex));

        ce = mapNandCE(npc, partIndex, partBank, partBlock, allowSpare);
        block = mapNandBlock(npc, partIndex, partBank, partBlock, allowSpare);
        addr = calcNandAddress(npc, block, pageInBlock, usingSLCBlocks(npc, partIndex));

        ok = ((kNandPartCEInvalid != ce) && (kNandPartBlockInvalid != block));
        require_action(ok, finish_up, bug(npc, "unable to map - part:%u bank:%u block:%u spare:%c",
                                          partIndex, partBank, partBlock, bool2Char(allowSpare)));

        ok = !isBlockBad(npc, partIndex, partBank, partBlock, allowSpare);
        require(ok, finish_up);

        if (npc->provider->params.isClientTraceEnabled) {
                debug(npc, "writePage(%u, %u, %u, %u, %p, %c)", 
                      partIndex, partBank, partBlock, pageInBlock, buf, bool2Char(allowSpare));
        }

        if (usingFullPages(npc, partIndex)) {
                ok = hostWriteFullPage(npc, ce, addr, buf, endorseMetaBuf(npc, partIndex));
        } else {
                ok = hostWriteBootPage(npc, ce, addr, buf);
        }

        if (!ok) {
                // XXX - double-check with NAND team that this is correct response to a page write failure
                markBlockDead(npc, partIndex, partBank, partBlock, allowSpare);
        }

finish_up:
        if (npc->provider->params.isClientTraceEnabled) {
                debug(npc, "writePage => %c", bool2Char(ok));
        }
        return(ok);
}

static bool 
eraseBlock(NandPartCore * npc, uint32_t partIndex, uint32_t partBank, uint32_t partBlock, bool allowSpare)
{
        bool ok = false;
        uint32_t ce;
        uint32_t block;
        uint32_t addr;

        ok = isEntryIndexValid(npc, partIndex, false);
        require_action(ok, finish_up, bug(npc, "out of bounds - part:%u", partIndex));

        ce = mapNandCE(npc, partIndex, partBank, partBlock, allowSpare);
        block = mapNandBlock(npc, partIndex, partBank, partBlock, allowSpare);
        addr = calcNandAddress(npc, block, 0, usingSLCBlocks(npc, partIndex));

        ok = ((kNandPartCEInvalid != ce) && (kNandPartBlockInvalid != block));
        require_action(ok, finish_up, bug(npc, "unable to map - part:%u bank:%u block:%u spare:%c",
                                          partIndex, partBank, partBlock, bool2Char(allowSpare)));

        ok = !isBlockBad(npc, partIndex, partBank, partBlock, allowSpare);
        require(ok, finish_up);

        if (npc->provider->params.isClientTraceEnabled) {
                debug(npc, "eraseBlock(%u, %u, %u, %c)", 
                      partIndex, partBank, partBlock, bool2Char(allowSpare));
        }

        ok = hostEraseBlock(npc, ce, addr, block);

        if (!ok) {
                markBlockDead(npc, partIndex, partBank, partBlock, allowSpare);
        }

finish_up:
        if (npc->provider->params.isClientTraceEnabled) {
                debug(npc, "eraseBlock => %c", bool2Char(ok));
        }
        return(ok);
}

static bool 
requestPTabDiff(NandPartCore * npc, uint32_t partIndex, void * buf, unsigned size)
{
        const unsigned diffSize = sizeof(NandPartDiff);
        NandPartDiff * diff = (NandPartDiff*)buf;
        bool ok = false;

        ok = isEntryIndexValid(npc, partIndex, false);
        require_action(ok, finish_up, bug(npc, "out of bounds - part:%u", partIndex));

        if (diffSize <= size) {

                // start clean
                hostSetMem(npc, diff, 0, diffSize);

                // tag data blob as one of our diffs
                diff->header.magic = kNandPartDiffMagic;
                diff->header.size = diffSize;
                diff->header.sequence = ++(npc->diffSequence[partIndex]);

                // record important info from current partition table
                // header to later be able to track whether it is
                // applicable to merge the diff
                diff->header.versionMajor = npc->ptab->header.versionMajor;
                diff->header.versionMinor = npc->ptab->header.versionMinor;
                diff->header.generation = npc->ptab->header.generation;
                
                // record current state of cemetery, so that we don't
                // forget about blocks that have died since last restore
                hostMoveMem(npc, &diff->cemetery, &npc->ptab->cemetery, sizeof(NandPartCemetery));

                // apply checksums
                fletcher16(npc, &diff->checks[0], &diff->checks[1], diff, diffSize - sizeof(diff->checks));

                ok = true;
        }

finish_up:
        return(ok);
};

static bool 
providePTabDiff(NandPartCore * npc, uint32_t partIndex, const void * buf, unsigned size)
{
        NandPartDiff * diff = (NandPartDiff*)buf;
        bool ok = false;

        ok = isEntryIndexValid(npc, partIndex, false);
        require_action(ok, finish_up, bug(npc, "out of bounds - part:%u", partIndex));

        ok = validatePTabDiff(npc, diff, size);
        require_action(ok, finish_up, warn(npc, "ignoring invalid diff - part:%u", partIndex));

        ok = mergePartitionCemetery(npc, partIndex, &diff->cemetery, &npc->ptab->cemetery);
        require_action(ok, finish_up, error(npc, "failed merge - part:%u", partIndex));

        npc->diffSequence[partIndex] = diff->header.sequence;
        debug(npc, "applied ptab diff provided by part:%u", partIndex);

finish_up:                     
        return(ok);
};

static bool
formatPTab(NandPartCore * npc)
{
        bool result = true;

        if (!isFormatEnabled(npc)) {
                bug(npc, "partition table format not allowed in this context");
                result = false;
        } else if (!validatePTab(npc, npc->ptab)) {
                error(npc, "partition table invalid");
                result = false;
        } else {
                npc->isFBBTCached = false;
                if (!createPartitionDevices(npc, true)) {
                        error(npc, "no partition devices created");
                        result = false;
                } else {
                        debug(npc, "copied new partition table and created specified partitions");
                        hostSetBooleanProperty(npc, kNandPartCoreIsFormattedKey, true);
                }
        }

        return(result);
}

static unsigned 
getPTabBytes(NandPartCore * npc, void * buf, unsigned size)
{
        unsigned bytesRead = 0;
        if (npc->isPTabValid) {
                bytesRead = (size > sizeof(NandPartTable)) ? sizeof(NandPartTable) : size;
                hostMoveMem(npc, buf, npc->ptab, bytesRead);
        }
        return bytesRead;
}

static bool
loadPartitionTable(NandPartCore * npc)
{
        bool ok = false;

        // We scan all of the available banks looking for partition tables.
        if (!readPTab(npc)) {
                
                npc->isFBBTCached = false;

                // If no partition table is found, create a single "unformatted" partition.
                ok = createUnformattedPartitionDevice(npc);
                require_action(ok, finish_up, bug(npc, "unable to create unformatted device"));

                debug(npc, "created unformatted device");
                hostSetBooleanProperty(npc, kNandPartCoreIsFormattedKey, false);

                if (isFormatEnabled(npc)) {
                        ok = formatWithSpecs(npc);
                }

                hostSetNumberProperty(npc, kNandPartCoreMaxSparesKey, countMaxSpares(npc), 32);

        } else {

                hostSetNumberProperty(npc, kNandPartCoreMaxSparesKey, countMaxSpares(npc), 32);

                // Force recache of factory BBT information iff version on
                // partition table precedes the FBBTCache feature.
                if (ptabPrecedesFeature(npc->ptab, FBBTCache)) {
                        npc->isFBBTCached = true;
                } else {
                        npc->isFBBTCached = false;
                }

                // Add kNandPartEntryFlagsIgnoreFBBT flag if necessary.
                addIgnoreFBBTFlag(npc);

                // Make certain each entry explicitly identifies partition type
                // iff version on partition table precedes the PoolPartition feature.
                makeEntryTypesExplicit(npc);

                // Otherwise, publish the partitions defined by the partition table.
                ok = createPartitionDevices(npc, false);
                require_action(ok, finish_up, bug(npc, "failed to create partition devices"));

                debug(npc, "created devices for partition entries");
                hostSetBooleanProperty(npc, kNandPartCoreIsFormattedKey, true);
        }

finish_up:
        return(ok);
}

static bool 
eraseBootPartition(NandPartCore * npc)
{
        bool ok = true;

        ok = isBootPartErasable(npc);
        require_action(ok, finish_up, bug(npc, "boot partition erase not allowed in this context"));

        if (!npc->isBootPartErased) {

                // Write partition table to nand, thereby erasing boot
                // partition as a side effect.
                ok = writePTab(npc);
                require_action(ok, finish_up, error(npc, "unable to erase boot partition"));

                npc->isBootPartErased = true;
        }

finish_up:
        return(ok);
}

static bool 
writeBootPartition(NandPartCore * npc, uint8_t * llbBuf, unsigned llbSize)
{
        bool            ok = false;
        uint8_t *       tempBuf = NULL;
        const bool      isPpn = isPpnDevice(npc);
        const size_t    fullPageSize = npc->provider->geometry.bytesPerFullPage;
        const size_t    bootPageSize = npc->provider->geometry.bytesPerBootPage;
        const uint32_t  numAvailPages = getPagesPerBlock(npc, npc->bootIdx);
        const uint32_t  numPtabPages = kNandPartTableCopies;
        const uint32_t  numCompletePages = (llbSize / bootPageSize);
        const size_t    partialPageSize = (llbSize % bootPageSize);
        const size_t    fillSize = (bootPageSize - partialPageSize);
        const uint32_t  numPartialPages = ((0 == partialPageSize) ? 0 : 1);
        const uint32_t  numLlbPages = (numCompletePages + numPartialPages);
        const uint32_t  numPages = (numPtabPages + numLlbPages);
        const uint32_t  lastPage = (isPpn ? numAvailPages : numPages) - 1;
        const uint32_t  block = 0;
        size_t          remaining = llbSize;
        size_t          offset = 0;
        uint32_t        page;
        uint32_t        ce;

        require_action(npc->bootIdx < kNandPartEntryMaxCount, clean_up, bug(npc, "missing boot partition"));
        require_action(isBootPartWritable(npc), clean_up, bug(npc, "boot partition write not allowed in this context"));
        require_action(numPages <= numAvailPages, clean_up, error(npc, "LLB is too large; can't write boot partition"));

        ok = eraseBootPartition(npc);
        require_action(ok, clean_up, error(npc, "failed erase of boot partition"));

        tempBuf = (uint8_t*)hostAllocMem(npc, fullPageSize, true);
        ok = (NULL != tempBuf);
        require_action(ok, clean_up, bug(npc, "alloc failed"));

        npc->isBootPartErased = false;
        for (page = kNandPartTableCopies; page <= lastPage; page++) {

                const uint32_t addr = calcNandAddress(npc, block, page, false);
                const bool isEmpty = (remaining == 0);
                const bool isPartial = (!isEmpty && (remaining < bootPageSize));
                const size_t copySize = (isEmpty ? 0 : (isPartial ? partialPageSize : bootPageSize));
                uint32_t goodCopies = 0;
                uint8_t * pageBuf = NULL;

                if (isEmpty) {

                        ok = ((numPages <= page) && (page < numAvailPages));
                        require_action(ok, clean_up, bug(npc, "bad page index %u", page));
                        hostReadRandom(npc, tempBuf, fullPageSize);
                        pageBuf = tempBuf;

                } else if (isPartial) {

                        ok = (page == (numPages - 1));
                        require_action(ok, clean_up, bug(npc, "bad page index %u", page));
                        hostMoveMem(npc, tempBuf, llbBuf + offset, copySize);
                        hostReadRandom(npc, tempBuf + copySize, fillSize);
                        pageBuf = tempBuf;

                } else {

                        ok = ((numPtabPages <= page) && (page < (numPages - numPartialPages)));
                        require_action(ok, clean_up, bug(npc, "bad page index %u", page));
                        pageBuf = llbBuf + offset;
                }

                ok = (NULL != pageBuf);
                require_action(ok, clean_up, bug(npc, "pageBuf unassigned"));

                for (ce = 0; ce < npc->provider->geometry.ceCount; ce++) {

                        if (isEmpty) {

                                // fill empty pages after LLB content by writing as a full page 
                                // iff writing to a PPN device; otherwise, for raw nand devices, 
                                // we leave empty pages unprogrammed, and the bounds defined
                                // by 'lastPage' above should prevent us from ever executing
                                // along this conditional path
                                ok = isPpn;
                                require_action(ok, clean_up, bug(npc, "empty boot pages should only be written for PPN devices"));
                                if (!hostWriteFullPage(npc, ce, addr, pageBuf, endorseMetaBuf(npc, npc->bootIdx))) {
                                        warn(npc, "failed empty boot page write at %u:%u", ce, addr);
                                }

                        } else if (!hostWriteBootPage(npc, ce, addr, pageBuf)) {

                                // note that a single page write error
                                // should not abort attempt to write
                                // boot partition
                                warn(npc, "failed boot page write at %u:%u", ce, addr);

                        } else {

                                // XXX - read back to confirm!
                                goodCopies++;
                        }
                }

                ok = (isEmpty || (0 < goodCopies));
                require_action(ok, clean_up, error(npc, "failed entire page stripe during LLB write"));

                // XXX - Based upon behavior of SecureROM; it only
                // matters if at least one CE has entire good copy of
                // LLB; therefore, this failure condition is actually
                // not sufficient.  Extend to correctly identify as
                // error the case where no complete copy LLB within
                // any CE was written correctly.

                offset += copySize;
                remaining -= copySize;
        }

        info(npc, "next %u pages of boot blocks written with LLB image of size %u bytes", 
             numLlbPages, llbSize);
        info(npc, "final %u pages of boot blocks %s", 
             (numAvailPages - numPages), 
             isPpn ? "written with random data in full page format" : "left unprogrammed");

clean_up:
        if (NULL != tempBuf) {
                hostFreeMem(npc, tempBuf, fullPageSize);
                tempBuf = NULL;
        }

        return(ok);
}


/* ========================================================================== */

#pragma mark - Unlocked External Interface Wrappers

static const NandPartEntry *
getEntryExternal(void * core, uint32_t partIndex)
{
        return getEntry((NandPartCore*)core, partIndex);
}

static uint32_t
getBankWidthExternal(void * core, uint32_t partIndex)
{
        return getBankWidth((NandPartCore*)core, partIndex);
}

static uint32_t
getBlockDepthExternal(void * core, uint32_t partIndex)
{
        return getBlockDepth((NandPartCore*)core, partIndex);
}

static uint32_t
getPagesPerBlockExternal(void * core, uint32_t partIndex)
{
        return getPagesPerBlock((NandPartCore*)core, partIndex);
}

static uint32_t
getBytesPerPageExternal(void * core, uint32_t partIndex)
{
        return getBytesPerPage((NandPartCore*)core, partIndex);
}


/* ========================================================================== */

#pragma mark - Locked External Interface Wrappers

static bool 
loadPartitionTableExternal(void * core)
{
        bool ok;
        funcLocked(&ok, loadPartitionTable, core);
        return(ok);
}

static bool 
eraseBootPartitionExternal(void * core)
{
        bool ok;
        funcLocked(&ok, eraseBootPartition, core);
        return(ok);
}

static bool 
writeBootPartitionExternal(void * core, void * llbBuf, unsigned llbSize)
{
        bool ok;
        funcLocked(&ok, writeBootPartition, core, (uint8_t *)llbBuf, llbSize);
        return(ok);     
}

static bool 
isBlockBadExternal(void * core, uint32_t partIndex, uint32_t partBank, uint32_t partBlock)
{
        bool ok;
        funcLocked(&ok, isBlockBad, core, partIndex, partBank, partBlock, true);
        return(ok);
}

static bool 
readPageExternal(void * core, uint32_t partIndex, uint32_t partBank, uint32_t partBlock, uint32_t pageInBlock, void * buf, bool * isClean)
{
        bool ok;
        funcLocked(&ok, readPage, core, partIndex, partBank, partBlock, pageInBlock, buf, isClean, true);
        return(ok);
}

static bool 
writePageExternal(void * core, uint32_t partIndex, uint32_t partBank, uint32_t partBlock, uint32_t pageInBlock, void * buf)
{
        bool ok;
        funcLocked(&ok, writePage, core, partIndex, partBank, partBlock, pageInBlock, buf, true);
        return(ok);
}

static bool 
eraseBlockExternal(void * core, uint32_t partIndex, uint32_t partBank, uint32_t partBlock)
{
        bool ok;
        funcLocked(&ok, eraseBlock, core, partIndex, partBank, partBlock, true);
        return(ok);
}

static bool 
requestPTabDiffExternal(void * core, uint32_t partIndex, void * buf, unsigned size)
{
        bool ok;
        funcLocked(&ok, requestPTabDiff, core, partIndex, buf, size);
        return(ok);
}

static bool 
providePTabDiffExternal(void * core, uint32_t partIndex, const void * buf, unsigned size)
{
        bool ok;
        funcLocked(&ok, providePTabDiff, core, partIndex, buf, size);
        return(ok);
}

static bool 
formatPTabExternal(void * core)
{
        bool ok;
        funcLocked(&ok, formatPTab, core);
        return(ok);
}

static unsigned
getPTabBytesExternal(void * core, void * buf, unsigned size)
{
        unsigned bytesRead;
        funcLocked(&bytesRead, getPTabBytes, core, buf, size);
        return(bytesRead);
}


/* ========================================================================== */
