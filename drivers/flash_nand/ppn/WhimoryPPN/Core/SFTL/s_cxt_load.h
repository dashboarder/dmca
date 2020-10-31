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

#ifndef __S_CXT_LOAD_H__
#define __S_CXT_LOAD_H__

#include "s_internal.h"

extern BOOL32 s_cxt_load(BOOL32 statsOnly, s_sb_fmt_t *shadowSB);

#ifndef AND_READONLY
extern void s_cxt_stats_load(void);
#else // ->!AND_READONLY
#define s_cxt_stats_load()
#endif // !AND_READONLY

#endif // __S_CXT_LOAD_H__
