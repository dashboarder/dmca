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

#include "s_dbg.h"
#include "s_sb.h"
#include "s_geom.h"

// Debug self-consistency checks
#if S_DBG_CONSISTENCY && !defined(AND_READONLY)

//S_DBG_CONSISTENCY==1
BOOL32 s_dbg_init(void)
{
    sftl.dbg.validSums = WMR_MALLOC(s_g_max_sb * sizeof(*sftl.dbg.validSums));

    return (NULL != sftl.dbg.validSums);
}

//S_DBG_CONSISTENCY==1
void s_dbg_close(void)
{
    if (NULL != sftl.dbg.validSums) {
        WMR_FREE(sftl.dbg.validSums, s_g_max_sb * sizeof(*sftl.dbg.validSums));
    }

    sftl.dbg.validSums = NULL;
}

//S_DBG_CONSISTENCY==1
void s_dbg_check_validSums(void)
{
    UInt32 lba, sb;
    UInt32 i;

    WMR_MEMSET(&sftl.dbg.validSums[0], 0, s_g_max_sb * sizeof(*sftl.dbg.validSums));

    // Make a complete pass through the tree, accumulating valid spans into a shadow
    // superblock valid counter array.
    lba = 0;
    L2V_Search_Init(&sftl.dbg.c);
    while (lba < sftl.max_lba) {
        sftl.dbg.c.lba = lba;
        L2V_Search(&sftl.dbg.c);
        if (sftl.dbg.c.vba < L2V_VBA_SPECIAL) {
            sb = s_g_vba_to_sb(sftl.dbg.c.vba);
            WMR_ASSERT(sb < s_g_max_sb);
            sftl.dbg.validSums[sb] += sftl.dbg.c.span;
        }

        // Next
        lba += sftl.dbg.c.span;
    }

    // Compare computed valids with maintained valids--should be exactly the same.
    for (i = 0; i < s_g_max_sb; i++) {
        WMR_ASSERT(sftl.dbg.validSums[i] == sftl.sb[i].validLbas);
    }
}

//S_DBG_CONSISTENCY==1
void s_dbg_check_data_counts(void)
{
    UInt32 i, sum = 0;
    UInt32 free_sum = 0;
    UInt32 type;

    for (i = 0; i < s_g_max_sb; i++)
    {
        sum += s_sb_get_validLbas(i);
        type = s_sb_get_type(i);
        if ((S_SB_ERASED == type)
            || (S_SB_PENDING_ERASE == type)
            || (S_SB_ERASE_BEFORE_USE == type))
        {
            free_sum += sftl.vfl->GetVbasPerVb(i);
        }
    }
    WMR_ASSERT(sum == sftl.seaState.validLbas);
    WMR_ASSERT(free_sum == sftl.seaState.vbas_in_free_sb);
}

//S_DBG_CONSISTENCY==1
#if AND_SIMULATOR

#define MAX_ERASE_GAP 100
#define MAX_ERASE_LIMIT 3000

typedef struct
{
    UInt32 eraseMin;
    UInt32 minCounter;
    UInt32 eraseMax;
    UInt32 maxCounter;
    UInt32 eraseAve;
    BOOL32 populated;
    s_stats_t statSnapShot;
} erase_stat;

erase_stat eraseStatArray[MAX_ERASE_LIMIT / MAX_ERASE_GAP] = {0};

BOOL32 checkWL(UInt32 index)
{
    BOOL32 retStatus = TRUE32;

    if((index > 1) && (index <= (MAX_ERASE_LIMIT / MAX_ERASE_GAP)))
    {
        if((eraseStatArray[index - 2].eraseMin >= eraseStatArray[index - 1].eraseMin) ||
           (eraseStatArray[index - 2].eraseAve >= eraseStatArray[index - 1].eraseAve))
        {
            UInt32 i;

            for (i = 0; i < s_g_max_sb; i++)
            {
                if((s_sb_get_type(i) != S_SB_DEAD) && (s_sb_get_erases(i) == eraseStatArray[index - 1].eraseMin))
                {
                    WMR_PRINT(ALWAYS,"block %d has type 0x%x validlbas 0x%x no of erases %d\n",
                              i, s_sb_get_type(i), s_sb_get_validLbas(i), s_sb_get_erases(i));
                }
            }

            retStatus = FALSE32;

            if(sftl.ignoreEraseGap)
            {
                WMR_PRINT(ALWAYS, "Does not look like wearleveling works Min : %d c %d - %d c %d, Ave : %d - %d, Max %d c %d - %d c %d\n",
                          eraseStatArray[index - 2].eraseMin, eraseStatArray[index - 2].minCounter,
                          eraseStatArray[index - 1].eraseMin, eraseStatArray[index - 1].minCounter,
                          eraseStatArray[index - 2].eraseAve, eraseStatArray[index - 1].eraseAve,
                          eraseStatArray[index - 2].eraseMax, eraseStatArray[index - 2].maxCounter,
                          eraseStatArray[index - 1].eraseMax, eraseStatArray[index - 1].minCounter);
            }
            else
            {
                WMR_PANIC("Does not look like wearleveling works Min : %d c %d - %d c %d, Ave : %d - %d, Max %d c %d - %d c %d\n",
                          eraseStatArray[index - 2].eraseMin, eraseStatArray[index - 2].minCounter,
                          eraseStatArray[index - 1].eraseMin, eraseStatArray[index - 1].minCounter,
                          eraseStatArray[index - 2].eraseAve, eraseStatArray[index - 1].eraseAve,
                          eraseStatArray[index - 2].eraseMax, eraseStatArray[index - 2].maxCounter,
                          eraseStatArray[index - 1].eraseMax, eraseStatArray[index - 1].minCounter);
            }
        }
    }

    return retStatus;
}

