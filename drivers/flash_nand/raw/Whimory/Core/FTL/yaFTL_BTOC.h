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

#ifndef  __YAFTL_BTOC_H__
#define  __YAFTL_BTOC_H__

// Includes
#include "yaFTL_whoami.h"
#include "ANDTypes.h"
#include "WMROAM.h"
#include "yaFTLTypes.h"
#include "yaFTL_meta.h"

// Defines and types declared elsewhere

// Prototypes:

// BTOC init/alloc/dealloc
extern BOOL32  BTOC_Init(void);
extern void    BTOC_BootFixup(void);
extern void    BTOC_Close(void);
extern UInt32 *BTOC_Alloc(UInt32 sb, BOOL32 isData);
extern void    BTOC_Dealloc(UInt32 *BTOC);
extern UInt32 *BTOC_Search(UInt32 sb, BOOL32 isData);
extern void BTOC_Lock(UInt32 sb);
extern void BTOC_Unlock(UInt32 sb);

// Read
extern ANDStatus BTOC_Read(UInt32 vpn, UInt32 *bTOC, PageMeta_t *mdPtr, BOOL32 val, BOOL32 scrubOnUECC, UInt32 upperBound);

// Src for GC
extern void   BTOC_SetSrc(UInt32 destVpn, UInt32 srcVpn);
extern UInt32 BTOC_GetSrc(UInt32 destVpn);

// Getters/setters
extern void   BTOC_SetAll(UInt32 *bTOC, UInt8 value);
extern UInt32 BTOC_Get(UInt32 *bTOC, UInt32 offset, UInt32 upperBound);
extern UInt32 BTOC_Set(UInt32 *bTOC, UInt32 offset, UInt32 val, UInt32 upperBound);

// Copy
extern void   BTOC_Copy(UInt32 *left, UInt32 *right, UInt32 upperBound);

#endif   // ----- #ifndef __YAFTL_BTOC_H__
