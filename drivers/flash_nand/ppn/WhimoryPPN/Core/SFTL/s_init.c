/*
 * Copyright (c) 2008-2010 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */

// Author:  Daniel J. Post (djp), dpost@apple.com

#include "s_internal.h"
#include "s_read.h"
#include "s_write.h"
#include "s_trim.h"
#include "s_sb.h"
#include "s_btoc.h"
#include "s_gc.h"
#include "s_dbg.h"
#include "s_geom.h"
#include "VFL.h"
#include "s_readdist.h"
#include "s_cxt.h"


static UInt32 do_quantize(UInt32 target)
{
    UInt32 out;
    UInt32 quant;

    // Multiply by superblock size
    out = target * s_g_vbas_per_sb;

    // Quantize
    quant = (1 << WMR_LOG2(out)) / S_RESERVE_QUANTIZE;

    // Adjust by a quarter to move quantization window
    out += (quant / 4);

    // Quantize
    out /= quant;
    out *= quant;

    // Scale back
    out /= s_g_vbas_per_sb;

    return out;
}

UInt32 s_calc_lbas(BOOL32 ideal)
{
    UInt32 quantize_sb;
    UInt32 target_sb;
    UInt32 quantize_vfl;
    UInt32 num_4gb;
    UInt64 reported_bytes;
    UInt32 numLba;

    if (ideal) {
        // Ideal capacity is based on VFL superblocks
        target_sb = s_g_vfl_max_sb;
    } else {
        // Otherwise, consider burnin partition size
        target_sb = s_g_max_sb;
    }

    quantize_sb = do_quantize(target_sb);
    quantize_vfl = do_quantize(s_g_vfl_max_sb);

    // Significantly smaller than full-sized (i.e. burnin partitioned)?
    if (!ideal && ((s_g_vfl_max_sb - s_g_max_sb) >= ((quantize_vfl * S_RESERVE_PERMIL_APPROX) / 1000))) {
        // Then ignore quantization
        numLba = (UInt32)(s_g_vbas_per_sb * target_sb);
        numLba -= (UInt32)((quantize_vfl * s_g_vbas_per_sb * S_RESERVE_PERMIL_APPROX) / 1000);
    } else {
        // Calculate LBAs, based on power-of-ten ratio
        num_4gb = (quantize_sb * s_g_vbas_per_sb * (s_g_bytes_per_lba / 1024)) / (4 * 1024 * 1024);
        reported_bytes = 4000000000ULL * num_4gb;
        numLba = (UInt32)((reported_bytes + s_g_bytes_per_lba - 1) / s_g_bytes_per_lba);
    }

    return numLba;
}

void s_freeMemory(void)
{
    WMR_BufZone_Free(&sftl.BufZone);
    s_dbg_close();
    s_gc_close();
#ifndef AND_READONLY
    s_trim_close();
    s_write_close();
    s_readDist_close();
#endif // AND_READONLY
    s_sb_close();
    s_btoc_close();
    L2V_Free();
    WMR_MEMSET((void*)&sftl, 0, sizeof(sftl));
}

