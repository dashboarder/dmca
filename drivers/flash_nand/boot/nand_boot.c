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

#include <AssertMacros.h>
#include <debug.h>
#include <stdarg.h>
#include <sys.h>
#include <sys/menu.h>
#include <drivers/nand_boot.h>

#include <platform/clocks.h>
#include <platform/soc/hwclocks.h>

#include <lib/profile.h>
#include <lib/random.h>

#include "nand_part.h"
#include "nand_part_interface.h"
#include "nand_export.h"

#if WITH_NAND_FIRMWARE
# include "firmware/nand_firmware.h"
#endif

#if WITH_NAND_NVRAM
# include "nvram/nand_nvram_platform.h"
# include "nvram/nand_nvram_impl.h"
#endif

#if WITH_NAND_SYSCFG
# include <drivers/nand_syscfg.h>
#endif

#if WITH_EFFACEABLE_NAND
# include <lib/effaceable_nand.h>
#endif

/* ========================================================================== */

#if WITH_NAND_NVRAM
extern bool flash_nand_nvram_init(NandPartInterface *npi, uint32_t part, bool isNewborn);
#endif

// =============================================================================

static const char * kLogOrigin = "nand_boot";

#define _logLine(fac, fmt, args...) _logf_impl(kNandPartLogLevel##fac, "[%s:%s@%4d] " fmt "\n", kLogOrigin, kNandPartLogTag##fac, __LINE__, ##args)

#define error(args...) _logLine(Error, ##args)
#define info(args...)  _logLine(Info,  ##args)

#if NAND_PART_EXTENDED_LOGGING
# define warn(args...)  _logLine(Warn,  ##args)
#else
# define warn(args...)  /* do nothing */
#endif

#if NAND_PART_EXTENDED_LOGGING
# define debug(args...) _logLine(Debug, ##args)
#else
# define debug(args...) /* do nothing */
#endif

#if NAND_PART_LOCAL_DEBUG_ONLY
# define spew(args...)  _logLine(Spew,  ##args)
#else
# define spew(args...)  /* do nothing */
#endif

/* ========================================================================== */

#if NAND_PART_LOCAL_DEBUG_ONLY
static void
_conditionally_force_enable_uarts(void)
{
        static bool isEnabled = false;
        if (!isEnabled) {
                debug_enable_uarts(3);
                isEnabled = true;
        }
}
#else
# define _conditionally_force_enable_uarts() /* do nothing */
#endif

/* ========================================================================== */

static nand_boot_context_t _context;

// =============================================================================

#if DEBUG_BUILD
# define USING_STRONG_SEED false
#else
# define USING_STRONG_SEED true
#endif

// =============================================================================

#if DEBUG_BUILD
# define USING_STRONG_SEED false
#else
# define USING_STRONG_SEED true
#endif

/* ========================================================================== */

// XXX - eliminate these references

#if WITH_NAND_FILESYSTEM
extern uint32_t nand_fsys_block_offset;
#elif WITH_NAND_SYSCFG
uint32_t nand_fsys_block_offset;
#endif

/* ========================================================================== */

static bool _create_partition_device(void * context, NandPartEntry * entry, uint32_t idx);
static bool _publish_partition_device(void * context, uint32_t idx);

static void * _alloc_mem(void * context, unsigned size, bool align);
static int _compare_mem(void * context, const void * left, const void * right, unsigned int size);
static void * _move_mem(void * context, void * dest, const void * src, unsigned size);
static void * _set_mem(void * context, void * mem, uint8_t val, unsigned size);
static void _free_mem(void * context, void * mem, unsigned size);

static void _lock_interface(void * context);
static void _unlock_interface(void * context);

static void _read_random(void * context, void * buf, unsigned size);

static bool _set_data_property(void * context, const char * key, void * buf, unsigned size);
static bool _set_number_property(void * context, const char * key, uint64_t num, unsigned bits);
static bool _set_string_property(void * context, const char * key, const char * cstring);
static bool _set_boolean_property(void * context, const char * key, bool val);

static bool _wait_for_fbbt_service(void * context);

static void _vlogf_message(void * context, NandPartLogLevel lvl, const char * fmt, va_list ap);
static void _vlogf_impl(NandPartLogLevel lvl, const char * fmt, va_list ap);
static void _logf_impl(NandPartLogLevel lvl, const char * fmt, ...);

static void init_nand_part_callbacks(NandPartProvider *npp);
static void init_nand_part_params(NandPartProvider *npp);
static bool init_nand_boot_context(nand_boot_context_t * cxt);

/* ========================================================================== */

