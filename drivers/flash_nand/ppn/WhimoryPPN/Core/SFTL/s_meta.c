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


#include "s_internal.h"


void s_SetupMeta_TrimRead(PageMeta_t *__meta, UInt32 lba, UInt32 count, UInt32 flags)
{
    PageMeta_Data_t *m = (PageMeta_Data_t*)__meta;

    while (count--) {
        m->c.PageType = PAGETYPE_FTL_DATA;
        m->c.FTLFlags = flags;
        m->lba = lba;
        m->c.weaveLo = sftl.write.weaveSeq;
        m->c.weaveHi = (UInt32)(sftl.write.weaveSeq >> 16);
        m->userSeq = (UInt32)(sftl.write.weaveSeq >> 16);

        // Next
        lba++;
        m++;
    }
}

void s_SetupMeta_Data_Padding(PageMeta_t *meta, UInt32 count, UInt32 flags)
{
    UInt32 i;
    
    for(i = 0; i < count ; i++)
    {
        s_SetupMeta_Data(&meta[i], S_TOK_PAD, 1, flags);
    }
    
}

void s_SetupMeta_Trim(PageMeta_t *__meta, UInt32 lba, UInt32 count, UInt32 flags)
{
    s_SetupMeta_Data(__meta, lba, count, flags);
}

void s_SetupMeta_Data(PageMeta_t *__meta, UInt32 lba, UInt32 count, UInt32 flags)
{
    PageMeta_Data_t *m = (PageMeta_Data_t*)__meta;

    while (count--) {
        m->c.PageType = PAGETYPE_FTL_DATA;
        m->c.FTLFlags = flags;
        m->lba = lba;
        m->c.weaveLo = sftl.write.weaveSeq;
        m->c.weaveHi = (UInt32)(sftl.write.weaveSeq >> 16);
        m->userSeq = (UInt32)(sftl.write.weaveSeq >> 16);

        // Next
        lba++;
        m++;
        sftl.write.weaveSeq++;
    }
}

void s_SetupMeta_IntData(PageMeta_t *__meta, UInt32 lba, UInt32 count, UInt32 flags)
{
    PageMeta_Data_t *m = (PageMeta_Data_t*)__meta;

    while (count--) {
        m->c.PageType = PAGETYPE_FTL_INTDATA;
        m->c.FTLFlags = flags;
        m->lba = lba;
        m->c.weaveLo = sftl.write.weaveSeq;
        m->c.weaveHi = (UInt32)(sftl.write.weaveSeq >> 16);
        m->userSeq = (UInt32)(sftl.write.weaveSeq >> 16);

        // Next
        lba++;
        m++;
        sftl.write.weaveSeq++;
    }
}

void s_SetupMeta_Data_UECC(PageMeta_t *__meta, UInt32 lba)
{
    PageMeta_Data_t *m = (PageMeta_Data_t*)__meta;

    m->c.FTLFlags |= PFLAG_UECC;
    m->lba = lba;
}

void s_SetupMeta_Data_BTOC(PageMeta_t *__meta, WeaveSeq_t minWeaveSeq, UInt32 num_btoc_vbas, UInt32 pageFlags)
{
    UInt32 count;

    PageMeta_BTOC_t *m = (PageMeta_BTOC_t*)__meta;

    count = num_btoc_vbas;

    while (count--) {
        m->c.PageType = PAGETYPE_FTL_BTOC_DATA;
        m->c.FTLFlags = (num_btoc_vbas << 1) | (pageFlags & 1);
        m->c.weaveLo = minWeaveSeq;
        m->c.weaveHi = (UInt32)(minWeaveSeq >> 16);
        m->maxWeaveSeqAdd = (UInt32)(sftl.write.weaveSeq - minWeaveSeq);
        m->btoc_vbas = num_btoc_vbas;
        m->__rfu__ = 0xffff;

        // Next
        m++;
    }
}

void s_SetupMeta_DataGC(PageMeta_t *__meta, UInt32 count)
{
    PageMeta_Data_t *m = (PageMeta_Data_t*)__meta;

    while (count--) {
        m->c.PageType = PAGETYPE_FTL_DATA;
        if (S_LBA_STATS == m->lba) {
        m->c.PageType = PAGETYPE_FTL_INTDATA;
        }
        // Don't touch FTLFlags--might have uECC flag
        m->c.weaveLo = sftl.write.weaveSeq;
        m->c.weaveHi = (UInt32)(sftl.write.weaveSeq >> 16);
        // Don't touch LBA or userSeq

        // Next
        m++;
        sftl.write.weaveSeq++;
    }
}

void s_SetupMeta_Cxt(PageMeta_t *__meta, UInt32 count, UInt32 tag, UInt32 ofs, UInt32 per, UInt32 len)
{
    PageMeta_Cxt_t *m = (PageMeta_Cxt_t*)__meta;
    UInt32 thisCount;

    while (count--) {
        thisCount = WMR_MIN(per, len);

        m->c.PageType = PAGETYPE_FTL_CXT;
        m->c.FTLFlags = tag;
        m->c.weaveLo = sftl.write.weaveSeq;
        m->c.weaveHi = (UInt32)(sftl.write.weaveSeq >> 16);
        m->ofs = ofs;
        m->len = thisCount;
        m->__rfu__ = 0xffffffff;

        // Next
        m++;
        sftl.write.weaveSeq++;
        ofs += per;
        len -= thisCount;
    }
}