Int32 sftl_init(VFLFunctions *vflfuncs)
{
    UInt32 thisSize;
#ifndef AND_READONLY
    UInt32 i, bufMax;
#endif // AND_READONLY

    // Do not allow multiple init() calls because number of user blocks can change
    WMR_ASSERT(!sftl_init_done);

    // Format checks:
    // s_btoc_entry_t
    WMR_ASSERT(WMR_OFFSETOF(s_btoc_entry_t, weaveSeqAdd) == 0);
    WMR_ASSERT(WMR_OFFSETOF(s_btoc_entry_t, userWeaveSeq) == 4);
    WMR_ASSERT(WMR_OFFSETOF(s_btoc_entry_t, lba) == 8);
    WMR_ASSERT(WMR_OFFSETOF(s_btoc_entry_t, count) == 12);
    // s_sb_fmt_t
    WMR_ASSERT(WMR_OFFSETOF(s_sb_fmt_t, type) == 0);
    WMR_ASSERT(WMR_OFFSETOF(s_sb_fmt_t, num_btoc_vbas_AND_staticFlag) == 1);
    WMR_ASSERT(WMR_OFFSETOF(s_sb_fmt_t, validLbas) == 2);
    WMR_ASSERT(WMR_OFFSETOF(s_sb_fmt_t, __was_reads) == 4);
    WMR_ASSERT(WMR_OFFSETOF(s_sb_fmt_t, erases) == 6);
    // s_cxt_contig_t
    WMR_ASSERT(WMR_OFFSETOF(s_cxt_contig_t, vba) == 0);
    WMR_ASSERT(WMR_OFFSETOF(s_cxt_contig_t, span) == 4);

    // Pre-clear sftl
    WMR_MEMSET((void*)&sftl, 0, sizeof(sftl));
    
    sftl.vfl = vflfuncs;

    s_debug(INIT, "S_INIT[start]");

    s_geom_init();
#ifndef AND_READONLY
    if (!s_trim_init()) {
        goto errorPath;
    }
    s_cxt_init();
    s_readDist_init();
#endif

    // Buffer allocations--all DMA-able buffers must be allocated here
    WMR_BufZone_Init(&sftl.BufZone);
#ifndef AND_READONLY
    bufMax = sftl.vfl->GetDeviceInfo(AND_DEVINFO_STREAM_BUFFER_MAX);
    for (i = 0; i < S_SBSTREAM_MAX; i++) {
        sftl.write.stream[i].bufSize = S_SBSTREAM_BUFSIZE(i, bufMax);
        sftl.write.stream[i].bufPage = (UInt8 *)WMR_Buf_Alloc_ForDMA(&sftl.BufZone, s_g_bytes_per_lba * sftl.write.stream[i].bufSize);
        sftl.write.stream[i].bufMeta = (PageMeta_t *)WMR_Buf_Alloc_ForDMA(&sftl.BufZone, sizeof(PageMeta_t) * sftl.write.stream[i].bufSize);
        sftl.write.stream[i].bufThreshold = S_WRITE_LBAS_THRESHOLD(&sftl.write.stream[i]);
    }
    thisSize = sizeof(sftl.write.meta_size);
    sftl.vfl->GetStruct(AND_STRUCT_VFL_MAX_TRANS_IN_PAGES, &sftl.write.meta_size, &thisSize);
    // make sure it's a multiplication of a pefrect stripes that does not exceed AND_STRUCT_VFL_MAX_TRANS_IN_PAGES
    sftl.write.meta_size = ROUNDDOWNTO(sftl.write.meta_size, s_g_num_banks);
    if(sftl.write.meta_size == 0)
    {
        sftl.write.meta_size = s_g_num_banks;
    }
    sftl.write.meta_size *= (s_g_vbas_per_page);
    sftl.write.meta = (PageMeta_t *)WMR_Buf_Alloc_ForDMA(&sftl.BufZone, sftl.write.meta_size * sizeof(PageMeta_t));
#endif // AND_READONLY
    sftl.read.meta = (PageMeta_t *)WMR_Buf_Alloc_ForDMA(&sftl.BufZone, s_g_vbas_per_sb * sizeof(PageMeta_t));
    sftl.boot.meta = (PageMeta_BTOC_t *)WMR_Buf_Alloc_ForDMA(&sftl.BufZone, sizeof(PageMeta_t));
    sftl.scan_meta = (PageMeta_t *)WMR_Buf_Alloc_ForDMA(&sftl.BufZone, S_SCAN_META_SIZE * sizeof(PageMeta_t));
    sftl.tmpBuf = (UInt8 *)WMR_Buf_Alloc_ForDMA(&sftl.BufZone, s_g_bytes_per_page);

    if (!WMR_BufZone_FinishedAllocs(&sftl.BufZone)) {
        goto errorPath;
    }

#ifndef AND_READONLY
    for (i = 0; i < S_SBSTREAM_MAX; i++) {
        WMR_BufZone_Rebase(&sftl.BufZone, (void**)&sftl.write.stream[i].bufPage);
        WMR_BufZone_Rebase(&sftl.BufZone, (void**)&sftl.write.stream[i].bufMeta);
    }
    WMR_BufZone_Rebase(&sftl.BufZone, (void**)&sftl.write.meta);
#endif // AND_READONLY
    WMR_BufZone_Rebase(&sftl.BufZone, (void**)&sftl.read.meta);
    WMR_BufZone_Rebase(&sftl.BufZone, (void**)&sftl.boot.meta);
    WMR_BufZone_Rebase(&sftl.BufZone, (void**)&sftl.scan_meta);
    WMR_BufZone_Rebase(&sftl.BufZone, (void**)&sftl.tmpBuf);
    WMR_BufZone_FinishedRebases(&sftl.BufZone);

    thisSize = sizeof(sftl.reported_lba);
    sftl.vfl->GetStruct(AND_STRUCT_VFL_EXPORTED_LBA_NO, &sftl.reported_lba, &thisSize);
    if (sftl.reported_lba == 0xffffffff) 
    {
        sftl.reported_lba = s_calc_lbas(FALSE32);
        sftl.vfl->SetStruct(AND_STRUCT_VFL_EXPORTED_LBA_NO, &sftl.reported_lba, thisSize);
    }
    
    sftl.max_lba = sftl.reported_lba + S_LBAS_INTERNAL; // Reserve some LBAs for internal use

    L2V_Search_Init(&sftl.read.c);

    if (!s_btoc_init())
    {
        goto errorPath;
    }

    if (!s_gc_init())
    {
        goto errorPath;
    }

    if (!s_sb_init())
    {
        goto errorPath;
    }

#ifndef AND_READONLY
    if (!s_write_init())
    {
        goto errorPath;
    }
#endif // AND_READONLY

    if (!s_dbg_init())
    {
        goto errorPath;
    }

    // Allocate the node pool
    thisSize = sizeof(L2V_nodepool_mem);
    sftl.vfl->GetStruct(AND_STRUCT_VFL_EXPORTED_L2V_POOL, &L2V_nodepool_mem, &thisSize);
    if (!L2V_Init(sftl.max_lba, s_g_max_sb, s_g_vbas_per_sb)) {
        return ANDErrorCodeHwErr;
    }

    // Success path
    sftl_init_done = TRUE32;
    
    return ANDErrorCodeOk;

errorPath:
    s_freeMemory();
    return ANDErrorCodeHwErr;
}

void sftl_close(void)
{
    s_freeMemory();
    sftl_init_done = FALSE32;
}

