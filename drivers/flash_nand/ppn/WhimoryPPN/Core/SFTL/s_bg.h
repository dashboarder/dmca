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

#ifndef  __S_BG_H__
#define  __S_BG_H__

#include "s_internal.h"

#ifndef AND_READONLY

// Called by Write
extern void s_bg_write(UInt32 writeSize);
extern void s_bg_read(UInt32 readSize);

extern void s_bg_enq(UInt32 sb, BOOL32 scrubOnUECC);

#endif // AND_READONLY

#endif   // #ifndef __S_BG_H__

