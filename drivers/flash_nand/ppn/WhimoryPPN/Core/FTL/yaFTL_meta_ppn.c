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
#include "yaFTL_meta_ppn.h"
#include "WMRConfig.h"
#include "yaFTLTypes.h"

extern yaFTL_t yaFTL;


void SetupMeta_Data(PageMeta_t *__meta, UInt32 lba, UInt32 count)
{
    PageMeta_Data_t *m = (PageMeta_Data_t*)__meta;

    while (count--) {
        yaFTL.wrState.weaveSeq++;

        m->c.PageType = PAGETYPE_FTL_DATA;
        m->c.FTLFlags = 0;
        m->lba = lba;
        m->c.weaveLo = yaFTL.wrState.weaveSeq;
        m->c.weaveHi = (UInt32)(yaFTL.wrState.weaveSeq >> 16);
        m->userSeq = (UInt32)(yaFTL.wrState.weaveSeq >> 16);

        // Next
        lba++;
        m++;
    }
}

void SetupMeta_Data_UECC(PageMeta_t *__meta)
{
    __meta->c.FTLFlags |= PFLAG_UECC;
}

void SetupMeta_Index(PageMeta_t *__meta, UInt32 ipn)
{
    PageMeta_Idx_t *m = (PageMeta_Idx_t*)__meta;

    yaFTL.wrState.weaveSeq++;

    m->c.PageType = PAGETYPE_FTL_IDX;
    m->c.FTLFlags = 0;
    m->c.weaveLo = yaFTL.wrState.weaveSeq;
    m->c.weaveHi = (UInt32)(yaFTL.wrState.weaveSeq >> 16);
    m->baseLba = ipn * yaFTL.indexPageRatio;
}

void SetupMeta_Data_BTOC(PageMeta_t *__meta)
{
    PageMeta_BTOC_t *m = (PageMeta_BTOC_t*)__meta;

    m->c.PageType = PAGETYPE_FTL_BTOC_DATA;
    m->c.FTLFlags = 0;
    m->c.weaveLo = yaFTL.wrState.data.minWeaveSeq;
    m->c.weaveHi = (UInt32)(yaFTL.wrState.data.minWeaveSeq >> 16);
    m->maxWeaveSeqAdd = (UInt32)(yaFTL.wrState.weaveSeq - yaFTL.wrState.data.minWeaveSeq);
    m->__rfu__ = 0xffffffff;
}

void SetupMeta_Index_BTOC(PageMeta_t *__meta)
{
    PageMeta_BTOC_t *m = (PageMeta_BTOC_t*)__meta;

    m->c.PageType = PAGETYPE_FTL_BTOC_IDX;
    m->c.FTLFlags = 0;
    m->c.weaveLo = yaFTL.wrState.index.minWeaveSeq;
    m->c.weaveHi = (UInt32)(yaFTL.wrState.index.minWeaveSeq >> 16);
    m->maxWeaveSeqAdd = (UInt32)(yaFTL.wrState.weaveSeq - yaFTL.wrState.index.minWeaveSeq);
    m->__rfu__ = 0xffffffff;
}

void SetupMeta_IndexGC(PageMeta_t *__meta, UInt32 count)
{
    PageMeta_Idx_t *m = (PageMeta_Idx_t*)__meta;

    while (count--) {
        yaFTL.wrState.weaveSeq++;

        m->c.PageType = PAGETYPE_FTL_IDX;
        m->c.FTLFlags = 0;
        m->c.weaveLo = yaFTL.wrState.weaveSeq;
        m->c.weaveHi = (UInt32)(yaFTL.wrState.weaveSeq >> 16);

        // Next
        m++;
    }
}

void SetupMeta_DataGC(PageMeta_t *__meta, UInt32 count)
{
    PageMeta_Data_t *m = (PageMeta_Data_t*)__meta;

    while (count--) {
        yaFTL.wrState.weaveSeq++;

        m->c.PageType = PAGETYPE_FTL_DATA;
        // Don't touch FTLFlags--might have uECC flag
        m->c.weaveLo = yaFTL.wrState.weaveSeq;
        m->c.weaveHi = (UInt32)(yaFTL.wrState.weaveSeq >> 16);
        // Don't touch LBA or userSeq

        // Next
        m++;
    }
}

void SetupMeta_Cxt(PageMeta_t *__meta, UInt64 weaveSeq)
{
    PageMeta_Cxt_t *m = (PageMeta_Cxt_t*)__meta;

    yaFTL.wrState.weaveSeq++;

    m->c.PageType = PAGETYPE_FTL_CXT;
    m->c.FTLFlags = 0;
    m->c.weaveLo = weaveSeq;
    m->c.weaveHi = (UInt32)(weaveSeq >> 16);
    m->__rfu1__ = 0;
    m->__rfu2__ = 0;
}