#endif

void s_dbg_check_sb_dist(void)
{
    UInt32 i;
    Int32 sb_data, sb_erased, sb_free, sb_cxt, sb_dead;
    BOOL32 err = FALSE32;
#if AND_SIMULATOR
    UInt32 maxErase = 0, minErase = 0xffffffff, accumErases = 0, accumCounter = 0, arrayEntry, maxCounter = 0, minCounter = 0;
#endif

    // Don't check during GC of power-on restore
    if (!sftl.booted)
        return;

    sb_data = sb_erased = sb_free = sb_cxt = sb_dead = 0;

    for (i = 0; i < s_g_max_sb; i++) {
#if AND_SIMULATOR
        if(s_sb_get_type(i) != S_SB_DEAD)
        {
            accumErases += s_sb_get_erases(i);
            accumCounter++;
            if(s_sb_get_erases(i) > maxErase)
            {
                maxErase = s_sb_get_erases(i);
                maxCounter = 1;
            }
            else
            {
                if(maxErase == s_sb_get_erases(i))
                {
                    maxCounter++;
                }
            }
            if(s_sb_get_erases(i) < minErase)
            {
                minErase = s_sb_get_erases(i);
                minCounter = 1;
            }
            else
            {
                if(minErase == s_sb_get_erases(i))
                {
                    minCounter++;
                }
            }
        }

#endif

        switch (s_sb_get_type(i)) {
            case S_SB_DEAD:
                sb_dead++;
                break;

            case S_SB_DATA:
            case S_SB_DATA_CUR:
            case S_SB_DATA_GC:
            case S_SB_DATA_PENDING_GC:
                sb_data++;
                break;

            case S_SB_ERASED:
                sb_erased++;
                sb_free++;
                break;

            case S_SB_PENDING_ERASE:
            case S_SB_ERASE_BEFORE_USE:
                sb_free++;
                break;

            case S_SB_CXT:
                sb_cxt++;
                break;

            default:
                WMR_PANIC("unknown block type: %d", s_sb_get_type(i));
                break;
        }
    }

#if AND_SIMULATOR
    arrayEntry = maxErase / MAX_ERASE_GAP;
    if((arrayEntry) && (arrayEntry <= (MAX_ERASE_LIMIT / MAX_ERASE_GAP)) && ( eraseStatArray[arrayEntry - 1].populated == FALSE32))
    {
        eraseStatArray[arrayEntry - 1].eraseMin = minErase;
        eraseStatArray[arrayEntry - 1].minCounter = minCounter;
        eraseStatArray[arrayEntry - 1].eraseMax = maxErase;
        eraseStatArray[arrayEntry - 1].maxCounter = maxCounter;
        eraseStatArray[arrayEntry - 1].eraseAve = accumErases / accumCounter;
        eraseStatArray[arrayEntry - 1].populated = TRUE32;
        WMR_MEMCPY(&(eraseStatArray[arrayEntry - 1].statSnapShot), &(sftl.stats), sizeof(s_stats_t));
        checkWL(arrayEntry);
    }
#endif

    if (sb_data != sftl.seaState.data_sb)
    {
        WMR_PRINT(ERROR, "Data sb counts don't match; %d != %d\n", sb_data, sftl.seaState.data_sb);
        err = TRUE32;
    }
    if (sb_cxt != sftl.seaState.cxt_sb)
    {
        WMR_PRINT(ERROR, "Cxt sb counts don't match; %d != %d\n", sb_cxt, sftl.seaState.cxt_sb);
        err = TRUE32;
    }
    if (sb_erased != sftl.seaState.erased_sb)
    {
        WMR_PRINT(ERROR, "Erased sb counts don't match; %d != %d\n", sb_erased, sftl.seaState.erased_sb);
        err = TRUE32;
    }
    if (sb_free != sftl.seaState.free_sb)
    {
        WMR_PRINT(ERROR, "Free sb counts don't match; %d != %d\n", sb_free, sftl.seaState.free_sb);
        err = TRUE32;
    }
    if (sb_dead != sftl.seaState.dead_sb)
    {
        WMR_PRINT(ERROR, "Dead sb counts don't match; %d != %d\n", sb_dead, sftl.seaState.dead_sb);
        err = TRUE32;
    }
    if (sftl.seaState.free_sb < 0)
    {
        WMR_PRINT(ERROR, "Free sb underflow error\n");
        err = TRUE32;
    }
    if (sftl.seaState.data_sb < 0)
    {
        WMR_PRINT(ERROR, "Data sb underflow error\n");
        err = TRUE32;
    }
    if (sftl.seaState.dead_sb < 0)
    {
        WMR_PRINT(ERROR, "Dead sb underflow error\n");
        err = TRUE32;
    }
    if ((sb_data+sb_free+sb_cxt+sb_dead) != (Int32)s_g_max_sb) {
        WMR_PRINT(ERROR, "sum not right: %d != %d\n", sb_data+sb_free+sb_cxt+sb_dead, s_g_max_sb);
        err = TRUE32;
    }
    WMR_ASSERT(FALSE32 == err);
}

#endif

