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

#ifndef __L2V_EXTERN_H__
#define __L2V_EXTERN_H__


#include "L2V_Types.h"


// Define the context structure for faster macro access
extern L2V_t L2V;
extern UInt8 *L2V_root_depth;

extern BOOL32 L2V_Init(UInt32 numLbas, UInt32 numSBs, UInt32 pagesPerSB);
extern void L2V_Free(void);
extern void L2V_Search_Init(L2V_SearchCtx_t *c);
extern void L2V_Search(L2V_SearchCtx_t *c);
extern void L2V_Update(UInt32 lba, UInt32 size, UInt32 vpn);
extern void L2V_Repack(UInt32 tree);
extern void L2V_ForgetBiggestTree(void);

#define L2V_LowMem (L2V.Pool.FreeCount <= (L2V_MAX_TREE_DEPTH*4))


#endif // __L2V_EXTERN_H__

