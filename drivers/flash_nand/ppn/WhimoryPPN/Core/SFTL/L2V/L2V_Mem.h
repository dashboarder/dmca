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

#ifndef __L2V_MEM_H__
#define __L2V_MEM_H__


#include "L2V_Types.h"


extern void _L2V_FreeNode(lNode_t *node);
extern lNode_t *_L2V_AllocNode(void);  // Warning: may return NULL; up to caller to handle gracefully


#endif // __L2V_MEM_H__

