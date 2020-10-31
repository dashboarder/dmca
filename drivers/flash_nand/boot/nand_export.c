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

#include <stdarg.h>

#include "nand_part.h"
#include "nand_part_interface.h"
#include "nand_export.h"

#include <FIL.h>
#include <debug.h>
#include <AssertMacros.h>

#include "ANDTypes.h"
#include "WMRFeatures.h"

// =============================================================================

#define _log(fac, line, fmt, args...) dprintf(DEBUG_CRITICAL, "[nand_export.c : %s @ %4d] " fmt "\n", kNandPartLogTag##fac, line, ##args)

#define error(args...) _log(Error, __LINE__, ##args)
#define info(args...)  _log(Info,  __LINE__, ##args)

#if NAND_PART_EXTENDED_LOGGING
# define warn(args...)  _log(Warn,  __LINE__, ##args)
# define debug(args...) _log(Debug, __LINE__, ##args)
#else
# define warn(args...)  /* do nothing */
# define debug(args...) /* do nothing */
#endif

#if NAND_PART_LOCAL_DEBUG_ONLY
# define spew(args...)  _log(Spew,  __LINE__, ##args)
#else
# define spew(args...)  /* do nothing */
#endif

/* ========================================================================== */

static bool _read_boot_page(void * context, uint32_t ce, uint32_t addr, void * buf, bool * is_clean);
static bool _read_full_page(void * context, uint32_t ce, uint32_t addr, void * buf, void * meta, bool * is_clean);
static bool _write_boot_page(void * context, uint32_t ce, uint32_t addr, void * buf);
static bool _write_full_page(void * context, uint32_t ce, uint32_t addr, void * buf, void * meta);
static bool _erase_block(void * context, uint32_t ce, uint32_t addr, uint32_t block);

static bool _is_block_factory_bad(void * context, uint32_t ce, uint32_t block);
static bool _validate_spare_list(void * context, uint32_t * ce_list, uint32_t * block_list, unsigned count);
static bool _request_spare_blocks(void * context, uint32_t * ce_list, uint32_t * block_list, unsigned count, unsigned * quantity);

static void _init_nand_export_geometry(NandPartProvider *npp);
static void _init_nand_export_callbacks(NandPartProvider *npp);

static int failVal[] = {0, 0, 0};
static int numOps[] = {0, 0, 0};

/* ========================================================================== */

static bool
_read_boot_page(void * context, uint32_t ce, uint32_t addr, void * buf, bool * is_clean)
{
        Int32 ret;
        nand_boot_context_t * cxt = withContext(context);

        require(cxt->is_fil_available, no_fil);
        require(0 != FIL_GetFuncTbl()->ReadBLPage, no_op);
        ret = FIL_GetFuncTbl()->ReadBLPage(ce, addr, buf);
        if (NULL != is_clean) {
                *is_clean = (ANDErrorCodeCleanOk == ret);
        }
        
        numOps[READ_OP]++;
        if (failVal[READ_OP] > 0 && numOps[READ_OP] >= failVal[READ_OP]) {
                debug("*** injecting error ***\n");
                numOps[READ_OP] = 0;
                return false;
        }
                
        return((ANDErrorCodeOk == ret) ? true : false);

no_op:
#if RELEASE_BUILD
        error("no FIL ReadBLPage");
#else
        panic("no FIL ReadBLPage");
#endif

no_fil:
        return false;
}

static bool  
_read_full_page(void * context, uint32_t ce, uint32_t addr, void * buf, void * meta, bool * is_clean)
{
        Int32 ret;
        nand_boot_context_t * cxt = withContext(context);

        require(cxt->is_fil_available, no_fil);
        require(0 != FIL_GetFuncTbl()->ReadWithECC, no_op);
        ret = FIL_GetFuncTbl()->ReadWithECC(ce, addr, buf, meta, NULL, NULL, false);
        if (NULL != is_clean) {
                *is_clean = (ANDErrorCodeCleanOk == ret);
        }
        
        numOps[READ_OP]++;
        if (failVal[READ_OP] > 0 && numOps[READ_OP] >= failVal[READ_OP]) {
                debug("*** injecting error ***\n");
                numOps[READ_OP] = 0;
                return false;
        }
        return((ANDErrorCodeOk == ret) ? true : false);

no_op:
#if RELEASE_BUILD
        error("no FIL ReadWithECC");
#else
        panic("no FIL ReadWithECC");
#endif

no_fil:
        return false;
}

