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

#ifndef __YAFTL_META_PPN_H__
#define __YAFTL_META_PPN_H__

#include "yaFTL_whoami.h"
#include "WMROAM.h"



// PageType defines--
#define PAGETYPE_FTL_DATA      (AND_SPARE_TYPE_REGION_FTL + 0x00)
#define PAGETYPE_FTL_BTOC_DATA (AND_SPARE_TYPE_REGION_FTL + 0x1C)
#define PAGETYPE_FTL_BTOC_IDX  (AND_SPARE_TYPE_REGION_FTL + 0x1D)
#define PAGETYPE_FTL_IDX       (AND_SPARE_TYPE_REGION_FTL + 0x1E)
#define PAGETYPE_FTL_CXT       (AND_SPARE_TYPE_REGION_FTL + 0x1F)


// FTLFlags defines--
#define PFLAG_UECC     0x01  // uECC state for data pages (moved by GC and thus re-encoded)


// Must be 16 bytes on PPN

typedef struct {
    UInt8 PageType;
    UInt8 FTLFlags;
    UInt16 weaveLo;
    UInt32 weaveHi;
} PageMeta_Common_t;

typedef struct {
    PageMeta_Common_t c;
    UInt32 __f1;
    UInt32 __f2;
} PageMeta_t;

typedef struct {
    PageMeta_Common_t c;
    UInt32 userSeq;
    UInt32 lba;
} PageMeta_Data_t;

typedef struct {
    PageMeta_Common_t c;
    UInt32 maxWeaveSeqAdd;
    UInt32 __rfu__;
} PageMeta_BTOC_t;

typedef struct {
    PageMeta_Common_t c;
    UInt16 __rfu__;
    UInt16 lbaAdd;
    UInt32 baseLba;
} PageMeta_Idx_t;

typedef struct {
    PageMeta_Common_t c;
    UInt32 __rfu1__;
    UInt32 __rfu2__;
} PageMeta_Cxt_t;


typedef UInt64 WeaveSeq_t;


// Setup
extern void SetupMeta_Data(PageMeta_t *__meta, UInt32 lba, UInt32 count);
extern void SetupMeta_Data_UECC(PageMeta_t *__meta);

extern void SetupMeta_Index(PageMeta_t *__meta, UInt32 ipn);

extern void SetupMeta_Data_BTOC(PageMeta_t *__meta);
extern void SetupMeta_Index_BTOC(PageMeta_t *__meta);

extern void SetupMeta_IndexGC(PageMeta_t *__meta, UInt32 count);
extern void SetupMeta_DataGC(PageMeta_t *__meta, UInt32 count);

extern void SetupMeta_Cxt(PageMeta_t *__meta, UInt64 weaveSeq);

#define META_SET_LBA(meta, foo)  do { ((PageMeta_Data_t*)(meta))->lba = (foo); } while (0)
#define META_SET_WEAVESEQ(meta) do { (meta)->c.weaveLo = (UInt16)yaFTL.wrState.weaveSeq; (meta)->c.weaveHi = (UInt32)(yaFTL.wrState.weaveSeq >> 16); } while (0)
#define META_SET_BTOC_DATA(meta) do { (meta)->c.PageType = PAGETYPE_FTL_BTOC_DATA; } while (0)
#define META_SET_IPN_FROM_BTOC(meta, bte) do { ((PageMeta_Idx_t*)(meta))->baseLba = (bte); } while (0)
#define BTOC_SET_FROM_IPN(bte, meta) do { (bte) = ((PageMeta_Idx_t*)(meta))->baseLba; } while (0)


// Query
#define BTOC_GET_IPN(bte)        ((bte) / yaFTL.indexPageRatio)
#define META_ARE_FLAGS_FF(meta)  ((0xff == (meta)->c.PageType) && (0xff == (meta)->c.FTLFlags))
#define META_IS_IDX(meta)        (PAGETYPE_FTL_IDX == (meta)->c.PageType)
#define META_IS_DATA(meta)       (PAGETYPE_FTL_DATA == (meta)->c.PageType)
#define META_IS_CXT(meta)        (PAGETYPE_FTL_CXT == (meta)->c.PageType)
#define META_IS_BTOC_IDX(meta)   (PAGETYPE_FTL_BTOC_IDX == (meta)->c.PageType)
#define META_IS_BTOC_DATA(meta)  (PAGETYPE_FTL_BTOC_DATA == (meta)->c.PageType)
#define META_IS_BTOC(meta)       ((PAGETYPE_FTL_BTOC_IDX == (meta)->c.PageType) || (PAGETYPE_FTL_BTOC_DATA == ((meta)->c.PageType)))
#define META_IS_UECC(meta)       ((meta)->c.FTLFlags & PFLAG_UECC)
#define META_GET_WEAVESEQ(meta)  ((UInt64)(meta)->c.weaveLo | ((UInt64)(meta)->c.weaveHi << 16))
#define META_GET_LBA(meta)       (((PageMeta_Data_t*)(meta))->lba)
#define META_GET_IPN(meta)       (((PageMeta_Idx_t*)(meta))->baseLba / yaFTL.indexPageRatio)
#define META_GET_IPNRAW(meta)    (((PageMeta_Idx_t*)(meta))->baseLba)
#define META_GET_ALL_FLAGS(meta) (((UInt32)((meta)->c.FTLFlags) << 8) | (UInt32)((meta)->c.PageType))



#endif // __YAFTL_META_PPN_H__

