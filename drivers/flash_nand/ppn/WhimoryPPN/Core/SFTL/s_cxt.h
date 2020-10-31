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

#ifndef __S_CXT_H__
#define __S_CXT_H__

#include "s_internal.h"

// Cxt contains:
//  - L2V tree
//  - block state and valid counts
//  - tl stats

#define s_cxt_is_firstSb(_m, _sb, _vbaOfs) ((PAGETYPE_FTL_CXT == (_m)->c.PageType) && (0 == (_vbaOfs)) && (S_CXTTAG_BASE == META_GET_CXT_TAG(_m)))

extern void   s_cxt_addSb(UInt32 sb, WeaveSeq_t weaveSeq);

extern void   s_cxt_init(void);
extern BOOL32 s_cxt_boot(const UInt32 readOnly);

#ifndef AND_READONLY
extern void   s_cxt_periodic(void);
extern void   s_cxt_save(void);
#else // -> !AND_READONLY
#define s_cxt_periodic()
#define s_cxt_save()
#endif // !AND_READONLY

#endif // __S_CXT_H__

