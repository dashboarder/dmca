/*
 * Copyright (c) 2009 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */

#include "yaFTL_whoami.h"
#include "WMRConfig.h"
#include "yaFTL_BTOC.h"
#include "ANDTypes.h"
#include "WMROAM.h"
#include "VFLBuffer.h"
#include "VFL.h"
#include "FTL.h"
#include "yaFTLTypes.h"
#include "yaFTL_Defines.h"

// Extern context struct
extern yaFTL_t yaFTL;
extern FTLWMRDeviceInfo yaFTL_FTLDeviceInfo;
extern Int32 _readPage(UInt32 vpn, UInt8 *pageData, PageMeta_t *mdPtr, BOOL32 bInternalOp, BOOL32 boolCleanCheck, BOOL32 scrubOnUECC);
extern ANDStatus invalidateCXT(void);

BOOL32 BTOC_Init()
{
    UInt32 i;

    yaFTL.BTOCcache.size = YAFTL_BTOCCACHE_SIZE_PER * 2;
    WMR_ASSERT(yaFTL.BTOCcache.size <= 32);
    yaFTL.BTOCcache.srcSize = YAFTL_BTOCCACHE_SRCSIZE;
    yaFTL.BTOCcache.curSrc = yaFTL.BTOCcache.srcSize-1;

    yaFTL.BTOCcache.curAge = 0;
    yaFTL.BTOCcache.age = WMR_MALLOC(sizeof(*yaFTL.BTOCcache.age) * yaFTL.BTOCcache.size);
    yaFTL.BTOCcache.sb = WMR_MALLOC(sizeof(*yaFTL.BTOCcache.sb) * yaFTL.BTOCcache.size);
    yaFTL.BTOCcache.BTOC = WMR_MALLOC(sizeof(*yaFTL.BTOCcache.BTOC) * yaFTL.BTOCcache.size);
    yaFTL.BTOCcache.srcVpn = WMR_MALLOC(sizeof(*yaFTL.BTOCcache.srcVpn) * yaFTL.BTOCcache.srcSize);
    yaFTL.BTOCcache.srcSb = WMR_MALLOC(sizeof(*yaFTL.BTOCcache.srcSb) * yaFTL.BTOCcache.srcSize);
    yaFTL.BTOCcache.isLocked = 0;
    
    // Check for allocation failure
    if ((NULL == yaFTL.BTOCcache.age) || (NULL == yaFTL.BTOCcache.sb) || (NULL == yaFTL.BTOCcache.BTOC)
        || (NULL == yaFTL.BTOCcache.srcVpn) || (NULL == yaFTL.BTOCcache.srcSb))
    {
        return FALSE32;
    }

    for (i = 0; i < yaFTL.BTOCcache.size; i++)
    {
        yaFTL.BTOCcache.sb[i] = 0xffffffff;
    }

    for (i = 0; i < yaFTL.BTOCcache.srcSize; i++)
    {
        yaFTL.BTOCcache.srcSb[i] = 0xffffffff;
        yaFTL.BTOCcache.srcVpn[i] = WMR_MALLOC(sizeof(**yaFTL.BTOCcache.srcVpn) * PAGES_PER_SUBLK);
        WMR_MEMSET(yaFTL.BTOCcache.srcVpn[i], 0xff, sizeof(**yaFTL.BTOCcache.srcVpn) * PAGES_PER_SUBLK);
    }

    yaFTL.BTOCcache.free = (1 << yaFTL.BTOCcache.size) - 1;
    yaFTL.BTOCcache.isData = (1 << YAFTL_BTOCCACHE_SIZE_PER) - 1;

    // Allocate buffers
    WMR_BufZone_Init(&yaFTL.BTOCcache.BufZone);

    for (i = 0; i < yaFTL.BTOCcache.size; i++)
    {
        yaFTL.BTOCcache.BTOC[i] = (UInt32*)WMR_Buf_Alloc_ForDMA(&yaFTL.BTOCcache.BufZone, yaFTL.controlPageNo * BYTES_PER_PAGE);
    }

    if (!WMR_BufZone_FinishedAllocs(&yaFTL.BTOCcache.BufZone)) {
        return FALSE32;
    }

    for (i = 0; i < yaFTL.BTOCcache.size; i++)
    {
        WMR_BufZone_Rebase(&yaFTL.BTOCcache.BufZone, (void**)&yaFTL.BTOCcache.BTOC[i]);
    }

    WMR_BufZone_FinishedRebases(&yaFTL.BTOCcache.BufZone);

    return TRUE32;
}

