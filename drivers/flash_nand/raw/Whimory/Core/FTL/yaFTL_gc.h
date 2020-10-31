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

#ifndef  __YAFTL_GC_H__
#define  __YAFTL_GC_H__

#include "yaFTL_whoami.h"
#include "ANDTypes.h"

// General setup/close
extern ANDStatus YAFTL_GC_Init(void);
extern void YAFTL_GC_Close(void);

// Called by Write
extern void YAFTL_GC_PreWrite(UInt32 writeSize);

// For error recovery
extern void      YAFTL_GC_Data_Enq(UInt32 block);
extern UInt32    YAFTL_GC_Data_Deq_sb(UInt32 block);
extern void      YAFTL_GC_Index_Enq(UInt32 block);

// Direct access to the machines
extern ANDStatus YAFTL_GC_Index(UInt32 block, BOOL32 scrubOnUECC);
extern ANDStatus YAFTL_GC_Data(UInt32 block, BOOL32 scrubOnUECC);

#ifdef AND_READONLY
#define CheckBlockDist(x)
#else
extern void CheckBlockDist(void);
#endif

#endif   // ----- #ifndef __YAFTL_GC_H__
