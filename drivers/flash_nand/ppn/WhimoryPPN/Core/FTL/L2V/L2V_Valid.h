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

#ifndef  __L2V_VALID_H__
#define  __L2V_VALID_H__


#include "L2V_Defines.h"

#if L2V_TRACK_VALID!=0
extern void _L2V_ValidUp(UInt32 vpn, UInt32 count);
extern void _L2V_ValidDown(UInt32 vpn, UInt32 count);
#else
#define _L2V_ValidUp(vpn, count)
#define _L2V_ValidDown(vpn, count)
#endif


#endif   // __L2V_VALID_H__

