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

#ifndef __S_INTERNAL_H__
#define __S_INTERNAL_H__

#include "WMROAM.h"
#include "WMRBufTypes.h"
#include "WMRBuf.h"
#include "s_meta.h"
#include "L2V/L2V_Extern.h"
#include "s_defines.h"
#include "VFL.h"
#include "FTL.h"

typedef struct
{
    UInt32 vbas_per_sb, vbas_per_sb_shift;
    UInt32 pages_per_sb, pages_per_sb_shift;
    UInt32 max_sb, vfl_max_sb;
    UInt32 vbas_per_page, vbas_per_page_shift;
    UInt32 bytes_per_page, bytes_per_page_shift;
    UInt32 bytes_per_lba, bytes_per_lba_shift;
    UInt32 bytes_per_lba_meta, bytes_per_lba_meta_shift;
    UInt32 num_banks, num_banks_shift;
    UInt32 vbas_per_stripe, vbas_per_stripe_shift;
    UInt32 max_pages_per_btoc;
} sftl_geom_t;

typedef struct {
    UInt16 wPagesPerVb;
    UInt16 wUserVbTotal;
    UInt32 dwUserPagesTotal;
    UInt16 wBytesPerPage;
    UInt16 wNumOfBanks;
    UInt16 wBytesPerPageMeta;
} sftl_devinfo_t;

typedef struct
{
    UInt32 sb[S_GCFIFO_DEPTH + 1];
    UInt32 head, tail;  // head==tail means it is empty
} GC_Fifo_t;

typedef enum
    {
        GCD_IDLE,
        GCD_EVICT,
        GCD_ERASE,
        GCD_FIXINDEX_RETRY,
        GCD_FIXDATA_RETRY,
    } GCDState;

typedef struct
{
    UInt32 weaveSeqAdd;
    UInt32 userWeaveSeq;
    UInt32 lba;
    UInt32 count;
} s_btoc_entry_t;
// Compile-time assertion to protect on-NAND format
WMR_CASSERT(sizeof(s_btoc_entry_t) == 16, s_btoc_entry_size_is_16);

typedef struct
{
    UInt32 blockStartLba;
    UInt32 blockStartCnt;
    UInt8 *blockStartBuf;
    BOOL32 gc, flatten, padding;
    UInt32 *lba;
    UInt32 *subCount;
    UInt8 *buf;
    PageMeta_t *meta;
    UInt32 count;
    UInt32 len;
    UInt32 cur;
} s_write_multi_t;

typedef struct
{
    // In:
    UInt32 in_sb;

    // Work FIFO for error handling
    GC_Fifo_t workFifo;

    // Invalid history
    UInt32 invalidHist[S_GC_VALIDHIST_SIZE];
    UInt32 invalidHistSize;
    UInt32 invalidHistIdx;
    UInt32 invalidHistSum;

    // Choose out:
    UInt32 sb;
    UInt32 chosenMerit;
    UInt32 chosenValid;
    UInt32 chosenErases;
    UInt32 chosenSize;

    // BTOC:
    s_btoc_entry_t *BTOC;
    UInt32 BTOC_len;
    UInt32 curBteIdx;

    UInt32 ueccCounter;

    // State:
    BOOL32 done;
    GCDState state;
    UInt32 vba;

    L2V_SearchCtx_t uecc_c; // For uECC lba search
} GC_Ctx_t;

typedef struct
{
    UInt8  type;
    UInt8  num_btoc_vbas_AND_staticFlag;
    UInt16 validLbas;
    UInt16 __was_reads;
    UInt16 erases;
} s_sb_fmt_t;
// Compile-time assertions to protect on-NAND format
WMR_CASSERT(sizeof(s_sb_fmt_t) == 8, s_sb_fmt_size_is_8);

typedef struct {
    UInt8  type;
    UInt16 num_btoc_vbas_AND_staticFlag;
    UInt16 validLbas;
    UInt16 erases;
    UInt32 reads;
} s_sb_t;
WMR_CASSERT(sizeof(s_sb_t) == 12, s_sb_size_is_12);

typedef struct
{
    UInt32 min;
    UInt32 max;
    UInt32 avg;
    UInt32 count;
} s_sb_userSeq_t;

