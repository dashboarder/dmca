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

#include "s_token.h"
#include "s_geom.h"

void s_token_insert(UInt32 tokenLba, UInt32 *srcBuf)
{
    UInt32 *dstBuf;
    s_wrstream_t *const wr = &sftl.write.stream[sftl.write.curStream];

    WMR_ASSERT(wr->bufLbas < wr->bufSize);

    // Put in buffer
    dstBuf = (UInt32*)&wr->bufPage[s_g_mul_bytes_per_lba(wr->bufLbas)];
    WMR_MEMCPY(dstBuf, srcBuf, s_g_bytes_per_lba);

    // Meta
    s_SetupMeta_IntData(&wr->bufMeta[wr->bufLbas], tokenLba, 1, wr->pageFlags);;

    // Next
    wr->bufLbas++;
}

