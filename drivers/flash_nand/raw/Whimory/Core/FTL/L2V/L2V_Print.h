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

#ifndef  __L2V_PRINT_H__
#define  __L2V_PRINT_H__


#include "L2V_Types.h"


extern void L2V_PrintPtr(lPtr_t lptr);
extern void L2V_PrintCU(ContigUnpacked_t *cu);
extern void L2V_PrintNode(lNode_t *node);
extern void L2V_PrintUsage(UInt32 tree);


#endif   // __L2V_PRINT_H__

