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

#ifndef __S_META_H__
#define __S_META_H__

#include "WMROAM.h"

// PageType defines--
#define PAGETYPE_FTL_DATA      (AND_SPARE_TYPE_REGION_FTL + 0x01)
#define PAGETYPE_FTL_INTDATA   (AND_SPARE_TYPE_REGION_FTL + 0x02)
#define PAGETYPE_FTL_BTOC_DATA (AND_SPARE_TYPE_REGION_FTL + 0x1C)
#define PAGETYPE___rfu1___     (AND_SPARE_TYPE_REGION_FTL + 0x1D)
#define PAGETYPE___rfu2___     (AND_SPARE_TYPE_REGION_FTL + 0x1E)
#define PAGETYPE_FTL_CXT       (AND_SPARE_TYPE_REGION_FTL + 0x1F)


// FTLFlags defines--
#define PFLAG_STATIC   0x01  // Preserve static/dynamic inferences and hints
#define PFLAG_UECC     0x02  // uECC state for data pages (moved by GC and thus re-encoded)


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
    UInt32 lba;
    UInt32 userSeq;
} PageMeta_Data_t;

typedef struct {
    PageMeta_Common_t c;
    UInt32 maxWeaveSeqAdd;
    UInt16 btoc_vbas;
    UInt16 __rfu__;
} PageMeta_BTOC_t;

typedef struct {
    PageMeta_Common_t c;
    UInt16 ofs;
    UInt16 len;
    UInt32 __rfu__;
} PageMeta_Cxt_t;


typedef UInt64 WeaveSeq_t;


// Setup
extern void s_SetupMeta_TrimRead(PageMeta_t *__meta, UInt32 lba, UInt32 count, UInt32 flags);
extern void s_SetupMeta_Trim(PageMeta_t *__meta, UInt32 lba, UInt32 count, UInt32 flags);
extern void s_SetupMeta_Data(PageMeta_t *__meta, UInt32 lba, UInt32 count, UInt32 flags);
extern void s_SetupMeta_IntData(PageMeta_t *__meta, UInt32 lba, UInt32 count, UInt32 flags);
extern void s_SetupMeta_Data_UECC(PageMeta_t *__meta, UInt32 lba);
extern void s_SetupMeta_Data_Padding(PageMeta_t *meta, UInt32 count, UInt32 flags);

extern void s_SetupMeta_Index(PageMeta_t *__meta, UInt32 ipn);

extern void s_SetupMeta_Data_BTOC(PageMeta_t *__meta, WeaveSeq_t minWeaveSeq, UInt32 num_btoc_pages, UInt32 pageFlags);
extern void s_SetupMeta_DataGC(PageMeta_t *__meta, UInt32 count);

extern void s_SetupMeta_Cxt(PageMeta_t *__meta, UInt32 count, UInt32 tag, UInt32 ofs, UInt32 per, UInt32 len);

#define META_SET_LBA(meta, foo)  do { ((PageMeta_Data_t*)(meta))->lba = (foo); } while (0)
#define META_SET_CXT_TAG(meta, foo)  do { ((PageMeta_Cxt_t*)(meta))->c.FTLFlags = (foo); } while (0)
#define META_SET_WEAVESEQ(meta) do { (meta)->c.weaveLo = (UInt16)sftl.write.weaveSeq; (meta)->c.weaveHi = (UInt32)(sftl.write.weaveSeq >> 16); sftl.write.weaveSeq++; } while (0)
#define META_SET_LBA_UECC(meta) do { ((PageMeta_Data_t*)(meta))->lba = 0xfffffffe; } while (0)
#define META_SET_LBA_CLEAN(meta) do { ((PageMeta_Data_t*)(meta))->lba = 0xffffffff; } while (0)

// Query
#define META_IS_STATIC(meta)     (((PageMeta_t*)(meta))->c.FTLFlags & PFLAG_STATIC)
#define META_IS_DATA(meta)       ((PAGETYPE_FTL_DATA == (meta)->c.PageType) || (PAGETYPE_FTL_INTDATA == (meta)->c.PageType))
#define META_IS_CXT(meta)        (PAGETYPE_FTL_CXT == (meta)->c.PageType)
#define META_IS_BTOC_DATA(meta)  (PAGETYPE_FTL_BTOC_DATA == (meta)->c.PageType)
#define META_IS_UECC(meta)       ((meta)->c.FTLFlags & PFLAG_UECC)
#define META_GET_PAGETYPE(meta)  ((UInt32)((meta)->c.PageType))
#define META_GET_WEAVESEQ(meta)  ((UInt64)(meta)->c.weaveLo | ((UInt64)(meta)->c.weaveHi << 16))
#define META_GET_USERSEQ(meta)   (((PageMeta_Data_t*)(meta))->userSeq)
#define META_GET_LBA(meta)       (((PageMeta_Data_t*)(meta))->lba)
#define META_GET_CXT_TAG(meta)   (((PageMeta_Cxt_t*)(meta))->c.FTLFlags)
#define META_GET_CXT_OFS(meta)   ((UInt32)(((PageMeta_Cxt_t*)(meta))->ofs))
#define META_GET_CXT_LEN(meta)   ((UInt32)(((PageMeta_Cxt_t*)(meta))->len))
#define META_IS_LBA_UECC(meta)   (0xfffffffe == ((PageMeta_Data_t*)(meta))->lba)
#define META_IS_LBA_CLEAN(meta)  (0xffffffff == ((PageMeta_Data_t*)(meta))->lba)

// BTOC:
#define META_GET_WEAVESEQ_MIN(meta) ((UInt64)(meta)->c.weaveLo | ((UInt64)(meta)->c.weaveHi << 16))
#define META_GET_WEAVESEQ_MAX(meta) (((UInt64)(meta)->c.weaveLo | ((UInt64)(meta)->c.weaveHi << 16)) + (UInt64)(meta)->maxWeaveSeqAdd)
#define META_BTOC_GET_NUM_VBAS(meta) ((((PageMeta_BTOC_t*)meta)->btoc_vbas == 0xffff) ? ((meta)->c.FTLFlags>>1) : (((PageMeta_BTOC_t*)meta)->btoc_vbas)) 
#define META_BTOC_SET_NUM_VBAS(meta, foo) do { (meta)->c.FTLFlags = (foo) << 1; ((PageMeta_BTOC_t*)meta)->btoc_vbas = foo; } while (0)

#endif // __S_META_H__