int
nand_boot_init(void)
{
        bool ok = true;
        static nand_boot_context_t * cxt = NULL;

        PROFILE_ENTER('NBI');

        // If we've already been initialized for some reason, short
        // circuit and finish.
        require(NULL == cxt, finish);
        cxt = &_context;

        info("nand_boot_init() starting");

        ok = init_nand_boot_context(cxt);
        require(ok, finally);

        spew("initializing nand_part subsystem");
        ok = nand_part_init(&cxt->npi, &cxt->npp);
        require(ok, finally);

        spew("scanning for nand boot partition tables");
        ok = npi_load_ptab(cxt->npi);
        require(ok, finally);

#if WITH_NAND_SYSCFG
        // Defer syscfg init until after file system has finished initializing.
        //
        // XXX - <rdar://problem/9192124> Open FTL before first syscfg request
        if (cxt->syscfg_needs_init && cxt->found_fsys_offset) {
                if (0 != nand_syscfg_init(cxt->npi)) {
                        spew("unable to init syscfg");
                        ok = false;
                }
        }
#endif

finally:

#if WITH_NAND_NVRAM
        // If nvram feature is needed but unable to find valid nand
        // nvram partition entry for any reason, create a mock
        // ram-based nvram device so that the system won't hang when
        // attempting to read the environment settings
        // (<rdar://problem/6675471>).
        if (!cxt->have_nvram_part) {
                if (!flash_nand_nvram_init(cxt->npi, kNandPartEntryMaxCount, true)) {
                        spew("unable to create temporary nvram device");
                        ok = false;
                }
        }
#endif

        info("nand_boot_init() %s", ok ? "succeeded" : "failed");

finish:
        PROFILE_EXIT('NBI');
        return(ok ? 0 : -1);
}

/* ========================================================================== */

static bool 
_create_partition_device(void * context, NandPartEntry * entry, uint32_t idx)
{
        bool ok = true;
        nand_boot_context_t * cxt = withContext(context);
        
        if (kNandPartEntryMaxCount != idx) {
                switch (entry->content) {
                case kNandPartContentSyscfg:
                        cxt->syscfg_needs_init = true;
                        break;
                case kNandPartContentNVRAM:
                        cxt->have_nvram_part = true;
                        break;
                default:
                        // nothing to do for most cases
                        break;
                }
        }
         
        return ok;
}

static bool 
_publish_partition_device(void * context, uint32_t idx)
{
        bool ok = true;
        nand_boot_context_t * cxt = withContext(context);
        const NandPartEntry * entry = NULL;

        if (kNandPartEntryMaxCount == idx) {
                debug("nand has not yet been formatted\n");
                return ok;
        }

        entry = npi_get_entry(cxt->npi, idx);
        switch (entry->content) {
        case kNandPartContentBootBlock:
#if ALLOW_NAND_FIRMWARE_ERASE
                // The only reason to publish boot block device is to allow 
                // the erasure of LLB in triage/development use cases.
                if (!nand_firmware_init(cxt->npi, idx)) {
                        spew("unable to create nand LLB device\n");
                        ok = false;
                }
#endif
                break;
        case kNandPartContentNVRAM:
#if WITH_NAND_NVRAM
                if (!flash_nand_nvram_init(cxt->npi, idx, false)) {
                        spew("unable to create nand nvram device\n");              
                        ok = false;
                }
#endif
                break;
        case kNandPartContentFirmware:
#if WITH_NAND_FIRMWARE
                if (!nand_firmware_init(cxt->npi, idx)) {
                        spew("unable to create nand firmware device\n");
                        ok = false;
                }
#endif
                break;
        case kNandPartContentFilesystem:
                cxt->found_fsys_offset = true;
                cxt->fsys_block_offset = entry->slice.offset;
#if WITH_NAND_SYSCFG
                nand_fsys_block_offset = entry->slice.offset;
#elif WITH_NAND_FILESYS
                // XXX never reachable
                nand_fsys_block_offset = entry->slice.offset;
#endif
                break;
        case kNandPartContentEffaceable:
#if WITH_EFFACEABLE_NAND
                effaceable_nand_init(cxt->npi, idx);
#endif
                break;
        default:
                // nothing to do...
                ok = true;
        }
         
        return ok;
}

/* ========================================================================== */

static void * 
_alloc_mem(void * context, unsigned size, bool align)
{
        void * buf = NULL;

        if (align) {
                buf = memalign(size, CPU_CACHELINE_SIZE);
        } else {
                buf = malloc(size);
        }

        return(buf);
}

static int   
_compare_mem(void * context, const void * left, const void * right, unsigned int size)
{
        return(memcmp(left, right, size));
}

static void * 
_move_mem(void * context, void * dest, const void * src, unsigned size)
{
        return(memmove(dest, src, size));
}

static void *    
_set_mem(void * context, void * mem, uint8_t val, unsigned size)
{
        return(memset(mem, val, size));
}

static void
_free_mem(void * context, void * mem, unsigned size)
{
        free(mem);
}

static void     
_lock_interface(void * context)
{
        // XXX - Given its cooperative multitasking nature, does iBoot
        // even need to bother with locking the nand_part_core
        // interface?
}

static void     
_unlock_interface(void * context)
{
        // XXX - Given its cooperative multitasking nature, does iBoot
        // even need to bother with locking the nand_part_core
        // interface?
}

static void
_read_random(void * context, void * buf, unsigned size)
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

