/*
 * Copyright (c) 2008-2010 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */

// Author:  Daniel J. Post (djp), dpost@apple.com

#ifndef __S_INIT_H__
#define __S_INIT_H__

#include "s_internal.h"
#include "VFL.h"

extern UInt32 s_calc_lbas(BOOL32 ideal);
extern Int32  sftl_init(VFLFunctions *pVFLFunctions);
extern void   sftl_close(void);

#endif // __S_INIT_H__

