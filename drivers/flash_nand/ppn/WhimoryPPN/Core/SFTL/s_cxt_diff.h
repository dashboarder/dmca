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

#ifndef __S_CXT_DIFF_H__
#define __S_CXT_DIFF_H__

#include "s_internal.h"

// Lifecycle:
extern BOOL32 s_wo_init(void);
extern void s_wo_close(void);

// sb filter:
extern void s_diff_sbFilter_add(UInt32 sb);
extern BOOL32 s_diff_sbFilter_has(UInt32 sb);
extern void s_diff_filterPost(WeaveSeq_t weave);

// Scan:
extern void s_scan_all_sb(void);
extern void s_cxt_scan_diff(void);

#endif // __S_CXT_DIFF_H__