void BTOC_Close()
{
    UInt32 i;

    WMR_BufZone_Free(&yaFTL.BTOCcache.BufZone);

    if (NULL != yaFTL.BTOCcache.age)
    {
        WMR_FREE(yaFTL.BTOCcache.age, sizeof(*yaFTL.BTOCcache.age) * yaFTL.BTOCcache.size);
    }
    if (NULL != yaFTL.BTOCcache.sb)
    {
        WMR_FREE(yaFTL.BTOCcache.sb, sizeof(*yaFTL.BTOCcache.sb) * yaFTL.BTOCcache.size);
    }
    if (NULL != yaFTL.BTOCcache.BTOC)
    {
        WMR_FREE(yaFTL.BTOCcache.BTOC, sizeof(*yaFTL.BTOCcache.BTOC) * yaFTL.BTOCcache.size);
    }
    // Free srcVpn memory
    if (NULL != yaFTL.BTOCcache.srcSb)
    {
        WMR_FREE(yaFTL.BTOCcache.srcSb, sizeof(*yaFTL.BTOCcache.srcSb) * yaFTL.BTOCcache.srcSize);
    }
    for (i = 0; i < yaFTL.BTOCcache.srcSize; i++)
    {
        WMR_FREE(yaFTL.BTOCcache.srcVpn[i], sizeof(**yaFTL.BTOCcache.srcVpn) * PAGES_PER_SUBLK);
    }
    if (NULL != yaFTL.BTOCcache.srcVpn)
    {
        WMR_FREE(yaFTL.BTOCcache.srcVpn, sizeof(*yaFTL.BTOCcache.srcVpn) * yaFTL.BTOCcache.srcSize);
    }

    yaFTL.BTOCcache.age = NULL;
    yaFTL.BTOCcache.sb = NULL;
    yaFTL.BTOCcache.BTOC = NULL;
    yaFTL.BTOCcache.srcSb = NULL;
    yaFTL.BTOCcache.srcVpn = NULL;

    yaFTL.BTOCcache.size = 0;

    yaFTL.BTOCcache.srcSize = 0;
}

void BTOC_BootFixup()
{
    UInt32 i;

    for (i = 0; i < yaFTL.BTOCcache.size; i++)
    {
        if (0xfffffffd == yaFTL.BTOCcache.sb[i])
        {
            yaFTL.BTOCcache.sb[i] = yaFTL.wrState.data.block;
        }
        if (0xfffffffe == yaFTL.BTOCcache.sb[i])
        {
            yaFTL.BTOCcache.sb[i] = yaFTL.wrState.index.block;
        }
    }
}

UInt32 *BTOC_Alloc(UInt32 sb, BOOL32 isData)
{
    UInt32 i, minIdx, isData32 = (isData ? 1 : 0);
    Int32 minAge;

    // Get a srcVpn
    yaFTL.BTOCcache.curSrc = (yaFTL.BTOCcache.curSrc + 1) % yaFTL.BTOCcache.srcSize;
    yaFTL.BTOCcache.srcSb[yaFTL.BTOCcache.curSrc] = sb;
    WMR_MEMSET(yaFTL.BTOCcache.srcVpn[yaFTL.BTOCcache.curSrc], 0xff, sizeof(**yaFTL.BTOCcache.srcVpn) * PAGES_PER_SUBLK);

    // Get a BTOC
    yaFTL.BTOCcache.curAge++;

    minAge = 0x7fffffff;
    minIdx = 0xffffffff;

    for (i = 0; i < yaFTL.BTOCcache.size; i++)
    {
        if (((yaFTL.BTOCcache.isData & (1 << i)) == (isData32 << i))
            && (yaFTL.BTOCcache.free & (1 << i))
            && (yaFTL.BTOCcache.age[i] < minAge)
            && ((yaFTL.BTOCcache.isLocked & (1 << i)) == 0))
        {
            minAge = yaFTL.BTOCcache.age[i];
            minIdx = i;
        }
    }

    if (0xffffffff != minIdx) {
        yaFTL.BTOCcache.free &= ~(1 << minIdx);
        yaFTL.BTOCcache.sb[minIdx] = sb;
        yaFTL.BTOCcache.age[minIdx] = yaFTL.BTOCcache.curAge;
        return yaFTL.BTOCcache.BTOC[minIdx];
    }
#ifndef AND_READONLY    
    // Invalidate context
    if (yaFTL.cxtValid == 1)
    {
        invalidateCXT();
        yaFTL.cxtValid = 0;
    }
    WMR_PANIC("Couldn't allocate a BTOC");
#endif    
    return NULL;
}

UInt32 BTOC_GetSrc(UInt32 destVpn)
{
    UInt32 i, sb, idx;

    sb = destVpn / PAGES_PER_SUBLK;
    idx = destVpn % PAGES_PER_SUBLK;

    for (i = 0; i < yaFTL.BTOCcache.srcSize; i++)
    {
        if (sb == yaFTL.BTOCcache.srcSb[i]) {
            return yaFTL.BTOCcache.srcVpn[i][idx];
        }
    }

    return 0xffffffff;
}

