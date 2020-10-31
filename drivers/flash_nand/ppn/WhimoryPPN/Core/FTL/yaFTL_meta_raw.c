/*
 * Copyright (c) 2008-2009 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */


#include "yaFTL_whoami.h"
#include "yaFTL_meta_raw.h"
#include "WMRConfig.h"
#include "yaFTLTypes.h"

extern yaFTL_t yaFTL;


void SetupMeta_Data(PageMeta_t *meta, UInt32 lba, UInt32 count)
{
    if (yaFTL.wrState.lastBlock != yaFTL.wrState.data.block)
    {
        yaFTL.wrState.weaveSeq++;
        yaFTL.wrState.lastBlock = yaFTL.wrState.data.block;
    }

    while (count--)
    {
        meta->FTLFlags = PAGETYPE_FTL_DATA;
        meta->lba = lba;
        meta->weaveSeq = (UInt32)yaFTL.wrState.weaveSeq;

        // Next
        lba++;
        meta++;
    }
}

void SetupMeta_Data_UECC(PageMeta_t *meta)
{
    meta->FTLFlags |= PFLAG_UECC;
}

void SetupMeta_Index(PageMeta_t *meta, UInt32 ipn)
{
    if (yaFTL.wrState.lastBlock != yaFTL.wrState.index.block)
    {
        yaFTL.wrState.weaveSeq++;
        yaFTL.wrState.lastBlock = yaFTL.wrState.index.block;
    }

    meta->FTLFlags = PAGETYPE_FTL_IDX;
    meta->lba = ipn;
    meta->weaveSeq = (UInt32)yaFTL.wrState.weaveSeq;
}

void SetupMeta_Data_BTOC(PageMeta_t *meta)
{
    meta->FTLFlags = PFLAG_BTOC;
    meta->weaveSeq = (UInt32)yaFTL.wrState.data.minWeaveSeq;
}

void SetupMeta_Index_BTOC(PageMeta_t *meta)
{
    meta->FTLFlags = PAGETYPE_FTL_IDX | PFLAG_BTOC;
    meta->weaveSeq = (UInt32)yaFTL.wrState.index.minWeaveSeq;
}

void SetupMeta_IndexGC(PageMeta_t *meta, UInt32 count)
{
    if (yaFTL.wrState.lastBlock != yaFTL.wrState.index.block)
    {
        yaFTL.wrState.weaveSeq++;
        yaFTL.wrState.lastBlock = yaFTL.wrState.index.block;
    }

    while (count--)
    {
        meta->FTLFlags = PAGETYPE_FTL_IDX;
        meta->weaveSeq = (UInt32)yaFTL.wrState.weaveSeq;

        // Next
        meta++;
    }
}

void SetupMeta_DataGC(PageMeta_t *meta, UInt32 count)
{
    if (yaFTL.wrState.lastBlock != yaFTL.wrState.data.block)
    {
        yaFTL.wrState.weaveSeq++;
        yaFTL.wrState.lastBlock = yaFTL.wrState.data.block;
    }

    while (count--)
    {
        // Or in the page type and clear all but uECC flag
        meta->FTLFlags = (meta->FTLFlags & PFLAG_UECC) | PAGETYPE_FTL_DATA;
        meta->weaveSeq = (UInt32)yaFTL.wrState.weaveSeq;

        // Next
        meta++;
    }
}

void SetupMeta_Cxt(PageMeta_t *meta, UInt64 weaveSeq)
{
    yaFTL.wrState.weaveSeq++;

    meta->FTLFlags = PAGETYPE_FTL_CXT;
    meta->weaveSeq = (UInt32)yaFTL.wrState.weaveSeq;
}

