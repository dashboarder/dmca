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

#ifndef __YAFTL_META_RAW_H__
#define __YAFTL_META_RAW_H__

#include "yaFTL_whoami.h"
#include "WMROAM.h"

// Page FTLFlags
#define PAGETYPE_FTL_IDX     0x04
#define PAGETYPE_FTL_DATA    0x10
#define PAGETYPE_FTL_CXT     0x20
#define PFLAG_BTOC           0x08
#define PFLAG_UECC           0x40  // uECC state (moved by GC and thus re-encoded)

// Must be 12 bytes on raw
typedef struct
{
    UInt32 lba; // formerly known as logicalPageNo
    UInt32 weaveSeq; // formerly known as allocationNo
    UInt8 __DEPRECATED_seqNo;
    UInt8 FTLFlags;
    UInt16 __pad;
} PageMeta_t;

typedef UInt32 WeaveSeq_t;


extern void SetupMeta_Data(PageMeta_t *meta, UInt32 lba, UInt32 count);
extern void SetupMeta_Data_UECC(PageMeta_t *meta);

extern void SetupMeta_Index(PageMeta_t *meta, UInt32 ipn);

extern void SetupMeta_Data_BTOC(PageMeta_t *meta);
extern void SetupMeta_Index_BTOC(PageMeta_t *meta);

extern void SetupMeta_IndexGC(PageMeta_t *meta, UInt32 count);
extern void SetupMeta_DataGC(PageMeta_t *meta, UInt32 count);

extern void SetupMeta_Cxt(PageMeta_t *meta, UInt64 weaveSeq);

#define META_SET_LBA(meta, foo) do { (meta)->lba = (foo); } while (0)
#define META_SET_WEAVESEQ(meta) do {  \
    if (yaFTL.wrState.lastBlock != yaFTL.wrState.index.block) \
    { \
        yaFTL.wrState.weaveSeq++; \
        yaFTL.wrState.lastBlock = yaFTL.wrState.index.block; \
    } \
    (meta)->weaveSeq = (UInt32)yaFTL.wrState.weaveSeq; \
} while(0)
#define META_SET_BTOC_DATA(meta) do { (meta)->FTLFlags = PFLAG_BTOC; } while (0)
#define META_SET_IPN_FROM_BTOC(meta, bte) do { (meta)->lba = (bte); } while (0)
#define BTOC_SET_FROM_IPN(bte, meta) do { (bte) = (meta)->lba; } while (0)


// Query
#define BTOC_GET_IPN(bte)        (bte)
#define META_ARE_FLAGS_FF(meta)  (0xff == (meta)->FTLFlags)
#define META_IS_IDX(meta)        ((meta)->FTLFlags & PAGETYPE_FTL_IDX)
#define META_IS_DATA(meta)       ((meta)->FTLFlags & PAGETYPE_FTL_DATA)
#define META_IS_CXT(meta)        ((meta)->FTLFlags & PAGETYPE_FTL_CXT)
#define META_IS_BTOC_IDX(meta)   (((meta)->FTLFlags & PAGETYPE_FTL_IDX) && ((meta)->FTLFlags & PFLAG_BTOC))
#define META_IS_BTOC_DATA(meta)  (!((meta)->FTLFlags & PAGETYPE_FTL_IDX) && ((meta)->FTLFlags & PFLAG_BTOC))
#define META_IS_BTOC(meta)       ((meta)->FTLFlags & PFLAG_BTOC)
#define META_IS_UECC(meta)       ((meta)->FTLFlags & PFLAG_UECC)
#define META_GET_WEAVESEQ(meta)  ((meta)->weaveSeq)
#define META_GET_LBA(meta)       ((meta)->lba)
#define META_GET_IPN(meta)       ((meta)->lba)
#define META_GET_IPNRAW(meta)    ((meta)->lba)
#define META_GET_ALL_FLAGS(meta) ((UInt32)((meta)->FTLFlags))



#endif // __YAFTL_META_RAW_H__