typedef struct
{
    // Per-stream state:
    WeaveSeq_t minWeaveSeq;
    s_btoc_entry_t *BTOC;
    UInt32 BTOCidx;
    UInt32 BTOC_vba;
    UInt32 sb;
    UInt32 nextVbaOfs;
    UInt32 maxVbaOfs;
    UInt32 pageFlags;
    UInt32 gcDataAdjust;
    BOOL32 abandon;

#ifndef AND_READONLY
    // Buffered-up LBAs:
    PageMeta_t *bufMeta;
    UInt8 *bufPage;
    UInt32 bufLbas;
    UInt32 bufSize;
    UInt32 bufThreshold;
#endif // AND_READONLY
} s_wrstream_t;

typedef struct
{
#ifndef AND_READONLY
    // Allocated memory:
    PageMeta_t *meta;
    UInt8      *writePad;
    UInt32     meta_size;
#endif // AND_READONLY

    // Write state:
    WeaveSeq_t weaveSeq;

    s_wrstream_t stream[S_SBSTREAM_MAX];
    UInt32 curStream;
    s_write_multi_t drain_wm;
    s_write_multi_t pad_wm;
    UInt32 sum_gcDataAdjust;

    // Host state:
    s_write_multi_t host_wm;
} s_wrstate_t;

typedef struct
{
    UInt32 size;
    UInt32 curAge;
    UInt32 free;
    s_btoc_entry_t **BTOC;
    PageMeta_t *meta;
    UInt32 *sb;
    Int32 *age;
    WMR_BufZone_t BufZone; // Buffer zone allocator
    VFL_ReadSpans_t rs;

    // For srcVpn:
    UInt32 srcSize;
    UInt32 curSrc;
    UInt32 *srcSb;
    UInt32 **srcVpn;
} s_btoccache_t;

typedef struct {
    UInt32 vba;
    UInt32 span;
} s_cxt_contig_t;
// Compile-time assertion to protect on-NAND format
WMR_CASSERT(sizeof(s_cxt_contig_t) == 8, s_cxt_contig_size_is_8);

typedef struct {
    UInt32 sb;
    WeaveSeq_t minWeaveSeq;
    WeaveSeq_t maxWeaveSeq;
} s_wo_item_t;

typedef struct
{
    s_wo_item_t *item;
    UInt32 *sbFilter;
    UInt32 size;
    UInt32 idx;
    UInt32 updates;

    // Weave-ordering:
    s_btoc_entry_t *btoc[S_SBSTREAM_PHY_MAX];
    s_btoc_entry_t *bte[S_SBSTREAM_PHY_MAX];
    WeaveSeq_t weave[S_SBSTREAM_PHY_MAX];
    UInt32 itemIdx[S_SBSTREAM_PHY_MAX];
    UInt32 vba[S_SBSTREAM_PHY_MAX];
    UInt32 btoc_len[S_SBSTREAM_PHY_MAX];
} s_cxt_diff_t;

typedef struct {
    // Save
    struct {
        L2V_SearchCtx_t c;
        UInt32 sb[S_CXT_MAX_SB];
        UInt32 num_sb, num_sb_used;
        UInt32 idxPerVba;

        // Current batch
        s_cxt_contig_t *curContig;
        UInt32 curIdx, curVba;

        // Physical location
        UInt32 vba, maxVba;

        // Old cxt:
        UInt32 old_sb[S_CXT_MAX_SB];
        UInt32 old_num_sb;
    } save;

    // Load:
    struct {
        WeaveSeq_t baseWeaveSeq;
        VFL_ReadSpans_t rs;

        UInt32 baseSb[S_CXT_MAX_SB];
        WeaveSeq_t baseWeaves[S_CXT_MAX_SB];
        UInt32 bases;
        UInt32 baseCur;

        UInt32 total_clean;
        UInt32 scan_clean;
        UInt32 scan_uecc;
        UInt32 scan_ok;

        UInt32 curLba;

        BOOL32 sawStats_cxt;
        BOOL32 sawStats_lba;
        UInt32 stats_cxt_vba;
        BOOL32 loadedStats;
    } load;

    // Periodicity:
    struct
    {
        UInt32 pagesWritten;
        UInt32 sbsSince;
        UInt32 sbsSinceStats;
    } periodic;

    // Diff:
    s_cxt_diff_t diff;

    UInt32 cxt_vbas;
    UInt32 cxt_sbs;
} s_cxt_t;

