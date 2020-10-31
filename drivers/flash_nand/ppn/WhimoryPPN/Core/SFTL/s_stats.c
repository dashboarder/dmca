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

#include "s_stats.h"
#include "s_geom.h"
#include "s_token.h"
#include "FTL.h"

#ifndef AND_READONLY

void voidFunc(UInt32 dwStructType, void *pvoidStructBuffer, UInt32 dwStructSize) {
}

#undef X_FTL_doStat
#define X_FTL_doStat(_part, _val) do { if (key == S_STATKEY_ ## _part) { WMR_PRINT(ALWAYS, #_part " = %lld\n", val64); continue; } } while(0);
FTL_makeStatParser(print_stats, voidFunc, voidFunc)

UInt32 s_stats_to_buf(UInt32 *buf)
{
    UInt32 *baseBuf;
    UInt32 thisSize, dwords;

    baseBuf = buf;

    // Number of elements
    *buf++ = (sizeof(s_stats_t)/sizeof(UInt64)) + 2;

    // sftl stats:
#undef X_FTL_doStat
#define X_FTL_doStat(_part, _val) do { buf[0] = (S_STATKEY_ ## _part); buf[1] = 2; *(UInt64*)&buf[2] = sftl.stats._part; buf += 4; } while(0);
    FTL_doAllStats
    WMR_ASSERT((buf - baseBuf) == (((sizeof(s_stats_t) * 2) / sizeof(UInt32)) + 1));

    // VFL
    thisSize = AND_STATISTICS_SIZE_PER_LAYER;
    sftl.vfl->GetStruct(AND_STRUCT_VFL_STATISTICS, ((UInt8*)buf)+sizeof(UInt64), &thisSize);
    buf[0] = S_STATKEY_VFL;
    dwords = (thisSize + sizeof(UInt32) - 1) / sizeof(UInt32);
    buf[1] = dwords;
    buf += 2;
    buf += dwords;

    // FIL
    thisSize = AND_STATISTICS_SIZE_PER_LAYER;
    sftl.vfl->GetStruct(AND_STRUCT_VFL_FILSTATISTICS, ((UInt8*)buf)+sizeof(UInt64), &thisSize);
    buf[0] = S_STATKEY_FIL;
    dwords = (thisSize + sizeof(UInt32) - 1) / sizeof(UInt32);
    buf[1] = dwords;
    buf += 2;
    buf += dwords;

    WMR_ASSERT((UInt32)((UInt8*)buf - (UInt8*)baseBuf) <= s_g_bytes_per_lba);
    return (UInt32)((UInt8*)buf - (UInt8*)baseBuf);
}

static void _UpdateStatisticsCounters(UInt8 * src, void *stat_buff, UInt32 size)
{
    UInt32 idx;
    const UInt32 num_of_items = (size / sizeof(UInt64));
    UInt64 * stat_buff_u64 = (UInt64 *)stat_buff;
	
    for (idx = 0; idx < num_of_items; idx++) {
        UInt64 *temp_ptr_u64 = (UInt64*)&src[idx * sizeof(UInt64)];
        stat_buff_u64[idx] += *temp_ptr_u64;
    }
}

BOOL32 _updateLowerStats(UInt32 struct_type, void * struct_buffer, UInt32 struct_size)
{
    UInt32 this_size;
    UInt64 curr_stat_buf[AND_STATISTICS_SIZE_PER_LAYER/sizeof(UInt64)];

    this_size = AND_STATISTICS_SIZE_PER_LAYER;
    sftl.vfl->GetStruct(struct_type, curr_stat_buf, &this_size);
    _UpdateStatisticsCounters((UInt8*)struct_buffer, curr_stat_buf, this_size);
    sftl.vfl->SetStruct(struct_type, curr_stat_buf, this_size);

    return TRUE32;
}

#undef X_FTL_doStat
#define X_FTL_doStat(_part, _val) do { if (key == S_STATKEY_ ## _part) { sftl.stats._part += val64; continue; } } while(0);
FTL_makeStatParser(s_stats_from_buf_jump, _updateLowerStats, _updateLowerStats)
void s_stats_from_buf(UInt8 *buf, UInt32 size)
{
    WMR_MEMSET(&sftl.stats, 0, sizeof(sftl.stats));
    s_stats_from_buf_jump(buf, size);
}

void s_stats_insert(void)
{
    UInt32 *buf;

    // Put stats in buffer
    buf = (UInt32*)sftl.tmpBuf;
    s_stats_to_buf(buf);

    // Insert the token
    s_token_insert(S_LBA_STATS, buf);
}

void s_stats_update(void)
{
    // Update stats from various places
    sftl.stats.valid_lbas = sftl.seaState.validLbas;
    sftl.stats.free_sb = sftl.seaState.free_sb;
    sftl.stats.data_sb = sftl.seaState.data_sb;
    sftl.stats.cxt_sb = sftl.seaState.cxt_sb;
    sftl.stats.dead_sb = sftl.seaState.dead_sb;
    sftl.stats.L2V_pool_free = L2V.Pool.FreeCount;
    sftl.stats.L2V_pool_count = L2V_nodepool_mem / sizeof(lNode_t);
}

#endif // !AND_READONLY