static bool     
_write_boot_page(void * context, uint32_t ce, uint32_t addr, void * buf)
{
        Int32 ret;
        nand_boot_context_t * cxt = withContext(context);

        require(cxt->is_fil_available, no_fil);
        require(0 != FIL_GetFuncTbl()->WriteBLPage, no_op);
        ret = FIL_GetFuncTbl()->WriteBLPage(ce, addr, buf);
        
        numOps[WRITE_OP]++;
        if (failVal[WRITE_OP] > 0 && numOps[WRITE_OP] >= failVal[WRITE_OP]) {
                debug("*** injecting error ***\n");
                numOps[WRITE_OP] = 0;
                return false;
        }

        return((ANDErrorCodeOk == ret) ? true : false);

no_op:
#if RELEASE_BUILD
        error("no FIL WriteBLPage");
#else
        panic("no FIL WriteBLPage");
#endif

no_fil:
        return false;
}

static bool     
_write_full_page(void * context, uint32_t ce, uint32_t addr, void * buf, void * meta)
{
        Int32 ret;
        nand_boot_context_t * cxt = withContext(context);

        require(cxt->is_fil_available, no_fil);
        require(0 != FIL_GetFuncTbl()->Write, no_op);
        ret = FIL_GetFuncTbl()->Write(ce, addr, buf, meta, false);

        numOps[WRITE_OP]++;
        if (failVal[WRITE_OP] > 0 && numOps[WRITE_OP] >= failVal[WRITE_OP]) {
                debug("*** injecting error ***\n");
                numOps[WRITE_OP] = 0;
                return false;
        }
        
        return((ANDErrorCodeOk == ret) ? true : false);

no_op:
#if RELEASE_BUILD
        error("no FIL Write");
#else
        panic("no FIL Write");
#endif

no_fil:
        return false;
}

static bool     
_erase_block(void * context, uint32_t ce, uint32_t addr, uint32_t block)
{
        Int32 ret;
        nand_boot_context_t * cxt = withContext(context);

        require(cxt->is_fil_available, no_fil);
        require(0 != FIL_GetFuncTbl()->Erase, no_op);

        // In iBoot, the FIL erase command expects an block offset;
        // however, other environments may expect an address instead.
        ret = FIL_GetFuncTbl()->Erase(ce, block);

        numOps[ERASE_OP]++;
        if (failVal[ERASE_OP] > 0 && numOps[ERASE_OP] >= failVal[ERASE_OP]) {
                debug("*** injecting error ***\n");
                numOps[ERASE_OP] = 0;
                return false;
        }
        
        return((ANDErrorCodeOk == ret) ? true : false);

no_op:
#if RELEASE_BUILD
        error("no FIL Erase");
#else
        panic("no FIL Erase");
#endif

no_fil:
        return false;
}

/* ========================================================================== */

static bool     
_is_block_factory_bad(void * context, uint32_t ce, uint32_t block)
{
        // XXX - Access to factory bad block table not yet supported
        // in iBoot context.  This probably should be fixed from an
        // academic perspective; however, by the time we've finished
        // formatting during first restore in iOS, the problem of
        // factory bad blocks should be moot for nand_part_core.
        return false;
}

static bool 
_validate_spare_list(void * context, uint32_t * ce_list, uint32_t * block_list, unsigned count)
{
        // XXX - Double-checking spare block list against what VFL
        // believes it should be is not yet supported in iBoot; therefore,
        // for the moment, simply lie when asked and say that all is well.
        return true;
}

static bool 
_request_spare_blocks(void * context, uint32_t * ce_list, uint32_t * block_list, unsigned count, unsigned * quantity)
{
        // Requesting spare blocks should only happen as a result of
        // initial format (in iOS); therefore, it's a bug if this
        // callback is ever in invoked in iBoot.
#if RELEASE_BUILD
        error("unexpected callback requesting spare blocks\n");
#else
        panic("unexpected callback requesting spare blocks\n");
#endif
        return false;
}