static bool 
_set_data_property(void * context, const char * key, void * buf, unsigned size)
{
        spew("%s=[%02x %02x %02x %02x ...]", 
             key, ((uint8_t*)buf)[0], ((uint8_t*)buf)[1], ((uint8_t*)buf)[2], ((uint8_t*)buf)[3]);
        return true;
}

static bool
_set_number_property(void * context, const char * key, uint64_t num, unsigned bits)
{
        spew("%s=%llu", key, num);
        return true;
}

static bool 
_set_string_property(void * context, const char * key, const char * cstring)
{
        spew("%s=%s", key, cstring);
        return true;
}

static bool 
_set_boolean_property(void * context, const char * key, bool val)
{
        spew("%s=%s", key, val ? "true" : "false");
        return true;
}

static bool 
_wait_for_fbbt_service(void * context)
{
        // This callback is not relevant in iBoot context.
        return true;
}

static void
_vlogf_message(void * context, NandPartLogLevel lvl, const char * fmt, va_list ap)
{
        _vlogf_impl(lvl, fmt, ap);
}

static void
_vlogf_impl(NandPartLogLevel lvl, const char * fmt, va_list ap)
{
#if DEBUG_BUILD
        const NandPartLogLevel max_level = kNandPartLogLevelDebug;
#elif DEVELOPMENT_BUILD
        const NandPartLogLevel max_level = kNandPartLogLevelInfo;
#else
        const NandPartLogLevel max_level = kNandPartLogLevelError;
#endif

        // XXX - Implement circular log buffer to help triage failures
        // before debug uart is turned on in development and debug
        // builds.

        if (max_level >= lvl) {
                if ((kNandPartLogLevelError == lvl) || (kNandPartLogLevelBug == lvl)) {
                        _conditionally_force_enable_uarts();
                }
                vprintf(fmt, ap);
        }
}

static void
_logf_impl(NandPartLogLevel lvl, const char * fmt, ...)
{
        va_list ap;

        va_start(ap, fmt);
        _vlogf_impl(lvl, fmt, ap);
        va_end(ap);
}

/* ========================================================================== */

static void
init_nand_part_callbacks(NandPartProvider *npp)
{
        npp->lock_interface           = _lock_interface;
        npp->unlock_interface         = _unlock_interface;
        npp->alloc_mem                = _alloc_mem;
        npp->compare_mem              = _compare_mem;
        npp->move_mem                 = _move_mem;
        npp->set_mem                  = _set_mem;
        npp->free_mem                 = _free_mem;
        npp->read_random              = _read_random;
        npp->set_data_property        = _set_data_property;
        npp->set_number_property      = _set_number_property;
        npp->set_string_property      = _set_string_property;
        npp->set_boolean_property     = _set_boolean_property;
        npp->wait_for_fbbt_service    = _wait_for_fbbt_service;
        npp->create_partition_device  = _create_partition_device;
        npp->publish_partition_device = _publish_partition_device;
        npp->vlogf_message            = _vlogf_message;

        // The following callbacks are initialized by nand_export_init():
        //
        //     npp->read_boot_page      
        //     npp->read_full_page      
        //     npp->write_boot_page     
        //     npp->write_full_page     
        //     npp->erase_block         
        //     npp->is_block_factory_bad
        //     npp->validate_spare_list 
        //     npp->request_spare_blocks
}

static void
init_nand_part_params(NandPartProvider *npp)
{
        // The only destructive operation allowed on boot block is
        // erase of boot partition (i.e. LLB portion but not partition
        // table copies), available only in debug builds.
#if ALLOW_NAND_FIRMWARE_ERASE
        npp->params.isBootPartErasable = true;
#else
        npp->params.isBootPartErasable = false;
#endif
        npp->params.isBootPartWritable = false;
        npp->params.isFormatEnabled    = false;

        // Enable debug-only features only in debug builds of iBoot.
#if DEBUG_BUILD
        npp->params.isDebugEnabled = true;
#else
        npp->params.isDebugEnabled = false;
#endif

        // In iBoot, only enable tracing for local developer builds.
        npp->params.isHostTraceEnabled   = false;
        npp->params.isClientTraceEnabled = false;
}

/* ========================================================================== */

static bool
init_nand_boot_context(nand_boot_context_t * cxt)
{
        memset(cxt, 0, sizeof(nand_boot_context_t));
        cxt->npp.context = cxt;

        init_nand_part_params(&cxt->npp);
        init_nand_part_callbacks(&cxt->npp);

        return nand_export_init(cxt);
}

/* ========================================================================== */

static int do_nand_failval_set(int argc, struct cmd_arg *args)
{           
        nand_failval_set(args[1].n, args[2].n);
        return 0;
}

MENU_COMMAND_DEBUG(nand_failval_set, do_nand_failval_set, "Set the number of operations before failure is injected", NULL);

// =============================================================================

static int do_nand_part_dump(int argc, struct cmd_arg *args)
{
        nand_part_dump(_context.npi);
        return 0;
}

MENU_COMMAND_DEBUG(nand_part_dump, do_nand_part_dump, "dump nand partition table info", NULL);

// =============================================================================