typedef struct
{
    UInt64 lbas_read;
    UInt64 lbas_written;
    UInt64 lbas_gc;
    UInt64 lbas_flatten;

    UInt64 xacts_write;
    UInt64 xacts_read;

    UInt64 data_gc;
    UInt64 zero_valid_cross;

    UInt64 valid_lbas;
    UInt64 free_sb;
    UInt64 data_sb;
    UInt64 cxt_sb;
    UInt64 dead_sb;

    UInt64 boot_count;
    UInt64 unclean_boot_count;
    UInt64 refresh_gcs;
    UInt64 readCount_gcs;
    UInt64 wearLev_gcs;
    UInt64 shutdowns;

    UInt64 lbas_read_1;
    UInt64 lbas_read_2;
    UInt64 lbas_read_3;
    UInt64 lbas_read_4;
    UInt64 lbas_read_5;
    UInt64 lbas_read_6;
    UInt64 lbas_read_7;
    UInt64 lbas_read_8;
    UInt64 lbas_read_over_8;

    UInt64 lbas_write_1;
    UInt64 lbas_write_2;
    UInt64 lbas_write_3;
    UInt64 lbas_write_4;
    UInt64 lbas_write_5;
    UInt64 lbas_write_6;
    UInt64 lbas_write_7;
    UInt64 lbas_write_8;
    UInt64 lbas_write_over_8;

    UInt64 L2V_pool_free;
    UInt64 L2V_pool_count;

    UInt64 lbas_written_static;
    UInt64 lbas_written_dynamic;

    UInt64 span_xacts_read;
    UInt64 span_freebies_read;
} s_stats_t;

typedef struct
{
    PageMeta_BTOC_t *meta;
} s_boot_t;

typedef struct
{
    BOOL32 readsSince;
    UInt32 readCount_sb;
    UInt32 refresh_sb;
} s_readDist_t;

typedef struct
{
    L2V_SearchCtx_t c;
    UInt16 *validSums;
} s_dbg_t;


typedef struct {
    UInt32 lba;
    UInt32 lbaEnd;
} s_trim_item_t;

 typedef struct
 {
    Int32 max;
    Int32 count;
    UInt32 sumSpan;
    s_trim_item_t *item;
    UInt32 vulnerableWords;
    UInt32 *vulnerable;
    UInt32 bloomFilterWords;
    UInt32 *bloomFilter;
} s_trim_t;

typedef struct
{
    WMR_BufZone_t BufZone; // Buffer zone allocators
    UInt32 reported_lba; // exposed logical space
    UInt32 max_lba; // managed logical space
    BOOL32 booted;
#ifdef AND_SIMULATOR
    BOOL32 ignoreEraseGap;
#endif

    s_stats_t stats;
    s_boot_t boot;
    s_cxt_t cxt;
    s_wrstate_t write;
    s_btoccache_t BTOCcache;
    sftl_geom_t __geom;
    s_sb_t *sb;
    UInt16 *minEraseLocations;
    s_sb_userSeq_t *sb_userSeq;
    s_readDist_t readDist;
    s_dbg_t dbg;
    s_trim_t trim;

    // Single-page read buffer for convenience
    UInt8 *tmpBuf;
    // Scan meta buffer
    PageMeta_t *scan_meta;

    struct
    {
        Int32 data_sb;
        Int32 free_sb;
        Int32 erased_sb;
        Int32 dead_sb;
        Int32 cxt_sb;
        UInt32 validLbas;
        UInt32 vbas_in_free_sb;
    } seaState;

    struct
    {
        UInt32 blocksSince;
        BOOL32 pointOfNoReturn;
        UInt32 maxErases;
    } wearLev;

    // Garbage collection structures
    struct
    {
        // Shared state:
        BOOL32 inProgress;
        UInt32 zoneSize;
        L2V_SearchCtx_t read_c; // Search context for GC translation
        UInt32 *vbas;
        UInt32 *lbas;
        UInt32 *uecc;
        UInt8  *zone;
        PageMeta_t *meta;
        UInt32 curZoneSize;
        s_write_multi_t wm;
        VFL_ReadSpans_t rs;

        GC_Ctx_t ctx[S_GC_NUM_CTX];

        WMR_BufZone_t BufZone; // Buffer zone allocator
        PageMeta_t *meta_buf;
    } gc;

    struct {
        struct {
            FTLExtent_t *extents;
            UInt32 offset;
        } batch;
        PageMeta_t *meta;
        // Read search context
        L2V_SearchCtx_t c;
        VFL_ReadSpans_t rs;
    } read;
    VFLFunctions *vfl;
} sftl_t;

extern sftl_t sftl;
extern BOOL32  sftl_init_done;

#endif // __S_INTERNAL_H__

