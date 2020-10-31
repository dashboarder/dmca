/*
 * Copyright (c) 2009-2010 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */

#define AND_TRACE_LAYER FTL

#include "s_bg.h"
#include "s_gc.h"

#ifndef AND_READONLY

void s_bg_write(UInt32 writeSize)
{
    s_gc_bg(writeSize);
}

void s_bg_read(UInt32 readSize)
{
    s_gc_bg(readSize);
}

void s_bg_enq(UInt32 sb, BOOL32 scrubOnUECC)
{
    s_gc_data_enq(sb, S_GC_CTX_BG);
}

#endif