void BTOC_SetSrc(UInt32 destVpn, UInt32 srcVpn)
{
    UInt32 i, sb, idx;

    sb = destVpn / PAGES_PER_SUBLK;
    idx = destVpn % PAGES_PER_SUBLK;

    for (i = 0; i < yaFTL.BTOCcache.srcSize; i++)
    {
        if (sb == yaFTL.BTOCcache.srcSb[i]) {
            yaFTL.BTOCcache.srcVpn[i][idx] = srcVpn;
        }
    }
}

void BTOC_Dealloc(UInt32 *BTOC)
{
    UInt32 i;

    for (i = 0; i < yaFTL.BTOCcache.size; i++)
    {
        if (BTOC == yaFTL.BTOCcache.BTOC[i])
        {
            yaFTL.BTOCcache.free |= (1 << i);
            return;
        }
    }
#ifndef AND_READONLY    
    // Invalidate context
    if (yaFTL.cxtValid == 1)
    {
        invalidateCXT();
        yaFTL.cxtValid = 0;
    }
    WMR_PANIC("Couldn't deallocate BTOC: 0x%x", BTOC);
#endif
}

UInt32 *BTOC_Search(UInt32 sb, BOOL32 isData)
{
    UInt32 i, isData32 = (isData ? 1 : 0);
    Int32 maxAge = -1;
    UInt32 *best = NULL;

    for (i = 0; i < yaFTL.BTOCcache.size; i++)
    {
        if ((yaFTL.BTOCcache.sb[i] == sb) && (yaFTL.BTOCcache.age[i] > maxAge) && ((yaFTL.BTOCcache.isData & (1 << i)) == (isData32 << i)))
        {
            maxAge = yaFTL.BTOCcache.age[i];
            best = yaFTL.BTOCcache.BTOC[i];
        }
    }

    return best;
}

void BTOC_Lock(UInt32 sb)
{
    UInt32 i;
    
    for (i = 0; i < yaFTL.BTOCcache.size; i++)
    {
        if ((yaFTL.BTOCcache.sb[i] == sb) && ((yaFTL.BTOCcache.free & (1 << i)) == 0))
        {
            yaFTL.BTOCcache.isLocked |= (1 << i);
            break;
        }
    }
    
}

void BTOC_Unlock(UInt32 sb)
{
    UInt32 i;
    
    for (i = 0; i < yaFTL.BTOCcache.size; i++)
    {
        if ((yaFTL.BTOCcache.sb[i] == sb) && (yaFTL.BTOCcache.isLocked & (1 << i)))
        {
            yaFTL.BTOCcache.isLocked &= ~(1 << i);
            break;
        }
    }
    
}

ANDStatus BTOC_Read(UInt32 vpn, UInt32 *bTOC, PageMeta_t *mdPtr, BOOL32 val, BOOL32 scrubOnUECC, UInt32 upperBound)
{
    UInt16 i;
    ANDStatus status = FTL_CRITICAL_ERROR;

    for (i = 0; i < yaFTL.controlPageNo; i++)
    {
        status = _readPage(vpn + i, ((UInt8 *)bTOC) + (i * BYTES_PER_PAGE), mdPtr, val, TRUE32, scrubOnUECC);
        if (status != FTL_SUCCESS)
        {
            break;
        }
    }
    if (status == FTL_SUCCESS)
    {
        UInt32 * tmpPtr = (UInt32 *)bTOC;
        for(i = 0; i < ((yaFTL.controlPageNo * BYTES_PER_PAGE) >> 2);i++)
            if(tmpPtr[i] >= upperBound)
            {
                tmpPtr[i] = 0xffffffff; 
            }
    }
    return status;
}


void BTOC_SetAll(UInt32 *bTOC, UInt8 value)
{
    WMR_MEMSET(bTOC, value, yaFTL.controlPageNo * BYTES_PER_PAGE);
}


void BTOC_Copy(UInt32 *left, UInt32 *right, UInt32 upperBound)
{
    UInt16 i;
    for(i = 0; i < ((yaFTL.controlPageNo * BYTES_PER_PAGE) >> 2);i++)
        if(right[i] >= upperBound)
        {
            left[i] = 0xffffffff;
        }
        else 
        {
            left[i] = right[i];
        }
}


UInt32 BTOC_Get(UInt32 *bTOC, UInt32 offset, UInt32 upperBound)
{
    if(bTOC[offset] >= upperBound)
    {
        return 0xffffffff;
    }
    else
    {
        return bTOC[offset];
    }
}


UInt32 BTOC_Set(UInt32 *bTOC, UInt32 offset, UInt32 val, UInt32 upperBound)
{
    if(val >= upperBound)
    {
        bTOC[offset] = 0xffffffff;    
    }
    else 
    {
        bTOC[offset] = val;
    }
    return val;
}