/* ========================================================================== */

static void
_init_nand_export_geometry(NandPartProvider *npp)
{
        npp->geometry.isPpn                = FIL_GetFuncTbl()->GetDeviceInfo(AND_DEVINFO_PPN_DEVICE);
        
        npp->geometry.ceCount              = FIL_GetFuncTbl()->GetDeviceInfo(AND_DEVINFO_NUM_OF_CS);
        
        /* The value of AND_DEVINFO_BLOCKS_PER_CS should NOT be used for PPN */
        npp->geometry.blocksPerCE          = FIL_GetFuncTbl()->GetDeviceInfo(AND_DEVINFO_BLOCKS_PER_CS);
        npp->geometry.blockStride          = FIL_GetFuncTbl()->GetDeviceInfo(AND_DEVINFO_BLOCK_STRIDE);

        npp->geometry.pagesPerMLCBlock     = FIL_GetFuncTbl()->GetDeviceInfo(AND_DEVINFO_MLC_PAGES_PER_BLOCK);
        npp->geometry.pagesPerSLCBlock     = FIL_GetFuncTbl()->GetDeviceInfo(AND_DEVINFO_SLC_PAGES_PER_BLOCK);

        npp->geometry.bytesPerBootPage     = FIL_GetFuncTbl()->GetDeviceInfo(AND_DEVINFO_BYTES_PER_BL_PAGE);
        npp->geometry.bytesPerFullPage     = FIL_GetFuncTbl()->GetDeviceInfo(AND_DEVINFO_BYTES_PER_PAGE);
        npp->geometry.bytesPerFullPageMeta = FIL_GetFuncTbl()->GetDeviceInfo(AND_DEVINFO_FIL_META_BUFFER_BYTES) * FIL_GetFuncTbl()->GetDeviceInfo(AND_DEVINFO_FIL_LBAS_PER_PAGE);
                
        npp->geometry.bitsPerCauAddr       = FIL_GetFuncTbl()->GetDeviceInfo(AND_DEVINFO_BITS_PER_CAU_ADDRESS);
        npp->geometry.bitsPerPageAddr      = FIL_GetFuncTbl()->GetDeviceInfo(AND_DEVINFO_BITS_PER_PAGE_ADDRESS);
        npp->geometry.bitsPerBlockAddr     = FIL_GetFuncTbl()->GetDeviceInfo(AND_DEVINFO_BITS_PER_BLOCK_ADDRESS);
}

static void
_init_nand_export_callbacks(NandPartProvider *npp)
{
        npp->read_boot_page           = _read_boot_page;
        npp->read_full_page           = _read_full_page;
        npp->write_boot_page          = _write_boot_page;
        npp->write_full_page          = _write_full_page;
        npp->erase_block              = _erase_block;
        npp->is_block_factory_bad     = _is_block_factory_bad;
        npp->validate_spare_list      = _validate_spare_list;
        npp->request_spare_blocks     = _request_spare_blocks;
}

/* ========================================================================== */

bool
nand_export_init(nand_boot_context_t * cxt)
{
        bool ok = false;
        Int32 ret = 0;
        NandPartProvider *npp = &(cxt->npp);

        ret = FIL_Init();

        // Always export callbacks.  Even if FIL failed to init, they
        // will be protected internally by 'cxt->is_fil_available'.
        _init_nand_export_callbacks(npp);

        if (ANDErrorCodeOk == ret) {
                _init_nand_export_geometry(npp);
                ok = true;
        } else {
                dprintf(DEBUG_SPEW, "unable to init nand FIL\n");
                ok = false;
        }

        cxt->is_fil_available = ok;
        return ok;
}

/* ========================================================================== */


extern void
nand_failval_set(int op_num, int fail_val)
{
        int i;
        failVal[op_num] = fail_val;
        
        /* print out the failure values set for each operation
         * where i = 0 = READ, i = 1 = WRITE, and i = 2 = ERASE
         */
        for (i = 0; i < 3; i++)
                debug("%d %d\n", i, failVal[i]);
}


