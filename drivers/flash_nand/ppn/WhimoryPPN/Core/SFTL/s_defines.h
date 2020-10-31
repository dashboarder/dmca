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

#ifndef __S_DEFINES_H__
#define __S_DEFINES_H__

#define S_RESERVE_QUANTIZE 8
#define S_RESERVE_PERMIL_APPROX 68

#if AND_SIMULATOR
#define S_DBG_CONSISTENCY 1
#endif

#define S_LBAS_INTERNAL 16 // LBAs reserved for TL-internal data (such as stats)
#define S_LBA_STATS (sftl.max_lba - 1)

#define S_CXT_PERIOD_SBS 100
#define S_STATS_PERIOD_SBS 10
#define S_CXT_MAX_SB 32

#define S_BTOCCACHE_SRCSIZE  2
#define S_SCAN_META_SIZE 256
#define S_BTOC_META_SIZE 64
#define S_GCZONE_DOUBLEUPTO 16
#define S_GCFIFO_DEPTH 32
#define S_GCFIFO_FULL 4
#define S_GC_NUM_CTX 2
#define S_GC_CTX_FG 0
#define S_GC_CTX_BG 1
#define S_GC_VALIDHIST_SIZE 16

#define S_READDIST_LIMIT_PER_SB S_RC_THRESHOLD
#define S_READDIST_LIMIT_PER_OPEN_SB (10000)
#define S_READDIST_PERIOD 100 // Only consider counter-relocations every x reads

#define S_WEARLEV_PERIOD 20
#define S_WEARLEV_MIN_GAP 50
#define S_WEARLEV_DYN_GAP 10

#define S_BOOT_MAX_GC 20

#define S_WEAVE_SEQ_ADD_TRS 0x8000000

#define S_TOKEN_LBA_BASE 0xffff0000

#define S_LOWMEM_REPACK_MAX 10
#define S_TRIM_CUTOFF_LOWMEM 256
#define S_TRIM_SUMSPAN_TRIGGER 1024
#define S_TRIM_BLOOMFILTER_SPAN 1024

#define USERSEQ_OLD_THRESH (sftl.reported_lba >> 16) // one cycle through the drive
#define S_USERSEQ_IS_OLD(x) ((x + USERSEQ_OLD_THRESH) < (UInt32)(sftl.write.weaveSeq >> 16))

// Superblock types:
typedef enum {
    S_SB_UNKNOWN = 0,
    S_SB_ERASED = 1,
    S_SB_PENDING_ERASE = 2,
    S_SB_DATA = 3,
    S_SB_DATA_CUR = 4,
    S_SB_DATA_GC = 5,
    S_SB_DATA_PENDING_GC = 6,
    S_SB_CXT = 7,
    S_SB_DEAD = 8,
    S_SB_ERASE_BEFORE_USE = 9,
} s_sb_type_e;

typedef enum {
    S_SBSTREAM_DYN = 0,
    S_SBSTREAM_STATIC = 1,
    S_SBSTREAM_WEARLEV = 2,
    S_SBSTREAM_MAX
} s_sbstream_type_e;
#define S_SBSTREAM_PHY_MAX 4

#define MIN_NOT_ZERO(a, b)            (0 == (b) ? (a) : (0 == (a) ? (b) : WMR_MIN(a, b)))
#define S_SBSTREAM_PAGES(_max)        MIN_NOT_ZERO(s_g_num_banks, (_max) / s_g_bytes_per_page)
#define S_SBSTREAM_BUFSIZE(_st, _max) (((_st) == 0) ?  (s_g_vbas_per_page * S_SBSTREAM_PAGES(_max)) : (s_g_vbas_per_page))

#define S_ROUNDUP_POW2(v, g)       (((v) + (g) - 1) & ~((g) - 1))
#define S_WRITE_LBAS_THRESHOLD(s)  WMR_MAX(S_ROUNDUP_POW2((s)->bufSize >> 2, s_g_vbas_per_page), /* 25% for large configs */ \
                                           WMR_MIN((s)->bufSize, s_g_vbas_per_page << 1))        /* 2 pages for small configs */

// Dependent on maximum SB streams:
#define S_BTOCCACHE_SIZE (2*S_SBSTREAM_MAX)
#define S_GC_FREE_THRESHOLD (2 + S_SBSTREAM_PHY_MAX)
#define S_GC_LOW_GAP 5
#define S_GC_IDLELOW_GAP (S_GC_LOW_GAP+5)

#define S_TOK_PAD    (S_TOKEN_LBA_BASE+0)
#define S_TOK_DELETE (S_TOKEN_LBA_BASE+1)
#define S_TOK_ERASED 0xffffffff
#define S_TOK_UECC   0xffffff00

typedef enum {
    S_CXTTAG_BASE = 1,
    S_CXTTAG_STATS = 2,
    S_CXTTAG_SB = 3,
    S_CXTTAG_TREE = 4,
    S_CXTTAG_USERSEQ = 5,
    S_CXTTAG_READS = 6,
} s_cxttag_e;
#define S_CXTTAG_CLEAN 0xff

#endif // __S_DEFINES_H__

