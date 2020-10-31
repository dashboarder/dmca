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

#ifndef __L2V_REPACK_H__
#define __L2V_REPACK_H__


#include "WMROAM.h"

// Functions

extern void L2V_PeriodicRepack(void);
extern void L2V_Repack(UInt32 tree);
extern UInt32 L2V_ForceRepack(void);

#endif // __L2V_REPACK_H__

