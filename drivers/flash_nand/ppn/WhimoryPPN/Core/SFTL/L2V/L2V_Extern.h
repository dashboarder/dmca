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
extern UInt32 L2V_nodepool_mem;
extern UInt8 *L2V_root_depth;
extern tempNodePool_t tempNodes;

extern BOOL32 L2V_Init(UInt32 numLbas, UInt32 max_sb, UInt32 vbas_per_sb);
extern void L2V_Nuke(void);
extern void L2V_Free(void);
extern void L2V_Search_Init(L2V_SearchCtx_t *c);
extern void L2V_Search(L2V_SearchCtx_t *c);
extern void L2V_Update(UInt32 lba, UInt32 size, UInt32 vba);
extern void L2V_Repack(UInt32 tree);
extern UInt32 L2V_ForceRepack(void);
extern void L2V_FindFrag(UInt32 *d_lba, UInt32 *d_span);

#define L2V_IdleRepackThr (L2V.Pool.FreeCount <= (L2V_MAX_TREE_DEPTH*32))
#define L2V_LowishMem (L2V.Pool.FreeCount <= (L2V_MAX_TREE_DEPTH*16))
#define L2V_LowMem (L2V.Pool.FreeCount <= (L2V_MAX_TREE_DEPTH*8))
#define L2V_CriticalMem (L2V.Pool.FreeCount <= (L2V_MAX_TREE_DEPTH+1))


#endif // __L2V_EXTERN_H__

